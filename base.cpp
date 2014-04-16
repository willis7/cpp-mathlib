//The base handling routines

#include "OR.h"

extern Player ship[2];
extern ofstream dbout;
extern Equipment equipdata[NUMEQUIPTYPES];
extern Weapon *weaponlist;
extern GraphicalObject	error, energybar, energybarframe, debris, smoke, missile, flame, redbullet, explosion, 
						font, menufont, whiteblob, messagescreen, interfacescreen, datascreen, buttonframe, buttong[6], meterframe, 
						meterpanel;
extern Mousetarget mousetarget;
extern SoundEffect soundeffect[NUMEFFECTS];
extern int thistick, numberofweapons, playmode;
extern float screenratio;
extern BaseLayout baselayout;
extern bool equipment[NUMEQUIPTYPES];

#define NUMSTART 8
#define NUMNOUNS 9
#define NUMBEGIN 7
#define NUMRESULT 11
#define NUMHARDWARE 6
#define NUMACTIONS 8
#define NUMMESSAGETYPES 17


string start[NUMSTART] = { "Attempting:","Warning:","Blocking:","Initialising:","Prepare:","Recommendation:","Testing:","Alert:" };
string nouns[NUMNOUNS] = { "message","protocol","command","data","file","connection","kernel","system","structure" };
string begin[NUMBEGIN] = { "hacking","testing","checking","initiating","blocking","loading","rebooting" };
string result[NUMRESULT] = { "failed","unsafe","succeeded","rejected","initiated","deleted","de-stabilised","selected","repaired","aligned","secured" };
string hardware[NUMHARDWARE] = { "turret","shield","system","scanner","laser","antenna" };
string action[NUMACTIONS] = { "activated","recharged","checked","online","offline","damaged","repaired","active" };

void Base::PrintMessage(char *message)
{
	int type = rand()%NUMMESSAGETYPES;

	switch(type)
	{
	case 0:
		sprintf_s(message,100,"%s %s %d: %s",begin[rand()%NUMBEGIN].c_str(),nouns[rand()%NUMNOUNS].c_str(),rand()%10,result[rand()%NUMRESULT].c_str());
		break;
	case 1:
		sprintf_s(message,100,"System: %s %s",nouns[rand()%NUMNOUNS].c_str(),result[rand()%NUMRESULT].c_str());
		break;
	case 2:
		sprintf_s(message,100,"%s ... %s: %s",start[rand()%NUMSTART].c_str(),nouns[rand()%NUMNOUNS].c_str(),result[rand()%NUMRESULT].c_str());
		break;
	case 3:
		sprintf_s(message,100,"%s : %s -- %s",nouns[rand()%NUMNOUNS].c_str(),begin[rand()%NUMBEGIN].c_str(),result[rand()%NUMRESULT].c_str());
		break;
	case 4:
		sprintf_s(message,100,"%s %s %s",nouns[rand()%NUMNOUNS].c_str(),nouns[rand()%NUMNOUNS].c_str(),result[rand()%NUMRESULT].c_str());
		break;
	case 5:
		sprintf_s(message,100,"%s - %s: %s",start[rand()%NUMSTART].c_str(),nouns[rand()%NUMNOUNS].c_str(),result[rand()%NUMRESULT].c_str());
		break;
	case 6:
		sprintf_s(message,100,"%s: %s.",start[rand()%NUMSTART].c_str(),result[rand()%NUMRESULT].c_str());
		break;
	case 7:
		sprintf_s(message,100,"--- %s %d ---",nouns[rand()%NUMNOUNS].c_str(),rand()%10);
		break;
	case 8:
		sprintf_s(message,100,"- %s -",result[rand()%NUMRESULT].c_str());
		break;
	case 9:
		sprintf_s(message,100,"---------",result[rand()%NUMRESULT].c_str());
		break;
	case 10:
		sprintf_s(message,100,"%s %s",begin[rand()%NUMBEGIN].c_str(),nouns[rand()%NUMNOUNS].c_str());
		break;
	case 11:
		sprintf_s(message,100,"%s %s",nouns[rand()%NUMNOUNS].c_str(),result[rand()%NUMRESULT].c_str());
		break;
	case 12:
		sprintf_s(message,100,":%ss %s",hardware[rand()%NUMHARDWARE].c_str(),action[rand()%NUMACTIONS].c_str());
		break;
	case 13:
		sprintf_s(message,100,"Caution: %s %s",hardware[rand()%NUMHARDWARE].c_str(),result[rand()%NUMACTIONS].c_str());
		break;
	case 14:
		sprintf_s(message,100,"%s %s",begin[rand()%NUMBEGIN].c_str(),hardware[rand()%NUMHARDWARE].c_str());
		break;
	case 15:
		sprintf_s(message,100,"%s\n",begin[rand()%NUMBEGIN].c_str());
		break;
	case 16:
		sprintf_s(message,100,"\n");
		break;
	default:
		sprintf_s(message,100,"\n");
		break;
	}
}

