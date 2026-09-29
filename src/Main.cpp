#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <cstdlib>

using namespace geode::prelude;

static bool g_noclip = false;
static bool g_autoJump = false;
static bool g_spinbot = false;
static bool g_ldm = false;
static bool g_autoLDM = false;
static bool g_speedhack = false;
static bool g_jumpHack = false;
static bool g_autoClicker = false;
static bool g_copyHack = false;
static float g_speed = 1.0f;
static float g_spinSpeed = 5.0f;
static bool g_touchHeld = false;
static int g_clickTimer = 0;

// Для обновления лейблов из попапа
static CCLabelBMFont* g_speedLabel = nullptr;
static CCLabelBMFont* g_spinLabel = nullptr;

// ================= POPUP =================
class ValueInputPopup : public CCLayer, public TextInputDelegate {
protected:
    CCTextInputNode* m_input = nullptr;
    bool m_isSpeed = true;
public:
    static ValueInputPopup* create(bool isSpeed) {
        auto r = new ValueInputPopup();
        r->m_isSpeed = isSpeed;
        if (r && r->init()) { r->autorelease(); return r; }
        delete r; return nullptr;
    }
    bool init() {
        if (!CCLayer::init()) return false;
        auto ws = CCDirector::get()->getWinSize();

        auto bg = CCLayerColor::create({0, 0, 0, 200});
        this->addChild(bg, -1);

        auto panel = CCScale9Sprite::create("GJ_square01.png");
        panel->setContentSize({340, 220});
        panel->setPosition(ws / 2);
        this->addChild(panel);

        auto title = CCLabelBMFont::create(m_isSpeed ? "Speedhack Value" : "Spinbot Speed", "goldFont.fnt");
        title->setPosition({ws.width / 2, ws.height / 2 + 70});
        title->setScale(0.7f);
        title->setColor({0, 200, 255});
        this->addChild(title);

        auto hint = CCLabelBMFont::create(m_isSpeed ? "0.000 - 100.000" : "1.00 - 100.00", "bigFont.fnt");
        hint->setPosition({ws.width / 2, ws.height / 2 + 35});
        hint->setScale(0.4f);
        hint->setColor({180, 180, 180});
        this->addChild(hint);

        m_input = CCTextInputNode::create(220, 40, "1.0", "bigFont.fnt");
        m_input->setPosition({ws.width / 2, ws.height / 2 - 5});
        m_input->setDelegate(this);
        if (m_isSpeed) {
            m_input->setString(CCString::createWithFormat("%.3f", g_speed)->getCString());
        } else {
            m_input->setString(CCString::createWithFormat("%.2f", g_spinSpeed)->getCString());
        }
        this->addChild(m_input);

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        this->addChild(menu);

        auto okBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("OK"), this, menu_selector(ValueInputPopup::onOK));
        okBtn->setPosition({ws.width / 2 - 60, ws.height / 2 - 70});
        menu->addChild(okBtn);

        auto cancelBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Cancel"), this, menu_selector(ValueInputPopup::onCancel));
        cancelBtn->setPosition({ws.width / 2 + 60, ws.height / 2 - 70});
        menu->addChild(cancelBtn);

        this->setKeypadEnabled(true);
        return true;
    }
    void onOK(CCObject*) {
        std::string s = m_input->getString();
        float v = (float)std::atof(s.c_str());
        if (m_isSpeed) {
            if (v < 0.0f) v = 0.0f;
            if (v > 100.0f) v = 100.0f;
            g_speed = v;
            if (g_speedhack) CCDirector::get()->getScheduler()->setTimeScale(g_speed);
            if (g_speedLabel) g_speedLabel->setString(CCString::createWithFormat("%.3f", g_speed)->getCString());
        } else {
            if (v < 1.0f) v = 1.0f;
            if (v > 100.0f) v = 100.0f;
            g_spinSpeed = v;
            if (g_spinLabel) g_spinLabel->setString(CCString::createWithFormat("%.2f", g_spinSpeed)->getCString());
        }
        this->removeFromParentAndCleanup(true);
    }
    void onCancel(CCObject*) { this->removeFromParentAndCleanup(true); }
    void keyBackClicked() { this->removeFromParentAndCleanup(true); }
};

// ================= MENU =================
class NeverloseMenu : public CCLayer {
protected:
    std::vector<CCNode*> m_mainNodes, m_rageNodes, m_visualsNodes, m_antiAimNodes, m_createNodes;

