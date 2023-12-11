#include "pong.h"
#include "lince/physics/boxcollider.h"

#include <miniaudio.h>


/*

To-Do

1 - ECS: Add function LinceInitEntityRegistry(&ecs, comps, ...)
2 - BoxCollider: Delete function 'LinceCalculateEntityCollisions'
3 - BoxCollider: Add function 'LinceComputeCollision(box1, box2)'
	- Returns value > 0 when colliders are in contact when moved by dx,dy.
	- Value is collider flag

LinceCollider {
	int type; // Box, Circle
	union {
		LinceBoxCollider,
		LinceCircleCollider,
		...
	}
}

4 - ECS: Add utility components and systems, e.g. check collisions for all entities with BoxCollider

5 - Maybe sprite doesn't need to hold x,y position? Provide this when calling LinceDrawSprite(sprite, transform, shader).
	- This warrants a new component Transform{x, y, width, height}.
	- We can then remove the x,y position from the box colliders
	

--- Components ---

	LinceUUID;

	LinceTag { char* tag; };

	LinceTransform {
		float x, y; // position
		float w, h; // scale
		float angle;
	}

	LinceSprite {
		LinceTexture texture;
		vec4 color;
	}

	LinceBoxCollider {
		float x, y; // offset from LinceTransform
		float w, h; // factor scale from LinceTransform
		float vx, vy;
		float mass, angle, etc;
		int flags; // whether static, kinematic, or dynamic
	}

	LincePhysicsData {
		float mass;
		float x, y, vx, vy, ax, ay;
		float wx, wy; // angular speed
		float tx, ty; // angular acceleration
		float 
	}

*/


enum Components {
	Comp_Sprite = 0,
	Comp_Collider,
	Comp_Num
};


typedef struct GameData{
	LinceCamera* cam;
	LinceEntityRegistry* ecs;
	uint32_t ball_id, lpad_id, rpad_id; /* Entity IDs */
	LinceTexture *ball_tex, *pad_tex;
	float pad_speed;
	vec2 speed_lim; /* Limits to ball speed */
	int lscore, rscore; /* Points scored by the players */
	LinceBool paused, new_game;
} GameData;


/* Returns a random number uniformly distributed between provided limits (min, max] */
float GetRandomFloat(float min, float max) {
	return min + (rand() / (float)RAND_MAX) * (max - min);
}


/*
Future DrawDebugTextBox(ui, config, "text", ...)

LinceDebugTextBox {
	float x, y; // position 
	float w, h; // 
	int font;
	int coords; // LINCE_COORDS_PIXELS, LINCE_COORDS_SCREEN, LINCE_COORDS_WORLD
}

*/
void DrawUIText(
		LinceUILayer* ui,
		float x,	/* X position of top-left corner */
		float y,	/* Y position of top-left corner */
		float w,	/* Width of text box */
		float h,	/* Height of text box */
		int font,	/* Text font */
		const char* name,		/* Unique name for the textbox */
		const char* fmt, ...	/* Formatted text and arguments */
	){
	
	struct nk_context *ctx = ui->ctx;
	nk_style_set_font(ctx, &ui->fonts[font]->handle);
	struct nk_style *s = &ctx->style;
	nk_style_push_color(ctx, &s->window.background, nk_rgba(0,0,0,0));
	nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(0,0,0,0)));

	if (nk_begin(ctx, name, nk_rect(x, y, w*1.5, h*1.5), 0)
	) {
		nk_layout_row_static(ctx, h, w, 1);
		va_list args;
		va_start(args, fmt);
		nk_labelfv(ctx, NK_TEXT_ALIGN_CENTERED, fmt, args);
		va_end(args);
	}
	nk_end(ctx);
	
	nk_style_pop_color(ctx);
	nk_style_pop_style_item(ctx);
}



void ResetBall(){
	GameData* data = LinceGetApp()->user_data;
	LinceBoxCollider* ball = LinceGetEntityComponent(data->ecs, data->ball_id, Comp_Collider);
	ball->x = 0;
	ball->y = 0;
	ball->dx = 0;
	ball->dy = 0;
	data->paused = LinceTrue;
}



