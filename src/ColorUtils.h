/*  ______                              ___
   / ____/_______  ___  ____  __  __   /   |  ________  ____  ____ _
  / /_  / ___/ _ \/ _ \/_  / / / / /  / /| | / ___/ _ \/ __ \/ __ `/
 / __/ / /  /  __/  __/ / /_/ /_/ /  / ___ |/ /  /  __/ / / / /_/ /
/_/ __/_/___\___/\___/ /___/\__, /  /_/  |_/_/   \___/_/ /_/\__,_/
   / ____/ ___// /_____  __/____/___
  / __/  \__ \/ __/ __ \/ __ \/ ___/
 / /___ ___/ / /_/ /_/ / /_/ (__  )
/_____//____/\__/\____/ .___/____/
                     /_/
*/
#ifndef COLORUTILS_H
#define COLORUTILS_H

#define FASTLED_INTERNAL // Suppress build banner
#include <FastLED.h>

inline CRGB toRGBColor(String color) {
    CRGB rgbColor = CRGB::White;
    if (color == "black")
        rgbColor = CRGB::Black;
    else if (color == "red")
        rgbColor = CRGB::Red;
    else if (color == "blue")
        rgbColor = CRGB::Blue;
    else if (color == "orange")
        rgbColor = CRGB::Orange;
    else if (color == "green")
        rgbColor = CRGB::Green;
    else if (color == "yellow")
        rgbColor = CRGB::Yellow;
    else if (color == "purple")
        rgbColor = CRGB::Purple;
    else if (color == "white")
        rgbColor = CRGB::White;
    else if (color == "teal")
        rgbColor = CRGB::Teal;
    else if (color == "navy")
        rgbColor = CRGB::Navy;
    else if (color == "magenta")
        rgbColor = CRGB::Magenta;
    else if (color == "violet")
        rgbColor = CRGB::Violet;
    else if (color == "orangered")
        rgbColor = CRGB::OrangeRed;
    else if (color == "darkred")
        rgbColor = CRGB::DarkRed;
    else if (color == "amber")
        rgbColor = CRGB(255, 191, 0);
    return rgbColor;
}

#endif // COLORUTILS_H
