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

#include "items.h"

static void tick(void);
static void pointsTick(void);
static void addHealthPowerup(int x, int y);
static void addUziPowerup(int x, int y);
static void addShotgunPowerup(int x, int y);
static void uziTouch(Entity *other);
static void shotgunTouch(Entity *other);
static void healthTouch(Entity *other);
static void pointsTouch(Entity *other);

static SDL_Texture *uziTexture;
static SDL_Texture *shotgunTexture;
static SDL_Texture *healthTexture;
static SDL_Texture *pointsTexture;

void initItems(void)
{
	uziTexture = loadTexture("gfx/uzi.png");
	shotgunTexture = loadTexture("gfx/shotgun.png");
	healthTexture = loadTexture("gfx/health.png");
	pointsTexture = loadTexture("gfx/points.png");
}

void addRandomPowerup(int x, int y)
{
	int r;
	
	r = rand() % 5;
	
	if (r == 0)
	{
		addHealthPowerup(x, y);
	}
	else if (r < 3)
	{
		addUziPowerup(x, y);
	}
	else
	{
		addShotgunPowerup(x, y);
	}
}

static Entity *createPowerup(int x, int y)
{
	Entity *e;
	
	e = malloc(sizeof(Entity));
	memset(e, 0, sizeof(Entity));
	stage.entityTail->next = e;
	stage.entityTail = e;
	
	e->x = x;
	e->y = y;
	e->health = FPS * 5;
	e->tick = tick;
	e->radius = 16;
	
	e->dx = -200 + (rand() % 400);
	e->dy = -200 + (rand() % 400);
	
	e->dx /= 100;
	e->dy /= 100;
	
	e->color.r = e->color.g = e->color.b = e->color.a = 255;
	
	return e;
}

static void tick(void)
{
	self->health--;
	
	self->dx *= 0.98;
	self->dy *= 0.98;
}

static void addHealthPowerup(int x, int y)
{
	Entity *e;
	
	e = createPowerup(x, y);
	
	e->texture = healthTexture;
	e->touch = healthTouch;
}

static void addUziPowerup(int x, int y)
{
	Entity *e;
	
	e = createPowerup(x, y);
	
	e->texture = uziTexture;
	e->touch = uziTouch;
}

static void addShotgunPowerup(int x, int y)
{
	Entity *e;
	
	e = createPowerup(x, y);
	
	e->texture = shotgunTexture;
	e->touch = shotgunTouch;
}

static void pointsTick(void)
{
	tick();
	
	self->angle += 5;
	
	while (self->angle >= 360)
	{
		self->angle -= 360;
	}
}

void addPointsPowerup(int x, int y)
{
	Entity *e;
	
	e = createPowerup(x, y);
	
	e->tick = pointsTick;
	e->health = FPS * 10;
	e->dx = e->dy = 0;
	
	e->texture = pointsTexture;
	e->touch = pointsTouch;
}

static void uziTouch(Entity *other)
{
	if (other == player)
	{
		self->health = 0;
		
		stage.ammo[WPN_UZI] += 25;
		
		playSound(SND_AMMO, CH_ITEM);
	}
}

static void shotgunTouch(Entity *other)
{
	if (other == player)
	{
		self->health = 0;
		
		stage.ammo[WPN_SHOTGUN] += 4;
		
		playSound(SND_AMMO, CH_ITEM);
	}
}

static void healthTouch(Entity *other)
{
	if (other == player)
	{
		self->health = 0;
		
		player->health++;
		
		playSound(SND_AMMO, CH_ITEM);
	}
}

static void pointsTouch(Entity *other)
{
	if (other == player)
	{
		self->health = 0;
		
		stage.score += 250;
		
		playSound(SND_POINTS, CH_ITEM);
	}
}
