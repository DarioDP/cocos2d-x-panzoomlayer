#include "HelloWorldScene.h"
#include "PanZoomLayer.h"

using namespace cocos2d;

Scene* HelloWorld::scene() {
    auto scene = Scene::create();
    auto layer = HelloWorld::create();
    scene->addChild(layer);
    return scene;
}

bool HelloWorld::init() {
    if (!Layer::init()) {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();

    auto map = Sprite::create("Fishfam_base_TH8.jpg");
    auto mapSize = map->getContentSize();
    map->setAnchorPoint(Vec2(0, 0));
    map->setPosition(mapSize * -0.5f);
    PanZoomLayer *pzLayer = PanZoomLayer::create();
    this->addChild(pzLayer);
    pzLayer->SetPanBoundsRect(Rect(mapSize.width * -0.5f, mapSize.height * -0.5f, mapSize.width * 1.0f, mapSize.height * 1.0f));
    pzLayer->setScale(1.0f);
    pzLayer->addChild(map);

    return true;
}


void HelloWorld::menuCloseCallback(Ref * pSender) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
  MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.", "Alert");
  return;
#endif

  Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  exit(0);
#endif
}

