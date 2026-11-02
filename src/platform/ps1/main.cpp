#include <cassert>
#include <cstddef>
#include <cstdint>

#include "platform/ps1/PS1Input.h"
#include "platform/ps1/PS1MemoryCard.h"
#include "core/GameSession.h"
#include "game/Dungeon.h"
#include "game/levels/Terrain.h"
#include "game/pathfinding/PathFinder.h"

extern "C" {
#include <psxgpu.h>
#include <stdlib.h>
}

namespace {

constexpr int ScreenWidth = 320;
constexpr int ScreenHeight = 240;
constexpr int OrderingTableLength = 8;
constexpr int PacketBufferSize = 16384;

extern "C" const std::uint32_t ps1_banners[];
extern "C" const std::uint32_t ps1_arcs1[];
extern "C" const std::uint32_t ps1_arcs2[];
extern "C" const std::uint32_t ps1_dashboard[];
extern "C" const std::uint32_t ps1_avatars[];

struct RenderBuffer {
    DISPENV display{};
    DRAWENV draw{};
    std::uint32_t orderingTable[OrderingTableLength]{};
    std::uint8_t packets[PacketBufferSize]{};
};

struct Renderer {
    RenderBuffer buffers[2]{};
    std::uint8_t* nextPacket = nullptr;
    int activeBuffer = 0;

    void initialize() {
        for (int index = 0; index < 2; ++index) {
            const int y = index * ScreenHeight;
            SetDefDrawEnv(&buffers[index].draw, 0, y, ScreenWidth, ScreenHeight);
            SetDefDispEnv(&buffers[index].display, 0, y, ScreenWidth, ScreenHeight);
            setRGB0(&buffers[index].draw, 8, 10, 16);
            buffers[index].draw.isbg = 1;
        }
        nextPacket = buffers[0].packets;
        ClearOTagR(buffers[0].orderingTable, OrderingTableLength);
        SetDispMask(1);
    }

    void drawPanel(int z, int x, int y, int width, int height, int r, int g, int b) {
        auto* tile = reinterpret_cast<TILE*>(nextPacket);
        nextPacket += sizeof(TILE);
        assert(nextPacket <= buffers[activeBuffer].packets + PacketBufferSize);
        setTile(tile);
        setXY0(tile, x, y);
        setWH(tile, width, height);
        setRGB0(tile, r, g, b);
        addPrim(&buffers[activeBuffer].orderingTable[z], tile);
    }

    void drawTexture(int z, int x, int y, const TIM_IMAGE& texture) {
        drawTextureRegion(z, x, y, texture, 0, 0, texture.prect->w, texture.prect->h);
    }

    void drawTextureRegion(int z, int x, int y, const TIM_IMAGE& texture,
        int u, int v, int width, int height) {
        auto* sprite = reinterpret_cast<SPRT*>(nextPacket);
        nextPacket += sizeof(SPRT);
        auto* page = reinterpret_cast<DR_TPAGE*>(nextPacket);
        nextPacket += sizeof(DR_TPAGE);
        assert(nextPacket <= buffers[activeBuffer].packets + PacketBufferSize);

        setSprt(sprite);
        setRGB0(sprite, 128, 128, 128);
        setXY0(sprite, x, y);
        setUV0(sprite, u, v);
        setWH(sprite, width, height);
        addPrim(&buffers[activeBuffer].orderingTable[z], sprite);

        setDrawTPage(page, 0, 0, getTPage(2, 0,
            texture.prect->x, texture.prect->y));
        addPrim(&buffers[activeBuffer].orderingTable[z], page);
    }

    void tileTexture(int z, const TIM_IMAGE& texture, int width, int height) {
        for (int y = 0; y < ScreenHeight; y += height) {
            for (int x = 0; x < ScreenWidth; x += width) {
                const int drawWidth = (x + width <= ScreenWidth) ? width : ScreenWidth - x;
                const int drawHeight = (y + height <= ScreenHeight) ? height : ScreenHeight - y;
                drawTextureRegion(z, x, y, texture, 0, 0, drawWidth, drawHeight);
            }
        }
    }

    void present() {
        DrawSync(0);
        VSync(0);
        RenderBuffer& drawing = buffers[activeBuffer];
        RenderBuffer& displaying = buffers[activeBuffer ^ 1];
        PutDispEnv(&displaying.display);
        DrawOTagEnv(&drawing.orderingTable[OrderingTableLength - 1], &drawing.draw);
        activeBuffer ^= 1;
        nextPacket = displaying.packets;
        ClearOTagR(displaying.orderingTable, OrderingTableLength);
    }
};

// PSn00bSDK's entry stack is intentionally small. Keep the frame renderer,
// 32x32 path map and full dungeon state in BSS rather than in main's frame.
// Starting a generated level then cannot overwrite the return/input state.
Renderer renderer;
std::array<bool, pdv::LevelLength> pathMap{};
alignas(4) std::uint8_t gameHeap[768 * 1024]{};

} // namespace

