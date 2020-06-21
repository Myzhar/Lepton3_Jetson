#include "Palettes.h"
#include <math.h>
#include <iostream>

using namespace std;

uint8_t colormap_byr[LUT_SIZE_8*3];
uint8_t colormap_bry[LUT_SIZE_8*3];
uint8_t colormap_bgyr[LUT_SIZE_8*3];
uint8_t colormap_whitehot[LUT_SIZE_8*3];
uint8_t colormap_blackhot[LUT_SIZE_8*3];
uint8_t colormap_whitehotBYR[LUT_SIZE_8*3];
uint8_t colormap_whitehotBRY[LUT_SIZE_8*3];

const uint8_t* palettes[PALETTES_COUNT] =
{
    colormap_byr,
    colormap_bry,
    colormap_bgyr,
    colormap_whitehot,
    colormap_blackhot,
    colormap_whitehotBYR,
    colormap_whitehotBRY
};

// From http://www.andrewnoske.com/wiki/Code_-_heatmaps_and_color_gradients
void getHeatMapColorBYR(double value, double& red, double& green, double& blue)
{
    const int NUM_COLORS = 3;
    static double color[NUM_COLORS][3] = { {0,0,1}, {1,1,0}, {1,0,0} };
    // A static array of 3 colors:  (blue, yellow, red) using {r,g,b} for each.

    int idx1;        // |-- Our desired color will be between these two indexes in "color".
    int idx2;        // |
    double fractBetween = 0;  // Fraction between "idx1" and "idx2" where our value is.

    if(value <= 0) // accounts for an input <=0
    {
        idx1 = idx2 = 0;
    }
    else if(value >= 1) // accounts for an input >=0
    {
        idx1 = idx2 = NUM_COLORS-1;
    }
    else
    {
        value = value * (NUM_COLORS-1);        // Will multiply value by NUM_COLORS (-1).
        idx1  = floor(value);                  // Our desired color will be after this index.
        idx2  = idx1+1;                        // ... and before this index (inclusive).
        fractBetween = value - static_cast<double>(idx1);    // Distance between the two indexes (0-1).
    }

    red   = (color[idx2][0] - color[idx1][0])*fractBetween + color[idx1][0];
    green = (color[idx2][1] - color[idx1][1])*fractBetween + color[idx1][1];
    blue  = (color[idx2][2] - color[idx1][2])*fractBetween + color[idx1][2];
}

void getHeatMapColorBRY(double value, double& red, double& green, double& blue)
{
    const int NUM_COLORS = 3;
    static double color[NUM_COLORS][3] = { {0,0,1}, {1,0,0}, {1,1,0} };
    // A static array of 3 colors:  (blue, red, yellow ) using {r,g,b} for each.

    int idx1;        // |-- Our desired color will be between these two indexes in "color".
    int idx2;        // |
    double fractBetween = 0;  // Fraction between "idx1" and "idx2" where our value is.

    if(value <= 0) // accounts for an input <=0
    {
        idx1 = idx2 = 0;
    }
    else if(value >= 1) // accounts for an input >=0
    {
        idx1 = idx2 = NUM_COLORS-1;
    }
    else
    {
        value = value * (NUM_COLORS-1);        // Will multiply value by NUM_COLORS (-1).
        idx1  = floor(value);                  // Our desired color will be after this index.
        idx2  = idx1+1;                        // ... and before this index (inclusive).
        fractBetween = value - static_cast<double>(idx1);    // Distance between the two indexes (0-1).
    }

    red   = (color[idx2][0] - color[idx1][0])*fractBetween + color[idx1][0];
    green = (color[idx2][1] - color[idx1][1])*fractBetween + color[idx1][1];
    blue  = (color[idx2][2] - color[idx1][2])*fractBetween + color[idx1][2];
}

void getHeatMapColorBGYR(double value, double& red, double& green, double& blue)
{
    const int NUM_COLORS = 4;
    static double color[NUM_COLORS][3] = { {0,0,1}, {0,1,0}, {1,1,0}, {1,0,0} };
    // A static array of 4 colors:  (blue, green, yellow, red) using {r,g,b} for each.

    int idx1;        // |-- Our desired color will be between these two indexes in "color".
    int idx2;        // |
    double fractBetween = 0;  // Fraction between "idx1" and "idx2" where our value is.

    if(value <= 0) // accounts for an input <=0
    {
        idx1 = idx2 = 0;
    }
    else if(value >= 1) // accounts for an input >=0
    {
        idx1 = idx2 = NUM_COLORS-1;
    }
    else
    {
        value = value * (NUM_COLORS-1);        // Will multiply value by NUM_COLORS (-1).
        idx1  = floor(value);                  // Our desired color will be after this index.
        idx2  = idx1+1;                        // ... and before this index (inclusive).
        fractBetween = value - static_cast<double>(idx1);    // Distance between the two indexes (0-1).
    }

    red   = (color[idx2][0] - color[idx1][0])*fractBetween + color[idx1][0];
    green = (color[idx2][1] - color[idx1][1])*fractBetween + color[idx1][1];
    blue  = (color[idx2][2] - color[idx1][2])*fractBetween + color[idx1][2];
}

