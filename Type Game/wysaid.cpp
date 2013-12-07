/*

* author		Wang Yang
* E-mail		wysaid@gmail.com OR admin@wysaid.org
* blog			http://blog.wysaid.org
* date			2012-03-16

*/

#include "wysaid.h"

float g_dt;
TypeGame* g_game = NULL;
int g_stageWords = 10, g_poppedWordsNum = 0, g_missedWordsNum = 0, g_killedWordsNum = 0;
int g_rightInputCount = 0, g_wrongInputCount = 0;
float g_playerPosX = PLAYER_POSX, g_playerPosY = PLAYER_POSY;
float g_delay = DELAY_TIME, g_accuracyRate = 0.0f;
float g_timeLeastLimit = 0.5f;
float g_deathSpeed = 1.1f;
HEFFECT g_effect[MUSIC_NUM] = {0};
extern int g_maxPageWords;
extern hgeGUI* g_gui;
extern LoseAction* g_lose;
extern HTEXTURE g_parTex;

Word::Word(string& str, hgeAnimation* leftanim, hgeAnimation* rightanim, hgeAnimation* downanim, hgeAnimation* upanim)
	: m_hitCount(0), m_hits(0)
{
	m_y = -10.0f;
	m_x = g_hge->Random_Int(50, 700);
	m_dx = g_hge->Random_Float(0.1, 1.0) * pow(-1.0, g_hge->Random_Int(0,3));
	m_dy = float(g_hge->Random_Float(0.1, 0.5)) / m_word.size() + 0.05f;
	init(str.c_str(), leftanim, rightanim, downanim, upanim);
}

Word::Word(const char* pStr, WORDINFO* pInfo, hgeAnimation* leftanim, hgeAnimation* rightanim, hgeAnimation* downanim, hgeAnimation* upanim)
	: m_hitCount(0), m_hits(0)
{
	m_x = pInfo->x;
	m_y = pInfo->y;
	m_dx = pInfo->dx;
	m_dy = pInfo->dy;
	init(pStr, leftanim, rightanim, downanim, upanim);
}

void Word::init(const char* pStr, hgeAnimation* leftanim, hgeAnimation* rightanim, hgeAnimation* downanim, hgeAnimation* upanim)
{
	m_text = pStr;
	m_anim[0] = leftanim;
	m_anim[1] = rightanim;
	m_anim[2] = downanim;
	m_anim[3] = upanim;
	int n = m_text.find(' ');
	if(n != m_text.npos) m_word.append(m_text.begin(), m_text.begin() + n);
	if(m_word.size() == 0) m_word = "wysaid";

	if(m_dx > 0) m_status = stRIGHT;
	else m_status = stLEFT;
}

void Word::fresh()
{
	m_hitCount = 0;
	m_hits = 0;
	m_y = -10.0f;
	m_dx = g_hge->Random_Float(0.1, 1.0) * pow(-1.0, g_hge->Random_Int(0,3));
	m_dy = float(g_hge->Random_Float(0.1, 0.5)) / m_word.size() + 0.05f;
	if(m_dx > 0) m_status = stRIGHT;
	else m_status = stLEFT;
}

bool Word::frame()
{
	m_x += m_dx;
	m_y += m_dy;
	if(m_x < 10 || m_x > SCR_WIDTH - 80)
	{
		m_dx = -m_dx;
		(m_dx > 0) ? m_status = stRIGHT : m_status = stLEFT;
	}
	if(m_hits >= m_word.size())
	{
		++g_killedWordsNum;
		return true;
	}
	if(m_y > g_playerPosY)
	{
		++g_missedWordsNum;
		g_hge->Effect_Play(g_effect[CRY]);
		return true;
	}
	return false;
}

void Word::render()
{
	if(m_status > 3) return;
	m_anim[m_status]->Render(m_x, m_y);
	g_fnt2->printf(m_x, m_y, HGETEXT_MIDDLE, getPrintStr());
}

bool Word::hit(int ch)
{
	if(m_hitCount < m_word.size() && m_word[m_hitCount] == ch)
	{
		++m_hitCount;
		return true;
	}
	return false;
}

bool Word::bomb()
{
	if(m_hits < m_hitCount)
	{
		++m_hits;
		if(m_y > 10.0f)	m_y -= 10.0f;
		if(g_game->mode == PRACTICE)
		{
			m_dy *= 0.95f;
			m_dx *= 0.9f;
		}
		else
		{
			if(m_dy < MAX_WORD_YSPEED) m_dy *= g_deathSpeed;
			if(m_dx < MAX_WORD_XSPEED) m_dx *= g_deathSpeed;
		}
		g_hge->Effect_Play(g_effect[BOMB]);
		return true;
	}
	return false;
}

