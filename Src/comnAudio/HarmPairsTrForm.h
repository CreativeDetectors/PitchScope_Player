/////////////////////////////////////////////////////////////////////////////
//
//  HarmPairsTrForm.h   -   
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_HARMPAIRSTRFORM_H__F15BC24F_B680_11D6_9A48_00036D156F73__INCLUDED_)
#define AFX_HARMPAIRSTRFORM_H__F15BC24F_B680_11D6_9A48_00036D156F73__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   HarmPairsTrForm      :   public   TransformMap            
{ 		                           									

										//  'DOMAIN' (Y-coord)  are   '12  SCALE-pitches'  ( harmonic groups )     

public:  		
    HarmPairsTrForm(  long  width,   long  horzScaleDown   ); 
	virtual	 ~HarmPairsTrForm()    {  }	
		


	/***
	  virtual	  bool        Get_DurationRectangle(    short x,  short y,    short   pixThresh,    short  pixCountThreshold,      
																		short&  retStartPix,     short&  retEndPix,    short&  retChannelIdx,  
																			short&  retAverageRunBriteness,     CString&  retErrorMesg   );
   ***/



//     virtual	  short		  Get_Xcolumns_Channel_Data(  short x,   int  channelCode,   short  pixThresh,  short&  retPixCount  );  
//  ...REPLACED by parent,  TransformMap::Get_Xcolumns_Channel_Value()


      virtual	  void		  Write_Pixel_Cumulative(  long x, long y,  short val  );
 
//					bool		Read_Pitch(   long x,    short  threshold,   CString&  retScaleNoteName   );   ...keep around to study.  12/09
 

					
	 virtual      long      Get_Channels_Center_Yval(  short   channelIdx  )      // WAS is same as  Pitch_2Ycoord(  scalePitch  )
																				 {
																					 ASSERT( 0 );    // *** BAD overide cause tthis has NO center ( 12 data lanes )
																					 return  -1;  
																				 }


	 virtual      long		 Find_ThreeBest_Scalepitch_Scores(   long  x,    short  minPixelCount,    short  retScalePitches[],  
															                  	long  retScores[],     short  retDensities[],    long  retComboScores[],    short  pixThresholdHPairs    );															   




//	 virtual      bool		Get_DuraRects_HarmonicDensity_Gapped(   short  startPixel,    short  endPixel,   short  scalePitch, 
//																                                 short&   retAvgDensity,  short&   retAvgBriteness,   CString&  retErrorMesg    );


	 
//   virtual	  short		 Get_Harmonics_Ycoord(   short  scalePitch,   short  harmNum   );  //  SUBchannel data    





public:						            


//	  short    m_pixelThresholdHPt;   //   in calc  DENSITY,  a pixel must have at least this value to be counted AS
													//  significant. The number of  Significant-Harms found greatly affects the
													//  single row score in CompsiteMap. 
											  
};






#define   kHarmPairsMAPHEIT   157     //   ( 12notes x 13 )[156]   +  HARMpAIRSmapTopPAD[ 1 ]     =  157 


#define   kHarmPairsCHANNELwIDTH   13     //  DATAwidth is  12,    have 1 row for SPACE between channels of MultiWIDTH channelmaps



#define   HARMpAIRSmapTopPAD  1      //  upper SPACE on map boundary




#endif // !defined(AFX_HARMPAIRSTRFORM_H__F15BC24F_B680_11D6_9A48_00036D156F73__INCLUDED_)
