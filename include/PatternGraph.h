#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iterator>
#include <set>
#include <pthread.h>

using namespace std;

class PatternGraph{
public:
    int edge;
    int node;
    int *index_ptr_of_pattern;
    int *indices_of_pattern;
    int *num_of_neighbor;
    int *order;
    PatternGraph(int e,int n);
    void GetTheNeighborOfEachNode();
    void GetTheMatchingOrder();
    void Clear();
};

