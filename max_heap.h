#include <stdexcept>
#include <algorithm>
//Dynamic array for internals of our heap, had to make sure no  includes were used unecessarily
template <typename T>
struct dynam_array{
    int size=0;
    int capacity;
    T* arr = nullptr;
    dynam_array(int a){
        capacity = a;
        arr = new T[capacity];
    };
    dynam_array(const dynam_array& other){
        capacity = other.capacity;
        size = other.size;
        arr = new T[capacity];
        for(int i=0; i<size; i++){
            arr[i] = other.arr[i];
        };
    };
    dynam_array(){
        capacity = 128;
        arr = new T[capacity];
    };
    bool empty(){
        return(size==0);
    }
    //Dynamic array internals
    void dynam_grow(){
        capacity = capacity*2;
        T* temp = new T[capacity];
        for(int i=0; i<size; i++){
            temp[i] = arr[i];
        }
        delete[] arr;
        arr= temp;
    };
    void dynam_shrink(){
        capacity = capacity/2;
        T* temp = new T[capacity];
        for(int i=0; i<size; i++){
            temp[i] = arr[i];
        }
        delete[] arr;
        arr= temp;
    };
    //add new data to final position (capacity is always larger than what we consider size in short term)
    void push_back(T b){
        if(size == capacity){
            dynam_grow();
        }
        arr[size] =b;
        size++;
        return;
    }
    T& operator[](int i){
        if(i >= size || i<0){
            throw std::out_of_range("Index is not within valid range");
        }
        return arr[i];
    }
    T operator[](int i) const{
        if(i >= size || i<0){
            throw std::out_of_range("Index is not within valid range");
        }
        return arr[i];
    }
    void swap(int a, int b){
        std::swap(arr[a], arr[b]);
    }
    //assignment operator
    dynam_array& operator=(const dynam_array& other){
        if(&other == this){
            return *this;
        }
        delete[] arr;
        capacity = other.capacity;
        size = other.size;
        arr = new T[capacity];
        for(int i=0; i<size; i++){
            arr[i] = other.arr[i];
        };
        return *this;
    };
    ~dynam_array(){
        delete[] arr;
    }
    void decrement_size(){
        this->size--;
        return;
    }
};

template <typename T>
class max_heap{
    dynam_array<T>* internal_arr;
    public:
    //default and parametrized constructor
    max_heap(){
        this->internal_arr = new dynam_array<T>();
    }
    max_heap(int a){
        this->internal_arr = new dynam_array<T>(a);
    }
    //Create copy cosntructor
    max_heap(const max_heap<T>& other){
        this->internal_arr = new dynam_array<T>(*(other.internal_arr));
    }
    //recursive heapify  function, continues until it reaches a leaf node
    void heapify(int i){
        int max = i;
        int left_index = 2*i+1;
        int right_index = 2*i+2;
        if(!(2*i+1 <internal_arr->size)|| !(2*i+2 <internal_arr->size)){
            return;
        }
        if((*internal_arr)[max]<(*internal_arr)[left_index]){
            max = left_index;
        }
        if((*internal_arr)[max]<(*internal_arr)[right_index]){
            max = right_index;
        }
        if(max != i){
             internal_arr->swap(i,max);
            heapify(max);
        }
    }
    //return the top value;
    T top(){
        if(internal_arr->size<1){
            throw std::out_of_range("Heap is empty");
        }
        return (*internal_arr)[0];
    }
    //return and delete top value
    T pop(){
        if(internal_arr->size<1){
            throw std::out_of_range("Heap is empty");
        }
        T temp = (*internal_arr)[0];
        internal_arr->swap(0,internal_arr->size-1);
        internal_arr->decrement_size();
        heapify(0);
        return temp;
    }
    // push after those heap should be done
    void push(T a){
        internal_arr->push_back(a);
        return;
    }
    //Assignment operator
    max_heap& operator=(const max_heap<T>& other){
        if(&other == this){
            return *this;
        }
        delete this->internal_arr;
        this->internal_arr = new dynam_array<T>(*(other.internal_arr));
        return *this;
    }
    ~max_heap(){
        delete internal_arr;
    }
};