/////////////////////////////////////////////////////////////////////////////
//
//  PlayBuffer.cpp   -   
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



//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"      //  #include <FixMath.h>    ...Mac's version of Fixedpoint 
#include  "..\ComnGrafix\CommonGrafix.h"      //  #include "commonmy.h"   	



#include "..\comnInterface\DetectorAbstracts.h"    // **** NEW ****






////////////////////////////////////
#include  <mmsystem.h>   // Is good on Sparky  ( ProgramFiles\Microsoft Visual Studio\VC\PlatformSDK\Include )


#include  "dsoundJM.h"          //  I copied it in, bigger than the VC++ version


#include  "..\ComnAudio\CalcNote.h"

#include   "..\ComnAudio\SPitchCalc.h"


#include  "EventMan.h"


#include  "..\comnAudio\BitSourceAudio.h"
////////////////////////////////////









#include  "PlayBuffer.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


PlayBufferStatic::PlayBufferStatic(   long  memorySize,   long  speedSlowDown,    EventMan&  eventMan   )    
																				:   m_waveMan( eventMan ),   m_speedSlowDown( speedSlowDown )  
{
	m_isOK =   false;

	m_DSoundBuffer =  NULL;
	m_totalBytes =      -1;



	m_slowPlayStep  =    4096L;     // *** ALWAYS ??? *************




    if(   (  m_waveFormat =    ( tWAVEFORMATEX* )GlobalAlloc(  GMEM_FIXED,   sizeof( WAVEFORMATEX )  )    ) 
										==  NULL   )
	{
		ASSERT( 0 );
		m_isOK =   false;
	}
	else
	{  m_waveFormat->wFormatTag         =    1; 
		m_waveFormat->nChannels           =    2; 
		m_waveFormat->nSamplesPerSec  =         44100; 
		m_waveFormat->nAvgBytesPerSec =       176400;     // copied in from file load 
		m_waveFormat->nBlockAlign          =    4; 
		m_waveFormat->wBitsPerSample   =   16; 
		m_waveFormat->cbSize                 =    0;  
	}



	CString  retErrorMesg;

	if(    !Resize_Memory(   memorySize,   retErrorMesg   )     )      //  creates the  DSoundBuffer with the OS
		AfxMessageBox(  retErrorMesg  );
	else
		m_isOK =  true;
}



											////////////////////////////////////////


PlayBufferStatic::~PlayBufferStatic()
{

	Release_DSoundBuffer();


	if(    m_waveFormat  !=  NULL    )
	{
		GlobalFree(  m_waveFormat   );
		m_waveFormat =  NULL;
	}
}


											////////////////////////////////////////


void   PlayBufferStatic::Release_DSoundBuffer()
{

	if(   m_DSoundBuffer  !=  NULL   )		  //  Free the  DirectSound buffer COM interface
    {
        m_DSoundBuffer->Release();
        m_DSoundBuffer =   NULL;	 	//  FLAG the release
	}
}


											////////////////////////////////////////


bool   PlayBufferStatic::Is_Playing()
{

	if(   m_DSoundBuffer  ==   NULL   )
	{
		ASSERT( 0 );
		return  false;
	}


	HRESULT  hr;
	DWORD   currentPlayCursor; 
	DWORD   currentWriteCursor;  


	if(    FAILED(  hr =     m_DSoundBuffer->GetCurrentPosition(  &currentPlayCursor,  &currentWriteCursor  )     )    )
	{
		    //  retErrorMesg =  "Play(),  Play() failed." ;
		ASSERT( 0 );
		return   true;
	}


	if(        currentPlayCursor   >   0   
		&&   currentPlayCursor   <   m_totalBytes    )    //  likes to keep returning  the  'LAST byte index'  after play
		return  true;  
	else
		return  false;
}


											////////////////////////////////////////

/*************   INSTALL 

 HRESULT  RestoreBuffers()
{
							//  Restore lost buffers and fill them up with sound if possible

    HRESULT hr;

    if( NULL == g_pDSBuffer )
        return S_OK;

    DWORD dwStatus;
    if( FAILED( hr = g_pDSBuffer->GetStatus( &dwStatus ) ) )
        return hr;

    if( dwStatus & DSBSTATUS_BUFFERLOST )
    {
        // Since the app could have just been activated, then
        // DirectSound may not be giving us control yet, so 
        // the restoring the buffer may fail.  
        // If it does, sleep until DirectSound gives us control.
        do 
        {
            hr = g_pDSBuffer->Restore();
            if( hr == DSERR_BUFFERLOST )
                Sleep( 10 );
        }
        while( hr = g_pDSBuffer->Restore() );

        if( FAILED( hr = FillBuffer() ) )
            return hr;
    }

    return S_OK;
}
***/

