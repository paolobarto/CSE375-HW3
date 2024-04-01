#include <iostream>
#include "list-sequential.cpp"
#include "list-parallel.cpp"
#include <chrono>
#include <random>
//#include "list.cpp"
int VALUES = 1000;
int OPERATIONS = 10;
void profile(List<int>* list);
int randomizer(int max);

int main()
{
    // include comparison on normal vector


    cout<<"Value limit: "<<VALUES<<"\n";
    cout<<"Operations: "<<OPERATIONS<<"\n";

    SequentialList *seqList = new SequentialList();
    cout<<"SequentialList\n";
    profile(seqList);

    ParallelList *paraList = new ParallelList();
    cout<<"ParallelList\n";
    profile(paraList);

    return 0;
}

void profile(List<int>* list)
{
    list->populate(VALUES);
    list->print();
    auto started = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < OPERATIONS; i++)
    {
        int chance = randomizer(10);
        if (chance==1)
        {
            list->remove(randomizer(VALUES));
        }
        else if (chance==2)
        {
            list->add(randomizer(VALUES));
        }
        else
        {
            list->contains(randomizer(VALUES));
        }
       
    }

    cout<<std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - started).count()<<"\n";
}

int randomizer(int max)
{
    static thread_local std::mt19937 generator;
    std::uniform_int_distribution<int> distribution(0, max);
    return distribution(generator);
}