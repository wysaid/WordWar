/*

* author		Wang Yang
* E-mail		wysaid@gmail.com OR admin@wysaid.org
* blog			http://blog.wysaid.org
* date			2012-03-16

*/

#include "about.h"

About* g_about;
extern hgeParticleSystem* g_particle;
extern hgeGUI* g_gui;


Author::Author()
{
	x = g_hge->Random_Float(0, SCR_WIDTH);
	y = g_hge->Random_Float(0, SCR_HEIGHT);
	dx = g_hge->Random_Float(-3.2f, 3.2f);
	dy = g_hge->Random_Float(-3.2f, 3.2f);
	scale = g_hge->Random_Float(0.5f, 2.0f);
	rot = g_hge->Random_Float(0, M_PI*2);
	dscale = g_hge->Random_Float(-1.0f, 1.0f);
	drot = g_hge->Random_Float(-1.0f, 1.0f);
	color = 0xffff0000 + g_hge->Random_Int(0x1010, 0xffff);
}

void Author::frameUpdate()
{
	if((x += dx) <0 || x > SCR_WIDTH)
		dx = -dx;
	if((y += dy) <0 || y > SCR_HEIGHT)
		dy = -dy;
	if((scale+=dscale*g_dt) < 0.5f || scale > 2.0f)
		dscale = -dscale;
	rot += drot*g_dt;
}

About::About()
{
	m_aboutTex = g_hge->Texture_Load("res/about.jpg");
	m_aboutSpr = new hgeSprite(m_aboutTex, 0, 0, 800, 600);
	m_aboutSpr->SetBlendMode(BLEND_COLORADD | BLEND_ALPHABLEND | BLEND_NOZWRITE);

	m_aboutSpr->SetColor(0xFF000000,0);
	m_aboutSpr->SetColor(0xFF000000,1);
	m_aboutSpr->SetColor(0xFF000040,2);
	m_aboutSpr->SetColor(0xFF000040,3);

	m_wysaidTex = g_hge->Texture_Load("res/wysaid.png");
	m_wysaidSpr = new hgeSprite(m_wysaidTex, 0, 0, 113, 161);
	m_wysaidSpr->SetBlendMode(BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_NOZWRITE);
	m_wysaidSpr->SetHotSpot(56, 80);
}

void About::frame()
{
	float x,y;
	g_hge->Input_GetMousePos(&x, &y);
	g_particle->MoveTo(x, y);
	g_particle->Update(g_dt);

	for(int i=0; i != WYSAID; ++i)
	{
		m_wysaid[i].frameUpdate();
	}
}

void About::render()
{
	m_aboutSpr->Render(0,0);
	for(int i = 0; i != WYSAID; ++i)
	{
		m_wysaidSpr->SetColor(m_wysaid[i].color);
		m_wysaidSpr->RenderEx(m_wysaid[i].x, m_wysaid[i].y, m_wysaid[i].rot, m_wysaid[i].scale);
	}
	g_particle->Render();
}

About::~About()
{
	g_hge->Texture_Free(m_aboutTex);
	g_hge->Texture_Free(m_wysaidTex);
	delete m_aboutSpr;
	delete m_wysaidSpr;
}

bool aboutFrameFunc()
{
	g_dt = g_hge->Timer_GetDelta();
	g_about->frame();
	return false;
}

bool aboutRenderFunc()
{
	g_hge->Gfx_BeginScene();
	g_about->render();
	g_hge->Gfx_EndScene();
	
	if(g_hge->Input_GetChar())
	{
		delete g_about;
		g_about = NULL;
		g_hge->System_SetState(HGE_FRAMEFUNC, menuFrameFunc);
		g_hge->System_SetState(HGE_RENDERFUNC, menuRenderFunc);
		g_gui->Enter();
	}
	return false;
}