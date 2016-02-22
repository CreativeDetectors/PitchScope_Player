/////////////////////////////////////////////////////////////////////////////
//
//  HarmPairsVerter.cpp   -   use Piszczalski algorythm for Harmonic Pairs to calulate the ScalePitch for a time duration
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

#include  "..\comnGrafix\mapFilters.h"

#include  "..\comnAudio\FundamentalCandidate.h"



#include  "DFTtransforms.h"
#include  "HarmPairsTrForm.h"



#include "HarmPairsVerter.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define NUMFluffRats  25

myPair  FluffRats[ 25 ]=		// **** ENHANCE by assigning weights to the different SETS( some are more important ) ****
  {  
     {  2,  1 },   {  4,  2 },    {  6,  3 },    {  8,  4 },   
     {  3,  1 },   {  6,  2 }, 
     {  4,  1 },   {  8,  2 },   
     {  5,  1 },   
     
     {  3,  2 },   {  6,  4 },  
     {  5,  2 },  
     
     {  4,  3 },   {  8,  6 },    //  THOUGH these are TRIGGERED by the same 'Real-Component Pair',  they VISUALLY 
										//  extend the screen write outward from the horizontal axis. 	
     {  5,  4 },    

     {  6,  1 },  
     {  6,  5 },   

     {  7,  1 },   
     {  7,  2 },   
     {  7,  3 },   
     {  7,  4 },   
     {  7,  5 },   
     {  7,  6 },   
    
     {  8,  1 },   
     {  8,  3 }       };

	
	//////////////////////////////////////////////////////////

#define NUMBasicRats  19

myPair  BasicRats[ 19 ]=     // no  'HIGHER multiples'  of   Same-Value
  {  
     {  2,  1 },     
     {  3,  1 },     
     {  4,  1 },  
     {  5,  1 },   
     
     {  3,  2 },    
    
     {  5,  2 },  
     {  4,  3 },  
    
     {  5,  3 },  
     {  5,  4 },  

     {  6,  1 },  
     {  6,  5 },    

     {  7,  1 },   
     {  7,  2 },   
     {  7,  3 },   
     {  7,  4 },   
     {  7,  5 },   
     {  7,  6 },   
    
     {  8,  1 },   
     {  8,  3 }    };


	//////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////


#define NUMFullHarmRATS 48

myPair  FullHarmRats[ 48 ]= 
  {  
	 {  7,  6 },  
     {  6,  5 },   { 12, 10 },  
     {  5,  4 },   { 10,  8 },  
     {  4,  3 },   {  8,  6 },   { 12,  9 },  
     {  7,  5 },  
     {  3,  2 },   {  6,  4 },   { 12,  8 },    { 9,  6 },  
     { 11,  7 },  
     {  5,  3 },   { 10,  6 },  
     {  7,  4 },  
     { 11,  6 },  
     {  2,  1 },   {  4,  2 },   {  6,  3 },    { 8,  4 },  
     { 11,  5 },  
     {  7,  3 },  
     {  5,  2 },   { 10,  4  },  
     {  8,  3 },  
     {  3,  1 },   {  6,  2 },   { 12,  4 },    { 9,  3 },  
     { 10,  3 },  
     {  7,  2 },  
     { 11,  3 },  
     {  4,  1 },   {  8,  2 },   { 12,  3  },  
     {  9,  2 },  
     {  5,  1 },   { 10,  2 },  
     {  6,  1 },   { 12,  2 },  
     {  7,  1 },  
     {  8,  1 },  
     {  9,  1 },  
     { 10,  1 },  
     { 11,  1 },  
     { 12,  1 }    };


		//////////////////////////////////////////////////////////

#define NUMFirRats 29

myPair  FirRats[ 29 ]=     // ****looks almos the SAME as 'BasicRats[]'  but goes to 12 th harmonic,  not 8
  {  {  7,  6 },  
     {  6,  5 }, 
     {  5,  4 },  
     {  4,  3 },  
     {  7,  5 },  
     {  3,  2 },  
     { 11,  7 },  
     {  5,  3 },  
     {  7,  4 },  
     { 11,  6 },  
     {  2,  1 },   
     { 11,  5 },  
     {  7,  3 },  
     {  5,  2 },   
     {  8,  3 },  
     {  3,  1 },  
     { 10,  3 },  
     {  7,  2 },  
     { 11,  3 },  
     {  4,  1 },  
     {  9,  2 },  
     {  5,  1 },   
     {  6,  1 },  
     {  7,  1 },  
     {  8,  1 },  
     {  9,  1 },  
     { 10,  1 },  
     { 11,  1 },  
     { 12,  1 }    };


myPair SecRats[ 19 ]=     
  {  { 12, 10 },  
     { 10,  8 },  
     {  8,  6 },   { 12,  9 },  
     {  6,  4 },   { 12,  8 },    { 9,  6 },  
     { 10,  6 },  
     {  4,  2 },   {  6,  3 },    { 8,  4 }, 
     { 10,  4 },  
     {  6,  2 },   { 12,  4 },    { 9,  3 },  
     {  8,  2 },   { 12,  3  },  
     { 10,  2 },  
     { 12,  2 }   
 };




////////////////////////////////////////////////////////////////////////////////////////////////////////////


short  Find_Freqs_ScalePitch(  short  freq  );



////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


HarmPairsVerter::HarmPairsVerter(   logDFTtrForm&  freqMap,    HarmPairsTrForm  *harmMap,   short  componentCount,
																								short  filterCodeDFT,    short  kernalWidthDFT   )  
				:   _DFTmap( freqMap ),     _harmMap( harmMap ),     m_componentCount( componentCount ),
				     m_filterCodeDFT( filterCodeDFT ),    m_kernalWidthDFT( kernalWidthDFT )
{ 

	m_overallVolume =   0.00;    //   is set by calling function and   HarmPairsVerter::Set_Volume() 

	m_overideDFTreadColumn =   -1;

	m_hiResRatioLogDFT =   1;   //  default,  so this will be OK for OLD PitchScope
}



												////////////////////////////////////////


void   HarmPairsVerter::Transform()
 {

     short   x;   
 //  short   compFreq[ 10 ],  compAmpl[ 10];    
		 

	 for(    short  i=0;    i < 12;     i++    )
		 m_harmPopulation[ i ] =   0L;


	m_overBriteCnt =  0L;



	long   sourceTransWidth;


	if(     Needs_Offsetting()     )       // *************************  REDO this   9/12   ??????????????????????????????????
	{
		sourceTransWidth  =     _harmMap->m_width;
	}
	else
		sourceTransWidth  =     _DFTmap.m_width;



//	short   componentCount  =     m_componentCount;    // ******** ADJUST *************************

//	short   componentCount  =     4;     //  ****   TEMP,  just for test ****     start conservative with   Piszczalski  original recommendation( 4 ) 

//	short   columnsNoteLikelyOctaveIdx;




     for(    x = 0;     x <  sourceTransWidth;      x++   )    
	 {

		 Transform_Column(  x  );    

// ******************************   TEMP,  just for test ********************************
//		 columnsNoteLikelyOctaveIdx  =      Get_Columns_OctavePick(   x,   componentCount   );
	 }
      


     //  ReDraw_Display_temp();       
	 
	 //  Status_Mesg( "HarmPairTransform is DONE. "  );

/***
	 TRACE(  "\n\nHarm-Population:  1[ %d ],  2[ %d ],  3[ %d ],  4[ %d ],  5[ %d ],  6[ %d ],  7[ %d ],  8[ %d ],  9[ %d ],  10[ %d ],  11[ %d ],  12[ %d ] \n" ,
		 m_harmPopulation[ 0],  m_harmPopulation[1 ],  m_harmPopulation[2 ],  m_harmPopulation[ 3],  m_harmPopulation[4 ],  m_harmPopulation[ 5],  
		 m_harmPopulation[ 6],  m_harmPopulation[7 ],  m_harmPopulation[8 ],  m_harmPopulation[9 ],  m_harmPopulation[10 ],  m_harmPopulation[11 ]  );
***/

//	 TRACE(    "\n\nHarmPAIRS  transform is  DONE [  Overbrite Count:  %d   ]  \n",    m_overBriteCnt    );
  }      




												////////////////////////////////////////


