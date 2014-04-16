/* Navigation/movement of all objects and AI*/

#include "OR.h"

extern DX9VARS dx9;
extern Level level;
extern Player ship[2];
extern ofstream dbout;
extern int g_screeny, g_screenx;
extern list<GameObject> entity; 
extern list<FadeObject> entityfade;
extern int playmode;
extern GraphicalObject flame, redblob, explosion, teleportblob, exitgateblob, spark;
extern int lasttick, thistick, ticks, numberofweapons;
extern float frametime, framespersec;
extern Weapon *weaponlist;
extern SoundEffect soundeffect[NUMEFFECTS];
extern Equipment equipdata[NUMEQUIPTYPES];
extern bool boundarychanged;
extern Options options;


extern std::list<GameObject>::iterator parentiterator;

#ifdef DEBUG
ofstream listout;
#endif

void MoveShip(int player)
{
	bool moved = false;
	t_vector velocity;
	float angle = 0, boost = 1.0;

	if(ship[player].state!=PLAY)
	{
		soundeffect[CHARGEUP].stop = true;
		return;
	}

	velocity.x = 0;
	velocity.y = 0;

	//Set speeds for this frame
	AccelerateShip(ship[player].keys.UP, &ship[player].currentforwardsspeed, ship[player].forwardsspeed);
	AccelerateShip(ship[player].keys.DOWN, &ship[player].currentbackwardsspeed, ship[player].backwardsspeed);
	AccelerateShip(ship[player].keys.LEFT, &ship[player].currentleftturnspeed, ship[player].turnspeed);
	AccelerateShip(ship[player].keys.RIGHT, &ship[player].currentrightturnspeed, ship[player].turnspeed);

	if(ship[player].keys.UP)
	{
		if(ship[player].equipment[BOOSTER]) boost = 1.2f;
		if(ship[player].equipment[AUXILIARY_GENERATOR]) boost = 0.8f;
		velocity.x = ship[player].keys.up*boost*frametime*ship[player].currentforwardsspeed*ship[player].unitvectorforward.x;
		velocity.y = ship[player].keys.up*boost*frametime*ship[player].currentforwardsspeed*ship[player].unitvectorforward.y;
		moved = true;
	}
	if(ship[player].keys.DOWN)
	{
		if(ship[player].equipment[BOOSTER]) boost = 1.1f;
		if(ship[player].equipment[AUXILIARY_GENERATOR]) boost = 0.8f;
		velocity.x = -ship[player].keys.down*boost*frametime*ship[player].currentbackwardsspeed*ship[player].unitvectorforward.x;
		velocity.y = -ship[player].keys.down*boost*frametime*ship[player].currentbackwardsspeed*ship[player].unitvectorforward.y;
		moved = true;
	}
	if(ship[player].keys.LEFT)
	{
		if(ship[player].equipment[BOOSTER]) boost = 1.2f;
		if(ship[player].equipment[AUXILIARY_GENERATOR]) boost = 0.8f;
		angle = ship[player].keys.left*boost*frametime*ship[player].currentleftturnspeed;
		moved = true;
	}
	if(ship[player].keys.RIGHT)
	{
		if(ship[player].equipment[BOOSTER]) boost = 1.2f;
		if(ship[player].equipment[AUXILIARY_GENERATOR]) boost = 0.8f;
		angle = -ship[player].keys.right*boost*frametime*ship[player].currentrightturnspeed;
		moved = true;
	}
	if(ship[player].equipment[SIDE_BOOST])
	{
		if(ship[player].equipment[BOOSTER]) boost = 1.2f;
		if(ship[player].equipment[AUXILIARY_GENERATOR]) boost = 0.8f;
		if(ship[player].keys.SHIFTLEFT)
		{
			ship[player].keys.shiftleft -= frametime;
			if(ship[player].keys.shiftleft<0) ship[player].keys.shiftleft = 0;
			velocity.x += ship[player].keys.shiftleft*boost*frametime*ship[player].backwardsspeed*ship[player].unitvectorforward.y;
			velocity.y -= ship[player].keys.shiftleft*boost*frametime*ship[player].backwardsspeed*ship[player].unitvectorforward.x;
			moved = true;
		}
		if(ship[player].keys.SHIFTRIGHT)
		{
			ship[player].keys.shiftright -= frametime;
			if(ship[player].keys.shiftright<0) ship[player].keys.shiftright = 0;
			velocity.x -= ship[player].keys.shiftright*boost*frametime*ship[player].backwardsspeed*ship[player].unitvectorforward.y;
			velocity.y += ship[player].keys.shiftright*boost*frametime*ship[player].backwardsspeed*ship[player].unitvectorforward.x;
			moved = true;
		}
	}

	if(moved)
	{
		GameObject playerobject;
		playerobject.outline = ship[player].outline;
		playerobject.rotatedoutline = ship[player].rotatedoutline;
		playerobject.predictedoutline = ship[player].predictedoutline;
		playerobject.position = ship[player].position;
		playerobject.angle = -ship[player].angle;

		Fsincos_t sincos = fsincos(playerobject.angle);
		playerobject.cosangle = sincos.cosine;
		playerobject.sinangle = -sincos.sine;
		//playerobject.cosangle = cos(playerobject.angle);
		//playerobject.sinangle = -sin(playerobject.angle);

		playerobject.changeangle = -angle;
		playerobject.velocity = velocity;
		playerobject.type = ENEMY;
		playerobject.behaviouroncollision = SLIDE;

		if(NewBoundaryCollisionRoutine(&playerobject,0,1.0)) soundeffect[IMPACT].SetSoundEffect(0);

		ship[player].rotatedoutline	= playerobject.rotatedoutline;
		ship[player].predictedoutline = playerobject.predictedoutline;
		ship[player].position = playerobject.position;
		ship[player].angle = -playerobject.angle;
		ship[player].sinangle = playerobject.sinangle;
		ship[player].cosangle = playerobject.cosangle;
		ship[player].unitvectorforward.x = -ship[player].sinangle;
		ship[player].unitvectorforward.y = -ship[player].cosangle;
	}

	if(ship[player].equipment[HOMING_COMPUTER]) ship[player].homingcomputerbusy = HomingComputer(player);
	else ship[player].targetangle = 0;

	if(ship[player].keys.UP || ship[player].keys.DOWN)
	{
			for(int i=0;i<ship[player].engines;i++)
			{
				t_vector trailpos, unitvelocity;
				trailpos.x = ship[player].engine[i].x;		//Rotate the engine mount position
				trailpos.y = ship[player].engine[i].y;
				trailpos = RotateVector(trailpos,TWOPI-ship[player].angle);
				trailpos.x += ship[player].position.x;
				trailpos.y += ship[player].position.y;
				unitvelocity.x = -ship[player].unitvectorforward.x;
				unitvelocity.y = -ship[player].unitvectorforward.y;
				if(ship[player].energy/ship[player].maxenergy>0.3f)
				{
					//CreateEngineTrail(trailpos, &explosion, 0.6f, 1.8f, 0.2f, 1.0f, true);
					if(ship[player].keys.UP) CreateEnginePulse(trailpos, unitvelocity, 100.0f, 0.6f, 1.8f, ship[player].keys.up>0.1f?ship[player].keys.up*0.2f:0.2f, 0.5f + 0.75f*(1.5f - (float)rand()/(float)RAND_MAX), 0, &explosion);
					for(int j=0;j<3;++j) CreateSpray(trailpos, unitvelocity,40.0f,1.0f,0.5f,1.0f,0,0,&spark);
					//CreateDebris(trailpos, &explosion, 0.6f, 1.8f, 0.2f, 1.0f, true);
					if(soundeffect[ENERGYLOW].looping) soundeffect[ENERGYLOW].stop = true;
				}
				else
				{
					CreateSmokeTrail(trailpos);
					soundeffect[ENERGYLOW].SetSoundEffect(0);
				}
			}
	}

	if(ship[player].keys.SHIFTLEFT && ship[player].keys.shiftleft>0)
	{
		t_vector trailpos, unitvelocity;
		trailpos.x = ship[player].position.x;
		trailpos.y = ship[player].position.y;
		unitvelocity.x = -ship[player].unitvectorforward.y;
		unitvelocity.y = ship[player].unitvectorforward.x;
		for(int j=0;j<2;++j) CreateSpray(trailpos, unitvelocity,ship[player].keys.shiftleft*100.0f,0.6f,0.5f,1.0f,0,0,&spark);
	}

	if(ship[player].keys.SHIFTRIGHT && ship[player].keys.shiftright>0)
	{
		t_vector trailpos, unitvelocity;
		trailpos.x = ship[player].position.x;
		trailpos.y = ship[player].position.y;
		unitvelocity.x = ship[player].unitvectorforward.y;
		unitvelocity.y = -ship[player].unitvectorforward.x;
		for(int j=0;j<2;++j) CreateSpray(trailpos, unitvelocity,ship[player].keys.shiftright*100.0f,0.6f,0.5f,1.0f,0,0,&spark);
	}

	if(ship[player].equipment[BOMB])
	{
		if(ship[player].bombtimer!=0xffffffff)
		{
			float ratio = (thistick-ship[player].bombtimer)/1500.0f;
			if(ratio<0.1f) soundeffect[CHARGEUP].SetSoundEffect(0);
			if(thistick-ship[player].bombtimer>1500)
			{
				ship[player].keys.FIREBOMB = true;
				ship[player].bombtimer = 0xffffffff;
				soundeffect[CHARGEUP].stop = true;
			}
		}
		else soundeffect[CHARGEUP].stop = true;
	}

	if(ship[player].keys.FIRE)
	{
		//Find hyperseeker and lightning targets
		if(ship[player].equipment[SEEKER]) HunterSelectTarget(ship[player].position, &ship[player].seekertarget);

		if(ship[player].equipment[LIGHTNING_DRONE] && ship[player].lightningdrone!=NULL)
			LightningSelectTarget(ship[player].lightningdrone,&ship[player].lightningtree,0);

		//Do main weapons
		if(thistick-ship[player].lastfired>ship[player].weapon->firerate)
		{
			for(int i=0;i<ship[player].weaponmounts;i++)
			{
				float spreadangle = 0;
				t_vector bulletpos, bulletvel;
				bulletpos.x = ship[player].weaponmount[i].x;		//Rotate the weapon mount position
				bulletpos.y = ship[player].weaponmount[i].y;
				bulletpos = RotateVector(bulletpos,-ship[player].angle - ship[player].weaponmount[i].angle/* - spreadangle*/);
				bulletpos.x += ship[player].position.x;
				bulletpos.y += ship[player].position.y;

				if(ship[player].equipment[SPREADER])
				{
					for(int j=0;j<3;++j)
					{
						spreadangle = (float)(j-1)*0.15f;
						//dbout<<spreadangle<<" "<<ship[player].weaponmount[i].angle<<" "<<(-ship[player].weaponmount[i].angle - spreadangle)<<endl;
						bulletvel.x = ship[player].unitvectorforward.x;
						bulletvel.y = ship[player].unitvectorforward.y;
						bulletvel = RotateVector(bulletvel,-ship[player].weaponmount[i].angle - spreadangle);
						CreateBullet(ship[player].weapon,bulletpos,bulletvel,(-ship[player].angle-ship[player].weaponmount[i].angle-spreadangle), true, 0.4f);
					}
				}
				else
				{
					bulletvel.x = ship[player].unitvectorforward.x;
					bulletvel.y = ship[player].unitvectorforward.y;
					bulletvel = RotateVector(bulletvel,-ship[player].weaponmount[i].angle);
					CreateBullet(ship[player].weapon,bulletpos,bulletvel,(-ship[player].angle-ship[player].weaponmount[i].angle), true, 1.0f);
				}
				//for(int j=0;j<3;++j) CreateSpray(bulletpos,bulletvel,120.0f,1.0f,2.0f,1.0f,0,0,&spark);
			}
			ship[player].lastfired = thistick;
		}
		if(thistick-ship[player].lastminimissile>200 && ship[player].missiledroneangle<0.1f)
		{
			if(ship[player].equipment[MINIMISSILES])
			{
				t_vector bulletpos, bulletvel;
				bulletvel.x = -ship[player].unitvectorforward.x;
				bulletvel.y = -ship[player].unitvectorforward.y;
				bulletpos.x = 40.0f*cos(ship[player].missiledroneangle);		//Rotate the engine mount position
				bulletpos.y = 0;
				bulletpos = RotateVectorQuick(bulletpos,ship[player].cosangle,-ship[player].sinangle);
				bulletpos.x += ship[player].position.x;
				bulletpos.y += ship[player].position.y;
				CreateBullet(&weaponlist[GetWeaponNumber("Mini_Missile")],bulletpos,ship[player].unitvectorforward,-ship[player].angle, true, 1.0f);
				for(int i=0;i<6;++i) CreateSpray(bulletpos,bulletvel,randlim(30.0f,150.0f),1.0f,1.0f,1.0f,0,0,&spark);
				bulletpos.x = 40.0f*cos(ship[player].missiledroneangle+PI);		//Rotate the engine mount position
				bulletpos.y = 0;
				bulletpos = RotateVectorQuick(bulletpos,ship[player].cosangle,-ship[player].sinangle);
				bulletpos.x += ship[player].position.x;
				bulletpos.y += ship[player].position.y;
				CreateBullet(&weaponlist[GetWeaponNumber("Mini_Missile")],bulletpos,ship[player].unitvectorforward,-ship[player].angle, true, 1.0f);
				for(int i=0;i<6;++i) CreateSpray(bulletpos,bulletvel,randlim(30.0f,150.0f),1.0f,1.0f,1.0f,0,0,&spark);
			}
			ship[player].lastminimissile = thistick;
		}
		if(thistick-ship[player].lastseeker>700)
		{
			if(ship[player].equipment[SEEKER])
			{
				if(ship[player].seekertarget!=NULL)
				{
					t_vector bulletpos, bulletvel;
					bulletpos.x = 40;		//Rotate the engine mount position
					bulletpos.y = 0;
					bulletpos = RotateVector(bulletpos,-ship[player].angle - PIBYFOUR);
					bulletpos.x += ship[player].position.x;
					bulletpos.y += ship[player].position.y;
					bulletvel.x = ship[player].unitvectorforward.x;
					bulletvel.y = ship[player].unitvectorforward.y;
					bulletvel = RotateVector(bulletpos,-ship[player].angle - PIBYFOUR);
					CreateHunterBullet(&weaponlist[GetWeaponNumber("Seeker")],bulletpos,bulletvel,ship[player].angle, player);

					bulletpos.x = 40;		//Rotate the engine mount position
					bulletpos.y = 0;
					bulletpos = RotateVector(bulletpos,-ship[player].angle - PIBYFOUR - PIBYTWO);
					bulletpos.x += ship[player].position.x;
					bulletpos.y += ship[player].position.y;
					bulletvel.x = ship[player].unitvectorforward.x;
					bulletvel.y = ship[player].unitvectorforward.y;
					bulletvel = RotateVector(bulletpos,-ship[player].angle - PIBYFOUR - PIBYTWO);
					CreateHunterBullet(&weaponlist[GetWeaponNumber("Seeker")],bulletpos,bulletvel,ship[player].angle, player);

					bulletpos.x = 40;		//Rotate the engine mount position
					bulletpos.y = 0;
					bulletpos = RotateVector(bulletpos,-ship[player].angle - PIBYFOUR + PI);
					bulletpos.x += ship[player].position.x;
					bulletpos.y += ship[player].position.y;
					bulletvel.x = ship[player].unitvectorforward.x;
					bulletvel.y = ship[player].unitvectorforward.y;
					bulletvel = RotateVector(bulletpos,-ship[player].angle - PIBYFOUR + PI);
					CreateHunterBullet(&weaponlist[GetWeaponNumber("Seeker")],bulletpos,bulletvel,ship[player].angle, player);

					bulletpos.x = 40;		//Rotate the engine mount position
					bulletpos.y = 0;
					bulletpos = RotateVector(bulletpos,-ship[player].angle - PIBYFOUR + PIBYTWO);
					bulletpos.x += ship[player].position.x;
					bulletpos.y += ship[player].position.y;
					bulletvel.x = ship[player].unitvectorforward.x;
					bulletvel.y = ship[player].unitvectorforward.y;
					bulletvel = RotateVector(bulletpos,-ship[player].angle - PIBYFOUR + PIBYTWO);
					CreateHunterBullet(&weaponlist[GetWeaponNumber("Seeker")],bulletpos,bulletvel,ship[player].angle, player);
				}
			}
			ship[player].lastseeker = thistick;
		}
	}

	if(ship[player].keys.FIREMISSILE)
	{
		ship[player].keys.FIREMISSILE = false;
		if(ship[player].equipment[MISSILE_LAUNCHER])
		{
			if(ship[player].missiles>0)
			{
				if(thistick-ship[player].lastlaunched>1000)
				{
					t_vector forward = ship[player].unitvectorforward;
					forward = RotateVector(forward,-ship[player].targetangle);
					CreateMissile(ship[player].position, forward, - ship[player].targetangle - ship[player].angle, ship[player].equipment[HOMING_COMPUTER], player);
					ship[player].lastlaunched = thistick;
					--ship[player].missiles;
				}
			}
			else soundeffect[NOMISSILES].SetSoundEffect(0);
		}
	}
	if(ship[player].keys.FIREBOMB && ship[player].equipment[BOMB])
	{
		ship[player].keys.FIREBOMB = false;
		ship[player].equipment[BOMB] = false;
		ship[player].power -= equipdata[BOMB].power;
		CreateBulletCircle(ship[player].position, 100, &weaponlist[GetWeaponNumber("Fire_Ball")], true);
	}


	if(ship[player].equipment[SELF_REPAIR])
	{	if(ship[player].energy<ship[player].maxenergy)
		{	ship[player].energy += frametime*5.0f;
			if(ship[player].energy>ship[player].maxenergy) ship[player].energy = ship[player].maxenergy;
		}
	}

	float energyfrac = ship[player].energy/ship[player].maxenergy;
	if(energyfrac<0.15f)
	{
		if(thistick-ship[player].emergencytimer>7000)
		{
			char message[100];
			sprintf_s(message,100,"Emergency! Energy %d%%",(int)(energyfrac*100.0f));
			ship[player].screenmessage.AddMessage(message);
			soundeffect[SP_EMERGENCY].SetSoundEffect(0);
			ship[player].emergencytimer = thistick;
		}
	}
	else if(energyfrac<0.35f)
	{
		if(thistick-ship[player].energylowtimer>10000)
		{
			char message[100];
			sprintf_s(message,100,"Warning! Energy %d%%",(int)(energyfrac*100.0f));
			ship[player].screenmessage.AddMessage(message);
			soundeffect[SP_ENERGY_LOW].SetSoundEffect(0);
			ship[player].energylowtimer = thistick;
		}
	}

	for(int i=0;i<2;++i)
	{
		ship[player].missiledroneangle += PIBYTWO*frametime;
		if(ship[player].missiledroneangle>=PI) ship[player].missiledroneangle -= PI;
	}

	//Work out screen corner coordinates
	DoScreen(player);

	return;
}


