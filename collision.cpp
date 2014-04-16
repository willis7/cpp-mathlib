/* Contains all the rountines for collision detection etc */

#include "OR.h"

extern list<GameObject> entity;
extern Level level;
extern Player ship[2];
extern int playmode, gamemode, thistick, deadtimer;
extern ofstream dbout;
//extern t_sound soundeffect[NUMEFFECTS];
extern bool boundarychanged;
extern Base base[2];
extern BaseData basedata[10];
extern Equipment equipdata[NUMEQUIPTYPES];
extern SoundEffect soundeffect[NUMEFFECTS];
extern float frametime;
extern string nextlevel;
extern GraphicalObject spark;
extern bool equipment[NUMEQUIPTYPES];
extern Weapon *weaponlist;

//Movement
//Repeat:
//Check all points for collisions
//Build list of those points in both objects which cause collisions
//Divide the movement into n ( = e.g. 10) smaller steps
//Find the nearest distance of approach allowed by checking only those points on the list - others will be irrelevant 
//Save the position to object, calculate the length of the remaining movement vector
//If it can't move at all, i.e. new position is within eps of original position, then break
//Find single point which is nearest to its colliding line at this distance 
//Resolve the moving object's velocity along the colliding line
    //If there is more than one, to within a small amount, check all
    //If resolved vectors are almost the same direction, continue
    //Otherwise, object cannot move, so break 
//Repeat with new vector, so long as the remaining resolved vector is longer than eps

int collisiontests;

//Any non-solid object types must have a number lower than EXPLOSION
//Any non-enemy-interacting object types must have a number higher than BASE
//enum GAME_OBJ_TYPE {INERT, FADE, EXPLOSION,										//Completely non-interacting types
//					BULLET, MISSILE, SHIELDDRONE, CONVERTER, ENEMY, MINE, ROCK, SPAWN, TURRET, SPINNER, TRIGGERSWITCH,	//Enemy collidable types
//					TELEPORT, BASE, SHIELDCHARGER, EXITGATE, DRONE, SPAWNINERT, GAMESAVER, MISSILECRATE, GEMSTONE, ENERGY, BUTTONSWITCH};	//Player interaction only types


void DoCollision(GameObject* object1, GameObject* object2)
{
	if(object1->type==TRIGGERSWITCH)
	{
		if(CHECK(object1->state,ARMED))
		{
			if(CHECK(object1->state,TRIGGEREDBYENEMY))
			{
				if(object2->type==ENEMY)
				{	object1->lastfired = thistick;
					UNSET(object1->state,ARMED);
					TOGGLE(level.boundary[object1->link].state,ON);
					boundarychanged = true;
					//dbout<<"*** Trigger for "<<object1->link<<"triggered, time "<<object1->timer<<" x "<<object2->position.x<<" y "<<object2->position.y<<"***"<<endl;
				}
			}
			else if(CHECK(object1->state,TRIGGEREDBYROCK))
			{
				if(object2->type==ROCK)
				{
					object1->lastfired = thistick;
					UNSET(object1->state,ARMED);
					TOGGLE(level.boundary[object1->link].state,ON);
					boundarychanged = true;
					//dbout<<"*** Trigger for "<<object1->link<<"triggered, time "<<object1->timer<<" x "<<object2->position.x<<" y "<<object2->position.y<<"***"<<endl;
				}
			}
		}
	}
	else if(object2->type==TRIGGERSWITCH)
	{
		if(CHECK(object2->state,ARMED))
		{
			if(CHECK(object2->state,TRIGGEREDBYENEMY))
			{
				if(object1->type==ENEMY)
				{	object2->lastfired = thistick;
					UNSET(object2->state,ARMED);
					TOGGLE(level.boundary[object2->link].state,ON);
					boundarychanged = true;
					//dbout<<"*** Trigger for "<<object2->link<<"triggered by "<<object1->name<<", time "<<object2->timer<<", o1 x "<<object1->position.x<<" o1 y "<<object1->position.y<<" ***"<<endl;
				}
			}
			else if(CHECK(object2->state,TRIGGEREDBYROCK))
			{
				if(object1->type==ROCK)
				{
					object2->lastfired = thistick;
					UNSET(object2->state,ARMED);
					TOGGLE(level.boundary[object2->link].state,ON);
					boundarychanged = true;
					//dbout<<"*** Trigger for "<<object2->link<<"triggered by "<<object1->name<<", time "<<object2->timer<<", o1 x "<<object1->position.x<<" o1 y "<<object1->position.y<<" ***"<<endl;
				}
			}
		}
	}
	else
	{	//Proper collision!
		//If it's an enemy bullet, only collide with rocks that are >1.0e6 in energy
		if( ( CHECK(object2->state,ENEMYBULLET) && (object1->type!=ROCK || object1->energy<1.0e6f)) || (CHECK(object1->state,ENEMYBULLET) && (object2->type!=ROCK || object2->energy<1.0e6f)) )
		{
			if(object1->type==SHIELDDRONE)
			{
				SET(object2->state,FORDESTRUCTION);
				SET(object2->state,CREATEPUFF);
				for(int j=0;j<5;++j) CreateDebris(object1->position,200.0f,1.0f,2.0f,1.0f,0,0,&spark);
				object2->energy = 0;
				UNSET(object2->state,ENEMYBULLET);
				soundeffect[BULLETIMPACT].SetSoundEffect(0);
			}
			if(object2->type==SHIELDDRONE)
			{
				SET(object1->state,FORDESTRUCTION);
				SET(object1->state,CREATEPUFF);
				for(int j=0;j<5;++j) CreateDebris(object1->position,200.0f,1.0f,2.0f,1.0f,0,0,&spark);
				object1->energy = 0;
				UNSET(object1->state,ENEMYBULLET);
				soundeffect[BULLETIMPACT].SetSoundEffect(0);
			}
			if(object1->type==CONVERTER)
			{
				UNSET(object2->state,ENEMYBULLET);
				//dbout<<"Bullet converted"<<endl;
			}
			if(object2->type==CONVERTER)
			{
				UNSET(object1->state,ENEMYBULLET);
				//dbout<<"Bullet converted"<<endl;
			}
			return;		//Enemy bullets only interact with rocks, converters and shield drones
		}
		if(object1->type==SHIELDDRONE || object2->type==SHIELDDRONE) return;
		if(object1->type==CONVERTER || object2->type==CONVERTER) return;
		float temp = object1->energy;
		object1->energy -= object2->energy;
		object2->energy -= temp;
		object1->baralpha = 0xa0ffffff;
		object2->baralpha = 0xa0ffffff;
		if(object1->energy<=0 || object1->type == BULLET || object1->type == MISSILE)
		{
			object1->energy = 0;
			SET(object1->state,FORDESTRUCTION);
			if(object1->type!=BULLET) SET(object1->state,CREATEEXPLOSION);
			else
			{
				SET(object1->state,CREATEPUFF);
				for(int j=0;j<5;++j) CreateDebris(object1->position,200.0f,1.0f,2.0f,1.0f,0,0,&spark);
				if(object2->type==ENEMY)
				{
					object2->shiftvector.x += object1->unitvectorforward.x;
					object2->shiftvector.y += object1->unitvectorforward.y;
					Normalise(&object2->shiftvector);
					object2->shiftspeed += SHIFTSPEED*object1->maxenergy/object2->maxenergy;
					if(object2->shiftspeed>130.0f) object2->shiftspeed = 130.0f;
				}
			}
		}
		if(object2->energy<=0 || object2->type == BULLET || object2->type == MISSILE)
		{
			object2->energy = 0;
			SET(object2->state,FORDESTRUCTION);
			if(object2->type!=BULLET) SET(object2->state,CREATEEXPLOSION);
			else
			{
				SET(object2->state,CREATEPUFF);
				for(int j=0;j<5;++j) CreateDebris(object2->position,200.0f,1.0f,2.0f,1.0f,0,0,&spark);
				if(object1->type==ENEMY)
				{
					object1->shiftvector.x += object2->unitvectorforward.x;
					object1->shiftvector.y += object2->unitvectorforward.y;
					Normalise(&object1->shiftvector);
					object1->shiftspeed += SHIFTSPEED*object2->maxenergy/object1->maxenergy;
					if(object1->shiftspeed>130.0f) object1->shiftspeed = 130.0f;
				}
			}
		}
		if((object2->type==ENEMY && object1->type<SHIELDDRONE) || (object1->type==ENEMY && object2->type<SHIELDDRONE))
		{
			UNSET(object2->state,WAITING);
			UNSET(object2->state,RANGELIMITED);
			UNSET(object1->state,WAITING);
			UNSET(object1->state,RANGELIMITED);
		}
	}
	return;
}

