/////////////////////////////////////////////////////////////////////////////
//
//  sndFilters.cpp   -   Sound FILTERS
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



///////////////////////////////

#include   "..\comnFacade\UniEditorAppsGlobals.h"

#include   "..\comnFacade\VoxAppsGlobals.h"



//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\ComnGrafix\CommonGrafix.h"    
#include  "..\comnFoundation\myMath.h"


#include  "..\comnGrafix\OffMap.h"  
#include  "..\comnGrafix\TransformMap.h"
///////////////////////////////


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






long   filtFixCoeffs[ 300 ],  filtHistory[ 300 ];   // 300 enough????  to receive 'iir_hpf6[]' in FIX

double     fir_custCoeffs[ 300 ];

long      gbFXmax,  gbFXmin;





double   iir_BandPass_Low1[5] =
  {  0.18,       //   0.0914007276,  ... boost volume
     -1.7119922638,   0.8171985149,   0.0,  -1.0  };

double   iir_BandPass_Mid1[5] =
  {  0.6,               //   0.1845672876,  ... boost volume
     -1.0703823566,   0.6308654547,   0.0,  -1.0  };

double   iir_BandPass_Hi1[5] =
  {  1.5,      //   0.3760778010,  ... boost volume
     0.6695288420,   0.2478443682,   0.0,  -1.0   };





/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// IIR highpass:  3 section (6th order) chebyshev filter with 1 dB passband ripple and cutoff frequency of 0.3*fs. 

double   iir_hpf6[  13  ] = 
    {
        0.04,     //  0.0025892381,    [ overall  'scale factor'  for the filter ]
        
        0.5913599133,  0.8879900575,  -2.0000000000,  1.0000000000,
        0.9156184793,  0.6796731949,  -2.0000000000,  1.0000000000,
        1.3316441774,  0.5193183422,  -2.0000000000,  1.0000000000
     };


/////////////////////////////////////////////////////////////////////////////////////////
// IIR lowpass:   3 section (5th order) elliptic filter with 0.28 dB passband ripple 
//                and 40 dB stopband attenuation. The cutoff frequency is 0.25*fs. 

double   iir_lpf5[  13  ] = 
       {
          0.0552961603,   //  0.0552961603,    [ overall  'scale factor'  for the filter ]
         
         -0.4363630712,  0.0000000000,  1.0000000000,  0.0000000000,
         -0.5233039260,  0.8604439497,  0.7039934993,  1.0000000000,
         -0.6965782046,  0.4860509932, -0.0103216320,  1.0000000000
        };


/////////////////////////////////////////////////////////////////////////////////////////
//           ...bandpass filter coefficients for rates: 
//
// at 44.1 kHz:      60,   150,   400,  1000,  2400,  6000,  15000 Hz 
//
// at 32 kHz:        44,   109,   290,   726,  1742,  4354,  10884 Hz 
// at 22.1 kHz:      30,    75,   200,   500,  1200,  3000,   7500 Hz 
// at 11.0 kHz ???   15,    37,   100,   250,     600,   1500,  3750 Hz 

double   bpf[ 7 ][ 5 ] = 
{
  {  0.0025579741,  
          -1.9948111773,   0.9948840737,   0.0,  -1.0 },
 
  {  0.0063700872,  
          -1.9868060350,   0.9872598052,   0.0,  -1.0 },
 
  {  0.0168007612,  
          -1.9632060528,   0.9663984776,   0.0,  -1.0 },
  
  {  0.0408578217,  
          -1.8988473415,   0.9182843566,   0.0,  -1.0 },
  
  {  0.0914007276,  
          -1.7119922638,   0.8171985149,   0.0,  -1.0 },
 
  {  0.1845672876,  
          -1.0703823566,   0.6308654547,   0.0,  -1.0 },
  
  {  0.3760778010,   
          0.6695288420,   0.2478443682,   0.0,  -1.0 },
};


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
//  Parks-McClellan:  35 point lowpass FIR filter cutoff at 0.19

double   fir_lpf35[  35  ] = 
 {
  -6.3600959e-03,  -7.6626200e-05,   7.6912856e-03,   5.0564148e-03,  -8.3598122e-03,
  -1.0400905e-02,   8.6960020e-03,   2.0170502e-02,  -2.7560785e-03,  -3.0034777e-02,
  -8.9075034e-03,   4.1715767e-02,   3.4108155e-02,  -5.0732918e-02,  -8.6097546e-02,
   5.7914939e-02,   3.1170085e-01,   4.4029310e-01,   3.1170085e-01,   5.7914939e-02,
  -8.6097546e-02,  -5.0732918e-02,   3.4108155e-02,   4.1715767e-02,  -8.9075034e-03,
  -3.0034777e-02,  -2.7560785e-03,   2.0170502e-02,   8.6960020e-03,  -1.0400905e-02,
  -8.3598122e-03,   5.0564148e-03,   7.6912856e-03,  -7.6626200e-05,  -6.3600959e-03
  };

/////////////////////////////////////////////////////////////////////////////////////////
// KSRFIR.C   37 point lowpass FIR filter cutoff at 0.19,  designed using the KSRFIR.C program 

double  fir_lpf37[  37  ] = 
 {
  -6.51000e-04,  -3.69500e-03,  -6.28000e-04,   6.25500e-03,   4.06300e-03,
  -8.18900e-03,  -1.01860e-02,   7.84700e-03,   1.89680e-02,  -3.05100e-03,
  -2.96620e-02,  -9.06500e-03,   4.08590e-02,   3.34840e-02,  -5.07550e-02,
  -8.61070e-02,   5.75690e-02,   3.11305e-01,   4.40000e-01,   3.11305e-01,
   5.75690e-02,  -8.61070e-02,  -5.07550e-02,   3.34840e-02,   4.08590e-02,
  -9.06500e-03,  -2.96620e-02,  -3.05100e-03,   1.89680e-02,   7.84700e-03,
  -1.01860e-02,  -8.18900e-03,   4.06300e-03,   6.25500e-03,  -6.28000e-04,
  -3.69500e-03,  -6.51000e-04
 };



/////////////////////////////////////////////////////////////////////////
 
long   Calc_Fir_BandPass_Coeffs(   double  lowEdge,   double  hiEdge,  
                                                             double  transWidth,    double att,   short filt_cat  );




/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////


SndFilter::SndFilter(   SndSample  *samp   )     :   _Samp( samp )   
{

	m_showProgressBar  =    false;
}


								//////////////////////////////


SndFilter::~SndFilter()   
{  
}     



								//////////////////////////////


long   SndFilter::Get_Average_Amplitude()
{

	// *** Have  to worry about overflow ????     ***INSTALL test for overflow****

	char   *src;         
	 
//	long   i, chk, numByts, lgVal, numer, totAmpl,   totAmplSqrd,  maxVal,  numChunks,  chunkSize;     
	long   retAvgValue= -1,    numByts,   numChunks,  chunkSize,  sum=0,   lgVal;     


	if(    _Samp   ==   NULL    )              
	{
		 ASSERT( 0 );
		 return  -2;
	}
    

	numByts     =      _Samp->Get_Length();

	if(   numByts == 0L )   
	{
		//  retErrorMesg  =  "IIRsecondSndFilter::Filter failed,  length is 0." ;
		return  -2;
	}




	chunkSize   =      _Samp->_chunkSize;
	numChunks =     numByts / chunkSize;
	src              =     _Samp->Get_StartByte();   



	for(   long  i = 0L;      i<  numByts;       i++    )   
	{ 

		lgVal =      (long)(  *src  );
            
		sum  +=    absj(   lgVal  );
 
		src++; 
	}



	retAvgValue  =      sum   /   numByts;    //  SHOULD it be 2x this value cause 


	return   retAvgValue;
}




								//////////////////////////////


long   SndFilter::Get_Maximum_AverageAmplitude_of_ChunkGroups(   long  adjChunkSize   )
{


	// *** Have  to worry about overflow ????     ***INSTALL test for overflow****


	char   *src;         
	 
//	long   i, chk, numByts, lgVal, numer, totAmpl,   totAmplSqrd,  maxVal,  numChunks,  chunkSize;     
	long    numByts,   numChunks,   lgVal;     


	if(    _Samp   ==   NULL    )              
	{
		 ASSERT( 0 );
		 return  -2;
	}
    


	numByts      =      _Samp->Get_Length();
	src              =      _Samp->Get_StartByte();   

	numChunks =      numByts / adjChunkSize;





	long   maxAvgAmpl =   -1;		


	for(    long  chk =0L;       chk <  numChunks;       chk++    )   
	{


		long   totAmpl = 0L,    avgAmpl;                           


										//  Calc the AcverageAmplitude for the  Chunk

		for(    long i =0;       i <  adjChunkSize;       i++   )
		{ 

			lgVal   =     (long)(  *src  );       //   make it a SIGNED quantity     vals:  {  -127  -  +128  }
           
            totAmpl  +=      absj(  lgVal  );					 //   SUM all Amplitudes ( absolute value )

			src++; 
		} 
          
		avgAmpl  =   	totAmpl   /  adjChunkSize;     



		if(    avgAmpl   >   maxAvgAmpl    )      //    Save the MAX-average
			maxAvgAmpl =    avgAmpl;
	}



	return   maxAvgAmpl;
}




										////////////////////////////////////////


void    SndFilter::Get_Amplitude_Populations(   long   population[],   long  bucketCount   )
{



	if(    _Samp   ==   NULL    )              
	{
		 ASSERT( 0 );
		 return;
	}
    


	char   *src;         	 
	long     numByts,     lgVal,   mag;     


	src              =     _Samp->Get_StartByte();   
	numByts     =      _Samp->Get_Length();
//	chunkSize   =      _Samp->_chunkSize;
//	numChunks =     numByts / chunkSize;



	 for(   long  i =0;     i < bucketCount;      i++    )
		  population[  i  ] =  0;




	for(   long  i = 0L;      i<  numByts;       i++    )   
	{ 

		lgVal =      (long)(  *src  );

		mag  =      absj(  lgVal  );
            

		if(    mag   >  128   )			  
			ASSERT( 0 );
		else
			population[  mag   ]++;

 
		src++; 
	}

		//  want to SORT the scores ...why ???    MEDIAN....
}




										////////////////////////////////////////


long     SndFilter::Get_Top_Value(    long  discardPercent,     long  population[],     long  bucketCount,    
																																long&  retAbsoluteTop    )
{


		//  *** CAREFUL:     'discardPercent'    in TENTHS of Percent !!!


		//  RETURNS:   0 [  ERROR in algo  ]



	retAbsoluteTop  =   -1;  


	long   numByts     =      _Samp->Get_Length();


	long   discardHighCount  =      (  numByts       *   discardPercent  )   /  1000;     //  the number of PEAK sample-amplitudes to ignore.




	long  topVal =  -1,    cntHighScores=0;

	long      i           =       ( bucketCount  -1 );    //  set to LAST member in array
	retAbsoluteTop  =   i;



	while(    topVal  < 0   )	    //   Add scores from top till the  'DISCARDABLE Percent'  of high scores is found
	{

		cntHighScores  +=      population[  i  ];

		if(   cntHighScores   >   discardHighCount   )
			topVal =   i;     //  this will EXIT loop



		if(     population[  i  ]   <=   0    )
			retAbsoluteTop  =    i;
		else
		{
			int  dummy =  9;   // **** OMIT,  debug 
		}



		i--;

		if(    i  <= 0    )
		{
			                        //  Got here for DoWhatLike(end, low signal ) detectzone at end of file 
			topVal =   0;     //  this should cause  NO change in later amplitude calc. YES,  7/06
		}
	}


	return   topVal;
}



										////////////////////////////////////////


