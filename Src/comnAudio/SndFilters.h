/////////////////////////////////////////////////////////////////////////////
//
//  sndFilters.h   -   FILTERS for my Sound-Samples
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined( _SNDFILTERS_H_  )

#define _SNDFILTERS_H_

/////////////////////////////////////////////////////////////////////


//  #include  "FastFourier.h"    

class   TransformMap;

class   FFTtrForm;



/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

class   SndFilter             
{ 		            			                 
																			// could use  'VISITOR'  pattern ?               		  
public:        
	SndFilter(   SndSample  *samp   );
	virtual  ~SndFilter();
   	  

	virtual		  bool		   Filter(   CString&  retErrorMesg   )  =0;         // could pass  'SndSample*'  as PARM ???




																//    'Sample-MEASUREMENT'   functions

	virtual		  long		  Get_Average_Amplitude();

	virtual		  void		  Get_Amplitude_Populations(   long   population[],     long  bucketCount   );

	virtual		  long		  Get_Maximum_AverageAmplitude_of_ChunkGroups(   long  adjChunkSize   );


	virtual		  long		  Get_Top_Value(    long  discardPercent,     long  population[],     long  bucketCount,  	
																																long&  retAbsoluteTop    );



public:  
      SndSample    *_Samp;    // in stereo scenerios,  this is the  LEFT stereo.  In VoiceRemoval, Emphasis, this is the MONO output.  5/07

 //   SndSample    *m_rightStereoSample;   //    this is the  LEFT stereo. 


	  bool   m_showProgressBar;   //  not yet installed to all subclasses   5/2007

};





		////////////////////////////////////////////////
		////////////////////////////////////////////////


class   HiBoostLoSndFilter  :  public  SndFilter             
  { 		            			                              		  
    public:  
	  virtual		  bool		   Filter(       CString&  retErrorMesg   );          
      HiBoostLoSndFilter(  SndSample *samp  )  :  SndFilter(samp)   {  }  
  };

class   HiBoostMedSndFilter  :  public  SndFilter             
  { 		            			                              		  
    public:  
	  virtual		  bool		   Filter(     CString&  retErrorMesg   );          
      HiBoostMedSndFilter(  SndSample *samp  )  :  SndFilter(samp)   {  }  
  };

class   HiBoostHiSndFilter  :  public  SndFilter             
  { 		            			                              		  
    public:  
 	  virtual		  bool		   Filter(      CString&  retErrorMesg   );          
      HiBoostHiSndFilter(  SndSample *samp  )  :  SndFilter(samp)   {  }  
  };



   ////////////////////////////////////////////

class   LowLoSndFilter  :  public  SndFilter             
  { 		            			                              		  
    public:  
	  virtual		  bool		   Filter(     CString&  retErrorMesg   );          
      LowLoSndFilter(  SndSample *samp  )  :  SndFilter(samp)   {  }  
  };


class   LowMedSndFilter  :  public  SndFilter             
  { 		            			                              		  
    public:  
 	  virtual		  bool		   Filter(      CString&  retErrorMesg   );          
      LowMedSndFilter(  SndSample *samp  )  :  SndFilter(samp)   {  }  
  };


class   LowHiSndFilter  :  public  SndFilter             
  { 		            			                              		  
    public:  
 	  virtual		  bool		   Filter(   CString&  retErrorMesg   );          
      LowHiSndFilter(  SndSample *samp  )  :  SndFilter(samp)   {  }  
  };


		////////////////////////////////////////////////

class   MidHighSndFilter  :  public  SndFilter             
  { 		            			                              		  
    public:  
	  virtual		  bool		   Filter(   CString&  retErrorMesg   );          

      MidHighSndFilter(  SndSample *samp  )  :  SndFilter(samp)   {  }  
  };



		////////////////////////////////////////////////


#define  kMidFlatSndFilter  4

class   MidFlatSndFilter  :  public  SndFilter             
  { 		            			                              		  
    public:  
 	  virtual		  bool		   Filter(     CString&  retErrorMesg   );          

      MidFlatSndFilter(  SndSample *samp  )  :  SndFilter(samp)   {  }  
  };


		////////////////////////////////////////////////

class   VolumeFilter  :  public  SndFilter             
  { 		            			                              		  
    public:  
	  virtual		  bool		   Filter(     CString&  retErrorMesg   );          

      VolumeFilter(  SndSample *samp,   short  numer, short  denom  )  
                               : SndFilter(samp),  _Numer(numer),  _Denom(denom)  { } 
      
    public:  
      short  _Numer,  _Denom;
  };





		////////////////////////////////////////////////


