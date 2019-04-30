#include <glad/glad.h>
#include <iostream>
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"

/*
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
*/

#include "C:/Users/Josh/Documents/Visual Studio 2019/SDKs/vcpkg-master/packages/glm_x64-windows/include/glm/gtc/type_ptr.hpp"
#include "C:/Users/Josh/Documents/Visual Studio 2019/SDKs/vcpkg-master/packages/glm_x64-windows/include/glm/gtc/matrix_transform.hpp"

using namespace std;
using namespace glm;

#define PLAYER_RADIUS 1.0
#define HEAD_RADIUS 2.0

class GameObject {
private:
  vec3 position;
  vec3 direction;
  vec3 rotation;
  float velocity;
  shared_ptr<Shape> shape;
  bool toDraw;
  bool collected;
  shared_ptr<Program> prog;
  // bounding box
  float minx = position.x - PLAYER_RADIUS;
  float minz = position.z - PLAYER_RADIUS;
  float maxx = position.x + PLAYER_RADIUS;
  float maxz = position.z + PLAYER_RADIUS;

public:
  vec3 getPos();
  vec3 getDir();
  vec3 getRot();
  float getVel();
  bool getDraw();
  bool getCollected();
  float getMinx();
  float getMinz();
  float getMaxx();
  float getMaxz();

  void setPos(vec3 pos);

  GameObject(vec3 position, vec3 direction, float velocity, shared_ptr<Shape> shape, shared_ptr<Program> prog);

  vec3 update(double dt);

  void draw(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> Model);

  void destroy();
};
