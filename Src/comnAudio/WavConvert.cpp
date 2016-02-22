/////////////////////////////////////////////////////////////////////////////
//
//  WavConvert.cpp   -  for the conversion of Sound Files like .WAV and .MP3 
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


#include <math.h>


#include  <mmsystem.h>   // Is good on Sparky  ( ProgramFiles\Microsoft Visual Studio\VC\PlatformSDK\Include )
  


#include  "..\comnFacade\VoxAppsGlobals.h"



#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"      //  #include <FixMath.h>    ...Mac's version of Fixedpoint 



#include "..\comnInterface\DetectorAbstracts.h"    // **** NEW , gives us an ABSTRACY definition of  BitSource, SourceAdmin,  etc




#include  "WaveJP.h"


#include   "..\comnMisc\FileUni.h"  



#include  "sndSample.h"


#include   "Mp3Decoder.h"       

#include   "ReSampler.h"

#include   "FFTslowDown.h"


#include  "BitSourceAudio.h"




#include "WavConvert.h"

/////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
/////////////////////////////////////////////////////////////////////


#define M_PI  3.14159265358979323846

//  #define MAX_FRAME_LENGTH  8192

/////////////////////////////////////////////////////////////////////




void        Begin_ProgressBar_Position_GLB(  char  *text   );   //   in  SPitchListWindow.cpp  
void        Set_ProgressBar_Position_GLB(   long  posInPercent   );
void        End_ProgressBar_Position_GLB();



////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


WavConvert::WavConvert()   
{

			// **** Is   CALLED by     PitchPlayerApp::Initialize_BitSource()   and   PitchPlayerApp::Make_NoteList_No_Audio()

	 CString   retErrorMesg;  


	m_srcSoundFilesPath.Empty();
	m_srcMP3FilePath.Empty();
	m_srcWavFilePath.Empty();


	/***

    m_playspeedSlowDownRatio =   1;      //    1:  is Normal speed



	m_slowDownAlgo =    0;     // default 0        0: PitchShift with Resampling       1:  NEW  HORIZONTAL  FFT



	m_fftFrameSizeSlowDown        =   4096;       //   2048     ...almost always        1024[ OK ]       512[ worse ]     128[ ???   ]   

	m_overSampleRatioSlowDown  =       8;        //    ...almost always        { 4 -  32 }      8[ good ]      4[ not bad]     2[bad]  

//	 long  fftFrameSize  =    2048;       //       2048[ Best? ]     1024[ OK ]       512[ worse ]     128[ not bad  ]      4096 [  ]    8192 [ good for voice removal ]
//
//	 long  osamp           =       8;        //       { 4  -   32 }      8[ good ]      4[ not bad]            2[bad]  
//
//	stepSize  =     fftFrameSize  /  osamp;       //   This is how many sample will actually get processed by a FFT-transform, considering how the
//																  //   "Overlap and Add"   recudes that ammount of  samples process per FFT-transform


	***/

	m_defaultSamplingFreqOutput     =    DEFAULTsAMPLINGrATEmp3;        //   44100 

	m_defaultBytesPerSampleOutput =   4;     //   always 4   ( 16bit stereo )



	m_bytesPerSampleSRCstream  =   4;
	m_stereoFlagSRCstream  =    true;

	m_bitsPerSampleSRCstream =  -1;

//	m_totalSamplesSRC =   -1;

	m_totalSourceSamples =   -1;   // NEW,  8/12



	m_srcSampRate =   m_dstSampRate  =   -1;      //  for automatic resampling


	m_totalSamplesBytesWAVsrc =  -1;

	m_formatTypeWAVsrc      =    999;            //   format type 
	m_channelsCountWAVsrc =   -1;       //   number of channels (i.e. mono, stereo...) 
	m_bitsPerSampleWAVsrc  =   -1;        //   number of bits per sample of mono data 
	m_sampleRateWAVsrc      =   -1;       //   sample rate 



	m_doResamplingStreaming =  false;
	m_isInitializedStreaming     =   false;



	m_outputBufferSizeStreaming =  0; 
	m_outputBufferSampleCount   =  0;



	m_outputBufferStreaming  =    NULL; 


	m_outputBufferStreamingNoSlowDown  =    NULL; 


	m_outputBufferSampleCountNotSlowDown =  -1;





	m_playModeUsingNotelist =  0;    //  NEW,  make sure it is UPDATED  [    BitSourceStreaming::Set_MidiSource_UsingNotelist_Variables()     ]   8/2012



/****
													//   For  PLAYER.exe  small   WAV-DELAY        7/2012


	m_baseDelay     =    1;    //   Sparky[ 0 ]      Fido[ 2 ]     Lassie[ 0- ]       ....But more computers are like Sparky,  so use value [ 1 ]
											                                 
	m_overideDelay =   -1;    //  Special MODE:   If this has a value  >=0,  then that is the REAL DELAY to use,  and  m_baseDelay is processed as ZERO   
***/
	m_baseDelayAddr      =   NULL;
	m_overideDelayAddr  =   NULL;


	m_delayInPieSlices =    -1;     //  **** NEW   

	m_outputBufferStreamingDelayed       =   NULL;
	m_outputBufferSizeStreamingDelayed =        0;

	m_indexIntoBufferStreamingDelayed =    0;






	m_currentByteIndexInBufferStreaming =  0;

	m_currentByteIndexInBufferStreamingNotSlowDown =  0;

	m_indexIntoSndSampleNotSlow =  0;



	m_reSampleArrayLeft =   m_reSampleArrayRight =   NULL;    //     the two are for RE-Sampling              2 intermediate samples


	m_wavFormat =  NULL;
	m_mmIO        =   NULL;  




	/**** alloc later when know more about the size

	m_backwardsPlayBuffer    =     new    BYTE[   BLOCKLOCKEDSIZE   ];
	if(   m_backwardsPlayBuffer  ==  NULL   )
	{
		 m_backwardsPlayBufferSize =  0;
		AfxMessageBox(  "WavConvert::WavConvert FAILED, could not alloc m_backwardsPlayBuffer."  );
	}
	else
		 m_backwardsPlayBufferSize =    BLOCKLOCKEDSIZE;  
	 ****/

	m_backwardsPlayBuffer       =  NULL;
	m_backwardsPlayBufferSize =   0;



	m_backwardsPlayBufferBlockIndex =  0;

	m_maxNumberOfbackwardsPlayBufferBlocks =  0;    //  10 or less 

	m_numberOfTenBlocksLoaded =   0;


	m_biggestIndexToSndSample =  -1;
}



					/////////////////////////////////////////////


WavConvert::~WavConvert()
{
													//  looks like this only gets called at the END of the program 
	Cleanup_After_Streaming();

	Release_WaveFormatEx();

	Release_ReSampling_Arrays();


//	DeAllocate_SlowDown_Buffers();


	if(    m_backwardsPlayBuffer  !=   NULL   )
	{
		delete   m_backwardsPlayBuffer;
		m_backwardsPlayBuffer =  NULL;

		m_backwardsPlayBufferSize =  0;
	}
}



					/////////////////////////////////////////////


void	   WavConvert::Cleanup_After_Streaming()
{


//	m_mp3DecoderStreaming.close_binfile();  
//	m_fileUniStreaming.close();   
	Release_Media_File();


	Release_WaveFormatEx();   //   ****  WANT this too???   will it hurt anything ???
	
  


	if(   m_outputBufferStreaming  !=  NULL  )      //  Need to do this because  Initialize_for_Streaming() will always alloc a new buffer when the song is loaded.
	{
		delete  m_outputBufferStreaming;
		m_outputBufferStreaming =   NULL;

		m_outputBufferSizeStreaming =  0;   //  want this ????
		m_outputBufferSampleCount   =  0;

		m_currentByteIndexInBufferStreaming =  0;
	}




	if(   m_outputBufferStreamingDelayed  !=  NULL  )      // 
	{
		delete  m_outputBufferStreamingDelayed;
		m_outputBufferStreamingDelayed =   NULL;

//		m_outputBufferSizeStreaming =  0;   //  want this ????
//		m_outputBufferSampleCount   =  0;

//		m_currentByteIndexInBufferStreaming =  0;
	}






	if(   m_outputBufferStreamingNoSlowDown  !=  NULL  )      //  Need to do this because  Initialize_for_Streaming() will always alloc a new buffer when the song is loaded.
	{
		delete  m_outputBufferStreamingNoSlowDown;
		m_outputBufferStreamingNoSlowDown =   NULL;

//		m_outputBufferSizeStreaming =  0;   //  want this ????
//		m_outputBufferSampleCount   =  0;

//		m_currentByteIndexInBufferStreaming =  0;      ****  INSTALL ?????

		m_currentByteIndexInBufferStreamingNotSlowDown =  0;
	}





	Release_ReSampling_Arrays();


	m_fftSlowDown.DeAllocate_SlowDown_Buffers();


	m_isInitializedStreaming =   false;
}



					/////////////////////////////////////////////


void	  WavConvert::Initialize_for_Play()
{

			//  NEW,   1/2012     allow some MORE future initialization



			//  Think ERRORS can build up in THERE,  seems like a good idea to FLUSH it occasionally.    1/12     *** ONLY really for NAVIGATOR ***

	m_fftSlowDown.Erase_OutputAccumulators();
}



					/////////////////////////////////////////////


void	   WavConvert::Initialze_For_File_Position_Change()
{


         // ***********   NEW,  what can I put in here ???   SlowDown  erase??    *****************

	int   dummy =  9;


	/*******   Possible function that should possibly go here.   9/3/2012


	Mp3Reader        m_mp3DecoderStreaming
	FFTslowDown	    m_fftSlowDown



	Erase_Players_AudioDelay_MemBuffer();

	ReAllocate_Players_AudioDelay_MemBuffer(   long  sampleCountNotSlowDown,    long  numberNeededToMatch   );  *** does this do too much ????


	m_fftSlowDown.Erase_SlowDown_Buffers();

	m_fftSlowDown.Erase_OutputAccumulators();


	m_fftSlowDown.Initialize_SlowDown_Variables();    *** does this do too much ????    CALLS,    Erase_SlowDown_Buffers();   


	****/

}




											////////////////////////////////////////
											////////////////////////////////////////


long    WavConvert::Get_Base_AudioDelay_inPieSlices()   
{

	                               //    allows a DELAY in Audio sound,  but only for Player at Speed1.     7/2012

	long     baseDelay =   -1; 


	if(    m_baseDelayAddr   !=  NULL    )
		baseDelay =    *m_baseDelayAddr;
	else
		baseDelay =    -1;    //  This would DISABLE  Player.exe  WAVdelay


	return  baseDelay;
}



long   WavConvert::Get_Overide_AudioDelay_inPieSlices()
{

	                               //    allows a DELAY in Audio sound,  but only for Player at Speed1.     7/2012

	long      overideDelay =   -1;    //  Special MODE:   If this has a value  >=0,  then that is the REAL DELAY to use,  and  m_baseDelay is processed as ZERO   


	if(    m_baseDelayAddr   !=  NULL    )
		overideDelay =    *m_overideDelayAddr;
	else
		overideDelay =    -1;    //  should disable this aspect of the delays calc    8/9/2012


	return   overideDelay;
}



											////////////////////////////////////////
											////////////////////////////////////////


void     WavConvert::Set_SRC_Sound_File(  CString&  srcFilePath,    bool&  retIsAmp3File    )
{

	//  **** CAREFUL *****     ,  this is meant for call from Navigator.  It will erase the other source files path.  Careful for CONVERSIONS   12/11


	retIsAmp3File =  false;

	
	bool   isAWAVfile =    Does_FileName_Have_Extension(  "wav",   srcFilePath   );
	bool   isAMP3file  =    Does_FileName_Have_Extension(  "mp3",   srcFilePath   );


	if(   ! isAWAVfile     &&   ! isAMP3file   )
	{
		ASSERT( 0 );

		m_srcMP3FilePath.Empty();
		m_srcWavFilePath.Empty();
		m_srcSoundFilesPath.Empty();

		return;
	//	retErrorMesg =  "Only pick a .WAV file or a .MP3 file." ;    // do not think this can happen
	//	return  false;
	}


	if(   isAWAVfile   )
	{

		retIsAmp3File =  false;

		m_srcMP3FilePath.Empty();    	//  **** CAREFUL *****

		m_srcWavFilePath      =   srcFilePath; 

		m_srcSoundFilesPath  =   srcFilePath; 
	}
	else if (  isAMP3file  )
	{

		retIsAmp3File =  true;

		m_srcWavFilePath.Empty();   	//  **** CAREFUL *****

		m_srcMP3FilePath =   srcFilePath;

		m_srcSoundFilesPath  =   srcFilePath; 
	}
	else
	{	ASSERT( 0 );  }
}



											////////////////////////////////////////


void   WavConvert::Release_WaveFormatEx()
{

	if(    m_wavFormat   !=  NULL    )
	{
		GlobalFree(   m_wavFormat   );      	m_wavFormat =   NULL; 
	}
}


											////////////////////////////////////////


void   WavConvert::Release_Media_File()
{

	if(    m_mmIO  !=   NULL   )                       //  1st file to potentially close...
	{
        mmioClose(   m_mmIO,   0  );		 
		m_mmIO =  NULL;	 	
    }



	m_mp3DecoderStreaming.close_binfile();     //  and 2nd file to potentially close

	m_fileUniStreaming.close();   


	m_isInitializedStreaming =   false;    //  OK here ???     2/18/10
}



					/////////////////////////////////////////////


void	   WavConvert::Release_ReSampling_Arrays()
{

	if(   m_reSampleArrayLeft  !=  NULL   )
	{
		free(  m_reSampleArrayLeft  );     m_reSampleArrayLeft =  NULL;   
	}

	if(   m_reSampleArrayRight  !=  NULL   )
	{
		free(  m_reSampleArrayRight  );     m_reSampleArrayRight =  NULL;   
	}
}


					/////////////////////////////////////////////


bool	   WavConvert::Alloc_ReSampling_Arrays(   long  numberOfSamples,   CString&  retErrorMesg   )
{

	retErrorMesg.Empty();
	ASSERT(  numberOfSamples > 0 );


	Release_ReSampling_Arrays();



	long     allocateSize  =      sizeof( float )  *  numberOfSamples;

	if(     (  m_reSampleArrayLeft =   ( float* )malloc( allocateSize )   )     == NULL    )  
	{
		AfxMessageBox(   "WavConvert::Alloc_ReSampling_Arrays  failed,  could not allocate  retSampleArrayLeft."  );
		return  false;
	}

	if(     (  m_reSampleArrayRight =   ( float* )malloc( allocateSize )   )     == NULL    )  
	{
		AfxMessageBox(   "WavConvert::Alloc_ReSampling_Arrays  failed,  could not allocate  retSampleArrayRight."  );
		return  false;
	}

	return  true;
}


											////////////////////////////////////////


bool    WavConvert::Open_Get_WavFormatEx(    CString&   filePath,     CString&   retErrorMesg   )
{

    int     errorCode;

	retErrorMesg.Empty();


//	m_isInitialized =   false;		  // **** WANT to use this always ????
			

												 //   re-initialize
	Release_WaveFormatEx();
	Release_Media_File();



    if(   (  errorCode=    WavFile_Open(   filePath.GetBuffer(0),    &m_mmIO,    &m_wavFormat,     &m_chunkInfoParent  )    )    !=  0   )
    {																		//   ALLOCATES  the  WaveFormatEx   structure

		if(    m_wavFormat   !=   NULL     )
		{  GlobalFree(  m_wavFormat   );	   m_wavFormat =  NULL;    }

		if(    m_mmIO  !=   NULL   )    
		{  mmioClose(   m_mmIO,   0  );		  m_mmIO =  NULL;       }	       


		retErrorMesg.Format(   "%s could NOT be opened or found" ,    filePath   );   //  goes directly to user
        return  false;
    }



																		  //  This moves us to the 'DATA' area in the file

    if(  (  errorCode=    WavFile_Start_Reading_Data(    &m_mmIO,     &m_mmChunkInfo,     &m_chunkInfoParent     )  )   != 0  )
    {

		if(    m_wavFormat   !=   NULL     )
		{  GlobalFree(  m_wavFormat   );	   m_wavFormat =  NULL;    }

		if(    m_mmIO  !=   NULL   )    
		{  mmioClose(   m_mmIO,   0  );		  m_mmIO =  NULL;       }	       

		retErrorMesg =     "BitSourceAudioMS::Open_Get_WavFormatEx  failed,  WavFile_Start_Reading_Data failed." ;
        return  false;
    }



	m_totalSamplesBytesWAVsrc  =     m_mmChunkInfo.cksize;    

	return  true;
}



									/////////////////////////////////////////////


bool	WavConvert::Is_A_StreamingFile_Loaded()
{

	if(       ! m_srcWavFilePath.IsEmpty()           // ******  IS this a reliable assumption ????  
		||   ! m_srcMP3FilePath.IsEmpty()    )
		return  true;
	else
		return  false;
}


									/////////////////////////////////////////////


bool	 WavConvert::Is_A_MP3_File_Loaded()
{

	if(    ! m_srcMP3FilePath.IsEmpty()    )
		return  true;
	else
		return  false;
}




											////////////////////////////////////////


long	   WavConvert::Calc_Samples_In_PieEvent(  double  playSpeed  )
{

						                           //   can use   SPitchCalc::m_playSpeed   for input value

	//  takes into account the REDUX  for  SPEED-slowDown


//	long   totalSamplesInPieSection  =       (  m_byteCountTwoSeconds  /  (  m_numberOfWavemanEvents  -1L ) )     / 4L;   
	long   totalSamplesInPieSection  =                                  (  88320   /  20   )                     /   4L;     //   1104



	long      sampCountInPieSliceWithSpeedRedux   =   (long)(    (double)totalSamplesInPieSection  /  playSpeed   );
	return   sampCountInPieSliceWithSpeedRedux;
}




										////////////////////////////////////////

/****
void    WavConvert::Set_Players_AudioDelay_MemberVar(    long   numberNeededToMatch    )
{

				//    'numberNeededToMatch'   is from the Primary CircQue's  effect as a MEDIAN filter.
				//
				//    Only changes the value in the Member Variable,  if want to also RESIZE the Buffer,   call ReAllocate_Players_AudioDelay_MemBuffer


	long   baseDelay =   Get_Base_AudioDelay_inPieSlices()  ;     //    2:  ***ADJUST***,     *******************   HARDWIRED ***********************



	short  appCode  =  Get_PitchScope_App_Code_GLB();         //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope

	ASSERT(   numberNeededToMatch  >=  1   );
	


	if(       m_fftSlowDown.m_playSpeed   !=   1.0  
		||   appCode  !=  0     )
	{
	           //   OK,   just do nothing.
	}
	else
	{  long   nuDelay =     baseDelay  +   numberNeededToMatch;

		m_delayInPieSlices =    nuDelay;
	}
}
****/


											////////////////////////////////////////


char*    WavConvert::Get_Oldest_Block_from_AudioDelays_CircularQue()		
{


		//  Since  'm_indexIntoBufferStreamingDelayed'  was incremented after the last DataWrite,  it now points to the OLDEST position in the Cue.


	ASSERT(     m_indexIntoBufferStreamingDelayed >= 0     &&     m_indexIntoBufferStreamingDelayed  <  m_delayInPieSlices    );  

	
	long   bytesPerSample  =   4;      //   ?????   always,   it is output
	long   sampsInPieSlice  =   Calc_Samples_In_PieEvent(   m_fftSlowDown.m_playSpeed  );  


	/***
	long   sizeOfBlock  =    sampsInPieSlice;  // *****************   BAD Assumptioon  7/26    ***********************
	ASSERT(  m_outputBufferSampleCount  ==   sizeOfBlock   );    //  *********** FAILS   for   GreySolo_22hz.wav     7/2012
	***/
	long   sizeOfBlock  =    m_outputBufferSampleCount;  


	long   offsetIntoCircQue  =    m_indexIntoBufferStreamingDelayed   *   sizeOfBlock     *   bytesPerSample; 

	char    *oldestBlockPtr   =      m_outputBufferStreamingDelayed    +    offsetIntoCircQue;
	return   oldestBlockPtr;  
}




											////////////////////////////////////////


char*    WavConvert::Get_Newest_Block_from_AudioDelays_CircularQue()		
{


ASSERT( 0 );   // *************   NOT  USED or TESTED yet.    7/2012    ******************************


	ASSERT(     m_indexIntoBufferStreamingDelayed >= 0     &&      m_indexIntoBufferStreamingDelayed  <  m_delayInPieSlices     );  


	long  newestIndex =   m_indexIntoBufferStreamingDelayed  -1;
	if(     newestIndex  < 0    )   
	{		
		newestIndex =   m_delayInPieSlices  - 1;
	}


	long   bytesPerSample  =   4;      //   ?????   alwyas,   it is output
	long   sampsInPieSlice  =   Calc_Samples_In_PieEvent(   m_fftSlowDown.m_playSpeed  );  


	/***
	long   sizeOfBlock  =    sampsInPieSlice;  // *****************   BAD Assumptioon  7/26    ***********************
	ASSERT(  m_outputBufferSampleCount  ==   sizeOfBlock   );
	***/
	long   sizeOfBlock  =    m_outputBufferSampleCount;  


	long   offsetIntoCircQue =     newestIndex   *   sizeOfBlock     *   bytesPerSample; 

	char    *newestBlockPtr  =      m_outputBufferStreamingDelayed    +    offsetIntoCircQue;
	return   newestBlockPtr;  
}




									/////////////////////////////////////////////////////


bool   WavConvert::Does_FileName_Have_Extension(   char* fileExtensionPtr,    CString&   filePath   )
{

	bool   hasTheExtention =  false;						//   new,   1/10


	if(    filePath.IsEmpty()    )
	{
		ASSERT( 0 );    // ERROR,  should have something
		return   false;
	}

	if(   fileExtensionPtr ==  NULL    )
	{
		ASSERT( 0 );    // ERROR,  should have something
		return   false;
	}



	CString  strOrigExten;


	int   pos =    filePath.ReverseFind(  '.'  );    // Strip out the extension
	if(   pos  <= 0  )  
	{
		ASSERT( 0 );    // ERROR,  should have something
		return   false;
	}


	strOrigExten  =    filePath.Right(   filePath.GetLength()  - pos   -1  ); 


	if(    strOrigExten.CompareNoCase(  fileExtensionPtr  )  ==   0   )
		hasTheExtention =   true;
	else
		hasTheExtention =   false;



	return  hasTheExtention;
}



									/////////////////////////////////////////////


short	 WavConvert::Get_Compatability_Code_for_Music_File(    CString&  srcFilePath,   CString&   retErrorMesg  )
{
	
					//	Returns:     0: Error, with message.     1: Can Use file as is.     2: Needs conversion.

	retErrorMesg.Empty();

	if(   srcFilePath.IsEmpty()   )
	{
		retErrorMesg =   "Create_VoxSep_Compatible_WAV_File failed,   srcFilePath is empty."  ;
		return  0;
	}


	CString   strExtension  =    srcFilePath.Right( 4 );


	if(          strExtension.CompareNoCase(  ".mp3"   )   ==  0    )
	{
		return 2;   
	}
	else if(   strExtension.CompareNoCase(  ".wav"   )   ==  0    )
	{


		Set_SRC_WAV_File(  srcFilePath  );



		if(    ! Get_WAV_Header_Info(    srcFilePath,
													   m_formatTypeWAVsrc,          
						                               m_channelsCountWAVsrc,     
													   m_bitsPerSampleWAVsrc,      
													   m_sampleRateWAVsrc,        
													  m_totalSamplesBytesWAVsrc,								
													  retErrorMesg  )     )
			return  0;




						// ********************   IS this for COMPRESSION ********************************
		if(    m_formatTypeWAVsrc   !=   1   )
		{
			retErrorMesg =   "When loading .WAV files,  their format must be  PCM and without compression."  ;
			return  0;
		}



		if(            m_sampleRateWAVsrc      ==   44100    						
		        &&   m_channelsCountWAVsrc  ==       2
			    &&   m_bitsPerSampleWAVsrc  ==      16    )
			return  1;    //  no conversion
		else
			return  2;   //   2: Needs conversion.
	}
	else
	{  retErrorMesg =   "Only attempt to load files with  .mp3  or  .wav  extensions."  ;
		return  0;
	}
}



					/////////////////////////////////////////////


bool	WavConvert::Create_VoxSep_Compatable_WAV_File(    CString&  srcFilePath,  CString&  retNewFileName,  
																															    CString&   retErrorMesg   )
{


			          //  first call Get_Compatability_Code_for_Music_File()  to see if the file EVEN NEEDS conversion 

	retErrorMesg.Empty();



	if(   srcFilePath.IsEmpty()   )
	{
		retErrorMesg =   "Create_VoxSep_Compatable_WAV_File failed,   srcFilePath is empty."  ;
		return  false;
	}



	CString   strExtension =    srcFilePath.Right( 4 );


	if(   strExtension.CompareNoCase(  ".mp3"   )   == 0   )
	{

		Set_SRC_MP3_File(  srcFilePath  );
		
		if(    ! Convert_MP3_to_WAV(  retNewFileName,   retErrorMesg   )     )
			return  false;
		else
			return  true;
	}
	else if(   strExtension.CompareNoCase(  ".wav"   )   == 0   )
	{

		Set_SRC_WAV_File(  srcFilePath  );
		
		if(   ! Convert_WAV_to_WAV(  retNewFileName,   retErrorMesg   )    ) 
			return  false;
		else
			return  true;
	}
	else
	{  retErrorMesg =   "Only attempt to load files with  .mp3  or  .wav  extensions."  ;
		return  false;
	}
}



					/////////////////////////////////////////////


bool	WavConvert::Set_ReSample_Rates(   long  srcSampRate,   long  dstSampRate,     CString&   retErrorMesg   )
{

	retErrorMesg.Empty();


	//  ***************************************************************************************
	//  ******Could run the input figures through	  rationalize()  [  ReSample.cpp  ],   to see if possible			


	m_srcSampRate  =    srcSampRate;    
		
	m_dstSampRate  =    dstSampRate;


	return  true;
}



					/////////////////////////////////////////////


bool    WavConvert::Get_MP3_Header_Info(    int&  layer,   int&  lsf,   int&  freq,   int&  stereo,   int&  rate,
																														  CString&   retErrorMesg  )
{

	retErrorMesg.Empty();
	layer =  lsf =  freq =  stereo = rate =   -1;


	if(   m_srcMP3FilePath.IsEmpty()   )
	{
		retErrorMesg =   "Get_MP3_Header_Info failed,   m_srcMP3FilePath is empty."  ;
		return  false;
	}



	FileUni   mp3File;

	try   
	{  
		if(   ! mp3File.open(   m_srcMP3FilePath,   0  )     )
		{
			retErrorMesg =   "Get_MP3_Header_Info failed,   could not open mp3File."  ;
			return  false;
		}


		if(    ! Get_MP3_Version_Set_Offset(   mp3File,  retErrorMesg  )      )
		{
			mp3File.close();   
			return  false;			
		}


		if(   ! Mp3Reader::getheader(  mp3File,   layer,  lsf,  freq,  stereo,  rate  )      )
		{
			retErrorMesg =   "Get_MP3_Header_Info failed, couyld not get header info."  ;

			mp3File.close();   
			return  false;			
		}


		mp3File.close();   
	}
	catch(   CFileException   *pException   )
	{
		TCHAR    szCause[ 255 ];  
		CString   strRaw;
		pException->GetErrorMessage(  szCause,  255  );
		pException->Delete();
		strRaw =  szCause;
		retErrorMesg.Format(   "Get_MP3_Header_Info  failed,  could not read MP3 file because %s." ,   strRaw  );
		return  false;
	}

	return   true;
}




					/////////////////////////////////////////////


