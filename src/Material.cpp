#include "Material.h"
#include "GLSL.h"
#include "Program.h"

using namespace std;
using namespace glm;

Material::Material(vec3 ambient, vec3 diffuse, vec3 specular, float shine) {
  this->ambient = ambient;
  this->diffuse = diffuse;
  this->specular = specular;
  this->shine = shine;
}

//adds material to current object
void Material::draw(shared_ptr<Program> prog) {
  glUniform3f(prog->getUniform("matAmb"), ambient.r, ambient.g, ambient.b);
  glUniform3f(prog->getUniform("matDif"),  diffuse.r, diffuse.g, diffuse.b);
  glUniform3f(prog->getUniform("matSpec"),  specular.r, specular.g, specular.b);
  glUniform1f(prog->getUniform("shine"), shine);
}
