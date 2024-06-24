/**
 * @file qtree.h
 * @description declaration of QTree class used for storing image data
 */

#ifndef _QTREE_H_
#define _QTREE_H_

#include <utility>
#include "imgUtil/PNG.h"
#include "imgUtil/RGBAPixel.h"

using namespace std;
using namespace imgUtil;

class Node {
public:
    Node(pair<unsigned int, unsigned int> ul, pair<unsigned int, unsigned int> lr, RGBAPixel a); // Node constructor

    pair<unsigned int, unsigned int> upLeft;   // image coordinates of upper-left corner of node's rectangular region
    pair<unsigned int, unsigned int> lowRight; // image coordinates of lower-right corner of node's rectangular region
    RGBAPixel avg;  // average color of node's rectangular region
    Node* NW; // upper-left child
    Node* NE; // upper-right child
    Node* SW; // lower-left child
    Node* SE; // lower-right child
};

/**
 * QTree: This is a structure used in decomposing an image
 * into rectangular regions.
 */

class QTree {
public:

    /* =============== start of given functions ====================*/

    /**
     * QTree destructor.
     * Destroys all of the memory associated with the
     * current QTree.
     */
    ~QTree();

    /**
     * Copy constructor for a QTree. GIVEN
     * Since QTrees allocate dynamic memory (i.e., they use "new", we
     * must define the Big Three).
     *
     * @param other The QTree  we are copying.
     */
    QTree(const QTree& other);

    /**
     * Counts the number of nodes in the tree
     */
    unsigned int CountNodes() const;

    /**
     * Counts the number of leaves in the tree
     */
    unsigned int CountLeaves() const;

    /**
     * Constructor that builds a QTree out of the given PNG.
     * Every leaf in the tree corresponds to a pixel in the PNG.
     * Every non-leaf node corresponds to a rectangle of pixels
     * in the original PNG, represented by an (x,y) pair for the
     * upper left corner of the rectangle and an (x,y) pair for
     * lower right corner of the rectangle. In addition, the Node
     * stores a pixel representing the average color over the
     * rectangle.
     *
     * Every node's children correspond to a partition of the
     * node's rectangle into (up to) four smaller rectangles. The node's
     * rectangle is split evenly (or as close to evenly as possible)
     * along both horizontal and vertical axes. If an even split along
     * the vertical axis is not possible, the extra line will be included
     * in the left side; If an even split along the horizontal axis is not
     * possible, the extra line will be included in the upper side.
     * If a single-pixel-wide rectangle needs to be split, the NE and SE children
     * will be null; likewise if a single-pixel-tall rectangle needs to be split,
     * the SW and SE children will be null.
     *
     * In this way, each of the children's rectangles together will have coordinates
     * that when combined, completely cover the original rectangle's image
     * region and do not overlap.
     */
    QTree(const PNG& imIn);

    /**
     * Overloaded assignment operator for QTrees.
     * Part of the Big Three that we must define because the class
     * allocates dynamic memory.
     *
     * @param rhs The right hand side of the statement.
     */
    QTree& operator=(const QTree& rhs);

    /**
     * Render returns a PNG image consisting of the pixels
     * stored in the tree. may be used on pruned trees. Draws
     * every leaf node's rectangle onto a PNG canvas using the
     * average color stored in the node.
     * 
     * For up-scaled images, no color interpolation will be done;
     * each rectangle is fully rendered into a larger rectangular region.
     * 
     * @param scale multiplier for each horizontal/vertical dimension
     * @pre scale > 0
     */
    PNG Render(unsigned int scale) const;

    /**
     *  Prune function trims subtrees as high as possible in the tree.
     *  A subtree is pruned (cleared) if all of the subtree's leaves are within
     *  tolerance of the average color stored in the root of the subtree.
     *
     * @param tolerance maximum RGBA distance to qualify for pruning
     * @pre this tree has not previously been pruned, nor is copied from a previously pruned tree.
     */
    void Prune(double tolerance);

    /**
     *  FlipHorizontal rearranges the contents of the tree, so that
     *  its rendered image will appear mirrored across a vertical axis.
     *  This may be called on a previously pruned/flipped/rotated tree.
     *
     *  After flipping, the NW/NE/SW/SE pointers map to what will be
     *  physically rendered in the respective NW/NE/SW/SE corners, but it
     *  is no longer necessary to ensure that 1-pixel wide rectangles have
     *  null eastern children
     *  (i.e. after flipping, a node's NW and SW pointers may be null, but
     *  have non-null NE and SE)
     */
    void FlipHorizontal();

    /**
     *  RotateCCW rearranges the contents of the tree, so that its
     *  rendered image will appear rotated by 90 degrees counter-clockwise.
     *  This may be called on a previously pruned/flipped/rotated tree.
     *
     *  Note that this may alter the dimensions of the rendered image, relative
     *  to its original dimensions.
     *
     *  After rotation, the NW/NE/SW/SE pointers map to what will be
     *  physically rendered in the respective NW/NE/SW/SE corners, but it
     *  is no longer necessary to ensure that 1-pixel tall or wide rectangles
     *  have null eastern or southern children
     *  (i.e. after rotation, a node's NW and NE pointers may be null, but have
     *  non-null SW and SE, or it may have null NW/SW but non-null NE/SE)
     */
    void RotateCCW();

private:
    /*
     * Private member variables.
     */
    Node* root; // pointer to the root of the QTree

    unsigned int height; // height of PNG represented by the tree
    unsigned int width; // width of PNG represented by the tree

    /**
     * Destroys all dynamically allocated memory associated with the
     * current QTree object.
     */
    void Clear();

    /**
    * Copies the parameter other QTree into the current QTree.
    * Does not free any memory. Called by copy constructor and operator=.
    * @param other The QTree to be copied.
    */
    void Copy(const QTree& other);

    /**
     * Private helper function for the constructor. Recursively builds
     * the tree according to the specification of the constructor.
     * @param img reference to the original input image.
     * @param ul upper left point of current node's rectangle.
     * @param lr lower right point of current node's rectangle.
     */
    Node* BuildNode(const PNG& img, pair<unsigned int, unsigned int> ul, pair<unsigned int, unsigned int> lr);

    /**
     * Private helper function for counting the total number of nodes in the tree. GIVEN
     * @param nd the root of the subtree whose nodes we want to count
     */
    unsigned int CountNodes(Node* nd) const;

    /**
     * Private helper function for counting the number of leaves in the tree. GIVEN
     * @param nd the root of the subtree whose leaves we want to count
     */
    unsigned int CountLeaves(Node* nd) const;

#include "qtree-private.h"
};

#endif
