#include <iostream>
#include <cstdio>
#include <vector>
using namespace std;


struct Node {
    int type; 
    // -2 represents structure node
    // -1 represents the leftover
    // positive integer represents the item type
    // the leaves are either -1 or positive integer 
    Node* parent;
    vector<Node*> children;

    Node() {}
    Node(const Node &x) {type = x.type; parent = NULL;children.clear();}
};

Node* clone(Node *x)
{
    Node *new_x = new Node(*x);
    for(auto child : x->children)
    {
        Node *new_x_child = clone(child);
        new_x_child->parent = new_x;
        new_x->children.push_back(new_x_child);
    }
    return new_x;
}

void function(Node* x, vector<Node*> &spaces)
{
    spaces.push_back(x);
}
int main ()
{
    vector<Node*> patterns;
    Node* x1 = new Node;
    Node* x2 = new Node;
    Node* x3 = new Node;
    x1->type = 1;
    x2->type = 2;
    x3->type = 3;
    function(x1, patterns);
    cout<<patterns[0]->type<<endl;
}