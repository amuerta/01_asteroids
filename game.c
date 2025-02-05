#include "entity.c"

bool Game_is_scheme_used(ControlScheme s) {
	return (
			IsKeyDown(s.up)			||
			IsKeyDown(s.down)		||
			IsKeyDown(s.left)		||
			IsKeyDown(s.right)		||
			IsKeyDown(s.shoot)		||
			IsKeyDown(s.bigshoot) 	||
			IsKeyDown(s.speed_break)
	);
}



void Game_sound_gun(Game* g, Sound sfx) {
	// change sound depending on tapping(firing) speed
	// change pan depending on moving speed
	Sound* 	sound_buffer = g->sound_buffer;
	size_t	current_sound = (g->current_sound);

	loop(i,MAX_SOUNDS) {
		sound_buffer[i] = LoadSoundAlias(sfx);
	}
	float sps = g->player.vals.sps;
	float deviation = GetRandomValue(-1,1) * 
		(1.0/GetRandomValue(SOUND_SHOT_DEVIATION_MIN,SOUND_SHOT_DEVIATION_MAX));
	float overload = MIN(sps/10, SOUND_SHOT_PITCH_MAX);
	
	if (IsAudioDeviceReady() && IsSoundValid(sfx)) {
		Sound snd = sound_buffer[current_sound % MAX_SOUNDS];
		SetSoundVolume(snd,SOUND_SHOT_VOLUME);
		SetSoundPitch(snd,SOUND_SHOT_PITCH_MIN + overload + deviation);                   
		if(IsSoundValid(snd)) PlaySound(snd);
		
		if (current_sound > MAX_SOUNDS)
			g->current_sound=0;
		
		g->current_sound++;
	}

}


float Game_soundsqrt_f(float x, float curve_modifier) {
	return (x*x)/curve_modifier;
}


void Game_read_player_controls(Game* g) {
	Entity* player = &(g->player);
	float rot = 0;
	Vector2 accel = {0};
	Vector2 looking_direction = Vector2Rotate(Vector2Scale(Vec2{0,-1},ACCELERATION),player->rotation);
	Vector2 recoil 			  = Vector2Rotate(Vector2Scale(Vec2{0,-1},RECOIL),player->rotation);
	Sound gunsfx = g->assets.gunfire;
	Sound enginesfx = g->assets.engine;
	float player_speed = Vector2Length(player->velocity);

	if (IsKeyPressed(KEY_F))
		g->flags.show_fps = !g->flags.show_fps;

	if (IsSoundValid(enginesfx)) {
		if (player_speed > SOUND_ENGINE_MIN && !IsSoundPlaying(enginesfx)) 
			PlaySound(enginesfx);
		else if (player_speed < SOUND_SHOT_PITCH_MIN && IsSoundPlaying(enginesfx))
			StopSound(enginesfx);
	}

	loop(i,MAX_CONTROL_SCHEMES) {
		ControlScheme scheme = g->schemes[i];

		if(IsKeyPressed(scheme.shoot)) {
			Entity_spawn_projectile(g);
			accel = Vector2Subtract(accel,recoil);
			if (IsSoundValid(gunsfx))
				Game_sound_gun(g,gunsfx);

			player->vals.sps += 1.0;
			
		}
		if (player->vals.sps <= 0)
			player->vals.sps = 0;
		player->vals.sps -= GetFrameTime()*3.5;


		// some magic numbers, but im ok with it.
		float engine_vol = Game_soundsqrt_f(player_speed,1.5)/(32*32);
		SetSoundVolume(enginesfx,engine_vol*SOUND_ENGINE_VOLUME);                   
		SetSoundPitch(enginesfx,1-engine_vol/2);                   

		if(IsKeyDown(scheme.up)) {
			accel = Vector2Add(accel, looking_direction);
		}
		if(IsKeyDown(scheme.down))
			accel = Vector2Scale(Vector2Subtract(accel, looking_direction),0.33);

#if 0 // This one is pretty lame, and obfuscates already good game, 
	  // i might add if i ever feel like to..
		if (IsKeyPressed(scheme.speed_break) && break_cooldown == 0) {
			player->velocity = Vector2Scale(player->velocity,SPEED_BREAK_FACTOR);
			SetSoundPitch(breaksfx,1.5);
			PlaySound(breaksfx);
		}
#endif

		if(IsKeyDown(scheme.left)) 
			rot -= (ROTATION_SPEED/360.0);
		if(IsKeyDown(scheme.right))
			rot += (ROTATION_SPEED/360.0);
	
		if (Game_is_scheme_used(scheme)) {
			break;
		}
	}
	player->acceleration = accel;
	player->rotation += rot;
}



