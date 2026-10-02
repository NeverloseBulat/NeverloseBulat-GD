#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include <vector>
#include <cstdlib>
#include <ctime>

using namespace geode::prelude;

static bool g_noclip=false, g_spinbot=false, g_shake=false,
            g_nlGravite=false, g_ldm=false, g_autoLDM=false, g_speedhack=false,
            g_jumpHack=false, g_aaEnabled=false, g_aaFlipX=false, g_aaFlipY=false,
            g_showCPS=false, g_showTime=false, g_showFPS=false,
            g_showPercent=false, g_hidePlayer=false, g_slowMo=false, g_fastMo=false,
            g_noDeathEffect=false, g_noWaveTrail=false, g_reverseGravity=false,
            g_glowCube=false, g_hideAttempts=false, g_autoRetry=false, g_godMode=false,
            g_instantComplete=false, g_noSpikes=false, g_cheatIndicator=false;

static int   g_menuTheme    = 0;
static int   g_menuOpacity  = 2;
static int   g_menuScale    = 1;
static bool  g_hideBranding = false;
static bool  g_blurEffect   = false;
static bool  g_rainbowMode  = false;
static float g_rainbowTimer = 0.f;
static float g_cubeScaleVal = 1.0f;

static float g_speed=1.0f, g_spinSpeed=5.0f, g_shakeTimer=0.0f,
             g_spinPhase=0.0f, g_nlGravPhase=0.0f;
static bool  g_gravState=false;
static int   g_clickCount=0;

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
    void ap() {
        if (g_speedhack && CCDirector::get() && CCDirector::get()->getScheduler())
            CCDirector::get()->getScheduler()->setTimeScale(g_speed);
    }
    void ref() {
        auto l = this->getChildByID("value-label");
        if (!l) return;
        if (auto x = typeinfo_cast<CCLabelBMFont*>(l))
            x->setString(CCString::createWithFormat("%.2f", m_isSpeed ? g_speed : g_spinSpeed)->getCString());
    }
    void onM500(CCObject*) { if (m_isSpeed){g_speed-=500.f; if(g_speed<1.f)g_speed=1.f; ap();} else {g_spinSpeed-=500.f; if(g_spinSpeed<1.f)g_spinSpeed=1.f;} ref(); }
    void onM1(CCObject*)   { if (m_isSpeed){g_speed-=1.f;   if(g_speed<1.f)g_speed=1.f; ap();} else {g_spinSpeed-=1.f;   if(g_spinSpeed<1.f)g_spinSpeed=1.f;} ref(); }
    void onP1(CCObject*)   { if (m_isSpeed){g_speed+=1.f;   if(g_speed>500.f)g_speed=500.f; ap();} else {g_spinSpeed+=1.f; if(g_spinSpeed>500.f)g_spinSpeed=500.f;} ref(); }
    void onP500(CCObject*) { if (m_isSpeed){g_speed+=500.f; if(g_speed>500.f)g_speed=500.f; ap();} else {g_spinSpeed+=500.f; if(g_spinSpeed>500.f)g_spinSpeed=500.f;} ref(); }
    void onCancel(CCObject*) { this->removeFromParentAndCleanup(true); }
    void keyBackClicked()    { this->removeFromParentAndCleanup(true); }
};

// ============================================================
//                    CUBE SIZE POPUP
// ============================================================
class CubeSizePopup : public CCLayer {
public:
    static CubeSizePopup* create() {
        auto r = new CubeSizePopup();
        if (!r) return nullptr;
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
        auto t = CCLabelBMFont::create("Cube Size (1 - 10)", "goldFont.fnt");
        t->setPosition({cx, cy + 90.f}); t->setScale(0.7f); t->setColor({0,200,255}); this->addChild(t);
        auto v = CCLabelBMFont::create(
            CCString::createWithFormat("%.2f", g_cubeScaleVal)->getCString(), "goldFont.fnt");
        v->setPosition({cx, cy + 30.f}); v->setScale(0.9f);
        v->setID("cube-label"); this->addChild(v);
        auto m = CCMenu::create(); m->setPosition({0,0}); this->addChild(m);
        auto mk = [&](const char* tt, cocos2d::SEL_MenuHandler cb, float x, float y){
            auto b = CCMenuItemSpriteExtra::create(ButtonSprite::create(tt), this, cb);
            b->setPosition({x, y}); b->setScale(0.7f); m->addChild(b);
        };
        mk("-5", menu_selector(CubeSizePopup::onM5), cx - 200.f, cy - 30.f);
        mk("-1", menu_selector(CubeSizePopup::onM1), cx - 70.f,  cy - 30.f);
        mk("+1", menu_selector(CubeSizePopup::onP1), cx + 70.f,  cy - 30.f);
        mk("+5", menu_selector(CubeSizePopup::onP5), cx + 200.f, cy - 30.f);
        mk("OK", menu_selector(CubeSizePopup::onClose), cx, cy - 110.f);
        this->setTouchEnabled(true); this->setKeypadEnabled(true);
        return true;
    }
    void refresh() {
        auto l = this->getChildByID("cube-label");
        if (!l) return;
        if (auto x = typeinfo_cast<CCLabelBMFont*>(l))
            x->setString(CCString::createWithFormat("%.2f", g_cubeScaleVal)->getCString());
    }
    void onM5(CCObject*) { g_cubeScaleVal -= 5.f; if(g_cubeScaleVal<1.f) g_cubeScaleVal=1.f; refresh(); }
    void onM1(CCObject*) { g_cubeScaleVal -= 1.f; if(g_cubeScaleVal<1.f) g_cubeScaleVal=1.f; refresh(); }
    void onP1(CCObject*) { g_cubeScaleVal += 1.f; if(g_cubeScaleVal>10.f) g_cubeScaleVal=10.f; refresh(); }
    void onP5(CCObject*) { g_cubeScaleVal += 5.f; if(g_cubeScaleVal>10.f) g_cubeScaleVal=10.f; refresh(); }
    void onClose(CCObject*) { this->removeFromParentAndCleanup(true); }
    void keyBackClicked()   { this->removeFromParentAndCleanup(true); }
};

