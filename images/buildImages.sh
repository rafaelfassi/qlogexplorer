#!/bin/bash

# Requires ImageMagick

SCRIPT_PATH=`readlink -f "$0"`

SCRIPT_DIR=`dirname "$SCRIPT_PATH"`
SVG_IMG_DIR=${SCRIPT_DIR}/svg
DEFAULT_IMG_DIR=${SCRIPT_DIR}/default
DARK_IMG_DIR=${SCRIPT_DIR}/dark

mkdir -p ${DEFAULT_IMG_DIR}
mkdir -p ${DARK_IMG_DIR}

function svg2Png()
{
    imgSz=$1
    svgInp=$2
    pngOut=$3

    convert -define png:exclude-chunks=all -density 1200 -background transparent -resize ${imgSz}x${imgSz} ${svgInp} ${pngOut}
}

svg2Png 64 ${SVG_IMG_DIR}/qlogexplorer.svg ${SCRIPT_DIR}/qlogexplorer.png

for svgFile in ${SVG_IMG_DIR}/*.svg
do
    baseFileName="${svgFile##*/}"
    baseFileName="${baseFileName%%.*}"

    if [ $baseFileName != "qlogexplorer" ];
    then
        svg2Png 32 ${svgFile} ${DEFAULT_IMG_DIR}/${baseFileName}.png
        sed -i "s/#000000/#FFFFFF/" $svgFile
        svg2Png 32 ${svgFile} ${DARK_IMG_DIR}/${baseFileName}.png
        sed -i "s/#FFFFFF/#000000/" $svgFile
    fi
done
