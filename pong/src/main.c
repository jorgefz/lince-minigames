#include "lince.h"
#include "pong.h"


void SetupGame(){
    LinceApp* app = LinceGetApp();

    app->screen_width = 900;
    app->screen_height = 600;
    // app->title = "Pong";

    app->on_init      = PongInit;
    app->on_update    = PongOnUpdate;
    app->on_terminate = PongQuit;
}


int main(int argc, const char* argv[]) {

    #ifdef LINCE_DEBUG
    LINCE_INFO(" --- DEBUG MODE --- ");
    #endif

    SetupGame();
    LinceRun();

    LINCE_UNUSED(argc);
    LINCE_UNUSED(argv);
    return 0;
}