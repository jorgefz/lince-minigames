
#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "core/app.h"
#include "renderer/renderer.h"
#include "gui/ui_layer.h"
#include "core/input.h"
#include "core/profiler.h"

/* Private application state - stack allocated */
static LinceApp app = {0};

/* Calls the program's main loop */
void LinceRun();


/* --- Static functions --- */

/* Initialises OpenGL window and layer stacks */
static void LinceInit();

/* Called once per frame, updates window and renders layers */
static void LinceOnUpdate();

/* Shuts down application and frees allocated memory */
static void LinceTerminate();

/* Called when game event occurs,
propagates it to layers and user */
static void LinceOnEvent(LinceEvent* e);

/* Window event callbacks */
static LinceBool LinceOnEventWindowResize(LinceEvent* e);
static LinceBool LinceOnEventWindowClose(LinceEvent* e);


static char const* GLGetErrorString(GLenum const err) {
  switch (err) {
    // opengl 2 errors (8)
    case GL_NO_ERROR:
      return "GL_NO_ERROR";

    case GL_INVALID_ENUM:
      return "GL_INVALID_ENUM";

    case GL_INVALID_VALUE:
      return "GL_INVALID_VALUE";

    case GL_INVALID_OPERATION:
      return "GL_INVALID_OPERATION";

    case GL_STACK_OVERFLOW:
      return "GL_STACK_OVERFLOW";

    case GL_STACK_UNDERFLOW:
      return "GL_STACK_UNDERFLOW";

    case GL_OUT_OF_MEMORY:
      return "GL_OUT_OF_MEMORY";

    // opengl 3 errors (1)
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      return "GL_INVALID_FRAMEBUFFER_OPERATION";

    // gles 2, 3 and gl 4 error are handled by the switch above
    default:
      return "Unknown Error";
  }
}

void LinceCheckErrors(){
    //return;
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR) {
        LINCE_ASSERT(LinceFalse, "GLerror %d: %s", err, GLGetErrorString(err));
    }
}


void LinceRun(){

    LinceInit();
    
    LINCE_INFO(" Running main loop...");
    while(app.running){
        LinceOnUpdate();
    }
    LinceTerminate();
}


LinceApp* LinceGetAppState(){
    return &app;
}

void LincePushLayer(LinceLayer* layer) {
    LinceLayerStackPush(app.layer_stack, layer);
}

void LincePushOverlay(LinceLayer* overlay) {
    LinceLayerStackPush(app.overlay_stack, overlay);
}

double LinceGetTimeMillis(){
    return (glfwGetTime() * 1000.0);
}

float LinceGetAspectRatio(){
    return (float)app.window->width / (float)app.window->height;
}

void LinceGetScreenSize(vec2 size){
    size[0] = (float)LinceGetAppState()->window->width;
    size[1] = (float)LinceGetAppState()->window->height;
}

void LinceTransformToWorld(vec2 screen_coords, vec2 screen_size, mat4 vp_inv){
	float sx = screen_coords[0], sy = screen_coords[1];
	const float w = screen_size[0];
	const float h = screen_size[1];

	// normalise screen coordinates to range (-1,1)
	sx = 2.0f*sx/w - 1.0f;
	sy = 1.0f - 2.0f*sy/h;
	vec4 svec = {sx, sy, 0.0f, 1.0f};
	vec4 wvec;
    glm_mat4_mulv(vp_inv, svec, wvec);
	screen_coords[0] = wvec[0] / wvec[3];
	screen_coords[1] = wvec[1] / wvec[3];
}

void LinceGetMousePosWorld(vec2 pos, LinceCamera* cam){
    vec2 screen_size;
    LinceGetMousePos(&pos[0], &pos[1]);
    LinceGetScreenSize(screen_size);
    LinceTransformToWorld(pos, screen_size, cam->view_proj_inv);
}

LinceLayer* LinceGetCurrentLayer(){
    if (app.current_layer < 0) return NULL;
    return app.layer_stack->layers[app.current_layer];
}

LinceLayer* LinceGetCurrentOverlay(){
    if (app.current_overlay < 0) return NULL;
    return app.overlay_stack->layers[app.current_overlay];
}

/* --- Implementations of static functions --- */

