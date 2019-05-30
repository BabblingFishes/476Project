#include "GamePlayer.h"

using namespace std;
using namespace glm;

GamePlayer::GamePlayer(Shape *shape, tTexture *texture, vec3 position, vec3 rotation, vec3 scale) {
  this->shape = shape;
  this->texture = texture;
  this->material = material;
  radius = 10; //this is the beam radius
  mass = 1;
  this->position = position + vec3(0, 1, 0); //this baby flies
  this->rotation = rotation;
  this->velocity = velocity;
  this->scale = scale;
  velocity = vec3(0);
  netForce = vec3(0);

  material = new Material(
    vec3(0.3294, 0.2235, 0.02745), //amb
    vec3(0.7804, 0.5686, 0.11373), //dif
    vec3(0.9922, 0.9412, 0.8078), //matSpec
    27.90); //shine

  shipRadius = 1;
  camPhi = rotation.x;
  camTheta = rotation.y;
  camZoom = 10;
  positionCamera();
}

vec3 GamePlayer::getCamPos() { return camPosition; }
float GamePlayer::getCamPhi() { return camPhi; }
float GamePlayer::getCamTheta() { return camTheta; }

void GamePlayer::positionCamera() {
  vec3 cameraForward = vec3(cos(camPhi) * sin(camTheta),
                            -sin(camPhi),
                            cos(camPhi) * cos(camTheta));
  camPosition = position - (cameraForward * camZoom);
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
  //material
  material->draw(prog);
  //draw
  glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
  shape->draw(prog);
  Model->popMatrix();
}

/* moves the player and camera */
//TODO this needs to be done real-time
//TODO the View logic can probably be abstracted out
void GamePlayer::update(bool *wasdIsDown, bool *arrowIsDown, float timeScale) {
  //TODO after implementing real-time, make these values constants
  float moveMagn = 0.01f; //force from player controls
  //float mass = 1; // player mass
  float rotSpeed = 0.1f * timeScale; // UFO spin speed

  //TODO drift camera too? or is that bad for motion sickness
  float cameraSpeed = 0.02f * timeScale; //camera rotation acceleration

  //spin UFO
  rotation += vec3(0, rotSpeed, 0);

  // camera rotation
  if (arrowIsDown[0]) camPhi = max(camPhi - cameraSpeed, 0.0f); // no clipping thru the floor
  if (arrowIsDown[1]) camTheta -= cameraSpeed;
  if (arrowIsDown[2]) camPhi = min(camPhi + cameraSpeed, 1.56f); // no flipping the camera
  if (arrowIsDown[3]) camTheta += cameraSpeed;

  //player orientation
  vec3 playerForward = normalize(vec3(sin(camTheta),
                            0,
                            cos(camTheta)));
  vec3 playerLeft = normalize(cross(vec3(0, 1, 0), playerForward));

  vec3 zForce = playerForward * moveMagn;
  vec3 xForce = playerLeft * moveMagn;

  //player controls
  if (wasdIsDown[0]) netForce += zForce;
  if (wasdIsDown[1]) netForce -= xForce;
  if (wasdIsDown[2]) netForce -= zForce;
  if (wasdIsDown[3]) netForce += xForce;

  move(timeScale);

  // place the camera,pointed at the player
  positionCamera();
}

void GamePlayer::collide(GOCow *cow) {
    if(!cow->isCollected()) {
        beamIn(cow);
    }
}

//moves an object towards the gravitation beam
void GamePlayer::beamIn(GameObject *other) {
  float beamStrength = 0.01;

  vec3 dir = position - other->getPos();
  //TODO str / distance
  float dist = length(dir);
  vec3 force = normalize(dir) * (float)(beamStrength / max(pow(dist, 2.0), 0.5)); //TODO might divide again by a mass-based beam constant?
  other->addForce(force);

  //this...is very broken
}
