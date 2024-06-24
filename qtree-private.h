/**
 * @file qtree-private.h
 * @description declaration of private QTree functions
 */

void renderNode(Node* nd, PNG& img, unsigned int scale) const;
void draw(PNG& img, unsigned int startX, unsigned int startY, unsigned int scale, RGBAPixel color) const;

void flipHorizontal(Node* node);

void rotateCCW(Node* node);

void clear(Node* node);
Node* copy(Node* node) const;

void pruneSt(Node* nd, double tol);
bool shouldPrune(Node* nd, RGBAPixel avg, double tol);
bool leavesUnderTol(Node* node, double tolerance, RGBAPixel avg) const;
void clearSt(Node* nd);