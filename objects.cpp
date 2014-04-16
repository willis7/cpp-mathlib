#include "OR.h"

extern DX9VARS dx9;
extern Level level;
extern Player ship[2];
extern LPDIRECT3DTEXTURE9 background, radar;
extern ofstream dbout;
extern int g_screeny, g_screenx;
extern t_Tiledata *Tile;
extern int playmode;
extern GraphicalObject debris, debris2, smoke, missile, flame, redblob, redbullet, explosion, whiteblob, drone, shielddrone, flare1, flare2, spark;
extern list<GameObject> entity;
extern list<FadeObject> entityfade;
extern int saveticks, lasttick, thistick, ticks, numberofweapons;
extern float frametime;
extern SoundEffect soundeffect[NUMEFFECTS];
extern bool boundarychanged;
extern Weapon *weaponlist;
list<GameObject>::iterator whereobject;

void CreateEngineTrail(t_vector position, GraphicalObject *trail, float fade, float faderate, float scale, float scalerate, bool setscalecentre)
{
	entityfade.push_back(0);
	entityfade.back().type = FADE;
	SET(entityfade.back().state,POINTDETECTION);
	entityfade.back().graphics = trail;
	entityfade.back().position = position;
	entityfade.back().angle = TWOPI*(float)rand()/(float)RAND_MAX;
	entityfade.back().changeangle = 0.0f;
	entityfade.back().cosangle = 1.0f;
	entityfade.back().sinangle = 0.0f;
	entityfade.back().fade = fade;
	entityfade.back().faderate = faderate;
	entityfade.back().scale = scale;
	entityfade.back().scalerate = scalerate;
	entityfade.back().scalecentre = setscalecentre;
	entityfade.back().frame = 0;
	entityfade.back().forwardsspeed = 0.0f;
	entityfade.back().turnspeed = 0.0f;
}

void CreateSmokeTrail(t_vector position)
{
	entityfade.push_back(0);
	entityfade.back().type = FADE;
	SET(entityfade.back().state,POINTDETECTION);
	entityfade.back().graphics = &smoke;
	entityfade.back().position = position;
	entityfade.back().angle = TWOPI*(float)rand()/(float)RAND_MAX;
	entityfade.back().changeangle = 0.0f;

	Fsincos_t sincos = fsincos(entityfade.back().angle);
	entityfade.back().cosangle = sincos.cosine;
	entityfade.back().sinangle = sincos.sine;
	//entityfade.back().cosangle = cos(entityfade.back().angle);
	//entityfade.back().sinangle = sin(entityfade.back().angle);
	entityfade.back().fade = 0.5f;
	entityfade.back().faderate = 0.7f;
	entityfade.back().scale = 0.9f;
	entityfade.back().scalerate = 3.2f;
	entityfade.back().scalecentre = false;
	entityfade.back().frame = 0;
	entityfade.back().forwardsspeed = 0.0f;
	entityfade.back().turnspeed = 0.0f;
}

