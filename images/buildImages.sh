#!/bin/bash

# Command line argument:
# 0 - For Inkscape version < v1.0 [default]
# 1 - For Inkscape version >= v1.0

inkscapeVersion=${1:-0}

mkdir default
mkdir dark

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

for svgFile in svg/*.svg
do
    baseFileName="${svgFile##*/}"
    baseFileName="${baseFileName%%.*}"

    svg2Png 32 ${svgFile} default/${baseFileName}.png
    sed -i "s/#000000/#FFFFFF/" $svgFile
    svg2Png 32 ${svgFile} dark/${baseFileName}.png
    sed -i "s/#FFFFFF/#000000/" $svgFile
done
