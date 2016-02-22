//  sndFilters2.cpp :    [   has   FFTtrForm ]
//
//  Copyright (C) 2007   Creative Detectors (of San Francisco, California).  All Rights Reserved.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"


#include <math.h>



///////////////////////////////

#include   "..\comnFacade\UniEditorAppsGlobals.h"


#include  "..\comnFacade\VoxAppsGlobals.h"



//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"


#include  "..\ComnGrafix\CommonGrafix.h"    
#include  "..\comnFoundation\myMath.h"



#include  "..\comnGrafix\OffMap.h"   //   #include "gnOffMap.h" 

#include  "..\comnGrafix\TransformMap.h"
///////////////////////////////



#include  "sndSample.h"





#include  "sndFilters.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////





void        Begin_ProgressBar_Position_GLB(  char  *text   );   //   in  SPitchListWindow.cpp  
void        Set_ProgressBar_Position_GLB(   long  posInPercent   );
void        End_ProgressBar_Position_GLB();


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////




						/////////////////////////////////////


	
FFTfilter::FFTfilter(  short  opCode   )  :    m_opCode( opCode )
{  

	_Samp =   NULL;


	m_kernalFFT =  NULL;

	m_showProgressBar =   true;



	m_freqCutoff =  0;    // 0  

	//	freqCutoff =   154;    //   the LOW value   //   154 is pretty good,  use for my low setting,      311  for med or high ???? 
	//  hiFreqVal  =    311;   //  the HIGH value

	m_freqSuperHighCutoff  =    19912;   //  Disabled at this,  nothing is reall above




	m_magPercentThreshold  =   33;     //    NOW  Only used for  CENTERpanEMPHASIShard     *****  ADJUST *****   
	/**
	33[ good ]       12[ some bass leaking ]    25[ still good]      50[ ehh ]   66[ lousey ]   

	If threshold is too high, some  Center-Harms rows are falsely identified as NONcenter-harms,  and are not only falsely subtracted from the MiddleOnly sample
	BUT... are then placed into the VoiceRemoval sample  ...letting the user hear the solo in voice removal.

	Consequently the threshold must be carefull selected or both separations suffer.  5/07
	**/


	m_bandwidthSpreadFactor =    5;  //     5[  unaltered bandwidth ]     10[  2x bandwidth  ]    15[  3x bandwidth ]      20[  4x bandwidth  ]


	m_eventList =  NULL;


	m_maskForScalepitchHarms =  false;   //  or for the basic 8 harms of a 


	 m_fftSize =   512;    //      512,       1024   256      ;  careful with array overflow in FFT workspace  




	m_targetPan                          =     50;    //   50 middle
	m_panTolerance                    =     30;    //   30:    +15 to -15   on either side of   m_targetPan 


	m_phaseDifferenceThreshold  =     179;      //  179...   Disables phaseChange as criteria


	m_pitchelCutoffScore        =    3000.0;

	m_pitchelCandidateCount  =   6;    //    8[ too much, but still leaks ]   6[ not bad]     5[ some leaking ]   4[ some leaking ]     


	m_xOffsetKernal =  0;
}  



												/////////////////////////////////////

FFTfilter::~FFTfilter()
{

	if(   m_kernalFFT  !=  NULL   )
		delete  m_kernalFFT;
}


												/////////////////////////////////////


