/*
Controller.cpp -- the controller page of the gamepad UI
Copyright (C) 2026 18woldemar

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

// The gamepad UI's one page for the pad, in place of Controls, Advanced controls and Gamepad, whose keyboard,
// mouse and axis settings a pad player has no use for: the layout drawn on the controller (a picture made
// per language from Half-Life's 25th anniversary layout, which the engine binds by default) and the four
// settings a pad player needs. Every setting applies as it changes.

#include <math.h>
#include "Framework.h"
#include "Bitmap.h"
#include "Slider.h"
#include "CheckBox.h"

#define ART_BANNER "gfx/shell/head_controller"
#define ART_LAYOUT "gfx/shell/controller" // 912x331 menu units: the picture, its leader lines and labels

class CMenuController : public CMenuFramework
{
public:
	CMenuController() : CMenuFramework( "CMenuController" ) { }

private:
	void _Init() override;
	void _VidInit() override;
	void LookChanged();
	void InvertChanged();

	CMenuBitmap layout;
	CMenuSlider look;
	CMenuCheckBox invert, vibration, autoaim;
	float pitchRatio; // joy_pitch to joy_yaw when the page opened: the look slider keeps it
};

void CMenuController::_Init()
{
	banner.SetPicture( ART_BANNER );
	bSaveOnBack = true;

	layout.SetPicture( ART_LAYOUT );
	layout.SetRenderMode( QM_DRAWTRANS );
	layout.iFlags = QMF_INACTIVE;
	// The settings in the column every page uses, the drawing beside them at the size it was drawn for: its
	// labels and leader lines go out on both sides of the pad, and a 16:9 screen is 1365 of these units wide,
	// so both fit side by side. Its first labels stand level with the first setting.
	layout.SetRect( 385, 245, 912, 331 );

	int y = UI_CONTENT_TOP;

	look.szName = L( "Look sensitivity" );
	look.Setup( 20.0f, 300.0f, 10.0f );
	look.onChanged = VoidCb( &CMenuController::LookChanged );
	look.SetCoord( UI_ITEM_COLUMN, y );
	look.size.w = UI_ITEM_WIDTH;
	y += UI_ROW_NAMED + UI_GROUP_STEP;

	invert.szName = L( "Invert look" );
	invert.onChanged = VoidCb( &CMenuController::InvertChanged );
	invert.SetCoord( UI_ITEM_COLUMN, y );
	y += UI_ROW_STEP;

	vibration.szName = L( "Vibration" );
	vibration.onChanged = CMenuEditable::WriteCvarCb;
	vibration.SetCoord( UI_ITEM_COLUMN, y );
	y += UI_ROW_STEP;

	autoaim.szName = L( "GameUI_AutoAim" );
	autoaim.onChanged = CMenuEditable::WriteCvarCb;
	autoaim.SetCoord( UI_ITEM_COLUMN, y );
	y += UI_ROW_STEP + UI_GROUP_STEP;

	SetButtonTop( y );

	AddItem( banner );
	AddItem( layout );
	AddItem( look );
	AddItem( invert );
	AddItem( vibration );
	AddItem( autoaim );
	AddButton( L( "Done" ), NULL, PC_DONE, VoidCb( &CMenuController::SaveAndPopMenu ));

	vibration.LinkCvar( "vibration_enable" );
	autoaim.LinkCvar( "sv_aim" );
}

void CMenuController::_VidInit()
{
	const float yaw = EngFuncs::GetCvarFloat( "joy_yaw" );
	const float pitch = EngFuncs::GetCvarFloat( "joy_pitch" );

	look.SetCurrentValue( fabs( yaw ));
	// a zero in either cvar would multiply the other one away and leave the page no way back, and a
	// negative yaw would turn the ratio negative and flip the pitch against the checkbox
	pitchRatio = ( yaw && pitch ) ? fabs( pitch ) / fabs( yaw ) : 0.75f; // the shipped 165 over 220
	invert.bChecked = pitch < 0.0f;
}

void CMenuController::LookChanged()
{
	const float yaw = look.GetCurrentValue();

	EngFuncs::CvarSetValue( "joy_yaw", yaw );
	EngFuncs::CvarSetValue( "joy_pitch", ( invert.bChecked ? -yaw : yaw ) * pitchRatio );
}

void CMenuController::InvertChanged()
{
	const float pitch = fabs( EngFuncs::GetCvarFloat( "joy_pitch" ));

	EngFuncs::CvarSetValue( "joy_pitch", invert.bChecked ? -pitch : pitch );
}

ADD_MENU( menu_controller, CMenuController, UI_Controller_Menu );
