/* Type definitions and function declarations */

#define DIRECTINPUT_VERSION 0x800
#define WIN32_LEAN_AND_MEAN
#define OR_DEBUG
//#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "windows.h"
#include "stdafx.h"
#include "windowsx.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <dinput.h>
#include "math.h"

#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <mmsystem.h>
#include <dsound.h>
#include <dshow.h>
//#include <dmusici.h>
#include "macros.h"
#include "float_cast.h"


using namespace std;

#define WM_GRAPHEVENT	WM_USER

#define RADARWIDTH 128
#define RADARHEIGHT 80
//#define NUMWEAPONS numberofweapons
#define MAXWEAPONMOUNTS 4
#define MAXENGINES 2
#define MAXEXPLOSIONS 10

#define MAXPOWERMETERVALUE 600.0f
#define MAXSPEEDMETERVALUE 450.0f
#define MAXENERGYMETERVALUE 500.0f

#define TILESIZE 64
#define HALFTILESIZE 32
#define FTILESIZE 64.0f
#define FHALFTILESIZE 32.0f
#define PI 3.141593f
#define TWOPI 6.283185f
#define FOURPI 12.56637f
#define ONEOVERTWOPI 0.159155f
#define PIBYTWO 1.570796f
#define PIBYFOUR 0.785398f

#define GRIDWIDTH 2
#define GRIDSIZE 128
#define GRIDSIZEF 128.0f
#define ONEOVERGRIDSIZE 0.0078125f

#define FONTWIDTH 12
#define FONTHEIGHT 26

#define NONE 0

#define DEFAULTSCREENHEIGHT 600.0f
#define SHIFTSLOWDOWN 500.0f
#define SHIFTSPEED 1000.0f
#define EXPLOSIONEXPANDSPEED 300.0f
#define ACCELTIME 0.15f

enum GAMEMODE {MENU=1, LOADLEVEL, PLAY, GAMEOVER, DEACTIVATED, INBASE, TELEPORTING, DEAD, SHORTINTRO, INTRO, OUTRO, EXITING, EXITLEVEL};
enum PLAYMODE {SINGLEPLAYER, TWOPLAYER};
enum {HORIZONTAL = 1, VERTICAL = 2, POSITIVE = 3, NEGATIVE = 4 };


//Any non-solid object types must have a number lower than EXPLOSION
//Any non-enemy-interacting object types must have a number higher than BASE
enum GAME_OBJ_TYPE {INERT, FADE, EXPLOSION,										//Completely non-interacting types
					BULLET, MISSILE, SHIELDDRONE, CONVERTER, ENEMY, MINE, ROCK, SPAWN, TURRET, SPINNER, TRIGGERSWITCH,	//Enemy collidable types
					TELEPORT, BASE, SHIELDCHARGER, EXITGATE, DRONE, LIGHTNINGDRONE, SPAWNINERT, GAMESAVER, MISSILECRATE, BLUEPRINT, GEMSTONE, ENERGY, PLAYER, BUTTONSWITCH};	//Player interaction only types

enum COMMANDS { CONTINUE, WAIT, OPERATE, SHOOT };
enum TARGETSTATE { NOTREADY, FINALTARGET, NEWPATH };
enum BUTTONS { SHIP, SELECT, EQUIP, WEAPON, REMOVE, EXIT, HOME, BACK, BLANK };
enum METERS { MPOWER, MENERGY, MSPEED, MTOTAL, MUSED };
enum CONTROLTYPE { KEYBOARD, MOUSE, GAMEPAD };
enum FONTCOLOUR { BLUE = 0xe07888c0, BLUEHIGHLIGHT = 0xff7888ff, GREEN = 0xe078c088, GREENHIGHLIGHT = 0xff78ff88, RED = 0xe0c07888, REDHIGHLIGHT = 0xffff7888 }; 

enum BEHAVIOUR { BOUNCE=1, SLIDE, ABSORB, STOP };

enum OBJSTATE { RETREATING = 0x1, HUNTERBULLET = 0x2, ARMED = 0x4, LOCKED = 0x8, STOREEQUIPMENT = 0x10, PLAYER1INSIGHT = 0x20,
				PLAYER2INSIGHT = 0x40, FORDESTRUCTION = 0x80, CREATEEXPLOSION = 0x100, ENEMYBULLET = 0x200, CREATESPARKLE = 0x400, 
				HOMINGMISSILE = 0x800, STOREWEAPON = 0x1000, OSCILLATE = 0x2000, RANGELIMITED = 0x4000, ON = 0x8000,
				WAITING = 0x10000, LINKEDTOBOUNDARY = 0x20000, LINKEDFROMBOUNDARY = 0x40000, CREATEBULLETS = 0x80000, 
				TRIGGEREDBYPLAYER = 0x100000, TRIGGEREDBYENEMY = 0x200000, TRIGGEREDBYROCK = 0x400000, STUCKONWALL = 0x800000,
				FREEINGSELF = 0x1000000, PLAYER1SIGHTED = 0x2000000, PLAYER2SIGHTED = 0x4000000, PLAYER1INRANGE = 0x8000000,
				PLAYER2INRANGE = 0x10000000, CREATEPUFF = 0x20000000, POINTDETECTION = 0x40000000, PLAYERBULLETS = 0x80000000};

enum DIRECTION { TOWARDS, AWAY, FORWARDS, BACKWARDS, CENTRED };
enum CORNER { TOPLEFT, TOPRIGHT, BOTTOMRIGHT, BOTTOMLEFT, LEFT, RIGHT, TOP, BOTTOM };

enum EQUIPMENT {NOTHING, RADAR, SCANNER, TARGET_LASER, GEMTRACTOR, SPREADER, BOOSTER, SIDE_BOOST, AUXILIARY_GENERATOR,
				MISSILE_LAUNCHER, MISSILES, HOMING_COMPUTER, LASER_DRONE, BOMB, MINIMISSILES, SHIELD, SHIELD_DRONE,
				SHIELD_RECHARGER, SEEKER, LIGHTNING_DRONE, SELF_REPAIR,	MISSILEBOMBS, LIGHTNING_POD, NUMEQUIPTYPES};

enum SOUNDEFFECT {MISSILEEXPLODE, ENEMYEXPLODE, MINEEXPLODE, PLAYEREXPLODE, GENERALEXPLODE, COLLECT, USETELEPORT, 
				BLOCKED, LANDINBASE, SHIELDHIT, TURRETROTATE, LEVELCOMPLETE, MISSILEFIRE, MISSILELOCK, 
				MISSILEPROX, SHIELDLOW, ENERGYLOW, IMPACT, BULLETIMPACT, MENUMOVE, MENUSELECT, MENUREJECT, 
				OPERATESWITCH, INSTALLITEM, INTROIMPACT, NOMISSILES, SPAWNSOUND, CHARGEUP, CRACKLE, SP_SYSTEMS_ACTIVATED, 
				SP_BASE_SYSTEMS_ACTIVATED, SP_SECTOR_CLEARED, SP_WARNING, SP_SYSTEMS_DOWN, SP_ENERGY_ZERO,
				SP_ENERGY_LOW, SP_EMERGENCY, NUMEFFECTS};

enum DRAWMETHOD { QUADS, SHADER };

class Scene
{
private:
	bool scenestarted, spritestarted;
public:
	Scene::Scene(void);
	Scene::~Scene(void);
	bool Scene::SceneBegin(const D3DVIEWPORT9 *pViewport);
	bool Scene::SceneEnd(void);
	bool Scene::SceneFlip(void);
	bool Scene::SpriteBegin(void);
	bool Scene::SpriteEnd(void);
	bool Scene::ChangeViewport(const D3DVIEWPORT9 *pViewport);
};

struct Keystate
{
	bool UP, DOWN, LEFT, RIGHT, FIRE, FIREMISSILE, FIREBOMB, TARGETLASER, SHIFTLEFT, SHIFTRIGHT, OPERATE, SHIFT;
	float up, down, left, right, fire, firemissile, firebomb, targetlaser, shiftleft, shiftright, operate, shift;
	Keystate::Keystate()
	{
		Clear();
	}
	void Keystate::Clear()
	{
		UP = false; DOWN = false; LEFT = false; RIGHT = false; FIRE = false; FIREMISSILE = false; FIREBOMB = false;
		TARGETLASER = false; SHIFTLEFT = false; SHIFTRIGHT = false; OPERATE = false;
		SHIFT = false;
		up = 1.0;
		down = 1.0;
		left = 1.0;
		right = 1.0;
		fire = 1.0;
		firemissile = 1.0;
		firebomb = 1.0;
		targetlaser = 1.0;
		shiftleft = 1.0;
		shiftright = 1.0;
		operate = 1.0;
		shift = 1.0;
	}
	Keystate::~Keystate() {}
};

