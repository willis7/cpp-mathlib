#include "OR.h"

extern Player ship[2];
extern ofstream dbout;
extern t_Tiledata *Tile;
extern vector<GraphicalObject> graphics;
extern BaseData basedata[10];
extern list<GameObject> entity;
extern GraphicalObject error, flame, *weapongraphics, *weaponpulsegfx, *weapontrails;
extern string nextlevel;
extern Weapon *weaponlist;
extern int playmode, gamemode, numberofweapons, thistick;
extern TileGraphics tilegfx;
extern SoundEffect soundeffect[NUMEFFECTS];
extern Mousetarget mousetarget;
extern float screenratio;
extern bool gamerunning;
extern Options options;
extern DX9VARS dx9;
extern Equipment equipdata[NUMEQUIPTYPES];
extern bool equipment[NUMEQUIPTYPES];

extern IGraphBuilder*	g_pGraphBuilder;
extern IMediaControl*	g_pMediaControl;
extern IMediaEventEx*	g_pMediaEvent;
extern IMediaPosition*	g_pMediaPosition;
extern IBasicAudio*		g_pBasicAudio;

bool LoadLevel(Level & level)
{	//list<GameObject>::iterator i;

	dbout<<"Reading "<<nextlevel<<endl;
	if(!ReadLevelFile(nextlevel.c_str(),level))
	{
		dbout<<"Level reading failure"<<endl;
		return false;
	}
	level.CreateMap();
	ReadObject(&entity.front(),"blue_gem");		//For adding gems when objects are destroyed
	CalculateGrid(level);
	LoadAssignGraphics();			//Assign graphics to each object
	CalculateNavigationMesh();		//Work out which points can see each other	
	PrepObjects(level);				//Do some geometric transformations and other initialisations

	if(options.PS2available)
	{
		dx9.rotozoom->SetTexture("tileTexture",tilegfx.tiles);
		dx9.rotozoom->SetTexture("mapTexture",level.map);

		ship[0].light = 1.0f;
		ship[1].light = 1.0f;
	}

	for(int j=0;j<playmode+1;j++)
	{
		if(ship[j].state != PLAY) ship[j].energy = ship[j].maxenergy;
		ship[j].state = PLAY;
		ship[j].screenalpha = 0xffffffff;
		ship[j].cosangle = cos(ship[j].angle);
		ship[j].sinangle = sin(ship[j].angle);
		ship[j].unitvectorforward.x = -ship[j].sinangle;
		ship[j].unitvectorforward.y = -ship[j].cosangle;
		ship[j].target = ship[j].position;
		RotateOutline(&ship[j].outline,&ship[j].rotatedoutline,ship[j].cosangle,ship[j].sinangle);
		TranslateOutline(&ship[j].rotatedoutline,&ship[j].predictedoutline,&ship[j].position);
		if(ship[j].equipment[LASER_DRONE]) CreateDrone(j);
		if(ship[j].equipment[LIGHTNING_DRONE]) CreateLightningDrone(j);
		if(ship[j].equipment[SHIELD_DRONE])
		{
			for(int i=0;i<3;i++) CreateShieldDrone(j,i);
		}
		ZeroMemory(&ship[j].keys,sizeof(Keystate));
		if(ship[j].mousecontrol) mousetarget.ResetMousetarget();
	}
	//for(int j=0;j<level.boundaries;j++) dbout<<"Bound "<<j<<" state "<<level.boundary[j].state<<endl;
	//dbout<<"Level read successfully"<<endl;

	MSG msg;
	ShowPrepareScreen();
	while(!TestKeyPressed())
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			// Check for a quit message
			if( msg.message == WM_QUIT )
				break;

			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	//char musicfile[50];
	//sprintf_s(musicfile,50,"i");
	//sprintf_s(musicfile,200,"%s",level.music);
	InitDirectShow();
	CreateGraph(level.music);
	g_pBasicAudio->put_Volume(soundfloattoint(options.musicvolume*0.9));
	g_pMediaControl->Run();
	ship[0].bombtimer = 0xffffffff;
	ship[1].bombtimer = 0xffffffff;
	return true;
}

bool LoadSavedGame(Level & level, bool restart)
{	//list<GameObject>::iterator i;

	//ReadPrefs();

	//If we want to restart the whole level
	if(restart)
	{
		//dbout<<"Reading \""<<(playmode==SINGLEPLAYER?"level1p.dat":"level2p.dat")<<"\""<<endl;
		if(!ReadSaveFile((playmode==SINGLEPLAYER?"level1p":"level2p"),level))
		{
			gamerunning = false;
			dbout<<"File reading failure"<<endl;
			gamemode = MENU;
			return false;
		}
	}
	else	//If the player died
	{
		//dbout<<"Reading \""<<(playmode==SINGLEPLAYER?"savefile1p.dat":"savefile2p.dat")<<"\""<<endl;
		if(!ReadSaveFile((playmode==SINGLEPLAYER?"savefile1p":"savefile2p"),level))
		{
			gamerunning = false;
			dbout<<"File reading failure"<<endl;
			gamemode = MENU;
			return false;
		}
	}
	level.CreateMap();
	ReadObject(&entity.front(),"blue_gem");		//For adding gems when objects are destroyed
	CalculateGrid(level);
	LoadAssignGraphics();			//Assign graphics to each object
	CalculateNavigationMesh();		//Work out which points can see each other
	PrepObjects(level);				//Do some geometric transformations and other initialisations

	if(options.PS2available)
	{
		dx9.rotozoom->SetTexture("tileTexture",tilegfx.tiles);
		dx9.rotozoom->SetTexture("mapTexture",level.map);

		ship[0].light = 1.0f;
		ship[1].light = 1.0f;
	}

	for(int j=0;j<playmode+1;j++)
	{
		ship[j].state = PLAY;
		ship[j].cosangle = cos(ship[j].angle);
		ship[j].sinangle = sin(ship[j].angle);
		ship[j].unitvectorforward.x = -ship[j].sinangle;
		ship[j].unitvectorforward.y = -ship[j].cosangle;
		RotateOutline(&ship[j].outline,&ship[j].rotatedoutline,ship[j].cosangle,ship[j].sinangle);
		TranslateOutline(&ship[j].rotatedoutline,&ship[j].predictedoutline,&ship[j].position);
		if(ship[j].equipment[LASER_DRONE]) CreateDrone(j);
		if(ship[j].equipment[LIGHTNING_DRONE]) CreateLightningDrone(j);
		if(ship[j].equipment[SHIELD_DRONE])
		{
			for(int i=0;i<3;i++) CreateShieldDrone(j,i);
		}
		ship[j].keys.LEFT = false;
		ship[j].keys.RIGHT = false;
		ship[j].keys.UP = false;
		ship[j].keys.DOWN = false;
		if(ship[j].mousecontrol) mousetarget.ResetMousetarget();
	}

	//dbout<<"File read successfully"<<endl;

	MSG msg;
	ShowPrepareScreen();
	while(!TestKeyPressed())
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			// Check for a quit message
			if( msg.message == WM_QUIT )
				break;

			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	//dbout<<"Doing music"<<endl;
	InitDirectShow();
	CreateGraph(level.music);
	g_pMediaControl->Run();
	return true;
}


