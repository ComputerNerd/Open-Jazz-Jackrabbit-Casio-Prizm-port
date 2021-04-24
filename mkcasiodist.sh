#!/bin/sh
rm -f openjazz.7z
7z a -t7z -m0=lzma -mx=9 -ms=on openjazz.7z openjazz.g3a Changelog README
# Cemetech requires a zip file.
rm -f openjazz.zip
advzip -a -i 9001 -4 openjazz.zip openjazz.g3a Changelog README
