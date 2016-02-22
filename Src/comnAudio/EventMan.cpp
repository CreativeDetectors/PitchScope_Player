/////////////////////////////////////////////////////////////////////////////
//
//  EventMan.cpp   -    Really is   Direct-Sound Manager   or  EVENT  Manager  
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



#include  "..\comnFacade\VoxAppsGlobals.h"



//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"      //  #include <FixMath.h>    ...Mac's version of Fixedpoint 
#include  "..\ComnGrafix\CommonGrafix.h"      //  #include "commonmy.h"   	
//////////////////////////////////////////////////     



#include   "..\ComnGrafix\AnimeStream.h"

#include "..\comnInterface\DetectorAbstracts.h"    // **** NEW ****






//////////////////////////////////////////////////////////////////////
#include  <mmsystem.h>   // Is good on Sparky  ( ProgramFiles\Microsoft Visual Studio\VC\PlatformSDK\Include )


#include  "dsoundJM.h"          //  I copied it in, bigger than the VC++ version
//////////////////////////////////////////////////////////////////////



#include  "..\comnAudio\BitSourceAudio.h"         //  needed for  AudioPlayer.h 




#include   "..\comnMisc\FileUni.h"  
#include   "..\comnAudio\Mp3Decoder.h"       //   Need  these for   WavConvert      7/2012
#include   "..\comnAudio\ReSampler.h"
#include  "..\ComnAudio\FFTslowDown.h"

#include  "..\comnAudio\WavConvert.h"




#include   "PlayBuffer.h"

#include   "sndSample.h"

#include   "SequencerMidi.h"



#include  "..\ComnAudio\CalcNote.h"

#include   "..\ComnAudio\SPitchCalc.h"


#include  "..\ComnAudio\NoteGenerator.h"


#include  "..\ComnFacade\SoundHelper.h"



#include   "EventMan.h"

#include   "AudioPlayer.h"			  //  needs   EventMan.h


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//	SoundHelper&     GetSoundHelper();  **** NO,  EventMan can NOT KNOW of  SoundHelper or SoundMan  ( a conflict if both are here  3/12 )



//  PitchPlayerApp&     Get_PitchPlayerApp();
UniApplication&           Get_UniApp();



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


EventMan::EventMan(   long   numberOfPlayEvents,     SequencerMidi  **midiSequencerAddress   )    
                        :  m_numberOfNotificationEvents( numberOfPlayEvents  ),    m_midiSequencerAddress(  midiSequencerAddress  ) 
{


//	SoundHelper&   GetSoundHelper();   **** NO,  EventMan can NOT KNOW of  SoundHelper or SoundMan  ( a conflict if both are here  3/12 )



	ASSERT(    numberOfPlayEvents > 1     &&     numberOfPlayEvents  <=  MAXnUMPLAYEVENTS   );


	m_pguid =  NULL;      //  think NULL is OK for the Primary buffer

	m_infcDSound =  NULL;        //  OK to do this ???      ...the  'DirectSound OBJECT'  ( COM )


	m_curAudioPlayer =  NULL;



	m_outSyncCountWavMidiPlay =  0;
	m_outOfSyncStatusCode  =   0;     //    0 :  OK and in sync       1:  OutOfSync event found,  waiting for HemisphereEvent to clean it up     3/11


	m_pieSliceCounter =   0;


//	m_sampleIdxCurPlayingBlockWrite  =   -9999999;   // this can have VALID negative values,  but not this small



	m_curScalePitch =   -1;
	m_curScalePitchDetectionScore =   0;
	m_curScalePitchAvgHarmonicMag =   0;


	m_noteCountMusicalKey =  0;
	Initialize_MusicalKey_Note_Counters();
}



											////////////////////////////////////////

EventMan::~EventMan()
{					
	
	Cleanup_DSound();


	for(     long i=0;     i <   (long)(m_numberOfNotificationEvents  +1);       i++     )
	{
		CloseHandle(    m_notificationEventHandles[ i ]    );
	}
}


										////////////////////////////////////////

	
BitSourceStreaming*    EventMan::Get_BitSourceStreaming()
{

	BitSourceStreaming   *retBitsource =  NULL;

	if(   m_curAudioPlayer  ==   NULL   )
	{
		ASSERT( 0 );
		return  NULL;
	}


	if(   m_curAudioPlayer->m_bitSource  ==   NULL   )
	{
		ASSERT( 0 );
		return  NULL;
	}
	else	
	{  retBitsource  =   m_curAudioPlayer->m_bitSource;

		return  retBitsource;
	}
}


											////////////////////////////////////////


SPitchCalc*    EventMan::Get_SPitchCalc()
{

	if(   m_curAudioPlayer  ==   NULL   )
		return  NULL;

	SPitchCalc  *retSPitchCalc  =    m_curAudioPlayer-> Get_SPitchCalc();
	ASSERT(     retSPitchCalc  );

	return   retSPitchCalc;
}

											////////////////////////////////////////


SequencerMidi*    EventMan::Get_Current_MidiSequencer()
{

	if(   m_midiSequencerAddress  ==  NULL  )    // this address point to   
	{
		ASSERT( 0 ); 
		return  false;
	}

	SequencerMidi  *sequencer =     *m_midiSequencerAddress;
	return                sequencer; 
}


											////////////////////////////////////////


void    EventMan::RegisterAudioPlayer(    StreamingAudioplayer   *buffr   )    
{  
								 //  **** SHOULD copy in new infor to the Notify EVENTS   *****   JPM
	m_curAudioPlayer =   buffr;    
} 

											////////////////////////////////////////


void    EventMan::UNregisterAudioPlayer()                                    
{   
	m_curAudioPlayer =   NULL;   
}  


										////////////////////////////////////////

void     EventMan::Cleanup_DSound( void )
{
							       //  Cleans up DirectSound object

	if(   m_infcDSound   )
	{			 
		m_infcDSound->Release();                //  release the  'DirectSound OBJECT'  ( COM )
		m_infcDSound =   NULL;    //  OK to do this ???
	}
}


											////////////////////////////////////////


bool    EventMan::Init_DSound(   HWND  hwnd,   CString&  retErrorMesg    )
{					       	
	
									//   Initialize DirectSound & Sound buffer & Notify interface
    HRESULT    hr;									
    DSCAPS     dscaps;

	retErrorMesg.Empty();

												 //  Create  DirectSound

    if(   FAILED(     hr =    DirectSoundCreate(   m_pguid,    &m_infcDSound,   NULL  )     )   )
	{

		retErrorMesg =  "Init_DSound FAILED, could not CREATE DirectSound.  Application must shut down." ;
		return   false;
	}


												 //  Set cooperative level.


// *******************************************************************************************************

	// *****   BIG,  on 3/4/12   Changed to   DSSCL_EXCLUSIVE  from   DSSCL_PRIORITY.  Semms like better performance on Sparky where I 
	//                                       had trouble with Status-messages of  Lost Events.   Watch for Complications.  

// ******************************************************************************************************


	DWORD   flagsCoopLevel  =    DSSCL_EXCLUSIVE;    //  myPrevDefault:  DSSCL_PRIORITY       ( or   DSSCL_NORMAL  ???  )   


	if(    FAILED(    hr =   m_infcDSound->SetCooperativeLevel(   hwnd,   flagsCoopLevel  )   )  )   
	{
		ASSERT( 0 );
		retErrorMesg.Format(   "Init_DSound FAILED,  could not Set Cooperative Level(  %d  ).  Application must shut down.",    flagsCoopLevel   );
		return   false;
	}





												  // Get   device capabilities,  we don't actually do anything with these.

    dscaps.dwSize  =   sizeof(  DSCAPS  );

    hr =   m_infcDSound->GetCaps(  &dscaps  );


// ************   Do I need to examine these ????   12/06  ************************************





													//  Allocate the raw NOTIFICATION events. We make an extra one for later use  by the output buffer.   3/12

// for(    int i =0;      i <      (int)m_numberOfNotificationEvents;       i++   )  **** this FAILS,  think need extra event for  

	for(    int i =0;      i <=   (int)m_numberOfNotificationEvents;       i++   )    //  m_numberOfNotificationEvents   is  21,   20 Play-Events,  and one  StopEvent[ 20 ]
    {

        m_notificationEventHandles[ i ]       =         CreateEvent(   NULL,   FALSE,   FALSE,   NULL  );
        if(  m_notificationEventHandles[ i ] ==   NULL  )
		{   
			ASSERT( 0 ); 	
			retErrorMesg =  "Init_DSound FAILED, could not CreateEvent.  Application must shut down." ;
			return  false;   
		}


//		if(     i <    (int)m_numberOfNotificationEvents    )   **** FAILS
		if(     i <=  (int)m_numberOfNotificationEvents    )   //  ***** ALSO need to connect the  Stop Event ?????    3/4/12  ************************************
		{

			m_positionNotify[  i  ].hEventNotify  =    m_notificationEventHandles[ i ];  
		}


		//  These guys will get more   Assignments/INITIALIZATIONS  in    StreamingAudioplayer::Initialize_Compatable_Buffer()    3/12
    }


    return   TRUE;
} 



										////////////////////////////////////////


long   EventMan::Get_SampleCount_In_PieSection_wSpeedRedux()
{


					//  Reduce that count as speedSlow down happen

	ASSERT(  m_curAudioPlayer  );

	long    retSampleCount =  -9;


	long    totalSamplesInPieSection  =     (long)(  TWOSECBYTES  /  (  m_numberOfNotificationEvents  -1 )   )   / 4;     //  [ 368 ]  How many samples in an Event  


	retSampleCount  =    (long)(    (double)totalSamplesInPieSection  /  m_curAudioPlayer->m_playSpeedFlt     );

	return   retSampleCount;
}



										////////////////////////////////////////


