#ifndef __GAME_DEFS_H
#define __GAME_DEFS_H

// NAME ALIASES
#define w width
#define h height
#define Rec (Rectangle)
#define Vec2 (Vector2)
#define Col (Color)
#define Ent (Entity)

// FUNCTION-LIKE MACROS
#define IGNORE (void)
#define UnfoldVec2(V) V.x, V.y 
#define loop(I,N) for(size_t I = 0; I < (N); (I)++)
#define MAX(L,R) ((L) > (R)) ? (L) : (R)
#define MIN(L,R) ((L) < (R)) ? (L) : (R)

// COOL DEBUGGING MACRO
#ifdef DEBUG
#define debug(...)  printf("\t[DEBUG] : " __VA_ARGS__)
#endif

#ifndef DEBUG
#define debug(...) // __VA__ARGS__
#endif

// CONSTANTS
#define DRAG 0.02
#define MIN_VELOCITY 0.05
#define ACCELERATION 2
#define CAMERA_ZOOM_LEVEL 5
#define FONT_SIZE 24
#define ROTATION_SPEED 30
#define BLINK_RATE 5
#define PROJECTILE_VELOCITY 50.0
#define PROJECTILE_SIZE 15.0
#define PROJECTILE_COLOR GetColor(0xFFFF4CFF)
#define OFFSET 0.5
#define ENGINE_SIZE 50
#define ENGINE_COLOR Col {175,220,250,255}


// #define STOP_COOLDOWN 3.0 NOT IMPLEMENTED
#define SOUND_SHOT_VOLUME 0.5
#define SOUND_ENGINE_VOLUME 0.5
#define SOUND_SHOT_PITCH_MAX 1.7
#define SOUND_SHOT_PITCH_MIN 1.0
#define SOUND_SHOT_DEVIATION_MIN 8
#define SOUND_SHOT_DEVIATION_MAX 10
#define SOUND_ENGINE_MIN 8
#define SOUND_ASTEROID_VOLUME_FACTOR 0.1

#define FPS 60
#define SPAWN_INTERVAL_SINGLE 	120
#define SPAWN_INTERVAL_WAVE 	120*3
#define MAX_ASTEROID_SPAWN_COUNT 14
#define MIN_ASTEROID_VELOCITY 5
#define MAX_ASTEROID_VELOCITY 15
#define MAX_ASTEROID_HP 3
#define MAX_ASTEROIDS_VERTECIES 9
#define MIN_ASTEROIDS_VERTECIES 5
#define MAX_ASTEROID_VERTICE_OFFSET 50
#define MAX_RANDOM_ROTATION 50

#define ANIMATION_SPAWN_LENGTH 1.0
#define STARTING_VELOCITY 25.0
#define PLAYER_GIBS_SPREAD_ANGLE 35
#define SHIP_EXPLOSION_COLOR Col {237,145,33,255}

#define PORTAL_DECAY_FACTOR 0.2
#define SOUND_SPAWN_TIMING 0.75
#define ANIMATION_DEATH_LENGTH 1.25
#define ANIMATION_DEATH_CLEANSE_TIMING 1.0

#define MAX_SOUNDS 10

#define ASTEROID_COLOR Col {176,175,176,255}
#define ARENA_OFFSCREEN_AREA_SIZE 300
#define ASTEROID_SIZE_DEVIATION 15
#define RECOIL 2

#define SCOREBOARD_FONT_SIZE 	24 
#define SCOREBOARD_BG_COLOR Col {10,10,10,255}
#define SCOREBOARD_FG_COLOR Col {200,200,200,255}
#define BACKGROUND_COLOR	Col {5,5,5,255}

// #define SPEED_BREAK_FACTOR 0.25 // NOT USED

// LIMITS
#define MAX_CONTROL_SCHEMES 4
#define MAX_ENTITY_COUNT 64*2 // DOUBLE THE CHAOS DOUBLE THE FUN

// TYPE SPECIFIC CONSTANTS
#define RELPTR_INVALID (uint) -1 // 4.2billion something

typedef unsigned int uint;
typedef unsigned char bitmask8;
typedef uint Relptr;


// TYPES
// TODO?
typedef struct {

} GameConfig;


typedef enum {
	Asteroid_null,
	Asteroid_small,
	Asteroid_medium,
	Asteroid_big,
} AsteroidType;

typedef enum {
	AsteroidSize_small = 100,
	AsteroidSize_medium = 150,
	AsteroidSize_big = 300,
} AsteroidSize;

typedef enum {
	AsteroidSpawnSide_left 		= 1,
	AsteroidSpawnSide_right 	= 2,
	AsteroidSpawnSide_top 		= 3,
	AsteroidSpawnSide_bottom 	= 4,
} AsteroidSpawnSide;

typedef enum {
	Model_PlayerGun,
	Model_PlayerLazer,
	Model_AsteroidBig,
	Model_AsteroidMedium,
	Model_AsteroidSmall,
	Model_ProjectileLazer,
} ModelType;

typedef enum {
	EntityKind_null,
	EntityKind_player,
	EntityKind_projectile,
	EntityKind_asteroid,
} EntityKind;

typedef struct {
	ModelType 	type;
	Vector2 	model_size;
	int			model_seed;
} GameModel;

typedef struct {
	float 			hp;
	uint 			score;
	float 			shoot_cooldown; // NOT USED
	float			damage_time; // flash
	float			speed_break_cooldown; // NOT USED
	float 			sps; // shots per second, used for gun pitch
	AsteroidType 	asteroid_class;
	bool			gothit;
} KeyValue;

typedef struct {
	Vector2 	position;
	Vector2 	velocity;
	float		rotation;
	Vector2 	acceleration;
	Vector2 	size;
	GameModel	model;
	EntityKind	kind;
	KeyValue	vals;
} Entity;

typedef struct {
	int up, down, left, right;
	int shoot, bigshoot;
	int speed_break;
} ControlScheme; 


typedef enum {
	EntityState_allocated = 1,
} EntityState;

typedef struct {
	Entity		buffer 			[MAX_ENTITY_COUNT];
	bitmask8	state_buffer	[MAX_ENTITY_COUNT];
	Relptr		free_buffer		[MAX_ENTITY_COUNT];
	//Relptr		reserved_buffer	[MAX_ENTITY_COUNT];
	size_t 		count;
	size_t		free_count;
	size_t		max_size;
} EntityPool;

typedef struct {
	Rectangle 	scoreboard_box;
	Rectangle 	menu_text;
	float	  	scoreboard_transperency;
	float 		menu_text_transperency;
} UiBuilder;

typedef struct {
	Sound engine;
	Sound gunfire;
	Sound lazerfire;
	Sound speedbreak;
	Sound speedrestore;
	Sound rockbreak;
	Sound shipbreak;
	Sound warpspawn;
} GameAssets;


typedef enum {
	Game_idle,
	Game_spawning,
	Game_going,
	Game_over,
} GameState;

typedef struct {
	uint 		highest_score;
	Rectangle 	arena;
	ControlScheme 
				schemes[MAX_CONTROL_SCHEMES];
	Entity 		player;
	EntityPool  ents;
	long 		tick;
	Camera2D 	camera;

	float 		spawn_time;
	float 		death_time;
	GameState	state;
	GameAssets	assets;
	UiBuilder 	ui;

	size_t 		current_sound;
	Sound 		sound_buffer[MAX_SOUNDS];

	struct {
		bool show_fps;
		bool show_entity_hitbox;
		bool show_entity_model;
	} flags; // debugging mostly
} Game;

#endif // __GAME_DEFS_H
