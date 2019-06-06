#pragma once
#ifndef __HC_GAMEPLAYER_INCLUDED__
#define __HC_GAMEPLAYER_INCLUDED__

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

class GamePlayer : public GameObject {
private:
  float shipRadius;
  float camPhi;
  float camTheta;
  float camZoom;
  vec3 camPosition;

  void positionCamera();

public:

  GamePlayer(Shape *shape, Texture *texture, vec3 position, vec3 rotation, vec3 scale);


  vec3 getCamPos();
  float getCamPhi();
  float getCamTheta();

  bool borderCollision(vec3 nextPos);
    
  void update(bool *wasdIsDown, bool *arrowIsDown, float timeScale, int Mwidth, int Mheight);

  void draw(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> Model);

  void collide(GOCow *cow);

  void collide(GOHaybale *hay);

  void beamIn(GameObject *other);
};

#endif
