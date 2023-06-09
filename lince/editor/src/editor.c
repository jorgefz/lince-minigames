#include "editor.h"


#include <time.h>

#define LINCE_PROFILE

#include <lince.h>
#include "lince/core/profiler.h"

#include "cglm/types.h"
#include "cglm/vec4.h"
#include "cglm/affine.h"

#include "nuklear_flags.h"
#include "nuklear.h"

#include "nk_test.h"

#include "collider.h"


enum { BLOCK_FREE = 0, BLOCK_SOLID = 1 };

enum { BASE_MAP, LOGIC_MAP, BKG_MAP };
int map_choice = BASE_MAP;

LinceBool LinceCollision2D(vec2 xy1, vec2 size1, vec2 xy2, vec2 size2){
    return (
        xy1[0] + size1[0]/2.0f > xy2[0] - size2[0]/2.0f &&
        xy1[0] - size1[0]/2.0f < xy2[0] + size2[0]/2.0f &&
        xy1[1] + size1[1]/2.0f > xy2[1] - size2[1]/2.0f &&
        xy1[1] - size1[1]/2.0f < xy2[1] + size2[1]/2.0f
    );
}

void PrintDebugTiledata(LinceTilemap* tm){
    printf("---------------------------\n");
    for(size_t j = 0; j != tm->height; ++j){
        for(size_t i = 0; i != tm->width; ++i){
            printf("%3d, ", tm->base_grid[j*tm->width + i]);
        }
        printf("\n");
    }
    printf("---------------------------\n");
}

void PrintDebugSolidData(LinceTilemap* tm){
    printf("---------------------------\n");
    for(size_t j = 0; j != tm->height; ++j){
        for(size_t i = 0; i != tm->width; ++i){
            printf("%d, ", tm->logic_grid[j*tm->width + i]);
        }
        printf("\n");
    }
    printf("---------------------------\n");
}

void PrintDebugBkgData(LinceTilemap* tm){
    printf("---------------------------\n");
    for(size_t j = 0; j != tm->height; ++j){
        for(size_t i = 0; i != tm->width; ++i){
            printf("%3d, ", tm->bkg_grid[j*tm->width + i]);
        }
        printf("\n");
    }
    printf("---------------------------\n");
}

/// NOTE: provide x,y size (in tiles) rather than total length
LinceTile* LoadTilesFromTexture(const char* texture_file, size_t* length, float px){
    LinceTexture* tex = LinceCreateTexture("tileset", texture_file);
    size_t xtiles = tex->width / (uint32_t)px;
    size_t ytiles = tex->height / (uint32_t)px;
    LinceTile tile;

    LinceTile* tiles = LinceMalloc(sizeof(LinceTile)*xtiles*ytiles);

    for(size_t y = 0; y != ytiles; ++y){
        for(size_t x = 0; x != xtiles; ++x){
            tile = LinceGetTile(tex, (vec2){x,y}, (vec2){px,px}, (vec2){1,1});
            memmove(tiles + y*xtiles + x, &tile, sizeof(LinceTile));
        }
    }

    *length = xtiles * ytiles;
    return tiles;
}

const uint32_t tm_width = 16, tm_height = 20;
uint32_t tilemap_data[] = {
105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 
112, 112, 110, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 
122, 122, 104, 105, 105, 105, 105, 105, 105, 105, 111, 112, 112, 112,  95, 105, 
122, 122, 104, 105, 105, 105, 105, 105, 105, 111,  99, 122, 123, 122, 116, 112, 
122, 122, 116, 112, 112, 118, 110, 105, 105, 106, 122, 122, 122, 122, 122, 124, 
122, 122, 122, 122, 122, 122, 104, 105, 105, 107, 102, 122, 122, 122, 122, 124, 
124, 122, 122, 122, 122, 122, 104, 105, 105, 105, 106, 122, 122, 122, 122, 124, 
124, 122, 122, 122, 122, 122, 114, 105, 105, 105, 107, 102, 122, 122, 122, 124, 
100, 100, 100, 100, 100, 100, 113, 105, 105, 105, 111,  99, 122, 122, 122, 124, 
105, 105, 105, 105, 105, 105, 105, 105, 105,  94,  99, 122, 122, 122, 122, 122, 
105, 105, 105, 105, 105, 105, 105, 105, 105, 106, 122, 122, 122, 122, 122, 122, 
105, 105, 105, 105, 105, 105, 105, 105, 105, 106, 122, 122, 122, 122, 122, 122, 
105, 105, 105, 105, 105, 105, 105, 105, 105, 106, 122, 122, 122, 122, 122, 122, 
105, 105, 105, 105, 105, 105, 105, 105, 105, 106, 122, 122, 123, 122, 122, 122, 
105, 105, 105, 105, 105, 105, 105, 105, 105, 106, 122, 122, 122, 122, 122, 124, 
105, 105, 105, 105, 105, 105, 105, 105, 105, 107, 102, 122, 122, 122, 122, 124, 
105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 106, 122, 122, 122, 122, 124, 
105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 107, 102, 122, 122, 122, 124, 
105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 111,  99, 122, 122, 122, 124, 
105, 105, 105, 105, 105, 105, 105, 105, 105,  94,  99, 122, 122, 122, 122, 122, 
};

