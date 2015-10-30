#include <SDL/SDL.h>
#include <iostream>
#include "displayer.h"

/**
 * This class creates a screen and manages the modifications. It uses SDL 1.2
 */

Displayer::Displayer():
    width(0), height(0), initialized(false), screen(NULL)
{
}

bool Displayer::initialize(unsigned int width, unsigned int height,
        const char *name)
{
    this->width = width;
    this->height = height;

    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Unable to init SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    screen = SDL_SetVideoMode(width, height, 32, SDL_HWSURFACE);
    if(!screen)
    {
        std::cerr << "Unable to set" << width << "x" << height << "video: ";
        std::cerr << SDL_GetError() << std::endl;
        return false;
    }
    SDL_WM_SetCaption(name, "");
    initialized = true;
    return true;
}

void Displayer::destroy()
{
    SDL_FreeSurface(screen);
    SDL_Quit();
}

bool Displayer::refresh()
{
    return (SDL_Flip(screen) != -1);
}

bool Displayer::setColor(unsigned int red, unsigned int green, unsigned int blue,
        SDL_Rect rect)
{
    if(screen == NULL || colorImpossible(red, green, blue))
        return false;

    SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, red, green, blue));
    return true;
}

bool Displayer::setColor(unsigned int red, unsigned int green, unsigned int blue,
        unsigned int x, unsigned int y)
{
    SDL_Rect rectangle;
    rectangle.x = x;
    rectangle.y = y;
    rectangle.w = 1;
    rectangle.h = 1;

    return setColor(red, green, blue, rectangle);
}

bool Displayer::colorImpossible(unsigned int red, unsigned int green,
        unsigned int blue)
{
    return (red > 255 || green > 255 || blue > 255);
}

bool Displayer::outside(int x, int y)
{
    return (x < 0 || static_cast<unsigned int>(x) >= width ||
            y < 0 || static_cast<unsigned int>(y) >= height);
}

unsigned int Displayer::getWidth()
{
    return width;
}

unsigned int Displayer::getHeight()
{
    return height;
}

bool Displayer::isInitialized()
{
    return initialized;
}