void AccelerateShip(bool keydown, float *current, float speed)
{
	float speedframe = frametime*speed/0.15f;		//Find the amount of acceleration needed this frame

	if(keydown)
	{	//Need to accelerate
		if(*current<speed) *current += speedframe;		//Set current speed
		if(*current>speed) *current = speed;			//Limit speed to ship's speed
	}

	else	//No key pressed
	{	//Need to decelerate
		if(*current>0) *current -= speedframe;			//Set current speed
		if(*current<0) *current = 0;					//Limit speed to 0
	}
}

void MoveObjects()
{
	list<GameObject>::iterator i = entity.begin(), k;
	list<FadeObject>::iterator j;
	FadeObject *obj;
	t_vector position, d;
	float speed, ftime = frametime, sizelimit, fadelimit;
	int fadeys = entityfade.size();
	int lastgrid;

	if(fadeys>4000)
	{
		sizelimit = 0.25f;
		fadelimit = 0.35f;
	}
	else if(fadeys>2500)
	{
		sizelimit = 0.10f;
		fadelimit = 0.20f;
	}
	else
	{
		sizelimit = 0.05f;
		fadelimit = 0.05f;
	}

	//Quick fade objects loop
	for(j = entityfade.begin();j!=entityfade.end();++j)
	{
		obj = &*j;
		if(obj->type==FADE)
		{
			obj->fade -= obj->faderate*ftime;
			obj->scale += obj->scalerate*ftime;
			if(obj->fade<fadelimit || obj->scale<sizelimit)
			{
				SET(obj->state,FORDESTRUCTION);
				continue;
			}
			obj->alpha = ((lrintf(obj->fade * 255.0f))<<24) + 0xffffff;
			speed = obj->forwardsspeed*ftime;
			obj->velocity.x = obj->unitvectorforward.x*speed;
			obj->velocity.y = obj->unitvectorforward.y*speed;
			obj->position.x += obj->velocity.x;
			obj->position.y += obj->velocity.y;
			if(obj->turnspeed!=0)
			{
				obj->angle += obj->turnspeed*ftime;

				Fsincos_t sincos = fsincos(obj->angle);
				obj->cosangle = sincos.cosine;
				obj->sinangle = sincos.sine;
				//obj->cosangle = cos(obj->angle);
				//obj->sinangle = sin(obj->angle);

				if(obj->angle>TWOPI) obj->angle -= TWOPI;
				else if(obj->angle<0) obj->angle += TWOPI;
			}
		}
		else		//Explosion
		{
			obj->scale += obj->scalerate*ftime;
			if(thistick-obj->lastfired>60)
			{
				obj->lastfired = thistick;
				++(obj->frame);
				if(obj->frame==obj->graphics->frames)
				{
					obj->frame = obj->graphics->frames - 1;
					SET(obj->state,FORDESTRUCTION);
				}
			}
		}

		obj->onscreen[0] = OnScreenFade(obj, 0);
		if(playmode==TWOPLAYER) obj->onscreen[1] = OnScreenFade(obj, 1);
	}

	/**************** Normal objects loop ************/
//Any non-solid object types must have a number lower than EXPLOSION
//Any non-enemy-interacting object types must have a number higher than BASE
//enum GAME_OBJ_TYPE {INERT, FADE, EXPLOSION,										//Completely non-interacting types
//					BULLET, MISSILE, SHIELDDRONE, CONVERTER, ENEMY, MINE, ROCK, SPAWN, TURRET, SPINNER, TRIGGERSWITCH,	//Enemy collidable types
//					TELEPORT, BASE, SHIELDCHARGER, EXITGATE, DRONE, SPAWNINERT, GAMESAVER, MISSILECRATE, GEMSTONE, ENERGY, BUTTONSWITCH};	//Player interaction only types

	CheckHunterTarget(&ship[0]);
	if(playmode==TWOPLAYER) CheckHunterTarget(&ship[1]);

  #ifdef DEBUG
	listout.open("data\\objectlog.txt",ios::out);
  #endif

	i = entity.begin();
	for(++i;i!=entity.end();)
	{
		parentiterator = i;
		switch(i->type)
		{
		case INERT:				//Non-moving object
			i->changeangle = i->turnspeed*ftime;
			if(i->changeangle!=0)
			{
				i->angle += i->changeangle;
				if(i->childobjects)
				{
					Fsincos_t sincos = fsincos(i->angle);
					i->cosangle = sincos.cosine;
					i->sinangle = -sincos.sine;
					//i->cosangle = cos(i->angle);
					//i->sinangle = -sin(i->angle);
				}
			}
			break;
		case BULLET:
			WrapToLevel(&i->position);
			lastgrid = i->gridnumber;
			i->fade -= i->faderate*ftime;
			i->scale += i->scalerate*ftime;
			if(i->fade<0.01 || i->scale<0.01)
			{
				SET(i->state,FORDESTRUCTION);
				++i;
				continue;
			}
			i->alpha = (((int)(i->fade * 255))<<24) + 0xffffff;
			if(i->weapon->trail)
				if(i->isonscreen)
					CreateEngineTrail(i->position,i->weapon->trailgfx,i->fade,i->weapon->trailfaderate,i->scale,i->weapon->trailgrowrate,i->weapon->scalecentre);
			i->energy = i->maxenergy*i->fade;
			if(CHECK(i->state,HUNTERBULLET)) MoveHunterBullet(&*i);
			i->velocity.x = i->unitvectorforward.x*ftime*i->forwardsspeed;
			i->velocity.y = i->unitvectorforward.y*ftime*i->forwardsspeed;
			i->changeangle = 0;
			if(NewBoundaryCollisionRoutine(&*i, 0, 1.0f))
			{
				for(int n=0;n<5;++n) CreateDebris(i->position,100.0f,i->fade,3.0f,1.0f,0,0,&spark);
				soundeffect[BULLETIMPACT].SetSoundEffect(Distance(i->position,ship[0].position));
				if(playmode==TWOPLAYER) soundeffect[BULLETIMPACT].SetSoundEffect(Distance(i->position,ship[1].position));
				if(i->behaviouroncollision!=BOUNCE)
				{
					SET(i->state,FORDESTRUCTION);
					SET(i->state,CREATEPUFF);
				}
			}
			if(lastgrid!=i->gridnumber)
			{
				level.grid[lastgrid].RemoveFromGrid(&*i);
				level.grid[i->gridnumber].AddToGrid(&*i);
			}
			break;
		case MISSILE:			//Moving object
			i->velocity.x = i->unitvectorforward.x*ftime*i->forwardsspeed;
			i->velocity.y = i->unitvectorforward.y*ftime*i->forwardsspeed;
			i->changeangle = 0;
			lastgrid = i->gridnumber;
			if((thistick - i->timer > 1000) ||  NewBoundaryCollisionRoutine(&*i, 0, 1.0f))
			{
				SET(i->state,FORDESTRUCTION);
				SET(i->state,CREATEEXPLOSION);
			}
			if(lastgrid!=i->gridnumber)
			{
				level.grid[lastgrid].RemoveFromGrid(&*i);
				level.grid[i->gridnumber].AddToGrid(&*i);
			}
			position.y = 25.0;		//For the smoke trail
			position.x = 0;
			position = RotateVector(position,i->angle);
			position.x += i->position.x;
			position.y += i->position.y;
			CreateSmokeTrail(position);
			CreateSmokeTrail(position);
			{
				t_vector trailposition = i->position;
				t_vector traildirection;
				traildirection.x = -i->unitvectorforward.x;
				traildirection.y = -i->unitvectorforward.y;
				CreateSpray(trailposition,traildirection,120.0f,1.0f,2.0f,1.0f,0,0,&spark);
			}
			break;
		case SHIELDDRONE:
			i->changeangle = 0;
			lastgrid = i->gridnumber;
			MoveShieldDrone(&*i);
			SpinObject(&*i);
			GridPosition(&*i);
			if(lastgrid!=i->gridnumber)
			{
				level.grid[lastgrid].RemoveFromGrid(&*i);
				level.grid[i->gridnumber].AddToGrid(&*i);
			}
			break;
		case ENEMY:
			#ifdef DEBUG
			listout<<i->name<<endl;
			#endif
			i->changeangle = 0;
			lastgrid = i->gridnumber;
			MoveEnemy(&*i);
			if(NewBoundaryCollisionRoutine(&*i, 0, 1.0)) SET(i->state,STUCKONWALL);
			if(lastgrid!=i->gridnumber)
			{
				level.grid[lastgrid].RemoveFromGrid(&*i);
				level.grid[i->gridnumber].AddToGrid(&*i);
			}
			break;
		case MINE:			//Non-moving object
			if(CHECK(i->state,ARMED))
			{
				if(thistick-i->lastfired>i->timer)
				{
					SET(i->state,FORDESTRUCTION);
					SET(i->state,CREATEEXPLOSION);
					SET(i->state,CREATEBULLETS);
				}
			}
			else CheckMine(&*i);
			i->velocity.x = 0;
			i->velocity.y = 0;
			i->changeangle = 0;
			break;
		case ROCK:
			{
				i->velocity.x = i->unitvectorforward.x*ftime*i->forwardsspeed;
				i->velocity.y = i->unitvectorforward.y*ftime*i->forwardsspeed;
				i->changeangle = i->turnspeed*ftime;
				float oldangle = i->angle;
				lastgrid = i->gridnumber;
				//BoundaryCollisionRoutine(&*i, 0);
				BoundaryCollisionRoutine(&*i, 0);
				i->changeangle = i->angle - oldangle;
			}
			if(lastgrid!=i->gridnumber)
			{
				level.grid[lastgrid].RemoveFromGrid(&*i);
				level.grid[i->gridnumber].AddToGrid(&*i);
			}
			break;
		case SPAWN:
			if(thistick - i->lastfired>i->timer) Spawn(&*i,i);
			{
				t_vector spawnpos;
				spawnpos.x = i->childobject[i->childobjects].initialposition.x;
				spawnpos.y = i->childobject[i->childobjects].initialposition.y;
				spawnpos = RotateVectorQuick(spawnpos,i->cosangle,i->sinangle);
				spawnpos.x += i->position.x;
				spawnpos.y += i->position.y;
				if(i->isonscreen)
				{
					CreateEngineTrail(spawnpos, &teleportblob, 0.6f, 1.2f, 0.2f, 1.2f, false);
					t_vector blobvelocity, blobposition;
					blobposition.x = randlim(-1.0f,1.0f);
					blobposition.y = randlim(-1.0f,1.0f);
					Normalise(&blobposition);
					blobposition.x = 20.0f*blobposition.x + i->position.x;
					blobposition.y = 20.0f*blobposition.y + i->position.y;
					blobvelocity.x = randlim(-1.0f,1.0f);
					blobvelocity.y = randlim(-1.0f,1.0f);
					Normalise(&blobvelocity);
					CreateEnginePulse(blobposition, blobvelocity, 10.0f, 1.0f, 0.5f, 1.0f, 0, 0, &spark);
				}
			}
			i->velocity.x = i->unitvectorforward.x*ftime*i->forwardsspeed;
			i->velocity.y = i->unitvectorforward.y*ftime*i->forwardsspeed;
			i->changeangle = i->turnspeed*ftime;
			lastgrid = i->gridnumber;
			BoundaryCollisionRoutine(&*i, 0);
			//NewBoundaryCollisionRoutine(&*i, 0, 1.0f);
			if(lastgrid!=i->gridnumber)
			{
				level.grid[lastgrid].RemoveFromGrid(&*i);
				level.grid[i->gridnumber].AddToGrid(&*i);
			}
			break;
		case TURRET:			//Non-moving object
			i->velocity.x = 0;
			i->velocity.y = 0;
			i->changeangle = 0;
			MoveTurret(&*i);
			break;
		case TRIGGERSWITCH:		//Non-moving object
			if(thistick-i->lastfired>i->timer) SET(i->state,ARMED);
			break;
		case TELEPORT:		//Non-moving object
			k = i;
			if(i->link>0) for(int m=0;m<i->link;m++)		//Find which object it's linked to
			{	++k;
				if(k==entity.end())
				{	k = i;
					break;
				}
			}
			else if(i->link<0) for(int m=i->link;m<0;m++)
			{	--k;
				if(k==entity.end())
				{	k = i;
					break;
				}
			}
			i->target = k->position;
			if(CheckTarget(i->target))
			{
				UNSET(i->state,LOCKED);
				if(i->isonscreen)
				{
					t_vector blobvelocity, blobposition;
					blobposition.x = randlim(-1.0f,1.0f);
					blobposition.y = randlim(-1.0f,1.0f);
					Normalise(&blobposition);
					blobvelocity.x = -blobposition.x;
					blobvelocity.y = -blobposition.y;
					blobposition.x = 40.0f*blobposition.x + i->position.x;
					blobposition.y = 40.0f*blobposition.y + i->position.y;
					CreateEnginePulse(blobposition, blobvelocity, 90.0f, 0.5f, 0.7f, 0.35f, -0.5f, 0, &teleportblob);
					blobvelocity.x = randlim(-1.0f,1.0f);
					blobvelocity.y = randlim(-1.0f,1.0f);
					Normalise(&blobvelocity);
					CreateEnginePulse(blobposition, blobvelocity, 10.0f, 1.0f, 0.5f, 1.0f, 0, 0, &spark);
				}
				//if(i->onscreen[0] || i->onscreen[1]) CreateEngineTrail(i->position, &teleportblob, 0.6f, 1.2f, 0.2f, 1.2f, false);
			}
			else SET(i->state,LOCKED);
			i->velocity.x = 0;
			i->velocity.y = 0;
			i->changeangle = i->turnspeed*ftime;
			SpinObject(&*i);
			break;
		case EXITGATE:		//Non-moving object
			if(level.objectives>0)
			{
				SET(i->state,LOCKED);
				i->radarcolour = 0;
			}
			else
			{
				UNSET(i->state,LOCKED);
				i->radarcolour = 0xffddffdd;
				if(i->isonscreen)
				{
					t_vector blobvelocity, blobposition;
					blobposition.x = randlim(-1.0f,1.0f);
					blobposition.y = randlim(-1.0f,1.0f);
					Normalise(&blobposition);
					blobvelocity.x = -blobposition.x;
					blobvelocity.y = -blobposition.y;
					blobposition.x = 50.0f*blobposition.x + i->position.x;
					blobposition.y = 50.0f*blobposition.y + i->position.y;
					CreateEnginePulse(blobposition, blobvelocity, 100.0f, 0.5f, 0.5f, 0.65f, -0.9f, 0, &exitgateblob);
					blobvelocity.x = randlim(-1.0f,1.0f);
					blobvelocity.y = randlim(-1.0f,1.0f);
					Normalise(&blobvelocity);
					CreateEnginePulse(blobposition, blobvelocity, 10.0f, 1.0f, 0.5f, 1.0f, 0, 0, &spark);
				}
			}
			i->velocity.x = 0;
			i->velocity.y = 0;
			i->changeangle = i->turnspeed*ftime;
			SpinObject(&*i);
			break;
		case DRONE:
			i->changeangle = 0;
			MoveDrone(&*i);
			break;
		case LIGHTNINGDRONE:
			i->changeangle = 0;
			MoveLightningDrone(&*i);
			break;
		case SPAWNINERT:		//Non-moving object
			if(thistick - i->lastfired>i->timer) Spawn(&*i,i);
			i->changeangle = i->turnspeed*ftime;
			if(i->changeangle!=0)
			{
				i->angle += i->changeangle;
				if(i->childobjects)
				{
					Fsincos_t sincos = fsincos(i->angle);
					i->cosangle = sincos.cosine;
					i->sinangle = sincos.sine;
					//i->cosangle = cos(i->angle);
					//i->sinangle = sin(i->angle);
				}
			}
			{
				t_vector spawnpos;
				spawnpos.x = i->childobject[i->childobjects].initialposition.x;
				spawnpos.y = i->childobject[i->childobjects].initialposition.y;
				spawnpos = RotateVectorQuick(spawnpos,i->cosangle,i->sinangle);
				spawnpos.x += i->position.x;
				spawnpos.y += i->position.y;
				if(i->isonscreen)
				{
					CreateEngineTrail(spawnpos, &teleportblob, 0.6f, 1.2f, 0.2f, 1.2f, false);
					t_vector blobvelocity, blobposition;
					blobposition.x = randlim(-1.0f,1.0f);
					blobposition.y = randlim(-1.0f,1.0f);
					Normalise(&blobposition);
					blobposition.x = 20.0f*blobposition.x + i->position.x;
					blobposition.y = 20.0f*blobposition.y + i->position.y;
					blobvelocity.x = randlim(-1.0f,1.0f);
					blobvelocity.y = randlim(-1.0f,1.0f);
					Normalise(&blobvelocity);
					CreateEnginePulse(blobposition, blobvelocity, 10.0f, 1.0f, 0.5f, 1.0f, 0, 0, &spark);
				}
			}
			break;
		case MISSILECRATE:			//Non-moving objects
		case GEMSTONE:
		case BLUEPRINT:
		case ENERGY:
			i->velocity.x = 0;
			i->velocity.y = 0;
			i->changeangle = i->turnspeed*ftime;
			SpinObject(&*i);
			break;
		case BUTTONSWITCH:		//Non-moving object
			if(i->childobjects>0)
			{
				if(CHECK(i->state,ON)) i->childobject[0].frame = 1;
				else  i->childobject[0].frame = 0;
			}
			i->changeangle = i->turnspeed*ftime;
			i->angle += i->changeangle;
			break;
		default:
			break;
		}
		/**/


		while(i->angle>TWOPI) i->angle -= TWOPI;
		while(i->angle<0) i->angle += TWOPI;

		i->onscreen[0] = OnScreen(&*i, 0);
		if(playmode==TWOPLAYER) i->onscreen[1] = OnScreen(&*i, 1);

		for(int j=0;j<i->childobjects;j++)
		{
			i->childobject[j].changeangle = i->changeangle;
			i->childobject[j].position = RotateVectorQuick(i->childobject[j].initialposition,i->cosangle,-i->sinangle);
			i->childobject[j].position.x += i->position.x;
			i->childobject[j].position.y += i->position.y;
			MoveChildObject(&i->childobject[j], i->angle);
		}
		++i;
	}
#ifdef DEBUG
	listout.close();
#endif
}

