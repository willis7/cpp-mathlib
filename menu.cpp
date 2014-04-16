//Menu handling functions

#include "OR.h"

enum MENUS { QUIT, MAINMENU, SOUNDMENU, GRAPHICSMENU, OPTIONSMENU, CONTROLMENU, CREDITS, STORY, LOADMENU, SCREENMENU, EXITMENU };

extern int gamemode, playmode;
extern int thistick, deadtimer;
extern Player ship[2];
extern int g_screenx, g_screeny;
extern ofstream dbout;
extern DX9VARS dx9;
extern Scene scene;
extern bool gamerunning, restartlevel;
extern SoundEffect soundeffect[NUMEFFECTS];
extern Options options;
extern float screenratio;

extern IBasicAudio*	g_pBasicAudio;

extern GraphicalObject font, menufont, projperilogo, menubg;

int xpos = 100, ypos = 50;

bool Menu()
{
	static int whichmenu = MAINMENU;

	xpos = 100.0f*screenratio;
	ypos = 50.0f*screenratio;

	if( dx9.pd3dDevice == NULL)
		return false;

	scene.SceneBegin(&dx9.MainViewPort);

	DrawGraphicalObject(&menubg,g_screenx/2,g_screeny/2,0,0,screenratio*1.171875f,true,0xffffffff);
	DrawGraphicalObject(&projperilogo,g_screenx/2,g_screeny/2,0,0,screenratio,true,0x80ffffff);		//Display logo

	switch(whichmenu)
	{
	case MAINMENU:
		whichmenu = MainMenu();			break;
	case SOUNDMENU:
		whichmenu = SoundMenu();		break;
	case GRAPHICSMENU:
		whichmenu = GraphicsMenu();		break;
	case OPTIONSMENU:
		whichmenu = OptionsMenu();		break;
	case CONTROLMENU:
		whichmenu = ControlMenu();		break;
	case CREDITS:
		whichmenu = Credits();			break;
	case STORY:
		whichmenu = Story();			break;
	case LOADMENU:
		whichmenu = LoadGameMenu();		break;
	case SCREENMENU:
		whichmenu = ScreenModeMenu();	break;
	case EXITMENU:
		whichmenu = ExitMenu();			break;
	case QUIT:
		return false;
	}

	scene.SceneFlip();
	PlaySounds();

	return true;
}


bool SelectPlay(int players, bool newgame)
{
	gamemode = OUTRO;
	playmode = players;
	if(newgame)
	{
		if(gamerunning) ClearCurrentLevel();
		else CleanUpDirectShow();

		if(!InitialiseGame(false)) return false;
		gamerunning = true;
	}
	//dbout<<"Game initialised"<<endl;
	thistick = timeGetTime();

	return true;
}