uint Game_spawn_count_f(uint x,uint wave_size,uint fall_off) {
	assert(wave_size != 0 && "FACTOR CANNOT BE 0");
	uint f = wave_size-((x*x)/pow(wave_size,fall_off));
	return (f < 1000000) ? f : 1;
}

#define ASTEROID_THROW_ACCELERATION 32


void Game_cleanup_all(Game* g) {
	EntityPool* pool = &(g->ents);
	loop(i,pool->max_size) {
		Entity* ent = EntityPool_refer(pool,i);
		if (!ent) continue;
		
		ent->velocity = Vector2Subtract(ent->velocity,
				Vec2
				{0,ASTEROID_THROW_ACCELERATION});
		ent->position = Vector2Add(ent->position,ent->velocity);
	}
}

void Game_spawn_automatic_asteroids(Game* g) {
	EntityPool* pool = &(g->ents);
	size_t asteroid_count = 0;
	loop(i,pool->max_size) {
		Entity* ent = EntityPool_refer(pool,i);
		if (ent && ent->kind == EntityKind_asteroid) {
			asteroid_count++;
		}
	}

	bool spawn_wave = g->tick > 1 && (g->tick % SPAWN_INTERVAL_WAVE == 1); 
	bool spawn_singe = g->tick > 1 && (g->tick % SPAWN_INTERVAL_SINGLE == 1);
	uint spawn_count = Game_spawn_count_f(asteroid_count, 3, 4);
	
	if (spawn_wave && asteroid_count < MAX_ASTEROID_SPAWN_COUNT) {
		debug("SPAWN ASTEROID: spawn_wave(%u), total_count(%u)\n",
				spawn_count,
				asteroid_count);
		loop(i,spawn_count) {
			Entity_spawn_random_asteroid(g);
		}
	}
	
	if (spawn_singe) 
			Entity_spawn_random_asteroid(g);

}

void Game_wrap_around(Game* g) {
	Rectangle arena_rec = g->arena;//RectangleScale(g->arena,1/g->camera.zoom);
								   //
	if (!CheckCollisionPointRec(g->player.position,arena_rec)) {
		g->player.position = Vector2Multiply(g->player.position,Vec2 {-1,-1});
	}
}

uint Entity_asteroid_damage(EntityPool* poolptr, Entity* other_ent, Relptr c) {
	uint value = 0;
	other_ent->vals.hp -= 1.0;
	if (other_ent->vals.hp <= 0) {
		switch(other_ent->vals.asteroid_class) {
			case Asteroid_medium:
			case Asteroid_big:
				Entity_spawn_asteroid_split(poolptr,*other_ent);
				value = other_ent->vals.asteroid_class;
				break;
			default: 
				break;
		}
		*other_ent = Ent {0};
		EntityPool_release(poolptr,c);
	}
	return value; // no dead asteroid => no value
}


