#!/bin/bash
set -o errexit

source ./config.sh

export ALCHEMIST_EXE=$ALCHEMIST_PATH/target/alchemist

CURR_DIR=$PWD

echo " "
cd $ALCHEMIST_PATH
echo "Building Alchemist for $SYSTEM"
LINE="======================="
for i in `seq 1 ${#SYSTEM}`;
do
	LINE="$LINE="
done
echo $LINE
echo " "
echo "Creating Alchemist executable:"
echo " "
cd ./build/$SYSTEM/
nice make -j4
cd ../..
echo " "
echo $LINE
echo " "
echo "Building process for Alchemist has completed"
echo " "
echo "If no issues occurred during build:"
echo "  Alchemist executable located at:    $ALCHEMIST_EXE"
echo " "
echo "  Run './start.sh' to start Alchemist"
echo " "
cd $CURR_DIR
