#!/bin/sh
ROOT_PATH=$(pwd)


cd ./hlmc/lib
make clean
make -f Makefile
echo "################ Finished build HLMC lib ###################"

cd ../demo
make clean
make -f Makefile
echo "################ Finished build HLMC demo ###################"

cd ../../hlmd/lib
make clean
make -f Makefile
echo "################ Finished build HLMD lib ###################"

cd ../demo
make clean
make -f Makefile
echo "################ Finished build HLMD demo ###################"