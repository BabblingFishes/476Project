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
  myObjs = list<GameObject*>();
  pendingObjs = queue<GameObject*>();

  lifespan = -1;
  peakLifespan = 8;
  activeNodes = 0;

  //myObjs = objList;
  for(list<GameObject*>::iterator cur = objList.begin(); cur != objList.end(); cur++) {
    addObject(*cur);
  }
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
  bounds[1][1] = vec2(regionMax.x, center.y);

  bounds[0][2] = vec2(regionMin.x, center.y);
  bounds[1][2] = vec2(center.x, regionMax.y);

  bounds[0][3] = center;
  bounds[1][3] = regionMax;

  /*BEGIN DEBUG
  cout << "subdivided:" << regionMin.x << ",  " << regionMin.y << " // " << regionMax.x << ",  " << regionMax.y
  << " into:" << endl;
  for(int j = 0; j < 4; j++){
    for(int i = 0; i < 2; i++){
      cout << bounds[i][j].x << ", " << bounds[i][j].y << " // ";

    }
    cout << endl;
  }
  //END DEBUG*/
}


//divides this tree's objects among its subtrees where applicable
void QuadTree::subdivideObjs(list<GameObject*> quadList[4], vec2 bounds[2][4]) {
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
    for(int i = 0; i < 4; i++) { //QUESTION necessary?
      quadList[i] = list<GameObject*>();
    }
    subdivideObjs(quadList, bounds);

    //create nodes
    for(int q = 0; q < 4; q++) {
      if(quadList[q].size() != 0) {
        children[q] = make_shared<QuadTree>(bounds[0][q], bounds[1][q], quadList[q], shared_from_this());
        activeNodes |= (unsigned char)(1 << q);

        /*DEBUG
        cout << "Making node at: " << bounds[0][q].x << ",  " << bounds[0][q].y << endl;
        cout << "               " << bounds[1][q].x << ",  " << bounds[1][q].y << endl;
        cout << "with objects: " << endl;
        for(list<GameObject*>::iterator cur = quadList[q].begin(); cur != quadList[q].end(); cur++) {
          cout << (*cur)->getID() <<endl;
        }*/
        //END DEBUG

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
    if(lifespan == -1) { // -1 is the "reset lifespan" flag
      lifespan = peakLifespan;
    }
    else if(lifespan > 0) {
      lifespan--;
    }
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
        cout << "WARNING: Attempted to delete a used node. Resetting node life." << endl;
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
        cout << "x: " << (*curObj)->getPos().x << " y: " << (*curObj)->getPos().y << " z: " << (*curObj)->getPos().z << " radius: " << (*curObj)->getRadius() << endl;

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
  processPending();

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

  //printTree(); //DEBUG
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
      cout << "WARNING: failed to insert object outside world, at x: " << obj->getPos().x << " z: " << obj->getPos().z << endl;
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

        /*DEBUG
        cout << "Making node at: " << bounds[0][q].x << ",  " << bounds[0][q].y << endl;
        cout << "               " << bounds[1][q].x << ",  " << bounds[1][q].y << endl;
        cout << "with objects: " << endl;
        for(list<GameObject*>::iterator cur = temp.begin(); cur != temp.end(); cur++) {
          cout << (*cur)->getID() <<endl;
        }*/
        //END DEBUG

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
  if(!treeBuilt) { //TODO maybe unnecessary
    buildTree();
  }
  else {
    if((hasChildren() || myObjs.size() > 0)) {
      //check node
      if(isColliding(obj)) {
        //check objects
        for(list<GameObject*>::iterator otherObj = myObjs.begin(); otherObj != myObjs.end(); otherObj++) {
          if(obj != *otherObj && obj->isColliding(*otherObj)) {

            if(obj->getID() == GOid::Player) { //DEBUG
              /*cout << "Player within: " << regionMin.x << ",  " << regionMin.y << endl;
              cout << "               " << regionMax.x << ",  " << regionMax.y << endl;
              vec3 temp = (*otherObj)->getPos();
              cout << "   is COLLIDING with object: " << (*otherObj)->getID() << endl;
              cout << "                       at: " << temp.x << ", " << temp.y << ", " << temp.z << endl;*/
            } //END DEBUG

            obj->collide(*otherObj);
            (*otherObj)->collide(obj);
          }
          else if(obj->getID() == GOid::Player) { //DEBUG
            /*cout << "Player within: " << regionMin.x << ",  " << regionMin.y << endl;
            cout << "               " << regionMax.x << ",  " << regionMax.y << endl;
            vec3 temp = (*otherObj)->getPos();
            cout << "IS NOT colliding with object " << (*otherObj)->getID() << " at: " << temp.x << ", " << temp.y << ", " << temp.z << endl;*/
          } //END DEBUG
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
}

// checks AABB collision between this node and the given object
bool QuadTree::isColliding(GameObject *obj) {

  //DEBUG
  /*if(regionMin == vec2(-140, 20) && regionMax == vec2(-60, 60)) {
    static int X = 0;
    cout << X++ << endl;
    printTree();
  }*/

  vec3 position = obj->getPos();
  float radius = obj->getRadius();
  vec2 oMin = vec2(position.x - radius, position.z - radius);
  vec2 oMax = vec2(position.x + radius, position.z + radius);

  //DEBUG
  /*cout << "NODE: " << regionMin.x << ",  " << regionMin.y << endl;
  cout << "         " << regionMax.x << ",  " << regionMax.y << endl;
  cout << "OBJ: " << obj->getID() << " at " << obj->getPos().x << ", " << obj->getPos().z << " with radius " << obj->getRadius() << endl;*/

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
  if(temp != (activeNodes != 0)) { //DEBUG sanity check
    cout << "WARNING: bitflag problem" << endl;
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

//prints tree from this node down
void QuadTree::printTree() {
  cout << "Node at: " << regionMin.x << ",  " << regionMin.y << endl;
  cout << "         " << regionMax.x << ",  " << regionMax.y << endl;
  cout << "with objects: " << endl;
  for(list<GameObject*>::iterator cur = myObjs.begin(); cur != myObjs.end(); cur++) {
    cout << (*cur)->getID() << " at " << (*cur)->getPos().x << ", " << (*cur)->getPos().z << " with radius " << (*cur)->getRadius() << endl;
  }

  for(int q = 0; q < 4; q++) {
    if(children[q] != nullptr) {
      children[q]->printTree();
    }
  }
}