    CCMenuItemSpriteExtra *m_jumpHackBtn = nullptr;
    CCMenuItemSpriteExtra *m_autoClickerBtn = nullptr;
    CCMenuItemSpriteExtra *m_copyHackBtn = nullptr;

    CCMenuItemSpriteExtra *m_noclipBtn, *m_autoJumpBtn;
    CCMenuItemSpriteExtra *m_ldmBtn, *m_autoLDMBtn;
    CCMenuItemSpriteExtra *m_spinbotBtn, *m_spinDown, *m_spinUp, *m_spinValueBtn;
    CCMenuItemSpriteExtra *m_speedhackBtn, *m_shDown, *m_shUp, *m_shValueBtn;

    CCMenuItemSpriteExtra *m_tabMain, *m_tabRage, *m_tabVisuals, *m_tabAntiAim, *m_tabCreate;
    CCMenuItemSpriteExtra *m_prevBtn, *m_nextBtn;

public:
    static NeverloseMenu* create() {
        auto r = new NeverloseMenu();
        if (r && r->init()) { r->autorelease(); return r; }
        delete r; return nullptr;
    }

    void refreshButtons() {
        m_jumpHackBtn->setNormalImage(ButtonSprite::create(g_jumpHack ? "Jump Hack: ON" : "Jump Hack: OFF"));
        m_autoClickerBtn->setNormalImage(ButtonSprite::create(g_autoClicker ? "Auto Clicker: ON" : "Auto Clicker: OFF"));
        m_noclipBtn->setNormalImage(ButtonSprite::create(g_noclip ? "Noclip: ON" : "Noclip: OFF"));
        m_autoJumpBtn->setNormalImage(ButtonSprite::create(g_autoJump ? "AutoJump: ON" : "AutoJump: OFF"));
        m_ldmBtn->setNormalImage(ButtonSprite::create(g_ldm ? "LDM: ON" : "LDM: OFF"));
        m_autoLDMBtn->setNormalImage(ButtonSprite::create(g_autoLDM ? "Auto LDM: ON" : "Auto LDM: OFF"));
        m_spinbotBtn->setNormalImage(ButtonSprite::create(g_spinbot ? "Spinbot: ON" : "Spinbot: OFF"));
        m_speedhackBtn->setNormalImage(ButtonSprite::create(g_speedhack ? "Speedhack: ON" : "Speedhack: OFF"));
        m_copyHackBtn->setNormalImage(ButtonSprite::create(g_copyHack ? "Copy Hack: ON" : "Copy Hack: OFF"));
    }

    void setPage(int p) {
        for (auto n : m_mainNodes) n->setVisible(p == 0);
        for (auto n : m_rageNodes) n->setVisible(p == 1);
        for (auto n : m_visualsNodes) n->setVisible(p == 2);
        for (auto n : m_antiAimNodes) n->setVisible(p == 3);
        for (auto n : m_createNodes) n->setVisible(p == 4);
        m_tabMain->setColor(p == 0 ? ccWHITE : ccGRAY);
        m_tabRage->setColor(p == 1 ? ccWHITE : ccGRAY);
        m_tabVisuals->setColor(p == 2 ? ccWHITE : ccGRAY);
        m_tabAntiAim->setColor(p == 3 ? ccWHITE : ccGRAY);
        m_tabCreate->setColor(p == 4 ? ccWHITE : ccGRAY);
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

        // Крестик — сдвинут немного левее
        auto cs = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        cs->setScale(0.8f);
        auto closeBtn = CCMenuItemSpriteExtra::create(cs, this, menu_selector(NeverloseMenu::onClose));
        closeBtn->setPosition({cx + 320, cy});
        menu->addChild(closeBtn);

        // Tabs
        float tabY[5] = {cy + 160, cy + 80, cy, cy - 80, cy - 160};
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

        m_tabCreate = CCMenuItemSpriteExtra::create(ButtonSprite::create("Create"), this, menu_selector(NeverloseMenu::onTabCreate));
        m_tabCreate->setPosition({cx - 270, tabY[4]});
        menu->addChild(m_tabCreate);

        // MAIN
        m_jumpHackBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Jump Hack: OFF"), this, menu_selector(NeverloseMenu::onJumpHack));
        m_jumpHackBtn->setPosition({cx + 60, cy + 130});
        menu->addChild(m_jumpHackBtn); m_mainNodes.push_back(m_jumpHackBtn);

        m_autoClickerBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Auto Clicker: OFF"), this, menu_selector(NeverloseMenu::onAutoClicker));
        m_autoClickerBtn->setPosition({cx + 60, cy + 60});
        menu->addChild(m_autoClickerBtn); m_mainNodes.push_back(m_autoClickerBtn);

        // RAGE
        m_noclipBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Noclip: OFF"), this, menu_selector(NeverloseMenu::onNoclip));
        m_noclipBtn->setPosition({cx + 60, cy + 60});
        menu->addChild(m_noclipBtn); m_rageNodes.push_back(m_noclipBtn);

        m_autoJumpBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("AutoJump: OFF"), this, menu_selector(NeverloseMenu::onAutoJump));
        m_autoJumpBtn->setPosition({cx + 60, cy - 10});
        menu->addChild(m_autoJumpBtn); m_rageNodes.push_back(m_autoJumpBtn);

