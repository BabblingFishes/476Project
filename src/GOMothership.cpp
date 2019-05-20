#include "GOMothership.h"

#define PI 3.14159

using namespace std;
using namespace glm;


// constructor
GOMothership::GOMothership(Shape *shape, Texture *texture, float radius, vec3 position, vec3 rotation, vec3 scale , int maxCows) {
  this->shape = shape;
  this->texture = texture;

  material = new Material(
    vec3(0.1745, 0.01175, 0.01175), //amb
    vec3(0.61424, 0.04136, 0.04136), //dif
    vec3(0.0727811, 0.0626959, 0.0626959), //matSpec
    27.90); //shine

  this->radius = radius;
  this->position = position;
  this->rotation = rotation;
  this->scale = scale;
  mass = 0;
  cowsCollected = 0;
  this->maxCows = maxCows;
}

void GOMothership::collect(GOCow *cow) {
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
