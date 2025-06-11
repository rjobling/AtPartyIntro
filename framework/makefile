# to generate assembler listing with LTO, add to LDFLAGS: -Wa,-adhln=$@.listing,--listing-rhs-width=200
# for better annotations add -dA -dP
# to generate assembler source with LTO, add to LDFLAGS: -save-temps=cwd

OUTPATH = out/
OBJPATH = obj/

OUTELFFLAGS = -s

OUT = $(OUTPATH)$(OUTNAME)

ifdef OS
	WINDOWS = 1
	SHELL = cmd.exe
endif

subdirs := $(wildcard */ */*/)
VPATH = $(subdirs)
cpp_sources := $(wildcard *.cpp) $(wildcard $(addsuffix *.cpp,$(subdirs)))
cpp_objects := $(addprefix $(OBJPATH),$(patsubst %.cpp,%.o,$(notdir $(cpp_sources))))
c_sources := $(wildcard *.c) $(wildcard $(addsuffix *.c,$(subdirs)))
c_objects := $(addprefix $(OBJPATH),$(patsubst %.c,%.o,$(notdir $(c_sources))))
s_sources := $(subst out/$(OUTNAME).s,,$(wildcard *.s) $(wildcard $(addsuffix *.s,$(subdirs))))
s_objects := $(addprefix $(OBJPATH),$(patsubst %.s,%.o,$(notdir $(s_sources))))
vasm_sources := $(wildcard *.asm) $(wildcard $(addsuffix *.asm, $(subdirs)))
vasm_objects := $(addprefix $(OBJPATH), $(patsubst %.asm,%.o,$(notdir $(vasm_sources))))
objects := $(cpp_objects) $(c_objects) $(s_objects) $(vasm_objects)

# https://stackoverflow.com/questions/4036191/sources-from-subdirectories-in-makefile/4038459
# http://www.microhowto.info/howto/automatically_generate_makefile_dependencies.html

CC = m68k-amiga-elf-gcc
VASM = vasmm68k_mot

ifdef WINDOWS
	SDKDIR = $(abspath $(dir $(shell where $(CC)))..\m68k-amiga-elf\sys-include)
else
	SDKDIR = $(abspath $(dir $(shell which $(CC)))../m68k-amiga-elf/sys-include)
endif

CCFLAGS = -g -MP -MMD -m68000 -O1 -nostdlib	\
 -fdata-sections							\
 -ffunction-sections						\
 -flto										\
 -fno-exceptions							\
 -fomit-frame-pointer						\
 -fwhole-program							\
 -falign-functions							\
 -falign-jumps								\
 -falign-labels								\
 -falign-loops								\
 -fcaller-saves								\
 -fcode-hoisting							\
 -fcse-follow-jumps							\
 -fcse-skip-blocks							\
 -fdelete-null-pointer-checks				\
 -fdevirtualize								\
 -fdevirtualize-speculatively				\
 -fexpensive-optimizations					\
 -ffinite-loops								\
 -fgcse										\
 -fgcse-lm									\
 -finline-functions							\
 -finline-small-functions					\
 -findirect-inlining						\
 -fipa-bit-cp								\
 -fipa-cp									\
 -fipa-icf									\
 -fipa-ra									\
 -fipa-sra									\
 -fipa-vrp									\
 -fisolate-erroneous-paths-dereference		\
 -flra-remat								\
 -foptimize-sibling-calls					\
 -fpartial-inlining							\
 -fpeephole2								\
 -freorder-functions						\
 -frerun-cse-after-loop						\
 -fstore-merging							\
 -fstrict-aliasing							\
 -fthread-jumps								\
 -ftree-pre									\
 -ftree-switch-conversion					\
 -ftree-vrp									\
 -fgcse-after-reload						\
 -floop-unroll-and-jam						\
 -fpeel-loops								\
 -fpredictive-commoning						\
 -fsplit-loops								\
 -fsplit-paths								\
 -fallow-store-data-races					\
 -pedantic-errors							\
 -Wall										\
 -Wdangling-else							\
 -Werror									\
 -Wextra									\
 -Wlogical-op								\
 -Wmisleading-indentation					\
 -Wno-multichar								\
 -Wno-volatile-register-var					\
 -Wpedantic									\
 -Wshadow									\
 -Wstrict-overflow=2						\
 -Wtrampolines								\
 -Wunsafe-loop-optimizations				\
 $(OUTCCFLAGS)

CPPFLAGS = $(CCFLAGS) -std=c++20 -fno-rtti -fno-use-cxa-atexit
ASFLAGS = $(OUTASFLAGS) -Wa,-g,--register-prefix-optional,-I$(SDKDIR),-D
LDFLAGS = -Wl,--gc-sections,--emit-relocs,-Ttext=0,-Map=$(OUT).map
VASMFLAGS = $(OUTVASMFLAGS) -warncomm -m68000 -Felf -opt-fconst -nowarn=62 -dwarf=3 -quiet -x -I. -I$(SDKDIR)

all: $(OUT).exe

out:
	@mkdir out

obj:
	@mkdir obj

$(OUT).exe: $(OUT).elf | out
	$(info Elf2Hunk $(OUT).exe)
	@elf2hunk $(OUT).elf $(OUT).exe $(OUTELFFLAGS)

$(OUT).elf: $(objects) | out
	$(info Linking $(OUT).elf)
	@$(CC) $(CCFLAGS) $(LDFLAGS) $(objects) -o $@
	@m68k-amiga-elf-objdump --disassemble --no-show-raw-ins --visualize-jumps -S $@ >$(OUT).s

clean: out obj
	$(info Cleaning...)
ifdef WINDOWS
	@rmdir /q /s obj
	@rmdir /q /s out
else
	@$(RM) obj/* out/*
endif

-include $(objects:.o=.d)

$(cpp_objects): $(OBJPATH)%.o : %.cpp | obj
	$(info Compiling $<)
	@$(CC) $(CPPFLAGS) -c -o $@ $(CURDIR)/$<

$(c_objects): $(OBJPATH)%.o : %.c | obj
	$(info Compiling $<)
	@$(CC) $(CCFLAGS) -c -o $@ $(CURDIR)/$<

$(s_objects): $(OBJPATH)%.o : %.s | obj
	$(info Assembling $<)
	@$(CC) $(CCFLAGS) $(ASFLAGS) -c -o $@ $(CURDIR)/$<

$(vasm_objects): $(OBJPATH)%.o : %.asm | obj
	$(info Assembling $<)
	@$(VASM) $(VASMFLAGS) -dependall=make -depfile $(@D)/$*.d -o $@ $(CURDIR)/$<
