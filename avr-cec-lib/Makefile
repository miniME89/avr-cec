#====================================== Options =======================================
# Output library name
LIBOUTPUT = avrcec

# Build directory
BUILDDIR = bin

# Source directory
SRCDIR = src

# include directory
INCDIRS = inc

# Optimization level [0, 1, 2, 3, s]
OPT = s

# Debugging format [dwarf-2, stabs]
DEBUG = dwarf-2

ifeq ($(OS), Windows_NT)
	INCDIRS += inc/windows
endif

ifeq ($(OS), linux)
	INCDIRS += inc/linux
endif

#================================== Advanced options ==================================
# Tools
SHELL = sh
CPP = g++
AR = ar

# Compiler flags
CPPFLAGS = -g$(DEBUG)
CPPFLAGS += -O$(OPT)
CPPFLAGS += -funsigned-char
CPPFLAGS += -funsigned-bitfields
CPPFLAGS += -fpack-struct
CPPFLAGS += -fshort-enums
CPPFLAGS += -Wall
CPPFLAGS += $(patsubst %,-I%,$(INCDIRS))

# Archiver flags
ARFLAGS = rcs

# Sources, object files and library
SOURCES_C = $(wildcard $(SRCDIR)/*.cpp) $(wildcard $(SRCDIR)/*/*.cpp)
OBJECTS = $(SOURCES_C:%.cpp=$(BUILDDIR)/%.o)
LIB = lib$(LIBOUTPUT).a

#====================================== Targets =======================================
# Default
all: $(LIB)

# From C source to .o object file
$(OBJECTS): $(SOURCES_C)
	@mkdir -p $(@D)
	$(CPP) $(CPPFLAGS) -c $< -o $@

# Create library
$(LIB): $(OBJECTS)
	@echo
	@echo Create library output:
	$(AR) $(ARFLAGS) $(BUILDDIR)/$@ $^

# Clean
clean:
	$(RM) -r $(BUILDDIR)

# Listing of phony targets
.PHONY : all lib clean

# Create build directory
$(shell mkdir $(BUILDDIR) 2>/dev/null)