//Any non-solid object types must have a number lower than EXPLOSION
//Any non-enemy-interacting object types must have a number higher than BASE
//enum GAME_OBJ_TYPE {INERT, FADE, EXPLOSION,										//Completely non-interacting types
//					BULLET, MISSILE, SHIELDDRONE, CONVERTER, ENEMY, MINE, ROCK, SPAWN, TURRET, SPINNER, TRIGGERSWITCH,	//Enemy collidable types
//					TELEPORT, BASE, SHIELDCHARGER, EXITGATE, DRONE, SPAWNINERT, GAMESAVER, MISSILECRATE, GEMSTONE, ENERGY, BUTTONSWITCH};	//Player interaction only types


//Check to see whether two objects are intersecting
bool CollisionTest(GameObject* object1, GameObject* object2)
{
	if(object1->type<SHIELDDRONE && object2->type<SHIELDDRONE) return false;
	if(object1->type==TRIGGERSWITCH && !(object2->type==ENEMY || object2->type==ROCK)) return false;
	if(object2->type==TRIGGERSWITCH && !(object1->type==ENEMY || object1->type==ROCK)) return false;

	++collisiontests;

	//Circle collision test
	float dx, dy, distance;
	dx = object1->position.x - object2->position.x;
	dy = object1->position.y - object2->position.y;
	distance = dx*dx + dy*dy;
	if( distance > 2.0f*(object1->radiussquared + object2->radiussquared) )	return false;		//Can't be colliding

	//Detect collision more precisely
	bool collided;
	if(CHECK(object1->state,POINTDETECTION)) collided = PointInPoly(object1->position, &object2->predictedoutline);
	else if(CHECK(object2->state,POINTDETECTION)) collided = PointInPoly(object2->position, &object1->predictedoutline);
	else collided = PolyCollisionDetect(&object1->predictedoutline, &object2->predictedoutline);

	if(collided) DoCollision(object1, object2);
	return collided;
}

void BroadPhase()
{
	bool collided = false;
	int objects = 0;
	collisiontests = 0;

	//Loop over every grid square
	for(int igrid = 0; igrid<level.gridsquares; ++igrid)
	{
		objects = level.grid[igrid].collideobjects.size();
		for(int i=0; i<objects; ++i)
		{
			//Loop over all other objects in this grid square
			for(int j=i+1; j<level.grid[igrid].collideobjects.size(); ++j)
				CollisionTest(level.grid[igrid].collideobjects[i], level.grid[igrid].collideobjects[j]);

			//Loop over all objects in the right grid square
			for(int j=0; j<level.grid[igrid].neighbour[RIGHT]->collideobjects.size(); ++j)
				CollisionTest(level.grid[igrid].collideobjects[i], level.grid[igrid].neighbour[RIGHT]->collideobjects[j]);

			//Loop over all objects in the bottom right grid square
			for(int j=0; j<level.grid[igrid].neighbour[BOTTOMRIGHT]->collideobjects.size(); ++j)
				CollisionTest(level.grid[igrid].collideobjects[i], level.grid[igrid].neighbour[BOTTOMRIGHT]->collideobjects[j]);

			//Loop over all objects in the bottom grid square
			for(int j=0; j<level.grid[igrid].neighbour[BOTTOM]->collideobjects.size(); ++j)
				CollisionTest(level.grid[igrid].collideobjects[i], level.grid[igrid].neighbour[BOTTOM]->collideobjects[j]);

		}
	}
}

