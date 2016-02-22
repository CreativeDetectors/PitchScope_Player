/////////////////////////////////////////////////////////////////////////////
//
//  AudioPlayer.cpp   -   
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


#include "..\comnInterface\DetectorAbstracts.h"     // **** NEW , gives us an ABSTRACT definition of  BitSource, SourceAdmin,  etc


#include  "..\ComnGrafix\OffMap.h"  
#include  "..\comnGrafix\TransformMap.h"

//////////////////////////////////////////////////     




////////////////////////////////////
#include  <mmsystem.h>   //  ( ProgramFiles\Microsoft Visual Studio\VC\PlatformSDK\Include )


#include  "dsoundJM.h"          //  I copied it in, bigger than the VC++ version


#include  "WaveJP.h"




#include  "..\comnAudio\BitSourceAudio.h"

////////////////////////////////////





//////////////////////////////////////////////////         ...my  AUDIO 
#include  "sndSample.h"

#include  "PlayBuffer.h"

////////////////////////////////////////////////// 

#include  "..\ComnAudio\CalcNote.h"

#include  "..\ComnAudio\SPitchCalc.h"


#include  "EventMan.h"  


#include  "AudioPlayer.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////



extern    long   mySinTab[],   myCosTab[];     //  *****CAREFUL if BIGGER !!******** PC:  9800


//extern  int   glbTravDir,     glbYpos;   //   TEMP ******



  EventMan&      Get_EventMan();        //  This class need to acess EventMan in many ways.



								////////////////////////////////////////////////////////////////////////////


//  bool            Set_ScrollCtrl_Horz_Position_ActvViewj_GLB(    double  posInPercent,    CString&  retErrorMesg   );


double		      Get_Hertz_Freq_from_MidiNumber(  short  midiNum  );

long			  Get_Best_Rational_Number_SevenTeenNumer(   long  freq,    long *numerRtn,   long  sampleRate   );

short			  Update_TrigTable(   double  angFreq,    long  numSamps,    TRIGTABLEfrc *tble,   bool  doWindow  );

unsigned short    Get_Rand( void );   







//   UniEditorApp&    Get_UniEditorApp();    ...keep thgis out,  Player now has an abstract class ( 	UniApplication m_theApp;    ) to do this   12/09     
 

//   BitSourceStatic&     Get_BitSource_Static_GLB();





////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////





					///////////////////////////////////////////////////////////////////////////////////////////////////
					////////////////////////////////     'STATIC  Player'    SUBclass    ///////////////////////////
					//////////////////////////////////////////////////////////////////////////////////////////////////


AudioStaticPlayer::AudioStaticPlayer(   EventMan&  eventMan,    BitSourceStatic  *bitSource,     UniApplication&  theApp     )   
																    		 :   AudioPlayer(  eventMan,   theApp  ),   m_bitSource( bitSource )
{
	ASSERT(  bitSource  );

	m_dataFetchCount =   0;          // only in  AudioStaticPlayer
	m_srcBytesProcessed =   0; 


	m_playSpeedFlt =  1.0;     
}



								////////////////////////////////////////


AudioStaticPlayer::~AudioStaticPlayer()
{  
}


								////////////////////////////////////////


HMMIO*	   AudioStaticPlayer::Get_MediaFile()
{

	if(   m_bitSource  ==   NULL   )
	{
		ASSERT( 0 );
		return  NULL;
	}

	return    &(   m_bitSource->m_mmIO  );
}



								////////////////////////////////////////


bool 	  AudioStaticPlayer::Set_BitSource(   BitSource  *bitSource,     CString&  retErrorMesg    )
{

	retErrorMesg.Empty();


	if(  bitSource ==  NULL  )
	{
		retErrorMesg =  "AudioStaticPlayer::Set_BitSource FAILED,  bitSource is  NULL."  ;
		return false;
	}


	BitSourceStatic   *bitSourceStatic     =       dynamic_cast< BitSourceStatic* >(  bitSource  );  
	if(   bitSourceStatic  ==  NULL   )
	{  
		retErrorMesg =   "AudioStaticPlayer::Set_BitSource  FAILED,  could not dcast to BitSourceStatic." ;
		return  false;
	}



	m_bitSource  =   bitSourceStatic;

	return true;
}



											////////////////////////////////////////


bool    AudioStaticPlayer::Initialize_Compatable_Buffer(   CString&   filePath,    CString&  retErrorMesg   )	//**** TAKE 
{

											//   the BUFFER must be compatible with the FILE to be loaded

	m_curSubFetch =  0;

	retErrorMesg.Empty();




   
	/***      ...not need for Static
	if(    filePath.IsEmpty()    )
		return   false;
	***/


	if(   m_bitSource  ==  NULL  )						//  INSTALL better error messaging 
		return  false;


	if(   m_bitSource->m_staticBits  ==  NULL  )
		return  false;



	
	if(     Get_MediaFile()  !=  NULL    )
	{
		WavFile_Close_ReadingFile(   Get_MediaFile(),   &m_pwfx   );
	}



	if(   m_infcDSoundBuffer  !=  NULL   )    //  free the OLD buffer.
    {
        m_infcDSoundBuffer->Release();
        m_infcDSoundBuffer =   NULL;
	}

	m_bufferIsOK =   false;    //  INIT for fail...




											//  Create secondary buffer able to hold 2 seconds( now is 1/2 sec )  of data 

	DSBUFFERDESC   dsbdesc;

    memset(  &dsbdesc,   0,   sizeof( DSBUFFERDESC )    ); 

    dsbdesc.dwSize   =     sizeof(  DSBUFFERDESC  ); 

    dsbdesc.dwFlags  =    DSBCAPS_GETCURRENTPOSITION2  |  DSBCAPS_GLOBALFOCUS  
		                                                            |  DSBCAPS_CTRLPAN  |   DSBCAPS_CTRLPOSITIONNOTIFY;    // **** CAREFULL with notify *** JPM 


//	dsbdesc.dwFlags  =    DSBCAPS_STATIC  |  DSBCAPS_CTRLPOSITIONNOTIFY ;   //    in   "C:\Program Files\Microsoft Platform SDK\Samples\Multimedia\DSound\Src\playsound"
// **** USE better FLAGS ??? ********************  3/03 
	

	dsbdesc.lpwfxFormat     =   m_bitSource->Get_Wav_Format_Struct();     //   Just copy from Bitsource for  STATICplayer


    dsbdesc.dwBufferBytes =     TWOSECBYTES;              //   1/2  sec       Get   sample data every 1/4 sec

    m_dwMidBuffer            =     dsbdesc.dwBufferBytes  / 2;       //  was:   TWOSECBYTES / 2; 




    HRESULT  hr;
    if(   FAILED(   hr =   m_waveMan.m_infcDSound->CreateSoundBuffer(   &dsbdesc,   &m_infcDSoundBuffer,   NULL   )    )    )
    {
		if(   Get_MediaFile()  !=  NULL   )     
			               WavFile_Close_ReadingFile(   Get_MediaFile(),   &m_pwfx   );

		retErrorMesg  =   "AudioStaticPlayer::Initialize_Compatable_Buffer  FAILED,   CreateSoundBuffer";
        return  false; 
    }


												// Assign up notification positions in capture buffer. These occur halfway 
												// through the buffer, at the end of the buffer, and when capture stops.

/***
    m_waveMan.m_positionNotify[ 0 ].dwOffset  =     ( TWOSECBYTES / 2 )  - 1;      // ( dscbDesc.dwBufferBytes / 2 ) - 1;
    m_waveMan.m_positionNotify[ 1 ].dwOffset  =       TWOSECBYTES          - 1;         //  dscbDesc.dwBufferBytes - 1;
***/


	DWORD   posStep =    TWOSECBYTES  /   ( Get_EventMan().m_numberOfNotificationEvents -1 );


    for(   int i=0;     i <    (int)( Get_EventMan().m_numberOfNotificationEvents -1 );      i++   )
    {
		m_waveMan.m_positionNotify[ i ].dwOffset  =   (  (DWORD)i  * posStep  );    //  (  (DWORD)(i+1)   *    posStep  )   - 1;
	}
		


//  m_waveMan.m_positionNotify[         2                            ].dwOffset  =    DSBPN_OFFSETSTOP;
	 m_waveMan.m_positionNotify[   Get_EventMan().m_numberOfNotificationEvents -1  ].dwOffset  =    DSBPN_OFFSETSTOP;





	//////////////////////////////////////////////////////    
	LPDIRECTSOUNDNOTIFY   dsNotify;					//  Get interface for play-buffer NOTIFICATION
											                 
    if(   FAILED(   hr =   m_infcDSoundBuffer->QueryInterface(   IID_IDirectSoundNotify,   (VOID**)&dsNotify )     ) )
	{               
		ASSERT( false );    
		retErrorMesg  =   "AudioStaticPlayer::Initialize_Compatable_Buffer  FAILED,   QueryInterface(   IID_IDirectSoundNotify )";
		return   false; 
	}		
															               // Set capture buffer notifications.

    if(    FAILED(   hr =   dsNotify->SetNotificationPositions(   Get_EventMan().m_numberOfNotificationEvents,   m_waveMan.m_positionNotify   )    ) )
	{
		retErrorMesg  =   "AudioStaticPlayer::Initialize_Compatable_Buffer  FAILED,   SetNotificationPositions";
		return   false;
	}
 
	dsNotify->Release();     // In the future this may cause problems( releasing so soon?? )
	//////////////////////////////////////////////////////



    FillBufferWithSilence(  m_infcDSoundBuffer  );		


														//  can I just assign the DataBits  and  WaveFormatStruct from BitSourceAudioMS ???  

//	m_totalPlayBytes =   m_bitSource->m_totalBytes;		//****** WRONG, this is  BYTES!!!




	m_bufferIsOK =  true;

	return   true;
}




								////////////////////////////////////////


BYTE*    AudioStaticPlayer::Get_Waves_DataBits_Start()
{

	//  Is a little DIFFERENT than that for  StreamingAudioplayer

															//  resides in  BitSourceAudioMS  
	if(   m_bytesPerSample  <=  0     )
	{
		ASSERT( 0 );
		return  NULL;
	}




	if(    m_bitSource  ==   NULL    )
	{
		ASSERT( 0 );
		return  NULL;
	}

        
	if(      (     !m_playingBackward      &&      m_startSample  >=   m_endSample      )   
		||   (     m_playingBackward       &&      m_endSample  >=   m_startSample      )      )
	{
		
		ASSERT( 0 );  // Still a little buggy.   6/21/02       // ***  BUG:   LAND her a lot when hit  PlayLastSegment BUTTON after going in reverse 

			
		long       tmp      =    m_startSample;		 //  Will a simple sort fix the problem ????    6/02
		m_startSample   =    m_endSample;
		m_endSample    =    tmp;

		//    return  0;
	}




	if(    m_startSample  <  0    )
		m_startSample =  0;


	char  *memStart =     m_bitSource->m_staticBits;

	long    skipBytes  =    m_startSample   *   m_bytesPerSample;


	return    ( BYTE* )(     ( m_bitSource->m_staticBits   +   skipBytes  )     );
}




								////////////////////////////////////////


