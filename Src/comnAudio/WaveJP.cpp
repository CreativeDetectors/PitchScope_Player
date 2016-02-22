/////////////////////////////////////////////////////////////////////////////
//
//  WaveJP.cpp   -     for .WAV files
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



#include  <mmsystem.h>   // Is good on Sparky  ( ProgramFiles\Microsoft Visual Studio\VC\PlatformSDK\Include )

#include  "dsoundJM.h"           //  MY VERSION, no longer in VStudio2005 !!!!    I copied it in,  bigger than the VC++ version



#include  "WaveJP.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////





/////////////////////////////////////   Cheezy error codes   ...replace

#ifndef ER_MEM
#define ER_MEM 				0xe000
#endif


#ifndef ER_CANNOTOPEN
#define ER_CANNOTOPEN 		0xe100
#endif


#ifndef ER_NOTWAVEFILE
#define ER_NOTWAVEFILE 		0xe101
#endif


#ifndef ER_CANNOTREAD
#define ER_CANNOTREAD 		0xe102
#endif


#ifndef ER_CORRUPTWAVEFILE
#define ER_CORRUPTWAVEFILE	0xe103
#endif
////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
////////////////////////////////////////////////////////////////////




int   WavFile_Read_File_2(   HMMIO  mmIO,   UINT  byteCountToRead,   BYTE  *destBufferBytes,  MMCKINFO  *retCheckInfo,   UINT  *actualBytesRead    );
				//   only called in this function    12/06





////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int  WavFile_Open(     char             *fileName,                        // (IN)
								HMMIO         *phmmioIn,                            // (OUT)
								WAVEFORMATEX  **ppwfxInfo,                  // (OUT)
								MMCKINFO    *pckInRIFF   )                        // (OUT)     //  MMCKINFO  contains information about a CHUNK in a RIFF file.
{
				/* This function will open a wave input file and prepare it for reading,
				 * so the data can be easily

				 * read with WaveReadFile. Returns 0 if successful, the error code if not.

				 *      fileName - Input filename to load.
				 *      phmmioIn    - Pointer to handle which will be used
				 *          for further mmio routines.
				 *      ppwfxInfo   - Ptr to ptr to WaveFormatEx structure
				 *          with all info about the file.                        
				*/
    HMMIO           mmIO;
    MMCKINFO     checkInfo;           // chunk info. for general use.
    PCMWAVEFORMAT   pcmWaveFormat;  // Temp PCM structure to load in.  
	
    WORD   cbExtraAlloc;   // Extra bytes for waveformatex 
    int         nError;            // Return value.

	

    *ppwfxInfo = NULL;      // Initialization...
    nError = 0;
    mmIO = NULL;   //   MSoft has not problems setting this toNULL 
    


    if(   (  mmIO =   mmioOpen(   fileName,    NULL,    MMIO_ALLOCBUF  |  MMIO_READ    )  ) == NULL   )
    {
        nError = ER_CANNOTOPEN;
        goto ERROR_READING_WAVE;
    }



    if(  (  nError =   ( int )mmioDescend(   mmIO,   pckInRIFF,   NULL,  0   )   )   != 0   )
    {
        goto   ERROR_READING_WAVE;
    }



    if((       pckInRIFF->ckid         !=   FOURCC_RIFF   ) 
		|| (   pckInRIFF->fccType   !=   mmioFOURCC('W', 'A', 'V', 'E')    )  )
    {
        nError =    ER_NOTWAVEFILE;

        goto   ERROR_READING_WAVE;
    }
            



																        //  Search the input file for for the 'fmt ' chunk.    
    checkInfo.ckid = mmioFOURCC(  'f',  'm',  't',  ' '   );


    if(  ( nError =  (int)mmioDescend(  mmIO,   &checkInfo,   pckInRIFF,   MMIO_FINDCHUNK  )  )  != 0  )
    {
        goto   ERROR_READING_WAVE;                
    }
     
	

    //     Expect the  'fmt'  chunk to be at least as large as     <PCMWAVEFORMAT> 

    //     if there are extra parameters at the end, we'll ignore them 
    

    if(    checkInfo.cksize   <    ( long )sizeof(  PCMWAVEFORMAT  )    )		//   data offset is 20
    {
        nError =    ER_NOTWAVEFILE;
        goto ERROR_READING_WAVE;
    }
  

													 //   Read the  'fmt'  chunk into   <pcmWaveFormat>   
	
    if(   mmioRead(   mmIO, (HPSTR) &pcmWaveFormat,  
		        (long) sizeof( pcmWaveFormat) ) !=   (long) sizeof(pcmWaveFormat)    )
    {
        nError = ER_CANNOTREAD;
        goto ERROR_READING_WAVE;
    }
                            

							 //   Ok, allocate the waveformatex, but if its not pcm  format, read the 
							 //   next word, and thats how many extra bytes to allocate.

    if(    pcmWaveFormat.wf.wFormatTag  ==  WAVE_FORMAT_PCM    )
                         cbExtraAlloc = 0;                               
    else
    {													//  Read in length of extra bytes.

        if(    mmioRead(    mmIO,   ( LPSTR )&cbExtraAlloc,     (long) sizeof( cbExtraAlloc)    )
															            !=    ( long )sizeof( cbExtraAlloc )    )
        {  nError = ER_CANNOTREAD;
            goto ERROR_READING_WAVE;
        }
     }
    
	
														// Ok, now allocate that waveformatex structure.

    if(   (  *ppwfxInfo =    ( tWAVEFORMATEX* )GlobalAlloc(  GMEM_FIXED,  
													       sizeof( WAVEFORMATEX ) + cbExtraAlloc  )   )   == NULL   )
    {
        nError =    ER_MEM;
        goto    ERROR_READING_WAVE;
    }



									  // Copy the bytes from the pcm structure to the waveformatex structure( RETURN parm )

    memcpy(   *ppwfxInfo,    &pcmWaveFormat,    sizeof( pcmWaveFormat )   );

    (  *ppwfxInfo  )->cbSize =     cbExtraAlloc;


 
										//   Now,  read those  EXTRA bytes  into the structure,  if cbExtraAlloc != 0.
    if(   cbExtraAlloc  !=   0  )
    {
        if (    mmioRead(   mmIO,   (LPSTR)(  (  (BYTE*)&((*ppwfxInfo)->cbSize) )    +sizeof(cbExtraAlloc)  ),
													(long) (cbExtraAlloc)  )   !=      (long)(  cbExtraAlloc  )    )
        {
            nError =   ER_NOTWAVEFILE;
            goto   ERROR_READING_WAVE;
        }
    }


																		  //   Ascend the input file  OUT of  the 'fmt ' chunk.                                                           
    if(   (  nError =   mmioAscend(  mmIO,  &checkInfo,  0 )   )   != 0  )
    {
        goto     ERROR_READING_WAVE;
    }
    

    goto   TEMPCLEANUP;           //   skip the error stur,  we're good.   



ERROR_READING_WAVE:
    if(   *ppwfxInfo   !=   NULL   )
    {
        GlobalFree(  *ppwfxInfo   );
        *ppwfxInfo =    NULL;
    }               

    if (  mmIO  !=  NULL  )
    {
		mmioClose(  mmIO,   0  );
        mmIO =    NULL;
    }
   

TEMPCLEANUP:
    *phmmioIn  =    mmIO;


    return   nError;
}




											////////////////////////////////////////



int  WavFile_Close_ReadingFile(   HMMIO                   *phmmio,               // IN
									            WAVEFORMATEX  **ppwfxSrc   )          // IN
{
/*     This will close the wave file opened with WavFile_Open.  

		phmmioIn -  Pointer to the handle to input MMIO.
		ppwfxSrc  -  Pointer to pointer to WaveFormatEx structure.

		Returns 0 if successful, non-zero if there was a warning.
*/

    if(   *ppwfxSrc  !=   NULL  )
    {
        GlobalFree(   *ppwfxSrc   );
        *ppwfxSrc =    NULL;
    }


    if(   *phmmio  !=   NULL   )
    {
        mmioClose(  *phmmio,  0  );
        *phmmio =   NULL;
    }

    return( 0 );
}



											////////////////////////////////////////



int    WavFile_Load_Data_And_Format(    char                *fileName,     
													UINT               *filesByteCount,                    // OUT
													WAVEFORMATEX  **waveFormatEx,              // OUT   CALLING function must realease
													BYTE                   **samplesBytes    	)            // OUT
{

		//  OPENs  and  CLOSES  the file,   is completely independant !!!    ...called by  STATICplayer &  BitSource::Load_Wav_File()


/*      This routine loads a full wave file into memory.  Be careful, wave files can get
    pretty big these days :).  
    szFileName      -       sz Filename
    filesByteCount          -       Size of loaded wave (returned)
    cSamples        -       # of samples loaded.
    waveFormatEx       -       Pointer to pointer to waveformatex structure.  The wfx structure
                    IS ALLOCATED by this routine!  Make sure to free it!
    samplesBytes         -       Pointer to a byte pointer (globalalloc) which is allocated by this 
                    routine.  Make sure to free it!

    Returns 0 if successful, else the error code.
*/

    HMMIO            mmIO;     
	
    MMCKINFO      checkInfo;     //  MMCKINFO  contains information about a CHUNK in a RIFF file.
    MMCKINFO      checkInfoRiff;   //   the Parent  of the chunk being searched for	...see   mmioDescend()   MFC notes

    int                   nError;
    UINT               actualBytesRead;


    *samplesBytes   =    NULL;
    *waveFormatEx  =    NULL;
    *filesByteCount  =    0;
    

    if(   (  nError =    WavFile_Open(   fileName,    &mmIO,    waveFormatEx,     &checkInfoRiff  )    )    !=  0   )
    {
        goto   ERROR_LOADING;   
   }


														  //  This moves us to the 'DATA' area in the file

    if(  (  nError =   WavFile_Start_Reading_Data(    &mmIO,     &checkInfo,     &checkInfoRiff     )  )   != 0  )
    {
        goto   ERROR_LOADING;
    }

		
															//  size of wave data is in checkInfo, allocate that buffer.

    if(    NULL ==  (  *samplesBytes  =       (BYTE*)GlobalAlloc(   GMEM_FIXED,   checkInfo.cksize    )       )    )
    {
        nError =    ER_MEM;        //   CHANGE to  malloc()    & sync with    BitSource::Release_Static_Block()
        goto   ERROR_LOADING;
    }




    if(  (  nError =   WavFile_Read_File_2(  mmIO,   
													  checkInfo.cksize,     //   the number of Bytes to read
													*samplesBytes,  
													&checkInfo,   
													&actualBytesRead     )       )   != 0   )
    {  goto   ERROR_LOADING;
    }        
    




    *filesByteCount  =       actualBytesRead;


    goto   DONE_LOADING;




ERROR_LOADING:

    if(   *samplesBytes  !=   NULL   )          //   ONLY if an ERROR do we free these structures
    {
        GlobalFree(  *samplesBytes  );
        *samplesBytes =     NULL;
    }

    if(   *waveFormatEx   !=   NULL     )
    {
        GlobalFree(  *waveFormatEx   );
        *waveFormatEx =     NULL;
    }


	
	
DONE_LOADING:
									
    if(  mmIO != NULL  )
    {
        mmioClose(   mmIO,  0  );					// ****   Close the file   ****
        mmIO =     NULL;
    }


    return   nError;
}



											////////////////////////////////////////


int    WavFile_Start_Reading_Data(      HMMIO         *phmmioIn,
														MMCKINFO   *pckIn,			    //  Chunk Info
														MMCKINFO   *pckInRIFF     )   //  Chunk Info PARENT
{


						//   ***** can do a SEEK to file position !!!!  ***********


/*      This routine has to be called before   WaveReadFile    as it searchs for the chunk to descend into for
    reading, that is, the  'data' chunk.  
	
	For simplicity, this used to be in the open routine, but was
    taken out and moved to a separate routine so there was more control on the chunks that are BEFORE
    the data chunk,   such as 'fact',   etc... 
*/
    
	int     nError;

    nError = 0;
    




// *************************************************************************************
// *************************************************************************************

										//  Does a nice little seek...

/***
  HMMIO   hmmio,  
  LONG     lOffset, 
  int          iOrigin   

  SEEK_CUR		Seeks to lOffset bytes from the current file position. 
  SEEK_END		Seeks to lOffset bytes from the end of the file. 
  SEEK_SET		Seeks to lOffset bytes from the beginning of the file. 

 ***/

	LONG  offset  =       pckInRIFF->dwDataOffset       +   sizeof( FOURCC );



    if((   nError =         mmioSeek(   *phmmioIn,    offset,    SEEK_SET   )      )     ==  -1   )
    {  
		//   Assert(  FALSE  );   ...think this was their error handling see   "Wassert.cpp"   

		ASSERT( false );         //  *****TEMP, want better error handle  ***** JPM 
    }

    nError =   0;

// *************************************************************************************
// *************************************************************************************




																   //  Search the input file for for the   'data'  chunk


    pckIn->ckid  =       mmioFOURCC(   'd',   'a',   't',   'a'   );    //   mmioFOURCC  macro:   converts four characters into a four-character code.



		//	 mmioDescend :    descends into a chunk of a RIFF file that was opened by using the
		//								mmioOpen function.   It can also search for a given chunk.


    if( ( nError =        mmioDescend(    *phmmioIn,     pckIn,    pckInRIFF,    MMIO_FINDCHUNK    )      )    !=  0   )
    {
        goto  ERROR_READING_WAVE;
    }


    goto  CLEANUP;    //   skip error stuf,   we're good.



ERROR_READING_WAVE:



CLEANUP:        
    return  nError;
}


											////////////////////////////////////////


bool    Seek_To_Virtual_FilePosition_WavFile(   HMMIO         *filesHandle,
														            MMCKINFO   *chunkInfo,			     //  Chunk Info
														            MMCKINFO   *parentChunkInfo,        //  Chunk Info PARENT
																    long           absoluteFilePosition,
																    CString&    retErrorMesg   )
{

	//  'absoluteFilePosition'   is absolute, but after we have passed the headers stuff

	//  This thing might be a little too slow,  but it can get you to the right place,  maybe best used just for debugging.   2/10

    
	int     nError =  0;

    if(       filesHandle ==  NULL
		||   chunkInfo  ==   NULL
		||   parentChunkInfo  ==   NULL   )
    {  
 		retErrorMesg  =     "Seek_To_Virtual_FilePosition_WavFile  failed,  some of input parms are NULL." ;
		return  false;        
    }



	if(  absoluteFilePosition  <  0   )
	{
		ASSERT( 0 );                         //  Very bad if this happens.    10/11    Land here when mis calulate the   'absoluteFilePosition'
		absoluteFilePosition  =  0;  
	}




/***
  SEEK_CUR		Seeks to lOffset bytes from the current file position. 
  SEEK_END		Seeks to lOffset bytes from the end of the file. 
  SEEK_SET		Seeks to lOffset bytes from the beginning of the file. 
 ***/

	LONG  offset  =       parentChunkInfo->dwDataOffset       +   sizeof( FOURCC );


    if((   nError =         mmioSeek(   *filesHandle,    offset,    SEEK_SET   )      )     ==  -1   )
    {  
 		retErrorMesg  =     "Seek_To_Virtual_FilePosition_WavFile  failed,   1st mmioSeek failed." ;
		return  false;        
    }

 

																   //  Search the input file for for the   'data'  chunk   ***** Is this too slow ???  


    chunkInfo->ckid  =      mmioFOURCC(   'd',   'a',   't',   'a'   );    //   mmioFOURCC  macro:   converts four characters into a four-character code.



		//	 mmioDescend :    descends into a chunk of a RIFF file that was opened by using the
		//								mmioOpen function.   It can also search for a given chunk.


    if( ( nError =      mmioDescend(    *filesHandle,     chunkInfo,    parentChunkInfo,    MMIO_FINDCHUNK    )      )    !=  0   )
    {
 		retErrorMesg  =     "Seek_To_Virtual_FilePosition_WavFile  failed,   mmioDescend failed." ;
		return  false;        
    }


		//  NOW we are at the start of the DATA,  from where  absoluteFilePosition will offset from



	if((   nError =    mmioSeek(    *filesHandle,    absoluteFilePosition,    SEEK_CUR  )      )     ==  -1   )    //  SEEK_CUR:		Seeks to lOffset bytes from the current file position. 
	{  

				//  ******  Be CAREFUL   not to enclose    mmioSeek()    with   {  mmioGetInfo(),    mmioSetInfo()   }  ***** TROUBLE ??????     2/10 
		retErrorMesg  =     "Seek_To_Virtual_FilePosition_WavFile  failed,   2nd mmioSeek failed." ;
		return  false;        
	}


    return  true;
}



													////////////////////////////////////


long    Get_WavFiles_Current_FilePosition(   HMMIO  mmIO   )
{

		//  Might not give exact info becase the buffer only fill itself occasionally.  Think this just points to the START of the last buffer's grab.  2/10


	long   retFilePos =  -1;    // **** NEW,   2/10     Is this OK ????

	long   bufferOffset =  -1;


    MMIOINFO    mmIOinfo;         //   current status of  <mmIO>

    int               nError = 0;


    if(    nError =   mmioGetInfo(    mmIO,    &mmIOinfo,    0    )    != 0    )    //  Assign/Init  the  MMIOINFO
    {
		ASSERT( 0 );
        return  -1;
    }



	char*    bufferStart     =      mmIOinfo.pchBuffer;    // start of I/O buffer (or NULL)


	char*    nextReadByte  =     mmIOinfo.pchNext;        // pointer to next byte to read/write  



	bufferOffset  =     mmIOinfo.lBufOffset;      //   "disk offset of start of buffer"                ( poorly defined as   "reserved"  does this give more info ???



	retFilePos     =     mmIOinfo.lDiskOffset;    //  "disk offset of next read or write"

	return  retFilePos;
}



													////////////////////////////////////


int   WavFile_Read_File_2(     HMMIO         mmIO,                 
									UINT            byteCountToRead,                          
									BYTE          *destBufferBytes,         
									MMCKINFO  *retCheckInfo,              //  MMCKINFO  contains information about a CHUNK in a RIFF file.          
									UINT           *actualBytesRead    )     // OUT.
{

 //     This will read wave data from the wave file.  Make sure we're descended into the data chunk, else 
//             this will fail bigtime!
//
//					mmIO         - Handle to mmio.
//					byteCountToRead          - # of bytes to read.   
//					destBufferBytes          - Destination buffer to put bytes.
//					actualBytesRead- # of bytes actually read.


    MMIOINFO    mmIOinfo;         //   current status of  <mmIO>

    int         nError = 0;
    UINT      byteIdx,   byteCountAdjusted;
	int         wordCnt =  0;
	UINT      byteCount = 0;       // mine,   count the bytes read

//	int   leftMax   =  0;  
//	int   rightMax =  0;  




    if(    nError =   mmioGetInfo(    mmIO,    &mmIOinfo,    0    )    != 0    )    //  Assign/Init  the  MMIOINFO
    {
        goto   ERROR_CANNOT_READ;
    }
       	


	byteCountAdjusted =    byteCountToRead;

    if(     byteCountAdjusted   >    retCheckInfo->cksize    ) 
		byteCountAdjusted =    retCheckInfo->cksize;       

  
    retCheckInfo->cksize  -=      byteCountAdjusted;    //  'retCheckInfo->cksize'   is now zero
    




									  //  Copy the bytes from the io to the buffer. 


	for(   byteIdx =0;      byteIdx <   byteCountAdjusted;      byteIdx++   )      //  Reads in CHUNKS
	{															 

																

		if(    mmIOinfo.pchNext    ==    mmIOinfo.pchEndRead    )     
		{
																	//  Fetch new CHUNK....   ( 8000+  bytes ) 

			if(   (  nError =    mmioAdvance(   mmIO,    &mmIOinfo,    MMIO_READ    )     )  != 0   )
			{
				goto   ERROR_CANNOT_READ;   
			} 
			else
			{  
				
				/****************
				unsigned long    end,  start,   numBytes;


				start =    (  unsigned long   )(  mmIOinfo.pchNext  );
				end  =    (  unsigned long   )(  mmIOinfo.pchEndRead  );   

				numBytes =   end -  start;
				*************/



							//  [ NECESSARY ???  4/00 ]      test to make sure that the  BYTES to be READ is  MOD 4, or I may have alignment problems
				
	//			if(    (  numBytes  %  4 )   !=  0    )
	//			{
	//				TRACE( "  WaveReadFile( )   BAD buffer size for 16bit stereo!!! \n"  );
	//				ASSERT( false );
	//			}
				
			}



			if(     mmIOinfo.pchNext    ==    mmIOinfo.pchEndRead     )
			{
				nError =    ER_CORRUPTWAVEFILE;

				goto   ERROR_CANNOT_READ;
			}
		}
 



		
		unsigned char   byVal;
		char                 ch1,  ch3;
		unsigned long   byVal0,    byVal1,   byVal2,    byVal3;      //  OMIT:  ***** NOT used !!! *****


			//  BAD:    *((BYTE*)destBufferBytes+byteIdx)  =       *((BYTE*)mmIOinfo.pchNext)++;  NEEDED to break into 2 lines.  **** jpm 

  		   //   WORKS:   *(   (BYTE*)destBufferBytes  + byteIdx   )  =   *(    ( BYTE* )mmIOinfo.pchNext    );      //  WORKS



		byVal  =      *(    ( BYTE* )mmIOinfo.pchNext    );     //  Get the value of the CURRENT BYTE

		byteCount++;



		if(          (byteIdx % 4) == 0   )         //   'byteIdx'   is relative to the ABSOLUTE start of  'ALL dataBytes'
		{
				byVal0 =   byVal;
				 *(   (BYTE*)destBufferBytes  + byteIdx   )  =    byVal;   //  Write the value to the  OUTSIDE MEMORY (  destBufferBytes  )
		}
		else if(   (byteIdx % 4) == 1   )   
		{
				byVal1 =   byVal;
				*(   (BYTE*)destBufferBytes  + byteIdx   )  =     byVal;  

				ch1 =   *(    ( char* )mmIOinfo.pchNext    );     //  Get LEFT channel's amplitude ( divided by 256 )
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

				ch3 =   *(    ( char* )mmIOinfo.pchNext    );    //  Get RIGHT channel's amplitude ( divided by 256 )


		//		if(     abs(  (int)ch1  )   >   leftMax	   )         leftMax    =    abs(  (int)ch1  );
		//		if(     abs(  (int)ch3  )   >   rightMax	   )    rightMax  =   abs(  (int)ch3  );	
		}
		else   ASSERT(  false );



		( BYTE* )mmIOinfo.pchNext++;    	
	}




			//   mmioSetInfo() :    updates the information retrieved by the mmioGetInfo function about a file opened 
			//                              by using the mmioOpen function. Use this function to terminate direct buffer access 
			//							    of a file opened for buffered I/O.

    if(    (  nError =     mmioSetInfo(    mmIO,    &mmIOinfo,    0    )       )      !=  0    )
		goto   ERROR_CANNOT_READ;


    *actualBytesRead  =     byteCountAdjusted;



    goto   FINISHED_READING;






ERROR_CANNOT_READ:

    *actualBytesRead  =    0;



FINISHED_READING:

    return  nError;
}





