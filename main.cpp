#pragma GCC optimize(3,"Ofast","inline")

#include <iostream>
#include <lahc.h>
#include <ctime>
#include <queue>

using namespace std;

// Constants
#define INF 1e9
#define elap 1000//1000000

// Parameters
const double time_limit = 5.0;
const double evaluation_power = 1.2;
const int average_removed_nodes = 4;
const int length_of_history = 1000;
const int max_iteration = 10000000;

// IO files
ofstream outfile;

// Data Structures
Rectangle bin(0, 0);
vector<Item> items;
int total_area = 0;
int iteration, i_accepted = 0;
// Time
clock_t start, finish;

/****************************************************************************************/
#pragma region Instance

void build_instance(const char *filename)
{
    ifstream infile(filename);
    int n;
    int j, w, h, d, b, p;

    if(!infile.is_open()) 
    {
        cerr << "cannot open " << filename << endl;
        exit(1);
    }

    infile >> n >> bin.w >> bin.h;

    while(n--)
    {
        infile >> j >> w >> h >> d >> b >> p;
        if(w < h) swap(w, h);
        items.push_back(Item(j, Rectangle(w, h)));
        total_area += w * h;
    }

    cerr << "Successfully build an instance from " << filename << endl;
}

#pragma endregion
/****************************************************************************************/

#pragma region Solution

Solution recreate(Solution S, int mat_limit)
{
    Solution S_ = S.deep_copy();
    // for(int i = 0; i < S_.excluded_items.size(); ++i)
    // cout << S_.excluded_items[i].index << " " << S_.excluded_items[i].rec.w << " " << S_.excluded_items[i].rec.h << endl;
    int num_excluded = S_.excluded_items.size();
    // vector<bool> is_included(num_excluded, 0);
    // for(int i = 0; i < num_excluded; ++i) cout << E_[i] << " ";
    while(num_excluded--)
    {
        //cout << spaces.size() << endl;
        int chosen_index = -1, minimum_cnt = INF;
        for(int i = 0; i < S_.option_cnt.size(); ++i)
        {
            if(S_.option_cnt[i] == -1) continue;
            if(S_.option_cnt[i] < minimum_cnt) minimum_cnt = S_.option_cnt[i], chosen_index = i;
        }
        // the above process is to choose the most restricted item
        
        Item chosen_item = S_.excluded_items[chosen_index];
        
        priority_queue<Insertion, vector<Insertion>, greater<Insertion> > insertion_options;
        for(int i = 0; i < S_.leftovers.size(); i++)
        {
            shared_ptr<Node> space = S_.leftovers[i];
            if(space->width() >= chosen_item.width(0) && space->height() >= chosen_item.height(0))
            {
                double cost = min(vertical_cost(space->rec, chosen_item.rectangle(0), evaluation_power)
                                ,horizontal_cost(space->rec, chosen_item.rectangle(0), evaluation_power));
                insertion_options.push(Insertion(i, cost, 0));
            }
            if(space->width() >= chosen_item.width(1) && space->height() >= chosen_item.height(1))
            {
                double cost = min(vertical_cost(space->rec, chosen_item.rectangle(1), evaluation_power)
                                ,horizontal_cost(space->rec, chosen_item.rectangle(1), evaluation_power));
                insertion_options.push(Insertion(i, cost, 1));
                //while(insertion_options.size() > 5) insertion_options.pop();
            }
        }

        if(insertion_options.empty() && mat_limit > S_.bin_number())
        {
            S_.add_new_bin(bin);
            double cost = min(vertical_cost(bin, chosen_item.rectangle(0), evaluation_power)
                                ,horizontal_cost(bin, chosen_item.rectangle(0), evaluation_power));
            insertion_options.push(Insertion(-1, cost, 0));
        }

        int blink = Rand(1, 100);
        while(blink <= 5 && insertion_options.size() > 1) blink = Rand(1, 100), insertion_options.pop(); 
        
        if(insertion_options.empty()) continue;
        Insertion chosen_insertion = insertion_options.top();
        shared_ptr<Node> space = NULL;
        if(chosen_insertion.space_id == -1) 
            {
                space = S_.patterns[S_.patterns.size() - 1];
                Rectangle tmp_space = space->rec;
                for(int i = 0; i < S_.option_cnt.size(); ++i)
                {
                    if(S_.option_cnt[i] == -1) continue;
                    Item tmp = S_.excluded_items[i];
                    S_.option_cnt[i] += (tmp_space.w >= tmp.width(0) && tmp_space.h >= tmp.height(0))
                                    + (tmp_space.w >= tmp.width(1) && tmp_space.h >= tmp.height(1));
                }
            }
        else {
            space = S_.leftovers[chosen_insertion.space_id];
            S_.leftovers.erase(S_.leftovers.begin() + chosen_insertion.space_id);
        }
        
        Rectangle tmp_space = space->rec;
        for(int i = 0; i < S_.option_cnt.size(); ++i)
        {
            if(S_.option_cnt[i] == -1) continue;
            Item tmp = S_.excluded_items[i];
            S_.option_cnt[i] -= (tmp_space.w >= tmp.width(0) && tmp_space.h >= tmp.height(0))
                            + (tmp_space.w >= tmp.width(1) && tmp_space.h >= tmp.height(1));
        }
        S_.option_cnt[chosen_index] = -1;
        insert(space, chosen_item, chosen_insertion.rotate, S_);
    }
    vector<Item> tmp;
    vector<int> cnt;
    for(int i = 0; i < S_.option_cnt.size(); ++i)
    {
        if(S_.option_cnt[i] != -1) tmp.push_back(S_.excluded_items[i]), cnt.push_back(S_.option_cnt[i]);
    }
    S_.excluded_items = tmp;
    S_.option_cnt = cnt;
    return S_;
}