void PongInit(){

	srand(time(NULL)); // Random seed

	LincePushAssetDir(&LinceGetApp()->asset_manager, "../../../lince/lince/assets");
	LincePushAssetDir(&LinceGetApp()->asset_manager, "../../../pong/assets");

	// Initial state
	GameData game_data = {
		.ecs = LinceCreateEntityRegistry(2, sizeof(LinceSprite), sizeof(LinceBoxCollider)),
		.cam = LinceCreateCamera(LinceGetAspectRatio()),
		.lscore     = 0,
		.rscore     = 0,
		.paused     = LinceTrue,
		.new_game   = LinceTrue,
		.pad_speed  = 1e-3, // units per second
		.ball_tex = LinceLoadTexture(LinceFetchAssetPath(&LinceGetApp()->asset_manager,"pong_ball.png"), 0),
		.pad_tex = LinceLoadTexture(LinceFetchAssetPath(&LinceGetApp()->asset_manager,"pong_pad.png"), 0),

	};

	// --- Initialise entities ---
	// Ball
	game_data.ball_id = LinceCreateEntity(game_data.ecs);
	LinceAddEntityComponent(game_data.ecs, game_data.ball_id, Comp_Sprite, &(LinceSprite){
		.x = 0.0, .y = 0.0, .w = 0.1, .h = 0.1, .color = { 0.0, 1.0, 0.0, 1.0 }, .texture = game_data.ball_tex
	});
	LinceAddEntityComponent(game_data.ecs, game_data.ball_id, Comp_Collider, &(LinceBoxCollider){
		.x = 0.0, .y = 0.0, .w = 0.1, .h = 0.1
	});
	// Left pad
	game_data.lpad_id = LinceCreateEntity(game_data.ecs);
	LinceAddEntityComponent(game_data.ecs, game_data.lpad_id, Comp_Sprite, &(LinceSprite){
		.x = -1.4, .y = 0.0, .w = 0.2, .h = 0.5, .color = { 0.0, 0.0, 1.0, 1.0 }, .texture = game_data.pad_tex
	});
	LinceAddEntityComponent(game_data.ecs, game_data.lpad_id, Comp_Collider, &(LinceBoxCollider){
		.x = -1.4, .y = 0.0, .w = 0.2, .h = 0.5
	});
	// Right pad
	game_data.rpad_id = LinceCreateEntity(game_data.ecs);
	LinceAddEntityComponent(game_data.ecs, game_data.rpad_id, Comp_Sprite, &(LinceSprite){
		.x = 1.4, .y = 0.0, .w = 0.2, .h = 0.5, .color = { 1.0, 0.0, 0.0, 1.0 }, .texture = game_data.pad_tex
	});
	LinceAddEntityComponent(game_data.ecs, game_data.rpad_id, Comp_Collider, &(LinceBoxCollider){
		.x = 1.4, .y = 0.0, .w = 0.2, .h = 0.5
	});
	// Walls
	uint32_t top_wall = LinceCreateEntity(game_data.ecs);
	LinceAddEntityComponent(game_data.ecs, top_wall, Comp_Sprite, &(LinceSprite){
		.x = 0, .y = 0.8, .w = 5, .h = 0.05, .color = { 1.0, 1.0, 1.0, 1.0 }
	});
	LinceAddEntityComponent(game_data.ecs, top_wall, Comp_Collider, &(LinceBoxCollider){
		.x = 0, .y = 0.8, .w = 5, .h = 0.05, .flags = LinceBoxCollider_Static
	});
	uint32_t bot_wall = LinceCreateEntity(game_data.ecs);
	LinceAddEntityComponent(game_data.ecs, bot_wall, Comp_Sprite, &(LinceSprite){
		.x = 0, .y = -0.8, .w = 5, .h = 0.05, .color = { 1.0, 1.0, 1.0, 1.0 }
	});
	LinceAddEntityComponent(game_data.ecs, bot_wall, Comp_Collider, &(LinceBoxCollider){
		.x = 0, .y = -0.8, .w = 5, .h = 0.05, .flags = LinceBoxCollider_Static
	});

	GameData* data = LinceNewCopy(&game_data, sizeof(GameData));
	LinceGetApp()->user_data = data;
}




