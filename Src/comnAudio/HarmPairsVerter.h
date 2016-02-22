/////////////////////////////////////////////////////////////////////////////
//
//  HarmPairsVerter.h   -   use Piszczalski algorythm for Harmonic Pairs to calulate the ScalePitch for a time duration
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


/*
   'ScalePitch' has 12 possible pitch values:  { E, F, F#, G, G#, A, A#, B, C, C#, D, D# } because there is NO octave calulation yet. 
	
	{  Do, Re, Mi, Fa, So, La, Ti  }  is another way to express ScalePitch because it also contains no octave assignment. 


	"Predicting Musical Pitch from Component Frequency Ratios" (1979),   Martin Piszczalski and Bernard Galler,  University of Michigan  
*/



#if !defined(AFX_HARMPAIRSVERTER_H__7C0DE061_B688_11D6_9A48_00036D156F73__INCLUDED_)
#define AFX_HARMPAIRSVERTER_H__7C0DE061_B688_11D6_9A48_00036D156F73__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   HarmPairsVerter     :   public   FowardMapVerter       
{ 		   
	

public:  
    HarmPairsVerter(   logDFTtrForm&  freqMap,     HarmPairsTrForm  *harmMap,    short  componentCount,
																	short  filterCodeDFT,    short  kernalWidthDFT	);  
	virtual	 ~HarmPairsVerter()    {  }	
  


				void       Set_Volume(  double volumeInPercent  )     {   m_overallVolume =   volumeInPercent / 100.0;   } 
										//   This  controls the PIXEL Briteness of the HarmPairsMap.  It does not impact the SOURCE calcs, just boosts the Piel briteness.



    virtual   void      Transform();   
    
    virtual   void      Transform_Column(    long x   ); 
      


    virtual   short     Get_Columns_OctavePick(   long  xLead,    short  targSPitchVal,     long  retOctvCandidsScores[]     );





public:  
      logDFTtrForm          &_DFTmap;        //  SRC   

      HarmPairsTrForm    *_harmMap;       //  DEST


	  double  m_overallVolume;     //  how bright the bitmap will be   6/07


	  short    m_componentCount;   // New,  toggle from Options dialog ??? 


	  long     m_harmPopulation[ 12 ];   //   0 - 11  scalePitches.     count how many o each type of harms we find


	  long     m_overBriteCnt;      //  How many pixel calcs were OVER 255 (and were badly truncated )




	  short  m_filterCodeDFT;  

	  short  m_kernalWidthDFT;    //  in Virtual pixels???    Or new var for High Resolution.   FLOATING-Point  for virtual kernal ????  **********



	  short  m_overideDFTreadColumn;     //  NEW,  is for SPitchCalc  so that I can specify the READ column          1/10


	long	   m_hiResRatioLogDFT;      // ****  NEW  9/12 ,  can make more HiRes when using CircQueLogDFT algo,  just read thye Que at more Tap-Points
};




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   DFTedgeVerter     :   public   FowardMapVerter       
{ 		   
	

public:  
    DFTedgeVerter(   logDFTtrForm&  freqMap,    TransformMap&  edgeMap 	);  
	virtual	 ~DFTedgeVerter()    {  }	
  

    virtual   void       Transform();   

    virtual   void       Transform_Column(    long x   ); 
      






// *** Install a Briteness Control ??   ...that does NOT disturb other maps ******



public:  
      logDFTtrForm&      m_DFTmap;         //  SRC   

      TransformMap&     m_edgeMap;       //  DEST




//	  long     m_overBriteCnt;      //  How many pixel calcs were OVER 255 (and were badly truncated )


//	  short  m_filterCodeDFT;  
//	  short  m_kernalWidthDFT;
};




#endif // !defined(AFX_HARMPAIRSVERTER_H__7C0DE061_B688_11D6_9A48_00036D156F73__INCLUDED_)
