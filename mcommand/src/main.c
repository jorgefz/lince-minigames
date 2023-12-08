#include "lince.h"
#include "missile_command.h"



int main(int argc, const char* argv[]) {

    #ifdef LINCE_DEBUG
    LINCE_INFO(" --- DEBUG MODE --- ");
    #endif

    LinceApp* app = LinceGetApp();

    app->screen_width = 900;
    app->screen_height = 600;
    app->title = "Sandbox";

    app->on_init = MCommandInit;
    app->on_update = MCommandUpdate;
    app->on_event = MCommandOnEvent;
    app->on_terminate = MCommandTerminate;
    
    LinceRun();

    LINCE_UNUSED(argc);
    LINCE_UNUSED(argv);
    return 0;
}