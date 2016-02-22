/////////////////////////////////////////////////////////////////////////////
//
//  SoundHelper.h   -    Does  High-Level  functions  for  Audio-Player  and   Midi-Player
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



////////////////////////////////////////////////////////////////////////////////////////////////////////////

class   BitSource;      
class   BitSourceStreaming;

class  AudioPlayer;
class  StreamingAudioplayer;

class  SequencerMidi;

class   CalcedNote;
class   SPitchCalc;



////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   SoundHelper    
{                                   //   Does  High-Level functions for {  Audio-Player,  Midi-Player  }.    Also can access  {  EventMan  and  UniApplication  }


// *************   Many of the  Shared SETTINGS  of { Navigator, Player }   should be MOVED in SoundHelper instead of PitchPlayerApp   ************* 
// *************	  Remember,  PitchScope 2.0  and others are coming!!!.   4/2012    ********************************************************


public:
	SoundHelper();
	virtual ~SoundHelper();



	StreamingAudioplayer*	  Get_Streaming_Audioplayer()       {   return  m_audioPlayer;    }  

	SequencerMidi*			      Get_Current_MidiSequencer()       {  return   m_midiSequencer;   }



	BitSourceStreaming*    Get_BitSource_Streaming()          {  return   m_bitSourceStreamingApp;   }

	SPitchCalc*				  Get_SPitchCalc();




	 virtual     bool			App_Has_Navigators_Delay();    //      Basically asks if this is Navigator or possibly future PitchScope 2.0



	 virtual     bool			Is_WAV_Loaded(); 

	 virtual     bool			Is_WAVplayer_Playing();



					long			Get_ActiveViewjs_FirstSample_Index();          // HACKED but works   ...BOTH these functions do the same thing.
					long			Get_AudioPlayers_Last_Played_SampleIdx();


					long			Calc_Biggest_VirtSample_Minus_DrivingVw_Lag();




																							//   Play MP3  functs    (  TRANSPORT )


	 virtual     bool			Continue_Play_Forward(        double  speed,   bool  justSpeedChange,    long  pauseSampleIdx,     CString&  retErrorMesg    );
	 virtual     bool			Continue_Play_Backwards(    double  speed,   bool  justSpeedChange,    long  pauseSampleIdx,     CString&  retErrorMesg    );

	 virtual     bool			Pause_Play(   CString&  retErrorMesg   );
	 virtual     bool			Stop_Play(     CString&  retErrorMesg   );  

	 virtual     bool			Move_to_Files_Start(   CString&   retErrorMesg    );
	 virtual     bool			Move_to_Files_End(     CString&   retErrorMesg    );
	 virtual     bool	        Move_to_Files_Position(   long   startSample,    CString&  retErrorMesg    );




																							//   BITSOURCE   functs


					bool			Do_Post_Roll_CalcNotes(                                  CString&   retErrorMesg  );
					bool			Do_Post_Roll_CalcNotes(   long  eventCount,     CString&   retErrorMesg    );


				    void			Set_BitSource_Streaming(  BitSourceStreaming*  bitsource  )    {  m_bitSourceStreamingApp =  bitsource;   }

				//	long		    Get_Bitsources_Sample_Count_wSlowDown_Expansion();   Want this ???


														//     the   "BIG  WAV-delay"       ( logDFT  and HarmMap  are also reset ) 

					bool			Initialize_Delay_Buffers_and_Two_CircQues(   CString&  retErrorMesg    );     //  Works for both  PLAYER  and NAVIGATOR   11/4/11



														//   ...and   "SMALL  WAV-delay"   just for PLAYER.exe     

					void			Erase_Players_AudioDelay_MemoryBuffer();                     

					bool		    ReAllocate_Players_AudioDelay_MemoryBuffer(   long  numberNeededToMatch   );





																							//   DETECTION  functs

					bool		    Execute_FreqFilter_Change(    short  nuFilterCode   );   

					long			Set_VolumeBoost_Factor(  short   factorCode   );




																							//   MIDI   functs


					bool			Play_Midi_Pitch(   short  pitchMidi,   short  volume,   short  auditionCode,    CString&  retErrorMesg   );    //  NEW,  not in SoundMan   1/10					
					bool			Stop_MidiPlay(   CString&  retErrorMesg    );

					bool				Alloc_Midi_Sequencer(    long  deviceID,    SequencerMidi  **retNewSequencerMidi,    CString&  retErrorMesg  );

					bool				Change_Midi_Instrument(               short   midiInstrumentPatch,    CString&  retErrorMesg   );

					bool				Get_Midi_Instrument_Text_Name(   short   midiInstrumentPatch,    CString&  retInstrumentsName   );

					bool				List_All_Midi_Devices(   short&  retDeviceCount,     CString&  retErrorMesg   );  //   NEW,  1/12

					bool				Assign_and_Open_MidiDevice(    long  deviceID,    CString&  retErrorMesg   );
															




																							//   AUDIOPLAYER   functs


	 virtual     bool		     Allocate_AudioPlayer(   UniApplication&  theApp,   short  *stereoChannelCodeAddr,   long *lastFilePositionAddr,   CString&  retErrorMesg  );

	 virtual     bool		     Alloc_AudioPlayer(   int   typePlayer,   UniApplication&  theApp,    short  *stereoChannelCodeAddr,  long *lastFilePositionAddr,   CString&  retErrorMesg    );
	 virtual     void	  	     Release_AudioPlayer();










public:
	StreamingAudioplayer	   *m_audioPlayer;           //  resides here

	SequencerMidi              *m_midiSequencer;	      //   resides here  ( user can change this from the Settings Dialog



	long                                      m_deviceIDofInternalSynth;       //   gets  assigned by  List_All_Midi_Devices()
	
	ListMemry< IndexedString >   m_midDeviceList;      //   gets  assigned by  List_All_Midi_Devices()

	short		                                m_midiInstrumentPatch;



	short		m_inputScaleingFactor;   


	short     m_curScalePitchAtStop;                             //  the current vale when the user hit the PAUSE BUTTON, or other stop
	short     m_curScalePitchDetectionScoreAtStop;
	short     m_curScalePitchAvgHarmonicMagAtStop;




	bool   m_muteMouseSelectionAudio;        //  Vars  for the   SETTINGS  DIALOG 

	bool   m_muteReversePlayAudio;

	long   m_displayBritnessFactor;    //    5 vals     {  0, 1, 2, 3, 4  }   



	short	  m_userFinalCircqueSizeTweak;        //  also the user to fine tune the DELAY for  MidiNotes/WavPlay   3/20/11

	short	  m_userFCircqSizeTweakMultiplier;   //   { 1, 2, 3 }      So values for  'm_userFinalCircqueSizeTweak'  will be a lot bigger



	/****   HAD to move BACK to EventMan because of   OLD  PitchScope.exe

	short     m_curScalePitch;
	short     m_curScalePitchDetectionScore;
	short     m_curScalePitchAvgHarmonicMag;

	long			m_noteScoresMusicalKey[  12  ];     //   12:  one for each scalePitch        ...our  Population Arrays.
	long			m_noteCountMusicalKey;      //  how MANY notes were polled for the above Population Arrays.
	CalcedNote    m_lastStartPlayingNoteMusicalKey;    back to EventMan
	****/



									//  These 2 vars can tell us what TRANSPORT Button was last pushed, and how FilePosition SEEK is needed.  12/11


	short		m_lastTransportActionActual;     //   The ACTUAL  Transport Button  that was last pushed

	short		m_lastTransportDirection;     //  {   CONTINUEfORWARD   or   CONTINUEbACKWARDS  }  Is assigned after PreRoll, and tell when PreRoll must be done in future
															  //
															  //      More of an interpretaion,  tells the next Play session if a  Data-SEEK and PreRoll   must be done.



	enum  transportActions {   UNKNOWNtrans,     CONTINUEfORWARD,     CONTINUEbACKWARDS,     FILEsTART,    FILEeND,    PAUSEpLAY,
											              SLIDERcHANGE,    PlayLASTpHRASE       };  
			//  Tells us what button was pressed last, and allow us to infer trave direction and File Position     11/11
 

private:
	BitSourceStreaming   *m_bitSourceStreamingApp;     																				
};

