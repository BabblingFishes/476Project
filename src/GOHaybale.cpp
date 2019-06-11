#include "GOHaybale.h"

#define PI 3.14159
#define NUMOBJS 11

using namespace std;
using namespace glm;

// random constructor
GOHaybale::GOHaybale(Shape* shape, Texture* texture, int x, int z) {
	this->shape = shape;
	this->texture = texture;
	//radius = 0.45;
	mass = 0.5;

	material = new Material(
		vec3(0, 0, 0), //amb
		vec3(0.5, 0.5, 0), //dif
		vec3(0.25, 0.25, 0.2), //matSpec
		.001); //shine

	position = vec3(x, 0, z);

	//pick a random rotation
	rotation = vec3(0, (((float)rand() / (RAND_MAX)) * 2 * PI), 0);

	scale = vec3(.75, .45, .45);
	velocity = vec3(0.0);

	physEnabled = true;
  mass = 1;
  bounce = 0.75;
  netForce = vec3(0.0f);

	computeDimensions();

	collected = false;
	idName = GOid::Haybale;
}


bool GOHaybale::isCollected() { return collected; }


bool GOHaybale::update(float timeScale) {
	vec3 oldPos = position;
	move(timeScale);
	return position != oldPos;
}

void GOHaybale::draw(shared_ptr<Program> prog, shared_ptr<MatrixStack> Model) {
	Model->pushMatrix();
	Model->translate(position + vec3(0, height / 2, 0));
	Model->rotate(rotation.x, vec3(1, 0, 0));
	Model->rotate(rotation.y, vec3(0, 1, 0));
	Model->rotate(rotation.z, vec3(0, 0, 1));
	Model->scale(scale);
	if (collected) { //DEBUG
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

//TODO TEST THIS
void GOHaybale::collide(GOMothership *MS) {
	cout << "Surprise, motherfucker" << endl; //DEBUG
}


void GOHaybale::collide(GameObject *other) {
	switch(other->getID()) {
    case GOid::Haybale:
    case GOid::Cow:
      bounceOff(other);
      break;
    case GOid::Mothership:
      if(!collected) {
        collected = true;
        physEnabled = false;
      }
      break;
  }
}
