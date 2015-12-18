#include <iostream>
#include <cmath>
#include <cstddef>

#include "top_types.h"
#include "quadtreeCentroid.h"

bool QuadtreeCentroid::closerToPoint(Centroid a, Centroid b, Centroid point)
{
    float distA = pow(point.x - a.x, 2) + pow(point.y - a.y, 2);
    float distB = pow(point.x - b.x, 2) + pow(point.y - b.y, 2);

    return distA < distB;
}

void QuadtreeCentroid::subdivise()
{
    float wWest = width / 2;
    float hNorth = height / 2;
    float wEst = width - wWest;
    float hSouth = height - hNorth;
    float xWest = x;
    float yNorth = y;
    float xEst = x + wWest;
    float ySouth = y + hNorth;
    size_t i;

    nw = new QuadtreeCentroid(xWest, yNorth, wWest, hNorth);
    ne = new QuadtreeCentroid(xEst, yNorth, wEst, hNorth);
    sw = new QuadtreeCentroid(xWest, ySouth, wEst, hSouth);
    se = new QuadtreeCentroid(xEst, ySouth, wEst, hSouth);

    for(i=0; i < points.size(); i++)
    {
        insertInChildren(points[i]);
    }
    points.clear();
}

bool QuadtreeCentroid::isNode() const
{
    return nw != NULL;
}

bool QuadtreeCentroid::insertInChildren(Centroid point)
{
    if(nw->insert(point))
        return true;
    if(ne->insert(point))
        return true;
    if(sw->insert(point))
        return true;
    return se->insert(point);
}

bool QuadtreeCentroid::intersectsAxe(Centroid point, float range, Centroid a, Centroid b, bool axeX)
{
    float range2 = range * range;
    float projMin, projMax, projPoint;  //Projection points on other axe
    float dist2, height, intersectionMax, intersectionMin;

    if(axeX)
    {
        projMin = (a.x < b.x) ? a.x : b.x;
        projMax = (a.x > b.x) ? a.x : b.x;
        projPoint = point.x;
        dist2 = (a.y - point.y) * (a.y - point.y);
    }
    else
    {
        projMin = (a.y < b.y) ? a.y : b.y;
        projMax = (a.y > b.y) ? a.y : b.y;
        projPoint = point.y;
        dist2 = (a.x - point.x) * (a.x - point.x);
    }

    if(dist2 > range2) //if does not even intersects the line
        return false;

    //if tangent to the segment
    if(dist2 == range2 && projMin <= projPoint && projPoint <= projMax)
        return true;

    height = sqrt(range2 - dist2);
    intersectionMin = projPoint - height;
    intersectionMax = projPoint + height;

    //if circle intersects the segment
    if((projMin <= intersectionMax && intersectionMax <= projMax) ||
            (projMin <= intersectionMin && intersectionMin <= projMax))
    {
        return true;
    }

    //if segment inside circle
    if(intersectionMin <= projMin && projMin <= intersectionMax &&
            intersectionMin <= projMax && projMax <= intersectionMax)
    {
        return true;
    }

    return false;
}

QuadtreeCentroid::QuadtreeCentroid(float x, float y, float width, float height):
    x(x), y(y), width(width), height(height),
    nw(NULL), ne(NULL), sw(NULL), se(NULL)
{
}

QuadtreeCentroid::~QuadtreeCentroid()
{
    if(isNode())  //Useful?
    {
        delete nw;
        delete ne;
        delete sw;
        delete se;
    }
}

float QuadtreeCentroid::getX() const { return x; }

float QuadtreeCentroid::getY() const { return y; }

float QuadtreeCentroid::getWidth() const { return width; }

float QuadtreeCentroid::getHeight() const { return height; }

size_t QuadtreeCentroid::getNumberMembers() const
{
    if(isNode())
    {
        return nw->getNumberMembers() + ne->getNumberMembers() +
            sw->getNumberMembers() + se->getNumberMembers();
    }
    return points.size();
}

bool QuadtreeCentroid::insert(Centroid point)
{
    if(!inBoundary(point))
        return false;

    if(isNode())
        return insertInChildren(point);

    if(points.size() < MAX)
        points.push_back(point);
    else
    {
        subdivise();
        insertInChildren(point);
    }

    return true;
}

bool QuadtreeCentroid::inBoundary(Centroid point)
{
    return (point.x >= x) && (point.x <= (x + width)) &&
        (point.y >= y) && (point.y <= (y + height));
}

bool QuadtreeCentroid::inRange(Centroid point, float range)
{
    Centroid a, b;

    if(inBoundary(point))
        return true;

    //Checking x axes
    a.x = x;
    a.y = y;
    b.x = x + width;
    b.y = y;
    if(intersectsAxe(point, range, a, b, true))
        return true;

    a.y += height;
    b.y += height;
    if(intersectsAxe(point, range, a, b, true))
        return true;

    //Checking y axes
    a.x = x;
    a.y = y;
    b.x = x;
    b.y = y + height;
    if(intersectsAxe(point, range, a, b, false))
        return true;

    a.x += width;
    b.x += width;
    if(intersectsAxe(point, range, a, b, false))
        return true;

    return false;
}

//Unsorted
v_Centroid QuadtreeCentroid::getClosestPoints(const Centroid point, float range)
{
    v_Centroid result, receive;
    float range2 = range * range;

    //if no point in the range area
    if(!inRange(point, range))
    {
        return result;
    }

    if(isNode())
    {
        receive = nw->getClosestPoints(point, range);
        result.insert(result.end(), receive.begin(), receive.end());
        receive = ne->getClosestPoints(point, range);
        result.insert(result.end(), receive.begin(), receive.end());
        receive = sw->getClosestPoints(point, range);
        result.insert(result.end(), receive.begin(), receive.end());
        receive = se->getClosestPoints(point, range);
        result.insert(result.end(), receive.begin(), receive.end());
        return result;
    }

    for(size_t i=0; i < points.size(); i++)
    {
        if(range2 >= (pow(points[i].x - point.x, 2) +
                    pow(points[i].y - point.y, 2)))
        {
            result.push_back(points[i]);
        }
    }

    return result;
}
v_Centroid QuadtreeCentroid::getClosestPointsSorted(const Centroid point, float range)
{
    v_Centroid result = getClosestPoints(point, range);

    if(result.size() <= 1)
        return result;

    // Bubble sort because static errors annoy me off (polite version)
    size_t i = 0, j = 0;
    Centroid temp;
    for(i=0; i < result.size() - 1; i++)
    {
        for(j= result.size()-1; j > i; j--)
        {
            if(closerToPoint(result[j], result[i], point))
            {
                temp = result[j];
                result[j] = result[i];
                result[i] = temp;
            }
        }
    }

    return result;
}

void QuadtreeCentroid::print()
{
    std::cout << "|| (" << x << "; " << y << ") to";
    std::cout << "(" << x + width << "; ";
    std::cout << y + height << ") ||" << std::endl;

    if(isNode())
    {
        std::cout << "[" << std::endl;
        nw->print();
        std::cout << "---" << std::endl;
        ne->print();
        std::cout << "---" << std::endl;
        sw->print();
        std::cout << "---" << std::endl;
        se->print();
        std::cout << "]" << std::endl;
        return;
    }

    for(size_t i=0; i < points.size(); i++)
    {
        std::cout << "(" << points[i].x << "; " << points[i].y << ")" << std::endl;
    }
}
