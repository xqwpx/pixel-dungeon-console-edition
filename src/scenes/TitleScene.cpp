#include "scenes/TitleScene.h"
#include "core/StringUtils.h"

#include "assets/AssetCatalog.h"
#include "assets/TextureCache.h"
#include "audio/AudioSystem.h"
#include "core/SceneManager.h"
#include "core/Localization.h"
#include "game/Progression.h"
#include "engine/input/IInputBackend.h"
#include "render/BitmapFont.h"
#include "render/Renderer2D.h"
#include "ui/VitaButtonHints.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
namespace pdv {
using VitaInput = IInputBackend;

namespace {

constexpr float TitleWidth = 128.0f;
constexpr float TitleHeight = 70.0f;

constexpr float SignsWidth = 128.0f;
constexpr float SignsHeight = 57.0f;

constexpr float ScrollSpeed = 20.0f;

constexpr int MenuItemCount = OnlineServiceEnabled ? 6 : 5;

constexpr const char* Labels[4] = {
    "Play", "Rankings", "Badges", "About"
};

constexpr int DashboardIconIndices[6] = {
    0, 2, 3, 1, 0, 2
};

} // namespace

bool TitleScene::initialize(TextureCache& textures, BitmapFont& font) {
    arcsBg_ = textures.get(Assets::ArcsBackground);
    arcsFg_ = textures.get(Assets::ArcsForeground);
    banners_ = textures.get(Assets::Banners);
    dashboard_ = textures.get(Assets::Dashboard);
    aboutQr_ = textures.get(Assets::AboutQr);
    badges_ = textures.get(Assets::Badges);
    uiButtons_ = textures.get(Assets::UIButtonsPSVita);
    font_ = &font;

    if (arcsBg_ == nullptr ||
        arcsFg_ == nullptr ||
        banners_ == nullptr ||
        dashboard_ == nullptr || badges_ == nullptr ||
        aboutQr_ == nullptr ||
        uiButtons_ == nullptr) {
        return false;
    }

    title_ = Sprite(
        banners_,
        TextureRegion(0.0f, 0.0f, TitleWidth, TitleHeight)
    );

    titleSigns_ = Sprite(
        banners_,
        TextureRegion(0.0f, 161.0f, SignsWidth, SignsHeight)
    );

    const float titleX =
        (Renderer2D::LogicalWidth - TitleWidth) * 0.5f;
    const float titleY = 38.0f;

    title_.setPosition(titleX, titleY);
    titleSigns_.setPosition(titleX, titleY);

    // Keep the dashboard as one horizontal strip: navigation is linear and
    // the 480px virtual canvas has room for all six 48px icons.
    const float buttonCount = static_cast<float>(MenuItemCount);
    const float firstX = (Renderer2D::LogicalWidth - DashboardButton::Size * buttonCount) * 0.5f;

    for (int i = 0; i < MenuItemCount; ++i) {
        buttons_[static_cast<std::size_t>(i)].initialize(
            dashboard_,
            i < 4 ? Labels[i] : "",
            DashboardIconIndices[i]
        );

        buttons_[static_cast<std::size_t>(i)].setPosition(
            firstX + DashboardButton::Size * static_cast<float>(i),
            145.0f
        );
    }

    updateButtonLabels();

    return true;
}

void TitleScene::shutdown() {
    arcsBg_ = nullptr;
    arcsFg_ = nullptr;
    banners_ = nullptr;
    dashboard_ = nullptr;
    aboutQr_ = nullptr;
    badges_ = nullptr;
    uiButtons_ = nullptr;
    font_ = nullptr;
}

void TitleScene::onEnter() {
    onlineRequest_.cancel();
    AudioSystem::instance().playMusic("theme.mp3");
    selected_ = 0;
    pressed_ = -1;
    analogLatched_ = false;

    statusText_.clear();
    statusTimer_ = 0.0f;
    panel_ = 0;
    badgePage_ = 0;
    rankingPage_ = 0;
    settingsSelection_ = 0;
    updateButtonLabels();
}

void TitleScene::onExit() {
    onlineRequest_.cancel();
    pressed_ = -1;
}

void TitleScene::update(
    float dt,
    const VitaInput& input,
    SceneManager& scenes
) {
    arcsBgOffset_ += dt * ScrollSpeed;
    arcsFgOffset_ += dt * ScrollSpeed * 2.0f;
    signsTime_ += dt;

    if (statusTimer_ > 0.0f) {
        statusTimer_ -= dt;
    }

    pressed_ = -1;

    if (panel_ != 0) {
        if (panel_ == 5 && onlineLoading_ && onlineRequest_.poll(onlineResults_)) {
            onlineLoading_ = false;
        }
        if (input.pressed(SCE_CTRL_CIRCLE) || (panel_ != 4 && input.pressed(SCE_CTRL_CROSS))) panel_ = 0;
        else if (panel_ == 1) {
            constexpr int perPage=5;
            constexpr int pages=(10+perPage-1)/perPage;
            if(input.pressed(SCE_CTRL_LEFT)||input.pressed(SCE_CTRL_UP))rankingPage_=(rankingPage_+pages-1)%pages;
            if(input.pressed(SCE_CTRL_RIGHT)||input.pressed(SCE_CTRL_DOWN))rankingPage_=(rankingPage_+1)%pages;
        } else if (panel_ == 2) {
            const int pages = (static_cast<int>(Badge::Count) + 6) / 7;
            if (input.pressed(SCE_CTRL_LEFT) || input.pressed(SCE_CTRL_UP)) badgePage_ = (badgePage_ + pages - 1) % pages;
            if (input.pressed(SCE_CTRL_RIGHT) || input.pressed(SCE_CTRL_DOWN)) badgePage_ = (badgePage_ + 1) % pages;
        } else if (panel_ == 4) {
            constexpr int Count = 6;
            if (input.pressed(SCE_CTRL_UP)) settingsSelection_ = (settingsSelection_ + Count - 1) % Count;
            if (input.pressed(SCE_CTRL_DOWN)) settingsSelection_ = (settingsSelection_ + 1) % Count;
            if (input.pressed(SCE_CTRL_LEFT) || input.pressed(SCE_CTRL_RIGHT) || input.pressed(SCE_CTRL_CROSS)) {
                AudioSystem& audio = AudioSystem::instance();
                if (settingsSelection_ == 0) audio.setCameraZoom(audio.cameraZoom() + (input.pressed(SCE_CTRL_LEFT) ? -1 : 1));
                else if (settingsSelection_ == 1) audio.setMusicEnabled(!audio.musicEnabled());
                else if (settingsSelection_ == 2) audio.setSoundEnabled(!audio.soundEnabled());
                else if (settingsSelection_ == 3) audio.setRemixesEnabled(!audio.remixesEnabled());
                else if (settingsSelection_ == 4) { audio.setLanguage(static_cast<Language>((static_cast<int>(audio.language()) + 1) % 4)); updateButtonLabels(); }
                else { panel_ = 0; }
            }
        }
        return;
    }

    handleButtons(input, scenes);
    handleAnalog(input);
    handleTouch(input, scenes);
}

void TitleScene::handleButtons(
    const VitaInput& input,
    SceneManager& scenes
) {
    if (input.pressed(SCE_CTRL_LEFT)) {
        moveSelection(-1);
    }

    if (input.pressed(SCE_CTRL_RIGHT)) {
        moveSelection(+1);
    }

    if (input.pressed(SCE_CTRL_CROSS)) {
        pressed_ = selected_;
        activate(static_cast<Item>(selected_), scenes);
    }
}

void TitleScene::handleAnalog(const VitaInput& input) {
    const float x = input.leftX();

    if (!analogLatched_) {
        if (x <= -0.65f) {
            moveSelection(-1);
            analogLatched_ = true;
        } else if (x >= 0.65f) {
            moveSelection(+1);
            analogLatched_ = true;
        }
    } else if (std::fabs(x) < 0.30f) {
        analogLatched_ = false;
    }
}

void TitleScene::handleTouch(
    const VitaInput& input,
    SceneManager& scenes
) {
    for (const auto& touch : input.touches()) {
        if (!touch.active || !touch.pressed) {
            continue;
        }

        const float logicalX = touch.x * 0.5f;
        const float logicalY = touch.y * 0.5f;

        const int index = itemIndexAt(logicalX, logicalY);
        if (index >= 0) {
            selected_ = index;
            pressed_ = index;
            activate(static_cast<Item>(index), scenes);
        }
    }
}

int TitleScene::itemIndexAt(float x, float y) const {
    for (int i = 0; i < MenuItemCount; ++i) {
        if (buttons_[static_cast<std::size_t>(i)].contains(x, y)) {
            return i;
        }
    }

    return -1;
}

void TitleScene::moveSelection(int delta) {
    selected_ += delta;

    while (selected_ < 0) {
        selected_ += MenuItemCount;
    }

    while (selected_ >= MenuItemCount) {
        selected_ -= MenuItemCount;
    }
}

void TitleScene::activate(
    Item item,
    SceneManager& scenes
) {
    switch (item) {
        case Item::Play:
            scenes.request(SceneId::Start);
            return;

        case Item::Rankings:
            panel_ = 1;
            rankingPage_ = 0;
            break;

        case Item::Badges:
            panel_ = 2;
            badgePage_ = 0;
            break;

        case Item::About:
            panel_ = 3;
            break;

        case Item::Settings:
            panel_ = 4;
            settingsSelection_ = 0;
            break;

        case Item::OnlineLeaderboard:
            if (!OnlineServiceEnabled) return;
            panel_ = 5;
            onlineLoading_ = true;
            onlineResults_ = {};
            onlineRequest_.cancel();
            onlineRequest_.startFetch();
            break;
    }

    statusTimer_ = 0.0f;
}

void TitleScene::render(Renderer2D& renderer) {
    if (font_ == nullptr) {
        return;
    }

    renderer.tiled(
        *arcsBg_,
        0.0f,
        0.0f,
        static_cast<float>(Renderer2D::LogicalWidth),
        static_cast<float>(Renderer2D::LogicalHeight),
        0.0f,
        arcsBgOffset_,
        1.0f
    );

    renderer.tiled(
        *arcsFg_,
        0.0f,
        0.0f,
        static_cast<float>(Renderer2D::LogicalWidth),
        static_cast<float>(Renderer2D::LogicalHeight),
        0.0f,
        arcsFgOffset_,
        1.0f
    );

    title_.draw(renderer);

    const float pulse = 0.55f + 0.45f * std::sin(signsTime_);
    titleSigns_.setAlpha(pulse);

    renderer.setAdditiveBlend(true);
    titleSigns_.draw(renderer);
    renderer.setAdditiveBlend(false);

    for (int i = 0; i < MenuItemCount; ++i) {
        buttons_[static_cast<std::size_t>(i)].draw(
            renderer,
            *font_,
            i == selected_,
            i == pressed_
        );
    }

    font_->draw(
        renderer,
        "v0.5.0 Beta",
        430.0f,
        263.0f,
        1.0f,
        0.55f,
        1.0f
    );

    if (statusTimer_ > 0.0f && !statusText_.empty()) {
        renderer.rect(
            110.0f,
            244.0f,
            260.0f,
            18.0f,
            0x000000B0
        );

        font_->drawCentered(
            renderer,
            statusText_,
            240.0f,
            249.0f,
            1.0f,
            1.0f,
            1.0f
        );
    }

    if (panel_ != 0) {
        renderer.rect(54, 40, 372, 190, 0x101018F4);
        renderer.outlineRect(54, 40, 372, 190, 2, 0xC9B878FF);
        const Progression& progress = Progression::instance();
        const Language language = AudioSystem::instance().language();
        const bool russian = language == Language::Russian;
        const bool portuguese = language == Language::Portuguese;
        const bool spanish = language == Language::Spanish;
        if (panel_ == 1) {
            constexpr int perPage = 5;
            constexpr int pages = 2;
            char heading[48];
            snprintf(heading, sizeof(heading), russian ? "РЕЙТИНГ  %d/%d" : portuguese ? "RANKING  %d/%d" : spanish ? "CLASIFICACIÓN  %d/%d" : "RANKINGS  %d/%d", rankingPage_ + 1, pages);
            font_->drawCentered(renderer, heading, 240, 53, 1, 1.25f, 1);
            char summary[96];
            snprintf(summary, sizeof(summary), russian ? "Игр %d   Побед %d   Макс. этаж %d" : portuguese ? "Partidas %d   Vitórias %d   Máx. andar %d" : spanish ? "Partidas %d   Victorias %d   Máx. piso %d" : "Runs %d   Wins %d   Deepest %d", progress.runs(), progress.wins(), progress.deepest());
            font_->drawCentered(renderer, summary, 240, 74, 1, .75f, 1);
            const int first=rankingPage_*perPage;
            const int last=std::min(first+perPage,10);
            static const char* heroClasses[] = {"Warrior", "Mage", "Rogue", "Huntress"};
            int row = 0;
            for (int index=first;index<last;++index) {
                const RankingEntry& entry=progress.rankings()[static_cast<std::size_t>(index)];
                if (entry.score <= 0) continue;
                char line[128];
                const std::string heroClass = Localization::text(entry.heroClass >= 0 && entry.heroClass < 4 ? heroClasses[entry.heroClass] : "Unknown");
                snprintf(line, sizeof(line), russian ? "%d. %s  очки %d  Э%d  У%d  %.0fс%s" : portuguese ? "%d. %s  pontos %d  A%d  M%d  %.0fs%s" : spanish ? "%d. %s  puntos %d  P%d  M%d  %.0fs%s" : "%d. %s  score %d  D%d  K%d  %.0fs%s", index + 1, heroClass.c_str(), entry.score, entry.depth, entry.kills, entry.duration, entry.won ? (russian ? "  ПОБЕДА" : portuguese ? "  VITÓRIA" : spanish ? "  VICTORIA" : "  WIN") : "");
                font_->draw(renderer, line, 82, 94 + row * 16, 1, .68f, 1);
                ++row;
            }
            if (row == 0) font_->drawCentered(renderer, russian ? "Пока нет завершённых игр" : portuguese ? "Ainda não há partidas concluídas" : spanish ? "Aún no hay partidas terminadas" : "No completed runs yet", 240, 125, 1, .8f, 1);
        } else if (panel_ == 2) {
            const int perPage = 7;
            const int pages = (static_cast<int>(Badge::Count) + perPage - 1) / perPage;
            char heading[48];
            snprintf(heading, sizeof(heading), russian ? "НАГРАДЫ  %d/%d" : portuguese ? "MEDALHAS  %d/%d" : spanish ? "MEDALLAS  %d/%d" : "BADGES  %d/%d", badgePage_ + 1, pages);
            font_->drawCentered(renderer, heading, 240, 53, 1, 1.25f, 1);
            const int first = badgePage_ * perPage;
            const int last = std::min(first + perPage, static_cast<int>(Badge::Count));
            for (int i = first; i < last; ++i) {
                const Badge badge = static_cast<Badge>(i);
                const float y = 74.0f + (i - first) * 17.0f;
                const int columns = std::max(1, badges_->width() / 16);
                const int icon = i % std::max(1, columns * (badges_->height() / 16));
                const bool unlocked = progress.hasBadge(badge);
                renderer.texture(*badges_, 62.0f, y, 16.0f, 16.0f,
                    TextureRegion(static_cast<float>((icon % columns) * 16), static_cast<float>((icon / columns) * 16), 16.0f, 16.0f),
                    1.0f, 1.0f, false, unlocked ? 0xFFFFFFFF : 0x707078FF);
                font_->draw(renderer, progress.badgeName(badge), 84.0f, y + 3.0f, 1, unlocked ? .78f : .48f, 1);
            }
        } else if (panel_ == 4) {
            const AudioSystem& audio = AudioSystem::instance();
            static const char* zoomNames[4] = {"75%", "100%", "125%", "150%"};
            const std::vector<std::string> values = {
                std::string(russian ? "Масштаб: " : portuguese ? "Zoom da câmera: " : spanish ? "Zoom de cámara: " : "Camera zoom: ") + zoomNames[audio.cameraZoom()],
                std::string(russian ? "Музыка: " : portuguese ? "Música: " : spanish ? "Música: " : "Music: ") + (audio.musicEnabled() ? "ON" : "OFF"),
                std::string(russian ? "Звук: " : portuguese ? "Som: " : spanish ? "Sonido: " : "Sound: ") + (audio.soundEnabled() ? "ON" : "OFF"),
                std::string(russian ? "Версия музыки: " : portuguese ? "Versão da música: " : spanish ? "Versión musical: " : "Music version: ") + (audio.remixesEnabled() ? "REMIX" : "ORIGINAL"),
                std::string(russian ? "Язык: " : portuguese ? "Idioma: " : spanish ? "Idioma: " : "Language: ") + (russian ? "РУССКИЙ" : portuguese ? "PORTUGUÊS" : spanish ? "ESPAÑOL" : "ENGLISH"),
                russian ? "Назад" : portuguese ? "Voltar" : spanish ? "Volver" : "Back"
            };
            font_->drawCentered(renderer, russian ? "НАСТРОЙКИ" : portuguese ? "CONFIGURAÇÕES" : spanish ? "AJUSTES" : "SETTINGS", 240, 53, 1, 1.25f, 1);
            for (int i = 0; i < static_cast<int>(values.size()); ++i) {
                const float y = 75.0f + i * 21.0f;
                if (i == settingsSelection_) renderer.rect(82, y - 3, 316, 16, 0x6A5B35D8);
                font_->drawCentered(renderer, values[i], 240, y, 1, i == settingsSelection_ ? .88f : .68f, 1);
            }
        } else if (panel_ == 5) {
            font_->drawCentered(renderer, "ONLINE LEADERBOARD", 240, 58, 1, 1.05f, 1);
            if (onlineLoading_) {
                font_->drawCentered(renderer, Localization::text("Loading records..."), 240, 122, 1, .8f, 1);
            } else if (!onlineResults_.ok) {
                font_->drawCentered(renderer, Localization::text(onlineResults_.message.empty() ? "Online service unavailable" : onlineResults_.message), 240, 112, 1, .68f, 1);
                font_->drawCentered(renderer, Localization::text("Check your network connection"), 240, 136, 1, .60f, 1);
            } else if (onlineResults_.entries.empty()) {
                font_->drawCentered(renderer, Localization::text("No records yet"), 240, 122, 1, .8f, 1);
            } else {
                constexpr float PlayerX = 72.0f;
                constexpr float HeroX = 125.0f;
                constexpr float ScoreRight = 190.0f;
                constexpr float FloorRight = 225.0f;
                constexpr float KillsRight = 280.0f;
                constexpr float GoldRight = 330.0f;
                constexpr float TimeRight = 370.0f;
                constexpr float HeaderY = 79.0f;
                constexpr float RowScale = 0.58f;

                const auto drawRight = [&](const std::string& value, float right, float y, float scale, float brightness) {
                    font_->draw(renderer, value, right - font_->measure(value, scale), y, scale, brightness, 1);
                };
                const auto truncate = [&](std::string value, float width) {
                    while (!value.empty() && font_->measure(value, RowScale) > width) value.pop_back();
                    return value;
                };

                font_->draw(renderer, Localization::text("PLAYER"), PlayerX, HeaderY, 1, .47f, 1);
                font_->draw(renderer, Localization::text("HERO"), HeroX, HeaderY, 1, .47f, 1);
                drawRight(Localization::text("SCORE"), ScoreRight, HeaderY, 1.0f, .47f);
                drawRight(Localization::text("FLOOR"), FloorRight, HeaderY, 1.0f, .47f);
                drawRight(Localization::text("KILLS"), KillsRight, HeaderY, 1.0f, .47f);
                drawRight(Localization::text("GOLD"), GoldRight, HeaderY, 1.0f, .47f);
                drawRight(Localization::text("TIME"), TimeRight, HeaderY, 1.0f, .47f);

                const int rows = std::min(6, static_cast<int>(onlineResults_.entries.size()));
                for (int i = 0; i < rows; ++i) {
                    const OnlineLeaderboardEntry& entry = onlineResults_.entries[static_cast<std::size_t>(i)];
                    const float y = 96.0f + i * 16.0f;
                    const std::string rank = numberToString(i + 1) + ".";
                    const std::string player = truncate(entry.name, HeroX - PlayerX - font_->measure(rank + " ", RowScale));
                    const std::string score = numberToString(entry.score);
                    const std::string hero = truncate(entry.hero, ScoreRight - HeroX - font_->measure(score, RowScale) - 3.0f);
                    char time[12];
                    snprintf(time, sizeof(time), "%02d:%02d", entry.duration / 60, entry.duration % 60);

                    font_->draw(renderer, rank, PlayerX, y, RowScale, .58f, 1);
                    font_->draw(renderer, player, PlayerX + font_->measure(rank + " ", RowScale), y, RowScale, .58f, 1);
                    font_->draw(renderer, hero, HeroX, y, RowScale, .58f, 1);
                    drawRight(score, ScoreRight, y, RowScale, .58f);
                    drawRight(numberToString(entry.depth), FloorRight, y, RowScale, .58f);
                    drawRight(numberToString(entry.kills), KillsRight, y, RowScale, .58f);
                    drawRight(numberToString(entry.gold), GoldRight, y, RowScale, .58f);
                    drawRight(time, TimeRight, y, RowScale, .58f);
                }
            }
        } else {
            font_->drawCentered(renderer, "PIXEL DUNGEON CONSOLE EDITION", 240, 66, 1, 1.15f, 1);
            if (russian) {
                font_->drawCentered(renderer, "Движок для консолей создан с нуля", 240, 89, 1, .62f, 1);
                font_->drawCentered(renderer, "За основу взят исходный код Pixel Dungeon", 240, 104, 1, .58f, 1);
                font_->drawCentered(renderer, "Лицензия: GNU General Public License v3.0", 240, 119, 1, .54f, 1);
                font_->drawCentered(renderer, "Порт создал HOMEBOX by xqspx", 240, 134, 1, .62f, 1);
            } else if (portuguese) {
                font_->drawCentered(renderer, "Motor para consoles criado do zero", 240, 89, 1, .62f, 1);
                font_->drawCentered(renderer, "Baseado no código-fonte de Pixel Dungeon", 240, 104, 1, .57f, 1);
                font_->drawCentered(renderer, "Licença: GNU General Public License v3.0", 240, 119, 1, .54f, 1);
                font_->drawCentered(renderer, "Port criado por HOMEBOX by xqspx", 240, 134, 1, .60f, 1);
            } else if (spanish) {
                font_->drawCentered(renderer, "Motor para consolas creado desde cero", 240, 89, 1, .61f, 1);
                font_->drawCentered(renderer, "Basado en el código fuente de Pixel Dungeon", 240, 104, 1, .56f, 1);
                font_->drawCentered(renderer, "Licencia: GNU General Public License v3.0", 240, 119, 1, .54f, 1);
                font_->drawCentered(renderer, "Port creado por HOMEBOX by xqspx", 240, 134, 1, .60f, 1);
            } else {
                font_->drawCentered(renderer, "Console engine created from scratch", 240, 89, 1, .62f, 1);
                font_->drawCentered(renderer, "Based on Pixel Dungeon source code", 240, 104, 1, .62f, 1);
                font_->drawCentered(renderer, "Licensed under GNU General Public License v3.0", 240, 119, 1, .52f, 1);
                font_->drawCentered(renderer, "Port by HOMEBOX by xqspx", 240, 134, 1, .62f, 1);
            }
            renderer.fullTexture(*aboutQr_, 215, 145, 50, 50);
        }
        if (uiButtons_ != nullptr) {
            if (panel_ == 1 || panel_ == 2) {
                drawVitaButtonHint(renderer, *font_, *uiButtons_, {
                    {VitaButtonIcon::DPad, russian ? " страница" : portuguese ? " página" : spanish ? " página" : " page"},
                    {VitaButtonIcon::Cross, " /"},
                    {VitaButtonIcon::Circle, russian ? " закрыть" : portuguese ? " fechar" : spanish ? " cerrar" : " close"}
                }, 240, 207, .62f, .62f);
            } else {
                drawVitaButtonHint(renderer, *font_, *uiButtons_, {
                    {VitaButtonIcon::Cross, " /"},
                    {VitaButtonIcon::Circle, russian ? " назад" : portuguese ? " voltar" : spanish ? " volver" : " close"}
                }, 240, 207, .62f, .62f);
            }
        }
    }
}

void TitleScene::updateButtonLabels() {
    const Language language = AudioSystem::instance().language();
    static const char* english[] = {"Play", "Rankings", "Badges", "About", "Settings", "Online"};
    static const char* russianLabels[] = {"Играть", "Рейтинг", "Награды", "Об игре", "Настройки", "Онлайн"};
    static const char* portugueseLabels[] = {"Jogar", "Ranking", "Medalhas", "Sobre", "Opções", "Online"};
    static const char* spanishLabels[] = {"Jugar", "Ranking", "Medallas", "Acerca de", "Ajustes", "Online"};
    const char* const* labels = language == Language::Russian ? russianLabels : language == Language::Portuguese ? portugueseLabels : language == Language::Spanish ? spanishLabels : english;
    for (int i = 0; i < MenuItemCount; ++i) buttons_[static_cast<std::size_t>(i)].setLabel(labels[i]);
}

} // namespace pdv
