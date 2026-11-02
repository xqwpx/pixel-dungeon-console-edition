#include "scenes/GameScene.h"
#include "core/StringUtils.h"

#include "assets/AssetCatalog.h"
#include "assets/TextureCache.h"
#include "audio/AudioSystem.h"
#include "core/SceneManager.h"
#include "core/Localization.h"
#include "game/Progression.h"
#include "game/actors/mobs/Mob.h"
#include "game/actors/mobs/Gnoll.h"
#include "game/actors/mobs/Crab.h"
#include "game/actors/mobs/Goo.h"
#include "game/actors/mobs/Mimic.h"
#include "game/actors/mobs/Swarm.h"
#include "game/actors/mobs/Albino.h"
#include "game/actors/mobs/Skeleton.h"
#include "game/actors/mobs/Thief.h"
#include "game/actors/mobs/Piranha.h"
#include "game/actors/mobs/ZoneMob.h"
#include "game/actors/mobs/npcs/DungeonNPC.h"
#include "game/items/KindOfWeapon.h"
#include "game/items/armor/Armor.h"
#include "game/items/Bag.h"
#include "game/items/food/Food.h"
#include "game/items/Heap.h"
#include "game/items/Item.h"
#include "game/items/SpecialItems.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/items/Generator.h"
#include "game/items/bags/Keyring.h"
#include "game/items/wands/Wand.h"
#include "game/items/rings/Ring.h"
#include "game/items/potions/Potion.h"
#include "game/items/scrolls/Scroll.h"
#include "game/items/weapon/missiles/MissileWeapon.h"
#include "game/actors/mobs/npcs/MirrorImage.h"
#include "game/actors/buffs/Blindness.h"
#include "game/items/weapon/missiles/Dart.h"
#include "game/items/weapon/missiles/Boomerang.h"
#include "game/items/EquipmentCatalog.h"
#include "game/items/ConsumableCatalog.h"
#include "game/ui/QuickSlot.h"
#include "game/ui/BuffIndicator.h"
#include "game/levels/Terrain.h"
#include "game/mechanics/Ballistica.h"
#include "engine/input/IInputBackend.h"
#include "render/BitmapFont.h"
#include "render/Renderer2D.h"
#include "ui/VitaButtonHints.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>