void MoveChildObject(GameObject *object, float parentangle)
{
	bool limited = false;
	object->onscreen[0] = OnScreen(object, 0);
	if(playmode==TWOPLAYER) object->onscreen[1] = OnScreen(object, 1);

	if(object->type==INERT || object->type==SPAWNINERT)
	{
		float oldangle = object->angle;
		object->changeangle += object->turnspeed*frametime;
		object->angle += object->changeangle;
		if(CHECK(object->state,OSCILLATE) && object->turnspeed!=0)
		{
			float relativeangle = object->angle - parentangle;
			if(relativeangle>PI) relativeangle -= TWOPI;
			else if(relativeangle<-PI) relativeangle += TWOPI;
			if(relativeangle > object->maxangle)
			{
				//limited = true;
				object->angle = parentangle + object->maxangle;
				object->changeangle = object->angle - oldangle;
				object->turnspeed = -object->turnspeed;
			}
			if(relativeangle < object->minangle)
			{
				//limited = true;
				object->angle = parentangle + object->minangle;
				object->changeangle = object->angle - oldangle;
				object->turnspeed = -object->turnspeed;
			}
		}
		if(object->timer)
		{
			if(object->type==SPAWNINERT)
			{
				t_vector spawnpos;
				spawnpos.x = object->childobject[object->childobjects].initialposition.x;
				spawnpos.y = object->childobject[object->childobjects].initialposition.y;
				spawnpos = RotateVectorQuick(spawnpos,object->cosangle,object->sinangle);
				spawnpos.x += object->position.x;
				spawnpos.y += object->position.y;
				if(thistick - object->lastfired>object->timer) Spawn(&*object,parentiterator);
				if(object->isonscreen)
				{
					CreateEngineTrail(spawnpos, &teleportblob, 0.6f, 1.2f, 0.2f, 1.2f, false);
					t_vector blobvelocity, blobposition;
					blobposition.x = randlim(-1.0f,1.0f);
					blobposition.y = randlim(-1.0f,1.0f);
					Normalise(&blobposition);
					blobposition.x = 20.0f*blobposition.x + object->position.x;
					blobposition.y = 20.0f*blobposition.y + object->position.y;
					blobvelocity.x = randlim(-1.0f,1.0f);
					blobvelocity.y = randlim(-1.0f,1.0f);
					Normalise(&blobvelocity);
					CreateEnginePulse(blobposition, blobvelocity, 10.0f, 1.0f, 0.5f, 1.0f, 0, 0, &spark);
				}
			}
			else if(thistick-object->lastfired>object->timer)
			{
				object->lastfired = thistick;
				if(++(object->frame)==object->frames) object->frame = 0;
			}
		}



	}
	else if(object->type==TURRET)
	{
		float oldangle = object->angle;
		MoveTurret(object);
		if(object->turnspeed!=0)
		{
			float relativeangle = object->angle - parentangle;
			if(relativeangle>PI) relativeangle -= TWOPI;
			else if(relativeangle<-PI) relativeangle += TWOPI;
			if(relativeangle > object->maxangle)
			{
				//limited = true;
				object->angle = parentangle + object->maxangle;
				object->changeangle = object->angle - oldangle;
				//if(strcmp("Rangefinder_Turret",object->name)==0) dbout<<object->name<<" old "<<oldangle<<" new "<<object->angle<<" changeangle "<<object->changeangle<<endl;
			}
			if(relativeangle < object->minangle)
			{
				//limited = true;
				object->angle = parentangle + object->minangle;
				object->changeangle = object->angle - oldangle;
				//if(strcmp("Rangefinder_Turret",object->name)==0) dbout<<object->name<<" old "<<oldangle<<" new "<<object->angle<<" changeangle "<<object->changeangle<<endl;
			}
		}
	}

	while(object->angle>=TWOPI) object->angle -= TWOPI;
	while(object->angle<0) object->angle += TWOPI;

	Fsincos_t sincos = fsincos(object->angle);
	object->cosangle = sincos.cosine;
	object->sinangle = sincos.sine;

	for(int j=0;j<object->childobjects;j++)
	{
		object->childobject[j].position = RotateVectorQuick(object->childobject[j].initialposition,object->cosangle,object->sinangle);
		object->childobject[j].position.x += object->position.x;
		object->childobject[j].position.y += object->position.y;
		object->childobject[j].changeangle = object->changeangle;
		MoveChildObject(&object->childobject[j], object->angle);
	}
}

