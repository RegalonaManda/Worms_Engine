#include "Application.h"
#include "Globals.h"
#include "ModulePhysics.h"
#include "math.h"
#include <cmath>

ModulePhysics::ModulePhysics(bool start_enabled) : Module(start_enabled)
{
}

// Destructor
ModulePhysics::~ModulePhysics()
{
}

bool ModulePhysics::Start()
{
	LOG("Creating Physics 2D environment");

	world.atmosphere.density = 1.0f;
	world.atmosphere.windx = 0.0f;
	world.atmosphere.windy = 0.0f;

	return true;
}

// 
update_status ModulePhysics::PreUpdate()
{
	if (App->debug->pause)
	{
		return UPDATE_CONTINUE;
	}
	p2List_item<PhysBody*>* element = world.Elements.getFirst();
	// Process all elements in the world
	while (element != NULL)
	{
		// Skip element if static or physics not enabled
		if (element->data->btype == BodyType::STATIC || !element->data->ArePhysicsEnabled()) {
			element = element->next;
			continue;
		}

		// Step #0: Clear old values
		// ----------------------------------------------------------------------------------------

		// Reset total acceleration and total accumulated force of the element
		element->data->force.x = 0.0f;
		element->data->force.y = 0.0f;
		element->data->acceleration.x = 0.0f;
		element->data->acceleration.y = 0.0f;

		// Step #1: Compute forces
		// ----------------------------------------------------------------------------------------

			// Gravity force
			if (App->debug->gravityEnabled == true) {
				p2Point<float> gforce;
				gforce.x = element->data->mass * 0.0f;
				gforce.y = element->data->mass * -10.0f; //Gravity is constant and downwards
				element->data->force += gforce; // Add this force to element's total force
			}
			
			// Aerodynamic Drag force (only when not in water)
			if (!is_colliding_with_water(element))
			{
				p2Point<float> dforce;
				dforce.x = 0.0f, dforce.y = 0.0f;
				if (App->debug->aerodynamicDragEnabled) {
					dforce = compute_aerodynamic_drag(dforce, element);
				}
				element->data->force += dforce; // Add this force to element's total force
			}

			// Hydrodynamic forces (only when in water)
			if (is_colliding_with_water(element))
			{
				//Iterate wuith all elements of the world
				p2List_item<PhysBody*>* element_to_check = world.Elements.getFirst();
				while (element_to_check != NULL)
				{
					//If element is water type, check if colliding
					if (element_to_check->data->IsWater()) {
						// Hydrodynamic Drag force
						p2Point<float> dforce;
						dforce.x = 0.0f, dforce.y = 0.0f;
						if (App->debug->hydrodynamicDragEnabled) {
							dforce = compute_hydrodynamic_drag(dforce, element, element_to_check);
						}
						element->data->force += dforce; // Add this force to ball's total force

						// Hydrodynamic Buoyancy force
						p2Point<float> bforce;
						bforce.x = 0.0f, bforce.y = 0.0f;
						if (App->debug->hydrodynamicBuoyancyEnabled) {
							bforce = compute_hydrodynamic_buoyancy(bforce, element, element_to_check);
						}
						element->data->force += bforce; // Add this force to ball's total force
					}

				//Next element of the world
				element_to_check = element_to_check->next;
			}
		}

		// Other forces
		// ...

		// Step #2: 2nd Newton's Law
		// ----------------------------------------------------------------------------------------

		// SUM_Forces = mass * accel --> accel = SUM_Forces / mass
		element->data->acceleration.x = element->data->force.x / element->data->mass;
		element->data->acceleration.y = element->data->force.y / element->data->mass;

		// Step #3: Integrate --> from accel to new velocity & new position
		// ----------------------------------------------------------------------------------------

		// We will use the 2nd order "Velocity Verlet" method for integration
		//Will use the dt (frame gap) declared on the header, modify it before if desired
		integrator_velocity_verlet(element);

		// Step #4: solve collisions
		// ----------------------------------------------------------------------------------------

			// Solve collision between element and ground
			//Iterate wuith all elements of the world
			p2List_item<PhysBody*>* element_to_check = world.Elements.getFirst();
			while (element_to_check != NULL)
			{
				if (are_colliding(element, element_to_check)) {
					element->data->collisions.add(element_to_check->data->ctype);
				}

				if (element_to_check->data->ctype == ColliderType::GROUND && are_colliding(element, element_to_check)) {
					collision_solver(element, element_to_check);
				}
				//Next element of the world
				element_to_check = element_to_check->next;
			}

		element = element->next;
	}

	//Next element of the world
	return UPDATE_CONTINUE;
}

