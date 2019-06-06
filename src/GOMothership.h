#pragma once
#ifndef __HC_GOMOTHERSHIP_INCLUDED__
#define __HC_GOMOTHERSHIP_INCLUDED__

#include <glad/glad.h>
#include <iostream>
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Texture.h"
#include "Material.h"
#include "GameObject.h"
#include "GOCow.h"
#include "GOHaybale.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

#define PLAYER_RADIUS 1.0
#define HEAD_RADIUS 2.0

class GOMothership : public GameObject {
private:
  int cowsCollected;
  int maxCows;
  int hayCollected;
  int maxHay;

public:
  GOMothership(Shape *shape, Texture *texture, float radius, vec3 position, vec3 rotation, vec3 scale, int maxCows, int maxHay);

  void collect(GOCow *cow);

  void collect(GOHaybale *hay);
};

#endif