//Will rotate to face or away from a target point; returns true if already facing it to within tolerance
bool RotateTo(GameObject *object, t_vector point, float tolerance, DIRECTION dn)
{
	float x = point.x - object->position.x;
	float y = point.y - object->position.y;
	float xl = -object->unitvectorforward.x;
	float yl = -object->unitvectorforward.y;
	float dist = magic_inv_sqrt(x*x+y*y);					//distance to selected point
	float arg = (-xl*x + -yl*y) * dist;
	float direction;

	if(arg>1)	//Sometimes, due to rounding error, this may come to be greater than one, causing a NaN
	{
		object->angletotarget = 0;
		direction = 0;
		if(dn == TOWARDS) return true;
	}
	else
	{
		object->angletotarget = acos(arg);
		direction = xl*y - yl*x;
	}
	float turnspeed = frametime*object->turnspeed;

	if(object->angletotarget!=object->angletotarget) return false;	//Sometimes goes wrong - not sure why
	if(object->angletotarget<tolerance) return true;

	if(dn == TOWARDS)
	{
		if(turnspeed<object->angletotarget)		//Angle to target must be reduced
		{
			if(direction>0)
			{
				object->changeangle -= turnspeed;
				if((object->angletotarget+object->changeangle)<tolerance) return true;
			}
			else if(direction<0)
			{
				object->changeangle += turnspeed;
				if((object->angletotarget-object->changeangle)<tolerance) return true;
			}
			else if((object->angletotarget-object->changeangle)<tolerance) return true;
			return false;
		}
		else		//In this case, turning by one whole unit will take us too far the other way:
		{
			if(direction>0)
			{
				object->changeangle -= object->angletotarget;
				return true;
			}
			else if(direction<0)
			{
				object->changeangle += object->angletotarget;
				return true;
			}
			else if((object->angletotarget-object->changeangle)<tolerance) return true;
			return false;
		}
	}
	if(dn == AWAY)
	{
		if(object->angletotarget<tolerance)		//Angle to target must be increased
		{
			if(direction<0)
			{
				object->changeangle -= turnspeed;
				if((object->angletotarget+object->changeangle)<tolerance) return true;
			}
			else
			{
				object->changeangle += turnspeed;
				if((object->angletotarget-object->changeangle)<tolerance) return true;
			}
			return false;
		}
	}
	return true;
}


/*
//Any non-solid object types must have a number lower than EXPLOSION
//Any non-enemy-interacting object types must have a number higher than BASE
enum GAME_OBJ_TYPE {INERT, FADE, EXPLOSION,										//Non-solid types
					BULLET, SHIELDDRONE, MISSILE, CONVERTER, ENEMY, MINE, ROCK, TURRET, SPINNER, TRIGGERSWITCH,	//Collidable types
					TELEPORT, BASE, SHIELDCHARGER, EXITGATE, DRONE, BLACKHOLEOBJ, GAMESAVER, MISSILECRATE, ENERGY, BUTTONSWITCH};	//Other types
*/

//Finds the actual point to steer towards, taking into account all the surrounding objects
t_vector CalculateTargetPoint(GameObject *object)
{
	long j, k;
	float length;
	t_vector vect, newvector;
	list<GameObject>::iterator i;
	GridSquare *square;

	newvector.x = object->target.x - object->position.x;	//Original target vector
	newvector.y = object->target.y - object->position.y;
	length = Normalise(&newvector);
	newvector.x *= 1000.0;
	newvector.y *= 1000.0;

	square = &level.grid[object->gridnumber];
	for(int i=0;i<square->collideobjects.size();++i)
	{
		if(square->collideobjects[i]->type<ENEMY || square->collideobjects[i]->type>TURRET) continue;
		if(RepulsionVector(object, &square->collideobjects[i]->position, 300.0, &vect, square->collideobjects[i]->rootradius))
		{
			newvector.x += vect.x;
			newvector.y += vect.y;
		}
	}

	//Loop over 8 adjacent grid squares
	for(int j=0; j<8; ++j)
	{
		square = level.grid[object->gridnumber].neighbour[j];
		for(int i=0;i<square->collideobjects.size();++i)
		{
			if(square->collideobjects[i]->type<ENEMY || square->collideobjects[i]->type>TURRET) continue;
			if(RepulsionVector(object, &square->collideobjects[i]->position, 300.0, &vect, square->collideobjects[i]->rootradius))
			{
				newvector.x += vect.x;
				newvector.y += vect.y;
			}
		}
	}

	if(CHECK(object->state,RETREATING))
	{
		if(RepulsionVector(object, &ship[0].position, 1000.0, &vect, 100.0))
		{
			newvector.x += vect.x;
			newvector.y += vect.y;
		}
		if(playmode==TWOPLAYER)
		{
			if(RepulsionVector(object, &ship[1].position, 1000.0, &vect, 100.0))
			{
				newvector.x += vect.x;
				newvector.y += vect.y;
			}
		}
	}


/*
	//Check boundaries
	int gridnumber = object->gridnumber;
	for(j=0;j<level.grid[gridnumber].boundaries;j++)	//Loop over boundaries in this grid
		if(level.boundary[level.grid[gridnumber].boundary[j]].state==ON)	//Check if they're ON
			for(k=0;k<level.boundary[level.grid[gridnumber].boundary[j]].numpoints;k++)		//Loop over points in this boundary
				if(RepulsionVector(object, &level.boundary[level.grid[gridnumber].boundary[j]].poly.vertex[k], 250.0, &vect, 4.0))
				{
					newvector.x += vect.x;
					newvector.y += vect.y;
				}
*/
	newvector.x += object->position.x;
	newvector.y += object->position.y;
	return newvector;
}

bool RepulsionVector(GameObject *object, t_vector *pos, float max, t_vector *vec, float sqrtradius)
{
	float distance, maxsq = max*max;
	t_vector repulse;

	repulse.x = object->position.x - pos->x;	//Vector from external entity to agent
	repulse.y = object->position.y - pos->y;
	distance = repulse.x*repulse.x + repulse.y*repulse.y;

	if(distance>maxsq)
	{	//Past the cutoff
		vec->x = 0;
		vec->y = 0;
		return false;
	}
	Normalise(&repulse);

	distance = 1.3f*sqrtradius*max*magic_inv_sqrt(distance);	//Find the distance

	//distance = max - distance;
	distance = distance*distance;
	if(distance>1000.0) distance = 1000.0;	//Never greater than the forwards vector

	vec->x = repulse.x*distance;		//Multiply
	vec->y = repulse.y*distance;
	return true;
}

