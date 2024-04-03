//parallel cuckoo list
#include "list.h"
#include <cstddef>
#include <vector>
#include <mutex>
#include <random>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <thread>
#include <shared_mutex>

using namespace std;



class ParallelList : public List<int>
{
public:
    //int N;
    std::vector<std::vector<std::vector<int>>> table;
    std::vector<std::mutex> locks;
    std::shared_mutex resize_lock;
    int PROBE_SIZE = 4;
    int THRESHOLD = 2;
    int LIMIT  = 20;
    ParallelList() : locks(2)
    {
        this->N  = 4;
        this->locks = std::vector<std::mutex>(2*N);
        this->table = std::vector<std::vector<std::vector<int>>>(2, std::vector<std::vector<int>>(this->N, std::vector<int>(PROBE_SIZE, 0)));
    }

    bool add(int x) override
    {
       //cout<<"Adding "<<x<<" Thread Id: "<<std::this_thread::get_id()<<endl;
       //print();
        int y = 0;
        // Normally the hash function generates the hash then outside of the function we reference it. But we dont do that here
        int index1 = hash1(x);
        int index2 = hash2(x);
        int i = -1, h = -1;
        bool must_resize = false;
        // if(contains(x))
        //     return false;
        if(add_contains(x))
            return false;
        // cout<<"Value does not exist"<<endl;
        //acquire(x);
        std::vector<int> table1List = this->table[0][index1];
        std::vector<int> table2List = this->table[1][index2];
        // output table1
        int table1Size = 0;
        for(int i = 0; i < table1List.size(); i++)
           if(table1List[i] != 0)
               table1Size++;
        int table2Size = 0;
        for(int i = 0; i < table2List.size(); i++)
           if(table2List[i] != 0)
               table2Size++;
        // cout<<"Table1 size: "<<table1Size<<endl;
        // cout<<"Table2 size: "<<table2Size<<endl;
        if(table1Size < THRESHOLD){
            table1List[table1Size] = x;
            this->table[0][index1] = table1List;
            // cout<<"Added to list 1 below threshold"<<endl;
            release(x);
             //print();
            return true;
        }
        else if(table2Size < THRESHOLD){
            table2List[table2Size] = x;
            this->table[1][index2] = table2List;
            //cout<<"Added to list 2 below threshold"<<endl;
            release(x);
             //print();
            return true;
        }
        else if (table1Size < PROBE_SIZE){
            table1List[table1Size] = x;
            this->table[0][index1] = table1List;
            //y = table1List[0];
            i = 0;
            //h = hash1(y);
            h = index1;
            //cout << "Added to list 1 with relocate needed. table1Size: " << table1Size<<" Index: "<< h << endl;
            //print();
            // cout << "i: " << i << " h: " << h << endl;
        }
        else if (table2Size < PROBE_SIZE){

            table2List[table2Size] = x;
            this->table[1][index2] = table2List;
            //y = table2List[0];
            i = 1;
            //h = hash2(y);
            h = index2;
            //cout << "Added to list 2 with relocate needed. table2size: " << table2Size <<" Index: "<< h << endl;
           //print();
        }
        else{
            must_resize = true;
        }
        release(x);
        //print();

        if(must_resize){
            resize();
            add(x);
        }
        else if(!relocate(i,h)){
            resize();
        }
        return true;
    }

    bool add_contains(int x){
        acquire(x);
        
        //print();
        //cout<<"hash1: "<<hash1(x)<<endl;
        std::vector<int> table1List = this->table[0][hash1(x)];
        //cout<<"this"<<endl;
        if(contains(table1List, x))
        {
            release(x);
            return true;
        }
        std::vector<int> table2List = this->table[1][hash2(x)];
        if(contains(table2List, x))
        {
            release(x);
            return true;
        }
        // cout<<" x: "<<x<<" does not exist in table"<<endl;
        //I inentionally do not release the locks
        return false;
    }

