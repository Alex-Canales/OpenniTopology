#ifndef _QUADTREE_CENTROID_
#define _QUADTREE_CENTROID_

#include <vector>

class QuadtreeCentroid
{
    float x, y, width, height;
    std::vector<Centroid> points;
    QuadtreeCentroid *nw, *ne, *sw, *se;

    bool closerToPoint(Centroid a, Centroid b, Centroid point);

    void subdivise();
    bool isNode() const;
    bool insertInChildren(Centroid point);

    bool intersectsAxe(Centroid point, float range, Centroid a, Centroid b,
            bool axeX);

public:
    static const int MAX = 8;

    QuadtreeCentroid(float x, float y, float width, float height);
    ~QuadtreeCentroid();

    float getX() const;
    float getY() const;
    float getWidth() const;
    float getHeight() const;
    size_t getNumberMembers() const;

    bool insert(Centroid point);
    bool inBoundary(Centroid point);
    bool inRange(Centroid point, float range);

    //Unsorted
    std::vector<Centroid> getClosestPoints(const Centroid point, float range);
    std::vector<Centroid> getClosestPointsSorted(const Centroid point,
            float range);

    void print();
};

#endif