int main() {
    ResetGraph(0);
    // libpsn00b's allocator is not configured by the menu-only bootstrap.
    // The shared dungeon creates rooms, mobs and items through new, so give
    // it a fixed PS1 RAM heap before any gameplay object is constructed.
    InitHeap(gameHeap, sizeof(gameHeap));
    static pdv::Dungeon dungeon;
    renderer.initialize();
    pdv::PS1Input input;
    input.initialize();
    pdv::PS1MemoryCard::initialize();
    TIM_IMAGE banners{};
    GetTimInfo(ps1_banners, &banners);
    LoadImage(banners.prect, banners.paddr);
    TIM_IMAGE arcs1{};
    GetTimInfo(ps1_arcs1, &arcs1);
    LoadImage(arcs1.prect, arcs1.paddr);
    TIM_IMAGE arcs2{};
    GetTimInfo(ps1_arcs2, &arcs2);
    LoadImage(arcs2.prect, arcs2.paddr);
    TIM_IMAGE dashboard{};
    GetTimInfo(ps1_dashboard, &dashboard);
    LoadImage(dashboard.prect, dashboard.paddr);
    TIM_IMAGE avatars{};
    GetTimInfo(ps1_avatars, &avatars);
    LoadImage(avatars.prect, avatars.paddr);

    enum class Screen : unsigned char { Title, HeroSelect, Loading, Dungeon };
    Screen screen = Screen::Title;
    pdv::GameSession session;
    pathMap.fill(true);
    pdv::PathFinder::setMapSize(pdv::LevelWidth, pdv::LevelHeight);
    int selectedItem = 0;
    int selectedHero = 0;
    int confirmFrames = 0;
    bool generationPending = false;

    for (;;) {
        // Native PS1 title layout. The same source atlas regions used by the
        // shared TitleScene are rendered through the PS1 TIM path.
        renderer.drawPanel(4, 0, 0, ScreenWidth, ScreenHeight, 10, 15, 24);
        renderer.tileTexture(3, arcs1, 32, 32);
        renderer.tileTexture(2, arcs2, 64, 64);
        input.update();
        if (screen == Screen::Title) {
            if (input.pressed(pdv::PS1Button::Left)) selectedItem = (selectedItem + 3) % 4;
            if (input.pressed(pdv::PS1Button::Right)) selectedItem = (selectedItem + 1) % 4;
            if (input.pressed(pdv::PS1Button::Confirm)) {
                if (selectedItem == 0) screen = Screen::HeroSelect;
                confirmFrames = 18;
            }
        } else if (screen == Screen::HeroSelect) {
            if (input.pressed(pdv::PS1Button::Left) || input.pressed(pdv::PS1Button::Up)) {
                selectedHero = (selectedHero + 3) % 4;
            }
            if (input.pressed(pdv::PS1Button::Right) || input.pressed(pdv::PS1Button::Down)) {
                selectedHero = (selectedHero + 1) % 4;
            }
            if (input.pressed(pdv::PS1Button::Cancel)) screen = Screen::Title;
            if (input.pressed(pdv::PS1Button::Confirm)) {
                session.selectedHero = static_cast<pdv::HeroClass>(selectedHero);
                session.newGameRequested = true;
                session.continueRequested = false;
                ++session.runSeed;
                // Present a complete frame first. If generation ever stalls,
                // this screen proves the call was reached rather than hiding
                // the fault behind the hero-select menu.
                screen = Screen::Loading;
                generationPending = true;
            }
        } else if (screen == Screen::Dungeon) {
            int dx = 0;
            int dy = 0;
            if (input.pressed(pdv::PS1Button::Left)) dx = -1;
            if (input.pressed(pdv::PS1Button::Right)) dx = 1;
            if (input.pressed(pdv::PS1Button::Up)) dy = -1;
            if (input.pressed(pdv::PS1Button::Down)) dy = 1;
            if (dx != 0 || dy != 0) dungeon.moveHeroBy(dx, dy);
            if (input.pressed(pdv::PS1Button::Confirm)) dungeon.pickUpAtHero();
            if (input.pressed(pdv::PS1Button::Pause)) dungeon.save();
            if (input.pressed(pdv::PS1Button::Cancel)) {
                dungeon.save();
                screen = Screen::Title;
            }
        }
        if (confirmFrames > 0) --confirmFrames;

        // First shared-logic integration check: PS1 reads the same terrain
        // flags used by every other platform. Rendering remains platform-only.
        const bool terrainReady = (pdv::Terrain::flagsFor(pdv::Terrain::EMPTY) & pdv::Terrain::PASSABLE) != 0;
        const bool pathReady = pdv::PathFinder::getStep(0, pdv::LevelWidth + 1, pathMap) == 1;

        if (screen == Screen::Title) {
            renderer.drawTextureRegion(0, 96, 38, banners, 0, 0, 128, 70);
            renderer.drawTextureRegion(0, 96, 38, banners, 0, 161, 128, 57);
            const int firstButtonX = 96;
            for (int item = 0; item < 4; ++item) {
                const int x = firstButtonX + item * 34;
                const bool active = item == selectedItem;
                renderer.drawPanel(1, x - 2, 164, 36, 36,
                    active ? (confirmFrames ? 220 : 190) : 42,
                    active ? (confirmFrames ? 177 : 143) : 51,
                    active ? (terrainReady && pathReady ? 76 : 54) : 65);
                renderer.drawTextureRegion(0, x, 166, dashboard, item * 32, 0, 32, 32);
            }
        } else if (screen == Screen::HeroSelect) {
            renderer.drawTextureRegion(0, 96, 32, banners, 0, 140, 128, 21);
            for (int hero = 0; hero < 4; ++hero) {
                const int x = 48 + hero * 58;
                const bool active = hero == selectedHero;
                renderer.drawPanel(1, x - 4, 92, 40, 40,
                    active ? (confirmFrames ? 220 : 190) : 42,
                    active ? (confirmFrames ? 177 : 143) : 51,
                    active ? 76 : 65);
                renderer.drawTextureRegion(0, x + 8, 96, avatars, hero * 24, 0, 24, 32);
            }
        } else if (screen == Screen::Loading) {
            renderer.drawPanel(1, 48, 82, 224, 76, 22, 31, 53);
            renderer.drawPanel(0, 64, 100, 192, 12, 202, 155, 64);
            renderer.drawPanel(0, 64, 122, 48, 14, 66, 120, 190);
            renderer.drawPanel(0, 120, 122, 48, 14, 104, 175, 82);
            renderer.drawPanel(0, 176, 122, 48, 14, 182, 79, 71);
        } else {
            // Compact native viewport of the procedurally generated shared map.
            // D-pad moves/attacks, Cross picks up, Start saves, Circle returns.
            const int heroCell = dungeon.hero().pos;
            const int heroX = heroCell % pdv::Level::Width;
            const int heroY = heroCell / pdv::Level::Width;
            const auto& map = dungeon.level().map();
            const auto& visible = dungeon.visible();
            constexpr int Tile = 8;
            constexpr int Radius = 9;
            const int originX = (ScreenWidth - (Radius * 2 + 1) * Tile) / 2;
            const int originY = 44;
            for (int vy = -Radius; vy <= Radius; ++vy) {
                for (int vx = -Radius; vx <= Radius; ++vx) {
                    const int cellX = heroX + vx;
                    const int cellY = heroY + vy;
                    const int sx = originX + (vx + Radius) * Tile;
                    const int sy = originY + (vy + Radius) * Tile;
                    if (!dungeon.level().inBounds(cellX, cellY)) {
                        renderer.drawPanel(1, sx, sy, Tile, Tile, 2, 3, 6);
                        continue;
                    }
                    const int cell = cellY * pdv::Level::Width + cellX;
                    const bool seen = visible[static_cast<std::size_t>(cell)];
                    const int terrain = map[static_cast<std::size_t>(cell)];
                    const bool wall = (pdv::Terrain::flagsFor(terrain) & pdv::Terrain::SOLID) != 0;
                    renderer.drawPanel(1, sx, sy, Tile, Tile,
                        seen ? (wall ? 58 : 24) : 5,
                        seen ? (wall ? 65 : 31) : 7,
                        seen ? (wall ? 77 : 38) : 11);
                }
            }
            renderer.drawPanel(0, originX + Radius * Tile + 2, originY + Radius * Tile + 2,
                Tile - 4, Tile - 4, 235, 205, 92);
            renderer.drawPanel(0, 8, 10, 304, 22, 20, 27, 43);
            renderer.drawPanel(0, 12, 14, dungeon.hero().HP * 90 / (dungeon.hero().HT ? dungeon.hero().HT : 1), 5, 192, 54, 49);
            renderer.drawPanel(0, 12, 23, dungeon.depth() * 8, 3, 83, 149, 219);
        }
        renderer.present();
        if (generationPending) {
            generationPending = false;
            if (dungeon.startNew(session.selectedHero, session.runSeed)) {
                screen = Screen::Dungeon;
                confirmFrames = 18;
            } else {
                screen = Screen::HeroSelect;
            }
        }
    }
}
