/////////////////////////////////////////////////////////////////////////////
//
//  BitSourceStreaming.cpp   -   
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#include  "stdafx.h"



#include  <mmsystem.h>   // Is good on Sparky  ( ProgramFiles\Microsoft Visual Studio\VC\PlatformSDK\Include )


#include  "..\comnFacade\UniEditorAppsGlobals.h"

#include  "..\comnFacade\VoxAppsGlobals.h"



//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"      //  #include <FixMath.h>    ...Mac's version of Fixedpoint 
#include  "..\ComnGrafix\CommonGrafix.h"      //  #include "commonmy.h"   		

#include  "..\ComnGrafix\OffMap.h"  
#include  "..\comnGrafix\TransformMap.h"


#include "..\comnInterface\DetectorAbstracts.h"    // **** NEW ****


#include   "..\ComnGrafix\AnimeStream.h"

//////////////////////////////////////////////////  

#include  "..\comnAudio\sndSample.h"



#include  "..\comnAudio\dsoundJM.h"    //  I COPIED it in, bigger than the VC++ version   ****FIX, use the one from SDK !!!   **** JPM



#include   "..\ComnAudio\WaveJP.h"
	  	  



#include  "..\comnMisc\FileUni.h"  
#include   "..\comnAudio\Mp3Decoder.h" 
#include   "..\comnAudio\ReSampler.h"
#include  "..\ComnAudio\FFTslowDown.h"

#include  "..\comnAudio\WavConvert.h"



#include  "..\ComnAudio\CalcNote.h"

#include   "..\ComnAudio\SPitchCalc.h"



#include  "..\ComnAudio\NoteGenerator.h"



#include  "BitSourceAudio.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////

/***
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
***/
////////////////////////////////////////////////////////////////////////////////////////////////////////////



BitSourceStreaming::BitSourceStreaming() 
{


	CString  retErrorMesg;


	m_isInitialized =   false;    //  Carefull  



	m_chunkSize           =        TransformMap::Get_ChunkSize_OLD_PitchScope();    //   ***** HARDWIRED,   fix *******



	m_bytesPerSample  =        4;      //   BitSourceAudioMS::Bytes_Per_Sample();   //   ***** HARDWIRED,   for   16bit,  STEREO

//	m_totalBytes = 0;
	
	m_sampleRate  =    DEFsAMPLERATE;    //   44100;     // ***INSTALL,  should be read from file ********************


	m_numberOfWavemanEvents =   -1;


	m_wavVolumeSoundmanAddr  =    NULL;      //  the  ADDRESS( in soundman )ScalePitchMask  that isused for animation  
	m_midiVolumeSoundmanAddr  =   NULL;



	m_startPlaySampleIdx =  m_endPlaySampleIdx =   -1; 

	m_sampleIdxLastBlockLoadNotSlowExpanded =  0;  

	m_wavConvert =  NULL;



	/****  NOT needed Only for DEBUG  ???

	long   allocateSize  =   BLOCKLOCKEDSIZE;    //  44160    ..size of a mem fetch

	if(     (  m_backupInputBuffer =   ( BYTE* )malloc( allocateSize )   )     == NULL    )  
	{
		AfxMessageBox(   "BitSourceStreaming::BitSourceStreaming  failed,  could not allocate  m_backupInputBuffer."  );
		return;
	}
	****/


	m_numberOfWavemanEvents  =   NUMPLAYeventsPitchPLAYER;    //    same,    NUMPLAYeventsPitchNAVIGATOR;



	m_sPitchCalc =  NULL;


	m_noteGenerator =  NULL;


	m_recordNotesNow  =  false;


	
	m_topFreqFilterLimit      =       19500;         //    Dummy init.  These are really changed in   EventMan::Set_Detections_Base_Frequency_Cutoff()    

	m_bottomFreqFilterLimit =        20;     //   20  :   FULL     ....see  SoundHelper::Execute_FreqFilter_Change()


	
	m_inputScalePercent  =   1000;  //  NOT  500,  is in 2nd position as default .        (  this will REALLY get INITIALIZED on OpenFile   by  EventMan::Set_VolumeBoost_Factor(  short   factorCode   )




	m_soundDelayBuffer          =  NULL;   // ******* NEW, a circular buffer to give the sound a delay.
	m_soundDelayFetchIndex =   0;       //   in bytes  ( 4bytes per 16bit sample )




	m_sizeOfPieSlicesBuffer =    BLOCKLOCKEDSIZE;
	

	m_pieSlicesBuffer      =       new    BYTE[   m_sizeOfPieSlicesBuffer  ];   
	if(   m_pieSlicesBuffer ==  NULL  )
	{
	//	m_soundDelayBufferCount =    0;  
		ASSERT( 0 );   
	//	retErrorMesg =  "BitSourceStreaming::Allocate_WAV_Circular_Que  FAILED, can NOT allocate  m_pieSlicesBuffer"   ;
	}

	m_byteIndexToBufferPieSlices =  -1;

}



											////////////////////////////////////////


BitSourceStreaming::~BitSourceStreaming()
{

	Release_Media_File();   

	Release_WaveFormatEx();    

	if(   m_wavConvert   !=  NULL  )
	{
		m_wavConvert->Cleanup_After_Streaming();

		delete  m_wavConvert; 
		m_wavConvert =   NULL;
	}


	if(    m_sPitchCalc  !=  NULL    )
	{
		delete   m_sPitchCalc;
		m_sPitchCalc =  NULL;
	}


	if(    m_noteGenerator  !=  NULL    )
	{
		delete   m_noteGenerator;
		m_noteGenerator =  NULL;
	}


	if(    m_soundDelayBuffer   !=   NULL   )
	{
		delete   m_soundDelayBuffer;
		m_soundDelayBuffer =  NULL;
	}


	Release_WAV_CircularQue();



	if(   m_pieSlicesBuffer  !=  NULL   )
	{
		delete  m_pieSlicesBuffer;
		m_pieSlicesBuffer =  NULL;
	}
}



											////////////////////////////////////////


void   BitSourceStreaming::Release_WAV_CircularQue()
{
	
	if(    m_soundDelayBuffer   !=   NULL   )
	{
		delete   m_soundDelayBuffer;

		m_soundDelayBuffer =  NULL;
	}
}


											////////////////////////////////////////


void   BitSourceStreaming::Release_WaveFormatEx()
{

	if(    m_wavFormat   !=  NULL    )
	{
		GlobalFree(   m_wavFormat   );      	
		m_wavFormat =   NULL; 
	}
}


											////////////////////////////////////////


void     BitSourceStreaming::Release_All_Resources()
{

	Release_WaveFormatEx();   

	Release_Media_File();


	if(    m_wavConvert   !=  NULL   )
		m_wavConvert->Cleanup_After_Streaming();      //   do I also want this???


	Release_WAV_CircularQue();


	m_isInitialized =  false;  
}


											////////////////////////////////////////


void   BitSourceStreaming::Release_Media_File()
{

	if(   m_wavConvert  !=  NULL   )
	{
		m_wavConvert->Release_Media_File();
	}
}



											////////////////////////////////////////


bool    BitSourceStreaming::Allocate_WAV_Circular_Que(  short  appCode,    CString&  retErrorMesg   )
{

			//   Input  appCode = 1  to make sure this is allocated  


			//  Will allocate .62 MegaBytes of memory  ( 618 KB  )


	Release_WAV_CircularQue();


	if(    appCode   ==   1    )      //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope
		m_soundDelayFetchCount  =    kSoundDelayFetchCount;      //    9   seems better than 7 ???   Less bugs ...  why ????
	else
		m_soundDelayFetchCount  =    0;    //  currently,  only Navigtor   uses a delay, so it can creat the bitmaps on the fly       4/10



	if(   m_soundDelayFetchCount  ==  0   )
	{
		
		m_soundDelayBuffer          =   NULL;
		m_soundDelayBufferCount =    0;  
		m_soundDelayFetchIndex   =    0;
		
		return  true;     // *****  Do Nothing *****
	}


	long   totalBytes =     m_soundDelayFetchCount    *   BLOCKLOCKEDSIZE;   //   44,160  bytes in a fetch.   A fetch has 10 Events ( i.e.  PiePosition, it becomes a Note )


		
	m_soundDelayBuffer   =       new    BYTE[   totalBytes  ];   

	if(    m_soundDelayBuffer  ==  NULL     )
	{
		m_soundDelayBufferCount =    0;  

		ASSERT( 0 );   
		retErrorMesg =  "BitSourceStreaming::Allocate_WAV_Circular_Que  FAILED, can NOT allocate  m_soundDelayBuffer"   ;
		return  false;
	}


	m_soundDelayBufferCount  =    totalBytes;  

	m_soundDelayFetchIndex   =    0;

	return  true;
}


										////////////////////////////////////////


