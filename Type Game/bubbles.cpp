/*

* author		Wang Yang
* E-mail		wysaid@gmail.com OR admin@wysaid.org
* blog			http://blog.wysaid.org
* date			2012-7-29

*/

#include "bubbles.h"

float Ball::s_direction;
Scene* g_bubbleScene = NULL;

Ball::Ball()
{
	init();
}

void Ball::init()
{
	m_x = (SCR_WIDTH / 2);
	m_y = (SCR_HEIGHT / 2);
	m_speed = g_hge->Random_Float(1.0f, 3.0f);
	m_dx = m_speed * cosf(s_direction);
	m_dy = m_speed * sinf(s_direction);
	m_radius = 16;
	m_color = g_hge->Random_Int(0x7fff0000, 0xffffffff);
	s_direction += 0.1f;
}

void Ball::update()
{
	m_x += m_dx;
	m_y += m_dy;

	if(m_x < .0f || m_x > SCR_WIDTH)
	{
		m_dx = -m_dx;
		m_x += m_dx;
	}
	if(m_y < .0f || m_y > SCR_HEIGHT)
	{
		m_dy = -m_dy;
		m_y += m_dy;
	}

#ifdef _DEBUG

	if(m_x < .0f || m_x > SCR_WIDTH || m_y < .0f || m_y > SCR_HEIGHT)
		g_hge->System_SetState(HGE_TITLE, "检测到某些点出界了!!");

#endif
}

void Ball::render()
{
	g_bubbleScene->renderBubble(m_x, m_y, m_color);
}

void Ball::turn(float x, float y)
{
	float aglBall = getAngleByNormal(-m_dx, -m_dy);
	float aglLine = getAngleByNormal(x, y);
	float angle = 2 * aglLine - aglBall;
	m_dx = cosf(angle) * m_speed;
	m_dy = sinf(angle) * m_speed;
	m_x += m_dx;
	m_y += m_dy;
}

Line::Line(float x1, float y1, float x2, float y2)
	: m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2), m_maxx(x1), m_minx(x2), m_maxy(y1), m_miny(y2)
{
	if(m_maxx < m_minx) std::swap(m_maxx, m_minx);
	if(m_maxy < m_miny) std::swap(m_maxy, m_miny);
	setLineEquation();
}

void Line::setLineEquation()  //ax + by + c = 0
{
	float a = m_y1 - m_y2, b = m_x1 - m_x2, c = m_x1 * m_y2 - m_x2 * m_y1;
	if(FLOATCOMP0(b))
	{
		m_a = 1;
		m_b = 0.0f;
		m_c = -m_x1;
	}
	else
	{
		m_a = a / b;
		m_b = -1.0f;
		m_c = c / b;
	}
	m_d2 = m_a * m_a + m_b * m_b;
	m_d = sqrtf(m_d2);
}

void Line::collisionDetect(Ball& ball)
{
	float x,y;
	ball.getPos(x,y);
	float distance = fabs(m_a * x + m_b * y + m_c) / m_d;

	if(distance < ball.getRadius() )
	{
		float prjX = (m_b*m_b*x - m_a*m_b*y - m_a*m_c) / m_d2;
		float prjY = (m_a*m_a*y - m_b*m_c - m_a*m_b*x) / m_d2;
		if(prjX >= m_minx && prjX <= m_maxx && prjY >= m_miny && prjY <= m_maxy)
		{
			ball.turn(m_a, m_b);
		}
		else
		{		// Other endpoint collision detection
			
		}
	}
}

Scene::Scene(HTARGET bgtar) : m_bUpdate(true)
{
	m_bgtar = bgtar;
	init(g_hge->Target_GetTexture(bgtar));
}

Scene::~Scene()
{
	g_hge->Texture_Free(m_sprtex);
	g_hge->Target_Free(m_bgtar);
	delete m_bubble;
	delete m_cursor;
	delete m_bgspr;
	delete m_par;
}

