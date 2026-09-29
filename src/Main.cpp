#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

static bool g_noclip = false;
static bool g_autoJump = false;
static float g_speed = 1.0f;

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
    }

    void onExit() {
        CCDirector::get()->getScheduler()->setTimeScale(1.0f);
        PlayLayer::onExit();
    }

    void keyDown(cocos2d::enumKeyCodes key, double timestamp) {
        if (key == cocos2d::KEY_N) {
            g_noclip = !g_noclip;
            Notification::create(g_noclip ? "Noclip: ON" : "Noclip: OFF", NotificationIcon::Success)->show();
        }
        if (key == cocos2d::KEY_J) {
            g_autoJump = !g_autoJump;
            Notification::create(g_autoJump ? "Auto Jump: ON" : "Auto Jump: OFF", NotificationIcon::Success)->show();
        }
        if (key == cocos2d::KEY_S) {
            if (g_speed == 1.0f) g_speed = 2.0f;
            else if (g_speed == 2.0f) g_speed = 0.5f;
            else g_speed = 1.0f;
            
            CCDirector::get()->getScheduler()->setTimeScale(g_speed);
            Notification::create("Speed: " + std::to_string(g_speed) + "x", NotificationIcon::Success)->show();
        }
        
        PlayLayer::keyDown(key, timestamp);
    }
};

class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto winSize = CCDirector::get()->getWinSize();

        auto btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Neverlose"),
            this,
            menu_selector(MyMenuLayer::onMenuBtn)
        );
        btn->setPosition({0, -120});

        auto menu = CCMenu::create();
        menu->addChild(btn);
        menu->setPosition({winSize.width / 2, winSize.height / 2});
        this->addChild(menu);

        return true;
    }

    void onMenuBtn(CCObject*) {
        auto popup = FLAlertLayer::create(
            "Neverlose | Bulat",
            "Noclip: N\nAuto Jump: J\nSpeedhack: S",
            "OK"
        );
        popup->show();
    }
};
