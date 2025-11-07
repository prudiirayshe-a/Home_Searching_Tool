#include "R&B.h"
#include <stdexcept>
#include <vector>
//constructor and copy constructor w/helper
template <typename T>
red_b<T>::red_b(){
    root = nullptr;
}
template <typename T>
red_b<T>::red_b(T a){
    insert(a);
}
//helper
template <typename T>
typename red_b<T>::Node* red_b<T>::copy_helper(Node* source, Node* parent){
    if(source == nullptr){
        return nullptr;
    }
    //create the new node
    Node* new_node = new Node*(source->data);
    //assign all the internal data
    new_node->is_red = source->is_red;
    new_node->parent = parent;
    new_node->lc = copy_helper(source->lc, new_node);
    new_node->rc = copy_helper(source->rc, new_node);
    return new_node;
}
//actual copy constructor
template <typename T>
red_b<T>::red_b(const red_b<T>& original){
    root = copy_helper(original.root, nullptr);
    tree_size = original.tree_size;
}
template <typename T>
bool red_b<T>::empty(){
    return (root==nullptr);
}
template <typename T>
bool red_b<T>::insert(T a){
    if(root==nullptr){
        root = new Node(a);
        root->is_red = false;
        return true;
    }
    Node* curr = root;
    Node* lagging = nullptr;
    while(curr !=nullptr){
        if(a < curr->data){
            lagging = curr;
            curr = curr->lc;
            continue;
        }
        lagging = curr;
        curr = curr->rc;
        continue;
    }
    //lagging will be the parent node which must be checked current will be a nullptr
    //we now add the new node into the tree
    Node* child;
    if(lagging->data > a){
        lagging->lc = new Node(a);
        child = lagging->lc;
    }else{
        lagging->rc = new Node(a);
        child = lagging->rc;
    }
    child->parent = lagging;
    tree_size++;
    if(lagging->is_red == false){
        return true; 
    }else{
        recoloring_rotation(child);
    }
    return false;
};

template <typename T>
bool red_b<T>::recoloring_rotation(Node* N){
    if(N->parent == nullptr && N->is_red == true){
        N->is_red = false;
        return true;
    }else if(N->parent->is_red == false){
        return true;
    }
    //tracks is the node is a left or right child; and sets uncle
    Node* uncle;
    if(N->parent->lc == N){
        uncle = N->parent->rc;
    }else{
        uncle = N->parent->lc;
    }
    if(uncle == nullptr || uncle->is_red == false){
        return rotation(N);
    }else{
        uncle->is_red =  false;
        N->parent->is_red = false;
        if(N->parent->parent != root){
            N->parent->parent->is_red = !N->parent->parent->is_red;
            return recoloring_rotation(N->parent->parent);
        }else{
            return true;
        }
    }

};

template <typename T>
bool red_b<T>::rotation(Node* N){
    Node* former_grand = N->parent->parent;
    Node* former_parent = N->parent;

    //LL case
    if(N == N->parent->lc && N->parent == N->parent->parent-> lc){
        rotate_right(N->parent, N->parent->parent);
        former_grand->is_red = !former_grand->is_red;
        former_parent->is_red = !former_parent->is_red;
        return true;

        //Next is RR case
    }else if(N == N->parent->rc && N->parent == N->parent->parent-> rc){
        rotate_left(N->parent, N->parent->parent);
        former_grand->is_red = !former_grand->is_red;
        former_parent->is_red = !former_parent->is_red;
        return true;

        //RL case
    }else if(N == N->parent->lc && N->parent == N->parent->parent-> rc){
        rotate_right(N, N->parent);
        rotate_left(N, N->parent->parent);
        former_grand->is_red = !former_grand->is_red;
        N->is_red = !N->is_red;
        return true;

        //Finally the LR case
    }else if(N == N->parent->rc && N->parent == N->parent->parent-> lc){
        rotate_left(N, N->parent);
        rotate_right(N, N->parent->parent);
        former_grand->is_red = !former_grand->is_red;
        N->is_red = !N->is_red;
        return true;
    }
    return false;
};

