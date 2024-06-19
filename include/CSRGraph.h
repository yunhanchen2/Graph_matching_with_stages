#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iterator>
#include <set>
#include <pthread.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>
#include <random>
#include <unordered_set>

using namespace std;

class CSRGraph {
public:
    int node;
    int edge;
    int *col_indices;
    int *row_offsets;
    int *true_index;
    int **array;
    int **array_origen;
    int *query_list;
    int *number_of_neighbor;

    int new_edge;
    int* new_col_indices;
    int* new_row_offsets;

    CSRGraph();
    int check_neighbor(int a,int b);
    static bool compare(const int* a, const int* b);
    void ReadTheGraph(char *pathname);
    void removeDuplicates();
    void GetFourArray();
    void Clear();
    void Clear_new();
    void Generate_stages();
    static vector<int> generateUniqueRandomInts(int n, int m);

};