bool	 FFTfilter::Initialize_FFT_Engine(  CString&  retErrorMesg    )
{


	short  kernaFFTwidth  =   7;   //    Have a small with so we could later do a  'PIPELINE processing'  to do MEDIAN smoothing to the 


	long   sampleRate      =     DEFsAMPLERATE;     // 44100L  

	long  horzScaleDown  =    m_fftSize;  // **** OK???


	if(    m_kernalFFT  !=  NULL   )
		delete  m_kernalFFT;





	m_kernalFFT  =       new     FFTtrForm(     kernaFFTwidth, 
																 m_fftSize,    
																 sampleRate,  
																 horzScaleDown    ); 
	if(   m_kernalFFT  ==  NULL  )
	{		
		retErrorMesg  =  "FFTfilter::Initialize  failed,  m_kernalFFT is null" ;
		return  false;
	}



//	m_kernalFFT->m_useFloatingPoint  =    true;   ...is now the default
	m_kernalFFT->m_magPercentThreshold       =     m_magPercentThreshold;    // ****   ADD other memberVar initializations
	m_kernalFFT->m_eventList                         =     m_eventList;
	m_kernalFFT->m_maskForScalepitchHarms  =    m_maskForScalepitchHarms;

	m_kernalFFT->m_freqCutoff                        =    m_freqCutoff;
	m_kernalFFT->m_freqSuperHighCutoff          =   m_freqSuperHighCutoff;

	m_kernalFFT->m_phaseDifferenceThreshold  =     m_phaseDifferenceThreshold;
	m_kernalFFT->m_targetPan                          =     m_targetPan;      //   50 middle
	m_kernalFFT->m_panTolerance                    =     m_panTolerance;    //   30:    +15 to -15   on either side of   m_targetPan 

	m_kernalFFT->m_pitchelCutoffScore             =   	m_pitchelCutoffScore;

	m_kernalFFT->m_pitchelCandidateCount      =    m_pitchelCandidateCount;


				

									//  make a lookup table that gives   ALL Intersecting FFT-rows   for any give Harmonic's MidiNumber 

	if(         OpCode_Needs_FFTmidi_LookupTable(  m_opCode  )  
		&&    m_kernalFFT  !=  NULL   )
	{

		if(    ! m_kernalFFT->Make_Harmonics_Lookup_Table(   m_bandwidthSpreadFactor,   true,    retErrorMesg  )    )
			return  false;	
	}


	return  true;
}



												/////////////////////////////////////


bool	 FFTfilter::OpCode_Needs_FFTmidi_LookupTable(   short  opCode   )
{

	if(         opCode ==     FFTtrForm::HARMSmASK  
	      ||   opCode ==     FFTtrForm::HARMSiSOLATE  
	      ||   opCode ==     FFTtrForm::HARMSmASKmono  
	      ||   opCode ==     FFTtrForm::HARMSiSOLATEmono   
		  
	      ||   opCode ==     FFTtrForm::PITCHELeMPHASIS  
	      ||   opCode ==     FFTtrForm::PITCHELsUBTACT  
	      ||   opCode ==     FFTtrForm::PITCHELsTEREOhYBRID

	      ||   opCode ==     FFTtrForm::WRITEtoLogDFT	

	      ||   opCode ==     FFTtrForm::CENTERpanEMPHASIShARMS	
	      ||   opCode ==     FFTtrForm::CENTERpanSUBTRACThARMS	

	      ||   opCode ==     FFTtrForm::CENTERpanEMPHASIShARMSstereo 	
	      ||   opCode ==     FFTtrForm::CENTERpanSUBTRACThARMSstereo 		


	      ||   opCode ==     FFTtrForm::CENTERpanEMPHASIShARMSpHASEstereo 	
	      ||   opCode ==     FFTtrForm::CENTERpanSUBTRACThARMSpHASEstereo 		
     ) 
		return  true;
	else
		return  false;
}


												/////////////////////////////////////


bool	 FFTfilter::OpCode_Needs_Mono_Source_SndSample(   short  opCode   )
{

	if(         opCode ==     FFTtrForm::VERTICALsHARPEN  

	//      ||   opCode ==     FFTtrForm::PITCHELeMPHASIS    NO,  now does stereo   6/07
	//	  ||   opCode ==     FFTtrForm::PITCHELsUBTACT  

	      ||   opCode ==     FFTtrForm::PITCHELsTEREOhYBRID   //   start out as Mono,  but need to become a Stereo for its result during Erode()  ???
	  )
		return  true;
	else
		return  false;
}




													/////////////////////////////////////


void		FFTfilter::Set_ImBalanced_Mag_Percent_Threshold(   short  magPercentThreshold   )     
{   
																		 // NOW  only used for  CENTERpanEMPHASIShard

	m_magPercentThreshold =   magPercentThreshold;   
}


													/////////////////////////////////////


void	 FFTfilter::Set_BandwidthSpread_Factor(   short  spreadFactor    )
{

	if(   spreadFactor  <=  0   )    //    0:  no division by zero
	{
		ASSERT( 0 );
		m_bandwidthSpreadFactor  =   5;
	}
	else 
		m_bandwidthSpreadFactor  =   spreadFactor;
}


													/////////////////////////////////////


