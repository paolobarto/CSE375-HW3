

#ifndef LIST_H
#define LIST_H

template<class T> class List
{
    public:
        int N;
        // [[table1,[table2]]

        virtual bool contains(T item);

        virtual bool add(T item);

        virtual bool remove(T item);

        virtual int size();

        virtual void populate(int x);

        virtual void print();

        virtual void resize();
};

#endif