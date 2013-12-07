/*

 * author		Wang Yang
 * E-mail		wysaid@gmail.com OR admin@wysaid.org
 * blog			http://blog.wysaid.org
 * date			2012-03-16
 * 
 * modify		Some code of "menuitem.h、menuitem.cpp" is from the example of the HGE.
*/

#pragma once

#include "wysaid.h"

#ifndef _MENUITEM_H_
#define _MENUITEM_H_

class hgeGUIMenuItem : public hgeGUIObject
{
public:
	hgeGUIMenuItem(int id, hgeFont *fnt, HEFFECT snd, float x, float y, float delay, char *title);

	virtual void	Render();
	virtual void	Update(float dt);

	virtual void	Enter();
	virtual void	Leave();
	virtual bool	IsDone();
	virtual void	Focus(bool bFocused);
	virtual void	MouseOver(bool bOver);

	virtual bool	MouseLButton(bool bDown);
	virtual bool	KeyClick(int key, int chr);

private:
	hgeFont		*fnt;
	HEFFECT		snd;
	float		delay;
	char		*title;

	hgeColor	scolor, dcolor, scolor2, dcolor2, sshadow, dshadow;
	hgeColor	color, shadow;
	float		soffset, doffset, offset;
	float		timer, timer2;
};

bool menuRenderFunc();
bool initmenu();
void releaseMenuRes();
bool menuFrameFunc();
bool initPracticeMode();
bool initStageMode();
bool initOnlineMode();
bool initAbout();
bool settings();

#endif