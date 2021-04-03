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

#include "enemies.h"

static void tick1(void);
static void tick2(void);
static void tick3(void);
static void die(void);

static SDL_Texture *enemyTexture[3];

void initEnemies(void)
{
	enemyTexture[0] = loadTexture("gfx/enemy01.png");
	enemyTexture[1] = loadTexture("gfx/enemy02.png");
	enemyTexture[2] = loadTexture("gfx/enemy03.png");
}

void addEnemy(int x, int y)
{
	Entity *e;
	
	e = malloc(sizeof(Entity));
	memset(e, 0, sizeof(Entity));
	stage.entityTail->next = e;
	stage.entityTail = e;
	
	e->side = SIDE_ENEMY;
	e->x = x;
	e->y = y;
	
	e->color.r = e->color.g = e->color.b = e->color.a = 255;
	
	switch (rand() % 12)
	{
		case 0:
			e->texture = enemyTexture[1];
			e->tick = tick2;
			e->radius = 35;
			e->health = 25;
			break;
			
		case 1:
		case 2:
			e->texture = enemyTexture[2];
			e->tick = tick3;
			e->radius = 26;
			e->health = 1;
			break;
			
		default:
			e->texture = enemyTexture[0];
			e->tick = tick1;
			e->radius = 32;
			e->health = 5;
			break;
	}
	
	SDL_QueryTexture(e->texture, NULL, NULL, &e->w, &e->h);
	
	e->die = die;
}

static void tick1(void)
{
	if (player != NULL)
	{
		self->angle = getAngle(self->x, self->y, player->x, player->y);
		
		calcSlope(player->x, player->y, self->x, self->y, &self->dx, &self->dy);
		
		self->reload = MAX(self->reload - 1, 0);
		
		if (self->reload <= 0 && getDistance(self->x, self->y, player->x, player->y) < SCREEN_HEIGHT / 2)
		{
			fireEnemyBullet();
			
			self->reload = FPS * 3;
		}
	}
}

static void tick2(void)
{
	float dx, dy;
	
	if (++self->angle >= 360)
	{
		self->angle = 0;
	}
	
	if (player != NULL)
	{
		calcSlope(player->x, player->y, self->x, self->y, &dx, &dy);
		
		dx /= 10;
		dy /= 10;
		
		self->dx += dx;
		self->dy += dy;
		
		self->dx = MAX(MIN(self->dx, 3), -3);
		self->dy = MAX(MIN(self->dy, 3), -3);
		
		self->reload = MAX(self->reload - 1, 0);
		
		if (self->reload <= 0 && getDistance(self->x, self->y, player->x, player->y) < SCREEN_HEIGHT / 2)
		{
			fireEnemyBullet();
			
			self->reload = FPS / 2;
		}
	}
}

static void tick3(void)
{
	float dx, dy;
	
	self->angle -= 5;
	
	if (self->angle < 0)
	{
		self->angle = 359;
	}
	
	if (player != NULL)
	{
		calcSlope(player->x, player->y, self->x, self->y, &dx, &dy);
		
		dx /= 10;
		dy /= 10;
		
		self->dx += dx;
		self->dy += dy;
		
		self->dx = MAX(MIN(self->dx, 5), -5);
		self->dy = MAX(MIN(self->dy, 5), -5);
		
		self->reload = MAX(self->reload - 1, 0);
		
		if (self->reload <= 0 && getDistance(self->x, self->y, player->x, player->y) < SCREEN_HEIGHT)
		{
			fireEnemyBullet();
			
			self->reload = FPS * 3;
		}
	}
}

static void die(void)
{
	addEnemyDeathEffect();
	
	if (rand() % 3)
	{
		addRandomPowerup(self->x, self->y);
	}
	
	stage.score += 10;
	
	playSound(SND_ENEMY_DIE, -1);
}