void MoveTurret(GameObject *object)
{
	t_vector playerposition[2], newtarget;
	float pdistance[2] = {1.0e12f, 1.0e12f};

	SetPlayerStatusFlags(object, pdistance, playerposition);

	object->target = object->position;

	//Next find out whether we ought to go towards the player
	if(CHECK(object->state,PLAYER1INSIGHT) && CHECK(object->state,PLAYER2INSIGHT))
	{
		if(pdistance[0] < pdistance[1])	object->target = playerposition[0];
		else object->target = playerposition[1];
	}
	else if(CHECK(object->state,PLAYER1INSIGHT)) object->target = playerposition[0];
	else if(CHECK(object->state,PLAYER2INSIGHT)) object->target = playerposition[1];
	else if(object->changeangle!=0)		//No way we can shoot at the player
	{
		object->target = object->position;
		SpinObject(object);		//Apply rotation and quit
		return;
	}
	object->distancetotarget = DistanceSquared(object->position,object->target);

	RotateTo(object, object->target, 0.015f, TOWARDS);
	if(object->changeangle!=0) SpinObject(object);

	if(CHECK(object->state,PLAYER1INRANGE) || CHECK(object->state,PLAYER2INRANGE) || object->attackrange>object->sensorrange)
	{
		if( (object->angletotarget<0.25f || object->attackrange>object->sensorrange) && object->weapon!=NULL)
		{	//Shoot bullets!
			if(thistick - object->lastfired > object->weapon->firerate)
			{
				object->lastfired = thistick;
				for(int i=0;i<object->weaponmounts;i++)
				{
					t_vector bulletpos, bulletvel;
					bulletpos.x = object->weaponmount[i].x;		//Rotate the weapon mount position
					bulletpos.y = object->weaponmount[i].y;
					bulletpos = RotateVector(bulletpos,object->angle);
					bulletpos.x += object->position.x;
					bulletpos.y += object->position.y;
					bulletvel.x = object->unitvectorforward.x;
					bulletvel.y = object->unitvectorforward.y;
					if(object->weaponmount[i].angle!=0) bulletvel = RotateVector(bulletvel,object->weaponmount[i].angle);
					CreateBullet(object->weapon, bulletpos, bulletvel, object->angle+object->weaponmount[i].angle, false, 1.0f);
					//for(int j=0;j<5;++j) CreateSpray(bulletpos,bulletvel,120.0f,1.0f,2.0f,1.0f,0,0,&spark);
					//for(int j=0;j<5;++j) CreateDebris(bulletpos,120.0f,1.0f,2.0f,1.0f,0,0,&spark);
				}
			}
		}
	}
}

//Go towards the target
void TrackTarget(GameObject *object)
{
	t_vector temporarytarget = CalculateTargetPoint(object);
	//Within 5.0 degrees 0.087
	RotateTo(object, temporarytarget, 0.05f, TOWARDS);
	if(object->angletotarget<0.20f)
	{
		Fsincos_t sincos = fsincos(object->angle + object->changeangle);
		float ca = sincos.cosine;
		float sa = sincos.sine;
		//float sa = sin(object->angle + object->changeangle);
		//float ca = cos(object->angle + object->changeangle);
		t_vector uvf;
		uvf.x = sa;
		uvf.y = -ca;
		object->velocity.x = uvf.x*frametime*object->forwardsspeed;
		object->velocity.y = uvf.y*frametime*object->forwardsspeed;

		if(object->isonscreen)
		{
			if(object->energy/object->maxenergy<0.25)
			{
				for(int i=0;i<object->engines;i++)
				{
					t_vector smokeposition = RotateVectorQuick(object->engine[i],ca,sa);
					smokeposition.x += object->position.x;
					smokeposition.y += object->position.y;
					CreateSmokeTrail(smokeposition);
				}
			}
			else
			{
				for(int i=0;i<object->engines;i++)
				{
					t_vector trailposition = RotateVectorQuick(object->engine[i],ca,sa);
					t_vector traildirection;
					traildirection.x = -object->unitvectorforward.x;
					traildirection.y = -object->unitvectorforward.y;
					trailposition.x += object->position.x;
					trailposition.y += object->position.y;
					CreateEngineTrail(trailposition, &redblob, 0.5f, 1.2f, 1.0f, 1.2f, true);
					for(int j=0;j<3;++j) CreateSpray(trailposition,traildirection,120.0f,1.0f,2.0f,1.0f,0,0,&spark);
				}
			}
		}
	}
}

t_vector GetRandomTarget(GameObject *object)
{
	int counter = 0;
	t_vector target;
	float randomx, randomy;
	float range = magic_sqrt(object->rangelimit);

	do
	{
		randomx = (float)rand()/(float)RAND_MAX;
		randomy = (float)rand()/(float)RAND_MAX;
		if(CHECK(object->state,RANGELIMITED))
		{
			//Returns a point within a square of side 'range'
			target.x = range*(2.0f*randomx - 1.0f);
			target.y = range*(2.0f*randomy - 1.0f);
			if(target.x*target.x + target.y*target.y > object->rangelimit)		//Make sure it's within a circle
			{
				target.x *= 0.5f;
				target.y *= 0.5f;
			}
			target.x += object->initialposition.x;
			target.y += object->initialposition.y;
		}
		else
		{	//Returns a point within the level dimensions
			target.x = randomx*level.fwidth;
			target.y = randomy*level.fheight;
		}

		if(target.x<0) target.x += level.fwidth;
		else if(target.x>level.fwidth) target.x -= level.fwidth;
		if(target.y<0) target.y += level.fheight;
		else if(target.y>level.fheight) target.y -= level.fheight;
		counter++;
	} while(PointBoundariesCollision(target) && !CheckTarget(target) && counter<5 );		//Check for boundary collisions and other object collisions

	if(counter==5)	//No deal; the counter exceeded our imposed limit
	{
		target.x = 0;
		target.y = 0;
	}
	return target;
}

//Gets a target 50 units in the opposite direction to the target
t_vector GetInverseTarget(t_vector position, t_vector target)
{
	t_vector targetvector;
	targetvector.x =  position.x - target.x;
	targetvector.y =  position.y - target.y;
	Normalise(&targetvector);
	targetvector.x = targetvector.x*50.0f + position.x;
	targetvector.y = targetvector.y*50.0f + position.y;
	return targetvector;
}

void CalculateNavigationMesh()
{
	int count;
	std::vector<int> links;
	tPolygon line;
	line.SetPoints(2);
	links.reserve(1000);
	for(int i=0;i<level.meshpoints;i++)
	{
		count = 0;
		line.vertex[0] = level.mesh[i].position;
		for(int j=0;j<level.meshpoints;j++)
		{
			if(i==j) continue;
			line.vertex[1] = level.mesh[j].position;
			//Check every other point for line-of-sight
			if(LineBoundariesCollision(line)==false)
			{
				links.push_back(j);
				++count;				//Count how many meshpoints are accessible from this one
			}
		}
		level.mesh[i].SetLinks(count);	//Set number of accessible points and distances
		for(int j=0;j<count;j++)
		{
			level.mesh[i].link[j] = &level.mesh[links[j]];
			level.mesh[i].distance[j] = Distance(level.mesh[i].link[j]->position, level.mesh[i].position);
		}
		links.clear();		//Clear the links vector
	}
}


bool PathFind(int one, int two, int **path, int *numpoints)
{	//F is the total path cost
	//G is the distance travelled so far
	//H is the estimate of the distance to travel
	int current;
	bool onclosed, onopen;
	vector<int> openlist, closedlist, finalpath;
	openlist.reserve(500);		//Should be ample - no 500 meshpoint levels yet!
	closedlist.reserve(500);
	finalpath.reserve(500);


	//Add the starting node to the open list. 
	openlist.push_back(one);
	current = one;
	level.mesh[one].H = Distance(level.mesh[one].position,level.mesh[two].position);
	level.mesh[one].G = 0;
	level.mesh[one].F = 0;
	level.mesh[one].parent = one;		//It's its own parent

	//Repeat the following:
	while(openlist.size()!=0)
	{
		//Look for the lowest F cost node on the open list. We refer to this as the current node.
		current = 0;
		for(int i=1;i<openlist.size();i++)
			if(level.mesh[openlist[i]].F<level.mesh[current].F)	current = i;

		vector<int>::iterator element = openlist.begin();
		element += current;
		closedlist.push_back(openlist[current]);
		current = closedlist.back();
		openlist.erase(element);

		//Check whether this node is the target
		if(current==two)
		{	//Make path

			//Working backwards from the target node, go from each node to its parent node until you reach the starting node
			finalpath.push_back(current);

			while(current != level.mesh[current].parent)
			{
				finalpath.push_back(level.mesh[current].parent);
				current = level.mesh[current].parent;
			}
			*numpoints = finalpath.size();
			*path = new int[*numpoints];
			for(int i=0;i<*numpoints;i++) (*path)[i] = finalpath[i];
			return true;	//found a path
		}

		//For each of the nodes adjacent to this current node …
		for(int i=0;i<level.mesh[current].links;i++)
		{
			//Check if it's on the closed list - it's a node already in the path
			onclosed = false;
			for(int j=0;j<closedlist.size();j++)	//Check closed list
			{
				if(closedlist[j]==level.mesh[current].link[i]->number)
				{
					//Check to see whether the path to it is shorter through the current node or not
					//Is the distance to the one on the closed list longer than the path to the current node plsu the distance from current node to the one on the closed list
					if(level.mesh[closedlist[j]].G > level.mesh[current].G + level.mesh[current].distance[i])
					{	//Reset the parent of the one on the closed list to be the current node
						level.mesh[closedlist[j]].parent = current;
					}
					onclosed = true;
					break;
				}
			}
			if(onclosed) continue;	//Ignore nodes on the closed list

			//Check if it's on the open list
			onopen = false;
			for(int j=0;j<openlist.size();j++)
			{
				if(openlist[j]==level.mesh[current].link[i]->number)
				{
					onopen = true;
					break;
				}
			}

			if(!onopen)
			{
				//Calculate the heuristics etc
				level.mesh[current].link[i]->parent = current;
				level.mesh[current].link[i]->G = level.mesh[current].G + level.mesh[current].distance[i];		//Cost to reach node
				level.mesh[current].link[i]->H = Distance(level.mesh[current].position,level.mesh[current].link[i]->position);	//Heuristic
				level.mesh[current].link[i]->F = level.mesh[current].link[i]->G + level.mesh[current].link[i]->H;		//The F score

				//Add it to the open list
				openlist.push_back(level.mesh[current].link[i]->number);
			}

		}//End of links loop
	}//End of while loop

	//Fail to find the target node, and the open list is empty. In this case, there is no path.
	*path = NULL;
	*numpoints = 0;
	return false;	//No path possible
}


//Returns -1 if there isn't a valid node
int FindNearestNode(t_vector start)
{
	tPolygon line(2);
	float dist, mindist = 1.0e20f;
	int node = -1;
	line.vertex[0] = start;

	for(int i=0;i<level.meshpoints;i++)		//Find the nearest visible node to start and end positions
	{
		line.vertex[1] = level.mesh[i].position;
		if(!LineBoundariesCollision(line))
		{
			dist = Distance(start,level.mesh[i].position);
			if(dist<mindist)
			{
				mindist = dist;
				node = i;
			}
		}
	}
	return node;
}

void CheckMine(GameObject *object)
{
	t_vector playerdistance;
	int player = 0;
	float distance = 1.0e12f;

	if(ship[0].state==PLAY) distance = DistanceSquared(object->position,ship[0].position);
	if(ship[1].state==PLAY)
	{
		 float distance1 = DistanceSquared(object->position,ship[1].position);
		 if(distance1<distance)
		 {
			 distance = distance1;
			 player = 1;
		 }
	}

	if(distance<object->sensorrange*object->sensorrange)
	{
		tPolygon line(2);
		line.vertex[1] = ship[player].position;
		line.vertex[0] = object->position;
		if(!LineBoundariesCollision(line))	//Check for line-of-sight
		{
			SET(object->state,ARMED);
			object->lastfired = thistick;
		}
	}
	
}