void CollisionDetect()
{
	list<GameObject>::iterator i, j;
	float distance;
	t_vector d;
	bool collided = false;
	int collisiontests = 0;

	for(i=++entity.begin();i!=entity.end();++i)
	{
		if(i->type==INERT) continue;		// Changed from (< BULLET)
		if(i->type==MISSILE) continue;
		if(i->type==DRONE) continue;
		if(i->type==SHIELDDRONE) continue;
		if(i->type==LIGHTNINGDRONE) continue;
		if(i->type==CONVERTER) continue;

		/* This loop does object--player collision detection */
		//Collision detection with player
		for(int k=0;k<=playmode;k++)
		{
			int j = (k+1)&0x1;
			if(ship[k].state==PLAY)
			{
				++collisiontests;
				if(i->type == BULLET)
				{
					if(CHECK(i->state,ENEMYBULLET))
					{
						d.x = i->position.x - ship[k].position.x;
						d.y = i->position.y - ship[k].position.y;
						distance = d.x*d.x + d.y*d.y;
						if( distance > ship[k].radiussquared ) continue;
					}
					else
					{
						--collisiontests;
						break;
					}
				}
				else
				{
					d.x = i->position.x - ship[k].position.x;
					d.y = i->position.y - ship[k].position.y;

					if(d.x>level.hwidth) d.x = d.x - level.fwidth;
					else if(d.x<-level.hwidth) d.x = level.fwidth + d.x;

					if(d.y>level.hheight) d.y = d.y - level.fheight;
					else if(d.y<-level.hheight) d.y = level.fheight + d.y;

					distance = d.x*d.x + d.y*d.y;
					if(i->type==GEMSTONE)		//This has to be here in order for the field to reach distant gems
					{
						if(ship[k].equipment[GEMTRACTOR])
						{
							if( distance < equipdata[GEMTRACTOR].value )	//If gem is within range
							{
								tPolygon line(2);
								line.vertex[0] = i->position;
								line.vertex[1] = ship[k].position;
								if(!LineBoundariesCollision(line))	//Move gem towards player
								{
									float invdist = 1.0f/Normalise(&d);		//Convert d to a unit vector
									t_vector v;
									v.x = frametime*10000.0f*d.x*invdist;
									v.y = frametime*10000.0f*d.y*invdist;
									i->position.x -= v.x;
									i->position.y -= v.y;
								}
							}
						}
					}
					if( distance > 2.0f*(i->radiussquared + ship[k].radiussquared) ) continue;
				}

				//Detect collision more precisely
				collided = false;
				if(CHECK(i->state,POINTDETECTION)) collided = PointInPoly(i->position, &ship[k].predictedoutline);
				else collided = PolyCollisionDetect(&i->predictedoutline, &ship[k].predictedoutline);
				if(collided)
				{	//Collision!
					if(i->type < TRIGGERSWITCH && !ship[k].invulnerable)	//BULLET, SHIELDDRONE, MISSILE, CONVERTER, ENEMY, MINE, ROCK, SPAWN, TURRET, SPINNER
					{
						float temp = i->energy;
						i->energy -= ship[k].energy;
						ship[k].screenalpha = 0xc0ffffff;
						if(i->energy<0)
						{
							SET(i->state,FORDESTRUCTION);
							if(i->type != BULLET) SET(i->state,CREATEEXPLOSION);
							else for(int j=0;j<5;++j) CreateDebris(i->position,200.0f,1.0f,2.0f,1.0f,0,0,&spark);
						}
						if(ship[k].equipment[SHIELD] && ship[k].shields>0)
						{	ship[k].shieldhit = true;
							ship[k].shields -= temp;	//Shields are reduced by the impact
							temp *= 0.4f + 0.6f*(1.0f - ship[k].shields/ship[k].maxshields);				//Impact is reduced by the shield, by up to 60%
						}
						ship[k].energy -= temp;
						if(ship[k].energy<=0) KillShip(k);
						continue;
					}
					if(i->type==TELEPORT)
					{	if(!CHECK(i->state,LOCKED))
							if(ship[k].keys.OPERATE)
							{
								CreateSparkle(ship[k].position);
								ship[k].position = i->target;
								CreateSparkle(ship[k].position);
								ship[k].explosions.AddExplosion(ship[k].position.x,ship[k].position.y);
								t_point scrposition;
								if(PosToScreen(&ship[k].position, &scrposition, j)) ship[j].explosions.AddExplosion(ship[k].position.x,ship[k].position.y);
								soundeffect[USETELEPORT].SetSoundEffect(0);
							}
						continue;
					}
					if(i->type==BUTTONSWITCH)
					{	if(ship[k].keys.OPERATE)
						{
							TOGGLE(i->state,ON);
							TOGGLE(level.boundary[i->link].state,ON);
							//dbout<<"Boundary "<<i->link<<" toggled by switch to "<<level.boundary[i->link].state<<endl;
							boundarychanged = true;
							soundeffect[OPERATESWITCH].SetSoundEffect(0);
						}
						continue;
					}
					if(i->type==TRIGGERSWITCH && CHECK(i->state,TRIGGEREDBYPLAYER) && CHECK(i->state,ARMED))
					{	//dbout<<"Trigger triggered"<<endl;
						boundarychanged = true;
						i->lastfired = thistick;
						UNSET(i->state,ARMED);
						TOGGLE(level.boundary[i->link].state,ON);
						dbout<<"Boundary "<<i->link<<" toggled by trigger to "<<level.boundary[i->link].state<<endl;
						continue;
					}
					if(i->type==EXITGATE)
					{
						if(!CHECK(i->state,LOCKED))
						{
							ship[k].screenmessage.AddMessage("Transmission gate ready!");
							if(ship[k].keys.OPERATE)
							{
								nextlevel = i->storestring;
								strcpy_s(level.next,256,nextlevel.c_str());
								gamemode = EXITLEVEL;
								CreateSparkle(ship[k].position);
								ship[k].explosions.AddExplosion(ship[k].position.x,ship[k].position.y);
								t_point scrposition;
								if(PosToScreen(&ship[k].position, &scrposition, j)) ship[j].explosions.AddExplosion(ship[k].position.x,ship[k].position.y);
								soundeffect[SP_SYSTEMS_DOWN].SetSoundEffect(0);
							}
						}
						else
						{
							ship[k].screenmessage.AddMessage("Complete objectives to unlock gate");
						}
						continue;
					}
					if(i->type==BASE)
					{
						char message[100];
						sprintf_s(message,100,"Landing charge: %d",basedata[i->link].landingcharge);
						ship[k].screenmessage.AddMessage(message);
						if(ship[k].keys.OPERATE)
						{	//Check that we can afford to land in this base
							if(ship[k].credits>=basedata[i->link].landingcharge)
							{
								base[k].SetUpBase(&basedata[i->link]);
								ship[k].state = INBASE;
								ship[k].credits -= basedata[i->link].landingcharge;
								soundeffect[LANDINBASE].SetSoundEffect(0);
								soundeffect[SP_BASE_SYSTEMS_ACTIVATED].SetSoundEffect(0);
							}
							else soundeffect[NOMISSILES].SetSoundEffect(0);
						}
						continue;
					}
					if(i->type==ENERGY)
					{
						if(ship[k].energy == ship[k].maxenergy)
						{
							if(playmode==TWOPLAYER)
							{
								if(ship[(k+1)&0x1].state==DEAD)
								{
									ship[(k+1)&0x1].energy += i->energy;
									if(ship[(k+1)&0x1].energy >= ship[(k+1)&0x1].maxenergy)
									{
										ship[(k+1)&0x1].energy = ship[(k+1)&0x1].maxenergy;
										ship[(k+1)&0x1].state = PLAY;
										ship[k].screenmessage.AddMessage("Other player restored");
										ship[(k+1)&0x1].screenmessage.AddMessage("Other player restored you!");
									}
									else
									{
										ship[k].screenmessage.AddMessage("Other player energy increased");
									}
								}
								else continue;
							}
							else continue;
						}
						else
						{
							ship[k].energy += i->energy;
							if(ship[k].equipment[SHIELD])
							{	ship[k].shieldhit = true;
								ship[k].shields += i->energy;
								if(ship[k].shields>ship[k].maxshields) ship[k].shields = ship[k].maxshields;
							}
							if(ship[k].energy>ship[k].maxenergy) ship[k].energy = ship[k].maxenergy;
							char message[100];
							sprintf_s(message,100,"Collected %d energy",(int)i->energy);
							ship[k].screenmessage.AddMessage(message);
						}
						i->energy = 0;
						i->fade = 0;
						i->type = BULLET;
						i->gridnumber = 0;
						SET(i->state,FORDESTRUCTION);
						SET(i->state,CREATESPARKLE);
						soundeffect[COLLECT].SetSoundEffect(0);
						continue;
					}
					if(i->type==SHIELDCHARGER)
					{
						if(ship[k].equipment[SHIELD])
						{	ship[k].shieldhit = true;
							ship[k].shields += frametime*20.0f;
							if(ship[k].shields>ship[k].maxshields) ship[k].shields = ship[k].maxshields;
							char message[100];
							sprintf_s(message,100,"Shields %d%%",(int)(100.0f*ship[k].shields/ship[k].maxshields));
							ship[k].screenmessage.AddMessage(message);
						}
						continue;
					}
					if(i->type==GAMESAVER)
					{
						if(!CHECK(i->state,LOCKED))
						{
							if(ship[0].state==DEAD)
							{
								ship[0].position = i->position;
								ship[0].state = PLAY;
								ship[0].energy = ship[0].maxenergy*0.5f;
							}
							if(playmode==TWOPLAYER && ship[1].state==DEAD)
							{
								ship[1].position = i->position;
								ship[1].state = PLAY;
								ship[1].energy = ship[1].maxenergy*0.5f;
							}
							if(!SaveGame((playmode==SINGLEPLAYER?"data\\savefile1p.dat":"data\\savefile2p.dat"),&level)) dbout<<"Could not save game"<<endl;
							SET(i->state,LOCKED);
							CreateSparkle(i->position);
							CreateSparkle(i->position);
							ship[k].screenmessage.AddMessage("Quantum reality analyser activated!");
							ship[(k+1)&0x1].screenmessage.AddMessage("Quantum reality analyser activated!");
							ship[k].explosions.AddExplosion(ship[k].position.x,ship[k].position.y);
							t_point scrposition;
							if(PosToScreen(&ship[k].position, &scrposition, j)) ship[j].explosions.AddExplosion(ship[k].position.x,ship[k].position.y);
						}
						continue;
					}
					if(i->type==MISSILECRATE && ship[k].equipment[MISSILE_LAUNCHER] && ship[k].missiles<ship[k].maxmissiles)
					{
						ship[k].missiles += i->link;
						if(ship[k].missiles>ship[k].maxmissiles) ship[k].missiles = ship[k].maxmissiles;
						i->type = BULLET;
						i->gridnumber = 0;
						SET(i->state,FORDESTRUCTION);
						SET(i->state,CREATESPARKLE);
						char message[100];
						sprintf_s(message,100,"Collected %d missiles",i->link);
						ship[k].screenmessage.AddMessage(message);
						soundeffect[INSTALLITEM].SetSoundEffect(0);
						continue;
					}
					if(i->type==GEMSTONE)
					{
						ship[k].credits += i->link;
						if(ship[k].equipment[SHIELD])
						{
							ship[k].shields += (float)i->link*5.0f;
							ship[k].shieldhit = true;
							if(ship[k].shields>ship[k].maxshields) ship[k].shields = ship[k].maxshields;
						}
						i->type = BULLET;
						i->gridnumber = 0;
						SET(i->state,FORDESTRUCTION);
						SET(i->state,CREATESPARKLE);
						soundeffect[COLLECT].SetSoundEffect(0);
						continue;
					}
					if(i->type==BLUEPRINT)
					{
						if(CHECK(i->state,STOREEQUIPMENT)) equipment[i->link] = true;
						else if(CHECK(i->state,STOREWEAPON)) weaponlist[i->link].available = true;
						i->type = BULLET;
						i->gridnumber = 0;
						SET(i->state,FORDESTRUCTION);
						SET(i->state,CREATESPARKLE);
						char message[100];
						sprintf_s(message,100,"Collected blueprint for %s",i->storestring.c_str());
						ship[k].screenmessage.AddMessage(message);
						ship[(k+1)&1].screenmessage.AddMessage(message);
						soundeffect[COLLECT].SetSoundEffect(0);
						continue;
					}
				}
			}
		}//End k loop
	}//End i loop

	ship[0].keys.OPERATE = false;
	ship[1].keys.OPERATE = false;

	BroadPhase();
}


