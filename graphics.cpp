/* Drawing functions for Omega Revolution*/

#include "OR.h"

extern DX9VARS dx9;
extern Scene scene;
extern Level level;
extern Player ship[2];
//extern LPDIRECT3DTEXTURE9 ;
extern ofstream dbout;
extern int g_screeny, g_screenx;
extern t_Tiledata *Tile;
extern int playmode, thistick;
extern int maxwidth, maxheight, maxratio, tilesdrawn;
extern GraphicalObject	error, energybar, energybarframe, debris, debris2, smoke, missile, flame, redbullet, explosion, 
						font, menufont, whiteblob, messagescreen, interfacescreen, datascreen, buttonframe, buttong[6], meterframe, 
						meterpanel, selectionbar, loading, mousefollowtarget, targetlaser, shield, numbers, shieldbar, redblob,
						shieldframe, textpower,	textspeed, textpowerused, textenergy, texttotal, basebg, projperilogo, bloodstonelogo,
						hitbar, hitbarframe, targetcross, dividingline, flare1, flare2, targetcrossblue, lightning1[2], lightning2[2], lightning3[2],
						line, beamsectionred, beamsectiongreen, beamsectionblue, menubg, missiledrone;
extern TileGraphics tilegfx;
extern list<GameObject> entity;
extern list<FadeObject> entityfade;
extern Base base[2];
extern Mousetarget mousetarget;
//extern bool framecounter, aa;
extern Options options;
extern float frametime, framespersec, screenratio;
extern SoundEffect soundeffect[NUMEFFECTS];

float g_LightH = sqrt(0.5f), g_LightV = sqrt(0.5f);



//----------------------------------------------------------------------------
// BltSprite() : A replacement of the DirectDraw Blt() function for Direct3D9
// If pDestRect is NULL, copies it to the same co-ord as src
// If pSrcRect is NULL, copies the entire texture
//----------------------------------------------------------------------------
HRESULT BlitSimple( RECT *pDestRect, LPDIRECT3DTEXTURE9 pSrcTexture, RECT *pSrcRect, DWORD dwFlags )
{
	D3DXVECTOR3 scaling(1,1,1), rcenter(0,0,0), trans(0,0,1);

	// Set the parameters - translation (screen location)
	if (pDestRect)
	{
		trans.x = (float) pDestRect->left;
		trans.y = (float) pDestRect->top;
	}
	/*
	// Scaling - If Source & Destination are different size rects, then scale
	if (pDestRect && pSrcRect)
	{
		scaling.x = ((float) (pDestRect->right - pDestRect->left)) / ((float) (pSrcRect->right - pSrcRect->left));
		scaling.y = ((float) (pDestRect->bottom - pDestRect->top)) / ((float) (pSrcRect->bottom - pSrcRect->top));
	}*/
	/*
	// Handle the flags - need to change scaling & adjust translation
	if (pSrcRect && dwFlags)
	{
		if (dwFlags&DDBLTFX_MIRRORLEFTRIGHT)
		{ 
			scaling.x = -scaling.x; 
			trans.x += (float) (pDestRect->right - pDestRect->left); 
		}
		if (dwFlags&DDBLTFX_MIRRORUPDOWN)
		{ 
			scaling.y = -scaling.y; 
			trans.y += (float) (pDestRect->bottom - pDestRect->top); 
		}
	}*/

	// And here we go:
	//return dx9.sprite->Draw( pSrcTexture, pSrcRect, &scaling, &rcenter, 0, &trans, 0xFFFFFFFF );
	//return dx9.sprite->Draw( pSrcTexture, pSrcRect, &rcenter, &trans,  0xFFFFFFFF ); 
	return dx9.sprite->Draw( pSrcTexture, NULL, &rcenter, &trans,  0xffffffff ); 
}

HRESULT BlitSimple( RECT *pDestRect, LPDIRECT3DTEXTURE9 pSrcTexture, RECT *pSrcRect, float angle, D3DCOLOR colour, DWORD dwFlags )
{
	D3DXVECTOR2 scaling(1,1), rcenter(0,0), trans(0,0);

	// Set the parameters - translation (screen location)
	if (pDestRect)
	{
		trans.x = (float) pDestRect->left;
		trans.y = (float) pDestRect->top;
	}
/*
	// Scaling - If Source & Destination are different size rects, then scale
	if (pDestRect && pSrcRect)
	{
		scaling.x = ((float) (pDestRect->right - pDestRect->left)) / ((float) (pSrcRect->right - pSrcRect->left));
		scaling.y = ((float) (pDestRect->bottom - pDestRect->top)) / ((float) (pSrcRect->bottom - pSrcRect->top));
	}
*/
	// Texture being used is 64 by 64:
	D3DXVECTOR2 spriteCentre = D3DXVECTOR2(36.5f,44.0f);

	// Build our matrix to rotate, scale and position our sprite
	D3DXMATRIX mat;

	// out, scaling centre, scaling rotation, scaling, rotation centre, rotation, translation
	D3DXMatrixTransformation2D(&mat,NULL,0.0,&scaling,&spriteCentre,angle,&trans);

	// Tell the sprite about the matrix
	dx9.sprite->SetTransform(&mat);

	// And here we go:
	return dx9.sprite->Draw( pSrcTexture, pSrcRect, NULL, NULL, colour); 
}

/*
//----------------------------------------------------------------------------
// BltSpriteEx() : A replacement of the DirectDraw Blt() function for Direct3D8
// Supports DDBLTFX_MIRRORLEFTRIGHT and DDBLTFX_MIRRORUPDOWN,
// modulate is color/alpha modifier
// rotation is value of rotation in RADIANS, counter-clockwise
// prcenter is the rotation centre point of the obj, or NULL
// If pDestRect is NULL, copies it to the same co-ord as src
// If pSrcRect is NULL, copies the entire texture
//----------------------------------------------------------------------------
HRESULT BltSpriteEx( RECT *pDestRect, LPDIRECT3DTEXTURE9 pSrcTexture, RECT *pSrcRect, DWORD dwFlags, D3DCOLOR modulate = 0xFFFFFFFF, float rotation = 0, POINT *prcenter = NULL )
{
     D3DXVECTOR2 scaling(1,1), rcenter(0,0), trans(0,0); 

     // Set the parameters - translation (screen location)
     if (pDestRect)
	 {
          trans.x = (float) pDestRect->left;
          trans.y = (float) pDestRect->top;
     } 
     // Rotation Center
     if (prcenter)
	 {
          rcenter.x = (float) prcenter->x;
          rcenter.y = (float) prcenter->y;
     }
	 else if (pSrcRect)
	 {
          // Set Rotation Center to be object center if none provided
          rcenter.x = (float) (pSrcRect->right - pSrcRect->left) / 2;
          rcenter.y = (float) (pSrcRect->bottom - pSrcRect->top) / 2;
     }
     // Scaling - If Source & Destination are different size rects, then scale
     if (pDestRect && pSrcRect) {
          scaling.x = ((float) (pDestRect->right - pDestRect->left)) / ((float) (pSrcRect->right - pSrcRect->left));
          scaling.y = ((float) (pDestRect->bottom - pDestRect->top)) / ((float) (pSrcRect->bottom - pSrcRect->top));
     }

     // And here we go:
     return dx9.pd3dxSprite->Draw( pSrcTexture, pSrcRect, &scaling, &rcenter, rotation, &trans, modulate );

}
*/

void FrameAlphaInterpolateShip(int frames, int alpha, float angle, int *alphatwo, int *frame, int *nextframe)
{
	float framefloat = (float)frames * angle * ONEOVERTWOPI;
	*frame = (int)framefloat;

	if(*frame >= frames) *frame = 0;

	framefloat = framefloat - (float)(*frame);		//Interpolation factor
	float newalpha = (float)((alpha>>24)&0xff);
	*alphatwo = ((lrintf(newalpha*framefloat)<<24)&0xff000000) | (alpha&0xffffff);		//New blending alpha

	*nextframe = *frame + 1;
	if(*nextframe == frames) *nextframe = 0;

	return;
}

void FrameAlphaInterpolate(int frames, int alpha, float angle, int *alphatwo, unsigned int *frame, int *nextframe)
{
	float framefloat = (float)frames * (TWOPI - angle) * ONEOVERTWOPI;
	*frame = (int)framefloat;

	if(*frame >= frames) *frame = 0;

	framefloat = framefloat - (float)(*frame);		//Interpolation factor
	float newalpha = (float)((alpha>>24)&0xff);
	*alphatwo = ((lrintf(newalpha*framefloat)<<24)&0xff000000) | (alpha&0xffffff);		//New blending alpha

	*nextframe = *frame + 1;
	if(*nextframe == frames) *nextframe = 0;

	return;
}


int DrawGameObject(GameObject *object, int player)
{
	if(object->graphics==NULL) return 1;
	if(CHECK(object->state,LINKEDFROMBOUNDARY) && !CHECK(level.boundary[object->link].state,ON)) return 0;

	//When we find the 'player' object we draw all the fade objects, then the player
	if(object->type==PLAYER)
	{
		list<FadeObject>::iterator j;
		for(j = entityfade.begin();j!=entityfade.end();++j)
			DrawFadeGameObject(&*j,player);

		DrawShip(&ship[player],player);
	}
	else if(object->onscreen[player])
	{
		if(object->graphics->bumpmapped)	//This interpolates between frames to reduce memory requirements and make it look nicer
		//if(object->graphics->frames>1)
		{
			int alphaone, nextframe;
			FrameAlphaInterpolate(object->graphics->frames, object->alpha, object->angle, &alphaone, &object->frame, &nextframe);
			DrawGraphicalObject(object->graphics,object->scrposition[player].x,object->scrposition[player].y,object->frame,object->angle+ship[player].angle,ship[player].screenscale*object->scale,object->scalecentre,object->alpha);
			DrawGraphicalObject(object->graphics,object->scrposition[player].x,object->scrposition[player].y,nextframe,object->angle+ship[player].angle,ship[player].screenscale*object->scale,object->scalecentre,alphaone);
		}
		else DrawGraphicalObject(object->graphics,object->scrposition[player].x,object->scrposition[player].y,object->frame,object->angle+ship[player].angle,ship[player].screenscale*object->scale,object->scalecentre,object->alpha);
//		if((object->radarcolour&0xff000000)==0xfe000000) DrawHitBars(object, player);
/*		if(object->type==ROCK || object->type==ENEMY)
		{
			for(int i=0;i<object->predictedoutline.points;i++)
			{
				t_point screen;
				if(PosToScreen(&object->predictedoutline.vertex[i],&screen,0)) 
					DrawGraphicalObject(&redblob,screen.x,screen.y,0,0,0xffffffff);
			}
		}*/
	}

	if(object->type==EXITGATE)
	{
		for(int j=0;j<object->childobjects;j++)
			if( CHECK(object->state,LOCKED)?object->childobject[j].graphics->bumpmapped:true)
				DrawGameObject(&object->childobject[j], player);
	}
	else if( (object->type==TELEPORT?(!CHECK(object->state,LOCKED)):true) && (object->type==MINE?CHECK(object->state,ARMED):true) && (object->type==GAMESAVER?!CHECK(object->state,LOCKED):true))
	{
		for(int j=0;j<object->childobjects;j++)
			DrawGameObject(&object->childobject[j], player);
	}

	return 0;
}

int DrawFadeGameObject(FadeObject *object, int player)
{
	if(object->graphics->bumpmapped)
	{
		object->frame = lrintf((float)object->graphics->frames * (TWOPI - object->angle)*0.159155f);
		if(object->frame >= object->graphics->frames) object->frame = 0;
	}

	if(object->onscreen[player]) DrawGraphicalObject(object->graphics,object->scrposition[player].x,object->scrposition[player].y,object->frame,object->angle+ship[player].angle,ship[player].screenscale*object->scale,object->scalecentre,object->alpha);
	return 0;
}

//----------------------------------------------------------------------------
// LoadTexture() : Load a texture from a file, returns ptr to texture.
//                   D3DX8 supports BMP, PPM, DDS, JPG, PNG, TGA & DIB. 
//----------------------------------------------------------------------------
LPDIRECT3DTEXTURE9 LoadTexture(char *filename, D3DXIMAGE_INFO *SrcInfo)
{
     LPDIRECT3DTEXTURE9 pd3dTexture;

     // Use 0x00000000 for no 'color key' substitution
     D3DCOLOR colorkey = 0x00000000;

     // Load image from file - maintain size of original file.
     // It is also possible to specify a new height/width and ask D3DX to filter
     // the image to the new size (stretch/shrink). See SDK docs for details.
     if(FAILED(D3DXCreateTextureFromFileEx( dx9.pd3dDevice, filename, 0, 0, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, colorkey, SrcInfo , NULL, &pd3dTexture))) return NULL;

     // Remember, textures sizes may be larger or smaller than your image, 
     // to allow for legal texture sizes & maximums on user's hardware
     return pd3dTexture;
}


