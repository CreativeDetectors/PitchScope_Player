/////////////////////////////////////////////////////////////////////////////
//
//  FFTslowDown.cpp   -   
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"


#include <math.h>

  


#include   "..\comnFacade\UniEditorAppsGlobals.h"

#include  "..\comnFacade\VoxAppsGlobals.h"



//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"      //  #include <FixMath.h>    ...Mac's version of Fixedpoint 
#include  "..\ComnGrafix\CommonGrafix.h"      //  #include "commonmy.h"   		

#include  "..\ComnGrafix\OffMap.h"  
#include  "..\comnGrafix\TransformMap.h"

#include  "..\ComnGrafix\mapFilters.h"  
//////////////////////////////////////////////////    




#include  "sndSample.h"


#include  "ReSampler.h"




#include "FFTslowDown.h"   


////////////////////////////////////////////////////////////////////////////////////////////////////////

#define M_PI  3.14159265358979323846



#define  DEFAULTsAMPLINGrATEmp3   44100

#define  MP3rEADERbYTESpERsAMPLE  4     //   stereo 16 bit samples






////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


FFTslowDown::FFTslowDown(void)
{


	m_fftFrameSizeSlowDown        =   2048;       //   1024    2048   or   4096  ??      ...almost always        1024[ OK ]       512[ worse ]     128[ ???   ]   


	m_overSampleRatioSlowDown  =       8;        //   8      ...almost always        { 4 -  32 }      8[ good ]      4[ not bad]    3[ ???  ]     2[bad]  



//	 long  fftFrameSize  =    2048;       //       2048[ Best? ]     1024[ OK ]       512[ worse ]     128[ not bad  ]      4096 [  ]    8192 [ good for voice removal ]
//
//	 long  osamp           =       8;        //       { 4  -   32 }      8[ good ]      4[ not bad]            2[bad]  
//
//	stepSize  =     fftFrameSize  /  osamp;       //   This is how many sample will actually get processed by a FFT-transform, considering how the
//																  //   "Overlap and Add"   recudes that ammount of  samples process per FFT-transform



	m_fftCodeType_horizontal =    0;    //    0:  Use Standard OLD code      1:  use New EXPERIMENTAL code for Horz render [*** BAD EXPERIMENT ***]    

	m_allowFixedPointCalcs  =   true;      //   false is DEFAULT.    Another  questionable EXPERIMENTAL Code.  Seems like FixedPoint is too NOISY, and not really that much faster.  1/2012




	m_lessThanOneDivisor  =   16000.0 ;   



	m_areSlowDownBuffersInitialized =  false;


	 m_gRoverLeft  =   m_gRoverRight =  0;

	

	m_playSpeed =   1.0;      //    1:  is Normal speed



	m_outputSampleCountNotSlow =  -1;    //  BIG,  needs to be reassigned when new song is loaded



	m_computerPerformanceFactor =  0;    //     0: Fast     1: Average     2: Slow    


	m_slowDownAlgo =    0;     // default 0        0:  PitchShift with Resampling       1:  NEW  HORIZONTAL  FFT



	m_gInFIFOLeft   =    NULL;
	m_gOutFIFOLeft   =    NULL;
	m_gOutputAccumLeft   =    NULL;

	m_gInFIFORight   =    NULL;
	m_gOutFIFORight   =    NULL;
	m_gOutputAccumRight    =    NULL;

	m_gLastPhaseLeft   =    NULL;
	m_gLastPhaseRight    =    NULL;

	m_gSumPhaseLeft    =    NULL;
	m_gSumPhaseRight    =    NULL;



	m_gAnaFreq   =    NULL;
	m_gAnaMagn   =    NULL;

	m_gSynFreq   =    NULL;
	m_gSynMagn   =    NULL;



	m_gFFTworksp   =    NULL;

	m_realsMasters     =   m_imagMasters      =   NULL; 
	m_realsFixMasters =   m_imagFixMasters  =   NULL;    //  for fixed point calcs



	m_auxInputSamples =  NULL;


	m_leftSlowedDownSamples  =   NULL;
	m_rightSlowedDownSamples    =   NULL;




	m_fftTrForm =  NULL;  

	if(   m_fftCodeType_horizontal  ==   1   )
	{

		ASSERT( 0 );

		/*****   May need to reinstall   8/2012

		m_fftTrForm  =    new     FFTtrForm(        7,      //  bitmaps width         **** ONLY TESTIN so far.    1/2012
																1024,     //    m_fftSize,    
																 DEFAULTsAMPLINGrATEmp3,     //   sampleRate,  
																512     );   //    horzScaleDown    
		ASSERT(  m_fftTrForm   );

		****/
	}

}




					/////////////////////////////////////////////


FFTslowDown::~FFTslowDown(void)
{

	DeAllocate_SlowDown_Buffers();


/****
	if(   m_fftTrForm   !=  NULL  )
	{
		delete  m_fftTrForm;
		m_fftTrForm =   NULL;
	}
*****/
}



					/////////////////////////////////////////////


bool	  FFTslowDown::Process_SpeedChange(    double   newSlowSpeed,     long  computerPerformanceFactor,    CString&   retErrorMesg   )
{

				//   Only CALLED by   WavConvert::Change_PlaySpeed(  


	m_playSpeed  =    newSlowSpeed;      //   Set_SlowDown_Optimizations_this_Speed()   will need this to be updated


	if(     ! Set_SlowDown_Optimizations_this_Speed(    retErrorMesg  )     )
		return  false;
	else
		return  true;
}




					/////////////////////////////////////////////


bool   FFTslowDown::Set_SlowDown_Optimizations_this_Speed(   CString&   retErrorMesg    )
{


	//   CALLED By:    WavConvert::Set_Computer_Performance_Values(),    FFTslowDown::Process_SpeedChange(),    WaveConvert::Initialize_for_Streaming() 


	//   First  SELECT the  'SlowDownAlgo',    then  {  fftFrameSizeSlowDown,   m_overSampleRatioSlowDown }   are set in  Adjust_FFT_Parameters_for_Speed()


	short   preferenceCode =  -1;




	 if(          m_computerPerformanceFactor  ==   0   )       //   0:   FAST                  	(   0: Fast     1: Average     2: Slow   )
	 {

		 preferenceCode =     FFTslowDown::BESTofBOTHcALC;
	 }

	 else if(    m_computerPerformanceFactor  ==   1   )      //   1:   AVERAGE         
	 {

		if(   m_playSpeed  ==  1.5   )
			preferenceCode =    FFTslowDown::HORIZONTALcALC;     //    VERTICAL Calc is the most DEMANDING  for 1.5 Speed     6/2012
		else
			preferenceCode =    FFTslowDown::VERTICALcALC;          //   ...but for very slow speeds  ( 4, 6, 8 ) VERTICAL is the easiest.
	 }

	 else if(    m_computerPerformanceFactor  ==   2   )     //   2:   SLOW  computer              
	 {

		if(   m_playSpeed  ==  1.5   )
			preferenceCode =    FFTslowDown::HORIZONTALcALC;    //   This  1.5 speed works on Lassie    8/30/2012
		else
			preferenceCode =    FFTslowDown::VERTICALcALC;        //   ...but for very slow speeds  ( 4, 6, 8 ) VERTICAL is the easiest.
	 }





	short   newSlowDownAlgoCode =  -1;
	short    playSpeedTimesTen  =     (short)(   m_playSpeed   *  10.0  );   //   switch()  will NOT accept  DOUBLE   



	switch(   preferenceCode   )           
	{

		case  FFTslowDown::VERTICALcALC:    
			newSlowDownAlgoCode =       FFTslowDown::VERTICALcALC;
		break;


		case  FFTslowDown::HORIZONTALcALC: 
			newSlowDownAlgoCode =      FFTslowDown::HORIZONTALcALC;
		break;



		case  FFTslowDown::BESTofBOTHcALC:   

			 switch(   playSpeedTimesTen   )           
			 {

				case  10:    newSlowDownAlgoCode =    FFTslowDown::VERTICALcALC;      //    Keep compiler happy  ,  we should really not hit this
				break;


				case  15:    newSlowDownAlgoCode =     FFTslowDown::HORIZONTALcALC;     //    VERTICAL Calc is the most DEMANDING  for 1.5 Speed     6/2012
				break;

				case  20:    newSlowDownAlgoCode =     FFTslowDown::VERTICALcALC;        
				break;

				case  30:    newSlowDownAlgoCode =     FFTslowDown::VERTICALcALC;        
				break;


				case  40:    newSlowDownAlgoCode =      FFTslowDown::HORIZONTALcALC;         
				break;

				case  60:    newSlowDownAlgoCode =      FFTslowDown::HORIZONTALcALC;     
				break;

				case  80:    newSlowDownAlgoCode =      FFTslowDown::HORIZONTALcALC;     
				break;

				default:      ASSERT( 0 );    
				                 newSlowDownAlgoCode =    FFTslowDown::VERTICALcALC;
				break;
			}
		break;

		default:   
			ASSERT( 0 );   
			retErrorMesg =  "FFTslowDown::Set_SlowDown_Optimizations_this_Speed  FAILED,  missing case." ;
			return  false;
		break;
	}
		


 
	Adjust_FFT_Parameters_for_Speed(   m_playSpeed,    newSlowDownAlgoCode   );     //   Only called here  




	long          sampsInOutputBuffer   =     Get_OutputBuffers_Sample_Count();
	ASSERT(   sampsInOutputBuffer  >  0  );  


	if(     ! Initialize(   m_playSpeed,     newSlowDownAlgoCode,    sampsInOutputBuffer,    retErrorMesg  )     )  //  does NOTHING if the Speed is   1.0
		return  false;
	else
		return  true;
}



					/////////////////////////////////////////////


void	  FFTslowDown::Adjust_FFT_Parameters_for_Speed(    double  newSpeed,    short  newSlowDownAlgoCode   )
{

																//   ONLY CALLED  by   FFTslowDown::Set_SlowDown_Optimizations_this_Speed()


	short    playSpeedTimesTen  =    (short)(   newSpeed  *  10.0  );    //    switch()  will NOT accept  DOUBLE   



	m_slowDownAlgo =   newSlowDownAlgoCode;   //     OK to assign here ???    8/28/2012 




/////////////////////////     0:   FAST
	 if(          m_computerPerformanceFactor  ==   0   )   //   0:  FAST           	
	 {

		m_fftFrameSizeSlowDown  =   2048;       //   1024    2048   or   4096  ??      ...almost always        1024[ OK ]       512[ worse ]     128[ ???   ] 


		if(   m_slowDownAlgo  ==  0    )                   //    0:  VERTICAL       ( PitchShift 'upward' and then ReSampling )  
		{
		   switch(   playSpeedTimesTen   )      
		   {
				case  15:     m_overSampleRatioSlowDown =   8;			break;             //   NOT used                                           [ 4:  sounded bad ]  


				case  20:     m_overSampleRatioSlowDown =   4;	    /* 4; */	 	 break;     // ** used     [  USES  ReSampler  ]

				case  30:     m_overSampleRatioSlowDown =   4; 	/* 6; */	 	 break;    // ** used      [  USES  ReSampler  ]                4[ bad ]   


				case  40:     m_overSampleRatioSlowDown =   8;			break;           //   NOT used
				case  60:     m_overSampleRatioSlowDown =   8;			break;           //   NOT used
				case  80:     m_overSampleRatioSlowDown =   8;			 break;          //   NOT used

				case  10:     m_overSampleRatioSlowDown =                       8;		break;     //  just give it the default
				default:     ASSERT( 0 );    m_overSampleRatioSlowDown =   8; 							            
		  }
		}
		else if(    m_slowDownAlgo  ==  1  )		     //   1:   HORIZINTAL  algo
		{
		   switch(   playSpeedTimesTen   )                
		   {
									//  *** WEIRD:  The vertical 

				case  15:     m_overSampleRatioSlowDown =   4;			 break;        //  ** used    ( does NOT use ReSampler )     6[ best? ]   4[not bad]  8[ gives more ringing ]        1/201


				case  20:     m_overSampleRatioSlowDown =   4;			 break;             //   NOT used     
				case  30:     m_overSampleRatioSlowDown =   4;			 break;             //   NOT used           



				case  40:     m_overSampleRatioSlowDown =   4;    /* 6; */	  break;  // ** used       ( does NOT use ReSampler )              

				case  60:     m_overSampleRatioSlowDown =   4;    /* 6; */	  break;   // ** used       ( does NOT use ReSampler )            

				case  80:     m_overSampleRatioSlowDown =   4;    /* 6; */	  break;   // ** used       ( does NOT use ReSampler )    


				case  10:     m_overSampleRatioSlowDown =                       4;			 break;               //  just give it the default
				default:     ASSERT( 0 );    m_overSampleRatioSlowDown =   4; 			 break;     
		   }
		}
	 }


/////////////////////////     1:   AVERAGE
	 else if(    m_computerPerformanceFactor  ==   1   )      //    1:   AVERAGE
	 {
		m_fftFrameSizeSlowDown   =   2048;       //   1024    2048   or   4096  ??      ...almost always        1024[ OK ]       512[ worse ]     128[ ???   ]   


		if(   m_slowDownAlgo  ==  0    )    //    0:  VERTICAL  
		{

		   switch(   playSpeedTimesTen   )      
		   {

				case  15:     m_overSampleRatioSlowDown =   4;        //   ** NOT used				
								  m_fftFrameSizeSlowDown        =   1024;	
							ASSERT( 0 );					
				break;                                                           //   *****    VERTICAL  1.5 is more demanding that HORIZONTAL  1.5   8/12



				case  20:     m_overSampleRatioSlowDown =   4;			break; 

				case  30:     m_overSampleRatioSlowDown =   6;			 break;        //   4[ bad ]   

				case  40:     m_overSampleRatioSlowDown =    6;    /* 8;	*/		break;

				case  60:     m_overSampleRatioSlowDown =    6;    /* 8;	*/		break;      

				case  80:     m_overSampleRatioSlowDown =    6;    /* 8;	*/		 break;    



				case  10:     m_overSampleRatioSlowDown =                       8;		break;     //  just give it the default
				default:     ASSERT( 0 );    m_overSampleRatioSlowDown =   8; 							            
		  }
		}
		else if(    m_slowDownAlgo  ==  1  )	  //   1:   HORIZINTAL  algo  
		{							
			//                                                                                    ***   HORIZONTAL is the most computationally expensive for speeds 4 - 8 ****

ASSERT(  playSpeedTimesTen  ==  15    );   // *** MOSTLY Verical  for Average and Slow computers,  except  1.5 speed    *********


		   switch(   playSpeedTimesTen   )                
		   {

				case  15:     m_overSampleRatioSlowDown =    4;			//   **  USED

						          m_fftFrameSizeSlowDown   =    1024;									
				break;        


				case  20:     m_overSampleRatioSlowDown =   4;			 break;     //  all these are NOT used    
				case  30:     m_overSampleRatioSlowDown =   4;			 break;               
				case  40:     m_overSampleRatioSlowDown =   4;			 break;               
				case  60:     m_overSampleRatioSlowDown =   4;			 break;               
				case  80:     m_overSampleRatioSlowDown =   4;			 break;             //  still a lot of echo     1/27
				case  10:     m_overSampleRatioSlowDown =                       4;			 break;               //  just give it the default
				default:     ASSERT( 0 );    m_overSampleRatioSlowDown =   4; 			 break;     
		   }
		}
	 }


/////////////////////////     2:   SLOW
	 else if(    m_computerPerformanceFactor  ==   2   )     //     2: SLOW
	 {

		m_fftFrameSizeSlowDown   =   1024;       //   1024    2048   or   4096  ??      ...almost always        1024[ OK ]       512[ worse ]     128[ ???   ]   


		if(   m_slowDownAlgo  ==  0    )    //    0:  VERTICAL  
		{

		   switch(   playSpeedTimesTen   )      
		   {

				case  15:     m_overSampleRatioSlowDown =    4;      break;    // ** NOT  used          //       6    [ 6: fails on Lassie  6/12 ]        [ 8: fails on Lassie  6/12 ]     [ 4:  sounded bad ]  
	
				

				case  20:     m_overSampleRatioSlowDown =   4;			       break; 

				case  30:     m_overSampleRatioSlowDown =   4;     /* 6; */		 break;       

				case  40:     m_overSampleRatioSlowDown =   4;    /* 6; */   /* 8; */	break;     //   ***************  CHANGE   8/28/2012

				case  60:     m_overSampleRatioSlowDown =   4;    /* 6; */   /* 8; */	break;      

				case  80:     m_overSampleRatioSlowDown =   4;    /* 6; */ /* 8; */		break;    


				case  10:     m_overSampleRatioSlowDown =                       8;		break;     //  just give it the default
				default:     ASSERT( 0 );    m_overSampleRatioSlowDown =   8; 							            
		  }
		}

		else if(    m_slowDownAlgo  ==  1  )		     //   1:   HORIZINTAL  algo   [ ***** NOT to use this section  6/1/2012 ************************  ]
		{														         //                                         ***   HORIZONTAL is the most computationally expensive  ****


ASSERT(    playSpeedTimesTen  ==  15  );    // ***  MOSTLY Vertical for Average and Slow computers,  except  1.5 speed


		   switch(   playSpeedTimesTen   )                
		   {

				case  15:     m_overSampleRatioSlowDown =    4;       //   ** USED

					              m_fftFrameSizeSlowDown        =    512	;        //    512 :  Need it that low,  or fails on Lassie
				break;     


				case  20:     m_overSampleRatioSlowDown =   4;			 break;            //  all these are NOT used  
				case  30:     m_overSampleRatioSlowDown =   4;			 break;               
				case  40:     m_overSampleRatioSlowDown =   6;			 break;               
				case  60:     m_overSampleRatioSlowDown =   6;			 break;               
				case  80:     m_overSampleRatioSlowDown =   8;			 break;            
				case  10:     m_overSampleRatioSlowDown =                       4;			 break;               //  just give it the default
				default:     ASSERT( 0 );    m_overSampleRatioSlowDown =   4; 			 break;     
		   }
		}
	 }

	 else
	 {	ASSERT( 0 );   }   //  if(  m_slowDownAlgo >  1 



//	TRACE(   "\n SLOW-DOWN Speed parms have changed.   [  osamp  %d  ]       [  FrameSize  %d  ]   \n",   m_overSampleRatioSlowDown,   m_fftFrameSizeSlowDown   );
}




					/////////////////////////////////////////////


long     FFTslowDown::Get_OutputBuffers_Sample_Count()
{

	long  sampleCount =  -1;


	if(    m_outputSampleCountNotSlow  <  0     )
	{
		ASSERT( 0 );     //   this should have been SET for the SONG in    WavConvert::Initialize_for_Streaming()
		return  -1;
	}

	           sampleCount =    m_outputSampleCountNotSlow;
	return  sampleCount;
}




					/////////////////////////////////////////////


bool     FFTslowDown::Initialize(   double  playSpeed,   short  slowDownAlgo,    long  outputBufferSampleCountNotSlowDown,     CString&   retErrorMesg   )
{


		                           //   Only CALLED BY:    Set_SlowDown_Optimizations_this_Speed()


	int     outputFreq =    DEFAULTsAMPLINGrATEmp3;    

	int     inputFreq   =  (int)(    (double)DEFAULTsAMPLINGrATEmp3   /  playSpeed   );    // *****   BAD for half speeds

	long	 bytesPerSample  =  MP3rEADERbYTESpERsAMPLE;   // **** ALWAYS ????   12/11




//  Initialize_SlowDown_Variables()    ...Would this help get rid of the static sound when swith speeds ???     12/11

	DeAllocate_SlowDown_Buffers();



	m_slowDownAlgo =    slowDownAlgo;   

	m_playSpeed      =     playSpeed;



	if(   playSpeed  ==  1.0   )
	{
		return  true;    // ***********   ESCAPE, we do NOT need buffers if at Speed 1     12/11
	}
	///////////////////////////////////////////////////////////////




	m_gRoverLeft  =   m_gRoverRight =  0;   // **** If I do this early,  do I stop the 'static' sound when swithching speeds ???   12/11



	if(    ! Alloc_SlowDown_Buffers(   playSpeed,    slowDownAlgo,   outputBufferSampleCountNotSlowDown,    retErrorMesg   )     )  
		return  false;					



	long   retDstStreamBufferSizeInSamples =  -1;    
	long   allocateSizeInputArrays =  -1;

	if(         slowDownAlgo  ==  0   )
		allocateSizeInputArrays =          sizeof(float)  *                              outputBufferSampleCountNotSlowDown;   
	else if(   slowDownAlgo  ==  1   )
		allocateSizeInputArrays =         sizeof(float)   *    (long)(    (double)outputBufferSampleCountNotSlowDown    * m_playSpeed  );



	if(     slowDownAlgo  ==  0   )    //    0:   VERTICAL algo      It uses alawys uses the  ReSampler   
	{
		if(   ! m_reSamplerSlowDown.Initialize(   inputFreq,   outputFreq,     allocateSizeInputArrays,      retDstStreamBufferSizeInSamples,  
																																 bytesPerSample,    true,    retErrorMesg   )     )
			return  false;
	}


	return  true;
}




					/////////////////////////////////////////////


bool     FFTslowDown::Alloc_SlowDown_Buffers(   double  newPlaySpeed,   short  slowDownAlgo,    long  outputBufferSampleCountNotSlowDown,   CString&   retErrorMesg  )
{

			//   Only CALLED by:     Initialize_for_Streaming()          FFTslowDown::Apply_Phase_Filtering_wFFT()


	if(   outputBufferSampleCountNotSlowDown  < 0  )
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers FAILED,  outputBufferSampleCountNotSlowDown has bad value." ;
		return  false;
	}



	long   allocateSizeInputArrays =  0;


	if(         slowDownAlgo  ==  0   )
		allocateSizeInputArrays =          sizeof(float)  *  outputBufferSampleCountNotSlowDown;   
	else if(   slowDownAlgo  ==  1   )
		allocateSizeInputArrays =         sizeof(float)   *    (long)(    (double)outputBufferSampleCountNotSlowDown   *  m_playSpeed  );


  

	double   multiplier =   Calc_Buffers_Multiplier(   slowDownAlgo,   newPlaySpeed  );

	ASSERT(    slowDownAlgo  ==   m_slowDownAlgo    );    // ***** NEW,  8/30/12,   HITS: 





	long   fftFrameSize  =     m_fftFrameSizeSlowDown;

	long   sizeOfVertialComponents  =    fftFrameSize /2   +  1;    //   OR    fftFrameSize   ??? 



