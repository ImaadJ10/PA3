/*
*  File:        ptree.cpp
*  Description: Implementation of a partitioning tree class for CPSC 221 PA3
*  Date:        2022-03-03 01:53
*
*               ADD YOUR PRIVATE FUNCTION IMPLEMENTATIONS TO THE BOTTOM OF THIS FILE
*/

#include "ptree.h"
#include "hue_utils.h" // useful functions for calculating hue averages

using namespace cs221util;
using namespace std;

// The following definition may be convenient, but is not necessary to use
typedef pair<unsigned int, unsigned int> pairUI;

/////////////////////////////////
// PTree private member functions
/////////////////////////////////

/*
*  Destroys all dynamically allocated memory associated with the current PTree object.
*  You may want to add a recursive helper function for this!
*  POST: all nodes allocated into the heap have been released.
*/
void PTree::Clear() {
  Clear(root);
}

/*
*  Copies the parameter other PTree into the current PTree.
*  Does not free any memory. Should be called by copy constructor and operator=.
*  You may want a recursive helper function for this!
*  PARAM: other - the PTree which will be copied
*  PRE:   There is no dynamic memory associated with this PTree.
*  POST:  This PTree is a physically separate copy of the other PTree.
*/
void PTree::Copy(const PTree& other) {
  root = new Node();
  Copy(other, root, other.root);
}

/*
*  Private helper function for the constructor. Recursively builds the tree
*  according to the specification of the constructor.
*  You *may* change this if you like, but we have provided here what we
*  believe will be sufficient to use as-is.
*  PARAM:  im - full reference image used for construction
*  PARAM:  ul - upper-left image coordinate of the currently building Node's image region
*  PARAM:  w - width of the currently building Node's image region
*  PARAM:  h - height of the currently building Node's image region
*  RETURN: pointer to the fully constructed Node
*/
Node* PTree::BuildNode(PNG& im, pair<unsigned int, unsigned int> ul, unsigned int w, unsigned int h) {
  // terminate at base case
  if (w == 1 && h == 1) {
    Node* leaf = new Node(ul, w, h, *im.getPixel(ul.first, ul.second), nullptr, nullptr);
    return leaf;
  }

  // recursively construct children
  unsigned int width_A;
  unsigned int width_B;
  unsigned int height_A;
  unsigned int height_B;
  pair<unsigned int, unsigned int> ul_A;
  pair<unsigned int, unsigned int> ul_B;
  if (w >= h) {
    width_A = (w / 2);
    width_B = (w / 2) + (w % 2);
    height_A = h;
    height_B = h;
    ul_A = make_pair(ul.first, ul.second);
    ul_B = make_pair(ul.first + width_A, ul.second);
  } else {
    width_A = w;
    width_B = w;
    height_A = (h / 2);
    height_B = (h / 2) + (h % 2);
    ul_A = make_pair(ul.first, ul.second);
    ul_B = make_pair(ul.first, ul.second + height_A);
  }
  Node* left = BuildNode(im, ul_A, width_A, height_A);
  Node* right = BuildNode(im, ul_B, width_B, height_B);

  // average color across designated image region
  double sumHX = 0;
  double sumHY = 0;
  double sumS = 0;
  double sumL = 0;
  double sumA = 0;
  for (unsigned int x = ul.first; x < ul.first + w; x++) {
    for (unsigned int y = ul.second; y < ul.second + h; y++) {
      sumHX += Deg2X(im.getPixel(x, y)->h);
      sumHY += Deg2Y(im.getPixel(x, y)->h);
      sumS += im.getPixel(x, y)->s;
      sumL += im.getPixel(x, y)->l;
      sumA += im.getPixel(x, y)->a;
    }
  }
  double avgHX = sumHX / (w * h);
  double avgHY = sumHY / (w * h);
  double avgH = XY2Deg(avgHX, avgHY);
  double avgS = sumS / (w * h);
  double avgL = sumL / (w * h);
  double avgA = sumA / (w * h);
  HSLAPixel avgPixel = HSLAPixel(avgH, avgS, avgL, avgA);

  // construct parent
  Node* curr = new Node(ul, w, h, avgPixel, left, right);
  return curr;
}

////////////////////////////////
// PTree public member functions
////////////////////////////////

