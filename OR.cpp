/* Retaliation */

#include "OR.h"

DX9VARS dx9;
D3DCAPS9 d3dcaps;

Scene scene;

// globals
LPDIRECTINPUT8         lpdi;
LPDIRECTINPUTDEVICE8   m_keyboard;
LPDIRECTINPUTDEVICE8   m_mouse;

UCHAR keystate[256];
DIMOUSESTATE mouse_state;

Options options;
bool boundarychanged = false;
bool gamerunning = false;
bool restartlevel = false;
bool bquit = false;
bool newlevel = false;
//bool cheatmode = false, playintro = false, musicon = true, framecounter = false, aa = true;
int gamemode = INTRO, storegamemode = INTRO, playmode = TWOPLAYER, numberofweapons = 0;
int maxwidth, maxheight, maxratio;
int saveticks = 0, lasttick, thistick = 0, ticks, deadtimer;
float frametime, framespersec = 0;				//Time the last frame took to draw, and a counter
int fiftyframecounter = 0, fiftyframeticks = 0;
int tilesdrawn;
//float musicvolume = 1.0, soundvolume = 1.0;


list<GameObject> entity(1);				//Main game objects - entity[0] will always exist, and is an instance of a 'blue gem'
list<FadeObject> entityfade(0);			//For fade objects, etc, which don't interact
list<GameObject> entityplayer(0);		//For teleporters, bases, etc, which only interact with the player

vector<GraphicalObject> graphics(0);

std::list<GameObject>::iterator parentiterator;

BaseData basedata[10];			//Stores the data for all the bases in a level
BaseLayout baselayout;			//Scaled positions of all the graphical objects in a base
Player ship[2];					//Player objects
Level level;					//Current level

GraphicalObject error, energybar, energybarframe, debris, debris2, smoke, missile, flame, redbullet, explosion, 
				font, menufont, numbers, whiteblob, messagescreen, interfacescreen, datascreen, buttonframe, buttong[6], meterframe, 
				meterpanel, meterbar[3], *weapongraphics, *weaponpulsegfx, *weapontrails, selectionbar, loading,
				mousefollowtarget, targetcross, targetcrossblue, targetlaser, drone, shielddrone, droneshield, shield, shieldbar, shieldframe,
				redblob, textpower,	textspeed, textpowerused, textenergy, texttotal, basebg, projperilogo, bloodstonelogo, presents,
				hitbar, hitbarframe, teleportblob, dividingline, flare1, flare2, exitgateblob, spark, lightning1[2], lightning2[2], lightning3[2],
				line, beamsectionred, beamsectiongreen, beamsectionblue, menubg, missiledrone;

Weapon *weaponlist;
Base base[2];
Equipment equipdata[NUMEQUIPTYPES];
bool equipment[NUMEQUIPTYPES];
TileGraphics tilegfx;
Mousetarget mousetarget;

LPDIRECTSOUND8 lpDSO;		//this is a pointer to the DirectSound Object 
SoundEffect soundeffect[NUMEFFECTS];

IGraphBuilder*	g_pGraphBuilder = NULL;
IMediaControl*	g_pMediaControl = NULL;
IMediaEventEx*	g_pMediaEvent = NULL;
IMediaPosition*	g_pMediaPosition = NULL;
IMediaSeeking*  g_pMediaSeeking = NULL;
IBasicAudio*	g_pBasicAudio = NULL;

const DWORD D3DFVF_TLVERTEX = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX3;


int g_screenx = 800, g_screeny = 600;
float screenratio;
ofstream dbout;

string nextlevel, firstlevel;

TIMECAPS caps;
HWND hWnd;


INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
	// Register the window class
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "Retaliation", NULL };
	RegisterClassEx( &wc );
	//SetErrorMode (SEM_NOGPFAULTERRORBOX);

	dbout.open("data\\logfile.txt",ios::out);
	dbout<<"--- Retaliation logfile ---"<<endl;
	ReadPrefs();	//Read preferences file

	// Create the application's window
//	hWnd = CreateWindow( "Retaliation", "Retaliation: CreateDevice", WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, GetDesktopWindow(), NULL, wc.hInstance, NULL );
	hWnd = CreateWindow( "Retaliation", "Retaliation", WS_EX_TOPMOST | WS_POPUP, 0, 0, g_screenx, g_screeny, NULL, NULL, wc.hInstance, NULL );

	HRESULT hr = CoInitialize(NULL);
	if(FAILED(hr))
	{
		MessageBox(hWnd,"Error initialising!","Error!",MB_OK | MB_ICONEXCLAMATION);
	}

	// Initialize Direct3D
	if( SUCCEEDED( InitD3D() ) )
	{
		// Show the window
		ShowWindow( hWnd, SW_SHOWDEFAULT );
		//UpdateWindow( hWnd ); // Forces a WM_PAINT for windowed code
		ShowCursor(false);

		if (FAILED(D3DXCreateSprite(dx9.pd3dDevice,&(dx9.sprite))))
		{
			//NOT GOOD!
			MessageBox(hWnd,"Error initialising!","Error!",MB_OK | MB_ICONEXCLAMATION);
			dbout<<"Failed creating sprite!"<<endl;
		}

		SetUp();		//All initialisation and loading code goes in here

		InitDirectInput();

		//InitDirectShow();
		CreateGraph("music\\darker_theme.wma");
		g_pBasicAudio->put_Volume(soundfloattoint(options.musicvolume*0.9));
		g_pMediaControl->Run();

		// Enter the message loop
		MSG msg;
		while( 1 )		//Do this until a quit message is received
		{
			while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
			{
				// Check for a quit message
				if( msg.message == WM_QUIT )
				{
					bquit = true;
					break;
				}

				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
			if(bquit) break;

			if( CheckDeviceReady() )
			{
				lasttick = thistick;
				thistick = timeGetTime();
				ticks = thistick - lasttick;

				frametime = (float)ticks*0.001f;
				if(options.framecounter)		//Make this more accurate
				{
					if(fiftyframecounter==60)
					{
						framespersec = 60.0f/((float)fiftyframeticks*0.001f);
						//framespersec = (float)fiftyframeticks*0.001;
						fiftyframecounter = 0;
						fiftyframeticks = 0;
					}
					fiftyframecounter++;
					fiftyframeticks += ticks;
				}
				switch(gamemode)
				{
				case EXITING:
					if(Outro(EXITING)==EXITING)	PostQuitMessage(1);
					break;
				case INTRO:			//Run intro sequence
					gamemode = Intro();
					break;
				case SHORTINTRO:			//Run shorter intro sequence
					gamemode = ShortIntro();
					break;
				case OUTRO:			//Run sequence that leads into game
					gamemode = Outro(PLAY);
					if(gamemode == PLAY) SetUpTriggers();
					break;
				case MENU:			//Show main menu
					if(!Menu()) gamemode = EXITING;
					break;
				case EXITLEVEL:
					PlayExiting();	//The last sequence with ship fading out
					break;
				case PLAY:			//Run game
					Play();
					break;
				case LOADLEVEL:		//Load a new level
					//dbout<<"Showing screen"<<endl;
					scene.SceneBegin(&dx9.MainViewPort);
					DrawGraphicalObject(&menubg,g_screenx/2,g_screeny/2,0,0,screenratio*1.171875f,true,0xffffffff);
					DrawGraphicalObject(&loading,g_screenx/2,g_screeny/2,0,0,screenratio,true,0xff00ff00);
					scene.SceneFlip();
					//dbout<<"Clearing level"<<endl;
					nextlevel = level.next;
					ClearCurrentLevel();
					//dbout<<"Loading level"<<endl;
					if(!LoadLevel(level)) gamemode = EXITING;
					//dbout<<"Loaded"<<endl;

					gamemode = OUTRO;
					frametime = 0;

					Save();		//Save the game and create a level restart point

					thistick = timeGetTime();
					ship[0].invulnerable = true;
					ship[0].invulnerabletimer = thistick+1340;	//To allow for the fading effect
					ship[1].invulnerable = true;
					ship[1].invulnerabletimer = thistick+1340;	//To allow for the fading effect
					if(ship[0].energy/ship[0].maxenergy < 0.5f) ship[0].energy = ship[0].maxenergy*0.5f;
					if(ship[1].energy/ship[1].maxenergy < 0.5f) ship[1].energy = ship[1].maxenergy*0.5f;
					newlevel = true;
					break;
				case GAMEOVER:			//Run game for 3 seconds then restart
					if(thistick-deadtimer<3000) Play();		//Leave the game running for 3 seconds before reloading so player can see explosion etc
					else
					{
						scene.SceneBegin(&dx9.MainViewPort);
						DrawGraphicalObject(&menubg,g_screenx/2,g_screeny/2,0,0,screenratio*1.171875f,true,0xffffffff);
						DrawGraphicalObject(&loading,g_screenx/2,g_screeny/2,0,0,screenratio,true,0xff00ff00);
						scene.SceneFlip();

						dbout<<"Loading ships"<<endl;
						for(int il=0;il<(playmode+1);il++)
						{
							LoadShip(&ship[il],ship[il].startship);
							ship[il].energy = ship[il].startenergy;		//Restore quantities of energy etc.
							ship[il].power = ship[il].startpower;
							ship[il].shields = ship[il].startshields;
							ship[il].missiles = ship[il].startmissiles;
							ship[il].weapon = &weaponlist[ship[il].startweapon];
							if(ship[il].equipment[BOOSTER])
							{
								ship[il].forwardsspeed *= 1.15f;
								ship[il].turnspeed *= 1.15f;
								ship[il].backwardsspeed *= 1.15f;
							}
						}
						ClearCurrentLevel();
						LoadSavedGame(level,restartlevel);
						restartlevel = false;
						gamemode = OUTRO;
						frametime = 0;
						thistick = timeGetTime();
						ship[0].invulnerable = true;
						ship[0].invulnerabletimer = thistick+1340;	//To allow for the fading effect
						ship[1].invulnerable = true;
						ship[1].invulnerabletimer = thistick+1340;	//To allow for the fading effect
						if(ship[0].energy/ship[0].maxenergy < 0.5f) ship[0].energy = ship[0].maxenergy*0.5f;
						if(ship[1].energy/ship[1].maxenergy < 0.5f) ship[1].energy = ship[1].maxenergy*0.5f;
						dbout<<"Starting game ..."<<endl;
					}
					break;
				case DEACTIVATED:
					WaitMessage();
					break;
				default:
					gamemode = PLAY;
					break;
				}
			}
		}

	}
	else
	{
		MessageBox(hWnd,"Could not initialise Direct3D","Error!",MB_OK | MB_ICONEXCLAMATION);
	}

	// Clean up everything and exit the app
	Cleanup();
	UnregisterClass( "Retaliation", wc.hInstance );
	return 0;
}

void GetCaps()
{
		dx9.pD3D->GetDeviceCaps( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, dx9.pd3dcaps );
		maxwidth = dx9.pd3dcaps->MaxTextureWidth;
		maxheight = dx9.pd3dcaps->MaxTextureHeight;
		maxratio = dx9.pd3dcaps->MaxTextureAspectRatio;

		if(dx9.pd3dcaps->PixelShaderVersion < D3DPS_VERSION(3,0)) options.PS3available = false;
		else options.PS3available = true;

		if(dx9.pd3dcaps->PixelShaderVersion < D3DPS_VERSION(2,0)) options.PS2available = false;
		else options.PS2available = true;

		if(options.PS3available) options.maxexplosions = 10;
		else if(options.PS2available)
		{
			options.maxexplosions = 4;
			if(options.capexplosions>options.maxexplosions) options.capexplosions = options.maxexplosions;
		}
		else
		{
			options.drawmethod = QUADS;
			options.maxexplosions = 0;
			options.capexplosions = 0;
			options.explosiondistortions = false;
			options.explosionflashes = false;
		}

		//dbout<<"Max texture width = "<<maxwidth<<endl;
		//dbout<<"Max texture height = "<<maxheight<<endl;
		//dbout<<"Max texture aspect ratio = "<<maxratio<<endl;
		//dbout<<"Max primitives "<<dx9.pd3dcaps->MaxPrimitiveCount<<endl;
}