void Word::setTitle()
{
	SetWindowTextA(g_hwnd, m_text.c_str());
}

int Word::makeWordMsg(char* msg, int bufsize)
{
	WordInfo info;
	if(bufsize <= WORD_INFO_SIZE) return 0;
	info.x = m_x;
	info.y = m_y;
	info.dx = m_dx;
	info.dy = m_dy;
	memcpy(msg, &info, WORD_INFO_SIZE);
	bufsize -= WORD_INFO_SIZE;
	bufsize = m_text.size() < bufsize? m_text.size() : bufsize;
	memcpy(msg + WORD_INFO_SIZE, m_text.c_str(), bufsize);
	return bufsize + WORD_INFO_SIZE;
}

Words::Words() : m_dt(0), m_waitTime(1), m_empty(false), m_chosen(NULL), m_activeWords(0)
{
	char c[64];
	for(int i=0; i != MAX_ANIM_NUM; ++i)
	{
		sprintf(c, "res/word%d.png", i);
		m_tex[i] = g_hge->Texture_Load(c);
		m_animDown[i] = new hgeAnimation(m_tex[i], 4, 8, 0, 0, 80, 80);
		m_animLeft[i] = new hgeAnimation(m_tex[i], 4, 8, 0, 80,80, 80);
		m_animRight[i] = new hgeAnimation(m_tex[i], 4, 8, 0, 160, 80, 80);
		m_animUp[i] = new hgeAnimation(m_tex[i], 4, 8, 0, 240, 80, 80);
		m_animDown[i]->SetHotSpot(10,30);
		m_animLeft[i]->SetHotSpot(10,30);
		m_animRight[i]->SetHotSpot(10,30);
		m_animUp[i]->SetHotSpot(10,30);
		m_animDown[i]->Play();
		m_animLeft[i]->Play();
		m_animRight[i]->Play();
		m_animUp[i]->Play();
	}
}

bool Words::initWords(bool bReadFile = true)
{
	if(bReadFile)
	{
		using std::fstream;
		g_hge->Random_Seed(time(NULL));
		m_vec.clear();
		fstream in("res/vocabulary.txt", fstream::in);
		if(!in) 
		{
			m_empty = true;
			return false;
		}
		for(string str; !in.eof() && m_vec.size() < MAX_WORD_NUM; )
		{
			int n = g_hge->Random_Int(0, MAX_ANIM_NUM - 1);
			getline(in, str);
			m_vec.push_back(new Word(str, m_animLeft[n], m_animRight[n], m_animDown[n], m_animUp[n]));
		}
		in.close();
		if(m_empty = m_vec.empty())  // Notify: use '=' OK, not '=='
			return false;
		wordsRandomize();
	}
	else
	{
		m_vec.clear();
		m_listRandom.clear();
		m_wordInUse.clear();
		m_wordHit.clear();
		m_chosen = NULL;
		m_activeWords = 0;
		m_empty = true;
	}
	return true;
}

void Words::wordsRandomize()
{
//	_beginthread(playHappyAndBgsnd, 0, NULL);
	if(m_vec.empty()) return ;
	g_hge->Channel_StopAll();
	m_listRandom.clear();
	m_wordInUse.clear();
	vector<Word*> vec[26];
	int ch = m_vec[0]->getFirstChar();
	int cnt = 0;
	vector<Word*>::iterator rec = m_vec.begin();
	for(vector<Word*>::iterator iter = rec; iter != m_vec.end(); ++iter)
	{
		(*iter)->fresh();
		if(ch == (*iter)->getFirstChar())
			continue;
		vec[cnt].insert(vec[cnt].begin(), rec, iter);
		++cnt %= 26;
		rec = iter;
		ch = (*iter)->getFirstChar();
	}
	vec[cnt].insert(vec[cnt].end(), rec, m_vec.end());

	vector<vector<Word*>> tmp;
	for(int i=0; i != 26; ++i)
	{
		if(vec[i].size() != 0)
			tmp.push_back(vec[i]);
	}

	for(int n=tmp.size()-1, i= g_hge->Random_Int(0,n); !tmp.empty(); i = g_hge->Random_Int(0,n))
	{
		int m = g_hge->Random_Int(0, tmp[i].size()-1);
		m_listRandom.push_back(tmp[i][m]);
		tmp[i].erase(tmp[i].begin() + m);
		if(tmp[i].empty())
		{
			tmp.erase(tmp.begin() + i);
			--n;
			if(tmp.empty()) break;
		}
	}
}

