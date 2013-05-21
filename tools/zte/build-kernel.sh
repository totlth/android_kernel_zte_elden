#! /bin/bash
#$1: this is the board name,such as adam,baker
#cwd is kernel root dir

crosscompile=/home/zerg/workshop/8960/boot-0306/prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin/arm-eabi-
outdir=./out/$1/

function firstbuild()
{
echo "**$1**This is the firtbuild procedure"
mkdir -p ./out/$1/test
make O=./out/$1 ARCH=arm CROSS_COMPILE=$crosscompile msm8960-$1_defconfig
make -j4 O=./out/$1 ARCH=arm CROSS_COMPILE=$crosscompile
}

function rebuild()
{
echo "**$1**This is the rebuild procedure"
make -j4 O=./out/$1 ARCH=arm CROSS_COMPILE=$crosscompile
}

function original()
{
echo "**$1**This is the default kernel procedure"
mkdir -p ./out/$1
make -j4 O=./out/$1 ARCH=arm CROSS_COMPILE=$crosscompile $2
}

function cleanbuild()
{
echo "**$1**This is the clean procedure"
make -j4 O=./out/$1 ARCH=arm CROSS_COMPILE=$crosscompile distclean
}

function helpbuild()
{
echo "****This is the help"
make ARCH=arm CROSS_COMPILE=$crosscompile help
}

function packbuild()
{
echo "****This is the pack procedure"
./tools/zte/genbootimg.sh $1
}
