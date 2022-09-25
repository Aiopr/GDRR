#include <header.h>
#include <cstdio>
#include <iomanip>
#include <map>
#include <cmath>
#include <climits>

using namespace std;

// Constants 
#define INF 1e9

// Parameters 
const double time_limit = 5000; // 5s 
const double evaluation_power = 1.2;


/************************************************************************/
#pragma region Instance 

Rectangle bin;
vector<Item> items;
map<Rectangle, int> type;
int total_area = 0;

void build_instance(const char *filename)
{
    ifstream infile(filename);
    int n;
    int j, w, h, d, b, p;
    int t = 0;
    Rectangle r;
    
    if(!infile.is_open()) cerr << "cannot open " << filename << endl;
    
    infile >> n >> bin.w >> bin.h;

    for (int i = 0; i < n; ++i)
    {
        infile >> j >> w >> h >> d >> b >> p; 
        if(w < h) r.w = h, r.h = w; 
        else r.w = w, r.h = h;  // guarantee that w > h for every item
        if(!type[r]) {type[r] = ++t;}
        items.push_back(Item(type[r], r, j));
        total_area += r.area();
    }

    cerr << "Successfully build an instance from " << filename << endl;
}

#pragma endregion 

/************************************************************************/


/************************************************************************/

#pragma region Solution

void print(Node* node)
{
    cout << node << " " << node->rec.w << " " << node->rec.h << " " << node->type << endl;
    for(auto child : node->children)
    {
        print(child);
    }
}

Solution recreate(Solution S, int mat_limit)
{
    Solution S_ = S.deep_copy();
    int num_excluded = S_.excluded_items.size();
    vector<bool> E_;
    vector<bool> is_included; 
    for(int i = 0; i < num_excluded; i++) is_included.push_back(0), E_.push_back(0);
    while(num_excluded > 0)
    {
        vector<Node*> spaces;
        for(auto pattern : S_.patterns)
            extract_spaces(pattern, spaces);
        

        int chosen_index = -1, minimum_cnt = INF;
        for(int i = 0; i < S_.excluded_items.size(); i++)
        {
            if(is_included[i]) continue;
            int cnt = 0;
            for(auto space : spaces)
                cnt += space->rec.able_insert(S_.excluded_items[i].rectangle(0)) + space->rec.able_insert(S_.excluded_items[i].rectangle(1));
            if(cnt < minimum_cnt) minimum_cnt = cnt, chosen_index = i;
        }
        // the above process is to choose the most restricted item

        // then we can generate all the insertion options
        Item chosen_item = S_.excluded_items[chosen_index];
        priority_queue<Insertion, vector<Insertion>, greater<Insertion> > insertion_options;
        for(int i = 0; i < spaces.size(); i++)
        {
            Node* space = spaces[i];
            if(space->rec.able_insert(chosen_item.rectangle(0)))
            {
                double cost = min(vertical_cost(space, chosen_item, 0, evaluation_power), horizontal_cost(space, chosen_item, 0, evaluation_power));
                insertion_options.push(Insertion(i, chosen_index, cost, 0));
            }
            if(space->rec.able_insert(chosen_item.rectangle(1)))
            {
                double cost = min(vertical_cost(space, chosen_item, 1, evaluation_power), horizontal_cost(space, chosen_item, 1, evaluation_power));
                insertion_options.push(Insertion(i, chosen_index, cost, 1));
            }
        }

        // if the item can not be inserted into the current patterns
        if(insertion_options.empty() && mat_limit / bin.area() > S_.bin_number())
        {
            S_.add_new_bin(bin);
            Node *space = S_.patterns[S_.patterns.size() - 1];
            double cost = min(vertical_cost(space, chosen_item, 0, evaluation_power), horizontal_cost(space, chosen_item, 0, evaluation_power));
            insertion_options.push(Insertion(-1, chosen_index, cost, 0));
        }

        //while(Rand(1, 100) <= 5 && insertion_options.size() > 1) insertion_options.pop(); //blinks
        
        is_included[chosen_index] = 1;
        num_excluded--;
        if(insertion_options.empty()) continue;

        
        Insertion chosen_insertion = insertion_options.top();
        Node* space = NULL;
        if(chosen_insertion.space_id == -1) 
            space = S_.patterns[S_.patterns.size() - 1];
        else space = spaces[chosen_insertion.space_id];
        Item item = items[chosen_index];
        E_[chosen_index] = 1;
        insert(space, item, chosen_insertion.rotate);
    }

    vector<Item> tmp;
    for(int i = 0; i < E_.size(); ++i)
    {
        if(!E_[i]) tmp.push_back(S_.excluded_items[i]);
    }
    S_.excluded_items = tmp;
    
    return S_;
}

void extract_spaces(Node *root, vector<Node*> &spaces)
{
    if(root->type > 0) return;
    else if(root->type == -2)
    {
        for(auto child : root->children)
            extract_spaces(child, spaces);
    }
    else spaces.push_back(root);
} 

