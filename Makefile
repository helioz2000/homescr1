#
# Makefile
#
BIN = homescr1
BINDIR = /usr/sbin/
#MAINSRC = homescr1.cpp

CC=gcc
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

COBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(CSRCS))

#MAINOBJ = $(MAINSRC:.c=.o)

CSRCS += $(wildcard *.cpp)
CSRCS += $(wildcard *.c)

#SRCS = $(CSRCS) $(MAINSRC)
SRCS = $(CSRCS)
OBJS = $(COBJS)

#.PHONY: clean

all: default

$(OBJDIR)/%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"
	
#default: $(COBJS) $(MAINOBJ)
#	$(CC) -o $(BIN) $(MAINOBJ) $(COBJS) $(LDFLAGS)
	
default: $(COBJS)
	$(CC) -o $(BIN) $(COBJS) $(LDFLAGS)	

clean:
	rm -f $(COBJS)

install:
	install -o root $(BIN) $(BINDIR)$(BIN)
	@echo ++++++++++++++++++++++++++++++++++++++++++++
	@echo ++ $(BIN) has been installed in $(BINDIR)
	@echo ++ systemctl start $(BIN)
	@echo ++ systemctl stop $(BIN)