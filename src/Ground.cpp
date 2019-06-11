#include "Ground.h"

using namespace std;
using namespace glm;

Ground::Ground(Shape *shape, Texture *texture, float width, float length) {
  this->shape = shape;
  this->texture = texture;
  this->width = width;
  this->length = length;

  material = new Material(
    vec3(0.015, 0.2, 0.05), //amb
    vec3(0.05, 0.3, 0.1), //dif
	vec3(0.1, 0.1, 0.1), //matSpec
    0.01); //shine

  /*glUniform3f(curProg->getUniform("matAmb"), 0.1913f, 0.0735f, 0.0225f);
  glUniform3f(curProg->getUniform("matDif"), 0.7038f, 0.27048f, 0.0828f);
  glUniform3f(curProg->getUniform("matSpec"), 0.256777f, 0.137622f, 0.086014f);
  glUniform1f(curProg->getUniform("shine"), 12.8);*/
}

Texture *Ground::getTexture() { return texture; }

bool Ground::isColliding(vec3 point) {
  return point.y <= 0;
}

bool Ground::isColliding(GameObject *gameObj) {
  return gameObj->getPos().y <= gameObj->getRadius();
}

void Ground::draw(shared_ptr<Program> prog, shared_ptr<MatrixStack> Model) {
  Model->pushMatrix();
    Model->translate(vec3(0, -1.95, 0));
    Model->scale(vec3(width*1.5, 1, length*1.5)); //TODO
    material->draw(prog);
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
    shape->draw(prog);
  Model->popMatrix();
}
