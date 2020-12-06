set DebugBuildPath=./build/Debug

cmake -G "Visual Studio 16 2019" -B%DebugBuildPath% -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_BUILD_ARCH="x64"
cmake --build %DebugBuildPath% --target yae_run
