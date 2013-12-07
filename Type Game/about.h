/*

* author		Wang Yang
* E-mail		wysaid@gmail.com OR admin@wysaid.org
* blog			http://blog.wysaid.org
* date			2012-03-16

*/

#ifndef _ABOUT_H_
#define _ABOUT_H_

#include "wysaid.h"

#define WYSAID 5

struct Author
{
	Author();
	void frameUpdate();

	float x, y;
	float dx, dy;
	float scale, rot;
	float dscale, drot;
	DWORD color;
};

class About
{
public:
	About();
	void frame();
	void render();
	~About();
private:
	Author m_wysaid[WYSAID];
	HTEXTURE m_aboutTex, m_wysaidTex;
	hgeSprite *m_aboutSpr, *m_wysaidSpr;
};

#endif