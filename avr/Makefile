# WinAVR cross-compiler toolchain is used here
SHELL = sh
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
DUDE = avrdude
NM = avr-nm

# MCU name
MCU = atmega8

# Processor frequency.
#     This will define a symbol, F_CPU, in all source code files
F_CPU = 16000000

# Target file name
TARGET = main

# Build directory
BUILDDIR = bin

# Source directory
SRCDIR = src

# Optimization level, can be [0, 1, 2, 3, s]. 
#     0 = turn off optimization. s = optimize for size.
OPT = s

# Debugging format.
#     Native formats for AVR-GCC's -g are dwarf-2 [default] or stabs.
#     AVR Studio 4.10 requires dwarf-2.
#     AVR [Extended] COFF format requires stabs, plus an avr-objcopy run.
DEBUG = dwarf-2

# Compiler flag to set the C Standard level.
#     c89   = "ANSI" C
#     gnu89 = c89 plus GCC extensions
#     c99   = ISO C99 standard (not yet fully implemented)
#     gnu99 = c99 plus GCC extensions
CSTANDARD = gnu99

# Compiler flags
CFLAGS = -g$(DEBUG)
CFLAGS += -DF_CPU=$(F_CPU)UL
CFLAGS += -O$(OPT)
CFLAGS += -funsigned-char
CFLAGS += -funsigned-bitfields
CFLAGS += -fpack-struct
CFLAGS += -fshort-enums
CFLAGS += -Wall
CFLAGS += -Wstrict-prototypes
CFLAGS += -Iinc -I. -Iusbdrv
CFLAGS += -mmcu=$(MCU)
CFLAGS += -std=$(CSTANDARD)
#CFLAGS += -MMD -MP -MF .dep/$(@F).d

# Library flags
LDFLAGS = -Wl,-Map=$(BUILDDIR)/$(TARGET).map,--cref
LDFLAGS += -lm

# Flash flags
DUDEFLAGS = -c stk500v2 -p $(MCU) -P com8

# Object files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:%.c=%.o)
OBJECTS += usbdrv/usbdrv.o usbdrv/oddebug.o usbdrv/usbdrvasm.o

# default
all: elf hex eep lss sym

elf: $(BUILDDIR)/$(TARGET).elf
hex: $(BUILDDIR)/$(TARGET).hex
eep: $(BUILDDIR)/$(TARGET).eep
lss: $(BUILDDIR)/$(TARGET).lss
sym: $(BUILDDIR)/$(TARGET).sym

# From C source to .o object file
%.o: %.c	
	$(CC) $(CFLAGS) -c $< -o $@

# From assembler source to .o object file
%.o: %.S
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

# Create elf output file from object files
%.elf: $(OBJECTS)
	@echo
	@echo Create elf output:
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

# Create final hex output file
%.hex: %.elf
	@echo
	@echo Create hex output:
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

# Create eep
%.eep: %.elf
	@echo
	@echo Create eep output:
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 --no-change-warnings -O ihex $< $@ || exit 0

# Create extended listing file from ELF output file.
%.lss: %.elf
	@echo
	@echo Create lss output:
	$(OBJDUMP) -h -S -z $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo
	@echo Create sym output:
	$(NM) -n $< > $@

# clean
clean:
	$(RM) $(BUILDDIR)/*.elf 
	$(RM) $(BUILDDIR)/*.hex
	$(RM) $(BUILDDIR)/*.eep
	$(RM) $(BUILDDIR)/*.lss
	$(RM) $(BUILDDIR)/*.sym
	$(RM) $(BUILDDIR)/*.map

# Flash
flash: $(TARGET).hex
	$(DUDE) $(DUDEFLAGS) -U flash:w:$<

# Create build directory
$(shell mkdir $(BUILDDIR) 2>/dev/null)
