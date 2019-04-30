#include "GameObject.h"
#include <glad/glad.h>
#include <iostream>
#include "GLSL.h"
#include "Program.h"
#include "Shape.h"

using namespace std;
using namespace glm;

vec3 GameObject::getPos() { return position; }
vec3 GameObject::getDir() { return direction; }
vec3 GameObject::getRot() { return rotation; }
float GameObject::getVel() { return velocity; }
bool GameObject::getDraw() { return toDraw; }
bool GameObject::getCollected() { return collected; }
float GameObject::getMinx() { return minx; }
float GameObject::getMinz() { return minz; }
float GameObject::getMaxx() { return maxx; }
float GameObject::getMaxz() { return maxz; }

void GameObject::setPos(vec3 pos) { position = pos; }

GameObject::GameObject(vec3 position, vec3 direction, float velocity, shared_ptr<Shape> shape, shared_ptr<Program> prog) {
  this->position = position;
  this->direction = direction;
  this->velocity = velocity;
  this->shape = shape;
  this->prog = prog;

  collected = false;
  toDraw = true;
  rotation = vec3(0, 0, 0);
  minx = position.x - HEAD_RADIUS;
  minz = position.z - HEAD_RADIUS;
  maxx = position.x + HEAD_RADIUS;
  maxz = position.z + HEAD_RADIUS;
}

vec3 GameObject::update(double dt) {
  vec3 move = direction * velocity * (float)dt;
  vec3 newPos = position + move;
  position = newPos;
  minx = position.x - HEAD_RADIUS;
  minz = position.z - HEAD_RADIUS;
  maxx = position.x + HEAD_RADIUS;
  maxz = position.z + HEAD_RADIUS;

  return newPos;
}

void GameObject::draw(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> Model) {
  if (toDraw) {
    Model->pushMatrix();
      Model->translate(position);
      Model->rotate(radians(-90.f), vec3(1, 0, 0));
      glUniform3f(prog->getUniform("matAmb"), 0.02, 0.04, 0.2);
      glUniform3f(prog->getUniform("matDif"), 0.0, 0.16, 0.9);
      glUniform3f(prog->getUniform("matSpec"), 0.14, 0.2, 0.8);
      glUniform1f(prog->getUniform("shine"), 120.0);
      glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
      shape->draw(prog);
    Model->popMatrix();
  }
}

void GameObject::destroy() {
  toDraw = false;
}