void   EventMan::Process_Event_Notification_PPlayer(   unsigned long   iEvent,   bool  outOfSync,   long  expectedEventNumber,    long  pieSliceCounter    )
{

						 	//   Is  CALLED  by    PsNavigatorDlg::Player_Messaging(),    and   PitchPlayerDlg::Player_Messaging()



	//	     1 PieSlice =  1/10 second    or    1 PieSlice =   100 milli-seconds           [    44160  =  BLOCKLOCKEDSIZE  =   m_byteCountTwoSeconds / 2    ]



//	SoundHelper&   GetSoundHelper();   **** NO,  EventMan can NOT KNOW of  SoundHelper or SoundMan  ( a conflict if both are here  3/12 )
	BitSourceStreaming  *bitSource     =     Get_BitSourceStreaming();

	SequencerMidi  *midiSequencer  =		  Get_Current_MidiSequencer();


	CString   retErrorMesg;
	bool        justDidaBlockLoad =   false;
	short   preRollCode =  0;    // always for this function


	short   appCode  =    Get_PitchScope_App_Code_GLB();     //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope

	bool    useNavigatorDelay;
	if(   appCode  ==  0   )
		useNavigatorDelay =   false;
	else
		useNavigatorDelay =   true;



	if(         m_curAudioPlayer  ==  NULL    
		||   ! m_curAudioPlayer->IsOK()     
		||     bitSource ==  NULL   )
	{   
		ASSERT( false );	
		AfxMessageBox(  "EventMan::Process_Event_Notification_PPlayer  FAILED,   m_curAudioPlayer  or  m_curAudioPlayer->m_bitSource  is NULL."   );
		return; 	
	}

	ASSERT(  midiSequencer   );


	SPitchCalc   *sPitchCalcer  =    m_curAudioPlayer->Get_SPitchCalc();
	ASSERT(      sPitchCalcer   );              //  ALWAYS for this function ????    11/11    

	ASSERT(   sPitchCalcer->m_spitchCalcsSndSample  !=  NULL   );      //  ALWAYS for this function ????    11/11    



	NoteGenerator    *noteGenerator   =      bitSource->m_noteGenerator;

	if(          bitSource->m_recordNotesNow       //  Must have a  noteGenerator  if we are to record notes at this stage of the app    3/11
		  &&   noteGenerator  ==  NULL    )
	{
		AfxMessageBox(   "Process_Event_Notification_PPlayer  FAILED,  NoteGenerator is NULL."   );
		bitSource->m_recordNotesNow =   false;
	//	return;     We could keep going,  but there may be a mess.    3/11
	}


	double     playSpeedFlt   =      m_curAudioPlayer->m_playSpeedFlt;
	ASSERT(  playSpeedFlt  >= 1.0  );



	unsigned long   hemisEvent  =    ( (unsigned long)( m_numberOfNotificationEvents -1 ) )  /2;   // The event at 180 degrees,    for Player is 30
	long    halfEventsCount    =   (    (long)m_numberOfNotificationEvents  -1L   )   /2L;    //  10
	long   samplesInDataFetch       =      BLOCKLOCKEDSIZE  /  bitSource->m_bytesPerSample;   //    11,040    *** Need to adjust for speed ????? 

	long   samplesInDataFetchSpeedRedux  =   (long)(    (double)samplesInDataFetch   /   playSpeedFlt   );

	long   sampsInPieSliceWithSpeedRedux   =     Get_SampleCount_In_PieSection_wSpeedRedux();


	/***		move to a new function																      //    Calc the  Delay in samples of the   PRIMARY CircQue's  notes

	long    primaryCircQuesDelayNotes   =    sPitchCalcer->m_sizeOfPrimNotesCircque /2;
	long    primaryCircQuesDelaySamps  =   (long)(     ( (double)primaryCircQuesDelayNotes  *  (double)sampsInPieEvent  )  / playSpeedFlt    ); 

	long    dftMapsDelayNotes    =    sPitchCalcer->Calc_LogDFTs_Delay_In_CalcedNotes();    // this also contributes to the Delay that we hear from the Primary Circ Que
	long    dftMapsDelaySamps  =    (long)(    ( (double)dftMapsDelayNotes  *  (double)sampsInPieEvent  )  / playSpeedFlt    ); 

	long    totalPrimaryCalcedNotesDelay  =    primaryCircQuesDelaySamps    +     dftMapsDelaySamps;
	***/

	long  curSample =  -8;	  //   NEED  -8   for the initialization.    'curSample'   is always VIRTUAL,  that is it does NOT have the slowedDown EXPANSION in its time-coords.

	m_curScalePitch =  -1;   

	


	if(     iEvent    <    (unsigned long)( m_numberOfNotificationEvents -1 )     )          
	{



		if(         m_curAudioPlayer->m_doneFetchingPlayBytes ==  true     //  Happens when  Load_Next_DataFetch_Forward_PPlayer()  hits EOF  1/2012
			||    ! m_curAudioPlayer->m_isPlaying   )
		{																	//  (  Seems like I only land here for MP3 files.  I have trouble calcing the last byte in their files.  )

			Finish_Process_For_PPlayer(   iEvent,   curSample,   midiSequencer  );   //  'curSample'  might be -8,  but that is OK for  Finish_Process_For_PPlayer()  3/12
			return;   //  *** ESCAPE  ***
		}


		
														//  If the play cursor has just reached the first or second half of the buffer, it's time to stream data to the other half.
		if(         iEvent ==   0								
			 ||    iEvent  ==   hemisEvent    )   
		{

			if(     m_curAudioPlayer->m_playingBackward    )
			{
				m_curAudioPlayer->Load_Next_DataFetch_Backward_PPlayer(   iEvent  );

				justDidaBlockLoad =   true;
		    }
			else
			{   m_curAudioPlayer->Load_Next_DataFetch_Forward_PPlayer(   iEvent    );  
	
				justDidaBlockLoad =   true;
				
				/***    	//    Calculate the sampleIndex of the  'WAVdata writeBlock'   that we are currently HEARING  ( it has been delayed by the Buffer in AudioPlayer )

				long   latestSampleIdx  =    m_curAudioPlayer->Calc_SampleIdx_Forward_PPlayer(   iEvent,    m_curAudioPlayer->m_dataFetchCount  ); 
																                        //   'latestSampleIdx'   should be the same value as   'curSample'  just below   3/11
				long   wavBuffersDelay =    bitSource->Calc_WAV_DelayBuffers_Ideal_Delay_In_Samples(   playSpeedFlt,   m_curAudioPlayer->m_bytesPerSample  );
				m_sampleIdxCurPlayingBlockWrite  =     latestSampleIdx   -    wavBuffersDelay;   //  Is this really accurate??   3/2012
				***/
			}
		}



				
		if(    m_curAudioPlayer->m_playingBackward    )
		{
			curSample  =   ( bitSource->m_sampleIdxLastBlockLoadNotSlowExpanded  -  samplesInDataFetchSpeedRedux  )   
				                                                              -      (   bitSource->m_currentPieClockVal   *  sampsInPieSliceWithSpeedRedux  );
		}
		else
		   curSample  =   ( bitSource->m_sampleIdxLastBlockLoadNotSlowExpanded  -  samplesInDataFetchSpeedRedux  )    
				                                                              +      (   bitSource->m_currentPieClockVal   *  sampsInPieSliceWithSpeedRedux  );
		

		


		if(    m_curAudioPlayer->m_doneFetchingPlayBytes ==   true   )       //  Happens when   Load_Next_DataFetch_Forward_PPlayer()  hits EOF   1/2012
		{			
			Finish_Process_For_PPlayer(    iEvent,    curSample,    midiSequencer   );
			return;                //  *** can ESCAPE early,   but must first calulate  'curSample' up above.    2/2/2012   ***
		}


		

		if(          m_curAudioPlayer->m_isPlaying        //  isPlaying :    Need this because, even after user has hit stop play button,  this function continues to be called. Causes MIDI not not to turn off  2/10
			&&   ! m_curAudioPlayer->m_doneFetchingPlayBytes   )
		{				


			long   eventNum =   iEvent;    //  can be { 0 to 19 },  but reload with Load_Next_DataFetch_Forward() on  iEvent = 0 or 10
			if(    iEvent  >=   hemisEvent   )
				eventNum =    iEvent  -  hemisEvent;   
			else
				eventNum =   iEvent;    //  eventNum is { 0 thru 9 }   		
 




													//   'pieSliceIdx'  is really just for DEBUG.   Its real use is in  Make_NoteList_No_Audio()  3/12
			long   pieSliceIdx =  -13,   pieSliceIdxALT =  -11;
			long   pieSliceCountAtStart  =      m_curAudioPlayer->m_startSample   /   sampsInPieSliceWithSpeedRedux;    // ********* WRONG ????   12/8 ********

			pieSliceIdxALT  =    curSample  /  sampsInPieSliceWithSpeedRedux;    //  Probably the most accurate


										//   This CALC of  'pieSliceIdx'  looks CORRECT!!!  ( especially when testing with 'pieSliceIdxALT'  3/12 ).   'pieSliceIdx'  real use is in  Make_NoteList_No_Audio()  3/12  ****

			if(    m_curAudioPlayer->m_playingBackward    )
				pieSliceIdx =   pieSliceCountAtStart    -   (  eventNum   +   ( halfEventsCount   *  ( m_curAudioPlayer->m_dataFetchCount  )    )  );   //  OK
			else
				pieSliceIdx =    eventNum   +   (   halfEventsCount   *  ( m_curAudioPlayer->m_dataFetchCount   -1)    )     +   pieSliceCountAtStart;  							
																							                                                              //  -1:  m_dataFetchCount   was updated before this   
			
			if(    pieSliceIdx   !=    pieSliceIdxALT    )
			{
				int  dummy =  9;  
			}




			CalcedNote  retCurrentNote;   
			long    eventsOffset       =     (long)(   eventNum   *  sampsInPieSliceWithSpeedRedux  );    //  eventNum is { 0 thru 9 }   ...10 pieSlices.		
			long    eventsOffsetAdj  =      (long)(    (double)eventsOffset   /  playSpeedFlt    );   

				

			if(    sPitchCalcer->m_playModeUsingNotelist  ==   0   )     //  0:   Doing   DETECTION 
			{ 					

				if(    ! sPitchCalcer->Detect_Note_for_Pie_Slice(   curSample,    eventsOffsetAdj,    retCurrentNote,    m_curAudioPlayer->m_playingBackward,  
						                                                                      *(sPitchCalcer->m_spitchCalcsSndSample),    useNavigatorDelay,   pieSliceIdx,   preRollCode,   
																							            pieSliceCounter,    retErrorMesg   )    )
				{  AfxMessageBox(  retErrorMesg  );   
					return;
				}
					

				if(              appCode  ==  1       //    1:   NAVIGATOR,    cause it has a  FinalCircque  for delay
						&&   ! m_curAudioPlayer->m_playingBackward  )                      
					sPitchCalcer->Get_Oldest_Note_from_Final_CircularQue(   retCurrentNote   );		


				m_curScalePitch                         =      retCurrentNote.scalePitch;       //  MUST return results to Wavman  member vars so it can RENDER to the display.
				m_curScalePitchDetectionScore   =      retCurrentNote.detectScoreHarms;
				m_curScalePitchAvgHarmonicMag  =    retCurrentNote.detectAvgHarmonicMag;     //   NEW   2/12

					
			    if(    bitSource->m_recordNotesNow   )
				{							
					if(     noteGenerator->m_sampleIdxRecordingStart  <  0    )
						noteGenerator->m_sampleIdxRecordingStart =     retCurrentNote.beginingSampleIdxFile;     //   curSample is too far in future. 

					noteGenerator->m_sampleIdxRecordingEnd       =     retCurrentNote.beginingSampleIdxFile;  //  Keep updating so we know value when play stops.


				//	short   retSynthsCurPitch =   midiSequencer->m_curPitch;   //  **** WEIRD,  usually we need this value to PERSIST ( it does NOT here
																			//  because it is Local ).  BUT it is correctly re-assigned on each cycle, so I guess that is enough.  3/25/11

					if(    ! noteGenerator->Record_Midi_Note(   retCurrentNote,    retErrorMesg  )    )
					{   
						bitSource->m_recordNotesNow =   false;
						AfxMessageBox(  retErrorMesg  );
					}
				}					
			}   //   if(   doRealtimePitchDetect   )

			else
			{	ASSERT(   sPitchCalcer->m_playModeUsingNotelist  ==  1  );     //    1: Playing from  NOTELIST

//  with  AUDIO

				if(    ! sPitchCalcer->Search_NoteList_for_AudioMusical_Values(   curSample,    retCurrentNote,    m_curAudioPlayer->m_playingBackward,  useNavigatorDelay,  													                                                               
																		                                          pieSliceIdx,    expectedEventNumber,   iEvent,	  pieSliceCounter,    preRollCode,   retErrorMesg  )     )
				{  AfxMessageBox(  retErrorMesg  );  }
				

				
				if(              appCode  ==  1       //    1:   NAVIGATOR,    cause it has a  FinalCircQue  for Delay
						&&   ! m_curAudioPlayer->m_playingBackward  )                      
					sPitchCalcer->Get_Oldest_Note_from_Final_CircularQue(   retCurrentNote   );		


				m_curScalePitch                         =      retCurrentNote.scalePitch;       //  MUST return results to Wavman  member vars so it can RENDER to the display.
				m_curScalePitchDetectionScore   =      retCurrentNote.detectScoreHarms;
				m_curScalePitchAvgHarmonicMag =     retCurrentNote.detectAvgHarmonicMag;
			}



			if(   ! Process_Midi_Event(   retCurrentNote,   midiSequencer,   retErrorMesg )   )  // Looks at  .synthCode,  turns on/off by the current 'retCurrentNote' on the Midi Synth  1/12
				AfxMessageBox(  retErrorMesg  );
	

			bitSource->Increment_Pie_Clock();     //  ****  NEW,  very important for the CALC of  'curSample'   2/2012  ****

		}  //    if(   m_curAudioPlayer->m_isPlaying     &&    ! m_curAudioPlayer->m_doneFetchingPlayBytes 

			




		if(    m_curAudioPlayer->Is_Playing()     )
		{
												//   Do NOT want to change  UniBasic::m_lastFilePosition [   Set_LastPlayed_SampleIdx()  ].  
												//   If in  'LoopPlay'  want to PRESERVE  this value in case RePlay_Previous_Phrase() is called again     2/03

			if(          m_curAudioPlayer->m_playMode   !=   AudioPlayer::LASTnOTEpLAYfORWARD  
			      &&   m_curAudioPlayer->m_playMode   !=   AudioPlayer::LASTnOTEpLAYbACKWARD    ) 	
			{	
				m_curAudioPlayer->Set_LastPlayed_SampleIdx(  curSample  );   
			} 
	
																	                                                   //  Only for  Play_Closest_Note_Forward/Backward()  mode

			if(        m_curAudioPlayer->m_playMode   !=    AudioPlayer::RETURNtoPLAYSTART   
				&&   m_curAudioPlayer->m_playMode   !=    AudioPlayer::LOOPpLAYsELCT  
				
				&&   m_curAudioPlayer->m_playMode   !=    AudioPlayer::PLAYwINDOW   )   // ******* ???   WANT this ???   5/12/2012 *******************  
			{								

				m_curAudioPlayer->m_lastNotePlaySample =    curSample;     //  Want the first StepNotePlay to happen at the window's left edge          
			}



		    m_curAudioPlayer->m_theApp.Animate_All_Viewjs(   curSample,   m_curScalePitch  );    //   Only moves the  FILE-SLIDER  in Player and Navigator.
		}

	}  //    if(     iEvent  <  ( m_numberOfNotificationEvents -1 ) 



	Finish_Process_For_PPlayer(   iEvent,   curSample,   midiSequencer  );
}





										////////////////////////////////////////


