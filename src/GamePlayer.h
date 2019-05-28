#pragma once
#ifndef __HC_GAMEPLAYER_INCLUDED__
#define __HC_GAMEPLAYER_INCLUDED__

#include <glad/glad.h>
#include <iostream>
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "GameObject.h"
#include "GOCow.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

#define PLAYER_RADIUS 1.0
#define HEAD_RADIUS 2.0

class GamePlayer : public GameObject {
private:
  float shipRadius;
  float camPhi;
  float camTheta;
  float camZoom;
  vec3 camPosition;

  void positionCamera();

public:
    vec3 getCamPos();
    vec3 getPos();
    
  GamePlayer(shared_ptr<Shape> shape, vec3 position, vec3 rotation, vec3 scale);

  void update(std::shared_ptr<MatrixStack> View, bool *wasdIsDown, bool *arrowIsDown);

  void draw(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> Model);

  void collide(GOCow *cow);

  void beamIn(GameObject *other);
};

#endif