void CreateExplosion(t_vector position, int size, int type)
{	int explosiontype;
	float debrisspeed, redspeed, shockspeed;
	int debriscount, redcount, shockcount, sparkcount;

	entityfade.push_back(0);
	strcpy(entityfade.back().name,"Explosion");
	entityfade.back().type = EXPLOSION;
	entityfade.back().graphics = &explosion;
	entityfade.back().position = position;
	entityfade.back().scale = 1.2f;
	entityfade.back().scalerate = 1.0f;
	entityfade.back().fade = 0;

	switch(size)
	{
	case 5:
		entityfade.back().alpha = 0xffffffff;
		entityfade.back().frame = 0;
		debrisspeed = 160.0f;
		redspeed = 175.0f;
		shockspeed = 400.0f;
		debriscount = 4;
		redcount = 140;
		shockcount = 120;
		sparkcount = 80;
		break;
	case 4:
		entityfade.back().alpha = 0xffffffff;
		entityfade.back().frame = 0;
		debrisspeed = 160.0f;
		redspeed = 175.0f;
		shockspeed = 400.0f;
		debriscount = 3;
		redcount = 90;
		shockcount = 70;
		sparkcount = 60;
		break;
	case 3:
		entityfade.back().alpha = 0xffffffff;
		entityfade.back().frame = 1;
		debrisspeed = 150.0f;
		redspeed = 175.0f;
		shockspeed = 400.0f;
		debriscount = 2;
		redcount = 60;
		shockcount = 45;
		sparkcount = 40;
		break;
	case 2:
		entityfade.back().alpha = 0xffffffff;
		entityfade.back().frame = 2;
		debrisspeed = 140.0f;
		redspeed = 175.0f;
		shockspeed = 400.0f;
		debriscount = 1;
		redcount = 30;
		shockcount = 30;
		sparkcount = 15;
		break;
	case 1:
		entityfade.back().alpha = 0xffffffff;
		entityfade.back().frame = 3;
		debrisspeed = 130.0f;
		redspeed = 175.0f;
		shockspeed = 400.0f;
		debriscount = 0;
		redcount = 15;
		shockcount = 30;
		sparkcount = 5;
		break;
	case 0:
		entityfade.back().alpha = 0xffffffff;
		entityfade.back().frame = 4;
		debrisspeed = 120.0f;
		redspeed = 150.0f;
		shockspeed = 300.0f;
		debriscount = 1;
		redcount = 10;
		shockcount = 30;
		sparkcount = 0;
		break;
	}
	entityfade.back().forwardsspeed = 0;
	entityfade.back().turnspeed = 0;
	entityfade.back().angle = (TWOPI*rand())/(float)RAND_MAX;

	Fsincos_t sincos = fsincos(entityfade.back().angle);
	entityfade.back().cosangle = sincos.cosine;
	entityfade.back().sinangle = sincos.sine;
	//entityfade.back().cosangle = cos(entityfade.back().angle);
	//entityfade.back().sinangle = sin(entityfade.back().angle);
	entityfade.back().lastfired = thistick;

	CreateSmokeCloud(position);

	for(int i=0;i<redcount;i++) CreateDebris(position, redspeed, 0.6f, 1.1f, 1.0f, 0.0f, 0.0f, &redbullet);
	for(int i=0;i<redcount;i++) CreateDebris(position, debrisspeed*(0.5f + (float)rand()/(float)RAND_MAX), 0.60f, 1.10f, 1.00f, 0.0f, 0.0f, &redbullet);
	for(int i=0;i<debriscount;i++)
	{
		CreateDebris(position, debrisspeed*(0.5f + (float)rand()/(float)RAND_MAX), 0.8f, 0.5f, 1.0f, 0.0f, (10.0f*rand())/(float)RAND_MAX, &debris);
		CreateDebris(position, debrisspeed*(0.5f + (float)rand()/(float)RAND_MAX), 0.8f, 0.5f, 1.0f, 0.0f, (10.0f*rand())/(float)RAND_MAX, &debris2);
	}
	for(int i=0;i<shockcount;i++)
		CreateDebris(position, shockspeed, 0.15f, 0.25f, 1.0f, 0.5f, 0.0f, &whiteblob);
	for(int i=0;i<sparkcount;i++)
		CreateDebris(position,randlim(0,500.0f),1.0f,randlim(1.0f,3.0f),1.0f,0,0,&spark);


	switch(type)
	{
	case NONE:
		explosiontype = -1;
	case ENEMY:
		explosiontype = ENEMYEXPLODE;	break;
	case MINE:
		explosiontype = MINEEXPLODE;	break;
	case MISSILE:
		explosiontype = MISSILEEXPLODE;	break;
	default:
		explosiontype = ENEMYEXPLODE;	break;
	}

	//CreateSmokeTrail(position);

	if(explosiontype>=0)
	{
		soundeffect[explosiontype].SetSoundEffect(Distance(position,ship[0].position));
		if(playmode==TWOPLAYER) soundeffect[explosiontype].SetSoundEffect(Distance(position,ship[1].position));
	}
}

void CreateSparkle(t_vector position)
{
	for(int i=0;i<20;i++) CreateDebris(position, 240.0f*((float)rand()/(float)RAND_MAX), 1.0f, 1.2f, 0.8f, -0.4f, 0, &flare1);		//Blue
	for(int i=0;i<20;i++) CreateDebris(position, 275.0f*((float)rand()/(float)RAND_MAX), 1.0f, 1.2f, 0.8f, -0.4f, 0, &flare2);		//Red/orange
}

void CreateGems()
{
	list<GameObject>::iterator it = whereobject;
	int gemcount = 0, energypergem = 0;

	if(it->maxenergy>=20.0f) gemcount = 1;
	if(it->maxenergy>=100.0f) gemcount = 2;
	if(it->maxenergy>=200.0f) gemcount = 3;
	if(it->maxenergy>=500.0f) gemcount = 4;

	if(gemcount==0) return;

	energypergem = (int)(it->maxenergy/((float)gemcount*20.0f));
	t_vector position = it->position;

	for(int i=0;i<gemcount;i++)
	{
		it = entity.insert(it,GEMSTONE);
		CopyObject(&entity.front(),&*it);

		it->link = energypergem;
		it->position.x = position.x + 30.0f*(1.0f - 2.0f*(float)rand()/(float)RAND_MAX);
		it->position.y = position.y + 30.0f*((float)rand()/(float)RAND_MAX);
		it->angle = TWOPI*(float)rand()/(float)RAND_MAX;


		Fsincos_t sincos = fsincos(it->angle);
		it->cosangle = sincos.cosine;
		it->sinangle = sincos.sine;
		//it->cosangle = cos(it->angle);
		//it->sinangle = sin(it->angle);

		it->changeangle = 0;
		it->fade = 1.0f;
		it->faderate = 0;
		it->scale = 1.0f;
		it->scalerate = 0;
		it->scalecentre = true;
		it->frame = 0;
		it->turnspeed = 1.0f;
		it->radiussquared = 16.0f;
		it->radius = 4.0f;
		it->rootradius = 2.0f;
		++it;
	}

}