//	long   maxFrameLengthMultiplied  =     MAX_FRAME_LENGTH     * multiplier;       //   MAX_FRAME_LENGTH  is  8192

	long   frameLengthMultiplied         =    (long)(    (double)fftFrameSize   *  multiplier    );   




	DeAllocate_SlowDown_Buffers();    




	TRACE(  "\n\n\nAlloc_SlowDown_Buffers  [  fftFrameSize  %d     Mult  %d   speed  %.1f ]      [  allocateSizeInputArrays  %d  ]     \n",   
		                                                              fftFrameSize,    (long)multiplier,     newPlaySpeed,       allocateSizeInputArrays      );




	////////////////////////////////////////////

	if(     (    m_leftSlowedDownSamples  =    new    float[  allocateSizeInputArrays ]    )   == NULL    )  
	{
		retErrorMesg =    "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_leftSlowedDownSamples."  ;
		return  false;
	}

	if(     (   m_rightSlowedDownSamples  =    new    float[  allocateSizeInputArrays  ]    )   == NULL    )  
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_rightSlowedDownSamples."  ;
		return false;
	}




	if(     (   m_auxInputSamples  =    new    float[  allocateSizeInputArrays  ]    )   == NULL    )  
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_auxInputSamples."  ;
		return false;
	}

	
	
									/////////////////////////////
									/////////////////////////////


	if(     (    m_gFFTworksp   =       new    float[     2 *  fftFrameSize     ]    )   == NULL    )  
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_gFFTworksp."  ;
		return false;
	}




	if(     (    m_realsMasters   =       new    float[    fftFrameSize     ]    )   == NULL    )   // ******* SIZE OK ???  *************
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_realsMasters."  ;
		return false;
	}


	if(     (    m_imagMasters   =       new    float[    fftFrameSize     ]    )   == NULL    )    // ******* SIZE OK ???  *************
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_imagMasters."  ;
		return false;
	}




	if(    m_allowFixedPointCalcs    )
	{

		if(     (    m_realsFixMasters   =       new    long[    fftFrameSize     ]    )   == NULL    )   // ******* SIZE OK ???  *************
		{
			retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_realsFixMasters."  ;
			return false;
		}
							//  NOTE:   These 2 are LONGs   because of fixed point   

		if(     (    m_imagFixMasters   =       new    long[    fftFrameSize     ]    )   == NULL    )    // ******* SIZE OK ???  *************
		{
			retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_imagFixMasters."  ;
			return false;
		}
	}



									/////////////////////////////

	   
	if(     (    m_gInFIFOLeft   =       new    float[    fftFrameSize  ]    )   == NULL    )  
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_gInFIFOLeft."  ;
		return false;
	}

	   
	if(     (    m_gOutFIFOLeft   =       new    float[    frameLengthMultiplied   ]    )   == NULL    )  
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_gOutFIFOLeft."  ;
		return false;
	}

	   


	if(     (    m_gOutputAccumLeft   =       new    float[    2 * fftFrameSize   ]    )   == NULL    )  
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_gOutputAccumLeft."  ;
		return false;
	}



	   
	if(     (    m_gInFIFORight   =       new    float[    fftFrameSize  ]    )   == NULL    )  
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_auxInputSamples."  ;
		return false;
	}

	   
	if(     (    m_gOutFIFORight   =       new    float[   frameLengthMultiplied  ]    )   == NULL    )  
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_gInFIFORight."  ;
		return false;
	}

	  
	if(     (     m_gOutputAccumRight   =       new    float[    2 * fftFrameSize   ]   )   == NULL    )  
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_gOutputAccumRight."  ;
		return false;
	}




	     
	if(     (    m_gLastPhaseLeft   =       new    float[    sizeOfVertialComponents   ]    )   == NULL    )  
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_gLastPhaseLeft."  ;
		return false;
	}
	   
	if(     (    m_gLastPhaseRight   =       new    float[    sizeOfVertialComponents   ]    )   == NULL    )  
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_gLastPhaseRight."  ;
		return false;
	}



	   
	if(     (   m_gSumPhaseLeft   =       new    float[   sizeOfVertialComponents   ]     )   == NULL    )  
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_gSumPhaseLeft."  ;
		return false;
	}

	   
	if(     (    m_gSumPhaseRight   =       new    float[   sizeOfVertialComponents   ]    )   == NULL    )  
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_gSumPhaseRight."  ;
		return false;
	}



					//////////////////////////////

	   
	if(     (    m_gAnaFreq  =       new    float[    sizeOfVertialComponents    ]    )   == NULL    )  
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_gAnaFreq."  ;
		return false;
	}
	   
	if(     (    m_gAnaMagn   =       new    float[   sizeOfVertialComponents     ]    )   == NULL    )  
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_gAnaMagn."  ;
		return false;
	}


	   
	if(     (    m_gSynFreq   =       new    float[   sizeOfVertialComponents     ]    )   == NULL    )  
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_gSynFreq."  ;
		return false;
	}
	  
	if(     (    m_gSynMagn   =       new    float[   sizeOfVertialComponents     ]     )   == NULL    )  
	{
		retErrorMesg =  "FFTslowDown::Alloc_SlowDown_Buffers  failed,  could not allocate  m_gSynMagn."  ;
		return false;
	}



	Initialize_SlowDown_Variables();


	return  true;
}




					/////////////////////////////////////////////


void	  FFTslowDown::Erase_OutputAccumulators()
{


//	CALLED BY:    WavConvert::Initialize_for_Play()


		//   ******  Keep in sync with   Erase_SlowDown_Buffers()  *******************
	
	if(     m_gOutputAccumLeft  !=  NULL   )
		memset(  m_gOutputAccumLeft,    0,       2 *  m_fftFrameSizeSlowDown  *  sizeof(float)    );

	if(     m_gOutputAccumRight  !=  NULL   )
		memset(  m_gOutputAccumRight,  0,       2 *  m_fftFrameSizeSlowDown  *  sizeof(float)     );


//	TRACE(  "Erase_Output ACCUMULATORS    [ fftFrameSize   %d  ] \n",    m_fftFrameSizeSlowDown    ); 
}




				/////////////////////////////////////////////


double    FFTslowDown::Calc_Buffers_Multiplier(   short  slowDownAlgo,    double  playSpeed    )
{


	double   multiplier  =   0.0;



	if(   slowDownAlgo  ==  0   )                    //   0:   VERTICAL
	{
		multiplier  =    1.0;
	}
	else  if(   slowDownAlgo  ==  1   )            //   1:   HORIZONTAL
	{

		if(   m_playSpeed  ==  1.5   )     //  Decimate by 2 (every other sample),   and then expand Horizontally 3x      *****   HARDWIRED  ********
			multiplier  =    3.0;                
		else
			multiplier  =    playSpeed;

	}
	else
	{	ASSERT( 0 );
		multiplier  =    1.0;   
	}


	return  multiplier;
}



				/////////////////////////////////////////////


void     FFTslowDown::Erase_SlowDown_Buffers()
{

			                 //   ***  Keep in sync with   Erase_OutputAccumulators()  ***



	double   multiplier =    Calc_Buffers_Multiplier(   m_slowDownAlgo,   m_playSpeed   );


	long   fftFrameSize                    =    m_fftFrameSizeSlowDown;

	long   sizeOfVertialComponents  =    fftFrameSize /2   + 1;  


	long   frameLengthMultiplied      =    (long)(    (double)fftFrameSize   *  multiplier   );    



	
//	TRACE(  "    Erase_SlowDown_BUFFERS  [   fftFrameSize  %d      mult  %d  ]     [  frameLengthMultiplied  %d  ]    \n",       fftFrameSize,    (long)multiplier,     frameLengthMultiplied   );



	if(    m_gInFIFOLeft  !=  NULL   )
	{
		memset(  m_gInFIFOLeft,            0,    fftFrameSize  * sizeof(float)     );
		memset(  m_gInFIFORight,          0,   fftFrameSize  * sizeof(float)     );
	}


	if(    m_gOutFIFOLeft  !=  NULL   )
	{
		memset(  m_gOutFIFOLeft,         0,        frameLengthMultiplied   *  sizeof(float)     );
		memset(  m_gOutFIFORight,       0,        frameLengthMultiplied   *  sizeof(float)      );
	}



	/***
	if(    m_gOutputAccumLeft  !=  NULL   )
	{
		memset(  m_gOutputAccumLeft,    0,      2 *  fftFrameSize  * sizeof(float)    );
		memset(  m_gOutputAccumRight,  0,      2 * fftFrameSize  * sizeof(float)     );
	}
	***/
	Erase_OutputAccumulators();




	if(    m_gLastPhaseLeft  !=  NULL   )
	{
		memset(  m_gLastPhaseLeft,          0,     (  fftFrameSize/2   +1)  * sizeof(float)     );
		memset(  m_gLastPhaseRight,        0,     (  fftFrameSize/2   +1)  * sizeof(float)     );

		memset(  m_gSumPhaseLeft,         0,      (  fftFrameSize/2  +1)  * sizeof(float)      );
		memset(  m_gSumPhaseRight,       0,      (  fftFrameSize/2  +1)  * sizeof(float)      );
	}




	if(    m_gFFTworksp  !=  NULL   )
		memset(    m_gFFTworksp,            0,       2 * fftFrameSize * sizeof(float)     );



	//   m_auxInputSamples;     **************   THIS TOO ??? ******************************
}






					/////////////////////////////////////////////


void     FFTslowDown::DeAllocate_SlowDown_Buffers()
{


	m_areSlowDownBuffersInitialized =   false;



	if(    m_gInFIFOLeft  !=   NULL   )
	{
		delete   m_gInFIFOLeft;
		m_gInFIFOLeft =  NULL;
	}

	if(    m_gOutFIFOLeft  !=   NULL   )
	{
		delete   m_gOutFIFOLeft;
		m_gOutFIFOLeft =  NULL;
	}

	if(   m_gOutputAccumLeft   !=   NULL   )
	{
		delete   m_gOutputAccumLeft;
		m_gOutputAccumLeft =  NULL;
	}



	if(   m_gInFIFORight   !=   NULL   )
	{
		delete   m_gInFIFORight;
		m_gInFIFORight =  NULL;
	}

	if(   m_gOutFIFORight   !=   NULL   )
	{
		delete   m_gOutFIFORight;
		m_gOutFIFORight =  NULL;
	}

	if(    m_gOutputAccumRight  !=   NULL   )
	{
		delete   m_gOutputAccumRight;
		m_gOutputAccumRight =  NULL;
	}



	if(    m_gLastPhaseLeft  !=   NULL   )
	{
		delete   m_gLastPhaseLeft;
		m_gLastPhaseLeft =  NULL;
	}

	if(   m_gLastPhaseRight   !=   NULL   )
	{
		delete   m_gLastPhaseRight;
		m_gLastPhaseRight =  NULL;
	}



	if(   m_gSumPhaseLeft   !=   NULL   )
	{
		delete   m_gSumPhaseLeft;
		m_gSumPhaseLeft =  NULL;
	}

	if(    m_gSumPhaseRight  !=   NULL   )
	{
		delete   m_gSumPhaseRight;
		m_gSumPhaseRight =  NULL;
	}



					////////////////////////////////////////////////

	if(   m_gAnaFreq   !=   NULL   )
	{
		delete   m_gAnaFreq;
		m_gAnaFreq =  NULL;
	}

	if(  m_gAnaMagn    !=   NULL   )
	{
		delete   m_gAnaMagn;
		m_gAnaMagn =  NULL;
	}


	if(  m_gSynFreq    !=   NULL   )
	{
		delete   m_gSynFreq;
		m_gSynFreq =  NULL;
	}

	if(    m_gSynMagn  !=   NULL   )
	{
		delete   m_gSynMagn;
		m_gSynMagn =  NULL;
	}



	if(   m_gFFTworksp   !=   NULL   )
	{
		delete   m_gFFTworksp;
		m_gFFTworksp =  NULL;
	}



	if(   m_realsMasters   !=   NULL   )
	{
		delete   m_realsMasters;
		m_realsMasters =  NULL;
	}

	if(   m_imagMasters   !=   NULL   )
	{
		delete   m_imagMasters;
		m_imagMasters =  NULL;
	}



	if(   m_realsFixMasters   !=   NULL   )    //  Fixed point  
	{
		delete   m_realsFixMasters;
		m_realsFixMasters =  NULL;
	}

	if(   m_imagFixMasters   !=   NULL   )
	{
		delete   m_imagFixMasters;
		m_imagFixMasters =  NULL;
	}




	if(    m_leftSlowedDownSamples  !=   NULL    )
	{
		delete   m_leftSlowedDownSamples;
		m_leftSlowedDownSamples =  NULL;
	}

	if(    m_rightSlowedDownSamples  !=   NULL    )
	{
		delete   m_rightSlowedDownSamples;
		m_rightSlowedDownSamples =  NULL;
	}



	if(    m_auxInputSamples  !=   NULL    )
	{
		delete   m_auxInputSamples;
		m_auxInputSamples =  NULL;
	}
}




							/////////////////////////////////////////////////////////////////


void    FFTslowDown::Initialize_SlowDown_Variables()
{

	    //   m_fftFrameSizeSlowDown  &  m_overSampleRatioSlowDown   must be properly assigned before calling this function.


	ASSERT(   m_leftSlowedDownSamples   );
	
	ASSERT(   m_gFFTworksp   );



	long   stepSize         =    m_fftFrameSizeSlowDown  /  m_overSampleRatioSlowDown;    //  2048,   256      as osamp  gets bigger,    stepSize gets smaller

	long   inFifoLatency   =    m_fftFrameSizeSlowDown  -  stepSize;   //   1792  for   {  2048 }



	Erase_SlowDown_Buffers();   




	m_gRoverLeft  =   m_gRoverRight =    inFifoLatency;


	if(         m_fftCodeType_horizontal  ==  1  
		 &&   m_slowDownAlgo  ==  1     )
	{
		m_gRoverLeft  =   m_gRoverRight =    0;
	}
	


	m_areSlowDownBuffersInitialized =    true;      //  this is a little controversial,  but seems to work OK	
}




						/////////////////////////////////////////////////////////////////
						////////////////    new  FFT  slowdown   /////////////////
						/////////////////////////////////////////////////////////////////


bool     FFTslowDown::Shift_Pitch(  bool  rightStereo,   float pitchShift,     long numSampsToProcess,    long fftFrameSize,     long osamp,   float sampleRate,
																	                    float *indata,   float *outdata,      CString&   retErrorMesg  )   
{


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

//	sampleRate -    takes the sample rate for the signal  in unit Hz,   ie. 44100  for  44.1 kHz  audio. 


//	The data passed to the routine in  indata[] should be in the range [ -1.0, 1.0 ],  which is also the output range 
 //	for the data, make sure you scale the data accordingly ( for 16bit signed integers you would have to divide (and multiply) by 32768 ). 


	retErrorMesg.Empty();


	if(   m_gInFIFOLeft  ==  NULL  )
	{
		retErrorMesg  =     "FFTslowDown::Shift_Pitch  FAILED,   SlowDown BUFFERS were NOT allocated."  ;
		return   false;
	}

//	m_realsMasters,    m_imagMasters


	if(   m_realsMasters  ==  NULL    ||    m_imagMasters ==  NULL    )
	{
		retErrorMesg  =     "FFTslowDown::Shift_Pitch  FAILED,   m_realsMasters  and m_imagMasters   were NOT allocated."  ;
		return   false;
	}




//	static long  gRoverLeft   =   0;   //  do i NOT need this initialization??  will it create trouble ????
//	static long  gRoverRight =   0;

	long      gRover = 0;



	double   magn, phase, window, real, imag;
	double   freqPerBin, expct;
	long      i,k,  inFifoLatency, stepSize, fftFrameSize2;
//	long    index;


																		// set up some handy variables
	fftFrameSize2 =    fftFrameSize /2;

	stepSize         =    fftFrameSize  /  osamp;       //   as osamp  gets bigger,    stepSize gets smaller

	freqPerBin       =   sampleRate  /  (double)fftFrameSize;

	expct              =    2.*M_PI * (double)stepSize / (double)fftFrameSize;

	inFifoLatency   =    fftFrameSize  -  stepSize;


	
	long   sizeOfVertialComponents  =    fftFrameSize /2   +  1;    //   OR    fftFrameSize   ??? 



																	// initialize our static arrays 
	if(   m_areSlowDownBuffersInitialized  ==   false   )
	{
		Initialize_SlowDown_Variables();
	}





//	memset(    m_gFFTworksp,    0,      2 * fftFrameSize             * sizeof(float)    ); 
	memset(    m_realsMasters,    0,          fftFrameSize             * sizeof(float)    ); 
	memset(    m_imagMasters,    0,          fftFrameSize             * sizeof(float)    ); 




	float    *gInFIFO=NULL,   *gOutFIFO=NULL,  *gOutputAccum=NULL,     *gLastPhase=NULL,   *gSumPhase=NULL; 

	if(   rightStereo   )     //  save values for next entrance to this function,  or we will hear 'static'  from variable confusion from switching from left to right stereo
	{
		gInFIFO       =     m_gInFIFORight;
		gOutFIFO      =    m_gOutFIFORight;
		gOutputAccum =    m_gOutputAccumRight;  

		gLastPhase  =      m_gLastPhaseRight;
		gSumPhase =      m_gSumPhaseRight;

		gRover =   m_gRoverRight;
	}
	else
	{  gInFIFO      =     m_gInFIFOLeft;
		gOutFIFO    =     m_gOutFIFOLeft;
		gOutputAccum =    m_gOutputAccumLeft;  

		gLastPhase  =      m_gLastPhaseLeft;
		gSumPhase =      m_gSumPhaseLeft;

		gRover =   m_gRoverLeft;
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



//			memset(  m_gFFTworksp,    0,   2 * fftFrameSize  *  sizeof(float)  );
			memset(    m_realsMasters,    0,          fftFrameSize             * sizeof(float)    ); 
			memset(    m_imagMasters,    0,          fftFrameSize             * sizeof(float)    ); 





/////////////////////////////////
																			//  do windowing and input the data to the Real and Imaginary arrays 

			for(  k = 0;    k < fftFrameSize;    k++  )       
			{

//				window  =    -.5 * cos(   2.*M_PI * (double)k   /  (double)fftFrameSize   ) + .5;     //  Yes,  need windowing.  It also stops Frequency Leaks across Bins.  1/12
	

																// Welch Window from Masters  pp. 93   (  He says it will discourage Frequency-Leaking across Bins.  1/12  )

		//		window  =   1.0  -  (           (   (double)k   - 0.5 *  (  fftFrameSize - 1)   )    /   (  0.5*(  fftFrameSize + 1)   )
		//										   *   (   (double)k   - 0.5 *  (  fftFrameSize - 1)   )    /   (  0.5*(  fftFrameSize + 1)   )      );   
																
				window  =   1.0  -  (           (   (double)k  -  0.5 *  (double)(fftFrameSize - 1)   )    /    (  0.5 *  (double)(fftFrameSize + 1)   )
												   *   (   (double)k  -  0.5 *  (double)(fftFrameSize - 1)   )    /    (  0.5 *  (double)(fftFrameSize + 1)   )       );   
										


			//	m_gFFTworksp[ 2*k     ]   =     gInFIFO[ k ]   * window;    //   FFT  inputs  are take  from the   gInFIFO[]   circularQue-delay
			//	m_gFFTworksp[ 2*k +1]   =     0.0;


									//   Because we take advantage of ONLY Reals for Input,   pack BOTH  m_realsMasters[],  m_imagMasters[]  with signal's data.

				if(   (k % 2)  ==  0   )
					m_realsMasters[  k/2  ]   =     gInFIFO[ k ]   * window;      //    Even  terms in real part and... 
				else
					m_imagMasters[  k/2  ]   =     gInFIFO[ k ]   * window;      //    ...Odd  terms  in imaginary part. 
			}

			
			
																										//   ****************** ANALYSIS *******************
																	
		//	smbFft(   m_gFFTworksp,      fftFrameSize,        -1   );  
			SndSample::FFT_with_only_Real_Input(    fftFrameSize/2,    m_realsMasters,  m_imagMasters   );    //  Faster, uses half as many calulations because of only Reals for Input.   
																												                                             //   from Book by Timithothy Masters 
/////////////////////////////////





			memset(  m_gAnaFreq,          0,   sizeOfVertialComponents  * sizeof(float)  );
			memset(  m_gAnaMagn,         0,   sizeOfVertialComponents  * sizeof(float) );

			
																								
			for(   k = 0;    k <=   fftFrameSize2;    k++   )    // this is the analysis step 
			{				

/////////////////////////////////
				/***
				real   =   m_gFFTworksp[  2*k       ];			// de-interlace FFT buffer 
				imag =   m_gFFTworksp[  2*k  +1  ];
				****/
				real   =     m_realsMasters[  k  ];			
				imag  =     m_imagMasters[  k  ];
/////////////////////////////////				


				magn =    2. *  sqrt(  real*real  +   imag*imag  );    // compute magnitude and phase 
				phase =   atan2(  imag, real );


				double   lastPhase =    gLastPhase[ k ];
				
				gLastPhase[ k ] =   phase;    //  save the  calculated-phase  for next iteration



					      //  Looks like they use the   DEVIATION in the EXPECTED-Phase   to calculate the  TRUE-Frequency  that is present in the Bin.     12/11

				double   trueFreq  =     Calculate_True_Frequency(    phase,    lastPhase,    k,    freqPerBin,    expct,    osamp  );

		//		double   approxFreq =        (double)k     *  freqPerBin;      ** DEBUG only 


				m_gAnaMagn[ k ]  =    magn;       //  store magnitude and true frequency in analysis arrays
				m_gAnaFreq[  k ]  =    trueFreq;  
			}



																								//    PROCESSING the PitchShift,  by moving values to other bins. 
		
			memset(  m_gSynMagn,  0,   sizeOfVertialComponents * sizeof(float)   );    
			memset(  m_gSynFreq,    0,   sizeOfVertialComponents * sizeof(float)   );

			/*****
			for(    k = 0;    k  <=  fftFrameSize2;    k++  )       //   this does the actual pitch shifting 
			{ 
				double   indexFlt  =     (double)k   *  (double)pitchShift;		//  going to VERTICALLY  shift the  Mag and Freq  values  to other   FFT-frequencyRows, changes the pitch
				long      indexInt  =    (long)indexFlt;

				bool   skipThisBin =  false;  // We may get better result without  EXTRAPOLATING  when 'index' is not a good integer fit   1/12
		//		bool   skipThisBin =  true; 
				double  remainder    =     indexFlt    -   (double)indexInt;
				if(        remainder  >=  0.5  )
				{
			//		indexInt++;
			//		skipThisBin =  true;
			//		skipThisBin =  false;     //   Only do one with a remainder
				}				
				if(   ! skipThisBin   )
				{
			//		if(   indexInt   <=   fftFrameSize2   ) 
					if(   indexInt   <     fftFrameSize2   )    // ********* WHICH is right ???   12/20/11   **************************
					{ 
						m_gSynMagn[   indexInt   ]    +=      m_gAnaMagn[ k ]; 
						m_gSynFreq[    indexInt   ]       =     m_gAnaFreq[   k ]     * pitchShift; 
					} 		
				}  
			}
			***/

			short   opCode =   0;

			FFTslowDown::Shift_Frequency_Arrays_for_ReConstruction(   pitchShift,    fftFrameSize,    m_gAnaMagn,   m_gAnaFreq,      
														                                                                                      m_gSynMagn,   m_gSynFreq,    opCode   );


			

																															//    SYNTHESIS 

			memset(    m_realsMasters,    0,          fftFrameSize   * sizeof(float)    );  
			memset(    m_imagMasters,    0,          fftFrameSize   * sizeof(float)    ); 

			
			double  wt  =   2.0;                               // Real and imaginary parts WOULD have reinforced (so simulate with 2x )

//			wt   /=    (double)fftFrameSize;  //   Masters:   "Must scale down by 'n', before or after transform"  
														   //   BUT here we do it AFTER so can apply 'osamp'  ...
														   //                  ...SEE Below:   m_realsMasters[  k  ]      *    2.0 * window   /  ( fftFrameSize2 * osamp ); 




	//		for(    k = 0;     k <=   fftFrameSize2;      k++   ) 
			for(    k = 1;     k <     fftFrameSize2;      k++   )    //  From  Masters. 
			{
																
				magn =    m_gSynMagn[ k ];      // get magnitude and true frequency from synthesis arrays 

				double   trueFreq  =    m_gSynFreq[  k ];


					         //   Looks like the   DEVIATION in the Bin's Frequency  is used to calulate a corrected Phase-Difference between FFT frames.   12/11

				double   deltaPhase  =    Calculate_True_DeltaPhase(   trueFreq,    k,    freqPerBin,    expct,    osamp   );   //   'deltaPhase'  is in RADIANS

														
				gSumPhase[ k ]   +=    deltaPhase;     //  accumulate  delta-phase  to get  bin's phase 

				phase =    gSumPhase[ k ];


/////////////////////////////////		
				/****
				m_gFFTworksp[ 2 * k      ]   =    magn   *   cos( phase );     // get real and imag part and re-interleave
				m_gFFTworksp[ 2 * k +1 ]   =    magn   *   sin( phase );
				****/

																//   Data is in RIGHT half of array,  and LEFT half was ZEROED-out( to stop wrap-arround )

				m_realsMasters[   fftFrameSize  - k   ]  =       wt      *   magn   *   cos( phase );  // ******** PROBLEM???   should  "-1"  be in this ?? **************


			//	m_imagMasters[   fftFrameSize  - k   ]  =      -wt      *   magn   *   sin( phase );   // sign change(-) cause this is now at UPPER end 
				m_imagMasters[   fftFrameSize  - k   ]  =       wt      *   magn   *   sin( phase );   // Avoid the  "DUPLICATE Sign Flip"   below
/////////////////////////////////
			} 




/////////////////////////////////
			/*****																			// zero negative frequencies 
			for(   k =  fftFrameSize +2;     k < (2 * fftFrameSize);       k++ ) 
				m_gFFTworksp[ k ] =   0.0;
																						
			smbFft(   m_gFFTworksp,   fftFrameSize,   1   );     // **   do   INVERSE  FFT  **   
			****/

			m_realsMasters[0] =  m_imagMasters[0]  =  m_realsMasters[  fftFrameSize2  ] =  m_imagMasters[  fftFrameSize2  ] =   0.0;     // Also zero these


		/*******************************************************	Avoid this  "DUPLICATE Sign Flip"  by using a positve  'wt'  value up above													         				
		   for(   k= 0;    k <  fftFrameSize;    k++  )     //   YES,  we need this or the sound is bad.     1/12
				m_imagMasters[ k ] =   -1.0  *  m_imagMasters[ k ];						 //  '-' FLIP SIGN of imaginary part
		****/


			SndSample::FFT_Standard(    fftFrameSize,    m_realsMasters,   m_imagMasters   );   
   


			for(  k=0;    k< fftFrameSize;  k++ )   // Not really necessary because we do NOT use Imaginary array after this, but leav it in for clarity of the algo.
			{  
				m_imagMasters[ k ]  =    -1.0  *  m_imagMasters[ k ];         // '-'  to FLIP SIGN of imaginary part BACK    (  ???  Not really necessary 
			}
	


/////////////////////////////////
																//    do windowing and 'ADD'  to output accumulator ( this is the 'ADD' part to  "OverlapAndAdd"  ) 

			for(  k =0;   k < fftFrameSize;   k++  ) 
			{																//  Yes, we need windowon.

		//		window  =      -0.5  *  cos(  2.0 * M_PI * (double)k  /  (double)fftFrameSize  )   + 0.5;   //  Sames as above

				window  =   1.0  -  (           (   (double)k  -  0.5 *  (double)(fftFrameSize - 1)   )    /    (  0.5  *  (double)(fftFrameSize + 1)   )
												   *   (   (double)k  -  0.5 *  (double)(fftFrameSize - 1)   )    /    (  0.5  *  (double)(fftFrameSize + 1)   )      );    // Welch Window



/////////////////////////////////
//				gOutputAccum[  k  ]   +=     m_gFFTworksp[  2*k  ]    *    2.0 * window   /  ( fftFrameSize2 * osamp );   

		//		gOutputAccum[  k  ]  +=      m_realsMasters[  k  ]      *    2.0 * window   /  ( fftFrameSize2 * osamp );   // in Masters this was done ABOVE by:   wt  /=  fftFrameSize
				gOutputAccum[  k  ]  +=      m_realsMasters[  k  ]      *              window   /  ( fftFrameSize2 * osamp );   // in Masters this was done ABOVE by:   wt  /=  fftFrameSize
/////////////////////////////////
			}





			for (  k = 0;     k < stepSize;    k++  )             //  Read the left part of the Accumulator's data for  Output-RESULTS
				gOutFIFO[ k ]  =    gOutputAccum[ k ];

																														
			memmove(   gOutputAccum,    gOutputAccum + stepSize,     fftFrameSize  *  sizeof(float)   );    //  shift the Accumulator's data to the left 
			

																				// move input FIFO 
			for( k = 0;   k < inFifoLatency;   k++ )      
				gInFIFO[ k ]  =     gInFIFO[  k + stepSize  ];
		}
	}



	
	if(   rightStereo   )				         //   save values for next entrance to this function,  or we will hear audio 'static'  from variable confusion 
		m_gRoverRight =     gRover;       //   from switching from left to right stereo during realtime play
	else
		m_gRoverLeft   =     gRover;

	return   true;
}



							/////////////////////////////////////////////////////////////////