//----------------------------------------------------------------------------
// InitD3D() : Initializes Direct3D
//----------------------------------------------------------------------------
HRESULT InitD3D()
{
	// Create the D3D object, which is needed to create the D3DDevice.
	if( ( dx9.pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) == NULL ) return E_FAIL;

	// Get the current desktop display mode
	D3DDISPLAYMODE d3ddm;
	if( FAILED( dx9.pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ))) return E_FAIL;

	dx9.refreshrate = d3ddm.RefreshRate;
	float deskwidescreenratio = (float)d3ddm.Width/(float)d3ddm.Height;
	float prevwidescreenratio = (float)g_screenx/(float)g_screeny;
	if(deskwidescreenratio>1.4) options.widescreen = true;
	else options.widescreen = false;

	//If we can't build a screen mode list, or the requested ratio doesn't match the desktop, set the resolution to match the desktop mode
	if( !BuildScreenModeList() || (prevwidescreenratio>1.4 && deskwidescreenratio<1.4) || (prevwidescreenratio<1.4 && deskwidescreenratio>1.4) )
	{
/*		if(options.widescreen) d3ddm = dx9.widemodes[0];
		else d3ddm = dx9.normalmodes[0];*/
		g_screenx = d3ddm.Width;
		g_screeny = d3ddm.Height;
		dbout<<"Saved screen mode not correct aspect ratio or not available! Setting resolution to "<<g_screenx<<" x "<<g_screeny<<endl;
	}

	screenratio = (float)g_screeny/DEFAULTSCREENHEIGHT;		//This is a scaling factor to scale the display for higher resolutions
	//dbout<<"Screen ratio "<<screenratio<<endl;

	// Parameters for the D3DDevice. Most parameters are zero'd out.
	// This sets the video format to match the current desktop display.
	// Check docs for other options you can set here, such as
	// 'Windowed' which would create a window-based app (not full screen)
	ZeroMemory( &dx9.d3dpp, sizeof(dx9.d3dpp) );
	dx9.d3dpp.Windowed = FALSE;
	dx9.d3dpp.BackBufferCount = 2;
	dx9.d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	dx9.d3dpp.BackBufferWidth = g_screenx;
	dx9.d3dpp.BackBufferHeight = g_screeny;
	dx9.d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	dx9.d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	dx9.d3dpp.FullScreen_RefreshRateInHz = d3ddm.RefreshRate;
	//dx9.d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	//dx9.d3dpp.hDeviceWindow = hWnd;
	//dx9.d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	// Create the Direct3D device, using the default adapter (most systems 
	// only have one, unless they have multiple graphics hardware cards
	// installed). See SDK for more details on other options here.
	if( FAILED( dx9.pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &dx9.d3dpp, &dx9.pd3dDevice ))) return E_FAIL;

	// Device state set here
	SetDeviceStates();
	SetFiltering(options.aa?ON:NONE);
	GetCaps();

	if(options.PS2available)
	{
		dbout<<"Doing shader"<<endl;
		LPD3DXBUFFER buffer;
		HRESULT hr = D3DXCreateEffectFromFile(dx9.pd3dDevice,"data\\rotozoom.fx",NULL,NULL,0,NULL,&dx9.rotozoom,&buffer);
		if(SUCCEEDED(hr))
		{
			dbout<<"SUCCEEDED"<<endl;
		}
		else
		{
			dbout<<"FAILED"<<endl;
			if(buffer->GetBufferSize())
			{
				char *errors = new char[buffer->GetBufferSize()];
				char *data = (char *)buffer->GetBufferPointer();
				for(int i=0;i<buffer->GetBufferSize();++i)
				{
					errors[i] = data[i];
				}
				dbout<<errors<<endl;
				delete [] errors;
			}
		}

		dx9.hTech = dx9.rotozoom->GetTechniqueByName("rotozoom");
		if(dx9.hTech == NULL) dbout<<"Problem finding valid shader technique"<<endl;

		char techname[100];

		for(int i=0;i<4;++i)
		{
			sprintf_s(techname,100,"rotozoomdistort%d",i);
			dx9.hDistort[i] = dx9.rotozoom->GetTechniqueByName(techname);
			if(dx9.hDistort[i] == NULL) dbout<<"Problem finding shader technique "<<techname<<endl;
		}

		if(options.PS3available)
		{
			for(int i=4;i<MAXEXPLOSIONS;++i)
			{
				sprintf_s(techname,100,"rotozoomdistort%d",i);
				dx9.hDistort[i] = dx9.rotozoom->GetTechniqueByName(techname);
				if(dx9.hDistort[i] == NULL) dbout<<"Problem finding shader technique "<<techname<<endl;
			}
		}
	}

	
	return S_OK;
}

void SetDeviceStates()
{
	//Set vertex shader
	dx9.pd3dDevice->SetVertexShader(NULL);
	dx9.pd3dDevice->SetFVF(D3DFVF_TLVERTEX);

	//Create vertex buffer
	dx9.pd3dDevice->CreateVertexBuffer(sizeof(TLVERTEX) * 3000, D3DUSAGE_WRITEONLY, D3DFVF_TLVERTEX, D3DPOOL_MANAGED, &dx9.vertexBuffer, NULL);
	dx9.pd3dDevice->SetStreamSource(0, dx9.vertexBuffer, 0, sizeof(TLVERTEX));

	dx9.pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	dx9.pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	dx9.pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	dx9.pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	dx9.pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
}

bool BuildScreenModeList()
{
	bool currentmodeavailable = false;
	D3DDISPLAYMODE d3ddm;
	UINT modecount = dx9.pD3D->GetAdapterModeCount( D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8);
	dx9.widescreenmodes = 0;
	dx9.normalmodes = 0;

	for(UINT i = 0; i<modecount; ++i)
	{
		dx9.pD3D->EnumAdapterModes( D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, i, &d3ddm);
		if(d3ddm.Height<600) continue;
		if(d3ddm.RefreshRate!=dx9.refreshrate) continue;
		float moderatio = (float)d3ddm.Width/(float)d3ddm.Height;

		if(g_screenx == d3ddm.Width && g_screeny == d3ddm.Height) currentmodeavailable = true;

		if(moderatio>1.4)
		{
			//dbout<<d3ddm.Width<<" x "<<d3ddm.Height<<": "<<d3ddm.RefreshRate<<" Hz, widescreen"<<endl;
			dx9.widescreenmodes++;
		}
		if(moderatio<1.4)
		{
			//dbout<<d3ddm.Width<<" x "<<d3ddm.Height<<": "<<d3ddm.RefreshRate<<" Hz, normal"<<endl;
			dx9.normalscreenmodes++;
		}

	}

	//dbout<<dx9.normalscreenmodes<<" normal screen modes and "<<dx9.widescreenmodes<<" wide screen modes"<<endl;

	dx9.widemodes = new D3DDISPLAYMODE[dx9.widescreenmodes];
	dx9.normalmodes = new D3DDISPLAYMODE[dx9.normalscreenmodes];

	for(UINT i = 0, w = 0, n = 0; i<modecount; ++i)
	{
		dx9.pD3D->EnumAdapterModes( D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, i, &d3ddm);
		if(d3ddm.Height<600) continue;
		if(d3ddm.RefreshRate != dx9.refreshrate) continue;
		float moderatio = (float)d3ddm.Width/(float)d3ddm.Height;

		if(moderatio>1.4) dx9.widemodes[w++] = d3ddm;
		else dx9.normalmodes[n++] = d3ddm;
	}

	return currentmodeavailable;
}

void SetFiltering(int action)
{
	if(action==ON)
	{
		dx9.pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		dx9.pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		dx9.pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	}
	else
	{
		dx9.pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		dx9.pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
		dx9.pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_POINT );
	}
}


//----------------------------------------------------------------------------
// Cleanup() : Releases all previously initialized objects
//----------------------------------------------------------------------------
void Cleanup()
{
	timeEndPeriod(caps.wPeriodMin);

	SavePrefs();

	DeleteLevel(level);

	ReleaseInput();

	CleanUpDirectShow();

	//dbout<<"Deleting weapon list"<<endl;
	for(int i=0;i<numberofweapons;i++)
	{
		HELPER_RELEASE(weaponlist[i].sound.buffer)
		//dbout<<weaponlist[i].soundfile<<" released"<<endl;
	}
	delete [] weaponlist;	//Sound effects from weapons will be deleted here too

	//All sound buffer releases must come before this point!
	Sound_Exit();

	//Clear stream source
	dx9.pd3dDevice->SetStreamSource(0, NULL, 0, 0);

	//Release vertex buffer
	if(dx9.vertexBuffer) dx9.vertexBuffer->Release();

	//delete temporaryobject;
	graphics.clear();

	//dbout<<"Releasing textures"<<endl;
	ReleaseTextures();

	//dbout<<"Deleting weapon graphics"<<endl;
	delete [] weapongraphics;
	delete [] weaponpulsegfx;
	delete [] weapontrails;

	if(dx9.pd3dDevice) dx9.pd3dDevice->Release();
	if(dx9.pD3D) dx9.pD3D->Release();

	entity.clear();

	CoUninitialize();

	ShowCursor(true);

	dbout.close();
}



