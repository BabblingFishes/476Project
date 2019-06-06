#include "GOCow.h"

#define PI 3.14159

using namespace std;
using namespace glm;

// random constructor
GOCow::GOCow(Shape *shape, Texture *texture, int worldSize) {
  this->shape = shape;
  this->texture = texture;
  mass = 1;

  material = new Material(
    vec3(0.1745, 0.01175, 0.01175), //amb
    vec3(0.61424, 0.04136, 0.04136), //dif
    vec3(0.0727811, 0.0626959, 0.0626959), //matSpec
    27.90); //shine

  scale = vec3(0.5);

  // compute the radius as the widest part of the shape
  float width = shape->getWidth() * scale.x;
  float length = shape->getLength() * scale.z;
  if (width >= length) radius = width / 2.0;
  else radius = length / 2.0;

  //compute the height from the shape
  height = shape->getHeight() * scale.y;

  //pick some random position
  int randXPos = (((float)rand() / (RAND_MAX)) * worldSize * 2) - worldSize;
  int randZPos = (((float)rand() / (RAND_MAX)) * worldSize * 2) - worldSize;
  position = vec3(randXPos, 5, randZPos);

  //pick a random Y rotation
  rotation = vec3(0 , (((float)rand() / (RAND_MAX)) * 2 * PI), 0);
  velocity = vec3(0.0);
  collected = false;
}

// specific constructor
GOCow::GOCow(Shape *shape, Texture *texture, Material *material, float radius, vec3 position, vec3 rotation, vec3 scale, vec3 velocity) {
  this->shape = shape;
  this->texture = texture;
  this->material = material;
  this->radius = radius;
  this->position = position;
  this->rotation = rotation;
  this->scale = scale;
  this->velocity = velocity;
  mass = 1; //TODO
  collected = false;
}

bool GOCow::isCollected() { return collected; }

// sphere collision for cows
bool GOCow::isColliding(GOCow *other) {
  return length(getMidPt() - other->getMidPt()) < (radius + other->getRadius());
}

// cylinder @ cylinder collision
bool GOCow::isColliding(GameObject *other) {
  vec3 oPos = other->getPos();
  float oHgt = other->getHeight();
  return position.y - height < oPos.y + oHgt
    && oPos.y - oHgt < position.y + height
    && length(vec2(oPos.x, oPos.z) - vec2(position.x, position.z)) < (radius + other->getRadius());
}

void GOCow::update(float timeScale) {
  float moveMagn = 0.0001f; //walkin' power
  if(position.y == 0) { //if on the ground
    //move in the direction of the rotation
    netForce += vec3(sin(rotation.y), 0, cos(rotation.y)) * vec3(moveMagn);
  }
  move(timeScale);
}

void GOCow::collect() {
  if (!collected) {
    collected = true;
    material = new Material(
      vec3(0.02, 0.04, 0.2), //amb
      vec3(0.0, 0.16, 0.9), //dif
      vec3(0.14, 0.2, 0.8), //matSpec
      120.0);
  }
}

void GOCow::collide(GOCow *other) {
  float bounce = 0.75; //TODO see bounce constant in GameObject::move()
  float oBounce = 0.75;
  vec3 momentum = velocity * mass;
  vec3 oMomentum = other->getVel() * other->getMass();

}

/* catchall for other objs */
void GOCow::collide(GameObject *other) {
  return;
}
