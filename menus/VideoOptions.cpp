/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "Framework.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "Slider.h"
#include "CheckBox.h"
#include "SpinControl.h"
#include "StringArrayModel.h"
#include "YesNoMessageBox.h"

#define ART_BANNER	  	"gfx/shell/head_vidoptions"
#define ART_GAMMA		"gfx/shell/gamma"

#define LEGACY_VIEWSIZE 0

class CMenuVidOptions : public CMenuFramework
{
private:
	void _Init() override;
	void _VidInit() override;
	void Reload() override;

public:
	CMenuVidOptions() : CMenuFramework( "CMenuVidOptions" ) { }
	void SaveAndPopMenu() override;
	void UpdateConfig();
	void GetConfig();

	int		outlineWidth;

	class CMenuVidPreview : public CMenuBitmap
	{
		void Draw() override;
	} testImage;

	CMenuPicButton	done;
	CMenuSpinControl	style; // ref_gl3's render style
	bool		gl3Active;

#if LEGACY_VIEWSIZE
	CMenuSlider	screenSize;
#endif
	CMenuSlider	gammaIntensity;
	CMenuSlider	brightness;
	CMenuCheckBox	vbo;
	CMenuCheckBox	swwater;
	CMenuCheckBox	overbright;
	CMenuCheckBox	filtering;
	CMenuCheckBox	detailtex;
	CMenuCheckBox	hudscale;
	CMenuCheckBox	hdmodels;
	CMenuYesNoMessageBox	msgBox;

	HIMAGE		hTestImage;
};

/*
=================
CMenuVidOptions::UpdateConfig
=================
*/
void CMenuVidOptions::UpdateConfig( void )
{
	float val1 = RemapVal( gammaIntensity.GetCurrentValue(), 0.0, 1.0, 1.8, 3.0 );
	float val2 = RemapVal( brightness.GetCurrentValue(), 0.0, 1.0, 0.0, 3.0 );
	EngFuncs::CvarSetValue( "gamma", val1 );
	EngFuncs::CvarSetValue( "brightness", val2 );
	EngFuncs::ProcessImage( hTestImage, val1, val2 );
}

void CMenuVidOptions::GetConfig( void )
{
	float val1 = EngFuncs::GetCvarFloat( "gamma" );
	float val2 = EngFuncs::GetCvarFloat( "brightness" );

	gammaIntensity.SetCurrentValue( RemapVal( val1, 1.8f, 3.0f, 0.0f, 1.0f ) );
	brightness.SetCurrentValue( RemapVal( val2, 0.0f, 3.0f, 0.0f, 1.0f ) );
	EngFuncs::ProcessImage( hTestImage, val1, val2 );

	gammaIntensity.SetOriginalValue( val1 );
	brightness.SetOriginalValue( val2 );
}

void CMenuVidOptions::SaveAndPopMenu( void )
{
#if LEGACY_VIEWSIZE
	screenSize.WriteCvar();
#endif
	// the gamepad UI's page has none of the renderer switches, and a switch that is not on the page has no
	// state to save
	if( !uiStatic.gamepadUI )
	{
		if( !gl3Active ) // under ref_gl3 the render style owns these
		{
			detailtex.WriteCvar();
			vbo.WriteCvar();
			swwater.WriteCvar();
			overbright.WriteCvar();
			filtering.WriteCvar();
		}
		hudscale.WriteCvar();
	}
	// gamma and brightness is already written

	const bool hdChanged = ( EngFuncs::GetCvarFloat( "fs_mount_hd" ) != 0.0f ) != hdmodels.bChecked;
	hdmodels.WriteCvar();
	// the HD content folder mounts at once from the menu; during a game the engine mounts it at the next new
	// game or load, when nothing loaded before is left
	if( hdChanged && !EngFuncs::ClientInGame( ))
		EngFuncs::ClientCmd( false, "fs_rescan\n" );

	CMenuFramework::SaveAndPopMenu();
}