//----------------------------------------------------------------------------
// Render() : Prepares for drawing the scene
//----------------------------------------------------------------------------
void Render()
{
     if( dx9.pd3dDevice == NULL)
	 {
		 dbout<<"D3D device NULL!"<<endl;
         return;
	 }

	 scene.SceneBegin(&dx9.MainViewPort);
     // Rendering of scene objects
     DrawScene();
	 scene.SceneFlip();

}

HRESULT DrawDividingLine()
{
	long int i, Screen_Offset;
	DWORD *lpscreen, pitch;
	D3DLOCKED_RECT locked;

	if(SUCCEEDED(dividingline.texture->LockRect(0,&locked,NULL,NULL)))		//Lock the background texture for writing
	{
		pitch = (DWORD)(locked.Pitch);
		lpscreen = (DWORD *)locked.pBits;

		Screen_Offset = 0;
		for(i=0;i<g_screeny;i++)	//X pixel direction of tile
		{	//Write pixel
			WritePixel(lpscreen,Screen_Offset,0xffffffff);
			WritePixel(lpscreen,Screen_Offset+4,0xffffffff);
			Screen_Offset += pitch;
		}
		dividingline.texture->UnlockRect(0);
	}

	return 0;
}

bool NewPreCache(GraphicalObject *object)
{
	int x, y, rotation, pixel;
	int HorGrad, VerGrad;
	int Left,Right,Up,Down,Val;
	int width, height;
	int framex, framey;
	float average,diff,main_angle;
	float HG, VG, TL, Red, Green, Blue, Alpha;
	float LH, LV;
	DWORD pitchdest, *lpscreendest, *lpstartdest, destoffset = 0;
	DWORD pitchcol, *lpscreencol, *lpstartcol, coloffset = 0;
	DWORD pitchbump, *lpscreenbump, *lpstartbump, bumpoffset = 0;
	DWORD *lptexturestart, *lpframestart, *lpframerowstart, framerowoffset = 0;
	D3DLOCKED_RECT locked;

	width = object->info.Width;
	height = object->info.Height * object->frames;
	object->xframes = 1;
	object->yframes = object->frames;

	if(height>maxheight || width>maxwidth)
	{
		//dbout<<"Width "<<width<<" height "<<height<<endl;
		FindOptimumRatio(object->info.Width, object->info.Height, &object->frames, &object->xframes, &object->yframes);
		width = object->info.Width * object->xframes;
		height = object->info.Height * object->yframes;
		//dbout<<"Resized to "<<object->xframes<<" * "<<object->yframes<<" = "<<object->frames<<" frames, width "<<width<<", height "<<height<<endl;
	}

	//This was D3DPOOL_DEFAULT
	//if(FAILED(D3DXCreateTexture(dx9.pd3dDevice, width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &object->texture )))
	if(FAILED(D3DXCreateTexture(dx9.pd3dDevice, width, height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &object->texture )))
		return false;

	if(FAILED(object->colourtexture->LockRect(0,&locked,NULL,D3DLOCK_READONLY)))		//Lock the colour texture for reading
		return false;

	pitchcol = (DWORD)locked.Pitch/4;
	lpscreencol = (DWORD *)locked.pBits;
	lpstartcol = lpscreencol;

	if(FAILED(object->bumptexture->LockRect(0,&locked,NULL,NULL)))		//Lock the bump texture for reading
	{
		object->colourtexture->UnlockRect(0);
		return false;
	}
	pitchbump = (DWORD)locked.Pitch/4;
	lpscreenbump = (DWORD *)locked.pBits;
	lpstartbump = lpscreenbump;
	//dbout<<pitchbump<<endl;

	if(FAILED(object->texture->LockRect(0,&locked,NULL,NULL)))		//Lock the object texture for writing
	{
		object->bumptexture->UnlockRect(0);
		object->colourtexture->UnlockRect(0);
		dbout<<"Failed locking texture for writing"<<endl;
		return false;
	}
	pitchdest = (DWORD)locked.Pitch/4;
	lpscreendest = (DWORD *)locked.pBits;
	lpstartdest = lpscreendest;
	lptexturestart = lpscreendest;

	//We must first convert the bumpmap format into one we can use
	for(y=0;y<object->info.Height;y++)
	{
		bumpoffset = 0;
		for(x=0;x<object->info.Width;x++)
		{
			ReadPixel(lpscreenbump,bumpoffset,pixel);
			pixel = pixel&0xff;
			if(pixel<100) pixel = 0;
			else
			{
				pixel = (pixel - pixel%5);		//Ensure it's a multiple of 5
				pixel = ((unsigned int)pixel/5) - 20;	//Convert to the range 0-31
			}
			WritePixel(lpscreenbump,bumpoffset,D3DCOLOR_ARGB(0,0,0,pixel));
			bumpoffset+=4;
		}
		lpscreenbump += pitchbump;
	}
	lpscreenbump = lpstartbump;
	//End of conversion

	//dbout<<"Precaching "<<object->frames<<" frames..."<<endl;
	//Loop over each frame
	for(rotation=0;rotation<(object->frames);rotation++)
	{
		//Work out x and y position
		framex = rotation%object->xframes;
		framey = rotation/object->xframes;
		//dbout<<rotation<<" fx "<<framex<<" fy "<<framey<<endl;
		lpframestart = lptexturestart + framex*object->info.Width + object->info.Height*framey*pitchdest;
		lpframerowstart = lpframestart;

		main_angle = (float)rotation*TWOPI/(float)object->frames;

		LH = g_LightH*cos(-main_angle-PIBYFOUR+0.3);		//Experimental
		LV = g_LightH*sin(-main_angle-PIBYFOUR+0.3);

		for(y=0;y<object->info.Height;y++)
		{
			destoffset = 0;
			coloffset = 0;
			bumpoffset = 0;
			framerowoffset = 0;
			for(x=0;x<object->info.Width;x++)
			{
				ReadPixel(lpscreencol,coloffset,pixel);
				if(pixel&0xff000000)	//Ignore empty pixels
				{
					Red = ((pixel&0xff0000)>>16)/255.0f;
					Green = ((pixel&0xff00)>>8)/255.0f;
					Blue = (pixel&0xff)/255.0f;
					Alpha = ((pixel&0xff000000)>>24)/255.0f;

					ReadPixel(lpscreenbump,bumpoffset,Val);		//Read current bump value
					Val = Val&0xff;		//Mask off the last 8 bits to get 0-255

					//Get pixel values of neighbouring pixels
					if(x==0) Left = 0;
					else ReadPixel(lpscreenbump,bumpoffset-4,Left);

					if(x==object->info.Width-1) Right = 0;
					else ReadPixel(lpscreenbump,bumpoffset+4,Right);

					if(y==object->info.Height-1) Down = 0;
					else ReadPixel(lpscreenbump,bumpoffset+pitchbump*4,Down);

					if(y==0) Up = 0;
					else ReadPixel(lpscreenbump,bumpoffset-pitchbump*4,Up);

					Up = Up&0xff;				//Mask off the last 8 bits of adjacent pixels to get 0-255
					Down = Down&0xff;			//Mask off the last 8 bits of adjacent pixels
					Left = Left&0xff;			//Mask off the last 8 bits of adjacent pixels
					Right = Right&0xff;			//Mask off the last 8 bits of adjacent pixels

					HorGrad = (Right - Left)*3;		//The multipliers control the slope of all the gradients
					VerGrad = (Up - Down)*3;			//and therefore the contrast
					average = (float)(Left + Right + Up + Down)*0.25f;	//This multiplier controls the overall brightness of the objects - higher number for darker objects

					diff = ((float)Val - average)/object->flatness;

					HG = LH*HorGrad/object->flatness;
					VG = LV*VerGrad/object->flatness;

					TL = (HG + VG + diff + 1.9f)/2.0f;

					Red = Red*TL;
					Green = Green*TL;
					Blue = Blue*TL;

					if(Red<0) Red=0;
					else if(Red>1.0f) Red=1.0f;

					if(Green<0) Green=0;
					else if(Green>1.0f) Green=1.0f;

					if(Blue<0) Blue=0;
					else if(Blue>1.0f) Blue=1.0f;

					//WritePixel(lpscreendest,destoffset,D3DCOLOR_COLORVALUE(Red,Green,Blue,Alpha));
					WritePixel(lpframerowstart,framerowoffset,D3DCOLOR_COLORVALUE(Red,Green,Blue,Alpha));
				}
				else WritePixel(lpframerowstart,framerowoffset,0);

				coloffset+=4;
				bumpoffset+=4;
				destoffset+=4;
				framerowoffset+=4;
			}//x
			lpscreendest += pitchdest;		//Increment screen pointer by one row
			lpscreencol += pitchcol;		//Increment colour pointer by one row
			lpscreenbump += pitchbump;		//Increment bump map pointer by one row
			lpframerowstart += pitchdest;	//Increment frame row
		}//y
		lpscreencol = lpstartcol;			//Reset colour pointer to beginning
		lpscreenbump = lpstartbump;			//Reset bump pointer to beginning
	}//rotation

	object->texture->UnlockRect(0);
	object->colourtexture->UnlockRect(0);
	object->bumptexture->UnlockRect(0);

	//dbout<<"Finished precaching"<<endl;
	return true;
}