uint32_t tilemap_bkg[] = {
 12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12, 
 12,  12,  12,  12,  12,  12,  12,  12, 128,  12,  12,  12,  12,  12,  12,  12, 
 12,  12,  12,  12,  12,  12, 128,  12,  12,  12,  12,  12,  12,  12,  12,  12, 
 12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12, 
 12,  12,  12,  12,  12,  12,  12, 127,  12,  12,  30,  31,  32,  33,  34,  12, 
 12,   6,   6,   6,   6,  12,  12,  12,  12,  12,  43,  44,  45,  46,  47,  12, 
 12,   6,   6,   6,   6,  12,  12,  12,  12,  12,  56,  57,  58,  59,  60,  12, 
 12,   6,   6,   6,   6,  12,  12,  12,  12,  12,  69,  70,  71,  72,  73,  12, 
 12,  12,  12,  12,  12,  12,  12,  12,  12, 129, 129,  83,  84,  85,  12,  12, 
 12,  12,  12,  12,  12,  12,  12,  12,  12, 129, 129,  12,  12,  12,  12,  12, 
 12,  12,  12,  12,  12,  12,  12,  12, 109, 129, 129, 129, 129, 129, 129, 129, 
 12,  12, 128,  12,  12, 128,  12, 109, 109, 129, 129,  10,  10,  10,  10, 129, 
 12,  12, 128, 109,  12, 128, 105,  12,  12, 129, 129,  10,  10,  10,  10, 129, 
 12,  17, 128,  12,  12, 128,  12,  12,  12, 129, 129,  10,  10,  10,  10, 129, 
 12,  17,  26,  27,  28,  29,  12, 105, 109, 129, 129,  10,  10,  10,  10, 129, 
 12, 129,  39,  40,  41,  42,  12,  12,  12,  12, 129, 129, 129, 129, 129, 129, 
 12, 129,  52,  53,  54,  55,  12,  12,  12,  12, 129, 129,  22,  22,  22, 129, 
 12, 129,  65,  66,  67,  68,  12,  12,  12,  12, 129, 129,  22,  22,  22, 129, 
 12,  12,  12,  79,  80,  81,  12,  12,  12,  12, 129, 129,  22,  22,  22, 129, 
 12,  12,  12,  12,  12,  12,  12,  12,  12,  12, 129, 129, 129, 129, 129, 129,
};

uint8_t tilemap_solid[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
    1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
    1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 
    1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
    1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
    1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};


void DrawCollisionTiles(LinceTilemap* tm){
    if(!tm || !tm->logic_grid) return;

    LinceDrawTilemap(tm);

    for(size_t j = 0; j != tm->height; ++j){
        for(size_t i = 0; i != tm->width; ++i){
            size_t idx = j*tm->width + i;
            if(tm->logic_grid[idx] != BLOCK_SOLID){
                continue;
            }
            LinceQuadProps quad = {
                .x = (float)i - tm->offset[0],
                .y = (float)j - tm->offset[1],
                .w = 1.001f, .h = 1.001f,
                .zorder = 0.9,
                .color = {1,0,0,0.3},
            };
            LinceDrawQuad(quad);
        }
    }
}



int GetTilemapIndexAtPos(LinceTilemap* tm, vec2 pos, vec2 xy_ind){
    int tm_x = floorf(pos[0] + tm->offset[0] + 0.5f);
    int tm_y = floorf(pos[1] + tm->offset[1] + 0.5f);

    xy_ind[0] = (float)tm_x;
    xy_ind[1] = (float)tm_y;

    if(tm_x >= 0 && tm_x < (int)tm->width &&
       tm_y >= 0 && tm_y < (int)tm->height) {
        return (int)(tm_y * tm->width + tm_x);
    }
    return -1;
}