void Base::ProcessBase()
{
	if(ship[player].state != INBASE) return;

	switch(screen)
	{
	case HOME:		//Base screen
		button[0] = EXIT;
		button[1] = BLANK;
		button[2] = BLANK;
		button[3] = SHIP;
		button[4] = WEAPON;
		button[5] = EQUIP;
		BaseScreen();
		break;
	case SHIP:		//Ship screen
		button[0] = EXIT;
		button[1] = SELECT;
		button[2] = BLANK;
		button[3] = SHIP;
		button[4] = WEAPON;
		button[5] = EQUIP;
		ShipScreen();
		break;
	case WEAPON:		//Weapon screen
		button[0] = EXIT;
		button[1] = SELECT;
		button[2] = BLANK;
		button[3] = SHIP;
		button[4] = WEAPON;
		button[5] = EQUIP;
		WeaponScreen();
		break;
	case EQUIP:		//Equip screen
		button[0] = EXIT;
		button[1] = SELECT;
		button[2] = REMOVE;
		button[3] = SHIP;
		button[4] = WEAPON;
		button[5] = EQUIP;
		EquipScreen();
		break;
	}

	if(timeGetTime()-lasttick>15)
	{
		lasttick = timeGetTime();
		angle += 0.02f;
		if(angle>=TWOPI) angle -= TWOPI;
		frame = dummy.graphics.frames - (int)(dummy.graphics.frames * angle / TWOPI);
	}
	if(ship[player].keys.LEFT)
	{
		buttonselector = (buttonselector+5)%6;
		while(button[buttonselector]==BLANK) buttonselector = (buttonselector+5)%6;
		ship[player].keys.LEFT = false;
		ship[player].keys.RIGHT = false;
		soundeffect[MENUMOVE].SetSoundEffect(0);
	}
	if(ship[player].keys.RIGHT)
	{
		buttonselector = (buttonselector+1)%6;
		while(button[buttonselector]==BLANK) buttonselector = (buttonselector+1)%6;
		ship[player].keys.LEFT = false;
		ship[player].keys.RIGHT = false;
		soundeffect[MENUMOVE].SetSoundEffect(0);
	}

	return;
}

//Handle the operation of the home screen
void Base::BaseScreen()
{
	if(data->visited) sprintf_s(datastring,2048,"%s",data->txtrevisit);
	else sprintf_s(datastring,2048,"%s",data->txtarrive);

	//sprintf_s(descstring2,2048,"\0");
	
	if(playmode==SINGLEPLAYER)
	{
		char shieldstring[20];
		sprintf_s(descstring,2048,"Ship: %s\nEnergy: %.0f%%\nMissiles: %d\nMain: %s\nShields: %s%%\nPower Usage: %.0f%%\n\nCredits: %d",ship[player].name,100.0f*ship[player].energy/ship[player].maxenergy,ship[player].missiles,ship[player].weapon->name.c_str(),ship[player].equipment[SHIELD]?(sprintf_s(shieldstring,20,"%.0f",100.0f*ship[player].shields/ship[player].maxshields),shieldstring):"N/A",100.0f*ship[player].power/ship[player].maxpower,ship[player].credits);
		UINT m[6];
		m[0] = messageindex;
		for(int i=1;i<6;++i)
			m[i] = (m[0]+i)%6;
		if(newmessagetime<=thistick)
		{
			newmessagetime = thistick + rand()%800;
			PrintMessage(message[messageindex].txt);
			messageindex = (messageindex + 1)%6;
		}
		sprintf_s(descstring2,2048,"%s\n%s\n%s\n%s\n%s\n%s\n",message[m[0]].txt,message[m[1]].txt,message[m[2]].txt,message[m[3]].txt,message[m[4]].txt,message[m[5]].txt);
	}
	else
	{
		sprintf_s(descstring,2048," \0");
		sprintf_s(descstring2,2048," \0");
	}

	if(ship[player].keys.OPERATE)
	{
		ship[player].keys.OPERATE = false;
		soundeffect[MENUSELECT].SetSoundEffect(0);
		switch (button[buttonselector])
		{
		case EXIT:
			sprintf_s(messagestring,1024,"Leaving base ...");
			screen = HOME;
			LeaveBase();
			return;
		case SHIP:		//Ship screen
			ChangeScreen(SHIP);
			return;
		case WEAPON:		//Weapon screen
			ChangeScreen(WEAPON);
			return;
		case EQUIP:		//Equip screen
			ChangeScreen(EQUIP);
			return;
		}
	}
	return;
}

