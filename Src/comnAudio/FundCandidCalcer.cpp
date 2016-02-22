/////////////////////////////////////////////////////////////////////////////
//
//  FundCandidCalcer.cpp   -   calulates the Octave for a ScalePitch by examining all the possible Harmonics for all of the 4 Octave Candidates
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

#include  <math.h>

#include  "Mmsystem.h"     //  for MIDI  and  SoundMan





#include  "..\comnFacade\VoxAppsGlobals.h"

//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"      //  #include <FixMath.h>    ...Mac's version of Fixedpoint 
#include  "..\ComnGrafix\CommonGrafix.h"      //  #include "commonmy.h"   		

#include  "..\ComnGrafix\OffMap.h"  
#include  "..\comnGrafix\TransformMap.h"

#include "..\comnInterface\DetectorAbstracts.h"    // **** NEW ****


#include  "..\comnGrafix\mapFilters.h"



#include  "..\comnAudio\FundamentalCandidate.h"

//////////////////////////////////////////////////     




///////////////////////////
#include  "..\ComnAudio\sndSample.h"

#include  "..\comnAudio\DFTtransforms.h"


///////////////////////////
#include  "..\comnAudio\sndFilters.h"


#include  "..\comnAudio\BitSourceAudio.h"
//////////////////////////





#include "FundCandidCalcer.h"

//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////


extern    OctaveCorrectParms      lastOctaveCorrectParmsGLB;  



//SoundMan&     Get_SoundMan();





////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


FundCandidCalcer::FundCandidCalcer(  short  candidCount,   bool sameSPitchMode,  
																   long  startSample,   long  endSample,
										                            bool  dynamicAllocFCandids,   short sPitchVal,   
																//	short  stereoChannel,  
													  logDFTtrForm&  logDFT,   
												//	  FundamentalTemplateTrForm&  fundamentalMap,
													  long  topFrequencyLimit,    long  bottomFrequencyLimit   
											//		     ,  ScalepitchSubject& sPitchSubj,   DetectZone&   detectZone   
													  )

								:     m_fundCandidCount( candidCount  ),   
								    //  m_stereoChannelCode(  stereoChannel  ),
								      m_startSample( startSample ),   m_endSample( endSample ),   m_dynamicAllocFCandids( dynamicAllocFCandids  ),
									  m_logDFT(   logDFT  ),   
								//	  m_fundamentalMap(  fundamentalMap  ),
									  m_topFrequencyLimit(  topFrequencyLimit ),    m_bottomFrequencyLimit(  bottomFrequencyLimit )
								//	  ,   m_scalePitchSubj(  sPitchSubj  ),   m_detectZone(  detectZone  )
{

	//  the Octave is calculated by examining all the possible Harmonics for all of the 4 Octave Candidates


		//  ****  CALLING FUNCTION must also initialize each FundCandidate's   'm_avgScoreGapped'  ( ultimately from FundamentalMap )




//	m_doNotDoAutoCorrelation =  true;    //    6/07,  do NOT do ACorrelation,  unless for the ScalePitch Details dialog
	


//	bool       useFFTharmonicFiltering  =    true;   //     ***** SWITCH ****  It looks like the FFT filtering has not really made a significant difference.  ********************



//	m_useWeitsNeighborhoodStrength =   true;   // ********************   TEMP TEST...  is working real good.  7/3/07  *************************************


	CString   retErrorMesg;




	short   appCode =    Get_PitchScope_App_Code_GLB();          //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope


	if(       appCode  == 3
		||   appCode  ==  2    )        //  Is ONLY   for   NAVIGATOR/PLAYER    9/2012
	{
		ASSERT( 0 );
	}






									//  How much initialize is necessary???

	m_bestFinalScore = -1;
	m_chosenFundamentalCandIdx  =  -1;
//	m_sndSample =  NULL; 

	m_chosenFundamentalsAvgHarmonicMag =  -1;


//	m_certaintyFirst   =   m_certaintySecond   =    -1;   // inpercent.  Is the difference between Best and 2nd best,  and  Best and 3rdBest


	short  totalCandidCount =  -1;

	if(   candidCount  ==  FUNDAMENTALTEMPLATEoCTAVECOUNT   )
	{
		m_subOctaveCount =    Get_SubOctave_Count();    //  3;
		totalCandidCount    =    Get_Total_FundCandids_In_Array();
	}
	else
	{	ASSERT( 0 );   //   Write and install
	}





	if(    dynamicAllocFCandids   )
	{

		ASSERT(  totalCandidCount > 0  );

		m_fundCandids     =       new   FundamentalCandidate[  totalCandidCount  ];
		if(  m_fundCandids == NULL )        // ***** NOTE:  Special syntax(   []    )   for the 'delete' on this ARRAY  ******
		{
			ASSERT( 0  );
			AfxMessageBox(  "FundCandidCalcer::FundCandidCalcer failed, could not allocate arrays."  );
		}
	}
	else
	{  ASSERT( 0 );  //  I may never use Fixed Arrays
	}




	/*********************  NEED this ??????

	for(    short  octave =0;        octave<  totalCandidCount;       octave++    )    
	{
		m_fundCandids[ octave ].m_scoreDetectionAvgToneFC  =     m_scalePitchSubj.m_scoreDetectionAvgTone;
	}

	m_fundCandidsFive.m_scoreDetectionAvgToneFC  =     m_scalePitchSubj.m_scoreDetectionAvgTone;
	****/




	short  travMidi;  


	if(   sameSPitchMode   )    //  is true
	{



// *********************************  PROBLEM here    1/2010  ????	  May be OK (11/11).  	


//		short  fundamentalMapsFirstMidi  =   FundamentalTemplateTrForm::Get_FundTemplMaps_FirstRows_MidiNumber();
		short  fundamentalMapsFirstMidi  =    kMIDIsTARTnOTE;    //   52

		m_firstMidiFundTemplate  =   fundamentalMapsFirstMidi;   //    kMIDIsTARTnOTE    52
      



		travMidi  =       fundamentalMapsFirstMidi   -   (  m_subOctaveCount * 12 );   //  start out at lowest SUBoctave



		for(   short  octave =0;        octave<  totalCandidCount;       octave++    )     
		{

			m_fundCandids[ octave ].m_octaveIdx =      octave  -  m_subOctaveCount;  // ****** CAREFUL,  now is the 
																			//  FundamentalMap's  Octave index,  not the same as the array
																			//  Negative numbers tell what SUBoctave.


			m_fundCandids[ octave ].m_midiPitch   =	     travMidi  +  sPitchVal;

//			m_fundCandids[ octave ].m_vetoScore =   0;


			if(    octave  < m_subOctaveCount  )
				m_fundCandids[ octave ].m_isAsubOctave =    true;
			else
				m_fundCandids[ octave ].m_isAsubOctave =    false;


			travMidi  +=   12;    //  increment by and octave
		}
	}
	else
	{                         //  Must intialize for each midipitch of the 

		ASSERT( 0 );   //  Write and INSTALL  ****
	}



	m_fundCandidsFive.m_octaveIdx     =     totalCandidCount  -  m_subOctaveCount;

	m_fundCandidsFive.m_midiPitch     =	    travMidi  +  sPitchVal;

	m_fundCandidsFive.m_isAsubOctave  =     false;

}



											////////////////////////////////////////