void   EventMan::Finish_Process_For_PPlayer(    unsigned long   iEvent,     long  curSample,    SequencerMidi  *midiSequencer   )
{

		//   basically CHECKS to see if    PLAY should STOP (or Loop, etc),   and what to do if it receives the STOP-iEVENT    NEW   2/2012, 


//	SoundHelper&   GetSoundHelper();   **** NO,  EventMan can NOT KNOW of  SoundHelper or SoundMan  ( a conflict if both are here  3/12 )
	BitSourceStreaming   *bitSource    =     Get_BitSourceStreaming();


	if(         m_curAudioPlayer  ==  NULL    
		||   ! m_curAudioPlayer->IsOK()     
		||     bitSource ==  NULL   )
	{   
		ASSERT( false );	
		AfxMessageBox(  "EventMan::Finish_Process_For_PPlayer  FAILED,   m_curAudioPlayer  or  m_curAudioPlayer->m_bitSource  is NULL."   );
		return; 	
	}

	ASSERT(  midiSequencer   );


	SPitchCalc   *sPitchCalcer  =    m_curAudioPlayer->Get_SPitchCalc();
	ASSERT(      sPitchCalcer   );              //  ALWAYS for this function ????    11/11    

	ASSERT(   sPitchCalcer->m_spitchCalcsSndSample  !=  NULL   );      //  ALWAYS for this function ????    11/11    


	CString   retErrorMesg;

	NoteGenerator    *noteGenerator   =      bitSource->m_noteGenerator;

	if(          bitSource->m_recordNotesNow       //  Must have a  noteGenerator  if we are to record notes at this stage of the app    3/11
		  &&   noteGenerator  ==  NULL    )
	{
		AfxMessageBox(   "Finish_Process_For_PPlayer  FAILED,  NoteGenerator is NULL."   );
		bitSource->m_recordNotesNow =   false;
	//	return;     We could keep going,  but there may be a mess.    3/11
	}




	if(     iEvent  <    (unsigned long)( m_numberOfNotificationEvents -1 )     )      //   iEvent { 0 - 19 }   20: StopEvent      Still Processing  REGULAR  EVENTS...    
	{


		ASSERT(   iEvent   <=  19   );


				                                                                       //    STOP Play if hit  last INTENDED  sample 
		if(     m_curAudioPlayer->m_doneFetchingPlayBytes	  

			       ||   (        (   ! m_curAudioPlayer->m_playingBackward    &&     curSample  >=  m_curAudioPlayer->m_endSample   )
			                 ||  (     m_curAudioPlayer->m_playingBackward    &&     curSample  <=  m_curAudioPlayer->m_endSample   )    )       )										  
		{

			m_curAudioPlayer->StopPlaying_Hardware();                              //  get here from  StepNote_Play   or  Section  play   or   EOF-play


			if(    ! Stop_MidiPlay_WMan(    retErrorMesg  )    )
				AfxMessageBox(  retErrorMesg  );  
		
																																//  BUG for PlayLast ????    2/12
			if(     m_curAudioPlayer->m_playMode  ==    AudioPlayer::NORMALpLAY   )
				m_curAudioPlayer->m_prevEndSamplePlay  =    m_curAudioPlayer->m_endSample;	 




																				     //  For  'LOOP Play'  we just restart the thing

			if(     m_curAudioPlayer->m_playMode  ==   AudioPlayer::LOOPpLAYsELCT   )
			{				


														                        //   Need to do a  PRE-ROLL  because we hop back to a new spot.  12/11
				sPitchCalcer->Initialize_for_Play();   
			

				sPitchCalcer->Erase_CircularQues_and_DrivingBitmap();   //  these 2 functions recaptilate  Empty_SoundBuffer_Init_FinalCircQue_Draw_DriveVw() for most part  12/11 
				m_curAudioPlayer->Fill_Buffer_With_Silence();     


				long    retLastSamplePlayed;
				bool    preventFileSeek =   true;     //  want TRUE,   because when we do a PreRoll, and do NOT need to Seek.   12/11
			

	


				Begin_Wait_Cursor_GLB();	

				if(    ! PreRoll_NoteDetect(    m_curAudioPlayer->m_prevStartSamplePlay,    false,    retLastSamplePlayed,    true,    retErrorMesg   )    )
				    AfxMessageBox(  retErrorMesg  );  
				
				End_Wait_Cursor_GLB();	

				m_curAudioPlayer->m_theApp.ReDraw_Bitmaps(  true  );   //  true: do NOT render notes TEXT   (  NEW way to acces an UGLY GLOBAL function.   2/12



				if(   ! m_curAudioPlayer->StartPlaying(   AudioPlayer::LOOPpLAYsELCT,   m_curAudioPlayer->m_playSpeedFlt,   false,   m_curAudioPlayer->m_prevStartSamplePlay,   
																		           m_curAudioPlayer->m_prevEndSamplePlay,   false,   preventFileSeek,   retErrorMesg  )    )
					AfxMessageBox(  retErrorMesg  ); 
			}

			else if(     m_curAudioPlayer->m_playMode  ==   AudioPlayer::RETURNtoPLAYSTART   )    //  coming to the END of  "Play Last PHRASE"  command   4/12
			{

		//		m_curAudioPlayer->m_theApp.ReDraw_Bitmaps(  false  );    **** With this,  we would see the END of the PHRASE


											//  NOW,    This should bring us back to the   BEGINNING of the PHRASE

				long   sampleIdxTarg  =     m_curAudioPlayer->m_prevStartSamplePlay;   // ********   How to make this more accurate ???  5/12/  *******************

				sPitchCalcer->Initialize_for_Play();   
			
				sPitchCalcer->Erase_CircularQues_and_DrivingBitmap();   //  these 2 functions recaptilate  Empty_SoundBuffer_Init_FinalCircQue_Draw_DriveVw() for most part  12/11 
				m_curAudioPlayer->Fill_Buffer_With_Silence();     


				bool  useBoundaryRounding =  false;  

				long    retLastSamplePlayed;
				bool    preventFileSeek =   true;     //  want TRUE,   because when we do a PreRoll, and do NOT need to Seek.   12/11
			



				Begin_Wait_Cursor_GLB();	

				if(     ! PreRoll_NoteDetect(    sampleIdxTarg,    false,    retLastSamplePlayed,    useBoundaryRounding,    retErrorMesg   )     )
				    AfxMessageBox(  retErrorMesg  );  
				
				End_Wait_Cursor_GLB();	

				m_curAudioPlayer->m_theApp.ReDraw_Bitmaps(  true  );   //  true: do NOT render notes TEXT   (  NEW way to acces an UGLY GLOBAL function.   2/12
			}


			else if (    m_curAudioPlayer->m_playMode  ==   AudioPlayer::PLAYwINDOW  )
			{

				long   sampleIdxTarg;


				if(    m_curAudioPlayer->m_lastWindowPlayStartSample  >=  0  )  // *******  CAREFUL to have enough INITIALIZATIONS (to -1) for this new var.  6/2012 
				{																							    //                 SEE  On_BnClicked_Nudge_DView_Forward_Button()
					sampleIdxTarg  =    m_curAudioPlayer->m_lastWindowPlayStartSample;
				}
				else
				{  ASSERT( 0 );											 // ***** CAN'T happen   5/12/2012    ??????
					sampleIdxTarg  =    m_curAudioPlayer->m_prevStartSamplePlay; 
				}
				


				sPitchCalcer->Initialize_for_Play();   
			
				sPitchCalcer->Erase_CircularQues_and_DrivingBitmap();   //  these 2 functions recaptilate  Empty_SoundBuffer_Init_FinalCircQue_Draw_DriveVw() for most part  12/11 
				m_curAudioPlayer->Fill_Buffer_With_Silence();     


				bool  useBoundaryRounding =  false;  

				long    retLastSamplePlayed;
				bool    preventFileSeek =   true;     //  want TRUE,   because when we do a PreRoll, and do NOT need to Seek.   12/11
			


				Begin_Wait_Cursor_GLB();	

				if(     ! PreRoll_NoteDetect(    sampleIdxTarg,    false,    retLastSamplePlayed,    useBoundaryRounding,   retErrorMesg   )     )
				    AfxMessageBox(  retErrorMesg  );  
				
				End_Wait_Cursor_GLB();	


	//			TRACE(  "\nEventMan::Finish_Process_For_PPlayer   after PreRoll    [  targSampIdx %d  ]    [   retLastSamplePlayed    %d   ]   \n\n",   sampleIdxTarg,   retLastSamplePlayed    );


				m_curAudioPlayer->m_theApp.ReDraw_Bitmaps(  true  );   //  true: do NOT render notes TEXT   (  NEW way to acces an UGLY GLOBAL function.   2/12


	//			m_curAudioPlayer->m_theApp.Animate_All_Viewjs(  long  curSample,   short curScalePitch  );  **** WANT for the FileSlider ??? ******************
			}

			else
			{  if(    bitSource->m_recordNotesNow    )
				{
					ASSERT(  noteGenerator   );
					long    lastProcessedSampleIdx;  
					bool    retUserDidMerge =  false;


					if(     noteGenerator->m_sampleIdxLastPause  >=  0    ) 
						lastProcessedSampleIdx =   noteGenerator->m_sampleIdxLastPause;   //  there was a PauseButton hit.    usually is same as  m_prevEndSamplePlay
					else
						lastProcessedSampleIdx =   m_curAudioPlayer->m_prevEndSamplePlay;  //  played to END.  this will be  cNoteExternal.m_totalSampleBytes /4


					noteGenerator->m_musicalKey =     sPitchCalcer->m_musicalKey;     //  user might have changed the MusicalKey setting during the Recording Session


					if(    ! noteGenerator->Ask_User_Merge_or_Save_New_Notes(   lastProcessedSampleIdx,  retUserDidMerge,   retErrorMesg  )    )																		
						AfxMessageBox(  retErrorMesg  ); 
					
					if(    retUserDidMerge   )
						m_curAudioPlayer->m_theApp.Set_DSTlist_Modified(  true  );   
				}
			}

		}    //   if(     m_curAudioPlayer->m_doneFetchingPlayBytes	 ...STOPPING  PLAY

	}   //  if(     iEvent  <    (unsigned long)( m_numberOfNotificationEvents -1 )    ...STILL Playing


	else    
	{	         //  ***  the  STOP EVENT ***       iEvent  ==   ( m_numberOfNotificationEvents -1 )      ****  PLAY  has  STOPPED ***   
				
	
		if(       m_curAudioPlayer->m_playMode  ==   AudioPlayer::LASTnOTEpLAYfORWARD  
			 ||   m_curAudioPlayer->m_playMode  ==   AudioPlayer::LASTnOTEpLAYbACKWARD    )  
		{
			m_curAudioPlayer->Set_Play_Mode(   AudioPlayer::NORMALpLAY    );      //   reset to default    ??? Is this NECESSARY ???  
		}
		


		if(    ! m_curAudioPlayer->m_isPlaying    )			//  Play has STOPPED by reaching the  'END-of-FILE' ,  or  'PAUSE Button'
		{		


//   ***********************   IS this accurate ???  **********************************************			 
			 long   lastPlayedSampleIdx  =     m_curAudioPlayer->Get_LastPlayed_SampleIdx(); 
// ****************************************************************************************



			 long   biggestVirtSample      =     m_curAudioPlayer->Get_Biggest_SampleIndex_No_SpeedExpansion();     //  does NOT have the  expansion for SlowedDown 
			 


			 if(     lastPlayedSampleIdx   >=    biggestVirtSample    )
			 {

				long   sampleCnt  =       m_curAudioPlayer->m_sampleCountInViewjsWidth;   //  -118     ..WHY ???  4/11
				if(      sampleCnt  >  0   )
				{
					lastPlayedSampleIdx  -=    ( sampleCnt  *    3L)  /  4L;   //  Nudge image to left so we can see some of Files last samples

					if(    lastPlayedSampleIdx  < 0    )
						lastPlayedSampleIdx =  0;


					m_curAudioPlayer->Set_LastPlayed_SampleIdx(       lastPlayedSampleIdx      );	

					m_curAudioPlayer->m_theApp.Animate_All_Viewjs(   lastPlayedSampleIdx,  -1  );   
				}
				else
				{  
					if(   sampleCnt  ==  -118  )   //   means SoundMan::Sync_AudioPlayers_Vars() was never called,  and this is PitchPlayer.exe that is running.
					{                                         //   ( Install a better test for these applications ???  )

					//	lastPlayedSampleIdx =   0;          //  for   PitchPlayer.exe   we go to the start of the sample if we play to the end.    1/10 

					//  *** Now try to let it play to the end.    11/4/11


					int   dummy =  9;     //   Does this get HIT??    YES, at the end of the Plunge 2/12        

                     // ****************  BIG PROBLEM if I go to this from PitchScope.exe or VoxSep.exe  ...   1/10   ********************************


					}
					else
					{  long   sampleCnt  =    22100;    //  Is a half second reasonable ???


						lastPlayedSampleIdx  -=    ( sampleCnt  *    3L)  /  4L;   //  Nudge image to left so we can see some of Files last samples

						if(    lastPlayedSampleIdx  < 0    )
							lastPlayedSampleIdx =  0;
					}


					m_curAudioPlayer->Set_LastPlayed_SampleIdx(    lastPlayedSampleIdx   );	

					m_curAudioPlayer->m_theApp.Animate_All_Viewjs(  lastPlayedSampleIdx,  -1  );
				}
			 }


				             //  Now that PLAY has STOPPED,  do the FINAL render to the DISPLAY  ( should these 3 function have their own master function? )  4/12

			m_curAudioPlayer->m_theApp.Animate_All_Viewjs(   lastPlayedSampleIdx,   m_curScalePitch  );    //   Only moves the File-Slider in Player.      

			m_curAudioPlayer->m_theApp.ReDraw_All_Anime_Viewjs();   //    Bullets for Revolver and Gagues
	


// ********************  PUT BACK   ALLOW_logDFT_DRIVING_VIEW  *********************
			m_curAudioPlayer->m_theApp.ReDraw_Bitmaps();    //   Need this for Backwards Play to FileStart.   



		}	 //   if(    ! m_curAudioPlayer->m_isPlaying  			

	}   //   else  ( the  STOP EVENT  )
}




										////////////////////////////////////////