bool   PlayBufferStatic::Resize_Memory(   long   byteCount,    CString&  retErrorMesg    )
{

	if(    m_waveFormat  ==  NULL   )
	{
		ASSERT( 0 );
		retErrorMesg =   "PlayBufferStatic::Resize_Memory(),  no WAVEFORMATEX structure." ;

		m_isOK =   false;
		return  false;
	}


	Release_DSoundBuffer();

	m_totalBytes =  -1;  //   FLAG the release of the OLD buffer


	/***  ...see	C:\Program Files\Microsoft Platform SDK\Samples\Multimedia\DSound\Src\playsound   for Sample Code																		

    SAFE_DELETE( g_pWaveSoundRead );		 // Free any previous globals 
    SAFE_RELEASE( g_pDSBuffer );
																				// Create a new wave file class
    g_pWaveSoundRead = new CWaveSoundRead();
																		// Load the wave file
    if( FAILED( g_pWaveSoundRead->Open( strFileName ) ) )
        SetFileUI( hDlg, TEXT("Bad wave file.") );
	***/
														// Set up the direct sound buffer, and only request the flags needed
														// since each requires some overhead and limits if the buffer can 
														// be hardware accelerated    	
	HRESULT   hr; 
	DSBUFFERDESC   bufferDescription;

    ZeroMemory(   &bufferDescription,    sizeof( DSBUFFERDESC )   );

    bufferDescription.dwSize			=   sizeof(DSBUFFERDESC);

    bufferDescription.dwFlags			=   DSBCAPS_STATIC;   // **** ADJUST??? *****


	bufferDescription.dwBufferBytes  =	 byteCount;   	      //   g_pWaveSoundRead->m_ckIn.cksize;

    bufferDescription.lpwfxFormat     =   m_waveFormat;    //   g_pWaveSoundRead->m_pwfx;



																							// Create the  'STATIC'  DirectSound buffer 

    if(    FAILED(  hr =   m_waveMan.m_infcDSound->CreateSoundBuffer(   &bufferDescription,   &m_DSoundBuffer,   NULL  )   ) )
	{
		ASSERT( 0 );
		retErrorMesg =   "PlayBufferStatic::Resize_Memory(),  CreateSoundBuffer() failed." ;

		m_isOK =   false;
		return   false;
	}

																
    m_totalBytes =    bufferDescription.dwBufferBytes;    // This is the FLAG that the buffer is OK, and teells the MAXIMUM bytes thatcan be played.

    return  true;
}




											////////////////////////////////////////


bool    PlayBufferStatic::Fill_With_Silence()
{
														    
	if(    m_DSoundBuffer  ==  NULL   )
		return  false;


	/***
    DWORD               dwSizeWritten;
	WAVEFORMATEX   wfx;

    if(     FAILED(    m_DSoundBuffer->GetFormat(   &wfx,   sizeof( WAVEFORMATEX ),   &dwSizeWritten  )     ) )
        return   false;
	***/


    PBYTE      memBlock;
    DWORD   memBlockByteCount;

    if(    SUCCEEDED(     m_DSoundBuffer->Lock(    0,  
																		   0, 
														( LPVOID* )&memBlock,
																		 &memBlockByteCount,            //  the COUNT in bytes
													                       NULL,   NULL,   
																           DSBLOCK_ENTIREBUFFER  )       ) )
    {
//      FillMemory(   memBlock,   memBlockByteCount,      ( wfx.wBitsPerSample == 8 )   ?   128 : 0     );	...we use  16 bit
		FillMemory(    memBlock,   memBlockByteCount,    0     );


        m_DSoundBuffer->Unlock(   memBlock,   memBlockByteCount,   NULL,   0   );

        return  true;
    }

    return  false;
}  



											////////////////////////////////////////


bool   PlayBufferStatic::Play_MyBuffer(   char*  srcBytes,   long  numSrcBytes,   CString&  retErrorMesg    )
{


	long  maxOversizeRatio =  2;   //  ***ADJUST***    do NOT allow the buffer to be more than this size or too much SILENCE in play.



	if(    m_DSoundBuffer  ==  NULL    )
	{
		retErrorMesg =  "Play(),  the extra Sound Buffer was not created." ;

//  *** BUG:  sometime loose buffer,  need to automatically reAlloc it  2/03   ***************************

		return   false;
	}





	if(   srcBytes  ==  NULL   )
	{
		retErrorMesg =  "Play()  was input NULL srcBytes." ;
		return   false;
	}




	bool     stillPlaying =   true;	  	//  WAIT till previous note has finished playing
	while(  stillPlaying  )
	{
		stillPlaying =   Is_Playing();    //  SKIP trying to play this pixel if the buffer is still playing 
	}




	long  totalSrcPackets =     numSrcBytes   /   m_slowPlayStep;


//  m_slowPlayStep
	long  numDstBytes    =     totalSrcPackets   *   m_slowPlayStep   *   m_speedSlowDown;    
																							  	



	if(      numDstBytes  >   (long)m_totalBytes    )								//  INCREASE  memory,  if necessary  	
	{
		if(    !Resize_Memory(  numDstBytes,   retErrorMesg  )     )
			return  false;
	}
	else if(   numDstBytes  >    (  (long)m_totalBytes  / maxOversizeRatio  )     )  // DECREASE memory if necessary
	{
		if(    !Resize_Memory(  numDstBytes,   retErrorMesg  )     )
			return  false;
	}





	if(    numDstBytes   <   (long)m_totalBytes    )	    //  playing LESS than the total  memory...
	{		
		if(    !Fill_With_Silence()    )
			ASSERT( 0 );
	}





/***
	if(   !Fill_With_Silence()   )		//	... does this stop the clicks ????   4/02
			ASSERT( 0 );
***/





	HRESULT  hr;
    VOID*     bufferMemory   =  NULL;
    VOID*     bufferMemory2 =  NULL;
    DWORD   memLength;
    DWORD   memLength2;
												 //  Lock the buffer down ( causes  bufferMemory to get initialized to Buffer's memory space )

    if(   FAILED(  hr =   m_DSoundBuffer->Lock(  0,   m_totalBytes,   &bufferMemory,     &memLength, 
																						           &bufferMemory2,   &memLength2,   0L   )    ) )
	{  retErrorMesg =  "Play(),  Lock() failed." ;
		return   false;
	}

	if(   bufferMemory  ==   NULL   )
	{
		retErrorMesg =  "Play(),  Lock could not alloc memory." ;
		return  false;   
	}
			





												  //  use  memcpy() to  create/copy  the Repetitions necessary for SlowedDown Play
	long    lastPlayByte =  0;
	char  *dst  =    (char*)bufferMemory;     
	char  *src  =  srcBytes;



	for(    long  i= 0;     i <  totalSrcPackets;     i++    )
	{

		for(    long  rep=0;     rep <   m_speedSlowDown;      rep++    )
		{

			lastPlayByte  +=     m_slowPlayStep;


												//  test that I do NOT go beyond the bounds of the memory and get EXCEPTION 
			if(    lastPlayByte  >   (long)m_totalBytes    )
			{
				ASSERT( 0 );
				break;
			}
			else
				memcpy(   dst,   src,    m_slowPlayStep   ); 


			dst  +=    m_slowPlayStep;   
		}


		src  +=    m_slowPlayStep;     //  Advance SRC for the next  'packet to duplicate'   for slowDown
	}



																					  
    m_DSoundBuffer->Unlock(   bufferMemory,   m_totalBytes,   NULL,   0   );   //  Unlock the buffer, we don't need it anymore.


/********* INSTALL this ( function def is above )  ??? 
														
    if(   FAILED( hr = RestoreBuffers()  )   )					// Restore the buffers if they are lost
	{
		retErrorMesg =  "Play(),  RestoreBuffers() failed." ;
		return   false;
	}
***/




// **** Best here ??? or move up ???  *****
	if(     FAILED(  hr =    m_DSoundBuffer->SetCurrentPosition(  0  )      )    )   
	{
		ASSERT( 0 );    
		retErrorMesg  =   "PlayBufferStatic::Play_MyBuffer failed,  SetCurrentPosition." ;
		return  false;	
	}



	

    if(    FAILED(  hr =     m_DSoundBuffer->Play(  0,  0,  0L  )     )    )
	{
		retErrorMesg =  "PlayBufferStatic::Play_MyBuffer  failed,  DSoundBuffer::Play()." ;
		return   false;
	}

	return  true;
}



											////////////////////////////////////////

/***
bool   PlayBufferStatic::Play_Chunk(   BitSource&  bitSource,   long  byteOffset,   long  numSrcBytes, 	  long  speedSlowDown, 
																												CString&  retErrorMesg    )
{

ASSERT( 0 );    // ***** OMIT for new ???? ********



	long  slowPlayStep  =   numSrcBytes;    //  ***DIFFERENT than  Play_MyBuffer(),  here us a SMALLER local value


	HRESULT  hr;

	if(   m_DSoundBuffer  ==  NULL    )
	{
		retErrorMesg =  "Play(),  the extra Sound Buffer was not created." ;
		return   false;
	}


	if(     ( byteOffset  +  slowPlayStep )    >    bitSource.m_totalBytes     )
	{
		//  ASSERT( 0 );
		retErrorMesg  =  "PlayBufferStatic::Play_Chunk,  trying to read beyond memory." ;
		return  false;
	}


	char  *srcBytes  =     bitSource.Get_Start_Byte()    +    byteOffset;


	bool     stillPlaying =   true;	  	//  WAIT till previous note has finished playing
	while(  stillPlaying  )
		stillPlaying =   Is_Playing();    //  SKIP trying to play this pixel if the buffer is still playing 


	long   totalSrcPackets =     numSrcBytes   /   slowPlayStep;

	ASSERT(   totalSrcPackets  ==  1   );

	long  numDstBytes  =     totalSrcPackets   *   slowPlayStep   *   speedSlowDown;    
																							  	


	if(     numDstBytes   !=    (long)m_totalBytes    )								//  adjust  memory,  if necessary  	
	{
		if(    !Resize_Memory(  numDstBytes,   retErrorMesg  )     )
			return  false;
	}



//	if(   !Fill_With_Silence()   )		//	... does this stop the clicks ????   4/02
//			ASSERT( 0 );


//   Clicks might come from fetching on NON  4 byte boundaries(  4 butesPerSample  ) ???  ...no click if no slow down *****



    VOID*     bufferMemory   =  NULL;
    VOID*     bufferMemory2 =  NULL;
    DWORD   memLength;
    DWORD   memLength2;
												 //  Lock the buffer down ( causes  bufferMemory to get initialized to Buffer's memory space )

    if(   FAILED(  hr =   m_DSoundBuffer->Lock(  0,   m_totalBytes,   &bufferMemory,     &memLength, 
																						           &bufferMemory2,   &memLength2,   0L   )    ) )
	{  retErrorMesg =  "Play(),  Lock() failed." ;
		return   false;
	}

	if(   bufferMemory  ==   NULL   )
	{
		retErrorMesg =  "Play(),  Lock could not alloc memory." ;
		return  false;   
	}
			

												 //  use  memcpy() to  create/copy  the Repetitions necessary for SlowedDown Play
	long    lastPlayByte =  0;
	char  *dst  =    (char*)bufferMemory;     
	char  *src  =     srcBytes;


	for(    long  i= 0;     i <  totalSrcPackets;     i++    )
	{

		for(    long  rep=0;     rep <   speedSlowDown;      rep++    )
		{

			lastPlayByte  +=     slowPlayStep;

												//  test that I do NOT go beyond the bounds of the memory and get EXCEPTION 
			if(    lastPlayByte  >   (long)m_totalBytes    )
			{
				ASSERT( 0 );
				break;
			}
			else
			{

				memcpy(   dst,   src,    slowPlayStep   ); 
			
//				char  *srcMv =   src;		//  Does this stop the clicks ???? 
//				char  *dstMv =   dst;
//				for(    long  k= 0L;     k < slowPlayStep;      k++    )
//				{
//					*dstMv =    *srcMv;
//					dstMv++;
//					srcMv++;
//				}
				

			}


			dst  +=    slowPlayStep;   
		}

		src  +=    slowPlayStep;     //  Advance SRC for the next  'packet to duplicate'   for slowDown
	}


																					  
    m_DSoundBuffer->Unlock(   bufferMemory,   m_totalBytes,   NULL,   0   );   //  Unlock the buffer, we don't need it anymore.




//     ********* INSTALL this ( function def is above )  ??? 
														
//    if(   FAILED( hr = RestoreBuffers()  )   )					// Restore the buffers if they are lost
//	{
//		retErrorMesg =  "Play(),  RestoreBuffers() failed." ;
//		return   false;
//	}

	
    if(    FAILED(  hr =     m_DSoundBuffer->Play(  0,  0,  0L  )     )    )
	{
		retErrorMesg =  "Play(),  DSoundBuffer::Play()  failed." ;
		return   false;
	}

	return  true;
}
***/

bool   PlayBufferStatic::Play_Chunk(   BitSourceStatic&  bitSource,   long  byteOffset,    long speedSlowDown,    CString&  retErrorMesg    )
{

		//  This works for SlowDown,  while  Play_Chunk()  fails  ...WHY ???




// ***HARDWIRED.... adjust *********

	long   slowPlayStep  =   2048;     //   2048  bytes in ONE ChunkOfSamples  (    512[ samples]   x   4[ bytesPerSample]   =  2048    




	
	long   numDstBytes  =    slowPlayStep   *   speedSlowDown;    																							  	



	HRESULT  hr;

	if(   m_DSoundBuffer  ==  NULL    )
	{
		retErrorMesg =  "Play_Chunk(),  the extra Sound Buffer was not created." ;
		return   false;
	}



	char  *srcBytes  =     bitSource.Get_Start_Byte()    +    byteOffset;



	bool     stillPlaying =   true;	  	//  WAIT till previous note has finished playing
	while(  stillPlaying  )
		stillPlaying =   Is_Playing();    //  SKIP trying to play this pixel if the buffer is still playing 




	if(     ( byteOffset  +  slowPlayStep )    >    bitSource.m_totalBytes     )
	{
		//  ASSERT( 0 );
		retErrorMesg  =  "PlayBufferStatic::Play_Chunk,  trying to read beyond memory." ;
		return  false;
	}



	if(     numDstBytes   !=    (long)m_totalBytes    )								//  adjust  memory,  if necessary  	
	{
		if(    !Resize_Memory(  numDstBytes,   retErrorMesg  )     )
			return  false;
	}




/***
	if(   !Fill_With_Silence()   )		//	... does this stop the clicks ????   4/02
			ASSERT( 0 );
***/

//   Clicks might come from fetching on NON  4 byte boundaries(  4 butesPerSample  ) ???  ...no click if no slow down *****





    VOID*     bufferMemory   =  NULL;
    VOID*     bufferMemory2 =  NULL;
    DWORD   memLength;
    DWORD   memLength2;
												 //  Lock the buffer down ( causes  bufferMemory to get initialized to Buffer's memory space )

    if(   FAILED(  hr =   m_DSoundBuffer->Lock(  0,   m_totalBytes,   &bufferMemory,     &memLength, 
																						           &bufferMemory2,   &memLength2,   0L   )    ) )
	{  retErrorMesg =  "Play_Chunk(),  Lock() failed." ;
		return   false;
	}

	if(   bufferMemory  ==   NULL   )
	{
		retErrorMesg =  "Play_Chunk(),  Lock could not alloc memory." ;
		return  false;   
	}
			



												 //  use  memcpy() to  create/copy  the Repetitions necessary for SlowedDown Play
	long    lastPlayByte =  0;
	char  *dst  =    (char*)bufferMemory;     
	char  *src  =     srcBytes;



		for(    long  rep=0;      rep <   speedSlowDown;      rep++    )
		{

			lastPlayByte  +=     slowPlayStep;

												//  test that I do NOT go beyond the bounds of the memory and get EXCEPTION 
			if(    lastPlayByte  >   (long)m_totalBytes    )
			{
				ASSERT( 0 );
				break;
			}
			else
			{
				memcpy(   dst,   src,    slowPlayStep   ); 

				/****
				char  *srcMv =   src;		//  Does this stop the clicks ???? 
				char  *dstMv =   dst;

				for(    long  k= 0L;     k < slowPlayStep;      k++    )
				{
					*dstMv =    *srcMv;

					dstMv++;
					srcMv++;
				}
				***/
			}


			dst  +=    slowPlayStep;   
		}



																					  
    m_DSoundBuffer->Unlock(   bufferMemory,   m_totalBytes,   NULL,   0   );   //  Unlock the buffer, we don't need it anymore.


/********* INSTALL this ( function def is above )  ??? 
														
    if(   FAILED( hr = RestoreBuffers()  )   )					// Restore the buffers if they are lost
	{
		retErrorMesg =  "Play(),  RestoreBuffers() failed." ;
		return   false;
	}
***/
	

    if(    FAILED(  hr =     m_DSoundBuffer->Play(  0,  0,  0L  )     )    )
	{
		retErrorMesg =  "Play(),  DSoundBuffer::Play()  failed." ;
		return   false;
	}

	return  true;
}



											////////////////////////////////////////


bool    PlayBufferStatic::Set_Speed(   int   slowDownRatio,    long  numSrcBytes,    CString&  retErrorMesg    )
{

				//	If do NOT want a lot of memoryresizing at  Play(),   exaggerate  numSrcBytes

	ASSERT(   slowDownRatio  >=  1   );

	ASSERT(   numSrcBytes  > 0    );      // *** TEST more,  need to be on 4-byte boundaries 



	long  numDstBytes  =    numSrcBytes   *   slowDownRatio;    


	if(     numDstBytes   <   (long )m_totalBytes    )
	{
		if(   !Resize_Memory(   numDstBytes,    retErrorMesg    )   )
			return  false;
	}


	m_speedSlowDown =    slowDownRatio;

	return  true;
}

