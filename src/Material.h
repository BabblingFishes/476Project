#pragma once
#ifndef __HC_MATERIAL_INCLUDED__
#define __HC_MATERIAL_INCLUDED__

#include <glad/glad.h>
#include "glm/glm.hpp"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"


using namespace std;
using namespace glm;

class Material {
private:
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shine;

public:
  Material(vec3 ambient, vec3 diffuse, vec3 specular, float shine);

  void draw(shared_ptr<Program> prog);
};

#endif
