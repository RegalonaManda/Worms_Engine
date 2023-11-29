#include "Globals.h"
#include "Application.h"
#include "ModuleSceneIntro.h"

#define IDLE 1
#define WALK 2
#define JUMPING 3
#define DYING 4
#define SHOOTING 5
#define WAITING 6
#define WINNING 7



ModuleSceneIntro::ModuleSceneIntro(bool start_enabled) : Module(start_enabled)
{
	graphics = NULL;
}

ModuleSceneIntro::~ModuleSceneIntro()
{}

// Load assets
bool ModuleSceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;
	App->audio->PlayMusic("Assets/Audio/backgroundMusic.ogg");
	App->renderer->camera.x = App->renderer->camera.y = 0;
	background = App->textures->Load("Assets/Textures/background.png");

	//create ground & water
	ground1 = App->physics->CreateRectangle(-1.0f, 0.0f, 1.0f, 20.0f, BodyType::STATIC);
	ground1->ctype = ColliderType::GROUND;

	ground2 = App->physics->CreateRectangle(0.0f, 0.0f, 15.0f, 2.5f, BodyType::STATIC);
	ground2->ctype = ColliderType::GROUND;

	water = App->physics->CreateWaterRectangle(ground2->w, 0.0f, 15.0f, 2.5f);

	ground3 = App->physics->CreateRectangle(water->position.x + water->w, 0.0f, 15.0f, 2.5f, BodyType::STATIC);
	ground3->ctype = ColliderType::GROUND;

	ground4 = App->physics->CreateRectangle(ground3->position.x + ground3->w, 0.0f, 1.0f, 20.0f, BodyType::STATIC);
	ground4->ctype = ColliderType::GROUND;

	ground5 = App->physics->CreateRectangle(ground2->w, 7.5f, 15.0f, 1.0f, BodyType::STATIC);
	ground5->ctype = ColliderType::GROUND;

	ground6 = App->physics->CreateRectangle(-1.0f, -1.0f, ground4->position.x + ground4->w, 1.0f, BodyType::STATIC);
	ground6->ctype = ColliderType::GROUND;
  
	//Create entity
	//creating 1 player
	Player* player1 = (Player*)App->entityManager->CreateEntity(EntityType::PLAYER, "Assets/Textures/spriteplayer1.png", { METERS_TO_PIXELS(1.5), METERS_TO_PIXELS(3.5f) });
	player1->setIndex(1);
	players.add(player1);

	Player* player2 = (Player*)App->entityManager->CreateEntity(EntityType::PLAYER, "Assets/Textures/spriteplayer2.png", { METERS_TO_PIXELS((ground5->position.x + 1.5)), METERS_TO_PIXELS(9.5f) });
	player2->setIndex(2);
	players.add(player2);

	Player* player3 = (Player*)App->entityManager->CreateEntity(EntityType::PLAYER, "Assets/Textures/spriteplayer3.png", { METERS_TO_PIXELS(((ground5->position.x + ground5->w) - 1.5)), METERS_TO_PIXELS(9.5f) });
	player3->setIndex(3);
	players.add(player3);

	Player* player4 = (Player*)App->entityManager->CreateEntity(EntityType::PLAYER, "Assets/Textures/spriteplayer4.png", { METERS_TO_PIXELS(((ground3->position.x + ground3->w) - 1.5)), METERS_TO_PIXELS(3.5f) });
	player4->setIndex(4);
	players.add(player4);

	actualPlayer = players.getFirst();

	despawnTimer = 0;
	shot = false;

	return ret;
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");
	App->audio->PlayMusic("");

	/*p2List_item<Projectile*>* projectileItem = projectiles.getFirst();

	while (projectileItem != NULL)
	{
		delete projectileItem->data;
		projectileItem->data = NULL;
		projectileItem = projectileItem->next;
	}*/
	projectiles.clear();
	return true;
}