bool PreCache(GraphicalObject *object)
{
	int x, y, rotation, pixel;
	int HorGrad, VerGrad;
	int Left,Right,Up,Down,Val;
	int width, height;
	int framex, framey;
	float average,diff,main_angle;
	float HG, VG, TL, Red, Green, Blue, Alpha;
	float LH, LV;
	DWORD pitchdest, *lpscreendest, *lpstartdest, destoffset = 0;
	DWORD pitchcol, *lpscreencol, *lpstartcol, coloffset = 0;
	DWORD pitchbump, *lpscreenbump, *lpstartbump, bumpoffset = 0;
	DWORD *lptexturestart, *lpframestart, *lpframerowstart, framerowoffset = 0;
	D3DLOCKED_RECT locked;

	width = object->info.Width;
	height = object->info.Height * object->frames;
	object->xframes = 1;
	object->yframes = object->frames;

	if(height>maxheight || width>maxwidth)
	{
		//dbout<<"Width "<<width<<" height "<<height<<endl;
		FindOptimumRatio(object->info.Width, object->info.Height, &object->frames, &object->xframes, &object->yframes);
		width = object->info.Width * object->xframes;
		height = object->info.Height * object->yframes;
		//dbout<<"Resized to "<<object->xframes<<" * "<<object->yframes<<" = "<<object->frames<<" frames, width "<<width<<", height "<<height<<endl;
	}

	//if(FAILED(D3DXCreateTexture(dx9.pd3dDevice, width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &object->texture)))
	if(FAILED(D3DXCreateTexture(dx9.pd3dDevice, width, height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &object->texture)))
		return false;

	if(FAILED(object->colourtexture->LockRect(0,&locked,NULL,D3DLOCK_READONLY)))		//Lock the colour texture for reading
		return false;

	pitchcol = (DWORD)locked.Pitch/4;
	lpscreencol = (DWORD *)locked.pBits;
	lpstartcol = lpscreencol;

	if(FAILED(object->bumptexture->LockRect(0,&locked,NULL,NULL)))		//Lock the bump texture for reading
	{
		object->colourtexture->UnlockRect(0);
		return false;
	}
	pitchbump = (DWORD)locked.Pitch/4;
	lpscreenbump = (DWORD *)locked.pBits;
	lpstartbump = lpscreenbump;
	//dbout<<pitchbump<<endl;

	if(FAILED(object->texture->LockRect(0,&locked,NULL,NULL)))		//Lock the object texture for writing
	{
		object->bumptexture->UnlockRect(0);
		object->colourtexture->UnlockRect(0);
		dbout<<"Failed locking texture for writing"<<endl;
		return false;
	}
	pitchdest = (DWORD)locked.Pitch/4;
	lpscreendest = (DWORD *)locked.pBits;
	lpstartdest = lpscreendest;
	lptexturestart = lpscreendest;

	//We must first convert the bumpmap format into one we can use
	for(y=0;y<object->info.Height;y++)
	{
		bumpoffset = 0;
		for(x=0;x<object->info.Width;x++)
		{
			ReadPixel(lpscreenbump,bumpoffset,pixel);
			pixel = pixel&0xff;
			if(pixel<100) pixel = 0;
			else
			{
				pixel = (pixel - pixel%5);		//Ensure it's a multiple of 5
				pixel = ((unsigned int)pixel/5) - 20;	//Convert to the range 0-31
			}
			WritePixel(lpscreenbump,bumpoffset,D3DCOLOR_ARGB(0,0,0,pixel));
			bumpoffset+=4;
		}
		lpscreenbump += pitchbump;
	}
	lpscreenbump = lpstartbump;
	//End of conversion

	//dbout<<"Precaching "<<object->frames<<" frames..."<<endl;
	//Loop over each frame
	for(rotation=0;rotation<(object->frames);rotation++)
	{
		//Work out x and y position
		framex = rotation%object->xframes;
		framey = rotation/object->xframes;
		//dbout<<rotation<<" fx "<<framex<<" fy "<<framey<<endl;
		lpframestart = lptexturestart + framex*object->info.Width + object->info.Height*framey*pitchdest;
		lpframerowstart = lpframestart;

		main_angle = (float)rotation*TWOPI/(float)object->frames;

		LH = g_LightH*cos(-main_angle-PIBYFOUR);		//Experimental
		LV = g_LightH*sin(-main_angle-PIBYFOUR);

		for(y=0;y<object->info.Height;y++)
		{
			destoffset = 0;
			coloffset = 0;
			bumpoffset = 0;
			framerowoffset = 0;
			for(x=0;x<object->info.Width;x++)
			{
				ReadPixel(lpscreencol,coloffset,pixel);
				if(pixel&0xff000000)	//Ignore empty pixels
				{
					Red = ((pixel&0xff0000)>>16)/255.0f;
					Green = ((pixel&0xff00)>>8)/255.0f;
					Blue = (pixel&0xff)/255.0f;
					Alpha = ((pixel&0xff000000)>>24)/255.0f;

					ReadPixel(lpscreenbump,bumpoffset,Val);	//Read current bump value
					Val = Val&0xff;

					if(x==0) Left = 0;
					else ReadPixel(lpscreenbump,bumpoffset-4,Left);

					if(x==object->info.Width-1) Right = 0;
					else ReadPixel(lpscreenbump,bumpoffset+4,Right);

					if(y==object->info.Height-1) Down = 0;
					else ReadPixel(lpscreenbump,bumpoffset+pitchbump*4,Down);

					if(y==0) Up = 0;
					else ReadPixel(lpscreenbump,bumpoffset-pitchbump*4,Up);

					Up = Up&0xff;
					Down = Down&0xff;
					Left = Left&0xff;
					Right = Right&0xff;

					HorGrad = (Right - Left)*2;
					VerGrad = (Up - Down)*2;
					average = (float)(Left + Right + Up + Down)*0.25f;

					diff = ((float)Val - average)/object->flatness;

					HG = LH*HorGrad/object->flatness;
					VG = LV*VerGrad/object->flatness;

					TL = (HG + VG + diff + 1.9f)/2.0f;

					Red = Red*TL;
					Green = Green*TL;
					Blue = Blue*TL;

					if(Red<0) Red=0;
					else if(Red>1.0f) Red=1.0f;

					if(Green<0) Green=0;
					else if(Green>1.0f) Green=1.0f;

					if(Blue<0) Blue=0;
					else if(Blue>1.0f) Blue=1.0f;

					//WritePixel(lpscreendest,destoffset,D3DCOLOR_COLORVALUE(Red,Green,Blue,Alpha));
					WritePixel(lpframerowstart,framerowoffset,D3DCOLOR_COLORVALUE(Red,Green,Blue,Alpha));
				}
				else WritePixel(lpframerowstart,framerowoffset,0);

				coloffset+=4;
				bumpoffset+=4;
				destoffset+=4;
				framerowoffset+=4;
			}//x
			lpscreendest += pitchdest;		//Increment screen pointer by one row
			lpscreencol += pitchcol;		//Increment colour pointer by one row
			lpscreenbump += pitchbump;		//Increment bump map pointer by one row
			lpframerowstart += pitchdest;	//Increment frame row
		}//y
		lpscreencol = lpstartcol;			//Reset colour pointer to beginning
		lpscreenbump = lpstartbump;			//Reset bump pointer to beginning
	}//rotation

	object->texture->UnlockRect(0);
	object->colourtexture->UnlockRect(0);
	object->bumptexture->UnlockRect(0);

	//dbout<<"Finished precaching"<<endl;
	return true;
}


//Finds the best way to fit x * y frames into the hardware maximum texture size
bool FindOptimumRatio(int objwidth, int objheight, int *objframes, int *optwidth, int *optheight)
{
	int totalarea = *objframes*objwidth*objheight;		//Total requested area
	int maxarea = maxheight*maxwidth;					//Total available area
	int maxframesx = maxwidth/objwidth;					//Maximum frames in the x direction
	int maxframesy = maxheight/objheight;				//Maximum frames in the y direction
	int wastedsquares;									//Wasted squares for each x size 
	int xsize, squares;									//Number of frames in the x direction, number of frames used in total
	int minwasted = maxframesx;							//The number of squares wasted at the minimum
	int minwastedsizex = maxframesx;					//The xsize at which least squares are wasted
	int minwastedsizey;									//The ysize at which least squares are wasted
	int i;

	if(maxframesx<1 || maxframesy<1) return false;

	if(totalarea>maxarea) *objframes = maxframesx*maxframesy;		//The only possible solution is to reduce the number of frames

	for(xsize=maxframesx;xsize>0;xsize--)		//Run through every possible x size
	{
		//Find the other factor
		for(i=1;i<maxframesy;i++)
		{	squares = xsize*i;
			if( (squares) >= *objframes ) break;
		}
		wastedsquares = squares - *objframes;	//Number of squares that are not used in this grid

		if(wastedsquares==0)	//If it fits perfectly then we've found what we're looking for
		{
			*optwidth = xsize;
			*optheight = i;
			return true;
		}
		else if(wastedsquares<minwasted)		//If fewer are wasted than before then use this one
		{
			minwasted = wastedsquares;
			minwastedsizex = xsize;
			minwastedsizey = i;
		}
	}

	*optwidth = minwastedsizex;
	*optheight = minwastedsizey;
	*objframes = minwastedsizex*minwastedsizey;		//No point wasting the space - might as well use it for extra graphics data

	return true;
}

//Just a simple function to avoid having to rewrite parts of the code or duplicate functions
bool GraphicalObject::CreateGraphicalObject(const char *filename, int numframes)
{
	return GraphicalObject::CreateGraphicalObject(filename, numframes, 24.0);
}


//Creates (loads) a graphics file and bumpmaps it if necessary
bool GraphicalObject::CreateGraphicalObject(const char *filename, int numframes, float objectflatness)
{
	char filenm[100], bfilenm[100];
	D3DXIMAGE_INFO bumpinfo;

	//Release texture if it hasn't already been
	if(texture!=NULL) ClearGraphicalObject();

	sprintf_s(filenm,100,"graphics\\%s.png",filename);	//Set up our file names
	sprintf_s(bfilenm,100,"graphics\\%s_bump.png",filename);

	if( (bumptexture = LoadTexture(bfilenm,&bumpinfo)) == NULL) bumpmapped = false;		//Can't find file name, thus can't be bumpmapped
	else bumpmapped = true;		//Has a bump map with it

	colourtexture = LoadTexture(filenm,&info);
	if(colourtexture == NULL)
	{
		if(bumptexture!=NULL) bumptexture->Release();
		bumptexture = NULL;
		bumpmapped = false;
		dbout<<"Error in "<<filenm<<" or can't find file"<<endl;
		return false;
	}
	strcpy_s(name,100,filename);

	frames = numframes;
	if(bumpmapped)
	{
		if(bumpinfo.Width != info.Width)
		{
			if(bumptexture!=NULL) bumptexture->Release();
			bumptexture = NULL;
			bumpmapped = false;
			//dbout<<"Width mismatch"<<endl;
			return false;
		}
		if(bumpinfo.Height != info.Height)
		{
			if(bumptexture!=NULL) bumptexture->Release();
			bumptexture = NULL;
			bumpmapped = false;
			//dbout<<"Height mismatch"<<endl;
			return false;
		}

		//orientation = VERTICAL;
		flatness = objectflatness;
		framewidth = info.Width;
		frameheight = info.Height;

		//dbout<<"Precaching "<<filenm<<" ..."<<endl;
		if(!NewPreCache(this))
		{
			bumpmapped = false;
			//orientation = NONE;
			totalwidth = info.Width;
			totalheight = info.Height;
			xframes = 1;
			yframes = 1;
			return false;
		}

		totalwidth = info.Width;
		totalheight = info.Height*frames;

		bumptexture->Release();
		colourtexture->Release();
		bumptexture = NULL;
		colourtexture = NULL;

	}
	else
	{
		texture = colourtexture;
		colourtexture = NULL;
		if(frames>1)
		{
			framewidth = info.Width / frames;
			frameheight = info.Height;
			xframes = frames;
			yframes = 1;
		}
		else
		{
			orientation = NONE;
			framewidth = info.Width;
			frameheight = info.Height;
			xframes = 1;
			yframes = 1;
		}
		totalwidth = info.Width;
		totalheight = info.Height;
	}

	return true;
}

//Create a blank texture
bool GraphicalObject::CreateGraphicalObject(int x, int y)
{
	orientation = NONE;
	framewidth = x;
	frameheight = y;

	if(texture!=NULL) texture->Release();

	//if(FAILED(D3DXCreateTexture(dx9.pd3dDevice, x, y, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture)))
	if(FAILED(D3DXCreateTexture(dx9.pd3dDevice, x, y, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture)))
	{
		dbout<<"Texture creation failed"<<endl;
		texture = NULL;
		return false;
	}

	strcpy_s(name,100,"blank");

	info.Width = x;
	info.Height = y;
	info.Format = D3DFMT_A8R8G8B8;
	info.MipLevels = 1;

	return true;
}

HRESULT DrawGraphicalObject(GraphicalObject *object, int x, int y, int frame, float angle, D3DCOLOR colour)
{
	RECT dest, src;
	int halfwidth = object->framewidth>>1, halfheight = object->frameheight>>1;
	int xpos, ypos;

	if(object->texture == NULL) return 1;

	if(frame>=object->frames) frame = 0;

	dest.left = x - halfwidth;
	dest.top = y - halfheight;

	//Optimise this - use loop
	if(frame == 0)
	{
		xpos = 0;
		ypos = 0;
	}
	else
	{
		xpos = frame%object->xframes;
		ypos = frame/object->xframes;
	}

	src.top = object->frameheight * ypos;
	src.bottom = src.top + object->frameheight;
	src.left = object->framewidth * xpos;
	src.right = src.left + object->framewidth;
/*
	src.top = 0;
	src.bottom = object->frameheight;
	src.left = object->framewidth * frame;
	src.right = src.left + object->framewidth;

	src.top = 0;
	src.bottom = object->frameheight;
	src.left = 0;
	src.right = object->framewidth;*/



	D3DXVECTOR2 scaling(1,1), rcenter(0,0), trans(0,0);

	// Set the parameters - translation (screen location)
	trans.x = (float) dest.left;
	trans.y = (float) dest.top;

	//Set the centre of the object:
	D3DXVECTOR2 spriteCentre = D3DXVECTOR2((float)halfwidth,(float)halfheight);

	// Build our matrix to rotate, scale and position our sprite
	D3DXMATRIX mat;

	// out, scaling centre, scaling rotation, scaling, rotation centre, rotation, translation
	D3DXMatrixTransformation2D(&mat,NULL,0.0,&scaling,&spriteCentre,angle,&trans);

	// Tell the sprite about the matrix
	dx9.sprite->SetTransform(&mat);

	// And here we go:
	return dx9.sprite->Draw( object->texture, &src, NULL, NULL, colour); 
}