#define  kAutoVolumeAdj   5


class   VolumeAutoAdjFilter  :  public  SndFilter             
{ 		 
	
public:  
	VolumeAutoAdjFilter(   SndSample *samp,   short  level   )     :  SndFilter( samp )  //  ,   _Level( level )   
	{  m_retScalePercentage =   -1;  } 

	virtual  ~VolumeAutoAdjFilter()   {   }     


	  virtual		  bool		   Filter(    CString&  retErrorMesg   );          
	



public:  
 //     short  _Level;       // what average level to shoot for

	  
	  long    m_retScalePercentage;     //  what was the  CHANGE in VOLUME of the signal  ...the result
};
		





		
    ////////////////////////////////////////////////
    ////////////////////////////////////////////////

class   FIRsndFilter  :  public  SndFilter             
  { 		            			//  'FINITE'  response( "C alog for DSP" )              		  
    public:  
	  virtual		  bool		   Filter(     CString&  retErrorMesg   );          

      FIRsndFilter(  SndSample *samp,   long  numCoeffs,   double  *floatCoeffs  ) 
                :  SndFilter(samp),  _numCoeffs(numCoeffs),  _floatCoeffs(floatCoeffs)   {  } 
    
    public:  
       long     _numCoeffs;   
       double  *_floatCoeffs;  
  };




#define  kIIRsndFilterLow  1
#define  kIIRsndFilterMid   2
#define  kIIRsndFilterHi   3



class   IIRsndFilter  :  public  SndFilter             
  { 		                   //  'INFINITE'  response( "C alog for DSP" )                         		  
    public:  
	  virtual		  bool		   Filter(   CString&  retErrorMesg   );          

      IIRsndFilter(  SndSample *samp,   long  filtSects,   double  *floatCoeffs  ) 
                :  SndFilter(samp),  _filtSects(filtSects),  _floatCoeffs(floatCoeffs)   {  } 
      
    public:  
       long     _filtSects;       //  (_filtSects  * 4) +1 =  'num' coeffs
       double  *_floatCoeffs;  
  };




		///////////////////////////////


#define   kIIRsecondSndFilter  0


class   IIRsecondSndFilter  :  public  SndFilter             
{ 		                
																	//  'INFINITE'  response( 2nd order ) [ article ]      
	

	//  I  like :   	edgeFreq0 ( topFrequencyLimit)            =     1800  	...signal preprocess for  logDFT
	//                    edgeFreq1 (  bottomFrequencyLimit  )   =       329 				


public:  
      IIRsecondSndFilter(     SndSample *samp,      short  edgeFreq0,     short  edgeFreq1  /*  ,    short  topVal  */  )  
							  :  SndFilter(samp),    _EdgeFreq0( edgeFreq0 ),    _EdgeFreq1( edgeFreq1 )  //   ,    m_topVal( topVal )  
	  {	   
		  m_retScalePercentage =   -1;
	  }
	  

	  virtual		 bool		  Filter(    CString&  retErrorMesg   );     

					bool		  Filter_Fixed_Scaling(    long  inputScalePercent,     double&  retOverbriteRatio,	   CString&  retErrorMesg   );     //   1/10   
	  


	  virtual		short        Get_CenterFrequency();

	  virtual		double      Get_Q_Bandwidth();

      

public: 
      short     _EdgeFreq0,   _EdgeFreq1;  
	  short    m_topVal;

	  long    m_retScalePercentage;     //  what was the  CHANGE in VOLUME of the signal  ...the result
};







class   FIRloPassCalcCoefSndFilter  :  public  SndFilter             
  { 		                  //  'INFINITE'  response( 2nd order ) [ article ]                         		  
    public:  
	  virtual		  bool		   Filter(    CString&  retErrorMesg   );          

      FIRloPassCalcCoefSndFilter(  SndSample *samp,   short edgeFreq0,  
                         short edgeFreq1,  double  atten  )    :  SndFilter(samp),  
                     _EdgeFreq0(edgeFreq0),  _EdgeFreq1(edgeFreq1),  _atten(atten)  { } 
      
    public: 
      short   _EdgeFreq0,   _EdgeFreq1; 
      double  _atten;   
  };



