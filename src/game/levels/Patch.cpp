#include "game/levels/Patch.h"
#include "game/utils/Random.h"
namespace pdv {
std::array<bool,Level::Length> Patch::cur_{};
std::array<bool,Level::Length> Patch::off_{};
const std::array<bool,Level::Length>& Patch::generate(float seed,int nGen){
 for(int i=0;i<Level::Length;++i)off_[i]=Random::Float()<seed;
 for(int g=0;g<nGen;++g){
  for(int y=1;y<Level::Height-1;++y)for(int x=1;x<Level::Width-1;++x){
   int p=x+y*Level::Width,count=0;
   for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx)if(dx||dy)count+=off_[p+dy*Level::Width+dx]?1:0;
   cur_[p]=(!off_[p]&&count>=5)||(off_[p]&&count>=4);
  }
  cur_.swap(off_);
 }
 return off_;
}}
