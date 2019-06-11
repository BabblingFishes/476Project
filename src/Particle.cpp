//
// sueda - geometry edits Z. Wood
// 3/16
//

#include <iostream>
#include "Particle.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Texture.h"


float randFloat(float l, float h)
{
	float r = rand() / (float) RAND_MAX;
	return (1.0f - r) * l + r * h;
}

void Particle::load()
{
	// Random initialization
	rebirth(0.0f);
}

// all particles born at the origin
void Particle::rebirth(float t)
{
	charge = randFloat(0.0f, 1.0f) < 0.5f ? -1.0f : 1.0f;
	m = 1.0f;
	d = randFloat(0.0f, 0.02f);
	x.x = 0;
	x.y = 0;
	x.z = randFloat(-2.f, 1.f);
	v.x = randFloat(-1.0f, 1.0f);
	v.y = randFloat(-1.0f, 1.0f);
	v.z = randFloat(-1.0f, 1.0f);
	lifespan = randFloat(0.3f, 0.5f);
	tEnd = t + lifespan;

	scale = randFloat(9.0f, 10.0f);
	color.r = randFloat(0.75f, 1.0f);
	color.g = randFloat(0.75f, 1.0f);
	color.b = randFloat(0.0f, 0.2f);
	color.a = 1.0f;
}

void Particle::update(float t, float h, const vec3& g, bool sparking)
{
	if (sparking) {
		if (t > tEnd)
		{
			rebirth(t);
		}
	}

	// very simple update
	x += h * v + (g * 0.1f);
	scale = 10.f;
	color.a = (tEnd - t) / lifespan;
}
