#ifndef _KDTREE_CENTROID_H_
#define _KDTREE_CENTROID_H_

#include <vector>

#include "top_types.h"

/**
 * Will store Centroid (two dimensionnal points).
 */
class KdtreeCentroid
{
    KdtreeCentroid *inferior, *superior;  // The next branches
    Centroid point;
    bool splitX;

    // If cannot insert, return false. results is sorted
    void insertListResults(Centroid centroid, Centroid toInsert,
            std::vector<Centroid> &results, unsigned int number);

    bool isNode();
    Centroid nearestNeighbour(Centroid centroid, float *smallestRange2);
    void nearestNeighboursChildren(Centroid centroid,
            std::vector<Centroid> &results, unsigned int number);
public:
    KdtreeCentroid(Centroid centroid);
    KdtreeCentroid(Centroid centroid, bool splitX);
    ~KdtreeCentroid();
    void insert(Centroid centroid);
    Centroid nearestNeighbour(Centroid centroid);
    std::vector<Centroid> nearestNeighbours(Centroid centroid,
            unsigned int number);
};

#endif
