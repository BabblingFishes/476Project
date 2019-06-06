#include "GOCow.h"

#define PI 3.14159
#define NUMOBJS 11

using namespace std;
using namespace glm;


GOCow::GOCow(Shape *shape, Texture *texture, int x, int z) {
  this->shape = shape;
  this->texture = texture;
  radius = 0.5;
  mass = 1;

  material = new Material(
    vec3(0.04, 0.04, 0.02), //amb
    vec3(0.2, 0.2, 0.1), //dif
    vec3(0.05, 0.04, 0.04), //matSpec
    .005); //shine

  position = vec3(x, -1, z);

  //pick a random rotation
  rotation = vec3(0 , (((float)rand() / (RAND_MAX)) * 2 * PI), 0);

  scale = vec3(0.85);
  velocity = vec3(0.0);
  collected = false;
}

// specific constructor
GOCow::GOCow(Shape *shape, Texture *texture, Material *material, float radius, vec3 position, vec3 rotation, vec3 scale, vec3 velocity) {
  this->shape = shape;
  this->texture = texture;
  this->material = material;
  this->radius = radius;
  this->position = position;
  this->rotation = rotation;
  this->scale = scale;
  this->velocity = velocity;
  mass = 1; //TODO
  collected = false;
}

bool GOCow::isCollected()   {   return collected;   }


void GOCow::update(float timeScale, int Mwidth, int Mheight) {
  float moveMagn = 0.0001f; //walkin' power
  if(position.y == 0) { //if on the ground
    //move in the direction of the rotation
    netForce += vec3(sin(rotation.y), 0, cos(rotation.y)) * vec3(moveMagn);
  }
  move(timeScale, Mwidth, Mheight);
}

void GOCow::draw(shared_ptr<Program> prog, shared_ptr<MatrixStack> Model) {
  Model->pushMatrix();
    Model->translate(position);
    Model->rotate(rotation.x, vec3(1, 0, 0));
    Model->rotate(rotation.y, vec3(0, 1, 0));
    Model->rotate(rotation.z, vec3(0, 0, 1));
    Model->scale(scale);
	Model->translate(vec3(0, -0.65, 0)); //TODO: remove this with new plane
    if(collected) { //DEBUG
      glUniform3f(prog->getUniform("matAmb"), 0.02, 0.04, 0.2);
      glUniform3f(prog->getUniform("matDif"), 0.0, 0.16, 0.9);
      glUniform3f(prog->getUniform("matSpec"), 0.14, 0.2, 0.8);
      glUniform1f(prog->getUniform("shine"), 120.0);
    }
    else {
      material->draw(prog);
    }
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
    shape->draw(prog);
  Model->popMatrix();
}


void GOCow::collect() {
  collected = true;
}
