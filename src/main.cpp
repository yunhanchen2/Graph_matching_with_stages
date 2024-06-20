#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iterator>
#include <set>
#include <pthread.h>
#include "../include/PatternGraph.h"
#include "../include/CSRGraph.h"
#include <chrono>

using namespace std;
using namespace chrono;

static int number_of_thread;

static CSRGraph graph;

class DataPassingToThreads{
public:
    static int * num_of_neighbor;
    static int * order;
    int * passing_node_to_thread_of_each;
    int round_index;
    int *neighbor_of_prenode_pattern;
    int size_of_neighbor_of_prenode_pattern;
    int number_of_matching;
    DataPassingToThreads(int *aPassing_node_to_thread_of_each,int aRound_index,int * aNeighbor_of_prenode_pattern,int aSize_of_neighbor_of_prenode_pattern,int aNumber_of_matching){
        passing_node_to_thread_of_each=aPassing_node_to_thread_of_each;
        round_index=aRound_index;
        neighbor_of_prenode_pattern=aNeighbor_of_prenode_pattern;
        size_of_neighbor_of_prenode_pattern=aSize_of_neighbor_of_prenode_pattern;
        number_of_matching=aNumber_of_matching;
    }
};

int* DataPassingToThreads::num_of_neighbor = nullptr;
int* DataPassingToThreads::order = nullptr;


struct DataForPassingBack{
    int number_of_matching_node;
    vector <int> matching_node;
    int origin_matching; //the duplicate
    int new_matching;
};

struct ThreadData{
    DataPassingToThreads *data;
};

static vector<int> vectors_intersection(vector<int> v1,vector<int> v2){
    vector<int> v;
    sort(v1.begin(),v1.end());
    sort(v2.begin(),v2.end());
    set_intersection(v1.begin(),v1.end(),v2.begin(),v2.end(),back_inserter(v));//求交集
    return v;
}

void* graph_matching_threads(void *n){
    //in threads: get the neighbor and check degree store them in the vectors
    //record time

    ThreadData* dataT=(ThreadData*) n;
    DataPassingToThreads *dataPassingToThreads=dataT->data;

    DataForPassingBack *passingBack=new DataForPassingBack();

    passingBack->number_of_matching_node=0;


    if(dataPassingToThreads->round_index==0){
        //only check the degree
        for (int j = 0; j < dataPassingToThreads->number_of_matching; j++) {//满足其邻居条件以后,j为candidate node中的jth元素
            if ((graph.row_offsets[graph.query_list[dataPassingToThreads->passing_node_to_thread_of_each[j]]] - graph.row_offsets[graph.query_list[dataPassingToThreads->passing_node_to_thread_of_each[j]] - 1]) >= dataPassingToThreads->num_of_neighbor[dataPassingToThreads->order[dataPassingToThreads->round_index]]) {//degree也满足了
                passingBack->matching_node.push_back(dataPassingToThreads->passing_node_to_thread_of_each[j]);//存新match的
                passingBack->number_of_matching_node++;
            }

        }

    } else {
        int *tem;
        for(int i=0;i<dataPassingToThreads->number_of_matching;i++){
            //each time pick one group
            tem=new int[dataPassingToThreads->round_index];
            for(int j=0;j<dataPassingToThreads->round_index;j++){
                tem[j]=dataPassingToThreads->passing_node_to_thread_of_each[i*dataPassingToThreads->round_index+j];
            }

            //get the neighbors
            vector<int> back;
            vector< vector<int> > neibor(dataPassingToThreads->size_of_neighbor_of_prenode_pattern);
            for(int k=0;k<dataPassingToThreads->size_of_neighbor_of_prenode_pattern;k++){//将邻居放入vector中
                for(int r=graph.row_offsets[graph.query_list[tem[dataPassingToThreads->neighbor_of_prenode_pattern[k]]]-1];r<graph.row_offsets[graph.query_list[tem[dataPassingToThreads->neighbor_of_prenode_pattern[k]]]];r++){
                    neibor[k].push_back(graph.col_indices[r]);//放入的是对应的编号而非第几个
                }
                //join the vector
                if(k==0){
                    back=neibor[0];
                } else {
                    back= vectors_intersection(back,neibor[k]);
                }
            }


            //cut off the node before the node and have a new matching
            vector<int>::iterator it;
            for(it=back.begin();it!=back.end();){
                bool check=true;
                for(int j=0;j<dataPassingToThreads->round_index;j++){
                    if(*it==tem[j]){
                        it=back.erase(it);
                        check= false;
                    }
                }
                if(check){
                    it++;
                }
            }


            //check the degree
            for (int j = 0; j < back.size(); j++) {//满足其邻居条件以后,j为candidate node中的jth元素
                if ((graph.row_offsets[graph.query_list[back[j]]] - graph.row_offsets[graph.query_list[back[j]] - 1]) >= dataPassingToThreads->num_of_neighbor[dataPassingToThreads->order[dataPassingToThreads->round_index]]) {//degree也满足了
                    for (int k = 0; k < dataPassingToThreads->round_index; k++) {
                        passingBack->matching_node.push_back(tem[k]);//将原来的存回去

                    }
                    passingBack->matching_node.push_back(back[j]);//存新match的
                    passingBack->number_of_matching_node++;
                }
            }
        }
    }

    pthread_exit(passingBack);
}

