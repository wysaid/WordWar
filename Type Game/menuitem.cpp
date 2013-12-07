/*

* author		Wang Yang
* E-mail		wysaid@gmail.com OR admin@wysaid.org
* blog			http://blog.wysaid.org
* date			2012-03-16

*/

#include "menuitem.h"

extern HINSTANCE g_hInst;
extern HWND g_hwnd;
extern HGE* g_hge;
extern hgeGUI* g_gui;
extern hgeFont *g_fnt1, *g_fnt2, *g_fnt3;
extern hgeParticleSystem *g_particle;
extern hgeQuad g_quad;
extern hgeSprite *g_cursor, *g_parSpr, *g_bgSpr;
extern HTEXTURE g_cursorTex, g_bgTex, g_parTex;
extern HEFFECT g_menuSnd, g_effect[];
extern TypeGame *g_game;
extern Settings* g_set;
extern int g_wordsBlendMode, g_maxPageWords;
extern float g_delay, g_timeLeastLimit, g_deathSpeed;
extern About* g_about;

hgeGUIMenuItem::hgeGUIMenuItem(int _id, hgeFont *_fnt, HEFFECT _snd, float _x, float _y, float _delay, char *_title)
{
	float w;

	id=_id;
	fnt=_fnt;
	snd=_snd;
	delay=_delay;
	title=_title;

	color.SetHWColor(0xFFFFE060);
	shadow.SetHWColor(0x30000000);
	offset=0.0f;
	timer=-1.0f;
	timer2=-1.0f;

	bStatic=false;
	bVisible=true;
	bEnabled=true;

	w=fnt->GetStringWidth(title);
	rect.Set(_x-w/2, _y, _x+w/2, _y+fnt->GetHeight());
}

void hgeGUIMenuItem::Render()
{
	fnt->SetColor(shadow.GetHWColor());
	fnt->Render(rect.x1+offset+3, rect.y1+3, HGETEXT_LEFT, title);
	fnt->SetColor(color.GetHWColor());
	fnt->Render(rect.x1-offset, rect.y1-offset, HGETEXT_LEFT, title);
}

void hgeGUIMenuItem::Update(float dt)
{
	if(timer2 != -1.0f)
	{
		timer2+=dt;
		if(timer2 >= delay+0.1f)
		{
			color=scolor2+dcolor2;
			shadow=sshadow+dshadow;
			offset=0.0f;
			timer2=-1.0f;
		}
		else
		{
			if(timer2 < delay) { color=scolor2; shadow=sshadow; }
			else { color=scolor2+dcolor2*(timer2-delay)*10; shadow=sshadow+dshadow*(timer2-delay)*10; }
		}
	}
	else if(timer != -1.0f)
	{
		timer+=dt;
		if(timer >= 0.2f)
		{
			color=scolor+dcolor;
			offset=soffset+doffset;
			timer=-1.0f;
		}
		else
		{
			color=scolor+dcolor*timer*5;
			offset=soffset+doffset*timer*5;
		}
	}
}

void hgeGUIMenuItem::Enter()
{
	hgeColor tcolor2;

	scolor2.SetHWColor(0x00FFE060);
	tcolor2.SetHWColor(0xFFFFE060);
	dcolor2=tcolor2-scolor2;

	sshadow.SetHWColor(0x00000000);
	tcolor2.SetHWColor(0x30000000);
	dshadow=tcolor2-sshadow;

	timer2=0.0f;
}

void hgeGUIMenuItem::Leave()
{
	hgeColor tcolor2;

	scolor2.SetHWColor(0xFFFFE060);
	tcolor2.SetHWColor(0x00FFE060);
	dcolor2=tcolor2-scolor2;

	sshadow.SetHWColor(0x30000000);
	tcolor2.SetHWColor(0x00000000);
	dshadow=tcolor2-sshadow;

	timer2=0.0f;
}

bool hgeGUIMenuItem::IsDone()
{
	if(timer2==-1.0f) return true;
	else return false;
}