bool    WavConvert::Get_MP3_Version_Set_Offset(    FileUni&  mp3File,    CString&   retErrorMesg   )
{   

/****
The mp3 heaer always starts with   FFF  and the last 4bits contain info on { Version(1bit)  Layer(2bits) and  Error Protection(1bit)  }

So a test for the start of the header loos for values between, butrnot including,    0xFFFF   and  0xFFF0 

See Wikipedia's article on MP3 for a good diagram of the MP3 header and a good summary og the format.

Usually the heaer is right t the start of the file, but some half-assed software will pad the beginning of the file with zeroes or other fluff.

****/

	retErrorMesg.Empty();

	long    lgRaw,   fourBuf;

//	short                  shRaw,  twoBuf;    ...< or >  do not work if it is a SIGNED Short ( see below )
	unsigned  short   shRaw,  twoBuf;




	mp3File.peek(   &lgRaw,    4   );       //                    0x03334449
	fourBuf =    ((lgRaw & 0xFF)<<24) |  ((lgRaw&0xFF00)<<8)  |  ((lgRaw>>8)&0xFF00)  |  ((lgRaw>>24)&0xFF  ); 
														//  ...now becomes   0x49443303

	mp3File.peek(   &shRaw,    2   );                              //    0x4449
	twoBuf  =    ( shRaw  <<  8 )  |  (  shRaw  >> 8  );    //    0x4944




	if(	    fourBuf  ==   0x1BA   )              //   full mpeg stream
	{
		retErrorMesg =   "Can not read this mp3 file. 'Full mpeg stream' is an unsupported format."  ;
		mp3File.close();  
		return  false;			
	}

	if(    fourBuf  ==    0x52494646   )     //  RIFF aka .WAV
	{
		retErrorMesg =   "Can not read this mp3 file. 'RIFF by WAV' is an unsupported format."  ;
		mp3File.close();  
		return  false;			
	}

	if(	   fourBuf  ==    0x494E464F   )   //  "what's this?    ah: I N F O ... ? did i do this?"   ( ...J M did not write this )
	{
		ASSERT( 0 );   //  **** Is this branch a mistake?  see comment above.   8/06

		retErrorMesg =   "Can not read this mp3 file. It has an UNKNOWN format."  ;
		mp3File.close();  
		return  false;			//  ****  OMIT this case ???  ******
	}




	if(    ( fourBuf   &   0xFFFFFF00 )   ==   0x49443300    )    //  ID3v2
	{

		mp3File.seek(  6 );   
		int  fileposMS  =   mp3File.Get_File_Position_Actual();		//  Get here from    mother_in_law.mp3,  talkin.mp3   


		int                   blklenMS  =    0;
		unsigned char  buf1;  

		mp3File.read(   &buf1,   1  );
		blklenMS  +=    buf1   <<  21;     //  0

		mp3File.read(   &buf1,   1  );
		blklenMS  +=    buf1   <<  14;     //   0

		mp3File.read(   &buf1,   1  );
		blklenMS  +=    buf1   <<  7;     //  1152

		mp3File.read(   &buf1,   1  );
		blklenMS  +=    buf1;                //  1244			????   BlockSize???   is this just for header???     1/10




		fileposMS  =     mp3File.Get_File_Position_Actual();    //  10


	    while (   //   blklenMS  
					      blklenMS  > 0     //  *************    '> 0'  :     ME,  hope this does not cause trouble. Untested. ********

		         &&   ! mp3File.eof()   )   
		{
			  mp3File.seekcur(  blklenMS );    //  MILaw   from  pos 10,  move out another  1244 positions to  1254	
			  blklenMS  -=   blklenMS;  //  and then keep going backwards in steps of 1244 till near the start of the file.  
		}											//  But for MILaw, can not back up anymore so we stay a filePos  1254


		fileposMS        =     mp3File.Get_File_Position_Actual();   //  	1254  for  mother_in_law.mp3       [   272   ???

		int  virtLenMS  =     mp3File.length()   -   fileposMS;       //  984944


		mp3File.Set_Virtual_Mode_On(   fileposMS,    virtLenMS  );   //  so now we assign the   OFFSET as 1254   for the  virutalMode  ops
		mp3File.seek(  fileposMS  );   //  now at pos 2508,   we advance another 1254 to  2508
	}


	else if(             twoBuf  <   0xFFFF   

		 //        &&    twoBuf  >   0x0000     //    0x0000  Is this too low???   For some unoffical mp3 files there might be a lot of zeros and garbage befor a header.   2/10
				   &&    twoBuf  >=  0xFFF0     //    ...  more cionsistent with the test below.   2/27/10
				)    
	{

		// In this case the identifying bytes are right at the beginning of the file.   2/10


		//  8/16/06   Get here with:   mother_2.mp3[ manually trimmed ].    downloads/Mp3PlayLib/sample.mp3
		//					jules.mp3,  SampleVBR.mp3



		int  blklenMS =    1024;   


		while(   //   blklen  
		                 blklenMS  > 0     //  *************    ">0"   ....ME,  hope this does not cause trouble ******

		     &&   ! mp3File.eof()    )			  
		{
			short  shLoc; 
		    mp3File.peek(   &shLoc,    2   );                                        //    0xFAFF        ...we are still at filePos = 0


		    unsigned short   twoBit  =    ( shLoc  <<  8 )  |  (  shLoc  >> 8  );     //    0xFFFA     ...need unsigned or the inequality test below will fail
	
			if(   (  twoBit  >=  0xFFF0  ) && (  twoBit   !=  0xFFFF  )   )
				 break;              //  goes here for  mother_2.mp3  ( had trim-job ),  sample.mp3



			mp3File.seekcur(  blklenMS );
			blklenMS  -=   blklenMS;
		}

		//  int  fileposMS   =    mp3File.Get_File_Position_Actual();  




		char tag[3];

		int   pos      =    mp3File.Get_File_Position_Actual();    //  0      here it returns memberVar  'filepos'  which is the OFFSET

		int   fileLen  =    mp3File.length();    //   899994

	
		mp3File.seekend(  -128  );
		mp3File.read(   tag,   3    );


		int  var           =     memcmp(  tag, "TAG", 3  )?0:128;    //  128,    cause value of 'tag' is  TAG

		int  virtLength  =     fileLen   - pos   - ( memcmp(  tag, "TAG", 3  )?0:128);  //  899866
		

		mp3File.Set_Virtual_Mode_On(   pos,    virtLength  );   //   0,    899866
		mp3File.seek(  pos  );
	}

	else if(   twoBuf  ==  0x0000   )    //    0x0000  Is this too low??   For some unoffical mp3 files there might be a lot of zeros befor a header.   2/10
	{

		bool   foundStart =   false;
		long   travPos = 0,  fileposMS,    maxSearchBytes =  2048;   // ********* ADSJUST ???  **********

	
		while(   ! foundStart    
		     &&   ! mp3File.eof() 
			 &&     travPos  <  maxSearchBytes  )			  
		{
			short  shLoc; 
			mp3File.seek(  travPos );

		    mp3File.peek(   &shLoc,    2   );                                        //    0xFAFF        ...we are still at filePos = 0


		    unsigned short   twoBit  =    ( shLoc  <<  8 )  |  (  shLoc  >> 8  );     //    0xFFFA     ...need unsigned or the inequality test below will fail
	
			if(   (  twoBit  >=  0xFFF0  ) && (  twoBit   !=  0xFFFF  )   )
			{
				fileposMS        =     mp3File.Get_File_Position_Actual();   
				foundStart =  true;
				break;              //  first fourBit groups of header start are    { F F F ? }      0xFFF0 because we do NOT have a fixed value for the fourth fourBit 
			}

			travPos +=  1;   //  is 2 better ???
		}  //  while( ! foundStart    


		if(   ! foundStart  )
		{
			retErrorMesg =   "Can NOT read this mp3 file. It has an unknown and unsupported format. [ Get_MP3_Version_Set_Offset 2 ]"  ;
			mp3File.close();  
			return  false;			
		}


		char tag[ 3 ];
		int   pos      =    mp3File.Get_File_Position_Actual();    //  0      here it returns memberVar  'filepos'  which is the OFFSET
		int   fileLen  =    mp3File.length();    //  

		
		mp3File.seekend(  -128  );       //  now try to read the VIRTUAL length at the end of the file.
		mp3File.read(   tag,   3    );

		int  var          =     memcmp(  tag, "TAG", 3  )?0:128;    //  128,    cause value of 'tag' is  TAG

		int  virtLength =    fileLen   - pos   - ( memcmp(  tag, "TAG", 3  )?0:128);    //  many of the time this is 0

		mp3File.Set_Virtual_Mode_On(   pos,    virtLength  );   // 


		mp3File.seek(  pos  );     //    this works      [  would not this put us ahead oft the first header ???  But this is what was in the original code.   2/10
//		mp3File.seek(  0  );        //    and this also works.  
	}

	else
	{  retErrorMesg =   "Can NOT read this mp3 file. It has an unknown and unsupported format. [ Get_MP3_Version_Set_Offset  1 ]"  ;
		mp3File.close();  
		return  false;			
	}

	return  true;
}



					/////////////////////////////////////////////


void    WavConvert::Write_WAV_Header(   CFile &file,    int rate,   int stereo,   int bit16,    int len   )
{

	/****
	  file->write( "RIFF",  4 );
	  putil4(  *file,  len +38  );
	  file->write("WAVE", 4);
	  file->write("fmt ", 4);
	  putil4(  *file, 18  );
	  putil2(  *file, 1 );
	  putil2(  *file, stereo?2:1  );
	  putil4(  *file, rate  );
	  putil4(  *file, rate * (stereo?2:1)*(bit16?2:1)  );
	  putil2(  *file, (stereo?2:1) * (bit16?2:1)  );
	  putil2(  *file, bit16?16:8 );
	  putil2(  *file, 0);
	  file->write(  "data", 4  );
	  putil4(  *file, len);
	****/
	unsigned long   buf4 =  0;
	unsigned short  buf2;

	file.Write(  "RIFF",  4    );	

	buf4 =   len +38;
	file.Write(   &buf4,    4  );	

	file.Write(  "WAVE",  4  );	

	file.Write(  "fmt ",   4   );	

	buf4 =   18;
	file.Write(    &buf4,  4      );	

	buf2 =   1;
	file.Write(    &buf2,   2   );	



	buf2 =   stereo?2:1;
	file.Write(  &buf2,   2     );	

	buf4 =   rate;
	file.Write(   &buf4,   4   );	
	
	buf4 =   rate * (stereo?2:1)*(bit16?2:1) ;
	file.Write(   &buf4,   4   );	

	buf2 =   ( stereo?2:1 )  *  ( bit16?2:1 );
	file.Write(  &buf2,   2     );	



	buf2 =      bit16?16:8;
	file.Write(  &buf2,   2     );	

	buf2 =      0;
	file.Write(  &buf2,   2     );	

	file.Write(  "data",  4  );

	buf4 =   len;
	file.Write(   &buf4,   4   );	
}



					/////////////////////////////////////////////


void   WavConvert::Write_WAVfiles_Length(   CFile &file   )
{

	DWORD  length  =    file.GetLength();


	file.Seek(   4,   CFile::begin   );
	DWORD  buf =    length  -  8;
	file.Write(   &buf,   4   );


	file.Seek(   42,   CFile::begin   );
	buf =    length  -  46;
	file.Write(   &buf,   4   );
}













					/////////////////////////////////////////////

#define  MAXmp3BUFFERSIZE    4608



bool    WavConvert::Convert_MP3_to_WAV(   CString&   retNewFileName,   CString&   retErrorMesg   )
{   

	//	 Must REWRITE this if I want a streaming player. 



	long   defaultSamplingRate  =   m_defaultSamplingFreqOutput;    // 44100; 

    long   bytesPerSample        =   m_defaultBytesPerSampleOutput;    //    4 for  16bit stereo        


	long   origBufSize =     4416;      //   was  4608    **** Can ADJUST***********,  just do not make it bigger than the array below




	retNewFileName.Empty();
	retErrorMesg.Empty();

	if(   m_srcMP3FilePath.IsEmpty()   )
	{
		retErrorMesg =   "Convert_MP3_to_WAV failed,   m_srcMP3FilePath is empty."  ;
		return  false;
	}

	
	if(   m_srcMP3FilePath.IsEmpty()   )
	{
		retErrorMesg =   "Convert_MP3_to_WAV failed,   m_srcMP3FilePath is empty."  ;
		return  false;
	}


	if(    origBufSize  >  MAXmp3BUFFERSIZE   )
	{
		retErrorMesg =   "Convert_MP3_to_WAV failed,   origBufSize is set too big."  ;
		return  false;
	}




	char  *filename =    m_srcMP3FilePath.GetBuffer( 0 );  
	FileUni   mp3File;


	try   
	{  
		if(   ! mp3File.open(   m_srcMP3FilePath,   0  )     )
		{
			retErrorMesg =   "Convert_MP3_to_WAV failed,   could not open mp3File."  ;
			return  false;
		}


		long   srcFileLength  =    mp3File.length();
		ASSERT(   srcFileLength  >  0  );


		if(    ! Get_MP3_Version_Set_Offset(   mp3File,  retErrorMesg  )     )  //  sets the virtual FileOffset for later reads of mp3File
		{
			mp3File.close();   
			return  false;			
		}



		int  layer,  lsf,   freq,  stereo,  rate;


		if(   ! Mp3Reader::getheader(  mp3File,   layer,  lsf,  freq,  stereo,  rate  )      )
		{
			retErrorMesg =   "Convert_MP3_to_WAV failed, could not get header info."  ;
			mp3File.close();   
			return  false;			
		}

	//	TRACE(  "\n\n   layer %i,   %i Hz %s,   %i kbps   \n\n",      layer+1,   freq,    stereo?"stereo":"mono",   rate/1000  );





		ReSampler    reSampler;

		float    *retSampleArrayLeft =  NULL,   *retSampleArrayRight =  NULL;   //  2 intermediate samples
		char    *dstSamplesPtr =   NULL;   //   final dsample at new freq

		bool    doResampling;
		int       outputFreq;
		long    totSamplesCnt =  -1;
		long    retDstStreamBufferSizeInSamples =  -1;    



		if(   freq  !=   defaultSamplingRate   )    //  do NOT always need to do this.   Mother In Law (Tysse)
//		if(   2 ==  3   )   //  disable
		{

			if(    ! Set_ReSample_Rates(   freq,   defaultSamplingRate,   retErrorMesg   )    )
			{
				ASSERT( 0 );
				return  false;
			}
		
			outputFreq       =    defaultSamplingRate;   //   44100  hz



			//     long  streamBufferSizeInSamples,    long&  retDstStreamBufferSizeInSamples,    

			bool   stereoFlag =  true;

			if(   stereo  != 1   )     // **** did not test this yet
				stereoFlag =  false;



			if(    ! reSampler.Initialize(   freq,   outputFreq,   origBufSize,  retDstStreamBufferSizeInSamples,  bytesPerSample,  stereoFlag,  retErrorMesg  )     )
			{
				// ******  INSTALL cleanup ??? *****************************
				ASSERT( 0 );
				return  false;
			}



			totSamplesCnt =     origBufSize  /  bytesPerSample;   
		
			long    allocateSize     =     sizeof( float )  *  totSamplesCnt;


			if(     (  retSampleArrayLeft =   ( float* )malloc( allocateSize )   )     == NULL    )  
			{
				ASSERT( 0 );
				retErrorMesg =   "Convert_MP3_to_WAV  failed,  could not allocate  retSampleArrayLeft."  ;
				return  false;
			}


			if(     (  retSampleArrayRight =   ( float* )malloc( allocateSize )   )     == NULL    )  
			{
				ASSERT( 0 );
				retErrorMesg =   "Convert_MP3_to_WAV  failed,  could not allocate  retSampleArrayRight."  ;
				return  false;
			}




			long  dstSampleCount =     (long)(      ( (double)outputFreq / (double)freq    +  0.05 )   * (double)origBufSize   ); 
																														  //  0.05 :  make 5% bigger for safety		
			long  allocateDstSize   =     dstSampleCount  *   bytesPerSample;			

			if(     (  dstSamplesPtr =   ( char* )malloc( allocateDstSize )   )     == NULL    )  
			{
				ASSERT( 0 );
				retErrorMesg =   "Convert_MP3_to_WAV failed,  could not allocate  dstSamplesPtr."  ;
				return  false;
			}
			


			doResampling  =    true;					
		}
		else
		{  doResampling      =   false;			
			m_dstSampRate  =   m_srcSampRate  =   -1;
			outputFreq =   freq;
		}





		Mp3Reader    decoder;	 //  ***  the Big Kahuna  ***

		int  chn=0;
		int  down =  0;      //   1:  Cuts sample rate in half



		if(    ! decoder.open(   mp3File,   freq,   stereo,   1,   down,   chn  )     )
		{
			retErrorMesg =   "Convert_MP3_to_WAV failed,  could not open decoder." ;
			mp3File.close();   
			return  false;  
		}



		char *newname=0;

		newname  =   new  char[ strlen(  filename)  +5 ];

		  if (strrchr(filename, '\\'))
			strcpy(newname, strrchr(filename, '\\')+1);
		  else if (strrchr(filename, '/'))
			strcpy(newname, strrchr(filename, '/')+1);
		  else if (strrchr(filename, ':'))
			strcpy(newname, strrchr(filename, ':')+1);
		  else
			strcpy(newname, filename);

		if (   !strcmp(   newname  +strlen(newname)  -4,   ".mp3"  )   )
			newname[  strlen(newname) - 4  ] =  0;

		strcat(  newname, ".wav"  );

		retNewFileName =    newname;
		




		CFile   wavFile(     newname,   //  strJMwavPath, 
										 CFile::modeCreate    |  CFile::modeWrite
									 //  CFile::modeCreate    |  CFile::modeWrite     |   CFile::modeNoTruncate
								 );				  // ***NOTE:   'modeNoTruncate'   allows APPEND to existing fie,  does not EMPTY/ERASE an EXISTING file

		Write_WAV_Header(   wavFile,   outputFreq,   stereo,   1,  0   );   // **** CAREFUL,  freq is REPLACED

		delete  newname;




		float   bal= 0;
		float   ctr= 0;
		float   sep= 1;
	    float   srnd= 1;
		float   vol= 1;
		float   vols[3][3];

		vols[0][0]=0.5*(1.0-bal)*(1.0-ctr+sep);
		vols[0][1]=0.5*(1.0-bal)*(1.0+ctr-sep);
		vols[0][2]=1-bal;
		vols[1][0]=0.5*(1.0+bal)*(1.0-ctr-sep)*srnd;
		vols[1][1]=0.5*(1.0+bal)*(1.0+ctr+sep)*srnd;
		vols[1][2]=(1.0+bal)*srnd;
		vols[2][0]=0.5*(1.0-ctr);
		vols[2][1]=0.5*(1.0+ctr);
		vols[2][2]=1.0;

		decoder.ioctl(   decoder.ioctlsetstereo,  vols,   0   );




		char  sampbuf[  MAXmp3BUFFERSIZE  ];      //    4608 the read buffer,  receives data from the decoder
	//	int     bps       =   ( stereo?4:2 ) * freq;
		int     chvol    =  1;
		int     bread   =  0;     //  number of bytes read

		long   blockCount =  0;
		



		Begin_ProgressBar_Position_GLB(   "MP3 conversion..."   );   
		Set_ProgressBar_Position_GLB(  5  );
		



		while(    ! decoder.eof()    )		//   loop to PROCESS all the bytes in the file 
		{

		  if(  chvol  )
		  {
			chvol  =  0;			//   goes here
			float vol0 = vol;
			decoder.ioctl(    decoder.ioctlsetvol,   &vol0,   0    );
		  }



		  int      chpitch =       1;
		  float   pitch    =       0;
		  float   pitch0  =   1764;


		  if(  chpitch  )          //  *** Can I change pitch here for  frequecy change ????  *************** 
		  {
			chpitch =  0;
			float   l3equal[ 576 ];

			for (  int  i= 0;   i< 576;   i++  )
			  l3equal[ i ]  =      exp(       pitch *      log(     (i+0.5) * freq   /  ( 576.0 * 2 * pitch0 )     )              );

			decoder.ioctl(   decoder.ioctlsetequal576,    l3equal,   0  );
		  }




//		  int     bufSize       =    4608;    //  must init this var for each loop cycle  ( could I use different sizes????  It is the size of the ReadBuffer    sampbuf[  4608  ];
		  int     bufSize  =    origBufSize;


		  bufSize  =    decoder.read_binfile(  sampbuf,   bufSize  );    //  *** BIG,   ultimately calls   ampegdecoder::decode()   *****
		  if(  ! bufSize  )
		  {
			  retErrorMesg  =   "Convert_MP3_to_WAV failed,  decoder's read was bad." ;
			  mp3File.close();  
			  decoder.close_binfile();
		 //  Write_WAVfiles_Length(  wavFile   );
			  wavFile.Close();
			  End_ProgressBar_Position_GLB();
			  return  false;  
		  }

		  bread  +=   bufSize;    //  Bytes Read







//  ******** REFINE for different sample sizes **************************
		  if(   doResampling   )
		  {
							//   separate into Left and Right samples, do resampling 2x,  then merge together for final WAV format 

			    bool  lessThanOneValues =   false;

																																	//  left

			  	if(   ! SndSample::Make_Float_Sample(  0,    sampbuf,    origBufSize,   bytesPerSample,    retSampleArrayLeft,   lessThanOneValues,	retErrorMesg  )   )
				{
					End_ProgressBar_Position_GLB();
					return  false;
				}


				float   *yptptLeft =   NULL;        //  intdecF_ReSample  will alloc memory for this return sample.
				int        ylenLeft =   -1;             //  intdecF_ReSample  will return this value


			//	  reSampler.intdecF_ReSample(  	retSampleArrayLeft,   totSamplesCnt,   m_srcSampRate,   &yptptLeft,  	&ylenLeft,    m_dstSampRate   );
				reSampler.Resample_Signal(  	retSampleArrayLeft,   totSamplesCnt,  &yptptLeft,   	&ylenLeft );




																																	//	right

			  	if(   ! SndSample::Make_Float_Sample(  1,    sampbuf,    origBufSize,   bytesPerSample,    retSampleArrayRight, lessThanOneValues,   retErrorMesg  )   )
				{
					End_ProgressBar_Position_GLB();
					return  false;
				}


				float   *yptptRight =   NULL;       
				int        ylenRight =   -1;            


			//	 reSampler.intdecF_ReSample(  	retSampleArrayRight,   totSamplesCnt,   m_srcSampRate,   &yptptRight,   &ylenRight,    m_dstSampRate   );
				reSampler.Resample_Signal(  	retSampleArrayRight,   totSamplesCnt,      &yptptRight,     &ylenRight );




				ASSERT(   ylenRight  ==   ylenRight   );

				bool    toStereo =   true;
			//	char   *retDstSamplesPtr =  NULL; 
				long    retByteCount =  -1; 

				bool   noMemoryRelease  =   false; 

				double  volumeTweak  =   1.0;    //   1.0 is default  



				if(    ! SndSample::Merge_Float_Channels_To_Sample(    toStereo,    ylenRight,     bytesPerSample,    
					                                                            &yptptLeft,    &yptptRight,   
																			           &dstSamplesPtr,      retByteCount,  lessThanOneValues,  noMemoryRelease, volumeTweak,   retErrorMesg   )   )
				{
						// ******  INSTALL other cleanup ????  ****************************************
					End_ProgressBar_Position_GLB();
					return  false;
				}
				else
				   wavFile.Write(   dstSamplesPtr,    retByteCount   );
		  }
		  else     
		      wavFile.Write(   sampbuf,     bufSize   );	



		  blockCount++;





		  int  curSrcPos            =       mp3File.Get_File_Position_Actual();

		  double  percentDone  =    (  (double)curSrcPos    /   (double)srcFileLength   )    * 100.0;

		  Set_ProgressBar_Position_GLB(  (long)percentDone  );



		  if(  (  blockCount  %  30  )   ==  0    )
			  TRACE(  "Block  %d   has been written( mp3->WAV ).\n",   blockCount   ); 

		  /****
			fprintf(  stdout, "\r%.3f",   (float)((float)decoder.tell()/bps)   );
			if (  decoder.length()  )
			  fprintf(stdout, "/%.3f=%.3f", (float)((float)decoder.length()/bps), (float)((float)decoder.tell()/decoder.length()));
		  ****/      

		}   //   while(  !decoder.eof()   )




														//   CLEANUP,  close all files  

		End_ProgressBar_Position_GLB();



		if(   doResampling  )
		{

			if(   retSampleArrayLeft  !=  NULL   )
			{
				free(  retSampleArrayLeft  );     retSampleArrayLeft =  NULL;   
			}
			else
				ASSERT( 0 );   //  it sould have been allocated


			if(   retSampleArrayRight  !=  NULL   )
			{
				free(  retSampleArrayRight  );     retSampleArrayRight =  NULL;   
			}
			else
				ASSERT( 0 );   //  it sould have been allocated



			if(   dstSamplesPtr  !=  NULL   )
			{
				free(  dstSamplesPtr  );     dstSamplesPtr =  NULL;   
			}
			else
				ASSERT( 0 );   //  it sould have been allocated
		}


		decoder.close_binfile();

		Write_WAVfiles_Length(  wavFile   );
		wavFile.Close();

		mp3File.close();   
	}
	catch(   CFileException   *pException   )
	{
		TCHAR    szCause[ 255 ];  
		CString   strRaw;
		pException->GetErrorMessage(  szCause,  255  );
		pException->Delete();
		strRaw =  szCause;
		retErrorMesg.Format(   "Convert_MP3_to_WAV  failed,  could not read MP3 file because:  %s." ,   strRaw  );
		return  false;
	}
 
	return true;
}




				/////////////////////////////////////////////


long	   WavConvert::Calc_Files_Total_Output_Bytes_no_SpeedExpansion()
{

					 //  **** No speed- Expansion,  but it does have RE-SAMPLING  expansion.  OUTPUT BYTES Format[44hz 16bit ]   ****


		//   ONLY CALLED by     WavConvert::Calc_Files_Total_Output_Bytes_With_SpeedExpansion()   and     BitSourceStreaming::Get_Files_Total_Samples_No_SpeedExpansion()


		//   BUT   BitSourceStreaming::Get_Files_Total_Samples_No_SpeedExpansion()  is called by MANY functions,  thru  AudioPlayer::Get_Biggest_SampleIndex_No_SpeedExpansion()




											//  Also allows for   ReSampler's  EXPANSION,   and the expansion from MONO to STEREO
	long   retTotalBytes =  -1;



	double   biggestLongFlt  =   2147483000.0;               //                               2,147,483,647
														//  Do What you like [ 15 minutes]  is       161,932, 848    ...which is   1/13  the size of MAX-Intiger  



	ASSERT(   m_isInitializedStreaming   );



	long   approximateLength  = -1;						//	  1)  	get the ACTUAL length

	if(    Is_A_MP3_File_Loaded()    )
		approximateLength  =     m_mp3DecoderStreaming.Get_Total_FileSize_inBytes_OutputCoords();
	else
		approximateLength  =     m_totalSamplesBytesWAVsrc;    // ******  OK ????   *************




	long   expandLength =  -1;


	if(    m_doResamplingStreaming     )               //	  2)  	if doing ReSampling, then the file will SEEM longer. 
	{

		ASSERT(   m_reSamplerStreaming.m_isInitialized    );
		ASSERT(   m_reSamplerStreaming.m_sampleExpansionRatio  !=  1.0   );      

		expandLength =    (long)(    m_reSamplerStreaming.m_sampleExpansionRatio   *   (double)approximateLength    );  
	}																										//  ReSample will create even more bytes for 16bit stereo
	else
		expandLength  =   approximateLength;



																		//  3)   If it uses  8bit samples then the 16bit OUTPUT will be 2x as long

	if(        m_bitsPerSampleSRCstream  ==  16  )
	{
		//  do nothing,  output is always 16 bit
	}
	else if(   m_bitsPerSampleSRCstream  ==  8  )
		expandLength  =   expandLength   *  2;
	else
	{	ASSERT( 0 );   }


																		//  4)    and a MONO file will make it seem like there are more OUTPUT-bytes in the file
	if(   ! m_stereoFlagSRCstream   )
		expandLength =   expandLength  * 2; 






	if(    expandLength  < 0   )      // Did the number OVERFLOW its LIMITS ???
	{

		ASSERT( 0 );       // *** WHEN last LAND here??  [     ]

		retTotalBytes  =   (long)biggestLongFlt;    //  *****************   FIX  this crappy substitute??   3/2012  ***********************
	}
	else
		retTotalBytes  =    (long) expandLength;   



	return   retTotalBytes;
}





				/////////////////////////////////////////////


