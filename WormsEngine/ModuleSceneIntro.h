#pragma once
#include "Module.h"
#include "Player.h"
#include "Projectile.h"
#include "Animation.h"
#include "p2DynArray.h"
#include "Globals.h"

#define BOUNCER_TIME 200

enum lightTypes
{
	tiny,
	medium,
	big
};

class ModuleSceneIntro;

struct Light
{
	Light() : texture(NULL), on(false), fx(0)
	{}

	Light(ModuleSceneIntro* physics, int x, int y, lightTypes type);

	lightTypes type;
	SDL_Texture* texture;
	bool on;
	uint fx;
	int x, y;
};

class ModuleSceneIntro : public Module
{
public:
	ModuleSceneIntro(bool start_enabled = true);
	~ModuleSceneIntro();

	bool Start();
	update_status Update();
	bool CleanUp();

public:

	SDL_Texture* graphics;
	SDL_Texture* background;

	PhysBody* ground1;
	PhysBody* ground2;
	PhysBody* ground3;
	PhysBody* ground4;
	PhysBody* ground5;
	PhysBody* ground6;
	PhysBody* water;

	SDL_Texture* tex_light_tiny;
	SDL_Texture* tex_light_medium;
	SDL_Texture* tex_light_big;
	
	uint fx_light_tiny;
	uint fx_light_medium;
	uint fx_light_big;

	p2DynArray<Light> lights;

	uint player_lose_fx;

	p2List<Projectile*> projectiles;
	p2List<Player*> players;
	p2List_item<Player*>* actualPlayer;

	int despawnTimer;
	bool shot;
};