void   EventMan::Process_Event_Notification_PPlayer_NoAudio(   unsigned long  iEvent,    long&  retLastSamplePlayed,   long  pieSliceCounter   )
{


				//   Is  used for  PRE-ROLL and POST-ROLL,   does calcs for the Delay-Pipeline without playing the sound through the Hardware.  3/2011   


//	SoundHelper&   GetSoundHelper();   **** NO,  EventMan can NOT KNOW of  SoundHelper or SoundMan  ( a conflict if both are here  3/12 )
	BitSourceStreaming   *bitSource      =     Get_BitSourceStreaming();
	ASSERT(  bitSource  );       //  ALWAYS for this function ????    11/11


	short         appCode     =     Get_PitchScope_App_Code_GLB();     //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope
	ASSERT(   appCode  !=  0  );     //  Only for Navigator,   not Player


	bool    useNavigatorDelay;

	if(   appCode  ==  0   )
		useNavigatorDelay =   false;
	else
		useNavigatorDelay =   true;


	
	if(        m_curAudioPlayer  ==  NULL    
		||     bitSource  ==  NULL    
		||   ! m_curAudioPlayer->IsOK()     )
	{   
		ASSERT( false );	
		AfxMessageBox(  "EventMan::Process_Event_Notification_PPlayer_NoAudio  FAILED,   m_curAudioPlayer  or  m_curAudioPlayer->m_bitSource  is NULL."   );
		return; 	
	}


	SPitchCalc   *sPitchCalcer  =    m_curAudioPlayer->Get_SPitchCalc();
	ASSERT(      sPitchCalcer   );              //  ALWAYS for this function ????    11/11    

	ASSERT(   sPitchCalcer->m_spitchCalcsSndSample  !=  NULL   );      //  ALWAYS for this function ????    11/11    



	NoteGenerator    *noteGenerator   =      bitSource->m_noteGenerator;

	if(          bitSource->m_recordNotesNow       
		  &&   noteGenerator  ==  NULL    )
	{
		AfxMessageBox(   "Process_Event_Notification_PPlayer_NoAudio  FAILED,  NoteGenerator is NULL."   );
		bitSource->m_recordNotesNow =   false;
	//	return;     We could keep going,  but there may be a mess.    3/11
	}


	double      playSpeedFlt   =      m_curAudioPlayer->m_playSpeedFlt;
	ASSERT(   playSpeedFlt  >= 1  );



	CString           retErrorMesg;
	bool                justDidaBlockLoad =   false;

	unsigned long   hemisEvent  =    ( (unsigned long)( m_numberOfNotificationEvents -1 ) )  /2;   // The event at 180 degrees,    for Player is 30

	long     halfEventsCount       =   (    (long)m_numberOfNotificationEvents  -1L   )   /2L; 

	long   samplesInDataFetch   =      BLOCKLOCKEDSIZE  /  bitSource->m_bytesPerSample;   //    11,040    *** Need to adjust for speed ????? 

	long   samplesInDataFetchSpeedRedux  =   (long)(    (double)samplesInDataFetch   /   playSpeedFlt   );


	long   totalSamplesInPieSection  =     (long)(  TWOSECBYTES  /  (  m_numberOfNotificationEvents  -1 )   )   / 4;     //  [ 1104 ]  How many samples in an Event  
						                    //  loads  44160 bytes ( 11,040 samples ) at a time.     11,040 / 10  hemisphereEvents =   1104  samples in  a PieSection    [ 3/11


	long          sampsInPieSliceWithSpeedRedux   =     Get_SampleCount_In_PieSection_wSpeedRedux();  
	ASSERT(   sampsInPieSliceWithSpeedRedux  > 0   );



	m_curScalePitch =     -1;  
	retLastSamplePlayed    =    -99;




	bool    doingFirstIteration =  false;
	short   preRollCode =  2;   //  default for this function

	if(     pieSliceCounter  ==  0    )
	{
		doingFirstIteration =   true;
		preRollCode =  1;
	}




	if(     iEvent  <    (unsigned long)( m_numberOfNotificationEvents -1 )     )          
	{
			
														//  If the play cursor has just reached the first or second half of the
														//  buffer, it's time to stream data to the other half.
		if(         iEvent ==   0								
			 ||    iEvent  ==   hemisEvent    )   
		{

			if(     m_curAudioPlayer->m_playingBackward    )
			{
				m_curAudioPlayer->Load_Next_DataFetch_Backward_PPlayer(   iEvent  );
				justDidaBlockLoad =   true;
		    }
			else
			{  m_curAudioPlayer->Load_Next_DataFetch_Forward_PPlayer(   iEvent    );  // ***************  ?????????????????   ***************
				justDidaBlockLoad =   true;
			}
		}


			
		if(    m_curAudioPlayer->m_doneFetchingPlayBytes ==   true   )    //  Happens when   Load_Next_DataFetch_Forward_PPlayer_NoAudio()  hits EOF  1/2012
		{
			return;        //  ***************   NEW,   1/31/2012   Is it OK ????   2/18 after 1.5 speed   *********************************
		}

	

		long  curSample =  -13;		//   'curSample'  is always VIRTUAL, that is it does NOT have the slowedDown expansion, or ReSample,  in its time-coords.
																	 	
		if(     m_curAudioPlayer->m_playingBackward    )
		{			
			curSample  =   ( bitSource->m_sampleIdxLastBlockLoadNotSlowExpanded  -  samplesInDataFetchSpeedRedux  )   
				                                                              -   (   bitSource->m_currentPieClockVal   *  sampsInPieSliceWithSpeedRedux  );    //  Notice the MINUS SIGN,  otherwise same as Forward  2/12
		}
		else
		    curSample  =   ( bitSource->m_sampleIdxLastBlockLoadNotSlowExpanded  -   samplesInDataFetchSpeedRedux  )      
				                                                              +   (   bitSource->m_currentPieClockVal   *  sampsInPieSliceWithSpeedRedux  );
		



			
		retLastSamplePlayed  =    curSample;     //   RETURN this value to the CALLING Function

		

		long   eventNum =   iEvent;    //  can be { 0 to 19 },  but reload with Load_Next_DataFetch_Forward() on  iEvent = 0 or 10

		if(    iEvent  >=   hemisEvent   )
			eventNum =    iEvent  -  hemisEvent;   
		else
			eventNum =   iEvent;    //  eventNum is { 0 thru 9 }   		




		m_outOfSyncStatusCode =   0;     // ******  TEMP DISABLE  *********


					
		CalcedNote    retCurrentNote; 
		long    eventsOffset      =     (long)(   eventNum   *  totalSamplesInPieSection  );    //   in SAMPLES,     eventNum is { 0 thru 9 }   ...10 pieSlices.
		long    eventsOffsetAdj  =    (long)(    (double)eventsOffset   /  playSpeedFlt    );  



		long     pieSliceIdx =  -13;
		long     pieSliceCountAtStart  =      m_curAudioPlayer->m_startSample   /   totalSamplesInPieSection;   // ********* WRONG ???  *******

	
		if(     m_curAudioPlayer->m_playingBackward    )
		{
			pieSliceIdx =   pieSliceCountAtStart    -   (  eventNum   +   ( halfEventsCount   *  ( m_curAudioPlayer->m_dataFetchCount   -1)    )  );  // *** UNTESTED  11/30/11 ****
			     //		TRACE(   "PieIDX  [  %d  ]                        Process_Event_Notification_PPlayer_NoAudio \n",    pieSliceIdx   );   
		}
		else
			pieSliceIdx =    eventNum   +   (   halfEventsCount   *  ( m_curAudioPlayer->m_dataFetchCount   -1)    )     +   pieSliceCountAtStart;  
																																	                 //  -1:  m_dataFetchCount   was updated before this   




		if(   sPitchCalcer->m_playModeUsingNotelist  ==   0   )      //  0:   Doing   DETECTION  
		{
			     
			if(    ! sPitchCalcer->Detect_Note_for_Pie_Slice(   curSample,    eventsOffsetAdj,    retCurrentNote,   m_curAudioPlayer->m_playingBackward,  
						                                                            *(sPitchCalcer->m_spitchCalcsSndSample),    useNavigatorDelay,   
																					           pieSliceIdx,   preRollCode,     pieSliceCounter,   retErrorMesg  )    )
			{  AfxMessageBox(  retErrorMesg  );   
				return;
			}
					

			if(               appCode  ==  1       //    1:   NAVIGATOR,    cause it has a  FinalCircque  for delay
				     &&   ! m_curAudioPlayer->m_playingBackward  )                      
				sPitchCalcer->Get_Oldest_Note_from_Final_CircularQue(   retCurrentNote   );		
				

			m_curScalePitch                         =      retCurrentNote.scalePitch;      //  MUST return results to Wavman  member vars so it can RENDER to the display.
			m_curScalePitchDetectionScore   =      retCurrentNote.detectScoreHarms;
			m_curScalePitchAvgHarmonicMag =     retCurrentNote.detectAvgHarmonicMag;



			if(     bitSource->m_recordNotesNow    )
			{			
			//	short   retSynthsCurPitch =   midiSequencer->m_curPitch;   //  **** WEIRD,  usually we need this value to PERSIST ( it does NOT here
																			//  because it is Local ).  BUT it is correctly re-assigned on each cycle, so I guess that is enough.  3/25/11

				if(    ! noteGenerator->Record_Midi_Note(   retCurrentNote,    retErrorMesg  )    )
				{  
					bitSource->m_recordNotesNow =   false;
					AfxMessageBox(  retErrorMesg  );
				}
			}					
		} 

		else
		{  ASSERT(  sPitchCalcer->m_playModeUsingNotelist  ==  1  );      //    1:  Playing from NOTELIST

//  NO Audio


			if(    ! sPitchCalcer->Search_NoteList_for_AudioMusical_Values(   curSample,   retCurrentNote,    m_curAudioPlayer->m_playingBackward,  
													                                                useNavigatorDelay,     pieSliceIdx,    iEvent,  iEvent,  
																									                               pieSliceCounter,   preRollCode,   retErrorMesg  )     )
			{  ASSERT( 0 );
				AfxMessageBox(  retErrorMesg  ); 
			}
				

			if(               appCode  ==  1       //    1:   NAVIGATOR,    cause it has a  FinalCircque  for delay
				     &&   ! m_curAudioPlayer->m_playingBackward  )                      
				sPitchCalcer->Get_Oldest_Note_from_Final_CircularQue(   retCurrentNote   );		
				

			m_curScalePitch                           =      retCurrentNote.scalePitch;       //  MUST return results to Wavman  member vars so it can RENDER to the display.
			m_curScalePitchDetectionScore     =      retCurrentNote.detectScoreHarms;
			m_curScalePitchAvgHarmonicMag   =     retCurrentNote.detectAvgHarmonicMag;
		}

	}	 //  	if(     iEvent  <    (unsigned long)( m_numberOfNotificationEvents -1 )
}





											////////////////////////////////////////


void   EventMan::Stop_Output()
{


//    ASSERT( 0 );    // ******  NEVER gets called ???   1/03 **********************


	/***
    This is called when a notification has been received 
    from the output buffer that all available data has been 
    played. The function stops the output buffer and clears 
    the notification position.
	***/

	if(         m_curAudioPlayer  ==  NULL    
		||   ! m_curAudioPlayer->IsOK()     )
	{   
	//	ASSERT( false );	
		return; 	
	}
	else
	{	m_curAudioPlayer->m_infcDSoundBuffer->Stop();
	}


    


// ***INSTALL   ... Reset some of MY flags ???   1/03



/***    ....NEED this ????    ...clears out all the Notivy events.

    if(   m_dsNotify  !=  NULL  )    //    WAS: lpdsnOutput
 			HRESULT hr =    m_dsNotify->SetNotificationPositions(  0,  NULL );   // **** NEEDED???
***/
}




										////////////////////////////////////////
										////////////////////////////////////////