int MainMenu()
{
	static int menuselector = 0;
	int maxmenu = (gamerunning?9:7);

	DrawString(&menufont, "Start New Single Player Game", xpos, ypos, g_screenx, menuselector==0?REDHIGHLIGHT:GREEN);
	DrawString(&menufont, "Start New Two Player Game", xpos, ypos+screenratio*21, g_screenx, menuselector==1?REDHIGHLIGHT:GREEN);
	DrawString(&menufont, "Load Saved Game", xpos, ypos+screenratio*42, g_screenx, menuselector==2?REDHIGHLIGHT:GREEN);
	DrawString(&menufont, "Game Options", xpos, ypos+screenratio*63, g_screenx, menuselector==3?REDHIGHLIGHT:GREEN);
	DrawString(&menufont, "Controls", xpos, ypos+screenratio*84, g_screenx, menuselector==4?REDHIGHLIGHT:GREEN);
	DrawString(&menufont, "Credits", xpos, ypos+screenratio*105, g_screenx, menuselector==5?REDHIGHLIGHT:GREEN);
	DrawString(&menufont, "Story", xpos, ypos+screenratio*126, g_screenx, menuselector==6?REDHIGHLIGHT:GREEN);
	DrawString(&menufont, "Exit", xpos, ypos+screenratio*147, g_screenx, menuselector==7?REDHIGHLIGHT:GREEN);
	if(gamerunning)
	{
		DrawString(&menufont, "Resume", xpos, ypos+screenratio*171, g_screenx, menuselector==8?REDHIGHLIGHT:RED);
		DrawString(&menufont, "Restart Current Level", xpos, ypos+screenratio*192, g_screenx, menuselector==9?REDHIGHLIGHT:RED);
	}

	if(TestKeyPressed())
	{
		switch(menuselector)
		{
		case 0:		//New single player game
			SelectPlay(SINGLEPLAYER, true);
			return MAINMENU;
		case 1:		//New two player game
			SelectPlay(TWOPLAYER, true);
			return MAINMENU;
		case 2:		//Load game menu
			return LOADMENU;
		case 3:		//Options menu
			return OPTIONSMENU;
		case 4:		//Control Menu
			return CONTROLMENU;
		case 5:		//Credits
			return CREDITS;
		case 6:		//Credits
			return STORY;
		case 7:		//Exit
			return EXITMENU;
		case 8:		//Resume current game
			gamemode = PLAY;
			ResetTimers();
			return MAINMENU;
		case 9:		//Restart current level
			gamemode = GAMEOVER;
			deadtimer = 0;
			restartlevel = true;
			if(!InitialiseGame(true)) return MAINMENU;
			gamerunning = true;
			return MAINMENU;
		}
	}

	if(ship[options.keyboardplayer].keys.DOWN || ship[options.gamepadplayer].keys.DOWN)
	{
		ship[options.keyboardplayer].keys.DOWN = false;
		ship[options.gamepadplayer].keys.DOWN = false;
		++menuselector;
	}
	if(ship[options.keyboardplayer].keys.UP || ship[options.gamepadplayer].keys.UP)
	{
		ship[options.keyboardplayer].keys.UP = false;
		ship[options.gamepadplayer].keys.UP = false;
		--menuselector;
	}

	if(menuselector<0) menuselector = maxmenu;
	else if(menuselector>maxmenu) menuselector = 0;

	return MAINMENU;
}

int SoundMenu()
{
	char tempstring[200];
	static int menuselector = 0;
	int maxmenu = 2;
//	sprintf_s(tempstring,200,"Music: %s",musicon?"On":"Off");
//	DrawString(&menufont, tempstring, xpos, ypos, g_screenx, menuselector==0?GREENHIGHLIGHT:GREEN);
	sprintf_s(tempstring,200,"Music Volume:   ...........");
	tempstring[(int)(options.musicvolume*10.0)+16] = 124;
	DrawString(&menufont, tempstring, xpos, ypos, g_screenx, menuselector==0?REDHIGHLIGHT:GREEN);
	sprintf_s(tempstring,200,"Effects Volume: ...........");
	tempstring[(int)(options.soundvolume*10.0)+16] = 124;
	DrawString(&menufont, tempstring, xpos, ypos+screenratio*21, g_screenx, menuselector==1?REDHIGHLIGHT:GREEN);
	DrawString(&menufont, "Main Menu", xpos, ypos+screenratio*42, g_screenx, menuselector==2?REDHIGHLIGHT:GREEN);

	if(TestKeyPressed())
	{
		switch(menuselector)
		{
/*		case 0:		//Music Switch
			musicon = !musicon;
			break;*/
		case 0:		//Music Volume
			options.musicvolume += 0.1f;
			if(options.musicvolume>1.05f) options.musicvolume = 0;
			g_pBasicAudio->put_Volume(soundfloattoint(options.musicvolume*0.8f));
			break;
		case 1:		//Sound Volume
			options.soundvolume += 0.1f;
			if(options.soundvolume>1.05f) options.soundvolume = 0;
			break;
		case 2:		//Main Menu
			return MAINMENU;
		}
	}

	if(ship[options.keyboardplayer].keys.DOWN || ship[options.gamepadplayer].keys.DOWN)
	{
		ship[options.keyboardplayer].keys.DOWN = false;
		ship[options.gamepadplayer].keys.DOWN = false;
		++menuselector;
	}
	if(ship[options.keyboardplayer].keys.UP || ship[options.gamepadplayer].keys.UP)
	{
		ship[options.keyboardplayer].keys.UP = false;
		ship[options.gamepadplayer].keys.UP = false;
		--menuselector;
	}

	if(menuselector<0) menuselector = maxmenu;
	else if(menuselector>maxmenu) menuselector = 0;

	return SOUNDMENU;
}

