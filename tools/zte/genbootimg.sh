#mkbootimg  --kernel zImage --ramdisk ramdisk.img --output boot.img
KERNEL_ROOT=`pwd`
PACK_DIR=$KERNEL_ROOT/out/$1/test
TOOLS_DIR=$KERNEL_ROOT/tools/zte
cd $PACK_DIR

echo "****Starting pack a new bootimg.Info:"
echo "	KERNEL_ROOT: $KERNEL_ROOT"
echo "	PACK_DIR: $PACK_DIR"
echo "	TOOLS_DIR: $TOOLS_DIR"

$TOOLS_DIR/unpack-bootimg.pl boot.img
mv boot.img boot.img.old
$TOOLS_DIR/mkbootfs boot.img-ramdisk | $TOOLS_DIR/minigzip > ramdisk.img
$TOOLS_DIR/mkbootimg  --kernel $KERNEL_ROOT/out/$1/arch/arm/boot/zImage --ramdisk ramdisk.img --base 0x80200000 --pagesize 2048 --cmdline "console=ttyHSL0,115200,n8 androidboot.hardware=qcom user_debug=31 loglevel=7" --output boot.img