bool    EventMan::PreRoll_NoteDetect(   long  advanceCalcSampleIdx,   bool  backwardsPlay,    long&  retLastSamplePlayed,    bool  useBoundaryRounding,   
																												                                    CString&  retErrorMesg   ) 
{

				//  Silently does calculations that will FILL the Delay-Pipeline ( CircQues, WAV Delay-Buffer, logDFTmap )
				//  so the DrivingView is updated, and later hitting the Play-Button does NOT have a significant Delay   3/11


				//  This funct is typically used when changing direction ( ex:  from forward to backward ).  The FinalCircQue needs to be reCalculated, and the
				//  current filePosition must be synced for the upcoming  Play_Forward  or  Play Backwards     10/11


				// 	  'advanceCalcSampleIdx'  is the INTENDED leading-sample of Current-Calculation for the Pipeline. When this funct finishes, 'retLastSamplePlayed'  
				//                                         will be very close to the value of  'advanceCalcSampleIdx'  ...it is a TARGET sampleIndex for this funct. 
				//
				//    'advanceCalcSampleIdx'  also pertains to  which end of the DrivingView will receive the latest  Note Calculations ( 
				//					ex:   For Forward,    'advanceCalcSampleIdx'  is the sampleIdx for the  x-column at the far RIGHT of the DriveView ( but  Hear-Sample  is on LEFT ) 
				//					ex:   For Backward,  'advanceCalcSampleIdx'  is the sampleIdx for the  x-column at the far LEFT of the DriveView  


				//    For Forward,  this function must reCalc the FinCircQue Note values, starting from the Left x-column (small SampleIdx)  and ending 
				//		                   at the far Right x-column ( large SampleIdx,  most in the future )    


			// ******   NEVER put  PreRoll  functions  in  SoundHelper,  because they are NOT for Pitchscope PLAYER *******

			
	retErrorMesg.Empty();
	retLastSamplePlayed = -98;


	BitSourceStreaming   *bitSource        =      Get_BitSourceStreaming();
	if(                              bitSource  ==  NULL  ) 
	{   retErrorMesg =   "PitchPlayerApp::PreRoll_NoteDetect  FAILED,   bitSource  is  NULL."   ;
		return  false;
	}

	ASSERT(   m_curAudioPlayer  !=  NULL );


	SPitchCalc  *sPitchCalc    =     Get_SPitchCalc();
	ASSERT(      sPitchCalc  );


	WavConvert   *wavConvert   =       bitSource->m_wavConvert;
	ASSERT(          wavConvert  );




	SequencerMidi  *midiSequencer =	  Get_Current_MidiSequencer();
	ASSERT(  midiSequencer  );


	/*********************   MAYBE  INSTALL   later...    If I want to use note lists.   1/2012    ***************************************

	NoteGenerator  *noteGenerator        =    bitSource->Allocate_NoteGenerator(  retErrorMesg  );
	if(                      noteGenerator  ==  NULL   )
	{  retErrorMesg   =   "PitchPlayerApp::PreRoll_NoteDetect  FAILED,   noteGenerator  is  NULL."   ;
		return  false;		
	}

	if(    ! noteGenerator->Init_NoteGenerator(   &m_calcedNoteList,   bitSource->m_strWavFilePath,    bitSource->Get_Files_Total_Output_Bytes(),  bitSource,   retErrorMesg  )    )
		return  false;	
	****/


	long	 pieSampleCount  =    sPitchCalc->Calc_Samples_In_PieEvent(   sPitchCalc->m_playSpeedFlt   ); 


				
	// ****************   NEW,  Need to RE-INITIALIZE the  SndSample- CircQue  (  SPitchCalc::m_circularSndSample[]   ) whenever 
	//										    we make a MAJOR move in FILE POSITION    1/12          **************

	sPitchCalc->Initialize_SndSample_CircularQue();    //  1/12



	bitSource->Initialze_For_File_Position_Change();




	double   playSpeedFlt          =     sPitchCalc->m_playSpeedFlt;


	long   earlyNotesCount     =     sPitchCalc->Approximate_Note_Count_In_FinalCircQue_with_BoundaryRounding(  playSpeedFlt,   
																							                                   useBoundaryRounding,    m_curAudioPlayer->m_bytesPerSample  );   // we need to re-populate the calced NOTES in the FinalCircQue

													//  Its sometimes important to ROUND to Load Boundaries or the sound will NOT sound continuous.   11/11

	long   earlySamplesCount  =   sPitchCalc->Approximate_Sample_Count_In_FinalCircQue_with_BoundaryRounding(  playSpeedFlt,   
																															  useBoundaryRounding,    m_curAudioPlayer->m_bytesPerSample     );



	long    laggedStartSampleIdx = 0;  




	long	  lastSamplesIdx = 		m_curAudioPlayer->Get_Biggest_SampleIndex_No_SpeedExpansion();     //  does NOT have the  expansion for SlowedDown, but has Expansion for ReSample 



	if(   backwardsPlay   )
	{
		laggedStartSampleIdx =     advanceCalcSampleIdx  +   earlySamplesCount;      //  Now calc the  'early'  ABSOLUTE File Position  to SEEK to. 

		if(   laggedStartSampleIdx   >=   lastSamplesIdx  )
			laggedStartSampleIdx  =    lastSamplesIdx  -  1;
	}
	else
	{	laggedStartSampleIdx =     advanceCalcSampleIdx  -   earlySamplesCount;   

		if(     laggedStartSampleIdx  < 0    )
			laggedStartSampleIdx =   0;   
	}




// ************   NEW  7/11/12    Also alaign the  Byte-ReadPosition in the FILE  to   "PieSlice-Samples  Boundary"   [ for speed 1 is mod of 1104 ]   ***********
//
//    ( I'm hoping that this will give move consistent results when we start to Record INSIDE the File, as opposed to starting from the front ( NoAudio record )


	long    laggedStartSampModulus  =    ( laggedStartSampleIdx      /   pieSampleCount )    *   pieSampleCount;




	long     laggedStartByteOffset  = 0;


	if(    laggedStartSampleIdx  !=   laggedStartSampModulus   )
		laggedStartByteOffset  =    laggedStartSampModulus    *   bitSource->m_bytesPerSample;  
	else
		laggedStartByteOffset  =    laggedStartSampleIdx        *   bitSource->m_bytesPerSample;  




	if(    ! bitSource->Seek_New_DataPosition_Fast(   laggedStartByteOffset,     BitSource::PREROLLinit,    retErrorMesg  )    )   //  PREROLLinit - Forces  move pf filePointer.
	{
		ASSERT( 0 );         
		return  false;
	}




								 	//     Also INITIALIZE  anything else for  Process_Event_Notification_PPlayer_NoAudio()  ???
  
	m_curAudioPlayer->m_startSample =    laggedStartSampleIdx;      // *****************   PROBLEM???  

	m_curAudioPlayer->Initialize_for_Playing_Section(); 




											//   read the file and do the  NoteDetection Calcs (and load the WAV Delay Buffer)  WITHOUT  Audio-HARDWARE
				

	bool   oldPlayerPlayingBackwardValue =   m_curAudioPlayer->m_playingBackward;    // ************************************* Was a BAD BUG that screwed up the RENDER and SELECTION  Disconnect on 4/27/2012
														//		***********************   WHY did I originally do this ???    4/2012 *****************************




	if(   backwardsPlay   )   
		m_curAudioPlayer->m_playingBackward  =   true;    
	else
		m_curAudioPlayer->m_playingBackward  =   false;      // *******  CAREFULL,  think I need to reint this when changing directions


	bool    hitEndOfFile =  false;
	long    retLastSamplePlayedProcessEventOuter =  -9;




//	long  pieClockDebug =  -1;


	for(    long   fakeEvent =0;      fakeEvent <   earlyNotesCount;       fakeEvent++   )  
	{

	    long  iEvent =      fakeEvent   %   (  m_numberOfNotificationEvents  -1);
	   	long  retLastSamplePlayedProcessEventLoc;



		Process_Event_Notification_PPlayer_NoAudio(    iEvent,    retLastSamplePlayedProcessEventLoc,    fakeEvent    );



		if(   m_curAudioPlayer->m_doneFetchingPlayBytes  ==  true   )       //  Might have hit EOF
		{			  

			hitEndOfFile =   true;     //  can land here on 1st Iteration when hit the  EndOfFile-Button 3/1/12

			if(         retLastSamplePlayedProcessEventLoc  <  0
				  &&  retLastSamplePlayedProcessEventOuter  <  0   )
			{
				long  totalSamples  =     m_curAudioPlayer->Get_Biggest_SampleIndex_No_SpeedExpansion();

				retLastSamplePlayedProcessEventOuter  =   totalSamples  -   earlySamplesCount;      // ******  APPROX??? ********** 
			}
			break;
		}
		else
		{				
			ASSERT(   retLastSamplePlayedProcessEventLoc  >=  0   );  // ****  Landed HERE:    [3/2/12, changing from slow speeds ]
			
		    retLastSamplePlayedProcessEventOuter =    retLastSamplePlayedProcessEventLoc;


			bitSource->Increment_Pie_Clock();     //    m_currentPieClockVal   **** NEED this or  'curSample' will not be advanced for each new


			m_pieSliceCounter++;   // ***********   BIG,  try to count how many times it is TOTALLY called  *******
		}
	}



//////////////////////////
	bool   didAPostRoll =  false;

	long   retCurSampleIdxLastCalcNoteAddedPOSTroll =  -1;   //   Since the next push of PlayButton will force  iEvent  to ZERO,  we must CALC/process the REMAINING
																		                  //   PieSlices ( LostEvents ) in the BlockLoad of  10 PieSlices.  3/12



	long   currentPieClockPosition =     bitSource->m_currentPieClockVal;        //  was  just INCREMENTED, after calcing the last CalcNote that went to the FinCircQue.

	long   lostEventCount             =     ( 9  - currentPieClockPosition  )   +1;      //  +1:  Inclusive Counting.    (  9 is the highest PieClock value )  



	if(    backwardsPlay    )
	{

		if(            lostEventCount  == 0    )   
		{
	//		TRACE(  "\n\n........Just Finished   ***PRE-Roll***    With   NO  LostEvents !!!!!    \n\n"   );
		}
		else  if(   lostEventCount   >  0    )   
		{
	//		TRACE(  "\n\n........Just Finished   ***PRE-Roll***    With   %d  LOST EVENTS      (  curSampleIdx LastNote:  %d   )   \n\n"  ,
	//																							                       lostEventCount,       retLastSamplePlayedProcessEventOuter   );
		}
	}
	else
	{  if(             lostEventCount  ==    0    //   playing   FORWARD

	               ||  lostEventCount  ==   10     )          // *********   NEW,  think this is right.      currentPieClockPosition =  0;     8/2012
		{

		//	TRACE(  "\n\n........Just Finished   ***PRE-Roll***    With NO  LostEvents !!!!!    \n"   );
		}
		else  if(           lostEventCount   >     0 
			          &&   lostEventCount    <=   9    )   // *********   NEW,  think this is right.      currentPieClockPosition =  0;     8/2012
		{
	//		  TRACE(  "\n\n........Just Finished   ***PRE-Roll***    curSampleIdx LastNote:  %d       ( doing  POST-Roll  next for  %d LostEvents )  \n"  ,
	//																				                                   retLastSamplePlayedProcessEventOuter,    lostEventCount   );


			if(     ! Post_Roll_NoteDetect(   backwardsPlay,   retCurSampleIdxLastCalcNoteAddedPOSTroll,  -1,   retErrorMesg  )     )
			{
				AfxMessageBox(  retErrorMesg   );
			//	return  false;   ** NO, let it keep going.  Not that big a deal
			}
			else
				didAPostRoll =   true;
		}
	}
/////////////////////////




	if(    didAPostRoll    )
	{
		if(   retCurSampleIdxLastCalcNoteAddedPOSTroll  <  0   )
		{
			int  dummy =   9;    //  WHEN last land HERE????   [     ]    When loading  GreySolo_22hz.wav[8/2/12]
		}

		retLastSamplePlayed  =      retCurSampleIdxLastCalcNoteAddedPOSTroll;  
	}
	else
	{  if(   retLastSamplePlayedProcessEventOuter  <  0   )
		{
			int  dummy =   9;   //  Last LAND HERE??   [     ]                 (  Land HERE once EndOfFile-Button, hit EOF, now fixed  3/1/12  )
		}

		retLastSamplePlayed  =      retLastSamplePlayedProcessEventOuter;    //  return to CALLING Function,  it will be a LITTLE different than 'advanceCalcSampleIdx'  
	}



	if(      retLastSamplePlayed  >=  0    )
		m_curAudioPlayer->Set_LastPlayed_SampleIdx(   retLastSamplePlayed   );    //   this way we can Start the Audio play at the right FilePos  3/11
	else
	{  int   dummy =   9;  }      //  WHEN last land HERE????   [     ]
	



// ************************************* Was a BAD BUG that screwed up the RENDER and SELECTION  Disconnect on 4/27/2012

//	m_curAudioPlayer->m_playingBackward =     oldPlayerPlayingBackwardValue;     // *****  Why WANT this type of restore ????  *******   11/11

// ***********************************************************************************************************



	bitSource->m_recordNotesNow =   false;     //  play it safe and restore 



	if(   backwardsPlay   )
	{
//		TRACE(  "\n\n    ...end of  BACKWARDS  Preroll   [   retLastSamplePlayed  %d ,      laggedStartSampleIdx %d    ]    \n\n",   retLastSamplePlayed,   laggedStartSampleIdx     );
	}
	else
	{   
//		TRACE(  "\n\n    ...end of  FORWARD  Preroll        [  laggedStartSampleIdx %d ,     retLastSamplePlayed  %d  ]    \n\n",      laggedStartSampleIdx,        retLastSamplePlayed   );
	}

	return  true;
}




										////////////////////////////////////////