bool    IIRsecondSndFilter::Filter(    CString&  retErrorMesg   )         
{                                     

		//      VolumeAutoAdjFilter  is NOT necessary after this is done....                  


														//    'FLOATING'-Point  version	   3/04	


				//    'IIR'  filter (   2nd order   )     [  ....Dan Ellis( Correlogram )   uses an   '8th Order'    IIR filter  ]
							
	
	retErrorMesg.Empty();

	if(    _Samp  ==   NULL    )    
	{
		ASSERT( 0 );
		retErrorMesg  =  "IIRsecondSndFilter::Filter failed,  sample is NULL." ;
		return  false;
	}

    long   numByts  =      _Samp->Get_Length();
 	if(     numByts == 0L )   
	{
		retErrorMesg  =  "IIRsecondSndFilter::Filter failed,  length is 0." ;
		return  false;
	}


     SndSample  *cloneSample  =       _Samp->Clone();    	 
     if(    cloneSample  ==   NULL   )   
	 {
		retErrorMesg  =  "IIRsecondSndFilter::Filter failed,  Clone is NULL." ;
		return  false;
	 }

	bool   doStereo  =   _Samp->Is_Stereo_Sample();   // *************   NEW,   ***************************




	double   upperPercentThresh  =     0.50;     //   in percent   ...1 is one percent     *****  ADJUST *****

	double   lowerPercentThresh  =     0.10;      //    *****  ADJUST *****

	long        maxIters   =  30;    //  ***ADJUST***




	double    volumeAdjust        =	    1.0;		//  init
	double    volumeAdjustLast   =	 1.0;	


	/***
	long       upperThresh  =      (long)(       (double)numByts       *    0.05      );			//   .04 :    PERCENT of  'Clipped-Samples'				
	long	     lowerThresh   =      (long)(       (double)numByts       *    0.01      );
	***/
	long     upperThresh  =       (long)(       (double)numByts       *     ( upperPercentThresh  /100.0 )      );			//      PERCENT of  'Clipped-Samples'				
	long	   lowerThresh   =      (long)(        (double)numByts       *     ( lowerPercentThresh  /100.0 )      );



	short    overbrightThreshold  =   90;       //  80 was a little weak      [ 80 -  125 ]    ...determines the overall loudness    *****  ADJUST *****





	 
     char     *src, *dst;          				      
     long      outVal,  i,  overBriteCnt =0,   iter= 0;
	 long      biggestOutVal  =   -1;
     double   q,   oo,   nh,  beta,  alpha,   gamma,     X,  X1,  X2,     Y,  Y1,  Y2;   
     short     tp,   freqEdgeLow,   freqEdgeHi,     centFreq;



     gbFXmax  =    0;      
	 gbFXmin   =    16555444;    // init
     
    
     if(           _EdgeFreq0    ==    _EdgeFreq1  
          ||  (   _EdgeFreq0 == 0     &&    _EdgeFreq1 == 0   )     )     
	 {
		 ASSERT( 0 );
		 retErrorMesg  =  "IIRsecondSndFilter::Filter failed,   EdgeFreq has bad value." ;
		 return  false;
	 }
     
     
     freqEdgeLow =    _EdgeFreq0;    
	 freqEdgeHi    =    _EdgeFreq1;       
     
     if(     freqEdgeLow   >    freqEdgeHi     )     // get order right
	 {  
		 tp                  =   freqEdgeLow;    
		 freqEdgeLow  =   freqEdgeHi;    
		 freqEdgeHi    =   tp;  
	 }     
        



     centFreq  =     freqEdgeLow    +      (  freqEdgeHi  -  freqEdgeLow  ) / 2;       //   is centered between range
          
    q    =      (double)centFreq   /   (double)( freqEdgeHi - freqEdgeLow );      //  Bandwidth

    oo  =     twoPI   *      (    centFreq   /   (double)( _Samp->_sampleRate)    );  // get angular freq

    nh  =     oo   /   ( 2.0  *  q );
     
	 
    beta      =   (( 1.0 - tan(nh) )   /   ( 1.0 + tan(nh) ))  / 2.0;  // up to 1.0
    gamma =     ( 0.5  +  beta )   *   cos(oo);                     // up to 0.5        
    alpha    =      ( 0.5  -  beta )   /   2.0;                         // up to 0.05 ???


    X1 = X2 =  Y2 =     0.0;    // init  ( OPTIONAL )

	Y1 =  0.0;    // init    ****  NEW,  4/06  ....OK ?????





	bool   needsReducing =   true;   
	bool   isDone =   false;
	bool   finalLap =  false;


	while(    ! isDone     )
	{

		//////////////////////
		src    =		cloneSample->Get_StartByte();       //   NOTE:  clone is now the SOURCE, write to  '_Samp' 
		dst    =		_Samp->Get_StartByte();

		if(   src  ==  NULL  ||   dst  ==  NULL   )
		{
			retErrorMesg  =  "IIRsecondSndFilter::Filter failed,  one of LEFT or MONO pointers is NULL" ;
			return  false;
		}

		overBriteCnt    =   0;
		biggestOutVal  =   -1;



		for(    i =0;      i <  numByts;     i++    )   
		{ 

 			long    byteVal =     (long)( *src );

	//		X   =		(double)(     (long)( *src )  - 128L    )    /  128.0;			//  get in {1.0 to -1.0}  ...from MACINTOSH
			X   =		 (double)(   byteVal  )     /  128.0;

			Y   =		2.0  *   (  alpha * (X - X2)    +    gamma * Y1    -    beta * Y2  );
            
			X2 =  X1;    X1 = X;         Y2 = Y1;    Y1 = Y;         



			Y  =     volumeAdjust   *   Y;			           //    amplify volume           

			outVal  =     (long)(    Y   *  128.0   );                  //  back to  { 128.0 to -128.0 }
        



			if(     absj( outVal )   >    biggestOutVal     )
				biggestOutVal  =     absj( outVal );


			if(         outVal  >    overbrightThreshold  )   
				overBriteCnt++;
			else if(  outVal  <  ( overbrightThreshold * -1)   )  
				overBriteCnt++;



			if(         outVal  >   127L  )   
				*dst =     127;	
			else if(  outVal  <  -127L  )  
				*dst =    -127;	
			else                                
				*dst =   (char)outVal;
			   

			src++;    dst++;     
		}
		//////////////////////




		if(    doStereo   )
		{
			//////////////////////
			src    =		cloneSample->Get_StartByte_RightStereo();       //   NOTE:  clone is now the SOURCE, write to  '_Samp' 
			dst    =		_Samp->Get_StartByte_RightStereo();

			if(   src  ==  NULL  ||   dst  ==  NULL   )
			{
				retErrorMesg  =  "IIRsecondSndFilter::Filter failed,  one of RIGHT stereo pointers is NULL" ;
				return  false;
			}


			overBriteCnt    =   0;
			biggestOutVal  =   -1;



			for(    i =0;      i <  numByts;     i++    )   
			{ 
 				long    byteVal =     (long)( *src );

		//		X   =		(double)(     (long)( *src )  - 128L    )    /  128.0;			//  get in {1.0 to -1.0}  ...from MACINTOSH
				X   =		 (double)(   byteVal  )     /  128.0;

				Y   =		2.0  *   (  alpha * (X - X2)    +    gamma * Y1    -    beta * Y2  );
	            
				X2 =  X1;    X1 = X;         Y2 = Y1;    Y1 = Y;         

				Y  =     volumeAdjust   *   Y;			           //    amplify volume           

				outVal  =     (long)(    Y   *  128.0   );                  //  back to  { 128.0 to -128.0 }
	        

				/***
				if(     absj( outVal )   >    biggestOutVal     )
					biggestOutVal  =     absj( outVal );


				if(         outVal  >   127L  )   
				{
					*dst =     127;	overBriteCnt++;
				}
				else if(  outVal  <  -127L  )  
				{
					*dst =    -127;		overBriteCnt++;
				}
				else                                
				*dst =   (char)outVal;
				***/
				if(     absj( outVal )   >    biggestOutVal     )
					biggestOutVal  =     absj( outVal );


				if(         outVal  >    overbrightThreshold  )   
					overBriteCnt++;
				else if(  outVal  <  ( overbrightThreshold * -1)   )  
					overBriteCnt++;


				if(         outVal  >   127L  )   
					*dst =     127;	
				else if(  outVal  <  -127L  )  
					*dst =    -127;	
				else                                
					*dst =   (char)outVal;

				   
				src++;   		dst++;     
			}
			//////////////////////

		}   //  if(    doStereo 






		/////////////////////   check volume and figure out if need to do this again to get the best volume

		iter++;

		if(           (  overBriteCnt    <=     upperThresh    &&    overBriteCnt    >=     lowerThresh    ) 
			       ||	 finalLap ==  true 	 )  
			   isDone =    true;
		else
		{  
			if(           overBriteCnt   >    upperThresh     )
			{
				if(    iter  ==  1   )
					needsReducing =    true;									   	//   must   'DECREASE'   the volume
				else
				{	if(    ! needsReducing    )
					{
						//  isDone =    true;			//   trying to REVERSE DIRECTIONS,  so stop and live with last iter
						finalLap          =    true;    //  let it do it one more time at the previous amplification. The RightStereo sample
					}
				}
				


				if(   finalLap   )
					volumeAdjust       =    volumeAdjustLast;
				else
					volumeAdjustLast =     volumeAdjust;     //   save for final lap


				volumeAdjust       =      volumeAdjust   *   0.80;
			}
			else if(    overBriteCnt   <    lowerThresh      )
			{

				if(    iter  ==  1   )
					needsReducing =     false;										//   must   'INCREASE'   the volume
				else
				{	if(    needsReducing    )
					{
						//  isDone =    true;			//   trying to REVERSE DIRECTIONS,  so stop and live with last iter
						finalLap          =    true;    //  let it do it one more time at the previous amplification. The RightStereo sample
					}	
				}


				if(   finalLap   )
					volumeAdjust       =    volumeAdjustLast;
				else
					volumeAdjustLast =     volumeAdjust;     //   save for final lap


				volumeAdjust      =      volumeAdjust   *   1.25;
			}
		}




		if(    iter  ==   maxIters    )   //  MAX  iters
		{

		//	ASSERT( 0 );	   		// ***** ERROR:   hit max iters    Hit here when too much silence at start of track. No big deal.   1/10

		//	AfxMessageBox(  "IIRsecondSndFilter failed( MAX iterations ).\nThe resultant Wave sample may be empty or poorly calculated."   );   
			TRACE(  "IIRsecondSndFilter failed( MAX iterations ).\nThe resultant Wave sample may be empty or poorly calculated."   );

			
			finalLap          =    true;    //  let it do it one more time at the original amplification. The RightStereo sample

			volumeAdjust =    1.0;    //   originalVolumeAdjust;

			/****
			isDone =    true;
			delete   cloneSample;        cloneSample =  NULL;
			retErrorMesg  =  "IIRsecondSndFilter::Filter failed,   hit MAX iters." ;
			return  false;                 NO...    *** Let it go,  see what happens. Get here if the signal is very weak.  ***
			****/
		}
		else if(   iter  >  maxIters   ) 
			isDone =    true;


	}    //   while(   ! isDone   )



	 m_retScalePercentage      =       (long)(    volumeAdjust    *   100.0    );


//	 TRACE(   "\n FREQUENCY Filter ( IIRsecondSndFilter )     VolumeChange[  %d percent  ]      Iters[  %d  ]  \n",       (long)(  volumeAdjust * 100.0  ),     iter   );

  
     delete   cloneSample;      // *** IMPORTANT,  get rid of  'temp WORKSPACE'


	 return  true;
 }



 											////////////////////////////////////////


