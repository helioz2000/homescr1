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
#for debug
#CFLAGS += -O0 -g3
# for production
CFLAGS += -O3
CFLAGS += -I./
CXXFLAGS = $(CFLAGS)

# directory for local libs
LDFLAGS = -L$(DESTDIR)$(PREFIX)/lib
LIBS += -lstdc++ -lm -lmosquitto

#LVGL_DIR =  ${shell pwd}
LVGL_DIR = lvgl
CFLAGS += -I$(LVGL_DIR)

#LIBRARIES
include $(LVGL_DIR)/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk
#include mcp9808/mcp9808.mk

# folder for our object files
OBJDIR = ./obj

CSRCS += $(wildcard *.c)
CPPSRCS += $(wildcard *.cpp)

COBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(CSRCS))
CPPOBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(CPPSRCS))

SRCS = $(CSRCS) $(CPPSRCS)
OBJS = $(COBJS) $(CPPOBJS)

# dependencies
#homescr1.o: homescr1.cpp topics.h mqtt.h hardware.h screen.h datatag.h mcp9808.h
#	$(CC) -c homescr1.cpp

#screen.o: screen.h topics.h datatag.h
#	$(CC) - screen.cpp

#.PHONY: clean


all: default

$(OBJDIR)/%.o: %.cpp
	@echo "CXX $<"
	@$(CXX)  $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.c
	@echo "CC $<"
	@$(CC)  $(CFLAGS) -c $< -o $@

default: $(OBJS)
	$(CC) -o $(BIN) $(OBJS) $(LDFLAGS) $(LIBS)

#	nothing to do but will print info
nothing:
	$(info OBJS ="$(OBJS)")
	$(info ----)
	$(info SRCS ="$(SRCS)")
	$(info ----)
	$(info CPPSRCS ="$(CPPSRCS)")
	$(info ----)
	$(info CFLAGS ="$(CFLAGS)")
	$(info ----)
	$(info CSXXFLAGS ="$(CXXFLAGS)")
	$(info DONE)

clean:
	rm -f $(OBJS)

install:
ifneq ($(shell id -u), 0)
	@echo "!!!! install requires root !!!!"
else
	install -o root $(BIN) $(BINDIR)$(BIN)
	@echo ++++++++++++++++++++++++++++++++++++++++++++
	@echo ++ $(BIN) has been installed in $(BINDIR)
	@echo ++ systemctl start $(BIN)
	@echo ++ systemctl stop $(BIN)
endif