bool    EventMan::PreRoll_NoteDetect_DEBUG(   long  advanceCalcSampleIdx,      bool  backwardsPlay,      long&  retLastSamplePlayed, 
																									                                    CString&  retErrorMesg   ) 
{

		//  Wrote this to help DEBUG a problem I had,  in that  the Recorded NoteList of a middle portion of the song were not getting the same 
		//  NoteStart values as a NoteList made from  Make_NoteList_No_Audio, which is the most accurate.   7/2012


	bool   forRecording =   true;   //  *************  SWITCH *************************




	retErrorMesg.Empty();
	retLastSamplePlayed = -98;


	BitSourceStreaming   *bitSource        =      Get_BitSourceStreaming();
	if(                              bitSource  ==  NULL  ) 
	{   retErrorMesg =   "EventMan::PreRoll_NoteDetect_DEBUG  FAILED,   bitSource  is  NULL."   ;
		return  false;
	}

	ASSERT(   m_curAudioPlayer  !=  NULL );

	ASSERT(  ! backwardsPlay   );


	SPitchCalc  *sPitchCalc    =     Get_SPitchCalc();
	ASSERT(      sPitchCalc  );


	SequencerMidi  *midiSequencer =	  Get_Current_MidiSequencer();
	ASSERT(  midiSequencer  );


	bool   fowardFlag;

	if(   backwardsPlay   )   
	{
		m_curAudioPlayer->m_playingBackward  =   true;  
		fowardFlag =  false;
	}
	else
	{	m_curAudioPlayer->m_playingBackward  =   false;      // *******  CAREFULL,  think I need to reint this when changing directions
		fowardFlag =  true;
	}



	bool   hasAmp3 =    Get_UniApp().Has_A_MP3();



	short    rightChannelPercent =     50;     //    50%    ******* ADJUST*****



	long      numberOfEvents =  21;
	double   playSpeedFlt      =     sPitchCalc->m_playSpeedFlt;

	long  numberBytesIn16bitSample =   4;
	long  hemisEvent  =   ( numberOfEvents -1 )   / 2;   // The event at 180 degrees,    for Player is 10

	long  numberOfBytesInFetch         =     BLOCKLOCKEDSIZE;      //   44160
	long  numSamplesInBlockLocked  =     BLOCKLOCKEDSIZE  /   numberBytesIn16bitSample;    //  11,040  samples in  Fetch_Streaming_Samples_Direct() block-load
	long  samplesInDataFetchSpeedRedux  =   (long)(    (double)numSamplesInBlockLocked   /   playSpeedFlt   );
	
	long  totalSamplesInPiesliceSpeedOne  =     (long)(  TWOSECBYTES  /  (  numberOfEvents  -1 )   )   / 4;     //  [ 1104 ]  How many samples in an Event  
						                //  loads  44160 bytes ( 11,040 samples ) at a time.     11,040 / 10  hemisphereEvents =   1104  samples in  a PieSection    [ 3/11

	long	pieSampleCountWithSpeedRedux =    sPitchCalc->Calc_Samples_In_PieEvent(   sPitchCalc->m_playSpeedFlt   ); 

	long  curSample =  0;  


	

	BYTE    *destBufferBytes =   NULL;     //  Allocate DUMMY buffer for Audio, which is NOT used here

	destBufferBytes  =    new    BYTE[   BLOCKLOCKEDSIZE   ];      //    44160  bytes  for   11,040  samples
	if(  destBufferBytes  ==  NULL   )
	{
		retErrorMesg =    "PreRoll_NoteDetect_DEBUG  FAILED,   destBufferBytes is  NULL ."  ; 
		return  false;	
	}


	WavConvert   *wavConvert   =       bitSource->m_wavConvert;
	ASSERT(          wavConvert  );



				
	// ****************   NEW,  Need to RE-INITIALIZE the  SndSample- CircQue  (  SPitchCalc::m_circularSndSample[]   ) whenever 
	//										    we make a MAJOR move in FILE POSITION    1/12          **************

	sPitchCalc->Initialize_SndSample_CircularQue();    //  1/12



	bitSource->Initialze_For_File_Position_Change();     //    m_currentPieClockVal  =  0;       m_byteIndexToBufferPieSlices =  0; 






// ********************   TRICKY need to calulate a new start point  from   'advanceCalcSampleIdx'

	long   laggedStartSampleIdx =0,     laggedStartByteOffset  =0;


	long    approxLagNoteCount  =     130;      // *******************  HARDWIRED,   FIX this  ******************



	long  sampsInMP3block  =    4608;   // *****  ALWAYS ????  *************************




	laggedStartSampleIdx  =    advanceCalcSampleIdx   -    (  approxLagNoteCount   *  pieSampleCountWithSpeedRedux  );      // *** 130,  that is what I usually use

//	laggedStartSampleIdx =   0;   //  **************  TEMP SWITCH,  force it to work from the front ***********





	long   modLaggedStartSample  =  -1;       //  Decide on the  BOUNDARY-Rounding   so we can get a good Seek    7/2012 


	if(   hasAmp3   )    // This is a little outdated,  these were for  Seek_fromBeginning_in_OutputCoords_BlockAlaign()   and   Seek_WalkfromBeginning_in_OutputCoords()
	{

		if(    forRecording   )
		{
			modLaggedStartSample  =     ( laggedStartSampleIdx    /   pieSampleCountWithSpeedRedux )    *  pieSampleCountWithSpeedRedux;   
																						// will WALK from FileStart   [  for  Seek_WalkfromBeginning_in_OutputCoords()
		}
		else
		{	long          mp3FrameSizeActual     =    wavConvert->m_mp3DecoderStreaming.Get_Frame_Size();
			ASSERT(   mp3FrameSizeActual   ==   sampsInMP3block  ); 
			

			modLaggedStartSample  =     ( laggedStartSampleIdx    /   sampsInMP3block )                          *  sampsInMP3block;       
												   //  Faster Seek algo,  but less accurate   [ for  Seek_fromBeginning_in_OutputCoords_BlockAlaign()   ] 7/2012
		}
	}
	else
		modLaggedStartSample  =     ( laggedStartSampleIdx    /   pieSampleCountWithSpeedRedux )    *  pieSampleCountWithSpeedRedux;   // WAV files
															                                                                           //   or MP3  using   Seek_fromBeginning_in_OutputCoords()



	laggedStartByteOffset  =      modLaggedStartSample   *  numberBytesIn16bitSample;






	long   totalEvents  =      ( advanceCalcSampleIdx  -   modLaggedStartSample )     /   pieSampleCountWithSpeedRedux;

//	long   totalEvents  =      advanceCalcSampleIdx   /   pieSampleCountWithSpeedRedux;





	if(    ! bitSource->Seek_New_DataPosition_Fast(   laggedStartByteOffset,   BitSource::PREROLLinit,   retErrorMesg  )    )   //  PREROLLinit - Forces  move pf filePointer.
	{
		ASSERT( 0 );         //  this will assign   'm_sampleIdxLastBlockLoadNotSlowExpanded'     [ see calc of  'curSample'   ]
		return  false;
	}



								 	//     Also INITIALIZE  anything else for  Process_Event_Notification_PPlayer_NoAudio()  ???
  
	m_curAudioPlayer->m_startSample =    modLaggedStartSample;     //  laggedStartSampleIdx;      


	m_curAudioPlayer->Initialize_for_Playing_Section(); 





	for(    long   fakeEvent =0;      fakeEvent <   totalEvents;              fakeEvent++   )  
	{

		bool    retHitEOF =   false;


	    long   iEvent =     ( fakeEvent   %   (numberOfEvents -1)    );

		long   eventNum =   iEvent;    //  can be { 0 to 19 },  but reload with Load_Next_DataFetch_Forward() on  iEvent = 0 or 10

		if(    iEvent  >=   hemisEvent   )
			eventNum =    iEvent  -  hemisEvent;   
		else
			eventNum =   iEvent;    //  eventNum is { 0 thru 9 }   		



		BYTE  *destBuffersPieSlicePtr   =   destBufferBytes                       //  I do NOT use the data in this buffer...   this would be for AUDIO of SlowedDown samples     7/2012
			                         +    ( eventNum *  totalSamplesInPiesliceSpeedOne  *  numberBytesIn16bitSample  );  




		if(   !  bitSource->Fetch_PieSlice_Samples(   totalSamplesInPiesliceSpeedOne,     //   'totalSamplesInPiesliceSpeedOne'  is in SlowedDown-Samples,  and  
			                                                                              destBuffersPieSlicePtr,                  //   Fetch_PieSlice_Samples will DECIMATE that sample count for the 
																						                 retHitEOF,   fowardFlag,  //  SndSample of detection, when calcing for SlowedDown-Speeds.    7/2012                       
													 						                                            rightChannelPercent,   0,  retErrorMesg   )    )
		{	if(   retHitEOF   )
			{
				break;
			}
			else if(  ! retErrorMesg.IsEmpty()    )
			{
				ASSERT( 0 );
				return  false;   
			}
		}
	


		curSample  =   ( bitSource->m_sampleIdxLastBlockLoadNotSlowExpanded  -   samplesInDataFetchSpeedRedux  )   
				                                                              +      (   bitSource->m_currentPieClockVal   *  pieSampleCountWithSpeedRedux  );

			//	  samplesInDataFetchSpeedRedux:	 [ 11,040  at speed 1 ]  

		    //   If at  SlowedDown of 2,   then sequence of  curSample   is  {  0,   552,   1104,   1656,   2208,  etc  }        7/201




		bitSource->Increment_Pie_Clock();    //   m_currentPieClockVal++;   


	}  //  for(  fakeEvent =0




	
	long    lastProcessedSampleIdx =    curSample;     //   ...like in DetectionZone     

	retLastSamplePlayed                =    curSample;    //  Looks OK


	if(      retLastSamplePlayed  >=  0    )
	{
		m_curAudioPlayer->Set_LastPlayed_SampleIdx(   retLastSamplePlayed   );    // ***********  ???  TROUBLE ??? ****************************************
	}																										//    this way we can Start the Audio play at the right FilePos  3/11
	


														 //  CLEANUP
	if(   destBufferBytes  !=  NULL   )       
		delete   destBufferBytes;

	return  true;
}






										////////////////////////////////////////


bool    EventMan::Post_Roll_NoteDetect(    bool  backwardsPlay,    long&  retLastSamplePlayed,  	 long  eventCount,    CString&  retErrorMesg   ) 
{

		//  if   'eventCount' is  <0,    then just use NORMAL mode to do LOST NOTES after Play has stopped.   4/12 
			

	retErrorMesg.Empty();
	retLastSamplePlayed =  -118;


//	SoundHelper&   soundHelper  =    GetSoundHelper();  **** NO,  EventMan can NOT Know of  SoundHelper or SoundMan  ( a conflict if both are here  3/12 )

	BitSourceStreaming   *bitSource        =      Get_BitSourceStreaming();
	if(                              bitSource  ==  NULL  ) 
	{   retErrorMesg =   "PitchPlayerApp::PreRoll_NoteDetect  FAILED,   bitSource  is  NULL."   ;
		return  false;
	}

	ASSERT(   m_curAudioPlayer  !=  NULL );

	SPitchCalc  *sPitchCalc    =     Get_SPitchCalc();
	ASSERT(      sPitchCalc  );

	SequencerMidi  *midiSequencer  =	    Get_Current_MidiSequencer();
	ASSERT(            midiSequencer  );

	if(   backwardsPlay   )
	{
		retErrorMesg =  "Post_Roll_NoteDetect FAILED, not implemented for Backwards Play." ;
		return false;
	}




	long  lastPieClockValue   =    bitSource->m_currentPieClockVal;    //   **** NEW,

	ASSERT(  lastPieClockValue >= 0    &&    lastPieClockValue  <=  10  );




	long   nextEvent  =     bitSource->m_currentPieClockVal;        //  was  just INCREMENTED, after calcing the last CalcNote that went to the FinCircQue.


	long   lastEventIdx,    lostEventCount;



	if(    eventCount  >=  0    )
	{												//  >0 :   SPECIAL Mode

		lostEventCount   =    eventCount;           

		lastEventIdx      =    nextEvent    +  eventCount   ;
	}
	else
	{	lastEventIdx      =       10;      //   < 0 :   NORMAL fill of  LOST Notes  ( 1 to 9  PieSlices will be processed ) 
													//                                                                      ....Only going to calc the remaining PieSlices in the Last DataLoad   2/28/12

		lostEventCount  =      ( 9  - nextEvent  )   +1;      //  +1:  Inclusive Counting.    (  9 is the highest PieClock value )  


		if(    lastPieClockValue  >  9   )                //  9 means 1 LostEvent to do.   if it were 10,  then a DBlockLoad will next happen.  3/12
			return  true;   // there should be NO Lost events
	}



	bool  playWithNoteList =  false;

	if(     sPitchCalc->m_playModeUsingNotelist  ==  1    )
		playWithNoteList =  true;

	/*********************   MAYBE  INSTALL   later...    If I want to use note lists.   1/2012    ***************************************

	NoteGenerator  *noteGenerator        =    bitSource->Allocate_NoteGenerator(  retErrorMesg  );
	if(                      noteGenerator  ==  NULL   )
	{  retErrorMesg   =   "PitchPlayerApp::PreRoll_NoteDetect  FAILED,   noteGenerator  is  NULL."   ;
		return  false;		
	}

	if(    ! noteGenerator->Init_NoteGenerator(   &m_calcedNoteList,   bitSource->m_strWavFilePath,    bitSource->Get_Files_Total_Output_Bytes(),  bitSource,   retErrorMesg  )    )
		return  false;	
	****/

	
	double   playSpeedFlt  =     sPitchCalc->m_playSpeedFlt;
	long   	sampsInPieSliceWithSpeedRedux  =     Get_SampleCount_In_PieSection_wSpeedRedux();



	CalcedNote   retNoteLastPlacedInCQue;
	sPitchCalc->Get_Newest_Note_from_Final_CircularQue(   retNoteLastPlacedInCQue   )	;

	long  curSamplePrevAtDetection  =     retNoteLastPlacedInCQue.curSampleWaveman;  



//	TRACE(   "\nSTART   POST-Roll....  \n"   );


	bool    hitEndOfFile =  false;
	long    retLastSamplePlayedProcessEventOuter =  -9;
	short   eventCounter =  0;



	for(   long  fakeEvent =  nextEvent;      fakeEvent <  lastEventIdx;       fakeEvent++   )     //   'lastEventIdx'  is usually 10
	{
			
	    long  iEvent =    fakeEvent   %   (  m_numberOfNotificationEvents  -1);   

		long  retLastSamplePlayedProcessEventLoc;



		Process_Event_Notification_PPlayer_NoAudio(    iEvent,    retLastSamplePlayedProcessEventLoc,   fakeEvent    );
																	                 //   'retLastSamplePlayedProcessEventLoc'   is   CalcNote::curSampleWaveman  for the  NEW CalcNote that was just added to the FinalCircQue


		if(   m_curAudioPlayer->m_doneFetchingPlayBytes  ==  true   )       //  Might have hit EOF
		{			  
			hitEndOfFile =   true;
			break;
		}
		else
		{	ASSERT(   retLastSamplePlayedProcessEventLoc  >=  0   );			
		    retLastSamplePlayedProcessEventOuter =    retLastSamplePlayedProcessEventLoc;

			bitSource->Increment_Pie_Clock();   //   *** BIG,  does  m_currentPieClockVal  ****     NEED this or  curSample will not be advanced for each new
			eventCounter++;    
		}
	}



	if(    retLastSamplePlayedProcessEventOuter   >  0   )
		retLastSamplePlayed  =    retLastSamplePlayedProcessEventOuter;     //  RETURN results to calling function
	else
	{
		int   dummy =  9;   //  WHEN does this get HIT (2/29/12) ??   At the END of the File
	}




	CalcedNote   retNoteLastPlacedInCQueAfterRoll;


	sPitchCalc->Get_Newest_Note_from_Final_CircularQue(   retNoteLastPlacedInCQueAfterRoll  );

	long  curSampleAfterPostRoll  =     retNoteLastPlacedInCQueAfterRoll.curSampleWaveman;  




	bitSource->m_recordNotesNow =   false;     //  play it safe and restore 


	if(   backwardsPlay   )
	{
//		TRACE(  "\n\n    ...end of  BACKWARDS  Preroll   [   retLastSamplePlayed  %d ,      laggedStartSampleIdx %d    ]    \n\n",   retLastSamplePlayed,   laggedStartSampleIdx     );
	}
	else
	{  // TRACE(  "     ...END of POST-Roll  [  %d  PIE-Slices processed  ]      %d  [%d]   LASTnoteInFinCircQue.curSampleWaveman    \n",   
		//															eventCounter,    curSampleAfterPostRoll,  retLastSamplePlayedProcessEventOuter     );
	}

	return  true;
}




											////////////////////////////////////////


