/////////////////////////////////////////////////////////////////////////////
//
//  PitchPlayer.cpp   -     Main startup module  for   PitchScope  Player 
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"


#include  "Mmsystem.h"     //  for MIDI        Version 4.00    from the  SDK(careful)



#include    "..\comnFacade\UniEditorAppsGlobals.h"
#include    "..\comnFacade\VoxAppsGlobals.h"



//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"     
#include  "..\ComnGrafix\CommonGrafix.h"  

#include  "..\ComnGrafix\OffMap.h"  
#include  "..\comnGrafix\TransformMap.h"


#include  "..\comnGrafix\FontsCd.h" 
#include  "..\comnInterface\Gauge.h"  

#include "..\comnInterface\DetectorAbstracts.h"   

//////////////////////////////////////////////////    



///////////////////////////

#include  "..\ComnAudio\dsoundJM.h"      


#include   "..\ComnAudio\CalcNote.h"
#include   "..\ComnAudio\SPitchCalc.h"


#include  "..\ComnAudio\EventMan.h"   

#include  "..\comnAudio\BitSourceAudio.h"

///////////////////////////



#include  "..\ComnFacade\SoundHelper.h"   

#include  "..\comnFacade\PitchPlayerApp.h"



#include "PitchPlayer.h"

#include "PitchPlayerDlg.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


PitchPlayerAppMS   theApp;      //   The one and only PitchPlayerAppMS object

SoundHelper         soundHelperGLB;

EventMan              waveManGLB(    NUMPLAYeventsPitchPLAYER,     &(  soundHelperGLB.m_midiSequencer  )      );   
														//  do NOT use  'MAXnUMPLAYEVENTS',  it just creates arrays' in EventMan
													   //   We can fine tune the amout of notifications, specifically for EACH APP, with this parm to EventMan   3/11

PitchPlayerApp      pitchPlayerAppGLB;




//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


EventMan&         Get_EventMan()        
{   return   waveManGLB;   
}

SoundHelper&      GetSoundHelper()        
{   return   soundHelperGLB;   
}


UniApplication&      Get_UniApp()            //	 HIGH-level( abstract )   PARENT ( most generic )			(   GLOBAL function 
{
	return   pitchPlayerAppGLB;
}

PitchPlayerApp&     Get_PitchPlayerApp()     //	  UniApplication's  subclass for this particular app		(   GLOBAL function 
{
	return   pitchPlayerAppGLB;
}


HINSTANCE    Get_Modules_hInstance()  
{    
	return  theApp.m_hInstance;   
}





void    Animate_All_Viewjs_PitchPlayer_GLB(   long  curSample,    short curScalePitch    );


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


PitchPlayerAppMS::PitchPlayerAppMS()
{

	EnableHtmlHelp();    //  Need install this BY HAND  this for   HTML Help    6/2012      [  careful NOT to use   QS_ALLEVENTS    in  MsgWaitForMultipleObjects(), bad for Help   ] 
}



											////////////////////////////////////////


BOOL PitchPlayerAppMS::InitInstance()
{
									//  PitchPlayerAppMS   initialization


	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);


	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);


	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));



	PitchPlayerDlg   dlg;

	m_pMainWnd =   &dlg;
//	HWND    hWinDesktop =    GetDesktopWindow();      //  this works OK on my other test program    12/09



	INT_PTR  nResponse  =    dlg.DoModal();  
	if(           nResponse ==  IDOK  )
	{
		int  dummy =   9;
	}
	else if (nResponse ==  IDCANCEL)
	{
		int  dummy =   9;
	}



	GetSoundHelper().Release_AudioPlayer();     


			// Since the dialog has been closed, return FALSE so that we exit the application, rather than start the application's message pump.
	return FALSE;
}



											////////////////////////////////////////

BEGIN_MESSAGE_MAP(PitchPlayerAppMS, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

											////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////   Ugly  GLOBALS    //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////


short    Get_PitchScope_App_Code_GLB()
{

	short   appCode =   0;       //    0:  Player               1:  Navigator      2: VoxSep     3:   PitchScope              
	return  appCode;
}



				////////////////////////////////////
	

PitchPlayerDlg*     Get_PitchPlayers_Dialog_GLB()
{

	CWnd  *windPtr     =     theApp.m_pMainWnd;
	if(         windPtr  ==  NULL  )
	{
		AfxMessageBox(  "Get_PitchPlayers_Dialog_GLB FAILED,  m_pMainWnd is NULL."   );
		return  NULL;
	}

	PitchPlayerDlg   *dialogPtr     =       dynamic_cast< PitchPlayerDlg* >(  windPtr  );  
	if(   dialogPtr  ==  NULL   )
	{
		AfxMessageBox(  "Get_PitchPlayers_Dialog_GLB FAILED,  could not do dynamic cast."   );
		return  NULL;
	}

	return   dialogPtr;
}


								////////////////////////////////////////


void    Animate_All_Viewjs_PitchPlayer_GLB(   long  curSample,    short curScalePitch    )   
{

	PitchPlayerDlg  *dialogPtr     =      Get_PitchPlayers_Dialog_GLB();
	if(                     dialogPtr  ==  NULL  )
	{  ASSERT( 0 );
		return;
	}


	long   panesSampleCount =   0;    //  dummy parm,  at least for now

	dialogPtr->Sync_FileSlider_Control(    panesSampleCount,   curSample   );
}


								////////////////////////////////////////


void    Draw_LastFrame_Bullets_Gagues_PitchPlayer_GLB()   
{

	PitchPlayerDlg   *dialogPtr     =      Get_PitchPlayers_Dialog_GLB();
	if(                      dialogPtr  ==  NULL  )
	{  ASSERT( 0 );
		return;
	}

	dialogPtr->Render_Last_Amimation_Frame();
}



								////////////////////////////////////////

void  Begin_Wait_Cursor_GLB()
{

	CWnd   *mainFrame  =      AfxGetApp()->m_pMainWnd;

	if(    mainFrame  !=  NULL    )
		mainFrame->BeginWaitCursor();	
}


								////////////////////////////////////////

void  End_Wait_Cursor_GLB()
{

	CWnd   *mainFrame  =      AfxGetApp()->m_pMainWnd;

	if(    mainFrame  !=  NULL    )
		mainFrame->EndWaitCursor();	
}


								////////////////////////////////////////
								////////////////////////////////////////

//  Dummy Globals to keep compiler happy.....

void        Begin_ProgressBar_Position_GLB(  char  *text   )  //   in  SPitchListWindow.cpp  
{
	int  dummy =  9;
}

void        Set_ProgressBar_Position_GLB(   long  posInPercent   )
{
}

void        End_ProgressBar_Position_GLB()
{
}


void   ReDraw_DrivingView_OnDialog_GLB(   bool  renderWithoutText   )
{
		//   ****	 DUMMY function,  to keep the compiler and linker happy.     ***  Only really for  Navigator    12/11  
}


void    Write_To_StatusBar_GLB(   CString&   mesg   )
{
		//    *** DUMMY to keep compiler happy.  Do I want this for Player ????   3/2012
}


