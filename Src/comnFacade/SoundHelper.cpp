/////////////////////////////////////////////////////////////////////////////
//
//  SoundHelper.cpp   -     a Sound and Midi Utility class
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


#include  <math.h>   

#include  "Mmsystem.h"     //   or   #include  <mmsystem.h> ??      for MIDI    Version 4.00    from the  SDK(careful)



#include   "..\comnFacade\UniEditorAppsGlobals.h"

#include  "..\comnFacade\VoxAppsGlobals.h"



//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"     
#include  "..\comnGrafix\CommonGrafix.h"     



#include   "..\ComnGrafix\AnimeStream.h"


#include  "..\comnAudio\SequencerMidi.h"			



#include "..\comnInterface\DetectorAbstracts.h"    
///////////////////////////	





#include  "..\ComnAudio\NoteGenerator.h"



//////////////////////////

#include  "..\ComnAudio\dsoundJM.h"      

#include  "..\ComnAudio\CalcNote.h"

#include  "..\ComnAudio\SPitchCalc.h"


#include  "..\ComnAudio\EventMan.h"    

#include  "..\comnAudio\BitSourceAudio.h"

///////////////////////////




///////////////////////////

#include  "..\ComnAudio\PlayBuffer.h"

#include  "..\ComnAudio\AudioPlayer.h" 

///////////////////////////





#include  "..\ComnFacade\SoundHelper.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////




			////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////


#define  PATCHnAMEScOUNT  96