t_vector FindNearestObject(t_vector position, float limit)
{
	list<GameObject>::iterator i = entity.begin();
	float distance = 1.0e20f, dist, limsq = limit*limit;
	t_vector targetposition;
	tPolygon line(2);

	targetposition.x = 1.0e20f;
	targetposition.y = 0;

	for(++i;i!=entity.end();++i)
	{
		if(i->radarcolour == 0xffff0000 || i->radarcolour == 0xffff2020)
		{	line.vertex[0] = position;
			line.vertex[1] = i->position;
			if(!LineBoundariesCollision(line))
			{	dist = DistanceSquared(position,i->position);
				if(dist<distance && dist<limsq)
				{	distance = dist;
					targetposition = i->position;
				}
			}
		}
	}
	return targetposition;
}

t_vector FindNearestEnemyBullet(t_vector position, float limit)
{
	list<GameObject>::iterator i = entity.begin();
	float distance = 1.0e20f, dist, limsq = limit*limit;
	t_vector targetposition;
	tPolygon line(2);

	targetposition.x = 1.0e20f;
	targetposition.y = 0;

	for(++i;i!=entity.end();++i)
	{
		if(i->type==BULLET && CHECK(i->state,ENEMYBULLET))
		{	line.vertex[0] = position;
			line.vertex[1] = i->position;
			if(!LineBoundariesCollision(line))
			{	dist = DistanceSquared(position,i->position);
				if(dist<distance && dist<limsq)
				{	distance = dist;
					targetposition = i->position;
				}
			}
		}
	}
	return targetposition;
}

void MoveDrone(GameObject *object)
{	t_vector shipposition = ship[object->link].position;
	t_vector offset;

	Fsincos_t sincos = fsincos(ship[object->link].droneangle);
	offset.y = 60.0f*sincos.cosine;
	offset.x = 60.0f*sincos.sine;
	//offset.x = 60.0f*sin(ship[object->link].droneangle);
	//offset.y = 60.0f*cos(ship[object->link].droneangle);
	ship[object->link].droneangle += frametime*6.0f;
	if(ship[object->link].droneangle>=TWOPI) ship[object->link].droneangle = 0;

	object->position.x = shipposition.x + offset.x;
	object->position.y = shipposition.y + offset.y;
	object->angle = -ship[object->link].angle;

	sincos = fsincos(object->angle);
	object->unitvectorforward.x = sincos.sine;
	object->unitvectorforward.y = -sincos.cosine;
	//object->unitvectorforward.x = sin(object->angle);
	//object->unitvectorforward.y = -cos(object->angle);
	object->sinangle = object->unitvectorforward.x;
	object->cosangle = -object->unitvectorforward.y;

	if(ship[object->link].keys.FIRE)
	{
		if( thistick - object->lastfired > object->weapon->firerate )
		{	//Also check to see if it can make a bullet
			GridPosition(object);
			RotateOutline(&(object->outline), &(object->rotatedoutline), object->cosangle, object->sinangle);
			TranslateOutline(&(object->rotatedoutline), &(object->predictedoutline), &(object->position));
			if(!BoundaryCollisionDetect(object))
			{
				object->lastfired = thistick;
				t_vector bulletpos, bulletvel;
				bulletvel = object->unitvectorforward;
				bulletpos = object->position;
				bulletpos.x += bulletvel.x*10.0f;
				bulletpos.y += bulletvel.y*10.0f;
				CreateBullet(object->weapon, bulletpos, bulletvel, object->angle, true, 1.0f);
			}
		}
	}
}

void MoveLightningDrone(GameObject *object)
{
	t_vector shipposition = ship[object->link].position;
	t_vector offset;

	Fsincos_t sincos = fsincos(ship[object->link].droneangle);
	offset.y = -60.0f*sincos.cosine;
	offset.x = -60.0f*sincos.sine;

	object->position.x = shipposition.x + offset.x;
	object->position.y = shipposition.y + offset.y;

	object->changeangle = 0;
	if(ship[object->link].lightningtree.bolts!=0)
	{
		RotateTo(object,ship[object->link].lightningtree.targetlist[0].endtarget->position,0.04,TOWARDS);
		object->angle += object->changeangle;
	}
	else object->angle = -ship[object->link].droneangle;

	sincos = fsincos(object->angle);
	object->unitvectorforward.x = sincos.sine;
	object->unitvectorforward.y = -sincos.cosine;
	//object->unitvectorforward.x = sin(object->angle);
	//object->unitvectorforward.y = -cos(object->angle);
	object->sinangle = object->unitvectorforward.x;
	object->cosangle = -object->unitvectorforward.y;

	UNSET(object->state,LOCKED);
	if(ship[object->link].keys.FIRE)
	{
		GridPosition(object);
		RotateOutline(&(object->outline), &(object->rotatedoutline), object->cosangle, object->sinangle);
		TranslateOutline(&(object->rotatedoutline), &(object->predictedoutline), &(object->position));
		if(!BoundaryCollisionDetect(object))
		{
			if(ship[object->link].lightningtree.bolts!=0)
			{
				soundeffect[CRACKLE].SetSoundEffect(0);
				SET(object->state,LOCKED);
				for(int i=0;i<ship[object->link].lightningtree.bolts;++i)
				{
					ship[object->link].lightningtree.targetlist[i].endtarget->energy -= ship[object->link].lightningtree.targetlist[i].proportion*50.0f*frametime;
					if(ship[object->link].lightningtree.targetlist[i].endtarget->energy<=0)
					{
						ship[object->link].lightningtree.targetlist[i].endtarget->energy = 0;
						SET(ship[object->link].lightningtree.targetlist[i].endtarget->state,FORDESTRUCTION);
						SET(ship[object->link].lightningtree.targetlist[i].endtarget->state,CREATEEXPLOSION);
					}
					if(ship[object->link].lightningtree.targetlist[i].endtarget->isonscreen) CreateDebris(ship[object->link].lightningtree.targetlist[i].endtarget->position,200.0f,1.0f,2.0f,1.0f,0,0,&spark);
				}
				return;
			}
		}
	}
	soundeffect[CRACKLE].stop = true;
}

void MoveShieldDrone(GameObject *object)
{	t_vector shipposition = ship[object->link].position;
	t_vector offset;

	offset.x = 80.0f*sin(ship[object->link].deflectordroneangle[object->targetnode]);
	offset.y = 80.0f*cos(ship[object->link].deflectordroneangle[object->targetnode]);
	ship[object->link].deflectordroneangle[object->targetnode] -= frametime*10.0f;
	if(ship[object->link].deflectordroneangle[object->targetnode]<0) ship[object->link].deflectordroneangle[object->targetnode] += TWOPI;

	object->position.x = shipposition.x + offset.x;
	object->position.y = shipposition.y + offset.y;
	object->angle = ship[object->link].angle;
	object->unitvectorforward = ship[object->link].unitvectorforward;

	WrapToLevel(&object->position);
/*
	CreateEngineTrail(object->position, &redblob, 0.5, 1.2, 1.0, 1.2, true);

	if(ship[object->link].keys.FIRE)
	{
		if( thistick - object->lastfired > object->weapon->firerate )
		{	//Also check to see if it can make a bullet
			GridPosition(object);
			if(!BoundaryCollisionDetect(object))
			{
				object->lastfired = thistick;
				t_vector bulletpos, bulletvel;
				//bulletpos.x = object->weaponmount[0].x;		//Rotate the weapon mount position
				//bulletpos.y = object->weaponmount[0].y;
				//bulletpos = RotateVector(bulletpos,-object->angle);
				bulletpos = object->position;
				bulletvel = object->unitvectorforward;
				CreateBullet(object->weapon, bulletpos, bulletvel, -object->angle, true);
				//CreateBullet(object->weapon, object->position, object->unitvectorforward, -object->angle, false);
			}
		}
	}*/
}

int FindFreeDirection(GameObject *object)
{
	t_vector initialvector, modifiervector, testpoint;
	int pointtotal = 0;

	/* The function tests six points around the enemy craft. Each point is assigned a sequential 
	power of two. If a point is in a boundary it adds its value to the total. From the total the
	ship can work out what is the best direction to move in to free itself from being trapped */

	//initialvector = object->unitvectorforward;

	//Forwards direction
	modifiervector.x = 1.3f*object->radius*object->unitvectorforward.x;
	modifiervector.y = 1.3f*object->radius*object->unitvectorforward.y;

	for(int i=0;i<6;i++)
	{
		testpoint.x = object->position.x + modifiervector.x;
		testpoint.y = object->position.y + modifiervector.y;

		if(testpoint.x>=level.fwidth) testpoint.x -= level.fwidth;
		if(testpoint.x<0) testpoint.x += level.fwidth;
		if(testpoint.y>=level.fheight) testpoint.y -= level.fheight;
		if(testpoint.y<0) testpoint.y += level.fheight;

		if(PointBoundariesCollision(testpoint))
		{
			initialvector = object->position;
			initialvector.x += modifiervector.x;
			initialvector.y += modifiervector.y;
			//CreateMarker(initialvector);
			pointtotal += (0x1<<i);
		}
		modifiervector = RotateVectorQuick(modifiervector,0.5f,-0.866025f);	//60 degrees clockwise
	}

//	Normalise(&initialvector);

	switch(pointtotal)
	{
	case 1:
	case 2:
	case 3:
	case 7:
	case 32:
	case 33:
	case 34:
	case 35:
	case 49:
		//Backwards
		return BACKWARDS;
	case 4:
	case 8:
	case 12:
	case 14:
	case 16:
	case 20:
	case 24:
	case 28:
	case 56:
		//Forwards
		return FORWARDS;
	case 6:
	case 15:
		//Left
		return LEFT;
	case 48:
	case 57:
		//Right
		return RIGHT;
	}

	//Oh dear, real problems
	//dbout<<"PT "<<pointtotal<<endl;
	return BACKWARDS;
}


void SpinObject(GameObject *object)
{
	object->angle += object->changeangle;
	if(object->angle>=TWOPI) object->angle -= TWOPI;
	if(object->angle<0) object->angle += TWOPI;

	Fsincos_t sincos = fsincos(object->angle);
	object->sinangle = -sincos.sine;
	object->cosangle = sincos.cosine;

	//object->cosangle = cos(object->angle);
	//object->sinangle = -sin(object->angle);
	object->unitvectorforward.x = -object->sinangle;
	object->unitvectorforward.y = -object->cosangle;
	RotateOutline(&(object->outline), &(object->rotatedoutline), object->cosangle, object->sinangle);
	TranslateOutline(&(object->rotatedoutline), &(object->predictedoutline), &(object->position));
}