/*
=================
CMenuVidOptions::Ownerdraw
=================
*/
void CMenuVidOptions::CMenuVidPreview::Draw( )
{
	int		color = 0xFFFF0000; // 255, 0, 0, 255
	int		viewport[4];
	int		viewsize, size, sb_lines;

#if LEGACY_VIEWSIZE
	viewsize = EngFuncs::GetCvarFloat( "viewsize" );
#else
	viewsize = 120;
#endif

	if( viewsize >= 120 )
		sb_lines = 0;	// no status bar at all
	else if( viewsize >= 110 )
		sb_lines = 24;	// no inventory
	else sb_lines = 48;

	size = Q_min( viewsize, 100 );

	viewport[2] = m_scSize.w * size / 100;
	viewport[3] = m_scSize.h * size / 100;

	if( viewport[3] > m_scSize.h - sb_lines )
		viewport[3] = m_scSize.h - sb_lines;
	if( viewport[3] > m_scSize.h )
		viewport[3] = m_scSize.h;

	viewport[2] &= ~7;
	viewport[3] &= ~1;

	viewport[0] = (m_scSize.w - viewport[2]) / 2;
	viewport[1] = (m_scSize.h - sb_lines - viewport[3]) / 2;

	UI_DrawPic( m_scPos.x + viewport[0], m_scPos.y + viewport[1], viewport[2], viewport[3], uiColorWhite, szPic );
	UI_DrawRectangleExt( m_scPos, m_scSize, color, ((CMenuVidOptions*)Parent())->outlineWidth );
}

