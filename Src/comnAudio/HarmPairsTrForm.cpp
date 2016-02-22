/////////////////////////////////////////////////////////////////////////////
//
//  HarmPairsTrForm.cpp   -   
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


#include  "..\comnFacade\VoxAppsGlobals.h"


////////////////////////////////////////////////// 
#include  "..\comnFoundation\AbstractFounda.h"   
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"
#include  "..\ComnGrafix\CommonGrafix.h"      //  #include "commonmy.h"   	

#include  "..\ComnGrafix\OffMap.h"  
#include  "..\comnGrafix\TransformMap.h"



#include  "..\comnAudio\FundamentalCandidate.h"





#include "HarmPairsTrForm.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////



extern   SPitchListCreateParms           lastCreateParmsGLB;

// extern   DerivitiveMapsCreateParms    lastDerivitiveMapsCreateParmsGLB;       *****  Keep this out  for PsPlayer.exe    12/09



void      Get_ScalePitch_LetterName(  short  sclPitch,   char *firLet,  char *secLet  );




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


HarmPairsTrForm::HarmPairsTrForm(   long  width,   long  horzScaleDown   )  
														:  TransformMap(   width,   kHarmPairsMAPHEIT,  8,  horzScaleDown  )
{



// **********************   HARDWIRE,  fix with Parms Structure ****************************
	
//	m_duraRectPixelThresh =   30;    //  40     for   Get_DurationRectangle  ...tricky to pass this thry polmorphism
//	m_duraRectPixelCount  =     2;    



	m_hasChannels  =   true;



	m_channelCount =   12;


					//  for  MULTI-CHANNEL  maps(  Composite & HarmPairs ),   m_channelDataWidth =   m_channelWidth - 1 

	m_channelWidth         =     kHarmPairsCHANNELwIDTH;          //   13;         //  12 plus 1 row padding between chnnels
	m_channelDataWidth  =     kHarmPairsCHANNELwIDTH  -1;     //   12


	m_topPad  =   HARMpAIRSmapTopPAD;    //  1   



    Clear( 0 );    // 0: BLACK          ???SHOULD check if got Mac GWorld OK????  
}



											////////////////////////////////////////

/***
short   HarmPairsTrForm::Pitch_2Ycoord(  short  scalePitch  )  
{   

							//  Gives the Ycoord of the CENTER harmonic( H-0 ) of the channel


	short    channelWidth =   Get_Channel_Width();



// ***** LOOKS funny ????   ***********

    short    yCoord  =    (m_height  -1)    -    (   (scalePitch  *  channelWidth)    + HARMMAPTopPAD   );   




	short   yCenterDebug =      Get_Channels_Center_Yval(   scalePitch   );      // ...so DEBUG test here 

	ASSERT(   yCoord  ==    yCenterDebug   );





	return   yCoord;				 //  ( 12notes x 14 )[168]  +  HARMMAPTopPAD( 8 )  + 1    =  177    
}
***/											


											////////////////////////////////////////

/***
bool    HarmPairsTrForm::Ycoord_2Pitch(   short y,     short&  retScalePitch   )  
{   
																			//  retScalePitch :   { 0 - 11 }   


// ******* FIX,  still gives a value for the  'Pad rows'  ????   ...should i return false ??? *********   4/02




	short    channelWidth =   Get_Channel_Width();      //   the separation between ScaleNote-Channels

	short    pad  =     HARMpAIRSmapTopPAD;      //  1        



    short   ycoordVirt  =     ( ( m_height  - 1 )   -  y )   -  pad;     //  what the coord would be if the map were  NOT INVERTED, and NOT offseted by PAD at top 



	retScalePitch  =     ycoordVirt   /  channelWidth;    
										
	


	short       channelIdxDebug =  	  Ycoord_2ChannelIdx(  y  );    // ***** OMIT,  just a test *****

	ASSERT(  channelIdxDebug  ==   retScalePitch   );



	
	return  true;
}
***/

											////////////////////////////////////////

/***
short   HarmPairsTrForm::Get_Harmonics_Ycoord(    short  scalePitch,   short harmNum    )  
{           
	

ASSERT( 0 );   //  ******Currently  DISABLED******    3/02



     short  y,  vrOfst,  yCoord;           //  'harmNum'   start with ZERO [ 0 ],  not one 


     y =   Pitch_2Ycoord(  scalePitch  ); 
     
     
     if(    (harmNum % 2  ) == 0   )  
		 vrOfst =   harmNum /2;            // Offset harmonic CONTRIBUTION  
     else                     
		 vrOfst =  -harmNum /2;
// ******* FIX *****************


	 yCoord  =    y  +  vrOfst;     

     return    yCoord;
}
***/


											////////////////////////////////////////


