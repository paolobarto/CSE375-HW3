

template<class T>
struct node {
    T data;
    struct node<T> *next;
};

template<class T> class List
{
    public:
        //node<T> starting_node;
        int LIMIT; // Used to indicate when max shuffle is reached and resize is needed
        int N;
        // [[table1],[table2]]

    virtual bool contains(T item);

    virtual bool add(T item);

    virtual bool remove(T item);

    virtual int size();

    virtual void populate(int x);

    virtual void print();

    virtual void resize();
};

