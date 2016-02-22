/////////////////////////////////////////////////////////////////////////////
//
//  sndSample.h   -   my 'SOUND SAMPLE' 
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined( _SNDSAMPLE_H_  )

#define _SNDSAMPLE_H_

/////////////////////////////////////////////////////////////////////



// ****  FIX:  get rid of this HARDWIRED value *********************  JPM ****************************

#define   DEFsAMPLERATE   44100L       //   Mac:   11127,   PC: 44100     




class  TransformMap;




/////////////////////////////////////////////////////////////////////

typedef  char**  devSAMPLE;   // 'Handle' on Mac, is 'SoundSample' with HEADER( 42 bytes )




/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

class    SndSample                     
{ 	
	
public:  
	  SndSample(   long  numSamps,    long  chkSize   ); 
      ~SndSample();              




	virtual	 bool			Is_Stereo_Sample();  


	virtual	 bool			Make_Stereo(  CString&  retErrorMesg  );
	virtual	 bool			Make_Mono(    CString&  retErrorMesg  );   //  turn a stereo SndSample in to mono by discarding the LEFT channel  


	virtual	 bool			Copy_LeftStereo_To_Right(  CString&  retErrorMesg   );



	virtual   bool		    CopyBits_From_16bit_Sample(   short  channelCode,  char *srcByte,    long  sampOffset,    long  totalSamples,   CString&   retErrorMesg  );




	virtual	 void		    Reverse_Byte_Order(  long  validSampleCount   );    //  used for note-detection  BACKWARDS  play


	static     void		    Reverse_16bit_Sample_Order(     BYTE *srcByte,     long  byteCount   );    //  nice NEW STATIC funct,  use it   12/11

	static     void         Reverse_16bit_Samples_On_Copy(  BYTE *src,     BYTE  *dst,     long  numberOfBytes   );   //   new, from   StreamingAudioplayer  12/11





	static  	bool      Make_Float_Sample(  short channelCode,  char  *srcBytes,   long  totalByteCount,  long  bytesPerSample,
									                                                       float  *dstSamplesPtr,   bool  lessThanOneValues,    CString&  retErrorMesg    );

	static  	bool	  Multiply_Float_Samples(   long  sampleCount,    float  *sampleArrayPtr,    float  multFactor,    CString&   retErrorMesg  );   // use to convert from 'lessThanOneValues'


	static  	bool	  Merge_Float_Channels_To_Sample(  bool  toStereo,    long  sampleCount,   long  bytesPerSample,   
																	                            float **leftSampleArray,   float **rightSampleArray,  
																char **retDstSamplesPtr,   long& retByteCount,   bool  lessThanOneValues,   
																				bool noMemoryRelease,     double  volumeTweak,   CString&  retErrorMesg   );


				 char*       Get_StartByte();  
				 char*       Get_StartByte_RightStereo();



	 virtual   SndSample*   Clone();

	 virtual   SndSample*   Clone_DownSampled(   long  downRatio   );    


     virtual   void         Erase();


				 long        Get_Length();
				 long        Get_Sample_Count()   {   return   m_totalSamps;   }    //  these two are the same,  but get length is not describptive name




     virtual   bool		  Make_WAV_File(   CString&  destFilePath,      CString&  retErrorMesg   );   //   NEW,  5/07




																//  The latest  ( 1/2012 )  version of FFT  functions
													

	static    void		    FFT_Standard(               int  n,     float  *xr,   float  *xi   );    //  double *xr,   double *xi )   from Book by Timithothy Masters     1/12

	static    void			FFT_FixedPoint_Standard(    int  n,     long *xr,   long *xi      );


	static    void		    FFT_with_only_Real_Input(   int n,      float  *xr,   float  *xi   );    //  double *xr,   double *xi  )   //  Faster, uses half as many calulation.   from Book by Timithothy Masters     1/12


	static    void			Init_FFT_Tools_for_FixedPoint_Calcs(  short  n  );    //  Unecessary,  but could be implemented for optimization with FixedPoint   1/12




				bool            Apply_Sound_Filter(    int   filterType,     long  parm1,    long  parm2,   	long&  retValue,   CString&  retErrorMesg   );

			   bool             Apply_VolumeAdjust_Filtering(    long&   retScalePercentVolAdjust,	  CString&  retErrorMesg   );



				bool			Apply_Full_Filtering(    long  topFrequencyLimit,   long  bottomFrequencyLimit,   long&   retScalePercent,  	CString&  retErrorMesg   );

				bool			Apply_Full_Filtering_FixedScaling(    long  topFrequencyLimit,   long  bottomFrequencyLimit,       long  inputScalePercent,  
																		                              double&  retOverbriteRatio,		CString&  retErrorMesg   );


     virtual   bool		       Create_Flanged_Sample(  long  delayInSamples,    CString&  retErrorMesg   );   //  try to delay by 1 or 2 bars   



			    double         Calc_AutoCorrelation_ByLag(   long  numSampsInLag,   char* srcStart,     long  offsetIntoSample,
																				           long samplesInWindow,      long&  retZeroCrossingCount   );


              
public:        
      TransformMap   *_ChunkMap;     
      

      long        _chunkSize;                   // HERE or in  'Editor' ???? 

      long        _sampleRate; 



	  char    *m_bits;         // new for PC version,   is SIGNED,   on the Mac it was uchar 

	  char    *m_bitsRightStereo;  



	  long     m_totalSamps;

	  bool     m_showProgressBar;   //  not yet installed to all subclasses   5/07



public:
	enum   stereoChannels {   LEFT,    RIGHT,    BOTH    };		// *** NEW,  use them
};




			////////////////////////////////////////////////////////////////////////////


typedef struct 
{    	
	 //   ****  Is it used anymore?????    9/06  *****

	DWORD    dWord0;	  	
	DWORD    dWord1;	//	  totalSampleBytes  +  36	  [   3047460   ]	       Typically:  filesize - 8	   
	DWORD    dWord2;	  	
	DWORD    dWord3;		
	DWORD    dWord4;		
	DWORD    dWord5;	  	
	DWORD    dWord6;	 	
	DWORD    dWord7;	  	
	DWORD    dWord8;	  	
	DWORD    dWord9;	  	
	DWORD    dWord10;	  //   totalSampleBytes			 [   3047424  =    761856  *  4  ]            


}  JimboWAVfileHeader;    //   ****  Is it used anymore?????    9/2006  *****



////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#endif   // __H_

