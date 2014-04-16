#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "TVout.h"

extern void refresh();
extern void TVsetup();
extern int pollEvent();
extern int pollFire();
extern int pollLeft();
extern int pollRight();
extern int pollUp();
extern int pollDown();
extern void myTone(int,int);

extern TVout TV;
extern sf::Texture TVtexture;
extern sf::Sprite TVsprite;
extern sf::Sound mysound;
extern sf::SoundBuffer sndbuffer;
extern unsigned int windowWidth, windowHeight, viewWidth, viewHeight;
extern float viewZoom;
extern sf::RectangleShape myPixel;
extern sf::RenderWindow window;
extern sf::View tv;
extern sf::Event event;
extern bool keyLeft, keyRight, keyUp, keyDown, keyFire;

#define PI 3.14159265