long	   WavConvert::Calc_Files_Total_Source_Samples()
{

		//  NEW,  8/2012     Accounts for ReSampling,  so resample files will have LESS samples for same time-length because of the difference freq rate  

		//    This is used yet.  8/2012

	long     totalSamples =   -1;


	if(    Is_A_MP3_File_Loaded()    )
	{	
//		approximateLength  =     m_mp3DecoderStreaming.Get_Total_FileSize_inBytes_OutputCoords();
		ASSERT( 0 );  // ***************   INSTALL *****************          	totalSamples =    m_totalSourceSamples;
	}
	else
		totalSamples =    m_totalSourceSamples;



	return   totalSamples;
}




					/////////////////////////////////////////////


long	   WavConvert::Calc_Files_Total_Output_Bytes_With_SpeedExpansion()
{

							   //  **** CAREFUL  also calcs  with  *** SLOW DOWN   expansion ***


											//  Also allows for   ReSampler's  EXPANSION,   and the expansion from MONO to STEREO
	long   retTotalBytes =  -1;


	ASSERT(   m_isInitializedStreaming   );

	double   biggestLongFlt  =   2147483000.0;           //   2147483647



	long	 byteCount =    Calc_Files_Total_Output_Bytes_no_SpeedExpansion();



	if(    m_fftSlowDown.m_playSpeed  >  1.0   )
	{

		double   expandedLengthFlt   =       (double)byteCount   *   m_fftSlowDown.m_playSpeed;

		if(          expandedLengthFlt  >   biggestLongFlt   )
		{

			int  dummy  =  9;    // Landed here for slow speed with a Big File  (  35  minutes at Speed 6  )

			retTotalBytes  =    (long)biggestLongFlt;    //  *****************   FIX  crappy substitute  ***********************
		}
		else
			retTotalBytes  =    (long) expandedLengthFlt;   

	}
	else
		retTotalBytes  =    byteCount;


	return   retTotalBytes;
}



					/////////////////////////////////////////////


bool	 WavConvert::Seek_to_Frame_Position_for_Streaming(   long   frameIdx,    CString&   retErrorMesg  )
{

			//  so far,  only called  by test functions    2/4/10


	if(   ! m_isInitializedStreaming   )
	{
		retErrorMesg =   "WavConvert::Seek_to_Frame_Position_for_Streaming  FAILED,  WavConvert is not initialized.";
		return  false;
	}



	long  frameSize   =		 m_mp3DecoderStreaming.Get_Frame_Size(); 	   
//	long  frameCount = 	 m_mp3DecoderStreaming.Get_Number_Frames();		

//	long  fileSizeVirtual    =    frameSize   *   frameCount;  
	long  fileSizeVirtual    =    m_mp3DecoderStreaming.Get_Total_FileSize_inBytes_OutputCoords();


	long   newFilePos =    frameIdx  *  frameSize;


	if(   newFilePos  >=   fileSizeVirtual   )
	{
		retErrorMesg =   "WavConvert::Seek_to_Frame_Position_for_Streaming  FAILED,  newFilePos is out of bounds..";
		return  false;
	}



//	int  changedFilePos =     m_mp3DecoderStreaming.seek_binfile(  newFilePos  );              
	int  changedFilePos =     m_mp3DecoderStreaming.Seek_fromBeginning_in_OutputCoords(  newFilePos  );      


	return   true;
}





					/////////////////////////////////////////////


void    WavConvert::Set_MP3reader_Timing_Correction(  long  newValue  )
{

		//  ******* BIG and DANGEROUS ******  Helps the seek be more accurate.  Need to test with all formats.    7/31/2012 

		//   '4'  seems to work the best     7/31/2012


		//   CALLED by:    BitSourceStreamingMP3::Seek_New_DataPosition_Fast()



	m_mp3DecoderStreaming.Set_Frame_Timing_Correction(  newValue  );
}




					/////////////////////////////////////////////


bool	 WavConvert::Seek_to_BytePosition_in_OutputCoords_for_StreamingMP3(   long  byteIdx,    long&  retNewFilePos,    CString&   retErrorMesg   )
{


	//   only CALLED by    BitSourceStreamingMP3::Seek_New_DataPosition_Fast()



			//    byteIdx :   Is in the  UN-compressed  Output  bytes   ...BUT
			//
			//       This should be fixed so that   it can also account for the   ReSampler's   expansion to the  bytes
			//       or should I do that in the calling function  [   ]


	retNewFilePos =  -1;
	retErrorMesg.Empty();

	if(   ! m_isInitializedStreaming   )
	{
		retErrorMesg =   "WavConvert::Seek_to_Frame_Position_for_Streaming  FAILED,  WavConvert is not initialized.";
		return  false;
	}


	long   frameSize   =		 m_mp3DecoderStreaming.Get_Frame_Size(); 	      
	long   frameCount = 	     m_mp3DecoderStreaming.Get_Number_Frames();		

	long   fileSizeVirtual    =    frameSize   *   frameCount;      //   this is in the   UNCOMPRESSED-coords   of the mp3 file



//	long   bytesPerSample     =   m_defaultBytesPerSampleOutput;      // 4;   Isnt this really SRC-coords ???  
	long   bytesPerSample     =   m_bytesPerSampleSRCstream;  



//	double  playSpeed  =    m_fftSlowDown.m_playSpeed;   //   OK ????    7/12
//	long	 sampsInPieSlice  =    Calc_Samples_In_PieEvent(   playSpeed  );
//	long   bytesInPieSlice   =    sampsInPieSlice  *  bytesPerSample;


	
//	long   bytIndexBoundary  =    (  byteIdx / bytesPerSample  )     *   bytesPerSample;    //  ??? WRONG???    7/2012    needs to be on sample boundaries ...every 4 bytes (16bit stereo )
																																  //                 and PROBABLY  every 2 bytes for a mono.

//	if(   bytIndexBoundary   >=   fileSizeVirtual   )
	if(          byteIdx            >=   fileSizeVirtual   )
	{
		retErrorMesg =   "WavConvert::Seek_to_BytePosition_in_OutputCoords_for_StreamingMP3  FAILED,  newFilePos is out of bounds..";
		return  false;
	}





/****
	if(    forRecording    )
	{
		retNewFilePos =     m_mp3DecoderStreaming.Seek_WalkfromBeginning_in_OutputCoords(   byteIdx,   bytesInPieSlice   );  
	}
	else
	{  long  modNewPos    =      (  byteIdx  /  mp3FrameSize  )   *    mp3FrameSize;

		if(     modNewPos  !=   byteIdx    )
		{
			retNewFilePos =     m_mp3DecoderStreaming.Seek_fromBeginning_in_OutputCoords(  byteIdx  );  //  Just use the WEAKER of the 3 Seek functions.  7/2012
		}
		else
		{
			retNewFilePos =     m_mp3DecoderStreaming.Seek_fromBeginning_in_OutputCoords_BlockAlaign(  modNewPos  );
		}
	}
***/
	retNewFilePos =     m_mp3DecoderStreaming.Seek_fromBeginning_in_OutputCoords(  byteIdx  );              //  Now that I fixed the SEEK BUG in
	                                                    //  Mp3Reader [ with  m_frameRetardFactor  ],  can just use the WEAKER of the 3 Seek functions and get good results.   7/31/2012
	




	if(   retNewFilePos  <  0   )
	{
		retErrorMesg.Format(  "Seek_to_BytePosition_in_OutputCoords_for_StreamingMP3  FAILED,  Seek_WalkfromBeginning_in_OutputCoords[ %d ]",   retNewFilePos   );
		return  false;
	}

	return   true;
}




					/////////////////////////////////////////////


long		WavConvert::Get_Current_FilePos_ReadersMemberVar_UNcompressed_Output_Bytes()
{
		//   in UN-COMPRESSED   Output   file coords

	long      filePos  =	m_mp3DecoderStreaming.Get_MemberVar_FilePosition_OutputCoords();    //  [  m_mp3DecoderStreaming.filepos   ]     This is the actual filepos,  offsetted from the header
	return   filePos;
}



long		WavConvert::Get_Current_FilePos_In_UNcompressed_Output_Bytes()
{
														//   in UN-COMPRESSED   Output   file coords    Maybe the same as   Get_Current_FilePos_ReadersMemberVar_UNcompressed_Output_Bytes()    ?????
							
	//  but  NOT expanded by ReSampler


	long      exFilePos  =    m_mp3DecoderStreaming.Get_Calced_FilePosition_OutputCoords();   
	return   exFilePos;
}




					/////////////////////////////////////////////


long		WavConvert::Get_Current_SampleIndex_ReSamplerExpanded()
{

			//   NO CALLERS...  will have to fix if want to use.  Also have to think about speed slow down ????   


			//  gets File Postion  in OUPUT-coords (uncomporesseed)   and EXPANDED  by the ReSampler's  enlargement    2/10


	long   bytesPerSample =   m_defaultBytesPerSampleOutput; 



	long     retSampleIdx =  -1;
	long     reSampFilePos  = -1; 



	long	    filePos       =    Get_Current_FilePos_ReadersMemberVar_UNcompressed_Output_Bytes();               //   {  return  filepos;   }    Is this VIRTUAL ??? 

	long		exFilePos  =    Get_Current_FilePos_In_UNcompressed_Output_Bytes();    //  in the OUTPUT-coords of the file,  when the compressed bytes are expanded to output size


	if(   filePos   !=  exFilePos   )    //  two function may do the same calc
	{
		int   dummy =   9;
	}




	if(    m_doResamplingStreaming ==  false   )      //  NO   ReSampling,  can I count on the fact that		
	{

		retSampleIdx  =    exFilePos  /   bytesPerSample;  

		if(    ( exFilePos  %   bytesPerSample )   !=   0   )
		{
			int   dummy  =   9;
		}

	}
	else
	{  
		if(     m_reSamplerStreaming.m_sampleExpansionRatio  !=  1.0    )    //   Need to calc expansion
		{

			reSampFilePos  =    (long)(     m_reSamplerStreaming.m_sampleExpansionRatio   *   (double)exFilePos       );   // **** APPROXIMATE,  need to be exact ?????  
						// ******* SEEMS like an APPROXIMATION...   try to do better   **************************

			retSampleIdx       =    reSampFilePos  /   bytesPerSample;


			if(    ( reSampFilePos  %   bytesPerSample )   !=   0   )
			{
				int   dummy  =   9;     //  **** Hit here some of the time.  Do bot think it is a problem because I only return the 'retSampleIdx'    2/4/10
			}
		}   //      m_reSamplerStreaming.m_sampleExpansionRatio  !=  1.0  
		else
		{
			retSampleIdx  =    exFilePos  /   bytesPerSample;  

			if(    ( exFilePos  %   bytesPerSample )   !=   0   )
			{
				int   dummy  =   9;
			}
		}

	}


	return    retSampleIdx;
}




					/////////////////////////////////////////////


bool	 WavConvert::Initialize_for_Streaming(   bool  isMP3file,      long  numberNeededToMatch,     CString&   retErrorMesg  )
{


			//  *****  ONLY CALLED by   BitSourceStreaming::Initialize()



	        //    m_fftSlowDown.m_slowDownAlgo   must be assigned  before this,  from setting in  PitchPlayerApp


	double      playSpeed  =   1.0;   //    *****  ALWAYS whenever this funnction is called ???     12/2011




	long   defaultSamplingRate  =    m_defaultSamplingFreqOutput;    //    44100;  

	long   srcBuffersByteCount  =    STREAMINGmp3BUFFERSIZE;     //   4416



	retErrorMesg.Empty();
	m_doResamplingStreaming =  false;
	m_isInitializedStreaming =   false;

	int      layer= -1,   lsf= -1,   freq= -1,  stereo = -1,  rate = -1;
	long   bitsPerSample =  -1,  blockAlaignment = -1;


	if(   isMP3file   )
	{

		if(   m_srcMP3FilePath.IsEmpty()   )
		{
			retErrorMesg =   "WavConvert::Initialize_for_Streaming failed,   m_srcMP3FilePath is empty."  ;
			return  false;
		}


		char  *filename =    m_srcMP3FilePath.GetBuffer( 0 );  

		try   
		{  
			if(   ! m_fileUniStreaming.open(   m_srcMP3FilePath,   0  )     )
			{
				retErrorMesg =   "WavConvert::Initialize_for_Streaming failed,   could not open m_fileUniStreaming."  ;
				return  false;
			}


			if(    ! Get_MP3_Version_Set_Offset(   m_fileUniStreaming,  retErrorMesg  )     )  //  sets the virtual FileOffset for later reads of m_fileUniStreaming
			{
				m_fileUniStreaming.close();   
				return  false;			
			}


			if(   ! Mp3Reader::getheader(  m_fileUniStreaming,   layer,  lsf,  freq,  stereo,  rate  )      )
			{
				retErrorMesg =   "Could not read the MP3 header. \nTry loading a different music file.  \n\n[ WavConvert::Initialize_for_Streaming ]"  ;
				m_fileUniStreaming.close();   
				return  false;			
			}



			bitsPerSample =   16;   // ******  Always????  ( this is BITS,  not bytes ).  So  16 bit, even for mono

			// ***** Do I have to worry that the MP3  Reader will generate 8bit samples,  and not always  16-bit samples ????


			m_bitsPerSampleSRCstream =    bitsPerSample;      //   ***CAREFULL***    is  'BITS',  not 'bytes'



			if(   stereo  )
				m_bytesPerSampleSRCstream =   4;
			else
				m_bytesPerSampleSRCstream =   2;   //  the 2 SRC bytes are duplicate to 4   *************  PROBLEM ??? *************




// *************   INSTALL,  will be tricky   8/3/2012   ( this member var is NOT yet used    ********************************

			m_totalSourceSamples  =   -1;        //   (long)(  m_wavFormat->cbSize  )   /    m_bytesPerSampleSRCstream;  // *** OK ???   is NEW    




	//		TRACE(  "\n\n   layer %i,   %i Hz,    %s,     %i kbps   \n\n",      layer+1,   freq,    stereo?"stereo":"MONO",   rate/1000   );

		}
		catch(   CFileException   *pException   )
		{
			TCHAR    szCause[ 255 ];  
			CString   strRaw;
			pException->GetErrorMessage(  szCause,  255  );
			pException->Delete();
			strRaw =  szCause;
			retErrorMesg.Format(   "WavConvert::Initialize_for_Streaming  failed,  could not read MP3 file because:  %s." ,   strRaw  );
			return  false;
		}

	}  //   if(   isMP3file

	else												
	{  bool   fileAcessError;					 //  is a WAV file  

		if(   m_srcWavFilePath.IsEmpty()   )
		{
			retErrorMesg =   "WavConvert::Initialize_for_Streaming failed,   m_srcWavFilePath is empty."  ;
			return  false;
		}


		if(    ! Initialize_for_WAV_Streaming(   m_srcWavFilePath,    fileAcessError,     retErrorMesg   )     )   //  initializes  m_wavFormat
			return  false;



		if(   m_wavFormat   !=  NULL   )
		{
			freq                  =    m_wavFormat->nSamplesPerSec;
			bitsPerSample   =     m_wavFormat->wBitsPerSample;     //   16   or   8   
			blockAlaignment =    m_wavFormat->nBlockAlign;           //     4:  16bit stereo      2:  8bit stereo


			if(   m_wavFormat->nChannels  ==   2   )
				stereo =  1;
			else
				stereo =  0;


			m_bitsPerSampleSRCstream =   bitsPerSample;



			if(        bitsPerSample  ==   16   )
			{
				if(  stereo  )
					m_bytesPerSampleSRCstream =  4;    // 4 bytesPer SOURCE sample
				else
					m_bytesPerSampleSRCstream =  2;    
			}
			else if(   bitsPerSample  ==   8   )    
			{
				if(  stereo  )
					m_bytesPerSampleSRCstream =  2;    
				else
					m_bytesPerSampleSRCstream =  1;    
			}
			else
			{	retErrorMesg =   "This file can not be loaded. \nPick another music file. [ Initialize_for_Streaming, bad Bits Per Sample ]."  ;
				return  false;			
			}




//			m_totalSourceSamples  =     (long)(  m_wavFormat->cbSize  )   /    m_bytesPerSampleSRCstream;     BAD,  cbSize =  0   

			m_totalSourceSamples  =     m_totalSamplesBytesWAVsrc   /    m_bytesPerSampleSRCstream;   //   NEW,  OK ???  


			
  
		}
		else
		{	ASSERT( 0 );  }

	}  //  is a WAV file  


	

	if(   stereo  !=   1  )
		 m_stereoFlagSRCstream =   false;
	else
		m_stereoFlagSRCstream  =   true;





	if(   freq  >   m_defaultSamplingFreqOutput   )   //  is this a VIABLE  problem?????   might have to change code in ReSampler, etc if this happens. But it might work.  2/10
	{
		ASSERT( 0 );
	
//		retErrorMesg.Format(   "WavConvert::Initialize_for_Streaming  failed,  could not read MP3 file because sampling rate was TOO HIGH[  %d  ]" ,   freq  );
//		return  false;
	}




	long   retOutputSampleCount =   -1;


	if(    freq  !=   defaultSamplingRate 	)     
	{

		TRACE(  "Needs  RESAMPLING...   \n\n"   );


		if(    ! Set_ReSample_Rates(   freq,    defaultSamplingRate,    retErrorMesg   )    )
		{
			ASSERT( 0 );
			return  false;
		}
	

		if(    ! m_reSamplerStreaming.Initialize(   freq,   defaultSamplingRate,   srcBuffersByteCount,  retOutputSampleCount, 
																					m_bytesPerSampleSRCstream,  m_stereoFlagSRCstream,  retErrorMesg  )     )
		{				
			return  false;    // ******  INSTALL better   cleanup ??? *****************************
		}


		if(     ! Alloc_ReSampling_Arrays(   retOutputSampleCount,   retErrorMesg   )     )
			return  false;


		m_doResamplingStreaming =   true;
	}
	else         
	{				                       //   'freq'  of sample is at   44,100   HZ			
		if(  isMP3file   )
		{
			if(   m_stereoFlagSRCstream  )
				retOutputSampleCount =            STREAMINGmp3BUFFERSIZE  /  MP3rEADERbYTESpERsAMPLE;    //   4416 / 4  =  1104
			else
				retOutputSampleCount =     2 *  (STREAMINGmp3BUFFERSIZE  /  MP3rEADERbYTESpERsAMPLE);  // mono signal must be EXPANDED in bytes for STEREO output buffer
		}
		else
			retOutputSampleCount =     STREAMINGmp3BUFFERSIZE  /  m_bytesPerSampleSRCstream;		// looks OK



		Release_ReSampling_Arrays();

		m_doResamplingStreaming =   false;
	}




	m_outputBufferSampleCountNotSlowDown        =      retOutputSampleCount;    //  need this to persist for later calls to  Alloc_OutputBuffer() 

	m_fftSlowDown.m_outputSampleCountNotSlow  =     retOutputSampleCount;    //  do I need to know   Bytes Per Sample  ??





	if(   isMP3file   )
	{

		int  chn   =   0;
		int  down =  0;      //   1:  Cuts sample rate in half

		if(    ! m_mp3DecoderStreaming.open(   m_fileUniStreaming,   freq,   stereo,   1,   down,   chn  )     )
		{
			retErrorMesg =   "WavConvert::Initialize_for_Streaming  failed,  could not open decoder." ;
			m_fileUniStreaming.close();   
			return  false;  
		}
		


		float   bal= 0;    //  do I need these to PERSIST to  fetch bytes  in the middle of file ????   ****************
		float   ctr= 0;
		float   sep= 1;
		float   srnd= 1;
		float   vol= 1;
		float   vols[3][3];

			vols[0][0] =	0.5*(1.0-bal)*(1.0-ctr+sep);
			vols[0][1] =	0.5*(1.0-bal)*(1.0+ctr-sep);
			vols[0][2] =	1-bal;
			vols[1][0] =	0.5*(1.0+bal)*(1.0-ctr-sep)*srnd;
			vols[1][1] =	0.5*(1.0+bal)*(1.0+ctr+sep)*srnd;
			vols[1][2] =	(1.0+bal)*srnd;
			vols[2][0] =	0.5*(1.0-ctr);
			vols[2][1] =	0.5*(1.0+ctr);
			vols[2][2] =	1.0;

		m_mp3DecoderStreaming.ioctl(   m_mp3DecoderStreaming.ioctlsetstereo,   vols,    0   );


		int  chvol  =  1;		

		if(  chvol  )
		{
			chvol  =  0;			//   goes here
			float vol0 = vol;
			m_mp3DecoderStreaming.ioctl(    m_mp3DecoderStreaming.ioctlsetvol,   &vol0,   0    );
		}


		int     chpitch =        1;
		float   pitch    =       0;
		float   pitch0  =   1764;

		if(    chpitch   )          //  *** Can I change pitch here for  frequecy change ????  *************** 
		{
			chpitch =  0;
			float   l3equal[ 576 ];

			for (  int  i= 0;   i< 576;   i++  )
				  l3equal[ i ]  =      exp(       pitch *      log(     (i+0.5) * freq   /  ( 576.0 * 2 * pitch0 )     )          );

			m_mp3DecoderStreaming.ioctl(   m_mp3DecoderStreaming.ioctlsetequal576,    l3equal,   0  );
		}

	}  //  if(   isMP3file





//   Initialize_for_Streaming()


	if(    ! Alloc_OutputBuffer(   m_outputBufferSampleCountNotSlowDown,    numberNeededToMatch,    retErrorMesg   )     )    //  final calulation of total  "bytes per fetch"  is calced here
	{
		if(   isMP3file   )
			m_fileUniStreaming.close(); 
		else
		{  ASSERT( 0 );   }  //  Need INSTALL file close for wav file ???		

		return  false;
	}
	else
	{	m_currentByteIndexInBufferStreaming =  0;

	   m_currentByteIndexInBufferStreamingNotSlowDown  =   0;
	}




	ASSERT(   m_fftSlowDown.m_playSpeed  ==   playSpeed   );


	if(     ! m_fftSlowDown.Set_SlowDown_Optimizations_this_Speed(  retErrorMesg   )    )    //   NEW,  4/21/12
		return  false;   // ***************************  Do I need this HERE???   It will get cCALLED again in  PitchPlayerApp::Initialize_BitSource()     *********************************


 
	m_isInitializedStreaming =   true;

	return  true;
}




					/////////////////////////////////////////////


bool   WavConvert::Initialize_for_WAV_Streaming(   CString&   filePath,    bool&  fileAcessError,    CString&   retErrorMesg   )
{

														//   WAS:   Register_WavFile_for_Streaming()
	retErrorMesg.Empty();
	fileAcessError =   false;


	if(    filePath.IsEmpty()    )
	{
		retErrorMesg =    "WavConvert::Initialize_for_WAV_Streaming  failed,   filePath is empty."  ;
		return  false;
	}


//	m_isInitialized  =  false;

	/****
	Release_BufferBits();   	
	Release_WaveFormatEx();
	Release_Media_File();
	****/


//		*******				Release_All_Resources();
	

//	Release_BufferBits();   	  ***** NEED this ????



	Release_WaveFormatEx(); 

	Release_Media_File();



	if(     ! Open_Get_WavFormatEx(   filePath,    retErrorMesg  )      )  
	{

		fileAcessError =   true;   //  the file could not be found or opened. 

		return  false;
	}



	/****
	long   bufferSize =     4096L;     //    SPEEDZONESIZE    4096;    

	if(     ! Alloc_LocalBuffer(   bufferSize  )     )
	{
		retErrorMesg =     "BitSource::Open_File_for_StreamingAnalysis  failed,  could not allocate local buffer." ;
        return  false;
    }
	*****/



// 	m_currentByteIdx =  0L;   ********  INSTALL,  have calling function do this ******


	return  true;
}



					/////////////////////////////////////////////