    bool relocate(int i, int hi) {
        // i is the index of the table
        // hi is the hashed index of the value added to a list above threshold

        // What if relocation happens and the first value in the index moves to an empty list

        // I want to resize the oldest value
        int hj = 0;
        int j = 1 - i;
        for(int round = 0; round < LIMIT; round++){
            // This is an empty list

            //cout<<"Relocating oldest value in table "<<i<<" at index "<<hi<<" N: "<<this->N<<" Attempt: "<<round<<" Thread Id:"<<std::this_thread::get_id()<<endl;
            std::vector<int> iList = this->table[i][hi];
            int iListSize = 0;

            // Finding size of populated list
            for(int k = 0; k < iList.size(); k++){
               // cout<<"k: "<<iList[k]<<endl;
                if(iList[k] != 0)
                    iListSize++;

            }
            //print();

            int y = iList[0];           

            switch(i){
                case 0:
                    hj = hash2(y);
                    break;
                case 1:
                    hj = hash1(y);
                    break;
            }
            //cout<<"Acquiring lock for: "<<y<<" Thread Id: "<<std::this_thread::get_id()<<endl;
            acquire(y);
            //cout << "Acquired lock for: " << y << " Thread Id: " << std::this_thread::get_id() << endl;

            //cout<<"Relocating "<<y<<" to table: "<<j<<" index: "<<hj<<endl;
            std::vector<int> jList = this->table[j][hj];
            int jListSize = 0;
            for(int i = 0; i < jList.size(); i++)
                if(jList[i] != 0)
                    jListSize++;
            
            if(remove(iList, y, i, hi)){ // remove an empty list
                if(jListSize < THRESHOLD){
                    jList[jListSize] = y;
                    this->table[j][hj] = jList;
                    release(y);
                    //cout<<"Successfully relocated to opposite list below threshold"<<endl;
                    return true;
                }
                else if (jListSize < PROBE_SIZE) {
                    jList[jListSize] = y;
                    this->table[j][hj] = jList;
                    i = 1-i;
                    hi = hj;
                    j = 1-j;
                    //cout<<"Successfully relocated to opposite list above threshold"<<endl;

                    release(y);
                }
                else{
                    //iList.push_back(y);
                    iList[iListSize] = y;
                    this->table[i][hi] = iList;
                    release(y);
                    //cout << "place the item back into the list" << endl;
                    return false;
                }
            } else if(iListSize >= THRESHOLD){
                release(y);
                continue;
            }
            else{
                release(y);
                return true;
            }
        }
        return false;

    }

    bool remove(int x) override
    {

        acquire(x);
        std::vector<int> table1List = this->table[0][hash1(x)];
        if(contains(table1List, x))
        {
            for(int j=0; j<PROBE_SIZE; j++)
            {
                if(table1List[j]==x){
                    table1List[j]=0;
                    for (int k = j; k < PROBE_SIZE - 1; k++)
                    {
                        if (table1List[k + 1] == 0)
                            break;
                        table1List[k] = table1List[k + 1];
                        table1List[k + 1] = 0;
                    }
                    this->table[0][hash1(x)] = table1List;
                    release(x);
                    return true;
                }
            }
            // shift original values back
        }
        std::vector<int> table2List = this->table[1][hash2(x)];
        if(contains(table2List, x))
        {

            for (int j = 0; j < PROBE_SIZE; j++)
            {
                if (table2List[j] == x)
                {
                    table2List[j] = 0;
                    for (int k = j; k < PROBE_SIZE - 1; k++)
                    {
                        if (table2List[k + 1] == 0)
                            break;
                        table2List[k] = table2List[k + 1];
                        table2List[k + 1] = 0;
                    }
                    this->table[1][hash2(x)] = table2List;
                    release(x);
                    return true;
                }
            }
        }
        release(x);
        return false;
    }

    bool remove(vector<int> tableIndex, int value, int i, int hi)
    {
        //cout<<"Removing "<<value<<" from table "<<i<<" at index "<<hi<<endl;

        for(int j = 0; j < PROBE_SIZE; j++)
        {
            if(tableIndex[j] == value)
            {
                tableIndex[j] = 0;

                // shift original values back
                for(int k = j; k < 3; k++)
                {
                    if(tableIndex[k+1] == 0)
                        break;
                    tableIndex[k] = tableIndex[k+1];
                    tableIndex[k+1] = 0;
                }
                this->table[i][hi] = tableIndex;
                return true;
            }
        }
        return false;
    }

    bool contains(int x) override
    {
        acquire(x);
        //print();
        //cout<<"hash1: "<<hash1(x)<<endl;
        std::vector<int> table1List = this->table[0][hash1(x)];
        //cout<<"this"<<endl;
        if(contains(table1List, x))
        {
            release(x);
            return true;
        }
        std::vector<int> table2List = this->table[1][hash2(x)];
        if(contains(table2List, x))
        {
            release(x);
            return true;
        }
        release(x);
        return false;
    }

    bool contains(vector<int> tableIndex, int value)
    {
        for(int i = 0; i < PROBE_SIZE; i++)
        {
            if(tableIndex[i] == value)
                return true;
        }
        return false;
    }

    void print() override
    {
        //cout<<"this should be 2: "<< this->table.size()<<endl;
        for(int i=0; i<2; i++)
        {
            cout<<"Table "<<i<<endl;
            for(int j=0; j<this->N; j++)
            {
                cout<<"Index "<<j<<": ";
                for(int k=0; k<PROBE_SIZE; k++)
                {
                    cout<<this->table[i][j][k]<<" ";
                }
                cout<<"|";
            }
            cout<<endl;
        }
    }

    int size() override
    {
        this->resize_lock.lock();
        int total = 0;
        for(int i=0; i<2;i++)
            for(int j=0; j<this->N;j++)
                for(int k=0; k<PROBE_SIZE;k++)
                    if(this->table[i][j][k]!=0)
                        total+=1;
        this->resize_lock.unlock();
        return total;   
    }