/////////////////////////////////////////////////////////////////////

/******************
class   FIRsndFilter  :  public  SndFilter             
  { 		            			//  'FINITE'  response( "C alog for DSP" )              		  
    public:  
      virtual   void   Filter();            

      FIRsndFilter(  SndSample *samp,   long  numCoeffs,   double  *floatCoeffs  ) 
                :  SndFilter(samp),  _numCoeffs(numCoeffs),  _floatCoeffs(floatCoeffs)   {  } 
    
    public:  
       long     _numCoeffs;   
       double  *_floatCoeffs;  
  };
*******************/


		////////////////////////////////////////////////
		////////////////////////////////////////////////

class   RectifyAndSquareFilter    :  public  SndFilter             
{ 		    
	
public:  
      RectifyAndSquareFilter(  SndSample *samp  )     :  SndFilter(samp)   
	  {  }  

	  virtual		  bool		   Filter(     CString&  retErrorMesg   );          
   
};




		////////////////////////////////////////////////
		////////////   FOURIER  /////////
		////////////////////////////////////////////////


class   FFTfilter     //   :  public  SndFilter             
{ 		    
	
public:  
      FFTfilter(   short  opCode   );  
      ~FFTfilter();          


	  bool			  Initialize_FFT_Engine(  CString&  retErrorMesg    );

	  void            Attach_SndSample(   SndSample  *sndSample   )     {   _Samp =    sndSample;   }


	  void            Set_xOffset_Kernal(  long offset  )        {   m_xOffsetKernal  =  offset;   } 



	  virtual		    bool				Filter(   CString&  retErrorMesg   );   



	  virtual		    bool		        ForwardFFT_Writes_logDFT(   TransformMap  *logDFTtransMap,     long   detectZonesStartOffset,    long   detectZonesEndOffset,     
																			           short  startMidiRow,  short  endMidiRow,     double  brightenFactor,     short   stereoChannelCode,    CString&  retErrorMesg   );



	  static		    bool				OpCode_Needs_FFTmidi_LookupTable(   short  opCode   );

	  static		    bool		        OpCode_Needs_Mono_Source_SndSample(   short  opCode   );



						void				Set_FFTs_Size(    short  fftSizeCode   );

					    void				Set_Cutoff_Frequency(  short  freq   )      {   m_freqCutoff  =    freq;  }

					    void				Set_BandwidthSpread_Factor(   short  spreadFactor   );

						void		        Set_ImBalanced_Mag_Percent_Threshold(   short  magPercentThreshold   );    // ***OMIT **** only used for  CENTERpanEMPHASIShard

						void		        Set_Scalepitch_Harmonic_Masking(   bool  doScalepitchHarms   )     {   m_maskForScalepitchHarms =   doScalepitchHarms;   }



						ListMemry< LongIndexed >*	 Create_SingleElement_EventList(   long  startOffset,  long  endOffset,   short  midiVal,     CString&  retErrorMesg   );




protected:
	  virtual		    bool				Erode(   short  opCode,    SndSample  **srcSamplePtr,     CString&  retErrorMesg   );




public:  
	short             m_opCode;


    SndSample    *_Samp;    // in stereo scenerios,  this is the  LEFT stereo.  In VoiceRemoval, Emphasis, this is the MONO output.  5/07


	FFTtrForm	  *m_kernalFFT;  

	long       m_xOffsetKernal;   //  for doing a whold WAV,  one FFTchunk at a time and translating the events xOffset


	bool   m_showProgressBar;   //  not yet installed to all subclasses   5/07

	short   m_magPercentThreshold;     // NOW  only used for  CENTERpanEMPHASIShard


	short   m_freqCutoff;     //  only for  low or high cutoff,  dependant on the opCode
	short   m_freqSuperHighCutoff;   // strip out everytning at very top


	short   m_bandwidthSpreadFactor;     // used when masking Harmonics agains the FFT freq-bins 


	bool    m_maskForScalepitchHarms;


	ListMemry< LongIndexed >  *m_eventList;
   
	short    m_fftSize;   //  512,   careful with array overflow in FFT workspace  


	short    m_phaseDifferenceThreshold;
	short    m_targetPan;
	short    m_panTolerance;   



	double   m_pitchelCutoffScore;    //  do not  synth column if less than this 

	short	 m_pitchelCandidateCount; 
};




//////////////////////////////////////////////////////////////////////////

#endif   // __H_