void CreateBullet(Weapon *weapon, t_vector position, t_vector velocity, float angle, bool player, float factor)
{
	if(weapon==NULL) return;

	CreateEngineTrail(position,weapon->pulsegraphics,1.0f,15.0f,1.0f,0,true);
	CreateEngineTrail(position,weapon->pulsegraphics,1.0f,15.0f,1.0f,0,true);

	entity.push_back(0);
	strcpy(entity.back().name,weapon->name.c_str());
	entity.back().energy = weapon->energy*factor;
	entity.back().maxenergy = entity.back().energy*factor;
	entity.back().weapon = weapon;
	entity.back().type = BULLET;
	SET(entity.back().state,POINTDETECTION);
	if(weapon->bounce) entity.back().behaviouroncollision = BOUNCE;
	else entity.back().behaviouroncollision = ABSORB;

	if(weapon->explode) SET(entity.back().state,CREATEEXPLOSION);

	entity.back().graphics = weapon->graphics;
	entity.back().position = position;

	entity.back().angle = angle;

	if(player)
	{
		weapon->sound.SetSoundEffect(0);
	}
	else
	{
		SET(entity.back().state,ENEMYBULLET);
		weapon->sound.SetSoundEffect(Distance(position,ship[0].position));
		if(playmode==TWOPLAYER) weapon->sound.SetSoundEffect(Distance(position,ship[1].position));
	}
	if(entity.back().angle>=TWOPI) entity.back().angle -= TWOPI;
	if(entity.back().angle<0) entity.back().angle += TWOPI;


	Fsincos_t sincos = fsincos(angle);
	entity.back().cosangle = sincos.cosine;
	entity.back().sinangle = -sincos.sine;
	//entity.back().cosangle = cos(angle);
	//entity.back().sinangle = -sin(angle);
	entity.back().changeangle = 0;
	entity.back().fade = weapon->fadevalue;
	entity.back().faderate = weapon->faderate;
	entity.back().scale = weapon->bulletscale;
	entity.back().scalerate = weapon->bulletgrowrate;
	entity.back().scalecentre = weapon->scalecentre;
	entity.back().frame = 0;
	entity.back().forwardsspeed = weapon->speed;
	entity.back().turnspeed = 0.0f;
	entity.back().outline.SetPoints(4);
	entity.back().rotatedoutline.SetPoints(4);
	entity.back().predictedoutline.SetPoints(4);
	entity.back().outline.vertex[0].x = -2.0f;
	entity.back().outline.vertex[0].y = -2.0f;
	entity.back().outline.vertex[1].x = 2.0f;
	entity.back().outline.vertex[1].y = -2.0f;
	entity.back().outline.vertex[2].x = 2.0f;
	entity.back().outline.vertex[2].y = 2.0f;
	entity.back().outline.vertex[3].x = -2.0f;
	entity.back().outline.vertex[3].y = 2.0f;
	entity.back().radiussquared = 16.0f;
	entity.back().radius = 4.0f;
	entity.back().rootradius = 2.0f;
	entity.back().radarcolour = 0;
	Normalise(&velocity);
	entity.back().unitvectorforward = velocity;
	std::list<GameObject>::iterator it = entity.end();
	GridPosition(&*(--it));
	level.grid[it->gridnumber].AddToGrid(&*it);
}