Word* Words::addWords()
{
	Word* word = NULL;
	m_dt += g_dt;
	if(m_dt > m_waitTime && m_activeWords < g_maxPageWords)
	{
		m_waitTime = g_hge->Random_Float(g_timeLeastLimit, g_delay);
		m_dt = 0;
		if(!m_listRandom.empty()) 
		{
			m_wordInUse.push_back(word = m_listRandom.front());
			m_listRandom.pop_front();
			++g_poppedWordsNum;
			g_hge->Effect_Play(g_effect[TIMER]);
		}
		else 
		{
			restart();
		}
		m_activeWords = m_wordInUse.size();
	}
	return word;
}

void Words::addWords(Word* word)
{
	if(word == NULL) return ;
	m_wordInUse.push_back(word);
	m_activeWords = m_wordInUse.size();
}

void Words::wordsUpdate(bool bDel = false)
{
	for(vector<Word*>::iterator iter = m_wordInUse.begin(); iter < m_wordInUse.end(); )
	{
		if((*iter)->frame())
		{
			if(bDel) delete *iter;
			iter = m_wordInUse.erase(iter);
		}
		else ++iter;
	}
	for(vector<Word*>::iterator iter = m_wordHit.begin(); iter != m_wordHit.end();)
	{

		if((*iter)->frame())
		{
			if(bDel) delete *iter;
			iter = m_wordHit.erase(iter);
			--m_activeWords;
		}
		else ++iter;		
	}
	for(int i=0; i != MAX_ANIM_NUM; ++i)
	{
		m_animDown[i]->Update(g_dt);
		m_animLeft[i]->Update(g_dt);
		m_animRight[i]->Update(g_dt);
		m_animUp[i]->Update(g_dt);
	}
}

Word* Words::frame()
{
	Word* word = addWords();
	wordsUpdate();
	return word;
}
// 
// void Words::frame(Word* word)
// {
// //	addWords(word);
// 	wordsUpdate(true);
// }

void Words::restart()
{
	vector<Word*> vecUse(m_wordInUse);
	vector<Word*> vecHit(m_wordHit);
	Word* word = m_chosen;
	wordsRandomize();
	m_wordInUse = vecUse;
	m_wordHit = vecHit;
	m_chosen = word;
}

void Words::render()
{
	for(vector<Word*>::iterator iter = m_wordInUse.begin(); iter != m_wordInUse.end(); ++iter)
	{
		(*iter)->render();
	}
	for(vector<Word*>::iterator iter = m_wordHit.begin(); iter != m_wordHit.end(); ++iter)
	{
		(*iter)->render();
	}
	g_fnt1->printf(10, 570, HGETEXT_LEFT, "There're %d active words", m_activeWords);
	if(m_chosen) g_fnt1->printf(10, 550, HGETEXT_LEFT, "The chosen word: %s", m_chosen->getPrintStr());
}

void Words::changeBlendMode(int n)
{
	static int sprBlend[5]=
	{
		BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_NOZWRITE,
		BLEND_COLORADD | BLEND_ALPHABLEND | BLEND_NOZWRITE,
		BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_NOZWRITE,
		BLEND_COLORMUL | BLEND_ALPHAADD   | BLEND_NOZWRITE,
		BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_NOZWRITE
	};

	static DWORD fntColor[5]=
	{
		0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF
	};

	static DWORD sprColors[5][5]=
	{
		{ 0xFFFFFFFF, 0xFFFFE080, 0xFF80A0FF, 0xFFA0FF80, 0xFFFF80A0 },
		{ 0xFF000000, 0xFF303000, 0xFF000060, 0xFF006000, 0xFF600000 },
		{ 0x80FFFFFF, 0x80FFE080, 0x8080A0FF, 0x80A0FF80, 0x80FF80A0 },
		{ 0x80FFFFFF, 0x80FFE080, 0x8080A0FF, 0x80A0FF80, 0x80FF80A0 },
		{ 0x40202020, 0x40302010, 0x40102030, 0x40203010, 0x40102030 }
	};
	for(int i=0; i != MAX_ANIM_NUM; ++i)
	{
		m_animDown[i]->SetBlendMode(sprBlend[n]);
		m_animLeft[i]->SetBlendMode(sprBlend[n]);
		m_animRight[i]->SetBlendMode(sprBlend[n]);
		m_animUp[i]->SetBlendMode(sprBlend[n]);
		m_animDown[i]->SetColor(sprColors[n][g_hge->Random_Int(0,4)]);
		m_animLeft[i]->SetColor(sprColors[n][g_hge->Random_Int(0,4)]);
		m_animRight[i]->SetColor(sprColors[n][g_hge->Random_Int(0,4)]);
		m_animUp[i]->SetColor(sprColors[n][g_hge->Random_Int(0,4)]);
	}
	g_fnt2->SetColor(fntColor[n]);
}