struct Options
{
	bool boundarychanged;
	bool gamerunning;
	bool cheatmode;
	bool playintro;
	bool musicon;
	bool framecounter;
	bool aa;
	bool acceleration;
	bool widescreen;
	bool hidcontrollers;
	bool explosiondistortions;
	bool explosionflashes;
	bool PS3available;
	bool PS2available;
	float musicvolume, soundvolume;
	int gamepadplayer, keyboardplayer, mouseplayer;
	int maxexplosions;
	int capexplosions;
	int drawmethod;
	Options::Options()
	{
		boundarychanged = false;
		gamerunning = false;
		cheatmode = false;
		playintro = false;
		musicon = true;
		framecounter = false;
		aa = true;
		acceleration = true;
		widescreen = false;
		hidcontrollers = false;
		explosiondistortions = true;
		explosionflashes = true;
		PS3available = false;
		PS2available = false;
		musicvolume = 1.0f;
		soundvolume = 1.0f;
		gamepadplayer = 0;
		mouseplayer = 1;
		keyboardplayer = 0;
		maxexplosions = 5;
		capexplosions = 5;
		drawmethod = QUADS;
	}
	Options::~Options() {}
};

//Custom vertex
struct oldTLVERTEX
{
    float x;
    float y;
    float z;
    float rhw;
    D3DCOLOR colour;
    float u;
    float v;
};

//Custom vertex
struct TLVERTEX
{
    float x;
    float y;
    float z;
    float rhw;
	D3DCOLOR colour;
	float u0;
	float v0;
    float u1;
    float v1;
	float u2;
	float v2;
};

struct Fsincos_t
{
   float sine;
   float cosine;
};

struct t_vector
{
	float x, y;
	t_vector::t_vector() {x = 0; y = 0;}
	t_vector::~t_vector() {}
};

struct Vertexlist
{
	bool draw;
	t_vector position;
};

struct tPolygon
{
	t_vector *vertex;
	int points;
	tPolygon::tPolygon() {points = 0; vertex = NULL;}
	tPolygon::tPolygon(int number)
	{
		vertex = NULL;
		points = number;
		if(number>0) vertex = new(nothrow) t_vector[points];
	}
	tPolygon::tPolygon(const tPolygon& s)
	{
		if(s.vertex==NULL)
		{
			vertex = NULL;
			points = 0;
		}
		else
		{
			vertex = new t_vector[s.points];
			points = s.points;
			for(int i=0;i<points;i++) vertex[i] = s.vertex[i];
		}
	}
	tPolygon& tPolygon::operator=(const tPolygon& s)
	{
		if(s.vertex==NULL)
		{
			vertex = NULL;
			points = 0;
		}
		else
		{
			vertex = new t_vector[s.points];
			points = s.points;
			for(int i=0;i<points;i++) vertex[i] = s.vertex[i];
		}
		return *this;
	}
	tPolygon::~tPolygon()
	{
		if(points>0 && vertex!=NULL) delete [] vertex;
		vertex = NULL;
	}
	void tPolygon::SetPoints(int number)
	{
		if(points>0 && vertex!=NULL) delete [] vertex;
		points = number;
		vertex = NULL;
		if(number>0) vertex = new(nothrow) t_vector[points];
		if(vertex == NULL) points = 0;
	}
	void tPolygon::Clear()
	{
		if(points>0 && vertex!=NULL) delete [] vertex;
		points = 0;
		vertex = NULL;
	}
};


struct t_point
{
	long x, y;
};


struct t_location
{
	float x, y;
	float angle;
};

struct TileCorner
{
	t_vector realposition;
	t_point screenposition;
	t_vector texturecoord;
	bool onscreen;
};

struct TestTile
{
	t_point tilelocation;
	int tilenumber;
	bool onscreen;
	TileCorner corner[4];
};

struct ExplosionData
{
	float x;
	float y;
	float radius;		//In pixels
	float scale;		//To dampen wider circles
};

struct Explosion
{
	t_vector position;
	float radius;		//In pixels
	float scale;		//Size of effect
	bool active;
	Explosion::Explosion() {}
	Explosion::Explosion(int dummy) {}
	void Explosion::Update();
};

struct Explosions
{
	Explosion exps[MAXEXPLOSIONS];		//List of distortion explosions
	int explosioncounter;
	Explosions::Explosions() {explosioncounter = 0;}
	void Explosions::AddExplosion(float x, float y);
	void Explosions::Update();
};

//Layout for the base
struct BaseLayout
{
	t_point interfacescreenposition, datascreenposition, messagescreenposition, panelposition, logoposition, selectionbarposition;
	t_point interfacescreen2position, interfacescreen3position;
	t_point descstringposition, descstring2position, datastringposition, messagestringposition;
	t_point bposition[6];
	t_point meterposition[3];
	int descstringlimit, descstring2limit, datastringlimit, messagestringlimit;
};


/*** Make a proper copy constructor for this struct ***/
struct SoundEffect
{
	bool play, loop, stop, looping;
	float distance, volumefloat;
	int volume;
	LPDIRECTSOUNDBUFFER buffer;
	SoundEffect::SoundEffect()
	{
		stop = false;
		play = false;
		loop = false;
		looping = false;
		distance = 0;
		volumefloat = 1.0;
		volume = 0;
		buffer = NULL;
	}
	SoundEffect::~SoundEffect()
	{
		if(buffer!=NULL) buffer->Release();
		buffer = NULL;
	}
	void SoundEffect::SetSoundEffect(float dist);
};


/*** Make a proper copy constructor for this struct ***/
struct GraphicalObject
{
	LPDIRECT3DTEXTURE9 texture, bumptexture, colourtexture;
	int totalwidth, totalheight, framewidth, frameheight, frames, orientation, xframes, yframes;
	float flatness;
	bool bumpmapped;
	char name[100];
	D3DXIMAGE_INFO info;
	GraphicalObject::GraphicalObject()
	{
		texture = NULL;
		bumptexture = NULL;
		colourtexture = NULL;
		frames = 0;
		orientation = NONE;
		bumpmapped = false;
	}
	GraphicalObject::GraphicalObject(int numframes)
	{
		texture = NULL;
		bumptexture = NULL;
		colourtexture = NULL;
		frames = numframes;
		orientation = NONE;
		bumpmapped = false;
	}
	GraphicalObject::GraphicalObject(const GraphicalObject& s)
	{
		texture = s.texture;
		if(texture!=NULL) texture->AddRef();
		bumptexture = NULL;
		colourtexture = NULL;
		totalwidth = s.totalwidth;
		totalheight = s.totalheight;
		framewidth = s.framewidth;
		frameheight = s.frameheight;
		frames = s.frames;
		orientation = s.orientation;
		flatness = s.flatness;
		bumpmapped = s.bumpmapped;
		strcpy_s(name,100,s.name);
		info = s.info;
	}
	GraphicalObject& GraphicalObject::operator=(const GraphicalObject& s)
	{
		texture = s.texture;
		if(texture!=NULL) texture->AddRef();
		bumptexture = NULL;
		colourtexture = NULL;
		totalwidth = s.totalwidth;
		totalheight = s.totalheight;
		framewidth = s.framewidth;
		frameheight = s.frameheight;
		frames = s.frames;
		orientation = s.orientation;
		flatness = s.flatness;
		bumpmapped = s.bumpmapped;
		strcpy_s(name,100,s.name);
		info = s.info;
		return *this;
	}
	bool GraphicalObject::CreateGraphicalObject(const char *, int, float);
	bool GraphicalObject::CreateGraphicalObject(const char *, int);
	bool GraphicalObject::CreateGraphicalObject(int, int);
	GraphicalObject::GraphicalObject(const char *filename, int numframes, float objectflatness)
	{
		texture = NULL;
		bumptexture = NULL;
		colourtexture = NULL;
		frames = numframes;
		flatness = objectflatness;
		orientation = NONE;
		bumpmapped = false;
		if(!CreateGraphicalObject(filename, frames, flatness)) strcpy_s(name,100,"Error_creating_object");
	}
	void GraphicalObject::ClearGraphicalObject()
	{
		if(texture != NULL)	texture->Release();
		if(bumptexture != NULL) texture->Release();
		if(colourtexture != NULL) texture->Release();
		ZeroMemory(this,sizeof(GraphicalObject));
	}
	GraphicalObject::~GraphicalObject()
	{
		if(texture != NULL)	texture->Release();
		if(bumptexture != NULL) texture->Release();
		if(colourtexture != NULL) texture->Release();
	}
};