void   HarmPairsVerter::Transform_Column(   long  xLead   )  
{  


	            //  From    "Predicting Musical Pitch from Component Frequency Ratios" (1979),   Martin Piszczalski and Bernard Galler,  University of Michigan  


	bool    useDFTmask      =   false;    //   true     ***CHANGED  3/2004 ******



	bool    useKlapuriWeits =    false;   //   false   11/11      true[ very bad PeopleStrange,  12/2011  ..do I have a BUG?                  
														//  **** KlapuriWeits BUGGY ???  Need to examine??    12/2011   ***********************
				//   BUT Since Piszczalski was so brilliant in the invention and execution of this algo, it makes sense to trust his judgement
				//   Varying the values does not seem to have an INCONSISTANT effect    11/2011



	double   ratioTolerance  =   0.025;     //    0.025;        0.015    *****  ADJUST different values ???   .30 did not look bad ???  *********************
													//  Varying the values does not seem to have much effect  11/2011


	double   briteRedux;           //  0.40   ****ADJUST (below in switch()  BRITENESS,  affects BOTH weiting systems equally




	short    ratioSetChoice  =   2;     //  2   { 2 or 1  } is good    [  1 seems like it misses notes for Clarenet     11/11  ]
			/***
					case 0:   numIdealRats =    NUMFluffRats;           break;     //  only  8 harmonics,  ALL combos  
					case 1:   numIdealRats =    NUMBasicRats;          break;     
					case 2:   numIdealRats =    NUMFullHarmRATS;    break;    //         12 harmonics,  ALL combos 
					case 3:   numIdealRats =    NUMFirRats;              break;
			***/




	///////////////////////////
	long   xRead= -1,    xWrite=-1;

	TransformMap    *destinationMap =     _harmMap;
	if(  destinationMap == NULL )
	{   ASSERT( 0 );	 return;  }


//	TransformMap   *sourceMap =    destinationMap->m_sourceMapFloat;     //  here should be same as  '_DFTmap' 
	TransformMap   *sourceMap =    &_DFTmap;



	if(     destinationMap->m_inFloatmapMode     )  
	{
		xWrite  =   destinationMap->m_width  -1;       //  ALWAYS writing to the  LAST COLUMN,    for an OLD-STYLE  FloatingMap
	}
	else
		xWrite  =    xLead;			//   NORMAL,  NON-Floaing mode   for    HARM-PAIRS  map
		



	if(     sourceMap->m_inFloatmapMode    )
	{

	    ASSERT(   sourceMap  ==   &_DFTmap   );       //   from above


		if(    m_overideDFTreadColumn  >=  0    )
		{
			xRead =     m_overideDFTreadColumn;     //  ***NEW,  is for SPitchCalc ***  so that I can specify the read column   ( NEW STYLE  FloatingMap  )   1/2010
		}
		else
		{  ASSERT( 0 );   // Do I USE this ????   9/2/2012:

			xRead  =    sourceMap->m_width  / 2;      //   the  CENTER Column  in the FloatMap ( log DFT )
		}
	}
	else
	{  /***
		if(   Using_DetectZones_logDFT()   )
			xRead  =    xLead;	
		else
		{
			if(   Needs_Offsetting()    )			//   are we transforming just a SEGMENT of the entire sample...
				xRead  =    xLead    +   (short)m_pixelReadOffset;
			else
				xRead  =    xLead;			//   NORMAL,  NON-Floaing mode  ...but JUST during 
		}		
		***/
		xRead  =    xLead;	
	}
	///////////////////////////




	short   i,   compFreq[ 20 ],  compAmpl[ 20 ];    

	for(   i= 0;     i < 20;     i++   )
	{
		compAmpl[ i ] =   -1;
		compFreq[ i ] =   -1;
	}




	short   adjustKernalWidthDFT   =    m_kernalWidthDFT;


	if(    m_hiResRatioLogDFT  >  1     )            // **** Do I really want this ???   9/2012  ****
	{
//		adjustKernalWidthDFT   =     m_kernalWidthDFT   *     m_hiResRatioLogDFT;   
	}




	_DFTmap.Get_Columns_Biggest_FreqComponents(    xRead,     compFreq,  compAmpl,    m_componentCount,   
																useDFTmask,	   m_filterCodeDFT,    adjustKernalWidthDFT    ); 



	myPair   *curCompIdx =  NULL;


	
	myPair  cmpIDX2[ 1 ] =   {     { 0, 1 }    };    // *** MY new experiment   12/11 ****************************  
															          // Is this all   'Pair-Combinations'  for a 2 number set  [ 0, 1  ]  

	
	myPair  cmpIDX3[ 3 ] =   {     { 0, 1 },   { 0, 2 },       // *** MY new experiment   12/11 ****************************  
																{ 1, 2 } 	     };    // Is this all   'Pair-Combinations'  for a 3 number set  [ 0, 1, 2  ]  



	myPair  cmpIDX4[ 6 ] =   {     { 0, 1 },   { 0, 2 },   { 0, 3 },   
																{ 1, 2 },   { 1, 3 },   
																				{ 2, 3 }  	     };    // Is this all   'Pair-Combinations'  for a 4 number set  [ 0, 1, 2, 3  ]  ?



	 myPair  cmpIDX5[ 10 ] =   {   { 0, 1 },   { 0, 2 },     { 0, 3 },    { 0, 4 }, 
																{ 1, 2 },     { 1, 3 },     { 1, 4 },   
																				{ 2, 3 },     { 2, 4 },  
																								{ 3, 4 }		};
	 	 

	 myPair  cmpIDX6[ 15 ] =   {   { 0, 1 },   { 0, 2 },   { 0, 3 },    { 0, 4 },     { 0, 5 }, 
																{ 1, 2 },   { 1, 3 },    { 1, 4 },      { 1, 5 }, 
																				{ 2, 3 },    { 2, 4 },    { 2, 5 }, 
																								{ 3, 4 },    { 3, 5 },
																												{ 4, 5 }   
																															};


	 myPair  cmpIDX7[ 21 ] =   {   { 0, 1 },   { 0, 2 },   { 0, 3 },    { 0, 4 },     { 0, 5 },    { 0, 6 }, 
																{ 1, 2 },   { 1, 3 },     { 1, 4 },    { 1, 5 },    { 1, 6 },
																				{ 2, 3 },    { 2, 4 },    { 2, 5 },    { 2, 6 }, 
																								{ 3, 4 },    { 3, 5 },    { 3, 6 },
																												{ 4, 5 },     { 4, 6 },
																																{ 5, 6 }  
																																				};

	 myPair  cmpIDX8[ 28 ] =   {   { 0, 1 },   { 0, 2 },   { 0, 3 },    { 0, 4 },     { 0, 5 },    { 0, 6 },    { 0, 7 }, 
																{ 1, 2 },   { 1, 3 },     { 1, 4 },    { 1, 5 },    { 1, 6 },    { 1, 7 },
																				{ 2, 3 },    { 2, 4 },    { 2, 5 },    { 2, 6 },    { 2, 7 }, 
																								{ 3, 4 },    { 3, 5 },    { 3, 6 },    { 3, 7 },
																												{ 4, 5 },     { 4, 6 },    { 4, 7 },
																																{ 5, 6 },     { 5, 7 },  
																																				{ 6, 7 }  
																																		 };


	 myPair  cmpIDX9[ 36 ] =   {   { 0, 1 },   { 0, 2 },   { 0, 3 },    { 0, 4 },     { 0, 5 },    { 0, 6 },    { 0, 7 },     { 0, 8 }, 
																{ 1, 2 },   { 1, 3 },     { 1, 4 },    { 1, 5 },    { 1, 6 },    { 1, 7 },    { 1, 8 },
																				{ 2, 3 },    { 2, 4 },    { 2, 5 },    { 2, 6 },    { 2, 7 },    { 2, 8 }, 
																								{ 3, 4 },    { 3, 5 },    { 3, 6 },    { 3, 7 },    { 3, 8 },
																												{ 4, 5 },     { 4, 6 },    { 4, 7 },    { 4, 8 },
																																{ 5, 6 },     { 5, 7 },     { 5, 8 },    
																																				{ 6, 7 },      { 6, 8 },   
																																									{ 6, 8 }
																																													};


	 myPair  cmpIDX10[ 45 ] = {   { 0, 1 },   { 0, 2 },   { 0, 3 },    { 0, 4 },     { 0, 5 },    { 0, 6 },    { 0, 7 },     { 0, 8 },      { 0, 9 }, 
																{ 1, 2 },   { 1, 3 },     { 1, 4 },    { 1, 5 },    { 1, 6 },    { 1, 7 },    { 1, 8 },       { 1, 9 },
																				{ 2, 3 },    { 2, 4 },    { 2, 5 },    { 2, 6 },    { 2, 7 },    { 2, 8 },       { 2, 9 }, 
																								{ 3, 4 },    { 3, 5 },    { 3, 6 },    { 3, 7 },    { 3, 8 },       { 3, 9 },
																												{ 4, 5 },     { 4, 6 },    { 4, 7 },    { 4, 8 },      { 4, 9 },
																																{ 5, 6 },     { 5, 7 },     { 5, 8 },     { 5, 9 },      
																																				{ 6, 7 },      { 6, 8 },     { 6, 9 },   
																																									{ 6, 8 },   { 6, 9 },
																																													{ 7, 9 }
																																													};


	short  realRatioCount;      // ****** can ADJUST,  [ 4 - 6  ]  *******


	if(          m_componentCount  ==   2    )             
	{
		realRatioCount =   1; 
		curCompIdx =     &(   cmpIDX2[ 0 ]   );

		briteRedux  =        2.40  *  m_overallVolume;   	
	}
	else if(          m_componentCount  ==   3    )   
	{
		realRatioCount =   3; 
		curCompIdx =     &(   cmpIDX3[ 0 ]   );

		briteRedux  =        0.80  *  m_overallVolume;   	 
	}

	else if(          m_componentCount  ==   4    )
	{
		realRatioCount =   6; 
		curCompIdx =     &(   cmpIDX4[ 0 ]   );

		briteRedux  =        0.40  *  m_overallVolume;    
	}
	else if(   m_componentCount  ==   5    ) 
	{
		realRatioCount =   10; 
		curCompIdx =     &(   cmpIDX5[ 0 ]   );

		briteRedux  =        0.35  *  m_overallVolume; 	 
	}
	else if(   m_componentCount  ==   6    ) 
	{
		realRatioCount =   15; 
		curCompIdx =     &(   cmpIDX6[ 0 ]   );

		briteRedux  =        0.30  *  m_overallVolume;	
	}
	else if(   m_componentCount  ==   7    ) 
	{
		realRatioCount =   21; 
		curCompIdx =     &(   cmpIDX7[ 0 ]   );

		briteRedux  =        0.25  *  m_overallVolume; 	 //   .25  Current   **** ADJUST *****   Get pretty good results with .50.    6/07
	}
	else if(   m_componentCount  ==   8    ) 
	{
		realRatioCount =   28; 
		curCompIdx =     &(   cmpIDX8[ 0 ]   );

		briteRedux  =        0.20  *  m_overallVolume;   	
	}
	else if(   m_componentCount  ==   9    ) 
	{
		realRatioCount =   36; 
		curCompIdx =     &(   cmpIDX9[ 0 ]   );

		briteRedux  =        0.18  *  m_overallVolume;  	 
	}
	else if(   m_componentCount  ==   10    ) 
	{
		realRatioCount =   45; 
		curCompIdx =     &(   cmpIDX10[ 0 ]   );

		briteRedux  =        0.16  *  m_overallVolume;   
	}
	else
	{	ASSERT( 0 );
		realRatioCount =   6; 
		curCompIdx =     &(   cmpIDX4[ 0 ]   );
	}




     short   bigest,  bigIdx,  ratSet,  numIdealRats,   bandMult= 8;
     short   bestNote,  HratLft, HratRgt,  gr, bl;
     short   fqPair, rVal, flip;
     double  realRat, idealRat, err, freq;



    switch(   ratioSetChoice   )
	{  

		case 0:    ASSERT( 0 );   //  not used
			
					numIdealRats =  NUMFluffRats;     
		break;				


        case 1:   //   ASSERT( 0 );   //  WAS not used...      How long has this been out ????    11/2011
							
					numIdealRats =  NUMBasicRats;      // Not hat good for Bonnie Rait  Clarenet song   11/2011
		break;        // MENU



        case 2:    numIdealRats =      NUMFullHarmRATS;        	// ****CURRENT***    default  ****************
		break;     



        case 3:     ASSERT( 0 );   //  not used					
					numIdealRats =  NUMFirRats;        
		break;


        default:    numIdealRats =  NUMFluffRats;        	
		break;
	}


  

     ////////////   write to  'biggest Component' (& vertical neighbors) in case 'ONLY a fundamental' harmonic exists...
      
     bigest= -1;      bigIdx= 0;				//   find BIGGEST Amplitude

     for(   i=0;    i< m_componentCount;    i++  )                   
     {  
		 if(   compAmpl[ i ]  >  bigest    )  
		 { 
			bigIdx =  i;   
			bigest =  compAmpl[i]; 
		 }  
	 }
     ///////////////////////////////////////////////////////////////////////////
 
           

    
	for(    fqPair= 0;     fqPair < realRatioCount;      fqPair++    )		  //  check all 'REAL-ratio' pairs
    {  

		short  origLeftFreq   =    compFreq[  curCompIdx[fqPair].h  ];
		short  origRightFreq =    compFreq[  curCompIdx[fqPair].v  ];

		short  origLeftAmplitude   =    compAmpl[    curCompIdx[ fqPair ].h    ];
		short  origRightAmplitude =    compAmpl[    curCompIdx[ fqPair ].v    ];
	

		if(   origLeftFreq <= 0    ||   origRightFreq  <=  0   )   //  in case  Get_Columns_Biggest_FreqComponents() could not succeed
		{
			ASSERT( 0 );   //  ist this possible??   7/06
		}
		else
		{
			realRat =   (double)(     compFreq[  curCompIdx[fqPair].h  ]  )    /    (double)( compFreq[  curCompIdx[fqPair].v  ]     );
         
			if(   realRat  <  1.0   )     //  is the  Left( .h )   frequecy the biggest,  if not
			{  
				flip       =   1;     
				realRat =   1.0 / realRat;  
			} 
			else   
				flip =  0;  				  
                                                         
																							 // check  'IDEAL-ratios'  for a 'FIT'

			for(    ratSet= 0;       ratSet <  numIdealRats;      ratSet++   )
			{   
				
				switch(   ratioSetChoice   )
				{ 
					case  0:   HratLft =   FluffRats[ ratSet ].h;            HratRgt =   FluffRats[ ratSet ].v;           break;
					case  1:   HratLft =   BasicRats[ ratSet ].h;            HratRgt =   BasicRats[ ratSet ].v;          break;
					case  2:   HratLft =   FullHarmRats[ ratSet].h;          HratRgt =   FullHarmRats[ ratSet ].v;     break;
					case  3:   HratLft =   FirRats[ ratSet ].h;              HratRgt =   FirRats[ ratSet ].v;              break;
					default:   HratLft =   FluffRats[ ratSet ].h;            HratRgt =   FluffRats[ ratSet ].v;            break;
				 }

				idealRat  =     (double)( HratLft )   /   (double)( HratRgt );  
               
                

				err  =      absj(    (realRat - idealRat)   /   idealRat    );   
            
				if(   err  <  ratioTolerance   )  
				{ 

					if(   ! flip   )						   // ...note:  freq = freq2			****IS this OK ??? 4/02 *****  try with  HratLft, is it the same ???*******  JPM
						freq = (double)( compFreq[  curCompIdx[fqPair].h  ] )  / (double)HratLft;  
					else                   // switch(flip) numerator and denominator for these ratios
						freq = (double)( compFreq[  curCompIdx[fqPair].h  ] )  / (double)HratRgt;  
                    



				   // **** COULD  filter here if the  freq is NOT in a  'realistic'  range ( or is that a good thing ??? ********* JPM 4/01
					bool      isOutOfRange =  false;

	//				if(     freq <  164.0     ||    freq >  2490.0   )      //    
					if(     freq <    82.0     ||    freq >  2490.0   )      //    freq  <  82.41    ...this gives bottom harmonic for low E string on guitar( downward read )
						 isOutOfRange =  true;
                   



					bestNote   =     Find_Freqs_ScalePitch(  (short)freq  );   //  ONLY returns  0(E) - 11(Eb),   ...'NOTES'  ( no octave reference )
					if(      bestNote ==  -99   
			//	        ||   isOutOfRange   //    ...what the hell,  it give a better display  **** BACK in  .... 6/07
					  ) 
					{  if(    bestNote ==  -99   )					
						{
							//  ASSERT( 0 );	  //  No mistake,  just too low to of meaning.  See this now that I extended
							int  dummy =   9;
						}
					}
					else
					{  short      pixValLeft,   pixValRight,     ampLeft,  ampRight;
						double    resultLeft,   resultRight;

						ampLeft    =    compAmpl[    curCompIdx[ fqPair ].h    ];  	   					 
						ampRight  =    compAmpl[    curCompIdx[ fqPair ].v    ]; 

						ASSERT(    HratLft  >=  1     &&    HratLft   <=  12    );
						ASSERT(    HratRgt >=  1     &&    HratRgt  <=  12   );





	// ********* NEED to better calc the Last Portion of the Klapuri weits( adjWeits[] )  ...I just approximated   6/2002 ********


																	//    a)   KLAPURI  calc....


//	short      templOffst[ 12 ]=  {   0,       12,       19,    24,       28,      31,    34,        36,       38,     40,     42,     43    };    // only really need seven
//   EX:  in E							     E          E         B      E          G#       B       D          E         F#     G#      A#      B




						//																					               39      61,       26,      61,     26    
			long   adjWeits[ 12 ]=  {  100,     61,     61,     39,       61,     26,     61,         39,     25,       35,      20,     15    };    // by  Anssi Klapuri  on interfering signal OVERLAP probabatilities 

			//                                     0,       1,       2,       3,         4,       5,     6,           7,       8,         9,      10,     11
						//																								...Guessing beyond 6.  ****BAD,  do the calc.  6/02
						





						long   leftWeitKlap    =    adjWeits[   ( HratLft  -1)  ];    //  -1,  so we go to the zero index of array
						long   rightWeitKlap  =    adjWeits[   ( HratRgt -1)  ];





																	//    b)    PITZCALDI   calc....

						double   amplAdjustedPitz  =      (double)(   minj( ampLeft,  ampRight )   )    
																						+    0.1  *   (double)(   maxj( ampLeft, ampRight )    );


						double   harmonicCombosWeitPitz =    1.0    -    (   0.03  *    (double)(  HratLft  +  HratRgt  -3  )    ); 




						if(   useKlapuriWeits   )
						{

						//	ASSERT( 0 );   // Not used.  Notes say this was buggy.  Try again someday


							double  weitAdjustKlap  =    0.015;    //  .010   ***ADJUST,  really just to get to a  'double's fractional value'   from  the integer Percentages ***


							double  leftWeitDbKlap    =     (double)leftWeitKlap         *   weitAdjustKlap;
							double  rightWeitDbKlap  =     (double)rightWeitKlap       *   weitAdjustKlap;


							resultLeft    =     (double)ampLeft       *   leftWeitDbKlap; 
							resultRight  =     (double)ampRight     *   rightWeitDbKlap;    
						}
						else		
						{														//   the ORIGINAL weighting that  Piszczalski recommended  
							double  weitAdjustPitz  =     1.0;    //  ***ADJUST,  just to get consistant with  Klapuri weits


							double  leftWeitDbPitz  =      harmonicCombosWeitPitz    *  weitAdjustPitz;


							resultLeft   =     amplAdjustedPitz       *   leftWeitDbPitz;
							resultRight =   resultLeft;   
						}			//  try to get the 2 different method producing weits in the same range,  so we can COMPAIR weighting strategies and values


						
						pixValLeft    =     (short)(   resultLeft      *  briteRedux    );
						pixValRight  =     (short)(   resultRight    *  briteRedux    );
			
 



					   ///////////////////////////////////		    
						short  yLeftRat,  yRightRat;

						long	yTop       =     _harmMap->Get_Channels_Top_Yval(        bestNote   );  // the bigger value cause map is inverted
						long	yBottom  =     _harmMap->Get_Channels_Bottom_Yval(   bestNote   );  
						ASSERT(  yTop  >  yBottom  );

						short   halfDataWidth  =     _harmMap->Get_DataChannel_Width()  /2;


																						//  Count the occurence of  'Harmonic-NUMBERS'....
						m_harmPopulation[   (HratLft  -1 )   ]++;			
						m_harmPopulation[   (HratRgt -1 )   ]++;		//  -1,  so we get the Indexes to the array to go down to zero



						if(     (HratLft % 2)  == 0   )    
							yLeftRat =   ( yBottom  +  halfDataWidth )    -      HratLft   /2;			 // Write  even on BOTTOM( remember DIB is inverted )
						else                       
							yLeftRat =   ( yTop        -  halfDataWidth )    +   ( HratLft +1)  /2;	    // +1,  move down to OLD center lane( Biggest component )


						if(     (HratRgt % 2)  == 0   )    
							yRightRat =   ( yBottom  +  halfDataWidth )    -    HratRgt   /2;			 // Write  even on BOTTOM( remember DIB is inverted )
						else                       
							yRightRat =   ( yTop       -  halfDataWidth )     +   ( HratRgt +1)  /2;	    // +1,  move down to OLD center lane( Biggest component )




						if(           yLeftRat   <   yBottom     ||     yLeftRat    >  yTop  
							   ||	 yRightRat  <   yBottom     ||     yRightRat  >  yTop     )
							ASSERT( 0 );     //  int  dummyBreak =  9;

						if(           yLeftRat    >=    _harmMap->m_height    ||   yLeftRat    < 0      //  make sure the map is big enough!!!					
								||   yRightRat   >=   _harmMap->m_height  	 ||   yRightRat  < 0     )
							ASSERT( 0 );




					   _harmMap->Lock_Bits();			  //  write & read to 'GWorld'.....


						_harmMap->Read_Pixel(  xWrite,   yLeftRat,     &rVal,    &gr, &bl  );  // note we use  xWRITE, caue it IS the DST HarmMap's coords
						rVal +=  pixValLeft;      

						if(  rVal  > 255 )  
						{
							rVal =   255;     
							m_overBriteCnt++;
						}
						else if(  rVal < 0 )    
							rVal =  0;  

						_harmMap->Write_Pixel(  xWrite,   yLeftRat,     rVal, rVal, rVal   );



						_harmMap->Read_Pixel(  xWrite,   yRightRat,     &rVal,    &gr, &bl  );  // note we use  xWRITE, caue it IS the DST HarmMap's coords
						rVal +=  pixValRight;    
						
						if(  rVal  > 255 )  
						{
							rVal =   255;     
							m_overBriteCnt++;
						}
						else if(  rVal < 0 )    
							rVal =  0;  

					   _harmMap->Write_Pixel(  xWrite,   yRightRat,     rVal, rVal, rVal   );
  						

						_harmMap->Unlock_Bits();                        // ...restore
					}					

				}   // if( err <          
            
			}     // for(  ratSet =

		}   //   if(   origLeftFreq <= 0    ||   origRightFreq  <=  0   )

	}    // for(  fqPair =
}




												////////////////////////////////////////


