#include "animation.h"


Animation::Animation(unsigned int start_frame,
                     unsigned int end_frame,
                     float duration) :
        start_frame(start_frame),
        end_frame(end_frame),
        duration(duration)
{ }

unsigned int Animation::length() const
{
    return end_frame - start_frame + 1;
}