void Words::makeAndAddWord(const char* pMsg, int buflen) // pMsg must be end with '\0'
{
	WORDINFO info;
	int n;
	if(buflen <= WORD_INFO_SIZE) return ;
//	pMsg[buflen] = 0;
	memcpy(&info, pMsg, WORD_INFO_SIZE);
	n = g_hge->Random_Int(0, MAX_ANIM_NUM);
	m_wordInUse.push_back(new Word(pMsg + WORD_INFO_SIZE, &info, m_animLeft[n], m_animRight[n], m_animDown[n], m_animUp[n]));
}

bool Words::hit(int ch/*, float x, float y*/)
{
	if(m_chosen != NULL && m_chosen->died())
		m_chosen = NULL;
	else if(m_chosen != NULL)
	{
		if(m_chosen->hit(ch))
		{
			return true;
		}
		return false;
	}

	int n = -1;
	float distance = -1000.0;

	for(vector<Word*>::size_type i = 0; i < m_wordInUse.size(); ++i )
	{
		if(ch == m_wordInUse[i]->getFirstChar())
		{
			float fx, fy;
			m_wordInUse[i]->getPos(fx, fy);

			if(fy > distance)
			{
				n = i;
				distance = fy;
			}
		}
	}

	if(n == -1) return false;

	m_chosen = m_wordInUse[n];
	m_wordInUse.erase(m_wordInUse.begin() + n);
	m_wordHit.push_back(m_chosen);

	m_chosen->hit(ch);
	m_chosen->setTitle();
	return true;
}

bool Words::hits(int ch, vector<Word*>& vec)
{
	for(vector<Word*>::iterator iter = m_wordInUse.begin(); iter != m_wordInUse.end();)
	{
		char c = (*iter)->getNextChar();
		if(ch == c)
		{
			(*iter)->hit(ch);
			vec.push_back((*iter));
		}
		else if(c == 0)
		{
			iter = m_wordInUse.erase(iter);
			continue;
		}
		 ++iter;
	}
	return !vec.empty();
}

Words::~Words()
{
	for(vector<Word*>::iterator iter = m_vec.begin(); iter != m_vec.end(); ++iter)
		delete *iter;

	for(int i=0; i != MAX_ANIM_NUM; ++i)
	{
		delete m_animDown[i];
		delete m_animLeft[i];
		delete m_animRight[i];
		delete m_animUp[i];
		g_hge->Texture_Free(m_tex[i]);
	}
}

Player::Player()
{
	if(TypeGame::mode == PRACTICE)
	{
		m_playerTex = g_hge->Texture_Load("res/player.png");
		m_playerSpr = new hgeSprite(m_playerTex,0,0,70,20);
		m_playerSpr->SetHotSpot(30,10);
		m_parSpr = new hgeSprite(g_parTex, 32, 0, 32, 32);
		m_parSpr->SetBlendMode(BLEND_COLORMUL | BLEND_ALPHAADD | BLEND_NOZWRITE);
		m_parSpr->SetHotSpot(16,16);
		m_parSpr->SetColor(0x7fffff00);
		m_par = new hgeParticleSystem("res/particle1.psi", m_parSpr);
		m_rot = -M_PI_2;
	}
	else
	{
		m_playerTex = g_hge->Texture_Load("res/playerrot.png");
		m_playerSpr = new hgeSprite(m_playerTex,0,0,20,70);
		m_playerSpr->SetHotSpot(10,30);
		m_parSpr = NULL;
		m_playerSpr->SetColor(0x4fffff00);
		m_playerSpr->SetBlendMode(BLEND_COLORMUL | BLEND_ALPHAADD | BLEND_NOZWRITE);
		m_par = new hgeParticleSystem("res/particle1.psi", m_playerSpr);
		m_rot = 0;
	}
	m_par->Fire();
}

