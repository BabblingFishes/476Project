#include "GOTree.h"

#define PI 3.14159

using namespace std;
using namespace glm;


// constructor
GOTree::GOTree(Shape* shape, Texture* texture, float radius, vec3 position, vec3 rotation, vec3 scale) {
	this->shape = shape;
	this->texture = texture;

	material = new Material(
		vec3(0.0, 0.5, 0.0), //amb
		vec3(0.1, 0.35, 0.1), //dif
		vec3(0.4, 0.55, 0.45), //matSpec
		.25); //shine

	//this->radius = radius;
	this->position = position;
	this->rotation = rotation;
	this->scale = scale;

	physEnabled = false;
  mass = 1;
  bounce = 0.75;
  netForce = vec3(0.0f);

	computeDimensions();

	idName = GOid::Tree;
}
