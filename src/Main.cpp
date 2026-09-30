#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>

#include <vector>
#include <cstdlib>
#include <ctime>

using namespace geode::prelude;

static bool g_noclip=false, g_autoJump=false, g_spinbot=false, g_shake=false,
            g_nlGravite=false, g_ldm=false, g_autoLDM=false, g_speedhack=false,
            g_jumpHack=false, g_copyHack=false, g_autoSafeMode=false,
            g_aaEnabled=false, g_aaFlipX=false, g_aaFlipY=false,
            g_cheatIndicator=false, g_showCPS=false, g_showTime=false,
            g_showFPS=false, g_unlockIcons=false, g_unlockVault=false,
            g_unlockColors=false, g_unlockLevels=false;

static float g_speed = 1.0f, g_spinSpeed = 5.0f, g_shakeTimer = 0.0f,
             g_spinPhase = 0.0f, g_gravTimer = 0.0f;
static bool  g_gravState = false;
static int   g_clickCount = 0;

static ButtonSprite* makeToggle(const char* on, const char* off, bool state) {
    return ButtonSprite::create(state ? on : off);
}
static ButtonSprite* makeNumber(float v) {
    return ButtonSprite::create(
        CCString::createWithFormat("%.2f", v)->getCString());
}

// ============================================================
//                    VALUE INPUT POPUP
// ============================================================
class ValueInputPopup : public CCLayer {
protected:
    bool m_isSpeed = true;
public:
    static ValueInputPopup* create(bool s) {
        auto r = new ValueInputPopup();
        if (!r) return nullptr;
        r->m_isSpeed = s;
        if (r->init()) { r->autorelease(); return r; }
        delete r; return nullptr;
    }

    bool init() override {
        if (!CCLayer::init()) return false;
        auto ws = CCDirector::get()->getWinSize();
        float cx = ws.width  / 2.f;
        float cy = ws.height / 2.f;

        auto bg = CCLayerColor::create({0,0,0,200});
        this->addChild(bg, -1);

        auto p = CCScale9Sprite::create("GJ_square01.png");
        p->setContentSize({520.f, 260.f});
        p->setPosition({cx, cy});
        this->addChild(p);

        auto t = CCLabelBMFont::create(
            m_isSpeed ? "Speedhack Value" : "Spinbot Speed", "goldFont.fnt");
        t->setPosition({cx, cy + 90.f});
        t->setScale(0.7f);
        t->setColor({0, 200, 255});
        this->addChild(t);

        auto v = CCLabelBMFont::create(
            CCString::createWithFormat("%.2f",
                m_isSpeed ? g_speed : g_spinSpeed)->getCString(),
            "goldFont.fnt");
        v->setPosition({cx, cy + 30.f});
        v->setScale(0.9f);
        v->setColor({255, 255, 255});
        v->setID("value-label");
        this->addChild(v);

        auto m = CCMenu::create();
        m->setPosition({0,0});
        this->addChild(m);

        auto mk = [&](const char* tt, cocos2d::SEL_MenuHandler cb, float x, float y){
            auto b = CCMenuItemSpriteExtra::create(ButtonSprite::create(tt), this, cb);
            b->setPosition({x, y});
            b->setScale(0.7f);
            m->addChild(b);
        };

        mk("-500", menu_selector(ValueInputPopup::onM500), cx - 200.f, cy - 30.f);
        mk("-1",   menu_selector(ValueInputPopup::onM1),   cx - 70.f,  cy - 30.f);
        mk("+1",   menu_selector(ValueInputPopup::onP1),   cx + 70.f,  cy - 30.f);
        mk("+500", menu_selector(ValueInputPopup::onP500), cx + 200.f, cy - 30.f);
        mk("OK",   menu_selector(ValueInputPopup::onCancel), cx, cy - 110.f);

        this->setTouchEnabled(true);
        this->setKeypadEnabled(true);
        return true;
    }

    void ref() {
        auto l = this->getChildByID("value-label");
        if (!l) return;
        if (auto x = typeinfo_cast<CCLabelBMFont*>(l)) {
            x->setString(CCString::createWithFormat("%.2f",
                m_isSpeed ? g_speed : g_spinSpeed)->getCString());
        }
    }
    void ap() {
        if (g_speedhack && CCDirector::get() && CCDirector::get()->getScheduler())
            CCDirector::get()->getScheduler()->setTimeScale(g_speed);
    }

