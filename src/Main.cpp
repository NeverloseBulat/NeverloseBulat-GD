#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

static bool g_noclip = false;
static bool g_fly = false;
static bool g_autoJump = false;
static bool g_spinbot = false;
static bool g_spikeESP = false;
static bool g_ldm = false;
static bool g_autoLDM = false;
static bool g_speedhack = false;
static bool g_showHitbox = false;
static bool g_jumpHack = false;
static float g_speed = 1.0f;
static float g_spinSpeed = 5.0f;
static bool g_touchHeld = false;

// ================= МЕНЮ =================
class NeverloseMenu : public CCLayer {
protected:
    std::vector<CCNode*> m_mainNodes, m_rageNodes, m_visualsNodes, m_antiAimNodes;

    CCMenuItemSpriteExtra *m_jumpHackBtn = nullptr;
    CCMenuItemSpriteExtra *m_noclipBtn, *m_flyBtn, *m_spikeESPBtn, *m_autoJumpBtn;
    CCMenuItemSpriteExtra *m_ldmBtn, *m_autoLDMBtn, *m_hitboxBtn;
    CCMenuItemSpriteExtra *m_spinbotBtn, *m_spinDown, *m_spinUp, *m_speedhackBtn, *m_shDown, *m_shUp;
    CCLabelBMFont *m_spinLabel, *m_spinText, *m_shLabel, *m_shText;
    CCMenuItemSpriteExtra *m_tabMain, *m_tabRage, *m_tabVisuals, *m_tabAntiAim;

public:
    static NeverloseMenu* create() {
        auto r = new NeverloseMenu();
        if (r && r->init()) { r->autorelease(); return r; }
        delete r; return nullptr;
    }

    void refreshButtons() {
        m_jumpHackBtn->setNormalImage(ButtonSprite::create(g_jumpHack ? "Jump Hack: ON" : "Jump Hack: OFF"));
        m_noclipBtn->setNormalImage(ButtonSprite::create(g_noclip ? "Noclip: ON" : "Noclip: OFF"));
        m_flyBtn->setNormalImage(ButtonSprite::create(g_fly ? "Fly: ON" : "Fly: OFF"));
        m_spikeESPBtn->setNormalImage(ButtonSprite::create(g_spikeESP ? "ESP Spikes: ON" : "ESP Spikes: OFF"));
        m_autoJumpBtn->setNormalImage(ButtonSprite::create(g_autoJump ? "AutoJump: ON" : "AutoJump: OFF"));
        m_ldmBtn->setNormalImage(ButtonSprite::create(g_ldm ? "LDM: ON" : "LDM: OFF"));
        m_autoLDMBtn->setNormalImage(ButtonSprite::create(g_autoLDM ? "Auto LDM: ON" : "Auto LDM: OFF"));
        m_hitboxBtn->setNormalImage(ButtonSprite::create(g_showHitbox ? "Hitbox: ON" : "Hitbox: OFF"));
        m_spinbotBtn->setNormalImage(ButtonSprite::create(g_spinbot ? "Spinbot: ON" : "Spinbot: OFF"));
        m_speedhackBtn->setNormalImage(ButtonSprite::create(g_speedhack ? "Speedhack: ON" : "Speedhack: OFF"));
        m_shLabel->setString(CCString::createWithFormat("%.3f", g_speed)->getCString());
    }

    void setPage(int p) {
        for (auto n : m_mainNodes) n->setVisible(p == 0);
        for (auto n : m_rageNodes) n->setVisible(p == 1);
        for (auto n : m_visualsNodes) n->setVisible(p == 2);
        for (auto n : m_antiAimNodes) n->setVisible(p == 3);
        m_tabMain->setColor(p == 0 ? ccWHITE : ccGRAY);
        m_tabRage->setColor(p == 1 ? ccWHITE : ccGRAY);
        m_tabVisuals->setColor(p == 2 ? ccWHITE : ccGRAY);
        m_tabAntiAim->setColor(p == 3 ? ccWHITE : ccGRAY);
    }

