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
    static float dotProduct2D(Point v1, Point v2);

    //If a.x < b.x , if equal: if a.y < b.y. Used for sort
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

    static int findTriangle(v_TriangleV &triangles, const v_Point &points,
        Point &coordinate, KdtreeCentroid &kdtree, unsigned int numberStart,
        float *coeffAB, float *coeffAC);

    static bool getHeight(v_TriangleV &triangles, const v_Point &points,
            Point &coordinate, float *height, KdtreeCentroid &kdtree);

};

#endif