    void onM500(CCObject*) {
        if (m_isSpeed) { g_speed -= 500.f; if (g_speed < 1.f) g_speed = 1.f; ap(); }
        else           { g_spinSpeed -= 500.f; if (g_spinSpeed < 1.f) g_spinSpeed = 1.f; }
        ref();
    }
    void onM1(CCObject*) {
        if (m_isSpeed) { g_speed -= 1.f; if (g_speed < 1.f) g_speed = 1.f; ap(); }
        else           { g_spinSpeed -= 1.f; if (g_spinSpeed < 1.f) g_spinSpeed = 1.f; }
        ref();
    }
    void onP1(CCObject*) {
        if (m_isSpeed) { g_speed += 1.f; if (g_speed > 500.f) g_speed = 500.f; ap(); }
        else           { g_spinSpeed += 1.f; if (g_spinSpeed > 500.f) g_spinSpeed = 500.f; }
        ref();
    }
    void onP500(CCObject*) {
        if (m_isSpeed) { g_speed += 500.f; if (g_speed > 500.f) g_speed = 500.f; ap(); }
        else           { g_spinSpeed += 500.f; if (g_spinSpeed > 500.f) g_spinSpeed = 500.f; }
        ref();
    }
    void onCancel(CCObject*) { this->removeFromParentAndCleanup(true); }
    void keyBackClicked()    { this->removeFromParentAndCleanup(true); }
};

// ============================================================
//                       NEVERLOSE MENU
// ============================================================
class NeverloseMenu : public CCLayer {
protected:
    std::vector<CCNode*> m_mainNodes, m_rageNodes, m_visualsNodes, m_antiAimNodes,
                         m_createNodes, m_legitNodes, m_cosmeticsNodes;

    CCMenuItemSpriteExtra *m_jumpHackBtn=nullptr, *m_copyHackBtn=nullptr,
        *m_autoSafeModeBtn=nullptr, *m_cheatIndBtn=nullptr, *m_cpsBtn=nullptr,
        *m_timeBtn=nullptr, *m_fpsBtn=nullptr, *m_unlockIconsBtn=nullptr,
        *m_unlockVaultBtn=nullptr, *m_unlockColorsBtn=nullptr, *m_unlockLevelsBtn=nullptr,
        *m_noclipBtn=nullptr, *m_autoJumpBtn=nullptr, *m_ldmBtn=nullptr,
        *m_autoLDMBtn=nullptr, *m_spinbotBtn=nullptr, *m_shakeBtn=nullptr,
        *m_nlGraviteBtn=nullptr, *m_spinDown=nullptr, *m_spinUp=nullptr,
        *m_spinValueBtn=nullptr, *m_speedhackBtn=nullptr, *m_shDown=nullptr,
        *m_shUp=nullptr, *m_shValueBtn=nullptr, *m_aaEnabledBtn=nullptr,
        *m_aaFlipXBtn=nullptr, *m_aaFlipYBtn=nullptr,
        *m_tabMain=nullptr, *m_tabRage=nullptr, *m_tabVisuals=nullptr,
        *m_tabAntiAim=nullptr, *m_tabCreate=nullptr, *m_tabLegit=nullptr,
        *m_tabCosmetics=nullptr;

    // --- drag ---
    bool  m_dragging   = false;
    CCPoint m_dragStart = {0,0};

public:
    static NeverloseMenu* create() {
        auto r = new NeverloseMenu();
        if (!r) return nullptr;
        if (r->init()) { r->autorelease(); return r; }
        delete r; return nullptr;
    }

