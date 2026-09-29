#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/ShareLevelLayer.hpp>
#include <Geode/modify/SliderTouchLogic.hpp>
#include <Geode/modify/GJScaleControl.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/modify/HardStreak.hpp>
#include <Geode/modify/GameStatsManager.hpp>

using namespace geode::prelude;

static bool g_noclip = false;
static bool g_fly = false;
static bool g_autoJump = false;
static bool g_spinbot = false;
static bool g_spikeESP = false;
static bool g_ldm = false;
static bool g_autoLDM = false;
static bool g_speedhack = false;
static bool g_copyHack = false;
static bool g_verifyHack = false;
static bool g_sliderBypass = false;
static bool g_toolboxBypass = false;
static bool g_levelEdit = false;
static bool g_resetPercent = false;
static bool g_waveTrail = false;
static bool g_smoothTrail = false;
static bool g_showHitbox = false;
static bool g_hideUI = false;
static bool g_noObjLimit = false;
static bool g_jumpHack = false;
static bool g_autoCoins = false;
static bool g_autoQuest = false;
static float g_speed = 1.0f;
static float g_spinSpeed = 5.0f;
static bool g_touchHeld = false;

// ================= МЕНЮ =================
class NeverloseMenu : public CCLayer {
protected:
    std::vector<CCNode*> m_mainNodes, m_rageNodes, m_visualsNodes, m_antiAimNodes, m_createNodes;

    CCMenuItemSpriteExtra* m_jumpHackBtn = nullptr;

    CCMenuItemSpriteExtra *m_noclipBtn, *m_flyBtn, *m_spikeESPBtn, *m_autoJumpBtn;
    CCMenuItemSpriteExtra *m_autoCoinsBtn, *m_autoQuestBtn;
    CCMenuItemSpriteExtra *m_ldmBtn, *m_autoLDMBtn, *m_hitboxBtn;
    CCMenuItemSpriteExtra *m_spinbotBtn, *m_spinDown, *m_spinUp, *m_speedhackBtn, *m_shDown, *m_shUp;
    CCLabelBMFont *m_spinLabel, *m_spinText, *m_shLabel, *m_shText;
    CCMenuItemSpriteExtra *m_copyHackBtn, *m_customStarBtn, *m_verifyHackBtn, *m_sliderBypassBtn;
    CCMenuItemSpriteExtra *m_toolboxBypassBtn, *m_levelEditBtn, *m_resetPercentBtn;
    CCMenuItemSpriteExtra *m_waveTrailBtn, *m_smoothTrailBtn, *m_hideUIBtn, *m_noObjLimitBtn;
    CCMenuItemSpriteExtra *m_noZoomLimitBtn;

    CCMenuItemSpriteExtra *m_tabMain, *m_tabRage, *m_tabVisuals, *m_tabAntiAim, *m_tabCreate;

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
        m_autoCoinsBtn->setNormalImage(ButtonSprite::create(g_autoCoins ? "Auto Coins: ON" : "Auto Coins: OFF"));
        m_autoQuestBtn->setNormalImage(ButtonSprite::create(g_autoQuest ? "Auto Quest: ON" : "Auto Quest: OFF"));
        m_ldmBtn->setNormalImage(ButtonSprite::create(g_ldm ? "LDM: ON" : "LDM: OFF"));
        m_autoLDMBtn->setNormalImage(ButtonSprite::create(g_autoLDM ? "Auto LDM: ON" : "Auto LDM: OFF"));
        m_hitboxBtn->setNormalImage(ButtonSprite::create(g_showHitbox ? "Hitbox: ON" : "Hitbox: OFF"));
        m_spinbotBtn->setNormalImage(ButtonSprite::create(g_spinbot ? "Spinbot: ON" : "Spinbot: OFF"));
        m_speedhackBtn->setNormalImage(ButtonSprite::create(g_speedhack ? "Speedhack: ON" : "Speedhack: OFF"));
        m_shLabel->setString(CCString::createWithFormat("%.3f", g_speed)->getCString());
        m_copyHackBtn->setNormalImage(ButtonSprite::create(g_copyHack ? "Copy Hack: ON" : "Copy Hack: OFF"));
        m_customStarBtn->setNormalImage(ButtonSprite::create("Custom Star: OFF"));
        m_verifyHackBtn->setNormalImage(ButtonSprite::create(g_verifyHack ? "Verify Hack: ON" : "Verify Hack: OFF"));
        m_sliderBypassBtn->setNormalImage(ButtonSprite::create(g_sliderBypass ? "Slider Bypass: ON" : "Slider Bypass: OFF"));
        m_toolboxBypassBtn->setNormalImage(ButtonSprite::create(g_toolboxBypass ? "Toolbox Bypass: ON" : "Toolbox Bypass: OFF"));
        m_noZoomLimitBtn->setNormalImage(ButtonSprite::create("No Zoom Limit: OFF"));
        m_levelEditBtn->setNormalImage(ButtonSprite::create(g_levelEdit ? "Level Edit: ON" : "Level Edit: OFF"));
        m_resetPercentBtn->setNormalImage(ButtonSprite::create(g_resetPercent ? "Reset %: ON" : "Reset %: OFF"));
        m_waveTrailBtn->setNormalImage(ButtonSprite::create(g_waveTrail ? "Wave Trail: ON" : "Wave Trail: OFF"));
        m_smoothTrailBtn->setNormalImage(ButtonSprite::create(g_smoothTrail ? "Smooth Trail: ON" : "Smooth Trail: OFF"));
        m_hideUIBtn->setNormalImage(ButtonSprite::create(g_hideUI ? "Hide UI: ON" : "Hide UI: OFF"));
        m_noObjLimitBtn->setNormalImage(ButtonSprite::create(g_noObjLimit ? "No Obj Limit: ON" : "No Obj Limit: OFF"));
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

