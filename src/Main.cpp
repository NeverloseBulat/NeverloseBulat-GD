#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

static bool g_noclip = false;
static bool g_autoJump = false;
static bool g_spinbot = false;
static float g_speed = 1.0f;
static float g_spinSpeed = 5.0f;

class NeverloseMenu : public CCLayer {
protected:
    CCMenuItemSpriteExtra* m_noclipBtn = nullptr;
    CCMenuItemSpriteExtra* m_autoJumpBtn = nullptr;
    CCMenuItemSpriteExtra* m_speedBtn = nullptr;
    CCMenuItemSpriteExtra* m_spinbotBtn = nullptr;
    CCLabelBMFont* m_spinSpeedLabel = nullptr;

public:
    static NeverloseMenu* create() {
        auto ret = new NeverloseMenu();
        if (ret && ret->init()) { ret->autorelease(); return ret; }
        delete ret; return nullptr;
    }

    void refreshButtons() {
        m_noclipBtn->setNormalImage(ButtonSprite::create(
            g_noclip ? "Noclip: ON" : "Noclip: OFF"));
        m_autoJumpBtn->setNormalImage(ButtonSprite::create(
            g_autoJump ? "AutoJump: ON" : "AutoJump: OFF"));
        m_speedBtn->setNormalImage(ButtonSprite::create(
            g_speed == 1.0f ? "Speed: 1.0x" : (g_speed == 2.0f ? "Speed: 2.0x" : "Speed: 0.5x")));
        m_spinbotBtn->setNormalImage(ButtonSprite::create(
            g_spinbot ? "Spinbot: ON" : "Spinbot: OFF"));
    }

    bool init() {
        if (!CCLayer::init()) return false;
        auto winSize = CCDirector::get()->getWinSize();

        auto overlay = CCLayerColor::create({0, 0, 0, 180});
        this->addChild(overlay, -1);

        auto panel = CCScale9Sprite::create("GJ_square01.png");
        panel->setContentSize({420, 420});
        panel->setPosition(winSize / 2);
        panel->setColor({0, 0, 0});
        this->addChild(panel);

        auto title = CCLabelBMFont::create("Neverlose | Bulat", "goldFont.fnt");
        title->setPosition({winSize.width / 2, winSize.height / 2 + 170});
        title->setScale(0.8f);
        title->setColor({0, 200, 255});
        this->addChild(title);

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        this->addChild(menu);

        float cx = winSize.width / 2;
        float cy = winSize.height / 2;

        auto closeSprite = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        closeSprite->setScale(0.8f);
        auto closeBtn = CCMenuItemSpriteExtra::create(
            closeSprite, this, menu_selector(NeverloseMenu::onClose));
        closeBtn->setPosition({cx + 190, cy + 185});
        menu->addChild(closeBtn);

        m_noclipBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Noclip: OFF"), this, menu_selector(NeverloseMenu::onNoclip));
        m_noclipBtn->setPosition({cx, cy + 110});
        menu->addChild(m_noclipBtn);

        m_autoJumpBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("AutoJump: OFF"), this, menu_selector(NeverloseMenu::onAutoJump));
        m_autoJumpBtn->setPosition({cx, cy + 60});
        menu->addChild(m_autoJumpBtn);

        m_speedBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Speed: 1.0x"), this, menu_selector(NeverloseMenu::onSpeed));
        m_speedBtn->setPosition({cx, cy + 10});
        menu->addChild(m_speedBtn);

        m_spinbotBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Spinbot: OFF"), this, menu_selector(NeverloseMenu::onSpinbot));
        m_spinbotBtn->setPosition({cx, cy - 40});
        menu->addChild(m_spinbotBtn);

        auto spinSpeedText = CCLabelBMFont::create("Spin Speed:", "bigFont.fnt");
        spinSpeedText->setPosition({cx - 60, cy - 95});
        spinSpeedText->setScale(0.5f);
        spinSpeedText->setColor({0, 200, 255});
        this->addChild(spinSpeedText);