bool	   WavConvert::Alloc_OutputBuffer(   long   sampleCountNotSlowDown,    long  numberNeededToMatch,     CString&   retErrorMesg   )
{


		//   ONLY CALLED BY :    WavConvert::Initialize_for_Streaming(),   WavConvert::Change_PlaySpeed() 

 
	long   bytesPerSampleOutput =    MP3rEADERbYTESpERsAMPLE;    //  Always is 4  because this is the OUTPUT buffer ????


	short  appCode  =  Get_PitchScope_App_Code_GLB();         //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope



	if(    sampleCountNotSlowDown  <=  0    )
	{
		m_outputBufferSizeStreaming =  0;
		m_outputBufferSampleCount =  0;

		retErrorMesg =   "WavConvert::Alloc_OutputBuffer  failed,  could not alloc   outputBufferStreaming." ;
	//	m_fileUniStreaming.close();   
		return  false;  
	}


	ASSERT(  m_fftSlowDown.m_playSpeed  >=  1.0  );      // ****** MORE ERROR management ******      if Slowdown is too big for FFT static arrays   2/7/10



								//     the  SIZE of the output buffer(bytesToAllocate)   can vary if the ReSasmpler is invoked,  oif using play speed slow down

	long   bytesToAllocate            =  (long)(  m_fftSlowDown.m_playSpeed  *  (double)sampleCountNotSlowDown  *  (double)bytesPerSampleOutput   );

	long   bytesToAllocateNoSlow =                                                   sampleCountNotSlowDown  *  bytesPerSampleOutput;



	if(   m_outputBufferStreaming  !=  NULL  )    	//  get rid of previous buffer,  might be the wrong size
	{
		delete  m_outputBufferStreaming;	
		m_outputBufferStreaming =   NULL;
	}

	if(   m_outputBufferStreamingDelayed  !=  NULL  )    	//  get rid of previous buffer,  might be the wrong size
	{
		delete  m_outputBufferStreamingDelayed;	
		m_outputBufferStreamingDelayed =   NULL;
	}



	if(   m_outputBufferStreamingNoSlowDown  !=  NULL  )    	//  get rid of previous buffer,  might be the wrong size
	{
		delete  m_outputBufferStreamingNoSlowDown;	
		m_outputBufferStreamingNoSlowDown =   NULL;
	}



	if(   m_backwardsPlayBuffer  !=  NULL  )    	//  get rid of previous buffer,  might be the wrong size
	{
		delete  m_backwardsPlayBuffer;	
		m_backwardsPlayBuffer =   NULL;

		m_backwardsPlayBufferSize  =  0;
		m_backwardsPlayBufferBlockIndex =  0;
		m_maxNumberOfbackwardsPlayBufferBlocks =  0;
	}



					////////////////////////////////////////////
	

	m_outputBufferStreaming  =    new   char[  bytesToAllocate  ];   //  Do I want to PAD this up?????
	if(  m_outputBufferStreaming == NULL   )
	{
		m_outputBufferSizeStreaming =  0;

		m_outputBufferSampleCount =  0;

		retErrorMesg =   "WavConvert::Alloc_OutputBuffer  failed,  could not alloc   outputBufferStreaming." ;
//		m_fileUniStreaming.close();   
		return  false;  
	}
	else
	{  m_outputBufferSizeStreaming  =     bytesToAllocate;    

	    m_outputBufferSampleCount   =    (long)(    (double)sampleCountNotSlowDown   *   m_fftSlowDown.m_playSpeed    );

		m_currentByteIndexInBufferStreaming =  0;    //   OK to do this here  ???
	}




//   Alloc_OutputBuffer() 


								//  For Player.exe,  must also maintain a DELAY Buffer for AudioBytes  ( allows sync between Midi and Audio whne no FinalCircQue is present   


	if(        m_fftSlowDown.m_playSpeed  ==   1.0        //  So far,  the DELAY is only for Player.exe at Speed 1    7/2012
		&&   appCode  ==   0    )      //   0:  Player
	{						                      //   Do NOT restrict this to only Backward Play, because this is at song load, and we do NOT know direction yet.


		if(     ! ReAllocate_Players_AudioDelay_MemBuffer(   sampleCountNotSlowDown,   numberNeededToMatch   )    )   
		{
			retErrorMesg =   "WavConvert::Alloc_OutputBuffer  failed,  ReAllocate_Players_AudioDelay_MemBuffer." ; 
			return  false;  
		}
	}






	m_outputBufferStreamingNoSlowDown  =    new   char[  bytesToAllocateNoSlow  ];   //  Do I want to PAD this up?????
	if(  m_outputBufferStreamingNoSlowDown == NULL   )
	{
		retErrorMesg =   "WavConvert::Alloc_OutputBuffer  failed,  could not alloc   m_outputBufferStreamingNoSlowDown." ;
//		m_fileUniStreaming.close();   
		return  false;  
	}
	else
	{  //m_outputBufferSizeStreaming   =   bytesToAllocate;    
	    //m_outputBufferSampleCount   =      sampleCount;    This is the same ????
		//m_currentByteIndexInBufferStreaming =  0;    //   OK to do this here  ???

		ASSERT(   sampleCountNotSlowDown  ==   m_outputBufferSampleCountNotSlowDown   );

		m_currentByteIndexInBufferStreamingNotSlowDown =  0;
	}





	long   bufferSize        =     BLOCKLOCKEDSIZE;
	long   subBufferSize  =     bytesToAllocateNoSlow;

	ASSERT(  subBufferSize   ==   ( m_outputBufferSampleCountNotSlowDown  *  MP3rEADERbYTESpERsAMPLE  )       );



	m_maxNumberOfbackwardsPlayBufferBlocks  =     BLOCKLOCKEDSIZE  /  subBufferSize;

	long   remainder      =                      (long)BLOCKLOCKEDSIZE    %    subBufferSize;
	if(      remainder  !=   0   )
	{								   
		//   *****  need these to divide evenly, so must use' old' BACKWARDS PLAY  algo.   12/11   ******** 
		
		ASSERT(   m_backwardsPlayBuffer ==   NULL   );    //  File will not be able to play BACKWARDS with new method.   12/22/2011

		m_backwardsPlayBufferSize  =  0;
		m_backwardsPlayBufferBlockIndex =  0;
		m_maxNumberOfbackwardsPlayBufferBlocks =  0;


		TRACE(   "\n\n\n  ****** Could NOT USE new BACKWARDS Buffer.  [ subBufferSize =  %d  ]  ********  \n\n\n ",     subBufferSize  );
	}
	else
	{  m_backwardsPlayBuffer      =        new    BYTE[   bufferSize   ];

		if(   m_backwardsPlayBuffer  ==   NULL   )
		{
			m_backwardsPlayBufferSize  =  0;
			m_backwardsPlayBufferBlockIndex =  0;
			m_maxNumberOfbackwardsPlayBufferBlocks =  0;

			retErrorMesg =   "WavConvert::Alloc_OutputBuffer FAILED,  could not alloc m_backwardsPlayBuffer."  ;
			return  false;
		}
		else
		{	m_backwardsPlayBufferSize           =    BLOCKLOCKEDSIZE;  
			m_backwardsPlayBufferBlockIndex =  0;
			m_numberOfTenBlocksLoaded         =  0;
		}
	}

	return  true;
}



					/////////////////////////////////////////////


bool    WavConvert::ReAllocate_Players_AudioDelay_MemBuffer(   long  sampleCountNotSlowDown,    long  numberNeededToMatch   )
{


		//  CALLED BY:    PitchPlayerDlg::Set_SPitchCalcs_Detail_Threshold(),     WavConvert::Alloc_OutputBuffer(),   
		//
		//	                      PitchPlayerDlg::On_NMReleasedCapture_MidiSync_Slider(),        PitchPlayerDlg::Change_Midi_Source()



		//    If  (  sampleCountNotSlowDown   < 0  )   then the buffer is deleted.

		//    Also if   baseDelay ==  -1     then the buffer is deleted.


	
	long    bytesPerSampleOutput =    MP3rEADERbYTESpERsAMPLE;    //  Always is 4  because this is the OUTPUT buffer 


	short         appCode  =     Get_PitchScope_App_Code_GLB();         //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope
	ASSERT(   appCode  ==  0   );


	ASSERT(  m_fftSlowDown.m_playSpeed  ==   1.0   );


	long   baseDelay      =    Get_Base_AudioDelay_inPieSlices();    

	long   overideDelay  =    Get_Overide_AudioDelay_inPieSlices();



	long   newDelay =   -1;
	long   ratioOfBlockSize  =    -1;
	bool   supressMemCopy =  false;

	long   bytesInDelayOld  =    m_outputBufferSizeStreamingDelayed;
//	long   oldDelay =      m_delayInPieSlices;


	bool   bufferHadData =   false;
	
	if(     m_outputBufferStreamingDelayed   !=   NULL    )    
		 bufferHadData =   true;



																				//    NO WAVdelay for  NoteList-Play
	if(     m_playModeUsingNotelist  ==   1   )    
		 sampleCountNotSlowDown  =  -1;    //   will delete the buffer, like on a speed change




	if(      (  overideDelay  >=   0      &&    ! m_playModeUsingNotelist    )   		
		  ||    baseDelay ==  -1     )
	{

		if(      overideDelay  ==   0   
			||   baseDelay ==  -1     )
		{
			supressMemCopy =   true;      //  Causes the BUFFER  to be completely DELETED.     Do NOT want a WAV delay for a Notelist Mode.   8/12
		    
			ratioOfBlockSize =  -1;

			newDelay =  0;
		}
		else
		{  newDelay           =   overideDelay;    //  do NOT add in  baseDelay  

			ratioOfBlockSize =   1;    //   **********   OK ??????
		}	
	}
	else
	{						 //  m_overideDelay < 0      ... NORMAL  mode  


		if(         sampleCountNotSlowDown  ==   1104   )    //  1104:   Standard size  BLOCK  for   most  MP3   and  WAV at   44,100 hz
		{

			ratioOfBlockSize =  1;    //  ideal,  do nothing     

			newDelay =    baseDelay  +   numberNeededToMatch;
		}
		else if(   sampleCountNotSlowDown   >   1104   )
		{

			ratioOfBlockSize =    sampleCountNotSlowDown  /  1104;    // Since the oversized Blocks have more than on PieSlice,  we must REDUCE by that factor.  7/2012


			if(   ratioOfBlockSize  > 0  )
				newDelay =    ( baseDelay  +   numberNeededToMatch  )    /  ratioOfBlockSize ;   // **** ????  Am I sure ???     7/27/2012  *********
			else
				newDelay =   0;


			if(     newDelay  ==  1   )      //   *****  1:    is this the same as a  0  delay??    THINK YES !!!!     7/27/2012
			{
				 newDelay =  0;    
			}
			else if(   newDelay  ==  0   )
			{  int   dummy =   9;  }

		}
		else if(    sampleCountNotSlowDown  < 0    )
		{	
			supressMemCopy =   true;      //  Causes the BUFFER  to be completely DELETED.  Do NOT want a WAV delay for a Notelist Mode.   8/12
		    
			ratioOfBlockSize =  -1;

			newDelay =  0;
		}
		else
		{	ASSERT(  0  );    //  not prepared for this
			supressMemCopy =   true;	    
			ratioOfBlockSize =  -1;
			newDelay =    baseDelay  +   numberNeededToMatch;
			ASSERT(   newDelay  >=  1   );		
		}

	}   //  m_overideDelay < 0      ...Normal mode  




	/////////////////////////////////////////////////////////////////////////////////////////////////////
											
														//   Might NOT NEED a delay 
	if(   newDelay  <=  0   )
	{

		m_delayInPieSlices =    0;    //                                                    land here for   Clarinet_16hz_mono.wav

//		m_baseDelay =  1;           ** NO,   want to preserve this user setting,  In case they switch the MIDI-SOURCE Mode  back to Detection

//     m_overideDelay =  -1;      ** NO,   want to preserve this user setting,  In case they switch the MIDI-SOURCE Mode  back to Detection 



		if(   m_outputBufferStreamingDelayed  !=  NULL   )
		{
			delete  m_outputBufferStreamingDelayed;	 //  Now delete OLD memory...  we will NOT have a delay	
		}

		m_outputBufferStreamingDelayed       =  NULL;
		m_outputBufferSizeStreamingDelayed =      0;

		m_indexIntoBufferStreamingDelayed =   0;

		return  true;   // **********   OK ??? *******************
	}




//	long   bytesToAllocateNew   =      (long)(   (double)sampleCountNotSlowDown  *  (double)bytesPerSampleOutput   *   m_fftSlowDown.m_playSpeed  *   );		
	long   bytesToAllocateNew   =      (long)(   (double)sampleCountNotSlowDown  *  (double)bytesPerSampleOutput   );

	long   bytesInDelayNew  =      newDelay   *    bytesToAllocateNew;   //   'bytesToAllocate'   gets used in algo instead of   "Bytes in PieSlice"     7/27/2012






	char*  nuBufferMemory  =    new    char[  bytesInDelayNew  ];  
	if(       nuBufferMemory  ==   NULL    )
	{
		ASSERT( 0 );
	//	retErrorMesg =   "WavConvert::Alloc_OutputBuffer  failed,  could not alloc   m_outputBufferStreamingDelayed." ; 
		return  false;  
	}
	else
	{  m_delayInPieSlices                            =    newDelay;

	    m_outputBufferSizeStreamingDelayed =    bytesInDelayNew;



		if(    bufferHadData    )
		{

			ASSERT(  m_outputBufferStreamingDelayed  !=  NULL  );

			memset(  nuBufferMemory,    0,    bytesInDelayNew   );    //  clear out the audio bytes, and stop the clicking sounds at play start    7/2012



			if(       ! supressMemCopy
			    &&    bytesInDelayOld   > 0
				&&    bytesInDelayNew  > 0    )		
			{
																//  Copy in the previous memory's data,  to avoid a SILENCE when user moves the Detail Slider
				if(   bytesInDelayNew  >=   bytesInDelayOld    )       
					memmove(    nuBufferMemory,   m_outputBufferStreamingDelayed,      bytesInDelayOld   );
				else
					memmove(    nuBufferMemory,   m_outputBufferStreamingDelayed,      bytesInDelayNew   );
			}
						


			delete  m_outputBufferStreamingDelayed;	                 //  Now delete OLD memory,  and ASSIGN the NEW mem block
			m_outputBufferStreamingDelayed  =   nuBufferMemory;



			if(     m_indexIntoBufferStreamingDelayed   >=   newDelay    )
				m_indexIntoBufferStreamingDelayed  =    newDelay  -1;     //   Good way to approximate this value.

			ASSERT(   m_indexIntoBufferStreamingDelayed  >=  0   );
		}
		else
		{  
			m_outputBufferStreamingDelayed =    nuBufferMemory;    //   buffer was NULL,  did not exist yet,  so just assign to membervar

			memset(  m_outputBufferStreamingDelayed,    0,    bytesInDelayNew   );    //  clear out the audio bytes, and stop the clicking sounds at play start    7/2012

			m_indexIntoBufferStreamingDelayed =   0;    //  INIT for the load of a new song
		}
	}


	return  true;
}



					/////////////////////////////////////////////


void   WavConvert::Erase_Players_AudioDelay_MemBuffer()
{

			//  should call this for any OP that changes the file position


	if(   m_outputBufferSizeStreamingDelayed   <=  0    )
		return;      // No Buffer to erase


	if(   m_outputBufferStreamingDelayed  ==   NULL   )
	{
		ASSERT( 0 );    // ****** ERROR ******
		return;
	}



	long   bytesInDelay  =     m_outputBufferSizeStreamingDelayed;


	memset(  m_outputBufferStreamingDelayed,    0,    bytesInDelay   );    //  clear out the audio bytes, and stop the clicking sounds at play start    7/2012
}




					/////////////////////////////////////////////


void   WavConvert::Set_Computer_Performance_Values(    long  computerPerformanceFactor   )
{


	CString  retErrorMesg;


	m_fftSlowDown.m_computerPerformanceFactor  =     computerPerformanceFactor;    //   'FFTslowDown'  is where  PERFORMANCE will VARY


	if(    ! m_fftSlowDown.Set_SlowDown_Optimizations_this_Speed(   retErrorMesg   )    )
	{
		ASSERT( 0 );	   //  will cause the new Song File to be intialized to whatever   SlowDown 'computerPerformanceFactor'   that is stored in  SoundHelper
		return;
	}
}




					/////////////////////////////////////////////


bool	   WavConvert::Change_PlaySpeed(    double  newSlowDownRatio,    long  computerPerformanceFactor,    long  numberNeededToMatch,   CString&  retErrorMesg  )
{

							//   TRICKY,  will have to change the size of buffers,  maybe redo the initialization    2/7/10


					//	ASSERT(  m_outputBufferSampleCount  > 0  );   NO,  this can be called if theire is NOT yet an output buffer


	m_fftSlowDown.m_playSpeed  =     newSlowDownRatio;    //   Alloc_OutputBuffer() will need this new assignment to do its thing.


	if(   m_outputBufferSampleCount   > 0   )
	{

		ASSERT(    m_outputBufferSampleCountNotSlowDown  >  0    );   //  must have been previously assigned


		if(    ! Alloc_OutputBuffer(   m_outputBufferSampleCountNotSlowDown,   numberNeededToMatch,    retErrorMesg  )     )   //  should recalc a lot of important variables
		{
			m_fileUniStreaming.close(); 
			return  false;
		}
	}



	if(     ! m_fftSlowDown.Process_SpeedChange(   newSlowDownRatio,    computerPerformanceFactor,   retErrorMesg  )      )   // ************** BIG **************
		return  false;


	return  true;
}



					/////////////////////////////////////////////


bool	 WavConvert::Fetch_Current_Sample_and_Increment(   bool&   retEndOfFile,    long  currentByteIdx,    
														                                    char&  retChVal0,     char&  retChVal1,    char&  retChVal2,     char&  retChVal3,           
																	bool  backwardsFlag,	  SndSample  *sndSample,    long  sampIdxBlock,  short  rightChannelPercent, 
																	           bool&  retDidBlockLoad,    bool  isPlayingNoteList,    CString&   retErrorMesg   )
{

										  //   always returns  4byte samples  (16bit stereo )
	retErrorMesg.Empty();

	retEndOfFile =  false;
	retDidBlockLoad =  false;
	retChVal0 = retChVal1  =  retChVal2  =   retChVal3 =   -1;

	
	ASSERT(   m_fftSlowDown.m_playSpeed   >  0.0    );


	short  appCode  =  Get_PitchScope_App_Code_GLB();         //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope




	bool   isAmp3 =   Is_A_MP3_File_Loaded();


	if(      m_outputBufferStreaming  ==  NULL
		||  m_outputBufferSizeStreaming <=  0    )
	{
		retErrorMesg =  "WavConvert::Fetch_Current_Sample_and_Increment  FAILED,  m_outputBufferStreaming was not initialized." ;
		return  false;
	}

	ASSERT(  sndSample !=  NULL  );


	/******************  Creates problems for Backwards play.  If user hits FileEnd Button,  the get part of  10 subBlocks.  12/21/11
	if(         isAmp3
		 &&   m_mp3DecoderStreaming.eof()   )         Better to let  Load_Next_MemoryBlock_Frame
	{
		retEndOfFile =    true;    //  no error,  just hit the end of file
		return   false;
	}
	****/



	bool    bufferNeedsFilling =   false;


	if(    currentByteIdx ==  0   )
		bufferNeedsFilling =   true;      


	if(    m_currentByteIndexInBufferStreaming   >=   m_outputBufferSizeStreaming    )     //  for Slow2,    m_outputBufferSizeStreaming  =  8832
		bufferNeedsFilling =   true;                       //    'm_outputBufferSizeStreaming'    is the  SlowedDown Samples  buffer





	long   retBytesRead =   0; 
	bool   willHaveAReadOutOfBounds =   false;



	if(    bufferNeedsFilling   )  
	{



		if(     ! Load_Next_MemoryBlock_Frame(    retEndOfFile,     retBytesRead,   backwardsFlag,   retErrorMesg   )     )  
		{
			return  false;    
		}

		retDidBlockLoad =   true;   // *** ERROR handling may be finally fixed   1/2012    ***

		//		When  Load_Next_MemoryBlock_Frame()  returns, it will have filled 2 buffers:
		//			a)   	'm_outputBufferStreaming'                      ...Will hold the slowedDown samples if slowSpeed ( currently do NOT use these for detection, missing harmonics  3/11  )
	    //			b)     'm_outputBufferStreamingNoSlowDown'   ...Will hold the normal SRC-samples (use these for detection at slowed down speed


		if(   retEndOfFile  ==   true   )
		{
			ASSERT( 0 );    //  should NOT reach this,  because of  above 'return'  for   Load_Next_MemoryBlock_Frame()    1/12
		}
//		TRACE(   "Just Loaded Block:     currentByteIdx [ %d ]  \n",       currentByteIdx    );  
			
	


													//   NEW,   Apply a slight DELAY to the AudioBytes,  so we can SYNC Player's  Midi and WAV play.    7/2012


		if(         m_fftSlowDown.m_playSpeed  ==   1.0       //  So far,  the DELAY is only for Player.exe at Speed 1 forward.     7/2012
			&&    appCode  ==   0     //   0:  Player
			&&  ! backwardsFlag   
			&&    m_delayInPieSlices  >=  2   )       //   *** OK ???    Actually,  1 is the same as 0.   Should not really ever see a value for 1.
		{

			long    bytesInSample  =   4;
			long	  sampsInPieSliceSpeedOne  =		Calc_Samples_In_PieEvent(  1.0  );    //   1104


			/***
			long    blocksSize  =    sampsInPieSliceSpeedOne   *   bytesInSample;   // ***** BAD, might be bigger **************************************
			ASSERT(  m_outputBufferSampleCount  ==   sampsInPieSliceSpeedOne   );   // ***  FAILS for  GreySolo_22hz.wav
			***/
			long    blocksSize  =    m_outputBufferSampleCount    *   bytesInSample; 



			char  *oldestBlockPtr  =		Get_Oldest_Block_from_AudioDelays_CircularQue();	
			char  *srcBytes          =      m_outputBufferStreaming;

			memmove(    oldestBlockPtr,   srcBytes,      blocksSize   );



			m_indexIntoBufferStreamingDelayed++;     //   INCREMENT the Index,  to show the new state of the CircQue after a DataLoad

			if(    m_indexIntoBufferStreamingDelayed   >=   m_delayInPieSlices   )
				m_indexIntoBufferStreamingDelayed =   0;
		}




		m_currentByteIndexInBufferStreaming =  0;        //   re-init  the traversing pointer to the buffer  

		m_currentByteIndexInBufferStreamingNotSlowDown =   0;
	}
	else
	{	
		if(     ( m_currentByteIndexInBufferStreaming  +3)   >=  m_outputBufferSizeStreaming      )    //  +3  because we will read 3 beyond, below.
		{
			willHaveAReadOutOfBounds =   true;
			int  dummy =  9;     //  ****   gets hit by   Clarinet_16hz_mono.wav  ( does NOT crash, just sounds funky.    1/28/12  *********************************
		}
	}
	



	if(   ! willHaveAReadOutOfBounds   )    //  Want this test ????
	{

							//  If using Player.exe,   then we can read from the DELAYED BUFFER to lag the Audio against the Midi Play    7/2012


		if(         m_fftSlowDown.m_playSpeed  ==   1.0     //  So far,  the DELAY is only for Player.exe at Speed 1   *****   7/2012
			&&    appCode  ==   0     //   0:  Player
			&&  ! backwardsFlag 			
			&&    m_delayInPieSlices  >=  2    
			
			&&   ! isPlayingNoteList
			
			)       //   *** OK ???    Actually,  1 is the same as 0.   Should not really ever see a value for 1.
		{

			char  *oldestBlockPtr  =	   Get_Oldest_Block_from_AudioDelays_CircularQue();	

			retChVal0  =   *(     oldestBlockPtr					+   ( m_currentByteIndexInBufferStreaming      )     );     
			retChVal1  =   *(     oldestBlockPtr					+   ( m_currentByteIndexInBufferStreaming  +1 )     );
			retChVal2  =   *(     oldestBlockPtr					+   ( m_currentByteIndexInBufferStreaming  +2 )     );
			retChVal3  =   *(     oldestBlockPtr					+   ( m_currentByteIndexInBufferStreaming  +3 )     );
		}
		else            //   Normal use,  and for Navigator
		{
			retChVal0  =   *(     m_outputBufferStreaming   +   ( m_currentByteIndexInBufferStreaming      )     );     //  NOT Reversed during backwards play
			retChVal1  =   *(     m_outputBufferStreaming   +   ( m_currentByteIndexInBufferStreaming  +1 )     );
			retChVal2  =   *(     m_outputBufferStreaming   +   ( m_currentByteIndexInBufferStreaming  +2 )     );
			retChVal3  =   *(     m_outputBufferStreaming   +   ( m_currentByteIndexInBufferStreaming  +3 )     );
		}
	}




														//   B)    Must also get bytes for the  "NOT SlowedDown Buffer".   It is shorter than the m_outputBufferStreaming 


	double   speedFlt  =   m_fftSlowDown.m_playSpeed;

	long      currentSampleIndex  =     m_currentByteIndexInBufferStreaming  /  4;    //  4 always in output


	/****   TESTED and seems OK.  3/3/10
	if(    (  currentSampleIndex  %  m_fftSlowDown.m_playspeedSlowDownRatio  )  ==  0   )    //  this is when  chSlowVal's should be fetched and assigned to SndSample  in   Fetch_Streaming_Samples_Direct_PPlayer()
	{
		int   dummy  =  9;
	}
	*****/



// ****************************   BAD  for  1.5  speed ??  Still seems accurate for 1.5 (see test below  1/26/12 )   ******************

	long   currentSampleIndexSlowDown   =    (long)(    (double)currentSampleIndex  /  speedFlt   );

	long   currentByteIndexNotSlowDown  =     currentSampleIndexSlowDown  *   4;       //  going from sample to bytes

// ******************************************************************************************************************



												            //  add  signal's BYTES  to the  'SndSample'  for DETECTION.    They are NOT slowed-down.
	if(   sampIdxBlock ==  0   )
	{
		sndSample->Erase();
		m_indexIntoSndSampleNotSlow =   0;
	}


	long       lengthSndSample         =     sndSample->Get_Length(); 
	double   leftChannelPercentFlt    =     (  100.0 - (double)rightChannelPercent  )      / 100.0;                
	double   rightChannelPercentFlt  =                   (double)rightChannelPercent          / 100.0;
	long      slowedDownSpeedINT   =     (long)speedFlt;   

		

								//   For slowedDown speeds need to ADD a SAMPLE at  "every-other" iteration of  'sampIdx'  because the SlowedDown 
								//	 signal will have  "Speed times (ie 2x)"  as many samples as the oringal SRC-sample count   3/11

	long   samplesToAdd     =    1; 
	bool   addToSndSample =    false;


	if(   speedFlt  ==  1.5  )
	{
		 if(   (  sampIdxBlock %  3 )  ==  0    )    //    0  or 2  any difference  
		 {
			addToSndSample =    true;
			samplesToAdd     =    2; 
		 }
	}
	else
	{	 if(   (  sampIdxBlock %  slowedDownSpeedINT )  ==  0    )   //  If we slowedDown by factor 2,  then we would only fetch from every other AUDIBLE-sample, etc.
		 {
			 addToSndSample =    true;
			 samplesToAdd     =    1; 
		 }
	}




	if(    addToSndSample   )
	{
														 

		if(    currentByteIndexNotSlowDown    !=   m_currentByteIndexInBufferStreamingNotSlowDown   )
		{
			int  dummy =  9;       // ********** Does this EVER get hit ????  1/26/2012         YES,  1/26 ********************
	//		ASSERT( 0 );
		}



		for(    long i= 0;    i < samplesToAdd;     i++   )   
		{

			char   chNotSlowVal1 =  0,   chNotSlowVal3 = 0;     


			if(      ( m_currentByteIndexInBufferStreamingNotSlowDown  +3 )   >=    ( m_outputBufferSampleCountNotSlowDown * 4 )    )
			{
				int  dummy =  9;
			}
			else
			{//chNotSlowVal0  =      *(     m_outputBufferStreamingNoSlowDown   +   ( m_currentByteIndexInBufferStreamingNotSlowDown       )     );  
				chNotSlowVal1  =      *(     m_outputBufferStreamingNoSlowDown   +   ( m_currentByteIndexInBufferStreamingNotSlowDown  +1 )     );

			//	chNotSlowVal2  =      *(     m_outputBufferStreamingNoSlowDown   +   ( m_currentByteIndexInBufferStreamingNotSlowDown  +2 )     );
				chNotSlowVal3  =      *(     m_outputBufferStreamingNoSlowDown   +   ( m_currentByteIndexInBufferStreamingNotSlowDown  +3 )     );
			}



																					//	  Create a MONO signal,  by averaging left and right    
			double   leftVal    =    chNotSlowVal1;    
			double   rightVal  =    chNotSlowVal3;     

											                    //   Do not think I need to BoostVolume like below because I am ADDING the two values and have reinforcement.   3/10

			short   combinedVal =    (short)(	     ( leftVal  *  leftChannelPercentFlt )    +    ( rightVal  *  rightChannelPercentFlt )	    );
														
			if(         combinedVal   >   127  )       //  range of   CHAR  is  { -128  to  127  }
				combinedVal =    127;
			else if(   combinedVal   <  -127  )
				combinedVal =   -127;           //  we have hit this ...is it a problem??  1/17/10




			ASSERT(   m_indexIntoSndSampleNotSlow   <   lengthSndSample   );


			char  *destSndSample =      sndSample->Get_StartByte()    +    m_indexIntoSndSampleNotSlow;  
			*destSndSample         =      (char)combinedVal;


			m_indexIntoSndSampleNotSlow++;

			m_currentByteIndexInBufferStreamingNotSlowDown  +=   4;     //   'increment'  for  NEXT write to the SndSample ( NOT Slowed )  


			if(     m_indexIntoSndSampleNotSlow  >  m_biggestIndexToSndSample    )
				m_biggestIndexToSndSample =   m_indexIntoSndSampleNotSlow;    //  ***TEMP,  just to check calcs			
		}			
	}



	m_currentByteIndexInBufferStreaming  +=   4;     // 'increment'  for  NEXT call  of this funct


	return  true;
}




					/////////////////////////////////////////////