void hgeGUIMenuItem::Focus(bool bFocused)
{
	hgeColor tcolor;

	if(bFocused)
	{
		hge->Effect_Play(snd);
		scolor.SetHWColor(0xFFFFE060);
		tcolor.SetHWColor(0xFFFFFFFF);
		soffset=0;
		doffset=4;
	}
	else
	{
		scolor.SetHWColor(0xFFFFFFFF);
		tcolor.SetHWColor(0xFFFFE060);
		soffset=4;
		doffset=-4;
	}

	dcolor=tcolor-scolor;
	timer=0.0f;
}

void hgeGUIMenuItem::MouseOver(bool bOver)
{
	if(bOver) gui->SetFocus(id);
}

bool hgeGUIMenuItem::MouseLButton(bool bDown)
{
	if(!bDown)
	{
		offset=4;
		return true;
	}
	else 
	{
		hge->Effect_Play(snd);
		offset=0;
		return false;
	}
}

bool hgeGUIMenuItem::KeyClick(int key, int chr)
{
	if(key==HGEK_ENTER || key==HGEK_SPACE)
	{
		MouseLButton(true);
		return MouseLButton(false);
	}

	return false;
}



bool menuRenderFunc()
{
	g_hge->Gfx_BeginScene();	
	g_hge->Gfx_RenderQuad(&g_quad);
	g_gui->Render();
	g_particle->Render();
//	g_fnt2->printf(5, 5, HGETEXT_LEFT, "What a loli game to play?!\nIsn't it??");
	g_hge->Gfx_EndScene();
	return false;
}

bool initmenu()
{
	g_quad.tex = g_hge->Texture_Load("res/bg.png");
	g_cursorTex = g_hge->Texture_Load("res/cursor.png");
	g_menuSnd = g_hge->Effect_Load("res/menu.wav");
	g_parTex = g_hge->Texture_Load("res/particles.png");

	if(!(g_quad.tex && g_cursorTex && g_menuSnd && g_parTex))
	{
		MessageBoxA(g_hwnd,"加载资源失败!","请检查当前文件夹下是否存在所需资源",0);
		return false;
	}
	g_quad.blend = BLEND_ALPHABLEND | BLEND_COLORMUL | BLEND_NOZWRITE;

	for(int i=0; i<4; ++i)
	{
		g_quad.v[i].z = 0.5f;
		g_quad.v[i].col = 0xFFFFFFFF;
	}
	g_quad.v[0].x=0; g_quad.v[0].y=0; 
	g_quad.v[1].x=SCR_WIDTH; g_quad.v[1].y=0; 
	g_quad.v[2].x=SCR_WIDTH; g_quad.v[2].y=SCR_HEIGHT; 
	g_quad.v[3].x=0; g_quad.v[3].y=SCR_HEIGHT; 

	g_fnt1=new hgeFont("res/font1.fnt");
	g_fnt1->SetColor(0xFFFFE060);
	g_fnt2=new hgeFont("res/font3.fnt");
	g_fnt2->SetColor(0xFFFFFFFF);
	g_fnt3 = new hgeFont("res/font2.fnt");
	g_fnt3->SetColor(0xFFFF0000);
//	g_fnt2->SetScale(2.0f);
	g_cursor = new hgeSprite(g_cursorTex,0,0,32,32);

	g_parSpr = new hgeSprite(g_parTex,0,0,32,32);
	g_parSpr->SetBlendMode(BLEND_COLORMUL | BLEND_ALPHAADD | BLEND_NOZWRITE);
	g_parSpr->SetHotSpot(16,16);
	g_particle = new hgeParticleSystem("res/particle.psi", g_parSpr);
	g_particle->Fire();

	g_gui = new hgeGUI();

	g_gui->AddCtrl(new hgeGUIMenuItem(1,g_fnt1,g_menuSnd,400,200,0.0f,"Practise"));
	g_gui->AddCtrl(new hgeGUIMenuItem(2,g_fnt1,g_menuSnd,400,240,0.1f,"Challange"));
	g_gui->AddCtrl(new hgeGUIMenuItem(3,g_fnt1,g_menuSnd,400, 280, 0.2f, "OnLine Chanllenge"));
	g_gui->AddCtrl(new hgeGUIMenuItem(4,g_fnt1,g_menuSnd,400,320,0.3f,"Settings"));
	g_gui->AddCtrl(new hgeGUIMenuItem(5,g_fnt1,g_menuSnd,400,360,0.4f,"About me"));
	g_gui->AddCtrl(new hgeGUIMenuItem(6,g_fnt1,g_menuSnd,400,400,0.5f,"Exit"));

	g_gui->SetNavMode(HGEGUI_UPDOWN | HGEGUI_CYCLED);
	g_gui->SetCursor(g_cursor);
	g_gui->Enter();
	return true;
}