namespace pdv {
using VitaInput = IInputBackend;

namespace {
constexpr float AutoStep = 0.12f;

constexpr float InventoryX = 12.0f;
constexpr float InventoryY = 25.0f;
constexpr float InventoryWidth = 222.0f;
constexpr float InventoryHeight = 216.0f;
constexpr float InventoryDetailsX = 246.0f;
constexpr float InventoryDetailsY = 25.0f;
constexpr float InventoryDetailsWidth = 222.0f;
constexpr float InventoryDetailsHeight = 216.0f;
constexpr float InventoryGridWidth =
    InventoryModel::Columns * InventoryModel::SlotSize +
    (InventoryModel::Columns - 1) * InventoryModel::SlotMargin;
constexpr float InventoryGridX =
    InventoryX + (InventoryWidth - InventoryGridWidth) * 0.5f;
constexpr int InventoryRows = 6;

std::string inventoryPurpose(const Item& item) {
    if (const Potion* potion = dynamic_cast<const Potion*>(&item)) return potion->description();
    if (const Scroll* scroll = dynamic_cast<const Scroll*>(&item)) return scroll->description();
    if (dynamic_cast<const Wand*>(&item)) return Localization::text("Zap a target with its magic.");
    if (dynamic_cast<const Food*>(&item)) return Localization::text("Eat to restore hunger.");
    if (dynamic_cast<const Armor*>(&item)) return Localization::text("Equip to improve defense.");
    if (dynamic_cast<const Ring*>(&item)) return Localization::text("Equip to gain a passive effect.");
    if (dynamic_cast<const Bag*>(&item)) return Localization::text("Stores matching items.");
    if (dynamic_cast<const KindOfWeapon*>(&item)) return Localization::text("Equip or throw it in combat.");
    if (const SpecialItem* special=dynamic_cast<const SpecialItem*>(&item)) {
        switch (special->kind()) {
            case SpecialKind::DewVial: return Localization::text("Stores dew and can restore health.");
            case SpecialKind::Ankh: return Localization::text("Restores you after a fatal blow.");
            case SpecialKind::TomeOfMastery: return Localization::text("Choose a hero subclass.");
            case SpecialKind::ArmorKit: return Localization::text("Turns armor into class armor.");
            case SpecialKind::Weightstone: return Localization::text("Balances an equipped melee weapon.");
            case SpecialKind::Torch: return Localization::text("Adds light for a limited time.");
            default: break;
        }
    }
    return Localization::text("Use it from the inventory when needed.");
}

std::size_t nextUtf8Character(const std::string& text,std::size_t offset) {
    if(offset>=text.size()) return offset;
    const unsigned char first=static_cast<unsigned char>(text[offset]);
    std::size_t bytes=(first&0x80u)==0 ? 1 : ((first&0xE0u)==0xC0u ? 2 : ((first&0xF0u)==0xE0u ? 3 : 4));
    return std::min(text.size(),offset+bytes);
}

std::size_t utf8CharacterOffset(const std::string& text,int characters) {
    std::size_t offset=0;
    while(offset<text.size()&&characters-->0) offset=nextUtf8Character(text,offset);
    return offset;
}

std::size_t wrappedLineEnd(const std::string& text,std::size_t start,int characters) {
    std::size_t end=start;
    std::size_t lastSpace=std::string::npos;
    for(int count=0;end<text.size()&&count<characters;++count){
        if(text[end]==' ') lastSpace=end;
        end=nextUtf8Character(text,end);
    }
    return end<text.size()&&lastSpace!=std::string::npos&&lastSpace>start ? lastSpace : end;
}

std::string inventoryLabel(const std::string& text,int limit=24) {
    const std::size_t end=utf8CharacterOffset(text,limit);
    return end==text.size() ? text : text.substr(0,utf8CharacterOffset(text,limit-3))+"...";
}

int inventoryWrappedLineCount(const std::string& text,int characters) {
    int rows=0;
    std::size_t start=0;
    while(start<text.size()){
        const std::size_t end=wrappedLineEnd(text,start,characters);
        start=end;
        while(start<text.size()&&text[start]==' ')++start;
        ++rows;
    }
    return rows;
}

void drawInventoryWrapped(BitmapFont& font,Renderer2D& renderer,const std::string& text,float x,float y,int characters,float brightness=0.7f,int maxRows=7,int firstRow=0) {
    int row=0;
    int drawn=0;
    std::size_t start=0;
    while(start<text.size()&&drawn<maxRows){
        const std::size_t end=wrappedLineEnd(text,start,characters);
        if(row>=firstRow){
            font.draw(renderer,text.substr(start,end-start),x,y+drawn*9.0f,1,brightness,1);
            ++drawn;
        }
        start=end;
        while(start<text.size()&&text[start]==' ')++start;
        ++row;
    }
}
}

bool GameScene::initialize(
    TextureCache& textures,
    BitmapFont& font
) {
    font_ = &font;

    zoneTiles_={{textures.get(Assets::TilesSewers),textures.get(Assets::TilesPrison),textures.get(Assets::TilesCaves),textures.get(Assets::TilesCity),textures.get(Assets::TilesHalls)}};tiles_=zoneTiles_[0];
    for(std::size_t i=0;i<waterSheets_.size();++i)waterSheets_[i]=textures.get(Assets::WaterSheets[i]);
    for(std::size_t i=0;i<zoneMobSheets_.size();++i)zoneMobSheets_[i]=textures.get(Assets::ZoneMobSheets[i]);
    for(std::size_t i=0;i<zoneBossSheets_.size();++i)zoneBossSheets_[i]=textures.get(Assets::ZoneBossSheets[i]);
    for(std::size_t i=0;i<npcSheets_.size();++i)npcSheets_[i]=textures.get(Assets::NPCSheets[i]);
    chrome_ = textures.get(Assets::Chrome);
    ratSheet_ = textures.get(Assets::Rat);
    gnollSheet_ = textures.get(Assets::Gnoll);
    crabSheet_ = textures.get(Assets::Crab);
    gooSheet_ = textures.get(Assets::Goo);
    swarmSheet_ = textures.get(Assets::Swarm);
    skeletonSheet_ = textures.get(Assets::Skeleton);
    thiefSheet_ = textures.get(Assets::Thief);
    piranhaSheet_ = textures.get(Assets::Piranha);
    buffsSheet_ = textures.get(Assets::BuffsSmall);
    itemsSheet_ = textures.get(Assets::Items);
    effectsSheet_ = textures.get(Assets::Effects);
    specksSheet_ = textures.get(Assets::Specks);
    badgesSheet_ = textures.get(Assets::Badges);
    plantsSheet_ = textures.get(Assets::Plants);
    statusSheet_=textures.get(Assets::StatusPSVita);uiGameSheet_=textures.get(Assets::UIGame);levelUpSheet_=textures.get(Assets::LevelUp);vitaButtonsSheet_=textures.get(Assets::VitaButtons);bannersSheet_=textures.get(Assets::Banners);uiButtonsSheet_=textures.get(Assets::UIButtonsPSVita);

    heroSheets_[0] = textures.get(Assets::Warrior);
    heroSheets_[1] = textures.get(Assets::Mage);
    heroSheets_[2] = textures.get(Assets::Rogue);
    heroSheets_[3] = textures.get(Assets::Huntress);

    if (!tiles_ || !chrome_ || !ratSheet_ || !gnollSheet_ || !crabSheet_ || !gooSheet_ || !swarmSheet_ || !skeletonSheet_ || !thiefSheet_ || !piranhaSheet_ ||
        !buffsSheet_ || !itemsSheet_ || !effectsSheet_ || !specksSheet_ || !badgesSheet_ || !plantsSheet_ || !statusSheet_ || !uiGameSheet_ || !levelUpSheet_ || !vitaButtonsSheet_ || !bannersSheet_ || !uiButtonsSheet_) {
        return false;
    }

    for (auto* texture : heroSheets_) {
        if (!texture) {
            return false;
        }
    }
    for(auto* texture:zoneTiles_)if(!texture)return false;
    for(auto* texture:waterSheets_)if(!texture)return false;
    for(auto* texture:zoneMobSheets_)if(!texture)return false;
    for(auto* texture:zoneBossSheets_)if(!texture)return false;
    for(auto* texture:npcSheets_)if(!texture)return false;

    return true;
}

void GameScene::shutdown() {
    corpses_.clear();
    waterRipples_.clear();
    thrownItems_.clear();
    impactEffects_.clear();
    heroSpecks_.clear();
    itemPopTimers_.clear();
    doorRevealTimers_.clear();
    knownDoorCells_.clear();
    for (int cell = 0; cell < Level::Length; ++cell) {
        const int terrain = dungeon_.level().cell(cell);
        if (terrain == Terrain::DOOR || terrain == Terrain::OPEN_DOOR || terrain == Terrain::LOCKED_DOOR) knownDoorCells_[cell] = true;
    }
    tiles_ = nullptr;
    zoneTiles_.fill(nullptr);waterSheets_.fill(nullptr);zoneMobSheets_.fill(nullptr);zoneBossSheets_.fill(nullptr);npcSheets_.fill(nullptr);
    chrome_ = nullptr;
    ratSheet_ = nullptr;
    gnollSheet_ = nullptr;
    crabSheet_ = nullptr;
    gooSheet_ = nullptr;
    swarmSheet_ = nullptr;
    skeletonSheet_ = nullptr;
    thiefSheet_ = nullptr;
    piranhaSheet_ = nullptr;
    buffsSheet_ = nullptr;
    itemsSheet_ = nullptr;
    effectsSheet_ = nullptr;
    specksSheet_ = nullptr;
    badgesSheet_ = nullptr;
    plantsSheet_ = nullptr;
    statusSheet_=nullptr;uiGameSheet_=nullptr;levelUpSheet_=nullptr;vitaButtonsSheet_=nullptr;bannersSheet_=nullptr;uiButtonsSheet_=nullptr;
    heroSheets_.fill(nullptr);
    font_ = nullptr;
}

void GameScene::onEnter(GameSession& session) {
    bool beganNewRun = false;
    if (session.continueRequested) {
        if (!dungeon_.load()) {
            session.newGameRequested = true;
        }
        session.continueRequested = false;
    }
    if (session.newGameRequested || !dungeon_.started()) {
        const uint32_t seedTime = static_cast<uint32_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
        uint32_t seed = seedTime;

        seed ^= session.runSeed * 0x9E3779B9u;
        seed ^= static_cast<uint32_t>(
            static_cast<int>(session.selectedHero) + 1
        ) * 0x85EBCA6Bu;

        dungeon_.startNew(session.selectedHero, seed);
        dungeon_.save();
        session.newGameRequested = false;
        beganNewRun = true;
    }

    touchTarget_ = -1;
    corpses_.clear();
    waterRipples_.clear();
    heroSpecks_.clear();
    itemPopTimers_.clear();
    mobDeathTimer_ = 0.0f;
    mobDeathCell_ = -1;
    mobDeathSpeckFrame_ = -1;
    mobDeathSpecksShown_ = false;
    analogLatched_ = false;
    moveRepeatTimer_=0.0f;heldMoveDx_=heldMoveDy_=0;
    autoWalkTimer_ = 0.0f;

    heroAnimationTime_ = 0.0f;
    waterAnimationTime_ = 0.0f;
    heroIdleFrame_ = 0;
    heroAttackTimer_ = 0.0f;
    mobAttackTimer_ = 0.0f;
    hitEffectTimer_ = 0.0f;
    hitEffectCell_ = -1;
    searchEffectTimer_ = 0.0f;
    searchEffectCell_ = -1;
    seenMobAttackSerial_ = dungeon_.mobAttackSerial();
    attackingMobCell_ = -1;

    ratAnimationTime_ = 0.0f;
    ratIdleFrame_ = 0;

    status_ = "START inventory | Combat parity";
    statusTimer_ = 2.0f;
    lastLoggedStatus_.clear();lastLoggedTimer_=0.0f;messageLog_.clear();messageLogColors_.clear();captureLogMessage();

    pendingScroll_ = nullptr;
    pendingScrollWasKnown_ = true;
    pendingCancelConfirm_ = false;
    pendingTargetItem_ = nullptr;
    pendingQuickSlot_ = false;
    targetCursor_ = -1;
    pendingFallCell_ = -1;
    fallConfirmed_ = false;

    inventory_.bind(
        &dungeon_.hero()
    );
    inventory_.close();

    gameMenuOpen_ = false;
    settingsMenuOpen_ = false;
    skillMenuOpen_ = false;
    skillMenuSelection_ = 0;
    journalOpen_ = false;
    journalPage_ = 0;
    questRewardOpen_ = false;

    questRewardSelection_ = 0;
    blacksmithReforgeOpen_ = false;
    blacksmithFirstItem_ = nullptr;
    amuletChoiceOpen_ = false;
    amuletChoiceSelection_ = 0;
    npcDialogueOpen_ = false;
    npcDialogueTitle_.clear();
    npcDialogueText_.clear();
    interlevelTimer_ = 0.0f;
    interlevelDepth_ = dungeon_.depth();
    interlevelIsTeleport_ = false;
    introTimer_ = beganNewRun ? 3.0f : 0.0f;
    surfaceTimer_ = 0.0f;
    deathSummaryOpen_ = false;
    deathSummarySelection_ = 0;
    deathOnlineLoading_ = false;
    deathOnlineStatus_.clear();
    gameMenuSelection_ = 0;
    zoomIndex_ = AudioSystem::instance().cameraZoom();
    static const float zooms[4] = {0.75f, 1.0f, 1.25f, 1.5f};
    TileSize = 16.0f * zooms[zoomIndex_];
    const int heroPos = dungeon_.hero().pos;
    heroVisual_.x = static_cast<float>(heroPos % Level::Width);
    heroVisual_.y = static_cast<float>(heroPos / Level::Width);
    displayedHeroHp_ = dungeon_.hero().HP;
    displayedHeroLevel_ = dungeon_.hero().level();
    displayedGold_ = dungeon_.gold();
    badgeNotificationRemaining_ = 0.0f;
    mobVisuals_.clear();
    for (const auto& mob : dungeon_.level().mobs()) {
        if (!mob) continue;
        mobVisuals_[mob->id()] = VisualPosition{
            static_cast<float>(mob->pos % Level::Width),
            static_cast<float>(mob->pos / Level::Width)
        };
    }

    AudioSystem::instance().playMusic("game.mp3");

    updateCamera();
}

void GameScene::onExit() {
    deathOnlineRequest_.cancel();
    deathOnlineLoading_ = false;
    dungeon_.save();
    touchTarget_ = -1;
    inventory_.close();
    pendingScroll_ = nullptr;
    pendingCancelConfirm_ = false;
    pendingTargetItem_ = nullptr;
    pendingQuickSlot_ = false;
    targetCursor_ = -1;
}

void GameScene::update(
    float dt,
    const VitaInput& input,
    SceneManager& scenes,
    GameSession&
) {
    if (statusTimer_ > 0.0f) {
        statusTimer_ -= dt;
    }
    const std::string dungeonMessage=dungeon_.takeMessage();if(!dungeonMessage.empty()){status_=dungeonMessage;statusTimer_=4.0f;}
    captureLogMessage();

    OnlineLeaderboardResult result;
    if (deathOnlineLoading_ && deathOnlineRequest_.poll(result)) {
        deathOnlineLoading_ = false;
        deathOnlineStatus_ = result.ok ? "Record sent to online leaderboard" : result.message;
    }

    if (introTimer_ > 0.0f) {
        introTimer_ = std::max(0.0f, introTimer_ - dt);
        updateAnimations(dt);
        updateVisualPositions(dt);
        updateCamera();
        return;
    }

    if (interlevelTimer_ > 0.0f) {
        interlevelTimer_ = std::max(0.0f, interlevelTimer_ - dt);
        updateAnimations(dt);
        updateVisualPositions(dt);
        updateCamera();
        return;
    }

    if (amuletChoiceOpen_) {
        handleAmuletChoice(input);
        updateAnimations(dt);
        updateVisualPositions(dt);
        updateCamera();
        return;
    }

    if (npcDialogueOpen_) {
        handleNpcDialogue(input);
        updateAnimations(dt);
        updateVisualPositions(dt);
        updateCamera();
        return;
    }

    if (surfaceTimer_ > 0.0f) {
        surfaceTimer_ = std::max(0.0f, surfaceTimer_ - dt);
        updateAnimations(dt);
        updateVisualPositions(dt);
        updateCamera();
        return;
    }

    if (dungeon_.won()) {
        if (input.pressed(SCE_CTRL_START) || input.pressed(SCE_CTRL_CROSS)) scenes.request(SceneId::Start);
        updateAnimations(dt);
        updateVisualPositions(dt);
        updateCamera();
        return;
    }

    if (!dungeon_.hero().isAlive()) {
        if (!deathSummaryOpen_) {
            deathSummaryOpen_ = true;
            deathSummarySelection_ = 0;
            deathOnlineStatus_.clear();
            dungeon_.save();
        }
        handleDeathSummary(input, scenes);
        updateAnimations(dt);
        updateVisualPositions(dt);
        updateCamera();
        return;
    }

    if (dungeon_.questState(2) == 650 && !blacksmithReforgeOpen_) {
        blacksmithReforgeOpen_ = true;
        blacksmithFirstItem_ = nullptr;
        inventory_.bind(&dungeon_.hero());
        inventory_.open(InventoryMode::All, "Blacksmith: choose first item");
    }

    if (dungeon_.pendingQuestReward() >= 0) {
        questRewardOpen_ = true;
        handleQuestReward(input);
        updateAnimations(dt);
        updateVisualPositions(dt);
        updateCamera();
        return;
    }
    questRewardOpen_ = false;

    if (pendingFallCell_ >= 0) {
        if (input.pressed(SCE_CTRL_CIRCLE)) {
            pendingFallCell_ = -1;
            return;
        }
        if (input.pressed(SCE_CTRL_CROSS)) {
            const int heroPos = dungeon_.hero().pos;
            if (Level::adjacent(heroPos, pendingFallCell_) && dungeon_.level().pit(pendingFallCell_)) {
                const int oldDepth = dungeon_.depth();
                fallConfirmed_ = true;
                manualMove(pendingFallCell_ % Level::Width - heroPos % Level::Width,
                           pendingFallCell_ / Level::Width - heroPos / Level::Width);
                fallConfirmed_ = false;
                if (dungeon_.depth() != oldDepth) {
                    resetLevelVisuals();
                    AudioSystem::instance().playSound("snd_descend.mp3");
                    status_ = "You fall to a lower floor";
                    statusTimer_ = 1.2f;
                    interlevelDepth_ = dungeon_.depth();
                    interlevelIsTeleport_ = false;
                    interlevelTimer_ = 3.0f;
                }
            }
            pendingFallCell_ = -1;
        }
        updateAnimations(dt); updateVisualPositions(dt); updateCamera(); return;
    }

    if (cheatMenuOpen_) {
        handleCheatMenu(input);
        updateAnimations(dt);
        updateVisualPositions(dt);
        updateCamera();
        return;
    }

    if (input.pressed(SCE_CTRL_START)) {
        if(journalOpen_)journalOpen_=false;
        else {gameMenuOpen_ = !gameMenuOpen_;gameMenuSelection_ = 0;}
        AudioSystem::instance().playSound("snd_click.mp3");
    }

    if (gameMenuOpen_) {
        handleGameMenu(input, scenes);
        updateAnimations(dt);
        updateVisualPositions(dt);
        updateCamera();
        return;
    }

    if(skillMenuOpen_){
        handleSkillMenu(input);
        updateAnimations(dt);updateVisualPositions(dt);updateCamera();return;
    }

    if (pendingTargetItem_ || pendingQuickSlot_) {
        if (input.pressed(SCE_CTRL_CIRCLE)) {
            pendingTargetItem_ = nullptr;
            pendingTargetAction_ = ItemAction::Use;
            pendingQuickSlot_ = false;
            targetCursor_ = -1;
            status_ = "Targeting cancelled";
            return;
        }
        int dx = input.pressed(SCE_CTRL_LEFT) ? -1 : (input.pressed(SCE_CTRL_RIGHT) ? 1 : 0);
        int dy = input.pressed(SCE_CTRL_UP) ? -1 : (input.pressed(SCE_CTRL_DOWN) ? 1 : 0);
        if ((dx || dy) && targetCursor_ >= 0) {
            const int x = targetCursor_ % Level::Width + dx;
            const int y = targetCursor_ / Level::Width + dy;
            if (dungeon_.level().inBounds(x, y)) targetCursor_ = y * Level::Width + x;
        }
        if (input.pressed(SCE_CTRL_CROSS)) {
            const int target = targetCursor_;
            const int from = dungeon_.hero().pos;
            const bool quickSlot = pendingQuickSlot_;
            Item* item = pendingTargetItem_;
            const ItemAction action = pendingTargetAction_;
            Item* quickItem = quickSlot ? dungeon_.hero().primaryQuickSlot().select(dungeon_.hero()) : nullptr;
            const int itemImage = quickSlot ? (quickItem ? quickItem->image() : 0) : (item ? item->image() : 0);
            Item* effectItem = quickSlot ? quickItem : item;
            const CatalogWand* effectWand = dynamic_cast<const CatalogWand*>(effectItem);
            const bool firebolt = effectWand && effectWand->kind() == WandKind::Firebolt;
            const bool potionImpact = dynamic_cast<const Potion*>(effectItem) != nullptr;
            const CatalogMisc* effectMisc = dynamic_cast<const CatalogMisc*>(effectItem);
            const bool bombImpact = effectMisc && effectMisc->kind() == MiscKind::Bomb;
            const bool thrown = (quickSlot && !dynamic_cast<Wand*>(effectItem)) || action == ItemAction::Throw;
            const int impact = thrown ? Ballistica::cast(dungeon_, from, target, false, true) : target;
            const bool deathCaptured = dungeon_.mobAt(impact) && captureMobDeathVisual(*dungeon_.mobAt(impact));
            pendingTargetItem_ = nullptr;
            pendingTargetAction_ = ItemAction::Use;
            pendingQuickSlot_ = false;
            targetCursor_ = -1;
            const bool acted = quickSlot ? dungeon_.usePrimaryQuickSlot(target) : dungeon_.executeItemAction(item, action, target);
            if (acted && (quickSlot || action == ItemAction::Throw)) showThrownItem(itemImage, from, impact);
            if (acted && deathCaptured) {
                if (!dungeon_.mobAt(impact)) mobDeathTimer_ = 0.5f;
                else mobDeathCell_ = -1;
            }
            if (acted) {
                if (firebolt) showImpactEffect(impact, 0xFF7B19FF, 1.35f, 0.55f);
                else if (potionImpact) showImpactEffect(impact, 0x78D9FFFF, 1.10f, 0.42f);
                else if (bombImpact) showImpactEffect(impact, 0xFFB020FF, 2.25f, 0.65f);
            }
            status_ = acted ? (quickSlot ? "Quickslot used" : std::string(itemActionName(action))) : "Action failed";
            statusTimer_ = 0.9f;
            if (acted) AudioSystem::instance().playSound("snd_item.mp3");
            captureLogMessage();
        }
        updateAnimations(dt); updateVisualPositions(dt); updateCamera(); return;
    }

    if (inventory_.openState()) {
        handleInventoryInput(input);
        updateAnimations(dt);
        return;
    }

    if (input.pressed(SCE_CTRL_TRIANGLE) &&
        dungeon_.hero().isAlive()) {
        openInventory();
        return;
    }

    if (dungeon_.hero().isAlive()) {
        if (input.pressed(SCE_CTRL_SELECT) && dungeon_.hero().upgradePoints() > 0) {
            skillMenuOpen_ = true;
            skillMenuSelection_ = 0;
            AudioSystem::instance().playSound("snd_click.mp3");
            return;
        }
        if (input.pressed(SCE_CTRL_CIRCLE)) {
            const int found = dungeon_.searchAdjacent();
            if (found > 0) revealNewDoors();
            spawnHeroSpecks(found > 0 ? 2 : 3, found > 0 ? 3 : 1);
            searchEffectTimer_=0.65f;searchEffectCell_=dungeon_.hero().pos;
            AudioSystem::instance().playSound(found > 0 ? "snd_secret.mp3" : "snd_click.mp3");
            status_ = found > 0 ? "Hidden feature found" : "You search nearby";
            statusTimer_ = 0.9f;
        }
        if (input.pressed(SCE_CTRL_CROSS)) {
            if (dungeon_.waitTurn()) { status_="You wait a turn";statusTimer_=1.0f;captureLogMessage(); }
        }

        if (input.pressed(SCE_CTRL_SQUARE)) {
            if (dungeon_.hero().pos == dungeon_.level().exit()) {
                descendFromExit();
                updateAnimations(dt);
                updateVisualPositions(dt);
                updateCamera();
                return;
            } else if (dungeon_.hero().pos == dungeon_.level().entrance() && dungeon_.depth() > 1) {
                ascendFromEntrance();
                updateAnimations(dt);
                updateVisualPositions(dt);
                updateCamera();
                return;
            } else if (dungeon_.pickUpAtHero()) {
                AudioSystem::instance().playSound("snd_item.mp3");
                status_ = "Picked up item";
                statusTimer_ = 0.8f;
                if(dungeon_.consumeAmuletPickup()){amuletChoiceOpen_=true;amuletChoiceSelection_=0;}
            } else {
                status_ = "Nothing to pick up";
                statusTimer_ = 0.6f;
                spawnHeroSpecks(3, 1);
            }
        }

        if (input.pressed(SCE_CTRL_RTRIGGER)) {
            Item* quickItem = dungeon_.hero().primaryQuickSlot().select(dungeon_.hero());
            if (quickItem) {
                pendingQuickSlot_ = true;
                targetCursor_ = touchTarget_ >= 0 ? touchTarget_ : dungeon_.firstVisibleMobCell();
                if (targetCursor_ < 0) targetCursor_ = dungeon_.hero().pos;
                status_ = "Choose quickslot target";
                statusTimer_ = 1.2f;
            } else {
                status_ = "No quickslot item";
                statusTimer_ = 0.7f;
            }
        }

        handleButtons(input);
        handleAnalog(input);
        updateHeldMovement(dt,input);
        handleTouch(input, scenes);
        updateAutoAction(dt);
    } else {
        touchTarget_ = -1;
        status_ = "You died";
        statusTimer_ = 0.25f;
        if(input.pressed(SCE_CTRL_START))scenes.request(SceneId::Start);
    }

    updateAnimations(dt);
    updateVisualPositions(dt);
    updateCamera();

}

bool GameScene::descendFromExit() {
    if (!dungeon_.descend()) { status_ = "The way down is blocked"; statusTimer_ = 0.8f; return false; }
    AudioSystem::instance().playSound("snd_descend.mp3");
    resetLevelVisuals();
    interlevelDepth_ = dungeon_.depth();
    interlevelIsTeleport_ = false;
    interlevelTimer_ = 3.0f;
    return true;
}

bool GameScene::ascendFromEntrance() {
    if (!dungeon_.ascend()) { status_ = "The way up is blocked"; statusTimer_ = 0.8f; return false; }
    AudioSystem::instance().playSound("snd_descend.mp3");
    resetLevelVisuals();
    interlevelDepth_ = dungeon_.depth();
    interlevelIsTeleport_ = false;
    interlevelTimer_ = 1.2f;
    status_ = "Ascended to floor " + numberToString(dungeon_.depth());
    statusTimer_ = 1.0f;
    return true;
}

void GameScene::resetLevelVisuals() {
    touchTarget_ = -1;
    corpses_.clear();
    waterRipples_.clear();
    thrownItems_.clear();
    impactEffects_.clear();
    itemPopTimers_.clear();
    doorRevealTimers_.clear();
    knownDoorCells_.clear();
    mobDeathTimer_ = 0.0f;
    mobDeathCell_ = -1;
    hitEffectTimer_ = 0.0f;
    hitEffectCell_ = -1;
    mobVisuals_.clear();
    const int heroPos = dungeon_.hero().pos;
    heroVisual_ = VisualPosition(static_cast<float>(heroPos % Level::Width), static_cast<float>(heroPos / Level::Width));
    updateCamera();
}

void GameScene::revealNewDoors() {
    for (int cell = 0; cell < Level::Length; ++cell) {
        const int terrain = dungeon_.level().cell(cell);
        if (terrain != Terrain::DOOR) continue;
        if (knownDoorCells_.find(cell) == knownDoorCells_.end()) doorRevealTimers_[cell] = 0.65f;
        knownDoorCells_[cell] = true;
    }
}


void GameScene::openInventory() {
    touchTarget_ = -1;
    autoWalkTimer_ = 0.0f;

    pendingScroll_ = nullptr;
    pendingScrollWasKnown_ = true;
    pendingCancelConfirm_ = false;
    inventoryDescriptionItem_ = nullptr;
    inventoryDescriptionScroll_ = 0;

    inventory_.bind(
        &dungeon_.hero()
    );

    inventory_.open(
        InventoryMode::All,
        "Backpack"
    );

    analogLatched_ = false;
}

void GameScene::closeInventory() {
    inventory_.close();

    pendingScroll_ = nullptr;
    pendingScrollWasKnown_ = true;
    pendingCancelConfirm_ = false;
    inventoryDescriptionItem_ = nullptr;
    inventoryDescriptionScroll_ = 0;

    analogLatched_ = false;
}

void GameScene::beginScrollSelection(
    Scroll* scroll
) {
    if (!scroll) {
        return;
    }

    pendingScroll_ = scroll;
    pendingScrollWasKnown_ =
        scroll->isKnown();

    // Original InventoryScroll reveals an unknown scroll before opening
    // the item-selection WndBag.
    scroll->setKnown();

    InventoryMode mode =
        InventoryMode::All;

    switch (scroll->kind()) {
        case ScrollKind::Identify:
            mode =
                InventoryMode::
                    Unidentified;
            break;

        case ScrollKind::Upgrade:
            mode =
                InventoryMode::
                    Upgradable;
            break;

        case ScrollKind::Enchantment:
            mode =
                InventoryMode::
                    Enchantable;
            break;

        default:
            break;
    }

    inventory_.setMode(
        mode,
        scroll->selectionTitle()
    );

    pendingCancelConfirm_ = false;
    inventoryDescriptionItem_ = nullptr;
    inventoryDescriptionScroll_ = 0;
}

int GameScene::inventorySlotAt(
    float x,
    float y
) const {
    const auto& slots =
        inventory_.slots();

    for (int i = 0;
         i <
            static_cast<int>(
                slots.size()
            );
         ++i) {

        const InventorySlot& slot =
            slots[
                static_cast<std::size_t>(
                    i
                )
            ];

        const float sx =
            InventoryGridX +
            slot.col *
            (
                InventoryModel::SlotSize +
                InventoryModel::SlotMargin
            );

        const float sy =
            InventoryY +
            InventoryModel::TitleHeight +
            slot.row *
            (
                InventoryModel::SlotSize +
                InventoryModel::SlotMargin
            );

        if (x >= sx &&
            y >= sy &&
            x < sx +
                InventoryModel::SlotSize &&
            y < sy +
                InventoryModel::SlotSize) {

            return i;
        }
    }

    return -1;
}

void GameScene::handleInventoryInput(
    const VitaInput& input
) {
    if (blacksmithReforgeOpen_) {
        handleBlacksmithReforge(input);
        return;
    }
    if (pendingCancelConfirm_) {
        if (input.pressed(
                SCE_CTRL_CIRCLE
            )) {

            if (pendingScroll_) {
                dungeon_
                    .consumeScrollOnCancel(
                        pendingScroll_
                    );
            }

            status_ =
                "Scroll consumed";
            statusTimer_ = 0.8f;

            closeInventory();
            return;
        }

        if (input.pressed(
                SCE_CTRL_CROSS
            )) {

            pendingCancelConfirm_ =
                false;

            status_ =
                "Selection resumed";
            statusTimer_ = 0.6f;
        }

        return;
    }

    if (input.pressed(
            SCE_CTRL_CIRCLE
        ) ||
        input.pressed(
            SCE_CTRL_START
        )) {

        if (inventory_.contextOpen()) {
            inventory_.closeContext();
        } else if (pendingScroll_ &&
            !pendingScrollWasKnown_) {

            pendingCancelConfirm_ =
                true;

            status_ =
                "Circle again: consume | Cross: return";
            statusTimer_ = 2.0f;

        } else {
            closeInventory();
        }

        return;
    }

    if (input.pressed(SCE_CTRL_LTRIGGER)) {
        inventoryDescriptionScroll_ = std::max(0, inventoryDescriptionScroll_ - 3);
    }

    if (input.pressed(SCE_CTRL_RTRIGGER)) {
        inventoryDescriptionScroll_ += 3;
    }

    if (input.pressed(
            SCE_CTRL_LEFT
        )) {

        inventory_
            .moveSelection(
                -1,
                0
            );
    }

    if (input.pressed(
            SCE_CTRL_RIGHT
        )) {

        inventory_
            .moveSelection(
                1,
                0
            );
    }

    if (input.pressed(
            SCE_CTRL_UP
        )) {
        if (inventory_.contextOpen()) inventory_.moveContextSelection(-1); else inventory_
            .moveSelection(
                0,
                -1
            );
    }

    if (input.pressed(
            SCE_CTRL_DOWN
        )) {
        if (inventory_.contextOpen()) inventory_.moveContextSelection(1); else inventory_
            .moveSelection(
                0,
                1
            );
    }

    const float ax =
        input.leftX();

    const float ay =
        input.leftY();

    const float magnitude =
        std::max(
            std::fabs(ax),
            std::fabs(ay)
        );

    if (!analogLatched_ &&
        magnitude >= 0.65f) {

        if (std::fabs(ax) >
            std::fabs(ay)) {

            inventory_
                .moveSelection(
                    ax < 0.0f
                    ? -1
                    : 1,
                    0
                );

        } else {
            inventory_
                .moveSelection(
                    0,
                    ay < 0.0f
                    ? -1
                    : 1
                );
        }

        analogLatched_ = true;

    } else if (
        analogLatched_ &&
        magnitude < 0.30f
    ) {
        analogLatched_ = false;
    }

    if (input.pressed(
            SCE_CTRL_CROSS
        )) {

        activateInventorySelection();
        return;
    }

    for (const auto& touch :
         input.touches()) {

        if (!touch.active ||
            !touch.pressed) {
            continue;
        }

        const float x =
            touch.x *
            0.5f;

        const float y =
            touch.y *
            0.5f;

        const int slot =
            inventorySlotAt(
                x,
                y
            );

        if (slot >= 0) {
            inventory_
                .selectIndex(slot);

            activateInventorySelection();
            return;
        }

        if (x >= InventoryDetailsX &&
            x < InventoryDetailsX + InventoryDetailsWidth &&
            y >= InventoryDetailsY + 76.0f &&
            y < InventoryDetailsY + InventoryDetailsHeight) {
            if (y < InventoryDetailsY + InventoryDetailsHeight * 0.5f) {
                inventoryDescriptionScroll_ = std::max(0, inventoryDescriptionScroll_ - 3);
            } else {
                inventoryDescriptionScroll_ += 3;
            }
            return;
        }
    }
}

void GameScene::handleBlacksmithReforge(const VitaInput& input) {
    if (input.pressed(SCE_CTRL_CIRCLE) || input.pressed(SCE_CTRL_START)) {
        if (blacksmithFirstItem_) {
            blacksmithFirstItem_ = nullptr;
            inventory_.open(InventoryMode::All, "Blacksmith: choose first item");
        } else {
            blacksmithReforgeOpen_ = false;
            closeInventory();
        }
        return;
    }
    if (input.pressed(SCE_CTRL_LTRIGGER)) inventoryDescriptionScroll_=std::max(0,inventoryDescriptionScroll_-3);
    if (input.pressed(SCE_CTRL_RTRIGGER)) inventoryDescriptionScroll_+=3;
    if (input.pressed(SCE_CTRL_LEFT)) inventory_.moveSelection(-1, 0);
    if (input.pressed(SCE_CTRL_RIGHT)) inventory_.moveSelection(1, 0);
    if (input.pressed(SCE_CTRL_UP)) inventory_.moveSelection(0, -1);
    if (input.pressed(SCE_CTRL_DOWN)) inventory_.moveSelection(0, 1);
    const float ax = input.leftX();
    const float ay = input.leftY();
    const float magnitude = std::max(std::fabs(ax), std::fabs(ay));
    if (!analogLatched_ && magnitude >= 0.65f) {
        if (std::fabs(ax) > std::fabs(ay)) inventory_.moveSelection(ax < 0.0f ? -1 : 1, 0);
        else inventory_.moveSelection(0, ay < 0.0f ? -1 : 1);
        analogLatched_ = true;
    } else if (analogLatched_ && magnitude < 0.30f) {
        analogLatched_ = false;
    }
    bool confirm = input.pressed(SCE_CTRL_CROSS);
    for (const auto& touch : input.touches()) {
        if (!touch.active || !touch.pressed) continue;
        const int slot = inventorySlotAt(touch.x * 0.5f, touch.y * 0.5f);
        if (slot >= 0) { inventory_.selectIndex(slot); confirm = true; break; }
    }
    if (!confirm) return;

    Item* selected = inventory_.selectedItem();
    if (!selected || !dungeon_.canReforge(*selected)) {
        status_ = "Choose an identified, uncursed upgradable item";
        statusTimer_ = 1.5f;
        return;
    }
    if (!blacksmithFirstItem_) {
        blacksmithFirstItem_ = selected;
        inventory_.open(InventoryMode::All, "Blacksmith: choose matching item");
        status_ = "Now choose a matching second item";
        statusTimer_ = 1.2f;
        return;
    }
    if (dungeon_.reforge(blacksmithFirstItem_, selected)) {
        blacksmithReforgeOpen_ = false;
        blacksmithFirstItem_ = nullptr;
        closeInventory();
        status_ = "Item reforged";
        statusTimer_ = 1.2f;
    } else {
        status_ = "Items must be different and of the same type";
        statusTimer_ = 1.5f;
    }
}

void GameScene::activateInventorySelection() {
    Item* item =
        inventory_
            .selectedItem();

    if (!item) {
        return;
    }

    if (pendingScroll_) {
        const ScrollKind scrollKind = pendingScroll_->kind();
        const std::string scrollName =
            pendingScroll_
                ->displayName();

        if (dungeon_
                .readScroll(
                    pendingScroll_,
                    item
                )) {

            status_ =
                scrollName +
                " read";

            statusTimer_ = 1.0f;
            AudioSystem::instance().playSound("snd_read.mp3");
            spawnHeroSpecks(scrollKind == ScrollKind::Upgrade ? 1 : 2, 4);
            closeInventory();

        } else {
            status_ =
                "Item cannot be selected";
            statusTimer_ = 0.8f;
        }

        return;
    }

    if (!inventory_.contextOpen()) {
        inventory_.openContext();
        return;
    }

    const ItemAction action=inventory_.selectedAction();
    const std::string itemName=item->displayName();
    const Scroll* selectedScroll = dynamic_cast<const Scroll*>(item);
    const ScrollKind selectedScrollKind = selectedScroll ? selectedScroll->kind() : ScrollKind::Identify;
    if (action == ItemAction::Use) {
        SpecialItem* special = dynamic_cast<SpecialItem*>(item);
        if (special && special->kind() == SpecialKind::Amulet) {
            amuletChoiceOpen_ = true;
            amuletChoiceSelection_ = 0;
            closeInventory();
            return;
        }
    }
    bool acted=false;
    if (action==ItemAction::Eat || action==ItemAction::Drink || action==ItemAction::Read ||
        action==ItemAction::Equip || action==ItemAction::Unequip || action==ItemAction::Drop ||
        action==ItemAction::QuickSlot || (action>=ItemAction::Gladiator&&action<=ItemAction::Warden)||action==ItemAction::BalanceSpeed||action==ItemAction::BalanceAccuracy||action==ItemAction::Mine||action==ItemAction::Shatter) {
        if (action==ItemAction::Read) {
            Scroll* scroll=dynamic_cast<Scroll*>(item);
            if (scroll && scroll->requiresInventoryTarget()) { inventory_.closeContext(); beginScrollSelection(scroll); return; }
        }
        acted=dungeon_.executeItemAction(item,action);
    } else if (action==ItemAction::Throw || action==ItemAction::Zap||action==ItemAction::HeroicLeap||action==ItemAction::SmokeBomb) {
        status_="Select a target from the dungeon"; statusTimer_=1.2f;
        pendingTargetItem_=item;pendingTargetAction_=action;
        targetCursor_=touchTarget_>=0?touchTarget_:dungeon_.firstVisibleMobCell();
        if(targetCursor_<0)targetCursor_=dungeon_.hero().pos;
        closeInventory();return;
    } else if(action==ItemAction::MoltenEarth||action==ItemAction::SpectralBlades){
        acted=dungeon_.executeItemAction(item,action);
    }
    if (acted) {
        if (action == ItemAction::Read && selectedScroll != nullptr) {
            int speck = 1;
            switch (selectedScrollKind) {
                case ScrollKind::Teleportation: speck = 10; break;
                case ScrollKind::RemoveCurse: speck = 13; break;
                case ScrollKind::Recharging: case ScrollKind::MagicMapping: case ScrollKind::Enchantment: speck = 2; break;
                case ScrollKind::Challenge: case ScrollKind::Terror: case ScrollKind::PsionicBlast: case ScrollKind::WipeOut: speck = 5; break;
                case ScrollKind::Lullaby: speck = 9; break;
                case ScrollKind::MirrorImage: speck = 11; break;
                default: break;
            }
            spawnHeroSpecks(speck, speck == 5 ? 4 : 3);
        }
        const char* sound = "snd_item.mp3";
        if (action == ItemAction::Eat) sound = "snd_eat.mp3";
        else if (action == ItemAction::Drink) sound = "snd_drink.mp3";
        else if (action == ItemAction::Read) sound = "snd_read.mp3";
        else if (action == ItemAction::Equip || action == ItemAction::Unequip) sound = "snd_meld.mp3";
        AudioSystem::instance().playSound(sound);
        status_=std::string(itemActionName(action))+" " + itemName;
        statusTimer_=0.8f;
        closeInventory();
    }
    else { status_="Action failed";statusTimer_=0.8f;inventory_.closeContext(); }
    return;

    if (Food* food =
            dynamic_cast<Food*>(
                item
            )) {

        if (dungeon_.eatFood(food)) {
            status_ =
                "Ate " +
                food->displayName();

            statusTimer_ = 0.8f;
            closeInventory();
        }

        return;
    }

    if (Potion* potion =
            dynamic_cast<Potion*>(
                item
            )) {

        const std::string name =
            potion->displayName();

        if (dungeon_
                .drinkPotion(
                    potion
                )) {

            status_ =
                "Drank " +
                name;

            statusTimer_ = 1.0f;
            closeInventory();
        }

        return;
    }

    if (Scroll* scroll =
            dynamic_cast<Scroll*>(
                item
            )) {

        if (scroll
                ->requiresInventoryTarget()) {

            beginScrollSelection(
                scroll
            );

            return;
        }

        const std::string name =
            scroll->displayName();

        if (dungeon_
                .readScroll(
                    scroll
                )) {

            status_ =
                name +
                " read";

            statusTimer_ = 1.0f;
            closeInventory();

        } else {
            status_ =
                dungeon_
                    .hero()
                    .hasBuff(
                        typeid(Blindness)
                    )
                ? "You can't read while blinded"
                : "Scroll failed";

            statusTimer_ = 0.8f;
        }

        return;
    }

    if (dungeon_
            .assignPrimaryQuickSlot(
                item
            )) {

        status_ =
            item->displayName() +
            " -> quickslot";

        statusTimer_ = 0.8f;
        closeInventory();
        return;
    }

    status_ =
        item->displayName();

    statusTimer_ = 1.0f;
}

void GameScene::updateCombatStatus(
    int heroHpBefore,
    std::size_t mobsBefore
) {
    bool changed=false;
    const int heroHpAfter = std::max(0, dungeon_.hero().HP);
    const std::size_t mobsAfter = dungeon_.level().mobs().size();

    if (mobsAfter < mobsBefore) {
        AudioSystem::instance().playSound("snd_hit.mp3");
    }

    if (heroHpAfter < heroHpBefore) {
        mobAttackTimer_ = 0.34f;
        hitEffectTimer_ = 0.8f;
        hitEffectCell_ = dungeon_.hero().pos;
        AudioSystem::instance().playSound("snd_hit.mp3");
        char buffer[96];
        snprintf(
            buffer,
            sizeof(buffer),
            "Rat hit you for %d",
            heroHpBefore - heroHpAfter
        );
        status_ = buffer;
        statusTimer_ = 1.0f;
        changed=true;
    }

    if (!dungeon_.hero().isAlive()) {
        AudioSystem::instance().playSound("snd_death.mp3");
        status_ = "You died";
        statusTimer_ = 2.0f;
        changed=true;
    }
    if(changed){lastLoggedStatus_.clear();captureLogMessage();}
}

bool GameScene::manualMove(int dx, int dy) {
    touchTarget_ = -1;
    autoWalkTimer_ = 0.0f;

    const int heroHpBefore = dungeon_.hero().HP;
    const std::size_t mobsBefore = dungeon_.level().mobs().size();
    const int depthBefore = dungeon_.depth();

    const int heroPos = dungeon_.hero().pos;
    const int x = heroPos % Level::Width;
    const int y = heroPos / Level::Width;

    const int targetX = x + dx;
    const int targetY = y + dy;

    int targetCell = -1;
    bool hadTargetMob = false;
    bool interactingNpc = false;
    std::string npcName;
    bool openingChest = false;
    bool openingLockedDoor = false;
    bool capturedDeathVisual = false;

    if (dungeon_.level().inBounds(targetX, targetY)) {
        targetCell = targetY * Level::Width + targetX;
        hadTargetMob = dungeon_.mobAt(targetCell) != nullptr;
        if (const Mob* targetMob = dungeon_.mobAt(targetCell)) {
            interactingNpc = dynamic_cast<const DungeonNPC*>(targetMob) != nullptr && !targetMob->hostile();
            if (interactingNpc) npcName = Localization::text(targetMob->name);
            if (!interactingNpc) capturedDeathVisual = captureMobDeathVisual(*targetMob);
        }
        if (const Heap* heap = dungeon_.level().heapAt(targetCell)) {
            openingChest = heap->type == Heap::Type::Chest || heap->type == Heap::Type::LockedChest ||
                heap->type == Heap::Type::CrystalChest || heap->type == Heap::Type::Mimic;
        }
        const int terrain = dungeon_.level().cell(targetCell);
        openingLockedDoor = terrain == Terrain::LOCKED_DOOR || terrain == Terrain::LOCKED_EXIT;
        if (!fallConfirmed_ && dungeon_.level().pit(targetCell) && !dungeon_.hero().flying) {
            pendingFallCell_ = targetCell;
            return true;
        }
    }

    if (openingChest) {
        if (dungeon_.openChestAt(targetCell)) {
            heroOperateTimer_ = 0.5f;
            if (const Heap* opened = dungeon_.level().heapAt(targetCell);
                opened && opened->type == Heap::Type::Heap && !opened->empty()) {
                triggerItemPop(targetCell);
            }
            AudioSystem::instance().playSound("snd_unlock.mp3");
            status_ = "Chest opened";
            statusTimer_ = 0.8f;
        } else {
            AudioSystem::instance().playSound("snd_click.mp3", 0.5f);
            status_ = "Chest is locked";
            statusTimer_ = 0.7f;
        }
        captureLogMessage();
        return true;
    }

    const bool acted = dungeon_.moveHeroBy(dx, dy);

    if (!acted) {
        if (capturedDeathVisual) mobDeathCell_ = -1;
        status_ = "Blocked";
        statusTimer_ = 0.55f;
        captureLogMessage();
        return false;
    }

    // Falling replaces the whole floor.  Remaining movement effects refer to
    // old cells and must not be evaluated against the new map.
    if (dungeon_.depth() != depthBefore) {
        return true;
    }

    if (openingLockedDoor) {
        heroOperateTimer_ = 0.5f;
        AudioSystem::instance().playSound("snd_door_open.mp3");
        status_ = "Door unlocked";
        statusTimer_ = 0.7f;
    }

    if (hadTargetMob && !interactingNpc) {
        heroAttackTimer_ = 0.28f;
        hitEffectTimer_ = 0.8f;
        hitEffectCell_ = targetCell;
        AudioSystem::instance().playSound(
            targetCell >= 0 && dungeon_.mobAt(targetCell) != nullptr
                ? "snd_hit.mp3" : "snd_death.mp3"
        );
        status_ =
            targetCell >= 0 && dungeon_.mobAt(targetCell) != nullptr
            ? "You attack the rat"
            : "Rat defeated";
        statusTimer_ = 0.8f;
        if (targetCell >= 0 && dungeon_.mobAt(targetCell) == nullptr) {
            mobDeathTimer_ = 0.5f;
        } else if (capturedDeathVisual) {
            // The hit did not kill: do not leave a stale visual candidate that
            // could later appear beside a living mob.
            mobDeathCell_ = -1;
        }
        lastLoggedStatus_.clear();captureLogMessage();
    } else if (interactingNpc) {
        heroOperateTimer_ = 0.35f;
        const std::string dialogue = dungeon_.takeMessage();
        if (!dialogue.empty()) {
            npcDialogueTitle_ = npcName;
            npcDialogueText_ = dialogue;
            npcDialogueOpen_ = true;
            status_ = dialogue;
            statusTimer_ = 4.0f;
        } else {
            status_ = "You talk to " + npcName;
            statusTimer_ = 1.1f;
        }
        AudioSystem::instance().playSound("snd_click.mp3");
        lastLoggedStatus_.clear(); captureLogMessage();
    }

    if (!hadTargetMob && dungeon_.hero().pos != heroPos) {
        // A ripple belongs to the water tile the hero has just left, not to
        // the destination tile.  This keeps the wake visually behind them.
        if (dungeon_.level().water(heroPos)) {
            spawnWaterRipple(heroPos);
            AudioSystem::instance().playSound("snd_water.mp3", 0.55f);
        } else {
            AudioSystem::instance().playSound("snd_step.mp3", 0.45f);
        }
    }
    updateCombatStatus(heroHpBefore, mobsBefore);
    return true;
}

void GameScene::handleButtons(const VitaInput& input) {
    const bool event =
        input.pressed(SCE_CTRL_LEFT) ||
        input.pressed(SCE_CTRL_RIGHT) ||
        input.pressed(SCE_CTRL_UP) ||
        input.pressed(SCE_CTRL_DOWN);

    if (!event) {
        return;
    }

    int dx = 0;
    int dy = 0;

    if (input.held(SCE_CTRL_LEFT)) {
        dx = -1;
    } else if (input.held(SCE_CTRL_RIGHT)) {
        dx = 1;
    }

    if (input.held(SCE_CTRL_UP)) {
        dy = -1;
    } else if (input.held(SCE_CTRL_DOWN)) {
        dy = 1;
    }

    manualMove(dx, dy);
}

void GameScene::handleAnalog(const VitaInput& input) {
    const float x = input.leftX();
    const float y = input.leftY();

    const float magnitude = std::max(
        std::fabs(x),
        std::fabs(y)
    );

    if (!analogLatched_ && magnitude >= 0.65f) {
        int dx = 0;
        int dy = 0;

        if (x <= -0.38f) dx = -1;
        else if (x >= 0.38f) dx = 1;

        if (y <= -0.38f) dy = -1;
        else if (y >= 0.38f) dy = 1;

        if (dx || dy) {
            manualMove(dx, dy);
        }

        analogLatched_ = true;

    } else if (analogLatched_ && magnitude < 0.30f) {
        analogLatched_ = false;
    }
}

void GameScene::updateHeldMovement(float dt,const VitaInput&input){
    int dx=0,dy=0;
    if(input.held(SCE_CTRL_LEFT))dx=-1;else if(input.held(SCE_CTRL_RIGHT))dx=1;
    else if(input.held(SCE_CTRL_UP))dy=-1;else if(input.held(SCE_CTRL_DOWN))dy=1;
    else {const float ax=input.leftX(),ay=input.leftY();if(std::max(std::fabs(ax),std::fabs(ay))>=0.65f){if(std::fabs(ax)>std::fabs(ay))dx=ax<0?-1:1;else dy=ay<0?-1:1;}}
    if(dx==0&&dy==0){heldMoveDx_=heldMoveDy_=0;moveRepeatTimer_=0.0f;return;}
    if(dx!=heldMoveDx_||dy!=heldMoveDy_){heldMoveDx_=dx;heldMoveDy_=dy;moveRepeatTimer_=-0.27f;return;}
    moveRepeatTimer_+=std::max(0.0f,dt);
    if(moveRepeatTimer_<0.0f)return;
    const int pos=dungeon_.hero().pos;const int x=pos%Level::Width,y=pos/Level::Width;
    if(!dungeon_.level().inBounds(x+dx,y+dy)){moveRepeatTimer_=-0.12f;return;}
    const int target=(y+dy)*Level::Width+(x+dx);
    // Holding a direction may walk, open doors and traverse corridors, but it
    // never repeats attacks. A fresh press is required for every strike.
    if(dungeon_.mobAt(target)){moveRepeatTimer_=-0.27f;return;}
    manualMove(dx,dy);moveRepeatTimer_-=0.11f;
}

int GameScene::screenToCell(float sx, float sy) const {
    if (sy < ViewportY || sy >= ViewportY + ViewportH) {
        return -1;
    }

    const float wx = sx + cameraX_;
    const float wy = (sy - ViewportY) + cameraY_;

    const int x = static_cast<int>(wx / TileSize);
    const int y = static_cast<int>(wy / TileSize);

    return
        x >= 0 && y >= 0 &&
        x < Level::Width && y < Level::Height
        ? y * Level::Width + x
        : -1;
}

void GameScene::handleTouch(
    const VitaInput& input,
    SceneManager& scenes
) {
    for (const auto& touch : input.touches()) {
        if (!touch.active || !touch.pressed) {
            continue;
        }

        const float x = touch.x * 0.5f;
        const float y = touch.y * 0.5f;

        if(x>=334.0f&&x<366.0f&&y>=240.0f&&
           dungeon_.hero().isAlive()&&dungeon_.hero().upgradePoints()>0){
            skillMenuOpen_=true;skillMenuSelection_=0;
            AudioSystem::instance().playSound("snd_click.mp3");return;
        }

        // Touch equivalents for the four right-hand action buttons.
        if(x>=442.0f&&y>=144.0f){
            if(y<174.0f){
                const int found=dungeon_.searchAdjacent();
                if(found>0)revealNewDoors();
                spawnHeroSpecks(found>0?2:3,found>0?3:1);
                searchEffectTimer_=0.65f;searchEffectCell_=dungeon_.hero().pos;
                AudioSystem::instance().playSound(found>0?"snd_secret.mp3":"snd_click.mp3");
                status_=found>0?"Hidden feature found":"You search nearby";statusTimer_=0.9f;
            }else if(y<204.0f){
                if(dungeon_.waitTurn()){status_="You wait a turn";statusTimer_=1.0f;}
            }else if(y<234.0f){
                const bool picked=dungeon_.pickUpAtHero();status_=picked?"Picked up item":"Nothing to pick up";statusTimer_=0.8f;if(picked&&dungeon_.consumeAmuletPickup()){amuletChoiceOpen_=true;amuletChoiceSelection_=0;}if(!picked)spawnHeroSpecks(3,1);
            }else{
                openInventory();return;
            }
            captureLogMessage();return;
        }

        const int target = screenToCell(x, y);

        if (target < 0) {
            continue;
        }

        if(pendingTargetItem_||pendingQuickSlot_){
            const bool quickSlot=pendingQuickSlot_; Item*item=pendingTargetItem_; const ItemAction action=pendingTargetAction_;
            Item*quickItem=quickSlot?dungeon_.hero().primaryQuickSlot().select(dungeon_.hero()):nullptr;
            Item*effectItem=quickSlot?quickItem:item;
            const CatalogWand*effectWand=dynamic_cast<const CatalogWand*>(effectItem);
            const bool firebolt=effectWand&&effectWand->kind()==WandKind::Firebolt;
            const bool potionImpact=dynamic_cast<const Potion*>(effectItem)!=nullptr;
            const CatalogMisc*effectMisc=dynamic_cast<const CatalogMisc*>(effectItem);
            const bool bombImpact=effectMisc&&effectMisc->kind()==MiscKind::Bomb;
            const int itemImage=effectItem?effectItem->image():0, from=dungeon_.hero().pos;
            const bool thrown=(quickSlot&&!dynamic_cast<Wand*>(effectItem))||action==ItemAction::Throw;
            const int impact=thrown?Ballistica::cast(dungeon_,from,target,false,true):target;
            const bool deathCaptured=dungeon_.mobAt(impact)&&captureMobDeathVisual(*dungeon_.mobAt(impact));
            pendingTargetItem_=nullptr; pendingTargetAction_=ItemAction::Use; pendingQuickSlot_=false; targetCursor_=-1;
            const bool acted=quickSlot?dungeon_.usePrimaryQuickSlot(target):dungeon_.executeItemAction(item,action,target);
            if(acted&&(quickSlot||action==ItemAction::Throw))showThrownItem(itemImage,from,impact);
            if(acted&&deathCaptured){if(!dungeon_.mobAt(impact))mobDeathTimer_=0.5f;else mobDeathCell_=-1;}
            if(acted){if(firebolt)showImpactEffect(impact,0xFF7B19FF,1.35f,.55f);else if(potionImpact)showImpactEffect(impact,0x78D9FFFF,1.10f,.42f);else if(bombImpact)showImpactEffect(impact,0xFFB020FF,2.25f,.65f);}
            status_=acted?(quickSlot?"Quickslot used":std::string(itemActionName(action))):"Action failed";statusTimer_=0.9f;if(acted)AudioSystem::instance().playSound("snd_item.mp3");captureLogMessage();return;
        }

        if (Mob* mob = dungeon_.mobAt(target)) {
            if (!dungeon_.visible()[static_cast<std::size_t>(target)]) {
                continue;
            }

            if (Level::adjacent(dungeon_.hero().pos, mob->pos)) {
                const int heroHpBefore = dungeon_.hero().HP;
                const std::size_t mobsBefore = dungeon_.level().mobs().size();
                const bool capturedDeathVisual = captureMobDeathVisual(*mob);
                const CombatActionResult result = dungeon_.heroAttackAt(target);

                if (result.acted) {
                    heroAttackTimer_ = 0.28f;
                    if (result.hit) {
                        hitEffectTimer_ = 0.8f;
                        hitEffectCell_ = target;
                    }
                    AudioSystem::instance().playSound(result.hit ? "snd_hit.mp3" : "snd_miss.mp3");
                    status_ = result.hit ? "You hit the rat" : (std::string(mob->defenseVerb()) == "parried" ? (std::string("The ") + Localization::text(mob->name) + " parried") : "You missed");
                    statusTimer_ = 0.8f;
                    if (result.targetDied) {
                        mobDeathTimer_ = 0.5f;
                    } else if (capturedDeathVisual) {
                        mobDeathCell_ = -1;
                    }
                    updateCombatStatus(heroHpBefore, mobsBefore);
                } else if (capturedDeathVisual) {
                    mobDeathCell_ = -1;
                }

                touchTarget_ = -1;
            } else {
                // Mirrors HeroAction.Attack: walk closer while the enemy stays
                // visible, then attack when adjacent.
                touchTarget_ = target;
                autoWalkTimer_ = AutoStep;
            }

            continue;
        }

        if (!(dungeon_.level().passable(target) ||
              (dungeon_.hero().flying&&dungeon_.level().avoid(target)))) {
            status_ = "Can't walk there";
            statusTimer_ = 0.8f;
            continue;
        }

        touchTarget_ = target;
        autoWalkTimer_ = AutoStep;
    }
}

void GameScene::updateAutoAction(float dt) {
    if (touchTarget_ < 0 || !dungeon_.hero().isAlive()) {
        return;
    }

    autoWalkTimer_ += dt;

    if (autoWalkTimer_ < AutoStep) {
        return;
    }

    autoWalkTimer_ -= AutoStep;

    const int heroHpBefore = dungeon_.hero().HP;
    const std::size_t mobsBefore = dungeon_.level().mobs().size();

    if (Mob* mob = dungeon_.mobAt(touchTarget_)) {
        if (!mob->isAlive()) {
            touchTarget_ = -1;
            return;
        }

        if (Level::adjacent(dungeon_.hero().pos, mob->pos)) {
            const CombatActionResult result = dungeon_.heroAttackAt(mob->pos);
            if (!result.acted) {
                touchTarget_ = -1;
                return;
            }

            status_ = result.hit ? "You hit the rat" : (std::string(mob->defenseVerb()) == "parried" ? (std::string("The ") + Localization::text(mob->name) + " parried") : "You missed");
            heroAttackTimer_ = 0.28f;
            if (result.hit) {
                hitEffectTimer_ = 0.8f;
                hitEffectCell_ = mob->pos;
            }
            AudioSystem::instance().playSound(result.hit ? "snd_hit.mp3" : "snd_miss.mp3");
            statusTimer_ = 0.7f;
            updateCombatStatus(heroHpBefore, mobsBefore);

            if (result.targetDied || !dungeon_.hero().isAlive()) {
                touchTarget_ = -1;
            }
            return;
        }
    }

    if (dungeon_.hero().pos == touchTarget_) {
        touchTarget_ = -1;
        return;
    }

    if (!dungeon_.stepHeroToward(touchTarget_)) {
        // A mob target is occupied by design; choose a best adjacent reachable
        // square and keep pursuing it.
        if (Mob* mob = dungeon_.mobAt(touchTarget_)) {
            int best = -1;
            int bestDistance = 9999;

            for (int off : Level::Neighbours8) {
                const int candidate = mob->pos + off;
                if (!dungeon_.level().inBoundsCell(candidate) ||
                    !(dungeon_.level().passable(candidate) || dungeon_.level().avoid(candidate)) ||
                    dungeon_.scheduler().findChar(candidate) != nullptr) {
                    continue;
                }

                const int step = dungeon_.findStep(dungeon_.hero().pos, candidate);
                if (step < 0) continue;

                const int d = Level::distance(dungeon_.hero().pos, candidate);
                if (d < bestDistance) {
                    bestDistance = d;
                    best = candidate;
                }
            }

            if (best >= 0 && dungeon_.stepHeroToward(best)) {
                updateCombatStatus(heroHpBefore, mobsBefore);
                return;
            }
        }

        status_ = "Path blocked";
        statusTimer_ = 0.8f;
        touchTarget_ = -1;
        return;
    }

    updateCombatStatus(heroHpBefore, mobsBefore);
}

void GameScene::handleGameMenu(const VitaInput& input, SceneManager& scenes) {
    if (journalOpen_) {
        if (input.pressed(SCE_CTRL_CIRCLE) || input.pressed(SCE_CTRL_CROSS)) journalOpen_ = false;
        else {
            const int records=static_cast<int>(Progression::instance().journal().size());
            const int pages=std::max(1,(records+5)/6);
            if(input.pressed(SCE_CTRL_LEFT)||input.pressed(SCE_CTRL_UP))journalPage_=(journalPage_+pages-1)%pages;
            if(input.pressed(SCE_CTRL_RIGHT)||input.pressed(SCE_CTRL_DOWN))journalPage_=(journalPage_+1)%pages;
        }
        return;
    }
    if (input.pressed(SCE_CTRL_CIRCLE)) {
        if(settingsMenuOpen_){settingsMenuOpen_=false;gameMenuSelection_=0;}else gameMenuOpen_ = false;
        return;
    }

    const int itemCount = settingsMenuOpen_ ? 6 : 5;

    if (input.pressed(SCE_CTRL_UP)) {
        gameMenuSelection_ = (gameMenuSelection_ + itemCount-1) % itemCount;
        AudioSystem::instance().playSound("snd_click.mp3");
    } else if (input.pressed(SCE_CTRL_DOWN)) {
        gameMenuSelection_ = (gameMenuSelection_ + 1) % itemCount;
        AudioSystem::instance().playSound("snd_click.mp3");
    }

    const bool change = input.pressed(SCE_CTRL_LEFT) ||
                        input.pressed(SCE_CTRL_RIGHT) ||
                        input.pressed(SCE_CTRL_CROSS);
    if (!change) return;

    if(!settingsMenuOpen_){
        if(gameMenuSelection_==0)gameMenuOpen_=false;
        else if(gameMenuSelection_==1){settingsMenuOpen_=true;gameMenuSelection_=0;}
        else if(gameMenuSelection_==2){cheatMenuOpen_=true;cheatMenuSelection_=0;cheatTeleportDepth_=dungeon_.depth();gameMenuOpen_=false;}
        else if(gameMenuSelection_==3){journalOpen_=true;journalPage_=0;}
        else{dungeon_.save();gameMenuOpen_=false;scenes.request(SceneId::Start);}
    } else if (gameMenuSelection_ == 0) {
        const int delta = input.pressed(SCE_CTRL_LEFT) ? -1 : 1;
        zoomIndex_ = std::max(0, std::min(3, zoomIndex_ + delta));
        static const float zooms[4] = {0.75f, 1.0f, 1.25f, 1.5f};
        TileSize = 16.0f * zooms[zoomIndex_];
        AudioSystem::instance().setCameraZoom(zoomIndex_);
    } else if (gameMenuSelection_ == 1) {
        AudioSystem& audio = AudioSystem::instance();
        audio.setMusicEnabled(!audio.musicEnabled());
    } else if (gameMenuSelection_ == 2) {
        AudioSystem& audio = AudioSystem::instance();
        audio.setSoundEnabled(!audio.soundEnabled());
    } else if (gameMenuSelection_ == 3) {
        AudioSystem& audio = AudioSystem::instance();
        audio.setRemixesEnabled(!audio.remixesEnabled());
    } else if (gameMenuSelection_ == 4) {
        AudioSystem& audio = AudioSystem::instance();
        audio.setLanguage(static_cast<Language>((static_cast<int>(audio.language()) + 1) % 4));
    } else {
        settingsMenuOpen_=false;gameMenuSelection_=0;
    }
    AudioSystem::instance().playSound("snd_click.mp3");
}

void GameScene::handleSkillMenu(const VitaInput& input){
    if(input.pressed(SCE_CTRL_CIRCLE)){skillMenuOpen_=false;return;}
    if(input.pressed(SCE_CTRL_UP)){skillMenuSelection_=(skillMenuSelection_+3)%4;AudioSystem::instance().playSound("snd_click.mp3");}
    else if(input.pressed(SCE_CTRL_DOWN)){skillMenuSelection_=(skillMenuSelection_+1)%4;AudioSystem::instance().playSound("snd_click.mp3");}
    if(input.pressed(SCE_CTRL_CROSS)){
        if(dungeon_.hero().spendUpgrade(skillMenuSelection_)){
            dungeon_.save();status_="Ability upgraded";statusTimer_=1.2f;
            AudioSystem::instance().playSound("snd_click.mp3");
        }else{status_="No upgrade points";statusTimer_=1.0f;AudioSystem::instance().playSound("snd_miss.mp3");}
    }
}

void GameScene::handleCheatMenu(const VitaInput& input) {
    static constexpr int Count = 10;
    if (input.pressed(SCE_CTRL_CIRCLE)) {
        cheatMenuOpen_ = false;
        gameMenuOpen_ = true;
        gameMenuSelection_ = 2;
        return;
    }
    if (input.pressed(SCE_CTRL_UP)) { cheatMenuSelection_ = (cheatMenuSelection_ + Count - 1) % Count; AudioSystem::instance().playSound("snd_click.mp3"); }
    if (input.pressed(SCE_CTRL_DOWN)) { cheatMenuSelection_ = (cheatMenuSelection_ + 1) % Count; AudioSystem::instance().playSound("snd_click.mp3"); }
    if (cheatMenuSelection_ == 8 && (input.pressed(SCE_CTRL_LEFT) || input.pressed(SCE_CTRL_RIGHT))) {
        const int delta = input.pressed(SCE_CTRL_LEFT) ? -1 : 1;
        cheatTeleportDepth_ = std::clamp(cheatTeleportDepth_ + delta, 1, 26);
        AudioSystem::instance().playSound("snd_click.mp3");
        return;
    }
    if (!input.pressed(SCE_CTRL_CROSS)) return;

    Hero& hero = dungeon_.hero();
    const auto give = [&](std::unique_ptr<Item> item) {
        if (!item) return;
        item->identify();
        const std::string name = item->displayName();
        if (hero.belongings().backpack().canCollect(*item)) hero.belongings().backpack().collect(std::move(item));
        else dungeon_.level().drop(std::move(item), hero.pos);
        status_ = "Added " + name;
    };

    switch (cheatMenuSelection_) {
    case 0: hero.setInvulnerable(!hero.invulnerable()); status_ = hero.invulnerable() ? "God mode ON" : "God mode OFF"; break;
    case 1: dungeon_.addGold(100); status_ = "+100 gold"; break;
    case 2: hero.earnExp(hero.maxExp()); status_ = "Level increased"; break;
    case 3: hero.HP = hero.HT; hero.clearBuffs(); status_ = "Healed and cleansed"; break;
    case 4: give(Generator::random(Generator::Category::Weapon, dungeon_.depth())); break;
    case 5: give(Generator::random(Generator::Category::Potion, dungeon_.depth())); break;
    case 6: give(Generator::random(Generator::Category::Scroll, dungeon_.depth())); break;
    case 7: give(Generator::random(Generator::Category::Wand, dungeon_.depth())); break;
    case 8:
        if (dungeon_.teleportToDepth(cheatTeleportDepth_)) {
            resetLevelVisuals();
            interlevelDepth_ = dungeon_.depth();
            interlevelIsTeleport_ = true;
            interlevelTimer_ = 1.2f;
            status_ = "Teleported to floor " + numberToString(dungeon_.depth());
        } else {
            status_ = "Teleport unavailable";
        }
        break;
    case 9:
        if (dungeon_.enterDeveloperTestRoom()) {
            resetLevelVisuals();
            status_ = "Developer test room";
        } else status_ = "Test room unavailable";
        break;
    }
    statusTimer_ = 1.2f;
    AudioSystem::instance().playSound("snd_click.mp3");
}

void GameScene::updateVisualPositions(float dt) {
    const float maxStep = std::max(0.0f, dt) * 7.0f;
    const auto approach=[maxStep](float value,float target){const float delta=target-value;if(std::fabs(delta)<=maxStep)return target;return value+(delta<0?-maxStep:maxStep);};
    const int heroPos = dungeon_.hero().pos;
    const float heroTargetX = static_cast<float>(heroPos % Level::Width);
    const float heroTargetY = static_cast<float>(heroPos / Level::Width);
    if(heroTargetX < heroVisual_.x-0.01f)heroFacingLeft_=true;else if(heroTargetX > heroVisual_.x+0.01f)heroFacingLeft_=false;
    heroVisual_.x = approach(heroVisual_.x,heroTargetX);
    heroVisual_.y = approach(heroVisual_.y,heroTargetY);

    std::unordered_map<int, bool> alive;
    for (const auto& mob : dungeon_.level().mobs()) {
        if (!mob || !mob->isAlive()) continue;
        const int id = mob->id();
        alive[id] = true;
        const float tx = static_cast<float>(mob->pos % Level::Width);
        const float ty = static_cast<float>(mob->pos / Level::Width);
        auto found = mobVisuals_.find(id);
        if (found == mobVisuals_.end()) {
            mobVisuals_[id] = VisualPosition(tx, ty);
        } else {
            if (const ZoneBoss* boss = dynamic_cast<const ZoneBoss*>(mob.get());
                boss && boss->kind() == ZoneBossKind::Tengu &&
                (std::fabs(found->second.x - tx) > 2.0f || std::fabs(found->second.y - ty) > 2.0f)) {
                // A Tengu jump is an instant relocation, not a walk through
                // walls across the whole arena.
                found->second = VisualPosition(tx, ty);
                continue;
            }
            found->second.x = approach(found->second.x,tx);
            found->second.y = approach(found->second.y,ty);
        }
    }
    for (auto it = mobVisuals_.begin(); it != mobVisuals_.end();) {
        if (alive.find(it->first) == alive.end()) it = mobVisuals_.erase(it);
        else ++it;
    }
}

void GameScene::updateCamera() {
    const float mapW = Level::Width * TileSize;
    const float mapH = Level::Height * TileSize;

    const float heroX =
        heroVisual_.x * TileSize + TileSize * 0.5f;

    const float heroY =
        heroVisual_.y * TileSize + TileSize * 0.5f;

    cameraX_ = std::max(
        0.0f,
        std::min(
            mapW - Renderer2D::LogicalWidth,
            heroX - Renderer2D::LogicalWidth * 0.5f
        )
    );

    cameraY_ = std::max(
        0.0f,
        std::min(
            mapH - ViewportH,
            heroY - ViewportH * 0.5f
        )
    );
}

void GameScene::updateAnimations(float dt) {
    if (seenMobAttackSerial_ != dungeon_.mobAttackSerial()) {
        seenMobAttackSerial_ = dungeon_.mobAttackSerial();
        attackingMobCell_ = dungeon_.lastMobAttackCell();
        mobAttackTimer_ = 0.34f;
    }
    heroAnimationTime_ += dt;
    waterAnimationTime_ += dt;
    heroIdleFrame_ = std::fmod(heroAnimationTime_, 1.2f) > 0.9f ? 1 : 0;

    ratAnimationTime_ += dt;
    const int phase = static_cast<int>(ratAnimationTime_ * 2.0f) % 4;
    ratIdleFrame_ = phase == 3 ? 1 : 0;
    heroAttackTimer_ = std::max(0.0f, heroAttackTimer_ - dt);
    heroOperateTimer_ = std::max(0.0f, heroOperateTimer_ - dt);
    mobAttackTimer_ = std::max(0.0f, mobAttackTimer_ - dt);
    mobDeathTimer_ = std::max(0.0f, mobDeathTimer_ - dt);
    if (mobDeathTimer_ > 0.0f && mobDeathCell_ >= 0 && mobDeathSpeckFrame_ >= 0 && !mobDeathSpecksShown_) {
        spawnSpecks(mobDeathCell_, mobDeathSpeckFrame_, 5);
        mobDeathSpecksShown_ = true;
    }
    if (mobDeathTimer_ <= 0.0f && mobDeathCell_ >= 0) {
        corpses_.push_back({mobDeathCell_, mobDeathSheet_, mobDeathFrameWidth_, mobDeathFrameHeight_, mobDeathXOffset_, mobDeathLastFrame_});
        mobDeathCell_ = -1;
    }
    if (mobDeathTimer_ <= 0.0f && mobDeathCell_ < 0) {
        mobDeathSpeckFrame_ = -1;
        mobDeathSpecksShown_ = false;
    }
    for (CorpseVisual& corpse : corpses_) corpse.fadeRemaining -= dt;
    corpses_.erase(std::remove_if(corpses_.begin(), corpses_.end(), [](const CorpseVisual& corpse) {
        return corpse.fadeRemaining <= 0.0f;
    }), corpses_.end());
    for (WaterRipple& ripple : waterRipples_) ripple.remaining -= dt;
    waterRipples_.erase(std::remove_if(waterRipples_.begin(), waterRipples_.end(), [](const WaterRipple& ripple) {
        return ripple.remaining <= 0.0f;
    }), waterRipples_.end());
    for (ThrownItemVisual& thrown : thrownItems_) thrown.remaining -= dt;
    thrownItems_.erase(std::remove_if(thrownItems_.begin(), thrownItems_.end(), [](const ThrownItemVisual& thrown) {
        return thrown.remaining <= 0.0f;
    }), thrownItems_.end());
    for (ImpactEffect& effect : impactEffects_) effect.remaining -= dt;
    impactEffects_.erase(std::remove_if(impactEffects_.begin(), impactEffects_.end(), [](const ImpactEffect& effect) {
        return effect.remaining <= 0.0f;
    }), impactEffects_.end());
    for (auto& entry : itemPopTimers_) entry.second -= dt;
    for (auto it = itemPopTimers_.begin(); it != itemPopTimers_.end();) {
        if (it->second <= 0.0f) it = itemPopTimers_.erase(it);
        else ++it;
    }
    for (auto& entry : doorRevealTimers_) entry.second -= dt;
    for (auto it = doorRevealTimers_.begin(); it != doorRevealTimers_.end();) {
        if (it->second <= 0.0f) it = doorRevealTimers_.erase(it);
        else ++it;
    }
    if (mobAttackTimer_ <= 0.0f) attackingMobCell_ = -1;
    hitEffectTimer_ = std::max(0.0f, hitEffectTimer_ - dt);
    if (hitEffectTimer_ <= 0.0f) hitEffectCell_ = -1;
    searchEffectTimer_=std::max(0.0f,searchEffectTimer_-dt);
    if(searchEffectTimer_<=0.0f)searchEffectCell_=-1;
    updateHeroSpecks(dt);
    updateBadgeNotification(dt);
}

void GameScene::updateBadgeNotification(float dt) {
    if (badgeNotificationRemaining_ > 0.0f) {
        badgeNotificationRemaining_ = std::max(0.0f, badgeNotificationRemaining_ - dt);
        return;
    }

    Badge badge;
    if (Progression::instance().takeUnlockedBadge(badge)) {
        activeBadge_ = badge;
        badgeNotificationRemaining_ = 5.0f;
        AudioSystem::instance().playSound("snd_badge.mp3");
    }
}

void GameScene::renderBadgeNotification(Renderer2D& renderer) const {
    if (badgeNotificationRemaining_ <= 0.0f || badgesSheet_ == nullptr || font_ == nullptr) return;

    constexpr float Duration = 5.0f;
    constexpr float Slide = 0.35f;
    float y = 4.0f;
    const float elapsed = Duration - badgeNotificationRemaining_;
    if (elapsed < Slide) y = -24.0f + 28.0f * (elapsed / Slide);
    else if (badgeNotificationRemaining_ < Slide) y = -24.0f + 28.0f * (badgeNotificationRemaining_ / Slide);

    constexpr float x = 114.0f;
    constexpr float width = 252.0f;
    renderer.rect(x, y, width, 24.0f, 0x171A22F2);
    renderer.outlineRect(x, y, width, 24.0f, 1.0f, 0xE2BD56FF);

    const int columns = std::max(1, badgesSheet_->width() / 16);
    const int frames = std::max(1, columns * (badgesSheet_->height() / 16));
    const int icon = static_cast<int>(activeBadge_) % frames;
    renderer.texture(*badgesSheet_, x + 5.0f, y + 4.0f, 16.0f, 16.0f,
        TextureRegion(static_cast<float>((icon % columns) * 16), static_cast<float>((icon / columns) * 16), 16.0f, 16.0f));
    font_->draw(renderer, Localization::text("Achievement unlocked"), x + 26.0f, y + 3.0f, 1.0f, .70f, 1.0f);
    font_->draw(renderer, Progression::instance().badgeName(activeBadge_), x + 26.0f, y + 12.0f, 1.0f, 1.0f, 1.0f);
}

void GameScene::spawnHeroSpecks(int frame, int count) {
    spawnSpecks(dungeon_.hero().pos, frame, count);
}

void GameScene::spawnSpecks(int cell, int frame, int count) {
    // The 15 cells of Speck.java's 7x7 atlas are supported. Keep every burst
    // small so the Vita renderer never has a particle spike during combat.
    static constexpr float offsets[] = {-0.24f, 0.18f, -0.08f, 0.28f, -0.31f};
    if (!dungeon_.level().inBoundsCell(cell)) return;
    const float baseX = static_cast<float>(cell % Level::Width) + 0.5f;
    const float baseY = static_cast<float>(cell / Level::Width) + 0.54f;
    const int burst = std::min(count, 5);

    for (int index = 0; index < burst; ++index) {
        const float offset = offsets[index];
        HeroSpeck speck{baseX + offset, baseY + 0.10f * static_cast<float>(index % 2),
                         offset * 0.28f, -1.18f - 0.10f * static_cast<float>(index % 3), 1.0f, 1.0f, 0.0f, frame};
        switch (frame) {
            case 1:  speck.velocityX = offset * 3.8f; speck.velocityY = -0.60f - 0.18f * index; speck.gravity = 3.2f; break; // STAR
            case 2:  speck.velocityY = -0.25f; break; // LIGHT
            case 3:  speck.velocityY = -0.35f; speck.duration = speck.remaining = .80f; break; // QUESTION
            case 4:  break; // UP
            case 5:  speck.velocityY = -0.10f; speck.duration = speck.remaining = .90f; break; // SCREAM
            case 6:  speck.velocityX = offset * 4.2f; speck.velocityY = -0.8f; speck.gravity = 3.8f; speck.duration = speck.remaining = .35f; break; // BONE
            case 7:  speck.velocityY = -2.0f; speck.duration = speck.remaining = .50f; break; // WOOL
            case 8:  speck.velocityX = offset * 2.6f; speck.velocityY = 1.8f; speck.gravity = 2.5f; speck.duration = speck.remaining = .30f; break; // ROCK
            case 9:  speck.velocityX = offset * .55f; speck.velocityY = -1.65f; break; // NOTE
            case 10: speck.velocityX = offset * 1.4f; speck.velocityY = -0.55f; speck.duration = speck.remaining = 1.5f; break; // CHANGE
            case 11: speck.velocityX = offset * .75f; speck.velocityY = -1.65f; break; // HEART
            case 12: speck.velocityY = -0.95f; speck.duration = speck.remaining = 1.25f; break; // BUBBLE
            case 13: speck.velocityX = offset * 1.1f; speck.velocityY = -1.4f; break; // STEAM
            case 14: speck.velocityX = offset * 2.6f; speck.velocityY = -2.4f; speck.gravity = 5.0f; break; // COIN
            default: break; // HEALING
        }
        heroSpecks_.push_back(speck);
    }
}

void GameScene::updateHeroSpecks(float dt) {
    const Hero& hero = dungeon_.hero();
    const bool leveledUp = displayedHeroLevel_ >= 0 && hero.level() > displayedHeroLevel_;

    // A level-up also restores HP.  Show the green UP burst in that case,
    // rather than mixing it with healing crosses in the same moment.
    if (leveledUp) {
        spawnHeroSpecks(4, 4);
    } else if (displayedHeroHp_ >= 0 && hero.HP > displayedHeroHp_) {
        spawnHeroSpecks(0, std::min(5, 2 + hero.HP - displayedHeroHp_));
    }
    if (displayedGold_ >= 0 && dungeon_.gold() > displayedGold_) {
        spawnHeroSpecks(14, 3);
    }

    displayedHeroHp_ = hero.HP;
    displayedHeroLevel_ = hero.level();
    displayedGold_ = dungeon_.gold();

    for (HeroSpeck& speck : heroSpecks_) {
        speck.velocityY += speck.gravity * dt;
        speck.x += speck.velocityX * dt;
        speck.y += speck.velocityY * dt;
        speck.remaining -= dt;
    }
    heroSpecks_.erase(std::remove_if(heroSpecks_.begin(), heroSpecks_.end(), [](const HeroSpeck& speck) {
        return speck.remaining <= 0.0f;
    }), heroSpecks_.end());
}

bool GameScene::captureMobDeathVisual(const Mob& mob) {
    // A death animation already owns these fields.  Never overwrite it with
    // a living target from a repeated input event.
    if (mobDeathTimer_ > 0.0f || mobDeathCell_ >= 0) return false;

    mobDeathCell_ = mob.pos;
    mobDeathSpeckFrame_ = dynamic_cast<const Skeleton*>(&mob) ? 6 :
                          dynamic_cast<const Swarm*>(&mob) ? 7 :
                          dynamic_cast<const Mimic*>(&mob) ? 1 : -1;
    mobDeathSpecksShown_ = false;
    mobDeathSheet_ = ratSheet_;
    mobDeathFrameWidth_ = 16;
    mobDeathFrameHeight_ = 15;
    mobDeathXOffset_ = 0.0f;
    mobDeathFirstFrame_ = 11;
    mobDeathLastFrame_ = 14;

    // These indices come from the original Sprite classes in reference/.
    // Keeping them here means every platform renders the same shared battle result.
    if (dynamic_cast<const Albino*>(&mob)) {
        mobDeathFirstFrame_ = 27;
        mobDeathLastFrame_ = 30;
    } else if (dynamic_cast<const Gnoll*>(&mob)) {
        mobDeathSheet_ = gnollSheet_;
        mobDeathFrameWidth_ = 12;
        mobDeathXOffset_ = 2.0f;
        mobDeathFirstFrame_ = 8;
        mobDeathLastFrame_ = 10;
    } else if (dynamic_cast<const Goo*>(&mob)) {
        mobDeathSheet_ = gooSheet_;
        mobDeathFrameWidth_ = 20;
        mobDeathFrameHeight_ = 14;
        mobDeathXOffset_ = -2.0f;
        mobDeathFirstFrame_ = 2;
        mobDeathLastFrame_ = 4;
    } else if (dynamic_cast<const Swarm*>(&mob)) {
        mobDeathSheet_ = swarmSheet_;
        mobDeathFrameWidth_ = 16;
        mobDeathFrameHeight_ = 16;
        mobDeathFirstFrame_ = 10;
        mobDeathLastFrame_ = 14;
    } else if (dynamic_cast<const Skeleton*>(&mob)) {
        mobDeathSheet_ = skeletonSheet_;
        mobDeathFrameWidth_ = 12;
        mobDeathXOffset_ = 2.0f;
        mobDeathFirstFrame_ = 10;
        mobDeathLastFrame_ = 13;
    } else if (dynamic_cast<const Thief*>(&mob)) {
        mobDeathSheet_ = thiefSheet_;
        mobDeathFrameWidth_ = 12;
        mobDeathFrameHeight_ = 13;
        mobDeathXOffset_ = 2.0f;
        mobDeathFirstFrame_ = 5;
        mobDeathLastFrame_ = 9;
    } else if (dynamic_cast<const Crab*>(&mob)) {
        mobDeathSheet_ = crabSheet_;
        mobDeathFrameWidth_ = 16;
        mobDeathFrameHeight_ = 16;
        mobDeathFirstFrame_ = 10;
        mobDeathLastFrame_ = 13;
    } else if (dynamic_cast<const Piranha*>(&mob)) {
        mobDeathSheet_ = piranhaSheet_;
        mobDeathFrameWidth_ = 12;
        mobDeathFrameHeight_ = 16;
        mobDeathXOffset_ = 2.0f;
        mobDeathFirstFrame_ = 12;
        mobDeathLastFrame_ = 14;
    } else if (const ZoneBoss* boss = dynamic_cast<const ZoneBoss*>(&mob)) {
        mobDeathSheet_ = zoneBossSheets_[static_cast<std::size_t>(boss->kind())];
        switch(boss->kind()){
        case ZoneBossKind::Tengu: mobDeathFrameWidth_=14;mobDeathFrameHeight_=16;mobDeathXOffset_=1.0f;mobDeathFirstFrame_=8;mobDeathLastFrame_=10;break;
        case ZoneBossKind::DM300: mobDeathFrameWidth_=22;mobDeathFrameHeight_=20;mobDeathXOffset_=-3.0f;mobDeathFirstFrame_=7;mobDeathLastFrame_=8;break;
        case ZoneBossKind::King: mobDeathFrameWidth_=16;mobDeathFrameHeight_=16;mobDeathFirstFrame_=12;mobDeathLastFrame_=15;break;
        case ZoneBossKind::Yog: mobDeathFrameWidth_=20;mobDeathFrameHeight_=19;mobDeathXOffset_=-2.0f;mobDeathFirstFrame_=7;mobDeathLastFrame_=9;break;
        }
    } else if (const ZoneMob* zone = dynamic_cast<const ZoneMob*>(&mob)) {
        mobDeathSheet_ = zoneMobSheets_[static_cast<std::size_t>(zone->kind())];
        if (zone->kind() == ZoneMobKind::Brute) {
            mobDeathFrameWidth_=12;mobDeathFrameHeight_=16;mobDeathXOffset_=2.0f;mobDeathFirstFrame_=8;mobDeathLastFrame_=10;
        } else if(zone->kind()==ZoneMobKind::Shielded){
            mobDeathFrameWidth_=12;mobDeathFrameHeight_=16;mobDeathXOffset_=2.0f;mobDeathFirstFrame_=29;mobDeathLastFrame_=31;
        } else if(zone->kind()==ZoneMobKind::Monk){
            mobDeathFrameWidth_=15;mobDeathFrameHeight_=14;mobDeathXOffset_=.5f;mobDeathFirstFrame_=7;mobDeathLastFrame_=10;
        } else if(zone->kind()==ZoneMobKind::Senior){
            mobDeathFrameWidth_=15;mobDeathFrameHeight_=14;mobDeathXOffset_=.5f;mobDeathFirstFrame_=24;mobDeathLastFrame_=27;
        } else if(zone->kind()==ZoneMobKind::Undead){
            mobDeathFrameWidth_=12;mobDeathFrameHeight_=16;mobDeathXOffset_=2.0f;mobDeathFirstFrame_=10;mobDeathLastFrame_=13;
        } else if(zone->kind()==ZoneMobKind::RottingFist||zone->kind()==ZoneMobKind::BurningFist){
            mobDeathFrameWidth_=24;mobDeathFrameHeight_=17;mobDeathXOffset_=-4.0f;mobDeathFirstFrame_=2;mobDeathLastFrame_=4;
        } else if(zone->kind()==ZoneMobKind::Elemental){
            mobDeathFrameWidth_=12;mobDeathFrameHeight_=14;mobDeathXOffset_=2.0f;mobDeathFirstFrame_=7;mobDeathLastFrame_=13;
        } else if(zone->kind()==ZoneMobKind::Succubus){
            mobDeathFrameWidth_=12;mobDeathFrameHeight_=15;mobDeathXOffset_=2.0f;mobDeathFirstFrame_=12;mobDeathLastFrame_=12;
        } else if(zone->kind()==ZoneMobKind::Warlock){
            mobDeathFrameWidth_=12;mobDeathFrameHeight_=15;mobDeathXOffset_=2.0f;mobDeathFirstFrame_=7;mobDeathLastFrame_=10;
        } else if(zone->kind()==ZoneMobKind::Scorpio){
            mobDeathFrameWidth_=18;mobDeathFrameHeight_=17;mobDeathXOffset_=-1.0f;mobDeathFirstFrame_=7;mobDeathLastFrame_=10;
        } else if(zone->kind()==ZoneMobKind::Acidic){
            mobDeathFrameWidth_=18;mobDeathFrameHeight_=17;mobDeathXOffset_=-1.0f;mobDeathFirstFrame_=21;mobDeathLastFrame_=24;
        } else if (zone->kind() == ZoneMobKind::Shaman) {
            mobDeathFrameWidth_ = 12;
            mobDeathFrameHeight_ = 15;
            mobDeathXOffset_ = 2.0f;
            mobDeathFirstFrame_ = 8;
            mobDeathLastFrame_ = 10;
        } else if (zone->kind() == ZoneMobKind::Bat) {
            mobDeathFrameWidth_ = 15;
            mobDeathFrameHeight_ = 15;
            mobDeathXOffset_ = 0.5f;
            mobDeathFirstFrame_ = 4;
            mobDeathLastFrame_ = 6;
        } else {
            mobDeathFrameWidth_ = 16;
            mobDeathFrameHeight_ = 16;
            const int frames = std::max(1, (mobDeathSheet_->width() / mobDeathFrameWidth_) *
                                           (mobDeathSheet_->height() / mobDeathFrameHeight_));
            mobDeathFirstFrame_ = std::max(0, frames - 3);
            mobDeathLastFrame_ = frames - 1;
        }
    }
    return true;
}

void GameScene::spawnWaterRipple(int cell) {
    if (cell < 0 || !dungeon_.level().inBoundsCell(cell)) return;
    waterRipples_.push_back({cell, 0.55f});
}

void GameScene::triggerItemPop(int cell) {
    if (cell >= 0 && dungeon_.level().inBoundsCell(cell)) {
        itemPopTimers_[cell] = 0.38f;
        spawnSpecks(cell, 1, 3);
    }
}

void GameScene::showThrownItem(int image, int from, int to) {
    if (from < 0 || to < 0) return;
    thrownItems_.push_back({image, from, to, 0.24f});
}

void GameScene::showImpactEffect(int cell, uint32_t color, float radius, float duration) {
    if (cell < 0 || !dungeon_.level().inBoundsCell(cell)) return;
    impactEffects_.push_back({cell, duration, duration, radius, color});
    if (color == 0xFF7B19FF) spawnSpecks(cell, 2, 3);
    else if (color == 0x78D9FFFF) spawnSpecks(cell, 13, 3);
    else if (color == 0xFFB020FF) spawnSpecks(cell, 8, 5);
}

void GameScene::captureLogMessage() {
    if (status_.empty()) return;
    if (status_ == lastLoggedStatus_ && statusTimer_ <= lastLoggedTimer_ + 0.05f) {
        lastLoggedTimer_=statusTimer_;
        return;
    }
    lastLoggedStatus_ = status_;
    lastLoggedTimer_ = statusTimer_;
    messageLog_.push_back(status_);
    uint32_t color=0xB8B8B8FF;
    if(status_.find("Killed ")==0||status_.find("defeated")!=std::string::npos||status_.find("died")!=std::string::npos)color=0xFF6666FF;
    else if(status_.find("Ate ")==0||status_.find("Drink ")==0||status_.find("Use ")==0||status_.find("healed")!=std::string::npos||status_.find("dewdrop")!=std::string::npos)color=0x70E878FF;
    messageLogColors_.push_back(color);
    while(messageLog_.size()>6){messageLog_.pop_front();messageLogColors_.pop_front();}
}


void GameScene::renderInventory(
    Renderer2D& renderer
) {
    renderer.rect(
        0,
        0,
        Renderer2D::LogicalWidth,
        Renderer2D::LogicalHeight,
        0x000000B8
    );

    renderer.rect(
        InventoryX - 3,
        InventoryY - 3,
        InventoryWidth + 6,
        InventoryHeight + 6,
        0x18191CFA
    );

    renderer.outlineRect(
        InventoryX - 3,
        InventoryY - 3,
        InventoryWidth + 6,
        InventoryHeight + 6,
        1,
        0x777A82FF
    );

    font_->drawCentered(
        renderer,
        inventory_.title(),
        InventoryX +
            InventoryWidth *
            0.5f,
        InventoryY + 2,
        1,
        1,
        1
    );

    // Draw the whole backpack grid, including available slots.  This keeps
    // the left panel balanced when the hero carries only a few items.
    for (int row = 0; row < InventoryRows; ++row) {
        for (int col = 0; col < InventoryModel::Columns; ++col) {
            const float sx = InventoryGridX + col * (InventoryModel::SlotSize + InventoryModel::SlotMargin);
            const float sy = InventoryY + InventoryModel::TitleHeight + row * (InventoryModel::SlotSize + InventoryModel::SlotMargin);
            renderer.rect(sx, sy, InventoryModel::SlotSize, InventoryModel::SlotSize, 0x202226FF);
            renderer.outlineRect(sx, sy, InventoryModel::SlotSize, InventoryModel::SlotSize, 1, 0x3C3F46FF);
        }
    }

    const auto& slots =
        inventory_.slots();

    for (int i = 0;
         i <
            static_cast<int>(
                slots.size()
            );
         ++i) {

        const InventorySlot& slot =
            slots[
                static_cast<std::size_t>(
                    i
                )
            ];

        const float sx =
            InventoryGridX +
            slot.col *
            (
                InventoryModel::SlotSize +
                InventoryModel::SlotMargin
            );

        const float sy =
            InventoryY +
            InventoryModel::TitleHeight +
            slot.row *
            (
                InventoryModel::SlotSize +
                InventoryModel::SlotMargin
            );

        renderer.rect(
            sx,
            sy,
            InventoryModel::SlotSize,
            InventoryModel::SlotSize,
            slot.equipped
            ? 0x3C3E39FF
            : 0x292B2FFF
        );

        if (i ==
            inventory_
                .selectedIndex()) {

            renderer.outlineRect(
                sx,
                sy,
                InventoryModel::SlotSize,
                InventoryModel::SlotSize,
                2,
                0xE6D36BFF
            );

        } else {
            renderer.outlineRect(
                sx,
                sy,
                InventoryModel::SlotSize,
                InventoryModel::SlotSize,
                1,
                slot.enabled
                ? 0x55585FFF
                : 0x303238FF
            );
        }

        if (!slot.item) {
            continue;
        }

        const int image =
            slot.item->image();

        const int atlasX =
            (image % 8) *
            16;

        const int atlasY =
            (image / 8) *
            16;

        renderer.texture(
            *itemsSheet_,
            sx + (InventoryModel::SlotSize - 16) * 0.5f,
            sy + (InventoryModel::SlotSize - 16) * 0.5f,
            16,
            16,
            TextureRegion(
                static_cast<float>(
                    atlasX
                ),
                static_cast<float>(
                    atlasY
                ),
                16,
                16
            ),
            slot.enabled
            ? 1.0f
            : 0.28f,
            1.0f
        );

        if (slot.item
                ->quantity() > 1) {

            char quantity[16];

            snprintf(
                quantity,
                sizeof(quantity),
                "%d",
                slot.item
                    ->quantity()
            );

            const float quantityWidth = font_->measure(quantity, 0.75f);
            font_->draw(
                renderer,
                quantity,
                sx + InventoryModel::SlotSize - quantityWidth - 2,
                sy + InventoryModel::SlotSize - 11,
                0.75f,
                slot.enabled
                ? 1.0f
                : 0.35f,
                1
            );
        }
    }

    Item* selected =
        inventory_
            .selectedItem();

    if (selected != inventoryDescriptionItem_) {
        inventoryDescriptionItem_ = selected;
        inventoryDescriptionScroll_ = 0;
    }

    renderer.rect(InventoryDetailsX-3,InventoryDetailsY-3,InventoryDetailsWidth+6,InventoryDetailsHeight+6,0x18191CFA);
    renderer.outlineRect(InventoryDetailsX-3,InventoryDetailsY-3,InventoryDetailsWidth+6,InventoryDetailsHeight+6,1,0x777A82FF);
    font_->drawCentered(renderer,Localization::text("ITEM INFO"),InventoryDetailsX+InventoryDetailsWidth*0.5f,InventoryDetailsY+2,1,0.8f,1);

    if (selected) {
        const int image=selected->image();
        renderer.texture(*itemsSheet_,InventoryDetailsX+7,InventoryDetailsY+19,32,32,
            TextureRegion(static_cast<float>((image%8)*16),static_cast<float>((image/8)*16),16,16),1,1);
        font_->draw(renderer,inventoryLabel(selected->displayName(),27),InventoryDetailsX+43,InventoryDetailsY+20,1,0.95f,1);
        font_->draw(renderer,Localization::text("EFFECTS"),InventoryDetailsX+43,InventoryDetailsY+32,1,0.5f,1);

        std::string effects;
        const int selectedIndex=inventory_.selectedIndex();
        if(selectedIndex>=0&&selectedIndex<static_cast<int>(slots.size())&&slots[static_cast<std::size_t>(selectedIndex)].equipped)effects=Localization::text("Equipped");
        if(selected->cursedKnown())effects+=(effects.empty()?"":" | ")+Localization::text(selected->cursed()?"Cursed":"Uncursed");
        if(selected->isBroken())effects+=(effects.empty()?"":" | ")+Localization::text("Broken");
        if(const Wand* wand=dynamic_cast<const Wand*>(selected)){char charges[24];snprintf(charges,sizeof(charges),"Charges %d/%d",wand->curCharges(),wand->maxCharges());effects+=(effects.empty()?"":" | ")+std::string(charges);}
        if(const Armor* armor=dynamic_cast<const Armor*>(selected)){char defense[24];snprintf(defense,sizeof(defense),"DR %d, STR %d",armor->DR(),armor->STR());effects+=(effects.empty()?"":" | ")+std::string(defense);}
        if(effects.empty())effects=Localization::text("Ready to use");
        drawInventoryWrapped(*font_,renderer,effects,InventoryDetailsX+43,InventoryDetailsY+42,27,0.68f,3);
        font_->draw(renderer,Localization::text("DESCRIPTION  L1/R1"),InventoryDetailsX+7,InventoryDetailsY+72,1,0.5f,1);
        const std::string description=inventoryPurpose(*selected);
        const int descriptionRows=inventoryWrappedLineCount(description,35);
        const int visibleDescriptionRows=14;
        const int maxDescriptionScroll=std::max(0,descriptionRows-visibleDescriptionRows);
        inventoryDescriptionScroll_=std::min(inventoryDescriptionScroll_,maxDescriptionScroll);
        drawInventoryWrapped(*font_,renderer,description,InventoryDetailsX+7,InventoryDetailsY+83,35,0.72f,visibleDescriptionRows,inventoryDescriptionScroll_);
        if(inventoryDescriptionScroll_>0)font_->draw(renderer,"^",InventoryDetailsX+InventoryDetailsWidth-12,InventoryDetailsY+72,1,0.8f,1);
        if(inventoryDescriptionScroll_<maxDescriptionScroll)font_->draw(renderer,"v",InventoryDetailsX+InventoryDetailsWidth-12,InventoryDetailsY+224-12,1,0.8f,1);
    } else {
        font_->drawCentered(renderer,Localization::text("Select an item"),InventoryDetailsX+InventoryDetailsWidth*0.5f,InventoryDetailsY+57,1,0.65f,1);
    }

    if (inventory_.contextOpen()) {
        const auto& actions=inventory_.contextActions();
        const float menuW=126.0f;
        const float menuH=24.0f+static_cast<float>(actions.size())*14.0f;
        const int selectedIndex=inventory_.selectedIndex();
        const InventorySlot* selectedSlot=(selectedIndex>=0&&selectedIndex<static_cast<int>(slots.size()))
            ? &slots[static_cast<std::size_t>(selectedIndex)] : nullptr;
        const float itemX=selectedSlot
            ? InventoryGridX+selectedSlot->col*(InventoryModel::SlotSize+InventoryModel::SlotMargin)
            : InventoryGridX;
        const float itemY=selectedSlot
            ? InventoryY+InventoryModel::TitleHeight+selectedSlot->row*(InventoryModel::SlotSize+InventoryModel::SlotMargin)
            : InventoryY+InventoryModel::TitleHeight;
        // The menu follows the item: immediately to the right of its cell.
        // Only its vertical position is constrained for the bottom rows.
        const float menuX=std::min(itemX+InventoryModel::SlotSize+3.0f, 480.0f-menuW-4.0f);
        const float menuY=std::max(InventoryY+3.0f,std::min(itemY,InventoryY+InventoryHeight-menuH-3.0f));
        renderer.rect(menuX,menuY,menuW,menuH,0x15171AF8);
        renderer.outlineRect(menuX,menuY,menuW,menuH,1,0xD0C06AFF);
        font_->draw(renderer,selected?selected->displayName():Localization::text("Item"),menuX+5,menuY+5,1,0.85f,1);
        for(int i=0;i<static_cast<int>(actions.size());++i){
            const float y=menuY+21+i*14.0f;
            if(i==inventory_.contextSelection()) renderer.rect(menuX+3,y-2,menuW-6,13,0x56513AFF);
            font_->draw(renderer,itemActionName(actions[static_cast<std::size_t>(i)]),menuX+7,y,1,i==inventory_.contextSelection()?1.0f:0.72f,1);
        }
    }

    char gold[32];

    snprintf(
        gold,
        sizeof(gold),
        "Gold %d",
        dungeon_.gold()
    );

    font_->draw(
        renderer,
        gold,
        InventoryGridX +
            InventoryGridWidth -
            45,
        InventoryY + 1,
        1,
        0.72f,
        1
    );

    if (pendingCancelConfirm_) {
        renderer.rect(
            86,
            222,
            308,
            22,
            0x090909F4
        );

        font_->drawCentered(
            renderer,
            "Cancel unknown scroll? O consume / X return",
            240,
            228,
            1,
            1,
            1
        );

    } else {
        drawVitaButtonHint(renderer, *font_, *uiButtonsSheet_, {
            {VitaButtonIcon::Cross, " use"}, {VitaButtonIcon::Circle, " close"},
            {VitaButtonIcon::DPad, " move"}
        }, 240, 250, .72f, .72f);
    }
}

void GameScene::render(Renderer2D& renderer) {
    if (!font_ || !tiles_ || !ratSheet_ || !dungeon_.started()) {
        return;
    }

    renderer.rect(
        0, 0,
        Renderer2D::LogicalWidth,
        Renderer2D::LogicalHeight,
        0x050508FF
    );

    const auto& level = dungeon_.level();
    const auto& visible = dungeon_.visible();
    const auto& seen = level.visited();
    const auto& mapped = level.mapped();
    const float spriteScale = TileSize / 16.0f;

    const int minX = std::max(0, static_cast<int>(cameraX_ / TileSize) - 1);
    const int maxX = std::min(
        Level::Width - 1,
        static_cast<int>((cameraX_ + Renderer2D::LogicalWidth) / TileSize) + 1
    );

    const int minY = std::max(0, static_cast<int>(cameraY_ / TileSize) - 1);
    const int maxY = std::min(
        Level::Height - 1,
        static_cast<int>((cameraY_ + ViewportH) / TileSize) + 1
    );

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            const int pos = y * Level::Width + x;

            if (!visible[pos] && !seen[pos] && !mapped[pos]) {
                continue;
            }

            int tile = level.cell(pos);
            if (tile < 0 || tile > 63) tile = Terrain::EMPTY;

            const int atlasX = (tile % 16) * 16;
            const int atlasY = (tile / 16) * 16;

            const float brightness = visible[pos] ? 1.0f : 0.32f;
            float tileAlpha = 1.0f;
            const auto reveal = doorRevealTimers_.find(pos);
            if (reveal != doorRevealTimers_.end()) {
                tileAlpha = 1.0f - std::clamp(reveal->second / 0.65f, 0.0f, 1.0f);
            }

            if (level.water(pos)) {
                // Scroll liquid downward.  Drawing the wrapped source in two
                // integer-sized passes prevents a filtering/rounding seam
                // from briefly exposing the black frame buffer between tiles.
                const Texture2D& water = *waterSheets_[static_cast<std::size_t>(std::min(4, (dungeon_.depth() - 1) / 5))];
                const float scroll = static_cast<float>(static_cast<int>(waterAnimationTime_ * 8.0f) % 32);
                const float sourceY = std::fmod(static_cast<float>(y) * 16.0f - scroll + 32.0f, 32.0f);
                const float firstSourceHeight = std::min(16.0f, 32.0f - sourceY);
                const float firstScreenHeight = std::ceil(TileSize * (firstSourceHeight / 16.0f));
                const float screenX = x * TileSize - cameraX_;
                const float screenY = ViewportY + y * TileSize - cameraY_;
                renderer.texture(water, screenX, screenY, TileSize, firstScreenHeight,
                    TextureRegion(0.0f, sourceY, 16.0f, firstSourceHeight), 1.0f, brightness);
                if (firstSourceHeight < 16.0f) {
                    renderer.texture(water, screenX, screenY + firstScreenHeight,
                        TileSize, TileSize - firstScreenHeight,
                        TextureRegion(0.0f, 0.0f, 16.0f, 16.0f - firstSourceHeight), 1.0f, brightness);
                }
            }

            renderer.texture(
                *zoneTiles_[static_cast<std::size_t>(std::min(4,(dungeon_.depth()-1)/5))],
                x * TileSize - cameraX_,
                ViewportY + y * TileSize - cameraY_,
                TileSize,
                TileSize,
                TextureRegion(
                    static_cast<float>(atlasX),
                    static_cast<float>(atlasY),
                    16, 16
                ),
                tileAlpha,
                brightness
            );
        }
    }

    for (const auto& entry : level.plants()) {
        const int pos=entry.first;
        if(!level.inBoundsCell(pos)||!visible[static_cast<std::size_t>(pos)])continue;
        const int x=pos%Level::Width,y=pos/Level::Width;
        const int image=static_cast<int>(entry.second);
        renderer.texture(*plantsSheet_,x*TileSize-cameraX_,ViewportY+y*TileSize-cameraY_,16*spriteScale,16*spriteScale,TextureRegion(static_cast<float>(image*16),0,16,16));
    }

    for (const auto& entry : level.heaps()) {
        const Heap* heap = entry.second.get();
        if (!heap || heap->empty() || heap->type == Heap::Type::Hidden) continue;
        const int pos = entry.first;
        if (!level.inBoundsCell(pos) || !visible[static_cast<std::size_t>(pos)]) continue;

        int image = ItemSpriteSheet::SMTH;
        switch (heap->type) {
            case Heap::Type::Heap:
            case Heap::Type::ForSale:
                if (const Item* item = heap->peek()) image = item->image();
                break;
            case Heap::Type::Chest:
            case Heap::Type::Mimic: image = ItemSpriteSheet::CHEST; break;
            case Heap::Type::LockedChest: image = ItemSpriteSheet::LOCKED_CHEST; break;
            case Heap::Type::CrystalChest: image = ItemSpriteSheet::CRYSTAL_CHEST; break;
            case Heap::Type::Tomb:
            case Heap::Type::Skeleton: image = ItemSpriteSheet::BONES; break;
            case Heap::Type::Hidden: break;
        }

        const int x = pos % Level::Width;
        const int y = pos / Level::Width;
        const int atlasX = (image % 8) * 16;
        const int atlasY = (image / 8) * 16;
        float itemPopOffset = 0.0f;
        const auto itemPop = itemPopTimers_.find(pos);
        if (itemPop != itemPopTimers_.end()) {
            const float progress = 1.0f - std::clamp(itemPop->second / 0.38f, 0.0f, 1.0f);
            itemPopOffset = -std::sin(progress * 3.14159265f) * TileSize * 0.32f;
        }

        renderer.texture(
            *itemsSheet_,
            x * TileSize - cameraX_,
            ViewportY + y * TileSize - cameraY_ + itemPopOffset,
            16 * spriteScale,
            16 * spriteScale,
            TextureRegion(static_cast<float>(atlasX),static_cast<float>(atlasY),16,16)
        );
    }

    if (touchTarget_ >= 0 &&
        level.inBoundsCell(touchTarget_) &&
        (visible[touchTarget_] || seen[touchTarget_])) {

        const int x = touchTarget_ % Level::Width;
        const int y = touchTarget_ / Level::Width;

        renderer.outlineRect(
            x * TileSize - cameraX_ + 1,
            ViewportY + y * TileSize - cameraY_ + 1,
            TileSize - 2,
            TileSize - 2,
            1,
            0xFFFF66E0
        );
    }

    for (const auto& mob : level.mobs()) {
        if (!mob || !mob->isAlive() ||
            !visible[static_cast<std::size_t>(mob->pos)]) {
            continue;
        }

        const auto visualIt = mobVisuals_.find(mob->id());
        const float x = visualIt != mobVisuals_.end()
            ? visualIt->second.x : static_cast<float>(mob->pos % Level::Width);
        const float y = visualIt != mobVisuals_.end()
            ? visualIt->second.y : static_cast<float>(mob->pos / Level::Width);

        if (dynamic_cast<
                const MirrorImage*
            >(mob.get())) {

            Texture2D* mirrorSheet =
                heroSheets_[
                    static_cast<int>(
                        dungeon_
                            .hero()
                            .heroClass()
                    )
                ];

            const bool mirrorMoving =
                std::fabs(x - static_cast<float>(mob->pos % Level::Width)) > 0.02f ||
                std::fabs(y - static_cast<float>(mob->pos / Level::Width)) > 0.02f;
            int mirrorFrame = mirrorMoving
                ? 2 + static_cast<int>(heroAnimationTime_ * 20.0f) % 6
                : heroIdleFrame_;

            renderer.texture(
                *mirrorSheet,
                x * TileSize -
                    cameraX_ + 2 * spriteScale,
                ViewportY +
                    y * TileSize -
                    cameraY_ + 1 * spriteScale,
                12 * spriteScale,
                15 * spriteScale,
                TextureRegion(
                    static_cast<float>(
                        mirrorFrame *
                        12
                    ),
                    0,
                    12,
                    15
                ),
                0.55f,
                1.15f
            );

        } else {
            Texture2D* mobSheet = ratSheet_;
            int frameWidth = 16;
            int frameHeight = 15;
            float xOffset = 0.0f;
            int mobFrame = ratIdleFrame_;
            const bool mobMoving =
                std::fabs(x - static_cast<float>(mob->pos % Level::Width)) > 0.02f ||
                std::fabs(y - static_cast<float>(mob->pos / Level::Width)) > 0.02f;
            const bool mobAttacking =
                mobAttackTimer_ > 0.0f &&
                mob->pos == attackingMobCell_;
            if (dynamic_cast<const Gnoll*>(mob.get())) {
                mobSheet = gnollSheet_;
                frameWidth = 12;
                frameHeight = 15;
                xOffset = 2.0f;
                if (mobAttacking) {
                    static const int frames[] = {2, 3, 0};
                    mobFrame = frames[std::min(2, static_cast<int>((0.34f - mobAttackTimer_) * 12.0f))];
                } else if (mobMoving) {
                    mobFrame = 4 + static_cast<int>(ratAnimationTime_ * 12.0f) % 4;
                }
            } else if (const Goo* goo = dynamic_cast<const Goo*>(mob.get())) {
                mobSheet = gooSheet_;
                frameWidth = 20;
                frameHeight = 14;
                xOffset = -2.0f;
                if (mobAttacking) {
                    static const int frames[] = {5, 0, 6};
                    mobFrame = frames[std::min(2, static_cast<int>((0.34f - mobAttackTimer_) * 10.0f))];
                } else {
                    mobFrame = static_cast<int>(ratAnimationTime_ * 10.0f) % 2;
                }
                if (goo->pumpedUp()) mobFrame = static_cast<int>(ratAnimationTime_ * 20.0f) % 2;
            } else if (dynamic_cast<const Swarm*>(mob.get())) {
                mobSheet = swarmSheet_;
                frameWidth = 16;
                frameHeight = 16;
                if (mobAttacking) {
                    mobFrame = 6 + std::min(3, static_cast<int>((0.34f - mobAttackTimer_) * 20.0f));
                } else {
                    mobFrame = static_cast<int>(ratAnimationTime_ * 15.0f) % 6;
                }
            } else if (dynamic_cast<const Skeleton*>(mob.get())) {
                mobSheet=skeletonSheet_;frameWidth=12;frameHeight=15;xOffset=2.0f;
                if(mobAttacking){static const int frames[]={14,15,16};mobFrame=frames[std::min(2,static_cast<int>((0.34f-mobAttackTimer_)*15.0f))];}
                else if(mobMoving)mobFrame=4+static_cast<int>(ratAnimationTime_*15.0f)%6;
                else {static const int frames[]={0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3};mobFrame=frames[static_cast<int>(ratAnimationTime_*12.0f)%16];}
            } else if (dynamic_cast<const Thief*>(mob.get())) {
                mobSheet=thiefSheet_;frameWidth=12;frameHeight=13;xOffset=2.0f;
                if(mobAttacking){static const int frames[]={10,11,12,0};mobFrame=frames[std::min(3,static_cast<int>((0.34f-mobAttackTimer_)*12.0f))];}
                else if(mobMoving){static const int frames[]={0,0,2,3,3,4};mobFrame=frames[static_cast<int>(ratAnimationTime_*15.0f)%6];}
                else mobFrame=static_cast<int>(ratAnimationTime_)%9==3?1:0;
            } else if (dynamic_cast<const Piranha*>(mob.get())) {
                mobSheet = piranhaSheet_;
                frameWidth = 12;
                frameHeight = 16;
                xOffset = 2.0f;
                if (mobAttacking) {
                    mobFrame = 3 + std::min(8, static_cast<int>((0.34f - mobAttackTimer_) * 27.0f));
                } else {
                    static const int frames[] = {0, 1, 2, 1};
                    mobFrame = frames[static_cast<int>(ratAnimationTime_ * 8.0f) % 4];
                }
            } else if(const ZoneMob* zoneMob=dynamic_cast<const ZoneMob*>(mob.get())){
                const int kind=static_cast<int>(zoneMob->kind());mobSheet=zoneMobSheets_[static_cast<std::size_t>(kind)];
                if(zoneMob->kind()==ZoneMobKind::Bandit){frameWidth=12;frameHeight=13;xOffset=2.0f;if(mobAttacking)mobFrame=10+std::min(2,static_cast<int>((0.34f-mobAttackTimer_)*10.0f));else if(mobMoving)mobFrame=2+static_cast<int>(ratAnimationTime_*12.0f)%3;}
                else if(zoneMob->kind()==ZoneMobKind::Shaman){frameWidth=12;frameHeight=15;xOffset=2.0f;if(mobAttacking){static const int frames[]={2,3,0};mobFrame=frames[std::min(2,static_cast<int>((0.34f-mobAttackTimer_)*12.0f))];}else if(mobMoving)mobFrame=4+static_cast<int>(ratAnimationTime_*12.0f)%4;else{static const int frames[]={0,0,0,1,0,0,1,1};mobFrame=frames[static_cast<int>(ratAnimationTime_*2.0f)%8];}}
                else if(zoneMob->kind()==ZoneMobKind::Bat){frameWidth=15;frameHeight=15;xOffset=.5f;if(mobAttacking){static const int frames[]={2,3,0,1};mobFrame=frames[std::min(3,static_cast<int>((0.34f-mobAttackTimer_)*12.0f))];}else mobFrame=static_cast<int>(ratAnimationTime_*8.0f)%2;}
                else if(zoneMob->kind()==ZoneMobKind::Brute||zoneMob->kind()==ZoneMobKind::Shielded){
                    frameWidth=12;frameHeight=16;xOffset=2.0f;
                    const int base=zoneMob->kind()==ZoneMobKind::Shielded?21:0;
                    if(mobAttacking){static const int f[]={2,3,0};mobFrame=base+f[std::min(2,static_cast<int>((0.34f-mobAttackTimer_)*10.0f))];}
                    else if(mobMoving)mobFrame=base+(zoneMob->kind()==ZoneMobKind::Shielded?4:4)+static_cast<int>(ratAnimationTime_*10.0f)%4;
                    else mobFrame=base+(static_cast<int>(ratAnimationTime_*4.0f)%2);
                } else if(zoneMob->kind()==ZoneMobKind::Monk||zoneMob->kind()==ZoneMobKind::Senior){
                    frameWidth=15;frameHeight=14;xOffset=.5f;
                    const int base=zoneMob->kind()==ZoneMobKind::Senior?18:0;
                    if(mobAttacking){static const int monkFrames[]={3,4,3,4},seniorFrames[]={2,3,2,3};const int* f=zoneMob->kind()==ZoneMobKind::Senior?seniorFrames:monkFrames;mobFrame=base+f[std::min(3,static_cast<int>((0.34f-mobAttackTimer_)*12.0f))];}
                    else if(mobMoving)mobFrame=base+(zoneMob->kind()==ZoneMobKind::Senior?10:11)+static_cast<int>(ratAnimationTime_*12.0f)%6;
                    else {static const int f[]={1,0,1,2};mobFrame=base+f[static_cast<int>(ratAnimationTime_*4.0f)%4];}
                } else if(zoneMob->kind()==ZoneMobKind::Undead){
                    frameWidth=12;frameHeight=16;xOffset=2.0f;
                    if(mobAttacking){static const int f[]={14,15,16};mobFrame=f[std::min(2,static_cast<int>((0.34f-mobAttackTimer_)*12.0f))];}
                    else if(mobMoving)mobFrame=4+static_cast<int>(ratAnimationTime_*12.0f)%6;
                    else {static const int f[]={0,0,0,1,2,3};mobFrame=f[static_cast<int>(ratAnimationTime_*4.0f)%6];}
                } else if(zoneMob->kind()==ZoneMobKind::RottingFist||zoneMob->kind()==ZoneMobKind::BurningFist){
                    frameWidth=24;frameHeight=17;xOffset=-4.0f;
                    if(mobAttacking&&zoneMob->kind()==ZoneMobKind::BurningFist){static const int f[]={0,5,6};mobFrame=f[std::min(2,static_cast<int>((0.34f-mobAttackTimer_)*10.0f))];}
                    else mobFrame=static_cast<int>(ratAnimationTime_*3.0f)%2;
                } else if(zoneMob->kind()==ZoneMobKind::Elemental){
                    frameWidth=12;frameHeight=14;xOffset=2.0f;
                    if(mobAttacking)mobFrame=4+std::min(2,static_cast<int>((0.34f-mobAttackTimer_)*12.0f));
                    else if(mobMoving){static const int f[]={0,1,3};mobFrame=f[static_cast<int>(ratAnimationTime_*9.0f)%3];}
                    else mobFrame=static_cast<int>(ratAnimationTime_*10.0f)%3;
                } else if(zoneMob->kind()==ZoneMobKind::Succubus||zoneMob->kind()==ZoneMobKind::Warlock){
                    frameWidth=12;frameHeight=15;xOffset=2.0f;
                    if(mobAttacking){static const int warlockFrames[]={0,5,6},succubusFrames[]={9,10,11};const int* f=zoneMob->kind()==ZoneMobKind::Succubus?succubusFrames:warlockFrames;mobFrame=f[std::min(2,static_cast<int>((0.34f-mobAttackTimer_)*12.0f))];}
                    else if(mobMoving)mobFrame=(zoneMob->kind()==ZoneMobKind::Succubus?3:0)+static_cast<int>(ratAnimationTime_*12.0f)%4;
                    else mobFrame=static_cast<int>(ratAnimationTime_*3.0f)%2;
                } else if(zoneMob->kind()==ZoneMobKind::Scorpio||zoneMob->kind()==ZoneMobKind::Acidic){
                    // Scorpio.png uses 18x17 cells, not the default 16x16.
                    // The previous fallback cut one/two pixels off every frame.
                    frameWidth=18;frameHeight=17;xOffset=-1.0f;
                    const bool acidic=zoneMob->kind()==ZoneMobKind::Acidic;
                    if(mobAttacking){static const int normal[]={0,3,4},acid[]={14,17,18};const int* f=acidic?acid:normal;mobFrame=f[std::min(2,static_cast<int>((0.34f-mobAttackTimer_)*12.0f))];}
                    else if(mobMoving){static const int normal[]={5,5,6,6},acid[]={19,20};const int* f=acidic?acid:normal;mobFrame=f[static_cast<int>(ratAnimationTime_*(acidic?4.0f:8.0f))%(acidic?2:4)];}
                    else {static const int normal[]={0,0,0,0,0,0,0,0,1,2,1,2,1,2},acid[]={14,14,14,14,14,14,14,14,15,16,15,16,15,16};const int* f=acidic?acid:normal;mobFrame=f[static_cast<int>(ratAnimationTime_*12.0f)%14];}
                }
                else{frameWidth=16;frameHeight=16;if(mobAttacking)mobFrame=6+std::min(2,static_cast<int>((0.34f-mobAttackTimer_)*10.0f));else if(mobMoving)mobFrame=2+static_cast<int>(ratAnimationTime_*10.0f)%4;else mobFrame=static_cast<int>(ratAnimationTime_*4.0f)%2;}
                const int zoneFrames=std::max(1,(mobSheet->width()/frameWidth)*(mobSheet->height()/frameHeight));
                mobFrame%=zoneFrames;
            } else if(const ZoneBoss* boss=dynamic_cast<const ZoneBoss*>(mob.get())){
                mobSheet=zoneBossSheets_[static_cast<std::size_t>(boss->kind())];
                if(boss->kind()==ZoneBossKind::Tengu){frameWidth=14;frameHeight=16;xOffset=1.0f;if(mobAttacking){static const int f[]={6,7,7,0};mobFrame=f[std::min(3,static_cast<int>((0.34f-mobAttackTimer_)*12.0f))];}else if(mobMoving)mobFrame=2+static_cast<int>(ratAnimationTime_*12.0f)%4;else mobFrame=static_cast<int>(ratAnimationTime_*3.0f)%2;}
                else if(boss->kind()==ZoneBossKind::DM300){frameWidth=22;frameHeight=20;xOffset=-3.0f;if(mobAttacking)mobFrame=4+std::min(2,static_cast<int>((0.34f-mobAttackTimer_)*12.0f));else if(mobMoving)mobFrame=2+static_cast<int>(ratAnimationTime_*8.0f)%2;else mobFrame=static_cast<int>(ratAnimationTime_*5.0f)%2;}
                else if(boss->kind()==ZoneBossKind::Yog){frameWidth=20;frameHeight=19;xOffset=-2.0f;static const int f[]={0,1,2,2,1,0,3,4,4,3,0,5,6,6,5};mobFrame=f[static_cast<int>(ratAnimationTime_*4.0f)%15];}
                else {frameWidth=16;frameHeight=16;if(mobAttacking)mobFrame=9+std::min(2,static_cast<int>((0.34f-mobAttackTimer_)*12.0f));else if(mobMoving)mobFrame=3+static_cast<int>(ratAnimationTime_*10.0f)%6;else mobFrame=static_cast<int>(ratAnimationTime_*3.0f)%3;}
            } else if(const DungeonNPC*npc=dynamic_cast<const DungeonNPC*>(mob.get())){
                mobSheet=npcSheets_[static_cast<std::size_t>(npc->kind())];
                if(npc->kind()==NPCKind::Wandmaker){
                    frameWidth=12;frameHeight=14;xOffset=2.0f;
                    static const int frames[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,3,3,3,3,3,2,1};
                    mobFrame=frames[static_cast<int>(ratAnimationTime_*10.0f)%24];
                }else if(npc->kind()==NPCKind::Blacksmith){
                    frameWidth=13;frameHeight=16;xOffset=1.5f;
                    static const int frames[]={0,0,0,0,0,0,0,1,2,2,2,3};
                    mobFrame=frames[static_cast<int>(ratAnimationTime_*8.0f)%12];
                }else if(npc->kind()==NPCKind::Imp){
                    frameWidth=12;frameHeight=14;xOffset=2.0f;
                    static const int frames[]={0,1,2,3,0,1,2,3,0,0,0,4,4,4,4,4};
                    mobFrame=frames[static_cast<int>(ratAnimationTime_*10.0f)%16];
                }else{frameWidth=16;frameHeight=16;const int frames=std::max(1,mobSheet->width()/16);mobFrame=static_cast<int>(ratAnimationTime_*3.0f)%frames;}
            } else if (dynamic_cast<const Crab*>(mob.get())) {
                mobSheet = crabSheet_;
                frameWidth = 16;
                frameHeight = 16;
                if (mobAttacking) {
                    mobFrame = 7 + std::min(2, static_cast<int>((0.34f - mobAttackTimer_) * 12.0f));
                } else if (mobMoving) {
                    mobFrame = 3 + static_cast<int>(ratAnimationTime_ * 15.0f) % 4;
                } else {
                    static const int frames[] = {0, 1, 0, 2};
                    mobFrame = frames[static_cast<int>(ratAnimationTime_ * 5.0f) % 4];
                }
            } else if (mobAttacking) {
                static const int frames[] = {2, 3, 4, 5, 0};
                mobFrame = frames[std::min(4, static_cast<int>((0.34f - mobAttackTimer_) * 15.0f))];
            } else if (mobMoving) {
                mobFrame = 6 + static_cast<int>(ratAnimationTime_ * 10.0f) % 5;
            }
            if (dynamic_cast<const Albino*>(mob.get())) mobFrame += 16;

            renderer.texture(
                *mobSheet,
                x * TileSize - cameraX_ + xOffset * spriteScale,
                ViewportY + y * TileSize - cameraY_ +
                    (16 - frameHeight) * spriteScale,
                frameWidth * spriteScale,
                frameHeight * spriteScale,
                TextureRegion(
                    static_cast<float>((mobFrame % std::max(1, mobSheet->width() / frameWidth)) * frameWidth),
                    static_cast<float>((mobFrame / std::max(1, mobSheet->width() / frameWidth)) * frameHeight),
                    static_cast<float>(frameWidth),
                    static_cast<float>(frameHeight)
                )
            );
        }

        // Minimal platform presentation of actual Char HP state.
        const float hpRatio = mob->HT > 0
            ? static_cast<float>(mob->HP) / static_cast<float>(mob->HT)
            : 0.0f;

        renderer.rect(
            x * TileSize - cameraX_ + 1 * spriteScale,
            ViewportY + y * TileSize - cameraY_,
            14 * spriteScale,
            std::max(1.0f, spriteScale),
            0x3A0C0CFF
        );

        renderer.rect(
            x * TileSize - cameraX_ + 1 * spriteScale,
            ViewportY + y * TileSize - cameraY_,
            14.0f * spriteScale * std::max(0.0f, hpRatio),
            std::max(1.0f, spriteScale),
            0xC94A4AFF
        );

        // The developer arena is a visual inspection tool: identify each
        // atlas directly in-game without cluttering normal dungeon floors.
        if (dungeon_.developerTestRoom() && font_) {
            font_->drawCentered(
                renderer, Localization::text(mob->name),
                x * TileSize - cameraX_ + TileSize * 0.5f,
                ViewportY + y * TileSize - cameraY_ + 17.0f * spriteScale,
                1, 0.42f, 1
            );
        }
    }

    for (const CorpseVisual& corpse : corpses_) {
        if (!corpse.sheet || corpse.cell < 0) continue;
        const int columns = std::max(1, corpse.sheet->width() / corpse.frameWidth);
        const int x = corpse.cell % Level::Width;
        const int y = corpse.cell / Level::Width;
        renderer.texture(*corpse.sheet, x * TileSize - cameraX_ + corpse.xOffset * spriteScale,
            ViewportY + y * TileSize - cameraY_ + (16 - corpse.frameHeight) * spriteScale,
            corpse.frameWidth * spriteScale, corpse.frameHeight * spriteScale,
            TextureRegion(static_cast<float>((corpse.frame % columns) * corpse.frameWidth),
                static_cast<float>((corpse.frame / columns) * corpse.frameHeight),
                static_cast<float>(corpse.frameWidth), static_cast<float>(corpse.frameHeight)),
            std::clamp(corpse.fadeRemaining / 3.0f, 0.0f, 1.0f));
    }

    if (mobDeathTimer_ > 0.0f && mobDeathCell_ >= 0 && mobDeathSheet_) {
        const int frameCount = mobDeathLastFrame_ - mobDeathFirstFrame_ + 1;
        const int frame = mobDeathFirstFrame_ + std::min(
            frameCount - 1,
            static_cast<int>((0.5f - mobDeathTimer_) * 10.0f));
        const int columns = std::max(1, mobDeathSheet_->width() / mobDeathFrameWidth_);
        const int x = mobDeathCell_ % Level::Width;
        const int y = mobDeathCell_ / Level::Width;
        renderer.texture(*mobDeathSheet_, x * TileSize - cameraX_ + mobDeathXOffset_ * spriteScale,
            ViewportY + y * TileSize - cameraY_ + (16 - mobDeathFrameHeight_) * spriteScale,
            mobDeathFrameWidth_ * spriteScale, mobDeathFrameHeight_ * spriteScale,
            TextureRegion(static_cast<float>((frame % columns) * mobDeathFrameWidth_),
                static_cast<float>((frame / columns) * mobDeathFrameHeight_),
                static_cast<float>(mobDeathFrameWidth_), static_cast<float>(mobDeathFrameHeight_)));
    }

    for (const WaterRipple& ripple : waterRipples_) {
        if (ripple.cell < 0 || !level.inBoundsCell(ripple.cell)) continue;
        const float progress = 1.0f - std::clamp(ripple.remaining / 0.55f, 0.0f, 1.0f);
        const float size = TileSize * (0.25f + progress * 0.60f);
        const int x = ripple.cell % Level::Width;
        const int y = ripple.cell / Level::Width;
        renderer.texture(*effectsSheet_,
            x * TileSize - cameraX_ + (TileSize - size) * 0.5f,
            ViewportY + y * TileSize - cameraY_ + (TileSize - size) * 0.5f,
            size, size,
            TextureRegion(0, 0, 16, 16),
            (1.0f - progress) * 0.70f,
            1.0f,
            false,
            0xB8E8FFFF);
    }

    for (const ThrownItemVisual& thrown : thrownItems_) {
        if (!level.inBoundsCell(thrown.from) || !level.inBoundsCell(thrown.to)) continue;
        const float progress = 1.0f - std::clamp(thrown.remaining / 0.24f, 0.0f, 1.0f);
        const float fromX = static_cast<float>(thrown.from % Level::Width);
        const float fromY = static_cast<float>(thrown.from / Level::Width);
        const float toX = static_cast<float>(thrown.to % Level::Width);
        const float toY = static_cast<float>(thrown.to / Level::Width);
        const float x = (fromX + (toX - fromX) * progress) * TileSize - cameraX_;
        const float y = ViewportY + (fromY + (toY - fromY) * progress) * TileSize - cameraY_;
        const int image = thrown.image;
        renderer.texture(*itemsSheet_, x + (TileSize - 16 * spriteScale) * .5f,
                         y + (TileSize - 16 * spriteScale) * .5f, 16 * spriteScale, 16 * spriteScale,
                         TextureRegion(static_cast<float>((image % 8) * 16), static_cast<float>((image / 8) * 16), 16, 16));
    }

    for (const ImpactEffect& effect : impactEffects_) {
        if (!level.inBoundsCell(effect.cell)) continue;
        const float progress = 1.0f - std::clamp(effect.remaining / effect.duration, 0.0f, 1.0f);
        const float size = TileSize * (0.35f + effect.radius * progress);
        const float x = (effect.cell % Level::Width) * TileSize - cameraX_ + (TileSize - size) * .5f;
        const float y = ViewportY + (effect.cell / Level::Width) * TileSize - cameraY_ + (TileSize - size) * .5f;
        const uint32_t alpha = static_cast<uint32_t>((1.0f - progress) * 230.0f) & 0xFFu;
        const uint32_t color = (effect.color & 0xFFFFFF00u) | alpha;
        renderer.rect(x, y, size, size, color);
        renderer.outlineRect(x - size * .15f, y - size * .15f, size * 1.3f, size * 1.3f, 1, color);
    }

    const Hero& hero = dungeon_.hero();
    Texture2D* heroSheet = heroSheets_[static_cast<int>(hero.heroClass())];

    const float heroX = heroVisual_.x;
    const float heroY = heroVisual_.y;
    const bool heroMoving =
        std::fabs(heroX - static_cast<float>(hero.pos % Level::Width)) > 0.02f ||
        std::fabs(heroY - static_cast<float>(hero.pos / Level::Width)) > 0.02f;
    int heroFrame = heroIdleFrame_;
    const Armor* wornArmor=hero.belongings().armor();
    const int armorTier=std::max(0,std::min(7,wornArmor?wornArmor->tier():0));
    if (heroAttackTimer_ > 0.0f) {
        static const int attackFrames[] = {13, 14, 15, 0};
        heroFrame = attackFrames[std::min(3, static_cast<int>((0.28f - heroAttackTimer_) * 15.0f))];
    } else if (heroOperateTimer_ > 0.0f) {
        static const int operateFrames[] = {16, 17, 16, 17};
        heroFrame = operateFrames[std::min(3, static_cast<int>((0.5f - heroOperateTimer_) * 8.0f))];
    } else if (heroMoving) {
        heroFrame = 2 + static_cast<int>(heroAnimationTime_ * 20.0f) % 6;
    }

    if(hero.isAlive()){
        renderer.texture(*heroSheet,heroX*TileSize-cameraX_+2*spriteScale,
            ViewportY+heroY*TileSize-cameraY_+spriteScale,12*spriteScale,15*spriteScale,
            TextureRegion(static_cast<float>(heroFrame*12),static_cast<float>(armorTier*15),12,15),
            1.0f,1.0f,heroFacingLeft_);
    }else{
        const int tomb=ItemSpriteSheet::TOMB;
        renderer.texture(*itemsSheet_,heroX*TileSize-cameraX_,ViewportY+heroY*TileSize-cameraY_,
            16*spriteScale,16*spriteScale,TextureRegion(static_cast<float>((tomb%8)*16),static_cast<float>((tomb/8)*16),16,16));
    }

    if (specksSheet_ != nullptr) {
        for (const HeroSpeck& speck : heroSpecks_) {
            const float progress = 1.0f - std::clamp(speck.remaining / speck.duration, 0.0f, 1.0f);
            const float pulse = std::sqrt(std::max(0.0f, (progress < 0.5f ? progress : 1.0f - progress) * 2.0f));
            float scale = 1.0f;
            if (speck.frame == 3 || speck.frame == 4) scale = std::max(0.35f, pulse * 1.6f);
            else if (speck.frame == 5) scale = 0.45f + progress * 1.8f;
            else if (speck.frame == 10) scale = 0.65f + pulse * .55f;
            else if (speck.frame == 13) scale = 0.7f + progress * 1.1f;
            const float size = 7.0f * spriteScale * scale;
            float fade = progress < 0.70f ? 1.0f : (1.0f - progress) / 0.30f;
            if (speck.frame == 1 || speck.frame == 11) fade *= std::max(0.0f, 1.0f - progress * progress);
            renderer.texture(
                *specksSheet_,
                speck.x * TileSize - cameraX_ - size * 0.5f,
                ViewportY + speck.y * TileSize - cameraY_ - size * 0.5f,
                size,
                size,
                TextureRegion(static_cast<float>(speck.frame * 7), 0.0f, 7.0f, 7.0f),
                std::clamp(fade, 0.0f, 1.0f)
            );
        }
    }

    if (hitEffectTimer_ > 0.0f &&
        hitEffectCell_ >= 0 && level.inBoundsCell(hitEffectCell_)) {
        const float effectProgress = hitEffectTimer_ / 0.8f;
        const float effectScale = spriteScale * (1.0f + effectProgress);
        const int effectX = hitEffectCell_ % Level::Width;
        const int effectY = hitEffectCell_ / Level::Width;
        renderer.texture(
            *effectsSheet_,
            effectX * TileSize - cameraX_ + (TileSize - 16.0f * effectScale) * 0.5f,
            ViewportY + effectY * TileSize - cameraY_ + (TileSize - 8.0f * effectScale) * 0.5f,
            16.0f * effectScale,
            8.0f * effectScale,
            TextureRegion(16, 8, 16, 8),
            effectProgress,
            1.0f
        );
    }

    if(searchEffectTimer_>0.0f&&searchEffectCell_>=0&&level.inBoundsCell(searchEffectCell_)){
        const float progress=1.0f-searchEffectTimer_/0.65f;
        const float size=std::max(2.0f,TileSize*(1.0f-progress));
        const uint32_t alpha=static_cast<uint32_t>((1.0f-progress)*220.0f)&0xFFu;
        const uint32_t color=0xD8E8A000u|alpha;
        const int cx=searchEffectCell_%Level::Width,cy=searchEffectCell_/Level::Width;
        for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx){
            if(dx==0&&dy==0)continue;
            const float px=(cx+dx)*TileSize-cameraX_+(TileSize-size)*0.5f;
            const float py=ViewportY+(cy+dy)*TileSize-cameraY_+(TileSize-size)*0.5f;
            renderer.outlineRect(px,py,size,size,1.0f,color);
        }
    }

    if ((pendingTargetItem_ || pendingQuickSlot_) && targetCursor_ >= 0 && level.inBoundsCell(targetCursor_)) {
        const int x = targetCursor_ % Level::Width;
        const int y = targetCursor_ / Level::Width;
        const float pulse = 0.65f + 0.35f * std::sin(heroAnimationTime_ * 9.0f);
        renderer.outlineRect(x * TileSize - cameraX_ + 1, ViewportY + y * TileSize - cameraY_ + 1,
                             TileSize - 2, TileSize - 2, 2, 0xF0D35AFF);
        font_->drawCentered(renderer, Localization::text("TARGET"), x * TileSize - cameraX_ + TileSize * .5f,
                            ViewportY + y * TileSize - cameraY_ - 9, 1, pulse, 1);
    }

    const float hpRatio=hero.HT>0?std::max(0.0f,std::min(1.0f,static_cast<float>(hero.HP)/hero.HT)):0.0f;
    const float xpRatio=hero.maxExp()>0?std::max(0.0f,std::min(1.0f,static_cast<float>(hero.exp())/hero.maxExp())):0.0f;
    constexpr float hudX=150.0f,hudW=180.0f,hpY=246.0f;
    char hpText[32];
    snprintf(hpText,sizeof(hpText),"HP %d/%d",hero.HP,hero.HT);
    renderer.rect(hudX,hpY,hudW,12,0x292D32F2);
    renderer.rect(hudX,hpY,hudW*hpRatio,12,0xB92F36FF);
    renderer.outlineRect(hudX,hpY,hudW,12,1,0xC3C7C4FF);
    font_->drawCentered(renderer,hpText,hudX+hudW*0.5f,hpY+2,1.0f,1.0f,1.0f);
    renderer.rect(hudX,260,hudW,5,0x22272CF2);
    renderer.rect(hudX,260,hudW*xpRatio,5,0x397FBFFF);
    renderer.outlineRect(hudX,260,hudW,5,1,0xAAB7C2FF);
    const Keyring* hudKeys=hero.belongings().getItem<Keyring>();
    char floorInfo[64];snprintf(floorInfo,sizeof(floorInfo),"DUNGEON %d   KEYS %d",dungeon_.depth(),hudKeys?hudKeys->count():0);
    font_->draw(renderer,floorInfo,6,6,1.0f,0.9f,1);

    // Keep UIGame as one authored sprite: its right-hand controls still land
    // at (425,147), while the rest of the sheet is no longer clipped away.
    renderer.fullTexture(*uiGameSheet_,149,83,336,190);

    if(hero.upgradePoints()>0){
        constexpr float levelUpX=hudX+hudW+7.0f;
        constexpr float levelUpY=244.0f;
        renderer.texture(*levelUpSheet_,levelUpX,levelUpY,16,16,TextureRegion(0,0,16,16));
        char pointCount[16];snprintf(pointCount,sizeof(pointCount),"%d",hero.upgradePoints());
        font_->draw(renderer,pointCount,levelUpX+11.0f,levelUpY-6.0f,1.0f,0.8f,1.0f);
    }

    std::vector<int> buffIcons=BuffIndicator::iconsFor(hero);
    if(hitEffectTimer_>0.0f)buffIcons.push_back(BuffIndicator::Bleeding);
    const float buffRowWidth=buffIcons.empty()?0.0f:14.0f+static_cast<float>(buffIcons.size()-1)*15.0f;
    const float firstBuffX=240.0f-buffRowWidth*0.5f;

    for (std::size_t i = 0;
         i < buffIcons.size();
         ++i) {

        const int icon =
            buffIcons[i];

        const int atlasX =
            (icon %
             BuffIndicator::AtlasColumns) *
            BuffIndicator::Size;

        const int atlasY =
            (icon /
             BuffIndicator::AtlasColumns) *
            BuffIndicator::Size;

        renderer.texture(
            *buffsSheet_,
            firstBuffX +
                static_cast<float>(i) *
                15.0f,
            230.0f,
            14.0f,
            14.0f,
            TextureRegion(
                static_cast<float>(atlasX),
                static_cast<float>(atlasY),
                7.0f,
                7.0f
            )
        );
    }

    // The quick-slot frame is now authored into UIGame. Its red alignment
    // marker is centered at source (259.5, 170.5); UIGame itself is drawn at
    // (149, 83), so item sprites must be centered at (408.5, 253.5).
    if (const Item* quickItem = hero.primaryQuickSlot().select(hero)) {
        const int image = quickItem->image();
        renderer.texture(*itemsSheet_, 400.5f, 245.5f, 16, 16,
                         TextureRegion(static_cast<float>((image % 8) * 16), static_cast<float>((image / 8) * 16), 16, 16));
        if (quickItem->quantity() > 1) {
            char quantity[8]; snprintf(quantity, sizeof(quantity), "%d", quickItem->quantity());
            // The glyphs have a -1px source Y offset at half-scale.  A
            // half-logical-pixel baseline lands them exactly on output pixels.
            font_->drawColored(renderer, quantity, 413.0f, 253.5f, 0xFFFFFFFF, 1.0f, 1.0f);
        }
    }

    Heap* standingHeap=level.heapAt(hero.pos);
    if(standingHeap&&!standingHeap->empty()&&standingHeap->type!=Heap::Type::Hidden){
        const bool closed=standingHeap->type==Heap::Type::Chest||standingHeap->type==Heap::Type::LockedChest||standingHeap->type==Heap::Type::CrystalChest||standingHeap->type==Heap::Type::Mimic;
        const Item*standing=standingHeap->peek();
        // The pickup slot in UIGame has its marker centered at source
        // (288.5,170.5), i.e. screen (437.5,253.5) after the full-sheet offset.
        // Keep the 16px item centered on that marker.
        if (!dynamic_cast<const KindOfWeapon*>(standing)) {
            const int image=closed?ItemSpriteSheet::SMTH:(standing?standing->image():ItemSpriteSheet::SMTH);
            renderer.texture(*itemsSheet_,429.5f,245.5f,16,16,TextureRegion(static_cast<float>((image%8)*16),static_cast<float>((image/8)*16),16,16));
        }
    }

    if (!messageLog_.empty()) {
        const float bottom=225.0f;
        const float top=bottom-static_cast<float>(messageLog_.size())*9.0f;
        int row=0;
        for(const std::string& line:messageLog_){
            std::string shown=line.size()>24?line.substr(0,21)+"...":line;
            font_->drawColored(renderer,Localization::text(shown),8,top+row*9,messageLogColors_[static_cast<std::size_t>(row)],1.0f,1.0f);
            ++row;
        }
    }

    if (inventory_.openState()) {
        renderInventory(renderer);
    }

    if (gameMenuOpen_) {
        renderGameMenu(renderer);
    }
    if(skillMenuOpen_)renderSkillMenu(renderer);
    if(cheatMenuOpen_)renderCheatMenu(renderer);
    if(questRewardOpen_)renderQuestReward(renderer);
    if(amuletChoiceOpen_)renderAmuletChoice(renderer);
    if(npcDialogueOpen_)renderNpcDialogue(renderer);
    if(introTimer_>0.0f){const float fade=std::min(1.0f,introTimer_/0.7f);renderer.rect(0,0,Renderer2D::LogicalWidth,Renderer2D::LogicalHeight,0x00000000u|static_cast<uint32_t>(210*fade));const std::string floorText=Localization::text("FLOOR")+" "+numberToString(dungeon_.depth());font_->drawCentered(renderer,floorText,240,105,2,1.35f*fade,1);font_->drawCentered(renderer,Localization::text("THE DUNGEON AWAITS"),240,140,1,.85f*fade,1);}
    if(interlevelTimer_>0.0f){const float fade=std::clamp(interlevelTimer_/0.7f,0.0f,1.0f);renderer.rect(0,0,Renderer2D::LogicalWidth,Renderer2D::LogicalHeight,0x00000000u|static_cast<uint32_t>(255*fade));char floorText[48];snprintf(floorText,sizeof(floorText),"%s %d",Localization::text("FLOOR").c_str(),interlevelDepth_);font_->drawCentered(renderer,Localization::text(interlevelIsTeleport_ ? "TELEPORTING" : "DESCENDING"),240,108,1,1.15f*fade,1);font_->drawCentered(renderer,floorText,240,143,2,1.05f*fade,1);}
    if(pendingFallCell_>=0){renderer.rect(105,89,270,82,0x101018F5);renderer.outlineRect(105,89,270,82,2,0xD9B75BFF);font_->drawCentered(renderer,Localization::text("JUMP INTO THE CHASM?"),240,104,1,1.0f,1);font_->drawCentered(renderer,Localization::text("You will fall to a lower floor"),240,126,1,.65f,1);drawVitaButtonHint(renderer,*font_,*uiButtonsSheet_,{{VitaButtonIcon::Cross," "+Localization::text("jump")},{VitaButtonIcon::Circle," "+Localization::text("cancel")}},240,150,.65f,.65f);}
    if(surfaceTimer_>0.0f){renderer.rect(0,0,Renderer2D::LogicalWidth,Renderer2D::LogicalHeight,0x000000D8);font_->drawCentered(renderer,Localization::text("YOU RETURN TO THE SURFACE"),240,108,1,1.1f,1);font_->drawCentered(renderer,Localization::text("The Amulet's light guides you home"),240,139,1,.75f,1);}
    if(dungeon_.won()&&surfaceTimer_<=0.0f){
        renderer.rect(60,72,360,116,0x101018E8);renderer.outlineRect(60,72,360,116,2,0xE0C060FF);font_->drawCentered(renderer,Localization::text("YOU FOUND THE AMULET OF YENDOR"),240,92,1.0f,1.2f,1.0f);font_->drawCentered(renderer,Localization::text("The dungeon is conquered"),240,119,1.0f,0.9f,1.0f);const std::string result=Localization::text("Depth")+" "+numberToString(Progression::instance().run().depth)+"  "+Localization::text("Kills")+" "+numberToString(Progression::instance().run().kills)+"  "+Localization::text("Gold")+" "+numberToString(Progression::instance().run().gold);font_->drawCentered(renderer,result,240,142,1.0f,.72f,1.0f);drawVitaButtonHint(renderer,*font_,*uiButtonsSheet_,{{VitaButtonIcon::Cross," "+Localization::text("main menu")}},240,166,.62f,.62f);
    }
    if(!hero.isAlive()) renderDeathSummary(renderer);
    renderBadgeNotification(renderer);
}