struct TileGraphics
{
	LPDIRECT3DTEXTURE9 tiles;
	LPDIRECT3DTEXTURE9 loadtiles;
	GraphicalObject* tilegraphics;
	int numbertiles, width, height, xtiles, ytiles, xshift;
	float xtextureincrement, ytextureincrement;
	float textureshrinkx, textureshrinky;

	TileGraphics::TileGraphics() { tiles = NULL; loadtiles = NULL; tilegraphics = NULL; }
	TileGraphics::TileGraphics(char *filename)
	{
		loadtiles = NULL;
		tiles = NULL;
		tilegraphics = NULL;
		TileGraphics::LoadTiles(filename);
	}
	TileGraphics::~TileGraphics()
	{
		if(loadtiles!=NULL) loadtiles->Release();
		if(tiles!=NULL) tiles->Release();
		if(tilegraphics!=NULL) delete [] tilegraphics;
	}
	bool TileGraphics::LoadTiles(char *filename);
	bool TileGraphics::CreatePowerTwoSheet();
	void TileGraphics::FindOptimumDimensions(int *x, int *y);
	void TileGraphics::GetTextureCoordinates(TestTile *tile);
	bool TileGraphics::ClearTiles()
	{
		if(loadtiles!=NULL) loadtiles->Release();
		if(tiles!=NULL) tiles->Release();
		if(tilegraphics!=NULL) delete [] tilegraphics;
		tiles = NULL;
		loadtiles = NULL;
		tilegraphics = NULL;
		numbertiles = 0;
	}
};

struct Weapon
{
	GraphicalObject *graphics, *pulsegraphics, *trailgfx;
	int number;
	float energy, faderate, firerate, fadevalue, speed, power;
	float trailalpha, trailfaderate, trailgrowrate, bulletgrowrate, bulletscale;
	bool bounce, explode, trail, charge, scalecentre, available;
	string name, description, graphicsfile, pulsefile, soundfile;
	SoundEffect sound;
	Weapon::Weapon()
	{
		name = "Empty Weapon";
		description = "Empty Weapon";
		graphicsfile = "empty_object";
		pulsefile = "empty_object";
		soundfile = "Empty Weapon";
		graphics = NULL;
		sound.buffer = NULL;
		sound.play = false;
		explode = false;
		bounce = false;
		trail = false;
		charge = false;
		scalecentre = false;
	}
	Weapon::~Weapon() {}
};

struct Equipment
{
	float value, power;
	string name;
	string description;
	string graphics;
	Equipment::Equipment()
	{
		value = 0;
		power = 0;
	}
	Equipment::~Equipment() {}
};

struct MeshPoint
{
	t_vector position;			//Position of mesh point
	int action;					//action is the action to take on reaching this node
	int links;					//links is the number of links to nearby nodes
	int number, parent;			//This node's index number - so if we only have a pointer we can still use it
//	MeshPoint **shortestlink;	//shortestlink tells us which node to travel to next for shortest path to node i
	MeshPoint **link;			//link is an array of pointers to the nodes this one is linked to
	float *distance;			//array of straight-line distances to the nodes this one is linked to
	float F, G, H;				//The three quantities for the pathfinding algorithm
	MeshPoint::MeshPoint()
	{
		position.x = 0;
		position.y = 0;
		action = CONTINUE;
		links = 0;
		number = 0;
		//shortestlink = NULL;
		link = NULL;
		distance = NULL;
	}
	MeshPoint::MeshPoint(int meshlinks)
	{
		position.x = 0;
		position.y = 0;
		action = CONTINUE;
		links = 0;
		number = 0;
		//shortestlink = new MeshPoint*[meshlinks];
		//for(int i=0;i<meshlinks;i++) shortestlink[i] = NULL;
		link = new MeshPoint*[meshlinks];
		for(int i=0;i<meshlinks;i++) link[i] = NULL;
		distance = NULL;
	}
	int MeshPoint::GetNextNode(int finalnode)
	{
		return 0;
	}
/*	void MeshPoint::SetShortestLinks(int meshlinks)
	{
		if(shortestlink!=NULL) delete [] shortestlink;
		shortestlink = new MeshPoint*[meshlinks];
		for(int i=0;i<meshlinks;i++) shortestlink[i] = NULL;
	}*/
	void MeshPoint::SetLinks(int meshlinks)
	{
		if(link!=NULL) delete [] link;
		if(distance != NULL) delete [] distance;
		links = meshlinks;
		link = new MeshPoint*[meshlinks];
		for(int i=0;i<meshlinks;i++) link[i] = NULL;
		distance = new float[meshlinks];
	}
	MeshPoint::~MeshPoint()
	{
		if(link != NULL) delete [] link;
		//if(shortestlink != NULL) delete [] shortestlink;
		if(distance != NULL) delete [] distance;
	}
};

struct GameObject;

struct t_Path
{
	int nodes, node;
	bool valid, needschecking;
	MeshPoint **path;
	t_vector startpoint, endpoint;
	t_Path::t_Path()
	{
		path = NULL;
		nodes = 0;
		node = 0;
		valid = false;
		needschecking = false;
	}
	void t_Path::SetPathLength(int length)
	{
		path = new MeshPoint*[length];
		nodes = length;
		node = 0;
		valid = false;
		needschecking = false;
	}
	bool t_Path::MakePath(t_vector start, t_vector end);
	bool t_Path::CheckPath();
	bool t_Path::NextNode(GameObject *object);
	void t_Path::ClearPath()
	{
		if(path!=NULL) delete [] path;
		path = NULL;
		nodes = 0;
		node = 0;
		valid = false;
		needschecking = false;
	}
	t_Path::~t_Path()
	{
		if(path!=NULL) delete [] path;
	}
};

/*** Make a proper copy constructor for this struct ***/
struct GameObject
{
	int type, behaviouroncollision;
	t_vector position, velocity, unitvectorforward, target, initialtarget, initialposition, shiftvector;
	t_point scrposition[2];

	float angle, sinangle, cosangle, angletotarget, changeangle;
	float distancetotarget, radiussquared, radius, rootradius;
	float energy, maxenergy, sensorrange, attackrange, flatness;
	float forwardsspeed, backwardsspeed, turnspeed, shiftspeed;		//Speeds in each direction - shiftspeed is the speed of a object that's been shot
	float aggression, rangelimit, strength, minangle, maxangle;

	GameObject *childobject;
	int childobjects;

	union
	{
		bool onscreen[2];
		unsigned short isonscreen;
	};
	bool objective, shape;
	int gridnumber, gridx, gridy;

	GraphicalObject *graphics;
	//bool graphicsowned;
	unsigned int alpha, frame, frames, radarcolour, baralpha;
	float fade, faderate, scale, scalerate;
	bool scalecentre;

	Weapon *weapon;
	t_location weaponmount[MAXWEAPONMOUNTS];
	t_vector engine[MAXENGINES];
	int weaponmounts, engines;

	tPolygon outline, rotatedoutline, predictedoutline;
	int targetnode, nextnode;			//Pathfinding stuff - targetnode is the node we're aiming for, nextnode is the node we're going to next
	t_Path path;

	int link, state, lastfired, timer;		//For links to other objects - e.g. linking teleporters together, etc.

	string origfilename;
	char name[100];
	string shapefile;
	string graphicsname;
	string storestring;

