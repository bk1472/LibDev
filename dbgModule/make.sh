#!/bin/bash

CUR_DIR=`pwd`
CROSS_COMPILE=
PLATFORM="x64"

case $# in
	0) PLATFORM="x64";
		;;
	1) 	if [ "$1" == "arm64" ]; then
			CROSS_COMPILE="aarch64-linux-gnu-";
			PLATFORM="aarch64";
		fi;
esac

test -d ${CUR_DIR}/build_${PLATFORM} || mkdir ${CUR_DIR}/build_${PLATFORM}

pushd ${CUR_DIR}/build_${PLATFORM} > /dev/null 2>&1
cmake -DPLATFORM=${PLATFORM} \
	-DCMAKE_CXX_COMPILER=/usr/bin/${CROSS_COMPILE}g++ \
	-DCMAKE_C_COMPILER=/usr/bin/${CROSS_COMPILE}gcc \
	-DCMAKE_LINKER=/usr/bin/${CROSS_COMPILE}ld \
	-DCMAKE_AR=/usr/bin/${CROSS_COMPILE}ar \
	-DCMAKE_NM=/usr/bin/${CROSS_COMPILE}nm \
	-DCMAKE_OBJCOPY=/usr/bin/${CROSS_COMPILE}objcopy \
	-DCMAKE_OBJDUMP=/usr/bin/${CROSS_COMPILE}objdump \
	-DCMAKE_RANLIB=/usr/bin/${CROSS_COMPILE}ranlib \
	-DCMAKE_STRIP=/usr/bin/${CROSS_COMPILE}strip \
	..
popd > /dev/null 2>&1
