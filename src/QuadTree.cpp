#include "QuadTree.h"

using namespace std;
using namespace glm;

#define MAX_LIFESPAN 64
#define MIN_SIZE 1

bool QuadTree::treeReady = false;
bool QuadTree::treeBuilt = false;

QuadTree::QuadTree(vec2 regionMin, vec2 regionMax) {
  this->regionMin = regionMin;
  this->regionMax = regionMax;
  //NOTE parent is intentionally not set
  myObjs = list<GameObject*>();
  pendingObjs = queue<GameObject*>();
  lifespan = -1;
  peakLifespan = 8;
  activeNodes = 0;
}

QuadTree::QuadTree(vec2 regionMin, vec2 regionMax, list<GameObject*> objList, shared_ptr<QuadTree> parent) {
  this->regionMin = regionMin;
  this->regionMax = regionMax;
  this->parent = parent;
  myObjs = objList;
  pendingObjs = queue<GameObject*>();
  lifespan = -1;
  peakLifespan = 8;
  activeNodes = 0;
}


// allows external objects to be added
void QuadTree::addObject(GameObject *obj) {
  pendingObjs.push(obj);
  treeReady = false;
}


// deals with pending objects
void QuadTree::processPending() {
  if(treeBuilt) {
    while(!pendingObjs.empty()) {
      insert(pendingObjs.front());
      pendingObjs.pop();
    }
  }
  else {
    while(!pendingObjs.empty()) {
      myObjs.push_back(pendingObjs.front());
      pendingObjs.pop();
    }
    buildTree();
  }
  treeReady = true; //TODO buildTree already does this
}


//returns the mins and maxes (in the form of vec2 xz coords) of subdivisions as bounds[x][y]
// where for x, 0=min, 1=max
// and for y, 0,1,2,3 are the quadrants in reading order (l->r, top->bottom)
// NOTE: we pass in the dimensions because we've already calculated them
//   in the is-this-a-leaf check
void QuadTree::subdivideBounds(vec2 bounds[2][4], vec2 dimensions) {
  vec2 half = dimensions / 2.0f;
  vec2 center = regionMin + half;

  bounds[0][0] = regionMin;
  bounds[1][0] = center;

  bounds[0][1] = vec2(center.x, regionMin.y);
  bounds[1][2] = vec2(regionMax.x, center.y);

  bounds[0][2] = center;
  bounds[1][2] = regionMax;

  bounds[0][3] = vec2(regionMin.x, center.y);
  bounds[1][3] = vec2(center.x, regionMax.y);
}


//divides this tree's objects among its subtrees where applicable
void QuadTree::subdivideObjs(list<GameObject*> quadList[4], vec2 bounds[2][4]) {
  for(int i = 0; i < 4; i++) {
    quadList[i] = list<GameObject*>();
  }

  for(list<GameObject*>::iterator cur = myObjs.begin(); cur != myObjs.end(); cur++) {
    for(int q = 0; q < 4; q++) {
      if(isWithin(*cur, bounds[0][q], bounds[1][q])) {
        quadList[q].push_back(*cur);
        cur = myObjs.erase(cur);
        q = 4; //break
      }
    }
  }
}


void QuadTree::buildTree() {
  //check we're not a leaf
  vec2 dimensions = regionMax - regionMin;
  if(myObjs.size() > 1 && (dimensions.x > MIN_SIZE || dimensions.y > MIN_SIZE)){

    // calculate subtree bounds
    vec2 bounds[2][4];
    subdivideBounds(bounds, dimensions);

    // sort objects into subtrees
    list<GameObject*> quadList[4];
    subdivideObjs(quadList, bounds);

    //create nodes
    for(int q = 0; q < 4; q++) {
      if(quadList[q].size() != 0) {
        children[q] = make_shared<QuadTree>(bounds[0][q], bounds[1][q], quadList[q], shared_from_this());
        activeNodes |= (unsigned char)(1 << q);
        children[q]->buildTree();
      }
    }

    //TODO while it's unlikely this will get multithreaded or anything, it seems too early to set these
    treeBuilt = true;
    treeReady = true;
  }
}


