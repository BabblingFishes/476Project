#include "GOBarn.h"

using namespace std;
using namespace glm;


// constructor
GOBarn::GOBarn(Shape* shape, Texture* texture, vec3 position, vec3 rotation, vec3 scale) {
	this->shape = shape;
	this->texture = texture;

	material = new Material(
		vec3(.32, 0, 0), //amb
		vec3(0.5, 0.0, 0.0), //dif
		vec3(0.3, 0.2, 0.2), //matSpec
		.15); //shine

	this->position = position;
	this->rotation = rotation;
	this->scale = scale;
}
