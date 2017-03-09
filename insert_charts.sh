#!/bin/sh
# page 8 in A is empty page
pdftk A=$1 B=$2 cat A1-7 B A9-end output final_writing.pdf
