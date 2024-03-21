template<class T>
struct node {
    T data;
    node<T> next*;
}

template<class T> class List
{
    node<T> starting node;

    virtual bool add(T item);

    virtual bool remove(T item);

    virtual bool contains(T item);
}

