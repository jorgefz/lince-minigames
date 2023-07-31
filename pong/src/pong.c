#include "pong.h"

#include <miniaudio.h>


typedef struct GameObject {
	float w, h;   // size
	float x, y;   // position
	float vx, vy; // speed
} GameObject;

enum Collision {
	NO_COLLISION = 0,
	LEFT_COLLISION,
	RIGHT_COLLISION
};

typedef struct GameLayer{
	LinceCamera* cam; // camera
	LinceTexture *ball_tex, *pad_tex; // Textures
	GameObject ball, lpad, rpad;
	float pad_speed;

	/* Indicates whether ball just collided with left or right bounds */
	enum Collision ball_state;

	/* Points scored by the players */
	int lscore, rscore;

	/* Indicate whether game is paused or not */
	LinceBool paused, new_game;

	ma_engine audio_engine;

} GameLayer;


void DrawText(
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

void PlayBallBounceSound(GameLayer* data){
	ma_result res = ma_engine_play_sound(&data->audio_engine,
		"pong/assets/ball-single-bounce-concrete.wav",
		NULL
	);
	if(res != MA_SUCCESS){
		printf("Failed to play sound\n");
	}
}

/* Displaces paddle by 'dy' distance only if it's within bounds (ymin,ymax) */
void MovePaddle(GameObject* pad, float dy, float ymin, float ymax){
	enum {UP, DOWN};
	int dir = (dy < 0.0f) ? DOWN : UP;

	if (dir == DOWN){
		if (pad->y - pad->h/2.0f + dy > ymin){
			pad->y += dy;
		}
	}
	else if (dir == UP){
		if (pad->y + pad->h/2.0f + dy < ymax){
			pad->y += dy;
		}
	}
}

void MoveBall(GameObject* ball, float dt, float xmin, float xmax, float ymin, float ymax){
	GameLayer* data = LinceGetAppState()->user_data;
	
	// x direction
	enum {LEFT, RIGHT};
	float dx = ball->vx * dt;
	int xdir = (dx < 0.0f) ? LEFT : RIGHT;

	if (xdir == LEFT) {
		if (ball->x - ball->w/2.0f + dx < xmin){
			ball->vx *= -1.0f;
			data->ball_state = LEFT_COLLISION;
			PlayBallBounceSound(data);
		}
		ball->x += dx;
	}
	else if (xdir == RIGHT) {
		if (ball->x + ball->w/2.0f + dx > xmax){
			ball->vx *= -1.0f;
			data->ball_state = RIGHT_COLLISION;
			PlayBallBounceSound(data);
		}
		ball->x += dx;
	}

	// y direction
	enum {UP, DOWN};
	float dy = ball->vy * dt;
	int ydir = (dy < 0.0f) ? DOWN : UP;

	if (ydir == DOWN) {
		if (ball->y - ball->h/2.0f + dy < ymin){
			ball->vy *= -1.0f;
			PlayBallBounceSound(data);
		}
		ball->y += dy;
	}
	else if (ydir == UP) {
		if (ball->y + ball->h/2.0f + dy > ymax){
			ball->vy *= -1.0f;
			PlayBallBounceSound(data);
		}
		ball->y += dy;
	}

}

void ResetGame(){
	GameLayer* data = LinceGetAppState()->user_data;
	data->ball.x = 0.0f;
	data->ball.y = 0.0f;
	data->new_game = LinceTrue;
}

void CheckPaddleCollision(){
	GameLayer* data = LinceGetAppState()->user_data;
	int contact;

	switch (data->ball_state) {
	case LEFT_COLLISION:
		contact =
			(data->ball.y - data->ball.h / 2.0f <= data->lpad.y + data->lpad.h / 2.0f) &&
			(data->ball.y + data->ball.h / 2.0f >= data->lpad.y - data->lpad.h / 2.0f);
		if(!contact){
			data->rscore += 1;
			ResetGame();
		}
		data->ball_state = NO_COLLISION;
		break;

	case RIGHT_COLLISION:
		contact =
			(data->ball.y - data->ball.h / 2.0f <= data->rpad.y + data->rpad.h / 2.0f) &&
			(data->ball.y + data->ball.h / 2.0f >= data->rpad.y - data->rpad.h / 2.0f);
		if(!contact){
			data->lscore += 1;
			ResetGame();
		}
		data->ball_state = NO_COLLISION;
		break;
	
	case NO_COLLISION:
		break;
	}
}


/*******************
*	PUBLIC API     *
********************/

void PongInit(){

	GameLayer* data = LinceCalloc(sizeof(GameLayer));
	LinceGetAppState()->user_data = data;

	ma_engine_init(NULL, &data->audio_engine);

	data->ball_state = NO_COLLISION;
	data->lscore = 0;
	data->rscore = 0;
	data->paused = LinceFalse;
	data->new_game = LinceTrue;
	data->pad_speed = 2e-3f;

	static GameObject ball = {
		.w = 0.1f, .h = 0.1f,
		.x = 0.0f, .y = 0.0f,
		.vx = 0.0f, .vy = 0.0f
	};
	static GameObject lpad = {
		.w = 0.2f, .h = 0.5f,
		.x = -1.4f, .y = 0.0f,
		.vx = 0.0f, .vy = 0.0f
	};
	static GameObject rpad = {
		.w = 0.2f, .h = 0.5f,
		.x = 1.4f, .y = 0.0f,
		.vx = 0.0f, .vy = 0.0f
	};

	memmove(&data->ball, &ball, sizeof(GameObject));
	memmove(&data->rpad, &rpad, sizeof(GameObject));
	memmove(&data->lpad, &lpad, sizeof(GameObject));

	data->cam = LinceCreateCamera(LinceGetAspectRatio());
	data->ball_tex = LinceCreateTexture("PongBall", "pong/assets/pong_ball.png");
	data->pad_tex  = LinceCreateTexture("PongBall", "pong/assets/pong_pad.png");

	data->ball.vx = 1e-3f; // randomize initial speed and direction
	data->ball.vy = 2e-3f;
}

void PongOnUpdate(float dt){
	LinceCheckErrors();
	
	GameLayer* data = LinceGetAppState()->user_data;
	LinceUILayer* ui = LinceGetAppState()->ui;
	const float width = (float)LinceGetAppState()->window->width;
	const float height = (float)LinceGetAppState()->window->height;

	// update view
	LinceResizeCameraView(data->cam, LinceGetAspectRatio());
	LinceUpdateCamera(data->cam);

	// update pause state
	if(data->new_game){
		DrawText(ui, width/2-150, height/2+60, 300, 50, LinceFont_Droid30, "NewGame", " PRESS SPACE TO START ");

		if(LinceIsKeyPressed(LinceKey_Space)){
			data->new_game = LinceFalse;
		}
	
	} else if (data->paused) {
		DrawText(ui, width/2-150, height/2+60, 300, 50, LinceFont_Droid30, "ResumeGame", " PRESS SPACE TO RESUME ");

		if(LinceIsKeyPressed(LinceKey_Space)){
			data->paused = LinceFalse;
		}
	} else {
		if(LinceIsKeyPressed(LinceKey_p)){
			data->paused = LinceTrue;
		}
	}

	if (!data->new_game && !data->paused){
		// update paddle movement
		// static const float pad_speed = 2e-3f; //  MOVE TO
		const float pad_speed = data->pad_speed;
		// -- left paddle
		if (LinceIsKeyPressed(LinceKey_w)) MovePaddle(&data->lpad, pad_speed*dt, -1.0, 1.0); 
		if (LinceIsKeyPressed(LinceKey_s)) MovePaddle(&data->lpad, -pad_speed*dt, -1.0, 1.0);
		// -- right paddle
		if (LinceIsKeyPressed(LinceKey_Up))   MovePaddle(&data->rpad, pad_speed*dt, -1.0, 1.0); 
		if (LinceIsKeyPressed(LinceKey_Down)) MovePaddle(&data->rpad, -pad_speed*dt, -1.0, 1.0); 

		// update ball movement
		MoveBall(&data->ball, dt, data->lpad.x + data->lpad.w/2.0f, data->rpad.x - data->rpad.w/2.0f, -1.0, 1.0);
		CheckPaddleCollision();
	}

	// Print scores
	DrawText(ui, 20,       20, 40, 40, LinceFont_Droid50, "LScore", "%d", data->lscore);
	DrawText(ui, width-60, 20, 40, 40, LinceFont_Droid50, "RScore", "%d", data->rscore);

	// Draw paddles and ball
	LinceBeginScene(data->cam);

	LinceDrawSprite(&(LinceSprite){
		.x = data->ball.x, .y = data->ball.y,
		.w = data->ball.w, .h = data->ball.h,
		.color = {0.0, 1.0, 0.0, 1.0},
		.texture = data->ball_tex
	}, NULL);

	LinceDrawSprite(&(LinceSprite){
		.x = data->lpad.x, .y = data->lpad.y,
		.w = data->lpad.w, .h = data->lpad.h,
		.color = {0.0, 0.0, 1.0, 1.0},
		.texture = data->pad_tex
	}, NULL);

	LinceDrawSprite(&(LinceSprite){
		.x = data->rpad.x, .y = data->rpad.y,
		.w = data->rpad.w, .h = data->rpad.h,
		.color = {1.0, 0.0, 0.0, 1.0},
		.texture = data->pad_tex
	}, NULL);

	LinceEndScene();
}

void PongOnEvent(LinceEvent* event){
	LINCE_UNUSED(event);
}

void PongQuit(){
	GameLayer* data = LinceGetAppState()->user_data;
	LinceDeleteTexture(data->ball_tex);
	LinceDeleteTexture(data->pad_tex);
	LinceDeleteCamera(data->cam);
	ma_engine_uninit(&data->audio_engine);
	free(data);
}