//Should be a much faster routine than previously, as most boundaries are ignored if they aren't nearby
bool BoundaryCollisionDetect(GameObject *object)
{
	return GridCollision(object, object->gridnumber);
}

bool GridCollision(GameObject *object, int gridnumber)
{
	//Collision detect with the boundaries in this grid square
	if(gridnumber>=level.gridsquares)
	{
		//dbout<<"Out of bounds error - "<<gridnumber<<" >= "<<level.gridsquares<<endl;
		return true;
	}
	if(CHECK(object->state,POINTDETECTION))
	{
		for(int i=0;i<level.grid[gridnumber].boundaries;i++)
			if(level.boundary[level.grid[gridnumber].boundary[i]].state == ON)
				if(PointInPoly(object->position,&level.boundary[level.grid[gridnumber].boundary[i]].poly)) return true;
	}
	else
	{
		for(int i=0;i<level.grid[gridnumber].boundaries;i++)
			if(level.boundary[level.grid[gridnumber].boundary[i]].state == ON)
				if(PolyCollisionDetect(&object->predictedoutline,&(level.boundary[level.grid[gridnumber].boundary[i]].poly))) return true;	
	}

	return false;
}

float FindClosestLine(tPolygon *poutline, tPolygon *output, t_vector position, int gridnumber, int *poly, t_vector *point)
{
	int i, whichpolygon;
	float distance, mindistance = 1.0e11f;
	tPolygon store(2);
	t_vector storepoint;

	if(output->points!=2) output->SetPoints(2);
	//Collision detect with boundaries
	for(i=0;i<level.grid[gridnumber].boundaries;i++)
	{
		if(level.boundary[level.grid[gridnumber].boundary[i]].state == ON)
		{
			distance = PointThatsNearest(poutline, &(level.boundary[level.grid[gridnumber].boundary[i]].poly), &store, &whichpolygon, &storepoint);
			if(distance<mindistance)
			{
				*poly = whichpolygon;
				mindistance = distance;
				output->vertex[0] = store.vertex[0];
				output->vertex[1] = store.vertex[1];
				*point = storepoint;
			}
		}
	}

	return mindistance;
}


