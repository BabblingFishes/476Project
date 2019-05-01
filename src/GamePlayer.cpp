#include "GamePlayer.h"
#include <glad/glad.h>
#include <iostream>
#include "GLSL.h"
#include "Program.h"
#include "Shape.h"

using namespace std;
using namespace glm;

GamePlayer::GamePlayer(shared_ptr<Shape> shape, vec3 position, vec3 rotation, vec3 scale) {
  this->shape = shape;
  this->position = position;
  this->rotation = rotation;
  this->velocity = velocity;
  this->scale = scale;

  camPhi = rotation.x;
  camTheta = rotation.y;
  camZoom = -10;
  //maybe move this somewhere more accessible
  vec3 cameraForward = vec3(cos(camPhi) * -sin(camTheta),
                            sin(camPhi),
                            cos(camPhi) * -cos(camTheta));
  camPosition = position - (cameraForward * camZoom);
}

void GamePlayer::pointCamera(shared_ptr<MatrixStack> View) {
  View->lookAt(camPosition, position, vec3(0, 1, 0));
}

void GamePlayer::draw(shared_ptr<Program> prog, shared_ptr<MatrixStack> Model){
  //player model
  Model->pushMatrix();
  Model->translate(position);
  Model->rotate(rotation.x, vec3(1, 0, 0));
  Model->rotate(rotation.z, vec3(0, 0, 1));
  Model->rotate(rotation.y, vec3(0, 1, 0));
   // offset for wobble
  Model->rotate(0.2, vec3(0, 0, 1));
  Model->translate(vec3(0.3, 0, 0));
  //material TODO
  glUniform3f(prog->getUniform("matAmb"), 0.3294, 0.2235, 0.02745);
  glUniform3f(prog->getUniform("matDif"), 0.7804, 0.5686, 0.11373);
  glUniform3f(prog->getUniform("matSpec"), 0.9922, 0.9412, 0.8078);
  glUniform1f(prog->getUniform("shine"), 27.90);
  //draw
  glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
  shape->draw(prog);
  Model->popMatrix();
}

/* moves the player and camera */
//TODO this needs to be done real-time
//TODO the View logic can probably be abstracted out
vec3 GamePlayer::update(shared_ptr<MatrixStack> View, bool *wasdIsDown, bool *arrowIsDown) {
  //TODO after implementing real-time, make these values constants
  float moveMagn = 0.01f; //player force
  //float mass = 1; // player mass
  float friction = 0.98f;
  float rotSpeed = 0.1f; // rotationSpeed

  //TODO implement drifty camera too
  float cameraSpeed = 0.02f; //camera rotation acceleration

  rotation += vec3(0, rotSpeed, 0);

  // camera rotation
  if (arrowIsDown[0]) camPhi =  std::max(camPhi - cameraSpeed, 0.0f); // no clipping thru the floor
  if (arrowIsDown[1]) camTheta -= cameraSpeed;
  if (arrowIsDown[2]) camPhi = std::min(camPhi + cameraSpeed, 1.56f); // no flipping the camera
  if (arrowIsDown[3]) camTheta += cameraSpeed;

  //player and camera orientation
  vec3 cameraForward = vec3(cos(camPhi) * -sin(camTheta),
                            sin(camPhi),
                            cos(camPhi) * -cos(camTheta));
  //TODO normalize?
  vec3 playerForward = normalize(vec3(-sin(camTheta),
                            0,
                            -cos(camTheta)));
  vec3 playerLeft = normalize(cross(playerForward, vec3(0, 1, 0)));

  //NOTE generally, vec3 velocity += (vec3 acceleration * float timePassed)

  vec3 zAccel = playerForward * moveMagn;
  vec3 xAccel = playerLeft * moveMagn;

  //player velocity

  vec3 acceleration = vec3(0, 0, 0);


  //player controls
  if (wasdIsDown[0]) acceleration -= zAccel;
  if (wasdIsDown[1]) acceleration -= xAccel;
  if (wasdIsDown[2]) acceleration += zAccel;
  if (wasdIsDown[3]) acceleration += xAccel;

  //acceleration = normalize(acceleration) * moveMagn / mass;
  //TODO gravity
  //TODO more accurate friction
  velocity *= friction;// ""friction""
  velocity += acceleration;


  position += velocity;

  //NOTE this was for updating the collision box
  /*minx = position.x - HEAD_RADIUS;
  minz = position.z - HEAD_RADIUS;
  maxx = position.x + HEAD_RADIUS;
  maxz = position.z + HEAD_RADIUS;*/

  // place the camera, pointed at the player
  //TODO this can be abstracted out maybe
  camPosition = position - (cameraForward * camZoom);
  pointCamera(View);
  return position;
}
