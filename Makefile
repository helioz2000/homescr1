#
# Makefile
#
BIN = homescr1
BINDIR = /usr/sbin/
DESTDIR = /usr
PREFIX = /local

CC=gcc
CXX=g++
CFLAGS = -Wall -Wshadow -Wundef -Wmaybe-uninitialized
CFLAGS += -O3 -g3 -I./

# directory for local libs
LDFLAGS = -L$(DESTDIR)$(PREFIX)/lib
LIBS += -lstdc++ -lm -lmosquitto

VPATH =

$(info LDFLAGS ="$(LDFLAGS)")

#LVGL_DIR =  ${shell pwd}
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
CPPSRCS += $(wildcard *.cpp)
CPPSRCS += mcp9808/mcp9808.cpp

COBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(CSRCS))
//COBJS = $(patsubst %.c,%.o,$(CSRCS))
//CPPOBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(CPPSRCS))
CPPOBJS = $(patsubst %.cpp,%.o,$(CPPSRCS))

SRCS = $(CSRCS) $(CPPSRCS)
OBJS = $(COBJS) $(CPPOBJS)

#.PHONY: clean

all: default

$(OBJDIR)/%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"

#$(OBJDIR)/%.o: %.cpp
%.o: %.cpp
	@$(CXX)  $(CFLAGS) -c $< -o $@
	@echo "CXX $<"

default: $(OBJS)
	$(CC) -o $(BIN) $(OBJS) $(LDFLAGS) $(LIBS)

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
