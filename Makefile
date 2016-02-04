# query Makefile

CPP=g++
CFLAGS=-g -c
LDFLAGS=-g

# comment these out to disable
#GPIB=1
#NIDAQ=1

# detect for windows, MingW and GNU Windows make are different
ifdef WINDIR 
 WINDOWS=1
endif
ifdef windir
  WINDOWS=1
endif

WINDOWS=1

default: bin/query

all: default

objs = obj/main.o obj/common.o obj/timeStamp.o obj/nmea.o obj/serialPort.o obj/commandPort.o obj/IpPort.o obj/port.o obj/query.o

ifdef GPIB
 objs += obj/gpibPort.o
 CFLAGS += -DENABLE_GPIB
 ifdef WINDOWS
  LDFLAGS += lib/gpib-32.obj
 else
  LDFLAGS += lgpib
 endif
endif
ifdef NIDAQ
 objs += obj/NIDAQPort.o
 CFLAGS += -DENABLE_NIDAQ
 ifdef WINDOWS
  LDFLAGS += lib/NIDAQmx.lib
 endif
endif

ifdef WINDOWS
 LDFLAGS += -lwsock32 -Wl,--enable-auto-import -static-libgcc -static -L lib
endif

bin/query: obj/main.o $(objs)
	$(CPP) -o bin/query $(objs) $(LDFLAGS) 

$(objs): obj/%.o: src/%.cpp
	$(CPP) $(CFLAGS) $< -o $@ -I inc

clean:
	-rm -rf obj/*.o
	-rm -rf bin/query
	-rm -rf bin/query.exe

bin/QueryRead: src/QueryRead.cpp
	$(CPP) -o $@ src/QueryRead.cpp

bin/stats: src/stats.c
	$(CPP) -o $@ src/stats.c


tar: 
	tar cvzf HwQuery.tar.gz -C .. HwQuery --exclude HwQuery.tar.gz