int ReadLevelFile(const char *filename, Level& level)
{
	using namespace std;
	int i, j, number, numbases = 0;
	std::ifstream::pos_type point, point2;
	char filenm[200], input_string[200], name[100];
	string junk, morejunk;
	ifstream fp_in;

	sprintf_s(filenm, 200, "levels\\%s.dat", filename);
	//dbout<<filenm<<endl;
	fp_in.open(filenm, ios::in);  // declare and open

	if(!fp_in.is_open()) return false;


// Read all global level data

	//Read name part ***************************************************************
	do
	{
		fp_in.getline( input_string, 200 );		//Read next line
	} while(strcmp(input_string,"<name>")!=0);
	fp_in.getline( level.name, 256 );	//Read level name
	//dbout<<"Level name "<<level.name<<endl;

	//Read description part ***************************************************************
	do
	{
		fp_in.getline( input_string, 200 );		//Read next line
	} while(strcmp(input_string,"<description>")!=0);
	fp_in.getline( level.description, 4096 );	//Read level name
	//dbout<<"Mission: "<<level.description<<endl;

	//Read the next level part *****************************************************
	do
	{
		fp_in.getline( input_string, 200 );		//Read next line
	} while(strcmp(input_string,"<next>")!=0);
	fp_in.getline( level.next, 256 );	//Read next level name
	//dbout<<"Next level file "<<level.next<<endl;

	//Read tile set part ************************************************************
	do
	{
		fp_in.getline( input_string, 200 );		//Read next line
	} while(strcmp(input_string,"<tileset>")!=0);

	fp_in.getline( level.tiles, 30 );	//Read tile set
	//dbout<<"Tile set "<<level.tiles<<endl;


	//Read tile map part ************************************************************
	do
	{
		fp_in.getline( input_string, 200 );		//Read next line
	} while(strcmp(input_string,"<tilemap>")!=0);

	fp_in>>junk>>level.width;					//Get width
	fp_in>>junk>>level.height;				//Get height
	level.maskx = level.width-1;
	level.masky = level.height-1;
	level.pixelwidth = level.width*TILESIZE;			//Width in pixels
	level.pixelheight = level.height*TILESIZE;			//Height in pixels
	level.fwidth = (float)level.pixelwidth;			//Full width
	level.fheight = (float)level.pixelheight;
	level.hwidth = level.fwidth/2.0f;				//Half width
	level.hheight = level.fheight/2.0f;
	level.mhwidth = -level.hwidth;					//Minus half width
	level.mhheight = -level.hheight;
	level.tilemap = new int[level.width*level.height];
	level.shiftleft = level.width;
	for(i=0;level.shiftleft>1;i++)
		level.shiftleft = level.shiftleft>>1;
	level.shiftleft = i;

	for(j=0;j<level.height;j++)
		for(i=0;i<level.width;i++)
			fp_in>>level.tilemap[i + j*level.width];	//Read tile map

	//dbout<<"Width "<<level.width<<" Height "<<level.height<<endl;

	//Read player part ***********************************************************************
	do
	{
		fp_in.getline( input_string, 200 );		//Read next line
	} while(strcmp(input_string,"<player1pos>")!=0);
	fp_in>>level.start[0].x>>level.start[0].y>>ship[0].angle;		//Read ship starting position and angle
	ship[0].position = level.start[0];

	do
	{
		fp_in.getline( input_string, 200 );		//Read next line
	} while(strcmp(input_string,"<player2pos>")!=0);
	fp_in>>level.start[1].x>>level.start[1].y>>ship[1].angle;		//Read ship starting position and angle
	if(playmode!=TWOPLAYER)
	{
		level.start[1].x = -10000;
		level.start[1].y = -10000;
	}
	ship[1].position = level.start[1];

	//Read music **********************************************************************
	do
	{
		fp_in.getline( input_string, 200 );		//find music data
	} while(strcmp(input_string,"<music>")!=0);
	fp_in>>input_string;
	sprintf_s(level.music,256,"music\\%s.wma",input_string);

	//Read objects part ***************************************************************

	level.objectives = 0;

	// First time through -----------------------------------
	do {
		fp_in.getline( input_string, 200 );			//Read next line
	} while(strcmp(input_string,"<objects>")!=0);	//Find start of objects section

	point = fp_in.tellg();		//Store file position to return to later

	BaseData junkbasedata;
	numbases = 0;
	//Loop over all groups of objects
	do 
	{
		fp_in.getline( input_string, 200 );			//Read next line
		if(strcmp(input_string,"<group>")==0)
		{
			int current;
			fp_in>>number>>name;
//			#ifdef OR_DEBUG
//			dbout<<number<<" "<<name<<"s"<<endl;
//			#endif
			fp_in.ignore();				//Ignore newline in input buffer

			ReadObject(&entity.front(),name);

			current = entity.size();
			number += current;

			list<GameObject>::iterator it;
			for(i=current;i<number;i++)
			{
				streampos fileposition;

				entity.push_back(0);

				CopyObject(&entity.front(),&entity.back());

				it = entity.end();
				--it;
				it->objective = 0;
				fp_in>>junk>>it->position.x>>junk>>it->position.y>>junk>>it->angle;
				switch(it->type)
				{
					case PLAYER:
						break;
					case MINE:
						fp_in>>junk>>it->sensorrange>>junk>>it->attackrange>>junk>>it->objective;
						it->timer = (int)(it->attackrange*1000);						
					break;
					case SPAWNINERT:
						it->lastfired = 0;
					case INERT:
						fp_in>>junk>>it->turnspeed;
						it->turnspeed = it->turnspeed*TWOPI/360.0;
						break;
					case SPINNER:
					case TURRET:
						fp_in>>junk>>it->objective;
						break;
					case ENEMY:
						fp_in>>junk>>it->rangelimit>>junk>>it->objective;
						it->initialposition = it->position;
						if(it->rangelimit!=0)
						{
							if(it->rangelimit<11.0) SET(it->state,WAITING);
							else
							{
								it->rangelimit *= it->rangelimit;
								SET(it->state,RANGELIMITED);
							}
						}
						break;
					case BUTTONSWITCH:
						UNSET(it->state,ON);
						break;
					case TRIGGERSWITCH:
						fp_in>>junk>>morejunk>>junk>>it->timer;
						it->timer *= 1000;
						it->lastfired = 0;
						SET(it->state,ARMED);
						if(strcmp(morejunk.c_str(),"enemy")==0) SET(it->state,TRIGGEREDBYENEMY);
						else if(strcmp(morejunk.c_str(),"rock")==0) SET(it->state,TRIGGEREDBYROCK);
						else if(strcmp(morejunk.c_str(),"player")==0) SET(it->state,TRIGGEREDBYPLAYER);
						//dbout<<morejunk<<endl;
						break;
					case BASE:
						if(numbases==10) break;		//Not good code - probably will do something bad if more than ten
						fp_in>>junk;
						if(strcmp(junk.c_str(),"landingcharge")==0)
						{
							fp_in>>basedata[numbases].landingcharge>>junk>>basedata[numbases].cost;
						}
						else
						{
							basedata[numbases].landingcharge = 3;
							basedata[numbases].cost = 100;
						}
						do { fp_in>>junk; } while(strcmp(junk.c_str(),"<ships>")!=0);	//Get the start of the ships bit

						int readnumber; fp_in>>readnumber;
						//dbout<<"Read "<<readnumber<<endl;
						if(readnumber == 0)
						{
							basedata[numbases].numships = 1;		//Always available
							sprintf_s(basedata[numbases].ships[0].filename,200,"gambit_g550");
						}
						else
						{
							basedata[numbases].numships = readnumber;
							//dbout<<basedata[numbases].numships<<" ships"<<endl;

							for(int k=0;k<readnumber;k++)
							{
								fp_in>>basedata[numbases].ships[k].filename;
								//dbout<<numbases<<" "<<k<<" "<<basedata[numbases].ships[k].filename<<endl;
							}
						}

						do { fp_in>>junk; } while(strcmp(junk.c_str(),"<text1>"));	//Find start of first text
						fp_in.ignore();
						fp_in.getline(basedata[numbases].txtarrive,500);
						do { fp_in>>junk; } while(strcmp(junk.c_str(),"<text2>"));	//Find start of second text
						fp_in.ignore();
						fp_in.getline(basedata[numbases].txtrevisit,500);
						//dbout<<basedata.back().txtarrive<<endl<<basedata.back().txtrevisit<<endl;
						do { fp_in>>junk; } while(strcmp(junk.c_str(),"</contains>"));	//Find end of section
						it->link = numbases++;
						break;
					case EXITGATE:
						fp_in>>junk>>it->turnspeed>>junk>>it->storestring;	//Exit gate can spin, take player to bonus level
						it->turnspeed = it->turnspeed *(TWOPI/360.0f);
						break;
					case GAMESAVER:
					case SHIELDCHARGER:
						break;
					case TELEPORT:
						fp_in>>junk>>it->turnspeed>>junk>>it->link;	//May adjust this so that if link is zero, the teleporter can have a target position
						it->turnspeed = it->turnspeed *(TWOPI/360.0f);
						break;
					case SPAWN:
						it->lastfired = 0;
					case ROCK:
						fp_in>>junk>>it->unitvectorforward.x>>junk>>it->unitvectorforward.y>>junk>>it->turnspeed>>junk>>it->objective;
						it->turnspeed = it->turnspeed *(TWOPI/360.0f);
						it->forwardsspeed = Normalise(&(it->unitvectorforward));
						for(int loopy=0;loopy<it->childobjects;loopy++)
							if(it->childobject[loopy].type==TURRET && it->radarcolour==0xffff9855)
							{
								it->radarcolour = 0xffff0000;
								break;
							}
						break;
					case ENERGY:
						fp_in>>junk>>it->energy>>junk>>it->turnspeed;
						it->turnspeed = it->turnspeed *(TWOPI/360.0f);
						it->objective = 0;
						it->forwardsspeed = 0;
						break;
					case BLUEPRINT:
						fp_in>>junk>>it->turnspeed>>junk>>it->storestring;
						if(junk=="equipment")
						{
							it->link = GetEquipmentType(it->storestring.c_str());
							SET(it->state,STOREEQUIPMENT);
						}
						else if(junk=="weapon")
						{
							it->link = GetWeaponNumber(it->storestring.c_str());
							if(it->link<0) it->link = 0;
							SET(it->state,STOREWEAPON);
						}
						else it->link = 0;
						it->turnspeed = it->turnspeed *(TWOPI/360.0f);
						it->objective = 0;
						it->forwardsspeed = 0;
						break;
					case GEMSTONE:
					case MISSILECRATE:
						fp_in>>junk>>it->link>>junk>>it->turnspeed;
						it->turnspeed = it->turnspeed *(TWOPI/360.0f);
						it->objective = 0;
						it->forwardsspeed = 0;
						break;
				}
				if(it->type<TELEPORT || it->type==BUTTONSWITCH || it->type==TRIGGERSWITCH)
				{
					fileposition = fp_in.tellg();
					fp_in>>junk;
					if(strcmp(junk.c_str(),"setboundary")==0)
					{
						fp_in>>it->link;
						SET(it->state,LINKEDTOBOUNDARY);
						//dbout<<"     ------   "<<it->name<<" linked to boundary "<<it->link<<endl;
					}
					else if(strcmp(junk.c_str(),"fromboundary")==0)
					{
						fp_in>>it->link;
						SET(it->state,LINKEDFROMBOUNDARY);
						//dbout<<"     ------   "<<it->name<<" linked from boundary "<<it->link<<endl;
					}
					else fp_in.seekg(fileposition);
				}
				it->angle = it->angle*PI/180.0;
				it->cosangle = cos(it->angle);
				it->sinangle = sin(it->angle);
				ReSumChildAngles(&*it, it->angle);
				it->initialposition = it->position;
				it->target = it->position;
				it->graphics = &error;		//For safety
				it->changeangle = 0;
				it->velocity.x = 0;
				it->velocity.y = 0;
				it->scale = 1.0;
				it->scalerate = 0;
				if(it->objective) ++level.objectives;
			}

		}
		else if(strcmp(input_string,"</objects>")==0) break;
	} while(1);

	//dbout<<level.objectives<<" objectives"<<endl;

/*	case SPINNER:
		fp_in>>junk>>Host_Object[obj_counter].X_Pos>>junk>>Host_Object[obj_counter].Y_Pos>>junk>>Host_Object[obj_counter].angle>>junk>>Host_Object[obj_counter].objective;
		break;	*/

	point = fp_in.tellg();		//Store file position to return to later

	//Find how many sets of boundaries there are
	level.boundaries = 0;
	fp_in.ignore();
	do {
		fp_in.getline( input_string, 200 );		//Read next line
		if(strncmp(input_string,"<boundary>",11)==0)
			level.boundaries++;
		else if(strncmp(input_string,"</boundaries>",14)==0) break;
	} while(1);

	//dbout<<level.boundaries<<" boundaries with: ";

	if(level.boundaries>0)
	{
		int counter=0;
		level.boundary = new Boundary[level.boundaries];

		for(i=0;i<level.boundaries;i++)
			level.boundary[i].state = ON;

		//Now we return to the boundaries part of the file
		fp_in.seekg(point);

		do {
			fp_in.getline( input_string, 200 );		//Read next line
			if(strncmp(input_string,"<boundary>",11)==0)
			{
				fp_in>>junk>>level.boundary[counter].numpoints;
				//dbout<<level.boundary[counter].numpoints<<", ";
				level.boundary[counter].poly.SetPoints(level.boundary[counter].numpoints);
				for(i=0;i<level.boundary[counter].numpoints;i++)
					fp_in>>junk>>level.boundary[counter].poly.vertex[i].x>>junk>>level.boundary[counter].poly.vertex[i].y;

				fp_in.ignore();
				counter++;
			}
			else if(strncmp(input_string,"</boundaries>",14)==0 || fp_in.eof()) break;
		} while(1);
	}
	else level.boundary = NULL;

	fp_in.ignore();
	do {
		fp_in.getline( input_string, 200 );		//Read next line
		if(strncmp(input_string,"<mesh>",8)==0)
		{
			fp_in>>junk>>level.meshpoints;
			//dbout<<endl<<level.meshpoints<<" navigation mesh points"<<endl;
			level.SetMeshPoints();
			char actionname[20];
			for(i=0;i<level.meshpoints;i++)
			{
				fp_in>>junk>>level.mesh[i].position.x>>junk>>level.mesh[i].position.y>>actionname;
				_strupr(actionname);
				if(strncmp(actionname,"CONTINUE",20)==0) level.mesh[i].action = CONTINUE;
				if(strncmp(actionname,"WAIT",20)==0) level.mesh[i].action = WAIT;
				if(strncmp(actionname,"OPERATE",20)==0) level.mesh[i].action = OPERATE;
				level.mesh[i].number = i;
			}
			fp_in.ignore();
		}
		else if(strncmp(input_string,"</mesh>",8)==0 || fp_in.eof()) break;
	} while(1);
	//dbout<<"Level file read .. BO!"<<endl;
	fp_in.close();

	return tilegfx.LoadTiles(level.tiles);
	//return LoadTileSet(level.tiles, level);	//If this fails then everything fails
}

void DeleteLevel(Level& level)
{
	//dbout<<"Deleting level structure"<<endl;
	level.ClearLevel();
}

/*
bool LoadTileSet(char *filenm, Level& level)
{
	using namespace std;
	ifstream fp_in;
	int i, j, dat;
	char filename[30];
	unsigned char in;

	sprintf_s(filename,30,"graphics\\%s.raw",filenm);
	fp_in.open(filename, ios::in|ios::binary);  // declare and open
	if(!fp_in.is_open()) return false;

	fp_in.read((char *)&in,1);
	level.num_tiles = in;
	Tile = new t_Tiledata[(int)in];		//Allocate the array to store the objects

	for(i=0;i<in;i++)
	{
		fp_in.read((char *)Tile[i].data,TILESIZE*TILESIZE*3);

		for(j=TILESIZE*TILESIZE*4;j>=0;j-=4)		//Store into 32-bit data
		{
			dat = 3*(j>>2);		//3*j/4
			Tile[i].data[j+3] = 255;
			Tile[i].data[j+2] = Tile[i].data[dat];
			Tile[i].data[j+1] = Tile[i].data[dat+1];
			Tile[i].data[j] = Tile[i].data[dat+2];
		}

	}

	return true;
}*/


bool LoadShip(Player *inship, char *filename)
{
	if(!ReadShip(inship,filename)) return false;
	if(!inship->graphics.CreateGraphicalObject(inship->graphicsfile,inship->graphics.frames,inship->graphics.flatness)) return false;
	if(!inship->logographics.CreateGraphicalObject(inship->logo,1,1.0)) inship->logographics = NULL;
	if(!inship->shape)
	{
		inship->shape = true;
		inship->outline.Clear();
		CreatePolygonOutline(&inship->outline,(float)inship->graphics.framewidth, (float)inship->graphics.frameheight);
		inship->rotatedoutline.SetPoints(4);
		inship->predictedoutline.SetPoints(4);
		//dbout<<"Created outline for "<<inship->name<<endl;
	}
	inship->radiussquared = FindRadius(&inship->outline);

	inship->homingsweep.SetPoints(4);
	inship->homingsweeprotated.SetPoints(4);
	inship->homingsweep.vertex[0].x = -80.0;
	inship->homingsweep.vertex[0].y = -20.0;
	inship->homingsweep.vertex[1].x = -350.0;
	inship->homingsweep.vertex[1].y = -500.0;
	inship->homingsweep.vertex[2].x = 350.0;
	inship->homingsweep.vertex[2].y = -500.0;
	inship->homingsweep.vertex[3].x = 80.0;
	inship->homingsweep.vertex[3].y = -20.0;
	return true;
}


//Read in a ship file
bool ReadShip(Player *inship, char *filename)
{
	using namespace std;
	ifstream fp_in, shapefile_in;
	char filenm[100], shapefile[100], junk[100];

	sprintf_s(filenm,50,"ships\\%s.dat",filename);
	fp_in.open(filenm, ios::in);  // declare and open

	if(!fp_in.is_open()) return false;

	//dbout<<"Attempting to read ship "<<filename<<endl;

	fp_in.getline( inship->name, 50 );			//Read ship name
	fp_in.getline( inship->manufacturer, 50 );	//Read ship manufacturer's name
	fp_in.getline( inship->description, 300 );	//Read ship descriptive text
	fp_in.getline( inship->logo, 50 );			//Read ship logo
	strcpy_s(inship->filename,50,filename);

	fp_in>>junk>>inship->graphicsfile;
//	fp_in>>junk>>inship->blueprintfile;
	fp_in>>junk>>inship->graphics.flatness;
	fp_in>>junk>>inship->graphics.frames;
	fp_in>>junk>>inship->shape;
	//dbout<<"Read initial values"<<endl;

	inship->outline.Clear();
	inship->predictedoutline.Clear();
	inship->rotatedoutline.Clear();

	//dbout<<"Reading shape"<<endl;

	/* Read in the file with points defining the object's shape */
	if(inship->shape)
	{
		//Clear previous memory
		fp_in>>inship->shapefile;		//Read in name of file containing boundaries
		sprintf_s(shapefile,50,"bound\\%s.shape",inship->shapefile);
		shapefile_in.open(shapefile, ios::in);  // declare and open
		if(!shapefile_in.is_open())	//If opening failed
		{
			inship->shape = 0;		//No boundaries here
			inship->numpoints = 0;
			inship->outline.Clear();
			dbout<<"Could not read shape file "<<shapefile<<endl;
		}
		else
		{
			shapefile_in>>inship->numpoints;

			inship->outline.SetPoints(inship->numpoints);
			inship->rotatedoutline.SetPoints(inship->numpoints);
			inship->predictedoutline.SetPoints(inship->numpoints);

			for(int i=0;i<inship->numpoints;i++)
				shapefile_in>>inship->outline.vertex[i].x>>inship->outline.vertex[i].y;	//Read in list of points

			//dbout<<"Read "<<inship->numpoints<<" points"<<endl;
			shapefile_in.close();
		}
	}

	fp_in>>junk>>inship->strength;		//dbout<<"Read strength "<<inship->strength<<endl;
	fp_in>>junk>>inship->shields;		//dbout<<"Read shields "<<inship->shields<<endl;
	fp_in>>junk>>inship->maxshields;	//dbout<<"Read max shields "<<inship->maxshields<<endl;
	fp_in>>junk>>inship->energy;		//dbout<<"Read energy "<<inship->energy<<endl;
	fp_in>>junk>>inship->maxenergy;		//dbout<<""<<<<endl;
	fp_in>>junk>>inship->power;			//dbout<<""<<<<endl;
	inship->power = 0;
	inship->energy = inship->maxenergy;
	inship->shields = 0;
	fp_in>>junk>>inship->maxpower;
	fp_in>>junk>>inship->cost;
	fp_in>>junk>>inship->maxmissiles;	//dbout<<"Read missiles "<<inship->maxmissiles<<endl;
	if(inship->maxmissiles>0) inship->missilemount = true;
	else inship->missilemount = false;

	fp_in>>junk>>inship->forwardsspeed;		//Read speeds
	fp_in>>junk>>inship->backwardsspeed;
	fp_in>>junk>>inship->turnspeed; inship->turnspeed = inship->turnspeed * TWOPI/360.0f;

	fp_in>>junk>>inship->maxengine;
	fp_in>>junk>>inship->weaponmounts;
	for(int i=0;i<inship->weaponmounts;i++)
	{
		fp_in>>junk>>inship->weaponmount[i].x>>junk>>inship->weaponmount[i].y>>junk>>inship->weaponmount[i].angle;
		inship->weaponmount[i].y = -inship->weaponmount[i].y;
		inship->weaponmount[i].angle *= PI/180.0f;
	}
	fp_in>>junk>>inship->engines;
	for(int i=0;i<inship->engines;i++)
	{
		fp_in>>junk>>inship->engine[i].x>>junk>>inship->engine[i].y;
		inship->engine[i].y = -inship->engine[i].y;
	}

	fp_in.close();

	for(int j=0;j<NUMEQUIPTYPES;j++) inship->equipment[j] = false;
	inship->missiles = 0;
	inship->homingcomputerbusy = false;
	inship->tracking = false;

	//dbout<<"Finished reading ship"<<endl;
	return true;
}