	GameObject::GameObject()
	{
		graphics = NULL;
		childobject = NULL;
		childobjects = 0;
		shape = false;
		onscreen[0] = false;
		onscreen[1] = false;
		state = 0;
		objective = false;
		//graphicsowned = false;
		alpha = 0xffffffff;
		baralpha = 0;
		radarcolour = 0;
		scale = 1.0;
		scalerate = 0;
		scalecentre = true;
		type = INERT;
		sinangle = 0;
		cosangle = 1.0;
		unitvectorforward.x = 1.0;
		unitvectorforward.y = 0;
		angle = 0;
		minangle = 0;
		maxangle = 0;
		changeangle = 0;
		distancetotarget = 0;
		strcpy_s(name,100,"EMPTY");
		timer = 0;
		lastfired = 0;
		gridnumber = 0;
		shiftspeed = 0;
	}
	GameObject::GameObject(int objecttype)
	{
		graphics = NULL;
		childobject = NULL;
		childobjects = 0;
		shape = false;
		onscreen[0] = false;
		onscreen[1] = false;
		state = 0;
		objective = false;
		//graphicsowned = false;
		alpha = 0xffffffff;
		baralpha = 0;
		radarcolour = 0;
		scale = 1.0;
		scalerate = 0;
		scalecentre = true;
		type = objecttype;
		sinangle = 0;
		cosangle = 1.0;
		unitvectorforward.x = 1.0;
		unitvectorforward.y = 0;
		angle = 0;
		minangle = 0;
		maxangle = 0;
		changeangle = 0;
		distancetotarget = 0;
		strcpy_s(name,100,"EMPTY");
		timer = 0;
		lastfired = 0;
		gridnumber = 0;
		shiftspeed = 0;
	}
	GameObject::GameObject(const GameObject& s)
	{
		type = s.type;
		behaviouroncollision = s.behaviouroncollision;
		position = s.position;
		velocity = s.velocity;
		unitvectorforward = s.unitvectorforward;
		target = s.target;
		initialtarget = s.initialtarget;
		initialposition = s.initialposition;
		scrposition[0] = s.scrposition[0];
		scrposition[1] = s.scrposition[1];
		angle = s.angle;
		minangle = s.minangle;
		maxangle = s.maxangle;
		sinangle = s.sinangle;
		cosangle = s.cosangle;
		angletotarget = s.angletotarget;
		changeangle = s.changeangle;
		distancetotarget = s.distancetotarget;
		radiussquared = s.radiussquared;
		rootradius = s.rootradius;
		energy = s.energy;
		maxenergy = s.maxenergy;
		strength = s.strength;
		sensorrange = s.sensorrange;
		attackrange = s.attackrange;
		flatness = s.flatness;
		forwardsspeed = s.forwardsspeed;
		backwardsspeed = s.backwardsspeed;
		turnspeed = s.turnspeed;
		aggression = s.aggression;
		rangelimit = s.rangelimit;
		childobject = s.childobject;
		childobjects = s.childobjects;
		onscreen[0] = s.onscreen[0];
		onscreen[1] = s.onscreen[1];
		objective = s.objective;
		shape = s.shape;
		graphics = NULL;
		//graphicsowned = false;
		alpha = s.alpha;
		baralpha = s.baralpha;
		frame = s.frame;
		frames = s.frames;
		fade = s.fade;
		faderate = s.faderate;
		scale = s.scale;
		scalerate = s.scalerate;
		scalecentre = s.scalecentre;
		weapon = s.weapon;
		for(int i=0;i<MAXWEAPONMOUNTS;i++) weaponmount[i] = s.weaponmount[i];
		for(int i=0;i<MAXENGINES;i++) engine[i] = s.engine[i];
		weaponmounts = s.weaponmounts;
		engines = s.engines;
		outline = s.outline;
		rotatedoutline = s.rotatedoutline;
		predictedoutline = s.predictedoutline;
		targetnode = s.targetnode;
		nextnode = s.nextnode;
		link = s.link;
		state = s.state;
		lastfired = s.lastfired;
		timer = s.timer;
		radarcolour = s.radarcolour;
		strcpy_s(name,100,s.name);
		origfilename = s.origfilename;
		shapefile = s.shapefile;
		graphicsname = s.graphicsname;
		gridnumber = s.gridnumber;
		shiftspeed = s.shiftspeed;
	}
	GameObject& GameObject::operator=(const GameObject& s)
	{
		type = s.type;
		behaviouroncollision = s.behaviouroncollision;
		position = s.position;
		velocity = s.velocity;
		unitvectorforward = s.unitvectorforward;
		target = s.target;
		initialtarget = s.initialtarget;
		initialposition = s.initialposition;
		scrposition[0] = s.scrposition[0];
		scrposition[1] = s.scrposition[1];
		angle = s.angle;
		minangle = s.minangle;
		maxangle = s.maxangle;
		sinangle = s.sinangle;
		cosangle = s.cosangle;
		angletotarget = s.angletotarget;
		changeangle = s.changeangle;
		distancetotarget = s.distancetotarget;
		radiussquared = s.radiussquared;
		rootradius = s.rootradius;
		radarcolour = s.radarcolour;
		energy = s.energy;
		maxenergy = s.maxenergy;
		strength = s.strength;
		sensorrange = s.sensorrange;
		attackrange = s.attackrange;
		flatness = s.flatness;
		forwardsspeed = s.forwardsspeed;
		backwardsspeed = s.backwardsspeed;
		turnspeed = s.turnspeed;
		aggression = s.aggression;
		rangelimit = s.rangelimit;
		childobject = s.childobject;
		childobjects = s.childobjects;
		onscreen[0] = s.onscreen[0];
		onscreen[1] = s.onscreen[1];
		objective = s.objective;
		shape = s.shape;
		graphics = s.graphics;
		//graphicsowned = false;
		alpha = s.alpha;
		baralpha = s.baralpha;
		frame = s.frame;
		frames = s.frames;
		fade = s.fade;
		faderate = s.faderate;
		scale = s.scale;
		scalerate = s.scalerate;
		scalecentre = s.scalecentre;
		weapon = s.weapon;
		for(int i=0;i<MAXWEAPONMOUNTS;i++) weaponmount[i] = s.weaponmount[i];
		for(int i=0;i<MAXENGINES;i++) engine[i] = s.engine[i];
		weaponmounts = s.weaponmounts;
		engines = s.engines;
		outline = s.outline;
		rotatedoutline = s.rotatedoutline;
		predictedoutline = s.predictedoutline;
		targetnode = s.targetnode;
		nextnode = s.nextnode;
		link = s.link;
		state = s.state;
		gridnumber = s.gridnumber;
		lastfired = s.lastfired;
		timer = s.timer;
		strcpy_s(name,100,s.name);
		origfilename = s.origfilename;
		shapefile = s.shapefile;
		graphicsname = s.graphicsname;
		shiftspeed = s.shiftspeed;
		return *this;
	}
	GameObject::~GameObject()
	{
		if(childobject!=NULL) delete [] childobject;
		outline.Clear();
		rotatedoutline.Clear();
		predictedoutline.Clear();
	}
};


/*** Make a proper copy constructor for this struct ***/
struct FadeObject
{
	int type;
	t_vector position, velocity, unitvectorforward;
	t_point scrposition[2];

	float angle, sinangle, cosangle, changeangle;
	float forwardsspeed, backwardsspeed, turnspeed;		//Speeds in each direction

	union
	{
		bool onscreen[2];
		unsigned short isonscreen;
	};

	GraphicalObject *graphics;

	unsigned int alpha, frame, frames;
	float fade, faderate, scale, scalerate;
	bool scalecentre;

	int state, lastfired, timer;		//For links to other objects - e.g. linking teleporters together, etc.

	char name[100];