////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////  Below are  NOT  USED   12/06    //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////



/***********************  Start  of Unused sample code  ***************

int   WaveReadFile(        HMMIO         mmIO,             // **** Persistant   
				   
									UINT            byteCountToRead,                      
									BYTE          *destBufferBytes,         //   the exterior MEMORY Block  that we write to 
									
									MMCKINFO  *retCheckInfo,     // **** Persistant   ,   MMCKINFO  contains information about a CHUNK in a RIFF file.
									
									UINT           *actualBytesRead,          // OUT.

									unsigned long   iEvent,
									UINT                bytsInSect    )
{

			//  CALLED  from     StreamingAudioplayer::Load_Next_DataFetch_Forward()



 //     This will read wave data from the wave file.  Make sure we're descended into
 //        the data chunk, else this will fail bigtime!
//
//					mmIO         - Handle to mmio.
//					byteCountToRead          - # of bytes to read.   
//					destBufferBytes          - Destination buffer to put bytes.
//					actualBytesRead- # of bytes actually read.


    MMIOINFO     mmioinfoIn;     // current status of <mmIO>

    int       errorCode = 0;
    UINT    byteIdx,   byteCountAdjusted;
	int       wordCnt =  0;
	UINT    byteCount = 0;      // mine, count the bytes read




    if(    errorCode =      mmioGetInfo(   mmIO,   &mmioinfoIn,   0    )      != 0    )
    {
        goto   ERROR_CANNOT_READ;
    }
       	


    byteCountAdjusted =     byteCountToRead;

    if(     byteCountAdjusted   >   retCheckInfo->cksize    ) 
		byteCountAdjusted =   retCheckInfo->cksize;       

    retCheckInfo->cksize  -=      byteCountAdjusted;
    


																				// Copy the bytes from the io to the buffer. 

	for(     byteIdx =0;       byteIdx <    byteCountAdjusted;        byteIdx++     )
	{															 


													//   Are we ready to ADVANCE to another BlockRead...

		if(     mmioinfoIn.pchNext   ==    mmioinfoIn.pchEndRead    )
		{

			if(   (  errorCode =    mmioAdvance(   mmIO,    &mmioinfoIn,    MMIO_READ   )     )  != 0   )
			{
				goto    ERROR_CANNOT_READ;
			} 
			else
			{  unsigned long    end,  start,   numBytes;

				start =    (  unsigned long   )( mmioinfoIn.pchNext );
				end  =    (  unsigned long   )( mmioinfoIn.pchEndRead );   
				numBytes =   end -  start;

						//  test to make sure that the  BYTES to be READ is  MOD 4, or I may have alignment problems
				if(    (  numBytes  %  4 )   !=  0    )
				{
					TRACE( "  WaveReadFile( )   BAD buffer size for 16bit stereo!!! \n"  );
					ASSERT( 0 );
				}
			}


			if(    mmioinfoIn.pchNext   ==    mmioinfoIn.pchEndRead   )
			{
				errorCode =   ER_CORRUPTWAVEFILE;
				goto   ERROR_CANNOT_READ;
			}
		}
 



		unsigned char   byVal;
		char                ch1,  ch3;
		unsigned long   byVal0,    byVal1,   byVal2,    byVal3;


			//  BAD:    *((BYTE*)destBufferBytes+byteIdx)  =       *((BYTE*)mmioinfoIn.pchNext)++;  NEEDED to break into 2 lines.  **** jpm 
  		   //   WORKS:   *(   (BYTE*)destBufferBytes  + byteIdx   )  =   *(    ( BYTE* )mmioinfoIn.pchNext    );      //  WORKS


		byVal   =      *(    ( BYTE* )mmioinfoIn.pchNext    );  
		byteCount++;



		if(          (byteIdx % 4) == 0   )   
		{
			byVal0 =   byVal;
			 *(   (BYTE*)destBufferBytes  + byteIdx   )  =    byVal;   
		}
		else if(   (byteIdx % 4) == 1   )   
		{
			byVal1 =   byVal;
			*(   (BYTE*)destBufferBytes  + byteIdx   )  =     byVal;  

			ch1 =   *(    ( char* )mmioinfoIn.pchNext    );     //  Get LEFT channel's amplitude ( divided by 256 )
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

			ch3 =   *(    ( char* )mmioinfoIn.pchNext    );    //  Get RIGHT channel's amplitude ( divided by 256 )

			
		//		if(     abs(  (int)ch1  )   >   leftMax	   )         
		//			leftMax    =    abs(  (int)ch1  );

		//		if(     abs(  (int)ch3  )   >   rightMax	   )    
		//			rightMax  =   abs(  (int)ch3  );	
				
		}
		else   
			ASSERT(  false );


		( BYTE* )mmioinfoIn.pchNext++;    	
	}



    if(    (  errorCode =     mmioSetInfo(   mmIO,    &mmioinfoIn,   0   )     )      != 0    )
	{
		goto   ERROR_CANNOT_READ;
	}


    *actualBytesRead  =     byteCountAdjusted;

    goto   FINISHED_READING;




ERROR_CANNOT_READ:
    *actualBytesRead  =   0;



FINISHED_READING:

    return   errorCode;
}






int   WaveReadFile_Offsetted_GLB(      HMMIO         mmIO,             // **** Persistant   				   
												UINT            byteCountToRead,                      
												BYTE          *destBufferBytes,         //   the exterior MEMORY Block  that we write to 												
												MMCKINFO  *retCheckInfo,     // **** Persistant   ,   MMCKINFO  contains information about a CHUNK in a RIFF file.												
												UINT           *actualBytesRead,          // OUT.
												unsigned long   iEvent,
												UINT                bytsInSect,
												
												long    srcBytesProcessed,
												
												long    absCurOffsetFromFileStart    )
{

			//  NEW,   for Continued-Play,    CALLED  from     StreamingAudioplayer::Load_Next_DataFetch_Forward()


//      This will read wave data from the wave file.  Make sure we're descended into
//         the data chunk, else this will fail bigtime!
//
//					mmIO         - Handle to mmio.
//					byteCountToRead          - # of bytes to read.   
//					destBufferBytes          - Destination buffer to put bytes.
//					actualBytesRead- # of bytes actually read.


    MMIOINFO     mmioinfoIn;     // current status of <mmIO>

    int       errorCode = 0;
    UINT    byteIdx,   byteCountAdjusted;
	int       wordCnt =  0;
	UINT    byteCount = 0;      // mine, count the bytes read

	long    blockSize =  -1;


	UINT                 fileOffset,    curFileOffset;
	unsigned long    end,  start,   numBytes;




    if(    errorCode =      mmioGetInfo(   mmIO,   &mmioinfoIn,   0    )      != 0    )
    {
        goto   ERROR_CANNOT_READ;
    }
       	


    byteCountAdjusted  =     byteCountToRead;

    if(     byteCountAdjusted   >   retCheckInfo->cksize    ) 
		byteCountAdjusted =   retCheckInfo->cksize;       

    retCheckInfo->cksize  -=      byteCountAdjusted;
    

	
	blockSize =   mmioinfoIn.cchBuffer;

	ASSERT(   blockSize  >  0   );






	static   UINT  origFileOffset; 

	if(    srcBytesProcessed   ==   0    )   
		origFileOffset  =     mmioinfoIn.lDiskOffset;     //  ...DEBUG


	curFileOffset  =     mmioinfoIn.lDiskOffset;  //   ...and get CURRENT offset  for each CALL



						//   Calc current position,  and if not in right place,  advance  to correctblock and ASSIGN  'mmioinfoIn.pchNext'

// **** FIX,  also need BACKWARDS play  & ability to  SEEK for cases other than   (  srcBytesProcessed  ==  0  )    




//	if(    srcBytesProcessed   ==   0    )     //   1st call for play,    so do the one ADVANCE now,  all other fetches will be USUALLY( ? ) sequential   ???  
	if(     curFileOffset    !=      (  absCurOffsetFromFileStart   +   origFileOffset    )      )
	{

		if(    srcBytesProcessed   !=   0    )   
		{
			int   dummyTake =  9;
		}


		long   destBlockIdx             =      absCurOffsetFromFileStart    /     blockSize;
		long   destBlockRemainder  =      absCurOffsetFromFileStart    %    blockSize;




		for(    long  block =0;        block <   destBlockIdx;       block++    )
		{

			mmioinfoIn.pchNext  =      mmioinfoIn.pchEndRead;      //   hack:   MUST do this bogus assignment to TRIGGER  mmioAdvance()

			
			if(   (  errorCode =    mmioAdvance(   mmIO,    &mmioinfoIn,    MMIO_READ   )     )     != 0   )
			{
				goto    ERROR_CANNOT_READ;
			} 
			else
			{  start =    (  unsigned long   )( mmioinfoIn.pchNext );
				end  =    (  unsigned long   )( mmioinfoIn.pchEndRead );   
				numBytes =   end -  start;

				//    fileOffset  =     mmioinfoIn.lDiskOffset;			 ...DEBUG


						//  test to make sure that the  BYTES to be READ is  MOD 4, or I may have alignment problems
				if(    (  numBytes  %  4 )   !=  0    )
				{  TRACE( "  WaveReadFile( )   BAD buffer size for 16bit stereo!!! \n"  );
					ASSERT( 0 );
				}
			}


			if(    mmioinfoIn.pchNext   ==    mmioinfoIn.pchEndRead   )
			{
				errorCode =   ER_CORRUPTWAVEFILE;
				goto   ERROR_CANNOT_READ;
			}
		}

																		//   and finally advance the pointer with the remainder
	    mmioinfoIn.pchNext   +=      destBlockRemainder;     





		UINT   nuFileOffset  =     mmioinfoIn.lDiskOffset;	


		UINT   offsetDiff   =       nuFileOffset   -    origFileOffset;     //  should equal    (  destBlockIdx  *  blockSize )

		offsetDiff            +=    destBlockRemainder;				 //  and now should now  equal   absCurOffsetFromFileStart



		if(    (long)offsetDiff     !=    absCurOffsetFromFileStart     )
		{
			int  dummyBrek  =   9;
		}




//	**************    FAILS,   why I dont know 

//		long  offset  =       destBlockIdx    *   blockSize;
//		fileOffset  =     mmioinfoIn.lDiskOffset; 

//		if((   errorCode =        mmioSeek(   mmIO,    offset,      SEEK_CUR   )      )     ==  -1   )    //    SEEK_SET
//		{  
//			ASSERT( false );         
//			goto   ERROR_CANNOT_READ;
//		}
//		fileOffset  =     mmioinfoIn.lDiskOffset; 

//		for(  long j =0;     j <  destBlockRemainder;     j++    )
//			( BYTE* )mmioinfoIn.pchNext++;     //  	 ***** WHY cast ???? 
		


	}

// ***************************************************************************************************



																				//  Copy the bytes from the IO to the buffer. 

	for(     byteIdx =0;       byteIdx <    byteCountAdjusted;        byteIdx++     )
	{															 

																	//   Are we ready to ADVANCE to another BlockRead...

		if(     mmioinfoIn.pchNext   ==    mmioinfoIn.pchEndRead    )
		{

					//   fileOffset  =   mmioinfoIn.lDiskOffset;    ....DEBUG

			if(   (  errorCode =    mmioAdvance(   mmIO,    &mmioinfoIn,    MMIO_READ   )     )  != 0   )
			{
				goto    ERROR_CANNOT_READ;
			} 
			else
			{  unsigned long    end,  start,   numBytes;

				start =    (  unsigned long   )(  mmioinfoIn.pchNext   );
				end  =    (  unsigned long   )(  mmioinfoIn.pchEndRead   );   
				numBytes  =    end  -  start;

				fileOffset    =    mmioinfoIn.lDiskOffset;    //     ....DEBUG



						//  test to make sure that the  BYTES to be READ is  MOD 4, or I may have alignment problems
				if(    (  numBytes  %  4 )   !=  0    )
				{
					TRACE( "  WaveReadFile( )   BAD buffer size for 16bit stereo!!! \n"  );
					ASSERT( 0 );
				}
			}


			if(    mmioinfoIn.pchNext   ==    mmioinfoIn.pchEndRead   )
			{
				errorCode =   ER_CORRUPTWAVEFILE;
				goto   ERROR_CANNOT_READ;
			}
		}
 



		unsigned char   byVal;
		char                ch1,  ch3;
		unsigned long   byVal0,    byVal1,   byVal2,    byVal3;


			//  BAD:    *((BYTE*)destBufferBytes+byteIdx)  =       *((BYTE*)mmioinfoIn.pchNext)++;  NEEDED to break into 2 lines.  **** jpm 
  		   //   WORKS:   *(   (BYTE*)destBufferBytes  + byteIdx   )  =   *(    ( BYTE* )mmioinfoIn.pchNext    );      //  WORKS


		byVal   =      *(    ( BYTE* )mmioinfoIn.pchNext    );  
		byteCount++;



		if(          (byteIdx % 4) == 0   )   
		{
			byVal0 =   byVal;
			 *(   (BYTE*)destBufferBytes  + byteIdx   )  =    byVal;   
		}
		else if(   (byteIdx % 4) == 1   )   
		{
			byVal1 =   byVal;
			*(   (BYTE*)destBufferBytes  + byteIdx   )  =     byVal;  

			ch1 =   *(    ( char* )mmioinfoIn.pchNext    );     //  Get LEFT channel's amplitude ( divided by 256 )
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

			ch3 =   *(    ( char* )mmioinfoIn.pchNext    );    //  Get RIGHT channel's amplitude ( divided by 256 )
		}
		else   
			ASSERT(  false );


		( BYTE* )mmioinfoIn.pchNext++;    	
	}



				//  mmioSetInfo()  :    updates the information retrieved by the mmioGetInfo function about a file opened 
				//								by using the mmioOpen function. Use this function to terminate direct buffer access 
				//								of a file opened for buffered I/O.

    if(    (  errorCode =     mmioSetInfo(   mmIO,    &mmioinfoIn,   0   )     )      != 0    )
	{
		goto   ERROR_CANNOT_READ;
	}




    *actualBytesRead  =     byteCountAdjusted;

    goto   FINISHED_READING;




ERROR_CANNOT_READ:
    *actualBytesRead  =   0;



FINISHED_READING:

    return   errorCode;
}




										//////////////////////////////////

  
int   Wave_ReadBytes(   HMMIO   mmIO,           // IN
									UINT      byteCountToRead,             // IN          
																	
									BYTE  *leftBytes,       	//   BYTE     *pbDest,       // IN
									BYTE  *rightBytes,  

									MMCKINFO   *pckIn,           // IN.
									UINT            *actualBytesRead         // OUT.
								)        
{
//      This will read wave data from the wave file.  Make sure we're descended into
//         the data chunk, else this will fail bigtime!
//
//					mmIO         - Handle to mmio.
//					byteCountToRead          - # of bytes to read.   
//					pbDest          - Destination buffer to put bytes.
//					actualBytesRead- # of bytes actually read.


    MMIOINFO   mmioinfoIn;         // current status of <mmIO>
    int              nError = 0;
    UINT             byteIdx,   cbDataIn;
	int   leftMax   =  0;  
	int   rightMax =  0;  
	int   wordCnt =  0;
	UINT    byteCount = 0;  // mine, count the bytes read




    if(    nError =   mmioGetInfo(   mmIO,   &mmioinfoIn,   0  )    != 0    )
    {
        goto   ERROR_CANNOT_READ;
    }
       	

    cbDataIn =   byteCountToRead;

    if(   cbDataIn   >   pckIn->cksize  ) 
							                 cbDataIn =   pckIn->cksize;       

    pckIn->cksize  -=    cbDataIn;
    


			  // Copy the bytes from the io to the buffer. 


	for(   byteIdx= 0;     byteIdx <  cbDataIn;     byteIdx++   )     // Reads in CHUNKS ( 8000+  bytes ),  
	{															 

		if(    mmioinfoIn.pchNext   ==   mmioinfoIn.pchEndRead    )     // Fetch new CHUNK
		{
			if(   (  nError =    mmioAdvance(  mmIO,   &mmioinfoIn,   MMIO_READ  )     )  != 0   )
			{
				goto    ERROR_CANNOT_READ;   
			} 
			else
			{  unsigned long    end,  start,   numBytes;

				start =    (  unsigned long   )( mmioinfoIn.pchNext );
				end  =    (  unsigned long   )( mmioinfoIn.pchEndRead );   
				numBytes =   end -  start;

							//  [ NECESSARY ???  4/00 ]      test to make sure that the  BYTES to be READ is  MOD 4, or I may have alignment problems
				if(    (  numBytes  %  4 )   !=  0    )
				{
					TRACE( "  WaveReadFile( )   BAD buffer size for 16bit stereo!!! \n"  );
					ASSERT( false );
				}
			}


			if(    mmioinfoIn.pchNext   ==   mmioinfoIn.pchEndRead   )
			{
				nError =   ER_CORRUPTWAVEFILE;
				goto   ERROR_CANNOT_READ;
			}
		}
 


		unsigned char   byVal;
		char                 ch1,  ch3;   //  WHY use signed here ????

			//  BAD:    *((BYTE*)pbDest+byteIdx)  =       *((BYTE*)mmioinfoIn.pchNext)++;  NEEDED to break into 2 lines.  **** jpm 
  		    //   WORKS:   *(   (BYTE*)pbDest  + byteIdx   )  =   *(    ( BYTE* )mmioinfoIn.pchNext    );      //  WORKS


		byVal =   *(    ( BYTE* )mmioinfoIn.pchNext    );     //  Get the value of the CURRENT BYTE
		byteCount++;



		if(          (byteIdx % 4) == 0   )         //   'byteIdx'   is relative to the ABSOLUTE start of  'ALL dataBytes'
		{
			//	 *(   (BYTE*)pbDest  + byteIdx   )  =    byVal;   //  Write the value to the  OUTSIDE MEMORY (  pbDest  )
		}
		else if(   (byteIdx % 4) == 1   )   
		{
	  //	*(   (BYTE*)pbDest  + byteIdx   )  =     byVal;  
			*leftBytes =  byVal;        leftBytes++;     


			ch1 =   *(    ( char* )mmioinfoIn.pchNext    );     //  Get LEFT channel's amplitude ( divided by 256 )
		}
		else if(   (byteIdx % 4) == 2   )   
		{
			//	*(   (BYTE*)pbDest  + byteIdx   )  =    byVal;  
		}
		else if(   (byteIdx % 4) == 3   )
		{
	  //	*(   (BYTE*)pbDest  + byteIdx   )  =    byVal;  
			*rightBytes =  byVal;        rightBytes++;     


			ch3 =   *(    ( char* )mmioinfoIn.pchNext    );    //  Get RIGHT channel's amplitude ( divided by 256 )

				//   if(     abs(  (int)ch1  )   >   leftMax	   )         leftMax    =    abs(  (int)ch1  );
				//   if(     abs(  (int)ch3  )   >   rightMax	   )    rightMax  =   abs(  (int)ch3  );	
		}
		else   ASSERT(  false );



		( BYTE* )mmioinfoIn.pchNext++;    	
	}




    if(    (  nError =     mmioSetInfo(   mmIO,   &mmioinfoIn,   0  )  )   != 0   )
							goto   ERROR_CANNOT_READ;

    *actualBytesRead  =   cbDataIn;


    goto   FINISHED_READING;




ERROR_CANNOT_READ:
    *actualBytesRead = 0;


FINISHED_READING:

    return(  nError  );
}




					/////////////////////////////////////////////


int    WaveLoadFile_JP(   char          *fileName,      // (IN)
									UINT         *filesByteCount,                              // (OUT)
									DWORD    *pcSamples,                       // (OUT)
									WAVEFORMATEX  **ppwfxInfo,           // (OUT)
									BYTE        **ppbData  	)                     // (OUT)
{

//      This routine loads a full wave file into memory.  Be careful, wave files can get
//    pretty big these days :).  
//    szFileName      -       sz Filename
//    filesByteCount          -       Size of loaded wave (returned)
//    cSamples        -       # of samples loaded.
 //   ppwfxInfo       -       Pointer to pointer to waveformatex structure.  The wfx structure
 //                   IS ALLOCATED by this routine!  Make sure to free it!
 //   ppbData         -       Pointer to a byte pointer (globalalloc) which is allocated by this 
 //                   routine.  Make sure to free it!
//
//    Returns 0 if successful, else the error code.



    HMMIO            mmIO;        
    MMCKINFO      checkInfoRiff;
    MMCKINFO      checkInfo;
    int                  nError;
    UINT               actualBytesRead;


    *ppbData = NULL;
    *ppwfxInfo = NULL;
    *filesByteCount = 0;
    

    if(   (  nError =    WavFile_Open(   fileName,   &mmIO,    ppwfxInfo,   &checkInfoRiff  )    ) != 0   )
    {
        goto   ERROR_LOADING;
   }

											//  This moves us to the 'DATA' area in the file
    if(  (  nError =   WavFile_Start_Reading_Data(   &mmIO,    &checkInfo,    &checkInfoRiff   )  )   != 0  )
    {
        goto   ERROR_LOADING;
    }

		
						// Ok, size of wave data is in checkInfo, allocate that buffer.

    if(    NULL ==  (  *ppbData  =       (BYTE*)GlobalAlloc(  GMEM_FIXED,  checkInfo.cksize  )    )    )
    {
        nError = ER_MEM;        //   CHANGE to  malloc()    & sync with    BitSource::Release_Static_Block()
        goto ERROR_LOADING;
    }




    if(  (  nError =   WavFile_Read_File_2(  mmIO,   
													  checkInfo.cksize,   
													*ppbData,  
													&checkInfo,   
													&actualBytesRead      )       )   != 0     )
    {  goto   ERROR_LOADING;
    }        
    




    *filesByteCount   =   actualBytesRead;


    goto   DONE_LOADING;



ERROR_LOADING:

    if (*ppbData != NULL)
    {
        GlobalFree(  *ppbData  );
        *ppbData = NULL;
    }

    if (*ppwfxInfo != NULL)
    {
        GlobalFree(  *ppwfxInfo  );
        *ppwfxInfo = NULL;
    }

	
	
DONE_LOADING:
										 // Close the wave file. 
    if (mmIO != NULL)
    {
        mmioClose(  mmIO,  0  );					// ****   Close the file   ****

        mmIO = NULL;
    }

    return(  nError  );
}



***********************  End of Unused sample code  ******************/




