/////////////////////////////////////////////////////////////////////////////
//
//  FundCandidCalcer.h   -   calulates the Octave for a ScalePitch by examining all the possible Harmonics for ALL of the 4 Octave Candidates
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_FUNDCANDIDCALCER_H__6969__INCLUDED_)
#define AFX_FUNDCANDIDCALCER_H__6969__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////


/***
Can be used 2 different ways:

  1)   To compair 4 FundCandids of same scalePitch for best octave
  2)   To generate results for all 48 Rows in the FundamentalMap, on a pixel by pixel basis



***/


class  logDFTtrForm;
class  FundamentalCandidate;





#define  kSUBoCTAVEcOUNTsPITCHmODE   3    //  number of SUBoctaves when ( sameSPitchMode == true )



////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


class  FundCandidCalcer  
{




	//  this is only for  NAVIGATOR/PLAYER,  and not for OLD PitchScope.  


public:
	FundCandidCalcer(  short  candidCount,   bool sameSPitchMode,   long    startSample,      long  endSample,
										                            bool    dynamicAllocFCandids,   
																	short   sPitchVal,  
																	logDFTtrForm&  logDFT,   
																	long  topFrequencyLimit,    long  bottomFrequencyLimit  	);
	virtual ~FundCandidCalcer();



	long		Get_ChunkSize();      //     512  or  1104   dependant on App    9/2012



	static     short       Get_Harmonic_Count()       {   return  12;   }     //  ***HARDWIRED *****

	static     short       Get_SubOctave_Count()      {   return  3;   }   //  the SOURCE of this value





	short		 Calc_Best_Octave_Candidate(         long    spikesWeight,              //   OctaveCorrectParms&  parms,    
													 bool    enableMidiClosenessNeiborsFactor,
													 short   neighborhoodMidi,    
													 bool&   retMarkSPitchRed,   
													 CString&    retErrorMesg  );    // does EVERYTHING



	bool        Gather_SpikeRatio_Data(   CString&  retErrorMesg   );


	bool        Read_Harmonic_Mags_from_logDFT(   CString&  retErrorMesg   );     //  for ALL candids including SUBoctaves

	bool		Read_Neighbor_Mags_from_logDFT(   CString&  retErrorMesg    );   //   the neighbors of a given Spike-Harmonic






	short          Get_Total_FundCandids_In_Array()      {  return   m_fundCandidCount  +  m_subOctaveCount;   }

	short          Get_Actual_FCandidArray_Index(  short  virtualOctaveIdx  )   {  return  virtualOctaveIdx + m_subOctaveCount;  }







public:
	long     m_startSample,    m_endSample;    //  ABSOLUTE,  from the  WAV's very start     ***** TRICKY need this so can read from the logDFT segment  1/10


	logDFTtrForm&   m_logDFT;




	short      m_chosenFundamentalCandIdx;    //  Final-Result:   if less than zero,  then we IGNORE the results in  m_fundCandids[ 3 ]   ..We have NO approx of an Octave!!!

	long       m_bestFinalScore;

	short      m_chosenFundamentalsAvgHarmonicMag;




	FundamentalCandidate    *m_fundCandids;   //  an array to be allocated with  new  operator in constructor

	short            m_fundCandidCount;     //  [ 4 ]  Specify the PASSED-IN, or dynamAllocated array

	short            m_subOctaveCount;      //  [ usally 3 ]  How many Dummy-FundamentalCandidates for SUBoctave measurment


	FundamentalCandidate    m_fundCandidsFive;   //  just to get mag info for 1above





	long   m_topFrequencyLimit;              //  for PRE-filtering the SndSample before AutoCorrel,  just like for the DFT    
	long   m_bottomFrequencyLimit;   


	bool    m_dynamicAllocFCandids;    //  will the FundamentalCandidate be allocted and released internally

	bool    m_useWeitsNeighborhoodStrength;


	short   m_firstMidiFundTemplate;   //  ******* NEW,   is this OK ????? ********************************
};





////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_FUNDCANDIDCALCER_H__6969__INCLUDED_)