bool	 WavConvert::Load_Ten_MemoryBlock_Frames(   bool&   retEndOfFile,    long&  retBytesRead,    long  byteCountToRead,   CString&   retErrorMesg   )
{


	retBytesRead =  0;
	retErrorMesg.Empty();


	ASSERT(  m_backwardsPlayBuffer  );


	long   sourceBuffersSampleCount  =     STREAMINGmp3BUFFERSIZE  /   m_bytesPerSampleSRCstream;     //  m_bytesPerSampleSRCstream:  2  or 4



//	long   blockSize  =     sourceBuffersSampleCount  *    4;    //   NO ???? m_bytesPerSampleSRCstream	
//	long   blockSize  =    m_outputBufferSizeStreaming;    **** NO, fails at slow speed  
	long   blockSize  =    m_outputBufferSampleCountNotSlowDown  *  MP3rEADERbYTESpERsAMPLE;



	long   numberOfBlocks  =    byteCountToRead   /   blockSize;    //  10   , but NOT always


	ASSERT(   numberOfBlocks   ==   m_maxNumberOfbackwardsPlayBufferBlocks  );     //  10    ,  but not always




	bool   isAmp3 =   Is_A_MP3_File_Loaded();


	bool   keepGoing =  true,     retEndOfFileLoc =  false,    gotBlock =  true;

	long   count =  0,   retOutputByteCount,   retBytesReadLoc=0;
	


	m_backwardsPlayBufferBlockIndex =   0;   // reinit for playback


	char*   destBlockPtr  =    (char*)m_backwardsPlayBuffer;   //   init  to the start of the BIG buffer ( 44160 )




	while(        keepGoing   
			  &&   count  <  numberOfBlocks   )
	{

		if(   isAmp3  )
		{                         
												               //  when Load_Next_MP3_Frame()  is done, it will ALWAYS leave a   16bit STEREO   sample in  m_outputBufferStreaming

			if(    ! Load_Next_MP3_Frame(    retEndOfFileLoc,    retBytesReadLoc,   retOutputByteCount,   retErrorMesg   )     )
			{

				keepGoing =  false;
				gotBlock =  false;    //   Get here when hit End-of-File ( FileEnd Button).  But may have loades some of the 10 blocks.   12/11
			}
			else
			{  gotBlock =  true;

		//		retBytesRead  +=   retBytesReadLoc;   
				retBytesRead  +=   retOutputByteCount;  
			}
		}
		else
		{			                                               //  when Load_Next_WAV_Frame()  is done, it will ALWAYS leave a   16bit STEREO   sample in  m_outputBufferStreaming

			if(    ! Load_Next_WAV_Frame(    retEndOfFileLoc,    retBytesReadLoc,    retOutputByteCount,   retErrorMesg   )     )
			{

//  ****************************  retOutputByteCount   does not look right for   GreySolo_22hz.wav, but it works.   12/22/11   **********************

				keepGoing =  false;
				gotBlock =  false;   //   Get here when hit end of file ( FileEnd Button).  But may have loades some of the 10 blocks.   12/11
			}
			else
			{	gotBlock =  true;

			//	retBytesRead  +=    retBytesReadLoc;
				retBytesRead  +=    retOutputByteCount;  
			}
		}


		if(   retEndOfFileLoc   )
		{
			keepGoing =  false;
			retEndOfFile =  true;
		}


		if(   gotBlock   )
		{
			long  bufSize  =   m_outputBufferSampleCountNotSlowDown  *   MP3rEADERbYTESpERsAMPLE;

			char*  src  =    m_outputBufferStreaming;    //  this is where  Load_Next_MP3_Frame()  and  Load_Next_WAV_Frame()  placed its bytes

		    char*  dst  =    destBlockPtr;    //  somewhere inside  m_backwardsPlayBuffer


			memmove(   dst,   src,      bufSize   ); 
		}



		if(   gotBlock   )
			count++;

		destBlockPtr   +=    blockSize;
	}  //  while


	m_numberOfTenBlocksLoaded =   count;

	return  true;
}




					/////////////////////////////////////////////


bool	 WavConvert::Load_Next_MemoryBlock_Frame(   bool&   retEndOfFile,    long&  retBytesRead,   bool  backwardsFlag,	 CString&   retErrorMesg   )
{

		//		When this function returns, it will have filled 2 buffers:
		//			a)   	'm_outputBufferStreaming'                      ...Will hold the slowedDown samples if slowSpeed ( currently do NOT use these for detection, missing harmonics  3/11  )
	    //			b)     'm_outputBufferStreamingNoSlowDown'   ...Will hold the normal SRC-samples (use these for detection at slowed down speed



	short   timeStretchAlgo  =    m_fftSlowDown.Get_SlowDown_Alorythm_Code();     //   0: PitchShift with Resampling      1:  Two different sized FFTs,  no resampling     **** ADJUST****


	ASSERT(   timeStretchAlgo >= 0   &&   timeStretchAlgo <= 1  );



	retBytesRead =  0;
	retErrorMesg.Empty();


	bool   isAmp3 =   Is_A_MP3_File_Loaded();




	long   maxNumberOfBlocksInBuffer =      m_maxNumberOfbackwardsPlayBufferBlocks;    // **********   HARDWIRED *******************

	long   trueNumberOfBlocksInBuffer  =     m_maxNumberOfbackwardsPlayBufferBlocks;   //  init



	bool   hadPartialOfTenBlocksRead  =   false;
	long   retOutputByteCount =  -1;



	long   bytesPerSampleOutput        =     MP3rEADERbYTESpERsAMPLE;    //   is ALWAYS 4  ( 16bit stereo )

	long   sourceBuffersSampleCount  =     STREAMINGmp3BUFFERSIZE  /   m_bytesPerSampleSRCstream;  // **** WRONG for   GreySolo_22hz.wav  ????


//	long   blockSize =     m_outputBufferSizeStreaming;          **** WRONG because increases size with SlowSpeed, not what we want
	long   blockSize =     m_outputBufferSampleCountNotSlowDown  *   bytesPerSampleOutput; 




	if(         backwardsFlag   
		 &&   m_backwardsPlayBufferSize  >  0    )   //  For some ODD File types this is not possible.  (    )   12/11 
	{													

				               //   Must read from NEW  'm_backwardsPlayBuffer'  which holds  Multiple ( usually 10 )    4416-byte blocks    12/21/11


		ASSERT(      m_backwardsPlayBufferBlockIndex    <      maxNumberOfBlocksInBuffer 
					&&   m_backwardsPlayBufferBlockIndex    >=    0    );

							// ************   TROUBLE here for   Fugue.mp3  [ number of subBlocks is 2.5    17664 bytes for  m_outputBufferSizeStreaming 
							//
							//    BUT now I trap the error above with :   m_backwardsPlayBufferSize  >  0    
							//
							//    (  Also bad for   Highway49_SOLO_8bit_Mono.wav,   Talkin.mp3,   MiniSkirt_Solo_22hz_8bit.wav,  Clarinet_16hz_mono.wav  )   12/2011   


		if(    m_numberOfTenBlocksLoaded   <   maxNumberOfBlocksInBuffer    )
		{
			trueNumberOfBlocksInBuffer =   m_numberOfTenBlocksLoaded;

			hadPartialOfTenBlocksRead  =   true;
		}
		



		long  realIndex   =      ( trueNumberOfBlocksInBuffer -1)  -  m_backwardsPlayBufferBlockIndex;   //  { 0 - 9}   remember , going backwards in time, so start at the end of the Buffer
		if(    realIndex   <  0  )
		{
			retEndOfFile =  true;
			return  false;    //  Means we his the last of the  PARTIAL of 10 subBlocks.    12/11
		}



		char   *src  =    (char*)(      m_backwardsPlayBuffer   +   (  realIndex  *  blockSize  )         );

		char   *dst  =    m_outputBufferStreaming;


		memmove(    dst,   src,     blockSize   );



// ***********   CAREFUL with these assignments     ???   12/11   ************************************

		retBytesRead           =     blockSize; 

		retOutputByteCount  =     blockSize;    // ********   ALWAYS ??????   Will be used below  in:    memmove(  dstBytAlt, srcByt,     retOutputByteCount   );   



		m_backwardsPlayBufferBlockIndex++;     //  increment for next call to this function with BACKWARDS play
	}    //   if(  backwardsFlag 

	else
	{						// Forward Play
		if(   isAmp3  )
		{                          //  when Load_Next_MP3_Frame()  is done, it will ALWAYS leave a   16bit STEREO   sample in  m_outputBufferStreaming

			if(    ! Load_Next_MP3_Frame(    retEndOfFile,      retBytesRead,   retOutputByteCount,   retErrorMesg   )     )
				return  false;
		}
		else
		{			              //  when Load_Next_WAV_Frame()  is done, it will ALWAYS leave a   16bit STEREO   sample in  m_outputBufferStreaming

			if(    ! Load_Next_WAV_Frame(    retEndOfFile,    retBytesRead,    retOutputByteCount,   retErrorMesg   )     )
				return  false;
		}
	}    

	




	if(    m_doResamplingStreaming    )
	{
	
							//   separate into Left and Right samples,  do resampling 2x,   then merge together for final WAV format 

		bool    lessThanOneValues =   false;    //  false is DEFAULT.    

//  left
		if(   ! SndSample::Make_Float_Sample(  0,    m_outputBufferStreaming,    retOutputByteCount,   bytesPerSampleOutput,    m_reSampleArrayLeft,	lessThanOneValues,   retErrorMesg  )   )
		{
			End_ProgressBar_Position_GLB();
			return  false;
		}


		float   *yptptLeft =   NULL;        //  intdecF_ReSample  will alloc memory for this return sample.
		int        ylenLeft =   -1;             //  intdecF_ReSample  will return this value

		m_reSamplerStreaming.Resample_Signal(  	m_reSampleArrayLeft,    sourceBuffersSampleCount,    &yptptLeft,  	&ylenLeft   );



//	right
	  	if(   ! SndSample::Make_Float_Sample(  1,    m_outputBufferStreaming,    retOutputByteCount,   bytesPerSampleOutput,    m_reSampleArrayRight,  lessThanOneValues,	 retErrorMesg  )   )
		{
			End_ProgressBar_Position_GLB();
			return  false;
		}


		float   *yptptRight =   NULL;       
		int        ylenRight =   -1;            

		m_reSamplerStreaming.Resample_Signal(  	m_reSampleArrayRight,   sourceBuffersSampleCount,     &yptptRight,    &ylenRight  );




		ASSERT(   ylenLeft  ==   ylenRight   );

		bool    toStereo       =   true;    
		long    retByteCount =   -1; 
		bool   noMemoryRelease  =   false; 

		double  volumeTweak  =    m_fftSlowDown.Get_Speeds_Volume_Tweak_Factor();   // ************UNTESTED here.   12/30/2011



		lessThanOneValues =   false;    //  false is DEFAULT  for   Merge_Float_Channels_To_Sample()   



		if(    ! SndSample::Merge_Float_Channels_To_Sample(   toStereo,    ylenRight,     bytesPerSampleOutput,    &yptptLeft,    &yptptRight, 		
		                                                                                        &( m_reSamplerStreaming.m_dDstSamplesPtr ),     retByteCount,    lessThanOneValues,
																					                                   noMemoryRelease,   volumeTweak,   retErrorMesg   )   )
		{  return  false;    
		}
		else
		{  ASSERT(   retByteCount  ==    ( m_outputBufferSampleCountNotSlowDown  * bytesPerSampleOutput )    );   

			    // ****************  Maybe    'retByteCount'   is what should really be returned ???  make a new parm ????   ************


			char*   srcByt     =    m_reSamplerStreaming.m_dDstSamplesPtr;   //  copy results back to WavConvert's  output buffer
			char*   dstByt     =    m_outputBufferStreaming;
			char*   dstBytAlt  =    m_outputBufferStreamingNoSlowDown;

																//   do Byte-reversal  for new BACKWARDS Play    12/11  
			if(    backwardsFlag   )
				SndSample::Reverse_16bit_Sample_Order(    (BYTE*)srcByt,     retByteCount   );    //  nice NEW static funct,  use it  12/11


			memmove(    dstByt,        srcByt,      retByteCount   );      //  copy to BOTH output buffers
			memmove(    dstBytAlt,    srcByt,      retByteCount   ); 
		}		
	}

	else     //   if(  ! m_doResamplingStreaming  
	{  
		ASSERT(    retOutputByteCount  ==    ( m_outputBufferSampleCountNotSlowDown * bytesPerSampleOutput )   );


			//   Right now both  'm_outputBufferStreaming'   and  'm_outputBufferStreamingNoSlowDown'  have the same number of samples. 
			//
			//	  But by the exit of this function   'm_outputBufferStreaming'   will have MORE samples,  becase it WAS slowed down.

		char*   srcByt     =    m_outputBufferStreaming;
		char*   dstBytAlt  =    m_outputBufferStreamingNoSlowDown;


								//    REVERSE bytes here for BACKWARDS play,   must do BEFORE  SlowDown-Algo does its expansion of bytes   12/11
		if(    backwardsFlag    )
		{                               //    Need to reversed BOTH buffers if going backwards.  
										//	   Reverse the first ( m_outputBufferStreaming ),  and then copy it to second (  m_outputBufferStreamingNoSlowDown )


// *****************  a PROBLEM  for   1.5  Speed ???   Seems fine.  1/27/2012  ***************************************

			SndSample::Reverse_16bit_Sample_Order(    (BYTE*)m_outputBufferStreaming,    retOutputByteCount  );  //  nice NEW static funct,  use it  12/11
		}    


		memmove(    dstBytAlt,    srcByt,     retOutputByteCount   );     //   for both FORWARD and BACKWARD play,  need this memove() 		
	}




	long  fftFrameSize =    m_fftSlowDown.m_fftFrameSizeSlowDown;           //   2048   ...almost always 

	long  osamp          =   m_fftSlowDown.m_overSampleRatioSlowDown;    //      8     ...almost always 






	bool    doPhaseFiltering  =  false;     //    false          TEST SWITCH:    false is DEFAULT.   This is very EXPERIMENTAL.   12/2011




	if(    m_fftSlowDown.m_playSpeed   !=   1.0   )
	{

		if(         timeStretchAlgo  ==  0   )     //  this is the  DEFAULT,   
		{

			if(   ! m_fftSlowDown.Apply_SlowDown_wPitchShift(           (BYTE**)( &m_outputBufferStreaming ),    m_outputBufferSampleCountNotSlowDown,    true,   fftFrameSize,   osamp,    retErrorMesg   )    )
				return  false;    
		}
		else if(    timeStretchAlgo  ==  1   )
		{

			if(   ! m_fftSlowDown.Apply_SlowDown_wHorizontal_FFTs(    (BYTE**)( &m_outputBufferStreaming ),    m_outputBufferSampleCountNotSlowDown,    true,   fftFrameSize,   osamp,    retErrorMesg   )    )
				return  false;    
		}
		else  
		{  ASSERT( 0 );   }
	}

	else    //   Speed =  1  
	{
																	//  EXPERIMENTAL:    a Phase-Filtering Algo here to remove CENTER-VOICE .
		if(   doPhaseFiltering   )
		{

			ASSERT( 0 );    // Do NOT use this for a while   7/2012

				//  Could also test this here ....
				//
				//	 FFTslowDown::Change_Samples_Pitch(   BYTE**  retSamplesPtr,    long numSamples,    bool stereoFlag,  float  pitchShift,   long  fftFrameSize,  long osamp,     CString&   retErrorMesg   )


			if(   ! m_fftSlowDown.Apply_Phase_Filtering_wFFT(    (BYTE**)( &m_outputBufferStreaming ),    m_outputBufferSampleCountNotSlowDown,    true,     fftFrameSize,
																																							osamp,     retErrorMesg   )   )
			{  return false;   }
		}
	}



// ********************   INSTALL delay here ???  **********************************



	return  true;
}




					/////////////////////////////////////////////


bool	 WavConvert::Load_Next_MP3_Frame(   bool&   retEndOfFile,    long&  retBytesRead,   long& retOutputByteCount,   CString&   retErrorMesg   )
{

	//  	retOutputByteCount   is NOT constant, expansion can occur here.   If this is a MONO sample we will send back 2x as many bytes as a STEREO sample.  2/10 

	
	retBytesRead =  0;
	retOutputByteCount = -1;
	retErrorMesg.Empty();


	int   origBufSize =    STREAMINGmp3BUFFERSIZE;     //  4416    


	if(    m_mp3DecoderStreaming.eof()   )
	{
		retErrorMesg =  "WavConvert::Load_Next_MP3_Frame   hit End of FILE,  without loading a block. " ;
		retEndOfFile =    true;    //  no error,  just hit the end of file
		return   false;
	}



	try
	{
		int   bufSize  =    m_mp3DecoderStreaming.read_binfile(   m_outputBufferStreaming,   origBufSize   );  //  BIG,   ultimately calls  ampegdecoder::decode()  


//  *****************   BUG  here, or is this OK way to deal with the END of the File during play?????     11/6/11  **********************
		if(   bufSize   >  origBufSize   )
		{
			ASSERT( 0 );
		}
		else if(   bufSize   <  origBufSize   )    //  this might mean the end of the file    11/6/11
		{

	//		m_fileUniStreaming.close();   // *************  BAD   11/11 ******************
	//		m_mp3DecoderStreaming.close_binfile();

			retErrorMesg =  "WavConvert::Load_Next_MP3_Frame  possibly hit  END of FILE...  it loaded a  Under-Sized  Block of Memory." ;
			retEndOfFile =    true;    //  no error,  just hit the end of file
			return   false;
		}
// ***********************


		if(  ! bufSize  )
		{
			retErrorMesg  =   "WavConvert::Load_Next_MP3_Frame failed,   decoder's read was bad." ;

			m_fileUniStreaming.close();  
			m_mp3DecoderStreaming.close_binfile();

		//	End_ProgressBar_Position_GLB();
			return  false;  
		}
		else
		{	if(    m_stereoFlagSRCstream    )    
			{
				retOutputByteCount  =   bufSize;     //  STEREO  

				retBytesRead  +=    bufSize;     
			}
			else
			{		//  For  MONO,  we must expand the mono bytes to both stereo channels. If we copy from the end of the memory, we will NOT overight the SRC bytes
					//  At this point only the first HALF of the buffer has the mono samples.

				long	 sourceBuffersSampleCount  =    STREAMINGmp3BUFFERSIZE  /   m_bytesPerSampleSRCstream;   //  m_bytesPerSampleSRCstream can change if MONO or Stereo

				long   sourceBuffersByteCount       =     sourceBuffersSampleCount   *   m_bytesPerSampleSRCstream; 


				char*   src  =     m_outputBufferStreaming    +    ( sourceBuffersByteCount         -1 );    //  point to the MIDDLE of the buffer
				char*   dst  =     m_outputBufferStreaming    +    ( sourceBuffersByteCount * 2   -1 );    //  point to the END of the buffer


				for(   long i =0;     i < (  sourceBuffersSampleCount -1);     i++   )  
				{
					*dst  =           *src;            //  duplicate HIGH byte of 16bit mono sample
					*(dst -2 )  =    *src;

					*(dst -1 )  =   *(src -1);     //  duplicate LOW byte of 16bit mono sample
					*(dst -3 )  =   *(src -1);

					src  -=  2;    //  backup to the next  MONO    sample
					dst  -=  4;    //  backup to the next  STEREO sample
				}


				retOutputByteCount  =   sourceBuffersByteCount  * 2;    //  Because of the Mono's EXPANSION to STEREO bytes, we send out 2x as many as were read in.

				retBytesRead +=    bufSize;     
			}			
		}

	}
	catch(   CFileException   *pException   )
	{
		TCHAR    szCause[ 255 ];  
		CString   strRaw;
		pException->GetErrorMessage(  szCause,  255  );
		pException->Delete();
		strRaw =  szCause;
		retErrorMesg.Format(   "WavConvert::Load_Next_MP3_Frame  failed,  could not read MP3 file because:  %s." ,   strRaw  );
		return  false;
	}

	return  true;
}




					/////////////////////////////////////////////


bool	 WavConvert::Load_Next_WAV_Frame(   bool&   retEndOfFile,    long&  retBytesRead,     long& retOutputByteCount,   CString&   retErrorMesg   )
{
		//   This function will READ from different WAV formats ( 16bit, 8bit,  stereo,  mono )  but its 
		//   OUTPUT bytes, written to  m_outputBufferStreaming,   will ALWAYS be in  16bit-stereo      2/10

	
	retBytesRead =  0;
	retOutputByteCount =  0; 
	retErrorMesg.Empty();


	UINT    byteCountAdjusted   =     STREAMINGmp3BUFFERSIZE;   




			//	mmioGetinfo :    Retrieves information about a file opened by using the mmioOpen function.  This information
			//							allows the application to  'directly access'  the I/O buffer,  if the file is opened for buffered I/O.

	MMIOINFO     mmIOinfo;     // current status of <mmIO>
	int                 errorCode;

    if(    errorCode =      mmioGetInfo(   m_mmIO,    &mmIOinfo,   0   )      != 0    )
    {
		retErrorMesg  =  "WavConvert::Load_Next_WAV_Frame   failed,    mmioGetInfo  failed." ;
		retBytesRead  =   0;
		return   false;
    }

//	if(    m_mp3DecoderStreaming.eof()   )    ***  WANT similar   test for ???
//	{
//		retErrorMesg =  "WavConvert::Load_Next_MP3_Frame   hit End of FILE,  without loading a block. " ;
//		retEndOfFile =    true;    //  no error,  just hit the end of file
//		return   false;
//	}
	


	unsigned char   bytVal0=0,    bytVal1=0,   bytVal2=0,    bytVal3=0; 

	char    chVal0=0,    chVal1=0,   chVal2=0,    chVal3=0;   //  When used to be below,  but got a  error   Run-Time Check Failure #3 ( this bug was in PitchScope 1.0
	short   byteMember;
	short	  sampLeft,   sampRight;

	char                 charErase =      0;
	unsigned char    byteErase =  128;   //  does this work ????


	char*   destBufferRover =    m_outputBufferStreaming;



																								//  Copy the bytes  from the IO to the buffer. 


	for(   UINT  byteIdx = 0;        byteIdx <   byteCountAdjusted;        byteIdx++   )
	{															 

																									//   Are we ready to ADVANCE to another BlockRead...
		if(     mmIOinfo.pchNext   ==    mmIOinfo.pchEndRead    )
		{
																													//   fileOffset  =   mmIOinfo.lDiskOffset;    ....DEBUG
			if(   (  errorCode =    mmioAdvance(   m_mmIO,    &mmIOinfo,    MMIO_READ   )     )  != 0   )
			{
				retErrorMesg  =     "WavConvert::Load_Next_WAV_Frame   failed,  mmioAdvance  failed." ;
				retBytesRead  =   0;
				return   false;
			}


			if(    mmIOinfo.pchNext    ==    mmIOinfo.pchEndRead    )
			{
				/****
				retErrorMesg  =     "WavConvert::Load_Next_WAV_Frame  failed,  corrupt file." ;   
				retBytesRead  =   0;     //  **** GET Here when SlowPlay of GreySolo  12/28/2011  ******************
				return   false;				  //   Is this just a healthy EOF ????
				****/
				retEndOfFile =  true;     // **** See how this goes .  SHOULD I be trying to use the remained of bytes??  So far, looks good.    12/28/2011  ****************
				return  false;
			}
		}
 


	    char                charVal =      *(  mmIOinfo.pchNext    );			 //  Fetch the input byte
		unsigned char  byteVal  =      *(  mmIOinfo.pchNext    ); 




		if(    m_stereoFlagSRCstream  ==  true   )    //   STEREO
		{

			if(    m_bytesPerSampleSRCstream  ==        4   )    //   16-bit   samples
			{
				*destBufferRover =   charVal;      destBufferRover++;  
				retOutputByteCount++;
			}
			else if(    m_bytesPerSampleSRCstream  ==  2   )     //  8bit samples
			{

				byteMember  =    (  byteIdx  %  2L  );       //  which of the    4 Bytes in the LONG    do we have for this  ' '

				if(    byteMember       ==  0   )   
				{
					chVal0   =   charVal;       //  left channel
					bytVal0 =    byteVal;
				}
				else if(   byteMember  == 1   )   
				{
					chVal1  =   charVal;      //  right channel
					bytVal1 =    byteVal;
									   //    ************* WEIRD,  but for   8bit-SRCsample   need to use  unsigned char's value.   
			                           //  							 Might have to do with the SignBit in a  8-bit number.    2/20/10 
									  //							 ( see  WavConvert::Process_Sample_Packet_NoResamp() notes )

					sampLeft =     (    (short)bytVal0    -  128    );    //  ***** also see this logic in   WavConvert::Process_Sample_Packet_NoResamp 

					if(           sampLeft  >   127   )
					{	sampLeft =  127;    ASSERT( 0 );  }    // ****  OMIT these tests if this never occurs
					else if(    sampLeft  <  -127   )
					{	sampLeft =  -127;   }    //  can land here if  bytVal0 =  0


					sampRight =     (    (short)bytVal1    -  128    );   //  TRANSLATE the   'unsigned char'   into a  SIGNED value
																						  //    (  also see this logic in   WavConvert::Process_Sample_Packet_NoResamp()
					if(           sampRight  >   127   )
					{	sampRight =  127;   ASSERT( 0 );  }
					else if(    sampRight  <  -127   )
					{	sampRight =  -127;    }    //  can land here if  bytVal0 =  0


					*destBufferRover =    charErase;   //   charErase      byteErase          
					destBufferRover++;    //  write to LEFT channel

					*destBufferRover =       sampLeft;        destBufferRover++;    

					*destBufferRover =   charErase;    //   charErase;       byteErase         
					destBufferRover++;    //  write same values to the  RIGHT channel

					*destBufferRover =       sampRight;      destBufferRover++;  
													
					retOutputByteCount +=  4;
				}
			}
			else
			{  ASSERT( 0 );  }
		}   //  stereo

		else       //  MONO
		{
			if(    m_bytesPerSampleSRCstream  ==    2   )    //   16-bit   sample
			{

				byteMember  =    (  byteIdx  %  2L  );       //  which of the    4 Bytes in the LONG    do we have for this  ' '

				if(    byteMember       ==  0   )   
				{
					chVal0   =   charVal;       //  left channel
					bytVal0 =    byteVal;
				}
				else if(   byteMember  == 1   )   
				{
					chVal1  =   charVal;      //  right channel
					bytVal1 =    byteVal;

					*destBufferRover =    chVal0;     	destBufferRover++;    //  write to LEFT channel
					*destBufferRover =      chVal1;       destBufferRover++;    

					*destBufferRover =   chVal0;    	   destBufferRover++;      //  write same values to the  RIGHT channel
					*destBufferRover =      chVal1;      destBufferRover++;  

					retOutputByteCount +=  4;
				}
			}
			else if(    m_bytesPerSampleSRCstream  ==  1   )     //  8bit sample
			{

				chVal0  =   charVal;      
				bytVal0 =    byteVal;

				sampLeft =     (    (short)bytVal0    -  128    );    //  ***** also see this logic in   WavConvert::Process_Sample_Packet_NoResamp 

				if(           sampLeft  >   127   )
				{	sampLeft =  127;    ASSERT( 0 );  }    // ****  OMIT these tests if this never occurs
				else if(    sampLeft  <  -127   )
				{	sampLeft =  -127;   }    //  can land here if  bytVal0 =  0

				*destBufferRover =    charErase;  	     destBufferRover++;    //  write to LEFT channel
				*destBufferRover =       sampLeft;        destBufferRover++;    

				*destBufferRover =   charErase;           destBufferRover++;    //  write same values to the  RIGHT channel
				*destBufferRover =       sampLeft;        destBufferRover++;  
													
				retOutputByteCount +=  4;
			}  
			else  
			{  ASSERT( 0 );   }

		}  //  mono



		( BYTE* )mmIOinfo.pchNext++;    	

//		m_currentByteIdx++;    *** CALLING function will have to update this
	}




//	endOffset   =      mmIOinfo.lDiskOffset;  


				//  mmioSetInfo()  :    updates the information retrieved by the mmioGetInfo function about a file opened 
				//								by using the mmioOpen function. Use this function to terminate   'direct buffer access' 
				//								of a file opened for buffered I/O.

    if(    (  errorCode =     mmioSetInfo(   m_mmIO,    &mmIOinfo,   0   )     )      != 0    )
	{
		retErrorMesg  =     "WavConvert::Load_Next_WAV_Frame   failed,  mmioSetInfo  failed." ;
		retBytesRead  =   0;
		return   false;
	}

	retBytesRead  =     byteCountAdjusted;


//	TRACE(  "...finished a BLOCK write.  \n"  );


	return  true;
}







					/////////////////////////////////////////////





					/////////////////////////////////////////////


