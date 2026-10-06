/*
Framework.cpp -- base menu fullscreen root window
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
#include "Framework.h"
#include "PicButton.h"
#include "Utils.h"
#include "FontManager.h"

// menu banners used fiexed rectangle (virtual screenspace at 640x480)
#define UI_BANNER_POSX		72
#define UI_BANNER_POSY		72
#define UI_BANNER_WIDTH		736
#define UI_BANNER_HEIGHT		128

CMenuFramework::CMenuFramework( const char *name ) : BaseClass( name )
{
	memset( m_apBtns, 0, sizeof( m_apBtns ) );
	m_iBtnsNum = 0;
	m_iBtnTop = 230;
	m_iStatusWidth = 1024 - UI_STATUS_COLUMN - 40;
	bannerAnimDirection = ANIM_NO;
	legendA = "Select";
	legendB = "Back";
	legendX = NULL;
}

CMenuFramework::~CMenuFramework()
{
	for( int i = 0; i < m_iBtnsNum; i++ )
	{
		RemoveItem( *( m_apBtns[i] ) );
		delete m_apBtns[i];
		m_apBtns[i] = NULL;
	}
}

void CMenuFramework::Show()
{
	BaseClass::Show();
}

void CMenuFramework::Draw()
{
	static int statusFadeTime;
	static CMenuBaseItem *lastItem;
	CMenuBaseItem *item;

	BaseClass::Draw();

	item = ItemAtCursor();
	// a1ba: maybe this should be somewhere else?
	if( item != lastItem )
	{
		if( item ) item->m_iLastFocusTime = uiStatic.realTime;
		statusFadeTime = uiStatic.realTime;

		lastItem = item;
	}

	// draw status text if status text isn't rendered at right side of an item
	// ideally, framework shouldn't even know about this but since everything
	// WON-styled goes here, let's leave it here...
	if( item && item == lastItem && !FBitSet( item->iFlags, QMF_NOTIFY ) && item->szStatusText != NULL )
	{
		float alpha = bound( 0, ((( uiStatic.realTime - statusFadeTime ) - 100 ) * 0.01f ), 1 );
		int r, g, b, x, y, len;

		EngFuncs::ConsoleStringLen( item->szStatusText, &len, NULL );

		UnpackRGB( r, g, b, uiColorHelp );
		EngFuncs::DrawSetTextColor( r, g, b, alpha * 255 );

		if( uiStatic.gamepadUI )
		{
			// Beside the item it belongs to, in the column the pages keep clear for it - the way the
			// pages that are lists of buttons already say it, and inside the safe area, which the centred
			// line below is not. A line too long for the column is wrapped rather than drawn over
			// whatever the page holds to its right.
			const int charH = EngFuncs::ConsoleCharacterHeight();
			const int room = m_iStatusWidth * uiStatic.scaleX;
			const char *rest = item->szStatusText;
			char line[4][256];
			int count = 0;

			x = UI_STATUS_COLUMN * uiStatic.scaleX;

			while( *rest && count < 4 )
			{
				int take = 0;

				for( int i = 0; ; i++ )
				{
					if( rest[i] && rest[i] != ' ' )
						continue;

					Q_strncpy( line[count], rest, Q_min( (size_t)i + 1, sizeof( line[0] )));
					EngFuncs::ConsoleStringLen( line[count], &len, NULL );

					if( len > room && take )
						break; // the previous word was the last one that fitted

					take = i;

					if( !rest[i] )
						break;
				}

				Q_strncpy( line[count], rest, Q_min( (size_t)take + 1, sizeof( line[0] )));
				rest += take;
				while( *rest == ' ' ) rest++;
				count++;
			}

			y = item->GetRenderPosition().y + item->GetRenderSize().h / 2 - charH * count / 2;

			for( int i = 0; i < count; i++ )
				EngFuncs::DrawConsoleString( x, y + i * charH, line[i] );
		}
		else
		{
			x = ( ScreenWidth - len ) * 0.5f; // centering
			y = uiStatic.yOffset + 720 * uiStatic.scaleY;

			EngFuncs::DrawConsoleString( x, y, item->szStatusText );
		}
	}
	else statusFadeTime = uiStatic.realTime;

	if( uiStatic.gamepadUI && m_pStack->Current() == this ) // a dialog on top has its own buttons
		DrawLegend();
}

/*
=================
UI_DrawLegend

The pad buttons the page answers to, right-aligned at the bottom of the TV safe area (5% in from the
edges): A, then X, then B at the edge, as console menus put them.
=================
*/
void UI_DrawLegend( const char *a, const char *x_label, const char *b )
{
	// a CImage loads when it is made and keeps the handle, so these are made once and not three times a
	// frame - but a video restart leaves the handles pointing at nothing, so they are made again then
	static CImage icons[3] = { CImage( "gfx/shell/pad_a" ), CImage( "gfx/shell/pad_x" ), CImage( "gfx/shell/pad_b" ) };
	static int loaded = uiVidGeneration;

	if( loaded != uiVidGeneration )
	{
		loaded = uiVidGeneration;
		icons[0].Load( "gfx/shell/pad_a" );
		icons[1].Load( "gfx/shell/pad_x" );
		icons[2].Load( "gfx/shell/pad_b" );
	}
	const char *labels[3] = { a, x_label, b };
	const int icon = 36 * uiStatic.scaleY;
	const int charH = UI_SMALL_CHAR_HEIGHT * uiStatic.scaleY;
	const int gap = 8 * uiStatic.scaleX;
	const int y = ScreenHeight - ScreenHeight / 20 - icon;
	int x = ScreenWidth - ScreenWidth / 20;

	for( int i = 2; i >= 0; i-- )
	{
		if( !labels[i] )
			continue;

		const char *text = L( labels[i] );
		const int w = g_FontMgr->GetTextWideScaled( uiStatic.hSmallFont, text, charH );

		x -= w;
		UI_DrawString( uiStatic.hSmallFont, x, y + ( icon - charH ) / 2, w, charH, text, uiPromptTextColor, charH,
			QM_LEFT, ETF_SHADOW | ETF_NOSIZELIMIT | ETF_FORCECOL );
		x -= gap + icon;
		UI_DrawPic( x, y, icon, icon, uiColorWhite, icons[i], QM_DRAWTRANS );
		x -= gap * 4;
	}
}

