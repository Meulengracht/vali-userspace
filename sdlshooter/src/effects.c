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

#include "effects.h"

static SDL_Texture *whiteSquare16;
static SDL_Texture *whiteSquare8;

void initEffects(void)
{
	whiteSquare16 = loadTexture("gfx/whiteSquare16.png");
	whiteSquare8 = loadTexture("gfx/whiteSquare8.png");
}

void doEffects(void)
{
	Effect *e, *prev;
	
	prev = &stage.effectHead;
	
	for (e = stage.effectHead.next ; e != NULL ; e = e->next)
	{
		e->x += e->dx;
		e->y += e->dy;
		
		e->color.a = MAX(e->color.a - 1, 0);
		
		if (--e->life <= 0)
		{
			if (e == stage.effectTail)
			{
				stage.effectTail = prev;
			}
			
			prev->next = e->next;
			free(e);
			e = prev;
		}
		
		prev = e;
	}
}

void drawEffects(void)
{
	Effect *e;
	
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_ADD);
	
	for (e = stage.effectHead.next ; e != NULL ; e = e->next)
	{
		SDL_SetTextureBlendMode(e->texture, SDL_BLENDMODE_ADD);
		
		SDL_SetTextureColorMod(e->texture, e->color.r, e->color.g, e->color.b);
		SDL_SetTextureAlphaMod(e->texture, e->color.a);
		
		blit(e->texture, e->x - stage.camera.x, e->y - stage.camera.y, 1);
	}
	
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
}

void addEnemyDeathEffect(void)
{
	Effect *e;
	int i;
	
	for (i = 0 ; i < 128 ; i++)
	{
		e = malloc(sizeof(Effect));
		memset(e, 0, sizeof(Effect));
		stage.effectTail->next = e;
		stage.effectTail = e;
		
		e->x = self->x;
		e->y = self->y;
		e->dx = 400 - rand() % 800;
		e->dy = 400 - rand() % 800;
		
		e->dx /= 100;
		e->dy /= 100;
		
		e->texture = whiteSquare16;
		
		e->color.r = 255;
		e->color.g = 128 + rand() % 128;
		e->color.b = 0;
		e->color.a = rand() % 255;
		
		e->life = rand() % FPS;
	}
}

void addDonkDeathEffect(void)
{
	Effect *e;
	int i;
	
	for (i = 0 ; i < 128 ; i++)
	{
		e = malloc(sizeof(Effect));
		memset(e, 0, sizeof(Effect));
		stage.effectTail->next = e;
		stage.effectTail = e;
		
		e->x = self->x;
		e->y = self->y;
		e->dx = 400 - rand() % 800;
		e->dy = 400 - rand() % 800;
		
		e->dx /= 100;
		e->dy /= 100;
		
		e->texture = whiteSquare16;
		
		e->color.r = rand() % 128;
		e->color.g = 128 + rand() % 128;
		e->color.b = 255;
		e->color.a = rand() % 255;
		
		e->life = rand() % FPS;
	}
}

void addBulletHitEffect(Entity *b)
{
	Effect *e;
	int i;
	
	for (i = 0 ; i < 16 ; i++)
	{
		e = malloc(sizeof(Effect));
		memset(e, 0, sizeof(Effect));
		stage.effectTail->next = e;
		stage.effectTail = e;
		
		e->x = b->x;
		e->y = b->y;
		e->dx = 400 - rand() % 800;
		e->dy = 400 - rand() % 800;
		
		e->dx /= 100;
		e->dy /= 100;
		
		e->texture = whiteSquare8;
		
		e->color = b->color;
		
		e->color.a = rand() % 255;
		
		e->life = rand() % FPS;
	}
}