    void refreshButtons() {
        if (m_jumpHackBtn)     m_jumpHackBtn->setNormalImage(makeToggle("Jump Hack: ON","Jump Hack: OFF", g_jumpHack));
        if (m_noclipBtn)       m_noclipBtn->setNormalImage(makeToggle("Noclip: ON","Noclip: OFF", g_noclip));
        if (m_autoJumpBtn)     m_autoJumpBtn->setNormalImage(makeToggle("AutoJump: ON","AutoJump: OFF", g_autoJump));
        if (m_ldmBtn)          m_ldmBtn->setNormalImage(makeToggle("LDM: ON","LDM: OFF", g_ldm));
        if (m_autoLDMBtn)      m_autoLDMBtn->setNormalImage(makeToggle("Auto LDM: ON","Auto LDM: OFF", g_autoLDM));
        if (m_spinbotBtn)      m_spinbotBtn->setNormalImage(makeToggle("Spinbot: ON","Spinbot: OFF", g_spinbot));
        if (m_shakeBtn)        m_shakeBtn->setNormalImage(makeToggle("Shake: ON","Shake: OFF", g_shake));
        if (m_nlGraviteBtn)    m_nlGraviteBtn->setNormalImage(makeToggle("NL.exe Gravite: ON","NL.exe Gravite: OFF", g_nlGravite));
        if (m_speedhackBtn)    m_speedhackBtn->setNormalImage(makeToggle("Speedhack: ON","Speedhack: OFF", g_speedhack));
        if (m_copyHackBtn)     m_copyHackBtn->setNormalImage(makeToggle("Copy Hack: ON","Copy Hack: OFF", g_copyHack));
        if (m_autoSafeModeBtn) m_autoSafeModeBtn->setNormalImage(makeToggle("Auto Safe Mode: ON","Auto Safe Mode: OFF", g_autoSafeMode));
        if (m_aaEnabledBtn)    m_aaEnabledBtn->setNormalImage(makeToggle("Anti-Aim: ON","Anti-Aim: OFF", g_aaEnabled));
        if (m_aaFlipXBtn)      m_aaFlipXBtn->setNormalImage(makeToggle("Flip Back: ON","Flip Back: OFF", g_aaFlipX));
        if (m_aaFlipYBtn)      m_aaFlipYBtn->setNormalImage(makeToggle("Flip Down: ON","Flip Down: OFF", g_aaFlipY));
        if (m_cheatIndBtn)     m_cheatIndBtn->setNormalImage(makeToggle("Cheat Ind: ON","Cheat Ind: OFF", g_cheatIndicator));
        if (m_cpsBtn)          m_cpsBtn->setNormalImage(makeToggle("CPS: ON","CPS: OFF", g_showCPS));
        if (m_timeBtn)         m_timeBtn->setNormalImage(makeToggle("Time: ON","Time: OFF", g_showTime));
        if (m_fpsBtn)          m_fpsBtn->setNormalImage(makeToggle("FPS: ON","FPS: OFF", g_showFPS));
        if (m_unlockIconsBtn)  m_unlockIconsBtn->setNormalImage(makeToggle("Icons: ON","Icons: OFF", g_unlockIcons));
        if (m_unlockVaultBtn)  m_unlockVaultBtn->setNormalImage(makeToggle("Vault: ON","Vault: OFF", g_unlockVault));
        if (m_unlockColorsBtn) m_unlockColorsBtn->setNormalImage(makeToggle("Colors: ON","Colors: OFF", g_unlockColors));
        if (m_unlockLevelsBtn) m_unlockLevelsBtn->setNormalImage(makeToggle("Levels: ON","Levels: OFF", g_unlockLevels));

        if (m_spinValueBtn) m_spinValueBtn->setNormalImage(makeNumber(g_spinSpeed));
        if (m_shValueBtn)   m_shValueBtn->setNormalImage(makeNumber(g_speed));
    }

    void setPage(int p) {
        if (p < 0) p = 0;
        if (p > 6) p = 6;

        for (auto n : m_mainNodes)      if (n) n->setVisible(p == 0);
        for (auto n : m_rageNodes)      if (n) n->setVisible(p == 1);
        for (auto n : m_visualsNodes)   if (n) n->setVisible(p == 2);
        for (auto n : m_antiAimNodes)   if (n) n->setVisible(p == 3);
        for (auto n : m_createNodes)    if (n) n->setVisible(p == 4);
        for (auto n : m_legitNodes)     if (n) n->setVisible(p == 5);
        for (auto n : m_cosmeticsNodes) if (n) n->setVisible(p == 6);

        if (m_tabMain)      m_tabMain->setColor(p == 0 ? ccWHITE : ccGRAY);
        if (m_tabRage)      m_tabRage->setColor(p == 1 ? ccWHITE : ccGRAY);
        if (m_tabVisuals)   m_tabVisuals->setColor(p == 2 ? ccWHITE : ccGRAY);
        if (m_tabAntiAim)   m_tabAntiAim->setColor(p == 3 ? ccWHITE : ccGRAY);
        if (m_tabCreate)    m_tabCreate->setColor(p == 4 ? ccWHITE : ccGRAY);
        if (m_tabLegit)     m_tabLegit->setColor(p == 5 ? ccWHITE : ccGRAY);
        if (m_tabCosmetics) m_tabCosmetics->setColor(p == 6 ? ccWHITE : ccGRAY);
    }

