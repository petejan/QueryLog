SHELL = /bin/sh

LIBS = ../lib/NIDAQmx.lib
LIBFLAGS = $(LIBS)
TARGET = ContAcq-IntClk
CC = gcc

OBJS = $(TARGET).o

CDEBUG = -g
LDFLAGS += -g

CFLAGS += $(CDEBUG) -I . -I ../inc

all: $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) core

$(TARGET) : $(OBJS)
	echo From $(OBJS) Target : $(TARGET)
	$(CC) $(LDFLAGS) -o $@.exe $(OBJS) $(LIBFLAGS)
