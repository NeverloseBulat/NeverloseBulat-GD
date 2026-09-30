#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <cstdlib>
#include <ctime>
using namespace geode::prelude;
static bool g_noclip=false,g_autoJump=false,g_spinbot=false,g_shake=false,g_nlGravite=false,g_ldm=false,g_autoLDM=false,g_speedhack=false,g_jumpHack=false,g_copyHack=false,g_autoSafeMode=false,g_aaEnabled=false,g_aaFlipX=false,g_aaFlipY=false,g_cheatIndicator=false,g_showCPS=false,g_showTime=false,g_showFPS=false,g_unlockIcons=false,g_unlockVault=false,g_unlockColors=false,g_unlockLevels=false,g_isHolding=false;
static float g_speed=1.0f,g_spinSpeed=5.0f,g_shakeTimer=0.0f,g_spinPhase=0.0f;
static int g_clickCount=0;

class ValueInputPopup : public CCLayer {
protected: bool m_isSpeed=true;
public:
    static ValueInputPopup* create(bool s){auto r=new ValueInputPopup();r->m_isSpeed=s;if(r&&r->init()){r->autorelease();return r;}delete r;return nullptr;}
    bool init(){
        if(!CCLayer::init())return false;
        auto ws=CCDirector::get()->getWinSize();
        auto bg=CCLayerColor::create({0,0,0,200});this->addChild(bg,-1);
        auto p=CCScale9Sprite::create("GJ_square01.png");p->setContentSize({520,260});p->setPosition(ws/2);this->addChild(p);
        auto t=CCLabelBMFont::create(m_isSpeed?"Speedhack Value":"Spinbot Speed","goldFont.fnt");
        t->setPosition({ws.width/2,ws.height/2+90});t->setScale(0.7f);t->setColor({0,200,255});this->addChild(t);
        auto v=CCLabelBMFont::create(CCString::createWithFormat("%.2f",m_isSpeed?g_speed:g_spinSpeed)->getCString(),"goldFont.fnt");
        v->setPosition({ws.width/2,ws.height/2+30});v->setScale(0.9f);v->setColor({255,255,255});v->setID("value-label");this->addChild(v);
        auto m=CCMenu::create();m->setPosition({0,0});this->addChild(m);
        float cx=ws.width/2,cy=ws.height/2;
        auto mk=[&](const char* tt,cocos2d::SEL_MenuHandler cb,float x,float y){auto b=CCMenuItemSpriteExtra::create(ButtonSprite::create(tt),this,cb);b->setPosition({x,y});b->setScale(0.7f);m->addChild(b);};
        mk("-500",menu_selector(ValueInputPopup::onM500),cx-200,cy-30);
        mk("-1",menu_selector(ValueInputPopup::onM1),cx-70,cy-30);
        mk("+1",menu_selector(ValueInputPopup::onP1),cx+70,cy-30);
        mk("+500",menu_selector(ValueInputPopup::onP500),cx+200,cy-30);
        mk("OK",menu_selector(ValueInputPopup::onCancel),cx,cy-110);
        return true;
    }
    void ref(){auto l=this->getChildByID("value-label");if(l){auto x=typeinfo_cast<CCLabelBMFont*>(l);if(x)x->setString(CCString::createWithFormat("%.2f",m_isSpeed?g_speed:g_spinSpeed)->getCString());}}
    void ap(){if(g_speedhack)CCDirector::get()->getScheduler()->setTimeScale(g_speed);}
    void onM500(CCObject*){if(m_isSpeed){g_speed-=500;if(g_speed<1)g_speed=1;ap();}else{g_spinSpeed-=500;if(g_spinSpeed<1)g_spinSpeed=1;}ref();}
    void onM1(CCObject*){if(m_isSpeed){g_speed-=1;if(g_speed<1)g_speed=1;ap();}else{g_spinSpeed-=1;if(g_spinSpeed<1)g_spinSpeed=1;}ref();}
    void onP1(CCObject*){if(m_isSpeed){g_speed+=1;if(g_speed>500)g_speed=500;ap();}else{g_spinSpeed+=1;if(g_spinSpeed>500)g_spinSpeed=500;}ref();}
    void onP500(CCObject*){if(m_isSpeed){g_speed+=500;if(g_speed>500)g_speed=500;ap();}else{g_spinSpeed+=500;if(g_spinSpeed>500)g_spinSpeed=500;}ref();}
    void onCancel(CCObject*){this->removeFromParentAndCleanup(true);}
    void keyBackClicked(){this->removeFromParentAndCleanup(true);}
};