// 
update_status ModulePhysics::PostUpdate()
{
	p2List_item<PhysBody*>* element = world.Elements.getFirst();
	// Process all elements in the world to clear the collisions stored of the last frame
	while (element != NULL) {
		element->data->collisions.clear();
		element = element->next;
	}

	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModulePhysics::CleanUp()
{
	LOG("Destroying physics world");

	world.Elements.clear();

	return true;
}

PhysBody* ModulePhysics::CreateCircle(float pos_x, float pos_y, float rad, BodyType bodyType) {
	PhysBody* body = new PhysBody(ShapeType::CIRCLE, false);
	body->position.x = pos_x;
	body->position.y = pos_y;
	body->radius = rad;
	body->btype = bodyType;

	body->mass = 10.0f;
	body->surface = 1.0f;
	body->cd = 0.4f;
	body->cl = 1.2f;
	body->b = 10.0f;
	body->coef_friction = 0.9f;
	body->coef_restitution = 0.8f;

	body->velocity.x = 0.0f;
	body->velocity.y = 0.0f;

	body->ctype = ColliderType::UNKNOWN;

	world.Elements.add(body);

	return body;
}

PhysBody* ModulePhysics::CreateRectangle(float pos_x, float pos_y, float w, float h, BodyType bodyType) {
	PhysBody* body = new PhysBody(ShapeType::RECTANGLE, false);
	body->position.x = pos_x;
	body->position.y = pos_y;
	body->w = w;
	body->h = h;
	body->btype = bodyType;

	body->mass = 10.0f;
	body->surface = 1.0f;
	body->cd = 0.4f;
	body->cl = 1.2f;
	body->b = 10.0f;
	body->coef_friction = 0.9f;
	body->coef_restitution = 0.8f;

	body->velocity.x = 0.0f;
	body->velocity.y = 0.0f;


	body->ctype = ColliderType::UNKNOWN;

	world.Elements.add(body);

	return body;
}

PhysBody* ModulePhysics::CreateWaterRectangle(float pos_x, float pos_y, float w, float h) {
	PhysBody* body = new PhysBody(ShapeType::RECTANGLE, true);
	body->position.x = pos_x;
	body->position.y = pos_y;
	body->w = w;
	body->h = h;
	body->btype = BodyType::STATIC;

	body->density = 50.0f;
	body->velocity.x = -1.0f;
	body->velocity.y = 0.0f;

	body->ctype = ColliderType::WATER;

	world.Elements.add(body);

	return body;
}

// Detect collision between circle and rectange
bool ModulePhysics::check_collision_circle_rectangle(float cx, float cy, float cr, float rx, float ry, float rw, float rh)
{
	// Distance from center of circle to center of rectangle
	float dist_x = std::abs(cx - rx);
	float dist_y = std::abs(cy - ry);

	// If circle is further than half-rectangle, not intersecting
	if (dist_x > (rw / 2.0f + cr)) { return false; }
	if (dist_y > (rh / 2.0f + cr)) { return false; }

	// If circle is closer than half-rectangle, is intersecting
	if (dist_x <= (rw / 2.0f)) { return true; }
	if (dist_y <= (rh / 2.0f)) { return true; }

	// If all of above fails, check corners
	float a = dist_x - rw / 2.0f;
	float b = dist_y - rh / 2.0f;
	float cornerDistance_sq = a * a + b * b;
	return (cornerDistance_sq <= (cr * cr));
}

// Detect collision between rectangle and rectange
bool ModulePhysics::check_collision_rectangle_rectangle(float rx1, float ry1, float rw1, float rh1, float rx2, float ry2, float rw2, float rh2)
{
	// Distance from center of rectangle to center of rectangle
	float dist_x = std::abs(rx1 - rx2);
	float dist_y = std::abs(ry1 - ry2);

	// If distance is further than sum of half-rectangles, not intersecting
	if (dist_x > (rw2 / 2.0f + rw1 / 2.0f)) { return false; }
	if (dist_y > (rh2 / 2.0f + rh1 / 2.0f)) { return false; }

	// If distance is closer than sum of half-rectangles, is intersecting
	if (dist_x <= (rw2 / 2.0f + rw1 / 2.0f)) { return true; }
	if (dist_y <= (rh2 / 2.0f + rh1 / 2.0f)) { return true; }
}

// Detect collision between circle and circle
bool ModulePhysics::check_collision_circle_circle(float cx1, float cy1, float cr1, float cx2, float cy2, float cr2)
{
	// Distance from center of circle to center of circle
	float dist = std::abs(pow((cx2-cx1), 2) + pow((cy2-cy1), 2));

	// If distance of circle centers is further than sum of radius, not intersecting
	if (dist <= std::abs(pow((cr1 + cr2), 2))) { return true; }
	else { return false; }
}

bool ModulePhysics::is_colliding_with_water(p2List_item<PhysBody*>* element) {
	bool ret = false;

	//Iterate wuith all elements of the world
	p2List_item<PhysBody*>* element_to_check = world.Elements.getFirst();
	while (element_to_check != NULL)
	{
		//If element is water type, check if colliding
		if (element_to_check->data->IsWater()) {
			if (element_to_check->data->GetShape() == ShapeType::RECTANGLE && element->data->GetShape() == ShapeType::CIRCLE) {
				float rect_x = (element_to_check->data->position.x + element_to_check->data->w / 2.0f); // Center of rectangle
				float rect_y = (element_to_check->data->position.y + element_to_check->data->h / 2.0f); // Center of rectangle
				ret = check_collision_circle_rectangle(element->data->position.x, element->data->position.y, element->data->radius, rect_x, rect_y, element_to_check->data->w, element_to_check->data->h);
			}
			else if (element_to_check->data->GetShape() == ShapeType::RECTANGLE && element->data->GetShape() == ShapeType::RECTANGLE) {
				float rect_x1 = (element->data->position.x + element->data->w / 2.0f); // Center of rectangle1
				float rect_y1 = (element->data->position.y + element->data->h / 2.0f); // Center of rectangle1
				float rect_x2 = (element_to_check->data->position.x + element_to_check->data->w / 2.0f); // Center of rectangle2
				float rect_y2 = (element_to_check->data->position.y + element_to_check->data->h / 2.0f); // Center of rectangle2
				ret = check_collision_rectangle_rectangle(rect_x1, rect_y1, element->data->w, element->data->h, rect_x2, rect_y2, element_to_check->data->w, element_to_check->data->h);
			}
		}

		//If detects one collision with water return true
		if (ret) { return ret; }
		//Else, next element of the world
		element_to_check = element_to_check->next;
	}

	return ret;
}

//Checks collision between two elements
bool ModulePhysics::are_colliding(p2List_item<PhysBody*>* element1, p2List_item<PhysBody*>* element2) {
	if (element1->data->GetShape() == ShapeType::CIRCLE && element2->data->GetShape() == ShapeType::CIRCLE) {
		return check_collision_circle_circle(element1->data->position.x, element1->data->position.y, element1->data->radius, element2->data->position.x, element2->data->position.y, element2->data->radius);
	}
	else if (element1->data->GetShape() == ShapeType::RECTANGLE && element2->data->GetShape() == ShapeType::CIRCLE) {
		float rect_x = (element1->data->position.x + element1->data->w / 2.0f); // Center of rectangle
		float rect_y = (element1->data->position.y + element1->data->h / 2.0f); // Center of rectangle
		return check_collision_circle_rectangle(element2->data->position.x, element2->data->position.y, element2->data->radius, rect_x, rect_y, element1->data->w, element1->data->h);
	}
	else if (element1->data->GetShape() == ShapeType::CIRCLE && element2->data->GetShape() == ShapeType::RECTANGLE) {
		float rect_x = (element2->data->position.x + element2->data->w / 2.0f); // Center of rectangle
		float rect_y = (element2->data->position.y + element2->data->h / 2.0f); // Center of rectangle
		return check_collision_circle_rectangle(element1->data->position.x, element1->data->position.y, element1->data->radius, rect_x, rect_y, element2->data->w, element2->data->h);
	}
	else if (element1->data->GetShape() == ShapeType::RECTANGLE && element2->data->GetShape() == ShapeType::RECTANGLE) {
		float rect_x1 = (element1->data->position.x + element1->data->w / 2.0f); // Center of rectangle1
		float rect_y1 = (element1->data->position.y + element1->data->h / 2.0f); // Center of rectangle1
		float rect_x2 = (element2->data->position.x + element2->data->w / 2.0f); // Center of rectangle2
		float rect_y2 = (element2->data->position.y + element2->data->h / 2.0f); // Center of rectangle2
		return check_collision_rectangle_rectangle(rect_x1, rect_y1, element1->data->w, element1->data->h, rect_x2, rect_y2, element2->data->w, element2->data->h);
	}
}

// Compute modulus of a vector
float ModulePhysics::modulus(float vx, float vy)
{
	return std::sqrt(vx * vx + vy * vy);
}

// Compute Aerodynamic Drag force
p2Point<float> ModulePhysics::compute_aerodynamic_drag(p2Point<float> dforce, p2List_item<PhysBody*>* element)
{
	//check if is activated by debug
	float rel_vel[2] = { element->data->velocity.x - world.atmosphere.windx, element->data->velocity.y - world.atmosphere.windy }; // Relative velocity
	float speed = modulus(rel_vel[0], rel_vel[1]); // Modulus of the relative velocity
	if (speed != 0) {
		float rel_vel_unitary[2] = { rel_vel[0] / speed, rel_vel[1] / speed }; // Unitary vector of relative velocity
		float fdrag_modulus = 0.5f * world.atmosphere.density * speed * speed * element->data->surface * element->data->cd; // Drag force (modulus)
		dforce.x = -rel_vel_unitary[0] * fdrag_modulus; // Drag is antiparallel to relative velocity
		dforce.y = -rel_vel_unitary[1] * fdrag_modulus; // Drag is antiparallel to relative velocity
	}
	else {
		float rel_vel_unitary[2] = { 0, 0 }; // Unitary vector of relative velocity
		float fdrag_modulus = 0.5f * world.atmosphere.density * speed * speed * element->data->surface * element->data->cd; // Drag force (modulus)
		dforce.x = -rel_vel_unitary[0] * fdrag_modulus; // Drag is antiparallel to relative velocity
		dforce.y = -rel_vel_unitary[1] * fdrag_modulus; // Drag is antiparallel to relative velocity
	}
	return dforce;
}

// Compute Hydrodynamic Drag force
p2Point<float> ModulePhysics::compute_hydrodynamic_drag(p2Point<float> dforce, p2List_item<PhysBody*>* element, p2List_item<PhysBody*>* water)
{
	//check if is activated by debug
	float rel_vel[2] = { element->data->velocity.x - water->data->velocity.x, element->data->velocity.y - water->data->velocity.y }; // Relative velocity
	float speed = modulus(rel_vel[0], rel_vel[1]); // Modulus of the relative velocity
	if (speed != 0) {
		float rel_vel_unitary[2] = { rel_vel[0] / speed, rel_vel[1] / speed }; // Unitary vector of relative velocity
		float fdrag_modulus = element->data->b * speed; // Drag force (modulus)
		dforce.x = -rel_vel_unitary[0] * fdrag_modulus; // Drag is antiparallel to relative velocity
		dforce.y = -rel_vel_unitary[1] * fdrag_modulus; // Drag is antiparallel to relative velocity
	}
	else {
		float rel_vel_unitary[2] = { 0, 0 }; // Unitary vector of relative velocity
		float fdrag_modulus = element->data->b * speed; // Drag force (modulus)
		dforce.x = -rel_vel_unitary[0] * fdrag_modulus; // Drag is antiparallel to relative velocity
		dforce.y = -rel_vel_unitary[1] * fdrag_modulus; // Drag is antiparallel to relative velocity
	}

	return dforce;
}

// Compute Hydrodynamic Buoyancy force
p2Point<float> ModulePhysics::compute_hydrodynamic_buoyancy(p2Point<float> bforce, p2List_item<PhysBody*>* element, p2List_item<PhysBody*>* water)
{
	if (element->data->GetShape() == ShapeType::CIRCLE) {
		// Compute submerged area (assume is a rectangle, for simplicity)
		float water_top_level = water->data->position.y + water->data->h; // Water top level y
		float h = 2.0f * element->data->radius; // Element "hitbox" height
		float surf = h * (water_top_level - element->data->position.y); // Submerged surface
		if ((element->data->position.y + element->data->radius) < water_top_level) surf = h * h; // If completely submerged, use just all element area
		surf *= 0.4; // FUYM to adjust values (should compute the area of circle segment correctly instead)

		// Compute Buoyancy force
		double fbuoyancy_modulus = water->data->density * 10.0 * surf; // Buoyancy force (modulus)
		bforce.x = 0.0; // Buoyancy is parallel to pressure gradient
		bforce.y = fbuoyancy_modulus; // Buoyancy is parallel to pressure gradient
	}
	else if (element->data->GetShape() == ShapeType::RECTANGLE) {
		// Compute submerged area
		float water_top_level = water->data->position.y + water->data->h; // Water top level y
		float h = element->data->h; // Element "hitbox" height
		float surf = h * (water_top_level - element->data->position.y); // Submerged surface
		if ((element->data->position.y + (element->data->h / 2)) < water_top_level) surf = h * h; // If completely submerged, use just all element area
		surf *= 0.4; // FUYM to adjust values (should compute the area of circle segment correctly instead)

		// Compute Buoyancy force
		double fbuoyancy_modulus = water->data->density * 10.0 * surf; // Buoyancy force (modulus)
		bforce.x = 0.0; // Buoyancy is parallel to pressure gradient
		bforce.y = fbuoyancy_modulus; // Buoyancy is parallel to pressure gradient
	}

	return bforce;
}

// Integration scheme: Velocity Verlet
void ModulePhysics::integrator_velocity_verlet(p2List_item<PhysBody*>* element)
{
	element->data->position.x += element->data->velocity.x * dt + 0.5f * element->data->acceleration.x * dt * dt;
	element->data->position.y += element->data->velocity.y * dt + 0.5f * element->data->acceleration.y * dt * dt;
	element->data->velocity.x += element->data->acceleration.x * dt;
	element->data->velocity.y += element->data->acceleration.y * dt;
}

void ModulePhysics::collision_solver(p2List_item<PhysBody*>* element, p2List_item<PhysBody*>* element_to_check) {
	if (element->data->GetShape() == ShapeType::CIRCLE) {
		if (((element_to_check->data->position.y + (element_to_check->data->h / 2)) < (element->data->position.y + element->data->radius)) && ((abs((element->data->position.x) - ((element_to_check->data->position.x) + ((element_to_check->data->w) / 2))) < ((element_to_check->data->w) / 2)) && (abs((element->data->position.x + element->data->radius) - ((element_to_check->data->position.x) + ((element_to_check->data->w) / 2))) < ((element_to_check->data->w) / 2)))) {
			// TP element to ground surface
			element->data->position.y = element_to_check->data->position.y + element_to_check->data->h + element->data->radius;

			// Elastic bounce with ground
			element->data->velocity.y = -element->data->velocity.y;

			// FUYM non-elasticity
			element->data->velocity.x *= element->data->coef_friction;
			element->data->velocity.y *= element->data->coef_restitution;
		}
		else if (((element_to_check->data->position.y + (element_to_check->data->h / 2)) >= (element->data->position.y + element->data->radius)) && ((abs((element->data->position.x) - ((element_to_check->data->position.x) + ((element_to_check->data->w) / 2))) < ((element_to_check->data->w) / 2)) && (abs((element->data->position.x + element->data->radius) - ((element_to_check->data->position.x) + ((element_to_check->data->w) / 2))) < ((element_to_check->data->w) / 2)))) {
			// TP element to ground bottom
			element->data->position.y = element_to_check->data->position.y - element->data->radius;
		}
		else if (((element_to_check->data->position.x + (element_to_check->data->w / 2)) < (element->data->position.x + element->data->radius)) && ((abs((element->data->position.y) - ((element_to_check->data->position.y) + ((element_to_check->data->h) / 2))) < ((element_to_check->data->h) / 2)) && (abs((element->data->position.y + element->data->radius) - ((element_to_check->data->position.y) + ((element_to_check->data->h) / 2))) < ((element_to_check->data->h) / 2)))) {
			// TP element to ground left
			element->data->position.x = element_to_check->data->position.x + element_to_check->data->w + element->data->radius;

			// Elastic bounce with ground
			element->data->velocity.x = -element->data->velocity.x;

			// FUYM non-elasticity
			element->data->velocity.y *= element->data->coef_friction;
		}
		else if (((element_to_check->data->position.x + (element_to_check->data->w / 2)) >= (element->data->position.x + element->data->radius)) && ((abs((element->data->position.y) - ((element_to_check->data->position.y) + ((element_to_check->data->h) / 2))) < ((element_to_check->data->h) / 2)) && (abs((element->data->position.y + element->data->radius) - ((element_to_check->data->position.y) + ((element_to_check->data->h) / 2))) < ((element_to_check->data->h) / 2)))) {
			// TP element to ground right
			element->data->position.x = element_to_check->data->position.x - element->data->radius;

			// Elastic bounce with ground
			element->data->velocity.x = -element->data->velocity.x;

			// FUYM non-elasticity
			element->data->velocity.y *= element->data->coef_friction;
		}
		else if ((element_to_check->data->position.y + (element_to_check->data->h / 2)) < (element->data->position.y + element->data->radius)) {
			// TP element to ground surface
			element->data->position.y = element_to_check->data->position.y + element_to_check->data->h + element->data->radius;

			// Elastic bounce with ground
			element->data->velocity.y = -element->data->velocity.y;

			// FUYM non-elasticity
			element->data->velocity.x *= element->data->coef_friction;
			element->data->velocity.y *= element->data->coef_restitution;
		}
		else if ((element_to_check->data->position.y + (element_to_check->data->h / 2)) >= (element->data->position.y + element->data->radius)) {
			// TP element to ground bottom
			element->data->position.y = element_to_check->data->position.y - element->data->radius;
		}
	}
	else if (element->data->GetShape() == ShapeType::RECTANGLE) {
		if (((element_to_check->data->position.y + (element_to_check->data->h / 2)) < (element->data->position.y + (element->data->h / 2))) && ((abs((element->data->position.x) - ((element_to_check->data->position.x) + ((element_to_check->data->w) / 2))) < ((element_to_check->data->w) / 2)) && (abs(((element->data->position.x) + (element->data->w)) - ((element_to_check->data->position.x) + ((element_to_check->data->w) / 2))) < ((element_to_check->data->w) / 2)))) {
			// TP element to ground surface
			element->data->position.y = element_to_check->data->position.y + element_to_check->data->h;

			// Elastic bounce with ground
			element->data->velocity.y = -element->data->velocity.y;

			// FUYM non-elasticity
			element->data->velocity.x *= element->data->coef_friction;
			element->data->velocity.y *= element->data->coef_restitution;
		}
		else if (((element_to_check->data->position.y + (element_to_check->data->h / 2)) >= (element->data->position.y + (element->data->h / 2))) && ((abs((element->data->position.x) - ((element_to_check->data->position.x) + ((element_to_check->data->w) / 2))) < ((element_to_check->data->w) / 2)) && (abs(((element->data->position.x) + (element->data->w)) - ((element_to_check->data->position.x) + ((element_to_check->data->w) / 2))) < ((element_to_check->data->w) / 2)))) {
			// TP element to ground bottom
			element->data->position.y = element_to_check->data->position.y - element->data->h;
		}
		else if (((element_to_check->data->position.x + (element_to_check->data->w / 2)) < (element->data->position.x + (element->data->w / 2))) && ((abs((element->data->position.y) - ((element_to_check->data->position.y) + ((element_to_check->data->h) / 2))) < ((element_to_check->data->h) / 2)) && (abs(((element->data->position.y) + (element->data->h)) - ((element_to_check->data->position.y) + ((element_to_check->data->h) / 2))) < ((element_to_check->data->h) / 2)))) {
			// TP element to ground left
			element->data->position.x = element_to_check->data->position.x + element_to_check->data->w;

			// Elastic bounce with ground
			element->data->velocity.x = -element->data->velocity.x;

			// FUYM non-elasticity
			element->data->velocity.y *= element->data->coef_friction;
		}
		else if (((element_to_check->data->position.x + (element_to_check->data->w / 2)) >= (element->data->position.x + (element->data->w / 2))) && ((abs((element->data->position.y) - ((element_to_check->data->position.y) + ((element_to_check->data->h) / 2))) < ((element_to_check->data->h) / 2)) && (abs(((element->data->position.y) + (element->data->h)) - ((element_to_check->data->position.y) + ((element_to_check->data->h) / 2))) < ((element_to_check->data->h) / 2)))) {
			// TP element to ground right
			element->data->position.x = element_to_check->data->position.x - element->data->w;

			// Elastic bounce with ground
			element->data->velocity.x = -element->data->velocity.x;

			// FUYM non-elasticity
			element->data->velocity.y *= element->data->coef_friction;
		}
		else if ((element_to_check->data->position.y + (element_to_check->data->h / 2)) < (element->data->position.y + (element->data->h / 2))) {
			// TP element to ground surface
			element->data->position.y = element_to_check->data->position.y + element_to_check->data->h;

			// Elastic bounce with ground
			element->data->velocity.y = -element->data->velocity.y;

			// FUYM non-elasticity
			element->data->velocity.x *= element->data->coef_friction;
			element->data->velocity.y *= element->data->coef_restitution;
		}
		else if ((element_to_check->data->position.y + (element_to_check->data->h / 2)) >= (element->data->position.y + (element->data->h / 2))) {
			// TP element to ground bottom
			element->data->position.y = element_to_check->data->position.y - element->data->h;
		}
	}
}

void ModulePhysics::DeleteElementOfWorld() {
	p2List_item<PhysBody*>* to_delete = world.ElementsToDelete.getFirst();
	while (to_delete != NULL)
	{
		world.Elements.del(to_delete);
		world.ElementsToDelete.del(to_delete);
		to_delete = to_delete->next;
	}
}