bool     FFTslowDown::Shift_Pitch_Horizontally(  bool  rightStereo,   float pitchShift,     long numSampsToProcess,    long fftFrameSize,     
											                                                    long osamp,   float sampleRate,      float *indata,   float *outdata,      
																					float  binsPitchShift,   	 long  decimationRatio,   long  outputRepetitionCount,   CString&   retErrorMesg  )   
{


// 	                   Try to figure out exactly WHAT this FUNCTION DOES and think how I could
//				                it's techniques to do other manipulation:    (check old experimental sound manupulation techniques)
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

//	sampleRate -    takes the sample rate for the signal  in unit Hz,   ie. 44100  for  44.1 kHz  audio. 


//	The data passed to the routine in  indata[] should be in the range [ -1.0, 1.0 ],  which is also the output range 
 //	for the data, make sure you scale the data accordingly ( for 16bit signed integers you would have to divide (and multiply) by 32768 ). 




	bool   useFixedPointCalcs =     false;   //  false     **************   NEW SWITCH ******************

	if(      useFixedPointCalcs    )
	{
		ASSERT(  m_allowFixedPointCalcs ==  true   );   //   Must also set this to use the controversial funct,  FFT_FixedPoint_Standard()

// ************************   For RELEASE,  set    m_allowFixedPointCalcs = false
	}



	retErrorMesg.Empty();  

	ASSERT(   decimationRatio  >=  1   );



	if(   m_gInFIFOLeft  ==  NULL  )
	{
		retErrorMesg  =     "FFTslowDown::Shift_Pitch_Horizontally  FAILED,   SlowDown BUFFERS were NOT allocated."  ;
		return   false;
	}


	if(   m_realsMasters  ==  NULL    ||    m_imagMasters ==  NULL    )
	{
		retErrorMesg  =     "FFTslowDown::Shift_Pitch_Horizontally  FAILED,   m_realsMasters  and m_imagMasters   were NOT allocated."  ;
		return   false;
	}


	if(    useFixedPointCalcs    )
	{
		if(   m_realsFixMasters  ==  NULL    ||    m_imagFixMasters ==  NULL    )
		{
			retErrorMesg  =     "FFTslowDown::Shift_Pitch_Horizontally  FAILED,   m_realsFixMasters  and m_imagFixMasters   were NOT allocated."  ;
			return   false;
		}
	}




//	long   slowDownSpeed =   (long)pitchShift;  

//	long   slowDownSpeed =  3;   ****  IS now  InputParm,    'outputRepetitionCount'   *****   BIG,  because for 1.5 speed,  we want to return 3 samps for 2 that are inputed



	long   samplesToProcessAdjusted  =   numSampsToProcess;   //  default


	if(   decimationRatio  !=   1  )    
	{		
		 samplesToProcessAdjusted  =    numSampsToProcess  /  decimationRatio;      //  Need this for speed  1.5 
	}




	long      gRover = 0;

	double   magn, phase, window, real, imag;
	double   freqPerBin, expct;
	long      i,  k,   inFifoLatency, stepSize, fftFrameSize2;


																		// set up some handy variables
	fftFrameSize2 =    fftFrameSize /2;

	stepSize         =    fftFrameSize  /  osamp;       //   as osamp  gets bigger,    stepSize gets smaller

	inFifoLatency   =    fftFrameSize  -  stepSize;


	freqPerBin       =   sampleRate  /  (double)fftFrameSize;

	expct              =    2.0  *  M_PI  *   (double)stepSize / (double)fftFrameSize;

	
	long   sizeOfVertialComponents  =    fftFrameSize /2   +  1;    //   OR    fftFrameSize   ??? 



																					// initialize our persisting arrays 
	if(     ! m_areSlowDownBuffersInitialized    )
		Initialize_SlowDown_Variables();




	//	memset(    m_gFFTworksp,    0,      2 * fftFrameSize * sizeof(float)    ); 

	memset(    m_realsMasters,    0,          fftFrameSize             * sizeof(float)    );    // *** Necessary ???   *******************
	memset(    m_imagMasters,    0,          fftFrameSize             * sizeof(float)    ); 

	if(    useFixedPointCalcs    )
	{
		memset(    m_realsFixMasters,    0,      fftFrameSize      * sizeof(  long  )    );    
		memset(    m_imagFixMasters,    0,      fftFrameSize       * sizeof( long )    ); 
	}

   


	float    *gInFIFO=NULL,   *gOutFIFO=NULL,  *gOutputAccum=NULL,     *gLastPhase=NULL,   *gSumPhase=NULL; 

	if(   rightStereo   )     //  save values for next entrance to this function,  or we will hear 'static'  from variable confusion from switching from left to right stereo
	{
		gInFIFO       =     m_gInFIFORight;
		gOutFIFO      =    m_gOutFIFORight;
		gOutputAccum =    m_gOutputAccumRight;  

		gLastPhase  =      m_gLastPhaseRight;
		gSumPhase =      m_gSumPhaseRight;

		gRover =   m_gRoverRight;
	}
	else
	{  gInFIFO      =     m_gInFIFOLeft;
		gOutFIFO    =     m_gOutFIFOLeft;
		gOutputAccum =    m_gOutputAccumLeft;  

		gLastPhase  =      m_gLastPhaseLeft;
		gSumPhase =      m_gSumPhaseLeft;

		gRover =   m_gRoverLeft;
	}




																												//   typical for MP3 :     numSampsToProcess =   1104  

	for(   i = 0;    i < samplesToProcessAdjusted;    i++  )
	{

			//  We can really set   'samplesToProcessAdjusted'   to whatever amount the CallingFunction wants.  ( the FFT will AUTOMATICALLY
			//	 happen when  the  Input BUFFER ( gInFIFO[] )  has received  'stepsize'  new samples.  ( see how 'gRover' is initialized after a FFT )  1/2012 


																		// As long as we have not yet collected enough data for a FFT,  just read in more sample and return OutputSamples
																	    //  from the Output PIPELINE ( gOutFIFO[] )
	

	//	gInFIFO[  gRover    ]  =     indata[   i   *  2   ];        //  ex:   For  a  decimationRatio =  2
		gInFIFO[  gRover    ]  =     indata[   i   *  decimationRatio  ];       
						
		                                                                       //  * 2 :   read every other one,  gives us  552 INPUT samps  (  3  * 552 = 1656   what we want )
																			   //            BUT because we took every other one, we RAISED the pitch by one OCTANE,  so further 
																			   //            we use the Pitch reassignment to LOWER the signal by one OCTAVE.
																			   //
												                               //   Remember: In the HORIZONTAL algo we do NOT change the pitch, just make it LAST LONGER
																			   //   by spreading the  MULTIPLE ReConstructions  over the  output numnber of bytes ( 1656 ).   1/12




	//	outdata[ i ]    =    gOutFIFO[   gRover  -  inFifoLatency   ];    *** OLD Code
		long   oldIndexRover =          gRover  -  inFifoLatency;    



		long   outputSampleIndex =   i  *   outputRepetitionCount;   //   for OUTPUT,   there will be  {  SlowDownSpeed   TIMES   number-of-Input-Samples  } 


		for(    long  rp =  0;      rp < outputRepetitionCount;    rp++    )
		{

			long   trueIndexRover  =     ( outputRepetitionCount  *  oldIndexRover  )    +  rp;    

			outdata[  outputSampleIndex  ]    =      gOutFIFO[   trueIndexRover   ];      //   read the array-elements in  gOutFIFO[],  one right after another                  

			outputSampleIndex++;   //  Like Rover,  this is  prematurely incremented. ( for i=0,  its 2 now )     Carefull at the bottom
		}


		gRover++;    //  need to increment it early





																		
		if(   gRover  >=  fftFrameSize   )     // we now have enough data for processing  ( Happens after we have READ in only  'stepSize'  number of InputSamples  )
		{

			gRover =   inFifoLatency;    //  re-initialization of this TRAVERSING index  



			memset(    m_realsMasters,    0,          fftFrameSize      * sizeof(float)    ); 
			memset(    m_imagMasters,    0,          fftFrameSize       * sizeof(float)    ); 
					// ***** Do NOT need to also do FIXED arrays,  we do not use FIX FFT till further down   1/12  ***************



			/////////////////////////////////
																			//  do windowing and input the data into the Real and Imaginary arrays 

			for(  k = 0;    k < fftFrameSize;    k++  )       
			{

//				window  =    -.5 * cos(   2.*M_PI * (double)k   /  (double)fftFrameSize   ) + .5;     //  Yes,  need windowing.  It also stops Frequency Leaks across Bins.  1/12
	
												//  Welch Window from Masters  pp. 93   (  He says it will discourage Frequency-Leaking across Bins.  1/12  )
																
				window  =   1.0  -  (           (   (double)k  -  0.5 *  (double)(fftFrameSize - 1)   )    /    (  0.5 *  (double)(fftFrameSize + 1)   )
												   *   (   (double)k  -  0.5 *  (double)(fftFrameSize - 1)   )    /    (  0.5 *  (double)(fftFrameSize + 1)   )       );   
										


									//   Because we take advantage of ONLY Reals for Input,   pack BOTH  m_realsMasters[],  m_imagMasters[]  with signal's data.

				if(   (k % 2)  ==  0   )
					m_realsMasters[  k/2  ]   =     gInFIFO[ k ]   * window;      //    Even  terms in real part and... 
				else
					m_imagMasters[  k/2  ]   =     gInFIFO[ k ]   * window;      //    ...Odd  terms  in imaginary part. 
			}
			
			
																										//   ******** ANALYSIS **********																

			SndSample::FFT_with_only_Real_Input(    fftFrameSize/2,    m_realsMasters,  m_imagMasters   );    //  Faster, uses half as many calulations because of only Reals for Input.   
																												                          //   from Book by Timithothy Masters 
			/////////////////////////////////




			memset(  m_gAnaFreq,          0,   sizeOfVertialComponents  *sizeof(float)  );
			memset(  m_gAnaMagn,         0,   sizeOfVertialComponents  *sizeof(float)  );

			
																								
			for(   k = 0;    k <=   fftFrameSize2;    k++   )    // this is the analysis step 
			{
				
				real   =     m_realsMasters[  k  ];			
				imag  =     m_imagMasters[  k  ];
			
				
				magn =    2.0 *  sqrt(  real*real  +   imag*imag  );    // compute magnitude and phase 
				phase =   atan2(  imag, real );


				double   lastPhase =    gLastPhase[ k ];
				
				gLastPhase[ k ] =   phase;    //  save the  calculated-phase  for next iteration



					      //  Looks like they use the   DEVIATION in the EXPECTED-Phase   to calculate the  TRUE-Frequency  that is present in the Bin.     12/11

				double   trueFreq  =      Calculate_True_Frequency(    phase,    lastPhase,    k,    freqPerBin,    expct,    osamp  );

		//		double   approxFreq =        (double)k     *  freqPerBin;      ** DEBUG only 


				m_gAnaMagn[ k ]  =    magn;       //  store magnitude and true frequency in analysis arrays
				m_gAnaFreq[  k ]  =    trueFreq;  
			}



																								//    PROCESSING the PitchShift,  by moving values to other bins. 
		

			memset(  m_gSynMagn,    0,    sizeOfVertialComponents * sizeof(float)   );    
			memset(  m_gSynFreq,     0,    sizeOfVertialComponents * sizeof(float)   );


			/***
			for (  k = 0;     k  <  fftFrameSize2;    k++  )       
			{ 
				long  index  =    k  /  2;      //  try to lower it by an octave

				m_gSynMagn[  index  ]   +=      m_gAnaMagn[ k ]; 

								     //      1 or 0 ??     1[ not bad ]    0[ tin sound,  has ringing ]
				if(     ( k % 2 )  ==  1    )      //  **** CAREFUL, do not want to write TWICE to the same Freq-BIN, that's why we do the ODD/EVEN  IF statement.  1/12
				{					
					m_gSynFreq[  index   ]  =    (  m_gAnaFreq[  k ]    /   2.0  );   
				}
			}
			***/

			short   opCode =  0;   //  just for special Test Cases    1/12

			FFTslowDown::Shift_Frequency_Arrays_for_ReConstruction(   binsPitchShift,    fftFrameSize,      m_gAnaMagn,   m_gAnaFreq,      
														                                                                               m_gSynMagn,   m_gSynFreq,     opCode   );




			

			for(   long  rep= 0;     rep <  outputRepetitionCount;     rep++   )    // *** the REPETION Loop to increase Output Samples for SlowSpeeds  ***
			{


				memset(    m_realsMasters,    0,     fftFrameSize   * sizeof( float )    );  
				memset(    m_imagMasters,    0,     fftFrameSize   * sizeof( float )    ); 

				if(    useFixedPointCalcs     )
				{
					memset(    m_realsFixMasters,    0,     fftFrameSize   *  sizeof( long )    );   // **** NOTE:   longs,  not float
					memset(    m_imagFixMasters,    0,     fftFrameSize   *  sizeof( long )    ); 
				}



				double   fixMultiplier =    SPFRACFACT  ;          //   SPFRACFACT  or something all together different  ????    1/12

//				double   fixMultiplier =    SPFRACFACT  / 2.0  ;   //  ????  Output still look high.   1/7/2012


				
				double  wt  =   2.0;                               // Real and imaginary parts WOULD have reinforced (so simulate with 2x )

				wt   /=    (double)fftFrameSize;  //   Masters:   "Must scale down by 'n', before or after transform"  
															   //   BUT here we do it AFTER so can apply 'osamp'  ...
															   //                  ...SEE Below:   m_realsMasters[  k  ]      *    2.0 * window   /  ( fftFrameSize2 * osamp ); 


																													  //    ***** SYNTHESIS ***** 

		//		for(    k = 0;     k <=   fftFrameSize2;      k++   ) 
				for(    k = 1;     k <     fftFrameSize2;      k++   )    //  From  Masters. 
				{
																	
					magn =    m_gSynMagn[ k ];      // get magnitude and true frequency from synthesis arrays 

					double   trueFreq  =    m_gSynFreq[  k ];


								 //   Looks like the   DEVIATION in the Bin's Frequency  is used to calulate a corrected Phase-Difference between FFT frames.   12/11

					double   deltaPhase  =    Calculate_True_DeltaPhase(  trueFreq,   k,   freqPerBin,   expct,   osamp  );   //   'deltaPhase'  is in RADIANS

															
					gSumPhase[ k ]   +=    deltaPhase;    //  accumulate  delta-phase to get  bin's phase (  gSumPhase[]   is DIFFERENT for each  'rep'  )   12/2011 

					phase =    gSumPhase[ k ];




																	//   Put data is in RIGHT half of array,  and LEFT half was ZEROED-out( to stop wrap-arround )

					if(    useFixedPointCalcs    )  
					{

						double     tempFloat     =       wt    *   magn   *   cos( phase ); 

						ASSERT(  tempFloat  <=   1.0   );    // Never gets hit.    What happens if I do NOT divide by fftFrameSize  via  'wt'   ????   1/12


						m_realsFixMasters[   fftFrameSize  - k   ]  =     (long)(   tempFloat    *  fixMultiplier  );  



						tempFloat   =                 wt   *   magn   *   sin( phase );     //  wt   is positive:   Avoid the  "DUPLICATE Sign Flip"   below

						ASSERT(  tempFloat  <=   1.0   );    // Never gets hit.    What happens if I do NOT divide by fftFrameSize  via  'wt'   ????   1/12


						m_imagMasters[   fftFrameSize  - k   ]  =          (long)(   tempFloat    *  fixMultiplier  );  
					}
					else
					{  m_realsMasters[   fftFrameSize  - k   ]  =       wt      *   magn   *   cos( phase );  // ******** PROBLEM???   should  "-1"  be in this ?? **************

					//	m_imagMasters[   fftFrameSize  - k   ]  =      -wt      *   magn   *   sin( phase );   // sign change(-) cause this is now at UPPER end 
						m_imagMasters[   fftFrameSize  - k   ]  =       wt      *   magn   *   sin( phase );   // Avoid the  "DUPLICATE Sign Flip"   below
					}
				} 





				/////////////////////////////////
				if(    useFixedPointCalcs    )  
					m_realsFixMasters[0] =  m_imagFixMasters[0]  =  m_realsFixMasters[  fftFrameSize2  ] =  m_imagFixMasters[  fftFrameSize2  ] =   0;     // Also zero these
				else
					m_realsMasters[0]     =  m_imagMasters[0]      =  m_realsMasters[  fftFrameSize2  ]     =  m_imagMasters[  fftFrameSize2  ]   =   0.0;     // Also zero these


				/*******************************************************	Avoid this  "DUPLICATE Sign Flip"  by using a positve  'wt'  value up above													         				
				for(   k= 0;    k <  fftFrameSize;    k++  )     //   YES,  we need this or the sound is bad.     1/12
							m_imagMasters[ k ] =   -1.0  *  m_imagMasters[ k ];						 //  '-' FLIP SIGN of imaginary part
				****/


				if(    useFixedPointCalcs    )  
					SndSample::FFT_FixedPoint_Standard(    fftFrameSize,    m_realsFixMasters,   m_imagFixMasters   );  	   
				else
					SndSample::FFT_Standard(                    fftFrameSize,    m_realsMasters,        m_imagMasters      );  



		//		for(  k=0;    k< fftFrameSize;  k++ )   // Not really necessary because we do NOT use Imaginary array after this, but leav it in for clarity of the algo.
		//		{  
		//			m_imagMasters[ k ]  =    -1.0  *  m_imagMasters[ k ];         // '-'  to FLIP SIGN of imaginary part BACK    (  ???  Not really necessary 
		//		}	
				/////////////////////////////////



																//    do windowing and 'ADD'  to output accumulator ( this is the 'ADD' part to  "OverlapAndAdd"  ) 

				for(  k =0;   k < fftFrameSize;   k++  ) 
				{																//  Yes, we need windowon.

			//		window  =      -0.5  *  cos(  2.0 * M_PI * (double)k  /  (double)fftFrameSize  )   + 0.5;   //  Sames as above

					window  =   1.0  -  (           (   (double)k  -  0.5 *  (double)(fftFrameSize - 1)   )    /    (  0.5  *  (double)(fftFrameSize + 1)   )
													   *   (   (double)k  -  0.5 *  (double)(fftFrameSize - 1)   )    /    (  0.5  *  (double)(fftFrameSize + 1)   )      );    // Welch Window



					/////////////////////////////////
			//		gOutputAccum[  k  ]  +=      m_realsMasters[  k  ]      *    2.0 * window   /  ( fftFrameSize2 * osamp );   // in Masters this was done ABOVE by:   wt  /=  fftFrameSize
			//		gOutputAccum[  k  ]  +=      m_realsMasters[  k  ]      *            window   /  ( fftFrameSize2 * osamp );   // in Masters this was done ABOVE by:   wt  /=  fftFrameSize


					if(    useFixedPointCalcs    ) 
					{
						double   tempFloat  =      (double)(   m_realsFixMasters[ k ]   )    /   fixMultiplier;    //   Bring back to Floating Point


						if(     tempFloat   >=    1.0    )    // ***** SHOULD really test the range to get this right.
						{
							int  dummy =  9;   //   Gets hit alot,   but barely over 1.0    Is that is a PROBLEM ???      1/12
						}


						    //  ****** WHY need the    2 *  ???      I have to artifically increase the volume.   1/12  ****************
						gOutputAccum[  k  ]  +=        2 *  tempFloat            *            window   /  (    osamp                               );   // Now doing the division up above in 'wt' 
					}
					else
					{
					//	gOutputAccum[  k  ]  +=      m_realsMasters[  k  ]    *            window   /  (   osamp   *   (double)fftFrameSize   );     ...OLD code style

						gOutputAccum[  k  ]  +=      m_realsMasters[  k  ]    *            window   /  (   osamp                       );   // Now doing the division up above in 'wt' 
					}
				}

	



				for(  k = 0;    k < stepSize;    k++  )    // DIFFERENT:  Put the   MULTIPLE( slowDownSpeed )  BLOCKS   of   'stepside'(256)   Samples  one after another
				{

			//		gOutFIFO[  k  ]   =    gOutputAccum[  k  ];     ***OLD CODE

					long   trueOutputIndex  =    k    +    ( rep  *  stepSize ); 


					gOutFIFO[   trueOutputIndex  ]  =     gOutputAccum[  k  ];     //    gOutFIFO[]   will directly feed the  OUTPUT samples  ( see above )
				}  


																															
				memmove(    gOutputAccum,    gOutputAccum + stepSize,     fftFrameSize  * sizeof(float)     );    //  shift  the accumulator 

			}     //  for(  rep=0      




																 //    shift the  'INPUT'  FIFO  Pipeline  by   'stepSize' number  of samples  ( needs to be OUTSIDE of the Repetition loop )
																 //
																 //   (  this should be OK for most situations,   'stepSize'  is the REAL amount of samples that we read in before 
																 //      a FFT,   and so we must  SHIFT the pipeline of  Input Samples by  'stepSize'  for next FFT.     1/2012 	 
			for(   k = 0;    k < inFifoLatency;    k++ )        
			{
				gInFIFO[ k ]  =     gInFIFO[    k  +  stepSize   ];
			}


		}   //  if(   gRover  >=  fftFrameSize     .... have read enough Input-Samples for FFT processing 


	}   //   for(   i = 0;    i < samplesToProcessAdjusted; 



	
	if(   rightStereo   )				         //   save values for next entrance to this function,  or we will hear audio 'static'  from variable confusion 
		m_gRoverRight =     gRover;       //   from switching from left to right stereo during realtime play
	else
		m_gRoverLeft   =     gRover;

	return   true;
}



											////////////////////////////////////////


