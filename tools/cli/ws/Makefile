#
# (c) 2001/2002 Jouni 'Mr.Spiv' Korhonen / Dead Coders Society
#
#

.PHONY: clean all dep dist bins cbins
.SUFFIXES:
.SUFFIXES: .cpp .o .h .s .c
.DEFAULT:
	make all
#

BMPS = $(wildcard *.bmp)
BINS = $(addsuffix .bin, $(basename $(wildcard *.bmp)))
BOBJS = $(addsuffix .o, $(basename $(wildcard *.bmp)))
CBINS = $(addsuffix .c, $(basename $(wildcard *.bmp)))

RAWS =  $(wildcard *.raw)
RAWOBJS = $(addsuffix .o, $(basename $(wildcard *.raw)))

SRCS = $(wildcard *.c) 
ASRCS = $(wildcard *.S)

HDRS =

OBJS = $(addsuffix .o, $(basename $(wildcard *.S))) $(addsuffix .o, $(basename $(wildcard *.c)))


#

PROG = WSLinkmasta

#
#

BIN_DIR =
INC_DIR = /opt/local/include/
#INC_DIR = /opt/local/include/libusb-1.0/
LIB_DIR = /opt/local/lib/
LOCAL_LIBDIR =
LOCAL_LIBS =

CC = cc
AS = cc
RM = rm -f
CP = cp
DEPEND = .dep
WILD = *
WRITER = /Volumes/Unlabeled
FOO = GP32 rules!
DEFINES =



LOCAL_CFLAGS = -I $(INC_DIR) -Wall -O2 -fpack-struct

LOCAL_LDFLAGS = -L $(LIB_DIR) -lusb-1.0
LOCAL_AFLAGS =

#
#
#
# rules

all: $(DEPEND) $(PROG)
	@echo $(FOO)


ifeq ($(DEPEND),$(wildcard $(DEPEND)))
include $(DEPEND)
endif

$(DEPEND): Makefile
	$(CC) $(LOCAL_CFLAGS) -MM $(SRCS) > $(DEPEND)
	@echo "Dependencies done"

%.o: %.cpp
	$(CC) $(LOCAL_CFLAGS) $(DEFINES) -c $< -o $@

%.o: %.c
	$(CC) $(LOCAL_CFLAGS) $(DEFINES) -c $< -o $@

%.o: %.S
	$(AS) $(LOCAL_AFLAGS) $< -o $@


$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(OBJS) $(LOCAL_LDFLAGS) $(LOCAL_LIBDIR) $(LOCAL_LIBS)
	strip $(PROG)


clean:
	-$(RM) $(WILD).map
	-$(RM) $(WILD).o
	-$(RM) $(WILD)~
	-$(RM) $(PROG).tgz
	-$(RM) $(DEPEND)
	-$(RM) $(PROG)

dist:
	tar zcvf $(PROG).tgz *.c *.h Makefile




