#include <iostream>
#include "list-sequential.cpp"
#include "list-parallel.cpp"
#include "list-transactional.cpp"
#include <chrono>
#include <random>
#include <thread>
//#include "list.cpp"
int VALUES = 100000;
int OPERATIONS = 500000;
int THREADS = 8;
int profile(List<int>* list, int operations);
int randomizer(int max);

int main()
{
    // include comparison on normal vector


    cout<<"Value limit: "<<VALUES<<"\n";
    cout<<"Operations: "<<OPERATIONS<<"\n";

    // SequentialList *seqList = new SequentialList();
    // //cout<<"SequentialList\n";
    // seqList->populate(VALUES);
    // cout<<"Sequential populated with "<<seqList->size()<<" values"<<endl;
    // int s_time = profile(seqList, OPERATIONS);
    // cout<<"Sequential Time: "<<s_time<<endl;

    // ParallelList *paraList = new ParallelList();
    // cout<<"ParallelList\n";
    // //paraList->populate(VALUES);

    // paraList->populate_parallel(VALUES, THREADS);
    // cout<<"Populated with "<<paraList->size()<<" values"<<endl;
    // std::vector<std::thread> threads;
    // std::atomic<int> max_time(0); 


    // for(int i = 0; i < THREADS; i++)
    //     threads.emplace_back(std::thread([&](){
    //         int time = profile(paraList, OPERATIONS/THREADS);
    //         //paraList->populate(VALUES);
    //         int curr_max = max_time.load();
    //         while (time > curr_max && !max_time.compare_exchange_weak(curr_max, time))
    //         {
    //         }
    //     }));
    
    // for (auto& thread : threads)
    //     thread.join();
    // cout<<"Parallel time: "<<max_time<<"\n";

    TransactionalList *transList = new TransactionalList();
    transList->populate(VALUES);
    cout <<"Transactional populated with "<<transList->size()<<" values"<<endl;
    int t_time = profile(transList, OPERATIONS);
    cout<<"Transactional time"<< t_time<<endl;



    return 0;
}

int profile(List<int>* list, int operations)
{
    auto started = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < operations; i++)
    {
        // if(i%100000==0)
        //     cout<<"i: "<<i<<" size: "<<list->size()<<endl;

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
    //cout<<elapsed<<"\n";
    return elapsed;
}

int randomizer(int max)
{
    static thread_local std::mt19937 generator;
    std::uniform_int_distribution<int> distribution(0, max);
    return distribution(generator);
}