void   FFTslowDown::Shift_Frequency_Arrays_for_ReConstruction(   float  pitchShift,    long  frameSize,    float  *analyMagnitude,   float  *analyFreq,      
														                                                                              float  *synthMagnitude,   float  *synthFreq,   short opCode     )
{

	ASSERT(  analyMagnitude );
	ASSERT(  analyFreq  );
	ASSERT(  synthMagnitude );
	ASSERT(  synthFreq  );

	long  halfFrameSize =    frameSize / 2;
	long       indexInt;
	double   indexFlt;  



	if(        pitchShift  ==   1.0   )
	{
			
		for(  long   k = 0;      k <=  halfFrameSize;      k++  )      //  Just copy them over
		{ 
			synthMagnitude[  k  ]    =      analyMagnitude[  k  ]; 
			synthFreq[          k  ]    =      analyFreq[           k  ];   
		}
	}

	else if(    pitchShift   >   1.0   ) 
	{


		if(   opCode  ==  1   )    	//  is EXPERIMENTAL, it might not be used   ( below is the default case )     1/2012
		{														


			for(  long   k = 0;     k <  halfFrameSize;     k++   )       //   this does the actual pitch shifting 
			{ 

				indexFlt  =     (double)k   *  (double)pitchShift;		//  going to VERTICALLY  shift the  Mag and Freq  values  to other   FFT-frequencyRows, changes the pitch

				indexInt  =    (long)indexFlt;


				bool   skipThisBin =  false;  // We may get better result without  EXTRAPOLATING  when 'index' is not a good integer fit   1/12
		//		bool   skipThisBin =  true; 


				double  remainder    =     indexFlt    -   (double)indexInt;

				if(        remainder  >=   0.5  )
				{
			//		indexInt++;

			//		skipThisBin =  true;
			//		skipThisBin =  false;     //   Only do one with a remainder
				}
				

				if(   ! skipThisBin   )
				{

			//		if(   indexInt   <=   fftFrameSize2   ) 
					if(   indexInt   <     halfFrameSize   )    // ********* WHICH is right ???   12/20/11   **************************
					{ 

						synthMagnitude[  indexInt  ]    +=      analyMagnitude[  k  ]; 

						synthFreq[          indexInt  ]       =             analyFreq[   k ]   *  pitchShift; 

						int  dummy =  9;
					} 		
				}  
			}
		}   //    opCode  ==  1 
		else     
		{										 //    if(   opCode  ==  0   )     ...this is the DEFAULT

			for(  long   k = 0;     k <=  halfFrameSize;     k++  )      
			{ 

				indexInt =    (long)(    (float)k   *  pitchShift   );


		//		if(   indexInt   <=   halfFrameSize   ) 
				if(   indexInt   <     halfFrameSize   )    // ********* WHICH is right ???   12/20/11   **************************
				{
					synthMagnitude[   indexInt   ]    +=      analyMagnitude[  k  ]; 
					synthFreq[           indexInt   ]      =      analyFreq[           k  ]    *  pitchShift; 
				}
			}
		}  
	}

	else if(    pitchShift   <   1.0   ) 
	{

		float   pitchShiftRecip  =      1.0   /   pitchShift; 
		long   pitchShiftInt      =     (long)pitchShiftRecip;        //  Need to use the RECIPROCAL here


		long   pitchMinusOne =     pitchShiftInt  - 1;

		if(      pitchMinusOne   <  0  )
		{
			pitchMinusOne =  0;
			ASSERT( 0 );   //  ***** This is really UNTESTED (verify),   except for   pitchShift = .5       1/27/2012
		}



		for (  long   k = 0;     k  <  halfFrameSize;    k++  )        //   this does the actual pitch shifting ( Need to shift DOWN by one octave )
		{ 

		//	indexInt  =                           k    /     2;                               //  2:   lower it by an octave
			indexInt  =    (long)(     (float)k    /  pitchShiftRecip     );


			synthMagnitude[  indexInt  ]   +=    analyMagnitude[ k ]; 


							                    //      1  or  0 ??     1[ not bad ]    0[ tin sound,  has ringing ]     (  WAS for  pitchShift  =  0.5 )

//			if(     ( k %           2      )  ==      1    )  
			if(     ( k % pitchShiftInt )  ==   pitchMinusOne    )   //  **** CAREFUL,  do not want to write TWICE to the same Freq-BIN, that's why we do the ODD/EVEN  IF statement.  1/12
			{					

				synthFreq[  indexInt  ]  =      analyFreq[ k ]   /   pitchShiftRecip;   
			}
		}
	}
}



											////////////////////////////////////////


double   FFTslowDown::UnWrap_Phase(   double   phaseDeviation   )
{


	long  qpd =    phaseDeviation  /  M_PI;          //  map delta phase into  + / - Pi  interval    ( these 5 lines of code )

													// ***** CHECK this again...    is it always right ?? Maybe make a new FUNCTION.    12/2011  ********
	if(    qpd  >=   0   )							
		qpd  +=   qpd & 1;
	else 
		qpd  -=    qpd & 1;


				
	double  unWrap              =      M_PI  *  (double)qpd;    


	double  phaseUnWrapped =      phaseDeviation   -   unWrap; 


	return   phaseUnWrapped;
}


											////////////////////////////////////////


double   FFTslowDown::Calculate_True_Frequency(   double phase,   double  lastPhase,    long k,   double  freqPerBin,   double  expct,   long osamp   )
{

		//   Looks like they use the   DEVIATION in the EXPECTED-Phase   to calculate the  TRUE-Frequency  that is present in the Bin.     12/11


	double   phaseDifference =    phase   -    lastPhase;  


	double   expectedPhaseDifference  =     (double)k  *  expct;       //   expected phase difference 


	double   phaseDeviationOverStepsize =    phaseDifference   -   expectedPhaseDifference;       // subtract the  expected phase difference 

		

	//   ***********************   can use this code substitution to verify that the Phase Unwraping works for    pitchShift > 2.0       2/10


//				float   deltaPhaseOrig,    deltaPhaseUnwrap;
//				float   tolerance =  0.0001;
//
//				deltaPhaseOrig  =    tmp;
//															
//				qpd  =   tmp / M_PI;        //  map delta phase into  + / - Pi  interval 

//				if(   qpd >= 0   ) 
//					qpd  +=  qpd&1;
//				else 
//					qpd  -=   qpd&1;
//

//			//  NOT un-comment           	tmp -=    M_PI * (double)qpd;          tmp =  tmp -    M_PI * (double)qpd; 
//
//				deltaPhaseUnwrap  =    tmp  -   M_PI * (double)qpd;
//
//				if(   deltaPhaseUnwrap >  ( M_PI + tolerance   )   ||    deltaPhaseUnwrap <   -(M_PI + tolerance)   )
//				{
//					int  dummy =  9;
//				}
//
//				tmp  =    deltaPhaseUnwrap;
//
//     ...THINK  'tmp'  is   phaseDeviationOverStepsize  at this point in code.   12/11

					
	long  qpd =    phaseDeviationOverStepsize  /  M_PI;          //  map delta phase into  + / - Pi  interval    ( these 5 lines of code )

													// ***** CHECK this again...    is it always right ?? Maybe make a new FUNCTION.    12/2011  ********
	if(    qpd  >=   0   )							
		qpd  +=   qpd & 1;
	else 
		qpd  -=    qpd & 1;


				
	double  unWrap =    M_PI  *  (double)qpd;    


	phaseDeviationOverStepsize =      phaseDeviationOverStepsize  -   unWrap;   // This is the difference of Phase between  'stepSize'  number of samples ( a portion of the full FFTframe )





	double  phaseDeviationOverFFTframe  =   osamp  *  phaseDeviationOverStepsize;  
													//  Since it(stepSize) was a portion of the fftFrame,  multiply it by  'osamp'  to get the deviation over a full FFTframe
													//       Remember...    osamp  =   fftFrameSize / stepSize
													//
													//       Looks like  'phaseDeviationOverFFTframe'   is in RADIANS ????



//	double   devFromBinFreq  =     osamp * phaseDeviation          /   (2.0 * M_PI);    ***OLD***


	double   devFromBinFreq  =     phaseDeviationOverFFTframe   /   (2.0 * M_PI);    // get   deviation from bin frequency   from the   +/- Pi interval



/****										
	double   trueFreq     =    (   (double)k  +  devFromBinFreq  )    *   freqPerBin;               // compute the  k-th partials'  true frequency

	double   approxFreq =        (double)k              *  freqPerBin;       // **** DEBUG only 
***/


	double   deviatedFreq  =      devFromBinFreq     *   freqPerBin;

	double   expectedFreq =       (double)k              *  freqPerBin;     



	double   trueFreq   =    expectedFreq     +    deviatedFreq;    
	return   trueFreq;
}




											////////////////////////////////////////


double   FFTslowDown::Calculate_True_DeltaPhase(    double trueFreq,    long k,    double  freqPerBin,   double  expct,   long osamp   )
{


		//   Looks like the   DEVIATION in the Bin's Frequency  is used to calulate a corrected Phase-Difference between FFT frames.   12/11


		//   RETURN VALUE,  'deltaPhase',     is in RADIANS


																	
	double   expectedFreq =       (double)k        *  freqPerBin;      


	double   deviationFreq =        trueFreq   -   expectedFreq;       // subtract  bin's  mid frequency 
														

	double   freqsDeviationOverFFTframe =    deviationFreq  /   freqPerBin;     // get   bin deviation    from    freq deviation 

	



														
//	double   freqsDeviationOverStepsize  =     ( 2.0  *  M_PI )     *     freqsDeviationOverFFTframe / osamp;      //  take osamp into account 


	double   freqsDeviationOverStepsize  =       freqsDeviationOverFFTframe  /  osamp;     //   *  (2.0  *  M_PI); 

														//  Remember...    osamp  =   fftFrameSize / stepSize,    so it could also be expressed as:
//
//	                                                        freqsDeviationOverStepsize  =       freqsDeviationOverFFTframe   *    stepSize  /  fftFrameSize      



	double   freqsDeviationInRadians  =      freqsDeviationOverStepsize     *    ( 2.0  *  M_PI ); 

	double   freqsDeviationInRadiansUnWrapped  =    UnWrap_Phase(   freqsDeviationInRadians   );   //  ***************   TEMP,  test    1/3/2012   *******************





	double   expectedPhaseDifference  =     (double)k  *  expct;     //   i.e.    "overlap phase advance" 



	double   expectedPhaseDifferenceUnWrapped  =   UnWrap_Phase(   expectedPhaseDifference   );   //  ***************   TEMP,  test    1/3/2012   *******************

//  *****************   'expectedPhaseDifferenceUnWrapped'   is  always ZERO  when I use the OLD  FFT Fast-Filter Algo    1/12 


	if(   expectedPhaseDifferenceUnWrapped    !=   0.0   )
	{
		int  dummy =  9;
	}


														

//	double   deltaPhase  =   expectedPhaseDifference                    +    freqsDeviationInRadians;                    // add the   "overlap phase advance"(expected)   back in 
	double   deltaPhase  =   expectedPhaseDifferenceUnWrapped    +    freqsDeviationInRadiansUnWrapped;    // add the   "overlap phase advance"(expected)   back in 	
	//	return    deltaPhase;


	double	 deltaPhaseUnwrapped = 	UnWrap_Phase(   deltaPhase   );   //  ***************   TEMP,  test    1/3/2012   *******************






//	return    expectedPhaseDifferenceUnWrapped;   //   0.0   ********************   TEMP,   sopunds worse  ***************************

	return    deltaPhaseUnwrapped;     //     deltaPhase;
}





					/////////////////////////////////////////////


double 	  FFTslowDown::Get_Speeds_Volume_Tweak_Factor()
{

		//   Make slight adjustments to VOLUME of the  SlowedDown Signal to make it sound like the Volume of Speed 1.  I derived these by observation.  


	double  scaleFactor =    1.0;    //   default


	short   playSpeedTimesTen  =    (short)(   m_playSpeed   *  10.0  );   //   switch()  will NOT acept  DOUBLE   



	if(   m_slowDownAlgo  ==  0    )   //  VERTICAL
	{

	   switch(   playSpeedTimesTen   )           
	   {

			case  10:     scaleFactor =    1.0;       //         Keep compiler happy  
				break;



			case  15:     scaleFactor =    1.4;     //   1.4      this is the wierd one
				break;



			case  20:     scaleFactor =     0.7;     //    1.4;          
				break;

			case  30:     scaleFactor =     0.9;     //     1.0;          
				break;

			case  40:     scaleFactor =     1.0;     //     1.3;          
				break;

			case  60:     scaleFactor =    1.0;     //      1.4;          
				break;

			case  80:     scaleFactor =     1.0;     //     1.5;          
				break;


			default:   ASSERT( 0 );    scaleFactor =    1.0;    break;
	  }
	}
	else if(    m_slowDownAlgo  ==  1  )      //  HORIZONTAL
	{

	   switch(   playSpeedTimesTen   )                
	   {


			case  10:     scaleFactor =    1.0 ;     //         Keep compiler happy  
				break;



		   	case  15:     scaleFactor =    1.7 ;     //     1.8[too loud]     1.5 [ too quiet]            2.0[ little loud
				break;


			case  20:     scaleFactor =    1.7 ;     //      1.8;          
				break;

			case  30:     scaleFactor =    1.0 ;     //      0.8;          
				break;

			case  40:     scaleFactor =    1.3 ;     //   1.0
				break;

			case  60:     scaleFactor =     1.1 ;     //     1.0     
				break;

			case  80:     scaleFactor =     1.2 ;     //     1.0
				break;

			default:   ASSERT( 0 );    scaleFactor =    1.0;    
				break;
	   }
	}
	else
	{	ASSERT( 0 );   }


	return  scaleFactor;
}






					/////////////////////////////////////////////


bool	  FFTslowDown::Apply_SlowDown_wPitchShift(   BYTE**  retSamplesPtr,    long numInputSamples,    bool stereoFlag,    long  fftFrameSize,
										                                                                                        long osamp,     CString&   retErrorMesg   )
{

			//   CALLED BY:    Load_Next_MemoryBlock_Frame()


						//  Also uses   RESAMPLING


	float   pitchShift  =    m_playSpeed;   //  Seems to work OK, although the author said over 2.0 we would need to tweak  
																						 //  the code in Shift_Pitch().     2/10/10

	ASSERT(  pitchShift  >  0.0  );


// **********  How can I set this up to do   1.5  SlowDown speed ????   

		//   numInputSamples -   is INPUT samples,  more will come out.
											
		//   retSamplesPtr  -   is a pointer to a pointer because calling funct will receive data from it.


//	float  pitchShift      =   0.5;    //   2 - one octave up    [ must be in range of  0.5 to 2.0  ]


//	long  fftFrameSize  =   1024;    //  512[ eh ]      256[ worse ]         Typical values are 1024, 2048 and 4096. It may be any value <= MAX_FRAME_LENGTH but it MUST be a power of 2. 


//	long osamp          =       4;    //   4[ lousey ]     16[ fails ]
											//     is the STFT  oversampling factor which also determines the overlap between adjacent STFT frames. 
							                  //    It should at least be 4 for moderate scaling ratios. A value of 32 is  recommended for best quality. 


//	ASSERT(   pitchShift  <=  2.0  );     //  at present,  Shift_Pitch() can not  handle parms bigger than  2.0.  But web site says it possibe to do higher if tweak code.  2/10



	float  sampleRateFl      =   (float)(  DEFAULTsAMPLINGrATEmp3   );
	
	long   bytesPerSample =    MP3rEADERbYTESpERsAMPLE;     //   4


	retErrorMesg.Empty();


	if(  ! stereoFlag  )
	{
		retErrorMesg =  "FFTslowDown::Apply_SlowDown_wPitchShift FAILED,  can not process MONO signal." ;
		return  false;
	}

	if(      *retSamplesPtr  ==  NULL
		||   numInputSamples  <= 0   )
	{
		retErrorMesg =  "FFTslowDown::Apply_SlowDown_wPitchShift FAILED,  bad input sample pointer." ;
		return  false;
	}

	if(         m_leftSlowedDownSamples    ==   NULL
		 ||    m_rightSlowedDownSamples  ==   NULL    )
	{
		retErrorMesg =   "FFTslowDown::Apply_SlowDown_wPitchShift FAILED,  one or more reSampleArrays were NOT allocated."   ;
		return  false;
	}



	long    totalInputBytes  =      numInputSamples  *  bytesPerSample;    //    numInputSamples   is    m_outputBufferSampleCountNotSlowDown
	bool   lessThanOneValues =   true;


	float   multFactor  =   32750.0;     //  31000.0;        32768    




	if(   ! SndSample::Make_Float_Sample(  0,    (char*)(*retSamplesPtr),    totalInputBytes,   bytesPerSample,    m_auxInputSamples,   	 lessThanOneValues,   retErrorMesg  )   )
		return  false;
	

	if(   ! Shift_Pitch(          false,        pitchShift,    numInputSamples,  fftFrameSize,    osamp,    sampleRateFl,  	m_auxInputSamples,  m_leftSlowedDownSamples,     retErrorMesg  )   )
		return  false;
	

	if(    ! SndSample::Multiply_Float_Samples(   numInputSamples,     m_leftSlowedDownSamples,      multFactor,    retErrorMesg  )  )    //  this makes the values "MORE than one".
		return  false;





	if(   ! SndSample::Make_Float_Sample(  1,   (char*)(*retSamplesPtr),    totalInputBytes,   bytesPerSample,    m_auxInputSamples,   lessThanOneValues,   retErrorMesg  )   )
		return  false;


	if(   ! Shift_Pitch(     true,     pitchShift,    numInputSamples,  fftFrameSize,    osamp,    sampleRateFl,  	m_auxInputSamples,  m_rightSlowedDownSamples,     retErrorMesg  )   )
		return  false;


	if(    ! SndSample::Multiply_Float_Samples(   numInputSamples,     m_rightSlowedDownSamples,      multFactor,    retErrorMesg  )   )     //  this makes the values "MORE than one".
		return  false;




	float   *yptptLeft =   NULL;      
	int        ylenLeft =   -1;         
	float   *yptptRight =   NULL;    
	int        ylenRight =   -1;            


	m_reSamplerSlowDown.Resample_Signal(  	m_leftSlowedDownSamples,      numInputSamples,     &yptptLeft,  	  &ylenLeft      );   // ylenLeft  is Output SAMPLE count   
	m_reSamplerSlowDown.Resample_Signal(  	m_rightSlowedDownSamples,    numInputSamples,     &yptptRight,      &ylenRight   );


	ASSERT(   ylenLeft  ==   ylenRight   );



	bool    toStereo       =   true;      //     *******  MAY  NEED  CHANGE if I start to support MONO   2/10   *********
	long    retByteCount =   -1; 
	bool   noMemoryRelease  =   false; 

	double  volumeTweak =   Get_Speeds_Volume_Tweak_Factor();


	lessThanOneValues =   false;      // ***BIG,  need to CHANGE its value 



	if(    ! SndSample::Merge_Float_Channels_To_Sample(   toStereo,    ylenRight,     bytesPerSample,       &yptptLeft,   &yptptRight,    (char**)retSamplesPtr,   															                             
																										retByteCount,    lessThanOneValues,  noMemoryRelease,  volumeTweak,  retErrorMesg   )   )
		return  false;   
															//  SndSample::Merge_Float_Channels_To_Sample() will automatically free  {  yptptLeft,  yptptRight  } 

	return  true;
}





					/////////////////////////////////////////////