FundCandidCalcer::~FundCandidCalcer()
{

	if(    m_dynamicAllocFCandids    )
	{
		if(    m_fundCandids  !=  NULL   )
			delete  []  m_fundCandids;       // ***** NOTE:  Special syntax(   []    )   for the delete on an ARRAY by 'new'  ******
	}

//	if(   m_sndSample   !=  NULL    )
//		delete   m_sndSample;
}



								////////////////////////////////////////


long		FundCandidCalcer::Get_ChunkSize()    //     512  or  1104   dependant on App    9/2012
{

	long         chunkSize  =   m_logDFT.m_horzScaleDown;


	ASSERT(  chunkSize  ==  1104   );


	return      chunkSize;
}





											////////////////////////////////////////


short    FundCandidCalcer::Calc_Best_Octave_Candidate(  long    spikesWeight,  
														bool    enableMidiClosenessNeiborsFactor,
														short   neighborhoodMidi,  
														bool&   retMarkSPitchRed,   
														CString&  retErrorMesg   )
{

	//   Now the Octave is calculated by examining all the possible Harmonics for all of the 4 Octave Candidates


					//   Does Everything:  Data fetch, calcs, and   returns  INDEX of Octave  with the best score 


	retErrorMesg.Empty();

	retMarkSPitchRed =   false;   //  to flag SPitches that I want to later examing



	double  redux  =   160.0;    //  this is the best 



	short   totalHarmonics        =    Get_Harmonic_Count() ;   

	short   firstMidiFundTemplate =    m_firstMidiFundTemplate;    //     FundamentalTemplateTrForm::Get_FundTemplMaps_FirstRows_MidiNumber();

	short   firstVirtualOctaveIdx =    Get_Actual_FCandidArray_Index(  0  ) ;   // ***  after the SUBoctave candidates ****

	short   totalOctaves   =           Get_Total_FundCandids_In_Array();  



													//   First,  FETCH all the necessary DATA


	if(   ! Gather_SpikeRatio_Data(  retErrorMesg  )   )       //  *** BIG ***   reads from the logDFT and then calls  Calc_All_Harms_NeighborhoodStrength()
	{
		return  -7;
	}


															 //   c)    Calc  'SPIKE  scores' ( FundCandidate  NEIGHBORHOOD-comparison )


	short   bestSpikeOctv,        octaveCount =  0;  //  Will be 4 after loop
	long    bestSpikeScore =  -1;



//                                      3                                         7
	for(   short  oct =  firstVirtualOctaveIdx;       oct <  totalOctaves;       oct++    )     //  Only does  4 iterations
	{

		short  virtOctave  =    oct   -   m_subOctaveCount;



		m_fundCandids[ oct ].m_spikeScore  =    (short)(    m_fundCandids[ oct ].m_harmsNeighborhoodStrength   / redux   );



		if(   virtOctave  >=  0   )
		{
			if(    m_fundCandids[ oct ].m_spikeScore   >   bestSpikeScore   )
			{
				bestSpikeScore =    m_fundCandids[ oct ].m_spikeScore;
				bestSpikeOctv  =     virtOctave;
			}

			octaveCount++;
		}
	}




													//   Finally,  EVALUATE  the  OCTAVE-Candidates by SUMMING all the weighted Scores

	long    bestScore         =  -1;  
	short   bestOctaveIdx  =  -99;        //  to  ScalepitchList::Delete_Bad_Octave_SPitches()  this will cause deletion
	short   bestOctavesAvgHarmonicMag =  -1;



	for(   short oct =  firstVirtualOctaveIdx;      oct <  totalOctaves;      oct++    )
	{

		short  virtualOctaveIdx =    oct -  m_subOctaveCount;



		m_fundCandids[ oct ].m_finalOctvCandidScore  =     spikesWeight  *   m_fundCandids[ oct ].m_spikeScore;
		
					//      'm_finalOctvCandidScore'   is the value that will be SAVED to FILE,  and  is  detectScoreOctaveCandids[]  in  CalcedNote  and   MidiNote classes.  2/2012




		if(       m_fundCandids[  oct  ].m_finalOctvCandidScore   >  bestScore  
		//	&&    m_fundCandids[  oct  ].m_failMinScoreCode ==  0    
			)
		{
			bestOctaveIdx              =     virtualOctaveIdx;             //  must get the VIRTUAL index, not the one compounded with early SUBoctaves

			bestScore                  =     m_fundCandids[ oct ].m_finalOctvCandidScore;

			bestOctavesAvgHarmonicMag  =     m_fundCandids[ oct ].m_avgHarmonicMag;    //   NEW,   1/12/10									
		}
	}




	m_chosenFundamentalCandIdx		    =   bestOctaveIdx;     //  assign ultimate result

	m_bestFinalScore				    =   bestScore;

	m_chosenFundamentalsAvgHarmonicMag  =   bestOctavesAvgHarmonicMag;


	return   bestOctaveIdx;  
}





											////////////////////////////////////////
											//////  below are support functs  /////
											////////////////////////////////////////



