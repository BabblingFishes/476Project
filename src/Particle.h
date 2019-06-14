//
// sueda
// November, 2014/ wood 16
//

#pragma once

#ifndef LAB471_PARTICLE_H_INCLUDED
#define LAB471_PARTICLE_H_INCLUDED

#include <vector>
#include <memory>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class MatrixStack;
class Program;
class Texture;


class Particle
{

public:

	void load();
	void rebirth(float t);
	void update(float t, float h, const glm::vec3 &g, bool sparking);
	const vec3 &getPosition() const { return x; };
	const vec3 &getVelocity() const { return v; };
	const vec3& getRotation() const { return r; };
	const vec4 &getColor() const { return color; };
	float getScale() { return scale; };

private:

	float charge = 1.f; // +1 or -1
	float m = 1.f; // mass
	float d = 0.f; // viscous damping
	glm::vec3 r = glm::vec3(0.f); // rotation
	glm::vec3 x = glm::vec3(0.f, 1.0f, 0.f); // position
	glm::vec3 v = glm::vec3(2.f); // velocity
	float lifespan = 1.f; // how long this particle lives
	float tEnd = 0.f;     // time this particle dies
	float scale = 1.f;
	glm::vec4 color = glm::vec4(1.f);

};

// Sort particles by their z values in camera space
class ParticleSorter
{

public:

	bool operator() (const std::shared_ptr<Particle> p0, const std::shared_ptr<Particle> p1) const
	{
		// Particle positions in world space
		const vec3 &x0 = p0->getPosition();
		const vec3 &x1 = p1->getPosition();

		// Particle positions in camera space
		vec4 x0w = C * glm::vec4(x0.x, x0.y, x0.z, 1.0f);
		vec4 x1w = C * glm::vec4(x1.x, x1.y, x1.z, 1.0f);
		return x0w.z < x1w.z;
	}

	// current camera matrix
	mat4 C;

};

#endif // LAB471_PARTICLE_H_INCLUDED
