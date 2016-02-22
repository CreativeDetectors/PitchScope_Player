/////////////////////////////////////////////////////////////////////////////
//
//  sndSample.cpp   -    Sound SAMPLE  
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


#include <math.h>




//////////////////////////////////
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"
		
#include  "..\ComnGrafix\CommonGrafix.h"    
#include  "..\comnFoundation\myMath.h"



#include  "..\comnGrafix\OffMap.h" 
#include  "..\comnGrafix\TransformMap.h"


#include  "..\comnAudio\FundamentalCandidate.h"





#include  "sndSample.h"

#include  "sndFilters.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////



void        Begin_ProgressBar_Position_GLB(   char  *text   );   //   in  SPitchListWindow.cpp  
void        Set_ProgressBar_Position_GLB(   long  posInPercent   );
void        End_ProgressBar_Position_GLB();




JimboWAVfileHeader   wavHeader  =
{    	
	0x46464952,					//	DWORD    dWord0;	  	
	0x00000000,					//	DWORD    dWord1;	  	//	  totalSampleBytes  +  36	    [   3047460   ]	    filesize - 8	   
	0x45564157,					//	DWORD    dWord2;	  	
	0x20746D66,					//	DWORD    dWord3;		
	0x00000010,					//	DWORD    dWord4;		
	0x00020001,					//	DWORD    dWord5;	  	
	0x0000AC44,					//	DWORD    dWord6;	 	
	0x0002B110,					//	DWORD    dWord7;	  	
	0x00100004,					//	DWORD    dWord8;	  	
	0x61746164,					//	DWORD    dWord9;	  	
	0x00000000					//	DWORD    dWord10;	  	//   totalSampleBytes			 [   3047424  =    761856  *  4  ]            
};





////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


extern    double    Get_Hertz_Freq_from_MidiNumber(  short  midiNum  );


extern    double   iir_BandPass_Low1[];
extern    double   iir_BandPass_Mid1[];
extern    double   iir_BandPass_Hi1[];




////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


SndSample::SndSample(   long  numSamps,    long  chkSize    /*  doStereo */       )     
{ 


// ************  FIX,   is hardwired,  pass in as parm    ******************
	_sampleRate =   DEFsAMPLERATE;     
// *************************************************************************	
	
	
	
	
	// default   11127L    **** ADJUST ************   JPM


    _ChunkMap =   NULL;  
    
    _chunkSize =    chkSize;    

	m_totalSamps =  0;
                              			  
    //  _devSamp  =    devSamp;    // OK ???



	m_bits                =     ( char*  )malloc(   numSamps   );

	m_bitsRightStereo =   NULL;


// ************************************************************************************************************

//  3/2011    I'm very sure that for Windows,  a char value of zero, is just that.  Set the  Local debug window to 'Hexadecimal Display'  and see that
// a zero value for a CHAR  has no bits set.  
//   Also step debug  Calc_AutoCorrelation_ByLag()  and watch the waves values showly rise to positive, and then slowli fall to ZERO and then slowly go into
//   NEGATIVE values.  Think that  '127'   might have come from the Macintosh and I might have been using a UNSIGNED CHAR.  
//
//   See   SndSample::Erase()
//
//  *******  NOW I have to wonder if all these assignments of  '127'   are WRONG!!!!  Slowly correct and TEST !!!!  ********************* 




	if(   m_bits   ==  NULL   )
	{
		ASSERT( 0 );
	}
	else
	{  m_totalSamps =  numSamps;      //   OPTIMIZE with a faster assignment 

		/***
		for(   int i = 0;   i< m_totalSamps;   i++   )
					m_bits[ i ] =  127;     //  ????? 127: no sound ?????
		****/
		memset(   m_bits,    0,   m_totalSamps   );    //   0:   will clear all BITS...   think that is what we want for Windows ???   3/11
	}



	if(   m_bitsRightStereo   !=  NULL   )
	{  
		/***
		for(   int i = 0;   i< m_totalSamps;   i++   )
					m_bitsRightStereo[ i ] =  127;     //  ????? 127: no sound ?????
		****/
		memset(   m_bitsRightStereo,    0,   m_totalSamps   );    //   0:   will clear all BITS...   think that is what we want for Windows ???   3/11

	}


	m_showProgressBar =   true;
}



											////////////////////////////////////////


SndSample::~SndSample()
{											     

     //   MAC:  Release_devSAMPLE(  _devSamp  );     _devSamp =  NULL;       // OK???

	if(   m_bits  !=  NULL   )
	{
		free( m_bits );    m_bits =  NULL;
	}
      

	if(   m_bitsRightStereo  !=  NULL   )
	{
		free( m_bitsRightStereo );    m_bitsRightStereo =  NULL;
	}
 

    if(    _ChunkMap  !=   NULL   )    
		 delete   _ChunkMap;    
}


											////////////////////////////////////////


void    SndSample::Erase()
{

     char  *dst; 
     long    numByts;  
	 long    i =  0;



	/****
	 char   buffer[] = "This is a test of the memset function";


     memset(  buffer,   127,   4 );   //  will NOT clear BITS

     memset(  buffer,   0,   4 );       //  will  clear BITS



	 char   bufferB[ 10 ];

	 for(  short k=0;    k < 5;    k++   )
	 {
		bufferB[ k ]  =   0;    //  will  clear BITS
	 }
	 *****/



    //   if((  _devSamp ==  NULL  )||(  Get_Length() == 0L  ))           return;   

	if(   m_bits  ==  NULL   )
	{
		ASSERT( 0 );  	
		return;
	}


     numByts =   Get_Length(); 
     dst         =   Get_StartByte();
    

	 /****
     for(   i=0L;    i<  numByts;    i++    )      // ***************  OPTIMIZE  *******************
     {  
		 *dst = 0;     
		 dst++;  
	 }  
	 ****/

//	memset(    dst,   127,   numByts   );      // ****************   WHICH is RIGHT ????  ***********************
	memset(    dst,     0,   numByts   );             //   0   will clear all BITS...   think that is what we want for Windows   3/11


// ************************************************************************************************************

//  3/11    I'm very sure that for Windows,  a char value of zero, is just that.  Set the  Local debug window to 'Hexadecimal Display'  and see that
//   a zero value for a CHAR  has no bits set.  
//   Also step debug  Calc_AutoCorrelation_ByLag()  and watch the waves values showly rise to positive, and then slowli fall to ZERO and then slowly go into
//   NEGATIVE values.  Think that  '127'   might have come from the Macintosh, and I might have been using a UNSIGNED CHAR back then 
//
//  *******  NOW I have to wonder if all these assignments of  '127'   are WRONG!!!!  Slowly correct and TEST !!!!  ********************* 



	if(   Is_Stereo_Sample()   )
	{

		if(   m_bitsRightStereo  ==  NULL   )
		{
			ASSERT( 0 );  	
			return;
		}

		 dst    =   Get_StartByte_RightStereo();
	    

		/****
		 for(   i=0L;    i<  numByts;    i++    )    
		 {  
			 *dst = 0;     
			 dst++;  
		 }  
		 ****/

//		 memset(    dst,   127,   numByts   );
		 memset(    dst,     0,   numByts   );     //   0:   will clear all BITS...   think that is what we want for Windows ???   3/11
	}
}




											////////////////////////////////////////


bool	  SndSample::Make_Stereo(  CString&  retErrorMesg  )
{

	
	if(  m_bitsRightStereo  !=  NULL  )
	{
		/****
		ASSERT( 0 );   //  THIS is bad ....

		free(   m_bitsRightStereo   );   // want an Error ????  Or keep going???   5/07  
		m_bitsRightStereo =  NULL;
		****/

	//	ASSERT( 0 );   ...could this ever be a problem ????  6/07

		return  true;   //  If it is alread stereo, then we have nothing to do.   OK ?????????????????????  6/07  
	}



	if(   m_totalSamps   <=  0   )
	{
		retErrorMesg  =   "SndSample::Make_Stereo FAILED,    m_totalSamps is zero." ;
		return  false;
	}



	m_bitsRightStereo        =     ( char*  )malloc(   m_totalSamps   );
	if(   m_bitsRightStereo   ==  NULL   )
	{		
		retErrorMesg  =   "SndSample::Make_Stereo FAILED,   malloc()  failed to get memory." ;
		return  false;
	}


	/***
	for(   int i = 0;   i< m_totalSamps;   i++   )
		m_bitsRightStereo[ i ] =  127;     //  ????? 127: no sound ?????
	****/
	memset(   m_bitsRightStereo,    0,   m_totalSamps   );    //   0:   will clear all BITS...   think that is what we want for Windows ???   3/11


	return  true;
}



											////////////////////////////////////////


bool	   SndSample::Make_Mono(   CString&  retErrorMesg  )
{

	if(   m_bitsRightStereo  !=  NULL   )
	{
		free(   m_bitsRightStereo   );     // want an Error ????  Or keep going???   5/07  
		m_bitsRightStereo =  NULL;
	}
	else
	{	//  ASSERT( 0 );     //   5/07   is this a BIG deal,  theoretically the calling function know the status of this  SndSample 
	}       //  came here with the PlayNoteTool...   seems OK   5/23  

	return  true;
}



											//////////////////////////////


bool    SndSample::Is_Stereo_Sample()
{

	if(    m_bitsRightStereo ==  NULL   )
		return  false;
	else
		return  true;
}


											////////////////////////////////////////


