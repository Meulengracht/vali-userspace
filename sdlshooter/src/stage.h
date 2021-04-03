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

#include "common.h"

extern void addEnemy(int x, int y);
extern void addHighscore(int score);
extern void addPlayer(void);
extern void addPointsPowerup(int x, int y);
extern void blit(SDL_Texture *texture, int x, int y, int center);
extern void doBullets(void);
extern void doCamera(void);
extern void doEffects(void);
extern void doEntities(void);
extern void doPlayer(void);
extern void drawBullets(void);
extern void drawEffects(void);
extern void drawEntities(void);
extern void drawText(int x, int y, int r, int g, int b, int align, char *format, ...);
extern int getDistance(int x1, int y1, int x2, int y2);
extern void initHighscores(void);
extern SDL_Texture *loadTexture(char *filename);

extern App app;
extern Entity *player;
extern Stage stage;