void Player::frame()
{
	m_par->MoveTo(g_playerPosX, g_playerPosY);
	m_par->Update(g_dt);
	if(g_hge->Input_GetKeyState(HGEK_LEFT)) 
	{
		g_playerPosX -= 200 * g_dt;
		if(g_playerPosX < 0)
			g_playerPosX = 0;
	}
	else if(g_hge->Input_GetKeyState(HGEK_RIGHT))
	{
		g_playerPosX += 200 * g_dt;
		if(g_playerPosX > SCR_WIDTH)
			g_playerPosX = SCR_WIDTH;
	}

	if(g_hge->Input_GetKeyState(HGEK_UP))
	{
		g_playerPosY -= 200 * g_dt;	
		if(g_playerPosY < 0)
			g_playerPosY = 0;
	}
	else if(g_hge->Input_GetKeyState(HGEK_DOWN))
	{
		g_playerPosY += 200 * g_dt;
		if(g_playerPosY > SCR_HEIGHT)
			g_playerPosY = SCR_HEIGHT;
	}
}

void Player::render()
{
	m_playerSpr->RenderEx(g_playerPosX, g_playerPosY, m_rot);
	m_par->Render();
}

void Player::setDes(float x, float y)
{
	float fx = x - g_playerPosX, fy = y - g_playerPosY;
	float distance = sqrtf(fx*fx + fy*fy);
	m_rot = asinf(fy / distance);
	if(fx < 0) m_rot = PI - m_rot;
}

void Player::reinit()
{
	g_playerPosX = PLAYER_POSX;
	g_playerPosY = PLAYER_POSY;
	m_rot = -M_PI_2;
	if(TypeGame::mode == PRACTICE)
	{
		delete m_playerSpr;
		delete m_parSpr;
		delete m_par;
		g_hge->Texture_Free(m_playerTex);
		m_playerTex = g_hge->Texture_Load("res/player.png");
		m_playerSpr = new hgeSprite(m_playerTex,0,0,70,20);
		m_playerSpr->SetHotSpot(30,10);
		m_parSpr = new hgeSprite(g_parTex, 32, 0, 32, 32);
		m_parSpr->SetBlendMode(BLEND_COLORMUL | BLEND_ALPHAADD | BLEND_NOZWRITE);
		m_parSpr->SetHotSpot(16,16);
		m_parSpr->SetColor(0x7fffff00);
		m_par = new hgeParticleSystem("res/particle1.psi", m_parSpr);
		m_par->Fire();
	}
}
Player::~Player()
{
	delete m_playerSpr;
	delete m_parSpr;
	delete m_par;
	g_hge->Texture_Free(m_playerTex);
}

Bullet::Bullet(float vx, float vy, Word* word, HTEXTURE tex, hgeParticleSystem* par)
{
	set(vx, vy, word);
	m_bombAnim = new hgeAnimation(tex, 14, 14, 0, 0, 50, 50);
	m_bombAnim->SetMode(HGEANIM_FWD | HGEANIM_NOLOOP);
	m_bombAnim->SetHotSpot(25,25);
	m_particle = par;
}

void Bullet::set(float vx, float vy, Word* word)
{
	float destinationX, destinationY;
	word->getPos(destinationX, destinationY);
	m_target = word;
	m_x = vx;
	m_y = vy;

	float fy = destinationY - m_y;
	float fx = destinationX - m_x;
	m_rot = asinf(fy/sqrtf(fx*fx + fy*fy));
	if(fx<0) m_rot = PI - m_rot;
	m_dx = g_bulletSpeed * cosf(m_rot);
	m_dy = g_bulletSpeed * sinf(m_rot);
	m_bombed = false;
}

bool Bullet::frame()
{
	m_particle->Update(g_dt);
	if(m_bombed)
	{
		if(m_bombAnim->IsPlaying())
		{
			m_bombAnim->Update(g_dt);
			return false;
		}
		else return true;
	}
	float dis,desX, desY, fx, fy;
	m_target->getPos(desX, desY);
	fx = desX - m_x; fy = desY - m_y;
	dis = sqrtf(pow(fx, 2) + pow(fy, 2));
	m_rot = asinf(fy/dis);
	if(fx<0) m_rot = PI - m_rot;
	m_dx = g_bulletSpeed * cosf(m_rot);
	m_dy = g_bulletSpeed * sinf(m_rot);
	if(dis < 1.0f || m_dy > 0 || m_x < 0 || m_x > SCR_WIDTH || m_y <0 || m_y > SCR_HEIGHT)
	{
		m_target->bomb();
		m_bombAnim->Play();
		m_particle->Stop(false);
		m_bombed = true;
		return false;
	}
	m_x += m_dx, m_y += m_dy;
	return false;
}