bool   EventMan::Process_Midi_Event(    CalcedNote&  calcedNote,     SequencerMidi  *midiSequencer,     CString&  retErrorMesg  )
{

				//  Only gets called by    Process_Event_Notification_PPlayer()

			//    This does NOT make any decisions about where a note starts or stops ( like 
			//    This function just turns on the  MIDI SYNTH  to conform to the properties of  'calcedNote'   and   'calcedNote.synthCode'     1/12


	double   volumeBoost =   5.0;     //  ***ADJUST***  [3.0]weak with new fixed-filtering    [ 4.0 sill weak ]   [ 6 little strong ]


	bool      dumpToTrace =   false;



	 short  synthCode  =   calcedNote.synthCode;    //  Same as when it was a redundant input parm


	retErrorMesg.Empty();


	ASSERT(   m_curAudioPlayer  !=  NULL   );


	SPitchCalc   *sPitchCalcer       =       Get_SPitchCalc();
	if(                 sPitchCalcer  ==   NULL   )
	{
		ASSERT( 0 );
		retErrorMesg =    "EventMan::Process_Midi_Event  FAILED,  can NOT record notes,  SPitchCalc is NULL. ";
		return  false;
	}




	long         playSpeedFlt   =      m_curAudioPlayer->m_playSpeedFlt;
	ASSERT(  playSpeedFlt  >=  1.0  );



	/****************************  SAVE,     may use later for DEBUG data

	long	 sampleInPieEventWithSpeed  =     sPitchCalcer->Calc_Samples_In_PieEvent(  playSpeed   );   

	long    primaryQueDelay =    sPitchCalcer->Calc_Primary_CircQues_Delay_In_CalcedNotes();     //     2
	primaryQueDelay  =     primaryQueDelay   *  sampleInPieEventWithSpeed;

	long	  logDFTdelay =   	sPitchCalcer->Calc_LogDFTs_Delay_In_CalcedNotes();     //   4
	logDFTdelay  =    logDFTdelay   *  sampleInPieEventWithSpeed;


//	long   totalPrimaryDelay  =     primaryQueDelay   +   logDFTdelay;   this was too big.  WHY???     3/11
//	long   totalPrimaryDelay  =       logDFTdelay;   //  works the best    ******************   DISABLE,  now done in Get_New_Filtered_ReturnNote_CircularQue   3/11
//	long   totalPrimaryDelay  =   0;  // ******  DISABLE,  now doen in  Get_New_Filtered_ReturnNote_CircularQue() 3/11 *********
	*****/



	bool   haveAnewNoteStart =   false;

	if(                calcedNote.synthCode  ==   1          //  1:  start of new note
			   &&    calcedNote.scalePitch   >=   0   
			   &&    calcedNote.beginingSampleIdxFile  >=   0       )      
		haveAnewNoteStart =   true;



	long   dataFetchCount  =     m_curAudioPlayer->m_dataFetchCount;




	if(    synthCode  ==  1   )
	{

		
		if(       calcedNote.octaveIndex   <   0  
			||   calcedNote.octaveIndex   >=  4  )    
		{  
			if(   dumpToTrace   )
			{	TRACE( "*** Can NOT find the octave***\n"  );  
			}
		}
		else
		{	ASSERT(    calcedNote.scalePitch    >=  0   );   //  Can land here if  FinCircQue is out of sync with the NoteList, which just got an EDIT.   6/4/2012 

			long    midiVolumeApp =   100;
			short   auditionCode =       1;      //  1:   AnimeStream::MIDIandSAMPLE 


			if(    m_curAudioPlayer->m_bitSource->m_midiVolumeSoundmanAddr   !=  NULL   )
			{
				midiVolumeApp  =      *(   m_curAudioPlayer->m_bitSource->m_midiVolumeSoundmanAddr   );
			}
			else	
			{ ASSERT( 0 );  }




					//   Uses the Average-Harmonic for the note Volume of the Note,  not the  DetectScore ( PitchScope uses the DetectScore to control volume  )

					//   ...a little TRICKY for the creation of a new Note in a notelist,  because  calcedNote only stores   'scoreDetectionAvgTone' 


			double   calcVolumeFlt  =      (double)(   calcedNote.detectAvgHarmonicMag )   *  volumeBoost   *   (  (double)midiVolumeApp /100.0); 

			short     calcedVolume =   (short)calcVolumeFlt;

			if(          calcedVolume   >  126    )
				calcedVolume =   126;
			else if(   calcedVolume   <     0    ) 
				calcedVolume =     0;




			if(   calcedNote.synthCode  !=  1  )    //   **** REDUNDANT???  it already has this value ******************************
			{	ASSERT( 0 );   }

			calcedNote.synthCode =   1;   //  big,   record what we did with this note   





			short   pitchMidi        =    m_curScalePitch   +  64;     // 76;
			

// *****  CAREFUL:    IS    logDFTdOWNWARDrEAD    is set to  ZERO  ?????   ******************************************

			pitchMidi  =     52   +   calcedNote.scalePitch   +    (  calcedNote.octaveIndex *  12 );        //   52  =   kMIDIsTARTnOTE
	//		pitchMidi  =     40   +   calcedNote.scalePitch   +    (  calcedNote.octaveIndex *  12 );        //   40 is an octave below
						


			
			bool   justSwitchingToANewPitch  =   false;


			if(     pitchMidi  !=    midiSequencer->m_curPitch 

//				    &&   (   midiSequencer->m_curPitch >= 0  &&   midiSequencer->m_curPitch  <= 11  )     ****** WANT this ???    3/11   
			  )   
			{   justSwitchingToANewPitch  =   true;    }
			
			



			if(         m_lastStartPlayingNoteMusicalKey.scalePitch  >= 0    &&   m_lastStartPlayingNoteMusicalKey.scalePitch  <= 11  
				 &&   m_lastStartPlayingNoteMusicalKey.beginingSampleIdxFile  >= 0  
				 
				 &&   justSwitchingToANewPitch   ) 	 // ************   OK ???  ************************
			{

				Add_New_Note_to_MusicalKey_Counter(    m_lastStartPlayingNoteMusicalKey.scalePitch,    m_lastStartPlayingNoteMusicalKey.beginingSampleIdxFile, 
																                                                                   calcedNote.beginingSampleIdxFile -1    );
			}

			m_lastStartPlayingNoteMusicalKey =   calcedNote;
							



			if(    ! midiSequencer->m_silenceMidi    )     //  user could have checked the MUTE MIDI  Checkbox on the dialog    1/12
			{

				if(    ! Play_Midi_Pitch_WMan(   pitchMidi,    calcedVolume,    auditionCode,    retErrorMesg   )      ) 
				{
					ASSERT( 0 );    //   ??????????   Should have error report ????							
				}
			}
		}
	}     //   if(    synthCode  ==  1 


	else if(   synthCode  ==  0   )
	{  

//		if(   dumpToTrace   )		{   TRACE(  "STOP synth  "   );   }



		if(   calcedNote.synthCode  !=  0  )    //   **** REDUNDANT???  it already has this value  *************************************
		{	ASSERT( 0 );   }

		calcedNote.synthCode =   0;   //  BIG,   record what we did with this note  **** REDUNDANT???  it already has this value




		if(         m_lastStartPlayingNoteMusicalKey.scalePitch  >= 0    &&   m_lastStartPlayingNoteMusicalKey.scalePitch  <= 11  
			 &&   m_lastStartPlayingNoteMusicalKey.beginingSampleIdxFile  >= 0   ) 	
		{
			Add_New_Note_to_MusicalKey_Counter(    m_lastStartPlayingNoteMusicalKey.scalePitch,     m_lastStartPlayingNoteMusicalKey.beginingSampleIdxFile, 
																		                                                      calcedNote.beginingSampleIdxFile -1     );
		}

		m_lastStartPlayingNoteMusicalKey =   calcedNote;




		if(   ! Stop_MidiPlay_WMan(         retErrorMesg  )     )
		{
			ASSERT( 0 );    //   ??????????   Should have error report ????							
		}
	}

	else if(   synthCode  ==  -1   )  	// ******  Unecessary SLOPPY,  now it seems that  synthCode  will have same value as  calcedNote.synthCode 
	{   


		if(   calcedNote.synthCode  !=  -1  )    //   **** REDUNDANT???  it already has this value ******************************
		{	ASSERT( 0 );   }


		calcedNote.synthCode =   -1;   // will probably not process these,  but record the  synth-state anyway    ************  MIGHT not need this assignment   3/30   ***********
	}     //   do nothing



	return  true;
}



											////////////////////////////////////////


bool   EventMan::Play_Midi_Pitch_WMan(    short  pitchMidi,    short   volume,    short  auditionCode,    CString&  retErrorMesg   )
{

	retErrorMesg.Empty();


	SequencerMidi   *midiSequencer  =		 Get_Current_MidiSequencer();

	if(   midiSequencer  ==  NULL  )     
	{
		ASSERT( 0 );

		retErrorMesg =   "Play_Midi_Pitch_WMan  failed,   m_midiSequencer  is NULL." ;
		return  false;
	}



	//		{  NORMAL,     MIDIandSAMPLE,     AllLEFT,  AllRIGHT,     JUSTMIDI,     MIDIandLEFT,   MIDIandRIGHT    };  

   //																							MIDIandSAMPLE( mid plus stereo ),    MIDIandLEFT( midi and JustLeft WAV  )			



	if(        auditionCode    !=    AnimeStream::MIDIandSAMPLE   
		&&	 auditionCode    !=    AnimeStream::JUSTMIDI    
		&&	 auditionCode    !=    AnimeStream::MIDIandLEFT    
		&&	 auditionCode    !=    AnimeStream::MIDIandRIGHT    )	 //  only Midi PLAY ops below	 
		return  true;
		


	if(         pitchMidi  <  0
		||  (   volume   <  0    ||   volume  >  127   )    	)			//   'volume'  is MIDI,   so it can NOT be bigger han 127  [ I tested this.  2/03 ]  
	{
		retErrorMesg =  "Play_Midi_Pitch_WMan  failed,   incorrect input params." ;
		return  false;
	}



	midiSequencer->m_tickCount++;




	if(   midiSequencer->m_curPitch    !=    pitchMidi    )   //  CurrentPlaying pitch is  not same  as INTENDED from animationMask
	{																		        //   ...OR  nothing  is playing,  but we need to turn a note on to match the  INTENDED 


		short   holdOldValue  =   midiSequencer->m_curPitch;



		if(    midiSequencer->m_curPitch  > 0   )     //  a note is still playing,  but it is NOT our INTENDED pitch
		{
			if(     ! midiSequencer->Stop_Play_curNote(   retErrorMesg   )    )
				return  false;


		//	TRACE( "Stop[%d, %d,    %d]   ",   holdOldValue,   m_midiSequencer->m_tickCount,   curSample  );


			midiSequencer->m_tickCount =  0;
		}




		if(                  pitchMidi  > 0											 	             //  We need a note playing but the currentPlaying pitch is not it
			     &&     midiSequencer->m_curPitch   !=   pitchMidi        //  m_midiSequencer->m_curPitch will equal -1 after  Stop_Play_curNote()   ****  OMIT ??? ***************
		  )
		{  

			 bool  overideToOrigVolume =  false;  


			if(     auditionCode   ==    AnimeStream::JUSTMIDI   )
			   overideToOrigVolume =  true;  



			if(    ! midiSequencer->Start_Play_curNote(   pitchMidi,   volume,   overideToOrigVolume,   retErrorMesg  )    )
				return  false;


		//	TRACE( "Start[%d,  %d,         %d]   \n",    pitchMidi,   m_midiSequencer->m_tickCount,  curSample    );

			midiSequencer->m_tickCount =  0;
		}
		else
		{	   TRACE( "\n  pitchMidi  <= 0  \n",   pitchMidi  );  }		

	}  

	return  true;
}



											////////////////////////////////////////