void releaseEffects()
{
	for(int i = 0; i != MUSIC_NUM; ++i)
	{
		g_hge->Effect_Free(g_effect[i]);
	}
}

void releaseMenuRes()
{
	delete g_gui; g_gui = NULL;
	delete g_cursor; g_cursor = NULL;
	delete g_fnt1; g_fnt1 = NULL;
	delete g_fnt2; g_fnt2 = NULL;
	delete g_fnt3, g_fnt3 = NULL;
	delete g_particle; g_particle = NULL;
	delete g_parSpr; g_parSpr = NULL;
	delete g_bgSpr; g_bgSpr = NULL;
	if(g_parTex)
	{
		g_hge->Texture_Free(g_parTex);
		g_parTex = 0;
	}
	if(g_cursorTex)
	{
		g_hge->Texture_Free(g_cursorTex);
		g_cursorTex = 0;;
	}
	if(g_bgTex)
	{
		g_hge->Texture_Free(g_bgTex);
		g_bgTex = 0;
	}
	if(g_menuSnd)
	{
		g_hge->Effect_Free(g_menuSnd);
		g_menuSnd = 0;
	}
	if(g_quad.tex)
	{
		g_hge->Texture_Free(g_quad.tex);
		g_quad.tex = 0;
	}
}

bool menuFrameFunc()
{
	float dt=g_hge->Timer_GetDelta();
	static float t=0.0f;
	float tx,ty;
	float x,y;
	int id;
	static int lastid=0;

	g_hge->Input_GetMousePos(&x,&y);
	g_particle->MoveTo(x,y);
	g_particle->Update(dt);

	id=g_gui->Update(dt);
	if(id == -1)
	{
		switch(lastid)
		{
		case 1:
			if(!initPracticeMode())
			{
				g_gui->Enter();
				break;
			}
			g_hge->System_SetState(HGE_FRAMEFUNC,practiceFrameFunc);
			g_hge->System_SetState(HGE_RENDERFUNC,practiceRenderFunc);
			lastid = 0;
			return false;
		case 2:
			if(!initStageMode())
			{
				g_gui->Enter();
				break;
			}
			g_hge->System_SetState(HGE_FRAMEFUNC,stageFrameFunc);
			g_hge->System_SetState(HGE_RENDERFUNC,stageRenderFunc);
			lastid = 0;
			return false;
		case 3:
			if(!(initOnlineMode() && DialogBox(g_hInst, MAKEINTRESOURCE(IDD_OLCONNECT), g_hwnd, Connect)))
			{
				g_gui->Enter();
				break;;
			}
			// The Jump function is contained in the DialogBox.
			lastid = 0;
			return false;
		case 4:
			if(!settings())
			{
				g_gui->Enter();
				break;
			}

			g_hge->System_SetState(HGE_FRAMEFUNC,settingFrame);
			g_hge->System_SetState(HGE_RENDERFUNC,settingRender);
			lastid = 0;
			return false;
		case 5:
			if(!initAbout())
			{
				g_gui->Enter();
				break;
			}
			g_hge->System_SetState(HGE_FRAMEFUNC,aboutFrameFunc);
			g_hge->System_SetState(HGE_RENDERFUNC,aboutRenderFunc);
			lastid = 0;
			return false;

		case 6: 
			return true;
		}
	}
	else if(id)
	{ 
		lastid=id; 
		g_gui->Leave(); 
	}

	t+=dt;
	tx=50*cosf(t/60);
	ty=50*sinf(t/60);

	g_quad.v[0].tx=tx;				g_quad.v[0].ty=ty;
	g_quad.v[1].tx=tx+SCR_WIDTH/64;	g_quad.v[1].ty=ty;
	g_quad.v[2].tx=tx+SCR_WIDTH/64;	g_quad.v[2].ty=ty+SCR_HEIGHT/64;
	g_quad.v[3].tx=tx;				g_quad.v[3].ty=ty+SCR_HEIGHT/64;
	return false;
}