    bool init() override {
        if (!CCLayer::init()) return false;

        auto ws = CCDirector::get()->getWinSize();
        float cx = ws.width  / 2.f;
        float cy = ws.height / 2.f;

        auto ov = CCLayerColor::create({0,0,0,180});
        this->addChild(ov, -1);

        auto pn = CCScale9Sprite::create("GJ_square01.png");
        pn->setContentSize({780.f, 520.f});
        pn->setPosition({cx, cy});
        pn->setColor({15,15,15});
        this->addChild(pn);

        auto sb = CCScale9Sprite::create("GJ_square01.png");
        sb->setContentSize({220.f, 500.f});
        sb->setPosition({cx - 270.f, cy});
        sb->setColor({25,25,30});
        this->addChild(sb);

        auto lg = CCLabelBMFont::create("NEVERLOSE", "goldFont.fnt");
        lg->setPosition({cx - 270.f, cy + 215.f});
        lg->setScale(0.65f);
        lg->setColor({255,255,255});
        this->addChild(lg);

        auto mn = CCMenu::create();
        mn->setPosition({0,0});
        this->addChild(mn);

        auto cs = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        if (cs) cs->setScale(1.2f);
        auto cb = CCMenuItemSpriteExtra::create(cs, this, menu_selector(NeverloseMenu::onClose));
        cb->setPosition({cx + 360.f, cy + 235.f});
        mn->addChild(cb);

        float ty[7] = {cy + 105.f, cy + 70.f, cy + 35.f, cy, cy - 35.f, cy - 70.f, cy - 105.f};
        auto mkTab = [&](const char* t, cocos2d::SEL_MenuHandler cb_, float y) {
            auto b = CCMenuItemSpriteExtra::create(ButtonSprite::create(t), this, cb_);
            b->setPosition({cx - 270.f, y});
            b->setScale(0.5f);
            mn->addChild(b);
            return b;
        };
        m_tabMain      = mkTab("Main",      menu_selector(NeverloseMenu::onTabMain),      ty[0]);
        m_tabRage      = mkTab("Rage",      menu_selector(NeverloseMenu::onTabRage),      ty[1]);
        m_tabVisuals   = mkTab("Visuals",   menu_selector(NeverloseMenu::onTabVisuals),   ty[2]);
        m_tabAntiAim   = mkTab("Anti-Aim",  menu_selector(NeverloseMenu::onTabAntiAim),   ty[3]);
        m_tabCreate    = mkTab("Create",    menu_selector(NeverloseMenu::onTabCreate),    ty[4]);
        m_tabLegit     = mkTab("Legit",     menu_selector(NeverloseMenu::onTabLegit),     ty[5]);
        m_tabCosmetics = mkTab("Cosmetics", menu_selector(NeverloseMenu::onTabCosmetics), ty[6]);

        // MAIN
        m_jumpHackBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Jump Hack: OFF"), this, menu_selector(NeverloseMenu::onJumpHack));
        m_jumpHackBtn->setPosition({cx + 60.f, cy + 130.f});
        m_jumpHackBtn->setScale(0.55f);
        mn->addChild(m_jumpHackBtn);
        m_mainNodes.push_back(m_jumpHackBtn);

        // RAGE
        m_noclipBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Noclip: OFF"), this, menu_selector(NeverloseMenu::onNoclip));
        m_noclipBtn->setPosition({cx + 60.f, cy + 130.f});
        m_noclipBtn->setScale(0.55f);
        mn->addChild(m_noclipBtn);
        m_rageNodes.push_back(m_noclipBtn);

        m_autoJumpBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("AutoJump: OFF"), this, menu_selector(NeverloseMenu::onAutoJump));
        m_autoJumpBtn->setPosition({cx + 60.f, cy + 60.f});
        m_autoJumpBtn->setScale(0.55f);
        mn->addChild(m_autoJumpBtn);
        m_rageNodes.push_back(m_autoJumpBtn);