bool ReadObject(GameObject *object, char *filename)
{
	using namespace std;
	float readaggression;
	ifstream fp_in, shapefile_in;
	ifstream::pos_type point;
	int i;
	char filenm[200], junk[200], temp[200];

	sprintf_s(filenm,200,"objects\\%s.dat",filename);

	fp_in.open(filenm, ios::in);  // declare and open
	if(!fp_in.is_open()) return false;

	object->origfilename = filename;		//Store filename for the save game routine

	fp_in>>junk>>temp;
	object->type = GetType(temp);
	switch(object->type)
	{
	case GAMESAVER:
	case SHIELDCHARGER:
		object->radarcolour = 0xff00aa00;
		break;
	case EXITGATE:
		object->radarcolour = 0xffffffff;
		break;
	case BUTTONSWITCH:
		object->radarcolour = 0xff00ffff;
		break;
	case PLAYER:
	case SPAWNINERT:
	case INERT:
		object->radarcolour = 0;
		break;
	case TELEPORT:
		object->radarcolour = 0xff00ffff;
		object->behaviouroncollision = SLIDE;
		break;
	case SPINNER:
		object->behaviouroncollision = SLIDE;
		object->radarcolour = 0xffff0000;
		break;
	case ROCK:
		object->radarcolour = 0xffff9855;
		object->behaviouroncollision = BOUNCE;
		break;
	case BLUEPRINT:
	case GEMSTONE:
		object->radarcolour = 0xff2c49da;
		break;
	case MISSILECRATE:
	case ENERGY:
		object->radarcolour = 0xff11f000;
		object->behaviouroncollision = SLIDE;
		break;
	case TURRET:
		object->radarcolour = 0xffff0000;
		object->behaviouroncollision = SLIDE;
		break;
	case SPAWN:
	case ENEMY:
		object->behaviouroncollision = SLIDE;
		object->radarcolour = 0xffff0000;
		break;
	case DRONE:
		object->behaviouroncollision = SLIDE;
		break;
	case MISSILE:
		object->behaviouroncollision = ABSORB;
		break;
	case MINE:
		object->radarcolour = 0xffff40A0;
		break;
	case TRIGGERSWITCH:
		object->radarcolour = 0;
		break;
	case BASE:
		object->radarcolour = 0xffffffff;
		break;
	case CONVERTER:
		object->radarcolour = 0;
		break;
	}

	fp_in.ignore();					//Have to ignore the newline which is still in the input buffer
	fp_in>>junk>>object->name;
	fp_in>>junk>>object->graphicsname;
	fp_in>>junk>>object->flatness;
	fp_in>>junk>>object->shape;

	if(object->shape)
	{
		//Clear previous memory
		fp_in>>object->shapefile;		//Read in name of file containing boundaries
		sprintf_s(filenm,200,"bound\\%s.shape",object->shapefile.c_str());
		object->shapefile.clear();
		shapefile_in.open(filenm, ios::in);  // declare and open
		if(!shapefile_in.is_open())	//If opening failed
		{
			object->shape = 0;		//No polygonal shape boundaries here
			object->outline.Clear();
			object->rotatedoutline.Clear();
			object->predictedoutline.Clear();
			dbout<<"Could not read shape file "<<filenm<<endl;
		}
		else
		{
			int in;
			shapefile_in>>in;	//Read number of points

			object->outline.SetPoints(in);
			object->rotatedoutline.SetPoints(in);
			object->predictedoutline.SetPoints(in);

			for(int i=0;i<in;i++)
				shapefile_in>>object->outline.vertex[i].x>>object->outline.vertex[i].y;	//Read in list of points

			//dbout<<"Read "<<in<<" points"<<endl;
			shapefile_in.close();
		}
	}
	else
	{
		object->outline.Clear();
		object->rotatedoutline.Clear();
		object->predictedoutline.Clear();
	}

	fp_in>>junk>>object->energy;
	object->maxenergy = object->energy;
	fp_in>>junk;
	if(strcmp(junk,"radarcolour")==0)
	{
		fp_in>>object->radarcolour;
	}
	else fp_in>>object->strength;
	fp_in>>junk>>object->frames;
	fp_in>>junk>>readaggression;		//Aggression (or number of bullets for mines)
	if(object->type == MINE)
	{
		object->targetnode = (int)readaggression;
		readaggression = 0;
	}
	fp_in>>junk>>object->forwardsspeed;
	fp_in>>junk>>object->backwardsspeed;
	fp_in>>junk>>object->turnspeed;
	object->turnspeed = object->turnspeed * TWOPI/360.0f;
	object->aggression = readaggression*0.1f;

	fp_in>>junk>>object->sensorrange;
	fp_in>>junk>>object->attackrange;

	object->weapon = &weaponlist[GetWeaponNumber("Ion_Blaster")];		//Make the weapon 'Ion_Blaster' for safety

	int weaponnumber;
	fp_in>>junk>>temp;
	weaponnumber = GetWeaponNumber(temp);
	if(weaponnumber<0)				//No weapon needed
	{
		object->weapon = NULL;
		object->weaponmounts = 0;
		//object->attackrange = 0;
	}
	else if(weaponnumber==numberofweapons)		//Couldn't find the requested weapon (maybe spelling?) but still need to read the weapon mounts
	{
		//object->weapon = &weaponlist[GetWeaponNumber("Ion_Blaster")];		//Make the weapon 'Ion_Blaster' for safety
		if(object->type!=MINE)
		{
			fp_in>>junk>>object->weaponmounts;
			for(int i=0;i<object->weaponmounts && i<MAXWEAPONMOUNTS;i++)
			{
				fp_in>>junk>>object->weaponmount[i].x>>junk>>object->weaponmount[i].y>>junk>>object->weaponmount[i].angle;
				object->weaponmount[i].y = -object->weaponmount[i].y;
				object->weaponmount[i].angle = object->weaponmount[i].angle*PI/180.0f;
			}
		}
	}
	else
	{
		object->weapon = &weaponlist[weaponnumber];
		if(object->type!=MINE)
		{
			fp_in>>junk>>object->weaponmounts;
			for(int i=0;i<object->weaponmounts && i<MAXWEAPONMOUNTS;i++)
			{
				fp_in>>junk>>object->weaponmount[i].x>>junk>>object->weaponmount[i].y>>junk>>object->weaponmount[i].angle;
				object->weaponmount[i].y = -object->weaponmount[i].y;
				object->weaponmount[i].angle = object->weaponmount[i].angle*PI/180.0f;
			}
		}
	}

	object->engines = 0;
	if(object->type==ENEMY)
	{
		fp_in>>junk>>object->engines;
		for(int i=0;i<object->engines && i<MAXENGINES;i++)
		{
			fp_in>>junk>>object->engine[i].x>>junk>>object->engine[i].y;
			object->engine[i].y = -object->engine[i].y;
		}
	}

	fp_in>>junk;
	fp_in>>junk>>object->childobjects;
	//dbout<<"   "<<object->name<<" has "<<object->childobjects<<" child objects"<<endl;

	if(object->childobjects>0) object->childobject = new (nothrow) GameObject [object->childobjects];

	string spinstatus;
	for(i=0;i<object->childobjects;i++)
	{
		fp_in>>object->childobject[i].name>>junk>>object->childobject[i].initialposition.x>>junk>>object->childobject[i].initialposition.y>>junk>>object->childobject[i].angle>>junk>>object->childobject[i].faderate;
		int readspawnrate = (int)(1000.0f/object->childobject[i].faderate);
		object->childobject[i].initialposition.y = -object->childobject[i].initialposition.y;	//This is actually a position relative to the parent
		object->childobject[i].angle = object->childobject[i].angle * (PI/180.0f);
		ReadObject(&object->childobject[i],object->childobject[i].name);
		//dbout<<"Added child object "<<object->childobject[i].name<<" angle "<<object->childobject[i].angle<<endl;
		object->childobject[i].target = object->position;
		//object->childobject[i].finaltarget = object->position;
		object->childobject[i].graphics = &error;		//For safety
		switch(object->childobject[i].type)
		{
		case INERT:
		case SPAWNINERT:
			fp_in>>spinstatus>>object->childobject[i].turnspeed;
			if(spinstatus=="osc") SET(object->childobject[i].state,OSCILLATE);	//To tell us it oscillates rather than just rotating

			point = fp_in.tellg();
			fp_in>>junk;
			if(strcmp(junk,"limitangle")==0) fp_in>>object->childobject[i].minangle>>object->childobject[i].maxangle;
			else
			{
				object->childobject[i].minangle = -360.0f;
				object->childobject[i].maxangle = 360.0f;
				fp_in.seekg(point);
			}
			object->childobject[i].minangle *= PI/180.0f;
			object->childobject[i].maxangle *= PI/180.0f;

			if(object->childobject[i].type==INERT)
			{
				if(object->childobject[i].faderate==0) object->childobject[i].timer = 0;
				else object->childobject[i].timer = (int)(1000.0/object->childobject[i].faderate);
			}
			object->childobject[i].frame = 0;
			object->childobject[i].lastfired = 0;
			point = fp_in.tellg();
			fp_in>>junk;
			if(strcmp(junk,"startframe")==0) fp_in>>object->childobject[i].frame;
			else fp_in.seekg(point);
			break;
		case TURRET:
			point = fp_in.tellg();
			fp_in>>junk;
			if(strcmp(junk,"limitangle")==0) fp_in>>object->childobject[i].minangle>>object->childobject[i].maxangle;
			else
			{
				object->childobject[i].minangle = -360.0f;
				object->childobject[i].maxangle = 360.0f;
				fp_in.seekg(point);
			}
			object->childobject[i].minangle *= PI/180.0f;
			object->childobject[i].maxangle *= PI/180.0f;

			break;
		case ENEMY:		//Only permissible if this is a spawning object
			if(object->type == SPAWN || object->type == SPAWNINERT)
			{
				if(i != object->childobjects - 1)		//If it isn't the last object, delete it
				{
					--i;		//Move the list down by one - this object is not valid!
					--object->childobjects;
				}
				else
				{	//Set the spawn rate
					object->timer = readspawnrate;
					object->lastfired = 0;
				}
				break;
			}
			//Otherwise fall through to the default behaviour, which is to delete it
		default:
			--i;		//Move the list down by one - this object is not valid!
			--object->childobjects;
			break;
		}
	}

	//dbout<<"Object \""<<object->name<<"\" read"<<endl;
	fp_in.close();

	return true;
}

int GetType(char *type)
{
	_strupr(type);

	if(strcmp(type,"INERT")==0) return INERT;
	if(strcmp(type,"PLAYER")==0) return PLAYER;
	if(strcmp(type,"ENEMY")==0) return ENEMY;
	if(strcmp(type,"MINE")==0) return MINE;
	if(strcmp(type,"MISSILE")==0) return MISSILE;
	if(strcmp(type,"ROCK")==0) return ROCK;
	if(strcmp(type,"TURRET")==0) return TURRET;
	if(strcmp(type,"TELEPORT")==0) return TELEPORT;
	if(strcmp(type,"SPINNER")==0) return SPINNER;
	if(strcmp(type,"BASE")==0) return BASE;
	if(strcmp(type,"EXIT")==0) return EXITGATE;
	if(strcmp(type,"ENERGY")==0) return ENERGY;
	if(strcmp(type,"SWITCH")==0) return BUTTONSWITCH;
	if(strcmp(type,"TRIGGER")==0) return TRIGGERSWITCH;
	if(strcmp(type,"DRONE")==0) return DRONE;
	if(strcmp(type,"SHIELDCHARGER")==0) return SHIELDCHARGER;
	if(strcmp(type,"GAMESAVER")==0) return GAMESAVER;
	if(strcmp(type,"MISSILECRATE")==0) return MISSILECRATE;
	if(strcmp(type,"CONVERTER")==0) return CONVERTER;
	if(strcmp(type,"GEMSTONE")==0) return GEMSTONE;
	if(strcmp(type,"BLUEPRINT")==0) return BLUEPRINT;
	if(strcmp(type,"SPAWN")==0) return SPAWN;
	if(strcmp(type,"SPAWNINERT")==0) return SPAWNINERT;
	return NONE;
}

void CopyObject(GameObject *source, GameObject *dest)
{
	if(dest==NULL)
	{
		dbout<<"Invalid destination pointer!"<<endl;
		return;
	}
	if(source==NULL)
	{
		dbout<<"Invalid source pointer!"<<endl;
		return;
	}

	dest->type = source->type;
	strcpy_s( dest->name, 100, source->name );
	dest->origfilename = source->origfilename;
	dest->graphicsname = source->graphicsname;
	dest->shapefile = source->shapefile;
	dest->graphics = source->graphics;
	dest->flatness = source->flatness;
	dest->shape = source->shape;

	if(dest->shape)
	{
		//dbout<<"Copying "<<source->outline.points<<" points for "<<source->name<<endl;
		CopyPolygon(&source->outline,&dest->outline);
		dest->rotatedoutline.SetPoints(source->outline.points);
		dest->predictedoutline.SetPoints(source->outline.points);
	}

	dest->radius = source->radius;
	dest->radiussquared = source->radiussquared;
	dest->rootradius = source->rootradius;
	dest->rangelimit = source->rangelimit;
	dest->initialposition = source->initialposition;
	dest->angle = source->angle;
	dest->velocity = source->velocity;
	dest->energy = source->energy;
	dest->maxenergy = source->maxenergy;
	dest->strength = source->strength;
	dest->frames = source->frames;
	dest->frame = source->frame;
	dest->aggression = source->aggression;
	dest->forwardsspeed = source->forwardsspeed;
	dest->backwardsspeed = source->backwardsspeed;
	dest->turnspeed = source->turnspeed;
	dest->sensorrange = source->sensorrange;
	dest->attackrange = source->attackrange;
	dest->behaviouroncollision = source->behaviouroncollision;
	dest->radarcolour = source->radarcolour;
	dest->weapon = source->weapon;
	dest->weaponmounts = source->weaponmounts;
	dest->engines = source->engines;
	dest->lastfired = 0;
	dest->timer = source->timer;
	dest->scale = source->scale;
	dest->scalerate = source->scalerate;
	dest->scalecentre = source->scalecentre;
	dest->state = source->state;
	dest->targetnode = source->targetnode;
	dest->nextnode = source->nextnode;
	dest->minangle = source->minangle;
	dest->maxangle = source->maxangle;

	for(int i=0;i<source->weaponmounts;i++) dest->weaponmount[i] = source->weaponmount[i];
	for(int i=0;i<source->engines;i++) dest->engine[i] = source->engine[i];
	dest->childobjects = source->childobjects;
	if(dest->childobjects>0) dest->childobject = new (nothrow) GameObject[dest->childobjects];
	for(int i=0;i<source->childobjects;i++) CopyObject(&source->childobject[i],&dest->childobject[i]);

}

