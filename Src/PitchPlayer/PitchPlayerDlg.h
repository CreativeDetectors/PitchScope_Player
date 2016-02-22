/////////////////////////////////////////////////////////////////////////////
//
//  PitchPlayerDlg.h   -   
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


#include "afxcmn.h"
#include "afxwin.h"


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

class   SPitchCalc;

class  BitSourceStreaming;

class  StreamingAudioplayer;

class  WavConvert;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   PitchPlayerDlg     : public CDialog
{

public:
	PitchPlayerDlg( CWnd* pParent = NULL );	



	BitSourceStreaming*       Get_BitSource_Streaming();

	StreamingAudioplayer*    Get_Audio_Player();

	SPitchCalc*                     Get_SPitchCalc();

//	WavConvert*                  Get_WavConvert();    I am trying to keep this OUT,  it is too low level.    8/2012




	bool			Is_Song_Loaded();

	bool			Is_Playing();

	bool			Is_Playing_Backwards();     //  safely uses the AUDIOPLAYER to determine


	bool		    Midi_Source_Is_NoteList();     //   FALSE  is  Detection mode



	
	void			Restore_Default_Settings_to_Controls(   SPitchCalc&   sPitchCalcer   );

	void			WriteOut_SPitchCalc_Parms();     

	void          Enable_Detection_Controls(  bool  enable  );



	void			User_Message_Stop_Play(  char*  mesg   );

	bool			Change_FontCd(    FontsCd&  fontCd,   short  fontID,   CString&  retErrorMesg  );


	bool			Change_Midi_Source(   short  newPlayMode,    bool  loadingNewFile  );

	bool			Fetch_Serial_Number(   CString&   retSerialNumber   ); 




	void			Render_Last_Amimation_Frame();   //  called when Play has STOPPED,  and need to render BULLETS to Revolver and Gagues    2/12

	void		    Paint_Bullets_to_Stationary_Gagues();

	void			Render_Revolvers_Background(   CDC&  dc  );

	void		    Render_Revolvers_Bullet_ToOffmap(    short  foundScalePitch,     short  spitchsVolume   );    //  main funct for the outside 

	void			Erase_Last_Bullet();   




	void			Set_MidiSync_Controls_Position();

	void			Set_SPitchCalcs_Detail_Threshold(   int  curSliderPosition   );

	void		    Update_Detail_Controls_StaticText();



	void			Set_Soundhelpers_WAV_Volume(  int  curSliderPosition   ); 
	void			Set_Soundhelpers_Midi_Volume(  int  curSliderPosition   ); 


	void			Sync_FileSlider_Control(   long   panesSampleCount,   long  firstSampleIdx   );


	void			Update_MidiInstrument_Name_Text(   short   instrumentCode   ); 


	void			Write_Dialogs_Title_Bar(   CString&  songTitle   );







	virtual       INT_PTR   DoModal();   

	int				RunModalLoop_JM(           DWORD dwFlags   );
	int		        RunModalLoop_JM_OLD(   DWORD dwFlags   );      // NOT USED,  but good for info if mine goes wrong  12/09

	void          Player_Messaging(   bool&  isDone   );

	void			Shutting_Down();      //  called  just BEFORE  Navigator shuts down




public:
	GaugeRevolver    m_gaugeOfRevolver;   // draws the Bullets in the radial animation

	short   m_panesWidthRevolver,    m_panesHeightRevolver;      //  For the Revolver's PANE
	short   m_xOffsetRevolver,      m_yOffsetRevolver;

	short   m_revolversBackgroundGray;



	double  m_notesColorBrighten;    //  Make the  ALL   Notes and Bullets  look brighter     NEW,  2/12



	int	      m_lastScrollPositionAbsoluteHorz;

	long    m_numberOfUnitsForFilePositionSlider;    //  200 ???



	long	    m_eventCounterMusicalKey;  

	short		m_musicalKeyDetectionState;     //    0:   Disable AUTO-set (user has touched control or a NoteList assigned MKey              
																//    1:   Init,  waiting to calc key,     2: Now automatically writing

	short    m_userPrefMusicalKeyAccidentals;      //   0:  No preference     1:  Use Sharps    2:  UseFlats



	short    m_defaultDetailSliderPosition;   //   at initialization  

	long     m_detailSliderReduceFactor;    //  4     2/12



	short	   m_midiInstrumentsControlsValue;



	CBitmapButton   m_gotoFilesStartButton;
	CBitmapButton   m_gotoFilesEndButton;
	CBitmapButton   m_playReverseButton;
	CBitmapButton   m_playForwardButton;
	CBitmapButton   m_pausePlayButton;





// Dialog Data
	enum { IDD = IDD_PITCHPLAYER_DIALOG };

protected:
	virtual void  DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual  BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg HCURSOR  OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void On_File_Open_WAV();
	afx_msg void On_Load_Notelist_FileMenu();
	afx_msg void On_About_PitchscopePlayer_help();

	afx_msg void OnGoFilesStartButton();
	afx_msg void OnReversePlayButton();
	afx_msg void OnPausePlayButton();
	afx_msg void OnContinuePlayButton();
	afx_msg void OnGoFilesEndButton();

	CSliderCtrl m_curPlayPositionSliderCtrl;
	afx_msg void OnNMReleasedCapture_curplay_PositionSlider(NMHDR *pNMHDR, LRESULT *pResult);

	CSliderCtrl   m_sensitivitySlider;
	CStatic        m_sensitivityCStatic;
	afx_msg void On_Releasedcapture_Sensitivity_Slider(NMHDR *pNMHDR, LRESULT *pResult);

	CSliderCtrl   m_wavsVolumnSliderCtrl;
	afx_msg void On_Releasedcapture_Wavs_Volume_Slider(NMHDR *pNMHDR, LRESULT *pResult);

	CSliderCtrl   m_stereoBalanceDetection;
	afx_msg void On_Releasedcapture_Detect_Stereo_Balance_Slider(NMHDR *pNMHDR, LRESULT *pResult);

	CSliderCtrl   m_detailSliderCtrl;
	CStatic         m_detailCStatic;
	afx_msg void On_Releasedcapture_Detail_Slider(NMHDR *pNMHDR, LRESULT *pResult);

	CSliderCtrl m_midiVolumnSliderCtrl;
	afx_msg void On_Released_Capture_MidiVolumn_Slider(NMHDR *pNMHDR, LRESULT *pResult);

	CStatic                m_midiInstrumentNameStatic;
	CSpinButtonCtrl   m_midiInstrumentSpinCtrl;
	afx_msg void On_Deltapos_Midi_Instrument_SpinCtrl(NMHDR *pNMHDR, LRESULT *pResult);

	CButton m_muteMidiCheckBox;
	afx_msg void On_BnClicked_Mute_MidiPlay_Checkbox();

	CButton m_boostSourceCheckBox;
	afx_msg void On_BnClicked_Boost_Source_Checkbox();

	CButton m_detectionMidiSourceRadioControl;
	int  m_detectionMidiSourceRadio;
	afx_msg void On_BnClicked_Midi_from_Detection_RadioButton();
	afx_msg void On_BnClicked_Midi_from_NoteList_RadioButton();


	afx_msg void OnHScroll(  UINT nSBCode,   UINT nPos,  CScrollBar* pScrollBar  );
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	CSliderCtrl   m_midiSyncSliderCtrl;
	CStatic         m_midiSyncValueStatic;
	afx_msg void On_NMReleasedCapture_MidiSync_Slider(NMHDR *pNMHDR, LRESULT *pResult);

};
