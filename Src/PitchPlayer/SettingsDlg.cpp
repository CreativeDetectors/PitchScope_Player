/////////////////////////////////////////////////////////////////////////////
//
//  SettingsDlg.cpp   -   
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



#include   "..\comnFacade\UniEditorAppsGlobals.h"


//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"      //  #include <FixMath.h>    ...Mac's version of Fixedpoint 
#include  "..\ComnGrafix\CommonGrafix.h"     	
//////////////////////////////////////////////////     



//  #include   "..\ComnGrafix\AnimePacket.h"

#include   "..\ComnGrafix\OffMap.h" 

//  #include  "..\comnGrafix\TransformMap.h"


#include "..\comnInterface\DetectorAbstracts.h"    // **** NEW ****


/****
#include   "..\ComnGrafix\FrameArtist.h"  



///////////////////////////							 ...INTERFACE
#include  "..\comnInterface\gnStatevar.h"
#include  "..\comnInterface\gnUniBasic.h"

#include  "..\comnInterface\gnVuCues.h"	 


#include  "..\comnInterface\gnPane.h"   
#include  "..\ComnGrafix\PaneMemory.h"
#include   "..\comnInterface\ListComponentView.h"


#include  "..\comnInterface\gnProperty.h"     
#include  "..\comnInterface\gnManipulators.h"
#include  "..\comnInterface\gnCommands.h"
#include  "..\comnInterface\ClipBoardj.h"


#include  "..\comnInterface\gnView.h"
#include  "..\comnInterface\gnEditor.h"   	  


#include   "..\comnInterface\ListComponentSubject.h"      
///////////////////////////



#include  "..\comnInterface\UniWindow.h"	   //   ******  TAKE OUT ******   Too Platform-Specific for this class  5/07
*****/



///////////////////////////
///////////////////////////
#include  <mmsystem.h>   // Is good on Sparky  ( ProgramFiles\Microsoft Visual Studio\VC\PlatformSDK\Include )


#include "..\ComnAudio\dsoundJM.h"        //  I copied it in, bigger than the VC++ version



//#include   "..\comnAudio\ReSampler.h"
//#include  "..\ComnAudio\FFTslowDown.h"  


#include  "..\ComnAudio\CalcNote.h"

#include   "..\ComnAudio\SPitchCalc.h"



#include  "..\ComnAudio\EventMan.h"      //  My encapsulating class for Direct Sound



#include  "..\comnAudio\BitSourceAudio.h"
///////////////////////////
///////////////////////////


#include  "..\ComnAudio\PlayBuffer.h"

#include  "..\ComnAudio\AudioPlayer.h" 



#include  "..\ComnFacade\SoundHelper.h"   





#include "PitchPlayer.h"

#include "SettingsDlg.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(SettingsDlg, CDialog)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////





SoundHelper&      GetSoundHelper();  



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


SettingsDlg::SettingsDlg(  CWnd* pParent /*=NULL*/  )    :  CDialog(  SettingsDlg::IDD, pParent  )
{

	m_testZeroEdit =   0;

	m_testOneEdit  =   -1;
	m_testTwoEdit  =   -1;

	m_test3Edit   =   -1;

	m_test4_Edit =   -1;


	m_dialogIsInitialized =   false;
}


					/////////////////////////////////////////////////


SettingsDlg::~SettingsDlg()
{

}



					/////////////////////////////////////////////////