// ==== static labels ====
static CCLabelBMFont* s_cpsLabel     = nullptr;
static CCLabelBMFont* s_timeLabel    = nullptr;
static CCLabelBMFont* s_fpsLabel     = nullptr;
static CCLabelBMFont* s_cheatLabel   = nullptr;
static CCLabelBMFont* s_percentLabel = nullptr;
static float s_timeAlive  = 0.f;
static int   s_frameCount = 0;
static float s_fpsTimer   = 0.f;
static float s_cpsTimer   = 0.f;

// ============================================================
//                       NEVERLOSE MENU
// ============================================================
class NeverloseMenu : public CCLayer {
protected:
    std::vector<CCNode*> m_mainNodes, m_rageNodes, m_visualsNodes, m_antiAimNodes,
                         m_createNodes, m_legitNodes, m_qolNodes, m_worldNodes;
    CCMenuItemSpriteExtra *m_jumpHackBtn=nullptr,*m_noclipBtn=nullptr,*m_ldmBtn=nullptr,
        *m_autoLDMBtn=nullptr,*m_spinbotBtn=nullptr,*m_shakeBtn=nullptr,
        *m_nlGraviteBtn=nullptr,*m_spinDown=nullptr,*m_spinUp=nullptr,
        *m_spinValueBtn=nullptr,*m_speedhackBtn=nullptr,*m_shDown=nullptr,
        *m_shUp=nullptr,*m_shValueBtn=nullptr,*m_aaEnabledBtn=nullptr,
        *m_aaFlipXBtn=nullptr,*m_aaFlipYBtn=nullptr,
        *m_showPercentBtn=nullptr,*m_hidePlayerBtn=nullptr,*m_slowMoBtn=nullptr,
        *m_fastMoBtn=nullptr,*m_noDeathEffectBtn=nullptr,
        *m_showHitboxBtn=nullptr,*m_noWaveBtn=nullptr,
        *m_instantCompleteBtn=nullptr,*m_noSpikesBtn=nullptr,
        *m_reverseGravBtn=nullptr,*m_hideAttemptsBtn=nullptr,
        *m_autoRetryBtn=nullptr,*m_godModeBtn=nullptr,
        *m_cheatIndBtn=nullptr,*m_cpsBtn=nullptr,*m_timeBtn=nullptr,*m_fpsBtn=nullptr,
        *m_copyHackBtn=nullptr,
        *m_colorBtn=nullptr,*m_opacityBtn=nullptr,*m_scaleBtn=nullptr,
        *m_brandingBtn=nullptr,*m_blurBtn=nullptr,
        *m_colorAllBtn=nullptr,*m_rainbowBtn=nullptr,
        *m_glowCubeBtn=nullptr,*m_cubeScaleBtn=nullptr,
        *m_tabMain=nullptr,*m_tabRage=nullptr,*m_tabVisuals=nullptr,
        *m_tabAntiAim=nullptr,*m_tabCreate=nullptr,*m_tabLegit=nullptr,
        *m_tabQOL=nullptr,*m_tabWorld=nullptr;

    CCScale9Sprite* m_mainPanel = nullptr;
    CCScale9Sprite* m_sidePanel = nullptr;
    CCLayerColor*   m_darkOverlay = nullptr;
    CCLabelBMFont*  m_brandLabel = nullptr;
    std::vector<CCScale9Sprite*> m_tabBgs;
    std::vector<CCLabelBMFont*>  m_tabLabels;
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

    void applyColorTheme() {
        ccColor3B c;
        switch (g_menuTheme) {
            case 0: c = {15,15,15};  break;
            case 1: c = {20,30,60};  break;
            case 2: c = {60,15,15};  break;
            case 3: c = {15,50,20};  break;
            case 4: c = {45,15,60};  break;
            default: c = {55,45,10}; break;
        }
        if (m_mainPanel) m_mainPanel->setColor(c);
        if (m_sidePanel) {
            GLubyte r = c.r + 10 > 255 ? 255 : c.r + 10;
            GLubyte g = c.g + 10 > 255 ? 255 : c.g + 10;
            GLubyte b = c.b + 15 > 255 ? 255 : c.b + 15;
            m_sidePanel->setColor({r, g, b});
        }
        if (m_darkOverlay) {
            GLubyte a = 180;
            if (g_blurEffect) a = 230;
            if (g_menuOpacity == 0) a = (GLubyte)(a * 0.5f);
            else if (g_menuOpacity == 1) a = (GLubyte)(a * 0.75f);
            m_darkOverlay->setOpacity(a);
        }
        if (m_brandLabel) m_brandLabel->setVisible(!g_hideBranding);
        float sc = 1.0f;
        if (g_menuScale == 0) sc = 0.85f;
        else if (g_menuScale == 2) sc = 1.15f;
        this->setScale(sc);
    }