void CMenuFramework::DrawLegend()
{
	UI_DrawLegend( legendA, legendX, legendB );
}

void CMenuFramework::Hide()
{
	BaseClass::Hide();
}

void CMenuFramework::Init()
{
	BaseClass::Init();
	pos.x = uiStatic.xOffset;
	pos.y = uiStatic.yOffset;
	size.w = uiStatic.width;
	size.h = 768;
}

void CMenuFramework::VidInit()
{
	pos.x = uiStatic.xOffset;
	pos.y = uiStatic.yOffset;
	size.w = uiStatic.width;
	size.h = 768;
	BaseClass::VidInit();
}

CMenuPicButton * CMenuFramework::AddButton(const char *szName, const char *szStatus, EDefaultBtns buttonPicId, CEventCallback onReleased, int iFlags)
{
	if( m_iBtnsNum >= MAX_FRAMEWORK_PICBUTTONS )
	{
		Host_Error( "Too many pic buttons in framework!" );
		return NULL;
	}

	CMenuPicButton *btn = new CMenuPicButton();

	btn->SetNameAndStatus( szName, szStatus );
	btn->SetPicture( buttonPicId );
	btn->iFlags |= iFlags;
	btn->onReleased = onReleased;
	btn->SetCoord( 72, m_iBtnTop + m_iBtnsNum * 50 );
	AddItem( btn );

	m_apBtns[m_iBtnsNum++] = btn;

	return btn;
}

