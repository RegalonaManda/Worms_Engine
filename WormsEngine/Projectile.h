#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "Entity.h"
#include "Animation.h"
#include "p2Point.h"
#include "SDL/include/SDL.h"

struct SDL_Texture;

class Projectile : public Entity
{
public:

	Projectile(const char* path, iPoint posi);

	virtual ~Projectile();

	bool Start();

	bool Update();

	bool CleanUp();

public:

	PhysBody* getBody()
	{
		return body;
	}

private:

	iPoint pos;
	SDL_Texture* texture;
	const char* texturePath;

	PhysBody* body = nullptr;

};

#endif // __PROJECTILE_H__