    void refreshButtons() {
        if (m_jumpHackBtn)     m_jumpHackBtn->setNormalImage(makeToggle("Jump Hack: ON","Jump Hack: OFF", g_jumpHack));
        if (m_noclipBtn)       m_noclipBtn->setNormalImage(makeToggle("Noclip: ON","Noclip: OFF", g_noclip));
        if (m_ldmBtn)          m_ldmBtn->setNormalImage(makeToggle("LDM: ON","LDM: OFF", g_ldm));
        if (m_autoLDMBtn)      m_autoLDMBtn->setNormalImage(makeToggle("Auto LDM: ON","Auto LDM: OFF", g_autoLDM));
        if (m_spinbotBtn)      m_spinbotBtn->setNormalImage(makeToggle("Spinbot: ON","Spinbot: OFF", g_spinbot));
        if (m_shakeBtn)        m_shakeBtn->setNormalImage(makeToggle("Shake: ON","Shake: OFF", g_shake));
        if (m_nlGraviteBtn)    m_nlGraviteBtn->setNormalImage(makeToggle("NL Gravite: ON","NL Gravite: OFF", g_nlGravite));
        if (m_speedhackBtn)    m_speedhackBtn->setNormalImage(makeToggle("Speedhack: ON","Speedhack: OFF", g_speedhack));
        if (m_aaEnabledBtn)    m_aaEnabledBtn->setNormalImage(makeToggle("Anti-Aim: ON","Anti-Aim: OFF", g_aaEnabled));
        if (m_aaFlipXBtn)      m_aaFlipXBtn->setNormalImage(makeToggle("Flip Back: ON","Flip Back: OFF", g_aaFlipX));
        if (m_aaFlipYBtn)      m_aaFlipYBtn->setNormalImage(makeToggle("Flip Down: ON","Flip Down: OFF", g_aaFlipY));
        if (m_showPercentBtn)  m_showPercentBtn->setNormalImage(makeToggle("Show %: ON","Show %: OFF", g_showPercent));
        if (m_hidePlayerBtn)   m_hidePlayerBtn->setNormalImage(makeToggle("Hide Player: ON","Hide Player: OFF", g_hidePlayer));
        if (m_slowMoBtn)       m_slowMoBtn->setNormalImage(makeToggle("Slow Motion: ON","Slow Motion: OFF", g_slowMo));
        if (m_fastMoBtn)       m_fastMoBtn->setNormalImage(makeToggle("Fast Motion: ON","Fast Motion: OFF", g_fastMo));
        if (m_noDeathEffectBtn)m_noDeathEffectBtn->setNormalImage(makeToggle("No Death FX: ON","No Death FX: OFF", g_noDeathEffect));
        if (m_showHitboxBtn)   m_showHitboxBtn->setNormalImage(makeToggle("Hitboxes: ON","Hitboxes: OFF", g_ldm));
        if (m_noWaveBtn)       m_noWaveBtn->setNormalImage(makeToggle("No Wave Trail: ON","No Wave Trail: OFF", g_noWaveTrail));
        if (m_instantCompleteBtn)m_instantCompleteBtn->setNormalImage(makeToggle("Instant: ON","Instant: OFF", g_instantComplete));
        if (m_noSpikesBtn)     m_noSpikesBtn->setNormalImage(makeToggle("No Spikes: ON","No Spikes: OFF", g_noSpikes));
        if (m_reverseGravBtn)  m_reverseGravBtn->setNormalImage(makeToggle("Reverse Grav: ON","Reverse Grav: OFF", g_reverseGravity));
        if (m_hideAttemptsBtn) m_hideAttemptsBtn->setNormalImage(makeToggle("Hide Attempts: ON","Hide Attempts: OFF", g_hideAttempts));
        if (m_autoRetryBtn)    m_autoRetryBtn->setNormalImage(makeToggle("Auto Retry: ON","Auto Retry: OFF", g_autoRetry));
        if (m_godModeBtn)      m_godModeBtn->setNormalImage(makeToggle("God Mode: ON","God Mode: OFF", g_godMode));
        if (m_cheatIndBtn)     m_cheatIndBtn->setNormalImage(makeToggle("Cheat Ind: ON","Cheat Ind: OFF", g_cheatIndicator));
        if (m_cpsBtn)          m_cpsBtn->setNormalImage(makeToggle("CPS: ON","CPS: OFF", g_showCPS));
        if (m_timeBtn)         m_timeBtn->setNormalImage(makeToggle("Time: ON","Time: OFF", g_showTime));
        if (m_fpsBtn)          m_fpsBtn->setNormalImage(makeToggle("FPS: ON","FPS: OFF", g_showFPS));
        if (m_copyHackBtn)     m_copyHackBtn->setNormalImage(makeToggle("Copy Hack: ON","Copy Hack: OFF", false));
        if (m_spinValueBtn) m_spinValueBtn->setNormalImage(makeNumber(g_spinSpeed));
        if (m_shValueBtn)   m_shValueBtn->setNormalImage(makeNumber(g_speed));

        const char* cn[6] = {"Dark","Blue","Red","Green","Purple","Gold"};
        const char* on[3] = {"50%","75%","100%"};
        const char* sc[3] = {"Small","Normal","Big"};
        if (m_colorBtn)     m_colorBtn->setNormalImage(ButtonSprite::create(CCString::createWithFormat("Color: %s", cn[g_menuTheme])->getCString()));
        if (m_opacityBtn)   m_opacityBtn->setNormalImage(ButtonSprite::create(CCString::createWithFormat("Opacity: %s", on[g_menuOpacity])->getCString()));
        if (m_scaleBtn)     m_scaleBtn->setNormalImage(ButtonSprite::create(CCString::createWithFormat("Scale: %s", sc[g_menuScale])->getCString()));
        if (m_brandingBtn)  m_brandingBtn->setNormalImage(ButtonSprite::create(g_hideBranding ? "Branding: Hide" : "Branding: Show"));
        if (m_blurBtn)      m_blurBtn->setNormalImage(makeToggle("Blur BG: ON","Blur BG: OFF", g_blurEffect));
        if (m_colorAllBtn)  m_colorAllBtn->setNormalImage(ButtonSprite::create(CCString::createWithFormat("Color All: %s", cn[g_menuTheme])->getCString()));
        if (m_rainbowBtn)   m_rainbowBtn->setNormalImage(makeToggle("Rainbow: ON","Rainbow: OFF", g_rainbowMode));
        if (m_glowCubeBtn)  m_glowCubeBtn->setNormalImage(makeToggle("Glow Cube: ON","Glow Cube: OFF", g_glowCube));
        if (m_cubeScaleBtn) m_cubeScaleBtn->setNormalImage(ButtonSprite::create(
            CCString::createWithFormat("Cube Size: %.2f", g_cubeScaleVal)->getCString()));
    }