void GameScene::handleDeathSummary(const VitaInput& input, SceneManager& scenes) {
    if (!OnlineServiceEnabled) {
        if (input.pressed(SCE_CTRL_CROSS) || input.pressed(SCE_CTRL_CIRCLE) || input.pressed(SCE_CTRL_START)) scenes.request(SceneId::Title);
        return;
    }
    if (input.pressed(SCE_CTRL_UP) || input.pressed(SCE_CTRL_DOWN) || input.pressed(SCE_CTRL_LEFT) || input.pressed(SCE_CTRL_RIGHT)) {
        deathSummarySelection_ = 1 - deathSummarySelection_;
        AudioSystem::instance().playSound("snd_click.mp3");
    }
    if (input.pressed(SCE_CTRL_CIRCLE) || input.pressed(SCE_CTRL_START)) {
        scenes.request(SceneId::Title);
        return;
    }
    if (!input.pressed(SCE_CTRL_CROSS) || deathOnlineLoading_) return;
    if (deathSummarySelection_ == 1) {
        scenes.request(SceneId::Title);
        return;
    }
    deathOnlineLoading_ = true;
    deathOnlineStatus_ = "Sending record...";
    const RunStatistics run = Progression::instance().run();
    deathOnlineRequest_.startSubmit(run);
}