void Game_update_all_npc_entites(Game* g) {
	EntityPool* poolptr = &(g->ents);
	Entity player = g->player;
	Rectangle player_bb = {
		player.position.x - player.size.x/2,
		player.position.y - player.size.y/2,
		player.size.x,
		player.size.y,
	};

	Sound rocksfx = g->assets.rockbreak;

	Rectangle avilable_arena = {
		.x = g->arena.x - ARENA_OFFSCREEN_AREA_SIZE,
		.y = g->arena.y - ARENA_OFFSCREEN_AREA_SIZE,
		.w = g->arena.w + ARENA_OFFSCREEN_AREA_SIZE*2,
		.h = g->arena.h + ARENA_OFFSCREEN_AREA_SIZE*2
	};

	loop(i,poolptr->max_size) {
		Entity* ent = EntityPool_refer(poolptr,i);
		if (!ent)
			continue;

		Vector2 model_size 	= ent->model.model_size;
		float asteroid_size = MAX(model_size.x,model_size.y);
		Entity_update_velocity(ent);

		if (!CheckCollisionPointRec(ent->position,avilable_arena)) {
			*ent = Ent {0};
			EntityPool_release(poolptr,i);
		}

		switch(ent->kind) {
			case EntityKind_projectile: 
				{
					loop(c,poolptr->max_size) {
						Entity* other_ent = EntityPool_refer(poolptr,c);
						if (c==i || !other_ent) 
							continue;
						
						if(Entity_bullet_colliding_asteroid(*ent,*other_ent)) {
							// bullet
							*ent = Ent {0};
							EntityPool_release(poolptr,i);
							// asteroid
							other_ent->vals.damage_time = 1.0;
							uint scored = Entity_asteroid_damage(poolptr,other_ent, c);
							if (scored && IsSoundValid(rocksfx)) {
								SetSoundPitch(rocksfx,0.8);
								SetSoundVolume(rocksfx,SOUND_ASTEROID_VOLUME_FACTOR*2);
								PlaySound(rocksfx);
								g->player.vals.score += scored;
							} else {
								if (IsSoundValid(rocksfx)) {
									SetSoundVolume(rocksfx,SOUND_ASTEROID_VOLUME_FACTOR*1);
									PlaySound(rocksfx);
								}
							}
						}
					}
				} break;

			case EntityKind_asteroid: 
				{
					if (ent->vals.damage_time < 0.0)
						ent->vals.damage_time = 0.0;
					else
						ent->vals.damage_time -= GetFrameTime();

					bool collision = CheckCollisionCircleRec(ent->position,asteroid_size,player_bb);
					if (collision && g->state != Game_over) {
						g->player.vals.gothit = true;
					}
				} break;

			default: break;
		}
	}
}




void Game_start_animation(Game g) {
	Entity player = g.player;
	Rectangle coverage = {
		0 - player.size.x,
		0 - player.size.y,
		player.size.x*2,
		player.size.y*2,
	};
	
	if (g.state == Game_idle || g.state == Game_spawning || g.spawn_time != 0) {
		DrawRectangleRec(coverage,BACKGROUND_COLOR);
	}
	const Vector2 middlepoint = {
		0, 0 - player.size.y
	};

	float portal_len = g.spawn_time*100;
	Vector2 
		rightpoint = {
			middlepoint.x-portal_len, 
			middlepoint.y
		},
		leftpoint = {
			   middlepoint.x+portal_len, 
			   middlepoint.y
		};


	DrawLineEx(middlepoint,leftpoint,20,WHITE);
	DrawLineEx(middlepoint,rightpoint,20,WHITE);
}



float Game_easeoutexpo(float x) {
	return ((x == 1) ? 1 : 1) - pow(2, -10 * x);
}


void Game_end_animation(Game g) {
	float explosion_size = (Game_easeoutexpo(g.death_time))*100;
	Color explosion_color = SHIP_EXPLOSION_COLOR;
	explosion_color.a = (ANIMATION_DEATH_LENGTH-g.death_time) * 255;
	DrawCircleV(g.player.position, explosion_size, explosion_color);
}


