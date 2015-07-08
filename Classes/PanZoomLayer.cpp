#include "PanZoomLayer.h"

PanZoomLayer *PanZoomLayer::create() {
  PanZoomLayer *pRet = new PanZoomLayer;
  if (pRet && pRet->init()) {
    pRet->autorelease();
    return pRet;
  }
  CC_SAFE_DELETE(pRet);
  return NULL;
}

bool PanZoomLayer::init() {
  if (!LayerColor::initWithColor(Color4B(255, 0, 0, 0))) return false;

  this->setAnchorPoint(Vec2(0, 0));

  _accelerationFactor = 0.0f;
  _productFactor = 55.0f;

  _maxScale = 8.0f;
  _minScale = 0.1f;

  _isHolding = false;

  return true;
}

void PanZoomLayer::onEnter() {
  Layer::onEnter();
  Director::getInstance()->getScheduler()->scheduleUpdate(this, 0, false);
  auto touchListener = EventListenerTouchAllAtOnce::create();
  touchListener->onTouchesBegan = CC_CALLBACK_2(PanZoomLayer::onTouchesBegan, this);
  touchListener->onTouchesMoved = CC_CALLBACK_2(PanZoomLayer::onTouchesMoved, this);
  touchListener->onTouchesEnded = CC_CALLBACK_2(PanZoomLayer::onTouchesEnded, this);
  this->_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
}

void PanZoomLayer::onExit() {
  Director::getInstance()->getScheduler()->unscheduleAllForTarget(this);
  Layer::onExit();
  _touches.clear();
}

void PanZoomLayer::update(float dt) {
  // Skip smoothe panning when dt is high value
  if (dt > 1.0f / 55) return;

  LayerColor::update(dt);

  if (_touches.size() == 1) {
    _accelerationFactor *= 40 * dt * 0.95f;
  }
  else if (_touches.size() == 0) {
    _accelerationFactor = fabs(_accelerationFactor - 0);
    if (_accelerationFactor < FLT_EPSILON) return;
    if (_accelerationFactor < 0.004f) {
      _accelerationFactor = 0;
    }
    else {
      double d = dt * 60;
      if (d > 0.99) d = 0.99;
      double i = (0 - _accelerationFactor) * 0.025 * d;

      _accelerationFactor = (_accelerationFactor + i) * d;

      Vec2 adder = _deltaSum;
      adder.x *= this->getContentSize().width;
      adder.y *= this->getContentSize().height;

      this->setPosition(this->getPosition() + adder * 2.5 * _accelerationFactor);
    }
  }
}

void PanZoomLayer::onTouchesBegan(const std::vector<Touch *> &touches, Event *event) {
  if (_isHolding) return;

  for (auto &touch: touches) {
    _touches.pushBack(touch);
  }

  _deltaSum = Vec2(0, 0);
  _accelerationFactor = 0;
  gettimeofday(&_timeStamp, NULL);
}

void PanZoomLayer::onTouchesMoved(const std::vector<Touch *> &touches, Event *event) {
  if (_isHolding) return;

  if (_touches.size() == 1) {
    Touch *touch = (Touch *) _touches.at(0);
    Vec2 curTouchPosition = Director::getInstance()->convertToGL(touch->getLocationInView());
    Vec2 prevTouchPosition = Director::getInstance()->convertToGL(touch->getPreviousLocationInView());
    Vec2 deltaPosition = curTouchPosition - prevTouchPosition;
    this->setPosition(this->getPosition() + deltaPosition);

    float prevAngle = CC_RADIANS_TO_DEGREES(_prevDeltaPoint.getAngle());
    float angle = CC_RADIANS_TO_DEGREES(deltaPosition.getAngle());
    if (fabs(prevAngle - angle) <= 30) {
      _deltaSum = Vec2(0, 0);
    }

    _prevDeltaPoint = deltaPosition;

    _deltaSum.x = _deltaSum.x + deltaPosition.x / this->getContentSize().width;
    _deltaSum.y = _deltaSum.y + deltaPosition.y / this->getContentSize().height;

    _accelerationFactor += _deltaSum.getLength() * 4.0;
  }
  else if (_touches.size() >= 2) {
    // Get the two first touches
    Touch *touch1 = (Touch *) _touches.at(0);
    Touch *touch2 = (Touch *) _touches.at(1);
    log("touch1 %f", touch1->getLocation().x);
    log("touch2 %f", touch2->getLocation().x);

    // Get current and previous positions of the touches
    Vec2 curPosTouch1 = Director::getInstance()->convertToGL(touch1->getLocationInView());
    Vec2 curPosTouch2 = Director::getInstance()->convertToGL(touch2->getLocationInView());
    Vec2 prevPosTouch1 = Director::getInstance()->convertToGL(touch1->getPreviousLocationInView());
    Vec2 prevPosTouch2 = Director::getInstance()->convertToGL(touch2->getPreviousLocationInView());

    // Calculate current and previous positions of the layer relative the anchor point
    Vec2 curPosLayer = curPosTouch1.getMidpoint(curPosTouch2);
    Vec2 prevPosLayer = prevPosTouch1.getMidpoint(prevPosTouch2);
    log("curPosLayer %f %f", curPosLayer.x, curPosLayer.y);
    log("prevPosLayer %f %f", prevPosLayer.x, prevPosLayer.y);

    // Calculate new scale
    float prevScale = this->getScale();
    float curScale = this->getScale() * curPosTouch1.distance(curPosTouch2) / prevPosTouch1.distance(prevPosTouch2);

    this->setScale(curScale);
    log("setScale %f", curScale);

    if (this->getScale() != prevScale) {
      Vec2 realCurPosLayer = this->convertToNodeSpaceAR(curPosLayer);
      float deltaX = (realCurPosLayer.x) * (this->getScale() - prevScale);
      float deltaY = (realCurPosLayer.y) * (this->getScale() - prevScale);

      this->setPosition(Vec2(this->getPosition().x - deltaX, this->getPosition().y - deltaY));
    }

    // If current and previous position of the multitouch's center aren't equal -> change position of the layer
    if (!prevPosLayer.equals(curPosLayer)) {
      this->setPosition(Vec2(this->getPosition().x + curPosLayer.x - prevPosLayer.x,
        this->getPosition().y + curPosLayer.y - prevPosLayer.y));
    }
  }
}