//----------------------------------------------------------------------------
// MsgProc() : The window's message handler
//----------------------------------------------------------------------------
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	//Game is minimised
	if(gamemode == DEACTIVATED)
	{
		switch(msg)
		{
		case WM_GRAPHEVENT:
			OnGraphEvent();		// handles events
			return 0;
		case WM_ACTIVATEAPP:
		case WM_ACTIVATE:
			switch(wParam)
			{
				case WA_ACTIVE:
				case WA_CLICKACTIVE:
					gamemode = MENU;
					thistick = lasttick = timeGetTime();
					ticks = 0;
					if(g_pMediaControl!=NULL) g_pMediaControl->Run();
					dx9.pd3dDevice->Reset(&dx9.d3dpp);
					if(CheckDeviceReady())
					{
						screenratio = (float)g_screeny/DEFAULTSCREENHEIGHT;
						SetDeviceStates();
						SetScreenCoords();
						SetViewPorts();
						//dbout<<"Device reset, screen ratio "<<screenratio<<endl;
						//dbout<<"Reactivated"<<endl;
					}
					else
					{
						gamemode = DEACTIVATED;
						//dbout<<"Not reactivated"<<endl;
					}
					return 0;
				case WA_INACTIVE:
					return 0;
			}
			break;
        case WM_DESTROY: // If you use Windowed mode process loss of it
			PostQuitMessage( 0 );
			return 0;
		}
		return DefWindowProc( hWnd, msg, wParam, lParam );
	}

	//Game is running fully
    switch( msg )
    {
		case WM_GRAPHEVENT:
			OnGraphEvent();		// handles events
			return 0;
		case WM_ACTIVATEAPP:
		case WM_ACTIVATE:
			switch(wParam)
			{
				case WA_ACTIVE:
				case WA_CLICKACTIVE:
					return 0;
				case WA_INACTIVE:
					if(g_pMediaControl!=NULL) g_pMediaControl->Pause();
					gamemode = DEACTIVATED;
					saveticks = thistick;
					//dbout<<"Inactivated, game paused."<<endl;
					return 0;
			}
			break;
		case WM_KEYDOWN:
			switch(wParam)
			{
				case VK_ESCAPE:
					if(gamemode==PLAY || gamemode==GAMEOVER || gamemode==INBASE)
					{
						saveticks = thistick;
						gamemode = MENU;		//Opens the main menu
					}
					return 0;

				case VK_UP:
					ship[options.keyboardplayer].keys.UP = true;
					ship[options.keyboardplayer].keys.up = 1.0;
					return 0;
				case VK_DOWN:
					ship[options.keyboardplayer].keys.DOWN = true;
					ship[options.keyboardplayer].keys.down = 1.0;
					return 0;
				case VK_LEFT:
					if(!ship[options.keyboardplayer].keys.LEFT)
					{
						ship[options.keyboardplayer].keys.LEFT = true;
						ship[options.keyboardplayer].keys.left = 1.0;
					}
					return 0;
				case VK_RIGHT:
					if(!ship[options.keyboardplayer].keys.RIGHT)
					{
						ship[options.keyboardplayer].keys.RIGHT = true;
						ship[options.keyboardplayer].keys.right = 1.0;
					}
					return 0;
				case VK_Z:
					ship[options.keyboardplayer].keys.FIRE = true;
					return 0;
				case VK_X:
					ship[options.keyboardplayer].keys.FIREMISSILE = true;
					return 0;
				case VK_S:
					if(options.cheatmode)
					{
						ship[0].weapon = &weaponlist[GetWeaponNumber("C-Beam")];
						ship[1].weapon = &weaponlist[GetWeaponNumber("C-Beam")];
					}
					return 0;
				case VK_SPACE:
					ship[options.keyboardplayer].keys.OPERATE = true;
					if(ship[options.keyboardplayer].state == PLAY) if(ship[options.keyboardplayer].bombtimer==0xffffffff) ship[options.keyboardplayer].bombtimer = thistick;
					return 0;
				case VK_CONTROL:
					//Had to hack this as WM_KEYDOWN not working with VK_LCONTROL or VK_RCONTROL
					if(GetAsyncKeyState(VK_LCONTROL)&0x7fff)
					{
						if(!ship[options.keyboardplayer].keys.SHIFTLEFT) ship[options.keyboardplayer].keys.shiftleft = 1.0;
						ship[options.keyboardplayer].keys.SHIFTLEFT = true;
					}
					if(GetAsyncKeyState(VK_RCONTROL)&0x7fff)
					{
						if(!ship[options.keyboardplayer].keys.SHIFTRIGHT) ship[options.keyboardplayer].keys.shiftright = 1.0;
						ship[options.keyboardplayer].keys.SHIFTRIGHT = true;
					}
					return 0;
				case VK_1:
					if(options.cheatmode)
					{
						ship[0].weapon = &weaponlist[GetWeaponNumber("Ion_Blaster")];
						ship[1].weapon = &weaponlist[GetWeaponNumber("Ion_Blaster")];
					}
					return 0;
				case VK_2:
					if(options.cheatmode)
					{
						ship[0].weapon = &weaponlist[GetWeaponNumber("Ripper")];
						ship[1].weapon = &weaponlist[GetWeaponNumber("Ripper")];
					}
					return 0;
				case VK_3:
					if(options.cheatmode)
					{
						ship[0].weapon = &weaponlist[GetWeaponNumber("Plasma_Cannon")];
						ship[1].weapon = &weaponlist[GetWeaponNumber("Plasma_Cannon")];
					}
					return 0;
				case VK_4:
					if(options.cheatmode)
					{
						ship[0].weapon = &weaponlist[GetWeaponNumber("Laser")];
						ship[1].weapon = &weaponlist[GetWeaponNumber("Laser")];
					}
					return 0;
				case VK_5:
					if(options.cheatmode)
					{
						ship[0].weapon = &weaponlist[GetWeaponNumber("Power_Blaster")];
						ship[1].weapon = &weaponlist[GetWeaponNumber("Power_Blaster")];
					}
					return 0;
				case VK_6:
					if(options.cheatmode)
					{
						ship[0].weapon = &weaponlist[GetWeaponNumber("Photon_Disruptor")];
						ship[1].weapon = &weaponlist[GetWeaponNumber("Photon_Disruptor")];
					}
					return 0;
				case VK_7:
					if(options.cheatmode)
					{
						ship[0].weapon = &weaponlist[GetWeaponNumber("Fusion_Bolt")];
						ship[1].weapon = &weaponlist[GetWeaponNumber("Fusion_Bolt")];
					}
					return 0;
				case VK_8:
					if(options.cheatmode && (gamemode == PLAY || gamemode == GAMEOVER))
					{
						ship[0].energy = ship[0].maxenergy;
						ship[0].shields = ship[0].maxshields;
						ship[0].credits += 100;
						ship[0].equipment[RADAR] = true;
						ship[0].equipment[TARGET_LASER] = true;
						ship[0].equipment[GEMTRACTOR] = true;
						ship[0].equipment[SCANNER] = true;
						ship[0].equipment[SPREADER] = true;
						ship[0].equipment[SEEKER] = true;
						ship[0].equipment[MISSILE_LAUNCHER] = true;
						ship[0].equipment[MINIMISSILES] = true;
						ship[0].equipment[SHIELD] = true;
						ship[0].equipment[SHIELD_RECHARGER] = true;
						ship[0].equipment[HOMING_COMPUTER] = true;
						ship[0].equipment[BOMB] = true;
						ship[0].equipment[SELF_REPAIR] = true;
						ship[0].equipment[SIDE_BOOST] = true;
						//ship[0].equipment[MISSILEBOMBS] = true;
						ship[0].missiles = ship[0].maxmissiles;
						if(!ship[0].equipment[LASER_DRONE])
						{
							ship[0].equipment[LASER_DRONE] = true;
							CreateDrone(0);
						}
						if(!ship[0].equipment[LIGHTNING_DRONE])
						{
							ship[0].equipment[LIGHTNING_DRONE] = true;
							CreateLightningDrone(0);
						}
						if(!ship[0].equipment[SHIELD_DRONE])
						{
							ship[0].equipment[SHIELD_DRONE] = true;
							for(int i=0;i<3;i++) CreateShieldDrone(0,i);
						}
						ship[0].state = PLAY;
						deadtimer = thistick+100000000;
						if(playmode==TWOPLAYER)
						{
							//dbout<<"Two player cheat"<<endl;
							ship[1].energy = ship[1].maxenergy;
							ship[1].shields = ship[1].maxshields;
							ship[1].credits += 100;
							ship[1].equipment[RADAR] = true;
							ship[1].equipment[GEMTRACTOR] = true;
							ship[1].equipment[SCANNER] = true;
							ship[1].equipment[SEEKER] = true;
							ship[1].equipment[MISSILE_LAUNCHER] = true;
							ship[1].equipment[MINIMISSILES] = true;
							ship[1].equipment[SHIELD] = true;
							ship[1].equipment[SHIELD_RECHARGER] = true;
							ship[1].equipment[HOMING_COMPUTER] = true;
							ship[1].equipment[BOMB] = true;
							ship[1].equipment[SELF_REPAIR] = true;
							ship[1].equipment[SIDE_BOOST] = true;
							ship[1].missiles = 10;
							if(!ship[1].equipment[LASER_DRONE])
							{
								ship[1].equipment[LASER_DRONE] = true;
								CreateDrone(1);
							}
							if(!ship[1].equipment[LIGHTNING_DRONE])
							{
								ship[1].equipment[LIGHTNING_DRONE] = true;
								CreateLightningDrone(1);
							}
							ship[1].state = PLAY;
						}
						//dbout<<"Cheat done"<<endl;
					}
					return 0;
				case VK_9:
					if(options.cheatmode)
					{
						ship[0].equipment[MISSILE_LAUNCHER] = true;
						ship[0].equipment[SHIELD] = true;
						ship[0].equipment[HOMING_COMPUTER] = true;
						ship[0].energy = ship[0].maxenergy;
						ship[0].shields = ship[0].maxshields;
						ship[0].state = PLAY;
						ship[0].forwardsspeed = 400.0;
						ship[1].forwardsspeed = 400.0;
						ship[0].maxpower = 1000.0;
						ship[1].maxpower = 1000.0;
					}
					return 0;

				default:
					return 0;
			}

		case WM_KEYUP:
			switch(wParam)
			{
				case VK_UP:
					ship[options.keyboardplayer].keys.UP = false;
					return 0;
				case VK_DOWN:
					ship[options.keyboardplayer].keys.DOWN = false;
					return 0;
				case VK_LEFT:
					ship[options.keyboardplayer].keys.LEFT = false;
					ship[options.keyboardplayer].keys.SHIFTLEFT = false;
					return 0;
				case VK_RIGHT:
					ship[options.keyboardplayer].keys.RIGHT = false;
					ship[options.keyboardplayer].keys.SHIFTRIGHT = false;
					return 0;
				case VK_0:
					ship[options.keyboardplayer].keys.FIRE = false;
					return 0;

				case VK_Z:
					ship[options.keyboardplayer].keys.FIRE = false;
					return 0;
				case VK_X:
					ship[options.keyboardplayer].keys.FIREMISSILE = false;
					return 0;
				case VK_SPACE:
					ship[options.keyboardplayer].keys.OPERATE = false;
					ship[options.keyboardplayer].bombtimer = 0xffffffff;
					return 0;
				case VK_CONTROL:
					//Had to hack this as WM_KEYDOWN not working with VK_LCONTROL or VK_RCONTROL
					if(!(GetAsyncKeyState(VK_LCONTROL)&0x7fff)) ship[options.keyboardplayer].keys.SHIFTLEFT = false;
					if(!(GetAsyncKeyState(VK_RCONTROL)&0x7fff)) ship[options.keyboardplayer].keys.SHIFTRIGHT = false;
					return 0;
				default:
					return 0;
			}
		case WM_INPUT:
			GetInput(wParam, lParam, options.gamepadplayer);
			return 0;

        case WM_DESTROY: // If you use Windowed mode process loss of it
			PostQuitMessage( 0 );
			return 0;
/*
        case WM_PAINT:
			// NOTE: In a full-screen app, you may choose to draw the screen at
			// a different point, such as using a timer message or outside of
			// the windows message loop
			Render();
			ValidateRect( hWnd, NULL );
        return 0;*/
     }

     return DefWindowProc( hWnd, msg, wParam, lParam );
}

void SetViewPorts()
{
	dx9.MainViewPort.X = 0;
	dx9.MainViewPort.Y = 0;
	dx9.MainViewPort.Width = g_screenx;
	dx9.MainViewPort.Height = g_screeny;
	dx9.MainViewPort.MaxZ = 1.0;
	dx9.MainViewPort.MinZ = 0.0;

	if(playmode==SINGLEPLAYER)
	{
		ship[0].viewport.X = 0;
		ship[0].viewport.Y = 0;
		ship[0].viewport.Width = g_screenx;
		ship[0].viewport.Height = g_screeny;
		ship[0].viewport.MaxZ = 1.0;
		ship[0].viewport.MinZ = 0.0;
	}

	if(playmode==TWOPLAYER)
	{
		ship[0].viewport.X = 0;
		ship[0].viewport.Y = 0;
		ship[0].viewport.Width = (g_screenx>>1) - 1;
		ship[0].viewport.Height = g_screeny;
		ship[0].viewport.MaxZ = 1.0;
		ship[0].viewport.MinZ = 0.0;

		ship[1].viewport.X = ship[1].scrtopleft.x;
		ship[1].viewport.Y = ship[1].scrtopleft.y;
		ship[1].viewport.Width = (g_screenx>>1) - 1;
		ship[1].viewport.Height = g_screeny;
		ship[1].viewport.MaxZ = 1.0;
		ship[1].viewport.MinZ = 0.0;
	}
}

