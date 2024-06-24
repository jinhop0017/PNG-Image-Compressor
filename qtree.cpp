#include "qtree.h"

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
 * This way, each of the children's rectangles together will have coordinates
 * that when combined, completely cover the original rectangle's image
 * region and do not overlap.
 */
QTree::QTree(const PNG& imIn) {
	width = imIn.width();
	height = imIn.height();

	pair<unsigned int, unsigned int> ul(0, 0);
	pair<unsigned int, unsigned int> lr(width - 1, height - 1);

	root = BuildNode(imIn, ul, lr);
}

/**
 * Overloaded assignment operator for QTrees.
 * Part of the Big Three that we must define because the class
 * allocates dynamic memory.
 *
 * @param rhs
 */
QTree& QTree::operator=(const QTree& rhs) {
	if (this != &rhs) {
		Clear();
		Copy(rhs);
    }

    return *this;
}

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
PNG QTree::Render(unsigned int scale) const {
	PNG img(width * scale, height * scale);
	renderNode(root, img, scale);
	return img;
}

/**
 *  Prune function trims subtrees as high as possible in the tree.
 *  A subtree is pruned (cleared) if all of the subtree's leaves are within
 *  tolerance of the average color stored in the root of the subtree.
 *
 * @param tolerance maximum RGBA distance to qualify for pruning
 * @pre this tree has not previously been pruned, nor is copied from a previously pruned tree.
 */
void QTree::Prune(double tolerance) {
	pruneSt(root, tolerance);
}

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
 *
 */
void QTree::FlipHorizontal() {
	flipHorizontal(root);
}

/**
 *  RotateCCW rearranges the contents of the tree, so that its
 *  rendered image will appear rotated by 90 degrees counter-clockwise.
 *  This may be called on a previously pruned/flipped/rotated tree.
 *
 *  After rotation, the NW/NE/SW/SE pointers maps to what will be
 *  physically rendered in the respective NW/NE/SW/SE corners, but it
 *  is no longer necessary to ensure that 1-pixel tall or wide rectangles
 *  have null eastern or southern children
 *  (i.e. after rotation, a node's NW and NE pointers may be null, but have
 *  non-null SW and SE, or it may have null NW/SW but non-null NE/SE)
 */
void QTree::RotateCCW() {
    unsigned int temp = height;
	height = width;
	width = temp;
	rotateCCW(root);
}

/**
 * Destroys all dynamically allocated memory associated with the
 * current QTree object.
 */
void QTree:: Clear() {
	clear(root);
	height = 0;
	width = 0;
}

/**
 * Copies the parameter other QTree into the current QTree.
 * Does not free any memory. Called by copy constructor and operator=.
 * @param other The QTree to be copied.
 */
void QTree::Copy(const QTree& other) {
	width = other.width;
    height = other.height;
	root = copy(other.root);
}

/**
 * Private helper function for the constructor. Recursively builds
 * the tree according to the specification of the constructor.
 * @param img reference to the original input image.
 * @param ul upper left point of current node's rectangle.
 * @param lr lower right point of current node's rectangle.
 */
Node* QTree::BuildNode(const PNG& img, pair<unsigned int, unsigned int> ul, pair<unsigned int, unsigned int> lr) {
	if (ul == lr) {
		Node * nd = new Node(ul, lr, *img.getPixel(ul.first, ul.second));
		return nd;
	}

	unsigned int midX = (ul.first + lr.first) / 2;
	unsigned int midY = (ul.second + lr.second) / 2;

	Node *NW = BuildNode(img, ul, pair<unsigned int, unsigned int>(midX, midY));
	Node *NE = nullptr;
	Node *SW = nullptr;
	Node *SE = nullptr;

	unsigned int areaNW;
	unsigned int areaSW;
	unsigned int areaNE;
	unsigned int areaSE;

	if (lr.first == ul.first){
		SW = BuildNode(img, make_pair(ul.first, midY + 1), make_pair(midX, lr.second));
	} else {
		NE = BuildNode(img, make_pair(midX + 1, ul.second), make_pair(lr.first, midY));
		if (ul.second != lr.second) {
			SW = BuildNode(img, make_pair(ul.first, midY + 1), make_pair(midX, lr.second));
			SE = BuildNode(img, make_pair(midX + 1, midY + 1), lr);
		}
	}

	areaNW = (midX - ul.first + 1) * (midY - ul.second + 1);
	areaNE = (lr.first - (midX + 1) + 1) * (midY - ul.second + 1);
	areaSW = (midX - ul.first + 1) * (lr.second - (midY + 1) + 1);
	areaSE = (lr.first - (midX + 1) + 1) * (lr.second - (midY + 1) + 1);

	unsigned int totalArea = (lr.first - ul.first + 1) * (lr.second - ul.second + 1);
	
	int totalR = 0;
	int totalB = 0;
	int totalG = 0;
	double totalA = 0.0;

	if (NW != nullptr) {
    	totalR += NW->avg.r * areaNW;
    	totalB += NW->avg.b * areaNW;
    	totalG += NW->avg.g * areaNW;
    	totalA += NW->avg.a * areaNW;
	}

	if (NE != nullptr) {
    	totalR += NE->avg.r * areaNE;
    	totalB += NE->avg.b * areaNE;
    	totalG += NE->avg.g * areaNE;
    	totalA += NE->avg.a * areaNE;
	}

	if (SW != nullptr) {
    	totalR += SW->avg.r * areaSW;
    	totalB += SW->avg.b * areaSW;
    	totalG += SW->avg.g * areaSW;
    	totalA += SW->avg.a * areaSW;
	}

	if (SE != nullptr) {
    	totalR += SE->avg.r * areaSE;
    	totalB += SE->avg.b * areaSE;
    	totalG += SE->avg.g * areaSE;
    	totalA += SE->avg.a * areaSE;
	}

	int r = totalR / totalArea;
	int g = totalG / totalArea;
	int b = totalB / totalArea;
	double a = totalA / totalArea;

	Node *newNode = new Node(ul, lr, RGBAPixel(r, g, b, a));
	newNode->NW = NW;
	newNode->NE = NE;
	newNode->SW = SW;
	newNode->SE = SE;

	return newNode;
}

