//parallel cuckoo list
#include "list.h"
#include <cstddef>
#include <vector>
#include <mutex>
#include <random>
#include <algorithm>
#include <iostream>

using namespace std;



class ParallelList : public List<int>
{
public:
    //int N;
    std::vector<std::vector<std::vector<int>>> table;
    std::vector<std::mutex> locks;
    int PROBE_SIZE = 4;
    int THRESHOLD = 2;
    int LIMIT  = 10;
    ParallelList() : locks(2)
    {
        this->N  = 4;
        this->locks = std::vector<std::mutex>(2*N);
        this->table = std::vector<std::vector<std::vector<int>>>(4, std::vector<std::vector<int>>(4, std::vector<int>(PROBE_SIZE, 0)));
    }

    bool add(int x) override
    {
        cout<<"Adding "<<x<<endl;
        int y = 0;
        int index1 = hash1(x) % N;
        int index2 = hash2(x) % N;
        int i = -1, h = -1;
        bool must_resize = false;
        if(contains(x))
            return false;
        acquire(x);
        std::vector<int> table1List = this->table[0][hash1(x)];
        std::vector<int> table2List = this->table[1][hash2(x)];
        // output table1
        int table1Size = 0;
        for(int i = 0; i < table1List.size(); i++)
           if(table1List[i] != 0)
               table1Size++;
        int table2Size = 0;
        for(int i = 0; i < table2List.size(); i++)
           if(table2List[i] != 0)
               table2Size++;
        //cout<<"Table1 size: "<<table1Size<<endl;
        //cout<<"Table2 size: "<<table2Size<<endl;
        if(table1Size < THRESHOLD){
            table1List[table1Size] = x;
            this->table[0][hash1(x)] = table1List;
            release(x);
            print();
            return true;
        }
        else if(table2Size < THRESHOLD){
            table2List[table2Size] = x;
            this->table[1][hash2(x)] = table2List;
            release(x);
            print();
            return true;
        }
        else if (table1Size < PROBE_SIZE){
            
            cout << "Added to list 1 with resize needed. table1Size: " << table1Size << endl;

            table1List[table1Size] = x;
            this->table[0][hash1(x)] = table1List;
            y = table1List[0];
            i = 0;
            h = hash1(y);

            cout << "i: " << i << " h: " << h << endl;
        }
        else if (table2Size < PROBE_SIZE){
            cout << "Table2 size: " << table2Size << endl;

            table2List[table2Size] = x;
            this->table[1][hash2(x)] = table2List;
            y = table2List[0];
            i = 1;
            h = hash2(y);
        }
        else{
            must_resize = true;
        }
        release(x);
        print();

        if(must_resize){
            resize();
            add(x);
        }
        else if(!relocate(i,h)){
            resize();
        }
        return true;
    }

    bool relocate(int i, int hi) {
        // i is the index of the table
        // hi is the hashed index of the value added to a list above threshold
        cout<<"Relocating oldest value in table "<<i<<" at index "<<hi<<" N: "<<this->N<<endl;

        // What if relocation happens and the first value in the index moves to an empty list

        // I want to resize the oldest value
        int hj = 0;
        int j = 1 - i;
        for(int round = 0; round < LIMIT; round++){
            // This is an empty list
            std::vector<int> iList = this->table[i][hi];
            int iListSize = 0;

            // Finding size of populated list
            for(int k = 0; k < iList.size(); k++){
                cout<<"k: "<<iList[k]<<endl;
                if(iList[k] != 0)
                    iListSize++;

            }
            //print();

            int y = iList[0];

            // This should not be reached if successfully relocates number to empty list
            // if(y==0){
            //     cout<<"empty List operation not needed"<<endl;
            //     continue;
            // }
            

            switch(i){
                case 0:
                    hj = hash2(y);
                    break;
                case 1:
                    hj = hash1(y);
                    break;
            }
            acquire(y);
            cout<<"Relocating "<<y<<" to table: "<<j<<" index: "<<hj<<endl;
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
                    cout<<"Successfully relocated to list below threshold"<<endl;
                    return true;
                }
                else if (jListSize < PROBE_SIZE) {
                    jList[jListSize] = y;
                    this->table[j][hj] = jList;
                    i = 1-i;
                    hi = hj;
                    j = 1-j;
                    release(y);
                }
                else{
                    //iList.push_back(y);
                    iList[iListSize] = y;
                    this->table[i][hi] = iList;
                    release(y);
                    cout << "Successfully relocated to list below threshold" << endl;
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
            table1List.erase(std::remove_if(table1List.begin(), table1List.end(), [&](int var){return var==x; }), table1List.end());
            this->table[0][hash1(x)] = table1List;
            release(x);
            return true;
        }
        std::vector<int> table2List = this->table[1][hash2(x)];
        if(contains(table2List, x))
        {
            table2List.erase(std::remove(table2List.begin(), table2List.end(), x), table2List.end());
            this->table[1][hash2(x)] = table2List;
            release(x);
            return true;
        }
        release(x);
        return false;
    }

    bool remove(vector<int> tableIndex, int value, int i, int hi)
    {
        for(int j = 0; j < PROBE_SIZE; j++)
        {
            if(tableIndex[j] == value)
            {
                tableIndex.erase(std::remove(tableIndex.begin(), tableIndex.end(), value), tableIndex.end());
                this->table[i][hi] = tableIndex;
                return true;
            }
        }
        return false;
    }

    bool contains(int x) override
    {
        acquire(x);
        std::vector<int> table1List = this->table[0][hash1(x)];
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

    bool contains(vector<int> tableIndex,int value)
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
        for(int i=0; i<2; i++)
        {
            cout<<"Table "<<i<<endl;
            for(int j=0; j<this->N; j++)
            {
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
        return 0;
    }

    // rand is not thread safe, but this should only be called on a single thread
    void populate(int x) override
    {
        for (int i = 0; i < x; i++)
        {
            this->add(rand() % x + 1);
        }
    }

    void resize() override
    {
        cout<<"Resizing"<<endl;
        std::vector<std::vector<std::vector<int>>> *newTable = new std::vector<std::vector<std::vector<int>>>(this->N*2, std::vector<std::vector<int>>(this->N*2, std::vector<int>(PROBE_SIZE, 0)));

        for(int i=0;i<this->N; i++)
        {
            (*newTable)[0][i] = this->table[0][i];
            (*newTable)[1][i] = this->table[1][i];
        }

        this->table = (*newTable);
        this->N = 2*this->N;
        this->locks = std::vector<std::mutex>(2*this->N);
    }



    void acquire(int x)
    {
        this->locks[hash1(x)].lock();
        this->locks[this->N + hash2(x)].lock();
    }

    void release(int x)
    {
        this->locks[hash1(x)].unlock();
        this->locks[this->N + hash2(x)].unlock();
    }

    int hash1(int x)
    {
        return x % this->N; 
        // if N = 4
        // 0 1 2 3
    }

    int hash2(int x)
    {
        return 1 + (x % (this->N - 1));
        // if N = 4 -> n-1 = 3
        // 0 1 2
        // 1 2 3
    }
    
    int randomizer(int max)
    {
        static thread_local std::mt19937 generator;
        std::uniform_int_distribution<int> distribution(0, max);
        return distribution(generator);
    }
};
