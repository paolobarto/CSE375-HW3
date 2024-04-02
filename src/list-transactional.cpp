// Using example of sequential list from 13.1
#include "list.h"
#include <cstddef>
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;
using std::vector;
class TransactionalList: public List<int>
{
    public:
    int LIMIT=40;
    TransactionalList(){
        this->N = 2;
        this->table = vector<vector<int>>(2, vector<int>(2));
    }

        std::vector<std::vector<int>> table;
    // Size per each table should be equal to N=2k
        // Since sequential do not need locks
        bool contains(int x) override
        {
            int index1 = hash1(x);
            int index2 = hash2(x);
            if(this->table[0][index1] == x || this->table[1][index2] == x)
                return true;
            return false;
        }

        bool add(int x) override
        {
            // cout<<"Adding "<<x<<endl;
            int index1 = hash1(x);
            int index2 = hash2(x);
            if(contains(x))
                return false;
            for(int i=0; i<LIMIT;i++){
                if((x = swap(0, 0, x)) == 0)
                    return true;
                if((x = swap(1, 1, x)) == 0)
                    return true;
            }
            // If both tables are full. Resize
            resize();
            return this->add(x);
        }

        bool remove(int x) override
        {
            int index1 = hash1(x);
            int index2 = hash2(x);
            if(this->table[0][index1] == x)
            {
                this->table[0][index1] = 0;
                return true;
            }
            if(this->table[1][index2] == x)
            {
                this->table[1][index2] = 0;
                return true;
            }
            return false;
        }

        int size() override
        {
            int count = 0;
            for(int i=0; i<2; i++)
            {
                for(int j=0; j<this->N; j++)
                {
                    if(this->table[i][j] != 0)
                        count++;
                }
            }
            return count;
        }

        void populate(int x) override
        {
            for(int i=0;i<x;i++)
            {
                this->add(rand()%x+1);
            }
            
        }

        int swap(int tableIndex, int hashFunc, int x)
        {
            int index = 0;
            if(hashFunc == 0)
                index = hash1(x);
            else
                index = hash2(x);
            int temp = this->table[tableIndex][index];
            this->table[tableIndex][index] = x;
            return temp;
        }

        void resize() override
        {
            // Resize the table to 2N
            //int new_table[2][2*this->N];
            
            vector<vector<int>>* new_table = new vector<vector<int>>(2, vector<int>(2*this->N));
            for(int i=0; i<this->N; i++)
            {
                (*new_table)[0][i] = this->table[0][i];
                (*new_table)[1][i] = this->table[1][i];
            }
            this->table = (*new_table);
            delete new_table;
            this->N = 2*this->N;
            //cout<<"resized to: "<<this->N<<endl;
        }

        void print() override
        {
        //printing of 2D array
            for(int i=0; i<2; i++)
            {
                cout << "Table "<<i<<": ";
                for(int j=0; j<this->N; j++)
                {
                    std::cout << this->table[i][j] << " ";
                }
                std::cout << std::endl;
            }
        }

        int hash1(int key) {
            const double A = 0.6180339887; // Fractional part of (√5 - 1) / 2
            return static_cast<int>(std::floor(this->N * std::fmod(key * A, 1)));
        }

        int hash2(int key) {
            const double A = 0.6180339887; // Fractional part of (√5 - 1) / 2
            return static_cast<int>((this->N - 1) - std::floor(this->N * std::fmod(key * A, 1)));
        }
};