void insert(shared_ptr<Node> space, Item item, bool rotate, Solution &S)
{
    //if(item.rec.w > 100 || item.rec.w < 0) { cout<< "bug " << item.index << endl; }
    //if(space->rec.w > 100 || space->rec.w < 0) { cout<< "bug2 " << item.index << endl;}
    Rectangle item_rec = item.rectangle(rotate);
    Rectangle space_rec = space->rec;    
        /*
             Scenario 1: Part fits exactly into Node
             ---*****          ---*****             *       ->      *
                *   *             *$$$*
                *   *     ->      *$$$*
                *   *             *$$$*
             ---*****          ---*****

             -> node gets replaced by one node on same level
         */
        if(item_rec.w == space_rec.w && item_rec.h == space_rec.h) 
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
        if(space->next_cut_orientation == 1 && item_rec.h == space_rec.h)
        //current cut is performed vertically
        {
            shared_ptr<Node> remain_node(new Node);
            remain_node->parent = space->parent; 
            space->parent.lock()->children.push_back(remain_node);
            remain_node->next_cut_orientation = space->next_cut_orientation;
            remain_node->rec.w = space_rec.w - item_rec.w;
            remain_node->rec.h = space_rec.h;
            space->rec.w = item_rec.w;
            space->type = item.index;
            
            S.leftovers.push_back(remain_node);
            Rectangle tmp_space = remain_node->rec;
            for(int i = 0; i < S.option_cnt.size(); ++i)
            {
                if(S.option_cnt[i] == -1) continue;
                Item tmp = S.excluded_items[i];
                S.option_cnt[i] += (tmp_space.w >= tmp.width(0) && tmp_space.h >= tmp.height(0))
                                + (tmp_space.w >= tmp.width(1) && tmp_space.h >= tmp.height(1));
            }
            return;
        }
        if(space->next_cut_orientation == 0 && item_rec.w == space_rec.w && !space->parent.expired())
        //current cut is performed horizontally
        {
            shared_ptr<Node> remain_node(new Node);
            remain_node->parent = space->parent; 
            space->parent.lock()->children.push_back(remain_node);
            remain_node->next_cut_orientation = space->next_cut_orientation;
            remain_node->rec.h = space_rec.h - item_rec.h;
            remain_node->rec.w = space_rec.w;
            space->rec.h = item_rec.h;
            space->type = item.index;
            
            S.leftovers.push_back(remain_node);
            Rectangle tmp_space = remain_node->rec;
            for(int i = 0; i < S.option_cnt.size(); ++i)
            {
                if(S.option_cnt[i] == -1) continue;
                Item tmp = S.excluded_items[i];
                S.option_cnt[i] += (tmp_space.w >= tmp.width(0) && tmp_space.h >= tmp.height(0))
                                + (tmp_space.w >= tmp.width(1) && tmp_space.h >= tmp.height(1));
            }
            return;
        }


        if(space->next_cut_orientation == 0 && item_rec.w == space_rec.w && space->parent.expired())
        {
           space->next_cut_orientation = 1; 
        }
        /*
             Scenario 3: Part fits exactly in opposite dimension of cut
             ---*****          ---*****             *       ->      * 
                *   *             *   *                            / \
                *   *     ->      *****                           $   *
                *   *             *$$$*
             ---*****          ---*****
         */
        if(space->next_cut_orientation == 1 && item_rec.w == space_rec.w)
        //next cut is performed horizontally
        {
            shared_ptr<Node> left_node(new Node);
            shared_ptr<Node> right_node(new Node);

            space->children.push_back(left_node);
            space->children.push_back(right_node);
            space->type = -2;

            left_node->parent = space;
            left_node->next_cut_orientation = 0;
            left_node->rec.h = item_rec.h;
            left_node->rec.w = item_rec.w;
            left_node->type = item.index;

            right_node->parent = space;
            right_node->next_cut_orientation = 0;
            right_node->rec.h = space_rec.h - item_rec.h;
            right_node->rec.w = item_rec.w;
            
            S.leftovers.push_back(right_node);
            Rectangle tmp_space = right_node->rec;
            for(int i = 0; i < S.option_cnt.size(); ++i)
            {
                if(S.option_cnt[i] == -1) continue;
                Item tmp = S.excluded_items[i];
                S.option_cnt[i] += (tmp_space.w >= tmp.width(0) && tmp_space.h >= tmp.height(0))
                                + (tmp_space.w >= tmp.width(1) && tmp_space.h >= tmp.height(1));
            }
            return;
        }

        if(space->next_cut_orientation == 0 && item_rec.h == space_rec.h)
        //next cut is performed vertically
        {
            shared_ptr<Node> left_node(new Node);
            shared_ptr<Node> right_node(new Node);

            space->children.push_back(left_node);
            space->children.push_back(right_node);
            space->type = -2;

            left_node->parent = space;
            left_node->next_cut_orientation = 1;
            left_node->rec.h = item_rec.h;
            left_node->rec.w = item_rec.w;
            left_node->type = item.index;

            right_node->parent = space;
            right_node->next_cut_orientation = 1;
            right_node->rec.h = item_rec.h;
            right_node->rec.w = space_rec.w - item_rec.w;
            
            S.leftovers.push_back(right_node);
            Rectangle tmp_space = right_node->rec;
            for(int i = 0; i < S.option_cnt.size(); ++i)
            {
                if(S.option_cnt[i] == -1) continue;
                Item tmp = S.excluded_items[i];
                S.option_cnt[i] += (tmp_space.w >= tmp.width(0) && tmp_space.h >= tmp.height(0))
                                + (tmp_space.w >= tmp.width(1) && tmp_space.h >= tmp.height(1));
            }
            return;         
        }
        
        bool orientation = (vertical_cost(space_rec, item_rec, evaluation_power) > horizontal_cost(space_rec, item_rec, evaluation_power));

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
        // current cut is performed vertically
        {
            shared_ptr<Node> neighbor(new Node);
            shared_ptr<Node> left_child(new Node);
            shared_ptr<Node> right_child(new Node);

            neighbor->parent = space->parent;
            neighbor->next_cut_orientation = 1;
            neighbor->rec.h = space_rec.h;
            neighbor->rec.w = space_rec.w - item_rec.w;

            left_child->parent = space;
            left_child->next_cut_orientation = 0;
            left_child->rec.h = item_rec.h;
            left_child->rec.w = item_rec.w;
            left_child->type = item.index;

            right_child->parent = space;
            right_child->next_cut_orientation = 0;
            right_child->rec.h = space_rec.h - item_rec.h;
            right_child->rec.w = item_rec.w;

            space->parent.lock()->children.push_back(neighbor);
            space->children.push_back(left_child);
            space->children.push_back(right_child);
            space->rec.w = item_rec.w;
            space->type = -2;

            S.leftovers.push_back(right_child);
            Rectangle tmp_space = right_child->rec;
            for(int i = 0; i < S.option_cnt.size(); ++i)
            {
                if(S.option_cnt[i] == -1) continue;
                Item tmp = S.excluded_items[i];
                S.option_cnt[i] += (tmp_space.w >= tmp.width(0) && tmp_space.h >= tmp.height(0))
                                + (tmp_space.w >= tmp.width(1) && tmp_space.h >= tmp.height(1));
            }

            S.leftovers.push_back(neighbor);
            tmp_space = neighbor->rec;
            for(int i = 0; i < S.option_cnt.size(); ++i)
            {
                if(S.option_cnt[i] == -1) continue;
                Item tmp = S.excluded_items[i];
                S.option_cnt[i] += (tmp_space.w >= tmp.width(0) && tmp_space.h >= tmp.height(0))
                                + (tmp_space.w >= tmp.width(1) && tmp_space.h >= tmp.height(1));
            }
            return;
        }
        if(space->next_cut_orientation == 0 && orientation == 1 && !space->parent.expired())
        // current cut is performed horizontally
        {
            shared_ptr<Node> neighbor(new Node);
            shared_ptr<Node> left_child(new Node);
            shared_ptr<Node> right_child(new Node);

            neighbor->parent = space->parent;
            neighbor->next_cut_orientation = 0;
            neighbor->rec.h = space_rec.h - item_rec.h;
            neighbor->rec.w = space_rec.w;

            left_child->parent = space;
            left_child->next_cut_orientation = 1;
            left_child->rec.h = item_rec.h;
            left_child->rec.w = item_rec.w;
            left_child->type = item.index;

            right_child->parent = space;
            right_child->next_cut_orientation = 1;
            right_child->rec.h = item_rec.h;
            right_child->rec.w = space_rec.w - item_rec.w;

            space->parent.lock()->children.push_back(neighbor);
            space->children.push_back(left_child);
            space->children.push_back(right_child);
            space->rec.h = item_rec.h;
            space->type = -2;

            S.leftovers.push_back(right_child);
            Rectangle tmp_space = right_child->rec;
            for(int i = 0; i < S.option_cnt.size(); ++i)
            {
                if(S.option_cnt[i] == -1) continue;
                Item tmp = S.excluded_items[i];
                S.option_cnt[i] += (tmp_space.w >= tmp.width(0) && tmp_space.h >= tmp.height(0))
                                + (tmp_space.w >= tmp.width(1) && tmp_space.h >= tmp.height(1));
            }

            S.leftovers.push_back(neighbor);
            tmp_space = neighbor->rec;
            for(int i = 0; i < S.option_cnt.size(); ++i)
            {
                if(S.option_cnt[i] == -1) continue;
                Item tmp = S.excluded_items[i];
                S.option_cnt[i] += (tmp_space.w >= tmp.width(0) && tmp_space.h >= tmp.height(0))
                                + (tmp_space.w >= tmp.width(1) && tmp_space.h >= tmp.height(1));
            }
            return;
        }
        if(space->next_cut_orientation == 0 && orientation == 1 && space->parent.expired())
        space->next_cut_orientation = 1;

        /*
             Scenario 4.2: First cut in opposite of current orientation
             ---*****          ---*****             *       ->      * 
                *   *             *   *                            / \
                *   *     ->      *****                           *   *
                *   *             *$* *                          / \
             ---*****          ---*****                         $   *

         */ 
        if(space->next_cut_orientation == 1 && orientation == 1)
        // current cut is performed vertically
        {
            shared_ptr<Node> left_node(new Node);
            shared_ptr<Node> right_node(new Node);
            shared_ptr<Node> left_child(new Node);
            shared_ptr<Node> right_child(new Node);
            
            left_child->parent = left_node; 
            left_child->next_cut_orientation = 1;
            left_child->rec.h = item_rec.h;
            left_child->rec.w = item_rec.w;
            left_child->type = item.index;

            right_child->parent = left_node;
            right_child->next_cut_orientation = 1;
            right_child->rec.h = item_rec.h;
            right_child->rec.w = space_rec.w - item_rec.w;

            left_node->parent = space;
            left_node->next_cut_orientation = 0;
            left_node->rec.h = item_rec.h;
            left_node->rec.w = space_rec.w;
            left_node->type = -2;
            left_node->children.push_back(left_child);
            left_node->children.push_back(right_child);

            right_node->parent = space;
            right_node->next_cut_orientation = 0;
            right_node->rec.h = space_rec.h - item_rec.h;
            right_node->rec.w = space_rec.w;

            space->children.push_back(left_node);
            space->children.push_back(right_node);
            space->type = -2;
            
            S.leftovers.push_back(right_child);
            Rectangle tmp_space = right_child->rec;
            for(int i = 0; i < S.option_cnt.size(); ++i)
            {
                if(S.option_cnt[i] == -1) continue;
                Item tmp = S.excluded_items[i];
                S.option_cnt[i] += (tmp_space.w >= tmp.width(0) && tmp_space.h >= tmp.height(0))
                                + (tmp_space.w >= tmp.width(1) && tmp_space.h >= tmp.height(1));
            }

            S.leftovers.push_back(right_node);
            tmp_space = right_node->rec;
            for(int i = 0; i < S.option_cnt.size(); ++i)
            {
                if(S.option_cnt[i] == -1) continue;
                Item tmp = S.excluded_items[i];
                S.option_cnt[i] += (tmp_space.w >= tmp.width(0) && tmp_space.h >= tmp.height(0))
                                + (tmp_space.w >= tmp.width(1) && tmp_space.h >= tmp.height(1));
            }
            return;
        }

        if(space->next_cut_orientation == 0 && orientation == 0)
        // current cut is performed horizontally 
        {
            shared_ptr<Node> left_node(new Node);
            shared_ptr<Node> right_node(new Node);
            shared_ptr<Node> left_child(new Node);
            shared_ptr<Node> right_child(new Node);
            
            left_child->parent = left_node; 
            left_child->next_cut_orientation = 0;
            left_child->rec.h = item_rec.h;
            left_child->rec.w = item_rec.w;
            left_child->type = item.index;

            right_child->parent = left_node;
            right_child->next_cut_orientation = 0;
            right_child->rec.h = space_rec.h - item_rec.h;
            right_child->rec.w = item_rec.w;

            left_node->parent = space;
            left_node->next_cut_orientation = 1;
            left_node->rec.h = space_rec.h;
            left_node->rec.w = item_rec.w;
            left_node->type = -2;
            left_node->children.push_back(left_child);
            left_node->children.push_back(right_child);

            right_node->parent = space;
            right_node->next_cut_orientation = 1;
            right_node->rec.h = space_rec.h;
            right_node->rec.w = space_rec.w - item_rec.w;

            space->children.push_back(left_node);
            space->children.push_back(right_node);
            space->type = -2;
            
            S.leftovers.push_back(right_child);
            Rectangle tmp_space = right_child->rec;
            for(int i = 0; i < S.option_cnt.size(); ++i)
            {
                if(S.option_cnt[i] == -1) continue;
                Item tmp = S.excluded_items[i];
                S.option_cnt[i] += (tmp_space.w >= tmp.width(0) && tmp_space.h >= tmp.height(0))
                                + (tmp_space.w >= tmp.width(1) && tmp_space.h >= tmp.height(1));
            }

            S.leftovers.push_back(right_node);
            tmp_space = right_node->rec;
            for(int i = 0; i < S.option_cnt.size(); ++i)
            {
                if(S.option_cnt[i] == -1) continue;
                Item tmp = S.excluded_items[i];
                S.option_cnt[i] += (tmp_space.w >= tmp.width(0) && tmp_space.h >= tmp.height(0))
                                + (tmp_space.w >= tmp.width(1) && tmp_space.h >= tmp.height(1));
            }
            return;
        }
}


