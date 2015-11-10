#include <iostream>
#include <cstdlib>

#include "geometry.h"
#include "delaunay.h"

float Geometry::pointsEqual2D(Point a, Point b)
{
    return (a.x == b.x && a.y == b.y);
}

float Geometry::pointsEqual3D(Point a, Point b)
{
    return (a.x == b.x && a.y == b.y && a.z == b.z);
}

float Geometry::dotProduct2D(Point v1, Point v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

bool Geometry::xCompare(Point a, Point b)
{
    return a.x < b.x;
}

bool Geometry::compare(Point a, Point b)
{
    if(a.x < b.x)
       return true;
    return (a.x == b.x && a.y < b.y);
}

v_Point Geometry::getTopology(v_Point reference, v_Point topologyRef)
{
    v_Point topology;

    if(reference.size() < 3)
    {
        std::cout << "Not enough points for in the reference." << std::endl;
        return topology;
    }
    if(topologyRef.size() == 0)
    {
        std::cout << "Not enough points for doing the topology." << std::endl;
        return topology;
    }


    float height;
    size_t i = 0, iEndTopology = 0;

    std::cout << "Starting triangulation" << std::endl;
    v_TriangleV triangles = triangulate(reference);
    std::cout << "Triangulation is done." << std::endl;

    std::cout << "Calculating the height and adding." << std::endl;
    for(i = 0; i < topologyRef.size(); i++)
    {
        if(i % 500 == 0)
        {
            std::cout << ( static_cast<float>(i) /
                    static_cast<float>(topologyRef.size()) * 100.0 );
            std::cout << " \% calcultated" << std::endl;
        }

        if(getHeight(triangles, reference, topologyRef[i], &height))
        {
            topology.push_back(topologyRef[i]);
            topology[iEndTopology].z = height  - topologyRef[i].z;
            iEndTopology++;
        }
    }
    std::cout << "Returning the topology." << std::endl;
    std::cout << topology.size() << std::endl;

    return topology;
}

v_TriangleV Geometry::triangulate(v_Point points)
{
    unsigned int i = 0, numTriangles = 0, numPoints = points.size();
    delaunay2d_t* poly = NULL;
    tri_delaunay2d_t* triangles = NULL;
    v_TriangleV realTriangles;
    Triangle_vertices triVert;
    del_point2d_t *pointsDel = (del_point2d_t*) calloc(sizeof(del_point2d_t),
            numPoints);
    if (pointsDel == NULL)
        return realTriangles;

    //Do Delaunay
    for (i = 0; i < numPoints; i++)
    {
        pointsDel[i].x = points[i].x;
        pointsDel[i].y = points[i].y;
    }
    poly = delaunay2d_from(pointsDel, numPoints);
    std::cout << "poly is done" << std::endl;
    triangles = tri_delaunay2d_from(poly);
    std::cout << "triangles is done" << std::endl;

    //Convert Delaunay type by personnalized type
    numTriangles = triangles->num_triangles;
    for (i = 0; i < numTriangles; i++)
    {
        triVert.a = *(triangles->tris);
        triangles->tris++;
        triVert.b = *(triangles->tris);
        triangles->tris++;
        triVert.c = *(triangles->tris);
        triangles->tris++;
        realTriangles.push_back(triVert);
    }

    delete[] pointsDel;
    delete[] triangles;
    return realTriangles;
}

bool Geometry::pointOutsideBoundary(Triangle_vertices triangle,
        const v_Point &points, Point coordinate)
{
    Point a = points[triangle.a];
    Point b = points[triangle.b];
    Point c = points[triangle.c];

    return( (coordinate.x > a.x && coordinate.x > b.x && coordinate.x > c.x) ||
            (coordinate.x < a.x && coordinate.x < b.x && coordinate.x < c.x) ||
            (coordinate.y > a.y && coordinate.y > b.y && coordinate.y > c.y) ||
            (coordinate.y < a.y && coordinate.y < b.y && coordinate.y < c.y));
}

bool Geometry::pointInTriangle(Triangle_vertices triangle,
        const v_Point &points, Point coordinate, float *coeffAB,
        float *coeffAC)
{
    if(pointOutsideBoundary(triangle, points, coordinate))
        return false;

    //Algorithm from http://www.blackpawn.com/texts/pointinpoly/

    Point vAC, vAB, vAP;
    vAC.x = points[triangle.c].x - points[triangle.a].x;
    vAC.y = points[triangle.c].y - points[triangle.a].y;
    vAB.x = points[triangle.b].x - points[triangle.a].x;
    vAB.y = points[triangle.b].y - points[triangle.a].y;
    vAP.x = coordinate.x - points[triangle.a].x;
    vAP.y = coordinate.y - points[triangle.a].y;

    float dotACAC = dotProduct2D(vAC, vAC), dotACAB = dotProduct2D(vAC, vAB);
    float dotACAP = dotProduct2D(vAC, vAP), dotABAB = dotProduct2D(vAB, vAB);
    float dotABAP = dotProduct2D(vAB, vAP);

    //Find barycenter coefficient
    float invDenominator = 1 / (dotACAC * dotABAB - dotACAB * dotACAB);
    *coeffAC = (dotABAB * dotACAP - dotACAB * dotABAP) * invDenominator;
    *coeffAB = (dotACAC * dotABAP - dotACAB * dotACAP) * invDenominator;

    // if(*coeffAC < 0 || *coeffAB < 0 || (*coeffAC + *coeffAB > 1))
    //     return false;
    //
    // return true;
    return (*coeffAC >= 0 && *coeffAB >= 0 && (*coeffAC + *coeffAB <= 1));
}

v_Centroid Geometry::getCentroids(v_TriangleV triangles, const v_Point &points)
{
    v_Centroid centroids;
    Centroid centroid;
    Point a, b, c, middleAB;
    float coefficient = 1/3;

    for(size_t i=0; i < triangles.size(); i++)
    {
        a = points[triangles[i].a];
        b = points[triangles[i].b];
        c = points[triangles[i].c];

        middleAB.x = (b.x - a.x) / 2 + a.x;
        middleAB.y = (b.y - a.y) / 2 + a.y;

        centroid.x = (c.x - middleAB.x) * coefficient + middleAB.x;
        centroid.y = (c.y - middleAB.y) * coefficient + middleAB.y;
        centroid.index = i;

        centroids.push_back(centroid);
    }

    return centroids;
}

int Geometry::findTriangle(v_TriangleV triangles, const v_Point &points,
        Point coordinate, float *coeffAB, float *coeffAC)
{
    for(unsigned int i = 0; i < triangles.size(); i++)
    {
        if(pointInTriangle(triangles[i], points, coordinate, coeffAB, coeffAC))
            return i;
    }
    return -1;
}

bool Geometry::getHeight(v_TriangleV triangles, const v_Point &points,
        Point coordinate, float *height)
{
    float coeffAB = 0, coeffAC = 0;
    int index = findTriangle(triangles, points, coordinate, &coeffAB, &coeffAC);
    if (index == -1)
        return false;

    Point a = points[triangles[index].a];
    Point b = points[triangles[index].b];
    Point c = points[triangles[index].c];
    *height = a.z + coeffAB * (b.z - a.z) + coeffAC * (c.z - a.z);

    return true;
}