bool PointBoundariesCollision(t_vector point)
{	//Collision detect with boundaries
	int gridx, gridy, gridnumber;

	gridx = (int)(point.x*ONEOVERGRIDSIZE);
	gridy = (int)(point.y*ONEOVERGRIDSIZE);
	gridnumber = gridx + gridy*level.gridsize.x;

	if(gridnumber>=level.gridsquares || gridnumber<0)
	{
		//dbout<<"Point Boundaries Collision - gn "<<gridnumber<<" bounds "<<level.grid[gridnumber].boundaries<<" gx "<<gridx<<" gy "<<gridy<<" px "<<point.x<<" py "<<point.y<<endl;
		return true;
	}
	//Collision detect with the boundaries in this grid square
	for(int i=0;i<level.grid[gridnumber].boundaries;i++)
	{
		if(level.boundary[level.grid[gridnumber].boundary[i]].state == ON)
			if(PointInPoly(point,&(level.boundary[level.grid[gridnumber].boundary[i]].poly)))
				return true;
	}

	return false;
}

void SwapLineEnds(tPolygon *line)
{
	t_vector temp = line->vertex[0];
	line->vertex[0] = line->vertex[1];
	line->vertex[1] = temp;
}

bool LineGridBoundaries(tPolygon *line, int thisgrid)
{
	if(thisgrid<0) return false;
	if(thisgrid>=level.gridsquares) return false;

	for(int j=0;j<level.grid[thisgrid].boundaries;++j)
		if(level.boundary[level.grid[thisgrid].boundary[j]].state == ON)
			if(LinePolygonIntersection(line,&level.boundary[level.grid[thisgrid].boundary[j]].poly)) return true;
	return false;
}

