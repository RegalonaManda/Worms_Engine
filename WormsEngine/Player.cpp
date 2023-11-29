#include "Application.h"
#include "Module.h"
#include "Player.h"
#include "Log.h"
#include "p2Point.h"

#define FACING_LEFT false
#define FACING_RIGHT true
#define IDLE 1
#define WALK 2
#define JUMPING 3
#define DYING 4
#define SHOOTING 5
#define WAITING 6
#define WINNING 7

Player::Player(const char* path, iPoint posi) : Entity(EntityType::PLAYER)
{
	IdleAnimation.PushBack({ 0 * 32,0 * 32,32,32 });
	IdleAnimation.PushBack({ 1 * 32,0 * 32,32,32 });
	IdleAnimation.PushBack({ 2 * 32,0 * 32,32,32 });
	IdleAnimation.PushBack({ 3 * 32,0 * 32,32,32 });
	IdleAnimation.PushBack({ 4 * 32,0 * 32,32,32 });
	IdleAnimation.loop = true;
	IdleAnimation.speed = 0.1f;

	WalkRightAnimation.PushBack({ 0 * 32,1 * 32,32,32 });
	WalkRightAnimation.PushBack({ 1 * 32,1 * 32,32,32 });
	WalkRightAnimation.PushBack({ 2 * 32,1 * 32,32,32 });
	WalkRightAnimation.PushBack({ 3 * 32,1 * 32,32,32 });
	WalkRightAnimation.PushBack({ 4 * 32,1 * 32,32,32 });
	WalkRightAnimation.PushBack({ 5 * 32,1 * 32,32,32 });
	WalkRightAnimation.loop = true;
	WalkRightAnimation.speed = 0.12f;

	WalkLeftAnimation.PushBack({ 0 * 32,2 * 32,32,32 });
	WalkLeftAnimation.PushBack({ 1 * 32,2 * 32,32,32 });
	WalkLeftAnimation.PushBack({ 2 * 32,2 * 32,32,32 });
	WalkLeftAnimation.PushBack({ 3 * 32,2 * 32,32,32 });
	WalkLeftAnimation.PushBack({ 4 * 32,2 * 32,32,32 });
	WalkLeftAnimation.PushBack({ 5 * 32,2 * 32,32,32 });
	WalkLeftAnimation.loop = true;
	WalkLeftAnimation.speed = 0.12f;

	JumpRightAnimation.PushBack({ 0 * 32,1 * 32,32,32 });
	JumpRightAnimation.PushBack({ 0 * 32,3 * 32,32,32 });
	JumpRightAnimation.PushBack({ 1 * 32,3 * 32,32,32 });
	JumpRightAnimation.PushBack({ 2 * 32,3 * 32,32,32 });
	JumpRightAnimation.PushBack({ 3 * 32,3 * 32,32,32 });
	JumpRightAnimation.PushBack({ 4 * 32,3 * 32,32,32 });
	JumpRightAnimation.PushBack({ 4 * 32,3 * 32,32,32 });
	JumpRightAnimation.PushBack({ 5 * 32,3 * 32,32,32 });
	JumpRightAnimation.loop = false;
	JumpRightAnimation.speed = 0.15f;

	JumpLeftAnimation.PushBack({ 0 * 32,2 * 32,32,32 });
	JumpLeftAnimation.PushBack({ 0 * 32,4 * 32,32,32 });
	JumpLeftAnimation.PushBack({ 1 * 32,4 * 32,32,32 });
	JumpLeftAnimation.PushBack({ 2 * 32,4 * 32,32,32 });
	JumpLeftAnimation.PushBack({ 3 * 32,4 * 32,32,32 });
	JumpLeftAnimation.PushBack({ 4 * 32,4 * 32,32,32 });
	JumpLeftAnimation.PushBack({ 4 * 32,4 * 32,32,32 });
	JumpLeftAnimation.PushBack({ 5 * 32,4 * 32,32,32 });
	JumpLeftAnimation.loop = false;
	JumpLeftAnimation.speed = 0.15f;

	DeathAnimation.PushBack({ 0 * 32,5 * 32,32,32 });
	DeathAnimation.PushBack({ 1 * 32,5 * 32,32,32 });
	DeathAnimation.PushBack({ 2 * 32,5 * 32,32,32 });
	DeathAnimation.PushBack({ 3 * 32,5 * 32,32,32 });
	DeathAnimation.PushBack({ 4 * 32,5 * 32,32,32 });
	DeathAnimation.PushBack({ 5 * 32,5 * 32,32,32 });
	DeathAnimation.PushBack({ 5 * 32,0 * 32,32,32 });
	DeathAnimation.loop = false;
	DeathAnimation.speed = 0.1f;


	texturePath = path;
	position = posi;
	w = 32;
	h = 32;
}

Player::~Player() {

}

bool Player::Start() {

	//initilize textures
	texture = App->textures->Load(texturePath);
	deathSound = App->audio->LoadFx("Assets/Audio/deathSound.wav");
	body = App->physics->CreateRectangle(	PIXEL_TO_METERS(position.x), 
											PIXEL_TO_METERS((position.y + h)),
											PIXEL_TO_METERS(w),
											PIXEL_TO_METERS(h),
											BodyType::DYNAMIC);
	// Set static properties of the player
	body->mass = 10.0f; // [kg]
	body->surface = 1.0f; // [m^2]
	body->cd = 0.4f; // [-]
	body->cl = 1.2f; // [-]
	body->b = 10.0f; // [...]
	body->coef_friction = 0.9f; // [-]
	body->coef_restitution = 0.8f; // [-]
	//
	//Set initial position and velocity of the player
	body->ctype = ColliderType::ENTITY;

	rad = 50.0f;
	angle = 0.0f;
	turn = false;
	state = IDLE;
	canJump = true;

	return true;
}