double vertical_cost(Rectangle space, Rectangle item, double power)
{
    return pow(space.area(), power)
            - pow(space.h * (space.w - item.w), power)
            - pow(item.w  * (space.h - item.h), power);
}

double horizontal_cost(Rectangle space, Rectangle item, double power)
{
    return pow(space.area(), power)
            - pow(space.w * (space.h - item.h), power)
            - pow(item.h * (space.w - item.w), power);
}

#pragma endregion

/****************************************************************************************/

#pragma region Ruin

Solution ruin(Solution S, int mat_limit, int average_nodes)
{
    Solution S_ = S.deep_copy();
    int max_bound = average_nodes << 1;
    int _i = average_nodes == 0 ? 0 : (rand() % (max_bound - 1) + 1);\
    
    //_i = 1;
    // cout << S_.leftovers.size() << endl;
    // cout << _i << endl;
    int unchange_layout_cnt = S_.unchanged_number();
    //cout << unchange_layout_cnt << endl;

    while(_i > 0 || S_.bin_number() >= mat_limit)
    {
        // cout << "No. " << _i << endl;
        if(S_.bin_number() == unchange_layout_cnt) break;
        vector<shared_ptr<Node>> removable_nodes;
        int patterns_len = S_.patterns.size();
        int choose_one = Rand(0, patterns_len - 1);
        
        while(S_.patterns[choose_one]->usage == 1) 
        choose_one = Rand(0, patterns_len - 1);
        
        shared_ptr<Node> pattern = S_.patterns[choose_one];
        get_all_removable_nodes(pattern, removable_nodes);


        int choose_node_index = Rand(0, removable_nodes.size() - 1);
        //cout << removable_nodes[choose_node_index]->type << " ";
        exclude(removable_nodes[choose_node_index], S_);

        remove_node(removable_nodes[choose_node_index], S_);
        if(pattern->type == -1)
        {
            auto it = S_.patterns.begin();
            while(*it != pattern) it++;
            S_.patterns.erase(it);

            it = S_.leftovers.begin();
            while(*it != pattern) it++;
            S_.leftovers.erase(it);
        }
        _i--;
    }
    return S_;
}

