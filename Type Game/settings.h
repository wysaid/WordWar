/*

* author		Wang Yang
* E-mail		wysaid@gmail.com OR admin@wysaid.org
* blog			http://blog.wysaid.org
* date			2012-03-16

*/

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "wysaid.h"

class Settings
{
public:
	Settings();
	void frame();
	void render();
	void set(bool);
	bool nowExit() { return m_exit; }
	bool backToMenu() { return m_backToMenu; }
	~Settings();
private:
	HTEXTURE m_tex, m_parTex;
	hgeSprite *m_sprite, *m_parSpr;
	hgeParticleSystem* m_par;
	int m_choice;
	bool m_exit, m_backToMenu;
};

bool settingFrame();
bool settingRender();

#endif