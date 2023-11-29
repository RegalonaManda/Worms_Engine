#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "p2Point.h"
#include "SString.h"
#include "ModuleInput.h"
#include "ModuleRender.h"

struct Collider;

enum class EntityType
{
	PLAYER,
	PROJECTILE,
	UNKNOWN
};

class Entity
{
public:

	Entity(EntityType type) : type(type), active(true) {}

	virtual bool Start()
	{
		return true;
	}

	virtual bool Update()
	{
		return true;
	}

	virtual bool CleanUp()
	{
		return true;
	}

	void Entity::Enable()
	{
		if (!active)
		{
			active = true;
			Start();
		}
	}

	void Entity::Disable()
	{
		if (active)
		{
			active = false;
			CleanUp();
		}
	}

	virtual void OnCollision(Collider* c1, Collider* c2) {
	
	};

public:

	SString name;
	EntityType type;
	bool active = true;

	// Possible properties, it depends on how generic we
	// want our Entity class, maybe it's not renderable...
	iPoint position;       
	bool renderable = true;
};

#endif // __ENTITY_H__