#pragma once

namespace pdv {
namespace Assets {

static constexpr const char* ArcsBackground = "ux0:data/PixelDungeonVita/assets/arcs1.png";
static constexpr const char* ArcsForeground = "ux0:data/PixelDungeonVita/assets/arcs2.png";
static constexpr const char* Banners = "ux0:data/PixelDungeonVita/assets/banners.png";
static constexpr const char* Dashboard = "ux0:data/PixelDungeonVita/assets/dashboard.png";
static constexpr const char* AboutQr = "ux0:data/PixelDungeonVita/assets/qr.png";
static constexpr const char* UIButtonsPSVita = "ux0:data/PixelDungeonVita/assets/UIButtonsPSVita.png";
// The UI font is boot-critical and is bundled in the VPK. External data may
// contain other font variants, but cannot override or break the interface.
static constexpr const char* Font1x = "app0:font1x.png";
static constexpr const char* FontOTF = "app0:JeuDePixel-Regular.otf";
static constexpr const char* UIFontAtlas = "ux0:data/PixelDungeonVita/assets/font_ui.png";
static constexpr const char* UIFontMetrics = "ux0:data/PixelDungeonVita/assets/font_ui.xml";
static constexpr const char* Avatars = "ux0:data/PixelDungeonVita/assets/avatars.png";
static constexpr const char* HeroBackgrounds[] = {
    "ux0:data/PixelDungeonVita/assets/avatars/Warrior.png",
    "ux0:data/PixelDungeonVita/assets/avatars/Mage.png",
    "ux0:data/PixelDungeonVita/assets/avatars/Rogue.png",
    "ux0:data/PixelDungeonVita/assets/avatars/Huntress.png"
};
static constexpr const char* Chrome = "ux0:data/PixelDungeonVita/assets/chrome.png";
static constexpr const char* BuffsSmall = "ux0:data/PixelDungeonVita/assets/buffs.png";
static constexpr const char* Items = "ux0:data/PixelDungeonVita/assets/items.png";
static constexpr const char* Effects = "ux0:data/PixelDungeonVita/assets/effects.png";
static constexpr const char* Specks = "ux0:data/PixelDungeonVita/assets/specks.png";
static constexpr const char* Badges = "ux0:data/PixelDungeonVita/assets/badges.png";
static constexpr const char* Plants = "ux0:data/PixelDungeonVita/assets/plants.png";
static constexpr const char* Toolbar = "ux0:data/PixelDungeonVita/assets/toolbar.png";
static constexpr const char* VitaButtons = "ux0:data/PixelDungeonVita/assets/psvita.png";
static constexpr const char* StatusPSVita = "ux0:data/PixelDungeonVita/assets/status_psvita.png";
static constexpr const char* UIGame = "ux0:data/PixelDungeonVita/assets/UIGame.png";
static constexpr const char* LevelUp = "ux0:data/PixelDungeonVita/assets/levelup.png";

static constexpr const char* TilesSewers = "ux0:data/PixelDungeonVita/assets/tiles0.png";
static constexpr const char* TilesPrison = "ux0:data/PixelDungeonVita/assets/tiles1.png";
static constexpr const char* TilesCaves = "ux0:data/PixelDungeonVita/assets/tiles2.png";
static constexpr const char* TilesCity = "ux0:data/PixelDungeonVita/assets/tiles3.png";
static constexpr const char* TilesHalls = "ux0:data/PixelDungeonVita/assets/tiles4.png";
static constexpr const char* WaterSheets[] = {
    "ux0:data/PixelDungeonVita/assets/water0.png", "ux0:data/PixelDungeonVita/assets/water1.png",
    "ux0:data/PixelDungeonVita/assets/water2.png", "ux0:data/PixelDungeonVita/assets/water3.png",
    "ux0:data/PixelDungeonVita/assets/water4.png"};

static constexpr const char* Warrior = "ux0:data/PixelDungeonVita/assets/warrior.png";
static constexpr const char* Mage = "ux0:data/PixelDungeonVita/assets/mage.png";
static constexpr const char* Rogue = "ux0:data/PixelDungeonVita/assets/rogue.png";
static constexpr const char* Huntress = "ux0:data/PixelDungeonVita/assets/ranger.png";
static constexpr const char* Rat = "ux0:data/PixelDungeonVita/assets/rat.png";
static constexpr const char* Gnoll = "ux0:data/PixelDungeonVita/assets/gnoll.png";
static constexpr const char* Crab = "ux0:data/PixelDungeonVita/assets/crab.png";
static constexpr const char* Goo = "ux0:data/PixelDungeonVita/assets/goo.png";
static constexpr const char* Swarm = "ux0:data/PixelDungeonVita/assets/swarm.png";
static constexpr const char* Skeleton = "ux0:data/PixelDungeonVita/assets/skeleton.png";
static constexpr const char* Thief = "ux0:data/PixelDungeonVita/assets/thief.png";
static constexpr const char* Piranha = "ux0:data/PixelDungeonVita/assets/piranha.png";
// Keep this ordered exactly like ZoneMobKind. The last five are boss
// minions, which occur on floor 20 and are also shown in the test room.
static constexpr const char* ZoneMobSheets[] = {"ux0:data/PixelDungeonVita/assets/shaman.png","ux0:data/PixelDungeonVita/assets/bat.png","ux0:data/PixelDungeonVita/assets/brute.png","ux0:data/PixelDungeonVita/assets/spinner.png","ux0:data/PixelDungeonVita/assets/elemental.png","ux0:data/PixelDungeonVita/assets/monk.png","ux0:data/PixelDungeonVita/assets/golem.png","ux0:data/PixelDungeonVita/assets/warlock.png","ux0:data/PixelDungeonVita/assets/succubus.png","ux0:data/PixelDungeonVita/assets/eye.png","ux0:data/PixelDungeonVita/assets/scorpio.png","ux0:data/PixelDungeonVita/assets/thief.png","ux0:data/PixelDungeonVita/assets/brute.png","ux0:data/PixelDungeonVita/assets/monk.png","ux0:data/PixelDungeonVita/assets/scorpio.png","ux0:data/PixelDungeonVita/assets/undead.png","ux0:data/PixelDungeonVita/assets/rotting_fist.png","ux0:data/PixelDungeonVita/assets/burning_fist.png","ux0:data/PixelDungeonVita/assets/larva.png","ux0:data/PixelDungeonVita/assets/sheep.png"};
static constexpr const char* ZoneBossSheets[] = {"ux0:data/PixelDungeonVita/assets/tengu.png","ux0:data/PixelDungeonVita/assets/dm300.png","ux0:data/PixelDungeonVita/assets/king.png","ux0:data/PixelDungeonVita/assets/yog.png"};
static constexpr const char* NPCSheets[] = {"ux0:data/PixelDungeonVita/assets/ghost.png","ux0:data/PixelDungeonVita/assets/wandmaker.png","ux0:data/PixelDungeonVita/assets/blacksmith.png","ux0:data/PixelDungeonVita/assets/demon.png","ux0:data/PixelDungeonVita/assets/shopkeeper.png","ux0:data/PixelDungeonVita/assets/ratking.png"};

} // namespace Assets
} // namespace pdv
