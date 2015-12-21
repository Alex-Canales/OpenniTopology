#include <iostream>
#include <cmath>

#include "kdtreeCentroid.h"
#include "geometry.h"

bool KdtreeCentroid::isNode()
{
    return (inferior != NULL || superior != NULL);
}

KdtreeCentroid::KdtreeCentroid(Centroid centroid):
    inferior(NULL), superior(NULL), point(centroid), splitX(true)
{
}

KdtreeCentroid::KdtreeCentroid(Centroid centroid, bool splitX):
    inferior(NULL), superior(NULL), point(centroid), splitX(splitX)
{
}

KdtreeCentroid::~KdtreeCentroid()
{
    if(isNode())
    {
        delete inferior;
        delete superior;
    }
}

void KdtreeCentroid::insert(Centroid centroid)
{
    bool goInferior = false;
    KdtreeCentroid **next = NULL;

    if(splitX)
        goInferior = (centroid.x < point.x);
    else
        goInferior = (centroid.y < point.y);

    next = goInferior ? &inferior : &superior;

    if(*next)
        (*next)->insert(centroid);
    else
        *next = new KdtreeCentroid(centroid, !splitX);
}

// centroid is the centroid which we look for neighbours
// outDistance2 is the distance between centroid and the returned centroid
Centroid KdtreeCentroid::nearestNeighbour(Centroid centroid, float *outDistance2)
{
    float distance2 = pow(centroid.x - point.x, 2) + pow(centroid.y - point.y, 2);
    *outDistance2 = distance2;
    bool goInferior = false;
    float distanceSplit2 = 0, distanceInChildren = *outDistance2;
    Centroid toGive = centroid;
    KdtreeCentroid **inside = NULL, **outside = NULL;  // order of the leaf check
    bool checkedInside = false, checkedOutside = false;
    Centroid givenInside, givenOutside;
    float distInside = 0, distOutside = 0;

    if(!isNode())
    {
        *outDistance2 = distance2;
        return point;
    }

    if(splitX)
    {
        goInferior = (centroid.x < point.x);
        distanceSplit2 = pow(centroid.x - point.x, 2);
    }
    else
    {
        goInferior = (centroid.y < point.y);
        distanceSplit2 = pow(centroid.y - point.y, 2);
    }

    if(goInferior)
    {
        inside = &inferior;
        outside = &superior;
    }
    else
    {
        inside = &superior;
        outside = &inferior;
    }

    if((*inside) != NULL)
    {
        checkedInside = true;
        givenInside = (*inside)->nearestNeighbour(centroid, &distanceInChildren);
        distInside = distanceInChildren;
    }
    if((*outside) != NULL && distanceInChildren >= distanceSplit2)
    {
        checkedOutside = true;
        givenOutside = (*outside)->nearestNeighbour(centroid, &distanceInChildren);
        distOutside = distanceInChildren;
    }

    if(checkedInside && checkedOutside)
    {
        if(distInside < distOutside)
        {
            toGive =  givenInside;
            distanceInChildren = distInside;
        }
        else
        {
            toGive =  givenOutside;
            distanceInChildren = distOutside;
        }
    }
    else
        toGive = (checkedInside) ? givenInside : givenOutside;

    if(distanceInChildren < distance2)
    {
        *outDistance2 = distanceInChildren;
        return toGive;
    }

    *outDistance2 = distance2;
    return point;
}

Centroid KdtreeCentroid::nearestNeighbour(Centroid centroid)
{
    float d;
    return  nearestNeighbour(centroid, &d);
}

//does not check number
void KdtreeCentroid::insertListResults(Centroid centroid, Centroid toInsert,
        std::vector<Centroid> &results, unsigned int number)
{
    float distance2 = pow(centroid.x - point.x, 2) + pow(centroid.y - point.y, 2);
    float dist2;
    std::vector<Centroid>::iterator it = results.end();

    if(results.size() == 0)
    {
        results.push_back(point);
        return;
    }

    it = results.begin();
    while(it < results.end())
    {
        dist2 = pow(centroid.x - (*it).x, 2) + pow(centroid.y - (*it).y, 2);
        if(distance2 <= dist2)
            break;
        it++;
    }

    results.insert(it, point);
    if(results.size() > number)
        results.pop_back();
}

void KdtreeCentroid::nearestNeighboursChildren(Centroid centroid,
            std::vector<Centroid> &results, unsigned int number)
{
    bool goInferior = false;
    float distanceSplit2 = 0;
    KdtreeCentroid **inside = NULL, **outside = NULL;  // order of the leaf check

    insertListResults(centroid, point, results, number);
    if(!isNode())
    {
        return;
    }

    if(splitX)
    {
        goInferior = (centroid.x < point.x);
        distanceSplit2 = pow(centroid.x - point.x, 2);
    }
    else
    {
        goInferior = (centroid.y < point.y);
        distanceSplit2 = pow(centroid.y - point.y, 2);
    }

    if(goInferior)
    {
        inside = &inferior;
        outside = &superior;
    }
    else
    {
        inside = &superior;
        outside = &inferior;
    }

    if((*inside) != NULL)
        (*inside)->nearestNeighboursChildren(centroid, results, number);

    Centroid &c = results[results.size() - 1];
    float d = pow(c.x - centroid.x, 2) + pow(c.y - centroid.y, 2);

    // If in the other side can but inserted closer points
    if((*outside) != NULL && d >= distanceSplit2)
        (*outside)->nearestNeighboursChildren(centroid, results, number);

}

std::vector<Centroid> KdtreeCentroid::nearestNeighbours(Centroid centroid,
            unsigned int number)
{
    std::vector<Centroid> results;
    if(number > 0)
        nearestNeighboursChildren(centroid, results, number);

    return results;
}