short   HarmPairsVerter::Get_Columns_OctavePick(   long  xLead,    short  targSPitchVal,    long  retOctvCandidsScores[]   )  
{  



	bool      useKlapuriWeits =    false;   //   **** KlapuriWeits BUGGY ???(  Not too bad in 7/06, BUT since Piszczalski  was so
											//   brillant in the invention and execution of this algo, it makes sense to trust his judgement.



	double   ratioTolerance =   0.025;     //  0.015    *****  ADJUST different values ???   .30 did not look bad ???  ******


	bool     useDFTmask     =   false;   


	double   briteRedux;           //  0.40   ****ADJUST (below in switch()  BRITENESS,  affects BOTH weiting systems equally



	short      ratioSetChoice  =   2;     //  2   { 2 or 1  } is good    ********* TEMP,  set with MENU   ********************
			/***
					case 0:   numIdealRats =    NUMFluffRats;           break;     //  only  8 harmonics,  ALL combos  
					case 1:   numIdealRats =    NUMBasicRats;          break;     
					case 2:   numIdealRats =    NUMFullHarmRATS;    break;    //         12 harmonics,  ALL combos 
					case 3:   numIdealRats =    NUMFirRats;              break;
			***/



	//   m_filterCodeDFT,    m_kernalWidthDFT   are linked to the GLOBAL setting for DFT Filtered-READS
	//																(  from  DerivitiveMapsCreateParms  )



	short  retBestOctave;  
	short  fundCandidCount  =        FundamentalCandidate::Get_Octave_Candidate_Count();  



	for(    short  oct = 0;    oct <  fundCandidCount;     oct++   )
		retOctvCandidsScores[ oct ]  =    -1;    //  init results for fail





	///////////////////////////
	long   xRead= -1;
	TransformMap   *sourceMap =    &_DFTmap;


	if(        sourceMap  !=   NULL 
		&&     sourceMap->m_inFloatmapMode    )
	{
	    ASSERT(   sourceMap  ==   &_DFTmap   );       //   from above

		xRead  =    sourceMap->m_width  / 2;         //   the  CENTER Column  in the FloatMap ( log DFT )
	}
	else
		xRead  =    xLead;	
	///////////////////////////




	short   i,   compFreq[ 30 ],  compAmpl[ 30 ];    

	for(   i= 0;     i < 30;     i++   )
	{
		compAmpl[ i ] =   -1;
		compFreq[ i ] =   -1;
	}



//	long	   hiResRatioLogDFT  =    1;      // ****  NEW  9/2012 ,  can make more HiRes when using CircQueLogDFT algo,  just read thye Que at more Tap-Points

	ASSERT(   m_hiResRatioLogDFT  ==   1    );    //  LOOKS like this is for OLD PitchScope and does NOT get called in Navigator
                                           



	_DFTmap.Get_Columns_Biggest_FreqComponents(    xRead,     compFreq,  compAmpl,      m_componentCount, 
														 useDFTmask,	   m_filterCodeDFT,    m_kernalWidthDFT   ); 



	myPair   *curCompIdx =  NULL;

	
	myPair  cmpIDX4[ 6 ] =   {     { 0, 1 },   { 0, 2 },   { 0, 3 },   
																{ 1, 2 },   { 1, 3 },   
																				{ 2, 3 }  	     };    // Is this all   'Pair-Combinations'  for a 4 number set  [ 0, 1, 2, 3  ]  ?



	 myPair  cmpIDX5[ 10 ] =   {   { 0, 1 },   { 0, 2 },     { 0, 3 },    { 0, 4 }, 
																{ 1, 2 },     { 1, 3 },     { 1, 4 },   
																				{ 2, 3 },     { 2, 4 },  
																								{ 3, 4 }		};
	 
	 

	 myPair  cmpIDX6[ 15 ] =   {   { 0, 1 },   { 0, 2 },   { 0, 3 },    { 0, 4 },     { 0, 5 }, 
																{ 1, 2 },   { 1, 3 },    { 1, 4 },      { 1, 5 }, 
																				{ 2, 3 },    { 2, 4 },    { 2, 5 }, 
																								{ 3, 4 },    { 3, 5 },
																												{ 4, 5 }   
																															};



	 myPair  cmpIDX7[ 21 ] =   {   { 0, 1 },   { 0, 2 },   { 0, 3 },    { 0, 4 },     { 0, 5 },    { 0, 6 }, 
																{ 1, 2 },   { 1, 3 },     { 1, 4 },    { 1, 5 },    { 1, 6 },
																				{ 2, 3 },    { 2, 4 },    { 2, 5 },    { 2, 6 }, 
																								{ 3, 4 },    { 3, 5 },    { 3, 6 },
																												{ 4, 5 },     { 4, 6 },
																																{ 5, 6 }  
																																				};


	 myPair  cmpIDX8[ 28 ] =   {   { 0, 1 },   { 0, 2 },   { 0, 3 },    { 0, 4 },     { 0, 5 },    { 0, 6 },    { 0, 7 }, 
																{ 1, 2 },   { 1, 3 },     { 1, 4 },    { 1, 5 },    { 1, 6 },    { 1, 7 },
																				{ 2, 3 },    { 2, 4 },    { 2, 5 },    { 2, 6 },    { 2, 7 }, 
																								{ 3, 4 },    { 3, 5 },    { 3, 6 },    { 3, 7 },
																												{ 4, 5 },     { 4, 6 },    { 4, 7 },
																																{ 5, 6 },     { 5, 7 },  
																																				{ 6, 7 }  
																																		 };


	 myPair  cmpIDX9[ 36 ] =   {   { 0, 1 },   { 0, 2 },   { 0, 3 },    { 0, 4 },     { 0, 5 },    { 0, 6 },    { 0, 7 },     { 0, 8 }, 
																{ 1, 2 },   { 1, 3 },     { 1, 4 },    { 1, 5 },    { 1, 6 },    { 1, 7 },    { 1, 8 },
																				{ 2, 3 },    { 2, 4 },    { 2, 5 },    { 2, 6 },    { 2, 7 },    { 2, 8 }, 
																								{ 3, 4 },    { 3, 5 },    { 3, 6 },    { 3, 7 },    { 3, 8 },
																												{ 4, 5 },     { 4, 6 },    { 4, 7 },    { 4, 8 },
																																{ 5, 6 },     { 5, 7 },     { 5, 8 },    
																																				{ 6, 7 },      { 6, 8 },   
																																									{ 7, 8 }
																																													};


	 myPair  cmpIDX10[ 45 ] = {   { 0, 1 },   { 0, 2 },   { 0, 3 },    { 0, 4 },     { 0, 5 },    { 0, 6 },    { 0, 7 },     { 0, 8 },      { 0, 9 }, 
																{ 1, 2 },   { 1, 3 },     { 1, 4 },    { 1, 5 },    { 1, 6 },    { 1, 7 },    { 1, 8 },       { 1, 9 },
																				{ 2, 3 },    { 2, 4 },    { 2, 5 },    { 2, 6 },    { 2, 7 },    { 2, 8 },       { 2, 9 }, 
																								{ 3, 4 },    { 3, 5 },    { 3, 6 },    { 3, 7 },    { 3, 8 },       { 3, 9 },
																												{ 4, 5 },     { 4, 6 },    { 4, 7 },    { 4, 8 },      { 4, 9 },
																																{ 5, 6 },     { 5, 7 },     { 5, 8 },     { 5, 9 },      
																																				{ 6, 7 },      { 6, 8 },     { 6, 9 },   
																																									{ 7, 8 },   { 7, 9 },
																																													{ 8, 9 }
																																													};


	 myPair  cmpIDX11[ 55 ] = 
	 {   { 0, 1 },   { 0, 2 },   { 0, 3 },    { 0, 4 },     { 0, 5 },    { 0, 6 },    { 0, 7 },     { 0, 8 },    { 0, 9 },    { 0, 10 },  
						{ 1, 2 },   { 1, 3 },     { 1, 4 },    { 1, 5 },    { 1, 6 },    { 1, 7 },    { 1, 8 },     { 1, 9 },	  { 1, 10 },
										{ 2, 3 },    { 2, 4 },    { 2, 5 },    { 2, 6 },    { 2, 7 },    { 2, 8 },     { 2, 9 },     { 2, 10 }, 
														{ 3, 4 },    { 3, 5 },    { 3, 6 },    { 3, 7 },    { 3, 8 },     { 3, 9 },     { 3, 10 },
																		{ 4, 5 },    { 4, 6 },    { 4, 7 },    { 4, 8 },      { 4, 9 },    { 4, 10 },
																						{ 5, 6 },    { 5, 7 },     { 5, 8 },     { 5, 9 },    { 5, 10 },      
																										{ 6, 7 },    { 6, 8 },      { 6, 9 },    { 6, 10},    
																														{ 7, 8 },      { 7, 9 },     { 6, 10 },
																																			{ 8, 9 },   { 7, 10 },
																																							{ 9, 10 } 								 	 
																																											};


	 myPair  cmpIDX12[ 66 ] = 
	 {   { 0, 1 },   { 0, 2 },   { 0, 3 },    { 0, 4 },     { 0, 5 },    { 0, 6 },    { 0, 7 },     { 0, 8 },    { 0, 9 },    { 0, 10 },     { 0, 11 },  
						{ 1, 2 },   { 1, 3 },     { 1, 4 },    { 1, 5 },    { 1, 6 },    { 1, 7 },    { 1, 8 },     { 1, 9 },	  { 1, 10 },	 { 1, 11 },
										{ 2, 3 },    { 2, 4 },    { 2, 5 },    { 2, 6 },    { 2, 7 },    { 2, 8 },     { 2, 9 },     { 2, 10 },     { 2, 11 }, 
														{ 3, 4 },    { 3, 5 },    { 3, 6 },    { 3, 7 },    { 3, 8 },     { 3, 9 },     { 3, 10 },     { 3, 11 },
																		{ 4, 5 },    { 4, 6 },    { 4, 7 },    { 4, 8 },      { 4, 9 },    { 4, 10 },     { 4, 11 },
																						{ 5, 6 },    { 5, 7 },     { 5, 8 },     { 5, 9 },    { 5, 10 },     { 5, 11 },      
																										{ 6, 7 },    { 6, 8 },      { 6, 9 },    { 6, 10},      { 6, 11 },    
																														{ 7, 8 },      { 7, 9 },     { 7, 10 },    { 7, 11 },
																																			{ 8, 9 },   { 8, 10 },     { 8, 11 },
																																							{ 9, 10 }, 	  { 9, 11 },			
																																											{ 10, 11 }	
																																									};



	 myPair  cmpIDX13[ 78 ] = 
	 {   { 0, 1 },   { 0, 2 },   { 0, 3 },    { 0, 4 },     { 0, 5 },    { 0, 6 },    { 0, 7 },     { 0, 8 },    { 0, 9 },    { 0, 10 },     { 0, 11 },     { 0, 12 },  
						{ 1, 2 },   { 1, 3 },     { 1, 4 },    { 1, 5 },    { 1, 6 },    { 1, 7 },    { 1, 8 },     { 1, 9 },	  { 1, 10 },	 { 1, 11 },	    { 1, 12 },
										{ 2, 3 },    { 2, 4 },    { 2, 5 },    { 2, 6 },    { 2, 7 },    { 2, 8 },     { 2, 9 },     { 2, 10 },     { 2, 11 },     { 2, 12 }, 
														{ 3, 4 },    { 3, 5 },    { 3, 6 },    { 3, 7 },    { 3, 8 },     { 3, 9 },     { 3, 10 },     { 3, 11 },     { 3, 12 },
																		{ 4, 5 },    { 4, 6 },    { 4, 7 },    { 4, 8 },      { 4, 9 },    { 4, 10 },     { 4, 11 },     { 4, 12 },
																						{ 5, 6 },    { 5, 7 },     { 5, 8 },     { 5, 9 },    { 5, 10 },     { 5, 11 },     { 5, 12 },       
																										{ 6, 7 },    { 6, 8 },      { 6, 9 },    { 6, 10},      { 6, 11 },     { 6, 12 },   
																														{ 7, 8 },      { 7, 9 },     { 7, 10 },    { 7, 11 },     { 7, 12 },
																																			{ 8, 9 },   { 8, 10 },     { 8, 11 },    { 8, 12 },
																																							{ 9, 10 }, 	  { 9, 11 }, 	{ 9, 12 },			
																																											{ 10, 11 },	  { 10, 12 },
																																																{ 11, 12 }
																																												};





	short  realRatioCount;      // ****** can ADJUST,  [ 4 - 13  ]  **********************************



	if(          m_componentCount  ==   4    )
	{
		realRatioCount =   6; 
		curCompIdx =     &(   cmpIDX4[ 0 ]   );

		briteRedux  =        0.40;   	 //      **** ADJUST ***** 
	}
	else if(   m_componentCount  ==   5    ) 
	{
		realRatioCount =   10; 
		curCompIdx =     &(   cmpIDX5[ 0 ]   );

		briteRedux  =        0.35;   	 //      **** ADJUST ***** 
	}
	else if(   m_componentCount  ==   6    ) 
	{
		realRatioCount =   15; 
		curCompIdx =     &(   cmpIDX6[ 0 ]   );

		briteRedux  =        0.30;   	 //      **** ADJUST ***** 
	}
	else if(   m_componentCount  ==   7    ) 
	{
		realRatioCount =   21; 
		curCompIdx =     &(   cmpIDX7[ 0 ]   );

		briteRedux  =        0.25;   	 //      **** ADJUST ***** 
	}
	else if(   m_componentCount  ==   8    ) 
	{
		realRatioCount =   28; 
		curCompIdx =     &(   cmpIDX8[ 0 ]   );

		briteRedux  =        0.20;   	 //      **** ADJUST ***** 
	}
	else if(   m_componentCount  ==   9    ) 
	{
		realRatioCount =   36; 
		curCompIdx =     &(   cmpIDX9[ 0 ]   );

		briteRedux  =        0.18;   	 //      **** ADJUST ***** 
	}
	else if(   m_componentCount  ==   10    ) 
	{
		realRatioCount =   45; 
		curCompIdx =     &(   cmpIDX10[ 0 ]   );

		briteRedux  =        0.16;   	 //     **** ADJUST ***** 
	}
	else if(   m_componentCount  ==   11    ) 
	{
		realRatioCount =   55; 
		curCompIdx =     &(   cmpIDX11[ 0 ]   );

		briteRedux  =        0.15;   	 //     **** ADJUST ***** 
	}
	else if(   m_componentCount  ==   12    ) 
	{
		realRatioCount =   66; 
		curCompIdx =     &(   cmpIDX12[ 0 ]   );

		briteRedux  =        0.14;   	 //     **** ADJUST ***** 
	}
	else if(   m_componentCount  ==   13    ) 
	{
		realRatioCount =   78; 
		curCompIdx =     &(   cmpIDX13[ 0 ]   );

		briteRedux  =        0.13;   	 //     **** ADJUST ***** 
	}
	else
	{	ASSERT( 0 );

		realRatioCount =   6; 
		curCompIdx =     &(   cmpIDX4[ 0 ]   );
	}





     short   bigest,  bigIdx,  ratSet,  numIdealRats,   bandMult= 8;
     short   HratLft, HratRgt;
     short   fqPair, flip;
     double  realRat, idealRat, err, freq;
	 short   successCount=0,    missedCount=0,    bestScalePitch;   //  init counters




	ASSERT(  fundCandidCount ==  4   );   //  if not will have a big problem below with small array(  sumOfResultsInOctaves[ 4 ]  )

	DoubleIndexed   sumOfResultsInOctaves[ 4 ];      //  for each octave, we will sum the results for all of the RatioPairs
								
	

	for(    i =0;     i < fundCandidCount;    i++   )
	{
		sumOfResultsInOctaves[ i ].index  =    i;   //  hold index for a later bubble sort

		sumOfResultsInOctaves[ i ].value  =    0.0;     //  init the sums
	}



    switch(   ratioSetChoice   )
	{  

		case 0:    ASSERT( 0 );   //  not used
			
				   numIdealRats =  NUMFluffRats;     
		//			  briteRedux  =    0.30;     ***********   Now up above
		break;				


        case 1:    ASSERT( 0 );   //  not used
							
				  numIdealRats =  NUMBasicRats;   
		//			  briteRedux  =    0.45;   			
		break;        // MENU




        case 2:    numIdealRats =      NUMFullHarmRATS;        	// ****CURRENT***    default  ****************

			//		  briteRedux  =        0.30;   	 
		break;     



        case 3:   ASSERT( 0 );   //  not used
					
					numIdealRats =  NUMFirRats;        
		//			  briteRedux  =    0.30;   			
		break;


        default:    numIdealRats =  NUMFluffRats;        	break;
	}


  

     //////////// write to  'biggest Component' (& vertical neighbors) in case 'ONLY a fundamental'...
      
     bigest= -1;      bigIdx= 0;				//   find BIGGEST Amplitude

     for(   i=0;    i< m_componentCount;    i++  )                   
     {  
		 if(   compAmpl[ i ]  >  bigest    )  
		 { 
			bigIdx =  i;   
			bigest =  compAmpl[i]; 
		 }  
	 }
     ///////////////////////////////////////////////////////////////////////////
 
           


    
	for(     fqPair= 0;       fqPair < realRatioCount;       fqPair++     )		  //  check all 'REAL-ratio' pairs
    {  


		short  origLeftFreq   =    compFreq[  curCompIdx[fqPair].h  ];
		short  origRightFreq =    compFreq[  curCompIdx[fqPair].v  ];

		short  origLeftAmplitude   =    compAmpl[    curCompIdx[ fqPair ].h    ];
		short  origRightAmplitude =    compAmpl[    curCompIdx[ fqPair ].v    ];
	

		if(   origLeftFreq <= 0    ||   origRightFreq  <=  0   )   //  in case  Get_Columns_Biggest_FreqComponents() could not succeed
		{
			ASSERT( 0 );   //  ist this possible??   7/06
		}
		else
		{
			realRat =   (double)(     compFreq[  curCompIdx[fqPair].h  ]  )    /    (double)( compFreq[  curCompIdx[fqPair].v  ]     );
         
			if(   realRat  <  1.0   )     //  is the  Left( .h )   frequecy the biggest,  if not
			{  
				flip       =   1;     
				realRat =   1.0 / realRat;  
			} 
			else   
				flip =  0;  				  
                                                         
																							 // check  'IDEAL-ratios'  for a 'FIT'

			for(    ratSet= 0;       ratSet <  numIdealRats;      ratSet++   )
			{   
				
				switch(   ratioSetChoice   )
				{ 
					case  0:   HratLft =   FluffRats[ ratSet ].h;            HratRgt =   FluffRats[ ratSet ].v;           break;
					case  1:   HratLft =   BasicRats[ ratSet ].h;            HratRgt =   BasicRats[ ratSet ].v;          break;
					case  2:   HratLft =   FullHarmRats[ ratSet].h;          HratRgt =   FullHarmRats[ ratSet ].v;     break;
					case  3:   HratLft =   FirRats[ ratSet ].h;              HratRgt =   FirRats[ ratSet ].v;              break;
					default:   HratLft =   FluffRats[ ratSet ].h;            HratRgt =   FluffRats[ ratSet ].v;            break;
				 }

				idealRat  =     (double)( HratLft )   /   (double)( HratRgt );  
               
                

				err  =      absj(    (realRat - idealRat)   /   idealRat    );   
            
				if(   err  <  ratioTolerance   )  
				{ 

					if(   !flip   )						   // ...note:  freq = freq2			****IS this OK ??? 4/02 *****  try with  HratLft, is it the same ???*******  JPM
						freq = (double)( compFreq[  curCompIdx[fqPair].h  ] )  / (double)HratLft;  
					else                   // switch(flip) numerator and denominator for these ratios
						freq = (double)( compFreq[  curCompIdx[fqPair].h  ] )  / (double)HratRgt;  
                    



				                              // **** COULD  filter here if the  freq is NOT in a  'realistic'  range ( or is that a good thing ??? ********* JPM 4/2001
					bool      isOutOfRange =  false;

					if(     freq <  164.0     ||    freq >  2490.0   )
						 isOutOfRange =  true;
                   


					bestScalePitch    =     Find_Freqs_ScalePitch(  (short)freq  );   //  ONLY returns  0(E) - 11(Eb),   ...'NOTES'  ( no octave reference )
					if(    bestScalePitch ==  -99   )  
					{
						int   dummy =  9;
					}



					short   octPick        =    FundamentalCandidate::Get_Octave_Index(  freq  );     

					// **************************************************					
					short   octPickAdjst   =    octPick  +1;   //  It works( 7/2006)!!!   Is it the   logDFTdOWNWARDrEAD   effect ?????   
											                   //  Seems like any read from the DFT  that  'implies a FundamentalPitch'  needs this octave correction.

					// **************************************************


					if(       octPickAdjst   <    0    
						||   octPickAdjst  >=   4
						||   bestScalePitch ==  -99   )
					{  
						int dummy =  9;  // Want to count any veryLow to Oct0,  and  veryHigh to Oct3  ????? 
					}
					else
					{  short      ampLeft,  ampRight;
						double    resultLeft,   resultRight;

						ampLeft   =    compAmpl[    curCompIdx[ fqPair ].h    ];  	   					 
						ampRight =    compAmpl[    curCompIdx[ fqPair ].v    ]; 

						ASSERT(    HratLft  >=  1     &&    HratLft   <=  12    );
						ASSERT(    HratRgt >=  1     &&    HratRgt  <=  12   );



						if(   useKlapuriWeits   )
						{

						            // *** NEED to better calc the Last Portion of the Klapuri weits( adjWeits[] )  ...I just approximated   6/2002 ****


																		//    a)   KLAPURI  calc....


														//	      0,       12,       19,    24,       28,      31,    34,        36,       38,     40,     42,     43    };    // only really need seven
														//       E          E         B       E          G#       B       D          E         F#     G#      A#      B


										//																					               39      61,       26,      61,     26    
							long   adjWeits[ 12 ]=  {  100,     61,     61,     39,       61,     26,     61,         39,     25,       35,      20,     15    };    // by  Anssi Klapuri  on interfering signal OVERLAP probabatilities 

							//                                     0,       1,       2,       3,         4,       5,     6,           7,       8,         9,      10,     11
										//																								...Guessing beyond 6.  ****BAD,  do the calc.  6/02
										

							long   leftWeitKlap    =    adjWeits[   ( HratLft  -1)  ];    //  -1,  so we go to the zero index of array
							long   rightWeitKlap  =    adjWeits[   ( HratRgt -1)  ];



							double  weitAdjustKlap  =    0.015;    //  .010   ***ADJUST,  really just to get to a  'double's fractional value'   from  the integer Percentages ***


							double  leftWeitDbKlap    =     (double)leftWeitKlap         *   weitAdjustKlap;
							double  rightWeitDbKlap  =     (double)rightWeitKlap       *   weitAdjustKlap;


							resultLeft    =     (double)ampLeft       *   leftWeitDbKlap; 
							resultRight  =     (double)ampRight     *   rightWeitDbKlap;    
						}
						else		
						{	 //   b)    Piszczalski's  calc....     the ORIGINAL weighting that  Piszczalski recomended  


							double   amplAdjustedPitz  =      (double)(   minj( ampLeft,  ampRight )   )    
																							+    0.1  *   (double)(   maxj( ampLeft, ampRight )    );

							double   harmonicCombosWeitPitz =    1.0    -    (   0.03  *    (double)(  HratLft  +  HratRgt  -3  )    ); 


							double  weitAdjustPitz  =     1.0;    //  ***  ADJUST,  just to get consistant with  Klapuri weits


							double  leftWeitDbPitz  =      harmonicCombosWeitPitz    *  weitAdjustPitz;


							resultLeft  =     amplAdjustedPitz       *   leftWeitDbPitz;

						//  resultRight =     resultLeft;   
						}			//  try to get the 2 different method producing weits in the same range,  so we can COMPAIR weighting strategies and values



						if(    bestScalePitch  ==    targSPitchVal   )
						{	
							sumOfResultsInOctaves[  octPickAdjst  ].value   +=     resultLeft;      	//  Need to sum values 
							successCount++;
						}
						else
							missedCount++;

					}					

				}   // if( err <          
            
			}     // for(  ratSet =

		}   //   if(   origLeftFreq <= 0    ||   origRightFreq  <=  0   )

	}    // for(  fqPair =





	for(      i = 0;    i <  fundCandidCount;     i++   )
	{	
		ASSERT(   i ==   sumOfResultsInOctaves[ i ].index   );      //  should be in the righ order at this point

		retOctvCandidsScores[  sumOfResultsInOctaves[ i ].index  ]  =    (long)(   sumOfResultsInOctaves[ i ].value   );    
	}





	short      j,  tpIndex;			//  sort the   'DoubleIndexed'    struct
	double   tpVal;
			
	for(   i= 0;    i <  fundCandidCount -1;    ++i    )            // sort in order( BUBBLE SORT ) in descending order of Value
	{ 

		for(    j=  fundCandidCount -1;     i< j;     --j    )   
		{  

			if(    sumOfResultsInOctaves[ j-1 ].value    <    sumOfResultsInOctaves[ j ].value     )
            { 
						//  Swap  DATA{ value and index )  so the newly arranged array is in descending order

				tpVal                                               =     sumOfResultsInOctaves[  j- 1 ].value; 
                sumOfResultsInOctaves[ j -1 ].value  =     sumOfResultsInOctaves[   j    ].value;
                sumOfResultsInOctaves[ j ].value      =     tpVal;


				tpIndex                                           =    sumOfResultsInOctaves[  j- 1 ].index; 
                sumOfResultsInOctaves[ j -1 ].index  =    sumOfResultsInOctaves[   j    ].index;
                sumOfResultsInOctaves[ j ].index      =    tpIndex;
             }
		} 
	}

																
	        retBestOctave  =     sumOfResultsInOctaves[  0  ].index;
	return  retBestOctave;
}





////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


DFTedgeVerter::DFTedgeVerter(   logDFTtrForm&  freqMap,     TransformMap&  edgeMap    )  
																					:   m_DFTmap( freqMap ),     m_edgeMap( edgeMap )
{ 
}


												////////////////////////////////////////


void   DFTedgeVerter::Transform()
 {

     long   x;   
		 

//	m_overBriteCnt =  0L;


	long   sourceTransWidth;

	if(     Needs_Offsetting()     )
		sourceTransWidth  =     m_edgeMap.m_width;
	else
		sourceTransWidth  =     m_DFTmap.m_width;



	 m_edgeMap.Clear(  0   ) ;     //  0 black




	long  skipColumns =  2;   // because our kernal if 5 wide, do not try to read where there is nothing


     for(    x = skipColumns;     x <  ( sourceTransWidth  -   skipColumns  );      x++   )    
	 {
		 Transform_Column(  x  );    
	 }
      


//	 TRACE(    "\n\nEdgeDetect   transform is  DONE [  Overbrite Count:  %d   ]  \n",    m_overBriteCnt    );
 }      




												////////////////////////////////////////


void   DFTedgeVerter::Transform_Column(    long  xLead   )  
{  



	double  briteIncrease =   5.0;    //  10.0        **** ADJUST *****


	double  upperSpectrumPercent  =    0.30;    //  3  5   **** ADJUST *****


	//   brite[ percent ] :     6.0[ 100% ],    10.0[ 30% ],   


	long     stopBand  =       (long)(    upperSpectrumPercent   *   (double)( m_DFTmap.m_height -1 )    );
				
	


	///////////////////////////
	long   xRead= -1,    xWrite=-1;

	TransformMap&    destinationMap =    m_edgeMap;
	TransformMap&    sourceMap       =    m_DFTmap;


	if(     destinationMap.m_inFloatmapMode     )  
		xWrite =   destinationMap.m_width  -1;       //  ALWAYS writing to the  LAST COLUMN,    for a FloatingMap
	else
		xWrite =   xLead;			//   NORMAL,  NON-Floaing mode
		

	if(    sourceMap.m_inFloatmapMode    )
	{
		xRead  =    sourceMap.m_width  / 2;      //   the  CENTER Column  in the FloatMap ( log DFT )
	}
	else
	    xRead  =    xLead;	
	///////////////////////////




	long    sumAllRowsDiffs =   0;
	short   pixVal,  yCount=0;


	for(    long y= 0;      y <  stopBand;      y++   )      //   kTOTALdftBANDs  =   76  rows
	{

		// Remember map is inverted,  so y=0 is the highest frequency.  We will just check highest freq for a drum sound  7/06

		short   val, gr, bl;
		long    thisKernal  =   0;


		sourceMap.Read_Pixel(   xRead -2,     y,     &val,  &gr, &bl   );       //   LEFT  side of edge kernal 
		thisKernal  -=   val;		//    '-'   the left side of edge kernal   

		sourceMap.Read_Pixel(   xRead -1,     y,     &val,  &gr, &bl   );    
		thisKernal  -=   val;		//    '-'   the left side of edge kernal   


		sourceMap.Read_Pixel(   xRead +1,    y,     &val,  &gr, &bl   );       //   RIGHT  side of edge kernal 
		thisKernal  +=   val;		//    '+'   the right side of edge kernal   

		sourceMap.Read_Pixel(   xRead +2,    y,     &val,  &gr, &bl   );    
		thisKernal  +=   val;		//    '+'   the right side of edge kernal   





//  Will this create a problem if negative cancel out positives ???   experiment.


		sumAllRowsDiffs  +=    thisKernal;                 //  Works best?     (  but  negative might cancel out,  look for biggest chnge
//		sumAllRowsDiffs  +=    absj(   thisKernal  );   //  not so good???


		yCount++;
	}



	pixVal  =     (short)(      ((double)(  sumAllRowsDiffs)  *  briteIncrease  )    /    (double)yCount     );

	if(   pixVal  >  255  )
		pixVal =   255;
	
	if(   pixVal  <  0  )
		pixVal =   0;


	destinationMap.Write_Pixel(   xWrite,  0,     pixVal,  pixVal,  pixVal   );  
}






