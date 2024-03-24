#include <iostream>
#include "list-sequential.cpp"

int main()
{
    SequentialList *list = new SequentialList();

    list->populate(1000);
    //list->print();
    return 0;
}