void get_all_removable_nodes(shared_ptr<Node> pattern, vector<shared_ptr<Node>> &removable_nodes)
{
    if(pattern->type == -1) return; 
    else if(pattern->type == -2)
    {
        removable_nodes.push_back(pattern);
        for(auto child : pattern->children)
        {
            get_all_removable_nodes(child, removable_nodes);
        }
    }
    else removable_nodes.push_back(pattern);
}

void remove_node(shared_ptr<Node> node, Solution &S)
{
    node->type = -1;
    if(node->rec.w == bin.w && node->rec.h == bin.h) { S.leftovers.push_back(node); return; }
    vector<shared_ptr<Node>> neighbors = node->parent.lock()->children;
    weak_ptr<Node> lneighbor, rneighbor;
    for(int i = 0; i < neighbors.size(); i++)
    {
        if(neighbors[i] == node)
        {
            if(i > 0) lneighbor = neighbors[i - 1];
            if(i < neighbors.size() - 1) rneighbor = neighbors[i + 1];
            break;
        }
    }
    if(!lneighbor.expired()) merge_two_nodes(node, lneighbor.lock(), S);
    if(!rneighbor.expired()) merge_two_nodes(node, rneighbor.lock(), S);
    
    shared_ptr<Node> parent = node->parent.lock();
    if(node->rec == parent->rec)
    {
        parent->type = -1;
        S.leftovers.push_back(parent);
        parent->children.clear(); 
        if(!(node->rec == bin))
        {
            Rectangle tmp_space = parent->rec;
            for(int i = 0; i < S.option_cnt.size(); ++i)
            {
                if(S.option_cnt[i] == -1) continue;
                Item tmp = S.excluded_items[i];
                S.option_cnt[i] += (tmp_space.w >= tmp.width(0) && tmp_space.h >= tmp.height(0))
                                + (tmp_space.w >= tmp.width(1) && tmp_space.h >= tmp.height(1));
            }
        }

    }
    else {
        S.leftovers.push_back(node);
        if(!(node->rec == bin))
        {
            Rectangle tmp_space = node->rec;
            for(int i = 0; i < S.option_cnt.size(); ++i)
            {
                if(S.option_cnt[i] == -1) continue;
                Item tmp = S.excluded_items[i];
                S.option_cnt[i] += (tmp_space.w >= tmp.width(0) && tmp_space.h >= tmp.height(0))
                                + (tmp_space.w >= tmp.width(1) && tmp_space.h >= tmp.height(1));
            }
        }
    }
}

