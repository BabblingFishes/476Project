#include "GOBorder.h"

#define PI 3.14159

using namespace std;
using namespace glm;


// constructor
GOBorder::GOBorder(Shape* shape, Texture* texture, float radius, vec3 position, vec3 rotation, vec3 scale) {
	this->shape = shape;
	this->texture = texture;

	material = new Material(
		vec3(0.0, 0.2, 0.0), //amb
		vec3(0.075, 0.2, 0.075), //dif
		vec3(0.35, 0.45, 0.35), //matSpec
		.25); //shine

	this->radius = radius;
	this->position = position;
	this->rotation = rotation;
	this->scale = scale;
	mass = 0;
}