/*
*  Constructor that builds the PTree using the provided PNG.
*
*  The PTree represents the sub-image (actually the entire image) from (0,0) to (w-1, h-1) where
*  w-1 and h-1 are the largest valid image coordinates of the original PNG.
*  Each node corresponds to a rectangle of pixels in the original PNG, represented by
*  an (x,y) pair for the upper-left corner of the rectangle, and two unsigned integers for the
*  number of pixels on the width and height dimensions of the rectangular sub-image region the
*  node defines.
*
*  A node's two children correspond to a partition of the node's rectangular region into two
*  equal (or approximately equal) size regions which are either tiled horizontally or vertically.
*
*  If the rectangular region of a node is taller than it is wide, then its two children will divide
*  the region into vertically-tiled sub-regions of equal height:
*  +-------+
*  |   A   |
*  |       |
*  +-------+
*  |   B   |
*  |       |
*  +-------+
*
*  If the rectangular region of a node is wider than it is tall, OR if the region is exactly square,
*  then its two children will divide the region into horizontally-tiled sub-regions of equal width:
*  +-------+-------+
*  |   A   |   B   |
*  |       |       |
*  +-------+-------+
*
*  If any region cannot be divided exactly evenly (e.g. a horizontal division of odd width), then
*  child B will receive the larger half of the two subregions.
*
*  When the tree is fully constructed, each leaf corresponds to a single pixel in the PNG image.
* 
*  For the average colour, this MUST be computed separately over the node's rectangular region.
*  Do NOT simply compute this as a weighted average of the children's averages.
*  The functions defined in hue_utils.h and implemented in hue_utils.cpp will be very useful.
*  Computing the average over many overlapping rectangular regions sounds like it will be
*  inefficient, but as an exercise in theory, think about the asymptotic upper bound on the
*  number of times any given pixel is included in an average calculation.
* 
*  PARAM: im - reference image which will provide pixel data for the constructed tree's leaves
*  POST:  The newly constructed tree contains the PNG's pixel data in each leaf node.
*/
PTree::PTree(PNG& im) {
  root = BuildNode(im, make_pair(0, 0), im.width(), im.height());
}

/*
*  Copy constructor
*  Builds a new tree as a copy of another tree.
*
*  PARAM: other - an existing PTree to be copied
*  POST:  This tree is constructed as a physically separate copy of other tree.
*/
PTree::PTree(const PTree& other) {
  Copy(other);
}

/*
*  Assignment operator
*  Rebuilds this tree as a copy of another tree.
*
*  PARAM: other - an existing PTree to be copied
*  POST:  If other is a physically different tree in memory, all pre-existing dynamic
*           memory in this tree is deallocated and this tree is reconstructed as a
*           physically separate copy of other tree.
*         Otherwise, there is no change to this tree.
*/
PTree& PTree::operator=(const PTree& other) {
  if (this != &other) {
    Clear();
    Copy(other);
  }
  return *this;
}

/*
*  Destructor
*  Deallocates all dynamic memory associated with the tree and destroys this PTree object.
*/
PTree::~PTree() {
  Clear();
}

/*
*  Traverses the tree and puts the leaf nodes' color data into the nodes'
*  defined image regions on the output PNG.
*  For non-pruned trees, each leaf node corresponds to a single pixel that will be coloured.
*  For pruned trees, each leaf node may cover a larger rectangular region that will be
*  entirely coloured using the node's average colour attribute.
*
*  You may want to add a recursive helper function for this!
*
*  RETURN: A PNG image of appropriate dimensions and coloured using the tree's leaf node colour data
*/
PNG PTree::Render() const {
  unsigned int img_width = root->width;
  unsigned int img_height = root->height;
  PNG img = PNG(img_width, img_height);

  ColorImage(img, root);

  return img;
}

/*
*  Trims subtrees as high as possible in the tree. A subtree is pruned
*  (its children are cleared/deallocated) if ALL of its leaves have colour
*  within tolerance of the subtree root's average colour.
*  Pruning criteria should be evaluated on the original tree, and never on a pruned
*  tree (i.e. we expect that Prune would be called on any tree at most once).
*  When processing a subtree, you should determine if the subtree should be pruned,
*  and prune it if possible before determining if it has subtrees that can be pruned.
* 
*  You may want to add (a) recursive helper function(s) for this!
*
*  PRE:  This tree has not been previously pruned (and is not copied/assigned from a tree that has been pruned)
*  POST: Any subtrees (as close to the root as possible) whose leaves all have colour
*        within tolerance from the subtree's root colour will have their children deallocated;
*        Each pruned subtree's root becomes a leaf node.
*/
void PTree::Prune(double tolerance) {
  PruneSubtree(root, tolerance);
}

/*
*  Returns the total number of nodes in the tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*/
int PTree::Size() const {
  return CountNodes(root);
}

/*
*  Returns the total number of leaf nodes in the tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*/
int PTree::NumLeaves() const {
  return CountLeaves(root);
}

