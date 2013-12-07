/*

* author		Wang Yang
* E-mail		wysaid@gmail.com OR admin@wysaid.org
* blog			http://blog.wysaid.org
* date			2012-03-16

*/

#include "lose.h"

LoseAction* g_lose = NULL;
extern TypeGame* g_game;
extern hgeGUI* g_gui;
extern HEFFECT g_effect[];

LoseAction::LoseAction() : m_exit(false), m_highScore(0)
{
	m_tex = g_hge->Texture_Load("res/lose.png");
	m_sprite = new hgeSprite(m_tex, 0, 0, 600, 480);
	m_sprite->SetHotSpot(300, 240);
	
	using std::fstream;
	fstream in("res/data.dat", fstream::in);
	if(!in)
	{
		in.open("res/data.dat", fstream::out);
		in.close();
		m_newHighScore = true;
	}
	else
	{
		m_highScore = 0.0f;
		getline(in, m_usrName);
		if(in.eof())
			m_newHighScore = true;
		in >> m_highScore;

		if(m_highScore < g_game->getScore())
		{
			m_newHighScore = true;
		}
		else m_newHighScore = false;
	}
	if(m_newHighScore)
	{
		m_usrName.clear();
		m_highScore = g_game->getScore();
	}
	g_hge->Channel_StopAll();
	g_hge->Effect_Play(g_effect[GAMEOVER]);
}

LoseAction::~LoseAction()
{
	g_hge->Texture_Free(m_tex);
	delete m_sprite;
}

void LoseAction::frame()
{
	int ch;
	if(ch = g_hge->Input_GetChar())
	{
		if(m_newHighScore)
		{
			if(isprint(ch) || ch == ' ')
			{
				if(m_usrName.size() < 64)
					m_usrName.push_back(ch);
			}
			else if(ch == 27 || ch == '\n' || ch == '\r')
			{
				if(m_usrName.empty())
					m_usrName = "wysaid";
				m_exit = true;
				saveHighScore();
			}
			else if(ch == 8)
			{
				if(m_newHighScore && !m_usrName.empty()) m_usrName.pop_back();
			}
		}
		else if(ch == 27 || ch == '\n' || ch == '\r' || ch == ' ')
		{
			m_exit = true;
		}
	}
}

void LoseAction::render()
{
	m_sprite->Render(SCR_WIDTH/2, SCR_HEIGHT/2);
	if(m_newHighScore)
	{
		g_fnt1->printf(100, 200, HGETEXT_LEFT, "New High Score %.2f !!!\n\nPlease input your name: %s", m_highScore, m_usrName.c_str());
	}
	else
	{
		g_fnt1->printf(100, 200, HGETEXT_LEFT, "The Highest Score: %.2f\n\nName: %s", m_highScore, m_usrName.c_str());
	}
}

bool loseFrameFunc()
{
	g_dt = g_hge->Timer_GetDelta();
	g_lose->frame();
	return false;
}

bool loseRenderFunc()
{
	g_hge->Gfx_Clear(0);
	g_hge->Gfx_BeginScene();
	g_game->stageRender();
	g_lose->render();
	g_hge->Gfx_EndScene();

	if(g_lose->readyToExit())
	{
		g_hge->System_SetState(HGE_FRAMEFUNC, menuFrameFunc);
		g_hge->System_SetState(HGE_RENDERFUNC, menuRenderFunc);
		g_gui->Enter();
		delete g_lose;
		g_lose = NULL;
		g_fnt1->SetScale(1.0f);
		g_hge->Effect_Play(g_effect[HAPPY]);
	}
	return false;
}

void LoseAction::saveHighScore()
{
	using std::fstream;
	fstream out("res/data.dat", fstream::out);
	out << m_usrName << std::endl;
	out << m_highScore;
}