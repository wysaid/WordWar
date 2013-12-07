/*

* author		Wang Yang
* E-mail		wysaid@gmail.com OR admin@wysaid.org
* blog			http://blog.wysaid.org
* date			2012-03-16

*/

#include "settings.h"

float g_bulletSpeed = BULLET_SPEED;
int g_maxPageWords = 20;
int g_wordsBlendMode = 2;
float g_bgSpeed = BG_SPEED;
extern float g_dt, g_delay;

Settings* g_set = NULL;

Settings::Settings() : m_choice(0), m_exit(false), m_backToMenu(false)
{
	m_tex = g_hge->Texture_Load("res/settingsbg.png");
	m_parTex = g_hge->Texture_Load("res/particles.png");
	m_sprite = new hgeSprite(m_tex, 0, 0, 600, 480);
	m_sprite->SetHotSpot(300, 240);
	m_parSpr = new hgeSprite(m_parTex, 32, 32, 32, 32);
	m_parSpr->SetHotSpot(16,16);
	m_parSpr->SetBlendMode(BLEND_COLORMUL | BLEND_ALPHAADD | BLEND_NOZWRITE);
	m_parSpr->SetColor(0xefffff00);
	m_par = new hgeParticleSystem("res/particle.psi", m_parSpr);
	m_par->MoveTo(250, 188);
	m_par->Fire();
}

Settings::~Settings()
{
	g_hge->Texture_Free(m_tex);
	g_hge->Texture_Free(m_parTex);
	delete m_parSpr;
	delete m_par;
	delete m_sprite;
}

void Settings::frame()
{
	int ch;
	m_par->Update(g_dt);
	if(ch = g_hge->Input_GetKey())
	{
		g_hge->Effect_Play(g_effect[TIMER]);
		switch(ch)
		{
		case HGEK_F1: m_choice = 0; m_par->MoveTo(250, 188); break;
		case HGEK_F2: m_choice = 1; m_par->MoveTo(250, 230); break;
		case HGEK_F3: m_choice = 2; m_par->MoveTo(250, 270); break;
		case HGEK_F4: m_choice = 3; m_par->MoveTo(250, 310); break;
		case HGEK_F5: m_choice = 4; m_par->MoveTo(250, 355); break;
		case HGEK_ADD: case HGEK_EQUALS: set(true); break;
		case HGEK_SUBTRACT: case HGEK_MINUS: set(false); break;

		case HGEK_ESCAPE:  m_backToMenu = true; //fall through
		case HGEK_ENTER: case HGEK_SPACE: m_exit = true;break;
		
		default: g_hge->Effect_Play(g_effect[WRONG]);
		}
	}	
}

void Settings::render()
{
	if(g_game && g_game->mode == PRACTICE) g_game->practiceRender();
	m_sprite->Render(SCR_WIDTH / 2, SCR_HEIGHT / 2);
	g_fnt1->printf(420, 180, HGETEXT_LEFT, "%g", g_bulletSpeed);
	g_fnt1->printf(420, 220, HGETEXT_LEFT, "%g", g_bgSpeed);
	g_fnt1->printf(480, 265, HGETEXT_LEFT, "%d", g_wordsBlendMode);
	g_fnt1->printf(470, 305, HGETEXT_LEFT, "%d", g_maxPageWords);
	g_fnt1->printf(500, 342, HGETEXT_LEFT, ": %g", g_delay);
	m_par->Render();
}

void Settings::set(bool bAdd)
{
	switch(m_choice)
	{
	case 0:
		if(bAdd)
		{
			if((g_bulletSpeed+=1.0f) > MAX_BUL_SPEED) g_bulletSpeed = MAX_BUL_SPEED;
		}
		else
		{
			if((g_bulletSpeed-=1.0f) < MIN_BUL_SPEED) g_bulletSpeed = MIN_BUL_SPEED;
		}
		break;
	case 1:
		if(bAdd)
		{
			if((g_bgSpeed+=0.1f) > MAX_BG_SPEED) g_bgSpeed = MAX_BG_SPEED;
		}
		else
		{
			if((g_bgSpeed-=0.1f) < 0) g_bgSpeed = 0;
		}
		break;
	case 2:
		if(bAdd)
		{
			if(++g_wordsBlendMode > BLEND_MODE_NUM) g_wordsBlendMode = BLEND_MODE_NUM;
		}
		else
		{
			if(--g_wordsBlendMode < 0) g_wordsBlendMode = 0;
		}
		if(g_game) g_game->changeWordsBlendMode(g_wordsBlendMode);
		break;
	case 3:
		if(bAdd)
		{
			if(++g_maxPageWords > MAX_WORD_NUM) g_maxPageWords = MAX_WORD_NUM;
		}
		else
		{
			if(--g_maxPageWords < MIN_WORD_NUM) g_maxPageWords = MIN_WORD_NUM;
		}
		break;
	case 4:
		if(bAdd)
		{
			if((g_delay+=0.1) > MAX_DELAY_TIME) g_delay = MAX_DELAY_TIME; 
		}
		else
		{
			if((g_delay-=0.1) < MIN_DELAY_TIME) g_delay = MIN_DELAY_TIME;
		}
	default:;
	}
}

bool settingFrame()
{
	g_dt = g_hge->Timer_GetDelta();
	g_set->frame();
	return false;
}

bool settingRender()
{
	g_hge->Gfx_Clear(0x3fd0d0d0);
	g_hge->Gfx_BeginScene();

	g_set->render();

	g_hge->Gfx_EndScene();

	if(g_set->nowExit())
	{
		if(g_game && g_game->mode == PRACTICE && !g_set->backToMenu())
		{
			g_hge->System_SetState(HGE_FRAMEFUNC,practiceFrameFunc);
			g_hge->System_SetState(HGE_RENDERFUNC,practiceRenderFunc);
			g_fnt1->SetScale(0.7f);
		}
		else
		{
			g_hge->Channel_StopAll();
			g_hge->Effect_Play(g_effect[HAPPY]);
			g_hge->System_SetState(HGE_FRAMEFUNC, menuFrameFunc);
			g_hge->System_SetState(HGE_RENDERFUNC, menuRenderFunc);
			g_hge->System_SetState(HGE_TITLE, "»¶ÀÖÓ¢Óï´ò×Ö¿ñ");
			g_gui->Enter();
		}
		delete g_set;
		g_set = NULL;
	}
	return false;
}