int GetTilemapIndexAtMouse(LinceTilemap* tm, LinceCamera* cam, vec2 xy_ind){
    vec2 mouse_pos;
    LinceGetMousePosWorld(mouse_pos, cam);

    int tm_x = floorf(mouse_pos[0] + tm->offset[0] + 0.5f);
    int tm_y = floorf(mouse_pos[1] + tm->offset[1] + 0.5f);

    xy_ind[0] = (float)tm_x;
    xy_ind[1] = (float)tm_y;

    if(tm_x >= 0 && tm_x < (int)tm->width &&
       tm_y >= 0 && tm_y < (int)tm->height) {
        return (int)(tm_y * tm->width + tm_x);
    }
    return -1;
}

/*
Returns the tile of a tilemap over which the mouse pointer is hovering.
If no tile is present, it returns NULL.
*/
LinceTile* GetTileAtMouse(LinceTilemap* tm, LinceCamera* cam){

    vec2 mouse_pos;
    LinceGetMousePosWorld(mouse_pos, cam);

    int tm_x = (int)(mouse_pos[0] + tm->offset[0]);
    int tm_y = (int)(mouse_pos[1] + tm->offset[1]);

    if(tm_x >= 0 && tm_x < (int)tm->width &&
       tm_y >= 0 && tm_y < (int)tm->height) {
        size_t idx = tm->base_grid[tm_y * tm->width + tm_x];
        return &tm->tileset[idx];
    }
    return NULL;
}


int LinceTilemapCheckCollision(LinceTilemap* tm, LinceBoxCollider player_box){
    LINCE_PROFILER_START(timer);

    for(size_t i = 0; i != tm->width; ++i){
        for(size_t j = 0; j != tm->height; ++j){

            size_t idx = j * tm->width + i;
            if(tm->logic_grid[idx] != BLOCK_SOLID) continue;

            LinceBoxCollider box = {
                .pos = {
                    (float)i - tm->offset[0],
                    (float)j - tm->offset[1],
                },
                .size = {1.0f, 1.0f}
            };
            
            int result = LinceCollideBox(&player_box, &box);
            if(result != LinceBoxCollision_None) return result; 
        }
    }

    LINCE_PROFILER_END(timer);
    return LinceBoxCollision_None;
}



// WALKING
enum WalkingAnims {
    ANIM_FRONT = 0,
    ANIM_BACK,
    ANIM_LEFT,
    ANIM_RIGHT,
    ANIM_FRONT_IDLE,
    ANIM_BACK_IDLE,
    ANIM_LEFT_IDLE,
    ANIM_RIGHT_IDLE,
    ANIM_COUNT
};


void ChickenLoops(LinceTileAnim* anim, void* args){
    LINCE_UNUSED(anim);
    LINCE_UNUSED(args);
}

void ChickenEnds(LinceTileAnim* anim, void* args){
    LINCE_UNUSED(anim);
    LINCE_UNUSED(args);
    printf("Chicken stopped moving!\n");
}



// TEST LAYER

typedef struct EditorLayer {
    char name[LINCE_NAME_MAX];
    float dt;
    float cam_speed;

    LinceCamera* cam;
    LinceVertexArray* va;
    LinceVertexBuffer vb;
    LinceIndexBuffer ib;
    LinceShader* shader;
    LinceTexture* tex_front;
    LinceTexture* tex_back;
    
    LinceTexture* walking_tileset;

    LinceTile* tiles;
    size_t tile_count;
    LinceTile tree_tile;

    uint8_t current_anim;
    uint32_t player_anim_order[ANIM_COUNT*2];
    LinceTileAnim* player_anim;
    LinceTileAnim* chicken_anim;

    LinceTexture* tileset;
    LinceTexture* tileset_noflip;
    LinceTilemap* tilemap;

    int chosen_menu_tile;

} EditorLayer;

/*
Highlights the tile where the mouse is
*/
void DrawHoverTile(EditorLayer* data){

    vec2 xy_ind;
    int chosen_tile = GetTilemapIndexAtMouse(data->tilemap, data->cam, xy_ind);
    if(chosen_tile == -1) return;

    LinceDrawQuad((LinceQuadProps){
        .x = xy_ind[0] - data->tilemap->offset[0],
        .y = xy_ind[1] - data->tilemap->offset[1],
        .w = 1.001f, .h = 1.001f,
        .color = {1,1,1,0.3},
        .zorder = 0.9
    });
}