/* Working? Uses DDA algorithm to avoid having to test every single boundary */
bool LineBoundariesCollision(tPolygon line)
{
	int thisgrid;
	float dx, dy, dydx, x, y;
	int gridx, gridy, slopetype;

	if(line.vertex[0].x < line.vertex[1].x)	//If the start of the line is less than the end in x
	{
		if(line.vertex[0].y < line.vertex[1].y)			//Positive slope, exactly as we want it
			slopetype = POSITIVE;
		else if(line.vertex[0].y > line.vertex[1].y)		//Negative, no need to swap
			slopetype = NEGATIVE;
		else												//Vertical line
			slopetype = HORIZONTAL;
	}
	else if(line.vertex[0].x > line.vertex[1].x)	//Second region - we need to swap the ends of the line
	{
		if(line.vertex[0].y < line.vertex[1].y)			//Positive slope, but the ends are switched over
			slopetype = NEGATIVE;
		else if(line.vertex[0].y > line.vertex[1].y)		//Negative, we still need to swap the ends
			slopetype = POSITIVE;
		else												//Vertical line
			slopetype = HORIZONTAL;
		SwapLineEnds(&line);
	}
	else
	{
		slopetype = VERTICAL;
		if(line.vertex[0].y > line.vertex[1].y) SwapLineEnds(&line);
	}

	//Set the start point
	x = line.vertex[0].x;
	y = line.vertex[0].y;

	dx = line.vertex[1].x - line.vertex[0].x;
	dy = line.vertex[1].y - line.vertex[0].y;

	//dbout<<line.vertex[0].x<<", "<<line.vertex[0].y<<" -> "<<line.vertex[1].x<<", "<<line.vertex[1].y<<" sl "<<slopetype<<" dy/dx = "<<dy/dx<<endl;
	//Do the looping and testing depending on the type and magnitude of the slope
	switch(slopetype)
	{
		case POSITIVE:
			dx = line.vertex[1].x - line.vertex[0].x;
			dy = line.vertex[1].y - line.vertex[0].y;
			dydx = dy/dx;
			if(dydx<=1.0)	//Less than 45 degrees
			{
				while(x<line.vertex[1].x)		//Loop until we go past the end of the line in x
				{
					gridx = (int)(x*ONEOVERGRIDSIZE);
					gridy = (int)(y*ONEOVERGRIDSIZE);
					thisgrid = gridx + gridy*level.gridsize.x;
					if(LineGridBoundaries(&line, thisgrid))	return true;
					x += GRIDSIZEF;
					y += dydx*GRIDSIZEF;
				}
			}
			else		//Greater than 45 degrees
			{
				dydx = dx/dy;
				while(y<line.vertex[1].y)		//Loop until we go past the end of the line in y
				{
					gridx = (int)(x*ONEOVERGRIDSIZE);
					gridy = (int)(y*ONEOVERGRIDSIZE);
					thisgrid = gridx + gridy*level.gridsize.x;
					if(LineGridBoundaries(&line, thisgrid))	return true;
					x += dydx*GRIDSIZEF;
					y += GRIDSIZEF;
				}
			}
			break;
		case NEGATIVE:
			dx = line.vertex[1].x - line.vertex[0].x;
			dy = line.vertex[1].y - line.vertex[0].y;
			dydx = dy/dx;
			if(ABS(dydx)<=1.0)	//Less than 45 degrees
			{
				while(x<line.vertex[1].x)		//Loop until we go past the end of the line in x
				{
					gridx = x*ONEOVERGRIDSIZE;
					gridy = y*ONEOVERGRIDSIZE;
					thisgrid = gridx + gridy*level.gridsize.x;
					if(LineGridBoundaries(&line, thisgrid))	return true;
					x += GRIDSIZEF;
					y += dydx*GRIDSIZEF;
				}
			}
			else			//Greater than 45 degrees
			{
				dydx = dx/dy;
				while(y>line.vertex[1].y)		//Loop until we go past the end of the line in y
				{
					gridx = x*ONEOVERGRIDSIZE;
					gridy = y*ONEOVERGRIDSIZE;
					thisgrid = gridx + gridy*level.gridsize.x;
					if(LineGridBoundaries(&line, thisgrid)) return true;
					x -= dydx*GRIDSIZEF;
					y -= GRIDSIZEF;
				}
			}
			break;
		case HORIZONTAL:
			while(x<line.vertex[1].x)		//Loop until we go past the end of the line in x
			{
				gridx = x*ONEOVERGRIDSIZE;
				gridy = y*ONEOVERGRIDSIZE;
				thisgrid = gridx + gridy*level.gridsize.x;
				if(LineGridBoundaries(&line, thisgrid)) return true;
				x += GRIDSIZEF;
			}
			break;
		case VERTICAL:
			while(y<line.vertex[1].y)		//Loop until we go past the end of the line in x
			{
				gridx = x*ONEOVERGRIDSIZE;
				gridy = y*ONEOVERGRIDSIZE;
				thisgrid = gridx + gridy*level.gridsize.x;
				if(LineGridBoundaries(&line, thisgrid)) return true;
				y += GRIDSIZEF;
			}
			break;
	}

	return false;
}


/* Not very fast */

bool LineBoundariesCollisionOLD(tPolygon *line)
{
	//Collision detect with boundaries
	for(int i=0;i<level.boundaries;i++)
	{
		if(level.boundary[i].state == ON)
			if(LinePolygonIntersection(line,&level.boundary[i].poly)) return true;
	}
	return false;
}

/**/

//Check to see whether a position is clear to within 64 units radius
bool CheckTarget(t_vector position)
{
	list<GameObject>::iterator i = entity.begin();
	float dx, dy, distance;

	WrapToLevel(&position);

	int gridx = (int)(position.x*ONEOVERGRIDSIZE);
	int gridy = (int)(position.y*ONEOVERGRIDSIZE);
	int gridnumber = gridx + gridy*level.gridsize.x;

	GridSquare *square = &level.grid[gridnumber];
	for(int i=0;i<square->collideobjects.size();++i)
	{
		if(square->collideobjects[i]->type<ENEMY || square->collideobjects[i]->type>TURRET) continue;
		dx = square->collideobjects[i]->position.x - position.x;
		dy = square->collideobjects[i]->position.y - position.y;
		distance = dx*dx + dy*dy;
		if(distance<4096.0) return false;
	}

	//Loop over 8 adjacent grid squares
	for(int j=0; j<8; ++j)
	{
		square = level.grid[gridnumber].neighbour[j];
		for(int i=0;i<square->collideobjects.size();++i)
		{
			if(square->collideobjects[i]->type<ENEMY || square->collideobjects[i]->type>TURRET) continue;
			dx = square->collideobjects[i]->position.x - position.x;
			dy = square->collideobjects[i]->position.y - position.y;
			distance = dx*dx + dy*dy;
			if(distance<4096.0) return false;
		}
	}

	return true;
}


void GridPosition(GameObject *object)
{
	int gridx, gridy;

	WrapToLevel(&object->position);

	gridx = (int)(object->position.x*ONEOVERGRIDSIZE);
	gridy = (int)(object->position.y*ONEOVERGRIDSIZE);
	object->gridnumber = gridx + gridy*level.gridsize.x;

	object->gridx = gridx;
	object->gridy = gridy;
}