//This function does all the loading and initialising of the entire program
void SetUp()
{
	MMRESULT mr = timeGetDevCaps(&caps,sizeof(caps));
	if(mr == TIMERR_NOERROR) timeBeginPeriod(caps.wPeriodMin);
	dbout<<"Setting timer to "<<caps.wPeriodMin<<endl;

	unsigned int rseed = timeGetTime()&RAND_MAX;
	srand(rseed);
	//dbout<<"Random seed "<<rseed<<endl;

	//Set the viewports to the correct size for the screen resolution
	SetViewPorts();

	if(!SetUpControllers())
	{
		options.hidcontrollers = false;
		//dbout<<"No controllers attached!"<<endl;
	}
	else
	{
		options.hidcontrollers = true;
		//dbout<<"Found HID controller(s)"<<endl;
	}

	if(!(Sound_Initialize(hWnd)))
	{	//Bad!
		dbout<<"Problem with sound initialisation!"<<endl;
		lpDSO = NULL;
	}

	InitDirectShow();

	graphics.reserve(200);

	//dbout<<"Game object size "<<sizeof(GameObject)<<" bytes"<<endl;

	//tilegfx.LoadTiles("outerspace");

	ZeroMemory(&ship[0],sizeof(Player));
	ZeroMemory(&ship[1],sizeof(Player));

	if(options.keyboardplayer == 0) ship[0].controltype = KEYBOARD;
	else if(options.keyboardplayer == 1) ship[1].controltype = KEYBOARD;

	if(options.gamepadplayer == 0) ship[0].controltype = GAMEPAD;
	else if(options.gamepadplayer == 1) ship[1].controltype = GAMEPAD;

	if(options.mouseplayer == 1) ship[1].controltype = MOUSE;

	ship[0].radar.CreateGraphicalObject(RADARWIDTH,RADARHEIGHT);
	ship[1].radar.CreateGraphicalObject(RADARWIDTH,RADARHEIGHT);

	flare1.CreateGraphicalObject("flare1",1);
	flare2.CreateGraphicalObject("flare2",1);
	dividingline.CreateGraphicalObject(2,g_screeny);
	loading.CreateGraphicalObject("loading_white",1);
	error.CreateGraphicalObject("error",1);
	missile.CreateGraphicalObject("missile",4,16.0);
	flame.CreateGraphicalObject("flame",4);
	smoke.CreateGraphicalObject("smoke",1);
	whiteblob.CreateGraphicalObject("white_blob",1);
	redblob.CreateGraphicalObject("red_engine_blob",1);
	redbullet.CreateGraphicalObject("red_bullet",1);
	explosion.CreateGraphicalObject("explosion3",16);
	interfacescreen.CreateGraphicalObject("interface_screen",1);
	datascreen.CreateGraphicalObject("data_screen",1);
	messagescreen.CreateGraphicalObject("message_screen",1);
	buttonframe.CreateGraphicalObject("button_frame",2);
	buttong[EXIT].CreateGraphicalObject("button_exit",1);
	buttong[SELECT].CreateGraphicalObject("button_select",1);
	buttong[REMOVE].CreateGraphicalObject("button_remove",1);
	buttong[SHIP].CreateGraphicalObject("button_ship",1);
	buttong[EQUIP].CreateGraphicalObject("button_equip",1);
	buttong[WEAPON].CreateGraphicalObject("button_weapon",1);
	meterframe.CreateGraphicalObject("meter",1);
	meterpanel.CreateGraphicalObject("meter_panel",1);
	selectionbar.CreateGraphicalObject("selection_bar",1);
	font.CreateGraphicalObject("ascii_font_cool_thick",95);
	menufont.CreateGraphicalObject("ascii_font_bigger",95);
	numbers.CreateGraphicalObject("numbers",10);
	debris.CreateGraphicalObject("debris",16,4.0);
	debris2.CreateGraphicalObject("debris_2",16,4.0);
	energybar.CreateGraphicalObject("energy_bar",1);
	energybarframe.CreateGraphicalObject("energy_bar_frame",1);
	targetcross.CreateGraphicalObject("target_cross",1);
	targetcrossblue.CreateGraphicalObject("target_cross_blue",1);
	targetlaser.CreateGraphicalObject("laser_beam",1);
	drone.CreateGraphicalObject("laser_drone",16,8.0);
	shielddrone.CreateGraphicalObject("shield_drone",16,8.0);
	droneshield.CreateGraphicalObject("drone_shield",1);
	shield.CreateGraphicalObject("shield",1);
	shieldbar.CreateGraphicalObject("shield_bar",1);
	shieldframe.CreateGraphicalObject("shield_bar_frame",1);
	textpower.CreateGraphicalObject("text_power",1);
	textspeed.CreateGraphicalObject("text_speed",1);
	textpowerused.CreateGraphicalObject("text_powerused",1);
	textenergy.CreateGraphicalObject("text_energy",1);
	texttotal.CreateGraphicalObject("text_total",1);
	basebg.CreateGraphicalObject("base_bg_tile",1);
	bloodstonelogo.CreateGraphicalObject("bloodstonelogo",1);
	projperilogo.CreateGraphicalObject("retaliation",1);
	presents.CreateGraphicalObject("presents",1);
	hitbar.CreateGraphicalObject("hit_bar",1);
	hitbarframe.CreateGraphicalObject("hit_bar_frame",1);
	teleportblob.CreateGraphicalObject("teleport_blob",1);
	exitgateblob.CreateGraphicalObject("exit_gate_blob",1);
	spark.CreateGraphicalObject("spark",1);
	line.CreateGraphicalObject("rendered_line",1);
	lightning1[0].CreateGraphicalObject("lightning_bolt_sections_1",16);
	lightning2[0].CreateGraphicalObject("lightning_bolt_sections_2",16);
	lightning3[0].CreateGraphicalObject("lightning_bolt_sections_3",16);
	lightning1[1].CreateGraphicalObject("lightning_bolt_sections_1g",16);
	lightning2[1].CreateGraphicalObject("lightning_bolt_sections_2g",16);
	lightning3[1].CreateGraphicalObject("lightning_bolt_sections_3g",16);
	beamsectionred.CreateGraphicalObject("beam_section_red",1);
	beamsectiongreen.CreateGraphicalObject("beam_section_green",1);
	beamsectionblue.CreateGraphicalObject("beam_section_blue",1);
	menubg.CreateGraphicalObject("spacebackground",1);
	missiledrone.CreateGraphicalObject("missile_drone",32,8.0);
	AlphaFromColour(&teleportblob);
	AlphaFromColour(&exitgateblob);
	AlphaFromColour(&explosion);
	AlphaFromColour(&font);
	AlphaFromColour(&menufont);
	AlphaFromColour(&loading);

	//temporaryobject = new GraphicalObject;
	//temporaryobject->CreateGraphicalObject("text_total",1);

	mousefollowtarget.CreateGraphicalObject("mouse_follow_target",1);
/*	menuexit.CreateGraphicalObject("menu_exit",1);
	menuresume.CreateGraphicalObject("menu_resume",1);
	menu1p.CreateGraphicalObject("menu_1p",1);
	menu2p.CreateGraphicalObject("menu_2p",1);
	menugame.CreateGraphicalObject("menu_game",1);
	menugraphics.CreateGraphicalObject("menu_graphics",1);
	menusound.CreateGraphicalObject("menu_sound",1);
	menucontrol.CreateGraphicalObject("menu_control",1);*/

	base[0].player = 0;
	base[1].player = 1;

	meterbar[0].CreateGraphicalObject("meter1",1);
	meterbar[1].CreateGraphicalObject("meter2",1);
	meterbar[2].CreateGraphicalObject("meter3",1);

	for(int i=0;i<3;i++)
	{
		base[0].meter[i].bar = &meterbar[i];
		base[1].meter[i].bar = &meterbar[i];
	}

	ReadWeapons();
	for(int i=0;i<10;i++) basedata[i].SetWeapons(numberofweapons);

	SetUpEquipment();
	if(lpDSO!=NULL) LoadSounds();

	if(options.playintro) gamemode = INTRO;	//INTRO to see the intro logos, and MENU to skip them
	else gamemode = SHORTINTRO;
	storegamemode = gamemode;

	DrawDividingLine();
}

void SetScreenCoords()
{
	if(playmode==SINGLEPLAYER)
	{
		ship[0].scrtopleft.x = 0;
		ship[0].scrtopleft.y = 0;
		ship[0].scrbottomright.x = g_screenx-1;
		ship[0].scrbottomright.y = g_screeny-1;
		ship[0].scrposition.x = g_screenx/2;
		ship[0].scrposition.y = (g_screeny*2)/3;
		ship[0].screencorner[TOPLEFT].x = -ship[0].scrposition.x;
		ship[0].screencorner[TOPLEFT].y = -ship[0].scrposition.y;
		ship[0].screencorner[TOPRIGHT].x = ship[0].scrposition.x;
		ship[0].screencorner[TOPRIGHT].y = -ship[0].scrposition.y;
		ship[0].screencorner[BOTTOMRIGHT].x = ship[0].scrposition.x;
		ship[0].screencorner[BOTTOMRIGHT].y = g_screeny - ship[0].scrposition.y; 
		ship[0].screencorner[BOTTOMLEFT].x = -ship[0].scrposition.x;
		ship[0].screencorner[BOTTOMLEFT].y = g_screeny - ship[0].scrposition.y;
		ship[0].screen.SetPoints(4);
		ship[0].screenscale = screenratio;
		ship[0].radarposition.x = ship[0].scrposition.x;		//Centre of radar position on screen
		ship[0].radarposition.y = g_screeny - screenratio*(float)(RADARHEIGHT/2 + 10);
		ship[0].energyposition.x = screenratio*(float)(ship[0].scrtopleft.x + 20 + energybar.info.Width/2);
		ship[0].energyposition.y = screenratio*(float)(ship[0].scrtopleft.y + 20 + energybar.info.Height/2);
		ship[0].messageposition.x = lrintf(screenratio*100.0f);
		ship[0].messageposition.y = ship[0].radarposition.y - lrintf(screenratio*50.0f);
		base[0].CreateLayout(0);
	}
	else
	{
		ship[0].scrtopleft.x = 0;
		ship[0].scrtopleft.y = 0;
		ship[0].scrbottomright.x = g_screenx/2 - 1;
		ship[0].scrbottomright.y = g_screeny - 1;
		ship[0].scrposition.x = g_screenx/4;
		ship[0].scrposition.y = 2*g_screeny/3;
		ship[0].screencorner[TOPLEFT].x = -ship[0].scrposition.x;
		ship[0].screencorner[TOPLEFT].y = -ship[0].scrposition.y;
		ship[0].screencorner[TOPRIGHT].x = ship[0].scrposition.x;
		ship[0].screencorner[TOPRIGHT].y = -ship[0].scrposition.y;
		ship[0].screencorner[BOTTOMRIGHT].x = ship[0].scrposition.x;
		ship[0].screencorner[BOTTOMRIGHT].y = g_screeny - ship[0].scrposition.y; 
		ship[0].screencorner[BOTTOMLEFT].x = -ship[0].scrposition.x;
		ship[0].screencorner[BOTTOMLEFT].y = g_screeny - ship[0].scrposition.y;
		ship[0].screen.SetPoints(4);
		ship[0].screenscale = screenratio;
		ship[0].radarposition.x = ship[0].scrposition.x;		//Centre of radar position on screen
		ship[0].radarposition.y = g_screeny - screenratio*(float)(RADARHEIGHT/2 + 10);
		ship[0].energyposition.x = screenratio*(float)(ship[0].scrtopleft.x + 20 + energybar.info.Width/2);
		ship[0].energyposition.y = screenratio*(float)(ship[0].scrtopleft.y + 20 + energybar.info.Height/2);
		ship[0].messageposition.x = lrintf(screenratio*100.0f);
		ship[0].messageposition.y = ship[0].radarposition.y - lrintf(screenratio*50.0f);
		base[0].CreateLayout(0);

		ship[1].scrtopleft.x = g_screenx/2 + 1;
		ship[1].scrtopleft.y = 0;
		ship[1].scrbottomright.x = g_screenx - 1;
		ship[1].scrbottomright.y = g_screeny - 1;
		ship[1].scrposition.x = 3*g_screenx/4;
		ship[1].scrposition.y = 2*g_screeny/3;
		ship[1].screencorner[TOPLEFT].x = -ship[0].scrposition.x;
		ship[1].screencorner[TOPLEFT].y = -ship[0].scrposition.y;
		ship[1].screencorner[TOPRIGHT].x = ship[0].scrposition.x;
		ship[1].screencorner[TOPRIGHT].y = -ship[0].scrposition.y;
		ship[1].screencorner[BOTTOMRIGHT].x = ship[0].scrposition.x;
		ship[1].screencorner[BOTTOMRIGHT].y = g_screeny - ship[0].scrposition.y; 
		ship[1].screencorner[BOTTOMLEFT].x = -ship[0].scrposition.x;
		ship[1].screencorner[BOTTOMLEFT].y = g_screeny - ship[0].scrposition.y;
		ship[1].screen.SetPoints(4);
		ship[1].screenscale = screenratio;
		ship[1].radarposition.x = ship[1].scrposition.x;		//Centre of radar position on screen
		ship[1].radarposition.y = g_screeny - screenratio*(float)(RADARHEIGHT/2 + 10);
		ship[1].energyposition.x = ship[1].scrtopleft.x + screenratio*(20.0f + (float)(energybar.info.Width*0.5));
		ship[1].energyposition.y = ship[1].scrtopleft.y + screenratio*(20.0f + (float)(energybar.info.Height*0.5));
		ship[1].messageposition.x = ship[1].scrtopleft.x + lrintf(screenratio*100.0f);
		ship[1].messageposition.y = ship[1].radarposition.y - lrintf(screenratio*50.0f);
		base[1].CreateLayout(1);
	}
}

