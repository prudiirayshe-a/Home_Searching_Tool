#pragma once
template <typename T>
class red_b{
    struct Node{
        Node* parent = nullptr;
        Node* lc = nullptr; //Left Child
        Node* rc = nullptr; //Right Child
        bool is_red = true;
        T data;
        //Template constructor given data
        Node(T input_1){
            data = input_1;
        };
        Node(Node A){
            parent = A.parent;
            lc= A.lc;
            rc = A.rc;
            is_red = A.is_red;
            data = A.data;
        }
        Node operator=(Node A){
            parent = A.parent;
            lc = A.lc;
            rc = A.rc;
            is_red = A.is_red;
            data = A.data;
        }
        ~Node(){};
    };
    void rotate_left(Node* right_node, Node* former_parent);
    void rotate_right(Node* left_node, Node* former_parent);
    int tree_size = 0;
    Node* root = nullptr;
    public:
    red_b();
    red_b(T a);
    Node* copy_helper(Node* source, Node* parent);
    red_b(const red_b<T>& original);
    void double_B_helper(Node* N);
    void double_B_helper(Node* exsibling, Node* expar);
    bool empty();
    bool insert(T a);
    bool remove(Node* N);
    bool erase(int pos);
    bool erase(T a);
    bool erase(int first, int last);
    bool recoloring_rotation(Node* parent);
    bool rotation(Node* N);
    T operator[](int a);
    T at(int a);
    void delete_subtree(Node* n);
    ~red_b();
};