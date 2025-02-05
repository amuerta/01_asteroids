#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <assert.h>

// GAME REQUIREMENTS:
//
// + Asteroids that spawn off screen
// + Asteroids off screen
// + Player
//  - there are 2 player selected ships: gunship and lazership
// 	- move
// 	- shoot
// 	- gun has recoil but bigger damage
// 	- lazer has no recoil but is less effective
// 	- make player not get lost (wrap around screen edges)
// 	- player has speed break on cooldown
// + GUI (Graphical User Interface)
//  - score counter in the middle of the screen
// 	- start game screen
// 	- loose screen
// 	- statistics


#include "defs.h"
#include "pool.c"
#include "entity.c"
#include "game.c"

GameAssets Game_load_assets(const char* asset_path) {
	GameAssets assets = {0};
	char scratch_buffer[256] = {0};
	

	if(!DirectoryExists(asset_path))
		return assets;
	
	sprintf(scratch_buffer,"%s%s", asset_path, "gun.mp3");
	if(FileExists(scratch_buffer))
		assets.gunfire 	= LoadSound(scratch_buffer);

	sprintf(scratch_buffer,"%s%s", asset_path, "engine.mp3");
	if(FileExists(scratch_buffer))
		assets.engine	= LoadSound(scratch_buffer);

	sprintf(scratch_buffer,"%s%s", asset_path, "warp.mp3");
	if(FileExists(scratch_buffer))
		assets.warpspawn = LoadSound(scratch_buffer);

	sprintf(scratch_buffer,"%s%s", asset_path, "shipbreak.mp3");
	if(FileExists(scratch_buffer))
		assets.shipbreak = LoadSound(scratch_buffer);

#if 0 // UNUSED
	sprintf(scratch_buffer,"%s%s", asset_path, "stop.mp3");
	assets.speedbreak = LoadSound(scratch_buffer);
#endif

	if(FileExists(scratch_buffer))
		sprintf(scratch_buffer,"%s%s", asset_path, "rock.mp3");
	assets.rockbreak = LoadSound(scratch_buffer);

	return assets;
}

void Game_unload_assets(Game g) {
	GameAssets assets  = g.assets;
	UnloadSound(assets.gunfire);
	UnloadSound(assets.engine);
	UnloadSound(assets.warpspawn);
	UnloadSound(assets.shipbreak);
	//UnloadSound(assets.speed_break); // UNUSED
	UnloadSound(assets.rockbreak);
}

int main(void) {
	SetTargetFPS(60);
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	
	Vector2 window_size = {800,600};


	InitWindow(UnfoldVec2(window_size),"Asteroids");
	InitAudioDevice();
	SetMasterVolume(0.5);

	Game asteroids = {

		.ents = {
			.count = 0,
		},

		.arena = {
			.x = -(window_size.x/2) * CAMERA_ZOOM_LEVEL,
			.y = -(window_size.y/2) * CAMERA_ZOOM_LEVEL,
			.w = window_size.x		* CAMERA_ZOOM_LEVEL,
			.h = window_size.y		* CAMERA_ZOOM_LEVEL,
		},

		.player = {
			.position = {0,0},
			.size = {100,100},
			.model = {
				.model_size = {100,150}
			}
		},

		.schemes = { 
			{
				.up = KEY_W, .down = KEY_S,
				.left = KEY_A, .right = KEY_D,
				.shoot = KEY_UP, .bigshoot = KEY_LEFT_ALT,
				.speed_break = KEY_LEFT_SHIFT
			}, 
		},

		.camera = {
			.offset = {800/2,600/2},
			.target = {0,0},
			.zoom = (1.0/CAMERA_ZOOM_LEVEL),
		},

		.assets = Game_load_assets("./assets/"),
	
		.ui = {
			{350,0,100,33},
			{0,200,100,800},
			0,0
		},

		.flags = {
			.show_fps = false,
			false // disable debug
		},
	};

	while(!WindowShouldClose()) {
		Game_update(&asteroids);
		BeginDrawing();
		Game_draw(asteroids);
		Game_debug(asteroids);
		EndDrawing();
	}

	loop(i,MAX_SOUNDS) {
		Sound* sound_buffer = asteroids.sound_buffer;
		UnloadSoundAlias(sound_buffer[i]);
	}

	Game_unload_assets(asteroids);
	CloseAudioDevice();
	CloseWindow();
}