    // rand is not thread safe, but this should only be called on a single thread
    void populate(int x) override
    {
        for (int i = 0; i < x; i++)
        {
            //this->add(rand() % x + 1);
            this->add(i);
        }
    }

    void populate_parallel(int upper_limit, int thread_count)
    {
        //create threads
        std::vector<std::thread> threads;
        for (int i = 0; i < thread_count; i++)
        {
            threads.emplace_back(std::thread([&](int i) {
                for (int j = i; j < upper_limit; j += thread_count)
                {   
                    //cout<<"Thread Id: "<<std::this_thread::get_id()<<" Adding: "<<j<<endl;
                    this->add(j);
                }
               // cout<<"THREAD COMPLETE: "<<std::this_thread::get_id()<<endl;
            }, i));
        }

        //join threads
        //cout<<"Thread Count: "<<threads.size()<<endl;
        for (std::thread &thread : threads){
            //cout<<"Waiting for: "<<thread.get_id()<<endl;
            thread.join();
        }
    }

    void resize() override
    {
        // cout << "acquiring resize locks"
        //      << " Thread Id: " << std::this_thread::get_id()<<endl;
        this->resize_lock.lock();
        int oldN = this->N;
        for(int i = 0; i < oldN; i++){
            this->locks[i].lock();
            this->locks[this->N + i].lock();
        }
        // cout << "acquired locks"
        //      << " Thread Id: " << std::this_thread::get_id() << endl;

        // std::vector<std::vector<std::vector<int>>> *newTable = new std::vector<std::vector<std::vector<int>>>(this->N*2, std::vector<std::vector<int>>(this->N*2, std::vector<int>(PROBE_SIZE, 0)));
        // for(int i=0;i<this->N; i++)
        // {//TODO 
        //     (*newTable)[0][i] = this->table[0][i];
        //     (*newTable)[1][i] = this->table[1][i];
        // }



        // // this->table = (*newTable);
        std::vector<std::vector<std::vector<int>>> *newTable = new std::vector<std::vector<std::vector<int>>>(2, std::vector<std::vector<int>>(this->N*2, std::vector<int>(PROBE_SIZE, 0)));

        // Copy elements from this->table to newTable
        for(int i = 0; i < 2; i++) {
            for(int j = 0; j < this->N; j++) {
                for(int k = 0; k < PROBE_SIZE; k++) {
                    (*newTable)[i][j][k] = this->table[i][j][k];
                    (*newTable)[i][j][k] = this->table[i][j][k];
                }
            }
        }



        // Deallocate memory for this->table if necessary
        // (Assuming this->table was previously dynamically allocated)
        //delete this->table;

        // Assign newTable to this->table
        this->table = *newTable;
        // Deallocate memory for newTable since we no longer need it
        delete newTable;
        this->N = 2 * this->N;

        
        for(int i = 0; i < oldN; i++){
            
            this->locks[i].unlock();
            this->locks[oldN + i].unlock();
        }

        // cout << "releasing locks"
        //      << " Thread Id: " << std::this_thread::get_id() << endl;

        this->locks = std::vector<std::mutex>(2*this->N);
        //cout<<"locklist size: "<<this->locks.size()<<endl;
        this->resize_lock.unlock();

        //cout<<"Resized to "<<this->N<<endl;
    }



    void acquire(int x)
    {
        // cout<<"acquire lockindex1: "<<hash1(x)<<" lockindex2: "<<this->N+hash2(x)<<" locksize: "<<this->locks.size()<<endl;
        this->resize_lock.lock_shared();
        this->locks[hash1(x)].lock();
        this->locks[this->N + hash2(x)].lock();
    }

    void release(int x)
    {
        // cout<<"release lockindex1: "<<hash1(x)<<" lockindex2: "<<this->N+hash2(x)<<" locksize: "<<this->locks.size()<<endl;
        this->resize_lock.unlock_shared();
        this->locks[hash1(x)].unlock();
        this->locks[this->N + hash2(x)].unlock();
    }

    // int hash1(int x)
    // {
    //     return x % this->N; 
    //     // if N = 4
    //     // 0 1 2 3
    // }

    // int hash2(int x)
    // {
    //     return 1 + (x % (this->N - 1));
    //     // if N = 4 -> n-1 = 3
    //     // 0 1 2
    //     // 1 2 3
    // }


    int hash1(int key) {
    const double A = 0.6180339887; // Fractional part of (√5 - 1) / 2
    return static_cast<int>(std::floor(this->N * std::fmod(key * A, 1)));
    }

    int hash2(int key) {
        const double A = 0.6180339887; // Fractional part of (√5 - 1) / 2
        return static_cast<int>((this->N - 1) - std::floor(this->N * std::fmod(key * A, 1)));
    }
    
    int randomizer(int max)
    {
        static thread_local std::mt19937 generator;
        std::uniform_int_distribution<int> distribution(0, max);
        return distribution(generator);
    }
};