bool     FundCandidCalcer::Read_Harmonic_Mags_from_logDFT(   CString&  retErrorMesg    )
{

			//  Does  4 candidateOctaves  PLUS the   3 SUBoctaves



/*****   DISABLED  ****

	bool   onlyDo2ndHalfOfEnelope =   true;     //  If autocorrelation gives better results without the ATTACK component of the envelope...  6/2006

	short  attackPortion =     3;              //   2    Bigger cust LESS of the attack out[  only if(  onlyDo2ndHalfOfEnelope ==   true  )
****/


	retErrorMesg.Empty();


	short   specialHarmCount  =    FundamentalCandidate::Harmonic_Count_Extra();



//	long    chunkSize  =      TransMapAdmin::Get_ChunkSize();                // ************  WRONG for  NAVIGATOR
	long    chunkSize  =      Get_ChunkSize();          //  1104    samples in a PieSlice



	long    x0   =          m_startSample  /  chunkSize;     //  in  'Virtual-WORLD'  coords
	long    x1   =          m_endSample    /  chunkSize;


/**********   MIGHT work as is   

	if(    	    m_startPixelShortMap  >=   0                  //  **** NEW  9/2012,  avoid all the confusion of the   chunkSize
		 &&   m_endPixelShortMap   >     0     )
	{
		 x0  =     m_startPixelShortMap;
		 x1  =     m_endPixelShortMap;    
	}
*********/




	short   totalCandidCount  =      Get_Total_FundCandids_In_Array();

	long    harmProfilesAvgMagnitude  =  0,     xStart;
	short   retMissedBandCnt;



	/***
	if(    onlyDo2ndHalfOfEnelope   )
		xStart =    x0   +    (     ( x1  -  x0 ) /attackPortion    );      //  start  1/3 of way  into the Time-Envelope 
	else
	****/
		xStart =    x0;




	for(    short  oct =0;      oct <   totalCandidCount;       oct++    )     //  should be 7 loops including SUBoctaves 
	{


		short    midiPitchFundCand  =     m_fundCandids[ oct ].m_midiPitch;


		if(   ! m_logDFT.Sum_All_Harmonics_Magnitudes_inSpan(   xStart,  x1,    midiPitchFundCand,    specialHarmCount,  
																						     m_fundCandids[ oct ],	 retMissedBandCnt,  retErrorMesg )   )
			return  false;



					//  Calc the   Average-STRENGTH   of the HarmonicProfile   over time( Avg Harm's Magnitude in profile )
					//  ( Is this the same algo as previous to calc  'm_fundCandids[].m_avgScoreGapped'  ???  
					//   ...see  FundamentalTemplateTrForm::Measure_DuraRects_Fundamental_Candidates()

		long    harmMag;
		bool    retHagIsUndefined;
		short   realHarmCnt =  0;

		harmProfilesAvgMagnitude =  0;   //   init  sum


		for(   short  harm=0;      harm <  FundamentalCandidate::Harmonic_Count();     harm++   )     //   12 
		{

			harmMag  =     m_fundCandids[ oct ].Read_Hamonics_Mag(   harm,   retHagIsUndefined   );

			if(    !retHagIsUndefined    )
			{
				harmProfilesAvgMagnitude +=    harmMag; 
				realHarmCnt++;
			}
		}




		if(   realHarmCnt  >  0   )
			m_fundCandids[ oct ].m_avgHarmonicMag  =     (short)(   harmProfilesAvgMagnitude  /  realHarmCnt  );
		else
			m_fundCandids[ oct ].m_avgHarmonicMag  =   0;
	}





	if(   ! m_logDFT.Sum_All_Harmonics_Magnitudes_inSpan(   xStart,  x1,    m_fundCandidsFive.m_midiPitch,    specialHarmCount,  
																						     m_fundCandidsFive,	 retMissedBandCnt,  retErrorMesg )   )
		return  false;


	return  true;
}



											////////////////////////////////////////


bool     FundCandidCalcer::Read_Neighbor_Mags_from_logDFT(   CString&  retErrorMesg    )
{


			//  Does  4 candidateOctaves  PLUS the   3 SUBoctaves



	bool   onlyDo2ndHalfOfEnelope =   false;     //  This NOT longer is NECESSARY.   9/2012

	short  attackPortion =     3;     //   2    Bigger cust LESS of the attack out[  only if(  onlyDo2ndHalfOfEnelope ==   true




	bool   doDownwardRead  =   true;   // *****************************  



	short  downwardReadOffset;

	if(   doDownwardRead  )
		downwardReadOffset  =    logDFTdOWNWARDrEAD;
	else
		downwardReadOffset  =   0;



//HrmIdx:  0              1                     2                     3                       4                     5                    6                      7

	PairShort    m2pairs [ 10 ]      =  
		
	{  { -12,  +7 },   { +7,  +16 },   { +16, +22 },   { +22, +26 },     { +26, +30 },  { +30, +32 },   { +32, +35  },    { +35, +37 },   { 37,  39 },   { 39,  41 },  /* { 41,  43 }  */                 };  


	PairShort    m2outer[ 10 ]     =    
	{ { -36,  +16 },  { -12,  +22 },    { +7,  +26 },   { +16,  +30 },   { +22,  +32 },  { +26,  +35 },  { +30,  +37 },   { +32,  +39 },  { 35,  41  },   { 37,  43 }                     };   //  for calcMethod =  2



//	PairShort    m4pairs [ 5 ]       =   {     { -5,   +4 },       { +10, +14 },     { +18, +20 },     { +23, +25 },       { +27, +29 }  };   ...Not used, cause we only do calc 2

//	PairShort    m4pairsOpt [ 5 ]  =   {    { -24, +10 },      {  +4, +18 },       { +14, +23 },      { +20, +27 },      { +25, /* repeat */+30 }   };






	short    neibOffsets [ NEIGHBORcOUNT ]   =   {   -36,    -24,    -12,      -5,       +4,      +7,      +10,    +14,     +16,     +18,    +20,   +22,      
		                                                                  +23,    +25,    +26,     +27,    +29,     +30,    +32,     +35,      +37,   +39,    41,    43   };         

//  new offset for table from m2outer[]  ( relative to 1st harm = 79 )    are  -36[ midi 43],      39[midi 118]


	retErrorMesg.Empty();



//	long    chunkSize    =      TransMapAdmin::Get_ChunkSize();        // ************  WRONG for  NAVIGATOR
	long    chunkSize    =      Get_ChunkSize();   //    1104    ...NAVIGATOR  uses this chunksize

	
	long    x0   =          m_startSample  /  chunkSize;     //  in  'Virtual-WORLD'  coords
	long    x1   =          m_endSample   /  chunkSize;



	short   totalCandidCount  =      Get_Total_FundCandids_In_Array();

	long    xStart;




	if(    onlyDo2ndHalfOfEnelope   )
	{
		ASSERT( 0 );      // **** If I am to NOW use this,  need to do some thinking and testing.   9/2012   *****

		xStart =    x0   +    (     ( x1  -  x0 ) /attackPortion    );      //  start  1/3 of way  into the Time-Envelope
	}
	else
		xStart =    x0;




	for(    short  oct =0;      oct <   totalCandidCount;       oct++    )     //  should be 7 loops including SUBoctaves 
	{

		short    midiPitchFundCand  =     m_fundCandids[ oct ].m_midiPitch;


		for(    short  neibIdx =0;      neibIdx <   NEIGHBORcOUNT;       neibIdx++    )     //  should be 7 loops including SUBoctaves 
		{
			
			long    retAvgMag =  0;
			short   offsetFromFundamental  =     neibOffsets[   neibIdx  ];     

			m_fundCandids[ oct ].m_neighborMags[  neibIdx  ].L  =   offsetFromFundamental;


			/****
			if(   !m_logDFT.Sum_All_Harmonics_Magnitudes_inSpan(   xStart,  x1,    midiPitchFundCand,    specialHarmCount,  
																								 m_fundCandids[ oct ],	 retMissedBandCnt,  retErrorMesg )   )
				return  false;
			****/
			short  neibMidi  =    midiPitchFundCand     +     offsetFromFundamental    -   downwardReadOffset;



			if(    ! m_logDFT.Is_MidiPitch_On_Map( neibMidi )     )
			{
				m_fundCandids[ oct ].m_neighborMags[  neibIdx  ].R  =     -9;    //  -9,  not on map
			}
			else
			{	bool   retOffMap;

				if(     ! m_logDFT.Calc_Average_PixelMagnitude_inSpan(  x0,  x1,    neibMidi,   retAvgMag,   retOffMap,    retErrorMesg   )
					||    retOffMap    )
				{
					ASSERT( 0 );
					m_fundCandids[ oct ].m_neighborMags[  neibIdx  ].R  =     -5;    //  -5 error
				}
				else
				  m_fundCandids[ oct ].m_neighborMags[  neibIdx  ].R  =    retAvgMag;
			}
			
		}

	}   //   for(    short  oct =


	return  true;
}



											////////////////////////////////////////


