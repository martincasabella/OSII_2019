#!/bin/bash
rm -R output
mkdir output

echo "Proceding to do MAKEFILE"
echo " "
set OMP_NUM_THREADS = 8
export OMP_NUM_THREADS=8
set OMP_NUM_THREADS = 8
make clean
make

echo " "
echo "Removing old archives"
echo " "
rm src/Python\ files/org_img.bin
rm src/Python\ files/procedural_out.bin
rm src/Python\ files/parallel_out1.bin
rm src/Python\ files/procedural.nc
rm src/Python\ files/parallel.nc
rm src/Python\ files/parallel2.nc



echo "Proceeding to execute program"
cd bin
./main


cd ../output
ls
echo "Copying program output"
cp org_img.bin ../src/Python\ files
cp procedural_out.bin ../src/Python\ files
cp parallel_out1.bin ../src/Python\ files
#cp procedural.nc ../src/Python\ files
#cp parallel.nc ../src/Python\ files
cp parallel2.nc ../src/Python\ files
#cp parallel_out2.bin ../src/Python\ files