void getHeatMapColorWhiteHot(double value, double& red, double& green, double& blue)
{
    const int NUM_COLORS = 2;
    static double color[NUM_COLORS][3] = { {0,0,0}, {1,1,1} };
    // A static array of 2 colors:  (black, white) using {r,g,b} for each.

    int idx1;        // |-- Our desired color will be between these two indexes in "color".
    int idx2;        // |
    double fractBetween = 0;  // Fraction between "idx1" and "idx2" where our value is.

    if(value <= 0) // accounts for an input <=0
    {
        idx1 = idx2 = 0;
    }
    else if(value >= 1) // accounts for an input >=0
    {
        idx1 = idx2 = NUM_COLORS-1;
    }
    else
    {
        value = value * (NUM_COLORS-1);        // Will multiply value by NUM_COLORS (-1).
        idx1  = floor(value);                  // Our desired color will be after this index.
        idx2  = idx1+1;                        // ... and before this index (inclusive).
        fractBetween = value - static_cast<double>(idx1);    // Distance between the two indexes (0-1).
    }

    red   = (color[idx2][0] - color[idx1][0])*fractBetween + color[idx1][0];
    green = (color[idx2][1] - color[idx1][1])*fractBetween + color[idx1][1];
    blue  = (color[idx2][2] - color[idx1][2])*fractBetween + color[idx1][2];
}

void getHeatMapColorBlackHot(double value, double& red, double& green, double& blue)
{
    const int NUM_COLORS = 2;
    static double color[NUM_COLORS][3] = { {1,1,1}, {0,0,0} };
    // A static array of 2 colors:  (black, white) using {r,g,b} for each.

    int idx1;        // |-- Our desired color will be between these two indexes in "color".
    int idx2;        // |
    double fractBetween = 0;  // Fraction between "idx1" and "idx2" where our value is.

    if(value <= 0) // accounts for an input <=0
    {
        idx1 = idx2 = 0;
    }
    else if(value >= 1) // accounts for an input >=0
    {
        idx1 = idx2 = NUM_COLORS-1;
    }
    else
    {
        value = value * (NUM_COLORS-1);        // Will multiply value by NUM_COLORS (-1).
        idx1  = floor(value);                  // Our desired color will be after this index.
        idx2  = idx1+1;                        // ... and before this index (inclusive).
        fractBetween = value - static_cast<double>(idx1);    // Distance between the two indexes (0-1).
    }

    red   = (color[idx2][0] - color[idx1][0])*fractBetween + color[idx1][0];
    green = (color[idx2][1] - color[idx1][1])*fractBetween + color[idx1][1];
    blue  = (color[idx2][2] - color[idx1][2])*fractBetween + color[idx1][2];
}

void getHeatMapColorWhiteHotBYR(double value, double& red, double& green, double& blue)
{
    const int NUM_COLORS = 5;
    static double color[NUM_COLORS][3] = { {0,0,0}, {0,0,1}, {1,1,0}, {1,0,0}, {1,1,1} };
    // A static array of 5 colors:  (black, blue, yellow, red, white) using {r,g,b} for each.

    int idx1;        // |-- Our desired color will be between these two indexes in "color".
    int idx2;        // |
    double fractBetween = 0;  // Fraction between "idx1" and "idx2" where our value is.

    if(value <= 0) // accounts for an input <=0
    {
        idx1 = idx2 = 0;
    }
    else if(value >= 1) // accounts for an input >=0
    {
        idx1 = idx2 = NUM_COLORS-1;
    }
    else
    {
        value = value * (NUM_COLORS-1);        // Will multiply value by NUM_COLORS (-1).
        idx1  = floor(value);                  // Our desired color will be after this index.
        idx2  = idx1+1;                        // ... and before this index (inclusive).
        fractBetween = value - static_cast<double>(idx1);    // Distance between the two indexes (0-1).
    }

    red   = (color[idx2][0] - color[idx1][0])*fractBetween + color[idx1][0];
    green = (color[idx2][1] - color[idx1][1])*fractBetween + color[idx1][1];
    blue  = (color[idx2][2] - color[idx1][2])*fractBetween + color[idx1][2];
}