void   HarmPairsTrForm::Write_Pixel_Cumulative(   long x, long y,    short val )
{

    short    rd, gr, bl;                 // should i do BOUNDS CHECK ????

    Lock_Bits();			         // write & read to 'GWorld'.....



    Read_Pixel(  x,y,     &rd,   &gr, &bl  );     // **** No big deal( no filtering, ever ) cause read from SELF 
    


    rd  +=   val;      
    if( rd > 255 )    rd = 255;
    
    Write_Pixel(  x,y,   rd, rd, rd   );
  
    Unlock_Bits();                        // ...restore
}  



											////////////////////////////////////////

/***
bool   HarmPairsTrForm::Read_Pitch(   long x,    short  threshold,     CString&  retScaleNoteName   )
{



//	short  pixThresh  =   m_pixelThreshold;   //  40  ******* ADJUST,  but not yet connected to anything   5/2002  *************


ASSERT( 0  );    // *******not yet connected to anything   5/2002




	// **** SHOULD reexamine what I am doing here, and where this funct is CALLED FROM ******  5/2002



										      //  threshold <0 :     thresholding is disabled 
	short   bigestVal    =  -1;      
	int       strongestPitch  =   -1;     
	char    firLet,  secLet; 

	retScaleNoteName.Empty();
	Lock_Bits();					 




	for(    int scalePitch= 0;     scalePitch <  12;      scalePitch++    )
	{
		short   retPixCount =0;

		short  averageVal  =      Get_Xcolumns_Channel_Value(   x,   scalePitch,    
																						lastDerivitiveMapsCreateParmsGLB.pixThresholdHPairs,	//	m_pixelThresholdHPt, 
																								retPixCount   );
		if(      averageVal   >   bigestVal    )                                 
		{   
			strongestPitch =    scalePitch;     
			bigestVal        =    averageVal;  
		}
	}

	Unlock_Bits();    

    

	if(    strongestPitch  ==  -1   )
	{
		ASSERT( 0 );
		return  false;
	}



	if(   threshold  > 0   )	  //  Is threshold mode active ??? 
	{

		if(    bigestVal  >=   threshold    )
		{
			Get_ScalePitch_LetterName(   strongestPitch,    &firLet,  &secLet   );
			retScaleNoteName.Format(   "%c%c",    firLet,  secLet   );
		}
		else
			retScaleNoteName.Empty();
	}

	return  true;
}
*****/


											////////////////////////////////////////


