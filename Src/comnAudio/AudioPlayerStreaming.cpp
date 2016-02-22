/////////////////////////////////////////////////////////////////////////////
//
//  AudioPlayerStreaming.cpp   -   
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

#include  "..\ComnGrafix\CommonGrafix.h"   


#include "..\comnInterface\DetectorAbstracts.h"    // gives us an ABSTRACY definition of  BitSource, SourceAdmin,  etc


#include  "..\ComnGrafix\OffMap.h"  
#include  "..\comnGrafix\TransformMap.h"

#include   "..\ComnGrafix\AnimeStream.h"




////////////////////////////////////

#include  <mmsystem.h>   // Is good on Sparky  ( ProgramFiles\Microsoft Visual Studio\VC\PlatformSDK\Include )


#include  "dsoundJM.h"          //  I copied it in, bigger than the VC++ version



#include  "WaveJP.h"

  

#include  "..\comnAudio\BitSourceAudio.h"



//////////////////////////////////////////////////         ...my  AUDIO 
#include  "sndSample.h"

#include  "PlayBuffer.h"

////////////////////////////////////////////////// 


#include  "..\ComnAudio\CalcNote.h"




#include  "EventMan.h"  


#include  "AudioPlayer.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/***
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
***/
////////////////////////////////////////////////////////////////////////////////////////////////////////////



extern    long   mySinTab[],   myCosTab[];     //  *****CAREFUL if BIGGER !!******** PC:  9800




int   glbTravDir=  1,   glbYpos = 0;   //   TEMP ******





EventMan&   Get_EventMan();        //  This class need to acess EventMan in many ways.



								////////////////////////////////////////////////////////////////////////////


//  bool            Set_ScrollCtrl_Horz_Position_ActvViewj_GLB(    double  posInPercent,    CString&  retErrorMesg   );


double		   Get_Hertz_Freq_from_MidiNumber(  short  midiNum  );

long			Get_Best_Rational_Number_SevenTeenNumer(   long  freq,    long *numerRtn,   long  sampleRate   );

short			Update_TrigTable(   double  angFreq,    long  numSamps,    TRIGTABLEfrc *tble,   bool  doWindow  );

unsigned short    Get_Rand( void );   




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

AudioPlayer::AudioPlayer(   EventMan&  eventMan,      UniApplication&   theApp    )    
				            :   m_waveMan( eventMan ),     m_theApp( theApp ),      m_shortPlayBuffer(  (4L * 4096),   2,   eventMan )
{									
	
											//  This class need to acess EventMan in many ways.

	m_slowPlayStep =    4096L;  


	m_playSpeedFlt =   1.0;       // will not affect spped if changed here

	m_bytesPerSample =  4;   // **** HARDWIRED ,  should be read in from  FILE ****    for  16-bit  stereo this is 4 (  2 bytes per 16bit sample  x  2 stereo channels )




	m_infcDSoundBuffer   =  NULL;

	m_pwfx  =     NULL;


	m_bufferIsOK =  false;

	m_curSubFetch =  0;


	m_dataFetchCount     =   0;        
	m_srcBytesProcessed =   0; 



	m_startSample =  -1;   
	m_endSample  =  -1; 


	m_playingBackward  =   false;

	m_doneFetchingPlayBytes =   false;

	m_isPlaying  =   false;


	m_playingCurNotesEndOffset  =  -1;


    m_stereoChannelCodeAddr =    NULL;    // pointer to a soundMan variable



	m_onsetMaskAddrLeft   =  NULL;
	m_onsetMaskAddrRight =  NULL;
	m_onsetMaskAddrCenter =  NULL;


	m_lastPlayedSampleAddr =  NULL;    //	m_lastPlayedSample =  -1;  //  OK init ???

	m_lastNotePlaySample =  0;   
	m_firstNotePlaySample =  0;


																				//   Init secondary buffer for play of  synthesized notes
	long   numSeconds =   20;	 // **ADJUST**

	long   totalBytesSynthesizedBuffer  =    
					(   m_bitSourceOnTheFlySynthed.m_sampleRate *  (long)m_bytesPerSample  )   *   numSeconds;    




	if(     ! m_bitSourceOnTheFlySynthed.Create_Secondary_Buffer(   totalBytesSynthesizedBuffer  )     )   // For play of synthesized notes
	{
		AfxMessageBox(   "AudioPlayer  could NOT create secondary buffer.   "  );
	}


	m_animationSamplesDelay  =   2L;      //   Sync with Soundman var  



	m_auditionCode  =     AnimeStream::NORMAL;

	m_playMode       =     AudioPlayer::NORMALpLAY;

	m_sampleCountInViewjsWidth  =   -118;



	m_prevStartSamplePlay  =   -1;			   //   NEW  state vars  for    'Previous Play's Positions'
	m_prevEndSamplePlay  =   -1;	

    m_lastWindowPlayStartSample =   -1;    // ****  NEW,   


	m_prevBackwardDirectionPlay  =    false;


	m_wasPlayingBackwardsPrev =  false;


	m_muteReversePlayAudio =  false;
}



											////////////////////////////////////////


AudioPlayer::~AudioPlayer()
{

	if(   m_infcDSoundBuffer  !=  NULL   )      //  Free the  DirectSound buffer COM interface
    {
        m_infcDSoundBuffer->Release();
        m_infcDSoundBuffer =   NULL;
	}
}



											////////////////////////////////////////


long	  AudioPlayer::Get_Biggest_SampleIndex_With_SpeedExpansion()
{																				// **** CAREFULL, has the  expansion for slowDown speeds

			                               // ****  Careful,  could get interer overflow. 


	double    biggestLongFlt   =   2147483000.0;         //   2147483647  



	/***
	if(   m_bytesPerSample  !=  4    )      //  see   StreamingAudioplayer::Get_Biggest_SampleIndex_With_SpeedExpansion()   below,  think this is always the case. 
	{
		ASSERT( 0 );
		return  -2;
	}

	long  totalPlayBytes  =    Get_BitSource().Calc_Files_Total_Output_Bytes_With_SpeedExpansion();   	//   has the  expansion for slowDown speeds

	long      idx    =      totalPlayBytes   /    m_bytesPerSample; 
	****/

	long  idx =  -1;

	long         totalSamples  =     Get_BitSource().Get_Files_Total_Samples_No_SpeedExpansion();  

	ASSERT(   totalSamples >=  0    );



	if(   m_playSpeedFlt   >  1.0   )   
	{

				//   ******   OLD PitchScope Code NEVER goes here,  it does NOT represent   playSpeed this way  3/2012


		double   sampsExpanded =     (double)totalSamples   *    m_playSpeedFlt;


		if(    sampsExpanded   >   biggestLongFlt   )   // **********  test for  Integer OVERFLOW  **********************************
		{
			ASSERT( 0 );

			idx =   (long)biggestLongFlt;   //   ***********    FIX this,   sloppy  **********************
		}
		else
			idx =    (long)sampsExpanded;
	}
	else
		idx =   totalSamples;



	return   idx;
}


											////////////////////////////////////////


long	  AudioPlayer::Get_Biggest_SampleIndex_No_SpeedExpansion()
{																				// **** CAREFUL, does NOT have the slowDown expansion


		//    Careful...     CALLED by MANY   different function.


	long     totalSamples  =     Get_BitSource().Get_Files_Total_Samples_No_SpeedExpansion();   //    does NOT have the slowDown expansion
	return   totalSamples;
}


											////////////////////////////////////////


long    AudioPlayer::Get_LastPlayed_SampleIdx()                                    
{   

	if(    m_lastPlayedSampleAddr  ==   NULL    )		 //  resides in   UniBasic  object
	{
		ASSERT( 0 );
		return  0;
	}

	long      idx  =    *m_lastPlayedSampleAddr;
	return   idx;  
}


											////////////////////////////////////////