bool	  FFTslowDown::Apply_SlowDown_wHorizontal_FFTs(   BYTE**  retSamplesPtr,    long numInputSamples,    bool stereoFlag,    long  fftFrameSize,
										                                                                                        long osamp,     CString&   retErrorMesg   )
{

			//   CALLED BY:    Load_Next_MemoryBlock_Frame()


						//   **** NEW ****      Also uses   RESAMPLING


	float   pitchShift  =    m_playSpeed;   //  Seems to work OK, although the author said over 2.0 we would need to tweak  
																						 //  the code in Shift_Pitch().     2/10/10

	ASSERT(  pitchShift  >  0.0  );


// **********  How can I set this up to do   1.5  SlowDown speed ????   

		//   numInputSamples -   is INPUT samples,  more will come out.
											
		//   retSamplesPtr  -   is a pointer to a pointer because calling funct will receive data from it.


//	float  pitchShift      =   0.5;    //   2 - one octave up    [ must be in range of  0.5 to 2.0  ]


//	long  fftFrameSize  =   1024;    //  512[ eh ]      256[ worse ]         Typical values are 1024, 2048 and 4096. It may be any value <= MAX_FRAME_LENGTH but it MUST be a power of 2. 


//	long osamp          =       4;    //   4[ lousey ]     16[ fails ]
											//     is the STFT  oversampling factor which also determines the overlap between adjacent STFT frames. 
							                  //    It should at least be 4 for moderate scaling ratios. A value of 32 is  recommended for best quality. 


//	ASSERT(   pitchShift  <=  2.0  );     //  at present,  Shift_Pitch() can not  handle parms bigger than  2.0.  But web site says it possibe to do higher if tweak code.  2/10



	float  sampleRateFl  =  (float)(  DEFAULTsAMPLINGrATEmp3   );
	
	long   bytesPerSample =  MP3rEADERbYTESpERsAMPLE;     //   4


	retErrorMesg.Empty();


	if(  ! stereoFlag  )
	{
		retErrorMesg =  "FFTslowDown::Apply_SlowDown_wPitchShift FAILED,  can not process MONO signal." ;
		return  false;
	}

	if(      *retSamplesPtr  ==  NULL
		||   numInputSamples  <= 0   )
	{
		retErrorMesg =  "FFTslowDown::Apply_SlowDown_wPitchShift FAILED,  bad input sample pointer." ;
		return  false;
	}

	if(         m_leftSlowedDownSamples    ==   NULL
		 ||    m_rightSlowedDownSamples  ==   NULL    
		 ||    m_auxInputSamples  ==  NULL   )
	{
		retErrorMesg =   "FFTslowDown::Apply_SlowDown_wPitchShift FAILED,  one or more reSampleArrays were NOT allocated."   ;
		return  false;
	}



	long    totalInputBytes  =      numInputSamples  *  bytesPerSample;    //    numInputSamples   is    m_outputBufferSampleCountNotSlowDown
	bool   lessThanOneValues =   true;

	long   speedExpandedNumSamples  =    (long)(     (double)numInputSamples    *   m_playSpeed     );

	float   multFactor  =   32750.0;     //  31000.0;        32768    





//  **********  Could do ReSampling here for 1.5 Speed    Would it really SOUND better ???    3/1/12  *************************************************

	if(   ! SndSample::Make_Float_Sample(  0,    (char*)(*retSamplesPtr),    totalInputBytes,   bytesPerSample,    m_auxInputSamples,    lessThanOneValues,   retErrorMesg  )   )
		return  false;
	




	float   binsPitchShift;  	 
	long   decimationRatio,   outputRepetitionCount;


	if(     pitchShift  ==  1.5    )    //  *** Special case for  1.5  *** 
	{													

		decimationRatio           =     2;    //      2 :    Raise the SOURCE signal by one octave with decimation. ( Input NEEDS to be 552 samples )

		binsPitchShift              =    0.5;  	//   0.5 :   Now must bring the signal  DOWN the octave  that we raised it with DECIMATION

		outputRepetitionCount  =     3;  	//      3 :      3  x  552  =    1656      (  1656:  the number of EXPANDED output samples that we need )
	}
	else
	{
		decimationRatio           =    1;              //     1 :   No decimation for most HORIZONTAL

		binsPitchShift              =    1.0;            //   1.0 :    No pitchShifting for most HORIZONTAL,  just REPEAT the output  fft-ReConstructions	

		outputRepetitionCount  =   (long)pitchShift;     //  this ratio will EXPAND the number of Output Samples,  must adjust the Output Buffers' sizes
	}





				//  In HORIZONTAL Case we can NOT use 'm_leftSlowedDownSamples'  for both Input and Output in  Shift_Pitch_Horizontally()   ...they are different sizes


	if(   m_fftCodeType_horizontal  ==  1  )
	{
		if(   ! Shift_Pitch_Horizontally_OLDfft(    false,        pitchShift,    numInputSamples,    fftFrameSize,     osamp,    sampleRateFl,  	m_auxInputSamples,  
																																					m_leftSlowedDownSamples,    retErrorMesg  )   )
			return  false;
	}
	else
	{  if(   ! Shift_Pitch_Horizontally(    false,      pitchShift,    numInputSamples,    fftFrameSize,     osamp,    sampleRateFl,  	m_auxInputSamples,  
																		  m_leftSlowedDownSamples,    binsPitchShift,   decimationRatio,  outputRepetitionCount,   retErrorMesg  )   )
			return  false;
	}


	if(    ! SndSample::Multiply_Float_Samples(   speedExpandedNumSamples,     m_leftSlowedDownSamples,      multFactor,    retErrorMesg  )  )    //  this makes the values "MORE than one".
		return  false;






	if(   ! SndSample::Make_Float_Sample(  1,   (char*)(*retSamplesPtr),    totalInputBytes,   bytesPerSample,    m_auxInputSamples,     lessThanOneValues,   retErrorMesg  )   )
		return  false;



	if(   m_fftCodeType_horizontal  ==  1  )
	{
		if(   ! Shift_Pitch_Horizontally_OLDfft(     true,     pitchShift,    numInputSamples,     fftFrameSize,     osamp,    sampleRateFl,     m_auxInputSamples,    
																																					m_rightSlowedDownSamples,   retErrorMesg  )   )
			return  false;
	}
	else
	{  if(   ! Shift_Pitch_Horizontally(   true,   pitchShift,    numInputSamples,     fftFrameSize,     osamp,    sampleRateFl,     m_auxInputSamples,    
																			m_rightSlowedDownSamples,     binsPitchShift,   decimationRatio,  outputRepetitionCount,    retErrorMesg  )   )
			return  false;
	}


	if(    ! SndSample::Multiply_Float_Samples(   speedExpandedNumSamples,     m_rightSlowedDownSamples,      multFactor,    retErrorMesg  )   )     //  this makes the values "MORE than one".
		return  false;





	bool    toStereo       =   true;     
	long    retByteCount =   -1; 
	bool    noMemoryRelease  =   true;   // *** BIG

	double  volumeTweak =   Get_Speeds_Volume_Tweak_Factor();


	lessThanOneValues =   false;      //  false is DEFAULT.    The  OUTPUT signal(  retSamplesPtr  )  can NOT be less than one,  but needs the integer representation of a sample.  12/11




	if(    ! SndSample::Merge_Float_Channels_To_Sample(   toStereo,     speedExpandedNumSamples,      bytesPerSample,     
		                                                                        &m_leftSlowedDownSamples,     &m_rightSlowedDownSamples,   
																	(char**)retSamplesPtr,   retByteCount,    lessThanOneValues,   noMemoryRelease,  volumeTweak,   retErrorMesg   )   )
		return  false;   
															//  For HORIZONTAL,  Merge_Float_Channels_To_Sample() will NOT automatically free blocks  {  yptptLeft,  yptptRight  } 

	return  true;
}





					/////////////////////////////////////////////


bool	  FFTslowDown::Apply_Phase_Filtering_wFFT(   BYTE**  retSamplesPtr,    long numInputSamples,    bool stereoFlag,    long  fftFrameSize,
										                                                                                                          long osamp,     CString&   retErrorMesg   )
{

		// ******************   Very EXPERIMENTAL and Computationally EXPENSIVE.   Is it worth it ???     12/2011   ****************************


		//   CALLED BY:    Load_Next_MemoryBlock_Frame()


	float  sampleRateFl       =   (float)(  DEFAULTsAMPLINGrATEmp3   );
	
	long   bytesPerSample  =    MP3rEADERbYTESpERsAMPLE;     //   4


	retErrorMesg.Empty();


	if(    m_playSpeed   !=   1.0   )
	{
		retErrorMesg =  "FFTslowDown::Apply_Phase_Filtering_wFFT  FAILED,   it can CURRENTLY only work at SlowSpeed = 1." ;
		return  false;
	}


	if(  ! stereoFlag  )
	{
		retErrorMesg =  "FFTslowDown::Apply_Phase_Filtering_wFFT  FAILED,  can not process MONO signal." ;
		return  false;
	}

	if(      *retSamplesPtr  ==  NULL
		||   numInputSamples  <= 0   )
	{
		retErrorMesg =  "FFTslowDown::Apply_Phase_Filtering_wFFT  FAILED,  bad input sample pointer." ;
		return  false;
	}




	if(         m_leftSlowedDownSamples    ==   NULL
		 ||    m_rightSlowedDownSamples  ==   NULL    )
	{
		//   retErrorMesg =   "FFTslowDown::Apply_Phase_Filtering_wFFT  FAILED,  one or more reSampleArrays were NOT allocated."   ;
		//   return  false;

		long  dummySpeed =  2;



		                                    //     double  newPlaySpeed,   short  slowDownAlgo,    long  outputBufferSampleCountNotSlowDown,    CString&   retErrorMesg 

		if(     ! Alloc_SlowDown_Buffers(   dummySpeed,   m_slowDownAlgo,    numInputSamples,   retErrorMesg  )    )  // ****** WORK with this change to funct  4/21/12
			return  false;


		/*************  NOW done in Alloc_SlowDown_Buffers    ...is the SIZE right for this weird function ???   12/30/2011


		if(     (    m_leftSlowedDownSamples  =    new    float[  numInputSamples ]    )   == NULL    )  
		{
			retErrorMesg =    "FFTslowDown::Apply_Phase_Filtering_wFFT  failed,  could not allocate  m_leftSlowedDownSamples."  ;
			return  false;
		}

		if(     (   m_rightSlowedDownSamples  =    new    float[  numInputSamples  ]    )   == NULL    )  
		{
			retErrorMesg =  "FFTslowDown::Apply_Phase_Filtering_wFFT  failed,  could not allocate  m_rightSlowedDownSamples."  ;
			return false;
		}	
		*****/

	}

	ASSERT(   m_leftSlowedDownSamples  !=   NULL      &&      m_rightSlowedDownSamples  !=   NULL     );





	long    totalInputBytes  =      numInputSamples  *  bytesPerSample;    //    numInputSamples   is    m_outputBufferSampleCountNotSlowDown

	float   multFactor  =   32750.0;     //  31000.0;        32768    


	bool   lessThanOneValues =   true;   //   'true'  for  SndSample::Make_Float_Sample(),   but NOT for Merge_Float_Channels_To_Sample()



	/****
	if(   ! SndSample::Make_Float_Sample(   0,   (char*)(*retSamplesPtr),    totalInputBytes,   bytesPerSample,   m_leftSlowedDownSamples,    lessThanOneValues,   retErrorMesg  )   )
		return  false;	

	if(   ! SndSample::Make_Float_Sample(   1,   (char*)(*retSamplesPtr),    totalInputBytes,   bytesPerSample,   m_rightSlowedDownSamples,   lessThanOneValues,   retErrorMesg  )   )
		return  false;




//	if(    ! Do_Phase_Filtering(    numInputSamples,      fftFrameSize,      osamp,    sampleRateFl,      																																								
//																						m_leftSlowedDownSamples,      m_rightSlowedDownSamples,    	
//																						m_leftSlowedDownSamples,      m_rightSlowedDownSamples,       retErrorMesg  )   )
//	{  return  false;   }
	
	


	if(    ! FastSharpen_Column(    numInputSamples,    fftFrameSize,      sampleRateFl,      m_leftSlowedDownSamples,     m_leftSlowedDownSamples,    retErrorMesg  )   )
	{  return  false;   }


	if(    ! FastSharpen_Column(    numInputSamples,    fftFrameSize,      sampleRateFl,  	   m_rightSlowedDownSamples,   m_rightSlowedDownSamples,   retErrorMesg  )   )
	{  return  false;   }





	if(    ! SndSample::Multiply_Float_Samples(   numInputSamples,     m_leftSlowedDownSamples,        multFactor,    retErrorMesg  )   )  //  this makes the values "MORE than one".
		return  false;

	if(    ! SndSample::Multiply_Float_Samples(   numInputSamples,     m_rightSlowedDownSamples,      multFactor,    retErrorMesg  )   )   
		return  false;
	****/

	ASSERT(  m_auxInputSamples   );


	if(   ! SndSample::Make_Float_Sample(   0,   (char*)(*retSamplesPtr),    totalInputBytes,   bytesPerSample,   m_auxInputSamples,    lessThanOneValues,   retErrorMesg  )   )
		return  false;	


//          FastSharpen_Column   
	if(    ! Fast_Filter_MemBlock(   false,  numInputSamples,    fftFrameSize,      sampleRateFl,      m_auxInputSamples,     m_leftSlowedDownSamples,    retErrorMesg  )   )
	{  return  false;   }


	if(    ! SndSample::Multiply_Float_Samples(   numInputSamples,     m_leftSlowedDownSamples,        multFactor,    retErrorMesg  )   )  //  this makes the values "MORE than one".
		return  false;





	if(   ! SndSample::Make_Float_Sample(   1,   (char*)(*retSamplesPtr),    totalInputBytes,   bytesPerSample,   m_auxInputSamples,   lessThanOneValues,   retErrorMesg  )   )
		return  false;


//          Fast_Filter_MemBlock
	if(    ! Fast_Filter_MemBlock(  true,   numInputSamples,    fftFrameSize,      sampleRateFl,  	   m_auxInputSamples,   m_rightSlowedDownSamples,   retErrorMesg  )   )
	{  return  false;   }


	if(    ! SndSample::Multiply_Float_Samples(   numInputSamples,     m_rightSlowedDownSamples,      multFactor,    retErrorMesg  )   )   
		return  false;



 

	bool    toStereo       =   true;     
	long    retByteCount =   -1; 
	bool    noMemoryRelease  =   true;   // *** BIG

	double  volumeTweak =  1.0;    //   Get_Speeds_Volume_Tweak_Factor();



	lessThanOneValues =   false;    //  false is DEFAULT.    The  OUTPUT signal(  retSamplesPtr  )  can NOT be less than one,  but needs the integer representation of a sample.  12/11



	if(    ! SndSample::Merge_Float_Channels_To_Sample(   toStereo,     numInputSamples,     bytesPerSample,    &m_leftSlowedDownSamples,   &m_rightSlowedDownSamples, 
																	                 (char**)retSamplesPtr,   retByteCount,    lessThanOneValues,   noMemoryRelease,  volumeTweak,  retErrorMesg   )   )
		return  false;   
															//  For this funct,  Merge_Float_Channels_To_Sample() will NOT automatically free blocks {  m_leftSlowedDownSamples,  m_rightSlowedDownSamples  } 

	return  true;
}





					/////////////////////////////////////////////


