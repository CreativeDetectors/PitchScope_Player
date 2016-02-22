/////////////////////////////////////////////////////////////////////////////
//
//  DFTtransforms.h  -  calculate a Discrete Fourier Transform (DFT) that has its frequency rows spaced logarithmically(corresponding to location of musical harmonics) 
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined( _DFTtRANSFORMS_H_  )

#define _DFTtRANSFORMS_H_

/////////////////////////////////////////////////////////////////////

class  TransformMap;

class  SndSample;

class  logDFTtrForm;

class  PitchEl;

class  FundamentalCandidate;
		
		
///////////////////////////////////////////
		

 // **** can NOT go below  '52'  without fixing  'noteFreq[]'  ***********************

#define  kMIDIsTARTbANDdft  52        //   WAS:   59 [B 246hz],    should be    'kMIDIsTARTnOTE -12'

                       //  '52'  is  E [ 165 hz ] TEMPLATEtrasnsformers reads  '-12'  from its hypothetical fundamental, 
                       //  '71'  is  B [ 493 hz ],  ('midi 64' is  329 Hz [ 3rd string E ]) 



#define  TOTALhARMONICS  7	  //  [ 6: bad,  8: unnecessary ]	****SYNC with 'PitchEl' ************




								
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   logDFTtrForm    :  public  TransformMap            
{ 		      					      
    
public:  			
	logDFTtrForm(   long width,   long chunkSize,   short  usePhaseData,   long horzScaleDown    );
    virtual  ~logDFTtrForm();
	


	virtual   short       Get_Midi_of_FirstBand()      {  return  m_startBandMidi;  }


	
	virtual   bool		  Load_Leading_XColumn(   long  xLead,    CString&  retErrorMesg   );



    long		Pitch_2Ycoord(  short  midiPitch  );   //  'DOMAIN',  with 'OCTAVE reference'!!!

	short		Ycoord_2Pitch(  long  y  );
      

      
    void		Get_Columns_Biggest_FreqComponents(   long x,    short  compFreq[],    short  compAmpl[],   
																			    short  numComponents,   bool  harmonicMasked,
																			    short  filterCode,   short  kernalWidthHorz    );


    long		Get_Columns_Biggest_Y_val(   long x,   short *pixVal,    bool  harmonicMasked,
																				 short  filterCode,   short  kernalWidthHorz   );  // just ONE maximum, and its Ycoord

      

        
	bool		Assign_Pitchels(    ListMemry<PitchEl>&  pitchelList,     CString&  retErrorMesg    );  // *** OMIT ***  7/2002

//	long     	Assign_Specific_Pitchels(    ListMemry<PitchEl>&   pitchelList,    short  targetScalePitch,   long  startOffset,
//															    long  endOffset,    bool  atMapStart,   ListLink<PitchEl>*  indexesListlink  );



	short		Make_SharedHarmonics_List(   short scalePitch0,  short scalePitch1,    short  sharedHarmonicsBaseMidi,
																												short  retShareHarms[]  );
	



																					//   'Harmonic-TEMPLATE'   measurement


	bool		Is_HarmonicBand_On_Map(   short  fundamentalPitch,   short  harmonicIdx  );  

	bool		Is_MidiPitch_On_Map(   short  midiPitch   );




	bool		Sum_All_Harmonics_Magnitudes_inSpan(   long  x0,   long  x1,    short  midiPitch,     short  numHarms,
																		         FundamentalCandidate&   fundCandid,   																
																		         short&  retMissedBandCnt,    CString&  retErrorMesg   );

	bool		Calc_Average_PixelMagnitude_inSpan(   long  x0,   long  x1,    short  midiPitch,    long&  retAvgMag,  
																				             bool&  retOffMap,    CString&  retErrorMesg   );



	long		Sum_All_Octaves_ScalePitch_Mags_inSpan(   short  midiPitch,      long x0,   long x1,      short  filterCode, 
																					short  kernalWidth,     CString&  retErrorMesg  );    //  EX:   for E, sums values of ALL E's in DFT





    long		Get_FundamentalTemplate_Score(   short  midiPitch,   long x,    short  harmMags[],   short  harmonicThreshold, 
																	  short&   retHarmonicCount,   short  filterCode,   short  kernalWidthHorz    );


	long	    Get_ScalepitchTemplate_Score(   short  scalePitch,    long x,    short  harmMags[],    short   totHarms, 
														           short  harmonicThreshold,    short&  retHarmonicCount,   long&  retRawScore,  
																   short  filterCode,   short  kernalWidthHorz  );

	long		Get_ScalepitchTemplate_Score_wExclusions(  short  scalePitch,   long x,    short  sharedHarms[],   
																									 short  sharedCount,   long&  retRawScore,
																								  short  filterCode,   short  kernalWidthHorz  );

	long		Get_ScalepitchTemplate_Score_xSpan(   short  scalePitch,    long x0,   long x1,    long&  retRawScore   );



	bool		Get_HarmonicJudgement_Data(    long x0,   long x1,     short  retFirstFifthScores[],   
																				   short  retThirdSeventhScores[],     CString&  retErrorMesg  );




	long		FindBest_ScalepitchTemplate_Score_2Phase(   long  x,    short&  retScalePitch,    bool  useWeightedScores,  //  *** NOT USED ***
																		bool&  retDidSwap,    short&  retScalepitchSecond,   long&  retScoreSecond  );
																			

	long		Find_ThreeBest_ScalepitchTemplate_Scores(   long  x,   bool  useWeightedScores,short  retScalePitches[],    
																					  long  retScores[],  short   filterCode,   short  kernalWidthHorz  );															   


	bool		Calc_ScalePitch_Strength_in_Spectrum(   long  x,    short  scalepitchIdx,    long&  retScore,   short  filterCode,     
																			   short  kernalWidthHorz,   bool  readDftMasked,   CString&  retErrorMesg  );


//  ***INSTALL:   short   Get_Xcolumns_Channel_Data(  short x,   int  channelCode,   short  pixThresh,  short  retPixCount  );  





public:  
	  short        m_startBandMidi;    


//	 double     _Freq[  MAXfreqMAPHEIT  ];      //  in Hertz 
  	 double     _Freq[  kTOTALdftBANDs  ];      //  in Hertz 


      long        _timeWidth;      //  ChunkSize,  in  'SAMPLEs'...
   

      long        _origSampLen;   // careful to keep updated 
       					      
	  

	       
      TransformMap    *_phaseMap;     //  **** OMIT eventually,  do NOT use it


	  long    	m_totalFloatmapPixels;   


	  short     m_sharedHarmonicsBaseMidi;     //  ***???  was  64,  E  and the start of Fundamental.    Should change ???  2/04 ********
};




#define  LOGdFTmapTOPpAD  0       //  Do NOT change,  too much code depends on this.   11/2003




/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

#endif   // __H_

