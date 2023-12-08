
/*
	===== PONG =====

	Simple game of Pong built with the Lince engine.

	Player BLUE (left)
		W - paddle up
		S - paddle down

	Player RED (right)
		UP   - paddle up
		DOWN - paddle down

	P     - pause game
	SPACE - resume game

*/

#ifndef PONG_H
#define PONG_H

#include "lince.h"

void PongInit();
void PongOnUpdate(float dt);
void PongOnEvent(LinceEvent* e);
void PongQuit();

#endif /* PONG_H */