	FadeObject::FadeObject()
	{
		graphics = NULL;
		onscreen[0] = false;
		onscreen[1] = false;
		state = 0;
		alpha = 0xffffffff;
		scale = 1.0;
		scalerate = 0;
		scalecentre = true;
		type = INERT;
		sinangle = 0;
		cosangle = 1.0;
		unitvectorforward.x = 1.0;
		unitvectorforward.y = 0;
		angle = 0;
		changeangle = 0;
		strcpy_s(name,100,"EMPTY");
		timer = 0;
		lastfired = 0;
	}
	FadeObject::FadeObject(int objecttype)
	{
		graphics = NULL;
		onscreen[0] = false;
		onscreen[1] = false;
		state = 0;
		alpha = 0xffffffff;
		scale = 1.0;
		scalerate = 0;
		scalecentre = true;
		type = objecttype;
		sinangle = 0;
		cosangle = 1.0;
		unitvectorforward.x = 1.0;
		unitvectorforward.y = 0;
		angle = 0;
		changeangle = 0;
		strcpy_s(name,100,"EMPTY");
		timer = 0;
		lastfired = 0;
	}
	FadeObject::FadeObject(const GameObject& s)
	{
		type = s.type;
		position = s.position;
		velocity = s.velocity;
		unitvectorforward = s.unitvectorforward;
		scrposition[0] = s.scrposition[0];
		scrposition[1] = s.scrposition[1];
		angle = s.angle;
		sinangle = s.sinangle;
		cosangle = s.cosangle;
		changeangle = s.changeangle;
		forwardsspeed = s.forwardsspeed;
		backwardsspeed = s.backwardsspeed;
		turnspeed = s.turnspeed;
		onscreen[0] = s.onscreen[0];
		onscreen[1] = s.onscreen[1];
		graphics = NULL;
		alpha = s.alpha;
		frame = s.frame;
		frames = s.frames;
		fade = s.fade;
		faderate = s.faderate;
		scale = s.scale;
		scalerate = s.scalerate;
		scalecentre = s.scalecentre;
		state = s.state;
		lastfired = s.lastfired;
		timer = s.timer;
		strcpy_s(name,100,s.name);
	}
	FadeObject& FadeObject::operator=(const GameObject& s)
	{
		type = s.type;
		position = s.position;
		velocity = s.velocity;
		unitvectorforward = s.unitvectorforward;
		scrposition[0] = s.scrposition[0];
		scrposition[1] = s.scrposition[1];
		angle = s.angle;
		sinangle = s.sinangle;
		cosangle = s.cosangle;
		changeangle = s.changeangle;
		forwardsspeed = s.forwardsspeed;
		backwardsspeed = s.backwardsspeed;
		turnspeed = s.turnspeed;
		onscreen[0] = s.onscreen[0];
		onscreen[1] = s.onscreen[1];
		graphics = NULL;
		alpha = s.alpha;
		frame = s.frame;
		frames = s.frames;
		fade = s.fade;
		faderate = s.faderate;
		scale = s.scale;
		scalerate = s.scalerate;
		scalecentre = s.scalecentre;
		state = s.state;
		lastfired = s.lastfired;
		timer = s.timer;
		strcpy_s(name,100,s.name);
		return *this;
	}
	FadeObject::~FadeObject()
	{
	}
};

struct LightningBolt
{
	GameObject *starttarget, *endtarget;
	float proportion;
	LightningBolt::LightningBolt() {starttarget = NULL; endtarget = NULL; proportion = 0; }
	LightningBolt::LightningBolt(int dummy) {starttarget = NULL; endtarget = NULL;  proportion = 0; }
	LightningBolt::LightningBolt(const LightningBolt& s) {starttarget = s.starttarget; endtarget = s.endtarget;  proportion = s.proportion;}
	LightningBolt::~LightningBolt() {}
	LightningBolt& LightningBolt::operator=(const LightningBolt& s) { if(this!=&s) {starttarget = s.starttarget; endtarget = s.endtarget; proportion = s.proportion;} return *this; }
};

struct LightningTree
{
	LightningBolt targetlist[20];
	int bolts;

	bool LightningTree::AddBolt(GameObject* starttarg, GameObject* endtarg, float prop)
	{
		if(bolts==20) return false;
		if(starttarg==NULL) return false;
		if(endtarg==NULL) return false;
		targetlist[bolts].starttarget = starttarg;
		targetlist[bolts].endtarget = endtarg;
		targetlist[bolts].proportion = prop;
		++bolts;
		return true;
	}
	bool LightningTree::CheckIfAlreadySelected(GameObject* object)
	{
		for(int i=0;i<bolts;++i)
			if(object==targetlist[i].endtarget) return true;
		return false;
	}
};

struct ScreenMessage
{
	string message;
	float alpha;
	DWORD colour;
	void ScreenMessage::AddMessage(const char * messagein);
	void ScreenMessage::DrawMessage(int player);
	bool ScreenMessage::Update();
};

struct Player
{
	t_point scrposition;					//Position of centre of ship on screen
	t_point scrpositionfixed;				//Fixed-point version for the drawing routine
	t_point scrtopleft, scrbottomright;		//Size of screen we're drawing to - depends on whether split-screen or not
	t_point size;							//Width/Height in the data file
	t_point radarposition;					//Position of radar on screen
	t_point energyposition;					//Position of energy bar on screen
	t_vector position;						//Physical position of ship on the level
	t_vector unitvectorforward;				//The direction of forwards;
	t_vector target;						//Target from homing computer
	t_vector targetunitvectorforward;		//Unit vector towards target from homing computer
	t_point targetscr;						//Target in screen coords
	t_point messageposition;				//Position that the game messages are drawn
	t_vector screencorner[4];				//The distances to the corners of the screen relative to the ship's on screen position
	tPolygon screen;						//The actual position of the screen corners
	float angle, radiussquared;				//Angle of the ship
	float sinangle, cosangle;				//Recalculated each time ship rotates
	float forwardsspeed, backwardsspeed, turnspeed;		//Speeds in each direction
	float currentforwardsspeed;				//Speed ship has accelerated to forwards
	float currentbackwardsspeed;			//Back
	float currentleftturnspeed;				//Left
	float currentrightturnspeed;			//Right
	float droneangle;						//Angular position of drone as it orbits
	float deflectordroneangle[3];			//Angular position of deflector drones as they orbit
	float targetangle;						//Angle between ship's unit vector forwards and the target
	float targetenergy;						//Energy of the target that's acquired
	float targetdistance;					//Distance to the target that's acquired

	char name[100], manufacturer[50], logo[50], shapefile[50], graphicsfile[50], blueprintfile[50], description[300], filename[100], startship[100];
	char targetname[200];					//Name of target

	GraphicalObject graphics;
	GraphicalObject radar;
	GraphicalObject logographics;

	int alpha, screenalpha, frame;

	D3DVIEWPORT9 viewport;
	GameObject *drone, *shielddrone[3], *lightningdrone, *seekertarget;

	LightningTree lightningtree;

	float flatness, screenscale;					//Flatness value for the bumpmapping, scaling of the screen
	float strength, shields, energy, power, startenergy, startshields, startpower;				//Energy of the ship and shields
	float maxstrength, maxshields, maxenergy, maxpower;	//Power level of the ship's generator
	float enginepower, shieldpower, weaponpower, maxengine, shieldbrightness, shieldfrac;
	float radarrange;
	float missiledroneangle;
	float fade;
	bool equipment[NUMEQUIPTYPES], homingcomputerbusy, missilemount, stealthed, shieldhit, invulnerable, invframe, bombnew;

	bool mousecontrol, moving;
	Keystate keys;
	t_point nose;

	CONTROLTYPE controltype;
	int weaponmounts, startweapon, missiles, tracking, engines, shape, numpoints, cost;
	int maxmissiles, startmissiles, lastfired, lastlaunched, lastminimissile, lastseeker, invulnerabletimer, bombtimer, credits;
	int energylowtimer, emergencytimer;
	t_location weaponmount[MAXWEAPONMOUNTS];
	Weapon *weapon;
	t_point engine[MAXENGINES];

	tPolygon outline, rotatedoutline, predictedoutline;
	tPolygon homingsweep;
	tPolygon homingsweeprotated;
	//bool collision;
	int state;

	Explosions explosions;
	float light;
	ScreenMessage screenmessage;

	Player::Player()
	{
		weapon = NULL;
		angle = 0;
		unitvectorforward.x = -sin(angle);
		unitvectorforward.y = -cos(angle);
		alpha = 0xffffffff;
		screenalpha = 0xffffffff;
		lastfired = 0;
		lastlaunched = 0;
		lastminimissile = 0;
		lastseeker = 0;
		energylowtimer = 0;
		emergencytimer = 0;
		credits = 0;
		cost = 0;
		currentforwardsspeed = 0;
		currentbackwardsspeed = 0;
		currentleftturnspeed = 0;
		currentrightturnspeed = 0;
		homingcomputerbusy = false; missilemount = false, stealthed = false, bombnew = true;
		controltype = KEYBOARD;
		state = PLAY;
		for(int i=0;i<NUMEQUIPTYPES;i++) equipment[i] = false;
		lightningtree.bolts = 0;
		missiledroneangle = 0;
	}
	Player::~Player()
	{
		screen.Clear();
		homingsweep.Clear();
		homingsweeprotated.Clear();
		//homingsweeptranslated.Clear();
	}
};


