#ifndef __GAME_ENTITY_H
#define __GAME_ENTITY_H

#include "defs.h"
#include "pool.c"

void Entity_update_acceleration(Entity* e) {
	e->velocity = Vector2Add(e->velocity,e->acceleration);
}

void Entity_update_velocity(Entity* e) {
	e->position = Vector2Add(e->position,e->velocity);
}



void Entity_spawn_asteroid_split(EntityPool* poolptr, Entity father) {
	Vector2 r_force = Vector2Normalize(father.velocity), 
			l_force = Vector2Normalize(father.velocity);

	r_force = Vector2Rotate(r_force,GetRandomValue(0,MAX_RANDOM_ROTATION)*DEG2RAD);
	l_force = Vector2Rotate(r_force,GetRandomValue(-MAX_RANDOM_ROTATION,0)*DEG2RAD);

	float rand_vel_mult = 1.0 + (1.0/(float)GetRandomValue(7,10));

	r_force = Vector2Scale(r_force,
			Vector2Length(father.velocity) * rand_vel_mult
			);
	l_force = Vector2Scale(l_force,
			Vector2Length(father.velocity) * rand_vel_mult
			);

	Vector2 spawn_positions	[2] = {
		father.position,
		father.position,
	};
	Vector2 spawn_velocities[2] = {
		r_force,
		l_force,
	};

	AsteroidSize size = AsteroidSize_small;
	AsteroidType type = Asteroid_small;
	int hp = 1;

	switch (father.vals.asteroid_class) {
		case Asteroid_big:
			{ 
				size = AsteroidSize_medium; 
				type = Asteroid_medium; 
				hp = 2;
			} break;
		case Asteroid_medium:
			{ 
				size = AsteroidSize_small; 
				type = Asteroid_small; 
				hp = 1;
			} break;
		default: break;
	}

	Relptr id_children_1 = EntityPool_reserve(poolptr);
	Relptr id_children_2 = EntityPool_reserve(poolptr);
	assert( id_children_1 != RELPTR_INVALID &&  
			id_children_2 != RELPTR_INVALID &&  
			"Overflow the entity count");
	Entity* child1 = EntityPool_refer(poolptr,id_children_1);
	Entity* child2 = EntityPool_refer(poolptr,id_children_2);

	if (child1 && child2) {
		*child1 = Ent {
			.position = spawn_positions[0],
				.velocity = spawn_velocities[0],
				.size 		= {size},
				.kind = EntityKind_asteroid,
				.model = {
					.model_size = {size},
					.model_seed = GetRandomValue(0,65556),
				},
				.vals = {
					.hp = hp,
					.asteroid_class = type
				}
		};
		*child2 = Ent {
			.position = spawn_positions[1],
				.velocity = spawn_velocities[1],
				.size 		= {size},
				.kind = EntityKind_asteroid,
				.model = {
					.model_size = {size},
					.model_seed = GetRandomValue(0,65556),
				},
				.vals = {
					.hp = hp,
					.asteroid_class = type
				}
		};
	}
}