char    *patchNames[  PATCHnAMEScOUNT  ]  =
{
	"Acoustic grand piano",	 "Bright acoustic piano",	  "Electric grand piano",   "Honky-tonk piano",    "Rhodes piano",   
	
	"Chorused piano",   "Harpsichord",   	"Clavinet",   "Celesta",   	"Glockenspiel",    

	"Music box",    "Vibraphone",    "Marimba",    "Xylophone",    "Tubular bells",   

	"Dulcimer",    "Hammond organ",    "Percussive organ",    "Rock organ",    "Church organ",   

	"Reed organ",    "Accordion",    "Harmonica",    "Tango accordion",    "Acoustic guitar (nylon)",   

	"Acoustic guitar (steel)",    "Electric guitar (jazz)",    "Electric guitar (clean)",    "Electric guitar (muted)",    "Overdriven guitar",   

	"Distortion guitar",    "Guitar harmonics",    "Acoustic bass",    "Electric bass (finger)",    "Electric bass (pick)",   

	"Fretless bass",    "Slap bass 1",    "Slap bass 2",    "Synth bass 1",    "Synth bass 2",   

	"Violin",    "Viola",    "Cello",    "Contrabass",    "Tremolo strings",   

	"Pizzicato strings",    "Orchestral harp",    "Timpani",    "String ensemble 1",    "String ensemble 2",   

	"Synth. strings 1",    "Synth. strings 2",    "Choir Aahs",    "Voice Oohs",    "Synth voice",   

	"Orchestra hit",    "Trumpet",    "Trombone",    "Tuba",    "Muted trumpet",   

	"French horn",    "Brass section",    "Synth. brass 1",    "Synth. brass 2",    "Soprano sax",   

	"Alto sax",    "Tenor sax",    "Baritone sax",    "Oboe",    "English horn",   

	"Bassoon",    "Clarinet",    "Piccolo",    "Flute",    "Recorder",   

	"Pan flute",    "Bottle blow",    "Shakuhachi",    "Whistle",    "Ocarina",   

	"Lead 1 (square)",    "Lead 2 (sawtooth)",    "Lead 3 (calliope)",    "Lead 4 (chiff)",    "Lead 5 (charang)",   

	"Lead 6 (voice)",    "Lead 7 (fifths)",    "Lead 8 (brass)",    "Pad 1 (new age)",    "Pad 2 (warm)",   

	"Pad 3 (polysynth)",    "Pad 4 (choir)",    "Pad 5 (bowed)",    "Pad 6 (metallic)",    "Pad 7 (halo)",   

	"Pad 8 (sweep)"
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


UniApplication&      Get_UniApp();

EventMan&            Get_EventMan(); 




									///////////////////////////////////////////////////


void        Begin_ProgressBar_Position_GLB(  char  *text    );
void        Set_ProgressBar_Position_GLB(   long  posInPercent   );
void        End_ProgressBar_Position_GLB();


void    Write_To_StatusBar_GLB(   CString&   mesg   );




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


SoundHelper::SoundHelper()
{

	m_audioPlayer     =    NULL;

	m_midiSequencer =   NULL;

	m_deviceIDofInternalSynth =  -1;

	Set_BitSource_Streaming(  NULL  );


						/***
						short    clarinet =    71;
						short    sax      =    64;      //  64 -  67  
						short    violin     =    40;
						short    viola      =    41;
						short    cello    =      42;
						short    strings   =    48;
						short     guitarOverdrive    =    29;
						short     guitarDistort         =    30;
						short     reedOrgan   =    20;      //  16 - 18,   20
						short     harpsicord   =     6;
						short     oboe    =      68;
						short     fiddle    =    110;
						***/
	m_midiInstrumentPatch  =   17;       //   17:   PercussiveOrgan,   DEFAULT           

	m_lastTransportDirection          =    SoundHelper::UNKNOWNtrans;
	m_lastTransportActionActual =    SoundHelper::UNKNOWNtrans;


	m_inputScaleingFactor =   0;    //   0:  default   


	m_curScalePitchAtStop  =   -1;                             //  the current vale when the user hit the PAUSE BUTTON, or other stop
	m_curScalePitchDetectionScoreAtStop =  0;
	m_curScalePitchAvgHarmonicMagAtStop =   0;


	m_muteMouseSelectionAudio  =  false;    //  NEW

	m_muteReversePlayAudio       =   false;


	m_displayBritnessFactor  =   2;              //   2: Init with middle value.     5 values    {  0,  1,  2,  3,  4  }


//	m_computerPerformanceFactor =  0;     //   3 values       0: Fast     1: Average     2: Slow


	m_userFinalCircqueSizeTweak     =   0;

	m_userFCircqSizeTweakMultiplier =   1;      //   { 1, 2, 3 }
}




											////////////////////////////////////////

SoundHelper::~SoundHelper()
{

	CString   retErrorMesg;


	if(   m_midiSequencer  !=  NULL   )
	{
		if(     ! m_midiSequencer->Close_Midi_Device( retErrorMesg   )      )
			AfxMessageBox(  retErrorMesg  );

		delete  m_midiSequencer;
		m_midiSequencer =  NULL;
	}
}


										////////////////////////////////////////


bool    SoundHelper::App_Has_Navigators_Delay()
{

	short   appCode  =     Get_PitchScope_App_Code_GLB();     //   0:  Player    1:  Navigator    2: VoxSep   3:  PitchScope    (  4: PitchScope 2.0  ??  )


	if(      appCode  ==   1   )    //   1:  Navigator   
		return   true;
	else
		return   false;
}


										////////////////////////////////////////


SPitchCalc*    SoundHelper::Get_SPitchCalc()
{
			                             
	if(   m_audioPlayer  ==   NULL  )      //  return NULL if a Song is NOT loaded
	{
		return  NULL;
	}

	SPitchCalc   *sPitchCalcer  =     m_audioPlayer->Get_SPitchCalc();
	return           sPitchCalcer;
}


										////////////////////////////////////////


bool   SoundHelper::Is_WAV_Loaded() 
{

	if(    ! Get_UniApp().SRC_File_Is_Loaded()     )
		return  false;  


	if(   m_audioPlayer  ==   NULL  )     //   do another test
		return  false;

	return  true;
}


											////////////////////////////////////////


bool	SoundHelper::Is_WAVplayer_Playing()
{

	if(    m_audioPlayer  ==  NULL   )
		return  false;

	return    m_audioPlayer->Is_Playing();
}



					////////////////////////////////////////////////////////


long    SoundHelper::Get_AudioPlayers_Last_Played_SampleIdx()
{

		//   This is also UPDATED by    PostRoll(),   and   PreRoll_NoteDetect()


	long     retSampleIdx =    m_audioPlayer->Get_LastPlayed_SampleIdx(); 	 //  Not likely playing without ANY view, but is allowed.
	return  retSampleIdx;
}


// **************   BOTH functions are same.  CLEAN this up   12/11


long    SoundHelper::Get_ActiveViewjs_FirstSample_Index()
{

ASSERT( 0 );   //     8/23/2012

	long     retSampleIdx =    m_audioPlayer->Get_LastPlayed_SampleIdx(); 	 //  Not likely playing without ANY view, but is allowed.
	return  retSampleIdx;
}



											////////////////////////////////////////


long    SoundHelper::Calc_Biggest_VirtSample_Minus_DrivingVw_Lag()
{


	 bool   useBoundaryRounding  =   true;    //  false     *********EXPERIMENTING with true.    3/1/12  ************************


	    /***  CALLED BY:    
			
		PsNavigatorDlg::Calc_VirtSampleIdx_from_FileSliders_Position()

		PsNavigatorDlg::On_GoFileEnd_Button()

		PsNavigatorDlg::Sync_FileSlider_Control(   long   panesSampleCount,   long  firstSampleIdx   )
		***/


		//   The full range of VirtualSamples, acessible from the File-Slider,  must be less than total file size
		//   because of the Future Lag  calcs that must be done to fill the pipeline.   11/11


	if(    m_audioPlayer  ==  NULL   )
	{
		ASSERT( 0 );
		return  -2;
	}
	
	SPitchCalc   *sPitchCalcer   =    Get_SPitchCalc();
	if(                sPitchCalcer  ==  NULL   )
	{  return  -3;
	}



	CString   retErrorMesg;
	long   retBigVirtSample =  -1;



	long         totalSamples   =     m_audioPlayer->Get_Biggest_SampleIndex_No_SpeedExpansion();

	ASSERT(   totalSamples  >= 0    );   //   if we have overflow of a long,  then value will be negative   3/12
														 //                  Crash here with Stanislaw Pruszynski  assinaltion tape at  Speed 8 .  ( 35 minutes long )



	long   earlySamplesCount  =   sPitchCalcer->Approximate_Sample_Count_In_FinalCircQue_with_BoundaryRounding(    sPitchCalcer->m_playSpeedFlt,   
																									                                      useBoundaryRounding,   m_audioPlayer->m_bytesPerSample   );


	retBigVirtSample   =     totalSamples   -   earlySamplesCount;

	return   retBigVirtSample;
}




											////////////////////////////////////////
											////////////////////////////////////////
											////////////////////////////////////////


bool		SoundHelper::List_All_Midi_Devices(   short&  retDeviceCount,    CString&  retErrorMesg    )
{

			//  Wiil re-build the list each time this is called, in case the user just plugged in a NEW  Midi Device


	retDeviceCount =  -1;
	retErrorMesg.Empty();

	m_deviceIDofInternalSynth =  -1;      //  init for fail
	CString  mesg,  composString;


	int   numDevs    =         midiOutGetNumDevs();   //      retrieves the number of MIDI output devices present in the system. 
	if(    numDevs  ==  0   )
	{
		 retErrorMesg   =  "No Midi Devices are currently available on this computer. Midi playback of detected notes will NOT be possible. (List_All_Midi_Devices)" ;
		 return  false;
	}


	if(    m_midDeviceList.Count()  > 0    )
		m_midDeviceList.Empty();




/********
typedef struct tagMIDIOUTCAPSA 
{
    WORD    wMid;                  // manufacturer ID
    WORD    wPid;                  // product ID 
    MMVERSION vDriverVersion;      // version of the driver 
    CHAR    szPname[ MAXPNAMELEN ];  // product name (NULL terminated string) 
    WORD    wTechnology;                // type of device 

    WORD    wVoices;               // # of voices              (internal synth only) 
    WORD    wNotes;                // max # of notes        (internal synth only) 
    WORD    wChannelMask;          // channels used     (internal synth only) 

    DWORD   dwSupport;             // functionality supported by driver 

} MIDIOUTCAPSA,        *PMIDIOUTCAPSA, *NPMIDIOUTCAPSA, *LPMIDIOUTCAPSA;



wTechnology           // type of device     flags for wTechnology field of MIDIOUTCAPS structure 

#define MOD_MIDIPORT    1   output port 
#define MOD_SYNTH       2   generic internal synth 
#define MOD_SQSYNTH     3   square wave internal synth
#define MOD_FMSYNTH     4   FM internal synth 
#define MOD_MAPPER      5   MIDI mapper 
#define MOD_WAVETABLE   6   hardware wavetable synth 
#define MOD_SWSYNTH     7   software synth 

*********/
	MIDIOUTCAPS    moutCaps;



	for(  int  idx = 0;     idx < numDevs;    idx++  )
    {


        if(     midiOutGetDevCaps(   idx,   &moutCaps,    sizeof(moutCaps)  )       ==   MMSYSERR_NOERROR   )
        {

			CString   technologyText;
			bool       techPickOfInternalSynth =    false;


			switch(   moutCaps.wTechnology    )
			{
				case       MOD_MIDIPORT:     //	 1   output port      **** Hammond Organ when through the     USB Devide *****    1/2012
						technologyText  =    "output port " ;
					break;

				case       MOD_SYNTH:	   //	 2   generic internal synth 
						technologyText  =    "generic internal synth" ;
					break;

				case       MOD_SQSYNTH:    //	   3   square wave internal synth
						technologyText  =    "square wave internal synth" ;
					break;

				case       MOD_FMSYNTH:    //	 4   FM internal synth 
						technologyText  =    "FM internal synth " ;
					break;

				case       MOD_MAPPER:    //	 5   MIDI mapper 
						technologyText  =    "MIDI Mapper" ;
					break;

				case       MOD_WAVETABLE:    //	  6   hardware wavetable synth 
						technologyText  =    "hardware wavetable synth" ;
					break;

				case       MOD_SWSYNTH:    //	  7   software synth 
						technologyText  =    "software synth" ;

						techPickOfInternalSynth =    true;   //   **** this is what I get on Fido and Sparky.  Tyr Lassie.   ****   1/2012
					break;

				 default:              
					 break;    
			}


			bool   namePickOfInternalSynth =  false;
			CString  lowerCase  =     moutCaps.szPname;    // ******** CAREFUL,  I'm making a big assumption *********
			lowerCase.MakeLower(); 

			int   loc  =    lowerCase.Find(  "microsoft"   );      //      "Microsoft GS Wavetable Synth"    ******  What ELSE could I CHECK for ???  1/2012
			if(    loc  >= 0    )
				namePickOfInternalSynth =  true;



			if(     techPickOfInternalSynth   &&   namePickOfInternalSynth  )		 //  Try to find a consensus		
				m_deviceIDofInternalSynth =   idx;
			else if(       (      techPickOfInternalSynth    &&    ! namePickOfInternalSynth     ) 
				        ||   (   ! techPickOfInternalSynth    &&      namePickOfInternalSynth     )     )
			{
				mesg.Format(   "Having difficulty locating the Internal Synthesizer. Going to use  %s",   moutCaps.szPname   );
				AfxMessageBox(  mesg  );

				m_deviceIDofInternalSynth =   idx;
			}
			else {  }


			if(    technologyText.IsEmpty()   )
				composString.Format(  "%s" ,         moutCaps.szPname  );
			else
				composString.Format(  "%s (%s)" ,  moutCaps.szPname,   technologyText   );





			IndexedString   *nuLongIndexed     =         new   IndexedString();
			if(                     nuLongIndexed ==  NULL  )
			{  retErrorMesg =   "List_All_Midi_Devices  FAILED, could NOT alloc  nuLongIndexed." ;
				return  false;
			}

				nuLongIndexed->index   =    idx;

				nuLongIndexed->value0  =   moutCaps.wTechnology;     //  Can I use this ???

				nuLongIndexed->value1  =   moutCaps.wMid;        //  Can I use this ???

				nuLongIndexed->cString =   composString;  




			m_midDeviceList.Add_Tail(  *nuLongIndexed  );  


			TRACE(   "Midi Device[ %d  ]     Name[  %s  ]    TypeOfDevice[ %s,  %d ]     ManufacturerID[ %d ]    ProductID[ %d ]     \n",   
				                       idx,   moutCaps.szPname,   technologyText,   moutCaps.wTechnology,   moutCaps.wMid,    moutCaps.wPid     );
        }
		else
		{	ASSERT( 0 );  // have an error to deal with
		}
	}


	retDeviceCount =   numDevs; 

	return  true;
}


										////////////////////////////////////////


bool    SoundHelper::Alloc_Midi_Sequencer(   long  deviceID,    SequencerMidi  **retNewSequencerMidi,    CString&  retErrorMesg   )
{

		//  Now can be called multiple times.   1/12


	retErrorMesg.Empty();

	SequencerMidi   *nuSequencerMidi =   NULL;


	/****   Moved to calling functions

	if(   m_midiSequencer  !=  NULL   )
	{
		delete  m_midiSequencer;
		m_midiSequencer =  NULL;
	}
	***/

	
	nuSequencerMidi      =     new   SequencerMidi();
	if(  nuSequencerMidi  ==  NULL   )
	{
		retErrorMesg =   "Alloc_Midi_Sequencer failed, could not alloc SequencerMidi. No midi playback will be available."  ; 
		return  false;
	}



	if(             m_deviceIDofInternalSynth  >=  0
		     &&   m_deviceIDofInternalSynth  ==  deviceID   )
	{
		nuSequencerMidi->m_isTheInternalSynth =   true;           // *** BIG,  lets the Internal Synth identify itself
	}



	if(    ! nuSequencerMidi->Open_Midi_Device(   deviceID,    retErrorMesg   )    )
	{
		delete   nuSequencerMidi;      
		nuSequencerMidi =  NULL;    // Get rid of it,  it is useless

		return  false;
	}


	*retNewSequencerMidi  =    nuSequencerMidi;

	return  true;
}


											////////////////////////////////////////


bool	  SoundHelper::Assign_and_Open_MidiDevice(    long  newDevicesID,   CString&  retErrorMesg   )
{


	short    channelID =  0;   // ***** ALWAYS ???   1/12


	if(    newDevicesID  <  0   )
	{
		retErrorMesg  =    "SoundHelper::Assign_and_Open_MidiDevice FAILED,  bad  newDevicesID. " ;
		return  false;
	}


	bool   isTheInternalSynth =   false;

	if(             m_deviceIDofInternalSynth  >=  0                               //  *********  Can I always count on this ???   *******************
		     &&   newDevicesID  ==   m_deviceIDofInternalSynth   )
		isTheInternalSynth =   true;    




	if(    m_midiSequencer   !=  NULL   )
	{

		/*****************************   DISABLE forever.   This might be the best,  force it to { Close,  ReAllocate,  and Open }.    1/10/2012 

		if(   m_midiSequencer->m_deviceID  ==  newDevicesID  )
		{
			retErrorMesg =  "That Midi Device is already in use.  [Assign_and_Open_MidiDevice]" ;
			return  false;
		}
		***********/

		if(    ! m_midiSequencer->Close_Midi_Device(  retErrorMesg   )    )   //  If it is closed,  then another SequencerMidi OBJECT can access it (see below ) 
		{
			m_midiSequencer->m_silenceMidi =  false;	
			return  false;
		}
	}




	SequencerMidi   *retNewSequencerMidi =  NULL;    //  The DEFAULT device is the internal Microsoft synth ("Microsoft GS Wavetable Synth") which should always be allocated first 

	CString   mesg1,   retErrorMesgLoc1,   retErrorMesgLoc2; 



	if(    ! Alloc_Midi_Sequencer(   newDevicesID,   &retNewSequencerMidi,    retErrorMesgLoc1  )     )
	{

		if(            m_midiSequencer  !=  NULL     //  We FAILED to allocate the NEW one,  so try and RE-Open the PREVIOUS one (We closed it just above )
				&&   m_midiSequencer->m_midiIsOpen ==  false   )			
		{

			if(     ! m_midiSequencer->Open_Midi_Device(   m_midiSequencer->m_deviceID,    retErrorMesgLoc2  )     )	 // reopen it, cause we just closed it.
			{				
				AfxMessageBox(  retErrorMesgLoc2  );
			}
			else
				retErrorMesg  =   retErrorMesgLoc1;

		}
		else
			retErrorMesg  =   retErrorMesgLoc1;

		return  false;    
	}



	if(   m_midiSequencer  !=  NULL   )    //   DEALLOCATE the previous device,  we now have a new one.
	{
		//    m_midiSequencer->Close_Midi_Device( retErrorMesg   )     **** We already did this above.  

		delete  m_midiSequencer;
		m_midiSequencer =  NULL;
	}
		

	m_midiSequencer =    retNewSequencerMidi;     //  now assign it to the 'memberVar'  for play-back




	if(    ! m_midiSequencer->Initialize_New_Midi_Device(  channelID,   mesg1  )     )  
	{
//	    return  false;                    // This is NOT a big deal if it if it fails.  So we still allow this function to return true    1/12
		AfxMessageBox(   mesg1  );
	}



	if(        isTheInternalSynth  
		&&   m_midiSequencer  !=  NULL   )
	{
		if(    ! m_midiSequencer->Change_Instrument(   m_midiInstrumentPatch,    retErrorMesgLoc2   )      )   
		{
			AfxMessageBox(  retErrorMesgLoc2  );        //   return  false;    ** NOT important enough to return false   1/12
		}
	}
	
	return  true;
}



										////////////////////////////////////////


bool		SoundHelper::Change_Midi_Instrument(   short   midiInstrumentPatch,    CString&  retErrorMesg   )
{

	retErrorMesg.Empty();


	if(  m_midiSequencer ==  NULL  )
	{
		retErrorMesg =  "SoundHelper::Change_Midi_Instrument  FAILED, m_midiSequencer is NULL. " ;  
		return  false;
	}


	if(    ! m_midiSequencer->Change_Instrument(  midiInstrumentPatch,   retErrorMesg )     )
		return  false;
	else
		return  true;
}


											////////////////////////////////////////


bool	SoundHelper::Stop_MidiPlay(   CString&  retErrorMesg    )
{

	retErrorMesg.Empty();


	if(   m_midiSequencer  ==  NULL   )
	{
		//  retErrorMesg  =  "SoundMan::MidiPlay_DSTlist  failed,  no  midiSequencer." ; 
		//  return  false;
		return  true;
	}


	if(     ! m_midiSequencer->Stop_Play_curNote(  retErrorMesg   )      )   //  stop any sounds at end
		return  false;
	
//	if(     !Analyze_Tap_Events(  retErrorMesg   )      )   //  stop any sounds at end
//		return  false;

	return  true;
}



											////////////////////////////////////////


bool   SoundHelper::Play_Midi_Pitch(    short  pitchMidi,    short   volume,    short  auditionCode,     CString&  retErrorMesg   )
{

	retErrorMesg.Empty();


	if(   m_midiSequencer  ==  NULL  )     
	{
		ASSERT( 0 );

		retErrorMesg =   "Play_Midi_Pitch  failed,   m_midiSequencer  is NULL." ;
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
		retErrorMesg =  "Play_Midi_Pitch  failed,   incorrect input params." ;
		return  false;
	}



	m_midiSequencer->m_tickCount++;




	if(   m_midiSequencer->m_curPitch    !=    pitchMidi    )   //  CurrentPlaying pitch is  not same  as INTENDED from animationMask
	{																		        //   ...OR  nothing  is playing,  but we need to turn a note on to match the  INTENDED 


		short   holdOldValue  =   m_midiSequencer->m_curPitch;



		if(    m_midiSequencer->m_curPitch  > 0   )     //  a note is still playing,  but it is NOT our INTENDED pitch
		{
			if(     ! m_midiSequencer->Stop_Play_curNote(   retErrorMesg   )    )
				return  false;


		//	TRACE( "Stop[%d, %d,    %d]   ",   holdOldValue,   m_midiSequencer->m_tickCount,   curSample  );


			m_midiSequencer->m_tickCount =  0;
		}




		if(                  pitchMidi  > 0											 	             //  We need a note playing but the currentPlaying pitch is not it
			     &&     m_midiSequencer->m_curPitch   !=   pitchMidi        //  m_midiSequencer->m_curPitch will equal -1 after  Stop_Play_curNote()   ****  OMIT ??? ***************
		  )
		{  

			 bool  overideToOrigVolume =  false;  


			if(     auditionCode   ==    AnimeStream::JUSTMIDI   )
			   overideToOrigVolume =  true;  



			if(    ! m_midiSequencer->Start_Play_curNote(   pitchMidi,   volume,   overideToOrigVolume,   retErrorMesg  )    )
				return  false;


		//	TRACE( "Start[%d,  %d,         %d]   \n",    pitchMidi,   m_midiSequencer->m_tickCount,  curSample    );

			m_midiSequencer->m_tickCount =  0;
		}
		else
		{	   TRACE( "\n  pitchMidi  <= 0  \n",   pitchMidi  );  }		

	}  

	return  true;
}



											////////////////////////////////////////
											////////////////////////////////////////


bool    SoundHelper::Allocate_AudioPlayer(   UniApplication&  theApp,   short  *stereoChannelCodeAddr,    long *lastFilePositionAddr,    CString&  retErrorMesg    )
{

									              //  Only gets CALLED  by  PitchPlayerApp::Initialize_BitSource()
	retErrorMesg.Empty();

						                  //   Alloc_AudioPlayer()    will alloc the DSound-Buffer,  AudioPlayer,   register it with Eventman,  and install the EventNotify


	if(     ! Alloc_AudioPlayer(    AudioPlayer::STREAMINGBUFR,   theApp,   stereoChannelCodeAddr,  lastFilePositionAddr,     retErrorMesg    )    )    
		return  false;
	else
		return  true;
}


											////////////////////////////////////////


void    SoundHelper::Release_AudioPlayer()
{

	if(    m_audioPlayer  ==  NULL    )
		return;			// OK,  sometime called beforthe first is allocated


	if(      m_audioPlayer  ==   Get_EventMan().m_curAudioPlayer   )
		Get_EventMan().UNregisterAudioPlayer();     // Set it the NEW active window's  buffer ???? ***** JPM  


	delete   m_audioPlayer;       
	m_audioPlayer =  NULL;		
}



											////////////////////////////////////////


bool    SoundHelper::Alloc_AudioPlayer(   int  typePlayer,   UniApplication&  theApp,   short  *stereoChannelCodeAddr,   long *lastFilePositionAddr,    CString&  retErrorMesg    )
{

			               //    was/is   in SoundMan  in VoxSep


	bool  doRealtimePitchDetection =   true;   //   *******  SWITCH   12/09  **********



	retErrorMesg.Empty();

	ASSERT(  stereoChannelCodeAddr  );
	ASSERT(  lastFilePositionAddr  );
	
	BitSourceStreaming  *bitSource =    NULL;   



	Release_AudioPlayer();    



	switch(   typePlayer   )
	{		
					
	    case  AudioPlayer::STATICBUFR:    
			ASSERT( 0 );
			/***
			retErrorMesg =  "SoundMan::Alloc_AudioPlayer  failed,   Static Bitsource no longer used."  ;   //  5/07
			return  false;  
		
			bitSource         =      Get_BitSource_Static();
			m_audioPlayer =     new    AudioStaticPlayer(   *m_waveMan,     Get_BitSource_Static()    );      //  ALLOC DSound buffer object	
			****/
			m_audioPlayer =  NULL;
			retErrorMesg =  "SoundMan::Alloc_AudioPlayer  failed,   Static Bitsource no longer used."  ;   //  5/07
			return  false;  
			
		break;


	    case  AudioPlayer::STREAMINGBUFR:   
			bitSource  =      Get_BitSource_Streaming();
			ASSERT(  bitSource  );
			m_audioPlayer =     new    StreamingAudioplayer(    Get_EventMan(),    bitSource,   theApp,   doRealtimePitchDetection    );    
		break;


		default:     
			retErrorMesg =   "SoundMan::Alloc_AudioPlayer  failed,   unknown player code."  ;
			return  false;
		break;
	}
		

	if(    m_audioPlayer  ==  NULL  )
	{   
		retErrorMesg =   "SoundMan::Alloc_AudioPlayer  failed,   player is NULL."  ;
		return  false;   
	}



	Get_EventMan().RegisterAudioPlayer(   m_audioPlayer  );   //  just assigns one variable
 


																				//  INITIALIZE  the new Player's vars
	/***      
	m_audioPlayer->Set_Anime_Maps(      Get_PitchDetectorApp().m_leftChannelDetectSubj.m_animeAdmin.Get_MidiPitch_Map_Addr(),      
														    Get_PitchDetectorApp().m_rightChannelDetectSubj.m_animeAdmin.Get_MidiPitch_Map_Addr(),
															Get_PitchDetectorApp().m_centerChannelDetectSubj.m_animeAdmin.Get_MidiPitch_Map_Addr()	);      
	****/


	m_audioPlayer->Set_Focus_StereoChannel_Addr(   stereoChannelCodeAddr  );         //  Get_StereoChannelCode_Address()    ); 
	


	/****
	Editorj  *ed     =           Get_UniEditorApp().Get_Cur_Editor();
	if(          ed ==  NULL )
	{
		retErrorMesg =  "SoundMan::Alloc_AudioPlayer  failed,   Editor is null."  ;
		return  false;  
	}
	else
		m_audioPlayer->Attach_LastPlayed_SampleIdx(     &(   ed->m_lastFilePosition  )     );   //  hold the ADDRESS of this var in UniBasic so we can directly update 
	****/

		m_audioPlayer->Attach_LastPlayed_SampleIdx(   lastFilePositionAddr  );     //  hold the ADDRESS of this var in UniBasic so we can directly update 





								//   ALLOC and INITIALIZE   the   WaveFortax  struct,  and prepare file if going to stream 


	if(   ! m_audioPlayer->ExamineWavFile(  bitSource->m_strWavFilePath,   retErrorMesg  )     )   //  this sets up the DSound PlayBuffer, and gets WAVFORMATEX info							 ONLY place CALLED !!!
	{	                             

		Get_EventMan().UNregisterAudioPlayer();     // The app
		
		Release_AudioPlayer();

		bitSource->m_strWavFilePath.Empty();    

	//	retErrorMesg =  "SoundMan::Alloc_AudioPlayer  failed,   ExamineWavFile  failed."  ;   //  could be a bad/unknown file name
	//    ***********   WANT a compound error message ????  **********************

		return  false;
	}


	return   true;
}







											////////////////////////////////////////
											///////////  TRANSPORT  ////////////
											////////////////////////////////////////



bool    SoundHelper::Continue_Play_Forward(   double  speed,   bool  justSpeedChange,   long  pauseSampleIdx,   CString&  retErrorMesg    )
{
 
						                                             //   'pauseSampleIdx'   is rarely USED,   
	retErrorMesg.Empty();

	if(    ! Is_WAV_Loaded()    )
	{
		retErrorMesg =   "Load a sound file." ;
		return  false;
	}

	ASSERT( m_audioPlayer );    //  if a WAV is loaded,  then this should not a be problem    12/09


	BitSourceStreaming   *bitSource =    m_audioPlayer->m_bitSource;  
	ASSERT(  bitSource  );  




	bool  preventFileSeek =   false;     

	if(     m_lastTransportDirection  ==   CONTINUEfORWARD    )     //  then do NOT need to do a File-Seek    1/12
		preventFileSeek =   true;  




										//   CALC  the   'startSampleIdx'   for  resumed-PLAY  (  value gets refreshed by a move of the file slider. )

//	long   lastPlayedSample  =    Get_ActiveViewjs_FirstSample_Index();   //   Lousey Function NAME  ************ value is in     PitchPlayerApp::m_lastFilePosition
	long   lastPlayedSample  =    Get_AudioPlayers_Last_Played_SampleIdx();    // should be same value as  PitchPlayerApp::m_lastFilePosition   12/14/11


	long   startSample  =  -2,   endSample =  -2;





	if(    lastPlayedSample   < 0    )      //  == -1 :  Happens when play was stopped cause LoadNextSampleChunk() hit the end of file;
	{

		m_audioPlayer->Set_LastPlayed_SampleIdx(  0  );

		startSample  =   0; 
	}
	else
	{  				
		if(        justSpeedChange															 //  ***  RARE,  need to step-TEST this.   12/11
			&&   (         m_audioPlayer->m_playMode ==  AudioPlayer::RETURNtoPLAYSTART   
				       ||    m_audioPlayer->m_playMode ==  AudioPlayer::LOOPpLAYsELCT    //  4/07  Want this??  Seems to be working OK.

			//			||    m_audioPlayer->m_playMode ==  AudioPlayer::PLAYwINDOW    ************  ??????????   WANT,  NEED this too ???    5/12/2012 *************** 
				   )			
		  )
		{				
		    startSample  =     pauseSampleIdx;   //  want to continue playing where we did the pause for speed change


			//  ******  TEST, this might be BUGGY ,  ( BUT does it ever get called ???  *****       12/14/11


			//  ****** Want this ????   WRONG???*******  We should want to keep the Circular Data if just a pause.  11/11  *****************************
			//   *********   BUT this rarely gets called.  It doesn NOT even get called during a Speed Change  2/12

			if(     ! Initialize_Delay_Buffers_and_Two_CircQues(  retErrorMesg  )    )    
				return  false;


		}
		else                        //   NORMAL          
		{
			if(   preventFileSeek   )
			{

				if(    lastPlayedSample  !=   pauseSampleIdx  )   // Do all calling function submit 'lastPlayedSample'   ???     12/14/2011
				{
					ASSERT( 0 );   //  Does this get hit?    12/14/11        YES,  did gets hit in Player when jiggle the FilePos Slider. 1/12
				//	int  dummy =  9;                  //								BUT think that since I now STOP play when the FileSlider is released, think this will not occur.  2/1/2012
				}

											
			//	startSample =    pauseSampleIdx;    	// ***  'lastPlayedSample'  is BETTER??   BUT all calling function submit 'lastPlayedSample'    12/11          
				startSample =    lastPlayedSample;  
			}
			else
				startSample =    lastPlayedSample;   //   get here when move the SLIDER,  get here on file start,  Backwards-Play,    											
		}														          
	}
																											

//	endSample =     m_audioPlayer->Get_Biggest_SampleIndex();        ***BAD,  has the slowDown expansion in it
	endSample =     m_audioPlayer->Get_Biggest_SampleIndex_No_SpeedExpansion();   




	short   playMode;
	bool    playBackwards =     false;  

	/***
	if(   justSpeedChange   )
		playMode  =    m_audioPlayer->m_playMode;
	else
	    playMode  =     AudioPlayer::NORMALpLAY;   
	****/
		playMode  =     AudioPlayer::NORMALpLAY;   



//	if(    ! Sync_AudioPlayers_Vars(  retErrorMesg   )     )   
//		return  false;

//	Init_Tap_Count(    startSample    );    //  in case the user set the FLAG for this in   SPitchListWindow::OnCalcBeat_shSPL() 

//	Get_UniEditorApp().Set_Anime_Range_Mode_AllPanes(    FrameArtist::SCROLLINGfRAMES  /*,    true*/    );   



	m_audioPlayer->m_wasPlayingBackwardsPrev =  false;    //    OK to init HERE     3/11  



	if(    ! m_audioPlayer->StartPlaying(   playMode,   speed,    playBackwards,    startSample,   endSample,   justSpeedChange,  preventFileSeek,  retErrorMesg  )    )
		return  false;



	m_lastTransportActionActual =    SoundHelper::CONTINUEfORWARD;

	return  true;
}




					////////////////////////////////////////////////////////


bool    SoundHelper::Continue_Play_Backwards(    double  speed,   bool  justSpeedChange,    long  pauseSampleIdx,    CString&  retErrorMesg    )
{

													                //  if   ( ! justSpeedChange  )   then  'pauseSampleIdx'   is ignored
		                                          
	UniApplication&   uniApp =    Get_UniApp();

	retErrorMesg.Empty();


	if(    ! Is_WAV_Loaded()    )
	{
		retErrorMesg =    "Load a sound file." ;
		return  false;
	}

	BitSourceStreaming  *bitSource  =		Get_BitSource_Streaming();
	ASSERT(                   bitSource   );




	bool  preventFileSeek =   false;    

	if(     m_lastTransportDirection  ==   CONTINUEbACKWARDS    )      //   NOT always need to do a File-Seek .   1/12
		preventFileSeek =   true;  




	m_audioPlayer->Mute_Audio_for_Backwards_Play(   m_muteReversePlayAudio   );   


														
	long   startSample  =  -2,   endSample =  -2;

	long   lastPlayedSample  =      m_audioPlayer->Get_LastPlayed_SampleIdx();    //   CALC  the   'startSampleIdx'   for  resumed-PLAY





	if(    lastPlayedSample  <  0    )    //  == -1 :  Happens when play was stopped cause LoadNextSampleChunk() hit the end of file
	{

		endSample  =  0;      //   Set to FRONT of file,   that is our boundary
		
		startSample =      m_audioPlayer->Get_Biggest_SampleIndex_No_SpeedExpansion();   


		m_audioPlayer->Set_LastPlayed_SampleIdx(  0  );


	//	uniApp.Update_FirstSampleIdx_Active_RankingViewj(   lastPlayedSample   );   
		uniApp.Animate_All_Viewjs(  lastPlayedSample,  -1  );    //  better
	}
	else
	{  
		 endSample  =   0;    

		if(          justSpeedChange   
			  &&   m_audioPlayer->m_playMode ==  AudioPlayer::RETURNtoPLAYSTART   )
		{

			ASSERT( 0 );   //  Is this installed for  Player/Navigator ????    3/11

			startSample  =     pauseSampleIdx;   //  want to continue playing where we did the pause for speed change
		}
		else
			startSample  =     lastPlayedSample;
	}
		



	short   playMode;
	bool    playBackwards =    true;  

	if(   justSpeedChange   )
		playMode =    m_audioPlayer->m_playMode;
	else
	    playMode =    AudioPlayer::NORMALpLAY;   



	if(    ! m_audioPlayer->StartPlaying(   playMode,   speed,    playBackwards,    startSample,   endSample,  justSpeedChange,  preventFileSeek,  retErrorMesg  )    )
		return  false;



	m_lastTransportActionActual =    SoundHelper::CONTINUEbACKWARDS;

	return  true;
}



					////////////////////////////////////////////////////////


bool    SoundHelper::Pause_Play(   CString&  retErrorMesg   )
{


	EventMan&                 eventMan  =   Get_EventMan();
	BitSourceStreaming   *bitSourceStreaming  =		Get_BitSource_Streaming(); 

	UniApplication&           uniApp =    Get_UniApp();


	retErrorMesg.Empty();


	if(    m_audioPlayer  ==  NULL   )
	{
		retErrorMesg  =    "Load a Music File ( .MP3  or .WAV )." ;
		return  false;   //  If there is no song loaded,  the there is nothing for this funtion to do
	}


	if(    ! m_audioPlayer->Is_Playing()    )
		return  true;


	ASSERT(  bitSourceStreaming  );


	SPitchCalc  *sPitchCalcer      =        Get_SPitchCalc();
	if(                sPitchCalcer  ==   NULL  )
	{
		retErrorMesg =   "SoundHelper::Pause_Play FAILED,   spCalcer is NULL." ;
		return  false;
	}


	bool  playingNoteList =   false;

	if(    sPitchCalcer->m_playModeUsingNotelist  ==   1   )
		 playingNoteList =   true;




//	long   lastPlayedSampleBeforeRoll  =     m_audioPlayer->Get_LastPlayed_SampleIdx();	    // **** DEBUG only *******


	if(    ! Stop_Play(  retErrorMesg  )    )	 //  will halt midi notes as well
		return  false;




	long   lastPlayedSampleBeforeRoll  =    m_audioPlayer->Get_LastPlayed_SampleIdx();	 



	if(    bitSourceStreaming->m_recordNotesNow    )
	{
		bool  retUserDidMerge =  false;
		NoteGenerator  *noteGenerator       =            bitSourceStreaming->m_noteGenerator;
		if(                      noteGenerator  ==  NULL    )
		{  retErrorMesg =  "SoundHelper::Pause_Play  FAILED,  NoteGenerator is null." ;
			return  false;
		}

		noteGenerator->m_musicalKey =     sPitchCalcer->m_musicalKey;      //  user might have changed the MusicalKey setting during the Recording Session


		if(    ! noteGenerator->Ask_User_Merge_or_Save_New_Notes(     lastPlayedSampleBeforeRoll,      retUserDidMerge,     retErrorMesg   )    )
			return  false;


		if(    retUserDidMerge    )
			uniApp.Set_DSTlist_Modified(  true  );   
	}  

	bitSourceStreaming->m_recordNotesNow =   false;    //  OK to reset this in all cases ??? 





	bool   skipPostRoll          =  false;	
	long   lastPieClockValue   =    bitSourceStreaming->m_currentPieClockVal;    


	if(     lastPieClockValue   ==   10   )
	{
		skipPostRoll =  true;      //   YES.        we do NOT have to do a POST-Roll in this situation   8/12/2012
	}


	if(    lastPieClockValue   ==   0   )       // Think we do NOT have to do a POST-Roll in this situation   8/12/2012
	{
		ASSERT( 0 );    //     ***Last time HERE since   8/12/2012 :   
		skipPostRoll =  true;
	}





	sPitchCalcer->Initialize_All_DFTprobe_CircularQues_ArrayElements_and_Sums();   // **** TESTS GOOD.   9/4/2012:    Do NOT want any error to build up there or I am SCREWED!!!!   9/2012
		
//					When TESTING this,  remember that with the stop and ContinueButton(with Erase of CircQue sums) the EFFECT would be seen at the far
//					RIGHT of a Horizontal DrivingView( logDFT ),  where the NEW Notes are first being formed.   9/4/2012
//
//					Go to PsNavigatorDlg::Scroll_ThinColumn_to_Driving_View() and have it display the LogDFT while playing -- that would be the easiest way 
//					to see a POSSIBLE DISRUPTION by this to  LogDFT.   I have not seen such a disruption.   9/4/2012
//
//					BUT,  this USED to happen on the PLAY Button, and I was afraid it might take too long and throw off some Lost-Slices.  9/2012 






//  /********  PUT  BACK    ALLOW_logDFT_DRIVING_VIEW  ( comment this out might cause LOST PieSlices  )   ******

	if(           App_Has_Navigators_Delay()      //   ONLY for Navigator 
		&&    ! skipPostRoll     )  
	{

		if(     ! Do_Post_Roll_CalcNotes(  retErrorMesg  )     )   
			AfxMessageBox(  retErrorMesg  );
	}
//  ****/


	long   lastPlayedSampleAfterRoll  =    m_audioPlayer->Get_LastPlayed_SampleIdx();	 



 

												//   save/set  the   'Sample Index values'   for our last play position for later RePlay functs


	m_audioPlayer->m_prevEndSamplePlay  =    lastPlayedSampleAfterRoll;     


	m_curScalePitchAtStop                          =    eventMan.m_curScalePitch;                       //  Must SAVE the current vale when the user hit the PAUSE BUTTON, or other stop
	m_curScalePitchDetectionScoreAtStop    =     eventMan.m_curScalePitchDetectionScore;  // because  Process_Event_Notification_PPlayer() still get called after this funct.  1/12
	m_curScalePitchAvgHarmonicMagAtStop =     eventMan.m_curScalePitchAvgHarmonicMag;  




	uniApp.Animate_All_Viewjs(  0,   -1  );   //   Only moves the File-Slider in Player ( what TYPE of Animation object is this ?  If I dont know, then these 2 belong together  )

	uniApp.ReDraw_All_Anime_Viewjs();     //   Only render the BULLETS to the   Stationary  { Revolver and  Gagues }  Animations




// ***********  PUT BACK   ALLOW_logDFT_DRIVING_VIEW  ************************

	uniApp.ReDraw_Bitmaps(  false  );    //  false:  DRAW notes TEXT           ReDraws the DrivingView from the FinalCircQue.  Also does Post-Rendering (draw LetterNames to notes)





	CString   mesgEmpty; 
	Write_To_StatusBar_GLB(   mesgEmpty   );    //   Now a dummy function  6/2012 


	m_lastTransportActionActual =    SoundHelper::PAUSEpLAY;

	return  true;
}



					////////////////////////////////////////////////////////


bool    SoundHelper::Stop_Play(   CString&  retErrorMesg   )
{

	retErrorMesg.Empty();


	if(   ! Is_WAV_Loaded()    )
	{
		retErrorMesg =   "Load a sound file." ;
		return  false;
	}


	if(    ! Stop_MidiPlay(   retErrorMesg   )     )    	    //   Also CAN call  soundman::Analyze_Tap_Events()
	{	
		                                                                    //   Get  a 'BeatTapping' error here if too few taps.   INSTALL,  sould eralse old SRC lines


		m_audioPlayer->StopPlaying_Hardware();      //   should do this BEFORE the  below calulations   ONLY CALLED here.  1/03
		return  false;
	}
	else
	{  m_audioPlayer->StopPlaying_Hardware();   
		return  true;
	}
}	



					////////////////////////////////////////////////////////


long	  SoundHelper::Set_VolumeBoost_Factor(  short   factorCode   )
{

			//  Whenever a new song is loaded, we can count on  'm_curAudioPlayer->m_inputScalePercent'  being reset to default from this funct   11/11

		   //   This value is used in StreamingAudioplayer::Load_Next_DataFetch_Forward_PPlayer()  to regulate the amplification in the
			//  8-bit soundSample

	short  highestValue =  4;  // ******************   HARDWIRED ************************


	long   scalePercentBase =  500;     //   default,   was  400  

	long   newScalePercent =  -1;     //   default  


//	long   step =      200;   //   up by 200
//	long   step =      300;   
//	long   step =      700;     //   ???????
//	long   step =      1000;     *** tooo crazy
	long   step =      500;      //  OK    ...tests pretty well  



     BitSourceStreaming    *bitSource =     Get_BitSource_Streaming();   

	if(    bitSource  ==   NULL   )
	{
		m_inputScaleingFactor =   0;     //  0:  default

		return  scalePercentBase;   //   default
	}



	if(          factorCode   <  0  )  
	{

		ASSERT( 0 );

		m_inputScaleingFactor =   0;

		bitSource->m_inputScalePercent  =     scalePercentBase; 
		
		return    scalePercentBase;     //     400;   //   default
	}
	else if(    factorCode   >  highestValue  )    //   What is too high ????
	{

		ASSERT( 0 );

		m_inputScaleingFactor =   highestValue;

		newScalePercent  =    scalePercentBase    +      (    (long)m_inputScaleingFactor  *  step    ); 

		bitSource->m_inputScalePercent  =      newScalePercent; 
		
		return    newScalePercent;     //     400;   //   default
	}





	m_inputScaleingFactor =   factorCode;   //  do I really need this ????  


	newScalePercent  =    scalePercentBase    +      (    (long)factorCode  *  step    ); 


	bitSource->m_inputScalePercent  =     newScalePercent; 



//	TRACE(  "\n  Source Boost   (  Input Scaling -    [ %d  ]    \n",    newScalePercent  );   
		
	return    newScalePercent;     
}



					////////////////////////////////////////////////////////


bool   SoundHelper::Execute_FreqFilter_Change(   short  nuFilterCode   )
{


	StreamingAudioplayer  *audioPlayer     =   m_audioPlayer;  
	if(                                audioPlayer  ==  NULL   )
	{  return  false;   }


	BitSourceStreaming  *bitSource  =	  Get_BitSource_Streaming();   


	SPitchCalc  *sPitchCalcer   =      Get_SPitchCalc();    // if thger is an audio player
	ASSERT(      sPitchCalcer  );



	sPitchCalcer->Set_Bottom_Frequency_Cutoff_DFTrows(  0  );     //  0:  DEFAULT,  do not skip any rows...   use  Midi 52
	



	long   newHighFilter =   19500;    // ****  [ ALWAYS ??   11/11 ]      *** ADJUST here *********************

	long   newLowFilter =   -1;



	if(         nuFilterCode ==  0  )     //  0:  FULL   ( no filtering to speak of )
	{
		newLowFilter =         20;     
	}
	else if(   nuFilterCode ==  1  )    //  1:  Middle
	{
		newLowFilter =        300;		
	}
	else if(   nuFilterCode ==  2  )    //  2:  High
	{
		newLowFilter =       1200;		 


		long    bottomDFTrowsToSkip =   12;    //    to   Midi 64,    an octave higher than Midi 52


		sPitchCalcer->Set_Bottom_Frequency_Cutoff_DFTrows(   bottomDFTrowsToSkip  );


		sPitchCalcer->Erase_logDFTmap_and_HarmPairsMap();  // *** NEW, since it will NOT write to bottom ROWS of map,  clear out any previous values.   7/2012
	}
	else
	{	ASSERT( 0 );  }



	if(   bitSource  !=  NULL   )
	{
		bitSource->m_topFreqFilterLimit       =    newHighFilter;

		bitSource->m_bottomFreqFilterLimit  =    newLowFilter;
	}
	else
	{	ASSERT( 0 );  }


	return   true;
}



					////////////////////////////////////////////////////////


bool    SoundHelper::Initialize_Delay_Buffers_and_Two_CircQues(   CString&  retErrorMesg    )
{


		//   ERASES  the contents of the   DelayBuffer,   the two Circular Ques,   and the  DrivingView

		//  *** WORKS for both  PLAYER  and NAVIGATOR   11/11  ***


	if(   ! Is_WAV_Loaded()    )
	{
		retErrorMesg =   "Load a sound file [ Initialize_Delay_Buffers_and_Two_CircQues ]." ;
		return  false;
	}


	ASSERT( m_audioPlayer );    //  if a WAV is loaded,  then this should not a be problem    12/09



	SPitchCalc  *spCalcer      =        Get_SPitchCalc();
	if(                spCalcer  ==   NULL  )
	{
		retErrorMesg =   "SoundHelper::Initialize_Delay_Buffers_and_Two_CircQues FAILED,   spCalcer is NULL." ;
		return  false;
	}


	BitSourceStreaming   *bitSource  =    Get_BitSource_Streaming(); 
	ASSERT(                    bitSource  );



	spCalcer->Erase_CircularQues_and_DrivingBitmap();



	spCalcer->Erase_logDFTmap_and_HarmPairsMap();   //  ***** NEW,   7/2012    Will this help problems with accuract of NoteDetect after a PreRoll ???? 



	bitSource->Erase_FFTSlowDowns_Buffers();    //    *******    NEW,  8/30       OK ???   




	m_audioPlayer->Fill_Buffer_With_Silence();     //    2/11,   this will also ERASE the  'WAV CircularQue'  for our delay of sound in Navigator.

	return  true;
}



					////////////////////////////////////////////////////////


void	 SoundHelper::Erase_Players_AudioDelay_MemoryBuffer()
{

						//   PLAYER.exe has its own secondary  SMALL WAV-delay


	BitSourceStreaming   *bitSource =	  Get_BitSource_Streaming(); 
	if(   bitSource  ==  NULL  )
	{
		ASSERT( 0 );
		return;
	}

	bitSource->Erase_Players_AudioDelay_MemoryBuffer();
}



					////////////////////////////////////////////////////////


bool	  SoundHelper::ReAllocate_Players_AudioDelay_MemoryBuffer(   long  numberNeededToMatch  )
{

							//   PLAYER.exe has its own  SMALL  WAV-delay     

	BitSourceStreaming   *bitSource =	  Get_BitSource_Streaming(); 
	if(   bitSource  ==  NULL  )
	{
		ASSERT( 0 );
		return  false;
	}


	bool     sucess =    bitSource->ReAllocate_Players_AudioDelay_MemoryBuffer(   numberNeededToMatch  );
	return  sucess; 
}




					////////////////////////////////////////////////////////


bool    SoundHelper::Do_Post_Roll_CalcNotes(   CString&   retErrorMesg    )
{

	//   CALLED By:   SoundHelper::Pause_Play(),   On_BnClicked_Nudge_DView_Forward_Button(),     Hop_Forward_to_New_FilePosition()


	EventMan&                 eventMan  =    Get_EventMan(); 
	BitSourceStreaming   *bitSource =    Get_BitSource_Streaming();  

	if(   bitSource  ==  NULL  )
	{
		retErrorMesg =  "SoundHelper::Do_Post_Roll_CalcNotes  FAILED,  bitSource is NULL." ;
		return  false;
	}


	bool   wasPlayingBackward =  false;

	if(   m_audioPlayer  !=  NULL    )
	{
		if(   m_audioPlayer->m_playingBackward    )
			wasPlayingBackward =   true;
	}

	if(    wasPlayingBackward    )   
		return  true;             // ***** ESCAPE,  so far is only for  Forward Play ******




	long   currentPieClockPosition =     bitSource->m_currentPieClockVal;        //  was  just INCREMENTED, after calcing the last CalcNote that went to the FinCircQue.

	long   lostEventCount             =     ( 9  - currentPieClockPosition  )   +1;      //  +1:  Inclusive Counting.    (  9 is the highest PieClock value )  




	/*************   NO,  sometimes this is supposed to do  10 or more events.  Usage is different than  EventMan::Post_Roll_NoteDetect()   8/2012

	if(    currentPieClockPosition  >  9   )                //  9 means 1 LostEvent to do.   if it were 10,  then a DBlockLoad will next happen.  3/12
			return  true;   // there should be NO Lost events
	***/



	long   retLastSamplePlayed=   -1;


	if(    lostEventCount  > 0   )
	{
	//	TRACE(   "\n\n\nPAUSEplay:   EXPECT  %d  Lost-Events from POST-Roll.     (  Last PIE-CLOCK:  %d  ) .....     \n",   lostEventCount,   currentPieClockPosition   );


		if(    ! eventMan.Post_Roll_NoteDetect(   wasPlayingBackward,     retLastSamplePlayed,   -1,    retErrorMesg  )     )
			return  false;


		if(    retLastSamplePlayed  >=  0   )
		{

			m_audioPlayer->Set_LastPlayed_SampleIdx(   retLastSamplePlayed   );   // **********   NEW,  OK ???  *********************

//          TRACE(   "\nSoundHelper::Do_Post_Roll_CalcNotes  NO_PARM   Set_LastPlayed_SampleIdx   %d   \n\n  ",    retLastSamplePlayed   );
		}
		else
		{
			 ASSERT( 0 );       //    int   dummy =  9;   //  When does this get HIT (2/29/12)  ??????
		}
	}


	return  true;
}



					////////////////////////////////////////////////////////


bool    SoundHelper::Do_Post_Roll_CalcNotes(   long  eventCount,     CString&   retErrorMesg    )
{


	//	CALLED  BY:    PsNavigatorDlg::Hop_Forward_to_New_FilePosition(),     PsNavigatorDlg::On_BnClicked_Nudge_DView_Forward_Button()


	EventMan&                 eventMan  =    Get_EventMan(); 
	BitSourceStreaming   *bitSource =    Get_BitSource_Streaming();  

	if(   bitSource  ==  NULL  )
	{
		retErrorMesg =  "SoundHelper::Do_Post_Roll_CalcNotes  FAILED,  bitSource is NULL." ;
		return  false;
	}


	bool   wasPlayingBackward =  false;

	if(   m_audioPlayer  !=  NULL    )
	{
		if(   m_audioPlayer->m_playingBackward    )
			wasPlayingBackward =   true;
	}

	if(    wasPlayingBackward    )   
		return  true;             // ***** ESCAPE,  so far is only for  Forward Play ******




	long   retLastSamplePlayed=   -1;


	if(    eventCount  > 0   )
	{
		//   TRACE(   "\n\nHOP FORWARD play:   Moved  %d   Events with POST-Roll.   .....     \n",    eventCount   );


		if(     ! eventMan.Post_Roll_NoteDetect(   wasPlayingBackward,     retLastSamplePlayed,    eventCount,   retErrorMesg  )     )
			return  false;


		if(    retLastSamplePlayed  >=  0   )
		{
			m_audioPlayer->Set_LastPlayed_SampleIdx(   retLastSamplePlayed   );   // **********   NEW,  OK ???  *********************

//			TRACE(   "\n       SoundHelper::Do_Post_Roll_CalcNotes WithPARMS  [ %d PieSlices ]    Set_LastPlayed_SampleIdx  %d  \n "  ,  eventCount,   retLastSamplePlayed   );
		}
		else
		{
			int   dummy =  9;   //  When does this get HIT (2/29/12)  ??????
		}
	}

	return  true;
}





					////////////////////////////////////////////////////////
					////////////////////////////////////////////////////////


bool   SoundHelper::Move_to_Files_Position(   long   startSample,    CString&  retErrorMesg    ) 
{


		//  ****  This is only CALLED by Player  ...looks like it would be wrong for Navigator   11/11   ****


	ASSERT(    ! App_Has_Navigators_Delay()     );    //  This function is NOT YET for Navigator.   



	UniApplication&  uniApp =  Get_UniApp();


	retErrorMesg.Empty();


	if(   ! Is_WAV_Loaded()   )
	{
		retErrorMesg =  "Load a sound file." ;
		return  false;
	}


	m_audioPlayer->Set_LastPlayed_SampleIdx(  startSample  );   //  assign value to   PitchPlayerApp::m_lastFilePosition



	uniApp.Animate_All_Viewjs(  startSample,  -1  );   //  only moved the File slider

	uniApp.ReDraw_All_Anime_Viewjs();      //   Redraws the BULLETS  in Revolver .   WANT this ??????? .

	return  true;
}




					////////////////////////////////////////////////////////


bool   SoundHelper::Move_to_Files_Start(  CString&  retErrorMesg   ) 
{


		  //   now CALLED  from Both  PLAYER and NAVIGATOR   ????


	bool   useBoundaryRounding =   true;     //   false    ******  CAREFUL   11/11  *******************




	retErrorMesg.Empty();

	if(   ! Is_WAV_Loaded()    )
	{
		retErrorMesg =  "Load a sound file." ;
		return  false;
	}


	SequencerMidi  *midiSequencer   =	   Get_Current_MidiSequencer();
	ASSERT(            midiSequencer  );


	long   newPosition =  0;   //  default,  for Player




	if(    ! Initialize_Delay_Buffers_and_Two_CircQues( retErrorMesg )    )     //  For BOTH Player and Navigator  
	{	
		AfxMessageBox(  retErrorMesg  );	
		return  false;
	}

	Erase_Players_AudioDelay_MemoryBuffer();





	if(    App_Has_Navigators_Delay()    )     //  for Navigator, duh 
	{

		EventMan&   eventMan  =     Get_EventMan();  	//     do  PRE-ROLL  here,  after move and not wait for Play-Start												
		long    targSampleIdx =   0;   
		bool    backwardsPlay =  false; 
		long    retLastSamplePlayed;


		if(    ! eventMan.PreRoll_NoteDetect(   targSampleIdx,   backwardsPlay,    retLastSamplePlayed,  useBoundaryRounding,   retErrorMesg  )    )
		{
			AfxMessageBox(  retErrorMesg  );	
		}

		newPosition =   retLastSamplePlayed;
	}

	


	if(    ! Move_to_Files_Position(   newPosition,    retErrorMesg   )    )   
		return  false;



	m_lastTransportActionActual =   SoundHelper::FILEsTART;

	return  true;
}




					////////////////////////////////////////////////////////


bool   SoundHelper::Move_to_Files_End(   CString&  retErrorMesg   ) 
{

			  //   *****   Only CALLED  in  PitchPLAYER


	retErrorMesg.Empty();


	if(   ! Is_WAV_Loaded()   )
	{
		retErrorMesg =  "Load a sound file." ;
		return  false;
	}


//	long   lastSampleIdx    =       m_audioPlayer->Get_Biggest_SampleIndex();   *** BAD,  had the slowDown expansion in it
	long   lastSampleIdx    =      m_audioPlayer->Get_Biggest_SampleIndex_No_SpeedExpansion();   

	if(      lastSampleIdx  <= 0    )
	{  
		retErrorMesg =     "Load a sound file." ;
		return  false;
	}



	if(     ! Initialize_Delay_Buffers_and_Two_CircQues(  retErrorMesg  )     )      //   OK for PLAYER as well as Navigator
		return  false;


	Erase_Players_AudioDelay_MemoryBuffer(); 


	/****
		long   retSampleCnt;

		if(     !Get_DrivingViews_Sample_Count(   retSampleCnt,   retErrorMesg   )     )
			return  false;

		if(    retSampleCnt   > 0   )
			nuSampleIdx  -=    ( retSampleCnt  *    3L)  /  4L;   //  Nudge image to left so we can see some of Files last samples
		else
			nuSampleIdx  -=    ( 3L   *  512 );    //  just to allow Play buttons to work 

		if(    nuSampleIdx  < 0    )
			nuSampleIdx =  0;
	****/


//	long   padSamples  =    3     *     512;     //  Fails for   CrosscutSawLW.mp3()   ...may need more for MP3 files
	long   padSamples  =    15   *   1104; 



	long    nuSampleIdx  =    lastSampleIdx  -  padSamples;    //  just to allow Play buttons to work 



	if(     ! Move_to_Files_Position(   nuSampleIdx,    retErrorMesg   )     )
		return  false;


	m_lastTransportActionActual =   SoundHelper::FILEeND;

	return true;
}



					////////////////////////////////////////////////////////


bool	   SoundHelper::Get_Midi_Instrument_Text_Name(   short   midiInstrumentPatch,    CString&  retInstrumentsName   )
{

	retInstrumentsName.Empty();


//	#define  PATCHnAMEScOUNT  96
//  char    *patchNames[  PATCHnAMEScOUNT  ]  =

	if(   midiInstrumentPatch  < 0      ||     midiInstrumentPatch   >=  PATCHnAMEScOUNT   )
	{
		ASSERT( 0 );
		retInstrumentsName  =     "ERROR: midi number is out of range."  ;
		return  false;
	}


	retInstrumentsName  =    patchNames[   midiInstrumentPatch  ]; 


	return  true;
}