char*   SndSample::Get_StartByte()    //    _LeftStereo
{

	/***   ...MAC
     if(   _devSamp == NULL  )    
				return   NULL;
     else     return   Get_Samples_StartByte(  _devSamp  );
	 ***/

	if(   m_bits  ==  NULL   )
	{
		ASSERT( 0 );
		return   NULL;
	}
	else
		return   m_bits;
}


											////////////////////////////////////////


char*   SndSample::Get_StartByte_RightStereo()
{

	if(   m_bitsRightStereo  ==  NULL   )
	{
		ASSERT( 0 );
		return   NULL;
	}
	else
		return   m_bitsRightStereo;
}

											////////////////////////////////////////


long   SndSample::Get_Length()
{
	/***   ...MAC
     if(   _devSamp == NULL  )    
			  return   0L;   
     else   return   Get_Snds_Data_Length(  _devSamp  );   
	***/
	return   m_totalSamps;
}



											////////////////////////////////////////


void   SndSample::Reverse_Byte_Order(  long  validSampleCount   )
{

		//  Reverses the order of the BYTES in the RELEVANT part of a  SndSample.   Sometimes only the FIRST part of 
		//  the bytes are filled DATA, and the bytes after that are zeroes.  So 'validSampleCount'  tell us if we are only
		//  supposed to consider the FIRST 'validSampleCount'  number of bytes at the START of the oversized  SndSample.  3/11


	if(   m_bits  ==  NULL   )
	{
		ASSERT( 0 );  	
		return;
	}


	ASSERT(   (validSampleCount % 2 )  ==  0   );     //  only for EVEN number of samples


	long    numByts =   Get_Length();

	ASSERT(    validSampleCount  <=   numByts   );




	long   validHalfDistance  =   validSampleCount /2;


	char   *src =    Get_StartByte();     
						
	char	 *dst  =   Get_StartByte()    +    ( validSampleCount  - 1  );  //  remember, the last half of the bytes might be zero for SlowSpeeds,
																									//  so we can NOT automatically start at the end of the memory block.  11/11



	for(   long i =0;     i < validHalfDistance;    i++   )
	{
		char  holdValue  =   *src;

		*src =   *dst;

		*dst =    holdValue;


		src++;
		dst--;
	}
}


											////////////////////////////////////////


SndSample*   SndSample::Clone()
{

	 //   devSAMPLE   devSoundHandle= NULL;
  // if(  _devSamp ==  NULL  )           return NULL;   

	SndSample  *dstSamp  =  NULL;

	CString    retErrorMesg;
  

     long  numByts =   Get_Length();

  

	/***
     if((  devSoundHandle =  Alloc_Snds_Memory( numByts )  )  == 0  )   
         {  TRACE( "***MemFAIL [ SndSample::Clone ]" );   return NULL; }
            
     dstSamp =   new   SndSample(  devSoundHandle,  _chunkSize  );  // *** OK for chunksize ????
	***/
	dstSamp    =        new    SndSample(   numByts,    _chunkSize   );
	if(  dstSamp  ==  NULL   )
	{
		ASSERT( 0 );
		return  NULL;
	}



	if(    Is_Stereo_Sample()    )   //  maybe have to make the Clone  stereo as well 
	{

		if(   ! dstSamp->Make_Stereo( retErrorMesg  )     )
		{
			AfxMessageBox(  retErrorMesg  );
			return  NULL;
		}
	}




	char   *src,  *dst;

		//  Copy_devSAMPLE(   _devSamp,   devSoundHandle  );
		 //  long                   len,  i;
		 //   if((  srcSmp == NULL  )||(  dstSmp == NULL  ))     return;
		 //  len =  Get_Snds_Data_Length(  srcSmp  ); 
   
												 // *** HLock( devSoundHandle ); ...NEEDED???? 
	src =    this->Get_StartByte();                //    Get_Samples_StartByte(  srcSmp  );
	dst =    dstSamp->Get_StartByte();      //  Get_Samples_StartByte(  dstSamp->  );
     

	if(   src  !=  NULL    &&    dst  !=  NULL   )
		memcpy(   dst,  src,   numByts   );    // ******** SHOULD i be LOCKING( &unLocking ) memory???????? **************
	else
	{	AfxMessageBox(  "SndSample::Clone FAILED,   src or dst  in LEFT stereo  is NULL." );
		return  NULL;
	}
	 


	if(    Is_Stereo_Sample()    )
	{
		src =    this->Get_StartByte_RightStereo();                //    Get_Samples_StartByte(  srcSmp  );
		dst =    dstSamp->Get_StartByte_RightStereo();      //  Get_Samples_StartByte(  dstSamp->  );
	     
		if(   src  !=  NULL    &&    dst  !=  NULL   )
			memcpy(   dst,  src,   numByts   );    // ******** SHOULD i be LOCKING( &unLocking ) memory???????? **************
		else
		{	AfxMessageBox(  "SndSample::Clone FAILED,   src or dst  in RIGHT stereo  is NULL." );
			return  NULL;
		}
	}

  
    return   dstSamp;
}




											////////////////////////////////////////


bool    SndSample::Create_Flanged_Sample(  long  delayInSamples,    CString&  retErrorMesg   )
{


	double   reduxVolume  =  1.00;     //  .50


	if(   m_bits  ==  NULL   )
	{
		retErrorMesg =   "SndSample::Create_Flanged_Sample  FAILED,  m_bits  ==  NULL" ;
		return  false;
	}

	if(   ! Is_Stereo_Sample()    )   //  maybe have to make the Clone  stereo as well 
	{

		retErrorMesg =   "SndSample::Create_Flanged_Sample  FAILED,  this is not a stereo signal." ;
		return  false;
	}




	 long  numByts            =   Get_Length(); 
     
	 long  remainingBytes  =    numByts  -   delayInSamples   - 100;   //  -100,  padding


												 // *** HLock( devSoundHandle ); ...NEEDED???? 
	char   *src =                 this->Get_StartByte();   

	char   *dstDelayed =    this->Get_StartByte_RightStereo();                //    Get_Samples_StartByte(  srcSmp  );

	double   val;



	 for(  long  i=0L;    i<  delayInSamples;    i++    )    
	 {  
		 *dstDelayed = 0;     
		  dstDelayed++;  
	 }  


     for(  long i=0L;    i<  remainingBytes;    i++    )    
     {  

		 val =     (double)(  *src  );    

		 *dstDelayed =    (char)(   val  *  reduxVolume   );  

		 src++;  
		 dstDelayed++;  
	 }  


	return  true;
}


											////////////////////////////////////////


bool   SndSample::Copy_LeftStereo_To_Right(   CString&  retErrorMesg   )
{


	if(   m_bits  ==  NULL   )
	{
		retErrorMesg =   "SndSample::Copy_LeftStereo_To_Right  FAILED,  m_bits  ==  NULL" ;
		return  false;
	}

	if(   ! Is_Stereo_Sample()    )   //  maybe have to make the Clone  stereo as well 
	{

		retErrorMesg =   "SndSample::Copy_LeftStereo_To_Right  FAILED,  this is not a stereo signal." ;
		return  false;
	}



	 long  numByts            =   Get_Length(); 
     
												 // *** HLock( devSoundHandle ); ...NEEDED???? 
	char   *src =        this->Get_StartByte();   

	char   *dst =        this->Get_StartByte_RightStereo();               



     for(  long i=0L;    i<  numByts;    i++    )    
     {  
		 *dst =   *src;     

		 src++;  
		 dst++;  
	 }  

	return  true;
}



											////////////////////////////////////////


SndSample*    SndSample::Clone_DownSampled(   long  downRatio   )
{


	if(   Is_Stereo_Sample()   )
	{
		//  INSTALL code   5/07
		ASSERT( 0 );
		return  NULL;
	}



	 if(   downRatio  <=  0    )
	 {
		ASSERT( 0 );
		return   NULL;
	 }

     long  numByts  =   Get_Length();



	 long  nuLength  =               numByts    /  downRatio;   



	 long  nuChunkSize   =     _chunkSize    /  downRatio;      // ****????    _chunkSize change for DOWNsampling ??? ***

	 long  nuSampleRate =    _sampleRate   /  downRatio;     // ****????     OK ???? *******





	 SndSample   *dstSamp    =      new    SndSample(   nuLength,    nuChunkSize   );   
     if(  dstSamp ==  NULL  )    
	 {
		ASSERT( 0 );
		return   NULL;
	 }
	 else
	 {  dstSamp->_sampleRate  =     nuSampleRate;     //  CAREFUL with reprocussions for this NEW change   .... 2/20002 
		 
		 
		 char   *src,  *dst,   srcVal;   
												 // *** HLock( devSoundHandle ); ...NEEDED???? 

		 src =    this->Get_StartByte();                //    Get_Samples_StartByte(  srcSmp  );
		 dst =    dstSamp->Get_StartByte();      //  Get_Samples_StartByte(  dstSamp->  );



		 for(    int i =0;     i <  nuLength;     i++   )
		 {
			srcVal  =   *src;

			*dst     =    srcVal;


			src  +=    downRatio;   // ***** WORKS ???  *********

			dst++;
		 }
	 }
	 
  
     return   dstSamp;
}




											////////////////////////////////////////