bool BoundaryCollisionRoutine(GameObject *object, int count)
{
	int counter = 0;
	float newangle, da, nca, nsa, velmag = 0;
	bool safe = false, changed = false;
	t_vector newposition, dposition;

	if(count==2) return false;	//We're too deep into recursion; exit as we don't need any more accuracy than this

	da = -object->changeangle*0.2f;				//Change in angle per reverse integration step
	dposition.x = -object->velocity.x * 0.2f;	//Change in position
	dposition.y = -object->velocity.y * 0.2f;	//Change in position

	newangle = object->angle + object->changeangle;
	newposition.x = object->position.x + object->velocity.x;
	newposition.y = object->position.y + object->velocity.y;

	while(safe==false)
	{
			Fsincos_t sincos = fsincos(newangle);
			nca = sincos.cosine;
			nsa = -sincos.sine;
			//nsa = -sin(newangle);
			//nca = cos(newangle);


		WrapToLevel(&newposition);
		RotateOutline(&object->outline, &object->rotatedoutline, nca, -nsa);
		TranslateOutline(&object->rotatedoutline, &object->predictedoutline, &newposition);
		GridPosition(object);

		//If we've found a place that doesn't cause a collision:
		if(BoundaryCollisionDetect(object) != true)
		{
			safe = true;
			//Update variables
			object->angle = newangle;
			object->position = newposition;
			object->cosangle = nca;
			object->sinangle = nsa;
			if(object->behaviouroncollision!=BOUNCE)	//Turrets, enemies, drones
			{
				object->unitvectorforward.x = -nsa;		//This is opposite for enemies and players :o(
				//if(object->type==BULLET && !CHECK(object->state,ENEMYBULLET)) object->unitvectorforward.x = nsa;
				object->unitvectorforward.y = -nca;
			}
			if(changed)		//If we had to modify the object's position
			{
				//Work out how much of the velocity vector remains - don't bother with small amounts
				if(velmag<0.2) break;

				//Find which line is closest to the colliding point
				tPolygon line(2);
				int poly;
				t_vector point;
				FindClosestLine(&object->predictedoutline,&line,newposition,object->gridnumber,&poly, &point);

				object->velocity.x = object->unitvectorforward.x*velmag;
				object->velocity.y = object->unitvectorforward.y*velmag;

				switch(object->behaviouroncollision)
				{
				case BOUNCE:
					//Update object's velocity
					BounceVector(&line,&object->velocity);
					BounceVector(&line,&object->unitvectorforward);
					break;
				case SLIDE:
					//Reverse remaining vector away from line if it's long enough
					BounceVector(&line,&object->velocity);
					BounceVector(&line,&object->unitvectorforward);
					break;
				case ABSORB:
				case STOP:
					//Let the object be absorbed/explode/die
					velmag = 0;
					//Object stops in its tracks
					object->unitvectorforward.x = 0;
					object->unitvectorforward.y = 0;
					return true;
				}
				line.Clear();
				//Send it all back through this function
				if(velmag>0) BoundaryCollisionRoutine(object, ++count);
				return true;
			}
			return false;
		}
		else
		{
			newangle += da;
			newposition.x += dposition.x;
			newposition.y += dposition.y;
			velmag += 0.2f;
			changed = true;
		}

		if((counter++)>4) break;
	}

	return true;
}

//Reverse the integration until a free position is found
float FindEarliestFreePosition(GameObject *object, float fraction)
{
	float increment = 0.25;

	do
	{
		object->angle -= object->changeangle*fraction;
		object->position.x -= object->velocity.x*fraction;
		object->position.y -= object->velocity.y*fraction;
		GridPosition(object);
		if(!CHECK(object->state,POINTDETECTION))
		{
			if(object->changeangle!=0)
			{
				Fsincos_t sincos = fsincos(object->angle);
				object->cosangle = sincos.cosine;
				object->sinangle = -sincos.sine;
				RotateOutline(&object->outline, &object->rotatedoutline, object->cosangle, -object->sinangle);
			}
			TranslateOutline(&object->rotatedoutline, &object->predictedoutline, &object->position);
		}
		if(BoundaryCollisionDetect(object) == false) return fraction;
		fraction -= increment;
	} while(fraction>0.2);
	return 0;
}

t_vector DoCollisionBehaviour(GameObject *object)
{
	int poly = 0;
	bool pointdetect = CHECK(object->state,POINTDETECTION)?true:false;
	t_vector collisionpoint;

	//Find which line is closest to the colliding point
	tPolygon line(2);
	if(pointdetect)
	{
		GridPointFindClosestLine(&line, &object->position, object->gridnumber);
		collisionpoint = object->position;
	}
	else FindClosestLine(&object->predictedoutline,&line,object->position,object->gridnumber, &poly, &collisionpoint);

	if(object->type != ENEMY)	//Enemies don't change orientation on collisions
		BounceVector(&line,&object->unitvectorforward);		//Bullets need to flip when they bounce

	object->shiftspeed = 0;
	object->shiftvector.x = 0;
	object->shiftvector.y = 0;

	//Reverse remaining velocity vector away from line
	t_vector normal = BounceVector(&line,&object->velocity);
	t_vector n;
	n.x = object->position.x - collisionpoint.x;
	n.y = object->position.y - collisionpoint.y;

	if(pointdetect)
	{
		object->position.x -= normal.x;
		object->position.y -= normal.y;
	}
	else if(n.x*normal.x + n.y*normal.y >= 0)
	{
		object->position.x += normal.x;
		object->position.y += normal.y;
	}
	else
	{
		object->position.x -= normal.x;
		object->position.y -= normal.y;
	}

	if(object->type!=BULLET) CreateDebris(collisionpoint,100.0f,1.0f,5.0f,1.0f,0,0,&spark);

	line.Clear();
	return normal;
}