void PanZoomLayer::onTouchesEnded(const std::vector<Touch *> &touches, Event *event) {
  if (_isHolding) return;

  for (auto touch: touches) {
    _touches.eraseObject(touch);
  }
}

void PanZoomLayer::setPosition(Vec2 position) {
  Node::setPosition(position);

  if (_panBoundsRect.equals(Rect::ZERO) == false) {
    Rect boundBox;
    boundBox.origin = this->getPosition() / this->getScale();
    boundBox.size = this->getContentSize() / this->getScale();
//    log("boundBox : origin(%.1f, %.1f), size(%.1f, %.1f)", boundBox.origin.x, boundBox.origin.y, boundBox.size.width, boundBox.size.height);

    // OpenGL coordinate system
    float left = boundBox.getMinX();
    float right = boundBox.getMaxX();
    float top = boundBox.getMaxY();
    float bottom = boundBox.getMinY();
//    log("left,right(%.1f, %.1f), top,bottom(%.1f, %.1f)", left, right, top, bottom);

    float min_x = _panBoundsRect.getMinX() + boundBox.size.width;
    float max_x = _panBoundsRect.getMaxX() + boundBox.size.width;
    float min_y = _panBoundsRect.getMinY() + boundBox.size.height;
    float max_y = _panBoundsRect.getMaxY() + boundBox.size.height;
//    log("min(%.1f, %.1f), max(%.1f, %.1f)", min_x, min_y, max_x, max_y);

    float scale = this->getScale();
    float arLeft = min_x * scale;
    float arRight = max_x * scale - this->getContentSize().width;
    float arTop = max_y * scale - this->getContentSize().height;
    float arBottom = min_y * scale;

    if (left < min_x) {
      Node::setPosition(arLeft, this->getPosition().y);
    }

    if (right > max_x) {
      Node::setPosition(arRight, this->getPosition().y);
    }

    if (top > max_y) {
      Node::setPosition(this->getPosition().x, arTop);
    }

    if (bottom < min_y) {
      Node::setPosition(this->getPosition().x, arBottom);
    }
  }
}

void PanZoomLayer::setScale(float scale) {
  LayerColor::setScale(MIN(MAX(scale, _minScale), _maxScale));
  this->setPosition(this->getPosition());
}

void PanZoomLayer::SetPanBoundsRect(Rect rect) {
  _panBoundsRect = rect;

  Size visibleSize = Director::getInstance()->getVisibleSize();
  float wFactor = _panBoundsRect.size.width / visibleSize.width;
  float hFactor = _panBoundsRect.size.height / visibleSize.height;
  float minScale;
  if (wFactor > hFactor)
    minScale = wFactor;
  else
    minScale = hFactor;
  SetMinScale(minScale);
}

void PanZoomLayer::SetMaxScale(float maxScale) {
  _maxScale = maxScale;
}

float PanZoomLayer::GetMaxScale() {
  return _maxScale;
}

void PanZoomLayer::SetMinScale(float minScale) {
  _minScale = minScale;
}

float PanZoomLayer::GetMinScale() {
  return _minScale;
}

void PanZoomLayer::Holding() {
  _isHolding = true;
}

void PanZoomLayer::UnHolding() {
  _isHolding = false;
}

void PanZoomLayer::SetProductFactor(float v) {
  _productFactor = v;
}