bool    SndSample::CopyBits_From_16bit_Sample(   short  channelCode,	  char  *srcByte,   
																			long  sampOffset,   long  totalSamples,   CString&  retErrorMesg  )
{

					// This is fine.  have been using it a lot for PitchPlayer.exe    1/10

	if(   Is_Stereo_Sample()    )
	{
		//  INSTALL code   5/07
		ASSERT( 0 );
		retErrorMesg =   "SndSample::CopyBits_From_16bit_Sample FAILED,  needs code for stereo SndSample."  ; 
		return  false;
	}



									 //   write BACK to the ACTUAL buffer used for playback
	                                      

	char  *dest =    Get_StartByte();      //  the 8-bit buffers

	long   bytesInSample = 4,     offsetInBytes;   // ***** HARDWIRE:  4 bytes per sample  *********



	offsetInBytes  =   sampOffset   *   bytesInSample;   //  offset into the SRC-bytes

	srcByte +=  offsetInBytes;




//	long    totalSamples  =     dstSndSample.Get_Length();



	retErrorMesg.Empty();

	if(   srcByte == NULL    )
	{
		retErrorMesg  =    "SndSample::CopyBits_From_16bit_Sample  failed,  NULL source byte." ;
		return  false;
	}

	if(    totalSamples  <=  0    )
	{
		retErrorMesg  =    "SndSample::CopyBits_From_16bit_Sample  failed,  SoundSample is empty." ;
		return  false;
	}

	if(   dest ==  NULL    )
	{
		retErrorMesg  =    "SndSample::CopyBits_From_16bit_Sample  failed,  SoundSample is NULL." ;
		return  false;
	}



	short  leftVal,   rightVal;


	if(   channelCode ==    TransformMap::LEFTj    )       //    doLeftChannel   )
	{

		for(    int  k=0;     k<  totalSamples;      k++   )     
		{
			srcByte++; 				
			*dest =   *srcByte;               //  left  
leftVal =   *srcByte; 

			srcByte++;    


			dest++;  

			srcByte++;  
			//  *destRight =  *srcByte;         //  right
			srcByte++;    
			//  destRight++;
		}

	}
	else if(   channelCode ==    TransformMap::RIGHTj    )   
	{

		for(    int  k=0;     k<  totalSamples;      k++   )     
		{
			srcByte++; 				
			//  *destLeft =   *srcByte;           //  left  
			srcByte++;    
		

			srcByte++;  
			*dest =  *srcByte;              //  right
			srcByte++;    


			dest++;
		}
	}
	else if(     channelCode ==    TransformMap::CENTEREDj      )   
	{

		for(    int  k=0;     k<  totalSamples;      k++   )     
		{


/***   from  BitSourceStreamingMS::Create_8bit_Sample_Segment( )  ...just to check accuracy of this    12/3

				src++; 							//  left  
				//  *destLeft =   *src;          
				leftVal =   *src;  

				src++;    
				//  destLeft++;


				src++;							//  right
				// *dest =  *src;             
				rightVal =   *src;  
				src++;    
***/


			srcByte++; 					//  left  
			leftVal =   *srcByte;           
			srcByte++;    
		

			srcByte++;					//  right
			rightVal =   *srcByte;  

			*dest =     (char)(      (  leftVal  +  rightVal  ) /2     );   //  create a MONO signal,  by averaging left and right
			srcByte++;    


			dest++;
		}

	}
	else
	{  retErrorMesg  =    "SndSample::CopyBits_From_16bit_Sample  failed,  unknown channelCode." ;
		return  false;
	}


	return  true;
}

											////////////////////////////////////////



void    SndSample::Reverse_16bit_Sample_Order(   BYTE *srcByte,    long  byteCount   )
{

	if(   srcByte  ==  NULL   )
	{
		ASSERT( 0 );  	
		return;
	}


	long   sampleCount  =   byteCount /  4;


	if(    (sampleCount % 2 )  !=   0     )			//   Think it should still work OK,  it will just not process the MIDDLE Sample in the array,  
	{																//   which is what is supposed to do.     12/22/2011
		int   dummy =  9;    //   STEP tested and seems fine.  12/11
	}



	long   halfNumberOfSamples =   sampleCount / 2;




	BYTE  *travSRC =    srcByte  +   ( byteCount - 4 );

	BYTE  *travDST =    srcByte;


														//  The logic in this is very similar to   SndSample::Reverse_Byte_Order()    3/11


//	for(   long  samp =0;     samp <  halfNumberOfSamples;     samp++   )    //   **** NO, need to do ALL the samples ( 'numberSamples' ) because we are
																											   //   copying from one MemoryBlock to a DIFFERENT 2nd MemoryBlock ( we are NOT
																											   //   merely reordering the bytes within a SINGLE Memory Block   11/11
//	for(   long  samp =0;     samp <  numberSamples;     samp++   )  


	for(   long  samp =0;     samp <  halfNumberOfSamples;     samp++   ) 
	{                                                                                                         

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




										////////////////////////////////////////////////


void    SndSample::Reverse_16bit_Samples_On_Copy(    BYTE *src,     BYTE  *dst,     long  numberOfBytes   )
{

					//   used to be in   StreamingAudioplayer::  , but is now STATIC function  12/11


// *************   INSTALL   3/11   this logic for  Apply_Volume_Adjust_PPlayer()   ??? so that the sound can be modified as the bits are copied in reverse *************


	if(      src ==  NULL    ||    dst ==  NULL  
		||   numberOfBytes <= 0  )
	{
		ASSERT( 0 );
		return;
	}

/***  OLDER CODE ( its a little buggy )

	BYTE*   travSrc =    src  +   ( numberOfBytes  );     //     FAILS for  ( numberOfBytes -1 )

	for(   long i =0;     i <  numberOfBytes;     i++   )
	{
		*dst  =    *travSrc;			//  Copy the Play-Bytes from  the  WAV-DelayBuffer  in REVERSE order to the  SoundHARDWARE for immediate play.		 
		 dst++;   
		 travSrc--;
	}
***/
	long   numberSamples           =    numberOfBytes /4;

//	long   halfNumberOfSamples  =   numberSamples  /2;    WRONG,  see below.  


	BYTE  *travSRC =    src  +   ( numberOfBytes - 4 );

	BYTE  *travDST =    dst;


														//  The logic in this is very similar to   SndSample::Reverse_Byte_Order()    3/11


//	for(   long  samp =0;     samp <  halfNumberOfSamples;     samp++   )    //   **** NO, need to do ALL the samples ( 'numberSamples' ) because we are
																											   //   copying from one MemoryBlock to a DIFFERENT 2nd MemoryBlock ( we are NOT
																											   //   merely reordering the bytes within a SINGLE Memory Block   11/11
	for(   long  samp =0;     samp <  numberSamples;     samp++   )            
	{                                                                                                         

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



											////////////////////////////////////////


double   SndSample::Calc_AutoCorrelation_ByLag(   long  numSampsInLag,   char* srcStart,    long  offsetIntoSample,
											                                        long samplesInWindow,   long&  retZeroCrossingCount     )
{

												//  MY version that uses  Differences...   


	double   redux  =    0.07;     //        0.50        0.10      ******  ADJUST ******




	long  zeroCrossingSpacing =  1;

	char     *templatePtr,   *lagPtr;
	long       diff;
	double    retScore = 0.0,    sumDiffs= 0.0;
	short     curVal,   prevVal=0;
//	short     prevPrevVal=0,   prevPrevPrevVal=0;



	if(   Is_Stereo_Sample()    )
	{
		//  INSTALL code   5/07
		ASSERT( 0 );
		AfxMessageBox(   "SndSample::Calc_AutoCorrelation_ByLag FAILED,  needs code for stereo SndSample."   ); 
		return  0.0;
	}



	retZeroCrossingCount =   0;    //  init 



	templatePtr =       srcStart  +  offsetIntoSample;

	lagPtr      =    (  srcStart  +  offsetIntoSample  )    +   numSampsInLag;



	for(  long  i= 0;     i < samplesInWindow;     i++   )
	{

	//	templVal =   (long)( *templatePtr );            //  Later  optimize with this:     templVal =   (long)( *templatePtr++ );
	//	lagVal =       (long)( *lagPtr ); 

		curVal  =     (short)( *templatePtr );  



		/***  Think this will give double results and not smooth out

		if(            (   curVal >0     &&   prevPrevPrevVal <0    )
			   ||	  (   curVal <0     &&   prevPrevPrevVal >0    )     )
			retZeroCrossingCount++;
		***/

		

		if(            (   (curVal -2) >0     &&   (prevVal +2) <0    )
			   ||	  (   (curVal +2) <0     &&   (prevVal -2) >0    )     )
			retZeroCrossingCount++;
	

		/****
		if(            (   curVal >0     &&   prevVal <0    )
			   ||	  (   curVal <0     &&   prevVal >0    )     )
			retZeroCrossingCount++;
		****/



		diff  =    absj(    (long)( *templatePtr )  -  (long)( *lagPtr )     );   


		sumDiffs  +=    (double)( diff  *  diff );     //  *******  should SQUARE to magnify differences  **********




		prevVal  =    curVal;


		templatePtr++;
		lagPtr++;
	}



	retScore =    redux   *   (  sumDiffs   /  (double)samplesInWindow  );     //  Make measurememnt  proportional to  samplesInWindow 



	/*****
	successVal  =      256L    -   avgError;      // ****** ADJUST *******


	if(    successVal  <  0L    )
	{
		int   dummyBreak = 9;

		retScore =  0;
	}
	else if(   successVal   >  255L   )
	{
		int   dummyBreak = 9;

		retScore =  255;
	}
	else
	   retScore  =    (short)successVal;
	*****/


	retZeroCrossingCount  =    retZeroCrossingCount / 10;   // **** REDUX, for easier number 


	return   retScore;
}




											////////////////////////////////////////


bool    SndSample::Apply_Sound_Filter(    int   filterType,     long  parm1,    long  parm2,   	long&  retValue,     
																											CString&  retErrorMesg   )
{


					//  also used to  ADJUST the VOLUME  of the sample


//  HiBoostHiSndFilter      filt(  m_sndSample  );  
//	HiBoostMedSndFilter   filt(  m_leftSample  );  


//	IIRsndFilter   filt(   m_sndSample,    1L,    iir_BandPass_Mid1    );   //  iir_BandPass_Mid1   iir_BandPass_Hi1
//	filt.Filter(); 


	retValue =   -1;
	retErrorMesg.Empty();


	if(   Is_Stereo_Sample()    )
	{
		//  INSTALL code   5/07
		ASSERT( 0 );
		retErrorMesg =   "SndSample::Apply_Sound_Filter FAILED,  needs code for stereo SndSample."  ; 
		return false;
	}



	switch(   filterType  )   
	{


		case   kAutoVolumeAdj :
		{															// **** use BRACKETS to avoid the initialization warning from Compiler *****
			//  ASSERT(   parm1  > 0   );  param1  is no longer used for this filter

			VolumeAutoAdjFilter    filter(   this,    (short)parm1   ); 	  

			if(    !filter.Filter(  retErrorMesg  )     )
				return  false;

			retValue =    filter.m_retScalePercentage;
		}  
		break;




		case   kIIRsecondSndFilter :				//  needs 2  parms
		{			 
			ASSERT(  parm1 >0  );      ASSERT(  parm2 >0  );

			short   dummyParm =  -1;

			IIRsecondSndFilter    filter(    this,      (short)parm1,	   (short)parm2   ); 
										      //    topFrequencyLimit,	   bottomFrequencyLimit,         topVal  ???  

			if(    !filter.Filter(    retErrorMesg  )     )
				return  false;
		}  
		break;



		case   kIIRsndFilterLow :
		{
			IIRsndFilter    filter(   this,    1L,    iir_BandPass_Low1    ); 

			if(    !filter.Filter(     retErrorMesg  )     )
				return  false;
		}  
		break;


		case   kIIRsndFilterMid :
		{
			IIRsndFilter    filter(   this,    1L,    iir_BandPass_Mid1    ); 

			if(    !filter.Filter(    retErrorMesg  )     )
				return  false;
		}  
		break;


		case   kIIRsndFilterHi :
		{
			IIRsndFilter    filter(   this,    1L,      iir_BandPass_Hi1    ); 

			if(    !filter.Filter(   retErrorMesg  )     )
				return  false;
		}  
		break;



		case   kMidFlatSndFilter :
		{
			MidFlatSndFilter    filter(   this  ); 

			if(    !filter.Filter(    retErrorMesg  )     )
				return  false;
		}  
		break;


		default:   ASSERT( 0 );      break;
	}


	return  true;
}




											////////////////////////////////////////


bool    SndSample::Apply_Full_Filtering(   long  topFrequencyLimit,    long  bottomFrequencyLimit,   long&   retScalePercent,	  CString&  retErrorMesg   )
{



							//   measure the  'TOP-value'  of the sample BEFORE it has possibly had its VOLUME changed

							//  This also gets called for auto correlation(  FundCandidEvaluator::FundCandidEvaluator  )

	retScalePercent =  -1;


	/***
	long     minBytesForProgressBar =    150000;   // **** ADJUST, do not let FundCandidEvaluator::FundCandidEvaluator  trigger ProgressBar  ****




	long    discardPercent =        5;    //  *** ADJUST,  in TENTHS of Percent !!!  ***
  	long     bucketCount   =  128,    	  retAbsoluteTopBefore,    retAbsoluteTopAfter,     retScalePercentVolAdjust =  -1;
	long     population[         129  ];

	VolumeAutoAdjFilter    measurementFilter(   this,    -1   ); 	
	****/


	if(   Is_Stereo_Sample()    )
	{
		//  INSTALL code   5/07
		ASSERT( 0 );
		retErrorMesg =   "SndSample::Apply_Full_Filtering FAILED,  needs code for stereo SndSample."  ; 
		return false;
	}	


	/***
	if(        m_totalSamps   >  minBytesForProgressBar 
		&&   enableProgressBar  )
	{
		Begin_ProgressBar_Position_GLB(   "Sound filtering..."    );   //   in  SPitchListWindow.cpp  
		Set_ProgressBar_Position_GLB(   10   );
	}
	***/



	/***
	measurementFilter.Get_Amplitude_Populations(    population,     bucketCount   );

	long   topValBefore    =       measurementFilter.Get_Top_Value(    discardPercent,    population,    bucketCount,   retAbsoluteTopBefore   );
	if(      topValBefore  ==  0    )
	{		
	   int  dummy =  9;   	//  Get here on DoWhatLike( end ).  Low signal, does not seem like a problem  7/06
	}

	if(           m_totalSamps   >  minBytesForProgressBar   
		   &&   enableProgressBar   )
		Set_ProgressBar_Position_GLB(   30   );

	***/


							//     'FILTER  the  FREQUENCY'   8-bit sample  for the target  frequency range( filter will AUTO-Adjust volume )
							
//	long   retDummy;


	if(     topFrequencyLimit   > 0      &&      bottomFrequencyLimit  > 0     )
	{

		/****

			IIRsecondSndFilter    filter(    this,      (short)parm1,	   (short)parm2   ); 
										      //    topFrequencyLimit,	   bottomFrequencyLimit,      

			

			retValue =    filter.m_retScalePercentage;

		****/

		IIRsecondSndFilter    filter(    this,       topFrequencyLimit,     bottomFrequencyLimit    ); 

	//	if(    ! Apply_Sound_Filter(    kIIRsecondSndFilter,     topFrequencyLimit,     bottomFrequencyLimit,   retDummy,     retErrorMesg  )   )

	

		if(    !filter.Filter(    retErrorMesg  )     )
			return  false;
		


		retScalePercent  =    filter.m_retScalePercentage;
	}	
	else   //  if there is not filtering by the 
	{

		/***
			VolumeAutoAdjFilter    filter(   this,    (short)parm1   ); 	  

			if(    !filter.Filter(  retErrorMesg  )     )
				return  false;

			retValue =    filter.m_retScalePercentage;

		****/
		VolumeAutoAdjFilter    filter(   this,   -1    ); 	  


//		if(    ! Apply_Sound_Filter(    kAutoVolumeAdj,     -1,   -1,     retScalePercentVolAdjust,   retErrorMesg  )  )

		if(    !filter.Filter(  retErrorMesg  )     )
			return   false;
		

		retScalePercent  =    filter.m_retScalePercentage;
	}



	/***
	if(           m_totalSamps   >  minBytesForProgressBar  
		   &&   enableProgressBar  )
		Set_ProgressBar_Position_GLB(   50   );
	***/






	/***
	measurementFilter.Get_Amplitude_Populations(   population,    bucketCount   );

	long   topValAfter    =       measurementFilter.Get_Top_Value(    discardPercent,    population,    bucketCount,   
																														retAbsoluteTopAfter  );
	if(      topValAfter  ==  0    )
	{		
		ASSERT( 0 );   // **** INSTALL   code ???    if(  >=  126,  might have a problem )    
	}
	***/




	/***																//	 will later ATTENUATE by this RATIO of  'Before and After'   VOLUMES 
	if(          topValBefore  > 0     
		  &&   topValAfter    > 0    )
		retScalePercent  =      (  100L   *   topValAfter  )    /   topValBefore;     //  in percent,   gets saved in the Project File
	else
	{
		
		if(              topValBefore  >    0     //  In DoWhatYou like, waveform is weak at end, but still get a value for 
			     &&    topValAfter    <=  0   
			     &&    retAbsoluteTopBefore  > 0   )
			retScalePercent  =      (  100L   *   topValAfter  )    /   retAbsoluteTopBefore;     //  in percent,   gets saved in the Project File
		else
			retScalePercent =  -1;
		
		retScalePercent =  -1;   //  YES, tests OK. 7/06   Maybe best not to appl thiis later readjust, becase other part of Amplitude would become too high from thhis ratio  7/06
	}
	***/




/****				//    REVISE the  initial AmplitudeTransform with this FINAL 8-bit sample 

	char  *staticBits8bit  =     nuSample->Get_StartByte();


	if(    ! Make_Amplitude_Transform_from_8bitSample(    channelCode,    staticBits8bit,   sampleCount,   retErrorMesg  )   )  
		return  false;
// ******************  PROBLEM, doe not do the whole number of samples  5/09 ***************

*****/

	/***
	if(          m_totalSamps   >  minBytesForProgressBar 
		   &&  enableProgressBar   )
		End_ProgressBar_Position_GLB();
	***/


	return  true;
}


											////////////////////////////////////////


bool    SndSample::Apply_Full_Filtering_FixedScaling(   long  topFrequencyLimit,   long  bottomFrequencyLimit,   long  inputScalePercent,     double&  retOverbriteRatio,	 CString&  retErrorMesg   )
{



							//   measure the  'TOP-value'  of the sample BEFORE it has possibly had its VOLUME changed

							//  This also gets called for auto correlation(  FundCandidEvaluator::FundCandidEvaluator  )



	retOverbriteRatio =  -1.0;

	retErrorMesg.Empty();


	/***
	long     minBytesForProgressBar =    150000;   // **** ADJUST, do not let FundCandidEvaluator::FundCandidEvaluator  trigger ProgressBar  ****


	long    discardPercent =        5;    //  *** ADJUST,  in TENTHS of Percent !!!  ***
  	long     bucketCount   =  128,    	  retAbsoluteTopBefore,    retAbsoluteTopAfter,     retScalePercentVolAdjust =  -1;
	long     population[         129  ];

	VolumeAutoAdjFilter    measurementFilter(   this,    -1   ); 	
	****/


	if(   Is_Stereo_Sample()    )
	{
		//  INSTALL code   5/07
		ASSERT( 0 );
		retErrorMesg =   "SndSample::Apply_Full_Filtering FAILED,  needs code for stereo SndSample."  ; 
		return false;
	}	







							//     'FILTER  the  FREQUENCY'   8-bit sample  for the target  frequency range( filter will AUTO-Adjust volume )
							
//	long   retDummy;


	if(     topFrequencyLimit   > 0      &&      bottomFrequencyLimit  > 0     )
	{


		IIRsecondSndFilter    filter(    this,       topFrequencyLimit,     bottomFrequencyLimit    ); 

	

//		if(    ! filter.Filter(    retErrorMesg  )     )
		if(   ! filter.Filter_Fixed_Scaling(     inputScalePercent,      retOverbriteRatio,    retErrorMesg   )      )
			return  false;
		


//		retScalePercent  =    filter.m_retScalePercentage;
	}	
	else
	{	retErrorMesg   =  "SndSample::Apply_Full_Filtering_FixedScaling  FAILED,  bad input parms"   ;
		return  false;
	}




	/***
	measurementFilter.Get_Amplitude_Populations(   population,    bucketCount   );

	long   topValAfter    =       measurementFilter.Get_Top_Value(    discardPercent,    population,    bucketCount,   
																														retAbsoluteTopAfter  );
	if(      topValAfter  ==  0    )
	{		
		ASSERT( 0 );   // **** INSTALL   code ???    if(  >=  126,  might have a problem )    
	}
	***/




	/***																//	 will later ATTENUATE by this RATIO of  'Before and After'   VOLUMES 
	if(          topValBefore  > 0     
		  &&   topValAfter    > 0    )
		retScalePercent  =      (  100L   *   topValAfter  )    /   topValBefore;     //  in percent,   gets saved in the Project File
	else
	{
		
		if(              topValBefore  >    0     //  In DoWhatYou like, waveform is weak at end, but still get a value for 
			     &&    topValAfter    <=  0   
			     &&    retAbsoluteTopBefore  > 0   )
			retScalePercent  =      (  100L   *   topValAfter  )    /   retAbsoluteTopBefore;     //  in percent,   gets saved in the Project File
		else
			retScalePercent =  -1;
		
		retScalePercent =  -1;   //  YES, tests OK. 7/06   Maybe best not to appl thiis later readjust, becase other part of Amplitude would become too high from thhis ratio  7/06
	}
	***/


	return  true;
}




											////////////////////////////////////////



bool    SndSample::Apply_VolumeAdjust_Filtering(    long&   retScalePercentVolAdjust,			CString&  retErrorMesg   )
{


	retScalePercentVolAdjust =  -1;

	
	long    discardPercent =        5;    //  *** ADJUST,  in TENTHS of Percent !!!  ***
  	long     bucketCount   =  128,    	  retAbsoluteTopBefore,    retAbsoluteTopAfter;    
	long     population[         129  ];

	VolumeAutoAdjFilter    measurementFilter(   this,    -1   ); 	  //  NO filtering,  just to get measurements 



	if(   Is_Stereo_Sample()    )
	{
		//  INSTALL code   5/07
		ASSERT( 0 );
		retErrorMesg =   "SndSample::Apply_VolumeAdjust_Filtering FAILED,  needs code for stereo SndSample."  ; 
		return false;
	}




	measurementFilter.Get_Amplitude_Populations(    population,     bucketCount   );



	long   absoluteTopBefore    =       measurementFilter.Get_Top_Value(    discardPercent,    population,    bucketCount,   retAbsoluteTopBefore   );
	if(      absoluteTopBefore  ==  0    )
	{		
	   int  dummy =  9;   	//  Get here on DoWhatLike( end ).  Low signal, does not seem like a problem  7/06
	}



	if(    ! Apply_Sound_Filter(    kAutoVolumeAdj,     -1,   -1,     retScalePercentVolAdjust,   retErrorMesg  )  )
	{
		ASSERT( 0 );
		return  false;
	}




	measurementFilter.Get_Amplitude_Populations(    population,     bucketCount   );


	long   absoluteTopAfter    =       measurementFilter.Get_Top_Value(    discardPercent,    population,    bucketCount,   retAbsoluteTopAfter   );
	if(      absoluteTopAfter  ==  0    )
	{		
	   int  dummy =  9;   	//  Get here on DoWhatLike( end ).  Low signal, does not seem like a problem  7/06
	}


	return  true;
}



									/////////////////////////////////////////////////////


bool    SndSample::Make_WAV_File(   CString&  destFilePath,      CString&  retErrorMesg   )     
{ 

					//   submit  'destFilePath'  as empty to launch a File Dialog,  but it will return the user's file name.


	bool   doStereoDest =    false;

	if(    Is_Stereo_Sample()    )
		doStereoDest =   true;



	long  totalSamples =   Get_Sample_Count();



	CString  strFilePath =   destFilePath;  

	if(    destFilePath.IsEmpty()   )
	{

		CFileDialog    dlg(     FALSE,
									   _T( "wav" ), 
									   NULL, 
									   OFN_HIDEREADONLY   |    OFN_OVERWRITEPROMPT,
									   _T(    "WAV Files (*.wav)|*.wav||" )    );

		LONG  nResult    =      dlg.DoModal();
		if(       nResult  !=   IDOK  )
		{
			retErrorMesg =   "Not perform  FSndSample::Make_WAV_File  because of USER cancel."  ;  
			return  false;			//  Do not want CALLING function to give a confirmation message
		}

		strFilePath =    dlg.GetPathName();
	}



	long	step,  pgVal,   progPosPercent =   10;

	if(  m_showProgressBar  ) 
	{

		Begin_ProgressBar_Position_GLB(  NULL  );   //	Begin_Wait_Cursor_GLB();

		Set_ProgressBar_Position_GLB(  progPosPercent );
	}



	
																			//	Copy the data bytes to the file   
	try
    {  
		CFile   file(    strFilePath, 
						   CFile::modeCreate    |  CFile::modeWrite
					   // CFile::modeCreate    |  CFile::modeWrite     |   CFile::modeNoTruncate
					  );														   // ***NOTE:   'modeNoTruncate'   allows APPEND to existing fie,  does not EMPTY/ERASE an EXISTING file


		DWORD   totalSampleBytes  =     totalSamples  *  4;
		DWORD   approxFileSize      =      totalSampleBytes  +  36;

		wavHeader.dWord1    =    approxFileSize;
		wavHeader.dWord10  =    totalSampleBytes;

		long    ldSize =    sizeof( JimboWAVfileHeader );
		file.Write(   &wavHeader,    ldSize   );	




	//  DWORD   mapStartPos  =     file.GetPosition();
		char     dest[ 4 ];


		char    *srcB = NULL;     
			

//////////////////////   CAREFUL  ////////////////////////////////
		char 	 *srcA =   m_bits;     ///    dstSampleA->Get_StartByte();       //  a secondary 8-bit buffer 

		/***
		if(   dstSampleB  !=  NULL   )
			 srcB   =   dstSampleB->Get_StartByte();   
		 ***/
		if(   m_bitsRightStereo  !=  NULL   )
			 srcB   =   m_bitsRightStereo;   
//////////////////////   CAREFUL  ////////////////////////////////



		step =   totalSamples /  ( 100 -  progPosPercent );


		for(   int  k=0;     k<  totalSamples;      k++   )     
		{


			if(   doStereoDest   )
			{
				dest[ 0 ] =     127;
				dest[ 1 ] =   *srcA;		  //  left  

				dest[ 2 ] =     127;
				dest[ 3 ] =   *srcB;       //  right  	
			}
			else
			{
				dest[ 0 ] =     127;
				dest[ 1 ] =   *srcA;		  //  left  

				dest[ 2 ] =     127;
				dest[ 3 ] =   *srcA;       //  right  	( the same for mono )
			}




			srcA++;

			if(  srcB   !=  NULL  )
				srcB++;



			long    ldSize =   4;
			file.Write(   dest,    ldSize   );	


			if(    (  k %  step )  ==  0   )
			{
				pgVal  =    k /  step;  
				if(  m_showProgressBar  )    Set_ProgressBar_Position_GLB(  pgVal  );
			}		

		}
	}
	catch(   CFileException   *pException   )
	{
		End_ProgressBar_Position_GLB();   

		TCHAR    szCause[ 255 ];  
		CString   strRaw;
        pException->GetErrorMessage(  szCause,  255  );
		pException->Delete();
		strRaw =  szCause;
		retErrorMesg.Format(   "PitchDetectorApp::FFT_Erode_StereoChannels_2File_OLD :  could not save WAV file because %s." ,   strRaw  );
		return  false;
	}



	destFilePath =   strFilePath;   //  return the user's file path to calling funct


	if(  m_showProgressBar  )    
		End_ProgressBar_Position_GLB();   //   End_Wait_Cursor_GLB();


	return  true;
}




					/////////////////////////////////////////////


bool   SndSample::Make_Float_Sample(  short channelCode,  char  *srcBytes,   long  totalByteCount,   long  bytesPerSample,
									                                              float  *dstSamplesPtr,    bool  lessThanOneValues,    CString&   retErrorMesg    )
{

	//   channelCode  {    0:  Left,    1: Right,    2:  Mono   }
	//
	//	bytesPerSample is for BOTH Left and fight Stereo(  ex:  is 4 for 16bit stereo   )


	//   lessThanOneValues -  will give values  in the range [  -1.0,  1.0  ]   by dividing by  reDuxFact  



//	float    reDuxFact  =     16390.0;      //   ****BAD
	float    reDuxFact  =     32800.0;       //  32800.0 :  Need a little bigger than  32768.0 to make sure is less than 1.  See test below.   12/11   **** could ADJUST,  but have to fix in other functs too 



	retErrorMesg.Empty();


	if(  srcBytes  ==  NULL  )
	{
		retErrorMesg =   "Make_Float_Sample failed,  srcBytes is NULL." ;
		return  false;
	}

	if(  dstSamplesPtr  ==  NULL  )
	{
		retErrorMesg =   "Make_Float_Sample failed,  dstSamplesPtr is NULL." ;
		return  false;
	}



	if(   channelCode  ==  2  )   //   mono
	{
		ASSERT( 0 );
		//  Need to thinkabout and write extra code for this

		retErrorMesg =   "Make_Float_Sample failed,  missing source code for mono case." ;
		return  false;
	}



	float   lessOneSampleVal =  0.0;

	long    totSamplesCnt =     totalByteCount  /  bytesPerSample;   
		
	short    samp[  4  ],      bytHi;   //  these worked badly
	long     signedSampleVal;


	float    *dstFloatPtr =    dstSamplesPtr;    //  init

	char    *src  =   srcBytes;     //  char:    Think I need this so that  'signedSampleVal'   does indeed get a signed value.


	long    leftSample16,   rightSample16;                                //    12/11 
	char    chVal0=0,    chVal1=0,   chVal2=0,    chVal3=0;       //    12/11



	if(   bytesPerSample  ==  4   )   //  16bit stereo
	{

		for(   long  k=0;     k<  totSamplesCnt;      k++   )     
		{

			samp[ 0 ] =   *src;			
			chVal0  =    *src;
			src++;	  //  left  

			samp[ 1 ] =   *src;			
			chVal1  =    *src;
			src++;


			samp[ 2 ] =   *src;			
			chVal2  =    *src;
			src++;    //  right  

			samp[ 3 ] =   *src;			
			chVal3  =    *src;
			src++;

			/****
			if(        samp[ 0 ]  <  0                Tests OK.      ...LOW Bytes should always be  0 -255      8/06
				||     samp[ 2 ]  <  0     ) 
			{  int  dummy =  0;   }
			****/


														// ***BEST***   I use this LOGIC in   BitSourceStreaming::Fetch_Streaming_Samples_Direct_PPlayer()
														//
														//     ...above logic looks wring for some values.  12/30/2011
			leftSample16    =    chVal1;

			leftSample16  =       (  leftSample16   <<  8  )    |    ( BYTE )chVal0;    //   Need cast so   '|'   will  pack  with negative numbers 


			rightSample16    =    chVal3;

			rightSample16  =      (  rightSample16   <<  8  )    |    ( BYTE )chVal2;    //   Need cast so   '|'   will  pack  with negative numbers 





			if(         channelCode ==  0   )
			{

				bytHi   =    samp[ 1 ]   <<   8;

			//	signedSampleVal   =     (    samp[ 1 ]   *  baseMult    )     +     samp[ 0 ];   //  BAD, static and crackel
			//	signedSampleVal   =      bytHi    +    samp[ 0 ];     // ***** Messed up****************************

				signedSampleVal =   leftSample16;   //   fixed   12/11
			}

			else if(  channelCode ==  1   )
			{

				bytHi   =    samp[ 3 ]   <<   8;

			//  signedSampleVal   =     (    samp[ 3 ]   *  baseMult    )     +     samp[ 2 ];
			//	signedSampleVal  =     bytHi    +    samp[ 2 ];

				signedSampleVal =    rightSample16;    //   fixed   12/11
			}

			else if(  channelCode ==  2   )
			{
				ASSERT( 0 );   // *******  INSTALL code  ********************
				retErrorMesg =   "Make_Float_Sample failed,  missing source code for channelCode = 2."  ;
				return  false;
			}
			else
			{  ASSERT( 0 );
				retErrorMesg =   "Make_Float_Sample failed,  missing channelCode case."  ;
				return  false;
			}



			if(   lessThanOneValues   )
			{

				lessOneSampleVal =     ( (float)signedSampleVal )   /   reDuxFact;

				*dstFloatPtr  =   lessOneSampleVal;

				ASSERT(    lessOneSampleVal <  1.0     &&     lessOneSampleVal >  -1.0    );    //    Land here if  reDuxFact is   =>   32768.0  		
			}
			else
				*dstFloatPtr  =    (float)signedSampleVal;



			dstFloatPtr++;
		}

	}
	else
	{  ASSERT( 0 );
		retErrorMesg =   "Make_Float_Sample failed,  bytesPerSample has bad value."  ;
		return  false;
	}

	return  true;
}




					/////////////////////////////////////////////


bool   SndSample::Merge_Float_Channels_To_Sample(  bool  toStereo,   long  sampleCount,   long  bytesPerSample,    float **leftSampleArray,   float **rightSampleArray,  
											                                           char **retDstSamplesPtr,   long& retByteCount,     bool  lessThanOneValues,    
																					   bool noMemoryRelease,    double  volumeTweak,    CString&   retErrorMesg  )
{

		// ******** CAREFUL -  this function will AUTOMATICALLY  FREE   the input ARRAYS ( **leftSampleArray,  **rightSampleArray,   )


		//   *** this is a STATIC function,  so I could put it ANY CLASS    ( SndSample ??? ,   ReSampler   ) 



	//  **OMIT ??? ****    lessThanOneValues -  will work with INPUT values  in the range [  -1.0,  1.0  ]   by multiplying up
	if(   lessThanOneValues   )
	{
		ASSERT( 0 );   // **************  Do I ever use this mode ???  If NOT,  pull it from the parms.      12/28/2011   *****************************
	}						  //
	                         //  Looks like only SndSample::Change_Samples_Pitch(), which is NOT connected,  is only one to set it to true.

	
	float    expandFactor  =  32768.0;    //  ONLY USED   if   lessThanOneValues = TRUE   





	long    bitsPerSample =   4;   //  *** HARDWIRED,  OK ???  ******

	retByteCount =  -1;

	retErrorMesg.Empty();



	if(    ! toStereo   )   //  Even when streaming in a MONO signal, it has already been converted to STEREO at this stage of the algo
	{  
		ASSERT( 0 );
		retErrorMesg =   "Merge_Float_Channels_To_Sample failed,  missing code for DST-Mono case."  ;  //  I have not yet created this code  8/06
		return  false;
	}


	if(   sampleCount  <=  0   )
	{  
		retErrorMesg =   "Merge_Float_Channels_To_Sample failed,  sampleCount is non-existant."  ;
		return  false;
	}


	if(     toStereo  
		&&  (   leftSampleArray ==  NULL   ||    rightSampleArray ==  NULL  )     )
	{  
		retErrorMesg =   "Merge_Float_Channels_To_Sample failed,  input array-pointers have not been assigned."  ;
		return  false;
	}

	if(     toStereo  
		&&    leftSampleArray ==  NULL    )   //   rightSampleArray can be NULL if doing Mono->Stereo
	{  
		retErrorMesg =   "Merge_Float_Channels_To_Sample failed,  input arrays are NULL."  ;
		return  false;
	}

	if(  bytesPerSample  !=  bitsPerSample   )
	{  
		retErrorMesg =   "Merge_Float_Channels_To_Sample failed,  bytesPerSample is not 4."  ;
		return  false;
	}

	if(    *retDstSamplesPtr ==  NULL     )
	{  
		retErrorMesg =   "Merge_Float_Channels_To_Sample failed,  retDstSamplesPtr is NULL."  ;
		return  false;
	}




	long     totalBytes    =      sampleCount  *   bytesPerSample;    //   sampleCount =  2116  for   24hz  to  44.1 

	float   *srcLeft   =    *leftSampleArray;
	float   *srcRight =    *rightSampleArray;   // might be NULL for Mono



	short    sampleValue,   highByte,   lowByte;

	float     sampleValueFloat;

	unsigned char   *destPtr  =    ( unsigned char* )*retDstSamplesPtr;





	double   upperLimit =   32500;       //   Remember,  out-going  Integer-Samples  are in range   {  -32768.0   to   +32768.0  }.    See below.    12/11

	double   lowerLimit =    upperLimit   *   -1.0;



	/****
	long   leftSample16,    rightSample16;    Keep arond in case do the below test again.    12/11

    volumeTweak =  1.0;   // ***   TEMP for test *****************
	****/



	for(   long k=0;    k < sampleCount;     k++  ) 
	{
																				//  LEFT  stereo

		sampleValueFloat  =    *srcLeft    *   volumeTweak;     //   *srcLeft   is roughly in range of    {  -32768.0   to   +32768.0  }


		if(         sampleValueFloat   >=   upperLimit   )
		{
			int  dummy =  9;
			sampleValueFloat =    upperLimit;
		}
		else if(   sampleValueFloat   <=   lowerLimit   )
		{
			int  dummy =  9;
			sampleValueFloat =    lowerLimit;
		}




		if(   lessThanOneValues   )
			sampleValue =    (short)(    sampleValueFloat  *  expandFactor     );  
		else
			sampleValue =    (short)sampleValueFloat;  
		
				
		srcLeft++;  


		/***********************************************************************   
		         This should be right, but simpler gives a better sgnal with Treats Mean. On  Create_Test_WAV_File()
		         even though the numbers are different, produces the same waveform in WavLab.
				 TreatsMean sound much better( less crackle ) with SIMPLE syntax.

		if(   sampleValue  >=  0   )
		{
			highByte  =    sampleValue  >>  8; 
			lowByte   =    sampleValue   &  0x00FF; 
		}
		else
		{  short   tp  =   -1 *  sampleValue;   // need positive sign for the bit shifting and mask to work right

			highByte  =    (  tp  >>  8  )   *  -1;     //  -1:  will preserve the sign for the pair
			lowByte  =        tp   &  0x00FF; 
		}
		***************************************************************/

	//	  WIERD????  BUG???    if negative,  get bad numbers for  highByte or not  ??? [ BELOW sound the BEST !!!  9/06  ]

		highByte =     sampleValue   >>   8;                 //  this will preserve the sign   [  SIMPLE syntax  ]
		lowByte  =     sampleValue    &    0x00FF;         //  256 units of decimal



		/****  GOOD reflexive TEST.   See  if  leftSample16   has the same value as  'sampleValue'   12/30/11

		leftSample16  =    highByte; 
		leftSample16  =       (  leftSample16   <<  8  )    |    ( BYTE )lowByte;      //   Need cast so   '|'   will  pack  with negative numbers 
		***/



		if(          highByte  >   127   )        //  Think the biggest we get is  126      12/11
		{  highByte =    127;    ASSERT( 0 );  
		}			
		else if(   highByte  <  -128   )       //   Think the smallest we get is  -127       12/11
		{  highByte =   -128;     ASSERT( 0 );   
		}			  
		

		*destPtr =   lowByte;       destPtr++;     
		*destPtr =   highByte;      destPtr++;  




																				//  RIGHT  stereo
		if(   *rightSampleArray   !=   NULL    )
		{

			//    sampleValue =   (short)(  *srcRight );    

			sampleValueFloat  =    *srcRight     *  volumeTweak;


			if(         sampleValueFloat   >=   upperLimit   )
			{
				int  dummy =  9;
				sampleValueFloat =    upperLimit;
			}
			else if(   sampleValueFloat   <=   lowerLimit   )
			{
				int  dummy =  9;
				sampleValueFloat =    lowerLimit;
			}



			if(   lessThanOneValues   )
				sampleValue =    (short)(    sampleValueFloat  *  expandFactor     );  
			else
				sampleValue =    (short)sampleValueFloat;  

			
			srcRight++;  
		}
		else
		{	//  Mono signal( to stereo ):  Do nothing,  use the LEFT channels value for  'sampleValue'
			int  dummy =  9;
		}




		/***
		if(   sampleValue  >=  0   )
		{
			highByte  =    sampleValue  >>  8; 
			lowByte  =    sampleValue   &  0x00FF; 
		}
		else
		{  short   tp  =   -1 *  sampleValue;   // need positive sign for the bit shifting and mask to work right

			highByte  =    (  tp  >>  8  )   *  -1;     //  -1:  will preserve the sign for the pair
			lowByte  =        tp    &  0x00FF; 
		}
		***/
		highByte =     sampleValue   >>   8;  
		lowByte  =     sampleValue    &    0x00FF;    


		/****  GOOD reflexive TEST.   See  if  rightSample16   has the same value as  'sampleValue'    12/30/11

		rightSample16   =    highByte;             //  see if we get the same values back   12/11

		rightSample16  =      (  rightSample16   <<  8  )    |    ( BYTE )lowByte;    //   Need cast so   '|'   will  pack  with negative numbers 
		****/



		if(          highByte  >   127   )     //  Think the biggest we get is  126            12/11
		{   highByte =    127;      ASSERT( 0 );    	
		}			
		else if(   highByte  <  -128   )     //   Think the smallest we get is  -127       12/11
		{   highByte =   -128;      ASSERT( 0 );      
		}			   
		

		*destPtr =   lowByte;       destPtr++;     // ******  OK ????  ***************************
		*destPtr =   highByte;      destPtr++;  
	}




																//  Cleanup and return values
	retByteCount  =    totalBytes;



	if(   ! noMemoryRelease    )
	{

		free(  *leftSampleArray    );      //  *****************   CAREFUL   12/2011  ******************************
		*leftSampleArray   =  NULL;	  //   Were intially allocated by   ReSampler::Interpolate_Data()   [ after 1st calling  ReSampler::Resample_Signal() 

		free(  *rightSampleArray  );      
		*rightSampleArray =  NULL;
	}

	return  true;
}




					/////////////////////////////////////////////


bool   SndSample::Multiply_Float_Samples(   long  sampleCount,    float  *sampleArrayPtr,    float  multFactor,    CString&   retErrorMesg  )
{

	retErrorMesg.Empty();

//	float   sixteenBitLimit =   31000.0;      //  could go almost to  32768.0    if I want


	if(       sampleArrayPtr ==  NULL   
		||   sampleCount <=  0    )
	{
		retErrorMesg =   "SndSample::Multiply_Float_Samples FAILED,  bad input sampleArrayPtr parm." ;
		return  false;
	}



	float   temp;


	for(    long k= 0;     k <  sampleCount;    k++    )
	{


		temp  =     *(   sampleArrayPtr  +  k   )    *    multFactor;



// **********  Do I really want this test here???   Maybe better done in  Merge_Float_Channels_To_Sample() that happens right after thisw

		/****

		if(         temp >    sixteenBitLimit )
			temp =    sixteenBitLimit;                //  got hits here...  may have been what caused the popping on some mp3 recordings    2/10
		else if(  temp  <  -sixteenBitLimit   )
			temp =   -sixteenBitLimit;		         //  got hits here...  may have been what caused the popping on some mp3 recordings
		*****/



		 *(   sampleArrayPtr  +  k   )   =   temp;
	}

	return  true;
}



											//////////////////////////////////////////////////
											/////////   static FFT Functs  1/2012 /////////
											//////////////////////////////////////////////////


void      SndSample::FFT_with_only_Real_Input(   int n,    float  *xr,   float  *xi   )     //  double *xr,   double *xi    ) 
{                                  	

		//    'n'    is only HALF the size  of the Relultant  FFTframeSize   !!!!!


		//		*xr   In:   sig[0],   sig[2],    sig[4],   ... 
        //		*xi    In:   sig[1],   sig[3],   sig[5],   ... 

		//		*xr    Out:   Real  parts
		//		*xi     Out:   Imaginary  parts

				//    Got this from Book by Timothy Masters pp. 87 , called  "Signal and Image Processing with Neural Networks"    1/12

   int        i, j;			

//   double  theta,  tp;  
 //  double  wr, wi, wkr, wki,  t,   h1r, h1i, h2r, h2i;  
   
   float  theta,  tp;  
   float  wr, wi, wkr, wki,  t,   h1r, h1i, h2r, h2i;  


   
   SndSample::FFT_Standard(   n,   xr, xi   );    



									 //   Use the guaranteed zero xi[0] to actually return xr[n]
   t =         xr[0];  
   xr[0] =   t + xi[0];
   xi[0] =    t - xi[0];  			// temp use of 't'
   
   							           


//   theta =  PI    /   (double)n;		 //  Now do the remainder through n-1
	theta  =   PI    /   (float)n;


   tp =      sin( 0.5 * theta );
   

   wr =             1.0    +    (   wkr =  -2.0 * tp * tp   );
   wi =  wki =    sin( theta );   




   for(   i = 1;     i <  n/2;      i++   ) 
   {

      j =   n - i;
      
      h1r =      ( xr[i]  +  xr[j] )   *  0.5;	     //  values as big as input
      h1i =      ( xi[i]   -  xi[j] )    *  0.5;	   
      h2r =      ( xi[i]  +  xi[j] )   *  0.5;	
      h2i =  -(  ( xr[i]  -  xr[j] )   *  0.5  );	   
      
      
      xr[i] =   (wr  *  h2r)   -   (wi * h2i)    +  h1r;   //  vals as big as input
      xi[i] =    (wr * h2i)     +   (wi * h2r)   +  h1i;   

      xr[j] =  -(wr * h2r)    +   (wi * h2i)    +  h1r;   
      xi[j] =     (wr * h2i)    +   (wi * h2r)    -   h1i;   



      t =  wr;		

      wr  +=     (t * wkr)   -    (wi * wki);	
      wi   +=     (t * wki)   +   (wi * wkr);
   }
}



											////////////////////////////////////////


void     SndSample::FFT_Standard(    int  n,     float  *xr,   float  *xi   )       //  double *xr,   double *xi      )
{

			//  Got this from Book by Timithothy Masters     1/12

   int     i,  bitrev, k,  half_n;       
   int     m,  mmax, step,  other;	
   
// double    tr, ti,   wr, wi,  wkr, wki,   temp; 
	float    tr, ti,   wr, wi,  wkr, wki,   temp; 


   

   //    Not needed now:  (implement in future???   1/2012 )    Init_FFT_Tools_for_FixedPoint_Calcs(   (short)n   );     // check lookup tables
     


   bitrev   =      0;            // Will count in bit-reversed order
   half_n  =     n/2;


   for(   i= 1;    i < n;    i++   )       //  Do every element( BIT-REVERSE )
   {
    
       ++bitrev;			           //  Increment bit-reversed counter
       k =   half_n;

       while(   bitrev  >  k   )  
	   {   
		   bitrev -= k;   
		   k >>= 1;  
	   }


       bitrev  +=   k -1;


       if(  i < bitrev  )   //  Swap straight-counter element with bit-reversed element (just once!)
       { 
		   temp         =   xr[ i ];
           xr[ i ]        =   xr[ bitrev ];
           xr[ bitrev ] =   temp;

           temp         =   xi[ i ];
           xi[i]           =   xi[ bitrev ];
           xi[ bitrev ] =   temp;
       }
    }



  								
   for(    mmax= 1;     mmax < n;     mmax =  step    )          // ** butterflies
   {

       step =    2 * mmax;			           
       
       wr  =   1.0;     //    1073741824L;   
	   wi  =   0.0;     // wr= 1.0;   wi= 0.0;   
       


	   double   theta =    PI  /  (double)mmax;

	   tr      =    sin(  0.5  *   theta   );

	   wkr  =    -2.0   *  tr  *  tr;
	   wki  =     sin(  theta  );



       for(   m= 0;     m< mmax;     m++   ) 
       { 
		   
		   for(    i= m;     i < n;       i  +=  step     ) 
           {
               other =    i  +  mmax;
            			    
               tr =      wr  *  xr[other]     -     wi  *  xi[other];   
               ti =      wr  *   xi[other]     +    wi  *  xr[other];  
            
               xr[ other ] =  xr[i] - tr;     
               xi[ other ] =  xi[i] - ti;   
			   
               xr[ i ] +=  tr;				  
               xi[ i ] +=  ti;				   
           }



           tr =  wr;
		   wr  +=    (  tr  * wkr )    -     (  wi  *  wki  );	
           wi  +=    (  tr  * wki  )   +     (  wi  *  wkr  ); 
       }
   }
}




											////////////////////////////////////////


void     SndSample::FFT_FixedPoint_Standard(    int  n,     long *xr,   long *xi      )
{


   int       i,  bitrev, k,  half_n;       
   int      m,  mmax, step,  other;		 
   long    tr, ti,   wr, wi,  wkr, wki,  temp;   // WAS double
   
   

// ************************************   INSTALL   Init_FFT_Tools_for_FixedPoint_Calcs   if want to seriously use this.   1/2012 ************************

//   FFTslowDown::Init_FFT_Tools_for_FixedPoint_Calcs(   (short)n   );     // check lookup tables   ***********   MAYBE not need ***************
     

// ************************************   INSTALL   Init_FFT_Tools_for_FixedPoint_Calcs   if want to seriously use this.   1/2012 ************************



   bitrev   =      0;            // Will count in bit-reversed order
   half_n  =    n/2;



   for(   i = 1;     i< n;    i++   )       //  Do every element( BIT-REVERSE )
   {
    

  //  Get_doubles_MinMax(   (double)(xr[i])/SPFRACFACT   );   // *** TEMP ****
  //  Get_doubles_MinMax(   (double)(xi[i])/SPFRACFACT   );   // *** TEMP ****

  //  vals  .0155  to -.0156
    
       ++bitrev;			           //  Increment bit-reversed counter
       k =   half_n;

       while(   bitrev  >  k   )  
	   {   
		   bitrev -= k;   
		   k >>= 1;  
	   }


       bitrev  +=   k -1;


       if(  i < bitrev  )   //  Swap straight-counter element with bit-reversed element (just once!)
       { 
		   temp         =  xr[ i ];
           xr[ i ]      =  xr[ bitrev ];
           xr[ bitrev ] =  temp;

           temp         =  xi[ i ];
           xi[i]        =  xi[ bitrev ];
           xi[ bitrev ] = temp;
       }
    }


  								
   for(    mmax= 1;     mmax < n;     mmax =  step    )         // ** butterflies **
   {

       step =    2 * mmax;			           
       
       wr  =   1073741824L;   //   wr =   1.0
	   wi  =   0L;                   //   wi  =   0.0  
       


	   /***********************************  This code segment is from    Init_FFT_Tools_for_FixedPoint_Calcs
		 for(  i=0;  i< n;  i++  )
		 {
			 theta =     PI  /  (double)i;

			 tr =         sin( 0.5 * theta );

										  // wkr=  -2.0 * tr * tr;   wki= sin ( theta );

			 wkrINI[ i ] =    (long)(    (-2.0 * tr * tr)   * 1073741824.0    ); 

			 wkiINI[ i ] =    (long)(       sin ( theta )    * 1073741824.0    );
		 }
	   ****/

		double  theta =     PI  /  (double)mmax;

		double  trLoc =     sin( 0.5 * theta );



 //   wkr  =     wkrINI[ mmax ];     
	   wkr  =     (long)(    ( -2.0 * trLoc * trLoc )   * 1073741824.0    ); 



//	   wki  =     wkiINI[ mmax ];   // from fast lookup-tables  

	   wki	 =     (long)(     sin ( theta )    * 1073741824.0    );





       for(  m= 0;   m< mmax;   m++  ) 
       { 
		   
		   for(    i= m;    i < n;     i += step     ) 
           {
               other =  i + mmax;
            			    



            			    // *****[ 18 to -20 ]  BIGGEST vals ???? BOTTLENECK or at end????
              
               tr =   FracMulJM(   wr, xr[other] )   -    FracMulJM( wi, xi[other]    ); 
               ti =   FracMulJM(   wr, xi[other] )    +   FracMulJM( wi, xr[other]   );  

      
    //  Get_doubles_MinMax(   (double)tr /SPFRACFACT   );   // *** TEMP ****
    //  Get_doubles_MinMax(   (double)ti /SPFRACFACT   );   // *** TEMP ****
 

            
               xr[ other ] =  xr[i] - tr;     
               xi[ other ] =  xi[i] - ti;     

               xr[ i ] +=  tr;				  
               xi[ i ] +=  ti;				   
           }

           tr =  wr;

           wr  +=   FracMulJM(tr, wkr)   -   FracMulJM(wi, wki);		 // -1 to 1

           wi   +=   FracMulJM(tr, wki)   +   FracMulJM(wi, wkr);     
       }
   }


 //  ***** OR...  is BOTTLENECK here... at end ????***************************
  

  /*****************************
   for(  i=0;  i< n;  i++  )     // Do every element( BIT-REVERSE )
     {
    
      Get_doubles_MinMax(   (double)(xr[ i ])/SPFRACFACT   );   // *** TEMP ****
      Get_doubles_MinMax(   (double)(xi[ i ])/SPFRACFACT   );   // *** TEMP ****

             // vals:   ?????  
     }  
  ******************************/
}



											////////////////////////////////////////


void   SndSample::Init_FFT_Tools_for_FixedPoint_Calcs(  short  n  )
{

     short    i;
     double  theta,  tr;
     


 //    if(  n  ==   (short)curFFTlen    )       ******************  INSTALL soem kind of test  **********************
//		 return;                   //   OK,   already done
 


     for(  i=0;  i< n;  i++  )
     {
         theta =     PI  /  (double)i;

         tr =         sin( 0.5 * theta );

                                      // wkr=  -2.0 * tr * tr;   wki= sin ( theta );

		ASSERT( 0 );
		 /*****   RE INSTALL if I want to use this function **************************************************

         wkrINI[ i ] =    (long)(    (-2.0 * tr * tr)   * 1073741824.0    ); 
         wkiINI[ i ] =    (long)(       sin ( theta )    * 1073741824.0    );
		****/
     }


 //    curFFTlen =   (long)n; 


 /***************************  OUT!!  see    'Make_Standard_DataWindow()'   
     winSize =  (short)chunkSize;  // ******* BAD, fixed at  'chunkSize'  *******
    
     for(  i=0;  i< winSize;  i++  )   // see T. Masters(pp 93), Image Process with neural networks
        {
           tp= ( (double)i  - 0.5*((double)winSize - 1.0)  ) / ( 0.5*((double)winSize + 1.0) );
       
           Dwindow[ i ]= (long)(  ( 1.0 - pow( tp, 2.0 )   ) * 1073741824.0 ); // data windowing function
        }
   *******************/ 
}








