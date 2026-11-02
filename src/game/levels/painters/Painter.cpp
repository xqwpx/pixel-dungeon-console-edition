#include "game/levels/painters/Painter.h"
#include "game/levels/Level.h"
#include "game/levels/Room.h"
#include <stdlib.h>
namespace pdv {
void Painter::set(Level&l,int x,int y,int v){l.setRaw(x,y,v);}
void Painter::set(Level&l,const Point&p,int v){set(l,p.x,p.y,v);}
void Painter::fill(Level&l,int x,int y,int w,int h,int v){
 for(int yy=y;yy<y+h;++yy)for(int xx=x;xx<x+w;++xx)set(l,xx,yy,v);
}
void Painter::fill(Level&l,const Rect&r,int v){fill(l,r.left,r.top,r.width()+1,r.height()+1,v);}
void Painter::fill(Level&l,const Rect&r,int m,int v){fill(l,r.left+m,r.top+m,r.width()+1-m*2,r.height()+1-m*2,v);}
Point Painter::drawInside(Level&l,const Room&r,const Point&from,int n,int v){
 Point step;
 if(from.x==r.left)step.set(1,0);else if(from.x==r.right)step.set(-1,0);
 else if(from.y==r.top)step.set(0,1);else if(from.y==r.bottom)step.set(0,-1);
 Point p(from.x+step.x,from.y+step.y);
 for(int i=0;i<n;++i){if(v!=-1)set(l,p,v);p.offset(step.x,step.y);}
 return p;
}}
