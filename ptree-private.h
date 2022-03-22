/*
*  File:        ptree-private.h
*  Description: Private functions for the PTree class
*  Date:        2022-03-06 03:30
*
*               DECLARE YOUR PTREE PRIVATE MEMBER FUNCTIONS HERE
*/

#ifndef _PTREE_PRIVATE_H_
#define _PTREE_PRIVATE_H_

/////////////////////////////////////////////////
// DEFINE YOUR PRIVATE MEMBER FUNCTIONS HERE
//
// Just write the function signatures.
//
// Example:
//
// Node* MyHelperFunction(int arg_a, bool arg_b);
//
/////////////////////////////////////////////////

void Clear(Node* curr);

void Copy(const PTree& other, Node* curr, Node* other_curr);

void FlipHorizontal(Node* curr);

void FlipVertical(Node* curr);

void ColorImage(PNG& img, Node* root) const;

int CountNodes(Node* root) const;

int CountLeaves(Node* root) const;

//TODO
void PruneSubtree(Node* root, double tolerance);

//TODO
bool Prunable(Node* node, double tolerance);

//TODO
void PruneNodes(Node* node);

#endif