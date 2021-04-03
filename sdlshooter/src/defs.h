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

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define STRNCPY(dest, src, n) strncpy(dest, src, n); dest[n - 1] = '\0'
#define PI 3.14159265358979323846

#define SCREEN_WIDTH   1280
#define SCREEN_HEIGHT  720

#define ARENA_WIDTH  (SCREEN_WIDTH * 5)
#define ARENA_HEIGHT (SCREEN_HEIGHT * 5)

#define MAX_NAME_LENGTH        32
#define MAX_LINE_LENGTH        1024
#define MAX_SCORE_NAME_LENGTH  16

#define FPS 60

#define GRID_SIZE 64

#define GRID_RENDER_WIDTH  (SCREEN_WIDTH / GRID_SIZE)
#define GRID_RENDER_HEIGHT ((SCREEN_HEIGHT / GRID_SIZE) + 1)

#define MAX_KEYBOARD_KEYS 350
#define MAX_MOUSE_BUTTONS	6

#define MAX_SND_CHANNELS 16

#define NUM_HIGHSCORES 8

#define PLAYER_SPEED 6

#define GLYPH_WIDTH  18
#define GLYPH_HEIGHT 29

enum
{
	TEXT_LEFT,
	TEXT_CENTER,
	TEXT_RIGHT
};

enum
{
	WPN_PISTOL,
	WPN_UZI,
	WPN_SHOTGUN,
	WPN_MAX
};

enum
{
	SIDE_NONE,
	SIDE_PLAYER,
	SIDE_ENEMY
};

enum
{
	SND_PISTOL,
	SND_UZI,
	SND_SHOTGUN,
	SND_ENEMY_BULLET,
	SND_AMMO,
	SND_POINTS,
	SND_ENEMY_HIT,
	SND_DONK_HIT,
	SND_ENEMY_DIE,
	SND_DONK_DIE,
	SND_MAX
};

enum
{
	CH_ANY = -1,
	CH_DONK,
	CH_ITEM,
	CH_ENEMY_BULLET,
	CH_HIT
};
