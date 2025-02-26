#pragma once
#ifndef __HC_GROUND_INCLUDED__
#define __HC_GROUND_INCLUDED__

#include <glad/glad.h>
#include <iostream>
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Texture.h"
#include "Material.h"
#include "GameObject.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

class Ground {
private:
  Shape *shape;
  Texture *texture;
  Material *material;
  float width;
  float length;

public:
  Ground(Shape *shape, Texture *texture, float width, float length);

  Texture *getTexture();

  bool isColliding(vec3 point);
  bool isColliding(GameObject *gameObj);

  void draw(shared_ptr<Program> prog, shared_ptr<MatrixStack> Model);
};

#endif