long     AudioPlayer::Get_Last_SingleNotePlay_SampleIdx()
{
																 //  Only for  'Single-Note Play'  BUTTONS
	return   m_lastNotePlaySample;
}


											////////////////////////////////////////


void	AudioPlayer::Set_LastPlayed_SampleIdx(   long   sampleIdx   )    
{   

		//    CALLED by a lot of functions,       ALSO  moves the  ScrollBar

	    //   This is also UPDATED by    PostRoll(),   and   PreRoll_NoteDetect()



	if(   m_lastPlayedSampleAddr  ==   NULL   )
	{
		ASSERT( 0 );
	}

	*m_lastPlayedSampleAddr  =    sampleIdx;
}


											////////////////////////////////////////


void   AudioPlayer::Set_Anime_Maps(   TransformMap   **onsetMaskAddrLeft,     
														  TransformMap   **onsetMaskAddrRight,
														  TransformMap   **onsetMaskAddrCenter   )
{

	m_onsetMaskAddrLeft      =      onsetMaskAddrLeft;
	m_onsetMaskAddrRight    =      onsetMaskAddrRight;

	m_onsetMaskAddrCenter  =      onsetMaskAddrCenter;
}


											////////////////////////////////////////


short    AudioPlayer::Get_StereoChannelCode()
{
	
	if(    m_stereoChannelCodeAddr ==   NULL    )
	{
		ASSERT( 0 );    // should have been assigned at construction
		return  true;
	}

	return  *m_stereoChannelCodeAddr;
}


											////////////////////////////////////////


void	AudioPlayer::Set_Focus_StereoChannel_Addr(   short  *soundmanVarAddr  )  //   bool  *soundmanVarAddr   )   
{  

//	m_detectFromLeftStereoAddr =    soundmanVarAddr;  

	m_stereoChannelCodeAddr =      soundmanVarAddr;  
}


											////////////////////////////////////////


bool    AudioPlayer::Is_Byte_In_ScalepitchSubj(   long  sampleIdx   ) 
{


	// **** RENAME:   Really is    Is_There_A_Rest_at_CurSample_Position()    ...check AnimeMask for a valid curMidiPitch ****



		// Should work with a STREAMING bitsource as well as static  

  	short   val,  gr,  bl;
	TransformMap   *onsetMask =   NULL;

	short  channelCode =    Get_StereoChannelCode(); 



	if(   channelCode  ==     TransformMap::LEFTj      )   
	{

		if(   m_onsetMaskAddrLeft  ==   NULL   )
		{
			ASSERT( 0 );    // should have been assigned during construction
			return  true;            // default,  will allow the sound to play
		}

		onsetMask =    *m_onsetMaskAddrLeft;
	}

	else 	if(   channelCode  ==     TransformMap::RIGHTj      )   
	{  
		if(   m_onsetMaskAddrRight  ==   NULL   )
		{
			ASSERT( 0 );  
			return  true;   
		}

		onsetMask =    *m_onsetMaskAddrRight;
	}

	else 	if(   channelCode  ==     TransformMap::CENTEREDj      )   
	{  
		if(   m_onsetMaskAddrCenter  ==   NULL   )
		{
			ASSERT( 0 );  
			return  true;   
		}

		onsetMask =    *m_onsetMaskAddrCenter;
	}
	else
		ASSERT( 0 );



	if(   onsetMask  ==   NULL   )
		return  true;       // default,  will allow the sound to play   ( ok,  just no map was made )




	long    chunkSize  =    onsetMask->m_horzScaleDown;

	short   x   =       (short)(    sampleIdx  /  chunkSize   );


	onsetMask->Read_Pixel(   x, 0,     &val,  &gr,&bl    );     //  1st read the CENTER pixel,  if BAD skip to the next iteration


	if(   val  ==   255  )
		return  false;
	else
	{  
		//  ASSERT(   val >= 0    &&     val < 12   );   // **** BUG,  now I have Midi numbers in the mask   ..bigger range. 

		return  true;
	}


	return  false;
}


											////////////////////////////////////////


void   AudioPlayer::StopPlaying_Hardware()
{


	if(        m_infcDSoundBuffer ==  NULL
		||   !IsOK()      )
	{   
		ASSERT( false );	
		return; 	
	}

	long   lastPlayedSampleIdx =   Get_LastPlayed_SampleIdx();

//	TRACE(  "\n  ...STOP Playing   m_sampleIdxCurPlayingBlockWrite  %d,    lastPlayedSamp  %d    \n\n" ,    eventMan.m_sampleIdxCurPlayingBlockWrite,   lastPlayedSampleIdx   );


    m_infcDSoundBuffer->Stop();   


//	Fill_Buffer_With_Silence();               //  ***** BAD for Navigator, this will destroy the  WAV CircQue STORAGE,  and give us gaps of silence.     12/11
	FillBufferWithSilence(  m_infcDSoundBuffer  );  	//   NEW here  3/02.      clear out old sound from LAST PLAY  


																//    ReSet   'some'  FLAGS   ( careful!!!  )
	m_isPlaying  =    false;

    
	if(    m_playingBackward   )
		m_wasPlayingBackwardsPrev =   true;       //  new,  be careful.
	else
		m_wasPlayingBackwardsPrev =   false;
}



					////////////////////////////////////////////////////////


void    AudioPlayer::Fill_Buffer_With_Silence()
{
	ASSERT( 0 );    // *********   Ever called ????     12/10/11

	FillBufferWithSilence(  m_infcDSoundBuffer  );
}



											////////////////////////////////////////


BOOL    AudioPlayer::FillBufferWithSilence(   LPDIRECTSOUNDBUFFER   soundBuffer   )
{
															//    Write silence to entire buffer 
    WAVEFORMATEX   wfx;
    DWORD               dwSizeWritten;
    PBYTE     memBlock;							
    DWORD   cb1;


    if(    FAILED(    soundBuffer->GetFormat(   &wfx,   sizeof( WAVEFORMATEX ),   &dwSizeWritten  )     ) )
        return   FALSE;


    if(    SUCCEEDED(     soundBuffer->Lock(    0,  
																    0, 
								               ( LPVOID* )&memBlock,
													            &cb1,  //  the COUNT in bytes
													              NULL,   NULL,   DSBLOCK_ENTIREBUFFER  )       ) )
    {
        FillMemory(   memBlock,   cb1,      ( wfx.wBitsPerSample == 8 )   ?   128 : 0     );

        soundBuffer->Unlock(   memBlock,   cb1,   NULL,  0   );

        return  TRUE;
    }

    return  FALSE;
}  



											////////////////////////////////////////


void    AudioPlayer::Initialize_for_Playing_Section() 
{

			//  NEW   3/11     Need to better clarity what these weird variable do during play.

	m_dataFetchCount      =    0;
	m_srcBytesProcessed  =   0;
	m_curSubFetch  =   0;

	m_doneFetchingPlayBytes =    false;     //  init switch that can stop playing   *****  NEW,  want this 
}




											////////////////////////////////////////


