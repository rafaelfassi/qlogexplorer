#!/bin/bash

mkdir default
mkdir dark

for svgFile in svg/*.svg
do
	baseFileName="${svgFile##*/}"
	baseFileName="${baseFileName%%.*}"
	inkscape -z -w 32 -h 32 ${svgFile} -e default/${baseFileName}.png
	sed -i "s/#000000/#FFFFFF/" $svgFile
	inkscape -z -w 32 -h 32 ${svgFile} -e dark/${baseFileName}.png
	sed -i "s/#FFFFFF/#000000/" $svgFile
done
