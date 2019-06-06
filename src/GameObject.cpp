#include "GameObject.h"

using namespace std;
using namespace glm;

GameObject::GameObject(){}

GameObject::GameObject(Shape *shape, Texture *texture, vec3 position, vec3 rotation, vec3 scale) {
  this->shape = shape;
  this->texture = texture;

  // default material since this will probably be replaced with a normal map
  material = new Material(
    vec3(0.1745, 0.01175, 0.01175), //amb
    vec3(0.61424, 0.04136, 0.04136), //dif
    vec3(0.0727811, 0.0626959, 0.0626959), //matSpec
    27.90); //shine

  // compute the radius as the widest part of the shape
  float width = shape->getWidth() * scale.x;
  float length = shape->getLength() * scale.z;
  if (width >= length) radius = width / 2.0;
  else radius = length / 2.0;

  //compute the height from the shape
  height = shape->getHeight() * scale.y;

  this->position = position;

  this->rotation = rotation;
  this->scale = scale;

  // GameObjects don't have physics enabled by default
  velocity = vec3(0);
  mass = 0;
  netForce = vec3(0);
}

//hyperspecific constructor
GameObject::GameObject(Shape *shape, Texture *texture, Material *material, float radius, float height, vec3 position, vec3 rotation, vec3 scale, vec3 velocity, float mass) {
  this->shape = shape;
  this->texture = texture;
  this->material = material;
  this->radius = radius;
  this->height = height;
  this->position = position;
  this->rotation = rotation;
  this->scale = scale;
  this->velocity = velocity;
  this->mass = mass;
  netForce = vec3(0.0f);
}

Texture *GameObject::getTexture() { return texture; }
float GameObject::getRadius() { return radius; }
float GameObject::getHeight() { return height; }
float GameObject::getMass() { return mass; }
vec3 GameObject::getPos() { return position; }
vec3 GameObject::getRot() { return rotation; }
vec3 GameObject::getScale() { return scale; }
vec3 GameObject::getVel() { return velocity; }
vec3 GameObject::getMidPt() { return position + vec3(0, height / 2, 0); }

void GameObject::setPos(vec3 position) { this->position = position; }
void GameObject::setRot(vec3 rotation) { this->rotation = rotation; }
void GameObject::setScale(vec3 scale) { this->scale = scale; }
void GameObject::setVel(vec3 velocity) { this->velocity = velocity; }

// cylinder @ point collision
bool GameObject::isColliding(vec3 point) {
  return position.y - height < point.y
    && point.y < position.y + height
    && length(vec2(point.x, point.z) - vec2(position.x, position.z)) < radius;
}

// cylinder @ cylinder collision
bool GameObject::isColliding(GameObject *other) {
  vec3 oPos = other->getPos();
  float oHgt = other->getHeight();
  return position.y - height < oPos.y + oHgt
    && oPos.y - oHgt < position.y + height
    && length(vec2(oPos.x, oPos.z) - vec2(position.x, position.z)) < (radius + other->getRadius());
}

// NOTE cylinder @ sphere collisions can't be done efficiently
// and must be computed as cylinder @ cylinder (see above)

void GameObject::addForce(vec3 force) {
  netForce += force;
}

// called once per frame
void GameObject::update(float timeScale) {
  move(timeScale);
}

// uses physics to decide new position
void GameObject::move(float timeScale) {
  float epsilon = 0.1; // near-zero epsilon
  //TODO make this a configurable thing
  float bounce = 0.75; // bounciness constant
  //TODO: spin?

  if(position.y > epsilon) {
    netForce += vec3(0, -0.01, 0) * mass;
  }
  else {
    position.y = 0;
    if (velocity.y < 0) {
      velocity.y *= -bounce; //we don't have momentum rn
      netForce.y -= netForce.y; //normal force from ground
    }
  }

  velocity *= 1 - (0.02f * timeScale); // ""friction"" TODO
  velocity += netForce * timeScale / mass;
  position += velocity;
  netForce = vec3(0);
}

// default collision behavior is nothing
void GameObject::collide(GameObject *other) {
  return;
}

void GameObject::draw(shared_ptr<Program> prog, shared_ptr<MatrixStack> Model) {
  Model->pushMatrix();
    // in this house we adjust by height
    Model->translate(position + vec3(0, height / 2, 0));
    Model->rotate(rotation.x, vec3(1, 0, 0));
    Model->rotate(rotation.y, vec3(0, 1, 0));
    Model->rotate(rotation.z, vec3(0, 0, 1));
    Model->scale(scale);
    if (material) {
      material->draw(prog);
    }
    else {
      cerr << "Object missing material!" << endl; //DEBUG
    }
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
    shape->draw(prog);
  Model->popMatrix();
}
