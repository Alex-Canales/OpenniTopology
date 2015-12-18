#ifndef _QUADTREE_H_
#define _QUADTREE_H_

#include <vector>
#include <iostream>
#include <cmath>
#include <cstddef>
#include <algorithm>

typedef struct {
    float x, y, width, height;
} Boundary;

/**
 * If want to optimise this class for the searching function:
 * use KD-tree instead. No more optimization for this class.
 */

/**
 * Class T must have public fields x and y
 */
template <typename T>
class Quadtree
{
    Boundary boundary;
    std::vector<T> points;
    Quadtree<T> *nw, *ne, *sw, *se;

    //If a closer to point than b: true.
    bool closerToPoint(T a, T b, T point)
    {
        float distA = pow(point.x - a.x, 2) + pow(point.y - a.y, 2);
        float distB = pow(point.x - b.x, 2) + pow(point.y - b.y, 2);

        return distA < distB;
    }

    void subdivise()
    {
        float wWest = boundary.width / 2;
        float hNorth = boundary.height / 2;
        float wEst = boundary.width - wWest;
        float hSouth = boundary.height - hNorth;
        float xWest = boundary.x;
        float yNorth = boundary.y;
        float xEst = boundary.x + wWest;
        float ySouth = boundary.y + hNorth;
        size_t i;

        nw = new Quadtree<T>(xWest, yNorth, wWest, hNorth);
        ne = new Quadtree<T>(xEst, yNorth, wEst, hNorth);
        sw = new Quadtree<T>(xWest, ySouth, wEst, hSouth);
        se = new Quadtree<T>(xEst, ySouth, wEst, hSouth);

        for(i=0; i < points.size(); i++)
        {
            insertInChildren(points[i]);
        }
        points.clear();
    }

    bool isNode()
    {
        return nw != NULL;
    }

    bool insertInChildren(T point)
    {
        if(nw->insert(point))
            return true;
        if(ne->insert(point))
            return true;
        if(sw->insert(point))
            return true;
        return se->insert(point);
    }

    //If axeX is true, considers a.y == b.y, else considers a.x == b.x
    bool intersectsAxe(T point, float range, T a, T b, bool axeX)
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

public:

    static const int MAX = 8;

    Quadtree(Boundary boundary):
        boundary(boundary), nw(NULL), ne(NULL), sw(NULL), se(NULL)
    {}

    Quadtree(float x, float y, float width, float height):
        nw(NULL), ne(NULL), sw(NULL), se(NULL)
    {
        boundary.x = x;
        boundary.y = y;
        boundary.width = width;
        boundary.height = height;
    }

    ~Quadtree()
    {
        if(isNode())  //Useful?
        {
            delete nw;
            delete ne;
            delete sw;
            delete se;
        }
    }

    bool insert(T point)
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

    bool inBoundary(T point)
    {
        return (point.x >= boundary.x) &&
            (point.x <= (boundary.x + boundary.width)) &&
            (point.y >= boundary.y) &&
            (point.y <= (boundary.y + boundary.height));
    }

    bool inRange(T point, float range)
    {
        T a, b;

        if(inBoundary(point))
            return true;

        //Checking x axes
        a.x = boundary.x;
        a.y = boundary.y;
        b.x = boundary.x + boundary.width;
        b.y = boundary.y;
        if(intersectsAxe(point, range, a, b, true))
            return true;

        a.y += boundary.height;
        b.y += boundary.height;
        if(intersectsAxe(point, range, a, b, true))
            return true;

        //Checking y axes
        a.x = boundary.x;
        a.y = boundary.y;
        b.x = boundary.x;
        b.y = boundary.y + boundary.height;
        if(intersectsAxe(point, range, a, b, false))
            return true;

        a.x += boundary.width;
        b.x += boundary.width;
        if(intersectsAxe(point, range, a, b, false))
            return true;

        return false;
    }

    //Unsorted
    std::vector<T> getClosestPoints(const T point, float range)
    {
        std::vector<T> result, receive;
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

    std::vector<T> getClosestPointsSorted(const T point, float range)
    {
        std::vector<T> result = getClosestPoints(point, range);

        if(result.size() <= 1)
            return result;

        // Bubble sort because static errors annoy me off (polite version)
        size_t i = 0, j = 0;
        T temp;
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

    void print()
    {
        std::cout << "|| (" << boundary.x << "; " << boundary.y << ") to";
        std::cout << "(" << boundary.x + boundary.width << "; ";
        std::cout << boundary.y + boundary.height << ") ||" << std::endl;

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
};

#endif
