#include "GOCow.h"
#include <glad/glad.h>
#include <iostream>
#include "GLSL.h"
#include "Program.h"
#include "Shape.h"

using namespace std;
using namespace glm;

GOCow::GOCow(shared_ptr<Shape> shape, float radius, vec3 position, vec3 rotation, vec3 scale, vec3 velocity) {
  this->shape = shape;
  this->radius = radius;
  this->position = position;
  this->rotation = rotation;
  this->scale = scale;
  this->velocity = velocity;
  collected = false;
  toDraw = true;
}

bool GOCow::getDraw() { return toDraw; }
bool GOCow::getCollected() { return collected; }

void GOCow::update() {
  //TODO cow AI goes here
  position += velocity;
}

void GOCow::draw(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> Model) {
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

void GOCow::collect() {
  collected = true;
  toDraw = false;
}
