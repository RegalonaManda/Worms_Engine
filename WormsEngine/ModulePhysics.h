#pragma once
#include "Module.h"
#include "Globals.h"
#include <vector>
#include "p2Point.h"
#include "p2List.h"


#define PIXELS_PER_METER (40.0f) //  40 pixel -> 1 meter
#define METER_PER_PIXEL (1.0f / PIXELS_PER_METER) // 1 meter / Pixels per meter

#define METERS_TO_PIXELS(m) ((int) floor(PIXELS_PER_METER * m))
#define PIXEL_TO_METERS(p)  ((float) METER_PER_PIXEL * p)

// types of bodies
enum BodyType {
	DYNAMIC,
	STATIC
};

enum ShapeType {
	CIRCLE,
	RECTANGLE
};

// types of Colliders
enum class ColliderType {
	ENTITY,
	GROUND,
	WATER,
	SHOT,
	GRENADE,
	UNKNOWN
};

class PhysBody {
public:
	ColliderType ctype;
	BodyType btype;

	//Position
	p2Point<float> position;

	//Velocity
	p2Point<float> velocity;

	//Acceleration
	p2Point<float> acceleration;

	//Force
	p2Point<float> force;

	//Mass
	float mass;

	//Density
	float density;

	//Aerodynamics
	float surface; // Effective wet surface
	float cl; // Aerodynamic Lift coefficient
	float cd; // Aerodynamic Drag coefficient
	float b; // Hydrodynamic Drag coefficient

	// Coefficients of friction and restitution in Bounces
	float coef_friction;
	float coef_restitution;

	//Shape CIRCLE
	float radius;

	//Shape RECTANGLE
	float w, h;

	p2List<ColliderType> collisions;

private:
	ShapeType stype;
	bool water = false;
	bool physics_enabled = true;

public:

	PhysBody(ShapeType stype, bool water) {
		this->stype = stype;
		this->water = water;
	}

	SDL_Rect pixelRect() {
		SDL_Rect pos_px;
		pos_px.x = METERS_TO_PIXELS(position.x);
		pos_px.y = SCREEN_HEIGHT - METERS_TO_PIXELS(position.y);
		pos_px.w = METERS_TO_PIXELS(w);
		pos_px.h = METERS_TO_PIXELS(-h);
		return pos_px;
	}

	void SetPosition(int x, int y) {
		position.x = x;
		position.y = y;
	}

	void SetMass(int mass) {
		this->mass = mass;
	}

	void SetVelocity(int x, int y) {
		velocity.x = x;
		velocity.y = y;
	}

	void SetAcceleration(int x, int y) {
		acceleration.x = x;
		acceleration.y = y;
	}

	void SetForce(int x, int y) {
		force.x = x;
		force.y = y;
	}

	ShapeType GetShape() {
		return stype;
	}

	float GetMass() {
		return mass;
	}

	bool IsWater() {
		return water;
	}

	void DisablePhysics() {
		physics_enabled = false;
	}

	void EnablePhysics() {
		physics_enabled = true;
	}

	bool ArePhysicsEnabled() {
		return physics_enabled;
	}
};

struct Atmosphere
{
	float density;
	float windx;
	float windy;
};

struct World
{
	Atmosphere atmosphere;
	p2List<PhysBody*> Elements;
	p2List<PhysBody*> ElementsToDelete;
};

class ModulePhysics : public Module
{
public:
	ModulePhysics(bool start_enabled = true);
	~ModulePhysics();

	bool Start();
	update_status PreUpdate();
	update_status PostUpdate();
	bool CleanUp();

	PhysBody* CreateCircle(float pos_x, float pos_y, float rad, BodyType bodyType);
	PhysBody* CreateRectangle(float pos_x, float pos_y, float w, float h, BodyType bodyType);
	PhysBody* CreateWaterRectangle(float pos_x, float pos_y, float w, float h);

	World world;

	void ModulePhysics::DeleteElementOfWorld();

	//Time between frames
	float dt = 1.0 / 60.0;


private:

	// Detect collision with water
	bool is_colliding_with_water(p2List_item<PhysBody*>* element);

	//Checks collision between two elements
	bool are_colliding(p2List_item<PhysBody*>* element1, p2List_item<PhysBody*>* element2);

	// Detect collision between circle and rectange
	bool check_collision_circle_rectangle(float cx, float cy, float cr, float rx, float ry, float rw, float rh);

	// Detect collision between rectangle and rectange
	bool check_collision_rectangle_rectangle(float rx1, float ry1, float rw1, float rh1, float rx2, float ry2, float rw2, float rh2);

	// Detect collision between circle and circle
	bool check_collision_circle_circle(float cx1, float cy1, float cr1, float cx2, float cy2, float cr2);

	// Compute modulus of a vector
	float modulus(float vx, float vy);

	// Compute Aerodynamic Drag force
	p2Point<float> compute_aerodynamic_drag(p2Point<float> dforce, p2List_item<PhysBody*>* element);

	// Compute Hydrodynamic Drag force
	p2Point<float> compute_hydrodynamic_drag(p2Point<float> dforce, p2List_item<PhysBody*>* element, p2List_item<PhysBody*>* water);

	// Compute Hydrodynamic Buoyancy force
	p2Point<float> compute_hydrodynamic_buoyancy(p2Point<float> bforce, p2List_item<PhysBody*>* element, p2List_item<PhysBody*>* water);

	// Integration scheme: Velocity Verlet
	void integrator_velocity_verlet(p2List_item<PhysBody*>* element);

	//Collision Solver
	void collision_solver(p2List_item<PhysBody*>* element, p2List_item<PhysBody*>* element_to_check);
};