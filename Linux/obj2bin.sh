#!/bin/bash
set -e            # Exit on error
set -o pipefail # Exit if any command in a pipe fails

LOGFILE=step2gltf_x64.txt

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Helper functions
print_error() {
    echo -e "${RED}ERROR: $1${NC}"
}

print_success() {
    echo -e "${GREEN}$1${NC}"
}

print_warning() {
    echo -e "${YELLOW}WARNING: $1${NC}"
}

compile_module() {
    local module=$1
    echo "Compiling $module..."
    if ! g++ -Wall -Wno-deprecated-declarations -Wno-unused-function -fPIC -DNDEBUG -DEMBEDDED_SCHEMAS -c $module/*.cpp 2>&1 | tee -a $LOGFILE; then
        print_error "Compilation failed for $module/*.cpp"
        return 1
    fi
}

# Cleanup
rm -f ./$LOGFILE
if ls ./*.o 1> /dev/null 2>&1; then
    rm ./*.o;
fi

export CPLUS_INCLUDE_PATH=./step2gltf/include:./step2gltf/parsers:./step2gltf/obj:./step2gltf/sdk/src:./step2gltf/gltf

echo "Compiling obj2bin..."
g++ ./obj2bin.cpp ./libstep2gltf.a ./libifcengine.a -o obj2bin 2>&1 | tee -a $LOGFILE
if [ $? -ne 0 ]; then
    print_error "Compilation failed for obj2bin"
    exit 1
fi

# Cleanup
if ls ./*.o 1> /dev/null 2>&1; then
    rm ./*.o;
fi