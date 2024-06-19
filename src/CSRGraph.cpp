#include <iostream>
#include <algorithm>
#include <vector>
#include <iterator>
#include <time.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <random>
#include <unordered_set>
#include "../include/CSRGraph.h"

using namespace std;

CSRGraph::CSRGraph() {
    node=0;
    edge=0;
}

void CSRGraph::ReadTheGraph(char *pathname){
    ifstream fin;
    fin.open(pathname);//n edges and m nodes
    fin >> edge >> node;
    edge = edge * 2;

    array = new int *[edge];  // 动态分配二维数组的行
    for (int i = 0; i < edge; ++i) {
        array[i] = new int[2];  // 动态分配二维数组的列
    }

    array_origen = new int *[edge];  // 动态分配二维数组的行
    for (int i = 0; i < edge; ++i) {
        array_origen[i] = new int[2];  // 动态分配二维数组的列
    }


    col_indices = new int[edge]();
    row_offsets = new int[node + 1]();
    number_of_neighbor=new int[node]();
    true_index = new int[edge]();

    for (int i = 0; i < edge; ++i) {
        int m1, n1;
        fin >> m1 >> n1;

        array[i][0] = m1;
        array[i][1] = n1;
        array_origen[i][0] = m1;
        array_origen[i][1] = n1;
        i++;
        array[i][0] = n1;
        array[i][1] = m1;
        array_origen[i][0] = n1;
        array_origen[i][1] = m1;
    }

    fin.close();

    sort(array, array + edge, compare);

    query_list = new int[array[edge - 1][0] + 1]();

}

bool CSRGraph::compare(const int* a, const int* b) {
    if (abs(a[0]) != abs(b[0])) {
        return abs(a[0]) < abs(b[0]);  // 按照第一个数从小到大排列
    }
    return abs(a[1]) < abs(b[1]);  // 对于相同的第一个数，按照第二个数从小到大排列
}

void CSRGraph::removeDuplicates() {
    int newSize = 0;
    for (int i = 0; i < edge; i++) {
        if (i == 0 || !(array[i][0] == array[i - 1][0] && array[i][1] == array[i - 1][1])) {  // 如果当前元素不等于前一个元素，则将其保留
            array[newSize][0] = array[i][0];
            array[newSize][1] = array[i][1];
            newSize++;
        }
    }

    edge = newSize;  // 更新数组大小
}

void CSRGraph::GetFourArray(){
    for (int i = 0; i < edge; i++) {
        true_index[i] = array[i][0];
    }

    //get the true index
    int slow = 0; // 慢指针
    for (int fast = 1; fast < edge; fast++) { // 快指针
        if (true_index[fast] != true_index[slow]) {
            slow++;
            true_index[slow] = true_index[fast]; // 将非重复元素移到慢指针位置
        }
    }

    for (int i = 1; i < node + 1; i++) {
        query_list[true_index[i - 1]] = i;//已知真正的编号 要找到对应是第几个（对应1~m)
    }

    for (int i = 0; i < edge; i++) {
        row_offsets[query_list[array[i][0]] - 1]++;
        number_of_neighbor[query_list[array[i][0]] - 1]++;
    }

    int temp = 0;
    int accumulate = 0;

    for (int i = 0; i < node; i++) {
        temp = row_offsets[i];
        row_offsets[i] = accumulate;
        accumulate += temp;
    }
    row_offsets[node] = accumulate;

    for (int i = 0; i < edge; i++) {
        col_indices[i] = array[i][1];
    }

}

vector<int> CSRGraph::generateUniqueRandomInts(int n, int m) {
        // 创建一个包含 0 到 n 范围内所有数字的向量
        vector<int> nums(n + 1);
        for (int i = 0; i < nums.size(); i++) {
            nums[i] = i;
        }

        // 使用 std::random_device 和 std::mt19937 随机打乱向量
        random_device rd;
        mt19937 gen(rd());
        shuffle(nums.begin(), nums.end(), gen);

        // 取出前 m 个元素
        vector<int> result(nums.begin(), nums.begin() + m);
        return result;
}