void MovePlayer(EditorLayer* data){

    const float cam_speed = data->cam_speed;
    const float zoom      = data->cam->zoom;
    const float dr = cam_speed * data->dt * zoom;

    // camera & player movement
    uint32_t next_anim = data->current_anim;

    enum {IDLE=0x0, UP=0x1, DOWN=0x2, LEFT=0x4, RIGHT=0x8};
    uint8_t direction = IDLE;
    direction |= LinceIsKeyPressed(LinceKey_w) * UP;
    direction |= LinceIsKeyPressed(LinceKey_s) * DOWN;
    direction |= LinceIsKeyPressed(LinceKey_a) * LEFT;
    direction |= LinceIsKeyPressed(LinceKey_d) * RIGHT;

    vec2 next_pos = {data->cam->pos[0], data->cam->pos[1]};

    if (direction & UP){
        next_pos[1] += dr;
        next_anim = ANIM_FRONT;
    }
    if (direction & DOWN){
        next_pos[1] -= dr;
        next_anim = ANIM_BACK;
    }
    if (direction & RIGHT){
        next_pos[0] += dr;
        next_anim = ANIM_RIGHT;
    }
    if (direction & LEFT){
        next_pos[0] -= dr;
        next_anim = ANIM_LEFT;
    }
    if(direction == IDLE){
        switch(data->current_anim){
        case ANIM_FRONT: next_anim = ANIM_FRONT_IDLE; break;
        case ANIM_BACK:  next_anim = ANIM_BACK_IDLE;  break;
        case ANIM_RIGHT: next_anim = ANIM_RIGHT_IDLE; break;
        case ANIM_LEFT:  next_anim = ANIM_LEFT_IDLE;  break;
        default: break;
        };
    }
    
    LinceBoxCollider player_box = {
        .pos = {next_pos[0], next_pos[1] - 0.5f},
        .size = {0.5f, 0.5f}
    };

    int collision_result = LinceTilemapCheckCollision(data->tilemap, player_box);
    if(!(collision_result & LinceBoxCollision_X)){
        data->cam->pos[0] = next_pos[0];
    }
    if(!(collision_result & LinceBoxCollision_Y)){
        data->cam->pos[1] = next_pos[1];
    }
    
    // if( !LinceTilemapCheckCollision(data->tilemap, player_box) ){
    //     data->cam->pos[0] = next_pos[0];
    //     data->cam->pos[1] = next_pos[1];
    // }

    // Player box
    /*
	LinceDrawQuad((LinceQuadProps){
        .x = player_box.pos[0],
        .y = player_box.pos[1],
        .w = player_box.size[0],
        .h = player_box.size[1],
        .color = {0.5,0.7,1,1},
        .zorder = 0.8
    });
	*/


    // data->cam->pos[0] = next_pos[0];
    // data->cam->pos[1] = next_pos[1];

    data->player_anim->order[0] = data->player_anim_order[next_anim*2];
    data->player_anim->order[1] = data->player_anim_order[next_anim*2+1];

    if(next_anim != data->current_anim){        
        LinceResetTileAnim(data->player_anim);
        data->current_anim = next_anim;
    }

}