void CopyPolygon(tPolygon *source, tPolygon *dest)
{
	if(source->points == 0) return;
	dest->points = source->points;
	dest->SetPoints(source->points);
	for(int i=0;i<source->points;i++)
		dest->vertex[i] = source->vertex[i];
}


void LoadAssignGraphics()
{
	GraphicsItem glist[5000];	//Up to 500 different graphics items - far in excess of what's plausible
	bool onlist = false;
	int listlength = 0;

	list<GameObject>::iterator i;
	for(i=entity.begin();i!=entity.end();++i)
	{
		CheckForGraphics(glist, &listlength, &*i);
	}

	//dbout<<listlength<<" graphics items"<<endl;

	for(int k=0;k<listlength;k++)
	{
		graphics.push_back(0);
		graphics[k].CreateGraphicalObject(glist[k].name.c_str(), glist[k].frames, glist[k].flatness);
	}

	//dbout<<"Assigning"<<endl; 
	for(i=entity.begin();i!=entity.end();++i)		//Check all game objects
	{
		for(int k=0;k<listlength;k++)
		{
			if(i->graphicsname == glist[k].name)
			{
				i->graphics = &graphics[k];
				i->graphicsname.clear();
				//dbout<<"Assigning graphics "<<glist[k].name<<" to entity "<<i->name<<endl;
				if(i->shape == 0)
				{	CreatePolygonOutline(&i->outline,(float)graphics[k].framewidth,(float)graphics[k].frameheight);
					i->rotatedoutline.SetPoints(4);
					i->predictedoutline.SetPoints(4);
					i->shape = 1;
				}
				i->radiussquared = FindRadius(&i->outline);
				i->radius = sqrt(i->radiussquared);
				i->rootradius = sqrt(i->radius);
				WrapToLevel(&i->position);
				GridPosition(&*i);
				RotateOutline(&i->outline, &i->rotatedoutline, i->cosangle, i->sinangle);
				TranslateOutline(&i->rotatedoutline, &i->predictedoutline, &i->position);
				//if(i->outline.points) dbout<<"First point "<<i->outline.vertex[0].x<<" and "<<i->outline.vertex[0].y<<endl;
				//dbout<<"Setting radius to "<<i->radiussquared<<" and "<<i->rootradius<<endl;
				break;
			}
		}
		for(int j=0;j<i->childobjects;j++) AssignGraphics(glist, listlength, &i->childobject[j]);

		if(i->childobjects>0)
		{
			if(i->type == SPAWN || i->type == SPAWNINERT)
			{
				--i->childobjects;		//Don't want its spawn object being drawn or moved, now we've given it graphics
			}
		}
	}
	//dbout<<"New graphics size "<<graphics.size()<<endl;
}

void CheckForGraphics(GraphicsItem *glist, int *listlength, GameObject *object)
{
	bool onlist = false;
	for(int j=0;j<(*listlength);j++)
	{
		if(object->graphicsname == glist[j].name)	//Check the list to see if it's already on there
		{
			onlist = true;
			if(glist[j].flatness > object->flatness) glist[j].flatness = object->flatness;	//We use the maximum contrast
			if(glist[j].frames < object->frames) glist[j].frames = object->frames;			//We use the maximum number of frames
			break;
		}
	}

	if(!onlist)
	{
		//dbout<<"Adding "<<object->graphicsname<<" to list"<<endl;
		glist[*listlength].name = object->graphicsname;	//Add to list
		glist[*listlength].flatness = object->flatness;		//
		glist[*listlength].frames = object->frames;
		++(*listlength);
	}
	for(int k=0;k<object->childobjects;k++) CheckForGraphics(glist, listlength, &object->childobject[k]);
		
}

//See if an object matches this graphics, if so, copy the pointer
void AssignGraphics(GraphicsItem *glist, int listlength, GameObject *object)
{
	for(int k=0;k<listlength;k++)
	{
		if(object->graphicsname == glist[k].name)
		{
			object->graphics = &graphics[k];
			if(object->type == ENEMY)
			{
				if(object->shape == 0)
				{	CreatePolygonOutline(&object->outline,(float)graphics[k].framewidth,(float)graphics[k].frameheight);
					object->rotatedoutline.SetPoints(4);
					object->predictedoutline.SetPoints(4);
					object->shape = 1;
				}
				object->radiussquared = FindRadius(&object->outline);
				object->radius = sqrt(object->radiussquared);
				object->rootradius = sqrt(object->radius);
				WrapToLevel(&object->position);
				GridPosition(&*object);
				RotateOutline(&object->outline, &object->rotatedoutline, object->cosangle, object->sinangle);
				TranslateOutline(&object->rotatedoutline, &object->predictedoutline, &object->position);
			}
			break;
		}
	}
	//dbout<<"This child object "<<object->name<<" has "<<object->childobjects<<" child objects"<<endl;
	for(int j=0;j<object->childobjects;j++) AssignGraphics(glist, listlength, &object->childobject[j]);

	if(object->childobjects>0)
		if(object->type == SPAWN || object->type == SPAWNINERT) --object->childobjects;		//Don't want its spawn object being drawn or moved, now we've given it graphics

}



bool ReadWeapons()
{
	using namespace std;
	ifstream fp_in, fp_in2, shapefile_in;
	int i, counter;
	char junk[100], temp[100], strin[500];

	fp_in2.open("data\\weapons.dat", ios::in);  // declare and open
	if(!fp_in2.is_open()) return false;

	numberofweapons = 0;
	while(fp_in2.getline(junk,100)) if(strcmp(junk,"<weapon>")==0) numberofweapons++;
	fp_in2.close();

	//dbout<<"Found "<<numberofweapons<<" weapons in file"<<endl;
	weaponlist = new Weapon[numberofweapons];
	weapongraphics = new GraphicalObject[numberofweapons];
	weaponpulsegfx = new GraphicalObject[numberofweapons];
	weapontrails = new GraphicalObject[numberofweapons];

	fp_in.open("data\\weapons.dat", ios::in);  // declare and open
	if(!fp_in.is_open()) return false;

	for(i=0;i<numberofweapons;i++)
	{
		counter = 0;
		do
		{
			fp_in.getline( junk, 100 );
			++counter;
		} while(strcmp(junk,"<weapon>")!=0 && counter<100);	//Make sure we don't get into an infinite loop, or try to read blank file

		fp_in.getline( junk, 100);
		weaponlist[i].name = junk;
		fp_in.getline( strin, 500);
		weaponlist[i].description = strin;
		fp_in.ignore();					//Have to ignore the newline which is still in the input buffer

		fp_in>>junk>>weaponlist[i].graphicsfile;
		fp_in>>junk>>weaponlist[i].pulsefile;
		//dbout<<weaponlist[i].graphicsfile<<endl;

		fp_in>>temp>>junk;
		sprintf_s(strin,500,"sounds\\%s.wav",junk);
		weaponlist[i].soundfile = strin;

		weapongraphics[i].CreateGraphicalObject((char *)weaponlist[i].graphicsfile.c_str(),1);
		weaponpulsegfx[i].CreateGraphicalObject((char *)weaponlist[i].pulsefile.c_str(),1);
		weaponlist[i].graphics = &weapongraphics[i];
		weaponlist[i].pulsegraphics = &weaponpulsegfx[i];

		weaponlist[i].available = false;

		fp_in>>junk>>weaponlist[i].energy;
		fp_in>>junk>>weaponlist[i].faderate;
		fp_in>>junk>>weaponlist[i].firerate;
		weaponlist[i].power = weaponlist[i].energy * weaponlist[i].firerate;
		weaponlist[i].firerate = 1000.0f / weaponlist[i].firerate;
		fp_in>>junk>>weaponlist[i].fadevalue;
		fp_in>>junk>>weaponlist[i].bulletgrowrate;
		fp_in>>junk>>weaponlist[i].bulletscale;
		fp_in>>junk>>junk;
		if(strcmp(junk,"yes")==0) weaponlist[i].scalecentre = true;
		else weaponlist[i].scalecentre = false;
		fp_in>>junk>>weaponlist[i].speed;
		//Impact
		fp_in>>junk>>temp;
		if(strcmp(temp,"bounce")==0)
		{
			weaponlist[i].bounce = true;
			weaponlist[i].explode = false;
		}
		else
		{
			weaponlist[i].bounce = false;
			if(strcmp(temp,"explode")==0) weaponlist[i].explode = true;
			else weaponlist[i].explode = false;
		}
		//Charge
		fp_in>>junk>>temp;
		if(strcmp(temp,"no")==0) weaponlist[i].charge = false;
		else weaponlist[i].charge = false;
		//Trail
		fp_in>>junk>>temp;
		if(strcmp(temp,"no")==0) weaponlist[i].trail = false;
		else
		{
			if(weapontrails[i].CreateGraphicalObject(temp,1))
			{
				weaponlist[i].trailgfx = &weapontrails[i];
				fp_in>>junk>>weaponlist[i].trailfaderate;
				fp_in>>junk>>weaponlist[i].trailgrowrate;
				weaponlist[i].trail = true;
				//dbout<<"Loaded trail graphics: "<<temp<<", "<<weaponlist[i].name<<" grow rate "<<weaponlist[i].trailgrowrate<<endl;
			}
			else
			{
				weaponlist[i].trail = false;
				dbout<<"Couldn't load weapon trail"<<endl;
			}
		}

		weaponlist[i].number = i;
	}
	fp_in.close();


	SortWeapons();
	for(i=0;i<numberofweapons;i++)
	{
		if(LoadDirectSound(&weaponlist[i].sound.buffer, (char *)weaponlist[i].soundfile.c_str())!=DS_OK)
		{
			weaponlist[i].sound.buffer = NULL;
			dbout<<"Problem loading sound "<<weaponlist[i].soundfile<<endl;
		}
	}

	return true;
}

int GetWeaponNumber(const char *weaponname)
{
	char weaponlistname[100], weaponrequiredname[100];
	//Prevent uppering the source text
	strcpy_s(weaponrequiredname,100,weaponname);	//weaponrequiredname = weaponname
	_strupr(weaponrequiredname);

	if(strcmp(weaponrequiredname,"NONE")==0) return -1;
	if(strcmp(weaponrequiredname,"-1")==0) return -1;

	for(int i=0;i<numberofweapons;i++)
	{
		strcpy_s(weaponlistname,100,weaponlist[i].name.c_str());	//weaponlistname = weaponlist[i].name
		_strupr(weaponlistname);
		if(strcmp(weaponrequiredname,weaponlistname)==0) return i;
	}
	return numberofweapons;
}


int GetEquipmentType(const char *instr)
{
	char instring[100];
	strcpy_s(instring,100,instr);
	_strupr(instring);
	if(strcmp(instring,"BOOSTER")==0) return BOOSTER;
	if(strcmp(instring,"RADAR")==0) return RADAR;
	if(strcmp(instring,"SCANNER")==0) return SCANNER;
	if(strcmp(instring,"GEMTRACTOR")==0) return GEMTRACTOR;
	if(strcmp(instring,"HYPERSEEKER")==0) return SEEKER;
	if(strcmp(instring,"MISSILE_LAUNCHER")==0) return MISSILE_LAUNCHER;
	if(strcmp(instring,"MISSILES")==0) return MISSILES;
	if(strcmp(instring,"SHIELD_DRONE")==0) return SHIELD_DRONE;
	if(strcmp(instring,"LIGHTNING_DRONE")==0) return LIGHTNING_DRONE;
	if(strcmp(instring,"SHIELD")==0) return SHIELD;
	if(strcmp(instring,"SELF_REPAIR")==0) return SELF_REPAIR;
	if(strcmp(instring,"SIDE_BOOST")==0) return SIDE_BOOST;
	if(strcmp(instring,"BOMB")==0) return BOMB;
	if(strcmp(instring,"TARGET_LASER")==0) return TARGET_LASER;
	if(strcmp(instring,"DRONE")==0) return LASER_DRONE;
	if(strcmp(instring,"HOMING_COMPUTER")==0) return HOMING_COMPUTER;
	if(strcmp(instring,"SHIELD_RECHARGER")==0) return SHIELD_RECHARGER;
	if(strcmp(instring,"LIGHTNING_POD")==0) return LIGHTNING_POD;
	if(strcmp(instring,"SPREADER")==0) return SPREADER;
	if(strcmp(instring,"MINI_MISSILE_LAUNCHER")==0) return MINIMISSILES;
	if(strcmp(instring,"AUXILIARY_GENERATOR")==0) return AUXILIARY_GENERATOR;
	if(strcmp(instring,"MISSILEBOMBS")==0) return MISSILEBOMBS;
	return NOTHING;
}

void GetEquipmentString(int etype, char *instring)
{
	switch(etype)
	{
		case BOOSTER:				sprintf(instring,"booster");				return;
		case RADAR:					sprintf(instring,"radar");					return;
		case MISSILE_LAUNCHER:		sprintf(instring,"missile_launcher");		return;
		case MISSILES:				sprintf(instring,"missiles");				return;
		case SHIELD_DRONE:			sprintf(instring,"shield_drone");			return;
		case SHIELD:				sprintf(instring,"shield");					return;
		case SELF_REPAIR:			sprintf(instring,"self_repair");			return;
		case SIDE_BOOST:			sprintf(instring,"side_boost");				return;
		case BOMB:					sprintf(instring,"bomb");					return;
		case TARGET_LASER:			sprintf(instring,"target_laser");			return;
		case LASER_DRONE:			sprintf(instring,"drone");					return;
		case HOMING_COMPUTER:		sprintf(instring,"homing_computer");		return;
		case SHIELD_RECHARGER:		sprintf(instring,"shield_recharger");		return;
		case LIGHTNING_POD:			sprintf(instring,"lightning_pod");			return;
		case SPREADER:				sprintf(instring,"spreader");				return;
		case MINIMISSILES:			sprintf(instring,"mini_missile_launcher");	return;
		case AUXILIARY_GENERATOR:	sprintf(instring,"auxiliary_generator");	return;
		case MISSILEBOMBS:			sprintf(instring,"missilebombs");			return;
		case LIGHTNING_DRONE:		sprintf(instring,"lightning_drone");			return;
		default: return;
	}
}