bool initPracticeMode()
{
	
	if(g_game == NULL) g_game = new TypeGame();
	else if(TypeGame::mode == PRACTICE) g_game->reinit();
	else
	{
		delete g_game;
		TypeGame::mode = PRACTICE;
		g_game = new TypeGame();
	}

	g_game->changeWordsBlendMode(g_hge->Random_Int(0,4));
	if(g_maxPageWords > 1000) g_maxPageWords = 20;
	g_fnt1->SetScale(0.7f);
	g_fnt1->SetColor(0xffffff00);
	g_fnt2->SetColor(0xffffff00);
	g_missedWordsNum = 0;
	g_killedWordsNum = 0;
	g_timeLeastLimit = 0.5f;
	g_delay = DELAY_TIME;
	g_hge->System_SetState(HGE_TITLE, "练习模式！ 请一个个干掉屏幕上的单词！");
	g_hge->Effect_PlayEx(g_effect[BGSOUND], 100, 0, 1.0f, true);
	return true;
}

bool initStageMode()
{
	TypeGame::mode = STAGE;
	delete g_game;
	g_game = new TypeGame();
	g_game->changeWordsBlendMode(g_wordsBlendMode);
	g_fnt1->SetScale(0.7f);
	g_fnt2->SetColor(0xffff7f00 + g_hge->Random_Int(0,0x80ff));
	g_bulletSpeed = g_bulletSpeed;
	g_delay = 1.5f;
	g_timeLeastLimit = 0.5f;
	g_maxPageWords = MAX_WORD_NUM;
	g_missedWordsNum = 0;
	g_killedWordsNum = 0;
	g_deathSpeed = 1.1f;
	g_hge->System_SetState(HGE_TITLE, "游戏模式！ 请一片一片干掉屏幕上的单词！");
	g_hge->Effect_PlayEx(g_effect[CHALLENGE], 100, 0, 1.0f, true);
	return true;
}

bool  initOnlineMode()
{
	TypeGame::mode = STAGE;
	delete g_game;
	g_game = new TypeGame();
	g_game->changeWordsBlendMode(g_wordsBlendMode);
	g_fnt1->SetScale(0.7f);
	g_fnt2->SetColor(0xffff7f00 + g_hge->Random_Int(0,0x80ff));
	g_bulletSpeed = g_bulletSpeed;
	g_delay = 1.5f;
	g_timeLeastLimit = 0.5f;
	g_maxPageWords = MAX_WORD_NUM;
	g_missedWordsNum = 0;
	g_killedWordsNum = 0;
	g_deathSpeed = 1.1f;
	g_hge->System_SetState(HGE_TITLE, "请按界面连接其他用户");
//	g_hge->Effect_PlayEx(g_effect[CHALLENGE], 100, 0, 1.0f, true);
	return true;
}

bool initAbout()
{
	if(g_about == NULL) g_about = new About();
	g_hge->System_SetState(HGE_TITLE, "关于我，关于本游戏~ 谢谢喜欢！");
	return true;
}

bool settings()
{
	if(g_set == NULL) g_set = new Settings();
	g_fnt1->SetScale(1.0f);
	g_fnt1->SetColor(0xFFEFE060);
	g_hge->System_SetState(HGE_TITLE, "请按F1-F5进行设置！！");
	return true;
}

bool initEffects()
{
	char str[256];
	bool fail = true;
	for(int i=0; i != MUSIC_NUM; ++i)
	{
		sprintf(str, "res/effect%d.mp3", i);
		fail = fail && (g_effect[i] = g_hge->Effect_Load(str));
	}
	g_hge->Effect_Play(g_effect[HAPPY]);
	return fail;
}