    bool init() {
        if (!CCLayer::init()) return false;
        auto ws = CCDirector::get()->getWinSize();
        float cx = ws.width / 2, cy = ws.height / 2;

        auto overlay = CCLayerColor::create({0, 0, 0, 180});
        this->addChild(overlay, -1);

        auto panel = CCScale9Sprite::create("GJ_square01.png");
        panel->setContentSize({780, 520});
        panel->setPosition({cx, cy});
        panel->setColor({15, 15, 15});
        this->addChild(panel);

        auto sidebar = CCScale9Sprite::create("GJ_square01.png");
        sidebar->setContentSize({220, 500});
        sidebar->setPosition({cx - 270, cy});
        sidebar->setColor({25, 25, 30});
        this->addChild(sidebar);

        auto logo = CCLabelBMFont::create("NEVERLOSE", "goldFont.fnt");
        logo->setPosition({cx - 270, cy + 215});
        logo->setScale(0.65f);
        logo->setColor({255, 255, 255});
        this->addChild(logo);

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        this->addChild(menu);

        auto cs = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        cs->setScale(0.8f);
        auto closeBtn = CCMenuItemSpriteExtra::create(cs, this, menu_selector(NeverloseMenu::onClose));
        closeBtn->setPosition({cx + 350, cy});
        menu->addChild(closeBtn);

        float tabY[4] = {cy + 130, cy + 40, cy - 50, cy - 140};
        m_tabMain = CCMenuItemSpriteExtra::create(ButtonSprite::create("Main"), this, menu_selector(NeverloseMenu::onTabMain));
        m_tabMain->setPosition({cx - 270, tabY[0]});
        menu->addChild(m_tabMain);

        m_tabRage = CCMenuItemSpriteExtra::create(ButtonSprite::create("Rage"), this, menu_selector(NeverloseMenu::onTabRage));
        m_tabRage->setPosition({cx - 270, tabY[1]});
        menu->addChild(m_tabRage);

        m_tabVisuals = CCMenuItemSpriteExtra::create(ButtonSprite::create("Visuals"), this, menu_selector(NeverloseMenu::onTabVisuals));
        m_tabVisuals->setPosition({cx - 270, tabY[2]});
        menu->addChild(m_tabVisuals);

        m_tabAntiAim = CCMenuItemSpriteExtra::create(ButtonSprite::create("Anti-Aim"), this, menu_selector(NeverloseMenu::onTabAntiAim));
        m_tabAntiAim->setPosition({cx - 270, tabY[3]});
        menu->addChild(m_tabAntiAim);

        // MAIN
        m_jumpHackBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Jump Hack: OFF"), this, menu_selector(NeverloseMenu::onJumpHack));
        m_jumpHackBtn->setPosition({cx + 60, cy + 100});
        menu->addChild(m_jumpHackBtn); m_mainNodes.push_back(m_jumpHackBtn);

        // RAGE
        m_noclipBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Noclip: OFF"), this, menu_selector(NeverloseMenu::onNoclip));
        m_noclipBtn->setPosition({cx + 60, cy + 130});
        menu->addChild(m_noclipBtn); m_rageNodes.push_back(m_noclipBtn);

        m_flyBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Fly: OFF"), this, menu_selector(NeverloseMenu::onFly));
        m_flyBtn->setPosition({cx + 60, cy + 65});
        menu->addChild(m_flyBtn); m_rageNodes.push_back(m_flyBtn);

        m_spikeESPBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("ESP Spikes: OFF"), this, menu_selector(NeverloseMenu::onSpikeESP));
        m_spikeESPBtn->setPosition({cx + 60, cy});
        menu->addChild(m_spikeESPBtn); m_rageNodes.push_back(m_spikeESPBtn);

