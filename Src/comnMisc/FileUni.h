/////////////////////////////////////////////////////////////////////////////
//
//  FileUni.h   -    generic file class for MP3 decodeer
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_FILEUNI_H__3BE16179_C52D_455D_8280_F143285AD40F__INCLUDED_)
#define AFX_FILEUNI_H__3BE16179_C52D_455D_8280_F143285AD40F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////


class   FileUni  
{

public:
	FileUni();
	virtual ~FileUni();


//	int     sbinfile::open(   const char *name,    int type   );
	int     open(   const char *name,    int type   );

	int     close();

	int     length();  


//	boolm       eof() { return ioctl(ioctlreof); }
	int             eof();


//	binfilepos binfile::seek(   binfilepos  p     )
	int                      seek(             int  pos  );



//	binfilepos   binfile::read(  void *buf,    binfilepos len)
	int		                   read(   void  *buf,             int len   );


//	binfilepos   binfile::peek(  void *buf,   binfilepos len  )
	int		                   peek(   void  *buf,            int len   );



//	binfilepos  seekcur(  binfilepos pos  )
	int             seekcur(  int  pos  );


	int             seekend(  int  pos  );



	void	   Set_Virtual_Mode_On(   int  offset,     int  virtLength   );

	int         Get_File_Position_Virtual();

	int         Get_File_Position_Actual();






public:
	CFile   m_file;


	bool   m_inVirtualMode;


	int     m_virtualOffset;

	int     m_virtualLength;
};





/////////////////////////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_FILEUNI_H__3BE16179_C52D_455D_8280_F143285AD40F__INCLUDED_)
