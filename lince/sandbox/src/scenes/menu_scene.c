#include "menu_scene.h"
#include "gamedata.h"

void DrawMainMenu(LinceScene* scene){
    LINCE_UNUSED(scene);

    LinceUILayer* ui = LinceGetApp()->ui;
    struct nk_context *ctx = ui->ctx;
    LinceApp* app = LinceGetApp();

    const struct nk_style_button button_style = {
        /* background */
        .normal = nk_style_item_color((struct nk_color){0,0,0,255}),
        .hover  = nk_style_item_color((struct nk_color){50,50,50,255}),
        .active = nk_style_item_color((struct nk_color){18,44,102,255}),
        /* text */
        .text_background = {0,0,0,255},
        .text_normal     = {255,255,255,255},
        .text_hover      = {255,255,255,255},
        .text_active     = {255,255,255,255},
        .text_alignment  = NK_TEXT_CENTERED,
    };
    
    nk_style_set_font(ctx, &ui->fonts[LinceFont_Droid30]->handle);
    nk_style_push_color(
        ctx,
        &ctx->style.window.background,
        (struct nk_color){0,0,0,255}
    );

    nk_style_push_style_item(
        ctx, &ctx->style.window.fixed_background,
        nk_style_item_color((struct nk_color){0,0,0,255})
    );

    vec2 sc;
    LinceGetScreenSize(sc);
    vec2 btn_sz = {200, 120};
    vec2 btn_pos = {sc[0]/2 - btn_sz[0]/2, sc[1]/2 - btn_sz[1]/2};
    float row_h = 40.0;
    
    if (nk_begin(ctx, "MainMenu", nk_rect(btn_pos[0], btn_pos[1], btn_sz[0], btn_sz[1]), 0)) {
        nk_layout_row_dynamic(ctx, row_h, 1);
        if (nk_button_label_styled(ctx, &button_style, "Play")){
            LinceLoadScene("World");
        }
        nk_layout_row_dynamic(ctx, row_h, 1);
        if (nk_button_label_styled(ctx, &button_style, "Exit")){
            app->running = LinceFalse;
        }
    }
    nk_end(ctx);

    nk_style_pop_color(ctx);
    nk_style_pop_style_item(ctx);
}