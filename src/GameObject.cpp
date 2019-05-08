#include "GameObject.h"

using namespace std;
using namespace glm;

GameObject::GameObject(){}

GameObject::GameObject(shared_ptr<Shape> shape, float radius, vec3 position, vec3 rotation, vec3 scale, vec3 velocity) {
  this->shape = shape;
  this->radius = radius;
  this->position = position;
  this->rotation = rotation;
  this->scale = scale;
  this->velocity = velocity;
  mass = 1; //TODO
  netForce = vec3(0.0f);
}

float GameObject::getRadius() { return radius; }
float GameObject::getMass() { return mass; }
vec3 GameObject::getPos() { return position; }
vec3 GameObject::getRot() { return rotation; }
vec3 GameObject::getScale() { return scale; }
vec3 GameObject::getVel() { return velocity; }

void GameObject::setPos(vec3 position) { this->position = position; }
void GameObject::setRot(vec3 rotation) { this->rotation = rotation; }
void GameObject::setScale(vec3 scale) { this->scale = scale; }
void GameObject::setVel(vec3 velocity) { this->velocity = velocity; }

bool GameObject::isColliding(vec3 point) {
  return length(position - point) < radius;
}

bool GameObject::isColliding(GameObject *other) {
  return length(position - other->getPos()) < (radius + other->getRadius());
}

void GameObject::addForce(vec3 force) {
  netForce += force;
}

// called once per frame
void GameObject::update() {
  move();
}

// uses physics to decide new position
void GameObject::move() {
  //TODO: add gravity
  //TODO: spin?

  velocity *= 0.98f; // ""friction"" TODO
  //velocity += netForce * timePassed / mass
  velocity += netForce / mass;
  position += velocity;
  netForce = vec3(0);

  if(position.y < 0) {
    position.y -= position.y;
  } //TODO we need actual ground collision but this is a fix for now
}

// default collision behavior
// TODO: bounce & spin?
void GameObject::collide(GameObject *other) {
  return;
}

void GameObject::draw(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> Model) {
  Model->pushMatrix();
    //First translate to put scaled trees back on ground plane
    Model->translate(vec3(0, 4, 0));
    Model->translate(position);
    Model->rotate(rotation.x, vec3(1, 0, 0));
    Model->rotate(rotation.z, vec3(0, 0, 1));
    Model->rotate(rotation.y, vec3(0, 1, 0));
    //Model->scale(scale);
    //TODO set material here
    glUniform3f(prog->getUniform("matAmb"), 0.02, 0.04, 0.2);
    glUniform3f(prog->getUniform("matDif"), 0.0, 0.16, 0.9);
    glUniform3f(prog->getUniform("matSpec"), 0.14, 0.2, 0.8);
    glUniform1f(prog->getUniform("shine"), 120.0);
    Model->scale(scale);
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
    shape->draw(prog);
  Model->popMatrix();
}