void EditorLayerOnAttach(LinceLayer* layer) {
    LINCE_PROFILER_START(timer);
    EditorLayer* data = LinceGetLayerData(layer);
    LINCE_INFO(" Layer '%s' attached", data->name);

    data->cam = LinceCreateCamera(LinceGetAspectRatio());
    data->cam_speed = 9e-4f;
    data->cam->zoom = 4.0;

    data->tex_front = LinceCreateTexture("PatrickF", "editor/assets/textures/front.png");
    data->tex_back  = LinceCreateTexture("PatrickB", "editor/assets/textures/back.png");
    data->tileset = LinceCreateTexture("Tileset", "editor/assets/textures/shubibubi-cozy-farm.png");
    data->tileset_noflip = LinceLoadTexture("Tileset", "editor/assets/textures/shubibubi-cozy-farm.png", 0);
    data->walking_tileset = LinceCreateTexture("Walking", "editor/assets/textures/elv-games-movement.png");

    // Tilemap & tileset
    data->tiles = LoadTilesFromTexture(
        "editor/assets/textures/shubibubi-cozy-farm.png",
        &data->tile_count, 16
    );
    data->tree_tile = LinceGetTile(data->tileset, (vec2){9,5}, (vec2){16,16}, (vec2){2, 2});


    data->tilemap = LinceCreateTilemap(&(LinceTilemap){
        .base_grid = tilemap_data,
        .bkg_grid = tilemap_bkg,
        .logic_grid = tilemap_solid,

        .width = tm_width,
        .height = tm_height,
        .tileset = data->tiles,
        .tileset_size = data->tile_count,
        .offset = {5.0f, 5.0f},

        .overlay_count = 2,
        .overlay_positions = (vec2[]){{8,7},{3,3}},
        .overlay_tiles = (LinceTile[]){data->tree_tile, data->tree_tile}
    });
    
    // PLAYER MOVEMENT
    LinceTile player_tiles[] = {
        // Walking Forward
        LinceGetTile(data->walking_tileset, (vec2){0,0}, (vec2){24,24}, (vec2){1,1}),
        LinceGetTile(data->walking_tileset, (vec2){2,0}, (vec2){24,24}, (vec2){1,1}),
        // Walking backwards
        LinceGetTile(data->walking_tileset, (vec2){0,3}, (vec2){24,24}, (vec2){1,1}),
        LinceGetTile(data->walking_tileset, (vec2){2,3}, (vec2){24,24}, (vec2){1,1}),
        // Walking Left
        LinceGetTile(data->walking_tileset, (vec2){0,2}, (vec2){24,24}, (vec2){1,1}),
        LinceGetTile(data->walking_tileset, (vec2){2,2}, (vec2){24,24}, (vec2){1,1}),
        // Walking Right
        LinceGetTile(data->walking_tileset, (vec2){0,1}, (vec2){24,24}, (vec2){1,1}),
        LinceGetTile(data->walking_tileset, (vec2){2,1}, (vec2){24,24}, (vec2){1,1}),
        // Idle Forward
        LinceGetTile(data->walking_tileset, (vec2){1,0}, (vec2){24,24}, (vec2){1,1}),
        // Idle backwards
        LinceGetTile(data->walking_tileset, (vec2){1,3}, (vec2){24,24}, (vec2){1,1}),
        // Idle Left
        LinceGetTile(data->walking_tileset, (vec2){1,2}, (vec2){24,24}, (vec2){1,1}),
        // Idle Right
        LinceGetTile(data->walking_tileset, (vec2){1,1}, (vec2){24,24}, (vec2){1,1}),
    };
    uint32_t order_indices[] = {
        0,  1,   // walk forward
        2,  3,   // walk backwards
        4,  5,   // walk left
        6,  7,   // walk right
        8,  8,   // idle forward
        9,  9,   // idle backward
        10, 10,  // idle left
        11, 11   // idle right
    };
    memmove(data->player_anim_order, order_indices, sizeof(uint32_t)*ANIM_COUNT*2);
    data->current_anim = ANIM_FRONT_IDLE;
    data->player_anim = LinceCreateTileAnim(&(LinceTileAnim){
        .frames = player_tiles,
        .frame_count = sizeof(player_tiles)/sizeof(LinceTile),
        .frame_time = 300.0f,
        .order = order_indices + ANIM_FRONT_IDLE*2,
        .order_count = 2
    });

    // CHICKEN ANIMATION
    LinceTile chicken_tiles[] = {
        LinceGetTile(data->tileset, (vec2){0,1}, (vec2){16,16}, (vec2){1, 1}),
        LinceGetTile(data->tileset, (vec2){1,1}, (vec2){16,16}, (vec2){1, 1}),
        LinceGetTile(data->tileset, (vec2){2,1}, (vec2){16,16}, (vec2){1, 1}),
        LinceGetTile(data->tileset, (vec2){3,1}, (vec2){16,16}, (vec2){1, 1}),
        LinceGetTile(data->tileset, (vec2){0,0}, (vec2){16,16}, (vec2){1, 1}),
        LinceGetTile(data->tileset, (vec2){1,0}, (vec2){16,16}, (vec2){1, 1}),
        LinceGetTile(data->tileset, (vec2){2,0}, (vec2){16,16}, (vec2){1, 1}),
        LinceGetTile(data->tileset, (vec2){3,0}, (vec2){16,16}, (vec2){1, 1}),
    };
    data->chicken_anim = LinceCreateTileAnim(&(LinceTileAnim){
        .frames = chicken_tiles,
        .frame_count = 8,
        .frame_time = 400.0f,
        .on_repeat = ChickenLoops,
        .on_finish = ChickenEnds,
        .repeats = 0,
        .start = 1,
        .order = (uint32_t[]){0,1,4,5},
        .order_count = 4
    });

    data->chosen_menu_tile = -1;
    LINCE_PROFILER_END(timer);
}