long   BitSourceStreaming::Calc_WAV_DelayBuffers_Ideal_Delay_In_Samples(   double   playSpeed,    long  bytesPerSample   )
{
			// **** NO Callers ???    7/24/12

	long  totalDelay =  -99999;


												//    this is the  SoundHardware-DELAY  in samples of the PlayBuffer's Hemisphere  

	long    hardwareHemisSampleCount  =    (long)( TWOSECBYTES / 2 )    / 4;    //   11,040 


	hardwareHemisSampleCount  =    (long)(    (double)hardwareHemisSampleCount  /  playSpeed  );  



												//   This is for the Delay  from  the  AudioPlayer's  Sound-BUFFERS  for   WAV data


	long   delayBuffersSampleCount  =    (  (  (long)kSoundDelayFetchCount  - 1)    *   (long)BLOCKLOCKEDSIZE  )  /  bytesPerSample;    //   m_curAudioPlayer->m_bytesPerSample;    //   /4
																			
	                                                                     // *******************  -1  ...seems right becase of the way we read and write to the DelayBuffer ??????	


	delayBuffersSampleCount  =    (long)(    (double)delayBuffersSampleCount   /  playSpeed   );   


//	/************   It's use at a Hemisphere event  iEvent = 0 or 30

//				m_sampleIdxCurPlayingBlockWrite  =    latestSampleIdx    -    delayBuffersSampleCount   
//																		                              -     hardwareHemisSampleCount;  //  + or - ????  THINK need to SUBTRACT because would have OLDER sampleIdx values (smaller ) 
																										                //   for what we are hearing, than the sampleIdx of the block that we wrote in the hardware
																										                //   buffer's  idle hemisphere.   3/11				

	totalDelay  =    delayBuffersSampleCount   +   hardwareHemisSampleCount;

	return   totalDelay;
}





											////////////////////////////////////////
											////////////////////////////////////////


void	  BitSourceStreaming::Erase_Players_AudioDelay_MemoryBuffer()
{

	if(   m_wavConvert  ==  NULL  )
	{
		ASSERT( 0 );     //    ***************** Can this Happen?   8/21/2012   
		return;
	}


	m_wavConvert->Erase_Players_AudioDelay_MemBuffer();
}




											////////////////////////////////////////


bool	  BitSourceStreaming::ReAllocate_Players_AudioDelay_MemoryBuffer(   long  numberNeededToMatch  )
{


	if(   m_wavConvert  ==  NULL  )
	{
		ASSERT( 0 );     //    ***************** Can this Happen?   8/21/2012   
		return  false;
	}


	long  sampleCountNotSlowDown =     m_wavConvert->m_outputBufferSampleCountNotSlowDown;


	if(     numberNeededToMatch   <=   0     )
		sampleCountNotSlowDown =   -1;                       //   -1:   This will DELETE the DelayBuffer...  NO WAVdelay for NoteList mode


	                              

	if(    ! m_wavConvert->ReAllocate_Players_AudioDelay_MemBuffer(    sampleCountNotSlowDown,    numberNeededToMatch   )     )  
	{
		ASSERT( 0 );
		return  false;
	}
	else
		return  true;
}


											////////////////////////////////////////


void	   BitSourceStreaming::Set_MidiSource_UsingNotelist_Variables(   bool  usingNotelist   )
{
													
									//   HAVE two vars to set,   {  SPitchCalc    and  in    WavConvert   }     8/2012


	if(    m_sPitchCalc  ==  NULL   )
	{
		ASSERT( 0 );     //    ***************** Can this Happen?   8/21/2012  
	}
	else
	{  if(    usingNotelist    )
			m_sPitchCalc->m_playModeUsingNotelist  =   1;
		else
			m_sPitchCalc->m_playModeUsingNotelist  =   0;
	}


	if(    m_wavConvert  ==  NULL   )
	{
		ASSERT( 0 );       //    ***************** Can this Happen?   8/21/2012  
	}
	else
	{  if(    usingNotelist    )
			m_wavConvert->m_playModeUsingNotelist  =   1;
		else
			m_wavConvert->m_playModeUsingNotelist  =   0;
	}
}



											////////////////////////////////////////


bool    BitSourceStreaming::Allocate_SPitchCalc(    long  numberOfWavemanEvents,    long  byteCountTwoSeconds,    long  numSamplesInBlockLocked, 
												                                   ListDoubLinkMemry< MidiNote >  *noteList ,   long  *noteListWeirdDelayAddr,   
																				                                                                 bool  usingCircQueDFTprobes,      CString&  retErrorMesg   )
{

	retErrorMesg.Empty();			//   the  'SPitchCalc'  object will reside in BitSource


	if(    m_sPitchCalc  !=  NULL    )
	{
		delete   m_sPitchCalc;
		m_sPitchCalc =  NULL;
	}


	m_sPitchCalc  =      new   SPitchCalc(    numberOfWavemanEvents,   byteCountTwoSeconds,   numSamplesInBlockLocked,   noteList,   usingCircQueDFTprobes   );    
	if(  m_sPitchCalc  ==  NULL   )
	{
		retErrorMesg  =   "BitSourceStreaming::Allocate_SPitchCalc  FAILED,  can NOT allocate  SPitchCalc"   ;
		return   false;
	}


	m_sPitchCalc->m_noteListHardwareDelayForPlayerAddr  =    noteListWeirdDelayAddr;     //   assign the member vars   


	return  true;
}



											////////////////////////////////////////


NoteGenerator*    BitSourceStreaming::Allocate_NoteGenerator(    short   filesVersionNumber,    CString&  retErrorMesg   )
{

	retErrorMesg.Empty();


	if(    m_noteGenerator  !=  NULL    )
	{
		delete   m_noteGenerator;
		m_noteGenerator =  NULL;
	}


	m_noteGenerator          =        new   NoteGenerator();    
	if(  m_noteGenerator  ==  NULL   )
	{
		retErrorMesg  =   "BitSourceStreaming::Allocate_NoteGenerator  FAILED,  can NOT allocate  NoteGenerator"   ;

		m_recordNotesNow =   false;
		return   NULL;
	}


	m_noteGenerator->m_filesVersionNumber  =     filesVersionNumber;



	return  m_noteGenerator;
}


											////////////////////////////////////////


 bool   BitSourceStreaming::Initialize(    long  chunkSize,    CString&   filePath,  	 bool&  fileAcessError,    long  numberNeededToMatch,  
	                                                                     long  *noteListWeirdDelayAddr,     bool  usingCircQueDFTprobes,     CString&   retErrorMesg   )
{


	
	short    appCode =   Get_PitchScope_App_Code_GLB();    //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope

// ********************  PROBLEMS for   Make_NoteList_NoAudio()   Do not want this circQue  of WAV  **********************************



	long  numberBytesIn16bitSample =   4;

	long  numSamplesInBlockLocked  =     BLOCKLOCKEDSIZE  /   numberBytesIn16bitSample;    //  11,040  samples in  Fetch_Streaming_Samples_Direct() block-load



	retErrorMesg.Empty();
	fileAcessError =   false;


	m_isInitialized  =  false;

	if(        filePath.IsEmpty()   
		||    m_wavConvert ==  NULL   )
	{
		retErrorMesg =    " BitSourceStreamingMS::Initialize  failed,   filePath is empty or WaveConvert is NULL."  ;
		return  false;
	}


	ASSERT(   m_numberOfWavemanEvents  > 0  );



	Release_All_Resources();   //   this was already called in  PitchPlayerApp::Initialize_BitSource(),  but we can leave it and play safe.  1/28/10

	


	if(    ! Allocate_WAV_Circular_Que(   appCode,   retErrorMesg   )     )
		return  false;




	ListDoubLinkMemry< MidiNote >  *noteList =   NULL;    //   SLOPPY,  but keep compiler happy,  we will attach it in a following function call   12/11


	if(    ! Allocate_SPitchCalc(    m_numberOfWavemanEvents,   (long)(TWOSECBYTES),   numSamplesInBlockLocked,   noteList,  noteListWeirdDelayAddr,   
																						                                 usingCircQueDFTprobes,  	retErrorMesg   )     )
		return  false;


	m_strWavFilePath  =    filePath;


	bool   retIsAmp3File;

	m_wavConvert->Set_SRC_Sound_File(   filePath,    retIsAmp3File  );   // this will now pick accoring to files extension




	if(     ! m_wavConvert->Initialize_for_Streaming(   retIsAmp3File,    numberNeededToMatch,    retErrorMesg  )      )
		return  false;




	m_currentByteIdx =   0L;   // ******************  this seems to be unreliable ???    2/9/12

	m_sampleIdxLastBlockLoadNotSlowExpanded =  0;    //  NEW,  should be more accurate than OLD  m_currentByteIdx



	m_currentPieClockVal  =  0;      //    {  0 to   9  }   10 Events

    m_byteIndexToBufferPieSlices =   0;   // ******************  OK ???  ******************************



	m_isInitialized       =   true;

	return  true;
}



											////////////////////////////////////////


long		BitSourceStreaming::Calc_Files_Total_Output_Bytes_With_SpeedExpansion()
{

											// ****  CAREFUL,  also has the SLOWDOWN  expansion included		
	if(   m_wavConvert   ==   NULL   )
	{ 
		ASSERT( 0 );
		return  0;
	}

	long     byteCount  =    m_wavConvert->Calc_Files_Total_Output_Bytes_With_SpeedExpansion();     //  also include SlowDown,  is that OK ?????
	return  byteCount;
}




											////////////////////////////////////////


