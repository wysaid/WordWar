/*

* author		Wang Yang
* E-mail		wysaid@gmail.com OR admin@wysaid.org
* blog			http://blog.wysaid.org
* date			2012-03-16

*/

#include "wysaid.h"

HINSTANCE g_hInst;
HWND g_hwnd;
HGE* g_hge;
hgeGUI* g_gui;
hgeFont *g_fnt1, *g_fnt2, *g_fnt3;
hgeParticleSystem *g_particle;
hgeQuad g_quad;
hgeSprite *g_cursor, *g_parSpr, *g_bgSpr;
HTEXTURE g_cursorTex, g_bgTex, g_parTex;
HEFFECT g_menuSnd;

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
	g_hInst = hInst;
	g_hge = hgeCreate(HGE_VERSION);

	g_hge->System_SetState(HGE_LOGFILE, "Word War.log");
	g_hge->System_SetState(HGE_FRAMEFUNC, menuFrameFunc);
	g_hge->System_SetState(HGE_RENDERFUNC, menuRenderFunc);
	g_hge->System_SetState(HGE_TITLE, "Word War");
	g_hge->System_SetState(HGE_USESOUND, true);
	g_hge->System_SetState(HGE_WINDOWED, true);
	g_hge->System_SetState(HGE_SCREENWIDTH, SCR_WIDTH);
	g_hge->System_SetState(HGE_SCREENHEIGHT, SCR_HEIGHT);
	g_hge->System_SetState(HGE_SCREENBPP, 32);
	g_hge->System_SetState(HGE_SHOWSPLASH,false);
	g_hge->System_SetState(HGE_FPS, 60);
	g_hge->System_SetState(HGE_ICON, MAKEINTRESOURCEA(IDI_GAMEICON));
	if(g_hge->System_Initiate())
	{
		g_hwnd = g_hge->System_GetState(HGE_HWND);
		if(initGame()) g_hge->System_Start();
	}
	else
		MessageBoxA(NULL, g_hge->System_GetErrorMessage(),"Unknow Problem!!",MB_ICONERROR);
	releaseGameRes();
	g_hge->System_Shutdown();
	g_hge->Release();
	return 0;
}