//This function sets up a single game within the program
bool InitialiseGame(bool skiploading)
{
	//dbout<<"Initialising"<<endl;

	scene.SceneBegin(&dx9.MainViewPort);
	DrawGraphicalObject(&menubg,g_screenx/2,g_screeny/2,0,0,screenratio*1.171875f,true,0xffffffff);
	DrawGraphicalObject(&loading,g_screenx/2,g_screeny/2,0,0,screenratio,true,0xff00ff00);
	scene.SceneFlip();

	ship[0].graphics.ClearGraphicalObject();
	ship[1].graphics.ClearGraphicalObject();

	SetScreenCoords();
	SetViewPorts();

	ship[1].state = NONE;
	for(int i=0;i<playmode+1;++i)
	{
		if(!LoadShip(&ship[i],"aegis_defence")) dbout<<"Loading of ship failed"<<endl;
		else dbout<<"Loading of ship successful"<<endl;
		ship[i].radarrange = 0.032f;
		ship[i].radar.bumpmapped = false;
		ship[i].radar.frames = 1;
		ship[i].weapon = &weaponlist[GetWeaponNumber("Ion_Blaster")];
		ship[i].power += weaponlist[GetWeaponNumber("Ion_Blaster")].power;
		ship[i].state = PLAY;
		ship[i].mousecontrol = false;
		ship[i].stealthed = false;
		ship[i].invulnerable = true;
		ship[i].invulnerabletimer = timeGetTime();
		ship[i].invframe = true;
		ship[i].bombtimer = 0xffffffff;
		ship[i].bombnew = true;
		ship[i].alpha = 0xffffffff;
		ship[i].screenalpha = 0xffffffff;
		ship[i].lightningtree.bolts = 0;
	}

	//dbout<<"Ship initialisation done"<<endl;

	//Only do this stuff if it's a new game, skip it if it's a loaded game
	if(!skiploading)
	{
		nextlevel = firstlevel;
		if(!LoadLevel(level)) return false;

		for(int i=0;i<playmode+1;i++)
		{
			strcpy_s(ship[i].startship,100,ship[i].filename);		//For reloading a failed level
			for(int il=0;il<NUMEQUIPTYPES;il++)
			{
				ship[i].equipment[il] = false;
				equipment[il] = false;
			}
			for(int il=0;il<numberofweapons;++il) weaponlist[il].available = false;
			ship[i].equipment[RADAR] = true;
			ship[i].equipment[SCANNER] = true;
			ship[i].cosangle = cos(ship[i].angle);
			ship[i].sinangle = sin(ship[i].angle);
			ship[i].unitvectorforward.x = -ship[i].sinangle;
			ship[i].unitvectorforward.y = -ship[i].cosangle;
			RotateOutline(&ship[i].outline,&ship[i].rotatedoutline,ship[i].cosangle,ship[i].sinangle);
			TranslateOutline(&ship[i].rotatedoutline,&ship[i].predictedoutline,&ship[i].position);
		}
		weaponlist[GetWeaponNumber("Ion_Blaster")].available = true;
		Save();
	}

	newlevel = true;
	#ifdef OR_DEBUG
	//dbout<<"Starting game ..."<<endl;
	#endif
	return true;
}


void Play()
{
	if(newlevel)
	{
		newlevel = false;
		soundeffect[SP_SYSTEMS_ACTIVATED].SetSoundEffect(0);
	}
	if(lasttick-ship[0].invulnerabletimer>2500) ship[0].invulnerable = false;
	if(lasttick-ship[1].invulnerabletimer>2500) ship[1].invulnerable = false;

	UpdateInput(options.mouseplayer);
	MoveShip(0);

	base[0].ProcessBase();
	if(playmode==TWOPLAYER)
	{
		MoveShip(1);
		base[1].ProcessBase();
	}

	MoveObjects();			//Possibly the most expensive function
	if(boundarychanged)	boundarychanged = false;

	CollisionDetect();
	if(boundarychanged)
	{
		//Finds which nodes are visible to each other
		CalculateNavigationMesh();
		//Check to see whether player has been trapped or crushed
		if(PlayerBoundaryCollision(0)) KillShip(0);
		if(playmode==TWOPLAYER) if(PlayerBoundaryCollision(1)) KillShip(1);
	}

	Update();
	Render();
	PlaySounds();
}

void PlayExiting()
{
	static float shipfade = 1.0f;

	soundeffect[CHARGEUP].stop = true;

	MoveObjects();
	BroadPhase();
	Update();
	Render();
	PlaySounds();

	if(shipfade>0)
	{
		shipfade -= 1.0f*frametime;
		ship[0].alpha = ship[0].alpha&0xffffff | (lrintf(shipfade*255.0f)<<24);
		if(playmode==TWOPLAYER) ship[1].alpha = ship[0].alpha;
	}
	else
	{
		gamemode = LOADLEVEL;
		shipfade = 1.0f;
		ship[0].alpha = 0xffffffff;
		ship[1].alpha = 0xffffffff;
	}
}

//This function clears out the level and all associated files ready for the next one
void ClearCurrentLevel()
{
	g_pMediaControl->Stop();
	CleanUpDirectShow();

	entity.clear();
	entity.resize(1);
	entityfade.clear();
	entityplayer.clear();

	base[0].data = NULL;
	base[1].data = NULL;

	ship[0].drone = NULL;
	ship[1].drone = NULL;
	for(int m=0;m<3;m++) ship[0].shielddrone[m] = NULL;
	for(int m=0;m<3;m++) ship[1].shielddrone[m] = NULL;

	vector<GraphicalObject>::iterator iter;
	int i=0;
	for(iter = graphics.begin(); iter != graphics.end(); i++)
	{
		//dbout<<"Erasing "<<iter->name<<", "<<i<<endl;
		iter = graphics.erase(iter);
	}

	DeleteLevel(level);
}

void SetUpEquipment()
{
	equipdata[NOTHING].name = "Empty";
	equipdata[BOOSTER].name = "Booster";
	equipdata[RADAR].name = "Radar";
	equipdata[SCANNER].name = "Scanner";
	equipdata[SEEKER].name = "Hyperseeker";
	equipdata[GEMTRACTOR].name = "Gemtractor";
	equipdata[MISSILE_LAUNCHER].name = "Missile Launcher";
	equipdata[MISSILES].name = "Missile Crate";
	equipdata[SHIELD_DRONE].name = "Shield Drones";
	equipdata[SHIELD].name = "Shield";
	equipdata[SELF_REPAIR].name = "Self Repair Unit";
	equipdata[SIDE_BOOST].name = "Side Boost";
	equipdata[BOMB].name = "Bomb";
	equipdata[TARGET_LASER].name = "Target Laser";
	equipdata[LASER_DRONE].name = "Drone";
	equipdata[LIGHTNING_DRONE].name = "Lightning Drone";
	equipdata[HOMING_COMPUTER].name = "Targetting Computer";
	equipdata[SHIELD_RECHARGER].name = "Shield Recharger";
	equipdata[LIGHTNING_POD].name = "Lightning Pod";
	equipdata[SPREADER].name = "Spreader";
	equipdata[MINIMISSILES].name = "Mini-Missile Launcher";
	equipdata[MISSILEBOMBS].name = "Missile Bombs";
	equipdata[AUXILIARY_GENERATOR].name = "Auxiliary Generator";

	equipdata[NOTHING].description = "No item selected";
	equipdata[BOOSTER].description = "Increases the speed and handling capabilities of your ship.";
	equipdata[RADAR].description = "Scans the surrounding area for enemy activity - virtually essential.";
	equipdata[SCANNER].description = "Combined with a radar, this piece of kit makes sure you can locate that last pesky enemy ship by altering your radar's range.";
	equipdata[GEMTRACTOR].description = "A resonant EM field pulls nearby gemtonium crystals towards your current location. Never miss crystals in the heat of a battle again.";
	equipdata[MISSILE_LAUNCHER].description = "Fires devastating missiles at the enemy.";
	equipdata[MISSILES].description = "Ten missiles for use with the launcher.";
	equipdata[SEEKER].description = "The ultimate defensive accessory - a burst of four plasma torpedoes that seek out the most threatening object in the vicinity. This weapon will certainly give you the edge in any melee, though it may not target very small craft.";
	equipdata[SHIELD_DRONE].description = "A trio of small but tough drones that absorb enemy bullets. Some pilots think of this as a novelty item, but it can be a lifesaver in that low-energy dash to the teleporter.";
	equipdata[SHIELD].description = "Reduces damage caused by enemy ordnance - but requires recharging after taking hits.";
	equipdata[SELF_REPAIR].description = "Gradually repairs damage to your craft - requires a lot of power but can make you almost invincible.";
	equipdata[SIDE_BOOST].description = "Enables your craft to jump sideways to avoid enemy fire.";
	equipdata[BOMB].description = "Sends out a massive 360` burst of liquid fire to incinerate enemy craft - single use only, but devastating in enclosed spaces.";
	equipdata[TARGET_LASER].description = "Cheap, low-power laser beam for targeting purposes.";
	equipdata[LASER_DRONE].description = "Small, tough drone that orbits you and shoots when you do - basically a gun with an engine.";
	equipdata[LIGHTNING_DRONE].description = "Small, tough drone that orbits you and unleashes bolts of incredible electrical energy at the nearest enemy";
	equipdata[HOMING_COMPUTER].description = "Guides missiles towards the nearest enemy target with deadly accuracy. Additionally, supplies information on the nearest enemy in range.";
	equipdata[SHIELD_RECHARGER].description = "Recharges your shield over time.";
	equipdata[LIGHTNING_POD].description = "Get a friend and one of these each and watch the buggers fry!";
	equipdata[SPREADER].description = "Creates triple streams of bullets but the energy is divided between each bullet - nonetheless it's great for multiple weak opponents and rapid-fire weapons.";
	equipdata[MINIMISSILES].description = "Fires twin miniature missiles with unlimited ammunition.";
	equipdata[MISSILEBOMBS].description = "Adds a bit of extra punch to your missiles ...";
	equipdata[AUXILIARY_GENERATOR].description = "Adds extra power to your ship's generator by taking it from the engines - and therefore slows your ship down slightly.";

	equipdata[NOTHING].graphics = "empty_object";
	equipdata[BOOSTER].graphics = "equip_booster";
	equipdata[RADAR].graphics = "equip_radar";
	equipdata[SCANNER].graphics = "equip_scanner";
	equipdata[GEMTRACTOR].graphics = "equip_gemtractor";
	equipdata[MISSILE_LAUNCHER].graphics = "equip_missilelauncher";
	equipdata[MISSILES].graphics = "equip_missilecrate";
	equipdata[SEEKER].graphics = "equip_hyperseeker";
	equipdata[SHIELD_DRONE].graphics = "equip_shielddrone";
	equipdata[SHIELD].graphics = "equip_shield";
	equipdata[SELF_REPAIR].graphics = "equip_selfrepairunit";
	equipdata[SIDE_BOOST].graphics = "equip_sideboost";
	equipdata[BOMB].graphics = "equip_bomb";
	equipdata[TARGET_LASER].graphics = "equip_targetlaser";
	equipdata[LASER_DRONE].graphics = "equip_drone";
	equipdata[LIGHTNING_DRONE].graphics = "equip_lightningdrone";
	equipdata[HOMING_COMPUTER].graphics = "equip_targettingcomputer";
	equipdata[SHIELD_RECHARGER].graphics = "equip_shieldrecharger";
	equipdata[LIGHTNING_POD].graphics = "equip_lightningpod";
	equipdata[SPREADER].graphics = "equip_spreader";
	equipdata[MINIMISSILES].graphics = "equip_minimissilelauncher";
	equipdata[MISSILEBOMBS].graphics = "equip_missilebombs";
	equipdata[AUXILIARY_GENERATOR].graphics = "equip_auxgenerator";

	/*NOTHING, BOOSTER, RADAR, MISSILE_LAUNCHER, MISSILES, SHIELD_DRONE, TELEPORTER, TELEPORT_POD, SHIELD, SELF_REPAIR, SIDE_BOOST, 
	STEALTH, BOMB, TARGET_LASER, DRONE, HOMING_COMPUTER, SHIELD_RECHARGER, LIGHTNING_POD, SPREADER, MINIMISSILES, MISSILEBOMBS, NUMEQUIPTYPES */

	equipdata[BOOSTER].value = 10.0f;				//Implemented
	equipdata[BOOSTER].power = 20.0f;
	equipdata[RADAR].value = 400.0f;				//Implemented
	equipdata[RADAR].power = 10.0f;
	equipdata[SCANNER].value = 1.0f;				//Implemented
	equipdata[SCANNER].power = 10.0f;
	equipdata[GEMTRACTOR].value = 40000.0f;			//Implemented
	equipdata[GEMTRACTOR].power = 15.0f;
	equipdata[MISSILE_LAUNCHER].value = 20.0f;		//Implemented
	equipdata[MISSILE_LAUNCHER].power = 25.0f;
	equipdata[MISSILES].value = 10.0f;				//Implemented
	equipdata[MISSILES].power = 0;
	equipdata[SEEKER].value = 40000.0f;				//Implemented
	equipdata[SEEKER].power = 80.0f;
	equipdata[SHIELD_DRONE].value = 200.0f;			//Implemented
	equipdata[SHIELD_DRONE].power = 25.0f;
	equipdata[SHIELD].value = 100.0f;				//Implemented
	equipdata[SHIELD].power = 40.0f;
	equipdata[SELF_REPAIR].value = 10.0f;			//Implemented
	equipdata[SELF_REPAIR].power = 120.0f;
	equipdata[SIDE_BOOST].value = 5.0f;				//Not implemented
	equipdata[SIDE_BOOST].power = 20.0f;
	equipdata[BOMB].value = 10.0f;					//Implemented
	equipdata[BOMB].power = 20.0f;
	equipdata[TARGET_LASER].value = 10.0f;			//Implemented
	equipdata[TARGET_LASER].power = 5.0f;
	equipdata[LASER_DRONE].value = 10.0f;			//Implemented
	equipdata[LASER_DRONE].power = 50.0f;
	equipdata[LIGHTNING_DRONE].value = 10.0f;		//Implemented
	equipdata[LIGHTNING_DRONE].power = 80.0f;
	equipdata[HOMING_COMPUTER].value = 10.0f;		//Implemented
	equipdata[HOMING_COMPUTER].power = 20.0f;
	equipdata[SHIELD_RECHARGER].value = 10.0f;		//Implemented
	equipdata[SHIELD_RECHARGER].power = 100.0f;
	equipdata[LIGHTNING_POD].value = 10.0f;			//Not implemented
	equipdata[LIGHTNING_POD].power = 200.0f;
	equipdata[SPREADER].value = 10.0f;				//Implemented
	equipdata[SPREADER].power = 10.0f;
	equipdata[MINIMISSILES].value = 10.0f;			//Implemented
	equipdata[MINIMISSILES].power = 50.0f;
	equipdata[MISSILEBOMBS].value = 20.0f;			//Implemented
	equipdata[MISSILEBOMBS].power = 40.0f;
	equipdata[AUXILIARY_GENERATOR].value = 0.8f;	//Implemented
	equipdata[AUXILIARY_GENERATOR].power = -40.0f;

	return;
}