void EditorLayerOnDetach(LinceLayer* layer) {
    EditorLayer* data = LinceGetLayerData(layer);
    LINCE_INFO(" Layer '%s' detached", data->name);

    LinceDeleteTexture(data->tex_front);
    LinceDeleteTexture(data->tex_back);
    LinceDeleteTexture(data->walking_tileset);
    LinceDeleteCamera(data->cam);

    LinceDeleteTileAnim(data->player_anim);
    LinceDeleteTileAnim(data->chicken_anim);

    LinceDeleteTilemap(data->tilemap);

    LinceFree(data);
}

void DrawGUI(EditorLayer* data){
    LINCE_PROFILER_START(timer);

    LinceUILayer* ui = LinceGetAppState()->ui;
    vec2 screen_size;
    LinceGetScreenSize(screen_size);
    float width = screen_size[0], height = screen_size[1];

    nk_style_set_font(ui->ctx, &ui->fonts[LinceFont_Droid15]->handle);
    
    // TOP BAR
    float topbar_height = 50;
    static LinceBool show_new_tm_popup = LinceFalse;
    LINCE_UNUSED(show_new_tm_popup);
    
    if (nk_begin(ui->ctx, "TopBar", nk_rect(0,0, width, topbar_height), 0)){

        nk_layout_row_static(ui->ctx, 30, 50, 3);
        if( nk_button_label(ui->ctx, "Load") ){ }
        if( nk_button_label(ui->ctx, "Save") ){ }
        if( nk_button_label(ui->ctx, "New") ){
            show_new_tm_popup = LinceTrue;
        }
        nk_end(ui->ctx);
    }

    // TILE SELECTOR
    if (nk_begin(ui->ctx, "TilesetSelector", nk_rect(0, topbar_height, 250, height-topbar_height),
        NK_WINDOW_BORDER)) {

        // TILESET OPTIONS
        nk_layout_row_dynamic(ui->ctx, 30, 2);
        if( nk_button_label(ui->ctx, "Load tileset") ){ }
        nk_label(ui->ctx, "tileset.png", NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_MIDDLE);
        
        nk_layout_row_dynamic(ui->ctx, 30, 3);

        if (nk_option_label(ui->ctx, "Base",  map_choice == BASE_MAP)){
            map_choice = BASE_MAP;
        }
        if (nk_option_label(ui->ctx, "Bkg", map_choice == BKG_MAP)){
            map_choice = BKG_MAP;
        }
        if (nk_option_label(ui->ctx, "Logic", map_choice == LOGIC_MAP)){
            map_choice = LOGIC_MAP;
        }

        // TILE GRID
        for(size_t i = 0; i != data->tile_count; ++i){
            if(i % 4 == 0){
                nk_layout_row_static(ui->ctx, 50, 50, 4);
            }
            LinceTile* tile = &data->tiles[i];
            struct nk_rect rect = {
                .x = tile->pos[0]      * tile->cellsize[0],
                .y = tile->pos[1]      * tile->cellsize[1],
                .w = tile->tilesize[0] * tile->cellsize[0],
                .h = tile->tilesize[1] * tile->cellsize[1]        
            };
            struct nk_image img = nk_subimage_id(
                data->tileset_noflip->id,
                data->tileset_noflip->width,
                data->tileset_noflip->height,
                rect
            );
            if( nk_button_image(ui->ctx, img) ){
                // convert unflipped to flipped coordinates
                size_t w = data->tileset_noflip->width  / (size_t)(tile->cellsize[0]);
                size_t h = data->tileset_noflip->height / (size_t)(tile->cellsize[1]);
                size_t x = i % w;
                size_t y = h - i / w - 1;
                data->chosen_menu_tile = x + w * y;
            }
        }
    }
    nk_end(ui->ctx);

    LINCE_PROFILER_END(timer);
}