void Bullet::render(hgeSprite* spr)
{
	if(m_bombed)
	{
		m_bombAnim->Render(m_x, m_y);
	}
	else
	{
		spr->RenderEx(m_x, m_y, m_rot);
	}

	m_particle->MoveTo(m_x, m_y);
	m_particle->Render();
}

Bullets::Bullets() : m_totalBullet(0)
{
	m_parTex = g_hge->Texture_Load("res/particles.png");
	m_bulletTex = g_hge->Texture_Load("res/bullet.png");
	m_bombTex = g_hge->Texture_Load("res/bomb.png");
	for(int i=0; i != 16; ++i)
	{
		m_parSpr[i] = new hgeSprite(m_parTex, i/4 * 32, i%4 * 32, 32, 32);
		m_parSpr[i]->SetBlendMode(BLEND_COLORMUL | BLEND_ALPHAADD | BLEND_NOZWRITE);
		m_parSpr[i]->SetHotSpot(16,16);
//		m_parSpr[i]->SetColor/*(0xffffff00); // */(g_hge->Random_Int(0xffff0000, 0xffffffff));
		m_particle[i] = new hgeParticleSystem("res/particle5.psi", m_parSpr[i]);
		m_particle[i]->Fire();
	}
	m_bulletSpr = new hgeSprite(m_bulletTex, 0,0,72,18);
	m_bulletSpr->SetHotSpot(50,9);

}

Bullets::~Bullets()
{
	for(int i=0; i != 16; ++i)
	{
		delete m_parSpr[i];
		delete m_particle[i];
	}
	delete m_bulletSpr;
	g_hge->Texture_Free(m_bulletTex);
	g_hge->Texture_Free(m_parTex);
	for(vector<Bullet*>::iterator iter = m_vec.begin(); iter != m_vec.end(); ++iter)
		delete *iter;
}

void Bullets::add(Word* word)
{
	static int i = 0;
	hgeParticleSystem* par = m_particle[++i & 0xf];
	par->Fire();
	m_vec.push_back(new Bullet(g_playerPosX, g_playerPosY, word, m_bombTex, par));
}

void Bullets::add(vector<Word*>& word)
{
	for(vector<Word*>::iterator iter = word.begin(); iter != word.end(); ++iter)
	{
		add(*iter);
	}
}

void Bullets::frame()
{
	for(vector<Bullet*>::iterator iter = m_vec.begin(); iter != m_vec.end(); )
	{		
		if((*iter)->frame())
		{
			delete *iter;
			iter = m_vec.erase(iter);
		}
		else ++iter;
	}
}

void Bullets::render()
{
	for(vector<Bullet*>::iterator iter = m_vec.begin(); iter != m_vec.end(); ++iter)
	{
		(*iter)->render(m_bulletSpr);
	}
	g_fnt1->printf(10, 490, HGETEXT_LEFT, "there're %d bullets", m_vec.size());
}

MODE TypeGame::mode = PRACTICE;

TypeGame::TypeGame() : m_way(0), m_timeLimit(10000.0f), m_escape(false)
{
	m_words.initWords();
	m_quad.tex = g_hge->Texture_Load("res/bg2.png");	
	if(!m_quad.tex) m_quad.tex = g_hge->Texture_Load("res/bg.png");

	m_quad.blend = BLEND_ALPHABLEND | BLEND_COLORMUL | BLEND_NOZWRITE;
	for(int i=0; i != 4; ++i)
	{
		m_quad.v[i].z = 0.5f;
		m_quad.v[i].col = 0xffffffff;
	}
	m_quad.v[0].x = 0; m_quad.v[0].y = 0;
	m_quad.v[1].x = SCR_WIDTH; m_quad.v[1].y = 0;
	m_quad.v[2].x = SCR_WIDTH; m_quad.v[2].y = SCR_HEIGHT;
	m_quad.v[3].x = 0; m_quad.v[3].y = SCR_HEIGHT;
	g_fnt1->SetColor(0xFFFFE060);
}