// initialization function
bool InitDirectInput()
{
	if (FAILED(DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&lpdi, NULL)))
		return false;

	// initialize the mouse
	if (FAILED(lpdi->CreateDevice(GUID_SysMouse, &m_mouse, NULL)))
		return false;
	if (FAILED(m_mouse->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE)))
		return false;
	if (FAILED(m_mouse->SetDataFormat(&c_dfDIMouse)))
		return false;
	if (FAILED(m_mouse->Acquire()))
		return false;

	return true;
}

bool ReleaseInput(void)
{
  m_mouse->Unacquire();
  m_mouse->Release();
  m_mouse = NULL;

  lpdi->Release();
  lpdi = NULL;

  return true;
}


bool UpdateInput(int player)
{
	if(player<0) return false;
	if (FAILED(m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&mouse_state)))
		return false;

	mousetarget.UpdatePosition((float)mouse_state.lX,(float)mouse_state.lY, player);
	return true;
}

/*
typedef struct DIMOUSESTATE { 
  LONG lX;
  LONG lY;
  LONG lZ; 
  BYTE rgbButtons[4]; 
} DIMOUSESTATE, *LPDIMOUSESTATE;
*/

void Mousetarget::ResetMousetarget()
{
	direction = CENTRED;
	angle = 0;
	distance = 70.0;
	scrposition.x = 0;
	scrposition.y = 0;
}


void Mousetarget::PolarToCart(int player)
{
	position.x = ship[player].position.x + (float)sin((float)angle)*distance;
	position.y = ship[player].position.y + (float)cos((float)angle)*distance;
}


void Mousetarget::UpdatePosition(float dx, float dy, int player)
{	static float tx = 0.0f, ty = 0.0f, operatetimer = 0.0f;
	static bool operatepressed = false;

	angle -= 0.0040f*dx;
	if(angle<0.0f) angle += TWOPI;
	else if(angle>=TWOPI) angle -= TWOPI;

	distance -= 0.08f*dy;
	if(distance>90.0f) distance = 90.0f;
	if(distance<40.0f) distance = 40.0f;

	PolarToCart(player);							//Convert the input to cartesian

	ship[player].keys.UP = false;
	ship[player].keys.up = 1.0;
	ship[player].keys.DOWN = false;
	ship[player].keys.down = 1.0;
	ship[player].keys.LEFT = false;
	ship[player].keys.left = 1.0;
	ship[player].keys.RIGHT = false;
	ship[player].keys.right = 1.0;
	ship[player].keys.FIRE = false;
	ship[player].keys.FIREMISSILE = false;

	if(ship[player].state==PLAY)
	{
		RotateToMouse(&ship[player],0.08f);

		if(mouse_state.rgbButtons[0]) ship[player].keys.FIRE = true;
		if(mouse_state.rgbButtons[1]) ship[player].keys.UP = true;
		if(mouse_state.rgbButtons[2])
		{
			ship[player].keys.OPERATE = true;
			if(ship[player].bombtimer==0xffffffff)
			{
				ship[player].bombtimer = thistick; //Start the bomb timer running
			}
		}
		else
		{
			ship[player].bombtimer = 0xffffffff;
		}
		if(mouse_state.lZ!=0) ship[player].keys.FIREMISSILE = true;

		//if(distance>75.0f) ship[player].keys.UP = true;
		if(distance<60.0f)
		{
			ship[player].keys.DOWN = true;
			distance += frametime*10.0f;
		}
		if(distance>75.0f)
		{
			ship[player].keys.UP = true;
			distance -= frametime*10.0f;
		}

		PosToScreen(&position,&scrposition,player);		//Find target position for drawing
	}
	else if(ship[player].state==INBASE)
	{
		tx += dx;
		ty += dy;

		if(tx>100.0f)
		{	tx = 0.0f;
			ship[player].keys.RIGHT = true;
		}
		else if(tx<-100.0f)
		{	tx = 0.0f;
			ship[player].keys.LEFT = true;
		}

		if(ty>100.0f)
		{	ty = 0.0f;
			ship[player].keys.DOWN = true;
		}
		else if(ty<-100.0f)
		{	ty = 0.0f;
			ship[player].keys.UP = true;
		}

		if(operatepressed)
		{	operatetimer += frametime;
			if(operatetimer>0.5f)
			{	operatetimer = 0.0f;
				operatepressed = false;
			}
		}
		else if(mouse_state.rgbButtons[0] || mouse_state.rgbButtons[1] || mouse_state.rgbButtons[2])
		{
			ship[player].keys.OPERATE = true;
			operatepressed = true;
		}

	}
	mouse_state.rgbButtons[0] = 0;
	mouse_state.rgbButtons[1] = 0;
	mouse_state.rgbButtons[2] = 0;

}

bool Mousetarget::RotateToMouse(Player *object, float tolerance)
{	float angletotarget;
	float x = position.x - object->position.x;
	float y = position.y - object->position.y;
	float xl = cos(-object->angle + PIBYTWO);		//x component of ship's angle
	float yl = sin(-object->angle + PIBYTWO);		//y component of ship's angle
	float dist = magic_inv_sqrt(x*x+y*y);					//distance to selected point
	angletotarget = acos((-xl*x + -yl*y) * dist);
	float crossp = xl*y - yl*x;

	if(angletotarget>tolerance)		//Angle to target must be reduced
	{
		if(crossp<0) object->keys.RIGHT = true;
		else object->keys.LEFT = true;
	}

	return true;
}

void ReleaseTextures()
{
	error.ClearGraphicalObject();
	energybar.ClearGraphicalObject();
	energybarframe.ClearGraphicalObject();
	debris.ClearGraphicalObject();
	smoke.ClearGraphicalObject();
	missile.ClearGraphicalObject();
	flame.ClearGraphicalObject();
	redbullet.ClearGraphicalObject();
	explosion.ClearGraphicalObject();
	font.ClearGraphicalObject();
	whiteblob.ClearGraphicalObject();
	messagescreen.ClearGraphicalObject();
	interfacescreen.ClearGraphicalObject();
	datascreen.ClearGraphicalObject();
	buttonframe.ClearGraphicalObject();
	for(int i=0;i<6;i++) buttong[i].ClearGraphicalObject(); 
	meterframe.ClearGraphicalObject();	
	meterpanel.ClearGraphicalObject();
	for(int i=0;i<3;i++) meterbar[i].ClearGraphicalObject(); 
	for(int i=0;i<numberofweapons;i++)
	{
		weapongraphics[i].ClearGraphicalObject(); 
		weaponpulsegfx[i].ClearGraphicalObject();
	}
	selectionbar.ClearGraphicalObject();
	loading.ClearGraphicalObject();
	mousefollowtarget.ClearGraphicalObject();
	targetcross.ClearGraphicalObject();
	targetlaser.ClearGraphicalObject();
	drone.ClearGraphicalObject();
	shielddrone.ClearGraphicalObject();
	droneshield.ClearGraphicalObject();
	shield.ClearGraphicalObject();
	shieldbar.ClearGraphicalObject();
	shieldframe.ClearGraphicalObject();
	textpower.ClearGraphicalObject();
	textspeed.ClearGraphicalObject();
	textpowerused.ClearGraphicalObject();
	textenergy.ClearGraphicalObject();
	texttotal.ClearGraphicalObject();
	basebg.ClearGraphicalObject();
	bloodstonelogo.ClearGraphicalObject();
	projperilogo.ClearGraphicalObject();
	presents.ClearGraphicalObject();
	hitbar.ClearGraphicalObject();
	hitbarframe.ClearGraphicalObject();
	teleportblob.ClearGraphicalObject();
	exitgateblob.ClearGraphicalObject();
	spark.ClearGraphicalObject();
	line.ClearGraphicalObject();
	lightning1[0].ClearGraphicalObject();
	lightning2[0].ClearGraphicalObject();
	lightning3[0].ClearGraphicalObject();
	lightning1[1].ClearGraphicalObject();
	lightning2[1].ClearGraphicalObject();
	lightning3[1].ClearGraphicalObject();
	beamsectionred.ClearGraphicalObject();
	beamsectiongreen.ClearGraphicalObject();
	beamsectionblue.ClearGraphicalObject();
	menubg.ClearGraphicalObject();
}

int InitDirectShow()
{
	HRESULT hr;
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder,(void**)&g_pGraphBuilder);

	if (FAILED(hr))	return -1;

	g_pGraphBuilder->QueryInterface(IID_IMediaControl, (void**)&g_pMediaControl);
	g_pGraphBuilder->QueryInterface(IID_IMediaSeeking, (void**)&g_pMediaSeeking);
	g_pGraphBuilder->QueryInterface(IID_IMediaEvent, (void**)&g_pMediaEvent);
	g_pGraphBuilder->QueryInterface(IID_IMediaPosition, (void**)&g_pMediaPosition);
	g_pGraphBuilder->QueryInterface(IID_IBasicAudio, (void**)&g_pBasicAudio);

	SetNotifications();
	return 0;
}

