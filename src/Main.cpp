#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <cstdlib>

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
static bool g_flyW = false, g_flyA = false, g_flyS = false, g_flyD = false;

// ============ INPUT POPUP ============
class SpeedInputPopup : public CCLayer, public TextInputDelegate {
protected:
    CCTextInputNode* m_input = nullptr;
public:
    static SpeedInputPopup* create() {
        auto ret = new SpeedInputPopup();
        if (ret && ret->init()) { ret->autorelease(); return ret; }
        delete ret; return nullptr;
    }
    bool init() {
        if (!CCLayer::init()) return false;
        auto ws = CCDirector::get()->getWinSize();

        auto bg = CCLayerColor::create({0, 0, 0, 200});
        this->addChild(bg, -1);

        auto panel = CCScale9Sprite::create("GJ_square01.png");
        panel->setContentSize({340, 200});
        panel->setPosition(ws / 2);
        this->addChild(panel);

        auto title = CCLabelBMFont::create("Speedhack", "goldFont.fnt");
        title->setPosition({ws.width / 2, ws.height / 2 + 65});
        title->setScale(0.7f);
        title->setColor({0, 200, 255});
        this->addChild(title);

        auto hint = CCLabelBMFont::create("0.000 - 10.000", "bigFont.fnt");
        hint->setPosition({ws.width / 2, ws.height / 2 + 30});
        hint->setScale(0.4f);
        hint->setColor({180, 180, 180});
        this->addChild(hint);

        m_input = CCTextInputNode::create(220, 40, "1.000", "bigFont.fnt");
        m_input->setPosition({ws.width / 2, ws.height / 2 - 10});
        m_input->setDelegate(this);
        m_input->setString(CCString::createWithFormat("%.3f", g_speed)->getCString());
        this->addChild(m_input);

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        this->addChild(menu);

        auto okBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("OK"), this, menu_selector(SpeedInputPopup::onOK));
        okBtn->setPosition({ws.width / 2, ws.height / 2 - 65});
        menu->addChild(okBtn);

        this->setKeypadEnabled(true);
        return true;
    }
    void onOK(CCObject*) {
        std::string s = m_input->getString();
        float v = std::atof(s.c_str());
        if (v < 0.0f) v = 0.0f;
        if (v > 10.0f) v = 10.0f;
        g_speed = v;
        if (g_speedhack) CCDirector::get()->getScheduler()->setTimeScale(g_speed);
        this->removeFromParentAndCleanup(true);
    }
    void keyBackClicked() { this->removeFromParentAndCleanup(true); }
};

// ============ MENU ============
class NeverloseMenu : public CCLayer {
protected:
    std::vector<CCNode*> m_rageNodes;
    std::vector<CCNode*> m_visualsNodes;
    std::vector<CCNode*> m_antiAimNodes;

    CCMenuItemSpriteExtra* m_noclipBtn = nullptr;
    CCMenuItemSpriteExtra* m_flyBtn = nullptr;
    CCMenuItemSpriteExtra* m_spikeESPBtn = nullptr;
    CCMenuItemSpriteExtra* m_autoJumpBtn = nullptr;
    CCMenuItemSpriteExtra* m_speedBtn = nullptr;

    CCMenuItemSpriteExtra* m_ldmBtn = nullptr;
    CCMenuItemSpriteExtra* m_autoLDMBtn = nullptr;

    CCMenuItemSpriteExtra* m_spinbotBtn = nullptr;
    CCMenuItemSpriteExtra* m_spinDown = nullptr;
    CCMenuItemSpriteExtra* m_spinUp = nullptr;
    CCLabelBMFont* m_spinLabel = nullptr;
    CCLabelBMFont* m_spinText = nullptr;
    CCMenuItemSpriteExtra* m_speedhackBtn = nullptr;
    CCMenuItemSpriteExtra* m_speedHackValue = nullptr;
    CCLabelBMFont* m_shText = nullptr;

    CCMenuItemSpriteExtra* m_tabRage = nullptr;
    CCMenuItemSpriteExtra* m_tabVisuals = nullptr;
    CCMenuItemSpriteExtra* m_tabAntiAim = nullptr;