void GameScene::renderDeathSummary(Renderer2D& renderer) {
    const RunStatistics& run = Progression::instance().run();
    renderer.rect(62,36,356,193,0x101018F5);
    renderer.outlineRect(62,36,356,193,2,0xC9B878FF);
    renderer.texture(*bannersSheet_,112,48,256,70,TextureRegion(0,105,128,35));
    static const char* heroes[] = {"Warrior", "Mage", "Rogue", "Huntress"};
    const int classIndex = static_cast<int>(run.heroClass);
    const char* heroName = classIndex >= 0 && classIndex < 4 ? heroes[classIndex] : "Warrior";
    const int score = run.gold + run.kills * 10 + run.depth * 100;
    char statsTop[96]; char statsBottom[96];
    snprintf(statsTop, sizeof(statsTop), "%s   %s %d   %s %d", Localization::text(heroName).c_str(), Localization::text("SCORE").c_str(), score, Localization::text("FLOOR").c_str(), run.depth);
    snprintf(statsBottom, sizeof(statsBottom), "%s %d   %s %d   %s %02d:%02d", Localization::text("KILLS").c_str(), run.kills, Localization::text("GOLD").c_str(), run.gold, Localization::text("TIME").c_str(),
                  static_cast<int>(run.duration) / 60, static_cast<int>(run.duration) % 60);
    font_->drawCentered(renderer, statsTop, 240, 116, 1, .72f, 1);
    font_->drawCentered(renderer, statsBottom, 240, 134, 1, .72f, 1);
    const char* options[] = {"Send to online leaderboard", "Return to main menu"};
    const int optionCount = OnlineServiceEnabled ? 2 : 1;
    for (int i = 0; i < optionCount; ++i) {
        const char* option = OnlineServiceEnabled ? options[i] : options[1];
        const float y = 166.0f + i * 23.0f;
        if (i == deathSummarySelection_) renderer.rect(104,y-4,272,18,0x6A5B35D8);
        font_->drawCentered(renderer, Localization::text(option), 240, y, 1, i == deathSummarySelection_ ? .78f : .62f, 1);
    }
    if (!deathOnlineStatus_.empty()) font_->drawCentered(renderer, Localization::text(deathOnlineStatus_), 240, 211, 1, .52f, 1);
}

