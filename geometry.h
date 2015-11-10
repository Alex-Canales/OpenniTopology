#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include <vector>

typedef struct {
    float x, y, z;
} Point;

typedef struct {
    float x, y;
    size_t index;  //Index of the triangle
} Centroid;

typedef struct {
    unsigned int a;
    unsigned int b;
    unsigned int c;
} Triangle_vertices;

typedef std::vector<Triangle_vertices> v_TriangleV;
typedef std::vector<Point> v_Point;
typedef std::vector<Centroid> v_Centroid;

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

};

#endif
