/*

* author		Wang Yang
* E-mail		wysaid@gmail.com OR admin@wysaid.org
* blog			http://blog.wysaid.org
* date			2012-03-16

*/

#ifndef _WYSAID_H_
#define _WYSAID_H_

#include <Windows.h>
#include <commctrl.h>
#include <process.h>
//#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <list>
#include "hge\hge.h"
#include "hge\hgefont.h"
#include "hge\hgegui.h"
#include "hge\hgecolor.h"
#include "hge\hgeparticle.h"
#include "hge\hgeanim.h"

#include "about.h"
#include "menuitem.h"
#include "settings.h"
#include "lose.h"
#include "netcontrol.h"
#include "bubbles.h"
#include "resource.h"

#pragma comment(lib,"hge.lib")
#pragma comment(lib,"hgehelp.lib")

#define PI				3.14159f

#define SCR_WIDTH		800
#define SCR_HEIGHT		600

#define WORDS_IMG_NUM	1

#define BG_SPEED		5.0f
#define MAX_BG_SPEED	20.0f
#define MIN_BG_SPEED	0
#define BULLET_SPEED	10.0f
#define MAX_BUL_SPEED	20.0f
#define MIN_BUL_SPEED	2.0f

#define MAX_WORD_XSPEED	1.0f
#define MAX_WORD_YSPEED	1.5f

#define DELAY_TIME		3.5f
#define MIN_DELAY_TIME	1.5f
#define MAX_DELAY_TIME	20.0f

#define BLEND_MODE_NUM	4

#define MAX_WORD_NUM	3660
#define MIN_WORD_NUM	1
#define MAX_ANIM_NUM	5
#define PLAYER_POSX		(SCR_WIDTH / 2)
#define PLAYER_POSY		(SCR_HEIGHT - 30)

#define MUSIC_NUM		10

#define WORD_INFO_SIZE	sizeof(WordInfo)

using std::string;
using std::vector;
using std::list;

extern HGE*			g_hge;
extern hgeFont		*g_fnt1, *g_fnt2, *g_fnt3;
extern float		g_dt;
extern HWND			g_hwnd;
extern float		g_bulletSpeed, g_bgSpeed, g_playerPosY;
extern int			g_missedWordsNum, g_killedWordsNum;
extern hgeGUI*		g_gui;
extern HEFFECT		g_effect[];

enum STATUS { stLEFT, stRIGHT, stDOWN, stUP, stHIT, stDIE };
enum MODE { PRACTICE, STAGE };
enum MUSIC { BGSOUND, CRY, HAPPY, SHOOT, SETTING, WRONG, TIMER, BOMB, GAMEOVER, CHALLENGE };

typedef struct WordInfo
{
	float x, y;
	float dx, dy;
}WORDINFO;

class Word
{
public:
	Word() : m_x(0), m_y(0), m_dx(0), m_dy(0), m_status(stDOWN) {}
	Word(string& str, hgeAnimation* leftanim, hgeAnimation* rightanim, hgeAnimation* downanim, hgeAnimation* upanim);
	Word(const char*, WORDINFO*, hgeAnimation*, hgeAnimation*, hgeAnimation*, hgeAnimation*);

	void init(const char*, hgeAnimation*, hgeAnimation*, hgeAnimation*, hgeAnimation*);

 	void fresh();
	bool frame();
	void render();
	bool hit(int ch);
	bool bomb();
	void setTitle();
	int makeWordMsg(char*, int);

	bool died() { return m_hitCount >= m_word.size(); }
	void getPos(float& x, float& y) { x = m_x; y = m_y; }
	string& getWord() { return m_word; }
	string& getText() { return m_text; }
	char getFirstChar() { return m_word[0]; }
	char getNextChar() { if(m_hitCount >= m_word.size()) return 0;	return m_word[m_hitCount]; }
	const char* getPrintStr() { return m_word.c_str() + m_hits; }
	STATUS getStatus() { return m_status; }

	~Word() {}
private:
	string m_word, m_text;
	int m_hitCount;
	int m_hits;
	float m_x, m_y;
	float m_dx, m_dy;
	STATUS m_status;
	hgeAnimation* m_anim[4];
};

