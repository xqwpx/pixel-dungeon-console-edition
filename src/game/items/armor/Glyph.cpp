#include "game/items/armor/Glyph.h"
#include "game/items/armor/Armor.h"
#include "game/actors/Char.h"
#include "game/actors/Hero.h"
#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Burning.h"
#include "game/actors/buffs/Charm.h"
#include "game/actors/buffs/Frost.h"
#include "game/actors/buffs/Roots.h"
#include "game/actors/buffs/Hunger.h"
#include "game/actors/buffs/Poison.h"
#include "game/actors/buffs/DeferredDamage.h"
#include "game/actors/buffs/HerbalArmor.h"
#include "game/actors/mobs/npcs/MirrorImage.h"
#include "game/Dungeon.h"
#include "game/actors/blobs/Blob.h"
#include "game/items/rings/Ring.h"
#include "game/levels/Level.h"
#include "game/utils/Random.h"
#include <algorithm>

namespace pdv { namespace {
class OriginalGlyph final : public Glyph {
public:
    explicit OriginalGlyph(GlyphKind kind): kind_(kind) {}
    GlyphKind kind() const override { return kind_; }
    std::unique_ptr<Glyph> clone() const override { return std::unique_ptr<Glyph>(new OriginalGlyph(kind_)); }
    std::string name(const std::string& n) const override {
        static const char* suffix[] = {"bounce","affection","anti-entropy","multiplicity","potential","metabolism","stench","viscosity","displacement","entanglement","self-repair"};
        return n + " of " + suffix[static_cast<int>(kind_)-1];
    }
    int proc(Armor& armor, Char& attacker, Char& defender, int damage) override {
        const int level = std::max(0, armor.effectiveLevel());
        switch (kind_) {
            case GlyphKind::Bounce:
                if(Level::adjacent(attacker.pos,defender.pos)&&Random::Int(level+5)>=4){Level*area=defender.levelContext();Dungeon*game=area?area->dungeonContext():nullptr;const int offset=attacker.pos-defender.pos;const int destination=attacker.pos+offset;if(area&&area->inBoundsCell(destination)&&(area->passable(destination)||area->avoid(destination))&&(!game||!game->scheduler().findChar(destination))){if(game)game->relocateChar(attacker,destination);else attacker.moveTo(destination,*area);}}
                break;
            case GlyphKind::Affection:
                if (Level::adjacent(attacker.pos, defender.pos) && Random::Int(std::min(level,6)/2+5)>=4) {
                    int duration=Random::IntRange(3,7); Charm* a=Buff::affect<Charm>(attacker,elementsDurationFactor(attacker)*duration); a->object=defender.id();
                    Charm* d=Buff::affect<Charm>(defender,elementsDurationFactor(defender)*duration*Random::Float(0.5f,1.0f)); d->object=attacker.id();
                } break;
            case GlyphKind::AntiEntropy:
                if (Level::adjacent(attacker.pos, defender.pos) && Random::Int(level+6)>=5) {
                    Buff::prolong<Frost>(attacker, Frost::duration(attacker)*Random::Float(1.0f,1.5f));
                    Buff::affect<Burning>(defender)->reignite(defender);
                } break;
            case GlyphKind::Potential:
                if (damage>0 && Level::adjacent(attacker.pos,defender.pos) && Random::Int(level+7)>=6) {
                    int reflected=Random::IntRange(1,damage); attacker.damage(reflected,this); defender.damage(Random::IntRange(1,reflected),this);
                } break;
            case GlyphKind::Metabolism:
                if (Random::Int(level/2+5)>=4) {
                    Hero* hero=dynamic_cast<Hero*>(&defender); Hunger* hunger=hero ? hero->hunger() : nullptr;
                    int heal=std::min(defender.HT-defender.HP, Random::Int(1,std::max(2,defender.HT/5)));
                    if (heal>0 && hunger && !hunger->isStarving()) { hunger->satisfy(-Hunger::Starving/10); defender.HP+=heal; }
                } break;
            case GlyphKind::Entanglement:
                if(Random::Int(4)==0){Buff::prolong<Roots>(defender,static_cast<float>(5-level/5));if(HerbalArmor*earth=Buff::affect<HerbalArmor>(defender))earth->level(5*(level+1));}
                break;
            case GlyphKind::Multiplicity:
                if(Random::Int(level/2+6)>=5){Hero*hero=dynamic_cast<Hero*>(&defender);Level*area=defender.levelContext();Dungeon*game=area?area->dungeonContext():nullptr;if(hero&&area&&game){std::vector<int>cells;for(int offset:Level::Neighbours8){const int p=defender.pos+offset;if(area->inBoundsCell(p)&&(area->passable(p)||area->avoid(p))&&!game->scheduler().findChar(p))cells.push_back(p);}if(!cells.empty()){std::unique_ptr<MirrorImage>image(new MirrorImage());MirrorImage*raw=image.get();image->duplicate(*hero);image->pos=cells[static_cast<std::size_t>(Random::Int(static_cast<int>(cells.size())))];image->bind(game);area->addMob(std::move(image));game->scheduler().add(raw);defender.damage(Random::IntRange(1,std::max(1,defender.HT/6)),this);}}}
                break;
            case GlyphKind::Stench:
                if(Level::adjacent(attacker.pos,defender.pos)&&Random::Int(level+5)>=4){if(Level*area=defender.levelContext())area->seedBlob(BlobKind::ToxicGas,attacker.pos,20);}
                break;
            case GlyphKind::Viscosity:
                if(damage>0&&Random::Int(level+7)>=6){if(DeferredDamage*deferred=Buff::affect<DeferredDamage>(defender))deferred->prolong(damage);damage=0;}
                break;
            case GlyphKind::Displacement: {
                Level*area=defender.levelContext();Dungeon*game=area?area->dungeonContext():nullptr;if(game&&game->depth()%5!=0){const int tries=(armor.effectiveLevel()<0?1:armor.effectiveLevel()+1)*5;for(int i=0;i<tries;++i){const int p=Random::Int(Level::Length);if(game->visible()[static_cast<std::size_t>(p)]&&area->passable(p)&&!game->scheduler().findChar(p)){game->relocateChar(defender,p);game->observe();break;}}}
                break;
            }
            case GlyphKind::AutoRepair:
                if(dynamic_cast<Hero*>(&defender)){Level*area=defender.levelContext();Dungeon*game=area?area->dungeonContext():nullptr;if(game&&game->spendGold(armor.tier()))armor.polish();}
                break;
        }
        return damage;
    }
private: GlyphKind kind_;
}; }
std::unique_ptr<Glyph> Glyph::create(GlyphKind kind){ return std::unique_ptr<Glyph>(new OriginalGlyph(kind)); }
std::unique_ptr<Glyph> Glyph::random(){ return create(static_cast<GlyphKind>(Random::IntRange(1,11))); }
} // namespace pdv