void CreateHunterBullet(Weapon *weapon, t_vector position, t_vector velocity, float angle, int player)
{
	if(weapon==NULL) return;

	entity.push_back(0);
	strcpy(entity.back().name,weapon->name.c_str());
	entity.back().energy = weapon->energy;
	entity.back().maxenergy = entity.back().energy;
	entity.back().weapon = weapon;
	entity.back().type = BULLET;
	SET(entity.back().state,POINTDETECTION);
	SET(entity.back().state,HUNTERBULLET);
	entity.back().behaviouroncollision = BOUNCE;
	entity.back().graphics = weapon->graphics;
	entity.back().position = position;
	entity.back().angle = angle;
	if(player==0) SET(entity.back().state,PLAYER1INSIGHT);
	else SET(entity.back().state,PLAYER2INSIGHT);

	weapon->sound.SetSoundEffect(0);

	if(entity.back().angle>=TWOPI) entity.back().angle -= TWOPI;
	if(entity.back().angle<0) entity.back().angle += TWOPI;

	Fsincos_t sincos = fsincos(angle);
	entity.back().cosangle = sincos.cosine;
	entity.back().sinangle = -sincos.sine;
	//entity.back().cosangle = cos(angle);
	//entity.back().sinangle = -sin(angle);
	entity.back().changeangle = 0;
	entity.back().fade = weapon->fadevalue;
	entity.back().faderate = weapon->faderate;
	entity.back().scale = weapon->bulletscale;
	entity.back().scalerate = weapon->bulletgrowrate;
	entity.back().scalecentre = weapon->scalecentre;
	entity.back().frame = 0;
	entity.back().forwardsspeed = weapon->speed;
	entity.back().turnspeed = 10.0f;
	entity.back().outline.SetPoints(4);
	entity.back().rotatedoutline.SetPoints(4);
	entity.back().predictedoutline.SetPoints(4);
	entity.back().outline.vertex[0].x = -2.0f;
	entity.back().outline.vertex[0].y = -2.0f;
	entity.back().outline.vertex[1].x = 2.0f;
	entity.back().outline.vertex[1].y = -2.0f;
	entity.back().outline.vertex[2].x = 2.0f;
	entity.back().outline.vertex[2].y = 2.0f;
	entity.back().outline.vertex[3].x = -2.0f;
	entity.back().outline.vertex[3].y = 2.0f;
	entity.back().radiussquared = 16.0f;
	entity.back().radius = 4.0f;
	entity.back().rootradius = 2.0f;
	entity.back().radarcolour = 0;
	Normalise(&velocity);
	entity.back().unitvectorforward = velocity;
	std::list<GameObject>::iterator it = entity.end();
	GridPosition(&*(--it));
	level.grid[it->gridnumber].AddToGrid(&*it);
}

void CreateSpray(t_vector position, t_vector direction, float speed, float fade, float rate, float scale, float scalerate, float spinspeed, GraphicalObject *gobject)
{
	t_vector velocity;
	velocity.x = direction.x + direction.y*randlim(-0.8f,0.8f);
	velocity.y = direction.y + direction.x*randlim(-0.8f,0.8f);
	Normalise(&velocity);
	CreateEnginePulse(position,velocity,speed,fade,rate,scale,scalerate,spinspeed,gobject);
}

void CreateDebris(t_vector position, float speed, float fade, float rate, float scale, float scalerate, float spinspeed, GraphicalObject *gobject)
{
	entityfade.push_back(0);
	entityfade.back().type = FADE;
	SET(entityfade.back().state,POINTDETECTION);
	entityfade.back().graphics = gobject;
	entityfade.back().position = position;
	entityfade.back().angle = TWOPI*(float)rand()/(float)RAND_MAX;
	entityfade.back().changeangle = 0.0f;

	Fsincos_t sincos = fsincos(entityfade.back().angle);
	entityfade.back().cosangle = sincos.cosine;
	entityfade.back().sinangle = sincos.sine;
	//entityfade.back().cosangle = cos(entityfade.back().angle);
	//entityfade.back().sinangle = sin(entityfade.back().angle);

	entityfade.back().fade = fade;
	entityfade.back().faderate = rate;
	entityfade.back().scale = scale;
	entityfade.back().scalerate = scalerate;
	entityfade.back().scalecentre = true;
	entityfade.back().frame = 0;
	entityfade.back().forwardsspeed = speed;
	entityfade.back().turnspeed = spinspeed;
	entityfade.back().velocity.x = -1.0f + 2.0f*(float)rand()/(float)RAND_MAX;
	entityfade.back().velocity.y = -1.0f + 2.0f*(float)rand()/(float)RAND_MAX;
	entityfade.back().unitvectorforward = entityfade.back().velocity;
	Normalise(&(entityfade.back().unitvectorforward));
}

void CreateEnginePulse(t_vector position, t_vector unitvelocity, float speed, float fade, float rate, float scale, float scalerate, float spinspeed, GraphicalObject *gobject)
{
	entityfade.push_back(0);
	entityfade.back().type = FADE;
	entityfade.back().graphics = gobject;
	entityfade.back().position = position;
	entityfade.back().angle = TWOPI*(float)rand()/(float)RAND_MAX;
	entityfade.back().changeangle = 0.0f;

	Fsincos_t sincos = fsincos(entityfade.back().angle);
	entityfade.back().cosangle = sincos.cosine;
	entityfade.back().sinangle = sincos.sine;
	//entityfade.back().cosangle = cos(entityfade.back().angle);
	//entityfade.back().sinangle = sin(entityfade.back().angle);

	entityfade.back().fade = fade;
	entityfade.back().faderate = rate;
	entityfade.back().scale = scale;
	entityfade.back().scalerate = scalerate;
	entityfade.back().scalecentre = true;
	entityfade.back().frame = 0;
	entityfade.back().turnspeed = spinspeed;
	entityfade.back().velocity.x = unitvelocity.x*speed;
	entityfade.back().velocity.y = unitvelocity.y*speed;
	entityfade.back().unitvectorforward = unitvelocity;
	entityfade.back().forwardsspeed = speed;
}