        // VISUALS
        m_ldmBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("LDM: OFF"), this, menu_selector(NeverloseMenu::onLDM));
        m_ldmBtn->setPosition({cx + 60.f, cy + 90.f});
        m_ldmBtn->setScale(0.55f);
        mn->addChild(m_ldmBtn);
        m_visualsNodes.push_back(m_ldmBtn);

        m_autoLDMBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Auto LDM: OFF"), this, menu_selector(NeverloseMenu::onAutoLDM));
        m_autoLDMBtn->setPosition({cx + 60.f, cy + 25.f});
        m_autoLDMBtn->setScale(0.55f);
        mn->addChild(m_autoLDMBtn);
        m_visualsNodes.push_back(m_autoLDMBtn);

        // ANTI-AIM
        float lx = cx - 150.f;
        float rx = cx + 150.f;

        auto mkArrow = [&](const char* txt, cocos2d::SEL_MenuHandler cb_, CCPoint pos) {
            auto b = CCMenuItemSpriteExtra::create(
                ButtonSprite::create(txt), this, cb_);
            b->setPosition(pos);
            b->setScale(0.9f);
            mn->addChild(b);
            return b;
        };

        m_spinbotBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Spinbot: OFF"), this, menu_selector(NeverloseMenu::onSpinbot));
        m_spinbotBtn->setPosition({lx, cy + 150.f});
        m_spinbotBtn->setScale(0.55f);
        mn->addChild(m_spinbotBtn);
        m_antiAimNodes.push_back(m_spinbotBtn);

        auto st = CCLabelBMFont::create("Spinbot Speed", "goldFont.fnt");
        st->setPosition({lx, cy + 105.f});
        st->setScale(0.55f);
        st->setColor({0, 200, 255});
        this->addChild(st);
        m_antiAimNodes.push_back(st);

        m_spinDown = mkArrow("-", menu_selector(NeverloseMenu::onSpinDown), {lx - 70.f, cy + 65.f});
        m_antiAimNodes.push_back(m_spinDown);

        m_spinValueBtn = CCMenuItemSpriteExtra::create(
            makeNumber(g_spinSpeed), this, menu_selector(NeverloseMenu::onSpinValue));
        m_spinValueBtn->setPosition({lx, cy + 65.f});
        mn->addChild(m_spinValueBtn);
        m_antiAimNodes.push_back(m_spinValueBtn);

        m_spinUp = mkArrow("+", menu_selector(NeverloseMenu::onSpinUp), {lx + 70.f, cy + 65.f});
        m_antiAimNodes.push_back(m_spinUp);

        m_aaEnabledBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Anti-Aim: OFF"), this, menu_selector(NeverloseMenu::onAAEnabled));
        m_aaEnabledBtn->setPosition({lx, cy + 5.f});
        m_aaEnabledBtn->setScale(0.55f);
        mn->addChild(m_aaEnabledBtn);
        m_antiAimNodes.push_back(m_aaEnabledBtn);

        m_aaFlipXBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Flip Back: OFF"), this, menu_selector(NeverloseMenu::onAAFlipX));
        m_aaFlipXBtn->setPosition({lx, cy - 45.f});
        m_aaFlipXBtn->setScale(0.55f);
        mn->addChild(m_aaFlipXBtn);
        m_antiAimNodes.push_back(m_aaFlipXBtn);

        m_aaFlipYBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Flip Down: OFF"), this, menu_selector(NeverloseMenu::onAAFlipY));
        m_aaFlipYBtn->setPosition({lx, cy - 95.f});
        m_aaFlipYBtn->setScale(0.55f);
        mn->addChild(m_aaFlipYBtn);
        m_antiAimNodes.push_back(m_aaFlipYBtn);

        m_shakeBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Shake: OFF"), this, menu_selector(NeverloseMenu::onShake));
        m_shakeBtn->setPosition({rx, cy + 150.f});
        m_shakeBtn->setScale(0.55f);
        mn->addChild(m_shakeBtn);
        m_antiAimNodes.push_back(m_shakeBtn);

        m_nlGraviteBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("NL.exe Gravite: OFF"), this, menu_selector(NeverloseMenu::onNLGravite));
        m_nlGraviteBtn->setPosition({rx, cy + 95.f});
        m_nlGraviteBtn->setScale(0.55f);
        mn->addChild(m_nlGraviteBtn);
        m_antiAimNodes.push_back(m_nlGraviteBtn);

        m_speedhackBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Speedhack: OFF"), this, menu_selector(NeverloseMenu::onSpeedhack));
        m_speedhackBtn->setPosition({rx, cy + 40.f});
        m_speedhackBtn->setScale(0.55f);
        mn->addChild(m_speedhackBtn);
        m_antiAimNodes.push_back(m_speedhackBtn);

        auto sht = CCLabelBMFont::create("Speedhack Value", "goldFont.fnt");
        sht->setPosition({rx, cy - 10.f});
        sht->setScale(0.55f);
        sht->setColor({0, 200, 255});
        this->addChild(sht);
        m_antiAimNodes.push_back(sht);

        m_shDown = mkArrow("-", menu_selector(NeverloseMenu::onSpeedhackDown), {rx - 70.f, cy - 55.f});
        m_antiAimNodes.push_back(m_shDown);

        m_shValueBtn = CCMenuItemSpriteExtra::create(
            makeNumber(g_speed), this, menu_selector(NeverloseMenu::onSpeedValue));
        m_shValueBtn->setPosition({rx, cy - 55.f});
        mn->addChild(m_shValueBtn);
        m_antiAimNodes.push_back(m_shValueBtn);

        m_shUp = mkArrow("+", menu_selector(NeverloseMenu::onSpeedhackUp), {rx + 70.f, cy - 55.f});
        m_antiAimNodes.push_back(m_shUp);

        // CREATE
        m_copyHackBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Copy Hack: OFF"), this, menu_selector(NeverloseMenu::onCopyHack));
        m_copyHackBtn->setPosition({cx + 60.f, cy + 100.f});
        m_copyHackBtn->setScale(0.55f);
        mn->addChild(m_copyHackBtn);
        m_createNodes.push_back(m_copyHackBtn);

        // LEGIT
        m_autoSafeModeBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Auto Safe Mode: OFF"), this, menu_selector(NeverloseMenu::onAutoSafeMode));
        m_autoSafeModeBtn->setPosition({cx + 60.f, cy + 130.f});
        m_autoSafeModeBtn->setScale(0.55f);
        mn->addChild(m_autoSafeModeBtn);
        m_legitNodes.push_back(m_autoSafeModeBtn);

        m_cheatIndBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Cheat Ind: OFF"), this, menu_selector(NeverloseMenu::onCheatIndicator));
        m_cheatIndBtn->setPosition({cx + 60.f, cy + 60.f});
        m_cheatIndBtn->setScale(0.55f);
        mn->addChild(m_cheatIndBtn);
        m_legitNodes.push_back(m_cheatIndBtn);

        m_cpsBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("CPS: OFF"), this, menu_selector(NeverloseMenu::onCPS));
        m_cpsBtn->setPosition({cx + 60.f, cy - 10.f});
        m_cpsBtn->setScale(0.55f);
        mn->addChild(m_cpsBtn);
        m_legitNodes.push_back(m_cpsBtn);

        m_timeBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Time: OFF"), this, menu_selector(NeverloseMenu::onTime));
        m_timeBtn->setPosition({cx + 60.f, cy - 80.f});
        m_timeBtn->setScale(0.55f);
        mn->addChild(m_timeBtn);
        m_legitNodes.push_back(m_timeBtn);

        m_fpsBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("FPS: OFF"), this, menu_selector(NeverloseMenu::onFPS));
        m_fpsBtn->setPosition({cx + 60.f, cy - 150.f});
        m_fpsBtn->setScale(0.55f);
        mn->addChild(m_fpsBtn);
        m_legitNodes.push_back(m_fpsBtn);

        // COSMETICS
        m_unlockIconsBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Icons: OFF"), this, menu_selector(NeverloseMenu::onUnlockIcons));
        m_unlockIconsBtn->setPosition({cx + 60.f, cy + 130.f});
        m_unlockIconsBtn->setScale(0.55f);
        mn->addChild(m_unlockIconsBtn);
        m_cosmeticsNodes.push_back(m_unlockIconsBtn);

        m_unlockVaultBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Vault: OFF"), this, menu_selector(NeverloseMenu::onUnlockVault));
        m_unlockVaultBtn->setPosition({cx + 60.f, cy + 60.f});
        m_unlockVaultBtn->setScale(0.55f);
        mn->addChild(m_unlockVaultBtn);
        m_cosmeticsNodes.push_back(m_unlockVaultBtn);

        m_unlockColorsBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Colors: OFF"), this, menu_selector(NeverloseMenu::onUnlockColors));
        m_unlockColorsBtn->setPosition({cx + 60.f, cy - 10.f});
        m_unlockColorsBtn->setScale(0.55f);
        mn->addChild(m_unlockColorsBtn);
        m_cosmeticsNodes.push_back(m_unlockColorsBtn);

        m_unlockLevelsBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Levels: OFF"), this, menu_selector(NeverloseMenu::onUnlockLevels));
        m_unlockLevelsBtn->setPosition({cx + 60.f, cy - 80.f});
        m_unlockLevelsBtn->setScale(0.55f);
        mn->addChild(m_unlockLevelsBtn);
        m_cosmeticsNodes.push_back(m_unlockLevelsBtn);

        setPage(0);
        refreshButtons();

        this->setPosition({0.f, 0.f});

        this->setScale(0.3f);
        this->runAction(CCEaseBackOut::create(CCScaleTo::create(0.35f, 1.0f)));
        this->setKeypadEnabled(true);
        this->setTouchEnabled(true);
        return true;
    }

    // ============================================
    //             DRAG HANDLING
    // ============================================
    void registerWithTouchDispatcher() override {
        CCDirector::get()->getTouchDispatcher()->addTargetedDelegate(this, -501, true);
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent*) override {
        auto worldPos = touch->getLocation();
        auto nodePos  = this->convertToNodeSpace(worldPos);
        if (nodePos.x < -400.f || nodePos.x > 400.f ||
            nodePos.y < -270.f || nodePos.y > 270.f) return false;

        m_dragging  = true;
        m_dragStart = worldPos - this->getPosition();
        return true;
    }

    void ccTouchMoved(CCTouch* touch, CCEvent*) override {
        if (!m_dragging) return;
        auto worldPos = touch->getLocation();
        auto newPos   = worldPos - m_dragStart;
        this->setPosition(newPos);
    }

    void ccTouchEnded(CCTouch*, CCEvent*) override {
        m_dragging = false;
    }

    void ccTouchCancelled(CCTouch*, CCEvent*) override {
        m_dragging = false;
    }

    // ============================================
    //                CALLBACKS
    // ============================================
    void onTabMain(CCObject*)      { setPage(0); }
    void onTabRage(CCObject*)      { setPage(1); }
    void onTabVisuals(CCObject*)   { setPage(2); }
    void onTabAntiAim(CCObject*)   { setPage(3); }
    void onTabCreate(CCObject*)    { setPage(4); }
    void onTabLegit(CCObject*)     { setPage(5); }
    void onTabCosmetics(CCObject*) { setPage(6); }

    void onJumpHack(CCObject*) { g_jumpHack = !g_jumpHack; refreshButtons(); }
    void onNoclip(CCObject*)   { g_noclip   = !g_noclip;   refreshButtons(); }
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

    void onSpinbot(CCObject*) {
        g_spinbot = !g_spinbot;
        if (g_spinbot) { g_shake = false; g_nlGravite = false; }
        refreshButtons();
    }
    void onShake(CCObject*) {
        g_shake = !g_shake; g_shakeTimer = 0.f;
        if (g_shake) { g_spinbot = false; g_nlGravite = false; }
        refreshButtons();
    }
    void onNLGravite(CCObject*) {
        g_nlGravite = !g_nlGravite;
        if (g_nlGravite) { g_spinbot = false; g_shake = false; }
        g_gravTimer = 0.f;
        refreshButtons();
    }

    void onSpinUp(CCObject*) {
        g_spinSpeed += 1.f; if (g_spinSpeed > 500.f) g_spinSpeed = 500.f;
        if (m_spinValueBtn) m_spinValueBtn->setNormalImage(makeNumber(g_spinSpeed));
    }
    void onSpinDown(CCObject*) {
        g_spinSpeed -= 1.f; if (g_spinSpeed < 1.f) g_spinSpeed = 1.f;
        if (m_spinValueBtn) m_spinValueBtn->setNormalImage(makeNumber(g_spinSpeed));
    }
    void onSpinValue(CCObject*) {
        if (auto p = ValueInputPopup::create(false)) this->addChild(p, 9);
    }
    void onSpeedhackUp(CCObject*) {
        g_speed += 1.f; if (g_speed > 500.f) g_speed = 500.f;
        if (g_speedhack && CCDirector::get() && CCDirector::get()->getScheduler())
            CCDirector::get()->getScheduler()->setTimeScale(g_speed);
        if (m_shValueBtn) m_shValueBtn->setNormalImage(makeNumber(g_speed));
    }
    void onSpeedhackDown(CCObject*) {
        g_speed -= 1.f; if (g_speed < 1.f) g_speed = 1.f;
        if (g_speedhack && CCDirector::get() && CCDirector::get()->getScheduler())
            CCDirector::get()->getScheduler()->setTimeScale(g_speed);
        if (m_shValueBtn) m_shValueBtn->setNormalImage(makeNumber(g_speed));
    }
    void onSpeedValue(CCObject*) {
        if (auto p = ValueInputPopup::create(true)) this->addChild(p, 9);
    }
    void onSpeedhack(CCObject*) {
        g_speedhack = !g_speedhack;
        if (CCDirector::get() && CCDirector::get()->getScheduler())
            CCDirector::get()->getScheduler()->setTimeScale(g_speedhack ? g_speed : 1.0f);
        refreshButtons();
    }

    void onCopyHack(CCObject*)       { g_copyHack       = !g_copyHack;       refreshButtons(); }
    void onAutoSafeMode(CCObject*)   { g_autoSafeMode   = !g_autoSafeMode;   refreshButtons(); }
    void onCheatIndicator(CCObject*) { g_cheatIndicator = !g_cheatIndicator; refreshButtons(); }
    void onCPS(CCObject*)            { g_showCPS        = !g_showCPS;        refreshButtons(); }
    void onTime(CCObject*)           { g_showTime       = !g_showTime;       refreshButtons(); }
    void onFPS(CCObject*)            { g_showFPS        = !g_showFPS;        refreshButtons(); }
    void onUnlockIcons(CCObject*)    { g_unlockIcons    = !g_unlockIcons;    refreshButtons(); }
    void onUnlockVault(CCObject*)    { g_unlockVault    = !g_unlockVault;    refreshButtons(); }
    void onUnlockColors(CCObject*)   { g_unlockColors   = !g_unlockColors;   refreshButtons(); }
    void onUnlockLevels(CCObject*)   { g_unlockLevels   = !g_unlockLevels;   refreshButtons(); }
    void onAAEnabled(CCObject*)      { g_aaEnabled      = !g_aaEnabled;      refreshButtons(); }
    void onAAFlipX(CCObject*)        { g_aaFlipX        = !g_aaFlipX;        refreshButtons(); }
    void onAAFlipY(CCObject*)        { g_aaFlipY        = !g_aaFlipY;        refreshButtons(); }

    void onClose(CCObject*) { this->removeFromParentAndCleanup(true); }
    void keyBackClicked()   { this->removeFromParentAndCleanup(true); }
};