int CreateGraph(char* filename)
{
	int	length;		// length of filename
	WCHAR*	wfilename;	// where we store WCHAR version of filename

	length = strlen(filename)+1;
	wfilename = new WCHAR[length];
	MultiByteToWideChar(CP_ACP, 0, filename, -1, wfilename, length);
	if (FAILED(g_pGraphBuilder->RenderFile(wfilename, NULL))) return -1;
	else return 0;
}

void CleanUpDirectShow()
{
	g_pMediaControl->Stop();
	HELPER_RELEASE(g_pBasicAudio)
    HELPER_RELEASE(g_pMediaPosition)
    HELPER_RELEASE(g_pMediaEvent)
	HELPER_RELEASE(g_pMediaSeeking)
    HELPER_RELEASE(g_pMediaControl)
    HELPER_RELEASE(g_pGraphBuilder)
}

void OnGraphEvent()
{	LONGLONG position = 0;
	long EventCode, Param1, Param2;
	while (g_pMediaEvent->GetEvent(&EventCode, &Param1, &Param2, 0) != E_ABORT)
	{
		switch (EventCode)
		{
		case EC_COMPLETE:
			//dbout<<"Reached end of song"<<endl;
			// here when media is completely done playing
			//g_pMediaControl->Stop();

			g_pMediaSeeking->SetPositions(&position, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);

	//		g_pMediaPosition->put_CurrentPosition(0);   // reset to beginning
			//dbout<<"Reset song"<<endl;
			//g_pMediaControl->Run();
			break;
		default:
			break;
		}	
		g_pMediaEvent->FreeEventParams(EventCode, Param1, Param2);
	}
}

void SetNotifications()
{
	g_pMediaEvent->SetNotifyWindow((OAHWND)hWnd, WM_GRAPHEVENT, 0);
	g_pMediaEvent->SetNotifyFlags(0);	// turn on notifications
}

int ShortIntro()
{
	static int projperitime = 0, alpha = 0xffffff, bgalpha = 0xffffff;
	static bool alphaup = true, soundtriggered = false;


	//Show the 'RETALIATION' logo and the background
	if(thistick - projperitime>20)	//50 frames per second
	{
		if(alphaup)
		{
			ShowLogo(&projperilogo,alpha);	//Display logo
			alpha += 0x3000000;		//Increase alpha value if increasing
		}
		else
		{
			scene.SceneBegin(&dx9.MainViewPort);
			DrawGraphicalObject(&menubg,g_screenx/2,g_screeny/2,0,0,screenratio*1.171875f,true,bgalpha);
			DrawGraphicalObject(&projperilogo,g_screenx/2,g_screeny/2,0,0,screenratio,true,alpha);
			scene.SceneFlip();
			alpha -= 0x3000000;
			bgalpha += 0x6000000;
		}
		if(alphaup && alpha==0xffffffff)
		{
			alphaup = false;	//Change alpha fade direction
			//soundeffect[INTROIMPACT].buffer->Play(0,0,0);
		}
		else if(!alphaup && alpha<=0x80ffffff)
		{
			bloodstonelogo.ClearGraphicalObject();
			presents.ClearGraphicalObject();
			return MENU;
		}
		projperitime = thistick;
	}
	return SHORTINTRO;

}

int Intro()
{
	static int bslogotime = 0, preslogotime = 0, projperitime = 0, alpha = 0xffffff, bgalpha = 0xffffff;
	static bool bslogo = true, preslogo = false, projlogo = false, alphaup = true, soundtriggered = false;

	if(bslogo)	//Showing the 'company' logo
	{
		if(thistick - bslogotime>20)	//50 frames per second
		{
			ShowLogo(&bloodstonelogo,alpha);	//Display logo
			if(alphaup)	alpha += 0x3000000;		//Increase alpha value if increasing
			else alpha -= 0x5000000;
			if(alpha==0xffffffff) alphaup = false;	//Change alpha fade direction
			else if(alpha==0xffffff)
			{
				soundtriggered = false;
				alphaup = true;
				bslogo = false;
				preslogo = true;
			}
			bslogotime = thistick;
		}
		return INTRO;
	}
	else if(preslogo)		//Show the word 'Presents'
	{
		if(thistick - preslogotime>20)	//50 frames per second
		{
			ShowLogo(&presents,alpha);	//Display logo
			if(alphaup)	alpha += 0x5000000;		//Increase alpha value if increasing
			else alpha -= 0x5000000;
			if(alpha==0xffffffff) alphaup = false;	//Change alpha fade direction
			else if(alpha==0xffffff)
			{
				alphaup = true;
				preslogo = false;
				projlogo = true;
			}
			preslogotime = thistick;
		}
		return INTRO;
	}
	else if(projlogo)		//Show the 'RETALIATION' logo and the background
	{
		if(thistick - projperitime>20)	//50 frames per second
		{
			if(alphaup)
			{
				ShowLogo(&projperilogo,alpha);	//Display logo
				alpha += 0x3000000;		//Increase alpha value if increasing
			}
			else
			{
				scene.SceneBegin(&dx9.MainViewPort);
				DrawGraphicalObject(&menubg,g_screenx/2,g_screeny/2,0,0,screenratio*1.171875f,true,bgalpha);
				DrawGraphicalObject(&projperilogo,g_screenx/2,g_screeny/2,0,0,screenratio,true,alpha);
				scene.SceneFlip();
				alpha -= 0x3000000;
				bgalpha += 0x6000000;
			}
			if(alphaup && alpha==0xffffffff)
			{
				alphaup = false;	//Change alpha fade direction
				//soundeffect[INTROIMPACT].buffer->Play(0,0,0);
			}
			else if(!alphaup && alpha<=0x80ffffff)
			{
				bloodstonelogo.ClearGraphicalObject();
				presents.ClearGraphicalObject();
				return MENU;
			}
			projperitime = thistick;
		}
		return INTRO;
	}
	else return MENU;
}

int Outro(int retstate)
{
	D3DCOLOR alphaval, bgalphaval;
	static int projperitime = 0;
	static D3DCOLOR alpha = 127, bgalpha = 268;
	static bool alphaup = true;

	//Show the 'RETALIATION' logo at half brightness, fade to max, then fade out

	if(thistick - projperitime>20)	//50 frames per second
	{
		alphaval = ((alpha<<24)&0xff000000) + 0xffffff;

		if(bgalpha<=255) bgalphaval = ((bgalpha<<24)&0xff000000) + 0xffffff;
		else bgalphaval = 0xffffffff;

		scene.SceneBegin(&dx9.MainViewPort);
		DrawGraphicalObject(&menubg,g_screenx/2,g_screeny/2,0,0,screenratio*1.171875f,true,bgalphaval);
		DrawGraphicalObject(&projperilogo,g_screenx/2,g_screeny/2,0,0,screenratio,true,alphaval);
		scene.SceneFlip();

		if(alphaup)	alpha+=8;		//Increase alpha value if increasing
		else alpha-=5;				//Decrease alpha value if decreasing

		if(bgalpha>=4) bgalpha -= 4;

		if(alphaup && (alpha==255) ) alphaup = false;		//Change alpha fade direction

		if(!alphaup && (alpha==0) )				//Reached minimum, so go to next state
		{
			alphaup = true;
			alpha = 127;
			bgalpha = 268;
			projperitime = 0;
			return retstate;
		}

		projperitime = thistick;
	}
	return OUTRO;
}


void ReadPrefs()
{
	ifstream prefs;
	string junk;
	//Read preferences file *********************
	prefs.open("data\\prefs.dat",ios::in);
	prefs>>junk>>firstlevel;
	prefs>>junk>>junk;
	if(junk == "yes") options.cheatmode = true;
	else options.cheatmode = false;
	prefs>>junk>>junk;
	if(junk == "yes") options.playintro = true;
	else options.playintro = false;
	prefs>>junk>>options.musicvolume;
	prefs>>junk>>options.soundvolume;
	prefs>>junk>>junk;
	if(junk == "yes") options.framecounter = true;
	else options.framecounter = false;
	prefs>>junk>>junk;
	if(junk == "yes") options.aa = true;
	else options.aa = false;
	prefs>>junk>>junk;
	if(junk == "yes") options.acceleration = true;
	else options.acceleration = false;
	prefs>>junk>>junk;

	if(junk == "keyboard") ship[0].controltype = KEYBOARD;
	else ship[0].controltype = GAMEPAD;

	prefs>>junk>>junk;

	if(junk == "keyboard") ship[1].controltype = KEYBOARD;
	else if(junk == "gamepad") ship[1].controltype = GAMEPAD;
	else ship[1].controltype = MOUSE;

	SetPlayerControllers();

	prefs>>junk>>g_screenx;
	prefs>>junk>>g_screeny;

	prefs>>junk>>junk;
	if(junk == "quads") options.drawmethod = QUADS;
	else options.drawmethod = SHADER;

	prefs>>junk>>options.capexplosions;
	prefs>>junk>>junk;
	if(junk == "yes") options.explosionflashes = true;
	else options.explosionflashes = false;
	prefs>>junk>>junk;
	if(junk == "yes") options.explosiondistortions = true;
	else options.explosiondistortions = false;

	prefs.close();
	//End ****************************************
}

void SavePrefs()
{
	ofstream prefs;
	//Save preferences file *********************
	prefs.open("data\\prefs.dat",ios::out);
	prefs<<"first_level "<<firstlevel<<endl;
	prefs<<"cheatmode "<<(options.cheatmode==true?"yes":"no")<<endl;
	prefs<<"playintro "<<(options.playintro==true?"yes":"no")<<endl;
	prefs<<"musicvolume "<<options.musicvolume<<endl;
	prefs<<"soundvolume "<<options.soundvolume<<endl;
	prefs<<"frameratecounter "<<(options.framecounter==true?"yes":"no")<<endl;
	prefs<<"antialiasing "<<(options.aa==true?"yes":"no")<<endl;
	prefs<<"acceleration "<<(options.acceleration==true?"yes":"no")<<endl;
	prefs<<"p1controls "<<(ship[0].controltype == KEYBOARD ? "keyboard" : (ship[0].controltype == MOUSE ? "mouse" : "gamepad" ))<<endl;
	prefs<<"p2controls "<<(ship[1].controltype == KEYBOARD ? "keyboard" : (ship[1].controltype == MOUSE ? "mouse" : "gamepad" ))<<endl;
	prefs<<"screenx "<<g_screenx<<endl;
	prefs<<"screeny "<<g_screeny<<endl;
	prefs<<"drawmethod "<<(options.drawmethod==QUADS?"quads":"shader")<<endl;
	prefs<<"capexplosions "<<options.capexplosions<<endl;
	prefs<<"explosionflashes "<<(options.explosionflashes?"yes":"no")<<endl;
	prefs<<"explosiondistortions "<<(options.explosiondistortions?"yes":"no")<<endl;
	prefs.close();
	//End ****************************************
}

void SetPlayerControllers()
{
	if(ship[0].controltype == KEYBOARD) options.keyboardplayer = 0;
	else options.keyboardplayer = 1;

	if(ship[0].controltype == GAMEPAD) options.gamepadplayer = 0;
	else options.gamepadplayer = 1;

	if(ship[0].controltype == MOUSE) options.mouseplayer = 0;
	else if(ship[1].controltype == MOUSE) options.mouseplayer = 1;
	else options.mouseplayer = -1;
}

string FPClass(float x)
{
    int i = _fpclass(x);
    string s;
    switch (i)
    {
    case _FPCLASS_SNAN: s = "Signaling NaN";                break;
    case _FPCLASS_QNAN: s = "Quiet NaN";                    break; 
    case _FPCLASS_NINF: s = "Negative infinity (-INF)";     break; 
    case _FPCLASS_NN:   s = "Negative normalized non-zero"; break;
    case _FPCLASS_ND:   s = "Negative denormalized";        break; 
    case _FPCLASS_NZ:   s = "Negative zero (-0)";           break; 
    case _FPCLASS_PZ:   s = "Positive 0 (+0)";              break; 
    case _FPCLASS_PD:   s = "Positive denormalized";        break; 
    case _FPCLASS_PN:   s = "Positive normalized non-zero"; break; 
    case _FPCLASS_PINF: s = "Positive infinity (+INF)";     break;
    }
    return s;
}