void TypeGame::reinit()
{
	m_words.wordsRandomize();
	m_player.reinit();
	m_way = 0;
	m_timeLimit = 10000.0f;
	m_bullet.reinit();
	mode = PRACTICE;
}

void TypeGame::practiceFrame()
{
	int ch;
	if(ch = g_hge->Input_GetChar())
	{
		if(isalpha(ch) || ch == '-' || ch == '.' || ch == '\'') 
		{
			if(m_words.hit(ch))
			{
				g_hge->Effect_Play(g_effect[SHOOT]);
				float x, y;
				m_bullet.add(m_words.getChosenWord());
				m_words.getChosenPos(x, y);
				m_player.setDes(x, y);
				++g_rightInputCount;
			}
			else
			{
				g_hge->Effect_Play(g_effect[CRY]);
				++g_wrongInputCount;
			}
			g_accuracyRate = (g_rightInputCount * 100.0f) / (g_rightInputCount + g_wrongInputCount);
		}
		else if(ch == 27)
			m_escape = true;
		else
		{

		}
	}

	m_words.frame();
	m_player.frame();
	m_bullet.frame();

	m_way += g_dt/10;
	float ty = -g_bgSpeed*sinf(m_way);
	m_quad.v[0].tx=m_way;					m_quad.v[0].ty=ty;
	m_quad.v[1].tx=m_way+(SCR_WIDTH/64);	m_quad.v[1].ty=ty;
	m_quad.v[2].tx=m_way+(SCR_WIDTH/64);	m_quad.v[2].ty=ty+(SCR_HEIGHT/64);
	m_quad.v[3].tx=m_way;					m_quad.v[3].ty=ty+(SCR_HEIGHT/64);
}

void TypeGame::practiceRender()
{
	g_hge->Gfx_RenderQuad(&m_quad);
	m_player.render();
	m_words.render();
	m_bullet.render();
}

Word* TypeGame::stageFrame()
{
	int ch;
	if(ch = g_hge->Input_GetChar())
	{
		if(isalpha(ch) || ch == '-' || ch == '.' || ch == '\'') 
		{
//			float x, y;
			vector<Word*> vec;
			if(m_words.hits(ch, vec))
			{
				g_hge->Effect_Play(g_effect[SHOOT]);
				m_bullet.add(vec);
//				m_words.getChosenPos(x, y);
//				m_player.setDes(x, y);
			}
			else
			{
				g_hge->Effect_Play(g_effect[CRY]);
				g_deathSpeed += 0.02f;
				if(g_timeLeastLimit > 0.1f) g_timeLeastLimit -= 0.01f;
				if(g_delay > 1.0f) g_delay -= 0.01f;
			}
			
		}
		else if(ch == 27)
			m_escape = true;
// 		else
// 		{
// 			
// 		}
	}

	m_player.frame();
	m_bullet.frame();

	m_way += g_dt;
	float ty = -2.0f*sinf(m_way / 10);
	m_quad.v[0].tx=m_way;					m_quad.v[0].ty=ty;
	m_quad.v[1].tx=m_way+(SCR_WIDTH/64);	m_quad.v[1].ty=ty;
	m_quad.v[2].tx=m_way+(SCR_WIDTH/64);	m_quad.v[2].ty=ty+(SCR_HEIGHT/64);
	m_quad.v[3].tx=m_way;					m_quad.v[3].ty=ty+(SCR_HEIGHT/64);
	return m_words.frame();
}

void TypeGame::stageRender()
{
	g_hge->Gfx_RenderQuad(&m_quad);
	m_player.render();
	m_words.render();
	m_bullet.render();
	g_fnt1->printf(10, 510, HGETEXT_LEFT, "You've stood for %.2f seconds!", m_way);
}

Word* TypeGame::onlineServerFrame()
{
	return stageFrame();
}