// Update: draw background
update_status ModuleSceneIntro::Update()
{
	/*if (actualPlayer->data->position.x > 400 / SCREEN_SIZE && actualPlayer->data->position.x < ((SCREEN_HEIGHT * SCREEN_WIDTH) - (METERS_TO_PIXELS((ground4->position.x + ground4->w)) + SCREEN_WIDTH) / SCREEN_SIZE)) {
		App->renderer->camera.x = ((actualPlayer->data->position.x - 400 / SCREEN_SIZE) * -1) * SCREEN_SIZE;
	}*/
	if (actualPlayer->data->getState() != SHOOTING && !App->debug->debug)
	{
		App->renderer->camera.x = ((actualPlayer->data->position.x - 400 / SCREEN_SIZE) * -1) * SCREEN_SIZE;
		App->renderer->camera.y = 0;
	}

	/*if (actualPlayer->data->position.y > 300 / SCREEN_SIZE && actualPlayer->data->position.y < ((SCREEN_HEIGHT * SCREEN_WIDTH) - 458 / SCREEN_SIZE)) {
		App->renderer->camera.y = ((actualPlayer->data->position.y - 300 / SCREEN_SIZE) * -1) * SCREEN_SIZE;
	}*/
	int speed = 10;
	if (actualPlayer->data->getState() == SHOOTING || App->debug->debug)
	{
		if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
		{
			App->renderer->camera.x -= speed;
		}
		if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
		{
			App->renderer->camera.x += speed;
		}
	}


	actualPlayer->data->playerTurn();


	//projectile
	if ((App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) && (actualPlayer->data->getState() == SHOOTING))
	{
		shot = true;
		Projectile* projectile;
		switch (actualPlayer->data->getIndex())
		{
		case 1:
			projectile = (Projectile*)App->entityManager->CreateEntity(EntityType::PROJECTILE, "Assets/Textures/shotplayer1.png", actualPlayer->data->position);
			break;
		case 2:
			projectile = (Projectile*)App->entityManager->CreateEntity(EntityType::PROJECTILE, "Assets/Textures/shotplayer2.png", actualPlayer->data->position);
			break;
		case 3:
			projectile = (Projectile*)App->entityManager->CreateEntity(EntityType::PROJECTILE, "Assets/Textures/shotplayer3.png", actualPlayer->data->position);
			break;
		case 4:
			projectile = (Projectile*)App->entityManager->CreateEntity(EntityType::PROJECTILE, "Assets/Textures/shotplayer4.png", actualPlayer->data->position);
			break;
		default:
			break;
		}
		projectiles.add(projectile);
		actualPlayer->data->setState(WAITING);
	}
	if (shot == true)
	{
		despawnTimer++;
		if (despawnTimer > 60)
		{
			actualPlayer->data->endTurn();
			shot = false;
			despawnTimer = 0;
			int i = 0;
			if (actualPlayer->next != NULL)
			{
				actualPlayer = actualPlayer->next;
			}
			else
			{
				actualPlayer = players.getFirst();
			}
			while (actualPlayer->data->getState() == DYING)
			{
				i++;
				if (actualPlayer->next != NULL)
				{
					actualPlayer = actualPlayer->next;
					continue;
				}
				else
				{
					actualPlayer = players.getFirst();
					continue;
				}
			}
			if (i > 2) actualPlayer->data->setState(WINNING);
			if (actualPlayer->data->getState() != WINNING)
			{
				actualPlayer->data->setState(IDLE);
			}


			projectiles.clear();
		}
	}
	App->renderer->Blit(background, 1, -250);
	switch (actualPlayer->data->getIndex())
	{
	case 1:
		App->fonts->BlitText(500, 30, 0, "actual player 1");
		break;
	case 2:
		App->fonts->BlitText(500, 30, 0, "actual player 2");
		break;
	case 3:
		App->fonts->BlitText(500, 30, 0, "actual player 3");
		break;
	case 4:
		App->fonts->BlitText(500, 30, 0, "actual player 4");
		break;
	default:
		break;
	}
	return UPDATE_CONTINUE;
}
