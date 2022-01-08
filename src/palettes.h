#pragma once

#define FASTLED_INTERNAL
#include <FastLED.h>

DEFINE_GRADIENT_PALETTE (yankeeBrave_gp) {
      0,   255,  106,    0,   //Yellow
     64,   255,    0,  127,   //PINK
    128,   255,   38,    0,   //Orange
    255,    75,    0,  130    //Purple
};

DEFINE_GRADIENT_PALETTE (greenPinkPurple_gp) {
      0,    15,  245,  145,   //BLUEGREEN
    127,   255,    0,  127,   //PINK
    255,    75,    0,  130    //Purple
};

CRGBPalette16 yankeeBrave = yankeeBrave_gp;
CRGBPalette16 greenPinkPurple = greenPinkPurple_gp;