void GameScene::renderSkillMenu(Renderer2D& renderer){
    const Hero& hero=dungeon_.hero();
    renderer.rect(0,0,Renderer2D::LogicalWidth,Renderer2D::LogicalHeight,0x000000B8);
    renderer.rect(42,35,396,194,0x171922F5);renderer.outlineRect(42,35,396,194,2,0xB6A56FFF);
    font_->drawCentered(renderer,Localization::text("HERO UPGRADES"),240,58,1,1.2f,1);
    const std::string pointsText=Localization::text("Available points:")+" "+numberToString(hero.upgradePoints());
    font_->drawCentered(renderer,pointsText,240,78,1,0.9f,1);
    const std::string values[4]={
        Localization::text("Health")+" +5   "+Localization::text("Rank")+" "+numberToString(hero.hpUpgrades()),
        Localization::text("Attack")+" +1   "+Localization::text("Rank")+" "+numberToString(hero.strengthUpgrades()),
        Localization::text("Endurance")+"   "+Localization::text("Rank")+" "+numberToString(hero.enduranceUpgrades()),
        Localization::text("Luck")+"        "+Localization::text("Rank")+" "+numberToString(hero.luckUpgrades())};
    const char* details[4]={
        "Raises maximum health by 5 and heals 5 HP.",
        "Adds 1 damage and 2 accuracy to every attack.",
        "Adds 3 backpack slots. Also slows hunger.",
        "Monster drops +20%. Chests more often contain equipment."};
    for(int i=0;i<4;++i){const float y=105.0f+i*25.0f;if(i==skillMenuSelection_)renderer.rect(60,y-5,168,20,0x6A5B35D8);font_->drawCentered(renderer,values[i],144,y,1,i==skillMenuSelection_?0.84f:0.65f,1);}
    renderer.rect(244,93,174,94,0x11121BE8);renderer.outlineRect(244,93,174,94,1,0x665B45FF);
    font_->drawCentered(renderer,values[skillMenuSelection_],331,110,1,0.82f,1);
    drawInventoryWrapped(*font_,renderer,Localization::text(details[skillMenuSelection_]),252,126,28,0.62f,4);
    const std::string capacity=Localization::text("Backpack:")+" "+numberToString(hero.backpackCapacity())+" "+Localization::text("slots");
    font_->drawCentered(renderer,capacity,331,177,1,0.58f,1);
    drawVitaButtonHint(renderer,*font_,*uiButtonsSheet_,{{VitaButtonIcon::DPad," choose"},{VitaButtonIcon::Cross," upgrade"},{VitaButtonIcon::Circle," close"}},240,213,.55f,.55f);
}

