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
    return ButtonSprite::create(CCString::createWithFormat("%.2f", v)->getCString());
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
        float cx = ws.width / 2.f, cy = ws.height / 2.f;
        this->addChild(CCLayerColor::create({0,0,0,200}), -1);
        auto p = CCScale9Sprite::create("GJ_square01.png");
        p->setContentSize({520.f, 260.f}); p->setPosition({cx, cy}); this->addChild(p);
        auto t = CCLabelBMFont::create(m_isSpeed ? "Speedhack Value" : "Spinbot Speed", "goldFont.fnt");
        t->setPosition({cx, cy + 90.f}); t->setScale(0.7f); t->setColor({0,200,255}); this->addChild(t);
        auto v = CCLabelBMFont::create(
            CCString::createWithFormat("%.2f", m_isSpeed ? g_speed : g_spinSpeed)->getCString(),
            "goldFont.fnt");
        v->setPosition({cx, cy + 30.f}); v->setScale(0.9f);
        v->setID("value-label"); this->addChild(v);
        auto m = CCMenu::create(); m->setPosition({0,0}); this->addChild(m);
        auto mk = [&](const char* tt, cocos2d::SEL_MenuHandler cb, float x, float y){
            auto b = CCMenuItemSpriteExtra::create(ButtonSprite::create(tt), this, cb);
            b->setPosition({x, y}); b->setScale(0.7f); m->addChild(b);
        };
        mk("-500", menu_selector(ValueInputPopup::onM500), cx - 200.f, cy - 30.f);
        mk("-1",   menu_selector(ValueInputPopup::onM1),   cx - 70.f,  cy - 30.f);
        mk("+1",   menu_selector(ValueInputPopup::onP1),   cx + 70.f,  cy - 30.f);
        mk("+500", menu_selector(ValueInputPopup::onP500), cx + 200.f, cy - 30.f);
        mk("OK",   menu_selector(ValueInputPopup::onCancel), cx, cy - 110.f);
        this->setTouchEnabled(true); this->setKeypadEnabled(true);
        return true;
    }
    void ref() {
        auto l = this->getChildByID("value-label");
        if (!l) return;
        if (auto x = typeinfo_cast<CCLabelBMFont*>(l))
            x->setString(CCString::createWithFormat("%.2f",
                m_isSpeed ? g_speed : g_spinSpeed)->getCString());
    }
    void ap() {
        if (g_speedhack && CCDirector::get() && CCDirector::get()->getScheduler())
            CCDirector::get()->getScheduler()->setTimeScale(g_speed);
    }
    void onM500(CCObject*) { if (m_isSpeed) { g_speed -= 500.f; if (g_speed < 1.f) g_speed = 1.f; ap(); } else { g_spinSpeed -= 500.f; if (g_spinSpeed < 1.f) g_spinSpeed = 1.f; } ref(); }
    void onM1(CCObject*)   { if (m_isSpeed) { g_speed -= 1.f;   if (g_speed < 1.f) g_speed = 1.f; ap(); } else { g_spinSpeed -= 1.f;   if (g_spinSpeed < 1.f) g_spinSpeed = 1.f; } ref(); }
    void onP1(CCObject*)   { if (m_isSpeed) { g_speed += 1.f;   if (g_speed > 500.f) g_speed = 500.f; ap(); } else { g_spinSpeed += 1.f; if (g_spinSpeed > 500.f) g_spinSpeed = 500.f; } ref(); }
    void onP500(CCObject*) { if (m_isSpeed) { g_speed += 500.f; if (g_speed > 500.f) g_speed = 500.f; ap(); } else { g_spinSpeed += 500.f; if (g_spinSpeed > 500.f) g_spinSpeed = 500.f; } ref(); }
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

    CCScale9Sprite* m_mainPanel = nullptr;
    CCScale9Sprite* m_sidePanel = nullptr;

    bool m_dragging = false;
    bool m_closing  = false;
    CCPoint m_dragStart = {0,0};
    int m_curPage = 0;

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
        if (p < 0) p = 0; if (p > 6) p = 6;

        if (p != m_curPage && m_mainPanel) {
            m_mainPanel->stopAllActions();
            m_mainPanel->runAction(CCSequence::create(
                CCScaleTo::create(0.08f, 0.96f),
                CCEaseBackOut::create(CCScaleTo::create(0.18f, 1.0f)),
                nullptr));
        }
        m_curPage = p;

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
        float cx = ws.width / 2.f, cy = ws.height / 2.f;
        this->addChild(CCLayerColor::create({0,0,0,180}), -1);

        m_mainPanel = CCScale9Sprite::create("GJ_square01.png");
        m_mainPanel->setContentSize({780.f, 520.f}); m_mainPanel->setPosition({cx, cy});
        m_mainPanel->setColor({15,15,15}); this->addChild(m_mainPanel);

        m_sidePanel = CCScale9Sprite::create("GJ_square01.png");
        m_sidePanel->setContentSize({220.f, 500.f}); m_sidePanel->setPosition({cx - 270.f, cy});
        m_sidePanel->setColor({25,25,30}); this->addChild(m_sidePanel);

        auto lg = CCLabelBMFont::create("NEVERLOSE", "goldFont.fnt");
        lg->setPosition({cx - 270.f, cy + 215.f}); lg->setScale(0.65f); this->addChild(lg);
        auto mn = CCMenu::create(); mn->setPosition({0,0}); this->addChild(mn);
        auto cs = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        if (cs) cs->setScale(1.2f);
        auto cb = CCMenuItemSpriteExtra::create(cs, this, menu_selector(NeverloseMenu::onClose));
        cb->setPosition({cx + 360.f, cy + 235.f}); mn->addChild(cb);

        float ty[7] = {cy + 105.f, cy + 70.f, cy + 35.f, cy, cy - 35.f, cy - 70.f, cy - 105.f};
        auto mkTab = [&](const char* t, cocos2d::SEL_MenuHandler cb_, float y) {
            auto b = CCMenuItemSpriteExtra::create(ButtonSprite::create(t), this, cb_);
            b->setPosition({cx - 270.f, y}); b->setScale(0.5f); mn->addChild(b); return b;
        };
        m_tabMain      = mkTab("Main",      menu_selector(NeverloseMenu::onTabMain),      ty[0]);
        m_tabRage      = mkTab("Rage",      menu_selector(NeverloseMenu::onTabRage),      ty[1]);
        m_tabVisuals   = mkTab("Visuals",   menu_selector(NeverloseMenu::onTabVisuals),   ty[2]);
        m_tabAntiAim   = mkTab("Anti-Aim",  menu_selector(NeverloseMenu::onTabAntiAim),   ty[3]);
        m_tabCreate    = mkTab("Create",    menu_selector(NeverloseMenu::onTabCreate),    ty[4]);
        m_tabLegit     = mkTab("Legit",     menu_selector(NeverloseMenu::onTabLegit),     ty[5]);
        m_tabCosmetics = mkTab("Cosmetics", menu_selector(NeverloseMenu::onTabCosmetics), ty[6]);

        auto addBtn = [&](CCMenuItemSpriteExtra*& btn, const char* txt, cocos2d::SEL_MenuHandler cb, CCPoint pos, float sc, std::vector<CCNode*>& vec) {
            btn = CCMenuItemSpriteExtra::create(ButtonSprite::create(txt), this, cb);
            btn->setPosition(pos); btn->setScale(sc); mn->addChild(btn); vec.push_back(btn);
        };

        addBtn(m_jumpHackBtn, "Jump Hack: OFF", menu_selector(NeverloseMenu::onJumpHack), {cx + 60.f, cy + 130.f}, 0.55f, m_mainNodes);
        addBtn(m_noclipBtn,   "Noclip: OFF",    menu_selector(NeverloseMenu::onNoclip),   {cx + 60.f, cy + 130.f}, 0.55f, m_rageNodes);
        addBtn(m_autoJumpBtn, "AutoJump: OFF",  menu_selector(NeverloseMenu::onAutoJump), {cx + 60.f, cy + 60.f},  0.55f, m_rageNodes);
        addBtn(m_ldmBtn,      "LDM: OFF",       menu_selector(NeverloseMenu::onLDM),      {cx + 60.f, cy + 90.f},  0.55f, m_visualsNodes);
        addBtn(m_autoLDMBtn,  "Auto LDM: OFF",  menu_selector(NeverloseMenu::onAutoLDM),  {cx + 60.f, cy + 25.f},  0.55f, m_visualsNodes);

        float lx = cx - 150.f, rx = cx + 150.f;
        auto mkArrow = [&](const char* txt, cocos2d::SEL_MenuHandler cb_, CCPoint pos) {
            auto b = CCMenuItemSpriteExtra::create(ButtonSprite::create(txt), this, cb_);
            b->setPosition(pos); b->setScale(0.9f); mn->addChild(b); return b;
        };

        addBtn(m_spinbotBtn, "Spinbot: OFF", menu_selector(NeverloseMenu::onSpinbot), {lx, cy + 150.f}, 0.55f, m_antiAimNodes);
        auto st = CCLabelBMFont::create("Spinbot Speed", "goldFont.fnt");
        st->setPosition({lx, cy + 105.f}); st->setScale(0.55f); st->setColor({0,200,255});
        this->addChild(st); m_antiAimNodes.push_back(st);
        m_spinDown = mkArrow("-", menu_selector(NeverloseMenu::onSpinDown), {lx - 70.f, cy + 65.f});
        m_antiAimNodes.push_back(m_spinDown);
        m_spinValueBtn = CCMenuItemSpriteExtra::create(makeNumber(g_spinSpeed), this, menu_selector(NeverloseMenu::onSpinValue));
        m_spinValueBtn->setPosition({lx, cy + 65.f}); mn->addChild(m_spinValueBtn); m_antiAimNodes.push_back(m_spinValueBtn);
        m_spinUp = mkArrow("+", menu_selector(NeverloseMenu::onSpinUp), {lx + 70.f, cy + 65.f});
        m_antiAimNodes.push_back(m_spinUp);
        addBtn(m_aaEnabledBtn, "Anti-Aim: OFF", menu_selector(NeverloseMenu::onAAEnabled), {lx, cy + 5.f}, 0.55f, m_antiAimNodes);
        addBtn(m_aaFlipXBtn,   "Flip Back: OFF", menu_selector(NeverloseMenu::onAAFlipX), {lx, cy - 45.f}, 0.55f, m_antiAimNodes);
        addBtn(m_aaFlipYBtn,   "Flip Down: OFF", menu_selector(NeverloseMenu::onAAFlipY), {lx, cy - 95.f}, 0.55f, m_antiAimNodes);
        addBtn(m_shakeBtn,     "Shake: OFF",     menu_selector(NeverloseMenu::onShake),   {rx, cy + 150.f}, 0.55f, m_antiAimNodes);
        addBtn(m_nlGraviteBtn, "NL.exe Gravite: OFF", menu_selector(NeverloseMenu::onNLGravite), {rx, cy + 95.f}, 0.55f, m_antiAimNodes);
        addBtn(m_speedhackBtn, "Speedhack: OFF", menu_selector(NeverloseMenu::onSpeedhack), {rx, cy + 40.f}, 0.55f, m_antiAimNodes);
        auto sht = CCLabelBMFont::create("Speedhack Value", "goldFont.fnt");
        sht->setPosition({rx, cy - 10.f}); sht->setScale(0.55f); sht->setColor({0,200,255});
        this->addChild(sht); m_antiAimNodes.push_back(sht);
        m_shDown = mkArrow("-", menu_selector(NeverloseMenu::onSpeedhackDown), {rx - 70.f, cy - 55.f});
        m_antiAimNodes.push_back(m_shDown);
        m_shValueBtn = CCMenuItemSpriteExtra::create(makeNumber(g_speed), this, menu_selector(NeverloseMenu::onSpeedValue));
        m_shValueBtn->setPosition({rx, cy - 55.f}); mn->addChild(m_shValueBtn); m_antiAimNodes.push_back(m_shValueBtn);
        m_shUp = mkArrow("+", menu_selector(NeverloseMenu::onSpeedhackUp), {rx + 70.f, cy - 55.f});
        m_antiAimNodes.push_back(m_shUp);

        addBtn(m_copyHackBtn, "Copy Hack: OFF", menu_selector(NeverloseMenu::onCopyHack), {cx + 60.f, cy + 100.f}, 0.55f, m_createNodes);
        addBtn(m_autoSafeModeBtn, "Auto Safe Mode: OFF", menu_selector(NeverloseMenu::onAutoSafeMode), {cx + 60.f, cy + 130.f}, 0.55f, m_legitNodes);
        addBtn(m_cheatIndBtn, "Cheat Ind: OFF", menu_selector(NeverloseMenu::onCheatIndicator), {cx + 60.f, cy + 60.f}, 0.55f, m_legitNodes);
        addBtn(m_cpsBtn,      "CPS: OFF",       menu_selector(NeverloseMenu::onCPS),   {cx + 60.f, cy - 10.f},  0.55f, m_legitNodes);
        addBtn(m_timeBtn,     "Time: OFF",      menu_selector(NeverloseMenu::onTime),  {cx + 60.f, cy - 80.f},  0.55f, m_legitNodes);
        addBtn(m_fpsBtn,      "FPS: OFF",       menu_selector(NeverloseMenu::onFPS),   {cx + 60.f, cy - 150.f}, 0.55f, m_legitNodes);
        addBtn(m_unlockIconsBtn,  "Icons: OFF",  menu_selector(NeverloseMenu::onUnlockIcons),  {cx + 60.f, cy + 130.f}, 0.55f, m_cosmeticsNodes);
        addBtn(m_unlockVaultBtn,  "Vault: OFF",  menu_selector(NeverloseMenu::onUnlockVault),  {cx + 60.f, cy + 60.f},  0.55f, m_cosmeticsNodes);
        addBtn(m_unlockColorsBtn, "Colors: OFF", menu_selector(NeverloseMenu::onUnlockColors), {cx + 60.f, cy - 10.f},  0.55f, m_cosmeticsNodes);
        addBtn(m_unlockLevelsBtn, "Levels: OFF", menu_selector(NeverloseMenu::onUnlockLevels), {cx + 60.f, cy - 80.f},  0.55f, m_cosmeticsNodes);

        setPage(0);
        refreshButtons();

        this->setScale(0.3f);
        this->runAction(CCEaseBackOut::create(CCScaleTo::create(0.35f, 1.0f)));

        this->setKeypadEnabled(true);
        this->setTouchEnabled(true);
        return true;
    }

    void registerWithTouchDispatcher() override {
        CCDirector::get()->getTouchDispatcher()->addTargetedDelegate(this, -501, true);
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent*) override {
        m_dragging = true;
        m_dragStart = touch->getLocation() - this->getPosition();
        return true;
    }
    void ccTouchMoved(CCTouch* touch, CCEvent*) override {
        if (m_dragging) this->setPosition(touch->getLocation() - m_dragStart);
    }
    void ccTouchEnded(CCTouch*, CCEvent*) override { m_dragging = false; }
    void ccTouchCancelled(CCTouch*, CCEvent*) override { m_dragging = false; }

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
    void onLDM(CCObject*)      { g_ldm = !g_ldm;           refreshButtons(); }
    void onAutoLDM(CCObject*)  { g_autoLDM = !g_autoLDM;   refreshButtons(); }
    void onSpinbot(CCObject*)  { g_spinbot = !g_spinbot;   refreshButtons(); }
        void onShake(CCObject*)    { g_shake = !g_shake; g_shakeTimer = 0.f; refreshButtons(); }
    void onNLGravite(CCObject*){ g_nlGravite = !g_nlGravite; g_gravTimer = 0.f; refreshButtons(); }

    void onSpinUp(CCObject*) {
        g_spinSpeed += 1.f; if (g_spinSpeed > 500.f) g_spinSpeed = 500.f;
        if (m_spinValueBtn) m_spinValueBtn->setNormalImage(makeNumber(g_spinSpeed));
    }
    void onSpinDown(CCObject*) {
        g_spinSpeed -= 1.f; if (g_spinSpeed < 1.f) g_spinSpeed = 1.f;
        if (m_spinValueBtn) m_spinValueBtn->setNormalImage(makeNumber(g_spinSpeed));
    }
    void onSpinValue(CCObject*) { if (auto p = ValueInputPopup::create(false)) this->addChild(p, 9); }
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
    void onSpeedValue(CCObject*) { if (auto p = ValueInputPopup::create(true)) this->addChild(p, 9); }
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

    void onClose(CCObject*) {
        if (m_closing) return;
        m_closing = true;
        this->runAction(CCSequence::create(
            CCEaseBackIn::create(CCScaleTo::create(0.18f, 0.4f)),
            CCCallFunc::create(this, callfunc_selector(NeverloseMenu::removeFromParentAndCleanup)),
            nullptr));
    }
    void keyBackClicked() { this->onClose(nullptr); }
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
            if (f->cpsLabel) { f->cpsLabel->setScale(0.4f); f->cpsLabel->setPosition({60.f, ws.height - 20.f}); f->cpsLabel->setColor({0,255,255}); this->addChild(f->cpsLabel, 100); }
        }
        if (g_showTime) {
            f->timeLabel = CCLabelBMFont::create("Time: 0.00", "bigFont.fnt");
            if (f->timeLabel) { f->timeLabel->setScale(0.4f); f->timeLabel->setPosition({60.f, ws.height - 45.f}); f->timeLabel->setColor({255,255,0}); this->addChild(f->timeLabel, 100); }
        }
        if (g_showFPS) {
            f->fpsLabel = CCLabelBMFont::create("FPS: 60", "bigFont.fnt");
            if (f->fpsLabel) { f->fpsLabel->setScale(0.4f); f->fpsLabel->setPosition({60.f, ws.height - 70.f}); f->fpsLabel->setColor({255,100,255}); this->addChild(f->fpsLabel, 100); }
        }
        if (g_cheatIndicator) {
            f->cheatLabel = CCLabelBMFont::create("[NL.exe]", "bigFont.fnt");
            if (f->cheatLabel) { f->cheatLabel->setScale(0.5f); f->cheatLabel->setPosition({ws.width - 60.f, ws.height - 20.f}); f->cheatLabel->setColor({255,50,50}); this->addChild(f->cheatLabel, 100); }
        }
        return true;
    }

    void update(float dt) override {
        PlayLayer::update(dt);
        auto f = m_fields.self();
        if (!f) return;

        auto player = m_player1;
        if (player) {
            if (g_autoJump || g_jumpHack) player->pushButton(PlayerButton::Jump);
            if (g_shake) {
                g_shakeTimer += dt;
                if (g_shakeTimer >= 0.08f) {
                    g_shakeTimer = 0.f;
                    g_gravState = !g_gravState;
                    player->flipGravity(g_gravState, true);
                }
            }
            if (g_nlGravite) {
                g_gravTimer += dt;
                if (g_gravTimer >= 0.15f) {
                    g_gravTimer = 0.f;
                    g_gravState = !g_gravState;
                    player->flipGravity(g_gravState, true);
                }
            }
        }

        f->timeAlive += dt; f->frameCount++; f->fpsTimer += dt;
        if (f->fpsTimer >= 0.5f) {
            float fps = f->frameCount / f->fpsTimer;
            if (f->fpsLabel) f->fpsLabel->setString(CCString::createWithFormat("FPS: %.0f", fps)->getCString());
            f->frameCount = 0; f->fpsTimer = 0.0f;
        }
        if (f->timeLabel) f->timeLabel->setString(CCString::createWithFormat("Time: %.2f", f->timeAlive)->getCString());
        if (f->cpsLabel) {
            f->cpsTimer += dt;
            if (f->cpsTimer >= 1.0f) {
                f->cpsLabel->setString(CCString::createWithFormat("CPS: %d", g_clickCount)->getCString());
                g_clickCount = 0; f->cpsTimer = 0.0f;
            }
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* obj) override {
        if (g_noclip) return;
        PlayLayer::destroyPlayer(player, obj);
    }

    void onQuit() {
        if (g_speedhack && CCDirector::get() && CCDirector::get()->getScheduler())
            CCDirector::get()->getScheduler()->setTimeScale(1.0f);
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
        menu->setPosition({0,0}); menu->addChild(btn);
        this->addChild(menu, 100);
    }
    void onOpenNLMenu(CCObject*) {
        auto scene = CCDirector::get()->getRunningScene();
        if (!scene) return;
        if (auto menu = NeverloseMenu::create()) scene->addChild(menu, 9999);
    }
};

// ============================================================
//                   PLAYEROBJECT HOOK
// ============================================================
class $modify(NLPlayerObject, PlayerObject) {
    void update(float dt) override {
        PlayerObject::update(dt);
        if (g_spinbot) {
            g_spinPhase += g_spinSpeed * 60.f * dt;
            if (g_spinPhase > 100000.f) g_spinPhase -= 100000.f;
            this->setRotation(g_spinPhase);
        } else {
            this->setRotation(0.f);
            g_spinPhase = 0.f;
        }
        if (g_aaEnabled) {
            if (g_aaFlipX) this->setScaleX(-1.0f);
            if (g_aaFlipY) this->setScaleY(-1.0f);
        } else {
            this->setScaleX(1.0f); this->setScaleY(1.0f);
        }
    }
};

// ============================================================
//                  LEVELINFOLAYER HOOK
// ============================================================
class $modify(NLLevelInfoLayer, LevelInfoLayer) {
    void onPlay(CCObject* sender) { LevelInfoLayer::onPlay(sender); }
};

$execute {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}