bool	EventMan::Stop_MidiPlay_WMan(   CString&  retErrorMesg    )
{

	retErrorMesg.Empty();

	
	SequencerMidi   *midiSequencer       =		 Get_Current_MidiSequencer();
	if(                      midiSequencer  ==  NULL   )
	{
		//  retErrorMesg  =  "SoundMan::MidiPlay_DSTlist  failed,  no  midiSequencer." ; 
		//  return  false;
		return  true;
	}



	if(     ! midiSequencer->Stop_Play_curNote(  retErrorMesg   )      )   //  stop any sounds at end
		return  false;

	
//	if(     !Analyze_Tap_Events(  retErrorMesg   )      )   //  stop any sounds at end
//		return  false;


	return  true;
}


											////////////////////////////////////////
											////////////////////////////////////////


void   EventMan::Initialize_MusicalKey_Note_Counters()
{

	for(   long i=0;    i< 12;    i++   )
	{
		m_noteScoresMusicalKey[  i  ] =   0;
//		m_noteCountsMusicalKey[  i  ] =   0;
	}


	m_lastStartPlayingNoteMusicalKey.beginingSampleIdxFile =    -9;

//	m_lastStartPlayingNoteMusicalKey.endingSampleIdxFile    =   -9;  

	m_lastStartPlayingNoteMusicalKey.scalePitch =  -9;


	m_noteCountMusicalKey =  0;
}


											////////////////////////////////////////


void   EventMan::Add_New_Note_to_MusicalKey_Counter(   short  scalePitch,   long  beginingSampleIdxFile,   long  endingSampleIdxFile   )
{


	long   divisor =   1000;    //   ***********  ADJUST ??  *********************


	if(       scalePitch  < 0   
		||   scalePitch >  11  )
	{
		return;    // is not a real note
	}



	long    notesLength  =       endingSampleIdxFile    -    beginingSampleIdxFile;



	if(    notesLength  <=  0     )
	{
		int   dummy =  0;   // Get here sometimes when changing speeds as music plays   1/7/2012
	}  
	else
	{
		notesLength =    notesLength / divisor;

	  
		long   notesScore =     notesLength  *  notesLength;    //  Us the SQUARE:     want to geive extra weighting to long notes.   1/2012
				

		m_noteScoresMusicalKey[   scalePitch   ]   +=   notesScore;
	 

		m_noteCountMusicalKey++;
	}
}






										////////////////////////////////////////
										//////   OLD PitchScope  /////////


void   EventMan::Process_Event_Notification(   unsigned long   iEvent,     SequencerMidi  *midiSequencer    )
{

													   	//   Is   CALLED  by    CVoxSepApp::Run(),  


//	SoundHelper&   GetSoundHelper();   **** NO,  EventMan can NOT KNOW of  SoundHelper or SoundMan  ( a conflict if both are here  3/12 )



	CString   retErrorMesg;

	unsigned long   hemisEvent  =    ( (unsigned long)( m_numberOfNotificationEvents -1 ) )  /2;   // The event at 180 degrees


	m_curScalePitch =  -1;   //  OK to initialize like this ????     12/09



	if(       m_curAudioPlayer  ==  NULL    
		||   !m_curAudioPlayer->IsOK()     )
	{   
		ASSERT( false );	
		return; 	
	}


	ASSERT( midiSequencer );


	if(     iEvent  <   (unsigned long)( m_numberOfNotificationEvents -1 )      )          
	{

		long  curSample;
		
														//  If the play cursor has just reached the first or second half of the
														//  buffer, it's time to stream data to the other half.
		if(         iEvent ==   0								
			 ||    iEvent  ==   hemisEvent    )   
		{
			if(     m_curAudioPlayer->m_playingBackward    )
				m_curAudioPlayer->Load_Next_DataFetch_Backward(   iEvent  );      //  the ONLY place this is CALLED 
			else
				m_curAudioPlayer->Load_Next_DataFetch_Forward(     iEvent   );   
		}
		
		

		if(     m_curAudioPlayer->m_playingBackward    )
			curSample =   m_curAudioPlayer->Calc_Current_Sample_Backward(   iEvent   );
		else
			curSample =   m_curAudioPlayer->Calc_Current_Sample_Forward(      iEvent   );   // BIG Problems if this is before Load_Next_DataFetch_Forward()  





									//   CALC  a   'fine tune'  a  DELAY  for  ANIMATION  with   StreamingFile / StaticBuffer   play


		// **** BUG,  if  OptionsDlg  'Animation Time Delay'  is NOT zero, may hear( midi + animation ) neighbor notes as well

		long   numHemisphereEvents =    m_numberOfNotificationEvents  / 2,        animeCurSample;

		long   pieSliceSamples  =    m_curAudioPlayer->m_bytesInFetch   /   (  m_curAudioPlayer->m_bytesPerSample  *  numHemisphereEvents  );



		if(    m_curAudioPlayer->m_playSpeedFlt   ==   1.0   )
		{
			animeCurSample  =   curSample   -    (  m_curAudioPlayer->m_animationSamplesDelay    *  pieSliceSamples );   
		}
		else
		{	
			if(    m_curAudioPlayer->m_playSpeedFlt   !=  0.0    )
			{

				double  speedSquared =    m_curAudioPlayer->m_playSpeedFlt   *    m_curAudioPlayer->m_playSpeedFlt ;  
				
				long     adjDelay =    		
					   (long)(      (  (double)m_curAudioPlayer->m_animationSamplesDelay  *  (double)pieSliceSamples  )   / speedSquared     );
				

				animeCurSample  =    curSample   -   adjDelay;
			}
			else
				animeCurSample  =    curSample;
		}


		if(     animeCurSample   < 0    )
			animeCurSample =  0;





		if(    m_curAudioPlayer->Is_Playing()    )
		{
					  //   Do NOT want to change  UniBasic::m_lastFilePosition [   Set_LastPlayed_SampleIdx()  ].   If 
			          //   in  'LoopPlay'  want to PRESERVE  this value in case RePlay_Previous_Phrase() is called again     2/03

			if(           
				         m_curAudioPlayer->m_playMode   !=   AudioPlayer::LASTnOTEpLAYfORWARD  
			      &&   m_curAudioPlayer->m_playMode   !=   AudioPlayer::LASTnOTEpLAYbACKWARD  		
		    //   &&   m_curAudioPlayer->m_playMode   !=    AudioPlayer::RETURNtoPLAYSTART  *** Disconnected 4/14/07. This way allows speed-change not to interrupt play. Be careful of sideeffects !!!  4/07

	        //	  &&   m_curAudioPlayer->m_playMode   !=   AudioPlayer::LOOPpLAYsELCT   ***NO, unecessary !  This screws up on speed-change.
			  )  
			{	m_curAudioPlayer->Set_LastPlayed_SampleIdx(   animeCurSample   );     //   save for SoundMan::Continue_Play_Forward(),  but only if the buffer IS STILL playing.   
			}



			if(        m_curAudioPlayer->m_playMode   !=    AudioPlayer::RETURNtoPLAYSTART   
				&&   m_curAudioPlayer->m_playMode   !=    AudioPlayer::LOOPpLAYsELCT    //  Not sure if this is necessary, but works so far.  4/07
			  )   
																			  //  Want the first StepNotePlay to happen at the window's left edge 
				m_curAudioPlayer->m_lastNotePlaySample =    animeCurSample;    //  Only for  Play_Closest_Note_Forward/Backward()  mode

		}






														  //    STOP  hardwarePlay  if hit  last INTENDED  sample 


		if(      (   ! m_curAudioPlayer->m_playingBackward    &&     animeCurSample  >=  m_curAudioPlayer->m_endSample    )
			||   (    m_curAudioPlayer->m_playingBackward    &&     animeCurSample  <=  m_curAudioPlayer->m_endSample    )   )
		{

			m_curAudioPlayer->StopPlaying_Hardware();   //  get her form  StepNote_Play   or  Section  play   or   EOF-play



		//	if(     ! Stop_MidiPlay_WMan(  midiSequencer,   retErrorMesg  )     ) 
			if(    ! Stop_MidiPlay_WMan(  retErrorMesg  )     )
			{																		  //    soundman::Analyze_Tap_Events()   ..throws errorMesg																	
				AfxMessageBox(  retErrorMesg  );  
			}

		
			if(     m_curAudioPlayer->m_playMode  ==    AudioPlayer::NORMALpLAY   )
				m_curAudioPlayer->m_prevEndSamplePlay  =    m_curAudioPlayer->m_endSample;	 


							//  For  Loop Play  we just restart the thing [ watch out that I do not 'blow the stack' with the recursion to StartPlaying()  4/07  

			if(     m_curAudioPlayer->m_playMode  ==   AudioPlayer::LOOPpLAYsELCT   )
			{

				 bool  preventFileSeek =   false;   // **** NEW,  untested     11/11


				if(    ! m_curAudioPlayer->StartPlaying(   AudioPlayer::LOOPpLAYsELCT,   m_curAudioPlayer->m_playSpeedFlt,    false,    
																		   m_curAudioPlayer->m_prevStartSamplePlay,   m_curAudioPlayer->m_prevEndSamplePlay,    
																		   false,  preventFileSeek,    retErrorMesg  )    )
					AfxMessageBox(  retErrorMesg  ); 
			}
		}



														  //    Render  ANIMATION  and do  MIDI-Play 


		if(    m_curAudioPlayer->m_isPlaying    )    //  at  EndOfPlay  we still go thru this a couple of times, even though we no longer Play or Load_Next_DataChunks  1/03
		{
			m_curAudioPlayer->m_theApp.Animate_All_Viewjs(   animeCurSample,   m_curScalePitch   );                           //  only called here  12/09
		}																					//  also triggers  MIDI-play  




		//   TRACE(  "notifiction:   sampleIDX[ %d  ],   delta[ %d ],      iEvent[ %d ]  \n",    	curSample,    (curSample - lastSamplePos),     iEvent    );  

		//		lastSamplePos =   curSample;    //  a local var,  debug only
		//		m_curAudioPlayer->m_notificationCount++;     // increment our counter	..do NOT think anything READS this vale( could omit ?? )	

	}

	else    //    iEvent  ==   ( m_numberOfNotificationEvents -1 )       3)  [   play is  STOPPED,    adjust  vars    ]   
	{			

	
		if(       m_curAudioPlayer->m_playMode  ==   AudioPlayer::LASTnOTEpLAYfORWARD  
			 ||   m_curAudioPlayer->m_playMode  ==   AudioPlayer::LASTnOTEpLAYbACKWARD    )  
		{
			m_curAudioPlayer->Set_Play_Mode(   AudioPlayer::NORMALpLAY    );      //   reset to default    ??? Is this NECESSARY ???  
		}

		

		if(    !m_curAudioPlayer->m_isPlaying    )			//  Play has STOPPED by reaching the  'END-of-FILE' 
		{		
			 
			 long   lastPlayedSampleIdx  =     m_curAudioPlayer->Get_LastPlayed_SampleIdx(); 
			 
			 long   biggestVirtSample      =     m_curAudioPlayer->Get_Biggest_SampleIndex_No_SpeedExpansion();     //  does NOT have the  expansion for SlowedDown 
			 


			 if(     lastPlayedSampleIdx   >=    biggestVirtSample    )
			 {

				long   sampleCnt  =       m_curAudioPlayer->m_sampleCountInViewjsWidth; 
				if(      sampleCnt  >  0   )
				{
					lastPlayedSampleIdx  -=    ( sampleCnt  *    3L)  /  4L;   //  Nudge image to left so we can see some of Files last samples

					if(    lastPlayedSampleIdx  < 0    )
						lastPlayedSampleIdx =  0;


					m_curAudioPlayer->Set_LastPlayed_SampleIdx(      lastPlayedSampleIdx    );							

					m_curAudioPlayer->m_theApp.Update_FirstSampleIdx_Active_RankingViewj(   lastPlayedSampleIdx   );   
					m_curAudioPlayer->m_theApp.Update_FirstSampleIdx_All_AnimeViewjs(          lastPlayedSampleIdx  );
				}
				else
				{  
					if(   sampleCnt  ==  -118  )   //  means SoundMan::Sync_AudioPlayers_Vars() was never called,  and this is PitchPlayer.exe that is running.
					{                                         //  ( Install a better test for these applications ???  )

						lastPlayedSampleIdx =   0;   //  for   PitchPlayer.exe   we go to the start of the sample if we play to the end.    1/10 

// ****************  BIG PROBLEM if I go to this from PitchScope.exe or VoxSep.exe  ...   1/10   ********************************
					}
					else
					{  long   sampleCnt  =    22100;    //  Is a half second reasonable ???


						lastPlayedSampleIdx  -=    ( sampleCnt  *    3L)  /  4L;   //  Nudge image to left so we can see some of Files last samples

						if(    lastPlayedSampleIdx  < 0    )
							lastPlayedSampleIdx =  0;
					}


					m_curAudioPlayer->Set_LastPlayed_SampleIdx(      lastPlayedSampleIdx    );							

					m_curAudioPlayer->m_theApp.Update_FirstSampleIdx_Active_RankingViewj(   lastPlayedSampleIdx   );   
					m_curAudioPlayer->m_theApp.Update_FirstSampleIdx_All_AnimeViewjs(          lastPlayedSampleIdx  );
				}
			 }

			 
																				   //  Since we did NOT hit Pause()  in songs middle( while animating ),  we must
			m_curAudioPlayer->Draw_Last_AnimeFrame();   //  explicitly do this Draw, because last animation just showed a blank image( EndOfFile )
		}																              		
	}
}