void Update()
{
	list<GameObject>::iterator i;
	list<FadeObject>::iterator j;

	ship[0].explosions.Update();
	if(playmode==TWOPLAYER) ship[1].explosions.Update();

	for(i=++entity.begin();i!=entity.end();)
	{	//GAME_OBJ_TYPE {INERT, FADE, EXPLOSION,										//Completely non-interacting types
		//BULLET, MISSILE, SHIELDDRONE, CONVERTER, ENEMY, MINE, ROCK, SPAWN, TURRET, SPINNER, TRIGGERSWITCH,	//Enemy collidable types
		//TELEPORT, BASE, SHIELDCHARGER, EXITGATE, DRONE, LIGHTNINGDRONE, SPAWNINERT, GAMESAVER, MISSILECRATE, BLUEPRINT, GEMSTONE, ENERGY, PLAYER, BUTTONSWITCH};	//Player interaction only types

		if(i->type == INERT || (i->type > TRIGGERSWITCH && i->type!=DRONE && i->type!=LIGHTNINGDRONE))
		{
			++i;
			continue;
		}
		if(CHECK(i->state,FORDESTRUCTION))
		{
			if(CHECK(i->state,CREATEEXPLOSION))
			{
				int size;
				switch(i->type)
				{
				case ENEMY:
					size = 5; break;
				case MINE:
					size = 4; break;
				case BULLET:
					size = 3; break;
				case TURRET:
					size = 5; break;
				case MISSILE:
					size = 4; break;
				case ROCK:
					size = 5; break;
				default:
					size = 3;
				}
				if(i->onscreen[0])
				{
					ship[0].light += 0.5f;
					if(ship[0].light>2.0f) ship[0].light = 2.0f;
					ship[0].explosions.AddExplosion(i->position.x,i->position.y);
				}
				if(playmode==TWOPLAYER && i->onscreen[1])
				{
					ship[1].light += 0.5f;
					if(ship[1].light>2.0f) ship[1].light = 2.0f;
					ship[1].explosions.AddExplosion(i->position.x,i->position.y);
				}
				CreateExplosion(i->position,size,i->type);
				CreateChildExplosions(&*i);
			}
			if(CHECK(i->state,CREATESPARKLE)) CreateSparkle(i->position);
			if(CHECK(i->state,CREATEPUFF)) CreateSmokeTrail(i->position);
			if(CHECK(i->state,CREATEBULLETS)) CreateBulletCircle(i->position,i->targetnode,i->weapon,CHECK(i->state,PLAYERBULLETS)?true:false);
			if(CHECK(i->state,LINKEDTOBOUNDARY))
			{
				TOGGLE(level.boundary[i->link].state,ON);
				boundarychanged = true;
			}
			if(i->objective)
			{
				--level.objectives;
				if(level.objectives==0)
				{
					ship[0].screenmessage.AddMessage("Objectives completed");
					if(playmode==TWOPLAYER) ship[1].screenmessage.AddMessage("Objectives completed");
					soundeffect[SP_SECTOR_CLEARED].SetSoundEffect(0);
				}
			}
			if(i->type==DRONE) ship[i->link].equipment[LASER_DRONE] = false;
			if(i->type==LIGHTNINGDRONE) ship[i->link].equipment[LIGHTNING_DRONE] = false;
			if(i->type==SHIELDDRONE) ship[i->link].equipment[SHIELD_DRONE] = false;
			level.grid[i->gridnumber].RemoveFromGrid(&*i);
			if(i->type==ROCK || i->type==ENEMY || i->type==TURRET || i->type == SPAWN)
			{
				whereobject = i;
				if(i->maxenergy<10000.0) CreateGems();
			}
			i = entity.erase(i);
			continue;
		}
		++i;
	}

	//Loop for the fade objects only
	for(j=entityfade.begin();j!=entityfade.end();)
	{
		if(CHECK(j->state,FORDESTRUCTION))
		{
			j = entityfade.erase(j);
			continue;
		}
		++j;
	}
}

void CreateChildExplosions(GameObject *i)
{
	for(int j=0;j<i->childobjects;j++)
	{	if(i->childobject[j].type==TURRET) CreateExplosion(i->childobject[j].position,1,NONE);
		CreateChildExplosions(&i->childobject[j]);
	}
}

