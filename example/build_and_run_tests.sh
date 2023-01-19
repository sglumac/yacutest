echo Build ...
CURRENT_DIR=$(pwd)
SCRIPT_DIR=$(dirname "$0")
BUILD_DIR=$SCRIPT_DIR/build

mkdir $BUILD_DIR
cd $BUILD_DIR
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
cd $CURRENT_DIR

echo
echo Run tests ...
./$BUILD_DIR/tests/tests
