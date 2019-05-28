#pragma once
#ifndef __HC_VFC_INCLUDED__
#define __HC_VFC_INCLUDED__

#include <glad/glad.h>
#include <iostream>
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "WindowManager.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

    mat4 SetProjectionMatrix(shared_ptr<Program> curShade);
    mat4 SetOrthoMatrix(shared_ptr<Program> curShade);
    mat4 SetTopView(shared_ptr<Program> curShade, vec3 camPosition, vec3 position);
    mat4 SetView(shared_ptr<Program> curShade, vec3 camPosition, vec3 position);
    void ExtractVFPlanes(mat4 P, mat4 V);
    float DistToPlane(float A, float B, float C, float D, vec3 point);
    int ViewFrustCull(vec3 center, float radius, bool cull);
    
#endif