    void setPage(int p) {
        if (p < 0) p = 0; if (p > 7) p = 7;
        if (p != m_curPage && m_mainPanel) {
            m_mainPanel->stopAllActions();
            m_mainPanel->runAction(CCSequence::create(
                CCScaleTo::create(0.08f, 0.96f),
                CCEaseBackOut::create(CCScaleTo::create(0.18f, 1.0f)), nullptr));
        }
        m_curPage = p;
        for (auto n : m_mainNodes)    if (n) n->setVisible(p == 0);
        for (auto n : m_rageNodes)    if (n) n->setVisible(p == 1);
        for (auto n : m_visualsNodes) if (n) n->setVisible(p == 2);
        for (auto n : m_antiAimNodes) if (n) n->setVisible(p == 3);
        for (auto n : m_createNodes)  if (n) n->setVisible(p == 4);
        for (auto n : m_legitNodes)   if (n) n->setVisible(p == 5);
        for (auto n : m_qolNodes)     if (n) n->setVisible(p == 6);
        for (auto n : m_worldNodes)   if (n) n->setVisible(p == 7);

        ccColor3B aBg = {50, 90, 200}, iBg = {28,28,35};
        ccColor3B aLbl = {255,255,255}, iLbl = {170,170,180};
        for (size_t i = 0; i < m_tabBgs.size(); i++) {
            bool act = (i == (size_t)p);
            if (m_tabBgs[i]) m_tabBgs[i]->setColor(act ? aBg : iBg);
            if (i < m_tabLabels.size() && m_tabLabels[i]) m_tabLabels[i]->setColor(act ? aLbl : iLbl);
        }
    }