/*
=================
CMenuVidOptions::Init
=================
*/
void CMenuVidOptions::_Init( void )
{
	hTestImage = EngFuncs::PIC_Load( ART_GAMMA, PIC_KEEP_SOURCE | PIC_EXPAND_SOURCE );

	banner.SetPicture( ART_BANNER );

	testImage.iFlags = QMF_INACTIVE;
	testImage.SetRect( 390, 225, 480, 450 );
	testImage.SetPicture( ART_GAMMA );

	int height = 280;

	// ref_vk is the same renderer on another API: the same render styles
	gl3Active = !stricmp( EngFuncs::GetCvarString( "r_refdll_loaded" ), "gl3" ) || !stricmp( EngFuncs::GetCvarString( "r_refdll_loaded" ), "vk" );

	// the styles, by the names the player knows them under: the two the original shipped, and ours
	static const char *styleStr[] = { L( "Original" ), L( "Software" ), L( "Enhanced" ) };
	static CStringArrayModel styleModel( styleStr, V_ARRAYSIZE( styleStr ));

	style.szName = L( "Renderer" );
	style.Setup( &styleModel );
	style.SetRect( 72, height, 280, 32 ); // the spin draws its name above itself
	if( gl3Active )
		height += 84;

	const int switchStep = gl3Active ? 0 : 50; // under ref_gl3 the render style owns the first five switches
	SET_EVENT_MULTI( style.onChanged,
	{
		CMenuVidOptions *parent = pSelf->GetParent( CMenuVidOptions );

		parent->style.WriteCvar();
	});

#if LEGACY_VIEWSIZE
	screenSize.szName = L( "Screen size" );
	screenSize.SetCoord( 72, height );
	screenSize.Setup( 30, 120, 10 );
	screenSize.onChanged = CMenuEditable::WriteCvarCb;

	height += 60;
#endif

	gammaIntensity.szName = L( "GameUI_Gamma" );
	gammaIntensity.SetCoord( 72, height );
	gammaIntensity.Setup( 0.0, 1.0, 0.025 );
	gammaIntensity.onChanged = VoidCb( &CMenuVidOptions::UpdateConfig );
	gammaIntensity.onCvarGet = VoidCb( &CMenuVidOptions::GetConfig );
	height += 60;

	brightness.SetCoord( 72, height );
	brightness.szName = L( "GameUI_Brightness" );
	brightness.Setup( 0, 1.0, 0.025 );
	brightness.onChanged = VoidCb( &CMenuVidOptions::UpdateConfig );
	brightness.onCvarGet = VoidCb( &CMenuVidOptions::GetConfig );
	height += 60;

	done.szName = L( "GameUI_OK" );
	done.SetCoord( 72, height );
	done.SetPicture( PC_DONE );
	done.onReleased = VoidCb( &CMenuVidOptions::SaveAndPopMenu );
	height += 60;

	detailtex.szName = L( "Detail textures" );
	detailtex.SetCoord( 72, height );
	height += switchStep;

	vbo.szName = L( "Use VBO" );
	vbo.SetCoord( 72, height );
	height += switchStep;

	swwater.szName = L( "Water ripples" );
	swwater.SetCoord( 72, height );
	height += switchStep;

	overbright.szName = L( "Overbrights" );
	overbright.SetCoord( 72, height );
	height += switchStep;

	filtering.szName = L( "Texture filtering" );
	filtering.SetCoord( 72, height );
	height += switchStep;

	hudscale.szName = L( "Auto scale HUD" );
	// the 25th update's high definition models (valve_hd)
	hdmodels.SetNameAndStatus( L( "GameUI_HDModels" ), L( "Takes effect when a game is started or loaded" ));
	hdmodels.SetCoord( 72, height );
	height += 50;

	hudscale.SetCoord( 72, height );
	height += 50;

	msgBox.SetMessage( L( "^1WARNING: This is an experimental option and turning it on might break mods!^7\n\nReload the game or reconnect to the server to apply the settings."));
	msgBox.onNegative.pExtra = &hudscale;
	SET_EVENT_MULTI( msgBox.onNegative,
	{
		CMenuCheckBox *cb = (CMenuCheckBox *)pExtra;

		cb->bChecked = false;
		cb->SetCvarValue( 0.0f );
	});

	SET_EVENT_MULTI( hudscale.onChanged,
	{
		CMenuVidOptions *parent = (CMenuVidOptions *)pSelf->Parent();

		if( EngFuncs::ClientInGame( ))
		{
			// bring up warning message box
			// FIXME: try to save the game, apply the settings, and then load it back. This will be useful when changing resolution too.
			parent->msgBox.Show();
		}
	});

	SET_EVENT_MULTI( hudscale.onCvarWrite,
	{
		CMenuCheckBox *cb = (CMenuCheckBox *)pSelf;

		if( cb->bChecked )
		{
			// automatically scale HUD as if we have 1024x768 screen
			// (saving aspect ratio)
			// FIXME: allow configuring this value?
			EngFuncs::CvarSetValue( cb->CvarName(), 1024.0f );
		}
		else
		{
			EngFuncs::CvarSetValue( cb->CvarName(), 0.0f );
		}
	});

	SET_EVENT_MULTI( hudscale.onCvarGet,
	{
		CMenuCheckBox *cb = (CMenuCheckBox *)pSelf;

		cb->bChecked = EngFuncs::GetCvarFloat( cb->CvarName() ) >= 640.0f;
	});

	if( uiStatic.gamepadUI )
	{
		// gamma and brightness in the column every page uses, the test picture hard against the right edge of
		// a 16:9 screen (1365 of these units wide), which leaves the column beside the items room for a line
		// that says something
		const int testX = Q_min( 837, uiStatic.width - 460 - 68 ); // a screen narrower than 16:9 moves it in
		testImage.SetRect( testX, 225, 460, 431 );
		SetStatusWidth( testX - UI_STATUS_COLUMN - 24 );

		height = UI_CONTENT_TOP;
		if( gl3Active )
		{
			style.SetRect( UI_ITEM_COLUMN, height, UI_ITEM_WIDTH, 32 );
			height += UI_ROW_NAMED + UI_GROUP_STEP * 2; // its box is taller than a slider's bar, and it is a group of one
		}
		gammaIntensity.SetCoord( UI_ITEM_COLUMN, height );
		height += UI_ROW_NAMED;
		brightness.SetCoord( UI_ITEM_COLUMN, height );
		height += UI_ROW_NAMED + UI_GROUP_STEP;
		gammaIntensity.size.w = brightness.size.w = UI_ITEM_WIDTH;
		hdmodels.SetCoord( UI_ITEM_COLUMN, height );
		height += 50 + UI_GROUP_STEP;
		done.SetCoord( UI_ITEM_COLUMN, height );
		bSaveOnBack = true;

		AddItem( banner );
		if( gl3Active )
		{
			AddItem( style );
			style.LinkCvar( "r_style", CMenuEditable::CVAR_VALUE );
		}
		AddItem( gammaIntensity );
		AddItem( brightness );
		AddItem( hdmodels );
		AddItem( done );
		AddItem( testImage );

		gammaIntensity.LinkCvar( "gamma" );
		brightness.LinkCvar( "brightness" );
		hdmodels.LinkCvar( "fs_mount_hd" );
		return;
	}

	AddItem( banner );
	if( gl3Active )
	{
		AddItem( style );
		style.LinkCvar( "r_style", CMenuEditable::CVAR_VALUE );
	}
	AddItem( done );
#if LEGACY_VIEWSIZE
	AddItem( screenSize );
#endif
	AddItem( gammaIntensity );
	AddItem( brightness );
	if( !gl3Active )
	{
		AddItem( detailtex );
		AddItem( vbo );
		AddItem( swwater );
		AddItem( overbright );
		AddItem( filtering );
	}
	AddItem( hdmodels );
	AddItem( hudscale );
	AddItem( testImage );

#if LEGACY_VIEWSIZE
	screenSize.LinkCvar( "viewsize" );
#endif

	gammaIntensity.LinkCvar( "gamma" );
	brightness.LinkCvar( "brightness" );

	detailtex.LinkCvar( "r_detailtextures" );
	swwater.LinkCvar( "r_ripple" );
	vbo.LinkCvar( "gl_vbo" );
	overbright.LinkCvar( "gl_overbright" );
	// skip filtering.LinkCvar, different names in ref_gl and ref_soft
	hdmodels.LinkCvar( "fs_mount_hd" );
	hudscale.LinkCvar( "hud_scale" );
}

