/////////////////////////////////////////////////////////////////////////////
//
//  AboutDialogRelease.h   -   
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





//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   AboutDialogRelease   :  public  CDialog
{


	DECLARE_DYNAMIC(AboutDialogRelease)


public:
	AboutDialogRelease(   CString&  editionNumber,   CWnd* pParent = NULL   );   // standard constructor
	virtual ~AboutDialogRelease();




public:
	CString   m_editionNumberStr;





// Dialog Data
	enum { IDD = IDD_ABOUT_DIALOG_RELEASE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