//left rotation function definition
template <typename T>
void red_b<T>::rotate_left(Node* right_node, Node* former_parent){
    if(right_node == nullptr || former_parent == nullptr){
        return;
    }

    //perform rotation in terms of child nodes
    Node* temp = right_node->lc;
    right_node->lc = former_parent;
    former_parent->rc = temp;

    //reassign parents
    if(temp != nullptr){
        temp->parent = former_parent;
    };
    right_node->parent = former_parent->parent;
    former_parent->parent = right_node;

    //check if root needs to be reassigned
    if(right_node->parent == nullptr){
        root =  right_node;
    }else if(right_node->parent->lc == former_parent){
        right_node->parent->lc = right_node;
    }else{
        right_node->parent->rc = right_node;
    }

};

//Right rotation function definition
template <typename T>
void red_b<T>::rotate_right(Node* left_node, Node* former_parent){
    if(left_node == nullptr || former_parent == nullptr){
        return;
    }

    //perform rotation operations, particularly child pointers
    Node* temp = left_node->rc;
    left_node->rc = former_parent;
    former_parent->lc = temp;

    //reassign parents
    if(temp != nullptr){
        temp->parent = former_parent;
    };
    left_node->parent = former_parent->parent;
    former_parent->parent = left_node;

    //check if root needs to be reassigned
    if(left_node->parent == nullptr){
        root =  left_node;
    }else if(left_node->parent->lc == former_parent){
        left_node->parent->lc = left_node;
    }else{
        left_node->parent->rc = left_node;
    };

};