long     HarmPairsTrForm::Find_ThreeBest_Scalepitch_Scores(   long  x,    short  minPixelCount,    short  retScalePitches[],    
												 	     long  retScores[],   short  retDensities[],    long  retComboScores[],    short  pixThresholdHPairs   )
{	


								//


  ASSERT( 0 );      //   REALLY  not USED except in some experimental commented out stuff. Keep it around though.   12/09



	long     reDux  =   4;      //  dim the volume on the ComboScores





//	short    pixThresh  =    m_pixelThreshold;       //  30  



	short    candidateCount =   3;


	short   retPixCount;
	long    comboScore,   pixScore;  


	for(    short i=0;     i< candidateCount;     i++    )
	{
		retScalePitches[ i ]  =  -1;  		
		retScores[ i ]          =  -1;  
		retDensities[ i ]       =  -1;  
		retComboScores[i]  =  -1; 
	}




	for(    short  scalePitch =0;      scalePitch <  12;      scalePitch++    )
	{

		pixScore =	   Get_Xcolumns_Channel_Value(   x,   scalePitch,    pixThresholdHPairs,    //   lastDerivitiveMapsCreateParmsGLB.pixThresholdHPairs, 
																															retPixCount   );  		



// **** HARD:  how to weight these ???  ********

		if(    retPixCount  >=   minPixelCount    )
		{

			long   numer =    10L;



// **** ADJUST Weights  ****************************************************************

			comboScore =      (     (long)pixScore   *    (long)retPixCount    )              /  reDux;   
//			comboScore =      (     (long)pixScore   *    (  (  (long)retPixCount  *    numer ) /10L   )      )        /  reDux;   



		}
		else
			comboScore =   0L;    //  OK ????




		if(    comboScore   >   retComboScores[ 0 ]   )    // Is it big enough to replace the current TOP score ??
		{

			retScores[ 2 ]         =    retScores[ 1 ];        //  First bump the Second Max to the Third,
			retScalePitches[ 2 ] =    retScalePitches[ 1 ];			
			retDensities[ 2 ]      =    retDensities[ 1 ];
			retComboScores[ 2 ]  =  retComboScores[ 1 ]; 
			

			retScores[ 1 ]         =    retScores[ 0 ];        //  ...bump the current Max to the Second,
			retScalePitches[ 1 ] =    retScalePitches[ 0 ];		
			retDensities[ 1 ]      =    retDensities[ 0 ];
			retComboScores[ 1 ]  =  retComboScores[ 0 ]; 
			

			retScores[ 0 ]         =     pixScore;
			retScalePitches[ 0 ] =     scalePitch; 
			retDensities[ 0 ]      =     retPixCount;
			retComboScores[ 0  ]  =  comboScore; 
		}
		else
		{														//  ...if not,  is it big enough to replace the current SECOND score

			if(    comboScore  >   retComboScores[ 1 ]   )
			{

				retScores[ 2 ]         =    retScores[ 1 ];        //   bump the Second Max to the Third
				retScalePitches[ 2 ] =    retScalePitches[ 1 ];				
				retDensities[ 2 ]      =    retDensities[ 1 ];
				retComboScores[ 2 ]  =  retComboScores[ 1 ]; 


				retScores[ 1 ]         =    pixScore;
				retScalePitches[ 1 ] =    scalePitch; 
				retDensities[ 1 ]      =    retPixCount;
				retComboScores[ 1  ]  =  comboScore; 
			}
			else
			{ 
				if(    comboScore  >   retComboScores[ 2 ]   )
				{

					retScores[ 2 ]         =     pixScore;
					retScalePitches[ 2 ] =     scalePitch; 
					retDensities[ 2 ]      =     retPixCount;
					retComboScores[ 2  ]  =  comboScore; 
				}
			}

		}
	}



	long     lastDiff  =     retComboScores[ 1 ]   -   retComboScores[ 2 ];      //  How TIGHT was the last match ??
	return  lastDiff;
}




											////////////////////////////////////////

/****
bool    HarmPairsTrForm::Get_DuraRects_HarmonicDensity_Gapped(   short  startPixel,    short  endPixel,   
																									   short  scalePitch,  short&   retAvgDensity,  																										
																						   short&   retAvgBriteness,    CString&  retErrorMesg   )
{

					//	now has a   2 digit  DECIMAL    measurement for  'retAvgDensity'    ( ex:   46  =   4.6 Rows high )    

	retErrorMesg.Empty();		//  CALLED from    AnalyzerAudio::Approximate_HarmonicDensity_All_Scalepitches()
										   //		  	   and      SoundMan::Add_ScalePitch_Object()
	retAvgDensity   =   -1;
	retAvgBriteness =   -1;


	if(    startPixel   >=    endPixel    )
	{
		ASSERT( 0 );	//  some times 2 have same value

		retAvgDensity   =     0;    // OK ??? or I should measure ??? 
		retAvgBriteness =    0;

		return  true;     // **** OR false ??? ****
	}



	long   totalTone           =   0;
	long   totalWidthPixCnt =   0;
	long   numPixReads      =   0;

	int     channelIdx  =     scalePitch;     // **** OK ????




	for(    short x =  startPixel;       x <=  endPixel;       x++    )
	{

		short  retPixCount;
		short  avgTone  =     Get_Xcolumns_Channel_Value(   x,  channelIdx,     m_pixelThreshold,    retPixCount   ); 

		totalTone            +=    avgTone;
		totalWidthPixCnt  +=    retPixCount;

		numPixReads++;
	}




	if(    numPixReads  > 0    )
	{

//	    retAvgDensity    =      (short)(     totalWidthPixCnt               / numPixReads    );
		retAvgDensity    =      (short)(   ( totalWidthPixCnt  * 10  )    / numPixReads     );     //  now has 2 digits ( ex:  46 =   4.6 Rows high ) 


		retAvgBriteness =       (short)(    totalTone              / numPixReads    );
	}
	else
		ASSERT( 0 );


	return  true;
}
*****/