void	  FFTfilter::Set_FFTs_Size(  short  fftSizeCode )    
{

	if(           fftSizeCode  ==   FFTtrForm::SIZE256   ) 
		m_fftSize =    256;  
	else if(    fftSizeCode  ==   FFTtrForm::SIZE512   ) 
		m_fftSize =    512;  
	else if(    fftSizeCode  ==   FFTtrForm::SIZE1024   ) 
		m_fftSize =   1024;  
	else if(    fftSizeCode  ==   FFTtrForm::SIZE2048   )   
		m_fftSize =   2048;  
	else if(    fftSizeCode  ==   FFTtrForm::SIZE4096   )   
		m_fftSize =   4096;  
	else if(    fftSizeCode  ==   FFTtrForm::SIZE8192   )   
		m_fftSize =   8192;  
	else 
	{	//   retErrorMesg.Format(  "FFTfilter::Set_FFTs_Size FAILED,  unknown  fftSizeCode[ %d ]." ,   fftSizeCode  );
		//  return  false;
		ASSERT( 0 );
		m_fftSize =    512;  
	}
}


													/////////////////////////////////////


ListMemry< LongIndexed >*	FFTfilter::Create_SingleElement_EventList(   long  startOffset,  long  endOffset,   short  midiVal,     CString&  retErrorMesg   )
{


					//    startOffset,  endOffset  are in  ABSOLUTE terms  with in the time frame of the ENTIRE sample 


	ListMemry< LongIndexed >*    nuList =      new   ListMemry< LongIndexed >();
	if(  nuList ==   NULL  )
	{
		retErrorMesg  =  "FFTfilter::Create_SingleElement_EventList  FAILED,  could not alloc nuList." ;
		return  NULL;
	}
	
	nuList->Set_Dynamic_Flag(  true  );



	LongIndexed   *nuLongIdx  =      new    LongIndexed();
	if(  nuLongIdx  ==  NULL   )
	{
		retErrorMesg	 =   "FFTfilter::Create_SingleElement_EventList  FAILED,  nuLongIdx is NULL."  ;
		delete   nuList;
		return  false;
	}

	nuLongIdx->index    =    midiVal;
	nuLongIdx->value0  =    0;                                     //   must put offsets in RELATIVE terms for the FFT  when doing a single note-play
	nuLongIdx->value1  =    endOffset  -  startOffset;    

	nuList->Add_Tail(  *nuLongIdx  );   


	return   nuList;
}


													/////////////////////////////////////


bool	FFTfilter::Filter(   CString&  retErrorMesg   )
{

						   //  can do stereo or mono  ...input or output     5/07

 	retErrorMesg.Empty();


	if(    _Samp  ==   NULL    )    
	{
		ASSERT( 0 );
		retErrorMesg  =  "FFTfilter::Filter failed,  sample is NULL." ;
		return  false;
	}

  	long   numByts    =       _Samp->Get_Length();
	if(   numByts == 0L )   
	{
		retErrorMesg  =  "FFTfilter::Filter failed,  length is 0." ;
		return  false;
	}
  

     SndSample  *clonedSample    =      _Samp->Clone();  //  Look at   FFTtrForm::Erode_Stereo_Sample() to see why we need a cloned sample.   	 
	 if(   clonedSample  ==  NULL   )   
	 {
		retErrorMesg  =  "FFTfilter::Filter failed,  Clone is NULL." ;
		return  false;
	 }


																				 //  Some ops need to start as a monoSample, BUT for its result it needs to become stereo. [  PITCHELsTEREOhYBRID ]   6/07  

	 if(   !  FFTtrForm::OpCode_Wants_Mono_Output(  m_opCode  )     )   // ***********************   TRICKY  ***************************
	 {
		if(    ! _Samp->Make_Stereo(  retErrorMesg  )    )      //  if it is alread a Stereo-SndSample( from the cloning ) nothing is done.
			return   false; 
	 }



	if(    ! Erode(   m_opCode,    &clonedSample,    retErrorMesg  )     )   //  will write the RESULTS  to   _Samp...  and deletes Clone   5/07
	{
		if(   clonedSample  !=  NULL   )
			delete   clonedSample; 

		return  false;
	}



	if(    clonedSample  !=  NULL   )   // Safety Net:   Sometimes  functions like  Erode()  delete the Clone when they are done... just playing it safe
	     delete   clonedSample;         
 
	return  true;
}



													/////////////////////////////////////


