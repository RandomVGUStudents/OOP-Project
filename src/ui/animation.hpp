#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include <array>
#include <cmath>

constexpr int ANIMATION_SAMPLES = 180;
constexpr float ANIMATION_DURATION = 3.0f;
constexpr float TIME_STEP = ANIMATION_DURATION / (ANIMATION_SAMPLES - 1);

constexpr float easeOutCubic(float t)
{
    return 1 - std::pow(1 - t / ANIMATION_DURATION, 3);
}

constexpr std::array<float, ANIMATION_SAMPLES> bakeAnimation()
{
    std::array<float, ANIMATION_SAMPLES> data = {};
    for (size_t i = 0; i < ANIMATION_SAMPLES; ++i)
    {
        float time = i * TIME_STEP;
        data[i] = easeOutCubic(time);
    }
    return data;
}

constexpr std::array<float, ANIMATION_SAMPLES> ANIMATION_DATA = bakeAnimation();
#endif /* ANIMATION_HPP */