int GraphicsMenu()
{
	static int menuselector = 0;
	int maxmenu = 1;
	char menutext[100];

	sprintf_s(menutext, "Screenmode: %s x %s", g_screenx, g_screeny);
	DrawString(&menufont, menutext, xpos, ypos, g_screenx, GREEN);
	sprintf_s(menutext, "Antialiasing: %s",options.aa==true?"ON":"OFF");
	DrawString(&menufont, menutext, xpos, ypos+screenratio*21, g_screenx, menuselector==0?REDHIGHLIGHT:GREEN);
	DrawString(&menufont, "Main Menu", xpos, ypos+screenratio*42, g_screenx, menuselector==1?REDHIGHLIGHT:GREEN);

	if(TestKeyPressed())
	{
		switch(menuselector)
		{
		case 0:		//Change antialiasing
			options.aa = !options.aa;
			return GRAPHICSMENU;
		case 1:		//Return
			return MAINMENU;
		}
	}

	if(ship[options.keyboardplayer].keys.DOWN || ship[options.gamepadplayer].keys.DOWN)
	{
		ship[options.keyboardplayer].keys.DOWN = false;
		ship[options.gamepadplayer].keys.DOWN = false;
		++menuselector;
	}
	if(ship[options.keyboardplayer].keys.UP || ship[options.gamepadplayer].keys.UP)
	{
		ship[options.keyboardplayer].keys.UP = false;
		ship[options.gamepadplayer].keys.UP = false;
		--menuselector;
	}

	if(menuselector<0) menuselector = maxmenu;
	else if(menuselector>maxmenu) menuselector = 0;

	return GRAPHICSMENU;
}

int ControlMenu()
{
	DrawString(&font, "- Keyboard -", xpos, ypos, g_screenx, GREENHIGHLIGHT);
	DrawString(&font, "       Move:  Arrow Keys", xpos, ypos+screenratio*20, g_screenx, GREENHIGHLIGHT);
	DrawString(&font, "       Fire:  Z", xpos, ypos+screenratio*40, g_screenx, GREENHIGHLIGHT);
	DrawString(&font, "   Missiles:  X", xpos, ypos+screenratio*60, g_screenx, GREENHIGHLIGHT);
	DrawString(&font, " Select/Use:  SPACE", xpos, ypos+screenratio*80, g_screenx, GREENHIGHLIGHT);
	DrawString(&font, "Charge Bomb:  Hold SPACE (when not over any device)", xpos, ypos+screenratio*100, g_screenx, GREENHIGHLIGHT);
	DrawString(&font, " Side Boost:  Left Ctrl and Right Ctrl (if Side Boost installed)", xpos, ypos+screenratio*120, g_screenx, GREENHIGHLIGHT);

	DrawString(&font, "- Mouse -", xpos, ypos+screenratio*150, g_screenx, GREENHIGHLIGHT);
	DrawString(&font, "       Move:  Mouse - RMB to move forwards", xpos, ypos+screenratio*170, g_screenx, GREENHIGHLIGHT);
	DrawString(&font, "       Fire:  LMB", xpos, ypos+screenratio*190, g_screenx, GREENHIGHLIGHT);
	DrawString(&font, "   Missiles:  Roll mouse wheel", xpos, ypos+screenratio*210, g_screenx, GREENHIGHLIGHT);
	DrawString(&font, " Select/Use:  Click mouse wheel", xpos, ypos+screenratio*230, g_screenx, GREENHIGHLIGHT);
	DrawString(&font, "Charge Bomb:  Hold mouse wheel (when not over any device)", xpos, ypos+screenratio*250, g_screenx, GREENHIGHLIGHT);
	DrawString(&font, " Side Boost:  Unavailable with mouse controls", xpos, ypos+screenratio*270, g_screenx, GREENHIGHLIGHT);

	DrawString(&font, "- Gamepad -", xpos, ypos+screenratio*300, g_screenx, GREENHIGHLIGHT);
	DrawString(&font, "       Move:  Pad or Stick", xpos, ypos+screenratio*320, g_screenx, GREENHIGHLIGHT);
	DrawString(&font, "       Fire:  Button 1", xpos, ypos+screenratio*340, g_screenx, GREENHIGHLIGHT);
	DrawString(&font, "   Missiles:  Button 2", xpos, ypos+screenratio*360, g_screenx, GREENHIGHLIGHT);
	DrawString(&font, " Select/Use:  Button 3", xpos, ypos+screenratio*380, g_screenx, GREENHIGHLIGHT);
	DrawString(&font, "Charge Bomb:  Hold button 4", xpos, ypos+screenratio*400, g_screenx, GREENHIGHLIGHT);
	DrawString(&font, " Side Boost:  Left/Right triggers (if Side Boost installed)", xpos, ypos+screenratio*420, g_screenx, GREENHIGHLIGHT);

	if(TestKeyPressed()) return MAINMENU;
	return CONTROLMENU;
}