void merge_two_nodes(shared_ptr<Node> master, shared_ptr<Node> slave, Solution &S)
{
    if(master->type != -1 || slave->type != -1) return;
    if(master->next_cut_orientation == 0)
        master->rec.h += slave->rec.h;
    else 
        master->rec.w += slave->rec.w;

    auto it = master->parent.lock()->children.begin();
    while(*it != slave) it++;
    master->parent.lock()->children.erase(it);

    it = S.leftovers.begin();
    while(*it != slave) it++;
    S.leftovers.erase(it);
}

void exclude(shared_ptr<Node> root, Solution &S)
{
    //cerr << "bug occur" << root->type << endl;
    //if(!root.use_count()) {cout << "bug" << endl; exit(1); }
    if(root->type == -1) {
        //cout << S.leftovers.size() << endl;

        Rectangle tmp_space = root->rec;
        for(int i = 0; i < S.option_cnt.size(); ++i)
        {
            if(S.option_cnt[i] == -1) continue;
            Item tmp = S.excluded_items[i];
            S.option_cnt[i] -= (tmp_space.w >= tmp.width(0) && tmp_space.h >= tmp.height(0))
                            + (tmp_space.w >= tmp.width(1) && tmp_space.h >= tmp.height(1));
        }

        auto it = S.leftovers.begin();
        while(*it != root) { it++; }
        S.leftovers.erase(it);
        return;
    }
    else if(root->type == -2) 
    {
        for(auto child : root->children)
        {
            exclude(child, S);
        }
        root->children.clear();
    }
    else if(root->type > 0)
    { 
        S.excluded_items.push_back(items[root->type - 1]);
        S.option_cnt.push_back(0);
        Item tmp = items[root->type - 1];
        //cout << tmp.index << " ";
        for(auto space : S.leftovers)
        {
            Rectangle tmp_space = space->rec;
            S.option_cnt[S.option_cnt.size() - 1] += (tmp_space.w >= tmp.width(0) && tmp_space.h >= tmp.height(0))
                            + (tmp_space.w >= tmp.width(1) && tmp_space.h >= tmp.height(1));
        }
        return;
    }
}