// ************ The ship handling screen ************ //
void Base::ShipScreen()
{
	char tempstring[200], temp2string[200];

	meter[0].value = dummy.maxpower;			//Ship power usage
	meter[0].maxvalue = MAXPOWERMETERVALUE;
	meter[0].type = MPOWER;
	meter[1].value = dummy.maxenergy;			//Ship energy
	meter[1].maxvalue = MAXENERGYMETERVALUE;
	meter[1].type = MENERGY;
	meter[2].value = dummy.forwardsspeed;			//Ship speed
	meter[2].maxvalue = MAXSPEEDMETERVALUE;
	meter[2].type = MSPEED;

	sprintf_s(descstring,2048,"%s",dummy.description);
	if(playmode==SINGLEPLAYER)
	{
		UINT m[6];
		m[0] = messageindex;
		for(int i=1;i<6;++i)
			m[i] = (m[0]+i)%6;
		if(newmessagetime<=thistick)
		{
			newmessagetime = thistick + rand()%800;
			PrintMessage(message[messageindex].txt);
			messageindex = (messageindex + 1)%6;
		}
		sprintf_s(descstring2,2048,"%s\n%s\n%s\n%s\n%s\n%s\n",message[m[0]].txt,message[m[1]].txt,message[m[2]].txt,message[m[3]].txt,message[m[4]].txt,message[m[5]].txt);
	}
	else sprintf_s(descstring2,2048," \0");

	sprintf_s(datastring,2048,"Ship: %s\nManufacturer: %s\nPower: %.1f\nEnergy: %.1f\nEngines: %d\nWeapon mounts: %d\nMissile capacity: %d \0",dummy.name,dummy.manufacturer,dummy.maxpower,dummy.maxenergy,dummy.engines,dummy.weaponmounts,dummy.maxmissiles);

	if(ship[player].keys.OPERATE)
	{
		ship[player].keys.OPERATE = false;
		soundeffect[MENUSELECT].SetSoundEffect(0);
		switch(button[buttonselector])
		{
		case EXIT:
			ChangeScreen(HOME);
			return;
		case SELECT:		//Select
			if(strcmp(data->ships[listselector].name,ship[player].name)==0)
			{
				soundeffect[MENUREJECT].SetSoundEffect(0);
				sprintf_s(messagestring,1024,"This is the same as your current ship!");
				break;
			}
			if(ship[player].credits>=dummy.cost)
			{
				ship[player].credits -= dummy.cost;

				if(ship[player].equipment[LASER_DRONE])
				{
					SET(ship[player].drone->state,FORDESTRUCTION);
					ship[player].drone = NULL;
				}
				if(ship[player].equipment[LIGHTNING_DRONE])
				{
					SET(ship[player].lightningdrone->state,FORDESTRUCTION);
					ship[player].lightningdrone = NULL;
				}
				if(ship[player].equipment[SHIELD_DRONE])
				{
					for(int i=0;i<3;i++)
					{
						SET(ship[player].shielddrone[i]->state,FORDESTRUCTION);
						ship[player].shielddrone[i] = NULL;
					}
				}
				for(int i=0;i<NUMEQUIPTYPES;i++)
					if(ship[player].equipment[i])
					{
						data->equipment[i] = true;
						ship[player].equipment[i] = false;
					}

				if(ship[player].missiles)
				{
					if(dummy.missilemount)
					{
						if(dummy.maxmissiles>=ship[player].missiles) dummy.missiles = ship[player].missiles;
						else dummy.missiles = dummy.maxmissiles;
					}
					else dummy.missiles = 0;
				}
				ship[player].power = 0;
				dummy.credits = ship[player].credits;
				strcpy_s(tempstring,50,ship[player].filename);	//Save file name of our ship
				strcpy_s(temp2string,50,ship[player].name);		//Save name of our ship
				DuplicatePlayer(&dummy,&ship[player]);
				ship[player].equipment[RADAR] = true;
				ship[player].equipment[SCANNER] = true;
				sprintf_s(messagestring,1024,"Changed to %s, charge %d",ship[player].name,ship[player].cost);
				//dbout<<"Current ship now "<<ship[player].name<<", ship in base "<<data->ships[listselector].name<<endl;
			}
			else
			{
				soundeffect[MENUREJECT].SetSoundEffect(0);
				sprintf_s(messagestring,1024,"Not enough credits - cost %d",dummy.cost);
			}
			break;
		case EQUIP:		//Equip screen
			ChangeScreen(EQUIP);
			return;
		case WEAPON:		//Weapon screen
			ChangeScreen(WEAPON);
			return;
		case SHIP:
			ChangeScreen(SHIP);
			return;
		}
	}
	if(ship[player].keys.UP)
	{
		ship[player].keys.UP = false;
		if(--listselector<0) listselector = data->numships-1;
		LoadDummy(data->ships[listselector].filename);
		//glyph.CreateGraphicalObject(dummy.blueprintfile,1);
		sprintf_s(messagestring,1024,"Cost: %d    Credits: %d",dummy.cost,ship[player].credits);
		soundeffect[MENUMOVE].SetSoundEffect(0);
	}
	else if(ship[player].keys.DOWN)
	{
		ship[player].keys.DOWN = false;
		if(++listselector==data->numships) listselector = 0;
		LoadDummy(data->ships[listselector].filename);
		//glyph.CreateGraphicalObject(dummy.blueprintfile,1);
		sprintf_s(messagestring,1024,"Cost: %d    Credits: %d",dummy.cost,ship[player].credits);
		soundeffect[MENUMOVE].SetSoundEffect(0);
	}

	return;
}

