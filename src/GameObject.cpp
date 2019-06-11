#include "GameObject.h"
#include "GOid.h"

#define MAP_WIDTH 200 //TODO remove
#define MAP_LENGTH 75 //TODO remove
#define EPSILON 0.01

using namespace std;
using namespace glm;

GameObject::GameObject(){}

GameObject::GameObject(Shape *shape, Texture *texture, float radius, vec3 position, vec3 rotation, vec3 scale, vec3 velocity) {
  this->shape = shape;
  this->texture = texture;
  //TODO remove radius from constructor (on this and others!)
  this->position = position;
  this->rotation = rotation;
  this->scale = scale;
  this->velocity = velocity;

  physEnabled = false;
  mass = 1;
  bounce = 0.75;
  netForce = vec3(0.0f);

  material = new Material(
    vec3(0.1745, 0.01175, 0.01175), //amb
    vec3(0.61424, 0.04136, 0.04136), //dif
    vec3(0.0727811, 0.0626959, 0.0626959), //matSpec
    27.90); //shine

  computeDimensions();

  idName = GOid::DefaultObject;
}

//computes cylinder collider (height and radius) from current shape and scale
void GameObject::computeDimensions() {
  // compute the radius as the widest part of the shape
  float width = shape->getWidth() * scale.x;
  float length = shape->getLength() * scale.z;
  if (width >= length) radius = width / 2.0;
  else radius = length / 2.0;

  //compute the height from the shape
  height = shape->getHeight() * scale.y;
}


Texture *GameObject::getTexture() { return texture; }
float GameObject::getRadius() { return radius; }
float GameObject::getMass() { return mass; }
float GameObject::getBounce() { return bounce; }
vec3 GameObject::getPos() { return position; }
vec3 GameObject::getRot() { return rotation; }
vec3 GameObject::getScale() { return scale; }
vec3 GameObject::getVel() { return velocity; }
GOid GameObject::getID() { return idName; }

void GameObject::setPos(vec3 position) { this->position = position; }
void GameObject::setRot(vec3 rotation) { this->rotation = rotation; }
void GameObject::setScale(vec3 scale) { this->scale = scale; }
void GameObject::setVel(vec3 velocity) { this->velocity = velocity; }

bool GameObject::isPhysEnabled() { return physEnabled; }

bool GameObject::isColliding(vec3 point) {
  return length(position - point) < radius;
}

bool GameObject::isColliding(GameObject *other) {
  return length(position - other->getPos()) < (radius + other->getRadius());
}

void GameObject::addForce(vec3 force) {
  if(physEnabled) {
    netForce += force;
  }
}

// called once per frame
bool GameObject::update(float timeScale) {
  return false;
}

//Checks if player's next movement is into a tree border, returns true if collision
bool GameObject::borderCollision(vec3 nextPos, int Mwidth, int Mheight) {
    //Get the x and z values of the next position
    float nextX = nextPos.x;
    float nextZ = nextPos.z;
    //cout << "nextX = " << nextX << ", nextZ = " << nextZ << endl;
    //cout << "velocity.z = " << velocity.z << endl;
    //Translate map width and heights to pixel space so it's easier to work with
    //Check outer borders
    if (-nextX < 6 || -nextX > Mwidth-6) { // left side, right side
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
void GameObject::move(float timeScale) {
  //TODO: bounce/spin?

  //gravity
  if(position.y > EPSILON) {
    netForce += vec3(0, -0.01, 0) * mass;
  }
  else {
    position.y = 0;
    if (velocity.y < 0) {
      velocity.y *= -bounce; //we don't have momentum rn
      netForce.y -= netForce.y; //normal force from ground
    }
  }

  velocity *= 1 - (0.02f * timeScale); // ""friction"" TODO
  velocity += netForce * timeScale / mass;

  if (borderCollision(position + velocity, MAP_WIDTH, MAP_LENGTH)){
  //      //cout << "X = " << position.x << ", Y = " << position.y << ", Z = " <<position.z << endl;
  //      //cout << velocity.x << velocity.y << velocity.z << endl;
  }

	position += velocity;
  netForce = vec3(0);
}

//TODO
void GameObject::bounceOff(GameObject *other) {
  //TODO subtract velocity from position until no longer overlapping?
  // no, because the other object is going to call bounceOff(this) -- we need them the same until then
  // could we instead add the velocity that will get them out?
  // jitter may occur

  //if(false) {
  if(other->isPhysEnabled()) {
    float oBounce = other->getBounce();
    vec3 momentum = velocity * mass;
    vec3 oMomentum = other->getVel() * other->getMass();
    //TODO
  }
  else {
    //all nonPhysEnabled objs are cylinders; translate to 2D space
    vec2 cylPos = vec2(position.x, position.z);
    vec2 cylVel = vec2(velocity.x, velocity.z);
    vec2 oCylPos = vec2(other->getPos().x, other->getPos().z);

    //unsure of actual term from this
    // vector pointing from other obj's center to our obj's center
    // useful for distance and tangent
    vec2 centerVec = cylPos - oCylPos;

    /*
      //remove ourselves from the object the same way we came in
      //TODO this requires more trig than i'm equipped for and may not even be
      // better than direct removal (below)
      vec3 outDir = -velocity;

      //NOTE we're normalizing the 3D velocity vector off the 2D velocity
      // such that it is moved completely out of the cylinder regardless of Y
      outDir /= vec3(length(cylVel));
      //we'll move by the amount overlap
      outDir *= length(centerVec)
    */

    //remove ourselves in the quickest way possible
    float overlap = radius + other->getRadius() - length(centerVec);
    position += normalize(vec3(centerVec.x, 0.0, centerVec.y)) * overlap;

    //NOTE reflect() is for light reflection but should work for our purposes
    vec3 newVel = reflect(velocity, normalize(vec3(centerVec.x, 0.0, centerVec.y)));
    velocity = newVel;

    //alternatively:
    // float theta = angleBetween(tangent, velocity);
    // rotate by 2*theta
  }
}

// default collision behavior is nothing
void GameObject::collide(GameObject *other) {
  return;
}

void GameObject::draw(shared_ptr<Program> prog, shared_ptr<MatrixStack> Model) {
  Model->pushMatrix();
    // position is at foot; adjust by height
    Model->translate(position + vec3(0, height / 2, 0));
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