bool Player::Update()
{
	p2List_item<ColliderType>* collision = body->collisions.getFirst();

	while (collision != NULL) {
		switch (collision->data)
		{
		case ColliderType::ENTITY:
			break;
		case ColliderType::GROUND:
			if (state != SHOOTING && state != WAITING && state != DYING && state != WINNING)
			{
				state = IDLE;
				canJump = true;
			}
			break;
		case ColliderType::WATER:
			if (!App->debug->debug)
			{
				state = DYING;
			}
			break;
		case ColliderType::SHOT:
			state = DYING;
			break;
		case ColliderType::GRENADE:
			break;
		case ColliderType::UNKNOWN:
			break;
		default:
			break;
		}
	    collision = collision->next;
	}

	if (App->debug->pause)
	{
		return UPDATE_CONTINUE;
	}
	if (App->debug->debugSpeed) {
		speed = PIXEL_TO_METERS(200);
	}
	else {
		speed = PIXEL_TO_METERS(20);
		speedJump = PIXEL_TO_METERS(200);
	}

	if (turn && state != WINNING)
	{
		if (state != DYING && state != SHOOTING && state != WAITING && state != JUMPING)
		{
			//body->velocity = { 0,0 };
			state = IDLE;
			DeathAnimation.Reset();
			//moves player when its alive
			//error: player is always moving to the right for no reason
			//to do: in each input it applies the speed, we must write somewhere... 
			//...what woule be the final velocity, and at the end of the update apply it
			if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
				state = WALK;
				facing = FACING_LEFT;
				body->velocity.x = -speed;
			}

			if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
				state = WALK;
				facing = FACING_RIGHT;
				body->velocity.x = speed;
			}
			if (App->input->GetKey(SDL_SCANCODE_W) == KEY_DOWN && canJump)
			{
				state = JUMPING;
				body->velocity.y = speedJump;
				canJump = false;
				JumpLeftAnimation.Reset();
				JumpRightAnimation.Reset();
			}
			if (App->input->GetKey(SDL_SCANCODE_O) == KEY_DOWN)
			{
				state = SHOOTING;
			}
		}
	}

    switch (state)
	{
	case IDLE:

		currentAnimation = &IdleAnimation;

		break;
	case WAITING:

		currentAnimation = &IdleAnimation;

		break;
	case WALK:
		if (facing == FACING_LEFT)
		{
			currentAnimation = &WalkLeftAnimation;
		}
		else if (facing == FACING_RIGHT)
		{
			currentAnimation = &WalkRightAnimation;
		}
		break;
	case JUMPING:
		if (facing == FACING_LEFT)
		{
			currentAnimation = &JumpLeftAnimation;
		}
		else if (facing == FACING_RIGHT)
		{
			currentAnimation = &JumpRightAnimation;
		}
		break;
	case DYING:

		currentAnimation = &DeathAnimation;

		break;
	case WINNING:

		App->fonts->BlitText(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0, "you win");

		break;
	case SHOOTING:
		body->velocity.x = 0;

		if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
		{
			rad += 8;
			if (rad > 400)
			{
				rad = 400;
			}
		}

		if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
		{
			rad -= 8;
			if (rad < 8)
			{
				rad = 8;
			}
		}

		if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
		{
			angle -= 0.1f;
		}

		if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
		{
			angle += 0.1f;
		}


		if (App->input->GetKey(SDL_SCANCODE_I) == KEY_DOWN)
		{
			//here should start the shoot, pass the projectilevector in pixels to the projectile
			state = IDLE;
		}
		//vector of the projectile in pixels
		if (angle * RADTODEG > 360 || angle*RADTODEG < -360) {
			angle = 0;
		}
		projectileVector = { angleToPoint(rad, angle).x ,  angleToPoint(rad, angle).y };
		break;
	default:
		break;

	}
	currentAnimation->Update();
	position.x = METERS_TO_PIXELS(body->position.x);
	position.y = METERS_TO_PIXELS(body->position.y) + h;
	//blit slime
	SDL_Rect rect = currentAnimation->GetCurrentFrame();
	App->renderer->Blit(texture, position.x, SCREEN_HEIGHT - position.y, &rect);
	if (state == SHOOTING)
	{
		App->renderer->DrawCircle(position.x + 16, SCREEN_HEIGHT - position.y + 16, rad, 255, 255, 255);
		App->renderer->DrawLine(projectileVector.x + position.x + 16, projectileVector.y + SCREEN_HEIGHT - position.y + 16, position.x + 16, SCREEN_HEIGHT - position.y + 16, 255, 255, 255);
	}

	return true;
}

bool Player::CleanUp()
{
	App->textures->Unload(texture);
	return true;
}

iPoint Player::angleToPoint(float radius, float angle)
{
	iPoint temp;
	float sinus = SDL_sin(angle);
	float cosinus = SDL_cos(angle);

	temp.y = radius * sinus;
	temp.x = radius * cosinus;
	
	return temp;
}
