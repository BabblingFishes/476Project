#pragma once
#ifndef __HC_GAMEOBJECT_INCLUDED__
#define __HC_GAMEOBJECT_INCLUDED__

#include <glad/glad.h>
#include <iostream>
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

class GameObject {
protected:
  float radius;
  shared_ptr<Shape> shape;
  vec3 position;
  vec3 rotation;
  vec3 scale;
  vec3 velocity;

  GameObject();

public:
  float getRadius();
  vec3 getPos();
  vec3 getRot();
  vec3 getScale();
  vec3 getVel();

  void setPos(vec3 position);
  void setRot(vec3 rotation);
  void setScale(vec3 scale);
  void setVel(vec3 velocity);

  GameObject(shared_ptr<Shape> shape, float radius, vec3 position, vec3 rotation, vec3 scale, vec3 velocity);

  bool isColliding(vec3 point);

  virtual void update();

  virtual void draw(shared_ptr<Program> prog, shared_ptr<MatrixStack> Model);
};

#endif
