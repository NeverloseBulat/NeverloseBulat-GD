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
static float g_speed = 1.0f;
static float g_spinSpeed = 5.0f;
static bool g_jumpHeld = false;

// ============ МЕНЮ ============
class NeverloseMenu : public CCLayer {
protected:
    std::vector<CCNode*> m_rageNodes;
    std::vector<CCNode*> m_visualsNodes;
    std::vector<CCNode*> m_antiAimNodes;

    CCMenuItemSpriteExtra* m_noclipBtn = nullptr;
    CCMenuItemSpriteExtra* m_flyBtn = nullptr;
    CCMenuItemSpriteExtra* m_spikeESPBtn = nullptr;
    CCMenuItemSpriteExtra* m_autoJumpBtn = nullptr;

    CCMenuItemSpriteExtra* m_ldmBtn = nullptr;
    CCMenuItemSpriteExtra* m_autoLDMBtn = nullptr;

    CCMenuItemSpriteExtra* m_spinbotBtn = nullptr;
    CCMenuItemSpriteExtra* m_spinDown = nullptr;
    CCMenuItemSpriteExtra* m_spinUp = nullptr;
    CCLabelBMFont* m_spinLabel = nullptr;
    CCLabelBMFont* m_spinText = nullptr;

    CCMenuItemSpriteExtra* m_speedhackBtn = nullptr;
    CCMenuItemSpriteExtra* m_shDown = nullptr;
    CCMenuItemSpriteExtra* m_shUp = nullptr;
    CCLabelBMFont* m_shLabel = nullptr;
    CCLabelBMFont* m_shText = nullptr;

    CCMenuItemSpriteExtra* m_tabRage = nullptr;
    CCMenuItemSpriteExtra* m_tabVisuals = nullptr;
    CCMenuItemSpriteExtra* m_tabAntiAim = nullptr;

public:
    static NeverloseMenu* create() {
        auto r = new NeverloseMenu();
        if (r && r->init()) { r->autorelease(); return r; }
        delete r; return nullptr;
    }

    void refreshButtons() {
        m_noclipBtn->setNormalImage(ButtonSprite::create(g_noclip ? "Noclip: ON" : "Noclip: OFF"));
        m_flyBtn->setNormalImage(ButtonSprite::create(g_fly ? "Fly: ON" : "Fly: OFF"));
        m_spikeESPBtn->setNormalImage(ButtonSprite::create(g_spikeESP ? "ESP Spikes: ON" : "ESP Spikes: OFF"));
        m_autoJumpBtn->setNormalImage(ButtonSprite::create(g_autoJump ? "AutoJump: ON" : "AutoJump: OFF"));
        m_ldmBtn->setNormalImage(ButtonSprite::create(g_ldm ? "LDM: ON" : "LDM: OFF"));
        m_autoLDMBtn->setNormalImage(ButtonSprite::create(g_autoLDM ? "Auto LDM: ON" : "Auto LDM: OFF"));
        m_spinbotBtn->setNormalImage(ButtonSprite::create(g_spinbot ? "Spinbot: ON" : "Spinbot: OFF"));
        m_speedhackBtn->setNormalImage(ButtonSprite::create(g_speedhack ? "Speedhack: ON" : "Speedhack: OFF"));
        m_shLabel->setString(CCString::createWithFormat("%.3f", g_speed)->getCString());
    }

    void setPage(int p) {
        for (auto n : m_rageNodes) n->setVisible(p == 0);
        for (auto n : m_visualsNodes) n->setVisible(p == 1);
        for (auto n : m_antiAimNodes) n->setVisible(p == 2);
        m_tabRage->setColor(p == 0 ? ccWHITE : ccGRAY);
        m_tabVisuals->setColor(p == 1 ? ccWHITE : ccGRAY);
        m_tabAntiAim->setColor(p == 2 ? ccWHITE : ccGRAY);
    }