struct Mousetarget
{
	int direction;
	t_point scrposition;
	t_vector position;
	float distance;
	float angle;
	bool forwards, backwards, operate, fire, firemissile;
	Mousetarget::Mousetarget() { direction = CENTRED; distance = 70.0; angle = 0; }
	void Mousetarget::ResetMousetarget();
	void Mousetarget::PolarToCart(int player);
	void Mousetarget::UpdatePosition(float dx, float dy, int player);
	bool Mousetarget::RotateToMouse(Player *object, float tolerance);
};


class DX9VARS {
public:
	LPDIRECT3D9 pD3D; // Used to create the D3DDevice

	LPDIRECT3DDEVICE9 pd3dDevice; // Our rendering device

	LPD3DXSPRITE sprite; // Interface to Sprite routines

	D3DCAPS9 *pd3dcaps; // Caps for the rendering device

	IDirect3DVertexBuffer9* vertexBuffer;	//Vertex buffer

	TLVERTEX vertices[3000];			//Temporary vertex buffer

	D3DVIEWPORT9 MainViewPort;

	D3DPRESENT_PARAMETERS d3dpp;

	D3DDISPLAYMODE *widemodes, *normalmodes;

	LPD3DXEFFECT rotozoom;			//Roto-zooming shader code

	D3DXHANDLE hTech;				//Handle to valid shader technique

	D3DXHANDLE hDistort[MAXEXPLOSIONS];		//Handle to valid shader techniques

	int widescreenmodes, normalscreenmodes, refreshrate;

	// Constructor/Destructor:
	DX9VARS()
	{
		pD3D = NULL;
		pd3dDevice = NULL;
		sprite = NULL;
		vertexBuffer = NULL;
		widemodes = NULL;
		normalmodes = NULL;
		pd3dcaps = new D3DCAPS9;
		widescreenmodes = 0;
		normalscreenmodes = 0;
		refreshrate = 75;
	}
	~DX9VARS() { if (pd3dcaps) delete pd3dcaps; if(widemodes) delete [] widemodes; if(normalmodes) delete [] normalmodes; }
	// If you feel very object-oriented, you could make InitD3D(), Cleanup()
	// and Render() all methods of the DX9VARS class.
};

struct t_Tiledata
{
	unsigned char data[TILESIZE*TILESIZE*4];
};

struct GridSquare
{
	int *boundary;
	int boundaries;
	std::vector<GameObject*> collideobjects;
	std::vector<GameObject*>::iterator iter;
	GridSquare* neighbour[8];
	t_point pos;
	t_point size;
	GridSquare::GridSquare(void)
	{
		boundary = NULL;
		boundaries = 0;
		collideobjects.reserve(50);
	}
	GridSquare::GridSquare(int bound)
	{
		boundaries = bound;
		boundary = new int[bound];
	}
	void GridSquare::SetBoundaries(int bound)
	{
		boundaries = bound;
		boundary = new int[bound];
	}
	void GridSquare::RemoveFromGrid(GameObject *object)
	{
		if(object->type==INERT) return;
		if(object->type<TRIGGERSWITCH)
		{
			for(iter = collideobjects.begin(); iter != collideobjects.end(); ++iter)
			{
				if(*&*iter == object)
				{
					iter = collideobjects.erase(iter);
					break;
				}
			}
		}
	}
	void GridSquare::AddToGrid(GameObject *object)
	{
		if(object->type<TELEPORT) collideobjects.push_back(object);
	}
	void GridSquare::MoveTo(GameObject *object, int nextgrid)
	{
		RemoveFromGrid(object);
		neighbour[nextgrid]->AddToGrid(object);
	}
	GridSquare::~GridSquare(void)
	{
		if(boundary != NULL) delete [] boundary;
		collideobjects.clear();
	}
};

struct Boundary
{
	int numpoints, link;
	unsigned int state;
	tPolygon poly;
	Boundary::Boundary()
	{
		state = ON;
	}
	Boundary::~Boundary()
	{
		poly.Clear();
	}
};

struct Meter
{
	GraphicalObject *bar;
	int x, y, length, type;
	RECT src;
	float value, maxvalue;
	Meter::Meter()
	{
		length = 112;
		src.left = 0;
		src.right = 12;
		src.bottom = 112;
		src.top = 0;
	}
	Meter::~Meter() {}
	void Meter::DrawMeter(float limit);
};

struct ShipName
{
	char name[200];
	char filename[50];
};

struct Message
{
	char txt[100];
	Message::Message()
	{
		txt[0] = '\0';
	}
};

struct BaseData
{
	bool visited;
	char name[200];
	bool *weapon;
	int equipment[NUMEQUIPTYPES];
	int numships, numequip, cost, landingcharge;
	ShipName ships[100];
	char txtarrive[500], txtrevisit[500];
	BaseData::BaseData()
	{
		visited = false;
		numships = 0;
		cost = 100;
		landingcharge = 3;
		for(int i=0;i<NUMEQUIPTYPES;i++) equipment[i] = 0;
	}
	BaseData::BaseData(int count)
	{
		visited = false;
		numships = count;
		for(int i=0;i<NUMEQUIPTYPES;i++) equipment[i] = 0;
	}
	BaseData::BaseData(const BaseData& p)
	{
		visited = false;
		for(int i=0;i<p.numships;i++)
		{
			strcpy_s(ships[i].filename,50,p.ships[i].filename);
			strcpy_s(ships[i].name,200,p.ships[i].name);
		}
		strcpy_s(name,200,p.name);
		strcpy_s(txtarrive,500,p.txtarrive);
		strcpy_s(txtrevisit,500,p.txtrevisit);

		for(int i=0;i<NUMEQUIPTYPES;i++) equipment[i] = 0;
	}
	void BaseData::SetWeapons(int numweapons)
	{
		weapon = new bool[numweapons];
		for(int i=0;i<numweapons;i++) weapon[i] = false;
	}
	void BaseData::SetShips(int shipcount)
	{
		if(shipcount<=0) numships = 0;
		else if(shipcount>100) numships = 100;
		else numships = shipcount;
	}
	BaseData::~BaseData()
	{
		if(weapon!=NULL) delete [] weapon;
	}
};

struct Base
{
	BaseData *data;
	BaseLayout layout;
	int buttonselector;
	int listselector;
	int selectbar;
	int player;
	int lasttick;
	int button[6];
	UINT messageindex;
	Player dummy;
	GraphicalObject glyph;
	int screen;
	int frame, newmessagetime;
	float angle, storeangle;
	char messagestring[1024];
	char datastring[2048];
	char descstring[2048];
	char descstring2[2048];
	Message message[6];
	Meter meter[3];
	Base::Base()
	{
		screen = BASE;
		buttonselector = 0;
		selectbar = -1;
		button[0] = EXIT;
		button[1] = SHIP;
		button[2] = WEAPON;
		button[3] = EQUIP;
	}
	Base::Base(int id)
	{
		player = id;
		screen = BASE;
		buttonselector = 0;
		selectbar = -1;
		button[0] = EXIT;
		button[1] = SHIP;
		button[2] = WEAPON;
		button[3] = EQUIP;
	}
	Base::~Base() {}
	void Base::ProcessBase();
	void Base::SetUpBase(BaseData *currentbase);
	void Base::DrawBase();
	void Base::BaseScreen();
	void Base::ShipScreen();
	void Base::EquipScreen();
	void Base::WeaponScreen();
	void Base::LeaveBase();
	void Base::LoadDummy(char *shipname);
	void Base::ChangeScreen(int which);
	void Base::CreateLayout(int player);
	void Base::PrintMessage(char *message);
};

