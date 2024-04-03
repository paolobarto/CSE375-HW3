#include <iostream>
#include "list-sequential.cpp"
#include "list-parallel.cpp"
#include "list-transactional.cpp"
#include <chrono>
#include <random>
#include <thread>
//#include "list.cpp"
int profile(List<int>* list, int operations, int values);
int randomizer(int max);

int main()
{
    int values = 1000;
    int operations = 1000;
    int threadCount = 2;
    // include comparison on normal vector
    while(operations<500000){
        while(values<10000000){
            cout<<"Value limit: "<<values<<"\n";
            cout<<"Operations: "<<operations<<"\n";

            SequentialList *seqList = new SequentialList();
            //cout<<"SequentialList\n";
            seqList->populate(values);
            cout<<"Sequential populated with "<<seqList->size()<<" values"<<endl;
            int s_time = profile(seqList, operations, values);
            cout<<"Sequential Time: "<<s_time<<endl;

    //paraList->populate(values);


    while(threadCount<24){
        cout<<"Threads: "<<threadCount<<endl;
    ParallelList *paraList = new ParallelList();
    cout<<"ParallelList\n";
    paraList->populate_parallel(values, threadCount);
    cout<<"Populated with "<<paraList->size()<<" values"<<endl;
    std::vector<std::thread> threads;
    std::atomic<int> max_time(0); 
        for(int i = 0; i < threadCount; i++)
            threads.emplace_back(std::thread([&](){
                int time = profile(paraList, operations/threadCount, values);
                //paraList->populate(values);
                int curr_max = max_time.load();
                while (time > curr_max && !max_time.compare_exchange_weak(curr_max, time))
                {
                }
            }));
        
                for (auto& thread : threads)
                    thread.join();
                cout<<"Parallel time: "<<max_time<<"\n";
                threadCount=threadCount*2;
            }
                threadCount = 2;
                values=values*10;
        }   
                values = 100000;
                operations=operations*10;
    }

    // TransactionalList *transList = new TransactionalList();
    // transList->populate(values);
    // cout <<"Transactional populated with "<<transList->size()<<" values"<<endl;
    // int t_time = profile(transList, operations);
    // cout<<"Transactional time"<< t_time<<endl;



    return 0;
}

int profile(List<int>* list, int operations,int values)
{
    auto started = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < operations; i++)
    {
        // if(i%100000==0)
        //     cout<<"i: "<<i<<" size: "<<list->size()<<endl;

        int chance = randomizer(10);
        if (chance==1)
        {
            list->remove(randomizer(values));
        }
        else if (chance==2)
        {
            list->add(randomizer(values));
        }
        else
        {
            list->contains(randomizer(values));
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
