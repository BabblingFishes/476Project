#include "GameObject.h"

using namespace std;
using namespace glm;

GameObject::GameObject(){}

GameObject::GameObject(Shape *shape, Texture *texture, float radius, vec3 position, vec3 rotation, vec3 scale, vec3 velocity) {
  this->shape = shape;
  this->texture = texture;
  this->radius = radius;
  this->position = position;
  this->rotation = rotation;
  this->scale = scale;
  this->velocity = velocity;
  mass = 1; //TODO
  netForce = vec3(0.0f);
  material = new Material(
    vec3(0.1745, 0.01175, 0.01175), //amb
    vec3(0.61424, 0.04136, 0.04136), //dif
    vec3(0.0727811, 0.0626959, 0.0626959), //matSpec
    27.90); //shine
}

Texture *GameObject::getTexture() { return texture; }
float GameObject::getRadius() { return radius; }
float GameObject::getMass() { return mass; }
vec3 GameObject::getPos() { return position; }
vec3 GameObject::getRot() { return rotation; }
vec3 GameObject::getScale() { return scale; }
vec3 GameObject::getVel() { return velocity; }

void GameObject::setPos(vec3 position) { this->position = position; }
void GameObject::setRot(vec3 rotation) { this->rotation = rotation; }
void GameObject::setScale(vec3 scale) { this->scale = scale; }
void GameObject::setVel(vec3 velocity) { this->velocity = velocity; }

bool GameObject::isColliding(vec3 point) {
  return length(position - point) < radius;
}

bool GameObject::isColliding(GameObject *other) {
  return length(position - other->getPos()) < (radius + other->getRadius());
}

void GameObject::addForce(vec3 force) {
  netForce += force;
}

// called once per frame
void GameObject::update(float timeScale, int Mwidth, int Mheight) {
  move(timeScale, Mwidth, Mheight);
}

//Checks if player's next movement is into a tree border, returns true if collision
bool GameObject::borderCollision(vec3 nextPos, int Mwidth, int Mheight) {
    //Get the x and z values of the next position
    float nextX = nextPos.x;
    float nextZ = nextPos.z;
	int hwidth = -Mwidth;
	int hheight = Mheight /2;
    //cout << "nextX = " << nextX << ", nextZ = " << nextZ << endl;
    //cout << "velocity.z = " << velocity.z << endl;
    //Translate map width and heights to pixel space so it's easier to work with
    //Check outer borders
    if (-nextX < 6 || -nextX > Mwidth-9) { // left side, right side
        velocity *= vec3(-1, 1, 1);
        //cout << "nextX = " << nextX << ", nextZ = " << nextZ << endl;
        //cout << "OUTSIDE WIDTH: " << endl;
        return true;
    }
    if (nextZ < 6 || nextZ > Mheight - 6) { //bottom, top
        velocity *= vec3(1, 1, -1);
        //cout << "OUTSIDE LENGTH" << endl;
        return true;
    }
    //cout << "NO COLLISION" << endl;
    return false;
}

// uses physics to decide new position
void GameObject::move(float timeScale, int Mwidth, int Mheight) {
  //TODO: add gravity
  //TODO: spin?

  velocity *= 1 - (0.02f * timeScale); // ""friction"" TODO
    velocity += netForce * timeScale / mass;
	//cout << position.x << " " << position.z << endl;
    
   if (borderCollision(position + velocity, Mwidth, Mheight)){
  //      //cout << "X = " << position.x << ", Y = " << position.y << ", Z = " <<position.z << endl;
  //      //cout << velocity.x << velocity.y << velocity.z << endl;
    }
	position += velocity;
    
  netForce = vec3(0);

  /* if(position.y > 0) { // ""gravity"" TODO
    position
  } */

  if(position.y < 0) {
    position.y -= position.y;
  } //TODO we need actual ground collision but this is a fix for now
}

// default collision behavior
// TODO: bounce & spin?
void GameObject::collide(GameObject *other) {
  return;
}

void GameObject::draw(shared_ptr<Program> prog, shared_ptr<MatrixStack> Model) {
  Model->pushMatrix();
    //TODO for trees: Model->translate(vec3(0, 4, 0));
    Model->translate(position);
    Model->rotate(rotation.x, vec3(1, 0, 0));
    Model->rotate(rotation.z, vec3(0, 0, 1));
    Model->rotate(rotation.y, vec3(0, 1, 0));
    Model->scale(scale);
    if (material) {
      material->draw(prog);
    }
    else {
      cerr << "Object missing material!" << endl;
    }
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
    shape->draw(prog);
  Model->popMatrix();
}