    bool init() override {
        if (!CCLayer::init()) return false;
        auto ws = CCDirector::get()->getWinSize();
        float cx = ws.width / 2.f, cy = ws.height / 2.f;

        m_darkOverlay = CCLayerColor::create({0,0,0,180});
        this->addChild(m_darkOverlay, -1);

        m_mainPanel = CCScale9Sprite::create("GJ_square01.png");
        m_mainPanel->setContentSize({780.f, 520.f}); m_mainPanel->setPosition({cx, cy});
        m_mainPanel->setColor({15,15,15}); this->addChild(m_mainPanel);

        m_sidePanel = CCScale9Sprite::create("GJ_square01.png");
        m_sidePanel->setContentSize({220.f, 500.f}); m_sidePanel->setPosition({cx - 270.f, cy});
        m_sidePanel->setColor({25,25,30}); this->addChild(m_sidePanel);

        m_brandLabel = CCLabelBMFont::create("NEVERLOSE", "bigFont.fnt");
        m_brandLabel->setPosition({cx - 270.f, cy + 225.f});
        m_brandLabel->setScale(0.9f); this->addChild(m_brandLabel);

        auto mn = CCMenu::create(); mn->setPosition({0,0}); this->addChild(mn);
        auto cs = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        if (cs) cs->setScale(1.2f);
        auto cb = CCMenuItemSpriteExtra::create(cs, this, menu_selector(NeverloseMenu::onClose));
        cb->setPosition({cx + 360.f, cy + 235.f}); mn->addChild(cb);

        float ty[8] = {cy + 150.f, cy + 117.f, cy + 84.f, cy + 51.f, cy + 18.f,
                        cy - 15.f, cy - 48.f, cy - 81.f};
        auto mkTab = [&](const char* t, cocos2d::SEL_MenuHandler cb_, float y) {
            auto bg = CCScale9Sprite::create("GJ_square01.png");
            bg->setContentSize({200.f, 32.f}); bg->setColor({28,28,35});
            auto b = CCMenuItemSpriteExtra::create(bg, this, cb_);
            b->setPosition({cx - 270.f, y}); mn->addChild(b);
            m_tabBgs.push_back(bg);
            auto lbl = CCLabelBMFont::create(t, "bigFont.fnt");
            lbl->setPosition({cx - 270.f, y}); lbl->setScale(0.55f); lbl->setColor({170,170,180});
            this->addChild(lbl); m_tabLabels.push_back(lbl);
            return b;
        };
        m_tabMain    = mkTab("Main",     menu_selector(NeverloseMenu::onTabMain),    ty[0]);
        m_tabRage    = mkTab("Rage",     menu_selector(NeverloseMenu::onTabRage),    ty[1]);
        m_tabVisuals = mkTab("Visuals",  menu_selector(NeverloseMenu::onTabVisuals), ty[2]);
        m_tabAntiAim = mkTab("Anti-Aim", menu_selector(NeverloseMenu::onTabAntiAim), ty[3]);
        m_tabCreate  = mkTab("Create",   menu_selector(NeverloseMenu::onTabCreate),  ty[4]);
        m_tabLegit   = mkTab("Legit",    menu_selector(NeverloseMenu::onTabLegit),   ty[5]);
        m_tabQOL     = mkTab("QOL",      menu_selector(NeverloseMenu::onTabQOL),     ty[6]);
        m_tabWorld   = mkTab("NL.World", menu_selector(NeverloseMenu::onTabWorld),   ty[7]);

        auto addBtn = [&](CCMenuItemSpriteExtra*& btn, const char* txt, cocos2d::SEL_MenuHandler cb, CCPoint pos, float sc, std::vector<CCNode*>& vec) {
            btn = CCMenuItemSpriteExtra::create(ButtonSprite::create(txt), this, cb);
            btn->setPosition(pos); btn->setScale(sc); mn->addChild(btn); vec.push_back(btn);
        };

        addBtn(m_jumpHackBtn, "Jump Hack: OFF", menu_selector(NeverloseMenu::onJumpHack), {cx + 60.f, cy + 130.f}, 0.55f, m_mainNodes);

        addBtn(m_noclipBtn,       "Noclip: OFF",       menu_selector(NeverloseMenu::onNoclip),   {cx + 60.f, cy + 160.f}, 0.55f, m_rageNodes);
        addBtn(m_instantCompleteBtn,"Instant: OFF",    menu_selector(NeverloseMenu::onInst),     {cx + 60.f, cy + 95.f},  0.55f, m_rageNodes);
        addBtn(m_noSpikesBtn,     "No Spikes: OFF",    menu_selector(NeverloseMenu::onNoSpk),    {cx + 60.f, cy + 30.f},  0.55f, m_rageNodes);
        addBtn(m_godModeBtn,      "God Mode: OFF",     menu_selector(NeverloseMenu::onGod),      {cx + 60.f, cy - 35.f},  0.55f, m_rageNodes);
        addBtn(m_reverseGravBtn,  "Reverse Grav: OFF", menu_selector(NeverloseMenu::onRevGrav),  {cx + 60.f, cy - 100.f}, 0.55f, m_rageNodes);
        addBtn(m_hideAttemptsBtn, "Hide Attempts: OFF",menu_selector(NeverloseMenu::onHideAtt),  {cx + 60.f, cy - 165.f}, 0.55f, m_rageNodes);

        addBtn(m_ldmBtn,      "LDM: OFF",           menu_selector(NeverloseMenu::onLDM),       {cx + 60.f, cy + 160.f}, 0.55f, m_visualsNodes);
        addBtn(m_autoLDMBtn,  "Auto LDM: OFF",      menu_selector(NeverloseMenu::onAutoLDM),   {cx + 60.f, cy + 95.f},  0.55f, m_visualsNodes);
        addBtn(m_showHitboxBtn,"Hitboxes: OFF",     menu_selector(NeverloseMenu::onHitbox),    {cx + 60.f, cy + 30.f},  0.55f, m_visualsNodes);
        addBtn(m_noWaveBtn,   "No Wave Trail: OFF", menu_selector(NeverloseMenu::onNoWave),    {cx + 60.f, cy - 35.f},  0.55f, m_visualsNodes);

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
        addBtn(m_nlGraviteBtn, "NL Gravite: OFF",menu_selector(NeverloseMenu::onNLGrav),  {rx, cy + 95.f}, 0.55f, m_antiAimNodes);
        addBtn(m_speedhackBtn, "Speedhack: OFF", menu_selector(NeverloseMenu::onSpeedhack), {rx, cy + 40.f}, 0.55f, m_antiAimNodes);
        auto sht = CCLabelBMFont::create("Speedhack Value", "goldFont.fnt");
        sht->setPosition({rx, cy - 10.f}); sht->setScale(0.55f); sht->setColor({0,200,255});
        this->addChild(sht); m_antiAimNodes.push_back(sht);
        m_shDown = mkArrow("-", menu_selector(NeverloseMenu::onShDown), {rx - 70.f, cy - 55.f});
        m_antiAimNodes.push_back(m_shDown);
        m_shValueBtn = CCMenuItemSpriteExtra::create(makeNumber(g_speed), this, menu_selector(NeverloseMenu::onShValue));
        m_shValueBtn->setPosition({rx, cy - 55.f}); mn->addChild(m_shValueBtn); m_antiAimNodes.push_back(m_shValueBtn);
        m_shUp = mkArrow("+", menu_selector(NeverloseMenu::onShUp), {rx + 70.f, cy - 55.f});
        m_antiAimNodes.push_back(m_shUp);

        addBtn(m_copyHackBtn, "Copy Hack: OFF", menu_selector(NeverloseMenu::onCopyHack), {cx + 60.f, cy + 100.f}, 0.55f, m_createNodes);

        addBtn(m_cheatIndBtn,   "Cheat Ind: OFF",   menu_selector(NeverloseMenu::onCheatInd), {cx + 60.f, cy + 160.f}, 0.55f, m_legitNodes);
        addBtn(m_cpsBtn,        "CPS: OFF",         menu_selector(NeverloseMenu::onCPS),      {cx + 60.f, cy + 95.f},  0.55f, m_legitNodes);
        addBtn(m_timeBtn,       "Time: OFF",        menu_selector(NeverloseMenu::onTime),     {cx + 60.f, cy + 30.f},  0.55f, m_legitNodes);
        addBtn(m_fpsBtn,        "FPS: OFF",         menu_selector(NeverloseMenu::onFPS),      {cx + 60.f, cy - 35.f},  0.55f, m_legitNodes);
        addBtn(m_autoRetryBtn,  "Auto Retry: OFF",  menu_selector(NeverloseMenu::onAutoRetry),{cx + 60.f, cy - 100.f}, 0.55f, m_legitNodes);

        addBtn(m_showPercentBtn,"Show %: OFF",       menu_selector(NeverloseMenu::onShowPct), {cx + 60.f, cy + 160.f}, 0.55f, m_qolNodes);
        addBtn(m_hidePlayerBtn, "Hide Player: OFF",  menu_selector(NeverloseMenu::onHidePl),  {cx + 60.f, cy + 95.f},  0.55f, m_qolNodes);
        addBtn(m_slowMoBtn,     "Slow Motion: OFF",  menu_selector(NeverloseMenu::onSlowMo),  {cx + 60.f, cy + 30.f},  0.55f, m_qolNodes);
        addBtn(m_fastMoBtn,     "Fast Motion: OFF",  menu_selector(NeverloseMenu::onFastMo),  {cx + 60.f, cy - 35.f},  0.55f, m_qolNodes);
        addBtn(m_noDeathEffectBtn,"No Death FX: OFF",menu_selector(NeverloseMenu::onNoDeath), {cx + 60.f, cy - 100.f}, 0.55f, m_qolNodes);

        addBtn(m_colorBtn,     "Color: Dark",       menu_selector(NeverloseMenu::onTheme),    {cx + 60.f, cy + 225.f}, 0.55f, m_worldNodes);
        addBtn(m_opacityBtn,   "Opacity: 100%",     menu_selector(NeverloseMenu::onOpacity),  {cx + 60.f, cy + 160.f}, 0.55f, m_worldNodes);
        addBtn(m_scaleBtn,     "Scale: Normal",     menu_selector(NeverloseMenu::onScale),    {cx + 60.f, cy + 95.f},  0.55f, m_worldNodes);
        addBtn(m_brandingBtn,  "Branding: Show",    menu_selector(NeverloseMenu::onBranding), {cx + 60.f, cy + 30.f},  0.55f, m_worldNodes);
        addBtn(m_blurBtn,      "Blur BG: OFF",      menu_selector(NeverloseMenu::onBlur),     {cx + 60.f, cy - 35.f},  0.55f, m_worldNodes);
        addBtn(m_colorAllBtn,  "Color All: Dark",   menu_selector(NeverloseMenu::onColorAll), {cx + 60.f, cy - 100.f}, 0.55f, m_worldNodes);
        addBtn(m_rainbowBtn,   "Rainbow: OFF",      menu_selector(NeverloseMenu::onRainbow),  {cx + 60.f, cy - 165.f}, 0.55f, m_worldNodes);
        addBtn(m_glowCubeBtn,  "Glow Cube: OFF",    menu_selector(NeverloseMenu::onGlowCube), {cx + 60.f, cy - 230.f}, 0.55f, m_worldNodes);
        addBtn(m_cubeScaleBtn, "Cube Size: 1.00",   menu_selector(NeverloseMenu::onCubeSize), {cx + 60.f, cy + 290.f}, 0.55f, m_worldNodes);

        setPage(0);
        refreshButtons();
        applyColorTheme();

        this->setKeypadEnabled(true);
        this->setTouchEnabled(true);
        return true;
    }