void Game_update(Game* g) {
	Entity* p_ptr = &(g->player);

	if (IsKeyPressed(KEY_MINUS))
		g->state = Game_over;

	if (IsKeyPressed(KEY_SLASH)) {
		g->flags.show_entity_hitbox = !g->flags.show_entity_hitbox;
	} 

	switch (g->state) {
		
		case Game_idle:
			{

				g->ui.menu_text_transperency = 
					((g->ui.menu_text_transperency < 1.0)? 
					  g->ui.menu_text_transperency + GetFrameTime() : 0) ;

				g->player.position = Vec2 {0};
				g->player.velocity = Vec2 {0};
				g->player.rotation = 0;
				g->player.vals.gothit = false;

				g->death_time = 0;
				if (IsKeyPressed(KEY_SPACE))
					g->state = Game_spawning;
				// hide gui with a swipe off the screen
			} break;

		case Game_spawning:
			{
				g->ui.menu_text_transperency = ANIMATION_SPAWN_LENGTH/g->spawn_time;
				g->spawn_time += GetFrameTime();
				if (g->spawn_time > ANIMATION_SPAWN_LENGTH/2)
					g->player.velocity = Vec2{0,-STARTING_VELOCITY};

				if (g->spawn_time > ANIMATION_SPAWN_LENGTH) {
					g->state = Game_going;
				}

				if (g->spawn_time > SOUND_SPAWN_TIMING && !IsSoundPlaying(g->assets.warpspawn))
					if(IsSoundValid(g->assets.warpspawn)) PlaySound(g->assets.warpspawn);

			} break;

		case Game_going: 
			{

				if ( g->spawn_time > PORTAL_DECAY_FACTOR) {
					g->spawn_time *= 1.0-PORTAL_DECAY_FACTOR;
				} else 
					g->spawn_time = 0;

				if (g->player.vals.gothit) {
					g->state = Game_over;
					SetSoundPitch(g->assets.shipbreak,0.75);
					if(IsSoundValid(g->assets.shipbreak)) PlaySound(g->assets.shipbreak);
				}

				Game_wrap_around(g);
				Game_read_player_controls(g);
				Entity_update_acceleration(p_ptr);
				Entity_update_velocity(p_ptr);
				Entity_apply_drag(p_ptr);
				Game_spawn_automatic_asteroids(g);
				Game_update_all_npc_entites(g);
			} break;

		case Game_over: 
			{
				StopSound(g->assets.engine);
				g->death_time += GetFrameTime();
				if (g->death_time > ANIMATION_DEATH_LENGTH) {
					if (g->highest_score < g->player.vals.score)
						g->highest_score = g->player.vals.score ;
					g->state = Game_idle;
					g->player.vals.gothit = false;
					g->player.vals.score = 0;
				}
			

				if (g->death_time > ANIMATION_DEATH_CLEANSE_TIMING)
					Game_cleanup_all(g);
			} break;
	}

	g->tick++;
}

// TODO: move to ui.c

#define FONT_FLASH_DURATION 1
#define SCOREBOARD_FONT_GROW_SIZE 6

void Game_draw_fancy_fps(Vector2 pos, float fnt_size, Color clr) {
	DrawText(TextFormat("%.0f",1/GetFrameTime()),UnfoldVec2(pos),fnt_size,clr);
}


void Game_draw_menutext(UiBuilder b, uint highest_score) {
	const char* first_text = "[WASD] [SPACE] [ARROW] [F] [?]";
	const char* text = (highest_score) ? TextFormat("[BP]: %u",highest_score) : first_text;
	size_t text_sz = 127;
	size_t text_len = MeasureText(text,text_sz);
	
	Rectangle rec = b.menu_text;
	

	Color fnt_color,
		  fg_color = ColorLerp(SCOREBOARD_BG_COLOR,SCOREBOARD_FG_COLOR,0.1);

	Vector2 text_position = {
		.x = rec.x+(rec.w-text_len)/2,
		.y = rec.y+(rec.h-text_sz)/2
	};
	
	fnt_color = fg_color;
	fnt_color.a = 255 * b.menu_text_transperency;
	DrawText(text,
			UnfoldVec2(text_position),
			text_sz,
			fnt_color);

}