    bool init() {
        if (!CCLayer::init()) return false;
        auto ws = CCDirector::get()->getWinSize();
        float cx = ws.width / 2, cy = ws.height / 2;

        auto overlay = CCLayerColor::create({0, 0, 0, 180});
        this->addChild(overlay, -1);

        auto panel = CCScale9Sprite::create("GJ_square01.png");
        panel->setContentSize({760, 480});
        panel->setPosition({cx, cy});
        panel->setColor({15, 15, 15});
        this->addChild(panel);

        auto sidebar = CCScale9Sprite::create("GJ_square01.png");
        sidebar->setContentSize({220, 460});
        sidebar->setPosition({cx - 260, cy});
        sidebar->setColor({25, 25, 30});
        this->addChild(sidebar);

        auto logo = CCLabelBMFont::create("NEVERLOSE", "goldFont.fnt");
        logo->setPosition({cx - 260, cy + 195});
        logo->setScale(0.65f);
        logo->setColor({255, 255, 255});
        this->addChild(logo);

        auto catAim = CCLabelBMFont::create("Aimbot", "bigFont.fnt");
        catAim->setPosition({cx - 320, cy + 130});
        catAim->setScale(0.4f);
        catAim->setColor({120, 120, 130});
        this->addChild(catAim);

        auto catVis = CCLabelBMFont::create("Visuals", "bigFont.fnt");
        catVis->setPosition({cx - 320, cy + 5});
        catVis->setScale(0.4f);
        catVis->setColor({120, 120, 130});
        this->addChild(catVis);

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        this->addChild(menu);

        // КРЕСТИК — спущен ниже, чтобы не улетал за экран
        auto cs = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        cs->setScale(0.8f);
        auto closeBtn = CCMenuItemSpriteExtra::create(cs, this, menu_selector(NeverloseMenu::onClose));
        closeBtn->setPosition({cx + 340, cy + 195});
        menu->addChild(closeBtn);

        m_tabRage = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Rage"), this, menu_selector(NeverloseMenu::onTabRage));
        m_tabRage->setPosition({cx - 260, cy + 90});
        menu->addChild(m_tabRage);

        m_tabVisuals = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Visuals"), this, menu_selector(NeverloseMenu::onTabVisuals));
        m_tabVisuals->setPosition({cx - 260, cy - 10});
        menu->addChild(m_tabVisuals);

        m_tabAntiAim = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Anti-Aim"), this, menu_selector(NeverloseMenu::onTabAntiAim));
        m_tabAntiAim->setPosition({cx - 260, cy - 110});
        menu->addChild(m_tabAntiAim);

        // ========== RAGE ==========
        m_noclipBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Noclip: OFF"), this, menu_selector(NeverloseMenu::onNoclip));
        m_noclipBtn->setPosition({cx + 60, cy + 120});
        menu->addChild(m_noclipBtn); m_rageNodes.push_back(m_noclipBtn);

        m_flyBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Fly: OFF"), this, menu_selector(NeverloseMenu::onFly));
        m_flyBtn->setPosition({cx + 60, cy + 55});
        menu->addChild(m_flyBtn); m_rageNodes.push_back(m_flyBtn);

        m_spikeESPBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("ESP Spikes: OFF"), this, menu_selector(NeverloseMenu::onSpikeESP));
        m_spikeESPBtn->setPosition({cx + 60, cy - 10});
        menu->addChild(m_spikeESPBtn); m_rageNodes.push_back(m_spikeESPBtn);