int OptionsMenu()
{
	static int menuselector = 0;
	int maxmenu = 10;
	char menutext[200];
	string explanation;

	sprintf_s(menutext, 200, "Music Volume:   ...........");
	menutext[(int)(options.musicvolume*10.0)+16] = 124;
	DrawString(&menufont, menutext, xpos, ypos, g_screenx, menuselector==0?REDHIGHLIGHT:GREEN);
	sprintf_s(menutext, 200, "Effects Volume: ...........");
	menutext[(int)(options.soundvolume*10.0)+16] = 124;
	DrawString(&menufont, menutext, xpos, ypos+screenratio*21.0f, g_screenx, menuselector==1?REDHIGHLIGHT:GREEN);
	sprintf_s(menutext, 200, "Screenmode: %d x %d x %dHz ...",g_screenx,g_screeny,dx9.refreshrate);
	DrawString(&menufont, menutext, xpos, ypos+screenratio*42.0f, g_screenx, menuselector==2?REDHIGHLIGHT:GREEN);
	DrawString(&menufont, "Player 1 Controls:", xpos, ypos+screenratio*63.0f, g_screenx, menuselector==3?REDHIGHLIGHT:GREEN);
	DrawString(&menufont, "Player 2 Controls:", xpos, ypos+screenratio*84.0f, g_screenx, menuselector==4?REDHIGHLIGHT:GREEN);
	sprintf_s(menutext, 200, "Show FPS: %s",options.framecounter?"On":"Off");
	DrawString(&menufont, menutext, xpos, ypos+screenratio*105.0f, g_screenx, menuselector==5?REDHIGHLIGHT:GREEN);
	sprintf_s(menutext, 200, "Explosion Distortions: %s",options.explosiondistortions?"On":"Off");
	DrawString(&menufont, menutext, xpos, ypos+screenratio*126.0f, g_screenx, menuselector==6?REDHIGHLIGHT:GREEN);
	sprintf_s(menutext, 200, "Explosion Flashes: %s",options.explosionflashes?"On":"Off");
	DrawString(&menufont, menutext, xpos, ypos+screenratio*147.0f, g_screenx, menuselector==7?REDHIGHLIGHT:GREEN);
	sprintf_s(menutext, 200, "Maximum Distortions:   ...........");
	if(options.capexplosions==10)
	{
		menutext[33] = '1';		menutext[34] = '0';		menutext[35] = '\0';
	}
	else if(!options.explosiondistortions || options.drawmethod==QUADS) menutext[23] = '0';
	else menutext[options.capexplosions+23] = '0' + options.capexplosions;
	DrawString(&menufont, menutext, xpos, ypos+screenratio*168.0f, g_screenx, menuselector==8?REDHIGHLIGHT:GREEN);
	sprintf_s(menutext, 200, "Draw Method: %s",options.drawmethod?"Pixel Shader":"Textured Quads");
	DrawString(&menufont, menutext, xpos, ypos+screenratio*189.0f, g_screenx, menuselector==9?REDHIGHLIGHT:GREEN);
	DrawString(&menufont, "Main Menu", xpos, ypos+screenratio*210.0f, g_screenx, menuselector==10?REDHIGHLIGHT:GREEN);

	DrawString(&menufont, "KEYBOARD", xpos + screenratio*19.0f*(float)menufont.framewidth, ypos+screenratio*63.0f, g_screenx, ship[0].controltype==KEYBOARD?REDHIGHLIGHT:GREEN);
	DrawString(&menufont, "MOUSE", xpos + screenratio*28.0f*(float)menufont.framewidth, ypos+screenratio*63.0f, g_screenx, ship[0].controltype==MOUSE?REDHIGHLIGHT:GREEN);
	DrawString(&menufont, "GAMEPAD", xpos + screenratio*34.0f*(float)menufont.framewidth, ypos+screenratio*63.0f, g_screenx, ship[0].controltype==GAMEPAD?REDHIGHLIGHT:GREEN);

	DrawString(&menufont, "KEYBOARD", xpos + screenratio*19.0f*(float)menufont.framewidth, ypos+screenratio*84.0f, g_screenx, ship[1].controltype==KEYBOARD?REDHIGHLIGHT:GREEN);
	DrawString(&menufont, "MOUSE", xpos + screenratio*28.0f*(float)menufont.framewidth, ypos+screenratio*84.0f, g_screenx, ship[1].controltype==MOUSE?REDHIGHLIGHT:GREEN);
	DrawString(&menufont, "GAMEPAD", xpos + screenratio*34.0f*(float)menufont.framewidth, ypos+screenratio*84.0f, g_screenx, ship[1].controltype==GAMEPAD?REDHIGHLIGHT:GREEN);


	if(TestKeyPressed())
	{
		switch(menuselector)
		{
		case 0:		//Music Volume
			options.musicvolume += 0.1f;
			if(options.musicvolume>1.05f) options.musicvolume = 0;
			g_pBasicAudio->put_Volume(soundfloattoint(options.musicvolume*0.8f));
			soundeffect[MENUMOVE].SetSoundEffect(0);
			break;
		case 1:		//Sound Volume
			options.soundvolume += 0.1f;
			if(options.soundvolume>1.05f) options.soundvolume = 0;
			soundeffect[MENUMOVE].SetSoundEffect(0);
			break;
		case 2:		//Change screenmode
			return SCREENMENU;
		case 3:		//Change control type for player 1
			if(ship[0].controltype == KEYBOARD)
			{
				ship[0].controltype = MOUSE;
				if(ship[1].controltype == MOUSE) ship[1].controltype = KEYBOARD;
			}
			else if(ship[0].controltype == MOUSE)
			{
				ship[0].controltype = GAMEPAD;
				if(ship[1].controltype == GAMEPAD) ship[1].controltype = KEYBOARD;
			}
			else
			{
				ship[0].controltype = KEYBOARD;
				if(ship[1].controltype == KEYBOARD) ship[1].controltype = MOUSE;
			}
			SetPlayerControllers();
			break;
		case 4:		//Change control type for player 2
			if(ship[1].controltype == KEYBOARD)
			{
				ship[1].controltype = MOUSE;
				if(ship[0].controltype == MOUSE) ship[0].controltype = KEYBOARD;
			}
			else if(ship[1].controltype == MOUSE)
			{
				ship[1].controltype = GAMEPAD;
				if(ship[0].controltype == GAMEPAD) ship[0].controltype = KEYBOARD;
			}
			else
			{
				ship[1].controltype = KEYBOARD;
				if(ship[0].controltype == KEYBOARD) ship[0].controltype = MOUSE;
			}
			SetPlayerControllers();
			break;
		case 5:		//Return
			options.framecounter = !options.framecounter;
			break;
		case 6:
			if(options.PS2available)
				options.explosiondistortions = !options.explosiondistortions;
			break;
		case 7:
			if(options.PS2available)
				options.explosionflashes = !options.explosionflashes;
			break;
		case 8:
			if(options.PS2available)
				if(++options.capexplosions>options.maxexplosions) options.capexplosions = 0;
			break;
		case 9:
			options.drawmethod = !options.drawmethod;
			if(!options.PS2available) options.drawmethod = QUADS;
			break;
		case 10:		//Return
			return MAINMENU;
		}
	}

	switch(menuselector)
	{
		case 0:		//Music Volume
			explanation = "Sets the volume of background music in the game";
			break;
		case 1:
			explanation = "Sets the volume of sound effects in the game";
			break;
		case 2:
			explanation = "Set the game's screen resolution";
			break;
		case 3:
			explanation = "Sets player 1's control method";
			break;
		case 4:
			explanation = "Sets player 2's control method";
			break;
		case 5:
			explanation = "Sets whether to show the Frames Per Second on screen during play";
			break;
		case 6:
			explanation = "Sets whether to show distortion shockwaves from explosions";
			if(!options.PS2available) explanation += " - requires Shader Model 2.0 or higher!";
			break;
		case 7:
			explanation = "Sets whether to show flashes from explosions";
			if(!options.PS2available) explanation += " - requires Shader Model 2.0 or higher!";
			break;
		case 8:
			explanation = "Sets the maximum number of shockwave distortions to display at one time";
			if(!options.PS2available) explanation += " - requires Shader Model 2.0 or higher!";
			else if(!options.PS3available) explanation += " - you need a graphics card supporting Shader Model 3.0 to increase this value higher than 4!";
			else explanation += ". Your graphics card supports Shader Model 3.0 so you can select higher values, though poor performing cards may suffer from slowdown if this number is too high";
			break;
		case 9:
			explanation = "The background graphics can be drawn using either a pixel shader or textured quads. Some effects are only supported with the pixel shader. ";
			if(options.PS3available) explanation += "Your graphics card supports Shader Model 3.0";
			else if(options.PS2available) explanation += "Your graphics card supports Shader Model 2.0";
			else explanation += "Your graphics card does not support Shader Model 2.0 or above so you are limited to using textured quads";
			break;
		case 10:
			explanation = "Return to the main menu";
			break;
	}
	//Show explanatory text
	DrawString(&font, explanation.c_str(), xpos, ypos+screenratio*350.0f, g_screenx-xpos, GREENHIGHLIGHT);

	if(ship[options.keyboardplayer].keys.RIGHT || ship[options.gamepadplayer].keys.RIGHT)
	{	
		ship[options.keyboardplayer].keys.RIGHT = false;
		ship[options.gamepadplayer].keys.RIGHT = false;
		switch(menuselector)
		{
		case 0:		//Music Volume
			options.musicvolume += 0.1f;
			if(options.musicvolume>1.05f) options.musicvolume = 0;
			g_pBasicAudio->put_Volume(soundfloattoint(options.musicvolume*0.8));
			soundeffect[MENUMOVE].SetSoundEffect(0);
			break;
		case 1:		//Sound Volume
			options.soundvolume += 0.1f;
			if(options.soundvolume>1.05f) options.soundvolume = 0;
			soundeffect[MENUMOVE].SetSoundEffect(0);
			break;
		}
	}

	if(ship[options.keyboardplayer].keys.LEFT || ship[options.gamepadplayer].keys.LEFT)
	{
		ship[options.keyboardplayer].keys.LEFT = false;
		ship[options.gamepadplayer].keys.LEFT = false;
		switch(menuselector)
		{
		case 0:		//Music Volume
			options.musicvolume -= 0.1f;
			if(options.musicvolume<0) options.musicvolume = 1.0f;
			g_pBasicAudio->put_Volume(soundfloattoint(options.musicvolume*0.8f));
			soundeffect[MENUMOVE].SetSoundEffect(0);
			break;
		case 1:		//Sound Volume
			options.soundvolume -= 0.1f;
			if(options.soundvolume<0) options.soundvolume = 1.0f;
			soundeffect[MENUMOVE].SetSoundEffect(0);
			break;
		}
	}

	if(ship[options.keyboardplayer].keys.DOWN || ship[options.gamepadplayer].keys.DOWN)
	{
		ship[options.keyboardplayer].keys.DOWN = false;
		ship[options.gamepadplayer].keys.DOWN = false;
		++menuselector;
	}
	if(ship[options.keyboardplayer].keys.UP || ship[options.gamepadplayer].keys.UP)
	{
		ship[options.keyboardplayer].keys.UP = false;
		ship[options.gamepadplayer].keys.UP = false;
		--menuselector;
	}

	if(menuselector<0) menuselector = maxmenu;
	else if(menuselector>maxmenu) menuselector = 0;

	return OPTIONSMENU;
}