        // ===== КРЕСТИК — середина правой стороны =====
        auto cs = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        cs->setScale(0.8f);
        auto closeBtn = CCMenuItemSpriteExtra::create(cs, this, menu_selector(NeverloseMenu::onClose));
        closeBtn->setPosition({cx + 350, cy});
        menu->addChild(closeBtn);

        // Tabs
        float tabY[5] = {cy + 140, cy + 70, cy, cy - 70, cy - 140};
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
        m_jumpHackBtn->setPosition({cx + 60, cy + 100});
        menu->addChild(m_jumpHackBtn); m_mainNodes.push_back(m_jumpHackBtn);

        // RAGE
        m_noclipBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Noclip: OFF"), this, menu_selector(NeverloseMenu::onNoclip));
        m_noclipBtn->setPosition({cx + 60, cy + 160});
        menu->addChild(m_noclipBtn); m_rageNodes.push_back(m_noclipBtn);

        m_flyBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Fly: OFF"), this, menu_selector(NeverloseMenu::onFly));
        m_flyBtn->setPosition({cx + 60, cy + 100});
        menu->addChild(m_flyBtn); m_rageNodes.push_back(m_flyBtn);

        m_spikeESPBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("ESP Spikes: OFF"), this, menu_selector(NeverloseMenu::onSpikeESP));
        m_spikeESPBtn->setPosition({cx + 60, cy + 40});
        menu->addChild(m_spikeESPBtn); m_rageNodes.push_back(m_spikeESPBtn);

        m_autoJumpBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("AutoJump: OFF"), this, menu_selector(NeverloseMenu::onAutoJump));
        m_autoJumpBtn->setPosition({cx + 60, cy - 20});
        menu->addChild(m_autoJumpBtn); m_rageNodes.push_back(m_autoJumpBtn);

        m_autoCoinsBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Auto Coins: OFF"), this, menu_selector(NeverloseMenu::onAutoCoins));
        m_autoCoinsBtn->setPosition({cx + 60, cy - 80});
        menu->addChild(m_autoCoinsBtn); m_rageNodes.push_back(m_autoCoinsBtn);

        m_autoQuestBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Auto Quest: OFF"), this, menu_selector(NeverloseMenu::onAutoQuest));
        m_autoQuestBtn->setPosition({cx + 60, cy - 140});
        menu->addChild(m_autoQuestBtn); m_rageNodes.push_back(m_autoQuestBtn);

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

        // CREATE
        float col1 = cx - 60, col2 = cx + 120;
        float rowY[6] = {cy + 160, cy + 100, cy + 40, cy - 20, cy - 80, cy - 140};

        m_copyHackBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Copy Hack: OFF"), this, menu_selector(NeverloseMenu::onCopyHack));
        m_copyHackBtn->setPosition({col1, rowY[0]});
        menu->addChild(m_copyHackBtn); m_createNodes.push_back(m_copyHackBtn);

        m_customStarBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Custom Star: OFF"), this, menu_selector(NeverloseMenu::onCustomStar));
        m_customStarBtn->setPosition({col1, rowY[1]});
        menu->addChild(m_customStarBtn); m_createNodes.push_back(m_customStarBtn);

        m_hideUIBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Hide UI: OFF"), this, menu_selector(NeverloseMenu::onHideEditorUI));
        m_hideUIBtn->setPosition({col1, rowY[2]});
        menu->addChild(m_hideUIBtn); m_createNodes.push_back(m_hideUIBtn);

        m_noZoomLimitBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("No Zoom Limit: OFF"), this, menu_selector(NeverloseMenu::onNoZoomLimit));
        m_noZoomLimitBtn->setPosition({col1, rowY[3]});
        menu->addChild(m_noZoomLimitBtn); m_createNodes.push_back(m_noZoomLimitBtn);

        m_sliderBypassBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Slider Bypass: OFF"), this, menu_selector(NeverloseMenu::onSliderBypass));
        m_sliderBypassBtn->setPosition({col1, rowY[4]});
        menu->addChild(m_sliderBypassBtn); m_createNodes.push_back(m_sliderBypassBtn);

        m_toolboxBypassBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Toolbox Bypass: OFF"), this, menu_selector(NeverloseMenu::onToolboxBypass));
        m_toolboxBypassBtn->setPosition({col1, rowY[5]});
        menu->addChild(m_toolboxBypassBtn); m_createNodes.push_back(m_toolboxBypassBtn);

        m_noObjLimitBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("No Obj Limit: OFF"), this, menu_selector(NeverloseMenu::onNoCustomObjLimit));
        m_noObjLimitBtn->setPosition({col2, rowY[0]});
        menu->addChild(m_noObjLimitBtn); m_createNodes.push_back(m_noObjLimitBtn);

        m_waveTrailBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Wave Trail: OFF"), this, menu_selector(NeverloseMenu::onWaveTrailEditor));
        m_waveTrailBtn->setPosition({col2, rowY[1]});
        menu->addChild(m_waveTrailBtn); m_createNodes.push_back(m_waveTrailBtn);

        m_levelEditBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Level Edit: OFF"), this, menu_selector(NeverloseMenu::onLevelEdit));
        m_levelEditBtn->setPosition({col2, rowY[2]});
        menu->addChild(m_levelEditBtn); m_createNodes.push_back(m_levelEditBtn);

        m_resetPercentBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Reset %: OFF"), this, menu_selector(NeverloseMenu::onResetPercentOnSave));
        m_resetPercentBtn->setPosition({col2, rowY[3]});
        menu->addChild(m_resetPercentBtn); m_createNodes.push_back(m_resetPercentBtn);

        m_smoothTrailBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Smooth Trail: OFF"), this, menu_selector(NeverloseMenu::onSmoothEditorTrail));
        m_smoothTrailBtn->setPosition({col2, rowY[4]});
        menu->addChild(m_smoothTrailBtn); m_createNodes.push_back(m_smoothTrailBtn);

        m_verifyHackBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Verify Hack: OFF"), this, menu_selector(NeverloseMenu::onVerifyHack));
        m_verifyHackBtn->setPosition({col2, rowY[5]});
        menu->addChild(m_verifyHackBtn); m_createNodes.push_back(m_verifyHackBtn);

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
    void onNoclip(CCObject*) { g_noclip = !g_noclip; refreshButtons(); }
    void onFly(CCObject*) { g_fly = !g_fly; refreshButtons(); }
    void onSpikeESP(CCObject*) { g_spikeESP = !g_spikeESP; refreshButtons(); }
    void onAutoJump(CCObject*) { g_autoJump = !g_autoJump; refreshButtons(); }
    void onAutoCoins(CCObject*) { g_autoCoins = !g_autoCoins; refreshButtons(); }
    void onAutoQuest(CCObject*) { g_autoQuest = !g_autoQuest; refreshButtons(); }

    void onLDM(CCObject*) { g_ldm = !g_ldm; GameManager::get()->setGameVariable("low_detail_mode", g_ldm); refreshButtons(); }
    void onAutoLDM(CCObject*) { g_autoLDM = !g_autoLDM; GameManager::get()->setGameVariable("low_detail_mode", g_autoLDM); refreshButtons(); }
    void onHitbox(CCObject*) { g_showHitbox = !g_showHitbox; refreshButtons(); }

    void onSpinbot(CCObject*) { g_spinbot = !g_spinbot; refreshButtons(); }
    void onSpinUp(CCObject*) { g_spinSpeed += 1.0f; if (g_spinSpeed > 100.0f) g_spinSpeed = 100.0f; m_spinLabel->setString(CCString::createWithFormat("%.2f", g_spinSpeed)->getCString()); }
    void onSpinDown(CCObject*) { g_spinSpeed -= 1.0f; if (g_spinSpeed < 1.0f) g_spinSpeed = 1.0f; m_spinLabel->setString(CCString::createWithFormat("%.2f", g_spinSpeed)->getCString()); }
    void onSpeedhack(CCObject*) { g_speedhack = !g_speedhack; if (g_speedhack) CCDirector::get()->getScheduler()->setTimeScale(g_speed); else CCDirector::get()->getScheduler()->setTimeScale(1.0f); refreshButtons(); }
    void onSpeedhackUp(CCObject*) { g_speed += 0.5f; if (g_speed > 10.0f) g_speed = 10.0f; if (g_speedhack) CCDirector::get()->getScheduler()->setTimeScale(g_speed); refreshButtons(); }
    void onSpeedhackDown(CCObject*) { g_speed -= 0.5f; if (g_speed < 0.0f) g_speed = 0.0f; if (g_speedhack) CCDirector::get()->