void* graph_matching_threads_new(void *n){
    //in threads: get the neighbor and check degree store them in the vectors
    //record time

    ThreadData* dataT=(ThreadData*) n;
    DataPassingToThreads *dataPassingToThreads=dataT->data;

    DataForPassingBack *passingBack=new DataForPassingBack();

    passingBack->number_of_matching_node=0;
    passingBack->new_matching=0;
    passingBack->origin_matching=0;

    if(dataPassingToThreads->round_index==0){
        //only check the degree
        for (int j = 0; j < dataPassingToThreads->number_of_matching; j++) {//满足其邻居条件以后,j为candidate node中的jth元素
            if ((graph.new_row_offsets[graph.query_list[abs(dataPassingToThreads->passing_node_to_thread_of_each[j])]] - graph.new_row_offsets[graph.query_list[abs(dataPassingToThreads->passing_node_to_thread_of_each[j])] - 1]) >= dataPassingToThreads->num_of_neighbor[dataPassingToThreads->order[dataPassingToThreads->round_index]]) {//degree也满足了
                passingBack->matching_node.push_back(dataPassingToThreads->passing_node_to_thread_of_each[j]);//存新match的
                passingBack->number_of_matching_node++;
            }

        }

    } else {
        int *tem;
        for(int i=0;i<dataPassingToThreads->number_of_matching;i++){
            //each time pick one group
            tem=new int[dataPassingToThreads->round_index];
            for(int j=0;j<dataPassingToThreads->round_index;j++){
                tem[j]=dataPassingToThreads->passing_node_to_thread_of_each[i*dataPassingToThreads->round_index+j];
            }

            //get the neighbors
            vector<int> back;
            vector< vector<int> > neibor(dataPassingToThreads->size_of_neighbor_of_prenode_pattern);

            for(int k=0;k<dataPassingToThreads->size_of_neighbor_of_prenode_pattern;k++){//将邻居放入vector中

                for(int r=graph.new_row_offsets[graph.query_list[abs(tem[dataPassingToThreads->neighbor_of_prenode_pattern[k]])]-1];r<graph.new_row_offsets[graph.query_list[abs(tem[dataPassingToThreads->neighbor_of_prenode_pattern[k]])]];r++){
                    //find the duplicate matching after changing stages
                    if(dataPassingToThreads->round_index==1){
                        if(graph.data_recording[r]==2){
                            tem[dataPassingToThreads->neighbor_of_prenode_pattern[k]]=-tem[dataPassingToThreads->neighbor_of_prenode_pattern[k]];
                            neibor[k].push_back(-graph.new_col_indices[r]);//放入的是对应的编号而非第几个
                            passingBack->new_matching++;
                        } else {
                            neibor[k].push_back(graph.new_col_indices[r]);//放入的是对应的编号而非第几个
                            passingBack->origin_matching++;
                        }
                    } else {
                        if(tem[dataPassingToThreads->neighbor_of_prenode_pattern[k]]<0||graph.data_recording[r]==2){
                            neibor[k].push_back(-graph.new_col_indices[r]);//放入的是对应的编号而非第几个
                            passingBack->new_matching++;
                        } else {
                            neibor[k].push_back(graph.new_col_indices[r]);//放入的是对应的编号而非第几个
                            passingBack->origin_matching++;
                        }
                    }

                }
                //join the vector
                if(k==0){
                    back=neibor[0];
                } else {
                    back= vectors_intersection(back,neibor[k]);
                }
            }


            //cut off the node before the node and have a new matching
            vector<int>::iterator it;
            for(it=back.begin();it!=back.end();){
                bool check=true;
                for(int j=0;j<dataPassingToThreads->round_index;j++){
                    if(abs(*it)==abs(tem[j])){
                        it=back.erase(it);
                        check= false;
                    }
                }
                if(check){
                    it++;
                }
            }


            //check the degree
            for (int j = 0; j < back.size(); j++) {//满足其邻居条件以后,j为candidate node中的jth元素
                if ((graph.new_row_offsets[graph.query_list[abs(back[j])]] - graph.new_row_offsets[graph.query_list[abs(back[j])] - 1]) >= dataPassingToThreads->num_of_neighbor[dataPassingToThreads->order[dataPassingToThreads->round_index]]) {//degree也满足了
                    for (int k = 0; k < dataPassingToThreads->round_index; k++) {
                        passingBack->matching_node.push_back(tem[k]);//将原来的存回去

                    }
                    passingBack->matching_node.push_back(back[j]);//存新match的
                    passingBack->number_of_matching_node++;
                }
            }
        }
    }

    pthread_exit(passingBack);
}