static void LinceInit(){
    // Open log file
    LinceOpenLogger(LINCE_DIR"log.txt");

    //Report Lince version and debug/release configuration
    LINCE_INFO("--- LINCE version "LINCE_VERSION" ---");
#ifdef LINCE_DEBUG
    LINCE_INFO("Debug configuration");
#else
    LINCE_INFO("Release configuration");
#endif

    // Check user settings and set defaults
    if (app.screen_width == 0) app.screen_width = 500;
    if (app.screen_height == 0) app.screen_height = 500;
    if (app.title == NULL) app.title = "Lince Window";
    if (app.enable_profiling && app.profiler_filename){
        app.profiler_file = fopen(app.profiler_filename, "w");
        LinceSetProfiler(app.profiler_file);
        if(!app.profiler_file){
            LINCE_INFO("Error: unable to open profiling file '%s'\n",
                app.profiler_filename);
        }
    }
    
    // Create a windowed mode window and its OpenGL context
    app.window = LinceCreateWindow(app.screen_width, app.screen_height, app.title);
    LinceSetMainEventCallback(app.window, LinceOnEvent);

    // init layer and overlay stacks
    app.layer_stack = LinceCreateLayerStack();
    app.overlay_stack = LinceCreateLayerStack();
    
    LinceInitRenderer(app.window);
    app.ui = LinceInitUI(app.window->handle);
    app.running = LinceTrue;

    if (app.game_init) app.game_init(); // user may push layers onto stack
}


static void LinceOnUpdate(){
    LINCE_PROFILER_START(timer);
    LinceClear();

    // Calculate delta time
    float new_time_ms = (float)(glfwGetTime() * 1000.0);
    app.dt = new_time_ms - app.time_ms;
    app.time_ms = new_time_ms;
    app.screen_width = app.window->width;
    app.screen_height = app.window->height;

    LinceBeginUIRender(app.ui);

    // update layers
    unsigned int i;
    for (i = 0; i != app.layer_stack->count; ++i) {
        LinceLayer* layer = app.layer_stack->layers[i];
        app.current_layer = i;
        if (layer && layer->OnUpdate) layer->OnUpdate(layer, app.dt);
    }
    app.current_layer = -1;

    // update overlays
    for (i = 0; i != app.overlay_stack->count; ++i) {
        LinceLayer* overlay = app.overlay_stack->layers[i];
        app.current_overlay = i;
        if (overlay && overlay->OnUpdate) overlay->OnUpdate(overlay, app.dt);
    }
    app.current_overlay = -1;

    // update user application
    if (app.game_on_update) app.game_on_update(app.dt);

    LinceEndUIRender(app.ui);
    LinceUpdateWindow(app.window);
    LINCE_PROFILER_END(timer);
}

static void LinceTerminate(){

    LinceTerminateRenderer();
    
    // free layer and overlay stacks
    LinceDestroyLayerStack(app.layer_stack);
    LinceDestroyLayerStack(app.overlay_stack);
    app.layer_stack = NULL;
    app.overlay_stack = NULL;
    
    if (app.game_terminate) app.game_terminate();

    LinceTerminateUI(app.ui);

    /* shutdown window last, as it destroys opengl context
    and all its functions */
    LinceDestroyWindow(app.window);
    app.window = NULL;
    app.running = 0;
    
    LinceSetProfiler(NULL);

    // Close log file
    LinceCloseLogger();
}

static void LinceOnEvent(LinceEvent* e){
    /* Pre-defined event responses:
    adapt viewport when window is resized,
    and shutdown program when window is closed */
    LinceDispatchEvent(
        e,
        LinceEventType_WindowResize,
        LinceOnEventWindowResize
    );
    LinceDispatchEvent(e,
        LinceEventType_WindowClose,
        LinceOnEventWindowClose
    );

    LinceUIOnEvent(app.ui, e);

    /* propagate event to layers and overlays,
    the ones in front (rendered last) receive it first */
    int i;
    for (i = (int)app.overlay_stack->count - 1; i >= 0; --i) {
        if (e->handled) break;
        LinceLayer* overlay = app.overlay_stack->layers[i];
        app.current_overlay = i;
        if (overlay && overlay->OnEvent) overlay->OnEvent(overlay, e);
    }
    app.current_overlay = -1;

    for (i = (int)app.layer_stack->count - 1; i >= 0; --i) {
        if (e->handled) break;
        LinceLayer* layer = app.layer_stack->layers[i];
        app.current_layer = i;
        if (layer && layer->OnEvent) layer->OnEvent(layer, e);
    }
    app.current_layer = -1;

    // propagate event to user
    if (app.game_on_event && !e->handled ) app.game_on_event(e);
}


static LinceBool LinceOnEventWindowResize(LinceEvent* e){
    LINCE_INFO(" Window resized to %d x %d", 
        (int)e->data.WindowResize->width,
        (int)e->data.WindowResize->height
    );
    return LinceFalse; // allow other layers to receive event
}

static LinceBool LinceOnEventWindowClose(LinceEvent* e) {
    app.running = LinceFalse;
    return LinceFalse; // allow other layers to receive event
    LINCE_UNUSED(e);
}