void Entity_spawn_random_asteroid(Game *g) {
	EntityPool* poolptr = &(g->ents);

	Vector2 spawn_position;
	Vector2 spawn_velocity;

	AsteroidSpawnSide side = GetRandomValue
		(AsteroidSpawnSide_left, AsteroidSpawnSide_bottom);
	float max_side_offset = 
		(side >= AsteroidSpawnSide_left && side < AsteroidSpawnSide_top)?
		g->arena.w : g->arena.h;
	float side_offset = GetRandomValue(0,max_side_offset);


	Rectangle a = {
		.x = g->arena.x - ARENA_OFFSCREEN_AREA_SIZE,
		.y = g->arena.y - ARENA_OFFSCREEN_AREA_SIZE,
		.w = g->arena.w + ARENA_OFFSCREEN_AREA_SIZE*2,
		.h = g->arena.h + ARENA_OFFSCREEN_AREA_SIZE*2
	};

	const Vector2 arena_side_table[4] = {
		 { a.x		, a.y	 	}, 
		 { a.x+a.w	, a.y		}, 
		 { a.x		, a.y	 	}, 
		 { a.x		, a.y+a.h 	}, 
	};

	spawn_position = arena_side_table[side-1];

	switch(side) {
		case AsteroidSpawnSide_left: 
		case AsteroidSpawnSide_right: 
			{
				spawn_position.y += side_offset;
			} break;
		case AsteroidSpawnSide_top: 
		case AsteroidSpawnSide_bottom: 
			{
				spawn_position.x += side_offset;
			} break;
	}
	

	// No small or medium asteroids can spawn
	const float asteroid_size = 
		AsteroidSize_big + GetRandomValue(
				-ASTEROID_SIZE_DEVIATION,
				ASTEROID_SIZE_DEVIATION);
	// spawn asteroids with only half max speed
	const float asteroid_speed = GetRandomValue(
			MIN_ASTEROID_VELOCITY,
			MAX_ASTEROID_VELOCITY*0.5);

	spawn_velocity = Vector2Subtract(g->player.position,spawn_position);
	spawn_velocity = Vector2Scale(Vector2Normalize(spawn_velocity),asteroid_speed);

	Relptr spawn_id = EntityPool_reserve(poolptr);
	assert(spawn_id != RELPTR_INVALID && "Overflow the entity count");
	Entity* spawn 	= EntityPool_refer(poolptr,spawn_id);


	if (spawn) {
		*spawn = Ent {
			.position = spawn_position,
				.velocity = spawn_velocity,
				.size 		= {asteroid_size},
				.kind = EntityKind_asteroid,
				.model = {
					.model_size = {asteroid_size},
					.model_seed = GetRandomValue(0,65556),
				},
				.vals = {
					.hp = MAX_ASTEROID_HP,
					.asteroid_class = Asteroid_big
				}
		};
	}

}

void Entity_spawn_projectile(Game* g) {
	EntityPool* poolptr = &(g->ents);
	Relptr proj_id = EntityPool_reserve(poolptr);
	assert(proj_id != RELPTR_INVALID && "Overflow the entity count");

	Entity e = g->player;
	Vector2 size = e.model.model_size;
	Vector2 looking_direction = Vector2Rotate(Vec2{0,-1},e.rotation);
	Vector2 model_position = 
		Vector2Add(e.position,
				Vector2Scale(looking_direction,-size.y/(2 + OFFSET)));
	looking_direction = Vector2Scale(looking_direction, size.y);
	looking_direction = Vector2Add(looking_direction,model_position);

	Vector2 proj_velocity = Vector2Scale(Vector2Rotate(Vec2{0,-1},e.rotation),PROJECTILE_VELOCITY);

	Entity* proj_ent = EntityPool_refer(poolptr,proj_id);
	*proj_ent = Ent {
		.position = looking_direction,
		.velocity = proj_velocity,
		.kind	  = EntityKind_projectile,
	};

	debug("Spawned Projectile: { pos: [%.1f,%.1f], vel: [%.1f,%.1f] }\n",
			UnfoldVec2(looking_direction),
			UnfoldVec2(proj_velocity)
	);
}


void Entity_draw_projectile(Entity ent) {
	//DrawCircleV(ent.position,PROJECTILE_SIZE,RED);
	Vector2 bullet[2] = {
		ent.position,
		Vector2Add(ent.position,ent.velocity)
	};
	DrawLineEx(bullet[0], bullet[1], PROJECTILE_SIZE, PROJECTILE_COLOR);
}