long    AudioStaticPlayer::Get_Waves_ByteCount_toEnd()
{

	/*****
															//   Used in playing only part of  the sample
	if(   m_bytesPerSample  <=  0   )
	{
		ASSERT( 0 );
		return  0;
	}




		if(    m_bitSource  ==   NULL    )
		{
			ASSERT( 0 );
			return  NULL;
		}


		if(      (     !m_playingBackward      &&      m_startSample  >=   m_endSample      )   
			||   (     m_playingBackward       &&      m_endSample  >=   m_startSample      )      )
		{

			//  ASSERT( 0 );  // Still a little buggy.    6/21/02        // ***  BUG:   LAND her a lot when hit  PlayLastSegment BUTTON after going in reverse 


			long       tmp      =    m_startSample;		 //  Will a simple sort fix the problem ????   6/02
			m_startSample   =    m_endSample;       //   NO,  animation then fails( but plays OK )
			m_endSample    =    tmp;

			//    return  0;
		}




		long   lastByte,    startByte;


		if(   m_playingBackward  )
		{

			if(     ( m_startSample  *  m_bytesPerSample )   >   (long)m_totalPlayBytes   )    
				m_startSample  =   (  (long)m_totalPlayBytes  /   m_bytesPerSample   );


			if(   m_endSample   <   0L   )    
				m_endSample  =   0L;


			startByte  =     m_endSample    *  m_bytesPerSample;    // have to w
			lastByte    =    m_startSample   *  m_bytesPerSample;
		}
		else
		{
			if(     ( m_endSample  *  m_bytesPerSample )   >   (long)m_totalPlayBytes   )    
				m_endSample  =   (  (long)m_totalPlayBytes  /   m_bytesPerSample   );

			startByte =    m_startSample  *  m_bytesPerSample;
			lastByte  =     m_endSample   *  m_bytesPerSample;
		}



		long   totalBytes =    lastByte  -  startByte;


		ASSERT(  totalBytes  >=  0  );

		return   totalBytes;
		*****/


									//   Used in   ****  BACKWARDS play  *****     only            playing only part of  the sample


			// ******  NOT  tested yet after  addiion of    Calc_Files_Total_Output_Bytes_With_SpeedExpansion()   2/10 **************



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


	long   totalPlayBytes =  	  m_bitSource->Calc_Files_Total_Output_Bytes_With_SpeedExpansion();




	if(      (     !m_playingBackward      &&      m_startSample  >=   m_endSample      )   
		||   (     m_playingBackward       &&      m_endSample  >=   m_startSample      )      )
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




								////////////////////////////////////////


WAVEFORMATEX*    AudioStaticPlayer::Get_Wave_Format()			//   ...NOT used ???    1/02
{
															//  resides in  BitSourceAudioMS  
	if(    m_bitSource  ==   NULL   )
	{
		ASSERT( 0 );
		return  NULL;
	}

	return      m_bitSource->m_wavFormat;
}



								////////////////////////////////////////


bool    AudioStaticPlayer::ExamineWavFile(  CString&   filePath,   CString&  retErrorMesg  )
{



	bool   succs =    Initialize_Compatable_Buffer(  filePath,  retErrorMesg  );
	if(   !  succs    )
		return  false;


	if(    Get_MediaFile()   !=  NULL   )   
		WavFile_Close_ReadingFile(   Get_MediaFile(),   &m_pwfx   );    //   the streaming version NOT do this  ...must have file open for read while play


/***
	if(    !Create_Extra_SoundBuffer(  m_maximumExtraBufferBytes,   retErrorMesg  )      )   //  a short STATIC buffer for very short sounds ( < 1 sec )
	{
		AfxMessageBox( retErrorMesg );
		return  false;
	}
***/

	return   succs;
}
			


								////////////////////////////////////////


bool    AudioStaticPlayer::StartPlaying(   short  playMode,   double  speed,     bool  backwards,     long  startSample,  
																		long  endSample,   bool  justSpeedChange,  bool   preventFileSeek,  CString&  retErrorMesg   )
{

	HRESULT  hr;

	retErrorMesg.Empty();

	if(        m_infcDSoundBuffer ==  NULL
		||   !IsOK()      )
	{   
		retErrorMesg  =   "AudioStaticPlayer::StartPlaying failed,  SoundBuffer is null." ;
		return  false; 	
	}


	ASSERT(   startSample  >=   0   );     //  ??  Calling function should set ????  




																//   a)   set  Play Parms


	m_startSample  =    startSample;      //  must change during  'justSpeedChange'  so we do not doubleBack to starty


	if(    !justSpeedChange    )    //   If we are changing Speed during MID-PLAY, we want these to persist
	{
		m_endSample    =         endSample;
		m_playMode       =         playMode;
		m_playingBackward =    backwards; 
	}


	if(    ! Set_PlaySpeed(  speed  )     )
	{
		retErrorMesg  =   "AudioStaticPlayer::StartPlaying failed,  Set_PlaySpeed." ;
		return  false;
	}




															 //    b)    save   'PREV-PLAY Positions'   state vars


	if(    ! justSpeedChange    )    //   If we are changing Speed during MID-PLAY, we do not want to disturb these 
	{

		switch(  playMode  )			
		{
			case    AudioPlayer::NORMALpLAY :

				m_prevBackwardDirectionPlay  =    backwards;
				m_prevStartSamplePlay           =    startSample;					
				m_prevEndSamplePlay    =   -2;	    //  We do NOT know where it will stop
			break;



			case    AudioPlayer::RETURNtoPLAYSTART :

				m_prevBackwardDirectionPlay  =    backwards;

				m_prevStartSamplePlay =     startSample;					
				m_prevEndSamplePlay   =     endSample;	
			break;   



			case    AudioPlayer::PLAYwINDOW :

				m_prevBackwardDirectionPlay  =    backwards;  // **********  ????????????????????    5/12

				m_prevStartSamplePlay  =    startSample;					
				m_prevEndSamplePlay    =    endSample;	
			break;   




			case    AudioPlayer::LOOPpLAYsELCT :

				ASSERT( 0 );   //  *****  should NOT be NEEDED for this mode    4/07

				m_prevBackwardDirectionPlay  =    backwards;
				m_prevStartSamplePlay           =    startSample;					
				m_prevEndSamplePlay    =   endSample;	
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
	
	/*****
	m_doneFetchingPlayBytes =    false;     //  init switch that can stop playing

	m_curSubFetch         =   0;     //  init for this playSession
	m_dataFetchCount     =   0;			 //   init for THIS play,   Load_Next_DataFetch_Forward() will correctly bring this to 0 on first read. 
	m_srcBytesProcessed =   0; 
	****/
	Initialize_for_Playing_Section(); 



	if(    Get_LastPlayed_SampleIdx()  <  0     )   //   -1:   Play was stopped when  Load_Next_DataFetch_Forward() hit the end of File
		Set_LastPlayed_SampleIdx(  0  );    //  ...need to get it out of negative so Calc_Current_Sample_Forward() will begin to update the var again.




//	FillBufferWithSilence(  m_infcDSoundBuffer  );			// *** OK here ???? *****   ...clear out old sound 



	if(    ! m_theApp.Create_Animating_CDCs_All_Viewjs()     )
	{
		ASSERT( 0 );
		retErrorMesg  =   "AudioStaticPlayer::StartPlaying failed,  Create_Animating_CDCs_All_Viewjs." ;
		return  false;
	}


	if(     FAILED(  hr =    m_infcDSoundBuffer->SetCurrentPosition(  0  )        )    ) 
	{
		ASSERT( 0 );    
		retErrorMesg  =   "AudioStaticPlayer::StartPlaying failed,  SetCurrentPosition." ;
		return  false;	
	}


	if(      FAILED(  hr =      m_infcDSoundBuffer->Play(   0,   0,   DSBPLAY_LOOPING   )        ) )
	{
		ASSERT( 0 );  
		retErrorMesg  =   "AudioStaticPlayer::StartPlaying failed,  Play." ;
		return  false;
	}



	m_isPlaying =    true;

	return  true;
}



											////////////////////////////////////////


void   AudioStaticPlayer::Load_Next_DataFetch_Forward(  unsigned long   iEvent   )
{										 

								//  Loads the next 'chunk' of sample data from memory(BitSourceAudioMS) and ALLOWS it to play
    HRESULT           hr;
    DWORD            dwStartOfs;
    static DWORD   dwLastPlayPos;
    VOID              *lpvData;
    DWORD            dwBytesLocked;
 

	if(    !IsOK()     )
	{   
		ASSERT( false );	
		return; 	
	}

	ASSERT(   m_playSpeedFlt  >=  1.0   );


	if(    m_dataFetchCount  < 0    )  
	{
		ASSERT( 0 );
		m_dataFetchCount =  0;
	}



	if(    m_doneFetchingPlayBytes    )
	{

	//	StopPlaying_Hardware();      //   ***** FIX like StreamingPlayer  1/03  ******

	//	return;
	}




	DWORD   numBytesToPlay =     ( DWORD )Get_Waves_ByteCount_toEnd();   // will shorten for play of PART of the sample that is SELECTED with RangeFrame

	unsigned long  hemisEvent =    ( (unsigned long)( Get_EventMan().m_numberOfNotificationEvents -1 ) )  / 2;    // The event at 180 dergees
	



												// If the play cursor has just reached the first or second half of the
												// buffer, it's time to stream data to the other half.
	if(         iEvent ==  0       
		 ||    iEvent ==  hemisEvent    )    //	 WAS:  if(   iEvent == 0   ||    iEvent == 1  )  
    {

		if(          iEvent ==    0   )
			dwStartOfs  =    m_dwMidBuffer;    //  want to write 180 degrees from this Event 'position'
		else if(   iEvent ==   hemisEvent    )                  
			dwStartOfs  =    0;  
		else              
			ASSERT( false );



        hr =   m_infcDSoundBuffer->Lock(   dwStartOfs,          // Offset of lock start.
														     m_dwMidBuffer,   // Number of bytes to lock.
														   &lpvData,               // Address of lock start. ( where we will bytes to )
														   &dwBytesLocked,   // Number of bytes locked.
															 NULL,             // Address of wraparound lock.
															 NULL,             // Number of wraparound bytes.
															 0 );                 // Flags.

		m_bytesInFetch =     dwBytesLocked;     // number of bytes we will read



		BYTE    *srcData,  *srcStart,  *dstData,   *zoneSrcStart;     //  SUBSTUTED  'WaveReadFile()' with read from the memory buffer 

		dstData =   ( BYTE* )lpvData;



		if(   m_playSpeedFlt  ==   1.0   )
		{

			srcStart  =     Get_Waves_DataBits_Start();	   //  in  BitSourceAudioMS::m_staticBits  [ OLD:  srcData =   m_ppbData  +  (  m_dataFetchCount  *  dwBytesLocked  );  ]

			srcData  =     srcStart   +    (   m_dataFetchCount   *   dwBytesLocked    );



			if(   !m_doneFetchingPlayBytes    )    
			{

				for(   DWORD  i =0;      i <  dwBytesLocked;      i++   )			//  Read the bytes from BitSourceAudioMS memory to the hardware
				{

					if(    m_srcBytesProcessed   <   numBytesToPlay    )
					{   

						/****   SEE  how  StreamingPlayer does this masking with   Fetch_Streaming_Samples_Direct()    1/03

						if(    m_muteWithOnsetmask   )
						{
							long   bytesOffsetFromStart  =     srcData   -    (BYTE*)(  m_bitSource->m_staticBits  );
							long   sampleIdx                 =      bytesOffsetFromStart  /   m_bytesPerSample;

							if(      Is_Byte_In_ScalepitchSubj(  sampleIdx  )     )
								*dstData =   *srcData;
							else
								*dstData =    0;  
						}
						else
							*dstData =   *srcData;
						***/
						*dstData =   *srcData;


						srcData++;
						dstData++;     
					}

					m_srcBytesProcessed++;

					if(    m_srcBytesProcessed   >=     numBytesToPlay   )     //  get a 'memory access' error if we try to read too far
						break;
				}
			}



			m_dataFetchCount++;   //  count how many times this funct was called during   

			m_curSubFetch =  0;
		}   

		else	 //  playing at  SLOWER speeds 	[  Static  Player  ]				 ***CAREFUL:  To always work on 32bit  BOUNDARIES!!!  ( 2x 16bit samples = 32 )
		{

			long   repeatSize =   m_slowPlayStep;	//  4096L    tried:  256   1024    2048    4096   	[ BEST:  4096   2048  ]	  [  bad:  8192  ]
			long   zoneReadCount = 0,    bytIdx = 0;		


			srcStart  =   Get_Waves_DataBits_Start();	   

			zoneSrcStart  =     srcStart    +    (     ( m_dataFetchCount  *  dwBytesLocked  )   /   (long)m_playSpeedFlt     );

			srcData         =     zoneSrcStart;  



			if(   !m_doneFetchingPlayBytes    )    
			{
				for(   long i =0;     i <  (long)dwBytesLocked;     i++   )	 //  Read the bytes from BitSourceAudioMS memory to the hardware
				{


					if(    m_srcBytesProcessed   >=     numBytesToPlay   )     //  get a 'memory access' error if we try to read too far
					{
						break;       
					}
					else     //  if(     m_srcBytesProcessed   <    (m_totalSamples * m_playSpeed)     )   //  Not hit the end of the sample?
					{   

						if(    bytIdx   >=   repeatSize   )      //  just hit 4096,  so crossing into a new zone
						{

							zoneReadCount++;        

							if(    zoneReadCount  >=   (long)m_playSpeedFlt    )   //  time to advance to next  zoneSrcStart ?
							{							

								zoneSrcStart  +=    repeatSize;          // advance to the next  source block

								zoneReadCount = 0;		 // reset the cycle


	//						    long   bytesOffsetFromStart  =     srcData   -    (BYTE*)(  m_bitSource->m_staticBits  );
								 long   bytesOffsetFromStart  =     zoneSrcStart   -    (BYTE*)(  m_bitSource->m_staticBits  );  // the NEW  'tap point'   in StaticBits

								 long   sampleIdx                  =     bytesOffsetFromStart  /   m_bytesPerSample;


							//   TRACE(  "Fetch STATIC Samples():    offsetFromStart[ %d ]    sampleIdx[  %d  ]   \n",   	bytesOffsetFromStart,      sampleIdx );
							}
							

							srcData  =  zoneSrcStart;    //  go to the zoneSrcStart
							bytIdx = 0;         //  reset for the next cycle
						}


						/**** REPLACE  with new  m_auditionCode   [  SEE  how  StreamingPlayer does this masking with   Fetch_Streaming_Samples_Direct()    1/03

						if(    m_muteWithOnsetmask   )
						{
							long   bytesOffsetFromStart  =     srcData   -    (BYTE*)(  m_bitSource->m_staticBits  );
							long   sampleIdx                  =     bytesOffsetFromStart  /   m_bytesPerSample;

							if(      Is_Byte_In_ScalepitchSubj(  sampleIdx  )     )
								*dstData =   *srcData;
							else
								*dstData =    0;   
						}
						else
							*dstData =   *srcData;
						***/
						*dstData =   *srcData;



						srcData++;
						dstData++;     //  destination just keeps advancing without issue

						bytIdx++;

						if(    zoneReadCount  ==  0   )  		// only count while in FIRST zone, other wis  occasionaly get an 'ACCESS exception'
							m_srcBytesProcessed++;
					}  

				}  //  for(  i =0

			}   //    if(   !m_doneFetchingPlayBytes  ) 





			if(     (m_dataFetchCount  %   ((long)m_playSpeedFlt)   )  ==  0    )  //  since m_curChunkPlayer=-1 at start,  m_curChunkPlayer goes to 0 on the first funct call 
				m_curSubFetch =  0;   //  init
			else
				m_curSubFetch++;    // Though we do NOT advance m_curChunkPlayer,  this tells us the implied advance by this fetch( for Calc_Current_Sample_Forward() 
			

			m_dataFetchCount++;     //  count how many times this funct was called during 
		}   




		m_infcDSoundBuffer->Unlock(    lpvData,    dwBytesLocked,    NULL,   0   );


		if(	   m_srcBytesProcessed   >=    numBytesToPlay    )	
        {
			m_doneFetchingPlayBytes =   true;				//    1)    ...first step in  'Stopping Play' 
        }
    }

	else if(    iEvent ==	(unsigned long)( Get_EventMan().m_numberOfNotificationEvents -1 )      )	 //   WAS:  2  in example code
	{
		ASSERT( 0 );     //  int   dummy = 2;      **** NEVER gets Called ???  *****   6/02
	}
}





											////////////////////////////////////////


void   AudioStaticPlayer::Load_Next_DataFetch_Backward(  unsigned long   iEvent   )
{										 

								//  Loads the next 'chunk' of sample data from memory(BitSourceAudioMS) and ALLOWS it to play
    HRESULT           hr;
    DWORD            dwStartOfs;
    static DWORD   dwLastPlayPos;
    VOID              *lpvData;
    DWORD            dwBytesLocked;
 

	if(    !IsOK()     )
	{   
		ASSERT( false );	
		return; 	
	}

	ASSERT(   m_playSpeedFlt  >=  1.0   );


	if(    m_dataFetchCount  < 0    )  
	{
		ASSERT( 0 );
		m_dataFetchCount =  0;
	}



	if(    m_doneFetchingPlayBytes    )
	{

	//	StopPlaying_Hardware();   //   ***** FIX like StreamingPlayer  1/03  ******

	//	return;
	}




	DWORD   numBytesToPlay =     (DWORD)Get_Waves_ByteCount_toEnd();   // will shorten for play of PART of the sample that is SELECTED with RangeFrame



	unsigned long  hemisEvent =    ( (unsigned long)( Get_EventMan().m_numberOfNotificationEvents -1 ) )  / 2;    // The event at 180 dergees
	

												// If the play cursor has just reached the first or second half of the
												// buffer, it's time to stream data to the other half.
	if(         iEvent ==  0       
		 ||    iEvent ==  hemisEvent    )    //	 WAS:  if(   iEvent == 0   ||    iEvent == 1  )  
    {

		if(          iEvent ==    0   )
			dwStartOfs  =    m_dwMidBuffer;    //  want to write 180 degrees from this Event 'position'
		else if(   iEvent ==   hemisEvent    )                  
			dwStartOfs  =    0;  
		else              
			ASSERT( false );



        hr =   m_infcDSoundBuffer->Lock(   dwStartOfs,          // Offset of lock start.
														     m_dwMidBuffer,   // Number of bytes to lock.
														   &lpvData,               // Address of lock start. ( where we will bytes to )
														   &dwBytesLocked,   // Number of bytes locked.
															 NULL,             // Address of wraparound lock.
															 NULL,             // Number of wraparound bytes.
															 0 );                 // Flags.

		m_bytesInFetch =     dwBytesLocked;     // number of bytes we will read


		
		BYTE    *srcData,  *srcStart,   *zoneSrcStart;    
		BYTE    *dstData  =     ( BYTE* )lpvData;


		if(   m_playSpeedFlt  ==   1.0   )
		{

			srcStart  =     Get_Waves_DataBits_Start();	   //  in  BitSourceAudioMS::m_staticBits  [ OLD:  srcData =   m_ppbData  +  (  m_dataFetchCount  *  dwBytesLocked  );  ]

			srcData  =     srcStart   -    (   m_dataFetchCount   *   dwBytesLocked    );  // NOTE  "-" cause going bacward



			if(    !m_doneFetchingPlayBytes    )
			{

				for(   DWORD i =0;      i <  dwBytesLocked;      i++   )			//  Read the bytes from BitSourceAudioMS memory to the hardware
				{

					if(    m_srcBytesProcessed    <    numBytesToPlay    )
					{   
						*dstData  =  *srcData;

						srcData--;		 //  (note SUBTRACT!! )
						dstData++;     
					}

					m_srcBytesProcessed++;

					if(    m_srcBytesProcessed   >=     numBytesToPlay   )     //  get a 'memory access' error if we try to read too far
						break;
				}
			}


			m_dataFetchCount++;   //  count how many times this funct was called during   
			m_curSubFetch =  0;


		/*************************************    ...FAILS but I dont know why???

		unsigned long    *srcData,  *srcStart;    
		unsigned long    *dstData  =     ( unsigned long* )lpvData;

		if(   m_playSpeed  ==   1   )
		{
			srcStart  =     ( unsigned long* )Get_Waves_DataBits_Start();	   //  in  BitSourceAudioMS::m_staticBits  [ OLD:  srcData =   m_ppbData  +  (  m_dataFetchCount  *  dwBytesLocked  );  ]

			srcData  =     srcStart   -    (   m_dataFetchCount  *   dwBytesLocked   );  // NOTE  "-" cause going bacward

			ASSERT(   (dwBytesLocked  %  4)  ==  0    );

			unsigned long   val;
			unsigned long   *absStartByte =    ( unsigned long* )(   m_bitSource->m_staticBits   );


			for(   DWORD i =0;      i <  ( dwBytesLocked / 4 );      i++   )			//  Read the bytes from BitSourceAudioMS memory to the hardware
			{

				if(    m_srcBytesProcessed    <    numBytesToPlay   
					
					||  (    (long)srcData    >    (long)absStartByte    )     )
				{   
					//  *dstData  =  *srcData;

					val          =    *srcData;      *******  get ACESS VILOLATION here ...WHY ???? *****

					*dstData  =     val;

					dstData++;     
					srcData--;		//  note that we are reading backwards
				}
				else
				{	int   dummyBreak =   9; 
				}

				m_srcBytesProcessed  +=  4;

				if(    m_srcBytesProcessed   >=     numBytesToPlay   )     //  get a 'memory access' error if we try to read too far
					break;
			}


			m_curChunkPlayer--;   //   Keep count of how many data 'chunks'  we process so  Calc_Current_Sample_Forward() can work

			m_dataFetchCount++;   //  count how many times this funct was called during   
			m_curSubFetch =  0;
		  ***/


		}     //  m_playSpeed  ==   1 

		else	 //  playing at  SLOWER speeds      ***CAREFUL:  To always work on 32bit  BOUNDARIES!!!  ( 2x 16bit samples = 32 )
		{

			long   repeatSize =   m_slowPlayStep;	//  4096L    tried:  256   1024    2048    4096   	[ BEST:  4096   2048  ]	  [  bad:  8192  ]
			long   zoneReadCount = 0,    bytIdx = 0;		


			srcStart  =   Get_Waves_DataBits_Start();	   

			zoneSrcStart  =     srcStart    -    (     ( m_dataFetchCount  *  dwBytesLocked  ) /   (long)m_playSpeedFlt     );   //   ( note SUBTRACTION )
			srcData         =     zoneSrcStart;  



			if(    !m_doneFetchingPlayBytes    )
			{

				for(   long i =0;     i <  (long)dwBytesLocked;     i++   )	 //  Read the bytes from BitSourceAudioMS memory to the hardware
				{


					if(    m_srcBytesProcessed   >=     numBytesToPlay   )     //  get a 'memory access' error if we try to read too far
					{
						break;       
					}
					else     //  if(     m_srcBytesProcessed   <    (m_totalSamples * m_playSpeed)     )   //  Not hit the end of the sample?
					{   

						if(    bytIdx   >=   repeatSize   )      //  just hit 4096,  so crossing into a new zone
						{
							zoneReadCount++;        

							if(    zoneReadCount  >=   (double)m_playSpeedFlt    )   //  time to advance to next  zoneSrcStart ?
							{							
								zoneSrcStart  -=    repeatSize;          //  ( note SUBTRACTION )  advance to the next  source block
								zoneReadCount = 0;		 // reset the cycle
							}
							
							srcData  =  zoneSrcStart;    //  go to the zoneSrcStart

							bytIdx = 0;         //  reset for the next cycle
						}

					
						*dstData  =  *srcData;

						srcData--;       //   ( note SUBTRACTION )
						dstData++;     //  destination just keeps advancing without issue


						bytIdx++;

						if(    zoneReadCount  ==  0   )  		// only count while in FIRST zone, other wis  occasionaly get an 'ACCESS exception'
							m_srcBytesProcessed++;
					}  
				}  //  for(  i =0

			}

			


			if(    ( m_dataFetchCount   %   ((long)m_playSpeedFlt)     )  ==  0    )  //  since m_curChunkPlayer=-1 at start,  m_curChunkPlayer goes to 0 on the first funct call 
				m_curSubFetch =  0;   //  init
			else
				m_curSubFetch++;    // Though we do NOT advance m_curChunkPlayer,  this tells us the implied advance by this fetch( for Calc_Current_Sample_Forward() 			


			m_dataFetchCount++;     //  count how many times this funct was called during 
		}   
		



		m_infcDSoundBuffer->Unlock(    lpvData,    dwBytesLocked,    NULL,   0   );



		if(	   m_srcBytesProcessed   >=    numBytesToPlay    )	   // did we run out of data...
		{
			m_doneFetchingPlayBytes =   true;				//    1)    ...first step in  'Stopping Play' 
		}
     }

	else if(    iEvent ==	(unsigned long)( Get_EventMan().m_numberOfNotificationEvents -1 )      )	 //   WAS:  2  in example code
	{
		ASSERT( 0 );     //  int   dummy = 2;      **** NEVER gets Called ???  *****   6/02
	}
}



//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////



											////////////////////////////////////////


long    AudioPlayer::Calc_Current_Sample_Forward(  unsigned long  iEvent   )
{


									//  A new dataChunk is load on   iEvent =  { 0  or 10 }  for   Get_EventMan().m_numberOfNotificationEvents = 21


	long   numberOfChannels =  2;           // stereo
	long   numberOfBytesInSample = 2;    //  ONLY for 16 bit samples!!!!     FIX**********

	long   retSampleIdx = -1,   remainder,    piePos;    // piePos = { 0 - 9 }

	long   numHemisphereEvents =    ( Get_EventMan().m_numberOfNotificationEvents   -1 )  /2;   //   30     [ HALF of total events ]   a hemisphere(  180 deg ) of traverse



	long   samplesPerDataFetch =     m_bytesInFetch    /    ( numberOfBytesInSample * numberOfChannels );     //  11040;    

	long   samplesPerPiePos      =     samplesPerDataFetch   /   numHemisphereEvents;    //  now  is 368,   Process 368 samples with each Event.    3/2010 




	if(    ( (long)iEvent )  >=  numHemisphereEvents    )
		piePos =    iEvent  -  numHemisphereEvents;
	else
		piePos =    iEvent;


	/***  NOTES...

		long     startOffsetBits  =     
						(  m_dataFetchCount   *   dwBytesLocked   )    /  m_playSpeed      //   How many bytes have we PREVIOUSLY fetched in earlier function calls...
												+    ( m_startSample  *  m_bytesPerSample  );    //   ...plus the  ContinuedPlay-OFFSET
	***/



																//  Must make adjustments for  SLOW-play...
	if(    m_playSpeedFlt  >  1.0    )
	{



		double    samplesPerVirtualDataFetch  =       (double)samplesPerDataFetch  /  m_playSpeedFlt;


   //  OLD: 	retSampleIdx  =     ( realCurChunk * samplesPerDataFetch )  	+    (  m_curSubFetch  *  samplesPerVirtualDataFetch  );

		double    realDataFetchCount        =     (double)(  m_dataFetchCount - 1 )   / m_playSpeedFlt;   //  m_dataFetchCount is only
						//   - 1 :    m_dataFetchCount  is an Index,  and on first round it has already been incremented to 1( we want0 )
		                //   / m_playSpeed :   See  Load_Next_DataFetch_Forward()   ...we count m_dataFetchCount faster than you would think.  1/03 



		retSampleIdx   =    (long)(  realDataFetchCount    *   samplesPerDataFetch   )   
												   +    (  m_curSubFetch  *  (long)samplesPerVirtualDataFetch  )    +  m_startSample;


// *****  CHANGE the calc order so can get a better estimate with Doubles  )



		remainder       =   (long)(      (double)piePos   *     (double)samplesPerPiePos /  m_playSpeedFlt    );    //  ...this also advances SLOWER


	}
	else
	{  
		retSampleIdx   =    (  m_dataFetchCount   *   samplesPerDataFetch   )   +    m_startSample;

		remainder        =    piePos   *  samplesPerPiePos;  
	}
	

	retSampleIdx  +=    remainder;




//	TRACE(  "curSample:  %d,   [ iEvent  %d ]      SUBchunk  %d,    CURchunk  %d\n",     retSampleIdx,   iEvent,   m_curSubFetch,    m_curChunkPlayer    );



	ASSERT(   retSampleIdx  >=   0   );  // ***BUG: 3/03  Some times get here from WindowPlay at beginning of app.

	return   retSampleIdx;   //  this value SOON gets written to    Viewj::m_curSampleIdx  during the Draw()
}


											////////////////////////////////////////


long    AudioPlayer::Calc_Current_Sample_Backward(  unsigned long  iEvent   )
{

									//  A new dataChunk is load on   iEvent =  { 0  or 10 }  for   Get_EventMan().m_numberOfNotificationEvents = 21


	long   numberOfChannels =  2;           // stereo
	long   numberOfBytesInSample = 2;    //  ONLY for 16 bit samples!!!!     FIX**********

	long   retSampleIdx = -1,   remainder,    piePos;    // piePos = { 0 - 9 }

	long   numHemisphereEvents =    ( Get_EventMan().m_numberOfNotificationEvents   -1 )  /2;   //  [ HALF events ]should be 10,  a hemisphere(  180 deg ) of traverse



	long   samplesPerDataFetch =     m_bytesInFetch /  ( numberOfBytesInSample * numberOfChannels );     //  11040;    

	long   samplesPerPiePos =     samplesPerDataFetch /  numHemisphereEvents;    //   30    3/2010



	if(    ( (long)iEvent )  >=  numHemisphereEvents    )
		piePos =    iEvent  -  numHemisphereEvents;
	else
		piePos =    iEvent;



																//  Must make adjustments for  SLOW-play...
	if(    m_playSpeedFlt  >  1.0   )
	{

		double    samplesPerVirtualDataFetch  =    (double)samplesPerDataFetch  /  m_playSpeedFlt;



//	OLD:	retSampleIdx  =   ( realCurChunk * samplesPerDataFetch )  	-    (  m_curSubFetch  *  samplesPerVirtualDataFetch  );


		double    realDataFetchCount        =    (double)( m_dataFetchCount - 1 )   / m_playSpeedFlt;   //  m_dataFetchCount is only
						//   - 1 :    m_dataFetchCount  is an Index,  and on first round it has already been incremented to 1( we want0 )
		                //   / m_playSpeed :   See  Load_Next_DataFetch_Forward()   ...we count m_dataFetchCount faster than you would think.  1/03 


		retSampleIdx  =      m_startSample   -    (long)(    realDataFetchCount    *   (double)samplesPerDataFetch   )   
												               -    (  m_curSubFetch  *  (long)samplesPerVirtualDataFetch  );



		remainder      =  (long)(     (double)piePos   *   (   (double)samplesPerPiePos /  m_playSpeedFlt    )        );    //  ...this also advances SLOWER
	}
	else
	{  
//  OLD:  retSampleIdx   =    realCurChunk  *  samplesPerDataFetch;
		retSampleIdx  =       m_startSample   -    (  m_dataFetchCount   *   samplesPerDataFetch   );


		remainder      =     piePos   *  samplesPerPiePos;  
	}
	

	retSampleIdx  -=    remainder;


	//	TRACE(  "curSample:  %d,   [ iEvent  %d ]      SUBchunk  %d,    CURchunk  %d\n",     retSampleIdx,   iEvent,   m_curSubFetch,    m_curChunkPlayer    );





//	ASSERT(   retSampleIdx  >=   0   );   //   BUG *******  Get her when try to play backwards from FileStart in  HOPPING mode   4/03 *************

// **** OK to Ignore ???   Seems like  EventMan::Process_Event_Notification() can handle the Negative value.    4/03 *******


	return   retSampleIdx;  
}





											////////////////////////////////////////


long    AudioPlayer::Calc_SampleIdx_Forward_PPlayer(  unsigned long  iEvent,   long   dataFetchCount     )
{

	 //  ******   NEW   3/11     This way I can hypothetically calc  SampleIdx without being dependant on memberVars ( they come in as Parms in this funct )



	long   numberOfChannels =  2;           // stereo
	long   numberOfBytesInSample = 2;    //  ONLY for 16 bit samples!!!!     FIX**********

	long   retSampleIdx = -1,   remainder,    piePos;    // piePos = { 0 - 9 }

	long   numHemisphereEvents =    ( Get_EventMan().m_numberOfNotificationEvents   -1 )  /2;   //   30     [ HALF of total events ]   a hemisphere(  180 deg ) of traverse



	long   samplesPerDataFetch =     m_bytesInFetch    /    ( numberOfBytesInSample * numberOfChannels );     //  11040;    

	long   samplesPerPiePos      =     samplesPerDataFetch   /   numHemisphereEvents;    //  now  is 368,   Process 368 samples with each Event.    3/2010 




	if(    ( (long)iEvent )  >=  numHemisphereEvents    )
		piePos =    iEvent  -  numHemisphereEvents;
	else
		piePos =    iEvent;



	long   dataFetchIndex =  0;
	/****
	if(   m_dataFetchCount  < 1    )
	{
		ASSERT( 0 );     // should never happen,    BUT   Get here when move FileSlider wihout stopping play   2/11

		dataFetchIndex =  0;    //  will this work OK ????   3/11
	}
	else
		dataFetchIndex   =    (long)m_dataFetchCount     -  1L;   // **************  BAD,  long time bug.  2/2011   For calcs below
	                                                                                      //                            need the Index,  and not the count
	****/


	if(   dataFetchCount  < 1    )
	{

//		ASSERT( 0 );     // should never happen,    *****  BUT   Get here when move FileSlider wihout stopping play   2/11  ******

		dataFetchIndex =  0;    //  will this work OK ????   3/11
	}
	else
		dataFetchIndex   =    dataFetchCount     -  1L;   



	/***  NOTES...

		long     startOffsetBits  =     
						(  m_dataFetchCount   *   dwBytesLocked   )    /  m_playSpeed      //   How many bytes have we PREVIOUSLY fetched in earlier function calls...
												+    ( m_startSample  *  m_bytesPerSample  );    //   ...plus the  ContinuedPlay-OFFSET
	***/



																
	if(    m_playSpeedFlt  >  1.0    )
	{																					//  Must make adjustments for  SLOW-play...


		double   samplesDiff   =    (  (double)dataFetchIndex        *   (double)samplesPerDataFetch   )  /  m_playSpeedFlt;

		double  remainder      =    ( (double)piePos   *  (double)samplesPerPiePos )  /  m_playSpeedFlt;  


		retSampleIdx   =     (long)(  samplesDiff    +   remainder   )      +    m_startSample;		


	}
	else
	{  
		retSampleIdx   =    (  dataFetchIndex        *   samplesPerDataFetch   )   +    m_startSample;


		remainder        =    piePos   *  samplesPerPiePos;  

		retSampleIdx  +=    remainder;


	}
	



//	ASSERT(   retSampleIdx  >=   0   );  // ***BUG: 3/03  Some times get here from WindowPlay at beginning of app.
														  //
														  //  Got here   12/12/11,  when loading a NoteList and Playing, cause  m_startSample =  -1   ????????  




	return      retSampleIdx;   //  this value SOON gets written to    Viewj::m_curSampleIdx  during the Draw()
}






											////////////////////////////////////////
											////////////////////////////////////////


long    AudioPlayer::Calc_Current_Sample_Forward_PPlayer(  unsigned long  iEvent   )
{



	int  dummy =   9;   //    ASSERT( 0);    2/10/2012  *****************  OBSOLETE Yet ???  *************************


									//  A new dataChunk is load on   iEvent =  { 0  or 10 }  for   Get_EventMan().m_numberOfNotificationEvents = 21


	long   numberOfChannels =  2;           // stereo
	long   numberOfBytesInSample = 2;    //  ONLY for 16 bit samples!!!!     FIX**********

	long   retSampleIdx = -1,   remainder,    piePos;    // piePos = { 0 - 9 }

	long   numHemisphereEvents =    ( Get_EventMan().m_numberOfNotificationEvents   -1 )  /2;   //   30     [ HALF of total events ]   a hemisphere(  180 deg ) of traverse


	long   samplesPerDataFetch =     m_bytesInFetch    /    ( numberOfBytesInSample * numberOfChannels );     //  11040;    

	long   samplesPerPiePos      =     samplesPerDataFetch   /   numHemisphereEvents;    //  now  is 368,   Process 368 samples with each Event.    3/2010 



	if(    ( (long)iEvent )  >=  numHemisphereEvents    )
		piePos =    iEvent  -  numHemisphereEvents;
	else
		piePos =    iEvent;



	long   dataFetchIndex =  0;

	if(   m_dataFetchCount  < 1    )
	{

	//	ASSERT( 0 );     // should never happen,    BUT   Get here when move FileSlider wihout stopping play   2/11

		dataFetchIndex =  0;    //  will this work OK ????   3/11
	}
	else
		dataFetchIndex   =    (long)m_dataFetchCount     -  1L;   // **************  BAD,  long time bug.  2/2011   For calcs below
	                                                                                      //                            need the Index,  and not the count




	/***  NOTES...

		long     startOffsetBits  =     
						(  m_dataFetchCount   *   dwBytesLocked   )    /  m_playSpeed      //   How many bytes have we PREVIOUSLY fetched in earlier function calls...
												+    ( m_startSample  *  m_bytesPerSample  );    //   ...plus the  ContinuedPlay-OFFSET
	***/



																//  Must make adjustments for  SLOW-play...
	if(    m_playSpeedFlt  >  1.0    )
	{

		/****
		long    samplesPerVirtualDataFetch  =    samplesPerDataFetch  /  m_playSpeed;


   //  OLD: 	retSampleIdx  =     ( realCurChunk * samplesPerDataFetch )  	+    (  m_curSubFetch  *  samplesPerVirtualDataFetch  );

		long    realDataFetchCount        =    ( m_dataFetchCount - 1 )   / m_playSpeed;   //  m_dataFetchCount is only
						//   - 1 :    m_dataFetchCount  is an Index,  and on first round it has already been incremented to 1( we want0 )
		                //   / m_playSpeed :   See  Load_Next_DataFetch_Forward()   ...we count m_dataFetchCount faster than you would think.  1/03 



		retSampleIdx   =    (  realDataFetchCount    *   samplesPerDataFetch   )   
												   +    (  m_curSubFetch  *  samplesPerVirtualDataFetch  )    +  m_startSample;


		remainder       =     piePos   *   ( samplesPerPiePos /  m_playSpeed  );    //  ...this also advances SLOWER
		

		retSampleIdx  +=    remainder;
		****/


	
//		long      samplesDiff  =    (  m_dataFetchCount   *   samplesPerDataFetch   )  /  m_playSpeed;
		double   samplesDiff  =    (  (double)dataFetchIndex        *   (double)samplesPerDataFetch   )  /  m_playSpeedFlt;



		double   remainder        =    (  (double)piePos   *  (double)samplesPerPiePos )  /  m_playSpeedFlt;  


		retSampleIdx   =   (long)(    samplesDiff    +   remainder  )        +    m_startSample;
		
	}
	else
	{  
//		retSampleIdx   =    (  m_dataFetchCount   *   samplesPerDataFetch   )   +    m_startSample;
		retSampleIdx   =    (  dataFetchIndex        *   samplesPerDataFetch   )   +    m_startSample;


		remainder        =    piePos   *  samplesPerPiePos;  

		retSampleIdx  +=    remainder;
	}
	



//	TRACE(  "curSample:  %d,   [ iEvent  %d ]      SUBchunk  %d,    CURchunk  %d\n",     retSampleIdx,   iEvent,   m_curSubFetch,    m_curChunkPlayer    );



// *****************   Is this a BIG Deal ??? ***************************************

//	ASSERT(   retSampleIdx  >=   0   );  // ***BUG: 3/03    12/2011  Some times get here from WindowPlay at beginning of app.



	return   retSampleIdx;   //  this value SOON gets written to    Viewj::m_curSampleIdx  during the Draw()
}




											////////////////////////////////////////


long    AudioPlayer::Calc_Current_Sample_Backward_PPlayer(  unsigned long  iEvent   )
{

									//  A new dataChunk is load on   iEvent =  { 0  or 10 }  for   Get_EventMan().m_numberOfNotificationEvents = 21


	long   numberOfChannels =  2;           // stereo
	long   numberOfBytesInSample = 2;    //  ONLY for 16 bit samples!!!!     FIX**********

	long   retSampleIdx = -1,   remainder = -1,    piePos;    // piePos = { 0 - 9 }

	long   numHemisphereEvents =    ( Get_EventMan().m_numberOfNotificationEvents   -1 )  /2;   //  [ HALF events ]should be 30,  a hemisphere(  180 deg ) of traverse



	long   samplesPerDataFetch =     m_bytesInFetch /  ( numberOfBytesInSample * numberOfChannels );     //   11040   samps   

	long   samplesPerPiePos       =     samplesPerDataFetch /  numHemisphereEvents;    //   1104  samples   3/11



	if(    ( (long)iEvent )  >=  numHemisphereEvents    )
		piePos =    iEvent  -  numHemisphereEvents;
	else
		piePos =    iEvent;



																//  Must make adjustments for  SLOW-play...
	if(    m_playSpeedFlt  >  1.0   )
	{

		/****
		long    samplesPerVirtualDataFetch  =    samplesPerDataFetch  /  m_playSpeed;



//	OLD:	retSampleIdx  =   ( realCurChunk * samplesPerDataFetch )  	-    (  m_curSubFetch  *  samplesPerVirtualDataFetch  );


		long    realDataFetchCount        =    ( m_dataFetchCount - 1 )   / m_playSpeed;   //  m_dataFetchCount is only
						//   - 1 :    m_dataFetchCount  is an Index,  and on first round it has already been incremented to 1( we want )
		                //   / m_playSpeed :   See  Load_Next_DataFetch_Forward()   ...we count m_dataFetchCount faster than you would think.  1/03 

		retSampleIdx  =      m_startSample   -    (  realDataFetchCount    *   samplesPerDataFetch   )   
												               -    (  m_curSubFetch  *  samplesPerVirtualDataFetch  );



		remainder      =      piePos   *   ( samplesPerPiePos /  m_playSpeed  );    //  ...this also advances SLOWER
		****/



		/***  OLD,  kind of buggy  3/11
		long   samplesDiff   =    (  m_dataFetchCount   *   samplesPerDataFetch   )   +    ( piePos   *  samplesPerPiePos );  


		retSampleIdx  =     m_startSample   -   (  samplesDiff  /  m_playSpeed  );
		****/


		long  blocksOffset         =      (m_dataFetchCount -1)   *   samplesPerDataFetch;   //  offset is  RIGHT( big sampIdx)    to    LEFT (small sampIdx)

		long  remaindersOffset  =      (piePos +1)   *   samplesPerPiePos;  



		double  totalOffset              =    (  (double)blocksOffset   +   (double)remaindersOffset )   / m_playSpeedFlt; 

		long   currentSampleIdx  =     m_startSample   -   (long)totalOffset;  



		if(     currentSampleIdx  <  0    )
		{
			int   dummy =   9;      //    land here for a little bit when SlowDown speeds play backwards to Files Beginning    3/11
		}

		retSampleIdx  =   currentSampleIdx;
	}
	else
	{  

//   Calcs just before THIS Block-dataLoad:      dataFetchOffset  =     (    (m_dataFetchCount   + 1)   *   dwBytesLocked   )    / m_playSpeed; 
	

		/***
		remainder      =      piePos   *  samplesPerPiePos;  

//		retSampleIdx  =     m_startSample   -    (  ( m_dataFetchCount -1 )    *   samplesPerDataFetch   )   -    remainder;
		retSampleIdx  =     m_startSample   -    (  ( m_dataFetchCount     )    *   samplesPerDataFetch   )   -    remainder;   // *** GOOD   3/29/11
		***/

		long  blocksOffset         =     (m_dataFetchCount -1)   *   samplesPerDataFetch;   //  offset is  RIGHT( big sampIdx)    to    LEFT (small sampIdx)


		long  remaindersOffset  =      (piePos +1)   *   samplesPerPiePos;  


		long  totalOffset              =    blocksOffset   +   remaindersOffset; 

		long   currentSampleIdx  =     m_startSample   -   totalOffset;  

		retSampleIdx  =   currentSampleIdx;
	}
	


	//	TRACE(  "curSample:  %d,   [ iEvent  %d ]      SUBchunk  %d,    CURchunk  %d\n",     retSampleIdx,   iEvent,   m_curSubFetch,    m_curChunkPlayer    );


//	ASSERT(   retSampleIdx  >=   0   );   //   BUG *******  Get her when try to play backwards from FileStart in  HOPPING mode   4/03 *************

// **** OK to Ignore ???   Seems like  EventMan::Process_Event_Notification() can handle the Negative value.    4/03 *******


	return   retSampleIdx;  
}




											////////////////////////////////////////


void   StreamingAudioplayer::Load_Next_DataFetch_Backward_PPlayer_NoAudio(   unsigned long   iEvent    )
{										 





ASSERT( 0 );   //  ****************   NOT USED ???     3/2012  ***********************************



		//  *****    New,  for preroll      10/11


								//  Loads the next 'chunk' of sample data from memory(BitSourceAudioMS) and ALLOWS it to play
//    HRESULT           hr;
//    DWORD            dwStartOfs;
    static DWORD   dwLastPlayPos;
//    VOID              *lpvData;
    DWORD            dwBytesLocked;
    UINT                 cbBytesRead; 
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
	//	return;     ....let it keep going, WITHOUT Fetching new file data,  just updating vars.
	}

	unsigned long   hemisEvent        =    ( (unsigned long)( Get_EventMan().m_numberOfNotificationEvents -1 ) )  / 2;    // The event at 180 dergees
	


//	DWORD    numBytesToPlay =     (DWORD)Get_Waves_ByteCount_toEnd();   ***BAD FUNCTION for Player***   3/11  
	DWORD    numBytesToPlay =     m_startSample   *  m_bytesPerSample;



	/*****
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
														   &lpvData,               // Address of lock start. ( where we will bytes to )
														   &dwBytesLocked,   // Number of bytes locked.
															 NULL,             // Address of wraparound lock.
															 NULL,             // Number of wraparound bytes.
															 0 );                 // Flags.

	m_bytesInFetch =     dwBytesLocked;     // number of bytes we will read
	*****/
	dwBytesLocked   =    BLOCKLOCKEDSIZE;      //  44160    ***********   OK ??? ********************
	m_bytesInFetch   =    BLOCKLOCKEDSIZE;      //  44160    dwBytesLocked;    // Number of bytes we will read for this Chunk


	UINT     bytsInSect  =     TWOSECBYTES   /   (  Get_EventMan().m_numberOfNotificationEvents  -1 );     // How many bytes in an Event 





// ***************************  BUGGY,  generates the same value TWICE .....


/****  code for FORWARD Play

	long     startOffsetBits  =     
						(  m_dataFetchCount   *   dwBytesLocked   )    /  m_playSpeed      //   How many bytes have we PREVIOUSLY fetched in earlier function calls...
												+    ( m_startSample  *  m_bytesPerSample  );    //   ...plus the  ContinuedPlay-OFFSET
****/

								 //  what is the  VIRTUAL-FilePosition  that  Lock()  intends for THIS dataFetch ...

/*****  Almost CURRENT code
	long    dataFetchOffset =   0;

															//   Not sure why  ( m_dataFetchCount -1 ) 
	if(   m_dataFetchCount  > 0   )
		dataFetchOffset  =       (   ( m_dataFetchCount -1 )    *   dwBytesLocked    )    /  m_playSpeed; 
******/	
	long     dataFetchOffset  =  (long)(       (    (double)(m_dataFetchCount   + 1)   *   (double)dwBytesLocked   )    / m_playSpeedFlt        ); 

				//    Need  +1  because,  even at the first iteration ( m_dataFetchCount= 0 ) we need to backup the WIDTH of a BlockLoad  ( dwBytesLocked/m_playSpeed )
				//    to get to the START of the Block that was PREVIOUS to the LastBlock that was played.
				//    Go to   SoundHelper::Continue_Play_Backwards()   to see how I rounded  'lastPlayedSample'  to be on BlockLoad BOUNDARIES
			



//	dataFetchOffset      =       (   ( m_dataFetchCount     )    *   dwBytesLocked    )    /  m_playSpeed;    // Is this better???  


	    // ******   ...OR would   ( m_dataFetchCount -1 )   be more ACCURATE ????   10/02   **************************


	/***
	long     startOffsetBits  =                      ( m_startSample   *   m_bytesPerSample  )      //   ...plus the  ContinuedPlay-OFFSET
						            //      -   (    (    m_dataFetchCount          *   dwBytesLocked   )    /  m_playSpeed    );        //   How many bytes have we PREVIOUSLY fetched in earlier function calls...
						                    -   (    (  ( m_dataFetchCount -1 )    *   dwBytesLocked   )    /  m_playSpeed    );        //   How many bytes have we PREVIOUSLY fetched in earlier function calls...
	****/

	long   startOffsetBits  =     ( m_startSample  *  m_bytesPerSample )    -  dataFetchOffset;

	if(     startOffsetBits  <  0    )   
	{
		             //  Get here if play backwards to beginning of file    3/30/11
		int   dummy =  9;
		startOffsetBits =  0;
		m_doneFetchingPlayBytes =    true; 
	}






	BYTE   *delayWriteByte     =     m_bitSource->Get_DelayBuffer_Write_Byte_BackwardsPlay(  (long)iEvent,   (long)m_dataFetchCount  );   // In the  WAV-DelayBuffer
	if(        delayWriteByte  ==   NULL  )
	{
	//	delayWriteByte =    (BYTE*)lpvData;  
		delayWriteByte =    m_backwardsBits;    //   PLAYER:  is NOT using a  WAV-DelayBuffer    [  eventually lpvData  points to HARDWARE  circ-soundBuffer
	}



	bool  retHitEOF =    false;  


	if(   ! m_doneFetchingPlayBytes    )
	{

		 if(    ! m_bitSource->Fetch_Streaming_Samples_Direct_PPlayer(  startOffsetBits,           //  how far to offset WITHIN Samplebits
																										dwBytesLocked,          //  number of bytes to read.

																										delayWriteByte,   //     m_backwardsBits,   //   (BYTE*)lpvData,          //  destination  MEMORY Block,    for Streaming-play it is in  the MSoft  SoundBuffer																						    

																			  						  &cbBytesRead,              //  number of bytes ACTUALLY read.
																										m_srcBytesProcessed,													
																										retHitEOF,

																										false,             //  Forward flag   *********************

																										m_auditionCode,    // ????  Not really used.  1/10       is 0   Where was this set ????   0 =  AnimeStream::NORMAL

																										true,     //  soundManControlsVolume,  YES, we want Soundman's variable to control the sound
																										
																									     m_rightChannelPercent,
																							             m_playSpeedFlt,
																										 retErrorMesg  )     )
		{  m_doneFetchingPlayBytes =    true;   }  
	}



	if(   retHitEOF  )
	{
		m_doneFetchingPlayBytes =    true;  

//		return;  //  *****************************  WANT this ????  ***********************
	}

	ASSERT(   dwBytesLocked    ==   BLOCKLOCKEDSIZE    );






	double   retOverbriteRatio  =  -2.0;
	long      usedSampleCount =   -1;             //    for SlowedSpeeds,  figure out how MANY samples in SndSample actually contain data


	if(   ! sPitchCalcer->m_spitchCalcsSndSample->Apply_Full_Filtering_FixedScaling(   m_bitSource->m_topFreqFilterLimit,    m_bitSource->m_bottomFreqFilterLimit,   
																																m_bitSource->m_inputScalePercent,   retOverbriteRatio,	  retErrorMesg   )    )
	{
		if(    ! retErrorMesg.IsEmpty()    )   //  This is good for DEBUG, cause we can see a falure by the CString message without halting the play of the WAV  1/10
		{
			int   dummy =  8;
		}
	}
	//	TRACE(   "Apply_Full_Filtering_FixedScaling [   OverbriteRatioInPercent =  %f   ]  \n",    retOverbriteRatio * 100.0   );     //   * 100.0 ...in percent	





							 	//  a  LARGE  CIRCULAR-Que  for  OLD SndSamples  ....so I can read BACK in TIME for very slow speeds that dont fetch many samples    2/12

	long  bytesToProcessAlt =    m_bitSource->Get_Biggest_SndSample_Index(); 

	long  bytesToProcess     =     sPitchCalcer->Get_SndSamples_Valid_Count(  m_playSpeedFlt  );  // ***** CAREFUL, not fully tested.  1/29/12  ****** 

	if(   bytesToProcess  !=   bytesToProcessAlt    )
	{	int dummy =  9;  }


	sPitchCalcer->Add_SndSample_Samples_to_SndSampleCircularQue(  bytesToProcess  );





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



											////////////////////////////////////////


void   StreamingAudioplayer::Load_Next_DataFetch_Backward_PPlayer_OMIT(   unsigned long   iEvent    )
{										 


ASSERT( 0 );   //  ****************   NOT USED ???     3/2012  ***********************************



								//  Loads the next 'chunk' of sample data from memory(BitSourceAudioMS) and ALLOWS it to play
    HRESULT           hr;
    DWORD            dwStartOfs;
    static DWORD   dwLastPlayPos;
    VOID              *lpvData;
    DWORD            dwBytesLocked;
    UINT                 cbBytesRead; 
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

	//	return;     ....let it keep going, WITHOUT Fetching new file data,  just updating vars.
	}

	unsigned long   hemisEvent        =    ( (unsigned long)( Get_EventMan().m_numberOfNotificationEvents -1 ) )  / 2;    // The event at 180 dergees
	


//	DWORD    numBytesToPlay =     (DWORD)Get_Waves_ByteCount_toEnd();   ***BAD FUNCTION for Player***   3/11  
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
														   &lpvData,               // Address of lock start. ( where we will bytes to )
														   &dwBytesLocked,   // Number of bytes locked.
															 NULL,             // Address of wraparound lock.
															 NULL,             // Number of wraparound bytes.
															 0 );                 // Flags.

	m_bytesInFetch =     dwBytesLocked;     // number of bytes we will read



// ***************************  BUGGY,  generates the same value TWICE .....


/****  code for FORWARD Play

	long     startOffsetBits  =     
						(  m_dataFetchCount   *   dwBytesLocked   )    /  m_playSpeed      //   How many bytes have we PREVIOUSLY fetched in earlier function calls...
												+    ( m_startSample  *  m_bytesPerSample  );    //   ...plus the  ContinuedPlay-OFFSET
****/

								 //  what is the  VIRTUAL-FilePosition  that  Lock()  intends for THIS dataFetch ...

/*****  Almost CURRENT code
	long    dataFetchOffset =   0;

															//   Not sure why  ( m_dataFetchCount -1 ) 
	if(   m_dataFetchCount  > 0   )
		dataFetchOffset  =       (   ( m_dataFetchCount -1 )    *   dwBytesLocked    )    /  m_playSpeed; 
******/	

 
	long     dataFetchOffset  =  (long)(       (    (double)(m_dataFetchCount  + 1)   *   (double)dwBytesLocked   )    /  m_playSpeedFlt     ); 

				//    Need  +1  because,  even at the first iteration ( m_dataFetchCount= 0 ) we need to backup the WIDTH of a BlockLoad  ( dwBytesLocked/m_playSpeed )
				//    to get to the START of the Block that was PREVIOUS to the LastBlock that was played.
				//    Go to   SoundHelper::Continue_Play_Backwards()   to see how I rounded  'lastPlayedSample'  to be on BlockLoad BOUNDARIES
			



//	dataFetchOffset      =       (   ( m_dataFetchCount     )    *   dwBytesLocked    )    /  m_playSpeed;    // Is this better???  


	    // ******   ...OR would   ( m_dataFetchCount -1 )   be more ACCURATE ????   10/02   **************************


	/***
	long     startOffsetBits  =                      ( m_startSample   *   m_bytesPerSample  )      //   ...plus the  ContinuedPlay-OFFSET
						            //      -   (    (    m_dataFetchCount          *   dwBytesLocked   )    /  m_playSpeed    );        //   How many bytes have we PREVIOUSLY fetched in earlier function calls...
						                    -   (    (  ( m_dataFetchCount -1 )    *   dwBytesLocked   )    /  m_playSpeed    );        //   How many bytes have we PREVIOUSLY fetched in earlier function calls...
	****/



	long   startOffsetBits  =     ( m_startSample  *  m_bytesPerSample )    -  dataFetchOffset;

	if(     startOffsetBits  <  0    )   
	{
		             //  Get here if play backwards to beginning of file    3/30/11
		int   dummy =  9;
		startOffsetBits =  0;
		m_doneFetchingPlayBytes =    true; 
	}






	BYTE   *delayWriteByte     =     m_bitSource->Get_DelayBuffer_Write_Byte_BackwardsPlay(  (long)iEvent,   (long)m_dataFetchCount  );   // In the  WAV-DelayBuffer
	if(        delayWriteByte  ==   NULL  )
	{
	//	delayWriteByte =    (BYTE*)lpvData;  
		delayWriteByte =    m_backwardsBits;    //   PLAYER:  is NOT using a  WAV-DelayBuffer    [  eventually lpvData  points to HARDWARE  circ-soundBuffer
	}



	bool  retHitEOF =    false;  


	if(   ! m_doneFetchingPlayBytes    )
	{

		 if(    ! m_bitSource->Fetch_Streaming_Samples_Direct_PPlayer(  startOffsetBits,           //  how far to offset WITHIN Samplebits
																										dwBytesLocked,          //  number of bytes to read.

																										delayWriteByte,   //     m_backwardsBits,   //   (BYTE*)lpvData,          //  destination  MEMORY Block,    for Streaming-play it is in  the MSoft  SoundBuffer																						    

																			  						  &cbBytesRead,              //  number of bytes ACTUALLY read.
																										m_srcBytesProcessed,													
																										retHitEOF,
																										false,             //  Forward flag
																										m_auditionCode,    // ????  Not really used.  1/10       is 0   Where was this set ????   0 =  AnimeStream::NORMAL

																										true,     //  soundManControlsVolume,  YES, we want Soundman's variable to control the sound
																										
																									     m_rightChannelPercent,
																							             m_playSpeedFlt,
																										 retErrorMesg  )     )
		{  m_doneFetchingPlayBytes =    true;   }  
	}



	if(   retHitEOF  )
	{
		m_doneFetchingPlayBytes =    true;  

//		return;  //  *****************************  WANT this ????  ***********************
	}

	ASSERT(   dwBytesLocked    ==   BLOCKLOCKEDSIZE    );





	if(   delayWriteByte  ==   NULL  )    //  delayWriteByte has same value as up above  ( For reverse Play we do NOT want a delay in sound.  Just use the last block loaded
	{	ASSERT( 0 );   }     //  NOT  POSSIBLE   3/30/11    	
	else
	{		
			//    PitchPLAYER does not go here, there is no delay mechanism for it.   It writes directly to the hardware. Volumn is adjusted in  Fetch_Streaming_Samples_Direct_PPlayer()

		BYTE   *src  =                 delayWriteByte;   
		BYTE   *dst  =     (BYTE*)lpvData;	

		memmove(    dst,    src,     (long)dwBytesLocked    );
	}



	m_infcDSoundBuffer->Unlock(   lpvData,    dwBytesLocked,    NULL,    0   );       // MOVED up to HERE for optimization. 






	double   retOverbriteRatio  =  -2.0;
	long      usedSampleCount =   -1;             //    for SlowedSpeeds,  figure out how MANY samples in SndSample actually contain data


	if(   ! sPitchCalcer->m_spitchCalcsSndSample->Apply_Full_Filtering_FixedScaling(   m_bitSource->m_topFreqFilterLimit,    m_bitSource->m_bottomFreqFilterLimit,   
																																m_bitSource->m_inputScalePercent,   retOverbriteRatio,	  retErrorMesg   )    )
	{  if(    ! retErrorMesg.IsEmpty()    )   //  This is good for DEBUG, cause we can see a falure by the CString message without halting the play of the WAV  1/10
		{
			int   dummy =  8;
		}
	}
	//	TRACE(   "Apply_Full_Filtering_FixedScaling [   OverbriteRatioInPercent =  %f   ]  \n",    retOverbriteRatio * 100.0   );     //   * 100.0 ...in percent	




							 	//  a  LARGE  CIRCULAR-Que  for  OLD SndSamples  ....so I can read BACK in TIME for very slow speeds that dont fetch many samples    2/12

	long  bytesToProcessAlt =    m_bitSource->Get_Biggest_SndSample_Index(); 

	long  bytesToProcess     =     sPitchCalcer->Get_SndSamples_Valid_Count(  m_playSpeedFlt  );  // ***** CAREFUL, not fully tested.  1/29/12  ****** 

	if(   bytesToProcess  !=   bytesToProcessAlt    )
	{	int dummy =  9;  }


	sPitchCalcer->Add_SndSample_Samples_to_SndSampleCircularQue(  bytesToProcess  );





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



											////////////////////////////////////////


void    StreamingAudioplayer::Load_Next_DataFetch_Forward(    unsigned long   iEvent    )
{	

	if(    m_playSpeedFlt  ==  1.0   )
	{
		Load_Next_DataFetch_Forward_RegularSpeed(   iEvent   );
	}
	else if(    m_playSpeedFlt  >  1.0   )
	{
		Load_Next_DataFetch_Forward_SlowSpeed(       iEvent   );
	}
	else
	{	ASSERT( 0 );
	}
}



											////////////////////////////////////////


void   StreamingAudioplayer::Load_Next_DataFetch_Backward(   unsigned long   iEvent    )
{	

	if(    m_playSpeedFlt  ==  1.0   )
	{
		Load_Next_DataFetch_Backward_RegularSpeed(   iEvent   );
	}
	else if(    m_playSpeedFlt  >  1.0   )
		Load_Next_DataFetch_Backward_SlowSpeed(       iEvent   );
	else
	{	ASSERT( 0 );
	}
}



											////////////////////////////////////////


void    StreamingAudioplayer::Load_Next_DataFetch_Forward_RegularSpeed(    unsigned long   iEvent    )
{										 

											//  Loads the next  'chunk' of sample data  from file and plays
    HRESULT          hr;
    DWORD            dwStartOfs;
    static DWORD   dwLastPlayPos;
    VOID               *lpvData;
    DWORD             dwBytesLocked;
    UINT                  cbBytesRead;
	CString     retErrorMesg;


	bool     readStaticBitsDEBUG    =      false;     // ******  ALWAYS off, now   5/07 DEBUG switch ******

//	m_buffersSrcByte =  NULL;



	if(    !IsOK()     )
	{   
		ASSERT( false );	
		return; 	
	}

	ASSERT(   m_playSpeedFlt  ==  1.0   );					// ****  INSTALL  Error-MESSAGING ******

//	ASSERT(   m_slowPlayStep   ==   SPEEDZONESIZE  );

	ASSERT(   m_dataFetchCount  >=  0   );


	short  channelCode =    Get_StereoChannelCode();



	if(    m_doneFetchingPlayBytes     )     
	{				    	//  2)    ...just before   EventMan::Process_Event_Notification()  [  with event =  Get_EventMan().m_numberOfNotificationEvents -1

//		StopPlaying_Hardware();       //      ....Better to set flags later
//		return;   //  Let it keep going,  but not try to load new data,  just rehash the existing.
	}


	if(   m_doRealtimePitchDetection     )   // **** LATER FIX
	{
		AfxMessageBox(  "StreamingAudioplayer::Load_Next_DataFetch_Forward_RegularSpeed FAILED,  m_spitchCalcsBuffer is NULL for Realtime Pitch Detect."  );
		return;
	}





//	DWORD   numBytesToPlay   =     ( DWORD )Get_Waves_ByteCount_toEnd();     //  OLD:   will shorten for play of PART of the sample that is SELECTED with RangeFrame


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
														  &lpvData,               // Address of lock start. ( DEST-memory where we will bytes to )
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



	m_bytesInFetch     =    dwBytesLocked;    // Number of bytes we will read for this Chunk

	UINT     bytsInSect  =     TWOSECBYTES   /   (  Get_EventMan().m_numberOfNotificationEvents  -1 );     // How many bytes in an Event 





								 //  what is the  ' VIRTUAL-FilePosition'   that  Lock()  intends for THIS dataFetch

	long     startOffsetBits  =     
					(long)(    	(  (double)m_dataFetchCount   *   (double)dwBytesLocked   )    /  m_playSpeedFlt      )      //   How many bytes have we PREVIOUSLY fetched in earlier function calls...
												+    ( m_startSample  *  m_bytesPerSample  );    //   ...plus the  ContinuedPlay-OFFSET






				//  ***  Whast about  the 44 BYTES in the File's Header  ???    May have to deal with it some day!!!   Too small to wory about ???   9/2003

	BYTE   *dstData      =      ( BYTE* )lpvData;





	if(    m_playSpeedFlt  ==   1.0    )
	{

		bool    retHitEOF =    false;  
		long    retScalePercent =  -1;

		if(   ! m_doneFetchingPlayBytes    )    
		{
			 if(     ! m_bitSource->Fetch_Streaming_Samples_Direct(       startOffsetBits,           //  how far to offset WITHIN Samplebits
																										dwBytesLocked,          //  number of bytes to read.

																										(BYTE*)lpvData,          //  destination  MEMORY Block,    for Streaming-play it is in  the MSoft  SoundBuffer
																								 //        (BYTE*)m_spitchCalcsBuffer,  

																			  						  &cbBytesRead,              //  number of bytes ACTUALLY read.
																										m_srcBytesProcessed,													
																										retHitEOF,
																										true,
																										m_auditionCode,  
																									//	channelCode,   //   Is_SourceChannel_Left(),
																										true,     //  soundManControlsVolume,  YES, we want Soundman's variable to control the sound
																										retErrorMesg   )     )
			 {  m_doneFetchingPlayBytes =    true;   
			 }		// ??? OK     ...to make sure we stop playing		
			 else
			 {

				 if(   m_doRealtimePitchDetection   )
				 {
					 ASSERT( 0 );
					 /******
					 if(       m_spitchCalcsBuffer    ==  NULL    
						 ||   m_spitchCalcsSndSample  ==  NULL 
						//  ||   m_spitchCalcsSampleNoSlowDown  ==  NULL 
						 )
					 {
						 ASSERT( 0 );   //  INSTALL error message 
						 AfxMessageBox(   "StreamingAudioplayer::Load_Next_DataFetch_Forward FAILED,  m_spitchCalcsBuffer or  m_spitchCalcsSndSample  is null. "   );
					 }
					 else
					 {  
						 char  *src =    (char*)lpvData;      // ******   BAD,  this should be done inside of  Fetch_Streaming_Samples_Direct
						 char  *dst =    m_spitchCalcsBuffer;  

						 for(   long  i =0;      i <  (long)dwBytesLocked;      i++    )	      //  Read the bytes from BitSourceAudioMS memory to the hardware
						 {
							*dst  =   *src;
							 src++;     dst++;
						 }
				

						long     bytesPerSample16  =    4;
						long     totalSamples         =    BLOCKLOCKEDSIZE  /  bytesPerSample16;
						double  retOverbriteRatio =  -2.0;

//						long    inputScalePercent  =     700;     //   476      **** ADJUST **************    500[ Bad sign, no overbrite, weak on Texas Flood ]   
		


						if(    ! m_spitchCalcsSndSample->CopyBits_From_16bit_Sample(   Get_StereoChannelCode(), 	 (char*)(  &( m_spitchCalcsBuffer[0] )   ),    
																																				               0,   totalSamples,   retErrorMesg   )    )
							AfxMessageBox(  retErrorMesg  );


//						if(   ! m_spitchCalcsSndSample->Apply_Full_Filtering(   m_topFreqFilterLimit,   m_bottomFreqFilterLimit,   retScalePercent,    retErrorMesg   )   )
						if(   ! m_spitchCalcsSndSample->Apply_Full_Filtering_FixedScaling(   m_topFreqFilterLimit,   m_bottomFreqFilterLimit,   m_bitSource->m_inputScalePercent,  retOverbriteRatio,	  retErrorMesg   )    )
						{

							//  AfxMessageBox(  retErrorMesg  );   NO!!!  will virtually crash app.  Get this if silence in track.  OK it we just keep goint.  1/10

							if(    ! retErrorMesg.IsEmpty()    )   //  This is good for DEBUG, cause we can see a falure by the CString message without halting the play of the WAV  1/10
							{
								int   dummy =  8;
							}
						}

//						TRACE(   "Apply_Full_Filtering_FixedScaling [   OverbriteRatioInPercent =  %f   ]  \n",    retOverbriteRatio * 100.0   );     //   * 100.0 ...in percent

					 }    //   else      m_spitchCalcsBuffer and  m_spitchCalcsSndSample  are NOT null
					*****/

				 }     //   else    m_bitSource->Fetch_Streaming_Samples_Direct( ) is sucessful


	
			 }   //   if(   doRealtimePitchAnalysis   )	
		

		}




		if(    retHitEOF   )
		{
			ASSERT(  0   );     //  Do not normally reach this.     1/2003
			m_doneFetchingPlayBytes =    true;  
		}
				
		ASSERT(   dwBytesLocked   ==    BLOCKLOCKEDSIZE   );    //  ** WANT an ERROR message ???? ***



		m_srcBytesProcessed  +=     dwBytesLocked;

		m_dataFetchCount++;   //  count how many times this funct was called during   
		m_curSubFetch =  0;
	}


	m_infcDSoundBuffer->Unlock(    lpvData,    dwBytesLocked,    NULL,    0   );
}





											////////////////////////////////////////


void    StreamingAudioplayer::Load_Next_DataFetch_Forward_PPlayer_OMIT(    unsigned long   iEvent    )
{										 



ASSERT( 0 );   //  ****************   NOT USED ???     3/2012  ***********************************



											//  Loads the next  'chunk' of sample data  from file and plays
    HRESULT          hr;
    DWORD            dwStartOfs;
    static DWORD   dwLastPlayPos;
    VOID               *lpvData;
    DWORD             dwBytesLocked;
    UINT                  cbBytesRead;
	CString     retErrorMesg;



	bool    allowAudibleChangeOfStereoBalance  =  true;    //     **********    ADJUST    3/11   **********************
																					//   Do NOT have to HEAR a stereoBalance shift for DETECTION if I do not want to. 


	bool      roundUsersStereoBalanceInput =   true;

	short     roundToleranceInPercent  =   10;     //  10%     If difference between Left and Right Stereo channels is less than this, then we round to equal balance 11/11




	bool     readStaticBitsDEBUG    =      false;     // ******  ALWAYS off, now   5/07 DEBUG switch ******




	short	  approxRightChannelPercent  =		m_rightChannelPercent;    //   default,  without rounding 

	short   absDifference  =    abs(   50  -  m_rightChannelPercent   );  


	if(         m_rightChannelPercent  !=  50 
		&&    absDifference   <  roundToleranceInPercent        //  Do I want to round this if the user is close ???
		&&    roundUsersStereoBalanceInput   ) 
	{
		approxRightChannelPercent  =	50;	
 //       TRACE(  "Rounding  STEREO Balance to EQUAL  \n"  );
	}
	else
	{
//		TRACE(  "                      No Rounding \n"  );
	}


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
														  &lpvData,               // Will receive address of Lock's Start. ( DEST-memory where we will bytes to )
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



	m_bytesInFetch     =    dwBytesLocked;    // Number of bytes we will read for this Chunk

	UINT     bytsInSect  =     TWOSECBYTES   /   (  Get_EventMan().m_numberOfNotificationEvents  -1 );     // How many bytes in an Event 





								    //  what is the  ' VIRTUAL-FilePosition'   that  Lock()  intends for THIS dataFetch  ( is just multiples of 44,160


									//  'startOffsetBits'  is is OUTPUT coords, so is kind of a virtual value while imagining a 44,100 signal with 16bit stereo

	long     startOffsetBits  =     
					(long)(     	(  (double)m_dataFetchCount   *   (double)dwBytesLocked   )    /  m_playSpeedFlt    )     //   How many bytes have we PREVIOUSLY fetched in earlier function calls...
												+    ( m_startSample  *  m_bytesPerSample  );    //   ...plus the  ContinuedPlay-OFFSET


				//  ***  What about  the 44 BYTES in the File's Header  ???    May have to deal with it some day!!!   Too small to wory about ???   9/2003





	bool    retHitEOF =    false;  
	long    retScalePercent =  -1;


	BYTE   *delayWriteByte  =    m_bitSource->Get_DelayBuffer_Write_Byte(    (long)iEvent,    (long)m_dataFetchCount    );

	if(   delayWriteByte  ==   NULL  )
		delayWriteByte =    (BYTE*)lpvData;   // PLAYER:   this would mean we are NOT using a buffer   [  lpvData  points to HARDWARE  circ-soundBuffer





	if(    ! m_bitSource->Fetch_Streaming_Samples_Direct_PPlayer(       startOffsetBits,           //  how far to offset WITHIN Samplebits
																										dwBytesLocked,          //  number of bytes to read.

																										delayWriteByte,   //    (BYTE*)lpvData,     destination  MEMORY Block,    for Streaming-play it is in  the MSoft  SoundBuffer																						    

																			  						  &cbBytesRead,              //  number of bytes ACTUALLY read.
																										m_srcBytesProcessed,													
																										retHitEOF,
																										true,      //  Forward flag

																										m_auditionCode,    // ????  Not really used.  1/10       is 0   Where was this set ????   0 =  AnimeStream::NORMAL

																										true,     //  soundManControlsVolume,  YES, we want Soundman's variable to control the sound																																																			

																									     approxRightChannelPercent,   //  m_rightChannelPercent,   *** Do rounding *****

																										 m_playSpeedFlt,

																										retErrorMesg   )     )

	{  m_doneFetchingPlayBytes =    true;   

		m_infcDSoundBuffer->Unlock(    lpvData,    dwBytesLocked,    NULL,    0   );  //   ***** MOVED up here  for faster processing.  3/11   OK ????
	}		// ??? OK     ...to make sure we stop playing		
	
	else
	{
		BYTE   *delayReadByte     =     m_bitSource->Get_DelayBuffer_Read_Byte(  (long)iEvent,    (long)m_dataFetchCount   ); 																									  
		if(         delayReadByte   !=   NULL   )
		{		
			BYTE  *dstHardware =   (BYTE*)lpvData;    //  Now copy  Play-Bytes  to the  HARDWARE's Circular-Buffer


						//    PitchPLAYER does not go here, there is no delay mechanism for it.   It writes directly to the hardware. Volumn is adjusted in  Fetch_Streaming_Samples_Direct_PPlayer()

			long  volumeInPercent =     *(  m_bitSource->m_wavVolumeSoundmanAddr   );    


			if(   ! Apply_Volume_Adjust_PPlayer(   (long)dwBytesLocked,  volumeInPercent,   delayReadByte,   dstHardware,   allowAudibleChangeOfStereoBalance,  retErrorMesg  )    )
			{
				ASSERT( 0 );     //  Doing this here avoids a delay for user when moving the slider.  Can also change the stereoBalance.   3/11
				AfxMessageBox(  retErrorMesg  );
			}
		}
		else	{  }  //  do nothing,  this means that we are NOT using a delayBuffer
				


		m_infcDSoundBuffer->Unlock(    lpvData,    dwBytesLocked,    NULL,    0   );  //   ***** MOVED up here  for faster processing.  3/11   OK ????




//		if(    m_doRealtimePitchDetection    )    // *********  PROBLEM ???    12/5/11      *************************
		if(    sPitchCalcer->m_playModeUsingNotelist  ==   0     )     //    0:   Detect
		{


			if(        sPitchCalcer->m_spitchCalcsSndSample  ==  NULL     
				//  ||   m_spitchCalcsBuffer    ==  NULL    				
			//		 ||   m_spitchCalcsSampleNoSlowDown  ==  NULL 
			  )
			{
				ASSERT( 0 );   //  INSTALL error message 
				AfxMessageBox(   "StreamingAudioplayer::Load_Next_DataFetch_Forward FAILED,  m_spitchCalcsBuffer or  m_spitchCalcsSndSample  is null. "   );
			}
			else
			{ double  retOverbriteRatio   =  -2.0;

																						//  ***   These can vary quite a bit.  Highway49_SOLO.wav is a lounder one

		//   NOOOOOO,  		m_bitSource->m_inputScalePercent  =   400;    
		//	 do not change here.    Can change with  EventMan::Set_VolumeBoost_Factor()          	

						/*****
						if(          m_playSpeed  ==   1   )
							m_bitSource->m_inputScalePercent  =  400;      //   600[  typicall only  .5 percent worst overbrite 
						else if(   m_playSpeed  ==   2   )
							m_bitSource->m_inputScalePercent  =  500;     //    700[ little weak
						else if(   m_playSpeed  ==   3   )
							m_bitSource->m_inputScalePercent  =  700;  
						else if(   m_playSpeed  ==   4   )
							m_bitSource->m_inputScalePercent  =   900;  
						else
						{  ASSERT( 0 );
							m_bitSource->m_inputScalePercent  =   1200;  
						}
						****/

						//     m_topFreqFilterLimit      =    4700;       	m_bottomFreqFilterLimit =      82;          ORIGINALO  **** ADJUST*****
						//		m_topFreqFilterLimit      =   15600;       	m_bottomFreqFilterLimit =    600;  
						//		m_topFreqFilterLimit      =    9500;       	m_bottomFreqFilterLimit =      160;     //     No bad, maybe better
						//		m_topFreqFilterLimit      =    19500;       	m_bottomFreqFilterLimit =    1200;     // *** Wierd, but higher seems better??
						//		m_topFreqFilterLimit      =    19500;       	m_bottomFreqFilterLimit =    600;        // ** maybe  3rd  best ????  [ GreySolo at 2 slowDown ]
						//        m_topFreqFilterLimit      =    19500;       	m_bottomFreqFilterLimit =    150;    // ** maybe  2nd best ????  [ GreySolo at 2 slowDown ]
							//	m_topFreqFilterLimit      =    19500;       	m_bottomFreqFilterLimit =    300;     // **** BEST ???     [ GreySolo at 2 slowDown ]

		
				if(   ! sPitchCalcer->m_spitchCalcsSndSample->Apply_Full_Filtering_FixedScaling(   m_bitSource->m_topFreqFilterLimit,   m_bitSource->m_bottomFreqFilterLimit,   m_bitSource->m_inputScalePercent,   retOverbriteRatio,	  retErrorMesg   )    )
				{
							//  AfxMessageBox(  retErrorMesg  );   NO!!!  will virtually crash app.  Get this if silence in track.  OK it we just keep goint.  1/10
					if(    ! retErrorMesg.IsEmpty()    )   //  This is good for DEBUG, cause we can see a falure by the CString message without halting the play of the WAV  1/10
					{
						int   dummy =  8;
					}
				}
					//	  TRACE(   "Apply_Full_Filtering_FixedScaling [   OverbriteRatioInPercent =  %f   ]  \n",    retOverbriteRatio * 100.0   );     //   * 100.0 ...in percent
			        //	  TRACE(   "OverbriteRatioInPercent =  %f      [ m_inputScalePercent  %d ]  \n",     retOverbriteRatio * 100.0,   m_inputScalePercent   );     //   * 100.0 ...in percent



							 	//  a  LARGE  CIRCULAR-Que  for  OLD SndSamples  ....so I can read BACK in TIME for very slow speeds that dont fetch many samples    2/12

				long  bytesToProcessAlt =    m_bitSource->Get_Biggest_SndSample_Index(); 

				long  bytesToProcess     =     sPitchCalcer->Get_SndSamples_Valid_Count(  m_playSpeedFlt  );  // ***** CAREFUL, not fully tested.  1/29/12  ****** 

				if(   bytesToProcess  !=   bytesToProcessAlt    )
				{	int dummy =  9;  }


				sPitchCalcer->Add_SndSample_Samples_to_SndSampleCircularQue(  bytesToProcess  );

			 }    //   else      m_spitchCalcsBuffer and  m_spitchCalcsSndSample  are NOT null


		}   //   if(    sPitchCalcer->m_playMode  ==   0 


	}     //   else    m_bitSource->Fetch_Streaming_Samples_Direct( ) is sucessful

	


	if(    retHitEOF   )
	{
		//	ASSERT(  0   );     //  **********	Hit this win new MP3  source   2/4/10 .  Is this an issue									 Do not normally reach this.     1/2003

		m_doneFetchingPlayBytes =    true;  
	}
				


	ASSERT(   dwBytesLocked   ==    BLOCKLOCKEDSIZE   );    //  ** WANT an ERROR message ???? ***


	m_srcBytesProcessed  +=     dwBytesLocked;

	m_dataFetchCount++;   //  count how many times this funct was called during   
	m_curSubFetch  =   0;
}



											////////////////////////////////////////


void    StreamingAudioplayer::Load_Next_DataFetch_Forward_PPlayer_NoAudio(   unsigned long   iEvent    )
{										 


ASSERT( 0 );   //  ****************   NOT USED ???     3/2012  ***********************************




			//   This is for PRE-ROLL,  at music-start  populate the PIPELINE so it can play right away  {  Final CircQue for


							//  Loads the next  BLOCKof  WAVsample data,  and puts it in the SndSample for later NoteDetection
	                        //  Also loads the WAV-bytes to  BitSource's   WAV-Delay Buffer  for when user LATER hits the Play-Button (nosound delay with PreRoll  3/11  )

    static DWORD   dwLastPlayPos;
    UINT                  cbBytesRead;
	CString     retErrorMesg;



	ASSERT(   m_bitSource->m_wavVolumeSoundmanAddr   !=  NULL   );    // ***** USE this ???  3/11 ***************

//	ASSERT(   m_bitSource->m_spitchCalcsSndSample  !=  NULL    );

	ASSERT(   m_dataFetchCount  >=  0   );



	short  appCode  =  Get_PitchScope_App_Code_GLB();         //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope
	if(      appCode   !=  1   )
	{
		AfxMessageBox(   "Load_Next_DataFetch_Forward_PPlayer_NoAudio  FAILED,  it is only for Navigator."   );
		return;
	}



	if(   ! IsOK()    )
	{   
		ASSERT( false );	
		return; 	
	}


	SPitchCalc  *sPitchCalcer   =     Get_SPitchCalc();     
	ASSERT(      sPitchCalcer  );



	if(   m_doneFetchingPlayBytes   )    //  ..just before   EventMan::Process_Event_Notification()  [  with event =  Get_EventMan().m_numberOfNotificationEvents -1
	{				    	
		return;   // ******  NEW   3/1/11   Try to avoid the messy logic at the bottom   
	}



	DWORD     dwBytesLocked   =    BLOCKLOCKEDSIZE;      //  44160    ***********   OK ??? ********************
	m_bytesInFetch                   =    BLOCKLOCKEDSIZE;      //  44160    dwBytesLocked;    // Number of bytes we will read for this Chunk


	UINT     bytsInSect  =     TWOSECBYTES   /   (  Get_EventMan().m_numberOfNotificationEvents  -1 );     // How many bytes in an Event 




			//  what is the  ' VIRTUAL-FilePosition'   that  Lock()  intends for THIS dataFetch  ( is just multiples of 44,160

									//  'startOffsetBits'  is is OUTPUT coords, so is kind of a virtual value while imagining a 44,100 signal with 16bit stereo
	long     startOffsetBits  =     
					(long)(      	(  (double)m_dataFetchCount   *    (double)dwBytesLocked   )    /  m_playSpeedFlt   )      //   How many bytes have we PREVIOUSLY fetched in earlier function calls...
												+    ( m_startSample  *  m_bytesPerSample  );    //   ...plus the  ContinuedPlay-OFFSET


		//  ***  What about  the 44 BYTES in the File's Header  ???    May have to deal with it some day!!!   Too small to wory about ???   9/2003




	bool    retHitEOF =    false;  
	long    retScalePercent =  -1;


	BYTE  *delayWriteByte       =         m_bitSource->Get_DelayBuffer_Write_Byte(    (long)iEvent,    (long)m_dataFetchCount    );
	if(        delayWriteByte  ==   NULL  )
	{
	//	delayWriteByte =    (BYTE*)lpvData;   // PLAYER:   this would mean we are NOT using a buffer   [  lpvData  points to HARDWARE  circ-soundBuffer
		ASSERT( 0 );    //   This function is for Navigator,   NOT Player.  
	}


	if(    ! m_bitSource->Fetch_Streaming_Samples_Direct_PPlayer(       startOffsetBits,           //  how far to offset WITHIN Samplebits
																										dwBytesLocked,          //  number of bytes to read.

																										delayWriteByte,   //  destination MEMORY array,  for Streaming-play it is in  the MSoft  SoundBuffer																						    

																			  						  &cbBytesRead,              //  number of bytes ACTUALLY read.
																										m_srcBytesProcessed,													
																										retHitEOF,

																								       true,      //  Forward flag  

																										m_auditionCode,    // ????  Not really used.  1/10       is 0   Where was this set ????   0 =  AnimeStream::NORMAL

																										true,     //  soundManControlsVolume,  YES, we want Soundman's variable to control the sound
																										
																									     m_rightChannelPercent,

																										 m_playSpeedFlt,

																										retErrorMesg   )     )

	{  m_doneFetchingPlayBytes =    true;   

		//   m_infcDSoundBuffer->Unlock(    lpvData,    dwBytesLocked,    NULL,    0   );  //   ***** MOVED up here  for faster processing.  3/11   OK ????
	}		// ??? OK     ...to make sure we stop playing		
	
	else
	{
		if(    sPitchCalcer->m_playModeUsingNotelist  ==   0   )    //  0:   Detection
		{

			if(    sPitchCalcer->m_spitchCalcsSndSample  ==  NULL     )
			{
				ASSERT( 0 );   //  INSTALL error message 
				AfxMessageBox(   "StreamingAudioplayer::Load_Next_DataFetch_Forward_PPlayer_NoAudio FAILED,  m_spitchCalcsBuffer or  m_spitchCalcsSndSample  is null. "   );
			}
			else
			{ double  retOverbriteRatio   =  -2.0;
																					
		

				//  The   RESULTANT  SndSample  can have fewer VALID samples,  as Speed changes:
				//
				//     11040 [ speed 1 ]      7360[ 1.5 ]       5520[ 2 ]       3680[ 3 ]       2760[ 4 ]       1840[ 6 ]       1380[  speed 8 ]


																								 //   Can change  volume with  EventMan::Set_VolumeBoost_Factor()  ????   1/12
		
				if(   ! sPitchCalcer->m_spitchCalcsSndSample->Apply_Full_Filtering_FixedScaling(  m_bitSource->m_topFreqFilterLimit,  m_bitSource->m_bottomFreqFilterLimit,  m_bitSource->m_inputScalePercent,   retOverbriteRatio,	  retErrorMesg   )    )
				{
							//  AfxMessageBox(  retErrorMesg  );   NO!!!  will virtually crash app.  Get this if silence in track.  OK it we just keep goint.  1/10
					if(    ! retErrorMesg.IsEmpty()    )   //  This is good for DEBUG, cause we can see a falure by the CString message without halting the play of the WAV  1/10
					{  int   dummy =  8;   }					
				}



							 	//  a LARGE  CIRCULAR-Que  for  OLD SndSamples  ....so I can read BACK in TIME for very slow speeds that dont fetch many samples    2/12


				long  bytesToProcessAlt =    m_bitSource->Get_Biggest_SndSample_Index(); 

				long  bytesToProcess     =     sPitchCalcer->Get_SndSamples_Valid_Count(  m_playSpeedFlt  );  // ***** CAREFUL, not fully tested.  1/29/12  ****** 

				if(   bytesToProcess  !=   bytesToProcessAlt    )
				{	int dummy =  9;  }


				sPitchCalcer->Add_SndSample_Samples_to_SndSampleCircularQue(  bytesToProcess  );

			 }    //   else   m_spitchCalcsSndSample  is NOT null


		}   //  if(    sPitchCalcer->m_playModeUsingNotelist  ==   0  

	}     //   else    m_bitSource->Fetch_Streaming_Samples_Direct( ) is sucessful

	


	if(    retHitEOF   )
		m_doneFetchingPlayBytes =    true;  
	
				

	m_srcBytesProcessed  +=     dwBytesLocked;

	m_dataFetchCount++;   //  count how many times this funct was called during   

	m_curSubFetch  =   0;
}


											////////////////////////////////////////


void   StreamingAudioplayer::Load_Next_DataFetch_Backward_RegularSpeed(   unsigned long   iEvent    )
{										 

								//  Loads the next 'chunk' of sample data from memory(BitSourceAudioMS) and ALLOWS it to play
    HRESULT           hr;
    DWORD            dwStartOfs;
    static DWORD   dwLastPlayPos;
    VOID              *lpvData;
    DWORD            dwBytesLocked;
    UINT                 cbBytesRead; 
	CString             retErrorMesg;


//	m_buffersSrcByte =  NULL;   //  init for fail


	if(    !IsOK()     )
	{   
		ASSERT( false );	
		return; 	
	}
	ASSERT(   m_playSpeedFlt  ==  1.0   );


	if(    m_dataFetchCount  < 0    )  
	{
		ASSERT( 0 );
		m_dataFetchCount =  0;
	}



	if(   m_doRealtimePitchDetection  )
	{
		AfxMessageBox(  "StreamingAudioplayer::Load_Next_DataFetch_Backward_RegularSpeed FAILED,  m_spitchCalcsBuffer is NULL for Realtime Pitch Detect."  );
		return;
	}



	if(    m_doneFetchingPlayBytes    )
	{

	//	StopPlaying_Hardware();       //    now done in   EventMan::Process_Event_Notification()

	//	return;     ....let it keep going, WITHOUT Fetching new file data,  just updating vars.
	}




	DWORD           numBytesToPlay =     (DWORD)Get_Waves_ByteCount_toEnd();   // will shorten for play of PART of the sample that is SELECTED with RangeFrame



	unsigned long   hemisEvent        =    ( (unsigned long)( Get_EventMan().m_numberOfNotificationEvents -1 ) )  / 2;    // The event at 180 dergees
	




												// If the play cursor has just reached the first or second half of the
												// buffer, it's time to stream data to the other half.

	if(          iEvent ==    0   )
		dwStartOfs  =    m_dwMidBuffer;    //  want to write 180 degrees from this Event 'position'
	else if(   iEvent ==   hemisEvent    )                  
		dwStartOfs  =    0;  
	else              
		ASSERT( false );



    hr =   m_infcDSoundBuffer->Lock(   dwStartOfs,          // Offset of lock start.
														     m_dwMidBuffer,   // Number of bytes to lock.
														   &lpvData,               // Address of lock start. ( where we will bytes to )
														   &dwBytesLocked,   // Number of bytes locked.
															 NULL,             // Address of wraparound lock.
															 NULL,             // Number of wraparound bytes.
															 0 );                 // Flags.

	m_bytesInFetch =     dwBytesLocked;     // number of bytes we will read




								 //  what is the  VIRTUAL-FilePosition  that  Lock()  intends for THIS dataFetch ...


	long     startOffsetBits  =                      ( m_startSample   *   m_bytesPerSample  )      //   ...plus the  ContinuedPlay-OFFSET
						            //      -   (    (    m_dataFetchCount          *   dwBytesLocked   )    /  m_playSpeed    );        //   How many bytes have we PREVIOUSLY fetched in earlier function calls...
						                    -   (    (  ( m_dataFetchCount -1 )    *   dwBytesLocked   )    /  (long)m_playSpeedFlt    );        //   How many bytes have we PREVIOUSLY fetched in earlier function calls...



	  // ******   ...OR would   ( m_dataFetchCount -1 )   be more ACCURATE ????   10/02   **************************




	short  channelCode =    Get_StereoChannelCode();





	if(    m_playSpeedFlt   ==   1.0    )
	{

		 bool  retHitEOF =    false;  


		 if(   !m_doneFetchingPlayBytes    )
		 {
			 if(     !m_bitSource->Fetch_Streaming_Samples_Direct(       startOffsetBits,           //  how far to offset WITHIN Samplebits

																									dwBytesLocked,          //  number of bytes to read.

																									m_backwardsBits,	//		(BYTE*)lpvData,          //  destination  MEMORY Block,    for Streaming-play it is in  the MSoft  SoundBuffer

																			  					  &cbBytesRead,              //  number of bytes ACTUALLY read.
																									m_srcBytesProcessed,			

																									retHitEOF,
																									false,
																									m_auditionCode,  																							
																									true,     //  soundManControlsVolume,  YES, we want Soundman's variable to control the sound
																									retErrorMesg   )     )
			{  m_doneFetchingPlayBytes =    true;   }  
		 }


		if(    retHitEOF   )
			m_doneFetchingPlayBytes =    true;  

		ASSERT(   dwBytesLocked    ==   BLOCKLOCKEDSIZE    );




// *******************************************************************************************************
// ***************   BUG???   should I preserve the byte order for 2 sizteen bit samples ????   12/09   *******************


		if(    m_doRealtimePitchDetection    )
		{

			ASSERT( 0 );
			/****
			ASSERT(  m_spitchCalcsBuffer  );

			BYTE   *src  =                 m_backwardsBits    +    m_backwardsBitsSize;     //   FAILS:  +  (  m_backwardsBitsSize   - 1  );	  ???? WHY, should be end of buffer  ???	
			BYTE   *dst  =     (BYTE*)lpvData;
			BYTE   *dst2  =    (BYTE*)m_spitchCalcsBuffer;


			for(   long by =0;     by <  (long)dwBytesLocked;     by++   )
			{
				*dst   =    *src;
				*dst2 =    *src;

				dst++;     dst2++;
				src--;
			}



			long   bytesPerSample16  =    4;
			long    retScalePercent,   totalSamples         =    BLOCKLOCKEDSIZE  /  bytesPerSample16;

			ASSERT(   (long)dwBytesLocked  ==   BLOCKLOCKEDSIZE   );    //  can I always assume this ???    12/09
		

			if(    ! m_spitchCalcsSndSample->CopyBits_From_16bit_Sample(   Get_StereoChannelCode(), 	 (char*)(  &( m_spitchCalcsBuffer[0] )   ),    
																																				               0,   totalSamples,   retErrorMesg   )    )
				AfxMessageBox(  retErrorMesg  );


			if(     ! m_spitchCalcsSndSample->Apply_Full_Filtering(   m_topFreqFilterLimit,   m_bottomFreqFilterLimit,   retScalePercent,    retErrorMesg   )   )
				AfxMessageBox(  retErrorMesg  );
			****/
		}
		else
		{  BYTE   *src  =                 m_backwardsBits    +    m_backwardsBitsSize;     //   FAILS:  +  (  m_backwardsBitsSize   - 1  );	  ???? WHY, shold be ebnd of buffer  ???	
			BYTE   *dst  =     (BYTE*)lpvData;

			for(   long by =0;     by <  (long)dwBytesLocked;     by++   )
			{
				*dst =    *src;

				dst++;
				src--;
			}
		}



		m_srcBytesProcessed   +=    dwBytesLocked;

		m_dataFetchCount++;						 //  count how many times this funct was called during   
		m_curSubFetch =  0;

	}     //  m_playSpeed  ==   1 





	m_infcDSoundBuffer->Unlock(    lpvData,    dwBytesLocked,    NULL,    0   );


	if(	   m_srcBytesProcessed   >=   numBytesToPlay    )	 // could be BOF,   or end of a Section's bytes( old SectionPlay )   1/03
	{
		m_doneFetchingPlayBytes =   true;				//    1)    ...first step in  'Stopping Play' 
	}     
}






										////////////////////////////////////////////////

/*********  NOW a static function in SndSample   12/11

void    StreamingAudioplayer::Reverse_16bit_Samples_On_Copy(    BYTE *src,     BYTE  *dst,     long  numberOfBytes   )
{


// *************   INSTALL   3/11   this logic for  Apply_Volume_Adjust_PPlayer()   ??? so that the sound can be modified as the bits are copied in reverse *************


	if(      src ==  NULL    ||    dst ==  NULL  
		||   numberOfBytes <= 0  )
	{
		ASSERT( 0 );
		return;
	}

	long   numberSamples           =    numberOfBytes /4;

//	long   halfNumberOfSamples  =   numberSamples  /2;    WRONG,  see below.  


	BYTE  *travSRC =    src  +   ( numberOfBytes - 4 );

	BYTE  *travDST =    dst;


														//  The logic in this is very similar to   SndSample::Reverse_Byte_Order()    3/11


//	for(   long  samp =0;     samp <  halfNumberOfSamples;     samp++   )    //   **** NO, need to do ALL the samples ( 'numberSamples' ) because we are
	for(   long  samp =0;     samp <  numberSamples;     samp++   )             //  copying from one MemoryBlock to a DIFFERENT 2nd MemoryBlock ( we are NOT
	{                                                                                                         //   merely reordering the bytes within a SINGLE Memory Block   11/11

		BYTE  holdCh0  =    *(   travSRC          );
		BYTE  holdCh1  =    *(   travSRC  + 1   );
		BYTE  holdCh2  =    *(   travSRC  + 2   );
		BYTE  holdCh3  =    *(   travSRC  + 3   );


		*(   travSRC          )  =		*(   travDST          );       //   *src =   *dst;
		*(   travSRC  + 1   )  =		*(   travDST  + 1   );
		*(   travSRC  + 2   )  =		*(   travDST  + 2   );
		*(   travSRC  + 3   )  =		*(   travDST  + 3   );


		*(   travDST          )  =		holdCh0;                         //   *dst =    holdValue;
		*(   travDST  + 1   )  =		holdCh1;
		*(   travDST  + 2   )  =		holdCh2;
		*(   travDST  + 3   )  =		holdCh3;


		travSRC  -=    4;

		travDST  +=   4;
	}

}

*******/


										////////////////////////////////////////////////


long   StreamingAudioplayer::Get_SndSamples_Valid_Count(  long  slowSpeed    )
{

			//   As we do our  Slow-Speeds,  a typical data load will have LESS sample because the 8bit samples in the SndSample
			//   for note analysis  are in  NormalSpeed,  and are thus fewer than the EXPANDED Count of  SlowSamples for the WAV play  3/11

	long   retSampleCount  =  -1;

	
	long   samplesInDataFetch  =      BLOCKLOCKEDSIZE  /  m_bitSource->m_bytesPerSample;   //    11,040    *** Need to adjust for speed ????? 


	retSampleCount  =    samplesInDataFetch  /  slowSpeed;


	return   retSampleCount;
}





										////////////////////////////////////////////////


void   StreamingAudioplayer::Load_Next_DataFetch_Backward_SlowSpeed(   unsigned long   iEvent    )
{										 

								//  Loads the next 'chunk' of sample data from memory(BitSourceAudioMS) and ALLOWS it to play
    HRESULT           hr;
    DWORD            dwStartOfs;
    static DWORD   dwLastPlayPos;
    VOID              *lpvData;
    DWORD            dwBytesLocked;
    UINT                 cbBytesRead; 
	CString             retErrorMesg;


//	m_buffersSrcByte =  NULL;   //  init for fail


	if(    !IsOK()     )
	{   
		ASSERT( false );	
		return; 	
	}
	ASSERT(   m_playSpeedFlt  >=  1.0   );


	if(    m_dataFetchCount  < 0    )  
	{
		ASSERT( 0 );
		m_dataFetchCount =  0;
	}



	if(    m_doneFetchingPlayBytes    )
	{
	//	StopPlaying_Hardware();       //    now done in   EventMan::Process_Event_Notification()
	//	return;     ....let it keep going, WITHOUT Fetching new file data,  just updating vars.
	}


	if(   m_doRealtimePitchDetection  )
	{
		AfxMessageBox(  "StreamingAudioplayer::Load_Next_DataFetch_Backward_SlowSpeed FAILED,  m_spitchCalcsBuffer is NULL for Realtime Pitch Detect."  );
		return;
	}





	DWORD           numBytesToPlay =     (DWORD)Get_Waves_ByteCount_toEnd();   // will shorten for play of PART of the sample that is SELECTED with RangeFrame



	unsigned long   hemisEvent        =    ( (unsigned long)( Get_EventMan().m_numberOfNotificationEvents -1 ) )  / 2;    // The event at 180 dergees
	




												// If the play cursor has just reached the first or second half of the
												// buffer, it's time to stream data to the other half.

	if(          iEvent ==    0   )
		dwStartOfs  =    m_dwMidBuffer;    //  want to write 180 degrees from this Event 'position'
	else if(   iEvent ==   hemisEvent    )                  
		dwStartOfs  =    0;  
	else              
		ASSERT( false );



    hr =   m_infcDSoundBuffer->Lock(   dwStartOfs,          // Offset of lock start.
														     m_dwMidBuffer,   // Number of bytes to lock.
														   &lpvData,               // Address of lock start. ( where we will bytes to )
														   &dwBytesLocked,   // Number of bytes locked.
															 NULL,             // Address of wraparound lock.
															 NULL,             // Number of wraparound bytes.
															 0 );                 // Flags.

	m_bytesInFetch =     dwBytesLocked;     // number of bytes we will read




								 //  what is the  VIRTUAL-FilePosition  that  Lock()  intends for THIS dataFetch ...

	long     startOffsetBits  =                      ( m_startSample   *   m_bytesPerSample  )      //   ...plus the  ContinuedPlay-OFFSET
						            //      -   (    (    m_dataFetchCount          *   dwBytesLocked   )    /  m_playSpeed    );        //   How many bytes have we PREVIOUSLY fetched in earlier function calls...
						                    -   (    (  ( m_dataFetchCount -1 )    *   dwBytesLocked   )    /  (long)m_playSpeedFlt    );        //   How many bytes have we PREVIOUSLY fetched in earlier function calls...

	  // ******   ...OR would   ( m_dataFetchCount -1 )   be more ACCURATE ????   10/02   **************************




	short  channelCode =    Get_StereoChannelCode();



	if(    m_playSpeedFlt   >   1.0    ) 
	{		
		//  playing at  SLOWER speeds      ***CAREFUL:  To always work on 32bit  BOUNDARIES!!!  ( 2x 16bit samples = 32 )

		long     repeatSize =   m_slowPlayStep;   	//  4096L    tried:  256   1024    2048    4096   	[ BEST:  4096   2048  ]	  [  bad:  8192  ]
		long     zoneReadCount = 0,    bytIdx = 0;		
		BYTE  *srcData =NULL,    *dstData,    *zoneSrcStart =NULL;
		bool     retHitEOF =   false;



		long     numSrcZonesNeeded  =      dwBytesLocked    /    (  repeatSize  *  (long)m_playSpeedFlt  );

		if(    (     dwBytesLocked    %    (  repeatSize  *  (long)m_playSpeedFlt  )   )     !=   0   )
			numSrcZonesNeeded++;

		numSrcZonesNeeded++;    //  another for good measure   ....OTHERWISE get a Clicking sound 


		long   bytesNeeded  =      numSrcZonesNeeded    *    repeatSize;


/***   moved down,  OK ???
		zoneSrcStart  =    (  m_backwardsBits    +    bytesNeeded  )    -  repeatSize;     //  try backing up to the first  repeatZone 

		srcData  =                zoneSrcStart;  
		dstData  =     (BYTE*)lpvData;
****/


																//  Fill the backwards buffer for the ENTIRE  functions duration

		 if(   ! m_doneFetchingPlayBytes    )
		 {
			if(     ! m_bitSource->Fetch_Streaming_Samples_Direct(        startOffsetBits,    //   startOffsetZoneBits,
																											bytesNeeded,   //   dwBytesLocked,       //  Number of bytes to read.
																											m_backwardsBits,                  //  destination  MEMORY Block,    for Streaming-play it is in  the MSoft  SoundBuffer
																			  							  &cbBytesRead,          //  Number of bytes ACTUALLY read.
																											m_srcBytesProcessed,													
																										//	absCurOffsetFromFileStart,    	
																											retHitEOF,
																											false,
																											m_auditionCode,  
																									     //   channelCode,       
																											true,     //  soundManControlsVolume,  YES, we want Soundman's variable to control the sound
																											retErrorMesg   )     )
			{  m_doneFetchingPlayBytes =    true;   
			}     
		 }


		if(    retHitEOF   )
			m_doneFetchingPlayBytes =    true;  


		ASSERT(   dwBytesLocked   ==    BLOCKLOCKEDSIZE   );    //  ** WANT an ERROR message ???? ***




		BYTE   *dst2  =   NULL;

		zoneSrcStart  =    (  m_backwardsBits    +    bytesNeeded  )    -  repeatSize;     //  try backing up to the first  repeatZone 

		srcData  =                zoneSrcStart;  
		dstData  =     (BYTE*)lpvData;



		if(    m_doRealtimePitchDetection    )
		{
			ASSERT( 0 );
			/****
			dst2  =    (BYTE*)m_spitchCalcsBuffer;
			****/
		}
		






		for(    long i =0;     i <  (long)dwBytesLocked;     i++    )	 //  Read the bytes from  'm_backwardsBits'  memory to the hardware
		{

			if(    m_srcBytesProcessed   >=     numBytesToPlay   )     //  get a 'memory access' error if we try to read too far
				break;       
			else      
			{			//  Not hit the end of the sample...     if(     m_srcBytesProcessed   <    (m_totalSamples * m_playSpeed)     )  



				if(    bytIdx   >=   repeatSize   )      //  just hit 4096,  so crossing into a new zone
				{

					zoneReadCount++;        

					if(    zoneReadCount  >=   (long)m_playSpeedFlt    )   //  time to advance to next  zoneSrcStart ?
					{							
						zoneSrcStart  -=    repeatSize;          //  ( note SUBTRACTION )  advance to the next  source block
						zoneReadCount = 0;		 // reset the cycle
					}
						
					srcData =    zoneSrcStart;    //  go to the zoneSrcStart
					bytIdx   =    0;                     //  reset for the next cycle
				}


				
				*dstData =    *srcData;

				if(     m_doRealtimePitchDetection    )
				{
					*dst2  =    *srcData;    //  also copy to  m_spitchCalcsBuffer
					 dst2++;
				}


				srcData--;       //   ( note SUBTRACTION )
				dstData++;     //  destination just keeps advancing without issue


				bytIdx++;

				if(    zoneReadCount  ==  0   )    //  only count while in FIRST zone, otherwise occasionaly get an 'ACCESS exception'
					m_srcBytesProcessed++;
			}  


		}  //  for(  i =0

			


		if(     (  m_dataFetchCount  %  ((long)m_playSpeedFlt)     )  ==  0    )  //  since m_curChunkPlayer=-1 at start,  m_curChunkPlayer goes to 0 on the first funct call 
			m_curSubFetch =  0;   //  init
		else
			m_curSubFetch++;    // Though we do NOT advance m_curChunkPlayer,  this tells us the implied advance by this fetch( for Calc_Current_Sample_Forward() 
			

		m_dataFetchCount++;     //  count how many times this funct was called during 
	}   
		




	m_infcDSoundBuffer->Unlock(    lpvData,    dwBytesLocked,    NULL,    0   );




	if(   m_doRealtimePitchDetection    )
	{

		/****
		long   bytesPerSample16  =    4;
		long    retScalePercent,   totalSamples         =    BLOCKLOCKEDSIZE  /  bytesPerSample16;

		ASSERT(   (long)dwBytesLocked  ==   BLOCKLOCKEDSIZE   );    //  can I always assume this ???    12/09
		

		if(    ! m_spitchCalcsSndSample->CopyBits_From_16bit_Sample(   Get_StereoChannelCode(), 	 (char*)(  &( m_spitchCalcsBuffer[0] )   ),    
																																				               0,   totalSamples,   retErrorMesg   )    )
			AfxMessageBox(  retErrorMesg  );

		if(     ! m_spitchCalcsSndSample->Apply_Full_Filtering(   m_topFreqFilterLimit,   m_bottomFreqFilterLimit,   retScalePercent,    retErrorMesg   )   )
			AfxMessageBox(  retErrorMesg  );
		****/
	}




	if(	   m_srcBytesProcessed   >=   numBytesToPlay    )	 // could be BOF,   or end of a Section's bytes( old SectionPlay )   1/03
	{
		m_doneFetchingPlayBytes =   true;				//    1)    ...first step in  'Stopping Play' 
	}     
}




											////////////////////////////////////////


void    StreamingAudioplayer::Load_Next_DataFetch_Forward_SlowSpeed(    unsigned long   iEvent    )
{										 

											//  Loads the next  'chunk' of sample data  from file and plays
    HRESULT          hr;
    DWORD            dwStartOfs;
    static DWORD   dwLastPlayPos;
    VOID               *lpvData;
    DWORD             dwBytesLocked;
    UINT                  cbBytesRead;
	CString     retErrorMesg;


	bool     readStaticBitsDEBUG    =      false;     // ******  ALWAYS off, now   5/07 DEBUG switch ******

//	m_buffersSrcByte =  NULL;



	if(    !IsOK()     )
	{   
		ASSERT( false );	
		return; 	
	}

	ASSERT(   m_playSpeedFlt >  1.0   );					// ****  INSTALL  Error-MESSAGING ******

	ASSERT(   m_dataFetchCount  >=  0   );


	if(   m_doRealtimePitchDetection   )
	{
		AfxMessageBox(  "StreamingAudioplayer::Load_Next_DataFetch_Forward_SlowSpeed FAILED,  m_spitchCalcsBuffer is NULL for Realtime Pitch Detect."  );
		return;
	}



	if(    m_doneFetchingPlayBytes     )     
	{				 //  2)    ...just before   EventMan::Process_Event_Notification()  [  with event =  Get_EventMan().m_numberOfNotificationEvents -1
//		StopPlaying_Hardware();       //      ....Better to set flags later
//		return;   //  Let it keep going,  but nottry to load new data,  just rehash the existing.
	}



	short  channelCode =    Get_StereoChannelCode();


//	DWORD   numBytesToPlay   =     ( DWORD )Get_Waves_ByteCount_toEnd();     //  OLD:   will shorten for play of PART of the sample that is SELECTED with RangeFrame


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
														  &lpvData,               // Address of lock start. ( DEST-memory where we will bytes to )
														  &dwBytesLocked,   // Number of bytes locked.
															NULL,                   // Address of wraparound lock.
															NULL,                   // Number of wraparound bytes.
															0         );                     // Flags.



	if(       (long)dwBytesLocked       !=    m_bytesInFetch     // **** ALWAYS Constant at  44160 ***** ???? 
		&&          m_dataFetchCount   !=    0        )  
	{
		ASSERT( 0 );       //  put this in at 12/7/09
		TRACE(  "\n***ERROR***   dwBytesLocked  CHANGED[   %d   ->   %d   ]    m_dataFetchCount[ %d ] ***\n\n",    
										m_bytesInFetch,  dwBytesLocked,   m_dataFetchCount    );
	}



	m_bytesInFetch     =    dwBytesLocked;    // Number of bytes we will read for this Chunk

	UINT     bytsInSect  =     TWOSECBYTES   /   (  Get_EventMan().m_numberOfNotificationEvents  -1 );     // How many bytes in an Event 





	ASSERT(   m_slowPlayStep   ==   SPEEDZONESIZE  );    // ******* TROUBLE ********

	long   repeatSize  =   m_slowPlayStep;	//  4096L    tried:  256   1024    2048    4096   	[ BEST:  4096   2048  ]	  [  bad:  8192  ]





								 //  what is the  ' VIRTUAL-FilePosition'   that  Lock()  intends for THIS dataFetch

	long     startOffsetBits  =     
					(long)(	    (  (double)m_dataFetchCount   *   (double)dwBytesLocked   )    /  m_playSpeedFlt      )      //   How many bytes have we PREVIOUSLY fetched in earlier function calls...
												+    ( m_startSample  *  m_bytesPerSample  );    //   ...plus the  ContinuedPlay-OFFSET

				//  ***  Whast about  the 44 BYTES in the File's Header  ???    May have to deal with it some day!!!   Too small to wory about ???   9/2003



	BYTE   *dstData  =  NULL;


	if(   m_doRealtimePitchDetection  )
	{
		ASSERT( 0 );
		/****
		ASSERT(  m_spitchCalcsBuffer  );
		dstData  =    ( BYTE* )m_spitchCalcsBuffer;
		*****/
	}
	else
		dstData  =    ( BYTE* )lpvData;   //  default, old way





	if(    m_playSpeedFlt  >  1.0    )			 //   playing at   SLOWER speeds    [  Streaming   ]
	{

		BYTE   *srcData =NULL,    *zoneSrcStart =NULL;
		long   startOffsetZoneBits;
		bool   retHitEOF =   false;

		long   zoneReadCount = 0,    bytIdx = 0;		
		long   zoneReadCountForFunction =  0;    //  does NOT reset



		if(    m_srcBytesProcessed   ==  0    )    //   1st CALL   for THIS playSession,   so  INITIALIZE/load  ZoneBuffer....
		{


			bool  retHitEOF =    false;     

			startOffsetZoneBits =       startOffsetBits    +     (  zoneReadCountForFunction   *   repeatSize  );   //  must advance for


			if(    ! m_doneFetchingPlayBytes    )    
			{
				if(     ! m_bitSource->Fetch_Streaming_Samples_Direct(       startOffsetZoneBits,

																										repeatSize,       //  Number of bytes to read.

																										m_speedZoneBuffer,    //  destination  MEMORY Block,    for Streaming-play it is in  the MSoft  SoundBuffer

																			  						  &cbBytesRead,          //  Number of bytes ACTUALLY read.
																										m_srcBytesProcessed,													
																										retHitEOF,
																										true,
																										m_auditionCode,  																					
																										true,     //  soundManControlsVolume,  YES, we want Soundman's variable to control the sound
																										retErrorMesg   )     )
				{  m_doneFetchingPlayBytes =    true;   
				}     
			}


			if(    retHitEOF   )
				m_doneFetchingPlayBytes =    true;  

			ASSERT(   dwBytesLocked   ==    BLOCKLOCKEDSIZE   );    //  ** WANT an ERROR message ???? ***


			zoneReadCountForFunction++;
		}





		zoneSrcStart  =     m_speedZoneBuffer;
		srcData         =     m_speedZoneBuffer; 


		for(   long  i =0;      i <  (long)dwBytesLocked;      i++    )	      //  Read the  bytes( 44160 bytes)  from  m_speedZoneBuffer to the  hardware-memory
		{


			if(     bytIdx   >=    repeatSize     )      //  just hit  4096,  so crossing into a  new/repeated  Zone
			{

				zoneReadCount++;        


				if(    zoneReadCount   >=   (long)m_playSpeedFlt    )    //  ******BAD,  time to advance to next  Zone...  ( that is refresh the ZoneBuffer  )
				{							
							

					bool  retHitEOF =    false;    

					startOffsetZoneBits =     startOffsetBits    +    (  zoneReadCountForFunction   *  repeatSize  );   //  ***FIX ****   must advance for
								

					if(    ! m_doneFetchingPlayBytes    )   
					{
						if(     ! m_bitSource->Fetch_Streaming_Samples_Direct(    startOffsetZoneBits,   
																												   repeatSize,             //  Number of bytes to read.

																												   m_speedZoneBuffer,   //  destination  MEMORY Block,    for Streaming-play it is in  the MSoft  SoundBuffer

																			  									 &cbBytesRead,          //  Number of bytes ACTUALLY read.
																													m_srcBytesProcessed,													
																													retHitEOF,
																													true,
																													m_auditionCode,  
																												//	channelCode,   //   Is_SourceChannel_Left(),
																													true,     //  soundManControlsVolume,  YES, we want Soundman's variable to control the sound
																													retErrorMesg   )     )
						{  m_doneFetchingPlayBytes =    true;   
						}  
					}




					if(    retHitEOF   )
						m_doneFetchingPlayBytes =    true;  

					ASSERT(   dwBytesLocked   ==    BLOCKLOCKEDSIZE   );    //  ** WANT an ERROR message ???? ***



					zoneReadCount =   0;		 //  ***FIX***  reset the cycle

					zoneReadCountForFunction++;
				}  //  if(    zoneReadCount   >=   m_playSpeed    ) 
						
			


				srcData  =    zoneSrcStart;      //   ***FIX***   RESET  to the start of the ZoneBuffer  (  m_speedZoneBuffer  )

				bytIdx    =    0;                      //   reset for the next cycle
			}




			*dstData  =    *srcData;

			srcData++;
			dstData++;     //  destination Pointer just keeps advancing without issue

			bytIdx++;

			if(    zoneReadCount  ==  0   )   // only count while in FIRST zone, otherwise  occasionaly get an 'ACCESS exception'
				m_srcBytesProcessed++;

		}   //   for(   long i =0;     ....Read the bytes from  m_speedZoneBuffer to the  hardware-memory






		if(    ( m_dataFetchCount   %   (long)m_playSpeedFlt )   ==  0    )  //   ****FIX****    since m_curChunkPlayer=-1 at start,  m_curChunkPlayer goes to 0 on the first funct call 
			m_curSubFetch  =   0;   //  init
		else
			m_curSubFetch++;    // Though we do NOT advance m_curChunkPlayer,  this tells us the implied advance by this fetch( for Calc_Current_Sample_Forward() 
			


		m_dataFetchCount++;     //  count how many times this funct was called during   ***** should stay the same
	}    //  if(  m_playSpeed  >  1  )    
        




	if(    m_doRealtimePitchDetection    )   //  now finally copy it out to the hardware
	{

		ASSERT( 0 );
		/****
		ASSERT(  m_spitchCalcsBuffer  );

		char  *src =    (char*)m_spitchCalcsBuffer;  //   replace with memcopy
		char  *dst =    (char*)lpvData;

		for(   long  i =0;      i <  (long)dwBytesLocked;      i++    )	      //  Read the bytes from BitSourceAudioMS memory to the hardware
		{
			*dst  =   *src;
			src++;     dst++;
		 }


		double  retOverbriteRatio;
		long      bytesPerSample16  =    4;
		long      totalSamples         =    BLOCKLOCKEDSIZE  /  bytesPerSample16;
		

		if(    ! m_spitchCalcsSndSample->CopyBits_From_16bit_Sample(   Get_StereoChannelCode(), 	 (char*)(  &( m_spitchCalcsBuffer[0] )   ),    
																																				               0,   totalSamples,   retErrorMesg   )    )
			AfxMessageBox(  retErrorMesg  );



	//	if(     ! m_spitchCalcsSndSample->Apply_Full_Filtering(   m_topFreqFilterLimit,   m_bottomFreqFilterLimit,   retScalePercent,    retErrorMesg   )   )
	//		AfxMessageBox(  retErrorMesg  );

		if(   ! m_spitchCalcsSndSample->Apply_Full_Filtering_FixedScaling(   m_topFreqFilterLimit,   m_bottomFreqFilterLimit,   m_bitSource->m_inputScalePercent,   retOverbriteRatio,	  retErrorMesg   )    )
		{
			//  AfxMessageBox(  retErrorMesg  );   NO!!!  will virtually crash app.  Get this if silence in track.  OK it we just keep goint.  1/10
			if(    ! retErrorMesg.IsEmpty()    )   //  This is good for DEBUG, cause we can see a falure by the CString message without halting the play of the WAV  1/10
			{
				int   dummy =  8;
			}
		}
		*****/

	}



	m_infcDSoundBuffer->Unlock(    lpvData,    dwBytesLocked,    NULL,    0   );
}




					////////////////////////////////////////////////////////


BitSourceStreamingMS*     StreamingAudioplayer::Get_BitSourceStreamingMS(   CString&  retErrorMesg   )
{

	ASSERT( 0 );   // ****************  Is this used ???   3/12  ******************************


	if(   m_bitSource  ==  NULL  )
	{  
		retErrorMesg =   "StreamingAudioplayer::Get_BitSourceStreamingMS  FAILED,  m_bitSource is NULL." ;
		return  NULL;
	}


	BitSourceStreamingMS   *bitSourceStreamingMS      =        dynamic_cast< BitSourceStreamingMS* >(  m_bitSource  );  
	if(                                  bitSourceStreamingMS  ==  NULL   )
	{  
		retErrorMesg =   "StreamingAudioplayer::Get_BitSourceStreamingMS  FAILED,  could not dcast to BitSourceStreamingMS." ;
		return  NULL;
	}
	else
		return   bitSourceStreamingMS;
}

