bool     FFTslowDown::Do_Phase_Filtering(     long numSampsToProcess,     long fftFrameSize,     long osamp,   float sampleRate,      																																								
																		  float *indataLeft,   float *indataRight,        float *outdataLeft,    float *outdataRight,    CString&   retErrorMesg  )   
{


		//   *************  Could  OPTIMIZE this  by using   SndSample::FFT_with_only_Real_Input()   1/12 **********************



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

//	sampleRate -    takes the sample rate for the signal  in unit Hz,   ie. 44100  for  44.1 kHz  audio. 


//	The data passed to the routine in  indata[] should be in the range [ -1.0, 1.0 ],  which is also the output range 
 //	for the data, make sure you scale the data accordingly ( for 16bit signed integers you would have to divide (and multiply) by 32768 ). 


	retErrorMesg.Empty();



	if(   m_gInFIFOLeft  ==  NULL  )
	{
		retErrorMesg  =     "FFTslowDown::Shift_Pitch  FAILED,   SlowDown BUFFERS were NOT allocated."  ;
		return   false;
	}

	if(    indataLeft  ==  NULL      ||     indataRight  == NULL      ||      outdataLeft == NULL      ||     outdataRight  == NULL    )   
	{
		retErrorMesg  =     "FFTslowDown::Shift_Pitch  FAILED,   some Input Parms are NULL."  ;
		return   false;
	}


	ASSERT(   m_playSpeed  ==   1.0    );      // ************************   TEMP

	 



	double   magn, phase, window, real, imag;
	double   freqPerBin, expct;
	long      i,k, inFifoLatency, stepSize, fftFrameSize2;


																		// set up some handy variables
	fftFrameSize2 =    fftFrameSize /2;

	stepSize         =    fftFrameSize  /  osamp;       //   as osamp  gets bigger,    stepSize gets smaller

	inFifoLatency   =    fftFrameSize  -  stepSize;

	freqPerBin       =   sampleRate  /  (double)fftFrameSize;

	expct              =    2.  *  M_PI  *   (double)stepSize / (double)fftFrameSize;

	long   sizeOfVertialComponents  =    fftFrameSize /2   +  1;   




																					// initialize our persisting arrays 
	if(    ! m_areSlowDownBuffersInitialized    )
	{
				
		/****
		Erase_SlowDown_Buffers();

		m_gRoverLeft  =   m_gRoverRight =    inFifoLatency;

		m_areSlowDownBuffersInitialized =    true;      //  this is a little controversial,  but seems to work OK	
		****/
		Initialize_SlowDown_Variables();
	}




	float        *gAnaFreqLeft  =       new    float[    sizeOfVertialComponents  ];     // ***********   MAKE these Persist as MemberVars  12/11 *****************
	ASSERT(  gAnaFreqLeft   );

	float        *gAnaMagnLeft   =       new    float[   sizeOfVertialComponents   ];     
	ASSERT(  gAnaMagnLeft   );

	float        *gSynFreqLeft  =       new    float[     sizeOfVertialComponents  ];     
	ASSERT(  gSynFreqLeft   );

	float        *gSynMagnLeft   =       new    float[   sizeOfVertialComponents  ];    
	ASSERT(  gSynMagnLeft   );




	memset(    m_gFFTworksp,    0,      2 * fftFrameSize * sizeof(float)    ); 



																										// main processing loop 
	for(   i = 0;    i < numSampsToProcess;    i++  )
	{

																	                  	// As long as we have not yet collected enough data just read in 
		m_gInFIFOLeft[    m_gRoverLeft  ]    =   indataLeft[   i  ];
		m_gInFIFORight[  m_gRoverRight  ]  =   indataRight[ i  ];

		outdataLeft[   i ]   =    m_gOutFIFOLeft[     m_gRoverLeft    -  inFifoLatency   ];    // Think I need to first read all input before I write out ???   12/11
		outdataRight[ i ]   =    m_gOutFIFORight[   m_gRoverRight  -  inFifoLatency   ];    //     CAREFUL,    outdataLeft[]    is the SAME ARRAY  as  indataLeft[]    and
																														//                        outdataRight[]  is the SAME ARRAY  as  indataRight[]

		m_gRoverLeft++;    m_gRoverRight++;



																		
		if(    m_gRoverLeft   >=   fftFrameSize    )      // now we have enough data for processing 
		{

			ASSERT(    m_gRoverRight  >=  fftFrameSize  );

			m_gRoverLeft =    m_gRoverRight  =     inFifoLatency;    //  re-initialization of this TRAVERSING index



		/////////////////////////////////////////
			memset(  m_gFFTworksp,    0,   2 * fftFrameSize   * sizeof(float)  );



																			// do windowing and  re,im  interleave
			for (  k = 0;   k < fftFrameSize;    k++) 
			{
				window =    -.5 * cos(   2.*M_PI * (double)k   /  (double)fftFrameSize   ) + .5;     //  Do I need windowing ?????

				m_gFFTworksp[ 2*k     ]   =   m_gInFIFOLeft[ k ]  * window;    //   FFT  inputs  are take  from the   gInFIFO[]   circularQue-delay
				m_gFFTworksp[ 2*k +1]   =   0.;
			}

																									//   ****************** ANALYSIS *******************
																	
			smbFft(   m_gFFTworksp,   fftFrameSize,   -1   );    //  do  FFT transform 



			memset(  gAnaFreqLeft,          0,   sizeOfVertialComponents  *sizeof(float)  );
			memset(  gAnaMagnLeft,         0,   sizeOfVertialComponents  *sizeof(float)  );

			
																								
			for(   k = 0;    k <=   fftFrameSize2;    k++   )    // this is the analysis step 
			{
				
				real   =   m_gFFTworksp[ 2*k      ];			// de-interlace FFT buffer 
				imag =   m_gFFTworksp[ 2*k +1 ];
				
				magn =    2. *  sqrt(  real*real  +   imag*imag  );    // compute magnitude and phase 
				phase =   atan2(  imag, real );


				double   lastPhase =    m_gLastPhaseLeft[ k ];
				
				m_gLastPhaseLeft[ k ] =   phase;    //  save the  calculated-phase  for next iteration



					      //  Looks like they use the   DEVIATION in the EXPECTED-Phase   to calculate the  TRUE-Frequency  that is present in the Bin.     12/11

				double   trueFreq  =      Calculate_True_Frequency(    phase,    lastPhase,    k,    freqPerBin,    expct,    osamp  );

		//		double   approxFreq =        (double)k     *  freqPerBin;      ** DEBUG only 


				gAnaMagnLeft[ k ]  =    magn;       //  store magnitude and true frequency in analysis arrays
				gAnaFreqLeft[  k ]  =    trueFreq;  
			}
		/////////////////////////////////////////



		/////////////////////////////////////////         Right Analysis
			memset(  m_gFFTworksp,    0,   2 * fftFrameSize   * sizeof(float)  );


																			// do windowing and  re,im  interleave
			for (  k = 0;   k < fftFrameSize;    k++) 
			{
				window =    -.5 * cos(   2.*M_PI * (double)k   /  (double)fftFrameSize   ) + .5;     //  Do I need windowing ?????

				m_gFFTworksp[ 2*k     ]   =   m_gInFIFORight[ k ]  * window;    //   FFT  inputs  are take  from the   gInFIFO[]   circularQue-delay
				m_gFFTworksp[ 2*k +1]   =   0.0;
			}

																									//   ****************** ANALYSIS *******************
																	
			smbFft(   m_gFFTworksp,   fftFrameSize,   -1   );    //        do  FFT transform 



			memset(  m_gAnaFreq,          0,   sizeOfVertialComponents  *sizeof(float)  );     //  this is for the right stereo channel
			memset(  m_gAnaMagn,         0,   sizeOfVertialComponents  *sizeof(float)  );

			
																								
			for(   k = 0;    k <=   fftFrameSize2;    k++   )    // this is the analysis step 
			{
				
				real   =   m_gFFTworksp[ 2*k      ];			// de-interlace FFT buffer 
				imag =   m_gFFTworksp[ 2*k +1 ];
				
				magn =    2.0 *  sqrt(  real*real  +   imag*imag  );    // compute magnitude and phase 
				phase =   atan2(  imag, real );


				double   lastPhase =    m_gLastPhaseRight[ k ];
				
				m_gLastPhaseRight[ k ] =   phase;    //  save the  calculated-phase  for next iteration



					      //  Looks like they use the   DEVIATION in the EXPECTED-Phase   to calculate the  TRUE-Frequency  that is present in the Bin.     12/11

				double   trueFreq  =      Calculate_True_Frequency(    phase,    lastPhase,    k,    freqPerBin,    expct,    osamp  );

		//		double   approxFreq =        (double)k     *  freqPerBin;      ** DEBUG only 


				m_gAnaMagn[ k ]  =    magn;       //  store magnitude and true frequency in analysis arrays
				m_gAnaFreq[  k ]  =    trueFreq;  
			}
		/////////////////////////////////////////




																								//  ******   processing the   Phase  FILTERING  *******
		
			memset(  gSynMagnLeft,    0,    sizeOfVertialComponents * sizeof(float)   );    
			memset(  gSynFreqLeft,     0,    sizeOfVertialComponents * sizeof(float)   );

			memset(  m_gSynMagn,    0,    sizeOfVertialComponents * sizeof(float)   );    
			memset(  m_gSynFreq,     0,    sizeOfVertialComponents * sizeof(float)   );


			for (  k = 0;    k  <=  fftFrameSize2;    k++  )       //   this does the actual pitch shifting 
			{ 
													//   No PitchShift here...   just copy back to original bins.

				if(   k   <=   fftFrameSize2   ) 
		//		if(   k   <     fftFrameSize2   )    // ********* WHICH is right ???   12/20/11   **************************
				{ 
					
					/****
					gSynMagnLeft[ k ]   =    gAnaMagnLeft[ k ];   //  ...using  EQUAL until the above test shows ups a problem.   12/28/2011
					gSynFreqLeft[  k ]   =    gAnaFreqLeft[ k ];   

					m_gSynMagn[ k ]   =    m_gAnaMagn[ k ];  
					m_gSynFreq[  k ]   =    m_gAnaFreq[ k ];   
					*****/

					long   leftFrequency   =     (long)(  gAnaFreqLeft[ k ]   );  
					long   rightFrequency =     (long)(  m_gAnaFreq[ k ]   );   

					float   expectedFreq =     (float)k   *  freqPerBin;     


					float  divisor =   100.0;      //   For CenterVoice REMOVAL:    the BIGGER this number,  the MORE center that will LEAK in.  
														   //
				                                           //   **** Can also make the  'm_fftFrameSizeSlowDown'  much bigger in FFTslowDown's contructor,  but changes scale for  'divisor'
														   //                 Works much better with   4096 and  8192


						//    200[ 4096, good removal ]      400[  8192,  not bad ]      800[  8192,  OK ]      100[ 2048,  not bad  ]

																																																										
							
					float   tolerance  =      expectedFreq  /   divisor; 



					bool   frequenciesAreClose =  false;

					if(         (    gAnaFreqLeft[ k ]    <    (  m_gAnaFreq[ k ]    +  tolerance )     )
						   && (    gAnaFreqLeft[ k ]    >    (  m_gAnaFreq[ k ]    -   tolerance )     )   
					  )
					  frequenciesAreClose =   true;



					if(       !   frequenciesAreClose         //  *** SWITCH:    Use   NOT ( ! )   for VoiceRemoval
					//	&&      leftFrequency  !=  0  
					  )
					{
						gSynMagnLeft[ k ]   =     gAnaMagnLeft[ k ]; 
						m_gSynMagn[  k ]   =     m_gAnaMagn[ k ];     //  this is the right stereo channel
					}


					gSynFreqLeft[  k ]   =    gAnaFreqLeft[ k ];     //  Freqs always get passed through
					m_gSynFreq[   k ]   =    m_gAnaFreq[ k ];   
				} 				
			}

			


																					            //    SYNTHESIS 
		/////////////////////////////////////////
			
			memset(    m_gFFTworksp,    0,      2 * fftFrameSize * sizeof(float)    );    // **************   NEW,  OK ?????????   ****************


			for(    k = 0;     k <=   fftFrameSize2;      k++   ) 
			{
																	
				magn =    gSynMagnLeft[ k ];      // get magnitude and true frequency from synthesis arrays 

				double   trueFreq  =    gSynFreqLeft[  k ];


								 //   Looks like the   DEVIATION in the Bin's Frequency  is used to calulate a corrected Phase-Difference between FFT frames.   12/11

				double   deltaPhase  =    Calculate_True_DeltaPhase(   trueFreq,    k,    freqPerBin,    expct,    osamp   );   //   'deltaPhase'  is in RADIANS

															
				m_gSumPhaseLeft[ k ]   +=    deltaPhase;    //  accumulate  delta-phase to get  bin's phase (  gSumPhase[]   is DIFFERENT for each  'rep'  )   12/2011 

				phase =    m_gSumPhaseLeft[ k ];

																				
				m_gFFTworksp[ 2 * k      ]   =    magn   *   cos( phase );     // get real and imag part and re-interleave
				m_gFFTworksp[ 2 * k +1 ]   =    magn   *   sin( phase );
			} 


																							// zero negative frequencies 
			for(   k =  fftFrameSize +2;     k < (2 * fftFrameSize);       k++ ) 
				m_gFFTworksp[ k ] =   0.0;


																							
			smbFft(   m_gFFTworksp,   fftFrameSize,   1   );     // **  do the  INVERSE  FFT  **   


																	// do windowing and 'ADD'  to output accumulator ( this is the 'ADD' part to  "OverlapAndAdd"  ) 

			for(  k=0;   k < fftFrameSize;   k++  ) 
			{
				window                   =      -0.5    *   cos(  2.0 * M_PI   * (double)k  /  (double)fftFrameSize  )    + 0.5;

				m_gOutputAccumLeft[ k ]  +=       2.  *  window  *  m_gFFTworksp[ 2*k ]  /  ( fftFrameSize2 * osamp );   //  ***** NOTE:   +=    ...it is SUMMED.
			}
		/////////////////////////////////////////
    


		/////////////////////////////////////////

			memset(    m_gFFTworksp,    0,      2 * fftFrameSize * sizeof(float)    );    // **************   NEW,  OK ?????????   ****************


			for(    k = 0;     k <=   fftFrameSize2;      k++   ) 
			{
																	
				magn =    m_gSynMagn[ k ];      // get magnitude and true frequency from synthesis arrays 

				double   trueFreq  =    m_gSynFreq[  k ];


								 //   Looks like the   DEVIATION in the Bin's Frequency  is used to calulate a corrected Phase-Difference between FFT frames.   12/11

				double   deltaPhase  =    Calculate_True_DeltaPhase(   trueFreq,    k,    freqPerBin,    expct,    osamp   );   //   'deltaPhase'  is in RADIANS

															
				m_gSumPhaseRight[ k ]   +=    deltaPhase;    //  accumulate  delta-phase to get  bin's phase (  gSumPhase[]   is DIFFERENT for each  'rep'  )   12/2011 

				phase =    m_gSumPhaseRight[ k ];

																				
				m_gFFTworksp[ 2 * k      ]   =    magn   *   cos( phase );     // get real and imag part and re-interleave
				m_gFFTworksp[ 2 * k +1 ]   =    magn   *   sin( phase );
			} 


																							// zero negative frequencies 
			for(   k =  fftFrameSize +2;     k < (2 * fftFrameSize);       k++ ) 
				m_gFFTworksp[ k ] =   0.0;


																							
			smbFft(   m_gFFTworksp,   fftFrameSize,   1   );     // **  do the  INVERSE  FFT  **   


																	// do windowing and 'ADD'  to output accumulator ( this is the 'ADD' part to  "OverlapAndAdd"  ) 

			for(  k=0;   k < fftFrameSize;   k++  ) 
			{
				window                   =      -0.5    *   cos(  2.0 * M_PI   * (double)k  /  (double)fftFrameSize  )    + 0.5;

				m_gOutputAccumRight[ k ]  +=       2.  *  window  *  m_gFFTworksp[ 2*k ]  /  ( fftFrameSize2 * osamp );   //  ***** NOTE:   +=    ...it is SUMMED.
			}
		/////////////////////////////////////////
			




			for(  k = 0;    k < stepSize;    k++  )    // 
			{

				m_gOutFIFOLeft[    k  ]   =    m_gOutputAccumLeft[    k  ];   
				m_gOutFIFORight[  k  ]   =    m_gOutputAccumRight[  k  ];   
			}  
																															
			memmove(    m_gOutputAccumLeft,      m_gOutputAccumLeft   + stepSize,       fftFrameSize  *  sizeof(float)    );    //  shift  the accumulator 
			memmove(    m_gOutputAccumRight,    m_gOutputAccumRight + stepSize,       fftFrameSize  *  sizeof(float)    );    



																									//  move  'INPUT'   FIFO    ( needs to be OUTSIDE of the Repetition loop )
			for(   k = 0;    k < inFifoLatency;    k++ )         
			{
				m_gInFIFOLeft[   k ]  =     m_gInFIFOLeft[    k + stepSize  ];
				m_gInFIFORight[ k ]  =     m_gInFIFORight[  k + stepSize  ];
			}

		}   //  if(   gRover  >=  fftFrameSize     .... have read enough Input-Samples for FFT processing 

	}   //   for(   i = 0;    i < numSampsToProcess; 




	if(   gAnaFreqLeft   !=   NULL   )
	{
		delete   gAnaFreqLeft;
		gAnaFreqLeft =  NULL;
	}

	if(  gAnaMagnLeft    !=   NULL   )
	{
		delete   gAnaMagnLeft;
		gAnaMagnLeft =  NULL;
	}


	if(  gSynFreqLeft    !=   NULL   )
	{
		delete   gSynFreqLeft;
		gSynFreqLeft =  NULL;
	}

	if(    gSynMagnLeft  !=   NULL   )
	{
		delete   gSynMagnLeft;
		gSynMagnLeft =  NULL;
	}


	return   true;
}






											////////////////////////////////////////
											////////////////////////////////////////


bool	  FFTslowDown::Change_Samples_Pitch(   BYTE**  retSamplesPtr,    long numSamples,    bool stereoFlag,  float  pitchShift,   long  fftFrameSize,
										                                                                      long osamp,     CString&   retErrorMesg   )
{


ASSERT( 0 );    // *******************  NOT CALLED by anyone  12/2011  ************************************

					   //    Could test this function in    WavConvert::Load_Next_MemoryBlock_Frame( 



												//  assumes that  16bit samples are fed in ( in WAV format )     2/10

			//   retSamplesPtr  -   is a pointer to a pointer because calling funct will receive data from it.



//	float  pitchShift      =   0.5;    //   2 - one octave up    [ must be in range of  0.5 to 2.0  ]


//	long  fftFrameSize  =   1024;    //  512[ eh ]      256[ worse ]         Typical values are 1024, 2048 and 4096. It may be any value <= MAX_FRAME_LENGTH but it MUST be a power of 2. 


//	long osamp          =       4;    //   4[ lousey ]     16[ fails ]
											//     is the STFT  oversampling factor which also determines the overlap between adjacent STFT frames. 
							                  //    It should at least be 4 for moderate scaling ratios. A value of 32 is  recommended for best quality. 




	long   bytesPerSample =  4;    // *** HARDWIRED...  OK ??? 



	retErrorMesg.Empty();


	if(  ! stereoFlag  )
	{
		retErrorMesg =  "FFTslowDown::Change_Samples_Pitch FAILED,  can not process MONO signal." ;
		return  false;
	}

	if(      *retSamplesPtr  ==  NULL
		||   numSamples  <= 0   )
	{
		retErrorMesg =  "FFTslowDown::Change_Samples_Pitch FAILED,  bad input sample pointer." ;
		return  false;
	}




	bool   lessThanOneValues =   true;

	float   *reSampleArrayLeft   =  NULL;     
	float   *reSampleArrayRight =   NULL;    //     the two are for RE-Sampling              2 intermediate samples
	long     retByteCount = 0;

	long    allocateSize   =     sizeof( float )  *  numSamples;




	if(     (  reSampleArrayLeft =   ( float* )malloc( allocateSize )   )     == NULL    )  
	{
		retErrorMesg =    "FFTslowDown::Change_Samples_Pitch  failed,  could not allocate  retSampleArrayLeft."  ;
		return  false;
	}

	if(     (  reSampleArrayRight =   ( float* )malloc( allocateSize )   )     == NULL    )  
	{
		retErrorMesg =  "FFTslowDown::Change_Samples_Pitch  failed,  could not allocate  retSampleArrayRight."  ;
		return false;
	}





	long   totalBytes      =   numSamples  *  bytesPerSample;
	float  sampleRateFl  =  (float)(  DEFAULTsAMPLINGrATEmp3   );






	if(   ! SndSample::Make_Float_Sample(  0,    (char*)(*retSamplesPtr),    totalBytes,   bytesPerSample,    reSampleArrayLeft,   	 lessThanOneValues,   retErrorMesg  )   )
		return  false;
	

	if(   ! Shift_Pitch(                  false,    pitchShift,    numSamples,  fftFrameSize,    osamp,    sampleRateFl,  	reSampleArrayLeft,  reSampleArrayLeft,     retErrorMesg  )   )
//	if(   ! Shift_Pitch_Stripped(    false,    numSamples,  fftFrameSize,    osamp,    sampleRateFl,  	reSampleArrayLeft,  reSampleArrayLeft,     retErrorMesg  )   )
	{	
		ASSERT( 0 );
		return  false;
	}




	if(   ! SndSample::Make_Float_Sample(  1,   (char*)(*retSamplesPtr),    totalBytes,   bytesPerSample,    reSampleArrayRight,   lessThanOneValues,   retErrorMesg  )   )
		return  false;


	if(   ! Shift_Pitch(                  true,    pitchShift,    numSamples,  fftFrameSize,    osamp,    sampleRateFl,  	reSampleArrayRight,  reSampleArrayRight,     retErrorMesg  )   )
//	if(   ! Shift_Pitch_Stripped(    true,    numSamples,  fftFrameSize,    osamp,    sampleRateFl,  	reSampleArrayRight,  reSampleArrayRight,     retErrorMesg  )   )
	{	
		ASSERT( 0 );
		return  false;
	}


	double  volumeTweak =  1.0;    //   Get_Speeds_Volume_Tweak_Factor();

	bool   noMemoryRelease  =   false; 


	if(    ! SndSample::Merge_Float_Channels_To_Sample(   true,    numSamples,     bytesPerSample,      &reSampleArrayLeft,   &reSampleArrayRight,  
		                                                                                   (char**)retSamplesPtr,      retByteCount,    lessThanOneValues,  noMemoryRelease,  volumeTweak,  retErrorMesg )   )
	{
		ASSERT( 0 );
		return  false;
	}




	if(  reSampleArrayLeft  !=  NULL   )				//  **** NOT really necessary because Merge_Float_Channels_To_Sample() aALREADY freed the mem block
	{	free( reSampleArrayLeft );   reSampleArrayLeft =  NULL;    }

	if(  reSampleArrayRight  !=  NULL   )
	{	free( reSampleArrayRight );   reSampleArrayRight =  NULL;    }


	return  true;
}






											////////////////////////////////////////
											////////////////////////////////////////
											////////////////////////////////////////


void   FFTslowDown::smbFft(   float *fftBuffer,   long fftFrameSize,   long signDirection   )
{


/* 
	FFT routine,        Sign =    -1 is FFT,   1 is iFFT (inverse)

	Fills fftBuffer[0...2*fftFrameSize-1] with the Fourier transform of the
	time domain data in fftBuffer[0...2*fftFrameSize-1]. The FFT array takes
	and returns the cosine and sine parts in an interleaved manner, ie.
	fftBuffer[0] = cosPart[0], fftBuffer[1] = sinPart[0], asf. 
	
	fftFrameSize  must be a power of 2. 
	
	It expects a complex input signal (see footnote 2),
	ie. when working with 'common' audio signals our input signal has to be
	passed as 
	               {   in[0],  0.0,      in[1],  0.0,    in[2],   0.0,   ...   } asf. 
	
	In that case, the transform
	of the frequencies of interest is in fftBuffer[0...fftFrameSize].
*/


	float wr, wi, arg,   temp;

	float tr, ti, ur, ui, *p1r, *p1i, *p2r, *p2i;

	long i, bitm, j, le, le2, k;


	float   *p1, *p2;




	for(   i = 2;     i <  2*fftFrameSize-2;    i += 2   )                               //    BIT REVERSE
	{

		for(     bitm = 2, j = 0;      bitm <  2*fftFrameSize;     bitm <<= 1     ) 
		{
			if (i & bitm)   j++;

			j <<= 1;
		}



		if(  i < j  ) 
		{
			p1 =		fftBuffer    + i; 
			p2 =		fftBuffer    + j;


			temp =    *p1;   

			*( p1++ ) =   *p2;
			*( p2++ ) =   temp; 


			temp =   *p1;

			*p1 =   *p2; 
			*p2 =    temp;
		}
	}




	double   logFftFrameSize  =    log(    (double)fftFrameSize    );			  //    BUTTERFLIES  

//	long       kLimit  =  (long)(     log( fftFrameSize )  /  log(2.)    +   .5   );
	long       kLimit  =  (long)(     logFftFrameSize      /  log(2.0)  +  0.5   );


//	for (  k = 0,  le = 2;       k < (long)(log(fftFrameSize)/log(2.)+.5);    k++     )     ...original
	for (  k = 0,  le = 2;       k < kLimit;                                                k++     )
	{
		le  <<=  1;

		le2 =   le>>1;


		ur =    1.0;
		ui =    0.0;

		arg =  M_PI / (le2>>1);


		wr =   cos( arg );

		wi =   signDirection  *  sin( arg );   //   The only time  'sign'



		for(  j = 0;     j < le2;    j += 2   ) 
		{

			p1r =  fftBuffer +j;     p1i = p1r +1;

			p2r =  p1r + le2;         p2i = p2r +1;


			for (   i = j;   i <   2*fftFrameSize;    i += le    ) 
			{
				tr =  *p2r * ur    -    *p2i * ui;
				ti =  *p2r * ui    +    *p2i * ur;


				*p2r =   *p1r   - tr; 
				*p2i =    *p1i   - ti;

				*p1r  +=  tr;   
				*p1i  +=   ti;

				p1r  +=   le; 
				p1i  +=   le;

				p2r  +=  le; 
				p2i  +=   le;
			}



			tr =   ur * wr   -   ui*wi;

			ui =   ur * wi   +   ui*wr;

			ur =   tr;
		}
	}
}






											////////////////////////////////////////


