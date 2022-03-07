#!/bin/bash

# Requires ImageMagick

SCRIPT_PATH=`readlink -f "$0"`
SCRIPT_DIR=`dirname "$SCRIPT_PATH"`

mkdir -p ${SCRIPT_DIR}/win
convert -density 256x256 -background transparent ${SCRIPT_DIR}/qlogexplorer.svg -define icon:auto-resize -colors 256 ${SCRIPT_DIR}/win/qlogexplorer.ico

XDG_ICONS_DIR=${SCRIPT_DIR}/linux/xdg/icons/hicolor
mkdir -p ${XDG_ICONS_DIR}/scalable
cp ${SCRIPT_DIR}/qlogexplorer.svg ${XDG_ICONS_DIR}/scalable

for iconSize in 16 24 32 64 128 256;
do
    targetImgDir=${XDG_ICONS_DIR}/${iconSize}x${iconSize}/apps
    mkdir -p ${targetImgDir}
    convert -density 1200 -background transparent -resize ${iconSize}x${iconSize} ${SCRIPT_DIR}/qlogexplorer.svg ${targetImgDir}/qlogexplorer.png
done