// progresses deletion countdown
void QuadTree::updateLife() {
  // unused leaf
  if(myObjs.size() == 0 && hasChildren() == false) { //NOTE the example actually separates these, but that seems unnecessary
    if(lifespan == -1) {
      //TODO hey, shouldn't it die when the lifespan < 0 ???
      lifespan = peakLifespan;
    }
    else if(lifespan > 0) {
      lifespan--;
    }
    //TODO see above
  }
  // used leaf
  else {
    if(lifespan != -1){
      if(peakLifespan <= MAX_LIFESPAN) {
        peakLifespan *= 2;
      }
      lifespan = -1;
    }
  }
}


// updates all objects in the tree
// returns a list of the ones that have moved
list<GameObject*> QuadTree::updateObjs(double timeScale) {
  list<GameObject*> movedObjs;

  for(list<GameObject*>::iterator cur = myObjs.begin(); cur != myObjs.end(); cur++) {
    if((*cur)->update(timeScale)) {
      movedObjs.push_back(*cur);
    }
  }

  return movedObjs;
}


// remove dead children
void QuadTree::prune() {
  for(int q = 0; q < 4; q++){
    if(children[q] != nullptr && children[q]->getLifespan() == 0) {
      if (children[q]->getObjCount() > 0) { //TODO DEBUG redundant safeguard
        cout << "Attempted to delete a used node. Resetting node life." << endl;
        children[q]->setLifespan(-1);
      }
      else {
        children[q] = nullptr;
        activeNodes &= ~(unsigned char)(1 << q);
      }
    }
  }
}


//removes an object from this node (and this node only!)
void QuadTree::removeObject(GameObject* obj) {
  myObjs.remove(obj);
}


//rearranges the objects listed
void QuadTree::updateArrangement(list<GameObject*> movedObjs) {
  //rearranged moved objects
  for(list<GameObject*>::iterator curObj = movedObjs.begin(); curObj != movedObjs.end(); curObj++) {
    shared_ptr<QuadTree> curTree = shared_from_this();

    //hop up as high as necessary
    while(!curTree->isWithin(*curObj)) {
      if(curTree->hasParent()) {
        curTree = curTree->getParent();
      }
      else {
        cout << "WARNING: Object left maximum bounds. It has been removed from the QuadTree." << endl;
        curTree->removeObject(*curObj);
        break;
      }
    }

    myObjs.remove(*curObj); //TODO could this have been done earlier?
    curTree->insert(*curObj);
  }
}


// update loop, updates tree and all GO's within
void QuadTree::update(double timeScale) {

  //ensure the tree is ready
  if(!treeBuilt || !treeReady) {
    if(pendingObjs.size() > 0) {
      processPending(); //TODO processPending() should only need to be called once to prep the tree. Tell me why i shouldn't just call this regardless of the above checks
    }
    else {
      buildTree();
    }
  }

  updateLife();

  list<GameObject*> movedObjs = updateObjs(timeScale);

  prune();

  //update child nodes
  for(int q = 0; q < 4; q++) {
    if(children[q] != nullptr) {
      children[q]->update(timeScale);
    }
  }

  updateArrangement(movedObjs);

  //everything is now in the correct box, so we check collisions
  if(!hasParent()) { //if this is root
    for(list<GameObject*>::iterator curObj = movedObjs.begin(); curObj != movedObjs.end(); curObj++) {
      checkCollisions(*curObj, (*curObj)->getPos(), (*curObj)->getPos()); //TODO you'll need significant refactoring if you plan on checking rays
    }
  }
}


