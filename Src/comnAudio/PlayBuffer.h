/////////////////////////////////////////////////////////////////////////////
//
//  PlayBuffer.h   -   
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_PLAYBUFFER_H__A50DF3A1_4555_11D6_9A48_00036D156F73__INCLUDED_)
#define AFX_PLAYBUFFER_H__A50DF3A1_4555_11D6_9A48_00036D156F73__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
///////////////////////////////////////////////////////////////



class  EventMan;

class  BitSource;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   PlayBufferStatic  
{

public:
	PlayBufferStatic(   long  memorySize,    long  speedSlowDown,    EventMan&  eventMan   );
	virtual  ~PlayBufferStatic();


	virtual    bool			Is_OK()       {   return   m_isOK;   }

	virtual    void			Release_DSoundBuffer();


	virtual    bool			Is_Playing();

	virtual    bool			Set_Speed(   int   slowDownRatio,   long  numSrcBytes,     CString&  retErrorMesg   );



	virtual    bool			Fill_With_Silence();

	virtual    bool			Resize_Memory(    long   byteCount,          CString&  retErrorMesg    );




	virtual    bool         Play_MyBuffer(    char*  bytes,       long  numSrcBytes,     CString&  retErrorMesg    );


	virtual    bool			Play_Chunk(   BitSourceStatic&  bitSource,    long  byteOffset,    long  speedSlowDown,   CString&  retErrorMesg    );









public:
	IDirectSoundBuffer   *m_DSoundBuffer;   //   2nd Buffer   for  'SHORT  play'  *******BUG : 
																 //   sometime looes it, need to automatically ReAllocate it again.  2/03

	WAVEFORMATEX       *m_waveFormat;

	EventMan&                 m_waveMan;


	bool	         m_isOK;

	DWORD       m_totalBytes;

	long            m_slowPlayStep;			//  so far fixed at   4096

	long			m_speedSlowDown;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_PLAYBUFFER_H__A50DF3A1_4555_11D6_9A48_00036D156F73__INCLUDED_)
