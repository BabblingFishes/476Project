#pragma once
#ifndef __HC_QUADTREE_INCLUDED__
#define __HC_QUADTREE_INCLUDED__

#include <glad/glad.h>
#include <iostream>
#include <stdlib.h>
#include <list>
#include <queue>
#include "GameObject.h"
#include "GOid.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace std;
using namespace glm;

class QuadTree;

class QuadTree : public std::enable_shared_from_this<QuadTree> {
private:
  vec2 regionMin;
  vec2 regionMax;

  weak_ptr<QuadTree> parent;
  shared_ptr<QuadTree> children[4];

  list<GameObject*> myObjs;
  queue<GameObject*> pendingObjs;

  static bool treeReady;
  static bool treeBuilt;
  int activeNodes; //this is a bitflag

  int lifespan;
  int peakLifespan;


  void checkCollisions(GameObject *obj, vec3 curPos, vec3 nextPos);
  bool isWithin (GameObject *obj);
  bool isWithin (GameObject *obj, vec2 tMin, vec2 tMax);
  void subdivideBounds(vec2 bounds[2][4], vec2 dimensions);
  void subdivideObjs(list<GameObject*> quadList[4], vec2 bounds[2][4]);

  void updateLife();
  list<GameObject*> updateObjs(double timeScale);
  void prune();
  void removeObject(GameObject* obj);
  void updateArrangement(list<GameObject*> movedObjs);



public:
  QuadTree(vec2 regionMin, vec2 regionMax);

  QuadTree(vec2 regionMin, vec2 regionMax, list<GameObject*> objList, shared_ptr<QuadTree> parent);

  void buildTree();
  void processPending();

  bool insert(GameObject *obj);

  void addObject(GameObject *obj);

  bool isColliding(GameObject *obj);
  void update(double timeScale);

  int getLifespan();
  void setLifespan(int i);
  int getObjCount();

  bool hasParent();
  bool hasChildren();
  shared_ptr<QuadTree> getParent();

  void printTree();
};

#endif