long		BitSourceStreaming::Get_Files_Total_Samples_No_SpeedExpansion()
{

	 //    does  NOT have the  SLOWDOWN  EXPANSION.   These are OUTPUT BYTES Format [44hz 16bit],  even if a ReSampled wav is loaded.  8/2012  

	//     CAREFULL,  this is called by    AudioPlayer::Get_Biggest_SampleIndex_No_SpeedExpansion(),   which is called by MANY functions  

		
	long  bytesPerSampleOutput =   4;     //  always for OUTPUT BYTES Format  [44hz 16bit]   


	if(   m_wavConvert   ==   NULL   )
	{ 
		ASSERT( 0 );
		return  0;
	}



	long         byteCount    =    m_wavConvert->Calc_Files_Total_Output_Bytes_no_SpeedExpansion();

	ASSERT(   byteCount  >=  0  );   //   if number exceeds the limits of long,  it might come back




	long     totalSamples =    byteCount  /   bytesPerSampleOutput;
	return  totalSamples;
}



											////////////////////////////////////////


long		BitSourceStreaming::Get_Biggest_SndSample_Index()  
{   

		//  Use this AFTER  Fetch_Streaming_Samples_Direct_PPlayer() has been CALLED.  This is the number of TRUR data samples in the NOT-Slow  SndSample 

		//  also see    SPitchCalc::::Get_SndSamples_Valid_Count()

	if(  m_wavConvert !=  NULL )   
		return    m_wavConvert->m_biggestIndexToSndSample;																					  
	else          
		return   -1;    
}


											////////////////////////////////////////


void		BitSourceStreaming::Initialze_For_File_Position_Change()
{


//			curSample  =   ( bitSource->m_sampleIdxLastBlockLoadNotSlowExpanded  -  samplesInDataFetch  )      
//																							+      (   bitSource->m_currentPieClockVal   *  sampCountInPieSliceWithSpeedRedux  );

	m_currentPieClockVal  =  0;     //  Seems OK.  The PieSlice Buffer ( m_pieSlicesBuffer )  will be REFRESHED after this ( PreeRoll  and FileSeek )


	m_byteIndexToBufferPieSlices =  0;   //   0:   This   WILL/SHOULD   cause an IMMEDIATE refresh of the  Pie Slices Buffer  (   m_pieSlicesBuffer  )   2/12


	//	m_sampleIdxLastBlockLoadNotSlowExpanded =  0;       ******  NO,  let  Seek_New_DataPosition_Fast()  change this value



	if(     m_sPitchCalc  !=  NULL    )	
		m_sPitchCalc->Initialze_For_File_Position_Change();




	if(   m_wavConvert   !=  NULL    )
		m_wavConvert->Initialze_For_File_Position_Change(); 
}




											////////////////////////////////////////


void	  BitSourceStreaming::Initialize_For_Playing()
{


		//   This new GENERIC Function  is supposed to be a place where all of the MINOR Classes ( WavConvert,  SPitchCalc,  FFTslowDown )  can INITIALIZE.



	short  appCode =    Get_PitchScope_App_Code_GLB();         //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope



	if(   m_sPitchCalc   !=   NULL    )     // Looks like this is for ALL PLAYING
	{
		m_sPitchCalc->Initialize_for_Play();      //   Has to do with VARS that determine whne to turn a SYNTH on for a new note or not (both Detection and Notelist play )
	}															  //   m_aNoteIsPlaying =  false;   m_prevFoundNote =  -1;   ...Used by  Get_New_Filtered_ReturnNote_CircularQue()




	if(          m_wavConvert  !=  NULL   
		 &&    appCode  ==   1    )      //   ****** ONLY for Navigator ???  ******************
	{


//	   Erase_FFTSlowDowns_Buffers();    NO!!!   Do not want to erase    Erase_SlowDown_Buffers(),   just the accumulators.   Get a static burst when does the change.  8/30/2012


		m_wavConvert->Initialize_for_Play();        //   Only  CALLS    FFTslowDown::Erase_OutputAccumulators()  
	}
}



											////////////////////////////////////////


void    BitSourceStreaming::Erase_FFTSlowDowns_Buffers()
{

									//   	 clears 2 types of buffers  


	if(    m_wavConvert  ==   NULL   )
	{
		ASSERT( 0 );
		return;
	}


	m_wavConvert->m_fftSlowDown.Erase_SlowDown_Buffers();  


	m_wavConvert->m_fftSlowDown.Erase_OutputAccumulators();  	
}



											////////////////////////////////////////


void	  BitSourceStreaming::Set_Computer_Performance_Factor(   long  computerPerformanceFactor   )
{


	short   appCode  =    Get_PitchScope_App_Code_GLB();         //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope

	if(       appCode   !=   1    )     //   1:   Only for NAVIGATOR
		return;   



	if(     m_wavConvert ==  NULL   )
	{
	//	ASSERT( 0 );    OK,  sometimes if a song is NOT loaded this bets called,  but that is OK.   9/2012
		return;
	}

	m_wavConvert->Set_Computer_Performance_Values(   computerPerformanceFactor   );
}



											////////////////////////////////////////


void	     BitSourceStreaming::Increment_Pie_Clock()   
{   
		
	m_currentPieClockVal++;    //  does this just keep going BEYOND 10...   untill a new BlockLoad is made ????
}	


											////////////////////////////////////////


