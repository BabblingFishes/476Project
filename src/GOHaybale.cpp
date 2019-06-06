#include "GOHaybale.h"

#define PI 3.14159
#define NUMOBJS 11

using namespace std;
using namespace glm;

// random constructor
GOHaybale::GOHaybale(Shape* shape, Texture* texture, int x, int z) {
	this->shape = shape;
	this->texture = texture;
	radius = 0.45;
	mass = 0.5;

	material = new Material(
		vec3(0, 0, 0), //amb
		vec3(0.5, 0.5, 0), //dif
		vec3(0.25, 0.25, 0.2), //matSpec
		.001); //shine

	position = vec3(x, 0, z);

	//pick a random rotation
	rotation = vec3(0, (((float)rand() / (RAND_MAX)) * 2 * PI), 0);

	scale = vec3(.35, .25, .25);
	velocity = vec3(0.0);
	collected = false;
}


bool GOHaybale::isCollected() { return collected; }


void GOHaybale::update(float timeScale, int Mwidth, int Mheight) {
	move(timeScale, Mwidth, Mheight);
}

void GOHaybale::draw(shared_ptr<Program> prog, shared_ptr<MatrixStack> Model) {
	Model->pushMatrix();
	Model->translate(position);
	Model->rotate(rotation.x, vec3(1, 0, 0));
	Model->rotate(rotation.y, vec3(0, 1, 0));
	Model->rotate(rotation.z, vec3(0, 0, 1));
	Model->translate(vec3(0, -0.75, 0)); //TODO: remove this with new plane
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


void GOHaybale::collect() {
	collected = true;
}
