#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Entity.h"
#include "Animation.h"
//#include "ModulePhysics.h"
#include "p2Point.h"
#include "SDL/include/SDL.h"

struct SDL_Texture;

class Player : public Entity
{
public:

	Player(const char* path, iPoint posi);
	
	virtual ~Player();

	bool Start();

	bool Update();

	bool CleanUp();

public:

	PhysBody* getBody()
	{
		return body;
	}

	iVector getProjectile()
	{
		return projectileVector;
	}

	int getState()
	{
		return state;
	}

	int getRadius()
	{
		return rad;
	}

	float getAngle()
	{
		return angle;
	}
	float getSpeed() {
		return speed;
	}
	void setSpeed(float speed1) {
		speed = speed1;
	}
	void setAngle(float angle1)
	{
		angle = angle1;
	}

	void setIndex(int i)
	{
		index = i;
	}
	void setState(int i)
	{
		state = i;
	}

	int getIndex()
	{
		return index;
	}

	void playerTurn()
	{
		turn = true;
	}

	void endTurn()
	{
		turn = false;
	}

	iPoint angleToPoint(float radius, float angle);

private:
	
	//L02: DONE 1: Declare player parameters
	iPoint pos;
	int w, h;

	iVector projectileVector;

	SDL_Texture* texture;
	const char* texturePath;
	//animation stuff
	Animation* currentAnimation = nullptr;

	Animation IdleAnimation;
	Animation WalkRightAnimation;
	Animation WalkLeftAnimation;
	Animation JumpRightAnimation;
	Animation JumpLeftAnimation;
	Animation DeathAnimation;

	PhysBody* body = nullptr;

	int deathSound;
	int state;
	bool facing;
	float speed;
	float speedJump;

	int rad;
	float angle;

	bool turn;
	int index;
	bool canJump;

};

#endif // __PLAYER_H__