    void registerWithTouchDispatcher() override {
        CCDirector::get()->getTouchDispatcher()->addTargetedDelegate(this, -501, true);
    }
    bool ccTouchBegan(CCTouch*, CCEvent*) override { m_dragging = true; return true; }
    void ccTouchMoved(CCTouch* t, CCEvent*) override {
        if (!m_dragging) return;
        this->setPosition(this->getPosition() + t->getDelta());
    }
    void ccTouchEnded(CCTouch*, CCEvent*) override { m_dragging = false; }
    void ccTouchCancelled(CCTouch*, CCEvent*) override { m_dragging = false; }

    void onTabMain(CCObject*)    { setPage(0); }
    void onTabRage(CCObject*)    { setPage(1); }
    void onTabVisuals(CCObject*) { setPage(2); }
    void onTabAntiAim(CCObject*) { setPage(3); }
    void onTabCreate(CCObject*)  { setPage(4); }
    void onTabLegit(CCObject*)   { setPage(5); }
    void onTabQOL(CCObject*)     { setPage(6); }
    void onTabWorld(CCObject*)   { setPage(7); }

    void onJumpHack(CCObject*)  { g_jumpHack = !g_jumpHack; refreshButtons(); }
    void onNoclip(CCObject*)    { g_noclip = !g_noclip; refreshButtons(); }
    void onInst(CCObject*)      { g_instantComplete = !g_instantComplete; refreshButtons(); }
    void onNoSpk(CCObject*)     { g_noSpikes = !g_noSpikes; refreshButtons(); }
    void onGod(CCObject*)       { g_godMode = !g_godMode; refreshButtons(); }
    void onRevGrav(CCObject*)   { g_reverseGravity = !g_reverseGravity; refreshButtons(); }
    void onHideAtt(CCObject*)   { g_hideAttempts = !g_hideAttempts; refreshButtons(); }
    void onLDM(CCObject*)       { g_ldm = !g_ldm; refreshButtons(); }
    void onAutoLDM(CCObject*)   { g_autoLDM = !g_autoLDM; refreshButtons(); }
    void onHitbox(CCObject*)    { refreshButtons(); }
    void onNoWave(CCObject*)    { g_noWaveTrail = !g_noWaveTrail; refreshButtons(); }
    void onSpinbot(CCObject*)   { g_spinbot = !g_spinbot; refreshButtons(); }
    void onShake(CCObject*)     { g_shake = !g_shake; g_shakeTimer=0.f; refreshButtons(); }
    void onNLGrav(CCObject*)    { g_nlGravite = !g_nlGravite; g_nlGravPhase=0.f; refreshButtons(); }
    void onSpeedhack(CCObject*) { g_speedhack = !g_speedhack;
        if (CCDirector::get() && CCDirector::get()->getScheduler())
            CCDirector::get()->getScheduler()->setTimeScale(g_speedhack ? g_speed : 1.0f);
        refreshButtons(); }
    void onAAEnabled(CCObject*) { g_aaEnabled = !g_aaEnabled; refreshButtons(); }
    void onAAFlipX(CCObject*)   { g_aaFlipX = !g_aaFlipX; refreshButtons(); }
    void onAAFlipY(CCObject*)   { g_aaFlipY = !g_aaFlipY; refreshButtons(); }
    void onCopyHack(CCObject*)  { refreshButtons(); }
    void onCheatInd(CCObject*)  { g_cheatIndicator = !g_cheatIndicator; refreshButtons(); }
    void onCPS(CCObject*)       { g_showCPS = !g_showCPS; refreshButtons(); }
    void onTime(CCObject*)      { g_showTime = !g_showTime; refreshButtons(); }
    void onFPS(CCObject*)       { g_showFPS = !g_showFPS; refreshButtons(); }
    void onAutoRetry(CCObject*) { g_autoRetry = !g_autoRetry; refreshButtons(); }
    void onShowPct(CCObject*)   { g_showPercent = !g_showPercent; refreshButtons(); }
    void onHidePl(CCObject*)    { g_hidePlayer = !g_hidePlayer; refreshButtons(); }
    void onSlowMo(CCObject*)    { g_slowMo = !g_slowMo; if (g_slowMo) g_fastMo=false; refreshButtons(); }
    void onFastMo(CCObject*)    { g_fastMo = !g_fastMo; if (g_fastMo) g_slowMo=false; refreshButtons(); }
    void onNoDeath(CCObject*)   { g_noDeathEffect = !g_noDeathEffect; refreshButtons(); }

