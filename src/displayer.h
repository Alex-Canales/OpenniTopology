#ifndef _DISPLAYER_
#define _DISPLAYER_

#include <SDL/SDL.h>

/**
 * This class creates a screen and manages the modifications. It uses SDL 1.2
 */

class Displayer
{
public:
    Displayer();
    bool initialize(unsigned int width, unsigned int height, const char *name);
    void destroy();
    bool refresh();
    bool setColor(unsigned int red, unsigned int green, unsigned int blue,
        SDL_Rect rect);
    bool setColor(unsigned int red, unsigned int green, unsigned int blue,
        unsigned int x, unsigned int y);
    bool colorImpossible(unsigned int red, unsigned int green, unsigned int blue);
    bool outside(int x, int y);
    unsigned int getWidth();
    unsigned int getHeight();
    bool isInitialized();
private:
    unsigned int width;
    unsigned int height;
    bool initialized;
    SDL_Surface *screen;
};

#endif