void CreateMarker(t_vector position)
{
	entityfade.push_back(0);
	entityfade.back().type = FADE;
	SET(entityfade.back().state,POINTDETECTION);
	entityfade.back().graphics = &redblob;
	entityfade.back().position = position;
	entityfade.back().angle = 0.0f;
	entityfade.back().fade = 1.0f;
	entityfade.back().faderate = 0.0f;
	entityfade.back().scale = 1.0f;
	entityfade.back().scalerate = 0.0f;
	entityfade.back().frame = 0;
	entityfade.back().forwardsspeed = 0.0f;
	entityfade.back().turnspeed = 0.0f;
	strcpy(entityfade.back().name,"MARKER");
}
void CreateRedMarker(t_vector position)
{
	entityfade.push_back(0);
	entityfade.back().type = FADE;
	SET(entityfade.back().state,POINTDETECTION);
	entityfade.back().graphics = &redbullet;
	entityfade.back().position = position;
	entityfade.back().angle = 0.0f;
	entityfade.back().fade = 1.0f;
	entityfade.back().faderate = 0.0f;
	entityfade.back().scale = 1.0f;
	entityfade.back().scalerate = 0.0f;
	entityfade.back().frame = 0;
	entityfade.back().forwardsspeed = 0.0f;
	entityfade.back().turnspeed = 0.0f;
	strcpy(entityfade.back().name,"MARKER");
}

void CreateMissile(t_vector position, t_vector velocity, float angle, bool homing, int player)
{
	entity.push_back(0);
	strcpy(entity.back().name,"Missile");
	entity.back().energy = 100.0f;
	entity.back().type = MISSILE;
	entity.back().behaviouroncollision = ABSORB;
	entity.back().graphics = &missile;
	entity.back().position = position;
	entity.back().angle = angle;
	if(entity.back().angle>=TWOPI) entity.back().angle -= TWOPI;
	if(entity.back().angle<0) entity.back().angle += TWOPI;
	entity.back().fade = 1.0f;
	entity.back().faderate = 0.0f;
	entity.back().scale = 1.0f;
	entity.back().scalerate = 0.0f;
	entity.back().frame = 0;
	entity.back().radarcolour = 0;
	entity.back().forwardsspeed = 550.0f;
	entity.back().turnspeed = 0.2f;
	entity.back().outline.SetPoints(4);
	entity.back().rotatedoutline.SetPoints(4);
	entity.back().predictedoutline.SetPoints(4);
	entity.back().outline.vertex[0].x = -1.0f;
	entity.back().outline.vertex[0].y = -2.0f;
	entity.back().outline.vertex[1].x = 1.0f;
	entity.back().outline.vertex[1].y = -2.0f;
	entity.back().outline.vertex[2].x = 1.0f;
	entity.back().outline.vertex[2].y = 2.0f;
	entity.back().outline.vertex[3].x = -1.0f;
	entity.back().outline.vertex[3].y = 2.0f;
	entity.back().radiussquared = 4.0f;
	entity.back().radius = 2.0f;
	entity.back().rootradius = 1.41f;
	entity.back().timer = thistick;
	Normalise(&velocity);
	entity.back().unitvectorforward = velocity;
	entity.back().link = player;
/*	if(ship[player].equipment[MISSILEBOMBS])
	{
		SET(entity.back().state,CREATEBULLETS);
		SET(entity.back().state,PLAYERBULLETS);
		entity.back().targetnode = 20;
		entity.back().weapon = &weaponlist[GetWeaponNumber("Bounce_Blaster")];
	}*/
	if(homing)
	{
		SET(entity.back().state,HOMINGMISSILE);
	}
	std::list<GameObject>::iterator it = entity.end();
	GridPosition(&*(--it));
	level.grid[entity.back().gridnumber].AddToGrid(&*it);
	soundeffect[MISSILEFIRE].SetSoundEffect(0);
}

void CreateBulletCircle(t_vector position, int number, Weapon *weapon, bool player)
{
	float angle;
	t_vector velocity;

	if(weapon==NULL) return;
	for(int i=0;i<number;i++)
	{
		velocity.x = -1.0f + 2.0f*(float)rand()/(float)RAND_MAX;
		velocity.y = -1.0f + 2.0f*(float)rand()/(float)RAND_MAX;
		angle = atan(velocity.y/velocity.x) + PIBYTWO;
		CreateBullet(weapon, position, velocity, angle, player, 1.0f);
	}
}


