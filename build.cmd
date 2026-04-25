@echo off
setlocal
set PATH=%PATH%;%USERPROFILE%\.vscode\extensions\bartmanabyss.amiga-debug-1.7.7\bin\win32
set PATH=%PATH%;%USERPROFILE%\.vscode\extensions\bartmanabyss.amiga-debug-1.7.7\bin\win32\opt\bin
@echo on

cd data
..\framework\tools\LSPConvert cosmic_makeup.mod -shrink -getpos -setpos
..\framework\tools\abc2 font.png -bpc 2 -b font_bpls.bin
..\framework\tools\abc2 logo.png -bpc 4 -uninterleaved -b logo_bpls.bin -p logo_pal.bin
..\framework\tools\abc2 image.png -bpc 2 -uninterleaved -b image_bpls.bin
..\framework\tools\abc2 palettes.png -rgb -b palettes.bin
cd ..

gnumake clean
gnumake
