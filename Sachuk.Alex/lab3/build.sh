BUILD_DIR="build"
PROG_DIR="./"
FILE="lab3"

mkdir $BUILD_DIR
cd $BUILD_DIR
cmake ..
make
cd ..

cp $BUILD_DIR/$FILE $PROG_DIR

rm -rf $BUILD_DIR
