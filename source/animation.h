#pragma once


class Animation
{
public:
    Animation(unsigned int start_frame,
              unsigned int end_frame,
              float duration);

    unsigned int length() const;


public:
    unsigned int start_frame;
    unsigned int end_frame;
    float duration;
};