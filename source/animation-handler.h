#pragma once


#include <SFML/Graphics.hpp>

#include "animation.h"


using std::vector;
using sf::IntRect;


class AnimationHandler
{
public:
    AnimationHandler();
    AnimationHandler(const IntRect& frame_size);

    void addAnimation(const Animation& animation);
    void changeAnimation(unsigned int animation_index);
    void update(const float dt);


public:
    IntRect bounds;
    IntRect frame_size;


private:
    vector<Animation> _animations;
    unsigned int _current_animation_index;
    float _t;
};