void IntegrateObject(GameObject *object, float fraction)
{
	if(fraction<0.01) return;
	object->position.x += fraction*object->velocity.x;
	object->position.y += fraction*object->velocity.y;
	GridPosition(object);
	if(!CHECK(object->state,POINTDETECTION))
	{
		if(object->changeangle!=0)
		{
			object->angle += fraction*object->changeangle;
			Fsincos_t sincos = fsincos(object->angle);
			object->cosangle = sincos.cosine;
			object->sinangle = -sincos.sine;
			//object->cosangle = cos(object->angle);
			//object->sinangle = -sin(object->angle);
			RotateOutline(&object->outline, &object->rotatedoutline, object->cosangle, -object->sinangle);
		}
		TranslateOutline(&object->rotatedoutline, &object->predictedoutline, &object->position);
	}

	//Only enemies, as they have to rotate to point in the direction they are moving in
	if(object->type==ENEMY)
	{
		object->unitvectorforward.x = -object->sinangle;
		object->unitvectorforward.y = -object->cosangle;
	}

}



bool NewBoundaryCollisionRoutine(GameObject *object, int count, float fraction)
{
	if(count==2) return false;	//We're too deep into recursion; exit as we don't need any more accuracy than this

	//Do integration
	IntegrateObject(object, fraction);

	//If there's no collision, that's fine
	if(BoundaryCollisionDetect(object) == false) return false;

	//Bullets that don't bounce return immediately, for destruction - reversing integration is pointless
	if(object->behaviouroncollision==ABSORB || object->behaviouroncollision==STOP) return true;

	//Reverse the integration - if we can't find a free point then return
	fraction = FindEarliestFreePosition(object, fraction);
	if(fraction == 0) return true;

	//Do the behaviour
	t_vector normal = DoCollisionBehaviour(object);

	//Check again recursively
	NewBoundaryCollisionRoutine(object, ++count, fraction);

	return true;
}

float GridPointFindClosestLine(tPolygon *output, t_vector *position, int gridnumber)
{
	int i, j, next;
	float distance, mindistance = 1.0e11f;
	tPolygon store(2);
	t_vector point;

	//Make sure we are outputting a line
	if(output->points!=2) output->SetPoints(2);

	//Collision detect with boundaries
	for(i=0;i<level.grid[gridnumber].boundaries;++i)
	{
		if(level.boundary[level.grid[gridnumber].boundary[i]].state == ON)
		{
			//Check the point with every line in boundary
			next = 1;
			for(j=0;j<level.boundary[level.grid[gridnumber].boundary[i]].numpoints;++j,++next)		//Loop over LINES
			{
				if(next==level.boundary[level.grid[gridnumber].boundary[i]].numpoints) next = 0;
				//Check a LINE from poly2 with a POINT from poly1
				distance = PointLineSegmentDistanceSq(&(level.boundary[level.grid[gridnumber].boundary[i]].poly.vertex[j]), &(level.boundary[level.grid[gridnumber].boundary[i]].poly.vertex[next]), position, &point );
				if(distance<mindistance)
				{
					mindistance = distance;
					output->vertex[0] = level.boundary[level.grid[gridnumber].boundary[i]].poly.vertex[j];
					output->vertex[1] = level.boundary[level.grid[gridnumber].boundary[i]].poly.vertex[next];
				}
			}

		}
	}
//dbout<<"Result:"<<endl<<output->vertex[0].x<<" "<<output->vertex[0].y<<endl;
//dbout<<output->vertex[1].x<<" "<<output->vertex[1].y<<endl;
	return mindistance;
}

bool PlayerBoundaryCollision(int player)
{
		GameObject playerobject;
		playerobject.outline = ship[player].outline;
		playerobject.rotatedoutline = ship[player].rotatedoutline;
		playerobject.predictedoutline = ship[player].predictedoutline;
		playerobject.position = ship[player].position;
		GridPosition(&playerobject);
		return GridCollision(&playerobject, playerobject.gridnumber);
}

void KillShip(int player)
{
	ship[player].energy = 0;
	ship[player].state = DEAD;
	ship[player].lastminimissile = 0;
	ship[player].lastseeker = 0;
	ship[player].lastfired = 0;
	ship[player].lastlaunched = 0;
	ship[player].seekertarget = NULL;
	CreateExplosion(ship[player].position,5,NONE);
	CreateExplosion(ship[player].position,4,NONE);
	CreateExplosion(ship[player].position,3,NONE);
	CreateExplosion(ship[player].position,2,NONE);
	if(ship[player].equipment[LASER_DRONE] && ship[player].drone!=NULL)
	{
		SET(ship[player].drone->state,FORDESTRUCTION);
		SET(ship[player].drone->state,CREATEEXPLOSION);
		ship[player].drone = NULL;
	}
	if(ship[player].equipment[LIGHTNING_DRONE] && ship[player].lightningdrone!=NULL)
	{
		SET(ship[player].lightningdrone->state,FORDESTRUCTION);
		SET(ship[player].lightningdrone->state,CREATEEXPLOSION);
		ship[player].lightningdrone = NULL;
	}
	if(ship[player].equipment[SHIELD_DRONE])
	{
		for(int m=0;m<3;m++)
			if(ship[player].shielddrone[m]!=NULL)
			{
				SET(ship[player].shielddrone[m]->state,FORDESTRUCTION);
				SET(ship[player].shielddrone[m]->state,CREATEEXPLOSION);
				ship[player].shielddrone[m] = NULL;
			}
	}
	//dbout<<"DEAD: P 0 "<<ship[0].state<<" P 1 "<<ship[1].state<<endl;
	if( ship[0].state==DEAD && (ship[1].state==NONE || ship[1].state==DEAD) )
	{
		gamemode = GAMEOVER;
		deadtimer = thistick;
		dbout<<"Both players dead: timer set to "<<deadtimer<<endl;
	}
	if(playmode==TWOPLAYER)
	{
		if(player==0) ship[1].screenmessage.AddMessage("Player 1 died! Collect energy to restore them");
		if(player==1) ship[0].screenmessage.AddMessage("Player 2 died! Collect energy to restore them");

	}
	soundeffect[PLAYEREXPLODE].SetSoundEffect(0);
	soundeffect[SP_ENERGY_ZERO].SetSoundEffect(0);
	if(soundeffect[ENERGYLOW].looping) soundeffect[ENERGYLOW].stop;
	dbout<<"Dead process finished"<<endl;
}