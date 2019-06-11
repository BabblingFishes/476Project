#include "GamePlayer.h"
#include "Particle.h"

#define MAP_WIDTH 200
#define MAP_LENGTH 75
#define EPSILON 0.01

using namespace std;
using namespace glm;
using namespace irrklang;

GamePlayer::GamePlayer(Shape *shape, Texture *texture, vec3 position, vec3 rotation, vec3 scale) {
  this->shape = shape;
  this->texture = texture;
  this->material = material;

  radius = 10; //this is the beam radius
  mass = 1;
  this->position = position;
  this->rotation = rotation;
  this->velocity = velocity;
  this->scale = scale;
  velocity = vec3(0);
  beamStrength = 0.02; //TODO adjust as necessary

  physEnabled = true;
  mass = 10; //TODO adjust as necessary
  bounce = 0.75;
  netForce = vec3(0.0f);

  material = new Material(
	vec3(0.1, 0.13, 0.2), //amb
	vec3(0.337, 0.49, 0.275), //dif
	vec3(0.14, 0.2, 0.8), //spec
	25); //shine

  computeDimensions();
  //save the model's radius as the ship radius
  shipRadius = radius;
  //larger base radius for gravitation beam
  radius = 10;

  camPhi = rotation.x;
  camTheta = rotation.y;
  camZoom = 10; //10
  positionCamera();

  sparking = false;

  engine = createIrrKlangDevice();
  if (!engine)
	  return;
  boing = engine->addSoundSourceFromFile("../resources/Audio/Boing.ogg");
  idName = GOid::Player;
}

bool GamePlayer::getSparking() { return sparking; }

float GamePlayer::getCamPhi() { return camPhi; }
float GamePlayer::getCamTheta() { return camTheta; }
vec3 GamePlayer::getCamPos() { return camPosition; }


void GamePlayer::positionCamera() {
  vec3 cameraForward = vec3(cos(camPhi) * sin(camTheta),
                            -sin(camPhi),
                            cos(camPhi) * cos(camTheta));
  camPosition = position + vec3(0, height / 2, 0) - (cameraForward * camZoom);
}


void GamePlayer::draw(shared_ptr<Program> prog, shared_ptr<MatrixStack> Model){
  //player model
  Model->pushMatrix();
    Model->translate(position + vec3(0, (height / 2) + 1.5, 0));
    Model->rotate(rotation.x, vec3(1, 0, 0));
    Model->rotate(rotation.z, vec3(0, 0, 1));
    Model->rotate(rotation.y, vec3(0, 1, 0));
     // offset for wobble
    Model->rotate(0.2, vec3(0, 0, 1));
    Model->translate(vec3(0.3, 0, 0));
    //material
    if (material) {
      material->draw(prog);
    }
    else {
      cerr << "Object missing material!" << endl;
    }
    //draw
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
    shape->draw(prog);
  Model->popMatrix();
}

void GamePlayer::doControls(bool *wasdIsDown, bool *arrowIsDown) {
  this->wasdIsDown = wasdIsDown;
  this->arrowIsDown = arrowIsDown;
}

/* moves the player and camera */
//TODO this needs to be done real-time
//TODO the View logic can probably be abstracted out

bool GamePlayer::update(float timeScale) {
  //TODO after implementing real-time, make these values constants
  float moveMagn = 0.1f; //force from player controls
  float rotSpeed = 0.1f * timeScale; // UFO spin speed

  //TODO drift camera too? or is that bad for motion sickness
  float cameraSpeed = 0.02f * timeScale; //camera rotation acceleration

  //spin UFO
  rotation += vec3(0, rotSpeed, 0);

  // camera rotation
  if (arrowIsDown[0]) camPhi = std::max(camPhi - cameraSpeed, 0.0f); // no clipping thru the floor
  if (arrowIsDown[1]) camTheta -= cameraSpeed;
  if (arrowIsDown[2]) camPhi = std::min(camPhi + cameraSpeed, 1.56f); // no flipping the camera
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

  movePlayer(timeScale, MAP_WIDTH, MAP_LENGTH);

  return true;
}

// uses physics to decide new position
void GamePlayer::movePlayer(float timeScale, int Mwidth, int Mheight) {
    //TODO: bounce/spin?

  //gravity
  if(position.y > EPSILON) {
    netForce += vec3(0, -0.01, 0) * mass;
  }
  else {
    position.y = 0;
    if (velocity.y < 0) {
      velocity.y *= -bounce; //we don't have momentum rn
      netForce.y -= netForce.y; //normal force from ground
    }
  }

  velocity *= 1 - (0.02f * timeScale); // ""friction"" TODO
  velocity += netForce * timeScale / mass;

  if (borderCollision(position + velocity, MAP_WIDTH, MAP_LENGTH)){
    sparking = true;
		engine->play2D(boing);
  }
  if (!engine->isCurrentlyPlaying(boing))
	{
		sparking = false;
	}

	position += velocity;
  netForce = vec3(0);
}

void GamePlayer::collide(GameObject *other) {
  switch(other->getID()) {
    case GOid::Haybale:
    case GOid::Cow:
      beamIn(other);
      break;
    case GOid::Tree:
      shipCollide(other);
      break;
  }
}

//moves an object towards the gravitation beam
void GamePlayer::beamIn(GameObject *other) {
  //pull towards top of the ship
  vec3 dir = position + vec3(0, height, 0) - other->getPos();
  float dist = length(dir);
  vec3 force = normalize(dir) * (float)(beamStrength / std::max(pow(dist, 2.0), 0.5)); //TODO might divide again by a mass-based beam constant?
  other->addForce(force);
}

//cheap hack to do ship collision with the correct radius
//note this is currently ONLY for use with trees...
void GamePlayer::shipCollide(GameObject *other) {
  float tempRadius = radius;
  radius = shipRadius;

  if(isColliding(other)) {
    bounceOff(other);
  }

  radius = tempRadius;
}
