#include "OR.h"

extern LPDIRECTSOUND8 lpDSO;
extern SoundEffect soundeffect[NUMEFFECTS];
extern ofstream dbout;
extern Weapon *weaponlist;
extern Options options;
extern int numberofweapons;

bool Sound_Initialize(HWND hWnd)
{ 
	HRESULT hr;				//handle to a result variable

	hr = DirectSoundCreate8(NULL,&lpDSO,NULL);
	if(hr==DS_OK)			//see if Create worked
	{
		hr = IDirectSound_SetCooperativeLevel(lpDSO, hWnd, DSSCL_EXCLUSIVE);
		if(hr == DS_OK)		//see if Coop worked 
		{
			return true;	//we initialized successfully, return true
        }
	}
	return false;			//problem occured, let's send back a false
}


void Sound_Exit()
{
	DWORD stat;
	for(int i=0;i<NUMEFFECTS;i++)
	{	if(soundeffect[i].buffer!=NULL)
		{
			soundeffect[i].buffer->GetStatus(&stat);
			if(stat==DSBSTATUS_PLAYING)
				soundeffect[i].buffer->Stop();
			HELPER_RELEASE(soundeffect[i].buffer)
		}
	}
	//make sure the SoundObject even exists
	if (lpDSO) IDirectSound_Release(lpDSO);
}

HRESULT LoadDirectSound(LPDIRECTSOUNDBUFFER *buffer,char *filename)
{
      //open a wav file
    HMMIO wavefile;

	wavefile = mmioOpen(filename,0,MMIO_READ|MMIO_ALLOCBUF);

	if(wavefile==NULL) 
		return 1;

      //find wave data
      MMCKINFO parent;
      memset(&parent,0,sizeof(MMCKINFO));
      parent.fccType = mmioFOURCC('W','A','V','E');
      mmioDescend(wavefile,&parent,0,MMIO_FINDRIFF);

      //find fmt data
      MMCKINFO child;
      memset(&child,0,sizeof(MMCKINFO));
      child.fccType = mmioFOURCC('f','m','t',' ');
      mmioDescend(wavefile,&child,&parent,0);

      //read the format
      WAVEFORMATEX wavefmt;
      mmioRead(wavefile,(char*)&wavefmt,sizeof(wavefmt));
      if(wavefmt.wFormatTag != WAVE_FORMAT_PCM)
            return 2;

      //find the wave data chunk
      mmioAscend(wavefile,&child,0);
      child.ckid = mmioFOURCC('d','a','t','a');
      mmioDescend(wavefile,&child,&parent,MMIO_FINDCHUNK);

      //create a directsound buffer to hold wave data
      DSBUFFERDESC bufdesc;
      memset(&bufdesc,0,sizeof(DSBUFFERDESC));
      bufdesc.dwSize = sizeof(DSBUFFERDESC);
      bufdesc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
      bufdesc.dwBufferBytes = child.cksize;
      bufdesc.lpwfxFormat = &wavefmt;
	  if(DS_OK!=(lpDSO->CreateSoundBuffer(&bufdesc,&(*buffer),NULL))) return 1;

      //write wave data to directsound buffer you just created
      void *write1=0,*write2=0;
      unsigned long length1,length2;
      (*buffer)->Lock(0,child.cksize,&write1,&length1,&write2,&length2,0);
      if(write1>0)
            mmioRead(wavefile,(char*)write1,length1);
      if(write2>0)
            mmioRead(wavefile,(char*)write2,length2);
      (*buffer)->Unlock(write1,length1,write2,length2);

      //close the wavefile, don't need it anymore, it's in the directsound buffer now
      mmioClose(wavefile,0);

	  return DS_OK;
}