void Base::EquipScreen()
{
	int first, elist[NUMEQUIPTYPES], displaylist[7];
	selectbar = -1;

	sprintf_s(descstring2,2048,"\0");

	int items = 0;
	for(int i=1;i<NUMEQUIPTYPES;i++) if(equipment[i]) elist[items++] = i;

	if(ship[player].keys.UP)
	{
		ship[player].keys.UP = false;
		if(listselector>0)
		{
			--listselector;
			glyph.CreateGraphicalObject(equipdata[elist[listselector]].graphics.c_str(),1);
			soundeffect[MENUMOVE].SetSoundEffect(0);
		}
		else soundeffect[MENUREJECT].SetSoundEffect(0);
	}
	if(ship[player].keys.DOWN)
	{
		ship[player].keys.DOWN = false;
		if(listselector<(items-1))
		{
			++listselector;
			glyph.CreateGraphicalObject(equipdata[elist[listselector]].graphics.c_str(),1);
			soundeffect[MENUMOVE].SetSoundEffect(0);
		}
		else soundeffect[MENUREJECT].SetSoundEffect(0);
	}

	meter[0].value = ship[player].power;			//Ship power usage
	meter[0].maxvalue = ship[player].maxpower;
	if(ship[player].equipment[elist[listselector]])
	{
		meter[1].value = 0;			//Equipment power usage
		meter[1].maxvalue = ship[player].maxpower;
		meter[2].value = ship[player].power;			//Predicted power usage
		meter[2].maxvalue = ship[player].maxpower;
	}
	else
	{
		if(elist[listselector] == AUXILIARY_GENERATOR)
		{
			meter[1].value = 0;									//Equipment power usage
			meter[1].maxvalue = ship[player].maxpower-equipdata[elist[listselector]].power;
			meter[2].value = ship[player].power;			//Predicted power usage
			meter[2].maxvalue = ship[player].maxpower-equipdata[elist[listselector]].power;
		}
		else
		{
			meter[1].value = equipdata[elist[listselector]].power;			//Equipment power usage
			meter[1].maxvalue = ship[player].maxpower;
			meter[2].value = ship[player].power+equipdata[elist[listselector]].power;			//Predicted power usage
			meter[2].maxvalue = ship[player].maxpower;
		}
	}
	meter[0].type = MUSED;
	meter[1].type = MPOWER;
	meter[2].type = MTOTAL;

	if(listselector<=3) first = 0;		//First section
	else if(listselector>=items-4) first = items-7;	//Last section
	else first = listselector-3;		//Scrolling section

	if(items<8)
	{
		for(int i=0;i<items;i++)
		{
			if(i==listselector)
			{
				selectbar = i;
				sprintf_s(descstring,2048,"%s \0",equipdata[elist[i]].description.c_str());
				//DrawSmallString(EqDesc[list[i]],391,276,621,false);
			}
			displaylist[i] = elist[i];
		}
		if(items<7) for(int i=0;i<7-items;i++) displaylist[items+i] = -1;
	}
	else
	{
		for(int i=0;i<7;i++,first++)
		{
			if(first==listselector)
			{
				selectbar = i;
				sprintf_s(descstring,2048,"%s \0",equipdata[elist[first]].description.c_str());
				//DrawSmallString(EqDesc[elist[first]],391,276,621,false);
			}
			displaylist[i] = elist[first];
		}
	}

	sprintf_s(datastring,2048,"%s\n%s\n%s\n%s\n%s\n%s\n%s\0",
				displaylist[0]>-1?equipdata[displaylist[0]].name.c_str():"\0",
				displaylist[1]>-1?equipdata[displaylist[1]].name.c_str():"\0",
				displaylist[2]>-1?equipdata[displaylist[2]].name.c_str():"\0",
				displaylist[3]>-1?equipdata[displaylist[3]].name.c_str():"\0",
				displaylist[4]>-1?equipdata[displaylist[4]].name.c_str():"\0",
				displaylist[5]>-1?equipdata[displaylist[5]].name.c_str():"\0",
				displaylist[6]>-1?equipdata[displaylist[6]].name.c_str():"\0");


	if(ship[player].keys.OPERATE)
	{
		ship[player].keys.OPERATE = false;
		switch (button[buttonselector])
		{
		case EXIT:
			ChangeScreen(HOME);
			return;
		case SELECT:		//Select
			if(!ship[player].equipment[elist[listselector]])
			{
				if((ship[player].power + equipdata[elist[listselector]].power)<=ship[player].maxpower )
				{
					ship[player].equipment[elist[listselector]] = true;
					ship[player].power += equipdata[elist[listselector]].power;
					sprintf_s(messagestring,1024,"Added %s ",equipdata[elist[listselector]].name.c_str());

					if(elist[listselector] == BOMB) data->equipment[BOMB]--;
					if(elist[listselector] == LASER_DRONE)
					{
						CreateDrone(player);
					}
					if(elist[listselector] == LIGHTNING_DRONE)
					{
						CreateLightningDrone(player);
					}
					if(elist[listselector] == SHIELD_DRONE)
					{
						for(int i=0;i<3;i++) CreateShieldDrone(player,i);
					}
					if(elist[listselector] == AUXILIARY_GENERATOR)
					{
						ship[player].maxpower -= equipdata[AUXILIARY_GENERATOR].power;
						ship[player].power -= equipdata[AUXILIARY_GENERATOR].power;		//To reverse the addition above
					}
					if(elist[listselector] == SHIELD)
					{
						ship[player].shields = ship[player].maxshields;
					}
					soundeffect[MENUSELECT].SetSoundEffect(0);
					soundeffect[INSTALLITEM].SetSoundEffect(0);
				}
				else
				{
					sprintf_s(messagestring,1024,"Not enough power");
					soundeffect[MENUREJECT].SetSoundEffect(0);
				}
			}
			else
			{
				sprintf_s(messagestring,1024,"Equipment already installed");
				soundeffect[MENUREJECT].SetSoundEffect(0);
			}
			break;
		case REMOVE:		//Remove
			if(elist[listselector]==RADAR || elist[listselector]==SCANNER)
			{
				sprintf_s(messagestring,1024,"Cannot remove Radar or Scanner");
			}
			else if(elist[listselector]==AUXILIARY_GENERATOR && (ship[player].maxpower + equipdata[AUXILIARY_GENERATOR].power < ship[player].power)) sprintf_s(messagestring,1024,"Cannot remove generator: power overload!");
			else if(ship[player].equipment[elist[listselector]])
			{
				ship[player].equipment[elist[listselector]] = false;
				//data->equipment[elist[listselector]]++;
				ship[player].power -= equipdata[elist[listselector]].power;
				sprintf_s(messagestring,1024,"Removed %s",equipdata[elist[listselector]].name.c_str());
				if(elist[listselector] == LASER_DRONE)
				{
					SET(ship[player].drone->state,FORDESTRUCTION);
					ship[player].drone = NULL;
				}
				if(elist[listselector] == LIGHTNING_DRONE)
				{
					SET(ship[player].lightningdrone->state,FORDESTRUCTION);
					ship[player].lightningdrone = NULL;
				}
				if(elist[listselector] == SHIELD_DRONE)
				{
					SET(ship[player].shielddrone[0]->state,FORDESTRUCTION);
					SET(ship[player].shielddrone[1]->state,FORDESTRUCTION);
					SET(ship[player].shielddrone[2]->state,FORDESTRUCTION);
					ship[player].shielddrone[0] = NULL;
					ship[player].shielddrone[1] = NULL;
					ship[player].shielddrone[2] = NULL;
				}
				if(elist[listselector] == BOMB) data->equipment[BOMB]++;
				if(elist[listselector] == AUXILIARY_GENERATOR)
				{
					ship[player].maxpower += equipdata[AUXILIARY_GENERATOR].power;
					ship[player].power += equipdata[AUXILIARY_GENERATOR].power;		//To reverse the subtraction above
				}
				if(elist[listselector]==RADAR)
				{
					if(ship[player].equipment[SCANNER])
					{
						ship[player].equipment[SCANNER] = false;
						ship[player].power -= equipdata[SCANNER].power;
						sprintf_s(messagestring,1024,"Removed Radar and Scanner");
					}
				}
				soundeffect[MENUSELECT].SetSoundEffect(0);
			}
			else
			{
				sprintf_s(messagestring,1024,"Equipment not installed");
				soundeffect[MENUREJECT].SetSoundEffect(0);
			}
			return;
		case EQUIP:		//Equip screen
			ChangeScreen(EQUIP);
			return;
		case WEAPON:		//Weapon screen
			ChangeScreen(WEAPON);
			return;
		case SHIP:
			ChangeScreen(SHIP);
			return;
		}
	}

	return;
}

