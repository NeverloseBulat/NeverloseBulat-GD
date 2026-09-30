#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <cstdlib>
#include <ctime>

using namespace geode::prelude;

static bool g_noclip = false;
static bool g_autoJump = false;
static bool g_spinbot = false;
static bool g_ldm = false;
static bool g_autoLDM = false;
static bool g_speedhack = false;
static bool g_jumpHack = false;
static bool g_copyHack = false;
static bool g_autoSafeMode = false;
static bool g_aaEnabled = false;
static bool g_aaFlipX = false;
static bool g_aaFlipY = false;
static bool g_cheatIndicator = false;
static bool g_showCPS = false;
static bool g_showTime = false;
static bool g_showFPS = false;
static bool g_unlockIcons = false;
static bool g_unlockVault = false;
static bool g_unlockColors = false;
static bool g_unlockLevels = false;
static float g_speed = 1.0f;
static float g_spinSpeed = 5.0f;

class ValueInputPopup : public CCLayer {
protected:
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
        panel->setContentSize({520, 260});
        panel->setPosition(ws / 2);
        this->addChild(panel);
        auto title = CCLabelBMFont::create(m_isSpeed ? "Speedhack Value" : "Spinbot Speed", "goldFont.fnt");
        title->setPosition({ws.width / 2, ws.height / 2 + 90});
        title->setScale(0.7f);
        title->setColor({0, 200, 255});
        this->addChild(title);
        auto value = CCLabelBMFont::create(
            CCString::createWithFormat("%.2f", m_isSpeed ? g_speed : g_spinSpeed)->getCString(),
            "goldFont.fnt");
        value->setPosition({ws.width / 2, ws.height / 2 + 30});
        value->setScale(0.9f);
        value->setColor({255, 255, 255});
        value->setID("value-label");
        this->addChild(value);
        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        this->addChild(menu);
        float cx = ws.width / 2, cy = ws.height / 2;
        auto mkBtn = [&](const char* t, cocos2d::SEL_MenuHandler cb, float x, float y) {
            auto b = CCMenuItemSpriteExtra::create(ButtonSprite::create(t), this, cb);
            b->setPosition({x, y});
            menu->addChild(b);
        };
        mkBtn("-500", menu_selector(ValueInputPopup::onMinus500), cx - 200, cy - 30);
        mkBtn("-1",   menu_selector(ValueInputPopup::onMinus1),   cx - 70,  cy - 30);
        mkBtn("+1",   menu_selector(ValueInputPopup::onPlus1),    cx + 70,  cy - 30);
        mkBtn("+500", menu_selector(ValueInputPopup::onPlus500),  cx + 200, cy - 30);
        mkBtn("OK",   menu_selector(ValueInputPopup::onCancel),   cx,      cy - 110);
        return true;
    }
    void refreshValue() {
        auto lbl = this->getChildByID("value-label");
        if (lbl) {
            auto l = typeinfo_cast<CCLabelBMFont*>(lbl);
            if (l) l->setString(CCString::createWithFormat("%.2f", m_isSpeed ? g_speed : g_spinSpeed)->getCString());
        }
    }
    void applySpeed() { if (g_speedhack) CCDirector::get()->getScheduler()->setTimeScale(g_speed); }
    void onMinus500(CCObject*) {
        if (m_isSpeed) { g_speed -= 500.0f; if (g_speed < 0) g_speed = 0; applySpeed(); }
        else { g_spinSpeed -= 500.0f; if (g_spinSpeed < 1) g_spinSpeed = 1; }
        refreshValue();
    }
    void onMinus1(CCObject*) {
        if (m_isSpeed) { g_speed -= 1.0f; if (g_speed < 0) g_speed = 0; applySpeed(); }
        else { g_spinSpeed -= 1.0f; if (g_spinSpeed < 1) g_spinSpeed = 1; }
        refreshValue();
    }
    void onPlus1(CCObject*) {
        if (m_isSpeed) { g_speed += 1.0f; if (g_speed > 100) g_speed = 100; applySpeed(); }
        else { g_spinSpeed += 1.0f; if (g_spinSpeed > 500) g_spinSpeed = 500; }
        refreshValue();
    }
    void onPlus500(CCObject*) {
        if (m_isSpeed) { g_speed += 500.0f; if (g_speed > 100) g_speed = 100; applySpeed(); }
        else { g_spinSpeed += 500.0f; if (g_spinSpeed > 500) g_spinSpeed = 500; }
        refreshValue();
    }
    void onCancel(CCObject*) { this->removeFromParentAndCleanup(true); }
    void keyBackClicked() { this->removeFromParentAndCleanup(true); }
};