void PlaySounds()
{
	int i;
	DWORD stat;

	for(i=0;i<NUMEFFECTS;i++)
	{
		if(soundeffect[i].play)
		{
			if(soundeffect[i].buffer!=NULL)
			{
				soundeffect[i].buffer->GetStatus(&stat);
				if(stat==DSBSTATUS_PLAYING)
				{
					soundeffect[i].buffer->Stop();
					soundeffect[i].buffer->SetCurrentPosition(0);
				}
				soundeffect[i].buffer->SetVolume((LONG)soundeffect[i].volume);
				soundeffect[i].buffer->Play(0,0,soundeffect[i].loop?DSBPLAY_LOOPING:0);
				soundeffect[i].play = false;
				if(soundeffect[i].loop) soundeffect[i].looping = true;
				soundeffect[i].volume = -10000;
				soundeffect[i].distance = 0;
			}
		}
		if(soundeffect[i].stop && soundeffect[i].looping)
		{
			if(soundeffect[i].buffer!=NULL)
			{	soundeffect[i].buffer->GetStatus(&stat);
				if(CHECK(stat,DSBSTATUS_LOOPING))
				{	soundeffect[i].buffer->Stop();
					soundeffect[i].buffer->SetCurrentPosition(0);
				}
			}
			soundeffect[i].stop = false;
		}
		if(soundeffect[i].stop)
		{
			if(soundeffect[i].buffer!=NULL)
			{
				soundeffect[i].buffer->GetStatus(&stat);
				if(CHECK(stat,DSBSTATUS_PLAYING))
				{
					soundeffect[i].buffer->Stop();
					soundeffect[i].buffer->SetCurrentPosition(0);
				}
			}
			soundeffect[i].stop = false;
		}
	}

	for(i=0;i<numberofweapons;i++)
	{
		if(weaponlist[i].sound.play)
			if(weaponlist[i].sound.buffer!=NULL)
			{
				weaponlist[i].sound.buffer->GetStatus(&stat);
				if(stat==DSBSTATUS_PLAYING)
				{
					weaponlist[i].sound.buffer->Stop();
					weaponlist[i].sound.buffer->SetCurrentPosition(0);
				}
				weaponlist[i].sound.buffer->SetVolume((LONG)weaponlist[i].sound.volume);
				weaponlist[i].sound.buffer->Play(0,0,weaponlist[i].sound.loop?DSBPLAY_LOOPING:0);
				weaponlist[i].sound.play = false;
				weaponlist[i].sound.volume = -10000;
				weaponlist[i].sound.distance = 0;
			}
	}
	return;
}

/*
MISSILEEXPLODE, ENEMYEXPLODE, MINEEXPLODE, PLAYEREXPLODE, COLLECT,
USETELEPORT, *blocked*, LANDINBASE, *shieldhit*, TURRETROTATE, *levelcomplete*, MISSILEFIRE,
*missilelock*, *missileprox*, *shieldlow*, ENERGYLOW, IMPACT, BULLETIMPACT, MENUMOVE, MENUSELECT,
MENUREJECT, OPERATESWITCH, INSTALLITEM, INTROIMPACT, NUMEFFECTS */

