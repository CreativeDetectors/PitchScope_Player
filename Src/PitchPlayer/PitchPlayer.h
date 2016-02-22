/////////////////////////////////////////////////////////////////////////////
//
//  PitchPlayer.h   -    Main startup module  for   PitchScope  Player 
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////

#pragma once


#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif




#include   "resource.h"		// main symbols


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


class    PitchPlayerAppMS    :  public  CWinApp
{

public:
	PitchPlayerAppMS();


	bool		Fetch_Edition_Number(   CString&   retSerialNumber   ); 




// Overrides
public:
	virtual   BOOL InitInstance();



// Implementation
	DECLARE_MESSAGE_MAP()
};



extern  PitchPlayerAppMS   theApp;