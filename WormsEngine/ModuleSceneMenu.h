#pragma once
#include "Module.h"
#include "Globals.h"

class ModuleSceneMenu : public Module
{
public:
	ModuleSceneMenu(bool start_enabled = true);
	~ModuleSceneMenu();

	bool Start();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

private:
	SDL_Texture* bgtexture = nullptr;
	SDL_Texture* enter = nullptr;
	unsigned int entertimer;
	unsigned int entrysound;

};