rm -r ./bin
rm -r ./CMakeFiles
rm cmake_install.cmake
rm CMakeCache.txt
rm Makefile
rm -r ./build

cd src/nvgpu && make clean
cd -
cd src/nvgpu_example && make clean
cd -
cd src/dcu && make clean
cd -
cd src/dcu_example && make clean
cd -
cd src/mlu && make clean
cd -