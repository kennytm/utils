#include <cstdlib>
#include <utils/ext/xterm256.hpp>
#include <ostream>

namespace utils { namespace xterm256 {

static const uint8_t rgb_intensities[] = {0x00, 0x5f, 0x87, 0xaf, 0xd7, 0xff};

color get_color(uint8_t index) noexcept
{
    color retval;

    // ANSI low intensity
    if (index < 8)
    {
        retval.r = (index & 1) ? 0xc0 : 0;
        retval.g = (index & 2) ? 0xc0 : 0;
        retval.b = (index & 4) ? 0xc0 : 0;
    }
    // ANSI high intensity
    else if (index < 16)
    {
        retval.r = (index & 1) ? 0xff : 0x80;
        retval.g = (index & 2) ? 0xff : 0x80;
        retval.b = (index & 4) ? 0xff : 0x80;
    }
    // RGB
    else if (index < 216+16)
    {
        size_t i = index - 16u;
        size_t blue_index = i % 6;
        i /= 6;
        size_t green_index = i % 6;
        size_t red_index = i / 6;
        retval.r = rgb_intensities[red_index];
        retval.g = rgb_intensities[green_index];
        retval.b = rgb_intensities[blue_index];
    }
    // Gray scale
    else
    {
        uint8_t intensity = static_cast<uint8_t>((index - (216u+16u)) * 10u + 8u);
        retval.r = retval.g = retval.b = intensity;
    }

    return retval;
}

struct inferred_index
{
    int diff;
    uint8_t index;
};

static inferred_index infer_ansi(color c) noexcept
{
    inferred_index retval;
    retval.diff = c.r + c.g + c.b;
    retval.index = 0;

    for (uint8_t i = 1; i < 16; ++ i)
    {
        auto ac = get_color(i);
        auto diff = abs(c.r - ac.r) + abs(c.g - ac.g) + abs(c.b - ac.b);
        if (diff < retval.diff)
        {
            retval.diff = diff;
            retval.index = i;
        }
    }

    return retval;
}

static inferred_index infer_rgb_component(uint8_t comp) noexcept
{
    inferred_index retval;
    retval.diff = comp;
    retval.index = 0;
    for (uint8_t i = 1; i < 6; ++ i)
    {
        auto diff = abs(rgb_intensities[i] - comp);
        if (diff < retval.diff)
        {
            retval.diff = diff;
            retval.index = i;
        }
    }
    return retval;
}

static inferred_index infer_rgb(color c) noexcept
{
    auto r = infer_rgb_component(c.r);
    auto g = infer_rgb_component(c.g);
    auto b = infer_rgb_component(c.b);
    uint8_t real_index = static_cast<uint8_t>(r.index*36u + g.index*6u + b.index + 16u);
    return {r.diff + g.diff + b.diff, real_index};
}

static inferred_index infer_gray(color c) noexcept
{
    auto sum24 = c.r + c.g + c.b - 24;
    // solve: 24 + 30n == sum
    auto index = (sum24 + 15) / 30;
    if (index < 0)
        index = 0;
    if (index > 23)
        index = 23;
    auto comp = 8 + index*10;
    auto diff = abs(comp - c.r) + abs(comp - c.g) + abs(comp - c.b);
    return {diff, static_cast<uint8_t>(index + 216+16)};
}

uint8_t get_index(color c) noexcept
{
    inferred_index indices[] = {infer_ansi(c), infer_rgb(c), infer_gray(c)};
    if (indices[1].diff < indices[0].diff)
        indices[0] = indices[1];
    if (indices[2].diff < indices[0].diff)
        indices[0] = indices[2];
    return indices[0].index;
}

std::ostream& operator<<(std::ostream& os, color c)
{
    unsigned r = c.r, g = c.g, b = c.b;
    return os << "{" << r << ", " << g << ", " << b << "}";
}

}}

