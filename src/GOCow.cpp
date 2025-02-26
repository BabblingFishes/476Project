#include "GOCow.h"

#define PI 3.14159
#define NUMOBJS 11

using namespace std;
using namespace glm;
using namespace irrklang;


GOCow::GOCow(Shape *shape, Texture *texture, float x, float z, Shape** cowWalk) {
  this->shape = shape;
  this->texture = texture;
  this->cowWalk = cowWalk;
  radius = 0.5;
  mass = 1;

  material = new Material(
    vec3(0.04, 0.04, 0.02), //amb
    vec3(0.2, 0.2, 0.1), //dif
    vec3(0.05, 0.04, 0.04), //matSpec
    .005); //shine

  position = vec3(x, 0, z);

  //pick a random rotation
  rotation = vec3(0 , (((float)rand() / (RAND_MAX)) * 2 * PI), 0);

  scale = vec3(0.85);
  velocity = vec3(0.0);

  physEnabled = true;
  mass = 1;
  bounce = 0.75;
  netForce = vec3(0.0f);

  computeDimensions();

  collected = false;

  walkframe = 0;
  framecounter = 0;
  engine = createIrrKlangDevice();
  if (!engine)
	  return;
  moo = engine->addSoundSourceFromFile("../resources/Audio/Moo.ogg");
  idName = GOid::Cow;
}

// specific constructor
GOCow::GOCow(Shape *shape, Texture *texture, Material *material, float radius, vec3 position, vec3 rotation, vec3 scale, vec3 velocity) {
  this->shape = shape;
  this->texture = texture;
  this->material = material;
  //this->radius = radius;
  this->position = position;
  this->rotation = rotation;
  this->scale = scale;
  this->velocity = velocity;

  physEnabled = true;
  mass = 1;
  bounce = 0.75;
  netForce = vec3(0.0f);

  computeDimensions();

  collected = false;
  idName = GOid::Cow;
}

bool GOCow::isCollected()   {   return collected;   }

void GOCow::walk() {
	int frame = walkframe % 10;
	shape = cowWalk[frame];
	framecounter++;
	if (framecounter % 3 == 0) {
		walkframe++;
	}
}


bool GOCow::update(float timeScale) {
  vec3 oldPos = position;

  float moveMagn = 0.0001f; //walkin' power
  if(position.y == 0) { //if on the ground
    //move in the direction of the rotation
	walk();
    netForce += vec3(sin(rotation.y), 0, cos(rotation.y)) * vec3(moveMagn);
  }

  move(timeScale);
  return position != oldPos;
}

void GOCow::draw(shared_ptr<Program> prog, shared_ptr<MatrixStack> Model) {
  Model->pushMatrix();
    Model->translate(position + vec3(0, height / 2, 0));
    Model->rotate(rotation.x, vec3(1, 0, 0));
    Model->rotate(rotation.y, vec3(0, 1, 0));
    Model->rotate(rotation.z, vec3(0, 0, 1));
    Model->scale(scale);
    if(collected) { //DEBUG
      glUniform3f(prog->getUniform("matAmb"), 0.02, 0.04, 0.2);
      glUniform3f(prog->getUniform("matDif"), 0.0, 0.16, 0.9);
      glUniform3f(prog->getUniform("matSpec"), 0.14, 0.2, 0.8);
      glUniform1f(prog->getUniform("shine"), 120.0);
    }
    else {
      material->draw(prog);
	  glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
	  shape->draw(prog);
    }
  Model->popMatrix();
}

void GOCow::collide(GameObject *other) {
  switch(other->getID()) {
    case GOid::Haybale:
    case GOid::Cow:
    case GOid::Tree:
      bounceOff(other);
      break;
    case GOid::Mothership:
      if(!collected) {
        collected = true;
        physEnabled = false;
        if (!engine->isCurrentlyPlaying(moo)) {
	      	engine->play2D(moo);
	      }
      }
      break;
    default:
      break;
  }
}
