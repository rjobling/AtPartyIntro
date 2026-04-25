cd data
../framework/tools/linux/LSPConvert cosmic_makeup.mod -shrink -getpos -setpos
../framework/tools/linux/abc2 font.png -bpc 2 -b font_bpls.bin
../framework/tools/abc2 logo.png -bpc 4 -uninterleaved -b logo_bpls.bin -p logo_pal.bin
../framework/tools/linux/abc2 image.png -bpc 2 -uninterleaved -b image_bpls.bin
../framework/tools/linux/abc2 palettes.png -rgb -b palettes.bin
../framework/tools/abc2 bobs.png -bpc 2 -uninterleaved -b bobs_bpls.bin -p bobs_pal.bin
../framework/tools/abc2 masks.png -bpc 1 -uninterleaved -b masks_bpls.bin
cd ..

make clean
make