bool    IIRsecondSndFilter::Filter_Fixed_Scaling(    long  inputScalePercent,     double&  retOverbriteRatio,	   CString&  retErrorMesg   )         
{                                     

		//      VolumeAutoAdjFilter  is NOT necessary after this is done....                  


														//    'FLOATING'-Point  version	   3/04	


				//    'IIR'  filter (   2nd order   )     [  ....Dan Ellis( Correlogram )   uses an   '8th Order'    IIR filter  ]
							


	char   zeroValueFor8bitSample  =   0;     //   *********Sometimes I use  127,   but think that is WRONG  ...it is from Mackintosh   3/11



	
	retErrorMesg.Empty();

	retOverbriteRatio =   -1.0;



	if(    _Samp  ==   NULL    )    
	{
		ASSERT( 0 );
		retErrorMesg  =  "IIRsecondSndFilter::Filter_Fixed_Scaling failed,  sample is NULL." ;
		return  false;
	}

    long   numByts  =      _Samp->Get_Length();
 	if(     numByts == 0L )   
	{
		retErrorMesg  =  "IIRsecondSndFilter::Filter_Fixed_Scaling failed,  length is 0." ;
		return  false;
	}


     SndSample  *cloneSample  =       _Samp->Clone();    	 
     if(    cloneSample  ==   NULL   )   
	 {
		retErrorMesg  =  "IIRsecondSndFilter::Filter_Fixed_Scaling failed,  Clone is NULL." ;
		return  false;
	 }


	bool   doStereo  =   _Samp->Is_Stereo_Sample();   // *************   NEW,   ***************************



	double   volumeAdjust        =	  (double)inputScalePercent   / 100.0;


	short    overbrightThreshold  =   120;       //  80 was a little weak      [  0 - 127  ]    ...measures the overall loudness    *****  ADJUST *****



	 
     char     *src, *dst;          				      
     long      outVal,  i,  overBriteCnt =0,   iter= 0;
	 long      biggestOutVal  =   -1;
     double   q,   oo,   nh,  beta,  alpha,   gamma,     X,  X1,  X2,     Y,  Y1,  Y2;   
     short     tp,   freqEdgeLow,   freqEdgeHi,     centFreq;



     gbFXmax  =    0;      
	 gbFXmin   =    16555444;    // init
     
    
     if(           _EdgeFreq0    ==    _EdgeFreq1  
          ||  (   _EdgeFreq0 == 0     &&    _EdgeFreq1 == 0   )     )     
	 {
		 ASSERT( 0 );
		 retErrorMesg  =  "IIRsecondSndFilter::Filter_Fixed_Scaling failed,   EdgeFreq has bad value." ;
		 return  false;
	 }
     
     
     freqEdgeLow =    _EdgeFreq0;    
	 freqEdgeHi    =    _EdgeFreq1;       
     
     if(     freqEdgeLow   >    freqEdgeHi     )     // get order right
	 {  
		 tp                  =   freqEdgeLow;    
		 freqEdgeLow  =   freqEdgeHi;    
		 freqEdgeHi    =   tp;  
	 }     
        



     centFreq  =     freqEdgeLow    +      (  freqEdgeHi  -  freqEdgeLow  ) / 2;       //   is centered between range
          
    q    =      (double)centFreq   /   (double)( freqEdgeHi - freqEdgeLow );      //  Bandwidth

    oo  =     twoPI   *      (    centFreq   /   (double)( _Samp->_sampleRate)    );  // get angular freq

    nh  =     oo   /   ( 2.0  *  q );
     
	 
    beta      =   (( 1.0 - tan(nh) )   /   ( 1.0 + tan(nh) ))  / 2.0;  // up to 1.0
    gamma =     ( 0.5  +  beta )   *   cos(oo);                     // up to 0.5        
    alpha    =      ( 0.5  -  beta )   /   2.0;                         // up to 0.05 ???


    X1 = X2 =  Y2 =     0.0;    // init  ( OPTIONAL )

	Y1 =  0.0;    // init    ****  NEW,  4/06  ....OK ?????





//	bool   needsReducing =   true;   
//	bool   isDone =   false;
//	bool   finalLap =  false;


//	while(    ! isDone     )
//	{

		//////////////////////
		src    =		cloneSample->Get_StartByte();       //   NOTE:  clone is now the SOURCE, write to  '_Samp' 
		dst    =		_Samp->Get_StartByte();

		if(   src  ==  NULL  ||   dst  ==  NULL   )
		{
			retErrorMesg  =  "IIRsecondSndFilter::Filter_Fixed_Scaling failed,  one of LEFT or MONO pointers is NULL" ;
			return  false;
		}

		overBriteCnt    =   0;
		biggestOutVal  =   -1;



		for(    i =0;      i <  numByts;     i++    )   
		{ 

 			long    byteVal =     (long)( *src );

	//		X   =		(double)(     (long)( *src )  - 128L    )    /  128.0;			//  get in {1.0 to -1.0}  ...from MACINTOSH
			X   =		 (double)(   byteVal  )     /  128.0;

			Y   =		2.0  *   (  alpha * (X - X2)    +    gamma * Y1    -    beta * Y2  );
            
			X2 =  X1;    X1 = X;         Y2 = Y1;    Y1 = Y;         



			Y  =     volumeAdjust   *   Y;			           //    amplify volume           

			outVal  =     (long)(    Y   *  128.0   );                  //  back to  { 128.0 to -128.0 }
        



			if(     absj( outVal )   >    biggestOutVal     )      // 
				biggestOutVal  =     absj( outVal );


			if(         outVal  >    overbrightThreshold  )   
				overBriteCnt++;
			else if(  outVal  <  ( overbrightThreshold * -1)   )  
				overBriteCnt++;



			if(         outVal  >   127L  )   
				*dst =     127;					
			else if(  outVal  <  -127L  )  
				*dst =    -127;	
			else                                
				*dst =   (char)outVal;
			   

			src++;    dst++;     
		}
		//////////////////////




		if(    doStereo   )
		{
			//////////////////////
			src    =		cloneSample->Get_StartByte_RightStereo();       //   NOTE:  clone is now the SOURCE, write to  '_Samp' 
			dst    =		_Samp->Get_StartByte_RightStereo();

			if(   src  ==  NULL  ||   dst  ==  NULL   )
			{
				retErrorMesg  =  "IIRsecondSndFilter::Filter_Fixed_Scaling failed,  one of RIGHT stereo pointers is NULL" ;
				return  false;
			}


			overBriteCnt    =   0;
			biggestOutVal  =   -1;



			for(    i =0;      i <  numByts;     i++    )   
			{ 
 				long    byteVal =     (long)( *src );

		//		X   =		(double)(     (long)( *src )  - 128L    )    /  128.0;			//  get in {1.0 to -1.0}  ...from MACINTOSH
				X   =		 (double)(   byteVal  )     /  128.0;

				Y   =		2.0  *   (  alpha * (X - X2)    +    gamma * Y1    -    beta * Y2  );
	            
				X2 =  X1;    X1 = X;         Y2 = Y1;    Y1 = Y;         

				Y  =     volumeAdjust   *   Y;			           //    amplify volume           

				outVal  =     (long)(    Y   *  128.0   );                  //  back to  { 128.0 to -128.0 }
	        
				if(     absj( outVal )   >    biggestOutVal     )
					biggestOutVal  =     absj( outVal );


				if(         outVal  >    overbrightThreshold  )   
					overBriteCnt++;
				else if(  outVal  <  ( overbrightThreshold * -1)   )  
					overBriteCnt++;


				if(         outVal  >   127L  )   
					*dst =     127;	
				else if(  outVal  <  -127L  )  
					*dst =    -127;	
				else                                
					*dst =   (char)outVal;

				   
				src++;   		dst++;     
			}
			//////////////////////

		}   //  if(    doStereo 



//	}    //   while(   ! isDone   )



	retOverbriteRatio   =     (double)overBriteCnt   /    (double)numByts;


//	 m_retScalePercentage      =       (long)(    volumeAdjust    *   100.0    );


//	 TRACE(   "\n FREQUENCY Filter ( IIRsecondSndFilter )     VolumeChange[  %d percent  ]      Iters[  %d  ]  \n",       (long)(  volumeAdjust * 100.0  ),     iter   );

  
     delete   cloneSample;      // *** IMPORTANT,  get rid of  'temp WORKSPACE'


	 return  true;
 }





											//////////////////////////////////////////


short    IIRsecondSndFilter::Get_CenterFrequency()
{
	
	short  tp;
    short  freqEdgeLow = _EdgeFreq0;    
	short  freqEdgeHi = _EdgeFreq1;       
     

     if( freqEdgeLow > freqEdgeHi )    
	 {  tp= freqEdgeLow;    freqEdgeLow= freqEdgeHi;    freqEdgeHi= tp;  }     // get order right
        

	short    centFreq  =      freqEdgeLow    +    (   ( freqEdgeHi - freqEdgeLow ) /2   );     //   is centered between range
	return  centFreq;
}




double    IIRsecondSndFilter::Get_Q_Bandwidth()
{

	short  tp;
    short  freqEdgeLow = _EdgeFreq0;    
	short  freqEdgeHi = _EdgeFreq1;       
     

     if( freqEdgeLow > freqEdgeHi )    
	 {  tp= freqEdgeLow;    freqEdgeLow= freqEdgeHi;    freqEdgeHi= tp;  }     // get order right


	short  centFreq  =      freqEdgeLow    +    (   ( freqEdgeHi - freqEdgeLow ) /2   );


	double	q  =    (double)centFreq  /   (double)( freqEdgeHi - freqEdgeLow );      //  Bandwidth
	return   q;
}
	
  



		
		//////////////////////////////////////////
		

bool   MidHighSndFilter::Filter(       CString&  retErrorMesg  )
{
     
	SndSample       *cloneSample;							      //  13 coeffs Mid 
     char   *src, *dst;          				      
     long             centVal,  i,   numByts,      skip= 6L;
    
     
	retErrorMesg.Empty();

	if(    _Samp  ==   NULL    )    
	{
		ASSERT( 0 );
		retErrorMesg  =  "MidHighSndFilter::Filter failed,  sample is NULL." ;
		return  false;
	}



     numByts =  _Samp->Get_Length();
  
	if(   numByts == 0L )   
	{
		retErrorMesg  =  "MidHighSndFilter::Filter failed,  length is 0." ;
		return  false;
	}


     cloneSample =  _Samp->Clone();    
	 
     if(    cloneSample  ==   NULL   )   
	 {
		retErrorMesg  =  "MidHighSndFilter::Filter failed,  Clone is NULL." ;
		return  false;
	 }



 

 //    TRACE( "Starting  MID-filter..." );


     src =     cloneSample->Get_StartByte();   // NOTE:  clone is now the SOURCE, write to  '_Samp' 
     dst =   _Samp->Get_StartByte();
 
     
     /////////////////////////////////////////
 
     *dst =  *src;           src++;  dst++;    // just copy in FIRST
     *dst =  *src;           src++;  dst++;    // just copy in Second
     *dst =  *src;           src++;  dst++;    // just copy in FIRST
     *dst =  *src;           src++;  dst++;    // just copy in Second
     *dst =  *src;           src++;  dst++;    // just copy in FIRST
     *dst =  *src;           src++;  dst++;    // just copy in Second
     
     
     for(  i= skip;   i<  numByts - skip;   i++   )   
         { 
           // centVal  =  144L *   ( (long)(  *src )        );
           ///////////////////////       
           
           centVal  =   12L *   ( (long)(  *src )        ); // little TRIM of highs( little low-pass )

           centVal +=    8L *   ( (long)( *(src -1L) )   );
           centVal +=    8L *   ( (long)( *(src +1L) )   );

           centVal +=    1L *   ( (long)( *(src -2L) )   );
           centVal +=    1L *   ( (long)( *(src +2L) )   );

          
           centVal  =   (centVal* 19L)  / 4L;      // 4.23 : ( boost to '144' )
           /////////////////////////////          
           
           
           centVal -=   24L *   ( (long)(  *src )        ); // subtract VERY-LOWs

           centVal -=   22L *   ( (long)( *(src -1L) )   );
           centVal -=   22L *   ( (long)( *(src +1L) )   );

           centVal -=   17L *   ( (long)( *(src -2L) )   );
           centVal -=   17L *   ( (long)( *(src +2L) )   );

           centVal -=   11L *   ( (long)( *(src -3L) )   );
           centVal -=   11L *   ( (long)( *(src +3L) )   );

           centVal -=    6L *   ( (long)( *(src -4L) )   );
           centVal -=    6L *   ( (long)( *(src +4L) )   );

           centVal -=    3L *   ( (long)( *(src -5L) )   );
           centVal -=    3L *   ( (long)( *(src +5L) )   );

           centVal -=    1L *   ( (long)( *(src -6L) )   );
           centVal -=    1L *   ( (long)( *(src +6L) )   );

          
           centVal  /=   32L;     //    25,  20  reduce Volume
          
          /****
           centVal +=  128L;            // convert back to UNsigned
           
           if(       centVal > 255L  )   *dst =  255;
           else if(  centVal <   0L  )   *dst =    0;
           else                          *dst =  ( unsigned char )centVal;
		   ***/
		   if(         centVal  >   127L  )   
			   *dst =     127;
           else if(  centVal  <  -127L  )  
			   *dst =    -127;
           else                                
			   *dst =  ( char )centVal;


 
           src++;   dst++;     
         }
     
     *dst =  *src;           src++;  dst++;  // just copy in 2nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in        LAST
     *dst =  *src;           src++;  dst++;  // just copy in 2nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in        LAST
     *dst =  *src;           src++;  dst++;  // just copy in 2nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in        LAST
  
  
     delete   cloneSample;      // *** IMPORTANT,  get rid of  'temp WORKSPACE'
     
     
 //    TRACE(  "...DONE( MID 'high-boost' FILTER )."  );

	 return  true;
  }




