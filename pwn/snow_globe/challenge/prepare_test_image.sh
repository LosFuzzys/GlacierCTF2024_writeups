#!/bin/sh

SIZE=512

convert "$1" -resize ${SIZE}x${SIZE} -background black -gravity center -extent ${SIZE}x${SIZE} test.rgb