void Base::WeaponScreen()
{
	int first, items = 0, *elist, displaylist[7];
	bool available = false;
	selectbar = -1;
	elist = new int[numberofweapons];

	//dbout<<listselector<<endl;
	sprintf_s(descstring,2048,"\0");
	sprintf_s(descstring2,2048,"\0");

	//Make list of weapons available for selection
	for(int i=0;i<numberofweapons;i++) if(weaponlist[i].available) elist[items++] = i;

	if(ship[player].keys.UP)		//Move the selector bar with arrow keys
	{
		ship[player].keys.UP = false;
		if(listselector>0)
		{
			--listselector;
			soundeffect[MENUMOVE].SetSoundEffect(0);
		}
		else soundeffect[MENUREJECT].SetSoundEffect(0);
	}
	if(ship[player].keys.DOWN)
	{
		ship[player].keys.DOWN = false;
		if(listselector<(items-1))
		{
			++listselector;
			soundeffect[MENUMOVE].SetSoundEffect(0);
		}
		else soundeffect[MENUREJECT].SetSoundEffect(0);
	}


	if(listselector<=3) first = 0;		//First section
	else if(listselector>items-5) first = items-7;	//Last section
	else first = listselector-3;		//Scrolling section

	if(items<8)
	{
		for(int i=0;i<items;i++)		//Fit all items onto one list and move bar up and down
		{
			if(i==listselector)
			{
				sprintf_s(descstring,2048,"%s",weaponlist[elist[listselector]].description.c_str());
				sprintf_s(descstring2,2048,"Power: %.1f MW\nFire Rate: %.1f /sec\nRange: %d\nRebound: %s",weaponlist[elist[listselector]].power,1000.0f/weaponlist[elist[listselector]].firerate,(int)(weaponlist[elist[listselector]].speed/weaponlist[elist[listselector]].faderate),weaponlist[elist[listselector]].bounce?"yes":"no");
				//if ship power			-	power used by current weapon							+	proposed weapon power				>	maximum power available
				if( (ship[player].power - (ship[player].weapon!=NULL?ship[player].weapon->power:0) + weaponlist[elist[listselector]].power) > ship[player].maxpower)
				{	//Not available
					sprintf_s(messagestring,1024,"Insufficient power for this weapon");
				}
				else
				{	sprintf_s(messagestring,1024,"Power required: %.1lf",weaponlist[elist[listselector]].power);
					available = true;
				}
				selectbar = i;
			}
			displaylist[i] = elist[i];
		}
		if(items<7) for(int i=0;i<7-items;i++) displaylist[items+i] = -1;
	}
	else						//Put items into long list and scroll list
	{
		for(int i=0;i<7;i++,first++)
		{
			if(first==listselector)
			{
				sprintf_s(descstring,2048,"%s",weaponlist[elist[listselector]].description);
				//if ship power			-	power used by current weapon							+	proposed weapon power				>	maximum power available
				if( (ship[player].power - (ship[player].weapon!=NULL?ship[player].weapon->power:0) + weaponlist[elist[listselector]].power) > ship[player].maxpower)
				{	//Not available
					sprintf_s(messagestring,1024,"Insufficient power for this weapon");
				}
				else
				{	sprintf_s(messagestring,1024,"Power required: %.1lf",weaponlist[elist[listselector]].power);
					available = true;
				}
				selectbar = i;
			}
			displaylist[i] = elist[first];
		}
	}

	sprintf_s(datastring,2048,"%s\n%s\n%s\n%s\n%s\n%s\n%s\0",
		displaylist[0]>-1?weaponlist[displaylist[0]].name.c_str():"\0",
		displaylist[1]>-1?weaponlist[displaylist[1]].name.c_str():"\0",
		displaylist[2]>-1?weaponlist[displaylist[2]].name.c_str():"\0",
		displaylist[3]>-1?weaponlist[displaylist[3]].name.c_str():"\0",
		displaylist[4]>-1?weaponlist[displaylist[4]].name.c_str():"\0",
		displaylist[5]>-1?weaponlist[displaylist[5]].name.c_str():"\0",
		displaylist[6]>-1?weaponlist[displaylist[6]].name.c_str():"\0");


	meter[0].value = ship[player].power;			//Ship power usage
	meter[0].maxvalue = ship[player].maxpower;
	if(ship[player].weapon == &weaponlist[elist[listselector]])		//If this is the current weapon
	{
		meter[1].value = ship[player].weapon->power;			//Equipment power usage
		meter[1].maxvalue = ship[player].maxpower;
		meter[2].value = ship[player].power;			//Predicted power usage
		meter[2].maxvalue = ship[player].maxpower;
	}
	else
	{
		meter[1].value = weaponlist[elist[listselector]].power;			//Equipment power usage
		meter[1].maxvalue = ship[player].maxpower;
		meter[2].value = ship[player].power - ship[player].weapon->power + weaponlist[elist[listselector]].power;			//Predicted power usage
		meter[2].maxvalue = ship[player].maxpower;
	}
	meter[0].type = MUSED;
	meter[1].type = MPOWER;
	meter[2].type = MTOTAL;

	if(ship[player].keys.OPERATE)
	{
		ship[player].keys.OPERATE = false;
		switch (button[buttonselector])
		{
		case EXIT:
			soundeffect[MENUSELECT].SetSoundEffect(0);
			delete [] elist;
			ChangeScreen(HOME);
			return;
		case SELECT:		//Select
			if(available)
			{	soundeffect[MENUSELECT].SetSoundEffect(0);
				soundeffect[INSTALLITEM].SetSoundEffect(0);
				ship[player].power -= ship[player].weapon->power;
				ship[player].weapon = &weaponlist[elist[listselector]];
				ship[player].power += ship[player].weapon->power;
				sprintf_s(messagestring,1024,"Installed a %s",weaponlist[elist[listselector]].name.c_str());
			}
			else
			{	soundeffect[MENUREJECT].SetSoundEffect(0);
				//sprintf_s(messagestring,1024,"Insufficient power for this weapon");
			}
			delete [] elist;
			return;
		case EQUIP:		//Equip screen
			delete [] elist;
			ChangeScreen(EQUIP);
			return;
		case WEAPON:	//Weapon screen
			delete [] elist;
			ChangeScreen(WEAPON);
			return;
		case SHIP:		//Ship screen
			delete [] elist;
			ChangeScreen(SHIP);
			return;
		}
	}
	delete [] elist;
	return;
}


