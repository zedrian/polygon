#include "animation-handler.h"


AnimationHandler::AnimationHandler() :
        _t(0.0f),
        _current_animation_index(-1)
{ }

AnimationHandler::AnimationHandler(const IntRect& frame_size) :
        _t(0.0f),
        _current_animation_index(-1),
        frame_size(frame_size)
{ }


void AnimationHandler::addAnimation(const Animation& animation)
{
    _animations.push_back(animation);
}

void AnimationHandler::changeAnimation(unsigned int animation_index)
{
    if (_current_animation_index == animation_index)
        return;
    if (animation_index >= _animations.size())
        return;
    if (animation_index < 0)
        return;

    _current_animation_index = animation_index;

    auto rectangle = frame_size;
    rectangle.top = rectangle.height * animation_index;
    bounds = rectangle;

    _t = 0.0f;
}

void AnimationHandler::update(const float dt)
{
    if (_current_animation_index >= _animations.size())
        return;
    if (_current_animation_index < 0)
        return;

    auto duration = _animations[_current_animation_index].duration;

    if (static_cast<int>((_t + dt) / duration) > static_cast<int>(_t / duration))
    {
        auto frame_index = static_cast<int>((_t + dt) / duration);
        frame_index %= _animations[_current_animation_index].length();

        auto rectangle = frame_size;
        rectangle.left = rectangle.width * frame_index;
        rectangle.top = rectangle.height * _current_animation_index;

        bounds = rectangle;
    }

    _t += dt;
    if (_t > duration * _animations[_current_animation_index].length())
        _t = 0.0f;
}