void DuplicatePlayer(Player *src, Player *dst)
{
	*dst = *src;
	dst->energy = src->maxenergy;
	dst->power = weaponlist[GetWeaponNumber("Ion_Blaster")].power;
	dst->shields = 0;

	dst->angle = 0;
	dst->cost = src->cost;
	dst->cosangle = 1.0;
	strcpy_s(dst->description, 300, src->description);
	strcpy_s(dst->filename, 300, src->filename);
	for(int i=0;i<NUMEQUIPTYPES;i++) dst->equipment[i] = src->equipment[i];
	dst->frame = 0;
	dst->graphics = src->graphics;
	dst->graphics.colourtexture = NULL;
	dst->graphics.bumptexture = NULL;
	dst->graphics.texture = NULL;
	strcpy_s(dst->graphics.name,50,src->graphics.name);
	strcpy_s(dst->graphicsfile,50,src->graphicsfile);

	if(!dst->graphics.CreateGraphicalObject(dst->graphicsfile,dst->graphics.frames,dst->graphics.flatness)) {/*Put error code in here*/}
	dst->graphics.info = src->graphics.info;
	dst->homingcomputerbusy = false;
	dst->lastfired = 0;
	dst->lastlaunched = 0;
	dst->lastminimissile = 0;
	dst->lastseeker = 0;
	dst->credits = src->credits;
	strcpy_s(dst->logo,50,src->logo);
	strcpy_s(dst->manufacturer,50,src->manufacturer);
	strcpy_s(dst->name,50,src->name);

	dst->outline.vertex = NULL;	dst->outline.points = 0;
	dst->rotatedoutline.vertex = NULL; dst->rotatedoutline.points = 0;
	dst->predictedoutline.vertex = NULL; dst->predictedoutline.points = 0;
	dst->outline.SetPoints(src->outline.points);
	dst->predictedoutline.SetPoints(src->outline.points);
	dst->rotatedoutline.SetPoints(src->outline.points);

	for(int i=0;i<dst->outline.points;i++)
	{
		dst->outline.vertex[i] = src->outline.vertex[i];
		dst->predictedoutline.vertex[i] = src->predictedoutline.vertex[i];
		dst->rotatedoutline.vertex[i] = src->rotatedoutline.vertex[i];
	}

	dst->screen.SetPoints(4);
	for(int i=0;i<4;i++) dst->screen.vertex[i] = src->screen.vertex[i];

	dst->radar = src->radar;
	dst->radar.bumptexture = NULL;
	dst->radar.colourtexture = NULL;
	dst->radar.info = src->radar.info;
	strcpy_s(dst->radar.name,50,src->radar.name);
	dst->radar.texture = NULL;

	dst->radar.CreateGraphicalObject(RADARWIDTH,RADARHEIGHT);
	strcpy_s(dst->shapefile,50,src->shapefile);
	dst->sinangle = 0;
	for(int i=0;i<src->weaponmounts;i++) dst->weaponmount[i] = src->weaponmount[i];
	dst->weapon = &weaponlist[GetWeaponNumber("Ion_Blaster")];
	dst->screenalpha = 0xffffffff;
	//dbout<<"Done"<<endl;
}



void ReSumChildAngles(GameObject *object, float angle)
{
	Fsincos_t sincos;
	for(int i=0;i<object->childobjects;i++)
	{
		object->childobject[i].angle += angle;
		if(object->childobject[i].angle>=TWOPI) object->childobject[i].angle -= TWOPI;
		else if(object->childobject[i].angle<0) object->childobject[i].angle += TWOPI;
		object->childobject[i].changeangle = 0;
		sincos = fsincos(object->childobject[i].angle);
		object->childobject[i].cosangle = sincos.cosine;
		object->childobject[i].sinangle = -sincos.sine;
		object->childobject[i].unitvectorforward.x = -object->childobject[i].sinangle;
		object->childobject[i].unitvectorforward.y = -object->childobject[i].cosangle;
		ReSumChildAngles(&object->childobject[i], object->childobject[i].angle);
	}
}

bool SaveGame(char *filename, Level *level)
{
	list<GameObject>::iterator it = entity.begin();
	int i, j;
	char tempstring[100];
	ofstream fileout;

	//SavePrefs();

	fileout.open(filename,ios::out);
	if(!fileout.is_open()) return false;

	fileout<<"Players "<<playmode+1<<endl;
	fileout<<"Player1ship "<<ship[0].filename<<endl;
	fileout<<"Player1energy "<<ship[0].energy<<endl;
	fileout<<"Player1power "<<ship[0].power<<endl;
	fileout<<"Player1shield "<<ship[0].shields<<endl;
	fileout<<"Player1weapon "<<ship[0].weapon->name<<endl;
	fileout<<"Player1missiles "<<ship[0].missiles<<endl;
	fileout<<"Player1forwardV "<<ship[0].forwardsspeed<<endl;
	fileout<<"Player1backwardsV "<<ship[0].backwardsspeed<<endl;
	fileout<<"Player1turnV "<<ship[0].turnspeed<<endl;
	fileout<<"Player1position "<<ship[0].position.x<<" "<<ship[0].position.y<<endl;
	fileout<<"Player1direction "<<ship[0].unitvectorforward.x<<" "<<ship[0].unitvectorforward.y<<endl;
	fileout<<"Player1angle "<<ship[0].angle<<endl;
	fileout<<"Player1stealthed "<<ship[0].stealthed<<endl;
	fileout<<"Player1credits "<<ship[0].credits<<endl;
	fileout<<"Player1equipment"<<endl;
	for(i=0;i<NUMEQUIPTYPES;i++)
	{
		if(i==RADAR || i==SCANNER) fileout<<i<<" yes"<<endl;
		else fileout<<i<<" "<<(ship[0].equipment[i]?"yes":"no")<<endl;
	}

	if(playmode==TWOPLAYER)
	{
		fileout<<"Player2ship "<<ship[1].filename<<endl;
		fileout<<"Player2energy "<<ship[1].energy<<endl;
		fileout<<"Player2power "<<ship[1].power<<endl;
		fileout<<"Player2shield "<<ship[1].shields<<endl;
		fileout<<"Player2weapon "<<ship[1].weapon->name<<endl;
		fileout<<"Player2missiles "<<ship[1].missiles<<endl;
		fileout<<"Player2forwardV "<<ship[1].forwardsspeed<<endl;
		fileout<<"Player2backwardsV "<<ship[1].backwardsspeed<<endl;
		fileout<<"Player2turnV "<<ship[1].turnspeed<<endl;
		fileout<<"Player2position "<<ship[1].position.x<<" "<<ship[1].position.y<<endl;
		fileout<<"Player2direction "<<ship[1].unitvectorforward.x<<" "<<ship[1].unitvectorforward.y<<endl;
		fileout<<"Player2angle "<<ship[1].angle<<endl;
		fileout<<"Player2stealthed "<<ship[1].stealthed<<endl;
		fileout<<"Player2credits "<<ship[1].credits<<endl;
		fileout<<"Player2equipment"<<endl;
		for(i=0;i<NUMEQUIPTYPES;i++)
		{
			if(i==RADAR || i==SCANNER) fileout<<i<<" yes"<<endl;
			else fileout<<i<<" "<<(ship[1].equipment[i]?"yes":"no")<<endl;
		}
		fileout<<endl;
	}
	fileout<<endl;

	for(i=0;i<NUMEQUIPTYPES;i++)
	{
		if(i==RADAR || i==SCANNER) fileout<<i<<" yes"<<endl;
		else fileout<<i<<" "<<(equipment[i]?"yes":"no")<<endl;
	}
	fileout<<endl;

	fileout<<"<weapons>"<<endl;
	for(i=0;i<numberofweapons;i++)
	{
		if(weaponlist[i].available)
			fileout<<weaponlist[i].name<<endl;
	}
	fileout<<"</weapons>"<<endl<<endl;


	fileout<<"<name>"<<endl<<level->name<<endl<<"</name>"<<endl<<endl;							//Write out level name

	fileout<<"<description>"<<endl<<level->description<<endl<<"</description>"<<endl<<endl;		//Write out level description

	fileout<<"<next>"<<endl<<level->next<<endl<<"</next>"<<endl<<endl;							//Write out next level name

	fileout<<"<tileset>"<<endl<<level->tiles<<endl<<"</tileset>"<<endl<<endl;						//Write out level tileset

	fileout<<"<tilemap>"<<endl<<"width "<<level->width<<" height "<<level->height<<endl;		//Write out level map

	for(j=0;j<level->height;j++)
	{
		for(i=0;i<level->width;i++)
		{
			fileout<<level->tilemap[i + j*level->width]<<" ";
		}
		fileout<<endl;
	}
	fileout<<"</tilemap>"<<endl<<endl;

	fileout<<"<music>"<<endl<<level->music<<endl<<"</music>"<<endl<<endl;							//Write out level name

	fileout<<"<objects>"<<endl<<endl;
/*
enum GAME_OBJ_TYPE {INERT, FADE, EXPLOSION,										//Non-solid types
					BULLET, SHIELDDRONE, MISSILE, ENEMY, MINE, ROCK, TURRET, SPINNER, TRIGGERSWITCH,	//Collidable types
					TELEPORT, BASE, SHIELDCHARGER, EXITGATE, TELEPOD, DRONE, BLACKHOLEOBJ, ENERGY, BUTTONSWITCH};	//Other types*/

	for(++it;it!=entity.end();it++)
	{
		switch(it->type)
		{
		case PLAYER:
			fileout<<"<group>"<<endl<<"1 player"<<endl<<"px 0 py 0 angle 0"<<endl<<"0"<<endl<<"</group>"<<endl<<endl;
			break;
		case SPAWNINERT:
			fileout<<"<group>"<<endl;
			fileout<<"1 "<<it->origfilename<<endl;
			fileout<<"px "<<it->position.x<<" py "<<it->position.y<<" angle "<<it->angle<<" angvel "<<it->turnspeed;
			if(CHECK(it->state,LINKEDFROMBOUNDARY)) fileout<<" fromboundary "<<it->link;
			fileout<<endl<<"time "<<(thistick - it->lastfired)<<endl;			
			fileout<<it->state<<endl;
			fileout<<"</group>"<<endl<<endl;
			break;
		case INERT:
			fileout<<"<group>"<<endl;
			fileout<<"1 "<<it->origfilename<<endl;
			fileout<<"px "<<it->position.x<<" py "<<it->position.y<<" angle "<<it->angle<<" angvel "<<it->turnspeed;
			if(CHECK(it->state,LINKEDFROMBOUNDARY)) fileout<<" fromboundary "<<it->link;
			fileout<<endl<<it->state<<endl;
			fileout<<"</group>"<<endl<<endl;
			break;
		case ENEMY:
			fileout<<"<group>"<<endl;
			fileout<<"1 "<<it->origfilename<<endl;
			fileout<<"px "<<it->position.x<<" py "<<it->position.y<<" angle "<<it->angle<<" range "<<it->rangelimit<<" objective "<<it->objective;
			if(CHECK(it->state,LINKEDTOBOUNDARY)) fileout<<" setboundary "<<it->link;
			fileout<<endl<<it->state<<endl;
			fileout<<"</group>"<<endl<<endl;
			break;
		case MINE:
			fileout<<"<group>"<<endl;
			fileout<<"1 "<<it->origfilename<<endl;
			fileout<<"px "<<it->position.x<<" py "<<it->position.y<<" angle "<<it->angle<<" range "<<it->sensorrange/screenratio<<" time "<<it->timer<<" objective "<<it->objective;
			if(CHECK(it->state,LINKEDTOBOUNDARY)) fileout<<" setboundary "<<it->link;
			fileout<<endl<<it->state<<endl;
			fileout<<"</group>"<<endl<<endl;
			break;
		case TURRET:
			fileout<<"<group>"<<endl;
			fileout<<"1 "<<it->origfilename<<endl;
			fileout<<"px "<<it->position.x<<" py "<<it->position.y<<" angle "<<it->angle<<" objective "<<it->objective;
			if(CHECK(it->state,LINKEDTOBOUNDARY)) fileout<<" setboundary "<<it->link;
			fileout<<endl<<it->state<<endl;
			fileout<<"</group>"<<endl<<endl;
			break;
		case TRIGGERSWITCH:
			if(CHECK(it->state,TRIGGEREDBYENEMY)) sprintf_s(tempstring,100,"enemy");
			else if(CHECK(it->state,TRIGGEREDBYROCK)) sprintf_s(tempstring,100,"rock");
			else if(CHECK(it->state,TRIGGEREDBYPLAYER)) sprintf_s(tempstring,100,"player");
			fileout<<"<group>"<<endl;
			fileout<<"1 "<<it->origfilename<<endl;
			fileout<<"px "<<it->position.x<<" py "<<it->position.y<<" angle "<<it->angle<<" trigger "<<tempstring<<" timeout "<<it->timer<<" setboundary "<<it->link<<endl;
			fileout<<"time "<<(thistick - it->lastfired)<<endl;
			fileout<<it->state<<endl;
			fileout<<"</group>"<<endl<<endl;
			break;
		case SPAWN:
			fileout<<"<group>"<<endl;
			fileout<<"1 "<<it->origfilename<<endl;
			fileout<<"px "<<it->position.x<<" py "<<it->position.y<<" angle "<<it->angle<<" vx "<<it->forwardsspeed*it->unitvectorforward.x<<" vy "<<it->forwardsspeed*it->unitvectorforward.y<<" angvel "<<it->turnspeed<<" objective "<<it->objective;
			if(CHECK(it->state,LINKEDTOBOUNDARY)) fileout<<" setboundary "<<it->link;
			fileout<<endl<<"time "<<(thistick - it->lastfired)<<endl;
			fileout<<it->state<<endl;
			fileout<<"</group>"<<endl<<endl;
			break;
		case ROCK:
			fileout<<"<group>"<<endl;
			fileout<<"1 "<<it->origfilename<<endl;
			fileout<<"px "<<it->position.x<<" py "<<it->position.y<<" angle "<<it->angle<<" vx "<<it->forwardsspeed*it->unitvectorforward.x<<" vy "<<it->forwardsspeed*it->unitvectorforward.y<<" angvel "<<it->turnspeed<<" objective "<<it->objective;
			if(CHECK(it->state,LINKEDTOBOUNDARY)) fileout<<" setboundary "<<it->link;
			fileout<<endl<<it->state<<endl;
			fileout<<"</group>"<<endl<<endl;
			break;
		case BUTTONSWITCH:
			fileout<<"<group>"<<endl;
			fileout<<"1 "<<it->origfilename<<endl;
			fileout<<"px "<<it->position.x<<" py "<<it->position.y<<" angle "<<it->angle;
			if(CHECK(it->state,LINKEDTOBOUNDARY)) fileout<<" setboundary "<<it->link;
			fileout<<endl<<it->state<<endl;
			fileout<<"</group>"<<endl<<endl;
			break;
		case ENERGY:
			fileout<<"<group>"<<endl;
			fileout<<"1 "<<it->origfilename<<endl;
			fileout<<"px "<<it->position.x<<" py "<<it->position.y<<" angle "<<it->angle<<" energy "<<it->energy<<" angvel "<<it->turnspeed<<endl;
			fileout<<it->state<<endl;
			fileout<<"</group>"<<endl<<endl;
			break;
		case MISSILECRATE:
			fileout<<"<group>"<<endl;
			fileout<<"1 "<<it->origfilename<<endl;
			fileout<<"px "<<it->position.x<<" py "<<it->position.y<<" angle "<<it->angle<<" missiles "<<it->link<<" angvel "<<it->turnspeed<<endl;
			fileout<<it->state<<endl;
			fileout<<"</group>"<<endl<<endl;
			break;
		case GEMSTONE:
			fileout<<"<group>"<<endl;
			fileout<<"1 "<<it->origfilename<<endl;
			fileout<<"px "<<it->position.x<<" py "<<it->position.y<<" angle "<<it->angle<<" credits "<<it->link<<" angvel "<<it->turnspeed<<endl;
			fileout<<it->state<<endl;
			fileout<<"</group>"<<endl<<endl;
			break;
		case BLUEPRINT:
			fileout<<"<group>"<<endl;
			fileout<<"1 "<<it->origfilename<<endl;
			fileout<<"px "<<it->position.x<<" py "<<it->position.y<<" angle "<<it->angle<<" angvel "<<it->turnspeed<<(CHECK(it->state,STOREEQUIPMENT)?" equipment ":" weapon ")<<it->storestring<<endl;
			fileout<<it->state<<endl;
			fileout<<"</group>"<<endl<<endl;
			break;
		case BASE:
			fileout<<"<group>"<<endl;
			fileout<<"1 "<<it->origfilename<<endl;
			fileout<<"px "<<it->position.x<<" py "<<it->position.y<<" angle "<<it->angle<<" landingcharge "<<basedata[it->link].landingcharge<<" cost "<<basedata[it->link].cost<<endl;
			fileout<<"<contains>"<<endl<<endl<<"<ships>"<<endl;
			fileout<<basedata[it->link].numships<<endl;
			for(i=0;i<basedata[it->link].numships;i++)
			{
				fileout<<basedata[it->link].ships[i].filename<<endl;
			}
			fileout<<"</ships>"<<endl<<endl;
/*			fileout<<"<equipment>"<<endl;
			for(i=0;i<NUMEQUIPTYPES;i++)
			{
				for(j=0;j<basedata[it->link].equipment[i];j++)
				{
					GetEquipmentString(i, tempstring);	//Get the name of this piece of equipment
					fileout<<tempstring<<endl;
				}
			}
			fileout<<"</equipment>"<<endl<<endl;
			fileout<<"<weapons>"<<endl;
			for(i=0;i<numberofweapons;i++) if(basedata[it->link].weapon[i]) fileout<<weaponlist[i].name<<endl;
			fileout<<"</weapons>"<<endl<<endl<<endl;*/
			fileout<<"<text1>"<<endl;		//Find start of first text
			fileout<<basedata[it->link].txtarrive<<endl<<endl;
			fileout<<"<text2>"<<endl;		//Find start of second text
			fileout<<basedata[it->link].txtrevisit<<endl<<endl;
			fileout<<"</contains>"<<endl;
			fileout<<it->state<<endl;
			fileout<<"</group>"<<endl<<endl;
			break;
		case TELEPORT:
			fileout<<"<group>"<<endl;
			fileout<<"1 "<<it->origfilename<<endl;
			fileout<<"px "<<it->position.x<<" py "<<it->position.y<<" angle "<<it->angle<<" angvel "<<it->turnspeed<<" link "<<it->link<<endl;
			fileout<<it->state<<endl;
			fileout<<"</group>"<<endl<<endl;
			break;
		case CONVERTER:
		case GAMESAVER:
		case SHIELDCHARGER:
			fileout<<"<group>"<<endl;
			fileout<<"1 "<<it->origfilename<<endl;
			fileout<<"px "<<it->position.x<<" py "<<it->position.y<<" angle "<<it->angle<<endl;
			fileout<<it->state<<endl;
			fileout<<"</group>"<<endl<<endl;
			break;
		case EXITGATE:
			fileout<<"<group>"<<endl;
			fileout<<"1 "<<it->origfilename<<endl;
			fileout<<"px "<<it->position.x<<" py "<<it->position.y<<" angle "<<it->angle<<" angvel "<<it->turnspeed<<" level "<<it->storestring<<endl;
			fileout<<it->state<<endl;
			fileout<<"</group>"<<endl<<endl;
			break;
		}
	}

	fileout<<"</objects>"<<endl<<endl;
	fileout<<"<boundaries>"<<endl<<endl;

	for(i=0;i<level->boundaries;i++)
	{
		fileout<<"<boundary>"<<endl;
		fileout<<"points "<<level->boundary[i].numpoints<<endl;
		for(j=0;j<level->boundary[i].numpoints;j++) fileout<<"px "<<level->boundary[i].poly.vertex[j].x<<" py "<<level->boundary[i].poly.vertex[j].y<<endl;
		fileout<<level->boundary[i].state<<endl;
		fileout<<"</boundary>"<<endl<<endl;
	}
	fileout<<"</boundaries>"<<endl<<endl;

	fileout<<"<mesh>"<<endl;
	fileout<<"points "<<level->meshpoints<<endl;
	for(i=0;i<level->meshpoints;i++) fileout<<"px "<<level->mesh[i].position.x<<" py "<<level->mesh[i].position.y<<" continue"<<endl;
	fileout<<"</mesh>"<<endl<<endl;

	fileout.close();
	return true;
}