void   BitSourceStreaming::Adjust_Volume_on_CopyBytes(  	char  chVal0,  char chVal1,  char chVal2,   char chVal3,     BYTE  *destBufferBytes,
																					                                             UINT  byteIdx,    short  rightChannelPercent    )  
{

				//  Since  PLAYER does not have a delay buffer for WAV data,  must adjust Volume here.


	bool   allowAudibleStereoBalancePlayer  =   true;     //  Only for Player (not Navigator) do we adjust Volume and or StreoBalance in this funct   3/11 




	short    auditionCode   =  AnimeStream::NORMAL ;  // **********   ALWAYS ??? *******************8


	short   appCode  =    Get_PitchScope_App_Code_GLB();         //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope


	unsigned char   eraseVal =   0;   //  was  127     {  0 - 255  }


	long   sixteenBitLimit  =   127L;    //   32512    Is this right ???    This is the biggest value the attenuated signal can receive.     3/2010
	sixteenBitLimit  =     sixteenBitLimit   <<  8;     //  looks right,   see  WavConvert::Multiply_Float_Samples()  "31000.0,   could go almost to  32768.0  if I want"


	ASSERT(   rightChannelPercent >= 0    &&    rightChannelPercent <=  100   );     //  how to  BALANCE the left and right STEREO samples

	double   leftChannelPercentFlt    =     (  100.0 - (double)rightChannelPercent  )      / 100.0;                
	double   rightChannelPercentFlt  =                   (double)rightChannelPercent          / 100.0;




		if(   appCode  ==   0  )      //    0:    ONLY for Player,   not Navigator  
		{

			double   boostVol =   2.0;     //  1.5      ***ADJUST***  Need this  because  { leftChannelPercentFlt,  rightChannelPercentFlt  }   are typically 50% each, and reduct the volume.


			long   leftSample16    =    chVal1;
			leftSample16  =       (  leftSample16   <<  8  )    |    ( BYTE )chVal0;    //   Need cast so   '|'   will  pack  with negative numbers 

			long   rightSample16    =    chVal3;
			rightSample16  =      (  rightSample16   <<  8  )    |    ( BYTE )chVal2;    //   Need cast so   '|'   will  pack  with negative numbers 



			long     attenSampleLeft,    attenSampleRight;  

			double  attenSampleLeftDB    =    (double)(       (    leftSample16    *  (*m_wavVolumeSoundmanAddr)   )  / 100L       ); 
			double  attenSampleRightDB  =    (double)(       (    rightSample16  *  (*m_wavVolumeSoundmanAddr)   )  / 100L       ); 



			if(    allowAudibleStereoBalancePlayer    )     //  Switch at top,  still deciding if I want to hear this during DETECTION or not.   3/22/11
			{

				double    reduceFactor  =   0.35;     //   .35[ very good, maybe perfect ]   11/11    **** ADJUST ?? ***  need to make volume equal in all channels. Watch out for value overflow below   11/11

				double   differenceFlt,   adjustedBoostVol;
				long      diffSignal;


				if(	   rightChannelPercent  ==   50   )			
				{
					attenSampleLeft      =     (long)(    attenSampleLeftDB    *  leftChannelPercentFlt         *  boostVol    );   
					attenSampleRight    =     (long)(    attenSampleRightDB  *  rightChannelPercentFlt       *  boostVol    );
				}
				else if(    leftChannelPercentFlt    >  rightChannelPercentFlt     )   // User pushed StereoBalance to far LEFT, so want LEFT channel emphgasized
				{

					differenceFlt  =    leftChannelPercentFlt   -  rightChannelPercentFlt;  

					adjustedBoostVol  =    boostVol     -    (   reduceFactor   *   ( differenceFlt  *   boostVol )     );     // need to reduce volumne as a whole,  becase we increase to the LEFT channel and right 


					attenSampleLeft      =     (long)(    attenSampleLeftDB    *  leftChannelPercentFlt         *  adjustedBoostVol    );   
					attenSampleRight    =     (long)(    attenSampleRightDB  *  rightChannelPercentFlt       *  adjustedBoostVol    );


					diffSignal  =    (long)(    attenSampleLeftDB    *  differenceFlt         *  adjustedBoostVol    );   //  This also empasizes the LEFT channel while keeping volume the same in both channels.  11/11

					attenSampleRight   +=   diffSignal; 
				}
				else if(    rightChannelPercentFlt    >  leftChannelPercentFlt   )
				{

					differenceFlt  =   rightChannelPercentFlt   -   leftChannelPercentFlt;  

					adjustedBoostVol  =    boostVol     -  (   reduceFactor   *   ( differenceFlt  *   boostVol )     ); 


					attenSampleLeft      =     (long)(    attenSampleLeftDB    *  leftChannelPercentFlt         *  adjustedBoostVol    );   // **** SHOULD be doing this boost
					attenSampleRight    =     (long)(    attenSampleRightDB  *  rightChannelPercentFlt       *  adjustedBoostVol    );


					diffSignal  =    (long)(    attenSampleRightDB    *  differenceFlt         *  adjustedBoostVol    ); 

					attenSampleLeft   +=   diffSignal; 
				}
				else
				{	ASSERT( 0 );   }

			}
			else
			{  attenSampleLeft    =     (long)attenSampleLeftDB;    //  Sometimes do NOT want to adjust the stereo balance.
				attenSampleRight  =     (long)attenSampleRightDB;
			}



			if(           attenSampleLeft  >  sixteenBitLimit   )     //  if we go out of range,  then will hear static.  [  Careful how adjust  ' reduceFactor' above    11/11
				attenSampleLeft  =   sixteenBitLimit;
			else if(    attenSampleLeft  <  -sixteenBitLimit   )
				attenSampleLeft  =   -sixteenBitLimit;

			if(           attenSampleRight  >  sixteenBitLimit   )
				attenSampleRight  =   sixteenBitLimit;
			else if(    attenSampleRight  <  -sixteenBitLimit   )
				attenSampleRight  =   -sixteenBitLimit;

																									//  Finally, write out the modified bytes for the sound buffer

			chVal0   =        (char)(    (  attenSampleLeft     &    0x000000ff  )              );      //  low left
			chVal1    =       (char)(    ( attenSampleLeft      &    0xffffff00    )  >> 8     );      //  hi right

			chVal2   =      (char)(    (  attenSampleRight    &    0x000000ff  )               );      //  low  right
			chVal3   =      (char)(    (  attenSampleRight    &    0xffffff00    )   >> 8     );      //  hi right

		}   //   if(   appCode  ==   Player  
		else
		{   } //   For NAVIGATOR,   NO longer allow   {  chVal0  chVal1  chVal2  chVal3  }  get modified in this function.  It is now done by  Apply_Volume_Adjust_PPlayer()  3/11
		



																	//  apply the MASKING of  'AUDITION modes'
		switch(    auditionCode    )
		{
				case   AnimeStream::NORMAL :
					*(   destBufferBytes  +   ( byteIdx -3 )   )  =    chVal0;    //  left
					*(   destBufferBytes  +   ( byteIdx -2 )   )  =    chVal1;   

					*(   destBufferBytes  +   ( byteIdx -1 )   )  =     chVal2;   //  right
					*(   destBufferBytes  +     byteIdx          )  =    chVal3;   
				break;

				case   AnimeStream::AllLEFT :
				case   AnimeStream::MIDIandLEFT : 
					*(   destBufferBytes  +   ( byteIdx -3 )   )  =    chVal0;
					*(   destBufferBytes  +   ( byteIdx -2 )   )  =    chVal1;   
					*(   destBufferBytes  +   ( byteIdx -1 )   )  =     chVal0;    
					*(   destBufferBytes  +     byteIdx          )  =    chVal1;   
				break;
			
				case   AnimeStream::AllRIGHT :   
				case   AnimeStream::MIDIandRIGHT :
					*(   destBufferBytes  +   ( byteIdx -3 )   )  =    chVal2;
					*(   destBufferBytes  +   ( byteIdx -2 )   )  =    chVal3;   
					*(   destBufferBytes  +   ( byteIdx -1 )   )  =    chVal2;  
					*(   destBufferBytes  +     byteIdx          )  =    chVal3;   
				break;

				case   AnimeStream::MIDIandSAMPLE :

						*(   destBufferBytes  +   ( byteIdx -3 )   )  =    chVal0;
						*(   destBufferBytes  +   ( byteIdx -2 )   )  =    chVal1;   
						*(   destBufferBytes  +   ( byteIdx -1 )   )  =    chVal2;  
						*(   destBufferBytes  +     byteIdx          )  =    chVal3;   
				break;

				case   AnimeStream::JUSTMIDI :
						*(   destBufferBytes  +   ( byteIdx -3 )   )  =    eraseVal;
						*(   destBufferBytes  +   ( byteIdx -2 )   )  =    eraseVal;   
						*(   destBufferBytes  +   ( byteIdx -1 )   )  =    eraseVal;    // write  'behind'  in the LONG sample
						*(   destBufferBytes  +     byteIdx          )  =    eraseVal;   
				break;

				default:   
					ASSERT( 0 );    
				break;
		}
}



											////////////////////////////////////////