#pragma endregion

/****************************************************************************************/

#pragma region LAHC

void LAHC(int mat_limit, Solution &S)
{
    int excluded_area = S.excluded_area();
    double value_S = S.value(evaluation_power);
    pair<int, double> cost_S = make_pair(S.excluded_area(), S.value(evaluation_power));
    vector<pair<int, double>> cost_functions(length_of_history, cost_S);
    int i_ = 0, i_dle = 0;
    Solution candidate = S.deep_copy();
    pair<int, double> cost_candidate = cost_S;
    vector<shared_ptr<Node>> append_spaces; 
    while(i_ <= max_iteration || i_dle <= 0.2 * max_iteration)
    {
        finish = clock();
        double time = (double) (finish - start) / elap;
        if(time > time_limit) break;
        //cout << time << endl;
        
        Solution S_ = ruin(candidate, mat_limit, average_removed_nodes);
        S_ = recreate(S_, mat_limit);
        append_spaces.clear();
        int excluded_area_S_ = S_.excluded_area();
        double value_S_ = S_.value(evaluation_power);
        pair<int, double> cost_S_ = make_pair(excluded_area_S_, value_S_);
        int v = i_ % length_of_history;
        
        if(compare(cost_S_, cost_functions[v]) <= 0 || compare(cost_S_, cost_candidate) <= 0)
        {
            i_accepted++;
            candidate = S_;
            cost_candidate = cost_S_;
            if(compare(cost_candidate, cost_functions[v]) < 0) 
            cost_functions[v] = cost_candidate;
        }
        else i_dle++;
        if(cost_candidate.first == 0) {
            S = candidate; 
            mat_limit = S.patterns.size();
            candidate = ruin(S, mat_limit, 0);
            excluded_area = candidate.excluded_area();
            double value_candidate = candidate.value(evaluation_power);
            cost_candidate = make_pair(excluded_area, value_candidate);
            cost_functions.clear();
            for(int i = 0; i < length_of_history; ++i)
            cost_functions.push_back(cost_candidate);
        }   
        i_++;     
    }
    iteration = i_;
}