double vertical_cost(Node *space, Item item, bool rotation, double power)
{
    return  pow(space->area(), power)
            - pow(space->height() * (space->width() - item.width(rotation)), power) 
            - pow(item.width(rotation) * (space->height() - item.height(rotation)), power);
}

double horizontal_cost(Node *space, Item item, bool rotation, double power)
{
    return  pow(space->area(), power)
            - pow(space->width() * (space->height() - item.height(rotation)), power)
            - pow(item.height(rotation) * (space->width() - item.width(rotation)), power);
}

void insert(Node *space, Item item, bool rotate)
{
    // cerr << space->rec.w << " " << space->rec.h << endl;
    // if(rotate) cerr<< "rotate ";
    // else cerr << "not rotate ";
    // cerr << item.rec.w << " " << item.rec.h << endl;
    Rectangle item_shape = item.rectangle(rotate);
        /*
             Scenario 1: Part fits exactly into Node
             ---*****          ---*****             *       ->      *
                *   *             *$$$*
                *   *     ->      *$$$*
                *   *             *$$$*
             ---*****          ---*****

             -> node gets replaced by one node on same level
         */    
        
        if(item_shape.w == space->width() && item_shape.h == space->height())
        {
            space->type = item.index;
            return; 
        }
        /*
            Scenario 2: Part has same dimensions in the direction of the current cut
             ---*****          ---*****             *       ->      $   *
                *   *             *$* *
                *   *     ->      *$* *
                *   *             *$* *
             ---*****          ---*****

             -> node splits into 2 new nodes on same level
         */

        if(space->next_cut_orientation == 1 && item_shape.h == space->height()) 
        //current cut is performed vertically
        {
            Node* remain_node = new Node;
            remain_node->parent = space->parent;
            space->parent->children.push_back(remain_node);
            remain_node->next_cut_orientation = space->next_cut_orientation;
            remain_node->rec.w = space->width() - item_shape.w;
            remain_node->rec.h = space->height();
            space->rec.w = item_shape.w;
            space->type = item.index;
            return;
        }
        if(space->next_cut_orientation == 0 && item_shape.w == space->width()) 
        //current cut is performed horizontally
        {
            Node* remain_node = new Node;
            remain_node->parent = space->parent;
            space->parent->children.push_back(remain_node);
            remain_node->next_cut_orientation = space->next_cut_orientation;
            remain_node->rec.h = space->height() - item_shape.h;
            remain_node->rec.w = space->width();
            space->rec.h = item_shape.h;
            space->type = item.index;
            return;
        }


        /*
             Scenario 3: Part fits exactly in opposite dimension of cut
             ---*****          ---*****             *       ->      * 
                *   *             *   *                            / \
                *   *     ->      *****                           $   *
                *   *             *$$$*
             ---*****          ---*****
         */
        
        if(space->next_cut_orientation == 1 && item_shape.w == space->width())
        //current cut is performed vertically
        {
            Node* left_node  = new Node;
            Node* right_node = new Node;
            
            space->children.push_back(left_node);
            space->children.push_back(right_node);
            space->type = -2;
        
            left_node->parent = space;
            left_node->next_cut_orientation  = !space->next_cut_orientation;
            left_node->rec.h = item_shape.h;
            left_node->rec.w = item_shape.w;
            left_node->type = item.index;

            right_node->parent = space;
            right_node->next_cut_orientation = !space->next_cut_orientation;
            right_node->rec.h = space->height() - item_shape.h;
            right_node->rec.w = space->width();
            return;
        }

        if(space->next_cut_orientation == 0 && item_shape.h == space->height())
        //current cut is performed horizontally
        {
            Node* left_node  = new Node;
            Node* right_node = new Node;
            
            space->children.push_back(left_node);
            space->children.push_back(right_node);
            space->type = -2;
        
            left_node->parent = space;
            left_node->next_cut_orientation  = !space->next_cut_orientation;
            left_node->rec.h = item_shape.h;
            left_node->rec.w = item_shape.w;
            left_node->type = item.index;

            right_node->parent = space;
            right_node->next_cut_orientation = !space->next_cut_orientation;
            right_node->rec.h = space->height();
            right_node->rec.w = space->width() - item_shape.w;
            return;
        }

        bool orientation = (vertical_cost(space, item, rotate, evaluation_power) < horizontal_cost(space, item, rotate, evaluation_power));
        //cerr << orientation << " " << space->next_cut_orientation << endl;
          /*
             Scenario 4: Part doesn't fit exactly in any dimension

             Scenario 4.1: First cut in same direction as current orientation
             ---*****          ---*****             *       ->      *   *
                *   *             * * *                            / \
                *   *     ->      *** *                           $   *
                *   *             *$* *
             ---*****          ---*****

             This requires an extra available level
         */

        if(space->next_cut_orientation == 1 && orientation == 0)
        //current cut is performed vertically
        {
            Node *neighbor = new Node;
            Node *left_child = new Node;
            Node *right_child = new Node;

            neighbor->parent = space->parent;
            neighbor->next_cut_orientation = space->next_cut_orientation;
            neighbor->rec.h = space->height();
            neighbor->rec.w = space->width() - item_shape.w;

            left_child->parent = space;
            left_child->next_cut_orientation = !space->next_cut_orientation;
            left_child->rec.h = item_shape.h;
            left_child->rec.w = item_shape.w;
            left_child->type = item.index;

            right_child->parent = space;
            right_child->next_cut_orientation = !space->next_cut_orientation;
            right_child->rec.h = space->height() - item_shape.h;
            right_child->rec.w = item_shape.w;

            space->parent->children.push_back(neighbor);
            space->children.push_back(left_child);
            space->children.push_back(right_child);
            space->rec.w = space->width() - item_shape.w;
            space->type = -2;

            return; 
        }
        if(space->next_cut_orientation == 0 && orientation == 1)
        //currrent cut is performed horizontally
        {
            Node *neighbor = new Node;
            Node *left_child = new Node;
            Node *right_child = new Node;

            neighbor->parent = space->parent;
            neighbor->next_cut_orientation = space->next_cut_orientation;
            neighbor->rec.h = space->height() - item_shape.h;
            neighbor->rec.w = space->width();

            left_child->parent = space;
            left_child->next_cut_orientation = !space->next_cut_orientation;
            left_child->rec.h = item_shape.h;
            left_child->rec.w = item_shape.w;
            left_child->type = item.index;

            right_child->parent = space;
            right_child->next_cut_orientation = !space->next_cut_orientation;
            right_child->rec.h = item_shape.h;
            right_child->rec.w = space->width() - item_shape.w;

            space->parent->children.push_back(neighbor);
            space->children.push_back(left_child);
            space->children.push_back(right_child);
            space->rec.h = item_shape.h;
            space->type = -2;

            return; 
        }

        /*
             Scenario 4.2: First cut in opposite of current orientation
             ---*****          ---*****             *       ->      * 
                *   *             *   *                            / \
                *   *     ->      *****                           *   *
                *   *             *$* *                          / \
             ---*****          ---*****                         $   *

         */ 
        if(space->next_cut_orientation == 1 && orientation == 1)
        //current cut is performed vertically
        {
            Node *left_node = new Node;
            Node *right_node = new Node;
            Node *left_child = new Node;
            Node *right_child = new Node;

            left_child->parent = left_node;
            left_child->next_cut_orientation = space->next_cut_orientation;
            left_child->rec = item_shape;
            left_child->type = item.index;

            right_child->parent = left_node;
            right_child->next_cut_orientation = space->next_cut_orientation;
            right_child->rec.w = space->width() - item_shape.w;
            right_child->rec.h = item_shape.h;

            left_node->next_cut_orientation = !space->next_cut_orientation;
            left_node->parent = space;
            left_node->children.push_back(left_child);
            left_node->children.push_back(right_child);
            left_node->rec.h = item_shape.h;
            left_node->rec.w = space->width();
            left_node->type = -2;

            right_node->next_cut_orientation = !space->next_cut_orientation;
            right_node->parent = space;
            right_node->rec.h = space->height() - item_shape.h;
            right_node->rec.w = space->width();

            space->children.push_back(left_node);
            space->children.push_back(right_node);
            space->type = -2;

            return;
        }
        if(space->next_cut_orientation == 0 && orientation == 0)
        //current cut is performed horizontally
        {
            Node *left_node = new Node;
            Node *right_node = new Node;
            Node *left_child = new Node;
            Node *right_child = new Node;

            left_child->parent = left_node;
            left_child->next_cut_orientation = space->next_cut_orientation;
            left_child->rec = item_shape;
            left_child->type = item.index;

            right_child->parent = left_node;
            right_child->next_cut_orientation = space->next_cut_orientation;
            right_child->rec.w = item_shape.w;
            right_child->rec.h = space->height() - item_shape.h;

            left_node->next_cut_orientation = !space->next_cut_orientation;
            left_node->parent = space;
            left_node->children.push_back(left_child);
            left_node->children.push_back(right_child);
            left_node->rec.h = space->height();
            left_node->rec.w = item_shape.w;
            left_node->type = -2;

            right_node->next_cut_orientation = !space->next_cut_orientation;
            right_node->parent = space;
            right_node->rec.h = space->height();
            right_node->rec.w = space->width()  - item_shape.w;

            space->children.push_back(left_node);
            space->children.push_back(right_node);
            space->type = -2;

            return;
        }         
}

#pragma endregion 

/************************************************************************/



int main()
{
    build_instance("test.ins2D");
    //srand(time(NULL));
    fstream outfile("test.out");
    outfile << bin.w << " " << bin.h << endl;
    for (int i = 0; i < items.size(); ++i)
    {
        outfile << setw(3) << items[i].index << "th item "
        << setw(3) <<items[i].rec.w << " " << setw(3) << items[i].rec.h
        << " has " << setw(3) << items[i].type << " th type" << endl;
    }
    outfile.close();

    int mat_limit = bin.area() * items.size();
    Solution S, initial_solution;
    S.excluded_items = items;
    S.total_area = total_area;
    initial_solution = recreate(S, mat_limit);
    cerr<<initial_solution.bin_number() <<endl;
    return 0;
}