// ============================================================
//                     PLAYLAYER HOOK
// ============================================================
class $modify(NLPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* cpsLabel   = nullptr;
        CCLabelBMFont* timeLabel  = nullptr;
        CCLabelBMFont* fpsLabel   = nullptr;
        CCLabelBMFont* cheatLabel = nullptr;
        float timeAlive  = 0.0f;
        int   frameCount = 0;
        float fpsTimer   = 0.0f;
        float cpsTimer   = 0.0f;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        auto ws = CCDirector::get()->getWinSize();
        auto f = m_fields.self();
        if (!f) return true;

        if (g_showCPS) {
            f->cpsLabel = CCLabelBMFont::create("CPS: 0", "bigFont.fnt");
            if (f->cpsLabel) {
                f->cpsLabel->setScale(0.4f);
                f->cpsLabel->setPosition({60.f, ws.height - 20.f});
                f->cpsLabel->setColor({0,255,255});
                this->addChild(f->cpsLabel, 100);
            }
        }
        if (g_showTime) {
            f->timeLabel = CCLabelBMFont::create("Time: 0.00", "bigFont.fnt");
            if (f->timeLabel) {
                f->timeLabel->setScale(0.4f);
                f->timeLabel->setPosition({60.f, ws.height - 45.f});
                f->timeLabel->setColor({255,255,0});
                this->addChild(f->timeLabel, 100);
            }
        }
        if (g_showFPS) {
            f->fpsLabel = CCLabelBMFont::create("FPS: 60", "bigFont.fnt");
            if (f->fpsLabel) {
                f->fpsLabel->setScale(0.4f);
                f->fpsLabel->setPosition({60.f, ws.height - 70.f});
                f->fpsLabel->setColor({255,100,255});
                this->addChild(f->fpsLabel, 100);
            }
        }
        if (g_cheatIndicator) {
            f->cheatLabel = CCLabelBMFont::create("[NL.exe]", "bigFont.fnt");
            if (f->cheatLabel) {
                f->cheatLabel->setScale(0.5f);
                f->cheatLabel->setPosition({ws.width - 60.f, ws.height - 20.f});
                f->cheatLabel->setColor({255,50,50});
                this->addChild(f->cheatLabel, 100);
            }
        }
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        auto f = m_fields.self();
        if (!f) return;

        if (g_autoSafeMode) GameManager::get()->setGameVariable("0021", true);
        if (g_autoLDM)      GameManager::get()->setGameVariable("low_detail_mode", true);

        auto player = m_player1;

        if (g_shake) {
            g_shakeTimer += dt;
            if (m_objectLayer) {
                float offX = ((rand() % 100) - 50) / 50.0f * 5.0f;
                float offY = ((rand() % 100) - 50) / 50.0f * 5.0f;
                m_objectLayer->setPosition({offX, offY});
            }
        } else if (m_objectLayer) {
            m_objectLayer->setPosition({0.f, 0.f});
        }

        if (g_nlGravite && player) {
            g_gravTimer += dt;
            if (g_gravTimer >= 0.15f) {
                g_gravTimer = 0.f;
                g_gravState = !g_gravState;
                player->flipGravity(g_gravState, false);
            }
        }

        f->timeAlive += dt;
        f->frameCount++;
        f->fpsTimer += dt;
        if (f->fpsTimer >= 0.5f) {
            float fps = f->frameCount / f->fpsTimer;
            if (f->fpsLabel)
                f->fpsLabel->setString(CCString::createWithFormat("FPS: %.0f", fps)->getCString());
            f->frameCount = 0;
            f->fpsTimer = 0.0f;
        }
        if (f->timeLabel)
            f->timeLabel->setString(CCString::createWithFormat("Time: %.2f", f->timeAlive)->getCString());
        if (f->cpsLabel) {
            f->cpsTimer += dt;
            if (f->cpsTimer >= 1.0f) {
                f->cpsLabel->setString(CCString::createWithFormat("CPS: %d", g_clickCount)->getCString());
                g_clickCount = 0;
                f->cpsTimer = 0.0f;
            }
        }
    }

    void onQuit() {
        if (g_speedhack && CCDirector::get() && CCDirector::get()->getScheduler())
            CCDirector::get()->getScheduler()->setTimeScale(1.0f);
        if (m_objectLayer) m_objectLayer->setPosition({0, 0});
        PlayLayer::onQuit();
    }
};

