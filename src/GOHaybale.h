#pragma once
#ifndef __HC_GOHAYBALE_INCLUDED__
#define __HC_GOHAYBALE_INCLUDED__

#include <glad/glad.h>
#include <iostream>
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Texture.h"
#include "Material.h"
#include "GameObject.h"
#include "GOMothership.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

#define PLAYER_RADIUS 1.0
#define HEAD_RADIUS 2.0

class GOHaybale : public GameObject {
private:
	bool collected;

public:
	GOHaybale(Shape* shape, Texture* texture, float x, float z);

	bool isCollected();

	bool update(float timeScale);
	void draw(shared_ptr<Program> prog, shared_ptr<MatrixStack> Model);

	void collide(GOMothership *MS); //TODO TEST THIS
	void collide(GameObject *other);
};

#endif