/*********************************************************/
/*** Helper functions ***/
/*********************************************************/

void QTree::renderNode(Node* nd, PNG& img, unsigned int scale) const {
	if (nd != nullptr) {
		if (!nd->NW && !nd->NE && !nd->SW && !nd->SE) {
			draw(img, nd->upLeft.first * scale, nd->upLeft.second * scale, scale, nd->avg);
		}

		else {
			renderNode(nd->NW, img, scale);
			renderNode(nd->NE, img, scale);
			renderNode(nd->SW, img, scale);
			renderNode(nd->SE, img, scale);
		}
	}
}

void QTree::draw(PNG& img, unsigned int startX, unsigned int startY, unsigned int scale, RGBAPixel color) const {
    for (unsigned int x = 0; x < scale; x++) {
        for (unsigned int y = 0; y < scale; y++) {
            *img.getPixel(startX + x, startY + y) = color;
        }
    }
}

void QTree::flipHorizontal(Node* nd) {
	if (nd != nullptr) {

		swap(nd->NW, nd->NE);
    	swap(nd->SW, nd->SE);

		unsigned int newLx = width - 1 - nd->lowRight.first;
		unsigned int newRx = width - 1 - nd->upLeft.first;
		nd->upLeft.first = newLx;
		nd->lowRight.first = newRx;
		if (nd->upLeft.first > nd->lowRight.first) {
			swap(nd->upLeft.first, nd->lowRight.first);
		}

		flipHorizontal(nd->NW);
		flipHorizontal(nd->NE);
		flipHorizontal(nd->SW);
		flipHorizontal(nd->SE);
	}
}

void QTree::rotateCCW(Node *nd) {
	if (nd) {
		Node *NW = nd->NW;
		Node *SW = nd->SW;
		Node *NE = nd->NE;
		Node *SE = nd->SE;
		nd->NW = NE;
		nd->SW = NW;
		nd->SE = SW;
		nd->NE = SE;

		pair<unsigned int, unsigned int> uL = make_pair(nd->upLeft.second, height - nd->lowRight.first - 1);
		pair<unsigned int, unsigned int> lR = make_pair(nd->lowRight.second, height - nd->upLeft.first - 1);
		nd->upLeft = uL;
		nd->lowRight = lR;

		rotateCCW(nd->NW);
		rotateCCW(nd->NE);
		rotateCCW(nd->SE);
		rotateCCW(nd->SW);
	}
}

void QTree::clear(Node* nd) {
	if (!nd) {
		return;
	}

	clear(nd->NW);
	clear(nd->NE);
	clear(nd->SW);
	clear(nd->SE);

	delete nd;
	nd = nullptr;
}

Node* QTree::copy(Node* nd) const {
	if (!nd) {
		return nd;
	}

	Node* newRoot = new Node(nd->upLeft, nd->lowRight, nd->avg);

	newRoot->NW = copy(nd->NW);
	newRoot->NE = copy(nd->NE);
    newRoot->SW = copy(nd->SW);
    newRoot->SE = copy(nd->SE);

	return newRoot;
}

void QTree::pruneSt(Node* node, double tolerance) {
	if (!node) {
		return;
	}

    if (!node->NW && !node->NE && !node->SW && !node->SE) {
		return;
	}

    if (leavesUnderTol(node, tolerance, node->avg)) {
        clearSt(node);
        return;
    }
	
    pruneSt(node->NW, tolerance);
    pruneSt(node->NE, tolerance);
    pruneSt(node->SW, tolerance);
    pruneSt(node->SE, tolerance);
}

bool QTree::leavesUnderTol(Node* node, double tolerance, RGBAPixel avg) const {
	if (!node) {
		return true;
	}

    if (!node->NW && !node->NE && !node->SW && !node->SE) {
        return (node->avg.distanceTo(avg) <= tolerance);
    }

    return (leavesUnderTol(node->NW, tolerance, avg) && leavesUnderTol(node->NE, tolerance, avg) &&
           leavesUnderTol(node->SW, tolerance, avg) && leavesUnderTol(node->SE, tolerance, avg));
}

void QTree::clearSt(Node* node) {
	if (!node) {
		return;
	}

    clearSt(node->NW);
    clearSt(node->NE);
    clearSt(node->SW);
    clearSt(node->SE);
	printf("My age is ");

    delete node->NW;
    delete node->NE;
    delete node->SW;
    delete node->SE;

    node->NW = nullptr;
    node->NE = nullptr;
    node->SW = nullptr;
    node->SE = nullptr;
}