CMenuPicButton * CMenuFramework::AddButton( const char *szName, const char *szStatus, const char *szButtonPath, CEventCallback onReleased, int iFlags, int hotkey )
{
	if( m_iBtnsNum >= MAX_FRAMEWORK_PICBUTTONS )
	{
		Host_Error( "Too many pic buttons in framework!" );
		return NULL;
	}

	CMenuPicButton *btn = new CMenuPicButton();

	btn->SetNameAndStatus( szName, szStatus );
	btn->SetPicture( szButtonPath, hotkey );
	btn->iFlags |= iFlags;
	btn->onReleased = onReleased;
	btn->SetCoord( 72, m_iBtnTop + m_iBtnsNum * 50 );
	AddItem( btn );

	m_apBtns[m_iBtnsNum++] = btn;

	return btn;
}

void CMenuFramework::RealignButtons( void )
{
	for( int i = 0, j = 0; i < m_iBtnsNum; i++ )
	{
		if( !m_apBtns[i]->IsVisible())
			continue;

		m_apBtns[i]->SetCoord( 72, m_iBtnTop + j * 50 );
		m_apBtns[i]->CalcPosition();
		j++;
	}
}

bool CMenuFramework::KeyDown( int key )
{
	bool b = BaseClass::KeyDown( key );

	if( UI::Key::IsEscape( key ))
	{
		const CMenuBaseWindow *newWindow = WindowStack()->Current();

		if( newWindow != nullptr && newWindow != this && newWindow->IsRoot() )
		{
			PrepareBannerAnimation( ANIM_CLOSING, nullptr );
		}
	}

	return b;
}

void CMenuFramework::PrepareBannerAnimation( EAnimation direction, CMenuPicButton *initiator )
{
	// banner is not present, ignore
	if( banner.Parent() != this )
		return;

	bannerAnimDirection = direction;
	if( initiator )
	{
		bannerRects[0].pt = initiator->GetRenderPosition();
		bannerRects[0].sz = initiator->GetRenderSize();

		bannerRects[1].pt = banner.GetRenderPosition();
		bannerRects[1].sz = banner.GetRenderSize();

		banner.szName = initiator->szName;
	}
}

bool CMenuFramework::DrawAnimation()
{
	bool b = CMenuBaseWindow::DrawAnimation( );

	if( bannerAnimDirection != ANIM_NO )
	{
		float frac;

		if( bannerAnimDirection == ANIM_OPENING )
			frac = ( uiStatic.realTime - m_iTransitionStartTime ) / TTT_PERIOD;
		else
			frac = 1.0f - ( uiStatic.realTime - m_iTransitionStartTime ) / TTT_PERIOD;
		Rect r = Rect::Lerp( bannerRects[0], bannerRects[1], frac );

		banner.Draw( r.pt, r.sz );
	}

	if( b )
		bannerAnimDirection = ANIM_NO;

	return b;
}

CMenuFramework::CMenuBannerBitmap::CMenuBannerBitmap()
{
	SetRect( UI_BANNER_POSX, UI_BANNER_POSY, UI_BANNER_WIDTH, UI_BANNER_HEIGHT );
	iFlags |= QMF_INACTIVE; // a heading: neither the pad nor the keyboard stops on it
}

void CMenuFramework::CMenuBannerBitmap::SetPicture(const char *pic)
{
	image.Load( pic );
}

void CMenuFramework::CMenuBannerBitmap::Draw( Point pt, Size sz )
{
	if( image.IsValid() )
	{
		UI_DrawPic( pt, sz, uiColorWhite, image, QM_DRAWADDITIVE );
	}
	else
	{
		UI_DrawString( uiStatic.hHeavyBlur, pt,
			sz,
			szName,
			uiPromptTextColor, m_scChSize,
			QM_LEFT, ETF_ADDITIVE | ETF_NOSIZELIMIT | ETF_FORCECOL );

		UI_DrawString( uiStatic.hLightBlur, pt,
			sz,
			szName,
			uiPromptTextColor, m_scChSize,
			QM_LEFT, ETF_ADDITIVE | ETF_NOSIZELIMIT | ETF_FORCECOL );
	}
}

void CMenuFramework::CMenuBannerBitmap::Draw()
{
	CMenuFramework *pParent = (CMenuFramework *)Parent();

	if (pParent && (pParent->bannerAnimDirection == ANIM_OPENING || pParent->eTransitionType == ANIM_OPENING))
		return;

	Draw( m_scPos, m_scSize );
}