bool	FFTfilter::Erode(   short  opCode,    SndSample  **srcSamplePtr,     CString&  retErrorMesg   )
{


			//   srcSample is a pointer to a pointer, so we can delete it and set it to nNULL at the end [  delete the clone when not needed  ]
 

	short  kernaFFTwidth  =   7;   //    Have a small with so we could later do a  'PIPELINE processing'  to do MEDIAN smoothing to the 



 	retErrorMesg.Empty();

	if(    srcSamplePtr  ==   NULL   )    
	{
		ASSERT( 0 );
		retErrorMesg  =  "FFTfilter::Erode failed,  srcSamplePtr is NULL." ;
		return  false;
	}

	if(   m_kernalFFT  ==   NULL   )
	{
		ASSERT( 0 );
		retErrorMesg  =  "FFTfilter::ForwardFFT_Writes_logDFT failed,   m_kernalFFT  is NULL." ;
		return  false;
	}



	if(    m_eventList ==  NULL     &&					// *****  INSTALL,, lso need for the LookUp Table to have been built. 
		    (  opCode ==     FFTtrForm::HARMSmASK  
	      ||   opCode ==     FFTtrForm::HARMSiSOLATE  
	      ||   opCode ==     FFTtrForm::HARMSmASKmono  
	      ||   opCode ==     FFTtrForm::HARMSiSOLATEmono  )    )	// ********  Simple  PASS-THROUGH  for these cases   ******************  
	{
		ASSERT( 0 );
		retErrorMesg  =  "FFTfilter::Erode failed,  harmonic masing opCodes need to have an Event List." ;
		return  false;
	}




	SndSample  *srcSample =      *srcSamplePtr; 

	SndSample  *dstSample  =    _Samp;   //  SWITCHING samples,  srcSample is the temp CLONE,  but results will be written to  _Samp 

	if(       dstSample  ==   NULL  
		||    srcSample  ==   NULL     )    
	{
		ASSERT( 0 );
		retErrorMesg  =  "FFTfilter::Erode failed,  dstSample  or  srcSample  is NULL." ;
		return  false;
	}



	short   widthInFFTchunks  =      (short)(    srcSample->m_totalSamps   /   m_fftSize    );   //  Now this can VARY with different FFT size  [  m_fftSize  ]   


	long  progPosPercent =   10;

	if(   m_showProgressBar   )
	{
		Begin_ProgressBar_Position_GLB(  NULL  );  
		Set_ProgressBar_Position_GLB(  progPosPercent );
	}


	/****    Moved to  Initialize ()

	long  horzScaleDown  =    m_fftSize;  // **** OK???

	m_kernalFFT  =       new     FFTtrForm(        kernaFFTwidth,   //		(short)(  numChannelSamples  / m_chunkSize  ),    
																				 m_fftSize,    
																				 srcSample->_sampleRate,  
																				 horzScaleDown,   
																				 srcSample->Get_Sample_Count()      ); 
	if(   m_kernalFFT  ==  NULL  )
	{
		if(   m_showProgressBar   )    
			End_ProgressBar_Position_GLB();
		
		retErrorMesg  =  "FFTfilter::Erode  failed,  m_kernalFFT is null" ;
		return  false;
	}

//	m_kernalFFT->m_useFloatingPoint  =    true;   ...is now the default
	m_kernalFFT->m_magPercentThreshold       =     m_magPercentThreshold;    // ****   ADD other memberVar initializations
	m_kernalFFT->m_eventList                         =     m_eventList;
	m_kernalFFT->m_maskForScalepitchHarms  =    m_maskForScalepitchHarms;

	m_kernalFFT->m_freqCutoff                        =    m_freqCutoff;
	m_kernalFFT->m_freqSuperHighCutoff          =   m_freqSuperHighCutoff;

	m_kernalFFT->m_phaseDifferenceThreshold  =     m_phaseDifferenceThreshold;
	m_kernalFFT->m_targetPan                          =     m_targetPan;      //   50 middle
	m_kernalFFT->m_panTolerance                    =     m_panTolerance;    //   30:    +15 to -15   on either side of   m_targetPan 

	m_kernalFFT->m_pitchelCutoffScore             =   	m_pitchelCutoffScore;


									//  make a lookup table that gives   ALL Intersecting FFT-rows   for any give Harmonic's MidiNumber 

	if(     OpCode_Needs_FFTmidi_LookupTable(  opCode  )    )
	{

		if(    ! m_kernalFFT->Make_Harmonics_Lookup_Table(   m_bandwidthSpreadFactor,   true,    retErrorMesg  )    )
		{  
			if(   m_showProgressBar   )    
				End_ProgressBar_Position_GLB();

			return  false;
		}
	}
************/


	progPosPercent +=  10;

	if(   m_showProgressBar   )   
		Set_ProgressBar_Position_GLB(  progPosPercent );

	long     step =   widthInFFTchunks /  ( 100 -  progPosPercent ),    pgVal;   //  progress bar




    for(    short  x= 0;     x<  widthInFFTchunks;     x++    )
    {  

		short  xOffsetted =  x;

		if(    m_xOffsetKernal  !=  0   )
			xOffsetted  =   x  +   m_xOffsetKernal;  



		if(    ! m_kernalFFT->Erode_Stereo_Sample(   opCode,   x,   xOffsetted,    srcSample,    _Samp,   	 retErrorMesg  )     )     //   Does a  Forward-FFT  and Reconstruction  in one step 
		{  	
			if(   m_showProgressBar   )    
				End_ProgressBar_Position_GLB();

			return  false;
		}



		if(    step  >  0         //  divide by zero for  single note syntheses
			&&(   ( x %  step )  ==  0   )   )
		{
			pgVal  =   x /  step;  
			if(  m_showProgressBar  )   
				Set_ProgressBar_Position_GLB(  pgVal  );
		}		
	}


	if(  m_showProgressBar  )    
		End_ProgressBar_Position_GLB();     //  stop now but will start again



	

	if(    m_kernalFFT->OpCode_Wants_Mono_Output( opCode )     )   //  some opCodes ask that we return a MONO  SndSample
	{
		if(   !  _Samp->Make_Mono( retErrorMesg  )    )   
			return  false;
	}

	

	if(   *srcSamplePtr  !=   NULL   )
	{
		delete  *srcSamplePtr;
		*srcSamplePtr =  NULL;   //  sets the pointer value in the CALLING function to NULL so no confusion   
	}

	return  true;
}




													/////////////////////////////////////


