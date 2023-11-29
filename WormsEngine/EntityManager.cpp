#include "Application.h"
#include "EntityManager.h"
#include "Player.h"
#include "Globals.h"

#include "Defs.h"
#include "log.h"

EntityManager::EntityManager(bool start_enabled) : Module(start_enabled)
{
	
}

// Destructor
EntityManager::~EntityManager()
{}


bool EntityManager::Start() {

	bool ret = true; 

	//Iterates over the entities and calls Start
	p2List_item<Entity*>* item;
	Entity* pEntity = NULL;

	for (item = entities.getFirst(); item != NULL && ret == true; item = item->next)
	{
		pEntity = item->data;

		if (pEntity->active == false) continue;
		ret = item->data->Start();
	}

	return ret;
}

// Called before quitting
bool EntityManager::CleanUp()
{
	bool ret = true;
	p2List_item<Entity*>* item;
	item = entities.getLast();

	while (item != NULL && ret == true)
	{
		ret = item->data->CleanUp();
		item = item->prev;
	}

	entities.clear();

	return ret;
}

Entity* EntityManager::CreateEntity(EntityType type, const char* path, iPoint position)
{
	Entity* entity = nullptr; 

	//L02: DONE 2: Instantiate entity according to the type and add the new entoty it to the list of Entities

	switch (type)
	{

	case EntityType::PLAYER:
		entity = new Player(path, position);
		break;
	case EntityType::PROJECTILE:
		entity = new Projectile(path, iPoint(position.x + 16, position.y + 16));
		break;

	default: break;
	}

	// Created entities are added to the list
	entity->Start();
	AddEntity(entity);

	return entity;
}

void EntityManager::DestroyEntity(Entity* entity)
{
	p2List_item<Entity*>* item;

	for (item = entities.getFirst(); item != NULL; item = item->next)
	{
		if (item->data == entity) entities.del(item);
	}
}

void EntityManager::AddEntity(Entity* entity)
{
	if ( entity != nullptr) entities.add(entity);
}

update_status EntityManager::Update()
{
	bool ret = true;
	p2List_item<Entity*>* item;
	Entity* pEntity = NULL;

	for (item = entities.getFirst(); item != NULL && ret == true; item = item->next)
	{
		pEntity = item->data;

		if (pEntity->active == false) continue;
		ret = item->data->Update();
	}

	return UPDATE_CONTINUE;
}