bool	  BitSourceStreaming::Fetch_PieSlice_Samples(   long  samplesToLoad,    BYTE  *destBuffersPieSlicePtr,    bool&  retHitEOF,   bool fowardFlag,	 
													 						        short  rightChannelPercent,    long   startOffsetByteBackwards,	  CString&  retErrorMesg   )
{

		//  Can and does load  ANY AMOUNT of Samples   [   sometimes 1 PieSlice ( 1104 samps ),  and sometimes 10 PieSlices (11,040 samps )
		//
		//   It can also read AHEAD,  to make sure that detection's  SndSample  always has data for calcs.    2/2012


	long   bytesInSample  =   m_bytesPerSample;   //   4   ******* HARDWIRED ***********



	retErrorMesg.Empty();
	retHitEOF =  false;

	ASSERT(   destBuffersPieSlicePtr  );
	ASSERT(   m_pieSlicesBuffer  );
	ASSERT(    fowardFlag   ||   startOffsetByteBackwards >= 0    );     

	SPitchCalc   *sPitchCalcer  =   m_sPitchCalc;  
	ASSERT(      sPitchCalcer   );       


	double  speedFlt  =    sPitchCalcer->m_playSpeedFlt; 
	long     blockSize   =    Get_Load_Blocks_Size();        //  44160  in  "Slow-Expanded bytes",     the bytes in a HALF rotation of the hardware Sound Buffer
	long     sampleCount     =   blockSize / 4;

	long     subBlockSize  =     m_wavConvert->m_outputBufferSampleCountNotSlowDown  *   MP3rEADERbYTESpERsAMPLE;                 //   4416  or  8832  ???


	bool   isPlayingNoteList =   false;
	if(   sPitchCalcer->m_playModeUsingNotelist  ==  1  )       //    1: Playing from  NOTELIST
		 isPlayingNoteList =   true;




	BYTE  *destRover =    destBuffersPieSlicePtr;       //   Point to the   WAVdelay  buffers   SECTION 




	for(    long   sampIdx = 0;     sampIdx  < samplesToLoad;     sampIdx++  )
	{

		
		bool    bufferNeedsFilling =   false;

		if(    m_byteIndexToBufferPieSlices  ==  0   )    // *********   BETTER with  -1  ???   *********************
			bufferNeedsFilling =   true;      

		if(    m_byteIndexToBufferPieSlices   >=   m_sizeOfPieSlicesBuffer    )     //  m_sizeOfPieSlicesBuffer   is always  44160 ???    7/17/2012
			bufferNeedsFilling =   true;                      



		if(    bufferNeedsFilling   )  
		{

																			//     Prepare   DETECTOR's  SndSample   for write
			bool     doPitchDetectionCalcs =   false;

			if(    m_sPitchCalc->m_spitchCalcsSndSample  !=  NULL   )
			{
				m_sPitchCalc->m_spitchCalcsSndSample->Erase();   //  If we go to a slowedDown-speed,  we will only use part of the SndSample, so clear out some nonsense at end.
												                                              //  If I wanted to do this right I should realloc a SMALLER sndSample when doing SlowedDown play.  
				doPitchDetectionCalcs =   true;
			}
			else
			{	ASSERT( 0 );   // Does this get hit ????   1/25/2012
				doPitchDetectionCalcs =   false;
			}
			//////////////////////////////////////////////   Between  the comment-bars  is from  Fetch_Streaming_Samples_Direct_PPlayer()    2/2012



			if(   ! fowardFlag    )             //   Going  BACKWARDS,   so need a  File-SEEK  for every BLOCK LOAD
			{

				if(      ! Seek_New_DataPosition_Fast(    startOffsetByteBackwards,   BitSource::BACKWARDS,    retErrorMesg   )     )   
					return   false;
			}


			
														 //   If going BACKWARDS,   then fill the NEW  'BackwardsPlay-BUFFER'  in WavConvert with 10 blocks ( 44,160 bytes ) 
			bool   retEndOfFileLoc =  false;
			long   retBytesReadLoc =  0;


			if(        ! fowardFlag   
				&&    m_wavConvert->m_backwardsPlayBufferSize  >  0   )
			{

				if(    m_wavConvert->m_backwardsPlayBuffer  ==  NULL   )
				{  
					retErrorMesg =   "Fetch_PieSlice_Samples  FAILED,  m_wavConvert->m_backwardsPlayBuffer is NULL." ;
					return  false;
				}
				else
				{	if(    ! m_wavConvert->Load_Ten_MemoryBlock_Frames(   retEndOfFileLoc,    retBytesReadLoc,   blockSize,    retErrorMesg  )     )    //   44160 bytes
					{
						ASSERT( 0 );   
						return  false;
					}


					if(   retEndOfFileLoc   )    // ***** REFINE,  it may have SOME of the 10 blocks.  How to partially use???   12/21/11
					{					

						if(   retBytesReadLoc  <   (  subBlockSize  )     )
						{
							retHitEOF =   true;    //   Do the same thing just after   Fetch_Current_Sample_and_Increment()           OK if this happens     2/10
							return   false;
						}
						else
							sampleCount  =    retBytesReadLoc  / 4;
					}
				}
			}



						                       //    INITIALIZATIONS  for the   BLOCK  DataFetch


			long   slowExpandedBytesRead =  0;   //     WAS:  actualBytesRead             If only loaded   part of   10 subBlocks above,  then this will give the accurate count.  12/11
														


			UINT  byteIdx =  3;     //  an  UGLY initialization,  but that is the way the pointers are set up below


			m_wavConvert->m_biggestIndexToSndSample   =   -1;     // ******  NEW INITIALIZATIONS  for BlockLoad and creation of NOT-Slow SndSample *****************


		          // ***** CAREFUL with this initialization,   1/31/2012   *************************************************
			m_sPitchCalc->m_indexLastSndSampleBlockload =  -8;   //  ****** Is it a MISTAKE to do this???  ( because of partial BlockLOAD??  ) INIT,   Fetch_Current_Sample_and_Increment() will assign a VALUE when it is done.
																			//  Actually this has been very useful in FINDING BUGS cleaning up some code.  Keep it around for a while.   1/31/2012
		          // *********************************************************************************************

			bool   retDidBlockLoadOuter  =  false;



			long  samplesInBlock =    m_sizeOfPieSlicesBuffer /  bytesInSample;    //  It could vary i we change m_sizeOfPieSlicesBuffer,  but would 
																												 //  be hard on  m_spitchCalcsSndSample



		    for(   long  spx= 0;     spx <  samplesInBlock;      spx++    )    //  Copy the bytes  from the IO to the buffer. 
			{															 

				char   chVal0=0,    chVal1=0,   chVal2=0,    chVal3=0;   //  When used to be below,  but got a  error   Run-Time Check Failure #3 ( this bug was in PitchScope 1.0
				bool   retEndOfFile =   false;
				bool   retDidBlockLoadLoc =  false;


				if(   ! m_wavConvert->Fetch_Current_Sample_and_Increment(   retEndOfFile,   m_currentByteIdx,    
																											  chVal0,    chVal1,  chVal2,    chVal3,  	 ! fowardFlag,  
																											  m_sPitchCalc->m_spitchCalcsSndSample,    
																											  spx,     rightChannelPercent,   retDidBlockLoadLoc,  isPlayingNoteList,  retErrorMesg  )   )        
				{  if(   retEndOfFile   )
						retHitEOF =   true;    //  OK if this happens   2/10     Get here from move FileSlider all the way to the end.    1/31/12

					return   false;
				}

				if(     retDidBlockLoadLoc    )
					retDidBlockLoadOuter  =   true;  


				Adjust_Volume_on_CopyBytes(    chVal0,  chVal1,  chVal2,  chVal3,     m_pieSlicesBuffer,   byteIdx,    rightChannelPercent   );  
																			 

				m_currentByteIdx  +=   4;
				byteIdx                 +=   4;     //  +4 :    because now we read a   whole 16bit stereo SAMPLE   at a time.
				slowExpandedBytesRead    +=  4;
	
			}   //  for(   long  spx= 0;    spx <  sampleCount




//			m_sampleIdxLastBlockLoadNotSlowExpanded  +=   ( slowExpandedBytesRead  /  m_bytesPerSample  ); **** WRONG ****      //  4  ***** NEW,  BUG,  divide by speed   2/8/12    **********************
			long   actualBytesRead  =      (long)(      (double)slowExpandedBytesRead  /  speedFlt     );

			m_sampleIdxLastBlockLoadNotSlowExpanded  +=    actualBytesRead /   m_bytesPerSample;   //  in the  "NOT-Slowed Domain"   of Samples



			m_currentPieClockVal =  0;     //   { 0 - 9  }    10 Events,  so  re INIT  for every load of this many  BLOCK bytes




			double   retOverbriteRatio   =  -2.0;			

			if(   ! sPitchCalcer->m_spitchCalcsSndSample->Apply_Full_Filtering_FixedScaling(   m_topFreqFilterLimit,   m_bottomFreqFilterLimit,   m_inputScalePercent,   
																																				retOverbriteRatio,	  retErrorMesg )    )
			{  ASSERT( 0 );   }			

							 	//  a  LARGE  CIRCULAR-Que  for  OLD SndSamples  ....so I can read BACK in TIME for very slow speeds that dont fetch many samples    2/12

			long  bytesToProcessAlt =     Get_Biggest_SndSample_Index(); 
			long  bytesToProcess     =     sPitchCalcer->Get_SndSamples_Valid_Count(  sPitchCalcer->m_playSpeedFlt  );  // ***** CAREFUL, not fully tested.  1/29/12  ****** 

			if(   bytesToProcess  !=   bytesToProcessAlt    )
			{	int dummy =  9;  }

			sPitchCalcer->Add_SndSample_Samples_to_SndSampleCircularQue(  bytesToProcess  );

			//////////////////////////////////////////////

					
			//  curSample  =   ( bitSource->m_sampleIdxLastBlockLoadNotSlowExpanded  -  samplesInDataFetch  )   +   (   bitSource->m_currentPieClockVal   *  sampCountInPieSliceWithSpeedRedux  );

			m_byteIndexToBufferPieSlices =  0;        //   re-init  the traversing pointer to the buffer  

			m_currentPieClockVal  =  0;   // ***********   ???????????   OK    FIX: Did this up above  ************** ?????????????     2/9/12  

		}   //   if(    bufferNeedsFilling 
		else
		{  int   dummy  =  9;   }  //  No BLOCKLOAD,  Just working its way through the BUFFER( m_pieSlicesBuffer )   [ index is m_currentPieClockVal  to 10   1104 slices 




																			            //   Finally,  copy out the  4 byte Sample to dest on this Iteration of 'sampIdx' 

		ASSERT(  m_byteIndexToBufferPieSlices  <  m_sizeOfPieSlicesBuffer  );



		for(  long i =0;    i < bytesInSample;   i++  )
		{

			*destRover  =     *(   m_pieSlicesBuffer  +   m_byteIndexToBufferPieSlices   );
			destRover++;

			m_byteIndexToBufferPieSlices++;


			if(    m_byteIndexToBufferPieSlices   >=   m_sizeOfPieSlicesBuffer    ) 
			{
				break;   //   send us to the OUTER loop,  and a refresh of the memblock.   *** TESTED,  is OK.  2/9/2012 ***
			}
		}

	}  //    for( sampIdx =0;   sampIdx < samplesToLoad


	return  true;
}



											////////////////////////////////////////


BYTE*    BitSourceStreaming::Get_DelayBuffer_Read_Byte(  long  iEvent,    long  dataFetchCount  )
{


	BYTE  *retByte =   NULL;

	if(    m_soundDelayBufferCount  <=  0  )   //  this would mean that we are NOT using a Delay buffer
		return   NULL;



//	long  oldestIndex  =   m_soundDelayFetchIndex;   //  Since Get_DelayBuffer_Write_Byte() incremented  m_soundDelayFetchIndex, it now points to the OLDEST block

	long   biggestIndex  =    (long)kSoundDelayFetchCount   -  1L;


	if(   m_soundDelayFetchIndex  >  biggestIndex   )
	{
		ASSERT( 0 );
		retByte  =     m_soundDelayBuffer    +      (  biggestIndex                    *  BLOCKLOCKEDSIZE   );
	}
	else		
		retByte  =     m_soundDelayBuffer    +      (  m_soundDelayFetchIndex  *  BLOCKLOCKEDSIZE   );


	return  retByte;
}



											////////////////////////////////////////


BYTE*    BitSourceStreaming::Get_DelayBuffer_Write_Byte(   long  iEvent,   long  dataFetchCount   )
{


	BYTE  *retByte =   NULL;


	if(    m_soundDelayBufferCount  <=  0  )   //  this would mean that we are NOT using a Delay buffer
		return   NULL;


	long   biggestIndex  =    (long)kSoundDelayFetchCount   -  1L;



	if(   m_soundDelayFetchIndex   >   biggestIndex   )
	{

		ASSERT( 0 );   //  should not happen

		retByte  =     m_soundDelayBuffer    +      ( biggestIndex  *  BLOCKLOCKEDSIZE   );    //  read the last byte

		m_soundDelayFetchIndex =   0;                       //   and reset the pointer to the beginning of the buffer    3/11
	}
	else		
	{	retByte  =     m_soundDelayBuffer    +      ( m_soundDelayFetchIndex  *  BLOCKLOCKEDSIZE   );

		m_soundDelayFetchIndex++;
	}



//	BYTE  *retByte  =     m_soundDelayBuffer    +      ( m_soundDelayFetchIndex  *  BLOCKLOCKEDSIZE   );


	if(   m_soundDelayFetchIndex  >=  m_soundDelayFetchCount   )  
		m_soundDelayFetchIndex =   0;


	return  retByte;
}



											////////////////////////////////////////