void PrepObjects(Level & level)
{
	list<GameObject>::iterator i=entity.begin();
	for(++i;i!=entity.end();++i)
	{
		i->cosangle = cos(i->angle);
		i->sinangle = -sin(i->angle);
		GridPosition(&*i);
		if(i->type != INERT && i->type<TELEPORT) level.grid[i->gridnumber].AddToGrid(&*i);
		RotateOutline(&(i->outline), &(i->rotatedoutline), i->cosangle, i->sinangle);
		TranslateOutline(&(i->rotatedoutline), &(i->predictedoutline), &(i->position));
		switch(i->type)
		{
			case ENEMY:
			case TURRET:
				i->unitvectorforward.x = -i->sinangle;
				i->unitvectorforward.y = -i->cosangle;
				break;
			default:
				break;
		}
	}
}

void Scan(ifstream& fp_in, char *text)
{
	string input_string;
	do {
		fp_in>>input_string;		//Read next line
	} while(input_string != text);
	fp_in.ignore();
}

int ReadSaveFile(char *filename, Level& level)
{
	using namespace std;
	int i, j, number, numbases = 0;
	std::ifstream::pos_type point, point2;
	char filenm[200], input_string[200], name[100];
	string input_st;
	string junk, morejunk;
	ifstream fp_in;

	sprintf_s(filenm, 200, "data\\%s.dat", filename);
	//dbout<<"Loading save file "<<filenm<<endl;
	fp_in.open(filenm, ios::in);  // declare and open

	if(!fp_in.is_open()) return false;

	fp_in>>junk>>playmode;

	for(j=0;j<playmode;j++)
	{
		fp_in>>junk>>ship[j].filename;
		if(!LoadShip(&ship[j],ship[j].filename)) dbout<<"Loading of ship "<<ship[j].filename<<" failed"<<endl;
		fp_in>>junk>>ship[j].energy;
		fp_in>>junk>>ship[j].power;
		fp_in>>junk>>ship[j].shields;
		fp_in>>junk>>junk;		//Weapon name
		for(i=0;i<numberofweapons;i++) if(strcmp(junk.c_str(),weaponlist[i].name.c_str())==0) ship[j].weapon = &weaponlist[i];
		fp_in>>junk>>ship[j].missiles;
		fp_in>>junk>>ship[j].forwardsspeed;
		fp_in>>junk>>ship[j].backwardsspeed;
		fp_in>>junk>>ship[j].turnspeed;
		fp_in>>junk>>ship[j].position.x>>ship[j].position.y;
		fp_in>>junk>>ship[j].unitvectorforward.x>>ship[j].unitvectorforward.y;
		fp_in>>junk>>ship[j].angle;
		fp_in>>junk>>ship[j].stealthed;
		fp_in>>junk>>ship[j].credits;
		fp_in>>junk;
		for(i=0;i<NUMEQUIPTYPES;i++)
		{
			fp_in>>number>>junk;
			if(strcmp(junk.c_str(),"yes")==0) ship[j].equipment[i] = true;
			else ship[j].equipment[i] = false;
		}
	}
	playmode--;

	//dbout<<"Read all ship data"<<endl;
	fp_in.ignore();

	for(i=0;i<NUMEQUIPTYPES;i++)
	{
		fp_in>>junk>>junk;
		if(junk == "yes") equipment[i] = true;
		else equipment[i] = false;
	}
	equipment[RADAR] = true;
	equipment[SCANNER] = true;

	Scan(fp_in,"<weapons>");
	while((fp_in>>junk,junk!="</weapons>"))	weaponlist[GetWeaponNumber(junk.c_str())].available = true;


	// Read all global level data

	//Read name part ***************************************************************
	Scan(fp_in,"<name>");
	fp_in.getline( level.name, 256 );	//Read level name
	dbout<<"Level name "<<level.name<<endl;

	//Read description part ***************************************************************
	Scan(fp_in,"<description>");
	fp_in.getline( level.description, 4096 );	//Read level name
	dbout<<"Mission: "<<level.description<<endl;

	//Read the next level part *****************************************************
	Scan(fp_in,"<next>");
	fp_in.getline( level.next, 256 );	//Read next level name
	dbout<<"Next level file "<<level.next<<endl;

	//Read tile set part ************************************************************
	Scan(fp_in,"<tileset>");
	fp_in.getline( level.tiles, 30 );	//Read tile set
	dbout<<"Tile set "<<level.tiles<<endl;


	//Read tile map part ************************************************************
	Scan(fp_in,"<tilemap>");

	fp_in>>junk>>level.width;					//Get width
	fp_in>>junk>>level.height;				//Get height
	level.maskx = level.width-1;
	level.masky = level.height-1;
	level.pixelwidth = level.width*TILESIZE;			//Width in pixels
	level.pixelheight = level.height*TILESIZE;			//Height in pixels
	level.fwidth = (float)level.pixelwidth;			//Full width
	level.fheight = (float)level.pixelheight;
	level.hwidth = level.fwidth/2.0;				//Half width
	level.hheight = level.fheight/2.0;
	level.mhwidth = -level.hwidth;					//Minus half width
	level.mhheight = -level.hheight;
	level.tilemap = new int[level.width*level.height];
	level.shiftleft = level.width;
	for(i=0;level.shiftleft>1;i++)
		level.shiftleft = level.shiftleft>>1;
	level.shiftleft = i;

	for(j=0;j<level.height;j++)
		for(i=0;i<level.width;i++)
			fp_in>>level.tilemap[i + j*level.width];	//Read tile map

	dbout<<"Width "<<level.width<<" Height "<<level.height<<endl;


	//Read music **********************************************************************
	Scan(fp_in,"<music>");
	fp_in>>level.music;
	//sprintf_s(level.music,256,"music\\%s.wma",input_string);

	//Read objects part ***************************************************************

	level.objectives = 0;

	// First time through -----------------------------------
	Scan(fp_in,"<objects>");	//Find start of objects section

	point = fp_in.tellg();		//Store file position to return to later

	BaseData junkbasedata;
	numbases = 0;
	//Loop over all groups of objects
	do 
	{
		fp_in>>input_st;			//Read next line
		if(input_st == "<group>")
		{
			int current;
			fp_in>>number>>name;
		#ifdef DEBUG
		dbout<<number<<" "<<name<<"s"<<endl;
		#endif
			fp_in.ignore();				//Ignore newline in input buffer

			ReadObject(&entity.front(),name);

			current = entity.size();
			number += current;

			list<GameObject>::iterator it;
			for(i=current;i<number;i++)
			{
				streampos fileposition;

				entity.push_back(0);

				CopyObject(&entity.front(),&entity.back());

				it = entity.end();
				--it;
				it->objective = 0;
				fp_in>>junk>>it->position.x>>junk>>it->position.y>>junk>>it->angle;
				switch(it->type)
				{
					case PLAYER:
						break;
					case MINE:
						fp_in>>junk>>it->sensorrange>>junk>>it->timer>>junk>>it->objective;
						it->sensorrange*=screenratio;
						it->radarcolour = 0xffff40A0;
						break;
					case SPAWNINERT:
						fp_in>>junk>>it->turnspeed>>junk>>it->lastfired;
						it->lastfired = thistick - it->lastfired;
						it->radarcolour = 0;
						it->lastfired = 0;
						break;
					case INERT:
						fp_in>>junk>>it->turnspeed;
						it->radarcolour = 0;
						break;
					case SPINNER:
					case TURRET:
						fp_in>>junk>>it->objective;
						it->radarcolour = 0xffff0000;
						break;
					case ENEMY:
						fp_in>>junk>>it->rangelimit>>junk>>it->objective;
						it->initialposition = it->position;
						it->radarcolour = 0xffff0000;
						break;
					case BUTTONSWITCH:
						it->radarcolour = 0xff00ffff;
						break;
					case TRIGGERSWITCH:
						//Read text, trigger,  text, time out,  text, boundary
						fp_in>>junk>>morejunk>>junk>>it->timer>>junk>>it->link;
						it->lastfired = 0;
						it->radarcolour = 0;
						SET(it->state,ARMED);
						if(strcmp(morejunk.c_str(),"enemy")==0) SET(it->state,TRIGGEREDBYENEMY);
						else if(strcmp(morejunk.c_str(),"rock")==0) SET(it->state,TRIGGEREDBYROCK);
						else if(strcmp(morejunk.c_str(),"player")==0) SET(it->state,TRIGGEREDBYPLAYER);
						fp_in>>junk>>it->lastfired;
						break;
					case BASE:
						if(numbases==10) break;		//Not good code - will do something bad if more than ten bases
						fp_in>>junk>>basedata[numbases].landingcharge>>junk>>basedata[numbases].cost;
						do { fp_in>>junk; } while(strcmp(junk.c_str(),"<ships>")!=0);	//Get the start of the ships bit

						int readnumber; fp_in>>readnumber;
						//dbout<<"Read "<<readnumber<<endl;
						if(readnumber == 0)
						{
							basedata[numbases].numships = 1;		//Always available
							sprintf_s(basedata[numbases].ships[0].filename,200,"gambit_g550");
						}
						else
						{
							basedata[numbases].numships = readnumber;
							//dbout<<basedata[numbases].numships<<" ships"<<endl;

							for(int k=0;k<readnumber;k++)
							{
								fp_in>>basedata[numbases].ships[k].filename;
								//dbout<<numbases<<" "<<k<<" "<<basedata[numbases].ships[k].filename<<endl;
							}
						}

		/*				do { fp_in>>junk; } while(strcmp(junk.c_str(),"<equipment>"));	//Find start of equipment
						for(int k=0;k<NUMEQUIPTYPES;k++) basedata[numbases].equipment[k] = 0;	//Clear equipment
						while((fp_in>>junk,strcmp(junk.c_str(),"</equipment>")!=0))
						{
							int etype = GetEquipmentType((char *)junk.c_str());
							if(etype!=RADAR && etype!=SCANNER) basedata[numbases].equipment[etype]++;
						}

						do { fp_in>>junk; } while(strcmp(junk.c_str(),"<weapons>")!=0);	//Find start of weapons
						for(int k=0;k<numberofweapons;k++) basedata[numbases].weapon[k] = false;	//Clear weapons
						while((fp_in>>junk,strcmp(junk.c_str(),"</weapons>")!=0))
						{
							int etype = GetWeaponNumber((char *)junk.c_str());
							if(etype==numberofweapons) continue;
							basedata[numbases].weapon[etype] = true;
						}*/
						do { fp_in>>junk; } while(strcmp(junk.c_str(),"<text1>")!=0);	//Find start of first text
						fp_in.ignore();
						fp_in.getline(basedata[numbases].txtarrive,500);

						do { fp_in>>junk; } while(strcmp(junk.c_str(),"<text2>")!=0);	//Find start of second text
						fp_in.ignore();
						fp_in.getline(basedata[numbases].txtrevisit,500);

						do { fp_in>>junk; } while(strcmp(junk.c_str(),"</contains>")!=0);	//Find end of section
						it->radarcolour = 0xffffffff;
						it->link = numbases++;
						break;
					case EXITGATE:
						fp_in>>junk>>it->turnspeed>>junk>>it->storestring;		//Exit gate can spin, take to bonus levels
						it->radarcolour = 0xffffffff;
						break;
					case CONVERTER:
						it->radarcolour = 0;
						break;
					case GAMESAVER:
					case SHIELDCHARGER:
						it->radarcolour = 0xff00aa00;
						break;
					case TELEPORT:
						fp_in>>junk>>it->turnspeed>>junk>>it->link;	//May adjust this so that if link is zero, the teleporter can have a target position
						it->radarcolour = 0xff00ffff;
						break;
					case SPAWN:
						fp_in>>junk>>it->unitvectorforward.x>>junk>>it->unitvectorforward.y>>junk>>it->turnspeed>>junk>>it->objective;
						it->forwardsspeed = Normalise(&(it->unitvectorforward));
						it->radarcolour = 0xffff9855;
						for(int loopy=0;loopy<it->childobjects;loopy++)
							if(it->childobject[loopy].type==TURRET)
							{
								it->radarcolour = 0xffff0000;
								break;
							}
						fp_in>>junk>>it->lastfired;
						it->lastfired = thistick - it->lastfired;
						break;
					case ROCK:
						fp_in>>junk>>it->unitvectorforward.x>>junk>>it->unitvectorforward.y>>junk>>it->turnspeed>>junk>>it->objective;
						it->forwardsspeed = Normalise(&(it->unitvectorforward));
						it->radarcolour = 0xffff9855;
						for(int loopy=0;loopy<it->childobjects;loopy++)
							if(it->childobject[loopy].type==TURRET)
							{
								it->radarcolour = 0xffff0000;
								break;
							}
						break;
					case ENERGY:
						fp_in>>junk>>it->energy>>junk>>it->turnspeed;
						it->objective = 0;
						it->forwardsspeed = 0;
						//it->radarcolour = 0xff00aa00;
						break;
					case BLUEPRINT:
						fp_in>>junk>>it->turnspeed>>junk>>it->storestring;
						if(junk=="equipment")
						{
							it->link = GetEquipmentType(it->storestring.c_str());
							SET(it->state,STOREEQUIPMENT);
						}
						else if(junk=="weapon")
						{
							it->link = GetWeaponNumber(it->storestring.c_str());
							if(it->link<0) it->link = 0;
							SET(it->state,STOREWEAPON);
						}
						else it->link = 0;
						it->turnspeed = it->turnspeed *(TWOPI/360.0f);
						it->objective = 0;
						it->forwardsspeed = 0;
						break;
					case MISSILECRATE:
						fp_in>>junk>>it->link>>junk>>it->turnspeed;
						it->objective = 0;
						it->forwardsspeed = 0;
						//it->radarcolour = 0xff00aa00;
						break;
					case GEMSTONE:
						fp_in>>junk>>it->link>>junk>>it->turnspeed;
						it->objective = 0;
						it->forwardsspeed = 0;
						//it->radarcolour = 0xff00aa00;
						break;
				}
				if(it->type<TELEPORT || it->type==BUTTONSWITCH)
				{
					fileposition = fp_in.tellg();
					fp_in>>junk;
					if(strcmp(junk.c_str(),"setboundary")==0)
					{
						fp_in>>it->link;
						SET(it->state,LINKEDTOBOUNDARY);
						//dbout<<"     ------   "<<it->name<<" linked to boundary "<<it->link<<endl;
					}
					else if(strcmp(junk.c_str(),"fromboundary")==0)
					{
						fp_in>>it->link;
						SET(it->state,LINKEDFROMBOUNDARY);
						//dbout<<"     ------   "<<it->name<<" linked from boundary "<<it->link<<endl;
					}
					else fp_in.seekg(fileposition);
				}
				fp_in>>it->state;		//Read the object's state
				dbout<<it->name<<" state "<<it->state<<endl;
				it->cosangle = cos(it->angle);
				it->sinangle = sin(it->angle);
				ReSumChildAngles(&*it, it->angle);
				it->initialposition = it->position;
				it->target = it->position;
				it->graphics = &error;		//For safety
				it->changeangle = 0;
				it->velocity.x = 0;
				it->velocity.y = 0;
				if(it->objective) ++level.objectives;
				//dbout<<"Object \""<<it->name<<"\" done"<<endl;
			}
			//dbout<<"Group \""<<it->name<<"\" completed"<<endl;

		}
		else if(input_st=="</objects>") break;
	} while(1);

	//dbout<<"Objects read, "<<level.objectives<<" objectives"<<endl;

	point = fp_in.tellg();		//Store file position to return to later

	//Find how many sets of boundaries there are
	level.boundaries = 0;
	fp_in.ignore();
	do {
		fp_in.getline( input_string, 200 );		//Read next line
		if(strncmp(input_string,"<boundary>",11)==0)
			level.boundaries++;
		else if(strncmp(input_string,"</boundaries>",14)==0) break;
	} while(1);

	//dbout<<level.boundaries<<" boundaries with: ";

	if(level.boundaries>0)
	{
		int counter=0;
		level.boundary = new Boundary[level.boundaries];

		for(i=0;i<level.boundaries;i++)
			level.boundary[i].state = ON;

		//Now we return to the boundaries part of the file
		fp_in.seekg(point);

		do {
			fp_in.getline( input_string, 200 );		//Read next line
			if(strncmp(input_string,"<boundary>",11)==0)
			{
				fp_in>>junk>>level.boundary[counter].numpoints;
				//dbout<<level.boundary[counter].numpoints<<", ";
				level.boundary[counter].poly.SetPoints(level.boundary[counter].numpoints);
				for(i=0;i<level.boundary[counter].numpoints;i++)
					fp_in>>junk>>level.boundary[counter].poly.vertex[i].x>>junk>>level.boundary[counter].poly.vertex[i].y;

				fp_in>>level.boundary[counter].state;
				//dbout<<counter<<" "<<level.boundary[counter].state<<endl;
				fp_in.ignore();
				counter++;
			}
			else if(strncmp(input_string,"</boundaries>",14)==0 || fp_in.eof()) break;
		} while(1);
	}
	else level.boundary = NULL;

	fp_in.ignore();
	do {
		fp_in.getline( input_string, 200 );		//Read next line
		if(strncmp(input_string,"<mesh>",8)==0)
		{
			fp_in>>junk>>level.meshpoints;
			//dbout<<endl<<level.meshpoints<<" navigation mesh points"<<endl;
			level.SetMeshPoints();
			char actionname[20];
			for(i=0;i<level.meshpoints;i++)
			{
				fp_in>>junk>>level.mesh[i].position.x>>junk>>level.mesh[i].position.y>>actionname;
				_strupr(actionname);
				if(strncmp(actionname,"CONTINUE",20)==0) level.mesh[i].action = CONTINUE;
				if(strncmp(actionname,"WAIT",20)==0) level.mesh[i].action = WAIT;
				if(strncmp(actionname,"OPERATE",20)==0) level.mesh[i].action = OPERATE;
				level.mesh[i].number = i;
			}
			fp_in.ignore();
		}
		else if(strncmp(input_string,"</mesh>",8)==0 || fp_in.eof()) break;
	} while(1);
	//dbout<<"Save file read .. BO!"<<endl;
	fp_in.close();

	return tilegfx.LoadTiles(level.tiles);
	//return LoadTileSet(level.tiles, level);	//If this fails then everything fails
}

