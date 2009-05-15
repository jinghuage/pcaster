LIBDIR = /usr/local

#------------------------------------------------------------------------------

ifeq ($(shell uname), Darwin)
	HDFDIR = $(LIBDIR)/hdf5/1.6.5
	OPTS += -I$(HDFDIR)/include -I/sw/include -I$(LIBDIR)/include -I$(HOME)/include
	LIBS += -L$(HDFDIR)/lib  -L/sw/lib -L$(LIBDIR)/lib -L$(HOME)/lib

	OPTS += $(shell /sw/bin/sdl-config --cflags)
	LIBS += $(shell /sw/bin/sdl-config --libs) -lm -ljpeg -lpng -lhdf5 -framework OpenGL
	CC =  g++
else
	OPTS += -I$(HDFDIR)/include -I$(LIBDIR)/include -I$(HOME)/include
	LIBS += -L$(HDFDIR)/lib  -L$(LIBDIR)/lib -L$(HOME)/lib
	OPTS += $(shell sdl-config --cflags)
	LIBS += $(shell sdl-config --libs) -lm -ljpeg -lpng -lGLEW -lGL -lglut
	CC = g++

endif

#------------------------------------------------------------------------------

# INCPATH = -I/usr/local/hdf5/1.6.8/include -I/usr/local/include -I$(HOME)/include
# LIBPATH = -L/usr/local/hdf5/1.6.8/lib  -L/usr/local/lib -L$(HOME)/lib

# FRAMEWORK = -framework GLUT
# FRAMEWORK += -framework OpenGL 


COMPILERFLAGS += -Wall -g
CFLAGS = $(COMPILERFLAGS) #-DMPICH_IGNORE_CXX_SEEK -D__LOCAL_HDF5__ -DTANGIBLE -DSAGE 
#LIBRARIES = -ljpeg -lpng -lm -lGL -lglut -lGLEW -lhdf5 -lmpich -lmpichcxx -lpthread
#LIBRARIES = -ljpeg -lpng -lm  -lhdf5 -lmpich -lmpichcxx

OBJS = glheaders.o misc.o draw_routines.o Timer.o\
	shader_arb.o imgfile.o texture.o fbo.o buffer_object.o	
OBJS_PBO = read_rawfile.o pbo_sdl.o data_loader.o\
	 renderer.o

TARGET_PBO = render_pbo


all: $(TARGET_PBO)
pbo: $(TARGET_PBO)


.cpp.o:
	$(CC) $(OPTS) $(CFLAGS) $(INCPATH) -c $<


$(TARGET_PBO): $(OBJS) $(OBJS_PBO) 
	$(CC) $(OPTS) $(CFLAGS) -o $@ $(LIBS) $(OBJS) $(OBJS_PBO)


clean:
	rm *.o $(TARGET_PBO)