HRESULT DrawGraphicalObject(GraphicalObject *object, int x, int y, int frame, float angle, float scale, bool setscalecentre, D3DCOLOR colour)
{
	RECT dest, src;
	int halfwidth = object->framewidth>>1, halfheight = object->frameheight>>1;
	int xpos, ypos;

	if(object->texture == NULL) return 1;

	if(frame>=object->frames) frame = 0;

	dest.left = x - halfwidth;
	dest.top = y - halfheight;

	//Optimise this - use loop
	if(frame == 0)
	{
		xpos = 0;
		ypos = 0;
	}
	else
	{
		xpos = frame%object->xframes;
		ypos = frame/object->xframes;
	}

	src.top = object->frameheight * ypos;
	src.bottom = src.top + object->frameheight;
	src.left = object->framewidth * xpos;
	src.right = src.left + object->framewidth;
/*
	src.top = 0;
	src.bottom = object->frameheight;
	src.left = object->framewidth * frame;
	src.right = src.left + object->framewidth;

	src.top = 0;
	src.bottom = object->frameheight;
	src.left = 0;
	src.right = object->framewidth;*/



	//D3DXVECTOR2 scaling(scale,scale), rcenter(0,0), trans(0,0);
	D3DXVECTOR2 scaling(scale,scale), rcenter(0,0), trans(0,0), scenter((float)halfwidth,(float)halfheight);

	// Set the parameters - translation (screen location)
	trans.x = (float) dest.left;
	trans.y = (float) dest.top;

	//Set the rotational centre of the object:
	D3DXVECTOR2 spriteCentre = D3DXVECTOR2((float)halfwidth,(float)halfheight);

	// Build our matrix to rotate, scale and position our sprite
	D3DXMATRIX mat;

	//Perform matrix transformation
	D3DXMatrixTransformation2D(&mat,				//Matrix
		setscalecentre?&scenter:NULL,				//Scaling centre (can be NULL to avoid setting centre)
								0.0,				//Scaling rotation
								&scaling,			//Scaling amount x & y
								&spriteCentre,		//Rotation centre
								angle,				//Rotation
								&trans);			//Translation

	// Tell the sprite about the matrix
	dx9.sprite->SetTransform(&mat);

	// And here we go:
	return dx9.sprite->Draw( object->texture, &src, NULL, NULL, colour); 
}

//Used for energy bars etc.
HRESULT DrawGraphicalObject(GraphicalObject *object, int x, int y, RECT src, int frame, float angle, float scale, D3DCOLOR colour)
{
	RECT dest;
	int halfwidth = (object->framewidth>>1), halfheight = (object->frameheight>>1);

	//if(object->texture == NULL) return 1;

	if(frame>=object->frames) frame = 0;

	dest.left = x - halfwidth;
	dest.top = y - halfheight;

	D3DXVECTOR2 scaling(scale,scale), rcenter(0,0), trans(0,0);

	// Set the parameters - translation (screen location)
	trans.x = (float) dest.left;
	trans.y = (float) dest.top;

	//Set the centre of the object:
	D3DXVECTOR2 spriteCentre = D3DXVECTOR2((float)halfwidth,(float)halfheight);

	// Build our matrix to rotate, scale and position our sprite
	D3DXMATRIX mat;

	// out, scaling centre, scaling rotation, scaling, rotation centre, rotation, translation
	D3DXMatrixTransformation2D(&mat,&spriteCentre,0.0,&scaling,&spriteCentre,angle,&trans);

	// Tell the sprite about the matrix
	dx9.sprite->SetTransform(&mat);

	// And here we go:
	return dx9.sprite->Draw( object->texture, &src, NULL, NULL, colour); 
}

void DrawShip(Player *inship, int player)
{
	float droneframe;
	if(inship->state==PLAY)
	{
		if(inship->equipment[TARGET_LASER])
		{
			Line(&beamsectionblue,0,inship->scrposition.x,0,inship->scrposition.x,inship->scrposition.y,0xffffffff);
			if(inship->equipment[HOMING_COMPUTER] && inship->homingcomputerbusy) Line(&beamsectionred,0,inship->scrposition.x,inship->scrposition.y,inship->targetscr.x + (rand()&4),inship->targetscr.y + (rand()&4),0xffffffff);
		}

		droneframe = (int)(missiledrone.frames * inship->angle / TWOPI);
		//Draw missile drone
		if(inship->equipment[MINIMISSILES]) DrawGraphicalObject(&missiledrone, inship->scrposition.x+lrintf(40.0f*inship->screenscale*cos(inship->missiledroneangle+PI)), inship->scrposition.y, droneframe, 0, (0.95f+0.25f*sin(inship->missiledroneangle+PI))*inship->screenscale, true, inship->alpha);

		//	DrawGraphicalObject(&targetlaser,inship->scrposition.x,inship->scrposition.y-inship->screenscale*260.0f,0,0,inship->screenscale,true,0x80ffffff);
		if(inship->angle>=TWOPI) inship->angle -= TWOPI;
		if(inship->angle<0) inship->angle += TWOPI;
		inship->frame = (int)(inship->graphics.frames * inship->angle / TWOPI);
		if(inship->frame>inship->graphics.frames) inship->frame = 0;

		if(inship->invulnerable) inship->invframe = !inship->invframe;
		else inship->invframe = true;
		if(inship->invframe)
		{
			int alphatwo, nextframe;
			FrameAlphaInterpolateShip(inship->graphics.frames, inship->alpha, inship->angle, &alphatwo, &inship->frame, &nextframe);
			DrawGraphicalObject(&(inship->graphics), inship->scrposition.x, inship->scrposition.y, inship->frame, 0, inship->screenscale, true, inship->alpha);
			DrawGraphicalObject(&(inship->graphics), inship->scrposition.x, inship->scrposition.y, nextframe, 0, inship->screenscale, true, alphatwo);
		}

		//Draw missile drone
		if(inship->equipment[MINIMISSILES]) DrawGraphicalObject(&missiledrone, inship->scrposition.x+lrintf(40.0f*inship->screenscale*cos(inship->missiledroneangle)), inship->scrposition.y, droneframe, 0, (0.95f+0.25f*sin(inship->missiledroneangle))*inship->screenscale, true, inship->alpha);

		if(inship->equipment[SHIELD])
		{	inship->shieldbrightness = 0.85f*inship->shields/inship->maxshields;
			float shieldstate = inship->shieldbrightness - 0.25f*(float)rand()/(float)RAND_MAX;
			if(shieldstate<0) shieldstate = 0;
			else if(shieldstate>1.0f) shieldstate = 1.0f;
			int shieldalpha = 255.0f*shieldstate;
			shieldalpha = ((shieldalpha<<24)&0xff000000) + 0xffffff;
			DrawGraphicalObject(&shield, inship->scrposition.x, inship->scrposition.y, 0, 0, inship->screenscale, true, shieldalpha);
		}
	}

	if(playmode==TWOPLAYER)
	{
		t_point scrposition;
		if(player==0) if(ship[1].state == PLAY) if(PosToScreen(&(ship[1].position), &scrposition, 0))
		{
			droneframe = (int)(missiledrone.frames * ship[1].angle / TWOPI);
			//Draw missile drone
			if(ship[1].equipment[MINIMISSILES])
			{
				t_vector droneposition;
				droneposition.x = lrintf(40.0f*ship[1].screenscale*cos(ship[1].missiledroneangle+PI));
				droneposition = RotateVector(droneposition,ship[0].angle-ship[1].angle);
				DrawGraphicalObject(&missiledrone, scrposition.x+droneposition.x, scrposition.y+droneposition.y, droneframe, ship[0].angle-ship[1].angle, (0.95f+0.25f*sin(ship[1].missiledroneangle+PI))*ship[1].screenscale, true, ship[1].alpha);
			}
			//Draw ship 1 on ship 0's screen
			DrawGraphicalObject(&ship[1].graphics, scrposition.x, scrposition.y, ship[1].frame, ship[0].angle-ship[1].angle, screenratio, true, ship[1].alpha);
			//Draw missile drone
			if(ship[1].equipment[MINIMISSILES])
			{
				t_vector droneposition;
				droneposition.x = lrintf(40.0f*ship[1].screenscale*cos(ship[1].missiledroneangle));
				droneposition = RotateVector(droneposition,ship[0].angle-ship[1].angle);
				DrawGraphicalObject(&missiledrone, scrposition.x+droneposition.x, scrposition.y+droneposition.y, droneframe, ship[0].angle-ship[1].angle, (0.95f+0.25f*sin(ship[1].missiledroneangle))*ship[1].screenscale, true, ship[1].alpha);
			}
			if(ship[1].equipment[SHIELD])
			{
				float shieldstate = ship[1].shieldbrightness - 0.25f*(float)rand()/(float)RAND_MAX;
				if(shieldstate<0) shieldstate = 0;
				else if(shieldstate>1.0f) shieldstate = 1.0;
				int shieldalpha = 255.0f*shieldstate;
				shieldalpha = ((shieldalpha<<24)&0xff000000) + 0xffffff;
				DrawGraphicalObject(&shield, scrposition.x, scrposition.y, 0, ship[0].angle-ship[1].angle, screenratio, true, shieldalpha);
			}
		}
		if(player==1) if(ship[0].state == PLAY) if(PosToScreen(&ship[0].position, &scrposition, 1))
		{
			droneframe = (int)(missiledrone.frames * ship[0].angle / TWOPI);
			//Draw missile drone
			if(ship[0].equipment[MINIMISSILES])
			{
				t_vector droneposition;
				droneposition.x = lrintf(40.0f*ship[0].screenscale*cos(ship[0].missiledroneangle+PI));
				droneposition = RotateVector(droneposition,ship[1].angle-ship[0].angle);
				DrawGraphicalObject(&missiledrone, scrposition.x+droneposition.x, scrposition.y+droneposition.y, droneframe, ship[1].angle-ship[0].angle, (0.95f+0.25f*sin(ship[0].missiledroneangle+PI))*ship[0].screenscale, true, ship[0].alpha);
			}
			DrawGraphicalObject(&ship[0].graphics, scrposition.x, scrposition.y, ship[0].frame, ship[1].angle-ship[0].angle, screenratio, true, ship[0].alpha);
			if(ship[0].equipment[MINIMISSILES])
			{
				t_vector droneposition;
				droneposition.x = lrintf(40.0f*ship[0].screenscale*cos(ship[0].missiledroneangle));
				droneposition = RotateVector(droneposition,ship[1].angle-ship[0].angle);
				DrawGraphicalObject(&missiledrone, scrposition.x+droneposition.x, scrposition.y+droneposition.y, droneframe, ship[1].angle-ship[0].angle, (0.95f+0.25f*sin(ship[0].missiledroneangle))*ship[0].screenscale, true, ship[0].alpha);
			}
			if(ship[0].equipment[SHIELD])
			{
				float shieldstate = ship[0].shieldbrightness - 0.25f*(float)rand()/(float)RAND_MAX;
				if(shieldstate<0) shieldstate = 0;
				else if(shieldstate>1.0f) shieldstate = 1.0;
				int shieldalpha = 255.0f*shieldstate;
				shieldalpha = ((shieldalpha<<24)&0xff000000) + 0xffffff;
				DrawGraphicalObject(&shield, scrposition.x, scrposition.y, 0, ship[1].angle-ship[0].angle, screenratio, true,  shieldalpha);
			}
		}
	}

}

void DrawShipGlyphs(Player *inship, int player)
{
	static unsigned int alpha = 0xffffffff;
	if(inship->equipment[BOMB])
	{
		if(inship->bombtimer!=0xffffffff)
		{
			float ratio = (thistick-inship->bombtimer)/1500.0f;
			if(ratio>0.2f)
			{
				RECT src;
				src.left = 0;
				src.top = 0;
				src.bottom = hitbar.frameheight;
				src.right = (LONG)(hitbar.framewidth*ratio);
				DrawGraphicalObject(&hitbarframe, inship->scrposition.x, inship->scrposition.y+40, 0, 0, screenratio, true, 0xffffffff);
				DrawGraphicalObject(&hitbar, inship->scrposition.x, inship->scrposition.y+40, src, 0, 0, screenratio, 0xffffffff);
			}
		}
	}

	if(inship->equipment[SEEKER] && inship->seekertarget!=NULL) DrawGraphicalObject(&targetcrossblue, inship->seekertarget->scrposition[player].x, inship->seekertarget->scrposition[player].y, 0, 0, screenratio, true, alpha);
	alpha += 0x4000000;

	if(inship->homingcomputerbusy)
	{
		if(inship->missiles>0) DrawGraphicalObject(&targetcross, inship->targetscr.x, inship->targetscr.y, 0, 0, screenratio, true, alpha);
		int length = strlen(inship->targetname);
		length *= font.framewidth;
		length = lrintf((float)length*screenratio*0.5f);
		DrawString(&font,inship->targetname,inship->targetscr.x-length,inship->targetscr.y,inship->scrbottomright.x,alpha>0x80000000?GREENHIGHLIGHT:GREEN);

		int width;
		RECT src;

		width = lrintf((float)hitbar.totalwidth*inship->targetenergy);
		src.left = 0;
		src.top = 0;
		src.bottom = hitbar.totalheight;
		src.right = width;
		DrawGraphicalObject(&hitbarframe, inship->targetscr.x, inship->targetscr.y+screenratio*50.0f, 0, 0, screenratio, true, alpha);
		DrawGraphicalObject(&hitbar, inship->targetscr.x, inship->targetscr.y+screenratio*50.0f, src, 0, 0, screenratio, alpha);

		if(alpha==0xffffffff) soundeffect[MISSILELOCK].SetSoundEffect(0);
	}
}


