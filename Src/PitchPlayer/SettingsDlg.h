/////////////////////////////////////////////////////////////////////////////
//
//  SettingsDlg.h   -   
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

#include "afxwin.h"
#include "afxcmn.h"




//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class   SettingsDlg :   public CDialog
{

	DECLARE_DYNAMIC(SettingsDlg)



public:
	SettingsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~SettingsDlg();



public:
	bool      m_dialogIsInitialized;



// Dialog Data
	enum { IDD = IDD_SETTINGS_DIALOG };

protected:
	virtual  void    DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual  BOOL  OnInitDialog();



	DECLARE_MESSAGE_MAP()
public:
	short m_testZeroEdit;
	short m_testOneEdit;
	short m_testTwoEdit;
	short m_test3Edit;
	short m_test4_Edit;
};