void CMenuVidOptions::_VidInit()
{
	outlineWidth = 2;
	UI_ScaleCoords( NULL, NULL, &outlineWidth, NULL );
}

void CMenuVidOptions::Reload()
{
	// the gamepad UI's page carries none of the renderer switches, and nothing here may touch their cvars
	if( uiStatic.gamepadUI )
	{
		CMenuFramework::Reload();
		return;
	}

	if( gl3Active )
	{
		CMenuFramework::Reload();
		return;
	}

	bool gl_active = !strnicmp( EngFuncs::GetCvarString( "r_refdll_loaded" ), "gl", 2 );
	bool soft_active = !stricmp( EngFuncs::GetCvarString( "r_refdll_loaded" ), "soft" );

	detailtex.SetGrayed( !gl_active );
	detailtex.SetInactive( !gl_active );

	vbo.SetGrayed( !gl_active );
	vbo.SetInactive( !gl_active );

	if( gl_active )
	{
		if( EngFuncs::textfuncs.pfnIsCvarReadOnly( "gl_vbo" ) > 0 )
		{
			SET_EVENT_MULTI( vbo.onCvarChange,
			{
				CMenuCheckBox *cb = (CMenuCheckBox *)pSelf;
				cb->bChecked = false;
				UI_ShowMessageBox( L( "Not supported on your GPU" ));
			});

			vbo.onCvarWrite = CEventCallback::NoopCb;
		}
		else
		{
			vbo.onCvarWrite.Reset();
			vbo.onCvarChange.Reset();
		}
	}

	swwater.SetGrayed( !gl_active );
	swwater.SetInactive( !gl_active );

	overbright.SetGrayed( !gl_active );
	overbright.SetInactive( !gl_active );

	if( soft_active || gl_active )
	{
		filtering.SetGrayed( false );
		filtering.SetInactive( false );

		if( soft_active )
		{
			// no need to invert, 0 disables filtering
			filtering.LinkCvar( "sw_texfilt" );
		}
		else
		{
			SET_EVENT_MULTI( filtering.onCvarGet,
			{
				CMenuCheckBox *cb = (CMenuCheckBox *)pSelf;

				if( EngFuncs::GetCvarFloat( cb->CvarName( )))
					cb->bChecked = false;
				else cb->bChecked = true;
			});
			SET_EVENT_MULTI( filtering.onCvarWrite,
			{
				CMenuCheckBox *cb = (CMenuCheckBox *)pSelf;
				if( cb->bChecked )
					EngFuncs::CvarSetValue( cb->CvarName(), 0.0f );
				else EngFuncs::CvarSetValue( cb->CvarName(), 1.0f );
			});

			filtering.LinkCvar( "gl_texture_nearest" );
		}
	}
	else
	{
		filtering.SetGrayed( true );
		filtering.SetInactive( true );
	}

	CMenuFramework::Reload();
}

ADD_MENU( menu_vidoptions, CMenuVidOptions, UI_VidOptions_Menu );
