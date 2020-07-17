# Build LVGL on PinePhone Ubuntu Touch

# Define $(CSRCS)
LVGL_DIR 	  := .
LVGL_DIR_NAME := .
include lvgl.mk

WAYLAND_CSRCS := \
	demo/lv_demo_widgets.c \
	wayland/util.c \
	wayland/texture.c

CC      := gcc

CCFLAGS := \
	-g \
	-I src/lv_core

LDFLAGS := \
    -lwayland-client \
    -lwayland-server \
    -lwayland-egl \
    -L/usr/lib/aarch64-linux-gnu/mesa-egl \
    -lEGL \
    /usr/lib/aarch64-linux-gnu/mesa-egl/libGLESv2.so.2 \
	-Wl,-Map=lvgl.map \

TARGETS:= wayland/lvgl
MAINS  := $(addsuffix .o, $(TARGETS) )
OBJ    := \
	$(CSRCS:.c=.o) \
	$(WAYLAND_CSRCS:.c=.o) \
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
