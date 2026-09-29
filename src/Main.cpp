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

// ============ МЕНЮ ============
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
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    // Обновить надписи на кнопках
    void refreshButtons() {
        m_noclipBtn->setSprite(ButtonSprite::create(
            g_noclip ? "Noclip: ON" : "Noclip: OFF",
            160, true, "bigFont.fnt", "GJ_button_01.png", 35));
        m_autoJumpBtn->setSprite(ButtonSprite::create(
            g_autoJump ? "AutoJump: ON" : "AutoJump: OFF",
            160, true, "bigFont.fnt", "GJ_button_01.png", 35));
        m_speedBtn->setSprite(ButtonSprite::create(
            g_speed == 1.0f ? "Speed: 1.0x" : (g_speed == 2.0f ? "Speed: 2.0x" : "Speed: 0.5x"),
            160, true, "bigFont.fnt", "GJ_button_01.png", 35));
        m_spinbotBtn->setSprite(ButtonSprite::create(
            g_spinbot ? "Spinbot: ON" : "Spinbot: OFF",
            160, true, "bigFont.fnt", "GJ_button_01.png", 35));
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
        m_noclipBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Noclip: OFF", 160, true, "bigFont.fnt", "GJ_button_01.png", 35),
            this, menu_selector(NeverloseMenu::onNoclip)
        );
        m_noclipBtn->setPosition({cx, cy + 110});
        menu->addChild(m_noclipBtn);

        // ===== AUTO JUMP =====
        m_autoJumpBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("AutoJump: OFF", 160, true, "bigFont.fnt", "GJ_button_01.png", 35),
            this, menu_selector(NeverloseMenu::onAutoJump)
        );
        m_autoJumpBtn->setPosition({cx, cy + 60});
        menu->addChild(m_autoJumpBtn);

        // ===== SPEED =====
        m_speedBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Speed: 1.0x", 160, true, "bigFont.fnt", "GJ_button_01.png", 35),
            this, menu_selector(NeverloseMenu::onSpeed)
        );
        m_speedBtn->setPosition({cx, cy + 10});
        menu->addChild(m_speedBtn);

        // ===== SPINBOT =====
        m_spinbotBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Spinbot: OFF", 160, true, "bigFont.fnt", "GJ_button_01.png", 35),
            this, menu_selector(NeverloseMenu::onSpinbot)
        );
        m_spinbotBtn->setPosition({cx, cy - 40});
        menu->addChild(m_spinbotBtn);

        // ===== SPIN SPEED =====
        auto spinSpeedText = CCLabelBMFont::create("Spin Speed:", "bigFont.fnt");
        spinSpeedText->setPosition({cx - 60, cy - 95});
        spinSpeedText->setScale(0.55f);
        this->addChild(spinSpeedText);

        auto leftBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("<", 40, true, "bigFont.fnt", "GJ_button_04.png", 30),
            this, menu_selector(NeverloseMenu::onSpinSpeedDown)
        );
        leftBtn->setPosition({cx + 30, cy - 95});
        menu->addChild(leftBtn);

        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", g_spinSpeed);
        m_spinSpeedLabel = CCLabelBMFont::create(buf, "bigFont.fnt");
        m_spinSpeedLabel->setPosition({cx + 75, cy - 95});
        m_spinSpeedLabel->setScale(0.6f);
        m_spinSpeedLabel->setColor(ccYELLOW);
        this->addChild(m_spinSpeedLabel);

        auto rightBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(">", 40, true, "bigFont.fnt", "GJ_button_04.png", 30),
            this, menu_selector(NeverloseMenu::onSpinSpeedUp)
        );
        rightBtn->setPosition({cx + 120, cy - 95});
        menu->addChild(rightBtn);

        // ===== DISABLE ALL =====
        auto disableBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Disable All", 180, true, "bigFont.fnt", "GJ_button_06.png", 35),
            this, menu_selector(NeverloseMenu::onDisableAll)
        );
        disableBtn->setPosition({cx, cy - 145});
        menu->addChild(disableBtn);

        // ===== CLOSE =====
        auto closeBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Close", 120, true, "bigFont.fnt", "GJ_button_06.png", 30),
            this, menu_selector(NeverloseMenu::onClose)
        );
        closeBtn->setPosition({cx, cy - 185});
        menu->addChild(closeBtn);

        this->setKeypadEnabled(true);
        return true;
    }

    void onNoclip(CCObject*) {
        g_noclip = !g_noclip;
        refreshButtons();
    }

    void onAutoJump(CCObject*) {
        g_autoJump = !g_autoJump;
        refreshButtons();
    }

    void onSpeed(CCObject*) {
        if (g_speed == 1.0f) g_speed = 2.0f;
        else if (g_speed == 2.0f) g_speed = 0.5f;
        else g_speed = 1.0f;

        CCDirector::get()->getScheduler()->setTimeScale(g_speed);
        refreshButtons();
    }

    void onSpinbot(CCObject*) {
        g_spinbot = !g_spinbot;
        refreshButtons();
    }

    void onSpinSpeedUp(CCObject*) {
        g_spinSpeed += 1.0f;
        if (g_spinSpeed > 50.0f) g_spinSpeed = 50.0f;
        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", g_spinSpeed);
        m_spinSpeedLabel->setString(buf);
    }

    void onSpinSpeedDown(CCObject*) {
        g_spinSpeed -= 1.0f;
        if (g_spinSpeed < 1.0f) g_spinSpeed = 1.0f;
        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", g_spinSpeed);
        m_spinSpeedLabel->setString(buf);
    }

    void onDisableAll(CCObject*) {
        g_noclip = false;
        g_autoJump = false;
        g_spinbot = false;
        g_speed = 1.0f;
        g_spinSpeed = 1.0f;

        CCDirector::get()->getScheduler()->setTimeScale(1.0f);

        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", g_spinSpeed);
        m_spinSpeedLabel->setString(buf);

        refreshButtons();
    }

    void onClose(CCObject*) {
        this->removeFromParentAndCleanup(true);
    }

    void keyBackClicked() {
        onClose(nullptr);
    }
};

// ============ ХУК ИГРЫ ============
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
            if (m_player1) {
                m_player1->setRotation(m_player1->getRotation() + g_spinSpeed);
            }
            if (m_player2) {
                m_player2->setRotation(m_player2->getRotation() + g_spinSpeed);
            }
        }
    }

    void onExit() {
        CCDirector::get()->getScheduler()->setTimeScale(1.0f);
        PlayLayer::onExit();
    }
};

// ============ КНОПКА В МЕНЮ ПАУЗЫ ============
class $modify(MyPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto winSize = CCDirector::get()->getWinSize();

        auto btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Neverlose"),
            this,
            menu_selector(MyPauseLayer::onNeverlose)
        );
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