void LoadSounds()
{
	LoadDirectSound(&soundeffect[MINEEXPLODE].buffer,"sounds\\explosion9.wav");
	LoadDirectSound(&soundeffect[ENEMYEXPLODE].buffer,"sounds\\explosion2.wav");
	LoadDirectSound(&soundeffect[MISSILEEXPLODE].buffer,"sounds\\explosion2.wav");
	LoadDirectSound(&soundeffect[PLAYEREXPLODE].buffer,"sounds\\explosion4.wav");
	LoadDirectSound(&soundeffect[MISSILEFIRE].buffer,"sounds\\missilelaunch.wav");
	LoadDirectSound(&soundeffect[MISSILELOCK].buffer,"sounds\\targetlocked.wav");
	LoadDirectSound(&soundeffect[NOMISSILES].buffer,"sounds\\nomissiles.wav");
//		soundeffect[MISSILELOCK].loop = true;
	LoadDirectSound(&soundeffect[USETELEPORT].buffer,"sounds\\teleport.wav");
	LoadDirectSound(&soundeffect[MENUMOVE].buffer,"sounds\\move.wav");
	LoadDirectSound(&soundeffect[MENUSELECT].buffer,"sounds\\select.wav");
	LoadDirectSound(&soundeffect[MENUREJECT].buffer,"sounds\\basereject.wav");
	LoadDirectSound(&soundeffect[LANDINBASE].buffer,"sounds\\enterbase.wav");
	LoadDirectSound(&soundeffect[COLLECT].buffer,"sounds\\collect.wav");
	LoadDirectSound(&soundeffect[BULLETIMPACT].buffer,"sounds\\hit.wav");
	LoadDirectSound(&soundeffect[OPERATESWITCH].buffer,"sounds\\switch.wav");
	LoadDirectSound(&soundeffect[INSTALLITEM].buffer,"sounds\\install.wav");
//	LoadDirectSound(&soundeffect[INTROIMPACT].buffer,"sounds\\intro.wav");
	LoadDirectSound(&soundeffect[IMPACT].buffer,"sounds\\impact.wav");
	LoadDirectSound(&soundeffect[SPAWNSOUND].buffer,"sounds\\spawn.wav");
//	LoadDirectSound(&soundeffect[TURRETROTATE].buffer,"sounds\\turretmove.wav");
//		soundeffect[TURRETROTATE].loop = true;
	LoadDirectSound(&soundeffect[ENERGYLOW].buffer,"sounds\\alarm.wav");
		soundeffect[ENERGYLOW].loop = true;
	LoadDirectSound(&soundeffect[CHARGEUP].buffer,"sounds\\chargeup.wav");
	LoadDirectSound(&soundeffect[CRACKLE].buffer,"sounds\\crackle.wav");
		soundeffect[CRACKLE].loop = true;
	LoadDirectSound(&soundeffect[SP_SYSTEMS_ACTIVATED].buffer,"sounds\\speech_systems_activated.wav");
	LoadDirectSound(&soundeffect[SP_BASE_SYSTEMS_ACTIVATED].buffer,"sounds\\speech_base_systems_activated.wav");
	LoadDirectSound(&soundeffect[SP_SECTOR_CLEARED].buffer,"sounds\\speech_objectives.wav");
	LoadDirectSound(&soundeffect[SP_WARNING].buffer,"sounds\\speech_warning.wav");
	LoadDirectSound(&soundeffect[SP_ENERGY_LOW].buffer,"sounds\\speech_energy_low.wav");
	LoadDirectSound(&soundeffect[SP_ENERGY_ZERO].buffer,"sounds\\speech_energy_zero.wav");
	LoadDirectSound(&soundeffect[SP_SYSTEMS_DOWN].buffer,"sounds\\speech_systems_powered_down.wav");
	LoadDirectSound(&soundeffect[SP_EMERGENCY].buffer,"sounds\\speech_emergency.wav");
//	LoadDirectSound(&soundeffect[SP_SELECT_SHIP].buffer,"sounds\\speech_select_ship.wav");
//	LoadDirectSound(&soundeffect[SP_SELECT_EQUIPMENT].buffer,"sounds\\speech_select_equipment.wav");
//	LoadDirectSound(&soundeffect[SP_SELECT_WEAPON].buffer,"sounds\\speech_select_weapon.wav");
}


void SoundEffect::SetSoundEffect(float dist)
{
	//Only play it if it's louder than the currently playing instance, or if it's within range
	if( (play && dist<distance) || (!play && (dist>=0 && dist<=1000.0f)) )
	{
		play = true;
		distance = dist;
		volumefloat = (1.0f - distance*0.001f)*0.7f*options.soundvolume;
		if(volumefloat<0) volume = -10000;
		else volume = soundfloattoint(volumefloat);
	}
}
