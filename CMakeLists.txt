cmake_minimum_required(VERSION 3.4)
cmake_policy(SET CMP0077 NEW) 

project(test-cicd LANGUAGES C)

############################################################
# Create 3rdparty Libraries
############################################################

if (WIN32)
  set(PLATFORM_NAME win)
elseif(APPLE)
  set(PLATFORM_NAME macos)
else()
  set(PLATFORM_NAME linux)
endif()

set (LIBRESSL_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/SSL)
set (LIBRESSL_INSTALL ${LIBRESSL_INSTALL_PREFIX}/${PLATFORM_NAME}/) 

# Find Include Path
find_path(LIBRESSL_INCLUDE_DIR NAMES tls.h HINTS ${LIBRESSL_INSTALL_PREFIX} NO_DEFAULT_PATH PATH_SUFFIXES include
)

# Find SSL Library
find_library(LIBRESSL_SSL NAMES ssl libssl ssl-48 NAMES_PER_DIR HINTS ${LIBRESSL_INSTALL} NO_DEFAULT_PATH)

# Find Crypto Library
find_library(LIBRESSL_CRYPTO NAMES crypto libcrypto crypto-46 NAMES_PER_DIR HINTS ${LIBRESSL_INSTALL} NO_DEFAULT_PATH)
  
# Find Version File
if(LIBRESSL_INCLUDE_DIR AND EXISTS "${LIBRESSL_INCLUDE_DIR}/openssl/opensslv.h")

    # Get Version From File
    file(STRINGS "${LIBRESSL_INCLUDE_DIR}/openssl/opensslv.h" OPENSSLV.H REGEX "#define LIBRESSL_VERSION_TEXT[ ]+\".*\"")

    # Match Version String
    string(REGEX REPLACE ".*\".*([0-9]+)\\.([0-9]+)\\.([0-9]+)\"" "\\1;\\2;\\3" LIBRESSL_VERSION_LIST "${OPENSSLV.H}")

    # Split Parts
    list(GET LIBRESSL_VERSION_LIST 0 LIBRESSL_VERSION_MAJOR)
    list(GET LIBRESSL_VERSION_LIST 1 LIBRESSL_VERSION_MINOR)
    list(GET LIBRESSL_VERSION_LIST 2 LIBRESSL_VERSION_REVISION)

    # Set Version String
    set(LIBRESSL_VERSION "${LIBRESSL_VERSION_MAJOR}.${LIBRESSL_VERSION_MINOR}.${LIBRESSL_VERSION_REVISION}")
endif()  
    
if(LIBRESSL_SSL AND LIBRESSL_CRYPTO AND LIBRESSL_INCLUDE_DIR) 
  message(STATUS "Found LibreSSL ${LIBRESSL_VERSION}, SSL: ${LIBRESSL_SSL}, CRYPTO: ${LIBRESSL_CRYPTO}")
  include_directories(${LIBRESSL_INCLUDE_DIR})
  message(STATUS "include: ${LIBRESSL_INCLUDE_DIR}")
      
else()  
  # fetch libressl if needed
  message(STATUS "Not Found LibreSSL in ${LIBRESSL_INSTALL}: adding libressl")
  include(FetchContent)
  
  set (LibreSSL_URL https://ftp.openbsd.org/pub/OpenBSD/LibreSSL/libressl-3.1.4.tar.gz)
  set (LibreSSL_HASH SHA256=414c149c9963983f805a081db5bd3aec146b5f82d529bb63875ac941b25dcbb6)

  FetchContent_Declare(
    LibreSSL
    QUIET OFF
    URL ${LibreSSL_URL}
    URL_HASH ${LibreSSL_HASH}
  )
  FetchContent_GetProperties(LibreSSL)
  if(NOT libressl_POPULATED)
    # real fetch
    message(STATUS "Downloading libressl from ${LibreSSL_URL}")
    FetchContent_Populate(libressl)
    message(STATUS "Downloading libressl - done")
    
    # set preferences for the LibreSSL CMake
    set (LIBRESSL_TESTS NO)
    set (LIBRESSL_APPS NO)
    set (LIBRESSL_SKIP_INSTALL YES)
    # fix the installation of pkgconfig, that is executed even with LIBRESSL_SKIP_INSTALL
    set (CMAKE_INSTALL_PREFIX SSL)
    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
    
    # add LibreSSL directories
    add_subdirectory(${libressl_SOURCE_DIR} ${libressl_BINARY_DIR})
  endif()
  
  set (LIBRESSL_SSL ssl)
  set (LIBRESSL_CRYPTO crypto)
  
  install(
	TARGETS ssl crypto
	ARCHIVE DESTINATION ${LIBRESSL_INSTALL}
  )
	
  install(DIRECTORY ${libressl_SOURCE_DIR}/include
	DESTINATION ${LIBRESSL_INSTALL_PREFIX}
	PATTERN "CMakeLists.txt" EXCLUDE
	PATTERN "compat" EXCLUDE
	PATTERN "pqueue.h" EXCLUDE
	PATTERN "Makefile*" EXCLUDE
  )	
  
  message(STATUS "Adding libressl - done")
endif()

############################################################
# Platform specific definitions
############################################################

if (WIN32)
  add_definitions(-DCUBESQL_WIN=1 -DCUBESQL_ASYNC_QUEUE=1 -DSQLITE_OS_WIN=1)
elseif(APPLE)
  add_definitions(-DCUBESQL_MAC=1 -DCUBESQL_KQUEUE=1 -DSQLITE_OS_UNIX=1 -Wno-multichar)
else()
  add_definitions(-DCUBESQL_UNIX=1 -DCUBESQL_EPOOL=1 -DSQLITE_OS_UNIX=1 -Wno-multichar)
endif()


############################################################
# Global definitions
############################################################

add_definitions(-DHAVE_USLEEP=1 -DSQLITE_THREADSAFE=1 -DNDEBUG=1 -DSQLITE_HAS_CODEC=1 -DSQLITE_ENABLE_LOAD_EXTENSION=1 -DSQLITE_ENABLE_COLUMN_METADATA=1 -DSQLITE_AMALGAMATION=1 -DSQLITE_OMIT_TCL_VARIABLE=1 -DSQLITE_ENABLE_FTS3_TOKENIZER=1 -DSQLITE_ENABLE_FTS3=1 -DSQLITE_ENABLE_FTS3_PARENTHESIS=1 -DSQLITE_ENABLE_FTS4=1 -DSQLITE_ENABLE_FTS4_UNICODE61=1 -DSQLITE_ENABLE_FTS5=1 -DSQLITE_ENABLE_RTREE=1 -DSQLITE_ENABLE_JSON1=1 -DSQLITE_ENABLE_GEOPOLY=1 -DSQLITE_ENABLE_ZIPVFS=1 -DSQLITE_OMIT_DEPRECATED=1 -DSQLITE_DEFAULT_FOREIGN_KEYS=1)


############################################################
# Add sources (and create the executable)
############################################################

set(test_sources 
	src/main.c)
    
add_executable(${PROJECT_NAME} ${test_sources})


############################################################
# Link libraries
############################################################

target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRESSL_SSL} ${LIBRESSL_CRYPTO})

# platform specific linked libraries
if (WIN32)

elseif(APPLE)
  find_package (ZLIB)
  if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    target_link_libraries (${PROJECT_NAME} PRIVATE ${ZLIB_LIBRARIES})
  endif (ZLIB_FOUND)

else()
  
endif()


############################################################
# Tests
############################################################

enable_testing()
add_test(test_executable test-cicd)