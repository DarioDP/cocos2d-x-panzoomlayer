cocos2d-x-panzoomlayer
======================

You can use panning and zooming with inheriting this class!


*Usage Example:*

```
  auto map = Sprite::create("map.jpg");
  auto mapSize = map->getContentSize();
  map->setAnchorPoint(Vec2(0, 0));
  map->setPosition(mapSize * -0.5f);
  PanZoomLayer *pzLayer = PanZoomLayer::create();
  this->addChild(pzLayer);
  pzLayer->SetPanBoundsRect(Rect(mapSize.width * -0.5f, mapSize.height * -0.5f, mapSize.width * 1.0f, mapSize.height * 1.0f));
  pzLayer->setScale(1.0f);
  pzLayer->addChild(map);
```

For reference: [Original post on Cocos2Dx Forum](http://www.cocos2d-x.org/forums/6/topics/5430)
