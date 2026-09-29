#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

static bool g_noclip = false;
static bool g_autoJump = false;
static bool g_spinbot = false;
static float g_speed = 1.0f;
static float g_spinSpeed = 1.0f;

class NeverloseMenu : public CCLayer {
protected:
    CCLabelBMFont* m_noclipLabel = nullptr;
    CCLabelBMFont* m_autoJumpLabel = nullptr;
    CCLabelBMFont* m_speedLabel = nullptr;
    CCLabelBMFont* m_spinbotLabel = nullptr;
    CCLabelBMFont* m_spinSpeedLabel = nullptr;

public:
    static NeverloseMenu* create() {
        auto ret = new NeverloseMenu();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    CCLabelBMFont* makeStateLabel(bool state, CCPoint pos) {
        auto lbl = CCLabelBMFont::create(state ? "ON" : "OFF", "bigFont.fnt");
        lbl->setPosition(pos);
        lbl->setScale(0.6f);
        lbl->setColor(state ? ccGREEN : ccRED);
        return lbl;
    }

    bool init() {
        if (!CCLayer::init()) return false;

        auto winSize = CCDirector::get()->getWinSize();

        auto overlay = CCLayerColor::create({0, 0, 0, 180});
        this->addChild(overlay, -1);

        auto panel = CCScale9Sprite::create("GJ_square01.png");
        panel->setContentSize({420, 420});
        panel->setPosition(winSize / 2);
        this->addChild(panel);

        auto title = CCLabelBMFont::create("Neverlose | Bulat", "goldFont.fnt");
        title->setPosition({winSize.width / 2, winSize.height / 2 + 170});
        title->setScale(0.8f);
        this->addChild(title);

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        this->addChild(menu);

        float cx = winSize.width / 2;
        float cy = winSize.height / 2;

        // ===== NOCLIP =====
        auto noclipBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Noclip"),
            this, menu_selector(NeverloseMenu::onNoclip));
        noclipBtn->setPosition({cx - 70, cy + 110});
        menu->addChild(noclipBtn);

        m_noclipLabel = makeStateLabel(g_noclip, {cx + 70, cy + 110});
        this->addChild(m_noclipLabel);

        // ===== AUTO JUMP =====
        auto ajBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("AutoJump"),
            this, menu_selector(NeverloseMenu::onAutoJump));
        ajBtn->setPosition({cx - 70, cy + 60});
        menu->addChild(ajBtn);

        m_autoJumpLabel = makeStateLabel(g_autoJump, {cx + 70, cy + 60});
        this->addChild(m_autoJumpLabel);