BYTE*    BitSourceStreaming::Get_DelayBuffer_Write_Byte_BackwardsPlay(   long  iEvent,   long  dataFetchCount   )
{

	BYTE  *retByte =   NULL;


	if(    m_soundDelayBufferCount  <=  0  )   //  this would mean that we are NOT using a Delay buffer
		return   NULL;


	long   biggestIndex  =    (long)kSoundDelayFetchCount   -  1L;



	m_soundDelayFetchIndex--;      //  for  BACKWARDS play,   FIRST we DECREMENT the index  ( rotates in CounterClockwise direction  )

												   //  for  BACKWARDS play  we write the JustLoaded BLOCK to NEWEST Block in the CircQue ( the opposite of Forwards play )  3/11

	if(   m_soundDelayFetchIndex  < 0 )          
		m_soundDelayFetchIndex =   m_soundDelayFetchCount  -1;         



	if(   m_soundDelayFetchIndex   >   biggestIndex   )
	{

		ASSERT( 0 );   //  should not happen

		retByte  =     m_soundDelayBuffer    +      ( biggestIndex  *  BLOCKLOCKEDSIZE   );    //  read the last byte

		m_soundDelayFetchIndex =   0;                       //   and reset the pointer to the beginning of the buffer    3/11
	}
	else		
	{	retByte  =     m_soundDelayBuffer    +      ( m_soundDelayFetchIndex  *  BLOCKLOCKEDSIZE   );

	//	m_soundDelayFetchIndex++;    //  for  FORWARD play	
	}


//	if(   m_soundDelayFetchIndex  >=  m_soundDelayFetchCount   )     //  for  FORWARD play
//		m_soundDelayFetchIndex =   0;


	return  retByte;
}



											////////////////////////////////////////


void    BitSourceStreaming::Erase_DelayBuffer()
{


	if(    m_soundDelayBufferCount  <=  0  )   //  this would mean that we are NOT using a Delay buffer
		return;


	BYTE  *srcByte  =     m_soundDelayBuffer;

	ASSERT(  srcByte  !=  NULL  );




	ASSERT(     m_soundDelayFetchCount  ==    (long)kSoundDelayFetchCount    );    // Is it possible that this can NOT be true????



	if(     m_soundDelayFetchCount  <=    (long)kSoundDelayFetchCount    )
	{
		long   totalBytes =     long(   m_soundDelayFetchCount    *   BLOCKLOCKEDSIZE    ); 

		memset(   srcByte,   0,    totalBytes  );  
	}
	else
	{	ASSERT( 0 );   }


	m_soundDelayFetchIndex =   0;    // *****   NEW,  OK  to reinit the index ????  ******************
}




			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////


BitSourceStreamingWAV::BitSourceStreamingWAV()        //  used by  NAVIGATOR  and  PLAYER
{
}

											////////////////////////////////////////


BitSourceStreamingWAV::~BitSourceStreamingWAV()
{
}

											////////////////////////////////////////


void	  BitSourceStreamingWAV::Clean_All_Resources() 
{

													//  NEW,  ok ???   11/11
//	Release_All_Resources();

//	Release_WaveFormatEx(); 


	if(   m_wavConvert   !=  NULL  )
	{
		m_wavConvert->Cleanup_After_Streaming();

		delete  m_wavConvert; 
		m_wavConvert =   NULL;
	}


	Release_Media_File();


	if(    m_sPitchCalc  !=  NULL    )
	{
		delete   m_sPitchCalc;
		m_sPitchCalc =  NULL;
	}


	Release_WAV_CircularQue();


	m_isInitialized =    false;      
}


											////////////////////////////////////////


bool		BitSourceStreamingWAV::Move_To_DataStart(   CString&   retErrorMesg   )
{

		//   Do NOT enter this function if you do NOT want a File-SEEK,  as with Forward motion     11/11


		//   CALLED by:   ????
		//	   
		//    PROBLEM:  This gets called not only at the start of file,  byt when continue play in middle of file.
		//							...thin that this should do NOTHING for MP3 reading.



	long   bytesInSampleOutput  =    4;    //  always ??? 




	retErrorMesg.Empty();     	//  This moves us to the  'DATA' area  in the file

	if(  ! m_isInitialized   )
	{					
		ASSERT( 0 );
		retErrorMesg  =  "BitSourceStreamingWAV::Move_To_DataStart, bitsource has not been INITIALIZED.."  ;     // the file is probably NOT yet opened, etc.
		return  false;
	}

	if(    m_wavConvert ==  NULL   )
	{
		retErrorMesg =    "BitSourceStreamingWAV::Move_To_DataStart  failed,   filePath is empty or WaveConvert is NULL."  ;
		return  false;
	}




	int    errorCode;        // ***** CAREFULL,  this function does a FILE SEEK,  and goes to the start of the data.   11/11
    if(  (  errorCode=    WavFile_Start_Reading_Data(    &( m_wavConvert->m_mmIO ),     &( m_wavConvert->m_mmChunkInfo ),    
																														&( m_wavConvert->m_chunkInfoParent )    )  )   != 0  )
    {
		//     WANT ????
//
//		if(    m_wavFormat   !=   NULL     )
//		{  GlobalFree(  m_wavFormat   );	   m_wavFormat =  NULL;    }
//		if(    m_mmIO  !=   NULL   )    
//		{  mmioClose(   m_mmIO,   0  );		  m_mmIO =  NULL;       }	       	
		retErrorMesg =     "BitSourceStreamingWAV::Move_To_DataStart  failed,  WavFile_Start_Reading_Data failed." ;
        return  false;
    }



	long   startSampleForPlay  =    m_startPlaySampleIdx;
	long   endSampleForPlay   =    m_endPlaySampleIdx;


	long   startSamplesVirtByteIndex  =    startSampleForPlay  *  bytesInSampleOutput;    // ***** COULD be WRONG :  because of ReSamples  'expansion of samples 




	m_currentByteIdx =    0L;		 //   MOVED this up HERE so no trouble with  NEW rewrite of  Seek_New_DataPosition_Fast()   2/10


	short  directionCode =   BitSource::FORCEiNIT; 



	if(    ! Seek_New_DataPosition_Fast(   startSamplesVirtByteIndex,   directionCode,   retErrorMesg   )    )   
	{
		ASSERT( 0 );
		return  false;
	}


//	long   newFilePos  =    m_wavConvert->Get_Current_FilePos_ReadersMemberVar_UNcompressed_Output_Bytes();

	return  true;
}



											////////////////////////////////////////


