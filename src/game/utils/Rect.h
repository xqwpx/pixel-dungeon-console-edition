#pragma once
#include <algorithm>
#include "game/utils/Point.h"
namespace pdv {
class Rect {
public:
    int left=0, top=0, right=0, bottom=0;
    Rect()=default;
    Rect(int l,int t,int r,int b):left(l),top(t),right(r),bottom(b){}
    Rect(const Rect&)=default;
    Rect& operator=(const Rect&)=default;
    int width() const { return right-left; }
    int height() const { return bottom-top; }
    int square() const { return width()*height(); }
    bool isEmpty() const { return right<=left || bottom<=top; }
    Rect& set(int l,int t,int r,int b){left=l;top=t;right=r;bottom=b;return *this;}
    Rect& set(const Rect& r){return set(r.left,r.top,r.right,r.bottom);}
    Rect intersect(const Rect& r) const {
        return Rect(std::max(left,r.left),std::max(top,r.top),
                    std::min(right,r.right),std::min(bottom,r.bottom));
    }
    bool inside(const Point& p) const {
        return p.x>=left && p.x<right && p.y>=top && p.y<bottom;
    }
    Rect shrink(int d) const { return Rect(left+d,top+d,right-d,bottom-d); }
};
}