void TypeGame::onlineClientFrame()
{
	extern NetControl g_netCtrl;
	int ch;
	if(ch = g_hge->Input_GetChar())
	{
		if(isalpha(ch) || ch == '-' || ch == '.' || ch == '\'') 
		{
			vector<Word*> vec;
			if(m_words.hits(ch, vec))
			{
				g_hge->Effect_Play(g_effect[SHOOT]);
				m_bullet.add(vec);
			}
			else
			{
				g_hge->Effect_Play(g_effect[CRY]);
				g_deathSpeed += 0.02f;
				if(g_timeLeastLimit > 0.1f) g_timeLeastLimit -= 0.01f;
				if(g_delay > 1.0f) g_delay -= 0.01f;
			}

		}
		else if(ch == 27)
			m_escape = true;

	}

	g_netCtrl.lock(20);
	m_words.wordsUpdate(true);
	g_netCtrl.unLock();
	m_player.frame();
	m_bullet.frame();

	m_way += g_dt;
	float ty = -2.0f*sinf(m_way / 10);
	m_quad.v[0].tx=m_way;					m_quad.v[0].ty=ty;
	m_quad.v[1].tx=m_way+(SCR_WIDTH/64);	m_quad.v[1].ty=ty;
	m_quad.v[2].tx=m_way+(SCR_WIDTH/64);	m_quad.v[2].ty=ty+(SCR_HEIGHT/64);
	m_quad.v[3].tx=m_way;					m_quad.v[3].ty=ty+(SCR_HEIGHT/64);
}

void TypeGame::onlineServerRender()
{
	g_hge->Gfx_RenderQuad(&m_quad);
	m_player.render();
	m_words.render();
	m_bullet.render();
	g_fnt1->printf(10, 510, HGETEXT_LEFT, "You've stood for %.2f seconds!", m_way);
}

void TypeGame::onlineClientRender()
{
	extern NetControl g_netCtrl;
	g_hge->Gfx_RenderQuad(&m_quad);
	m_player.render();
	g_netCtrl.lock(20);
	m_words.render();
	g_netCtrl.unLock();
	m_bullet.render();
	g_fnt1->printf(10, 510, HGETEXT_LEFT, "You've stood for %.2f seconds!", m_way);
}

TypeGame::~TypeGame()
{
	g_hge->Texture_Free(m_quad.tex);
}

bool initGame()
{
	return initmenu() && initEffects();
}

void releaseGameRes()
{
	releaseMenuRes();
	releaseEffects();
	delete g_game;
}

bool practiceFrameFunc()
{
	g_dt = g_hge->Timer_GetDelta();
	g_game->practiceFrame();
	return false;
}

bool practiceRenderFunc()
{
//	g_hge->Gfx_Clear(0);
	g_hge->Gfx_BeginScene();
	g_game->practiceRender();
	g_fnt1->printf(10, 510, HGETEXT_LEFT, "You missed %d words and you killed %d words.\nyou did %d wrong input,the accuracy rate is %g %%", g_missedWordsNum, g_killedWordsNum, g_wrongInputCount, g_accuracyRate);
	g_hge->Gfx_EndScene();

	if(g_game->showSettings())
	{
		settings();
		g_game->resetEscape();
		g_hge->System_SetState(HGE_FRAMEFUNC, settingFrame);
		g_hge->System_SetState(HGE_RENDERFUNC, settingRender);
	}

	return false;
}

bool stageFrameFunc()
{
	g_dt = g_hge->Timer_GetDelta();
	g_game->stageFrame();

	return false;
}

bool stageRenderFunc()
{
	g_hge->Gfx_BeginScene();
	g_game->stageRender();
	g_hge->Gfx_EndScene();

	if(g_game->showSettings())
	{
		g_game->resetEscape();
		g_hge->System_SetState(HGE_FRAMEFUNC, menuFrameFunc);
		g_hge->System_SetState(HGE_RENDERFUNC, menuRenderFunc);
		g_fnt1->SetScale(1.0f);
		g_gui->Enter();
		g_hge->Channel_StopAll();
		g_hge->Effect_Play(g_effect[HAPPY]);
	}
	if(g_missedWordsNum != 0)
	{
		//test onlineWin();
		initLoseAction();
		g_hge->System_SetState(HGE_FRAMEFUNC, loseFrameFunc);
		g_hge->System_SetState(HGE_RENDERFUNC, loseRenderFunc);
	}
	return false;
}

void initLoseAction()
{
	g_lose = new LoseAction();
	g_fnt1->SetScale(1.0f);
}
// 
// void playHappyAndBgsnd(void*)
// {
// 	g_hge->Channel_StopAll();
// 	HCHANNEL hchnl= g_hge->Effect_Play(g_effect[HAPPY]);
// 	int tme = 0;
// 	while(g_hge->Channel_IsPlaying(hchnl))
// 	{
// 		tme += 2000;
// 		Sleep(2000);
// 		if(tme > 20000)
// 			break;
// 	}
// 	g_hge->Channel_StopAll();
// 	g_hge->Effect_Play(g_effect[BGSOUND]);
// }