#
# Makefile
#
BIN = homescr1
BINDIR = /usr/sbin/
#MAINSRC = homescr1.cpp

CC=gcc
CXX=g++
CFLAGS = -Wall -Wshadow -Wundef -Wmaybe-uninitialized
CFLAGS += -O3 -g3 -I./
LDFLAGS += -lstdc++ -lm

VPATH = 

LVGL_DIR =  ${shell pwd}
#INC=-I$(LVGL_DIR)

#LIBRARIES
include ./lvgl/lv_core/lv_core.mk
include ./lvgl/lv_hal/lv_hal.mk
include ./lvgl/lv_objx/lv_objx.mk
include ./lvgl/lv_misc/lv_fonts/lv_fonts.mk
include ./lvgl/lv_misc/lv_misc.mk
include ./lvgl/lv_themes/lv_themes.mk
include ./lvgl/lv_draw/lv_draw.mk

#DRIVERS
include ./lv_drivers/display/display.mk
include ./lv_drivers/indev/indev.mk

# folder for our object files
OBJDIR = ../obj

CSRCS += $(wildcard *.c)
CPPSRCS = $(wildcard *.cpp)

COBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(CSRCS))
CPPOBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(CPPSRCS))

#SRCS = $(CSRCS) $(MAINSRC)
SRCS = $(CSRCS) $(CPPSRCS)
OBJS = $(COBJS) $(CPPOBJS)

#.PHONY: clean

all: default

$(OBJDIR)/%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"
	
$(OBJDIR)/%.o: %.cpp
	@$(CXX)  $(CFLAGS) -c $< -o $@
	@echo "CXX $<"

#default: $(COBJS) $(MAINOBJ)
#	$(CC) -o $(BIN) $(MAINOBJ) $(COBJS) $(LDFLAGS)
	
default: $(OBJS)
	$(CC) -o $(BIN) $(OBJS) $(LDFLAGS)	

#	nothing to do but will print info
nothing:
#	$(info OBJS ="$(OBJS)")
	$(info DONE)	


clean:
	rm -f $(OBJS)

install:
	install -o root $(BIN) $(BINDIR)$(BIN)
	@echo ++++++++++++++++++++++++++++++++++++++++++++
	@echo ++ $(BIN) has been installed in $(BINDIR)
	@echo ++ systemctl start $(BIN)
	@echo ++ systemctl stop $(BIN)