void SortWeapons()
{
	int i, j;

	//For each weapon
	for(i=0;i<numberofweapons;i++)
	{
		//dbout<<"Checking weapon "<<i<<" with:"<<endl;
		//Run over all other weapons
		for(j=i+1;j<numberofweapons;j++)
		{
			//dbout<<j<<endl;
			//If j power is less than or equal to i power, swap them
			if(weaponlist[j].power<=weaponlist[i].power)
			{
				//dbout<<"Swapping weapon "<<i<<" ("<<weaponlist[i].power<<") with "<<j<<" ("<<weaponlist[j].power<<")"<<endl;
				SwapWeapons(&weaponlist[i],&weaponlist[j]);
			}
		}
	}
}

void SwapWeapons(Weapon *a, Weapon * b)
{
	Weapon temp;
	CopyWeapon(&temp,a);		//temp = a
	CopyWeapon(a,b);		//a = b
	CopyWeapon(b,&temp);		//b = temp
}

//Copies b to a  ...  a = b
void CopyWeapon(Weapon *a, Weapon *b)
{
	//a->number = b->number;

	a->bounce = b->bounce;
	a->charge = b->charge;
	a->explode = b->explode;
	a->trail = b->trail;
	a->available = b->available;

	a->energy = b->energy;
	a->faderate = b->faderate;
	a->firerate = b->firerate;
	a->fadevalue = b->fadevalue;
	a->speed = b->speed;
	a->power = b->power;
	a->trailalpha = b->trailalpha;
	a->trailfaderate = b->trailfaderate;
	a->trailgrowrate = b->trailgrowrate;
	a->bulletgrowrate = b->bulletgrowrate;
	a->bulletscale = b->bulletscale;
	a->scalecentre = b->scalecentre;

	a->trailgfx = b->trailgfx;
	a->graphics = b->graphics;
	a->pulsegraphics = b->pulsegraphics;

	a->name = b->name;
	a->description = b->description;
	a->graphicsfile = b->graphicsfile;
	a->pulsefile = b->pulsefile;
	a->soundfile = b->soundfile;
//	strcpy_s(a->name,100,b->name);
//	strcpy_s(a->description,500,b->description);
//	strcpy_s(a->graphicsfile,100,b->graphicsfile);
//	strcpy_s(a->soundfile,100,b->soundfile);
	//a->sound = b->sound;
}


