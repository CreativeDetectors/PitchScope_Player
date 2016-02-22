/////////////////////////////////////////////////////////////////////////////
//
//  PitchPlayerDlg.cpp   -     A dialog interface for the Control Panel of   PitchScope Player  
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



#include  <math.h>    

#include  "Mmsystem.h"     //  for MIDI    [  or  #include  <mmsystem.h> ??  ]     Version 4.00  from the  SDK(careful)



#include   "..\comnFacade\UniEditorAppsGlobals.h"

#include  "..\comnFacade\VoxAppsGlobals.h"


//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"     
#include  "..\ComnGrafix\CommonGrafix.h"     

#include  "..\ComnGrafix\OffMap.h"  
#include  "..\comnGrafix\TransformMap.h"


#include  "..\comnGrafix\FontsCd.h" 
#include  "..\comnInterface\Gauge.h"   //  ** NEW,  can I move it upward ???   2/12


#include  "..\comnCatalog\External.h"

#include "..\comnInterface\DetectorAbstracts.h"   


#include  "..\comnAudio\SequencerMidi.h"			

//////////////////////////////////////////////////    



///////////////////////////

#include  "..\ComnAudio\dsoundJM.h"        //  I copied it in, bigger than the VC++ version

#include  "..\ComnAudio\CalcNote.h"
#include   "..\ComnAudio\SPitchCalc.h"

#include  "..\ComnAudio\EventMan.h"     



///////////////////////////

#include  "..\comnAudio\BitSourceAudio.h"

#include  "..\ComnAudio\PlayBuffer.h"
#include  "..\ComnAudio\AudioPlayer.h" 


#include  "..\comnFacade\PitchPlayerApp.h"

#include  "..\ComnFacade\SoundHelper.h" 

///////////////////////////




#include  "PitchPlayer.h"

#include "SettingsDlg.h"
#include "dlgAboutBoxRelease.h"

#include "PitchPlayerDlg.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


EventMan&          Get_EventMan();
SoundHelper&      GetSoundHelper();  

UniApplication&      Get_UniApp();
PitchPlayerApp&     Get_PitchPlayerApp();



void     Draw_SolidGray_Oval(      RectShrt  rct,    void *CDevC,    short val    );
void     Draw_OutlineGrey_Oval(   RectShrt  rct,    void *CDevC,    short val,     short  penWidth    );

void     Draw_Solid_Colored_Oval(   RectShrt  rct,    void *CDevC,    short red,  short green,  short  blue  );   



void     Fill_SolidGrey_Rect(    RectShrt&  rect,    short  greyVal,     CDC&  dc    );  
void     Fill_SolidColor_Rect(    RectShrt&  rect,    COLORREF  clr,     CDC&  dc    ); 




//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


PitchPlayerDlg::PitchPlayerDlg( CWnd* pParent /*=NULL*/ )
																			:  CDialog( PitchPlayerDlg::IDD, pParent ),  m_detectionMidiSourceRadio(0)
{

	SoundHelper&         soundHelper  =    GetSoundHelper();  
	PitchPlayerApp&      playerApp =    Get_PitchPlayerApp();


	CString  retErrorMesg;


	m_hIcon =    AfxGetApp()->LoadIcon(  IDR_MAINFRAME  );



    m_notesColorBrighten =   1.8;        //             1.5  ********* ADJUST ******   Makes NOTES Color Brighter     used to be  detectionBoost  
													    //   Want BRIGHTER  colors for Player than  Navigator   2/2012

											

	m_panesWidthRevolver   =   174;     //  160
	m_panesHeightRevolver =    m_panesWidthRevolver;

	m_xOffsetRevolver =    90;     //   80   120    20
	m_yOffsetRevolver =    82;    //      80          90



	/////////////////////	                                            //   Gauge CLASS   

	m_gaugeOfRevolver.m_orientation           =    0;    //      0:  Horizontal     Revolver is always Horizontal
	m_gaugeOfRevolver.m_bulletsShapeCode =   1;     //      0:   Rectangle      1: Oval

	m_gaugeOfRevolver.m_xOffset  =	  m_xOffsetRevolver;	 //  revolversXCenter    -    m_panesWidthRevolver/2;
	m_gaugeOfRevolver.m_yOffset  =	  m_yOffsetRevolver;			 //   revolversYCenter    -   m_panesHeightRevolver/2;

	m_gaugeOfRevolver.m_width     =      m_panesWidthRevolver;
	m_gaugeOfRevolver.m_height    =      m_panesHeightRevolver;         

	m_gaugeOfRevolver.Initialize_BulletList();


	if(    ! Change_FontCd(   m_gaugeOfRevolver.m_fontCustom,   6,     retErrorMesg   )    )
		AfxMessageBox(  retErrorMesg   );
	//////////////////////



	m_revolversBackgroundGray  =   0;

	m_lastScrollPositionAbsoluteHorz =    -1;   //   0

	m_numberOfUnitsForFilePositionSlider =  200;   



	m_defaultDetailSliderPosition =   18;      //  will give  4/7   RATIO on loading new files

	m_detailSliderReduceFactor    =   4;



	m_eventCounterMusicalKey =   0;

	m_userPrefMusicalKeyAccidentals =  0;  // Not really used as much here, as in Navigator 

	m_musicalKeyDetectionState =  0;   //     0:   Disable AUTO-set (user has touched control or a NoteList assigned MKey              
														 //     1:   Init,  waiting to calc key,     2: Now automatically writing



	m_midiInstrumentsControlsValue =    soundHelper.m_midiInstrumentPatch;


	m_detectionMidiSourceRadio =  0;
}




					/////////////////////////////////////////////////


BOOL PitchPlayerDlg::OnInitDialog()
{

	CString   retErrorMesg;

	PitchPlayerApp&     playerApp      =     Get_PitchPlayerApp();
	EventMan&             eventMan      =      Get_EventMan();
	SoundHelper&         soundHelper  =      GetSoundHelper();  
//	StreamingAudioplayer  *audioPlayer  =    soundHelper.m_audioPlayer;  


	if(   m_hWnd  ==  NULL   )
	{
		ASSERT( 0 );
		AfxMessageBox(   "OnInitDialog  FAILED,  windows Handle is NULL."    ); 
		return  FALSE;
	}
	else
	{  if(     ! eventMan.Init_DSound(   m_hWnd,    retErrorMesg  )     )  
		{
			AfxMessageBox(   retErrorMesg  ); 
			return  FALSE;
		}


		short  retDeviceCount =  -1;   

		if(    ! soundHelper.List_All_Midi_Devices(   retDeviceCount,    retErrorMesg   )   )
		{
			AfxMessageBox(  retErrorMesg  );
	//		return  FALSE;    //  want to escape ????  Is a Midi device a requirement?   
		}



		if(    retDeviceCount  <=  0   )
		{
			AfxMessageBox(  "No Midi Devices, including the Microsoft Midi Mapper were found on this computer. \nYou will not be able to hear the detected notes upon the display."  );
		}  
		else
		{	short   channelID =  0;   

			SequencerMidi   *retSequencerMidi =  NULL;   //  The DEFAULT device is the Microsoft Synth ("Microsoft GS Wavetable Synth"),  which should always be allocated first 
			CString   mesg1;
			long        deviceID =  -1;    


			if(    soundHelper.m_deviceIDofInternalSynth  >=  0   )
			{
				deviceID =   soundHelper.m_deviceIDofInternalSynth;    //    0: Best,   ( better than MIDI_MAPPER ) 
			}
			else
			{	deviceID =   MIDI_MAPPER;    // ****  A Lousy ASSUMPTION  ???   But works fine on Lassie   8/2012
				
	//		     AfxMessageBox(   "Could not find the Internal Synthesizer, so going to try to use the MIDI_MAPPER."  );   ** DEBUG only message **
			}



			ASSERT(   soundHelper.m_midiSequencer  ==  NULL   );   //   Should not happen.  Do I need to prepare for this in the future???    1/12


			if(    ! soundHelper.Alloc_Midi_Sequencer(   deviceID,     &retSequencerMidi,    retErrorMesg  )     )
			{
				AfxMessageBox(  retErrorMesg  );
		//		return  FALSE;     Do NOT escape,  Midi device is NOT a strict requirement.  
			}
			else
			{  soundHelper.m_midiSequencer =    retSequencerMidi;      //  sucess, so assign the NEW device to the memberVar


				if(    ! soundHelper.m_midiSequencer->Initialize_New_Midi_Device(  channelID,   mesg1  )     )   //  always call this after   Alloc_Midi_Sequencer()  
				{
			//	    return  false;                    // This is NOT a big deal if it if it fails.  So we still allow this function to return true    1/12
					AfxMessageBox(   mesg1  );
				}
			

				if(       soundHelper.m_deviceIDofInternalSynth  >=  0   
					||    deviceID ==   MIDI_MAPPER       )
				{
					if(    ! soundHelper.m_midiSequencer->Change_Instrument(   soundHelper.m_midiInstrumentPatch,    retErrorMesg   )      )  
						return  false;
				}
			}
		}   //   retDeviceCount >= 1
	}



	if(   !   m_gotoFilesStartButton.AutoLoad(     IDC_GO_FILESTART_BUTTON,     this  )    )
		AfxMessageBox(  "PitchPlayerDlg::OnInitDialog failed, could not AutoLoad Files Start bitmap."  );

	VERIFY(     m_gotoFilesEndButton.AutoLoad(       IDC_GO_FILEEND_BUTTON,       this  )    );
	VERIFY(     m_playReverseButton.AutoLoad(     IDC_REVERSE_PLAY_BUTTON,     this  )    );
	VERIFY(     m_playForwardButton.AutoLoad(     IDC_CONTINUE_PLAY_BUTTON,     this  )    );
	VERIFY(     m_pausePlayButton.AutoLoad(        IDC_PAUSE_BUTTON,                   this  )    );





	CDialog::OnInitDialog();   // ************************



	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon


	Enable_Detection_Controls(  true  );


	m_curPlayPositionSliderCtrl.SetRange(  0,   m_numberOfUnitsForFilePositionSlider   );   

	m_curPlayPositionSliderCtrl.SetPos(  0  );



	CString   strText,    instrumentNameText,   fullText;
	short  sensitivitySetting   =     playerApp.m_detectionSensitivityThreshold;    //  /  m_sensitivitySliderIncrease;     =  1
	m_sensitivitySlider.SetPos(   sensitivitySetting  );

	strText.Format(   "%d",   101  -  playerApp.m_detectionSensitivityThreshold    );
//	strText.Format(   "%d",   playerApp.m_detectionSensitivityThreshold  );

	m_sensitivityCStatic.SetWindowText(   strText   );



	short   rightStereoPercent   =    playerApp.m_rightChannelPercent;			//  stereo-balance  Slider

	if(        rightStereoPercent  <  0   )
		rightStereoPercent =      0;
	else if(  rightStereoPercent > 100  )
		rightStereoPercent =   100;

	m_stereoBalanceDetection.SetPos(  rightStereoPercent  );



	int    curWavVolume    =          playerApp.m_wavsVolumeApp;        	//  wav's volumn   Slider
	if(         curWavVolume   >=      0   
		 &&   curWavVolume   <=   100    )
	{
		short  setting =     curWavVolume;     //     100  -  curWavVolume;
		ASSERT(  setting >= 0    &&      setting  <= 100 );

		m_wavsVolumnSliderCtrl.SetPos(  setting  );
	}
	else
	{  ASSERT( 0 );
		m_wavsVolumnSliderCtrl.SetPos(  0  );
	}



	int    curMidiVolume    =          playerApp.m_midiVolume;        	//  MIDI   volumn   Slider
	if(         curMidiVolume   >=      0   
		 &&   curMidiVolume   <=   100    )
	{
		short  setting =     curMidiVolume;     //     100  -  curWavVolume;
		ASSERT(  setting >= 0    &&      setting  <= 100 );

		m_midiVolumnSliderCtrl.SetPos(  setting  );
	}
	else
	{  ASSERT( 0 );
		m_midiVolumnSliderCtrl.SetPos(  0  );
	}



	m_detailSliderCtrl.SetPos(  m_defaultDetailSliderPosition  );

	Update_Detail_Controls_StaticText();



	m_midiInstrumentSpinCtrl.SetRange(  0,  95  );      
	m_midiInstrumentSpinCtrl.SetPos(  m_midiInstrumentsControlsValue  );   

	if(    ! soundHelper.Get_Midi_Instrument_Text_Name(   m_midiInstrumentsControlsValue,    instrumentNameText  )      )
		AfxMessageBox (  instrumentNameText  );    //  holds an error message
	else
	{  fullText.Format(  "%d   %s",           m_midiInstrumentsControlsValue,   instrumentNameText    );
		m_midiInstrumentNameStatic.SetWindowText(   fullText   );
	}



	m_boostSourceCheckBox.SetCheck(  FALSE  );    //   FALSE is default  
//	long   scaleInPercent  =     eventMan.Set_VolumeBoost_Factor(  0  );	    NO...  we need a BitSource for this, and no file is loaded. 


	Set_MidiSync_Controls_Position();


	return TRUE;  // return TRUE  unless you set the focus to a control
}



						////////////////////////////////////////


void   PitchPlayerDlg::Restore_Default_Settings_to_Controls(   SPitchCalc&   sPitchCalcer   )
{

									//   Call this AFTER the LOAD of a File   ( .WAV,  .MP3,  or  NOTELIST  )       12/11

	float    defaultPlaySpeed =  1;


	PitchPlayerApp&   playerApp      =   Get_PitchPlayerApp();
	SoundHelper&      soundHelper  =    GetSoundHelper();  
	SequencerMidi    *midiSequencer =			soundHelper.Get_Current_MidiSequencer();

	CString	  retErrorMesg,   mesgText,   strText,   fullText;


																						//    INITIALIZE all variables to DEFAULT values
	soundHelper.m_userFinalCircqueSizeTweak     =   0;
	soundHelper.m_userFCircqSizeTweakMultiplier =   1;      


	m_detectionMidiSourceRadio =  0;   
	Enable_Detection_Controls(  true  );


	playerApp.m_rightChannelPercent =   50;
	m_stereoBalanceDetection.SetPos(  50  );    //  always set it for balanced( 50%) stero on load of a new file


	playerApp.m_detectionSensitivityThreshold  =   31;   //  default

	short  detectionSensitivity   =     playerApp.m_detectionSensitivityThreshold;    //     /  m_sensitivitySliderIncrease =  1      	//   29:  playerApp.m_detectionSensitivityThreshold 
	m_sensitivitySlider.SetPos(   detectionSensitivity  );

//	strText.Format(   "%d",               playerApp.m_detectionSensitivityThreshold   );   
	strText.Format(   "%d",   101 -    playerApp.m_detectionSensitivityThreshold  );

	m_sensitivityCStatic.SetWindowText(   strText   );



	m_muteMidiCheckBox.SetCheck(  FALSE  );    //   FALSE is default  

	if(    midiSequencer  !=  NULL   )
		midiSequencer->m_silenceMidi =   false;    // false :   dfault


/***********************************************  MOVED Up,  OK ????
	m_detectionMidiSourceRadio =  0;   
	Enable_Detection_Controls(  true  );
****/


	short   musicalKeyDefault =  0;

	sPitchCalcer.m_musicalKey =    musicalKeyDefault;

	m_userPrefMusicalKeyAccidentals =   0;      //   NEW,    0:  No preverence     1:  Use Sharps    2:  UseFlats

	m_musicalKeyDetectionState =   0;  

	


	 m_boostSourceCheckBox.SetCheck(  FALSE  );    //   FALSE is default  

	long   scaleInPercent  =    soundHelper.Set_VolumeBoost_Factor(  0  );	   //  0:   Default seeting at  500 



	playerApp.m_sizeOfPrimNotesCircque  =   7;       //      *** This must be done AFTER the above INIT of sPitchCalcer  ***
	playerApp.m_numberNeededToMatch  =   4;       //      These are default values that  SPitchCalc::SPitchCalc() will also init

	m_detailSliderCtrl.SetPos(               m_defaultDetailSliderPosition  );  

	Set_SPitchCalcs_Detail_Threshold(   m_defaultDetailSliderPosition  );  



	Set_MidiSync_Controls_Position();   //  BETTER SPOT down here,   Set_SPitchCalcs_Detail_Threshold() calls ReAllocate_Players_AudioDelay_MemBuffer()
														//                                    which now CORRECTLY sets the MemberVars for  Set_MidiSync_Controls_Position().    8/7/12 


	UpdateData(  false  );   
}




					/////////////////////////////////////////////////


void PitchPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(  pDX,  IDC_curPLAY_POSITION_SLIDER,   m_curPlayPositionSliderCtrl  );
	DDX_Control(pDX, IDC_SENSITIVITY_SLIDER, m_sensitivitySlider);
	DDX_Control(pDX, IDC_WAV_VOLUME_SLIDER, m_wavsVolumnSliderCtrl);
	DDX_Control(pDX, IDC_DETECT_STEREO_BALANCE_SLIDER, m_stereoBalanceDetection);
	DDX_Control(pDX, IDC_DETAIL_SLIDER1, m_detailSliderCtrl);
	DDX_Control(pDX, IDC_MIDI_VOLUMN_SLIDER, m_midiVolumnSliderCtrl);
	DDX_Control(pDX, IDC_SENSITIVITY_STATIC, m_sensitivityCStatic);
	DDX_Control(pDX, IDC_DETAIL_STATIC, m_detailCStatic);
	DDX_Control(pDX, IDC_MIDI_INSTRUMENT_TEXT_STATIC, m_midiInstrumentNameStatic);
	DDX_Control(pDX, IDC_MIDI_INSTRUMENT_SPIN, m_midiInstrumentSpinCtrl);
	DDX_Control(pDX, IDC_MUTE_MIDI_PLAY_CHECKBOX, m_muteMidiCheckBox);
	DDX_Control(pDX, IDC_BOOST_SOURCE_CHECKBOX, m_boostSourceCheckBox);
	DDX_Radio(     pDX,   IDC_MIDIsource_DETECTION_RADIO,    m_detectionMidiSourceRadio             );    // had to install by HAND    11/11
	DDX_Control(  pDX,    IDC_MIDIsource_DETECTION_RADIO,  m_detectionMidiSourceRadioControl   );
	DDX_Control(pDX, IDC_MIDI_SYNC_SLIDER, m_midiSyncSliderCtrl);
	DDX_Control(pDX, IDC_MIDI_SYNC_VALUE_STATIC, m_midiSyncValueStatic);
}


					/////////////////////////////////////////////////


BEGIN_MESSAGE_MAP(PitchPlayerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_GO_FILESTART_BUTTON, &PitchPlayerDlg::OnGoFilesStartButton)
	ON_BN_CLICKED(IDC_REVERSE_PLAY_BUTTON, &PitchPlayerDlg::OnReversePlayButton)
	ON_BN_CLICKED(IDC_PAUSE_BUTTON, &PitchPlayerDlg::OnPausePlayButton)
	ON_BN_CLICKED(IDC_CONTINUE_PLAY_BUTTON, &PitchPlayerDlg::OnContinuePlayButton)
	ON_BN_CLICKED(IDC_GO_FILEEND_BUTTON, &PitchPlayerDlg::OnGoFilesEndButton)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_curPLAY_POSITION_SLIDER, &PitchPlayerDlg::OnNMReleasedCapture_curplay_PositionSlider)
	ON_COMMAND(ID_FILE_OPEN_mainDialog, &PitchPlayerDlg::On_File_Open_WAV)
	ON_COMMAND(ID_HELP_ABOUTPITCHSCOPEPLAYER, &PitchPlayerDlg::On_About_PitchscopePlayer_help)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SENSITIVITY_SLIDER, &PitchPlayerDlg::On_Releasedcapture_Sensitivity_Slider)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_WAV_VOLUME_SLIDER, &PitchPlayerDlg::On_Releasedcapture_Wavs_Volume_Slider)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_DETECT_STEREO_BALANCE_SLIDER, &PitchPlayerDlg::On_Releasedcapture_Detect_Stereo_Balance_Slider)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_DETAIL_SLIDER1, &PitchPlayerDlg::On_Releasedcapture_Detail_Slider)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_MIDI_VOLUMN_SLIDER, &PitchPlayerDlg::On_Released_Capture_MidiVolumn_Slider)
	ON_NOTIFY(UDN_DELTAPOS, IDC_MIDI_INSTRUMENT_SPIN, &PitchPlayerDlg::On_Deltapos_Midi_Instrument_SpinCtrl)
	ON_COMMAND(ID_FILE_LOADNOTELIST, &PitchPlayerDlg::On_Load_Notelist_FileMenu)
	ON_BN_CLICKED(IDC_MUTE_MIDI_PLAY_CHECKBOX, &PitchPlayerDlg::On_BnClicked_Mute_MidiPlay_Checkbox)
	ON_BN_CLICKED(IDC_BOOST_SOURCE_CHECKBOX, &PitchPlayerDlg::On_BnClicked_Boost_Source_Checkbox)
	ON_BN_CLICKED(IDC_MIDIsource_DETECTION_RADIO, &PitchPlayerDlg::On_BnClicked_Midi_from_Detection_RadioButton)
	ON_BN_CLICKED(IDC_MIDIsource_NOTELIST_RADIO,   &PitchPlayerDlg::On_BnClicked_Midi_from_NoteList_RadioButton)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_MIDI_SYNC_SLIDER, &PitchPlayerDlg::On_NMReleasedCapture_MidiSync_Slider)
END_MESSAGE_MAP()



			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////


class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()



			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool     PitchPlayerDlg::Is_Song_Loaded()
{

	SPitchCalc   *sPitchCalcer     =   Get_SPitchCalc();

	if(    sPitchCalcer  ==  NULL  )
		return  false;         //   NO song is loaded
	else
		return  true;
}


				/////////////////////////////////////////////////


bool     PitchPlayerDlg::Is_Playing()
{

	if(    ! Is_Song_Loaded()    )
		return   false;

	bool      isPlaying  =    GetSoundHelper().Is_WAVplayer_Playing();
	return   isPlaying;
}



				/////////////////////////////////////////////////


bool     PitchPlayerDlg::Is_Playing_Backwards()
{

	if(    ! Is_Song_Loaded()    )
		return   false;


	StreamingAudioplayer  *audioPlayer  =     GetSoundHelper().m_audioPlayer;  
	ASSERT(                      audioPlayer  );


	bool   isPlayingBackward  =   false; 

	if(     audioPlayer->m_playingBackward    )
		isPlayingBackward  =   true;  


	return    isPlayingBackward;
}



				/////////////////////////////////////////////////


bool     PitchPlayerDlg::Midi_Source_Is_NoteList()
{

	if(    ! Is_Song_Loaded()    )
		return   false;


	if(          m_detectionMidiSourceRadio  ==   0   )     //  Detection
		return  false;
	else if(   m_detectionMidiSourceRadio  ==   1   )     //  Notelist
		return  true;
	else
	{	ASSERT( 0 );

		return  false;
	}
}


						////////////////////////////////////////


void    PitchPlayerDlg::User_Message_Stop_Play(   char*  mesg   )
{

			//  typical use:    "STOP the PLAY of music to use this button."
			//
			//                       "Change MIDI SOURCE to DETECTION to use this control."

	if(   mesg  ==   NULL   )
	{
		ASSERT( 0 );
		return;
	}

	SoundHelper&   soundHelper =     GetSoundHelper(); 
	CString   retErrorPause;


	if(     soundHelper.Is_WAVplayer_Playing()      )    //   1/12   Now feel it is better to stop, and allow a PreRoll,  than just let it keep on playing.
	{
		if(    ! soundHelper.Pause_Play(  retErrorPause  )     )	
		{
			AfxMessageBox(  retErrorPause  );

//			return;   //  Want this ???
		}
	}


	AfxMessageBox(  mesg  );
}



						////////////////////////////////////////


void  PitchPlayerDlg::Enable_Detection_Controls(   bool   enable   )
{


	if(   ! Is_Song_Loaded()    )
	{
		enable =  true;    // ****  OK ???? ****
	}


	if(   enable  )
	{
		m_detailSliderCtrl.EnableWindow(  TRUE   );     //   BEST way to   DISABLE   a Button in MFC. Set it to "Disable in Resource Editor, and then ENABLE here.    
	
		m_sensitivitySlider.EnableWindow(  TRUE   );    
	
		m_stereoBalanceDetection.EnableWindow(  TRUE   );    

		m_boostSourceCheckBox.EnableWindow(     TRUE   );    
	}
	else
	{ 	m_detailSliderCtrl.EnableWindow(   FALSE   );     //   BEST way to   DISABLE   a Button in MFC. Set it to "Disable in Resource Editor, and then ENABLE here.    
	
		m_sensitivitySlider.EnableWindow(  FALSE   );    
	
		m_stereoBalanceDetection.EnableWindow(   FALSE   );    

		m_boostSourceCheckBox.EnableWindow(     FALSE   );    
	}
}



				/////////////////////////////////////////////////


bool   PitchPlayerDlg::Change_FontCd(    FontsCd&  fontCd,   short  fontID,   CString&  retErrorMesg  )
{

					//  *****   Must SYNC  the CASE Statements with    FontsCd::Change_To_New_Font()  *********************

	retErrorMesg.Empty();
	bool  functionSucess =  true;


	if(    fontCd.m_height  > 0    )
	{
		if(   ! fontCd.m_fontsBitmap.DeleteObject()    )    //  If it has already been loaded with a Bitmap,  we must first delete it.   2/12
		{
			retErrorMesg =  "PitchPlayerDlg::Change_FontCd  FAILED,  DeleteObject."  ;
			return  false;
		}
	}


	switch(   fontID   )
	{

		case   6 :	
			if(    ! fontCd.m_fontsBitmap.LoadBitmap(  IDB_FONTS_6_BITMAP  )     )  
			{  
				retErrorMesg =  "PitchPlayerDlg::Change_FontCd  FAILED,  missing Case 6 could NOT LoadBitmap 6."  ;
				fontCd.m_height  =   -1;
				functionSucess =  false;
			}
			else
			    fontCd.m_height  =  11; 
		break;

		/***
		case   7 :	
			if(    ! fontCd.m_fontsBitmap.LoadBitmap(  IDB_FONTS_7_BITMAP  )     )  
			{  
				retErrorMesg =  "PitchPlayerDlg::Change_FontCd  FAILED,  missing Case 7 could NOT LoadBitmap 7."  ;
				fontCd.m_height  =   -1;
				functionSucess =  false;
			}
			else
			    fontCd.m_height  =   11;   
		break;

		case   8 :	
			if(    ! fontCd.m_fontsBitmap.LoadBitmap(  IDB_FONTS_8_BITMAP  )     )  
			{  
				retErrorMesg =  "PitchPlayerDlg::Change_FontCd  FAILED,  missing Case 8 could NOT LoadBitmap 8."  ;
				fontCd.m_height  =   -1;
				functionSucess =  false;
			}
			else
			    fontCd.m_height  =   25;   
		break;
		****/

		default:    
			ASSERT( 0 );    
			retErrorMesg =  "PitchPlayerDlg::Change_FontCd  FAILED,  missing Case."  ;

			if(    ! fontCd.m_fontsBitmap.LoadBitmap(  IDB_FONTS_6_BITMAP  )     )  
			{  
				retErrorMesg =  "PitchPlayerDlg::Change_FontCd  FAILED, at Case DEFAULT, could NOT LoadBitmap 6."  ;
				fontCd.m_height  =   -1;
				functionSucess =  false;
			}
			else
			    fontCd.m_height  =  11;   //  13;   

			functionSucess =  false;
		break; 
	}


	if(   ! fontCd.Change_To_New_Font(  fontID  )    )
	{
		retErrorMesg.Format(   "PitchPlayerDlg::Change_FontCd  FAILED,  Change_To_New_Font[  fontID = %d  ]." ,   fontID    );
		functionSucess =  false;
	}

	return  functionSucess;
}



					/////////////////////////////////////////////////


void   PitchPlayerDlg::Sync_FileSlider_Control(   long   panesSampleCount,   long  firstSampleIdx   )
{

	if(   firstSampleIdx  <  0  )
	{
	//	ASSERT( 0 );           //  Land here when hit the  FileStart Button and hit REVERSE Play button  2/12
		firstSampleIdx =  0;
	}


	long   totalSamples  =    GetSoundHelper().m_audioPlayer->Get_Biggest_SampleIndex_No_SpeedExpansion();


	long   newPos  =    (long)(  	  (double)m_numberOfUnitsForFilePositionSlider  *	  (  (double)firstSampleIdx  /  (double) totalSamples  )             );


	if(          newPos   >   m_numberOfUnitsForFilePositionSlider   )   
		newPos =   m_numberOfUnitsForFilePositionSlider;
	else if(    newPos <    0   ) 
		newPos =      0;


	if(    newPos   !=   m_lastScrollPositionAbsoluteHorz   )      //   try to cut down on function calls
	{
		m_curPlayPositionSliderCtrl.SetPos(  newPos  );

		m_lastScrollPositionAbsoluteHorz  =      newPos;
	}

//	TRACE(  "\nSync_Horizontal_Scroll_Knob_spWind:    origPos[ %d ],      newPos[ %d ]   \n"   ,    origPos,   newPos    );     
}



											////////////////////////////////////////


void   PitchPlayerDlg::Paint_Bullets_to_Stationary_Gagues()
{


	SoundHelper&   soundHelper  =     GetSoundHelper(); 


	short   sPitch               =                                  soundHelper.m_curScalePitchAtStop;    

	short   nuDetectScore  =   (short)(    (double)( soundHelper.m_curScalePitchAvgHarmonicMagAtStop )  *  m_notesColorBrighten   );   //  make the NOTES look brighter on the display


//	 Render_Revolvers_Background(  dc  );

	if(   Is_Song_Loaded()   )
		Render_Revolvers_Bullet_ToOffmap(   sPitch,   nuDetectScore   );   //  For each event it will render, regardless if a new note has started or not.
}																										   //   ...that is why the color value changes even on a long single note.   1/10



											////////////////////////////////////////


void	  PitchPlayerDlg::Render_Last_Amimation_Frame()
{

			//  called when Play has STOPPED,  and need to render BULLETS to Revolver and Gagues    2/12

//	 Paint_Bullets_to_Stationary_Gagues();   *******   DISABLED,  not worth it to paint the Bullets THIS WAY(?? ) after  PLAY has STOPPED  ******************
}



					/////////////////////////////////////////////////