bool	FFTfilter::ForwardFFT_Writes_logDFT(   TransformMap  *logDFTtransMap,     long   detectZonesStartOffset,    long   detectZonesEndOffset,     
																			  short  startMidiRow,  short  endMidiRow,    double  brightenFactor,    short   stereoChannelCode,     CString&  retErrorMesg   )
{


	bool   allowPartialIntersectionsForFFT  =   true;    //     true:  absolutely !!!



	short  kernaFFTwidth  =   7;   //    Have a small with so we could later do a  'PIPELINE processing'  to do MEDIAN smoothing to the 



 	retErrorMesg.Empty();

	if(    _Samp  ==   NULL     )    
	{
		ASSERT( 0 );
		retErrorMesg  =  "FFTfilter::ForwardFFT_Writes_logDFT failed,   srcSample  is NULL." ;
		return  false;
	}


	if(   m_kernalFFT  ==  NULL   )
	{
		ASSERT( 0 );
		retErrorMesg  =  "FFTfilter::ForwardFFT_Writes_logDFT failed,   m_kernalFFT  is NULL." ;
		return  false;
	}



	short   widthInFFTchunks  =      (short)(    _Samp->m_totalSamps   /   m_fftSize    );   //  Now this can VARY with different FFT size  [  m_fftSize  ]   


	long  progPosPercent =   10;

	if(   m_showProgressBar   )
	{
		Begin_ProgressBar_Position_GLB(  NULL  );  
		Set_ProgressBar_Position_GLB(  progPosPercent );
	}



	/***
	progPosPercent +=  10;

	if(   m_showProgressBar   )   
		Set_ProgressBar_Position_GLB(  progPosPercent );
	***/
	long     step =   widthInFFTchunks /  ( 100 -  progPosPercent ),    pgVal;   //  progress bar


 

    for(    short  xFFTchunk= 0;      xFFTchunk<  widthInFFTchunks;       xFFTchunk++    )    //   xFFTchunk  is not  always the Map's   512-chunk
    {  


		if(    ! m_kernalFFT->Write_logDFT_Column(    xFFTchunk,     _Samp,    logDFTtransMap,	    startMidiRow,   endMidiRow,   brightenFactor,  stereoChannelCode,  retErrorMesg  )    )
		{ 		

			if(   m_showProgressBar   )    
				End_ProgressBar_Position_GLB();
			return  false;
		}


		if(     step  >  0         //  divide by zero for  single note syntheses
			&&(     ( xFFTchunk %  step )  ==  0   )     )
		{  pgVal  =   xFFTchunk /  step;  
			if(  m_showProgressBar  )   
				Set_ProgressBar_Position_GLB(  pgVal  );
		}		
	}



	if(  m_showProgressBar  )    
		End_ProgressBar_Position_GLB();     //  stop now but will start again	

	return  true;
}




