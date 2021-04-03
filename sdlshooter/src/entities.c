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

#include "entities.h"

static void touchOthers(void);

void doEntities(void)
{
	Entity *e, *prev;
	
	prev = &stage.entityHead;
	
	for (e = stage.entityHead.next ; e != NULL ; e = e->next)
	{
		self = e;
		
		if (e->tick)
		{
			e->tick();
		}
		
		e->x += e->dx;
		e->y += e->dy;
		
		if (e->touch)
		{
			touchOthers();
		}
		
		e->reload = MAX(e->reload - 1, 0);
		
		if (e == player)
		{
			e->x = MIN(MAX(e->x, e->w / 2), ARENA_WIDTH - e->w / 2);
			e->y = MIN(MAX(e->y, e->h / 2), ARENA_HEIGHT - e->h / 2);
		}
		
		if (e->health <= 0)
		{
			if (e->die)
			{
				e->die();
			}
			
			if (e == stage.entityTail)
			{
				stage.entityTail = prev;
			}
			
			prev->next = e->next;
			free(e);
			e = prev;
		}
		
		prev = e;
	}
}

static void touchOthers(void)
{
	Entity *e;
	int distance;
	
	for (e = stage.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e != self)
		{
			distance = getDistance(self->x, self->y, e->x, e->y);
			
			if (distance < e->radius + self->radius)
			{
				self->touch(e);
			}
		}
	}
}

void drawEntities(void)
{
	Entity *e;
	
	for (e = stage.entityHead.next ; e != NULL ; e = e->next)
	{
		SDL_SetTextureColorMod(e->texture, e->color.r, e->color.g, e->color.b);
		
		blitRotated(e->texture, e->x - stage.camera.x, e->y - stage.camera.y, e->angle);
	}
}