/*
*  Rearranges the nodes in the tree, such that a rendered PNG will be flipped horizontally
*  (i.e. mirrored over a vertical axis).
*  This can be achieved by manipulation of the nodes' member attribute(s).
*  Note that this may possibly be executed on a pruned tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*
*  POST: Tree has been modified so that a rendered PNG will be flipped horizontally.
*/
void PTree::FlipHorizontal() {
  InvertTree(root);
}

/*
*  Like the function above, rearranges the nodes in the tree, such that a rendered PNG
*  will be flipped vertically (i.e. mirrored over a horizontal axis).
*  This can be achieved by manipulation of the nodes' member attribute(s).
*  Note that this may possibly be executed on a pruned tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*
*  POST: Tree has been modified so that a rendered PNG will be flipped vertically.
*/
void PTree::FlipVertical() {
  // add your implementation below
  
}

/*
    *  Provides access to the root of the tree.
    *  Dangerous in practice! This is only used for testing.
    */
Node* PTree::GetRoot() {
  return root;
}

//////////////////////////////////////////////
// PERSONALLY DEFINED PRIVATE MEMBER FUNCTIONS
//////////////////////////////////////////////

void PTree::Clear(Node* curr) {
  if (curr->A == NULL && curr->B == NULL) {
    delete curr;
    return;
  } else if (curr->A == NULL) {
    Clear(curr->B);
    delete curr;
    return;
  } else if (curr->B == NULL) {
    Clear(curr->A);
    delete curr;
    return;
  } else {
    Clear(curr->A);
    Clear(curr->B);
    delete curr;
    return;
  }
}

void PTree::Copy(const PTree& other, Node* curr, Node* other_curr) {
  curr->upperleft = other_curr->upperleft;
  curr->width = other_curr->width;
  curr->height = other_curr->height;
  curr->avg = other_curr->avg;

  if (other_curr->A == NULL && other_curr->B == NULL) {
    curr->A = NULL;
    curr->B = NULL;
  } else if (other_curr->A == NULL) {
    curr->A = NULL;
    curr->B = new Node();
    Copy(other, curr->B, other_curr->B);
  } else if (other_curr->B == NULL) {
    curr->A = new Node();
    curr->B = NULL;
    Copy(other, curr->A, other_curr->A);
  } else {
    curr->A = new Node();
    curr->B = new Node();
    Copy(other, curr->A, other_curr->A);
    Copy(other, curr->B, other_curr->B);
  }
}

bool PTree::Prunable(HSLAPixel rootAvg, Node* node, double tolerance) {
  if (node == NULL) {
    return true;
  }

  if (node->A != NULL && rootAvg.dist(node->A->avg) > tolerance) {
    return false;
  }

  if (node->B != NULL && rootAvg.dist(node->B->avg) > tolerance) {
    return false;
  }

  if (!Prunable(rootAvg, node->A, tolerance) || !Prunable(rootAvg, node->B, tolerance)) {
    return false;
  }

  return true;
}

void PTree::PruneSubtree(Node* node, double tolerance) {
  if (node == NULL) {
    return;
  }

  if (Prunable(node->avg, node, tolerance)) {
    // Need to figure this function out
  } else {
    PruneSubtree(node->A, tolerance);
    PruneSubtree(node->B, tolerance);
  }
}

void PTree::ColorImage(PNG& img, Node* root) const {
  if (root == NULL) {
    return;
  }
   
  if (root->A == NULL && root->B == NULL) {
    for (unsigned int i = 0; i < root->width; i++) {
      HSLAPixel* currPixel = img.getPixel(root->upperleft.first + i, root->upperleft.second);
      *currPixel = root->avg;
    }
  
    for (unsigned int i = 0; i < root->height; i++) {
      HSLAPixel* currPizel = img.getPixel(root->upperleft.first, root->upperleft.second + i);
      *currPizel = root->avg;
    }
  }

  ColorImage(img, root->A);
  ColorImage(img, root->B);

}

int PTree::CountNodes(Node* root) const {
  if (root == NULL) {
    return 0;
  } else {
    return 1 + CountNodes(root->A) + CountNodes(root->B);
  }
}

int PTree::CountLeaves(Node* root) const {
  if (root == NULL) {
    return 0;
  }

  if (root->A == NULL && root->B == NULL) {
    return 1;
  } else {
    return CountLeaves(root->A) + CountLeaves(root->B);
  }
}

void PTree::InvertTree(Node* node) {
  if (node == NULL) {
    return;
  }

  Node* temp = node->A;
  node->A = node->B;
  node->B = temp;

  InvertTree(node->A);
  InvertTree(node->B);
}