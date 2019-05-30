#include "GameObject.h"

#define MAP_WIDTH 120
#define MAP_LENGTH 162

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
void GameObject::update(float timeScale) {
  move(timeScale);
}

//Checks if player's next movement is into a tree border, returns true if collision
bool GameObject::borderCollision(vec3 nextPos) {
    //Get the x and z values of the next position
    float nextX = nextPos.x;
    float nextZ = nextPos.z;
    //cout << "nextX = " << nextX << ", nextZ = " << nextZ << endl;
    //cout << "velocity.z = " << velocity.z << endl;
    //Translate map width and heights to pixel space so it's easier to work with
    int width = MAP_WIDTH / 2;
    int length = MAP_LENGTH / 2;
    //Check outer borders
    if (nextX < -width + 2 || width - 2 < nextX) {
        velocity *= vec3(-1, 1, 1);
        //cout << "nextX = " << nextX << ", nextZ = " << nextZ << endl;
        //cout << "OUTSIDE WIDTH: " << endl;
        return true;
    }
    if (nextZ < -length - 2 || nextZ > length + 2) {
        velocity *= vec3(1, 1, -1);
        //cout << "OUTSIDE LENGTH" << endl;
        return true;
    }
    //Check inner borders, one if block per for loop block in generateMap()
    // -width < x < -40, -20 < x < width, -32 < z < -28
    if (nextZ <= -26 && nextZ >= -34) {
        if (nextX <= -40 || nextX >= -21) {
            if (nextZ < -26.3 && nextZ > -33.7) {
                velocity *= vec3(-1, 1, 1);
            }
            else velocity *= vec3(1, 1, -1);
            //cout << "INNER FIRST" << endl;
            return true;
        }
    }
    // -width < x < 0, -2 < z < 2
    if (nextZ >= -4 && nextZ <= 4) {
        if (nextX <= 2) {
            //velocity *= vec3(1, 1, -1);
            if (nextZ > -3.7 && nextZ < -3.7) {
                velocity *= vec3(-1, 1, 1);
            }
            else velocity *= vec3(1, 1, -1);
            //cout << "X = " << position.x << ", Y = " << position.y << ", Z = " <<position.z << endl;
            //cout << "INNER SECOND" << endl;
            return true;
        }
    }
    // -2 < x < 2, -5 < z < length - 25
    if (nextX >= -4 && nextX <= 4) {
        if (nextZ >= -1 && nextZ <= length - 24) {
            if (nextX > -1.7 && nextX < 1.7) {
                velocity *= vec3(1, 1, -1);
             }
            else velocity *= vec3(-1, 1, 1);
            //cout << "X = " << position.x << ", Y = " << position.y << ", Z = " <<position.z << endl;
            //cout << "INNER THIRD" << endl;
            return true;
        }
    }
    //cout << "NO COLLISION" << endl;
    return false;
}

// uses physics to decide new position
void GameObject::move(float timeScale) {
  //TODO: add gravity
  //TODO: spin?

  velocity *= 1 - (0.02f * timeScale); // ""friction"" TODO
    velocity += netForce * timeScale / mass;
    
    if (borderCollision(position + velocity)){
        //cout << "X = " << position.x << ", Y = " << position.y << ", Z = " <<position.z << endl;
        //cout << velocity.x << velocity.y << velocity.z << endl;
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