bool   MidFlatSndFilter::Filter(   CString&  retErrorMesg   )
  {     							            
	
					// MY original  'hack'  filter
     SndSample       *cloneSample;							    
     char   *src, *dst;          				      
     long             centVal,  i,       skip= 6L;
    
     											//  13 coeffs  Mid( lowish) 


 	retErrorMesg.Empty();

	if(    _Samp  ==   NULL    )    
	{
		ASSERT( 0 );
		retErrorMesg  =  "MidFlatSndFilter::Filter failed,  sample is NULL." ;
		return  false;
	}


	long   numByts  =      _Samp->Get_Length();


	if(   numByts == 0L )   
	{
		retErrorMesg  =  "MidFlatSndFilter::Filter failed,  length is 0." ;
		return  false;
	}


     cloneSample =  _Samp->Clone();    
	 
     if(    cloneSample  ==   NULL   )   
	 {
		retErrorMesg  =  "MidFlatSndFilter::Filter failed,  Clone is NULL." ;
		return  false;
	 }






 //    TRACE( "Starting  MID-flat Filter..." );


     src =     cloneSample->Get_StartByte();   // NOTE:  clone is now the SOURCE, write to  '_Samp' 
     dst =   _Samp->Get_StartByte();
 
     
     /////////////////////////////////////////

     *dst =  *src;           src++;  dst++;    // just copy in FIRST
     *dst =  *src;           src++;  dst++;    // just copy in Second
     *dst =  *src;           src++;  dst++;    // just copy in FIRST
     *dst =  *src;           src++;  dst++;    // just copy in Second
     *dst =  *src;           src++;  dst++;    // just copy in FIRST
     *dst =  *src;           src++;  dst++;    // just copy in Second
     
     
     for(  i= skip;   i<  numByts - skip;   i++   )   
         { 
           // centVal  =  144L *   ( (long)(  *src )        );
           ///////////////////////       
           
           centVal  =   14L *   ( (long)(  *src )        ); // little TRIM of highs( little low-pass )

           centVal +=   10L *   ( (long)( *(src -1L) )   );
           centVal +=   10L *   ( (long)( *(src +1L) )   );

           centVal +=    1L *   ( (long)( *(src -2L) )   );
           centVal +=    1L *   ( (long)( *(src +2L) )   );

          
           centVal  =   (centVal* 4L)  / 1L;   // 4.4 : ( boost to '144' from 32)
           /////////////////////////////          
           
           
           centVal -=   24L *   ( (long)(  *src )        ); // subtract VERY-LOWs

           centVal -=   22L *   ( (long)( *(src -1L) )   );
           centVal -=   22L *   ( (long)( *(src +1L) )   );

           centVal -=   17L *   ( (long)( *(src -2L) )   );
           centVal -=   17L *   ( (long)( *(src +2L) )   );

           centVal -=   11L *   ( (long)( *(src -3L) )   );
           centVal -=   11L *   ( (long)( *(src +3L) )   );

           centVal -=    6L *   ( (long)( *(src -4L) )   );
           centVal -=    6L *   ( (long)( *(src +4L) )   );

           centVal -=    3L *   ( (long)( *(src -5L) )   );
           centVal -=    3L *   ( (long)( *(src +5L) )   );

           centVal -=    1L *   ( (long)( *(src -6L) )   );
           centVal -=    1L *   ( (long)( *(src +6L) )   );

          
           centVal  /=   36L;     //   30,  20  reduce Volume
          
          /****
           centVal +=  128L;            // convert back to UNsigned
           
           if(       centVal > 255L  )   *dst =  255;
           else if(  centVal <   0L  )   *dst =    0;
           else                          *dst =  ( unsigned char )centVal;
			***/
		   if(         centVal  >   127L  )   
			   *dst =     127;
           else if(  centVal  <  -127L  )  
			   *dst =    -127;
           else                                
			   *dst =  ( char )centVal;


 
           src++;   dst++;     
         }
     
     *dst =  *src;           src++;  dst++;  // just copy in 2nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in        LAST
     *dst =  *src;           src++;  dst++;  // just copy in 2nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in        LAST
     *dst =  *src;           src++;  dst++;  // just copy in 2nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in        LAST
     ///////////////////////////////////////////////////////
  
  
     delete   cloneSample;      // *** IMPORTANT,  get rid of  'temp WORKSPACE'
     
     
   //  TRACE(  "...DONE(  MID-flat  FILTER )."  );

	 return  true;
  }





bool   VolumeFilter::Filter(    CString&  retErrorMesg   )
  {
     char   *src;          				      
     long      i,   numByts;      //  _Numer,  _Denom
     short    shVal;
     


	retErrorMesg.Empty();

	if(    _Samp  ==   NULL    )    
	{
		ASSERT( 0 );
		retErrorMesg  =  "VolumeFilter::Filter failed,  sample is NULL." ;
		return  false;
	}


	bool   doStereo =  _Samp->Is_Stereo_Sample();


     numByts =  _Samp->Get_Length();

	if(   numByts == 0L )   
	{
		retErrorMesg  =  "VolumeFilter::Filter failed,  length is 0." ;
		return  false;
	}




  //   TRACE(  "Starting  Volume Filter..."  );


     src =     _Samp->Get_StartByte();   

	 if(  src ==  NULL  )
	 {	
		 ASSERT( 0 );  
	 }
	 else
	 {     
         for(  i= 0L;   i<  numByts;   i++   )   
         { 

           shVal=  (short)( *src );
           
           shVal=    ( (shVal    * _Numer)  / _Denom   );
              
		   /***
           if(       shVal > 255  )    *src =   255;
           else if(  shVal <   0  )    *src =     0;
           else                        *src =  ( unsigned char )shVal;
		   ****/
		   if(         shVal  >   127L  )   
			   *src =     127;
           else if(  shVal  <  -127L  )  
			   *src =    -127;
           else                                
			   *src =  ( char )shVal;
 
           src++; 
         }
	 }     




	 if(   doStereo   )
	 {
	     src =     _Samp->Get_StartByte_RightStereo();   

		 if(  src ==  NULL  )
		 {	
			 ASSERT( 0 );  
		 }
		 else
		 {
			 for(  i= 0L;   i<  numByts;   i++   )   
			 { 

			   shVal=  (short)( *src );
	           

			   shVal=    ( (shVal    * _Numer)  / _Denom   );
	              
			   /***
			   if(       shVal > 255  )    *src =   255;
			   else if(  shVal <   0  )    *src =     0;
			   else                        *src =  ( unsigned char )shVal;
			   ****/
			   if(         shVal  >   127L  )   
				   *src =     127;
			   else if(  shVal  <  -127L  )  
				   *src =    -127;
			   else                                
				   *src =  ( char )shVal;

	 
			   src++; 
			 }
		 }     
	 }




   //  TRACE(  "...DONE(  Volume FILTER )."  );

	 return  true;
  }




										////////////////////////////////////////