struct Level
{
	int num_tiles, number;
	char name[256], next[256], music[256], description[4096];
	char tiles[60];
	int width, height, newwidth, newheight, xdifference, ydifference;
	int maskx, masky, pixelwidth, pixelheight, newpixelwidth, newpixelheight, shiftleft;
	float hwidth, hheight, fwidth, fheight, mhwidth, mhheight;
	int *tilemap;
	t_point gridsize;
	int gridsquares;
	int units, boundaries, objectives, meshpoints, exitgates;
	bool cleared;
	Boundary *boundary;
	MeshPoint *mesh;
	GridSquare *grid;
	t_vector start[2];
	LPDIRECT3DTEXTURE9 map;
	Level::Level()
	{
		boundary = NULL;
		mesh = NULL;
		grid = NULL;
		tilemap = NULL;
		map = NULL;
		cleared = false;
		gridsize.x = 0;
		gridsize.y = 0;
	}
	Level::~Level()
	{
		if(boundary != NULL) delete [] boundary;
		if(mesh != NULL) delete [] mesh;
		if(tilemap != NULL) delete [] tilemap;
		if(grid != NULL) delete [] grid;
		if(map != NULL) map->Release();
	}
	void Level::ClearLevel()
	{
		if(boundary != NULL) delete [] boundary;
		if(mesh != NULL) delete [] mesh;
		if(tilemap != NULL) delete [] tilemap;
		if(grid != NULL) delete [] grid;
		if(map != NULL) map->Release();
		map = NULL;
		boundary = NULL;
		mesh = NULL;
		tilemap = NULL;
		grid = NULL;
		gridsize.x = 0;
		gridsize.y = 0;
		gridsquares = 0;
		cleared = false;
		ZeroMemory(this,sizeof(Level));
	}
	void Level::SetMeshPoints()
	{
		if(mesh != NULL) delete [] mesh;
		mesh = new MeshPoint[meshpoints];
	}
	void Level::DeleteMeshPoints()
	{
		if(mesh != NULL) delete [] mesh;
		mesh = NULL;
	}
	bool Level::CreateMap();
};

struct t_3vector
{
	float x, y, z;
};

struct GraphicsItem
{
	string name;
	float flatness;
	int frames;
};


			//**************** Function Declarations ****************//


void GetCaps();
HRESULT InitD3D();
bool BuildScreenModeList();
void SetFiltering(int action);
void Cleanup();
void Render();
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
void Play();
void PlayExiting();
void SetUp();
bool InitialiseGame(bool skiploading);
void ClearCurrentLevel();
void SetUpEquipment();
bool InitDirectInput();
bool ReleaseInput();
bool UpdateInput(int player);
void MouseToPlayer(Player *player);
bool RotateToMouse(Player *object, t_vector point, float tolerance);
void ReleaseTextures();
int InitDirectShow();
int CreateGraph(char* filename);
void CleanUpDirectShow();
void OnGraphEvent();
void SetNotifications();
int ShortIntro();
int Intro();
int Outro(int retstate);
void ReadPrefs();
void SavePrefs();
string FPClass(float x);
void InitVolatileResources();
void FreeVolatileResources();
bool SetUpControllers();
void GetInput(WPARAM wParam, LPARAM lParam, int player);
void MapPadToPlayer(int player);
void SetPlayerControllers();
bool CheckDeviceReady();
void SetViewPorts();
void SetDeviceStates();
void SetScreenCoords();
bool TestKeyPressed();

// **** Graphics functions **** //
HRESULT BlitSimple( RECT *pDestRect, LPDIRECT3DTEXTURE9 pSrcTexture, RECT *pSrcRect, DWORD dwFlags );
HRESULT BlitSimple( RECT *pDestRect, LPDIRECT3DTEXTURE9 pSrcTexture, RECT *pSrcRect, float angle, D3DCOLOR colour, DWORD dwFlags );
//HRESULT BltSpriteEx( RECT *pDestRect, LPDIRECT3DTEXTURE9 pSrcTexture, RECT *pSrcRect, DWORD dwFlags, D3DCOLOR modulate = 0xFFFFFFFF, float rotation = 0, POINT *prcenter = NULL );
void FrameAlphaInterpolate(int frames, int alpha, float angle, int *alphatwo, unsigned int *frame, int *nextframe);
int DrawGameObject(GameObject *object, int player);
int DrawFadeGameObject(FadeObject *object, int player);
HRESULT DrawDividingLine();
bool NewPreCache(GraphicalObject *object);
bool PreCache(GraphicalObject *object);
bool FindOptimumRatio(int objwidth, int objheight, int *objframes, int *optwidth, int *optheight);
HRESULT DrawGraphicalObject(GraphicalObject *object, int x, int y, int frame, float angle, D3DCOLOR colour);
HRESULT DrawGraphicalObject(GraphicalObject *object, int x, int y, RECT src, int frame, float angle, float scale, D3DCOLOR colour);
HRESULT DrawGraphicalObject(GraphicalObject *object, int x, int y, int frame, float angle, float scale, bool setscalecentre, D3DCOLOR colour);
void DrawShip(Player *inship, int player);
void DrawShipGlyphs(Player *inship, int player);
bool DrawObjects(int player);
bool DrawRadar(int player);
bool AlphaFromColour(GraphicalObject *object);
void DrawEnergyBars(int player);
void DrawShieldBars(int player);
void RenderLightning(int ownerplayer, int drawplayer);
void DrawLightning(t_point start, t_point end, GraphicalObject *lightning);
void DrawHitBars(GameObject *object, int player);
//void DrawBase(Base *base, int player);
void DrawScene();
void DrawBG(int player);
void DrawBGshader(int player);
void ShowPrepareScreen();
void ShowLogo(GraphicalObject *object, D3DCOLOR alpha);
void TranslateTileVertices(TLVERTEX* vertices, TLVERTEX* rotatedvertices, float x, float y);
void CreateTileVertices(TLVERTEX* vertices);
void CreateRotatedTileVertices(TLVERTEX* vertices, float angle, float scale);
void BlitD3D(LPDIRECT3DTEXTURE9 texture, TLVERTEX *reqvertices);
int AddTileToVertexList(TestTile *tile, TLVERTEX *vertexbuffer, int position, int buffersize, DWORD colour);
int PickExplosions(ExplosionData *expdata, int player);
void Line(GraphicalObject *linetex, int frame, int x1, int y1, int x2, int y2, int colour);


// **** Loading and saving functions **** //
LPDIRECT3DTEXTURE9 LoadTexture(char *filename, D3DXIMAGE_INFO *SrcInfo);
void DeleteLevel(Level& level);
bool LoadLevel(Level & level);
int ReadLevelFile(const char *filename, Level& level);
//bool LoadTileSet(char *filenm, Level& level);
bool ReadShip(Player *inship, char *filename);
bool LoadShip(Player *inship, char *filename);
int GetType(char *type);
void CopyPolygon(tPolygon *source, tPolygon *dest);
void CopyObject(GameObject *source, GameObject *dest);
bool ReadObject(GameObject *object, char *filename);
void SetUpObject(GameObject *object);
void LoadAssignGraphics();
bool ReadWeapons();
int GetWeaponNumber(const char *weaponname);
void CheckForGraphics(GraphicsItem *glist, int *listlength, GameObject *object);
void AssignGraphics(GraphicsItem *glist, int listlength, GameObject *object);
int GetEquipmentType(const char *instring);
void GetEquipmentString(int etype, char *instring);
void DuplicatePlayer(Player *src, Player *dst);
void ReSumChildAngles(GameObject *object, float angle);
bool SaveGame(char *filename, Level *level);
void PrepObjects(Level & level);
int ReadSaveFile(char *filename, Level& level);
bool LoadSavedGame(Level & level, bool restart);
void SortWeapons();
void SwapWeapons(Weapon *a, Weapon * b);
void CopyWeapon(Weapon *a, Weapon *b);
void CalculateGrid(Level & level);
void SetOutline(tPolygon *outline, int gridx, int gridy, Level & level);
bool CopyTileFromTexture(GraphicalObject *tile, LPDIRECT3DTEXTURE9 tilesheet, int xtile, int ytile);
bool MoveTile(LPDIRECT3DTEXTURE9 sourcesheet, LPDIRECT3DTEXTURE9 destsheet, int sxtile, int sytile, int dxtile, int dytile);
void Save();
void Scan(ifstream& fp_in, char * text);
bool CopyMap(const RECT *map, const RECT *maptexture, DWORD* mapdata, DWORD mappitch, Level& level);
void SetUpTriggers();


