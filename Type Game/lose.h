/*

* author		Wang Yang
* E-mail		wysaid@gmail.com OR admin@wysaid.org
* blog			http://blog.wysaid.org
* date			2012-03-16

*/

#ifndef _LOSE_H_
#define _LOSE_H_

#include "wysaid.h"

class LoseAction
{
public:
	LoseAction();

	void frame();
	void render();
	bool readyToExit() { return m_exit; }
	void saveHighScore();
	~LoseAction();
private:
	HTEXTURE m_tex;
	hgeSprite* m_sprite;
	std::string m_usrName;
	float m_highScore;
	bool m_newHighScore, m_exit;
};

#endif