void PongOnUpdate(float dt){
	LinceCheckErrors();
	
	GameData* data = LinceGetApp()->user_data;
	LinceUILayer* ui = LinceGetApp()->ui;
	const float width = (float)LinceGetApp()->window->width;
	const float height = (float)LinceGetApp()->window->height;

	// Update view
	LinceResizeCameraView(data->cam, LinceGetAspectRatio());
	LinceUpdateCamera(data->cam);

	// Update pause state
	if(data->paused){
		
		if (data->new_game) {
			DrawUIText(ui, width / 2 - 150, height / 2 + 60, 300, 50,
				LinceFont_Droid30, "NewGame", " PRESS SPACE TO START ");
		}

		if(LinceIsKeyPressed(LinceKey_Space)){
			data->new_game = LinceFalse;
			data->paused = LinceFalse;

			LinceBoxCollider* ball = LinceGetEntityComponent(data->ecs, data->ball_id, Comp_Collider);
			// Randomize ball speed
			ball->dx = GetRandomFloat(0.01, 0.025);
			ball->dy = GetRandomFloat(0.01, 0.025);
			// Randomize direction
			ball->dx *= (rand() > RAND_MAX/2) ? (-1) : 1;
			ball->dy *= (rand() > RAND_MAX/2) ? (-1) : 1; 
		}
	}


	// Print scores
	DrawUIText(ui, 20,       3, 40, 40, LinceFont_Droid50, "LScore", "%d", data->lscore);
	DrawUIText(ui, width-60, 3, 40, 40, LinceFont_Droid50, "RScore", "%d", data->rscore);

	
	/* --- ECS queries  --- */
	array_t query;
	array_init(&query, sizeof(uint32_t));

	// Compute collisions
	LinceQueryEntities(data->ecs, &query, 1, Comp_Collider);
	for (uint32_t i = 0; i != query.size; ++i) {
		uint32_t id = *(uint32_t*)array_get(&query, i);

		LinceBoxCollider* box = LinceGetEntityComponent(data->ecs, id, Comp_Collider);
		if (box->flags & LinceBoxCollider_Static) continue;

		for (uint32_t j = 0; j != query.size; ++j) {
			if (i == j) continue;
			uint32_t id2 = *(uint32_t*)array_get(&query, j);

			LinceBoxCollider* box2 = LinceGetEntityComponent(data->ecs, id2, Comp_Collider);

			// Horizontal collision
			box->x += box->dx; // * dt
			box->flags |= LinceBoxCollider_CollisionX * LinceBoxCollides(box, box2);
			box->x -= box->dx; // * dt

			// Vertical collision
			box->y += box->dy; // * dt
			box->flags |= LinceBoxCollider_CollisionY * LinceBoxCollides(box, box2);
			box->y -= box->dy; // * dt

			// Both
			box->x += box->dx; // * dt
			box->y += box->dy; // * dt
			box->flags |= (LinceBoxCollider_CollisionX & LinceBoxCollider_CollisionY) * LinceBoxCollides(box, box2);
			box->x -= box->dx; // * dt
			box->y -= box->dy; // * dt
		}
	}

	// Update collider positions
	for (uint32_t i = 0; i != query.size; ++i) {
		uint32_t id = *(uint32_t*)array_get(&query, i);
		LinceBoxCollider* box = LinceGetEntityComponent(data->ecs, id, Comp_Collider);
		if (box->flags & LinceBoxCollider_Static) continue;

		if (box->flags & LinceBoxCollider_CollisionX) box->dx = -box->dx;
		if (box->flags & LinceBoxCollider_CollisionY) box->dy = -box->dy;
		box->x += box->dx;
		box->y += box->dy;
		box->flags = 0; // reset flags
	}
	array_clear(&query);

	// Move paddles
	LinceBoxCollider* lbox = LinceGetEntityComponent(data->ecs, data->lpad_id, Comp_Collider);
	if (LinceIsKeyPressed(LinceKey_w)) lbox->dy = data->pad_speed * dt;
	else if (LinceIsKeyPressed(LinceKey_s)) lbox->dy = -data->pad_speed * dt;
	else lbox->dy = 0.0;

	LinceBoxCollider* rbox = LinceGetEntityComponent(data->ecs, data->rpad_id, Comp_Collider);
	if (LinceIsKeyPressed(LinceKey_Up)) rbox->dy = data->pad_speed * dt;
	else if (LinceIsKeyPressed(LinceKey_Down)) rbox->dy = -data->pad_speed * dt;
	else rbox->dy = 0.0;
	
	// Check scoring
	LinceBoxCollider* ball = LinceGetEntityComponent(data->ecs, data->ball_id, Comp_Collider);
	if (ball->x > 1.5) {
		data->lscore += 1;
		ResetBall();
	}
	else if (ball->x < -1.5) {
		data->rscore += 1;
		ResetBall();
	}
	
	// Draw all entities
	LinceBeginScene(data->cam);

	LinceQueryEntities(data->ecs, &query, 1, Comp_Sprite);
	for (uint32_t i = 0; i != query.size; ++i) {
		uint32_t id = *(uint32_t*)array_get(&query, i);
		LinceSprite* sprite = LinceGetEntityComponent(data->ecs, id, Comp_Sprite);
		
		// Update sprite position if collider moved
		LinceBoxCollider* box;
		if (box = LinceGetEntityComponent(data->ecs, id, Comp_Collider)) {
			sprite->x = box->x; sprite->y = box->y;
		}
		LinceDrawSprite(sprite, NULL);
	}
	array_uninit(&query);

	LinceEndScene();
}

void PongQuit(){
	GameData* data = LinceGetApp()->user_data;
	LinceDeleteTexture(data->ball_tex);
	LinceDeleteTexture(data->pad_tex);
	LinceDeleteCamera(data->cam);

	LinceDestroyEntityRegistry(data->ecs);
	LinceFree(data);
}