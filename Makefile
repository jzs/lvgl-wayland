# Build LVGL on PinePhone Ubuntu Touch

LVGL_DIR 	  := .
LVGL_DIR_NAME := .
include lvgl.mk

CC      := gcc
CCFLAGS := -I src/lv_core
LDFLAGS :=

TARGETS:= lvgl
MAINS  := $(addsuffix .o, $(TARGETS) )
OBJ    := \
	$(CSRCS:.c=.o) \
	$(MAINS)
DEPS   := lvgl.h

.PHONY: all clean

all: $(TARGETS)

clean:
	rm -f $(TARGETS) $(OBJ)

$(OBJ): %.o : %.c $(DEPS)
	$(CC) -c -o $@ $< $(CCFLAGS)

$(TARGETS): % : $(filter-out $(MAINS), $(OBJ)) %.o
	$(CC) -o $@ $(LIBS) $^ $(CCFLAGS) $(LDFLAGS)
