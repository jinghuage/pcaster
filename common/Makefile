FRAMEWORK =
INCPATH = 
LIBPATH =

ifeq ($(shell uname), Darwin)
	LOCALDIR = /usr/local
	HDFDIR = $(LOCALDIR)/hdf5/1.8.1
	INCPATH += -I$(HDFDIR)/include -I/sw/include -I$(LOCALDIR)/include 
	LIBPATH += -L$(HDFDIR)/lib  -L/sw/lib -L$(LOCALDIR)/lib

else
	LOCALDIR = /usr/local
	HDFDIR = $(LOCALDIR)/hdf5-1.6.8
	INCPATH += -I$(HDFDIR)/include -I$(LOCALDIR)/include
	LIBPATH += -L$(HDFDIR)/lib  -L$(LOCALDIR)/lib

endif

COMPILERFLAGS = -Wall -g
CC = g++
CFLAGS = $(COMPILERFLAGS) 
LFLAGS = $(COMPILERFLAGS) 
LIBRARIES = -lm -lhdf5

OBJECTS = resampler_from_hdf.o #read_h5file.o #resampler_from_hdf.o
TARGET = resampler_from_hdf #readh5 #resample_from_hdf

all: $(TARGET)

.cpp.o:
	$(CC) $(CFLAGS) $(INCPATH) -c $<
		
$(TARGET): $(OBJECTS) 
	$(CC) $(FRAMEWORK) $(LFLAGS) -o $@ $(LIBPATH) $(OBJECTS) $(LIBRARIES)
  
clean:
	rm -rf *.o $(TARGET)   