        m_autoJumpBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("AutoJump: OFF"), this, menu_selector(NeverloseMenu::onAutoJump));
        m_autoJumpBtn->setPosition({cx + 60, cy - 65});
        menu->addChild(m_autoJumpBtn); m_rageNodes.push_back(m_autoJumpBtn);

        // VISUALS
        m_ldmBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("LDM: OFF"), this, menu_selector(NeverloseMenu::onLDM));
        m_ldmBtn->setPosition({cx + 60, cy + 90});
        menu->addChild(m_ldmBtn); m_visualsNodes.push_back(m_ldmBtn);

        m_autoLDMBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Auto LDM: OFF"), this, menu_selector(NeverloseMenu::onAutoLDM));
        m_autoLDMBtn->setPosition({cx + 60, cy + 25});
        menu->addChild(m_autoLDMBtn); m_visualsNodes.push_back(m_autoLDMBtn);

        m_hitboxBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Hitbox: OFF"), this, menu_selector(NeverloseMenu::onHitbox));
        m_hitboxBtn->setPosition({cx + 60, cy - 40});
        menu->addChild(m_hitboxBtn); m_visualsNodes.push_back(m_hitboxBtn);

        // ANTI-AIM
        m_spinbotBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Spinbot: OFF"), this, menu_selector(NeverloseMenu::onSpinbot));
        m_spinbotBtn->setPosition({cx + 60, cy + 130});
        menu->addChild(m_spinbotBtn); m_antiAimNodes.push_back(m_spinbotBtn);

        m_spinText = CCLabelBMFont::create("Spinbot Speed", "bigFont.fnt");
        m_spinText->setPosition({cx - 40, cy + 60});
        m_spinText->setScale(0.5f);
        m_spinText->setColor({0, 200, 255});
        this->addChild(m_spinText); m_antiAimNodes.push_back(m_spinText);

        m_spinDown = CCMenuItemSpriteExtra::create(ButtonSprite::create("<"), this, menu_selector(NeverloseMenu::onSpinDown));
        m_spinDown->setPosition({cx + 70, cy + 60});
        menu->addChild(m_spinDown); m_antiAimNodes.push_back(m_spinDown);

        m_spinLabel = CCLabelBMFont::create("5.00", "bigFont.fnt");
        m_spinLabel->setPosition({cx + 135, cy + 60});
        m_spinLabel->setScale(0.7f);
        m_spinLabel->setColor({0, 200, 255});
        this->addChild(m_spinLabel); m_antiAimNodes.push_back(m_spinLabel);

        m_spinUp = CCMenuItemSpriteExtra::create(ButtonSprite::create(">"), this, menu_selector(NeverloseMenu::onSpinUp));
        m_spinUp->setPosition({cx + 200, cy + 60});
        menu->addChild(m_spinUp); m_antiAimNodes.push_back(m_spinUp);

        m_speedhackBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Speedhack: OFF"), this, menu_selector(NeverloseMenu::onSpeedhack));
        m_speedhackBtn->setPosition({cx + 60, cy - 30});
        menu->addChild(m_speedhackBtn); m_antiAimNodes.push_back(m_speedhackBtn);

        m_shText = CCLabelBMFont::create("Speedhack Value", "bigFont.fnt");
        m_shText->setPosition({cx - 30, cy - 100});
        m_shText->setScale(0.5f);
        m_shText->setColor({0, 200, 255});
        this->addChild(m_shText); m_antiAimNodes.push_back(m_shText);

        m_shDown = CCMenuItemSpriteExtra::create(ButtonSprite::create("<"), this, menu_selector(NeverloseMenu::onSpeedhackDown));
        m_shDown->setPosition({cx + 70, cy - 100});
        menu->addChild(m_shDown); m_antiAimNodes.push_back(m_shDown);

        m_shLabel = CCLabelBMFont::create("1.000", "bigFont.fnt");
        m_shLabel->setPosition({cx + 135, cy - 100});
        m_shLabel->setScale(0.7f);
        m_shLabel->setColor({0, 200, 255});
        this->addChild(m_shLabel); m_antiAimNodes.push_back(m_shLabel);

        m_shUp = CCMenuItemSpriteExtra::create(ButtonSprite::create(">"), this, menu_selector(NeverloseMenu::onSpeedhackUp));
        m_shUp->setPosition({cx + 200, cy - 100});
        menu->addChild(m_shUp); m_antiAimNodes.push_back(m_shUp);

        setPage(0);
        refreshButtons();
        this->setScale(0.3f);
        this->runAction(CCEaseBackOut::create(CCScaleTo::create(0.35f, 1.0f)));
        this->setKeypadEnabled(true);
        return true;
    }

    void onTabMain(CCObject*) { setPage(0); }
    void onTabRage(CCObject*) { setPage(1); }
    void onTabVisuals(CCObject*) { setPage(2); }
    void onTabAntiAim(CCObject*) { setPage(3); }

    void onJumpHack(CCObject*) {
        g_jumpHack = !g_jumpHack;
        refreshButtons();
    }
    void onNoclip(CCObject*) {
        g_noclip = !g_noclip;
        refreshButtons();
    }
    void onFly(CCObject*) {
        g_fly = !g_fly;
        refreshButtons();
    }
    void onSpikeESP(CCObject*) {
        g_spikeESP = !g_spikeESP;
        refreshButtons();
    }
    void onAutoJump(CCObject*) {
        g_autoJump = !g_autoJump;
        refreshButtons();
    }
    void onLDM(CCObject*) {
        g_ldm = !g_ldm;
        GameManager::get()->setGameVariable("low_detail_mode", g_ldm);
        refreshButtons();
    }
    void onAutoLDM(CCObject*) {
        g_autoLDM = !g_autoLDM;
        GameManager::get()->setGameVariable("low_detail_mode", g_autoLDM);
        refreshButtons();
    }
    void onHitbox(CCObject*) {
        g_showHitbox = !g_showHitbox;
        refreshButtons();
    }
    void onSpinbot(CCObject*) {
        g_spinbot = !g_spinbot;
        refreshButtons();
    }
    void onSpinUp(CCObject*) {
        g_spinSpeed += 1.0f;
        if (g_spinSpeed > 100.0f) g_spinSpeed = 100.0f;
        m_spinLabel->setString(CCString::createWithFormat("%.2f", g_spinSpeed)->getCString());
    }
    void onSpinDown(CCObject*) {
        g_spinSpeed -= 1.0f;
        if (g_spinSpeed < 1.0f) g_spinSpeed = 1.0f;
        m_spinLabel->setString(CCString::createWithFormat("%.2f", g_spinSpeed)->getCString());
    }
    void onSpeedhack(CCObject*) {
        g_speedhack = !g_speedhack;
        if (g_speedhack) {
            CCDirector::get()->getScheduler()->setTimeScale(g_speed);
        } else {
            CCDirector::get()->getScheduler()->setTimeScale(1.0f);
        }
        refreshButtons();
    }
    void onSpeedhackUp(CCObject*) {
        g_speed += 0.5f;
        if (g_speed > 10.0f) g_speed = 10.0f;
        if (g_speedhack) {
            CCDirector::get()->getScheduler()->setTimeScale(g_speed);
        }
        refreshButtons();
    }
    void onSpeedhackDown(CCObject*) {
        g_speed -= 0.5f;
        if (g_speed < 0.0f) g_speed = 0.0f;
        if (g_speedhack) {
            CCDirector::get()->getScheduler()->setTimeScale(g_speed);
        }
        refreshButtons();
    }
    void onClose(CCObject*) {
        this->runAction(CCSequence::create(
            CCEaseBackIn::create(CCScaleTo::create(0.2f, 0.3f)),
            CCCallFunc::create(this, callfunc_selector(NeverloseMenu::removeMe)),
            nullptr));
    }
    void removeMe() { this->removeFromParentAndCleanup(true); }
    void keyBackClicked() { onClose(nullptr); }
};

