#pragma once
#ifndef __HC_GAMEOBJECT_INCLUDED__
#define __HC_GAMEOBJECT_INCLUDED__

#include <glad/glad.h>
#include <iostream>
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Texture.h"
#include "Material.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

class GameObject {
protected:
  Shape *shape;
  Texture *texture;
  Material *material;
  float radius;
  float mass;
  vec3 position;
  vec3 rotation;
  vec3 scale;
  vec3 velocity;
  vec3 netForce; //the net force acting on this object in this frame

public:
  GameObject();

  GameObject(Shape *shape, Texture *texture, float radius, vec3 position, vec3 rotation, vec3 scale, vec3 velocity);

  Texture *getTexture();
  float getRadius();
  float getMass();
  vec3 getPos();
  vec3 getRot();
  vec3 getScale();
  vec3 getVel();


  void setPos(vec3 position);
  void setRot(vec3 rotation);
  void setScale(vec3 scale);
  void setVel(vec3 velocity);

  virtual bool isColliding(vec3 point);
  virtual bool isColliding(GameObject *other);

  void addForce(vec3 force);

  virtual bool update(float timeScale);
  bool borderCollision(vec3 nextPos);
  virtual void move(float timeScale);
  virtual void collide(GameObject *other);

  virtual void draw(shared_ptr<Program> prog, shared_ptr<MatrixStack> Model);

};

#endif
