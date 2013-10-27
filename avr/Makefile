#====================================== Options =======================================
# MCU name
MCU = atmega8

# Processor frequency
F_CPU = 16000000

# Target file name
TARGET = main

# Build directory
BUILDDIR = bin

# Source directory
SRCDIR = src

# include directory
INCDIRS = inc inc/usbdrv

# Optimization level [0, 1, 2, 3, s]
OPT = s

# Debugging format [dwarf-2, stabs]
DEBUG = dwarf-2

# Compiler flag to set the C Standard level [c89, gnu89, c99, gnu99]
CSTANDARD = gnu99

#================================== Advanced options ==================================
# Tools
SHELL = sh
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
DUDE = avrdude
NM = avr-nm
SIZE = avr-size

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
CFLAGS += $(patsubst %,-I%,$(INCDIRS))
CFLAGS += -mmcu=$(MCU)
CFLAGS += -std=$(CSTANDARD)
CFLAGS += -MMD -MP -MF $(BUILDDIR)/.dep/$(@F).d

# Library flags
LDFLAGS = -Wl,-Map=$(BUILDDIR)/$(TARGET).map,--cref
LDFLAGS += -lm

# Flash flags
DUDEFLAGS = -c stk500v2 -p $(MCU) -P com8

# Sources and object files
SOURCES_C = $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/*/*.c)
SOURCES_ASM = $(wildcard $(SRCDIR)/*.asm) $(wildcard $(SRCDIR)/*/*.asm)

OBJECTS = $(SOURCES_C:%.c=$(BUILDDIR)/%.o) $(SOURCES_ASM:%.asm=$(BUILDDIR)/%.o)

#====================================== Targets =======================================
# Default
all: elf hex eep lss sym size

elf: $(BUILDDIR)/$(TARGET).elf

hex: $(BUILDDIR)/$(TARGET).hex

eep: $(BUILDDIR)/$(TARGET).eep

lss: $(BUILDDIR)/$(TARGET).lss

sym: $(BUILDDIR)/$(TARGET).sym

# From C source to .o object file
$(BUILDDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# From assembler source to .o object file
$(BUILDDIR)/%.o: %.S
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

# Create elf output file from object files
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(OBJECTS)
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
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 --no-change-warnings -O ihex $< $@ || exit 0

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

size:
	@if test -f $(BUILDDIR)/$(TARGET).elf; then echo; $(SIZE) --mcu=$(MCU) --format=avr $(BUILDDIR)/$(TARGET).elf; 2>/dev/null; echo; fi

# Clean
clean:
	$(RM) -r $(BUILDDIR)

# Flash
flash: $(BUILDDIR)/$(TARGET).hex
	$(DUDE) $(DUDEFLAGS) -U flash:w:$<

# Listing of phony targets
.PHONY : all build elf hex eep lss sym clean

# Create build directory
$(shell mkdir $(BUILDDIR) 2>/dev/null)

# Include the dependency files
-include $(shell mkdir $(BUILDDIR)/.dep 2>/dev/null) $(wildcard $(BUILDDIR)/.dep/*)