int main(int argc,char* argv[]) {

    if (argc > 2) {
        number_of_thread=atoi(argv[2]);
        //get the pattern graph
        int e;
        int nod;

        cout<<"input the number of edge and node pattern graph"<<endl;
        cin>>e>>nod;

        PatternGraph patternGraph(e,nod);

        cout<<"input the index_ptr_of_pattern(from 0 to n)"<<endl;
        for(int i=0;i<patternGraph.node+1;i++){
            cin>>patternGraph.index_ptr_of_pattern[i];
        }

        cout<<"input the indices_of_pattern(from 0 to n)"<<endl;
        for(int i=0;i<patternGraph.edge*2;i++){
            cin>>patternGraph.indices_of_pattern[i];
        }

        //get neighbor of each node
        patternGraph.GetTheNeighborOfEachNode();

        //get the order
        patternGraph.GetTheMatchingOrder();

        //find out the restriction of nodes
        vector < vector<int> > nei(patternGraph.node);
        for(int i=0;i<patternGraph.node;i++){
            for(int j=0;j<i;j++){//node before them in the order
                for(int k=patternGraph.index_ptr_of_pattern[patternGraph.order[i]];k<patternGraph.index_ptr_of_pattern[patternGraph.order[i]+1];k++){
                    if(patternGraph.indices_of_pattern[k]==patternGraph.order[j]){
                        nei[patternGraph.order[i]].push_back(j);
                    }
                }
            }
        }

        //get the data graph
        char *pathname = argv[1];

        graph.ReadTheGraph(pathname);//read+sort

        graph.removeDuplicates();

        //find the true index
        graph.GetFourArray();//true_index[2]是第三小的node对应的编号(i=1~n-1)

        DataPassingToThreads::num_of_neighbor=patternGraph.num_of_neighbor;
        DataPassingToThreads::order=patternGraph.order;

        //do the matching
        pthread_t tid[number_of_thread];
        int counter;

        //record time
        auto start = system_clock::now();

        //divide the node into groups i=0->all node; i!=0->the node for last time
        vector<int> node_of_matching;
        int number_of_node_for_last_matching=graph.node;
        int begin_ptr=0;
        ThreadData *args=new ThreadData[number_of_thread];
        DataPassingToThreads *dataPassingToThreads[number_of_thread];
        DataForPassingBack* ptr_get;

        for(int i=0;i<patternGraph.node;i++){
            int* neighbor_of_prenode;

            int id=patternGraph.order[i];
            counter=0;
            //preparing data
            //query graph中的neighbor限制
            int size_of_neighbor_of_prenode=0;

            if(nei[id].size()!=0){
                neighbor_of_prenode=new int[nei[id].size()]();
                for(int p=0;p<nei[id].size();p++){
                    neighbor_of_prenode[p]=nei[id][p];
                }
                size_of_neighbor_of_prenode=nei[id].size();
            }

            //lunch the threads
            int full_node_for_each_thread=number_of_node_for_last_matching/number_of_thread;
            int remaining=number_of_node_for_last_matching-full_node_for_each_thread*number_of_thread;
            if(remaining==0){
                remaining=number_of_thread;
            } else {
                full_node_for_each_thread++;
            }
            int sharing_node_ptr=0;

            int *passing_node_to_thread_of_each[number_of_thread];
            int *number_of_matching=new int[number_of_thread]();

            for (int p = 0; p < number_of_thread; p++) {
                if(p<remaining){
                    number_of_matching[p]=full_node_for_each_thread;
                }else {
                    number_of_matching[p]=full_node_for_each_thread-1;
                }

                if(i==0){
                    passing_node_to_thread_of_each[p]=new int[number_of_matching[p]];
                    for(int t=0;t<number_of_matching[p];t++){
                        passing_node_to_thread_of_each[p][t]=graph.true_index[sharing_node_ptr];
                        sharing_node_ptr++;
                    }
                } else {
                    passing_node_to_thread_of_each[p]=new int[number_of_matching[p]*i];
                    for(int t=0;t<number_of_matching[p];t++){
                        for(int k=0;k<i;k++){
                            passing_node_to_thread_of_each[p][t*i+k]=node_of_matching[i*sharing_node_ptr+k];
                        }
                        sharing_node_ptr++;
                    }
                }

                dataPassingToThreads[p]=new DataPassingToThreads(passing_node_to_thread_of_each[p],i,neighbor_of_prenode,size_of_neighbor_of_prenode,number_of_matching[p]);

                args[p].data = dataPassingToThreads[p];

                pthread_create(&tid[p], NULL, graph_matching_threads, &args[p]);
            }

            //get vectors in each thread and merge them together
            ptr_get=new DataForPassingBack[number_of_thread];

            node_of_matching.clear();

            for (int p = 0; p < number_of_thread; p++) {
                void * ptr;
                pthread_join(tid[p], &ptr);
                ptr_get[p]=*((DataForPassingBack*) ptr);

                counter+=ptr_get[p].number_of_matching_node;

                node_of_matching.insert(node_of_matching.end(),ptr_get[p].matching_node.begin(),ptr_get[p].matching_node.end());

                ptr_get[p].matching_node.clear();
            }

            number_of_node_for_last_matching=counter;

            begin_ptr+=i;
            for(int d=0;d<number_of_thread;d++){
                if(passing_node_to_thread_of_each[d]!=nullptr){
                    delete [] passing_node_to_thread_of_each[d];
                }
            }

            if(ptr_get!=nullptr){
                delete [] ptr_get;
            }

            if(nei[id].size()!=0){
                delete [] neighbor_of_prenode;
            }

            if(number_of_matching!=nullptr){
                delete [] number_of_matching;
            }

        }

        auto end = system_clock::now();
        auto duration= duration_cast<microseconds>(end-start);
        cout<<"time: "<<double (duration.count())*microseconds ::period ::num/microseconds::period::den<<endl;

        set < set<int> > ss;
        for(int i=0;i<counter;i++){
            set<int> each;
            for(int j=0;j<patternGraph.node;j++){
                each.insert(node_of_matching[i*patternGraph.node+j]);
            }
            ss.insert(each);
        }

        //testing
        cout<<"total counting of original: "<<ss.size()<<endl;



        //generate 16 graphs

        for(int g=0;g<16;g++){
            graph.Generate_stages();

            //testing
            cout<<"get into matching "<<endl;

            //record time
            auto start_new = system_clock::now();

            //divide the node into groups i=0->all node; i!=0->the node for last time
            number_of_node_for_last_matching=graph.node;
            begin_ptr=0;
//            args=new ThreadData[number_of_thread];
            int new_matching_num=graph.change_node_num;
            int origin_matching_num=graph.node-graph.change_node_num; //for record duplicate (round0)

            for(int i=0;i<patternGraph.node;i++){
                int* neighbor_of_prenode;

                cout<<"getting into the round: "<<i<<endl;

                int id=patternGraph.order[i];
                counter=0;
                //preparing data
                //query graph中的neighbor限制
                int size_of_neighbor_of_prenode=0;

                //testing
                cout<<"in main testing1"<<endl;

                if(nei[id].size()!=0){
                    neighbor_of_prenode=new int[nei[id].size()]();
                    for(int p=0;p<nei[id].size();p++){
                        neighbor_of_prenode[p]=nei[id][p];
                    }
                    size_of_neighbor_of_prenode=nei[id].size();
                }


                //lunch the threads
                int full_node_for_each_thread=number_of_node_for_last_matching/number_of_thread;
                int remaining=number_of_node_for_last_matching-full_node_for_each_thread*number_of_thread;
                if(remaining==0){
                    remaining=number_of_thread;
                } else {
                    full_node_for_each_thread++;
                }
                int sharing_node_ptr=0;

                int *passing_node_to_thread_of_each[number_of_thread];
                int *number_of_matching=new int[number_of_thread]();

                //testing
                cout<<"in main testing2"<<endl;

                for (int p = 0; p < number_of_thread; p++) {
                    if(p<remaining){
                        number_of_matching[p]=full_node_for_each_thread;
                    }else {
                        number_of_matching[p]=full_node_for_each_thread-1;
                    }

                    if(i==0){
                        passing_node_to_thread_of_each[p]=new int[number_of_matching[p]];
                        for(int t=0;t<number_of_matching[p];t++){
                            passing_node_to_thread_of_each[p][t]=graph.true_index[sharing_node_ptr];
                            sharing_node_ptr++;
                        }
                    } else {
                        passing_node_to_thread_of_each[p]=new int[number_of_matching[p]*i];
                        for(int t=0;t<number_of_matching[p];t++){
                            for(int k=0;k<i;k++){
                                passing_node_to_thread_of_each[p][t*i+k]=node_of_matching[i*sharing_node_ptr+k];
                            }
                            sharing_node_ptr++;
                        }
                    }

                    //testing
                    cout<<"in main testing3 in round "<<p<<endl;

                    dataPassingToThreads[p]=new DataPassingToThreads(passing_node_to_thread_of_each[p],i,neighbor_of_prenode,size_of_neighbor_of_prenode,number_of_matching[p]);

                    //testing
                    cout<<"in main testing4 in round "<<p<<endl;

                    args[p].data = dataPassingToThreads[p];

                    //testing
                    cout<<"in main testing5 in round "<<p<<endl;

                    pthread_create(&tid[p], NULL, graph_matching_threads_new, &args[p]);

                    //testing
                    cout<<"in main testing5 in round "<<p<<endl;
                }

                //get vectors in each thread and merge them together
                ptr_get=new DataForPassingBack[number_of_thread];

                node_of_matching.clear();

                for (int p = 0; p < number_of_thread; p++) {
                    void * ptr;
                    pthread_join(tid[p], &ptr);
                    ptr_get[p]=*((DataForPassingBack*) ptr);

                    counter+=ptr_get[p].number_of_matching_node;
                    new_matching_num+=ptr_get[p].new_matching;
                    origin_matching_num+=ptr_get[p].origin_matching; //for record duplicate (round0)

                    node_of_matching.insert(node_of_matching.end(),ptr_get[p].matching_node.begin(),ptr_get[p].matching_node.end());

                    ptr_get[p].matching_node.clear();
                }

                number_of_node_for_last_matching=counter;

                begin_ptr+=i;

                for(int d=0;d<number_of_thread;d++){
                    if(passing_node_to_thread_of_each[d]!=nullptr){
                        delete [] passing_node_to_thread_of_each[d];
                    }
                }

                //testing
                cout<<"in main testing6"<<endl;


                if(ptr_get!=nullptr){
                    delete [] ptr_get;
                }

                if(nei[id].size()!=0){
                    delete [] neighbor_of_prenode;
                }

                if(number_of_matching!=nullptr){
                    delete [] number_of_matching;
                }

            }

            auto end_new = system_clock::now();
            auto duration_new= duration_cast<microseconds>(end_new-start_new);
            cout<<"time: "<<double (duration_new.count())*microseconds ::period ::num/microseconds::period::den<<endl;

            set < set<int> > ss;
            for(int i=0;i<counter;i++){
                set<int> each;
                for(int j=0;j<patternGraph.node;j++){
                    each.insert(node_of_matching[i*patternGraph.node+j]);
                }
                ss.insert(each);
            }

            //testing
            cout<<"total counting of new graph #"<<g+1<<": "<<ss.size()<<endl;
            cout<<"the percent of the duplication is: "<<double(new_matching_num*100)/origin_matching_num<<"%"<<endl;

            graph.Clear_new();
        }

        patternGraph.Clear();

        graph.Clear();
    }
    return 0;
}