int compare(pair<int, double> a, pair<int, double> b) 
{
    if(a.first == b.first && a.second == b.second) return 0;
    if(a.first == b.first) 
    {
        if(a.second > b.second) return -1;
        else return 1;
    }
    else {
        if(a.first < b.first) return -1;
        else return 1;
    }
}

#pragma endregion


/****************************************************************************************/

#pragma region print

void print_test(shared_ptr<Node> node, pair<int, int> o, int bin)
{
    outfile << node->type << " " << bin << " " << node->rec.w << " " << node->rec.h << " " << o.first << " " << o.second << endl;
    if(node->type == -1 || node->type == 0) return;
    for(auto child : node->children)
    {
        if(node->next_cut_orientation == 0) print_test(child, o, bin), o.first += child->rec.w;
        else print_test(child, o, bin), o.second += child->rec.h;
    }
}

void print_item(shared_ptr<Node> node, pair<int, int> o, int bin)
{
    if(node->type == -1) return;
    if(node->type > 0) 
    {
        outfile << node->type << " " << bin << " " << node->rec.w << " " << node->rec.h << " " << o.first << " " << o.second << endl;
        return;
    }
    for(auto child : node->children)
    {
        if(node->next_cut_orientation == 0) print_item(child, o, bin), o.first += child->rec.w;
        else print_item(child, o, bin), o.second += child->rec.h;
    }
}

