/////////////////////////////////////////////////////////////////////////////
//
//  AboutDialogRelease.cpp   -   
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"




#include "PitchPlayer.h"


#include "dlgAboutBoxRelease.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


IMPLEMENT_DYNAMIC(AboutDialogRelease, CDialog)




//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


AboutDialogRelease::AboutDialogRelease(   CString&  editionNumber,     CWnd* pParent /*=NULL*/  )
	                                                 :  CDialog(  AboutDialogRelease::IDD,  pParent  ),     m_editionNumberStr(   editionNumber   )
{
}



					/////////////////////////////////////////////////


AboutDialogRelease::~AboutDialogRelease()
{
}



					/////////////////////////////////////////////////


void AboutDialogRelease::DoDataExchange(CDataExchange* pDX)
{

	CDialog::DoDataExchange(pDX);

//	DDX_Text(  pDX,     IDC_USER_NAME_STATIC,   m_usersNameStr  );

//	DDV_MaxChars(pDX, m_usersNameStr, 150);

//	DDX_Text(  pDX,     IDC_USERS_EMAIL_STATIC,    m_usersEmailStr  );


	DDX_Text(  pDX,     IDC_EDITION_NUMBER_STATIC,    m_editionNumberStr  );
}




					/////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(AboutDialogRelease, CDialog)
END_MESSAGE_MAP()

					/////////////////////////////////////////////////