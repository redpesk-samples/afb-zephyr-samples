#!/bin/bash

base=$(realpath $(dirname $0))

unset $BOARD
unset $ZEPHYR_PATH
unset $SDK_PATH

print_param()
{
    echo -e "\n\033[31mUsed parameters:\033[0m"
    echo -e "Target board:\t\t$BOARD"
    echo -e "Target CPU:\t\t$CPU"
    echo -e "Zephyr path:\t\t$ZEPHYR_PATH"
    echo -e "Zephyr SDK path:\t$SDK_PATH"
    for i in {1..3}
    do
        echo -n "."
        sleep 1
    done
    echo -e "\n"
}

usage()
{
    echo -e "Usage: $0 [options]"
    echo -e "\t-h, --help: display help"
    echo -e "\t-b, --board <board_string>: target board                       [default: b_u585i_iot02a --shield link_board_eth"
    echo -e "\t-z, --zephyr-path <path>: path to the zephyr kernel            [default: ~/zephyrproject/zephyr]"
    echo -e "\t-s, --sdk-path <path>: path to the zephyr SDK                  [default: ~/zephyr-sdk-0.16.8]"
    exit 2
}

options=$(getopt -o h,b:,z:,s: -l help,board:,zephyr-path:,sdk-path: -- "$@")
[ $? != "0" ] && usage

while true; do
	case "$1" in
        -h|--help)                                          usage ;;
        -b|--board)          shift; BOARD=$1                ; shift ;;
        -z|--zephyr-path)    shift; ZEPHYR_PATH=$1          ; shift ;;
        -s|--sdk-path)       shift; SDK_PATH=$1             ; shift ;;
        *)                                                  break ;;
    esac
done

[ -z "$BOARD" ]        && BOARD="b_u585i_iot02a --shield link_board_eth"
[ -z "$CPU" ]          && CPU="STM32U585xx"
[ -z "$ZEPHYR_PATH" ]  && ZEPHYR_PATH="$HOME/zephyrproject/zephyr"
[ -z "$SDK_PATH" ]     && SDK_PATH="$HOME/zephyr-sdk-0.16.9"

print_param

if [ -f $ZEPHYR_PATH/zephyr-env.sh ]; then
    source $ZEPHYR_PATH/zephyr-env.sh
else
    echo "Can't source zephyr env at \"$ZEPHYR_PATH/zephyr-env.sh\" as it does not exist, exiting ..."
    exit 1
fi

if [ -f $SDK_PATH/sdk_version ]; then
    echo -n "Using SDK version: " && cat $SDK_PATH/sdk_version && echo
else
    echo "Can't find your Zephyr SDK files, exiting ..."
    exit 1
fi

cd $(dirname $0)
cd sdk
west build -b $BOARD -p always
west build -t llext-edk
cd ..

export LLEXT_EDK_INSTALL_DIR=$PWD/afb-sdk
export ZEPHYR_SDK_INSTALL_DIR=$SDK_PATH
export AFB_SDK_INSTALL_DIR=$base/afb-sdk

tar xf sdk/build/zephyr/afb-sdk.tar.xz
if [ "$(awk -F' = ' '/VERSION_MAJOR/ {print $2}' $ZEPHYR_PATH/VERSION)" -lt 4 ]; then
	sed -i "s/^LLEXT_CFLAGS =/& -D$CPU /" afb-sdk/Makefile.cflags
	sed -i "s/^set(LLEXT_CFLAGS /&-D$CPU;/" afb-sdk/cmake.cflags
fi

cd ext1
rm -r build
cmake -B build
make -C build
cd ..

cd ext2
rm -r build
cmake -B build
make -C build
cd ..

cd app
west build -b $BOARD -p always
cd ..