void Scene::init(HTEXTURE bgtex)
{
	m_bgtex = bgtex;
	m_bgspr = new hgeSprite(m_bgtex, 0, 0, 800, 600);
	m_bgspr->SetBlendMode(BLEND_DEFAULT);
	m_bgspr->SetColor(0xffff7f7f);
	m_activeBallNum = 0;
	m_isLbuttonDown = false;
	m_sprtex = g_hge->Texture_Load("res/particles.png");
	m_bubble = new hgeSprite(m_sprtex, 96,64,32,32);
	m_bubble->SetBlendMode(BLEND_ALPHAADD | BLEND_COLORMUL | BLEND_NOZWRITE);

	m_bubble->SetHotSpot(16,16);
	m_bubble->SetColor(0x7fd00000);

	m_cursor = new hgeSprite(m_sprtex, 64, 96, 32, 32);

	m_cursor->SetBlendMode(BLEND_ALPHAADD | BLEND_COLORMUL | BLEND_NOZWRITE);
	m_cursor->SetHotSpot(16,16);

	m_par = new hgeParticleSystem("res/particle6.psi", m_cursor);
	m_par->Fire();
	m_x1 = 400; m_y1 = 300;
	m_cursor->SetColor(0xffffff00);
}

void Scene::update()
{
	static bool bFirstTime = true; 
	float dt = g_hge->Timer_GetDelta();
	m_par->Update(dt);
	g_hge->Input_GetMousePos(&m_x1, &m_y1);
	m_par->MoveTo(m_x1, m_y1);

	int key;
	if(bFirstTime && m_activeBallNum < MAX_OBJECTS) ++m_activeBallNum;
	if(key = g_hge->Input_GetKey())
	{
		switch(key)
		{
		case HGEK_LBUTTON:
			{
				if(g_hge->Input_GetKeyState(HGEK_LBUTTON))
				{
					m_x0 = m_x1;
					m_y0 = m_y1;
					m_isLbuttonDown = true;
				}
			}
			break;
		case HGEK_SPACE:
			bFirstTime = false;
			m_bUpdate = !m_bUpdate; 
			break;
		case HGEK_ADD: bFirstTime = false; if((m_activeBallNum += 10) > MAX_OBJECTS) m_activeBallNum = MAX_OBJECTS; break;
		case HGEK_SUBTRACT: bFirstTime = false; if((m_activeBallNum -= 10) < MIN_OBJECTS) m_activeBallNum = MIN_OBJECTS; break;
		case HGEK_BACKSPACE: case HGEK_DELETE:
			if(!m_vec.empty()) m_vec.pop_back();
			break;
		case HGEK_ESCAPE: 
		case HGEK_ENTER:
			g_game->resetEscape();
			g_hge->System_SetState(HGE_FRAMEFUNC, menuFrameFunc);
			g_hge->System_SetState(HGE_RENDERFUNC, menuRenderFunc);
			g_fnt1->SetScale(1.0f);
			g_gui->Enter();
			g_hge->Channel_StopAll();
			g_hge->Effect_Play(g_effect[HAPPY]);
			break;
		default:
			;
		}
	}
}

void Scene::show()
{
	m_bgspr->Render(0, 0);
	for(int i = 0; i != m_activeBallNum; ++i)
	{
		if(m_bUpdate) m_ball[i].update();
		m_ball[i].render();
		for(std::vector<Line>::iterator iter = m_vec.begin(); iter != m_vec.end(); ++iter)
		{
			iter->collisionDetect(m_ball[i]);
		}
	}

	for(std::vector<Line>::iterator iter = m_vec.begin(); iter != m_vec.end(); ++iter)
	{
		iter->render();
	}


	if(g_hge->Input_GetKeyState(HGEK_LBUTTON))
	{
		g_hge->Gfx_RenderLine(m_x0, m_y0, m_x1, m_y1, 0xFFFF0000);
	}
	else if(m_isLbuttonDown)
	{
		if(m_x0 != m_x1 && m_y0 != m_y1)
			m_vec.push_back(Line(m_x0, m_y0, m_x1, m_y1));
		m_isLbuttonDown = false;
	}
	m_par->Render();
}

void Scene::renderBubble(float x, float y, DWORD color)
{
	m_bubble->SetColor(color);
	m_bubble->Render(x, y);
}

float getAngleByNormal(float x, float y)
{
	float d = sqrtf(x*x+y*y);
	if(FLOATCOMP0(d)) return .0f;
	float angle = asinf(y / d);
	if(x > .0f)
	{
		if(y > .0f)	return angle;
		else return PI2 + angle;
	}
	else
	{
		return PI - angle;
	}
}

bool initBubbles(HTARGET htar)
{
	delete g_bubbleScene;
	g_bubbleScene = new Scene(htar);
	return true;
}

bool bubbleFrameFunc()
{
	g_bubbleScene->update();
	return false;
}

bool bubbleRenderFunc()
{
	g_hge->Gfx_BeginScene();
	g_hge->Gfx_Clear(0);
	g_bubbleScene->show();
	g_hge->Gfx_EndScene();
	return false;
}