#pragma once
namespace pdv {
struct Point {
    int x=0, y=0;
    Point()=default;
    Point(int px,int py):x(px),y(py){}
    Point& set(int px,int py){x=px;y=py;return *this;}
    Point& offset(int dx,int dy){x+=dx;y+=dy;return *this;}
    bool operator==(const Point& o) const { return x==o.x && y==o.y; }
    bool operator!=(const Point& o) const { return !(*this==o); }
};
}