class NeverloseMenu : public CCLayer {
protected:
    std::vector<CCNode*> m_mainNodes,m_rageNodes,m_visualsNodes,m_antiAimNodes,m_createNodes,m_legitNodes,m_cosmeticsNodes;
    CCMenuItemSpriteExtra *m_jumpHackBtn,*m_copyHackBtn,*m_autoSafeModeBtn,*m_cheatIndBtn,*m_cpsBtn,*m_timeBtn,*m_fpsBtn,*m_unlockIconsBtn,*m_unlockVaultBtn,*m_unlockColorsBtn,*m_unlockLevelsBtn,*m_noclipBtn,*m_autoJumpBtn,*m_ldmBtn,*m_autoLDMBtn,*m_spinbotBtn,*m_shakeBtn,*m_nlGraviteBtn,*m_spinDown,*m_spinUp,*m_spinValueBtn,*m_speedhackBtn,*m_shDown,*m_shUp,*m_shValueBtn,*m_aaEnabledBtn,*m_aaFlipXBtn,*m_aaFlipYBtn,*m_tabMain,*m_tabRage,*m_tabVisuals,*m_tabAntiAim,*m_tabCreate,*m_tabLegit,*m_tabCosmetics;
public:
    static NeverloseMenu* create(){auto r=new NeverloseMenu();if(r&&r->init()){r->autorelease();return r;}delete r;return nullptr;}
    void refreshButtons(){
        m_jumpHackBtn->setNormalImage(ButtonSprite::create(g_jumpHack?"Jump Hack: ON":"Jump Hack: OFF"));
        m_noclipBtn->setNormalImage(ButtonSprite::create(g_noclip?"Noclip: ON":"Noclip: OFF"));
        m_autoJumpBtn->setNormalImage(ButtonSprite::create(g_autoJump?"AutoJump: ON":"AutoJump: OFF"));
        m_ldmBtn->setNormalImage(ButtonSprite::create(g_ldm?"LDM: ON":"LDM: OFF"));
        m_autoLDMBtn->setNormalImage(ButtonSprite::create(g_autoLDM?"Auto LDM: ON":"Auto LDM: OFF"));
        m_spinbotBtn->setNormalImage(ButtonSprite::create(g_spinbot?"Spinbot: ON":"Spinbot: OFF"));
        m_shakeBtn->setNormalImage(ButtonSprite::create(g_shake?"Shake: ON":"Shake: OFF"));
        m_nlGraviteBtn->setNormalImage(ButtonSprite::create(g_nlGravite?"NL.exe Gravite: ON":"NL.exe Gravite: OFF"));
        m_speedhackBtn->setNormalImage(ButtonSprite::create(g_speedhack?"Speedhack: ON":"Speedhack: OFF"));
        m_copyHackBtn->setNormalImage(ButtonSprite::create(g_copyHack?"Copy Hack: ON":"Copy Hack: OFF"));
        m_autoSafeModeBtn->setNormalImage(ButtonSprite::create(g_autoSafeMode?"Auto Safe Mode: ON":"Auto Safe Mode: OFF"));
        m_aaEnabledBtn->setNormalImage(ButtonSprite::create(g_aaEnabled?"Anti-Aim: ON":"Anti-Aim: OFF"));
        m_aaFlipXBtn->setNormalImage(ButtonSprite::create(g_aaFlipX?"Flip Back: ON":"Flip Back: OFF"));
        m_aaFlipYBtn->setNormalImage(ButtonSprite::create(g_aaFlipY?"Flip Down: ON":"Flip Down: OFF"));
        m_cheatIndBtn->setNormalImage(ButtonSprite::create(g_cheatIndicator?"Cheat Ind: ON":"Cheat Ind: OFF"));
        m_cpsBtn->setNormalImage(ButtonSprite::create(g_showCPS?"CPS: ON":"CPS: OFF"));
        m_timeBtn->setNormalImage(ButtonSprite::create(g_showTime?"Time: ON":"Time: OFF"));
        m_fpsBtn->setNormalImage(ButtonSprite::create(g_showFPS?"FPS: ON":"FPS: OFF"));
        m_unlockIconsBtn->setNormalImage(ButtonSprite::create(g_unlockIcons?"Icons: ON":"Icons: OFF"));
        m_unlockVaultBtn->setNormalImage(ButtonSprite::create(g_unlockVault?"Vault: ON":"Vault: OFF"));
        m_unlockColorsBtn->setNormalImage(ButtonSprite::create(g_unlockColors?"Colors: ON":"Colors: OFF"));
        m_unlockLevelsBtn->setNormalImage(ButtonSprite::create(g_unlockLevels?"Levels: ON":"Levels: OFF"));
    }
    void setPage(int p){
        if(p<0)p=0;if(p>=7)p=6;
        for(auto n:m_mainNodes)n->setVisible(p==0);
        for(auto n:m_rageNodes)n->setVisible(p==1);
        for(auto n:m_visualsNodes)n->setVisible(p==2);
        for(auto n:m_antiAimNodes)n->setVisible(p==3);
        for(auto n:m_createNodes)n->setVisible(p==4);
        for(auto n:m_legitNodes)n->setVisible(p==5);
        for(auto n:m_cosmeticsNodes)n->setVisible(p==6);
        m_tabMain->setColor(p==0?ccWHITE:ccGRAY);
        m_tabRage->setColor(p==1?ccWHITE:ccGRAY);
        m_tabVisuals->setColor(p==2?ccWHITE:ccGRAY);
        m_tabAntiAim->setColor(p==3?ccWHITE:ccGRAY);
        m_tabCreate->setColor(p==4?ccWHITE:ccGRAY);
        m_tabLegit->setColor(p==5?ccWHITE:ccGRAY);
        m_tabCosmetics->setColor(p==6?ccWHITE:ccGRAY);
    }
    bool init(){
        if(!CCLayer::init())return false;
        auto ws=CCDirector::get()->getWinSize();
        float cx=ws.width/2,cy=ws.height/2;
        auto ov=CCLayerColor::create({0,0,0,180});this->addChild(ov,-1);
        auto pn=CCScale9Sprite::create("GJ_square01.png");pn->setContentSize({780,520});pn->setPosition({cx,cy});pn->setColor({15,15,15});this->addChild(pn);
        auto sb=CCScale9Sprite::create("GJ_square01.png");sb->setContentSize({220,500});sb->setPosition({cx-270,cy});sb->setColor({25,25,30});this->addChild(sb);
        auto lg=CCLabelBMFont::create("NEVERLOSE","goldFont.fnt");lg->setPosition({cx-270,cy+215});lg->setScale(0.65f);lg->setColor({255,255,255});this->addChild(lg);
        auto mn=CCMenu::create();mn->setPosition({0,0});this->addChild(mn);
        auto cs=CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");cs->setScale(0.8f);
        auto cb=CCMenuItemSpriteExtra::create(cs,this,menu_selector(NeverloseMenu::onClose));cb->setPosition({cx+300,cy-200});mn->addChild(cb);
        float ty[7]={cy+105,cy+70,cy+35,cy,cy-35,cy-70,cy-105};
        m_tabMain=CCMenuItemSpriteExtra::create(ButtonSprite::create("Main"),this,menu_selector(NeverloseMenu::onTabMain));m_tabMain->setPosition({cx-270,ty[0]});m_tabMain->setScale(0.5f);mn->addChild(m_tabMain);
        m_tabRage=CCMenuItemSpriteExtra::create(ButtonSprite::create("Rage"),this,menu_selector(NeverloseMenu::onTabRage));m_tabRage->setPosition({cx-270,ty[1]});m_tabRage->setScale(0.5f);mn->addChild(m_tabRage);
        m_tabVisuals=CCMenuItemSpriteExtra::create(ButtonSprite::create("Visuals"),this,menu_selector(NeverloseMenu::onTabVisuals));m_tabVisuals->setPosition({cx-270,ty[2]});m_tabVisuals->setScale(0.5f);mn->addChild(m_tabVisuals);
        m_tabAntiAim=CCMenuItemSpriteExtra::create(ButtonSprite::create("Anti-Aim"),this,menu_selector(NeverloseMenu::onTabAntiAim));m_tabAntiAim->setPosition({cx-270,ty[3]});m_tabAntiAim->setScale(0.5f);mn->addChild(m_tabAntiAim);
        m_tabCreate=CCMenuItemSpriteExtra::create(ButtonSprite::create("Create"),this,menu_selector(NeverloseMenu::onTabCreate));m_tabCreate->setPosition({cx-270,ty[4]});m_tabCreate->setScale(0.5f);mn->addChild(m_tabCreate);
        m_tabLegit=CCMenuItemSpriteExtra::create(ButtonSprite::create("Legit"),this,menu_selector(NeverloseMenu::onTabLegit));m_tabLegit->setPosition({cx-270,ty[5]});m_tabLegit->setScale(0.5f);mn->addChild(m_tabLegit);
        m_tabCosmetics=CCMenuItemSpriteExtra::create(ButtonSprite::create("Cosmetics"),this,menu_selector(NeverloseMenu::onTabCosmetics));m_tabCosmetics->setPosition({cx-270,ty[6]});m_tabCosmetics->setScale(0.5f);mn->addChild(m_tabCosmetics);
        m_jumpHackBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("Jump Hack: OFF"),this,menu_selector(NeverloseMenu::onJumpHack));m_jumpHackBtn->setPosition({cx+60,cy+130});m_jumpHackBtn->setScale(0.55f);mn->addChild(m_jumpHackBtn);m_mainNodes.push_back(m_jumpHackBtn);
        m_noclipBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("Noclip: OFF"),this,menu_selector(NeverloseMenu::onNoclip));m_noclipBtn->setPosition({cx+60,cy+60});m_noclipBtn->setScale(0.55f);mn->addChild(m_noclipBtn);m_rageNodes.push_back(m_noclipBtn);
        m_autoJumpBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("AutoJump: OFF"),this,menu_selector(NeverloseMenu::onAutoJump));m_autoJumpBtn->setPosition({cx+60,cy-10});m_autoJumpBtn->setScale(0.55f);mn->addChild(m_autoJumpBtn);m_rageNodes.push_back(m_autoJumpBtn);
        m_ldmBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("LDM: OFF"),this,menu_selector(NeverloseMenu::onLDM));m_ldmBtn->setPosition({cx+60,cy+90});m_ldmBtn->setScale(0.55f);mn->addChild(m_ldmBtn);m_visualsNodes.push_back(m_ldmBtn);
        m_autoLDMBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("Auto LDM: OFF"),this,menu_selector(NeverloseMenu::onAutoLDM));m_autoLDMBtn->setPosition({cx+60,cy+25});m_autoLDMBtn->setScale(0.55f);mn->addChild(m_autoLDMBtn);m_visualsNodes.push_back(m_autoLDMBtn);
        m_spinbotBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("Spinbot: OFF"),this,menu_selector(NeverloseMenu::onSpinbot));m_spinbotBtn->setPosition({cx-70,cy+150});m_spinbotBtn->setScale(0.5f);mn->addChild(m_spinbotBtn);m_antiAimNodes.push_back(m_spinbotBtn);
        auto st=CCLabelBMFont::create("Spinbot Speed:","bigFont.fnt");st->setPosition({cx-160,cy+105});st->setScale(0.45f);st->setColor({0,200,255});st->setAnchorPoint({0,0.5f});this->addChild(st);m_antiAimNodes.push_back(st);
        m_spinDown=CCMenuItemSpriteExtra::create(ButtonSprite::create("<"),this,menu_selector(NeverloseMenu::onSpinDown));m_spinDown->setPosition({cx-115,cy+105});m_spinDown->setScale(0.5f);mn->addChild(m_spinDown);m_antiAimNodes.push_back(m_spinDown);
        m_spinValueBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("5.00"),this,menu_selector(NeverloseMenu::onSpinValue));m_spinValueBtn->setPosition({cx-55,cy+105});m_spinValueBtn->setScale(0.5f);mn->addChild(m_spinValueBtn);m_antiAimNodes.push_back(m_spinValueBtn);
        m_spinUp=CCMenuItemSpriteExtra::create(ButtonSprite::create(">"),this,menu_selector(NeverloseMenu::onSpinUp));m_spinUp->setPosition({cx+5,cy+105});m_spinUp->setScale(0.5f);mn->addChild(m_spinUp);m_antiAimNodes.push_back(m_spinUp);
        m_aaEnabledBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("Anti-Aim: OFF"),this,menu_selector(NeverloseMenu::onAAEnabled));m_aaEnabledBtn->setPosition({cx-70,cy+50});m_aaEnabledBtn->setScale(0.5f);mn->addChild(m_aaEnabledBtn);m_antiAimNodes.push_back(m_aaEnabledBtn);
        m_aaFlipXBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("Flip Back: OFF"),this,menu_selector(NeverloseMenu::onAAFlipX));m_aaFlipXBtn->setPosition({cx-70,cy});m_aaFlipXBtn->setScale(0.5f);mn->addChild(m_aaFlipXBtn);m_antiAimNodes.push_back(m_aaFlipXBtn);
        m_aaFlipYBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("Flip Down: OFF"),this,menu_selector(NeverloseMenu::onAAFlipY));m_aaFlipYBtn->setPosition({cx-70,cy-50});m_aaFlipYBtn->setScale(0.5f);mn->addChild(m_aaFlipYBtn);m_antiAimNodes.push_back(m_aaFlipYBtn);
        m_shakeBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("Shake: OFF"),this,menu_selector(NeverloseMenu::onShake));m_shakeBtn->setPosition({cx+150,cy+140});m_shakeBtn->setScale(0.5f);mn->addChild(m_shakeBtn);m_antiAimNodes.push_back(m_shakeBtn);
        m_nlGraviteBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("NL.exe Gravite: OFF"),this,menu_selector(NeverloseMenu::onNLGravite));m_nlGraviteBtn->setPosition({cx+150,cy+75});m_nlGraviteBtn->setScale(0.5f);mn->addChild(m_nlGraviteBtn);m_antiAimNodes.push_back(m_nlGraviteBtn);
        m_speedhackBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("Speedhack: OFF"),this,menu_selector(NeverloseMenu::onSpeedhack));m_speedhackBtn->setPosition({cx+150,cy+10});m_speedhackBtn->setScale(0.5f);mn->addChild(m_speedhackBtn);m_antiAimNodes.push_back(m_speedhackBtn);
        auto sht=CCLabelBMFont::create("Speedhack Value:","bigFont.fnt");sht->setPosition({cx+70,cy-50});sht->setScale(0.45f);sht->setColor({0,200,255});sht->setAnchorPoint({0,0.5f});this->addChild(sht);m_antiAimNodes.push_back(sht);
        m_shDown=CCMenuItemSpriteExtra::create(ButtonSprite::create("<"),this,menu_selector(NeverloseMenu::onSpeedhackDown));m_shDown->setPosition({cx+110,cy-50});m_shDown->setScale(0.5f);mn->addChild(m_shDown);m_antiAimNodes.push_back(m_shDown);
        m_shValueBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("1.00"),this,menu_selector(NeverloseMenu::onSpeedValue));m_shValueBtn->setPosition({cx+170,cy-50});m_shValueBtn->setScale(0.5f);mn->addChild(m_shValueBtn);m_antiAimNodes.push_back(m_shValueBtn);
        m_shUp=CCMenuItemSpriteExtra::create(ButtonSprite::create(">"),this,menu_selector(NeverloseMenu::onSpeedhackUp));m_shUp->setPosition({cx+230,cy-50});m_shUp->setScale(0.5f);mn->addChild(m_shUp);m_antiAimNodes.push_back(m_shUp);
        m_copyHackBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("Copy Hack: OFF"),this,menu_selector(NeverloseMenu::onCopyHack));m_copyHackBtn->setPosition({cx+60,cy+100});m_copyHackBtn->setScale(0.55f);mn->addChild(m_copyHackBtn);m_createNodes.push_back(m_copyHackBtn);
        m_autoSafeModeBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("Auto Safe Mode: OFF"),this,menu_selector(NeverloseMenu::onAutoSafeMode));m_autoSafeModeBtn->setPosition({cx+60,cy+130});m_autoSafeModeBtn->setScale(0.55f);mn->addChild(m_autoSafeModeBtn);m_legitNodes.push_back(m_autoSafeModeBtn);
        m_cheatIndBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("Cheat Ind: OFF"),this,menu_selector(NeverloseMenu::onCheatIndicator));m_cheatIndBtn->setPosition({cx+60,cy+60});m_cheatIndBtn->setScale(0.55f);mn->addChild(m_cheatIndBtn);m_legitNodes.push_back(m_cheatIndBtn);
        m_cpsBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("CPS: OFF"),this,menu_selector(NeverloseMenu::onCPS));m_cpsBtn->setPosition({cx+60,cy-10});m_cpsBtn->setScale(0.55f);mn->addChild(m_cpsBtn);m_legitNodes.push_back(m_cpsBtn);
        m_timeBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("Time: OFF"),this,menu_selector(NeverloseMenu::onTime));m_timeBtn->setPosition({cx+60,cy-80});m_timeBtn->setScale(0.55f);mn->addChild(m_timeBtn);m_legitNodes.push_back(m_timeBtn);
        m_fpsBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("FPS: OFF"),this,menu_selector(NeverloseMenu::onFPS));m_fpsBtn->setPosition({cx+60,cy-150});m_fpsBtn->setScale(0.55f);mn->addChild(m_fpsBtn);m_legitNodes.push_back(m_fpsBtn);
        m_unlockIconsBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("Icons: OFF"),this,menu_selector(NeverloseMenu::onUnlockIcons));m_unlockIconsBtn->setPosition({cx+60,cy+130});m_unlockIconsBtn->setScale(0.55f);mn->addChild(m_unlockIconsBtn);m_cosmeticsNodes.push_back(m_unlockIconsBtn);
        m_unlockVaultBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("Vault: OFF"),this,menu_selector(NeverloseMenu::onUnlockVault));m_unlockVaultBtn->setPosition({cx+60,cy+60});m_unlockVaultBtn->setScale(0.55f);mn->addChild(m_unlockVaultBtn);m_cosmeticsNodes.push_back(m_unlockVaultBtn);
        m_unlockColorsBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("Colors: OFF"),this,menu_selector(NeverloseMenu::onUnlockColors));m_unlockColorsBtn->setPosition({cx+60,cy-10});m_unlockColorsBtn->setScale(0.55f);mn->addChild(m_unlockColorsBtn);m_cosmeticsNodes.push_back(m_unlockColorsBtn);
        m_unlockLevelsBtn=CCMenuItemSpriteExtra::create(ButtonSprite::create("Levels: OFF"),this,menu_selector(NeverloseMenu::onUnlockLevels));m_unlockLevelsBtn->setPosition({cx+60,cy-80});m_unlockLevelsBtn->setScale(0.55f);mn->addChild(m_unlockLevelsBtn);m_cosmeticsNodes.push_back(m_unlockLevelsBtn);
        setPage(0);refreshButtons();
        this->setScale(0.3f);this->runAction(CCEaseBackOut::create(CCScaleTo::create(0.35f,1.0f)));
        this->setKeypadEnabled(true);
        return true;
    }
    void onTabMain(CCObject*){setPage(0);}
    void onTabRage(CCObject*){setPage(1);}
    void onTabVisuals(CCObject*){setPage(2);}
    void onTabAntiAim(CCObject*){setPage(3);}
    void onTabCreate(CCObject*){setPage(4);}
    void onTabLegit(CCObject*){setPage(5);}
    void onTabCosmetics(CCObject*){setPage(6);}
    void onJumpHack(CCObject*){g_jumpHack=!g_jumpHack;refreshButtons();}
    void onNoclip(CCObject*){g_noclip=!g_noclip;refreshButtons();}
    void onAutoJump(CCObject*){g_autoJump=!g_autoJump;refreshButtons();}
    void onLDM(CCObject*){g_ldm=!g_ldm;GameManager::get()->setGameVariable("low_detail_mode",g_ldm);refreshButtons();}
    void onAutoLDM(CCObject*){g_autoLDM=!g_autoLDM;GameManager::get()->setGameVariable("low_detail_mode",g_autoLDM);refreshButtons();}
    void onSpinbot(CCObject*){g_spinbot=!g_spinbot;if(g_spinbot){g_shake=false;g_nlGravite=false;}refreshButtons();}
    void onShake(CCObject*){g_shake=!g_shake;g_shakeTimer=0.0f;if(g_shake){g_spinbot=false;g_nlGravite=false;}refreshButtons();}
    void onNLGravite(CCObject*){g_nlGravite=!g_nlGravite;if(g_nlGravite){g_spinbot=false;g_shake=false;}refreshButtons();}
    void onSpinUp(CCObject*){g_spinSpeed+=1;if(g_spinSpeed>500)g_spinSpeed=500;m_spinValueBtn->setNormalImage(ButtonSprite::create(CCString::createWithFormat("%.2f",g_spinSpeed)->getCString()));}
    void onSpinDown(CCObject*){g_spinSpeed-=1;if(g_spinSpeed<1)g_spinSpeed=1;m_spinValueBtn->setNormalImage(ButtonSprite::create(CCString::createWithFormat("%.2f",g_spinSpeed)->getCString()));}
        void onSpinValue(CCObject*){
        auto p = ValueInputPopup::create(false);
        if(p) this->addChild(p, 9);
    }
    void onSpeedhackUp(CCObject*){
        g_speed += 1;
        if(g_speed > 500) g_speed = 500;
        if(g_speedhack) CCDirector::get()->getScheduler()->setTimeScale(g_speed);
        m_shValueBtn->setNormalImage(ButtonSprite::create(CCString::createWithFormat("%.2f", g_speed)->getCString()));
    }
    void onSpeedhackDown(CCObject*){
        g_speed -= 1;
        if(g_speed < 1) g_speed = 1;
        if(g_speedhack) CCDirector::get()->getScheduler()->setTimeScale(g_speed);
        m_shValueBtn->setNormalImage(ButtonSprite::create(CCString::createWithFormat("%.2f", g_speed)->getCString()));
    }
    void onSpeedValue(CCObject*){
        auto p = ValueInputPopup::create(true);
        if(p) this->addChild(p, 9);
    }
    void onSpeedhack(CCObject*){
        g_speedhack = !g_speedhack;
        CCDirector::get()->getScheduler()->setTimeScale(g_speedhack ? g_speed : 1.0f);
        refreshButtons();
    }
    void onCopyHack(CCObject*){ g_copyHack = !g_copyHack; refreshButtons(); }
    void onAutoSafeMode(CCObject*){ g_autoSafeMode = !g_autoSafeMode; refreshButtons(); }
    void onCheatIndicator(CCObject*){ g_cheatIndicator = !g_cheatIndicator; refreshButtons(); }
    void onCPS(CCObject*){ g_showCPS = !g_showCPS; refreshButtons(); }
    void onTime(CCObject*){ g_showTime = !g_showTime; refreshButtons(); }
    void onFPS(CCObject*){ g_showFPS = !g_showFPS; refreshButtons(); }
    void onUnlockIcons(CCObject*){ g_unlockIcons = !g_unlockIcons; refreshButtons(); }
    void onUnlockVault(CCObject*){ g_unlockVault = !g_unlockVault; refreshButtons(); }
    void onUnlockColors(CCObject*){ g_unlockColors = !g_unlockColors; refreshButtons(); }
    void onUnlockLevels(CCObject*){ g_unlockLevels = !g_unlockLevels; refreshButtons(); }
    void onAAEnabled(CCObject*){ g_aaEnabled = !g_aaEnabled; refreshButtons(); }
    void onAAFlipX(CCObject*){ g_aaFlipX = !g_aaFlipX; refreshButtons(); }
    void onAAFlipY(CCObject*){ g_aaFlipY = !g_aaFlipY; refreshButtons(); }
    void onClose(CCObject*){ this->removeFromParentAndCleanup(true); }
    void keyBackClicked(){ this->removeFromParentAndCleanup(true); }
    void registerWithTouchDispatcher() override {
        CCDirector::get()->getTouchDispatcher()->addTargetedDelegate(this, -500, true);
    }
    bool ccTouchBegan(CCTouch* touch, CCEvent*) override {
        return true;
    }
};