        // VISUALS
        m_ldmBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("LDM: OFF"), this, menu_selector(NeverloseMenu::onLDM));
        m_ldmBtn->setPosition({cx + 60, cy + 90});
        menu->addChild(m_ldmBtn); m_visualsNodes.push_back(m_ldmBtn);

        m_autoLDMBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Auto LDM: OFF"), this, menu_selector(NeverloseMenu::onAutoLDM));
        m_autoLDMBtn->setPosition({cx + 60, cy + 25});
        menu->addChild(m_autoLDMBtn); m_visualsNodes.push_back(m_autoLDMBtn);

        // ANTI-AIM
        m_spinbotBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Spinbot: OFF"), this, menu_selector(NeverloseMenu::onSpinbot));
        m_spinbotBtn->setPosition({cx + 60, cy + 140});
        menu->addChild(m_spinbotBtn); m_antiAimNodes.push_back(m_spinbotBtn);

        auto spinText = CCLabelBMFont::create("Spinbot Speed:", "bigFont.fnt");
        spinText->setPosition({cx - 60, cy + 70});
        spinText->setScale(0.5f);
        spinText->setColor({0, 200, 255});
        this->addChild(spinText); m_antiAimNodes.push_back(spinText);

        m_spinDown = CCMenuItemSpriteExtra::create(ButtonSprite::create("<"), this, menu_selector(NeverloseMenu::onSpinDown));
        m_spinDown->setPosition({cx + 30, cy + 70});
        menu->addChild(m_spinDown); m_antiAimNodes.push_back(m_spinDown);

        m_spinValueBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("5.00"), this, menu_selector(NeverloseMenu::onSpinValue));
        m_spinValueBtn->setPosition({cx + 110, cy + 70});
        menu->addChild(m_spinValueBtn); m_antiAimNodes.push_back(m_spinValueBtn);

        m_spinUp = CCMenuItemSpriteExtra::create(ButtonSprite::create(">"), this, menu_selector(NeverloseMenu::onSpinUp));
        m_spinUp->setPosition({cx + 190, cy + 70});
        menu->addChild(m_spinUp); m_antiAimNodes.push_back(m_spinUp);

        m_speedhackBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Speedhack: OFF"), this, menu_selector(NeverloseMenu::onSpeedhack));
        m_speedhackBtn->setPosition({cx + 60, cy});
        menu->addChild(m_speedhackBtn); m_antiAimNodes.push_back(m_speedhackBtn);

        auto shText = CCLabelBMFont::create("Speedhack Value:", "bigFont.fnt");
        shText->setPosition({cx - 60, cy - 70});
        shText->setScale(0.5f);
        shText->setColor({0, 200, 255});
        this->addChild(shText); m_antiAimNodes.push_back(shText);

        m_shDown = CCMenuItemSpriteExtra::create(ButtonSprite::create("<"), this, menu_selector(NeverloseMenu::onSpeedhackDown));
        m_shDown->setPosition({cx + 30, cy - 70});
        menu->addChild(m_shDown); m_antiAimNodes.push_back(m_shDown);

        m_shValueBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("1.000"), this, menu_selector(NeverloseMenu::onSpeedValue));
        m_shValueBtn->setPosition({cx + 110, cy - 70});
        menu->addChild(m_shValueBtn); m_antiAimNodes.push_back(m_shValueBtn);

        m_shUp = CCMenuItemSpriteExtra::create(ButtonSprite::create(">"), this, menu_selector(NeverloseMenu::onSpeedhackUp));
        m_shUp->setPosition({cx + 190, cy - 70});
        menu->addChild(m_shUp); m_antiAimNodes.push_back(m_shUp);

        // CREATE
        m_copyHackBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Copy Hack: OFF"), this, menu_selector(NeverloseMenu::onCopyHack));
        m_copyHackBtn->setPosition({cx + 60, cy + 100});
        menu->addChild(m_copyHackBtn); m_createNodes.push_back(m_copyHackBtn);

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
    void onTabCreate(CCObject*) { setPage(4); }

    void onJumpHack(CCObject*) { g_jumpHack = !g_jumpHack; refreshButtons(); }
    void onAutoClicker(CCObject*) { g_autoClicker = !g_autoClicker; refreshButtons(); }
    void onNoclip(CCObject*) { g_noclip = !g_noclip; refreshButtons(); }
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
        m_spinValueBtn->setNormalImage(ButtonSprite::create(CCString::createWithFormat("%.2f", g_spinSpeed)->getCString()));
    }
    void onSpinDown(CCObject*) {
        g_spinSpeed -= 1.0f;
        if (g_spinSpeed < 1.0f) g_spinSpeed = 1.0f;
        m_spinValueBtn->setNormalImage(ButtonSprite::create(CCString::createWithFormat("%.2f", g_spinSpeed)->getCString()));
    }
    void onSpinValue(CCObject*) {
        auto popup = ValueInputPopup::create(false);
        if (popup) CCDirector::get()->getRunningScene()->addChild(popup, 99999);
    }
    void onSpeedhack(CCObject*) {
        g_speedhack = !g_speedhack;
        if (g_speedhack) CCDirector::get()->getScheduler()->setTimeScale(g_speed);
        else CCDirector::get()->getScheduler()->setTimeScale(1.0f);
        refreshButtons();
    }
    void onSpeedhackUp(CCObject*) {
        g_speed += 5.0f;
        if (g_speed > 100.0f) g_speed = 100.0f;
        if (g_speedhack) CCDirector::get()->getScheduler()->setTimeScale(g_speed);
        m_shValueBtn->setNormalImage(ButtonSprite::create(CCString::createWithFormat("%.3f", g_speed)->getCString()));
    }
    void onSpeedhackDown(CCObject*) {
        g_speed -= 5.0f;
        if (g_speed < 0.0f) g_speed = 0.0f;
        if (g_speedhack) CCDirector::get()->getScheduler()->setTimeScale(g_speed);
        m_shValueBtn->setNormalImage(ButtonSprite::create(CCString::createWithFormat("%.3f", g_speed)->getCString()));
    }
    void onSpeedValue(CCObject*) {
        auto popup = ValueInputPopup::create(true);
        if (popup) CCDirector::get()->getRunningScene()->addChild(popup, 99999);
    }
    void onCopyHack(CCObject*) { g_copyHack = !g_copyHack; refreshButtons(); }
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
    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        if (g_noclip) return;
        PlayLayer::destroyPlayer(player, obj);
    }
    void update(float dt) {
        PlayLayer::update(dt);
        if (g_autoJump && m_player1) m_player1->pushButton(PlayerButton::Jump);
        if (g_jumpHack && m_player1) m_player1->m_yVelocity = 20.0f;
        if (g_autoClicker && m_player1) {
            g_clickTimer++;
            if (g_clickTimer % 4 == 0) m_player1->pushButton(PlayerButton::Jump);
            else if (g_clickTimer % 4 == 2) m_player1->releaseButton(PlayerButton::Jump);
            if (g_clickTimer > 1000) g_clickTimer = 0;
        }
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

// ================= COPY HACK =================
class $modify(MyLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* p0, bool p1) {
        if (!LevelInfoLayer::init(p0, p1)) return false;
        if (!g_copyHack) return true;
        if (auto menu = getChildByID("left-side-menu")) {
            if (auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(getChildBySpriteFrameName(menu, "GJ_duplicateLockedBtn_001.png"))) {
                if (btn->isVisible()) {
                    btn->m_pfnSelector = menu_selector(LevelInfoLayer::confirmClone);
                    btn->setSprite(CCSprite::createWithSpriteFrameName("GJ_duplicateBtn_001.png"));
                }
            } else if (auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(getChildBySpriteFrameName(menu, "GJ_duplicateBtn_001.png"))) {
                btn->setVisible(true);
            }
        }
        return true;
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
