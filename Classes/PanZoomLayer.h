#ifndef __PANZOOMLAYER_H__
#define __PANZOOMLAYER_H__

#include "cocos2d.h"

using namespace cocos2d;

class PanZoomLayer : public LayerColor {

private:
  Vector<Touch *> _touches;
  Vec2 _beganTouchPoint;
  Vec2 _endedTouchPoint;
  Vec2 _deltaSum;
  Vec2 _prevDeltaPoint;
  double _accelerationFactor;
  timeval _timeStamp;
  Rect _panBoundsRect;
  float _maxScale;
  float _minScale;
  float _productFactor;
  bool _isHolding;

public:
  PanZoomLayer() {};
  virtual ~PanZoomLayer() {};

  static PanZoomLayer *create();

  virtual bool init();
  virtual void onEnter();
  virtual void onExit();

  virtual void update(float dt);

  virtual void onTouchesBegan(const std::vector<Touch *> &touches, Event *event);
  virtual void onTouchesMoved(const std::vector<Touch *> &touches, Event *event);
  virtual void onTouchesEnded(const std::vector<Touch *> &touches, Event *event);

  virtual void setPosition(Vec2 position);
  virtual void setScale(float scale);

  void SetPanBoundsRect(Rect rect);
  void SetMaxScale(float maxScale);
  float GetMaxScale();
  void SetMinScale(float minScale);
  float GetMinScale();

  void Holding();
  void UnHolding();

  void SetProductFactor(float v);
};

#endif // __PANZOOMLAYER_H__
