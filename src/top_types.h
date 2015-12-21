#ifndef _TOP_TYPES_
#define _TOP_TYPES_

#include <cmath>
#include <vector>

/**
 * Will store all commonly used types and classes.
 */

class Point
{
public:
    float x, y, z;

    Point(float x=0, float y=0, float z=0): x(x), y(y), z(z) { }

    Point& add(Point point)
    {
        x += point.x;
        y += point.y;
        z += point.z;
        return (*this);
    }

    Point& add(float x, float y, float z)
    {
        x += x;
        y += y;
        z += z;
        return (*this);
    }

    Point& substract(Point point)
    {
        x -= point.x;
        y -= point.y;
        z -= point.z;
        return (*this);
    }

    Point& substract(float x, float y, float z)
    {
        x -= x;
        y -= y;
        z -= z;
        return (*this);
    }

    bool equal(Point p)
    {
        return (x == p.x && y == p.y && z == p.z);
    }

    bool equal(float x, float y, float z)
    {
        return (x == this->x && y == this->y && z == this->z);
    }

    //If considered as a vector:
    //Length squarred
    float length2()
    {
        return (x * x) + (y * y) + (y * y);
    }

    float length()
    {
        return std::sqrt(length2());
    }

    // Return the unit vector (normalized version of the vector)
    Point unitVector()
    {
        float l = length();
        return Point(x / l, y / l, z / l);
    }

};

// class Centroid
class Centroid : public Point
{
public:
    // float x, y;
    size_t index;  //Index of the triangle

    // Centroid(float x=0, float y=0, float index=0): x(x), y(y), index(index) { }
    Centroid(float x=0, float y=0, float index=0): Point(x, y), index(index) { }

    bool equal(Point p)
    {
        return (x == p.x && y == p.y);
    }
};


typedef struct {
    unsigned int a;
    unsigned int b;
    unsigned int c;
} Triangle_vertices;

typedef std::vector<Triangle_vertices> v_TriangleV;
typedef std::vector<Point> v_Point;
typedef std::vector<Centroid> v_Centroid;

#endif