// ---------------- PlayLayer Hooks ----------------
class $modify(NLPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* cpsLabel = nullptr;
        CCLabelBMFont* timeLabel = nullptr;
        CCLabelBMFont* fpsLabel = nullptr;
        CCLabelBMFont* cheatLabel = nullptr;
        float timeAlive = 0.0f;
        int frameCount = 0;
        float fpsTimer = 0.0f;
        float cpsTimer = 0.0f;
        int cpsInWindow = 0;
        float lastClickTime = 0.0f;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if(!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        auto winSize = CCDirector::get()->getWinSize();

        if(g_showCPS){
            m_fields->cpsLabel = CCLabelBMFont::create("CPS: 0", "bigFont.fnt");
            m_fields->cpsLabel->setScale(0.4f);
            m_fields->cpsLabel->setPosition({60, winSize.height - 20});
            m_fields->cpsLabel->setColor({0, 255, 255});
            this->addChild(m_fields->cpsLabel, 100);
        }
        if(g_showTime){
            m_fields->timeLabel = CCLabelBMFont::create("Time: 0.00", "bigFont.fnt");
            m_fields->timeLabel->setScale(0.4f);
            m_fields->timeLabel->setPosition({60, winSize.height - 45});
            m_fields->timeLabel->setColor({255, 255, 0});
            this->addChild(m_fields->timeLabel, 100);
        }
        if(g_showFPS){
            m_fields->fpsLabel = CCLabelBMFont::create("FPS: 60", "bigFont.fnt");
            m_fields->fpsLabel->setScale(0.4f);
            m_fields->fpsLabel->setPosition({60, winSize.height - 70});
            m_fields->fpsLabel->setColor({255, 100, 255});
            this->addChild(m_fields->fpsLabel, 100);
        }
        if(g_cheatIndicator){
            m_fields->cheatLabel = CCLabelBMFont::create("[NL.exe]", "bigFont.fnt");
            m_fields->cheatLabel->setScale(0.5f);
            m_fields->cheatLabel->setPosition({winSize.width - 60, winSize.height - 20});
            m_fields->cheatLabel->setColor({255, 50, 50});
            this->addChild(m_fields->cheatLabel, 100);
        }
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        auto f = m_fields.self();

        // Auto Safe Mode
        if(g_autoSafeMode) {
            GameManager::get()->setGameVariable("0021", true);
        }

        // Auto LDM
        if(g_autoLDM) {
            GameManager::get()->setGameVariable("low_detail_mode", true);
        }

        // Noclip / AutoJump / JumpHack
        auto player = m_player1;
        if(player){
            if(g_noclip){
                player->m_isDead = false;
                if(player->getPositionY() < -1000){
                    player->setPositionY(300);
                }
            }
            if(g_autoJump || g_jumpHack){
                if(m_player1 && !m_player1->m_isDead){
                    // simulated jump triggered by player input in handleButton
                }
            }
            if(g_aaEnabled && g_spinbot){
                g_spinPhase += g_spinSpeed * dt;
                player->setRotation(g_spinPhase * 360.0f);
            }
            if(g_aaFlipX){
                player->setScaleX(-1.0f);
            }
            if(g_aaFlipY){
                player->setScaleY(-1.0f);
            }
        }

        // Shake
        if(g_shake){
            g_shakeTimer += dt;
            float offX = (rand() % 100 - 50) / 50.0f * 5.0f;
            float offY = (rand() % 100 - 50) / 50.0f * 5.0f;
            this->setPosition({offX, offY});
        } else {
            this->setPosition({0, 0});
        }

        // NL Gravite
        if(g_nlGravite && player){
            player->m_gravityMod = (rand() % 2 == 0) ? -1.0f : 1.0f;
        }

        // Timing / FPS / CPS update
        f->timeAlive += dt;
        f->frameCount++;
        f->fpsTimer += dt;
        if(f->fpsTimer >= 0.5f){
            float fps = f->frameCount / f->fpsTimer;
            if(f->fpsLabel){
                f->fpsLabel->setString(CCString::createWithFormat("FPS: %.0f", fps)->getCString());
            }
            f->frameCount = 0;
            f->fpsTimer = 0.0f;
        }
        if(f->timeLabel){
            f->timeLabel->setString(CCString::createWithFormat("Time: %.2f", f->timeAlive)->getCString());
        }
        if(f->cpsLabel){
            f->cpsTimer += dt;
            if(f->cpsTimer >= 1.0f){
                f->cpsLabel->setString(CCString::createWithFormat("CPS: %d", g_clickCount)->getCString());
                g_clickCount = 0;
                f->cpsTimer = 0.0f;
            }
        }
    }

    void handleButton(bool down, int button, bool isPlayer1) {
        PlayLayer::handleButton(down, button, isPlayer1);
        if(down && button == 1) g_clickCount++;
        if(g_autoJump && down){
            // Auto-jump: trigger a jump
            if(m_player1) m_player1->pushButton(PlayerButton::Jump);
        }
    }

    void onQuit() {
        if(g_speedhack){
            CCDirector::get()->getScheduler()->setTimeScale(1.0f);
        }
        if(g_shake) this->setPosition({0, 0});
        PlayLayer::onQuit();
    }
};

