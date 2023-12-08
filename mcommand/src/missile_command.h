#ifndef MISSILE_COMMAND_H
#define MISSILE_COMMAND_H

#include <lince.h>
#include "timer.h"

typedef struct GameState {

	int score, hp;
	float cannon_x, cannon_y;
	float missile_vmax;
	float ymin, ymax;
	float xmin, xmax;
	float angle; // cannon angle
	float dt;    // delta time
	LinceCamera* cam;

	Timer missile_timer; // how often player is able to launch
	Timer bomb_timer; // how often to drop bombs

	array_t missile_list;
	array_t bomb_list;
	array_t blast_list;  // kill radius generated when missile detonates
	array_t marker_list; // points where missile is directed

	LinceTexture* missile_tex;
	LinceTexture* bomb_tex;
	LinceTexture* blast_tex;
	LinceTexture* marker_tex;
	LinceTexture* bkg_city;

} GameState;

void MCommandInit();
void MCommandUpdate(float dt);
void MCommandOnEvent(LinceEvent* event);
void MCommandTerminate();

#endif /* MISSILE_COMMAND_H */