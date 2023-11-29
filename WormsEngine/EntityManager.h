#ifndef __ENTITYMANAGER_H__
#define __ENTITYMANAGER_H__

#include "Module.h"
#include "Entity.h"
#include "p2List.h"

class EntityManager : public Module
{
public:

	EntityManager(bool start_enabled = true);

	// Destructor
	virtual ~EntityManager();


	bool Start();
	update_status Update();
	bool CleanUp();

	// Additional methods
	Entity* CreateEntity(EntityType type, const char* path = 0, iPoint position = { 0,0 });

	void DestroyEntity(Entity* entity);

	void AddEntity(Entity* entity);

public:

	p2List<Entity*> entities;

};

#endif // __ENTITYMANAGER_H__