// Circle with N slices, where N < MAX_N
void Entity_draw_asteroid(Entity ent) {
	assert(MAX_ASTEROIDS_VERTECIES > MIN_ASTEROIDS_VERTECIES && "THIS IS BAD");
	
	int diff = MAX_ASTEROIDS_VERTECIES - MIN_ASTEROIDS_VERTECIES;
	const int seed = ent.model.model_seed;
	int vert_count = MIN_ASTEROIDS_VERTECIES + ((seed)% diff);
	int offset_sign = (((seed/7)*33) > MAX_ASTEROID_VERTICE_OFFSET) ? 1 : -1;
	int offset_value = offset_sign * (((seed/7)*33) % MAX_ASTEROID_VERTICE_OFFSET);

	Color clr = ColorLerp(ASTEROID_COLOR,WHITE,ent.vals.damage_time);
	// TODO: make offset depend on the asteroid size

	bool draw_damage = 
		ent.vals.asteroid_class != Asteroid_small ||
		ent.vals.asteroid_class == Asteroid_null;

	const Vector2 UP = {0,-1};
	Vector2 first_vertice;
	Vector2 model_size = ent.model.model_size;
	float angle_deg = (float)(360 / vert_count);
	float asteroid_size = MAX(model_size.x,model_size.y);

	//DrawText(TextFormat("Vert count: %i",vert_count),10,50,100,GREEN);
	
	// show cracks on the asteroid when they have low hp
	int hp = ent.vals.hp;
	int damage_line_count = (MAX_ASTEROID_HP-hp);
	
	for(int i = 0; i < vert_count; i++) {
		int inext= ((i+1) % vert_count);
		int innext = ((i+3) % vert_count); 

		Vector2 model_vert = Vector2Add(
				ent.position,
				Vector2Scale(
					Vector2Rotate(UP,(angle_deg*i)*DEG2RAD),
					offset_value + asteroid_size
					)
				);

		if (i == 0) {
			first_vertice = model_vert;
		}

		offset_sign = (((offset_sign/7)*33) > MAX_ASTEROID_VERTICE_OFFSET) ? 1 : -1;
		offset_value = offset_sign * (((offset_value/7)*33) % MAX_ASTEROID_VERTICE_OFFSET);
		
		Vector2 model_vert_nnext = (!innext) ? first_vertice :
			Vector2Add(
					ent.position,
					Vector2Scale(
						Vector2Rotate(UP,(angle_deg*innext)*DEG2RAD),
						offset_value + asteroid_size
						)
					);
		Vector2 model_vert_next = (!inext) ? first_vertice :
			Vector2Add(
				ent.position,
				Vector2Scale(
					Vector2Rotate(UP,(angle_deg*inext)*DEG2RAD),
					offset_value + asteroid_size
					)
				);
	
		// show cracks on the asteroid when they have low hp
		// dont do that for small asteroids to remove clutter
		if (draw_damage && damage_line_count > 0) {
			damage_line_count--;
			DrawLineEx(model_vert,model_vert_nnext,5,clr);
		}


		DrawLineEx(model_vert,model_vert_next,10,clr);
	}
	
}


void Entity_draw_asteroid_hitbox(Entity e) {
	Vector2 model_size = e.model.model_size;
	float asteroid_size = MAX(model_size.x,model_size.y);
	DrawCircleLinesV(e.position, asteroid_size, RED);                                  // Draw circle outline (Vector version)
	DrawCircleV(e.position,PROJECTILE_SIZE,RED);
}

void Entity_draw_all_npc_entites(Game g) {
	EntityPool* poolptr = &(g.ents);
	loop(i,poolptr->max_size) {
		Entity* ent = EntityPool_refer(poolptr,i);
		if (ent) switch (ent->kind) {
			case EntityKind_projectile:
				{
					Entity_draw_projectile(*ent);
				}
				break;
			case EntityKind_asteroid:
				{
					Entity_draw_asteroid(*ent);
					if (g.flags.show_entity_hitbox) {
						Entity_draw_asteroid_hitbox(*ent);
					}
				} break;
			default:
				break;
		}
	}
	
}

bool Entity_bullet_colliding_asteroid(Entity ent,Entity other) {
	Vector2 other_size 	= other.size;
	float other_bounding_size = MAX(other_size.x,other_size.y);

	Vector2 bullet[2] = {
		ent.position,
		Vector2Add(ent.position,ent.velocity)
	};

	Vector2 bullet_middle = Vector2Add(ent.position,
			Vector2Scale(ent.velocity,0.5));

	return
		(ent.kind == EntityKind_projectile) && 
		(other.kind == EntityKind_asteroid) &&
		(
		 CheckCollisionPointCircle
		 (bullet_middle,other.position,other_bounding_size) || 

		 CheckCollisionPointCircle
		 (bullet[0],other.position,other_bounding_size) || 

		 CheckCollisionPointCircle
		 (bullet[1],other.position,other_bounding_size) ||

		 CheckCollisionCircleLine(
			 other.position,other_bounding_size, 
			 bullet[0], bullet[1]
			 )
		);

}