bool     FundCandidCalcer::Gather_SpikeRatio_Data(   CString&  retErrorMesg   )
{



						//   Read_Harmonic_Mags_from_logDFT()   should have been called BEFORE this...   


	retErrorMesg.Empty();



	short     harmCount         =    Get_Harmonic_Count();   
	short     totalCandidCount  =    Get_Total_FundCandids_In_Array();



													// **  Now getting 18 harmonics **


	if(      ! Read_Harmonic_Mags_from_logDFT(  retErrorMesg  )     )      //   HARMONICS,    for ALL candids including SUBoctaves
		return  false;							 															



	if(      ! Read_Neighbor_Mags_from_logDFT(  retErrorMesg  )     )      //   NEIGHBORS,    for ALL candids including SUBoctaves
		return  false;							 	



	


	for(   short  oct =0;       oct<  totalCandidCount;       oct++   )
	{					

		short  candidsMidi    =   m_fundCandids[ oct ].Get_MidiPitch();

		bool   isAsubOctave  =    m_fundCandids[ oct ].m_isAsubOctave;        //  DIMINSH scores for these



		if(   !  m_fundCandids[ oct ].Calc_All_Harms_NeighborhoodStrength(   m_useWeitsNeighborhoodStrength,   retErrorMesg )    )
			return  false;


	//  double  theStrength  =    m_fundCandids[ oct ].m_harmsNeighborhoodStrength;   //  OMIT,  just for DEBUG 
	}


	return  true;
}

















