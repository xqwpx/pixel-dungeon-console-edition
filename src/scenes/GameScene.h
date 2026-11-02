#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <deque>
#include <vector>

#include "core/OnlineLeaderboard.h"
#include "core/OnlineLeaderboardRequest.h"
#include "game/Dungeon.h"
#include "game/ui/InventoryModel.h"
#include "ui/TextButton.h"

namespace pdv {

class BitmapFont;
class Renderer2D;
class SceneManager;
class Texture2D;
class TextureCache;
class IInputBackend;
class Scroll;
class Item;

class GameScene {
public:
    bool initialize(TextureCache& textures, BitmapFont& font);
    void shutdown();

    void onEnter(GameSession& session);
    void onExit();

    void update(
        float dt,
        const IInputBackend& input,
        SceneManager& scenes,
        GameSession& session
    );

    void render(Renderer2D& renderer);

private:
    struct VisualPosition {
        float x = 0.0f;
        float y = 0.0f;
        VisualPosition() = default;
        VisualPosition(float px, float py) : x(px), y(py) {}
    };
    struct CorpseVisual {
        int cell = -1;
        Texture2D* sheet = nullptr;
        int frameWidth = 16;
        int frameHeight = 16;
        float xOffset = 0.0f;
        int frame = 12;
        float fadeRemaining = 3.0f;
    };
    struct WaterRipple {
        int cell = -1;
        float remaining = 0.55f;
    };
    struct ThrownItemVisual {
        int image = 0;
        int from = -1;
        int to = -1;
        float remaining = 0.24f;
    };
    struct ImpactEffect {
        int cell = -1;
        float remaining = 0.45f;
        float duration = 0.45f;
        float radius = 1.0f;
        uint32_t color = 0xFFFFFFFF;
    };
    struct HeroSpeck {
        float x = 0.0f;
        float y = 0.0f;
        float velocityX = 0.0f;
        float velocityY = -1.25f;
        float remaining = 1.0f;
        float duration = 1.0f;
        float gravity = 0.0f;
        int frame = 0;
    };

    float TileSize = 24.0f;
    static constexpr float ViewportY = 27.0f;
    static constexpr float ViewportH = 217.0f;

    void handleButtons(const IInputBackend& input);
    void handleAnalog(const IInputBackend& input);
    void updateHeldMovement(float dt, const IInputBackend& input);
    void handleTouch(const IInputBackend& input, SceneManager& scenes);

    bool manualMove(int dx, int dy);
    bool descendFromExit();
    bool ascendFromEntrance();
    void resetLevelVisuals();
    void revealNewDoors();

    void updateAutoAction(float dt);
    void updateCamera();
    void updateAnimations(float dt);
    void updateVisualPositions(float dt);
    void updateCombatStatus(int heroHpBefore, std::size_t mobsBefore);
    bool captureMobDeathVisual(const Mob& mob);
    void spawnWaterRipple(int cell);
    void triggerItemPop(int cell);
    void showThrownItem(int image, int from, int to);
    void showImpactEffect(int cell, uint32_t color, float radius, float duration = 0.45f);
    void spawnHeroSpecks(int frame, int count);
    void spawnSpecks(int cell, int frame, int count);
    void updateHeroSpecks(float dt);
    void updateBadgeNotification(float dt);
    void renderBadgeNotification(Renderer2D& renderer) const;
    void captureLogMessage();

    void handleGameMenu(const IInputBackend& input, SceneManager& scenes);
    void renderGameMenu(Renderer2D& renderer);
    void handleQuestReward(const IInputBackend& input);
    void renderQuestReward(Renderer2D& renderer);
    void handleBlacksmithReforge(const IInputBackend& input);
    void handleAmuletChoice(const IInputBackend& input);
    void renderAmuletChoice(Renderer2D& renderer);
    void handleNpcDialogue(const IInputBackend& input);
    void renderNpcDialogue(Renderer2D& renderer);
    void handleSkillMenu(const IInputBackend& input);
    void renderSkillMenu(Renderer2D& renderer);
    void handleCheatMenu(const IInputBackend& input);
    void renderCheatMenu(Renderer2D& renderer);
    void handleDeathSummary(const IInputBackend& input, SceneManager& scenes);
    void renderDeathSummary(Renderer2D& renderer);

    void openInventory();
    void closeInventory();
    void beginScrollSelection(Scroll* scroll);
    void handleInventoryInput(const IInputBackend& input);
    void activateInventorySelection();
    void renderInventory(Renderer2D& renderer);

    int inventorySlotAt(float logicalX, float logicalY) const;

    int screenToCell(float logicalX, float logicalY) const;

    Texture2D* tiles_ = nullptr;
    std::array<Texture2D*,5> zoneTiles_{};
    std::array<Texture2D*,5> waterSheets_{};
    std::array<Texture2D*,20> zoneMobSheets_{};
    std::array<Texture2D*,4> zoneBossSheets_{};
    std::array<Texture2D*,6> npcSheets_{};
    Texture2D* chrome_ = nullptr;
    Texture2D* ratSheet_ = nullptr;
    Texture2D* gnollSheet_ = nullptr;
    Texture2D* crabSheet_ = nullptr;
    Texture2D* gooSheet_ = nullptr;
    Texture2D* swarmSheet_ = nullptr;
    Texture2D* skeletonSheet_ = nullptr;
    Texture2D* thiefSheet_ = nullptr;
    Texture2D* piranhaSheet_ = nullptr;
    Texture2D* buffsSheet_ = nullptr;
    Texture2D* itemsSheet_ = nullptr;
    Texture2D* effectsSheet_ = nullptr;
    Texture2D* specksSheet_ = nullptr;
    Texture2D* badgesSheet_ = nullptr;
    Texture2D* plantsSheet_ = nullptr;
    Texture2D* statusSheet_ = nullptr;
    Texture2D* uiGameSheet_ = nullptr;
    Texture2D* levelUpSheet_ = nullptr;
    Texture2D* vitaButtonsSheet_ = nullptr;
    Texture2D* bannersSheet_ = nullptr;
    Texture2D* uiButtonsSheet_ = nullptr;