class NeverloseMenu : public CCLayer {
protected:
    std::vector<CCNode*> m_mainNodes, m_rageNodes, m_visualsNodes, m_antiAimNodes, m_createNodes, m_legitNodes, m_cosmeticsNodes;
    std::vector<CCMenuItemSpriteExtra*> m_tabButtons;
    CCMenuItemSpriteExtra *m_jumpHackBtn, *m_copyHackBtn, *m_autoSafeModeBtn;
    CCMenuItemSpriteExtra *m_cheatIndBtn, *m_cpsBtn, *m_timeBtn, *m_fpsBtn;
    CCMenuItemSpriteExtra *m_unlockIconsBtn, *m_unlockVaultBtn, *m_unlockColorsBtn, *m_unlockLevelsBtn;
    CCMenuItemSpriteExtra *m_noclipBtn, *m_autoJumpBtn;
    CCMenuItemSpriteExtra *m_ldmBtn, *m_autoLDMBtn;
    CCMenuItemSpriteExtra *m_spinbotBtn, *m_spinDown, *m_spinUp, *m_spinValueBtn;
    CCMenuItemSpriteExtra *m_speedhackBtn, *m_shDown, *m_shUp, *m_shValueBtn;
    CCMenuItemSpriteExtra *m_aaEnabledBtn, *m_aaFlipXBtn, *m_aaFlipYBtn;
    CCMenuItemSpriteExtra *m_tabMain, *m_tabRage, *m_tabVisuals, *m_tabAntiAim, *m_tabCreate, *m_tabLegit, *m_tabCosmetics;
    CCMenuItemSpriteExtra *m_scrollUpBtn, *m_scrollDownBtn;
    int m_tabOffset = 0;
    const int VISIBLE_TABS = 4;
public:
    static NeverloseMenu* create() {
        auto r = new NeverloseMenu();
        if (r && r->init()) { r->autorelease(); return r; }
        delete r; return nullptr;
    }
    void refreshButtons() {
        m_jumpHackBtn->setNormalImage(ButtonSprite::create(g_jumpHack ? "Jump Hack: ON" : "Jump Hack: OFF"));
        m_noclipBtn->setNormalImage(ButtonSprite::create(g_noclip ? "Noclip: ON" : "Noclip: OFF"));
        m_autoJumpBtn->setNormalImage(ButtonSprite::create(g_autoJump ? "AutoJump: ON" : "AutoJump: OFF"));
        m_ldmBtn->setNormalImage(ButtonSprite::create(g_ldm ? "LDM: ON" : "LDM: OFF"));
        m_autoLDMBtn->setNormalImage(ButtonSprite::create(g_autoLDM ? "Auto LDM: ON" : "Auto LDM: OFF"));
        m_spinbotBtn->setNormalImage(ButtonSprite::create(g_spinbot ? "Spinbot: ON" : "Spinbot: OFF"));
        m_speedhackBtn->setNormalImage(ButtonSprite::create(g_speedhack ? "Speedhack: ON" : "Speedhack: OFF"));
        m_copyHackBtn->setNormalImage(ButtonSprite::create(g_copyHack ? "Copy Hack: ON" : "Copy Hack: OFF"));
        m_autoSafeModeBtn->setNormalImage(ButtonSprite::create(g_autoSafeMode ? "Auto Safe Mode: ON" : "Auto Safe Mode: OFF"));
        m_aaEnabledBtn->setNormalImage(ButtonSprite::create(g_aaEnabled ? "Anti-Aim: ON" : "Anti-Aim: OFF"));
        m_aaFlipXBtn->setNormalImage(ButtonSprite::create(g_aaFlipX ? "Flip Back: ON" : "Flip Back: OFF"));
        m_aaFlipYBtn->setNormalImage(ButtonSprite::create(g_aaFlipY ? "Flip Down: ON" : "Flip Down: OFF"));
        m_cheatIndBtn->setNormalImage(ButtonSprite::create(g_cheatIndicator ? "Cheat Ind: ON" : "Cheat Ind: OFF"));
        m_cpsBtn->setNormalImage(ButtonSprite::create(g_showCPS ? "CPS: ON" : "CPS: OFF"));
        m_timeBtn->setNormalImage(ButtonSprite::create(g_showTime ? "Time: ON" : "Time: OFF"));
        m_fpsBtn->setNormalImage(ButtonSprite::create(g_showFPS ? "FPS: ON" : "FPS: OFF"));
        m_unlockIconsBtn->setNormalImage(ButtonSprite::create(g_unlockIcons ? "Icons: ON" : "Icons: OFF"));
        m_unlockVaultBtn->setNormalImage(ButtonSprite::create(g_unlockVault ? "Vault: ON" : "Vault: OFF"));
        m_unlockColorsBtn->setNormalImage(ButtonSprite::create(g_unlockColors ? "Colors: ON" : "Colors: OFF"));
        m_unlockLevelsBtn->setNormalImage(ButtonSprite::create(g_unlockLevels ? "Levels: ON" : "Levels: OFF"));
    }
    void updateTabVisibility() {
        for (int i = 0; i < (int)m_tabButtons.size(); i++) {
            bool visible = (i >= m_tabOffset && i < m_tabOffset + VISIBLE_TABS);
            m_tabButtons[i]->setVisible(visible);
        }
    }
    void setPage(int p) {
        if (p < 0) p = 0;
        if (p >= 7) p = 6;
        for (auto n : m_mainNodes) n->setVisible(p == 0);
        for (auto n : m_rageNodes) n->setVisible(p == 1);
        for (auto n : m_visualsNodes) n->setVisible(p == 2);
        for (auto n : m_antiAimNodes) n->setVisible(p == 3);
        for (auto n : m_createNodes) n->setVisible(p == 4);
        for (auto n : m_legitNodes) n->setVisible(p == 5);
        for (auto n : m_cosmeticsNodes) n->setVisible(p == 6);
        m_tabMain->setColor(p == 0 ? ccWHITE : ccGRAY);
        m_tabRage->setColor(p == 1 ? ccWHITE : ccGRAY);
        m_tabVisuals->setColor(p == 2 ? ccWHITE : ccGRAY);
        m_tabAntiAim->setColor(p == 3 ? ccWHITE : ccGRAY);
        m_tabCreate->setColor(p == 4 ? ccWHITE : ccGRAY);
        m_tabLegit->setColor(p == 5 ? ccWHITE : ccGRAY);
        m_tabCosmetics->setColor(p == 6 ? ccWHITE : ccGRAY);
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
        closeBtn->setPosition({cx + 340, cy - 220});
        menu->addChild(closeBtn);
        m_scrollUpBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("^"), this, menu_selector(NeverloseMenu::onScrollUp));
        m_scrollUpBtn->setPosition({cx - 270, cy + 195});
        menu->addChild(m_scrollUpBtn);
        m_scrollDownBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("v"), this, menu_selector(NeverloseMenu::onScrollDown));
        m_scrollDownBtn->setPosition({cx - 270, cy - 195});
        menu->addChild(m_scrollDownBtn);
        float tabY[7] = {cy + 140, cy + 70, cy, cy - 70, cy - 140, cy - 210, cy - 280};
        m_tabMain = CCMenuItemSpriteExtra::create(ButtonSprite::create("Main"), this, menu_selector(NeverloseMenu::onTabMain));
        m_tabMain->setPosition({cx - 270, tabY[0]});
        menu->addChild(m_tabMain); m_tabButtons.push_back(m_tabMain);
        m_tabRage = CCMenuItemSpriteExtra::create(ButtonSprite::create("Rage"), this, menu_selector(NeverloseMenu::onTabRage));
        m_tabRage->setPosition({cx - 270, tabY[1]});
        menu->addChild(m_tabRage); m_tabButtons.push_back(m_tabRage);
        m_tabVisuals = CCMenuItemSpriteExtra::create(ButtonSprite::create("Visuals"), this, menu_selector(NeverloseMenu::onTabVisuals));
        m_tabVisuals->setPosition({cx - 270, tabY[2]});
        menu->addChild(m_tabVisuals); m_tabButtons.push_back(m_tabVisuals);
        m_tabAntiAim = CCMenuItemSpriteExtra::create(ButtonSprite::create("Anti-Aim"), this, menu_selector(NeverloseMenu::onTabAntiAim));
        m_tabAntiAim->setPosition({cx - 270, tabY[3]});
        menu->addChild(m_tabAntiAim); m_tabButtons.push_back(m_tabAntiAim);
        m_tabCreate = CCMenuItemSpriteExtra::create(ButtonSprite::create("Create"), this, menu_selector(NeverloseMenu::onTabCreate));
        m_tabCreate->setPosition({cx - 270, tabY[4]});
        menu->addChild(m_tabCreate); m_tabButtons.push_back(m_tabCreate);
        m_tabLegit = CCMenuItemSpriteExtra::create(ButtonSprite::create("Legit"), this, menu_selector(NeverloseMenu::onTabLegit));
        m_tabLegit->setPosition({cx - 270, tabY[5]});
        menu->addChild(m_tabLegit); m_tabButtons.push_back(m_tabLegit);
        m_tabCosmetics = CCMenuItemSpriteExtra::create(ButtonSprite::create("Cosmetics"), this, menu_selector(NeverloseMenu::onTabCosmetics));
        m_tabCosmetics->setPosition({cx - 270, tabY[6]});
        menu->addChild(m_tabCosmetics); m_tabButtons.push_back(m_tabCosmetics);
        updateTabVisibility();
        m_jumpHackBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Jump Hack: OFF"), this, menu_selector(NeverloseMenu::onJumpHack));
        m_jumpHackBtn->setPosition({cx + 60, cy + 130});
        menu->addChild(m_jumpHackBtn); m_mainNodes.push_back(m_jumpHackBtn);
        m_noclipBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Noclip: OFF"), this, menu_selector(NeverloseMenu::onNoclip));
        m_noclipBtn->setPosition({cx + 60, cy + 60});
        menu->addChild(m_noclipBtn); m_rageNodes.push_back(m_noclipBtn);
        m_autoJumpBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("AutoJump: OFF"), this, menu_selector(NeverloseMenu::onAutoJump));
        m_autoJumpBtn->setPosition({cx + 60, cy - 10});
        menu->addChild(m_autoJumpBtn); m_rageNodes.push_back(m_autoJumpBtn);
        m_ldmBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("LDM: OFF"), this, menu_selector(NeverloseMenu::onLDM));
        m_ldmBtn->setPosition({cx + 60, cy + 90});
        menu->addChild(m_ldmBtn); m_visualsNodes.push_back(m_ldmBtn);
        m_autoLDMBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Auto LDM: OFF"), this, menu_selector(NeverloseMenu::onAutoLDM));
        m_autoLDMBtn->setPosition({cx + 60, cy + 25});
        menu->addChild(m_autoLDMBtn); m_visualsNodes.push_back(m_autoLDMBtn);
        m_spinbotBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Spinbot: OFF"), this, menu_selector(NeverloseMenu::onSpinbot));
        m_spinbotBtn->setPosition({cx + 60, cy + 160});
        menu->addChild(m_spinbotBtn); m_antiAimNodes.push_back(m_spinbotBtn);
        auto spinText = CCLabelBMFont::create("Spinbot Speed:", "bigFont.fnt");
        spinText->setPosition({cx - 60, cy + 100});
        spinText->setScale(0.5f);
        spinText->setColor({0, 200, 255});
        this->addChild(spinText); m_antiAimNodes.push_back(spinText);
        m_spinDown = CCMenuItemSpriteExtra::create(ButtonSprite::create("<"), this, menu_selector(NeverloseMenu::onSpinDown));
        m_spinDown->setPosition({cx + 30, cy + 100});
        menu->addChild(m_spinDown); m_antiAimNodes.push_back(m_spinDown);
        m_spinValueBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("5.00"), this, menu_selector(NeverloseMenu::onSpinValue));
        m_spinValueBtn->setPosition({cx + 110, cy + 100});
        menu->addChild(m_spinValueBtn); m_antiAimNodes.push_back(m_spinValueBtn);
        m_spinUp = CCMenuItemSpriteExtra::create(ButtonSprite::create(">"), this, menu_selector(NeverloseMenu::onSpinUp));
        m_spinUp->setPosition({cx + 190, cy + 100});
        menu->addChild(m_spinUp); m_antiAimNodes.push_back(m_spinUp);
        m_aaEnabledBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Anti-Aim: OFF"), this, menu_selector(NeverloseMenu::onAAEnabled));
        m_aaEnabledBtn->setPosition({cx + 60, cy + 20});
        menu->addChild(m_aaEnabledBtn); m_antiAimNodes.push_back(m_aaEnabledBtn);
        m_aaFlipXBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Flip Back: OFF"), this, menu_selector(NeverloseMenu::onAAFlipX));
        m_aaFlipXBtn->setPosition({cx + 60, cy - 50});
        menu->addChild(m_aaFlipXBtn); m_antiAimNodes.push_back(m_aaFlipXBtn);
        m_aaFlipYBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Flip Down: OFF"), this, menu_selector(NeverloseMenu::onAAFlipY));
        m_aaFlipYBtn->setPosition({cx + 60, cy - 120});
        menu->addChild(m_aaFlipYBtn); m_antiAimNodes.push_back(m_aaFlipYBtn);
        m_speedhackBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Speedhack: OFF"), this, menu_selector(NeverloseMenu::onSpeedhack));
        m_speedhackBtn->setPosition({cx + 60, cy - 190});
        menu->addChild(m_speedhackBtn); m_antiAimNodes.push_back(m_speedhackBtn);
        auto shText = CCLabelBMFont::create("Speedhack Value:", "bigFont.fnt");
        shText->setPosition({cx - 60, cy - 230});
        shText->setScale(0.5f);
        shText->setColor({0, 200, 255});
        this->addChild(shText); m_antiAimNodes.push_back(shText);
        m_shDown = CCMenuItemSpriteExtra::create(ButtonSprite::create("<"), this, menu_selector(NeverloseMenu::onSpeedhackDown));
        m_shDown->setPosition({cx + 30, cy - 230});
        menu->addChild(m_shDown); m_antiAimNodes.push_back(m_shDown);
        m_shValueBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("1.00"), this, menu_selector(NeverloseMenu::onSpeedValue));
        m_shValueBtn->setPosition({cx + 110, cy - 230});
        menu->addChild(m_shValueBtn); m_antiAimNodes.push_back(m_shValueBtn);
        m_shUp = CCMenuItemSpriteExtra::create(ButtonSprite::create(">"), this, menu_selector(NeverloseMenu::onSpeedhackUp));
        m_shUp->setPosition({cx + 190, cy - 230});
        menu->addChild(m_shUp); m_antiAimNodes.push_back(m_shUp);
        m_copyHackBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Copy Hack: OFF"), this, menu_selector(NeverloseMenu::onCopyHack));
        m_copyHackBtn->setPosition({cx + 60, cy + 100});
        menu->addChild(m_copyHackBtn); m_createNodes.push_back(m_copyHackBtn);
        m_autoSafeModeBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Auto Safe Mode: OFF"), this, menu_selector(NeverloseMenu::onAutoSafeMode));
        m_autoSafeModeBtn->setPosition({cx + 60, cy + 130});
        menu->addChild(m_autoSafeModeBtn); m_legitNodes.push_back(m_autoSafeModeBtn);
        m_cheatIndBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Cheat Ind: OFF"), this, menu_selector(NeverloseMenu::onCheatIndicator));
        m_cheatIndBtn->setPosition({cx + 60, cy + 60});
        menu->addChild(m_cheatIndBtn); m_legitNodes.push_back(m_cheatIndBtn);
        m_cpsBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("CPS: OFF"), this, menu_selector(NeverloseMenu::onCPS));
        m_cpsBtn->setPosition({cx + 60, cy - 10});
        menu->addChild(m_cpsBtn); m_legitNodes.push_back(m_cpsBtn);
        m_timeBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Time: OFF"), this, menu_selector(NeverloseMenu::onTime));
        m_timeBtn->setPosition({cx + 60, cy - 80});
        menu->addChild(m_timeBtn); m_legitNodes.push_back(m_timeBtn);
        m_fpsBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("FPS: OFF"), this, menu_selector(NeverloseMenu::onFPS));
        m_fpsBtn->setPosition({cx + 60, cy - 150});
        menu->addChild(m_fpsBtn); m_legitNodes.push_back(m_fpsBtn);
        m_unlockIconsBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Icons: OFF"), this, menu_selector(NeverloseMenu::onUnlockIcons));
        m_unlockIconsBtn->setPosition({cx + 60, cy + 130});
        menu->addChild(m_unlockIconsBtn); m_cosmeticsNodes.push_back(m_unlockIconsBtn);
        m_unlockVaultBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Vault: OFF"), this, menu_selector(NeverloseMenu::onUnlockVault));
        m_unlockVaultBtn->setPosition({cx + 60, cy + 60});
        menu->addChild(m_unlockVaultBtn); m_cosmeticsNodes.push_back(m_unlockVaultBtn);
        m_unlockColorsBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Colors: OFF"), this, menu_selector(NeverloseMenu::onUnlockColors));
        m_unlockColorsBtn->setPosition({cx + 60, cy - 10});
        menu->addChild(m_unlockColorsBtn); m_cosmeticsNodes.push_back(m_unlockColorsBtn);
        m_unlockLevelsBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Levels: OFF"), this, menu_selector(NeverloseMenu::onUnlockLevels));
        m_unlockLevelsBtn->setPosition({cx + 60, cy - 80});
        menu->addChild(m_unlockLevelsBtn); m_cosmeticsNodes.push_back(m_unlockLevelsBtn);
        setPage(0);
        refreshButtons();
        this->setScale(0.3f);
        this->runAction(CCEaseBackOut::create(CCScaleTo::create(0.35f, 1.0f)));
        this->setKeypadEnabled(true);
        return true;
    }
    void onScrollUp(CCObject*) { if (m_tabOffset > 0) { m_tabOffset--; updateTabVisibility(); } }
    void onScrollDown(CCObject*) { if (m_tabOffset < (int)m_tabButtons.size() - VISIBLE_TABS) { m_tabOffset++; updateTabVisibility(); } }
    void onTabMain(CCObject*) { setPage(0); }
    void onTabRage(CCObject*) { setPage(1); }
    void onTabVisuals(CCObject*) { setPage(2); }
    void onTabAntiAim(CCObject*) { setPage(3); }
    void onTabCreate(CCObject*) { setPage(4); }
    void onTabLegit(CCObject*) { setPage(5); }
    void onTabCosmetics(CCObject*) { setPage(6); }
    void onJumpHack(CCObject*) { g_jumpHack = !g_jumpHack; refreshButtons(); }
    void onNoclip(CCObject*) { g_noclip = !g_noclip; refreshButtons(); }
    void onAutoJump(CCObject*) { g_autoJump = !g_autoJump; refreshButtons(); }
    void onLDM(CCObject*) { g_ldm = !g_ldm; GameManager::get()->setGameVariable("low_detail_mode", g_ldm); refreshButtons(); }
    void onAutoLDM(CCObject*) { g_autoLDM = !g_autoLDM; GameManager::get()->setGameVariable("low_detail_mode", g_autoLDM); refreshButtons(); }
    void onSpinbot(CCObject*) { g_spinbot = !g_spinbot; refreshButtons(); }
    void onSpinUp(CCObject*) {
        g_spinSpeed += 1.0f;
        if (g_spinSpeed > 500.0f) g_spinSpeed = 500.0f;
        m_spinValueBtn->setNormalImage(ButtonSprite::create(CCString::createWithFormat("%.2f", g_spinSpeed)->getCString()));
    }
    void onSpinDown(CCObject*) {
        g_spinSpeed -= 1.0f;
        if (g_spinSpeed < 1.0f) g_spinSpeed = 1.0f;
        m_spinValueBtn->setNormalImage(ButtonSprite::create(CCString::createWithFormat("%.2f", g_spinSpeed)->getCString()));
    }
    void onSpinValue(CCObject*) {
        auto popup = ValueInputPopup::create(false);
        if (popup) this->addChild(popup, 9999);
    }
    void onAAEnabled(CCObject*) { g_aaEnabled = !g_aaEnabled; refreshButtons(); }
    void onAAFlipX(CCObject*) { g_aaFlipX = !g_aaFlipX; refreshButtons(); }
    void onAAFlipY(CCObject*) { g_aaFlipY = !g_aaFlipY; refreshButtons(); }
    void onSpeedhack(CCObject*) {
        g_speedhack = !g_speedhack;
        if (g_speedhack) CCDirector::get()->getScheduler()->setTimeScale(g_speed);
        else CCDirector::get()->getScheduler()->setTimeScale(1.0f);
        refreshButtons();
    }
    void onSpeedhackUp(CCObject*) {
        g_speed += 1.0f;
        if (g_speed > 100.0f) g_speed = 100.0f;
        if (g_speedhack) CCDirector::get()->getScheduler()->setTimeScale(g_speed);
        m_shValueBtn->setNormalImage(ButtonSprite::create(CCString::createWithFormat("%.2f", g_speed)->getCString()));
    }
    void onSpeedhackDown(CCObject*) {
        g_speed -= 1.0f;
        if (g_speed < 0.0f) g_speed = 0.0f;
        if (g_speedhack) CCDirector::get()->getScheduler()->setTimeScale(g_speed);
        m_shValueBtn->setNormalImage(ButtonSprite::create(CCString::createWithFormat("%.2f", g_speed)->getCString()));
    }
    void onSpeedValue(CCObject*) {
        auto popup = ValueInputPopup::create(true);
        if (popup) this->addChild(popup, 9999);
    }
    void onCopyHack(CCObject*) { g_copyHack = !g_copyHack; refreshButtons(); }
    void onAutoSafeMode(CCObject*) { g_autoSafeMode = !g_autoSafeMode; refreshButtons(); }
    void onCheatIndicator(CCObject*) { g_cheatIndicator = !g_cheatIndicator; refreshButtons(); }
    void onCPS(CCObject*) { g_showCPS = !g_showCPS; refreshButtons(); }
    void onTime(CCObject*) { g_showTime = !g_showTime; refreshButtons(); }
    void onFPS(CCObject*) { g_showFPS = !g_showFPS; refreshButtons(); }
    void onUnlockIcons(CCObject*) { g_unlockIcons = !g_unlockIcons; refreshButtons(); }
    void onUnlockVault(CCObject*) { g_unlockVault = !g_unlockVault; refreshButtons(); }
    void onUnlockColors(CCObject*) { g_unlockColors = !g_unlockColors; refreshButtons(); }
    void onUnlockLevels(CCObject*) { g_unlockLevels = !g_unlockLevels; refreshButtons(); }
    void onClose(CCObject*) {
        this->runAction(CCSequence::create(
            CCEaseBackIn::create(CCScaleTo::create(0.2f, 0.3f)),
            CCCallFunc::create(this, callfunc_selector(NeverloseMenu::removeMe)),
            nullptr));
    }
    void removeMe() { this->removeFromParentAndCleanup(true); }
    void keyBackClicked() { onClose(nullptr); }
};

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* m_cheatLbl = nullptr;
        CCLabelBMFont* m_cpsLbl = nullptr;
        CCLabelBMFont* m_timeLbl = nullptr;
        CCLabelBMFont* m_fpsLbl = nullptr;
        int m_frames = 0;
        float m_timeAcc = 0.0f;
        int m_clicks = 0;
    };
    bool init(GJGameLevel* p0, bool p1, bool p2) {
        if (!PlayLayer::init(p0, p1, p2)) return false;
        auto ws = CCDirector::get()->getWinSize();
        auto f = m_fields.self();
        f->m_cheatLbl = CCLabelBMFont::create("CHEAT", "bigFont.fnt");
        f->m_cheatLbl->setPosition({30, ws.height - 30});
        f->m_cheatLbl->setAnchorPoint({0, 1});
        f->m_cheatLbl->setScale(0.6f);
        f->m_cheatLbl->setColor({255, 0, 0});
        f->m_cheatLbl->setVisible(false);
        this->addChild(f->m_cheatLbl, 1000);
        f->m_cpsLbl = CCLabelBMFont::create("CPS: 0", "bigFont.fnt");
        f->m_cpsLbl->setPosition({30, ws.height - 60});
        f->m_cpsLbl->setAnchorPoint({0, 1});
        f->m_cpsLbl->setScale(0.5f);
        f->m_cpsLbl->setColor({0, 200, 255});
        f->m_cpsLbl->setVisible(false);
        this->addChild(f->m_cpsLbl, 1000);
        f->m_timeLbl = CCLabelBMFont::create("00:00", "bigFont.fnt");
        f->m_timeLbl->setPosition({30, ws.height - 90});
        f->m_timeLbl->setAnchorPoint({0, 1});
        f->m_timeLbl->setScale(0.5f);
        f->m_timeLbl->setColor({0, 200, 255});
        f->m_timeLbl->setVisible(false);
        this->addChild(f->m_timeLbl, 1000);
        f->m_fpsLbl = CCLabelBMFont::create("FPS: 0", "bigFont.fnt");
        f->m_fpsLbl->setPosition({30, ws.height - 120});
        f->m_fpsLbl->setAnchorPoint({0, 1});
        f->m_fpsLbl->setScale(0.5f);
        f->m_fpsLbl->setColor({0, 200, 255});
        f->m_fpsLbl->setVisible(false);
        this->addChild(f->m_fpsLbl, 1000);
        return true;
    }
    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        if (g_noclip) return;
        PlayLayer::destroyPlayer(player, obj);
    }
    void update(float dt) {
        PlayLayer::update(dt);
        if (g_autoJump && m_player1) m_player1->pushButton(PlayerButton::Jump);
        if (g_jumpHack && m_player1) m_player1->m_yVelocity = 20.0f;
        auto f = m_fields.self();
        if (f->m_cheatLbl) f->m_cheatLbl->setVisible(g_cheatIndicator);
        if (f->m_cpsLbl) f->m_cpsLbl->setVisible(g_showCPS);
        if (f->m_timeLbl) f->m_timeLbl->setVisible(g_showTime);
        if (f->m_fpsLbl) f->m_fpsLbl->setVisible(g_showFPS);
        f->m_frames++;
        f->m_timeAcc += dt;
        if (f->m_timeAcc >= 1.0f) {
            int fps = (int)(f->m_frames / f->m_timeAcc);
            if (f->m_fpsLbl) f->m_fpsLbl->setString(CCString::createWithFormat("FPS: %d", fps)->getCString());
            f->m_frames = 0;
            f->m_timeAcc = 0;
            if (f->m_cpsLbl) f->m_cpsLbl->setString(CCString::createWithFormat("CPS: %d", f->m_clicks)->getCString());
            f->m_clicks = 0;
        }
        if (f->m_timeLbl) {
            time_t now = time(nullptr);
            struct tm* t = localtime(&now);
            f->m_timeLbl->setString(CCString::createWithFormat("%02d:%02d", t->tm_hour, t->tm_min)->getCString());
        }
    }
    void onExit() {
        CCDirector::get()->getScheduler()->setTimeScale(1.0f);
        PlayLayer::onExit();
    }
};

class $modify(MyPlayer, PlayerObject) {
    void pushButton(PlayerButton btn) {
        if (btn == PlayerButton::Jump) {
            auto pl = PlayLayer::get();
            if (pl) {
                auto myPl = static_cast<MyPlayLayer*>(pl);
                if (myPl) myPl->m_fields->m_clicks++;
            }
        }
        PlayerObject::pushButton(btn);
    }
    void releaseButton(PlayerButton btn) {
        if (g_autoJump && btn == PlayerButton::Jump) return;
        PlayerObject::releaseButton(btn);
    }
    void update(float dt) {
        PlayerObject::update(dt);
        if (g_aaEnabled) {
            this->setFlipX(g_aaFlipX);
            this->setFlipY(g_aaFlipY);
        } else {
            this->setFlipX(false);
            this->setFlipY(false);
        }
        if (g_spinbot) this->setRotation(this->getRotation() + g_spinSpeed);
    }
};

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
    };
    void onNeverlose(CCObject*) {
        auto menu = NeverloseMenu::create();
        if (menu) this->addChild(menu, 200);
    }
};