int Credits()
{
	DrawString(&font, "Design, Programming, Graphics, Sound and Music by Sam Stone", xpos, ypos, g_screenx, GREENHIGHLIGHT);
	//DrawString(&font, "Game Testing: Karen Pinches, Sam Stone", xpos, ypos+screenratio*40, g_screenx, true);
	DrawString(&font, "This game was developed over the course of three years, from September 2006. It's my first ever game programming project, so it was something of a learning process. I hope you find it fun. Sam Stone, September 2009\nshaolinspin@gmail.com", xpos, ypos+screenratio*100, g_screenx-50, GREENHIGHLIGHT);
//	DrawString(&font, "There are some secret areas in a few of the levels. To access them via the teleporters, try shooting at the red flashing wall lights ... ", xpos, ypos+screenratio*300, g_screenx-50, GREENHIGHLIGHT);


	if(TestKeyPressed()) return MAINMENU;
	return CREDITS;
}

int Story()
{
	DrawString(&font, "---Journal Entry 514.23 - 23 April 2451---", 50.0f*screenratio, ypos, g_screenx-50.0f*screenratio, GREEN);
	DrawString(&font, "\n\nAnother day, another alien invasion. They arrived exactly six months ago. Nasty, squishy jelly-things with their lobster claws and silly, wide, bovine mouths. I didn't like them the moment they arrived, despite their pretensions to interstellar friendship and their technological gifts. Being stationed out in Omicron Sector, I had little to do with them, though you can bet I followed the news of their activities with suspicion. The residents here told me I was being paranoid and xenophobic, and lamented the fact that such a small outpost was unlikely to ever receive an official squishy visit - I was damn glad they rarely came this far out.\n\n They first appeared near Jupiter, through some sort of quantum wormhole-thingy. The massive, black ships that drifted from the crackling swirl of primordial energy didn't look like diplomatic cruisers or exploratory vessels to me. After announcing themselves as friendly visitors from another star system the aliens proceeded to visit every major population centre, greeting the leaders and bestowing technology and knowledge upon the people. Or buying perfunctory friendship with trinkets, to be more precise. Damn squishies were damn-near revered by some people. I sure wouldn't want to touch one. That green slimy shit looks nasty.\n\nThey disappeared within the space of a day, leaving only a small fleet hanging around the still-active wormhole entrance. I thought that was ominous the moment I heard. Then, within hours, half a million people turned into zombies. All over the system, everywhere the squishies had been and many places they hadn't, people started acting like robots - responsive to some kind of virus controlling their brains. The alien gifts became weapons in the hands of the infected souls, and those half a million zombies soon had control of most of the solar system. Not a single alien life lost, and minimal cost to them. They're crafty bastards, to be sure. They sauntered back into our system then, just a handful of them, to become our new squishy overlords.\n\nHere's the thing: I'm not standing for it. I was sent here to seek out and eliminate terrorists. The squishies are close enough. It's just a shame that the hardware here might as well be from the dark ages. Ah well, needs must. The suits back home don't call me double-oh-unstoppable for nothing. It's time for ... RETALIATION.", 50.0f*screenratio, ypos, g_screenx-50.0f*screenratio, 0xffafffaf);
	DrawString(&font, "---Journal entry ends---", 50.0f*screenratio, g_screeny-50.0f*screenratio, g_screenx-50.0f*screenratio, GREEN);

	if(TestKeyPressed()) return MAINMENU;
	return STORY;
}

