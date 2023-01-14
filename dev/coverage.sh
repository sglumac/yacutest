CURRENT_DIR=$(pwd)
SCRIPT_DIR=$(dirname "$0")
BUILD_DIR=$SCRIPT_DIR/../build

rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR
cmake -DCOVERAGE=True -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --target yacucoverage
cd $CURRENT_DIR