void CreateDrone(int player)
{
	entity.push_back(0);
	strcpy(entity.back().name,"Drone");
	entity.back().energy = 1000.0f;
	entity.back().type = DRONE;
	entity.back().behaviouroncollision = ABSORB;
	entity.back().graphics = &drone;
	entity.back().position.x = ship[player].position.x + 50.0f;
	entity.back().position.y = ship[player].position.y;
	entity.back().angle = 0.0f;
	entity.back().fade = 1.0f;
	entity.back().faderate = 0.0f;
	entity.back().scale = 1.0f;
	entity.back().scalerate = 0.0f;
	entity.back().frame = 0;
	entity.back().forwardsspeed = ship[player].forwardsspeed*0.85;
	entity.back().turnspeed = 4.0f;
	entity.back().outline.SetPoints(4);
	entity.back().rotatedoutline.SetPoints(4);
	entity.back().predictedoutline.SetPoints(4);
	entity.back().outline.vertex[0].x = -10.0f;
	entity.back().outline.vertex[0].y = -16.0f;
	entity.back().outline.vertex[1].x = 10.0f;
	entity.back().outline.vertex[1].y = -16.0f;
	entity.back().outline.vertex[2].x = 10.0f;
	entity.back().outline.vertex[2].y = 16.0f;
	entity.back().outline.vertex[3].x = -10.0f;
	entity.back().outline.vertex[3].y = 16.0f;
	entity.back().radiussquared = 256.0f;
	entity.back().radius = 16.0f;
	entity.back().rootradius = 4.0f;
	entity.back().timer = thistick;
	entity.back().unitvectorforward.x = 1.0f;
	entity.back().unitvectorforward.y = 0.0f;
	entity.back().link = player;
	entity.back().lastfired = thistick;
	entity.back().weapon = &weaponlist[GetWeaponNumber("Propagator")];
	entity.back().weaponmounts = 1;
	entity.back().weaponmount[0].x = 0.0f;
	entity.back().weaponmount[0].y = 0.0f;
	entity.back().weaponmount[0].angle = 0.0f;
	entity.back().sensorrange = 400.0f;
	entity.back().attackrange = 200.0f;
	entity.back().radarcolour = 0xa8ffffff;
	ship[player].drone = &entity.back();
	ship[player].droneangle = 0.0f;
}

void CreateLightningDrone(int player)
{
	entity.push_back(0);
	strcpy(entity.back().name,"Lightning_Drone");
	entity.back().energy = 1000.0f;
	entity.back().type = LIGHTNINGDRONE;
	entity.back().behaviouroncollision = ABSORB;
	entity.back().graphics = &drone;
	entity.back().position.x = ship[player].position.x - 60.0f;
	entity.back().position.y = ship[player].position.y;
	entity.back().angle = 0.0f;
	entity.back().fade = 1.0f;
	entity.back().faderate = 0.0f;
	entity.back().scale = 1.0f;
	entity.back().scalerate = 0.0f;
	entity.back().frame = 0;
	entity.back().forwardsspeed = ship[player].forwardsspeed*0.85;
	entity.back().turnspeed = 4.0f;
	entity.back().outline.SetPoints(4);
	entity.back().rotatedoutline.SetPoints(4);
	entity.back().predictedoutline.SetPoints(4);
	entity.back().outline.vertex[0].x = -10.0f;
	entity.back().outline.vertex[0].y = -16.0f;
	entity.back().outline.vertex[1].x = 10.0f;
	entity.back().outline.vertex[1].y = -16.0f;
	entity.back().outline.vertex[2].x = 10.0f;
	entity.back().outline.vertex[2].y = 16.0f;
	entity.back().outline.vertex[3].x = -10.0f;
	entity.back().outline.vertex[3].y = 16.0f;
	entity.back().radiussquared = 256.0f;
	entity.back().radius = 16.0f;
	entity.back().rootradius = 4.0f;
	entity.back().timer = thistick;
	entity.back().unitvectorforward.x = 1.0f;
	entity.back().unitvectorforward.y = 0.0f;
	entity.back().link = player;
	entity.back().lastfired = thistick;
	entity.back().weapon = &weaponlist[GetWeaponNumber("Ion_Blaster")];
	entity.back().weaponmounts = 1;
	entity.back().weaponmount[0].x = 0.0f;
	entity.back().weaponmount[0].y = 0.0f;
	entity.back().weaponmount[0].angle = 0.0f;
	entity.back().sensorrange = 400.0f;
	entity.back().attackrange = 200.0f;
	entity.back().radarcolour = 0xa8ffffff;
	ship[player].lightningdrone = &entity.back();
	ship[player].droneangle = 0.0f;
}