//struct Scene member functions

Scene::Scene(void)
{
	scenestarted = false;
	spritestarted = false;
}

Scene::~Scene(void)
{
	SceneFlip();
}

bool Scene::SceneBegin(const D3DVIEWPORT9 *pViewport)
{
	if(scenestarted) SceneFlip();
	if(!scenestarted)
	{
		scenestarted = true;
		// Clear the backbuffer to a black color
		dx9.pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );
		// Begin the scene
		dx9.pd3dDevice->BeginScene();
		//Set the viewport and begin the sprite
		ChangeViewport(pViewport);
	}
	return scenestarted;
}

bool Scene::SceneEnd(void)
{
	SpriteEnd();
	if(scenestarted)
	{
		scenestarted = false;
		// End the scene
		dx9.pd3dDevice->EndScene();
	}
	return scenestarted;
}

bool Scene::SceneFlip(void)
{
	//Check and end the scene if necessary
	SceneEnd();
	// Present the backbuffer contents to the display
	if(FAILED(dx9.pd3dDevice->Present( NULL, NULL, NULL, NULL ))) dbout<<"Present failed!"<<endl;
	return scenestarted;
}

bool Scene::ChangeViewport(const D3DVIEWPORT9 *pViewport)
{
	SpriteEnd();
	dx9.pd3dDevice->SetViewport(pViewport);
	return SpriteBegin();
}

bool Scene::SpriteBegin(void)
{
	if(!spritestarted)
	{
		dx9.sprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_DO_NOT_ADDREF_TEXTURE);
		spritestarted = true;
	}
	return spritestarted;
}

bool Scene::SpriteEnd(void)
{
	if(spritestarted)
	{
		dx9.sprite->End();
		spritestarted = false;
	}
	return spritestarted;
}

void InitVolatileResources()
{
	dx9.sprite->OnResetDevice();
}

void FreeVolatileResources()
{
	dx9.sprite->OnLostDevice();
}

bool SetUpControllers()
{
	RAWINPUTDEVICE Rid[2], ridlist[20];
        
	Rid[0].usUsagePage = 0x01; 
	Rid[0].usUsage = 0x05; 
	Rid[0].dwFlags = 0;                 // adds game pad
	Rid[0].hwndTarget = 0;

	Rid[1].usUsagePage = 0x01; 
	Rid[1].usUsage = 0x04; 
	Rid[1].dwFlags = 0;                 // adds joystick
	Rid[1].hwndTarget = 0;

	if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE)
	{
		dbout<<"Controller error: "<<GetLastError();
		return false;
	}

	UINT num = 20, devs = 0;
	if( (devs = GetRegisteredRawInputDevices(ridlist, &num, sizeof(RAWINPUTDEVICE))) < 0)
	{
		dbout<<num<<" raw input HID devices registered - buffer too small"<<endl;
	}
	else
	{
		//dbout<<devs<<" raw input HID devices registered"<<endl;
	}

	return true;
}

void GetInput(WPARAM wParam, LPARAM lParam, int player)
{
	UINT dwSize;

	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));

	LPBYTE lpb = new BYTE[dwSize];
	if (lpb == NULL) return;

	if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize )
	{
		 dbout<<"GetRawInputData doesn't return correct size !\n"<<endl; 
	}
	else
	{
		RAWINPUT* raw = (RAWINPUT*)lpb;
		if (raw->header.dwType == RIM_TYPEHID)
		{/*
			dbout<<raw->data.hid.dwCount<<" "<<raw->data.hid.dwSizeHid<<endl;
			for(int i=0;i<raw->data.hid.dwCount*raw->data.hid.dwSizeHid;++i) dbout<<(int)(raw->data.hid.bRawData[i])<<" ";
			dbout<<endl;*/

			ship[player].keys.DOWN = false;
			ship[player].keys.UP = false;
			ship[player].keys.LEFT = false;
			ship[player].keys.RIGHT = false;
			ship[player].keys.down = 1.0f;
			ship[player].keys.up = 1.0f;
			ship[player].keys.left = 1.0f;
			ship[player].keys.right = 1.0f;

			//Start button
			if(raw->data.hid.bRawData[6] == 32)
			{
				if(gamemode==PLAY || gamemode==GAMEOVER || gamemode==INBASE) gamemode = MENU;		//Opens the main menu
			}

			//Shift Left
			if((raw->data.hid.bRawData[6]&0xf) == 0x1 || (raw->data.hid.bRawData[6]&0xf) == 0x2)
			{
				ship[player].keys.SHIFTLEFT = true;
			}
			else
			{
				ship[player].keys.shiftleft = 1.0f;
				ship[player].keys.SHIFTLEFT = false;
			}

			//Shift Right
			if((raw->data.hid.bRawData[6]&0xf) == 0x4 || (raw->data.hid.bRawData[6]&0xf) == 0x8)
			{
				ship[player].keys.SHIFTRIGHT = true;
			}
			else
			{
				ship[player].keys.shiftright = 1.0f;
				ship[player].keys.SHIFTRIGHT = false;
			}

			//----------------------Analogue stick-----------------------------

			//Left stick L/R
			if(raw->data.hid.bRawData[1]!=128)
			{
				if(raw->data.hid.bRawData[1]<128)
				{
					ship[player].keys.LEFT = true;
					ship[player].keys.left = (float)(128 - raw->data.hid.bRawData[1])/128.0f;
					if(ship[player].keys.left<0.05f)
					{
						ship[player].keys.left = 1.0f;
						ship[player].keys.LEFT = false;
					}
				}
				else
				{
					ship[player].keys.LEFT = false;
					ship[player].keys.left = 1.0f;
				}
				if(raw->data.hid.bRawData[1]>128)
				{
					ship[player].keys.RIGHT = true;
					ship[player].keys.right = (float)(raw->data.hid.bRawData[1] - 128)/128.0f;
					if(ship[player].keys.right<0.05f)
					{
						ship[player].keys.right = 1.0f;
						ship[player].keys.RIGHT = false;
					}
				}
				else
				{
					ship[player].keys.RIGHT = false;
					ship[player].keys.right = 1.0f;
				}
			}

			//Left stick U/D
			if(raw->data.hid.bRawData[2]!=128)
			{
				if(raw->data.hid.bRawData[2]<128)
				{
					ship[player].keys.UP = true;
					ship[player].keys.up = (float)(128 - raw->data.hid.bRawData[2])/128.0f;
					if(ship[player].keys.up<0.05f)
					{
						ship[player].keys.UP = false;
						ship[player].keys.up = 1.0f;
					}
				}
				else
				{
					ship[player].keys.UP = false;
					ship[player].keys.up = 1.0f;
				}
				if(raw->data.hid.bRawData[2]>128)
				{
					ship[player].keys.DOWN = true;
					ship[player].keys.down = (float)(raw->data.hid.bRawData[2] - 128)/128.0f;
					if(ship[player].keys.down<0.05f)
					{
						ship[player].keys.DOWN = false;
						ship[player].keys.down = 1.0f;
					}
				}
				else
				{
					ship[player].keys.DOWN = false;
					ship[player].keys.down = 1.0f;
				}
			}

			//---------------------------D-Pad--------------------------------

			//Left
			if((raw->data.hid.bRawData[5]&0xf) == 0x7 || (raw->data.hid.bRawData[5]&0xf) == 0x6 || (raw->data.hid.bRawData[5]&0xf) == 0x8)
			{
				ship[player].keys.LEFT = true;
				ship[player].keys.left = 1.0f;
			}

			//Up
			if((raw->data.hid.bRawData[5]&0xf) == 0x1 || (raw->data.hid.bRawData[5]&0xf) == 0x8 || (raw->data.hid.bRawData[5]&0xf) == 0x2)
			{
				ship[player].keys.UP = true;
				ship[player].keys.up = 1.0f;
			}

			//Right
			if((raw->data.hid.bRawData[5]&0xf) == 0x3 || (raw->data.hid.bRawData[5]&0xf) == 0x2 || (raw->data.hid.bRawData[5]&0xf) == 0x4)
			{
				ship[player].keys.RIGHT = true;
				ship[player].keys.right = 1.0f;
			}

			//Down
			if((raw->data.hid.bRawData[5]&0xf) == 0x5 || (raw->data.hid.bRawData[5]&0xf) == 0x4 || (raw->data.hid.bRawData[5]&0xf) == 0x6)
			{
				ship[player].keys.DOWN = true;
				ship[player].keys.down = 1.0f;
			}

			//---------------------------------------Buttons-------------------------------

			if(raw->data.hid.bRawData[5] & 0x10)
			{
				ship[player].keys.FIRE = true;
			}
			else
			{
				ship[player].keys.FIRE = false;
			}
			if(raw->data.hid.bRawData[5] & 0x20)
			{
				ship[player].keys.FIREMISSILE = true;
			}
			else
			{
				ship[player].keys.FIREMISSILE = false;
			}
			if(raw->data.hid.bRawData[5] & 0x40)
			{
				ship[player].keys.OPERATE = true;
			}
			else
			{
				ship[player].keys.OPERATE = false;
			}
			if(raw->data.hid.bRawData[5] & 0x80)
			{
				if(ship[player].bombtimer==0xffffffff) ship[player].bombtimer = thistick;
			}
			else
			{
				ship[player].bombtimer = 0xffffffff;
			}
		}
	}

	delete[] lpb; 
	return;
}

bool CheckDeviceReady()
{
	HRESULT hr = dx9.pd3dDevice->TestCooperativeLevel();
	if(FAILED(hr))
	{
		if(hr == D3DERR_DEVICELOST)
		{	//Our device is lost, can't get it back yet
			dbout<<"Device lost, not ready"<<endl;
			Sleep(500);
			return false;
		}
		else if(hr == D3DERR_DEVICENOTRESET)
		{	//Our device needs resetting
			dbout<<"Device needs resetting ";
			FreeVolatileResources();
			hr = dx9.pd3dDevice->Reset(&dx9.d3dpp);
			if(SUCCEEDED(hr))
			{
				Sleep(10);
				InitVolatileResources();
				lasttick = thistick = timeGetTime();
				dbout<<"- reset succeeded."<<endl;
				return true;
			}
			else
			{
				dbout<<"- reset failed! :"<<hr<<endl;
				Sleep(500);
				return false;
			}
		}
		else
		{
			dbout<<"Major error!"<<endl;
			PostQuitMessage(2);	//Something else wrong
			return false;
		}
	}
	else return true;
}

bool TestKeyPressed()
{
	bool keypressed = false;

	if(	ship[options.keyboardplayer].keys.OPERATE )
	{
		ship[options.keyboardplayer].keys.OPERATE = false;
		keypressed = true;
	}
	if(	ship[options.gamepadplayer].keys.OPERATE )
	{
		ship[options.gamepadplayer].keys.OPERATE = false;
		keypressed = true;
	}
	if(	ship[options.keyboardplayer].keys.FIRE )
	{
		ship[options.keyboardplayer].keys.FIRE = false;
		keypressed = true;
	}
	if(	ship[options.gamepadplayer].keys.FIRE )
	{
		ship[options.gamepadplayer].keys.FIRE = false;
		keypressed = true;
	}
	if(	ship[options.keyboardplayer].keys.FIREMISSILE )
	{
		ship[options.keyboardplayer].keys.FIREMISSILE = false;
		keypressed = true;
	}
	if(	ship[options.gamepadplayer].keys.FIREMISSILE )
	{
		ship[options.gamepadplayer].keys.FIREMISSILE = false;
		keypressed = true;
	}
	return keypressed;
}

void Save()
{
	if(!SaveGame((playmode==SINGLEPLAYER?"data\\savefile1p.dat":"data\\savefile2p.dat"),&level)) dbout<<"Could not save game"<<endl;
	else
	{
		BOOL errorcode;
		if(playmode==SINGLEPLAYER) errorcode = CopyFile("data\\savefile1p.dat","data\\level1p.dat",false);
		else errorcode = CopyFile("data\\savefile2p.dat","data\\level2p.dat",false);
		if(errorcode == 0) dbout<<"Error saving level file!"<<endl;
		//dbout<<"Game saved"<<endl;
	}
}


