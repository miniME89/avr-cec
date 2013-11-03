# Build directory
BUILDDIR = ../../bin

# Default
all: example

# Create example program
example:
	@echo
	@echo Create example program output:
	g++ -I../../inc -o $(BUILDDIR)/example_cpp example.cpp -L$(BUILDDIR) -lavrcec -lusb -lpthread

# Clean
clean:
	$(RM) -r $(BUILDDIR)

# Listing of phony targets
.PHONY : all example clean

