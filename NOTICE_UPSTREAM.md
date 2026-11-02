# Upstream notice

PixelDungeonVita is an in-progress native PlayStation Vita port based on the
open-source Pixel Dungeon project.

Pixel Dungeon:
- Copyright (C) 2012-2015 Oleg Dolya
- License: GPLv3 or later as stated by the upstream source.
- Upstream project: watabou/pixel-dungeon

PD-classes / Noosa:
- watabou/PD-classes

vitaGL:
- Rinnegatamante/vitaGL

v0.6.0 adds native C++ ports of the sewer level-generation core:
Rect, Graph, Room, Patch, Painter, EntrancePainter, ExitPainter,
StandardPainter, TunnelPainter, RegularLevel and SewerLevel.

Content-dependent branches that require unported Item/Mob/Quest systems are
explicitly feature-gated rather than replaced with unrelated gameplay logic.

Included Pixel Dungeon image assets under `assets/` originate from the upstream
project and must be redistributed consistently with its applicable license.


v0.10.0 additionally ports the depth-1 Bestiary/Rat AI layer and includes the upstream `rat.png` asset.


v0.10.0 ports the base Char combat pipeline and synchronous native combat resolution while retaining upstream formulas.

v0.10.0 adds native ports of XP, Belongings, Item/Bag/Heap, starter melee weapons and ClothArmor core.

v0.10.0 ports Hunger/Food, nested Bag/Keyring, Key types, pickup and basic key-unlock logic.

v0.15.0 adds native ports/foundations for Ballistica, MissileWeapon/Dart/Boomerang,
Wand/WandOfMagicMissile, Ring/RingOfShadows and QuickSlot behavior based on the
upstream GPLv3+ Pixel Dungeon source.

v0.15.0 adds native ports of the core Buff/FlavourBuff architecture and the first status-effect set.

v0.15.0 additionally ports Frost/Vertigo/Slow/Speed/Sleep/Light/MindVision/Amok/Shadows behavior and includes upstream buffs.png.

v0.15.0 adds the initial Blob, trap and Potion gameplay-source backend from upstream Pixel Dungeon.

v0.15.0 adds Generator/Gold/Scroll-bag foundations, exact RegularLevel loot/trap generation, Lightning/Summoning traps, and upstream items.png.

v0.16.0 additionally ports ItemStatusHandler-style Potion/Scroll identification, major Scroll actions, WndBag-compatible inventory layout logic, Rage, and MirrorImage behavior from upstream Pixel Dungeon.

v0.17.0 ports the original Weapon.Enchantment and Armor.Glyph object models,
weighted/equal random selection, supported combat procs, Item/EquipableItem actions,
equipment timing, drop/throw ownership rules, and WndItem-style context selection.
