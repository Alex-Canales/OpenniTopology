#include <iostream>
#include <cstdlib>

#include "top_types.h"
#include "delaunay.h"
#include "quadtreeCentroid.h"
#include "kdtreeCentroid.h"
#include "geometry.h"

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

    v_Centroid centroids = getCentroids(triangles, reference);

    // Begin with kdtree
    std::cout << "Pushing centroids in kdtree" << std::endl;
    KdtreeCentroid kdtree(centroids[0]);
    for(i=0; i < centroids.size(); i++)
    {
        kdtree.insert(centroids[i]);
    }
    // End with kdtree

    // // Begin with quadtree
    // std::cout << "Pushing centroids in quadtree" << std::endl;
    // float minX = 0, minY = 0, maxX = 0, maxY = 0;
    // std::cout << "Number centroids: " << centroids.size() << std::endl;
    // for(i=0; i < centroids.size(); i++)
    // {
    //     // std::cout << "(" << centroids[i].x << "; " << centroids[i].y << ")" << std::endl;
    //     if(centroids[i].x < minX)
    //         minX = centroids[i].x;
    //     if(centroids[i].y < minY)
    //         minY = centroids[i].y;
    //     if(centroids[i].x > maxX)
    //         maxX = centroids[i].x;
    //     if(centroids[i].y > maxY)
    //         maxY = centroids[i].y;
    // }
    // std::cout << " Min:" << "(" << minX << "; " << minY << ")" << " Max: " "(" << maxX << "; " << maxY << ")" << std::endl;
    // QuadtreeCentroid quad(minX, minY, maxX - minX, maxY - minY);
    // for(i=0; i < centroids.size(); i++)
    // {
    //     quad.insert(centroids[i]);
    // }
    // std::cout << "Done" << std::endl;
    // std::cout << "quad.x " << quad.getX() << ", quad.y " << quad.getY() << ", quad.width " << quad.getWidth() << ", quad.height " << quad.getHeight() << std::endl;
    // // End with quadtree

    std::cout << "Calculating the height and adding." << std::endl;
    for(i = 0; i < topologyRef.size(); i++)
    {
        // Display percentage of points calculated
        if(i % 500 == 0)
        {
            std::cout << ( static_cast<float>(i) /
                    static_cast<float>(topologyRef.size()) * 100.0 );
            std::cout << " \% calcultated" << std::endl;
        }

        // if(getHeight(triangles, reference, topologyRef[i], &height))
        // if(getHeight(triangles, reference, topologyRef[i], &height, quad))
        if(getHeight(triangles, reference, topologyRef[i], &height, kdtree))
        {
            topology.push_back(topologyRef[i]);
            topology[iEndTopology].z = height  - topologyRef[i].z;
            iEndTopology++;
        }
    }
    std::cout << "Returning the topology." << std::endl;
    std::cout << triangles.size() << " triangles was made." << std::endl;
    std::cout << topology.size() <<  "points were calculated." << std::endl;

    // Using the regular method to find triangle for each points:
    // around 2min30 for parsing 70142 point to 151826 triangles
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

v_Centroid Geometry::getCentroids(v_TriangleV &triangles, const v_Point &points)
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

int Geometry::findTriangle(v_TriangleV &triangles, const v_Point &points,
        Point coordinate, float *coeffAB, float *coeffAC)
{
    for(unsigned int i = 0; i < triangles.size(); i++)
    {
        if(pointInTriangle(triangles[i], points, coordinate, coeffAB, coeffAC))
            return i;
    }
    return -1;
}

