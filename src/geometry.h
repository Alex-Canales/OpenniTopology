#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include <vector>
#include <cmath>

#include "top_types.h"
#include "quadtreeCentroid.h"
#include "kdtreeCentroid.h"

class Geometry
{
public:
    static float pointsEqual2D(Point a, Point b);
    static float pointsEqual3D(Point a, Point b);
    static float dotProduct2D(Point v1, Point v2);

    //Compare if a.x < b.x, useful for sorting
    static bool xCompare(Point a, Point b);
    //Compare if a.x < b.x && a.y < b.y, useful for sorting (after xCompare and
    // using a stable sort)
    static bool xyCompare(Point a, Point b);
    static bool compare(Point a, Point b);

    //Points have to be unique
    static v_Point getTopology(v_Point reference, v_Point topologyRef);

    static bool pointOutsideBoundary(Triangle_vertices triangle,
        const v_Point &points, Point coordinate);

    static v_TriangleV triangulate(v_Point points);

    static bool pointInTriangle(Triangle_vertices triangle,
        const v_Point &points, Point coordinate, float *coeffAB,
        float *coeffAC);
    static v_Centroid getCentroids(v_TriangleV &triangles, const v_Point &points);

    static int findTriangle(v_TriangleV &triangles, const v_Point &points,
        Point coordinate, float *coeffAB, float *coeffAC);

    static bool getHeight(v_TriangleV &triangles, const v_Point &points,
            Point coordinate, float *height);


    static float getStartRange(unsigned int width, unsigned int height,
        unsigned int pointCloudSize);

    static int findTriangle(v_TriangleV &triangles, const v_Point &points,
        Point &coordinate, QuadtreeCentroid &quad, float startRange,
        float *coeffAB, float *coeffAC);

    static bool getHeight(v_TriangleV &triangles, const v_Point &points,
            Point &coordinate, float *height, QuadtreeCentroid &quad);

    static int findTriangle(v_TriangleV &triangles, const v_Point &points,
        Point &coordinate, KdtreeCentroid &kdtree, unsigned int numberStart,
        float *coeffAB, float *coeffAC);

    static bool getHeight(v_TriangleV &triangles, const v_Point &points,
            Point &coordinate, float *height, KdtreeCentroid &kdtree);

};

#endif
