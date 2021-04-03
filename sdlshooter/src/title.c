/*
Copyright (C) 2015-2018 Parallel Realities

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "title.h"

static void logic(void);
static void draw(void);
static void drawLogo(void);

static int timeout;
static SDL_Texture *battleArenaTexture;
static SDL_Texture *donkTexture[5];
static int battleArenaY = -200;
static float battleArenaDY = 0;
static float donkAngle[5];

void initTitle(void)
{
	app.delegate.logic = logic;
	app.delegate.draw = draw;
	
	memset(app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
	memset(&app.mouse, 0, sizeof(Mouse));
	
	battleArenaTexture = loadTexture("gfx/battleArena.png");
	donkTexture[0] = loadTexture("gfx/D.png");
	donkTexture[1] = loadTexture("gfx/O.png");
	donkTexture[2] = loadTexture("gfx/N.png");
	donkTexture[3] = loadTexture("gfx/K.png");
	donkTexture[4] = loadTexture("gfx/!.png");
	
	donkAngle[0] = rand() % 360;
	donkAngle[1] = rand() % 360;
	donkAngle[2] = rand() % 360;
	donkAngle[3] = rand() % 360;
	donkAngle[4] = rand() % 360;
	
	if (rand() % 5 == 0)
	{
		battleArenaY = -200;
		battleArenaDY = 0;
	}
	
	timeout = FPS * 5;
}

static void logic(void)
{
	int i;
	
	battleArenaDY = MIN(battleArenaDY + 0.25, 25);
	
	battleArenaY = MIN(battleArenaY + battleArenaDY, 200);
	
	if (battleArenaY == 200)
	{
		battleArenaDY = -battleArenaDY * 0.5;
		
		if (battleArenaDY > -1)
		{
			battleArenaDY = 0;
		}
	}
	
	for (i = 0 ; i < 5 ; i++)
	{
		donkAngle[i] = MIN(donkAngle[i] + 2, 360);
	}
	
	if (--timeout <= 0)
	{
		initHighscores();
	}
	
	if (app.mouse.button[SDL_BUTTON_LEFT])
	{
		initStage();
	}
}

static void draw(void)
{
	drawLogo();
	
	if (timeout % 40 < 20)
	{
		drawText(SCREEN_WIDTH / 2, 600, 255, 255, 255, TEXT_CENTER, "PRESS FIRE TO PLAY!");
	}
}

static void drawLogo(void)
{
	int x, i;
	
	blit(battleArenaTexture, SCREEN_WIDTH / 2, battleArenaY, 1);
	
	x = -300;
	
	for (i = 0 ; i < 5 ; i++)
	{
		blitRotated(donkTexture[i], (SCREEN_WIDTH / 2) + x, 350, donkAngle[i]);
		
		x += 150;
	}
}
