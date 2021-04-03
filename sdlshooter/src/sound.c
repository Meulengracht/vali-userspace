/*
Copyright (C) 2018 Parallel Realities

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

#include "sound.h"

static void loadSounds(void);

static Mix_Chunk *sounds[SND_MAX];
static Mix_Music *music;

void initSounds(void)
{
	memset(sounds, 0, sizeof(Mix_Chunk*) * SND_MAX);
	
	music = NULL;
	
	loadSounds();
}

void loadMusic(char *filename)
{
	if (music != NULL)
	{
		Mix_HaltMusic();
		Mix_FreeMusic(music);
		music = NULL;
	}

	music = Mix_LoadMUS(filename);
}

void playMusic(int loop)
{
	Mix_PlayMusic(music, (loop) ? -1 : 0);
}

void playSound(int id, int channel)
{
	Mix_PlayChannel(channel, sounds[id], 0);
}

static void loadSounds(void)
{
	sounds[SND_PISTOL] = Mix_LoadWAV("sound/Gun_44magnum-freesoundeffects.ogg");
	sounds[SND_UZI] = Mix_LoadWAV("sound/67020__ls__submachinegun2.ogg");
	sounds[SND_SHOTGUN] = Mix_LoadWAV("sound/10 Guage Shotgun-SoundBible.com-74120584.ogg");
	sounds[SND_AMMO] = Mix_LoadWAV("sound/Shotgun-SoundBible.com-862990674.ogg");
	sounds[SND_ENEMY_BULLET] = Mix_LoadWAV("sound/344809__limitsnap-creations__laserblast-effect.ogg");
	sounds[SND_POINTS] = Mix_LoadWAV("sound/342749__rhodesmas__notification-01.ogg");
	sounds[SND_ENEMY_HIT] = Mix_LoadWAV("sound/97792__cgeffex__metal-impact.ogg");
	sounds[SND_DONK_HIT] = Mix_LoadWAV("sound/163441__under7dude__man-getting-hit.ogg");
	sounds[SND_ENEMY_DIE] = Mix_LoadWAV("sound/260434__roganderrick__glass-break-medium-jar.ogg");
	sounds[SND_DONK_DIE] = Mix_LoadWAV("sound/219883__pablocandel__boy-scream.ogg");
}
