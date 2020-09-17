# test-ci-c

## Test commands
cmake -E make_directory build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
ctest -C Debug
cmake --build . --target install
