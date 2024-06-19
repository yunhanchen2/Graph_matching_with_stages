#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iterator>
#include <set>
#include <time.h>
#include <pthread.h>

#include "../include/PatternGraph.h"

using namespace std;

PatternGraph::PatternGraph(int e,int n){
    edge=e;
    node=n;
    index_ptr_of_pattern=new int[n+1]();
    indices_of_pattern=new int[e*2]();
    num_of_neighbor=new int[n];
    order=new int[n];
}
void PatternGraph::GetTheNeighborOfEachNode(){
    for(int i=0;i<node;i++){
        num_of_neighbor[i]=index_ptr_of_pattern[i+1]-index_ptr_of_pattern[i];
    }
}
void PatternGraph::GetTheMatchingOrder(){
    int max=node-1;
    int marker=0;
    while(marker!=node){
        for(int i=0;i<node;i++){
            if(max==num_of_neighbor[i]){
                order[marker]=i;
                marker++;
            }
        }
        max--;
    }
}

void PatternGraph::Clear(){
    if (index_ptr_of_pattern != nullptr) {
        delete[] index_ptr_of_pattern;
    }

    if (indices_of_pattern != nullptr) {
        delete[] indices_of_pattern;
    }

    if (num_of_neighbor != nullptr) {
        delete[] num_of_neighbor;
    }

    if (order != nullptr) {
        delete[] order;
    }
}


