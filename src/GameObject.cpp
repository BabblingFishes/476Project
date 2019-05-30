#include "GameObject.h"

using namespace std;
using namespace glm;

GameObject::GameObject(){}

GameObject::GameObject(Shape *shape, tTexture *texture, float radius, vec3 position, vec3 rotation, vec3 scale, vec3 velocity) {
  this->shape = shape;
  this->texture = texture;
  this->radius = radius;
  this->position = position;
  this->rotation = rotation;
  this->scale = scale;
  this->velocity = velocity;
  mass = 1; //TODO
  netForce = vec3(0.0f);
  material = new Material(
    vec3(0.1745, 0.01175, 0.01175), //amb
    vec3(0.61424, 0.04136, 0.04136), //dif
    vec3(0.0727811, 0.0626959, 0.0626959), //matSpec
    27.90); //shine
}

tTexture *GameObject::getTexture() { return texture; }
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
void GameObject::update(float timeScale) {
  move(timeScale);
}

// uses physics to decide new position
void GameObject::move(float timeScale) {
  //TODO: add gravity
  //TODO: spin?

  velocity *= 1 - (0.02f * timeScale); // ""friction"" TODO
  velocity += netForce * timeScale / mass;
  position += velocity;
  netForce = vec3(0);

  /* if(position.y > 0) { // ""gravity"" TODO
    position
  } */

  if(position.y < 0) {
    position.y -= position.y;
  } //TODO we need actual ground collision but this is a fix for now
}

// default collision behavior
// TODO: bounce & spin?
void GameObject::collide(GameObject *other) {
  return;
}

void GameObject::draw(shared_ptr<Program> prog, shared_ptr<MatrixStack> Model) {
  Model->pushMatrix();
    //TODO for trees: Model->translate(vec3(0, 4, 0));
    Model->translate(position);
    Model->rotate(rotation.x, vec3(1, 0, 0));
    Model->rotate(rotation.z, vec3(0, 0, 1));
    Model->rotate(rotation.y, vec3(0, 1, 0));
    Model->scale(scale);
    if (material) {
      material->draw(prog);
    }
    else {
      cerr << "Object missing material!" << endl;
    }
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
    shape->draw(prog);
  Model->popMatrix();
}
