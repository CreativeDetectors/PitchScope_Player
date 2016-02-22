/////////////////////////////////////////////////////////////////////////////
//
//  FontsCd.h   -   
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




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class  FontsCd
{

public:
	FontsCd(void);
	~FontsCd(void);


	virtual	long			Get_Characters_Width(    short  scalePitch,    short  userPrefMusicalKeyAccidentals   );


	virtual	void			Calc_NoteText_BoundBox_and_xOffset(   short  scalePitch,    short userPrefMusicalKeyAccidentals,    long& retXoffsetSrc,   
																																			long& retWidth,   long&  retHeight  );



	virtual	bool			Blit_Musical_Notes_NameText(   short scalePitch,   short userPrefMusicalKeyAccidentals,     long xDest,  long yDest,    
																																 bool doXOR,  	CDC& dc,    CString&  retErrorMesg   );


	virtual	bool			Change_To_New_Font(   short fontID   );



public:
	CBitmap   m_fontsBitmap;    // ***************

	short		   m_height;


	LongIndexed   *fontOffsetsTable;  

};