void CSRGraph::Clear(){
    delete[] col_indices;
    delete[] row_offsets;
    delete[] true_index;
    delete[] number_of_neighbor;

    for (int i = 0; i < node; i++) {
        delete[] array[i];
        delete[] array_origen[i];
    }
    delete[] array;
    delete [] array_origen;

    delete[] query_list;
}

void CSRGraph::Clear_new(){
    delete[] new_col_indices;
    delete[] new_row_offsets;
}

int CSRGraph::check_neighbor(int a,int b){
    int ret =0;
    for(int i=row_offsets[query_list[a]-1];i<row_offsets[query_list[a]];i++){
        if(col_indices[i]==b){
            ret=1;
        }
    }
    return ret;
}

void CSRGraph::Generate_stages(){
    int percent=100;
    int change_of_edge=edge/percent;
    int *new_number_of_neighbor=new int[node]();
    new_row_offsets=new int[node+1]();
    new_col_indices=new int[edge-(change_of_edge/2)*2+change_of_edge]();

    for(int i=0;i<node;i++){
        new_number_of_neighbor[i]=number_of_neighbor[i];
    }

    //deleting generatetion
    vector <int> random_delete= generateUniqueRandomInts((edge/3)-1,change_of_edge/2);//抽取个数 只取前半部分的点为改变对象 50%

//    //testing
//    for(int t=0;t<random_delete.size();t++){
//        cout<<random_delete[t]<<" ";
//    }
//    cout<<endl;

    array_changing = new int *[change_of_edge*2];//包括了add & delete正向和反向的
    for (int i = 0; i < change_of_edge*2; ++i) {
        array_changing[i] = new int[2];
    }

    //delete generation
    for(int i=0;i<change_of_edge/2;i++){
        int begin=array_origen[random_delete[i]*2][0];
        int end=array_origen[random_delete[i]*2][1];
        array_changing[i*2][0]=-begin;
        array_changing[i*2][1]=-end;
        array_changing[i*2+1][0]=-end;
        array_changing[i*2+1][1]=-begin;
    }


    //add generation
    vector <int> random= generateUniqueRandomInts(node/2-1,change_of_edge);

//    //testing
//    for(int i=0;i<change_of_edge;i++){
//        cout<<random[i]<<" ";
//    }
//    cout<<endl;

    for(int i=change_of_edge/2;i<change_of_edge;i++){
        int begin=random[(i-change_of_edge/2)*2];
        int end=random[(i-change_of_edge/2)*2+1];

        int begin_node=true_index[begin];
        int end_node=true_index[end];
        while (check_neighbor(begin_node,end_node)!=0||begin_node==end_node){
            end+=1;
            end=end%node;
            end_node=true_index[end];
        }

        array_changing[i*2][0]=begin_node;
        array_changing[i*2][1]=end_node;
        array_changing[i*2+1][0]=end_node;
        array_changing[i*2+1][1]=begin_node;
    }


    //testing
    cout<<"node for choosing finished"<<endl;
//    for(int t=0;t<change_of_edge*2;t++){
//        cout<<array_changing[t][0]<<" "<<array_changing[t][1]<<endl;
//    }


    sort(array_changing, array_changing + change_of_edge*2, compare);

    //testing
    cout<<"node after sorting finished"<<endl;
//    for(int t=0;t<change_of_edge*2;t++){
//        cout<<array_changing[t][0]<<" "<<array_changing[t][1]<<endl;
//    }

    //record the information of adding and deleting
    int * temp=new int[abs(array_changing[change_of_edge*2-1][0])+1]();
    int size=0;
    for(int i=0;i<change_of_edge*2;i++){
        if(temp[abs(array_changing[i][0])]==0){
            size++;
        }
        temp[abs(array_changing[i][0])]++;
    }

    int * edge_change=new int[size]();
    int * numFrequency=new int[size]();
    int ptr=0;
    for(int i=0;i<abs(array_changing[change_of_edge*2-1][0])+1;i++){
        if(temp[i]!=0){
            edge_change[ptr]=i;
            numFrequency[ptr]=temp[i];
            ptr++;
        }
    }

    int * changing_row_offset=new int[size+1]();
    int acc=0;
    for(int i=0;i<size;i++){
        changing_row_offset[i]=acc;
        acc+=numFrequency[i];
    }
    changing_row_offset[size]=acc;

    //testing
    cout<<"the two array finished"<<endl;
//    for(int i=0;i<size;i++){
//        cout<<edge_change[i]<<" "<<numFrequency[i]<<endl;
//    }
//    for(int i=0;i<size+1;i++){
//        cout<<changing_row_offset[i]<<" ";
//    }
//    cout<<endl;

    //testing
    cout<<"here 1"<<endl;


    delete [] numFrequency;
    delete [] temp;

    int ptr1=0; //ptr of edge_change
    int ptr2=0; //ptr of new_col_indices
    for(int i=0;i<node;i++){
        if(true_index[i]==edge_change[ptr1]){
            //compare and fill in
            int ptr_origen=row_offsets[query_list[true_index[i]]-1];
            int ptr_change=changing_row_offset[ptr1];
            //有一个到达了最后就停下
            while(ptr_origen<row_offsets[query_list[true_index[i]]]&&ptr_change<changing_row_offset[ptr1+1]){
                //deleting
                if(abs(array_changing[ptr_change][1])==col_indices[ptr_origen]){
                    new_number_of_neighbor[query_list[true_index[i]]-1]--;
                    ptr_origen++;
                    ptr_change++;
                } else {
                    //adding 而且不是废边（1，1）型
                    if(abs(array_changing[ptr_change][1])<col_indices[ptr_origen]){
                        new_number_of_neighbor[query_list[true_index[i]]-1]++;
                        new_col_indices[ptr2]=array_changing[ptr_change][1];
                        ptr2++;
                        ptr_change++;
                    } else if(abs(array_changing[ptr_change][1])>col_indices[ptr_origen]){
                        new_col_indices[ptr2]=col_indices[ptr_origen];
                        ptr2++;
                        ptr_origen++;
                    }
                }
            }
            //处理剩下没有重叠的部分
            if(ptr_origen<row_offsets[query_list[true_index[i]]]){
                while(ptr_origen<row_offsets[query_list[true_index[i]]]){
                    new_col_indices[ptr2]=col_indices[ptr_origen];
                    ptr2++;
                    ptr_origen++;
                }
            }

            if(ptr_change<changing_row_offset[ptr1+1]){
                while(ptr_change<changing_row_offset[ptr1+1]){
                    new_number_of_neighbor[query_list[true_index[i]]-1]++;
                    new_col_indices[ptr2]=array_changing[ptr_change][1];
                    ptr2++;
                    ptr_change++;
                }
            }
            ptr1++;
        } else {
            //directly copy
            for(int j=row_offsets[query_list[true_index[i]]-1];j<row_offsets[query_list[true_index[i]]];j++){
                new_col_indices[ptr2]=col_indices[j];
                ptr2++;
            }
        }
        if(i==node-2){
            cout<<"here 2"<<endl;
        }
    }

    //testing
    cout<<"the final two array for recording the new graph: "<<endl;
//    for(int i=0;i<node;i++){
//        cout<<new_number_of_neighbor[i]<<" ";
//    }
//    cout<<endl;
//    for(int i=0;i<ptr2;i++){
//        cout<<new_col_indices[i]<<" ";
//    }
//    cout<<endl;

    //get the new_row_offset
    int accu=0;
    for(int i=0;i<node;i++){
        new_row_offsets[i]=accu;
        accu+=new_number_of_neighbor[i];
    }
    new_row_offsets[node]=accu;

    new_edge=ptr2;


    //delete the new array
    for (int i = 0; i < change_of_edge*2; ++i) {
        delete [] array_changing[i];
    }
    delete [] array_changing;
    delete [] edge_change;
    delete [] new_number_of_neighbor;
    random_delete.clear();
    random.clear();
}