int LoadGameMenu()
{
	static int menuselector = 0;
	int maxmenu = 2;
	DrawString(&menufont, "Load single player or two player game?", xpos, ypos, g_screenx, GREEN);
	DrawString(&menufont, "Single Player Saved Game", xpos, ypos+screenratio*21, g_screenx, menuselector==0?REDHIGHLIGHT:GREEN);
	DrawString(&menufont, "Two Player Saved Game", xpos, ypos+screenratio*42, g_screenx, menuselector==1?REDHIGHLIGHT:GREEN);
	DrawString(&menufont, "Main Menu", xpos, ypos+screenratio*63, g_screenx, menuselector==2?REDHIGHLIGHT:GREEN);

	if(TestKeyPressed())
	{
		switch(menuselector)
		{
		case 0:		//Single player
			playmode = SINGLEPLAYER;
			break;
		case 1:		//Two player
			playmode = TWOPLAYER;
			break;
		case 2:
			return MAINMENU;
		}
		gamemode = GAMEOVER;
		deadtimer = -3000;
		if(!InitialiseGame(true)) return false;
		gamerunning = true;
		return MAINMENU;
	}

	if(ship[options.keyboardplayer].keys.DOWN || ship[options.gamepadplayer].keys.DOWN)
	{
		ship[options.keyboardplayer].keys.DOWN = false;
		ship[options.gamepadplayer].keys.DOWN = false;
		++menuselector;
	}
	if(ship[options.keyboardplayer].keys.UP || ship[options.gamepadplayer].keys.UP)
	{
		ship[options.keyboardplayer].keys.UP = false;
		ship[options.gamepadplayer].keys.UP = false;
		--menuselector;
	}

	if(menuselector<0) menuselector = maxmenu;
	else if(menuselector>maxmenu) menuselector = 0;

	return LOADMENU;
}

