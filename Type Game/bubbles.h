/*

* author		Wang Yang
* E-mail		wysaid@gmail.com OR admin@wysaid.org
* blog			http://blog.wysaid.org
* date			2012-7-29

*/

#ifndef _BUBBLES_H_
#define _BUBBLES_H_

#include "wysaid.h"

#define MIN_OBJECTS	10
#define MAX_OBJECTS 500

#ifdef	PI
#undef	PI
#endif

#define	PI	3.14159f
#define PI2	(2*PI)

#define FLOATCOMP0(x)	(x < 0.01f && x > -0.01f)

using std::vector;

extern HGE* g_hge;

class Ball
{
public:
	Ball();
	void init();

	void update();

	void render();

	void turn(float, float);

	void getPos(float& x, float& y)	{ x = m_x; y = m_y; }

	float getRadius() { return m_radius; }

	~Ball() {}

private:
	float m_x, m_y;
	float m_dx, m_dy;
	float m_radius;
	float m_speed;
	DWORD m_color;
	static float s_direction;
};

class Line
{
public:
	Line() {}
	Line(float x1, float y1, float x2, float y2);
	~Line() {}

	void setLineEquation();

	void render() { g_hge->Gfx_RenderLine(m_x1, m_y1, m_x2, m_y2); }

	void collisionDetect(Ball& ball);

private:
	float m_x1, m_y1;
	float m_x2, m_y2;
	float m_maxx, m_maxy, m_minx, m_miny;
	float m_a, m_b, m_c, m_d, m_d2;
};

class Scene
{
public:
	Scene(HTARGET);
	~Scene();

	void init(HTEXTURE);

	void update();

	void show();

	void renderBubble(float x, float y, DWORD color);

private:
	HTARGET m_bgtar;
	HTEXTURE m_sprtex, m_bgtex;
	hgeSprite *m_bubble, *m_cursor, *m_bgspr;
	hgeParticleSystem *m_par;
	Ball m_ball[MAX_OBJECTS];
	vector<Line> m_vec;
	float m_x0,m_y0, m_x1, m_y1;
	int m_activeBallNum;
	bool m_isLbuttonDown, m_bUpdate;
};

float getAngleByNormal(float, float);

bool initBubbles(HTARGET);

bool bubbleFrameFunc();
bool bubbleRenderFunc();

#endif