    void onTheme(CCObject*)    { g_menuTheme = (g_menuTheme + 1) % 6; applyColorTheme(); refreshButtons(); }
    void onOpacity(CCObject*)  { g_menuOpacity = (g_menuOpacity + 1) % 3; applyColorTheme(); refreshButtons(); }
    void onScale(CCObject*)    { g_menuScale = (g_menuScale + 1) % 3; applyColorTheme(); refreshButtons(); }
    void onBranding(CCObject*) { g_hideBranding = !g_hideBranding; applyColorTheme(); refreshButtons(); }
    void onBlur(CCObject*)     { g_blurEffect = !g_blurEffect; applyColorTheme(); refreshButtons(); }
    void onColorAll(CCObject*) { g_menuTheme = (g_menuTheme + 1) % 6; applyColorTheme(); refreshButtons(); }
    void onRainbow(CCObject*)  { g_rainbowMode = !g_rainbowMode; refreshButtons(); }
    void onGlowCube(CCObject*) { g_glowCube = !g_glowCube; refreshButtons(); }
    void onCubeSize(CCObject*) { if (auto p = CubeSizePopup::create()) this->addChild(p, 9); }

    void onSpinUp(CCObject*)   { g_spinSpeed+=1.f; if(g_spinSpeed>500.f)g_spinSpeed=500.f; if(m_spinValueBtn)m_spinValueBtn->setNormalImage(makeNumber(g_spinSpeed)); }
    void onSpinDown(CCObject*) { g_spinSpeed-=1.f; if(g_spinSpeed<1.f)g_spinSpeed=1.f; if(m_spinValueBtn)m_spinValueBtn->setNormalImage(makeNumber(g_spinSpeed)); }
    void onSpinValue(CCObject*){ if (auto p=ValueInputPopup::create(false)) this->addChild(p, 9); }
    void onShUp(CCObject*)     { g_speed+=1.f; if(g_speed>500.f)g_speed=500.f; if(g_speedhack && CCDirector::get() && CCDirector::get()->getScheduler()) CCDirector::get()->getScheduler()->setTimeScale(g_speed); if(m_shValueBtn)m_shValueBtn->setNormalImage(makeNumber(g_speed)); }
    void onShDown(CCObject*)   { g_speed-=1.f; if(g_speed<1.f)g_speed=1.f; if(g_speedhack && CCDirector::get() && CCDirector::get()->getScheduler()) CCDirector::get()->getScheduler()->setTimeScale(g_speed); if(m_shValueBtn)m_shValueBtn->setNormalImage(makeNumber(g_speed)); }
    void onShValue(CCObject*)  { if (auto p=ValueInputPopup::create(true)) this->addChild(p, 9); }

    void update(float dt) override {
        if (g_rainbowMode) {
            g_rainbowTimer += dt;
            if (g_rainbowTimer >= 0.15f) {
                g_rainbowTimer = 0.f;
                g_menuTheme = (g_menuTheme + 1) % 6;
                applyColorTheme();
                refreshButtons();
            }
        }
        if (m_cubeScaleBtn) {
            static float lastVal = -1.f;
            if (lastVal != g_cubeScaleVal) {
                lastVal = g_cubeScaleVal;
                m_cubeScaleBtn->setNormalImage(ButtonSprite::create(
                    CCString::createWithFormat("Cube Size: %.2f", g_cubeScaleVal)->getCString()));
            }
        }
    }