// ================= PLAYER =================
class $modify(MyPlayLayer, PlayLayer) {
    void applySpikeESP() {
        if (!m_objectLayer) return;
        auto children = m_objectLayer->getChildren();
        if (!children) return;
        for (unsigned int i = 0; i < children->count(); i++) {
            auto obj = typeinfo_cast<GameObject*>(children->objectAtIndex(i));
            if (!obj) continue;
            int id = obj->m_objectID;
            bool isSpike = (id == 8 || id == 39 || id == 103 || id == 392 || id == 421 || id == 422 || id == 1322 || id == 1323 || id == 1333);
            if (isSpike && g_spikeESP) {
                obj->setColor({255, 50, 50});
                if (!obj->getChildByID("nl_spike_red")) {
                    auto red = CCLayerColor::create({255, 0, 0, 130}, obj->getContentSize().width, obj->getContentSize().height);
                    red->setID("nl_spike_red");
                    red->setAnchorPoint({0, 0});
                    red->setPosition({0, 0});
                    obj->addChild(red, 999);
                }
            } else if (!g_spikeESP) {
                if (auto red = obj->getChildByID("nl_spike_red")) red->removeFromParent();
            }
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        if (g_noclip || g_fly) return;
        PlayLayer::destroyPlayer(player, obj);
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (g_autoJump && m_player1) m_player1->pushButton(PlayerButton::Jump);
        if (g_jumpHack && m_player1) m_player1->m_yVelocity = 20.0f;
        if (g_fly && m_player1) {
            auto pos = m_player1->m_position;
            if (g_touchHeld) pos.y += 7;
            else pos.y -= 5;
            m_player1->m_position = pos;
        }
        applySpikeESP();
    }

    void updateVisibility(float dt) {
        PlayLayer::updateVisibility(dt);
        if (!m_debugDrawNode) return;
        bool shouldVis = g_showHitbox;
        if (shouldVis) PlayLayer::updateDebugDraw();
        m_debugDrawNode->setVisible(shouldVis);
    }

    void onExit() {
        CCDirector::get()->getScheduler()->setTimeScale(1.0f);
        PlayLayer::onExit();
    }
};

class $modify(MyPlayer, PlayerObject) {
    void pushButton(PlayerButton btn) {
        if (btn == PlayerButton::Jump) g_touchHeld = true;
        PlayerObject::pushButton(btn);
    }
    void releaseButton(PlayerButton btn) {
        if (btn == PlayerButton::Jump) g_touchHeld = false;
        PlayerObject::releaseButton(btn);
    }
    void update(float dt) {
        PlayerObject::update(dt);
        if (g_spinbot) this->setRotation(this->getRotation() + g_spinSpeed);
    }
};

// ================= PAUSE BUTTON =================
class $modify(MyPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        auto ws = CCDirector::get()->getWinSize();
        auto btn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Neverlose"), this, menu_selector(MyPauseLayer::onNeverlose));
        btn->setPosition({ws.width * 0.12f, ws.height * 0.72f});
        auto menu = CCMenu::create();
        menu->addChild(btn);
        menu->setPosition({0, 0});
        this->addChild(menu, 100);
    }
    void onNeverlose(CCObject*) {
        auto menu = NeverloseMenu::create();
        if (menu) this->addChild(menu, 200);
    }
};
