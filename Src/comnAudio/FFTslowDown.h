/////////////////////////////////////////////////////////////////////////////
//
//  FFTslowDown.h   -   
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////

#pragma once



class  ReSampler;


class   FFTtrForm;


////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////


class  FFTslowDown
{

public:
	FFTslowDown(void);
	virtual  ~FFTslowDown(void);




	bool			Initialize(   double  playSpeed,   short  slowDownAlgo,    long  outputBufferSampleCountNotSlowDown,     CString&  retErrorMesg   );


	void		    Initialize_SlowDown_Variables();   //  does NOT allocate




	bool		    Set_SlowDown_Optimizations_this_Speed(   CString&   retErrorMesg   );


	short		    Get_SlowDown_Alorythm_Code()     {  return  m_slowDownAlgo;   }             //   CALLED by  WavConvert::Load_Next_MemoryBlock_Frame(





	bool			Process_SpeedChange(   double   newSlowSpeed,    long  computerPerformanceFactor,     CString&   retErrorMesg   );




	void			Adjust_FFT_Parameters_for_Speed(   double  newSpeed,    short  newSlowDownAlgoCode  );


	double 		Get_Speeds_Volume_Tweak_Factor();






	long			Get_OutputBuffers_Sample_Count();     //   this should have been SET for the SONG in    WavConvert::Initialize_for_Streaming()


	double		Calc_Buffers_Multiplier(   short  slowDownAlgo,    double  playSpeed    );


	bool			Alloc_SlowDown_Buffers(    double  newPlaySpeed,   short  slowDownAlgo,    long  outputBufferSampleCountNotSlowDown,    CString&  retErrorMesg  );
	void			DeAllocate_SlowDown_Buffers();


	void			Erase_SlowDown_Buffers();

	void			Erase_OutputAccumulators();





	static		double	    Calculate_True_DeltaPhase(   double synthFreq,                           long k,    double  freqPerBin,   double  expct,   long osamp  ); 
																		                              
	static		double      Calculate_True_Frequency(     double phase,   double  lastPhase,    long k,   double  freqPerBin,   double  expct,   long osamp   );

	static		double		UnWrap_Phase(   double   phase   );


	static		void			Shift_Frequency_Arrays_for_ReConstruction(   float  pitchShift,    long  frameSize,    float  *anaylMagnitude,   float  *anaylFreq,      
														                                                                              float  *synthMagnitude,   float  *synthFreq,  short opCode   );




	virtual  	bool		    Shift_Pitch(                     bool  rightStereo,     float pitchShift,     long numSampsToProcess,      long fftFrameSize,     long osamp,   float sampleRate,
																							                       float *indata,  float *outdata,    CString&   retErrorMesg  );   


	virtual  	bool          Shift_Pitch_Horizontally(  bool  rightStereo,   float pitchShift,     long numSampsToProcess,    long fftFrameSize,     
											                                          long osamp,   float sampleRate,      float *indata,   float *outdata,      
																					 float  binsPitchShift,    long  decimationRatio,    long  outputRepetitionCount,   CString&   retErrorMesg  );   
												


	virtual  	bool			Shift_Pitch_Horizontally_OLDfft(  bool  rightStereo,  float pitchShift,   long numSampsToProcess,  //  ******  MAYBE Omit,  it was a failed experiment **** 
											                                   long fftFrameSize,    long osampDummy,   float sampleRate,      float *indata,   float *outdata,      CString&   retErrorMesg  );   






				bool		 Apply_SlowDown_wPitchShift(            BYTE**  retSamplesPtr,    long numInputSamples,    bool stereoFlag,    long  fftFrameSize,
										                                                                                             long osamp,     CString&   retErrorMesg   );

				bool		 Apply_SlowDown_wHorizontal_FFTs(   BYTE**  retSamplesPtr,    long numInputSamples,    bool stereoFlag,    long  fftFrameSize,
										                                                                                             long osamp,     CString&   retErrorMesg   );


		
				bool		 Apply_Phase_Filtering_wFFT(    BYTE**  retSamplesPtr,     long numSamples,       bool stereoFlag,     long  fftFrameSize,
										                                                                                          long osamp,     CString&   retErrorMesg  );

