rm -r ./bin
rm -r ./CMakeFiles
rm cmake_install.cmake
rm CMakeCache.txt
rm Makefile
rm -r ./build

cd src/cuda && make clean
cd src/cuda_example && make clean
cd src/host && make clean