double    FFTslowDown::smbAtan2(  double x, double y  )
{

/*
    12/12/02    
    
    PLEASE NOTE:
    
    There have been some reports on domain errors when the atan2() function was used
    as in the above code. Usually, a domain error should not interrupt the program flow
    (maybe except in Debug mode) but rather be handled "silently" and a global variable
    should be set according to this error. However, on some occasions people ran into
    this kind of scenario, so a replacement atan2() function is provided here.
    
    If you are experiencing domain errors and your program stops, simply replace all
    instances of atan2() with calls to the smbAtan2() function below.
    
*/

  double signx;

  if (x > 0.) 
	  signx = 1.;  
  else 
	  signx = -1.;
  

  if (x == 0.) 
	  return 0.;

  if (y == 0.) 
	  return   signx  *   M_PI / 2.;
  
  return   atan2( x,  y );
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////



bool    FFTslowDown::FastSharpen_Column(    bool  rightStereo,    long numSampsToProcess,     long fftFrameSize,       float sampleRate,      																																								
																															float *indataLeft,   float *outdataLeft,    CString&   retErrorMesg    )
{

		// ***************   TEMP FUNCTION   ******************************


					//  OLD PARMS:  		short x,     SndSample  *srcSample,    SndSample  *destSample
 
//   short     row,   outSh;
    long       i,   halfn, totSize;		
    double   wt,   *xr,*xi;    

//  char      *dst,   *src;
	float		*dst=NULL,    *src=NULL;

   

	if(   m_gInFIFOLeft  ==  NULL  )
	{
		retErrorMesg  =     "FFTslowDown::FastSharpen_Column  FAILED,   SlowDown BUFFERS were NOT allocated."  ;
		return   false;
	}


    if(       indataLeft   ==  NULL  
		||   outdataLeft  ==  NULL    )              
	{
		ASSERT( 0 );
		retErrorMesg =    "FFTslowDown::FastSharpen_Column  FAILED,  input arrays zare NULL." ;
		return  false;
	}


	ASSERT(  m_playSpeed  ==   1.0 );      // ************************   TEMP



	long  gRoverLocal  =  -1;


	float    *gInFIFO=NULL,   *gOutFIFO=NULL,    *gLastPhase=NULL,    *gSumPhase=NULL;     //   *gOutputAccum=NULL,

	if(   rightStereo   )     //  save values for next entrance to this function,  or we will hear 'static'  from variable confusion from switching from left to right stereo
	{
		gInFIFO       =     m_gInFIFORight;
		gOutFIFO      =    m_gOutFIFORight;
	
	//	gOutputAccum =    m_gOutputAccumRight;  
		gLastPhase  =      m_gLastPhaseRight;
		gSumPhase =      m_gSumPhaseRight;

		gRoverLocal =   m_gRoverRight;
	}
	else
	{  gInFIFO      =     m_gInFIFOLeft;
		gOutFIFO    =     m_gOutFIFOLeft;
	
	//	gOutputAccum =    m_gOutputAccumLeft;  
		gLastPhase  =      m_gLastPhaseLeft;
		gSumPhase =      m_gSumPhaseLeft;
	
		gRoverLocal =   m_gRoverLeft;
	}



	
																					// initialize our persisting arrays 
	if(    ! m_areSlowDownBuffersInitialized    )
	{
				
		/****
		Erase_SlowDown_Buffers();

		m_gRoverLeft  =   m_gRoverRight =    inFifoLatency;

		m_areSlowDownBuffersInitialized =    true;      //  this is a little controversial,  but seems to work OK	
		****/
		Initialize_SlowDown_Variables();


		gRoverLocal  =  0;
	}




ASSERT( 0 );   // NEED to dynamically memory for    double  wkReal[ 8192   ]      wkImag[ 8192 ]     8/2012
 //   xr  =    &(     wkReal[ 0 ]     );       
//	xi  =    &(    wkImag[ 0 ]     );  
   xr  =   xi  =     NULL;    // ***** KEEP COMPILER HAPPY ******



 
    totSize =     fftFrameSize;    //  _timeWidth;      

	halfn    =     totSize / 2L;
		
	long   sizeOfVertialComponents  =     fftFrameSize /2   + 1;    //   OR    fftFrameSize   ??? 

          



	for(    long  s= 0;     s <  numSampsToProcess;     s++   )
	{

																					  	// As long as we have not yet collected enough data just read in 
		/***															                
		m_gInFIFOLeft[    m_gRoverLeft  ]    =   indataLeft[  s  ];
		outdataLeft[  s  ]   =    m_gOutFIFOLeft[     m_gRoverLeft    -  inFifoLatency   ];    // Think I need to first read all input before I write out ???   12/11
		***/							
		gInFIFO[   gRoverLocal   ]  =       indataLeft[  s  ];    

		outdataLeft[  s  ]               =       gOutFIFO[   gRoverLocal   ];   


		gRoverLocal++;    //  must increment it here,  before it possibly gets RE-INITOIALIZED in the block below.   1/12



																		
		if(    gRoverLocal    >=    fftFrameSize    )      // now we have enough data for processing 

		{

		//	m_gRoverLeft =    m_gRoverRight  =     inFifoLatency;    //  re-initialization of this TRAVERSING index
			gRoverLocal =   0;    //   re-initialization of this TRAVERSING index,     this is for  the next entry to this loop
			


  													 //  convert integer sampleData( and center it ) to FRACarray for input

		//  src =    (char*)(    srcSample->Get_StartByte()     )       +        (   (long)x  *  srcSample->_chunkSize   );  
			src  =    &(    gInFIFO[ 0 ]     ); 

		    
			for(   i= 0L;     i <  totSize;      i++   )
			{  
	  //		  xr[ i ]   =     *src;
			//   xr[ i ]   =    (double)(  *src  );      //   use decimal
		//		 gOutFIFO[  i ]  =    gInFIFO[ i ];    *** WORKS,  if comment out bottom
			  xr[ i ]   =   gInFIFO[ i ]; 

			   src++; 
			} 									



																							//   ****  Do the TRANSFORM to Freq-Domain  ****


			 for(    i= 0;     i<  halfn;     i++    )          // Because we take advantage of real input, pack 
			 {																											  // xr[], xi[]  vals:  1.0 to -1.0


				  xr[ i ]  =    xr[  (  2L * i  )             ];           //    Even  terms in real part and... 
				  xi[ i ]  =    xr[   ( 2L * i  )   +  1L   ];           //    Odd  terms  in imaginary part.


			 }

							// ******* ???  Need to fill LATTER part of array with zeroes ???   9/03 *********



// *************  Need to ReInstall this line,  dynamically memory for  xr,  xi  *******    8/2012
		            
//			 Real_FFT_doubles(   (int)halfn,    xr,  xi    );       //  real_fft(   (int)halfn,   xrDB,  xiDB  );   
		   
		 



																				//  *** Convert to  [ mag, phase ]


			memset(  m_gAnaFreq,          0,   sizeOfVertialComponents  * sizeof(float)  );
			memset(  m_gAnaMagn,         0,   sizeOfVertialComponents  * sizeof(float) );



			for(   i=  1L;     i<  halfn;      i++    )			
			{

				double   realVal   =   xr[ i ];      //   *   wt;      // **** ????  NEED wt  ????   *******************
				double   imagVal  =   xi[ i ];      //   *   wt;   


				double   mag  =     sqrt(     (realVal * realVal)   +   ( imagVal * imagVal )     ); 
				double   ang  =      atan2(  imagVal,   realVal  ); 


ASSERT( 0 );   // NEED to dynamically memory for    double  magsFFT[ 8192   ]      phaseFFT[ 8192 ]     8/2012

//				magsFFT[  i ]  =    mag;
//				phaseFFT[ i ]  =    ang;		


			}

/***
			magsFFT[ 0 ] =   phaseFFT[ 0 ] =    0.0;
			magsFFT[ halfn ] =   phaseFFT[ halfn ] =    magsFFT[ halfn +1 ] =   phaseFFT[ halfn +1 ] =   0.0;
			magsFFT[ halfn +2 ] =   phaseFFT[ halfn +2 ] =   magsFFT[ halfn +3 ] =   phaseFFT[ halfn +3 ] =    0.0;
***/



																				//   apply CONVOLUTION filter( sharpen ) to mags

			for(   i=  2L;     i<  halfn;      i++    )			
			{

				double   sum =   0;    //     magsFFT[  i  ];        // NEED to dynamically memory for    double  magsFFT[ 8192   ]      phaseFFT[ 8192 ]     8/2012
				
				// ************   JUST do a straight copy for test    1/12



ASSERT( 0 );   // NEED to dynamically memory for    double  magsFFT[ 8192   ]      phaseFFT[ 8192 ]     8/2012
			//	xr[ i ]  =      sum    *    cos(    phaseFFT[ i ]    );
			//	xi[ i ]  =       sum    *    sin(    phaseFFT[ i ]    );
			}





																			    //  ****  do  'INVERSE TRANSFORM'   for  ReConstruction  ****
		    

			wt    =    2.0;             // Real and imaginary parts WOULD have reinforced(so simulate with 2x )

			wt   /=    (double)totSize;  // must scale down by 'n', before or after transform
		 //   wtFrc  =  (long)( wt * VolFact  * SPFRACFACT );   // Volume factor & make 'Fract Point'




			for(   i=  1L;     i<  halfn;      i++    )			// NOTE:   data is in RIGHT half of array,  
			{													         //             and LEFT half was ZEROEDout( to stop wraparround )       

				 xr[  totSize - i  ]  =      wt   *   xr[ i ];    
				   
		//	    xi[  totSize - i  ]  =     -wt   *   xi[ i ];    // sign change(-) cause this is now at UPPER end   ...NOT in Book[ pp209 ]
				xi[  totSize - i  ]  =      wt   *   xi[ i ];


				xr[ i ]  =   0.0;   
				xi[ i ]  =    0.0;   
			}  
		   
		   xr[ 0 ] =  xi[ 0 ] =  xr[ halfn ] =  xi[ halfn ] =     0.0;     // Also zero these



	//   ***********  now done  ABOVE  when applying the 'weight'    								      
	//	   for( i=0;  i< totSize;  i++ )    
	//		 {  xi[i]=  xi[i] / (double)totSize; // must scale down by 'n', before or after transform
	//			xr[i]=  xr[i] / (double)totSize;   
	//		 }		 
		        
	 //   Not in Book,  is this done with the weight ???   9/03

	//	   for(   i= 0;    i <  totSize;    i++  )   
	//		   xi[ i ] =   -xi[ i ];						 // '-' FLIP SIGN of imaginary part
		   


								  //  ****  do  'INVERSE TRANSFORM'  with foward routine, data here is 0.28 to -0.32( in Frac form )  ****  

		// FFT_fix(           (int)totSize,   xr, xi     );   



// *************  Need to ReInstall this line,  dynamically memory for  xr,  xi  *******    8/2012
//		   FFT_doubles(    (int)totSize,   xr, xi     );   
		   
		   


		   for(   i= 0;    i<  totSize;    i++   )   
		   {  
			   xi[ i ] =    -xi[ i ];         // '-'  to FLIP SIGN of imaginary part BACK

											//  Get_doubles_MinMax(   (double)( xr[i]  ) / SPFRACFACT   );   // *** TEMP ****
		   }





																				 //   write RESULTS back to sample  
		                        				

	//     dst =     destSample->Get_StartByte()    +     (long)x  *  destSample->_chunkSize;   
		   dst  =     &(   gOutFIFO[ 0 ]     );    // ****************   OK ???   ****************************


		   for(    i= 0;      i<  totSize;     i++    )     
		   {  
			//	*dst  =    (float)(   xr[ i ]   );
				gOutFIFO[ i ]  =    xr[ i ];

				dst++;  
			}


		}   //   if(    m_gRoverLeft   >=   fftFrameSize    )  


	}    //   for(  long  s = 0;    s < numSampsToProcess




	if(   rightStereo   )				         //   save values for next entrance to this function,  or we will hear audio 'static'  from variable confusion 
		m_gRoverRight =     gRoverLocal;       //   from switching from left to right stereo during realtime play
	else
		m_gRoverLeft   =     gRoverLocal;


	return  true;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////



bool    FFTslowDown::Fast_Filter_MemBlock(    bool  rightStereo,    long numSampsToProcess,     long fftFrameSize,       float sampleRate,      																																								
																															float *indataLeft,   float *outdataLeft,    CString&   retErrorMesg    )
{

					//  OLD PARMS:  		short x,     SndSample  *srcSample,    SndSample  *destSample
 
//   short     row,   outSh;
    long       i,   halfn, totSize;		
    double   wt,   *xr,*xi;    

//  char      *dst,   *src;
	float		*dst=NULL,    *src=NULL;

   

	if(   m_gInFIFOLeft  ==  NULL  )
	{
		retErrorMesg  =     "FFTslowDown::Fast_Filter_MemBlock  FAILED,   SlowDown BUFFERS were NOT allocated."  ;
		return   false;
	}


    if(       indataLeft   ==  NULL  
		||   outdataLeft  ==  NULL    )              
	{
		ASSERT( 0 );
		retErrorMesg =    "FFTslowDown::Fast_Filter_MemBlock  FAILED,  input arrays zare NULL." ;
		return  false;
	}


	ASSERT(  m_playSpeed  ==   1.0 );      // ************************   TEMP



	long  gRoverLocal  =  -1;


	float    *gInFIFO=NULL,   *gOutFIFO=NULL,    *gLastPhase=NULL,    *gSumPhase=NULL;     //   *gOutputAccum=NULL,

	if(   rightStereo   )     //  save values for next entrance to this function,  or we will hear 'static'  from variable confusion from switching from left to right stereo
	{
		gInFIFO       =     m_gInFIFORight;
		gOutFIFO      =    m_gOutFIFORight;
	
	//	gOutputAccum =    m_gOutputAccumRight;  
		gLastPhase  =      m_gLastPhaseRight;
		gSumPhase =      m_gSumPhaseRight;

		gRoverLocal =   m_gRoverRight;
	}
	else
	{  gInFIFO      =     m_gInFIFOLeft;
		gOutFIFO    =     m_gOutFIFOLeft;
	
	//	gOutputAccum =    m_gOutputAccumLeft;  
		gLastPhase  =      m_gLastPhaseLeft;
		gSumPhase =      m_gSumPhaseLeft;
	
		gRoverLocal =   m_gRoverLeft;
	}


	
																					// initialize our persisting arrays 
	if(    ! m_areSlowDownBuffersInitialized    )
	{
				
		/****
		Erase_SlowDown_Buffers();
		m_gRoverLeft  =   m_gRoverRight =    inFifoLatency;
		m_areSlowDownBuffersInitialized =    true;      //  this is a little controversial,  but seems to work OK	
		****/
		Initialize_SlowDown_Variables();


		gRoverLocal  =  0;
	}




ASSERT( 0 );   // NEED to dynamically memory for    double  wkReal[ 8192   ]      wkImag[ 8192 ]     8/2012

/***
    xr  =    &(     wkReal[ 0 ]     );       
	xi  =    &(    wkImag[ 0 ]     );  
***/    
   xr  =   xi  =     NULL;    // ***** temp,   KEEP COMPILER HAPPY  8/2012  ******






    totSize =     fftFrameSize;    //  _timeWidth;      

	halfn    =     totSize / 2L;

		
	long   sizeOfVertialComponents  =    fftFrameSize /2   + 1;    //   OR    fftFrameSize   ??? 


	//////////////////////////
	double   magn, phase,  real, imag;
	double   freqPerBin,   expct;
	long      k,  index, fftFrameSize2,   osamp;
//	long      stepSize; 


	fftFrameSize2 =    fftFrameSize /2;

	freqPerBin      =   sampleRate  /  (double)fftFrameSize;





	float  pitchShift  =   1.0   ;    //   1.0    .5     *******************************   SWITCH *********************************





///////////////////////////////////////  ???????????????????????????????????????
	osamp    =  1;    //  was  8   (  like 1/8  of the frame size is the  'stepSize'
																		// set up some handy variables

//	stepSize         =    fftFrameSize  /  osamp;       //   as osamp  gets bigger,    stepSize gets smaller
//	stepSize    =   0;


//	expct              =    2.0  *  M_PI  * (double)stepSize / (double)fftFrameSize;   *** ORIG ***

	expct              =    2.0  *  M_PI  *  1.0;   // ************************************************* ????  ******************* 

//	expct              =    2.0  *  M_PI  *  .25;


///////////////////////////////////////  ???????????????????????????????????????

          



	for(    long  s= 0;     s <  numSampsToProcess;     s++   )
	{

																					  	// As long as we have not yet collected enough data just read in 
		/***															                
		m_gInFIFOLeft[    m_gRoverLeft  ]    =   indataLeft[  s  ];
		outdataLeft[  s  ]   =    m_gOutFIFOLeft[     m_gRoverLeft    -  inFifoLatency   ];    // Think I need to first read all input before I write out ???   12/11
		***/							
		gInFIFO[   gRoverLocal   ]  =       indataLeft[  s  ];    

		outdataLeft[  s  ]               =       gOutFIFO[   gRoverLocal   ];   


		gRoverLocal++;    //  must increment it here,  before it possibly gets RE-INITOIALIZED in the block below.   1/12



																		
		if(    gRoverLocal    >=    fftFrameSize    )      // now we have enough data for processing 

		{

		//	m_gRoverLeft =    m_gRoverRight  =     inFifoLatency;    //  re-initialization of this TRAVERSING index
			gRoverLocal =   0;    //   re-initialization of this TRAVERSING index,     this is for  the next entry to this loop
			


			memset(  xr,    0,     fftFrameSize * sizeof(double)   );     //  Might be unecessary,  but play it safe
			memset(  xi,     0,    fftFrameSize * sizeof(double)   );


  													 //  convert integer sampleData( and center it ) to FRACarray for input

		//  src =    (char*)(    srcSample->Get_StartByte()     )       +        (   (long)x  *  srcSample->_chunkSize   );  
			src  =    &(    gInFIFO[ 0 ]     ); 

		    
			for(   i= 0L;     i <  totSize;      i++   )
			{  
	  //		  xr[ i ]   =     *src;
			//   xr[ i ]   =    (double)(  *src  );      //   use decimal
		//		 gOutFIFO[  i ]  =    gInFIFO[ i ];    *** WORKS,  if comment out bottom
			  xr[ i ]   =   gInFIFO[ i ]; 

			   src++; 
			} 									


																							//   ****  Do the TRANSFORM to Freq-Domain  ****


			 for(    i= 0;     i<  halfn;     i++    )          // Because we take advantage of real input, pack 
			 {																											  // xr[], xi[]  vals:  1.0 to -1.0

				  xr[ i ]  =    xr[  (  2L * i  )             ];           //    Even  terms in real part and... 
				  xi[ i ]  =    xr[   ( 2L * i  )   +  1L   ];           //    Odd  terms  in imaginary part.
			 }

							// ******* ???  Need to fill LATTER part of array with zeroes ???   9/03 *********

		            


// // *************  Need to ReInstall this line,  dynamically memory for  xr,  xi  *******    8/2012
//			 Real_FFT_doubles(   (int)halfn,    xr,  xi    );       //  real_fft(   (int)halfn,   xrDB,  xiDB  );   
		   
		 



																				//  *** Convert to  [ mag, phase ]


			memset(  m_gAnaFreq,          0,   sizeOfVertialComponents  * sizeof(float)  );
			memset(  m_gAnaMagn,         0,   sizeOfVertialComponents  * sizeof(float) );



			for(   k=  1L;     k<  halfn;      k++    )		
	//		for(   k=    0;     k<  halfn;      k++    )	 // ***********   OK ?????????????  *******************************	
			{

				real   =   xr[ k ];      //   *   wt;      // **** ????  NEED wt  ????   *******************
				imag  =   xi[ k ];      //   *   wt;   

				/****
				double   mag  =     sqrt(     (real * real)   +   ( imag * imag )     ); 
				double   ang  =      atan2(  imag,   real  ); 

				magsFFT[  i ]  =    mag;
				phaseFFT[ i ]  =    ang;		
				****/

		//		magn =    2.0  *   sqrt(  real*real  +   imag*imag  );    //  2.0 ???******** WHY ?????                compute magnitude and phase 
				magn =               sqrt(  real*real  +   imag*imag  );    //  *****************************



				phase =   atan2(         imag,   real   );    
		//		phase =   smbAtan2(   imag,   real   );     //      ****************   BETTER ???  ************************




				double   lastPhase =    gLastPhase[ k ];
				
				gLastPhase[ k ] =   phase;    //  save the  calculated-phase  for next iteration



					      //  Looks like they use the   DEVIATION in the EXPECTED-Phase   to calculate the  TRUE-Frequency  that is present in the Bin.     12/11

				double   trueFreq  =     Calculate_True_Frequency(    phase,    lastPhase,    k,    freqPerBin,    expct,    osamp  );

		//		double   approxFreq =        (double)k     *  freqPerBin;      ** DEBUG only 


				m_gAnaMagn[ k ]  =    magn;       //  store magnitude and true frequency in analysis arrays
				m_gAnaFreq[  k ]  =    trueFreq;  
			}



			/***
			magsFFT[ 0 ] =   phaseFFT[ 0 ] =    0.0;

			magsFFT[ halfn ] =   phaseFFT[ halfn ] =    magsFFT[ halfn +1 ] =   phaseFFT[ halfn +1 ] =   0.0;

			magsFFT[ halfn +2 ] =   phaseFFT[ halfn +2 ] =   magsFFT[ halfn +3 ] =   phaseFFT[ halfn +3 ] =    0.0;
			****/


/***  **********************  ???    WANT this ????  ****************************
			m_gAnaMagn[ 0 ] =   m_gAnaFreq[ 0 ] =    0.0;

			m_gAnaMagn[ halfn ] =   m_gAnaFreq[ halfn ]    =    m_gAnaMagn[ halfn +1 ] =   m_gAnaFreq[ halfn +1 ] =   0.0;

			m_gAnaMagn[ halfn +2 ] =   m_gAnaFreq[ halfn +2 ] =   m_gAnaMagn[ halfn +3 ] =   m_gAnaFreq[ halfn +3 ] =    0.0;
****/    // **********************  ???    WANT this ????  ****************************



																				//   apply CONVOLUTION filter( sharpen ) to mags

			memset(  m_gSynMagn,  0,   sizeOfVertialComponents * sizeof(float)   );    
			memset(  m_gSynFreq,    0,   sizeOfVertialComponents * sizeof(float)   );


			/***
			for(   i=  2L;     i<  halfn;      i++    )			
			{
				double   sum =     magsFFT[  i  ];        // ************   JUST do a straight copy for test    1/12

				xr[ i ]  =      sum    *    cos(    phaseFFT[ i ]    );
				xi[ i ]  =       sum    *    sin(    phaseFFT[ i ]    );
			}
			****/
			float   magnAnaLoc,    freqAnaLoc; 


			for (  k = 0;    k  <=  fftFrameSize2;    k++  )       //   this does the actual pitch shifting 
			{ 

				index =   k   *  (long)pitchShift;		//  going to VERTICALLY  shift the  Mag and Freq  values  to other   FFT-frequencyRows, changes the pitch


		//		if(   index   <=   fftFrameSize2   ) 
				if(   index   <     fftFrameSize2   )    // ********* WHICH is right ???   12/20/11   **************************
				{ 

					/***
					m_gSynMagn[ index ]   +=    m_gAnaMagn[ k ]; 

					m_gSynFreq[  index ]      =    m_gAnaFreq[ k ]    * pitchShift; 
					***/

					magnAnaLoc  =    m_gAnaMagn[ k ]; 

					freqAnaLoc    =     m_gAnaFreq[ k ];


			//		if(  )
			//		{
						m_gSynMagn[ index ]   +=     magnAnaLoc; 

						m_gSynFreq[  index ]      =     freqAnaLoc   *   pitchShift; 


			//		}

					int  dummy =   9;


				} 				
			}




			memset(  xr,    0,     fftFrameSize * sizeof(double)   );    
			memset(  xi,     0,    fftFrameSize * sizeof(double)   );


			float   realLoc,    imagLoc; 


			for(    k = 0;     k <=   fftFrameSize2;      k++   ) 
			{
																
				magn =    m_gSynMagn[ k ];      // get magnitude and true frequency from synthesis arrays 

				double   trueFreq  =    m_gSynFreq[  k ];


				double   expectedFreq =       (double)k        *  freqPerBin;    // ****************  TEMP  DEBUG  *****************  




					         //   Looks like the   DEVIATION in the Bin's Frequency  is used to calulate a corrected Phase-Difference between FFT frames.   12/11

				double   deltaPhase  =    Calculate_True_DeltaPhase(   trueFreq,    k,    freqPerBin,    expct,    osamp   );   //   'deltaPhase'  is in RADIANS

														
				gSumPhase[ k ]   +=    deltaPhase;     //  accumulate  delta-phase  to get  bin's phase 

				phase =    gSumPhase[ k ];

				/****															
				m_gFFTworksp[ 2 * k      ]   =    magn   *   cos( phase );     // get real and imag part and re-interleave
				m_gFFTworksp[ 2 * k +1 ]   =    magn   *   sin( phase );


				double   sum =     magsFFT[  i  ];        // ************   JUST do a straight copy for test    1/12
				xr[ i ]  =      sum    *    cos(    phaseFFT[ i ]    );
				xi[ i ]  =       sum    *    sin(    phaseFFT[ i ]    );
				****/

			
				if(   k  >=   2  )    //  try to copy logic from above   
				{
					/****
					xr[  k  ]  =      magn   *   cos( phase );  
					xi[  k  ]  =      magn   *   sin( phase );
					****/
					realLoc   =      magn   *   cos( phase );  
					imagLoc  =      magn   *   sin( phase );

					xr[  k  ]  =      realLoc;  
					xi[  k  ]  =      imagLoc;

					int  dummy =   9;
				}

			} 



																			    //  ****  do  'INVERSE TRANSFORM'   for  ReConstruction  ****
		    

			wt    =    2.0;             // Real and imaginary parts WOULD have reinforced(so simulate with 2x )


			wt   /=    (double)totSize;  // must scale down by 'n', before or after transform   [   ***** More ACCURACY if done BELOW  1/2012 ???? ******************





			for(   i=  1L;     i<  halfn;      i++    )			// NOTE:   data is in RIGHT half of array,  
			{													            //             and LEFT half was ZEROEDout( to stop wraparround )       

				   
		//	    xi[  totSize - i  ]  =     -wt   *   xi[ i ];    // sign change(-) cause this is now at UPPER end   ...NOT in Book[ pp209 ]
				xi[  totSize - i  ]  =      wt   *   xi[ i ];


				xr[ i ]  =   0.0;   
				xi[ i ]  =    0.0;   
			}  
	   


		   xr[ 0 ] =  xi[ 0 ] =  xr[ halfn ] =  xi[ halfn ] =     0.0;     // Also zero these



	//   ***********  now done  ABOVE  when applying the 'weight'    ***** More ACCURACY if done down here  1/2012 ???? *********************								      
	//	   for( i=0;  i< totSize;  i++ )    
	//		 {  xi[i]=  xi[i] / (double)totSize; // must scale down by 'n', before or after transform
	//			xr[i]=  xr[i] / (double)totSize;   
	//		 }		 
		        
	 //   Not in Book,  is this done with the weight ???   9/03


	//	   for(   i= 0;    i <  totSize;    i++  )   
	//		   xi[ i ] =   -xi[ i ];						 // '-' FLIP SIGN of imaginary part
		   


								  //  ****  do  'INVERSE TRANSFORM'  with foward routine, data here is 0.28 to -0.32( in Frac form )  ****  

		// FFT_fix(           (int)totSize,   xr, xi     );   




// *************  Need to ReInstall this line,  dynamically memory for  xr,  xi  *******    8/2012
//		   FFT_doubles(    (int)totSize,   xr, xi     );   
		   
		   


		   for(   i= 0;    i<  totSize;    i++   )   // ******** WHY do I do this when it has no consequence ???   1/12 *************************
		   {  
			   xi[ i ] =    -xi[ i ];         // '-'  to FLIP SIGN of imaginary part BACK

											//  Get_doubles_MinMax(   (double)( xr[i]  ) / SPFRACFACT   );   // *** TEMP ****
		   }




																				 //   write RESULTS back to sample  
		                        				

	//     dst =     destSample->Get_StartByte()    +     (long)x  *  destSample->_chunkSize;   
		   dst  =     &(   gOutFIFO[ 0 ]     );    // ****************   OK ???   ****************************


		   for(    i= 0;      i<  totSize;     i++    )     
		   {  
			//	*dst  =    (float)(   xr[ i ]   );
				gOutFIFO[ i ]  =    xr[ i ];

				dst++;  
			}

		}   //   if(    m_gRoverLeft   >=   fftFrameSize    )  


	}    //   for(  long  s = 0;    s < numSampsToProcess




	if(   rightStereo   )				         //   save values for next entrance to this function,  or we will hear audio 'static'  from variable confusion 
		m_gRoverRight =     gRoverLocal;       //   from switching from left to right stereo during realtime play
	else
		m_gRoverLeft   =     gRoverLocal;


	return  true;
}





											////////////////////////////////////////


bool     FFTslowDown::Shift_Pitch_Horizontally_OLDfft(  bool  rightStereo,   float pitchShift,     long numSampsToProcess,    long fftFrameSize,     
											                                    long osampDummy,   float sampleRate,      float *indata,   float *outdata,      CString&   retErrorMesg  )   
{


	// **************************   MAYBE Omit,  it was a FAILED experiment   1/2012  ********************************************
	//
	//		I though I could do SlowDown without  "Overlap and Add"   ...but that seems impossible  ***************************************
	//
	// **************************   MAYBE Omit,  it was a FAILED experiment   1/2012  ********************************************



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

//	sampleRate -    takes the sample rate for the signal  in unit Hz,   ie. 44100  for  44.1 kHz  audio. 


//	The data passed to the routine in  indata[] should be in the range [ -1.0, 1.0 ],  which is also the output range 
 //	for the data, make sure you scale the data accordingly ( for 16bit signed integers you would have to divide (and multiply) by 32768 ). 


	retErrorMesg.Empty();



	if(   m_gInFIFOLeft  ==  NULL  )
	{
		retErrorMesg  =     "FFTslowDown::Shift_Pitch_Horizontally_OLDfft  FAILED,   SlowDown BUFFERS were NOT allocated."  ;
		return   false;
	}


	if(       indata   ==  NULL  
		||   outdata  ==  NULL    )              
	{
		ASSERT( 0 );
		retErrorMesg =    "FFTslowDown::Shift_Pitch_Horizontally_OLDfft  FAILED,  input arrays zare NULL." ;
		return  false;
	}



	long   slowDownSpeed =   (long)pitchShift;   // ***********   NEED test that it must be a Integer   12/11 **********************


	bool   useAlternateReconstruct =  true;    //   true    *********   SWITCH,  both are pretty crappy.  ********************




	long  gRoverLocal  =  -1;


	float    *gInFIFO=NULL,   *gOutFIFO=NULL,    *gLastPhase=NULL,    *gSumPhase=NULL;     //   *gOutputAccum=NULL,

	if(   rightStereo   )     //  save values for next entrance to this function,  or we will hear 'static'  from variable confusion from switching from left to right stereo
	{
		gInFIFO       =     m_gInFIFORight;
		gOutFIFO      =    m_gOutFIFORight;
	
	//	gOutputAccum =    m_gOutputAccumRight;  
		gLastPhase  =      m_gLastPhaseRight;
		gSumPhase =      m_gSumPhaseRight;

		gRoverLocal =   m_gRoverRight;
	}
	else
	{  gInFIFO      =     m_gInFIFOLeft;
		gOutFIFO    =     m_gOutFIFOLeft;
	
	//	gOutputAccum =    m_gOutputAccumLeft;  
		gLastPhase  =      m_gLastPhaseLeft;
		gSumPhase =      m_gSumPhaseLeft;
	
		gRoverLocal =   m_gRoverLeft;
	}


																					// initialize our persisting arrays 
	if(    ! m_areSlowDownBuffersInitialized    )
	{				
		/****
		Erase_SlowDown_Buffers();
		m_gRoverLeft  =   m_gRoverRight =    inFifoLatency;
		m_areSlowDownBuffersInitialized =    true;      //  this is a little controversial,  but seems to work OK	
		****/
		Initialize_SlowDown_Variables();

		gRoverLocal  =  0;
	}




    long       i,   halfn, totSize;		
    double   wt,   *xr,*xi;    
	float		*dst =NULL,    *src =NULL;





ASSERT( 0 );   // NEED to dynamically memory for    double  wkReal[ 8192   ]      wkImag[ 8192 ]     8/2012
 //   xr  =    &(     wkReal[ 0 ]     );       
//	xi  =    &(    wkImag[ 0 ]     );  
  xr  =   xi  =     NULL;    // ***** temp,   KEEP COMPILER HAPPY  8/2012  ******    
 




    totSize =     fftFrameSize;    //  _timeWidth;      

	halfn    =     totSize / 2L;

		
	long   sizeOfVertialComponents  =    fftFrameSize /2   + 1;    //   OR    fftFrameSize   ??? 


	//////////////////////////
	double   magn,  phase,    real, imag;
	double   freqPerBin,    expct;
	long      k,   fftFrameSize2,   osamp;
//	long      stepSize,   index; 
//	double    window;


	fftFrameSize2 =    fftFrameSize /2;

	freqPerBin      =   sampleRate  /  (double)fftFrameSize;



///////////////////////////////////////  ???????????????????????????????????????
	osamp    =  1;    //  was  8   (  like 1/8  of the frame size is the  'stepSize'
																		// set up some handy variables

//	stepSize         =    fftFrameSize  /  osamp;       //   as osamp  gets bigger,    stepSize gets smaller
//	stepSize    =   0;


//	expct              =    2.0  *  M_PI  * (double)stepSize / (double)fftFrameSize;   *** ORIG ***

	expct              =    2.0  *  M_PI  *  1.0;    // ************************* ????  ******************* 

///////////////////////////////////////  ???????????????????????????????????????




																										// main processing loop 
	for(  long  s = 0;    s< numSampsToProcess;    s++  )
	{
																		// As long as we have not yet collected enough data just read in 

		gInFIFO[   gRoverLocal   ]  =     indata[  s  ];    



		long   oldIndexRover         =    gRoverLocal;    //    gRover  -  inFifoLatency;    

		long   outputSampleIndex  =     s   *    slowDownSpeed;   //   for OUTPUT,   there will be     SlowDownSpeed  TIMES   number-of-Input-Samples  
	    


		for(   long  rp =  0;      rp < slowDownSpeed;    rp++   )
		{

			long   trueIndexRover  =     ( slowDownSpeed  *  oldIndexRover  )    +  rp;    

	//		outdataLeft[  s  ]                        =       gOutFIFO[   gRoverLocal   ];   
			outdata[  outputSampleIndex  ]    =      gOutFIFO[   trueIndexRover   ];      //   read the array-elements in  gOutFIFO[],  one right after another                  

			outputSampleIndex++;   //  Like Rover,  this is  prematurely incremented. ( for i=0,  its 2 now )     Carefull at the bottom
		}


		gRoverLocal++;




																		
		if(   gRoverLocal   >=   fftFrameSize   )     // now we have enough data for processing 
		{

	//		gRoverLocal =   inFifoLatency;    //  re-initialization of this TRAVERSING index
			gRoverLocal =   0;    //   re-initialization of this TRAVERSING index,     this is for  the next entry to this loop



			memset(  xr,    0,     fftFrameSize * sizeof(double)   );     //  Might be unecessary,  but play it safe
			memset(  xi,     0,    fftFrameSize * sizeof(double)   );



  													 //  convert integer sampleData( and center it ) to FRACarray for input


		    
			for(   i= 0L;     i <  totSize;      i++   )
			{  
				/****
				window  =    -.5 * cos(   2.*M_PI * (double)i   /  (double)fftFrameSize   ) + .5;     //  Do I need windowing ?????

				xr[ i ]   =   gInFIFO[ i ]     *  window;  
				***/
				xr[ i ]   =   gInFIFO[ i ]; 
			} 									


																							//   ****  Do the TRANSFORM to Freq-Domain  ****


			 for(    i= 0;     i<  halfn;     i++    )          // Because we take advantage of real input, pack 
			 {																											  // xr[], xi[]  vals:  1.0 to -1.0

				  xr[ i ]  =    xr[  (  2L * i  )             ];           //    Even  terms in real part and... 
				  xi[ i ]  =    xr[   ( 2L * i  )   +  1L   ];           //    Odd  terms  in imaginary part.
			 }

							// ******* ???  Need to fill LATTER part of array with zeroes ???   9/03 *********



// *************  Need to ReInstall this line,  dynamically memory for  xr,  xi  *******    8/2012		            
//			 Real_FFT_doubles(   (int)halfn,    xr,  xi    );       //  real_fft(   (int)halfn,   xrDB,  xiDB  );   
		   
	




			memset(   m_gAnaFreq,       0,    sizeOfVertialComponents  *sizeof(float)   );         // this is the analysis step 
			memset(   m_gAnaMagn,      0,    sizeOfVertialComponents  *sizeof(float)   );

																								
																						 

			for(   k=  1L;     k<  halfn;      k++    )		
	//		for(   k=    0;     k<  halfn;      k++    )	 // ***********   OK ?????????????  *******************************	
			{
				real   =   xr[ k ];      //   *   wt;      // **** ????  NEED wt  ????   *******************
				imag  =   xi[ k ];      //   *   wt;   


		//		magn =    2.0  *   sqrt(  real*real  +   imag*imag  );    //  2.0 ???******** WHY ?????                compute magnitude and phase 
				magn =               sqrt(  real*real  +   imag*imag  );    //  *****************************



				phase =   atan2(         imag,   real   );    
		//		phase =   smbAtan2(   imag,   real   );     //      ****************   BETTER ???  ************************


				double   lastPhase =    gLastPhase[ k ];
				
				gLastPhase[ k ] =   phase;    //  save the  calculated-phase  for next iteration


					      //  Looks like they use the   DEVIATION in the EXPECTED-Phase   to calculate the  TRUE-Frequency  that is present in the Bin.     12/11

				double   trueFreq  =     Calculate_True_Frequency(    phase,    lastPhase,    k,    freqPerBin,    expct,    osamp  );

		//		double   approxFreq =        (double)k     *  freqPerBin;      ** DEBUG only 


				m_gAnaMagn[ k ]  =    magn;       //  store magnitude and true frequency in analysis arrays
				m_gAnaFreq[  k ]  =    trueFreq;  				
			}




																								//    PROCESSING the PitchShift,  by moving values to other bins. 
		
			memset(  m_gSynMagn,    0,    sizeOfVertialComponents * sizeof(float)   );    
			memset(  m_gSynFreq,     0,    sizeOfVertialComponents * sizeof(float)   );



			for (  k = 0;    k  <=  fftFrameSize2;    k++  )       //   this does the actual pitch shifting 
			{ 

													//   No PitchShift here...   just copy back to original bins.

				if(   k   <=   fftFrameSize2   ) 
		//		if(   k   <     fftFrameSize2   )    // ********* WHICH is right ???   12/20/11   **************************
				{ 

					/****
					if(        m_gSynMagn[ k ]  !=   0.0                 // ***    TEMP    DEBUG (read below)   ***
						||    m_gSynFreq[  k ]  !=   0.0    )
					{   ASSERT( 0 );   }    //  never gets hit     12/27/2011
					***/

			//		m_gSynMagn[ k ]   +=    m_gAnaMagn[ k ];    //          Could use  EQUAL,  because of what I learned with above  Temp-DEBUG  Test   12/11
					m_gSynMagn[ k ]     =    m_gAnaMagn[ k ];   //  ...using  EQUAL until the above test shows ups a problem.   12/28/2011

					m_gSynFreq[  k ]      =    m_gAnaFreq[ k ];   
				} 				
			}

			



			float       prevPhase; 
			double    realLoc,    imagLoc;



			for(   long  rep= 0;     rep <  slowDownSpeed;     rep++   )    // *** the REPETION Loop to increase Output Samples for SlowSpeeds  ***
			{


				memset(  xr,    0,     fftFrameSize * sizeof(double)   );    
				memset(  xi,     0,    fftFrameSize * sizeof(double)   );


																					            //    SYNTHESIS 
				for(    k = 0;     k <=   fftFrameSize2;      k++   ) 
				{
								
					magn =    m_gSynMagn[ k ];      // get magnitude and true frequency from synthesis arrays 

					double   trueFreq  =    m_gSynFreq[  k ];

					double   expectedFreq =       (double)k        *  freqPerBin;    // ****************  TEMP  DEBUG  *****************  



								 //   Looks like the   DEVIATION in the Bin's Frequency  is used to calulate a corrected Phase-Difference between FFT frames.   12/11

					double   deltaPhase  =    Calculate_True_DeltaPhase(   trueFreq,    k,    freqPerBin,    expct,    osamp   );   //   'deltaPhase'  is in RADIANS



															
				//	gSumPhase[ k ]   +=    (float)deltaPhase;     //  accumulate  delta-phase  to get  bin's phase 
				//  phase =   gSumPhase[ k ];

					prevPhase  =     gSumPhase[ k ];  

					phase =     (double)(       (double)prevPhase   +   deltaPhase       );



					ASSERT(  slowDownSpeed == 2  );   //   Necessary for the test below  **********************


					if(          rep  ==   0   )						
					{
						gSumPhase[ k ]  =      (float)phase;
					}
					else if(   rep  ==   1   )
					{
				//		gSumPhase[ k ]  =      (float)phase;    //   Crappy sound,  like a helocopter.    4/1

						gSumPhase[ k ]  =      prevPhase;     // **** SEEMS much better,   WHY???   Look at the data.
					}
					else
					{	ASSERT( 0 );   }


					
				
			//		if(   k  >=   2  )    //  try to copy logic from above   
			//		{
						realLoc   =       magn   *   cos( phase );  
						imagLoc  =      magn   *   sin( phase );

						xr[  k  ]  =      (float)realLoc;  
						xi[  k  ]  =      (float)imagLoc;
			//		}

				}   //   for(    k = 0;     k <=   fftFrameSize2




																			    //  ****  do  'INVERSE TRANSFORM'   for  ReConstruction  ****
		    

				wt    =    2.0;             // Real and imaginary parts WOULD have reinforced(so simulate with 2x )


				wt   /=    (double)totSize;     // must scale down by 'n', before or after transform   [   ***** More ACCURACY if done BELOW  1/2012 ???? ******************






				if(    useAlternateReconstruct    )      //   A-alt.     This is the STANDARD way of setting up the data.
				{
					for(   i=  halfn +1;     i<  totSize;      i++    )			
					{													                             
						xr[ i ]  =   0.0;   
						xi[ i ]  =    0.0;   
					}  
				}
				else									//  This logic is from Book on Fast Filtering  
				{				
					for(   i=  1L;     i<  halfn;      i++    )			//   A.    NOTE:   data is in RIGHT half of array,  
					{													            //             and LEFT half was ZEROEDout( to stop wraparround )       
						   
				//	    xi[  totSize - i  ]  =     -wt   *   xi[ i ];    // sign change(-) cause this is now at UPPER end   ...NOT in Book[ pp209 ]
						xi[  totSize - i  ]  =      wt   *   xi[ i ];					

						xr[ i ]  =   0.0;   
						xi[ i ]  =    0.0;   
					}  
			
				}


			   xr[ 0 ] =  xi[ 0 ] =  xr[ halfn ] =  xi[ halfn ] =     0.0;     // Also zero these



				//   ***********  now done  ABOVE  when applying the 'weight'    ***** More ACCURACY if done down here  1/2012 ???? *********************								      
				//	   for( i=0;  i< totSize;  i++ )    
				//		 {  xi[i]=  xi[i] / (double)totSize; // must scale down by 'n', before or after transform
				//			xr[i]=  xr[i] / (double)totSize;   
				//		 }		 
					        
				 //   Not in Book,  is this done with the weight ???   9/03


				//	   for(   i= 0;    i <  totSize;    i++  )   
				//		   xi[ i ] =   -xi[ i ];						 // '-' FLIP SIGN of imaginary part
					   


									  //  ****  do  'INVERSE TRANSFORM'  with foward routine, data here is 0.28 to -0.32( in Frac form )  ****  

			// FFT_fix(           (int)totSize,    xr,  xi    );   



// *************  Need to ReInstall this line,  dynamically memory for  xr,  xi  *******    8/2012
//			   FFT_doubles(    (int)totSize,    xr,  xi    );  





			   	if(    useAlternateReconstruct    )   
				{

					for(  i= 0;  i< totSize;    i++  )    // ***********************   TEMP  for   A-alt    *********************
					{  
						
					//	window       =      -0.5    *   cos(  2.0 * M_PI   * (double)i  /  (double)fftFrameSize  )    + 0.5;


					//	xr[i] =    (  xr[i]  /  (double)totSize   )    *   window;   
						xr[i] =    (  xr[i]  /  (double)totSize   );              //   must scale down by 'n', before or after transform

						 xi[i] =       xi[i]   / ( double)totSize;     //   ****  WHY ???  I do not read this ag  ************   
					}		
				}

	

																					 //   write RESULTS back to sample  

			   for(    k= 0;      k <  fftFrameSize;     k++    )     
			   {  
				   /***
			   				//		gOutFIFO[  k  ]   =    gOutputAccum[  k  ];     ***OLD CODE

						long   trueOutputIndex  =    k    +    ( rep  *  stepSize ); 

						gOutFIFO[   trueOutputIndex  ]  =     gOutputAccum[  k  ];     //    gOutFIFO[]   will directly feed the  OUTPUT samples  ( see above )
					***/
			   		long   trueOutputIndex  =    k    +    ( rep  *  fftFrameSize ); 

					gOutFIFO[   trueOutputIndex   ]   =    xr[  k  ];
				}

			}     //  for(  rep=0      


		}   //  if(   gRoverLocal  >=  fftFrameSize     .... have read enough Input-Samples for FFT processing 

	}   //   for(   s = 0;    s < numSampsToProcess 



	
	if(   rightStereo   )				         //   save values for next entrance to this function,  or we will hear audio 'static'  from variable confusion 
		m_gRoverRight =     gRoverLocal;       //   from switching from left to right stereo during realtime play
	else
		m_gRoverLeft   =     gRoverLocal;

	return   true;
}






					/////////////////////////////////////////////

/****
bool	   FFTslowDown::Change_SlowDown_Algo(    short   newSlowDownAlgoCode,    CString&   retErrorMesg   )
{

		//    *** Do NOT CALL this directly,  use   Set_SlowDown_Optimizations_this_Speed()    1/12 ***



	long          sampsInOutputBuffer    =       Get_OutputBuffers_Sample_Count();
	ASSERT(   sampsInOutputBuffer  >  0  );  


	if(     ! Initialize(   m_playSpeed,     newSlowDownAlgoCode,    sampsInOutputBuffer,    retErrorMesg  )     )
		return  false;


	return  true;
}
****/