void Base::SetUpBase(BaseData *currentbase)
{
	//dbout<<"Setting up base"<<endl;
	lasttick = timeGetTime();
	angle = 0;
	storeangle = ship[player].angle;
	DuplicatePlayer(&ship[player],&dummy);
	screen = HOME;
	buttonselector = 0;
	listselector = 0;
	selectbar = -1;
	frame = 0;
	data = currentbase;
	messageindex = 0;
	newmessagetime = thistick;

	CreateLayout(player);

	for(int i=0;i<3;i++)
	{
		meter[i].x = layout.meterposition[i].x;
		meter[i].y = layout.meterposition[i].y;
	}

	meter[0].maxvalue = MAXPOWERMETERVALUE;
	meter[0].value = dummy.maxpower;
	meter[0].type = MPOWER;
	meter[1].maxvalue = MAXENERGYMETERVALUE;
	meter[1].value = dummy.maxenergy;
	meter[1].type = MENERGY;
	meter[2].maxvalue = MAXSPEEDMETERVALUE;
	meter[2].value = dummy.forwardsspeed;
	meter[2].type = MSPEED;

	sprintf_s(messagestring,1024,"Landing charge %d",currentbase->landingcharge);
	soundeffect[SP_BASE_SYSTEMS_ACTIVATED].SetSoundEffect(0);
	//dbout<<"Base setup complete"<<endl;
}