bool HomingComputer(int player)
{
	list<GameObject>::iterator i = entity.begin();
	float dotproduct, distance, mindistance = 1.0e20f;
	t_vector d;
	tPolygon line(2);
	bool foundtarget = false;

	line.vertex[0] = ship[player].position;

	for(++i;i!=entity.end();++i)
	{	//Run over all objects that are targettable
		if(i->type<SHIELDDRONE && i->type!=INERT) break;		//Bullets etc are always at the end
		if(i->radarcolour!=0xffff0000 && i->radarcolour!=0xffff0001 && i->radarcolour!=0xffff2020) continue;
		if(i->energy>1.0e5f) continue;		//Some objects can't be destroyed, don't bother target these

		d.x = i->position.x - ship[player].position.x;
		d.y = i->position.y - ship[player].position.y;
/*
		if(d.x>level.hwidth) d.x = d.x - level.fwidth;
		else if(d.x<-level.hwidth) d.x = level.fwidth + d.x;

		if(d.y>level.hheight) d.y = d.y - level.fheight;
		else if(d.y<-level.hheight) d.y = level.fheight + d.y;
*/
		//Find the dot product - gives distance in the unit vector forwards direction as uvf is a basis vector
		dotproduct = d.x*ship[player].unitvectorforward.x + d.y*ship[player].unitvectorforward.y;
		if(dotproduct>600.0f) continue;	//Can't be within the range polygon
		if(dotproduct<0) continue;		//Is not in front of player

		//Now we know that the object is within a 600 unit horizontal slice in front of the player
		d = RotateVectorQuick(d,ship[player].cosangle,ship[player].sinangle);
		if(PointInPoly(d,&ship[player].homingsweep))		//Might be in the sweep range
		{
			distance = d.x*d.x + d.y*d.y;
			line.vertex[1] = i->position;
			if(distance<mindistance && !LineBoundariesCollision(line))		//Is it close enough and not behind a wall?
			{
				mindistance = distance;
				ship[player].target = i->position;
				strcpy_s(ship[player].targetname,200,i->name);
				ship[player].targetenergy = i->energy/i->maxenergy;
				foundtarget = true;
			}
		}

	}

	if(foundtarget)		//distance will automatically be the correct distance, however, d may not be the correct displacement
	{
		PosToScreen(&ship[player].target,&ship[player].targetscr,player);
		ship[player].targetdistance = magic_sqrt(distance);
		ship[player].targetangle = PI - AngleToPoint(ship[player].position,ship[player].target,ship[player].unitvectorforward);
		if(ship[player].targetangle>PI) ship[player].targetangle -= TWOPI;
	}
	else
	{
		ship[player].targetangle = 0;
		ship[player].targetscr.x = ship[player].scrposition.x;
		ship[player].targetscr.y = 0;
	}

	return foundtarget;
}

bool t_Path::MakePath(t_vector start, t_vector end)
{
	int *foundpath, points;
	int startnode, endnode;
	tPolygon line(2);

	ClearPath();

	line.vertex[0] = start;
	line.vertex[1] = end;
	if(!LineBoundariesCollision(line))
	{
		startpoint = start;
		endpoint = end;
		valid = true;
		needschecking = false;
		nodes = 0;
		node = -1;
		return true;
	}

	if((startnode=FindNearestNode(start))<0) return false;			//No node reachable by the craft!
	if((endnode=FindNearestNode(end))<0) return false;				//The end position is not reachable from any node
	if(!PathFind(startnode, endnode, &foundpath, &points)) return false;		//There is no path possible between nodes. Note that PathFind allocates *foundpath

	//Translate the list of ints into a list of meshpoint pointers
	SetPathLength(points);
//	dbout<<"Complex path to "<<end.x<<" "<<end.y<<endl;
	for(int i=points-1,j=0; j<points; i--,j++)
	{
		path[j] = &level.mesh[foundpath[i]];
	//	dbout<<j<<": "<<foundpath[i]<<" "<<level.mesh[foundpath[i]].position.x<<" "<<level.mesh[foundpath[i]].position.y<<endl;
	}
	needschecking = false;
	valid = true;
	node = -1;
	nodes = points;
	startpoint = start;
	endpoint = end;

	//Delete the list of ints
	if(foundpath != NULL) delete [] foundpath;
	return true;
}

//Checks quickly to see if a path is still valid after CalculateNavigationMesh() has been called
bool t_Path::CheckPath()
{
	tPolygon line(2);
	bool linkfound = false;
	if(!valid)
	{
		//dbout<<"Path no longer valid!"<<endl;
		ClearPath();
		return false;
	}
	for(int i=node<0?0:node;i<nodes-1;i++)
	{
		linkfound = false;
		//Search through the unused links of this node to see if it's still linked to the next node
		for(int j=0;j<path[i]->links;j++)
		{
			if(path[i]->link[j] == path[i+1])	//Compare pointers
			{
				linkfound = true;
				break;
			}
		}
		if(!linkfound)
		{
			//dbout<<"Path found to be no longer valid"<<endl;
			valid = false;
			needschecking = false;
			return false;
		}
	}
	if(node<0 && path!=NULL)
	{
		line.vertex[0] = startpoint;
		line.vertex[1] = path[0]->position;
		if(LineBoundariesCollision(line))
		{
			//dbout<<"Path found to be no longer valid"<<endl;
			valid = false;
			needschecking = false;
			return false;
		}
	}
	else if(node==nodes-1  && path!=NULL)
	{
		line.vertex[1] = endpoint;
		line.vertex[0] = path[node]->position;
		if(LineBoundariesCollision(line))
		{
			//dbout<<"Path found to be no longer valid"<<endl;
			valid = false;
			needschecking = false;
			return false;
		}
	}
	//dbout<<"Path found to be still ok"<<endl;
	needschecking = false;
	valid = true;
	return true;
}

//Moves the path along one
bool t_Path::NextNode(GameObject *object)
{
	if(!valid) return false;		//Path not valid
	if(++node==nodes)				//We've reached the last node already
	{
		object->target = endpoint;	//Use the endpoint as a target
		valid = false;				//Make sure that the next time the path won't be valid
		return true;				//This time it's ok
	}
	object->target = path[node]->position;
	return true;
}

void MoveEnemy(GameObject *object)
{
	t_vector playerposition[2], newtarget;
	float pdistance[2];

#ifdef DEBUG
	listout<<object->name<<" stuck on wall"<<endl;
#endif

	if(CHECK(object->state,STUCKONWALL))		//Need some emergency freeing action
	{
		UNSET(object->state,STUCKONWALL);
		object->path.ClearPath();
		object->target = object->position;
	}

	SetPlayerStatusFlags(object, pdistance, playerposition);

	if(CHECK(object->state,RETREATING))		//Quick fix to make them stop shooting when they are retreating
	{
		UNSET(object->state,PLAYER1INRANGE);		//We can't shoot it now
		UNSET(object->state,PLAYER1SIGHTED);		//Forget that we have seen it in the past
		UNSET(object->state,PLAYER1INSIGHT);		//We can't see it now
		UNSET(object->state,PLAYER2INRANGE);		//We can't shoot it now
		UNSET(object->state,PLAYER2SIGHTED);		//Forget that we have seen it in the past
		UNSET(object->state,PLAYER2INSIGHT);		//We can't see it now
	}
	else
	{
		float ratio = 1.0f - object->energy/object->maxenergy;
		if(ratio>object->aggression)	//Higher ratio = more damage
		{
			SET(object->state,RETREATING);
			object->target = GetInverseTarget(object->position, object->target);
			object->path.ClearPath();
			return;
		}
	}

	if(CHECK(object->state,WAITING)) return;	//This object isn't allowed to move yet, it hasn't seen the player

	object->velocity.x = 0;
	object->velocity.y = 0;
	object->distancetotarget = DistanceSquared(object->position,object->target);

	if(boundarychanged) object->path.needschecking = true;

#ifdef DEBUG
	listout<<"Targetting"<<endl;
#endif

	//This function deals with all the pathfinding and targetting
	GetNextTargetPoint(object, pdistance, playerposition);
	object->angletotarget = TWOPI;

#ifdef DEBUG
	listout<<"Rotating & firing"<<endl;
#endif

	//The previous function will have returned a player position only if the players are in sight
	if(CHECK(object->state,PLAYER1INRANGE) || CHECK(object->state,PLAYER2INRANGE))
	{
		//Rotate and shoot
		RotateTo(object, object->target, 0.05f, TOWARDS);
		if(object->angletotarget<0.15f && object->weapon!=NULL)
		{	//Shoot bullets!
			if(thistick - object->lastfired > object->weapon->firerate)
			{
				object->lastfired = thistick;
				for(int i=0;i<object->weaponmounts;i++)
				{
					t_vector bulletpos, bulletvel;
					bulletpos.x = object->weaponmount[i].x;		//Rotate the weapon mount position
					bulletpos.y = object->weaponmount[i].y;
					bulletpos = RotateVector(bulletpos,object->angle);		//Use the quick version here?
					bulletpos.x += object->position.x;
					bulletpos.y += object->position.y;
					bulletvel.x = object->unitvectorforward.x;
					bulletvel.y = object->unitvectorforward.y;
					if(object->weaponmount[i].angle!=0) bulletvel = RotateVector(bulletvel,object->weaponmount[i].angle);
					CreateBullet(object->weapon, bulletpos, bulletvel, object->angle+object->weaponmount[i].angle, false, 1.0f);
					for(int j=0;j<5;++j) CreateSpray(bulletpos,bulletvel,120.0f,1.0f,2.0f,1.0f,0,0,&spark);
				}
			}
		}
	}
	else
	{
#ifdef DEBUG
		listout<<"Moving"<<endl;
#endif
		TrackTarget(object);	//Move towards the current target point
	}

	//Do the shifting on being hit by bullets
	if(object->shiftspeed>0)
	{
		object->velocity.x += object->shiftvector.x*object->shiftspeed*frametime;
		object->velocity.y += object->shiftvector.y*object->shiftspeed*frametime;
		object->shiftspeed -= SHIFTSLOWDOWN*frametime;
		if(object->shiftspeed<0)
		{
			object->shiftspeed = 0;
			object->shiftvector.x = 0;
			object->shiftvector.y = 0;
		}
	}
#ifdef DEBUG
	listout<<"Done"<<endl;
#endif

	return;
}

void GetNextTargetPoint(GameObject *object, float pdistance[], t_vector pposition[])
{
	//Firstly find out whether we're retreating
	if(CHECK(object->state,RETREATING))
	{
		//Maybe ignore player if retreating?
		//Or build a path that takes us away from the player
	}

#ifdef DEBUG
	listout<<"Targetting"<<endl;
#endif

	//Next find out whether we ought to go towards the player
	if(CHECK(object->state,PLAYER1INSIGHT) && CHECK(object->state,PLAYER2INSIGHT))
	{
		if(pdistance[0] < pdistance[1])	object->target = pposition[0];
		else object->target = pposition[1];
		object->path.ClearPath();
		return;
	}
	else if(CHECK(object->state,PLAYER1INSIGHT))
	{
		object->path.ClearPath();
		object->target = pposition[0];
		return;
	}
	else if(CHECK(object->state,PLAYER2INSIGHT))
	{
		object->path.ClearPath();
		object->target = pposition[1];
		return;
	}

#ifdef DEBUG
listout<<"Checking path"<<endl;
#endif

	//Next see if the path needs updating, i.e. if boundaries have changed
	if(object->path.needschecking)
	{
		if(!object->path.CheckPath()) object->distancetotarget = 0;		//Check the path
	}

#ifdef DEBUG
listout<<"Next node"<<endl;
#endif

	//Now either get the next point or make a new path
	if(object->distancetotarget>100.0) return;	//Keep going towards the old target
	if(object->path.valid)			//Move the path on by one
	{
		object->path.NextNode(object);
		return;
	}

#ifdef DEBUG
listout<<"Check if valid"<<endl;
#endif

	if(!object->path.valid)										//Otherwise, find a new target point, make new path
	{

#ifdef DEBUG
	listout<<"P1"<<endl;
#endif
		if(CHECK(object->state,PLAYER1SIGHTED) && ship[0].state == PLAY)
		{
			object->target = ship[0].position;
			if(object->path.MakePath(object->position,object->target))
			{
				object->path.NextNode(object);
				return;
			}
		}

#ifdef DEBUG
	listout<<"P2"<<endl;
#endif

		if(CHECK(object->state,PLAYER2SIGHTED) && ship[1].state == PLAY)	//If no path to player 1, or player 1 not spotted then try player 2
		{
			object->target = ship[1].position;
			if(object->path.MakePath(object->position,object->target))
			{
				object->path.NextNode(object);
				return;
			}
		}

#ifdef DEBUG
	listout<<"Random"<<endl;
#endif

		int counter = 0;
		do		//Otherwise find somewhere else to go
		{
			object->target = GetRandomTarget(object);
			if(object->target.x==0 && object->target.y==0)
			{
				object->path.ClearPath();
				break;
			}
			else object->path.MakePath(object->position,object->target);
		} while(!object->path.NextNode(object) && counter++<2);
	}

#ifdef DEBUG
	listout<<"Done GetTarget"<<endl;
#endif

	return;
}