        auto leftBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("<"), this, menu_selector(NeverloseMenu::onSpinSpeedDown));
        leftBtn->setPosition({cx + 30, cy - 95});
        menu->addChild(leftBtn);

        m_spinSpeedLabel = CCLabelBMFont::create("5.00", "bigFont.fnt");
        m_spinSpeedLabel->setPosition({cx + 75, cy - 95});
        m_spinSpeedLabel->setScale(0.6f);
        m_spinSpeedLabel->setColor({0, 200, 255});
        this->addChild(m_spinSpeedLabel);

        auto rightBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(">"), this, menu_selector(NeverloseMenu::onSpinSpeedUp));
        rightBtn->setPosition({cx + 120, cy - 95});
        menu->addChild(rightBtn);

        auto disableBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Disable All"), this, menu_selector(NeverloseMenu::onDisableAll));
        disableBtn->setPosition({cx, cy - 150});
        menu->addChild(disableBtn);

        // Анимация: только масштаб (без opacity)
        this->setScale(0.3f);
        auto scaleUp = CCEaseBackOut::create(CCScaleTo::create(0.35f, 1.0f));
        this->runAction(scaleUp);

        this->setKeypadEnabled(true);
        return true;
    }

    void onNoclip(CCObject*) { g_noclip = !g_noclip; refreshButtons(); }
    void onAutoJump(CCObject*) { g_autoJump = !g_autoJump; refreshButtons(); }
    void onSpeed(CCObject*) {
        if (g_speed == 1.0f) g_speed = 2.0f;
        else if (g_speed == 2.0f) g_speed = 0.5f;
        else g_speed = 1.0f;
        CCDirector::get()->getScheduler()->setTimeScale(g_speed);
        refreshButtons();
    }
    void onSpinbot(CCObject*) { g_spinbot = !g_spinbot; refreshButtons(); }
    void onSpinSpeedUp(CCObject*) {
        g_spinSpeed += 1.0f;
        if (g_spinSpeed > 50.0f) g_spinSpeed = 50.0f;
        m_spinSpeedLabel->setString(CCString::createWithFormat("%.2f", g_spinSpeed)->getCString());
    }
    void onSpinSpeedDown(CCObject*) {
        g_spinSpeed -= 1.0f;
        if (g_spinSpeed < 1.0f) g_spinSpeed = 1.0f;
        m_spinSpeedLabel->setString(CCString::createWithFormat("%.2f", g_spinSpeed)->getCString());
    }
    void onDisableAll(CCObject*) {
        g_noclip = false; g_autoJump = false; g_spinbot = false;
        g_speed = 1.0f; g_spinSpeed = 5.0f;
        CCDirector::get()->getScheduler()->setTimeScale(1.0f);
        m_spinSpeedLabel->setString("5.00");
        refreshButtons();
    }

    void onClose(CCObject*) {
        auto scaleDown = CCEaseBackIn::create(CCScaleTo::create(0.2f, 0.3f));
        auto callback = CCCallFunc::create(this, callfunc_selector(NeverloseMenu::removeMe));
        this->runAction(CCSequence::create(scaleDown, callback, nullptr));
    }

    void removeMe() { this->removeFromParentAndCleanup(true); }
    void keyBackClicked() { onClose(nullptr); }
};

class $modify(MyPlayLayer, PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        if (g_noclip) return;
        PlayLayer::destroyPlayer(player, obj);
    }
    void update(float dt) {
        PlayLayer::update(dt);
        if (g_autoJump && m_player1) m_player1->pushButton(PlayerButton::Jump);
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

class $modify(MyPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        auto winSize = CCDirector::get()->getWinSize();
        auto btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Neverlose"), this, menu_selector(MyPauseLayer::onNeverlose));
        btn->setPosition({winSize.width * 0.25f, winSize.height * 0.72f});
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