bool	  WavConvert::Fetch_Blocks_Bytes(   bool&   retEndOfFile,    long&  retBytesRead,   long  currentByteIdx,    CString&   retErrorMesg   )
{

	//  ****************************  NOT  CALLED    ********** OMIT *************************************



			//    this only gets called by  BitSourceStreamingMP3::Fetch_Streaming_Samples_Direct_PPlayer    2/10
			//
			//    It will only ask for  


	retErrorMesg.Empty();
	retEndOfFile   =   false;
	retBytesRead =  0;


	bool   doneReadingBlock =  false;
	long   blockCount =  0;
	long   bytesPerSample =   MP3rEADERbYTESpERsAMPLE;


	ASSERT(   m_outputBufferStreaming !=  NULL    );    // *** INSTALL error report



	while(        ! m_mp3DecoderStreaming.eof() 
			   &&  ! doneReadingBlock    )		//   loop to PROCESS all the bytes in the file 
	{

		/****
		int   origBufSize =   STREAMINGmp3BUFFERSIZE;     //  4416

	

		int   bufSize  =    m_mp3DecoderStreaming.read_binfile(   m_outputBufferStreaming,   origBufSize   );    //  *** BIG,   ultimately calls   ampegdecoder::decode()   *****
		if(  ! bufSize  )
		{
			  retErrorMesg  =   "WavConvert::Fetch_Blocks_Bytes failed,  decoder's read was bad." ;

			  m_fileUniStreaming.close();  
			  m_mp3DecoderStreaming.close_binfile();

		 //  Write_WAVfiles_Length(  wavFile   );
		//	  wavFile.Close();

			  End_ProgressBar_Position_GLB();
			  return  false;  
		}
		else
			retBytesRead +=   bufSize;



		  if(   m_doResamplingStreaming    )
		  {
	
							//   separate into Left and Right samples, do resampling 2x,  then merge together for final WAV format 

				long	  totInputSamplesCnt =     STREAMINGmp3BUFFERSIZE  /  bytesPerSample;   


																																	//  left
			  	if(   ! Make_Float_Sample(  0,    m_outputBufferStreaming,    origBufSize,   bytesPerSample,    m_reSampleArrayLeft,	retErrorMesg  )   )
				{
					End_ProgressBar_Position_GLB();
					return  false;
				}


				float   *yptptLeft =   NULL;        //  intdecF_ReSample  will alloc memory for this return sample.
				int        ylenLeft =   -1;             //  intdecF_ReSample  will return this value

				m_reSamplerStreaming.Resample_Signal(  	m_reSampleArrayLeft,   totInputSamplesCnt,     &yptptLeft,   	&ylenLeft   );


																																	//	right
			  	if(   ! Make_Float_Sample(  1,    m_outputBufferStreaming,    origBufSize,   bytesPerSample,    m_reSampleArrayRight, 	 retErrorMesg  )   )
				{
					End_ProgressBar_Position_GLB();
					return  false;
				}

				float   *yptptRight =   NULL;       
				int        ylenRight =   -1;            

				m_reSamplerStreaming.Resample_Signal(  	m_reSampleArrayRight,   totInputSamplesCnt,     &yptptRight,    &ylenRight   );



				ASSERT(   ylenLeft  ==   ylenRight   );

				bool    toStereo       =   true;
				long    retByteCount =   -1; 


				if(    ! Merge_Float_Channels_To_Sample(   toStereo,    ylenRight,     bytesPerSample,      &yptptLeft,   &yptptRight,  
		                                                                                &( m_reSamplerStreaming.m_dDstSamplesPtr ),   
																					                                    retByteCount,   retErrorMesg   )   )
				{
						// ******  INSTALL other cleanup ????  ****************************************
				//	End_ProgressBar_Position_GLB();
					ASSERT( 0 );
					return  false;
				}
				else
				{
					if(   retByteCount  !=   m_outputBufferSizeStreaming   )    //  should be the same
					{
						int   dummy =  9;
					}


					char*   srcByt  =    m_reSamplerStreaming.m_dDstSamplesPtr;   //  copy results back to WavConvert's  output buffer
					char*   dstByt  =    m_outputBufferStreaming;


					for(    long  byt = 0;     byt <  m_outputBufferSizeStreaming;      byt++    );
					{
						*dstByt  =   *srcByt;

						srcByt++;
						dstByt++;
					}

					//   wavFile.Write(   m_dDstSamplesPtr,    retByteCount   );
				}		
		  }
		  else     
		  {   // wavFile.Write(   sampbuf,     bufSize   );	
		  }
		****/
		

		  if(    ! Load_Next_MemoryBlock_Frame(    retEndOfFile,       retBytesRead,   false,   retErrorMesg   )    )
		  {
			 ASSERT( 0 );


			if(  retEndOfFile  )
				return  true;
			else
				return  false;
		  }




		  blockCount++;

		  doneReadingBlock =   true;


		   int  curSrcPos  =     m_fileUniStreaming.Get_File_Position_Actual();
	
		/****
		  int  curSrcPos            =       mp3File.Get_File_Position_Actual();
		  double  percentDone  =    (  (double)curSrcPos    /   (double)srcFileLength   )    * 100.0;
		  Set_ProgressBar_Position_GLB(  (long)percentDone  );

		  if(  (  blockCount  %  30  )   ==  0    )
			  TRACE(  "Block  %d   has been written( mp3->WAV ).\n",   blockCount   ); 
		****/

		}   //   while(  ! m_mp3DecoderStreaming.eof()   )





	if(    m_mp3DecoderStreaming.eof()   )
		retEndOfFile =   true;   //    SEEMS OK for this to happen now,  no crashes.    2/2/10


	return  true;
}






					/////////////////////////////////////////////
					//////////////    WAV  stuff   /////////////
					/////////////////////////////////////////////


bool	WavConvert::Get_WAV_Header_Info(   CString&   wavFilePath,
										                          unsigned short&    retFormatTypeWAVsrc,            //   format type 
																  short&                  retChannelsCountWAVsrc,       //   number of channels (i.e. mono, stereo...) 
																  short&                  retBitsPerSampleWAVsrc,        //   number of bits per sample of mono data 
																  long&                    retSampleRateWAVsrc,             //   sample rate 
																   long&           retTotalSamplesBytesWAVsrc,																										
										                           CString&      retErrorMesg  )
{


	retErrorMesg.Empty();
	retFormatTypeWAVsrc      =   0;            //   format type 
	retChannelsCountWAVsrc =   -1;               //   number of channels (i.e. mono, stereo...) 
	retBitsPerSampleWAVsrc  =   -1;       //   number of bits per sample of mono data 
	retSampleRateWAVsrc      =   -1;       //   sample rate 
	retTotalSamplesBytesWAVsrc  =      -1;      //   NEW,  seems accurate   9/15/02


	
	if(   wavFilePath.IsEmpty()   )
	{
		retErrorMesg =   "WavConvert::Get_WAV_Header_Info failed,   wavFilePath is empty."  ;
		return  false;
	}




	WAVEFORMATEX   *wavFormat =  NULL; 
	HMMIO                   mmIO;   	             //    handle to an  OPEN .wav  FILE 
	MMCKINFO             chunkInfoParent;    //    important to have this data PERSIST between Play stops
	MMCKINFO             mmChunkInfo;       //    'CHUNK'      contains information about a  'CHUNK'  in a  RIFF file
	int      errorCode;




   if(   (  errorCode=    WavFile_Open(   wavFilePath.GetBuffer(0),    &mmIO,    &wavFormat,     &chunkInfoParent  )    )    !=  0   )
    {																		//   ALLOCATES  the  WaveFormatEx   structure

		if(    wavFormat   !=   NULL     )
		{  GlobalFree(  wavFormat   );	      wavFormat =  NULL;    }

		if(    mmIO  !=   NULL   )    
		{  mmioClose(   mmIO,   0  );		  mmIO =  NULL;       }	       


		retErrorMesg.Format(   "%s could NOT be opened or found( WAV Hdr Info )." ,    wavFilePath   );   //  goes directly to user
        return  false;
    }




																		  //  This moves us to the 'DATA' area in the file

    if(  (  errorCode=    WavFile_Start_Reading_Data(    &mmIO,     &mmChunkInfo,     &chunkInfoParent     )  )   != 0  )
    {

		if(    wavFormat   !=   NULL     )
		{  GlobalFree(  wavFormat   );	   wavFormat =  NULL;    }

		if(    mmIO  !=   NULL   )    
		{  mmioClose(   mmIO,   0  );		  mmIO =  NULL;       }	       

		retErrorMesg =     "Get_WAV_Header_Info  failed,   WavFile_Start_Reading_Data  failed." ;
        return  false;
    }
	


	/********

   typedef struct  tWAVEFORMATEX
	{
		WORD        wFormatTag;              //  format type 
		WORD        nChannels;                //  number of channels (i.e. mono, stereo...) 
		DWORD       nSamplesPerSec;     //  sample rate 
		DWORD       nAvgBytesPerSec;    //  for buffer estimation 
		WORD        nBlockAlign;              //  block size of data 
		WORD        wBitsPerSample;        //  number of bits per sample of mono data 
		WORD        cbSize;                     //  the count in bytes of the size of 
											             //extra information (after cbSize) 

	} WAVEFORMATEX;
	***/

	retFormatTypeWAVsrc      =   wavFormat->wFormatTag;            //   format type 
	retChannelsCountWAVsrc =   wavFormat->nChannels;               //   number of channels (i.e. mono, stereo...) 
	retBitsPerSampleWAVsrc  =   wavFormat->wBitsPerSample;       //   number of bits per sample of mono data 
	retSampleRateWAVsrc      =   wavFormat->nSamplesPerSec;       //   sample rate 

	retTotalSamplesBytesWAVsrc  =      mmChunkInfo.cksize;      //   NEW,  seems accurate   9/15/02



																		// Cleanup
	if(    wavFormat   !=   NULL     )
	{  GlobalFree(  wavFormat   );	      wavFormat =  NULL;    }

	if(    mmIO  !=   NULL   )    
	{  mmioClose(   mmIO,   0  );		  mmIO =  NULL;          }	       

	return true;
}




					/////////////////////////////////////////////


bool    WavConvert::Convert_WAV_to_WAV(   CString&  retNewFileName,    CString&   retErrorMesg   )
{   


	long   defaultSamplingRate  =    44100;    //   *************   HARDWIRED,     PUT somewhere ****************

    long   bytesPerSampleDST  =   4;                 // ******  HARDWIRED ********

	int     doStereoDST =  1;     //  1: True   ******  HARDWIRED,  substitute with bool 

	long   outputChannelCount =  2;   //  2:  Always output stereo, at this time.   8/06

	long   outputPacketSize =   4;   //  DST is always stereo at 16bit  ...  4 bytes in a packet


	retNewFileName.Empty();
	retErrorMesg.Empty();

	if(   m_srcWavFilePath.IsEmpty()   )
	{
		retErrorMesg =   "Convert_WAV_to_WAV failed,   m_srcWavFilePath is empty."  ;
		return  false;
	}

	char  *filename =    m_srcWavFilePath.GetBuffer( 0 );  


	bool   stereoFlag =   false;
	if(   doStereoDST  ==   1   )
		stereoFlag =   true;




	int     errorCode;
	WAVEFORMATEX   *wavFormat =  NULL; 
	HMMIO                   mmIO;   	             //    handle to an  OPEN .wav  FILE 
	MMCKINFO             chunkInfoParent;    //    important to have this data PERSIST between Play stops
	MMCKINFO             mmChunkInfo;       //    'CHUNK'      contains information about a  'CHUNK'  in a  RIFF file


   if(   (  errorCode=    WavFile_Open(   m_srcWavFilePath.GetBuffer(0),    &mmIO,    &wavFormat,     &chunkInfoParent  )    )    !=  0   )
    {																		//   ALLOCATES  the  WaveFormatEx   structure
		if(    wavFormat   !=   NULL     )
		{  GlobalFree(  wavFormat   );	      wavFormat =  NULL;    }
		if(    mmIO  !=   NULL   )    
		{  mmioClose(   mmIO,   0  );		  mmIO =  NULL;       }	       

		retErrorMesg.Format(   "%s could NOT be opened or found( WAV to WAV )." ,    m_srcWavFilePath   );   //  goes directly to user
        return  false;
    }

																		  //  This moves us to the 'DATA' area in the file

    if(  (  errorCode=    WavFile_Start_Reading_Data(    &mmIO,     &mmChunkInfo,     &chunkInfoParent     )  )   != 0  )
    {
		if(    wavFormat   !=   NULL     )
		{  GlobalFree(  wavFormat   );	   wavFormat =  NULL;    }
		if(    mmIO  !=   NULL   )    
		{  mmioClose(   mmIO,   0  );		  mmIO =  NULL;       }	       

		retErrorMesg =     "Get_WAV_Header_Info  failed,   WavFile_Start_Reading_Data  failed." ;
        return  false;
    }
	

	m_formatTypeWAVsrc      =   wavFormat->wFormatTag;            //   format type 
	m_channelsCountWAVsrc =   wavFormat->nChannels;               //   number of channels (i.e. mono, stereo...) 
	m_bitsPerSampleWAVsrc  =   wavFormat->wBitsPerSample;       //   number of bits per sample of mono data 
	m_sampleRateWAVsrc      =   wavFormat->nSamplesPerSec;       //   sample rate 
	m_totalSamplesBytesWAVsrc  =      mmChunkInfo.cksize;      //   NEW,  seems accurate   9/15/02



	if(    m_sampleRateWAVsrc  ==   44100    )
	{

		if(        m_channelsCountWAVsrc  ==     2
			&&   m_bitsPerSampleWAVsrc  ==    16  )
		{
			if(    wavFormat   !=   NULL     )
			{  GlobalFree(  wavFormat   );	   wavFormat =  NULL;    }
			if(    mmIO  !=   NULL   )    
			{  mmioClose(   mmIO,   0  );		  mmIO =  NULL;       }	       

			retErrorMesg =     "This .WAV file does not need conversion. It is already at 44100 stereo, and 16bits per sample." ;
			return  false;
		}
	}



							  //  packetSize:  the number of bytes for 1 sample with StereoChannels combioned( ex:  stereo 16bit =  4  )

	long   maxPacketBufSize  =       128;
	unsigned   char   packetBuffer[   128   ];

	long  packetSize  =       m_channelsCountWAVsrc    *    ( m_bitsPerSampleWAVsrc  / 8 );   //  4 for

	if(     packetSize  >  maxPacketBufSize   )
	{
		retErrorMesg =     "Get_WAV_Header_Info failed, packetSize is too big." ;
		return  false;
	}



		


																		//   Create the NEW file name
	char *newname=0;

	newname  =   new  char[ strlen(  filename)  +5 ];

	if (strrchr(filename, '\\'))
		strcpy(newname, strrchr(filename, '\\')+1);
	else if (strrchr(filename, '/'))
		strcpy(newname, strrchr(filename, '/')+1);
	else if (strrchr(filename, ':'))
		strcpy(newname, strrchr(filename, ':')+1);
	else
		strcpy(newname, filename);

	if(   !strcmp(   newname  +strlen(newname)  -4,   ".wav"  )   )
		newname[  strlen(newname) - 4  ] =  0;

	//	strcat(  newname,      ".wav"   );
	strcat(  newname,         "_B.wav"   );


//	CString   newWAVfileName  =   newname;
	retNewFileName  =   newname;

	delete  newname;





	ReSampler    reSampler;

	long     origBufSize =   4608;          //   *** HARDWIRED...  pick a better number, or is this fine ????    8/06

	float    *sampleArrayLeft =  NULL,   *sampleArrayRight =  NULL;   //  2 intermediate samples
	char    *dstSamplesPtr =   NULL;   //   final DSTsample at new freq
	bool      doResampling;
	int         outputFreq  =   defaultSamplingRate;
	long       blocksSampleCount =  -1,   dstSampleCount =  -1;
	long      retDstStreamBufferSizeInSamples =  -1;    



	if(    m_sampleRateWAVsrc   ==   defaultSamplingRate   )
	{	

		doResampling =   false;

		dstSampleCount        =      origBufSize  /  outputPacketSize;  
		long  allocateDstSize  =      dstSampleCount  *   outputPacketSize;


		if(     (  dstSamplesPtr =   ( char* )malloc( allocateDstSize )   )     == NULL    )  
		{
			retErrorMesg =   "Convert_WAV_to_WAV failed,  could not allocate dstSamplesPtr for same frequency rate."  ;
			return  false;
		}			
	}
	else
	{	doResampling =   true;

		if(    ! Set_ReSample_Rates(   m_sampleRateWAVsrc,   defaultSamplingRate,   retErrorMesg   )    )
			return  false;
		
		outputFreq  =    defaultSamplingRate;   //   44100  hz



		if(    ! reSampler.Initialize(   m_sampleRateWAVsrc,   outputFreq,  origBufSize,   retDstStreamBufferSizeInSamples,  bytesPerSampleDST,  stereoFlag, retErrorMesg  )     )
		{
			//  ***  FAILS  for:    32 kHz,								8/06

			if(    wavFormat   !=   NULL     )
			{  GlobalFree(  wavFormat   );	   wavFormat =  NULL;    }
			if(    mmIO  !=   NULL   )    
			{  mmioClose(   mmIO,   0  );	   mmIO =  NULL;       }	       

			return  false;
		}




		blocksSampleCount =      origBufSize  /  packetSize;  		
		long    allocateSize  =      sizeof( float )  *   blocksSampleCount;


		if(     (  sampleArrayLeft =   ( float* )malloc( allocateSize )   )     == NULL    )  
		{
			ASSERT( 0 );
			retErrorMesg =   "Convert_WAV_to_WAV  failed,  could not allocate  sampleArrayLeft."  ;
			return  false;
		}



		if(   m_channelsCountWAVsrc  ==  2   )    //  2:  Only for Stereo 
		{
			if(     (  sampleArrayRight =   ( float* )malloc( allocateSize )   )     == NULL    )  
			{
				retErrorMesg =   "Convert_WAV_to_WAV  failed,  could not allocate  sampleArrayRight."  ;
				return  false;
			}
		}




		dstSampleCount =     (long)(      ( (double)outputFreq / (double)m_sampleRateWAVsrc    +  0.05 )   
																											   *   (double)blocksSampleCount   ); 
		long  allocateDstSize   =      dstSampleCount  *   outputPacketSize;


		if(     (  dstSamplesPtr =   ( char* )malloc( allocateDstSize )   )     == NULL    )  
		{
			retErrorMesg =   "Convert_WAV_to_WAV failed,  could not allocate  dstSamplesPtr for resample."  ;
			return  false;
		}			
	}   //    m_sampleRateWAVsrc   !=   defaultSamplingRate 






	try   
	{  CFile   wavDSTfile(     retNewFileName,  
										 CFile::modeCreate    |  CFile::modeWrite
									 //  CFile::modeCreate    |  CFile::modeWrite     |   CFile::modeNoTruncate
								 );				  // ***NOTE:   'modeNoTruncate'   allows APPEND to existing fie,  does not EMPTY/ERASE an EXISTING file




		int   do16bitDST =  1;     //  ( bool )   0:  8bit samples,     1:  16bit samples

		Write_WAV_Header(   wavDSTfile,   defaultSamplingRate,   doStereoDST,   do16bitDST,   0   );   // **** CAREFUL,  freq is REPLACED

		

		MMIOINFO   mmIOinfo;         //   current status of  <mmIO>

		if(    errorCode =   mmioGetInfo(    mmIO,    &mmIOinfo,    0    )    != 0    )    //  Assign/Init  the  MMIOINFO
		{													
			ASSERT( 0 );                 //   goto   ERROR_CANNOT_READ;
			retErrorMesg  =   "Convert_WAV_to_WAV failed,  mmioGetInfo can not read." ;
			return  false;
		}




																		//  Copy the bytes from the IO to the buffer. 
		long   packetIdx       =   0;
		long   bufSampleIdx =   0;

		bool   onLastBlock =  false;


		Begin_ProgressBar_Position_GLB(   ".WAV resampling..."    );  
		Set_ProgressBar_Position_GLB(  5  );




		for(   unsigned long byteIdx =0;      byteIdx <   mmChunkInfo.cksize;      byteIdx++   )      //  Reads in CHUNKS
		{									
			

			if(     byteIdx   ==    ( mmChunkInfo.cksize - 1 )     )
				onLastBlock =    true;


																	
			if(    mmIOinfo.pchNext    ==    mmIOinfo.pchEndRead    )     
			{																					//  Fetch new BLOCK....   (  ???  bytes ) 

				if(   (  errorCode =    mmioAdvance(   mmIO,    &mmIOinfo,    MMIO_READ    )     )  != 0   )
				{					  
					retErrorMesg  =   "Convert_WAV_to_WAV failed,  mmioAdvance." ;   //  goto   ERROR_CANNOT_READ; 
					End_ProgressBar_Position_GLB();
					return  false;
				} 
				else
				{  
					/****
					unsigned long    end,  start,   numBytes;              //  DEBUG info only     Do I need this????   it bothers the compiler
					start =    (  unsigned long   )(  mmIOinfo.pchNext  );
					end  =    (  unsigned long   )(  mmIOinfo.pchEndRead  );   
					numBytes =   end -  start;
					*****/
				}


				if(     mmIOinfo.pchNext    ==    mmIOinfo.pchEndRead     )
				{
					retErrorMesg  =   "Convert_WAV_to_WAV failed,  .WAV file is corrupt." ;  //  nError =  ER_CORRUPTWAVEFILE;               goto   ERROR_CANNOT_READ;
					End_ProgressBar_Position_GLB();
					return  false;
				}
			}
 



			bool   retProcessBuffer =  false;		
			long   retByteCount =  -1; 

		//	char                byVal  =      *(     ( unsigned char* )mmIOinfo.pchNext    );       ...BAD
			unsigned char   byVal  =      *(     ( unsigned char* )mmIOinfo.pchNext    );  //  ...Let the later functions cast this if they need to   6/08

			packetBuffer[ packetIdx ] =    byVal;




			if(   doResampling    )   
			{

				if(    packetIdx  ==    packetSize -1   )
				{
					if(   ! Process_Sample_Packet(    packetBuffer,  packetSize,  bufSampleIdx,  blocksSampleCount,   sampleArrayLeft, 
																	 sampleArrayRight,   retProcessBuffer,   retErrorMesg  )   )
					{
						End_ProgressBar_Position_GLB();
						return  false;
					}

					bufSampleIdx++;
				}


				if(   retProcessBuffer   )
				{

					int        ylenLeft =  -1,          ylenRight =   -1;             //  Resample_Signal  will return this value
					float   *yptptLeft =   NULL,   *yptptRight = NULL;        //  Resample_Signal  will alloc memory for this return sample.

					bool    lessThanOneValues =  false;
					bool   noMemoryRelease  =   false; 

					double  volumeTweak  =   1.0;    

					
					
					reSampler.Resample_Signal(       sampleArrayLeft,     blocksSampleCount,     &yptptLeft,    &ylenLeft  );


					if(    m_channelsCountWAVsrc  ==  2   )    //  2:  Only for Stereo 
						reSampler.Resample_Signal(   sampleArrayRight,    blocksSampleCount,   &yptptRight,    &ylenRight   );
					



					if(    ! SndSample::Merge_Float_Channels_To_Sample(   true,    ylenLeft,     bytesPerSampleDST,    &yptptLeft,    &yptptRight,   
																				                          &dstSamplesPtr,     retByteCount,    lessThanOneValues,   noMemoryRelease,  
																										             volumeTweak,  retErrorMesg   )   )
					{
							// ******  INSTALL other cleanup ????  *************************************
						End_ProgressBar_Position_GLB();
						return  false;
					}				
				



					wavDSTfile.Write(   dstSamplesPtr,    retByteCount   );
					bufSampleIdx =   0;

					double  progressPercent  =     (   (double)byteIdx  /  (double)mmChunkInfo.cksize   )   *  100.0;
					Set_ProgressBar_Position_GLB(    (long)progressPercent   );
				}
			}   
			else   
			{							                                          //   NO resampling,  just arrange bytes
				if(    packetIdx  ==    packetSize -1   )
				{
					if(    ! Process_Sample_Packet_NoResamp(    packetBuffer,    packetSize,     bufSampleIdx,    origBufSize,  
																		             (unsigned char*)dstSamplesPtr,    retProcessBuffer,   retErrorMesg   )    )
					{  End_ProgressBar_Position_GLB();
						return  false;
					}
										
					bufSampleIdx++;
				}



				if(    retProcessBuffer    )
				{
					wavDSTfile.Write(   dstSamplesPtr,    origBufSize   );
					bufSampleIdx =   0;

					double  progressPercent  =     (   (double)byteIdx  /  (double)mmChunkInfo.cksize   )   *  100.0;
					Set_ProgressBar_Position_GLB(    (long)progressPercent   );
				}


				if(       onLastBlock 
					&&  bufSampleIdx  > 0    )						//  write out only part of the LAST block
				{
					wavDSTfile.Write(   dstSamplesPtr,    bufSampleIdx   );
				}
			}


			
			packetIdx++;
			if(   packetIdx  >=   packetSize  )
				packetIdx =  0;
				
			( BYTE* )mmIOinfo.pchNext++;    	
		}   //   for(   long byteIdx =0;      byteIdx <   mmChunkInfo.cksize; 



		End_ProgressBar_Position_GLB();




					//   mmioSetInfo() :    updates the information retrieved by the mmioGetInfo function about a file opened 
					//                              by using the mmioOpen function. Use this function to terminate direct buffer access 
					//							    of a file opened for buffered I/O.

		  if(    (  errorCode =     mmioSetInfo(   mmIO,    &mmIOinfo,    0  )   )      !=  0    )
		  {			  
			 retErrorMesg =   "Convert_WAV_to_WAV  failed,  mmioSetInfo." ;
			 return  false;
		  }

																		//   close all files  
		Write_WAVfiles_Length(  wavDSTfile   );
		wavDSTfile.Close();
	}
	catch(   CFileException   *pException   )
	{
		TCHAR    szCause[ 255 ];  
		CString   strRaw;
		pException->GetErrorMessage(  szCause,  255  );
		pException->Delete();
		strRaw =  szCause;
		End_ProgressBar_Position_GLB();
		retErrorMesg.Format(   "Convert_WAV_to_WAV  failed,  could not write new .WAV file because:  %s." ,   strRaw  );
		return  false;
	}  



																		//   Cleanup

	if(    wavFormat   !=   NULL     )
	{  GlobalFree(  wavFormat   );	      wavFormat =  NULL;    }

	if(    mmIO  !=   NULL   )    
	{  mmioClose(   mmIO,   0  );		  mmIO =  NULL;          }	       



	if(   sampleArrayLeft  !=  NULL   )
	{
		free(  sampleArrayLeft  );     sampleArrayLeft =  NULL;   
	}

	if(   sampleArrayRight  !=  NULL   )
	{
		free(  sampleArrayRight  );     sampleArrayRight =  NULL;   
	}

	if(   dstSamplesPtr  !=  NULL   )
	{
		free(  dstSamplesPtr  );     dstSamplesPtr =  NULL;   
	}
	
	return true;
}




							/////////////////////////////////////



