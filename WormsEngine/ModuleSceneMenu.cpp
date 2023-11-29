#include "Globals.h"
#include "Application.h"
#include "ModuleSceneMenu.h"



ModuleSceneMenu::ModuleSceneMenu(bool start_enabled) : Module(start_enabled)
{

}


ModuleSceneMenu::~ModuleSceneMenu()
{

}

// Load assets
bool ModuleSceneMenu::Start()
{
	App->audio->PlayMusic("Assets/Audio/MenuTheme.wav");
	entrysound = App->audio->LoadFx("Assets/Audio/EntryEnter.wav");
	bgtexture = App->textures->Load("Assets/Textures/intro(1).png");
	enter = App->textures->Load("Assets/Textures/PressEnter.png");
	entertimer = 0;

	return true;
}

bool ModuleSceneMenu::CleanUp()
{
	LOG("Deleting background assets");
	App->textures->Unload(bgtexture);
	App->textures->Unload(enter);
	App->audio->PlayMusic("");
	bgtexture = nullptr;
	enter = nullptr;
	return true;
}

update_status ModuleSceneMenu::Update()
{

	if (App->input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN) {
		App->audio->PlayFx(entrysound);
		this->Disable();
		App->scene_intro->Enable();
		App->physics->Enable();
		App->debug->Enable();
	}

	return UPDATE_CONTINUE;

}

update_status ModuleSceneMenu::PostUpdate()
{
	App->renderer->Blit(bgtexture,0,60);
	if (entertimer == 70) {
		entertimer = 0;
	}
	if (entertimer >= 30 && entertimer < 70) {
		App->renderer->Blit(enter, 340, 580);
	}
	++entertimer;

	return UPDATE_CONTINUE;
}