bool   VolumeAutoAdjFilter::Filter(      CString&  retErrorMesg    )
{

						//  Now does stereo  

		//  Install a test for a virtuall dead signal 

		
	m_retScalePercentage =    -1;    //  init  returnVal



	short    overbrightThreshold  =   90;       //  80 was a little weak      [ 80 -  125 ]    ...determines the overall loudness    *****  ADJUST *****

 

	double   upperPercentThresh  =     0.50;     //   in percent   ...1 is one percent     *****  ADJUST *****

	double   lowerPercentThresh  =     0.10;      //    *****  ADJUST *****


	long        maxIters   =  40;    //  ***ADJUST***



	double     originalVolumeAdjust,   volumeAdjust;  


	originalVolumeAdjust  =   volumeAdjust  =	  1.0;     //   2.0;		//  init   ****  Why start so high ?????????????????????




	retErrorMesg.Empty();

	if(    _Samp  ==   NULL    )    
	{
		ASSERT( 0 );
		retErrorMesg  =  "VolumeAutoAdjFilter::Filter failed,  sample is NULL." ;
		return  false;
	}


	
	bool   doStereo =   _Samp->Is_Stereo_Sample();



	long	 numByts  =     _Samp->Get_Length();

	if(   numByts == 0L )   
	{
		retErrorMesg  =  "VolumeAutoAdjFilter::Filter failed,  length is 0." ;
		return  false;
	}



    char    *src,   *dst;
	long     overBriteCnt  =  0;


     SndSample   *cloneSample    =       _Samp->Clone();       

     if(    cloneSample  ==   NULL   )   
	 {
		retErrorMesg  =  "VolumeAutoAdjFilter::Filter failed,  Clone is NULL." ;
		return  false;
	 }




	long        biggestOutVal  =   -1,     iter= 0;


	long     upperThresh  =       (long)(       (double)numByts       *    ( upperPercentThresh  /100.0 )      );			//      PERCENT of  'Clipped-Samples'
				
	long	   lowerThresh   =      (long)(        (double)numByts       *     ( lowerPercentThresh  /100.0 )      );





	bool   needsReducing =   true;   
	bool   isDone =   false;
	bool   finalLap =  false;


	while(    ! isDone     )
	{

		src    =		cloneSample->Get_StartByte();       //   NOTE:  clone is now the SOURCE, write to  '_Samp' 
		dst    =		_Samp->Get_StartByte();
		overBriteCnt =  0;
		biggestOutVal  =   -1;


		for(     long  i =0L;       i <  numByts;       i++    )   
		{ 

 
			/***
			short    byteVal =      (long)( *src );
	//		           X   =		(double)(     (long)( *src )  - 128L    )    /  128.0;			//  get in {1.0 to -1.0}  ...from MACINTOSH
			double  X   =		 (double)(   byteVal  )     /  128.0;



			double   Y         =      volumeAdjust   *   X;			           //    amplify volume
           
			short      outVal  =      (long)(    Y   *  128.0   );                  //  back to  { 128.0 to -128.0 }
			****/
        

			short      shVal   =   (short)( *src );
			short      outVal  =      (short)(    volumeAdjust  *  (double)shVal   );              
 

			if(     absj( outVal )   >    biggestOutVal     )
				biggestOutVal  =     absj( outVal );


			if(         outVal  >    overbrightThreshold  )   
				overBriteCnt++;
			else if(  outVal  <  ( overbrightThreshold * -1)   )  
				overBriteCnt++;



			if(         outVal  >   127L  )   
			     *dst =     127;
			else if(  outVal  <  -127L  )  
				 *dst =    -127;
			else                                
				 *dst =  ( char )outVal;

			src++; 
			dst++;     
		 }



		/////////////////////
		iter++;




		if(       (       overBriteCnt    <=     upperThresh  
			       &&   overBriteCnt    >=     lowerThresh  )
			||	 finalLap ==  true 	 )
		{
			isDone =    true;
		}
		else
		{
			if(           overBriteCnt   >    upperThresh     )
			{

				if(    iter  ==  1   )
					needsReducing =    true;									   	//   must   'DECREASE'   the volume
				else
				{	if(    ! needsReducing    )
						isDone =    true;			//   trying to REVERSE DIRECTIONS,  so stop and live with last iter
				}

				volumeAdjust  =      volumeAdjust   *   0.80;
			}

			else if(    overBriteCnt   <    lowerThresh      )
			{

				if(    iter  ==  1   )
					needsReducing =     false;										//   must   'INCREASE'   the volume
				else
				{	if(    needsReducing    )
						isDone =    true;			
				}

				volumeAdjust  =      volumeAdjust   *   1.25;
			}

		}




		if(    iter  ==  maxIters    )   //  MAX  iters
		{


//			ASSERT( 0 );	   		// ***** ERROR:   hit max iters   [ Got here with Player.exe when there was not enough volume of a signal to analyize.  1/5/10 


						//  Bad signal in Left Stereo.  Got here with a bad threshold score for CalcCopy_CenterPan_by_Pitchels(). Give a message and let the functions finish.  Test OK.   6/07 



			retErrorMesg  =  "VolumeAutoAdjFilter failed( MAX iterations ).\nThe resultant sound sample may be empty or poorly calculated." ;
		//	AfxMessageBox(  "VolumeAutoAdjFilter failed( MAX iterations ).\nThe resultant Wave sample may be empty or poorly calculated."   );   NO!!! will virtually crash PitchPlayer   1/10


			finalLap          =    true;    //  let it do it one more time at the original amplification. The RightStereo sample

			volumeAdjust =    originalVolumeAdjust;

			//  return  false;   NO...    *** Let it go,  see what happens. Get here if the signal is very weak.  ***
		}
		else if(   iter  >  maxIters   ) 
			isDone =    true;

	}    //   while(   ! isDone   )






	 if(   doStereo   )    //  just do the right channel to what the left was adjusted to.
	 {

		 src  =		cloneSample->Get_StartByte_RightStereo();       //   NOTE:  clone is now the SOURCE, write to  '_Samp' 
		 dst  =		_Samp->Get_StartByte_RightStereo();

		 if(       src ==  NULL  
			 ||    dst ==  NULL      )
		{	
			retErrorMesg  =  "VolumeAutoAdjFilter::Filter failed,  one of the Right Stereo samples is NULL." ;
			return  false;
		}

			
		for(  long i= 0L;   i<  numByts;   i++   )   
		{ 

			short      shVal=  (short)( *src );
			short      outVal  =      (short)(    volumeAdjust  *  (double)shVal   );              
            
			if(         outVal  >   127L  )   
			     *dst =     127;
			else if(  outVal  <  -127L  )  
				 *dst =    -127;
			else                                
				 *dst =  ( char )outVal;

	 
			src++; 
			dst++;
		}		 
	 }




    delete   cloneSample;      // *** IMPORTANT,  get rid of  'temp WORKSPACE'



	m_retScalePercentage      =       (long)(    volumeAdjust    *   100.0    );

	double   percentOverBrite  =       (  (double)overBriteCnt  /   (double)numByts  )  *  100.0;   


//	TRACE(     "\nAUTO-Adjust  VOLUME  filter [   ITERS =  %d,      VolumeChange =  %d          Percent-OVERBRITE =   %.3f,    ]\n", 
//																						        	iter,     m_retScalePercentage,       percentOverBrite        );	

	 return  true;
}	





///////////////////////////////////////////////////
///////////////////////////////////////////////////	
///////////////////////////////////////////////////
///////////////////////////////////////////////////


long     Fir_Filter(   char  *dst,   long  *coef,   long n  ) 
  {
    	   //  NEW.... with
    	
    	       // NOO!!!!!    float   input       new float input sample
    	
    	//  float  *coef        pointer to filter coefficients
    	//  int     n           number of coefficients in filter
    	//  float  *history     history array pointer
    	//    ...Returns             float value giving the current output.
    
    long   *coef_ptr;
    long   i,  output=0L,  inVal;
    char                  shiftFact = 23;    //  23  *** can adjust??? 

    char  *src;          				      


    src       =   dst -  ( n /2L );             // offset BACK from filter-center

    coef_ptr  =   coef;            // point to  1st  coeff

    
    for(  i= 0;   i< n;   i++  ) 				// form output accumulation
      {
         inVal  =   ( (long)( *dst )     )  <<  shiftFact; 
                                             
         output  +=   FracMulJM(   inVal,   ( *coef_ptr )   );    // output +=  (*hist_ptr++) * (*coef_ptr--);
     
         coef_ptr++;    dst++;
      }
                                                      

    return   output;
  }


long   OLD__Fir_Filter(  long  input,   long  *coef,    long n,   long  *history   ) 
  {
    	//  float   input       new float input sample
    	//  float  *coef        pointer to filter coefficients
    	//  int     n           number of coefficients in filter
    	//  float  *history     history array pointer
    	//    ...Returns             float value giving the current output.
    
    long  *hist_ptr,  *hist1_ptr,  *coef_ptr;
    long   i,  output;

    hist_ptr  =   history;
    hist1_ptr =   hist_ptr;                // use for history update 
    coef_ptr  =   coef  +  n  - 1;         // point to last coef 

											          // form output accumulation
    output =  FracMulJM(  *hist_ptr++,  (*coef_ptr--)   );      //  output = *hist_ptr++  *  ( *coef_ptr-- );
    
    for(  i= 2;   i< n;   i++  ) 
      {
         *hist1_ptr++  =   *hist_ptr;                   // update history array 
                                             
         output  +=   FracMulJM(   (*hist_ptr++),   (*coef_ptr--)   );    // output +=  (*hist_ptr++) * (*coef_ptr--);
      }
                                                      
    output  +=   FracMulJM(  input,   (*coef_ptr)   );      // input tap,      output += input * (*coef_ptr); 
    
    *hist1_ptr  =   input;                           // last history 

    return   output;
  }




long   IIR_Filter(  long  input,  long  *coef,  long  filtSects,   long  *history  )
  {
   // 'IIR' filters are UNDESIREABLE beacause the coeffs are soo HARD to design( pp 145, c-Algo )
     
      //  The length (filtSects) of the filter specifies the number of sections.
      //  The size of the history array is  2*filtSects.
      //  The size of the coefficient array is  4*filtSects + 1   because...
      
      //  *** ????  the first coefficient is the overall scale factor for the filter.

    long   *hist1_ptr,  *hist2_ptr,  *coef_ptr;
    long   i,  output,  new_hist,   history1,  history2;

    
    coef_ptr  =  coef;                // coefficient pointer 
    hist1_ptr =  history;                 // first history 
    hist2_ptr =  hist1_ptr + 1;           //  next history 

          
    output =  FracMulJM( input,  ( *coef_ptr++ )  );   // overall input scale factor,   output= input  *  (*coef_ptr++); 
    
    for(  i= 0;   i< filtSects;   i++  ) 
      {
        history1 =  *hist1_ptr;       history2 =  *hist2_ptr;       // history values 

        output   =   output     -    FracMulJM(  history1,  ( *coef_ptr++ )  );
        new_hist =   output     -    FracMulJM(  history2,  ( *coef_ptr++ )  );  // poles 

        output   =   new_hist   +    FracMulJM(  history1,  ( *coef_ptr++ )  );
        output   =   output     +    FracMulJM(  history2,  ( *coef_ptr++ )  );  // zeros 

        *hist2_ptr++ =   *hist1_ptr;
        *hist1_ptr++ =    new_hist;
        
        hist1_ptr++;    hist2_ptr++;
      }

    return   output;
  }



bool   FIRsndFilter::Filter(     CString&  retErrorMesg   )         
  {            
	
	// 'FIR'  filter  
    //  float   input         new float input sample
    //  float  *coef          pointer to filter coefficients
    //  int     numCoeffs     number of coefficients in filter
    //  float  *history       history array pointer
    //   ...  Returns     float value giving the current output.

     SndSample       *cloneSample;			            // *** TEMP with IIR filter test				    
     char   *src, *dst;          				      
     long                  outVal,  i,   numByts;
     
     char     shiftFact = 23;            //  23  *** can adjust??? 
     
     double   reDux =  1073741824.0;    //  1073741824.0   *** ADJUST  ????

 
 
     //  numCoeffs = 35;   filtFloatCoeffSRC =  &(   fir_lpf35[ 0 ]   ); //  Parks-McClellan
     //  numCoeffs = 37;   filtFloatCoeffSRC =  &(   fir_lpf37[ 0 ]   ); // KAISER
     
	retErrorMesg.Empty();

	if(    _Samp  ==   NULL    )    
	{
		ASSERT( 0 );
		retErrorMesg  =  "FIRsndFilter::Filter failed,  sample is NULL." ;
		return  false;
	}     



     if(( _floatCoeffs == NULL )||( _numCoeffs <= 0 ))   
	 {
		retErrorMesg  =  "FIRsndFilter::Filter failed,  Coefs have bad value." ;
		 return  false;
	 }


     numByts =  _Samp->Get_Length();

	if(   numByts == 0L )   
	{
		retErrorMesg  =     "FIRsndFilter::Filter failed,  length is 0." ;
		return  false;
	}


  
     cloneSample =    _Samp->Clone();       

     if(    cloneSample  ==   NULL   )   
	 {
		retErrorMesg  =  "FIRsndFilter::Filter failed,  Clone is NULL." ;
		return  false;
	 }


   

   //  TRACE( "Starting  FIR  filter..." );

    
     for(  i=0;   i<  _numCoeffs;   i++   )         // make 'SPFRACFACT' filter coefs
       {  
          filtFixCoeffs[i] =  (long)(  _floatCoeffs[i]  *  reDux  );
          filtHistory[i]     =  0L;
       }


     src =     cloneSample->Get_StartByte();   // NOTE:  clone is now the SOURCE, write to  '_Samp' 
     dst =   _Samp->Get_StartByte();
 
     
     /////////////////////////////////////////
     for(  i= 0L;   i<  numByts;   i++   )   
         { 
           
            //  inVal  =   ( (long)( *src )     )  <<  shiftFact;    //  MacSign_2Frac() 
            //  outVal =   Fir_Filter(  inVal,  filtFixCoeffs,  _numCoeffs,   filtHistory  ); 
  
           outVal =   Fir_Filter(   src,  filtFixCoeffs,  _numCoeffs  ); 
									// a rewrite with better this for better centering....
  
  
           
           outVal =   ( outVal  >> shiftFact );            //  Frac_2MacSign()  and  convert back to UNsigned

           /***
           if(       outVal > 255L  )   *dst =  255;
           else if(  outVal <   0L  )   *dst =    0;
           else                         *dst =  ( unsigned char )outVal;
		   ****/
		   if(         outVal  >   127L  )   
			   *dst =     127;
           else if(  outVal  <  -127L  )  
			   *dst =    -127;
           else                                
			   *dst =  ( char )outVal;



 
           src++;   dst++;     
         }
     /////////////////////////////////////////
  
  
     delete   cloneSample;      // *** IMPORTANT,  get rid of  'temp WORKSPACE'
     
  //   TRACE(  "...DONE(  FIR  filter  )."  );

	 return  true;
  }