void GameScene::renderCheatMenu(Renderer2D& renderer) {
    static const char* values[] = {
        "God mode", "+100 gold", "+1 level", "Heal and cleanse",
        "Add random weapon", "Add random potion", "Add random scroll", "Add random wand",
        "Teleport to floor", "Open test room"
    };
    renderer.rect(0, 0, Renderer2D::LogicalWidth, Renderer2D::LogicalHeight, 0x000000C8);
    renderer.rect(96, 25, 288, 224, 0x251218F8);
    renderer.outlineRect(96, 25, 288, 224, 2, 0xE06B4FFF);
    font_->drawCentered(renderer, Localization::text("CHEAT MENU"), 240, 48, 1, 1.15f, 1);
    for (int i = 0; i < 10; ++i) {
        const float y = 68.0f + i * 17.0f;
        if (i == cheatMenuSelection_) renderer.rect(116, y - 4, 248, 15, 0x6A5B35D8);
        std::string label = Localization::text(values[i]);
        if (i == 0) label += dungeon_.hero().invulnerable() ? ": ON" : ": OFF";
        if (i == 8) label += " " + numberToString(cheatTeleportDepth_);
        font_->drawCentered(renderer, label, 240, y, 1, i == cheatMenuSelection_ ? .82f : .65f, 1);
    }
    drawVitaButtonHint(renderer,*font_,*uiButtonsSheet_,{{VitaButtonIcon::DPad," choose / set floor"},{VitaButtonIcon::Cross," apply"},{VitaButtonIcon::Circle," close"}},240,239,.55f,.55f);
}

