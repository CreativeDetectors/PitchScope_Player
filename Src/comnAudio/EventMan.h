/////////////////////////////////////////////////////////////////////////////
//
//  EventMan.h   -   Really is   Direct-Sound Manager   or  EVENT  Manager  
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////



#if !defined(AFX_WAVEMAN_H__A2897533_737D_11D3_9507_38E209C10000__INCLUDED_)
#define AFX_WAVEMAN_H__A2897533_737D_11D3_9507_38E209C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//////////////////////////////////////////////////////////////////////



class  AudioPlayer;
class  StreamingAudioplayer;

class  BitSourceStreaming;

class  SequencerMidi;



class  CalcedNote;
class   MidiNote;

class  SPitchCalc;


class  NoteGenerator;


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////



//#define  NUMPLAYEVENTS  61     //	This is now assigned as a memberVar in Waveman( m_numberOfNotificationEvents ) 	1/10		 21         Fails for  71 and above.... WHY ???      [ 61,  368 sampls per EventSlice  lots of resolution  ]


#define  MAXnUMPLAYEVENTS  31    //  71     MAX,  not Actual !!!!    Big for Array limits,  do not want over-write.    3/11  
													 //   For different audio app to pick their own NUMBER of EVENTS,  see  PitchPlayer.cpp,  PsNavigator.cpp,  PScope.h




/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////


class    EventMan	   //   really is    'Direct-Sound  Admin'     ...what is this class'  RELATIONSHIP with AudioPlayer
{							

public:
	EventMan(  long  numberOfPlayEvents,     SequencerMidi   **midiSequencerAddress   );
	virtual ~EventMan();



	BitSourceStreaming*       Get_BitSourceStreaming();

	SPitchCalc*                     Get_SPitchCalc();

	StreamingAudioplayer*    Get_AudioPlayer()          {   return   m_curAudioPlayer;   }   

	SequencerMidi*				  Get_Current_MidiSequencer();



	long		Get_SampleCount_In_PieSection_wSpeedRedux();  



																//  NOTIFICATION   functions


	void      Process_Event_Notification(                unsigned long  iEvent,     SequencerMidi  *midiSequencer    );    //   for OLD  PitchScope  2007



	void		Process_Event_Notification_PPlayer(    unsigned long  iEvent,     bool  outOfSync,   long  expectedEventNumber,    long  pieSliceCounter   );

	void		Process_Event_Notification_PPlayer_NoAudio(   unsigned long  iEvent,    long&  retLastSamplePlayed,    long  pieSliceCounter   );   //   for quiet calc of data without sound Hardware  3/11



	void		Finish_Process_For_PPlayer(   unsigned long   iEvent,     long  curSample,    SequencerMidi  *midiSequencer    );    //   NEW,  2/12





	bool		PreRoll_NoteDetect(              long  advanceCalcSampleIdx,   bool  backwardsPlay,    long&  retLastSamplePlayed,    bool  useBoundaryRounding,   
																																				   CString&  retErrorMesg  ); 

	bool		PreRoll_NoteDetect_DEBUG(   long  advanceCalcSampleIdx,   bool  backwardsPlay,    long&  retLastSamplePlayed,    CString&  retErrorMesg   );




	bool		Post_Roll_NoteDetect(    bool  backwardsPlay,   long&  retLastSamplePlayed,     long  eventCount,    CString&  retErrorMesg   ); 





																//    MIDI    functions  

	 virtual      bool			Process_Midi_Event(   CalcedNote&   calcedNote,    SequencerMidi  *midiSequencer,   CString&  retErrorMesg   );

					bool			Play_Midi_Pitch_WMan(    short  pitchMidi,    short   volume,    short  auditionCode,    CString&  retErrorMesg   );
					bool			Stop_MidiPlay_WMan(   CString&  retErrorMesg    );




																//    MISC   functions  


	 virtual     void			Initialize_MusicalKey_Note_Counters();
	 virtual     void			Add_New_Note_to_MusicalKey_Counter(   short  scalePitch,   long  beginingSampleIdxFile,   long  endingSampleIdxFile   );





																//   DIRECT-SOUND  functions  
			
	
	bool      Init_DSound(   HWND hwnd,   CString&  retErrorMesg   );
	void      Cleanup_DSound();

	void      RegisterAudioPlayer(   StreamingAudioplayer  *buffr    );    //  **** SHOULD copy in new infor to the Notify EVENTS
	void      UNregisterAudioPlayer();                                   

	void      Stop_Output();




//	void      Adjust_OutOfSync_TEST();     //   ****  OMIT ******             might be  OBSOLETE,     3/11





public:
	StreamingAudioplayer    *m_curAudioPlayer; 


	SequencerMidi     **m_midiSequencerAddress;  //  this address points to  SoundHelper::m_midiSequencer,  where it resides ( or SoundMan for PitchScope 2007 )



	unsigned long		  m_numberOfNotificationEvents;   //   21 [ Player,  Navigator ]    ....is different for PitchPlayer than for PitchScope



	short     m_curScalePitch;
	short     m_curScalePitchDetectionScore;
	short     m_curScalePitchAvgHarmonicMag;




	CalcedNote    m_lastStartPlayingNoteMusicalKey; 

	long			m_noteScoresMusicalKey[  12  ];     //   12:  one for each scalePitch        ...our  Population Arrays.
	long			m_noteCountMusicalKey;      //  how MANY notes were polled for the above Population Arrays.




																							//    DEBUGGING  variables  ( eventually *** OMIT ***   )
	long		m_pieSliceCounter;



	long     m_outSyncCountWavMidiPlay;       //   WAS    outOfSyncCountGLB 

	long     m_lastEventNumber;     

	short		m_outOfSyncStatusCode;     //    0 :  OK and in sync       1:  OutOfSync event found,  waiting for HemisphereEvent to clean it up     3/11

//	long	    m_sampleIdxCurPlayingBlockWrite;   // NEW  3/11  Really just for streaming and when we have a sound-Delay. This is a 
																			//  time-stamp to let us know what memory block is being written to the HARDWARE Buffer.






	GUID                  *m_pguid;            //  Which  DSound  DEVICE do we select ( NULL for 'Primary' )

	IDirectSound       *m_infcDSound;    //   was:   m_lpds;       //  pointer to a DirectSound Object created ( COM interface )




									    	//   Notify Events  for  'PLAYING'  ( not recording ) 

	DSBPOSITIONNOTIFY    m_positionNotify[    MAXnUMPLAYEVENTS          ];

	HANDLE                        m_notificationEventHandles[    MAXnUMPLAYEVENTS  +1    ];
};





////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_WAVEMAN_H__A2897533_737D_11D3_9507_38E209C10000__INCLUDED_)
