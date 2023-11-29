#pragma once

#include "p2List.h"
#include "Globals.h"
#include "Module.h"
#include "Dummy.h"
#include "ModuleWindow.h"
#include "ModuleRender.h"
#include "ModuleTextures.h"
#include "ModuleInput.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"
#include "ModuleSceneIntro.h"
#include "ModuleSceneMenu.h"
#include "ModuleFonts.h"
#include "ModuleDebug.h"
#include "EntityManager.h"

class Application
{
public:
	ModuleRender* renderer;
	ModuleWindow* window;
	ModuleTextures* textures;
	ModuleInput* input;
	ModuleAudio* audio;
	ModuleSceneIntro* scene_intro;
	ModuleSceneMenu* scene_Menu;
	ModulePhysics* physics;
	ModuleFonts* fonts;
	ModuleDebug* debug;
	EntityManager* entityManager;
	long long clock;

private:

	p2List<Module*> list_modules;

public:

	Application();
	~Application();

	bool Init();
	update_status Update();
	bool CleanUp();

private:

	void AddModule(Module* mod);
};

extern Application* App;