bool    WavConvert::Process_Sample_Packet_NoResamp(   unsigned char  packetBuffer[],     long  packetSize,  
										                                               long  bufSampleIdx,    long  bufferSize,   
														   unsigned char   *dstSamplesPtr,     bool&  retProcessBuffer,    CString&  retErrorMesg   )
{

	long      sampleSizeBytesDST =    m_defaultBytesPerSampleOutput;   //   4       4 bytes for   16bit stereo  DST sample  ( always for now,  8/06 )



	retProcessBuffer =   false;

	retErrorMesg.Empty();


	if(   dstSamplesPtr  ==   NULL   )
	{
		retErrorMesg =  "Process_Sample_Packet_NoResamp failed,  dstSamplesPtr is NULL. " ;
		return  false;
	}

	if(    bufSampleIdx  >=    ( bufferSize / sampleSizeBytesDST )     )   
	{
		retErrorMesg =  "Process_Sample_Packet_NoResamp failed,  index exceeds array size. " ;
		return  false;
	}



															//  INIT  if at first array element  ( but they will be rewritten anyway )
	if(   bufSampleIdx ==  0   )
	{
		for(  long i =0;    i <  bufferSize;     i++   )
			*dstSamplesPtr =    128;   //  should be Zero Amplitude
	}

	unsigned char  *dst =  NULL;



	


	if(         m_channelsCountWAVsrc  ==  1   )    //  1:  Mono
	{

		if(         m_bitsPerSampleWAVsrc  ==   16   )
		{

			dst  =         dstSamplesPtr   +    ( bufSampleIdx  *  sampleSizeBytesDST  );     //   4 DST bytes in a sample 

			*dst  =   packetBuffer[ 0 ];       dst++;
			*dst  =   packetBuffer[ 1 ];       dst++;
			*dst  =   packetBuffer[ 0 ];       dst++;
			*dst  =   packetBuffer[ 1 ];       dst++;
		}
		else if(   m_bitsPerSampleWAVsrc  ==    8    )
		{

			short	samp =    (    (short)packetBuffer[ 0 ]    -  128    );    //   *****  8bit Samples have a DIFFERENT bitFORMAT than   than 16bit   *******
		
			dst  =         dstSamplesPtr   +    ( bufSampleIdx  *  sampleSizeBytesDST  );   

			*dst  =        128;        dst++;      //  128:   Zero out low byte
			*dst  =      samp;       dst++;
			*dst  =        128;        dst++;
			*dst  =      samp;       dst++;
		}
		else
		{  retErrorMesg  =   "Process_Sample_Packet_NoResamp  failed,  missing BitsPerSample case for mono." ;
			return  false;
		}


		if(      bufSampleIdx    >=    ( bufferSize / sampleSizeBytesDST )  - 1  )
			retProcessBuffer =   true;
	} 

	else if(  m_channelsCountWAVsrc  ==  2   )    //  2:  Stereo
	{

		if(         m_bitsPerSampleWAVsrc  ==   16   )
		{  
			retErrorMesg  =   "Process_Sample_Packet_NoResamp,  no need to convert at 44.1 stereo at 16 bits." ;
			return  false;
		}
		else if(   m_bitsPerSampleWAVsrc  ==    8    )     //   *****  8bit Samples have a DIFFERENT bitFORMAT than   than 16bit   *******
		{

			unsigned char   eraseVal  =    128; 

			short	samp0 =    (    (short)packetBuffer[ 0 ]    -  128    );    
			short	samp1 =    (    (short)packetBuffer[ 1 ]    -  128    );    

			
			dst  =         dstSamplesPtr   +    ( bufSampleIdx  *  sampleSizeBytesDST  );   

			*dst  =        128;         
			dst++;     

			*dst  =      samp0;       
			dst++;

			*dst  =        128;         
			dst++;

			*dst  =      samp1;       
			dst++;
		}
		else
		{  retErrorMesg  =   "Process_Sample_Packet_NoResamp  failed,  missing BitsPerSample case for stereo." ;
			return  false;
		}


		if(      bufSampleIdx    >=    ( bufferSize / sampleSizeBytesDST )  - 1  )
			retProcessBuffer =   true;
	}

	else
	{  retErrorMesg  =   "Process_Sample_Packet_NoResamp  failed,  missing channel case." ;
		return  false;
	}


	return  true;
}




					/////////////////////////////////////////////


bool    WavConvert::Process_Sample_Packet(    unsigned char  packetBuffer[],   long  packetSize,   long  bufSampleIdx,
										                              long  bufferSize,    float *retSampleArrayLeft,    float *retSampleArrayRight,   
																	  bool&  retProcessBuffer,    CString&  retErrorMesg   )
{

	retProcessBuffer =   false;

	retErrorMesg.Empty();

	/***
	unsigned short        m_formatTypeWAVsrc;            //   format type 
	short                      m_channelsCountWAVsrc;       //   number of channels (i.e. mono, stereo...) 
	short                      m_bitsPerSampleWAVsrc;        //   number of bits per sample of mono data 
	long                       m_sampleRateWAVsrc;             //   sample rate 
	****/

	if(   retSampleArrayLeft  ==   NULL   )
	{
		retErrorMesg =  "Process_Sample_Packet failed,  retSampleArrayLeft is NULL. " ;
		return  false;
	}


	if(        retSampleArrayRight   ==   NULL 
		&&   m_channelsCountWAVsrc  !=  1   )
	{
		retErrorMesg =  "Process_Sample_Packet failed,  retSampleArrayRight is NULL for stereo. " ;
		return  false;
	}


	if(   bufSampleIdx  >=   bufferSize   )
	{
		retErrorMesg =  "Process_Sample_Packet failed,  index exceeds array size. " ;
		return  false;
	}



															//  INIT  if at first array element  
	if(   bufSampleIdx ==  0   )
	{
		for(  long i =0;    i <  bufferSize;     i++   )
			retSampleArrayLeft[  i  ] =   0.0;


		if(   retSampleArrayRight   !=   NULL   )
		{
			for(  long  i =0;    i <  bufferSize;     i++   )
				retSampleArrayRight[  i  ] =   0.0;
		}
	}


	


	if(    m_channelsCountWAVsrc  ==  2    )     //   2: Stereo  
	{

		if(    m_bitsPerSampleWAVsrc  ==   16    )
		{

		//	short   bytHi   =    ((short)pac1)   <<   8;
		//	short   bytLo  =     (short)pac0;
			short   bytHi   =    (  (short)packetBuffer[ 1 ]  )   <<   8;
			short   bytLo  =        (short)packetBuffer[ 0 ];


			short	signedSampleVal =    bytHi    +    bytLo;

			retSampleArrayLeft[    bufSampleIdx  ]  =     signedSampleVal;



	//		bytHi   =    ((short)pac3)   <<   8;
	//		bytLo  =     (short)pac2;
			bytHi   =    (  (short)packetBuffer[ 3 ]  )   <<   8;
			bytLo  =        (short)packetBuffer[ 2 ];


			signedSampleVal =    bytHi    +    bytLo;

			retSampleArrayRight[  bufSampleIdx  ]  =     signedSampleVal;
		}

		else if(    m_bitsPerSampleWAVsrc  ==   8    )
		{

				//   8-bit waveforms are   'unsigned char' [ 0 -255 ]   with  128 as the zero value( just cast to char ??  )

			short     samp[ 2 ];
			for(   short i=0;   i< 2;    i++  )
			{
				short   tp  =     packetBuffer[ i ];

				samp[ i ]  =    (  tp  -  128  );      //   128 [ YES!!  8/06 ]  
										     //  Writing 128 to dstWAV with a pointer to 'unsigned char' creates Zero amplitude
			}

			/***  Casting will not get this into proper form

			unsigned char   bytOrig  =   packetBuffer[ 1 ];
			char  byt        =                 packetBuffer[ 1 ];
			char  bytCast  =    (char)(   packetBuffer[ 1 ]   );
			***/


			short	signedSampleValLeft    =     samp[ 0 ]    <<   8;   //  << 8:   simulate the volume level of a 16bit sample
			short	signedSampleValRight  =     samp[ 1 ]    <<   8;

			retSampleArrayLeft[    bufSampleIdx  ]  =    signedSampleValLeft;
			retSampleArrayRight[  bufSampleIdx  ]  =    signedSampleValRight;
		}
		else
		{ ASSERT( 0 );
		   retErrorMesg =    "Process_Sample_Packet  failed,  missing BitsPerSample case for stereo." ;   
		}
	}

	else  if(    m_channelsCountWAVsrc  ==  1    )     //   1:  Mono
	{

		if(    m_bitsPerSampleWAVsrc  ==   16    )
		{

			short   bytHi   =    (  (short)packetBuffer[ 1 ]  )   <<   8;
			short   bytLo  =        (short)packetBuffer[ 0 ];

			short	signedSampleVal =    bytHi    +    bytLo;

			retSampleArrayLeft[    bufSampleIdx  ]  =     signedSampleVal;
		}
		else if(    m_bitsPerSampleWAVsrc  ==   8    )
		{

			short   tp       =     packetBuffer[ 0 ];

			short   samp  =    (  tp  -  128  );    

			short	signedSampleValLeft    =     samp   <<   8;       //  << 8:   simulate the volume level of a 16bit sample

			retSampleArrayLeft[    bufSampleIdx  ]  =    signedSampleValLeft;   // No  retSampleArrayRight[]  for Mono
		}
		else
		{ ASSERT( 0 );
		   retErrorMesg =    "Process_Sample_Packet  failed,  missing BitsPerSample case for mono." ;   
		}

	}   //  Mono

	else
	{  ASSERT( 0 );
		retErrorMesg =    "Process_Sample_Packet  failed,  missing channel case." ;   
	}



	if(     bufSampleIdx   ==    ( bufferSize -1 )     )
		retProcessBuffer =    true;   //  tell calling function to now process the FULL array of data

	return  true;
}






					/////////////////////////////////////////////


bool     WavConvert::Create_Test_WAV_File(    CString&  filePathDST,    CString&  retErrorMesg   )
{


	int   do16Bit         =   1;    //   0   **TOGGLE**    0:  8bit samples    1:  16bitsamples

	int   doStereoDST =   1;

	int    dstSampRate =   22000;     //  44100;



	retErrorMesg.Empty();

	if(   filePathDST.IsEmpty()   )
	{
		retErrorMesg =   "Create_Test_WAV_File failed,   filePathDST is empty."  ;
		return  false;
	}




		//   Creates test sine wave.    ***** Maybe good to test for  'HISS bug'   8/06  ****************



#define  twoPI  6.283185307179586


#define  DSTbyteCNT  2000
//	char                  dstWav[    2000  ];    // ******  OR   unsigned char  ?????
	unsigned   char   dstWav[    2000  ];    


	double  freq =    880;





	long  numSamples;

	if(   do16Bit   )
		numSamples =   DSTbyteCNT  /  4;
	else
		numSamples =   DSTbyteCNT  /  2;



	double  angFreq  =      freq   /   (double)(  dstSampRate  );   
	short    val;

//	char                  *dstPtrSigned    =         ( char* )(     &(  dstWav[ 0 ]   )    );    
	unsigned char    *dstPtrUnsigned  =                           &(  dstWav[ 0 ]   );  




	for(    long  s = 0L;     s <  numSamples;    s++   )
	{

		double  ang  =      (double)s    *  angFreq    *   twoPI;



		if(   do16Bit   )
		{


			float    sinComp   =      (float)(     sin( ang  )  *   32385.0     );     

			short   sinCmpShort   =    (short)sinComp;
			

			short   hiByte,   loByte,   hiBAD,  loBAD;



			/*****
			short   hiByte    =    sinCmpShort  >>  8;   //  *** BEST ***

			short   hiByteX  =    sinCmpShort  / 256;  //  inaccureate with some rounding 


			short   loByte  =    sinCmpShort  &  0x00FF;    //  *** BEST ***

			short   loByteX    =    sinCmpShort  %  256;   //  wrong,  give a negative value sometimes 
			****/


			if(   sinCmpShort  >=  0   )
			{
				hiByte  =    sinCmpShort  >>  8; 
				loByte  =    sinCmpShort   &  0x00FF; 


				hiBAD  =    sinCmpShort  >>  8; 
				loBAD  =    sinCmpShort   &  0x00FF; 
			}
			else
			{  short   tp  =   -1 *  sinCmpShort;   // need positive sign for the bit shifting and mask to work right

				hiByte  =    (  tp  >>  8  )   *  -1;     //  -1:  will preserve the sign for the pair
				loByte  =       tp    &  0x00FF; 

// **** WIERD, even though the numbers are different, it seems like they creat identical WAVes in WaveLab Lite

				hiBAD  =    sinCmpShort  >>  8; 
				loBAD  =    sinCmpShort   &  0x00FF; 
			}




			*dstPtrUnsigned =       loByte;          dstPtrUnsigned++;    //    LO       left     ****  BEST combo  *****

			*dstPtrUnsigned =       hiByte;          dstPtrUnsigned++;    //    HI



			*dstPtrUnsigned =        loBAD;         dstPtrUnsigned++;      //   LO		 right   (  show BAD writes )))     

			*dstPtrUnsigned =        hiBAD;         dstPtrUnsigned++;      //    HI





			/****
			*dstPtrUnsigned =          0;        dstPtrUnsigned++;    //    LO       left   

			*dstPtrUnsigned =          1;        dstPtrUnsigned++;    //    HI


			*dstPtrUnsigned =         255;       dstPtrUnsigned++;     //   LO		 right        

			*dstPtrUnsigned =             0;      dstPtrUnsigned++;      //    HI
			****/




		/****    NOTES:   ( sign is determined by High bit

		1)  Sign is determined by the HI-byte
		2)  All eigh tbits of the low byte are part of digital decimal


			Zero in both  makes a zero signal
			Zero in HI bit, and 30 in LO  give small positive 

			-30  in HI byte  makes  negative  signal 


			Only  127 in HI  cakes almost a total peak( or valley with -127  )   


			-127  in  LO( 0 in HI )  still makes a Positive signal     0.4
			-127  in both bytes give a negative valley (   -98.0  )


			-255  in just  LO,  give a zero signal   *************
			-127 in HI,  and   +127   in LO  gives negative (  -98.8,   deeper valley that -127 in Lo  )


			-220  in LO( o in HI )  give a lower valuePeak )  than  -180  ( *** WIERD *** )

			255 in LO( .8 )  gives bigger peak than  180 in LO

	255 in Lo    OR...    1in HI  gives almost same value   ( ****  BIG ***** )

		****/

		}
		else
		{
			float    sinComp   =      (float)(     sin( ang  )  *   127.0     );     

			val  =    (short)(   sinComp    +   128.0  );	 //   128   in hex is   0x0080

			if(    val   >  255   )
				val  =    255;
			else if(  val < 0  )
				val  =        0;


			*dstPtrUnsigned =     val;      dstPtrUnsigned++;     //  left
			*dstPtrUnsigned =    128;      dstPtrUnsigned++;     //  right     128 is true zero( verified with WaveLabLite magup  )
		}
	}





	try   
	{  CFile   wavDSTfile(     filePathDST,  
										 CFile::modeCreate    |  CFile::modeWrite
									 //  CFile::modeCreate    |  CFile::modeWrite     |   CFile::modeNoTruncate
								 );				  // ***NOTE:   'modeNoTruncate'   allows APPEND to existing fie,  does not EMPTY/ERASE an EXISTING file

		Write_WAV_Header(   wavDSTfile,   dstSampRate,   doStereoDST,   do16Bit,  0   );   // **** CAREFUL,  freq is REPLACED



		wavDSTfile.Write(   dstWav,    DSTbyteCNT   );
		

																		//   close all files  
		Write_WAVfiles_Length(  wavDSTfile   );

		wavDSTfile.Close();
	}
	catch(   CFileException   *pException   )
	{
		TCHAR    szCause[ 255 ];  
		CString   strRaw;
		pException->GetErrorMessage(  szCause,  255  );
		pException->Delete();
		strRaw =  szCause;
		retErrorMesg.Format(   "Create_Test_WAV_File  failed,  could not write new .WAV file because:  %s." ,   strRaw  );
		return  false;
	}  

	return  true;
}




/*******************  ORIGINAL VERSION from    December 2011


bool     WavConvert::Shift_Pitch(  bool  rightStereo,   float pitchShift,     long numSampsToProcess,    long fftFrameSize,     long osamp,   float sampleRate,
																	                    float *indata,   float *outdata,      CString&   retErrorMesg  )   
{


//  9/11  -         write another version ( _DEBUG )  with  extra variables for better clarity.  
//
// 	                   Try to figure out exactly WHAT this FUNCTION DOES and think how I could
//				                it's techniues to do other manipulation:    (check old experimental sound manupulation techniques)
//		 1)  Get good  Re-synthesis  by  SUBTRACTING  out  VOICE?? 
//      2)   Do clean  Re-synthesis  for  log-DFT ??
//      3)  Compaire   true  phase/freq  over  STEREO-channels  to find the  VOICE component (in center )  that I could subtract out.




//	The routine takes a  'pitchShift'  factor value which is between 0.5  (one octave down) and 2. (one octave up). 
//	A value of exactly 1 does not change the pitch.       [  ****** OK to use weird decimal values???  Not need to be integers??  9/11  *******   ]



//	numSampsToProcess  -     tells the routine how many samples in  indata[0...  numSampsToProcess-1]   should be pitch shifted 
//										  and moved to  outdata[0 ... numSampsToProcess-1]. 


//  The two buffers can be identical  (ie. it can process the  data in-place). 


//   fftFrameSize -    defines the FFT frame size used for the processing. Typical values are 1024, 2048 and 4096. 
//						It may be any   value <=  MAX_FRAME_LENGTH   but it MUST be a power of 2. 

//   osamp -  is the STFT  oversampling factor which also determines the overlap between adjacent STFT  frames.
 //             It should at least be 4 for moderate scaling ratios. A value of 32 is recommended for best quality. 

//	sampleRate -    takes the sample rate for the signal  in unit Hz, ie. 44100 for 44.1 kHz audio. 


//	The data passed to the routine in  indata[] should be in the range [ -1.0, 1.0 ],  which is also the output range 
 //	for the data, make sure you scale the data accordingly ( for 16bit signed integers you would have to divide (and multiply) by 32768 ). 


	retErrorMesg.Empty();

	static long   gInit =  false;


	static float  gInFIFOLeft[    MAX_FRAME_LENGTH  ];
	static float  gOutFIFOLeft[  MAX_FRAME_LENGTH  ];
	static float  gOutputAccumLeft[  2 * MAX_FRAME_LENGTH  ];


	static float  gInFIFORight[    MAX_FRAME_LENGTH  ];
	static float  gOutFIFORight[  MAX_FRAME_LENGTH  ];
	static float  gOutputAccumRight[  2 * MAX_FRAME_LENGTH  ];


	static float gLastPhaseLeft[  MAX_FRAME_LENGTH/2+1  ];    
	static float gLastPhaseRight[  MAX_FRAME_LENGTH/2+1  ];   


	static float gSumPhaseLeft[  MAX_FRAME_LENGTH/2+1  ];
	static float gSumPhaseRight[  MAX_FRAME_LENGTH/2+1  ];


	 float gAnaFreq[   MAX_FRAME_LENGTH  ];   //  do NOT think that these need to be static
	 float gAnaMagn[  MAX_FRAME_LENGTH  ];

	 float gSynFreq[   MAX_FRAME_LENGTH  ];
	 float gSynMagn[  MAX_FRAME_LENGTH  ];


	static long  gRoverLeft   =   0;   //  do i NOT need this initialization??  will it create trouble ????
	static long  gRoverRight =   0;

	long      gRover = 0;

	 float  gFFTworksp[   2 *  MAX_FRAME_LENGTH  ];    //  this is  2 times a big as  FFT-size,  beause it hold BOTH  the real and imaginary parts of the FFT


	double   magn, phase, tmp, window, real, imag;
	double   freqPerBin, expct;
	long      i,k, qpd, index, inFifoLatency, stepSize, fftFrameSize2;


																		// set up some handy variables
	fftFrameSize2 =    fftFrameSize /2;

	stepSize         =    fftFrameSize  /  osamp;       //   as osamp  gets bigger,    stepSize gets smaller

	freqPerBin       =   sampleRate  /  (double)fftFrameSize;

	expct              =    2.*M_PI * (double)stepSize / (double)fftFrameSize;

	inFifoLatency   =    fftFrameSize  -  stepSize;


//	if(   gRover ==  false  )      //  is this a one time initialization ???
//		gRover =   inFifoLatency;


																	// initialize our static arrays 
	if(  gInit  ==   false  )  
	{
		memset(  gInFIFOLeft,           0,   MAX_FRAME_LENGTH*sizeof(float));
		memset(  gOutFIFOLeft,         0,   MAX_FRAME_LENGTH*sizeof(float));
		memset(  gOutputAccumLeft,  0, 2*MAX_FRAME_LENGTH*sizeof(float));

		memset(  gInFIFORight,           0,   MAX_FRAME_LENGTH*sizeof(float));
		memset(  gOutFIFORight,         0,   MAX_FRAME_LENGTH*sizeof(float));
		memset(  gOutputAccumRight, 0, 2*MAX_FRAME_LENGTH*sizeof(float));


		memset(  gLastPhaseLeft,        0,   (MAX_FRAME_LENGTH/2+1)*sizeof(float));
		memset(  gLastPhaseRight,        0,   (MAX_FRAME_LENGTH/2+1)*sizeof(float));

		memset(  gSumPhaseLeft,       0,   (MAX_FRAME_LENGTH/2+1)*sizeof(float));
		memset(  gSumPhaseRight,       0,   (MAX_FRAME_LENGTH/2+1)*sizeof(float));

	
	//	memset(  gAnaFreq,          0,   MAX_FRAME_LENGTH*sizeof(float));
	//	memset(  gAnaMagn,         0,   MAX_FRAME_LENGTH*sizeof(float));
	//	memset(  gSynFreq,          0,   MAX_FRAME_LENGTH*sizeof(float));
	//	memset(  gSynMagn,         0,   MAX_FRAME_LENGTH*sizeof(float));


		memset(  gFFTworksp,     0, 2*MAX_FRAME_LENGTH*sizeof(float));


		gRoverLeft   =   gRoverRight =  gRover =    inFifoLatency;


		gInit =    true;      //  this is a little controversial,  but seems to work OK
	}



	memset(  gFFTworksp,           0, 2*MAX_FRAME_LENGTH*sizeof(float)  ); 



	float    *gInFIFO=NULL,   *gOutFIFO=NULL,  *gOutputAccum=NULL,     *gLastPhase=NULL,   *gSumPhase=NULL ; 

	if(   rightStereo   )     //  save values for next entrance to this function,  or we will hear 'static'  from variable confusion from switching from left to right stereo
	{
		gInFIFO       =     gInFIFORight;
		gOutFIFO      =    gOutFIFORight;
		gOutputAccum =    gOutputAccumRight;  

		gLastPhase  =      gLastPhaseRight;
		gSumPhase =      gSumPhaseRight;

		gRover =   gRoverRight;
	}
	else
	{  gInFIFO      =     gInFIFOLeft;
		gOutFIFO    =     gOutFIFOLeft;
		gOutputAccum =    gOutputAccumLeft;  

		gLastPhase  =      gLastPhaseLeft;
		gSumPhase =      gSumPhaseLeft;

		gRover =   gRoverLeft;
	}






																										// main processing loop 
	for(   i = 0;    i < numSampsToProcess;    i++  )
	{

																		// As long as we have not yet collected enough data just read in 
		gInFIFO[ gRover ]  =   indata[ i ];

		outdata[ i ]            =   gOutFIFO[  gRover  -  inFifoLatency  ];

		gRover++;


																		
		if(   gRover  >=  fftFrameSize   )     // now we have enough data for processing 
		{

			gRover =   inFifoLatency;    //  re-initialization of this TRAVERSING index



			memset(  gFFTworksp,    0,   2 * MAX_FRAME_LENGTH*sizeof(float)  );

																			// do windowing and re,im interleave
			for (  k = 0;   k < fftFrameSize;    k++) 
			{
				window =    -.5 * cos(   2.*M_PI * (double)k   /  (double)fftFrameSize   ) + .5;     //  Do I need windowing ?????

				gFFTworksp[ 2*k     ]   =   gInFIFO[ k ]  * window;    //   FFT  inputs  are take  from the   gInFIFO[]   circularQue-delay
				gFFTworksp[ 2*k +1]   =   0.;
			}


																									//  ****************** ANALYSIS *******************

																	
			smbFft(   gFFTworksp,   fftFrameSize,   -1   );    //        do  FFT transform 




			memset(  gAnaFreq,          0,   MAX_FRAME_LENGTH*sizeof(float));
			memset(  gAnaMagn,         0,   MAX_FRAME_LENGTH*sizeof(float));
			
																								
			for(   k = 0;   k <=   fftFrameSize2;    k++   )    // this is the analysis step 
			{

				
				real   =   gFFTworksp[ 2*k      ];			// de-interlace FFT buffer 
				imag =   gFFTworksp[ 2*k +1 ];

				
				magn =    2. *  sqrt(  real*real  +   imag*imag  );    // compute magnitude and phase 
				phase =   atan2(  imag, real );

			
				tmp =   phase  -  gLastPhase[k];    	// compute phase difference 

				gLastPhase[ k ] =   phase;


				
				tmp -=   (double)k * expct;       // subtract expected phase difference 


				//   ***********************   can use this code substitution to verify that the Phase Unwraping works for    pitchShift > 2.0       2/10

//				float   deltaPhaseOrig,    deltaPhaseUnwrap;
//				float   tolerance =  0.0001;

//				deltaPhaseOrig  =    tmp;
															
//				qpd  =   tmp / M_PI;        //  map delta phase into  + / - Pi  interval 

//				if(   qpd >= 0   ) 
//					qpd  +=  qpd&1;
//				else 
//					qpd  -=   qpd&1;


//			//  NOT un-comment           	tmp -=    M_PI * (double)qpd;          tmp =  tmp -    M_PI * (double)qpd; 

//				deltaPhaseUnwrap  =    tmp  -   M_PI * (double)qpd;

//				if(   deltaPhaseUnwrap >  ( M_PI + tolerance   )   ||    deltaPhaseUnwrap <   -(M_PI + tolerance)   )
//				{
//					int  dummy =  9;
//				}

//				tmp  =    deltaPhaseUnwrap;
			
				



				qpd =    tmp  /  M_PI;          //  map delta phase into  + / - Pi  interval    ( these 5 lines of code )

				if(  qpd  >=  0  ) 
					qpd  +=   qpd & 1;
				else 
					qpd  -=    qpd & 1;


				tmp  -=   M_PI * (double)qpd;       
				

				
				tmp =   osamp * tmp   /   (2. * M_PI);    // get deviation from bin frequency from the +/- Pi interval

				
				tmp =    (double)k * freqPerBin    +     tmp * freqPerBin;    // compute the  k-th partials'  true frequency

													

				gAnaMagn[ k ]  =   magn;       //  store magnitude and true frequency in analysis arrays
				gAnaFreq[  k ]  =   tmp;
			}


																								//    PROCESSING 
		

			memset(  gSynMagn,  0,   fftFrameSize * sizeof(float)   );    
			memset(  gSynFreq,    0,   fftFrameSize * sizeof(float)   );


			for (  k = 0;    k  <=  fftFrameSize2;    k++  )       // this does the actual pitch shifting 
			{ 
				index =   k * pitchShift;				//  going to VERTICALLY  shift the  Mag/Phase  values  to other   FFT-frequencyRows, changes the pitch

				if(  index  <=   fftFrameSize2  ) 
				{ 
					gSynMagn[index]  +=   gAnaMagn[k]; 

					gSynFreq[index]    =    gAnaFreq[k] * pitchShift; 
				} 
			}

			
																								//    SYNTHESIS 

			for(    k = 0;     k <=   fftFrameSize2;      k++   ) 
			{
																
				magn =    gSynMagn[ k ];      // get magnitude and true frequency from synthesis arrays 
				tmp   =    gSynFreq[  k ];

																
				tmp -=   (double)k  *  freqPerBin;     // subtract bin  mid frequency 

															
				tmp  /=  freqPerBin;    // get bin deviation from freq deviation 

															
				tmp =    2.  *  M_PI  *  tmp/osamp;    // take osamp into account 

															
				tmp   +=    (double)k  *  expct;     // add the overlap phase advance back in 

														
				gSumPhase[ k ]   +=   tmp;     // accumulate delta phase to get bin phase 

				phase =    gSumPhase[ k ];

																			
				gFFTworksp[ 2 * k      ]   =    magn  *  cos( phase );     // get real and imag part and re-interleave
				gFFTworksp[ 2 * k +1 ]   =    magn  *  sin( phase );
			} 




																						// zero negative frequencies 
			for (  k =  fftFrameSize +2;     k < (2 * fftFrameSize);       k++ ) 
				gFFTworksp[ k ] =   0.;


																						
			smbFft(   gFFTworksp,   fftFrameSize,   1   );   // ********   do   INVERSE  FFT  *******   



																// do windowing and 'ADD'  to output accumulator ( this is the 'ADD' part to  "OverlapAndAdd"  )  
			for(  k=0;   k < fftFrameSize;   k++  ) 
			{
				window                   =      -.5*cos(  2.*M_PI*(double)k  /  (double)fftFrameSize  )   +.5;

				gOutputAccum[k]  +=       2.  *  window  *  gFFTworksp[ 2*k ]  /  ( fftFrameSize2 * osamp );
			}


			for (  k = 0;     k < stepSize;    k++  ) 
				gOutFIFO[ k ] =   gOutputAccum[ k ];

																														
			memmove(  gOutputAccum,    gOutputAccum + stepSize,   fftFrameSize  *  sizeof(float)   );    // shift accumulator 

			
			for( k = 0;   k < inFifoLatency;   k++ )       // move input FIFO 
				gInFIFO[ k ] =    gInFIFO[ k + stepSize ];
		}
	}


	
	if(   rightStereo   )				         //   save values for next entrance to this function,  or we will hear audio 'static'  from variable confusion 
		gRoverRight =     gRover;       //   from switching from left to right stereo during realtime play
	else
		gRoverLeft   =     gRover;


	return   true;
}
************/