void Entity_draw_player_engine(Entity e, long tick) {
	short velocity_len = (short) round(Vector2Length(e.velocity)) / BLINK_RATE;
	Vector2 size = e.model.model_size;
	Vector2 looking_direction = Vector2Rotate(Vec2{0,-1},e.rotation);
	Vector2 engine_origin = 
		Vector2Add(e.position,
				Vector2Scale(looking_direction,-size.y/(2 + OFFSET)));
	Vector2 engine_end = 
		Vector2Add(engine_origin,
				Vector2Scale(looking_direction,
					-((velocity_len*velocity_len)
	)));

	Vector2 engine_left = Vector2Scale(Vector2Rotate(looking_direction,DEG2RAD*90),ENGINE_SIZE);
	engine_left = Vector2Add(engine_origin,engine_left);
	Vector2 engine_right = Vector2Scale(Vector2Rotate(looking_direction,DEG2RAD*90),-ENGINE_SIZE);
	engine_right = Vector2Add(engine_origin,engine_right);


	velocity_len = (3<velocity_len) ? velocity_len : 0;


	if (tick && velocity_len && (tick % velocity_len != 1)) {
		DrawTriangle(engine_left, engine_right, engine_end, ENGINE_COLOR);       
	}
}

void Entity_draw_player_debug(Entity e) {
	Vector2 size = e.model.model_size;
	Vector2 looking_direction = Vector2Rotate(Vec2{0,-1},e.rotation);
	Vector2 model_position = 
		Vector2Add(e.position,
				Vector2Scale(looking_direction,-size.y/(2 + OFFSET)));
	
	looking_direction = Vector2Scale(looking_direction, size.y);
	looking_direction = Vector2Add(looking_direction,model_position);
	
	DrawLineV(e.position, looking_direction, RED);
	DrawLineV(e.position, 
			Vector2Add(e.position,Vector2Scale(e.velocity,20)), PURPLE);
}


void Entity_draw_player_model_1(Entity e) {
	
	Vector2 size = e.model.model_size;
	Vector2 looking_direction = Vector2Rotate(Vec2{0,-1},e.rotation);
	
	Vector2 right_side = Vector2Rotate(looking_direction,90 * DEG2RAD);
	Vector2 left_side  = Vector2Rotate(looking_direction,-90 * DEG2RAD);
	Vector2 model_position = 
		Vector2Add(e.position,
				Vector2Scale(looking_direction,-size.y/(2 + OFFSET)));

	right_side = Vector2Scale(right_side, size.x/2.f);
	right_side = Vector2Add(right_side,model_position);

	left_side = Vector2Scale(left_side, size.x/2.f);
	left_side = Vector2Add(left_side,model_position);

	looking_direction = Vector2Scale(looking_direction, size.y);
	looking_direction = Vector2Add(looking_direction,model_position);
	
	float thickness = 10;
	Color line_color = WHITE;

	DrawLineEx(left_side, right_side, thickness, line_color);
	DrawLineEx(right_side, looking_direction, thickness, line_color);
	DrawLineEx(looking_direction,left_side, thickness, line_color);
}

void Entity_draw(Entity e, void* data) {
	switch (e.model.type) {
		case Model_PlayerGun:
		case Model_PlayerLazer: 
			{
				long* tick_ptr = data;
				Entity_draw_player_engine(e,*tick_ptr);
				Entity_draw_player_model_1(e);
			}
			break;

		default:
			break;
	}
}



void Entity_apply_drag(Entity* e) {
	e->velocity = Vector2Scale(e->velocity, 1-DRAG);

	bool moving_x	=  	fabs(e->velocity.x) > MIN_VELOCITY;
	bool moving_y	= 	fabs(e->velocity.y) > MIN_VELOCITY;
	bool accelerating_x = 	fabs(e->acceleration.x) > 0.0 ;
	bool accelerating_y =  	fabs(e->acceleration.y) > 0.0;
	
	if  (!moving_x && !accelerating_x)
		e->velocity.x = 0;
	if  (!moving_y && !accelerating_y)
		e->velocity.y = 0;
}

#endif //__GAME_ENTITY_H