void PitchPlayerDlg::OnPaint()
{
		// If you add a minimize button to your dialog, you will need the code below
		//  to draw the icon.  For MFC applications using the document/view model,
		//  this is automatically done for you by the framework.

	SoundHelper&   soundHelper =     GetSoundHelper(); 


	if(   IsIconic()   )
	{
		CPaintDC dc( this ); // device context for painting

		SendMessage(  WM_ICONERASEBKGND,  reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0   );
																
		int cxIcon =   GetSystemMetrics(SM_CXICON);       	// Center icon in client rectangle
		int cyIcon =   GetSystemMetrics(SM_CYICON);

		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{  CClientDC  dc( this ); 


		short   nuDetectScore  =   (short)(    (double)( soundHelper.m_curScalePitchDetectionScoreAtStop )  *  m_notesColorBrighten   );   //  make the NOTES look brighter on the display
		short   sPitch               =                               soundHelper.m_curScalePitchAtStop;    

 
		m_gaugeOfRevolver.Render_Gauges_Background(  dc  );


		if(   Is_Song_Loaded()    )
			Render_Revolvers_Bullet_ToOffmap(   sPitch,   nuDetectScore   );   //  For each event  it will render,  regardless if a new note has started or not.
																										     //   ...that is why the color value changes even on a long single note.   1/10		

		CDialog::OnPaint();
	}
}



					/////////////////////////////////////////////////


void   PitchPlayerDlg::Write_Dialogs_Title_Bar(   CString&  songTitle   )
{
	CString  finMesg;
	finMesg.Format(   "PitchScope Player  -  %s"  ,   songTitle   );

	SetWindowText(   finMesg   );
}


											////////////////////////////////////////


void   PitchPlayerDlg::Render_Revolvers_Background(   CDC&  dc  )  
{

	m_gaugeOfRevolver.Render_Gauges_Background(  dc  ); 

	m_gaugeOfRevolver.Mark_Bullets_Invisible();   
}



											////////////////////////////////////////


void      PitchPlayerDlg::Render_Revolvers_Bullet_ToOffmap(    short  foundScalePitch,     short  spitchsVolume   )  
{

					 //    ...renders ONE  'Bullet'   ( a single  Revolver's   'OVAL'   with text  Note-LetterName  )


	bool     useMultiColored =   true;

	long     britenessFactor =    150;     //   256  ****ADJUST to make the bullets brighter    



	short    keyInSPitch =  0;       //  0:   'E'     ****** HARDWIRED,  sometimes read from ProjectFile *********
//   ****   ScalepitchList::Get_Musical_Key(   short&  retSharpCount,    short&  retFlatCount,    bool&  isMinor   )


	SPitchCalc   *sPitchCalcer     =    Get_SPitchCalc();
	if(                 sPitchCalcer  ==  NULL  )
	{
		ASSERT( 0 );
		//   return  ;    //  just means that a song is not loaded
	}
	else
		keyInSPitch =    sPitchCalcer->m_musicalKey;  



	CClientDC   dc( this ); 

	short     scalePitch =  -9,    volumeMidi = 0;
	short        numMapChannels  =    12;            // ***** HARDWIRED  ??? but OK ??   *****
	CString    retErrorMesg;



	RectShrt   rctPane(      m_xOffsetRevolver,    //  UpperLeft              
						             m_yOffsetRevolver,   
							  m_xOffsetRevolver   +  m_panesWidthRevolver,      //  lowerRight
							  m_yOffsetRevolver   +  m_panesHeightRevolver    ); 



	if(    foundScalePitch  >=   -1  )      //  do we overide??  
	{

		if(   foundScalePitch  ==  -1    )    //  do NOT render the bullet
		{
		    scalePitch   =  11;
			volumeMidi =   0;   
	//		return;  //  true;
		}
		else
		{  scalePitch   =    foundScalePitch;
			volumeMidi =    spitchsVolume;        //    250;    // should reflect the intensity of the detectionValue   12/09
		}
	}



	short  quadrantPos  =     SPitchCalc::Get_ScalePitchs_MusicalKey_Transposed_Position(   scalePitch,    keyInSPitch  );

	ASSERT(    quadrantPos  >=  0   &&     quadrantPos  <  12   );



	COLORREF    crColor;


	if(    useMultiColored     )
	{
		short   redFill,   greenFill,    blueFill;
		SPitchCalc::Get_ScalePitchs_Color(    scalePitch,     keyInSPitch,             redFill,       greenFill,        blueFill     );  

		redFill    =     (short)(          ( (long)redFill     *       (long)volumeMidi )  / britenessFactor     );      //   moderate by the score's value
		greenFill =     (short)(          ( (long)greenFill *       (long)volumeMidi )  / britenessFactor      ); 
		blueFill   =      (short)(         ( (long)blueFill    *       (long)volumeMidi )  / britenessFactor     ); 

		if(   redFill      >  255  )      redFill =   255;  
		if(   greenFill  >  255  )      greenFill =   255;  
		if(   blueFill    >  255  )      blueFill =   255;  

		crColor  =    RGB(   redFill,    greenFill,   blueFill    );



		if(          m_gaugeOfRevolver.m_currentVisibleBulletIdx  ==   quadrantPos 
			 &&    m_gaugeOfRevolver.m_currentVisibleBulletIdx  >=   0  
			 &&    foundScalePitch  >=  0     )
		{
			int  dummy =  9;
		}
		else
		{  m_gaugeOfRevolver.Erase_Current_Bullet(  dc  ); 

			if(     foundScalePitch  >=  0   )
				m_gaugeOfRevolver.Render_Gauges_Bullet(   dc,   quadrantPos,   scalePitch,   crColor,    m_userPrefMusicalKeyAccidentals   ); 
		}
	}
	else
	{  
		ASSERT( 0 );
		/****
		crColor  =    RGB(   volumeMidi,   volumeMidi,   volumeMidi   );
		if(   foundScalePitch !=  -1    )
		{
			Fill_SolidGrey_Rect(   rctPane,    m_revolversBackgroundGray,     dc    );
			Draw_Solid_Colored_Oval(    rct,    &dc,     volumeMidi,  volumeMidi,  volumeMidi   );  
		}
		****/
	}
}


						////////////////////////////////////////


void	   PitchPlayerDlg::Erase_Last_Bullet()
{

	SoundHelper&   soundHelper =     GetSoundHelper(); 

	CClientDC   dc( this ); 


	soundHelper.m_curScalePitchDetectionScoreAtStop  =     0;

	soundHelper.m_curScalePitchAtStop  =    -1;  


	m_gaugeOfRevolver.Render_Gauges_Background(  dc  ); 
}


						////////////////////////////////////////


void PitchPlayerDlg::On_Releasedcapture_Sensitivity_Slider(  NMHDR *pNMHDR,   LRESULT *pResult   )
{

	short   smallestSetting =   1;   // **** Keep in mind  *****


	PitchPlayerApp&    playerApp  =      Get_PitchPlayerApp();
	CString    strText;

	
	short        curPos  =     m_sensitivitySlider.GetPos();

	ASSERT(   curPos  >=  0      &&     curPos  <= 100  );



	short	       nuSetting  =    curPos  +  1;   // want vaues in range  { 1  to 99 ]

	ASSERT(  nuSetting >= 1 );    //   smallestSetting =   1;  

	playerApp.m_detectionSensitivityThreshold  =  nuSetting;


	strText.Format(   "%d",   101  -  playerApp.m_detectionSensitivityThreshold  );     // RELEASE
//	strText.Format(   "%d",              playerApp.m_detectionSensitivityThreshold  );     // ******* TEMP ,  debug  ( better to show the true cutoff SCORE 

	m_sensitivityCStatic.SetWindowText(   strText   );


	
	if(     Midi_Source_Is_NoteList()    )   //   MOVED to bottom so that values in  playerApp  will SYNC with   SLider's values
	{
		User_Message_Stop_Play(   "Change MIDI SOURCE to DETECTION to use this control."    );
		return;
	}


	*pResult = 0;
}



						////////////////////////////////////////


void PitchPlayerDlg::On_Releasedcapture_Detail_Slider(  NMHDR *pNMHDR,   LRESULT *pResult  )
{

	CString   strText;
	short       curPos  =     m_detailSliderCtrl.GetPos();


	Set_SPitchCalcs_Detail_Threshold(   curPos   );

	Update_Detail_Controls_StaticText();


	
	if(     Midi_Source_Is_NoteList()    )   //   MOVED to bottom so that values in  playerApp  will SYNC with   SLider's values
	{
		User_Message_Stop_Play(   "Change MIDI SOURCE to DETECTION to use this control."    );
		return;
	}


	*pResult = 0;
}



						////////////////////////////////////////


void    PitchPlayerDlg::On_Releasedcapture_Detect_Stereo_Balance_Slider(NMHDR *pNMHDR, LRESULT *pResult)
{

	PitchPlayerApp&    playerApp  =      Get_PitchPlayerApp();

	short   curVal        =     m_stereoBalanceDetection.GetPos();
	short   newSetting  =    curVal;    //  

	if(        newSetting  <  0   )
		newSetting =   0;
	else if(  newSetting > 100  )
		newSetting =   100;

	playerApp.m_rightChannelPercent  =   newSetting;  


	
	if(     Midi_Source_Is_NoteList()    )   //   MOVED to bottom so that values in  playerApp  will SYNC with   SLider's values
	{
		User_Message_Stop_Play(   "Change MIDI SOURCE to DETECTION to use this control."    );
		return;
	}


	*pResult = 0;
}



						////////////////////////////////////////


void   PitchPlayerDlg::Set_MidiSync_Controls_Position()
{


	long    defaultControlPosition  =     0;   

	long    settingsMultiplier          =   10;


	PitchPlayerApp&    playerApp  =    Get_PitchPlayerApp();
	short      calcedPosition =  -1;
	CString   fullText;



	if(    ! Is_Song_Loaded()   )       
	{													

		m_midiSyncSliderCtrl.SetPos(   defaultControlPosition   );   //  0:   should  

		fullText.Format(   "%d" ,   0  );
		m_midiSyncValueStatic.SetWindowText(   fullText   );

		m_midiSyncValueStatic.UpdateWindow();    //  to force the immediate 
		return;
	}




	if(   m_detectionMidiSourceRadio ==  1  )    //    1:  NOTELIST  mode
	{

		calcedPosition =     playerApp.m_noteListHardwareDelayForPlayer  *  settingsMultiplier;

		fullText.Format(    "%d" ,      playerApp.m_noteListHardwareDelayForPlayer   );
	}
	else                                                         //    0:   DETECTION  Mode	
	{
		if(     playerApp.m_overideDelay  >=  0    )                              //   OVERIDE 
		{
			calcedPosition =    playerApp.m_overideDelay   *   settingsMultiplier; 

//			fullText.Format(   "%d   [ 0 ]"  ,     playerApp.m_overideDelay  );
			fullText.Format(   "%d"  ,              playerApp.m_overideDelay  );
		}
		else
		{	calcedPosition =      ( playerApp.m_baseDelay  + 4 )   *  settingsMultiplier; 

//			fullText.Format(   "%d   [ %d ]"  ,      ( playerApp.m_baseDelay  + 4 ),    playerApp.m_baseDelay  );
			fullText.Format(   "%d"  ,                  ( playerApp.m_baseDelay  + 4 ),    playerApp.m_baseDelay  );
		}
	}



	m_midiSyncSliderCtrl.SetPos(   calcedPosition   ); 

	m_midiSyncValueStatic.SetWindowText(   fullText   );

	m_midiSyncValueStatic.UpdateWindow();    //  to force the immediate 
}




						////////////////////////////////////////


void  PitchPlayerDlg::On_Releasedcapture_Wavs_Volume_Slider(  NMHDR *pNMHDR,   LRESULT *pResult   )
{

	PitchPlayerApp&    playerApp  =      Get_PitchPlayerApp();

	short   curSliderPosition  =    100  -    m_wavsVolumnSliderCtrl.GetPos();

	if(          curSliderPosition  >  100   )
		curSliderPosition =   100;
	else if(   curSliderPosition  <     0   ) 
		curSliderPosition =      0;

	 Set_Soundhelpers_WAV_Volume(   curSliderPosition   ); 

	*pResult = 0;
}



											////////////////////////////////////////


void PitchPlayerDlg::On_Released_Capture_MidiVolumn_Slider(NMHDR *pNMHDR, LRESULT *pResult)
{
	
	PitchPlayerApp&    playerApp  =      Get_PitchPlayerApp();

	short   curSliderPosition  =    100  -    m_midiVolumnSliderCtrl.GetPos();

	if(          curSliderPosition  >  100   )
		curSliderPosition =   100;
	else if(   curSliderPosition  <     0   ) 
		curSliderPosition =      0;

	 Set_Soundhelpers_Midi_Volume(   curSliderPosition   ); 

	*pResult = 0;
}


											////////////////////////////////////////


void   PitchPlayerDlg::OnNMReleasedCapture_curplay_PositionSlider(  NMHDR *pNMHDR,   LRESULT *pResult  )
{

	CString             retErrorMesg;
	SoundHelper&   soundHelper  =   GetSoundHelper(); 



	if(    ! Is_Song_Loaded()    )
	{																								
		m_curPlayPositionSliderCtrl.SetPos( 0 );   

		return;         //  No big deal,   get this if user tries to move the slider  when a WAV file is NOT loaded
	}



	bool   wasPlaying =  false;

	if(     soundHelper.Is_WAVplayer_Playing()      )
	{
		bool   succs  =     soundHelper.Pause_Play(   retErrorMesg  );	
		if(    ! succs   )
			AfxMessageBox(  retErrorMesg  );

		wasPlaying =   true;
	}




	if(    ! soundHelper.Initialize_Delay_Buffers_and_Two_CircQues( retErrorMesg )    )     //  For BOTH Player and Navigator  
	{	
		AfxMessageBox(  retErrorMesg  );	
		return;
	}

	soundHelper.Erase_Players_AudioDelay_MemoryBuffer();



	Erase_Last_Bullet();




	///////////////////////////
	long  lastSampleIndex =    soundHelper.m_audioPlayer->Get_Biggest_SampleIndex_No_SpeedExpansion();    //  does NOT have the slowDown expansion in it

	int         curVal  =   m_curPlayPositionSliderCtrl.GetPos();

	if(          curVal   <       0		          //  Value should always be between  { 0 - m_numberOfUnitsForFilePositionSlider }
		 &&    curVal   >    m_numberOfUnitsForFilePositionSlider    )
	{
		ASSERT( 0 );
		return;
	}
	

	long        newSampleIndex    =    (long)(       (   (double)lastSampleIndex       *     (double)curVal  ) /   (double)m_numberOfUnitsForFilePositionSlider         ); 
	ASSERT(  newSampleIndex  >=  0  ); 


	if(    ! soundHelper.Move_to_Files_Position(  newSampleIndex,   retErrorMesg  )    )   //  Only call this function from Player,  NOT  Navigator.  11/11
	{
		AfxMessageBox(  retErrorMesg  );
		return;
	}
	//////////////////////////////



	m_lastScrollPositionAbsoluteHorz  =    curVal;     


	soundHelper.m_lastTransportDirection      =   SoundHelper::SLIDERcHANGE;      //  This work differently in Player.exe than Navigator,  but can still trigger a FileSeek  
	soundHelper.m_lastTransportActionActual  =   SoundHelper::SLIDERcHANGE;  



//	UpdateData(  true );    //   Solve the  FILE-SLIDER bug  ???     NO,  tried true and false.   8/2012



	*pResult = 0;
}



											////////////////////////////////////////


void    PitchPlayerDlg::On_Deltapos_Midi_Instrument_SpinCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{

	LPNMUPDOWN   pNMUpDown    =     reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	ASSERT(          pNMUpDown  !=  NULL  );


	short   upperLimit  =   95;   // **********************************


	SoundHelper&     soundHelper  =    GetSoundHelper();  
	CString   retErrorMesg;

 
	bool  isGoingBackwards =   Is_Playing_Backwards(); 



	if(    pNMUpDown->iDelta  <  0    )    //  Hit the down arrow ?
		m_midiInstrumentsControlsValue--;
	else
		m_midiInstrumentsControlsValue++;
	


	if(    m_midiInstrumentsControlsValue  <  0    )
		m_midiInstrumentsControlsValue =  upperLimit;
	else if(   m_midiInstrumentsControlsValue  > upperLimit  )
		m_midiInstrumentsControlsValue =  0;



	short  controlsPosition  =   	 m_midiInstrumentSpinCtrl.GetPos();

	if(   m_midiInstrumentsControlsValue   !=   controlsPosition   )
	{
		m_midiInstrumentSpinCtrl.SetPos(  m_midiInstrumentsControlsValue  );
//		TRACE(  "\n  called SET_Pos   [ %d ]  \n",    controlsPosition    );
	}


	Update_MidiInstrument_Name_Text(   m_midiInstrumentsControlsValue   );    //  Just writes to the display


	if(   soundHelper.m_midiSequencer  !=  NULL   )
	{
		soundHelper.m_midiInstrumentPatch  =    m_midiInstrumentsControlsValue;

		if(    ! soundHelper.Change_Midi_Instrument(   m_midiInstrumentsControlsValue,   retErrorMesg   )    )
			AfxMessageBox(  retErrorMesg  );	
	}
	else
	   AfxMessageBox(   "Could NOT set the Midi Instrument Patch because a Midi DEVICE has not been established."   );				

	*pResult = 0;
}



					/////////////////////////////////////////////////


void     PitchPlayerDlg::Update_MidiInstrument_Name_Text(   short   instrumentCode   ) 
{

	SoundHelper&    soundHelper  =    GetSoundHelper();  
	CString             instrumentNameText =  "??",     mesg,   fullText;


	if(        instrumentCode   <    0  
		 ||   instrumentCode   >   95  )
	{
		ASSERT( 0 );   //  Is this possible ?????
		instrumentCode =  0;  //   default,  viola
	}

	if(    ! soundHelper.Get_Midi_Instrument_Text_Name(   instrumentCode,    instrumentNameText  )      )
		AfxMessageBox (  instrumentNameText  );    //  holds an error message
	else
	{  
//		fullText.Format(  "[ %d ]   %s",       instrumentCode,   instrumentNameText    );
		fullText.Format(  "%d   %s",            instrumentCode,   instrumentNameText );

		m_midiInstrumentNameStatic.SetWindowText(   fullText   );

		m_midiInstrumentNameStatic.UpdateWindow();    //  to force the immediate RENDER
	}
}



					/////////////////////////////////////////////////


HCURSOR PitchPlayerDlg::OnQueryDragIcon()
{
					// The system calls this function to obtain the cursor to display while the user drags the minimized window.

	return static_cast<HCURSOR>(m_hIcon);
}



						////////////////////////////////////////


void   PitchPlayerDlg::Set_Soundhelpers_WAV_Volume(  int  curSliderPosition   ) 
{

	CString   retErrorMesg;  
	PitchPlayerApp&      playerApp =    Get_PitchPlayerApp();


	int       curVal  =   curSliderPosition;

	if(          curVal   >=       0		          //  Value should always be between  { 0 - 100 }
		 &&   curVal   <=    100    )
	{
		short  wavsVolume;

//		wavsVolume =    ( 100  -  curVal );
		wavsVolume =       curVal ;

		//   TRACE(    "curVal[ %d ]      WAV[  %d  ] \n",      curVal,    wavsVolume   );
																					//  Now assign the 2 volume levels
		playerApp.m_wavsVolumeApp  =    wavsVolume;
	}
	else
		ASSERT( 0 );
}



						////////////////////////////////////////


void	 PitchPlayerDlg::Set_Soundhelpers_Midi_Volume(  int  curSliderPosition   )
{

	CString   retErrorMesg;  
	PitchPlayerApp&      playerApp =    Get_PitchPlayerApp();


	int       curVal  =   curSliderPosition;

	if(          curVal   >=       0		          //  Value should always be between  { 0 - 100 }
		 &&   curVal   <=    100    )
	{
		short  midiVolume;

	//	midiVolume =    ( 100  -  curVal );
		midiVolume =        curVal;

//   TRACE(    "curVal[ %d ]      midiVolme[ %d ]   \n",      curVal,    midiVolume   );

																						//  Now assign thevolume levels
		playerApp.m_midiVolume  =    midiVolume;
	}
	else
		ASSERT( 0 );
}


					/////////////////////////////////////////////////


void   PitchPlayerDlg::Shutting_Down()
{

						//  Does NO GOOD  to return  a value,  I can NOT stop the App's EXIT at this point.   5/2012

	PitchPlayerApp&   playerApp =    Get_PitchPlayerApp();

	playerApp.Query_User_For_ObjectList_Save_On_App_Exit();   
}


											////////////////////////////////////////


void PitchPlayerDlg::OnSysCommand(   UINT nID,   LPARAM lParam   )
{

	CString  retErrorMesg;

	if(  (nID & 0xFFF0) == IDM_ABOUTBOX  )
	{

		if(     GetSoundHelper().Is_WAVplayer_Playing()      )
		{
			bool  succs  =     GetSoundHelper().Pause_Play(   retErrorMesg  );	
			if(    !succs   )
				AfxMessageBox(  retErrorMesg  );
		}

		CAboutDlg  dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{  CDialog::OnSysCommand(nID, lParam);
	}
}


											////////////////////////////////////////


void PitchPlayerDlg::On_BnClicked_Mute_MidiPlay_Checkbox()
{


	SoundHelper&         soundHelper  =     GetSoundHelper();  
	SequencerMidi    *midiSequencer =	     soundHelper.Get_Current_MidiSequencer();
	CString   retErrorMesg;   


	if(   midiSequencer  ==  NULL   )     //    midiSequencer  CHANGES  if user picks a new one on the Settings dialog
		return;   


	if(     m_muteMidiCheckBox.GetCheck()  ==   BST_CHECKED    )  
	{
		if(    ! soundHelper.Stop_MidiPlay(  retErrorMesg  )    )
			AfxMessageBox(  retErrorMesg   );

		midiSequencer->m_silenceMidi =   true;
	}
	else
		midiSequencer->m_silenceMidi =   false;
}



											////////////////////////////////////////


void  PitchPlayerDlg::On_BnClicked_Boost_Source_Checkbox()
{


	SoundHelper&   soundHelper  =    GetSoundHelper();  
	long    scaleInPercent;


	if(     m_boostSourceCheckBox.GetCheck()  ==   BST_CHECKED    )  
		scaleInPercent  =     soundHelper.Set_VolumeBoost_Factor(  1  );	    //  or 2 ???   
	else
		scaleInPercent  =     soundHelper.Set_VolumeBoost_Factor(  0  );	


	
	if(     Midi_Source_Is_NoteList()    )   //   MOVED to bottom so that values in  playerApp  will SYNC with   SLider's values
	{
		User_Message_Stop_Play(   "Change MIDI SOURCE to DETECTION to use this control."    );
		return;
	}
}



											////////////////////////////////////////


void PitchPlayerDlg::On_BnClicked_Midi_from_Detection_RadioButton()
{

	SoundHelper&       soundHelper  =    GetSoundHelper();  
	CString   retErrorMesg;


	if(    m_detectionMidiSourceRadio  ==  0    )    // We already have that value    ( need this or I got an INFINITE LOOP with below message
		return;


	/****   For PLAYER we might not need to stop...

	if(     soundHelper.Is_WAVplayer_Playing()      )    //   1/12   Now feel it is better to stop, and allow a PreRoll,  than just let it keep on playing.
	{
		if(    ! soundHelper.Pause_Play(  retErrorMesg  )     )	
		{
			AfxMessageBox(  retErrorMesg  );
			return;  
		}
	}
	*****/



	if(    ! Change_Midi_Source(  0,   false  )     )
	{

		m_detectionMidiSourceRadio =   0;   //  Can happen if no song is loaded ?????

		Enable_Detection_Controls(  true  );


		if(             Is_Song_Loaded()    
			    &&    soundHelper.Get_BitSource_Streaming()  != NULL    )
			soundHelper.Get_BitSource_Streaming()->Set_MidiSource_UsingNotelist_Variables(  false  ); 


		UpdateData( false );    //   *** Not needed,  think message from  UpdateData() are part of the bug below.  12/19/11
	}
	else
	{	m_detectionMidiSourceRadio =   0;

	//   Enable_Detection_Controls(  true  );   ***NO,    Change_Midi_Source()   did this


//		UpdateData( false );    //   *** Not needed,  think message from  UpdateData() are part of the bug below.  12/19/11
	}
}




											////////////////////////////////////////


void PitchPlayerDlg::On_BnClicked_Midi_from_NoteList_RadioButton()
{

	/*******************************   IMPORTANT !!!!!   1/12  ***********************************************************************

		When you try to bring up a  AfxMessageBox  or UpdateData() it creates HAVOC for Radio buttons. ( get stuck in a LOOP )

		To get it to work here, need to open up Dialog in the Resource Editor and change the Properties for the Radio Button:
		by setting  the   "AUTO property" of the  Radio Button must be set to FALSE. Then the RadioButton can NOT  AUTOmatically  toggle its STATE.
	***************************************************************************************************************************/

	SoundHelper&       soundHelper  =    GetSoundHelper();  
	PitchPlayerApp&   playerApp =    Get_PitchPlayerApp();
	CString   retErrorMesg;



	if(   m_detectionMidiSourceRadio  ==  1  )    // We already have that value   ( need this or I got an INFINITE LOOP with below message
		return;



	if(   ! Is_Song_Loaded()   )                     
	{

		m_detectionMidiSourceRadio =  0;   

		Enable_Detection_Controls(  true  );


	  //  FAILS 1/12:    AfxMessageBox(  "Record some notes, or load a NoteList from file."   );
		UpdateData( false );
		
		return; 
	}



	SPitchCalc   *sPitchCalcer  =     Get_SPitchCalc();
	ASSERT(      sPitchCalcer  );  



	if(    playerApp.m_calcedNoteListMaster.Count()  <= 0     )
	{

		m_detectionMidiSourceRadio =  0;   

		soundHelper.Get_BitSource_Streaming()->Set_MidiSource_UsingNotelist_Variables(  false  ); 



	//	UINT   state =    m_detectionMidiSourceRadioControl.GetState();

	//	m_detectionMidiSourceRadioControl.SetCheck(   BST_UNCHECKED  );   **** None of these worked for my problem here.  1/2012
//		m_detectionMidiSourceRadioControl.UpdateData( false );  
//		m_detectionMidiSourceRadioControl.UpdateWindow();   //  *** These will still not cause the appearance to change 
//		m_detectionMidiSourceRadioControl.SetCheck(   BST_CHECKED  );
	//	m_detectionMidiSourceRadioControl.SetCheck(   BST_INDETERMINATE  );


	//	if(    m_lastCommandWasMidiSourceRadioButton   <=   0    )
	//	{

	//		m_lastCommandWasMidiSourceRadioButton++;   // *******  OMIT this variable, it did nothing to solve the problem  1/23/2012 ********



// ***************  Looks like I will have to live WITHOUT the  AfxMessageBox()  because it screw up everything.  1/23/2012    ********

	//		AfxMessageBox(  "Record some notes, or load a NoteList from file. [ On_BnClicked_Midi_from_Notelist_RadioButton ] "   );    //  ***  If I want to have Messages like this,  the   "AUTO property" of the  Radio Button must be set to FALSE

// ****************************************************************************************************************


	//		m_lastCommandWasMidiSourceRadioButton++;

		//	m_detectionMidiSourceRadio =  0;   

		//	UpdateData( false );
	//	}


		UpdateData( false );   //    ***  BEST,  1/12   Sometimes creates problems for  later Menu Choices if I use the  AfxMessageBox()

		return;
	}




	bool  wasDoingARecording =   false;

	BitSourceStreaming   *bitSourceStreaming  =	Get_BitSource_Streaming(); 

	if(     bitSourceStreaming  !=  NULL    )
		wasDoingARecording  =   bitSourceStreaming->m_recordNotesNow;   //  need to HOLD this value because  Pause_Play() could reset it   1/12  


	/****   For PLAYER we might not need to stop

	if(     soundHelper.Is_WAVplayer_Playing()      )    //   1/12   For Notelist I could just just let it keep going
	{
		if(   wasDoingARecording   )
		{
			if(    ! soundHelper.Pause_Play(  retErrorMesg  )     )	
			{
				AfxMessageBox(  retErrorMesg  );
				return;  
			}
		}
	}
	****/



	if(    ! Change_Midi_Source(  1,   false  )     )
	{

		ASSERT( 0 );   //   Ever happen???    8/2012

		m_detectionMidiSourceRadio =   0;     // If there is NO notelist loaded,  then  Change_Midi_Source()  fails we we set this to 0

		Enable_Detection_Controls(  true  );


		if(             Is_Song_Loaded()    
			    &&    soundHelper.Get_BitSource_Streaming()  != NULL    )
			soundHelper.Get_BitSource_Streaming()->Set_MidiSource_UsingNotelist_Variables(  false  ); 


		UpdateData( false );  
	}
	else
	{  m_detectionMidiSourceRadio =   1;   

	//   Enable_Detection_Controls(  false  );   ***NO,    Change_Midi_Source()   did this


//		UpdateData( false );     //  *** Not needed,  think message from  UpdateData() are part of the bug.  12/19/11
	}
}



									/////////////////////////////////////////////////////


void   PitchPlayerDlg::OnHScroll(   UINT  nSBCode,   UINT nPos,    CScrollBar  *pScrollBar   ) 
{



		//  This will send a message when ANY Horizontal SLIDER Control is  CHANGED.  Have to check  pScrollBar  to verify which control is is reporting from.    12/2011
		//
		//     ... CScrollBar  *pScrollBar  ...BUT it is a parent to the slicer contrls.

	
					//  IS used   5/07,   just that the call browser cant find it ...  caller is OnWndMsg



		//   nScrollCode  -   Value of the low-order word of wParam. Specifies a scroll bar value that indicates the user’s 
		//	                         scrolling request. It is one of the following values: 

	/****
	SB_THUMBPOSITION  -   The user has dragged the scroll box (thumb) and released the mouse button. 
	                                    The nPos parameter indicates the position of the scroll box at the end of the drag operation 


	****/

	SoundHelper&   soundHelper  =    GetSoundHelper(); 
	PitchPlayerApp&      playerApp =    Get_PitchPlayerApp();
	CString   retErrorMesg;

												
/***
		typedef struct tagSCROLLINFO 
		{ 
			   // si 

			UINT cbSize; 
			UINT fMask; 
			int nMin; 
			int nMax; 
			UINT nPage; 
			int nPos; 
			int nTrackPos; 

		} SCROLLINFO; 


		nMin  -   Specifies the minimum scrolling position.

		nMax -   Specifies the maximum scrolling position.

		nPage  -   Specifies the page size. A scroll bar uses this value to determine the appropriate size of the proportional scroll box.

		nPos  -   Specifies the position of the scroll box.

		nTrackPos   -   Specifies the immediate position of a scroll box that the user is dragging. An application can retrieve this value while processing the SB_THUMBTRACK notification message. An application cannot set the immediate scroll position; the SetScrollInfo function ignores this member.
***/



//	TRACE(  "OnHScroll()    nPos [ %d ]       nSBCode [ %d ]    \n" ,    nPos,   nSBCode    );    // ****************   DEBUG ONLY *******************



	/****  NO,  some controls may get initialized if no song is loaded
	if(   ! Is_Song_Loaded()     )
	{
		return;    // Nothing is loaded, so it does not make sense to change the values
	}
	***/

	SPitchCalc  *sPitchCalcer  =     Get_SPitchCalc();    //  might be NULL



	SequencerMidi  *midiSequencer      =	   soundHelper.Get_Current_MidiSequencer();
	ASSERT(            midiSequencer  );




	bool  hitThumbTracking   =    false;

	bool  hitFilePositionSlider =    false;
	bool  hitStereoBalanceSlider =   false;




	CSliderCtrl  *sliderCtrl  =    ( CSliderCtrl* )pScrollBar;  


	if(          sliderCtrl   ==      &m_curPlayPositionSliderCtrl    )
	{
		hitFilePositionSlider =   true;

		if(     soundHelper.Is_WAVplayer_Playing()      )
		{
			bool   succs  =     soundHelper.Pause_Play(  retErrorMesg  );	
			if(    ! succs   )
				AfxMessageBox(  retErrorMesg  );
		}
	}
	else if(   sliderCtrl   ==     &m_stereoBalanceDetection    )
		hitStereoBalanceSlider =  true;




	int       pos =   -1;
	bool    doingDoubleCalls =    false;   //   **** OMIT ****** This function sometimes gets called 2x by OS for one mouseclick   10/03
	bool    isAValidCode =    true;
	bool    supressGaugeDraw =   false;




	switch(   nSBCode   )
	{

		case  SB_LEFT:		//    Scroll to far left.
//			pos =   m_minPosSamplectrlHorz;
		break;

		case  SB_RIGHT:	//    Scroll to far right.
//			pos =     m_knobReducedMaxPosHorz;     //    m_maxPosSamplectrl;
		break;


		case SB_LINELEFT:	 //    Scroll left.
			pos--;
		break;

		case  SB_LINERIGHT:	 //    Scroll right.
			pos++;
		break;


		case  SB_PAGELEFT:	 //    Scroll one page left.
//			pos  -=   jumpUnits;   // if bigger than 1, knob will not work right for bigZoom   5/02
			doingDoubleCalls =    true;				//  ...at the files end are the problems.
		break;

		case  SB_PAGERIGHT:	//    Scroll one page right.
//			pos  +=   jumpUnits;    // if bigger than 1, knob will not work right for bigZoom  5/02
			doingDoubleCalls =    true; 
		break;


		case  SB_ENDSCROLL:				//    ***   ????  WHAT is this for ????
			isAValidCode =    false;
		break;


		case  SB_THUMBPOSITION:     //  The user has dragged the scroll box (thumb) and RELEASED the mouse button. 
			pos =    nPos;				     //  The nPos parameter indicates the position of the scroll box at the end of the drag operation.			
//			supressGaugeDraw =  false;
		break;


		case  SB_THUMBTRACK:    //   The user is dragging the scroll box.   This message is sent repeatedly until the user releases the mouse button. 
			pos =    nPos;               //   The nPos parameter indicates the position that the scroll box has been dragged to.
			hitThumbTracking   =    true;

		//	supressGaugeDraw  =   true;
		break;                             


		default:	
			 isAValidCode =    false;
		break;
	}




												//   Now figure out which of the slider controls got hit,  and what they should do.   12/11

	if(        hitThumbTracking
		&&   pos   >=   0      )
	{

		if(  hitFilePositionSlider    )
		{

			if(     sPitchCalcer  !=  NULL   )
			{
				if(    ! soundHelper.Stop_MidiPlay(  retErrorMesg  )     )
					AfxMessageBox(  retErrorMesg   );

			//	long   virtSampleIdx  =    Calc_VirtSampleIdx_from_FileSliders_Position(  pos  ); 		**** NO test to write for  FilePosition Slider in Player

			//	Write_Virtual_SampleIdx_to_FilePosition_StaticText(  virtSampleIdx  );   ///from Navigator, Player does NOT write text
			}
		}
		else if(    hitStereoBalanceSlider    )
		{
			short   newSetting  =    pos; 

			if(        newSetting  <  0   )
				newSetting =   0;
			else if(  newSetting > 100  )
				newSetting =   100;

			playerApp.m_rightChannelPercent  =   newSetting;  
		}	
	}



	/******   from article on    CWnd::OnHScroll()  
								 
   pScrollBar->SetScrollPos( curpos );     // Set the new position of the thumb (scroll box).

   ******/


//	CMDIChildWnd::OnHScroll(   nSBCode,   nPos,   pScrollBar  );     // *** ???   Did nothing in VoxSep,  but that uses a different Window subclass  ( CMDIChildWnd  )
	CDialog::OnHScroll(             nSBCode,   nPos,   pScrollBar  );    //  *****   TEST,   12/9/11   Do I need this,  will it give trouble  ????   ***********************  
}




									/////////////////////////////////////////////////////


void   PitchPlayerDlg::OnVScroll(   UINT  nSBCode,   UINT nPos,    CScrollBar  *pScrollBar   ) 
{



		//  This will send a message when ANY Horizontal SLIDER Control is  CHANGED.  Have to check  pScrollBar  to verify which control is is reporting from.    12/2011
		//
		//     ... CScrollBar  *pScrollBar  ...BUT it is a parent to the slicer contrls.

	
					//  IS used   5/07,   just that the call browser cant find it ...  caller is OnWndMsg



		//   nScrollCode  -   Value of the low-order word of wParam. Specifies a scroll bar value that indicates the user’s 
		//	                         scrolling request. It is one of the following values: 

	/****
	SB_THUMBPOSITION  -   The user has dragged the scroll box (thumb) and released the mouse button. 
	                                    The nPos parameter indicates the position of the scroll box at the end of the drag operation 
	****/

	SoundHelper&         soundHelper  =    GetSoundHelper();  
	PitchPlayerApp&      playerApp =    Get_PitchPlayerApp();
	CString   retErrorMesg;


												
/***
		typedef struct tagSCROLLINFO 
		{ 
			   // si 

			UINT cbSize; 
			UINT fMask; 
			int nMin; 
			int nMax; 
			UINT nPage; 
			int nPos; 
			int nTrackPos; 

		} SCROLLINFO; 


		nMin  -   Specifies the minimum scrolling position.

		nMax -   Specifies the maximum scrolling position.

		nPage  -   Specifies the page size. A scroll bar uses this value to determine the appropriate size of the proportional scroll box.

		nPos  -   Specifies the position of the scroll box.

		nTrackPos   -   Specifies the immediate position of a scroll box that the user is dragging. An application can retrieve this value while processing the SB_THUMBTRACK notification message. An application cannot set the immediate scroll position; the SetScrollInfo function ignores this member.
***/



//	TRACE(  "OnHScroll()    nPos [ %d ]       nSBCode [ %d ]    \n" ,    nPos,   nSBCode    ); 



	SPitchCalc  *sPitchCalcer  =     Get_SPitchCalc();    //  might be NULL

	/****  Some controls do not depend on a song being loaded.
	if(               sPitchCalcer  ==  NULL     )
	{
		return;    // Nothing is loaded, so it does not make sense to change the values
	}
	****/

	SequencerMidi  *midiSequencer      =	   soundHelper.Get_Current_MidiSequencer();
	ASSERT(            midiSequencer  );


	bool  hitThumbTracking   =    false;

	bool  hitSensitivitySlider =    false;
	bool  hitDetailSlider =    false;
	bool  hitWAVvolumeSlider =    false;
	bool   hitMidiVolumeSlider =  false;


	CSliderCtrl  *sliderCtrl  =    ( CSliderCtrl* )pScrollBar;  

	if(    sliderCtrl         ==       &m_sensitivitySlider    )
		hitSensitivitySlider = true;
	else if(   sliderCtrl   ==        &m_detailSliderCtrl    )
		hitDetailSlider      =  true;
	else if(   sliderCtrl   ==        &m_wavsVolumnSliderCtrl    )
		hitWAVvolumeSlider =  true;
	else if(   sliderCtrl   ==     &m_midiVolumnSliderCtrl    )
		hitMidiVolumeSlider =  true;

	


	int       pos =   -1;
	bool    doingDoubleCalls =    false;   //   **** OMIT ****** This function sometimes gets called 2x by OS for one mouseclick   10/03
	bool    isAValidCode =    true;

	bool    supressGaugeDraw =   false;



	switch(   nSBCode   )
	{

		case  SB_LEFT:		//    Scroll to far left.
//			pos =   m_minPosSamplectrlHorz;
		break;

		case  SB_RIGHT:	//    Scroll to far right.
//			pos =     m_knobReducedMaxPosHorz;     //    m_maxPosSamplectrl;
		break;


		case SB_LINELEFT:	 //    Scroll left.
			pos--;
		break;

		case  SB_LINERIGHT:	 //    Scroll right.
			pos++;
		break;


		case  SB_PAGELEFT:	 //    Scroll one page left.
//			pos  -=   jumpUnits;   // if bigger than 1, knob will not work right for bigZoom   5/02
			doingDoubleCalls =    true;				//  ...at the files end are the problems.
		break;

		case  SB_PAGERIGHT:	//    Scroll one page right.
//			pos  +=   jumpUnits;    // if bigger than 1, knob will not work right for bigZoom  5/02
			doingDoubleCalls =    true; 
		break;


		case  SB_ENDSCROLL:				//    ***   ????  WHAT is this for ????
			isAValidCode =    false;
		break;


		case  SB_THUMBPOSITION:     //  The user has dragged the scroll box (thumb) and RELEASED the mouse button. 
			pos =    nPos;				     //  The nPos parameter indicates the position of the scroll box at the end of the drag operation.			
//			supressGaugeDraw =  false;
		break;


		case  SB_THUMBTRACK:    //   The user is dragging the scroll box.   This message is sent repeatedly until the user releases the mouse button. 
			pos =    nPos;               //   The nPos parameter indicates the position that the scroll box has been dragged to.
			hitThumbTracking   =    true;

		//	supressGaugeDraw  =   true;
		break;                             


		default:	
			 isAValidCode =    false;
		break;
	}



												//   Now figure out which of the slider controls got hit,  and what they should do.   12/11
	CString  strText;


	if(        hitThumbTracking
		&&   pos  >=  0  
	//	&&   sPitchCalcer  !=  NULL     *****  NOT HERE,  test below if needed.
		)
	{

		if(      hitSensitivitySlider   )
		{	

			strText.Format(   "%d",   100  -  pos );     //   show the true cutoff SCORE   (    NOT need to apply setting, just write the number 
			m_sensitivityCStatic.SetWindowText(   strText   );
		}
		else if(    hitDetailSlider   )
		{

			Set_SPitchCalcs_Detail_Threshold(   pos   );
			Update_Detail_Controls_StaticText();
		}
		else if(    hitWAVvolumeSlider   )
		{

			short   curSliderPosition  =    100  -    pos;

			if(          curSliderPosition  >  100   )
				curSliderPosition =   100;
			else if(   curSliderPosition  <     0   ) 
				curSliderPosition =      0;

			Set_Soundhelpers_WAV_Volume(   curSliderPosition   ); 
		}
		else if(   hitMidiVolumeSlider   )
		{

			short   curSliderPosition  =    100  -    pos;

			if(          curSliderPosition  >  100   )
				curSliderPosition =   100;
			else if(   curSliderPosition  <     0   ) 
				curSliderPosition =      0;

			 Set_Soundhelpers_Midi_Volume(   curSliderPosition   ); 
		}
	}




	/******   from article on    CWnd::OnHScroll()  
								 
   pScrollBar->SetScrollPos( curpos );     // Set the new position of the thumb (scroll box).

   ******/


//	CMDIChildWnd::OnVScroll(   nSBCode,   nPos,   pScrollBar  );     // *** ???   Did nothing in VoxSep,  but that uses a different Window subclass  ( CMDIChildWnd  )
	CDialog::OnVScroll(             nSBCode,   nPos,   pScrollBar  );    //  *****   TEST,   12/9/11   Do I need this,  will it give trouble  ????   ***********************  
}





											////////////////////////////////////////
											////////////////////////////////////////

	
bool   PitchPlayerDlg::Change_Midi_Source(   short  newPlayMode,    bool  loadingNewFile  )
{

										//  Toggles  whether or not   to play MIDI NOTES  from a NoteList  or from Detection

	SoundHelper&       soundHelper  =    GetSoundHelper();  
	PitchPlayerApp&   playerApp =    Get_PitchPlayerApp();
	CString   retErrorMesg;



	if(     ! Is_Song_Loaded()    )
	{  

		m_detectionMidiSourceRadio =  0;

		Enable_Detection_Controls(  true  );

		return  false;
	}


	SPitchCalc  *sPitchCalcer  =     Get_SPitchCalc();
	ASSERT(     sPitchCalcer  );


    bool   isPlayingBackward  =   Is_Playing_Backwards(); 


	BitSourceStreaming  *bitSource  =     Get_BitSource_Streaming();
	ASSERT(                   bitSource   );




	if(         newPlayMode ==  1      		
	     &&   playerApp.m_calcedNoteListMaster.Count()  <= 0     )
	{

		m_detectionMidiSourceRadio =  0; 
		bitSource->Set_MidiSource_UsingNotelist_Variables(  false  ); 

		Enable_Detection_Controls(  true  );

		return  false;
	}



	if(    newPlayMode  == 0    )         //  0:   going to Detection-Mode
	{

		m_detectionMidiSourceRadio =  0;   
		bitSource->Set_MidiSource_UsingNotelist_Variables(  false  ); 

		Enable_Detection_Controls(  true  );



		bitSource->Initialize_For_Playing(); 


		if(    ! soundHelper.ReAllocate_Players_AudioDelay_MemoryBuffer(   playerApp.m_numberNeededToMatch   )     )  
		{
			AfxMessageBox(  "Change_Midi_Source  FAILED,   ReAllocate_Players_AudioDelay_MemBuffer,  Detection"    );
		}


		soundHelper.m_lastTransportDirection =   SoundHelper::CONTINUEfORWARD;   //  This prevents an unecessary FileSeek in Continue_Play_Forward()     1/21/12
	}

	else if(    newPlayMode  == 1    )    //  1:   going to   NoteLIST-Mode
	{

		m_detectionMidiSourceRadio =  1;   
		bitSource->Set_MidiSource_UsingNotelist_Variables(  true  ); 

		Enable_Detection_Controls(  false  );




		bitSource->Initialize_For_Playing();   // ***  NEW,  8/22/2012   OK ?????     ...CALLS   sPitchCalcer->Initialize_for_Play()


														//   make sure that the WAV delay is at zero for NoteList Play  

		if(    ! soundHelper.ReAllocate_Players_AudioDelay_MemoryBuffer( 0 )   )      //  0 :   This will DELETE the DelayBuffer...  NO WAVdelay for NoteList mode
		{
			AfxMessageBox(  "Change_Midi_Source  FAILED,   ReAllocate_Players_AudioDelay_MemBuffer,  Notelist"    );
		}



		if(    loadingNewFile    )
		{
			soundHelper.m_lastTransportDirection       =   SoundHelper::FILEsTART;   
			soundHelper.m_lastTransportActionActual  =   SoundHelper::FILEsTART;   

			m_curPlayPositionSliderCtrl.SetPos(    0  );   
			m_lastScrollPositionAbsoluteHorz  =    0;
		}	
	}
	else
	{	ASSERT( 0 );  }




	Set_MidiSync_Controls_Position();

	
	return  true;
}


											////////////////////////////////////////


SPitchCalc*    PitchPlayerDlg::Get_SPitchCalc()
{

								//  If   'sPitchCalcer' is NULL,  just means that a song is NOT Loaded

	SPitchCalc   *sPitchCalcer     =    GetSoundHelper().Get_SPitchCalc();
	return           sPitchCalcer;
}


											////////////////////////////////////////


BitSourceStreaming*     PitchPlayerDlg::Get_BitSource_Streaming()
{

	BitSourceStreaming*   bitSource  =		  GetSoundHelper().Get_BitSource_Streaming();     
	return                         bitSource;
}

											////////////////////////////////////////


StreamingAudioplayer*     PitchPlayerDlg::Get_Audio_Player()
{

								//  If   'audioPlayer' is NULL,  just means that a song is NOT Loaded

	StreamingAudioplayer  *audioPlayer    =     GetSoundHelper().m_audioPlayer;  
	return                           audioPlayer;
}



					/////////////////////////////////////////////////


void    PitchPlayerDlg::WriteOut_SPitchCalc_Parms( ) 
{

	SoundHelper&     soundHelper =    GetSoundHelper();  
	PitchPlayerApp&     playerApp =    Get_PitchPlayerApp();


	if(     ! Is_Song_Loaded()    )
		return;


	StreamingAudioplayer   *streamingAudioPlayer =     soundHelper.m_audioPlayer;  
	  


ASSERT(   streamingAudioPlayer->m_doRealtimePitchDetection   ==   true   );   //   **** ALWAYS??    8/27/2012  ***********

	if(         streamingAudioPlayer->m_doRealtimePitchDetection  )
	{


		SPitchCalc   *sPitchCalcer     =       streamingAudioPlayer->Get_SPitchCalc();
		if(                sPitchCalcer  ==  NULL   )
		{
			AfxMessageBox(  "PitchPlayerDlg::WriteOut_SPitchCalc_Parms  FAILED,   m_sPitchCalc is NULL."   );
			return;
		}

							//  Now write out the values from the app,  that the user has set through the Dialog's controls during song play.
	
		sPitchCalcer->m_detectionSensitivityThreshold    =    playerApp.m_detectionSensitivityThreshold;



		if(    Midi_Source_Is_NoteList()     )
		{
			streamingAudioPlayer->m_rightChannelPercent  =    50;   //   For NoteList play,  do NOT let the AUDIBLE sound be affected by this controls setting   8/27/2012
		}
		else
			streamingAudioPlayer->m_rightChannelPercent  =    playerApp.m_rightChannelPercent;		 //



		sPitchCalcer->m_numberNeededToMatch   =     playerApp.m_numberNeededToMatch;     //  BOTH mus be changed at exactly the same time   
		sPitchCalcer->m_sizeOfPrimNotesCircque   =     playerApp.m_sizeOfPrimNotesCircque;   
	}   	
}



						////////////////////////////////////////


void   PitchPlayerDlg::Set_SPitchCalcs_Detail_Threshold(   int  curSliderPosition   ) 
{


	PitchPlayerApp&             playerApp  =      Get_PitchPlayerApp();
	SoundHelper&     soundHelper =    GetSoundHelper();  
	CString   retErrorMesg;


	double   playSpeedFlt  =   playerApp.m_playSpeedFlt;

	long   origPrimaryQueSize        =    playerApp.m_sizeOfPrimNotesCircque;
	long   originalPrimaryQueDelay =    playerApp.m_sizeOfPrimNotesCircque /2;




	if(   ! Is_Song_Loaded()    )
	{

		m_detailSliderCtrl.SetPos(  m_defaultDetailSliderPosition );   

		return;
	}


	SPitchCalc   *sPitchCalcer  =     Get_SPitchCalc();  
	ASSERT(     sPitchCalcer  );




//	originalPrimaryQueDelay  =    sPitchCalcer->Calc_Primary_CircQues_Delay_In_CalcedNotes();
	originalPrimaryQueDelay  =    sPitchCalcer->m_sizeOfPrimNotesCircque /2;
	



	ASSERT(  curSliderPosition >=  0    &&     curSliderPosition <=   100   );

	short       testCurPos =      m_detailSliderCtrl.GetPos();
//	ASSERT(   curSliderPosition  ==   testCurPos  );   The can be different when we load a new song.  11/11



	long       positionNudge     =     SPitchCalc::Calc_Position_Nudge_for_Detail_Slider(   playSpeedFlt   );


	double    filterSizeDecimal  =   (double)(  curSliderPosition  +  positionNudge  )    /   (double)m_detailSliderReduceFactor;    //    4.0;    //  gives 25 as highest value




	// ******  IF change  from 8.0, then must change the value of 'm_defaultDetailSliderPosition',  so that  default-DetailSlider-Position 
	//              will give  5.?   filterSize   from   Calc_Median_Filter_Ratio_by_Speed( below)  when the interface is reinitialiuzed by a new file being loaded.  
	//
	// ******************************************************************************************************************

//   Think about some type of Exponential  function to make it big at the end,  but more sensitive on the lower numbers. 



//	TRACE(  "Detail Slider Pos is  %d \n",    curSliderPosition   );   

	short    filterSize =  -1;
	short    retMatchCount =  -1; 
	long     lengthFinalCircQue =  -1;


	filterSize =   sPitchCalcer->Calc_Median_Filter_Ratio_by_Speed(   playSpeedFlt,    filterSizeDecimal,    retMatchCount   );


	playerApp.m_numberNeededToMatch    =     retMatchCount;   
	playerApp.m_sizeOfPrimNotesCircque   =     filterSize;             


	lengthFinalCircQue  =    sPitchCalcer->m_sizeOfFinalNotesCircque;    //   was changed by    Calc_Median_Filter_Ratio_by_Speed()



																						//   For Player.exe,  we must attend to a DELAY mechanism for AudioBytes    7/2012


	if(    sPitchCalcer->m_playModeUsingNotelist  !=  1    )     //  1:   NoteList Mode    ... does NOT have WAV delay for Notelist mode  
	{

		if(   ! soundHelper.ReAllocate_Players_AudioDelay_MemoryBuffer(   retMatchCount   )     )  
		{
			ASSERT( 0 );			
		}
	}





	Update_Detail_Controls_StaticText();    //  writes text from   playerApp.m_numberNeededToMatch   &   playerApp.m_sizeOfPrimNotesCircque

	UpdateData(  FALSE );

//	TRACE(   "\n\nNew  DETAIL filter setting:   [  match  %d     filterSize %d ]   SliderPos %d   [ %f  ]     [  newFINALcircQueSize  %d    ]\n\n",     
//		                                                   retMatchCount,    filterSize,   curSliderPosition,   filterSizeDecimal,    lengthFinalCircQue   );
}



											////////////////////////////////////////


void   PitchPlayerDlg::Update_Detail_Controls_StaticText()
{

	CString   strText;
	PitchPlayerApp&   playerApp =     Get_PitchPlayerApp();


	double   scaleFactor  =   (double)m_detailSliderReduceFactor;    //   4.0;


	short      curSliderPosition =      m_detailSliderCtrl.GetPos();



	long        positionNudge     =     SPitchCalc::Calc_Position_Nudge_for_Detail_Slider(   playerApp.m_playSpeedFlt    );

	double    filterSizeDecimal  =   (double)(  curSliderPosition   +  positionNudge  )   /    (double)m_detailSliderReduceFactor;    //    4.0;    //  gives 25 as highest value



	short   numberNeededToMatch  = 	playerApp.m_numberNeededToMatch;          
	short   filterSize     =                      playerApp.m_sizeOfPrimNotesCircque;     



	double   biggestFilterSizeDecimal  =    ( 100.0  +  (double)positionNudge )     /   scaleFactor;  


	double   displayNumeral  =     ( biggestFilterSizeDecimal   -   filterSizeDecimal )   *  4.0;   //  4.0   Get it back in range of { 100 - 0 } for display test. 



//	strText.Format(   "%.1f  [ %d ]",           displayNumeral,    numberNeededToMatch   );
	strText.Format(   "%d [%d]",     (long)displayNumeral,     numberNeededToMatch   );   // Do not really need the decimal, with this scale    2/12


	m_detailCStatic.SetWindowText(   strText   );
}



						////////////////////////////////////////


void     PitchPlayerDlg::On_NMReleasedCapture_MidiSync_Slider(  NMHDR *pNMHDR,   LRESULT *pResult   )
{


	long    defaultControlPosition  =     0;   

	long    settingsMultiplier          =   10;

	
	PitchPlayerApp&    playerApp  =    Get_PitchPlayerApp();
	SoundHelper&     soundHelper  =    GetSoundHelper(); 


	short   curVal        =     m_midiSyncSliderCtrl.GetPos();
	short   newSetting =    curVal;    //  

	
	if(        newSetting  <  0   )
		newSetting =     0;
	else if(  newSetting > 100  )
		newSetting =  100;


	short      calcedVal =  -1;
	CString   fullText;



	if(    ! Is_Song_Loaded()    )
	{												

		m_midiSyncSliderCtrl.SetPos(   defaultControlPosition  );

		fullText.Format(  "%d",    0   );
		m_midiSyncValueStatic.SetWindowText(   fullText   );

		m_midiSyncValueStatic.UpdateWindow();    //  to force the immediate 

		return;
	}




	if(   m_detectionMidiSourceRadio ==  1   )        //    1:   NOTELIST  mode
	{

//		calcedVal =     newSetting  /  settingsMultiplier   -  ( settingsMultiplier / 2 );       //   Range:  -5   to    +5
		calcedVal =     newSetting  /  settingsMultiplier;                                              //    Range:   0   to  +10


		playerApp.m_noteListHardwareDelayForPlayer  =   calcedVal;

		TRACE(    "\nNew Midi-SYNC value [  m_noteListHardwareDelayForPlayer   %d    ]  \n",     calcedVal     );
	}
	else
	{                                                                  //    0:    DETECTION  Mode	
		short  interVal =    newSetting  /  settingsMultiplier;


		if(   interVal  <  4   )    //   OVERIDE  mode
		{
			playerApp.m_overideDelay =    interVal;

			playerApp.m_baseDelay     =    0;    //   m_baseDelay    is ignored for this calc 

			calcedVal =   interVal;    
		}
		else			                 //    NORMAL  mode
		{					               
			playerApp.m_overideDelay =   -1;

			playerApp.m_baseDelay     =    interVal  -   4;       //     [     On_NMReleasedCapture_MidiSync_Slider    ]

			calcedVal =   interVal;    
		}



		if(   ! soundHelper.ReAllocate_Players_AudioDelay_MemoryBuffer(   playerApp.m_numberNeededToMatch   )     )  
		{
			AfxMessageBox(   "On_NMReleasedCapture_MidiSync_Slider  FAILED,  ReAllocate_Players_AudioDelay_MemBuffer"  );
			//	return;
		}

//		TRACE(    "\nNew Midi-SYNC value [   interVal   %d   ]  \n",     calcedVal     );
	}





	if(   m_detectionMidiSourceRadio ==  1  )  //   1:  NOTELIST  mode
	{
		fullText.Format(    "%d" ,      playerApp.m_noteListHardwareDelayForPlayer   );
	}
	else                                                       //   0:   DETECTION  Mode	
	{
		if(     playerApp.m_overideDelay  >=  0    )                              //   overide
//			fullText.Format(   "%d   [ 0 ]"  ,           playerApp.m_overideDelay  );
			fullText.Format(   "%d"  ,                     playerApp.m_overideDelay  );
		else
//			fullText.Format(   "%d   [ %d ]"  ,      ( playerApp.m_baseDelay  + 4 ),    playerApp.m_baseDelay  );
			fullText.Format(   "%d"  ,                  ( playerApp.m_baseDelay  + 4 ),    playerApp.m_baseDelay  );
	}



	m_midiSyncValueStatic.SetWindowText(   fullText   );

	m_midiSyncValueStatic.UpdateWindow();    //  to force the immediate 

	*pResult = 0;
}




									/////////////////////////////////////////////////////
									/////////////////////////////////////////////////////
									/////////////////////////////////////////////////////


void   PitchPlayerDlg::On_About_PitchscopePlayer_help()
{

								//   Just replace their function with mine,  for both debug and release.      8/16/2012

	SoundHelper&   soundHelper  =   GetSoundHelper(); 

	CString   retErrorMesg; 


	if(     soundHelper.Is_WAVplayer_Playing()      )
	{
		bool  succs  =     soundHelper.Pause_Play(   retErrorMesg  );	
		if(    !succs   )
			AfxMessageBox(  retErrorMesg  );
	}



	CString   retEditionNumber =   "GNU General Public License Version 2" ;

	/***
	CString   retEditionNumber;


	if(    !  theApp.Fetch_Edition_Number(  retEditionNumber  )      )
	{
									//  Just means that ONE of the fetches failed.   The others may be OK.    4/12
		ASSERT( 0 );
		int   dummy =   9;
	}
	***/



	AboutDialogRelease   aboutDialogRelease(  retEditionNumber  );

	aboutDialogRelease.DoModal();
}





									/////////////////////////////////////////////////////
									/////////////  FILE  Commands  ////////////////
									/////////////////////////////////////////////////////
	

void   PitchPlayerDlg::On_File_Open_WAV()
{


	CString   retErrorMesg;
	bool        retUserCanceled =   false;

	PitchPlayerApp&   playerApp      =   Get_PitchPlayerApp();
	SoundHelper&    soundHelper  =    GetSoundHelper(); 


	if(     soundHelper.Is_WAVplayer_Playing()      )
	{
		bool  succs  =     soundHelper.Pause_Play(   retErrorMesg  );	
		if(    !succs   )
			AfxMessageBox(  retErrorMesg  );
	}



	if(    ! playerApp.On_View_SRC_File(   retUserCanceled,    retErrorMesg   )    )
	{
		AfxMessageBox(  retErrorMesg  );	
		return;
	}



	if(   ! retUserCanceled    )
	{	

		CClientDC   dc( this ); 

		m_stereoBalanceDetection.SetPos(  50  );    //  always set it for balanced( 50%) stero on load of a new file

		playerApp.m_rightChannelPercent =    50;


		SPitchCalc  *sPitchCalc  =    Get_EventMan().Get_SPitchCalc();   //  a NEW  'SPitchCalc'   is allocated in  Load_NoteList()
		ASSERT(      sPitchCalc  );



		Get_EventMan().Initialize_MusicalKey_Note_Counters();


		Restore_Default_Settings_to_Controls(  *sPitchCalc  );  



		m_musicalKeyDetectionState =   1;   //   1:  Auto-Assign mode,  looking to calc and auto-CHANGE the control   [ Restore_Default_Settings_to_Controls  just set it to 0 ) 


				
		CString   retFilesName; 
		External::Get_Files_Name_from_Path(    playerApp.m_sourceFilesPath,     retFilesName   );
		Write_Dialogs_Title_Bar(  retFilesName  );
		

		soundHelper.m_lastTransportDirection       =   SoundHelper::FILEsTART;     //  This work differently in Player.exe than Navigator,  but can still trigger a FileSeek
		soundHelper.m_lastTransportActionActual  =   SoundHelper::FILEsTART;  


		Render_Revolvers_Background(  dc  );
	//	AfxMessageBox(   "Music file has been loaded."   );
	}
}




											////////////////////////////////////////


void   PitchPlayerDlg::On_Load_Notelist_FileMenu()
{
		
	SoundHelper&         soundHelper  =    GetSoundHelper();  
	PitchPlayerApp&      playerApp      =   Get_PitchPlayerApp();
	CString  retErrorMesg,   strMesg,    listFilesPath;



	if(     soundHelper.Is_WAVplayer_Playing()      )
	{
		bool  succs  =     soundHelper.Pause_Play(   retErrorMesg  );	
		if(    !succs   )
			AfxMessageBox(  retErrorMesg  );
	}




	bool   retUserBailsOut =  false,     retUserCanceledFileSave =  false;	

	if(    ! playerApp.Query_User_For_ObjectList_Save(   retUserCanceledFileSave,   retUserBailsOut,    retErrorMesg   )   )
	{
		AfxMessageBox(   retErrorMesg   );		
//		return;   *********   Do I want to bail out in this case ???   2/12
	}

	if(   retUserBailsOut   )
		return;    //  user decides that they do not want to do the  'CALLING command'    7/07






	CFileDialog   dlg(	  TRUE,		  //  TRUE:  File open
									_T(    "pnl"   ),
									NULL,                         
									OFN_HIDEREADONLY,     // |   OFN_OVERWRITEPROMPT,
									_T(  "NoteList Files  (*.pnl) | *.pnl|"  ) 			
							);


	if(   dlg.DoModal()  !=   IDOK  )
	{
		//  retErrorMesg =  "User canceled the load of a WAV file." ;
		//   retUserCanceled =   true;
		return;
	}
			
	listFilesPath =    dlg.GetPathName();




	bool   retUserCanceled =  false;


	if(    ! playerApp.Load_NoteList(   listFilesPath,   retUserCanceled,    retErrorMesg  )     )   // allocs a NEW  SPitchCalc  
	{
		AfxMessageBox(  retErrorMesg  );
		return;
	}

													
	if(    retUserCanceled   )    //   user might have CANCELED if the VERSIONs Looked like a CONFLICT     8/2012
		return;




	SPitchCalc  *sPitchCalcer    =    Get_SPitchCalc();   //  a   NEW  'SPitchCalc'   was allocated in  Load_NoteList()
	ASSERT(      sPitchCalcer  );


	StreamingAudioplayer   *audioPlayer  =    soundHelper.m_audioPlayer;    //  a NEW  'Audioplayer'   is also allocated in  Load_NoteList()
	ASSERT(  audioPlayer  );



	CString  retNoteListFileName,   musicKeyText;

	if(    ! External::Get_Files_Name_from_Path(   listFilesPath,    retNoteListFileName   )    )
	{
		ASSERT( 0 );
		retNoteListFileName =    listFilesPath;    // ***** Do I want this ????   12/11
	}


	/***   title bar is too small for this much text

	CString   retMusicFilesName,    mesgCompound; 
	External::Get_Files_Name_from_Path(    playerApp.m_sourceFilesPath,     retMusicFilesName   );
	mesgCompound.Format(  "%s       [   %s   ]" ,    retNoteListFileName,    retMusicFilesName      );
	Write_Dialogs_Title_Bar(  mesgCompound  );
	****/
	CString   retMusicFilesName,    mesgCompound; 
	External::Get_Files_Name_from_Path(    playerApp.m_sourceFilesPath,     retMusicFilesName   );
	mesgCompound.Format(  "%s" ,    retNoteListFileName   );
	Write_Dialogs_Title_Bar(  mesgCompound  );




	short   holdMusicalKey  =     sPitchCalcer->m_musicalKey;    // this was assigned during the loading of the NoteList file



	Restore_Default_Settings_to_Controls(  *sPitchCalcer  );



	sPitchCalcer->m_musicalKey  =    holdMusicalKey;    //    Restore_Default_Settings_to_Controls()  set this to 0

	m_musicalKeyDetectionState  =    2;       //  2:  Disable AUTO-set,   because the NoteList file ASSIGNED the correct MusicalKey   1/12
 


	
	if(     audioPlayer  !=  NULL    )
		audioPlayer->Set_LastPlayed_SampleIdx(  0  );   //  think that PreRoll  did this in Navigator   2/27/12
	else
	{  ASSERT( 0 ); }    //  Need ERROR reporting ????  Or is this never reached?     1/12/2010


//	long  lastPlayed2 =     audioPlayer->Get_LastPlayed_SampleIdx();     



	UpdateData( false );      //  false:   Update the display with new speed setting



	Change_Midi_Source(  1,   true  );   //  Switch to NOTELIST play-MODE.  Can be a little SLOW because it must also call Do_PreRoll_Redraw_Display()   


	UpdateData( false );  //  Change_Midi_Source()  change a display variable for the RadioButtons,  need this to have it take effect  2/27/2012



	CClientDC  dc( this ); 
	Render_Revolvers_Background(  dc  );


	strMesg.Format(   "%s  has been loaded from file. \n\nThe MIDI SOURCE has been changed to NOTELIST." ,    retNoteListFileName    );
	AfxMessageBox(   strMesg   );		
}




										////////////////////////////////////////
										//////////   TRANSPORT  ////////////
										////////////////////////////////////////


void PitchPlayerDlg::OnContinuePlayButton()
{

	CString  retErrorMesg;
	SoundHelper&       soundHelper  =    GetSoundHelper(); 
	PitchPlayerApp&    playerApp      =    Get_PitchPlayerApp();



	if(     ! Is_Song_Loaded()    )
	{  
		AfxMessageBox(   "Load a Music File ( .MP3  or .WAV )."   );
		return;
	}


	bool   wasPlayingBackward  =   Is_Playing_Backwards(); 

	long   lastFilePositionApp     =    playerApp.m_lastFilePosition;  



	if(     soundHelper.Is_WAVplayer_Playing()      )
	{
		bool  succs  =     soundHelper.Pause_Play(   retErrorMesg  );	
		if(    !succs   )
			AfxMessageBox(  retErrorMesg  );


		if(   ! wasPlayingBackward  )   // If it was going FORWARDS, just STOP and do NOT continue play.  But if going BACKWARDS, let the change in play direction happen below.
			return;
	}



	BitSourceStreaming  *bitSource  =     Get_BitSource_Streaming();
	ASSERT(                   bitSource   );


	bitSource->Initialize_For_Playing();   // ***  NEW,  8/22/2012   OK ?????

	


	if(     ! soundHelper.Continue_Play_Forward(   playerApp.m_playSpeedFlt,   false,    lastFilePositionApp,    retErrorMesg  )     )   
		AfxMessageBox(  retErrorMesg  );



	soundHelper.m_lastTransportDirection   =       SoundHelper::CONTINUEfORWARD;     // This means we will NOT have to do a FileSeek  
	soundHelper.m_lastTransportActionActual  =   SoundHelper::CONTINUEfORWARD;
}



									/////////////////////////////////////////////////////


void PitchPlayerDlg::OnReversePlayButton() 
{

	CString  retErrorMesg;

	SoundHelper&       soundHelper  =    GetSoundHelper(); 
	PitchPlayerApp&    playerApp      =    Get_PitchPlayerApp();


	if(     ! Is_Song_Loaded()    )
	{  
		AfxMessageBox(   "Load a Music File ( .MP3  or .WAV )."   );
		return;
	}


	bool   wasPlayingBackward  =   Is_Playing_Backwards(); 




	if(     soundHelper.Is_WAVplayer_Playing()      )
	{
		bool  succs  =     soundHelper.Pause_Play(   retErrorMesg  );	
		if(    !succs   )
			AfxMessageBox(  retErrorMesg  );

		if(   wasPlayingBackward   )     // If it was going BACKWARDS, just STOP and do NOT continue play.  But if going FORWARDS, let the change in play direction happen below.
			return;
	}

	
	BitSourceStreaming  *bitSource  =     Get_BitSource_Streaming();
	ASSERT(                   bitSource   );


	bitSource->Initialize_For_Playing();   // ***  NEW,  8/22/2012   OK ?????




	if(     ! soundHelper.Continue_Play_Backwards(    playerApp.m_playSpeedFlt,   false,  0L,    retErrorMesg  )     )   
		AfxMessageBox(  retErrorMesg  );



	soundHelper.m_lastTransportDirection      =    SoundHelper::CONTINUEbACKWARDS;   // This means we will NOT have to do a FileSeek,   if we keep pressing the BACKWARDS Button
	soundHelper.m_lastTransportActionActual  =    SoundHelper::CONTINUEbACKWARDS;
}



									/////////////////////////////////////////////////////


void PitchPlayerDlg::OnPausePlayButton()
{

	CString    retErrorMesg;

	bool  succs =    GetSoundHelper().Pause_Play(   retErrorMesg  );	  //   This will tell the USER if NO song is loaded.
	if(   ! succs   )
		AfxMessageBox(  retErrorMesg  );


	m_eventCounterMusicalKey =   0;  


// GetSoundHelper().m_lastTransportDirection       =   SoundHelper::PAUSEpLAY;       *** NEVER assign this memberVar for a PAUSE ***

	GetSoundHelper().m_lastTransportActionActual  =   SoundHelper::PAUSEpLAY;
}



									/////////////////////////////////////////////////////


void  PitchPlayerDlg::OnGoFilesStartButton()
{
	
	CString   retErrorMesg;
	SoundHelper&    soundHelper  =    GetSoundHelper(); 



	if(     soundHelper.Is_WAVplayer_Playing()      )			 //   This will tell the USER if NO song is loaded.
	{
		bool   succs  =     soundHelper.Pause_Play(   retErrorMesg  );	 
		if(    ! succs   )
			AfxMessageBox(  retErrorMesg  );
	}



	BitSourceStreaming  *bitSource  =     Get_BitSource_Streaming();
	ASSERT(                   bitSource   );


	bitSource->Initialize_For_Playing();   // ***  NEW,  8/22/2012   OK ?????



	Erase_Last_Bullet();



	if(     ! soundHelper.Move_to_Files_Start(  retErrorMesg  )     )   //  Funky function,  only call it from  PLAYER?   Will erase the DELAYbuffers, and harware buffer.
		AfxMessageBox(  retErrorMesg  );	


	soundHelper.m_lastTransportDirection       =     SoundHelper::FILEsTART;   //  This work differently in Player.exe than Navigator,  but can still trigger a FileSeek
	soundHelper.m_lastTransportActionActual  =     SoundHelper::FILEsTART;
}




									/////////////////////////////////////////////////////


void PitchPlayerDlg::OnGoFilesEndButton()
{

	CString   retErrorMesg;
	SoundHelper&    soundHelper  =    GetSoundHelper(); 


	if(     soundHelper.Is_WAVplayer_Playing()      )              //   This will tell the USER if NO song is loaded.
	{
		bool  succs  =     soundHelper.Pause_Play(   retErrorMesg  );	
		if(    !succs   )
			AfxMessageBox(  retErrorMesg  );
	}



	BitSourceStreaming  *bitSource  =     Get_BitSource_Streaming();
	ASSERT(                   bitSource   );


	bitSource->Initialize_For_Playing();   // ***  NEW,  8/22/2012   OK ?????



	Erase_Last_Bullet();



	if(     ! soundHelper.Move_to_Files_End(  retErrorMesg  )     )    //  Funky function,  only call it from  PLAYER?   Will erase the DELAYbuffers, and harware buffer.
		AfxMessageBox(  retErrorMesg  );	


	soundHelper.m_lastTransportDirection      =     SoundHelper::FILEeND;
	soundHelper.m_lastTransportActionActual  =    SoundHelper::FILEeND;
}







					/////////////////////////////////////////////////
					/////////////////////////////////////////////////
					/////////////////////////////////////////////////
					/////////////////////////////////////////////////


void    PitchPlayerDlg::Player_Messaging(   bool&   isDone  ) 
{


	long   numEventsForMusicalKeyCalc =    150;     //    150 [ every 10 seconds ?  ]      500  [ not too often]    ***** ADJUST ******



	CString   retErrorMesg;
	MSG       msg;

	EventMan&          eventMan =     Get_EventMan();
	SoundHelper&     soundHelper  =    GetSoundHelper(); 
	StreamingAudioplayer   *audioPlayer  =	  soundHelper.Get_Streaming_Audioplayer();     



	DWORD  dwEvt  =  
	      MsgWaitForMultipleObjects(          eventMan.m_numberOfNotificationEvents  +1,										    //    Number of events  plus one
								                           Get_EventMan().m_notificationEventHandles,              // Location of handles. ***CAREFUL ***    JPM
															FALSE,                  // Wait for all?
															INFINITE,              // How long to wait?

															QS_ALLINPUT  );       //    QS_ALLINPUT  QS_ALLEVENTS (buggy, see below)     // Any message is an event.

		//  *****************  MUST be   QS_ALLINPUT,   or  HTML Help  will hang up ( non responding for long periods  )    6/2012  *************




													   // WAIT_OBJECT_0 == zero but is properly treated as an arbitrary
													   // index value assigned to the first event, therefore we subtract
													   // it from dwEvt to get the zero-based index of the event.
	dwEvt  -=   WAIT_OBJECT_0;



													//  If the event was set by the buffer,  you have input to process.

	if(         dwEvt   <   eventMan.m_numberOfNotificationEvents    ) 
    {


		SPitchCalc   *sPitchCalcer     =     eventMan.Get_SPitchCalc();
		if(                 sPitchCalcer  ==  NULL  )
			AfxMessageBox(   "PitchPlayerDlg::Player_Messaging FAILED,  sPitchCalcer = NULL."   );


		SequencerMidi   *midiSequencer  =	    soundHelper.Get_Current_MidiSequencer();
		ASSERT(             midiSequencer  );




													//   Test that we get all the Events that we should (should be in increasing order )

		long                 eventCount   =     eventMan.m_numberOfNotificationEvents;  
		unsigned long   hemisEvent  =    ( (unsigned long)( eventCount -1 ) )  /2;   // The event at 180 degrees,    for Player is 30

		bool    outOfSync =   false;   
		long    expectedEventNumber  =  -7;



		eventMan.m_lastEventNumber++;    //  Now it is what THIS event should be.

		if(    eventMan.m_lastEventNumber   >=   (eventCount -1)   )      //    60  
			eventMan.m_lastEventNumber =   0;



		if(     dwEvt   !=    eventMan.m_lastEventNumber    )
		{
																// *** Do NOT really need this for Player.exe,  but it is a good test area for these event problems   3/11
			long    realLastEvent  =    eventMan.m_lastEventNumber  -1;

			if(    realLastEvent  < 0   )
				realLastEvent =   0;


			long   anticipatedDataFetchCount  =   eventMan.m_curAudioPlayer->m_dataFetchCount;

			if(   dwEvt  ==   0    ||     dwEvt ==  hemisEvent   )     //   30:    hemisphere event
				anticipatedDataFetchCount++;    //  increment because that is what will happen next in   Process_Event_Notification_PPlayer

			/****
			TRACE(   "\nOut of SYNC [ %d  count]:    THIS event[  %d,    should be:  %d  ]     previous-Event[  %d  ]    DFetch %d   [ %d bytes processed ]   [ %d count ]   \n",    
									 Get_EventMan().m_outSyncCountWavMidiPlay,   		
				                     dwEvt,     eventMan.m_lastEventNumber,   //  should be
									 realLastEvent,     //    lastEventNumGLB,  

									  anticipatedDataFetchCount,
									eventMan.m_curAudioPlayer->m_srcBytesProcessed  					                    																		
						  ); 
			  ****/


			expectedEventNumber =   eventMan.m_lastEventNumber;

			eventMan.m_lastEventNumber  =   dwEvt;

			eventMan.m_outSyncCountWavMidiPlay++;     //   outOfSyncCountGLB++;



			short  appCode  =      Get_PitchScope_App_Code_GLB();      //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope

			if(      appCode  ==  1   )    //  Navigator ONLY
			{
				outOfSync =   true;  
				eventMan.m_outOfSyncStatusCode  =  1;     //    1:  OutOfSync event found,  waiting for HemisphereEvent to clean it up   
			}
			else
			{  outOfSync =   false;      // *************************   DISABLE  *********
				eventMan.m_outOfSyncStatusCode  =  0;     //    1:  OutOfSync event found,  waiting for HemisphereEvent to clean it up   
			}
		}




		m_eventCounterMusicalKey++;   // Need to count the notes that we are polling to calc the Musical Key.    1/12
					
		WriteOut_SPitchCalc_Parms();   //   Copy the detection settings before we do calcs
 


		eventMan.Process_Event_Notification_PPlayer(   dwEvt,     outOfSync,    expectedEventNumber,    eventMan.m_pieSliceCounter   );   //  Copy data to audio PLAY-buffer,  SndSample for PitchDetect, do Detection, play MidiNote



		bool   isPlayingMusic =    false;  

		if(    audioPlayer  !=  NULL   )
			isPlayingMusic  =   audioPlayer->m_isPlaying;



		short   nuAvgHarmMag  =    (short)(    m_notesColorBrighten   *   (double)( eventMan.m_curScalePitchAvgHarmonicMag )    );   //  make the NOTES look brighter on the display

		short   sPitch  =   eventMan.m_curScalePitch;                



		if(   sPitch  >=  -1   )
		{	


			if(    isPlayingMusic   )                     //  OMITING this will actally ERASE the Bullet when PLAY STOP  
			{
				Render_Revolvers_Bullet_ToOffmap(   sPitch,   nuAvgHarmMag   );   //    For each event it will render, regardless if a new note has started or not.
			}																						               //     ...that is why the color value changes even on a long single note.   1/10



			if(    m_eventCounterMusicalKey  >=   numEventsForMusicalKeyCalc    )    //  Calculation of MUSUCAL KEY,  goes on indefinately.   1/12
			{

				short  retMKeyBestProfiles,  retMKeySecondBestProfiles,  retMKeyThirdBestProfiles;    //  ,   retBestScore,  retSecondBestScore;  


				bool  sucess =    SPitchCalc::Calculate_Musical_Key(   -1,    eventMan.m_noteScoresMusicalKey,    eventMan.m_noteCountMusicalKey,       
					                                                                           retMKeyBestProfiles,   retMKeySecondBestProfiles,   retMKeyThirdBestProfiles,    retErrorMesg  );  
				if(        sucess  
					&&   m_musicalKeyDetectionState ==  1   )
				{

		//			Write_to_MusicalKey_Suggestions_to_Control(    retMKeyBestProfiles,    retMKeySecondBestProfiles,   retMKeyThirdBestProfiles,   retBestScore,    retSecondBestScore   );

					if(   sPitchCalcer  !=  NULL   )          //  This means that a file is loaded  
					{
						bool   keyUsuallyLikesSharps  =      SPitchCalc::Does_MusicalKey_Use_Sharps(  retMKeyBestProfiles  );

						if(     keyUsuallyLikesSharps  )
							m_userPrefMusicalKeyAccidentals =  1;       //   0:  No preference     1:  Use Sharps    2:  UseFlats
						else
						    m_userPrefMusicalKeyAccidentals =  2;   


						sPitchCalcer->m_musicalKey  =    retMKeyBestProfiles;	   //  BIG,   This is the memberVar that controls the calcs 
					}

					//   TRACE(   "\n...just sucessfully calced MUSICAL KEY.   \n"    );   
				}

				m_eventCounterMusicalKey =   0;
			}   //  if(    m_eventCounterMusicalKey  >=

		}
		else
		{   ASSERT( 0 ); 	} //  error got a bad value			
	}
		

	else if(   dwEvt  ==     eventMan.m_numberOfNotificationEvents            )			//   ONLY  'Record'   Event			[  21   ]
    {         
														//  The next event in the array is the one set by the output buffer
														//  when it has been told to play remaining data and stop.

	ASSERT( 0 );   //    Does NOT get CALLED 

        Get_EventMan().Stop_Output();
	}

	else if(   dwEvt  ==    eventMan.m_numberOfNotificationEvents  +1     )         //   If it's the last event,  it's a message.             
    {         
																									 
        while(    PeekMessage(   &msg,   NULL,   0,  0,   PM_REMOVE  )    ) 
        {                

			
//				if(   TranslateAccelerator(   hWndMain,   hAccel,   &msg  )    )
//					continue;
//		WM_KEYDOWN 


			if(    ! ContinueModal()    )   
			{

				Shutting_Down();

				isDone =    true;

		//   	AfxPostQuitMessage( 0 );     want this ????    Do not think that I need it
				break;
			}

				

            if(    msg.message  ==   WM_QUIT   )     //   If it's a quit message, we're outta here.
            {

			ASSERT( 0 );     //  NEVER get here when this is a dialog  

                isDone =    true;     //  Never get here when this is a dialog??  
            } 
			else if(    msg.message ==   VK_DELETE    )
			{
				int  dummy = 9;
			}
            else 
            {  TranslateMessage(  &msg  );
                DispatchMessage(   &msg  );
            }
        }
    } 


	int   dummy =   9;
}



						////////////////////////////////////////
						////////////////////////////////////////



#define DELETE_EXCEPTION(e) do { if(e) { e->Delete(); } } while (0)



INT_PTR    PitchPlayerDlg::DoModal()
{
						// ***  My OVERIDE of MSofts code.     ( also did this in   abbOldPrograms\MidiLowLevel.exe  )    12/09


														// can be constructed with a resource template or InitModalIndirect

	ASSERT(m_lpszTemplateName != NULL || m_hDialogTemplate != NULL ||
		m_lpDialogTemplate != NULL);



																									// load resource as necessary
	LPCDLGTEMPLATE   lpDialogTemplate =   m_lpDialogTemplate;

	HGLOBAL      hDialogTemplate =   m_hDialogTemplate;

	HINSTANCE  hInst =    AfxGetResourceHandle();


	if (m_lpszTemplateName != NULL)
	{
		hInst = AfxFindResourceHandle(m_lpszTemplateName, RT_DIALOG);
		HRSRC hResource = ::FindResource(hInst, m_lpszTemplateName, RT_DIALOG);
		hDialogTemplate = LoadResource(hInst, hResource);
	}

	if (hDialogTemplate != NULL)
		lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);



																// return -1 in case of failure to load the dialog template resource
	if (lpDialogTemplate == NULL)
		return -1;


																// disable parent (before creating dialog)
	HWND hWndParent =    PreModal();


//	AfxUnhookWindowCreate();		***************************  PUT BACK,  or not needed here.  12/09 *************************

	BOOL bEnableParent =  FALSE;


#ifndef _AFX_NO_OLE_SUPPORT
	CWnd*   pMainWnd = NULL;
	BOOL     bEnableMainWnd =   FALSE;
#endif
	if (   hWndParent && hWndParent != ::GetDesktopWindow() && ::IsWindowEnabled(hWndParent))
	{
		::EnableWindow(hWndParent, FALSE);
		bEnableParent = TRUE;
#ifndef _AFX_NO_OLE_SUPPORT
		pMainWnd =    AfxGetMainWnd();
		if (pMainWnd && pMainWnd->IsFrameWnd() && pMainWnd->IsWindowEnabled())
		{
									// We are hosted by non-MFC container
			pMainWnd->EnableWindow(FALSE);
			bEnableMainWnd = TRUE;
		}
#endif
	}

	TRY
	{								// create modeless dialog

//		AfxHookWindowCreate(  this  );        ***************************  PUT BACK,  or not needed here.  12/09 *************************



		if(   CreateDlgIndirect(   lpDialogTemplate,               //  *** here is where   this->m_hWin   finally gets assigned a value  **** 
						                    CWnd::FromHandle(hWndParent),  
											hInst   )  )
		{
			if(   m_nFlags  &  WF_CONTINUEMODAL   )
			{
																					// enter modal loop
				DWORD dwFlags =   MLF_SHOWONIDLE;

				if(    GetStyle()  &  DS_NOIDLEMSG   )
					dwFlags |=  MLF_NOIDLEMSG;



				VERIFY(    RunModalLoop_JM( dwFlags )   ==  m_nModalResult    );    // ********  SUBSTITUTE my message loop       RunModalLoop_JM(     12/09
			}


															// hide the window before enabling the parent, etc.
			if (  m_hWnd != NULL  )
				SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW|
					SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
		}
	}
	CATCH_ALL(e)
	{
		DELETE_EXCEPTION(e);
		m_nModalResult = -1;
	}
	END_CATCH_ALL


#ifndef _AFX_NO_OLE_SUPPORT
	if (bEnableMainWnd)
		pMainWnd->EnableWindow(TRUE);
#endif
	if (bEnableParent)
		::EnableWindow(hWndParent, TRUE);
	if (hWndParent != NULL && ::GetActiveWindow() == m_hWnd)
		::SetActiveWindow(hWndParent);


										// destroy modal window
	DestroyWindow();
	PostModal();


																							// unlock/free resources as necessary
	if (m_lpszTemplateName != NULL || m_hDialogTemplate != NULL)
		UnlockResource(hDialogTemplate);

	if (m_lpszTemplateName != NULL)
		FreeResource(hDialogTemplate);

	return   m_nModalResult;
}



					/////////////////////////////////////////////////


int    PitchPlayerDlg::RunModalLoop_JM(  DWORD dwFlags  )
{

					//  *********   my OVERIDE   ...careful    BUT it works !!!    12/09 ****************************


	ASSERT(  ::IsWindow( m_hWnd )  );                 // window must be created

	ASSERT(  !( m_nFlags & WF_MODALLOOP )  );   // window must not already be in modal state

												// for tracking the idle time state
	BOOL  bIdle = TRUE;
	LONG  lIdleCount  = 0;
	BOOL  bShowIdle  =    (dwFlags & MLF_SHOWONIDLE) && !(GetStyle() & WS_VISIBLE);
	HWND hWndParent =   ::GetParent(m_hWnd);

	m_nFlags |= (WF_MODALLOOP|WF_CONTINUEMODAL);



	bool       done =  false;

    while(  ! done   )				//  The main loop.
    {

		Player_Messaging(  done  );		
    }  



	m_nFlags &= ~(WF_MODALLOOP|WF_CONTINUEMODAL);
	return   m_nModalResult;
}







#define MLF_NOKICKIDLE      0x0002    // don't send WM_KICKIDLE messages      ***** DANGEROUS,  I hardwired these in.  What if they change ??  12/09
#define WM_KICKIDLE      874		      // don't send WM_KICKIDLE messages      ***** DANGEROUS,  I hardwired these in.  What if they change ??  12/09



int    PitchPlayerDlg::RunModalLoop_JM_OLD(  DWORD dwFlags  )
{

					//  *********  OLD,  unused function.  But study it.    


	ASSERT(::IsWindow(m_hWnd)); // window must be created
	ASSERT(!(m_nFlags & WF_MODALLOOP)); // window must not already be in modal state

	// for tracking the idle time state
	BOOL  bIdle = TRUE;
	LONG  lIdleCount  = 0;
	BOOL  bShowIdle  =    (dwFlags & MLF_SHOWONIDLE) && !(GetStyle() & WS_VISIBLE);
	HWND hWndParent =   ::GetParent(m_hWnd);

	m_nFlags |= (WF_MODALLOOP|WF_CONTINUEMODAL);

	MSG *pMsg =   AfxGetCurrentMessage();


								// acquire and dispatch messages until the modal state is done
	for( ; ;  )
	{
		ASSERT(  ContinueModal()   );


																	//  A)   phase1: check to see if we can do idle work
		while (     bIdle   &&
		   	        ! ::PeekMessage( pMsg, NULL, NULL, NULL, PM_NOREMOVE )   )
		{

			ASSERT(  ContinueModal()  );

			// show the dialog when the message queue goes idle
			if (bShowIdle)
			{
				ShowWindow(SW_SHOWNORMAL);
				UpdateWindow();
				bShowIdle = FALSE;
			}

			// call OnIdle while in bIdle state
			if (!(dwFlags & MLF_NOIDLEMSG) && hWndParent != NULL && lIdleCount == 0)
			{
				// send WM_ENTERIDLE to the parent
				::SendMessage(hWndParent, WM_ENTERIDLE, MSGF_DIALOGBOX, (LPARAM)m_hWnd);
			}


			if (   (dwFlags & MLF_NOKICKIDLE)  ||
				!  SendMessage(   WM_KICKIDLE,   MSGF_DIALOGBOX,   lIdleCount++)    )    // ****** BAD,  I hardwired the value for   WM_KICKIDLE     12/09 
			{
				// stop idle processing next time
				bIdle = FALSE;
			}
		}



																	// B)    phase2:     pump messages while available
		do
		{  ASSERT(  ContinueModal()  );

											
			
//			if(   ! AfxPumpMessage()                        )		// pump message, but quit on WM_QUIT
//			if(    ! AfxInternalPumpMessage_JM()    )
//			{
//				AfxPostQuitMessage( 0 );
//				return -1;
//			}			
			_AFX_THREAD_STATE  *pState =   AfxGetThreadState();          //  this is essence of code for  AfxInternalPumpMessage_JM()

			if(   ! ::GetMessage(  &( pState->m_msgCur ),   NULL, NULL, NULL  )    )
			{
				//  prevents calling message loop things in 'ExitInstance'   will never be decremented.   "CWinThread::PumpMessage - Received WM_QUIT."						
				AfxPostQuitMessage( 0 );
				return -1;
			}


																							// show the window when certain special messages rec'd
			if(   bShowIdle &&
				(   pMsg->message == 0x118   ||  pMsg->message == WM_SYSKEYDOWN))
			{
				ShowWindow(  SW_SHOWNORMAL  );
				UpdateWindow();

				bShowIdle =   FALSE;
			}


			if(    ! ContinueModal()    )
				goto  ExitModal;


																	// reset "no idle" state after pumping "normal" message
			if(   AfxIsIdleMessage( pMsg )    )
			{
				bIdle = TRUE;
				lIdleCount = 0;
			}


		} while (   ::PeekMessage(  pMsg,    NULL, NULL, NULL,   PM_NOREMOVE )    );


	}   //  	for( ; ;  )



ExitModal:
	m_nFlags &= ~(WF_MODALLOOP|WF_CONTINUEMODAL);
	return   m_nModalResult;
}












