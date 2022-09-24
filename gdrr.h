#pragma once

#include <iostream>
#include <fstream>
#include <algorithm>
#include <ctime>

using namespace std;

#pragma region Instance

struct Coord {
    int x; 
    int y;

    Coord(int x, int y) : x(x), y(y) {}
};

struct Rectangle {
    int w;
    int h;

    Rectangle(int _w, int _h) : w(_w), h(_h) {}
    Rectangle() : w(0), h(0) { }
    int area() const { return w * h; }
    bool AbleInsert(Rectangle r, bool Rotation)
    {
        if(Rotation)
            return  (w >= r.w && h >= r.h) || (w >= r.h&& h >= r.w);
        else
            return  (w >= r.w && h >= r.h);
    }

    bool operator<(const Rectangle& r) const { return area() < r.area(); }
};

struct Item {
    int type;
    Rectangle rec;
    int num;

    Item() : type(0), rec(Rectangle(0, 0)) {}
    Item(int _type, Rectangle _rec, int _num) : type(_type), rec(_rec), num(_num) {}
    int width(bool rotate) const { return (!rotate) ? rec.w : rec.h; }
    int height(bool rotate) const { return (!rotate)? rec.h : rec.w; }
};


void build_instance(const char* filename);

#pragma endregion



//----------------------------------------------------------------------------------------------------------------

// #pragma region solution

// struct Node {
//     int type; // 0 represents structure node, positive integer represents the item type, -1 represents the leftover
//     Rectangle rec;
//     Node *parent;
//     vector<Node*> children;
// };

// //void get_removable_children(Node *node, vector<Node*> &removable_children);

// //void remove_node(Node *node);

// struct Solution {
//     vector<Node*> patterns;
//     vector<Item> excluded_items;
//     int64_t total_area;

//     const int64_t excluded_area()
//     {
//         int64_t area = 0;
//         for(auto item : excluded_items)
//         {
//             area += item.rec.area();
//         }
//         return area;
//     }

//     const int64_t included_area()
//     {
//         return total_area - excluded_area();
//     }


// };

// #pragma endregion

// //----------------------------------------------------------------------------------------------------------------

// #pragma region ruin and recreate

// inline int Rand(int st, int ed) // return a random number ranging from 0 to x;
// {
//     return st + (int) ( ed * rand() / RAND_MAX);
// }

//Solution ruin(Solution S, int mat_limit, int mu);

//Solution recreate(Solution S, int mat_limit);

#pragma endregion
