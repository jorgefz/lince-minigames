#include "lince.h"
#include "pong.h"


void SetupGame(){
    LinceApp* app = LinceGetAppState();

    app->screen_width = 900;
    app->screen_height = 600;
    app->title = "Pong";

    app->game_init      = PongInit;
    app->game_on_update = PongOnUpdate;
    app->game_on_event  = PongOnEvent;
    app->game_terminate = PongQuit;
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