    std::array<Texture2D*, 4> heroSheets_{};

    BitmapFont* font_ = nullptr;

    Dungeon dungeon_;

    int touchTarget_ = -1;

    bool analogLatched_ = false;
    float moveRepeatTimer_ = 0.0f;
    int heldMoveDx_ = 0;
    int heldMoveDy_ = 0;
    float autoWalkTimer_ = 0.0f;

    float heroAnimationTime_ = 0.0f;
    float waterAnimationTime_ = 0.0f;
    int heroIdleFrame_ = 0;
    bool heroFacingLeft_ = false;

    float ratAnimationTime_ = 0.0f;
    int ratIdleFrame_ = 0;

    float heroAttackTimer_ = 0.0f;
    float heroOperateTimer_ = 0.0f;
    float mobAttackTimer_ = 0.0f;
    float hitEffectTimer_ = 0.0f;
    int hitEffectCell_ = -1;
    float searchEffectTimer_ = 0.0f;
    int searchEffectCell_ = -1;
    unsigned seenMobAttackSerial_ = 0;
    int attackingMobCell_ = -1;
    float mobDeathTimer_ = 0.0f;
    int mobDeathCell_ = -1;
    Texture2D* mobDeathSheet_ = nullptr;
    int mobDeathFrameWidth_ = 16;
    int mobDeathFrameHeight_ = 16;
    float mobDeathXOffset_ = 0.0f;
    int mobDeathFirstFrame_ = 0;
    int mobDeathLastFrame_ = 0;
    int mobDeathSpeckFrame_ = -1;
    bool mobDeathSpecksShown_ = false;
    std::vector<CorpseVisual> corpses_;
    std::vector<WaterRipple> waterRipples_;
    std::vector<ThrownItemVisual> thrownItems_;
    std::vector<ImpactEffect> impactEffects_;
    std::vector<HeroSpeck> heroSpecks_;
    std::unordered_map<int, float> itemPopTimers_;
    std::unordered_map<int, float> doorRevealTimers_;
    std::unordered_map<int, bool> knownDoorCells_;

    float cameraX_ = 0.0f;
    float cameraY_ = 0.0f;
    VisualPosition heroVisual_{};
    std::unordered_map<int, VisualPosition> mobVisuals_;
    int displayedHeroHp_ = -1;
    int displayedHeroLevel_ = -1;
    int displayedGold_ = -1;
    Badge activeBadge_ = Badge::FirstBlood;
    float badgeNotificationRemaining_ = 0.0f;

    bool gameMenuOpen_ = false;
    bool settingsMenuOpen_ = false;
    bool skillMenuOpen_ = false;
    int skillMenuSelection_ = 0;
    bool cheatMenuOpen_ = false;
    int cheatMenuSelection_ = 0;
    int cheatTeleportDepth_ = 1;
    bool journalOpen_ = false;
    int journalPage_ = 0;
    bool questRewardOpen_ = false;
    int questRewardSelection_ = 0;
    bool blacksmithReforgeOpen_ = false;
    Item* blacksmithFirstItem_ = nullptr;
    bool amuletChoiceOpen_ = false;
    int amuletChoiceSelection_ = 0;
    bool npcDialogueOpen_ = false;
    std::string npcDialogueTitle_;
    std::string npcDialogueText_;
    float interlevelTimer_ = 0.0f;
    int interlevelDepth_ = 1;
    bool interlevelIsTeleport_ = false;
    float introTimer_ = 0.0f;
    float surfaceTimer_ = 0.0f;
    bool deathSummaryOpen_ = false;
    int deathSummarySelection_ = 0;
    bool deathOnlineLoading_ = false;
    OnlineLeaderboardRequest deathOnlineRequest_;
    std::string deathOnlineStatus_;
    int gameMenuSelection_ = 0;
    int zoomIndex_ = 3;

    std::string status_;
    float statusTimer_ = 0.0f;
    std::string lastLoggedStatus_;
    float lastLoggedTimer_ = 0.0f;
    std::deque<std::string> messageLog_;
    std::deque<uint32_t> messageLogColors_;

    InventoryModel inventory_;
    const Item* inventoryDescriptionItem_ = nullptr;
    int inventoryDescriptionScroll_ = 0;

    Scroll* pendingScroll_ = nullptr;
    Item* pendingTargetItem_ = nullptr;
    ItemAction pendingTargetAction_ = ItemAction::Use;
    bool pendingQuickSlot_ = false;
    int targetCursor_ = -1;
    int pendingFallCell_ = -1;
    bool fallConfirmed_ = false;
    bool pendingScrollWasKnown_ = true;
    bool pendingCancelConfirm_ = false;
};

} // namespace pdv