void GameScene::handleQuestReward(const VitaInput& input) {
    if (dungeon_.pendingQuestRewardCount() > 1 && (input.pressed(SCE_CTRL_UP) || input.pressed(SCE_CTRL_DOWN) || input.pressed(SCE_CTRL_LEFT) || input.pressed(SCE_CTRL_RIGHT))) {
        questRewardSelection_ = 1 - questRewardSelection_;
        AudioSystem::instance().playSound("snd_click.mp3");
    }
    if (input.pressed(SCE_CTRL_CROSS) && dungeon_.claimPendingQuestReward(questRewardSelection_)) {
        questRewardOpen_ = false;
        AudioSystem::instance().playSound("snd_click.mp3");
    }
}

void GameScene::renderQuestReward(Renderer2D& renderer) {
    const int quest = dungeon_.pendingQuestReward();
    const std::array<std::string, 2> rewards = dungeon_.pendingQuestRewardNames();
    renderer.rect(0, 0, Renderer2D::LogicalWidth, Renderer2D::LogicalHeight, 0x000000B8);
    renderer.rect(82, 74, 316, 126, 0x171922F8);
    renderer.outlineRect(82, 74, 316, 126, 2, 0xC9B878FF);
    font_->drawCentered(renderer, Localization::text(quest == 0 ? "GHOST'S REWARD" : (quest == 1 ? "WANDMAKER'S REWARD" : "IMP'S REWARD")), 240, 89, 1, 1.05f, 1);
    const int rewardCount = dungeon_.pendingQuestRewardCount();
    font_->drawCentered(renderer, Localization::text(rewardCount == 1 ? "Your reward" : "Choose one reward"), 240, 108, 1, .7f, 1);
    for (int i = 0; i < rewardCount; ++i) {
        const float y = 134.0f + i * 25.0f;
        if (i == questRewardSelection_) renderer.rect(106, y - 4, 268, 18, 0x6A5B35D8);
        font_->drawCentered(renderer, rewards[static_cast<std::size_t>(i)], 240, y, 1, i == questRewardSelection_ ? .9f : .7f, 1);
    }
    if (rewardCount == 1) drawVitaButtonHint(renderer,*font_,*uiButtonsSheet_,{{VitaButtonIcon::Cross," accept"}},240,177,.6f,.6f);
    else drawVitaButtonHint(renderer,*font_,*uiButtonsSheet_,{{VitaButtonIcon::DPad," choose"},{VitaButtonIcon::Cross," accept"}},240,177,.6f,.6f);
}