//Deletion functions && helper
template <typename T>
bool red_b<T>::remove(Node* N){
    if(N == nullptr){
        return false;
    }
    //case of no children
    if(N->lc == nullptr && N->rc == nullptr){
        //root case
        if(N->parent == nullptr){
            root = nullptr;
            delete N;
            tree_size = 0;
            return true;
        }
        
        //nonroot case
        
        Node* exparent = N->parent;
        if(N->is_red){
            if(exparent->lc == N){
                exparent->lc = nullptr;
                delete N;
                tree_size--;
                return true;
            }else{
                exparent->rc = nullptr;
                delete N;
                tree_size--;
                return true;
            }
        }
        //case of 1 child:
    }else if(N->lc == nullptr && !(N->rc == nullptr)){
        //root case
        if(root ==  N){
            root = N->rc;
            root->is_red = false;
            delete N;
            tree_size--;
            return true;
        }

        //non root case
        Node* exparent = N->parent;
        Node* xkid = N->rc;
        if(N->is_red || xkid->is_red){
            if(exparent->lc == N){
                exparent->lc = xkid;
                xkid->is_red = false;
                delete N;
                tree_size--;
                return true;
            }else{
                exparent->rc = xkid;
                xkid->is_red = false;
                delete N;
                tree_size--;
                return true;
            }
        }
        
        //left child cases
    }else if(!(N->lc == nullptr) && N->rc == nullptr){
        //root case
        if(root ==  N){
            root = N->lc;
            root->is_red = false;
            delete N;
            tree_size--;
            return true;
        }

        //non root case
        Node* exparent = N->parent;
        Node* xkid = N->lc;
        if(N->is_red || xkid->is_red){
            if(exparent->lc == N){
                exparent->lc = xkid;
                xkid->is_red = false;
                delete N;
                tree_size--;
                return true;
            }else{
                exparent->rc = xkid;
                xkid->is_red = false;
                delete N;
                tree_size--;
                return true;
            }  
        }  
        //two child case, will have to add the relevant logic here by the end of the night
    }else if(!(N->lc == nullptr) && !(N->rc == nullptr)){
        Node* successor = N->rc;
        while(successor->lc != nullptr){
            successor=successor->lc;
        }
        N->data = successor->data;
        remove(successor);
    }
    //We only reach here in the double, non-root, black case, to resolve the double black case we must do the following
    Node* exparent =N->parent;
    Node* exsibling;
    Node* exchild;
    if(N==exparent->lc){
        exsibling = exparent->rc;
    }else{
        exsibling = exparent->lc;
    }
    if(N->lc != nullptr){
        exchild = N->lc;
    }else{
        exchild = N->rc;
    }
    //now we check if sibling is black or red and react accordingly after performing the deletion.

    if(exsibling->is_red){
        exparent->is_red = true;
        exsibling->is_red = false;
        if(exsibling == exparent->lc){
            exparent->rc=exchild;
            rotate_right(exsibling, exparent);
            delete N;
            tree_size--;
            return true;
        }else{
            exparent->lc = exchild;
            rotate_left(exsibling, exparent);
            delete N;
            tree_size--; 
            return true;
        }
    }else{
        if(exchild == nullptr){
            if(exparent->lc == N){
                exparent->lc = nullptr;
            }else{
                exparent->rc = nullptr;
            }
            double_B_helper(exsibling, exparent);
        }
    }
}
template <typename T>
void red_b<T>::double_B_helper(Node* N){
    if(N->parent == nullptr){
        N->is_red = false;
        return;
    }else if(N->is_red == true){
        N->is_red = false;
    }
    // ex parent, ex sibling, and x kid
    Node* xpar = N->parent;
    Node* xsib;
    Node* xkid;
    if(N->lc != nullptr){
        xkid = N->lc;
    }else{
        xkid = N->rc;
    }
    if(xpar->lc == N){
        xsib= xpar->rc;
    }else{
        xsib=xpar->lc;
    }
    //move up to parent to fix
    if(xsib == nullptr){
        double_B_helper(xpar);
    }
    //true == red false ==black;
    //ex sibling left child and ex sibling right child is_red are stored in these for easier refference
    bool xsiblc = true;
    bool xsibrc = true;
    //set false if conditions are not met, ensure we don't call functions of nullptr
    if(xsib->rc == nullptr || xsib->rc->is_red == false){
        xsibrc = false;
    }
    if(xsib->lc == nullptr || xsib->lc->is_red == false){
        xsiblc = false;
    }
    //RR case (if sibling is right and the rc of sib is red)
    if(xsib==xpar->rc && xsibrc ==true){
        rotate_left(xsib,xpar);
        xsib->rc->is_red = false;
        xsib->lc->is_red=false;
        //LL case
    }else if(xsib==xpar->lc && xsiblc ==true){
        rotate_right(xsib,xpar);
        xsib->rc->is_red = false;
        xsib->lc->is_red=false;
        //Rl case
    }else if(xsib==xpar->rc && xsiblc ==true){
        Node* temp = xsib->lc;
        rotate_right(temp, xsib);
        rotate_left(temp, xpar);
        temp->is_red = false;
        temp->lc->is_red = false;
        temp->rc->is_red=false;
        //LR case
    }else if(xsib==xpar->lc && xsibrc ==true){
        Node* temp = xsib->rc;
        rotate_left(temp, xsib);
        rotate_right(temp, xpar);
        temp->is_red = false;
        temp->lc->is_red = false;
        temp->rc->is_red=false;
    }else if(xsib->is_red == false && xsiblc == false && xsibrc == false){
        xsib->is_red = true;
        if(xpar->is_red==false){
            double_B_helper(xpar);}
        else{
            xpar->is_red = false;
        }
    }
}
//overload for the case of N having no children, pass in the 
template <typename T>
void red_b<T>::double_B_helper(Node* exsibling, Node* expar){
    if(expar == nullptr){
        return;
    }
    // ex parent, ex sibling, and x kid
    Node* xpar = expar;
    Node* xsib = exsibling;
    if(xpar->lc == nullptr){
        xsib= xpar->rc;
    }else{
        xsib=xpar->lc;
    }
    //move up to parent to fix
    if(xsib == nullptr){
        double_B_helper(xpar);
    }
    //true == red false ==black;
    //ex sibling left child and ex sibling right child is_red are stored in these for easier refference
    bool xsiblc = true;
    bool xsibrc = true;
    //set false if conditions are not met, ensure we don't call functions of nullptr
    if(xsib->rc == nullptr || xsib->rc->is_red == false){
        xsibrc = false;
    }
    if(xsib->lc == nullptr || xsib->lc->is_red == false){
        xsiblc = false;
    }
    //RR case (if sibling is right and the rc of sib is red)
    if(xsib==xpar->rc && xsibrc ==true){
        rotate_left(xsib,xpar);
        xsib->rc->is_red = false;
        xsib->lc->is_red=false;
        //LL case
    }else if(xsib==xpar->lc && xsiblc ==true){
        rotate_right(xsib,xpar);
        xsib->rc->is_red = false;
        xsib->lc->is_red=false;
        //Rl case
    }else if(xsib==xpar->rc && xsiblc ==true){
        Node* temp = xsib->lc;
        rotate_right(temp, xsib);
        rotate_left(temp, xpar);
        temp->is_red = false;
        temp->lc->is_red = false;
        temp->rc->is_red=false;
        //LR case
    }else if(xsib==xpar->lc && xsibrc ==true){
        Node* temp = xsib->rc;
        rotate_left(temp, xsib);
        rotate_right(temp, xpar);
        temp->is_red = false;
        temp->lc->is_red = false;
        temp->rc->is_red=false;
    }else if(xsib->is_red == false && xsiblc == false && xsibrc == false){
        xsib->is_red = true;
        if(xpar->is_red==false){
            double_B_helper(xpar);}
        else{
            xpar->is_red = false;
        }
    }
}