void CreateShieldDrone(int player, int count)
{
	float ca, sa;
	entity.push_back(0);
	strcpy(entity.back().name,"Deflector Drone");
	entity.back().energy = 1.0e20f;
	entity.back().type = SHIELDDRONE;
	entity.back().behaviouroncollision = ABSORB;
	entity.back().graphics = &shielddrone;
	entity.back().angle = 0.0f;
	entity.back().fade = 1.0f;
	entity.back().faderate = 0.0f;
	entity.back().scale = 1.0f;
	entity.back().scalerate = 0.0f;
	entity.back().frame = 0;
	entity.back().forwardsspeed = ship[player].forwardsspeed*0.85f;
	entity.back().turnspeed = 0;
	entity.back().outline.SetPoints(4);
	entity.back().rotatedoutline.SetPoints(4);
	entity.back().predictedoutline.SetPoints(4);
	entity.back().outline.vertex[0].x = -15.0f;
	entity.back().outline.vertex[0].y = -15.0f;
	entity.back().outline.vertex[1].x = 15.0f;
	entity.back().outline.vertex[1].y = -15.0f;
	entity.back().outline.vertex[2].x = 15.0f;
	entity.back().outline.vertex[2].y = 15.0f;
	entity.back().outline.vertex[3].x = -15.0f;
	entity.back().outline.vertex[3].y = 15.0f;
	entity.back().radiussquared = 400.0f;
	entity.back().radius = 20.0f;
	entity.back().rootradius = sqrt(20.0f);
	entity.back().timer = thistick;
	entity.back().unitvectorforward.x = 1.0f;
	entity.back().unitvectorforward.y = 0.0f;
	entity.back().link = player;
	entity.back().lastfired = thistick;
	entity.back().weapon = &weaponlist[GetWeaponNumber("Ripper")];
	entity.back().weaponmounts = 0;
	entity.back().weaponmount[0].x = 0.0f;
	entity.back().weaponmount[0].y = 0.0f;
	entity.back().weaponmount[0].angle = 0.0f;
	entity.back().sensorrange = 400.0f;
	entity.back().attackrange = 200.0f;
	entity.back().radarcolour = 0;
	entity.back().targetnode = count;
	ship[player].shielddrone[count] = &entity.back();
	ship[player].deflectordroneangle[count] = (float)count*TWOPI*0.333333f;
	ca = cos(ship[player].deflectordroneangle[count]);
	sa = sin(ship[player].deflectordroneangle[count]);
	entity.back().position.x = ship[player].position.x + (ca*40.0f - sa*40.0f);
	entity.back().position.y = ship[player].position.y + (sa*40.0f + ca*40.0f);
	GridPosition(ship[player].shielddrone[count]);
	level.grid[entity.back().gridnumber].AddToGrid(ship[player].shielddrone[count]);
	//dbout<<"Adding shield drone to grid "<<entity.back().gridnumber<<endl;
}

void ResetTimers()
{
	list<GameObject>::iterator i;

	for(i=++entity.begin();i!=entity.end();++i)
	{
		if(i->type == MISSILE)
		{
			i->timer += (thistick - saveticks);
		}
		if(i->type == MINE || i->type == TRIGGERSWITCH || i->type == SPAWN || i->type == SPAWNINERT)
		{
			i->lastfired += (thistick - saveticks);
		}
	}
}


std::list<GameObject>::iterator GetIterator(GameObject* object)
{
	std::list<GameObject>::iterator i;
	for(i=++entity.begin();i!=entity.end();++i)
	{
		if(&*i == object) return i;
	}
}


void Spawn(GameObject * parent, list<GameObject>::iterator newobject)
{
	parent->lastfired = thistick;
	newobject = entity.insert(newobject,0);
	CopyObject(&parent->childobject[parent->childobjects],&*newobject);
	newobject->graphics = parent->childobject[parent->childobjects].graphics;
	newobject->position = parent->position;
	newobject->position.x += newobject->initialposition.x;
	newobject->position.y += newobject->initialposition.y;
	newobject->angle = parent->angle;
	newobject->path.ClearPath();
	CreateSparkle(newobject->position);
	soundeffect[SPAWNSOUND].SetSoundEffect(Distance(newobject->position,ship[0].position));
	if(playmode==TWOPLAYER) soundeffect[SPAWNSOUND].SetSoundEffect(Distance(newobject->position,ship[1].position));
	GridPosition(&*newobject);
	level.grid[newobject->gridnumber].AddToGrid(&*newobject);
	return;
}

void CreateSmokeCloud(t_vector position)
{
	t_vector nposition;
	for(int i=0;i<5;++i)
	{
		nposition.x = position.x + 10.0f*randf() - 5.0f;
		nposition.y = position.y + 10.0f*randf() - 5.0f;
		//			position	speed					fade				faderate			scale				scalerate		spin	graphics	
		CreateDebris(nposition, randlim(10.0f,50.0f), randlim(0.2f,0.7f), randlim(0.3f,1.5f), randlim(1.0f,3.0f), randlim(1.5f,3.0f), 0, &smoke);
	}
}