void Game_draw_scoreboard(Entity player, UiBuilder b) {
	static uint prev_score = 0;
	static float flash_timer = 0;
	Rectangle rec = b.scoreboard_box;

	Color fnt_color,
		  bg_color = SCOREBOARD_BG_COLOR,
		  fg_color = SCOREBOARD_FG_COLOR;

	bg_color.a = 255*b.scoreboard_transperency;
	fg_color.a = 255*b.scoreboard_transperency;

	DrawRectangleRec(rec,bg_color);
	DrawRectangleLinesEx(rec,1,fg_color);

	const char* score = TextFormat("%u",player.vals.score);
	size_t text_sz = SCOREBOARD_FONT_SIZE + (SCOREBOARD_FONT_GROW_SIZE * flash_timer);
	size_t text_len = MeasureText(score,SCOREBOARD_FONT_SIZE);

	if (prev_score != player.vals.score) {
		if (player.vals.score == 0)
			flash_timer = FONT_FLASH_DURATION * 2;
		else
			flash_timer = FONT_FLASH_DURATION;
	}

	fnt_color = ColorLerp(SCOREBOARD_FG_COLOR,WHITE,flash_timer);
	fnt_color.a = 255*b.scoreboard_transperency;

	Vector2 text_position = {
		.x = rec.x+(rec.w-text_len)/2,
		.y = rec.y+(rec.h-text_sz)/2
	};
	DrawText(score,
			UnfoldVec2(text_position),
			text_sz,
			fnt_color);
	
	if (flash_timer > 0.0)
		flash_timer -= GetFrameTime();
	else 
		flash_timer = 0.0;

	prev_score = player.vals.score;
}

void Game_draw(Game g) {
	ClearBackground(BACKGROUND_COLOR);
	
	if (g.flags.show_fps)
		Game_draw_fancy_fps(Vec2{5,5},10,SCOREBOARD_FG_COLOR);
	//DrawFPS(0,0);
	BeginMode2D(g.camera);
	long* tick_ptr = &(g.tick);
	
	if (g.state != Game_over && g.state != Game_idle)
		Entity_draw(g.player,tick_ptr);
	
	switch (g.state) {
		case Game_idle:
		case Game_spawning:
		case Game_going:
			{
				if (g.state!=Game_going) {
					Game_draw_menutext(g.ui,g.highest_score);
					g.ui.scoreboard_transperency = g.spawn_time;
				}
				else
					g.ui.scoreboard_transperency = 1.0;
				
				Game_start_animation(g);
			}
			break;
		case Game_over: 
			{
				Game_end_animation(g);
				g.ui.scoreboard_transperency = g.spawn_time;
			}
			break;
		default: break;
	}

	Entity_draw_all_npc_entites(g);
	
	EndMode2D();
	Game_draw_scoreboard(g.player,g.ui);
}

void Game_debug(Game g) {
	Entity player = g.player;
	Rectangle arena_rec = g.arena;
	Rectangle player_bb = {
		player.position.x - player.size.x/2,
		player.position.y - player.size.y/2,
		player.size.x,
		player.size.y,
	};
	short velocity_len = (short) round(Vector2Length(g.player.velocity));
	
	if (g.flags.show_entity_hitbox) {
	
		BeginMode2D(g.camera);
		Entity_draw_player_debug(player);
		DrawRectangleLinesEx(player_bb,CAMERA_ZOOM_LEVEL,GREEN);
		EndMode2D();
		//DrawRectangleRec(arena_rec,);
		DrawText("Asteroids DEBUG MODE", 10,FONT_SIZE,FONT_SIZE,GREEN);
		DrawText(TextFormat("player within arena %s", 
					CheckCollisionPointRec(g.player.position,arena_rec) ? "true" : "false"), 10,FONT_SIZE*2,FONT_SIZE,GREEN);
		DrawText(TextFormat("count of the entites: %lu",g.ents.count),10,FONT_SIZE*3,FONT_SIZE,GREEN);
		DrawText(TextFormat("CPS: %f",g.player.vals.sps),10,FONT_SIZE*4,FONT_SIZE,GREEN);
		DrawText(TextFormat("Player Velocity Length: [%d]\n", velocity_len), 10, FONT_SIZE*5,FONT_SIZE,GREEN);

	}

	if (g.tick % 60 == 1) {
		Entity p = g.player;
		IGNORE p; // compiler thinks its unused, humble use bellow
		debug("Player Velocity: [%f,%f]\n", p.velocity.x,p.velocity.y);
		debug("Player Velocity Length: [%d]\n", velocity_len);
	}
}

