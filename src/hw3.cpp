#include <iostream>
#include "list-sequential.cpp"
#include "list-parallel.cpp"
#include <chrono>
#include <random>
#include <thread>
//#include "list.cpp"
int VALUES = 100000;
int OPERATIONS = 10000;
int THREADS = 4;
int profile(List<int>* list, int operations);
int randomizer(int max);

int main()
{
    // include comparison on normal vector


    cout<<"Value limit: "<<VALUES<<"\n";
    cout<<"Operations: "<<OPERATIONS<<"\n";

    // SequentialList *seqList = new SequentialList();
    // cout<<"SequentialList\n";
    // seqList->populate(VALUES);
    // profile(seqList, OPERATIONS);

    ParallelList *paraList = new ParallelList();
    cout<<"ParallelList\n";
    //paraList->populate(VALUES);
    // cout<<"Populated\n";
    std::vector<std::thread> threads;
    std::atomic<int> total_time(0); 

    for(int i = 0; i < THREADS; i++)
        threads.emplace_back(std::thread([&](){
            total_time += profile(paraList, OPERATIONS/THREADS);
            //paraList->populate(VALUES);
        }));
    
    for (auto& thread : threads)
        thread.join();

    // cout<<"Total time: "<<total_time<<"\n";

    return 0;
}

int profile(List<int>* list, int operations)
{
    auto started = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < operations; i++)
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
    time_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - started).count();
    cout<<elapsed<<"\n";
    return elapsed;
}

int randomizer(int max)
{
    static thread_local std::mt19937 generator;
    std::uniform_int_distribution<int> distribution(0, max);
    return distribution(generator);
}