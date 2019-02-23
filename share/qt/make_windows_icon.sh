#!/bin/bash
# create multiresolution windows icon
#mainnet
ICON_SRC=../../src/qt/res/icons/BS.png
ICON_DST=../../src/qt/res/icons/BS.ico
convert ${ICON_SRC} -resize 16x16 BS-16.png
convert ${ICON_SRC} -resize 32x32 BS-32.png
convert ${ICON_SRC} -resize 48x48 BS-48.png
convert BS-16.png BS-32.png BS-48.png ${ICON_DST}
#testnet
ICON_SRC=../../src/qt/res/icons/BS_testnet.png
ICON_DST=../../src/qt/res/icons/BS_testnet.ico
convert ${ICON_SRC} -resize 16x16 BS-16.png
convert ${ICON_SRC} -resize 32x32 BS-32.png
convert ${ICON_SRC} -resize 48x48 BS-48.png
convert BS-16.png BS-32.png BS-48.png ${ICON_DST}
rm BS-16.png BS-32.png BS-48.png
#regtest
ICON_SRC=../../src/qt/res/icons/BS_regtest.png
ICON_DST=../../src/qt/res/icons/BS_regtest.ico
convert ${ICON_SRC} -resize 16x16 BS-16.png
convert ${ICON_SRC} -resize 32x32 BS-32.png
convert ${ICON_SRC} -resize 48x48 BS-48.png
convert BS-16.png BS-32.png BS-48.png ${ICON_DST}
rm BS-16.png BS-32.png BS-48.png
#unittest
ICON_SRC=../../src/qt/res/icons/BS_unittest.png
ICON_DST=../../src/qt/res/icons/BS_unittest.ico
convert ${ICON_SRC} -resize 16x16 BS-16.png
convert ${ICON_SRC} -resize 32x32 BS-32.png
convert ${ICON_SRC} -resize 48x48 BS-48.png
convert BS-16.png BS-32.png BS-48.png ${ICON_DST}
rm BS-16.png BS-32.png BS-48.png