    void onClose(CCObject*) {
        if (m_closing) return;
        m_closing = true;
        this->runAction(CCSequence::create(
            CCEaseBackIn::create(CCScaleTo::create(0.18f, 0.4f)),
            CCCallFunc::create(this, callfunc_selector(NeverloseMenu::removeFromParentAndCleanup)), nullptr));
    }
    void keyBackClicked() { this->onClose(nullptr); }
};

class $modify(NLPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        auto ws = CCDirector::get()->getWinSize();
        s_cpsLabel = CCLabelBMFont::create("CPS: 0", "bigFont.fnt");
        if (s_cpsLabel) { s_cpsLabel->setScale(0.4f); s_cpsLabel->setPosition({60.f, ws.height-20.f}); s_cpsLabel->setColor({0,255,255}); s_cpsLabel->setVisible(false); this->addChild(s_cpsLabel, 100); }
        s_timeLabel = CCLabelBMFont::create("Time: 0.00", "bigFont.fnt");
        if (s_timeLabel) { s_timeLabel->setScale(0.4f); s_timeLabel->setPosition({60.f, ws.height-45.f}); s_timeLabel->setColor({255,255,0}); s_timeLabel->setVisible(false); this->addChild(s_timeLabel, 100); }
        s_fpsLabel = CCLabelBMFont::create("FPS: 60", "bigFont.fnt");
        if (s_fpsLabel) { s_fpsLabel->setScale(0.4f); s_fpsLabel->setPosition({60.f, ws.height-70.f}); s_fpsLabel->setColor({255,100,255}); s_fpsLabel->setVisible(false); this->addChild(s_fpsLabel, 100); }
        s_cheatLabel = CCLabelBMFont::create("[NL.exe]", "bigFont.fnt");
        if (s_cheatLabel) { s_cheatLabel->setScale(0.5f); s_cheatLabel->setPosition({ws.width-60.f, ws.height-20.f}); s_cheatLabel->setColor({255,50,50}); s_cheatLabel->setVisible(false); this->addChild(s_cheatLabel, 100); }
        s_percentLabel = CCLabelBMFont::create("0%", "bigFont.fnt");
        if (s_percentLabel) { s_percentLabel->setScale(0.5f); s_percentLabel->setPosition({ws.width-60.f, ws.height-60.f}); s_percentLabel->setColor({255,255,0}); s_percentLabel->setVisible(false); this->addChild(s_percentLabel, 100); }
        return true;
    }

    void update(float dt) override {
        PlayLayer::update(dt);

        if (s_cpsLabel)   s_cpsLabel->setVisible(g_showCPS);
        if (s_timeLabel)  s_timeLabel->setVisible(g_showTime);
        if (s_fpsLabel)   s_fpsLabel->setVisible(g_showFPS);
        if (s_cheatLabel) s_cheatLabel->setVisible(g_cheatIndicator);
        if (s_percentLabel) s_percentLabel->setVisible(g_showPercent);

        auto player = m_player1;
        if (player) {
            if (g_jumpHack) player->pushButton(PlayerButton::Jump);
            player->setVisible(!g_hidePlayer);

            if (g_instantComplete) {
                float endX = 10000.f;
                if (this->m_level) {
                    endX = this->m_level->m_levelLength * 30.f;
                    if (endX < 5000.f) endX = 5000.f;
                }
                player->setPositionX(endX);
                g_instantComplete = false;
            }
        }

        if (CCDirector::get() && CCDirector::get()->getScheduler()) {
            if (g_slowMo)          CCDirector::get()->getScheduler()->setTimeScale(0.5f);
            else if (g_fastMo)     CCDirector::get()->getScheduler()->setTimeScale(2.0f);
            else if (!g_speedhack) CCDirector::get()->getScheduler()->setTimeScale(1.0f);
        }

        if (s_percentLabel && player) {
            float pct = player->getPositionX() / 10.f;
            if (pct > 100.f) pct = 100.f;
            if (pct < 0.f)   pct = 0.f;
            s_percentLabel->setString(CCString::createWithFormat("%.0f%%", pct)->getCString());
        }

        s_timeAlive += dt; s_frameCount++; s_fpsTimer += dt;
        if (s_fpsTimer >= 0.5f) {
            float fps = s_frameCount / s_fpsTimer;
            if (s_fpsLabel) s_fpsLabel->setString(CCString::createWithFormat("FPS: %.0f", fps)->getCString());
            s_frameCount = 0; s_fpsTimer = 0.f;
        }
        if (s_timeLabel) s_timeLabel->setString(CCString::createWithFormat("Time: %.2f", s_timeAlive)->getCString());
        if (s_cpsLabel) {
            s_cpsTimer += dt;
            if (s_cpsTimer >= 1.f) {
                s_cpsLabel->setString(CCString::createWithFormat("CPS: %d", g_clickCount)->getCString());
                g_clickCount = 0; s_cpsTimer = 0.f;
            }
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* obj) override {
        if (g_noclip || g_godMode || g_noSpikes) return;
        PlayLayer::destroyPlayer(player, obj);
    }

    void onQuit() {
        if (CCDirector::get() && CCDirector::get()->getScheduler())
            CCDirector::get()->getScheduler()->setTimeScale(1.f);
        s_cpsLabel=s_timeLabel=s_fpsLabel=s_cheatLabel=s_percentLabel=nullptr;
        s_timeAlive=0.f; s_frameCount=0; s_fpsTimer=0.f; s_cpsTimer=0.f;
        PlayLayer::onQuit();
    }
};

class $modify(NLPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup
