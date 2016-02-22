/////////////////////////////////////////////////////////////////////////////
//
//  BitSourceAudio.cpp   -   
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




#include  "BitSourceAudio.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////



//void       Get_ScalePitch_LetterName(  short  sclPitch,   char *firLet,  char *secLet  );


////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////





			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////


BitSourceAudioMS::BitSourceAudioMS()
{



	m_chunkSize           =        TransformMap::Get_ChunkSize_OLD_PitchScope();   
//   *****   WRONG for NAVIGATOR *****     HARDWIRED,   fix *******





	m_bytesPerSample  =        BitSourceAudioMS::Bytes_Per_Sample();   //   ***** HARDWIRED,   for   16bit,  STEREO




	m_wavFormat =   NULL;

	m_totalBytes = 0;

	m_usingSecondaryBuffer =   false;

	m_sampleRate  =    DEFsAMPLERATE;    //   44100;     // ***INSTALL,  should be read from file ********************

	m_mmIO =   NULL;   // ***** OK or trouble ??? 



	m_filesDataStart =  -1;


	m_wavVolumeSoundmanAddr    =    NULL;      //  the  ADDRESS( in soundman )ScalePitchMask  that isused for animation  
}



											////////////////////////////////////////


BitSourceAudioMS::~BitSourceAudioMS()
{

	Release_WaveFormatEx();
	Release_Media_File();
}


											////////////////////////////////////////


void     BitSourceAudioMS::Release_All_Resources()
{

	Release_WaveFormatEx();

	Release_Media_File();

	m_isInitialized =  false;   //  NEW,  ok ???    5/07
}


											////////////////////////////////////////


void   BitSourceAudioMS::Release_WaveFormatEx()
{

	if(    m_wavFormat   !=  NULL    )
	{
		GlobalFree(   m_wavFormat   );      	m_wavFormat =   NULL; 
	}
}



											////////////////////////////////////////


void   BitSourceAudioMS::Release_Media_File()
{

	if(    m_mmIO  !=   NULL   )    
	{
        mmioClose(   m_mmIO,   0  );		 
		m_mmIO =  NULL;	 	
    }
}



											////////////////////////////////////////


int     BitSourceAudioMS::Number_Samples()    
{  

	// ***** FIX:  so that it gets these values from the WAV file ************  JPM


	long   retNumSamples = 0L;

	long   numberOfChannels  =  2L;				 //   ****ASSUME  stereo

	long   numberOfBytesPerSample =  2L;     //   ****ASSUME  16-bit
	

	retNumSamples  =      m_totalBytes   /   (   numberOfChannels  *  numberOfBytesPerSample   );  


	return   retNumSamples;   
}



											////////////////////////////////////////


WAVEFORMATEX*     BitSourceAudioMS::Alloc_New_WAVEFORMATEX()
{


	if(    m_wavFormat  !=  NULL    )
	{
		GlobalFree(   m_wavFormat   );      		
		m_wavFormat =   NULL;     //  FLAG the release
	}


	int  structSize =   sizeof(  WAVEFORMATEX  );    


    if(    NULL ==    (  m_wavFormat  =   (  WAVEFORMATEX* )GlobalAlloc(   GMEM_FIXED,   structSize  )    )    )
		return   NULL;
	else
		return   m_wavFormat;
}



											////////////////////////////////////////


bool    BitSourceAudioMS::Open_Get_WavFormatEx(   long  chunkSize,    CString&   filePath,     CString&   retErrorMesg   )
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



	long   totalSamplesBytes  =     m_mmChunkInfo.cksize;      //   NEW,  seems accurate   9/15/02

	m_totalBytes  =    totalSamplesBytes;




//	m_isInitialized =    true;

	m_currentByteIdx =   0;   


	return  true;
}







			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////


BitSourceStatic::BitSourceStatic()
{

	m_staticBits  =   NULL;
}


											////////////////////////////////////////


BitSourceStatic::~BitSourceStatic()
{
	
	Release_Static_Block();
}


											////////////////////////////////////////


void   BitSourceStatic::Release_Static_Block()
{

	if(    m_staticBits    !=  NULL    )
	{
		GlobalFree(   m_staticBits   );			m_staticBits  =    NULL;    
		
		m_totalBytes =    0;
	}
}


											////////////////////////////////////////


SndSample*     BitSourceStatic::Create_8bit_Sample_DynamicAlloc(   short  channelCode,   CString&  retErrorMesg   )
{



	SndSample  *nuSample =  NULL;


	retErrorMesg.Empty();


	int   totBytes     =     Number_Bytes();
	if(    totBytes  <=  0   )
	{
		retErrorMesg =  "BitSourceStatic::Create_8bit_Sample_DynamicAlloc  failed,  no bytes in BitSource." ;
		return  NULL;
	}

	int  	numChannelSamples  =     totBytes  / 4;      //  Total samples(BYTES) in ONE Channel of 8-bit 



	if(   (  nuSample  =    new    SndSample(   numChannelSamples,   m_chunkSize  )  )    ==  NULL  )
	{
		ASSERT( 0 );   	
		//  m_totalSamples =   0;      //  OK to show failure like this ???
		retErrorMesg =  "BitSourceStatic::Create_8bit_Sample_DynamicAlloc  failed,  could not alloc SndSample." ;
		return  NULL;
	}   



// ****** WANT  this ???? ******
//	m_totalSamples =      numChannelSamples;     //  Show the sucessful allocation.  ( Total samples(BYTES) in ONE Channel of 8-bit
	




//	Copy_To_Transform_Samples(   *nuSample,    channelCode   );       //  Fill the secondary SndSamples with 8-bit data of the WAV file	


	return  nuSample;
}





											////////////////////////////////////////


char*     BitSourceStatic::Alloc_Static_Block(   long  numBytes   )
{


	if(    m_staticBits  !=  NULL    )
	{

		GlobalFree(   m_staticBits   );      //  LATER:   free( m_staticBits );     ....sync with   WaveLoadFile_JP()    
		
		m_staticBits =   NULL;     //  FLAG the release
		m_totalBytes =  0;
	}


    if(    NULL ==     (  m_staticBits  =     ( char* )GlobalAlloc(   GMEM_FIXED,   numBytes  )    )      )
	{
		m_totalBytes =  0;
		return   NULL;
	}


	m_totalBytes =   numBytes;


	m_usingSecondaryBuffer  =    false;    // ****** CAREFUL *******

	return   m_staticBits;
}



											////////////////////////////////////////


bool   BitSourceStatic::Initialize(   long  chunkSize,    CString&   filePath,     CString&   retErrorMesg   )
{

	int        error;
	UINT    retBytesLoaded;			//   Load the File ....

	
	retErrorMesg.Empty();
	m_isInitialized =  false;


	Release_Static_Block();     // Get rid of the old memory buffer, if it exists
	Release_WaveFormatEx();
	Release_Media_File();


	m_chunkSize  =    chunkSize;



	if(  0   !=   ( error    =   WavFile_Load_Data_And_Format(    filePath.GetBuffer(0),      
																			     &retBytesLoaded,                   
																			     &m_wavFormat,              //  &m_ppwfxInfo,             // (OUT)   WAVEFORMATEX**
													                (BYTE**)&m_staticBits       )  ))    //   WAS:  &m_ppbData     (OUT)    BYTE**     																			         
	{  ASSERT( 0 );

		m_wavFormat =    NULL;    //   ok ???? 
		m_staticBits    =    NULL;  
		m_totalBytes   =     0;

		//  m_strWavFilePath.Empty();     ...????   is this a good idea ????     JPM

		retErrorMesg =  "BitSourceStatic::Initialize failed,   WavFile_Load_Data_And_Format  had error. " ;

		return  false;
	}
		



	m_totalBytes         =      ( int )retBytesLoaded;   

	m_strWavFilePath  =      filePath;

	m_isInitialized =    true;

	return  true;
}



											////////////////////////////////////////


bool    BitSourceStatic::Create_Secondary_Buffer(   long   byteCount   )
{

		//   When a file is NOT loaded,   but need an are to sysnthesize a note


	if(    m_staticBits  !=  NULL    )
	{
		GlobalFree(   m_staticBits   );      //  LATER:   free( m_staticBits );     ....sync with   WaveLoadFile_JP()    
		
		m_staticBits =   NULL;     //  FLAG the release
		m_totalBytes =  0;
	}



    if(    NULL ==  (  m_staticBits  =     ( char* )GlobalAlloc(  GMEM_FIXED,  byteCount  )    )    )
    {
		return   false;
	}



	m_usingSecondaryBuffer  =    true;

	m_totalBytes =   byteCount;

	return   true;
}



											////////////////////////////////////////


bool	 BitSourceStatic::Erase_SecondaryBuffer()
{

	if(     !m_usingSecondaryBuffer  
		||   m_staticBits  ==  NULL   
		||   m_totalBytes   <=  0      )
	{
		ASSERT( 0 );
		return  false;
	}


	char  *dst =   m_staticBits;

	for(   int i=0;    i< m_totalBytes;    i++  )     // **** later OPTIMIZE with single function to set memory *****
	{
		*dst =  0;   // *********   is  128 better ????  *********
		dst++;
	}

	return  true;
}


											////////////////////////////////////////


bool    BitSourceStatic::Copy_From_Transform_Samples(   SndSample&  srcSndSample,   short  channelCode,   CString&  retErrorMesg    )
{

					//  NEED WORK ??? :  now that we have onlyONE channel in an analyzer

	

                                               //  Now goin to write BACK to the ACTUAL buffer used for playback

	long     totalSamples  =    srcSndSample.Get_Length();

	char    *dest =     Get_Start_Byte();
	
	char    *src =       srcSndSample.Get_StartByte();       //  a secondary 8-bit buffer 

	char    *srcRight =NULL;  



	if(    channelCode  ==     TransformMap::SEPARATEstereo  )     
	{
		if(   ! srcSndSample.Is_Stereo_Sample()   )
		{
			ASSERT( 0 );     
			retErrorMesg =  "BitSourceStatic::Copy_From_Transform_Samples  FAILED,  missing channelCode case."  ; 
			return  false;
		}


		srcRight =       srcSndSample.Get_StartByte_RightStereo();    
	}






// ********** WHY do I write 127( and not 0 ) to a char  when knocking out the signal ??? ********


	if(     channelCode  ==     TransformMap::LEFTj    )   
	{

		for(   int  k=0;     k<  totalSamples;      k++   )     
		{

			*dest =   127;             
			dest++; 
					
			*dest =   *src;       //  left  
			 dest++;    
		     src++;


	//		*dest =   127;            ***NEW,  do not erase other channel
			  dest++;  

	//		*dest =   127;        //  right          // *** OK ??? ***      //	WAS:  *dest =  *srcRight;     
			  dest++;    
			  //  srcRight++;											
		}

	}

	else if(     channelCode  ==     TransformMap::RIGHTj    )   
	{

		for(   int  k=0;     k<  totalSamples;      k++   )     
		{

	//		*dest =   127;              ***NEW,  do not erase other channel
			dest++; 
					
	//		*dest =   127;      //  left       // *** OK ??? ***      //	WAS:  *dest =   *srcLeft;       
			 dest++;    
		 //  srcLeft++;


			*dest =   127;             	//	ALT knockout:  *dest =  127;      dest++;   srcRight++;     ...try to knockout the RIGHT channel
			  dest++;  

			*dest =    *src;        //  *src;						//   right 
			  dest++;    
			  src++;											
		}
	}

	else if(     channelCode  ==     TransformMap::CENTEREDj    )   
	{

		for(   int  k=0;     k<  totalSamples;      k++   )     
		{

			*dest =   127;             
			dest++; 
					
			*dest =   *src;       //  left  
			 dest++;    




			*dest =   127;             	//	ALT knockout:  *dest =  127;      dest++;   srcRight++;     ...try to knockout the RIGHT channel
			  dest++;  

			*dest =    *src;        //  *src;						//   right 
			  dest++;    


			  src++;	

		}
	}

	else if(    channelCode  ==     TransformMap::SEPARATEstereo    )   
	{

		for(   int  k=0;     k<  totalSamples;      k++   )     
		{

			*dest =   127;             
			dest++; 
					
			*dest =   *src;       //  left  
			 dest++;    




			*dest =   127;             	//	ALT knockout:  *dest =  127;      dest++;   srcRight++;     ...try to knockout the RIGHT channel
			  dest++;  

			*dest =    *srcRight;        //  *src;						//   right 
			  dest++;    


			  src++;	
			  srcRight++;
		}
	}	
	else
	{  ASSERT( 0 );     
		retErrorMesg =  "BitSourceStatic::Copy_From_Transform_Samples  FAILED,  missing channelCode case."  ; 
		return  false;
	}


	return  true;
}



											////////////////////////////////////////


bool    BitSourceStatic::Copy_To_Transform_Samples(   SndSample&  dstSndSample,    short  channelCode,   CString&  retErrorMesg   )
{ 


									 //  Now going to write BACK to the ACTUAL buffer used for playback
	                                      
	
	char  *src   =    Get_Start_Byte();

	char  *dest =    dstSndSample.Get_StartByte();      //  the secondary 8-bit buffers   ( ? from  InverseDFT  )
	char  *destRight = NULL;  

	long   totalSamples  =    dstSndSample.Get_Length();




	if(      dest ==  NULL    ||    src == NULL 
		||   totalSamples  <=  0    )
	{
		ASSERT( 0 );     
		retErrorMesg =  "BitSourceStatic::Copy_To_Transform_Samples  FAILED,  one of the input SndSample is BAD."  ; 
		return  false;
	}



	if(    channelCode  ==     TransformMap::SEPARATEstereo  )     
	{
		if(   ! dstSndSample.Is_Stereo_Sample()   )
		{
			ASSERT( 0 );     
			retErrorMesg =  "BitSourceStatic::Copy_To_Transform_Samples  FAILED,  missing channelCode case."  ; 
			return  false;
		}


		destRight =       dstSndSample.Get_StartByte_RightStereo();    
	}







	if(    channelCode  ==     TransformMap::LEFTj    )        
	{

		for(    int  k=0;     k<  totalSamples;      k++   )     
		{
			src++; 				
			*dest =   *src;               //  left  
			src++;    


			dest++;

			src++;  
			//  *destRight =  *src;         //  right
			src++;    
			//  destRight++;
		}

	}

	else if(    channelCode  ==     TransformMap::RIGHTj    )     
	{

		for(    int  k=0;     k<  totalSamples;      k++   )     
		{
			src++; 				
			//  *destLeft =   *src;           //  left  
			src++;    
			//  destLeft++;

			src++;  
			*dest =  *src;              //  right
			src++;    


			dest++;
		}
	}

	else if(    channelCode  ==     TransformMap::CENTEREDj    )     
	{

		for(    int  k=0;     k<  totalSamples;      k++   )     
		{
			src++; 				
			*dest =   *src;           //  left  
			src++;    

			src++;  
			*dest =  *src;              //  right
			src++;    


			dest++;
		}
	}
	else if(    channelCode  ==     TransformMap::SEPARATEstereo    )   
	{

		for(    int  k=0;     k<  totalSamples;      k++   )     
		{
			src++; 				
			*dest        =    *src;           //  left  
			src++;    

			src++;  
			*destRight =   *src;              //  right
			src++;    


			dest++;
			destRight++;
		}
	}	

	else
	{  ASSERT( 0 );     
		retErrorMesg =  "BitSourceStatic::Copy_To_Transform_Samples  FAILED,  missing channelCode case."  ; 
		return  false;
	}


	return  true;
}




			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////


BitSourceStreamingMS::BitSourceStreamingMS()			  //   ***INPUT   'SPEEDZONESIZE'    *******
{


	m_readBuffersBits          =   NULL;
	m_readBuffersBitsRight  =   NULL;

	m_reduxInVolumeWhenPlaying  =   50;  //    70 ********ADJUST    in percent.  **********
	     //Because the WAV is so much louder that the Midi, we reduce bu this amount when the sample is played.  6/07


	m_wavFormat =   NULL;


	m_usingSecondaryBuffer =   false;

	m_mmIO =   NULL;   // ***** OK or trouble ??? 


	m_totalBytes = 0;


	m_filesDataStart =  -1;


	m_wavVolumeSoundmanAddr    =    NULL;      //  the  ADDRESS( in soundman )ScalePitchMask  that isused for animation  
}


											////////////////////////////////////////


BitSourceStreamingMS::~BitSourceStreamingMS()
{
	
	Release_BufferBits();  //  This only gets called at program exit.  9/06
}



											////////////////////////////////////////


void	  BitSourceStreamingMS::Release_BufferBits()
{


	if(    m_readBuffersBits    !=  NULL    )
	{
		GlobalFree(   m_readBuffersBits   );			m_readBuffersBits   =    NULL;    
		
		m_bufferSize =    0;

//		m_totalBytes =    0;   // OK ????    do NOT think I should do this to the var.   10/02
	}


	

	if(    m_readBuffersBitsRight    !=  NULL    )
	{
		GlobalFree(   m_readBuffersBitsRight   );			m_readBuffersBitsRight   =    NULL;    
		
		m_bufferSize =    0;
	}	
}



											////////////////////////////////////////


void     BitSourceStreamingMS::Release_All_Resources()
{

		//  Only called by    BitSourceStreamingMS::Initialize(

	Release_BufferBits();   	


	Release_WaveFormatEx(); 

	Release_Media_File();

	Release_WAV_CircularQue();


	m_isInitialized =  false;   //  NEW,  ok ???    5/07
}


											////////////////////////////////////////


void   BitSourceStreamingMS::Release_Media_File()
{

	if(    m_mmIO  !=   NULL   )    
	{
        mmioClose(   m_mmIO,   0  );		 
		m_mmIO =  NULL;	 	
    }
}



											////////////////////////////////////////


bool    BitSourceStreamingMS::Alloc_LocalBuffer(   long  bufferSize  )
{


		//   used to allocate the  SpeedZone buffer  (  4096   bytes )    ... AND...      the buffer for StreamingAnalysis,   

		//   (   see    Fetch_Chunks_Samples_for_StreamingAnalysis()       )



	Release_BufferBits();


    if(       (    NULL ==  (  m_readBuffersBits         =      ( char* )GlobalAlloc(     GMEM_FIXED,    bufferSize   )  )       )
		||   (    NULL ==  (  m_readBuffersBitsRight  =      ( char* )GlobalAlloc(     GMEM_FIXED,    bufferSize   )  )       )  		
	  )
    {

		if(    m_wavFormat   !=   NULL     )
		{  GlobalFree(  m_wavFormat   );	   m_wavFormat =  NULL;    }

		if(    m_mmIO  !=   NULL   )    
		{  mmioClose(   m_mmIO,   0  );		  m_mmIO =  NULL;       }	       


		m_bufferSize =   0;

		//   retErrorMesg =   "BitSourceAudioMS::Open_File_for_StreamingAnalysis  failed,  not allocate Samples memory." ;
        return  false;
	}



//   NO,  done earlier.     	m_totalBytes  =     m_mmChunkInfo.cksize;    //  FLAG.   The count of  SampleBytes  for the  ENTIRE file

	m_bufferSize  =     bufferSize;    //   FLAG  that allocation was sucessful
	

	return  true;
}




											////////////////////////////////////////


bool   BitSourceStreamingMS::Initialize(   long  chunkSize,    CString&   filePath,    bool&  fileAcessError,   CString&   retErrorMesg   )
{

			//   NOT for Player or Navigator,   just used by Old PitchScope


														//   WAS:   Register_WavFile_for_Streaming()
	retErrorMesg.Empty();
	fileAcessError =   false;


	if(    filePath.IsEmpty()    )
	{
		retErrorMesg =    " BitSourceStreamingMS::Initialize  failed,   filePath is empty."  ;
		return  false;
	}


	m_isInitialized  =  false;


	/****
	Release_BufferBits();   	
	Release_WaveFormatEx();
	Release_Media_File();
	****/
	Release_All_Resources();



		
	short    appCode =   Get_PitchScope_App_Code_GLB();    //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope


	if(    ! Allocate_WAV_Circular_Que(   appCode,   retErrorMesg   )     )
		return  false;





	m_strWavFilePath  =    filePath;


	if(     ! Open_Get_WavFormatEx(   chunkSize,    filePath,     retErrorMesg   )      )  
	{

		fileAcessError =   true;   //  the file could not be found or opened. 

		return  false;
	}





	long   bufferSize =     4096L;     //    SPEEDZONESIZE    4096;    


	if(     ! Alloc_LocalBuffer(   bufferSize  )     )
	{
		retErrorMesg =     "BitSource::Open_File_for_StreamingAnalysis  failed,  could not allocate local buffer." ;
        return  false;
    }



	m_currentByteIdx =  0L;

	m_isInitialized =   true;

	return  true;
}


										////////////////////////////////////////


long	  BitSourceStreamingMS::Calc_Files_Total_Output_Bytes_With_SpeedExpansion()
{

	long      byteCount  =   m_totalBytes;    
	return   byteCount;  
}


										////////////////////////////////////////


long	  BitSourceStreamingMS::Get_Files_Total_Samples_No_SpeedExpansion()
{

	long      sampleCount  =   Number_Samples();
	return   sampleCount;  
}


											////////////////////////////////////////


int		  BitSourceStreamingMS::Number_Bytes()                   
{  

			   //    Is called by   AnalyzerAudio::Calc_Channels_Sample_Count()     ...PitchScope

	return  m_totalBytes;  
} 


											////////////////////////////////////////


int     BitSourceStreamingMS::Number_Samples()    
{  

			//  Gets called alot by OLD PitchScope   classes    ChannelDetectionSubject,   PitchDetectorApp 



	long   retNumSamples = 0L;

	long   numberOfChannels  =  2L;				 //   ****ASSUME  stereo

	long   numberOfBytesPerSample =  2L;     //   ****ASSUME  16-bit
	

	retNumSamples  =      m_totalBytes   /   (   numberOfChannels  *  numberOfBytesPerSample   );  


	return   retNumSamples;   
}



										////////////////////////////////////////


bool    BitSourceStreamingMS::Open_Get_WavFormatEx(   long  chunkSize,    CString&   filePath,     CString&   retErrorMesg   )
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




	long   totalSamplesBytes  =     m_mmChunkInfo.cksize;      //   NEW,  seems accurate   9/15/02

	m_totalBytes  =    totalSamplesBytes;





//	m_isInitialized =    true;

	m_currentByteIdx =   0;   


	return  true;
}



											////////////////////////////////////////


bool    BitSourceStreamingMS::Move_To_DataStart(   CString&   retErrorMesg   )
{		

									//   CALLED by:  StreamingAudioplayer::StartPlaying()


    int    errorCode;					    

	retErrorMesg.Empty();     	//  This moves us to the  'DATA' area  in the file


	if(  ! m_isInitialized   )
	{					
		ASSERT( 0 );
		retErrorMesg  =  "BitSourceAudioMS::Move_To_DataStart, bitsource has not been INITIALIZED.."  ;     // the file is probably NOT yet opened, etc.
		return  false;
	}



    if(  (  errorCode=    WavFile_Start_Reading_Data(    &m_mmIO,     &m_mmChunkInfo,     &m_chunkInfoParent    )  )   != 0  )
    {

		/***   WANT ????

		if(    m_wavFormat   !=   NULL     )
		{  GlobalFree(  m_wavFormat   );	   m_wavFormat =  NULL;    }

		if(    m_mmIO  !=   NULL   )    
		{  mmioClose(   m_mmIO,   0  );		  m_mmIO =  NULL;       }	       
		***/

		retErrorMesg =     "BitSourceAudioMS::Move_To_DataStart  failed,  WavFile_Start_Reading_Data failed." ;
        return  false;
    }


	m_currentByteIdx =    0L;      

	return  true;
}


											////////////////////////////////////////


bool    BitSourceStreamingMS::Fetch_Chunks_Samples_for_StreamingAnalysis(    bool&  retHitEOF,    CString&   retErrorMesg    )
{

									// *****  NO CALLERS  ********************

// ****** RESTORE,  maybe some changes from new StreamingPlayer



	long   bytesPerSixteenBitSample  =     4L;     // ***** HARDWIRE ***     OK ?????



	retErrorMesg.Empty();


	if(      !m_isInitialized    
		||    m_bufferSize  <= 0     )
	{
		retErrorMesg =   "BitSource::Fetch_Chunks_Samples_for_StreamingAnalysis failed,  stream file was not initialized." ;
		return  false;
	}


	ASSERT(   m_readBuffersBits   );



	BYTE   *destBufferBytes  =     ( BYTE* )m_readBuffersBits;



	long      localByteCount =  0;       //  just count the bytes read for THIS FUNCTION CALL
	int        errorCode;

	retHitEOF  =   false;



																  //  copy the bytes from the IO to the Buffer. 


	for(    long  byteIdx =0;       byteIdx <    m_bufferSize;        byteIdx++    )      //  reads in BLOCKS of   'm_bufferSize'
	{															 


																				//   check if we have hit the END of FILE
		if(     m_currentByteIdx   >=    m_totalBytes    )    
		{
			retHitEOF  =   true;       //  found the End of File without error.
			return  true;        
		}



																
		if(    m_mmIOinfo.pchNext    ==     m_mmIOinfo.pchEndRead    )     
		{

														//  Fetch new BLOCKS to internal buffer( I do not maintain it )....   ( 8000+  bytes ) 

			if(   (  errorCode =    mmioAdvance(   m_mmIO,    &m_mmIOinfo,    MMIO_READ    )     )    != 0    )
			{
				//  goto   ERROR_CANNOT_READ;   
				retErrorMesg =   "BitSource::Fetch_Chunks_Samples_for_StreamingAnalysis failed,   mmioAdvance()  could not read." ;
				return  false;
			}
			else
			{  
				/*********************  ORIGINAL

				unsigned long    end,  start,   numBytes;    //   *****  Need to fix this some time so that I use  HPSTR  instead of long  *********   1/10


				start =    (  unsigned long   )(  m_mmIOinfo.pchNext  );     // ****** Now get casting errors here ????   WHY?   12/11/09 *****************8
				end  =    (  unsigned long   )(  m_mmIOinfo.pchEndRead  );   //    warning C4311: 'type cast' : pointer truncation from 'HPSTR' to 'unsigned long'

				numBytes =   end -  start;
				******/


				/*******
				unsigned long    numBytes;         //  NEED to test this for a while and see if a problem arises
				unsigned long    numBytesHP;     //       ....WHEN happy,  take out this cast which bothers the compiler.     1/24/10


				HPSTR               endHP,  startHP;        //   typedef char _huge *    HPSTR
				unsigned long     end,     start;
			

				start =    (  unsigned long   )(  m_mmIOinfo.pchNext  );     // ****** Now get casting errors here ????   WHY?   12/11/09 *****************8
				end  =    (  unsigned long   )(  m_mmIOinfo.pchEndRead  );   //    warning C4311: 'type cast' : pointer truncation from 'HPSTR' to 'unsigned long'
				numBytes =     end  -  start;


				startHP =     m_mmIOinfo.pchNext;         
				endHP  =     m_mmIOinfo.pchEndRead;   
				numBytesHP =     endHP  -  startHP;


				if(    numBytes  !=   numBytesHP   )
				{
					ASSERT( 0 );
					retErrorMesg =  "Fetch_Chunks_Samples_for_StreamingAnalysis  FAILED,  bad pointer arithmetic." ;
					return  false;
				}
				else
					numBytes =    numBytesHP;
				*******/




							//  [ NECESSARY ???  4/00 ]      test to make sure that the  BYTES to be READ is  MOD 4, or I may have alignment problems
				/***
				if(    (  numBytes  %  4 )   !=  0    )
				{
					TRACE( "  WaveReadFile( )   BAD buffer size for 16bit stereo!!! \n"  );
					ASSERT( false );
				}
				***/
			}


			if(     m_mmIOinfo.pchNext    ==    m_mmIOinfo.pchEndRead     )
			{
				//  goto   ER_CORRUPTWAVEFILE;   
				retErrorMesg =   "BitSource::Fetch_Chunks_Samples_for_StreamingAnalysis failed,   file is corrupt." ;
				return  false;
			}
		}
 




		unsigned char    byVal;
		char                 ch1,  ch3;
		unsigned long    byVal0,    byVal1,   byVal2,    byVal3;      //  could  OMIT:  ***** NOT really used !!! *****

			//  BAD:    *((BYTE*)destBufferBytes+byteIdx)  =       *((BYTE*)m_mmIOinfo.pchNext)++;  NEEDED to break into 2 lines.  **** jpm 

  		   //   WORKS:   *(   (BYTE*)destBufferBytes  + byteIdx   )  =   *(    ( BYTE* )m_mmIOinfo.pchNext    );      //  WORKS




																		//   FETCH  the value of the  CURRENT BYTE

		byVal  =      *(    ( BYTE* )m_mmIOinfo.pchNext    );     


		m_currentByteIdx++;     //   global count   for entire file
		localByteCount++;	      //    local   count   for just this FUNCTION CALL



		if(          (byteIdx % 4) == 0   )         //   'byteIdx'   is relative to the ABSOLUTE start of  'ALL dataBytes'
		{
				byVal0 =   byVal;
				 *(   (BYTE*)destBufferBytes  + byteIdx   )  =    byVal;   //  Write the value to the  OUTSIDE MEMORY (  destBufferBytes  )
		}
		else if(   (byteIdx % 4) == 1   )   
		{
				byVal1 =   byVal;
				*(   (BYTE*)destBufferBytes  + byteIdx   )  =     byVal;  

				ch1 =   *(    ( char* )m_mmIOinfo.pchNext    );     //  Get LEFT channel's amplitude ( divided by 256 )
		}
		else if(   (byteIdx % 4) == 2   )   
		{
				byVal2 =   byVal;
				*(   (BYTE*)destBufferBytes  + byteIdx   )  =    byVal;  
		}
		else if(   (byteIdx % 4) == 3   )
		{
				byVal3 =   byVal;
				 *(   (BYTE*)destBufferBytes  + byteIdx   )  =    byVal;  

				ch3 =   *(    ( char* )m_mmIOinfo.pchNext    );    //  Get RIGHT channel's amplitude ( divided by 256 )


		//		if(     abs(  (int)ch1  )   >   leftMax	   )         leftMax    =    abs(  (int)ch1  );
		//		if(     abs(  (int)ch3  )   >   rightMax	   )    rightMax  =   abs(  (int)ch3  );	
		}
		else   
			ASSERT(  false );



		( BYTE* )m_mmIOinfo.pchNext++;    	
	}




	/******  NO !!!   Don't let the   calling function[   logDFTBankVerter::Load_Samples_Bits()   ]    know this yet.   

	if(     m_currentByteIdx   >=    m_totalBytes    )				  //   Might have hit it at the exact END of the read block
		retHitEOF =   true;       //  found the End of File without error.
	****/


	return   true;
}




											////////////////////////////////////////


bool    BitSourceAudioMS::Move_To_DataStart(  CString&   retErrorMesg   )
{

    int    errorCode;					    	//  This moves us to the  'DATA' area  in the file

	retErrorMesg.Empty();


	if(  ! m_isInitialized   )
	{					
		ASSERT( 0 );
		retErrorMesg  =  "BitSourceAudioMS::Move_To_DataStart, bitsource has not been INITIALIZED.."  ;     // the file is probably NOT yet opened, etc.
		return  false;
	}



    if(  (  errorCode=    WavFile_Start_Reading_Data(    &m_mmIO,     &m_mmChunkInfo,     &m_chunkInfoParent    )  )   != 0  )
    {

		/***   WANT ????

		if(    m_wavFormat   !=   NULL     )
		{  GlobalFree(  m_wavFormat   );	   m_wavFormat =  NULL;    }

		if(    m_mmIO  !=   NULL   )    
		{  mmioClose(   m_mmIO,   0  );		  m_mmIO =  NULL;       }	       
		***/

		retErrorMesg =     "BitSourceAudioMS::Move_To_DataStart  failed,  WavFile_Start_Reading_Data failed." ;
        return  false;
    }


	m_currentByteIdx =    0L;      

	return  true;
}




											////////////////////////////////////////



bool    BitSourceStreamingMS::Seek_New_DataPosition(   long    offsetFromDataStart,    CString&   retErrorMesg   )
{


ASSERT( 0 );      //  Eventually OMIT when sure  Seek_New_DataPosition_Fast()  is working OK


 
	MMIOINFO   mmIOinfo;              // current status of <mmIO>
    int               errorCode = 0;
	long            bytestToPass =   0;


	retErrorMesg.Empty();




			//	mmioGetinfo :    Retrieves information about a file opened by using the mmioOpen function.  This information
			//							allows the application to  'directly access'  the I/O buffer,  if the file is opened for buffered I/O.


	if(    errorCode =      mmioGetInfo(   m_mmIO,    &mmIOinfo,   0   )      != 0    )
    {
		retErrorMesg  =  "StreamingAudioplayer::Seek_New_DataPosition   failed,    mmioGetInfo  failed." ;
		return   false;
    }




	if(    offsetFromDataStart    <    m_currentByteIdx    )     
	{
														  //  This moves us to the  'DATA' area  in the file
																		
		if(  (  errorCode=    WavFile_Start_Reading_Data(    &m_mmIO,     &m_mmChunkInfo,     &m_chunkInfoParent    )  )   != 0  )
		{

			if(    m_wavFormat   !=   NULL     )
			{  GlobalFree(  m_wavFormat   );	   m_wavFormat =  NULL;    }

			if(    m_mmIO  !=   NULL   )    
			{  mmioClose(   m_mmIO,   0  );		  m_mmIO =  NULL;       }	       

			retErrorMesg =     "BitSourceAudioMS::Seek_New_DataPosition  failed,  WavFile_Start_Reading_Data failed." ;
			return  false;
		}



		m_currentByteIdx =     0L;       // **** OK to init  here ????

		
		bytestToPass  =      offsetFromDataStart;        //   Because we must traverse from the data START of the file

	}
	else  if(    offsetFromDataStart    >    m_currentByteIdx     )
	{
		bytestToPass  =      offsetFromDataStart    -    m_currentByteIdx;    
	}
	




	if(    bytestToPass  >  0    )
	{

		if(    ( bytestToPass  %  4 )   !=   0     )			  //  need to always read on  32bit boundaries
			ASSERT( 0 );  



		for(    long  byte =0;       byte <    bytestToPass;       byte++    )
		{															 

																						//   Are we ready to ADVANCE to another BlockRead...
			if(     mmIOinfo.pchNext   ==    mmIOinfo.pchEndRead    )
			{
				if(   (  errorCode =    mmioAdvance(   m_mmIO,    &mmIOinfo,    MMIO_READ   )     )  != 0   )
				{
					retErrorMesg  =     "StreamingAudioplayer::Seek_New_DataPosition   failed,  mmioAdvance  failed." ;
					return   false;
				}


				if(    mmIOinfo.pchNext    ==    mmIOinfo.pchEndRead    )
				{
					retErrorMesg  =     "StreamingAudioplayer::Seek_New_DataPosition   failed,  corrupt file." ;
					return   false;
				}
			}
 


			( BYTE* )mmIOinfo.pchNext++;    			

			m_currentByteIdx++;				//   that is....    OFFSETTED from the  Sample's  DataSTART
		}
	
	}



				//  mmioSetInfo()  :    updates the information retrieved by the mmioGetInfo function about a file opened 
				//								by using the mmioOpen function. Use this function to terminate   'direct buffer access' 
				//								of a file opened for buffered I/O.


    if(    (  errorCode =     mmioSetInfo(   m_mmIO,    &mmIOinfo,   0   )     )      != 0    )
	{
		retErrorMesg  =     "StreamingAudioplayer::Seek_New_DataPosition   failed,  mmioSetInfo  failed." ;
		return   false;
	}


	return  true;
}



											////////////////////////////////////////


bool    BitSourceStreamingMS::Seek_New_DataPosition_Fast(   long    offsetFromDataStart,  short directionCode,    CString&   retErrorMesg   )
{

													//   SEEMS to work OK,  but might be a little off    10/02

    int      errorCode =  0;				//  is CALLED by    Fetch_Streaming_Samples_Direct_PPlayer
	long   bytestToPass =   0;


	retErrorMesg.Empty();


	if(  ! m_isInitialized   )
	{					
		ASSERT( 0 );
		retErrorMesg  =  "BitSourceAudioMS::Seek_New_DataPosition_Fast, bitsource has not been INITIALIZED.."  ;     // the file is probably NOT yet opened, etc.   5/07
		return  false;
	}



	if(     offsetFromDataStart    <    m_currentByteIdx     )     
	{
																				 //  This moves us to the  'DATA' area  in the file

//  *********************************************************************************
//  ****** Does this only happen during REVERSE play ???? **********************************															


		if(  (  errorCode=    WavFile_Start_Reading_Data(    &m_mmIO,     &m_mmChunkInfo,     &m_chunkInfoParent    )  )   != 0  )
		{
			retErrorMesg =     "BitSourceAudioMS::Seek_New_DataPosition  failed,  WavFile_Start_Reading_Data failed." ;
			return  false;
		}



		m_currentByteIdx  =    0L;       //  Wierd for REVERSE play,  we set this to zero on every data-load.

		
		bytestToPass  =      offsetFromDataStart;        //   Because we must traverse from the data START of the file

	//	TRACE(  "stream SEEK:   Origin,  then   %d  \n",    bytestToPass   );

	}
	else  if(    offsetFromDataStart    >    m_currentByteIdx     )
	{

			  //  Only get here after INITIALLY hitting the Play button ???

			  //  any time we hit the PlayButton,  m_currentByteIdx gets set to zero   2/2/10


//		ASSERT(  m_currentByteIdx ==  0   );   // *****WRONG...  Is not zero for SLOWED-DOWN speeds (old algo) 



		bytestToPass  =      offsetFromDataStart    -    m_currentByteIdx;  // *** Think need to do it this way for SLOWED-DOWN speeds (old algo)  2/6/10
	//	bytestToPass  =      offsetFromDataStart;     // *** CAREFULL,  if  m_currentByteIdx always eqyals zero when we get here, this is OK



	//	TRACE(  "stream SEEK:      ...Advance   %d  \n",    bytestToPass   );
	}
	else if(    offsetFromDataStart    ==   m_currentByteIdx     ) 
	{
								       //  This gets called repetively during  FOWARD-Play  (  m_currentByteIdx  is incremented as we play the file in forward directions).    2/2/10
		int   dummy =   9;    //   Does NOT get called at any time during Reverse-Play
	}
	






	if(    bytestToPass  >  0    )
	{

			//  Only get here after INITIAL hit of PlayButton,  and WHILE playing in reverse.

			//  This does NOT get called during forward play.  Only gets called when we change the file pointer,  play in reverse,  or  Hit the Continue play button


		if(    ( bytestToPass  %  4 )   !=   0     )			  //  need to always read on  32bit boundaries
			ASSERT( 0 );  



		if((   errorCode =       mmioSeek(    m_mmIO,    bytestToPass,    SEEK_CUR  )      )     ==  -1   )    
		{  
															//   ...think this was their error handling,    see   "Wassert.cpp"   


	//  Be CAREFUL   not to enclose    mmioSeek()    with   {  mmioGetInfo(),    mmioSetInfo()   }

			retErrorMesg  =     "StreamingAudioplayer::Seek_New_DataPosition  failed,   mmioSeek failed." ;
			return  false;        
		}



//		ASSERT(   m_currentByteIdx  ==  0    );   // only get here after INITIAL hit of PlayButton,  and WHILE playing in reverse. Either scenerio sets m_currentByteIdx = 0; 


		m_currentByteIdx   +=     bytestToPass;    // *** Think need to do it this way for SLOWED-DOWN speeds (old algo)  2/6/10
//		m_currentByteIdx   =       bytestToPass;
	}


	return  true;
}



											////////////////////////////////////////


bool    BitSourceStreamingMS::Fetch_Streaming_Samples_Direct(         long       startOffsettedByte,   //    absCur  OffsetFrom  FileStart'   
																									UINT      byteCountToRead,    
																									BYTE    *destBufferBytes,     //  destination  buffer
																									UINT     *retActualBytesRead,  
																			 						 long       srcBytesProcessed,																				
																									 bool&        retHitEOF,    
																									 bool		   fowardFlag,  
																									 short         auditionCode,   
																									bool   soundManControlsVolume,
																									 CString&   retErrorMesg    )
{						

	 //   'knockOutCode':    0:  NO knockout,     1: Knockout LEFT stereoChannel,    2: Knockout RIGHT stereoChannel



	//  **************   This is used by PitchScope,    BUT is NOT used by Navigator or Player   12/2011  *********************




    MMIOINFO     mmIOinfo;     // current status of <mmIO>
    int       errorCode = 0;
    UINT    byteIdx,   byteCountAdjusted;
	int       wordCnt =  0;
	UINT    byteCount = 0;      // mine, count the bytes read
	long     blockSize =  -1;
	UINT    fileOffset,    curFileOffset,      startOffset,  endOffset;



	retErrorMesg.Empty();


	if(  ! m_isInitialized   )
	{					
		ASSERT( 0 );
		retErrorMesg  =  "BitSourceAudioMS::Fetch_Streaming_Samples_Direct, bitsource has not been INITIALIZED.."  ;     // the file is probably NOT yet opened, etc.   5/07
		return  false;
	}





	if(           fowardFlag
		 &&   ( startOffsettedByte   %  4L   )   !=  0   )    //  are we alwaysfetching on  STEREO-Pair  Sample Boundaries
	{
		ASSERT( 0 );											  //  ...it affects my Audio MASKING for Stereo channels 
	}





	long   headerLength  =     44;

	long   realFileOffset  =    startOffsettedByte    +   headerLength;   //  ****** OK????   9/2003  

// **** Am I accounting for  the  Write-AHEAD  delay  for the BUFFER  ?????  ****************



	short   directionCode; 

	if(    fowardFlag    )   
		directionCode =   BitSource::FORWARD;
	else
		directionCode =   BitSource::BACKWARDS;




//	if(      !Seek_New_DataPosition(           startOffsettedByte,    retErrorMesg   )     )    ***** WHICH to use ?????    10/02********

	if(      !Seek_New_DataPosition_Fast(    startOffsettedByte,  directionCode,   retErrorMesg   )     )   //   ...maybe buggy????
//	if(      !Seek_New_DataPosition_Fast(    realFileOffset,    retErrorMesg   )     )    // **** Am I accounting for  the  Write-AHEAD  delay  for the BUFFER  ?????
		return   false;







			//	mmioGetinfo :    Retrieves information about a file opened by using the mmioOpen function.  This information
			//							allows the application to  'directly access'  the I/O buffer,  if the file is opened for buffered I/O.

    if(    errorCode =      mmioGetInfo(   m_mmIO,    &mmIOinfo,   0   )      != 0    )
    {
		retErrorMesg  =  "BitSourceStreamingMS::Fetch_Streaming_Samples_Direct   failed,    mmioGetInfo  failed." ;
		*retActualBytesRead  =   0;
		return   false;
    }
       	

	if(        m_wavVolumeSoundmanAddr  ==  NULL    
		&&   soundManControlsVolume   )
    {
		retErrorMesg  =  "BitSourceStreamingMS::Fetch_Streaming_Samples_Direct   failed,   m_wavVolumeSoundmanAddr  is NULL." ;
		*retActualBytesRead  =   0;
		return   false;
    }
 




	DWORD     cksizeOrig  =  m_mmChunkInfo.cksize;


    byteCountAdjusted  =     byteCountToRead;

    if(     byteCountAdjusted   >   m_mmChunkInfo.cksize    ) 
		byteCountAdjusted =   m_mmChunkInfo.cksize;       

    m_mmChunkInfo.cksize  -=      byteCountAdjusted;
    

	
	blockSize =      mmIOinfo.cchBuffer;     //   the size of the  internal I/O  buffer
	ASSERT( blockSize  > 0 );


	if(    srcBytesProcessed   ==   0    )   
		m_filesDataStart  =     mmIOinfo.lDiskOffset;       //   Save for later file navigation


	curFileOffset  =     mmIOinfo.lDiskOffset;  //   ...and get CURRENT offset  for each CALL
	startOffset     =     mmIOinfo.lDiskOffset;  


	if(    ( byteCountAdjusted  %  4 )   !=   0    )     // Always fetching on  LONG,  4-byte boundaries ???   ...then can EXPLOIT the fact
		ASSERT( 0 );




	bool    byteIsInSPitchObj  =    true;
	long    lastPixelBoundary =    -1;







// *******************************************************************************************************************
	char    chVal0=0,    chVal1=0,   chVal2=0,    chVal3=0;   //  Used to be below,  but got a  error   Run-Time Check Failure #3
// *******************************************************************************************************************




															//  Copy the bytes  from the IO to the buffer. 


	for(     byteIdx =0;        byteIdx <    byteCountAdjusted;        byteIdx++    )
	{															 

																	//   Are we ready to ADVANCE to another BlockRead...

		if(     mmIOinfo.pchNext   ==    mmIOinfo.pchEndRead    )
		{

				//   fileOffset  =   mmIOinfo.lDiskOffset;    ....DEBUG

			if(   (  errorCode =    mmioAdvance(   m_mmIO,    &mmIOinfo,    MMIO_READ   )     )  != 0   )
			{
				retErrorMesg  =     "BitSourceStreamingMS::Fetch_Streaming_Samples_Direct   failed,  mmioAdvance  failed." ;
				*retActualBytesRead  =   0;
				return   false;
			}



			/****
			unsigned long   end,  start,   numBytes;

			start =    (  unsigned long   )(  mmIOinfo.pchNext   );
			end  =    (  unsigned long   )(  mmIOinfo.pchEndRead   );   

			numBytes  =    end  -  start;
			****/


			/*******************************  
			unsigned long    numBytes; 
			unsigned long    numBytesHP; 

			HPSTR               endHP,  startHP;        //   typedef char _huge *    HPSTR
			unsigned long     end,     start;

			
// VoxSep
			start =    (  unsigned long   )(  m_mmIOinfo.pchNext  );     // ****** Now get casting errors here ????   WHY?   12/11/09 *****************8
			end  =    (  unsigned long   )(  m_mmIOinfo.pchEndRead  );   //    warning C4311: 'type cast' : pointer truncation from 'HPSTR' to 'unsigned long'
			numBytes =     end  -  start;


			startHP =     m_mmIOinfo.pchNext;         
			endHP  =     m_mmIOinfo.pchEndRead;   
			numBytesHP =     endHP  -  startHP;


			if(    numBytes  !=   numBytesHP   )
			{
				ASSERT( 0 );
				retErrorMesg =  "Fetch_Streaming_Samples_Direct  FAILED,  bad pointer arithmetic." ;
				return  false;
			}
			else
				numBytes =    numBytesHP;


			if(   numBytes  !=  0   )     //   Wierd,   who is this always zero???   
			{
				int    dummy =  8;
				ASSERT( 0 );
			}
			**************/





			fileOffset    =    mmIOinfo.lDiskOffset;    //     ....DEBUG		
	//		TRACE(  "	  ***mmioAdvance()    curFileOffset  =  %d,     numBytes =   %d    \n",    fileOffset,    numBytes   );   


			if(    mmIOinfo.pchNext    ==    mmIOinfo.pchEndRead    )
			{

// ****************************************************************************************
// ***************   WEIRD,  get  NEW bug  here   12/11  ****************************************
// ****************************************************************************************


//		 Happens if I  select far to left  of display when trying to create a NEW detection zone.  Select right side first and then d



				retErrorMesg  =     "BitSourceStreamingMS::Fetch_Streaming_Samples_Direct   failed,  corrupt file." ;
				*retActualBytesRead  =   0;
				return   false;
			}
		}
 



// *******************************************************************************************************************
	//	char                 chVal0,    chVal1,   chVal2,    chVal3;   **************  MOVED up so do not get a "Run-Time Check Failure #3"  12/11/09
// *******************************************************************************************************************

		unsigned char   byVal0,    byVal1,   byVal2,    byVal3;


		unsigned char   byVal,   writeVal;
		unsigned char   eraseVal  =    0;   //  was  127     {  0 - 255  }

		char   charVal,    charErase =  0;

			//  BAD:    *((BYTE*)destBufferBytes+byteIdx)  =       *((BYTE*)mmIOinfo.pchNext)++;  NEEDED to break into 2 lines.  **** jpm 
  		   //   WORKS:   *(   (BYTE*)destBufferBytes  + byteIdx   )  =   *(    ( BYTE* )mmIOinfo.pchNext    );      //  WORKS




		byVal    =      *(    ( BYTE* )mmIOinfo.pchNext    );     //  LOUSEY cast,   I should not have done it.   2/03
	    charVal =      *(                  mmIOinfo.pchNext    );     //   ...much BETTER,  is more intuitive with design


		short   byteMember  =    (  byteIdx  %  4L  );       //  which of the    4 Bytes in the LONG    do we have for this  ' '




		char                 signedChar    =  0;    // **** DEBUG only,  just to test clacs 
		unsigned char   revSampleVal =  0;

		unsigned short   sampleVal =   0;
		short                 signedShortSampleVal  =  0;




		if(    byteMember  ==  0   )   
		{
			byVal0 =   byVal;
			chVal0 =   charVal;
	    //  byVal =   eraseVal;    //   ...low byte.  Does NOT silence the track
		}
		else if(   byteMember  == 1   )   
		{
			byVal1 =   byVal;
			chVal1 =   charVal;
		//  byVal =   eraseVal;      ..Just this can silences the track


			/***
			if(    byVal1  >   127   )     //  range of   CHAR  is  { -128  to  127  }
			    signedChar  =     (char)(   (short)byVal1  -  256      );     //   byVal1's BITS  and  signedChar's BITS  have same positions
			else																	         //   ..gets values to be signed & continuous as cross Zero-axis   2/03
				signedChar  =     (char)(             byVal1       );

			ASSERT(   charVal   ==    signedChar   );    //  DEBUG check that my calcs are OK 



			if(   signedChar  >  0   )
				revSampleVal  =         (unsigned char)signedChar;
			else
				revSampleVal  =        (unsigned char)(  256   +    (short)signedChar   );

			ASSERT(   revSampleVal   ==   byVal1    );
			***/

		
			signedShortSampleVal   =     (    (short)chVal1   *  128    )     +     (short)chVal0;


			long     percentVolRedux  =   100;

			short   signedShortAtten  =    (short)(    (  (long)signedShortSampleVal     *  percentVolRedux  )  / 100L     );


			char   nuCharVal  =      (char)(  signedShortAtten  >>  8   );

			byVal =      (BYTE)nuCharVal; 


		}																																	
		else if(   byteMember   == 2   )			//  RIGHT  channel   below
		{
			byVal2 =   byVal;
			chVal2 =   charVal;
			//  byVal =   eraseVal;    //   ...low byte.  Does NOT silence the track
		}
		else if(   byteMember   == 3   )
		{
			byVal3 =   byVal;
			chVal3 =   charVal;
			//   byVal =   eraseVal;    //   ..Just this silences the track
		}
		else   
			ASSERT( 0 );





				//  Find out  if this Sample( 4 bytes )  is CONTAINED in a  ScalePitchSubject  by animationMask


		long   sampleIdx  =    (  startOffsettedByte  +   byteIdx   )    /  m_bytesPerSample;      
				//  ASSERT(    (   ( startOffsettedByte  +  byteIdx )    %    m_bytesPerSample    )  ==   0   );   // Make sure we have 1st




			
		if(    byteMember  ==  3    )   //  are we at the LAST Byte in the  LONG( Sample ), so NOW we write out the 4 Sanple-BYTES
		{

															//   Attenuate signal by settings of VOLUME controls
		
			long   leftSample16    =    chVal1;
			leftSample16  =      (  leftSample16   <<  8  )    |    ( BYTE )chVal0;    //   Need cast so   '|'   will  pack  with negative numbers 

			long   rightSample16    =    chVal3;
			rightSample16  =      (  rightSample16   <<  8  )    |    ( BYTE )chVal2;    //   Need cast so   '|'   will  pack  with negative numbers 



		//	long   attenSampleLeft;
		//	long   attenSampleRight;
			long   attenSampleLeft   =    leftSample16;    //  *****  1/10   NOW initializing with the UN MODIFIED.   WATCH for trouble
			long   attenSampleRight =    rightSample16;





//		{  NORMAL,     MIDIandSAMPLE,     AllLEFT,  AllRIGHT,     JUSTMIDI,     MIDIandLEFT,   MIDIandRIGHT    };  

//																							MIDIandSAMPLE( mid plus stereo ),    MIDIandLEFT( midi and JustLeft WAV  )			



			if(          auditionCode ==   AnimeStream::JUSTMIDI    )
			{
				attenSampleLeft     =    0;     //  sometimes we want the direct,  un-attenuated data, like to creat a FFT transform
				attenSampleRight   =    0; 
			}
			else if(        auditionCode ==   AnimeStream::AllLEFT      //  left StereoMix channel,   Just Wave     ...trying to de WAVE-Only options, but Normal is a pain.  6/07
				        ||   auditionCode ==   AnimeStream::AllRIGHT    //  right StereoMix channel,   Just Wave
						||   auditionCode ==   AnimeStream::NORMAL     //  center StereoMix channel,   Just Wave
				)  
			{


		//		attenSampleLeft     =        (  leftSample16       *      m_reduxInVolumeWhenPlaying )    / 100L;   // *** BAD Get here form detect zone ??  12/09 BUG
		//		attenSampleRight   =        (  rightSample16     *       m_reduxInVolumeWhenPlaying)    / 100L;

				attenSampleLeft     =    leftSample16;     //   we want the direct,  un-attenuated data, for the Wave-Only AuditionCodes
				attenSampleRight   =    rightSample16;    //   *****   1/16/10  -  Put this back so that DFT will have access to the full signal.  In the filtering stage it will adjust the volume for the DFT's image.
	


				if(        auditionCode ==   AnimeStream::NORMAL  
					&&   soundManControlsVolume  )
				{
// **** WEIRD the way that I do this...   I "FURTHER reduce the volme because the WAV is just too loud"   by  reducing with the fixed value of m_reduxInVolumeWhenPlaying (50)   1/10 

					attenSampleLeft     =                     (  leftSample16       *      (*m_wavVolumeSoundmanAddr)   )  / 100L;   
					attenSampleLeft     =        ( attenSampleLeft   *         m_reduxInVolumeWhenPlaying )    / 100L;  // FURTHER reduce the volme because the WAV is just too loud

					attenSampleRight   =                     (  rightSample16          *      (*m_wavVolumeSoundmanAddr)    )  / 100L;
					attenSampleRight   =        (  attenSampleRight     *         m_reduxInVolumeWhenPlaying    )  / 100L;
				}
			}
			else
			{
					// **** FOR what values do I get here ????    1/10     MIDIandSAMPLE,


				if(   soundManControlsVolume   )   //  Get here when just playing   12/09 BUG
				{
					attenSampleLeft     =                   (  leftSample16       *      (*m_wavVolumeSoundmanAddr)   )  / 100L;
					attenSampleLeft     =        ( attenSampleLeft   *         m_reduxInVolumeWhenPlaying )    / 100L;

					attenSampleRight   =                   (  rightSample16          *      (*m_wavVolumeSoundmanAddr)    )  / 100L;
					attenSampleRight   =        (  attenSampleRight     *         m_reduxInVolumeWhenPlaying    )  / 100L;
				}
				else
				{  attenSampleLeft     =    leftSample16;     //  sometimes we want the direct,  un-attenuated data, like to creat a FFT transform
					attenSampleRight   =    rightSample16; 
				}
			}




			char    nuCharHiLeft    =       (char)(    ( attenSampleLeft      &    0xffffff00    )  >> 8     );
			char    nuCharLoLeft   =        (char)(    (  attenSampleLeft    &    0x000000ff  )              );    

			char    nuCharHiRight   =      (char)(    (  attenSampleRight    &    0xffffff00    )   >> 8     );
			char    nuCharLoRight   =      (char)(    (  attenSampleRight    &   0x000000ff  )               );    






			/***   DEBUG:     only if  attenSampleLeft,  right are NOT reduced.  
			if(       nuCharHiLeft    !=    chVal1
				||   nuCharLoLeft    !=    chVal0  
				||   nuCharHiRight    !=    chVal3
				||   nuCharLoRight    !=    chVal2    )
			{
				ASSERT( 0 );    //    int  dummy =  0;
			}
			***/


			chVal0  =      nuCharLoLeft;			//  replace with new ATTENUATED values of volume controls
			chVal1  =      nuCharHiLeft;
			chVal2  =      nuCharLoRight;
			chVal3  =      nuCharHiRight;




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
					writeVal =   byVal;	       //  copy it out anyway  
				break;
			}

		}   //  if(    byteMember  ==  3



		( BYTE* )mmIOinfo.pchNext++;    	

		byteCount++;
		m_currentByteIdx++;
	}





	endOffset   =      mmIOinfo.lDiskOffset;  

/***
	TRACE(  "     ***Fetch_Streaming_Samples_OFFSETTED():  Offsets[  %d,   %d,   Diff=  %d,   Abs  %d  ]   Ask[ %d,  returned %d ]   BytesProcessAfter[ %d ]    cksizeOrig[ %d ]   \n" ,   
						startOffset,    endOffset,      ( endOffset -  startOffset ),   absCurOffsetFromFileStart,   
						byteCountToRead,    byteCountAdjusted,   ( srcBytesProcessed  +  byteCountToRead ),   cksizeOrig   );
***/




				//  mmioSetInfo()  :    updates the information retrieved by the mmioGetInfo function about a file opened 
				//								by using the mmioOpen function. Use this function to terminate   'direct buffer access' 
				//								of a file opened for buffered I/O.


    if(    (  errorCode =     mmioSetInfo(   m_mmIO,    &mmIOinfo,   0   )     )      != 0    )
	{
		retErrorMesg  =     "StreamingAudioplayer::Fetch_Streaming_Samples_Direct   failed,  mmioSetInfo  failed." ;
		*retActualBytesRead  =   0;
		return   false;
	}



	*retActualBytesRead  =     byteCountAdjusted;

	return   true;      
}






											////////////////////////////////////////


bool    BitSourceStreamingMS::Fetch_Streaming_Samples_Direct_PPlayer( long       startOffsettedByte,   //    absCur  OffsetFrom  FileStart'   
																										UINT      byteCountToRead,    

																										BYTE    *destBufferBytes,     //  destination  buffer

																										UINT     *retActualBytesRead,  
																			 							 long       srcBytesProcessed,																				
																										 bool&        retHitEOF,    
																										 bool		   fowardFlag,  

																										 short         auditionCode,   // NOT really used. Omit??   1/10

																										 bool                 soundManControlsVolume,
																										 SndSample*    sndSample,   //  Is a switch whether or not to detect pitch

																										 short        rightChannelPercent,   //  how to spit the Streo signal for  sndSample 

																										 CString&   retErrorMesg    )
{						

	 //   'knockOutCode':    0:  NO knockout,     1: Knockout LEFT stereoChannel,    2: Knockout RIGHT stereoChannel





    MMIOINFO     mmIOinfo;     // current status of <mmIO>
    int       errorCode = 0;
    UINT    byteIdx,   byteCountAdjusted;
	int       wordCnt =  0;
	UINT    byteCount = 0;      // mine, count the bytes read
	long     blockSize =  -1;
	UINT    fileOffset,    curFileOffset,      startOffset,  endOffset;



	retErrorMesg.Empty();

	if(  ! m_isInitialized   )
	{					
		ASSERT( 0 );
		retErrorMesg  =  "BitSourceStreamingMS::Fetch_Streaming_Samples_Direct_PPlayer, bitsource has not been INITIALIZED.."  ;     // the file is probably NOT yet opened, etc.   5/07
		return  false;
	}


	if(           fowardFlag
		 &&   ( startOffsettedByte   %  4L   )   !=  0   )    //  are we alwaysfetching on  STEREO-Pair  Sample Boundaries
	{
		ASSERT( 0 );											  //  ...it affects my Audio MASKING for Stereo channels 
	}

	ASSERT(   rightChannelPercent >= 0    &&    rightChannelPercent <=  100   );



	bool     doPitchDetectionCalcs =   false;
	char   *destSndSample =   NULL;   

	if(   sndSample  !=  NULL  )
	{
		doPitchDetectionCalcs =   true;
		
		destSndSample  =     sndSample->Get_StartByte();      //  Get the startByte from the 8-bit SndSample that we will copy to.
		ASSERT(  destSndSample  ); 
	}
	else
		doPitchDetectionCalcs =   false;




	long   headerLength  =     44;

	long   realFileOffset  =    startOffsettedByte    +   headerLength;   //  ****** OK????   9/2003  


	short   directionCode; 

	if(    fowardFlag    )   
		directionCode =   BitSource::FORWARD;
	else
		directionCode =   BitSource::BACKWARDS;




//	if(      !Seek_New_DataPosition(           startOffsettedByte,    retErrorMesg   )     )    ***** WHICH to use ?????    10/02********

	if(      ! Seek_New_DataPosition_Fast(    startOffsettedByte,  directionCode,    retErrorMesg   )     )   //   ...maybe buggy????
//	if(      !Seek_New_DataPosition_Fast(    realFileOffset,    retErrorMesg   )     )    // **** Am I accounting for  the  Write-AHEAD  delay  for the BUFFER  ?????
		return   false;





			//	mmioGetinfo :    Retrieves information about a file opened by using the mmioOpen function.  This information
			//							allows the application to  'directly access'  the I/O buffer,  if the file is opened for buffered I/O.

    if(    errorCode =      mmioGetInfo(   m_mmIO,    &mmIOinfo,   0   )      != 0    )
    {
		retErrorMesg  =  "BitSourceStreamingMS::Fetch_Streaming_Samples_Direct_PPlayer   failed,    mmioGetInfo  failed." ;
		*retActualBytesRead  =   0;
		return   false;
    }
       	

	if(        m_wavVolumeSoundmanAddr  ==  NULL    
		&&   soundManControlsVolume   )
    {
		retErrorMesg  =  "BitSourceStreamingMS::Fetch_Streaming_Samples_Direct_PPlayer  failed,   m_wavVolumeSoundmanAddr  is NULL." ;
		*retActualBytesRead  =   0;
		return   false;
    }
 




	DWORD   cksizeOrig  =   m_mmChunkInfo.cksize;


    byteCountAdjusted  =     byteCountToRead;

    if(     byteCountAdjusted   >   m_mmChunkInfo.cksize    ) 
		byteCountAdjusted =   m_mmChunkInfo.cksize;       

    m_mmChunkInfo.cksize  -=      byteCountAdjusted;
    

	
	blockSize =      mmIOinfo.cchBuffer;     //   the size of the  internal I/O  buffer
	ASSERT( blockSize  > 0 );


	if(    srcBytesProcessed   ==   0    )   
		m_filesDataStart  =     mmIOinfo.lDiskOffset;       //   Save for later file navigation


	curFileOffset  =     mmIOinfo.lDiskOffset;  //   ...and get CURRENT offset  for each CALL
	startOffset     =     mmIOinfo.lDiskOffset;  


	if(    ( byteCountAdjusted  %  4 )   !=   0    )     // Always fetching on  LONG,  4-byte boundaries ???   ...then can EXPLOIT the fact
		ASSERT( 0 );





// *******************************************************************************************************************
	char    chVal0=0,    chVal1=0,   chVal2=0,    chVal3=0;   //  When used to be below,  but got a  error   Run-Time Check Failure #3 ( this bug was in PitchScope 1.0





																								//  Copy the bytes  from the IO to the buffer. 


	for(     byteIdx =0;        byteIdx <    byteCountAdjusted;        byteIdx++    )
	{															 

																	//   Are we ready to ADVANCE to another BlockRead...

		if(     mmIOinfo.pchNext   ==    mmIOinfo.pchEndRead    )
		{

				//   fileOffset  =   mmIOinfo.lDiskOffset;    ....DEBUG

			if(   (  errorCode =    mmioAdvance(   m_mmIO,    &mmIOinfo,    MMIO_READ   )     )  != 0   )
			{
				retErrorMesg  =     "StreamingAudioplayer::Fetch_Streaming_Samples_Direct_PPlayer   failed,  mmioAdvance  failed." ;
				*retActualBytesRead  =   0;
				return   false;
			}



			/*****
			unsigned long   end,  start,   numBytes;

			start =    (  unsigned long   )(  mmIOinfo.pchNext   );
			end  =    (  unsigned long   )(  mmIOinfo.pchEndRead   );   
			numBytes  =    end  -  start;
			*****/


			/********************************   NOT needed,  just a weird debug technique
			unsigned long    numBytes; 
			unsigned long    numBytesHP; 

			HPSTR               endHP,  startHP;        //   typedef char _huge *    HPSTR
			unsigned long     end,     start;

			
// Player
			start =    (  unsigned long   )(  m_mmIOinfo.pchNext  );     // ****** Now get casting errors here ????   WHY?   12/11/09 *****************8
			end  =    (  unsigned long   )(  m_mmIOinfo.pchEndRead  );   //    warning C4311: 'type cast' : pointer truncation from 'HPSTR' to 'unsigned long'
			numBytes =     end  -  start;


			startHP =     m_mmIOinfo.pchNext;         
			endHP  =     m_mmIOinfo.pchEndRead;   
			numBytesHP =     endHP  -  startHP;


			if(    numBytes  !=   numBytesHP   )
			{
				ASSERT( 0 );
				retErrorMesg =  "Fetch_Streaming_Samples_Direct_PPlayer  FAILED,  bad pointer arithmetic." ;
				return  false;
			}
			else
				numBytes =    numBytesHP;


			if(   numBytes  !=  0   )     //   Wierd,   who is this always zero???    Looks like I NEVER use 'numBytes'    ...this is just a debug technique
			{
				int    dummy =  8;
				ASSERT( 0 );
			}
			******/



			fileOffset    =    mmIOinfo.lDiskOffset;    //     ....DEBUG		
	//		TRACE(  "	  ***mmioAdvance()    curFileOffset  =  %d,     numBytes =   %d    \n",    fileOffset,    numBytes   );   


			if(    mmIOinfo.pchNext    ==    mmIOinfo.pchEndRead    )
			{
				retErrorMesg  =     "StreamingAudioplayer::Fetch_Streaming_Samples_Direct_PPlayer   failed,  corrupt file." ;
				*retActualBytesRead  =   0;
				return   false;
			}
		}
 


		unsigned char   byVal0,    byVal1,   byVal2,    byVal3;
		unsigned char   byVal,   writeVal;
		unsigned char   eraseVal  =    0;   //  was  127     {  0 - 255  }

		char   charVal,    charErase =  0;

			//  BAD:        *((BYTE*)destBufferBytes+byteIdx)  =       *((BYTE*)mmIOinfo.pchNext)++;  NEEDED to break into 2 lines.  **** jpm 
  		   //   WORKS:   *(   (BYTE*)destBufferBytes  + byteIdx   )  =   *(    ( BYTE* )mmIOinfo.pchNext    );      //  WORKS




		byVal    =      *(    ( BYTE* )mmIOinfo.pchNext    );     //  LOUSY cast,   I should not have done it.   2/03
	    charVal =      *(                  mmIOinfo.pchNext    );     //   ...much BETTER,  is more intuitive with design



		short   byteMember  =    (  byteIdx  %  4L  );       //  which of the    4 Bytes in the LONG    do we have for this  ' '

		char                 signedChar    =  0;    // **** DEBUG only,  just to test clacs 
		unsigned char   revSampleVal =  0;

		unsigned short   sampleVal =   0;
		short                 signedShortSampleVal  =  0;




		if(    byteMember  ==  0   )   
		{
			byVal0 =   byVal;
			chVal0 =   charVal;
	    //  byVal =   eraseVal;    //   ...low byte.  Does NOT silence the track
		}
		else if(   byteMember  == 1   )   
		{
			byVal1 =   byVal;
			chVal1 =   charVal;
		//  byVal =   eraseVal;      ..Just this can silences the track

			/***
			if(    byVal1  >   127   )     //  range of   CHAR  is  { -128  to  127  }
			    signedChar  =     (char)(   (short)byVal1  -  256      );     //   byVal1's BITS  and  signedChar's BITS  have same positions
			else																	         //   ..gets values to be signed & continuous as cross Zero-axis   2/03
				signedChar  =     (char)(             byVal1       );

			ASSERT(   charVal   ==    signedChar   );    //  DEBUG check that my calcs are OK 


			if(   signedChar  >  0   )
				revSampleVal  =         (unsigned char)signedChar;
			else
				revSampleVal  =        (unsigned char)(  256   +    (short)signedChar   );

			ASSERT(   revSampleVal   ==   byVal1    );
			***/
		}																																	
		else if(   byteMember   == 2   )			//  RIGHT  channel   below
		{
			byVal2 =   byVal;
			chVal2 =   charVal;
			//  byVal =   eraseVal;    //   ...low byte.  Does NOT silence the track
		}
		else if(   byteMember   == 3   )
		{
			byVal3 =   byVal;
			chVal3 =   charVal;
			//   byVal =   eraseVal;    //   ..Just this silences the track
		}
		else   
			ASSERT( 0 );






			
		if(    byteMember  ==  3    )   //  are we at the LAST Byte in the  LONG( Sample ), so NOW we write out the 4 Sanple-BYTES
		{

		
			long   leftSample16    =    chVal1;
			leftSample16  =      (  leftSample16   <<  8  )    |    ( BYTE )chVal0;    //   Need cast so   '|'   will  pack  with negative numbers 

			long   rightSample16    =    chVal3;
			rightSample16  =      (  rightSample16   <<  8  )    |    ( BYTE )chVal2;    //   Need cast so   '|'   will  pack  with negative numbers 


			long   attenSampleLeft   =    leftSample16;    //  *****  1/10   NOW initializing with the UN MODIFIED.   WATCH for trouble
			long   attenSampleRight =    rightSample16;

			long   origSampleLeft   =    leftSample16;      //  *****  1/10   NOW initializing with the UN MODIFIED.   WATCH for trouble
			long   origSampleRight =    rightSample16;




																	//  Copy the  ORIGINAL values  out to the 8-bit SndSample,  but apply stero balance calcs   1/10

			if(   doPitchDetectionCalcs   )
			{
				double   leftVal   =     chVal1;
				double   rightVal =     chVal3;
				double   leftChannelPercentFlt    =     (  100.0 - (double)rightChannelPercent  )      / 100.0;                
				double   rightChannelPercentFlt  =                   (double)rightChannelPercent          / 100.0;


	//			*destSndSample   =      (char)(         (  leftVal  +  rightVal  ) /2     );    //   TransformMap::CENTEREDj     Create a MONO signal,  by averaging left and right    

				short   combinedVal =    (short)(	       ( leftVal   *  leftChannelPercentFlt )     +     ( rightVal  *  rightChannelPercentFlt )	 		);
											
				if(         combinedVal   >   127  )       //  range of   CHAR  is  { -128  to  127  }
					combinedVal =    127;
				else if(   combinedVal   <  -127  )
					combinedVal =   -127;           //  we have hit this ...is it a problem??  1/17/10

				*destSndSample  =      (char)combinedVal;
				destSndSample++;

			}   //  if(   doPitchDetectionCalcs 


																								//   Attenuate signal by settings of VOLUME controls


//		{  NORMAL,     MIDIandSAMPLE,     AllLEFT,  AllRIGHT,     JUSTMIDI,     MIDIandLEFT,   MIDIandRIGHT    };  
//																							MIDIandSAMPLE( mid plus stereo ),    MIDIandLEFT( midi and JustLeft WAV  )			


			/***********************  KEEP for a while in case I want this back   1/17/10
			if(          auditionCode ==   AnimeStream::JUSTMIDI    )
			{
				attenSampleLeft     =    0;     //  sometimes we want the direct,  un-attenuated data, like to creat a FFT transform
				attenSampleRight   =    0; 
			}
			else if(        auditionCode ==   AnimeStream::AllLEFT      //  left StereoMix channel,   Just Wave     ...trying to de WAVE-Only options, but Normal is a pain.  6/07
				        ||   auditionCode ==   AnimeStream::AllRIGHT    //  right StereoMix channel,   Just Wave
						||   auditionCode ==   AnimeStream::NORMAL     //  center StereoMix channel,   Just Wave
				)  
			{

				attenSampleLeft     =    leftSample16;     //   we want the direct,  un-attenuated data, for the Wave-Only AuditionCodes
				attenSampleRight   =    rightSample16;    //   *****   1/16/10  -  Put this back so that DFT will have access to the full signal.  In the filtering stage it will adjust the volume for the DFT's image.
	

				if(        auditionCode ==   AnimeStream::NORMAL  
					&&   soundManControlsVolume  )
				{
// **** WEIRD the way that I do this...   I "FURTHER reduce the volme because the WAV is just too loud"   by  reducing with the fixed value of m_reduxInVolumeWhenPlaying (50)   1/10 

					attenSampleLeft     =                     (  leftSample16       *      (*m_wavVolumeSoundmanAddr)   )  / 100L;   
					attenSampleLeft     =        ( attenSampleLeft   *         m_reduxInVolumeWhenPlaying )    / 100L;  // FURTHER reduce the volme because the WAV is just too loud

					attenSampleRight   =                     (  rightSample16          *      (*m_wavVolumeSoundmanAddr)    )  / 100L;
					attenSampleRight   =        (  attenSampleRight     *         m_reduxInVolumeWhenPlaying    )  / 100L;
				}
			}
			else
			{
					// **** FOR what values do I get here ????    1/10     MIDIandSAMPLE,


				if(   soundManControlsVolume   )   //  Get here when just playing   12/09 BUG
				{
					attenSampleLeft     =                   (  leftSample16       *      (*m_wavVolumeSoundmanAddr)   )  / 100L;
					attenSampleLeft     =        ( attenSampleLeft   *         m_reduxInVolumeWhenPlaying )    / 100L;

					attenSampleRight   =                   (  rightSample16          *      (*m_wavVolumeSoundmanAddr)    )  / 100L;
					attenSampleRight   =        (  attenSampleRight     *         m_reduxInVolumeWhenPlaying    )  / 100L;
				}
				else
				{  attenSampleLeft     =    leftSample16;     //  sometimes we want the direct,  un-attenuated data, like to creat a FFT transform
					attenSampleRight   =    rightSample16; 
				}
			}
			*****/
											//	  BELOW is  From   auditionCode ==   AnimeStream::NORMAL   &&   soundManControlsVolume    1/17/10

			ASSERT(   auditionCode ==   AnimeStream::NORMAL    );   //  do I want other cases for this ???    1/10
			ASSERT(   soundManControlsVolume   );


			attenSampleLeft     =     (  leftSample16       *      (*m_wavVolumeSoundmanAddr)   )  / 100L;   
			attenSampleRight   =     (  rightSample16     *      (*m_wavVolumeSoundmanAddr)    )  / 100L;

			/*****  Very SLOPPY,  for PitchPlayer and PitchScope.    Think we can leave it as it is   1/10

			attenSampleLeft     =        ( attenSampleLeft   *         m_reduxInVolumeWhenPlaying )    / 100L;  // FURTHER reduce the volme because the WAV is just too loud
			attenSampleRight   =        (  attenSampleRight     *         m_reduxInVolumeWhenPlaying    )  / 100L;
			*****/




																								//  Finally, write out the modified bytes for the sound buffer

			char    nuCharHiLeft    =       (char)(    ( attenSampleLeft      &    0xffffff00    )  >> 8     );
			char    nuCharLoLeft   =        (char)(    (  attenSampleLeft    &    0x000000ff  )              );    

			char    nuCharHiRight   =      (char)(    (  attenSampleRight    &    0xffffff00    )   >> 8     );
			char    nuCharLoRight   =      (char)(    (  attenSampleRight    &   0x000000ff  )               );    

			chVal0  =      nuCharLoLeft;			//  replace with new ATTENUATED values of volume controls
			chVal1  =      nuCharHiLeft;
			chVal2  =      nuCharLoRight;
			chVal3  =      nuCharHiRight;


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
					writeVal =   byVal;	       //  copy it out anyway  
				break;
			}

		}   //  if(    byteMember  ==  3



		( BYTE* )mmIOinfo.pchNext++;    	

		byteCount++;
		m_currentByteIdx++;
	}



	endOffset   =      mmIOinfo.lDiskOffset;  

/***
	TRACE(  "     ***Fetch_Streaming_Samples_OFFSETTED():  Offsets[  %d,   %d,   Diff=  %d,   Abs  %d  ]   Ask[ %d,  returned %d ]   BytesProcessAfter[ %d ]    cksizeOrig[ %d ]   \n" ,   
						startOffset,    endOffset,      ( endOffset -  startOffset ),   absCurOffsetFromFileStart,   
						byteCountToRead,    byteCountAdjusted,   ( srcBytesProcessed  +  byteCountToRead ),   cksizeOrig   );
***/


				//  mmioSetInfo()  :    updates the information retrieved by the mmioGetInfo function about a file opened 
				//								by using the mmioOpen function. Use this function to terminate   'direct buffer access' 
				//								of a file opened for buffered I/O.

    if(    (  errorCode =     mmioSetInfo(   m_mmIO,    &mmIOinfo,   0   )     )      != 0    )
	{
		retErrorMesg  =     "StreamingAudioplayer::Fetch_Streaming_Samples_Direct_PPlayer   failed,  mmioSetInfo  failed." ;
		*retActualBytesRead  =   0;
		return   false;
	}


	*retActualBytesRead  =     byteCountAdjusted;

	return   true;      
}





											////////////////////////////////////////


SndSample*     BitSourceStreamingMS::Create_8bit_Sample_Segment(   long  startOffset,   long  endOffset,    
															                                          short  channelCode,   CString&  retErrorMesg  )
{

	  //  Now can create a Stereo  SndSample  with channelCode  ==  SEPARATEstereo


														//   alloc and populate the  SHORT-SndSample  for OnTheFly  ReSyntheseis

	SndSample  *nuSample =  NULL;


	retErrorMesg.Empty();


	if(  ! m_isInitialized   )
	{					
		ASSERT( 0 );
		retErrorMesg  =  "BitSourceAudioMS::Create_8bit_Sample_Segment, bitsource has not been INITIALIZED.."  ;     // the file is probably NOT yet opened, etc.   5/07
		return  false;
	}



//	long   numChannelSamples     =       endOffset   -   startOffset;      //  Total samples in ONE Channel of 8-bit(  also number of BYTES for 8-bit 

	long   numChannelSamples     =       endOffset   -   startOffset    +1;    // ******  INCLUSIVE counting *****


	if(   numChannelSamples  <=  0   )
	{
		retErrorMesg =  "BitSourceStreamingMS::Create_8bit_Sample_Segment  faile,  bad input range." ;
		return  NULL;
	}

	long   totalSegmentBytes  =     (  numChannelSamples  *  m_bytesPerSample  );



	int   totBytes     =     Number_Bytes();
	if(    totBytes  <=  0   )
	{
		retErrorMesg =  "BitSourceStreamingMS::Create_8bit_Sample_Segment  failed,  no bytes." ;
		return  NULL;
	}


	if(   (  nuSample  =    new    SndSample(   numChannelSamples,   m_chunkSize  )  )    ==  NULL  )
	{
		retErrorMesg =  "BitSourceStreamingMS::Create_8bit_Sample_Segment  faile,  could not alloc  SndSample." ;
		return  NULL;
	}   



	char  *dest =  NULL,   *destRight =  NULL;  

	if(    channelCode  ==     TransformMap::SEPARATEstereo    )
	{
		if(    ! nuSample->Make_Stereo(  retErrorMesg  )     )
			return  false;

		 destRight =    nuSample->Get_StartByte_RightStereo(); 
	}

	dest =    nuSample->Get_StartByte();      //  the secondary 8-bit buffers   ( ? from  InverseDFT  )




	long     bytesRead =   0L;
	bool     keepGoing =    true;
	long      loopCount =  0;
	long      startOffsettedByte  =    startOffset   *    m_bytesPerSample;
	UINT     byteCountToRead  =    m_bufferSize;


	bool     retHitEOF =    false;   



	while(   keepGoing   )
	{

		UINT     retActualBytesRead=0;   //   ,   retActualBytesReadRight=0;
	//	bool      detectingLeftStereo =  true;  // not important ???
		long      srcBytesProcessedParm =  2;  //  anything but zero has no impact 



// ***************************************************************************************************************************
// ******* RECENTLY  12/11     got a bug HERE in  BitSourceStreamingMS::Fetch_Streaming_Samples_Direct()     ...see Notes in function.  ***********   


		if(     ! Fetch_Streaming_Samples_Direct(     startOffsettedByte,   //    absCur  OffsetFrom  FileStart'   
																		byteCountToRead,    
														  ( BYTE*  )m_readBuffersBits,     //  destination  buffer
																	  &retActualBytesRead,  
																		srcBytesProcessedParm,																				
																		retHitEOF,    
																		true,  
																		0,   
																		false,     //   soundManControlsVolume,   NO, we want the direct data
																		retErrorMesg      )    )
		{
			int   dummy =   9;

			keepGoing =   false;

//			return  false;   ***** TEMP,  see if it can  RECOVER ********
		}




//		ASSERT(    byteCountToRead  ==   retActualBytesRead   );    // ???? Not a problem ???     9/03 
		ASSERT(   ( retActualBytesRead  %  m_bytesPerSample )  ==  0   );      //  must stay on  16bit stereoSample  boundaries

		if(   retHitEOF   )
		{
			int  dummy =  9;
		}




																	//  Now copy  from buffer to  8bit Sample
		char  *src        =    m_readBuffersBits;
//		char  *srcRight =  NULL;

//		if(    channelCode  ==     TransformMap::SEPARATEstereo    )
//			srcRight =   m_readBuffersBitsRight; 


		char   ch0,  ch1,  ch2,  ch3;



		if(      channelCode  ==     TransformMap::LEFTj    )      //    leftStereo   )
		{

			for(    long  k=0;     k<  (long)( retActualBytesRead /4 );      k++   )     
			{

/*************************************************************************   TEMP,  put it back   12/09
				src++; 				
				*dest =   *src;               //  left  
				src++;    
				dest++;

				src++;  
				//  *destRight =  *src;         //  right
				src++;    
				//  destRight++;
***/

								ch0 =  *src; 
				src++; 	
				*dest =   *src;               //  left   ( the SECOND byte of the 16bit sample has the 8bit value that we use to create a SndSample 


								ch1 =  *src; 
				src++;    


				dest++;


								ch2 =  *src; 
				src++;  
				//  *destRight =  *src;         //  right


								ch3 =  *src; 
				src++;    
				//  destRight++;





				bytesRead  +=    4L;

				if(     bytesRead   >=   totalSegmentBytes   )
				{
					keepGoing =   false;
					break;
				}
			}
		}

		else if(      channelCode  ==     TransformMap::RIGHTj    )       //   right channel
		{

			for(    long  k=0;     k<  (long)( retActualBytesRead /4 );      k++   )     
			{
				src++; 				
				//  *destLeft =   *src;           //  left  
				src++;    
				//  destLeft++;

				src++;  
				*dest =  *src;              //  right
				src++;    
				dest++;


				bytesRead  +=    4L;

				if(     bytesRead   >=   totalSegmentBytes   )
				{
					keepGoing =   false;
					break;
				}
			}
		}


		else if(      channelCode  ==     TransformMap::CENTEREDj    )    
		{
			short   rightVal,  leftVal;

			for(    long  k=0;     k<  (long)( retActualBytesRead /4 );      k++   )     
			{



				src++; 							//  left  
				//  *destLeft =   *src;          
				leftVal =   *src;  
				src++;    
				//  destLeft++;


				src++;							//  right
				// *dest =  *src;             
				rightVal =   *src;  
				src++;    



				*dest =  (      ( leftVal  +  rightVal )/2     );    //  create mon by averaging

				dest++;


				bytesRead  +=    4L;

				if(     bytesRead   >=   totalSegmentBytes   )
				{
					keepGoing =   false;
					break;
				}
			}
		}
		else if(      channelCode  ==     TransformMap::SEPARATEstereo    )       
		{

			short   rightVal,  leftVal;

			for(    long  k=0;     k<  (long)( retActualBytesRead /4 );      k++   )     
			{


				src++; 							//  left  
				//  *destLeft =   *src;          
				leftVal =   *src;  
				src++;    
				//  destLeft++;

				src++;							//  right
				//   *dest =  *src;             
				rightVal =   *src;  
				src++;    



				*dest       =  leftVal;    
				dest++;

				*destRight =  rightVal;    
				destRight++;


				bytesRead  +=    4L;

				if(     bytesRead   >=   totalSegmentBytes   )
				{
					keepGoing =   false;
					break;
				}
			}
		}
		else
			ASSERT( 0 );




		loopCount++;  

		startOffsettedByte   +=     byteCountToRead;     //  advance the File Offset

		if(          bytesRead   >=   totalSegmentBytes
			   ||   retHitEOF    )
			keepGoing =   false;

	}   //   while(   keepGoing



	return  nuSample;
}



											////////////////////////////////////////


bool     BitSourceStreamingMS::Copy_To_8bit_Sample_Segment(   long  startOffset,   long  numSamplesToRead,       short  channelCode,    
															                                             SndSample&  sndSample,   CString&  retErrorMesg  )
{

	retErrorMesg.Empty();


	if(  ! m_isInitialized   )
	{					
		ASSERT( 0 );
		retErrorMesg  =  "BitSourceAudioMS::Copy_To_8bit_Sample_Segment, bitsource has not been INITIALIZED.."  ;     // the file is probably NOT yet opened, etc.   5/07
		return  false;
	}



//	long   numSamplesToRead     =       endOffset   -   startOffset    +1;    // ******  INCLUSIVE counting *****

	if(   numSamplesToRead  <=  0   )
	{
		retErrorMesg =  "BitSourceStreamingMS::Copy_To_8bit_Sample_Segment  failed,  bad input range." ;
		return  NULL;
	}

	long   totalSegmentBytes  =     (  numSamplesToRead  *  m_bytesPerSample  );  // this is OK for this to be  less than the samples size 



	int   totBytes     =     Number_Bytes();
	if(    totBytes  <=  0   )
	{
		retErrorMesg =  "BitSourceStreamingMS::Copy_To_8bit_Sample_Segment  failed,  no bytes." ;
		return  NULL;
	}




	char  *dest =  NULL,   *destRight =  NULL;  

	if(    channelCode  ==     TransformMap::SEPARATEstereo    )
	{

		if(    ! sndSample.Make_Stereo(  retErrorMesg  )     )       // *******  CAREFULL  **************************************
			return  false;


		 destRight =    sndSample.Get_StartByte_RightStereo(); 
	}

	dest =    sndSample.Get_StartByte();      //  the secondary 8-bit buffers   ( ? from  InverseDFT  )


	sndSample.Erase();






	long     bytesRead =   0L;
	bool     keepGoing =    true;
	long      loopCount =  0;
	long      startOffsettedByte  =    startOffset   *    m_bytesPerSample;



	UINT   byteCountToRead  =   m_bufferSize;


	if(    totalSegmentBytes   <   m_bufferSize   )   //  will this work for small reads ???    6/07   ***********   NEW *******************
		byteCountToRead =    totalSegmentBytes;
	else
		byteCountToRead =    m_bufferSize;     //  4096   





	bool     retHitEOF =    false;   



	while(   keepGoing   )
	{

		UINT     retActualBytesRead=0;   //   ,   retActualBytesReadRight=0;
	//	bool      detectingLeftStereo =  true;  // not important ???

		long      srcBytesProcessedParm =  2;  //  anything but zero has no impact 


		if(     ! Fetch_Streaming_Samples_Direct(     startOffsettedByte,   //    absCur  OffsetFrom  FileStart'   
																		byteCountToRead,    
														  ( BYTE*  )m_readBuffersBits,     //  destination  buffer
																	  &retActualBytesRead,  
																		srcBytesProcessedParm,																				
																		retHitEOF,    
																		true,  
																		0,   
																		false,     //   soundManControlsVolume,   NO, we want the direct data
																		retErrorMesg      )    )
			return  false; 




//		ASSERT(    byteCountToRead  ==   retActualBytesRead   );    // ???? Not a problem ???     9/03 
		ASSERT(   ( retActualBytesRead  %  m_bytesPerSample )  ==  0   );      //  must stay on  16bit stereoSample  boundaries

		if(   retHitEOF   )
		{
			int  dummy =  9;
		}




																	//  Now copy  from buffer to  8bit Sample
		char  *src        =    m_readBuffersBits;
//		char  *srcRight =  NULL;

//		if(    channelCode  ==     TransformMap::SEPARATEstereo    )
//			srcRight =   m_readBuffersBitsRight; 



		if(      channelCode  ==     TransformMap::LEFTj    )      //    leftStereo   )
		{

			for(    long  k=0;     k<  (long)( retActualBytesRead /4 );      k++   )     
			{
				src++; 				
				*dest =   *src;               //  left  
				src++;    
				dest++;

				src++;  
				//  *destRight =  *src;         //  right
				src++;    
				//  destRight++;


				bytesRead  +=    4L;

				if(     bytesRead   >=   totalSegmentBytes   )
				{
					keepGoing =   false;
					break;
				}
			}
		}

		else if(      channelCode  ==     TransformMap::RIGHTj    )       //   right channel
		{

			for(    long  k=0;     k<  (long)( retActualBytesRead /4 );      k++   )     
			{
				src++; 				
				//  *destLeft =   *src;           //  left  
				src++;    
				//  destLeft++;

				src++;  
				*dest =  *src;              //  right
				src++;    
				dest++;


				bytesRead  +=    4L;

				if(     bytesRead   >=   totalSegmentBytes   )
				{
					keepGoing =   false;
					break;
				}
			}
		}


		else if(      channelCode  ==     TransformMap::CENTEREDj    )    
		{
			short   rightVal,  leftVal;

			for(    long  k=0;     k<  (long)( retActualBytesRead /4 );      k++   )     
			{



				src++; 							//  left  
				//  *destLeft =   *src;          
				leftVal =   *src;  
				src++;    
				//  destLeft++;


				src++;							//  right
				// *dest =  *src;             
				rightVal =   *src;  
				src++;    



				*dest =  (      ( leftVal  +  rightVal )/2     );    //  create mon by averaging

				dest++;


				bytesRead  +=    4L;

				if(     bytesRead   >=   totalSegmentBytes   )
				{
					keepGoing =   false;
					break;
				}
			}
		}
		else if(      channelCode  ==     TransformMap::SEPARATEstereo    )       
		{

			short   rightVal,  leftVal;

			for(    long  k=0;     k<  (long)( retActualBytesRead /4 );      k++   )     
			{


				src++; 							//  left  
				//  *destLeft =   *src;          
				leftVal =   *src;  
				src++;    
				//  destLeft++;

				src++;							//  right
				//   *dest =  *src;             
				rightVal =   *src;  
				src++;    



				*dest       =  leftVal;    
				dest++;

				*destRight =  rightVal;    
				destRight++;


				bytesRead  +=    4L;

				if(     bytesRead   >=   totalSegmentBytes   )
				{
					keepGoing =   false;
					break;
				}
			}
		}
		else
			ASSERT( 0 );




		loopCount++;  

		startOffsettedByte   +=     byteCountToRead;     //  advance the File Offset

		if(          bytesRead   >=   totalSegmentBytes
			   ||   retHitEOF    )
			keepGoing =   false;

	}   //   while(   keepGoing



	return  true;
}




			//////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////



											////////////////////////////////////////


bool    BitSourceStreaming::Fetch_Streaming_Samples_Direct_PPlayer( long       startOffsettedByte,   //    absCur  Offset  From  FileStart'   

																										UINT      byteCountToRead,      //   ALWAYS   44160 ???   

																										BYTE    *destBufferBytesInParm,     //  destination  buffer  ( sometimes is HARDWARE buffer

																										UINT     *retActualBytesRead,  
																			 							 long       srcBytesProcessed,																				
																										 bool&        retHitEOF,    
																										 bool		   fowardFlag,  

																										 short      auditionCode,   // NOT really used. Omit??   1/10

																										 bool        soundManControlsVolume,

																										 short        rightChannelPercent,   //  how to spit the Streo signal for  sndSample 

																										 double      slowDownSpeed,

																										 CString&   retErrorMesg    )
{						
	 //   'knockOutCode':    0:  NO knockout,     1: Knockout LEFT stereoChannel,    2: Knockout RIGHT stereoChannel




//	bool   allowAudibleStereoBalancePlayer  =   true;     //  Only for Player (not Navigator) do we adjust Volume and or StreoBalance in this funct   3/11 



    int       errorCode = 0;
    UINT    byteIdx;
	int       wordCnt =  0;
	UINT    byteCount = 0;      // mine, count the bytes read
	long     blockSize =  -1;


//	short   appCode  =    Get_PitchScope_App_Code_GLB();         //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope



	retErrorMesg.Empty();

	if(  ! m_isInitialized   )
	{					
		ASSERT( 0 );
		retErrorMesg  =  "BitSourceStreaming::Fetch_Streaming_Samples_Direct_PPlayer, bitsource has not been INITIALIZED.."  ;     // the file is probably NOT yet opened, etc.   5/07
		return  false;
	}

	if(        m_wavVolumeSoundmanAddr  ==  NULL    
		&&   soundManControlsVolume   )
    {
		retErrorMesg  =  "BitSourceStreaming::Fetch_Streaming_Samples_Direct_PPlayer  failed,   m_wavVolumeSoundmanAddr  is NULL." ;
		*retActualBytesRead  =   0;
		return   false;
    }

	if(        m_wavConvert ==  NULL
		||    m_wavConvert->m_outputBufferStreaming  ==   NULL    )
    {
		retErrorMesg  =  "BitSourceStreaming::Fetch_Streaming_Samples_Direct_PPlayer  failed,   m_wavConvert or  m_outputBufferStreaming  is NULL." ;
		*retActualBytesRead  =   0;
		return   false;
    }

	if(    m_wavConvert->m_outputBufferSizeStreaming < 0    )
    {
		retErrorMesg  =  "BitSourceStreaming::Fetch_Streaming_Samples_Direct_PPlayer  failed,   m_wavConvert did not initialize output buffer." ;
		*retActualBytesRead  =   0;
		return   false;
    }

	ASSERT(   m_sPitchCalc   );


	if(           fowardFlag
		 &&   ( startOffsettedByte   %  4L   )   !=  0   )    //  are we always fetching on  STEREO-Pair  Sample Boundaries
	{
		ASSERT( 0 );											  //  ...it affects my Audio MASKING for Stereo channels 
	}




	ASSERT(   rightChannelPercent >= 0    &&    rightChannelPercent <=  100   );     //  how to  BALANCE the left and right STEREO samples

//	double   leftChannelPercentFlt    =     (  100.0 - (double)rightChannelPercent  )      / 100.0;                
//	double   rightChannelPercentFlt  =                   (double)rightChannelPercent          / 100.0;



//	long   sixteenBitLimit  =   127L;    //   32512    Is this right ???    This is the biggest value the attenuated signal can receive.     3/2010
//	sixteenBitLimit  =     sixteenBitLimit   <<  8;     //  looks right,   see  WavConvert::Multiply_Float_Samples()  "31000.0,   could go almost to  32768.0  if I want"



	BYTE    *destBufferBytes  =   destBufferBytesInParm;       //  Will play with this so can do the slow down




																			//     Prepare the DETECTION's   SndSample   for write
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



	bool   isPlayingNoteList =   false;
	if(   m_sPitchCalc->m_playModeUsingNotelist  ==  1  )       //    1: Playing from  NOTELIST
		 isPlayingNoteList =   true;



//	long  oldFilePos =    Get_WavFiles_Current_FilePosition(   m_wavConvert->m_mmIO   );   **** TEMP debug,  will fail for MP3 files

	short   directionCode; 

	if(    fowardFlag    )   
		directionCode =   BitSource::FORWARD;
	else
		directionCode =   BitSource::BACKWARDS;



	if(      ! Seek_New_DataPosition_Fast(    startOffsettedByte,   directionCode,     retErrorMesg   )     )   
		return   false;

  
//	long  intFilePos =    Get_WavFiles_Current_FilePosition(   m_wavConvert->m_mmIO   );



	UINT     byteCountAdjusted =     Get_Load_Blocks_Size();     //  44160,     the bytes in a HALF rotation of the hardware Sound Buffer

	long      sampleCount =   byteCountAdjusted /4;

	char     chVal0=0,    chVal1=0,   chVal2=0,    chVal3=0;   //  When used to be below,  but got a  error   Run-Time Check Failure #3 ( this bug was in PitchScope 1.0
	short    sndSamplesCount =  0;


	long    subBlockSize  =     m_wavConvert->m_outputBufferSampleCountNotSlowDown  *   MP3rEADERbYTESpERsAMPLE;                 //   4416  or  8832  ???





	
											     //   If going BACKWARDS,   then fill the NEW  'BackwardsPlay-BUFFER'  in WavConvert with 10 blocks ( 44,160 bytes ) 
	bool   retEndOfFileLoc =  false;
	long   retBytesReadLoc =  0;


	if(        ! fowardFlag   
		&&    m_wavConvert->m_backwardsPlayBufferSize  >  0   )
	{

		if(    m_wavConvert->m_backwardsPlayBuffer  ==  NULL   )
		{  
			retErrorMesg =   "Fetch_Streaming_Samples_Direct_PPlayer  FAILED,  m_wavConvert->m_backwardsPlayBuffer is NULL." ;
			return  false;
		}
		else
		{	if(    ! m_wavConvert->Load_Ten_MemoryBlock_Frames(   retEndOfFileLoc,    retBytesReadLoc,   byteCountAdjusted,    retErrorMesg  )     )    //   44160 bytes
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



				//    Final  INITIALIZATIONS  for the   BLOCK  DataFetch


	long   actualBytesRead =  0;   //   If only loaded   part of   10 subBlocks above,  then this will give the accurate count.  12/11
												
	byteIdx =  3;     //  an  UGLY initialization,  but that is the way the pointers are set up below



	m_wavConvert->m_biggestIndexToSndSample   =   -1;     // ******  NEW INITIALIZATIONS  for BlockLoad and creation of NOT-Slow SndSample *****************



// ***** CAREFUL with this initialization,   1/31/2012   *************************************************

	m_sPitchCalc->m_indexLastSndSampleBlockload =  -8;   //  ****** Is it a MISTAKE to do this???  ( because of partial BlockLOAD??  ) INIT,   Fetch_Current_Sample_and_Increment() will assign a VALUE when it is done.
																	//  Actually this has been very useful in FINDING BUGS cleaning up some code.  Keep it around for a while.   1/31/2012

// *********************************************************************************************




	bool   retDidBlockLoadOuter  =  false;



////////////////////////// arf /////////////////////////////

    for(   long  sampIdx= 0;      sampIdx <   sampleCount;      sampIdx++    )    //  Copy the bytes  from the IO to the buffer. 
	{															 

		bool   retEndOfFile =   false;
		bool   retDidBlockLoadLoc =  false;
		long   retBytesRead =  0; 
		unsigned char   eraseVal =   0;   //  was  127     {  0 - 255  }



		if(   ! m_wavConvert->Fetch_Current_Sample_and_Increment(   retEndOfFile,   m_currentByteIdx,    
			                                                                                          chVal0,    chVal1,  chVal2,    chVal3,  	! fowardFlag,    
																									  m_sPitchCalc->m_spitchCalcsSndSample,    
																									  sampIdx,     rightChannelPercent,   retDidBlockLoadLoc,  isPlayingNoteList,  retErrorMesg  )   )        
		{  if(   retEndOfFile   )
				retHitEOF =   true;    //  OK if this happens   2/10     Get here from move FileSlider all the way to the end.    1/31/12

			return   false;
		}


		if(     retDidBlockLoadLoc    )
			retDidBlockLoadOuter  =   true;  


		Adjust_Volume_on_CopyBytes(    chVal0,  chVal1,  chVal2,  chVal3,    destBufferBytes,  byteIdx,    rightChannelPercent   );  


		byteCount             +=  4;       //  +4 :    because now we read a   whole 16bit stereo SAMPLE   at a time.

		m_currentByteIdx  +=   4;

		byteIdx                 +=   4;   
		actualBytesRead    +=  4;

	}   //  for(   long  sampIdx= 0;    sampIdx <  sampleCount




	m_sampleIdxLastBlockLoadNotSlowExpanded  +=   ( actualBytesRead  /  m_bytesPerSample  );    //  4  ***** NEW,  careful  2/8/12    **********************

	m_currentPieClockVal =  0;     //   { 0 - 9  }    10 Events,  so  re INIT  for every load of this many  BLOCK bytes




	
	long    countOfDataSamples  =    m_sPitchCalc->Get_SndSamples_Valid_Count(  slowDownSpeed  );    //  11040[ speed 1 ]

			                                                 //  11040 [ speed 1 ]      7360[1.5]       5520[ 2]       3680[ 3 ]      2760[ 4 ]      1840[ 6 ]     1380[  8 ]

	if(   m_wavConvert->m_biggestIndexToSndSample   >   countOfDataSamples   )   // *** TEST to see if  Get_SndSamples_Valid_Count() is accurate ****
	{
		int  dummy =  9;
	}




	*retActualBytesRead  =     actualBytesRead;   

	return   true;      
}