        m_autoJumpBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("AutoJump: OFF"), this, menu_selector(NeverloseMenu::onAutoJump));
        m_autoJumpBtn->setPosition({cx + 60, cy - 75});
        menu->addChild(m_autoJumpBtn); m_rageNodes.push_back(m_autoJumpBtn);

        // ========== VISUALS ==========
        m_ldmBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("LDM: OFF"), this, menu_selector(NeverloseMenu::onLDM));
        m_ldmBtn->setPosition({cx + 60, cy + 40});
        menu->addChild(m_ldmBtn); m_visualsNodes.push_back(m_ldmBtn);

        m_autoLDMBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Auto LDM: OFF"), this, menu_selector(NeverloseMenu::onAutoLDM));
        m_autoLDMBtn->setPosition({cx + 60, cy - 20});
        menu->addChild(m_autoLDMBtn); m_visualsNodes.push_back(m_autoLDMBtn);

        // ========== ANTI-AIM ==========
        m_spinbotBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Spinbot: OFF"), this, menu_selector(NeverloseMenu::onSpinbot));
        m_spinbotBtn->setPosition({cx + 60, cy + 130});
        menu->addChild(m_spinbotBtn); m_antiAimNodes.push_back(m_spinbotBtn);

        m_spinText = CCLabelBMFont::create("Spinbot Speed", "bigFont.fnt");
        m_spinText->setPosition({cx - 40, cy + 60});
        m_spinText->setScale(0.5f);
        m_spinText->setColor({0, 200, 255});
        this->addChild(m_spinText); m_antiAimNodes.push_back(m_spinText);

        m_spinDown = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("<"), this, menu_selector(NeverloseMenu::onSpinDown));
        m_spinDown->setPosition({cx + 70, cy + 60});
        menu->addChild(m_spinDown); m_antiAimNodes.push_back(m_spinDown);

        m_spinLabel = CCLabelBMFont::create("5.00", "bigFont.fnt");
        m_spinLabel->setPosition({cx + 135, cy + 60});
        m_spinLabel->setScale(0.7f);
        m_spinLabel->setColor({0, 200, 255});
        this->addChild(m_spinLabel); m_antiAimNodes.push_back(m_spinLabel);

        m_spinUp = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(">"), this, menu_selector(NeverloseMenu::onSpinUp));
        m_spinUp->setPosition({cx + 200, cy + 60});
        menu->addChild(m_spinUp); m_antiAimNodes.push_back(m_spinUp);

        m_speedhackBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Speedhack: OFF"), this, menu_selector(NeverloseMenu::onSpeedhack));
        m_speedhackBtn->setPosition({cx + 60, cy - 30});
        menu->addChild(m_speedhackBtn); m_antiAimNodes.push_back(m_speedhackBtn);

        m_shText = CCLabelBMFont::create("Speedhack Value", "bigFont.fnt");
        m_shText->setPosition({cx - 30, cy - 100});
        m_shText->setScale(0.5f);
        m_shText->setColor({0, 200, 255});
        this->addChild(m_shText); m_antiAimNodes.push_back(m_shText);

        m_shDown = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("<"), this, menu_selector(NeverloseMenu::onSpeedhackDown));
        m_shDown->setPosition({cx + 70, cy - 100});
        menu->addChild(m_shDown); m_antiAimNodes.push_back(m_shDown);

        m_shLabel = CCLabelBMFont::create("1.000", "bigFont.fnt");
        m_shLabel->setPosition({cx + 135, cy - 100});
        m_shLabel->setScale(0.7f);
        m_shLabel->setColor({0, 200, 255});
        this->addChild(m_shLabel); m_antiAimNodes.push_back(m_shLabel);

        m_shUp = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(">"), this, menu_selector(NeverloseMenu::onSpeedhackUp));
        m_shUp->setPosition({cx + 200, cy - 100});
        menu->addChild(m_shUp); m_antiAimNodes.push_back(m_shUp);

        setPage(0);
        refreshButtons();

        this->setScale(0.3f);
        this->runAction(CCEaseBackOut::create(CCScaleTo::create(0.35f, 1.0f)));
        this->setKeypadEnabled(true);
        return true;
    }

    void onTabRage(CCObject*) { setPage(0); }
    void onTabVisuals(CCObject*) { setPage(1); }
    void onTabAntiAim(CCObject*) { setPage(2); }

    void onNoclip(CCObject*) { g_noclip = !g_noclip; refreshButtons(); }
    void onFly(CCObject*) { g_fly = !g_fly; refreshButtons(); }
    void onSpikeESP(CCObject*) { g_spikeESP = !g_spikeESP; refreshButtons(); }
    void onAutoJump(CCObject*) { g_autoJump = !g_autoJump; refreshButtons(); }

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

    void onSpinbot(CCObject*) { g_spinbot = !g_spinbot; refreshButtons(); }
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
        if (g_speedhack) CCDirector::get()->getScheduler()->setTimeScale(g_speed);
        else CCDirector::get()->getScheduler()->setTimeScale(1.0f);
        refreshButtons();
    }
    void onSpeedhackUp(CCObject*) {
        g_speed += 0.5f;
        if (g_speed > 10.0f) g_speed = 10.0f;
        if (g_speedhack) CCDirector::get()->getScheduler()->setTimeScale(g_speed);
        refreshButtons();
    }
    void onSpeedhackDown(CCObject*) {
        g_speed -= 0.5f;
        if (g_speed < 0.0f) g_speed = 0.0f;
        if (g_speedhack) CCDirector::get()->getScheduler()->setTimeScale(g_speed);
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

// ============ PLAYER ============
class $modify(MyPlayLayer, PlayLayer) {
    void applySpikeESP() {
        if (!g_spikeESP || !m_objectLayer) return;
        auto children = m_objectLayer->getChildren();
        if (!children) return;

        for (unsigned int i = 0; i < children->count(); i++) {
            auto obj = typeinfo_cast<GameObject*>(children->objectAtIndex(i));
            if (!obj) continue;

            bool isSpike = (obj->m_objectID == 8    || obj->m_objectID == 39  ||
                            obj->m_objectID == 103  || obj->m_objectID == 392 ||
                            obj->m_objectID == 421  || obj->m_objectID == 422 ||
                            obj->m_objectID == 1322 || obj->m_objectID == 1323 ||
                            obj->m_objectID == 1333);

            if (!isSpike && obj->m_objectType != GameObjectType::Hazard) continue;

            obj->setColor({255, 50, 50});

            if (!obj->getChildByID("nl_spike_red")) {
                auto redOverlay = CCLayerColor::create(
                    {255, 0, 0, 130},
                    obj->getContentSize().width,
                    obj->getContentSize().height
                );
                redOverlay->setID("nl_spike_red");
                redOverlay->setPosition({0, 0});
                redOverlay->setAnchorPoint({0, 0});
                obj->addChild(redOverlay, 999);
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

        if (g_fly && m_player1) {
            auto pos = m_player1->getPosition();
            if (g_jumpHeld) pos.y += 7;
            else pos.y -= 7;
            m_player1->setPosition(pos);
        }

        applySpikeESP();
    }

    void onExit() {
        CCDirector::get()->getScheduler()->setTimeScale(1.0f);
        PlayLayer::onExit();
    }
};

class $modify(MyPlayer, PlayerObject) {
    void pushButton(PlayerButton btn) {
        if (btn == PlayerButton::Jump) g_jumpHeld = true;
        PlayerObject::pushButton(btn);
    }
    void releaseButton(PlayerButton btn) {
        if (btn == PlayerButton::Jump) g_jumpHeld = false;
        PlayerObject::releaseButton(btn);
    }
    void update(float dt) {
        PlayerObject::update(dt);
        if (g_spinbot) this->setRotation(this->getRotation() + g_spinSpeed);
    }
};

// ============ PAUSE BUTTON ============
class $modify(MyPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        auto ws = CCDirector::get()->getWinSize();
        auto btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Neverlose"), this, menu_selector(MyPauseLayer::onNeverlose));
        btn->setPosition({ws.width * 0.25f, ws.height * 0.72f});
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