bool    AudioPlayer::Set_PlaySpeed(   double  speedSlowRatio   )    
{   

	if(       speedSlowRatio    <=    0.0   
		||   speedSlowRatio    >       12.0	   )		//   ***** WHY  12  ??? ****    1/2003
	{
		ASSERT( 0 );
		m_playSpeedFlt =  1.0;
		return  false;
	}	
	

	m_playSpeedFlt =   speedSlowRatio;  

	return  true;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


StreamingAudioplayer::StreamingAudioplayer(   EventMan&  eventMan,   BitSourceStreaming  *bitSource,    UniApplication&  theApp,     bool  doRealtimePitchDetection    )  	
													:  AudioPlayer( eventMan,  theApp ),   m_bitSource( bitSource ),      m_doRealtimePitchDetection(  doRealtimePitchDetection  )
{
	
										         //   400 is default,  can change with  EventMan::Set_VolumeBoost_Factor()
														//          250[  Bad sign, no overbrite, weak on Texas Flood ]    350[ much overBrite in Highway 49 ]

	m_playSpeedFlt =  1.0;    


	m_rightChannelPercent =   50;     //  50:    50 percent left, and 50 percent right



	m_speedZoneBuffer  =    new    BYTE[  SPEEDZONESIZE   ];		
	ASSERT(  m_speedZoneBuffer   );


	m_backwardsBits       =    new    BYTE[   BLOCKLOCKEDSIZE   ];
	m_backwardsBitsSize =    BLOCKLOCKEDSIZE;  
	ASSERT(  m_backwardsBits   );
}



											////////////////////////////////////////


StreamingAudioplayer::~StreamingAudioplayer()
{  

	if(    m_speedZoneBuffer   !=   NULL   )
		delete   m_speedZoneBuffer;


	if(   m_backwardsBits  !=   NULL  )
		delete   m_backwardsBits;
}



					////////////////////////////////////////////////////////


BitSourceStreaming*     StreamingAudioplayer::Get_BitSourceStreaming(   CString&  retErrorMesg   )
{

	if(   m_bitSource  ==  NULL  )
	{  
		retErrorMesg =   "StreamingAudioplayer::Get_BitSourceStreaming  FAILED,  m_bitSource is NULL." ;
		return  NULL;
	}

	return   m_bitSource;
}


					////////////////////////////////////////////////////////


SPitchCalc*     StreamingAudioplayer::Get_SPitchCalc()      //    {   return m_sPitchCalc;   } 
{

	if(   m_bitSource  ==  NULL  )
	{  
		ASSERT( 0 );
	//	retErrorMesg =   "StreamingAudioplayer::Get_BitSourceStreaming  FAILED,  m_bitSource is NULL." ;
		return  NULL;
	}

	SPitchCalc   *retSCalc  =   m_bitSource->m_sPitchCalc;
	ASSERT(      retSCalc  );

	return  retSCalc;
}



								////////////////////////////////////////


HMMIO*  	StreamingAudioplayer::Get_MediaFile()
{

	CString  retErrorMesg;   


	if(   m_bitSource  ==   NULL  )
	{
		ASSERT( 0 );
		return  NULL;
	}


	BitSourceStreamingMS   *bitsStreamMS  =     Get_BitSourceStreamingMS(  retErrorMesg   );
	if(                                  bitsStreamMS  ==   NULL  )
	{  ASSERT( 0 );
		return  NULL;
	}
	else
		return    &(   bitsStreamMS->m_mmIO  );
}



								////////////////////////////////////////


bool 	  StreamingAudioplayer::Set_BitSource(   BitSource  *bitSource,     CString&  retErrorMesg    )
{

	retErrorMesg.Empty();


	if(  bitSource ==  NULL  )
	{
		retErrorMesg =  "StreamingAudioplayer::Set_BitSource FAILED,  bitSource is  NULL."  ;
		return false;
	}



	/****
	BitSourceStreamingMS   *bitSourceStreaming     =       dynamic_cast< BitSourceStreamingMS* >(  bitSource  );  
	if(   bitSourceStreaming  ==  NULL   )
	{  
		retErrorMesg =   "StreamingAudioplayer::Set_BitSource  FAILED,  could not dcast to BitSourceStreamingMS." ;
		return  false;
	}

	m_bitSource  =   bitSourceStreaming;
	****/
	BitSourceStreaming   *bitSourceStreaming     =       dynamic_cast< BitSourceStreaming* >(  bitSource  );  
	if(   bitSourceStreaming  ==  NULL   )
	{  
		retErrorMesg =   "StreamingAudioplayer::Set_BitSource  FAILED,  could not dcast to BitSourceStreaming." ;
		return  false;
	}

	m_bitSource  =   bitSourceStreaming;	


	return true;
}




											////////////////////////////////////////


bool    StreamingAudioplayer::ExamineWavFile(   CString&   filePath,   CString&  retErrorMesg   )
{

	bool      succs =   Initialize_Compatable_Buffer(   filePath,    retErrorMesg   );    //  this sets up the DSound PlayBuffer, and gets WAVFORMATEX info
	return   succs;
}



											////////////////////////////////////////


bool    StreamingAudioplayer::Initialize_Compatable_Buffer(   CString&   filePath,   CString&  retErrorMesg   )
{

												//   Create a  SoundBUFFER  that is compatible with the FILE to be loaded
																					

	int    dwInputWavIndex  =   15;     // **** HARDWIRED   OK ???   ****


	EventMan&   eventMan  =   Get_EventMan();   


	retErrorMesg.Empty();

  
	if(    filePath.IsEmpty()    )
	{
		retErrorMesg =   "StreamingAudioplayer::Initialize_Compatable_Buffer  FAILED,     filePath is empty."  ;
		return  false;
	}

	if(   m_bitSource ==  NULL  )					
	{
		retErrorMesg =   "StreamingAudioplayer::Initialize_Compatable_Buffer  FAILED,   m_bitSource is NULL."  ;
		return  false;
	}



	BitSourceStreaming	 *bitSourceStreaming  =   NULL;
	BitSourceAudioMS       *bitSourceAudioMS    =    NULL;


	BitSourceStreamingMS       *bitSourceStreamingMS =     dynamic_cast<  BitSourceStreamingMS*  >(   m_bitSource  );   	 // old,  for PitchScope
	if(   bitSourceStreamingMS ==  NULL  )
	{
		bitSourceAudioMS =      dynamic_cast<  BitSourceAudioMS*  >(   m_bitSource  );    //  BitSourceStatic    for STATIC PLAYED
		if(   bitSourceAudioMS  !=  NULL   )
		{
			ASSERT( 0 );    //  Do NOT think we need this because there is a SEPARATE   Initialize_Compatable_Buffer()  for AudioStaticPlayer
			return  false;
		}
		else
		{	bitSourceStreaming   =        dynamic_cast<  BitSourceStreaming*  >(   m_bitSource  );   // BitSourceStreamingWAV,  and   BitSourceStreamingMP3    	
			if(   bitSourceStreaming  ==  NULL   )
			{
				retErrorMesg = "StreamingAudioplayer::Initialize_Compatable_Buffer  FAILED,  could not do a DCAST"  ;
				return  false;
			}
		}
	}     //   if(   bitSourceStreamingMS ==  NULL 





	if(    m_infcDSoundBuffer   !=  NULL    )    //  free the OLD buffer.
    {
        m_infcDSoundBuffer->Release();
        m_infcDSoundBuffer =   NULL;
	}

	m_bufferIsOK =   false;    //  INIT for fail...





							    //   Create a  secondary buffer   able to hold 2 seconds( now is 1/2 sec )  of data 

	DSBUFFERDESC   dsbdesc;

    memset(    &dsbdesc,    0,   sizeof( DSBUFFERDESC )    ); 

    dsbdesc.dwSize   =     sizeof(  DSBUFFERDESC  ); 

    dsbdesc.dwFlags  =    DSBCAPS_GETCURRENTPOSITION2   |   DSBCAPS_GLOBALFOCUS  
		                                                            |  DSBCAPS_CTRLPAN  |   DSBCAPS_CTRLPOSITIONNOTIFY;    //  NEED   'DSBCAPS_CTRLPOSITIONNOTIFY'   3/12

													   //           |  DSBCAPS_CTRLPAN ;    ****FAILS,  need DSBCAPS_CTRLPOSITIONNOTIFY.    3/4/12   **************




//	dsbdesc.dwFlags  =    DSBCAPS_STATIC  |  DSBCAPS_CTRLPOSITIONNOTIFY ;   //    in   "C:\Program Files\Microsoft Platform SDK\Samples\Multimedia\DSound\Src\playsound"
// **** USE better FLAGS ??? ********************  3/03 
	



	if(   bitSourceStreamingMS  !=  NULL   )  
	{																			//  get here form  PitchScope 

		if(    m_pwfx  ==   NULL   )				//   NOW  must copy   WavFormatEx    from bitsource...
		{

						//   what if the loading file plays at a different freq than 44,100.  Will this init of the player give us trouble ??  2/16/10

			if(    bitSourceStreamingMS->m_wavFormat   ==  NULL   )
				ASSERT( 0 );
			else
			{  WORD   cbExtraAlloc;     // Extra bytes for waveformatex 
				

				if(    bitSourceStreamingMS->m_wavFormat->wFormatTag  ==  WAVE_FORMAT_PCM    )
							 cbExtraAlloc = 0;        
				else
					ASSERT( 0 );    //   SEE :    WavFile_Open()
				
				

				if(   (  m_pwfx =    ( tWAVEFORMATEX* )GlobalAlloc(   GMEM_FIXED,    sizeof( WAVEFORMATEX ) + cbExtraAlloc  )     )   == NULL   )
					return  false;      //    goto    ERROR_READING_WAVE;



				*m_pwfx  =     *(   bitSourceStreamingMS->m_wavFormat   );
			}
		}

		dsbdesc.lpwfxFormat  =   m_pwfx;   
	}

	else    //  if(   bitSourceStreamingMS  ==  NULL   )   
	{  		
		ASSERT(  bitSourceStreaming  !=  NULL  );    //  get here  from  most   BitSourceStreaming  {  BitSourceStreamingWAV,  and   BitSourceStreamingMP3   } 


		if(    m_pwfx  ==   NULL   )			
		{													//  allocate a  WAVEFORMATEX   with generic settings  for   44hz  16bit stereo
			WORD   cbExtraAlloc = 0;   

			if(   (  m_pwfx =    ( tWAVEFORMATEX* )GlobalAlloc(   GMEM_FIXED,    sizeof( WAVEFORMATEX ) + cbExtraAlloc  )     )   == NULL   )
				return  false;      //    goto    ERROR_READING_WAVE;

			m_pwfx->wFormatTag =  1;
			m_pwfx->nChannels =  2;
			m_pwfx->nSamplesPerSec =  44100;
			m_pwfx->nAvgBytesPerSec =  176400;
			m_pwfx->nBlockAlign =   4;
			m_pwfx->wBitsPerSample =  16;

			m_pwfx->cbSize  =   0;    // *******  CAREFUL...   should it ALWAYS be zero  ????
		}

		dsbdesc.lpwfxFormat  =   m_pwfx;     //  ************   BIG,  I will have to fill this out myself for MP3	  
	}

	



    dsbdesc.dwBufferBytes =   TWOSECBYTES;    //   1/2  sec       Get   sample data every 1/4 sec

    m_dwMidBuffer  =    dsbdesc.dwBufferBytes  / 2;       //  was:   TWOSECBYTES / 2; 





    HRESULT  hr;
    if(   FAILED(   hr =   m_waveMan.m_infcDSound->CreateSoundBuffer(   &dsbdesc,   &m_infcDSoundBuffer,   NULL   )    )    )
    {

		retErrorMesg =   "StreamingAudioplayer::Initialize_Compatable_Buffer  FAILED,   CreateSoundBuffer"  ;


		if(   bitSourceStreamingMS  !=  NULL   )
		{
			if(   Get_MediaFile()   !=   NULL   )     
			{
				WavFile_Close_ReadingFile(   Get_MediaFile(),   &m_pwfx    );
			}
		}
		else
		{	m_bitSource->Release_All_Resources();   }    	//  should  also  CLOSE the mp3 file 		
		 
        return  false; 
    }

												//  Assign up notification positions in capture buffer. These occur halfway 
												//  through the buffer, at the end of the buffer, and when capture stops.


	DWORD   numberOfEvents  =     eventMan.m_numberOfNotificationEvents  - 1;


	DWORD   posStep =     TWOSECBYTES  /   numberOfEvents;



																    //    Setup the notification position OFFSES for   'PLAYING'  the buffer.


    for(   int i =0;     i <  (int)numberOfEvents;     i++   )
    {
		eventMan.m_positionNotify[  i  ].dwOffset  =    (  (DWORD)i   *   posStep  );      //  (  (DWORD)(i+1)   *    posStep  )   - 1;
	}
		

	eventMan.m_positionNotify[   numberOfEvents   ].dwOffset  =    DSBPN_OFFSETSTOP;    //  Need this FINAL  Notifyobj   ...even with Other Record Notification objects





	//////////////////////////////////////////////////////    
	LPDIRECTSOUNDNOTIFY   dsNotify;					//  Get interface for  'PLAY'-buffer   NOTIFICATION
											                 
    if(   FAILED(   hr =   m_infcDSoundBuffer->QueryInterface(   IID_IDirectSoundNotify,   ( VOID** )&dsNotify )     ) )
	{  
		retErrorMesg =   "StreamingAudioplayer::Initialize_Compatable_Buffer  FAILED,   QueryInterface( IID_IDirectSoundNotify"  ;
		ASSERT( false );     
		return   false; 
	}		

															              //  Set capture buffer notifications  for  'PLAY'

    if(    FAILED(   hr =   dsNotify->SetNotificationPositions(     Get_EventMan().m_numberOfNotificationEvents,    m_waveMan.m_positionNotify   )    ) )
	{		
		retErrorMesg =   "StreamingAudioplayer::Initialize_Compatable_Buffer  FAILED,   SetNotificationPositions"  ;
		ASSERT( false );     		
		return   false;
	}
 

	dsNotify->Release();     // ***** In the future this may cause problems( releasing so soon?? )  See  InsideDirectX by Bargen,  pp 274   ***************

	//////////////////////////////////////////////////////


    FillBufferWithSilence(  m_infcDSoundBuffer  );		



	m_bufferIsOK =    true;

	m_bitSource->m_isInitialized  =    true;

	m_bitSource->m_currentByteIdx =    0;    //   my global counter

	return   true;
}



											////////////////////////////////////////


bool    StreamingAudioplayer::StartPlaying(   short  playMode,   double  speed,     bool  backwards,     long  startSample,  
																		long  endSample,   bool  justSpeedChange,   bool  preventFileSeek,     CString&  retErrorMesg   )
{

	HRESULT  hr;

	retErrorMesg.Empty();

	if(        m_infcDSoundBuffer ==  NULL
		||   !IsOK()      )
	{   
		retErrorMesg  =   "StreamingAudioplayer::StartPlaying failed,  SoundBuffer is null." ;
		return  false; 	
	}


	ASSERT(   startSample  >=   0   );     //  ??  Calling function should set ????  

	BitSourceStreaming   *bitSourceStreaming       =    Get_BitSourceStreaming(  retErrorMesg   );
	if(                              bitSourceStreaming ==  NULL   )
	{  ASSERT( 0 );
		return  false;
	}


																//   a)   set  Play Parms


	long   oldm_startSample =   m_startSample;    //  ****  HOLD this value before it gets overwritten *****

	m_startSample  =    startSample;      //  must change during  'justSpeedChange'  so we do not doubleBack to start




	if(    justSpeedChange    )    
	{

		/***
		m_endSample    =         endSample;     //   If we are changing Speed during MID-PLAY, we want these to commented-out Vars to persist
		m_playMode      =         playMode;
		m_playingBackward =    backwards; 
		***/

		/******  Maybe these 2 for   PlayLast and   LoopPlay

			startSample  =    audioPlayer->m_prevStartSamplePlay;         //   FORWARD  play

			endSample    =    audioPlayer->m_prevEndSamplePlay
		****/
	}
	else
	{  m_endSample    =         endSample;
		m_playMode      =         playMode;

		m_playingBackward =    backwards; 
	}



	if(    ! Set_PlaySpeed(  speed  )     )
	{
		retErrorMesg  =   "StreamingAudioplayer::StartPlaying failed,  Set_PlaySpeed." ;
		return  false;
	}


															 //    b)    save   'PREV-PLAY Positions'   state vars
	if(    justSpeedChange   )       
	{
		int   dummy =   9;
	}
	else
	{
		switch(  playMode  )			
		{							              //   IF we WERE changing Speed during MID-PLAY or going forward without a File-SEEK,  we would NOT want to disturb these  vars

			case    AudioPlayer::NORMALpLAY :

				m_prevBackwardDirectionPlay  =    backwards;                   

				m_prevStartSamplePlay  =    startSample;					
				m_prevEndSamplePlay    =   -2;	    //  We do NOT know where it will stop




				m_lastWindowPlayStartSample =  -1;     //   ********  Need to INIT,  is there a better spot ????    5/12/2012

//   ****  m_lastWindowPlayStartSample       is NEW, and just had to Initialize this in  On_BnClicked_Nudge_DView_Forward_Button() for a BUG.    6/3/12
//   **************************************  CAREFUL   It may still need more initializations elsewhere!!!!    6/2012    ***************


			break;





			case    AudioPlayer::RETURNtoPLAYSTART :
			case    AudioPlayer::LOOPpLAYsELCT :

				m_prevBackwardDirectionPlay  =    backwards;

				m_prevStartSamplePlay  =    startSample;		//  Not really need this assignment,  these values have persisted. 			
				m_prevEndSamplePlay    =   endSample;	

				m_lastWindowPlayStartSample =  -1;     //   ********  Need to INIT,  is there a better spot ????    5/12/2012
			break;  




			case    AudioPlayer::PLAYwINDOW :

				m_prevBackwardDirectionPlay  =    backwards;

	//			m_prevStartSamplePlay  =    startSample;		//  Might be good to keep them around.  User could Nudge to a differnt spot and still maintain the just LOADED LICK.	   5/2012		
	//			m_prevEndSamplePlay    =   endSample;	
			break;  



			case    AudioPlayer::LASTnOTEpLAYfORWARD :

				//m_prevBackwardDirectionPlay  =    backwards;
		//		m_prevStartSamplePlay           =   startSample;		//  ??? OMIT  ???			
			//	m_prevEndSamplePlay    =   endSample;	
		//		m_prevEndSamplePlay             =   -2;     	//     *****  also set below in   switch()
				m_playingCurNotesEndOffset  =     endSample; 			
			break;

			case    AudioPlayer::LASTnOTEpLAYbACKWARD :

				//m_prevBackwardDirectionPlay  =    backwards;
			//	m_prevStartSamplePlay           =   startSample;	//  ??? OMIT  ???			
			//	m_prevEndSamplePlay    =   endSample;			
			//	m_prevEndSamplePlay             =   -2;     	//     *****  also set below in   switch()
				m_playingCurNotesEndOffset  =     startSample  - 1; 
			break;

			default:    ASSERT( 0 );         break;
		}
	}



																				//   set   'STATE'  vars  for play
	if(    preventFileSeek   )
	{

		m_startSample  =   oldm_startSample;   // ******  Need to keep it's old value,  so that the Current Sample Idx can be correctly calulated.   11/11
																	//      ...remember, we are starting WITHOUT a File-SEEK, so many of the previous values must be preserved.
		
		m_prevBackwardDirectionPlay =    backwards;



		m_doneFetchingPlayBytes =    false;     //  init switch that can stop playing
		m_curSubFetch         =   0;     // **** MIGHT not use this.  Was for old style SlowDown Algo  11/11 

//		m_dataFetchCount     =   0;	 ***   Must let these values persist	
//		m_srcBytesProcessed =   0;  
	}
	else
	{
		/***                                 ...this is the contents of  	Initialize_for_Playing_Section() 
		m_doneFetchingPlayBytes =    false;   
		m_curSubFetch  =   0;

		m_dataFetchCount      =   0;   **** NOTICE how we must preserve the values of  { m_dataFetchCount, m_srcBytesProcessed }  for  'preventFileSeek'  mode
		m_srcBytesProcessed  =   0;
		***/				
		Initialize_for_Playing_Section(); 
	}
	



	if(    Get_LastPlayed_SampleIdx()  <  0     )   //   -1:   Play was stopped when  Load_Next_DataFetch_Forward() hit the end of File
		Set_LastPlayed_SampleIdx(  0  );             //           ...need to get it out of negative so Calc_Current_Sample_Forward() will begin to update the var again.




	if(    ! m_theApp.Create_Animating_CDCs_All_Viewjs()     )
	{
		ASSERT( 0 );
		retErrorMesg  =   "StreamingAudioplayer::StartPlaying failed,  Create_Animating_CDCs_All_Viewjs." ;
		return  false;
	}




	if(    preventFileSeek    )    //  here we do NOT want to do a Seek, which is done in  BitSourceStreamingWAV::Move_To_DataStart()   .....WavFile_Start_Reading_Data()   11/11
	{
		int   dummy  =   9;
		/*****   Since there is no File-SEEK, we do not have to update these    11/11

		bitSourceStreaming->m_startPlaySampleIdx =    startSample;    // ***** NEW,  carefull,    so can  do a file Seek  inside of 	Move_To_DataStart()				
		bitSourceStreaming->m_endPlaySampleIdx  =    endSample;
		****/
	}
	else        //   this is for normal operation
	{
		bitSourceStreaming->m_startPlaySampleIdx =    startSample;    //   carefull,   so can  do a   File-SEEK  inside of 	Move_To_DataStart()				
		bitSourceStreaming->m_endPlaySampleIdx  =    endSample;

		if(    ! m_bitSource->Move_To_DataStart(  retErrorMesg  )    )		//  Does a File-SEEK.    
		{
			ASSERT( 0 );
			return  false;
		}
	}




	if(     FAILED(  hr =    m_infcDSoundBuffer->SetCurrentPosition(  0  )        )    )   //  Is this a problem for   'preventFileSeek'
	{
		retErrorMesg  =   "StreamingAudioplayer::StartPlaying failed,  SetCurrentPosition." ;
		return  false;	
	}


	if(      FAILED(  hr =      m_infcDSoundBuffer->Play(   0,   0,   DSBPLAY_LOOPING   )        ) )
	{
		retErrorMesg  =   "StreamingAudioplayer::StartPlaying failed,  Play." ;
		return  false;
	}


	m_isPlaying =    true;

	return  true;
}



											////////////////////////////////////////


void    StreamingAudioplayer::Draw_Last_AnimeFrame()
{		


	//   ASSERT( 0 );     	//   BAD function for  Player and Navigator.  Just goes to dummy functions.   11/11 *******************
									//
									//   ...BUT it is used by PitchScope   ...keep thios maintained   11/24/11



	/*****
	virtual		void		  Animate_All_Viewjs(   long  curSample,    short curScalePitch    )  =0;

	virtual		void		  ReDraw_All_Anime_Viewjs()  =0;
	*****/


//		m_theApp.ReDraw_All_Anime_Viewjs();    //   *** BAD, does nothing.  11/11     dummy function for Navigator

//	Get_UniEditorApp().ReDraw_All_Viewjs();


	m_theApp.ReDraw_All_Anime_Viewjs();
	


	m_theApp.Release_Animating_CDCs_All_Viewjs();    //   DUMMY function in  Navigator and  Player.  2/12        (  YES,  or they do NOT get deallocated   1/03
}




								////////////////////////////////////////


long    StreamingAudioplayer::Get_Waves_ByteCount_toEnd()
{


	// ****** This function looks all MESSED UP *******      But I do not really think it is very valuable or used.     3/2010  

	//  Do NOT use this for  Player or Navigator...   just hope it is still OK for PitchScope   3/11


    //  It uses  BitSource->Calc_Files_Total_Output_Bytes_With_SpeedExpansion()   which expands for SlowSpeeds,  but the rest oif the calcs do NOT
	//  consider the effect of slowed down speeds   3/11


									//   Used in   ****  BACKWARDS play  *****     only            playing only part of  the sample

	if(   m_bytesPerSample  <=  0   )
	{
		ASSERT( 0 );
		return  0;
	}


	if(    m_bitSource  ==   NULL    )
	{
		ASSERT( 0 );
		return  -1;
	}

	ASSERT(   m_bytesPerSample  ==   4  );    //   ALWAYS ???   3/4/2010




	long   totalPlayBytes  =  m_bitSource->Calc_Files_Total_Output_Bytes_With_SpeedExpansion();      //  **** CAREFUL  also calcs  with  *** SLOW DOWN   expansion ***

	ASSERT(  totalPlayBytes  >=  0   );     // If it is negative, we have OVERFLOW  of the data unit 'long'    3/12


    

	if(      (     ! m_playingBackward      &&      m_startSample  >=   m_endSample      )   
		||   (      m_playingBackward       &&      m_endSample  >=   m_startSample      )      )
	{

		//	ASSERT( 0 );  //  OK to swap ??  1/03     // ***  BUG:   LAND her a lot when hit  PlayLastSegment BUTTON after going in reverse 


		long       tmp      =    m_startSample;		 //  Will a simple sort fix the problem ????   6/02
		m_startSample   =    m_endSample;       //   NO,  animation then fails( but plays OK )
		m_endSample    =    tmp;
	}




	long   lastByte,    startByte;


	if(   m_playingBackward  )
	{

		if(     ( m_startSample  *  m_bytesPerSample )   >   totalPlayBytes   )    
			m_startSample  =   (  totalPlayBytes  /   m_bytesPerSample   );


		if(   m_endSample   <   0L   )    
			m_endSample  =   0L;


		startByte  =     m_endSample    *  m_bytesPerSample;    
		lastByte    =    m_startSample   *  m_bytesPerSample;
	}
	else
	{
		if(     ( m_endSample  *  m_bytesPerSample )   >   totalPlayBytes   )    
			m_endSample  =   (  totalPlayBytes  /   m_bytesPerSample   );


		startByte =    m_startSample  *  m_bytesPerSample;
		lastByte  =     m_endSample   *  m_bytesPerSample;
	}



	long   totalBytes =    lastByte  -  startByte;


	ASSERT(  totalBytes  >=  0  );

	return   totalBytes;
}




					////////////////////////////////////////////////////////


void    StreamingAudioplayer::Fill_Buffer_With_Silence()
{

	if(   m_bitSource   !=  NULL   )
		m_bitSource->Erase_DelayBuffer();    //   tricky  sound can also come out of this    2/11


	FillBufferWithSilence(  m_infcDSoundBuffer  );
}



											////////////////////////////////////////


bool    StreamingAudioplayer::Apply_Volume_Adjust_PPlayer(  long  byteCount,   long  volumeInPercent,    BYTE *srcByte,   BYTE *dstByte,   
														                                                    bool  modifyAudibleStereoBalance,    CString&  retErrorMesg   )
{			


	double   boostVol =   2.0;    //  1.5   ***ADJUST***  Need this  because  { leftChannelPercentFlt,  rightChannelPercentFlt  }  are typically 50% each, and reduct the volume.



	long      bytesPerSample  =    4;    // ****  HARDWIRED  *******



	retErrorMesg.Empty();

	if(    srcByte == NULL   ||   dstByte  ==  NULL   )
	{
		retErrorMesg  =   "StreamingAudioplayer::Apply_Volume_Adjust_PPlayer  FAILED,    srcByte  or  dstByte  is NULL.  "  ;
		return  false;
	}



	long   sixteenBitLimit  =   127L;    //   32512    Is this right ???    This is the biggest value the attenuated signal can receive.     3/2010
	sixteenBitLimit  =     sixteenBitLimit   <<  8;     //  looks right,   see  WavConvert::Multiply_Float_Samples()  "31000.0,   could go almost to  32768.0  if I want"



	ASSERT(   m_rightChannelPercent >= 0    &&    m_rightChannelPercent <=  100   );     //  how to  BALANCE the left and right STEREO samples

	double   leftChannelPercentFlt    =     (  100.0 - (double)m_rightChannelPercent  )      / 100.0;                
	double   rightChannelPercentFlt  =                   (double)m_rightChannelPercent          / 100.0;




	long    sampleCount  =    byteCount  / bytesPerSample;

	char    chVal0=0,    chVal1=0,   chVal2=0,    chVal3=0;   //  When used to be below,  but got a  error   Run-Time Check Failure #3 ( this bug was in PitchScope 1.0


 
	for(    long i= 0;      i < sampleCount;     i++    )
	{

		chVal0  =   *srcByte;      srcByte++;
		chVal1  =   *srcByte;      srcByte++;
		chVal2  =   *srcByte;      srcByte++;
		chVal3  =   *srcByte;      srcByte++;


		long   leftSample16    =    chVal1;
		leftSample16  =       (  leftSample16   <<  8  )    |    ( BYTE )chVal0;    //   Need cast so   '|'   will  pack  with negative numbers 

		long   rightSample16    =    chVal3;
		rightSample16  =      (  rightSample16   <<  8  )    |    ( BYTE )chVal2;    //   Need cast so   '|'   will  pack  with negative numbers 	

		long    attenSampleLeft=0,    attenSampleRight=0; 




		if(    modifyAudibleStereoBalance    )    //   Change the balance of LEFT and RIGHT stereo channels to help user figure out which channel has best separation for instrument. 11/24/11
		{

			double    reduceFactor  =   0.35;     //   .35[ very good, maybe perfect ]      **** ADJUST ?? ***  need to make volume equal in all channels. Watch out for value overflow below   11/11

	
			double  attenSampleLeftDB    =    (double)(       (    leftSample16    *  volumeInPercent   )  / 100L       ); 
			double  attenSampleRightDB  =    (double)(       (    rightSample16  *  volumeInPercent   )  / 100L       ); 

			double   differenceFlt,   adjustedBoostVol;
			long      diffSignal;
		//	long    roundToleranceMiddleBalance  =    10;       //       **** ADJUST,   in Percent    ******
		//	long    percentDiff =  abs(   (long)(  leftChannelPercentFlt * 100.0   -   rightChannelPercentFlt * 100.0  )   );  ****BAD if done here:,  this should also be reflected in SndSample   11/11
																			// 11/11     ....This rounding is NOW done in   StreamingAudioplayer::Load_Next_DataFetch_Forward_PPlayer()


			if(	   m_rightChannelPercent  ==   50   )			
				//   TRIED...    percentDiff    <  roundToleranceMiddleBalance   )  BUT WANTED Make it easier for user to select EQUAL Balance????   But then this should also be reflected in SndSample too. 
			{

				attenSampleLeft      =     (long)(    attenSampleLeftDB    *  leftChannelPercentFlt         *  boostVol    );   // **** SHOULD be doing this boost
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
		else     // Sometimes do NOT want to hear the change in StereoBalance,  though it is always applied to Note-Detection  3/11
		{  
			attenSampleLeft    =        (   leftSample16    *  volumeInPercent   )   / 100L; 
			attenSampleRight   =        (   rightSample16  *  volumeInPercent   )   / 100L; 
		}




		if(           attenSampleLeft  >  sixteenBitLimit   )     //  if we go out of range,  then will hear static.
			attenSampleLeft  =   sixteenBitLimit;
		else if(    attenSampleLeft  <  -sixteenBitLimit   )    // Watch out that if   reduceFactor[ 0.35 ]   is adjusted too LOW, then we get overflow here,  
			attenSampleLeft  =   -sixteenBitLimit;                //  and an unintended and unwanted increase in volume.    11/11

		if(           attenSampleRight  >  sixteenBitLimit   )
			attenSampleRight  =   sixteenBitLimit;
		else if(    attenSampleRight  <  -sixteenBitLimit   )
			attenSampleRight  =   -sixteenBitLimit;


																								//  Finally, write out the modified bytes for the sound buffer


		chVal0   =      (char)(    (  attenSampleLeft     &    0x000000ff  )              );      //  low left
		chVal1    =     (char)(    ( attenSampleLeft      &    0xffffff00    )  >> 8     );      //  hi right

		chVal2   =      (char)(    (  attenSampleRight    &    0x000000ff  )               );      //  low  right
		chVal3   =      (char)(    (  attenSampleRight    &    0xffffff00    )   >> 8     );      //  hi right


		*dstByte =   chVal0;       dstByte++;        //  left
		*dstByte =   chVal1;       dstByte++;      

		*dstByte =   chVal2;       dstByte++;         //  right
		*dstByte =   chVal3;       dstByte++;      

	}   //   for(   long i= 


	return  true;
}




											////////////////////////////////////////


void    StreamingAudioplayer::Load_Next_DataFetch_Forward_PPlayer(    unsigned long   iEvent    )
{										 

											//  Loads the next  'chunk' of sample data  from file and plays
    HRESULT          hr;
    DWORD            dwStartOfs;
    static DWORD   dwLastPlayPos;
    VOID               *hardwareBufferPtr;
    DWORD             dwBytesLocked;
//    UINT                  cbBytesRead;
	CString     retErrorMesg;



	bool    allowAudibleChangeOfStereoBalance  =  true;    //     **********    ADJUST    3/11   **********************
																					//   Do NOT have to HEAR a stereoBalance shift for DETECTION if I do not want to. 

	bool      roundUsersStereoBalanceInput =   true;

	short     roundToleranceInPercent  =   10;     //  10%     If difference between Left and Right Stereo channels is less than this, then we round to equal balance 11/11

	bool     readStaticBitsDEBUG    =      false;     // ******  ALWAYS off, now   5/07 DEBUG switch ******


	long    bytesInSample =  4;   // **************   HARDWIRED  ***************************



	short	  approxRightChannelPercent  =		m_rightChannelPercent;    //   default,  without rounding 

	short   absDifference  =    abs(   50  -  m_rightChannelPercent   );  


	if(         m_rightChannelPercent  !=  50 
		&&    absDifference   <  roundToleranceInPercent        //  Do I want to round this if the user is close ???
		&&    roundUsersStereoBalanceInput   ) 
	{
		approxRightChannelPercent  =	50;	
	}
	else  {   }   //		TRACE(  "             No Rounding \n"  );



	SPitchCalc  *sPitchCalcer   =     Get_SPitchCalc();     
	ASSERT(      sPitchCalcer  );

	ASSERT(   m_bitSource->m_wavVolumeSoundmanAddr   !=  NULL   );   

	if(    ! IsOK()     )
	{   
		ASSERT( false );	
		return; 	
	}

	ASSERT(   m_dataFetchCount  >=  0   );


	short  channelCode =    Get_StereoChannelCode();




	if(    m_doneFetchingPlayBytes     )     
	{				    	//  2)    ...just before   EventMan::Process_Event_Notification()  [  with event =  Get_EventMan().m_numberOfNotificationEvents -1

//		StopPlaying_Hardware();       //      ....Better to set flags later
//		return;   //  Let it keep going,  but not try to load new data,  just rehash the existing.

		return;   // ******  NEW   3/1/11   Tr to avoid the messy logic at the bottom   
	}




	unsigned long   hemisEvent  =     ( (unsigned long)( Get_EventMan().m_numberOfNotificationEvents -1 ) )   / 2;    // The event at 180 dergees

													//   If the play cursor has just reached the first or second half of the
													//   buffer, it's time to stream data to the other half.
	if(          iEvent ==    0   )
		dwStartOfs  =   m_dwMidBuffer;    //  want to write 180 degrees from this Event 'position'
	else if(   iEvent ==    hemisEvent     )                  
		dwStartOfs  =    0;  
	else              
		ASSERT( 0 );



    hr =   m_infcDSoundBuffer->Lock(     dwStartOfs,          // Offset of lock start.
															m_dwMidBuffer,   // Number of bytes to lock.
														  &hardwareBufferPtr,               // Will receive address of Lock's Start. ( DEST-memory where we will bytes to )
														  &dwBytesLocked,   // Number of bytes locked.
															NULL,                   // Address of wraparound lock.
															NULL,                   // Number of wraparound bytes.
															0         );                     // Flags.



	if(       (long)dwBytesLocked     !=    m_bytesInFetch     // **** ALWAYS Constant at  44160 ***** ???? 
		&&        m_dataFetchCount   !=    0        )  
	{
		TRACE(  "\n***ERROR***   dwBytesLocked  CHANGED[   %d   ->   %d   ]    m_dataFetchCount[ %d ] ***\n\n",    
										m_bytesInFetch,  dwBytesLocked,   m_dataFetchCount    );
	}




	m_bytesInFetch      =    dwBytesLocked;    // Number of bytes we will read for this Chunk

	UINT     bytsInSect  =     TWOSECBYTES   /   (  Get_EventMan().m_numberOfNotificationEvents  -1 );     // How many bytes in an Event 


    bool    fowardFlag =  true;
	bool    retHitEOF =    false;  
	long    samplesToLoad   =     m_bytesInFetch /  bytesInSample;


	BYTE   *delayWriteByte  =    m_bitSource->Get_DelayBuffer_Write_Byte(    (long)iEvent,    (long)m_dataFetchCount    );

	if(   delayWriteByte  ==   NULL  )
		delayWriteByte =    (BYTE*)hardwareBufferPtr;   // PLAYER:   this would mean we are NOT using a buffer   [  hardwareBufferPtr  points to HARDWARE  circ-soundBuffer




	if(   ! m_bitSource->Fetch_PieSlice_Samples(   samplesToLoad,    delayWriteByte,   retHitEOF,   fowardFlag,   m_rightChannelPercent,   0,  retErrorMesg   )   )
	{  

		m_doneFetchingPlayBytes =    true;   
		m_infcDSoundBuffer->Unlock(    hardwareBufferPtr,    dwBytesLocked,    NULL,    0   ); 		
		return;
	}
	else
	{
		BYTE   *delayReadByte     =     m_bitSource->Get_DelayBuffer_Read_Byte(  (long)iEvent,    (long)m_dataFetchCount   ); 																									  
		if(         delayReadByte   !=   NULL   )
		{		
			BYTE  *dstHardware =   ( BYTE* )hardwareBufferPtr;    //  Now copy  Play-Bytes  to the  HARDWARE's Circular-Buffer


						//    PitchPLAYER does not go here, there is no delay mechanism for it.   It writes directly to the hardware. Volumn is adjusted in  Fetch_Streaming_Samples_Direct_PPlayer()

			long  volumeInPercent =     *(  m_bitSource->m_wavVolumeSoundmanAddr   );    


			if(   ! Apply_Volume_Adjust_PPlayer(   (long)dwBytesLocked,  volumeInPercent,   delayReadByte,   dstHardware,   allowAudibleChangeOfStereoBalance,  retErrorMesg  )    )
			{
				ASSERT( 0 );     //  Doing this here avoids a delay for user when moving the slider.  Can also change the stereoBalance.   3/11
				AfxMessageBox(  retErrorMesg  );
			}
		}
		else	{  }  //  do nothing,  just means that we are NOT using a delayBuffer
				


		m_infcDSoundBuffer->Unlock(    hardwareBufferPtr,    dwBytesLocked,    NULL,    0   );  //   ***** MOVED up here  for faster processing.  3/11   OK ????
	}


	

	if(    retHitEOF   )
	{
		//	ASSERT(  0   );     //  **********	Hit this win new MP3  source   2/4/10 .  Is this an issue		 Do not normally reach this.     1/2003
		m_doneFetchingPlayBytes =    true;  
	}
				


	ASSERT(   dwBytesLocked   ==    BLOCKLOCKEDSIZE   );    //  ** WANT an ERROR message ???? ***


	m_srcBytesProcessed  +=     dwBytesLocked;   // *************   CAREFUL:  these are   SLOW-EXPANDED  bytes  2/19  **************************

	m_dataFetchCount++;   //  count how many times this funct was called during   

	m_curSubFetch  =   0;
}




											////////////////////////////////////////


void   StreamingAudioplayer::Load_Next_DataFetch_Backward_PPlayer(   unsigned long   iEvent    )
{										 

								//  Loads the next 'chunk' of sample data from memory(BitSourceAudioMS) and ALLOWS it to play
    HRESULT           hr;
    DWORD            dwStartOfs;
    static DWORD   dwLastPlayPos;
    VOID              *hardwareBufferPtr;
    DWORD            dwBytesLocked;
	CString             retErrorMesg;


	short   channelCode  =    Get_StereoChannelCode();

	long    bytesPerSample16  =    4;


	if(    ! IsOK()     )
	{   
		ASSERT( false );	
		return; 	
	}


	SPitchCalc  *sPitchCalcer   =     Get_SPitchCalc();     
	ASSERT(      sPitchCalcer  );


	if(    m_dataFetchCount  < 0    )  
	{
		ASSERT( 0 );
		m_dataFetchCount =  0;
	}


	if(    m_doneFetchingPlayBytes    )
	{
	//	StopPlaying_Hardware();       //    now done in   EventMan::Process_Event_Notification()

	//	return;    // ***************   Different on forward   ....let it keep going, WITHOUT Fetching new file data,  just updating vars.
	}



	UINT     byteCountAdjusted =     m_bitSource->Get_Load_Blocks_Size();     //  44160,     the bytes in a HALF rotation of the hardware Sound Buffer

	long      sampleCount =   byteCountAdjusted /4;



	unsigned long   hemisEvent        =    ( (unsigned long)( Get_EventMan().m_numberOfNotificationEvents -1 ) )  / 2;    // The event at 180 dergees
	

	DWORD    numBytesToPlay =     m_startSample   *  m_bytesPerSample;




													// If the play cursor has just reached the first or second half of the
													// buffer, it's time to stream data to the other half.
	if(          iEvent ==    0   )
		dwStartOfs  =    m_dwMidBuffer;                    //  want to write 180 degrees from this Event 'position'
	else if(   iEvent ==   hemisEvent    )                  
		dwStartOfs  =    0;  
	else              
		ASSERT( false );



    hr =   m_infcDSoundBuffer->Lock(   dwStartOfs,          // Offset of lock start.
														     m_dwMidBuffer,   // Number of bytes to lock.
														   &hardwareBufferPtr,               // Address of lock start. ( where we will bytes to )
														   &dwBytesLocked,   // Number of bytes locked.
															 NULL,             // Address of wraparound lock.
															 NULL,             // Number of wraparound bytes.
															 0 );                 // Flags.

	m_bytesInFetch =     dwBytesLocked;     // number of bytes we will read






	long     dataFetchOffset  =  (long)(       (    (double)(m_dataFetchCount  + 1)   *   (double)dwBytesLocked   )    /  m_playSpeedFlt     ); 

				//    Need  +1  because,  even at the first iteration ( m_dataFetchCount= 0 ) we need to backup the WIDTH of a BlockLoad  ( dwBytesLocked/m_playSpeed )
				//    to get to the START of the Block that was PREVIOUS to the LastBlock that was played.
				//    Go to   SoundHelper::Continue_Play_Backwards()   to see how I rounded  'lastPlayedSample'  to be on BlockLoad BOUNDARIES
			
	long   startOffsetBits      =     ( m_startSample  *  m_bytesPerSample )    -  dataFetchOffset;
	if(      startOffsetBits  <  0    )   
	{		             
		startOffsetBits =  0;                           //  Get here if play backwards to beginning of file    3/30/11
		m_doneFetchingPlayBytes =    true; 
	}




	bool  retHitEOF =    false;  


	BYTE   *delayWriteByte     =     m_bitSource->Get_DelayBuffer_Write_Byte_BackwardsPlay(   (long)iEvent,    (long)m_dataFetchCount   );   // In the  WAV-DelayBuffer
	if(        delayWriteByte  ==   NULL  )
	{
		delayWriteByte =    m_backwardsBits;    //   PLAYER:  is NOT using a  WAV-DelayBuffer    [  eventually hardwareBufferPtr  points to HARDWARE  circ-soundBuffer
	}



	
	if(   ! m_bitSource->Fetch_PieSlice_Samples(   sampleCount,   delayWriteByte,   retHitEOF,   false,   m_rightChannelPercent,   startOffsetBits,   retErrorMesg  )   )
	{  

		m_doneFetchingPlayBytes =    true;   
		m_infcDSoundBuffer->Unlock(    hardwareBufferPtr,    dwBytesLocked,    NULL,    0   ); 		
		return;
	}
	else
	{  BYTE   *src  =   delayWriteByte;   
		if(         src   !=   NULL   )
		{		
			BYTE  *dstHardware =   ( BYTE* )hardwareBufferPtr;    //  Now copy  Play-Bytes  to the  HARDWARE's Circular-Buffer

			bool   allowAudibleChangeOfStereoBalance =  true;


						//    PitchPLAYER does not go here, there is no delay mechanism for it.   It writes directly to the hardware. Volumn is adjusted in  Fetch_Streaming_Samples_Direct_PPlayer()

			long  volumeInPercent =     *(  m_bitSource->m_wavVolumeSoundmanAddr   );    

			if(    m_muteReversePlayAudio   )
				volumeInPercent =  0;


			if(   ! Apply_Volume_Adjust_PPlayer(   (long)dwBytesLocked,  volumeInPercent,   src,   dstHardware,   allowAudibleChangeOfStereoBalance,  retErrorMesg  )    )
			{
				ASSERT( 0 );     //  Doing this here avoids a delay for user when moving the slider.  Can also change the stereoBalance.   3/11
				AfxMessageBox(  retErrorMesg  );
			}
		}
		else	{  }  //  do nothing,  just means that we are NOT using a delayBuffer



		m_infcDSoundBuffer->Unlock(    hardwareBufferPtr,    dwBytesLocked,    NULL,    0   );  //   ***** MOVED up here  for faster processing.  3/11   OK ????
	}





	if(   retHitEOF  )
	{
		m_doneFetchingPlayBytes =    true;  
//		return;  //  *****************************  WANT this ????  ***********************
	}



	ASSERT(   dwBytesLocked    ==   BLOCKLOCKEDSIZE    );

	m_srcBytesProcessed   +=    dwBytesLocked;

	m_dataFetchCount++;						 //  count how many times this funct was called during   

	m_curSubFetch =  0;


 
	if(	   m_srcBytesProcessed   >=   (  numBytesToPlay *  (long)m_playSpeedFlt  )    )   // could be EOF,  or end of a Section's bytes( old SectionPlay )   1/03
	{
          //  The way we update 'm_srcBytesProcessed'  is not really true.  'dwBytesLocked'  is really the OutputBytes, and the SlowDown algo
		  //   increase the number of  OutputBytes   [ numberOutPutBytes  =  (m_playSpeed * numberInputBytes)  because of slowDown  ]   3/2011  

		m_doneFetchingPlayBytes =   true;			      //    1)    ...first step in  'Stopping Play' 
	}     
}