void CalculateGrid(Level & level)
{
	int gridx, gridy, numsquares, i, j, k, l, boundaries, nx, ny;
	bool *boundarylist;
	bool extrax = false, extray = false;
	tPolygon outline(4), soutline(4);

	boundarylist = new bool[level.boundaries];

	gridx = level.width/GRIDWIDTH;			//Divide by GRIDWIDTH and add one if there's a remainder
	gridy = level.height/GRIDWIDTH;			//level.width is the width in tiles, not in pixels
	if(level.width%GRIDWIDTH>0)
	{
		++gridx;
		extrax = true;
	}
	if(level.height%GRIDWIDTH>0)
	{
		++gridy;
		extray = true;
	}

	numsquares = gridx*gridy;
	level.gridsize.x = gridx;
	level.gridsize.y = gridy;
	level.gridsquares = numsquares;
	//dbout<<"Grid: "<<gridx<<" by "<<gridy<<endl;

	level.grid = new GridSquare[numsquares];	//Allocate memory
	for(j=0;j<gridy;j++)
		for(i=0;i<gridx;i++)
		{
			level.grid[j*gridx + i].pos.x = i*GRIDSIZE;
			level.grid[j*gridx + i].pos.y = j*GRIDSIZE;
			if(i<gridx-1 || (i==(gridx-1) && !extrax) ) level.grid[j*gridx + i].size.x = GRIDSIZE;
			else level.grid[j*gridx + i].size.x = level.pixelwidth - GRIDSIZE*i;
			if(j<gridy-1 || (j==(gridy-1) && !extray) ) level.grid[j*gridx + i].size.y = GRIDSIZE;
			else level.grid[j*gridx + i].size.y = level.pixelheight - GRIDSIZE*j;

			nx = i - 1;
			if(nx<0) nx = gridx - 1;
			level.grid[j*gridx + i].neighbour[LEFT] = &level.grid[j*gridx + nx];

			ny = j - 1;
			if(ny<0) ny = gridy - 1;
			level.grid[j*gridx + i].neighbour[TOPLEFT] = &level.grid[ny*gridx + nx];
			level.grid[j*gridx + i].neighbour[TOP] = &level.grid[ny*gridx + nx];

			nx = i + 1;
			if(nx == gridx) nx = 0;
			level.grid[j*gridx + i].neighbour[TOPRIGHT] = &level.grid[ny*gridx + nx];
			level.grid[j*gridx + i].neighbour[RIGHT] = &level.grid[j*gridx + nx];

			ny = j + 1;
			if(ny==gridy) ny = 0;
			level.grid[j*gridx + i].neighbour[BOTTOMRIGHT] = &level.grid[ny*gridx + nx];
			level.grid[j*gridx + i].neighbour[BOTTOM] = &level.grid[ny*gridx + i];

			nx = i - 1;
			if(nx<0) nx = gridx - 1;
			level.grid[j*gridx + i].neighbour[BOTTOMLEFT] = &level.grid[ny*gridx + nx];
		}

	for(j=0;j<gridy;j++)	//Loop over y grid squares
	{
		for(i=0;i<gridx;i++)	//Loop over x grid squares
		{
			for(k=0;k<level.boundaries;k++) boundarylist[k] = false;	//Clear boundary list

			for(int n=i-1;n<i+2;n++)	//Loop over adjacent x squares
			{
				for(int m=j-1;m<j+2;m++)	//Loop over adjacent y squares
				{
					//dbout<<"Setting outline for "<<n<<", "<<m<<endl;
					SetOutline(&outline,n,m,level);

					//boundaries = 0;
					for(k=0;k<level.boundaries;k++)
					{
						if(FullPolyCollisionDetect(&outline,&level.boundary[k].poly)) boundarylist[k] = true;	//Lines collision
						if(PolyCollisionDetect(&outline,&level.boundary[k].poly)) boundarylist[k] = true;		//Points collision
					}
				}
			}
			boundaries = 0;
			for(k=0;k<level.boundaries;k++) if(boundarylist[k] == true) boundaries++;

			//dbout<<i<<", "<<j<<" total "<<boundaries<<" boundaries"<<endl;

			level.grid[j*gridx + i].boundary = new int[boundaries];
			level.grid[j*gridx + i].boundaries = boundaries;
			for(k=0,l=0;k<level.boundaries;k++)
			{
				if(boundarylist[k])
				{
					//dbout<<k<<" ";
					level.grid[j*gridx + i].boundary[l] = k;
					l++;
				}
			}
		}
	}

	//dbout<<"Grid done"<<endl;
	delete [] boundarylist;

	return;
}

void SetOutline(tPolygon *outline, int gridi, int gridj, Level & level)
{
	int gridx = level.gridsize.x;

	if(gridi>=level.gridsize.x) gridi = 0;
	if(gridi<0) gridi = level.gridsize.x - 1;
	if(gridj>=level.gridsize.y) gridj = 0;
	if(gridj<0) gridj = level.gridsize.y - 1;

	//dbout<<"Setting outline for "<<gridi<<", "<<gridj<<endl;

	outline->vertex[0].x = level.grid[gridj*gridx + gridi].pos.x;
	outline->vertex[0].y = level.grid[gridj*gridx + gridi].pos.y;

	outline->vertex[1].x = outline->vertex[0].x + level.grid[gridj*gridx + gridi].size.x;
	outline->vertex[1].y = outline->vertex[0].y;

	outline->vertex[2].x = outline->vertex[1].x;
	outline->vertex[2].y = outline->vertex[1].y + level.grid[gridj*gridx + gridi].size.y;

	outline->vertex[3].x = outline->vertex[0].x;
	outline->vertex[3].y = outline->vertex[2].y;
}

bool CopyTileFromTexture(GraphicalObject *tile, LPDIRECT3DTEXTURE9 tilesheet, int xtile, int ytile)
{
	int i, j;
	int xread = xtile * 256;	//4 * 64
	int yread = ytile * 256;
	DWORD sheetpitch, tilepitch, *lpsheetdata, *lptiledata, sheetoffset, tileoffset, pixel;
	D3DLOCKED_RECT tilelocked, sheetlocked;

	if(FAILED(tilesheet->LockRect(0,&sheetlocked,NULL,NULL)))
	{
		dbout<<"Locking tile texture failed"<<endl;
		return false;
	}

	if(FAILED(tile->texture->LockRect(0,&tilelocked,NULL,NULL)))
	{
		dbout<<"Locking tile failed"<<endl;
		return false;
	}

	sheetpitch = sheetlocked.Pitch/4;
	lpsheetdata = (DWORD *)sheetlocked.pBits;

	tilepitch = tilelocked.Pitch/4;
	lptiledata = (DWORD *)tilelocked.pBits;

	for(j=0;j<256;j+=4)
	{
		for(i=0;i<256;i+=4)
		{
			sheetoffset = (xread + i) + ((yread + j)*sheetpitch);
			tileoffset = i + j*tilepitch;
			ReadPixel(lpsheetdata,sheetoffset,pixel);
			WritePixel(lptiledata,tileoffset,pixel);
		}
	}

	if(FAILED(tilesheet->UnlockRect(0)))
	{
		dbout<<"Unlocking tile texture failed"<<endl;
		return false;
	}

	if(FAILED(tile->texture->UnlockRect(0)))
	{
		dbout<<"Unlocking tile failed"<<endl;
		return false;
	}

	return true;
}

bool MoveTile(LPDIRECT3DTEXTURE9 sourcesheet, LPDIRECT3DTEXTURE9 destsheet, int sxtile, int sytile, int dxtile, int dytile)
{
	int i, j;
	int xread = sxtile * 256;	//4 * 64
	int yread = sytile * 256;
	int xwrite = dxtile * 256;
	int ywrite = dytile * 256;
	DWORD sourcepitch, destpitch, *lpsourcedata, *lpdestdata, sourceoffset, destoffset, pixel;
	D3DLOCKED_RECT sourcelocked, destlocked;

	if(FAILED(sourcesheet->LockRect(0,&sourcelocked,NULL,NULL)))
	{
		dbout<<"Locking source texture failed"<<endl;
		return false;
	}

	if(FAILED(destsheet->LockRect(0,&destlocked,NULL,NULL)))
	{
		sourcesheet->UnlockRect(0);
		dbout<<"Locking destination texture failed"<<endl;
		return false;
	}

	sourcepitch = sourcelocked.Pitch/4;
	lpsourcedata = (DWORD *)sourcelocked.pBits;

	destpitch = destlocked.Pitch/4;
	lpdestdata = (DWORD *)destlocked.pBits;

	for(j=0;j<256;j+=4)
	{
		for(i=0;i<256;i+=4)
		{
			sourceoffset = (xread + i) + ((yread + j)*sourcepitch);
			destoffset = (xwrite + i) + ((ywrite + j)*destpitch);
			ReadPixel(lpsourcedata,sourceoffset,pixel);
			WritePixel(lpdestdata,destoffset,pixel);
		}
	}

	if(FAILED(destsheet->UnlockRect(0)))
	{
		dbout<<"Unlocking dest texture failed"<<endl;
		return false;
	}

	if(FAILED(sourcesheet->UnlockRect(0)))
	{
		dbout<<"Unlocking source texture failed"<<endl;
		return false;
	}

	return true;
}

bool Level::CreateMap()
{
	bool widthchanged = false;
	bool heightchanged = false;

	newwidth = width;
	newheight = height;

	//Work out how big the texture needs to be - upsize to a power of two
	if(!IsPowerOfTwo(width))
	{
		newwidth = LowestPowerTwo(width)<<1;
		widthchanged = true;
		dbout<<"Upsized level from: "<<width<<" to "<<newwidth<<endl;
	}
	xdifference = newwidth - width;
	newpixelwidth = newwidth*TILESIZE;

	if(!IsPowerOfTwo(height))
	{
		newheight = LowestPowerTwo(height)<<1;
		heightchanged = true;
		dbout<<"Upsized level from: "<<height<<" to "<<newheight<<endl;
	}
	ydifference = newheight - height;
	newpixelheight = newheight*TILESIZE;

	//Create map texture for shader
	D3DLOCKED_RECT locked;
	if(FAILED(D3DXCreateTexture(dx9.pd3dDevice, newwidth, newheight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &map)))
	{
		dbout<<"**** WARNING! MAP NOT CREATED!!! ****"<<endl;
		map = NULL;
		return false;
	}
	else
	{
		map->LockRect(0, &locked, NULL, 0);
	}

	DWORD mappitch = locked.Pitch;
	DWORD *lpmapdata = (DWORD *)locked.pBits;
	DWORD mapoffset = 0;
	int pixel;
	int ipos, jpos;
	RECT mapr, maptex;

	mapr.left = 0;
	mapr.right = width;
	mapr.top = 0;
	mapr.bottom = height;
	CopyMap(&mapr, &mapr, lpmapdata, mappitch, *this);	//Copy map normally

	if(!widthchanged && !heightchanged) return true;

	maptex = mapr;

	if(widthchanged)		//Expand it horizontally
	{
		maptex.left = width;
		maptex.right = width<<1;
		CopyMap(&mapr, &maptex, lpmapdata, mappitch, *this);		//Copy map centre part

		maptex.left = newwidth - width;
		maptex.right = newwidth;
		CopyMap(&mapr, &maptex, lpmapdata, mappitch, *this);		//Copy map right part
	}

	if(heightchanged)		//Expand it vertically
	{
		maptex.left = 0;
		maptex.right = width;
		maptex.top = height;
		maptex.bottom = height<<1;
		CopyMap(&mapr, &maptex, lpmapdata, mappitch, *this);		//Copy map centre part

		maptex.top = newheight - height;
		maptex.bottom = newheight;
		CopyMap(&mapr, &maptex, lpmapdata, mappitch, *this);		//Copy map right part
	}

	if(widthchanged && heightchanged)		//Do bottom right corner
	{
		maptex.left = width;
		maptex.right = width<<1;
		maptex.top = newheight - height;
		maptex.bottom = newheight;
		CopyMap(&mapr, &maptex, lpmapdata, mappitch, *this);		//Copy map - bottom middle row

		maptex.left = newwidth - width;
		maptex.right = newwidth;
		maptex.top = height;
		maptex.bottom = height<<1;
		CopyMap(&mapr, &maptex, lpmapdata, mappitch, *this);		//Copy map - right middle row

		maptex.left = width;
		maptex.right = width<<1;
		maptex.top = height;
		maptex.bottom = height<<1;
		CopyMap(&mapr, &maptex, lpmapdata, mappitch, *this);		//Copy map - middle portion

		maptex.left = newwidth - width;
		maptex.right = newwidth;
		maptex.top = newheight - height;
		maptex.bottom = newheight;
		CopyMap(&mapr, &maptex, lpmapdata, mappitch, *this);		//Copy map - bottom right portion
	}

	if(map != NULL)
	{
		map->UnlockRect(0);
		dbout<<"**** MAP CREATED!!! ****"<<endl;
	}
	return true;
}

bool CopyMap(const RECT *map, const RECT *maptexture, DWORD* mapdata, DWORD mappitch, Level& level)
{
	DWORD mapoffset, pixel;
	int mapwidth = map->right - map->left;
	int mapheight = map->bottom - map->top;
	int maptexwidth = maptexture->right - maptexture->left;
	int maptexheight = maptexture->bottom - maptexture->top;

	if(mapwidth != maptexwidth) return false;
	if(mapheight != maptexheight) return false;

	for(int j=map->top, k=maptexture->top; k<maptexture->bottom; j++,k++)
	{
		for(int i=map->left, h=maptexture->left; h<maptexture->right; i++,h++)
		{
			mapoffset = (h<<2) + k*mappitch;
			pixel = ((level.tilemap[i + j*mapwidth]<<24)&0xff000000) + 
					((level.tilemap[i + j*mapwidth]<<16)&0xff0000) +
					((level.tilemap[i + j*mapwidth]<<8)&0xff00) +
					((level.tilemap[i + j*mapwidth])&0xff);			//Pixel value
			WritePixel(mapdata,mapoffset,pixel);			//Create tile map texture
		}
	}
	return true;
}

//This function takes the tile being tested and fills in the correct texture coordinates
//depending on the size of the tile sheet and the tile's index
void TileGraphics::GetTextureCoordinates(TestTile *tile)
{
	int x, y;

	//Work out x and y positions on sheet - we have already made sure xtiles is a power of two so
	//we can do tilenumber&(xtiles-1) rather than tilenumber%xtiles, and tilenumber>>xshift rather than
	// tilenumber/xtiles
	x = tile->tilenumber&(xtiles-1);
	y = tile->tilenumber>>xshift;

	//Work out texture coordinates
	tile->corner[0].texturecoord.x = (float)x*xtextureincrement + textureshrinkx;
	tile->corner[0].texturecoord.y = (float)y*ytextureincrement + textureshrinky;

	tile->corner[1].texturecoord.x = (float)(x+1)*xtextureincrement - textureshrinkx;
	tile->corner[1].texturecoord.y = (float)y*ytextureincrement + textureshrinky;

	tile->corner[2].texturecoord.x = (float)(x+1)*xtextureincrement - textureshrinkx;
	tile->corner[2].texturecoord.y = (float)(y+1)*ytextureincrement - textureshrinky;

	tile->corner[3].texturecoord.x = (float)x*xtextureincrement + textureshrinkx;
	tile->corner[3].texturecoord.y = (float)(y+1)*ytextureincrement - textureshrinky;
}


//This function must be run as soon as the game starts, as the triggers are timer-based
void SetUpTriggers()
{
	dbout<<"Setting triggers"<<endl;
	list<GameObject>::iterator it;
	for(it=entity.begin();it!=entity.end();++it)
	{
		if(it->type==TRIGGERSWITCH)	it->lastfired = thistick - it->lastfired;
	}
	dbout<<"Done"<<endl;
}