// **** Navigation and AI functions **** //
void MoveShip(int player);
void AccelerateShip(bool keydown, float *current, float speed);
void MoveObjects();
void MoveChildObject(GameObject *object, float parentangle);
t_vector CalculateTargetPoint(GameObject *object);
bool RepulsionVector(GameObject *object, t_vector *pos, float max, t_vector *vec, float sqrtradius);
void MoveEnemy(GameObject *object);
void MoveTurret(GameObject *object);
t_vector GetRandomTarget(GameObject *object);
bool RotateTo(GameObject *object, t_vector point, float tolerance, DIRECTION dn);
void CalculateNavigationMesh();
bool PathFind(int one, int two, int **path, int *numpoints);
void TrackTarget(GameObject *object);
t_vector GetInverseTarget(t_vector position, t_vector target);
bool CheckPath(int start, int end);
void CheckMine(GameObject *object);
t_vector FindNearestObject(t_vector position, float limit);
t_vector FindNearestEnemyBullet(t_vector position, float limit);
void MoveDrone(GameObject *object);
void MoveLightningDrone(GameObject *object);
void MoveShieldDrone(GameObject *object);
int FindFreeDirection(GameObject *object);
void SpinObject(GameObject *object);
bool HomingComputer(int player);
int FindNearestNode(t_vector start);
void GetNextTargetPoint(GameObject *object, float pdistance[], t_vector pposition[]);
void SetPlayerStatusFlags(GameObject *object, float pdistance[], t_vector playerposition[]);
void DoScreen(int player);
void MoveHunterBullet(GameObject *bullet);
void HunterSelectTarget(t_vector position, GameObject** seeker);
//void LightningDroneSelectTarget(t_vector position, GameObject** lightning, GameObject* drone);
bool CheckHunterTarget(Player *inship);
bool CheckLightningTarget(Player *inship);
void LightningSelectTarget(GameObject* starttarget, LightningTree* lightningtree, int count);


// **** Geometry functions **** //
bool PosToScreen(const t_vector *Pos, t_point *Screen, int player);
void ScreenToPos(t_vector *Pos, const t_point *Screen, int player);
t_3vector CrossProduct(t_3vector H, t_3vector V);
float Normalise(t_vector *in);
float Magnitude(t_3vector in);
void RotateOutline(const tPolygon *outline, tPolygon *rotatedoutline, float cosangle, float sinangle);
void TranslateOutline(const tPolygon *rotatedoutline, tPolygon *predictedoutline, const t_vector *position);
t_point GetRadarPosition(t_vector object, int player);
bool PointInPoly(t_vector hitPos, const tPolygon *poly);
bool PointInPolyOld(t_vector hitPos, const tPolygon *poly);
bool PolyCollisionDetect(const tPolygon *poly1, const tPolygon *poly2);
bool FullPolyCollisionDetect(const tPolygon *poly1, const tPolygon *poly2);
void CreatePolygonOutline(tPolygon *outline, float x, float y);
float FindRadius(const tPolygon *outline);
bool LinesIntersectionPoint(const t_vector *p1, const t_vector *p2, const t_vector *p3, const t_vector *p4, t_vector *intersect);
bool NearestPoint(const t_vector *P1, const t_vector *P2, const t_vector *P3, t_vector *point);
float PointLineSegmentDistance(const t_vector *P1, const t_vector *P2, const t_vector *P3, t_vector *point);
float PointLineSegmentDistanceSq(const t_vector *P1, const t_vector *P2, t_vector *P3, t_vector *point);
void WrapToLevel(t_vector *position);
float PointThatsNearest(const tPolygon *poly1, const tPolygon *poly2, tPolygon *output, int *polygon, t_vector *point);
void ResolveVector(const tPolygon *line, t_vector *velocity);
t_vector CalculateNormal(t_vector invector);
t_vector BounceVector(const tPolygon *line, t_vector *velocity);
void UnitVectorTowards(t_vector pos, t_vector dest, t_vector *unitvector, float *dist);
void UnitVectorAway(t_vector pos, t_vector dest, t_vector *unitvector, float *dist);
float DistanceSquared(t_vector one, t_vector two);
float Distance(t_vector one, t_vector two);
bool LinePolygonIntersection(const tPolygon *line, const tPolygon *poly);
t_vector RotateVector(t_vector vect, float angle);
t_vector RotateVectorQuick(t_vector vect, float ca, float sa);
bool OnScreen(GameObject *object, int player);
bool OnScreenFade(FadeObject *object, int player);
bool TileOnScreen(t_vector *position, t_point *scrposition, int player);
int WhichTile(int x, int y);
bool TestPathIntersection(t_vector point1, t_vector point2, int width);
bool TestPlayerSighting(t_vector enemyposition, t_vector *playerposition, int player, int type);
float AngleToPoint(t_vector origin, t_vector target, t_vector heading);


// **** Extra helpful functions **** //
float magic_inv_sqrt(float number);
float magic_sqrt(float number);
bool IsPowerOfTwo(int n);
int WhichPowerOfTwo(int n);
int LowestPowerTwo(int in);
float randf();
float randlim(float min, float max);
Fsincos_t fsincos(float x);


// **** Object manipulation and creation routines **** //
void CreateEngineTrail(t_vector position, GraphicalObject *trail, float fade, float faderate, float scale, float scalerate, bool setscalecentre);
void CreateBullet(Weapon *weapon, t_vector position, t_vector velocity, float angle, bool player, float factor);
void CreateHunterBullet(Weapon *weapon, t_vector position, t_vector velocity, float angle, int player);
void Update();
void CreateChildExplosions(GameObject *i);
void CreateExplosion(t_vector position, int size, int type);
void CreateDebris(t_vector position, float speed, float fade, float rate, float scale, float scalerate, float spinspeed, GraphicalObject *gobject);
void CreateSpray(t_vector position, t_vector direction, float speed, float fade, float rate, float scale, float scalerate, float spinspeed, GraphicalObject *gobject);
void CreateEnginePulse(t_vector position, t_vector unitvelocity, float speed, float fade, float rate, float scale, float scalerate, float spinspeed, GraphicalObject *gobject);
void CreateMarker(t_vector position);
void CreateRedMarker(t_vector position);
void CreateMissile(t_vector position, t_vector velocity, float angle, bool homing, int player);
void CreateSmokeTrail(t_vector position);
void CreateSparkle(t_vector position);
void CreateBulletCircle(t_vector position, int number, Weapon *weapon, bool player);
void CreateDrone(int player);
void CreateLightningDrone(int player);
void CreateShieldDrone(int player, int count);
void CreateGems(void);
void ResetTimers(void);
std::list<GameObject>::iterator GetIterator(GameObject* object);
void Spawn(GameObject * parent, list<GameObject>::iterator newobject);
void CreateSmokeCloud(t_vector position);


// **** Collision detection routines **** //
bool CollisionTest(GameObject* object1, GameObject* object2);
void DoCollision(GameObject* object1, GameObject* object2);
void BroadPhase();
void CollisionDetect();
bool DealWithCollision(Player *object, t_vector oldposition, float oldangle);
bool BoundaryCollisionDetect(GameObject *object);
float FindClosestLine(tPolygon *poutline, tPolygon *output, t_vector position, int gridnumber, int *poly, t_vector *point);
bool PointBoundariesCollision(t_vector point);
bool LineBoundariesIntersection(tPolygon *line);
bool LineBoundariesCollision(tPolygon line);
bool CheckTarget(t_vector position);
void GridPosition(GameObject *object);
bool BoundaryCollisionRoutine(GameObject *object, int count);
bool GridCollision(GameObject *object, int gridnumber);
float FindEarliestFreePosition(GameObject *object, float fraction);
float GridPointFindClosestLine(tPolygon *output, t_vector *position, int gridnumber);
t_vector DoCollisionBehaviour(GameObject *object);
bool NewBoundaryCollisionRoutine(GameObject *object, int count, float fraction);
void IntegrateObject(GameObject *object, float fraction);
bool PlayerBoundaryCollision(int player);
void KillShip(int player);


// **** Sound functions **** //
bool Sound_Initialize(HWND hWnd);
void Sound_Exit();
HRESULT LoadDirectSound(LPDIRECTSOUNDBUFFER *buffer,char* filename);
void PlaySounds();
void LoadSounds();


// **** Font routines **** //
int DrawString(GraphicalObject *usefont, const char *TextString, int x, int y, int limit, UINT colour);
int DrawNumber(GraphicalObject *usefont, int number, int x, int y, int alpha);


// **** Base routines **** //


// **** Menu Routines **** //
bool Menu();
bool SelectPlay(int players, bool newgame);
int MainMenu();
int SoundMenu();
int GraphicsMenu();
int ControlMenu();
int OptionsMenu();
int Credits();
int Story();
int LoadGameMenu();
int ScreenModeMenu();
int ExitMenu();