// ============================================================
//                     PAUSELAYER HOOK
// ============================================================
class $modify(NLPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        auto ws = CCDirector::get()->getWinSize();
        auto spr = ButtonSprite::create("NL");
        if (!spr) return;
        spr->setScale(0.9f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(NLPauseLayer::onOpenNLMenu));
        btn->setPosition({ws.width - 40.f, ws.height - 60.f});
        auto menu = CCMenu::create();
        menu->setPosition({0,0});
        menu->addChild(btn);
        this->addChild(menu, 100);
    }
    void onOpenNLMenu(CCObject*) {
        auto scene = CCDirector::get()->getRunningScene();
        if (!scene) return;
        if (auto menu = NeverloseMenu::create())
            scene->addChild(menu, 9999);
    }
};

// ============================================================
//                   PLAYEROBJECT HOOK
// ============================================================
class $modify(NLPlayerObject, PlayerObject) {
    void update(float dt) {
        PlayerObject::update(dt);

        if (g_aaEnabled) {
            if (g_aaFlipX) this->setScaleX(-1.0f);
            if (g_aaFlipY) this->setScaleY(-1.0f);
            if (g_spinbot) {
                g_spinPhase += g_spinSpeed * dt;
                this->setRotation(g_spinPhase * 360.0f);
            }
        }
    }
};

// ============================================================
//                  LEVELINFOLAYER HOOK
// ============================================================
class $modify(NLLevelInfoLayer, LevelInfoLayer) {
    void onPlay(CCObject* sender) {
        LevelInfoLayer::onPlay(sender);
    }
};

// ============================================================
//                          ENTRY
// ============================================================
$execute {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}