void getHeatMapColorWhiteHotBRY(double value, double& red, double& green, double& blue)
{
    const int NUM_COLORS = 5;
    static double color[NUM_COLORS][3] = { {0,0,0}, {0,0,1}, {1,0,0}, {1,1,0}, {1,1,1} };
    // A static array of 5 colors:  (black, blue, red, yellow, white) using {r,g,b} for each.

    int idx1;        // |-- Our desired color will be between these two indexes in "color".
    int idx2;        // |
    double fractBetween = 0;  // Fraction between "idx1" and "idx2" where our value is.

    if(value <= 0) // accounts for an input <=0
    {
        idx1 = idx2 = 0;
    }
    else if(value >= 1) // accounts for an input >=0
    {
        idx1 = idx2 = NUM_COLORS-1;
    }
    else
    {
        value = value * (NUM_COLORS-1);        // Will multiply value by NUM_COLORS (-1).
        idx1  = floor(value);                  // Our desired color will be after this index.
        idx2  = idx1+1;                        // ... and before this index (inclusive).
        fractBetween = value - static_cast<double>(idx1);    // Distance between the two indexes (0-1).
    }

    red   = (color[idx2][0] - color[idx1][0])*fractBetween + color[idx1][0];
    green = (color[idx2][1] - color[idx1][1])*fractBetween + color[idx1][1];
    blue  = (color[idx2][2] - color[idx1][2])*fractBetween + color[idx1][2];
}

void createColorMaps()
{
    double r,g,b;
    uint8_t r8,g8,b8;

    for( int i=0; i<LUT_SIZE_8; i++  )
    {
        double val = static_cast<double>(i)/LUT_SIZE_8;

        // >>>>> BLU YELLOW RED
        getHeatMapColorBYR( val, r,g,b );
        r8=static_cast<uint8_t>(r*255+0.5);
        g8=static_cast<uint8_t>(g*255+0.5);
        b8=static_cast<uint8_t>(b*255+0.5);

        colormap_byr[i*3+0] = r8;
        colormap_byr[i*3+1] = g8;
        colormap_byr[i*3+2] = b8;
        // <<<<< BLU YELLOW RED

        // >>>>> BLU RED YELLOW
        getHeatMapColorBRY( val, r,g,b );
        r8=static_cast<uint8_t>(r*255+0.5);
        g8=static_cast<uint8_t>(g*255+0.5);
        b8=static_cast<uint8_t>(b*255+0.5);

        colormap_bry[i*3+0] = r8;
        colormap_bry[i*3+1] = g8;
        colormap_bry[i*3+2] = b8;
        // <<<<< BLU RED YELLOW

        // >>>>> BLU GREEN YELLOW RED
        getHeatMapColorBGYR( val, r,g,b );

        r8=static_cast<uint8_t>(r*255+0.5);
        g8=static_cast<uint8_t>(g*255+0.5);
        b8=static_cast<uint8_t>(b*255+0.5);

        colormap_bgyr[i*3+0] = r8;
        colormap_bgyr[i*3+1] = g8;
        colormap_bgyr[i*3+2] = b8;
        // <<<<< BLU GREEN YELLOW RED

        // >>>>> WhiteHot
        getHeatMapColorWhiteHot( val, r,g,b );

        r8=static_cast<uint8_t>(r*255+0.5);
        g8=static_cast<uint8_t>(g*255+0.5);
        b8=static_cast<uint8_t>(b*255+0.5);

        colormap_whitehot[i*3+0] = r8;
        colormap_whitehot[i*3+1] = g8;
        colormap_whitehot[i*3+2] = b8;
        // <<<<< WhiteHot

        // >>>>> BlackHot
        getHeatMapColorBlackHot( val, r,g,b );

        r8=static_cast<uint8_t>(r*255+0.5);
        g8=static_cast<uint8_t>(g*255+0.5);
        b8=static_cast<uint8_t>(b*255+0.5);

        colormap_blackhot[i*3+0] = r8;
        colormap_blackhot[i*3+1] = g8;
        colormap_blackhot[i*3+2] = b8;
        // <<<<< BlackHot

        // >>>>> WhiteHot BYR
        getHeatMapColorWhiteHotBYR( val, r,g,b );

        r8=static_cast<uint8_t>(r*255+0.5);
        g8=static_cast<uint8_t>(g*255+0.5);
        b8=static_cast<uint8_t>(b*255+0.5);

        colormap_whitehotBYR[i*3+0] = r8;
        colormap_whitehotBYR[i*3+1] = g8;
        colormap_whitehotBYR[i*3+2] = b8;
        // <<<<< WhiteHot BYR

        // >>>>> WhiteHot BRY
        getHeatMapColorWhiteHotBRY( val, r,g,b );

        r8=static_cast<uint8_t>(r*255+0.5);
        g8=static_cast<uint8_t>(g*255+0.5);
        b8=static_cast<uint8_t>(b*255+0.5);

        colormap_whitehotBRY[i*3+0] = r8;
        colormap_whitehotBRY[i*3+1] = g8;
        colormap_whitehotBRY[i*3+2] = b8;
        // <<<<< WhiteHot BRY
    }
}