class Words
{
public:
	Words();
	bool initWords(bool bReadFile); // Not to read vocabulary file when you're a client of online mode!
	Word* frame();
//	void frame(Word*);
	void render();
	void changeBlendMode(int n);
	void wordsRandomize();
	void restart();
	Word* addWords();
	void addWords(Word*);
	void wordsUpdate(bool);

	//Warning: Only use for 'wordmsgs' that is created by Word::makeWordMsg()
	void makeAndAddWord(const char*, int); 

	bool hit(int ch);
	bool hits(int ch, vector<Word*>& vec);
	void getChosenPos(float& x, float& y) { m_chosen->getPos(x, y); }
	Word* getChosenWord() { return m_chosen; }
	~Words();
private:
	vector<Word*> m_vec;
	list<Word*> m_listRandom;
	vector<Word*> m_wordInUse;
	vector<Word*> m_wordHit;
	HTEXTURE m_tex[MAX_ANIM_NUM];
	hgeAnimation* m_animDown[MAX_ANIM_NUM];
	hgeAnimation* m_animLeft[MAX_ANIM_NUM];
	hgeAnimation* m_animRight[MAX_ANIM_NUM];
	hgeAnimation* m_animUp[MAX_ANIM_NUM];
	float m_dt, m_waitTime;
	Word* m_chosen;
	int m_activeWords;
	bool m_empty;
};

class Bullet
{
public:
	Bullet(float vx, float vy, Word* word, HTEXTURE tex, hgeParticleSystem* par);

	void set(float vx, float vy, Word* word);
	bool frame();
	void render(hgeSprite* spr);

	~Bullet() { delete m_bombAnim;}
private:
	Word* m_target;
	hgeAnimation* m_bombAnim;
	hgeParticleSystem* m_particle;
	float m_x, m_y;
	float m_dx, m_dy;
	float m_rot;
	bool m_bombed;
};

class Bullets
{
public:
	Bullets();
	void add(Word* word);
	void add(vector<Word*>& word);
	void reinit() { m_vec.clear(); }
	void frame();
	void render();
	~Bullets();
private:
	vector<Bullet*> m_vec;
	int m_totalBullet;
	HTEXTURE m_parTex, m_bulletTex, m_bombTex;
	hgeSprite *m_parSpr[16], *m_bulletSpr;
	hgeParticleSystem* m_particle[16];
};

class Player
{
public:
	Player();
	void reinit();
	void frame();
	void render();
	void move();
	void setDes(float x, float y);
	~Player();
private:
	float m_rot;
	HTEXTURE m_playerTex;
	hgeSprite *m_playerSpr, *m_parSpr;
	hgeParticleSystem* m_par;
};

class TypeGame
{
public:
	TypeGame();
	void reinit();
	void practiceFrame();
	void practiceRender();
	Word* stageFrame();
	void stageRender();
	Word* onlineServerFrame();
	void onlineClientFrame();
	void onlineServerRender();
	void onlineClientRender();

	void makeAndAddWord(LPCSTR pMsg, int buflen) { m_words.makeAndAddWord(pMsg, buflen); }

	void playsnd(MUSIC music);
	bool showSettings() { return m_escape; }
	void resetEscape() { m_escape = false; }
	float getScore() { return (mode == STAGE) ? m_way : 0; }
	void changeWordsBlendMode(int n) { m_words.changeBlendMode(n); }
	static MODE mode;
	~TypeGame();
private:
	Player m_player;
	Words m_words;
	Bullets m_bullet;
	hgeQuad m_quad;
	float m_way, m_timeLimit;
	bool m_escape;
};

bool practiceFrameFunc();
bool practiceRenderFunc();
bool stageFrameFunc();
bool stageRenderFunc();
bool loseFrameFunc();
bool loseRenderFunc();
bool aboutFrameFunc();
bool aboutRenderFunc();
bool initGame();
bool initEffects();
void releaseEffects();
void initLoseAction();
void releaseGameRes();
//void playHappyAndBgsnd(void*);

extern TypeGame*	g_game;

#endif