    int m_page = 0;

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
        m_speedBtn->setNormalImage(ButtonSprite::create(
            g_speed == 1.0f ? "Speed: 1.0x" : (g_speed == 2.0f ? "Speed: 2.0x" : "Speed: 0.5x")));
        m_ldmBtn->setNormalImage(ButtonSprite::create(g_ldm ? "LDM: ON" : "LDM: OFF"));
        m_autoLDMBtn->setNormalImage(ButtonSprite::create(g_autoLDM ? "Auto LDM: ON" : "Auto LDM: OFF"));
        m_spinbotBtn->setNormalImage(ButtonSprite::create(g_spinbot ? "Spinbot: ON" : "Spinbot: OFF"));
        m_speedhackBtn->setNormalImage(ButtonSprite::create(g_speedhack ? "Speedhack: ON" : "Speedhack: OFF"));
        m_speedHackValue->setNormalImage(ButtonSprite::create(
            CCString::createWithFormat("%.3f", g_speed)->getCString()));
    }

    void setPage(int p) {
        m_page = p;
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

        auto cs = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        cs->setScale(0.8f);
        auto closeBtn = CCMenuItemSpriteExtra::create(cs, this, menu_selector(NeverloseMenu::onClose));
        closeBtn->setPosition({cx + 360, cy + 215});
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

        // RAGE
        m_noclipBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Noclip: OFF"), this, menu_selector(NeverloseMenu::onNoclip));
        m_noclipBtn->setPosition({cx + 60, cy + 140});
        menu->addChild(m_noclipBtn); m_rageNodes.push_back(m_noclipBtn);

        m_flyBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Fly: OFF"), this, menu_selector(NeverloseMenu::onFly));
        m_flyBtn->setPosition({cx + 60, cy + 80});
        menu->addChild(m_flyBtn); m_rageNodes.push_back(m_flyBtn);

        m_spikeESPBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("ESP Spikes: OFF"), this, menu_selector(NeverloseMenu::onSpikeESP));
        m_spikeESPBtn->setPosition({cx + 60, cy + 20});
        menu->addChild(m_spikeESPBtn); m_rageNodes.push_back(m_spikeESPBtn);

        m_autoJumpBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("AutoJump: OFF"), this, menu_selector(NeverloseMenu::onAutoJump));
        m_autoJumpBtn->setPosition({cx + 60, cy - 40});
        menu->addChild(m_autoJumpBtn); m_rageNodes.push_back(m_autoJumpBtn);

        m_speedBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Speed: 1.0x"), this, menu_selector(NeverloseMenu::onSpeed));
        m_speedBtn->setPosition({cx + 60, cy - 100});
        menu->addChild(m_speedBtn); m_rageNodes.push_back(m_speedBtn);

        // VISUALS
        m_ldmBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("LDM: OFF"), this, menu_selector(NeverloseMenu::onLDM));
        m_ldmBtn->setPosition({cx + 60, cy + 40});
        menu->addChild(m_ldmBtn); m_visualsNodes.push_back(m_ldmBtn);

        m_autoLDMBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Auto LDM: OFF"), this, menu_selector(NeverloseMenu::onAutoLDM));
        m_autoLDMBtn->setPosition({cx + 60, cy - 20});
        menu->addChild(m_autoLDMBtn); m_visualsNodes.push_back(m_autoLDMBtn);

        // ANTI-AIM
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

        m_shText = CCLabelBMFont::create("Value (click to edit):", "bigFont.fnt");
        m_shText->setPosition({cx - 20, cy - 100});
        m_shText->setScale(0.45f);
        m_shText->setColor({0, 200, 255});
        this->addChild(m_shText); m_antiAimNodes.push_back(m_shText);

        m_speedHackValue = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("1.000"), this, menu_selector(NeverloseMenu::onSpeedValueClick));
        m_speedHackValue->setPosition({cx + 130, cy - 100});
        menu->addChild(m_speedHackValue); m_antiAimNodes.push_back(m_speedHackValue);

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
    void onSpeed(CCObject*) {
        if (g_speed == 1.0f) g_speed = 2.0f;
        else if (g_speed == 2.0f) g_speed = 0.5f;
        else g_speed = 1.0f;
        if (g_speedhack) CCDirector::get()->getScheduler()->setTimeScale(g_speed);
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
    void onSpeedValueClick(CCObject*) {
        auto popup = SpeedInputPopup::create();
        if (popup) this->addChild(popup, 999);
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
            int id = obj->m_objectID;
            if (id == 8 || id == 39 || id == 103 || id == 392 || id == 421 ||
                id == 422 || id == 1322 || id == 1323) {
                obj->setColor({255, 50, 50});
            }
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        if (g_noclip) return;
        PlayLayer::destroyPlayer(player, obj);
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (g_autoJump && m_player1) m_player1->pushButton(PlayerButton::Jump);
        if (g_fly && m_player1) {
            auto pos = m_player1->getPosition();
            float sp = 6.0f;
            if (g_flyW) pos.y += sp;
            if (g_flyS) pos.y -= sp;
            if (g_flyA) pos.x -= sp;
            if (g_flyD) pos.x += sp;
            m_player1->setPosition(pos);
        }
        applySpikeESP();
    }

    void keyDown(cocos2d::enumKeyCodes key, double t) {
        if (g_fly) {
            if (key == cocos2d::KEY_W) g_flyW = true;
            if (key == cocos2d::KEY_A) g_flyA = true;
            if (key == cocos2d::KEY_S) g_flyS = true;
            if (key == cocos2d::KEY_D) g_flyD = true;
        }
        PlayLayer::keyDown(key, t);
    }
    void keyUp(cocos2d::enumKeyCodes key, double t) {
        if (key == cocos2d::KEY_W) g_flyW = false;
        if (key == cocos2d::KEY_A) g_flyA = false;
        if (key == cocos2d::KEY_S) g_flyS = false;
        if (key == cocos2d::KEY_D) g_flyD = false;
        PlayLayer::keyUp(key, t);
    }

    void onExit() {
        CCDirector::get()->getScheduler()->setTimeScale(1.0f);
        PlayLayer::onExit();
    }
};

class $modify(MyPlayer, PlayerObject) {
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
