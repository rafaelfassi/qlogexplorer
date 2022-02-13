#!/bin/bash

# Command line argument:
# 0 - For Inkscape version < v1.0 [default]
# 1 - For Inkscape version >= v1.0

SCRIPT_PATH=`readlink -f "$0"`
inkscapeVersion=${1:-0}

SCRIPT_DIR=`dirname "$SCRIPT_PATH"`
SVG_IMG_DIR=${SCRIPT_DIR}/svg
DEFAULT_IMG_DIR=${SCRIPT_DIR}/default
DARK_IMG_DIR=${SCRIPT_DIR}/dark

mkdir ${DEFAULT_IMG_DIR}
mkdir ${DARK_IMG_DIR}

function svg2Png()
{
    imgSz=$1
    svgInp=$2
    pngOut=$3

    if [ $inkscapeVersion -lt 1 ]
    then
        inkscape -z -w ${imgSz} -h ${imgSz} ${svgInp} -e ${pngOut}
    else
        inkscape -w ${imgSz} -h ${imgSz} ${svgInp} -o ${pngOut}
    fi
}

for svgFile in ${SVG_IMG_DIR}/*.svg
do
    baseFileName="${svgFile##*/}"
    baseFileName="${baseFileName%%.*}"

    svg2Png 32 ${svgFile} ${DEFAULT_IMG_DIR}/${baseFileName}.png
    sed -i "s/#000000/#FFFFFF/" $svgFile
    svg2Png 32 ${svgFile} ${DARK_IMG_DIR}/${baseFileName}.png
    sed -i "s/#FFFFFF/#000000/" $svgFile
done