			    bool		 Change_Samples_Pitch(           BYTE**  retSamplesPtr,    long numSamples,        bool stereoFlag,    float  pitchShift,   long  fftFrameSize, 
										                                                                                          long osamp,     CString&   retErrorMesg  );   // **** WORK on IT  12/11   *****


				bool       FastSharpen_Column(    bool  rightStereo,   long  numSampsToProcess,   long fftFrameSize,   float sampleRate,   float *indataLeft,     float *outdataLeft,    CString&  retErrorMesg   );


				bool		 Fast_Filter_MemBlock(    bool  rightStereo,    long numSampsToProcess,     long fftFrameSize,       float sampleRate,      																																								
																															float *indataLeft,   float *outdataLeft,    CString&   retErrorMesg    );




	virtual  	bool		Do_Phase_Filtering(   long numSampsToProcess,     long fftFrameSize,     long osamp,     float sampleRate,      float *indataLeft,  float *indataRight,   																																								
																		                                                 float *outdataLeft,   float *outdataRight,     CString&   retErrorMesg  );   




	static  	void			smbFft(   float *fftBuffer,    long fftFrameSize,    long signDirection   );    // *****  Should ultimate replace these with  FFT_with_only_Real_Input() 
	static  	double		smbAtan2(   double x,    double y   );







private:
	short	     m_slowDownAlgo;   //   0:  VERTICAL  ( PitchShift 'upward' and then Resampling )       1:  HORIZONTAL  ( NO ReSampling,  uses the Horizontal Expand in FreqDomain algo )


	short     m_fftCodeType_horizontal;    //    0:  Use standard code( default)      1:  use disasterous EXPERIMENTAL code for Horz render [*** BAD EXPERIMENT ***] 

	bool		m_allowFixedPointCalcs;      //   false is DEFAULT.    Another  questionable EXPERIMENTAL Code.  Seems like FixedPoint is too NOISY, and not really that much faster.  1/2012




public:
	FFTtrForm    *m_fftTrForm;    //  DEFAULT:  this stays NULL.   1/2012


	ReSampler    m_reSamplerSlowDown;    //  Need a 2nd one,  for the SlowedDown Speeds    12/11

	float            *m_leftSlowedDownSamples,   *m_rightSlowedDownSamples;   



	long			m_computerPerformanceFactor;    //     0: Fast     1: Average     2: Slow    


	double	 m_playSpeed;   

	
	bool      m_areSlowDownBuffersInitialized;     

	long		m_outputSampleCountNotSlow;    //  BIG,  needs to be reassigned when new song is loaded



	long      m_fftFrameSizeSlowDown;             //   2048    ...almost always.


	long		m_overSampleRatioSlowDown;     //     8        ...almost always.    The bigger,  the greater OverSampling, usually more accuracy. 

//						  fftFrameSize  =    2048;       //   2048          2048[ Best? ]     1024[ OK ]       512[ worse ]     128[ not bad  ]   

//						  osamp           =       8;        //       4  -   32      8[ good ]      4[ not bad]            2[bad]  



	double   m_lessThanOneDivisor;


	long     m_gRoverLeft,  m_gRoverRight;



	float		*m_gFFTworksp;


	float     *m_realsMasters,         *m_imagMasters;   //  So can implement Book by Timithothy Masters   1/12

	long		*m_realsFixMasters,    *m_imagFixMasters;     //   for FIXED POINT Calcs  from Master algos,




	float		*m_auxInputSamples;


	float		*m_gInFIFOLeft ;			//    Thesse are the SlowDOWN  Buffers for   Shift_Pitch()    12/2011
	float		*m_gOutFIFOLeft ;
	float		*m_gOutputAccumLeft ;

	float		*m_gInFIFORight ;
	float		*m_gOutFIFORight ;
	float		*m_gOutputAccumRight ;

	float		*m_gLastPhaseLeft ;
	float		*m_gLastPhaseRight ;

	float		*m_gSumPhaseLeft ;
	float		*m_gSumPhaseRight ;


	float		*m_gAnaFreq;
	float		*m_gAnaMagn;

	float		*m_gSynFreq;
	float		*m_gSynMagn;



	enum   slowDownPreferences   {   VERTICALcALC,      HORIZONTALcALC,      BESTofBOTHcALC    };
};
