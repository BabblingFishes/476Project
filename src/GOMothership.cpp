#include "GOMothership.h"

#define PI 3.14159

using namespace std;
using namespace glm;


// constructor
GOMothership::GOMothership(Shape *shape, Texture *texture, float radius, vec3 position, vec3 rotation, vec3 scale , int maxCows, int maxHay) {
  this->shape = shape;
  this->texture = texture;

  material = new Material(
	  vec3(0.1, 0.13, 0.2), //amb
	  vec3(0.337, 0.49, 0.275), //dif
	  vec3(0.14, 0.2, 0.8), //matSpec
	  .01); //shine

  this->radius = radius;
  this->position = position;
  this->rotation = rotation;
  this->scale = scale;
  mass = 0;
  cowsCollected = 0;
  this->maxCows = maxCows;

  hayCollected = 0;
  this->maxHay = maxHay;
}

void GOMothership::collect(GOCow* cow) {
	if (!(cow->isCollected())) {
		cow->collect();
		cowsCollected++;
		if (cowsCollected == maxCows) { //TODO pass max in constructor
			cout << "Mission Accomplished! You've colleced all of the cows!" << endl;
		}
		else {
			cout << "Number of Cows Collected:" << cowsCollected << endl;
		}
	}
}

  void GOMothership::collect(GOHaybale * hay) {
	  if (!(hay->isCollected())) {
		  hay->collect();
		  hayCollected++;
		  if (hayCollected == maxHay) { //TODO pass max in constructor
			  cout << "Oh No! You've colleced all of the hay!" << endl;
		  }
		  else {
			  cout << "Number of Hay Collected:" << hayCollected << endl;
		  }
	  }
  }
