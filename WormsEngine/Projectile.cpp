#include "Application.h"
#include "Module.h"
#include "Projectile.h"
#include "Log.h"
#include "p2Point.h"

Projectile::Projectile(const char* path, iPoint posi) : Entity(EntityType::PROJECTILE)
{
	texturePath = path;
	position = posi;
}

Projectile::~Projectile() {

}

bool Projectile::Start() {
	int h = 32;
	float speed = 2.0f;
	//initilize textures
	texture = App->textures->Load(texturePath);
	body = App->physics->CreateCircle( PIXEL_TO_METERS((position.x + App->scene_intro->actualPlayer->data->angleToPoint(40 , App->scene_intro->actualPlayer->data->getAngle()).x)),
									   PIXEL_TO_METERS((position.y - h - App->scene_intro->actualPlayer->data->angleToPoint(40, App->scene_intro->actualPlayer->data->getAngle()).y)),
									   PIXEL_TO_METERS(4,5),
									   BodyType::DYNAMIC);

	body->mass = 10.0f; // [kg]
	body->surface = 1.0f; // [m^2]
	body->cd = 0.4f; // [-]
	body->cl = 1.2f; // [-]
	body->b = 10.0f; // [...]
	body->coef_friction = 0.9f; // [-]
	body->coef_restitution = 0.8f; // [-]

	body->ctype = ColliderType::SHOT;

	body->SetVelocity(speed * PIXEL_TO_METERS(App->scene_intro->actualPlayer->data->getProjectile().x),speed * -(PIXEL_TO_METERS(App->scene_intro->actualPlayer->data->getProjectile().y)));
	return true;
}

bool Projectile::Update()
{
	if (App->debug->pause)
	{ return true; }

	pos = { METERS_TO_PIXELS(body->position.x), METERS_TO_PIXELS(body->position.y) };
	App->renderer->Blit(texture, pos.x - 4, SCREEN_HEIGHT - pos.y - 4);
	return true;
}

bool Projectile::CleanUp()
{
	App->textures->Unload(texture);
	return true;
}
