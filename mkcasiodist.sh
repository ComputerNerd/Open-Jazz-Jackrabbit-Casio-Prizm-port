#!/bin/sh
rm -f openjazz.7z
7z a -t7z -m0=lzma -mx=9 -ms=on openjazz.7z openjazz.g3a Changelog README jazz/openjazz.000