void Base::LeaveBase()
{
	data->visited = true;
	ship[player].state = PLAY;
	ship[player].angle = storeangle;
	ship[player].cosangle = cos(ship[player].angle);
	ship[player].sinangle = sin(ship[player].angle);
	RotateOutline(&ship[player].outline, &ship[player].rotatedoutline, ship[player].cosangle, ship[player].sinangle);
	TranslateOutline(&ship[player].rotatedoutline, &ship[player].predictedoutline, &ship[player].position);
	ship[player].invulnerable = true;
	ship[player].invulnerabletimer = thistick;
	ship[player].keys.LEFT = false;
	ship[player].keys.RIGHT = false;
	ship[player].keys.UP = false;
	ship[player].keys.DOWN = false;
	if(ship[player].mousecontrol) mousetarget.ResetMousetarget();
	glyph.ClearGraphicalObject();
	soundeffect[SP_SYSTEMS_ACTIVATED].SetSoundEffect(0);
}

void Base::LoadDummy(char *shipname)
{
	LoadShip(&dummy,shipname);
}

void Base::ChangeScreen(int which)
{
	buttonselector = 1;
	selectbar = -1;
	listselector = 0;

	soundeffect[MENUSELECT].SetSoundEffect(0);

	switch(which)
	{
	case SHIP:
		sprintf_s(messagestring,1024,"Select your ship");
		DuplicatePlayer(&ship[player],&dummy);
		LoadDummy(data->ships[listselector].filename);
		glyph.ClearGraphicalObject();
		screen = SHIP;
		buttonselector = 1;
		//soundeffect[SP_SELECT_SHIP].SetSoundEffect(0);
		break;
	case EQUIP:
		sprintf_s(messagestring,1024,"Choose your equipment");
		DuplicatePlayer(&ship[player],&dummy);
		LoadDummy(ship[player].filename);
		glyph.CreateGraphicalObject(equipdata[RADAR].graphics.c_str(),1);
		//glyph.ClearGraphicalObject();
		screen = EQUIP;
		buttonselector = 1;
		for(int i=0;i<NUMEQUIPTYPES;++i)
		{
			if(ship[player].equipment[i]) equipment[i] = true;
			if(ship[(player+1)&1].equipment[i]) equipment[i] = true;
		}
		//soundeffect[SP_SELECT_EQUIPMENT].SetSoundEffect(0);
		break;
	case WEAPON:
		sprintf_s(messagestring,1024,"Choose your weapon");
		DuplicatePlayer(&ship[player],&dummy);
		LoadDummy(ship[player].filename);
		glyph.ClearGraphicalObject();
		screen = WEAPON;
		buttonselector = 1;
		//soundeffect[SP_SELECT_WEAPON].SetSoundEffect(0);
		break;
	case HOME:
		sprintf_s(messagestring,1024,"Awaiting orders ...");
		DuplicatePlayer(&ship[player],&dummy);
		LoadDummy(ship[player].filename);
		glyph.ClearGraphicalObject();
		screen = HOME;
		buttonselector = 0;
		break;
	}
	return;
}

