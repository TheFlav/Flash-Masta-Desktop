################################# CONFIGURATION ################################

TARGETS = tests
COMPILER = gcc
FLAGS = -Wall -DOS_MACOSX
CFLAGS =
CPPFLAGS = -std=c++11
LIBRARIES = -L/usr/local/lib -lusb-1.0 -framework IOKit -framework CoreFoundation

BUILDDIR = build
SRCDIR = src
TESTDIR = $(SRCDIR)/test

INCLUDEDIR = /usr/local/include src



# Prerequisites for target "test"
TESTS_PREREQ := test/main.o\
                test/libusb_usb_device_tester.o\
                usb_device/usb_device.o\
                usb_device/libusb_usb_device.o



############################ VARIABLE PREPROCESSING ############################

FLAGS += $(addprefix -I, $(INCLUDEDIR))

TESTS_PREREQ  := $(addprefix $(BUILDDIR)/, $(TESTS_PREREQ))



################################### TARGETS ####################################

.PHONEY: all
all: $(TARGETS)



## TESTS ################################

tests: $(TESTS_PREREQ)
	$(COMPILER) $(FLAGS) $(LIBRARIES) -o $(BUILDDIR)/$@ $^
	$(BUILDDIR)/$@



## CLEAN ################################

.PHONEY: clean
clean:
	@rm -f $(addprefix $(BUILDDIR)/, $(TARGETS)) $(BUILDDIR)/*.o



## OBJECT FILES #########################

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(dir $@)
	$(COMPILER) $(CFLAGS) $(FLAGS) $(LIBRARIES) -c -o $@ $<

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(dir $@)
	$(COMPILER) $(CPPFLAGS) $(FLAGS) $(LIBRARIES) -c -o $@ $<
