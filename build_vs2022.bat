git pull --recurse-submodules
git submodule update --init --recursive
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
