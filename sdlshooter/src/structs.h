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

typedef struct Texture Texture;
typedef struct Entity Entity;
typedef struct Effect Effect;

typedef struct {
	void (*logic)(void);
	void (*draw)(void);
} Delegate;

struct Texture {
	char name[MAX_NAME_LENGTH];
	SDL_Texture *texture;
	Texture *next;
};

typedef struct {
	int x;
	int y;
	int button[MAX_MOUSE_BUTTONS];
	int wheel;
} Mouse;

struct Effect {
	float x;
	float y;
	float dx;
	float dy;
	int life;
	SDL_Color color;
	SDL_Texture *texture;
	Effect *next;
};

struct Entity {
	int side;
	float x;
	float y;
	int w;
	int h;
	int radius;
	float dx;
	float dy;
	int health;
	int reload;
	int angle;
	int weaponType;
	SDL_Texture *texture;
	SDL_Color color;
	void (*tick)(void);
	void (*touch)(Entity *other);
	void (*die)(void);
	Entity *next;
};

typedef struct {
	SDL_Renderer *renderer;
	SDL_Window *window;
	Delegate delegate;
	int keyboard[MAX_KEYBOARD_KEYS];
	Mouse mouse;
	Texture textureHead, *textureTail;
	char inputText[MAX_LINE_LENGTH];
} App;

typedef struct {
	int score;
	Entity entityHead, *entityTail;
	Entity bulletHead, *bulletTail;
	Effect effectHead, *effectTail;
	int ammo[WPN_MAX];
	SDL_Point camera;
} Stage;

typedef struct {
	char name[MAX_SCORE_NAME_LENGTH];
	int recent;
	int score;
} Highscore;

typedef struct {
	Highscore highscore[NUM_HIGHSCORES];
} Highscores;