bool    BitSourceStreamingWAV::Seek_New_DataPosition_Fast(   long   offsetFromDataStartOutput,   short directionCode,   CString&   retErrorMesg   )
{

		//    offsetFromDataStartOutput -  Is a virtual BYTE offset in OUTPUT-BYTES Format,  as if we were acessing a 44hz  16bit Stereo stream of samples.  2/10 
		//
		//				                      (  offsetFromDataStartOutput was previously divided by SLOW-DOWN speed, so SlowDown speed is not an issue )


	retErrorMesg.Empty();

	if(  ! m_isInitialized   )
	{					
		ASSERT( 0 );
		retErrorMesg  =  "BitSourceStreamingWAV::Seek_New_DataPosition_Fast, bitsource has not been INITIALIZED.."  ;     // the file is probably NOT yet opened, etc.   5/07
		return  false;
	}

	if(    m_wavConvert ==  NULL   )
	{
		retErrorMesg =    " BitSourceStreamingWAV::Seek_New_DataPosition_Fast  failed,   filePath is empty or WaveConvert is NULL."  ;
		return  false;
	}


	long   bytesPerSampleSRC  =     m_wavConvert->m_bytesPerSampleSRCstream;   



								   	//    Now we must COMPENSATE  for different  Sample-Size EXPANSION and for ReSam-ple EXPANSION ( but NOT slowDown ) 

	bool   isStereo          =    m_wavConvert->m_stereoFlagSRCstream;

	long   bitsPerSample =    m_wavConvert->m_wavFormat->wBitsPerSample;    //  do we have  16bit   or  8bit   samples ?



								   	//    Now we must COMPENSATE  for different  Sample-Size EXPANSION and for ReSam-ple EXPANSION  

	long   bitsSampleFactor =   -1;   

	if(        bitsPerSample  ==   16   )
	{
		if(  isStereo  )
			bitsSampleFactor =  1;
		else
			bitsSampleFactor =  2;    //   Clarinet_16hz_mono.wav
	}
	else if(   bitsPerSample  ==   8   )    //  if only 2 bytes per sample( instead of 4), then we should be reading in closer to start of file.
	{
		if(  isStereo  )
			bitsSampleFactor =  2;     //    Highway49_SOLO_8bit_Mono.wav
		else
			bitsSampleFactor =  4;
	}
	else
	{	ASSERT( 0 );   }



	long    reSampExpandedOffsetFromDataStart;   


	if(    m_wavConvert->m_doResamplingStreaming    )   //  if resampling
	{
		ASSERT(    m_wavConvert->m_reSamplerStreaming.m_sampleExpansionRatio   !=   1.0   );
		ASSERT(    m_wavConvert->m_reSamplerStreaming.m_sampleExpansionRatio   !=   0.0  );

			                                           //  ReSample files are SMALLER ( like for 22hz),  and so we must reflect that with this division.

		reSampExpandedOffsetFromDataStart  =    
			         (long)(     (double)offsetFromDataStartOutput  /  (  bitsSampleFactor  *  m_wavConvert->m_reSamplerStreaming.m_sampleExpansionRatio )       );
	}
	else
	   reSampExpandedOffsetFromDataStart  =     offsetFromDataStartOutput  /   bitsSampleFactor;
	



				     //   must make sure that our FilePosition index is on the BOUNDARIES of SAMPLES

	if(     (reSampExpandedOffsetFromDataStart  %   m_wavConvert->m_bytesPerSampleSRCstream)  !=  0   )  
	{
		long  sampleCnt  =    reSampExpandedOffsetFromDataStart  /  bytesPerSampleSRC;

		reSampExpandedOffsetFromDataStart  =    sampleCnt  *  bytesPerSampleSRC; 
	}




	long   offsetFromDataStartFileCoords  =    reSampExpandedOffsetFromDataStart;


	if(     ( offsetFromDataStartFileCoords  %  bytesPerSampleSRC )   !=  0     )
	{
		//    ASSERT( 0 );    // does NOT seem like that this can happen.  If it does, step through make sure there are no bad side effects    2/2/10
		offsetFromDataStartFileCoords  =    (   offsetFromDataStartFileCoords /  bytesPerSampleSRC   )    *    bytesPerSampleSRC;
	}




	bool  forceFileSeek =  false;
	long   offsetFromDataStartPARM  =    offsetFromDataStartOutput;

	
	if(   directionCode  ==   BitSource::FORCEiNIT   )
	{
														//  also land here when doing BACKWARDS play
		forceFileSeek =   true;

		if(    offsetFromDataStartOutput  ==   0    )    //   Going to the BEGINNING of the file.  
		{
			offsetFromDataStartFileCoords =   0;
			m_currentByteIdx                   =   0; 
		}
	}

	else if(   directionCode  ==   BitSource::BACKWARDS   ) 
	{

		forceFileSeek =   true;    // have to keep seekeing continuously for   REVERSE play



		m_currentByteIdx  =    offsetFromDataStartOutput;		//  ****************   WHY?  3/11  [  try to keep in same COORD domain ]
																					    //  If I comment this out it does NOT seem to make a difference  11/6/11
	}

	else if(   directionCode  ==   BitSource::FORWARD   ) 
	{
		//  Do nothing,   it is at the right location,  and   m_currentByteIdx  will increment itself from the calling function.
	}

	else if(   directionCode  ==   BitSource::PREROLLinit   )     //   NEW   3/11
	{
		forceFileSeek =   true;		
	}
	else
	{	ASSERT( 0 );   }



//	long  curFilePos  =    Get_WavFiles_Current_FilePosition(   m_wavConvert->m_mmIO   );



	if(   forceFileSeek   )                             
	{

		if(   ! Seek_To_Virtual_FilePosition_WavFile(     &( m_wavConvert->m_mmIO   ),
														                    &( m_wavConvert->m_mmChunkInfo   ),			   
														                    &( m_wavConvert->m_chunkInfoParent   ),      offsetFromDataStartFileCoords,    retErrorMesg   )   )
		{
			ASSERT( 0 );   // Get here if hit  filesEnd BUTTON   and try reverse play.    2/2/10
			return  false;
		}		



		if(     m_wavConvert->m_doResamplingStreaming    )      //  if    resampling
		{

		//  Remember that   'm_sampleIdxLastBlockLoadNotSlowExpanded'  is in OUTPUT-Bytes Format[ 44hz 16bit ],  so we must EXPAND 
		//  this Physical SOURCE-BYTE Offset(  offsetFromDataStartFileCoords  ),  to one that would exits if the SOURCE wav was actually in OUTPUT Bytes Format.  8/2012



			m_sampleIdxLastBlockLoadNotSlowExpanded  = 
				    ( offsetFromDataStartFileCoords  *  m_wavConvert->m_reSamplerStreaming.m_sampleExpansionRatio         )  /  bytesPerSampleSRC;  // works for  Clarinet_16hz_mono.wav
		//		    ( offsetFromDataStartFileCoords  *  m_wavConvert->m_reSamplerStreaming.m_sampleExpansionRatio  *  bitsSampleFactor  )  /  bytesPerSampleSRC;   // WRONG for   Clarinet_16hz_mono.wav

// **************************************  WHICH is best ??  Clarinet_16hz_mono.wav   says the top   8/3/2012 *****************************************************************



		}
		else
			m_sampleIdxLastBlockLoadNotSlowExpanded  =     offsetFromDataStartFileCoords  /  bytesPerSampleSRC;



	//	TRACE(  "Seek_New_DataPosition_Fast()   Forced SEEK                               %d   bytes   \n" ,   offsetFromDataStartFileCoords   );
	}




//   *** These two are for  Mp3Reader   ...why are they here??    11/11
//
//	long   newVirtFilePos           =     m_wavConvert->Get_Current_FilePos_ReadersMemberVar_UNcompressed_Output_Bytes();    //   thes two will change,  when the ReSample does an expansion
//	long   newVirtSampleIndex  =     m_wavConvert->Get_Current_SampleIndex_ReSamplerExpanded();


	return  true;
}





			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////


BitSourceStreamingMP3::BitSourceStreamingMP3()			 //  used by  NAVIGATOR  and  PLAYER
{
}

											////////////////////////////////////////


BitSourceStreamingMP3::~BitSourceStreamingMP3()
{
}

											////////////////////////////////////////


void	  BitSourceStreamingMP3::Clean_All_Resources() 
{

	//    Release_All_Resources();   


	if(    m_sPitchCalc  !=  NULL    )
	{
		delete   m_sPitchCalc;
		m_sPitchCalc =  NULL;
	}


	if(   m_wavConvert   !=  NULL  )
	{
		m_wavConvert->Cleanup_After_Streaming();

		delete  m_wavConvert; 
		m_wavConvert =   NULL;
	}


	Release_WAV_CircularQue();
}


											////////////////////////////////////////


bool		BitSourceStreamingMP3::Move_To_DataStart(   CString&   retErrorMesg   )
{

		//   CALLED by:  StreamingAudioplayer::StartPlaying()
		//	   
		//    PROBLEM:  This gets called not only at the start of file,  byt when continue play in middle of file.
		//							...thin that this should do NOTHING for MP3 reading.
		//				BUT  WHY  is  m_currentByteIdx set to 0  ??????




	retErrorMesg.Empty();     	//  This moves us to the  'DATA' area  in the file


	if(  ! m_isInitialized   )
	{					
		ASSERT( 0 );
		retErrorMesg  =  "BitSourceStreamingMP3::Move_To_DataStart, bitsource has not been INITIALIZED.."  ;     // the file is probably NOT yet opened, etc.
		return  false;
	}

	if(    m_wavConvert ==  NULL   )
	{
		retErrorMesg =    " BitSourceStreamingMS::Move_To_DataStart  failed,   filePath is empty or WaveConvert is NULL."  ;
		return  false;
	}



	long   bytesInSampleOutput  =    4;    //  always ??? Cause if Output coords



//	long   oldFilePos  =     m_wavConvert->Get_Current_FilePos_ReadersMemberVar_UNcompressed_Output_Bytes();


	long   startSampleForPlay  =    m_startPlaySampleIdx;
	long   endSampleForPlay   =    m_endPlaySampleIdx;


	long   startSamplesVirtByteIndex  =    startSampleForPlay  *  bytesInSampleOutput;    // ***** COULD be WRONG :  because of ReSamples  'expansion of samples 




	m_currentByteIdx =    0L;     //   ******  MOVED this up HERE so no trouble with  NEW rewrite of  Seek_New_DataPosition_Fast()    OK ????   2/10



	short   directionCode =  BitSource::FORCEiNIT; 


	if(    ! Seek_New_DataPosition_Fast(   startSamplesVirtByteIndex,   directionCode,    retErrorMesg   )    )   // This is new in this position.   Is it OK ????    2/4/10
	{
		ASSERT( 0 );
		return  false;
	}


//	long   newFilePos  =    m_wavConvert->Get_Current_FilePos_ReadersMemberVar_UNcompressed_Output_Bytes();

	return  true;
}



											////////////////////////////////////////