        // ===== SPEED =====
        auto speedBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Speed"),
            this, menu_selector(NeverloseMenu::onSpeed));
        speedBtn->setPosition({cx - 70, cy + 10});
        menu->addChild(speedBtn);

        m_speedLabel = CCLabelBMFont::create("1.0x", "bigFont.fnt");
        m_speedLabel->setPosition({cx + 70, cy + 10});
        m_speedLabel->setScale(0.6f);
        m_speedLabel->setColor(ccWHITE);
        this->addChild(m_speedLabel);

        // ===== SPINBOT =====
        auto spinBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Spinbot"),
            this, menu_selector(NeverloseMenu::onSpinbot));
        spinBtn->setPosition({cx - 70, cy - 40});
        menu->addChild(spinBtn);

        m_spinbotLabel = makeStateLabel(g_spinbot, {cx + 70, cy - 40});
        this->addChild(m_spinbotLabel);

        // ===== SPIN SPEED =====
        auto spinSpeedText = CCLabelBMFont::create("Spin Speed:", "bigFont.fnt");
        spinSpeedText->setPosition({cx - 90, cy - 95});
        spinSpeedText->setScale(0.5f);
        this->addChild(spinSpeedText);

        auto leftBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("<"),
            this, menu_selector(NeverloseMenu::onSpinSpeedDown));
        leftBtn->setPosition({cx + 20, cy - 95});
        menu->addChild(leftBtn);

        m_spinSpeedLabel = CCLabelBMFont::create("1.00", "bigFont.fnt");
        m_spinSpeedLabel->setPosition({cx + 80, cy - 95});
        m_spinSpeedLabel->setScale(0.6f);
        m_spinSpeedLabel->setColor(ccYELLOW);
        this->addChild(m_spinSpeedLabel);

        auto rightBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(">"),
            this, menu_selector(NeverloseMenu::onSpinSpeedUp));
        rightBtn->setPosition({cx + 140, cy - 95});
        menu->addChild(rightBtn);

        // ===== DISABLE ALL =====
        auto disableBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Disable All"),
            this, menu_selector(NeverloseMenu::onDisableAll));
        disableBtn->setPosition({cx, cy - 150});
        menu->addChild(disableBtn);

        // ===== CLOSE =====
        auto closeBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Close"),
            this, menu_selector(NeverloseMenu::onClose));
        closeBtn->setPosition({cx, cy - 195});
        menu->addChild(closeBtn);

        this->setKeypadEnabled(true);
        return true;
    }

    void setState(CCLabelBMFont* lbl, bool state) {
        lbl->setString(state ? "ON" : "OFF");
        lbl->setColor(state ? ccGREEN : ccRED);
    }

    void onNoclip(CCObject*) {
        g_noclip = !g_noclip;
        setState(m_noclipLabel, g_noclip);
    }

    void onAutoJump(CCObject*) {
        g_autoJump = !g_autoJump;
        setState(m_autoJumpLabel, g_autoJump);
    }

    void onSpeed(CCObject*) {
        if (g_speed == 1.0f) { g_speed = 2.0f; m_speedLabel->setString("2.0x"); }
        else if (g_speed == 2.0f) { g_speed = 0.5f; m_speedLabel->setString("0.5x"); }
        else { g_speed = 1.0f; m_speedLabel->setString("1.0x"); }

        CCDirector::get()->getScheduler()->setTimeScale(g_speed);
    }

    void onSpinbot(CCObject*) {
        g_spinbot = !g_spinbot;
        setState(m_spinbotLabel, g_spinbot);
    }

    void onSpinSpeedUp(CCObject*) {
        g_spinSpeed += 1.0f;
        if (g_spinSpeed > 50.0f) g_spinSpeed = 50.0f;
        auto str = CCString::createWithFormat("%.2f", g_spinSpeed);
        m_spinSpeedLabel->setString(str->getCString());
    }

    void onSpinSpeedDown(CCObject*) {
        g_spinSpeed -= 1.0f;
        if (g_spinSpeed < 1.0f) g_spinSpeed = 1.0f;
        auto str = CCString::createWithFormat("%.2f", g_spinSpeed);
        m_spinSpeedLabel->setString(str->getCString());
    }

    void onDisableAll(CCObject*) {
        g_noclip = false;
        g_autoJump = false;
        g_spinbot = false;
        g_speed = 1.0f;
        g_spinSpeed = 1.0f;

        CCDirector::get()->getScheduler()->setTimeScale(1.0f);

        setState(m_noclipLabel, false);
        setState(m_autoJumpLabel, false);
        setState(m_spinbotLabel, false);
        m_speedLabel->setString("1.0x");
        m_spinSpeedLabel->setString("1.00");
    }

    void onClose(CCObject*) {
        this->removeFromParentAndCleanup(true);
    }

    void keyBackClicked() {
        onClose(nullptr);
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    void update(float dt) {
        PlayLayer::update(dt);
        if (g_noclip) {
            if (m_player1) m_player1->m_isDead = false;
            if (m_player2) m_player2->m_isDead = false;
        }
        if (g_autoJump) {
            if (m_player1) m_player1->pushButton(PlayerButton::Jump);
        }
        if (g_spinbot) {
            if (m_player1) m_player1->setRotation(m_player1->getRotation() + g_spinSpeed);
            if (m_player2) m_player2->setRotation(m_player2->getRotation() + g_spinSpeed);
        }
    }
    void onExit() {
        CCDirector::get()->getScheduler()->setTimeScale(1.0f);
        PlayLayer::onExit();
    }
};

class $modify(MyPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        auto winSize = CCDirector::get()->getWinSize();
        auto btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Neverlose"),
            this, menu_selector(MyPauseLayer::onNeverlose));
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
