#pragma once
#include "game/utils/Point.h"
#include "game/utils/Rect.h"
namespace pdv { class Level; class Room;
class Painter {
public:
 static void set(Level&,int x,int y,int v);
 static void set(Level&,const Point&,int v);
 static void fill(Level&,int x,int y,int w,int h,int v);
 static void fill(Level&,const Rect&,int v);
 static void fill(Level&,const Rect&,int margin,int v);
 static Point drawInside(Level&,const Room&,const Point&,int n,int v);
};}
