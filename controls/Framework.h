/*
Framework.h -- base menu fullscreen root window
Copyright (C) 2017 a1batross

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include "BaseWindow.h"
#include "PicButton.h"
#include "Primitive.h"

#define MAX_FRAMEWORK_PICBUTTONS 16

// the pad buttons a page or a dialog answers to, at the bottom right of the gamepad UI; NULL hides one
void UI_DrawLegend( const char *a, const char *x_label, const char *b );

/*
 * WON-style menu framework
 */
class CMenuFramework : public CMenuBaseWindow
{
public:
	typedef CMenuBaseWindow BaseClass;

	CMenuFramework( const char *name = "Unnamed Framework" );
	virtual ~CMenuFramework() override;

	void Show() override;
	void Draw() override;
	void Init() final override;
	void VidInit() final override;
	void Hide() override;
	bool IsRoot() const override { return true; }

	bool KeyDown( int key ) override;

	CMenuPicButton *AddButton( const char *szName, const char *szStatus,
		EDefaultBtns iButton, CEventCallback onReleased = CEventCallback(), int iFlags = 0 );

	CMenuPicButton *AddButton( const char *szName, const char *szStatus,
		const char *szButtonPath, CEventCallback onReleased = CEventCallback(), int iFlags = 0, int hotkey = 0 );

	void RealignButtons();

	// where the page's own buttons start, so a Done can sit under the controls rather than above them
	void SetButtonTop( int y ) { m_iBtnTop = y; RealignButtons(); }

	// how much room the gamepad UI's status column has before it would run into something the page draws
	void SetStatusWidth( int w ) { m_iStatusWidth = w; }

	bool DrawAnimation() override;

	void PrepareBannerAnimation( EAnimation direction, CMenuPicButton *initiator );

	// the pad buttons the page answers to in the gamepad UI (menu strings; NULL hides one). Pages change
	// them with their state
	const char *legendA, *legendB, *legendX;

	class CMenuBannerBitmap : public CMenuBaseItem
	{
	public:
		CMenuBannerBitmap();
		void Draw() override;
		void SetPicture( const char *pic );

		void Draw( Point pt, Size sz );

	private:
		CImage image;
	} banner;

protected:
	void DrawLegend();

	EAnimation bannerAnimDirection;
	Rect bannerRects[2];

	CMenuPicButton *m_apBtns[MAX_FRAMEWORK_PICBUTTONS];
	int m_iBtnsNum;
	int m_iBtnTop;
	int m_iStatusWidth;
};

#endif // FRAMEWORK_H