void print_leftover(shared_ptr<Node> node, pair<int, int> o, int bin)
{
    if(node->type > 0) return;
    if(node->type == -1) 
    {
        outfile << node->type << " " << bin << " " << node->rec.w << " " << node->rec.h << " " << o.first << " " << o.second << endl;
        return;
    }
    for(auto child : node->children)
    {
        if(node->next_cut_orientation == 0) print_leftover(child, o, bin), o.first += child->rec.w;
        else print_leftover(child, o, bin), o.second += child->rec.h;
    }
}
void print_pattern(vector<shared_ptr<Node>> patterns)
{
    for(int i = 0; i < patterns.size(); i++)
    print_item(patterns[i], make_pair(0, 0), i);
    for(int i = 0; i < patterns.size(); i++)
    print_leftover(patterns[i], make_pair(0, 0), i);
}

#pragma endregion


int main(int argc, char* argv[])
{
    ofstream result;
    //result.open("Solution.csv", std::ios::app);
    srand(time(NULL));
    start = clock();
    build_instance(argv[1]);
    outfile.open("test.out");
    Solution S;
    S.excluded_items = items;
    S.total_area = total_area;
    for(int i = 0; i < items.size(); ++i) S.option_cnt.push_back(0);
    //S = recreate(S, items.size());
    //int mat_limit = S.bin_number();
    //S = ruin(S, items.size(), average_removed_nodes);
    //S = recreate(S, mat_limit);
    
    //LAHC(items.size(), S);
    for(int i = 0; i < 1000; ++i) Solution S_ = recreate(S, items.size());
    
    //vector<shared_ptr<Node>> appends;
    //Solution best_sol;
    //best_sol = local_search(S, items.size(), best_sol, appends);
    print_pattern(S.patterns);
    cout << iteration / time_limit << endl;
    cout << i_accepted / time_limit << endl;
    outfile << S.patterns.size() << endl;
    
    // for(int i = 0; i < S.patterns.size(); ++i)
    // {
    //     outfile << "No." << i << " Pattern: " << endl;
    //     print_test(S.patterns[i], make_pair(0, 0), i);
    //     outfile << endl;
    // }
    
    // for(int i = 0; i < S.excluded_items.size(); ++i)
    // {
    //     cout << S.excluded_items[i].index << " " << S.excluded_items[i].rec.w << " " << S.excluded_items[i].rec.h << " " << S.option_cnt[i] << endl;
    // }
    // cout << endl;

    // cout << S.leftovers.size() << endl;
    // for(auto leftover : S.leftovers)
    // {
    //     cout << leftover.use_count() << " ";
    // }
    // cout << endl;
    finish = clock();
    double init_time = (double) (finish - start) / elap;
    printf("%6.2lf\n", init_time);
    return 0;
}