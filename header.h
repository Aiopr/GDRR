#pragma once 

#include <iostream>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <vector>
#include <queue>

using namespace std;

/****************************************************************************************/
#pragma region Instance

struct Rectangle {
    int w;
    int h;
    
    Rectangle(int _w, int _h) : w(_w), h(_h) {}
    Rectangle() : w(0), h(0) { }
    int area() const { return w * h; }

    bool able_insert(Rectangle r) const { return w >= r.w && h >= r.h; } 
    //Judge if one rectangle r can be inserted into another rectangle

    bool operator<(const Rectangle& r) const { return area() < r.area(); }
};

struct Item {
    int type; // items with the same type has the same dimensions
    Rectangle rec; 
    int index; 

    Item() : type(0), rec(Rectangle(0, 0)), index(0) {}
    Item(int _type, Rectangle _rec, int _index) : type(_type), rec(_rec), index(_index) {}
    int width(bool rotate)  const { return (!rotate) ? rec.w : rec.h; }
    int height(bool rotate) const { return (!rotate) ? rec.h : rec.w; }
    Rectangle rectangle(bool rotate) const { return Rectangle(width(rotate), height(rotate)); }
};



void build_instance(const char* filename);

#pragma endregion
/****************************************************************************************/



/****************************************************************************************/

#pragma region Solution

struct Node {
    int type; 
    // -2 represents structure node
    // -1 represents the leftover
    // positive integer represents the item type
    // the leaves are either -1 or positive integer 
    Rectangle rec;
    Node *parent;
    vector<Node*> children;
    bool next_cut_orientation; 
    // true if horizontal, false if vertical

    Node() { parent = NULL; type = -1; next_cut_orientation = 0; }
    Node(const Node &x) {type = x.type; rec = x.rec; parent = NULL; children.clear(); next_cut_orientation = x.next_cut_orientation;}

    int width()     const { return rec.w; }
    int height()    const { return rec.h; }
    int area()      const { return rec.area(); }
};

Node* clone(Node *x) // deep copy x
{
    Node *new_x = new Node(*x);
    for(auto child : x->children)
    {
        Node* new_x_child = clone(child);
        new_x_child->parent = new_x;
        new_x->children.push_back(new_x_child);
    }
    return new_x;
}

struct Solution {
    vector<Node*> patterns; 
    vector<Item> excluded_items;
    int total_area;

        
    const int bin_number() const { return patterns.size(); }

    const int excluded_area()
    {
        int area = 0;
        for(auto item : excluded_items)
        {
            area += item.rec.area();
        }
        return area;
    }

    const int included_area()
    {
        return total_area - excluded_area();
    }

    void add_new_bin(Rectangle bin_size)
    {
        Node *bin = new Node;
        bin->parent = new Node;
        bin->parent->type = -2;
        bin->rec = bin_size; 
        bin->rec = bin_size;
        patterns.push_back(bin);
    }
    
    Solution deep_copy()
    {
        Solution s; 
        for(auto pattern : patterns)
        {
            s.patterns.push_back(clone(pattern));
        }
        s.excluded_items = excluded_items;
        s.total_area = total_area;
        return s;
    }
};


struct Insertion {
    int space_id;
    int item_id;
    double cost;
    bool rotate;

    Insertion(int _space_id, int _item_id, double _cost, bool _rotate) : space_id(_space_id), item_id(_item_id), cost(_cost), rotate(_rotate) {}
    bool operator<(const Insertion &other) const { return cost < other.cost ;} 
    bool operator>(const Insertion &other) const { return cost > other.cost ;} 
};


Solution recreate(Solution S, int mat_limit);

double vertical_cost(Node *space, Item item, bool rotation, double power);

double horizontal_cost(Node *space, Item item, bool rotation, double power);

void extract_spaces(Node *root, vector<Node*> &spaces);

void insert(Node *space, Item item, bool rotate);

inline int Rand(int st, int ed) // return a random number ranging from 0 to x;
{
    return st + (int) ( ed * rand() / RAND_MAX);
}


#pragma endregion

/****************************************************************************************/




/****************************************************************************************/
#pragma region Print

struct Coord {
    int x;
    int y;

    Coord(int x, int y) : x(x), y(y) {}
};

struct Layout {
    int index; 
    int bin;
    Coord o;
    Rectangle rec;
};

void print_item(Node *node, Coord o, int bin);
void print_leftover(Node *node, Coord o, int bin);
void print_pattern(vector<Node*> patterns);

void print_solution(Solution best_solution);
#pragma endregion
/****************************************************************************************/