// inserts the given object into the tree
bool QuadTree::insert(GameObject *obj) {
  vec2 dimensions = regionMax - regionMin;
  //object doesn't fit here, needs to be moved to parent
  if(!isWithin(obj)) {
    if(hasParent()) {
      return getParent()->insert(obj);
    }
    else {
      cout << "WARNING: object outside world." << endl; //DEBUG
      return false;
    }
  }

  //if this is a leaf, just insert
  if((myObjs.size() == 0 && !hasChildren()) ||
     (dimensions.x <= MIN_SIZE && dimensions.y <= MIN_SIZE)) {
    myObjs.push_back(obj);
    return true;
  }

  //check children
  vec2 bounds[2][4];
  subdivideBounds(bounds, dimensions);

  for(int q = 0; q < 4; q++){
    if(isWithin(obj, bounds[0][q], bounds[1][q])) {
      // give to existing child
      if(children[q] != nullptr) {
        return children[q]->insert(obj);
      }
      // give to new child
      else {
        list<GameObject*> temp;
        temp.push_back(obj);
        children[q] = make_shared<QuadTree>(bounds[0][q], bounds[1][q], temp, shared_from_this());
        activeNodes |= (unsigned char)(1 << q);
        return true;
      }
      //TODO may be addt'l tutorial here
    }
  }

  //object fits here, but not in any of the children
  myObjs.push_back(obj);
  return true;
}

// checks for collisions between the given GO and other GO's in the tree
// when a collision is found, collide() is called on both objects
void QuadTree::checkCollisions(GameObject *obj, vec3 curPos, vec3 nextPos) {
  if(treeBuilt && (hasChildren() || myObjs.size() > 0)) {
    //check self
    if(isColliding(obj)) {
      //check objects
      for(list<GameObject*>::iterator otherObj = myObjs.begin(); otherObj != myObjs.end(); otherObj++) {
        if(obj->isColliding(*otherObj)) {
          cout << "Collision detected." << endl; //DEBUG
          obj->collide(*otherObj);
          (*otherObj)->collide(obj);
        }
      }
      //check quadrants
      for(int q = 0; q < 4; q++){
        if(children[q] != nullptr) {
          children[q]->checkCollisions(obj, curPos, nextPos);
        }
      }
    }
  }
}

// checks AABB collision between this node and the given object
bool QuadTree::isColliding(GameObject *obj) {
  vec3 position = obj->getPos();
  float radius = obj->getRadius();
  vec2 oMin = vec2(position.x - radius, position.z - radius);
  vec2 oMax = vec2(position.x + radius, position.z + radius);
  return regionMin.x < oMax.x && oMin.x < regionMax.x &&
         regionMin.y < oMax.y && oMin.y < regionMax.y;
}

//obj is completely within this node
bool QuadTree::isWithin(GameObject *obj) {
  return isWithin(obj, regionMin, regionMax);
}

//obj is completely within given bounding box
bool QuadTree::isWithin(GameObject *obj, vec2 tMin, vec2 tMax) {
  vec2 oMin = vec2(obj->getPos().x - obj->getRadius(),
                   obj->getPos().z - obj->getRadius());
  vec2 oMax = vec2(obj->getPos().x + obj->getRadius(),
                   obj->getPos().z + obj->getRadius());
  return tMin.x < oMin.x && oMax.x < tMax.x &&
         tMin.y < oMin.y && oMax.y < tMax.y;
}

bool QuadTree::hasChildren() {
  //DEBUG
  bool temp = children[0] != nullptr || children[1] != nullptr || children[2] != nullptr || children[3] != nullptr;
  if(temp != (activeNodes != 0)) {
    cout << "bitflag problem" << endl;
  }
  return activeNodes != 0;
}

int QuadTree::getLifespan() {
  return lifespan;
}

void QuadTree::setLifespan(int i) {
  lifespan = i;
}

int QuadTree::getObjCount() {
  return myObjs.size();
}

bool QuadTree::hasParent() {
  return !(parent.expired());
}

shared_ptr<QuadTree> QuadTree::getParent() {
  shared_ptr<QuadTree> sp = parent.lock();
  return sp;
}
