#-----------------------------------------------------------------------------
# Makefile for vizws07 and HighlandCreek, and jinghua-local
#-----------------------------------------------------------------------------
SRCS     := sdrender render viewer pcaster_onsite pcaster_remote

# this needs to be defined by user, where this project is installed
POFP_DIR ?= $(HOME)/pcaster/projs/parallel_ofp

ROOTDIR    ?= $(POFP_DIR)
BINDIR     ?= $(ROOTDIR)/bin
OBJDIR 	   ?= $(ROOTDIR)/obj
OBJDIR_COMM ?= $(OBJDIR)/common
LIBDIR     ?= $(ROOTDIR)/lib


%.ph_build :
	@make -C $*

%.ph_clear : 
	@make -C $* clear

%.ph_clean : 
	@make -C $* clean

%.ph_clobber :
	@make -C $* clobber

all:  makedirectories $(addsuffix .ph_build, $(SRCS))
	@echo "Finished building pcaster/projs/parallel_ofp"

makedirectories:
	$(VERBOSE)mkdir -p $(LIBDIR)
	$(VERBOSE)mkdir -p $(OBJDIR)
	$(VERBOSE)mkdir -p $(OBJDIR_COMM)
	$(VERBOSE)mkdir -p $(BINDIR)

tidy:
	@find | egrep "#" | xargs rm -f
	@find | egrep "\~" | xargs rm -f

clear: tidy $(addsuffix .ph_clear,$(SRCS))
	@echo "clear"


clean: tidy $(addsuffix .ph_clean,$(SRCS))
	@echo "clean"

clobber: clean $(addsuffix .ph_clobber,$(SRCS))
	@echo "clobber"
