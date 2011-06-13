echo EXPORTS > qtpaint.def
nm -g --defined-only CMakeFiles/qtpaint.dir/*.obj > tmp
sed -n '/^........ [T|C|B] _/s/^........ [T|C|B] _/ /p' tmp >> qtpaint.def
rm tmp