void EditorLayerOnUpdate(LinceLayer* layer, float dt) {
    LINCE_PROFILER_START(timer);

    EditorLayer* data = LinceGetLayerData(layer);
    data->dt = dt;
    LinceUILayer* ui = LinceGetAppState()->ui;
    LINCE_UNUSED(ui);

    // UI
    DrawGUI(data);

    LinceResizeCameraView(data->cam, LinceGetAspectRatio());
	LinceUpdateCamera(data->cam);

    if(LinceIsKeyPressed(LinceKey_i)){
        PrintDebugTiledata(data->tilemap);
    }
    if(LinceIsKeyPressed(LinceKey_o)){
        PrintDebugSolidData(data->tilemap);
    }
    if(LinceIsKeyPressed(LinceKey_p)){
        PrintDebugBkgData(data->tilemap);
    }

    LinceBeginScene(data->cam);

    MovePlayer(data);

    // Tilemap
    if(map_choice == LOGIC_MAP)
        DrawCollisionTiles(data->tilemap); 
    else
        LinceDrawTilemap(data->tilemap);
        
    // Chicken
    LinceUpdateTileAnim(data->chicken_anim, dt);
    LinceDrawQuad((LinceQuadProps){
        .x=5.0f, .y=3.0f,
        .w=1.0f, .h=1.0f,
        .color = {1,1,1,1},
        .tile = data->chicken_anim->current_tile,
        .zorder = LinceYSortedZ(
            3.0f-0.5f,
            (vec2){-100.0f,100.0f},
            (vec2){0.1f,0.9f}
        )
    });

    // PLAYER
    LinceUpdateTileAnim(data->player_anim, dt);
    LinceDrawQuad((LinceQuadProps){
        .x=data->cam->pos[0],
        .y=data->cam->pos[1],
        .w=1.5f, .h=1.5f,
        .color={1,1,1,1},
        .tile =  data->player_anim->current_tile,
        .zorder = LinceYSortedZ(
            data->cam->pos[1]-0.75f,
            (vec2){-100.0f,100.0f},
            (vec2){0.1f,0.9f}
        )
    });

    // Selected tile
    DrawHoverTile(data);

    
    LinceEndScene();

    LINCE_PROFILER_END(timer);
}

void EditorLayerOnEvent(LinceLayer* layer, LinceEvent* event){
    EditorLayer* data = LinceGetLayerData(layer);

    if(event->type == LinceEventType_MouseScrolled){
        LinceMouseScrolledEvent* scroll = event->data.MouseScrolled;    
        data->cam->zoom *= powf(0.80, scroll->yoff); // * 0.5 * dt;
        return;
    }

    else if(event->type == LinceEventType_MouseButtonPressed){
        LinceMouseButtonPressedEvent* press = event->data.MouseButtonPressed;
        if(press->button != LinceMouseButton_1) return;
        
        vec2 xy_ind;
        int change_tile = GetTilemapIndexAtMouse(data->tilemap, data->cam, xy_ind);
        if(change_tile == -1 || data->chosen_menu_tile == -1) return;
        if(map_choice == BASE_MAP)
            data->tilemap->base_grid[change_tile] = data->chosen_menu_tile;
        else if(map_choice == LOGIC_MAP)
            data->tilemap->logic_grid[change_tile] = !data->tilemap->logic_grid[change_tile];
        else if(map_choice == BKG_MAP)
            data->tilemap->bkg_grid[change_tile] = data->chosen_menu_tile;
    }
}

LinceLayer* EditorLayerInit(char* name) {

    EditorLayer* my_layer = LinceCalloc(sizeof(EditorLayer));

    size_t len_orig = strlen(name);
    size_t len = len_orig < LINCE_NAME_MAX ? len_orig : LINCE_NAME_MAX;
    memmove(my_layer->name, name, len);

    LinceLayer* layer = LinceCreateLayer(my_layer);
    layer->OnAttach = EditorLayerOnAttach;
    layer->OnDetach = EditorLayerOnDetach;
    layer->OnEvent  = EditorLayerOnEvent;
    layer->OnUpdate = EditorLayerOnUpdate;

    return layer;
}


// =============================================================

void EditorInit() {
	LINCE_INFO("\n User App Initialised");
    
    LincePushLayer(EditorLayerInit("Test"));
    //LincePushLayer(NKLayerInit());
}

void EditorOnUpdate(float dt) {
    LinceCheckErrors();
    LINCE_UNUSED(dt);
}

void EditorTerminate() {
    LINCE_INFO(" User App Terminated");
}