void SetPlayerStatusFlags(GameObject *object, float pdistance[], t_vector playerposition[])
{
	//Loop over each player
	for(int i=0;i<playmode+1;i++)
	{
		if(ship[i].state==PLAY)
		{
			playerposition[i] = ship[i].position;
			//Wrap level around
			if(playerposition[i].x - object->position.x > level.hwidth) playerposition[i].x -= level.fwidth;
			else if(playerposition[i].x - object->position.x < level.mhwidth) playerposition[i].x += level.fwidth;

			if(playerposition[i].y - object->position.y > level.hheight) playerposition[i].y -= level.fheight;
			else if(playerposition[i].y - object->position.y < level.mhheight) playerposition[i].y += level.fheight;

			//Check the distance if it's valid
			pdistance[i] = DistanceSquared(object->position, playerposition[i]);
			if(pdistance[i]<object->sensorrange*object->sensorrange)		//If it's within range, see if we can see it
			{
				if(!TestPlayerSighting(object->position, &playerposition[i], i, object->type))	//It's visible
				{
					if(i==0)
					{
						SET(object->state,PLAYER1SIGHTED);		//Remember that we have seen it in the past
						SET(object->state,PLAYER1INSIGHT);		//We see it now
					}
					else
					{	SET(object->state,PLAYER2SIGHTED);		//Remember that we have seen it in the past
						SET(object->state,PLAYER2INSIGHT);		//We see it now
					}

					UNSET(object->state,RANGELIMITED);		//Object no longer stays within its boundaries
					UNSET(object->state,WAITING);			//Object can move now

					if(pdistance[i]<object->attackrange*object->attackrange)
					{	if(i==0) SET(object->state,PLAYER1INRANGE);		//We can shoot it now
						else SET(object->state,PLAYER2INRANGE);			//We can shoot it now
					}
					else
					{	if(i==0) UNSET(object->state,PLAYER1INRANGE);		//We can't shoot it now
						else UNSET(object->state,PLAYER2INRANGE);			//We can't shoot it now
					}
				}
				else		//It's in range but we can't see it - wall blocking our view
				{
					if(i==0)
					{	//Things have changed, we don't want to keep following the old path to the player
						if(CHECK(object->state,PLAYER1INSIGHT)) object->path.ClearPath();
						UNSET(object->state,PLAYER1INSIGHT);		//We can't see it now
					}
					else
					{	//Things have changed, we don't want to keep following the old path to the player
						if(CHECK(object->state,PLAYER2INSIGHT)) object->path.ClearPath();
						UNSET(object->state,PLAYER2INSIGHT);			//We can't see it now
					}
				}
			}
			else	//We can't see it
			{
				if(i==0)
				{
					UNSET(object->state,PLAYER1INRANGE);		//We can't shoot it now
					UNSET(object->state,PLAYER1INSIGHT);		//We can't see it now
				}
				else
				{
					UNSET(object->state,PLAYER2INRANGE);
					UNSET(object->state,PLAYER2INSIGHT);		//We can't see it now
				}
			}
		}
		else		//Ship not in play
		{
			if(i==0)
			{
				UNSET(object->state,PLAYER1SIGHTED);		//Forget that we have seen it in the past
				UNSET(object->state,PLAYER1INSIGHT);		//We don't see it now
				UNSET(object->state,PLAYER1INRANGE);		//We don't see it now
			}
			else
			{	UNSET(object->state,PLAYER2SIGHTED);		//Forget that we have seen it in the past
				UNSET(object->state,PLAYER2INSIGHT);		//We don't see it now
				UNSET(object->state,PLAYER2INRANGE);		//We don't see it now
			}
		}
	}
	return;
}


void DoScreen(int player)
{
    //Find real positions of all four corners relative to the ship
	ship[player].screen.vertex[0] = RotateVectorQuick(ship[player].screencorner[TOPLEFT], ship[player].cosangle, -ship[player].sinangle);
    ship[player].screen.vertex[1] = RotateVectorQuick(ship[player].screencorner[TOPRIGHT], ship[player].cosangle, -ship[player].sinangle);
    ship[player].screen.vertex[2] = RotateVectorQuick(ship[player].screencorner[BOTTOMRIGHT], ship[player].cosangle, -ship[player].sinangle);
    ship[player].screen.vertex[3] = RotateVectorQuick(ship[player].screencorner[BOTTOMLEFT], ship[player].cosangle, -ship[player].sinangle);

	float invscale = 1.0f/ship[player].screenscale;
	for(int i=0;i<4;++i)
	{
		ship[player].screen.vertex[i].x*=invscale;
		ship[player].screen.vertex[i].y*=invscale;

		//Shift to absolute coordinates
		ship[player].screen.vertex[i].x += ship[player].position.x;
		ship[player].screen.vertex[i].y += ship[player].position.y;
	}
}

//Check to make sure the target is still valid
bool CheckHunterTarget(Player *inship)
{
	std::list<GameObject>::iterator it = entity.begin();
	for(++it;it!=entity.end();++it)
	{
		if(&*it == inship->seekertarget) return true;
	}
	inship->seekertarget = NULL;
	return false;
}


void MoveHunterBullet(GameObject *bullet)
{
	int player;
	if(CHECK(bullet->state,PLAYER1INSIGHT)) player = 0;
	else player = 1;

	//If the target has been destroyed, then make the bullets fade away quickly
	if(ship[player].seekertarget == NULL)
	{
		bullet->faderate = 5.0;
		return;
	}

	//Rotate the unit vector towards the enemy
	t_vector targetposition = ship[player].seekertarget->position;
	t_vector position = bullet->position;
	t_vector d;
	d.x = targetposition.x - position.x;
	d.y = targetposition.y - position.y;

	if(d.x>level.hwidth) targetposition.x -= level.fwidth;
	else if(d.x<-level.hwidth) targetposition.x += level.fwidth;

	if(d.y>level.hheight) targetposition.y -= level.fheight;
	else if(d.y<-level.hheight) targetposition.y += level.fheight; 

	RotateTo(bullet,targetposition,0.05f,TOWARDS);
	bullet->unitvectorforward = RotateVector(bullet->unitvectorforward,bullet->changeangle);
}


void HunterSelectTarget(t_vector position, GameObject** seeker)
{
	if(seeker==NULL) return;
	t_vector d, dronevec;
	float distance, maxenergy = 0.0f, action, maxaction = 0.0f;
	tPolygon line(2);

	*seeker = NULL;

	//Find target
	std::list<GameObject>::iterator it = entity.begin();
	for(++it;it!=entity.end();++it)
	{
		if(it->radarcolour!=0xffff0000 && it->radarcolour!=0xffff0001 && it->radarcolour!=0xffff2020) continue;
		if(it->maxenergy>1.0e5f) continue;		//Some objects can't be destroyed, don't bother target these
		if(it->maxenergy<10.0f) continue;		//Some objects aren't worth bothering with, don't target these

		d.x = it->position.x - position.x;
		d.y = it->position.y - position.y;

		if(d.x>level.hwidth) d.x = d.x - level.fwidth;
		else if(d.x<-level.hwidth) d.x = level.fwidth + d.x;

		if(d.y>level.hheight) d.y = d.y - level.fheight;
		else if(d.y<-level.hheight) d.y = level.fheight + d.y;

		distance = d.x*d.x + d.y*d.y;

		if(distance>200000.0f) continue;

		//Check line of sight
		line.vertex[0] = position;
		line.vertex[1] = it->position;
		if(LineBoundariesCollision(line)) continue;

		action = it->maxenergy/distance;		//We want to attack the most powerful object, but not let small ones get too close in the meantime

		if(action>maxaction)
		{
			maxaction = action;
			*seeker = &*it;		//Find the nearby object with the largest energy
		}
	}
}

void LightningDroneSelectTarget(t_vector position, GameObject** lightning, GameObject* drone)
{
	t_vector d, dronevec;
	float distance, maxenergy = 0.0f, action, maxlightningaction = 0.0f;
	tPolygon line(2);

	if(lightning == NULL) return;
	if(drone==NULL) return;

	*lightning = NULL;

	dronevec.x = drone->position.x - position.x;
	dronevec.y = drone->position.y - position.y;
	Normalise(&dronevec);

	//Find target
	std::list<GameObject>::iterator it = entity.begin();
	for(++it;it!=entity.end();++it)
	{
		if(it->radarcolour!=0xffff0000 && it->radarcolour!=0xffff2020) continue;
		if(it->maxenergy>1.0e5f) continue;		//Some objects can't be destroyed, don't bother target these
		if(it->maxenergy<10.0f) continue;		//Some objects aren't worth bothering with, don't target these

		d.x = it->position.x - position.x;
		d.y = it->position.y - position.y;

		if(d.x>level.hwidth) d.x = d.x - level.fwidth;
		else if(d.x<-level.hwidth) d.x = level.fwidth + d.x;

		if(d.y>level.hheight) d.y = d.y - level.fheight;
		else if(d.y<-level.hheight) d.y = level.fheight + d.y;

		distance = d.x*d.x + d.y*d.y;

		if(distance>200000.0f) continue;

		//Check line of sight
		line.vertex[0] = position;
		line.vertex[1] = it->position;
		if(LineBoundariesCollision(line)) continue;

		action = it->maxenergy/distance;		//We want to attack the most powerful object, but not let small ones get too close in the meantime

		Normalise(&d);
		if(d.x*dronevec.x + d.y*dronevec.y <0.3) continue;	//Make sure the target is within 72 degrees (146 deg total)

		if(action>maxlightningaction)
		{
			maxlightningaction = action;
			*lightning = &*it;		//Find the nearby object with the largest energy
		}
	}
}

void LightningSelectTarget(GameObject* starttarget, LightningTree* lightningtree, int count)
{
	t_vector d;
	float distance, maxenergy = 0.0f, action, maxlightningaction = 0.0f, maxdistance, proportion = 1.0f;
	tPolygon line(2);
	GameObject* temptarget = NULL;

	if(lightningtree == NULL) return;

	if(count>3) return;
	if(count==0) lightningtree->bolts = 0;
	else if(lightningtree->bolts==20) return;

	//Each jump loses one quarter of its reach distance - maxdistance is a squared distance
	maxdistance = (float)count*448.0f/4.0f;
	maxdistance = 200000.0f - maxdistance*maxdistance;
	proportion = 1.0f - (float)count*0.25f;

	//Find next target in chain
	std::list<GameObject>::iterator it = entity.begin();
	for(++it;it!=entity.end();++it)
	{
		if(it->radarcolour!=0xffff0000 && it->radarcolour!=0xffff2020) continue;
		if(it->maxenergy>1.0e5f) continue;		//Some objects can't be destroyed, don't bother target these
		if(it->maxenergy<10.0f) continue;		//Some objects aren't worth bothering with, don't target these

		d.x = it->position.x - starttarget->position.x;
		d.y = it->position.y - starttarget->position.y;

		//Level wrapping
		if(d.x>level.hwidth) d.x = d.x - level.fwidth;
		else if(d.x<-level.hwidth) d.x = level.fwidth + d.x;

		if(d.y>level.hheight) d.y = d.y - level.fheight;
		else if(d.y<-level.hheight) d.y = level.fheight + d.y;

		distance = d.x*d.x + d.y*d.y;

		if(distance>maxdistance) continue;

		//Check line of sight
		line.vertex[0] = starttarget->position;
		line.vertex[1] = it->position;
		if(LineBoundariesCollision(line)) continue;

		//Don't let the lightning chain go back to a previously zapped object
		if(lightningtree->CheckIfAlreadySelected(&*it)) continue;

		action = it->maxenergy/distance;		//We want to attack the most powerful object, but not let small ones get too close in the meantime

		if(action>maxlightningaction)
		{
			maxlightningaction = action;
			temptarget = &*it;
		}
	}

	if(temptarget!=NULL)
	{
		if(lightningtree->AddBolt(starttarget,temptarget,proportion))		//Add to target list
		{
			LightningSelectTarget(temptarget, lightningtree, count+1);
			LightningSelectTarget(temptarget, lightningtree, count+1);
		}
	}
}