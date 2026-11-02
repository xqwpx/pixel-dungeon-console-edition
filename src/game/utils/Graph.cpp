#include "game/utils/Graph.h"
#include "game/levels/Room.h"
#include <algorithm>
#include <limits>
#include <queue>
namespace pdv {
void Graph::setPrice(const std::vector<Room*>& nodes,float value){
    for(Room* r:nodes) if(r) r->price=value;
}
void Graph::buildDistanceMap(const std::vector<Room*>& nodes,Room* focus){
    const float inf=std::numeric_limits<float>::infinity();
    for(Room* r:nodes) if(r) r->distance=inf;
    if(!focus) return;
    focus->distance=0;
    using Item=std::pair<float,Room*>;
    struct Cmp{ bool operator()(const Item&a,const Item&b)const{return a.first>b.first;} };
    std::priority_queue<Item,std::vector<Item>,Cmp> q; q.push({0,focus});
    while(!q.empty()){
        Item item=q.top(); q.pop();
        float d=item.first;
        Room* r=item.second;
        if(d!=r->distance) continue;
        for(Room* n:r->neighbours){
            float nd=d+r->price;
            if(nd<n->distance){ n->distance=nd; q.push({nd,n}); }
        }
    }
}
std::vector<Room*> Graph::buildPath(const std::vector<Room*>& nodes,Room* from,Room* to){
    std::vector<Room*> result;
    if(!from||!to||from==to) return result;
    (void)nodes;
    Room* cur=from;
    int guard=0;
    while(cur!=to && guard++<10000){
        Room* best=nullptr;
        float bestD=cur->distance;
        for(Room* n:cur->neighbours){
            if(n->distance<bestD){bestD=n->distance;best=n;}
        }
        if(!best){result.clear();return result;}
        result.push_back(best); cur=best;
    }
    return result;
}
}
