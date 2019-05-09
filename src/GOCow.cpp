#include "GOCow.h"

#define PI 3.14159

using namespace std;
using namespace glm;

// random constructor
GOCow::GOCow(std::shared_ptr<Shape> shape, int worldSize) {
  this->shape = shape;
  radius = 0.5;
  mass = 1;

  //pick some random position
  int randXPos = (((float)rand() / (RAND_MAX)) * worldSize * 2) - worldSize;
  int randZPos = (((float)rand() / (RAND_MAX)) * worldSize * 2) - worldSize;
  position = vec3(randXPos, 0, randZPos);

  //pick a random rotation
  rotation = vec3(0 , (((float)rand() / (RAND_MAX)) * 2 * PI), 0);

  scale = vec3(1, 1, 1);
  velocity = vec3(0, 0, 0);
  collected = false;
  vec3 prevPos = vec3(0,0,0);
}

// specific constructor
GOCow::GOCow(shared_ptr<Shape> shape, float radius, vec3 position, vec3 rotation, vec3 scale, vec3 velocity) {
  this->shape = shape;
  this->radius = radius;
  this->position = position;
  this->rotation = rotation;
  this->scale = scale;
  this->velocity = velocity;
  mass = 1; //TODO
  collected = false;
  vec3 prevPos = vec3(0,0,0);
}

/*bool GOCow::checkInGoal(vec3 MSpos, float MSrad) {
    if (length(MSpos - position) < MSrad) {
        inGoal = true;
    }
    else {
        inGoal = false;
    }
}*/

//bool GOCow::isInGoal()   {   return inGoal;   }

bool GOCow::isCollected()   {   return collected;   }

void GOCow::update() {
  //prevPos = position;
  float moveMagn = 0.0001f; //walkin' power
  if(position.y == 0) { //if on the ground
    //move in the direction of the rotation
    netForce += vec3(sin(rotation.y), 0, cos(rotation.y)) * vec3(moveMagn);
  }
  move();
}

void GOCow::draw(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> Model) {
  Model->pushMatrix();
    Model->translate(position);
    Model->rotate(rotation.x, vec3(1, 0, 0));
    Model->rotate(rotation.y, vec3(0, 1, 0));
    Model->rotate(rotation.z, vec3(0, 0, 1));
    Model->scale(scale);
    if(collected) {
      glUniform3f(prog->getUniform("matAmb"), 0.02, 0.04, 0.2);
      glUniform3f(prog->getUniform("matDif"), 0.0, 0.16, 0.9);
      glUniform3f(prog->getUniform("matSpec"), 0.14, 0.2, 0.8);
      glUniform1f(prog->getUniform("shine"), 120.0);
    }
    else {
      glUniform3f(prog->getUniform("matAmb"), 0.1745f, 0.01175f, 0.01175f);
			glUniform3f(prog->getUniform("matDif"), 0.61424f, 0.04136f, 0.04136f);
			glUniform3f(prog->getUniform("matSpec"), 0.0727811f, 0.0626959f, 0.0626959f);
			glUniform1f(prog->getUniform("shine"), 0.1);
    }
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
    shape->draw(prog);
  Model->popMatrix();
}

void GOCow::collect(vec3 MSpos, float MSrad, int &numCollected) {
    if (length(MSpos - position) < MSrad) {
        /*if (prevPos != position) {
            numCollected++;
            if (numCollected == 11) {
                cout << "Mission Accomplished! You've colleced all of the cows!" << endl;
            }
            else {
                cout << "Number of Cows Collected:" << numCollected << endl;
            }
        }*/
        collected = true;
    }
}

