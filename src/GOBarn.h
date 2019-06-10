#pragma once
#ifndef __HC_GOBARN_INCLUDED__
#define __HC_GOBARN_INCLUDED__

#include <glad/glad.h>
#include <iostream>
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Texture.h"
#include "Material.h"
#include "GameObject.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

class GOBarn : public GameObject {

public:
	GOBarn(Shape* shape, Texture* texture, vec3 position, vec3 rotation, vec3 scale);
};

#endif