bool  IIRsndFilter::Filter(     CString&  retErrorMesg   )         
  {                                                 
	  //  'IIR'  filter
	  //
      //  The length (filtSects) of the filter specifies the number of sections.
      //  The size of the history array is       2*filtSects.
      //  The size of the coefficient array is   4*filtSects + 1  because...
      
      //  *** ????  the first coefficient is the overall scale factor for the filter.

     SndSample       *cloneSample;			            // *** TEMP with IIR filter test				    
     char   *src, *dst;          				      
     long             inVal, outVal,  i,   numByts;
     long   numCoeffs;
     short  bandNum=0;
     
     char     shiftFact = 23;            //  23  *** can adjust??? 
     double   reDux =  1073741824.0;    //  1073741824.0   *** ADJUST  ????
 
    
    
    //  filtSects= 3L;    filtFloatCoeffSRC=  &(  iir_lpf5[ 0 ]  );     // iir_hpf6[ 0 ]      
     

	retErrorMesg.Empty();

	if(    _Samp  ==   NULL    )    
	{
		ASSERT( 0 );
		retErrorMesg  =  "IIRsndFilter::Filter failed,  sample is NULL." ;
		return  false;
	}

     
     if(( _floatCoeffs == NULL )||( _filtSects <= 0 ))    
	 {
		 ASSERT( 0 );
		retErrorMesg  =  "IIRsndFilter::Filter failed,  FiltSects have bad value." ;
		 return  false;
	 }


     numByts =  _Samp->Get_Length();
  
	if(   numByts == 0L )   
	{
		retErrorMesg  =  "IIRsndFilter::Filter failed,  length is 0." ;
		return  false;
	}



     cloneSample =  _Samp->Clone();    
	 
     if(    cloneSample  ==   NULL   )   
	 {
		retErrorMesg  =  "IIRsndFilter::Filter failed,  Clone is NULL." ;
		return  false;
	 }



     if(  numByts == 0L )   
	 {
		 ASSERT( 0 );
		retErrorMesg  =  "IIRsndFilter::Filter failed,  numByts is 0." ;
		 return  false;
	 }

   //  TRACE( "Starting  IIR  BandPass-FILTER ..." );

     
     numCoeffs =    4L *  _filtSects     + 1L;    
    
     for(  i=0;   i<  numCoeffs;   i++   )           // make 'SPFRACFACT' filter coefs
         filtFixCoeffs[i] = (long)(  _floatCoeffs[i] *  reDux  );

     for(  i=0;   i<  (2L * _filtSects);    i++   )           // init to zero            
         filtHistory[i] = 0L;
         

     src =     cloneSample->Get_StartByte();   // NOTE:  clone is now the SOURCE, write to  '_Samp' 
     dst =   _Samp->Get_StartByte();
 
     
     /////////////////////////////////////////
     for(  i= 0L;   i< numByts;   i++  )   
         { 
           inVal  =   ( (long)( *src )      )   <<  shiftFact;   //  MacSign_2Frac() 

           outVal =  IIR_Filter(  inVal,  filtFixCoeffs,  _filtSects,  filtHistory  );
  
           outVal =   ( outVal  >> shiftFact );           //  Frac_2MacSign()  and  convert back to UNsigned

           /***
           if(       outVal > 255L  )   *dst =  255;
           else if(  outVal <   0L  )   *dst =    0;
           else                         *dst =  ( unsigned char )outVal;
		   ***/
		   if(         outVal  >   127L  )   
			   *dst =     127;
           else if(  outVal  <  -127L  )  
			   *dst =    -127;
           else                                
			   *dst =  ( char )outVal;


 
           src++;   dst++;     
         }
     /////////////////////////////////////////

  
     delete   cloneSample;      // *** IMPORTANT,  get rid of  'temp WORKSPACE'
     
    // TRACE(  "...DONE(  IIR  BandPass-FILTER  )."  );

	 return  true;
  }



bool   FIRloPassCalcCoefSndFilter::Filter(    CString&  retErrorMesg   )
  {
     long     numCoeffs;


	retErrorMesg.Empty();

	if(    _Samp  ==   NULL    )    
	{
		ASSERT( 0 );
		retErrorMesg  =  "FIRloPassCalcCoefSndFilter::Filter failed,  sample is NULL." ;
		return  false;
	}


     double   sampRate =   (double)(  _Samp->_sampleRate  );

     
     numCoeffs =  Calc_Fir_BandPass_Coeffs(   (double)_EdgeFreq0  /sampRate,   
                                               (double)_EdgeFreq1  /sampRate,  
                                     0.0,  _atten,   1  );   // 1: lowpass
     
     if(  numCoeffs  !=  0L  )
       {
          FIRsndFilter    filt(  _Samp,   numCoeffs,   fir_custCoeffs   ); 
  
          if(     ! filt.Filter(    retErrorMesg  )      )
			  return  false;
       }


	 return  true;
  }  


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////   KAISER calc of filter Coeffecients   ////////////////////


void   filter_length(  double att,  double deltaf, long *nfilt, long *npair, double *beta  )
  {
       //  Use att to get beta (for Kaiser window function) and nfilt (always odd
       //             valued and = 2*npair +1) using Kaiser's empirical formulas.

    *beta = 0;      //  value of beta if att < 21 
    
    if(  att  >=  50.0  )       *beta =   0.1102  *  (att - 8.71);
    

    if(  att < 50   &&   att >= 21  )
       *beta =   0.5842  *  pow(  (att-21),  0.4  )    +     .07886  *  (att - 21);
    

    *npair =  (long)(   (att - 8)  /  (29 * deltaf)   );
    

    *nfilt =  2  *   *npair +1;
  }


double   izero(  double y  )
  {                  // Compute Bessel function Izero(y) using a series approximation 

    double  s= 1.0, ds= 1.0,  d= 0.0;
    
    do{  d =   d + 2;     
         ds =  ds   *    (y*y) / (d*d);
         s =   s + ds;
      }while(   ds  >  1E-7 * s   );
    
    return   s;
  }



long   Calc_Fir_BandPass_Coeffs(   double  lowEdge,   double  hiEdge,  
                        double  transWidth,    double att,   short filt_cat  )
  {
       // att = 40.0; ...get_float("Desired stopband attenuation (dB)", 10, 200);
  
    char  strg[ 100 ];
    static  double   h[ 500 ], w[ 500 ], x[ 500 ];
    
    long       npair, nfilt, n;
    double   fa, fp, fa1, fa2, fp1, fp2, deltaf, d1, d2, fl, fu, beta;
    double   fc, fm, pifc, tpifm, i, y, valizb;
  
   //  short   filt_cat;  // filter type
  
   // filt_cat =  3;    //  "BP"  ...( BandPass )
   // filt_cat =  1;    //  "BP"  ...( LOWPass )


   //////////////////////
  
  
   switch(  filt_cat  )
     {
       case 3:    // ( BandPass )

         fp1 =  lowEdge;         fp2 =  hiEdge;            // get as parms 
         d1 = d2 = transWidth;
    
         fa1 = lowEdge - transWidth;        fa2 =  hiEdge + transWidth;
    
         if(  fabs(d1 - d2)   >  1E-5   )
           {  TRACE("***ERROR...transition bands not equal." );  return 0L;  }             

         deltaf =   d1; 
         if( filt_cat == 4 )    deltaf =  -deltaf;
    
         filter_length(  att,  deltaf,    &nfilt,  &npair,  &beta  );
    
         if( npair > 200 )
            { TRACE(  "***ERR: Filter length is too large." );   return 0L; }
         
         sprintf( strg,  "FiltLEN: %d,  beta: %f",    nfilt,  beta  );    TRACE( strg );
            
         fl =   ( fa1 + fp1 )  / 2;      
         fu =   ( fa2 + fp2 )  / 2;
         fc =   (  fu - fl  );            
         fm =   (  fu + fl  )  / 2;
    
         h[ npair ] =   2 * fc;
         if(  filt_cat ==  4  )     h[ npair ] =   1  -   2 * fc;
    
         pifc =  PI * fc;        tpifm =   2 * PI * fm;
    
         for(  n= 0;   n< npair;   n++  )
           {
             i      =   ( npair  -  n );
             h[ n ] =   2 * sin( i * pifc )    *     cos(i * tpifm)  /  (i * PI);
        
             if(  filt_cat ==  4 )    h[ n ] =  -h[ n ];
           }
         //////////////////////
       break;



       case 1:    // ( LOWPass )
           						     //  fp = get_float( fp_s, 0,  0.5 );
          							 //  fa = get_float( fa_s, fp, 0.5 );  
          fp =  lowEdge;    fa =  hiEdge;      // NOW from parms
          
          
          deltaf =  ( fa - fp );        // if( filt_cat == 2 )   deltaf =  -deltaf;
          
          filter_length(  att,  deltaf,  &nfilt,  &npair,  &beta  );
 
          if( npair > 200 )
            {  TRACE(  "***ERR: Filter length is too large." );   return 0L; }
         
          sprintf( strg,  "FiltLEN: %d,  beta: %f",    nfilt,  beta  );    TRACE( strg );
         
          fc = ( fp + fa );      h[ npair ] = fc;
            					          //  if( filt_cat ==  2 )   h[ npair ] =   1 - fc;
          
          pifc =   PI * fc;
          for(  n= 0;   n< npair;  n++  )
            {
              i =  ( npair - n );
              h[ n ] =   sin( i * pifc )   /   ( i * PI );
              
              //  if( filt_cat == 2 )   h[ n ] =   - h[ n ];
            }
       break;

     
       default:    return 0L;                break;     
     }
	
	
	
    //////////////////////////////////////////////////////////////////////////////////										 
    y =       beta;  				           // Compute Kaiser WINDOW sample values
    valizb =  izero( y );          
    for(   n= 0;   n<=  npair;   n++   )
      {
        i =        ( n  -  npair );
        y =        beta  *   sqrt(   1  -   (i / npair)  *  (i / npair)    );
        w[ n ] =   izero( y )  /  valizb;
      }

        										// first half of response 
    for(  n = 0;   n<=  npair;    n++  ) 
                                   x[ n ] =   w[n]  *  h[n];

      /*********************
         printf( "\n---First half of coefficient set...remainder by symmetry---" );
         printf( "\n  #      ideal        window     actual    " );
         printf( "\n         coeff        value    filter coeff" );
         for(  n=0;   n<=  npair;   n++  )
              printf(  "\n %4d   %9.6f   %9.6f   %9.6f",    n,  h[n],  w[n],  x[n]  );
      *********************/


  
                                           // copy to global area
    for(  n = 0;   n<=  npair;    n++  )
         fir_custCoeffs[ n ]  =  fir_custCoeffs[  (nfilt -1) - n  ]  =  x[ n ];
  
  
  
    return   nfilt;       // number of coeffs, not pairs
  }