bool Geometry::getHeight(v_TriangleV &triangles, const v_Point &points,
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

int Geometry::findTriangle(v_TriangleV &triangles, const v_Point &points,
    Point &coordinate, QuadtreeCentroid &quad, float startRange, float *coeffAB,
    float *coeffAC)
{
    v_TriangleV foundTriangles;
    std::vector<Centroid> foundCentroids;
    float range = startRange;
    size_t numberAnalyzed = 0, i = 0, iTri = 0;
    int numberTries = 3;
    Centroid coorTypeCentroid;
    coorTypeCentroid.x = coordinate.x;
    coorTypeCentroid.y = coordinate.y;

    do
    {
        //If failling too much, it is better to do the regular method
        if(numberTries <= 0)
        {
            // std::cout << "Failed to find, have to use regular for " << startRange << std::endl;
            return Geometry::findTriangle(triangles, points, coordinate,
                    coeffAB, coeffAC);
        }

        foundCentroids = quad.getClosestPointsSorted(coorTypeCentroid, range);

        // Should not occur
        if(foundCentroids.size() < numberAnalyzed)
            return -1;

        for(i=numberAnalyzed; i < foundCentroids.size(); i++)
        {
            iTri = foundCentroids[i].index;
            if(Geometry::pointInTriangle(triangles[iTri], points, coordinate,
                        coeffAB, coeffAC))
                return iTri;
        }

        numberAnalyzed = i;
        range *= 2;
        numberTries--;
    } while(triangles.size() > foundCentroids.size());

    return -1;
}

float Geometry::getStartRange(unsigned int width, unsigned int height,
        unsigned int pointCloudSize)
{
    float minNumberPoints = 10;
    float fW = static_cast<float>(width), fH = static_cast<float>(height);
    float fS = static_cast<float>(pointCloudSize - 1);

    //Average DISTance between points if it were fairly distributed
    float aDistX2 = pow(fW / fS, 2), aDistY2 = pow(fH / fS, 2);
    float aDistance = sqrt(aDistX2 + aDistY2);

    //If this is fairly distributed, each time the range should cath point in
    // a range of minNumberPoints
    return aDistance * minNumberPoints;
}

bool Geometry::getHeight(v_TriangleV &triangles, const v_Point &points,
            Point &coordinate, float *height, QuadtreeCentroid &quad)
{
    float coeffAB = 0, coeffAC = 0;
    float range = getStartRange(quad.getWidth(), quad.getHeight(),
            quad.getNumberMembers());
    int index = findTriangle(triangles, points, coordinate, quad, range,
            &coeffAB, &coeffAC);
    if (index == -1)
        return false;

    // std::cout << "Coordinate: (" << coordinate.x << "; " << coordinate.y << ")" << std::endl;

    Point a = points[triangles[index].a];
    Point b = points[triangles[index].b];
    Point c = points[triangles[index].c];
    *height = a.z + coeffAB * (b.z - a.z) + coeffAC * (c.z - a.z);

    return true;
}

int Geometry::findTriangle(v_TriangleV &triangles, const v_Point &points,
    Point &coordinate, KdtreeCentroid &kdtree, unsigned int numberStart,
    float *coeffAB, float *coeffAC)
{
    v_TriangleV foundTriangles;
    std::vector<Centroid> foundCentroids;
    unsigned int number = numberStart;
    size_t numberAnalyzed = 0, i = 0, iTri = 0;
    int numberTries = 3;
    Centroid coorTypeCentroid;
    coorTypeCentroid.x = coordinate.x;
    coorTypeCentroid.y = coordinate.y;

    do
    {
        //If failling too much, it is better to do the regular method
        if(numberTries <= 0)
        {
            // std::cout << "Failed to find, have to use regular for " << numberStart << std::endl;
            return Geometry::findTriangle(triangles, points, coordinate,
                    coeffAB, coeffAC);
        }

        foundCentroids = kdtree.nearestNeighbours(coorTypeCentroid, number);

        // Should not occur
        if(foundCentroids.size() < numberAnalyzed)
            return -1;

        for(i=numberAnalyzed; i < foundCentroids.size(); i++)
        {
            iTri = foundCentroids[i].index;
            if(Geometry::pointInTriangle(triangles[iTri], points, coordinate,
                        coeffAB, coeffAC))
                return iTri;
        }

        numberAnalyzed = i;
        number *= 2;
        numberTries--;
    } while(triangles.size() > foundCentroids.size());

    return -1;
}

bool Geometry::getHeight(v_TriangleV &triangles, const v_Point &points,
            Point &coordinate, float *height, KdtreeCentroid &kdtree)
{
    float coeffAB = 0, coeffAC = 0;
    unsigned int number = 5;
    int index = findTriangle(triangles, points, coordinate, kdtree, number,
            &coeffAB, &coeffAC);
    if (index == -1)
        return false;

    // std::cout << "Coordinate: (" << coordinate.x << "; " << coordinate.y << ")" << std::endl;

    Point a = points[triangles[index].a];
    Point b = points[triangles[index].b];
    Point c = points[triangles[index].c];
    *height = a.z + coeffAB * (b.z - a.z) + coeffAC * (c.z - a.z);

    return true;
}