bool DrawObjects(int player)
{
	t_vector position;
	t_point scrposition;

	list<GameObject>::iterator i = entity.begin();
	for(++i;i!=entity.end();++i)
		DrawGameObject(&*i,player);


	return true;
}

bool DrawRadar(int player)
{
	//long pixel=0xFFFFFF;
	list<GameObject>::iterator j;
	int i, bordercolour;
	t_point position;
	t_vector temp;
	DWORD pitch, *lpdata, offset;
	D3DLOCKED_RECT locked;

	if(FAILED(ship[player].radar.texture->LockRect(0,&locked,NULL,NULL)))
	{
		dbout<<"Locking radar surface failed"<<endl;
		return false;
	}

	pitch = locked.Pitch/4;
	lpdata = (DWORD *)locked.pBits;

	//Write background colour
	if(player==0) for(i=pitch;i<(pitch-1)*RADARHEIGHT;i++) WritePixel(lpdata,i*4,0x80004000);
	if(player==1) for(i=pitch;i<(pitch-1)*RADARHEIGHT;i++) WritePixel(lpdata,i*4,0x80000040);

	bool enemyinradar = false;
	bool enemiesexist = false;

	j=entity.begin();
	for(++j;j!=entity.end();++j)
	{
		if(j->radarcolour==0) continue;
		if(j->type<ENEMY) break;		//Should save some cycles once the bullets start flying
		if(j->radarcolour==0xffff0000 || j->radarcolour==0xffff2020 || j->objective) enemiesexist = true;

		position = GetRadarPosition(j->position, player);

		if(position.x>1 && position.x<RADARWIDTH-2)
			if(position.y>1 && position.y<RADARHEIGHT-2)
			{
				offset = (position.x + position.y*pitch)*4;
				WritePixel(lpdata,offset,j->radarcolour);
				int halfcolour = (j->radarcolour&0xffffff) | ((j->radarcolour>>1)&0x7f000000);
				if(j->radarcolour==0xffff0000 || j->radarcolour==0xffff2020 || j->objective)
				{
					if(position.x>3 && position.x<RADARWIDTH-4 && position.y>3 && position.y<RADARHEIGHT-4) enemyinradar = true;
					WritePixel(lpdata,offset+4,j->radarcolour);
					WritePixel(lpdata,offset-4,j->radarcolour);
					WritePixel(lpdata,offset-4*pitch,j->radarcolour);
					WritePixel(lpdata,offset+4*pitch,j->radarcolour);

					WritePixel(lpdata,offset+8,halfcolour);
					WritePixel(lpdata,offset-8,halfcolour);
					WritePixel(lpdata,offset-8*pitch,halfcolour);
					WritePixel(lpdata,offset+8*pitch,halfcolour);
				}
				else
				{
					WritePixel(lpdata,offset+4,halfcolour);
					WritePixel(lpdata,offset-4,halfcolour);
					WritePixel(lpdata,offset-4*pitch,halfcolour);
					WritePixel(lpdata,offset+4*pitch,halfcolour);
				}
			}
	}

	//Offset value of pixel for ship location
	offset = ((RADARWIDTH>>1) + (RADARHEIGHT>>1)*pitch)*4;

	WritePixel(lpdata,offset,0xffffffff);
	WritePixel(lpdata,offset+4,0xb0ccffcc);
	WritePixel(lpdata,offset-4,0xb0cccccc);
	WritePixel(lpdata,offset-4*pitch,0xb0ccffcc);
	WritePixel(lpdata,offset+4*pitch,0xb0ccffcc);

	if(playmode==TWOPLAYER)
	{	//Work out the position on the radar of the other player
		position = GetRadarPosition(ship[(!player)&0x1].position, player);
		if(position.x>1 && position.x<RADARWIDTH-2)
			if(position.y>1 && position.y<RADARHEIGHT-2)
			{
				offset = (position.x + position.y*pitch)*4;

				WritePixel(lpdata,offset,0xffffffff);
				WritePixel(lpdata,offset+4,0xb0ccffcc);
				WritePixel(lpdata,offset-4,0xb0cccccc);
				WritePixel(lpdata,offset-4*pitch,0xb0ccffcc);
				WritePixel(lpdata,offset+4*pitch,0xb0ccffcc);
			}
	}

	if(level.objectives==0) bordercolour = 0xa000e000;
	else bordercolour = 0xffffffff;

	//Write border
	for(i=0;i<pitch;i++) WritePixel(lpdata,i*4,bordercolour);
	for(i=pitch*(RADARHEIGHT-1);i<pitch*RADARHEIGHT;i++) WritePixel(lpdata,i*4,bordercolour);
	for(i=0;i<pitch*RADARHEIGHT;i+=pitch) WritePixel(lpdata,i*4,bordercolour);
	for(i=pitch-1;i<pitch*RADARHEIGHT;i+=pitch) WritePixel(lpdata,i*4,bordercolour);

	if(FAILED(ship[player].radar.texture->UnlockRect(0))) return false;


	if(ship[player].equipment[SCANNER])
	{
		//	ship[player].radarrange = 0.01f;	//Zoomed out long way
		//	ship[player].radarrange = 0.05f;	//Zoomed in to about 1:1 screen:radar ratio
		if(enemiesexist)
		{
			//To zoom out, decrease the radarrange parameter
			if(!enemyinradar) ship[player].radarrange -= frametime*0.01f;
			else ship[player].radarrange += frametime*0.01f;

			if(ship[player].radarrange>0.032f) ship[player].radarrange = 0.032f;
			else if(ship[player].radarrange<0.01f) ship[player].radarrange = 0.01f;
		}
		else
		{
			ship[player].radarrange += 0.001f;
			if(ship[player].radarrange>0.032f) ship[player].radarrange = 0.032f;
		}
	}
	else ship[player].radarrange = 0.032f;

	DrawGraphicalObject(&ship[player].radar,ship[player].radarposition.x,ship[player].radarposition.y,0,0,screenratio,true,0xffffffff);
	return true;
}


bool AlphaFromColour(GraphicalObject *object)
{
	DWORD pixel, offset, avg;
	D3DLOCKED_RECT locked;
	int i, j;

	if(FAILED(object->texture->LockRect(0,&locked,NULL,NULL)))
	{
		dbout<<"Locking surface of "<<object->name<<" failed in AlphaFromColour"<<endl;
		return false;
	}

	DWORD pitch = locked.Pitch/4;
	LPDWORD lpdata = (DWORD *)locked.pBits;

	for(j=0;j<object->info.Height;j++)
	{
		for(i=0;i<object->info.Width;i++)
		{
			offset = (i + j*pitch)*4;
			ReadPixel(lpdata,offset,pixel);
			avg = (pixel&0xff) + ((pixel>>8)&0xff) + ((pixel>>16)&0xff);
			avg = (((avg/3)&0xff)<<24)&0xff000000;
			//dbout<<avg<<endl;
			pixel = pixel&0xffffff;
			pixel = pixel|avg;
			WritePixel(lpdata,offset,pixel);
		}
	}

	object->texture->UnlockRect(0);
	return true;
}


void DrawEnergyBars(int player)
{
	static unsigned int alpha[2], timer[2];
	float ratio;
	int width;
	RECT src;

	if(ship[player].state==PLAY || ship[player].state==DEAD)
	{
		ratio = ship[player].energy / ship[player].maxenergy;
		if(ratio>1.0) ratio = 1.0;
		if(ratio<0.2)
		{
			if(timer[player]>=64)
			{
				timer[player] = 0;
				alpha[player] = 0xb0ffffff;
			}
			else
			{
				alpha[player] -= 4;		//Subtract blue
				alpha[player] -= 1024;	//Subtract green
				++timer[player];
			}
		}
		else
		{
			alpha[player] = 0xb0ffffff;
			timer[player] = 0;
		}
		width = energybar.totalwidth*ratio;
		src.left = 0;
		src.top = 0;
		src.bottom = energybar.totalheight;
		src.right = width;
		DrawGraphicalObject(&energybarframe, ship[player].energyposition.x, ship[player].energyposition.y, 0, 0, screenratio, true, alpha[player]);
		DrawGraphicalObject(&energybar, ship[player].energyposition.x, ship[player].energyposition.y, src, 0, 0, screenratio, 0xffffffff);
	}
}

//Draw the lightning if this drone is firing
void RenderLightning(int ownerplayer, int drawplayer)
{
	if(ship[ownerplayer].equipment[LIGHTNING_DRONE] && ship[ownerplayer].lightningdrone!=NULL)
	{
		if(CHECK(ship[ownerplayer].lightningdrone->state,LOCKED))
		{
			for(int i=0;i<ship[ownerplayer].lightningtree.bolts;++i)
			{
				if(ship[ownerplayer].lightningtree.targetlist[i].starttarget->onscreen[drawplayer] || ship[ownerplayer].lightningtree.targetlist[i].endtarget->onscreen[drawplayer])
				{
					DrawLightning(ship[ownerplayer].lightningtree.targetlist[i].starttarget->scrposition[drawplayer], ship[ownerplayer].lightningtree.targetlist[i].endtarget->scrposition[drawplayer],&lightning1[ownerplayer]);
					//DrawLightning(ship[ownerplayer].lightningtree.targetlist[i].starttarget->scrposition[drawplayer], ship[ownerplayer].lightningtree.targetlist[i].endtarget->scrposition[drawplayer],&lightning1[ownerplayer]);
					DrawLightning(ship[ownerplayer].lightningtree.targetlist[i].starttarget->scrposition[drawplayer], ship[ownerplayer].lightningtree.targetlist[i].endtarget->scrposition[drawplayer],&lightning2[ownerplayer]);
					//DrawLightning(ship[ownerplayer].lightningtree.targetlist[i].starttarget->scrposition[drawplayer], ship[ownerplayer].lightningtree.targetlist[i].endtarget->scrposition[drawplayer],&lightning2[ownerplayer]);
					DrawLightning(ship[ownerplayer].lightningtree.targetlist[i].starttarget->scrposition[drawplayer], ship[ownerplayer].lightningtree.targetlist[i].endtarget->scrposition[drawplayer],&lightning3[ownerplayer]);
				}
			}
		}
	}
}


//Draw a lightning beam
void DrawLightning(t_point start, t_point end, GraphicalObject *lightning)
{
	const float lightningspread = 1.0f;

	t_point pos, newpos;
	t_vector unitvector;
	float dist, temp;
	int counter = 0;
	newpos = start;
	do
	{
		pos = newpos;
		unitvector.x = end.x - pos.x;
		unitvector.y = end.y - pos.y;
		temp = randlim(-lightningspread,lightningspread)*ABS(unitvector.y);
		unitvector.y += randlim(-lightningspread,lightningspread)*ABS(unitvector.x);
		unitvector.x += temp;
		dist = Normalise(&unitvector);
		newpos.x += lrintf(unitvector.x*32.0f*ship[0].screenscale);
		newpos.y += lrintf(unitvector.y*32.0f*ship[0].screenscale);
		Line(lightning, rand()&0xf, pos.x, pos.y, newpos.x, newpos.y, 0xffffffff);
		++counter;
	}
	while(dist>64.0f*ship[0].screenscale && counter<1000);
	Line(lightning, rand()&0xf, newpos.x, newpos.y, end.x, end.y, 0xffffffff);
}

void DrawShieldBars(int player)
{
	static unsigned int alpha[2];
	float ratio;
	int width;
	RECT src;

	if(ship[player].shieldhit)
	{	ship[player].shieldhit = false;
		alpha[player] = 0x90ffffff;
	}

	ratio = ship[player].shields/ship[player].maxshields;
	if(ratio<=0)
	{	ratio = 0;
		alpha[player] = 0;
		return;
	}
	if(ratio>1.0) ratio = 1.0;

	if(alpha[player]>0xffffff)
	{	width = shieldbar.totalwidth*ratio;
		src.left = 0;
		src.top = 0;
		src.bottom = shieldbar.totalheight;
		src.right = width;
		DrawGraphicalObject(&shieldframe, ship[player].radarposition.x, ship[player].radarposition.y - lrintf(screenratio*80.0f), 0, 0, screenratio, true, alpha[player]);
		DrawGraphicalObject(&shieldbar, ship[player].radarposition.x, ship[player].radarposition.y - lrintf(screenratio*80.0f), src, 0, 0, screenratio, alpha[player]);
		alpha[player] -= 0x1000000;		//Subtract alpha
	}
}