BOOL SettingsDlg::OnInitDialog()
{

	CString   retErrorMesg;


	/****
	if(   m_hWnd  ==  NULL   )
	{
		ASSERT( 0 );
		AfxMessageBox(   retErrorMesg  ); 
		return  FALSE;
	}
	else
	{  if(     ! Get_EventMan().Init_DSound(   m_hWnd,    retErrorMesg  )     )  
		{
			AfxMessageBox(   retErrorMesg  ); 
			return  FALSE;
		}


		short  channelID =  0;  

		if(    ! GetSoundHelper().Alloc_Midi_Sequencer(   channelID,   retErrorMesg   )     )
		{

			AfxMessageBox(  retErrorMesg  );
	//		return  FALSE;    //  want to escape ????  Is a Midi device a requirement?   Is one always present on Win 98+ ma
		}

	}


	if(   !   m_gotoFilesStartButton.AutoLoad(     IDC_GO_FILESTART_BUTTON,     this  )    )
		AfxMessageBox(  "PitchPlayerDlg::OnInitDialog failed, could not AutoLoad Files Start bitmap."  );

	VERIFY(     m_gotoFilesEndButton.AutoLoad(       IDC_GO_FILEEND_BUTTON,       this  )    );
	VERIFY(     m_playReverseButton.AutoLoad(     IDC_REVERSE_PLAY_BUTTON,     this  )    );
	VERIFY(     m_playForwardButton.AutoLoad(     IDC_CONTINUE_PLAY_BUTTON,     this  )    );
	VERIFY(     m_pausePlayButton.AutoLoad(        IDC_PAUSE_BUTTON,                   this  )    );
	****/


//  	playerApp.m_detectionSensitivityThreshold =   m_testZeroEdit; 


	/***
	SoundMan&   soundMan =     Get_SoundMan();

	m_midiInstrumentPatch  =     soundMan.m_midiInstrumentPatch;
	****/





	CDialog::OnInitDialog();


	/***
		BIG!!!!!  To get a spin control installed,     8/07

			1)  Create an CEdit window next to a SpinControl
			2)  Set  AutoBuddy and SetBuddyInteger to true on Spin control's property page
			3)  Set up the TAB Order so that the SpinControl's number is one more that the CEdit(buddy) windows numbder
			4)  Create a variable(control ) for the spin control
			5)  Do NOT create a control for the buddy window, create a new VARIABLE as short
			6)  Install a  message map command receiver for the BuddyWindow(CEdit)  with the  ON_EN_CHANGE()   macro
			7)  Just ape the code here to get the updates
	***/

	m_dialogIsInitialized =   true;




	return TRUE;  // return TRUE  unless you set the focus to a control
}



					/////////////////////////////////////////////////

void  SettingsDlg::DoDataExchange(CDataExchange* pDX)
{


	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_DETECTION_DETAIL_EDIT, m_testZeroEdit);
	//	DDV_MinMaxInt(pDX, m_testZeroEdit, 1, 255);

	DDX_Text(pDX,   IDC_TEST1_EDIT,   m_testOneEdit  );
	DDX_Text(pDX,   IDC_TEST2_EDIT,   m_testTwoEdit  );
	DDX_Text(pDX,   IDC_TEST3_EDIT,   m_test3Edit     );
	DDX_Text(pDX,   IDC_TEST4_EDIT,   m_test4_Edit  );

}



					/////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(SettingsDlg, CDialog)
END_MESSAGE_MAP()

					/////////////////////////////////////////////////


/****
void     SettingsDlg::Update_Instruments_Name_Text() 
{


	CString    instrumentName =    "Unknown" ;



	if(  m_dialogIsInitialized  )
	{

		UpdateData(  TRUE  );    //  get the new value to  'm_midiInstrumentPatch'    variable



		if(        m_midiInstrumentPatchCEdit   <    0  
			 ||   m_midiInstrumentPatchCEdit   >   95  )
		{
			m_midiInstrumentPatchCEdit =  41;  //   default,  viola
		}


//		instrumentName =    patchNames[   m_midiInstrumentPatchCEdit  ]; 

		if(    ! GetSoundHelper().Get_Midi_Instrument_Text_Name(   m_midiInstrumentPatchCEdit,    instrumentName  )      )
		{
			AfxMessageBox (  instrumentName  );    //  holds an error message
		}



		m_midiInstrumentsNameStatic.SetWindowText(   instrumentName   );

		m_midiInstrumentsNameStatic.UpdateWindow();    //  to force the immediate RENDER
	}
}



					/////////////////////////////////////////////////


void SettingsDlg::On_Change_MidiInstrumentEdit()
{
	Update_Instruments_Name_Text();
}
*****/