bool    BitSourceStreamingMP3::Seek_New_DataPosition_Fast(   long   offsetFromDataStartOutput,   short directionCode,     CString&   retErrorMesg   )
{


		//   CALLED BY:    BitSourceStreamingMP3::Move_To_DataStart(),   Fetch_Streaming_Samples_Direct_PPlayer(),  
		//
		//			              EventMan::PreRoll_NoteDetect(),    EventMan::PreRoll_NoteDetect_DEBUG()   				



		//    offsetFromDataStartOutput -  Is a virtual BYTE offset in Output-Coords,  as if we were acessing a 44hz  16bit Stereo stream of samples.  2/10 
		//				                                 (  offsetFromDataStartOutput was previously divided by SLOW-DOWN speed, so SlowDown speed is not an issue )


	retErrorMesg.Empty();


	if(  ! m_isInitialized   )
	{					
		ASSERT( 0 );
		retErrorMesg  =  "BitSourceStreamingMP3::Seek_New_DataPosition_Fast, bitsource has not been INITIALIZED.."  ;     // the file is probably NOT yet opened, etc.   5/07
		return  false;
	}

	if(    m_wavConvert ==  NULL   )
	{
		retErrorMesg =    " BitSourceStreamingMP3::Seek_New_DataPosition_Fast  failed,   filePath is empty or WaveConvert is NULL."  ;
		return  false;
	}



	long   bytesPerSampleSRC  =     m_wavConvert->m_bytesPerSampleSRCstream;   


//	long   oldVirtFilePos  =      m_wavConvert->Get_Current_FilePos_ReadersMemberVar_UNcompressed_Output_Bytes();   // unreliable cause of buffer ???



								   	//    Now we must COMPENSATE  for different  Sample-Size EXPANSION and for ReSam-ple EXPANSION ( but NOT slowDown ) 


	long    reSampExpandedOffsetFromDataStart =    offsetFromDataStartOutput;   



																		//  Must CORRECT for Resampling EXPANSION

	if(    m_wavConvert->m_doResamplingStreaming    )   //  if resampling
	{
		ASSERT(    m_wavConvert->m_reSamplerStreaming.m_sampleExpansionRatio   !=   1.0   );
		ASSERT(    m_wavConvert->m_reSamplerStreaming.m_sampleExpansionRatio   !=   0.0  );

// *****************  AM I calcing this wrong???   and that is why I do all these Seeks for   Talkin.mp3     ???   ******************

		reSampExpandedOffsetFromDataStart  =     (long)(     (double)offsetFromDataStartOutput  /  m_wavConvert->m_reSamplerStreaming.m_sampleExpansionRatio   );
	}


																		//  And must CORRECT for Mono EXPANSION

	if(    ! m_wavConvert->m_stereoFlagSRCstream    )
		reSampExpandedOffsetFromDataStart  =   reSampExpandedOffsetFromDataStart   /  2;


	//  Note:  do NOT have to convert for  SLOW-DOWN,  than was previously done to  offsetFromDataStartOutput





	long   offsetFromDataStartFileCoords  =    reSampExpandedOffsetFromDataStart;


	if(     ( offsetFromDataStartFileCoords  %  bytesPerSampleSRC )   !=  0     )
	{
		//    ASSERT( 0 );    // does NOT seem like that this can happen.  If it does, step through make sure there are no bad side effects    2/2/10
		offsetFromDataStartFileCoords  =    (   offsetFromDataStartFileCoords /  bytesPerSampleSRC   )    *    bytesPerSampleSRC;
	}




	bool  forceFileSeek =  false;
	long  retNewFilePos =   -1;

	long   offsetFromDataStartPARM  =    offsetFromDataStartOutput;



	
	if(   directionCode  ==   BitSource::FORCEiNIT   )
	{

		forceFileSeek =   true;

		if(    offsetFromDataStartOutput  ==   0    )    //   Going to the BEGINNING of the file.  
		{
			offsetFromDataStartFileCoords =   0;
			m_currentByteIdx                   =   0; 
		}
	}

	else if(   directionCode  ==   BitSource::BACKWARDS   ) 
	{

		forceFileSeek =   true;    // have to keep seekeing continuously for   REVERSE play

		m_currentByteIdx  =    offsetFromDataStartOutput;		//  try to keep in same COORD domain
	}

	else if(   directionCode  ==   BitSource::FORWARD   ) 
	{
		//  Do nothing,   it is at the right location,  and   m_currentByteIdx  will increment itself from the calling function.
	}

	else if(   directionCode  ==   BitSource::PREROLLinit   )     //   NEW   3/11
	{
		forceFileSeek =   true;		
	}
	else
	{	ASSERT( 0 );   }




	if(   forceFileSeek   )                             
	{


//		if(            directionCode  ==   BitSource::PREROLLinit   
//			    &&   forRecording    )
			m_wavConvert->Set_MP3reader_Timing_Correction(  4  );   // ****  Now using this ALL the time for all operations.  OK???    7/31/2012  ********



		if(    ! m_wavConvert->Seek_to_BytePosition_in_OutputCoords_for_StreamingMP3(    offsetFromDataStartFileCoords,   retNewFilePos,   retErrorMesg  )    )
		{
			ASSERT( 0 );   // Get here if hit  filesEnd BUTTON   and try reverse play.    2/2/10  
			return  false;
		}		


//		m_wavConvert->Set_MP3reader_Timing_Correction(  0  );    //    RESET  




		if(     m_wavConvert->m_doResamplingStreaming    )      //  if    resampling
		{

					//  Remember that   'm_sampleIdxLastBlockLoadNotSlowExpanded'  is in OUTPUT-Bytes[ 44hz 16bit ],  so we must EXPAND 
					//  this Physical SOURCE-BYTE Offset(  offsetFromDataStartFileCoords  ),  to one that would exits if  the SRC was in  OUTPUT-Bytes.  8/2012

			m_sampleIdxLastBlockLoadNotSlowExpanded  = 
				    ( offsetFromDataStartFileCoords  *  m_wavConvert->m_reSamplerStreaming.m_sampleExpansionRatio         )  /  bytesPerSampleSRC;
		}
		else
			m_sampleIdxLastBlockLoadNotSlowExpanded  =     offsetFromDataStartFileCoords  /  bytesPerSampleSRC;
	}


//	long   newVirtFilePos           =     m_wavConvert->Get_Current_FilePos_ReadersMemberVar_UNcompressed_Output_Bytes();    //   thes two will change,  when the ReSample does an expansion
//	long   newVirtSampleIndex  =     m_wavConvert->Get_Current_SampleIndex_ReSamplerExpanded();

	return  true;
}




			//////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////


SourceAdminAudioMS::SourceAdminAudioMS()
{
}

									/////////////////////////////////////////////////////

SourceAdminAudioMS::~SourceAdminAudioMS()
{
}

									/////////////////////////////////////////////////////


bool	   SourceAdminAudioMS::Alloc_BitSource(    short  bitSourceKindCode,   BitSource  **retBitSource,   CString&  retErrorMesg   )
{

	retErrorMesg.Empty();

	*retBitSource =  NULL;


	BitSource   *bitSourceStreaming =  NULL;



	switch(   bitSourceKindCode   )                
	{

		case   BitSource::SOURCEmp3:   
			 
			bitSourceStreaming   =     new   BitSourceStreamingMP3();    //  small memory usage

				bitSourceStreaming->m_bitSourceKind =   	BitSource::SOURCEmp3;

			m_bitSources.Add_Tail(   *bitSourceStreaming   ); 
		break;


		case   BitSource::SOURCEsTREAMwav:   //  NEW,  is like  BitSourceStreamingMS  but can automatically do ReSampling and file conversion
			 
			bitSourceStreaming   =     new   BitSourceStreamingWAV();    //  small memory usage

				bitSourceStreaming->m_bitSourceKind =   	BitSource::SOURCEsTREAMwav;

			m_bitSources.Add_Tail(   *bitSourceStreaming   ); 
		break;





		case   BitSource::SOURCEwav:   
			 	
			bitSourceStreaming   =     new   BitSourceStreamingMS();    //  small memory usage

				bitSourceStreaming->m_bitSourceKind =   	BitSource::SOURCEwav;

			m_bitSources.Add_Tail(   *bitSourceStreaming   ); 
		break;



		case   BitSource::LEAD:   
			 
			bitSourceStreaming   =     new   BitSourceStreamingMS();    //  small memory usage

				bitSourceStreaming->m_bitSourceKind =   	BitSource::LEAD;

			m_bitSources.Add_Tail(   *bitSourceStreaming   ); 
		break;
		 


		case   BitSource::BACKGROUND:   
			 
			bitSourceStreaming   =     new   BitSourceStreamingMS();    //  small memory usage

				bitSourceStreaming->m_bitSourceKind =   	BitSource::BACKGROUND;

			m_bitSources.Add_Tail(   *bitSourceStreaming   ); 
		break;
		 


		case   BitSource::MODIFIEDwAV:   
			 
			bitSourceStreaming   =     new   BitSourceStreamingMS();    //  small memory usage

				bitSourceStreaming->m_bitSourceKind =   	BitSource::MODIFIEDwAV;

			m_bitSources.Add_Tail(   *bitSourceStreaming   ); 
		break;
		 

		default:  
			 retErrorMesg =  "SourceAdminAudioMS::Alloc_BitSource  FAILED,  missing  bitSourceKindCode  case."  ;
			 return  false;
		 break;
	  }


	*retBitSource =    bitSourceStreaming;

	return  true;
}


