cmake_minimum_required(VERSION 3.4)
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

# add target install_libs
add_custom_target(install_libs
  COMMAND ${CMAKE_COMMAND} -E echo install_libs
  COMMENT "install_libs"
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

set (LibreSSL_INSTALL ${PROJECT_SOURCE_DIR}/SSL/${PLATFORM_NAME}/) 
set (LibreSSL_INCLUDE ${PROJECT_SOURCE_DIR}/SSL/include)

find_library(LIBSSL NAMES ssl HINTS ${LibreSSL_INSTALL} NO_DEFAULT_PATH)
find_library(LIBCRYPTO NAMES crypto HINTS ${LibreSSL_INSTALL} NO_DEFAULT_PATH)
  
if(LIBSSL AND LIBCRYPTO) 
  message(STATUS "Found LIBSSL: ${LIBSSL}")
  message(STATUS "Found LIBCRYPTO: ${LIBCRYPTO}")
  include_directories(${LibreSSL_INCLUDE})
  message(STATUS "include: ${LibreSSL_INCLUDE}")
  link_directories(${LibreSSL_INSTALL})
  
else()
  # fetch libressl if needed
  message(STATUS "Not Found LIBSSL/LIBCRYPTO in ${LibreSSL_INSTALL}: adding libressl")
  include(FetchContent)
  FetchContent_Declare(
    LibreSSL
    QUIET OFF
    URL https://ftp.openbsd.org/pub/OpenBSD/LibreSSL/libressl-3.1.4.tar.gz
    URL_HASH SHA256=414c149c9963983f805a081db5bd3aec146b5f82d529bb63875ac941b25dcbb6
  )
  FetchContent_GetProperties(LibreSSL)
  if(NOT libressl_POPULATED)
    # real fetch
    message(STATUS "Downloading libressl")
    FetchContent_Populate(libressl)
    message(STATUS "Downloading libressl - done")
    add_subdirectory(${libressl_SOURCE_DIR} ${libressl_BINARY_DIR})
  endif()
  
  # use 'make install_libs' to install the static libraries to the SSL directory in the project root dir
  add_custom_command(
    TARGET install_libs POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${LibreSSL_INSTALL}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${LibreSSL_INCLUDE}/openssl
    
    if (WIN32)
      COMMAND ${CMAKE_COMMAND} -E copy ${libressl_BINARY_DIR}/ssl/${CMAKE_BUILD_TYPE}/* ${libressl_BINARY_DIR}/crypto/${CMAKE_BUILD_TYPE}/* ${LibreSSL_INSTALL}
    else()
      COMMAND ${CMAKE_COMMAND} -E copy ${libressl_BINARY_DIR}/ssl/libssl.a ${libressl_BINARY_DIR}/crypto/libcrypto.a ${LibreSSL_INSTALL}
      COMMAND ${CMAKE_COMMAND} -E copy ${libressl_SOURCE_DIR}/include/*.h ${LibreSSL_INCLUDE}
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${libressl_SOURCE_DIR}/include/openssl ${LibreSSL_INCLUDE}/openssl
    endif()
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

target_link_libraries(${PROJECT_NAME} PRIVATE ssl crypto)

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