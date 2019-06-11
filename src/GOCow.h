#pragma once
#ifndef __HC_GOCOW_INCLUDED__
#define __HC_GOCOW_INCLUDED__

#include <glad/glad.h>
#include <iostream>
#include <irrKlang.h>
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Texture.h"
#include "Material.h"
#include "GameObject.h"
#include "GOMothership.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <irrKlang/irrKlang.h>

using namespace std;
using namespace glm;
using namespace irrklang;

#define PLAYER_RADIUS 1.0
#define HEAD_RADIUS 2.0

class GOCow : public GameObject {
private:
  bool collected;
  int walkframe;
  int framecounter;
  ISoundEngine* engine;
  ISoundSource* moo;
  Shape** cowWalk;

public:
  //random constructor
  GOCow(Shape *shape, Texture *texture, float x,float z, Shape** cowWalk);

  //specific constructor
  GOCow(Shape *shape, Texture *texture, Material *material, float radius, vec3 position, vec3 rotation, vec3 scale, vec3 velocity);

  bool isCollected();

  void walk();
  bool update(float timeScale);
  
  void draw(shared_ptr<Program> prog, shared_ptr<MatrixStack> Model);

  void collide(GameObject *other);
};

#endif