/////////////////////////////////////////////////////////////////////////////




											////////////////////////////////////////

/*****   Crappy DEBUG function,  but it takes lots of memory because of STATIC arrays

bool     WavConvert::Shift_Pitch_Stripped(   bool  rightStereo,    long numSampsToProcess,    long fftFrameSizeSRC,     long osamp,   float sampleRate,
																	          float *indata,   float *outdata,    CString&   retErrorMesg  )   
{



//	numSampsToProcess  -     tells the routine how many samples in  indata[0...  numSampsToProcess-1]   should be pitch shifted 
//										  and moved to  outdata[0 ... numSampsToProcess-1]. 


//The two buffers can be identical  (ie. it can process the  data in-place). 


//fftFrameSizeSRC -    defines the FFT frame size used for the processing. Typical values are 1024, 2048 and 4096. 
//						It may be any   value <=  MAX_FRAME_LENGTH   but it MUST be a power of 2. 

//osamp -  is the STFT  oversampling factor which also determines the overlap between adjacent STFT  frames.
 //             It should at least be 4 for moderate scaling ratios. A value of 32 is recommended for best quality. 

//sampleRate -    takes the sample rate for the signal  in unit Hz, ie. 44100 for 44.1 kHz audio. 


//The data passed to the routine in  indata[] should be in the range [ -1.0, 1.0 ],  which is also the output range 
 //for the data, make sure you scale the data accordingly ( for 16bit signed integers you would have to divide (and multiply) by 32768 ). 



	retErrorMesg.Empty();


	static long   gInit =  false;


	static float  gInFIFOLeft[    MAX_FRAME_LENGTH  ];
	static float  gOutFIFOLeft[  MAX_FRAME_LENGTH  ];
	static float  gOutputAccumLeft[  2 * MAX_FRAME_LENGTH  ];


	static float  gInFIFORight[    MAX_FRAME_LENGTH  ];
	static float  gOutFIFORight[  MAX_FRAME_LENGTH  ];
	static float  gOutputAccumRight[  2 * MAX_FRAME_LENGTH  ];


	static long  gRoverLeft   =   0;   //  do i NOT need this initialization??  will it create trouble ????
	static long  gRoverRight =   0;


	long      gRover = 0;

//  Why did he make the   gFFTworksp   static?   It always gets reinitialized

//	static float  gFFTworkspLeft[     2 *  MAX_FRAME_LENGTH  ];    //  this is  2 times a big as  FFT-size,  beause it hold BOTH  the real and imaginary parts of the FFT
//	static float  gFFTworkspRight[   2 *  MAX_FRAME_LENGTH  ];    //  this is  2 times a big as  FFT-size,  beause it hold BOTH  the real and imaginary parts of the FFT

//	 float  gFFTworkspLeft[     2 *  MAX_FRAME_LENGTH  ];    //  this is  2 times a big as  FFT-size,  beause it hold BOTH  the real and imaginary parts of the FFT


	 float  gFFTworksp[        2 *  MAX_FRAME_LENGTH  ];    //  this is  2 times a big as  FFT-size,  beause it hold BOTH  the real and imaginary parts of the FFT

	float   gFFTworkspDST[   2 *  MAX_FRAME_LENGTH  ];  




	long       fftFrameSizeDST        =     m_playspeedSlowDownRatio   *   fftFrameSizeSRC;
	long       halfFftFrameSizeDST  =     fftFrameSizeDST /  2;


	double   window,  real, imag;
	long       i, k,    inFifoLatency,   stepSize,   halfFftFrameSizeSRC,   dstIndex;


	halfFftFrameSizeSRC =    fftFrameSizeSRC  / 2;

	stepSize                   =    fftFrameSizeSRC  /  osamp;       //   as osamp  gets bigger,    stepSize gets smaller

	inFifoLatency =    fftFrameSizeSRC  -  stepSize;

	long   stepSizeDST  =    m_playspeedSlowDownRatio *  stepSize;



//	          done better  BELOW in the init area ???

//	if(   gRover ==  false  )      //  is this a one time initialization ???    ******  Also need this for the 
//		gRover =   inFifoLatency;
	





																	// initialize our static arrays 
	if(  gInit  ==   false  )  
	{
		memset(  gInFIFOLeft,           0,   MAX_FRAME_LENGTH*sizeof(float));
		memset(  gOutFIFOLeft,         0,   MAX_FRAME_LENGTH*sizeof(float));
		memset(  gOutputAccumLeft,  0, 2*MAX_FRAME_LENGTH*sizeof(float));

		memset(  gInFIFORight,           0,   MAX_FRAME_LENGTH*sizeof(float));
		memset(  gOutFIFORight,         0,   MAX_FRAME_LENGTH*sizeof(float));
		memset(  gOutputAccumRight, 0, 2*MAX_FRAME_LENGTH*sizeof(float));


//		memset(  gFFTworkspLeft,     0, 2*MAX_FRAME_LENGTH*sizeof(float));
//		memset(  gFFTworkspRight,     0, 2*MAX_FRAME_LENGTH*sizeof(float));
		memset(  gFFTworksp,     0, 2*MAX_FRAME_LENGTH*sizeof(float));


//		gRoverLeft   =   gRoverRight =  gRover =    0;
		gRoverLeft   =   gRoverRight =  gRover =    inFifoLatency;


		gInit =    true;      //  this is a little controversial,  but seems to work OK
	}



	memset(  gFFTworksp,           0, 2*MAX_FRAME_LENGTH*sizeof(float)  );   //  ********  WILL this stop the ppooing sound and claps ???   **********
	memset(  gFFTworkspDST,     0, 2*MAX_FRAME_LENGTH*sizeof(float)  );   //  ********  WILL this stop the ppooing sound and claps ???   **********







	float    *gInFIFO=NULL,   *gOutFIFO=NULL,  *gOutputAccum=NULL; 

	if(   rightStereo   )     //  save values for next entrance to this function,  or we will hear 'static'  from variable confusion from switching from left to right stereo
	{
		gInFIFO       =     gInFIFORight;
		gOutFIFO      =    gOutFIFORight;
//		gFFTworksp  =      gFFTworkspRight;
		gOutputAccum =    gOutputAccumRight;  

		gRover =   gRoverRight;
	}
	else
	{  gInFIFO      =     gInFIFOLeft;
		gOutFIFO    =     gOutFIFOLeft;
//		gFFTworksp =     gFFTworkspLeft;
		gOutputAccum =    gOutputAccumLeft;  

		gRover =   gRoverLeft;
	}



																										// main processing loop 
	for(   i = 0;    i < numSampsToProcess;    i++  )
	{

																		//  As long as we have not yet collected enough data, just read it in 
		gInFIFO[ gRover ]  =   indata[ i ];



//		outdata[ i ]            =   gOutFIFO[  gRover  -  inFifoLatency  ];

		dstIndex  =    i *  m_playspeedSlowDownRatio; 

		long   gOutFifoIndex  =     ( gRover  * m_playspeedSlowDownRatio )    -    ( inFifoLatency  *  m_playspeedSlowDownRatio );

		for(    long n =  0;    n < m_playspeedSlowDownRatio;    n++    )
		{
			outdata[  dstIndex  ]  =     gOutFIFO[  gOutFifoIndex ];

			dstIndex++;
			gOutFifoIndex++;
		}

		gRover++;



																		
		if(   gRover  >=  fftFrameSizeSRC   )     // now we have enough data for processing 
		{


			gRover =   inFifoLatency;    //  think this is a re-initialization of this TRAVERSING index

																			
			for (  k = 0;   k < fftFrameSizeSRC;    k++)     //  do windowing and re, im interleave 
			{
				window =    -.5 * cos(   2.*M_PI * (double)k   /  (double)fftFrameSizeSRC   ) + .5;     //  Do I need windowing ?????

				gFFTworksp[ 2*k     ]   =   gInFIFO[ k ]  * window;    //   FFT  inputs  are take  from the   gInFIFO[]   circularQue-delay
				gFFTworksp[ 2*k +1]   =   0.;
			}


					
																	
			smbFft(   gFFTworksp,   fftFrameSizeSRC,   -1   );    //   ****BIG    do the   FFT transform     (  ie  the ANALYSIS )




///////////////////////////////////  Process the FFT's  column of  values ( filtering, pitchChange,  etc  )    /////////////////


			for(   k = 0;   k <=   halfFftFrameSizeSRC;    k++   )    // this is the analysis step 
			{

				dstIndex  =     k *  m_playspeedSlowDownRatio;
				
				real    =    gFFTworksp[  2*k       ];			// de-interlace FFT buffer 
				imag  =    gFFTworksp[  2*k  +1  ];

				gFFTworkspDST[  2 * dstIndex         ]   =    real;     // get real and imag part and re-interleave
				gFFTworkspDST[  2 * dstIndex    +1 ]   =    imag;
			}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		




/////////////////////////////////////////////////////////////////////

			
			for (  k =  fftFrameSizeDST +2;     k < 2 * fftFrameSizeDST;       k++ )    // zero negative frequencies  ( Carefull if bigger than fftFrameSizeSRC  ) 
				gFFTworkspDST[ k ] =   0.0;


																						
			smbFft(   gFFTworkspDST,   fftFrameSizeDST,   1   );   //   ********   do  the INVERSE  FFT  *******   




																// do windowing and 'ADD'  to output accumulator ( this is the 'ADD' part to  "OverlapAndAdd"  )   

			for(  k=0;   k < fftFrameSizeDST;   k++  ) 
			{
				window                     =      -.5 * cos(  2.*M_PI*(double)k  /  (double)fftFrameSizeDST  )   +.5;     // **** CAREFUL,   is   DST  appropriate ????

				gOutputAccum[ k ]   +=       2.  *  window  *  gFFTworkspDST[ 2*k ]  /  ( halfFftFrameSizeDST * osamp );
			}
			



																			//   SHIFT the values in the static ARRAYS for next call of funct


																			//   SHIFT the values in the static ARRAYS for next call of funct
		//	for (  k = 0;     k < stepSize;    k++  ) 
		//		gOutFIFO[ k ] =   gOutputAccum[ k ];
			for (  k = 0;     k < stepSizeDST;    k++  ) 
				gOutFIFO[ k ] =   gOutputAccum[ k ];

																				

		//	memmove(  gOutputAccum,    gOutputAccum + stepSize,           fftFrameSizeSRC  *  sizeof(float)    );    // shift the accumulator 
			memmove(  gOutputAccum,    gOutputAccum + stepSizeDST,     fftFrameSizeDST * sizeof(float)    );    // shift the accumulator 



	//		for( k = 0;   k < inFifoLatency;   k++ )       // move  input FIFO    ...more shifting
	//			gInFIFO[ k ] =    gInFIFO[ k + stepSize ];
			for( k = 0;   k < inFifoLatency;   k++ )       // move  INPUT    Fifo    ...more shifting
				gInFIFO[ k ] =    gInFIFO[ k + stepSize ];

		}
	}



	if(   rightStereo   )				         //   save values for next entrance to this function,  or we will hear 'static'  from variable confusion 
		gRoverRight =     gRover;       //   from switching from left to right stereo during realtime play
	else
		gRoverLeft   =     gRover;

	return   true;
}
*****/



											////////////////////////////////////////

/************
bool     WavConvert::Create_SlowDown_Samples(   bool  rightStereo,    long numInputSampsToProcess,    long fftFrameSizeSRC,     long osamp,   float sampleRate,
																	                        float *indata,   float *outdata,      CString&   retErrorMesg  )   
{

			//  this algo uses a second  FFT-transform that is  bigger than the first to create more samples.

			//  I am having trouble figuring how the phase should recalulated   2/8/10





//	numSampsToProcess  -     tells the routine how many INPUT   samples in  indata[0...  numSampsToProcess-1]   should be pitch shifted 
//										  and moved to  outdata[0 ... numSampsToProcess-1]. 


 //  2 buffers (  indata,  outdata )   probably need to be different,  becase the output one will be  larger.


//fftFrameSizeSRC -    defines the FFT frame size used for the processing. Typical values are 1024, 2048 and 4096. 
						It may be any   value <=  MAX_FRAME_LENGTH   but it MUST be a power of 2. 

//osamp -  is the STFT  oversampling factor which also determines the overlap between adjacent STFT  frames.
              It should at least be 4 for moderate scaling ratios. A value of 32 is recommended for best quality. 

//ampleRate -    takes the sample rate for the signal  in unit Hz, ie. 44100 for 44.1 kHz audio. 


//The data passed to the routine in  indata[] should be in the range [ -1.0, 1.0 ],  which is also the output range 
 //for the data, make sure you scale the data accordingly ( for 16bit signed integers you would have to divide (and multiply) by 32768 ). 



	retErrorMesg.Empty();


	static long   gInit =  false;

	static float  gInFIFOLeft[    MAX_FRAME_LENGTH  ];
	static float  gOutFIFOLeft[  MAX_FRAME_LENGTH  ];
	static float  gOutputAccumLeft[  2 * MAX_FRAME_LENGTH  ];
	static float gLastPhaseLeft[  MAX_FRAME_LENGTH/2+1  ];    
	static float gSumPhaseLeft[  MAX_FRAME_LENGTH/2+1  ];


	static float  gInFIFORight[    MAX_FRAME_LENGTH  ];
	static float  gOutFIFORight[  MAX_FRAME_LENGTH  ];
	static float  gOutputAccumRight[  2 * MAX_FRAME_LENGTH  ];
	static float  gLastPhaseRight[  MAX_FRAME_LENGTH/2+1  ];    
	static float  gSumPhaseRight[  MAX_FRAME_LENGTH/2+1  ];


	float   gAnaFreq[   MAX_FRAME_LENGTH  ];   //  do NOT think that these 2 need to be static
	float   gAnaMagn[  MAX_FRAME_LENGTH  ];



	float   gSynFreq[   MAX_FRAME_LENGTH  ];
	float   gSynMagn[  MAX_FRAME_LENGTH  ];





	static long  gRoverLeft   =   0;  
	static long  gRoverRight =   0;



	long      gRover = 0;
	double   window,  real, imag;
	long       i, k,    inFifoLatency,   halfFftFrameSizeSRC,   	 dstIndex;
	double   tmp,  freqPerBinSRC, expctSRC,    freqPerBinDST, expctDST;
	long      qpd,   index;

	long       fftFrameSizeDST        =     m_playspeedSlowDownRatio   *   fftFrameSizeSRC;
	long       halfFftFrameSizeDST  =     fftFrameSizeDST /  2;



	float      gFFTworksp[         2 *  MAX_FRAME_LENGTH  ];    //  this is  2 times a big as  FFT-size,  beause it hold BOTH  the real and imaginary parts of the FFT

	float      gFFTworkspDST[   2 *  MAX_FRAME_LENGTH  ];  



	halfFftFrameSizeSRC =    fftFrameSizeSRC  / 2;

	long   stepSizeSRC  =    fftFrameSizeSRC  /  osamp;       //   as osamp  gets bigger,    stepSize gets smaller

	long   stepSizeDST   =      m_playspeedSlowDownRatio  *  stepSizeSRC;

//  PROOF:    For slowdownRatio = 2,    Since  stepSizeDST  is  2 times as big as  stepSizeSRC,   and   osampDST =  fftFrameSizeDST / stepSizeDST,   osampSRC equals osampDST
//												        so  osamp  value is good in either  FFT sizes.    2/10				


	inFifoLatency             =    fftFrameSizeSRC  -  stepSizeSRC;

	freqPerBinSRC       =   sampleRate  /  (double)fftFrameSizeSRC;
	freqPerBinDST       =   sampleRate  /  (double)fftFrameSizeDST;

	expctSRC              =    2.  *  M_PI  * (double)stepSizeSRC / (double)fftFrameSizeSRC;   //  these two values are the same for either size FFT
	expctDST              =    2.  *  M_PI  * (double)stepSizeDST / (double)fftFrameSizeDST;



																	// initialize our static arrays 
	if(  gInit  ==   false  )  
	{
		memset(  gInFIFOLeft,           0,   MAX_FRAME_LENGTH*sizeof(float));
		memset(  gOutFIFOLeft,         0,   MAX_FRAME_LENGTH*sizeof(float));
		memset(  gOutputAccumLeft,  0, 2*MAX_FRAME_LENGTH*sizeof(float));

		memset(  gInFIFORight,           0,   MAX_FRAME_LENGTH*sizeof(float));
		memset(  gOutFIFORight,         0,   MAX_FRAME_LENGTH*sizeof(float));
		memset(  gOutputAccumRight, 0, 2*MAX_FRAME_LENGTH*sizeof(float));


		memset(  gLastPhaseLeft,        0,   (MAX_FRAME_LENGTH/2+1)*sizeof(float));
		memset(  gLastPhaseRight,        0,   (MAX_FRAME_LENGTH/2+1)*sizeof(float));

		memset(  gSumPhaseLeft,       0,   (MAX_FRAME_LENGTH/2+1)*sizeof(float));
		memset(  gSumPhaseRight,       0,   (MAX_FRAME_LENGTH/2+1)*sizeof(float));


		gRoverLeft   =   gRoverRight =  gRover =    inFifoLatency;

		gInit =    true;      //  this is a little controversial,  but seems to work OK
	}




	float    *gInFIFO=NULL,   *gOutFIFO=NULL,  *gOutputAccum=NULL,     *gLastPhase=NULL,   *gSumPhase=NULL ; 

	if(   rightStereo   )     //  save values for next entrance to this function,  or we will hear 'static'  from variable confusion from switching from left to right stereo
	{
		gInFIFO       =     gInFIFORight;
		gOutFIFO      =    gOutFIFORight;
		gOutputAccum =    gOutputAccumRight;  

		gLastPhase  =      gLastPhaseRight;
		gSumPhase =      gSumPhaseRight;

		gRover =   gRoverRight;
	}
	else
	{  gInFIFO      =     gInFIFOLeft;
		gOutFIFO    =     gOutFIFOLeft;
		gOutputAccum =    gOutputAccumLeft;  

		gLastPhase  =      gLastPhaseLeft;
		gSumPhase =      gSumPhaseLeft;

		gRover =   gRoverLeft;
	}




																										// main processing loop 
	for(   i = 0;    i < numInputSampsToProcess;    i++  )
	{

																		//  As long as we have not yet collected enough data, just read it in 
		gInFIFO[ gRover ]  =   indata[ i ];



//		outdata[ i ]    =   gOutFIFO[  gRover  -  inFifoLatency  ];   

		dstIndex                  =      i *  m_playspeedSlowDownRatio; 

		long   gOutFifoIndex  =     ( gRover  * m_playspeedSlowDownRatio )    -    ( inFifoLatency  *  m_playspeedSlowDownRatio );


		for(   long n= 0;    n < m_playspeedSlowDownRatio;    n++   )   //  For every Input Sample,  need to return  m_playspeedSlowDownRatio number of OutputSamples
		{
			outdata[  dstIndex  ]  =     gOutFIFO[  gOutFifoIndex ];

			dstIndex++;
			gOutFifoIndex++;
		}



		gRover++;


																		
		if(   gRover   >=   fftFrameSizeSRC   )     // now we have enough data for processing 
		{


			gRover =   inFifoLatency;	 //  re-initialization his TRAVERSING index


			memset(  gFFTworksp,         0,    2 *  MAX_FRAME_LENGTH*sizeof(float)    );   //  IMPORTANT,  need to zero-out or will get static
																			
			for (  k = 0;   k < fftFrameSizeSRC;    k++)     //  do windowing and re, im interleave 
			{
				window =    -.5 * cos(   2.*M_PI * (double)k   /  (double)fftFrameSizeSRC   ) + .5;   

				gFFTworksp[ 2*k     ]   =   gInFIFO[ k ]  * window;    //   FFT  inputs  are take  from the   gInFIFO[]   circularQue-delay
				gFFTworksp[ 2*k +1]   =   0.0;
			}


					
																	
			smbFft(   gFFTworksp,   fftFrameSizeSRC,   -1   );    //   do the Foward  FFT transform     (  ie  the ANALYSIS )




										//  Analyize the FFT's column of values and calulate the Estimated True Frequency( gAnaFreq[] )  and its magnitude
					

			long  regIndex,   regIndexUse,   dstIndexUse;
			float  magn,  phase;  



			memset(  gAnaMagn,  0,   fftFrameSizeDST * sizeof(float)   );    
			memset(  gAnaFreq,    0,   fftFrameSizeDST * sizeof(float)   );



//			for(   k = 0;   k <=   halfFftFrameSizeSRC;    k++   )   
			for(   k = 1;   k <      halfFftFrameSizeSRC;    k++   )      //  should not need the Nyquist bin,  or bottom bin values  ???
			{
				regIndex  =    k;
				dstIndex  =     k *  m_playspeedSlowDownRatio;
				regIndexUse  =    2 *  regIndex;
				dstIndexUse   =    2 *  dstIndex;
				
				real    =    gFFTworksp[  regIndexUse       ];			// de-interlace FFT buffer 
				imag  =    gFFTworksp[   regIndexUse  +1  ];


				magn =   2. * sqrt(  real*real  +  imag*imag  );     	// compute magnitude and phase 
				phase =  atan2(  imag,  real  );



				tmp =   phase  -  gLastPhase[ k ];    	// compute phase difference 
				gLastPhase[ k ] =   phase;

				
				tmp -=   (double)k * expctSRC;       // subtract expected phase difference 

															
				qpd  =   tmp / M_PI;        // map delta phase into +/- Pi interval 

				if(   qpd >= 0   ) 
					qpd  +=  qpd&1;
				else 
					qpd  -=   qpd&1;

				tmp -=    M_PI * (double)qpd;

				

				tmp =   osamp * tmp   /   (2. * M_PI);    // get deviation from bin frequency from the +/- Pi interval 

				
				tmp =    (double)k * freqPerBinSRC  +   tmp * freqPerBinSRC;    // compute the k-th partials' true frequency


													// store magnitude and true frequency in analysis arrays 
				gAnaMagn[ k ]  =   magn;
				gAnaFreq[  k ]  =   tmp;			
			}



																								//  PROCESSING...   assign values to the larger SYNTHESIS Fft's bins 
		

			memset(  gSynMagn,  0,   fftFrameSizeDST * sizeof(float)   );    
			memset(  gSynFreq,    0,   fftFrameSizeDST * sizeof(float)   );


			for (  k = 0;    k  <=  halfFftFrameSizeSRC;    k++  ) 
			{ 

				index =    k  *  m_playspeedSlowDownRatio;			  	//   m_playspeedSlowDownRatio =  2,  Copy to every other freq bin


				if(  index  <=   halfFftFrameSizeDST  ) 
				{ 
					gSynMagn[  index ]  +=   gAnaMagn[ k ]; 

			//		gSynFreq[  index ]    =    gAnaFreq[ k ] * pitchShift;      NO,  here we want to keep the same pitch.
					gSynFreq[  index ]    =    gAnaFreq[ k ];
				} 
			}

			


			memset(  gFFTworkspDST,   0,    2 * MAX_FRAME_LENGTH * sizeof(float)   );  


			for(   k = 0;     k <=   halfFftFrameSizeDST;     k++   )    //  calc [real, imaginary] bin values from the Estimated True Frequency( gAnaFreq[] ) and magnitude 
			{
																
				magn =    gSynMagn[ k ];      // get magnitude and true frequency from synthesis arrays 
				tmp   =    gSynFreq[   k ];

																
				tmp  -=   (double)k  *  freqPerBinDST;     // subtract bin mid frequency 

															
				tmp   /=   freqPerBinDST;    // get bin deviation from freq deviation

															
				tmp =    2.  *  M_PI  *  tmp /   osamp;    //   osamp  for   DST is the same as for osampSRC   ...  seee proof at top
		
															
				tmp   +=    (double)k  *  expctDST;     // add the overlap phase advance back in

									
				gSumPhase[ k ]   +=   tmp;     // accumulate delta phase to get bin phase
				phase =    gSumPhase[ k ];

																			
				gFFTworkspDST[ 2 * k      ]   =    magn  *  cos( phase );     // get real and imag part and re-interleave 
				gFFTworkspDST[ 2 * k +1 ]   =    magn  *  sin( phase );
			} 




																												//   SYNTHESIS 
																						
			
//  WHY do I need this???  I thought they would be full of zeroes ??????
																							
//			for (  k =  fftFrameSizeSRC +2;     k < 2 * fftFrameSizeSRC;       k++ )    // zero negative frequencies  ( Carefull if bigger than fftFrameSizeSRC  ) 
//				gFFTworkspDST[ k ] =   0.0;

//			for (  k =  fftFrameSizeDST +2;     k < 2 * fftFrameSizeDST;       k++ )    // zero negative frequencies  ( Carefull if bigger than fftFrameSizeSRC  ) 
//				gFFTworkspDST[ k ] =   0.0;

				
			smbFft(   gFFTworkspDST,   fftFrameSizeDST,   1   );    //   apply the INVERSE Fft




																//    do windowing and 'ADD'  to output accumulator ( this is the 'ADD' part of  "Overlap And Add"  )   

			for(  k=0;   k < fftFrameSizeDST;   k++  ) 
			{
				window                     =      -.5 * cos(  2.*M_PI*(double)k  /  (double)fftFrameSizeDST  )   +.5;   

				gOutputAccum[ k ]   +=       2.  *  window  *  gFFTworkspDST[ 2*k ]  /  ( halfFftFrameSizeDST * osamp );
			}


																			//   SHIFT the values in the static ARRAYS for next call of funct
			for (  k = 0;     k < stepSizeDST;    k++  ) 
				gOutFIFO[ k ] =   gOutputAccum[ k ];

																														
			memmove(  gOutputAccum,    gOutputAccum + stepSizeDST,     fftFrameSizeDST * sizeof(float)    );    // shift the accumulator 

			
			for( k = 0;   k < inFifoLatency;   k++ )						  // shift the  INPUT Fifo  
				gInFIFO[ k ] =    gInFIFO[ k + stepSizeSRC ];
		}
	}




	if(   rightStereo   )				         //   save the values for next entrance to this function,  or we will hear 'static'  from variable confusion 
		gRoverRight =     gRover;       //   from switching from left to right stereo during realtime calculation
	else
		gRoverLeft   =     gRover;

	return   true;
}
****************/