void DrawHitBars(GameObject *object, int player)
{
	float ratio;
	int width;
	RECT src;

	if(object->baralpha<0x1000000) return;

	ratio = object->energy/object->maxenergy;
	if(ratio<=0)
	{	object->baralpha = 0xffffff;
		return;
	}
	if(ratio>1.0) ratio = 1.0;

	if(object->baralpha>0xffffff)
	{	width = hitbar.totalwidth*ratio;
		src.left = 0;
		src.top = 0;
		src.bottom = hitbar.totalheight;
		src.right = width;
		DrawGraphicalObject(&hitbarframe, object->scrposition[player].x, object->scrposition[player].y+40, 0, 0, screenratio, true, object->baralpha);
		DrawGraphicalObject(&hitbar, object->scrposition[player].x, object->scrposition[player].y+40, src, 0, 0, screenratio, object->baralpha);
		if(player==0) object->baralpha -= 0x1000000;		//Subtract alpha
	}
}


void ShowLogo(GraphicalObject *object, D3DCOLOR alpha)
{
	if( dx9.pd3dDevice == NULL)
		return;

	scene.SceneBegin(&dx9.MainViewPort);
	DrawGraphicalObject(object,g_screenx/2,g_screeny/2,0,0,screenratio,true,alpha);
	scene.SceneFlip();
}


// Draws the basic base layout
void Base::DrawBase()
{
	float xp = 32.0, yp = 32.0;
	int xlim = playmode==SINGLEPLAYER?(int)(screenratio*650.0f):(ship[player].scrbottomright.x + 32);

	//Draw the background
	while((yp - 32.0)<g_screeny)
	{
		while(xp<xlim)
		{
			DrawGraphicalObject(&basebg,lrintf(xp),lrintf(yp),0,0,0xffffffff);
			xp += 64.0;
		}
		xp = 32.0;
		yp += 64.0;
	}

	//Draw screen
	DrawGraphicalObject(&interfacescreen,layout.interfacescreenposition.x,layout.interfacescreenposition.y,0,0,screenratio,true,0xffffffff);
	if(playmode==SINGLEPLAYER)
	{
		DrawGraphicalObject(&interfacescreen,layout.interfacescreen2position.x,layout.interfacescreen2position.y,0,0,screenratio,true,0xffffffff);
		DrawGraphicalObject(&interfacescreen,layout.interfacescreen3position.x,layout.interfacescreen3position.y,0,0,screenratio,true,0xffffffff);
	}

	int alphatwo, nextframe;
	FrameAlphaInterpolate(dummy.graphics.frames, 0xff80ff80, angle, &alphatwo, (unsigned int *)&frame, &nextframe);

	//Draw ship
	if(playmode==SINGLEPLAYER)
	{
		DrawGraphicalObject(&dummy.graphics,layout.interfacescreen2position.x,layout.interfacescreen2position.y,frame,angle,screenratio,true,0xff80ff80);
		DrawGraphicalObject(&dummy.graphics,layout.interfacescreen2position.x,layout.interfacescreen2position.y,nextframe,angle,screenratio,true,alphatwo);
		DrawGraphicalObject(&glyph,layout.interfacescreen3position.x,layout.interfacescreen3position.y,0,0,screenratio,true,0xd0ffffff);
	}
	else
	{
		DrawGraphicalObject(&dummy.graphics,layout.interfacescreenposition.x,layout.interfacescreenposition.y,frame,angle,screenratio,true,0xff80ff80);
		DrawGraphicalObject(&dummy.graphics,layout.interfacescreenposition.x,layout.interfacescreenposition.y,nextframe,angle,screenratio,true,alphatwo);
	}

	//Draw data screen
	DrawGraphicalObject(&datascreen,layout.datascreenposition.x,layout.datascreenposition.y,0,0,screenratio,true,0xffffffff);
	//Draw meter panel
	DrawGraphicalObject(&meterpanel,layout.panelposition.x,layout.panelposition.y,0,0,screenratio,true,0xffffffff);
	//Draw ship logo
	DrawGraphicalObject(&dummy.logographics,layout.logoposition.x,layout.logoposition.y,0,0,screenratio,true,0xffffffff);
	//Draw message screen
	DrawGraphicalObject(&messagescreen,layout.messagescreenposition.x, layout.messagescreenposition.y,0,0,screenratio,true,0xffffffff);
	if(selectbar>=0) DrawGraphicalObject(&selectionbar,layout.selectionbarposition.x,layout.selectionbarposition.y + screenratio*(float)(selectbar*font.frameheight),0,0,screenratio,true,0x70ffffff);

	for(int i=0;i<6;i++)
	{
		if(button[i]!=BLANK)
		{
			DrawGraphicalObject(&buttonframe,layout.bposition[i].x,layout.bposition[i].y,buttonselector==i?1:0,0,screenratio,true,0xffffffff);
			DrawGraphicalObject(&buttong[button[i]],layout.bposition[i].x,layout.bposition[i].y,0,0,screenratio,true,buttonselector==i?0xffffffff:0x90b0ffff);
		}
		else
		{
			DrawGraphicalObject(&buttonframe,layout.bposition[i].x,layout.bposition[i].y,buttonselector==i?1:0,0,screenratio,true,0x90ffffff);
		}
	}

	for(int i=0;i<3;i++)
	{
		DrawGraphicalObject(&meterframe,layout.meterposition[i].x,layout.meterposition[i].y,0,0,screenratio,true,0xffffffff);
		meter[i].DrawMeter(0);
		switch(meter[i].type)
		{
		case MPOWER:
			DrawGraphicalObject(&textpower,layout.meterposition[i].x,layout.meterposition[i].y,0,0,screenratio,true,0xffffffff);
			break;
		case MUSED:
			DrawGraphicalObject(&textpowerused,layout.meterposition[i].x,layout.meterposition[i].y,0,0,screenratio,true,0xffffffff);
			break;
		case MTOTAL:
			DrawGraphicalObject(&texttotal,layout.meterposition[i].x,layout.meterposition[i].y,0,0,screenratio,true,0xffffffff);
			break;
		case MSPEED:
			DrawGraphicalObject(&textspeed,layout.meterposition[i].x,layout.meterposition[i].y,0,0,screenratio,true,0xffffffff);
			break;
		case MENERGY:
			DrawGraphicalObject(&textenergy,layout.meterposition[i].x,layout.meterposition[i].y,0,0,screenratio,true,0xffffffff);
			break;
		}
	}

	DrawString(&font,descstring,layout.descstringposition.x,layout.descstringposition.y,layout.descstringlimit,GREENHIGHLIGHT);
	if(playmode==SINGLEPLAYER) DrawString(&font,descstring2,layout.descstring2position.x,layout.descstring2position.y,layout.descstring2limit,GREEN);
	DrawString(&font,datastring,layout.datastringposition.x,layout.datastringposition.y,layout.datastringlimit,GREENHIGHLIGHT);
	DrawString(&font,messagestring,layout.messagestringposition.x,layout.messagestringposition.y,layout.messagestringlimit,GREENHIGHLIGHT);

}


//----------------------------------------------------------------------------
// DrawScene() : Draws the contents of the scene
//----------------------------------------------------------------------------
void DrawScene()
{
	scene.ChangeViewport(&ship[0].viewport);

	if(options.aa)	//Set anti-aliasing to on
	{
		dx9.pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		dx9.pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	}
	else	//Set anti-aliasing to off
	{
		dx9.pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
		dx9.pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	}


	//For shader code only, fade screen if explosion happened
	if(ship[0].light>1.0f) ship[0].light -= 10.0f*frametime;
	if(ship[0].light<1.0f) ship[0].light = 1.0f; 

	if(ship[0].state == PLAY || ship[0].state==DEAD)
	{
		scene.SpriteEnd();
		if(options.drawmethod==SHADER) DrawBGshader(0);
		else DrawBG(0);
		scene.SpriteBegin();

		DrawObjects(0);
		RenderLightning(0,0);
		if(playmode==TWOPLAYER) RenderLightning(1,0);
		//Line(&lightning,(thistick/100)%8,100,100,356,100,0xffffffff);
		DrawEnergyBars(0);
		DrawShieldBars(0);
		if(ship[0].equipment[MISSILE_LAUNCHER]) DrawNumber(&numbers,ship[0].missiles,ship[0].energyposition.x+screenratio*60.0f,ship[0].energyposition.y,0xc0ffffff);
		if(ship[0].credits) DrawNumber(&numbers,ship[0].credits,ship[0].energyposition.x-35,screenratio*70.0f,0xc08080ff);
	}
	if(ship[0].state==PLAY)
	{
		DrawShipGlyphs(&ship[0],0);
		ship[0].screenmessage.DrawMessage(0);
		if(ship[0].equipment[RADAR]) DrawRadar(0);
	}
	if(ship[0].state==INBASE) base[0].DrawBase();

	if(options.framecounter)
	{
		char framestring[20];
		sprintf_s(framestring,20,"%f",framespersec);
		DrawString(&font,framestring,350,20,g_screenx,GREENHIGHLIGHT);
	}

	if(playmode==TWOPLAYER)
	{
		scene.ChangeViewport(&ship[1].viewport);

		if(options.aa)	//Set anti-aliasing to on
		{
			dx9.pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
			dx9.pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		}
		else	//Set anti-aliasing to off
		{
			dx9.pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
			dx9.pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
		}
		
		//For shader code only, fade screen if explosion happened
		if(ship[1].light>1.0f) ship[1].light -= 10.0f*frametime;
		if(ship[1].light<1.0f) ship[1].light = 1.0f; 

		if(ship[1].state == PLAY || ship[1].state==DEAD)
		{
			scene.SpriteEnd();
			if(options.drawmethod==SHADER) DrawBGshader(1);
			else DrawBG(1);
			scene.SpriteBegin();
			DrawObjects(1);
			RenderLightning(1,1);
			RenderLightning(0,1);
			DrawEnergyBars(1);
			DrawShieldBars(1);
			if(ship[1].equipment[MISSILE_LAUNCHER]) DrawNumber(&numbers,ship[1].missiles,ship[1].energyposition.x+screenratio*60.0f,ship[1].energyposition.y,0xc0ffffff);
			if(ship[1].credits) DrawNumber(&numbers,ship[1].credits,ship[1].energyposition.x-35,screenratio*70.0f,0xc08080ff);
			if(options.mouseplayer>0) DrawGraphicalObject(&mousefollowtarget, mousetarget.scrposition.x, mousetarget.scrposition.y, 0, 0, 0xc0ffffff);
		}
		if(ship[1].state==PLAY)
		{
			RenderLightning(1,1);
			DrawShipGlyphs(&ship[1],1);
			ship[1].screenmessage.DrawMessage(1);
			if(ship[1].equipment[RADAR]) DrawRadar(1);
		}
		if(ship[1].state==INBASE) base[1].DrawBase();

		scene.ChangeViewport(&dx9.MainViewPort);
		DrawGraphicalObject(&dividingline,g_screenx>>1,g_screeny>>1,0,0,0xffffffff);
	}

	//if(light>1.0f) light = 1.0f;

    return;
}

void Meter::DrawMeter(float limit)
{
	long alpha = 0xcfffffff;
	float flength = length;

	if(limit==0) limit = maxvalue;

	src.top = flength - (value / maxvalue)*flength;

	if(value>limit)
		alpha = 0xcfff0000;

	if(value>maxvalue)
		src.top = 0;
 
	DrawGraphicalObject(bar,x,y + screenratio*(float)(src.top),src,0,0,screenratio,alpha);

	return;
}

