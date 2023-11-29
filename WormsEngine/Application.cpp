#include "Application.h"
#include <chrono>

using namespace std::chrono;

Application::Application()
{
	renderer = new ModuleRender();
	window = new ModuleWindow();
	textures = new ModuleTextures();
	input = new ModuleInput();
	audio = new ModuleAudio(true);
	scene_intro = new ModuleSceneIntro(false);
	scene_Menu = new ModuleSceneMenu(true);
	physics = new ModulePhysics(false);
	fonts = new ModuleFonts();
	debug = new ModuleDebug(false);
	entityManager = new EntityManager();

	// The order of calls is very important!
	// Modules will Init() Start() and Update in this order
	// They will CleanUp() in reverse order

	// Main Modules
	AddModule(window);
	AddModule(physics);
	AddModule(textures);
	AddModule(input);
	AddModule(audio);
	AddModule(fonts);

	// Scenes
	AddModule(scene_intro);
	AddModule(scene_Menu);

	//entity manager here in order to render good the textures one in front of the other
	AddModule(entityManager);
  
	//Dbug info on screen
	AddModule(debug);
	//renderer the last one always
	AddModule(renderer);
}

Application::~Application()
{
	p2List_item<Module*>* item = list_modules.getLast();

	while(item != NULL)
	{
		delete item->data;
		item = item->prev;
	}
}

bool Application::Init()
{
	bool ret = true;

	// Call Init() in all modules
	p2List_item<Module*>* item = list_modules.getFirst();

	while(item != NULL && ret == true)
	{
		ret = item->data->Init();
		item = item->next;
	}

	// After all Init calls we call Start() in all modules
	LOG("Application Start --------------");
	item = list_modules.getFirst();

	while(item != NULL && ret == true)
	{
		if(item->data->IsEnabled())
			ret = item->data->Start();
		item = item->next;
	}
	
	return ret;
}

// Call PreUpdate, Update and PostUpdate on all modules
update_status Application::Update()
{
	update_status ret = UPDATE_CONTINUE;
	p2List_item<Module*>* item = list_modules.getFirst();

	auto start = steady_clock::now();
	while(item != NULL && ret == UPDATE_CONTINUE)
	{
		if(item->data->IsEnabled())
			ret = item->data->PreUpdate();
		item = item->next;
	}

	item = list_modules.getFirst();

	while(item != NULL && ret == UPDATE_CONTINUE)
	{
		if(item->data->IsEnabled())
  			ret = item->data->Update();
		item = item->next;
	}

	item = list_modules.getFirst();

	while(item != NULL && ret == UPDATE_CONTINUE)
	{
		if(item->data->IsEnabled())
			ret = item->data->PostUpdate();
		item = item->next;
	}
	auto end = steady_clock::now();

	clock = duration_cast<milliseconds>(end - start).count();

	switch (App->debug->deltaTime)
	{
	case DeltaTimeScheme::VARIABLE:
		physics->dt = clock / 1000.0;
		break;
	case DeltaTimeScheme::SEMIFIXED:
		if (clock < 1.0 / App->debug->FPS * 1000.0) {
			SDL_Delay((1.0 / App->debug->FPS * 1000.0) - clock);
		}
		physics->dt = 1.0 / App->debug->FPS;
		break;
	case DeltaTimeScheme::FIXED:
		physics->dt = 1.0 / App->debug->FPS;
		break;
	default:

		break;
	}

	return ret;
}

bool Application::CleanUp()
{
	bool ret = true;
	p2List_item<Module*>* item = list_modules.getLast();

	while(item != NULL && ret == true)
	{
		ret = item->data->CleanUp();
		item = item->prev;
	}
	return ret;
}

void Application::AddModule(Module* mod)
{
	list_modules.add(mod);
}