// ---------------- PauseLayer Hook (adds NL menu button) ----------------
class $modify(NLPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto winSize = CCDirector::get()->getWinSize();
        auto btnSprite = ButtonSprite::create("NL");
        btnSprite->setScale(0.9f);
        auto btn = CCMenuItemSpriteExtra::create(
            btnSprite,
            this,
            menu_selector(NLPauseLayer::onOpenNLMenu)
        );
        btn->setPosition({winSize.width - 40, winSize.height - 60});

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        menu->addChild(btn);
        this->addChild(menu, 100);
    }

    void onOpenNLMenu(CCObject*) {
        auto menu = NeverloseMenu::create();
        if(menu) {
            CCDirector::get()->getRunningScene()->addChild(menu, 9999);
        }
    }
};

// ---------------- PlayerObject Hook ----------------
class $modify(NLPlayerObject, PlayerObject) {
    void update(float dt) {
        if(g_aaEnabled){
            if(g_aaFlipX) this->setScaleX(-1.0f);
            if(g_aaFlipY) this->setScaleY(-1.0f);
            if(g_spinbot){
                g_spinPhase += g_spinSpeed * dt;
                this->setRotation(g_spinPhase * 360.0f);
            }
        }
        PlayerObject::update(dt);
    }
};

// ---------------- LevelInfoLayer Hook (Copy Hack) ----------------
class $modify(NLLevelInfoLayer, LevelInfoLayer) {
    void onPlay(CCObject* sender) {
        if(g_copyHack){
            // Copy hack: mark level as copied (cheat functionality placeholder)
            if(m_level) {
                auto levelString = m_level->m_levelString;
                // In a real impl you'd write to clipboard
                log::info("Copy Hack: level string length {}", levelString.size());
            }
        }
        LevelInfoLayer::onPlay(sender);
    }
};