int ScreenModeMenu()
{
	static int menuselector = 0;
	int maxmenu = options.widescreen?dx9.widescreenmodes:dx9.normalscreenmodes;
	char menutext[200];

//	DrawString(&menufont, "Change screenmode:", xpos, ypos, g_screenx, false);
	sprintf_s(menutext, 200, "Current screenmode: %d x %d x %dHz",g_screenx,g_screeny,dx9.refreshrate);
	DrawString(&menufont, menutext, xpos, ypos, g_screenx, GREEN);

	for(int i=0;i<maxmenu;++i)
	{
		sprintf_s(menutext, 200, "%d x %d x %dHz",options.widescreen?dx9.widemodes[i].Width:dx9.normalmodes[i].Width,options.widescreen?dx9.widemodes[i].Height:dx9.normalmodes[i].Height,dx9.refreshrate);
		DrawString(&menufont, menutext, xpos, ypos + screenratio*42.0f + screenratio*21.0f*(float)i, g_screenx, menuselector==i?REDHIGHLIGHT:GREEN);
	}
	DrawString(&menufont, "Options Menu", xpos, ypos + screenratio*63.0f + screenratio*(float)maxmenu*21.0f, g_screenx, menuselector==maxmenu?REDHIGHLIGHT:GREEN);

	if(TestKeyPressed())
	{
		if(menuselector<maxmenu)
		{
			//dbout<<"Resetting ..."<<endl;
			g_screenx = dx9.d3dpp.BackBufferWidth = options.widescreen?dx9.widemodes[menuselector].Width:dx9.normalmodes[menuselector].Width;
			g_screeny = dx9.d3dpp.BackBufferHeight = options.widescreen?dx9.widemodes[menuselector].Height:dx9.normalmodes[menuselector].Height;
			HRESULT hr = dx9.pd3dDevice->Reset(&dx9.d3dpp);
			if(CheckDeviceReady())
			{
				screenratio = (float)g_screeny/DEFAULTSCREENHEIGHT;
				SetDeviceStates();
				SetScreenCoords();
				SetViewPorts();
				//dbout<<"Device reset, screen ratio "<<screenratio<<endl;
			}
		}
		else return OPTIONSMENU;
	}

	if(ship[options.keyboardplayer].keys.DOWN || ship[options.gamepadplayer].keys.DOWN)
	{
		ship[options.keyboardplayer].keys.DOWN = false;
		ship[options.gamepadplayer].keys.DOWN = false;
		++menuselector;
	}
	if(ship[options.keyboardplayer].keys.UP || ship[options.gamepadplayer].keys.UP)
	{
		ship[options.keyboardplayer].keys.UP = false;
		ship[options.gamepadplayer].keys.UP = false;
		--menuselector;
	}

	if(menuselector<0) menuselector = maxmenu;
	else if(menuselector>maxmenu) menuselector = 0;

	return SCREENMENU;
}