void TileGraphics::FindOptimumDimensions(int *x, int *y)
{
	int xsize = numbertiles*TILESIZE, ysize = TILESIZE;
	int optx = 0, opty = 0;
	while(!IsPowerOfTwo(xsize)) xsize += TILESIZE;	//Make sure we start with a power of two size

	UINT minpixelswasted = 0xffffffff, pixelswasted;

	while(xsize)
	{
		//dbout<<"Checking "<<xsize<<" "<<ysize<<endl;
		//Check sizes are in range
		if(xsize>maxwidth)				//Too wide, go to next size
		{
			xsize/=2;		//Halve xsize, find ysize
			do
			{
				ysize += TILESIZE;
			}
			while( (xsize/TILESIZE)*(ysize/TILESIZE)<numbertiles && !IsPowerOfTwo(ysize) );
			continue;
		}
		if(ysize>maxheight) break;		//Too high, so stop

		//Calculate memory wastage
		pixelswasted = xsize*ysize - numbertiles*TILESIZE*TILESIZE;
		if(pixelswasted<minpixelswasted)
		{
			//Found a more efficient dimension, save it
			minpixelswasted = pixelswasted;
			optx = xsize;
			opty = ysize;
		}

		//Check next size
		xsize/=2;		//Halve xsize, find ysize
		do
		{
			 ysize += TILESIZE;
		}
		while( (xsize/TILESIZE)*(ysize/TILESIZE)<numbertiles && !IsPowerOfTwo(ysize) );
	}
	//dbout<<"Optimum "<<optx<<" * "<<opty<<", wasted: "<<minpixelswasted<<endl;
	*x = optx;
	*y = opty;
	return;
}

bool TileGraphics::CreatePowerTwoSheet()
{
	int nwidth, nheight, tlwidth, tlheight, x, y, nx, ny, oldsize = LowestPowerTwo(xtiles)*LowestPowerTwo(ytiles)*TILESIZE*TILESIZE;

	//dbout<<"Texture: "<<numbertiles<<" = "<<xtiles<<" * "<<ytiles<<" = "<<oldsize<<endl;

	FindOptimumDimensions(&nwidth, &nheight);
	tlwidth = nwidth/TILESIZE;
	tlheight = nheight/TILESIZE;

	if(tlwidth == xtiles && tlheight == ytiles)
	{
		//Already in optimum format; just use same data
		tiles = loadtiles;
		loadtiles = NULL;
	}
	else
	{
		//Create texture in new size
		if(FAILED(D3DXCreateTexture(dx9.pd3dDevice, nwidth, nheight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &tiles)))
		{
			dbout<<"Texture creation failed"<<endl;
			tiles = NULL;
			return false;
		}

		//Copy over all tiles
		x = 0;
		y = 0;
		nx = 0;
		ny = 0;
		for(int i=0;i<numbertiles;++i,++x,++nx)
		{
			if(x==xtiles)
			{
				x = 0;
				++y;
			}
			if(nx==tlwidth)
			{
				nx = 0;
				++ny;
			}
			MoveTile(loadtiles, tiles, x, y, nx, ny);
		}

		//Reset parameters
		xtiles = tlwidth;
		xshift = WhichPowerOfTwo(xtiles);
		ytiles = tlheight;
		numbertiles = xtiles*ytiles;
	}

	//This is the size of one tile in texture coordinates
	xtextureincrement = 1.0f/xtiles;
	ytextureincrement = 1.0f/ytiles;

	//This is the amount to shrink the texture coordinates by in order to avoid the bilinearly filtered edge showing
	//It amounts to 0.5 pixels
	textureshrinkx = 0.5f*xtextureincrement/FTILESIZE;
	textureshrinky = 0.5f*ytextureincrement/FTILESIZE;

	if(loadtiles!=NULL) loadtiles->Release();
	//if(tiles!=NULL) tiles->Release();
	//loadtiles = tiles;
	loadtiles = NULL;

	//dbout<<"Resized texture to "<<numbertiles<<", new size: "<<nwidth<<" * "<<nheight<<" = "<<tlwidth<<" * "<<tlheight<<endl;
	//dbout<<"Saving "<<(oldsize-(nwidth*nheight))<<endl;
	return true;
}

bool TileGraphics::LoadTiles(char *filename)
{	char filenm[50];
	D3DXIMAGE_INFO info;
	int xtile, ytile;

	//Release texture if it hasn't already been
	if(tiles!=NULL) tiles->Release();
	if(loadtiles!=NULL) loadtiles->Release();
	if(tilegraphics!=NULL) delete [] tilegraphics;

	sprintf_s(filenm,50,"graphics\\%s.png",filename);	//Set up our file names

	if( (loadtiles = LoadTexture(filenm,&info)) == NULL)
	{
		//Error code
		return false;
	}

	width = info.Width;
	height = info.Height;
	xtiles = width/TILESIZE;
	ytiles = height/TILESIZE;
	numbertiles = xtiles * ytiles;

	CreatePowerTwoSheet();
/*
	tilegraphics = new GraphicalObject[numbertiles];
	for(int i=0;i<numbertiles;i++)
	{
		tilegraphics[i].CreateGraphicalObject(TILESIZE,TILESIZE);
		xtile = i%xtiles;
		ytile = i/xtiles;
		CopyTileFromTexture(&tilegraphics[i],loadtiles,xtile,ytile);
	}
	if(loadtiles!=NULL) loadtiles->Release();
	loadtiles = NULL;*/

	//dbout<<"Loaded texture: "<<width<<" by "<<height<<", "<<xtiles<<" by "<<ytiles<<", total "<<numbertiles<<endl;

	return true;
}


void ShowPrepareScreen()
{
	if( dx9.pd3dDevice == NULL)
		return;

	scene.SceneBegin(&dx9.MainViewPort);

	DrawGraphicalObject(&menubg,g_screenx/2,g_screeny/2,0,0,screenratio*1.171875f,true,0xffffffff);
	DrawGraphicalObject(&projperilogo,g_screenx/2,g_screeny/2,0,0,screenratio,true,0x80ffffff);		//Display logo
	DrawString(&menufont, level.name, 100, 100, g_screenx-100, GREENHIGHLIGHT);
	DrawString(&font, level.description, 100, 150, g_screenx-100, GREENHIGHLIGHT);

	scene.SceneFlip();
}

/*
void BlitD3D(LPDIRECT3DTEXTURE9 texture, TLVERTEX *reqvertices)
{
	TLVERTEX* vertices;

	//Lock the vertex buffer
	HRESULT hr = dx9.vertexBuffer->Lock(0, 0, (void**)&vertices, NULL);
	if(FAILED(hr))
	{
		dbout<<"Failed to lock vertex buffer"<<endl;
		return;
	}

	vertices[0] = reqvertices[0];
	vertices[1] = reqvertices[1];
	vertices[2] = reqvertices[2];
	vertices[3] = reqvertices[3];

	//Unlock the vertex buffer
	dx9.vertexBuffer->Unlock();

	//Set texture
	dx9.pd3dDevice->SetTexture (0, texture);

	//Draw image
	dx9.pd3dDevice->DrawPrimitive (D3DPT_TRIANGLEFAN, 0, 2);
}*/


//Code to draw the background tiles 
void DrawBG(int player)
{
	tPolygon screen(4);
    t_vector position;
    t_point cornertile[4], screenposition;
    t_point smallest, largest;
	TestTile tile;
	//static TLVERTEX vertices[4000];
	int currentvertex = 0, numtris = 0;
	float angle = ship[player].angle;

    //Find real positions of all four corners relative to the ship
	screen.vertex[0] = ship[player].screen.vertex[0];
    screen.vertex[1] = ship[player].screen.vertex[1];
    screen.vertex[2] = ship[player].screen.vertex[2];
    screen.vertex[3] = ship[player].screen.vertex[3];


    //Work out which tiles are in the corners
    for(int i=0;i<4;i++)
    {
		//Work out the index of the tile in each corner	(no wrapping applied yet)
        cornertile[i].x = screen.vertex[i].x / TILESIZE;
        cornertile[i].y = screen.vertex[i].y / TILESIZE;
    }

    smallest.x = 2000000000;	//Big number
    smallest.y = 2000000000;
    largest.x = -1000;			//Less than zero
    largest.y = -1000;

    //Find the outer limits of the rectangle of tiles that may be on screen for this player, and add corner tiles to draw list
    for(int i=0;i<4;i++)
    {
        if(cornertile[i].x<smallest.x) smallest.x = cornertile[i].x;
        if(cornertile[i].x>largest.x) largest.x = cornertile[i].x;
		if(cornertile[i].y<smallest.y) smallest.y = cornertile[i].y;
        if(cornertile[i].y>largest.y) largest.y = cornertile[i].y;

		position.x = cornertile[i].x*TILESIZE + HALFTILESIZE;
		position.y = cornertile[i].y*TILESIZE + HALFTILESIZE;

		if(position.x>=level.pixelwidth) position.x -= level.pixelwidth;
		else if(position.x<0) position.x += level.pixelwidth;
		if(position.y>=level.pixelheight) position.y -= level.pixelheight;
		else if(position.y<0) position.y += level.pixelheight;

		//Generate the corner positions
		tile.corner[0].realposition.x = position.x - HALFTILESIZE;
		tile.corner[0].realposition.y = position.y - HALFTILESIZE;

		tile.corner[1].realposition.x = position.x + HALFTILESIZE;
		tile.corner[1].realposition.y = position.y - HALFTILESIZE;

		tile.corner[2].realposition.x = position.x + HALFTILESIZE;
		tile.corner[2].realposition.y = position.y + HALFTILESIZE;

		tile.corner[3].realposition.x = position.x - HALFTILESIZE;
		tile.corner[3].realposition.y = position.y + HALFTILESIZE;

		//Work out the transformed positions
		PosToScreen(&tile.corner[0].realposition,&tile.corner[0].screenposition,player);
		PosToScreen(&tile.corner[1].realposition,&tile.corner[1].screenposition,player);
		PosToScreen(&tile.corner[2].realposition,&tile.corner[2].screenposition,player);
		PosToScreen(&tile.corner[3].realposition,&tile.corner[3].screenposition,player);

		//Work out the texture coordinates
		tile.tilelocation.x = cornertile[i].x;
		tile.tilelocation.y = cornertile[i].y;
		tile.tilenumber = WhichTile(tile.tilelocation.x,tile.tilelocation.y);
		tilegfx.GetTextureCoordinates(&tile);

		//Add the corner tiles to the vertex list and increment vertex counter
		currentvertex = AddTileToVertexList(&tile, dx9.vertices, currentvertex, 4000, ship[player].screenalpha);
		numtris += 2;
    }

	//Loop over all other tiles
	for(int j=smallest.y;j<=largest.y;j++)
	{
		//Doing this here and copying tile corners across each tile should save calculating positions twice
		position.x = smallest.x*TILESIZE + HALFTILESIZE;
		position.y = j*TILESIZE + HALFTILESIZE;

		tile.corner[1].realposition.x = position.x - HALFTILESIZE;
		tile.corner[1].realposition.y = position.y - HALFTILESIZE;
		tile.corner[2].realposition.x = position.x - HALFTILESIZE;
		tile.corner[2].realposition.y = position.y + HALFTILESIZE;

		if(PosToScreen(&tile.corner[1].realposition,&tile.corner[1].screenposition,player)) tile.corner[1].onscreen = true;
		else tile.corner[1].onscreen = false;
		if(PosToScreen(&tile.corner[2].realposition,&tile.corner[2].screenposition,player)) tile.corner[2].onscreen = true;
		else tile.corner[2].onscreen = false;

		for(int i=smallest.x;i<=largest.x;i++)
		{
			position.x = i*TILESIZE + HALFTILESIZE;
			position.y = j*TILESIZE + HALFTILESIZE;

			if(position.x>=level.pixelwidth) position.x -= level.pixelwidth;
			else if(position.x<0) position.x += level.pixelwidth;
			if(position.y>=level.pixelheight) position.y -= level.pixelheight;
			else if(position.y<0) position.y += level.pixelheight;

			//Generate the corner positions - space here for optimisation
			tile.corner[0] = tile.corner[1];
			tile.corner[3] = tile.corner[2];

			tile.corner[1].realposition.x = position.x + HALFTILESIZE;
			tile.corner[1].realposition.y = position.y - HALFTILESIZE;
			tile.corner[2].realposition.x = position.x + HALFTILESIZE;
			tile.corner[2].realposition.y = position.y + HALFTILESIZE;

			//Work out the transformed positions
			if(PosToScreen(&tile.corner[1].realposition,&tile.corner[1].screenposition,player)) tile.corner[1].onscreen = true;
			else tile.corner[1].onscreen = false;
			if(PosToScreen(&tile.corner[2].realposition,&tile.corner[2].screenposition,player)) tile.corner[2].onscreen = true;
			else tile.corner[2].onscreen = false;

			//Check if tile's on screen
			tile.onscreen = false;
			for(int k=0;k<4;++k) if(tile.corner[k].onscreen) tile.onscreen = true;

			//Work out the texture coordinates
			tile.tilelocation.x = i;
			tile.tilelocation.y = j;
			tile.tilenumber = WhichTile(i,j);
			tilegfx.GetTextureCoordinates(&tile);

			//Add the corner tiles to the vertex list and increment vertex counter
			currentvertex = AddTileToVertexList(&tile, dx9.vertices, currentvertex, 4000, ship[player].screenalpha);
			numtris += 2;

		}
	}

	//Set texture to be tile sheet
	dx9.pd3dDevice->SetTexture(0, tilegfx.tiles);

	//Copy vertices to buffer
	void *newvertices;
	dx9.vertexBuffer->Lock(0,0,&newvertices,0);
	memcpy(newvertices, &dx9.vertices, 3*numtris*sizeof(TLVERTEX));
	dx9.vertexBuffer->Unlock();

	//Draw background
	//dx9.pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, numtris, vertices, sizeof(TLVERTEX));
	dx9.pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, numtris);

