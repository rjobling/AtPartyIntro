cd data
../framework/tools/linux/LSPConvert statetrue.mod -shrink -getpos -setpos
../framework/tools/linux/abc2 image.png -bpc 2 -uninterleaved -b image_bpls.bin
../framework/tools/linux/abc2 palettes.png -rgb -b palettes.bin
cd ..

make clean
make
