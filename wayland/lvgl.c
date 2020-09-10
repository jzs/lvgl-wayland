#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include "xdg-shell-protocol.h"
#include <EGL/egl.h>
#include "../lvgl.h"
#include "../demo/lv_demo_widgets.h"
#include "lv_port_disp.h"
#include "util.h"
//#include <GLES2/gl2.h>
/// Dimensions of the OpenGL region to be rendered
static int WIDTH  = LV_HOR_RES_MAX;
static int HEIGHT = LV_VER_RES_MAX;

struct app_state {
	struct wl_display    *display;
	struct wl_registry   *registry;
	struct wl_compositor *compositor;
	struct xdg_wm_base   *xdg_wm_base;

	struct wl_surface   *surface;
	struct xdg_surface  *xdg_surface;
	struct xdg_toplevel *xdg_toplevel;

	struct wl_egl_window *egl_window;

	/// Wayland EGL Interfaces for OpenGL Rendering
	EGLDisplay egl_display;  //  EGL Display
	EGLConfig  egl_conf;     //  EGL Configuration
	EGLSurface egl_surface;  //  EGL Surface
	EGLContext egl_context;  //  EGL Context
};

// lvgl stuff -----------------------------
/// Exported by texture.c
void Init(ESContext *esContext);
void Draw(ESContext *esContext);

/// Render the OpenGL ES2 display
static void render_display() {
    puts("Rendering display...");

    //  Init the LVGL display
    lv_init();
    lv_port_disp_init();

    //  Create the LVGL widgets
    //  render_widgets();  //  For button and label
    lv_demo_widgets();  //  For all kinds of demo widgets

    //  Render the LVGL widgets
    puts("Handle task...");
    lv_task_handler();

    //  Create the texture context
    static ESContext esContext;
    esInitContext ( &esContext );
    esContext.width = WIDTH;
    esContext.height = HEIGHT;

    //   Draw the texture
    Init(&esContext);
    Draw(&esContext);

    //  Render now
    glFlush();
}


// xdg_surface_listener
static void
xdg_surface_configure(void *data,
        struct xdg_surface *xdg_surface, uint32_t serial)
{
    struct app_state *state = data;
    xdg_surface_ack_configure(xdg_surface, serial);

	// Fix this...
    //struct wl_buffer *buffer = draw_frame(state);
    //wl_surface_attach(state->surface, buffer, 0, 0);
    //wl_surface_commit(state->surface);
}

const static struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

// xdg_wm_base_listener
static void
xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

const static struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

// Registry listener

static void
registry_handle_global(void *data, struct wl_registry *registry,
                uint32_t name, const char *interface, uint32_t version)
{
	printf("interface: '%s', version: %d, name: %d\n",
			interface, version, name);

	struct app_state *state = data;
	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		state->compositor = wl_registry_bind(
				state->registry, name, &wl_compositor_interface, 4);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        state->xdg_wm_base = wl_registry_bind(
                state->registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(state->xdg_wm_base,
                &xdg_wm_base_listener, state);
    }

}

static void
registry_handle_global_remove(void *data, struct wl_registry *registry,
		uint32_t name)
{
	// Left blank deliberately
}


static const struct wl_registry_listener
registry_listener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove,
};

// Local functions
void init_egl(struct app_state *state);
void create_window(struct app_state *state);

// Main

int
main(int argc, char *argv[])
{
	struct app_state state = { 0 };

	state.display = wl_display_connect(NULL);
	if (!state.display) {
		fprintf(stderr, "Failed to connect to Wayland display.\n");
		return 1;
	}

	fprintf(stderr, "Connection established");

	// Registry
	state.registry = wl_display_get_registry(state.display);
	wl_registry_add_listener(state.registry, &registry_listener, &state);
	wl_display_roundtrip(state.display);

	state.surface = wl_compositor_create_surface(state.compositor);
	state.xdg_surface = xdg_wm_base_get_xdg_surface(
			state.xdg_wm_base, state.surface);
	xdg_surface_add_listener(state.xdg_surface, &xdg_surface_listener, &state);
	state.xdg_toplevel = xdg_surface_get_toplevel(state.xdg_surface);
	xdg_toplevel_set_title(state.xdg_toplevel, "Hello world");
	wl_surface_commit(state.surface);

	wl_display_dispatch(state.display);

	init_egl(&state);

	create_window(&state);

	while (wl_display_dispatch(state.display)) {
		// something.
    	printf("loopmajor: ");
	}
	
	wl_display_disconnect(state.display);
	return 0;
}



//  Create the EGL Context for rendering OpenGL graphics
void init_egl(struct app_state *state) {
    puts("Init EGL...");

    //  Attributes for our EGL Display
    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
    static const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    //  Get the EGL Display
    state->egl_display = eglGetDisplay((EGLNativeDisplayType) state->display);
    assert(state->egl_display != EGL_NO_DISPLAY);  //  Failed to get EGL Display

    //  Init the EGL Display
    EGLint major, minor;
    EGLBoolean egl_init = eglInitialize(state->egl_display, &major, &minor);
    assert(egl_init);  //  Failed to init EGL Display
    printf("EGL major: %d, minor %d\n", major, minor);


    //  Get the EGL Configurations
    EGLint count, n;
    eglGetConfigs(state->egl_display, NULL, 0, &count);
    printf("EGL has %d configs\n", count);
    EGLConfig *configs = calloc(count, sizeof *configs);
    eglChooseConfig(state->egl_display, config_attribs,
        configs, count, &n);

    //  Choose the first EGL Configuration
    for (int i = 0; i < n; i++) {
        EGLint size;
        eglGetConfigAttrib(state->egl_display, configs[i], EGL_BUFFER_SIZE, &size);
        printf("Buffer size for config %d is %d\n", i, size);
        eglGetConfigAttrib(state->egl_display, configs[i], EGL_RED_SIZE, &size);
        printf("Red size for config %d is %d\n", i, size);
        state->egl_conf = configs[i];
        break;
    }
    assert(state->egl_conf != NULL);  //  Failed to get EGL Configuration

    //  Create the EGL Context based on the EGL Display and Configuration
    state->egl_context = eglCreateContext(state->egl_display, state->egl_conf,
        EGL_NO_CONTEXT, context_attribs);
    assert(state->egl_context != NULL);  //  Failed to create EGL Context
}

//  Create the EGL Window and render OpenGL graphics
void create_window(struct app_state *state) {
    //  Create an EGL Window from a Wayland Surface 
    state->egl_window = wl_egl_window_create(state->surface, WIDTH, HEIGHT);
    assert(state->egl_window != EGL_NO_SURFACE);  //  Failed to create OpenGL Window

    //  Create an OpenGL Window Surface for rendering
    state->egl_surface = eglCreateWindowSurface(state->egl_display, state->egl_conf,
        state->egl_window, NULL);
    assert(state->egl_surface != NULL);  //  Failed to create OpenGL Window Surface

    //  Set the current rendering surface
    EGLBoolean madeCurrent = eglMakeCurrent(state->egl_display, state->egl_surface,
        state->egl_surface, state->egl_context);
    assert(madeCurrent);  //  Failed to set rendering surface

    //  Render the display
    render_display();

    //  Swap the display buffers to make the display visible
    EGLBoolean swappedBuffers = eglSwapBuffers(state->egl_display, state->egl_surface);
    assert(swappedBuffers);  //  Failed to swap display buffers
}