void GameScene::handleAmuletChoice(const VitaInput& input) {
    if (input.pressed(SCE_CTRL_UP) || input.pressed(SCE_CTRL_DOWN) || input.pressed(SCE_CTRL_LEFT) || input.pressed(SCE_CTRL_RIGHT)) {
        amuletChoiceSelection_ = 1 - amuletChoiceSelection_;
        AudioSystem::instance().playSound("snd_click.mp3");
    }
    if (input.pressed(SCE_CTRL_CIRCLE)) {
        amuletChoiceOpen_ = false;
        return;
    }
    if (!input.pressed(SCE_CTRL_CROSS)) return;
    if (amuletChoiceSelection_ == 0) { dungeon_.completeRun(); Progression::instance().noteHappyEnd(); surfaceTimer_ = 1.5f; }
    amuletChoiceOpen_ = false;
    AudioSystem::instance().playSound("snd_click.mp3");
}

void GameScene::renderAmuletChoice(Renderer2D& renderer) {
    renderer.rect(0, 0, Renderer2D::LogicalWidth, Renderer2D::LogicalHeight, 0x000000B8);
    renderer.rect(65, 43, 350, 180, 0x171922F8);
    renderer.outlineRect(65, 43, 350, 180, 2, 0xE0C060FF);
    font_->drawCentered(renderer, Localization::text("AMULET OF YENDOR"), 240, 60, 1, 1.15f, 1);
    drawInventoryWrapped(*font_,renderer,"You finally hold the Amulet of Yendor. Its power can change the world, but ending the game is your choice.",88,80,43,.70f,7);
    static const char* choices[] = {"Claim victory", "Keep exploring"};
    for (int i = 0; i < 2; ++i) {
        const float y = 160.0f + i * 25.0f;
        if (i == amuletChoiceSelection_) renderer.rect(111, y - 4, 258, 18, 0x6A5B35D8);
        font_->drawCentered(renderer, Localization::text(choices[i]), 240, y, 1, i == amuletChoiceSelection_ ? .95f : .75f, 1);
    }
    drawVitaButtonHint(renderer,*font_,*uiButtonsSheet_,{{VitaButtonIcon::DPad," choose"},{VitaButtonIcon::Cross," accept"},{VitaButtonIcon::Circle," continue"}},240,207,.58f,.58f);
}

void GameScene::handleNpcDialogue(const VitaInput& input) {
    if (!input.pressed(SCE_CTRL_CROSS) && !input.pressed(SCE_CTRL_CIRCLE)) return;
    npcDialogueOpen_ = false;
    npcDialogueTitle_.clear();
    npcDialogueText_.clear();
    AudioSystem::instance().playSound("snd_click.mp3");
}

void GameScene::renderNpcDialogue(Renderer2D& renderer) {
    renderer.rect(0, 0, Renderer2D::LogicalWidth, Renderer2D::LogicalHeight, 0x000000A8);
    renderer.rect(58, 66, 364, 142, 0x171922FA);
    renderer.outlineRect(58, 66, 364, 142, 2, 0xC9B878FF);
    const std::string title = npcDialogueTitle_.empty() ? Localization::text("QUEST") : Localization::text(npcDialogueTitle_);
    font_->drawCentered(renderer, title, 240, 82, 1, 1.10f, 1);
    drawInventoryWrapped(*font_, renderer, npcDialogueText_, 82, 108, 44, 0.84f, 7);
    drawVitaButtonHint(renderer,*font_,*uiButtonsSheet_,{{VitaButtonIcon::Cross," /"},{VitaButtonIcon::Circle," continue"}},240,188,.62f,.62f);
}

void GameScene::renderGameMenu(Renderer2D& renderer) {
    renderer.rect(0, 0, Renderer2D::LogicalWidth, Renderer2D::LogicalHeight, 0x000000B8);
    renderer.rect(126, 32, 228, 204, 0x171922F2);
    renderer.outlineRect(126, 32, 228, 204, 2, 0xB6A56FFF);
    const Language language = AudioSystem::instance().language();
    const bool russian = language == Language::Russian;
    const bool portuguese = language == Language::Portuguese;
    const bool spanish = language == Language::Spanish;
    const char* settingsTitle = russian ? "НАСТРОЙКИ" : portuguese ? "CONFIGURAÇÕES" : spanish ? "AJUSTES" : "SETTINGS";
    const char* menuTitle = russian ? "МЕНЮ ИГРЫ" : portuguese ? "MENU DO JOGO" : spanish ? "MENÚ DEL JUEGO" : "GAME MENU";
    font_->drawCentered(renderer, settingsMenuOpen_ ? settingsTitle : menuTitle, 240, 46, 2.0f, 1.0f, 1.0f);

    static const char* zoomNames[4] = {"75% (more view)", "100%", "125%", "150%"};
    const AudioSystem& audio = AudioSystem::instance();
    const std::vector<std::string> values=settingsMenuOpen_?std::vector<std::string>{
        std::string(russian ? "Масштаб: " : portuguese ? "Zoom da câmera: " : spanish ? "Zoom de cámara: " : "Camera zoom: ")+zoomNames[zoomIndex_],
        std::string(russian ? "Музыка: " : portuguese ? "Música: " : spanish ? "Música: " : "Music: ")+(audio.musicEnabled()?"ON":"OFF"),
        std::string(russian ? "Звук: " : portuguese ? "Som: " : spanish ? "Sonido: " : "Sound: ")+(audio.soundEnabled()?"ON":"OFF"),
        std::string(russian ? "Версия музыки: " : portuguese ? "Versão da música: " : spanish ? "Versión musical: " : "Music version: ")+(audio.remixesEnabled()?"REMIX":"ORIGINAL"),
        std::string(russian ? "Язык: " : portuguese ? "Idioma: " : spanish ? "Idioma: " : "Language: ")+(russian?"РУССКИЙ":portuguese?"PORTUGUÊS":spanish?"ESPAÑOL":"ENGLISH"),
        russian ? "Назад" : portuguese ? "Voltar" : spanish ? "Volver" : "Back"
    }: (russian ? std::vector<std::string>{"Продолжить", "Настройки", "Читы", "Журнал", "Главное меню"} : portuguese ? std::vector<std::string>{"Continuar", "Configurações", "Trapaças", "Diário", "Menu principal"} : spanish ? std::vector<std::string>{"Continuar", "Ajustes", "Trucos", "Diario", "Menú principal"} : std::vector<std::string>{"Continue", "Settings", "Cheats", "Journal", "Main menu"});

    for (int i = 0; i < static_cast<int>(values.size()); ++i) {
        const float y = 75.0f + i * 25.0f;
        if (i == gameMenuSelection_) renderer.rect(144, y - 4, 192, 18, 0x6A5B35D8);
        font_->drawCentered(renderer, values[i], 240, y, 1.0f,
                            i == gameMenuSelection_ ? 1.25f : 0.75f, 1.0f);
    }
    font_->drawCentered(renderer, russian ? "D-Pad: выбор | X: выбрать | O: закрыть" : portuguese ? "D-Pad: escolher | X: selecionar | O: fechar" : spanish ? "D-Pad: elegir | X: seleccionar | O: cerrar" : "D-Pad choose/change | X select | O close",
                        240, 220, 1.0f, 0.55f, 1.0f);
    if (journalOpen_) {
        renderer.rect(70, 50, 340, 170, 0x101018FA);
        renderer.outlineRect(70, 50, 340, 170, 2, 0xC9B878FF);
        static const char* names[] = {"Well of Health", "Well of Awareness", "Well of Transmutation", "Sacrificial Fire", "Alchemy Pot", "Garden", "Animated Statue", "Sad Ghost", "Old Wandmaker", "Troll Blacksmith", "Ambitious Imp"};
        const std::vector<JournalRecord>& records=Progression::instance().journal();
        const int perPage=6;
        const int pages=std::max(1,(static_cast<int>(records.size())+perPage-1)/perPage);
        journalPage_=std::min(journalPage_,pages-1);
        char heading[48];snprintf(heading,sizeof(heading),"%s  %d/%d",Localization::text("QUEST JOURNAL").c_str(),journalPage_+1,pages);
        font_->drawCentered(renderer,heading,240,64,1,1.1f,1);
        if(records.empty())font_->drawCentered(renderer,Localization::text("No discoveries in this run"),240,124,1,.78f,1);
        const int first=journalPage_*perPage;
        const int last=std::min(first+perPage,static_cast<int>(records.size()));
        for(int i=first;i<last;++i){const JournalRecord& record=records[static_cast<std::size_t>(i)];const int feature=static_cast<int>(record.feature);char line[96];const char* featureName=(feature>=0&&feature<11)?names[feature]:"Unknown discovery";snprintf(line,sizeof(line),"D%02d  %s",record.depth,Localization::text(featureName).c_str());font_->draw(renderer,line,96,89+(i-first)*18,1,.74f,1);}
        font_->drawCentered(renderer,Localization::text("LEFT / RIGHT - page    X / O - back"),240,198,1,.6f,1);
    }
}

} // namespace pdv
