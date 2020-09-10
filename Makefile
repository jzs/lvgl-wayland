# Build LVGL on PinePhone Ubuntu Touch

# Define $(CSRCS)
LVGL_DIR 	  := .
LVGL_DIR_NAME := .
include lvgl.mk

WAYLAND_CSRCS := \
	demo/lv_demo_widgets.c \
	wayland/lv_port_disp.c \
	wayland/shader.c \
	wayland/texture.c \
	wayland/util.c \
	wayland/xdg-shell-protocol.c

TARGETS:= wayland/lvgl

DEPS   := lv_conf.h

CC     := gcc

CCFLAGS := \
	-g \
	-I src/lv_core \
	-D LV_USE_DEMO_WIDGETS

LDFLAGS := \
    -Wl,-Map=wayland/lvgl.map \
    -L/usr/lib/aarch64-linux-gnu/mesa-egl \
    -lwayland-client \
    -lwayland-server \
    -lwayland-egl \
    -lEGL \
    -lGLESv2

MAINS  := $(addsuffix .o, $(TARGETS) )
OBJ    := \
	$(MAINS) \
	$(CSRCS:.c=.o) \
	$(WAYLAND_CSRCS:.c=.o)

.PHONY: all clean

all: gen-wayland $(TARGETS)

gen-wayland:
	wayland-scanner private-code /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml wayland/xdg-shell-protocol.c
	wayland-scanner client-header /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml wayland/xdg-shell-protocol.h

clean:
	rm -f $(TARGETS) $(OBJ)
	rm -f wayland/xdg-shell-protocol.c
	rm -f wayland/xdg-shell-protocol.h

$(OBJ): %.o : %.c $(DEPS)
	$(CC) -c -o $@ $< $(CCFLAGS)

$(TARGETS): % : $(filter-out $(MAINS), $(OBJ)) %.o
	$(CC) -o $@ \
	-Wl,--start-group \
	$(LIBS) \
	$^ \
	-Wl,--end-group \
	$(CCFLAGS) \
	$(LDFLAGS)