int ExitMenu()
{
	static int menuselector = 1;
	int maxmenu = 1;
	DrawString(&menufont, "Are you sure you want to exit the game?", xpos, ypos, g_screenx, GREEN);
	DrawString(&menufont, "YES", xpos, ypos+screenratio*30.0f, g_screenx, menuselector==0?REDHIGHLIGHT:GREEN);
	DrawString(&menufont, "NO", xpos + 4.0f*menufont.framewidth*screenratio, ypos+screenratio*30.0f, g_screenx, menuselector==1?REDHIGHLIGHT:GREEN);

	if(TestKeyPressed())
	{
		switch(menuselector)
		{
		case 0:		//Exit
			return QUIT;
		case 1:		//Don't exit
			return MAINMENU;
		}
	}

	if(ship[options.keyboardplayer].keys.DOWN || ship[options.gamepadplayer].keys.DOWN)
	{
		ship[options.keyboardplayer].keys.DOWN = false;
		ship[options.gamepadplayer].keys.DOWN = false;
		++menuselector;
	}
	if(ship[options.keyboardplayer].keys.UP || ship[options.gamepadplayer].keys.UP)
	{
		ship[options.keyboardplayer].keys.UP = false;
		ship[options.gamepadplayer].keys.UP = false;
		--menuselector;
	}
	if(ship[options.keyboardplayer].keys.RIGHT || ship[options.gamepadplayer].keys.RIGHT)
	{
		ship[options.keyboardplayer].keys.RIGHT = false;
		ship[options.gamepadplayer].keys.RIGHT = false;
		++menuselector;
	}
	if(ship[options.keyboardplayer].keys.LEFT || ship[options.gamepadplayer].keys.LEFT)
	{
		ship[options.keyboardplayer].keys.LEFT = false;
		ship[options.gamepadplayer].keys.LEFT = false;
		--menuselector;
	}


	if(menuselector<0) menuselector = maxmenu;
	else if(menuselector>maxmenu) menuselector = 0;

	return EXITMENU;
}