bool   LowLoSndFilter::Filter(     CString&  retErrorMesg   )
  {
     SndSample       *cloneSample;							      //  7 coeffs Mid 
     char   *src, *dst;          				      
     long             centVal,  i,   numByts,      skip= 3L;
    
     
 	retErrorMesg.Empty();

	if(    _Samp  ==   NULL    )    
	{
		ASSERT( 0 );
		retErrorMesg  =  "LowLoSndFilter::Filter failed,  sample is NULL." ;
		return  false;
	}



     numByts =  _Samp->Get_Length();
  
	if(   numByts == 0L )   
	{
		retErrorMesg  =  "LowLoSndFilter::Filter failed,  length is 0." ;
		return  false;
	}



     cloneSample =  _Samp->Clone();       

     if(    cloneSample  ==   NULL   )   
	 {
		retErrorMesg  =  "LowLoSndFilter::Filter failed,  Clone is NULL." ;
		return  false;
	 }



  //   TRACE( "Starting   LOW-'lo' FILTER..." );


     src =     cloneSample->Get_StartByte();   // NOTE:  clone is now the SOURCE, write to  '_Samp' 
     dst =   _Samp->Get_StartByte();
 
     
     /////////////////////////////////////////
 
     *dst =  *src;           src++;  dst++;    // just copy in FIRST
     *dst =  *src;           src++;  dst++;    // just copy in Second
     *dst =  *src;           src++;  dst++;    // just copy in Third
     
     
     for(  i= skip;   i<  numByts - skip;   i++   )   
         { 
           ///////////////////////       
           
           centVal  =    1L *   ( (long)(  *src )        ); // little TRIM of highs( little low-pass )

           centVal +=    1L *   ( (long)( *(src -1L) )   );
           centVal +=    1L *   ( (long)( *(src +1L) )   );

           centVal +=    1L *   ( (long)( *(src -2L) )   );
           centVal +=    1L *   ( (long)( *(src +2L) )   );

           centVal +=    1L *   ( (long)( *(src -3L) )   );
           centVal +=    1L *   ( (long)( *(src +3L) )   );

          
           centVal  /=   5L;         //   reduce Volume
          
			/*****
           centVal +=  128L;            // convert back to UNsigned
           
           if(       centVal > 255L  )   *dst =  255;
           else if(  centVal <   0L  )   *dst =    0;
           else                          *dst =  ( unsigned char )centVal;
		   ****/
		   if(         centVal  >   127L  )   
			   *dst =     127;
           else if(  centVal  <  -127L  )  
			   *dst =    -127;
           else                                
			   *dst =  ( char )centVal;



 
           src++;   dst++;     
         }
     
     *dst =  *src;           src++;  dst++;  // just copy in 3nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in 2nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in        LAST
  
  
     delete   cloneSample;      // *** IMPORTANT,  get rid of  'temp WORKSPACE'
     
     
  //   TRACE(  "...DONE( LOW 'lo' FILTER )."  );

	 return  true;
  }




bool   LowMedSndFilter::Filter(   CString&  retErrorMesg    )
  {
     SndSample       *cloneSample;							      //  5 coeffs Mid 
     char   *src, *dst;          				      
     long             centVal,  i,   numByts,      skip= 2L;
    

     
     if( _Samp == NULL )      
	 {
		retErrorMesg  =  "LowMedSndFilter::Filter failed,  _Samp  is NULL." ;
		 return   false;
	 }



     numByts =  _Samp->Get_Length();
  
	if(   numByts == 0L )   
	{
		retErrorMesg  =  "LowMedSndFilter::Filter failed,  length is 0." ;
		return  false;
	}


	bool   doStereo =  _Samp->Is_Stereo_Sample();

     cloneSample =  _Samp->Clone();     
	 
     if(    cloneSample  ==   NULL   )   
	 {
		retErrorMesg  =  "LowMedSndFilter::Filter failed,  Clone is NULL." ;
		return  false;
	 }




    // TRACE( "Starting   LOW-'med' FILTER..." );


     src =     cloneSample->Get_StartByte();   // NOTE:  clone is now the SOURCE, write to  '_Samp' 
     dst =   _Samp->Get_StartByte();
 
     
     /////////////////////////////////////////
 
     *dst =  *src;           src++;  dst++;    // just copy in FIRST
     *dst =  *src;           src++;  dst++;    // just copy in Second
     
     
     for(  i= skip;   i<  numByts - skip;   i++   )   
         { 
           ///////////////////////       
           
           centVal  =    1L *   ( (long)(  *src )        ); // little TRIM of highs( little low-pass )

           centVal +=    1L *   ( (long)( *(src -1L) )   );
           centVal +=    1L *   ( (long)( *(src +1L) )   );

           centVal +=    1L *   ( (long)( *(src -2L) )   );
           centVal +=    1L *   ( (long)( *(src +2L) )   );

          
           centVal  /=   5L;         //   reduce Volume
          
          /****
           centVal +=  128L;            // convert back to UNsigned
           
           if(       centVal > 255L  )   *dst =  255;
           else if(  centVal <   0L  )   *dst =    0;
           else                          *dst =  ( unsigned char )centVal;
		   ***/
		   if(         centVal  >   127L  )   
			   *dst =     127;
           else if(  centVal  <  -127L  )  
			   *dst =    -127;
           else                                
			   *dst =  ( char )centVal;

		   
 
           src++;   dst++;     
         }
     
     *dst =  *src;           src++;  dst++;  // just copy in 2nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in        LAST
  



	 if(   doStereo   )
	 {
	     src =     _Samp->Get_StartByte_RightStereo();   
         dst =   _Samp->Get_StartByte_RightStereo();
 
 

		 if(      src  ==  NULL 
			 ||  dst  ==  NULL   )
		 {	
			 ASSERT( 0 );  
		 }
		 else
		 {
			 *dst =  *src;           src++;  dst++;    // just copy in FIRST
			 *dst =  *src;           src++;  dst++;    // just copy in Second
		     
		     
			 for(  i= skip;   i<  numByts - skip;   i++   )   
			 { 
				   ///////////////////////       
		           
				   centVal  =    1L *   ( (long)(  *src )        ); // little TRIM of highs( little low-pass )

				   centVal +=    1L *   ( (long)( *(src -1L) )   );
				   centVal +=    1L *   ( (long)( *(src +1L) )   );

				   centVal +=    1L *   ( (long)( *(src -2L) )   );
				   centVal +=    1L *   ( (long)( *(src +2L) )   );

		          
				   centVal  /=   5L;         //   reduce Volume
		          
				  /****
				   centVal +=  128L;            // convert back to UNsigned
		           
				   if(       centVal > 255L  )   *dst =  255;
				   else if(  centVal <   0L  )   *dst =    0;
				   else                          *dst =  ( unsigned char )centVal;
				   ***/
				   if(         centVal  >   127L  )   
					   *dst =     127;
				   else if(  centVal  <  -127L  )  
					   *dst =    -127;
				   else                                
					   *dst =  ( char )centVal;

				   
		 
				   src++;   dst++;     
			 }
		     

			 *dst =  *src;           src++;  dst++;  // just copy in 2nd to LAST
			 *dst =  *src;           src++;  dst++;  // just copy in        LAST
		 }
 
	}     




  
     delete   cloneSample;      // *** IMPORTANT,  get rid of  'temp WORKSPACE'
     
     
   //  TRACE(  "...DONE( LOW 'med' FILTER )."  );

	 return  true;
  }



bool   LowHiSndFilter::Filter(   CString&  retErrorMesg   )
  {

     SndSample       *cloneSample;							      //  3 coeffs Mid 
     char   *src, *dst;          				      
     long             centVal,  i,   numByts,      skip= 1L;
    
     
	retErrorMesg.Empty();

	if(    _Samp  ==   NULL    )    
	{
		ASSERT( 0 );
		retErrorMesg  =  "LowHiSndFilter::Filter failed,  sample is NULL." ;
		return  false;
	}


     numByts =  _Samp->Get_Length();
  
	if(   numByts == 0L )   
	{
		retErrorMesg  =  "LowHiSndFilter::Filter failed,  length is 0." ;
		return  false;
	}


	bool   doStereo =  _Samp->Is_Stereo_Sample();



     cloneSample =  _Samp->Clone();     
	 
     if(    cloneSample  ==   NULL   )   
	 {
		retErrorMesg  =  "LowHiSndFilter::Filter failed,  Clone is NULL." ;
		return  false;
	 }




  //   TRACE( "Starting   LOW-'hi' FILTER..." );


     src =     cloneSample->Get_StartByte();   // NOTE:  clone is now the SOURCE, write to  '_Samp' 
     dst =   _Samp->Get_StartByte();
 
     
     /////////////////////////////////////////
 
     *dst =  *src;           src++;  dst++;    // just copy in FIRST
     
     
     for(  i= skip;   i<  numByts - skip;   i++   )   
     { 
           ///////////////////////       
           
           centVal  =    1L *   ( (long)(  *src )        ); // little TRIM of highs( little low-pass )

           centVal +=    1L *   ( (long)( *(src -1L) )   );
           centVal +=    1L *   ( (long)( *(src +1L) )   );

          
           centVal  /=   3L;         //   reduce Volume
          

          /****
           centVal +=  128L;            // convert back to UNsigned
           
           if(       centVal > 255L  )   *dst =  255;
           else if(  centVal <   0L  )   *dst =    0;
           else                          *dst =  ( unsigned char )centVal;
		   ****/
		   if(         centVal  >   127L  )   
			   *dst =     127;
           else if(  centVal  <  -127L  )  
			   *dst =    -127;
           else                                
			   *dst =  ( char )centVal;


 
           src++;   dst++;     
     }
     
     *dst =  *src;           src++;  dst++;  // just copy in        LAST
  




	 if(   doStereo   )
	 {
	     src =     _Samp->Get_StartByte_RightStereo();   
         dst =   _Samp->Get_StartByte_RightStereo();
 
 

		 if(      src  ==  NULL 
			 ||  dst  ==  NULL   )
		 {	
			 ASSERT( 0 );  
		 }
		 else
		 {
			*dst =  *src;           src++;  dst++;    // just copy in FIRST
		     
		     
			 for(  i= skip;   i<  numByts - skip;   i++   )   
			 { 
				   ///////////////////////       
		           
				   centVal  =    1L *   ( (long)(  *src )        ); // little TRIM of highs( little low-pass )

				   centVal +=    1L *   ( (long)( *(src -1L) )   );
				   centVal +=    1L *   ( (long)( *(src +1L) )   );

		          
				   centVal  /=   3L;         //   reduce Volume
		          

				  /****
				   centVal +=  128L;            // convert back to UNsigned
		           
				   if(       centVal > 255L  )   *dst =  255;
				   else if(  centVal <   0L  )   *dst =    0;
				   else                          *dst =  ( unsigned char )centVal;
				   ****/
				   if(         centVal  >   127L  )   
					   *dst =     127;
				   else if(  centVal  <  -127L  )  
					   *dst =    -127;
				   else                                
					   *dst =  ( char )centVal;


		 
				   src++;   dst++;     
			 }
		     
			 *dst =  *src;           src++;  dst++;  // just copy in        LAST
		 }     

	 }








  
     delete   cloneSample;      // *** IMPORTANT,  get rid of  'temp WORKSPACE'
     
     
   //  TRACE(  "...DONE( LOW 'hi' FILTER )."  );

	 return  true;
  }



/////////////////////////////////////

