#include "GameObject.h"
#include <glad/glad.h>
#include <iostream>
#include "GLSL.h"
#include "Program.h"
#include "Shape.h"

using namespace std;
using namespace glm;

float GameObject::getRadius() { return radius; }
vec3 GameObject::getPos() { return position; }
vec3 GameObject::getRot() { return rotation; }
vec3 GameObject::getScale() { return scale; }
vec3 GameObject::getVel() { return velocity; }

void GameObject::setPos(vec3 position) { this->position = position; }
void GameObject::setRot(vec3 rotation) { this->rotation = rotation; }
void GameObject::setScale(vec3 scale) { this->scale = scale; }
void GameObject::setVel(vec3 velocity) { this->velocity = velocity; }

GameObject::GameObject() {}

GameObject::GameObject(shared_ptr<Shape> shape, float radius, vec3 position, vec3 rotation, vec3 scale, vec3 velocity) {
  this->shape = shape;
  this->radius = radius;
  this->position = position;
  this->rotation = rotation;
  this->scale = scale;
  this->velocity = velocity;
}

bool GameObject::isColliding(vec3 point) {
  return length(position - point) < radius;
}

void GameObject::update() {
  return;
}

void GameObject::draw(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> Model) {
  Model->pushMatrix();
    Model->translate(position);
    Model->rotate(rotation.x, vec3(1, 0, 0));
    Model->rotate(rotation.z, vec3(0, 0, 1));
    Model->rotate(rotation.y, vec3(0, 1, 0));
    //TODO set material here
    glUniform3f(prog->getUniform("matAmb"), 0.02, 0.04, 0.2);
    glUniform3f(prog->getUniform("matDif"), 0.0, 0.16, 0.9);
    glUniform3f(prog->getUniform("matSpec"), 0.14, 0.2, 0.8);
    glUniform1f(prog->getUniform("shine"), 120.0);
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
    shape->draw(prog);
  Model->popMatrix();
}