template <typename T>
bool red_b<T>::erase(int pos){
    if(pos >= tree_size || pos<0 ){
        return false;
    }
    int counter= 0;
    //Morris Traversal Implementation.
    // I wanted a non recursive solution which used as little space as possible
    //https://www.geeksforgeeks.org/dsa/inorder-tree-traversal-without-recursion-and-without-stack
    // The Morris Travel algorithm which I found at the above uses O(1) space and meets that criteria.
    Node* current_node = root;
    while(current_node != nullptr){
        if(counter==pos){
            return remove(current_node);
        }
        if(current_node->lc == nullptr){
            //this handles the case of no left child and moves to the right child
            current_node = current_node->rc;
        }else{
            //This handles the case of a left child
            Node* in_order_predecessor = current_node->lc;
            while(in_order_predecessor->rc != nullptr && in_order_predecessor->rc != current_node){
                in_order_predecessor = in_order_predecessor->rc;
            }
            if(in_order_predecessor->rc == nullptr){
                in_order_predecessor->rc = current_node;
                current_node = current_node->lc;
            }else{
                in_order_predecessor->rc = nullptr;
                current_node = current_node->rc;
            }
            }
        counter++;
    }
    return false;
}
// data based removal, removes only the node with the given data
template <typename T>
bool red_b<T>::erase(T a){
    //Morris Traversal Implementation.
    // I wanted a non recursive solution which used as little space as possible
    //https://www.geeksforgeeks.org/dsa/inorder-tree-traversal-without-recursion-and-without-stack
    // The Morris Travel algorithm which I found at the above uses O(1) space and meets that criteria.
    Node* current_node = root;
    while(current_node != nullptr){
        if(current_node->data==a){
            return remove(current_node);
        }
        if(current_node->lc == nullptr){
            //this handles the case of no left child and moves to the right child
            current_node = current_node->rc;
        }else{
            //This handles the case of a left child
            Node* in_order_predecessor = current_node->lc;
            while(in_order_predecessor->rc != nullptr && in_order_predecessor->rc != current_node){
                in_order_predecessor = in_order_predecessor->rc;
            }
            if(in_order_predecessor->rc == nullptr){
                in_order_predecessor->rc = current_node;
                current_node = current_node->lc;
            }else{
                in_order_predecessor->rc = nullptr;
                current_node = current_node->rc;
            }
            }
    }
    return false;
}
//range based removal
template <typename T>
bool red_b<T>::erase(int first, int last){
    if(first<0 || last > tree_size-1){
        return false;
    }
    int counter= 0;
    //Morris Traversal Implementation.
    // I wanted a non recursive solution which used as little space as possible
    //https://www.geeksforgeeks.org/dsa/inorder-tree-traversal-without-recursion-and-without-stack
    // The Morris Travel algorithm which I found at the above uses O(1) space and meets that criteria.
    Node* current_node = root;
    while(current_node != nullptr){
        if(counter>=first && counter<=last){
            bool failcheck = remove(current_node);
            if(failcheck == false){
                return  false;
            }
        }
        if(current_node->lc == nullptr){
            //this handles the case of no left child and moves to the right child
            current_node = current_node->rc;
        }else{
            //This handles the case of a left child
            Node* in_order_predecessor = current_node->lc;
            while(in_order_predecessor->rc != nullptr && in_order_predecessor->rc != current_node){
                in_order_predecessor = in_order_predecessor->rc;
            }
            if(in_order_predecessor->rc == nullptr){
                in_order_predecessor->rc = current_node;
                current_node = current_node->lc;
            }else{
                in_order_predecessor->rc = nullptr;
                current_node = current_node->rc;
            }
            }
        counter++;
    }
    return true;
}
//insertion
template <typename T>
T red_b<T>::at(int a){
    if(a>= tree_size || a<0){
        throw std::out_of_range("Index is not within valid range");
    }
    int counter= 0;
    //Morris Traversal Implementation.
    // I wanted a non recursive solution which used as little space as possible
    //https://www.geeksforgeeks.org/dsa/inorder-tree-traversal-without-recursion-and-without-stack
    // The Morris Travel algorithm which I found at the above uses O(1) space and meets that criteria.
    Node* current_node = root;
    while(current_node != nullptr){
        if(counter==a){
            return current_node->data;
        }
        if(current_node->lc == nullptr){
            //this handles the case of no left child and moves to the right child
            current_node = current_node->rc;
        }else{
            //This handles the case of a left child
            Node* in_order_predecessor = current_node->lc;
            while(in_order_predecessor->rc != nullptr && in_order_predecessor->rc != current_node){
                in_order_predecessor = in_order_predecessor->rc;
            }
            if(in_order_predecessor->rc == nullptr){
                in_order_predecessor->rc = current_node;
                current_node = current_node->lc;
            }else{
                in_order_predecessor->rc = nullptr;
                current_node = current_node->rc;
            }
            }
        counter++;
    }
    //for the compiler
    throw std::out_of_range("Index is not within valid range");
};
//Operator Overloads
template <typename T>
std::vector<T> red_b<T>::operator[](int a){
    if(a>tree_size || a<0){
        throw std::out_of_range("Tree is not large enough");
    }
    std::vector<T> output;
    if(a==0){
        return output;
    }
    int counter= 0;
    //Morris Traversal Implementation.
    // I wanted a non recursive solution which used as little space as possible
    //It is reversed to allow the pulling of the indexes with the a highest scores
    //https://www.geeksforgeeks.org/dsa/inorder-tree-traversal-without-recursion-and-without-stack
    // The Morris Travel algorithm which I found at the above uses O(1) space and meets that criteria.
    Node* current_node = root;
    while(current_node != nullptr){
        if(current_node->rc == nullptr){
            if(counter<a){
                output.push_back(current_node->data);
            }
            counter++;
            //this handles the case of no right child and moves to the left child
            current_node = current_node->lc;
            
        }else if(current_node->rc!= nullptr){
            //This handles the case of a right child
            Node* in_order_successor = current_node->rc;
            while(in_order_successor->lc != nullptr && in_order_successor->lc != current_node){
                in_order_successor = in_order_successor->lc;
            }
            if(in_order_successor->lc == nullptr){
                in_order_successor->lc = current_node;
                current_node = current_node->rc;
            }else{
                
                in_order_successor->lc = nullptr;
                if(counter<a){
                    output.push_back(current_node->data);
                }
                counter++;
                current_node = current_node->lc;
            }
            }
    }
    return output;
};
//Destructor and Destructor Helper
//helper
template <typename T>
void red_b<T>::delete_subtree(Node* n){
    if(n == nullptr){
        return;
    }
    delete_subtree(n->lc);
    delete_subtree(n->rc);
    delete n;
}
//Destructor
template <typename T>
red_b<T>::~red_b(){
    delete_subtree(root);
}