bool   HiBoostLoSndFilter::Filter(    CString&  retErrorMesg   )
  {

     SndSample       *cloneSample;							      //  15  coeffs 
     char   *src, *dst;          				      
     long             centVal,  i,   numByts,      skip= 7L;
    
     
	retErrorMesg.Empty();

	if(    _Samp  ==   NULL    )    
	{
		ASSERT( 0 );
		retErrorMesg  =  "HiBoostLoSndFilter::Filter failed,  sample is NULL." ;
		return  false;
	}



     numByts =  _Samp->Get_Length();
  
	if(   numByts == 0L )   
	{
		retErrorMesg  =  "HiBoostLoSndFilter::Filter failed,  length is 0." ;
		return  false;
	}



     cloneSample =  _Samp->Clone();      
	 
      if(    cloneSample  ==   NULL   )   
	 {
		retErrorMesg  =  "HiBoostLoSndFilter::Filter failed,  Clone is NULL." ;
		return  false;
	 }



    // TRACE( "Starting   HiBOOST-'lo' FILTER..." );


     src =     cloneSample->Get_StartByte();   // NOTE:  clone is now the SOURCE, write to  '_Samp' 
     dst =   _Samp->Get_StartByte();
 
     
     /////////////////////////////////////////
 
     *dst =  *src;           src++;  dst++;    // just copy in FIRST
     *dst =  *src;           src++;  dst++;    // just copy in Second
     *dst =  *src;           src++;  dst++;    // just copy in Third
     *dst =  *src;           src++;  dst++;    // just copy in Second
     *dst =  *src;           src++;  dst++;    // just copy in Third
     *dst =  *src;           src++;  dst++;    // just copy in Second
     *dst =  *src;           src++;  dst++;    // just copy in Third
     
     
     for(  i= skip;   i<  numByts - skip;   i++   )   
         { 
           ///////////////////////       
           
           centVal  =   15L *   ( (long)(  *src )        ); // little TRIM of highs( little low-pass )

           centVal -=    1L *   ( (long)( *(src ) )      );

           centVal -=    1L *   ( (long)( *(src -1L) )   );
           centVal -=    1L *   ( (long)( *(src +1L) )   );

           centVal -=    1L *   ( (long)( *(src -2L) )   );
           centVal -=    1L *   ( (long)( *(src +2L) )   );

           centVal -=    1L *   ( (long)( *(src -3L) )   );
           centVal -=    1L *   ( (long)( *(src +3L) )   );

           centVal -=    1L *   ( (long)( *(src -4L) )   );
           centVal -=    1L *   ( (long)( *(src +4L) )   );

           centVal -=    1L *   ( (long)( *(src -5L) )   );
           centVal -=    1L *   ( (long)( *(src +5L) )   );

           centVal -=    1L *   ( (long)( *(src -6L) )   );
           centVal -=    1L *   ( (long)( *(src +6L) )   );

           centVal -=    1L *   ( (long)( *(src -7L) )   );
           centVal -=    1L *   ( (long)( *(src +7L) )   );


          
           centVal  /=   11L;         //   reduce Volume         
         
          /****
           centVal +=  128L;            // convert back to UNsigned
           
           if(       centVal > 255L  )   *dst =  255;
           else if(  centVal <   0L  )   *dst =    0;
           else                          *dst =  ( unsigned char )centVal;
		   ****/
		   if(         centVal  >   127L  )   
			   *dst =     127;
           else if(  centVal  <  -127L  )  
			   *dst =    -127;
           else                                
			   *dst =  ( char )centVal;


 
           src++;   dst++;     
         }
     
     *dst =  *src;           src++;  dst++;  // just copy in 3nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in 2nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in 3nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in 2nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in        LAST
     *dst =  *src;           src++;  dst++;  // just copy in 2nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in        LAST
  
  
     delete   cloneSample;      // *** IMPORTANT,  get rid of  'temp WORKSPACE'
     
     
   //  TRACE(  "...DONE(  HiBOOST-'lo'  FILTER )."  );

	 return  true;
  }



bool   HiBoostMedSndFilter::Filter(  CString&  retErrorMesg   )
  {

     SndSample       *cloneSample;							      //  11 coeffs 
     char    *src, *dst;          				      
     long             centVal,  i,   numByts,      skip= 5L;
    
     
 	retErrorMesg.Empty();

	if(    _Samp  ==   NULL    )    
	{
		ASSERT( 0 );
		retErrorMesg  =  "HiBoostMedSndFilter::Filter failed,  sample is NULL." ;
		return  false;
	}


     numByts =  _Samp->Get_Length();
  
	if(   numByts == 0L )   
	{
		retErrorMesg  =  "HiBoostMedSndFilter::Filter failed,  length is 0." ;
		return  false;
	}




     cloneSample =  _Samp->Clone();       

     if(    cloneSample  ==   NULL   )   
	 {
		retErrorMesg  =  "HiBoostMedSndFilter::Filter failed,  Clone is NULL." ;
		return  false;
	 }



    //TRACE( "Starting   HiBOOST-'lo' FILTER..." );


     src =     (char*)cloneSample->Get_StartByte();   // NOTE:  clone is now the SOURCE, write to  '_Samp' 
     dst =     (char*)_Samp->Get_StartByte();
 
     
     /////////////////////////////////////////
 
     *dst =  *src;           src++;  dst++;    // just copy in FIRST
     *dst =  *src;           src++;  dst++;    // just copy in Second
     *dst =  *src;           src++;  dst++;    // just copy in Third
     *dst =  *src;           src++;  dst++;    // just copy in Second
     *dst =  *src;           src++;  dst++;    // just copy in Third
     
     
     for(  i= skip;   i<  numByts - skip;   i++   )   
         { 
           ///////////////////////       
           
           centVal  =   11L *   ( (long)(  *src )        ); // little TRIM of highs( little low-pass )

           centVal -=    1L *   ( (long)( *(src ) )      );

           centVal -=    1L *   ( (long)( *(src -1L) )   );
           centVal -=    1L *   ( (long)( *(src +1L) )   );

           centVal -=    1L *   ( (long)( *(src -2L) )   );
           centVal -=    1L *   ( (long)( *(src +2L) )   );

           centVal -=    1L *   ( (long)( *(src -3L) )   );
           centVal -=    1L *   ( (long)( *(src +3L) )   );

           centVal -=    1L *   ( (long)( *(src -4L) )   );
           centVal -=    1L *   ( (long)( *(src +4L) )   );

           centVal -=    1L *   ( (long)( *(src -5L) )   );
           centVal -=    1L *   ( (long)( *(src +5L) )   );


          
           centVal  /=   7L;         //   reduce Volume
 
 //   centVal  =   (centVal* 19L)  / 4L;      // 4.23 : ( boost to '144' )
         
         
			/****         
           centVal +=  128L;            // convert back to UNsigned
           
           if(       centVal > 255L  )   *dst =  255;
           else if(  centVal <   0L  )   *dst =    0;
           else                          *dst =  ( unsigned char )centVal;
		   ****/
           if(         centVal  >   127L  )   
			   *dst =     127;
           else if(  centVal  <  -127L  )  
			   *dst =    -127;
           else                                
			   *dst =  ( char )centVal;



           src++;   dst++;     
         }
     
     *dst =  *src;           src++;  dst++;  // just copy in 3nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in 2nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in 3nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in 2nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in        LAST
  
  
     delete   cloneSample;      // *** IMPORTANT,  get rid of  'temp WORKSPACE'
     
     
  //   TRACE(  "...DONE(  HiBOOST-'lo'  FILTER )."  );

	 return  true;
  }




  
 bool   HiBoostHiSndFilter::Filter(  CString&  retErrorMesg   )
  {

     SndSample  *cln;							      //  7 coeffs            
  	 long              centVal,  i,   numByts,      skip= 3L;
     char   *src,  *dst;   
     

 	retErrorMesg.Empty();

	if(    _Samp  ==   NULL    )    
	{
		ASSERT( 0 );
		retErrorMesg  =  "HiBoostHiSndFilter::Filter failed,  sample is NULL." ;
		return  false;
	}



     numByts =  _Samp->Get_Length();

	if(   numByts == 0L )   
	{
		retErrorMesg  =  "HiBoostHiSndFilter::Filter failed,  length is 0." ;
		return  false;
	}
  


     cln  =    _Samp->Clone();     
	 
	 if(    cln  ==   NULL   )   
	 {
		retErrorMesg  =  "HiBoostHiSndFilter::Filter failed,  Clone is NULL." ;
		return  false;
	 }




    // TRACE( "Starting   HiBOOST-'hi' FILTER..." );


     src =     (char*)cln->Get_StartByte();   // NOTE:  clone is now the SOURCE, write to  '_Samp' 

     dst =     (char*)_Samp->Get_StartByte();
 
     
     /////////////////////////////////////////
 
     *dst =  *src;           src++;  dst++;    // just copy in FIRST
     *dst =  *src;           src++;  dst++;    // just copy in Second
     *dst =  *src;           src++;  dst++;    // just copy in Third
     
     
     for(   i= skip;    i<  numByts - skip;     i++     )   
	{ 

           ///////////////////////       
           
           centVal  =    7L *   ( (long)(  *src )        ); // little TRIM of highs( little low-pass )

           centVal -=    1L *   ( (long)( *(src ) )      );

           centVal -=    1L *   ( (long)( *(src -1L) )   );
           centVal -=    1L *   ( (long)( *(src +1L) )   );

           centVal -=    1L *   ( (long)( *(src -2L) )   );
           centVal -=    1L *   ( (long)( *(src +2L) )   );

           centVal -=    1L *   ( (long)( *(src -3L) )   );
           centVal -=    1L *   ( (long)( *(src +3L) )   );


          
           centVal  /=   4L;         //   reduce Volume
 
 //   centVal  =   (centVal* 19L)  / 4L;      // 4.23 : ( boost to '144' )
         
 		
          if(         centVal  >   127L  )   
			   *dst =     127;
           else if(  centVal  <  -127L  )  
			   *dst =    -127;
           else                                
			   *dst =  ( char )centVal;



        src++;   dst++;     
	}
     

     *dst =  *src;           src++;  dst++;  // just copy in 3nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in 2nd to LAST
     *dst =  *src;           src++;  dst++;  // just copy in        LAST
  
  
     delete   cln;      // *** IMPORTANT,  get rid of  'temp WORKSPACE'
     
     
     //TRACE(  "...DONE(  HiBOOST-'hi'  FILTER ).\n"  );

	 return  true;
} 
  



bool    RectifyAndSquareFilter::Filter(    CString&  retErrorMesg   )
{

     SndSample  *cln;							    
  	 long              i,   numByts;
     char   *src,  *dst;   



	 long  volumeDecrease =   128L;         //   30L;   ****** ADJUST *************************
     

	retErrorMesg.Empty();

	if(    _Samp  ==   NULL    )    
	{
		ASSERT( 0 );
		retErrorMesg  =  "RectifyAndSquareFilter::Filter failed,  sample is NULL." ;
		return  false;
	}



	 numByts =  _Samp->Get_Length();

	if(   numByts == 0L )   
	{
		retErrorMesg  =  "RectifyAndSquareFilter::Filter failed,  length is 0." ;
		return  false;
	}



     cln   =     _Samp->Clone();     
	 
	 if(    cln  ==   NULL   )   
	 {
		retErrorMesg  =  "RectifyAndSquareFilter::Filter failed,  Clone is NULL." ;
		return  false;
	 }


    // TRACE( "Starting    RectifyAndSquare  FILTER..." );


     src  =     (char*)cln->Get_StartByte();         //  NOTE:  clone is now the SOURCE,  write modifications to  '_Samp' 
     dst  =     (char*)_Samp->Get_StartByte();
 
     
	/////////////////////////////////////////
       
	long   srcVal,  sqrRectifyVal;

 


	for(     i= 0;     i <  numByts;     i++    )   
	{ 

		srcVal  =     (long)(   (*src)   ); 


		sqrRectifyVal  =    ( srcVal  *   srcVal )   /  volumeDecrease;      //  keep in range  {  -127  to  +127  }    ...chars



		if(     sqrRectifyVal  >=   128L   )  
		{
			int  dummyBreak =  9;

			*dst =  ( char )127;
		}
		else if(   sqrRectifyVal  <  0L    )   
		{
			int  dummyBreak =  9;

			*dst =  ( char )0;
		}
		else
			*dst =  ( char )sqrRectifyVal;


        src++;    dst++;     
	}
     
    

     delete   cln;      // *** IMPORTANT,  get rid of  'temp WORKSPACE'
     
     
    // TRACE(  "...DONE(   RectifyAndSquare  FILTER ).\n"  );

	 return  true;
}

