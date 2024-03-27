//parallel cuckoo list
#include "list.cpp"
#include <cstddef>
#include <vector>
#include <mutex>
#include <random>

using namespace std;

int PROBE_SIZE = 4;
int THRESHOLD = 2;
int LIMIT  = 10;

class ParallelList : public List<int>
{
public:
    int capactiy;
    std::vector<std::vector<std::vector<int>>> table;
    std::vector<std::vector<std::mutex>> locks;
    ParallelList(int size)
    {
        this->capactiy = size;
        this->N  = 2;
        this->locks = std::vector<std::vector<std::mutex>>(2, std::vector<std::mutex>(size));
        this->table = std::vector<std::vector<std::vector<int>>>(size, std::vector<std::vector<int>>(size, std::vector<int>(PROBE_SIZE, 0)));
    }

    bool add(int x) override
    {
        int y = 0;
        int index1 = hash1(x) % capactiy;
        int index2 = hash2(x) % capactiy;
        int i = -1, h = -1;
        bool must_resize = false;
        acquire(x);
        if(contains(x))
            return false;
        std::vector<int> table1List = this->table[0][hash1(x)];
        std::vector<int> table2List = this->table[1][hash2(x)];
        if(table1List.size() < THRESHOLD){
            table1List.push_back(x);
            release(x);
            return true;
        }
        else if(table2List.size() < THRESHOLD){
            table2List.push_back(x);
            release(x);
            return true;
        }
        else if (table1List.size() < PROBE_SIZE){
            table1List.push_back(x);
            y = table1List[0];
            i = 0;
            h = hash2(y);
        }
        else if (table2List.size() < PROBE_SIZE){
            table2List.push_back(x);
            y = table2List[0];
            i = 1;
            h = hash1(y);
        }
        else{
            must_resize = true;
        }
        release(x);

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
        int hj = 0;
        int j = 1 - i;
        for(int round = 0; round < LIMIT; round++){
            std::vector<int> iList = this->table[i][hi];
            int y = iList[0];
            switch(i){
                case 0:
                    hj = hash2(y);
                    break;
                case 1:
                    hj = hash1(y);
                    break;
            }
            acquire(y);
            std::vector<int> jList = this->table[j][hj];
            if(remove(iList, y)){
                if(jList.size() < THRESHOLD){
                    jList.push_back(y);
                    release(y);
                    return true;
                }
                else if (jList.size() < PROBE_SIZE) {
                    jList.push_back(y);
                    i = 1-i;
                    hi = hj;
                    j = 1-j;
                }
                else{
                    iList.push_back(y);
                    release(y);
                    return false;
                }
            } else if(iList.size() >= THRESHOLD){
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
            table1List.erase(std::remove(table1List.begin(), table1List.end(), x), table1List.end());
            release(x);
            return true;
        }
        std::vector<int> table2List = this->table[1][hash2(x)];
        if(contains(table2List, x))
        {
            table2List.erase(std::remove(table2List.begin(), table2List.end(), x), table2List.end());
            release(x);
            return true;
        }
        release(x);
        return false;
    }

    bool remove(vector<int> tableIndex,int value)
    {
        for(int i = 0; i < PROBE_SIZE; i++)
        {
            if(tableIndex[i] == value)
            {
                tableIndex.erase(std::remove(tableIndex.begin(), tableIndex.end(), value), tableIndex.end());
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
        // TODO: Implement print function for ParallelList
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
        std::vector<std::vector<std::vector<int>>> *newTable = new std::vector<std::vector<std::vector<int>>>(this->N*2, std::vector<std::vector<int>>(this->N*2, std::vector<int>(PROBE_SIZE, 0)));
    }



    void acquire(int x)
    {
        this->locks[0][hash1(x)].lock();
        this->locks[1][hash2(x)].lock();
    }

    void release(int x)
    {
        this->locks[0][hash1(x)].unlock();
        this->locks[1][hash2(x)].unlock();
    }

    int hash1(int x)
    {
        return x % this->N;
    }

    int hash2(int x)
    {
        return 1 + (x % (this->N - 1));
    }
    
    int randomizer(int max)
    {
        static thread_local std::mt19937 generator;
        std::uniform_int_distribution<int> distribution(0, max);
        return distribution(generator);
    }
};