void Base::CreateLayout(int player)
{
	ship[player].scrtopleft.x += screenratio*30.0f;

	layout.interfacescreenposition.x = ship[player].scrtopleft.x + screenratio*(float)(interfacescreen.framewidth>>1);
	layout.interfacescreenposition.y = ship[player].scrtopleft.y + screenratio*(float)(interfacescreen.frameheight>>1);

	layout.interfacescreen2position.x = ship[player].scrtopleft.x + screenratio*(float)interfacescreen.framewidth*2.1f;
	layout.interfacescreen2position.y = layout.interfacescreenposition.y;

	layout.interfacescreen3position.x = layout.interfacescreen2position.x;
	layout.interfacescreen3position.y = ship[player].scrtopleft.y + screenratio*(float)(5 + interfacescreen.frameheight + (interfacescreen.frameheight>>1));

	layout.datascreenposition.x = ((float)ship[player].scrtopleft.x + screenratio*(float)(datascreen.framewidth>>1));
	layout.datascreenposition.y = ((float)ship[player].scrtopleft.y + screenratio*310.0f);

	layout.panelposition.x = ((float)ship[player].scrtopleft.x + screenratio*(229.0f + 67.0f));
	layout.panelposition.y = screenratio*110.0f;

	layout.messagescreenposition.x = ((float)ship[player].scrtopleft.x + screenratio*(float)(messagescreen.framewidth>>1));
	layout.messagescreenposition.y = ((float)ship[player].scrtopleft.y + screenratio*420.0f);

	layout.logoposition.x = ((float)ship[player].scrtopleft.x + screenratio*(229.0f + 67.0f));
	layout.logoposition.y = screenratio*175.0f;

	layout.selectionbarposition.x = ((float)ship[player].scrtopleft.x + screenratio*180.0f);
	layout.selectionbarposition.y = screenratio*262.0f;

	layout.descstringposition.x = ((float)ship[player].scrtopleft.x + screenratio*25.0f);
	layout.descstringposition.y = ((float)ship[player].scrtopleft.y + screenratio*25.0f);
	layout.descstringlimit = ((float)ship[player].scrtopleft.x + screenratio*210.0f);

	layout.descstring2position.x = ((float)ship[player].scrtopleft.x + screenratio*390.0f);
	layout.descstring2position.y = ((float)ship[player].scrtopleft.y + screenratio*260.0f);
	layout.descstring2limit = ((float)ship[player].scrtopleft.x + screenratio*580.0f);

	layout.datastringposition.x = ((float)ship[player].scrtopleft.x + screenratio*25.0f);
	layout.datastringposition.y = ((float)ship[player].scrtopleft.y + screenratio*262.0f);
	layout.datastringlimit = ((float)ship[player].scrtopleft.x + screenratio*340.0f);

	layout.messagestringposition.x = ((float)ship[player].scrtopleft.x + screenratio*25.0f);
	layout.messagestringposition.y = ((float)ship[player].scrtopleft.y + screenratio*420.0f);
	layout.messagestringlimit = ((float)ship[player].scrtopleft.x + screenratio*340.0f);


	layout.bposition[0].x = ((float)ship[player].scrtopleft.x + screenratio*56.0f);
	layout.bposition[0].y = (float)ship[player].scrbottomright.y - screenratio*100.0f;
	layout.bposition[1].x = ((float)ship[player].scrtopleft.x + screenratio*176.0f);
	layout.bposition[1].y = (float)ship[player].scrbottomright.y - screenratio*100.0f;
	layout.bposition[2].x = ((float)ship[player].scrtopleft.x + screenratio*296.0f);
	layout.bposition[2].y = (float)ship[player].scrbottomright.y - screenratio*100.0f;

	layout.bposition[3].x = ((float)ship[player].scrtopleft.x + screenratio*56.0f);
	layout.bposition[3].y = (float)ship[player].scrbottomright.y - screenratio*38.0f;
	layout.bposition[4].x = ((float)ship[player].scrtopleft.x + screenratio*176.0f);
	layout.bposition[4].y = (float)ship[player].scrbottomright.y - screenratio*38.0f;
	layout.bposition[5].x = ((float)ship[player].scrtopleft.x + screenratio*296.0f);
	layout.bposition[5].y = (float)ship[player].scrbottomright.y - screenratio*38.0f;

	layout.meterposition[0].x = ((float)ship[player].scrtopleft.x + screenratio*(229.0f + 30.0f));
	layout.meterposition[0].y = screenratio*83.0f;
	layout.meterposition[1].x = ((float)ship[player].scrtopleft.x + screenratio*(229.0f + 67.0f));
	layout.meterposition[1].y = screenratio*83.0f;
	layout.meterposition[2].x = ((float) ship[player].scrtopleft.x + screenratio*(229.0f + 104.0f));
	layout.meterposition[2].y = screenratio*83.0f;

	ship[player].scrtopleft.x -= screenratio*30.0f;
}