//	if(numtris>dx9.pd3dcaps->MaxPrimitiveCount) dbout<<numtris<<" triangles > max primitives "<<dx9.pd3dcaps->MaxPrimitiveCount<<endl;
//	if(numtris>690 || numtris<310) dbout<<numtris<<" triangles = "<<currentvertex<<" vertices"<<endl;
}


int AddTileToVertexList(TestTile *tile, TLVERTEX *vertexbuffer, int position, int buffersize, DWORD colour)
{
	for(int i=0;i<3;++i,++position)
	{
		//Check for buffer overflow
		if(position>=buffersize) return buffersize;

		//Copy data to vertex
		vertexbuffer[position].x = tile->corner[i].screenposition.x;
		vertexbuffer[position].y = tile->corner[i].screenposition.y;
		vertexbuffer[position].u0 = tile->corner[i].texturecoord.x;
		vertexbuffer[position].v0 = tile->corner[i].texturecoord.y;
		vertexbuffer[position].z = 0;
		vertexbuffer[position].rhw = 1.0f;
		vertexbuffer[position].colour = colour;
	}

	for(int i=0, j=2; i<3; ++i,++position,++j)
	{
		//Check for buffer overflow
		if(position>=buffersize) return buffersize;

		j = j&3;	//In order to copy:  2, 3, 0

		//Copy data to vertex
		vertexbuffer[position].x = tile->corner[j].screenposition.x;
		vertexbuffer[position].y = tile->corner[j].screenposition.y;
		vertexbuffer[position].u0 = tile->corner[j].texturecoord.x;
		vertexbuffer[position].v0 = tile->corner[j].texturecoord.y;
		vertexbuffer[position].z = 0;
		vertexbuffer[position].rhw = 1.0f;
		vertexbuffer[position].colour = colour;
	}

	return position;
}

//Sort which explosions to draw
int PickExplosions(ExplosionData *expdata, int player)
{
	int i = 0;
	int counter = ship[player].explosions.explosioncounter;
	while(i<options.capexplosions)
	{
		if(ship[player].explosions.exps[counter].active)
		{
			expdata[i].x = ship[player].explosions.exps[counter].position.x;
			expdata[i].y = ship[player].explosions.exps[counter].position.y;
			expdata[i].radius = ship[player].explosions.exps[counter].radius;
			expdata[i].scale = ship[player].explosions.exps[counter].scale;
			//dbout<<expdata[i].x<<" "<<expdata[i].y<<" "<<expdata[i].radius<<endl;
			++i;
		}
		++counter;
		if(counter==options.capexplosions) counter = 0;
		if(counter==ship[player].explosions.explosioncounter) break;
	}
	//dbout<<"----------------"<<endl;
	return i;
}

//Code to draw the background tiles 
void DrawBGshader(int player)
{
    t_vector position;
    t_point cornertile[4], screenposition;
	TLVERTEX vertices[4], rvertices[4];
	ExplosionData expdata[MAXEXPLOSIONS];
	int numexplosions = 0;

	if(dx9.rotozoom == NULL)
	{
		dbout<<"Error: Rotozoom effect NULL!"<<endl;
		return;
	}

	if(options.explosiondistortions) numexplosions = PickExplosions(expdata, player);

    //Find positions of all four corners relative to the ship
	ship[player].screen.vertex[0] = RotateVectorQuick(ship[player].screencorner[TOPLEFT], ship[player].cosangle, -ship[player].sinangle);
    ship[player].screen.vertex[1] = RotateVectorQuick(ship[player].screencorner[TOPRIGHT], ship[player].cosangle, -ship[player].sinangle);
    ship[player].screen.vertex[2] = RotateVectorQuick(ship[player].screencorner[BOTTOMRIGHT], ship[player].cosangle, -ship[player].sinangle);
    ship[player].screen.vertex[3] = RotateVectorQuick(ship[player].screencorner[BOTTOMLEFT], ship[player].cosangle, -ship[player].sinangle);

	double inverse = 1.0/ship[player].screenscale;		//Invert the screen scale for division later

    //Work out the positions of the screen corners
    for(int i=0;i<4;i++)
    {
		ship[player].screen.vertex[i].x *= inverse;
		ship[player].screen.vertex[i].y *= inverse;

		//Shift to absolute coordinates
        ship[player].screen.vertex[i].x += ship[player].position.x;
        ship[player].screen.vertex[i].y += ship[player].position.y;
    }

	vertices[0].x = ship[player].scrtopleft.x;
	vertices[0].y = ship[player].scrtopleft.y;
	vertices[1].x = ship[player].scrbottomright.x;
	vertices[1].y = ship[player].scrtopleft.y;
	vertices[2].x = ship[player].scrbottomright.x;
	vertices[2].y = ship[player].scrbottomright.y;
	vertices[3].x = ship[player].scrtopleft.x;
	vertices[3].y = ship[player].scrbottomright.y;

	for(int i=0;i<4;++i)
	{
		vertices[i].z = 0.0f;
		vertices[i].rhw = 1.0f;
		vertices[i].colour = 0xffffffff;
		vertices[i].u0 = ship[player].screen.vertex[i].x/64.0f;		//Actual position of this point, in tile units
		vertices[i].v0 = ship[player].screen.vertex[i].y/64.0f;
		vertices[i].u1 = ship[player].screen.vertex[i].x / (float)level.newpixelwidth;	//Floating point map position of this vertex
		vertices[i].v1 = ship[player].screen.vertex[i].y / (float)level.newpixelheight;
		vertices[i].u2 = ship[player].screen.vertex[i].x;		//Map position of this vertex
		vertices[i].v2 = ship[player].screen.vertex[i].y;
	}

	HRESULT hr;
	if(options.explosionflashes) hr = dx9.rotozoom->SetFloat("light",ship[player].light);
	else hr = dx9.rotozoom->SetFloat("light",1.0f);
	if(FAILED(hr)) dbout<<"Error: Fail on setting values!"<<endl;

	t_vector tilesheetsize;
	tilesheetsize.x = tilegfx.xtiles;
	tilesheetsize.y = tilegfx.ytiles;
	hr = dx9.rotozoom->SetValue("tilesheetsize",&tilesheetsize,sizeof(t_vector));			//Number of tiles on the tile sheet
	if(FAILED(hr)) dbout<<"Error: Fail on setting values!"<<endl;

	t_vector mapsize;
	mapsize.x = level.newpixelwidth;
	mapsize.y = level.newpixelheight;
	hr = dx9.rotozoom->SetValue("levelsize",&mapsize,sizeof(t_vector));
	if(FAILED(hr)) dbout<<"Error: Fail on setting values!"<<endl;

	if(options.explosiondistortions && numexplosions>0)
	{
		hr = dx9.rotozoom->SetValue("ExplosionPositions",expdata,MAXEXPLOSIONS*sizeof(ExplosionData));
		if(FAILED(hr)) dbout<<"Error: SetValue position failed!"<<endl;
		hr = dx9.rotozoom->SetTechnique(dx9.hDistort[numexplosions]);
	}
	else
	{
		hr = dx9.rotozoom->SetTechnique(dx9.hTech);
	}

	if (SUCCEEDED(hr) ) 
	{
		UINT passes;
		hr = dx9.rotozoom->Begin(&passes, D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESHADERSTATE);
		if (SUCCEEDED(hr) ) 
		{
			for (UINT i=0;i<passes;i++) 
			{
				dx9.rotozoom->BeginPass(i);		// Set the pass
				//Draw single quad for background
				TLVERTEX* svertices;

				//Lock the vertex buffer
				dx9.vertexBuffer->Lock(0, 0, (void**)&svertices, NULL);
				svertices[0] = vertices[0];
				svertices[1] = vertices[1];
				svertices[2] = vertices[2];
				svertices[3] = vertices[3];
				dx9.vertexBuffer->Unlock();		//Unlock the vertex buffer

				//Draw image
				dx9.pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

				dx9.rotozoom->EndPass();
			}
			dx9.rotozoom->End();
		}
		else
		{
			if(hr==D3DERR_INVALIDCALL) dbout<<"Error: rotozoom->Begin() failed! HRESULT: D3DERR_INVALIDCALL"<<endl;
			if(hr==D3DXERR_INVALIDDATA) dbout<<"Error: rotozoom->Begin() failed! HRESULT: D3DXERR_INVALIDDATA"<<endl;
		}
	}
	else dbout<<"Error: SetTechnique() failed!"<<endl;

}

void Explosion::Update()
{
	if(!active) return;
	radius += frametime*EXPLOSIONEXPANDSPEED;
	scale = 100.0f - 0.5f*radius;
	if(radius>200.0f) active = false;
}

void Explosions::AddExplosion(float x, float y)
{
	exps[explosioncounter].position.x = x;
	exps[explosioncounter].position.y = y;
	exps[explosioncounter].radius = 0;
	exps[explosioncounter].scale = 100.0f;
	exps[explosioncounter].active = true;
	++explosioncounter;
	if(explosioncounter>=options.capexplosions) explosioncounter = 0;
}

void Explosions::Update()
{
	for(int i=0;i<options.capexplosions;++i) exps[i].Update();
}

void ScreenMessage::AddMessage(const char * messagein)
{
	message = messagein;
	alpha = 1.0f;
	colour = GREENHIGHLIGHT;
}

void ScreenMessage::DrawMessage(int player)
{
	if(Update())
	{
		int chars = ship[player].screenmessage.message.size();
		int msgsize = lrintf(screenratio*(chars*font.framewidth/2));
		DrawString(&font,message.c_str(), ship[player].radarposition.x - msgsize, ship[player].messageposition.y, ship[player].scrbottomright.x, colour);
	}
}

bool ScreenMessage::Update()
{
	alpha -= (1.1f - alpha)*frametime;
	if(alpha<0)
	{
		alpha = 0;
		colour = 0;
		message.clear();
		return false;
	}
	int alph = (((int)(alpha*255.0f))<<24)&0xff000000;
	colour = alph|(colour&0xffffff);
	return true;
}


void Line(GraphicalObject *linetex, int frame, int x1, int y1, int x2, int y2, int colour)
{
	int deltax = x2 - x1;
	int deltay = y2 - y1;
	int dx = deltax>>1;
	int dy = deltay>>1;
	float length = magic_sqrt((float)(deltax*deltax + deltay*deltay));
	float scale = length/(float)linetex->frameheight;

	float angle = atan((float)deltay/(float)deltax);

	RECT src, dest;
	int halfwidth = (linetex->framewidth>>1), halfheight = (linetex->frameheight>>1);

	src.left = linetex->framewidth*frame;
	src.top = 0;
	src.right = src.left + linetex->framewidth;
	src.bottom = linetex->frameheight;
//	dest.left = x1 + (deltax - dx);
//	dest.top = y1 + (deltay - dy);

	D3DXVECTOR2 scaling(1.0,scale), trans(0,0);

	// Set the parameters - translation (screen location)
	trans.x = (float) x1 + dx - halfwidth;
	trans.y = (float) y1 + dy - halfheight;

	//Set the centre of the object:
	D3DXVECTOR2 spriteCentre = D3DXVECTOR2((float)halfwidth,(float)halfheight);

	// Build our matrix to rotate, scale and position our sprite
	D3DXMATRIX mat;

	// Matrix out, scaling centre, scaling rotation, scaling, rotation centre, rotation, translation
	D3DXMatrixTransformation2D(&mat,&spriteCentre,0.0,&scaling,&spriteCentre,angle+PIBYTWO,&trans);

	// Tell the sprite about the matrix
	dx9.sprite->SetTransform(&mat);

	// And here we go:
	dx9.sprite->Draw( linetex->texture, &src, NULL, NULL, colour);
	//DrawGraphicalObject(&redline,x1+deltax/2,y1+deltay/2,0,angle+PIBYTWO,colour);
}