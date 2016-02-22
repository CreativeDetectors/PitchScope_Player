/////////////////////////////////////////////////////////////////////////////
//
//  SPitchCalc.cpp   -   calculate properties for a new midi note:  ScalePitch, Octave, Duration
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
	
	Do, Re, Mi, Fa, So, La, Ti, Do is another way to express ScalePitch because it also contains no octave assignment.  
*/


#include "stdafx.h"


#include  <math.h>     //  for trig functions


#include   "..\comnFacade\UniEditorAppsGlobals.h"    


#include  "..\comnFacade\VoxAppsGlobals.h"



//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"     
#include  "..\ComnGrafix\CommonGrafix.h"      

#include  "..\comnGrafix\OffMap.h"  
#include  "..\comnGrafix\TransformMap.h"

#include  "..\comnGrafix\mapFilters.h"
//////////////////////////////////////////////////     



#include  "FundamentalCandidate.h"



#include  "DFTtransforms.h"
  

#include  "HarmPairsTrForm.h"
#include  "HarmPairsVerter.h"


#include  "sndSample.h"



#include  "FundCandidCalcer.h"

#include  "CalcNote.h"




#include  "SPitchCalc.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////




double  keyProfile0[  12  ]  =      //     grey Solo     A 
{
//			1                  1+                2nd              minor3            Mag3              4th

		6.667,            0.000,            0.701,            0.583,            0.095,            2.420, 


		0.136,            4.269,            0.000,            1.027,            4.083,            0.375 

};//	   4+                 5th                 5+                6th               minor7            Mag7




double  keyProfile1[  12  ]  =      //     Bad sign     G
{
//			1                  1+                2nd              minor3            Mag3              4th

        11.865,            0.000,            0.000,            2.609,            0.320,            1.200, 


         0.237,            3.159,            0.000,            0.098,            3.693,            0.098

};//	   4+                 5th                 5+                6th               minor7            Mag7




double  keyProfile2[  12  ]  =      //     Highway 49   E       [ ***DIFFICULT ]    Mag3   is strongest
{
//			  1                  1+                2nd              minor3            Mag3              4th

            2.95,             0.42,              0.14,             3.35,              8.80,             1.23, 


            0.45,             2.80,              0.25,             1.68,              1.29,             0.55 

};//	     4+                 5th                 5+               6th               minor7            Mag7




double  keyProfile3[  12  ]  =      //    Brown Sugar   E        [ ***DIFFICULT ]    4th  is strongest
{
//			   1                   1+                2nd              minor3             Mag3              4th

             4.67,              0.17,              0.95,              3.85,              0.30,              5.29, 


             1.43,              2.75,              0.39,              0.04,              1.33,              0.52

};//	      4+                 5th                 5+                 6th               minor7            Mag7



double  keyProfile50[  12  ]  =      //     Mini Skirt    C    [ ***DIFFICULT ]    5th is strongest
{
//			  1                  1+                2nd              minor3            Mag3              4th

           6.838,            0.677,            0.861,            5.224,            0.583,            2.944, 


           0.987,            7.363,            0.000,            0.502,            0.883,            0.098 

};//	     4+                 5th                 5+                6th               minor7            Mag7





double  keyProfile51[  12  ]  =      //     Clarinet    Bb      [ ***DIFFICULT ]    minor7   is strongest
{
//			1                  1+                2nd              minor3            Mag3              4th

         4.724,            0.033,            1.563,            1.797,            3.228,            1.557, 


         0.419,            1.189,            0.657,            1.874,            5.687,            0.760 

};//	   4+                 5th                 5+                6th               minor7            Mag7



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//		   7.0,              -5.0,               0.7,               3.5,                2.0,               3.0, 
//		  -4.0,               4.0,               -4.0,              1.5,                3.0,               0.3 

//		   7.0,              -5.0,               0.7,               3.5,                2.0,               3.0, 
//		  -3.0,               4.0,               -3.0,              1.5,                4.0,               0.3 


//		   8.0,              -9.0,               0.7,               4.0,                2.0,               3.0, 
//		  -4.0,               4.0,               -4.0,              1.5,                3.0,               0.3 


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
double  resultProfile0[  12  ]  =      //    GOOD: GreyMare           PROBLEMS:   Highway49,    DoWhatLike,	
{
//			1                  1+                2nd              minor3            Mag3              4th

		  10.0,              -9.0,               0.7,               4.0,               2.0,               3.0, 


		  -4.0,               4.0,               -4.0,              1.5,                3.0,               0.3                

};//	   4+                 5th                 5+                6th               minor7            Mag7
//////////////////////////////////////////////////////////////////////////////////////////////////////////



double  resultProfile1[  12  ]  =	  //    GOOD:   Highway49             PROBLEMS:  GreyMare
{
//			1                  1+                2nd              minor3            Mag3              4th

		  10.0,              -9.0,               0.7,               4.0,               2.0,               3.0, 


		  -4.0,               4.0,               -4.0,              1.5,                3.0,               -4.0                //   Negative weight for Mag7

};//	   4+                 5th                 5+                6th               minor7            Mag7




double  resultProfile2[  12  ]  =	  //    GOOD:          KeepFromCrying,   Clarnet,   JohnnyGoode,   GreyMare,        
{												  //    PROBLEM:     DoWhatLike,   Politician

//			1                  1+                2nd               min3              Mag3              4th     //  Ease up on neg-Mag7,  but INCREASE  1+  4+   5+

		  10.0,             -14.0,              0.7,               4.0,               2.0,               3.0, 


		  -6.0,               4.0,               -6.0,              1.5,                3.0,               -2.0                //  Negative weight for Mag7

};//	   4+                 5th                 5+                6th               min7               Mag7





 
double  resultProfile_NEXT[  12  ]  =	  //    GOOD:          
{												          //    PROBLEM:   

//			1                  1+                2nd              minor3            Mag3              4th     //  INCREASE  negs:   Mag7,   1+,   4+,   5+

		  10.0,             -14.0,              0.7,               4.0,               2.0,               3.0, 


		  -9.0,               4.0,               -9.0,              1.5,                3.0,               -4.0                //  Negative weight for Mag7

};//	   4+                 5th                 5+                6th               minor7            Mag7








/////////////////////////////////////////////////////////////////////////////////////////////////////
short   Get_PitchScope_App_Code_GLB();

void    Write_To_StatusBar_GLB(   CString&   mesg   );


extern   double   noteFreq[];



extern   long   Get_InputOne();				//  Easy to get these  TEMP values with 'extern'  declare
extern   long   Get_InputTwo();
extern   long   Get_InputThree();


short    Find_Freqs_ScalePitch(  short  freq  );


////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


SPitchCalc::SPitchCalc(  long   numberOfWavemanEvents,   long  byteCountTwoSeconds,    long  numSamplesInBlockLocked,    
					                ListDoubLinkMemry< MidiNote >  *noteList,     bool  usingCircQueDFTprobes      ) :    
                                          m_numberOfWavemanEvents( numberOfWavemanEvents  ),     m_byteCountTwoSeconds( byteCountTwoSeconds  ),
			                              m_calcedNoteListMasterApp( noteList  )
{

						//	CALLED BY:     BitSourceStreaming::Allocate_SPitchCalc()

	CString   retErrorMesg;


	
	m_numPeriods =  18;     //  works best.  Determines bandwidth.    ********  COULD ADJUST  ***********



	m_logDFTdarknessFactor =  48000.0;    //   Makes the DFT lighter or darker,  but does not impact the SOURCE-calcs   ***ADJUST***
										  //        ...just boost the Pixel briteness with a SMALLER value   9/2012



	m_playSpeedFlt =   1.0;    //  Only come here when LOADING a new song.   CALLED ultimately by StreamingAudioplayer::StreamingAudioplayer()


	m_playModeUsingNotelist =   0;      //   0:  Detect/Play      1:  Play-Notelist      2:  ????     ...MOVEDE to SPitchCalc


//	m_noteListHardwareDelayForPlayer        =   0;       //   NEW,  only Player.exe  should deal with this value
	m_noteListHardwareDelayForPlayerAddr =   NULL;    // Now in   PitchPlayerApp,  so its value can persist over multile song loads.   8/2012



	m_numberNeededToMatch    =     4;     
	m_sizeOfPrimNotesCircque =     7;     //  These will affect the size of the FinalCircQue


	m_displayBritnessFactor  =    2;



	m_letNotesSustain =   true;     //       true[ long time ]     affects decision making in   Get_New_Filtered_ReturnNote_CircularQue     **** WILL I ALWAYS want this ON ??? **** 
//		**** EXPERIMENT ...   is 'false'  better for poor signals ******************************************




//  ************************  HOOK this up to setting dialog,   and test ( only can change on reload ) ************************************

	long  bottomDFTrowsToSkip  =   0 ;   //  12      [ 0 - 12 ]         12[best]      ********  ADJUST,   HARDWIRED  *****************

											// *** Maybe this should be an INPUT PARM,  it effective freq-filters the algo and sees to cut out bass harmonics



	m_hiResRatioLogDFT  =   3;     //     *****??    Might fail if I set it to 1 ( 93/2012 )     5[ loosing slices  ...maybe from Blitting the LogDFT    9/12 ]


	ASSERT(   m_hiResRatioLogDFT  <=   kMAXhiResValueLogDFT   );



//	m_dftMapsWidth   =     10;       //  Want 10 for our new lag read system     
	m_dftMapsWidth   =     10   *   kMAXhiResValueLogDFT ;    //   40    [  is really TOO big for Player,  should I use a smaller DFTmap???
																					     //   BUT this would be easier in future if I want eo enable  DFTprobes with CircularQues   9/2012

//	m_kernalWidthDFT  =   3;   //  is  5   for  CircQue  Algo



	m_hpairsMapsWidth =   3;    //   can be 1 or 3,   but if 3 then  m_dftMapsWidth  must also be 3    

	m_componentCountHPairs =   4;      //    4,   7       ONLY[   2(new),    3(new),   4, 5, 6, 7, 8, 9,  10 ]        10[ bad resolution, misses some notes ]
                                                         //											...can set values here to test.  12/11



	m_logDFTtransformDEBUG =  NULL;   //  when I want to write to outside.  Is also a SWITCH for coords.




	m_rowList.Set_Dynamic_Flag(  true  );      //  true :    this Destructor will do the deallocation

	m_totalRows =  0;




	m_sampleRate       =   44100;   // **** HARDWIRED ***

	m_userFinalCircqueSizeTweak =   0;    //  NEW,  lets user make fine adjustments to Delay  for  Mide/WavePlay  sync    




//	m_startBandMidi  =    kMIDIsTARTbANDsPitchCalc;      	//   ( Midi  91,   G )    is near  high string, 12th fret  ( really is a  G ) 
//	m_startBandMidi  =    kMIDIsTARTbANDdft;      	                                     //   ( Midi  52 )  	164 Hz  :    Lowest E on Guitar   
	m_startBandMidi  =    kMIDIsTARTbANDdft   +  bottomDFTrowsToSkip;      //  to   Midi 64,      an octave higher




//	if(    ! Allocate_DFT_Probes(    m_useDFTrowProbeCircQue,   bottomDFTrowsToSkip,     retErrorMesg   )     )
	if(    ! Allocate_DFT_Probes(    usingCircQueDFTprobes,         bottomDFTrowsToSkip,     retErrorMesg   )     )
	{
		AfxMessageBox(  retErrorMesg  );    
	}




	long  chunkSizeNav  =    Get_ChunkSize();

																				
	m_logDFTtransform     =     new    logDFTtrForm(      m_dftMapsWidth,        chunkSizeNav,      0,        chunkSizeNav  ); 

	m_harmpairsTransform  =     new    HarmPairsTrForm(   m_hpairsMapsWidth,   chunkSizeNav   ); 


	if(        m_logDFTtransform       !=  NULL  
		&&   m_harmpairsTransform  !=  NULL    )
	{
		m_logDFTtransform ->Clear(  0  ); 
		m_harmpairsTransform ->Clear(  0  ); 
	}
	else
	{   ASSERT( 0 );    //  **********  INSTALL    error reporting  **********
	}




												 	//  these are for the note detect	
	m_aNoteIsPlaying =   false;		//	m_curNotesMode     =     -1;


	m_prevFoundNote       =     -1;          //   the last scalepitch that was detected.  If <0,  then it was a failure to detect

	m_prevPlayingNotesAvgHarmonicMag =  0;



	
//	m_volumeAdjust  =   0.90;    //  .9    .9[too quiet 6/28]        0.60 [ too loud]    1.20[ too quiet ]   ****ADJUST VOLUME***   The bigger, the SMALLER the volume.  ***NEW calibration because of new sound filtering.**** ,  6/07
											   //   the above notes seem WRONG (3/11)   rexamine this


	m_detectionSensitivityThreshold =   31;    //  is set by Settings dialog  ******* Does this need to be IN SYNC with  app setting ???  ****************





	Initialize_Notes_CircularQue();   //   FIRST   CircQue  ( does the smoothing )





																	//  2nd CircQue :       For the DELAYED presentation of the FINAL-notes


//	m_sizeOfFinalNotesCircque  =   180;    //  180      DECIDE  on DelayTime.       Remember a 'note' is really an iEvent(368 samples).  Might only need 90, but get a DELAY from the 1st circQue too.
														    //              **** MUST also sync with    StreamingAudioplayer::StreamingAudioplayer()  setting for Audio Delay   *******


	if(     Get_PitchScope_App_Code_GLB()  ==   1  )     //   1: Navigator                 [ 0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope
	{

//		long   primaryQueDelayInNotes  =     Calc_Primary_CircQues_Delay_In_CalcedNotes();
		long   primaryQueDelayInNotes  =     m_sizeOfPrimNotesCircque /2;

		m_sizeOfFinalNotesCircque        =     kSizeOfFinalNotesCircque1xSlow   +   primaryQueDelayInNotes   +    m_userFinalCircqueSizeTweak;
	}
	else
		m_sizeOfFinalNotesCircque  =     1;  // ***** UNECESSARY???   Think that  Player  does not even use the FinalNotesCircque   3/11





	ASSERT(   m_sizeOfFinalNotesCircque  <  kMAXsIZEofFINALnoteCircQUE   );


	Initialize_Notes_Final_CircularQue(   m_sizeOfFinalNotesCircque   );     





// *********  Might also make sense to Init  the   m_drivingOffMapHorz   INSIDE  of   Initialize_Notes_Final_CircularQue()   ***********************

	short   mapsHeight;

	m_useScalePitchDrivingOffMap =   true;    //   true    ***** ADJUST ******


	if(    m_useScalePitchDrivingOffMap    )
		mapsHeight =    12;
	else
		mapsHeight =    48;


																	//  The  OffMap for the  DrivingView   must also be in SYNC with the delay created in the  FinalNotesCircque 

  
	m_drivingOffMapHorz      =      new   OffMap(   kMAXsIZEofFINALnoteCircQUE,     mapsHeight,    24  );     //   Make the map bigger than what we use, and only write in left side of Bitmap
																																				      //   This allows realtime change of  'm_sizeOfFinalNotesCircque'  2/11
	if(  m_drivingOffMapHorz  ==  NULL   )
		AfxMessageBox(   "SPitchCalc::SPitchCalc FAILED, could not alloc m_drivingOffMapHorz."   );
	else
		m_drivingOffMapHorz->Clear(  0  );



	m_drivingOffMapVert  =      new   OffMap(   mapsHeight,    kMAXsIZEofFINALnoteCircQUE,      24  );   

	if(  m_drivingOffMapVert  ==  NULL   )
		AfxMessageBox(   "SPitchCalc::SPitchCalc FAILED, could not alloc m_drivingOffMapVert."   );
	else
		m_drivingOffMapVert->Clear(  0  );


	m_doVerticalDrivingview =  false;



	m_musicalKey =   0;   //   0:     key of 'E'     ...the default for loading a new file.  11/11  

	m_bottomFreqCutoffInMidi =  0;

	m_spitchCalcsSndSample =   NULL; 




// ***************************************************************************************************************

	long   chunkSize =   512;    // ******** OK ????   Is WRONG ( 1104 )  BUT does NOT seem to be impacting anything.  9/2012  *******************

// ***************************************************************************************************************

	if(     ! Allocate_SndSample_for_SPitchCalc(    numSamplesInBlockLocked,    chunkSize,   retErrorMesg  )     )
	{
		ASSERT( 0 );  
		AfxMessageBox( retErrorMesg   );
	}




	m_circularSndSample =  NULL;   //  NEW,  a circulr que for OLD  SndSample  samples   1/12


	m_sizeCircularSndSample =  -1;        //    kSizeOfCircularSndSample

	m_indexCircularSndSample  =  -1; 

	m_countValidBytesCircularSndSample  =    -1; 

	m_indexLastSndSampleBlockload =  -1;



	Initialize_MinMax_Variables();              //  **** Very BAD overflow test.  Could get rid of these   9/2012  *****


	
	m_circularSndSample                 =      new     char[   kSizeOfCircularSndSample    ];	 
	if(   m_circularSndSample  ==  NULL  )
	{
		AfxMessageBox(   "SPitchCalc::SPitchCalc  FAILED,  could not allocate m_circularSndSample."    );
	}
	else
	{  m_sizeCircularSndSample =    kSizeOfCircularSndSample;    

		Initialize_SndSample_CircularQue();
	}
}




											////////////////////////////////////////

SPitchCalc::~SPitchCalc()
{

	if(   m_logDFTtransform !=  NULL  )
		delete   m_logDFTtransform;


	if(   m_harmpairsTransform !=  NULL  )
		delete   m_harmpairsTransform;



	if(   m_drivingOffMapHorz !=  NULL  )
		delete  m_drivingOffMapHorz;


	if(   m_drivingOffMapVert !=  NULL  )
		delete  m_drivingOffMapVert;


	if(    m_spitchCalcsSndSample  !=  NULL    )
	{
		delete   m_spitchCalcsSndSample;
		m_spitchCalcsSndSample =  NULL;
	}



	if(    m_circularSndSample  !=  NULL    )
	{
		delete   m_circularSndSample;
		m_circularSndSample =  NULL;
	}
}



											////////////////////////////////////////


long	   SPitchCalc::Get_ChunkSize()    //     {   return  m_horzScaleDown;   }      //   always??? 
{

	long  chunkSize =  -1;


	short   appCode  =      Get_PitchScope_App_Code_GLB();         //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope

	if(       appCode  ==  3
		||    appCode  ==  2   )
	{
		chunkSize =  512;     //      2: VoxSep     3:  PitchScope
	}
	else
		chunkSize =  1104;    //      0:  Player      1:  Navigator  


	return  chunkSize;
}



											////////////////////////////////////////


long	   SPitchCalc::Get_Lowest_MidiStartBand()
{

	long     bottomBand   =      kMIDIsTARTbANDdft;    //   52
	return   bottomBand;
}


											////////////////////////////////////////


void    SPitchCalc::Set_Bottom_Frequency_Cutoff_DFTrows(    long  bottomRowsToSkip   )                  //    long  freqAsMidiNumber   )
{


	   //     <= 0     will set to  DEFAULT  (  Midi 52,    the lowest row that I do  )


       //  ********  Only the HIGH Freq-Filter setting will  trim range, and by one octave.     6/2012   *****************


	long  bottomMostRow =    Get_Lowest_MidiStartBand(); 



	if(    bottomRowsToSkip  >  0  )
	{

		m_bottomFreqCutoffInMidi  =      bottomMostRow   +   bottomRowsToSkip;        //   will change it from 52 to 62
	}
	else
		m_bottomFreqCutoffInMidi  =     0;    //  this is DEFAULT,  uses all rows  ( Midi 52 and higher )    6/12
}




											////////////////////////////////////////


void   SPitchCalc::Find_Long_Minmax(  long  number  )
{


	//  ********   This is a BAD function to do this.  See my notes where it was previously installed.  9/2012   ***********

	//  2147483647			m_biggestInt,   m_smallestInt;


	double  top       =     2147483647.0;
	double  bottom =    -2147483646.0;


	double   numberDbl =    (double)number;



	if(           numberDbl   >=    top     )
	{
		int   dummy =   9;      //   OVERFLOW
	}
	else if(    numberDbl   <=    bottom    )
	{
		int   dummy =   9;     //   OVERFLOW
	}



	/***
	if(   number  >  0   )
	{
		if(     (top  -  numberDbl)   <     m_biggestInt     )    // ????  WHY do I subtract out   ??????
		{
			m_biggestInt =     top  -  numberDbl;
		}
	}
	else if(   number  <  0   )
	{
		if(    (bottom  -  numberDbl)    >    m_smallestInt    )
		{
			m_smallestInt =    bottom  -  numberDbl;
		}
	}
	****/
	if(   number  >  0   )
	{

		if(    numberDbl   >    m_biggestInt   )
			m_biggestInt  =   numberDbl;

	}
	else if(   number  <  0   )
	{

		if(   numberDbl  <  m_smallestInt   )
			m_smallestInt  =    numberDbl;

	}




	double  closenessToUpperLimit =  0.0,      closenessToLowerLimit =  0.0;


	if(          number  >  0   )
	{

		closenessToUpperLimit  =     top   -   numberDbl;


		if(         closenessToUpperLimit   <   m_closestDiffToTop 
			&&   closenessToUpperLimit   >=   0.0   )
		{
			m_closestDiffToTop =    closenessToUpperLimit;
		}


		int   dummy =   9;
	}
	else if(   number  <  0   )
	{	
		
		closenessToLowerLimit  =    (  bottom  -    numberDbl  )   *   -1.0;


		if(           closenessToLowerLimit   <   m_closestDiffToBottom 
			   &&   closenessToLowerLimit  >=  0.0   )
		{
			m_closestDiffToBottom =    closenessToLowerLimit;
		}


		int   dummy =   9;
	}
}



										////////////////////////////////////////


void	  SPitchCalc::Initialize_MinMax_Variables()
{

	//  2147483647			m_biggestInt,   m_smallestInt;

	/***
	m_biggestInt   =     2147483647.0;     //  **** INIT for   FIXED-POINT overflow  ******

	m_smallestInt =    -2147483646.0;
	***/

	m_smallestInt  =     2147483647.0;     //  **** INIT for   FIXED-POINT overflow  ******

	m_biggestInt   =    -2147483646.0;



	 m_closestDiffToTop        =     2147483600.0;   
		 
	 m_closestDiffToBottom   =     2147483600.0;   
}



											////////////////////////////////////////


bool   SPitchCalc::Allocate_DFT_Probes(   bool  useCircQueProbes,   short  bottomDFTrowsToSkip,    CString&  retErrorMesg    )
{


 	                          //   If  'useCircQueProbes'  is true,  this thing uses  1.3 MB  of memory ( see bottom  )         9/2012

	retErrorMesg.Empty();


	ASSERT(  bottomDFTrowsToSkip ==   0   );


	if(   useCircQueProbes   )                              //   Now have 2 ways to calc/write the values for the DFTmaps pixels
		 m_useDFTrowProbeCircQue =   true;
	else
		m_useDFTrowProbeCircQue =   false;




	m_rowList.Empty();    //  ***** BIG *****   Will this deallocate what I want ???????

	m_totalRows =  0;


	long   totalBytesAllocated =  0;




//	m_startBandMidi  =    kMIDIsTARTbANDsPitchCalc;      	//   ( Midi  91,   G )    is near  high string, 12th fret  ( really is a  G ) 
//	m_startBandMidi  =    kMIDIsTARTbANDdft;      	                                     //   ( Midi  52 )  	164 Hz  :    Lowest E on Guitar   
	m_startBandMidi  =    kMIDIsTARTbANDdft   +  bottomDFTrowsToSkip;      //  to   Midi 64,      an octave higher





//	for(    long y =0;    y <   kTOTALdftBANDsSPitchCalc;						y++    )
//	for(    long y =0;    y <   kTOTALdftBANDs;										 y++    )

	for(    long y =0;    y <   ( kTOTALdftBANDs - bottomDFTrowsToSkip);     y++    )
	{


		long  midiNumber  =    y  +  m_startBandMidi;    //  Can start as low as Midi 52,  and it now does.  9/12

		

		short     freqArrayIdx =     midiNumber   -   noteFreqArrayFIRSTMIDI;

		double   freq      =      noteFreq[     freqArrayIdx     ];    //    **** should start at  164.81  **** 


		short   scalePitch =      Find_Freqs_ScalePitch(     (short)freq     );     //  ** Does this always work ???? **
		if(       scalePitch  <  0  )
		{
			ASSERT( 0 );
		}


		double   angFreq     =     freq   /   (double)(  m_sampleRate  );
		
        long      cellLength  =     (long)(    (double)m_numPeriods   *   ( 1.0 / angFreq )    );    //  also  WAS 'circCueLength'



		DFTrowProbe              *nuRow             =   NULL; 
		DFTrowProbeCircQue   *nuRowCircQue =   NULL; 
		short    lagWriteInPixels =  0;




		long   trigTableEntrySize =   2  *  4;     //   2 longs in an entry

		long   circQueEntrySize  =   2  *  4;     //   2 longs in an entry


		if(     useCircQueProbes   )        					  
		{
			nuRowCircQue  =   new    DFTrowProbeCircQue(   *this,   cellLength   );  // For this algo,  cellLength  is same as entry count for Trig table. 
			nuRow             =   nuRowCircQue;

			totalBytesAllocated  +=    (   (  cellLength  *  trigTableEntrySize  )   +    (  cellLength  *  circQueEntrySize  )    );
		}
		else
		{	nuRow              =   new    DFTrowProbe(  *this,   cellLength   ); 

			totalBytesAllocated  +=     (  cellLength  *  trigTableEntrySize  );
		}




		if(    nuRow  ==   NULL  )
		{
			ASSERT( 0 );
//			m_isOK =    false;
			retErrorMesg  =   "SPitchCalc::Allocate_DFT_Probes  FAILED, could not ALLOC  DFTrowProbe." ;
			return  false;
		}


			nuRow->m_yVirtual     =    (short)y;
			nuRow->m_frequency  =       freq;    
			nuRow->m_scalePitchIdx =   scalePitch;
			nuRow->m_midiNumber  =    midiNumber;


			if(   m_useDFTrowProbeCircQue    )
			{
				ASSERT( nuRowCircQue );


				long  chunkSizeNav  =  Get_ChunkSize();   //  1104    ********* OK ???    9/12 ****************



// ***************  Is this USED ????   ****************************************

//				nuRowCircQue->m_lagXwriteDFTpixel =    cellLength  /  (2 * m_horzScaleDown );  //  how far  horizontally-back  in the DFTmap do we have to write the pixelMag so that it is centered.
				nuRowCircQue->m_lagXwriteDFTpixel =    cellLength  /  (2 * chunkSizeNav );  //  how far  horizontally-back  in the DFTmap do we have to write the pixelMag so that it is centered.



// **** DO I want to use rounding ???  *************************



				lagWriteInPixels  =   nuRowCircQue->m_lagXwriteDFTpixel;
			}


		m_rowList.Add_Tail(   *nuRow   );

		m_totalRows++;





	    long  circCueLength   =     (long)(      (double)(  m_numPeriods   )   *   ( 1.0 / angFreq )     );      //   same as  'cellLength'  for this algo   
													

		ASSERT(   nuRow->m_cellLength  ==   circCueLength   );    //  NEW,  9/4/12



		/********************************  NO,  because even the OLD ProbeAlgo has a Kernal Count that is a multile of the Period of that row.   9/2012

		bool   useWindowingFunction =    true;

		if(    useCircQueProbes   )
			useWindowingFunction =    false;

		*****/
		bool   useWindowingFunction =    false;



		if(      ! nuRow->Create_TrigTable(   angFreq,    circCueLength,    useWindowingFunction   )     )  
		{
			ASSERT( 0 );
			retErrorMesg  =   "SPitchCalc::Allocate_DFT_Probes failed,    Create_TrigTable."  ;
			return  false;     
		}


	/***
		char       firLet,  secLet;
		CString   retNotesName;

	//	Get_ScalePitch_LetterName(   scalePitch,    &firLet,  &secLet,    false   );
		SPitchCalc::Get_ScalePitch_LetterName(   scalePitch,    &firLet,  &secLet,    0,   retNotesName    );   


	    TRACE(   "SPitchCalc  ROW:    Y= %d :    ScalePitch=  %d,    Freq = %d     Midi = %d      %c%c,     cellLength = %d    lagWriteInPixels = %d    \n",       		  
						         y,   scalePitch,    (short)(   nuRow->m_frequency  ),  midiNumber,  firLet,  secLet,    circCueLength,   lagWriteInPixels    );
	***/


	}    //  	for(    long y =0;    y <   ( kTOTALdftBANDs - bottomDFTrowsToSkip);  




//	TRACE(   "\n\nDFT  PROBES  have been allocated  [ %d  bytes  ]  \n\n"  ,   totalBytesAllocated    );    //  1.3 MB   (   1,355,424  bytes )    9/2012


	/****
	if(    useCircQueProbes    )
		TRACE(   "\nDFT  PROBES   to    USE   CircQues ( expensive )\n"   );
	else
		TRACE(   "\nDFT  PROBES   to    NOT use    CircQues     \n"    );
	***/



	return  true;
}



											////////////////////////////////////////


void   SPitchCalc::Initialize_Notes_CircularQue()
{

	ASSERT(   m_sizeOfPrimNotesCircque  >= 1   );


//	short  maxInitSizeValue  =   m_sizeOfPrimNotesCircque;
	short  maxInitSizeValue  =   kMAXsIZEofNOTEcircQUE;    //  BETTER , then we do NOT have to reinitialize when changing the settings for {  maxInitSizeValue,  m_numberNeededToMatch }


	
	for(   short i = 0;    i < maxInitSizeValue;    i++   )
	{

		m_circQuePrimNotes[ i ].scalePitch    =   -1;
		m_circQuePrimNotes[ i ].octaveIndex =   -1;
		m_circQuePrimNotes[ i ].detectScoreHarms =  -1;


		m_circQuePrimNotes[ i ].detectAvgHarmonicMag =  -1;		



//		m_circQuePrimNotes[ i ].detectScoreOctave =  -1;

		for(    short oct =0;    oct <  kCountOfNavOctaveCandidates;    oct++   )
			m_circQuePrimNotes[ i ].detectScoreOctaveCandids[  oct  ]  =   -1;
	}



	m_currentIndexPrimCircque =   0; 
}



											////////////////////////////////////////


void   SPitchCalc::Initialize_Notes_Final_CircularQue(   short   cuesSize   )
{

		//   cuesSize  can be as small as 1,  which wouldmean that there was NO delay 


	ASSERT(         cuesSize   >=     1    
		          &&    cuesSize   <     kMAXsIZEofFINALnoteCircQUE    );

//	short  maxInitSizeValue  =   kMAXsIZEofFINALnoteCircQUE;    //  BETTER , then we do NOT have to reinitialize when changing the settings for {  maxInitSizeValue,  m_numberNeededToMatch }

	
	for(   short i = 0;    i < cuesSize;    i++   )
	{

		m_circQueFinalNotes[ i ].scalePitch    =   -1;
		m_circQueFinalNotes[ i ].octaveIndex =   -1;
		m_circQueFinalNotes[ i ].detectScoreHarms =  -1;


//		m_circQueFinalNotes[ i ].detectScoreOctave =  -1;

		for(    short oct =0;    oct <  kCountOfNavOctaveCandidates;    oct++   )
			m_circQuePrimNotes[ i ].detectScoreOctaveCandids[  oct  ]  =   -1;




		m_circQueFinalNotes[ i ].detectAvgHarmonicMag =  -1;		


		m_circQueFinalNotes[ i ].synthCode =   -9;                    // ************  WANT init this too ????    3/19/11 ******************
		m_circQueFinalNotes[ i ].primaryQuesSampleIdx =   -9;
//		m_circQueFinalNotes[ i ].endingSampleIdxFile =  -9;

		m_circQueFinalNotes[ i ].beginingSampleIdxFile =  -9;
	}


	m_currentIndexFinalCircque =   0; 
}



											////////////////////////////////////////


void    SPitchCalc::Initialize_SndSample_CircularQue()
{


	char  zeroValue  =       0;    //   or   -128   [ look at this with hex in debugger    ***   [  is  0x00,   clears all bits

	char  otherZero  =    127;



	ASSERT(   m_sizeCircularSndSample  >= 1   );


	long  maxInitSizeValue  =   kSizeOfCircularSndSample;    //  BETTER , then we do NOT have to reinitialize when changing the settings for {  maxInitSizeValue,  m_numberNeededToMatch }



	char        *rover  =    m_circularSndSample;
	ASSERT(   rover );

	
	/***
	for(   long i = 0;    i < maxInitSizeValue;    i++   )
	{
		*rover  =   zeroValue;
		rover++;
	}

//		 memset(    dst,   127,   numByts   );
		 memset(    dst,      0,   numByts   );     //   0:   will clear all BITS...   think that is what we want for Windows ???   3/11
	***/

	memset(   rover,     0,    maxInitSizeValue   );     //   0:   will clear all BITS...   think that is what we want for Windows ???   3/11



	m_indexCircularSndSample  =  0; 

	m_countValidBytesCircularSndSample  =    0; 


	m_indexLastSndSampleBlockload  =  -1;     // ****** TRICKY,  am I calling this function too ofter??  ******
}





											////////////////////////////////////////


bool    SPitchCalc::Allocate_SndSample_for_SPitchCalc(   long  numSamplesInBlockLocked,   long  chunkSize,    CString&  retErrorMesg   )
{

	retErrorMesg.Empty();


	if(    m_spitchCalcsSndSample  !=  NULL    )
	{
		delete   m_spitchCalcsSndSample;
		m_spitchCalcsSndSample =  NULL;
	}


	m_spitchCalcsSndSample          =             new   SndSample(    numSamplesInBlockLocked,    chunkSize  );   //  now is   11,040   because of    44160 byte
	if(  m_spitchCalcsSndSample  ==  NULL   )
	{			
		retErrorMesg =    "SPitchCalc::Allocate_SndSample_for_SPitchCalc  FAILED,  can NOT allocate SndSample"   ;
		return  false;
	}

	return  true;
}



											////////////////////////////////////////


long	   SPitchCalc::Calc_Samples_In_PieEvent(  double  playSpeed  )
{

						                           //   can use   SPitchCalc::m_playSpeed   for input value

	//  takes into account the REDUX  for  SPEED-slowDown


	long   totalSamplesInPieSection  =       (  m_byteCountTwoSeconds  /  (  m_numberOfWavemanEvents  -1L ) )     / 4L;   


	long      sampCountInPieSliceWithSpeedRedux   =   (long)(    (double)totalSamplesInPieSection  /  playSpeed   );
	return   sampCountInPieSliceWithSpeedRedux;
}




											////////////////////////////////////////


long	   SPitchCalc::Calc_Ideal_FinalCircQue_Size(   short  newFilterSize,    short&  retUserTweakValueCalced   )
{

	 //   When we change the size of the PrimaryCircQue,  we change the DELAY which can be big for very SLOW Speeds. To compensate for 
	 //    the increased delay,  we  adjust the  DELAY in the FinalCircQue by changing its size.   11/11


//	long	 logDFTdelay  =     Calc_LogDFTs_Delay_In_CalcedNotes();   


	retUserTweakValueCalced =  -999;


	long   initPrimaryQueDelay  =     5  / 2;     //  these are the initalization values for app start, and loac of new song.   11/11

	long   newPrimaryQueDelay =     newFilterSize / 2;


    long	 initialLengthOfFinCircque  =    kSizeOfFinalNotesCircque1xSlow   +  initPrimaryQueDelay   +  m_userFinalCircqueSizeTweak;   //  128  at program startup



//	long   oldDelay   =     oldPrimaryQueDelay     +    kSizeOfFinalNotesCircque1xSlow    +  m_userFinalCircqueSizeTweak;   

//	long   newDelay =     newPrimaryQueDelay   +    kSizeOfFinalNotesCircque1xSlow     +  m_userFinalCircqueSizeTweak;   //  need to make these sor of equal


	long   newLengthOfFinCircque =    initialLengthOfFinCircque   +   (  initPrimaryQueDelay  -  newPrimaryQueDelay  );  //  As 'newPrimaryQueDelay'  increases, 
																												                                                             //  'newLengthOfFinCircque'  must decrease.  


	retUserTweakValueCalced   =    m_userFinalCircqueSizeTweak;

	short   userTweakValueALT =    newLengthOfFinCircque  -   (  kSizeOfFinalNotesCircque1xSlow  +   newPrimaryQueDelay  );




	if(    newLengthOfFinCircque  >=    kMAXsIZEofFINALnoteCircQUE    )    //  360
	{
		ASSERT( 0 );   // ****** NEVER gets hit   12/2011

		newLengthOfFinCircque  =    kMAXsIZEofFINALnoteCircQUE  - 2; 

		retUserTweakValueCalced =    newLengthOfFinCircque  -   (  kSizeOfFinalNotesCircque1xSlow  +   newPrimaryQueDelay  );

	}
	else if(    newLengthOfFinCircque  >=    kMAXsIZEofFINALnoteCircQUErealistic    )    //  140,   because of driving view
	{

				//  *****   Hit this when I adjust   the User Tweak slider  12/15/11  ****** 
				//
				//	  CAREFUL for this scenerio as it affects:   PsNavigatorDlg::Blit_Offscreen_Bitmap_to_DrivingView_Pane()     12/11

		newLengthOfFinCircque     =     kMAXsIZEofFINALnoteCircQUErealistic  - 2;         

		retUserTweakValueCalced =    newLengthOfFinCircque  -   (  kSizeOfFinalNotesCircque1xSlow  +  newPrimaryQueDelay  );
	}


	return   newLengthOfFinCircque;
}



											////////////////////////////////////////


void	   SPitchCalc::Change_Final_CircularQue_Size(   long  newQueSize  )
{

	//   Does NO initialization.   Calling FUNCTION   must calc DELAYs  by   {  speed,   delay of Primary-Circque,   userTweak  }     3/11


	 //  CALLED by  PsNavigatorDlg::On_Settings_optionsMenu()       PsNavigatorDlg::Set_SPitchCalcs_Detail_Threshold(   int  curSliderPosition   ) 


	m_sizeOfFinalNotesCircque =    newQueSize;


	if(   m_currentIndexFinalCircque  >=   newQueSize   )
	{
		m_currentIndexFinalCircque  =   newQueSize - 2;    // **********   NEW,   OK ????  *********  do this right away so we are not pointing to an unused part of array   3/11
	}
}



											////////////////////////////////////////


short    SPitchCalc::Calc_Median_Filter_Ratio_by_Speed(   double   playSpeed,    double  widthSpeedOneDecimal,   short&  retMatchCount     )
{

			//  RETURNS values for:     PitchPlayerApp::m_sizeOfPrimNotesCircque      PitchPlayerApp::m_numberNeededToMatch 

			//  CALLED by    PsNavigatorDlg::Set_SPitchCalcs_Detail_Threshold()

	retMatchCount =   -1;


	short  retUserTweakValueCalced;
	

	short  filterSize  =     (short)(    widthSpeedOneDecimal   *  playSpeed    );   //   Make filter  length/kernal  dependant on SlowDown speed    11/11


	if(    filterSize   >=   kMAXsIZEofNOTEcircQUE    )
	{
		ASSERT( 0 );  //  land here some of the time,  but only at Speed 8  with  very low detail setting.    11/6/11
		filterSize  =   kMAXsIZEofNOTEcircQUE  -1;
	}



	if(    filterSize   <   2   )    //  values are   1.0   to  1.9
	{

		short   actualSize =   1;         //   playerApp.m_sizeOfPrimNotesCircque       =    1;     
		retMatchCount     =   1;         //   playerApp.m_numberNeededToMatch  =    1;

		m_numberNeededToMatch  =     retMatchCount; //  Need to update these in sPitchCalcer so  Calc_Primary_CircQues_Delay_In_CalcedNotes() can work   3/11
		m_sizeOfPrimNotesCircque        =     actualSize;   

		long	 newLengthOfFinCircque =    Calc_Ideal_FinalCircQue_Size(   actualSize,   retUserTweakValueCalced   );

		Change_Final_CircularQue_Size(   newLengthOfFinCircque  );  

		return  actualSize;
	}
	else if(    filterSize   <   3   )    //  values are   2.0   to  2.9
	{

		short   actualSize =   2;    
		retMatchCount     =   2;      

		m_numberNeededToMatch  =     retMatchCount;     
		m_sizeOfPrimNotesCircque        =     actualSize;   

		long	 newLengthOfFinCircque =    Calc_Ideal_FinalCircQue_Size(   actualSize,   retUserTweakValueCalced   );

		Change_Final_CircularQue_Size(   newLengthOfFinCircque  );  

		return  actualSize;
	}



													         //   NOTE:   numberNeededToMatch  must be   MORE than  50% of filter size,   or it will fail

	short     numberNeededToMatch =    filterSize   /  2;       //    filterSize values here are   >= 3

	double   halfSizeDb  =     (double)filterSize   / 2.0;  
	short     halfSizeInt  =                 filterSize   /  2;

	if(   halfSizeDb   >   halfSizeInt   )
		numberNeededToMatch  +=  1;      //  2  would fail for filter size
	else
		numberNeededToMatch++;    //   make it 1 bigger than half    ( 50% will fail )



	if(    numberNeededToMatch  >=   filterSize    )
	{
		ASSERT( 0 );
		return  -1;
	}



		
	retMatchCount   =   numberNeededToMatch;   //	sPitchCalcer->m_numberNeededToMatch  =     numberNeededToMatch;     //  Need to update these in  sPitchCalcer so  Calc_Primary_CircQues_Delay_In_CalcedNotes() mcan work   3/11
																	   //   sPitchCalcer->m_sizeOfPrimNotesCircque        =     filterSize;   


	m_numberNeededToMatch  =     retMatchCount;     //    Need to update these in  sPitchCalcer so  Calc_Primary_CircQues_Delay_In_CalcedNotes() mcan work   3/11
	m_sizeOfPrimNotesCircque        =     filterSize;   
	


	long	 newLengthOfFinCircque =    Calc_Ideal_FinalCircQue_Size(   filterSize,   retUserTweakValueCalced  );

	Change_Final_CircularQue_Size(  newLengthOfFinCircque  );  
	

	return  filterSize;
}



											////////////////////////////////////////


short     SPitchCalc::Calc_Position_Nudge_for_Detail_Slider(   double  speedFlt    )
{


	double   detailSliderReduceFactor  =  4.0;

	short     playSpeedTimesTen  =    (short)(   speedFlt   *  10.0  );   //   switch()  will NOT acept  DOUBLE   

	short    retPositionNudge =   -1;
	double  minDecimalFilterSize =   -1.0;


	switch(   playSpeedTimesTen   )           
	{
			case  10:     minDecimalFilterSize =     3.0 ;     //    gives 2/3  at far end of slider.    ( 2 to match,  for filterSize of 3  )
				break;

			case  15:     minDecimalFilterSize =     2.0 ;      //    gives 2/3 
				break;

			case  20:     minDecimalFilterSize =     1.5 ;     //    gives 2/3 
				break;

			case  30:     minDecimalFilterSize =     1.0 ;     //    gives 2/3 
				break;

			case  40:     minDecimalFilterSize =     .75 ;     //    gives 2/3    
				break;

			case  60:     minDecimalFilterSize =     .50 ;     //    gives 2/3   
				break;

			case  80:     minDecimalFilterSize =     .50 ;     //   .50 gives  3/4                  .375 gives 2/2
				break;

			default:   ASSERT( 0 );    
				          minDecimalFilterSize =    3.0;    
			break;
	}


//	  filterSizeDecimal  =   (double)(  curSliderPosition   +  positionNudge  )       /    (double)m_detailSliderReduceFactor;    //    4.0;   

	short  minSliderPos =  0;

	retPositionNudge =      (short)(   minDecimalFilterSize   *   detailSliderReduceFactor   )   -   minSliderPos;

	return  retPositionNudge;
}



											////////////////////////////////////////


void    SPitchCalc::Erase_CircularQues_and_DrivingBitmap()
{

			//  call this before start to play again when in a    "different FILE POSITION"    than the last play


	Initialize_Notes_CircularQue();


	Initialize_Notes_Final_CircularQue(   m_sizeOfFinalNotesCircque   );    //  In Player, have just ONE Note in the FinalCircQue.  No problems so far.  11/4/11 




	if(   m_drivingOffMapHorz !=  NULL  )
		m_drivingOffMapHorz->Clear(  0  );   // Looks like I even have this BitMap in Player,  does nto seem to cause trouble.   11/4/11


	if(   m_drivingOffMapVert !=  NULL  )
		m_drivingOffMapVert->Clear(  0  );   // Looks like I even have this BitMap in Player,  does nto seem to cause trouble.   11/4/11
}




											////////////////////////////////////////


void    SPitchCalc::Erase_logDFTmap_and_HarmPairsMap()
{

			//  ***** NEW,   7/2012    Will this help problems with accuract of NoteDetect after a PreRoll ???? 


	if(   m_logDFTtransform  ==  NULL   )
	{
		ASSERT( 0 );
	}
	else
		m_logDFTtransform->Clear(  0   );    



	if(   m_harmpairsTransform  ==  NULL   )
	{
		ASSERT( 0 );
	}
	else
		m_harmpairsTransform->Clear(  0   );    
}








											////////////////////////////////////////


void    SPitchCalc::Add_New_Note_to_CircularQue(   CalcedNote&    newNote    )
{

	ASSERT(   m_currentIndexPrimCircque >= 0     &&     m_sizeOfPrimNotesCircque <=  kMAXsIZEofNOTEcircQUE    );  

	ASSERT(   m_currentIndexPrimCircque <   kMAXsIZEofNOTEcircQUE    ); 


	m_circQuePrimNotes[  m_currentIndexPrimCircque  ]  =    newNote;    //  BETTER this way,   all new member vars will get copied  


	m_currentIndexPrimCircque++;

	if(    m_currentIndexPrimCircque   >=   m_sizeOfPrimNotesCircque    )  
		m_currentIndexPrimCircque  =    0;      //  init  INDEX of circQue
}




											////////////////////////////////////////


void    SPitchCalc::Add_New_Note_to_Final_CircularQue(   CalcedNote&   newNote   )
{

		// **********   BUG   for   BACKWARDS play  ?????  Do not think so 10/11    *********************** 


	ASSERT(   m_currentIndexFinalCircque >= 0     &&     m_currentIndexFinalCircque <=  kMAXsIZEofFINALnoteCircQUE    );  

	ASSERT(     m_currentIndexFinalCircque <   kMAXsIZEofFINALnoteCircQUE    );   // ***** REPLACE with this


	m_circQueFinalNotes[  m_currentIndexFinalCircque  ]  =    newNote;     // Assign everything in the structure   3/21/11


	m_currentIndexFinalCircque++;

	if(    m_currentIndexFinalCircque   >=   m_sizeOfFinalNotesCircque    )  
		m_currentIndexFinalCircque  =    0;      //  init  INDEX of circQue
}




											////////////////////////////////////////


void    SPitchCalc::Add_SndSample_Samples_to_SndSampleCircularQue(   long   bytesToProcess   )
{


	ASSERT(   bytesToProcess  <  m_sizeCircularSndSample   );     //   if the Que is NOT bigger than the input bytes ( bytesToProcess ),  then 
																								//	 it woulld OVERWRITE data it just put in.  1/12

	ASSERT(   bytesToProcess > 0  );

	ASSERT(  m_spitchCalcsSndSample  !=  NULL  );

	ASSERT(  m_circularSndSample  !=  NULL  );

	ASSERT(   m_indexCircularSndSample  >=  0  );



	m_indexLastSndSampleBlockload  =    m_indexCircularSndSample;   // save the INDEX value for the   FIRST (start)  sample from the 'SndSample' 



	char       *srcRover   =     m_spitchCalcsSndSample->Get_StartByte(); 
	ASSERT(  srcRover  );



	for(   long samp = 0;      samp <  bytesToProcess;     samp++    )
	{

		m_circularSndSample[   m_indexCircularSndSample   ]  =    *srcRover;     //  index always points to the OLDEST member of the CircQue

		srcRover++;



		if(    m_countValidBytesCircularSndSample   <   m_sizeCircularSndSample    )
		{
			m_countValidBytesCircularSndSample++;    // **********   TRICKY,  do not want an unrealistic number here
		}



		m_indexCircularSndSample++;     //  init  INDEX of circQue

		if(    m_indexCircularSndSample   >=   m_sizeCircularSndSample    )  
			m_indexCircularSndSample  =    0;    
	}
}



											////////////////////////////////////////


char   SPitchCalc::Read_SndSample_CircularQue_Value(  long  indexVirtual   )
{


			//  *****  This gets called VERY Frequently,  might want to find a way to OPTIMIZE it,  IF it seems like a bottleneck.   1/12  ******


	char   retValue   =    0;   //   -128;  
	long   realIndex =   -1;


	ASSERT(  m_circularSndSample  );   // **** to OPTIMIZE,  take out the ASSERTs  when happy with its performance.  1/29/12  ******

	ASSERT(  indexVirtual >=  0  );

	ASSERT(  m_sizeCircularSndSample  > 0  );

	ASSERT(  m_indexLastSndSampleBlockload  >=  0  );



	if(      indexVirtual   >=   m_sizeCircularSndSample   )   
		 realIndex =    indexVirtual   -   m_sizeCircularSndSample; 
	else
		realIndex  =    indexVirtual;  


	
	//	  'indexVirtual'  is VIRTUAL,  in that when ADJUSTED,  it is the SAME INDEX we would use to find the SAME VALUE in   'm_spitchCalcsSndSample' ( see ALT test below ).
	//                                           But this might not be valid  for  "backed-up" OFFSET   reads  from  Calc_logDFTs_Values_Regular()



	ASSERT(   realIndex >=  0     &&    realIndex < m_sizeCircularSndSample   );    // **** to OPTIMIZE,  take out the ASSERTs  when happy with its performance.  1/29/12  ******

	retValue =  	m_circularSndSample[  realIndex  ];  




/****   a GOOD TEST to verify that the BIG Circular-SndSample has the same values as the   Smaller SndSample.    1/29/12

	   //        However it will fail when we start to read backwards in time for Slow-Speed SMALL data fetches of NOT-Slow Samples ( the 8 bit ones )
	   //        If want to use this test,  Must DISABLE the  Adjusting-Offset  mechanism in  Calc_logDFTs_Values_Regular()   2/2012


	long  adjustedIndex =    indexVirtual   -   m_indexLastSndSampleBlockload;     //  Now calling-functions ADD in  'm_indexLastSndSampleBlockload'

	char  altValue   =      *(          m_spitchCalcsSndSample->Get_StartByte()    +    adjustedIndex       );   // *** TEST,  is it right ??? ****

	if(     altValue   !=  retValue   )
	{
		 ASSERT( 0 );       //   int  dummy =   9;
	}
*****/

	return  retValue;
}



											////////////////////////////////////////


void    SPitchCalc::Get_Oldest_Note_from_Final_CircularQue(   CalcedNote&   retNote   )		
{


		//  Since  'm_currentIndexFinalCircque'  is incremented after last write,  it now points to the OLDEST position in the Cue.


	ASSERT(     m_currentIndexFinalCircque >= 0     &&     m_currentIndexFinalCircque <=  kMAXsIZEofFINALnoteCircQUE    );  


//	ASSERT(     m_currentIndexFinalCircque  <   kMAXsIZEofFINALnoteCircQUE    );   // ***** REPLACE with this
//	ASSERT(     m_currentIndexFinalCircque  <   m_sizeOfFinalNotesCircque        );

	if(              m_currentIndexFinalCircque  >=   m_sizeOfFinalNotesCircque    )   
	{
		ASSERT( 0 );    //  Is this hit?   1/19/2012
		m_currentIndexFinalCircque =   m_sizeOfFinalNotesCircque  - 1;
	}



	retNote  =   m_circQueFinalNotes[   m_currentIndexFinalCircque   ];   
}



											////////////////////////////////////////


void    SPitchCalc::Get_Newest_Note_from_Final_CircularQue(   CalcedNote&   retNote   )		
{


		//  Since  'm_currentIndexFinalCircque'  is incremented after last write,  it now points to the OLDEST position in the Cue.


	ASSERT(     m_currentIndexFinalCircque >= 0     &&     m_currentIndexFinalCircque <=  kMAXsIZEofFINALnoteCircQUE    );  


//	ASSERT(     m_currentIndexFinalCircque  <   kMAXsIZEofFINALnoteCircQUE    );   // ***** REPLACE with this
//	ASSERT(     m_currentIndexFinalCircque  <   m_sizeOfFinalNotesCircque        );


	long  newestIndex =   m_currentIndexFinalCircque  -1;

	if(     newestIndex  < 0    )   
	{		
		newestIndex =   m_sizeOfFinalNotesCircque  - 1;
	}



	retNote  =   m_circQueFinalNotes[   newestIndex   ];   
}




											////////////////////////////////////////


void    SPitchCalc::Transpose_DrivingViews_OffMap_by_MusicalKey(   short  newMusicalKey,   bool  isGoingBackwards,    bool  doVertical   )
{


	ASSERT(   doVertical  ==   m_doVerticalDrivingview   );


	OffMap  *drivingOffMap;

	if(    doVertical   )
		drivingOffMap =    m_drivingOffMapVert; 
	else
		drivingOffMap =    m_drivingOffMapHorz; 


	if(   drivingOffMap  ==  NULL   )
	{
		ASSERT( 0 );
		return;
	}


	if(   newMusicalKey  < 0    ||    newMusicalKey   >  11   )
	{
		ASSERT( 0 );
		return;
	}


	drivingOffMap->Clear(  0  );


	short   travIndex  =    m_currentIndexFinalCircque;    //  INIT index to point at the  OLDEST  CalcedNote




	if(   doVertical   )
	{

		if(    isGoingBackwards   )
		{

			short   yWriteColumn =   drivingOffMap->m_height  -1;     //  start at END of map and work backwards


			for(   short  i =0;      i <  ( m_sizeOfFinalNotesCircque );     i++    )
			{

				CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 


		//		Write_DrivingViews_Y_Row(   calcedNote,    yWriteColumn,    newMusicalKey,    *drivingOffMap   );

				Write_DrivingViews_Y_Row_RAW(    calcedNote.scalePitch,   calcedNote.detectAvgHarmonicMag,    calcedNote.octaveIndex,  
					                                                                       yWriteColumn,    	 newMusicalKey,    *drivingOffMap  );


				yWriteColumn--;   

				travIndex++;              //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes
				if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
					travIndex =   0;
			}

		}
		else        //  FORWARD:     For the DrivingView:    the  OLDEST notes are on the LEFT,  and the NEWER notes are on the RIGHT
		{ 

			short   yWriteColumn  =    Calc_Offmaps_DeadZone_WriteRow(  drivingOffMap->m_height  );    //  we start right into the middle of map,  near the deadzone


			for(   short  i =0;      i <  ( m_sizeOfFinalNotesCircque );     i++    )
			{

				CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 


			//	Write_DrivingViews_Y_Row(   calcedNote,    yWriteColumn,    newMusicalKey,    *drivingOffMap   );

				Write_DrivingViews_Y_Row_RAW(    calcedNote.scalePitch,   calcedNote.detectAvgHarmonicMag,    calcedNote.octaveIndex,  
					                                                                       yWriteColumn,      newMusicalKey,    *drivingOffMap   );


				yWriteColumn++;    
	
				travIndex++;              //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes
				if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
					travIndex =   0;
			}
		}
	}     //  end of VERTICAL

	else
	{  
		if(    isGoingBackwards   )
		{

			short  xWriteColumn  =    m_sizeOfFinalNotesCircque  -1;     //  start from the right and write to the left 


			while(  xWriteColumn  >=  0 )
			{

				CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 


			//	Write_DrivingViews_X_Column(   calcedNote,    xWriteColumn,   m_musicalKey,   *drivingOffMap  );
				Write_DrivingViews_X_Column_Raw(   calcedNote.scalePitch,   calcedNote.detectAvgHarmonicMag,    calcedNote.octaveIndex,  
					                                                                             xWriteColumn,   m_musicalKey,   *drivingOffMap  );



				xWriteColumn--;

				travIndex++;    //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes
				if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
					travIndex =   0;
			}
		}
		else        //  FORWARD:     For the DrivingView:    the  OLDEST notes are on the LEFT,  and the NEWER notes are on the RIGHT
		{ 

			short  xWriteColumn =   0;


			for(   short  i =0;      i <  ( m_sizeOfFinalNotesCircque );     i++    )
			{

				CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 


//				Write_DrivingViews_X_Column(   calcedNote,    xWriteColumn,    newMusicalKey,    *drivingOffMap   );
				Write_DrivingViews_X_Column_Raw(   calcedNote.scalePitch,   calcedNote.detectAvgHarmonicMag,    calcedNote.octaveIndex,  
					                                                                             xWriteColumn,   newMusicalKey,   *drivingOffMap  );



				xWriteColumn++;

				travIndex++;    //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes
				if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
					travIndex =   0;
			}
		}
	}    //  end of HORIZONTAL
}




											////////////////////////////////////////


void    SPitchCalc::ReDraw_DrivingViews_OffMap_from_FinalCircQues_Array(   bool  isPlayingBackward,   OffMap&  drivingOffMap,    bool  doVertical    )
{


	bool	        goingBackwardALT  =     Is_Playing_Backwards();  

//	ASSERT(   goingBackwardALT  ==   isPlayingBackward   );   ***** FAILS when first starting to go backwards    4/26



	drivingOffMap.Clear(  0  );


	short   travIndex =    m_currentIndexFinalCircque;    //  INIT index to point at the  OLDEST  CalcedNote




	if(   doVertical   )
	{
		short  yWriteRow;    
		short  i =0;


		if(   isPlayingBackward   )             //  for BACKWARDS,   need to traverse the  Virtual-Offmap ( i.e.  FinalNotesCircque )  in reverse direction    2/12
			yWriteRow  =    Calc_Offmaps_DeadZone_WriteRow(   drivingOffMap.m_height    );  
		else
			yWriteRow  =    drivingOffMap.m_height  -1;      //  start at  end of Offmap,  it has the OLDEST notes
	


		while(    i  <  m_sizeOfFinalNotesCircque    )
		{
			CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 


	//		Write_DrivingViews_Y_Row(   calcedNote,    yWriteRow,    m_musicalKey,    drivingOffMap  );

			Write_DrivingViews_Y_Row_RAW(    calcedNote.scalePitch,   calcedNote.detectAvgHarmonicMag,    calcedNote.octaveIndex,  
					                                                                       yWriteRow,    	 m_musicalKey,    drivingOffMap  );


			if(   isPlayingBackward   )
				yWriteRow++;
			else
				yWriteRow--;

			i++;
			travIndex++;    //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes

			if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
				travIndex =   0;
		}
	}     //   end of VERTICAL  drivingView

	else
	{															//   do HORIZONTAL  drivingView		 [    ReDraw_DrivingViews_OffMap_from_FinalCircQues_Array()    ]
		short   xWriteColumn;  


		if(   isPlayingBackward   )             //  for BACKWARDS,   need to traverse the  Virtual-Offmap ( i.e.  FinalNotesCircque )  in reverse direction    2/12
			xWriteColumn =  m_sizeOfFinalNotesCircque  -1;
		else
			xWriteColumn =  0;



		while(          xWriteColumn  >=   0   
			       &&   xWriteColumn   <     m_sizeOfFinalNotesCircque   )
		{
			CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 

			Write_DrivingViews_X_Column_Raw(   calcedNote.scalePitch,   calcedNote.detectAvgHarmonicMag,    calcedNote.octaveIndex,  
					                                                                                                                   xWriteColumn,   m_musicalKey,   drivingOffMap  );

			if(   isPlayingBackward   )
				xWriteColumn--;
			else
				xWriteColumn++;


			travIndex++;    //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes

			if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
				travIndex =   0;
		}
	}      //   end of HORIZONTAL  drivingView
}




											////////////////////////////////////////


short		SPitchCalc::Calc_ColorRuns_Octave_and_DetectScores(   short  scalePitch,    long  startPixel,   long  endPixel,       bool  doVertical,
														                                      short&  retAvgDetectScore,     short&  retAvgHarmMag,    long  retOctaveScores[]   )   							            
{


		//  ***************   INSTALL ?  -   If I RETURNED the SCORES of the 4 Candidates,  then I could write them to the   **********
		//  ***************                        NOTE DETAILS Dialog  for  COLOR-RUN  Selection-Mode       4/2012  ********************


	retAvgDetectScore  =   retAvgHarmMag =  -1; 


	if(   startPixel  >=   endPixel    )
	{
		int  dummy = 9;   //  Only land here if the are EQUAL,  and that seems to be OK ( 2/27/12 ).      2/18/2012
	}



	long   octaveCounters[  4  ];


	for(  short   oct =0;    oct < 4;    oct++   )
	{
		octaveCounters[   oct  ]  =   0;
		retOctaveScores[  oct  ]  =   0;
	}


	long   detectScoreTotal  =  0;
	long   avgHarmonicMag  =  0;


	short   countOfOutsideScalePitches =   0;
	long    countOfInsideIntervalScalePitches =  0;



	if(   doVertical   )
	{

		ASSERT(  m_drivingOffMapVert  );

		short   yWriteRow  =    m_drivingOffMapVert->m_height  -1;      //  start at  end of Offmap,  it has the OLDEST notes

		short   travIndex   =    m_currentIndexFinalCircque;    //  INIT index to point at the  OLDEST  CalcedNote



		for(   short  i =0;      i <  ( m_sizeOfFinalNotesCircque );     i++    )
		{

			CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 

			if(       calcedNote.scalePitch  ==   scalePitch   
				&&  calcedNote.octaveIndex >=  0    )
			{

				if(    yWriteRow  >=  startPixel      &&      yWriteRow <=  endPixel    )
				{

					octaveCounters[   calcedNote.octaveIndex   ]++;

					detectScoreTotal    +=     calcedNote.detectScoreHarms;
					avgHarmonicMag   +=     calcedNote.detectAvgHarmonicMag;

					for(   short oct=0;     oct < 4;     oct++   )
					{
						retOctaveScores[  oct  ]  +=    calcedNote.detectScoreOctaveCandids[  oct  ];
					}


					countOfInsideIntervalScalePitches++;
				}
				else
					countOfOutsideScalePitches++;
			}

			yWriteRow--;

			travIndex++;    //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes

			if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
				travIndex =   0;
		}

	}
	else
	{			//		HORIZONTAL

		short   travIndex        =    m_currentIndexFinalCircque;    //  INIT index to point at the  OLDEST  CalcedNote

		short   xWriteColumn =  0;                 //   INIT above:    travIndex =   m_currentIndexFinalCircque  



		for(   short  i =0;      i <  ( m_sizeOfFinalNotesCircque );     i++    )
		{

			CalcedNote&   calcedNote  =   m_circQueFinalNotes[  travIndex  ];   

			if(       calcedNote.scalePitch  ==   scalePitch   
				&&  calcedNote.octaveIndex >=  0    )
			{

				if(    xWriteColumn  >=  startPixel      &&      xWriteColumn <=  endPixel    )
				{

					octaveCounters[   calcedNote.octaveIndex   ]++;

					detectScoreTotal   +=     calcedNote.detectScoreHarms;
					avgHarmonicMag   +=    calcedNote.detectAvgHarmonicMag;

					for(   short oct=0;     oct < 4;     oct++   )
					{
						retOctaveScores[  oct  ]  +=    calcedNote.detectScoreOctaveCandids[  oct  ];
					}

					countOfInsideIntervalScalePitches++;
				}
				else
					countOfOutsideScalePitches++;
			}

			xWriteColumn++;

			travIndex++;   

			if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
				travIndex =   0;
		}
	}




	short   bestOctaveIdx =  -1;
	long    biggestOctavePopulation =  -1;

	for(   short  oct =0;     oct < 4;    oct++   )
	{		
		if(    octaveCounters[  oct  ]   >   biggestOctavePopulation    )
		{
			bestOctaveIdx                =    oct;
			biggestOctavePopulation  =    octaveCounters[  oct  ];
		}
	}



	if(    countOfInsideIntervalScalePitches  !=  0    )
	{

		retAvgDetectScore =     detectScoreTotal  /  countOfInsideIntervalScalePitches;
		
		retAvgHarmMag     =     avgHarmonicMag  /  countOfInsideIntervalScalePitches;


		for(   short oct=0;     oct < 4;     oct++   )
			retOctaveScores[ oct ]  =     (   retOctaveScores[ oct ]   /  countOfInsideIntervalScalePitches    );

	}


	return  bestOctaveIdx;
}



											////////////////////////////////////////


bool    SPitchCalc::ReDraw_DrivingViews_OffMap_from_NoteList(   bool  isPlayingBackward,   OffMap&  drivingOffMap,   bool  doVertical,   CString&  retErrorMesg  )
{


	// *******  TRICKY...    It DEPENDS on FinalCircQue's  'CalcedNotes'   having the right  TIME values ( beginingSampleIdxFile )  ****************
	//
	//    If the FinCircQue is messed up (from direction change, or change of MIDI-SOURCE Mode?) then need to FIRST call   PRE-ROLL   to CLEANUP the  FinCircQue  
	// 
	//    Could try writing a BETTER version,  one that  ASSUMES the correct TIME Values for each  CalcNote  in the FinalCircQue, but still might not sove all problems    8/2012 
	//
	// **************************************************************************************************************************



	   //    CALLED by:    PsNavigatorDlg::Change_Midi_Source(),       PsNavigatorDlg::Render_DrivingViews_Pane_from_NoteList()      
	   //
	   //                                    SPitchCalc::Delete_Note(),      Replace_Undo_Note_to_NoteList()       On_GoFileStart_Button()


		//   Is very SIMILAR with     SPitchCalc::Cleanup_FinalCircQue_from_NoteList()    and needs to coordinate with that SISTER function.



	retErrorMesg.Empty();

	if(   m_calcedNoteListMasterApp ==  NULL   )
	{
		retErrorMesg =   "SPitchCalc::ReDraw_DrivingViews_OffMap_from_NoteList  FAILED,  noteList  is  NULL" ;
		return  false;
	}


	long   noteCount    =     m_calcedNoteListMasterApp->Count();
	if(      noteCount  <= 0   )
	{
		retErrorMesg =     "Load a Notelist that has some notes."  ;
		return  false;
	}


	short   retScalePitchIdx,  retDetectScore,     retAvgHarmonicMag,   retOctaveIdx,   retSynthCode,   retPixelBoundaryType;   
	long    lastPieSliceIndex =  0;

	MidiNote  *retFoundNote =  NULL;	


	long	  noteListHardwareDelaySamps =  	Get_NoteList_Position_Delay_InSamples();     //   Uses SPEED 1  ( Virtual  SampleINDEX)





	short   travIndex =    m_currentIndexFinalCircque;    //  INIT index to point at the  OLDEST  CalcedNote

	drivingOffMap.Clear(  0  );



	if(    doVertical    )
	{
	
		short  yWriteRow;    


		if(   isPlayingBackward   )
		{
		
			yWriteRow =   Calc_Offmaps_DeadZone_WriteRow(   drivingOffMap.m_height    );    //  start at  beginning of Offmap,  it has the OLDEST notes when in backwards play


			for(   short  i =0;      i <  ( m_sizeOfFinalNotesCircque );     i++    )
			{

				CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 


				if(      (  lastPieSliceIndex + 1 )   !=  calcedNote.pieSliceIdxAtDetection 
					&&   i  !=  0    )
				{
					int  dummy =  9;
//					TRACE(  "\n\n *******  MISSED EVENT %d   [  Ideal %d     Actual %d   ]     ReDraw_DrivingViews_OffMap_from_NoteList() \n",  i,   (lastPieSliceIndex + 1) ,   calcedNote.pieSliceIdxAtDetection    );
				}
				lastPieSliceIndex =   calcedNote.pieSliceIdxAtDetection;



	//			long   targetSampleIdx  =     calcedNote.curSampleWaveman   -       noteListHardwareDelaySamps;   

				long   targetSampleIdx  =     calcedNote.beginingSampleIdxFile;   // ****  WANT this ????   Is like backwards  Horiz case




//  *******   SHOULD I  be testing  that   targetSampleIdx  > 0   ?????   *********************************************************  

				if(    targetSampleIdx   >=  0   )     //  This test was installed on   8/10/2012
				{


					if(    ! Get_Pixel_Info_from_Notelist(   targetSampleIdx,     retScalePitchIdx,   retDetectScore,  retAvgHarmonicMag,   retOctaveIdx,  
																			   m_calcedNoteListMasterApp,  m_calcedNoteListMasterApp->m_lastFoundCalcNoteLink,  
																			   isPlayingBackward,  	retSynthCode,   retPixelBoundaryType,   &retFoundNote,   retErrorMesg  )     )
						return  false;
					///////////////


					if(    retScalePitchIdx >=  0   )		//   need this test for accurate image.  12/6/11
						Write_DrivingViews_Y_Row_RAW(    retScalePitchIdx,  retAvgHarmonicMag,   retOctaveIdx,   yWriteRow,   	m_musicalKey,    drivingOffMap  );		
				//	else	{    }   //  do nothing, leave it black.
				}



				yWriteRow++;

				travIndex++;    //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes

				if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
					travIndex =   0;
			}
	
		}
		else
		{			  //   FORWARD -   For the DrivingView:    the  OLDEST notes are on the BOTTOM,  and the NEWER notes are on the TOP


			yWriteRow =    drivingOffMap.m_height  -1;      //  start at  end of Offmap,  it has the OLDEST notes


			for(   short  i =0;      i <  ( m_sizeOfFinalNotesCircque );     i++    )
			{

				CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 


				if(      (  lastPieSliceIndex + 1 )   !=  calcedNote.pieSliceIdxAtDetection 
					&&   i  !=  0    )
				{
					int  dummy =  9;   //  Landed here   2/18/2012    8/2012  
//					TRACE(  "\n\n ******* %d   MISSED EVENT   [  Ideal %d     Actual %d   ]     ReDraw_DrivingViews_OffMap_from_NoteList()  \n",  i,   (lastPieSliceIndex + 1) ,   calcedNote.pieSliceIdxAtDetection    );
				}
				lastPieSliceIndex =   calcedNote.pieSliceIdxAtDetection;


				long   targetSampleIdx  =     calcedNote.curSampleWaveman   -     noteListHardwareDelaySamps;   




//  *******   SHOULD I  be testing  that   targetSampleIdx  > 0   ?????   *********************************************************  

				if(   targetSampleIdx   >=  0   )     //  This test was installed on   8/10/2012
				{

					if(    ! Get_Pixel_Info_from_Notelist(   targetSampleIdx,     retScalePitchIdx,   retDetectScore,  retAvgHarmonicMag,    retOctaveIdx,  m_calcedNoteListMasterApp,  
																							m_calcedNoteListMasterApp->m_lastFoundCalcNoteLink,   isPlayingBackward,  
																											retSynthCode,    retPixelBoundaryType,   &retFoundNote,   retErrorMesg  )     )
						return  false;



					if(    retScalePitchIdx >=  0   )		//   need this test for accurate image.  12/6/11
						Write_DrivingViews_Y_Row_RAW(    retScalePitchIdx,  retAvgHarmonicMag,   retOctaveIdx,   yWriteRow,   	m_musicalKey,    drivingOffMap  );		
				//	else	{    }   //  do nothing, leave it black.
				}
				

				yWriteRow--;

				travIndex++;    //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes

				if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
					travIndex =   0;
			}
		}		

	}     //   end of VERTICAL  drivingView

	else
	{															//  do HORIZONTAL  drivingView		
		short   xWriteColumn;  


		if(    isPlayingBackward   )
		{

			xWriteColumn  =     m_sizeOfFinalNotesCircque  -1;    //  Now we right into the middle of map.


			while(  xWriteColumn  >=  0 )
			{

				CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 



				if(        (  lastPieSliceIndex + 1 )   !=  calcedNote.pieSliceIdxAtDetection 
					&&   xWriteColumn  !=   ( m_sizeOfFinalNotesCircque  -1)   )
				{

					int  dummy =  9;   //  ******  How often does this get HIT ??  HIT on  2/19/2012

//					TRACE(  "\n\n *******  MISSED EVENT  %d   [  Ideal %d     Actual %d   ]    ReDraw_DrivingViews_OffMap_from_NoteList()  \n",   xWriteColumn,   (lastPieSliceIndex + 1) ,   calcedNote.pieSliceIdxAtDetection    );
				}

				lastPieSliceIndex =   calcedNote.pieSliceIdxAtDetection;



	//			long   targetSampleIdx  =     calcedNote.curSampleWaveman   -       noteListHardwareDelaySamps;   
				long   targetSampleIdx  =     calcedNote.beginingSampleIdxFile;     // *****   WANT it this way ???    8/2012



 //  *******   SHOULD I  be testing  that   targetSampleIdx  > 0   ?????   *********************************************************  

				if(   targetSampleIdx   >=  0   )     //  This test was installed on   8/10/2012
				{


					if(    ! Get_Pixel_Info_from_Notelist(   targetSampleIdx,    retScalePitchIdx,  retDetectScore,    retAvgHarmonicMag,  retOctaveIdx,   m_calcedNoteListMasterApp,  
																						m_calcedNoteListMasterApp->m_lastFoundCalcNoteLink,  isPlayingBackward, 
																																retSynthCode,   retPixelBoundaryType,   &retFoundNote,   retErrorMesg  )    )
						return  false;


					if(    retScalePitchIdx >=  0   )		//   need this test for accurate image.  12/6/11
			//			Write_DrivingViews_X_Column_Raw(   retScalePitchIdx,     retDetectScore,            retOctaveIdx,   xWriteColumn,   m_musicalKey,   drivingOffMap  );
						Write_DrivingViews_X_Column_Raw(   retScalePitchIdx,     retAvgHarmonicMag,     retOctaveIdx,   xWriteColumn,   m_musicalKey,   drivingOffMap  );
				}



				xWriteColumn--;

				travIndex++;    //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes

				if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
					travIndex =   0;
			}

		}
		else
		{                  //   FORWARD -   For the DrivingView:    the  OLDEST notes are on the LEFT,  and the NEWER notes are on the RIGHT

			xWriteColumn =  0;  


			for(   short  i =0;      i <  ( m_sizeOfFinalNotesCircque );     i++    )
			{

				CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 


				if(        (  lastPieSliceIndex + 1 )   !=  calcedNote.pieSliceIdxAtDetection 
					&&     i  !=  0    )
				{
					long  lostNotes =    calcedNote.pieSliceIdxAtDetection  -   (lastPieSliceIndex + 1); 
 

//					TRACE(  "\n\n *******  MISSED EVENT  [ travIndex %d ]    %d  LostNOTES    [  IdealPieIdx %d     .pieSliceIdxAtDetection %d   ]   ReDraw_DrivingViews_OffMap_from_NoteList()   \n",  
//						                            travIndex,    lostNotes,   (lastPieSliceIndex + 1) ,   calcedNote.pieSliceIdxAtDetection    );	

					int  dummy =  9;				
				}


				lastPieSliceIndex =   calcedNote.pieSliceIdxAtDetection;

			

					//   Need to properly ADJUST  'curSampleIdxWierdDelay'   so this syncs with what XORbox shows as position
					//
					//   ***WEIRD, uses  'curSampleWaveman'  instead of  'beginingSampleIdxFile' ,  but this is the same way that Notelist-PLAY happens from  Search_NoteList_for_AudioMusical_Values()   2/12     

				long   targetSampleIdx =     calcedNote.curSampleWaveman   -    noteListHardwareDelaySamps;   





  //  *******   SHOULD I  be testing  that   targetSampleIdx  > 0   ?????   *********************************************************  

				if(   targetSampleIdx   >=  0   )     //  This test was installed on   8/10/2012
				{

					if(    ! Get_Pixel_Info_from_Notelist(   targetSampleIdx,    retScalePitchIdx,  retDetectScore,   retAvgHarmonicMag,   retOctaveIdx,    m_calcedNoteListMasterApp,  
																								  m_calcedNoteListMasterApp->m_lastFoundCalcNoteLink,   isPlayingBackward,  
																												retSynthCode,     retPixelBoundaryType,   &retFoundNote,   retErrorMesg  )    )
						return  false;


					if(    retScalePitchIdx >=  0   )    //   need this test for accurate image.  12/6/11
					{
						Write_DrivingViews_X_Column_Raw(   retScalePitchIdx,     retAvgHarmonicMag,     retOctaveIdx,   xWriteColumn,   m_musicalKey,   drivingOffMap  );
					}

					/****  TOO SLOPPY,  this is now done with   Cleanup_FinalCircQue_from_NoteList()    6/2012
					else
					{	calcedNote.scalePitch  =   -1;    // ************  6/4    NEW,  also update the  CalcNotes cause they are used to render in  Render_DrivingViews_Pane_wFinalCircQue()    ***************** 
						calcedNote.synthCode  =   0;  
					}
					*****/
				}


				
				xWriteColumn++;

				travIndex++;    //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes

				if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
					travIndex =   0;
			}
		}
	}      //   end of HORIZONTAL  drivingView



	return  true;
}





											////////////////////////////////////////


bool    SPitchCalc::Cleanup_FinalCircQue_from_NoteList(   bool  isPlayingBackward,   OffMap&  drivingOffMap,   bool  doVertical,   CString&  retErrorMesg  )
{


		//  NEW  6/2012.   **** POORLY WRITTEN,   this is only used after a DeletNote,  or UndoNote,  to fix the CalcNotes in FinalCircQue.
		//                                                           ...it SHOULD really know how to set the  CalcNote::synthCode  variable,  BUT it can NOT do that yet.



	//   Only CALLED By:      SPitchCalc::Delete_Note()      and      SPitchCalc::Replace_Undo_Note_to_NoteList()


	//   Is very SIMILAR with    SPitchCalc::ReDraw_DrivingViews_OffMap_from_NoteList()    and needs to coordinate with that SISTER function.




	// *******  TRICKY...    It DEPENDS on the  FinCircQue's  CalcedNotes  having the right  TIME ( beginingSampleIdxFile )  values.  ****************
	//
	//  If the FinCircQue is messed up (from direction change?) then need to FIRST call   PRE-ROLL   to CLEANUP the  FinCircQue  
	// 
	// **************************************************************************************************************************



	retErrorMesg.Empty();

	if(   m_calcedNoteListMasterApp ==  NULL   )
	{
		retErrorMesg =   "SPitchCalc::Cleanup_FinalCircQue_from_NoteList  FAILED,  noteList  is  NULL" ;
		return  false;
	}


	long   noteCount    =     m_calcedNoteListMasterApp->Count();
	if(      noteCount  <= 0   )
	{
		retErrorMesg =   "SPitchCalc::Cleanup_FinalCircQue_from_NoteList  FAILED,  NoteList is empty." ;    // ***** NOT an ERROR ????  *********
		return  false;
	}




	short   retScalePitchIdx,  retDetectScore,     retAvgHarmonicMag,   retOctaveIdx,   retSynthCode,   retPixelBoundaryType;   
	long    lastPieSliceIndex =  0;
	MidiNote  *retFoundNote =  NULL;	


	
	long	  noteListHardwareDelayInSamps =  	Get_NoteList_Position_Delay_InSamples();     //   Uses SPEED 1  ( Virtual  SampleINDEX)




	short   travIndex =    m_currentIndexFinalCircque;    //  INIT index to point at the  OLDEST  CalcedNote

	drivingOffMap.Clear(  0  );



	if(    doVertical    )
	{

		short  yWriteRow;    

		if(   isPlayingBackward   )
		{
		
			yWriteRow =   Calc_Offmaps_DeadZone_WriteRow(   drivingOffMap.m_height    );    //  start at  beginning of Offmap,  it has the OLDEST notes when in backwards play


			for(   short  i =0;      i <  ( m_sizeOfFinalNotesCircque );     i++    )
			{
				CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 


				if(      (  lastPieSliceIndex + 1 )   !=  calcedNote.pieSliceIdxAtDetection 
					&&   i  !=  0    )
				{
					int  dummy =  9;
//					TRACE(  "\n\n *******  MISSED EVENT %d   [  Ideal %d     Actual %d   ]    Cleanup_FinalCircQue_from_NoteList()  \n",  i,   (lastPieSliceIndex + 1) ,   calcedNote.pieSliceIdxAtDetection    );
				}
				lastPieSliceIndex =   calcedNote.pieSliceIdxAtDetection;

	
//				long   targetSampleIdx  =     calcedNote.curSampleWaveman   -    (   curSampleIdxWierdDelay  *  samplsInPieSliceSpeedOne   );   

	//			long   targetSampleIdx  =     calcedNote.curSampleWaveman   -          noteListHardwareDelayInSamps;   

				long   targetSampleIdx  =     calcedNote.beginingSampleIdxFile;    //  *******  LIKE Horizonral backwards case ????



				if(    ! Get_Pixel_Info_from_Notelist(   targetSampleIdx,     retScalePitchIdx,   retDetectScore,  retAvgHarmonicMag,   retOctaveIdx,  
					                                                       m_calcedNoteListMasterApp,  m_calcedNoteListMasterApp->m_lastFoundCalcNoteLink,  
																		   isPlayingBackward,  	retSynthCode,   retPixelBoundaryType,   &retFoundNote,   retErrorMesg  )     )
					return  false;
				///////////////



				if(    retScalePitchIdx >=  0   )   //   need this test for accurate image.  12/6/11
				{
			//		Write_DrivingViews_Y_Row_RAW(    retScalePitchIdx,  retDetectScore,   retOctaveIdx,   yWriteRow,   	m_musicalKey,    drivingOffMap  );		

					calcedNote.scalePitch  =   retScalePitchIdx;  
		//         calcedNote.synthCode  =  ???     ***** REALLY should set this,  but this function has limited uses.   6/2012   *************
				}
				else
				{	calcedNote.scalePitch  =   -1;    // ************  6/4    NEW,  also update the  CalcNotes cause they are used to render in  Render_DrivingViews_Pane_wFinalCircQue()    ***************** 
					calcedNote.synthCode  =   0;  
				}


				yWriteRow++;

				travIndex++;    //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes

				if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
					travIndex =   0;
			}

		}
		else
		{			  //   FORWARD -   For the DrivingView:    the  OLDEST notes are on the BOTTOM,  and the NEWER notes are on the TOP

			yWriteRow =    drivingOffMap.m_height  -1;      //  start at  end of Offmap,  it has the OLDEST notes


			for(   short  i =0;      i <  ( m_sizeOfFinalNotesCircque );     i++    )
			{

				CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 


				if(      (  lastPieSliceIndex + 1 )   !=  calcedNote.pieSliceIdxAtDetection 
					&&   i  !=  0    )
				{
					int  dummy =  9;   //  Landed here   2/18,  
//					TRACE(  "\n\n ******* %d   MISSED EVENT   [  Ideal %d     Actual %d   ]    Cleanup_FinalCircQue_from_NoteList()    \n",  i,   (lastPieSliceIndex + 1) ,   calcedNote.pieSliceIdxAtDetection    );
				}
				lastPieSliceIndex =   calcedNote.pieSliceIdxAtDetection;



			//	long   targetSampleIdx  =     calcedNote.curSampleWaveman   -    (   curSampleIdxWierdDelay  *  samplsInPieSliceSpeedOne   );   
				long   targetSampleIdx  =     calcedNote.curSampleWaveman   -          noteListHardwareDelayInSamps;   



				if(    ! Get_Pixel_Info_from_Notelist(   targetSampleIdx,     retScalePitchIdx,   retDetectScore,  retAvgHarmonicMag,    retOctaveIdx,  m_calcedNoteListMasterApp,  
																						m_calcedNoteListMasterApp->m_lastFoundCalcNoteLink,   isPlayingBackward,  
																										retSynthCode,    retPixelBoundaryType,   &retFoundNote,   retErrorMesg  )     )
					return  false;



				if(    retScalePitchIdx >=  0   )   //   need this test for accurate image.  12/6/11
				{
			//	   Write_DrivingViews_Y_Row_RAW(    retScalePitchIdx,  retDetectScore,   retOctaveIdx,   yWriteRow,   	m_musicalKey,    drivingOffMap  );	

				   calcedNote.scalePitch  =   retScalePitchIdx;  
		//         calcedNote.synthCode  =  ???     ***** REALLY should set this,  but this function has limited uses.   6/2012   *************
				}
				else
				{	calcedNote.scalePitch  =   -1;    // ************  6/4    NEW,  also update the  CalcNotes cause they are used to render in  Render_DrivingViews_Pane_wFinalCircQue()    ***************** 
					calcedNote.synthCode  =   0;  
				}
				


				yWriteRow--;

				travIndex++;    //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes

				if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
					travIndex =   0;
			}
		}
		
		
	}     //   end of VERTICAL  drivingView

	else
	{															//  do HORIZONTAL  drivingView		
		short   xWriteColumn;  


		if(    isPlayingBackward   )
		{

			xWriteColumn  =     m_sizeOfFinalNotesCircque  -1;    //  Now we right into the middle of map.

			while(  xWriteColumn  >=  0 )
			{
				CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 


				if(        (  lastPieSliceIndex + 1 )   !=  calcedNote.pieSliceIdxAtDetection 
					&&   xWriteColumn  !=   ( m_sizeOfFinalNotesCircque  -1)   )
				{

					int  dummy =  9;   //  ******  How often does this get HIT ??  HIT on  2/19/2012

//					TRACE(  "\n\n *******  MISSED EVENT  %d   [  Ideal %d     Actual %d   ]    Cleanup_FinalCircQue_from_NoteList()   \n",   xWriteColumn,   (lastPieSliceIndex + 1) ,   calcedNote.pieSliceIdxAtDetection    );
				}

				lastPieSliceIndex =   calcedNote.pieSliceIdxAtDetection;


	//			long   targetSampleIdx  =     calcedNote.curSampleWaveman   -    (   curSampleIdxWierdDelay  *  samplsInPieSliceSpeedOne   );   
				long   targetSampleIdx  =     calcedNote.beginingSampleIdxFile;



				if(    ! Get_Pixel_Info_from_Notelist(   targetSampleIdx,    retScalePitchIdx,  retDetectScore,    retAvgHarmonicMag,  retOctaveIdx,   m_calcedNoteListMasterApp,  
																					m_calcedNoteListMasterApp->m_lastFoundCalcNoteLink,  isPlayingBackward, 
																															retSynthCode,   retPixelBoundaryType,   &retFoundNote,   retErrorMesg  )    )
					return  false;




				if(    retScalePitchIdx >=  0   )   //   need this test for accurate image.  12/6/11
				{
			//		Write_DrivingViews_X_Column_Raw(   retScalePitchIdx,     retAvgHarmonicMag,     retOctaveIdx,   xWriteColumn,   m_musicalKey,   drivingOffMap  );

					calcedNote.scalePitch  =   retScalePitchIdx;  
		//         calcedNote.synthCode  =  ???     ***** REALLY should set this,  but this function has limited uses.   6/2012   *************
				}
				else
				{	calcedNote.scalePitch  =   -1;    // ************  6/4    NEW,  also update the  CalcNotes cause they are used to render in  Render_DrivingViews_Pane_wFinalCircQue()    ***************** 
					calcedNote.synthCode  =   0;  
				}


				xWriteColumn--;

				travIndex++;    //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes

				if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
					travIndex =   0;
			}
			
		}
		else
		{                  //   FORWARD -   For the DrivingView:    the  OLDEST notes are on the LEFT,  and the NEWER notes are on the RIGHT

			xWriteColumn =  0;                //  ********   inside of    Cleanup_FinalCircQue_from_NoteList()


			for(   short  i =0;      i <  ( m_sizeOfFinalNotesCircque );     i++    )
			{

				CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 


				if(        (  lastPieSliceIndex + 1 )   !=  calcedNote.pieSliceIdxAtDetection 
					&&     i  !=  0    )
				{
					long  lostNotes =    calcedNote.pieSliceIdxAtDetection  -   (lastPieSliceIndex + 1); 
 


//					TRACE(  "\n\n *******  MISSED EVENT  [ travIndex %d ]    %d  LostNOTES    [  IdealPieIdx %d     .pieSliceIdxAtDetection %d   ]   Cleanup_FinalCircQue_from_NoteList()   \n",  
//						                            travIndex,    lostNotes,   (lastPieSliceIndex + 1) ,   calcedNote.pieSliceIdxAtDetection    );	


					int  dummy =  9;				
				}


				lastPieSliceIndex =   calcedNote.pieSliceIdxAtDetection;

			

//  **********************  SHOULD I ADJUST  'curSampleIdxWierdDelay'    so this syncs better with what  XORbox shows as position ??? ***************


// *** WEIRD, uses  'curSampleWaveman'  instead of  'beginingSampleIdxFile' ,  but this is the same way that Notelist-PLAY happens from  Search_NoteList_for_AudioMusical_Values()   2/12     


	//			long   targetSampleIdx  =     calcedNote.curSampleWaveman   -    (   curSampleIdxWierdDelay  *  samplsInPieSliceSpeedOne   );   
				long   targetSampleIdx  =     calcedNote.curSampleWaveman   -          noteListHardwareDelayInSamps;   




				if(    ! Get_Pixel_Info_from_Notelist(   targetSampleIdx,    retScalePitchIdx,  retDetectScore,   retAvgHarmonicMag,   retOctaveIdx,    m_calcedNoteListMasterApp,  
																	                          m_calcedNoteListMasterApp->m_lastFoundCalcNoteLink,   isPlayingBackward,  
																											retSynthCode,     retPixelBoundaryType,   &retFoundNote,   retErrorMesg  )    )
					return  false;



				if(    retScalePitchIdx >=  0   )   //   need this test for accurate image.  12/6/11
				{
			//		Write_DrivingViews_X_Column_Raw(   retScalePitchIdx,     retAvgHarmonicMag,     retOctaveIdx,   xWriteColumn,   m_musicalKey,   drivingOffMap  );

					calcedNote.scalePitch  =   retScalePitchIdx;  
		//         calcedNote.synthCode  =  ???     ***** REALLY should set this,  but this function has limited uses.   6/2012   *************
				}
				else
				{	calcedNote.scalePitch  =   -1;    // ************  6/4    NEW,  also update the  CalcNotes cause they are used to render in  Render_DrivingViews_Pane_wFinalCircQue()    ***************** 
					calcedNote.synthCode  =   0;  
				}


				
				xWriteColumn++;

				travIndex++;    //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes

				if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
					travIndex =   0;
			}
		}
	}      //   end of HORIZONTAL  drivingView



	return  true;
}




											////////////////////////////////////////


void	SPitchCalc::Assign_NoteLists_Cache_ListLink(   ListsDoubleLink<MidiNote>  *cacheLink   )
{

	//   This is used in a LIMITED capacity.  2/2012   
	//    Only really needed  during a NoteList search function like  Get_Pixel_Info_from_Notelist()


  //  NOT that NECESSARY anymove, because  Get_Pixel_Info_from_Notelist()  can AUTO-correct the value for the CACHE-Link   1/12 

	//		This is only for NoteLists that PLAY-Audio.   PitchPlayerApp::m_calcedNoteListTemp  is just for Recording and Merging to  PitchPlayerApp::m_calcedNoteListMaster 






	//    CALLED BY:


	//						PsNavigatorDlg::On_Record_Notes()                                **********   OMIT here ???  *********************

					//		PsNavigatorDlg::On_BnClicked_Record_Notes_Button()     **********   OMIT here ???  *********************


				//			PsNavigatorDlg::Change_Midi_Source()

					//		PsNavigatorDlg::Do_PreRoll_Redraw_Display(  

					//		PsNavigatorDlg::On_FileMenu_Empty_Notelist()

					//		EventMan::Process_Event_Notification_PPlayer( 


			//	****   BUT  Have this called after  Merge_NoteLists()   *************************************************




	if(    m_calcedNoteListMasterApp  !=  NULL    )
		m_calcedNoteListMasterApp->m_lastFoundCalcNoteLink  =   cacheLink;
	else
	{
		ASSERT( 0 );    //    int  dummy =  9;  //    Does this get hit ????    1/23/2012
	}
}




											////////////////////////////////////////


bool    SPitchCalc::Get_Pixel_Info_from_Notelist(   long  targetSampleIdx,   short&  retScalePitchIdx,  short&   retDetectScore,   short&  retAvgHarmonicMag, 
																	                short& retOctaveIdx,      ListDoubLinkMemry<MidiNote>  *noteList,    ListsDoubleLink<MidiNote>  *lastFoundCalcNoteLink,
																					bool  isPlayingBackward,   short&  retSynthCode,    	short&  retPixelBoundaryType,   
																				MidiNote  **retFoundNotePtr,	 CString&  retErrorMesg   )
{

	//      Searches the NoteList for a  CalcNote where   'targetSampleIdx'    intersects   {  calcNote.beginingSampleIdxFile,   calcNote.endSampleIdxFile  }


	//      targetSampleIdx  =     currentSampleIdx  -   Get_NoteList_Position_Delay_InSamples() 
	//																										
	//     .beginingSampleIdxFile   is  ALMOST the same    as   'targetSampleIdx'



	//	  CALLED BY:    SPitchCalc::Search_NoteList_for_AudioMusical_Values() ,        SPitchCalc::Search_NoteList_For_MidiNote()   
	//
	//                                     SPitchCalc::ReDraw_DrivingViews_OffMap_from_NoteList()       SPitchCalc::Cleanup_FinalCircQue_from_NoteList() ,   




	//     Now can  AUTOMATICALLY reset the CACHE-Link,  if it is out of position.  ( do NOT have to call  Assign_NoteLists_Cache_ListLink()  anymore to initialize )  1/12 
	//            (   this is the ONLY FUNCTION that should call   Assign_NoteLists_Cache_ListLink()  !!!!!   ) 


	short  maxItersDebug =  2;     //   2[ just shows cacheResets and 1st big search ]      -1[ dumps everything ]     0    4



	retErrorMesg.Empty();

	*retFoundNotePtr =  NULL;


	retScalePitchIdx =    -1;   // INIT  that we would   "return a BLACK Pixel"  (not find a note, which happens often )
	retDetectScore  =     0;
	retAvgHarmonicMag =  0;
	retOctaveIdx =  -4;

	retSynthCode =  -8;
	retPixelBoundaryType =  -8;


	if(   noteList ==  NULL   )
	{
		retErrorMesg =   "SPitchCalc::Get_Pixel_Info_from_Notelist  FAILED,  noteList  is  NULL" ;
		return  false;
	}


	long   noteCount    =     noteList->Count();
	if(      noteCount  <= 0   )
	{
		//   retErrorMesg =   "SPitchCalc::Get_Pixel_Info_from_Notelist  FAILED,  NoteList is empty." ;  // ***** NOT an ERROR ????  *********
		retScalePitchIdx =    -1;
		retDetectScore  =      0;
		retAvgHarmonicMag =  0;
		retOctaveIdx =   -4;

		return  true;   // ***** NOT really an ERROR,  just an empty list .  BUT CALLING function should have checked for this.   *********
	}



	bool	 goingBackwardALT  =     Is_Playing_Backwards();  

//	ASSERT(   goingBackwardALT  ==   isPlayingBackward   );   ***** FAILS in  NoteList Mode,   when first starting to go in reverse  ******





	bool    keepSearching =   true;
	short   noteIdx =  0,    failureCount =  0; 
	long    pieSliceSampsSpeedOne =     Calc_Samples_In_PieEvent(  1 );    //  FAILS if I divide by 2

	ListsDoubleLink<MidiNote>*    startLink = NULL,    *tmpLink =NULL;    

	long   firstNotesSampleIdx   =     (  noteList->First()   ).beginingSampleIdxFile;
	long   lastNotesSampleIdx   =      (  noteList->Last()   ).endingSampleIdxFile;



											//   These test if   'targetSampleIdx'  off the  RANGE{ start, end } of the ENTIRE LIST,  and so would be a BLACK Pixel    2/12


	if(    isPlayingBackward   )      //  Playing  BACKWARDS  
	{																		

		if(     targetSampleIdx  <  firstNotesSampleIdx   )       //  dont bother to search,   targetSampleIdx  is to the LEFT of the FIRST note  ( going backwards )
		{
			tmpLink =    ( ListsDoubleLink<MidiNote>* )(    noteList->Get_Head_Link()    );  

			Assign_NoteLists_Cache_ListLink(   tmpLink );  
	//		TRACE(  "\nOUT of Bounds  HEADlink to Cache. \n"  );
			return   true;
		}

		if(     targetSampleIdx  >   lastNotesSampleIdx    )    //  dont bother to search,   targetSampleIdx  is to the RIGHT of the LAST note  ( going backwards
		{
			tmpLink =    ( ListsDoubleLink<MidiNote>* )(     noteList->Get_Last_Link()    );  

			Assign_NoteLists_Cache_ListLink(   tmpLink );  
	//		TRACE(  "\nOUT of Bounds  LASTlink to Cache. \n"  );
			return   true;
		}
	}
	else                                         //   Playing  FORWARD
	{                                        
		if(     targetSampleIdx  >   lastNotesSampleIdx    )    //  dont bother to search,   targetSampleIdx  is to the RIGHT of the LAST note  ( going forwards
		{
			tmpLink =    ( ListsDoubleLink<MidiNote>* )(     noteList->Get_Last_Link()    );  

			Assign_NoteLists_Cache_ListLink(   tmpLink );  
	//		TRACE(  "\nOUT of Bounds  LASTlink to Cache. \n"  );
			return   true;
		}

		if(     targetSampleIdx  <  firstNotesSampleIdx   )       //  dont bother to search,   targetSampleIdx  is to the LEFT of the FIRST note  ( going forwards )
		{
			tmpLink =    ( ListsDoubleLink<MidiNote>* )(    noteList->Get_Head_Link()    );  

			Assign_NoteLists_Cache_ListLink(   tmpLink ); 
	//		TRACE(  "\nOUT of Bounds  HEADlink to Cache. \n"  );
			return   true;
		}
	}   




	bool    startingFromMidList =  false;        //   These tests will automatically   RESET the CACHE-Link   if it is out of position.    2/12
															

	if(    lastFoundCalcNoteLink  !=  NULL   )
	{
		
		if(   isPlayingBackward   )      //  playing  BACKWARD
		{
			if(    lastFoundCalcNoteLink->_item.endingSampleIdxFile  <  targetSampleIdx   )		//  the CACHE-Link is way to far to the LEFT and would miss our  'targetSampleIdx'  at the start
			{
				Assign_NoteLists_Cache_ListLink(  NULL  ); 
				startingFromMidList =  false;
			}
			else
				startingFromMidList =  true;
		}
		else                                    //  playing  FORWARD
		{                
			if(    lastFoundCalcNoteLink->_item.beginingSampleIdxFile  >  targetSampleIdx    )     //   the CACHE-Link is way to far to the RIGHT and would miss our  'targetSampleIdx'  right off the bat
			{
				Assign_NoteLists_Cache_ListLink(  NULL  ); 
				startingFromMidList =  false;
			}
			else
				startingFromMidList =  true;
		}
	}



	if(     startingFromMidList   )
		startLink =     lastFoundCalcNoteLink;
	else
	{  if(    isPlayingBackward   ) 
			startLink =    ( ListsDoubleLink<MidiNote>* )(     noteList->Get_Last_Link()    );  
		else
			startLink =    ( ListsDoubleLink<MidiNote>* )(     noteList->Get_Head_Link()    );  

	//	TRACE(   "\n\n*******  Cache RESET   [  %d  ]  **************************  \n",   targetSampleIdx    );
	}






	bool    foundNote =  false;
	bool    foundBlackPixel =  false;

	SpeedIndexDoubleIterator<MidiNote>    iter(   *noteList,    startLink  );  



	if(    isPlayingBackward   )      //  Playing  BACKWARDS  
	{																		

		iter.Last();      //   a DUMMY function, it does nothing.    Just keep it here for 'form'.  


		do
		{  MidiNote&   calNote =     iter.Current_Item();


			if(    calNote.beginingSampleIdxFile   <=   targetSampleIdx   )     //   Have we yet found a note,  where its  BEGIN-boundary  is SMALLER that the 'interior'  targetSampleIdx
			{																		          //    (  all the  early notes'   BEGIN-boundaries   will be greater than  targetSampleIdx   )
				if(     calNote.endingSampleIdxFile   >=   targetSampleIdx   )     //    ...and is  the notes  END-boundary is bigger
				{
					retScalePitchIdx =    calNote.scalePitch;
					retOctaveIdx     =     calNote.octaveIndex;
					retDetectScore         =     calNote.detectScoreHarms;             
					retAvgHarmonicMag  =     calNote.detectAvgHarmonicMag;    

					foundNote =   true;

					*retFoundNotePtr =   &calNote;


					Assign_NoteLists_Cache_ListLink(   iter.m_latterLinkCache  );     //  Tested, need to do it this way, or the first gap of Black Pixels after the Note will cause a CACHE-RESET  2/1/2012
				//	Assign_NoteLists_Cache_ListLink(   iter.m_currentLink  );   FAILS 



					if(            targetSampleIdx   >    (  calNote.endingSampleIdxFile    -   pieSliceSampsSpeedOne  )   //  **********  TEST that this SLOPPY Fix does not get called multiple times for SAME NOTE  
						   &&    targetSampleIdx   <    (  calNote.endingSampleIdxFile   +   pieSliceSampsSpeedOne  )     )  			
					{
						//  ***** NEVER gets hit if I use HALF of PieSlice Samples,   need whole size 
						//    [ DEBUG by set break points for  'retPixelBoundaryType'  at  1 & 2.   Notice how they ALTERNATE to always find the Begin and End, consecutively.  12/11

						retPixelBoundaryType  =  1;   //   1:  BEGIN  Note Boundary
						retSynthCode =   1;     //   1:  Turn ON Synth 
					}
				//	else if(    calNote.beginingSampleIdxFile  ==     targetSampleIdx    )   ***** FAILS,  never gets hit.
					else if(     targetSampleIdx   >    (  calNote.beginingSampleIdxFile    -   pieSliceSampsSpeedOne  )   //  **********  TEST that this SLOPPY Fix does not get called multiple times for SAME NOTE  
						   &&    targetSampleIdx   <    (  calNote.beginingSampleIdxFile   +   pieSliceSampsSpeedOne  )     )  			
					{	
						retPixelBoundaryType  =  2;   //   2:  END  Note Boundary

						retSynthCode =   0;     //   0:  Turn OFF    
					}
					else
					{   retPixelBoundaryType  =  0;   //   0:  Inside a note,  but NOT on a boundary
						retSynthCode =  -1;     //  -1:  Do nothing
					}


					if(     noteIdx  >  maxItersDebug   )
					{  //   TRACE(   "\nFound a         NOTE in  %d  iterations.   [  %d  ]  \n ",   noteIdx,   targetSampleIdx   );   
					}
					
					break;
				}
			}

																				 //		A)    Have we GONE too FAR?  (  Test if  'targetSampleIdx'  is way AFTER  the LAST NOTE in list
  
			if(    calNote.endingSampleIdxFile   <  targetSampleIdx    )    //  With this NOTE, we have passed ALL possible locations -- all the remaining Notes happen too EARLY in time for  'targetSampleIdx'  
			{														                         //    ...and so we QUIT the search.  Get here on FileSlider move,  as it resets the CACHElink.  2/12

				foundBlackPixel =   true;

				Assign_NoteLists_Cache_ListLink(   iter.m_latterLinkCache  );   //  do NOT use   iter.m_currentLink !!!   Tested and it generates MANY Cache-RESETS   2/2/2012


			//	TRACE(   "TOO FAR                      Found a  BLACK-Void  in  %d  iterations.  \n ",   noteIdx  );
				if(        noteIdx  >  maxItersDebug  )
				{   // TRACE(   "\nFound a BLACK Pixel in  %d  iterations.  [  %d  ]  \n ",   noteIdx,   targetSampleIdx  );   
				}

				break;
			}
					


			iter.Previous();     

			noteIdx++;

			if(    iter.Is_Done()    ) 
				keepSearching  =   false;

		}  while(   keepSearching    );


	}   //  backwards

	else										 //    Going  FORWARD
	{	

		iter.First();      //   a DUMMY function, it does nothing.    Just keep it here for 'form'.  


		do
		{  MidiNote&   calNote =     iter.Current_Item();


			if(    calNote.endingSampleIdxFile   >=   targetSampleIdx   )     //   Have we yet found a note,  where its  END-boundary  is GREATER that the 'interior'  targetSampleIdx
			{																		           //    (  all the  early notes'   END-boundaries   will be smaller than  targetSampleIdx   )
				if(     calNote.beginingSampleIdxFile   <=   targetSampleIdx   )     //    ...and is  the notes BEGIN-boundary smaller
				{
					retScalePitchIdx        =    calNote.scalePitch;
					retOctaveIdx            =     calNote.octaveIndex;
					retDetectScore         =     calNote.detectScoreHarms;     
					retAvgHarmonicMag  =     calNote.detectAvgHarmonicMag;    


					foundNote =   true;

					*retFoundNotePtr =   &calNote;
																									  //  the next search will NOT have to start at the BEGINNING of the list, but with this link

					Assign_NoteLists_Cache_ListLink(   iter.m_previousLinkCache  );   //  Tested, need to do it this way, or the first gap of Black Pixels  after the Note will cause a CACHE-RESET  2/1/2012
			//		Assign_NoteLists_Cache_ListLink(   iter.m_currentLink  );    FAILS



					if(     calNote.beginingSampleIdxFile   ==     targetSampleIdx    ) 
					{
						retPixelBoundaryType  =  1;   //   1:  BEGIN  Note Boundary
						retSynthCode =   1;     //   1:  Turn ON Synth 
					}
	//				else if(     calNote.endingSampleIdxFile   ==     targetSampleIdx   )   **************   BAD,  never gets hit.  Need better detection   ****************
					else if(          targetSampleIdx   >    (  calNote.endingSampleIdxFile    -   pieSliceSampsSpeedOne   )  
						        &&    targetSampleIdx   <    (  calNote.endingSampleIdxFile   +   pieSliceSampsSpeedOne   )   						
						     )                 //  **********  TEST that this SLOPPY Fix does not get called multiple times for SAME NOTE  
					{	retPixelBoundaryType  =  2;   //   2:  END  Note Boundary

						//  ***** NEVER gets hit if I use half of PieSlice Samples,   need whole size 
						//    [ DEBUG by break points for  'retPixelBoundaryType'  at  1 & 2.   Notice how they alternate to always find the end.  12/11
						retSynthCode =   0;     //   0:  Turn OFF    
					}
					else
					{  retPixelBoundaryType  =  0;   //   0:  Inside a note,  but NOT on a boundary
						retSynthCode =  -1;     //  -1:  Do nothing
					}


				//	TRACE(   "Found a Note in  %d  iterations.  \n ",   noteIdx  );
					if(     noteIdx  >  maxItersDebug   )
					{   //  TRACE(   "\nFound a         NOTE in  %d  iterations.  [  %d  ]  \n ",   noteIdx,   targetSampleIdx   );   
					}

					break;
				}
			}

																							//	A)   Have we GONE too FAR?  ( Test if  'targetSampleIdx'  is way  BEFORE  the CURRENT  Note in list


			if(     targetSampleIdx  <   calNote.beginingSampleIdxFile    )    //  With this NOTE, we have passed ALL possible locations --  all the remaining Notes happen too late in time for  'targetSampleIdx'  
			{																				   //  ...and so we  QUIT the search.  Get here on FileSlider move,  as it resets the CACHElink.  2/12

				foundBlackPixel =   true;
																							  //  the next search will NOT have to start at the BEGINNING of the list, but with this link:

				Assign_NoteLists_Cache_ListLink(   iter.m_previousLinkCache  );   //  do NOT use   iter.m_currentLink !!!   Tested and it generates MANY Cache-RESETS   2/2/2012


			//	TRACE(   "TOO FAR                      Found a  BLACK-Void  in  %d  iterations.  \n ",   noteIdx  );
				if(        noteIdx  >  maxItersDebug  )
				{  //  TRACE(   "\nFound a BLACK Pixel in  %d  iterations [  %d  ].  \n ",   noteIdx,   targetSampleIdx   );   
				}

				break;
			}


			iter.Next();   

			noteIdx++;
	

			if(    iter.Is_Done()    ) 
				keepSearching  =   false;

		}  while(   keepSearching   );

	}   //   if(   Going  FORWARD





	if(        ! foundNote  
		&&   ! foundBlackPixel   )
	{
		failureCount++;        		
		ASSERT( 0 );    //   Does   this get HIT ???    2/1/2012

		TRACE(   "\n\n\n************  List  SEARCH   FAILED  [  %d  times  ]      (Get_Pixel_Info_from_Notelist )   ****************  \n\n",   failureCount   );			
	}

	return   true;
}



											////////////////////////////////////////


short    SPitchCalc::Calc_Offmaps_DeadZone_WriteRow(  short   mapsDimension   )
{

//	short   YwriteRow  =  drivingOffMap->m_height    -    m_sizeOfFinalNotesCircque; 

	short    yWriteRow    =     mapsDimension   -    m_sizeOfFinalNotesCircque; 
	return   yWriteRow;
}






											////////////////////////////////////////

/*****
CalcedNote*    SPitchCalc::Find_DrivingViews_CalcedNote_in_FinCircQue(   PointLong  pt,    long&  retIndexToFinCircque,   bool  isPlayingBackward   )
{


		// ******************  AVOID USING this function if possible,  still a little buggy....  4/2012  **************************



		// **********   Function is  DISORGANIZE  but works most of the time(?),  CAN/SHOULD  I fix it ???  4/25/2012 )  **********************************
		//
		//                       (  Actually it  FAILED  in  PsNavigatorDlg::Calc_ColorRuns_BoundBox()..  'isPlayingBackward'  created problems for 
		//                          render after Pre_Roll()   ***************************************************
		//
		// **********************************************************************************************************************



//  CALLED by:      PsNavigatorDlg::Make_Note_Selection_On_DrivingView()    	SPitchCalc::Find_FinalCircQues_ScalePitch_Run()


	CString   retErrorMesg;

	CalcedNote  *foundNoteInCircQue =   NULL; 
	retIndexToFinCircque =  -2;


	OffMap    *drivingOffMap =  NULL;
	short   rd, gr,  bl; 
	short   travIndex;
	long    pixelsRGBmouseHit =  -4;



	if(   m_doVerticalDrivingview   )      //  VERTICAL
	{

		drivingOffMap =   m_drivingOffMapVert;

		long   scalePitchPosition =      pt.X; 


		short  scalePitchReal  =     scalePitchPosition  +   m_musicalKey;    //  this is the INVERSE of   Get_ScalePitchs_MusicalKey_Transposed_Position() 															
		if(   scalePitchReal  >=  12   )
			scalePitchReal  =    scalePitchReal   -12;      //   Get in  { 0 - 11 }   range  			
		ASSERT(   scalePitchReal >= 0    &&    scalePitchReal <=  11   );



		long   yInverted  =     ( drivingOffMap->m_height -1)    -  pt.Y;     //   355;

		ASSERT(  yInverted  >=  0    &&    yInverted  < drivingOffMap->m_height   );



		drivingOffMap->Read_Pixel(    pt.X,  yInverted,    &rd, &gr, &bl   ); 

		pixelsRGBmouseHit =   RGBjm(  rd, gr, bl  );       // 	TRACE(   "\nSPitchCalc:   Mouse COORDS are  [  %d,  %d  ]    ColorRGB  {  %d,  %d,  %d  }    \n" ,      pt.X,  yInverted,   rd,  gr,  bl    );


		short  notesPixelCount =  0;
		short  lastYwithSameSPitch =  -1;


		short   yRead  =   0;  
		travIndex       =    m_currentIndexFinalCircque;     //   INIT index to point at the  OLDEST  CalcedNote ... so we will see CalcNotes with INCREASING TimeStamps 



		if(   isPlayingBackward   )             //  for BACKWARDS,   need to traverse the  Virtual-Offmap ( i.e.  FinalNotesCircque )  in reverse direction    2/12
			yRead  =   m_sizeOfFinalNotesCircque  -1;
		else
			yRead  =   0;


		while(          yRead  >=   0   
			       &&   yRead   <     m_sizeOfFinalNotesCircque   )
		{
			CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 


			if(    scalePitchReal   ==   calcedNote.scalePitch   )  
			{
				if(    yRead   ==   pt.Y    )
				{
					foundNoteInCircQue =    &calcedNote;
					retIndexToFinCircque  =   travIndex;
					break; 
				}
			}


			if(   isPlayingBackward   )
				yRead--;
			else
				yRead++;


			travIndex++;    //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes

			if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
				travIndex =   0;
		}



		
		if(    foundNoteInCircQue  ==   NULL   )      //   and of courcse if the user clicked on Black
		{
			//		Only happens when in Detection Mode,  that is because ANY  MouseHit search first check the FinalCirc Que  ( but if 
			//		I hit the NoteList Render Button,  the  FinCrcQue is NOT updated ...  and there is less there than

			//   TRACE(  "\n\n****  Could NOT FIND  CalcNote      [ Find_DrivingViews_CalcedNote_in_FinCircQue  VERTICAL  ]  \n\n"    );
			int  dummy =   9;
		}






// *******************************************************************************************************************
// *******  A capable EQUATION  'should'  be able to REPLACE the above SEARCH ???    2/5/12   ************************************
// *******************************************************************************************************************

		long   travIndexAlt;     		
		long	 tempIdx  =    pt.Y    +    m_currentIndexFinalCircque;


		if(     tempIdx   >=   m_sizeOfFinalNotesCircque   )        //  No index to the FinCircQue should be EQUAL to  m_sizeOfFinalNotesCircque
			travIndexAlt =   tempIdx  -   m_sizeOfFinalNotesCircque;
		else
			travIndexAlt =   tempIdx;

		ASSERT(  travIndexAlt  >=  0     &&    travIndexAlt  <  m_sizeOfFinalNotesCircque  );



		CalcedNote  *foundNoteAlternative =  NULL;

		if(    pixelsRGBmouseHit  >  0  )
		{
			foundNoteAlternative  =     &(    m_circQueFinalNotes[   travIndexAlt   ]    ); 


			if(     foundNoteAlternative->scalePitch  !=    scalePitchReal   )
			{	
			//	ASSERT( 0 );     //    2/17/ 12    Hit this a lot when in   Detection MidiSRC Mode
				int   dummy =  9;   //  Now land here when Select note ( NO Notelist  ...DetectionMode  )  after BACKWARDS  play    2/19/2012
			}  
		}


		if(        travIndexAlt    !=   travIndex  
			&&   pixelsRGBmouseHit  >  0     )
		{  
	//		ASSERT( 0 );     //  ****** Land HERE again ????    2/5/2012  ********		
			int   dummy =  9;    //  (from ABOVE bug )  Now land here when Select note ( NO Notelist  ...DetectionMode  )  after BACKWARDS  play    2/19/2012		
		}  

							

	}  //  vertical

	else
	{			                                    //   using  HORIZONTAL format            [      Find_DrivingViews_CalcedNote_in_FinCircQue()

		drivingOffMap =   m_drivingOffMapHorz;


		long    scalePitchPosition =    11  -   pt.Y;    //  must invert for  MS bitmap  {  0 - 11  }

		short   scalePitchReal  =     scalePitchPosition  +   m_musicalKey;    //  this is the INVERSE of   Get_ScalePitchs_MusicalKey_Transposed_Position() algo 															
		if(   scalePitchReal  >=  12   )
			scalePitchReal  =     scalePitchReal   -12;      //   Get in  { 0 - 11 }   range  			
		ASSERT(   scalePitchReal >= 0    &&    scalePitchReal <=  11   );



		drivingOffMap->Read_Pixel(   pt.X,  pt.Y,   &rd,  &gr,  &bl  ); 

		pixelsRGBmouseHit =     RGBjm(  rd,  gr,  bl   );       //	TRACE(   "\nSPitchCalc:   Mouse COORDS are  [  %d,  %d  ]    ColorRGB  {  %d,  %d,  %d  }    \n" ,    x, y,    rd,  gr,  bl    );


																	//   SEARCH  the FinalCircQue to find if the   MouseHit-Point's CalcedNote   has the same 'ScalePitch value'
																	//                 as the ScalePitch CHANNEL  who was selected by the  MouseHit-Point's  Y-coord.

		//  Iterates  X-values,  starting from the LEFT Side of the   "Virtual-Offmap( FinCircQue )",  fetching the associated  CalcedNote/Pixel for each X-position,
		//    and TESTS for the correct  ScalePitch Value  when  the  "INPUT Point's X-coord"  is the same as the X-value of the Virtual-Offmap( FinCircQue ), 


		//  Could also have been done by READING the   Physical-Offmap( m_drivingOffMapHorz )  for  a ScalPitch  HUE ( a GOOD SyncTest )
		//																( actually if only in Notelist-MidiSRC mode,  then Briteness does NOT vary for pixels in run )


		short  xRead  =  0;  
		travIndex   =    m_currentIndexFinalCircque;     //   INIT index to point at the  OLDEST  CalcedNote


		if(   isPlayingBackward   )             //  for BACKWARDS,   need to traverse the  Virtual-Offmap ( i.e.  FinalNotesCircque )  in reverse direction    2/12
			xRead =  m_sizeOfFinalNotesCircque  -1;
		else
			xRead =  0;



		while(          xRead  >=   0   
			       &&   xRead   <     m_sizeOfFinalNotesCircque   )
		{
			CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 


			if(   scalePitchReal  ==   calcedNote.scalePitch    )  
			{
				if(    xRead  ==   pt.X   )
				{
					foundNoteInCircQue   =    &calcedNote;
					retIndexToFinCircque  =   travIndex;
					break; 
				}
			}


			if(   isPlayingBackward   )
				xRead--;
			else
				xRead++;


			travIndex++;    //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes

			if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
				travIndex =   0;
		}



		if(    foundNoteInCircQue  ==   NULL   )    //   and of courcse if the user clicked on Black
		{
				//		Only happens when in Detection Mode,  that is because ANY  MouseHit search first check the FinalCirc Que  ( but if 
			    //		I hit the NoteList Render Button,  the  FinCrcQue is NOT updated ...  and there is less there than

			TRACE(  "\n\n****  Could NOT FIND  CalcNote      [ Find_DrivingViews_CalcedNote_in_FinCircQue  HORIZONTAL  ]  \n\n"    );
			int  dummy =   9;
		}




// *******************************************************************************************************************
// *******  A capable EQUATION  'should'  be able to REPLACE the above SEARCH ???    2/5/2012   ************************************
// *******************************************************************************************************************

		long   travIndexAlt;     		
		long	 tempIdx  =    pt.X    +    m_currentIndexFinalCircque;


		if(     tempIdx   >=   m_sizeOfFinalNotesCircque   )        //  No index to the FinCircQue should be EQUAL to  m_sizeOfFinalNotesCircque
			travIndexAlt =   tempIdx  -   m_sizeOfFinalNotesCircque;
		else
			travIndexAlt =   tempIdx;

		ASSERT(  travIndexAlt  >=  0     &&    travIndexAlt  <  m_sizeOfFinalNotesCircque  );



		CalcedNote  *foundNoteAlternative =  NULL;


		if(    pixelsRGBmouseHit  >  0  )
		{
			foundNoteAlternative  =     &(    m_circQueFinalNotes[   travIndexAlt   ]    ); 

			if(    foundNoteAlternative->scalePitch  !=    scalePitchReal   )
			{	
				
			//	ASSERT( 0 );     // ***************   LAND her a lot with new Note-Stuffing  2/10/12  *****************************
				int    dummy =  9;    //  Land here for  BACKWARDS play, and then select  2/19/12			
			}  
		}


		if(        travIndexAlt    !=   travIndex  
			&&   pixelsRGBmouseHit  >  0     )
		{  			
		//	ASSERT( 0 );  
			int    dummy =  9;   // ***************  Also  LAND her a lot with new Note-Stuffing  2/10/12  *****************************		
		}  

	}   //  Horizontal  


	return  foundNoteInCircQue;
}
*****/


											////////////////////////////////////////


bool    SPitchCalc::Is_Playing_Backwards()
{


		//  ***********   FAILS sometimes if the user is moving the  DETAIL SLIDER   6/16/2012  *****************


	bool             goingBackwards;
	CalcedNote   retCalcNoteFirst,   retCalcNoteLast;


	Get_Newest_Note_from_Final_CircularQue(   retCalcNoteFirst  );

	Get_Oldest_Note_from_Final_CircularQue(    retCalcNoteLast   );   



	if(        retCalcNoteLast.beginingSampleIdxFile   <  0    // **** Sometimes at  songs START, these values are unassigned.  4/26  *****************
		  ||   retCalcNoteFirst.beginingSampleIdxFile  < 0    )
	{
		return  false;
	}



	if(      retCalcNoteLast.beginingSampleIdxFile   >      retCalcNoteFirst.beginingSampleIdxFile     ) 
	{		
		goingBackwards =   true;
	}
	else
		goingBackwards =   false;


	return  goingBackwards;
}



											////////////////////////////////////////


CalcedNote*    SPitchCalc::Find_DrivingViews_CalcedNote_in_FinCircQue_NEW(   PointLong  ptOffmap,    long&  retIndexToFinCircque,   short  backwardsCode   )
{

		//  if  ( 'backwardsCode'  >=  0 )    then it OVERIDES  Is_Playing_Backwards(),  which is not that accurate ( File Slider when in reverse  )  
		//  if  (  backwardsCode  <  0    )    then it is ignore and the function is used  )


//  CALLED by:    PsNavigatorDlg::Make_Note_Selection_On_DrivingView(),   SPitchCalc::Find_DrivingViews_ColorRun(),   SPitchCalc::Find_FinalCircQues_ScalePitch_Run()


	CString   retErrorMesg;
	retIndexToFinCircque  =  -2;
	CalcedNote  *retFoundNoteInCircQue =    NULL; 

	OffMap  *drivingOffMap =  NULL;
	short     rd, gr,  bl; 
	long      pixelsRGBmouseHit =  -4;



	bool    backwards =   false;  

	if(   backwardsCode  >=  0   )     
	{
		if(         backwardsCode  ==   0    )
			backwards =  false;
		else if(   backwardsCode  ==   1    )
			backwards =  true;
		else
		{	ASSERT( 0 );
			backwards =  false;
		}
	}
	else
		  backwards =    Is_Playing_Backwards();   




	long   mapsRelevantWidth  =    m_sizeOfFinalNotesCircque;



	if(     m_doVerticalDrivingview     )      //    VERTICAL
	{

		drivingOffMap =   m_drivingOffMapVert;

		long   scalePitchPosition =     ptOffmap.X; 


		short  scalePitchReal  =     scalePitchPosition  +   m_musicalKey;    //  this is the INVERSE of   Get_ScalePitchs_MusicalKey_Transposed_Position() 															
		if(   scalePitchReal  >=  12   )
			scalePitchReal  =    scalePitchReal   -12;      //   Get in  { 0 - 11 }   range  			
		ASSERT(   scalePitchReal >= 0    &&    scalePitchReal <=  11   );


		long   indexCalcNote,   tempIdx;    
		long   yInverted  =     ( drivingOffMap->m_height -1)    -  ptOffmap.Y;   

		ASSERT(  yInverted  >=  0    &&    yInverted  < drivingOffMap->m_height   );




		drivingOffMap->Read_Pixel(    ptOffmap.X,  yInverted,    &rd, &gr, &bl   ); 
		pixelsRGBmouseHit =   RGBjm(  rd, gr, bl  );       // 	TRACE(   "\nSPitchCalc:   Mouse COORDS are  [  %d,  %d  ]    ColorRGB  {  %d,  %d,  %d  }    \n" ,      ptOffmap.X,  yInverted,   rd,  gr,  bl    );


		if(     pixelsRGBmouseHit   <=  0   )    // Did user hit a BLACK PIXEL  ?????
			return  NULL;  



		

		if(    backwards   )         //  VERTICAL
		{
			long  distRear  =    mapsRelevantWidth  -   ptOffmap.Y;  

			tempIdx =    m_currentIndexFinalCircque   +   distRear;     //   always  ADVANCE(offset)   the  FinCircQue in the  POSITIVE direction

			if(     tempIdx   >=   m_sizeOfFinalNotesCircque   )        //  No index to the FinCircQue should be EQUAL to  m_sizeOfFinalNotesCircque
				indexCalcNote =   tempIdx  -   m_sizeOfFinalNotesCircque;
			else
				indexCalcNote =   tempIdx;
		}
		else
		{  tempIdx =    ptOffmap.Y   +   m_currentIndexFinalCircque;

			if(    tempIdx  >=   m_sizeOfFinalNotesCircque   )        //  No index to the FinCircQue should be EQUAL to  m_sizeOfFinalNotesCircque
				indexCalcNote =   tempIdx  -   m_sizeOfFinalNotesCircque;
			else
				indexCalcNote =   tempIdx;
		}

		ASSERT(  indexCalcNote  >=  0     &&    indexCalcNote  <  m_sizeOfFinalNotesCircque  );




		retFoundNoteInCircQue  =     &(    m_circQueFinalNotes[   indexCalcNote   ]    ); 
		retIndexToFinCircque    =      indexCalcNote;


		if(     retFoundNoteInCircQue->scalePitch   !=   scalePitchReal   )             //  *** may have to do a slight CORRECTION,  Usually the right CalcNote is only one away ****
		{

			int   dummy =  9;          //  Land time LAND here since 5/11/12 :     ( but NOT a big deal, cause can recover down below  )


			if(        ( indexCalcNote -1 )  >=  0   
				&&    m_circQueFinalNotes[    ( indexCalcNote -1 )   ].scalePitch  ==    scalePitchReal   ) 
			{

				retFoundNoteInCircQue =     &(    m_circQueFinalNotes[  ( indexCalcNote -1 )   ]    ); 
				retIndexToFinCircque    =      indexCalcNote -1;
			}
			else if(        ( indexCalcNote +1 )  <  m_sizeOfFinalNotesCircque
				       &&    m_circQueFinalNotes[    ( indexCalcNote +1 )   ].scalePitch  ==    scalePitchReal   ) 
			{

				retFoundNoteInCircQue   =     &(    m_circQueFinalNotes[  ( indexCalcNote +1 )   ]    );         // Landed here sucessfully:  5/7
				retIndexToFinCircque =      indexCalcNote +1;
			}
			else
			{	int  dummy =   9;  }  //   ASSERT( 0 );     ***** HITS here since  5/7/2012 :    6/4/12[ switch to NotelistMode from Detection)			
		}  
	}  //  vertical

	else				          //   HORIZONTAL format          
	{			                                 
		drivingOffMap =   m_drivingOffMapHorz;


		long    scalePitchPosition =    11  -   ptOffmap.Y;    //  must invert for  MS bitmap  {  0 - 11  }

		short   scalePitchReal  =     scalePitchPosition  +   m_musicalKey;    //  this is the INVERSE of   Get_ScalePitchs_MusicalKey_Transposed_Position() algo 															
		if(   scalePitchReal  >=  12   )
			scalePitchReal  =     scalePitchReal   -12;      //   Get in  { 0 - 11 }   range  			
		ASSERT(   scalePitchReal >= 0    &&    scalePitchReal <=  11   );




		drivingOffMap->Read_Pixel(   ptOffmap.X,  ptOffmap.Y,   &rd,  &gr,  &bl  ); 
		pixelsRGBmouseHit  =     RGBjm(  rd,  gr,  bl   );       //	TRACE(   "\nSPitchCalc:   Mouse COORDS are  [  %d,  %d  ]    ColorRGB  {  %d,  %d,  %d  }    \n" ,    x, y,    rd,  gr,  bl    );


		if(    pixelsRGBmouseHit  <=  0   )    //   Did user hit a BLACK PIXEL  ?????
			return  NULL;  




		long   indexCalcNote,   tempIdx;    
		

		if(    backwards   )               //   HORIZONTAL
		{
			long  distRear  =    mapsRelevantWidth  -   ptOffmap.X;     //   this is the DISTANCE of   {  pt.X  to the maps RIGHT BOUNDARY  }
		
			tempIdx  =   m_currentIndexFinalCircque   +   distRear;     //   always  ADVANCE(offset)   the  FinCircQue in the  POSITIVE direction

			if(     tempIdx   >=   m_sizeOfFinalNotesCircque   )        //  No index to the FinCircQue should be EQUAL to  m_sizeOfFinalNotesCircque
				indexCalcNote =   tempIdx  -   m_sizeOfFinalNotesCircque;
			else
				indexCalcNote =   tempIdx;
		}
		else
		{   tempIdx  =    ptOffmap.X    +    m_currentIndexFinalCircque;

			if(     tempIdx   >=   m_sizeOfFinalNotesCircque   )        //  No index to the FinCircQue should be EQUAL to  m_sizeOfFinalNotesCircque
				indexCalcNote =   tempIdx  -   m_sizeOfFinalNotesCircque;
			else
				indexCalcNote =   tempIdx;
		}

		ASSERT(   indexCalcNote >=  0     &&     indexCalcNote <  m_sizeOfFinalNotesCircque   );




		retFoundNoteInCircQue  =     &(   m_circQueFinalNotes[   indexCalcNote   ]    ); 
		retIndexToFinCircque     =    indexCalcNote;


		if(    retFoundNoteInCircQue->scalePitch  !=    scalePitchReal   )          //  *** may have to do a slight CORRECTION,    Usually the right CalcNote is only one away ****
		{	
				
			int   dummy =  9;          //   ***** HITS here since  5/11/12 :    5/15      6/4[ after UndoNote  ]           ( but NOT a big deal, cause can recover down below  )


			if(        ( indexCalcNote -1 )  >=  0   
				&&    m_circQueFinalNotes[    ( indexCalcNote -1 )   ].scalePitch  ==    scalePitchReal   ) 
			{

				retFoundNoteInCircQue =      &(    m_circQueFinalNotes[  ( indexCalcNote -1 )   ]    ); 
				retIndexToFinCircque    =      indexCalcNote -1;
			}
			else if(        ( indexCalcNote +1 )  <  m_sizeOfFinalNotesCircque
			     &&    m_circQueFinalNotes[    ( indexCalcNote +1 )   ].scalePitch  ==    scalePitchReal   ) 
			{

				retFoundNoteInCircQue   =     &(    m_circQueFinalNotes[  ( indexCalcNote +1 )   ]    ); 
				retIndexToFinCircque      =      indexCalcNote +1;
			}

			else
			{	int  dummy =   9;  }    //  **** HITS here since  5/4/12 :      6/4/12[  switch to NotelistMode from Detection,  but it still seems to work.  )	
// *************************************  Is this a problem or not???    6/15/2012 ????   **********************************************




		}  
	}   //  horizontal  


	return   retFoundNoteInCircQue;
}





											////////////////////////////////////////


bool   SPitchCalc::Find_FinalCircQues_ScalePitch_Run(   PointLong  pt,    long&  retStartFinCirqueIdx,    long&  retEndFinCirqueIdx,    
																						          short&  retScalePitch,    long& retPixelLength,   
															long&  retIntervalBeginSampIdx,      long&  retIntervalEndSampIdx,     bool  isPlayingBackward  )
{



ASSERT( 0 );   //   *** ( Keep just for INFO )    *****NO CALLERS. *****     Do NOT use, it was always a CLUMSEY  function.  5/2012   *******************************




	CString   retErrorMesg;

	retScalePitch =  -2;
	retPixelLength =  -2;
	retStartFinCirqueIdx  =  retEndFinCirqueIdx  =  -3;
	retIntervalBeginSampIdx =  retIntervalEndSampIdx  =  -1;


	CalcedNote  *foundNoteInCircQue =   NULL; 
	OffMap    *drivingOffMap =  NULL;
	short   rd, gr,  bl; 
	short   travIndex;
	long    notesPixelLengthPhysical =  -1;
	short  scalePitchReal =  -2;
	bool   crossedFinCirqueBoundary =  false;

	long    pixelsRGBmouseHit =  -3;

	long    retIndexToFinCircque =  -3;



	bool	 goingBackwardALT  =     Is_Playing_Backwards();  

	ASSERT(   goingBackwardALT  ==   isPlayingBackward   );   //   ????  EVER been hit since 4/27 :     




//	CalcedNote   *calcNoteTarg    =       Find_DrivingViews_CalcedNote_in_FinCircQue(           pt,   retIndexToFinCircque,  isPlayingBackward  );    // *** Sometime a PROBLEM ??  4/2012   *****
	CalcedNote   *calcNoteTarg    =       Find_DrivingViews_CalcedNote_in_FinCircQue_NEW(   pt,   retIndexToFinCircque,   -1   ); 
	if(                 calcNoteTarg == NULL  )
	{  
		return  false;
	}

	ASSERT(   retIndexToFinCircque >=  0     &&     retIndexToFinCircque  <  m_sizeOfFinalNotesCircque  );  




	if(   m_doVerticalDrivingview   )          //     VERTICAL format
	{

		drivingOffMap =   m_drivingOffMapVert;


		long   scalePitchPosition =      pt.X; 

		scalePitchReal  =     scalePitchPosition  +   m_musicalKey;    //  this is the INVERSE of   Get_ScalePitchs_MusicalKey_Transposed_Position() 															
		if(   scalePitchReal  >=  12   )
			scalePitchReal  =    scalePitchReal   -12;      //   Get in  { 0 - 11 }   range  			
		ASSERT(   scalePitchReal >= 0    &&    scalePitchReal <=  11   );

		retScalePitch =     scalePitchReal;


		/***************  Now done by calling funct
		long   driveVwHorzPixelRatio  =  2;                  
		long   paneHeight =   280;
		long   panesY  =    ( 280  -1)   -    pt.Y; 				
		long   pt.Y  =    panesY   /    driveVwHorzPixelRatio; 
		*****/
		long   yInverted  =     ( drivingOffMap->m_height -1)    -  pt.Y;     //   355;

		ASSERT(  yInverted  >=  0    &&    yInverted  < drivingOffMap->m_height   );


		drivingOffMap->Read_Pixel(    pt.X,  yInverted,    &rd, &gr, &bl   ); 

		pixelsRGBmouseHit =     RGBjm(  rd,  gr,  bl  );      // 	TRACE(   "\nSPitchCalc:   Mouse COORDS are  [  %d,  %d  ]    ColorRGB  {  %d,  %d,  %d  }    \n" ,      pt.X,  yInverted,   rd,  gr,  bl    );

	}  //  vertical

	else													 //     HORIZONTAL format
	{			                                  
		drivingOffMap =   m_drivingOffMapHorz;


		long    scalePitchPosition =    11  -   pt.Y;    //  must invert for  MS bitmap

		scalePitchReal  =     scalePitchPosition  +   m_musicalKey;    //  this is the INVERSE of   Get_ScalePitchs_MusicalKey_Transposed_Position() 															
		if(   scalePitchReal  >=  12   )
			scalePitchReal  =     scalePitchReal   -12;      //   Get in  { 0 - 11 }   range  			
		ASSERT(   scalePitchReal >= 0    &&    scalePitchReal <=  11   );

		retScalePitch =     scalePitchReal;


		drivingOffMap->Read_Pixel(   pt.X,  pt.Y,     &rd,  &gr,  &bl   ); 

		pixelsRGBmouseHit =     RGBjm(  rd,  gr,  bl   );       	//	TRACE(   "\nSPitchCalc:   Mouse COORDS are  [  %d,  %d  ]    ColorRGB  {  %d,  %d,  %d  }    \n" ,    x, y,    rd,  gr,  bl    );

	}   //  Horizontal  



														//   A)   Traverse forward to find the END of the Interval's run


	travIndex =    retIndexToFinCircque   +1;   //   INIT index to the  NEXT CalcedNote  after the  MouseHit-CalcedNote

	if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
	{
		travIndex =   0;
		crossedFinCirqueBoundary =  true;
	}


	retEndFinCirqueIdx =    retIndexToFinCircque;    // this will be our answer if no more of same SPitch are found



	for(   short  i =0;     i <  m_sizeOfFinalNotesCircque;      i++    )
	{
		CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 


		if(    scalePitchReal  ==   calcedNote.scalePitch    )           
		{
			retEndFinCirqueIdx        =    travIndex;

			retIntervalEndSampIdx  =   calcedNote.beginingSampleIdxFile;    //   remember,   CalcedNote  is NOT an INTERVAL,  but a precise point in time
		}
		else
		{	break;   }
		


		travIndex++;                        //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes

		if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
		{
			travIndex =   0;
			crossedFinCirqueBoundary =  true;
		}
	}




										//  ****   B)    Now traverse in the REVERSE direction to find the START of the Interval  ****


	travIndex =    retIndexToFinCircque   -1;    //   INIT index to the  PREVIOUS CalcedNote  before the  MouseHit-CalcedNote

	if(     travIndex  <  0   )
	{
		travIndex =    m_sizeOfFinalNotesCircque  -1;
		crossedFinCirqueBoundary =  true;
	}


	retStartFinCirqueIdx =    retIndexToFinCircque;    // this will be our answer if no more of same SPitch are found



	for(   short  i =0;     i <  m_sizeOfFinalNotesCircque;      i++    )
	{
		CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 


		if(    scalePitchReal  ==   calcedNote.scalePitch    )           
		{
			retStartFinCirqueIdx =    travIndex;

			retIntervalBeginSampIdx  =   calcedNote.beginingSampleIdxFile;    //   remember,   CalcedNote  is NOT an INTERVAL,  but a precise point in time
		}
		else
		{	break;   }


		travIndex--;                        //  Need to move   CounterClockwise 

		if(     travIndex   <  0   )
		{
			travIndex =    m_sizeOfFinalNotesCircque  -1;
			crossedFinCirqueBoundary =  true;
		}
	}





	if(    retEndFinCirqueIdx  >  retStartFinCirqueIdx    )
	{
		retPixelLength =   retEndFinCirqueIdx  -   retStartFinCirqueIdx   +1;  //   +1  Inclusive Counting
	}
	else
	{	ASSERT( crossedFinCirqueBoundary  );
				//   If  the INTERVAL  {  retStartFinCirqueIdx,  retEndFinCirqueIdx }  straddles the ARRAY End
			    //   retStartFinCirqueIdx  might be bigger than retEndFinCirqueIdx,  so we adjust to calc the true length.  2/12  

		long  frontLength  =   retEndFinCirqueIdx                     -                    0              +1;      //   +1  Inclusive Counting

		long  backLength  =   (m_sizeOfFinalNotesCircque  -1)   -   retStartFinCirqueIdx   +1;      //   +1  Inclusive Counting

		retPixelLength =    frontLength  +  backLength;
	}



   if(         retStartFinCirqueIdx  >=  0     
	   &&    retEndFinCirqueIdx    >=  0  
	   &&    retPixelLength   >  0   )
	   return  true;
   else
	   return false;
}




											////////////////////////////////////////


bool    SPitchCalc::Find_DrivingViews_ColorRun(   PointLong  pt,  	 long&  retStartPixel,   long&  retEndPixel,     short&  retScalePitch, 
											                                               long&  retBeginSampleIdx,     long&  retEndSampleIdx,    short  backwardsCode   )
{

	//  CALLED by:     Make_Note_Selection_On_DrivingView()     On_BnClicked_Play_Next_Note_Button()    On_BnClicked_Play_Previous_Note_Button()
	//
	//                             Change_Direction_Maintain_Selection()          On_BnClicked_Play_Windows_Notes_Button()                


	CString   retErrorMesg;

	retStartPixel =  retEndPixel  =   -9;
	retScalePitch =  -5;


	OffMap      *drivingOffMap =  NULL;
	short   rd, gr,  bl; 
	long    colorRGB,   notesPixelLengthPhysical =  -1;
	bool    useNonZeroMode =    true;   



//	short  backwardsCode =  0;   //  Positive values cause an overide   (  Now comes in as an INPUT PARM ) 


    bool   isPlayingBackward  =   false; 

	/*******************************   Confusing, I know,  but  See   Make_Note_Selection_On_DrivingView()    5/4/2012
	if(   audioPlayer  !=  NULL  )
	{
		if(    audioPlayer->m_playingBackward    )
		{
			isPlayingBackward  =   true;  
			backwardsCode =  1;                  ***  backwardsCode:    Now comes in as an INPUT PARM.
		}
	}
	***/
	if(    Is_Playing_Backwards()    )
	{
		isPlayingBackward  =   true;  
//		backwardsCode =  1; 
	}



	PointLong   ptStart,    ptEnd;  



	if(   m_doVerticalDrivingview   )
	{

		drivingOffMap =   m_drivingOffMapVert;

		long   scalePitchPosition =    pt.X; 


		short  scalePitchReal  =     scalePitchPosition  +   m_musicalKey;    //  this is the INVERSE of   Get_ScalePitchs_MusicalKey_Transposed_Position() 															
		if(   scalePitchReal  >=  12   )
			scalePitchReal  =    scalePitchReal   -12;      //   Get in  { 0 - 11 }   range  			
		ASSERT(   scalePitchReal >= 0    &&    scalePitchReal <=  11   );


		retScalePitch  =    scalePitchReal;


		/***************  Now done by calling funct
		long   driveVwHorzPixelRatio  =  2;                  
		long   paneHeight =   280;
		long   panesY  =    ( 280  -1)   -    pt.Y; 				
		long   pt.Y  =    panesY   /    driveVwHorzPixelRatio; 
		*****/
		long   yInverted  =     ( drivingOffMap->m_height -1)    -  pt.Y;     //   355;

		ASSERT(  yInverted  >=  0    &&    yInverted  < drivingOffMap->m_height   );


		
		drivingOffMap->Read_Pixel(    pt.X,  yInverted,    &rd, &gr, &bl   ); 
		long  pixelsRGB =    RGBjm(  rd,  gr,  bl  );                   // 	TRACE(   "\nSPitchCalc:   Mouse COORDS are  [  %d,  %d  ]    ColorRGB  {  %d,  %d,  %d  }    \n" ,      pt.X,  yInverted,   rd,  gr,  bl    );



																		//  Get the  Note's  PHYSICAL  PIXEL-Length   from the Offmap ITSELF
		long    retYstart= -1,   retYend = -1;  
			

		colorRGB  =	 drivingOffMap->Find_Current_Interval_By_3Components_Vertical(   yInverted,    retYstart,  retYend,   useNonZeroMode   );   


		if(      retYstart < 0    ||    retYend < 0  
			||   retYend  <  retYstart  )
		{
			int   dummy =   9;	 //  	ASSERT( 0 );   	 // ***  LAST time HIT since  4/26/12 :
			return  false;
		}

		retStartPixel  =     retYstart;
		retEndPixel   =      retYend;


														                      //   Find the  PIXEL's   CalcedNote,   so can get its SampleIdx below

		long    retStartPixelMod= -2,   retEndPixelMod= -2;


		retStartPixelMod  =    ( m_drivingOffMapVert->m_height   -1)    -    retEndPixel;	 // Need to SWAP because of Inverted Bitmap 		
		retEndPixelMod    =    ( m_drivingOffMapVert->m_height   -1)    -   retStartPixel;

		ptStart =   pt;      ptEnd =  pt;      

		if(   isPlayingBackward   )
			ptStart.Y =   retStartPixelMod   +1;   //  +1:    WEIRD, but that is what it needs when it goes backwards    4/26/2012
		else
			ptStart.Y =   retStartPixelMod; 

		ptEnd.Y  =   retEndPixelMod;

	}  //  vertical

	else													  //   using  HORIZONTAL format
	{			                                  
		drivingOffMap =   m_drivingOffMapHorz;


		long    scalePitchPosition =    11  -   pt.Y;    //  must invert for  MS bitmap
		short   scalePitchReal  =     scalePitchPosition  +   m_musicalKey;    //  this is the INVERSE of   Get_ScalePitchs_MusicalKey_Transposed_Position() 															
		if(   scalePitchReal  >=  12   )
			scalePitchReal  =     scalePitchReal   -12;      //   Get in  { 0 - 11 }   range  			
		ASSERT(   scalePitchReal >= 0    &&    scalePitchReal <=  11   );


		retScalePitch =    scalePitchReal;


		drivingOffMap->Read_Pixel(   pt.X,  pt.Y,    &rd,  &gr,  &bl   ); 
		long  pixelsRGB =     RGBjm(  rd,  gr,  bl   );                //	TRACE(   "\nSPitchCalc:   Mouse COORDS are  [  %d,  %d  ]    ColorRGB  {  %d,  %d,  %d  }    \n" ,    x, y,    rd,  gr,  bl    );


																		//  Get the  Note's  PHYSICAL  PIXEL-Length   from the Offmap ITSELF
		long  retXstart= -1,  retXend= -1;  

		colorRGB  =	 drivingOffMap->Find_Current_Interval_By_3Components_Horizontal(   pt.X,    retXstart,   retXend,  useNonZeroMode   );  


		if(      retXstart < 0    ||    retXend < 0  
			||   retXend  <  retXstart  )
		{
			int   dummy =   9;	 //  	ASSERT( 0 );   	 // ***  LAST time HIT since  4/26/12 :
			return  false;
		}

		retStartPixel  =     retXstart;
		retEndPixel   =      retXend;    //	notesPixelLengthPhysical  =     retXend  -  retXstart   +1;   //  +1:    "Inclusive counting" 

														                  
		ptStart =  pt;      ptEnd =  pt;      

		if(    isPlayingBackward   )
			ptStart.X =   retStartPixel   +1;
		else
			ptStart.X =   retStartPixel; 

		ptEnd.X  =   retEndPixel;

	}   //  Horizontal  





	long   retIndexToFinCircqueStart = -1,    retIndexToFinCircqueEnd = -1;



	CalcedNote  *startCalcNote  =   Find_DrivingViews_CalcedNote_in_FinCircQue_NEW(     ptStart,     retIndexToFinCircqueStart,    backwardsCode    );

	if(   startCalcNote  !=  NULL   )
		retBeginSampleIdx  =    startCalcNote->beginingSampleIdxFile;      // need to SAVE this so we could use the INSERT NOTEmenu command.  4/12
	else
	{	int  dummy =  9;    }     //   **** ERROR?? ****     HITS since  5/15/12:     5/15[ after load phrase]    6/3[ hit play notes button ]
										   //    6/3,  6/15[ Play-NextNote Button ]     9/9/12 [ Play-NextNote Button ] 




	CalcedNote  *endCalcNote   =     Find_DrivingViews_CalcedNote_in_FinCircQue_NEW(    ptEnd,       retIndexToFinCircqueEnd,     backwardsCode    );

	if(   endCalcNote  !=  NULL   )
		retEndSampleIdx  =    endCalcNote->beginingSampleIdxFile;     
	else
	{	//  int  dummy =  9;     //  **** ERROR?? ****    HITS since  5/15/12:  
		
		ASSERT( 0 );     //  Assert put here on  9/9/2012,    HITS since: 
	}    



	if(   colorRGB >  0   )
		return  true;
	else
		return  false;
}



											////////////////////////////////////////


bool    SPitchCalc::Find_Next_Backward_ColorRun(    long  startCoord,     PointLong&  retRunsMidpoint,   short&  retScalePitch,   short  musicalKeyControlsValue,   CString&  retErrorMesg  )
{


	ASSERT(    musicalKeyControlsValue  ==    m_musicalKey  );   // ****************   OMIT the PARM???   5/13    *********************


	bool        useNonZeroMode =    true;   
	OffMap  *drivingOffMap =  NULL;



	if(    m_doVerticalDrivingview   )
	{

		drivingOffMap =   m_drivingOffMapVert;		

		long  y =  startCoord;  


		while(   y  <   (drivingOffMap->m_height  -1)   )
		{

			long   retComponentColor,    colorRGB;
			long   retYstart = -1,   retYend = -1;  

			long	 xMax =    	 drivingOffMap->Get_Xcoord_of_Rows_MaxValue_By_3Components(      y,    retComponentColor   );


			if(    retComponentColor  >0    &&    xMax >=0    )
			{

				colorRGB  =	drivingOffMap->Find_Current_Intervals_EndPoints_By_3Components_Vertical(      xMax,  y,   retYstart,  retYend,   useNonZeroMode    );

				if(   colorRGB  > 0  )
				{
					ASSERT(    retYstart  >= 0   &&    retYend >= 0   );    

					long    scalePitchPosition =    xMax;   
					short   scalePitchReal     =     scalePitchPosition  +   musicalKeyControlsValue;    //  this is the INVERSE of   Get_ScalePitchs_MusicalKey_Transposed_Position() 																	
					if(   scalePitchReal  >=  12   )
						scalePitchReal  =     scalePitchReal  - 12;      //   Get in  { 0 - 11 }   range  


					long            midPoint  =     retYstart    +   (retYend - retYstart ) /2;
					PointLong    pt(   xMax,  midPoint   );   

					retScalePitch =   scalePitchReal;        //   RETURN the results
					retRunsMidpoint             =   pt;

					return  true;
				}   
			}   //   if(  retComponentColor  >0 
			else
				y++;     //  keep INCREMENTING  y and trying to find a new ColorRun   ( NEWEST notes are at the TOP of the map...  

		}   //  while(  


		retErrorMesg =    "Find_Next_Backward_ColorRun  could NOT find another ColorRun on this Window's Offmap." ;
		return   false;														// went all the way across the map and could NOT find another color run
	}
	else            
	{
		drivingOffMap =   m_drivingOffMapHorz;

		long  x =  startCoord;    


		while(   x  >=  0   )     
		{

			long   retComponentColor,    colorRGB;
			long   retXstart = -1,   retXend = -1;  

			long   yMax =    drivingOffMap->Get_Ycoord_of_Columns_MaxValue_By_3Components(  x,   retComponentColor   );


			if(    retComponentColor  >0    &&    yMax >=0    )
			{

				colorRGB  =	drivingOffMap->Find_Current_Intervals_EndPoints_By_3Components_Horizontal(  x,  yMax,   retXstart,  retXend,   useNonZeroMode  ); 

				if(   colorRGB  > 0  )
				{
					ASSERT(    retXstart  >= 0   &&    retXend >= 0   );    

					long    scalePitchPosition =    11  -   yMax;    //   must invert for  MS bitmap
					short   scalePitchReal     =     scalePitchPosition  +   musicalKeyControlsValue;    //  this is the INVERSE of   Get_ScalePitchs_MusicalKey_Transposed_Position() 																	
					if(   scalePitchReal  >=  12   )
						scalePitchReal  =     scalePitchReal  - 12;      //   Get in  { 0 - 11 }   range  


					long            midPoint  =    retXstart    +   (retXend - retXstart ) /2;
					PointLong    pt(  midPoint,  yMax  );   

					retScalePitch =   scalePitchReal;        //   RETURN the results
					retRunsMidpoint             =   pt;

					return   true;
				}   
			}   //   if(  retComponentColor  >0 
			else
				x--;     //  keep DECREMENTING  x and trying to find a new ColorRun 

		}   //  while(  x < drivingOffMap->m_width  


		retErrorMesg =    "Find_Next_Backward_ColorRun  could NOT find another ColorRun on this Window's Offmap."  ; 
		return   false;														// went all the way across the map and could NOT find another color run
	}   // horizontal


	return  true;
}



					////////////////////////////////////////


bool    SPitchCalc::Find_Next_Forward_ColorRun(    long  startCoord,     PointLong&  retRunsMidpoint,   short&  retScalePitch,    short  musicalKeyControlsValue,    CString& retErrorMesg  )
{


	//  CALLED by:      PsNavigatorDlg::On_BnClicked_Play_Next_Note_Button() ,       PsNavigatorDlg::On_BnClicked_Play_Windows_Notes_Button()  


	ASSERT(    musicalKeyControlsValue  ==    m_musicalKey  );   // ****************   OMIT the PARM???   5/13    *********************



	retErrorMesg.Empty();

	retScalePitch =  -1;   // init for fail


	bool        useNonZeroMode =    true;   // ***  ADJUST  ?????  *****
	OffMap  *drivingOffMap =  NULL;



	if(    m_doVerticalDrivingview    )
	{

		drivingOffMap =   m_drivingOffMapVert;		

		short  yDeadZoneRow  =    Calc_Offmaps_DeadZone_WriteRow(   drivingOffMap->m_height   );  

		long   y =   startCoord;  



		while(    y  >   yDeadZoneRow    )
		{

			long   retComponentColor,    colorRGB;
			long   retYstart = -1,   retYend = -1;  

			long	 xMax =    	 drivingOffMap->Get_Xcoord_of_Rows_MaxValue_By_3Components(   y,    retComponentColor   );


			if(    retComponentColor  >0    &&    xMax >=0    )
			{

				colorRGB  =	drivingOffMap->Find_Current_Intervals_EndPoints_By_3Components_Vertical(   xMax,  y,    retYstart,  retYend,   useNonZeroMode   );

				if(   colorRGB  > 0  )
				{
					ASSERT(    retYstart  >= 0   &&    retYend >= 0   );    

					long    scalePitchPosition =    xMax;    
					short   scalePitchReal     =     scalePitchPosition  +   musicalKeyControlsValue;    //  this is the INVERSE of   Get_ScalePitchs_MusicalKey_Transposed_Position() 																	
					if(   scalePitchReal  >=  12   )
						scalePitchReal  =     scalePitchReal  - 12;      //   Get in  { 0 - 11 }   range  


					long            midPoint  =     retYstart    +   (retYend - retYstart ) /2;
					PointLong    pt(   xMax,  midPoint   );   


					retScalePitch =   scalePitchReal;        //   RETURN the results
					retRunsMidpoint             =   pt;

					return  true;
				}    //  if(  colorRGB >0  
			}   //   if(  retComponentColor  >0 
			else
				y--;     //  keep DECREMENTING  y and trying to find a new ColorRun   ( OLDEST notes are at the BOTTOM of the map...  

		}   //  while(  


		retErrorMesg =    "Find_Next_Forward_ColorRun  could NOT find another ColorRun on this Window's Offmap." ;
		return   false;														// went all the way across the map and could NOT find another color run
	}
	else
	{
		drivingOffMap =   m_drivingOffMapHorz;

		long  x =  startCoord;   


		while(    x <  drivingOffMap->m_width   )
		{

			long   retComponentColor,    colorRGB;
			long   retXstart = -1,   retXend = -1;  

			long   yMax =    drivingOffMap->Get_Ycoord_of_Columns_MaxValue_By_3Components(  x,   retComponentColor   );


			if(    retComponentColor  >0    &&    yMax >=0    )
			{

				colorRGB  =	drivingOffMap->Find_Current_Intervals_EndPoints_By_3Components_Horizontal(   x,  yMax,     retXstart,  retXend,   useNonZeroMode  ); 

				if(   colorRGB  > 0  )
				{
					ASSERT(    retXstart  >= 0   &&    retXend >= 0   );    

					long    scalePitchPosition =    11  -   yMax;    //   must invert for  MS bitmap
					short   scalePitchReal     =     scalePitchPosition  +   musicalKeyControlsValue;    //  this is the INVERSE of   Get_ScalePitchs_MusicalKey_Transposed_Position() 																	
					if(   scalePitchReal  >=  12   )
						scalePitchReal  =     scalePitchReal  - 12;      //   Get in  { 0 - 11 }   range  


					long            midPoint  =    retXstart    +   (retXend - retXstart ) /2;
					PointLong    pt(  midPoint,  yMax  );   


					retScalePitch =   scalePitchReal;        //   RETURN the results
					retRunsMidpoint             =   pt;

					return  true;
				}    //  if(  colorRGB >0  
			}   //   if(  retComponentColor  >0 
			else
				x++;     //  keep INCREMENTING  x and trying to find a new ColorRun 

		}   //  while(  x < drivingOffMap->m_width  


		retErrorMesg =    "Find_Next_Forward_ColorRun could NOT find another ColorRun on this Window's Offmap." ;
		return   false;														// went all the way across the map and could NOT find another color run
	}   // horizontal
 

	return  true;
}



											////////////////////////////////////////


bool    SPitchCalc::Find_ColorRun_Midpoint_by_SampleIdx(   long  targSampleIdx,    short  scalePitch,     PointLong&  retMidPoint,    bool  doVertical,    CString&  retErrorMesg   )
{

	long   toleranceInPieSlices  =  2;


	retErrorMesg.Empty();

	retMidPoint.X =  -1;
	retMidPoint.Y =  -1;


	if(       targSampleIdx  < 0  
		||   scalePitch          < 0    )
	{
		retErrorMesg =   "Find_ColorRun_by_SampleIdx  FAILED,  targSampleIdx is less than zero. " ;
		return  false;
	}


//	bool        useNonZeroMode =    true;   // ***  ADJUST  ?????  *****

	long   sampleIdxMiddle =  -1;
	short  scalePitchFound =   -1;




	long   sampsInPieSlice  = 		Calc_Samples_In_PieEvent(  m_playSpeedFlt  ); 

	long    toleranceInSamps  =    sampsInPieSlice   *  toleranceInPieSlices; 


	short   xColumnFound =  -1,   yColumnFound =  -1;

	long    travIndex =    m_currentIndexFinalCircque; 




	if(   doVertical   )
	{
		short  yWriteRow;    
		short  i =0;

		ASSERT(   m_drivingOffMapVert   );


	//	if(   isPlayingBackward   )             //  for BACKWARDS,   need to traverse the  Virtual-Offmap ( i.e.  FinalNotesCircque )  in reverse direction    2/12
	//		yWriteRow  =    Calc_Offmaps_DeadZone_WriteRow(   drivingOffMap.m_height    );  
	//	else
			yWriteRow  =    m_drivingOffMapVert->m_height  -1;      //  start at  end of Offmap,  it has the OLDEST notes
	

		while(    i  <  m_sizeOfFinalNotesCircque    )
		{
			CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 


			if(     calcedNote.scalePitch  ==   scalePitch
																				//  Want a range of TOLERANCE   (  remember,   CalcedNote  is NOT an INTERVAL,  but a precise point in time  )      
				&&     calcedNote.beginingSampleIdxFile    >     (  targSampleIdx  -   toleranceInSamps  )      
				&&     calcedNote.beginingSampleIdxFile    <     (  targSampleIdx  +  toleranceInSamps  )     )						
			{

				sampleIdxMiddle  =    calcedNote.beginingSampleIdxFile;

				yColumnFound    =    yWriteRow;
				break;
			}

			yWriteRow--;

			i++;
			travIndex++;    //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes

			if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
				travIndex =   0;
		}

	}     //   end of VERTICAL  drivingView

	else
	{															//   do HORIZONTAL  drivingView		 [    ReDraw_DrivingViews_OffMap_from_FinalCircQues_Array()    ]
		short   xWriteColumn =  0;  



		for(    short  i =0;     i <  m_sizeOfFinalNotesCircque;      i++    )
		{
			CalcedNote&   calcedNote  =    m_circQueFinalNotes[  travIndex  ]; 


			if(     calcedNote.scalePitch  ==   scalePitch
																				//  Want a range of TOLERANCE   (  remember,   CalcedNote  is NOT an INTERVAL,  but a precise point in time  )      
				&&     calcedNote.beginingSampleIdxFile    >     (  targSampleIdx  -   toleranceInSamps  )      
				&&     calcedNote.beginingSampleIdxFile    <     (  targSampleIdx  +  toleranceInSamps  )     )						
			{

				sampleIdxMiddle  =    calcedNote.beginingSampleIdxFile;

				xColumnFound    =    xWriteColumn;
				break;
			}
			

			xWriteColumn++;
			travIndex++;                        //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes

			if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
			{
				travIndex =   0;
		//		crossedFinCirqueBoundary =  true;
			}
		}

	}   //  do horizontal




	if(   sampleIdxMiddle  < 0  )
	{
		retErrorMesg =   "Find_ColorRun_by_SampleIdx  FAILED,  could not find time's CalcNote. " ;
		return  false;
	}



	short   scalePitchPos  =    Get_ScalePitchs_MusicalKey_Transposed_Position(   scalePitch,    m_musicalKey  );





	if(    m_doVerticalDrivingview    )
	{
		
		retMidPoint.X =   scalePitchPos;     


		long   yValInverted =     ( m_drivingOffMapVert->m_height -1)    -  yColumnFound;     //  *** INVERT the  Y- VALUES ****

		retMidPoint.Y =    yValInverted;  
	}
	else
	{  retMidPoint.X =    xColumnFound;


		long   yValInverted =    ( m_drivingOffMapHorz->m_height  -1)   -   scalePitchPos;       //  *** INVERT the  Y- VALUES ****

		retMidPoint.Y =    yValInverted;  
	}

	return  true;
}




											////////////////////////////////////////


MidiNote*     SPitchCalc::Search_NoteList_For_MidiNote(   long   targetSampleIdx   )
{


		//  a GRAPHIC function  for  Note-SELECTION      [   Only CALLED by     Make_Note_Selection_On_DrivingView()


	short         retScalePitchIdx,   retDetectScore,   retAvgHarmonicMag,   retOctaveIdx,   retSynthCode,  retPixelBoundaryType;  	 
	MidiNote   *retFoundNoteInList =   NULL;
	CString   retErrorMesg;
	ListsDoubleLink<MidiNote>   *lastFoundCalcNoteLink =  NULL;

	bool  isPlayingBackward  =    false;     // **** ALWAYS ???   NO,  tricky if the user was just going in


	long  noteListsCount     =      m_calcedNoteListMasterApp->Count();
	if(     noteListsCount  <=  0   )
	{
		return  NULL;   // Want an ERROR Message ????   2/12
	}




	if(   ! Get_Pixel_Info_from_Notelist(   targetSampleIdx,    retScalePitchIdx,  retDetectScore,  retAvgHarmonicMag,  retOctaveIdx,    m_calcedNoteListMasterApp,   
															       lastFoundCalcNoteLink,   isPlayingBackward,  retSynthCode,   retPixelBoundaryType,  &retFoundNoteInList,   retErrorMesg  )   )
	{  
		AfxMessageBox( retErrorMesg );  
	}
	else
	{
		if(    retFoundNoteInList  !=  NULL   )
		{
			//	TRACE(  "\n\n****  LIST Note was found:     ScalePitch %d     [  %d   -  %d  ]   \n" ,     retFoundNoteInList->scalePitch,
			//	                                                         retFoundNoteInList->beginingSampleIdxFile,   retFoundNoteInList->endingSampleIdxFile   );
		}
	}


	return   retFoundNoteInList;
}




											////////////////////////////////////////


void    SPitchCalc::Update_DrivingViews_OffMap_with_Nudge(   CalcedNote&   calcedNote,     bool  isPlayingBackward   )
{

				               //   Uses a   SCROLL function to just nudge   ONE PIXEL Column   the whoe map to the left or right

	
	CString  retErrorMesg;


	short   scalePitch     =    calcedNote.scalePitch;
	short   octaveIndex =    calcedNote.octaveIndex;

	short   detectAvgHarmonicMag   =    calcedNote.detectAvgHarmonicMag;
	short   volumeMidi                     =    calcedNote.detectAvgHarmonicMag;



	OffMap   *drivingOffMap =  NULL;

	if(    m_doVerticalDrivingview    )
		drivingOffMap =   m_drivingOffMapVert;
	else
		drivingOffMap =   m_drivingOffMapHorz;



	if(   drivingOffMap ==  NULL   )
	{
		ASSERT( 0 );
		return;
	}

 
	bool	 goingBackwardALT  =     Is_Playing_Backwards();     // ************   FAILS at program start  ******************

//	ASSERT(   goingBackwardALT  ==   isPlayingBackward   );   // ****  FAILS when STARTING to go in reverse





	long    xWriteColumn,   yWriteRow,   yInverted =  -1; 




	if(     m_doVerticalDrivingview    )
	{

		if(    isPlayingBackward    )
		{

			yWriteRow =    drivingOffMap->m_height  -1;     //  go to very end of map


			drivingOffMap->Scroll_Vertically(   true,    1  );    //  true:  need to scroll UPWARD 

			drivingOffMap->Assign_Yrow(   yWriteRow,    0    );     //   set the LAST column to black in case    scalePitch < 0  
		}
		else    
		{					  //  playing  FORWARD

	//		short   deadZoneHeightSRCmap   =    drivingOffMap->m_height    -    m_sizeOfFinalNotesCircque; 
			short   deadZoneHeightSRCmap   =     Calc_Offmaps_DeadZone_WriteRow(   drivingOffMap->m_height   );

			yWriteRow =    deadZoneHeightSRCmap;    //  the dead zone is at the top of

						//  Must also sync with   PsNavigatorDlg::Scroll_DrivingView()    and   PsNavigatorDlg::Blit_Offscreen_Bitmap_to_DrivingView_Pane() 



			drivingOffMap->Scroll_Vertically(   false,    1  );    //  false:  need to scroll down 
	//	    drivingOffMap->Clear(  255   );    //    temp,   good for  testing/debug    11/11

			drivingOffMap->Assign_Yrow(   yWriteRow,    0    );     //   set the LAST column to black in case    scalePitch < 0  
		}



		if(   scalePitch  <  0   )     //   Must come AFTER the scroll.   Not an error,  just NO detection-note to render,  and so we are done with JUST scrolling the bitmap
			return;



//		Write_DrivingViews_Y_Row(   calcedNote,    yWriteRow,   m_musicalKey,   *drivingOffMap   );  ****OMIT this function ****    //  just draws the pixel to the right ROW and with proper intensity.

		Write_DrivingViews_Y_Row_RAW(   calcedNote.scalePitch,   calcedNote.detectAvgHarmonicMag,   calcedNote.octaveIndex,    yWriteRow,    
																																		m_musicalKey,    *drivingOffMap   );


	}       //   end of  VERTICAL  Driving

	else
	{                                                           //   HORIZONTAL  driving
		if(    isPlayingBackward    )
		{
			xWriteColumn =   0;


			drivingOffMap->Scroll_Horizontally(   false,    1  ); 

			drivingOffMap->Assign_Xcolumn(   xWriteColumn,   0   );    //   set the FIRST column to black
		}
		else
		{  xWriteColumn =     m_sizeOfFinalNotesCircque  -1;    //  NOT to the very end of the map    11/11


			drivingOffMap->Scroll_Horizontally(   true,    1  ); 

			drivingOffMap->Assign_Xcolumn(    xWriteColumn,   0   );    //   set the LAST column to black
		}


		if(   scalePitch  <  0   )     //   Must come AFTER the scroll.   Not an error,  just NO detection-note to render,  and so we are done with JUST scrolling the bitmap
			return;


		Write_DrivingViews_X_Column_Raw(  calcedNote.scalePitch,   calcedNote.detectAvgHarmonicMag,   calcedNote.octaveIndex,   xWriteColumn, 
																																							  m_musicalKey,   *drivingOffMap  );
	}
}


											////////////////////////////////////////


long     SPitchCalc::Calculate_Briteness_Divisor(  long  displayBritnessFactor     )
{

				//   long   m_displayBritnessFactor;    //    5 vals     {  0, 1, 2, 3, 4  }     

	long    britenessFactor =  -1;          //   ( BIGGER is DARKER )  ADJUST to make the bullets brighter.  Changed with the SETTINGS DIALOG.  


	switch(   displayBritnessFactor   )           
	{

		/***
		case  0:		britenessFactor =    160;                             //   Multiply by  .75  to get the next value  ( 120 )
		break;

		case  1:		britenessFactor =    120;        
		break;

		case  2:     britenessFactor =     90;      //  this is the DEFAULT for the  'Settings Dialog'   when the app starts
		break;

		case  3:     britenessFactor =     68;  
		break;

		case  4:		britenessFactor =      50;
		break;
		***/



		case  0:		britenessFactor =    140;                             //   Multiply by  .75  to get the next value  ( 120 )
		break;

		case  1:		britenessFactor =    105;        
		break;

		case  2:     britenessFactor =     78;      //  this is the DEFAULT for the  'Settings Dialog'   when the app starts
		break;

		case  3:     britenessFactor =     59;  
		break;

		case  4:		britenessFactor =      44;
		break;



		default:   
			ASSERT( 0 );   
			AfxMessageBox(   "SPitchCalc::Calculate_Briteness_Divisor  FAILED,  missing case."   );
			britenessFactor =   120;
		break;
	}
		
	return   britenessFactor;
}



											////////////////////////////////////////


void    SPitchCalc::Write_DrivingViews_X_Column_Raw(   short  scalePitch,    short  detectAvgHarmonicMag,    short  octaveIndex,   short  xWriteColumn,      
																													short   keyInSPitch,    OffMap&  drivingOffMap   )
{

													//  For HORIZONTAL DrivingViews


	short    briteGreyVal  =  200;   // *****  TEMP ,  



	long     britenessFactor =   Calculate_Briteness_Divisor(  m_displayBritnessFactor  );   //    m_mapsBrightnessFactor;     //  120    can ADJUST to make the bullets brighter    

//   long        m_displayBritnessFactor;    //    5 vals     {  0, 1, 2, 3, 4  }     


	bool     drawInColor =   true;
	bool    useScalePitchDrivingOffMap =   true;  



	if(   scalePitch  <  0   )     //   not an error,  just no detection and we are done with just scrolling the bitmap
		return;



	short   redFill,   greenFill,    blueFill,    yOct;
//	short   scalePitch     =   calcedNote.scalePitch;

	short   volumeMidi  =    detectAvgHarmonicMag;    //    calcedNote.detectAvgHarmonicMag;

//	short   octaveIndex =    calcedNote.octaveIndex;



	short	  quadPosition  = 	 Get_ScalePitchs_MusicalKey_Transposed_Position(   scalePitch,   keyInSPitch   );   //  NEW,  translates it for the diff musical key   11/11


	Get_ScalePitchs_Color(    scalePitch,     keyInSPitch,             redFill,       greenFill,        blueFill     );  


	redFill    =     (short)(          ( (long)redFill     *       (long)volumeMidi )    /  britenessFactor      );      //   moderate by the score's value
	greenFill =     (short)(          ( (long)greenFill *       (long)volumeMidi )    /  britenessFactor      ); 
	blueFill   =      (short)(         ( (long)blueFill    *       (long)volumeMidi )    /  britenessFactor     ); 


	if(   redFill      >  255  )      redFill =   255;  
	if(   greenFill  >  255  )      greenFill =   255;  
	if(   blueFill    >  255  )      blueFill =   255;  




	short   yInverted =  -1;


	if(    useScalePitchDrivingOffMap   )
	{
		yInverted  =     (drivingOffMap.m_height  -1)   -   quadPosition;     //  cause its a DIB,   I need to write upside down
	}
	else
	{
		ASSERT(  0  );    // Would need to be REWRITTED because of Musical Key and  quadPosition    11/11 

		if(   octaveIndex  >=   4    ||    octaveIndex  < 0   )
		{	
			int   dummy =  9;   // ****** LAND here sometimes.  Is this a REAL problem ???   3/30/10     ***********************
		}
		else
		{  yOct         =      ( octaveIndex *  12 )    +    scalePitch;

			yInverted  =     (drivingOffMap.m_height  -1)   -   yOct; 
		}
	}



	if(   yInverted  >=   0   )
	{
		if(   drawInColor   )
			drivingOffMap.Write_Pixel(    xWriteColumn,  yInverted,       redFill,            greenFill,         blueFill    ); 
		else
			drivingOffMap.Write_Pixel(    xWriteColumn,  yInverted,       briteGreyVal,   briteGreyVal,   briteGreyVal    ); 
	}
}




											////////////////////////////////////////


void    SPitchCalc::Write_DrivingViews_Y_Row_RAW(    short  scalePitch,   short  detectAvgHarmonicMag,    short  octaveIndex,   short  yWriteRow,   
																														short   keyInSPitch,   OffMap&  drivingOffMap   )
{

												//    For VERTICAL  Driving Views 


	short    briteGreyVal  =  200;   // *****  TEMP ,  


	long     britenessFactor =    Calculate_Briteness_Divisor(  m_displayBritnessFactor  );            //     120     ADJUST to make the bullets brighter    


	bool    useScalePitchDrivingOffMap =   true;   // ****** ALWAYS,  ************
	bool    drawInColor =   true;



	if(   scalePitch  <  0   )     //   not an error,  just no detection and we are done with just scrolling the bitmap
		return;


	short   redFill,   greenFill,    blueFill;

//	short   scalePitch     =   calcedNote.scalePitch;
	short   volumeMidi  =    detectAvgHarmonicMag;			//		calcedNote.detectAvgHarmonicMag;
//	short   octaveIndex =    calcedNote.octaveIndex;



	short	  quadPosition  = 	 Get_ScalePitchs_MusicalKey_Transposed_Position(   scalePitch,     keyInSPitch  );   //  NEW,  translates it for the diff musical key   11/11


//	Get_ScalePitchs_Color_GLB(               scalePitch,     keyInSPitch,     redFill,   greenFill,  blueFill   );  
	Get_ScalePitchs_Color(    scalePitch,     keyInSPitch,             redFill,       greenFill,        blueFill     );  



	redFill    =     (short)(          ( (long)redFill     *       (long)volumeMidi )  / britenessFactor     );      //   moderate by the score's value
	greenFill =     (short)(          ( (long)greenFill *       (long)volumeMidi )  / britenessFactor      ); 
	blueFill   =      (short)(         ( (long)blueFill    *       (long)volumeMidi )  / britenessFactor     ); 


	if(   redFill      >  255  )      redFill =   255;  
	if(   greenFill  >  255  )      greenFill =   255;  
	if(   blueFill    >  255  )      blueFill =   255;  




	short   xCalced =  -1;


	if(    useScalePitchDrivingOffMap   )
	{
		xCalced  =    quadPosition;        //		xCalced  =     scalePitch;     
	}
	else
	{
		ASSERT(  0  );    // Would need to be REWRITTED because of Musical Key and  quadPosition    11/11 
		/*****
		short   yOct;

		if(   octaveIndex  >=   4    ||    octaveIndex  < 0   )
		{	
			int   dummy =  9;   // ****** LAND here sometimes.  Is this a REAL problem ???   3/30/10     ***********************
		}
		else
		{  yOct         =      ( octaveIndex *  12 )    +    scalePitch;

			yInverted  =     (drivingOffMap.m_height  -1)   -   yOct; 
		}
		*****/
	}



	if(   xCalced  >=   0   )
	{
		if(   drawInColor   )
			drivingOffMap.Write_Pixel(    xCalced,  yWriteRow,       redFill,            greenFill,         blueFill    ); 
		else
			drivingOffMap.Write_Pixel(    xCalced,  yWriteRow,       briteGreyVal,   briteGreyVal,   briteGreyVal    ); 
	}
}






											////////////////////////////////////////
											///////////   Musical KEY  ///////////
											////////////////////////////////////////


short    SPitchCalc::Calc_FileCode_from_MusicalKey(   short  keyInScalePitch,     CString&  retMKeysName    /*  uses flats,  sharps ??   Is minor ??  */   )
{


	short  retFileCode =  -999;

	retMKeysName =   "ERROR"  ;


     if((  keyInScalePitch > 11   )||(  keyInScalePitch < 0    ))        
	 {
		 ASSERT( 0 );
		 return  -999;
	 }


	 short   numberOfAccidentals =  -1;
	 bool    usesFlats =  true;


	  switch(   keyInScalePitch   )      //    keyInScalePitch   is the same as  scalePitch Indexes  { 0 - 11,   E is 0  }           
	  {

		 case  0:       retMKeysName =  "E" ;      
							
							usesFlats =  false;				numberOfAccidentals =   4;                 							
			 break;   //    4 sharps      
				

		 case  1:       retMKeysName =  "F" ;         
							
							usesFlats =  true;				numberOfAccidentals =   1;                 							
			 break;      //       1 flat  


		 case  2:       retMKeysName =  "Gb" ;      
							
							usesFlats =  true;				numberOfAccidentals =   6;                 							
			 break;      //      6 flats    (  ...or   6  sharps   ???  )
		 

		 case  3:       retMKeysName =  "G" ;        
							
							usesFlats =  false;				numberOfAccidentals =   1;                 							
			 break;     //      1 sharp      


		 case  4:        retMKeysName =  "Ab" ;      
							
							usesFlats =  true;				numberOfAccidentals =   4;                 							
			 break;      //         4 flats
		 

		 case  5:        retMKeysName =  "A" ;        
							
							usesFlats =  false;				numberOfAccidentals =   3;                 							
			 break;    //        3 sharps        


		 case  6:        retMKeysName =  "Bb" ;      
							
							usesFlats =  true;				numberOfAccidentals =   2;                 							
			 break;     //        2 flats


		 case  7:        retMKeysName =  "B" ;        
							
							usesFlats =  false;				numberOfAccidentals =   5;                 							
			 break;       //    5 sharps       
	 

		 case  8:        retMKeysName =  "C" ;        
							
							usesFlats =  true;				numberOfAccidentals =   0;                 							
			 break;      //       NO sharps or flats   


		 case  9:        retMKeysName =  "Db" ;       
							
							usesFlats =  true;				numberOfAccidentals =   5;                 							
			 break;      //         5 flats


		 case 10:       retMKeysName =  "D" ;         
							
							usesFlats =  false;				numberOfAccidentals =   2;                 							
			 break;      //          2 sharps         


		 case 11:       retMKeysName =  "Eb" ;       
							
							usesFlats =  true;				numberOfAccidentals =   3;                 							
			 break;      //        3 flats		 			 

    

		 default:     
			 ASSERT( 0 );	   
			 retMKeysName =  "Calc_FileCode_from_MusicalKey:    unknown case"  ;
		break;
	  }



	if(   usesFlats   )
		retFileCode  =    -1 *  numberOfAccidentals;     //  Negative numbers for  FLATS
	else
		retFileCode  =            numberOfAccidentals;    //  and Positive numbers for  SHARPS


	return   retFileCode;
}



											////////////////////////////////////////


bool   SPitchCalc::Does_MusicalKey_Use_Sharps(   short  musicalKeyIdx   )
{



	bool   retUsesSharps =   false;


	  switch(  musicalKeyIdx  )      //    musicalKeyIdx   is the same as  scalePitch Indexes  { 0 - 11,   E is 0  }           
	  {

		 case  0:       retUsesSharps =  true;     break;   //  *firLet = 'E';       4 sharps      			
		 
		 case  1:											break;         //   *firLet = 'F';        1 flat  

		 case  2:											break;   //    *firLet = 'Gb';       6 flats    (  ...or   6   sharps  )
		 
		 case  3:    retUsesSharps =  true;     break;  //   *firLet = 'G';        1 sharp      

		 case  4:												break;    //     *firLet = 'Ab';       4 flats
		 
		 case  5:    retUsesSharps =  true;     break; //   *firLet = 'A';         3 sharps        

		 case  6:												break;    //    *firLet = 'Bb';       2 flats

		 case  7:    retUsesSharps =  true;     break; //  *firLet = 'B';       5 sharps       		 

		 case  8:											break;   //  *firLet = 'C';            NO sharps or flats   

		 case  9:											break; //     *firLet = 'Db';      5 flats		 

		 case 10:     retUsesSharps =  true;       break;  //  *firLet = 'D'           2 sharps         

		 case 11:												break;   //     *firLet = 'Eb';       3 flats		 			 
    
		 default:     ASSERT( 0 );	   break;
	  }

	return   retUsesSharps;
}



											////////////////////////////////////////


short	   SPitchCalc::Get_Musical_Key_from_FileCode(  short  keysFileCode,   short&  retSharpCount,    short&  retFlatCount,   short&  retScalePitch,    bool&  retIsMinor   )
{   
																						
			//   'keysFileCode'   is same as   SPitchListHEADER::

		
			//   'Minor' values   have 10 added to number of sharps or flats

			//     For  Key of C,  the keysFileCode  is  zero.


	// Also see    ScalepitchList::Get_Musical_Key_from_FileCode(

														                   //   SEE   "The Guitar Handbook"    pp. 108 for  KeySignature info !!!  
	
	retSharpCount  =    -1;      //  init
	retFlatCount      =    -1;   
	retIsMinor              =    false;

	retScalePitch =  -1;


	short   codeInFile =  keysFileCode;

	ASSERT(    codeInFile  <=  17      &&      codeInFile  >=   -17     );


	 bool  useSharps =  false;  

	 short  numberAccidentals  =  -1; 
	

	
	if(    codeInFile  > 0    )		           //  'SHARPS'
	{

		useSharps =  true;


		if(   codeInFile  >=  10   )
		{
			retIsMinor        =    true;

			retSharpCount  =    codeInFile   - 10;      
			retFlatCount      =                 0;   
		}
		else
		{  retIsMinor        =    false;

			retSharpCount  =    codeInFile;      
			retFlatCount      =        0;   
		}

		numberAccidentals =    retSharpCount;
	}

	else if(    codeInFile  < 0    )        //    'FLATS'
	{

		useSharps =  false;


		if(   codeInFile  <=   -10   )
		{
			retIsMinor         =    true;

			retSharpCount  =      0;   
			retFlatCount      =    -1 *   ( codeInFile  + 10  );    
		}
		else
		{  retIsMinor         =    false;

			retSharpCount  =      0;      
			retFlatCount      =    -1 *    codeInFile;   
		}

		numberAccidentals =    retFlatCount;
	}

	else					 //     codeInFile = 0,   Key of C Major  
	{
		retIsMinor         =    false;
		retSharpCount  =    0;      //  init
		retFlatCount      =    0;   

		numberAccidentals =   0;
	}



	CString  retErrorMesg,    retMKeysName;  


	if(    ! Get_KeySignature_Name_and_ScalePitch(    numberAccidentals,     useSharps,   retIsMinor,   retMKeysName,   retScalePitch,   retErrorMesg  )    )
	{
		ASSERT( 0 );
		AfxMessageBox(  retScalePitch  );
	}


	return   codeInFile;     
}



											////////////////////////////////////////


bool	 SPitchCalc::Get_KeySignature_Name_and_ScalePitch(   short  numberAccidentals,   bool  useSharps,   bool  isMinor,    CString&  retName,   short&  retScalePitch,   CString&  retErrorMesg   )
{


	retErrorMesg.Empty();
	retName.Empty();

	retScalePitch =  -1;  //  NEW,  1/12


	if(    numberAccidentals  < 0     )
	{
		retName       =    "error" ;
		retErrorMesg =    "SPitchCalc::Get_KeySignature_Name failed,  bad parm." ;
		return  false;
	}



	if(    numberAccidentals ==  0   )
	{
		if(   isMinor   )
			retName =    "A minor" ;
		else
			retName =    "C major" ;

		retScalePitch =   8;

		return  true;
	}



	if(    useSharps   )	//  SHARPS
	{

		switch(    numberAccidentals    )
		{

			case   1:	if(    isMinor    )			
								retName =    "E minor" ;
							else															    
								retName =    "G major" ;	

							retScalePitch =   3;
			break;


			case   2:	if(    isMinor    )			
								retName =    "B minor" ;
							else															    
								retName =    "D major" ;		

							retScalePitch =   10;
			break;


			case   3:	if(    isMinor    )			
								retName =    "F# minor" ;
							else															    
								retName =    "A major" ;		

							retScalePitch =   5;
			break;


			case   4:	if(    isMinor    )			
								retName =    "C# minor" ;
							else															    
								retName =    "E major" ;		

							retScalePitch =   0;
			break;


			case   5:	if(    isMinor    )			
								retName =    "Ab minor" ;
							else															    
								retName =    "B major" ;		

							retScalePitch =   7;
			break;


			case   6:	if(    isMinor    )			
								retName =    "Eb minor" ;
							else															    
								retName =    "F# major" ;		

							retScalePitch =   2;
			break;


			case   7:	if(    isMinor    )			
								retName =    "Bb minor" ;
							else															    
								retName =    "C# major" ;			

							retScalePitch =   9;
			break;


			default:     // retErrorMesg =    "Enter a number between 0 and 7" ;  
						//	return  false;    ...Edit control validation will do this
							retName =  "invalid" ;
			break;
		}

		return  true;
	}





	if(   ! useSharps    )		//  FLATS
	{

		switch(    numberAccidentals    )
		{

			case   1:	if(    isMinor    )			
								retName =    "D minor" ;
							else															    
								retName =    "F major" ;			

							retScalePitch =   1;
			break;


			case   2:	if(    isMinor    )			
								retName =    "G minor" ;
							else															    
								retName =    "Bb major" ;		

							retScalePitch =   6;
			break;


			case   3:	if(    isMinor    )			
								retName =    "C minor" ;
							else															    
								retName =    "Eb major" ;		

							retScalePitch =   11;
			break;


			case   4:	if(    isMinor    )			
								retName =    "F minor" ;
							else															    
								retName =    "Ab major" ;		

							retScalePitch =   4;
			break;


			case   5:	if(    isMinor    )			
								retName =    "Bb minor" ;
							else															    
								retName =    "Db major" ;		

							retScalePitch =   9;
			break;


			case   6:	if(    isMinor    )			
								retName =    "Eb minor" ;
							else															    
								retName =    "Gb major" ;		

							retScalePitch =   2;
			break;


			case   7:	if(    isMinor    )			
								retName =    "Ab minor" ;
							else															    
								retName =    "Cb major" ;		

							retScalePitch =   7;
			break;


			default:       // retErrorMesg =    "Enter a number between 0 and 7" ;  
						//	return  false;    ...Edit control validation will do this
							retName =  "invalid" ;
			break;
		}

		return  true;
	}



	retName       =    "error" ;
	retErrorMesg =    "SPitchCalc::Get_KeySignature_Name failed,  missing case." ;

	return  false;
}




											////////////////////////////////////////


void   SPitchCalc::Get_ScalePitch_LetterName(  short  scalePitch,   char *firLet,  char *secLet,    short  userPrefMusicalKeyAccidentals,   CString&  retNotesName   )
{

			// GOOD,               //    'userPrefMusicalKeyAccidentals'    0:  No preverence     1:  Use Sharps    2:  UseFlats

	retNotesName =  "ERROR"  ;
     

     if((  scalePitch > 11   )||(  scalePitch < 0    ))        
	 {
		 ASSERT( 0 );
		 return;
	 }

     if((  firLet == NULL  )||(  secLet == NULL  ))        
	 {
		 ASSERT( 0 );
		 return;
	 }


	 bool  useSharps =   false;


	 if(          userPrefMusicalKeyAccidentals  ==   1   )      //    1:  Use Sharps
	 {
		useSharps =  true;
	 }
	 else if(   userPrefMusicalKeyAccidentals  ==   2   )		//    2:  UseFlats
	 {
		useSharps =  false;
	 }
	 else if(   userPrefMusicalKeyAccidentals  ==   0   )      //   ??????   
	 {
		useSharps =  false;   //  ????????????????????????
	 }
     



     switch(  scalePitch  )                
	  {

		 case  0:    *firLet = 'E';      *secLet = ' ';             break;
		 
		 case  1:    *firLet = 'F';      *secLet = ' ';             break;


		 case  2:    if(  useSharps  )      {   *firLet = 'F';       *secLet = '#';    }
					    else						 {   *firLet = 'G';      *secLet = 'b';    }
		 break;

		 
		 case  3:    *firLet = 'G';      *secLet = ' ';             break;


		 case  4:    if(  useSharps  )      {    *firLet = 'G';      *secLet = '#';   }
					    else						  {   *firLet = 'A';      *secLet = 'b';    }			 
		 break;
		 

		 case  5:    *firLet = 'A';      *secLet = ' ';             break;



		 case  6:    if(  useSharps  )      {    *firLet = 'A';      *secLet = '#';   }
					    else						  {   *firLet = 'B';      *secLet = 'b';    }			 			 
		 break;
		 

		 case  7:    *firLet = 'B';      *secLet = ' ';             break;
		 
		 case  8:    *firLet = 'C';      *secLet = ' ';             break;



		 case  9:    if(  useSharps  )      {    *firLet = 'C';      *secLet = '#';   }
					    else						  {   *firLet = 'D';      *secLet = 'b';    }			 			 
		 break;
		 

		 case 10:    *firLet = 'D';      *secLet = ' ';             break;


		 case 11:   if(  useSharps  )      {    *firLet = 'D';      *secLet = '#';   }
					    else						  {   *firLet = 'E';      *secLet = 'b';    }			 			 			 
		 break;

    
		 default:   {   *firLet = ' ';       *secLet = ' ';  
			                 ASSERT( 0 );		  }			    
		 break;
	  }


	 retNotesName.Format(   "%c%c" ,   *firLet ,   *secLet   );

	 int  dummy =  9;
}



											////////////////////////////////////////


void    SPitchCalc::Get_Notes_Numeral_Name(   short  scalePitch,   short  keyInScalePitch,     CString&  retNotesName   )
{

	retNotesName =    "err"  ;				


	ASSERT(  scalePitch >=  0              &&    scalePitch  <= 11    );
	ASSERT(  keyInScalePitch >=  0     &&    keyInScalePitch  <= 11    );



	short       transposedScalePitch  =     Get_ScalePitchs_MusicalKey_Transposed_Position(   scalePitch,   keyInScalePitch  );  

     switch(   transposedScalePitch   )                
	  {

		 case  0:     retNotesName =     "1st"  ;
			 break;
		 
		 case  1:     retNotesName =     "1+"  ;
			 break;

		 case  2:     retNotesName =     "2nd"  ;
		 break;
		 
		 case  3:     retNotesName =     "b 3"  ;               //    "min 3"  ;    //    "min 3rd"  ;
			 break;

		 case  4:     retNotesName =     "M 3"   ;                 //   "Mg 3"   ;      //  "Mag 3rd"  ;
		 break;
		 
		 case  5:     retNotesName =     "4th"  ;       
		 break;

		 case  6:     retNotesName =     "b 5"  ; 		          //  "5 dim"  ; 			 
		 break;
		 
		 case  7:     retNotesName =     "5th"  ;
		 break;	 

		 case  8:     retNotesName =     "5+"  ;
		 break;

		 case  9:     retNotesName =     "6th"  ;
		 break;
		 
		 case 10:     retNotesName =	   "b 7"  ;         //    "min 7"  ;    	//  "min 7th"  ;
			 break;

		 case 11:     retNotesName =	   "M 7"	 ;       	//    "Mg 7"	 ;    //  "Mag 7th"  ; 			 			 
		 break;
   

		 default:     retNotesName =       "err"  ;					 
			              ASSERT( 0 );		
		 break;
	  }
}



											////////////////////////////////////////


void   SPitchCalc::Get_ScalePitch_LetterName_NEW(   short  scalePitch,    short  userPrefMusicalKeyAccidentals, /* short  useNumerals, */  CString&  retNotesName   )
{

						 //    'userPrefMusicalKeyAccidentals'    0:  No preference     1:  Use Sharps    2:  UseFlats

	retNotesName =  "ERROR"  ;
     

     if((  scalePitch > 11   )||(  scalePitch < 0    ))        
	 {
		 ASSERT( 0 );
		 return;
	 }



	 bool  useSharps =   false;


	 if(          userPrefMusicalKeyAccidentals  ==   1   )      //    1:  Use Sharps
	 {
		useSharps =  true;
	 }
	 else if(   userPrefMusicalKeyAccidentals  ==   2   )		//    2:  UseFlats
	 {
		useSharps =  false;
	 }
	 else if(   userPrefMusicalKeyAccidentals  ==   0   )      //   ??????   
	 {
		useSharps =  false;   //  ????????????????????????
	 }
     


	 char    firLet,   secLet;


     switch(  scalePitch  )                
	  {

		 case  0:    firLet = 'E';      secLet = ' ';    
	//					if(  useNumerals  )			retNotesName =  ""  ;
			 break;
		 

		 case  1:    firLet = 'F';      secLet = ' ';             
			 break;


		 case  2:    if(  useSharps  )      {   firLet = 'F';       secLet = '#';    }
					    else						 {   firLet = 'G';      secLet = 'b';    }
		 break;

		 
		 case  3:    firLet = 'G';      secLet = ' ';             
			 break;


		 case  4:    if(  useSharps  )      {    firLet = 'G';      secLet = '#';   }
					    else						  {   firLet = 'A';      secLet = 'b';    }			 
		 break;
		 

		 case  5:    firLet = 'A';      secLet = ' ';            
			 break;



		 case  6:    if(  useSharps  )      {    firLet = 'A';      secLet = '#';   }
					    else						  {   firLet = 'B';      secLet = 'b';    }			 			 
		 break;
		 

		 case  7:    firLet = 'B';      secLet = ' ';             
			 break;
		 
		 case  8:    firLet = 'C';      secLet = ' ';             
			 break;



		 case  9:    if(  useSharps  )      {    firLet = 'C';      secLet = '#';   }
					    else						  {   firLet = 'D';      secLet = 'b';    }			 			 
		 break;
		 

		 case 10:    firLet = 'D';      secLet = ' ';             
			 break;


		 case 11:   if(  useSharps  )      {    firLet = 'D';      secLet = '#';   }
					    else						  {   firLet = 'E';      secLet = 'b';    }			 			 			 
		 break;


    
		 default:   {   firLet = ' ';       secLet = ' ';  
			                 ASSERT( 0 );		  }			    
		 break;
	  }



//	 if(   !  useNumerals  )
		retNotesName.Format(   "%c%c" ,    firLet ,  secLet   );
}





											////////////////////////////////////////

/******
void    SPitchCalc::Get_Text_of_Musical_Key(   short  musicalKeyIdx,    CString&  retKeysName  )
{


ASSERT( 0 );     	// *****************  OMIT,  obsolete  *******************************



	retKeysName.Empty();

    if((  musicalKeyIdx > 11   )||(  musicalKeyIdx < 0    ))        
	 {
		 ASSERT( 0 );
		 return;
	 }

	  switch(  musicalKeyIdx  )      //    musicalKeyIdx   is the same as  scalePitch Indexes  { 0 - 11,   E is 0  }           
	  {

		 case  0:       retKeysName =  "E" ;        break;   //    4 sharps      
				
		 case  1:       retKeysName =  "F" ;         break;      //       1 flat  

		 case  2:       retKeysName =  "Gb" ;      break;      //      6 flats    (  ...or   6  sharps   ???  )
		 
		 case  3:       retKeysName =  "G" ;        break;     //      1 sharp      

		 case  4:        retKeysName =  "Ab" ;      break;      //         4 flats
		 
		 case  5:        retKeysName =  "A" ;        break;    //        3 sharps        

		 case  6:        retKeysName =  "Bb" ;      break;     //        2 flats

		 case  7:        retKeysName =  "B" ;        break;       //    5 sharps       
	 
		 case  8:        retKeysName =  "C" ;        break;      //       NO sharps or flats   

		 case  9:        retKeysName =  "Db" ;       break;      //         5 flats

		 case 10:       retKeysName =  "D" ;         break;      //          2 sharps         

		 case 11:       retKeysName =  "Eb" ;       break;      //        3 flats		 			 

    
		 default:     ASSERT( 0 );	   
					retKeysName =  "unknown"  ;
		break;
	  }
}
*****/

											////////////////////////////////////////


bool	   SPitchCalc::Calculate_Musical_Key_From_NoteList(   short&  retBestCount,     short&  retBestScore,     CString&  retErrorMesg   )
{


	long   minimumNumberOfNotes =   30;   // *************  ADJUST **********************

	retBestCount =  retBestScore =   -1;
	retErrorMesg.Empty();


	if(   m_calcedNoteListMasterApp  ==  NULL   )
	{
		retErrorMesg  =  "SPitchCalc::Calculate_Musical_Key_From_NoteList  FAILED,  m_calcedNoteListMasterApp is NULL."  ;
		return  false;
	}

	if(   m_calcedNoteListMasterApp->Count()   <  minimumNumberOfNotes  )
	{
		retErrorMesg  =  "Create a NoteList with at least 30 notes."  ;
		return  false;
	}



	long   noteScores[  12  ];

	long   noteCount[ 12 ];


	for(   long i=0;    i< 12;    i++   )
	{
		noteScores[  i  ] =   0;
		noteCount[  i  ] =   0;
	}



	long   divisor =   1000;




	ListIterator< MidiNote >   iter(   *m_calcedNoteListMasterApp    );	 



	for(    iter.First();    !iter.Is_Done();    iter.Next()    )			//  save list's   CalcedNote  as  ScalePitchSubjects 						
	{						

		MidiNote&     calcedNote =     iter.Current_Item();	

		long  notesLength  =      calcedNote.endingSampleIdxFile   -    calcedNote.beginingSampleIdxFile;

		notesLength =    notesLength / divisor;



		short   notesSPitch      =     calcedNote.scalePitch;
		if(       notesSPitch  >=  0   )
		{

			long   notesScore =     notesLength  *  notesLength;    //  want to geive extra weighting to long notes.
			

			noteScores[   notesSPitch   ]   +=   notesScore;
 
			noteCount[    notesSPitch   ]++;
		}
	}



	long   bestScore           =   -99;
	short  bestScoresIndex =    -1;


	long   bestCount  =   -99;
	short  bestCountsIndex =    -1;


	for(   long i=0;    i< 12;    i++   )
	{

		if(   noteScores[ i ]   >   bestScore   )
		{
			bestScoresIndex  =    i;
			bestScore            =    noteScores[ i ];
		}


		if(   noteCount[ i ]   >   bestCount   )
		{
			bestCountsIndex  =    i;
			bestCount            =    noteCount[ i ];
		}
	}




	if(   bestScoresIndex  ==   bestCountsIndex   )
	{
		int   dummy = 9;
	}


						// **********  WHICH is BEST ???  ********************


	retBestScore   =     bestScoresIndex;

	retBestCount   =     bestCountsIndex;       //  **** Seems more accurate.   




	CString   strMesg1,    strMesg2;

	strMesg1.Format(    "\n\nNote:   E[ %d,  %d ],  F[ %d,  %d ],   F#[ %d,  %d ],   G[ %d,  %d ],   G#[ %d,  %d ],   A[ %d,  %d ]  \n\n",

		noteCount[ 0], noteScores[ 0],      noteCount[1 ], noteScores[1 ],     noteCount[ 2], noteScores[ 2],     noteCount[3 ], noteScores[3 ],     noteCount[4 ], noteScores[ 4],     noteCount[5 ], noteScores[5 ]     );


		
	strMesg2.Format(    "        Bb[ %d,  %d ],  B[ %d,  %d ],   C[ %d,  %d ],   Db[ %d,  %d ],   D[ %d,  %d ],   Eb[ %d,  %d ]  \n\n",

		noteCount[ 6], noteScores[6 ],      noteCount[7 ], noteScores[7 ],     noteCount[8 ], noteScores[ 8],     noteCount[9 ], noteScores[ 9],     noteCount[ 10], noteScores[10 ],     noteCount[11 ], noteScores[11 ]     );

	
	TRACE(  strMesg1   );
	TRACE(  strMesg2   );


	return  true;
}




											////////////////////////////////////////


void    SPitchCalc::Calc_MusicalKey_by_ScaleProfile(   long  pitchScores[],    short&  retMusicalKeyBest,     short&  retMusicalKeySecondBest,   
																													          short&  retMusicalKeyThirdBest,  	 long  retProfileScores[]   )													
{

									//   The    pitchScores[]   must  have 12 elements.

	ASSERT(  retProfileScores  !=  NULL   ||   pitchScores  != NULL    );


	retMusicalKeyBest =   retMusicalKeySecondBest =  retMusicalKeyThirdBest  =  -1;

		/****
			double  resultProfile0[  12  ]  =      //     RESULT  Weights
			{
			//			1                  1#                2nd              minor3            Mag3              4th

					   6.0,             -6.0,                0.7,               3.0,                2.0,               3.0, 


					   -4.0,              4.0,              -4.0,               1.5,                3.0,               0.3 

			};//	   4+                 5th                 5+                6th               minor7            Mag7
		***/


//	long      retProfileScores[  12  ];

	long      bestScore            =   -99999999;
	short     bestScoresIndex  =    -1;

	long		secondBestScore           =   -99999999;
	short		secondBestScoresIndex =    -1;

	long		thirdBestScore           =   -99999999;
	short		thirdBestScoresIndex =    -1;




	for(    long  sPitch =0;     sPitch <  12;      sPitch++    )
	{

		double   thisScore =   0.0;

		short     indexToDatasSPitch =    sPitch;     //  init with  the 1st Pitch of its scale  ( G for 
//   Bad Sign    G
//
//			  E[ 3,  7986 ],             F[ 22,  242605 ],      F#[ 5,  24563 ],       G[ 46,  772303 ],      G#[ 0,  0 ],               A[ 3,  9317 ]  
//			  Bb[ 16,  172546 ],      B[ 5,  16093 ],         C[ 14,  71995 ],       Db[ 8,  14036 ],        D[ 17,  160204 ],      Eb[ 0,  0 ]  



		for(   short  p =0;     p < 12;     p++    )
		{



//			double   thisFactor  =     resultProfile0[  p  ]    *    (double)(    pitchScores[  indexToDatasSPitch  ]    );      //   A

//			double   thisFactor  =     resultProfile1[  p  ]    *    (double)(    pitchScores[  indexToDatasSPitch  ]    );      //   B


			double   thisFactor  =     resultProfile2[  p  ]    *    (double)(    pitchScores[  indexToDatasSPitch  ]    );      //   C






			thisScore   +=      thisFactor;    


			indexToDatasSPitch++;

			if(   indexToDatasSPitch   >  11  )
				indexToDatasSPitch =   0;
		}


		retProfileScores[  sPitch  ] =    (long)thisScore;    //   RETURN results





		long   holdBestScore           =    bestScore;
		short  holdBestScoresIndex =    bestScoresIndex;


		long   holdSecondBestScore           =    secondBestScore;
		short  holdSecondBestScoresIndex =    secondBestScoresIndex;




		if(     ( (long)thisScore )   >   bestScore    )                //     A.
		{
			bestScoresIndex =    sPitch;
			bestScore           =    (long)thisScore;


			if(    holdBestScore   >    secondBestScore   )    //  It may be that the FORMER bestScore is bigger than the current   'secondBestScore' , so reassign  
			{
				secondBestScoresIndex  =     holdBestScoresIndex;
				secondBestScore            =     holdBestScore;


				if(    holdSecondBestScore   >    thirdBestScore   )    //  It may be that the FORMER secondBestScore is bigger than the current   'thirdBestScore' , so reassign  
				{
					thirdBestScoresIndex  =     holdSecondBestScoresIndex;
					thirdBestScore            =     holdSecondBestScore;
				}
			}

		}   //  if(   thisScore  >  bestScore


		else if(    ( (long)thisScore )  >     secondBestScore   )     //     B.
		{
			secondBestScoresIndex  =    sPitch;
			secondBestScore            =    (long)thisScore;


			if(    holdSecondBestScore   >    thirdBestScore   )    //  It may be that the FORMER secondBestScore is bigger than the current   'thirdBestScore' , so reassign  
			{
				thirdBestScoresIndex  =     holdSecondBestScoresIndex;
				thirdBestScore            =     holdSecondBestScore;
			}
		}

		else if(    ( (long)thisScore )  >     thirdBestScore   )        //     C.
		{
			thirdBestScoresIndex  =    sPitch;
			thirdBestScore            =    (long)thisScore;
		}

	}   //   for(    long  sPitch =0;     sPitch <  12

	


	if(     bestScore  > 0   )
		retMusicalKeyBest     =         bestScoresIndex;

	if(     secondBestScore  > 0  )
		retMusicalKeySecondBest =   secondBestScoresIndex;

	if(     thirdBestScore  > 0   )
		retMusicalKeyThirdBest  =      thirdBestScoresIndex;
}




											////////////////////////////////////////


bool   SPitchCalc::Calculate_Musical_Key(   short  usersMusicalKeyGuess,     long   noteScoresWaveMan[],   long noteCount,    short&  retMKeyBestProfiles, 
									                           short&  retMKeySecondBestProfiles,     short&  retMKeyThirdBestProfiles,    CString&  retErrorMesg   )
{


			//   If CALLING function does NOT want a TRACE Dump,  set   usersMusicalKeyGuess < 0


	long   minimumNumberOfNotes =    30;   // *************  ADJUST **********************


 
	short  retBestScore,  retSecondBestScore;   // **** NOT USED,  just keep compiler happy ****


	retBestScore =  retSecondBestScore =    -1;
	retErrorMesg.Empty();


	if(    noteScoresWaveMan  ==   NULL  )
	{
		retErrorMesg  =  "SPitchCalc::Calculate_Musical_Key  FAILED,  noteScoresWaveMan  is NULL"  ;
		return  false;
	}

	if(   noteCount  <  minimumNumberOfNotes  )
	{
		retErrorMesg  =  "Create a NoteList with at least 30 notes."  ;   // No big deal
		return  false;
	}



	bool   showTraceDump  =    false;

	if(    usersMusicalKeyGuess >= 0    )
		showTraceDump  =    true;



	long  retProfileScores[ 12 ];   // This is the most accurate 
	
	SPitchCalc::Calc_MusicalKey_by_ScaleProfile(    &( noteScoresWaveMan[0] ),     retMKeyBestProfiles,   retMKeySecondBestProfiles,
																												retMKeyThirdBestProfiles,		  &(  retProfileScores[0] )   );	



	long   bestScore           =   -99;
	short  bestScoresIndex =    -1;

//	long   bestCount  =   -99;
//	short  bestCountsIndex =    -1;

	long   secondBestScore           =   -99;
	short  secondBestScoresIndex =    -1;




	for(   long i=0;    i< 12;    i++   )
	{

		/****
		if(         m_noteCountsMusicalKey[ i ]   >   bestCount 
			 &&   m_noteCountsMusicalKey[ i ]   >=   0    )
		{
			bestCountsIndex  =    i;
			bestCount            =    m_noteCountsMusicalKey[ i ];
		}
		****/


		long   holdBestScore           =    bestScore;
		short  holdBestScoresIndex =    bestScoresIndex;



		if(        noteScoresWaveMan[ i ]   >     bestScore
			&&   noteScoresWaveMan[ i ]   >=   0    )
		{
			bestScoresIndex  =    i;
			bestScore            =    noteScoresWaveMan[ i ];


			if(    holdBestScore   >    secondBestScore   )    //  It may be that the FORMER bestScore is bigger than the current   'secondBestScore' , so reassign  
			{
				secondBestScoresIndex  =     holdBestScoresIndex;
				secondBestScore            =     holdBestScore;
			}
				
		}
		else if(        noteScoresWaveMan[ i ]   >     secondBestScore
			       &&   noteScoresWaveMan[ i ]   >=   0    )
		{
			secondBestScoresIndex  =    i;
			secondBestScore            =    noteScoresWaveMan[ i ];
		}

	}



	if(   bestScore  <  0  )
	{
		retErrorMesg  =  "SPitchCalc::Calculate_Musical_Key FAILED,  NO notes were counted in m_noteCountsMusicalKey."  ;
		return  false;
	}




	retBestScore   =     bestScoresIndex;

	retSecondBestScore =    secondBestScoresIndex;





	if(    showTraceDump  )
	{

		ASSERT(  usersMusicalKeyGuess  >=  0      &&     usersMusicalKeyGuess  <= 11   );


		/****
		CString   strMesg1,    strMesg2;

		strMesg1.Format(    "\n\nNote:   E[ %d,  %d ],  F[ %d,  %d ],   F#[ %d,  %d ],   G[ %d,  %d ],   G#[ %d,  %d ],   A[ %d,  %d ]  \n\n",
			m_noteCountsMusicalKey[ 0], noteScoresWaveMan[ 0],      m_noteCountsMusicalKey[1 ], noteScoresWaveMan[1 ],    m_noteCountsMusicalKey[ 2],   noteScoresWaveMan[ 2],   
				m_noteCountsMusicalKey[3 ], noteScoresWaveMan[3 ],     m_noteCountsMusicalKey[4 ], noteScoresWaveMan[ 4],     m_noteCountsMusicalKey[5 ], noteScoresWaveMan[5 ]     );
			
		strMesg2.Format(    "        Bb[ %d,  %d ],  B[ %d,  %d ],   C[ %d,  %d ],   Db[ %d,  %d ],   D[ %d,  %d ],   Eb[ %d,  %d ]  \n\n",
			m_noteCountsMusicalKey[ 6], noteScoresWaveMan[6 ],      m_noteCountsMusicalKey[7 ], noteScoresWaveMan[7 ],     m_noteCountsMusicalKey[8 ],   noteScoresWaveMan[ 8],     
			m_noteCountsMusicalKey[9 ], noteScoresWaveMan[ 9],     m_noteCountsMusicalKey[ 10], noteScoresWaveMan[10 ],     m_noteCountsMusicalKey[11 ], noteScoresWaveMan[11 ]     );
		
		TRACE(  strMesg1   );
		TRACE(  strMesg2   );
		*****/

		TRACE(  "\n\n"  );



		double   divisor  =    4.0;

		short  scaleMember =    usersMusicalKeyGuess;     //   initialize  to  1st Pitch in Scale


		for(   long i =0;     i< 12;    i++   )
		{

			long      pitchsPolulation =     noteScoresWaveMan[  scaleMember  ];

			double  itsPercentage    =     (double)pitchsPolulation    /    (  (double)noteCount  *  divisor  );

			TRACE(    "             %.2f, ",     itsPercentage   );

			if(   i   ==   5   )
				TRACE(  "\n\n"   );


			scaleMember++;

			if(   scaleMember  >  11  )
				scaleMember =  0;
		}

		TRACE(   "\n\n\n"  );
	}    //   if(    showTraceDump


	return  true;
}



											////////////////////////////////////////


void     SPitchCalc::Get_ScalePitchs_Color(    short  scalePitchIdx,     short  keyInSPitch,      short&  retRed,    short&  retGreen,   short&  retBlue    )  
{

//   ****   ScalepitchList::Get_Musical_Key_from_FileCode(   short&  retSharpCount,    short&  retFlatCount,    bool&  isMinor   )



	ASSERT(   scalePitchIdx  < 12    );
	ASSERT(   keyInSPitch    < 12    );



	short   adjustedSPitch  =      scalePitchIdx    + 12;

	adjustedSPitch             =     adjustedSPitch   -   keyInSPitch;



														//   Get in  { 0 - 11 }   range  
	if(   adjustedSPitch  >=  12   )
		adjustedSPitch  =     adjustedSPitch   -12;
	
	if(   adjustedSPitch  >=  12   )
		adjustedSPitch  =     adjustedSPitch   -12;

	ASSERT(          adjustedSPitch  >=  0
					&&   adjustedSPitch  <=   11   );


/*****
		case  5:  	   retRed =  128;        retGreen =  255;        retBlue =   64;           break;	 //    4th				  (  LIME	    -  harmonious

		case 10:  	   retRed =  255;       retGreen =    0;         retBlue =  255;            break;	   //    flat 7th		  (  MAGENTA	   -  funkey

***/


/**********

//  Harmonious
		case  0:  	   retRed =  255;        retGreen =     0;        retBlue =      96;           break;	   //  Fundamental 	 (  RED	  	-  harmonious

		case  7:  	   retRed =  255;        retGreen =  128;        retBlue =      0;            break;	    //   5th				  (  ORANGE	 -  harmonious

		case  5:  	   retRed =  232;        retGreen =  232;        retBlue =      0;            break;	    //    4th				(  YELLOW	    -  harmonious



//   'BlueNotes'  ( funkey,  soulfull )
		case  3:  	   retRed =    64;        retGreen =   64;        retBlue =  255;           break;	    //    minor 3rd		   (  BLUE			    -  funkey

		case  10:  	   retRed =  128;        retGreen =    0;        retBlue =   128;           break;	   //    7th				  (  PURPLE



//   Etheral
		case  2:  	   retRed =     0;        retGreen =  128;        retBlue =  128;            break;	       //	 2nd		(  TEAL		  -   VERY etherial	 

		case  9:  	   retRed =      0;        retGreen =  255;        retBlue =     0;            break;	     //    6th		   (  GREEN	   -  etherial,   sometimes  Funkey	



//  Classical
		case  4:  	   retRed =  128;        retGreen =  255;        retBlue =  128;          break;		//	major  3rd		(	greenBROWN				     -  traditional

		case 11:  	   retRed =  255;       retGreen =  128;        retBlue =  128;            break;	  //	major 7th	    (  redBROWN					    -  traditional

**********/

	
	switch(   adjustedSPitch   )
	{


//  Harmonious
		case  0:  	   retRed =  232;        retGreen =  232;             retBlue =      0;            break;	    //   Fundamental  1th				(  YELLOW	    -  harmonious


		case  5:  	   retRed =  255;        retGreen =     0;               retBlue =      96;           break;	   //     4th    	 (  RED	  	-  harmonious
//		case  5:  	   retRed =  255;        retGreen =     0;               retBlue =      40;           break;	   //     4th    	 too orange
//		case  5:  	   retRed =  255;        retGreen =     0;               retBlue =      110;           break;	 



//		case  7:  	   retRed =  255;        retGreen =    96;      retBlue =      0;            break;	    //   5th				  (  ORANGE	 -  harmonious
//		case  7:  	   retRed =  255;        retGreen =    50;      retBlue =      0;            break;	    //   5th				  (  ORANGE	 -  harmonious
		case  7:  	   retRed =  255;        retGreen =    110;      retBlue =      0;            break;	    //   5th		



//   'BlueNotes'  ( funkey,  soulfull )
//		case  3:  	   retRed =    64;        retGreen =   64;        retBlue =  255;           break;	    //    minor 3rd		   (  BLUE			    -  funkey
//		case  3:  	   retRed =    64;        retGreen =   100;        retBlue =  255;           break;	    //    minor 3rd		   (  BLUE			    -  funkey
		case  3:  	   retRed =    64;        retGreen =   128;        retBlue =  255;           break;	    //    minor 3rd		   (  BLUE			    -  funkey





//		case  10:  	   retRed =  128;        retGreen =    0;        retBlue =   128;           break;	   //   flat  7th				  (  PURPLE
//		case  10:  	   retRed =  128;        retGreen =    0;        retBlue =   200;           break;	   //   flat  7th				 more blue


		case  10:  	   retRed =     180;        retGreen =  0;        retBlue =  150;            break;	     //    flat  7th		      (   was 2nd

//		case  10:  	   retRed =     200;        retGreen =  64;        retBlue =  200;            break;	     //    flat  7th		      (   was 2nd

//		case  10:  	   retRed =     180;        retGreen =  64;        retBlue =  120;            break;	     //    flat  7th		      (   was 2nd




//   Etheral
//		case  2:  	   retRed =     0;        retGreen =  128;        retBlue =  128;            break;	       //	 2nd		(  TEAL		  -   VERY etherial	 
//		case  2:  	   retRed =     64;        retGreen =  128;        retBlue =  64;            break;	       //	BAD, too green
//	    case  2:  	   retRed =     128;        retGreen =  64;        retBlue =  128;            break;	       //	too purple
//	    case  2:  	   retRed =     200;        retGreen =  64;        retBlue =  128;            break;	       //	
		case  2:  	   retRed =  128;        retGreen =    0;        retBlue =   200;           break;	             //    2nd         (  WAS  flat  7th				 more blue





		case  9:  	   retRed =      0;        retGreen =  255;        retBlue =     0;            break;	    //    6th		   (  GREEN	   -  etherial,   sometimes  Funkey	






//  Classical
//		case  4:  	   retRed =  128;        retGreen =  255;        retBlue =  128;          break;		//	major  3rd		(	greenBROWN				     -  traditional
		case  4:  	   retRed =  64;        retGreen =  255;        retBlue =  128;          break;		//	major  3rd		(	greenBROWN				     -  traditional



//		case 11:  	   retRed =  255;       retGreen =  128;        retBlue =  128;            break;	    //	major 7th	    (  redBROWN					    -  traditional
//		case 11:  	   retRed =    64;        retGreen =  128;        retBlue =  128;            break;	    //                                    ( was  2nd
		case 11:  	   retRed =    64;        retGreen =  100;        retBlue =  128;            break;	    //                                    ( was  2nd




//   Obscure
		case  6:  	   retRed =  128;        retGreen =  128;        retBlue =  128;          break;		//		flat     5th	(				   -  obscure

		case  8:  	   retRed =  128;        retGreen =  128;        retBlue =  128;           break;	    //		sharp  5th   (				   -  obscure

		case  1:  	   retRed =  128;        retGreen =  128;        retBlue =  128;          break;		//		sharp  1st   (				   -  obscure






		default:  
			ASSERT( 0 );		 
			retRed =  128;       retGreen =  128;        retBlue =  128;   
		break;
	}
}




											////////////////////////////////////////


short     SPitchCalc::Get_ScalePitchs_MusicalKey_Transposed_Position(    short  scalePitchIdx,     short  keyInSPitch  )  
{

		//  EX:    if in key of D (10)  and we wanted to know the  QUADRANT (12 positions) for the Revolver's Bullet for a
		//           note that is also D (10),  then the returned Quadrant would be 0 (the bottom Origin of the circle's traverse ).    11/11 
		//
		//			The same logic applies for Transforming by MusicalKey, the  'ScalePitch-Positions'  for Stationary Gauges.  



	ASSERT(   scalePitchIdx  < 12    );
	ASSERT(   keyInSPitch    < 12    );


	short   adjustedSPitch  =      scalePitchIdx    + 12;

	adjustedSPitch             =     adjustedSPitch   -   keyInSPitch;



														//   Get in  { 0 - 11 }   range  
	if(   adjustedSPitch  >=  12   )
		adjustedSPitch  =     adjustedSPitch   -12;
	
	if(   adjustedSPitch  >=  12   )
		adjustedSPitch  =     adjustedSPitch   -12;


	ASSERT(          adjustedSPitch  >=  0
					&&   adjustedSPitch  <=   11   );

	return  adjustedSPitch;
}



											////////////////////////////////////////


void    SPitchCalc::Calc_Time_in_MinutesSecsMills(  long   sampleIdx,    long& retMinutes,   long& retSeconds,    long& retMilliSeconds      )
{

	long   freqRate =   44160;  // ***********   HARDWIRED *************************


	long  miliSecondsTot    =    (long)(    1000.0  *     (   (double)sampleIdx  /   (double)freqRate  )    );

	long  secondsTot          =      miliSecondsTot    /    1000;

	long  miliSecondsRem  =      miliSecondsTot   %   1000;

	long  minutesTotal  =      secondsTot     /   60;

	long  secondsRem  =      secondsTot    %   60;



	retMinutes        =    minutesTotal;

	retSeconds       =   secondsRem;

	retMilliSeconds  =  miliSecondsRem;

//    virtSampText.Format(   "%d Min   %d Sec   %d Milli    ( %s )",     minutesTotal,   secondsRem,   miliSecondsRem,   retNotesName    );
}



											////////////////////////////////////////


void    SPitchCalc::Calc_Time_Format_String(   long  sampleIdx,    CString&  retTextString   )
{


	long   freqRate =   44100;         // ***********   HARDWIRED,  should this be an Inut parm ??? *************************
												  //    Maybe be OK,  cause this function only thinks in   "OUTPUT Bytes" ( 44,100 at 16 bit stereo )


	long  miliSecondsTot    =    (long)(    1000.0  *     (   (double)sampleIdx  /   (double)freqRate  )    );

	long  secondsTot          =      miliSecondsTot    /    1000;

	long  miliSecondsRem  =      miliSecondsTot   %   1000;

	long  minutesTotal  =      secondsTot     /   60;

	long  secondsRem  =      secondsTot    %   60;



	short   spacesToInsertForMilli =  0;

	if(          miliSecondsRem  < 10   )
		spacesToInsertForMilli =  2;
	else if(    miliSecondsRem  < 100   )
		spacesToInsertForMilli =  1;
	

	short   spacesToInsertForSeconds =  0;

	if(     secondsRem  < 10   )
		spacesToInsertForSeconds =  1;




			/***********************    BE  AFRAID !!!    ***********************************************

			Do NOT change any of the BELOW TEXT strings.  I have done some wierd stuff to make sure that the POSITIONS of  "Sec"  "Min"
			do NOT WIGGLE when they change from  3 digits to 1 or 2 ( miliSecondsRem ),  or from 2 spaces to 1  ( secondsRem  );

			I had to  CAREFULLY insert by HAND  some extras SPACES to the strings so that they would format OK.

			NOTE:  If one Numeral-Digit  is not to be displayed, I have to hit the SPACEBAR  2  TIMES  is order for it to look that way on the screen.  2/2012

			*****************************************************************/


//	if(   displayMode ==  1   )
//	{


		if(   minutesTotal  >  0  )       //  Actually have to insert 2 spaces for every REAL space ( so below I had to manually insert 2 and 4 physical spaces to get effect  2/12
		{
			if(          spacesToInsertForMilli ==  2   )   //  Insert 2 spaces so the control does not WIGGLE  ( empty digits in  miliSecondsRem  )
			{

				if(   spacesToInsertForSeconds  ==  1  )
					retTextString.Format(       "%d Min     %d Sec       %d Milli",   minutesTotal,   secondsRem,   miliSecondsRem   );
				else
					retTextString.Format(       "%d Min   %d Sec       %d Milli",   minutesTotal,   secondsRem,   miliSecondsRem   );
			}
			else  if(  spacesToInsertForMilli ==  1   )     //  Insert 1 space so the control does not WIGGLE  ( empty digits in  miliSecondsRem  )
			{

				if(   spacesToInsertForSeconds  ==  1  )
					retTextString.Format(       "%d Min     %d Sec     %d Milli",     minutesTotal,   secondsRem,   miliSecondsRem    );
				else
					retTextString.Format(       "%d Min   %d Sec     %d Milli",     minutesTotal,   secondsRem,   miliSecondsRem    );
			}
			else if(  spacesToInsertForMilli ==  0   )  
			{

				if(   spacesToInsertForSeconds  ==  1  )
					retTextString.Format(       "%d Min     %d Sec   %d Milli",     minutesTotal,   secondsRem,   miliSecondsRem    );
				else
					retTextString.Format(       "%d Min   %d Sec   %d Milli",     minutesTotal,   secondsRem,   miliSecondsRem    );
			}
		}

		else if(   secondsRem  >  0  )
		{
			if(          spacesToInsertForMilli ==  2     )   
				retTextString.Format(         "%d Sec       %d  Milli",                secondsRem,   miliSecondsRem      );
			else  if(   spacesToInsertForMilli ==  1 )    
				retTextString.Format(         "%d Sec     %d  Milli",               secondsRem,   miliSecondsRem      );
			else if(  spacesToInsertForMilli ==  0   )  
				retTextString.Format(         "%d Sec   %d  Milli",                secondsRem,   miliSecondsRem    );
		}
		else if(   miliSecondsRem  >=  0  ) 
		{	
			retTextString.Format(        "%d  Milli",                             miliSecondsRem    );
		}
		else
			retTextString.Format(      "0"       );
	/****
	}
	else
	  	retTextString.Format(   "%d",    virtSampsIdx / 1000   );
	***/
}





											////////////////////////////////////////


void    SPitchCalc::Update_DrivingViews_OffMap_DEBUG(   short  scalePitch,    short  octaveIndex,     bool  isPlayingBackward   )
{


	//  ********   Only CALLED  by  EventMan::Process_Event_Notification_PPlayer()   for obscure debug mechanism  ...now disconnected   11/11  *****


	short   volumeMidi  =     127;       //   detectAvgHarmonicMag;



	bool   drawInColor =   true;

	short   keyInSPitch =   m_musicalKey;   // ******  FIX  ******

	short   briteGreyVal  =  200;   // *****  TEMP ,  


	long     britenessFactor =    70;     //   256  ****ADJUST to make the bullets brighter    



//	TRACE(  "     ...update BitMap.  \n\n"  );




	if(   m_drivingOffMapHorz  ==  NULL   )
	{
		ASSERT( 0 );
		return;
	}



	bool	 goingBackwardALT  =     Is_Playing_Backwards();  

	ASSERT(   goingBackwardALT  ==   isPlayingBackward   );




	long    xWriteColumn,   yInverted =  -1,    lastX  =   m_drivingOffMapHorz->m_width   -1;





	if(    isPlayingBackward    )
	{
		xWriteColumn =   0;

		m_drivingOffMapHorz->Scroll_Horizontally(   false,    1  ); 

		m_drivingOffMapHorz->Assign_Xcolumn(   xWriteColumn,   0   );    //   set the FIRST column to black
	}
	else
	{  xWriteColumn =   lastX;

		m_drivingOffMapHorz->Scroll_Horizontally(   true,    1  ); 

		m_drivingOffMapHorz->Assign_Xcolumn(    xWriteColumn,   0   );    //   set the LAST column to black
	}




	if(   scalePitch  <  0   )     //   not an error,  just no detection and we are done with just scrolling the bitmap
		return;



	

	short   redFill,   greenFill,    blueFill,    yOct;

//	Get_ScalePitchs_Color_GLB(    scalePitch,     keyInSPitch,     redFill,       greenFill,        blueFill   );  
	Get_ScalePitchs_Color(    scalePitch,     keyInSPitch,             redFill,       greenFill,        blueFill     );  


	redFill    =     (short)(          ( (long)redFill     *       (long)volumeMidi )  / britenessFactor     );      //   moderate by the score's value
	greenFill =     (short)(          ( (long)greenFill *       (long)volumeMidi )  / britenessFactor      ); 
	blueFill   =      (short)(         ( (long)blueFill    *       (long)volumeMidi )  / britenessFactor     ); 



	if(   redFill      >  255  )      redFill =   255;  
	if(   greenFill  >  255  )      greenFill =   255;  
	if(   blueFill    >  255  )      blueFill =   255;  



	if(    m_useScalePitchDrivingOffMap   )
		yInverted  =     (m_drivingOffMapHorz->m_height  -1)   -   scalePitch;     //  cause its a DIB,   I need to write upside down
	else
	{
		ASSERT(   0   );    //  Right now this should not happen    2/11


		if(   octaveIndex  >=   4    ||    octaveIndex  < 0   )
		{	
			int   dummy =  9;   // ****** LAND here sometimes.  Is this a REAL problem ???   3/30/10     ***********************
		}
		else
		{  yOct         =      ( octaveIndex *  12 )    +    scalePitch;

			yInverted  =     (m_drivingOffMapHorz->m_height  -1)   -   yOct; 
		}
	}



	if(   yInverted  >=   0   )
	{
		if(   drawInColor   )
			m_drivingOffMapHorz->Write_Pixel(    xWriteColumn,  yInverted,       redFill,            greenFill,         blueFill    ); 
		else
			m_drivingOffMapHorz->Write_Pixel(    xWriteColumn,  yInverted,       briteGreyVal,   briteGreyVal,   briteGreyVal    ); 
	}
}







											////////////////////////////////////////
											////////////////////////////////////////


short      SPitchCalc::Get_Notes_AbsoluteIndex_from_RelativeHistoryIndex_CircularQue(   short  relativeHistoryIndex   )
{


ASSERT( 0 );     // *************  NOT USED,  is this worthwhile ????   3/11  ******************************



	//   'relativeHistoryIndex' :    the smaller the number, the more RECENT the note.    (  ex:   relativeHistoryIndex = 0 is latest,   = 1 is SECOND to latest,    

	ASSERT(   relativeHistoryIndex  <   m_sizeOfPrimNotesCircque  );

	short   origAbsoluteIndex  =     m_currentIndexPrimCircque  -    relativeHistoryIndex;     //  just for debug



	short   absoluteIndex  =     m_currentIndexPrimCircque  -    relativeHistoryIndex;  

	if(       absoluteIndex  <  0   )
	{
		absoluteIndex =    absoluteIndex   +   m_sizeOfPrimNotesCircque;       // *********** ??????????????   Is this right ??? ***************
	}


	ASSERT(   absoluteIndex >= 0     &&     absoluteIndex <  m_sizeOfPrimNotesCircque  );

	return  absoluteIndex;  
}



											////////////////////////////////////////


bool      SPitchCalc::Get_Most_Common_ScalePitch_In_CircularQue(   short   minimumMatchCount,    short&  retFoundSPitch,  
																												                       short&  retBestOctaveIdx,     short&  retOctaveScore  )
{

	//    retFoundSPitch   returns:
	//					[ 0  to 11  ]  :    A valid match for a VALID-detected SPitch was found was found	
	//						-1            :   a   "Matchin Majority"  was found of  NO-Detect  chunks
	//					    -9             :    No Matchin Majority was found
	//
	//   returns    FALSE if no Matching Majority was found,  true if it was found


	retFoundSPitch     =  -9;
	retBestOctaveIdx =   -9;
	retOctaveScore    =   -9;

												//  Try and see if we have  'minimumMatchCount'  of a single kind   



	short   sPitchPopulation[  13  ];    //    13:   12 scalepitches and the -1 assignment as well

	for(   short i = 0;   i < 13;    i++   )    //  init all buckets for the polulation count
		sPitchPopulation[ i ] =  0;    



	for(   short n = 0;   n <  m_sizeOfPrimNotesCircque;   n++   )        //  make an array of the populations of scalePitchs that are present in the cue
	{

		short  sPitchVal     =      m_circQuePrimNotes[ n ].scalePitch;

		if(      sPitchVal  < 0    )
		{
			ASSERT(   sPitchVal ==  -1   );
			sPitchPopulation[ 12 ]++;     
		}
		else
		   sPitchPopulation[  sPitchVal   ]++;    
	}




	short  foundSPitch =   -9;

	for(   short i = 0;   i < 13;    i++   )    //  init all buckets
	{

		if(    sPitchPopulation[ i ]   >=   minimumMatchCount   ) 
		{
			ASSERT(   foundSPitch  ==   -9   );		// *******  should be checking for duplicates  [ get them if cue's length is 2 and minimumMatchCount is 1 ]
																	//                      ...think that  minimumMatchCount must be MORE than 50% of the cue's size
			foundSPitch =   i;						
		}
	}




	if(          foundSPitch ==   -9    )
	{
		retFoundSPitch =   -9;      // a MIXED cue with no  "Matching Majority Population"
		return  false;
	}
	else if(   foundSPitch  ==  12    )     //  REMEMBER :   a matching number of   -1 ( NO detection)  means that we have a moment of silence, NOT a completely 'mixed' cue.  1/10
	{
		retFoundSPitch =   -1;      //  a  Matching Majority Population   if   No-Detections (-1)   was found
		return  true;
	}

	ASSERT(   foundSPitch >= 0     &&   foundSPitch <= 11   );

	retFoundSPitch =   foundSPitch;  



							//////////////////////////////////////////////////////

			//  Search for the most dominant Octave.  
	
			//		*********  I really could use a different method that allows the scores to affect the decision   ***********



	short   octavePopulation[  5  ];    //    13:   12 scalepitches and the -1 assignment as well

	long   octavePopulationScores[  5  ];  


	for(   short i= 0;   i < 5;    i++   )    //  init all buckets for the population count
	{
		octavePopulation[ i ] =  0; 
		octavePopulationScores[ i ] =  0; 
	}



	short    biggestOctaveScore       =  -99;
	long     totalCountOfSPitchNotes =   0;



	for(   short n = 0;   n <  m_sizeOfPrimNotesCircque;   n++   )        //  make an array of the populations of scalePitchs that are present in the cue
	{

		short  sPitchVal  =      m_circQuePrimNotes[ n ].scalePitch;

		if(      sPitchVal  ==   foundSPitch   )
		{

			short  octaveIdx      =      m_circQuePrimNotes[ n ].octaveIndex;



//			short  octavesScore =      m_circQuePrimNotes[ n ].detectScoreOctave;
			short  octavesScore = 0;

			if(    octaveIdx  >=  0   )
				octavesScore =      m_circQuePrimNotes[ n ].detectScoreOctaveCandids[  octaveIdx  ];




			if(    octaveIdx  < 0    )     //  the note must have a VALID calculation for Octave
			{
				ASSERT(   sPitchVal ==  -1   );
				octavePopulation[ 4 ]++;     
			}
			else
			{  octavePopulation[  octaveIdx   ]++;    

				octavePopulationScores[  octaveIdx   ]   +=    (long)octavesScore;  




				//	if(   m_circQuePrimNotes[ n ].detectScoreOctave   >   biggestOctaveScore   )
				//		biggestOctaveScore   =      m_circQuePrimNotes[ n ].detectScoreOctave; 

				if(     octavesScore   >   biggestOctaveScore    )
					biggestOctaveScore   =     octavesScore; 
		




				totalCountOfSPitchNotes++;     //  count the number of  CNotes  that are at our SPitch and had valid octave determination
			}
		}
	}




	short   foundOctave =   -9;

	
	/****
	short   biggestOctaveCount =  -9;

	for(   short i = 0;   i < 5;    i++   )    
	{

		if(    octavePopulation[ i ]   >   biggestOctaveCount   ) 
		{
			foundOctave =   i;	

			biggestOctaveCount =    octavePopulation[ i ]; 
		}
	}
	****/




	long    biggestOctaveTotalScore =  -9;


	for(   short i = 0;   i < 5;    i++   )    
	{

		if(    octavePopulationScores[ i ]   >   biggestOctaveTotalScore   ) 
		{

		//	ASSERT(   foundOctave  ==   -9   );		// *******  should be checking for duplicates  [ get them if cue's length is 2 and minimumMatchCount is 1 ]
																	//                      ...think that  minimumMatchCount must be MORE than 50% of the cue's size
			foundOctave =   i;	

			biggestOctaveTotalScore =    octavePopulationScores[ i ]; 
		}
	}

	ASSERT(    foundOctave  >=  0     &&    foundOctave  <=  3    );




	long   totalNotesWithOctavePick  =    octavePopulation[  foundOctave  ];    
	

																//  get average score frfor this  ScalePitch and OctaveIdx
	if(    totalNotesWithOctavePick  >  0   )
	{
		biggestOctaveScore =    octavePopulationScores[  foundOctave  ]   /   totalNotesWithOctavePick;     //  use average score
	}
	else
		biggestOctaveScore =    999;      //  they are usually a little over  1000





	retBestOctaveIdx =    foundOctave;

	retOctaveScore    =    biggestOctaveScore;

	return  true;
}



											////////////////////////////////////////


long    SPitchCalc::Calc_Primary_CircQues_Delay_In_CalcedNotes()
{

		//  returns value in  CalcedNote  ... that is the delay for each Event or PieSlice

		//  Calling Function when calcing Samples,  must take 'slowedSpeed'  into account


//	long      delayInNotes  =    m_sizeOfPrimNotesCircque /2;    //  ROUGH:  the way I read the Primary CircQue it acts like a Linear MEDIAN filter's delay  3/11


//	long      delayInNotes  =    m_numberNeededToMatch  -1;  
	long      delayInNotes  =    m_numberNeededToMatch;        // ****   7/2012   Is BETTER,    tested though   Get_DetectionDelay_InPieSlices()  ***** 

	return   delayInNotes;
}




											////////////////////////////////////////


long   SPitchCalc::Calc_Final_CircQues_Delay_In_CalcedNotes()
{

	long      delayInNotes  =    m_sizeOfFinalNotesCircque;     //  NOT like a Median filter,  just a straightforward delay mechanism
	return   delayInNotes;
}



											////////////////////////////////////////


void   SPitchCalc::Get_New_Filtered_ReturnNote_CircularQue(   CalcedNote&  retNote   )
{

		//   Only CALLED  by    Estimate_ScalePitch(),    and this funct assigns the   'synthCode'


			// ************************************************************************************************************
			//  ********  BIG,  READ:     this essentially becomes a MEDIAN FILTER when we read the Most Common Scalepitch Value, and so the 
			//  Pixel Value (sampleIndex) is the MIDPOINT between the Oldest Pixel in the Que and the Newest Pixel in the cue.
			//       (  THUS:   sampleIdx of returnedNote  =   sampleIdx of OldestPixel  +  (  sizeOfCircQue *  samplsInPieFetch  )/2    ...just like a Median Linear function   3/11
			// ************************************************************************************************************


	retNote.scalePitch    =   -1;
	retNote.octaveIndex =   -1;
	retNote.detectScoreHarms =  -1;

	retNote.detectAvgHarmonicMag =  -1;		
	retNote.synthCode  =    -1;     //  INIT,   -1 means do nothing

	retNote.primaryQuesSampleIdx =  -1;		// **** NOT really used after this function,   so we will NOT explicitly assign it in this function
      
	retNote.beginingSampleIdxFile =  -3;	 //   Very IMPORTANT,   this will be assigned here !!!!
	
//	retNote.detectScoreOctave =  -1;
	for(    short oct =0;    oct <  kCountOfNavOctaveCandidates;    oct++   )
		retNote.detectScoreOctaveCandids[  oct  ]  =   -1;





	short   retFoundSPitch =   -9;
	short   synthCode       =   -1;


   /****************   Only for DEBUG   ( is an alternate way to calc    )   11/29/11


	      //  Find the OLDEST (smallest) and NEWEST   primaryQuesSampleIdx   in the que,  and then calc the CenterPixel in this  MEDIAN filter ( ie the CircQue )


	long    centerSampleIdx =   -8;    //  like the X-value of middle Pixel in a  MEDIAN Filter
	long   absoluteSmallestSampleIdx   =   2000483647;     //  biggest long  is  2,147,483,647
	long   absoluteBiggestSampleIdx    =   -1;  

	for(   short i = 0;     i < m_sizeOfPrimNotesCircque;   i++   )      
	{

		if(    m_circQuePrimNotes[ i ].primaryQuesSampleIdx   >= 0   )    //  only for valid values of  primaryQuesSampleIdx
		{

			if(     m_circQuePrimNotes[ i ].primaryQuesSampleIdx   <   absoluteSmallestSampleIdx   )
				absoluteSmallestSampleIdx  =   m_circQuePrimNotes[ i ].primaryQuesSampleIdx;
			
			if(     m_circQuePrimNotes[ i ].primaryQuesSampleIdx   >   absoluteBiggestSampleIdx   )
				absoluteBiggestSampleIdx  =   m_circQuePrimNotes[ i ].primaryQuesSampleIdx;
		}  
	}


	long   distanceInSamples =  0;

	if(   absoluteSmallestSampleIdx  >=  0   )    //  ****** This calc is INACCURATE  untill ALL the Notes in the CircQue have had  '.primaryQuesSampleIdx'  initialized
	{										                        //				with real values.   Not really sure why this works.   11/28/11

		distanceInSamples =   absoluteBiggestSampleIdx    -     absoluteSmallestSampleIdx;  

		centerSampleIdx   =    absoluteSmallestSampleIdx    +     distanceInSamples/2;    // same calc as for a linear MEDIAN Filter
	}
	else
	{	ASSERT( 0 );    // can not happen		
		centerSampleIdx  =  -7;   
	}   //  show that it couold not be computed ????    3/11
	

	long   adjustCenterSampleIdx  =    centerSampleIdx  -   logDFTdelaySamps;    //  bring in another form of delay.  This will cause sync when doing a file save.  3/11


//	retNote.beginingSampleIdxFile  =   adjustCenterSampleIdx;		//  *** BIG ***  get its true value here, and will be used for FILE SAVE 

******/



	/**************************************   BAD  CALCS    ...will I try this again ????    12/11
	long   wmansPieSliceIdx  =    wavemansPieSliceIdx;
	long   noteStartNotes    =     notesPieSliceIdx   -   logDFTdelayNotes;
	long   noteStartSamps  =     noteStartNotes   *   sampleInPieEventWithSpeed;   //  GOOD:   Same result if first convert to samps and then add together  

	retNote.pieSliceIdxAtDetection              =     notesPieSliceIdx;
	retNote.wavemansPieSliceIdx    =     wavemansPieSliceIdx;   
	****/



//	retNote.primaryQuesSampleIdx  =  ???     	NO!!!!!!!!!!    Only used before this function


	retNote.beginingSampleIdxFile =    -13;     //  will get assigned later on




	short  retBestOctaveIdx = -8,  retOctaveScore = -8;   


	bool   aMatchingMajorityWasFound  =     Get_Most_Common_ScalePitch_In_CircularQue(   m_numberNeededToMatch,    retFoundSPitch,
																																		           retBestOctaveIdx,   retOctaveScore  );



																					
	if(    retFoundSPitch   < 0    )     //		A)    FAILED to find a VALID scalePitch,  so we can return early		
	{


		if(     retFoundSPitch  ==  -1  )    //   Found 3 matches for  BAD-DETECTION scores,  so we should   DEFINATELY turn the synth OFF
		{	

			if(   m_aNoteIsPlaying   )    
			{
				//retNote.synthCode  =  0;      	//  retSynthCode =      0;       //   0: TURN synthesizer OFF 
				synthCode =   0;

				m_aNoteIsPlaying =   false;    
				m_prevFoundNote =    -1;      
			}
			else
			{	//retNote.synthCode  =  -1; 				//  retSynthCode =   -1;			
				synthCode =   -1;
			
			    ASSERT(  m_prevFoundNote  ==   -1   );
				m_prevFoundNote =    -1;   //  -1:   nothing was found ( NO scalePitch )
			}

		}
		else if(    retFoundSPitch  <=  -2   )    //  Is -9     ...could NOT find 3 matches,  so we MIGHT(?) let the current note keep playing ( until it gets 3 matches, either for a VALID scalePitch, or finds 3 bad detections
		{

			ASSERT(  aMatchingMajorityWasFound  ==  false );


			if(    m_aNoteIsPlaying    )     //   a note is PLAYING
			{


													// **********************************************************************************************************
				if(   m_letNotesSustain    )      //  let the note keep playing even though NO Majority ( only turn note off if there is a Majority of  -1  detections  )
				{									//  **********  A GREAT Threshold technique...  3 forces in competition:  a  'NewPitch',  the  'CurrentPitch', and the  'SilencePitch'     1/2012



	//				ASSERT(   m_prevPlayingNotesAvgHarmonicMag  >   0    );   //   Can happen if Change Midi Source Radio buttons while playing  12/14/11
					ASSERT(   m_prevPlayingNotesAvgHarmonicMag  >= 0    ); 


					ASSERT(   m_prevFoundNote  >= 0    );  


					retNote.scalePitch                     =    m_prevFoundNote;   //  hold the same
					retNote.detectAvgHarmonicMag  =   m_prevPlayingNotesAvgHarmonicMag;   //  need this to control the  LIGHTNESS of the 'bullet'  on the visual display

				//	retNote.detectScoreHarms  =   ?????     NOT really needed.    1/21/10 

					synthCode =   -1;      //  -1 :  do nothing   


					m_prevFoundNote  =    retNote.scalePitch;  				
				}
				else
				{  //retNote.synthCode  =    0;       //   retSynthCode  =    0;       //  (   0: turn OFF thesynthesizer )
				    synthCode =   0;

					m_aNoteIsPlaying  =    false;   
					m_prevFoundNote  =    -1; 
				}

			}
			else   //   NO note is playing
			{	
				//retNote.synthCode  =    -1;    		//	 retSynthCode        =    -1;                 			    
				synthCode =   -1;

			    ASSERT(   m_prevFoundNote ==  -1  );    //  ??always 			
			    m_prevFoundNote  =    -1;
			}

		}
		else   {   ASSERT( 0 );   }



		retNote.synthCode  =   synthCode;    //   ***BIG

		return;
	}   //    if(    retFoundSPitch  < 0   ) 




						//     B)   Since we have our  'minimumMatchCount'  matches for a VALID scalePitch,  then determine what the new note should play

							
	short   oldestNoteWithScalepitchValue     =   -8;      //  find the oldest in the circQue that has our SPitch value
	long    smallestSampleIdx   =   2000483647;     //  biggest long  is  2,147,483,647
	short   biggestHarmicMag   =  0;


	for(   short n =  (m_sizeOfPrimNotesCircque -1);     n >= 0;     n--   )      
	{

		short  sPitchVal     =      m_circQuePrimNotes[ n ].scalePitch;

		if(     sPitchVal  ==   retFoundSPitch   )
		{

			if(       m_circQuePrimNotes[ n ].primaryQuesSampleIdx   <   smallestSampleIdx   
				&&  m_circQuePrimNotes[ n ].primaryQuesSampleIdx   >= 0   )
			{
				oldestNoteWithScalepitchValue     =   n;
				smallestSampleIdx  =   m_circQuePrimNotes[ n ].primaryQuesSampleIdx;
			}


																			//   Find which  CalcedNote( with Targ-ScalePitch)  has the biggest  Average-Harmonic

			if(       m_circQuePrimNotes[ n ].detectAvgHarmonicMag   >  biggestHarmicMag   
				&&  m_circQuePrimNotes[ n ].primaryQuesSampleIdx   >= 0   )
			{
			//	oldestNoteWithScalepitchValue     =   n;    NO,  only set this for the oldest
				biggestHarmicMag  =   m_circQuePrimNotes[ n ].detectAvgHarmonicMag;   //  Controls volume of a Midi note  3/11
			}
		}
	}



	if(     oldestNoteWithScalepitchValue  ==  -8   )
	{
		//   ASSERT( 0 );   //    BUG, should have found something   (  *** Get here sometimes when hit  GoToFileStart button  4/4/11   ) 

		synthCode =   -1;    //  -1 means do nothing
		return;
	}







	if(   ! m_aNoteIsPlaying    )     //     NOTHING is Playing so looking to START a new note
	{

		ASSERT(   m_prevFoundNote  < 0  );
												 //		TRACE( "START  synthesizer    SPitchCalc" );

		m_aNoteIsPlaying =   true;     //   show that  synthesizer will NOW be playing with the new PITCH  ( calling function will automatically turn off old note and start new one )													
     
		synthCode      =   1;     //   1: START playing a NEW note on synth
	}

	else    //  ...we are currently playing a note,   so we are looking to possibly CHANGE  the playing Note
	{

		if(    retFoundSPitch  !=   m_prevFoundNote   )       //  Only if this is a DIFFERENT Pitch, will we mess with the synthesizer
		{									
															//		TRACE( "START  synthesizer    SPitchCalc" );
			m_aNoteIsPlaying =   true;   

			synthCode =   1;
		} 					
	}  



	m_prevFoundNote   =     retFoundSPitch;      //   only copies a scalePitch 'short' value,  NOT a full note struct




			              //  Using  'oldestNoteWithScalepitchValue'  is the most accurate.   *** TRY AVERAGING ...  better???  


	m_prevPlayingNotesAvgHarmonicMag  =    m_circQuePrimNotes[   oldestNoteWithScalepitchValue  ].detectAvgHarmonicMag;  //  save this for the SUSTAIN feature ( m_letNotesSustain )


// *******************   INSTALL,  should   average early ones ???   for better octave score.  12/11  ****************************
// ***************************************************************************************************************






	retNote =    m_circQuePrimNotes[   oldestNoteWithScalepitchValue  ];      //  COPY all of STRUCT    ...the results from the OLDEST note in the circQue

		retNote.synthCode                   =     synthCode;    //   The  copy-assignment  ABOVE just erased the previous synth code

		retNote.beginingSampleIdxFile  =    -13;      //  Will get assigned just after this function

 
		/******************    Because we COPY the entire STRUCT of this Note from the Primary-CircQue,  these values will persist.    12/11

		retNote.octaveIndex  =   

		retNote.detectScoreOctave =    All of ARRAY was copied by the COPY OPERATOR (  retNote.detectScoreOctaveCandids[  oct  ]  

		retNote.detectScoreHarms =   

		retNote.detectAvgHarmonicMag =  


		retNote.pieSliceIdxAtDetection  = 	//   Also  'FakeEvent'  in  Make_NoteList_No_Audio().                Has a true value, like 'BeginingSampleIdxFile'.   It is slightly lagged by the filtering effects of the PrimaryCircQue.   11/11

		****/



//		/***********************************  Do I want this algo ???   12/9/11     OR can I adapt it ???

		if(          retBestOctaveIdx  >=  0    
			  &&   retOctaveScore     >    0     )
		{			
			retNote.octaveIndex          =    retBestOctaveIdx;


//			retNote.detectScoreOctave =    retOctaveScore;    ******* NOT NECESSARY below,  it was

			for(    short oct =0;    oct <  kCountOfNavOctaveCandidates;    oct++   )    // *****  COPIED aready up above  **************
			{
				retNote.detectScoreOctaveCandids[  oct  ]  =   m_circQuePrimNotes[   oldestNoteWithScalepitchValue  ].detectScoreOctaveCandids[  oct  ];
			}


		}

//		*****/
			
		


//		retNote.primaryQuesSampleIdx =  NO!!!,   Is NOT necessary, but let it carry the assignment from  m_circQuePrimNotes[   oldestNoteWithScalepitchValue  ],  it may be useful sometime.  3/21/11



//		retNote.pieSliceIdxAtDetection   =      notesPieSliceIdx;                ***** BAD CALCS
//		retNote.wavemansPieSliceIdx    =     wavemansPieSliceIdx;    ***** BAD CALCS
}




											////////////////////////////////////////


long		SPitchCalc::Calc_LogDFTs_Delay_In_CalcedNotes()
{


		//   BECAUSE the RETURN values NEVER really CHANGES,  this function may not be relevant to all calcs.    3/11

		//   the  HarmPairsTrForm  also contributes to this delay.  See  values that are SET in   Estimate_ScalePitch()



	long   delayInCalcedNotes =   3;   //  Is always  3,  even with new  HI-RES mode for logDFT    9/1/2012

	/****************  BAD,  can not be dependant  on  dftWriteColumn   and   m_dftReadColumn   for new HI-RES  mode   9/1/12  



  	long   dftWriteColumn =    m_dftMapsWidth  - 1;       //  was   10 -1  =  9


	//                                             9               -                6                ==    3                                    

	delayInCalcedNotes    =    dftWriteColumn   -    m_dftReadColumn;    //     3
	*****/


	return   delayInCalcedNotes;
}




											////////////////////////////////////////


bool	 SPitchCalc::Is_CurrentSampleIdx_Continuous(   long  currentSampleIdx,   long  curPieSliceIdx,    bool  isPlayingBackward,     bool  usesNavigatorDelay,   
		                                                                                                                          long&  retLostNotesCount,    short  fromPreRoll,    CString& functionCode    )
{

		//    fromPreRoll:   0[ not from PreRoll]    1: PreRoll FakeEvents after 0     2: PreRoll,  FakeEvent = 0 the first.    3/12


		//   CALLED FROM:    SPitchCalc::Estimate_ScalePitch()        SPitchCalc::Search_NoteList_for_AudioMusical_Values()


		//  Sometimes on the Start of PLAY,  some  Events( PieSlices )  from last BlockLoad were not Processed.  
	    //
		//	 Here we first test for a Lack of Continuity in the timeStamps of the CalcNotes, and possibly  STUFF the LOST NOTES into the FinCircQue
	    //  Then the visual timing will be consistant, and  XORbox Selection  will seem accurate.  Only for Navigator because it only has DELAY with a FinCircQue.    2/2012
		//
		//  (  These TEST just duplicate what is now at the front of  Process_Event_Notification_PPlayer()    2/2012

	bool   retIsContinuous=  true;

	retLostNotesCount =  0;


	if(   isPlayingBackward   )
	{
		return  true;   //  ******   NOT yet  IMPLEMENTED for Bacwards Play  *******
	}
		


	bool	 goingBackwardALT  =     Is_Playing_Backwards();  

//	ASSERT(   goingBackwardALT  ==   isPlayingBackward   );   //  Landed here on 7/11/12 after adjusting DETAIL Control to a higher value.




	if(  ! usesNavigatorDelay   )
	{
		return  true;    //   Not for  PLAYER.exe 
	}


	CString    mesg;
    CalcedNote   retNotePrevious;  
	long              sampsInPieEvent    =    Calc_Samples_In_PieEvent(  m_playSpeedFlt  );



	Get_Newest_Note_from_Final_CircularQue(   retNotePrevious   );	   //  this was the  last note  that was added to the end of the FinCircQue






// ***********************  BAD ASSUMPTION  (   change in size of  PrimCircQue  could also throw this off   *************************

	long   expectedCurSampleIdx   =    retNotePrevious.curSampleWaveman  +   sampsInPieEvent; 

// *******************************************************************************************************************






	long   pieSliceCounterAbsolute  =   retNotePrevious.pieSliceCounter;    //  Not sure how to use this.  Only for debugging.  



			//  'pieSliceIdxAtDetection'  is NOT valid for CalcNotes once they go into the FinalCircQue,  use  'pieSliceIdxAfterFiltration'  instead.  pieSliceCounter is worthless.  3/12

	long       expectedPieSliceIndex     =      retNotePrevious.pieSliceIdxAfterFiltration   +1;


	if(          expectedPieSliceIndex   !=   curPieSliceIdx  

		&&     fromPreRoll  !=  1                 // ****** WHY isn't this as accurate as   'expectedCurSampleIdx'    ?????   ****************
		&&   ! isPlayingBackward 
		&&     retNotePrevious.curSampleWaveman  > 0 
		&&     usesNavigatorDelay  	)	
	{
		int   dummy =  9;

	//	TRACE(  "\n       ***PIE-SLICE Index out of SYNC   %d  SLICES    [   expected %d       actual  %d   ]  \n" ,    
	//		                                                (curPieSliceIdx - expectedPieSliceIndex),     expectedPieSliceIndex,    curPieSliceIdx  );
	}





	if(          currentSampleIdx   !=   expectedCurSampleIdx  
		&&     retNotePrevious.curSampleWaveman  > 0 
		&&     usesNavigatorDelay  	)	
	{


		if(   ! isPlayingBackward     )    //  NOT for BACKWARD Play  ...too CONTROVERSAL   2/10/2012
		{


			retLostNotesCount  =    (  currentSampleIdx  -  expectedCurSampleIdx )   /  sampsInPieEvent;



			if(        retLostNotesCount  > 0   )    // Can get here only ONCE after a MOVE of the FileSlider.  2/12
			{

				if(    fromPreRoll  !=  1    )    //  1:   First FakeEvent iteration in    Process_Event_Notification_PPlayer_NoAudio()
				{

					retIsContinuous =  false;   //  Hit this LAST:  [  3/1/12 Hit ChangeMidiSrc while playing   ]  [3/2  On ver Slow speed 4, 8  ]


			//		TRACE( "\n       ***OUT of SYNC    %d  LostNotes   [   %d expected     %d currentSampleIdx   ]   Is_CurrentSampleIdx_Continuous()  \n",   
			//								                                                        retLostNotesCount,    expectedCurSampleIdx,   currentSampleIdx    );
			

		    // ******** With the addition of  Post_Roll_NoteDetect(),  Add_False_CalcNotes_for_OutOfSync  should never have to be called again.   2/29/2012  ********


//  Maybe should disable this.  It might make things wrse.  3/3/12
			//		Add_False_CalcNotes_for_OutOfSync(   retLostNotesCount,   currentSampleIdx,    isPlayingBackward,    retNotePrevious   );    // CAREFUL !!!!

			//		TRACE(   "          ...Add_False_CalcNotes_for_OutOfSync()  just added   %d   CalcNotes   to FinalCircQue  \n",     retLostNotesCount   );


						
			//	    mesg.Format(   "***SYNC:   Added  %d   FALSE NOTES  [  curPieSliceIdx  %d   ]."  ,    retLostNotesCount,    curPieSliceIdx     ); 
			//	    mesg.Format(   "***SYNC:   Saw     %d   FALSE NOTES  [  curPieSliceIdx  %d   ]."  ,    retLostNotesCount,    curPieSliceIdx     ); 

					mesg.Format(   "%d lost slices %s [%d]"  ,    retLostNotesCount,   functionCode,   curPieSliceIdx     ); 


					Write_To_StatusBar_GLB(   mesg   );    //    *** Will be a dummy function for RELEASE   ***
				}
			}
			else
			{

				if(    fromPreRoll  !=  1    )    //  1:   First FakeEvent iteration in    Process_Event_Notification_PPlayer_NoAudio()
				{

					retIsContinuous =  false;   //  Hit this LAST:  [  3/1/12 Hit ChangeMidiSrc while playing   ]  [3/2  On ver Slow speed 4, 8  ]


			//		TRACE( "\n       ********OUT of SYNC-neg    %d   NEGATIVE LostNotes   [   %d expected     %d currentSampleIdx   ]   Is_CurrentSampleIdx_Continuous()  \n\n",   
			//								                                                        retLostNotesCount,    expectedCurSampleIdx,   currentSampleIdx    );			
				}

			}
		}  
	}

	return   retIsContinuous;
}




											////////////////////////////////////////


bool   SPitchCalc::Detect_Note_for_Pie_Slice(    long   newNotesSampleIdx,    long  eventsOffsetAdj,     CalcedNote&  retCurrentNotePlayer,   bool isPlayingBackwards,  
													                  SndSample&  sndSample,   bool  useNavigatorDelay,   long  pieSliceIdxAtDetection,   
																	                         short  fromPreRoll,    long  pieSliceCounter,   CString&  retErrorMesg    )
{	
	retErrorMesg.Empty();



	bool	 goingBackwardALT  =     Is_Playing_Backwards();  

//	ASSERT(   goingBackwardALT  ==   isPlayingBackwards   );   // ***   FAILS  when starting out in REVERSE Play     4/26



	if(    ! Calc_logDFTs_Values(   sndSample,    eventsOffsetAdj,    retErrorMesg   )      )   //   'sndSample'  must alread be populated with data  11/11
	{
		return  false;  			      //  read and process the bytes,  send the DFTrowProbes out.  
	}


	if(    ! Estimate_ScalePitch(   newNotesSampleIdx,   isPlayingBackwards,    retCurrentNotePlayer,   useNavigatorDelay,  pieSliceIdxAtDetection,  fromPreRoll,   pieSliceCounter,    retErrorMesg  )    )    //  put the new note in the CircularQues   {  Smoothing, FinalForDelay  }
		return  false;  
	
	return  true;
}





											////////////////////////////////////////
											////////////////////////////////////////



long    SPitchCalc::Read_Players_NoteList_Audio_Delay_inPieSlices()
{

			//   for  PLAYER.exe  ONLY.      This can  DELAY or ADVANCE   the sync of the Midi Notes relative to the WAV.   8/2012


	long   noteListsDelay =    0;


	if(     m_noteListHardwareDelayForPlayerAddr  !=  NULL   )
		noteListsDelay  =    *m_noteListHardwareDelayForPlayerAddr;
	else
		noteListsDelay  =  0;


	return   noteListsDelay;
}



											////////////////////////////////////////


long   SPitchCalc::Get_PlayNoteList_Audio_Delay_InPieSlices()
{

		//	 CALLED By :     Search_NoteList_for_AudioMusical_Values()


		//  ******   Use this to FineTune  to SYNC of Audio and Midi for NOTELIST Play,  in  PLAYER  and NAVIGATOR      [  NEW,  7/2012  ]    *****
		//
		//   Works by  "Offsetting In Time"   the  "READ of the NoteList"   in  Search_NoteList_for_AudioMusical_Values().  


		//  ***  Now this can work  INDEPENDANTLY of  Note-SELECTION,  and values in   Get_NoteList_Position_Delay_InPieSlices()   8/2012   ***



	long    weirdHardwareDelayPieSlices =   -1;      

	short   appCode =   Get_PitchScope_App_Code_GLB();     




	if(    appCode  ==  1    )    //   1 :  NAVIGATOR
	{

	
		weirdHardwareDelayPieSlices =   8  ;   //   8   or   6       [  use 22 or other numbers to test INDEPENDANCE  
			

		//  TEST by loading a good NoAudio NoteList,   and listen to the SYNC of Midi verses WAV play.   BUT user can also change SYNC with SliderControl.    8/2012


				//    NoAudio file tests:    10[ not bad  ]       8[ good ]      6[ good ]       4[ not bad ]      12[ off ]   ...but still have  SELECTION-ALAIGNMENT  to deal with. 
	}
	else
	{                                     //   0 :   PLAYER                  

		weirdHardwareDelayPieSlices =    Read_Players_NoteList_Audio_Delay_inPieSlices();    //   ***INIT:   Fido[ 0 ]    Sparky[ 5 ]     Lassie [  10 ]      8/2012


				//  NoAudio file tests:       0[ best? ]       -4[ ok ]      -2[ good ]     3[ better ]      -8[ off ]        7[  off  ]      12[ real bad
	}


	return  weirdHardwareDelayPieSlices;
}




											////////////////////////////////////////


long   SPitchCalc::Get_PlayNoteList_Audio_Delay_InSamples()
{																							
																						//  ***   in SAMPLES,    with  PieSlice SAMPLE-Count   always  at SPEED 1   ****


	long   weirdHardwareDelayPieSlices  =    Get_PlayNoteList_Audio_Delay_InPieSlices();    


	long	 samplsInPieSliceSpeedOne     =    Calc_Samples_In_PieEvent(  1  );    // ***  NO ReDux  for speed.  In Virtual samples.   8/2012



	long     delayInSamps  =       weirdHardwareDelayPieSlices    *    samplsInPieSliceSpeedOne;     //  always is   6624     [  6624 =   1104 * 6 ]
	return  delayInSamps;
}





											////////////////////////////////////////
											////////////////////////////////////////


long   SPitchCalc::Get_NoteList_Position_Delay_InSamples()
{																							
																						//  ***   in  VIRTUAL  SAMPLES,    with  PieSlice SAMPLE-Count   always  at SPEED 1   ****

		//    Solves a PROBLEM  with   CUT Note and  UNDO Note
		//
		//	   How TEST Value :   Perform  CUT and UNDO at  SLOW Speeds with a value other than '8'  in  Get_NoteList_Position_Delay_InPieSlices() and it will FAIL  8/2012



		//	 CALLED By :       SPitchCalc::ReDraw_DrivingViews_OffMap_from_NoteList(),     	SPitchCalc::Cleanup_FinalCircQue_from_NoteList()  

	

	long   weirdHardwareDelayPieSlices  =    Get_NoteList_Position_Delay_InPieSlices();    



                                                                                     //     Speed  1   is necessary for  CUT and UNDO note at  SLOW-Speeds

	long	 samplsInPieSliceSpeedOne =    Calc_Samples_In_PieEvent(    1    );             //   In VIRTUAL samples.  




	long     delayInSamps  =       weirdHardwareDelayPieSlices    *    samplsInPieSliceSpeedOne;     //  always is   8832     [  8832 =   1104 * 8 ]
	return  delayInSamps;
}



											////////////////////////////////////////


long   SPitchCalc::Get_NoteList_Position_Delay_InPieSlices()
{

		//    Solves a PROBLEM  with   CUT Note and  UNDO Note
		//
		//	   How TEST Value :   Perform  CUT and UNDO at slow speeds with a value other than  '8'   and it will FAIL


		//	 ONLY  CALLED By :     Get_NoteList_Position_Delay_InSamples 



		//  ***  Now this can work  INDEPENDANTLY of  Note AUDIO-DELAY, and values in   Get_PlayNoteList_Audio_Delay_InPieSlices()   8/2012   ***


	long    weirdHardwareDelayPieSlices =   -999;      


	short        appCode =   Get_PitchScope_App_Code_GLB();     

	ASSERT(   appCode  !=  0   );    // ***** ONLY  Navigator functions should clall this.   8/2012



	if(    appCode  ==  1    )    //   1 :  NAVIGATOR  Only
	{

		weirdHardwareDelayPieSlices =   8;       //      8[ PERFECT ]          9[ little off]    7[ little off]    6[ little off ]      0[ bad ]      22[ bad ]    
	}


	return  weirdHardwareDelayPieSlices;
}




											////////////////////////////////////////
											////////////////////////////////////////


long		SPitchCalc::Get_DetectionDelay_InSamples()
{

			//   Only CALLED  BY:     Estimate_ScalePitch()     ...is now very accurte   8/2012



	long   sampsInPieEventBySpeed  =     Calc_Samples_In_PieEvent(  m_playSpeedFlt  );     //   NOT at Speed 1  !!!!   Is DEPENDANT on Speed


	long   delayPieSlices                   =     Get_DetectionDelay_InPieSlices(); 



    long      detectionDelayInSamps      =     delayPieSlices   *  sampsInPieEventBySpeed; 
	return   detectionDelayInSamps;
}



											////////////////////////////////////////


long   SPitchCalc::Get_DetectionDelay_InPieSlices()
{

				//     ONLY to be CALLED by  Get_DetectionDelay_InSamples() 


	long   primeCircQuesDelayNotes  =    Calc_Primary_CircQues_Delay_In_CalcedNotes();


	long   logDFTDelayInNotes           =    Calc_LogDFTs_Delay_In_CalcedNotes();      //   3


	long   harmPairsMapDelay  =    1;     //  **** HARDWIRED,  but this is not likely to change ****



	long      newCalcedDelayInPieSlices   =      logDFTDelayInNotes    +   harmPairsMapDelay   +    primeCircQuesDelayNotes;  
	return   newCalcedDelayInPieSlices;
}



											////////////////////////////////////////


long	  SPitchCalc::Get_logDFTs_Write_Column()
{


	long   virtLastColumn =   10,     xWrite =  -1;   // *****************  HARDWIRED  9/2012  ************************



	if(   m_useDFTrowProbeCircQue    )
	{

		ASSERT(    m_hiResRatioLogDFT  >=  1     &&      m_hiResRatioLogDFT  <=    kMAXhiResValueLogDFT   );

		xWrite     =     (  virtLastColumn  *   m_hiResRatioLogDFT  )    -1;    //   1[ 9 ]     2[ 19 ]      3[ 39  ]  
	}
	else
		xWrite =   virtLastColumn  -1;    //   9   always

	
    
	return   xWrite;
}




										////////////////////////////////////////


long	  SPitchCalc::Get_logDFTs_Read_Column()
{


	long   virtReadColumn =   6,     xRead =  -1;     // *****************  HARDWIRED  9/2012  ************************

   

	if(   m_useDFTrowProbeCircQue    )
	{

		ASSERT(    m_hiResRatioLogDFT  >=  1     &&      m_hiResRatioLogDFT  <=    kMAXhiResValueLogDFT   );


		xRead   =     virtReadColumn    *  m_hiResRatioLogDFT;      //   1[ 6 ]     2[ 12 ]      3[ 18  ]  
	}
	else
		xRead   =     virtReadColumn;   //   6   always



	return   xRead;
}


											////////////////////////////////////////


short	  SPitchCalc::Get_logDFTs_Read_Kernal_Width()
{

	long   width;  


	if(   m_useDFTrowProbeCircQue    )
	{
		width  =  5;      // *****************  HARDWIRED  9/2012  ************************
	}
	else
		width  =  3;      // *****************  HARDWIRED  9/2012  ************************

    
	return   width;
}



										////////////////////////////////////////


void     SPitchCalc::Get_logDFTs_Octave_Zone(    long&  retXstart,    long&  retXend    )
{


	long   virtOctaveStartColumn =   3;     //              *****************  HARDWIRED  9/2012  ************************
															//    3[ long time,  tried 5 but did NOT seem as good  ...too many LOW mistakes ( read below )
															//
															//                          (  remember, the virtual read pixel is '6'  )
		/***
				A POSSIBLE problem with  '3' ,   is that  it tens to make more BAD guess making the Octave lowere than it really is     9/2012

		***/


	retXstart  =   retXend  =   -1; 



	if(    m_useDFTrowProbeCircQue    )
	{

		ASSERT(    m_hiResRatioLogDFT  >=  1     &&      m_hiResRatioLogDFT  <=    kMAXhiResValueLogDFT   );


		retXstart  =     virtOctaveStartColumn   *   m_hiResRatioLogDFT;     //   1[ 3 ]     2[ 6 ]      3[ 9  ]  
	}
	else
	  retXstart  =     virtOctaveStartColumn;    

	

	retXend   =     Get_logDFTs_Write_Column();
}





											////////////////////////////////////////
											////////////////////////////////////////


bool	SPitchCalc::Estimate_ScalePitch(   long  currentSampleIdx,    bool  isPlayingBackward,    CalcedNote&   retCurrentNoteBackwards,    
												                bool  useNavigatorDelay,     long  pieSliceIdxAtDetection,    
												                short  fromPreRoll,    long  pieSliceCounter,    CString&  retErrorMesg   )
{
	/*
		Pitch Detection is really done in 2 STAGES here:
			a)  First the ScalePitch is detected ( 'ScalePitch' has 12 possible pitch values:  { E, F, F#, G, G#, A, A#, B, C, C#, D, D# }   )
			b)  After ScalePitch is determined, then the Octave is calculated by examining all the harmonics for the 4 Octave Candidates



		In my opionion the best way to do difficult detection is to create as many STAGES as possible -- each Stage will find a separate Property. 
		The first step is break down the detection-entity into as many Properties as possible.
		
		With multiple stages, detection can be INCREMENTALLY solved by finding the easiest properties first, and then go after the more difficult 
		properties later. In this case I modified Martin Piszczalski's original algorithm to only find 12 values (i.e. ScalePitch), and later used
		a separate algorithm to detect the Octave of the note within a second stage.

		The problem with Piszczalski's original algorithm, and artificial intelligence and/or neural networks' approaches to pitch detection, is that  
		they try to do too much at once -- that is, in a single stage. Piszczalski tried to detect both Octave and ScalePitch at the same time, however 
		my 2-stage method is more accurate.

		For a while I tried to conceive of a 3-stage algorithm for pitch detection that would first find a more basic Harmonic-Entity that would precede
		the determination of ScalePitch -- because the Fundamental Pitch (First) and its Fifth and Fourth are often confused because they sound very similar.
		That new primitive Harmonic-Entity would CONTAIN the 3 possibilities: { First, Fifth, Fourth }. Then a later stage would determine which 
		of the three is correct. This is analogous to the way that the ScalePitch entity CONTAINS the 4 Octave possibilities.


		Also study  PitchPlayerApp::Make_NoteList_No_Audio() for simple execution of pitch detection.

		James Millard
	*/


		//   returns     -1 :            Too weak to make a detection
		//                 { 0 - 11  }    A detected scalePitch value was sucessfully found
		//				     <= -2         an Error has occured

		//   'retSynthCode' :   -1:  Do nothing      0: shut off synthesizer     1: Start synthesizer with current note 

		//   'retCurrentNoteBackwards'   is only for BACKWARDS play.  It returns the  CalcedNote that was just calculated.    11/11


						//  note:   The  DFTmap  must have values calced before this function is called.   1/2010


	bool    noHarmPairsPixelAveraging  =   false;   //   false  is default for years      ********************   ADJUST  11/11  ************************ 

	
						// ****** ADJUST,  but a Value= 6 causes a   'MEMORY ERROR'  with the PlayTool( really? 7/06 )  ....think I am writting OFF the Offmap memory  5/02 ************
	short   numberComponents =   m_componentCountHPairs;    //    4,   7       ONLY[  4, 5, 6, 7, 8, 9,  10 ]        10[ bad resolution, misses some notes ]

						//   WIERD, but in real time it almost looks like 4 is the best ( isn't that what Klapuri said??  )   3/10


	long    spikesWeight    =    4;     // ***** Does NOT need to be adjusted cause no other weights are really used   1/10


									// **** This value is NOT RELEVANT [12/09]  because it only affects 'retPixCount' which is for density score that is no longer WEIGHTED ******
	short    pixThresholdHPairs =    15;     //    15  ****** ADJUST, but NOT USEED *****  ..when couning how many rows of a HPairs-Cylinder are significant, their pixes must be over this value
										// ****  Is this value really used,  because 






	retErrorMesg.Empty();	//   init for fail


	long	 sampsInPieSpeedOne      =    Calc_Samples_In_PieEvent(  1  ); 
	long   sampsInPieEvent             =    Calc_Samples_In_PieEvent(  m_playSpeedFlt  );

	long    detectionDelayInSamps  =    Get_DetectionDelay_InSamples();    //   NEW,  now very accurate   8/2012




	CalcedNote  newDetectedNote;

	newDetectedNote.scalePitch    =   -1;
	newDetectedNote.octaveIndex =   -1;
	newDetectedNote.detectScoreHarms =  -1;
	newDetectedNote.detectAvgHarmonicMag =  0;		
	newDetectedNote.synthCode  =   -9;    
	newDetectedNote.primaryQuesSampleIdx =    currentSampleIdx;   //  Record what time this was detected, used in Get_New_Filtered_ReturnNote_CircularQue()  
//	newDetectedNote.detectScoreOctave =  -1;
	for(    short oct =0;    oct <  kCountOfNavOctaveCandidates;    oct++   )
			newDetectedNote.detectScoreOctaveCandids[  oct  ]  =   -1;

//	newDetectedNote.beginingSampleIdxFile  =     currentSampleIdx;	Will get  calced/assigned later   by   Get_New_Filtered_ReturnNote_CircularQue(




	retCurrentNoteBackwards.scalePitch    =   -1;    //  For BACKWARDS play we want to hear the Just-Calculated Note, wihout the DELAY from the FinalCircQue  11/11
	retCurrentNoteBackwards.octaveIndex =   -1;
	retCurrentNoteBackwards.detectScoreHarms =  -1;    
	retCurrentNoteBackwards.detectAvgHarmonicMag =  -1;		
	retCurrentNoteBackwards.synthCode  =   -1;
//	retCurrentNoteBackwards.detectScoreOctave =  -1;
	for(    short oct =0;    oct <  kCountOfNavOctaveCandidates;    oct++   )
			retCurrentNoteBackwards.detectScoreOctaveCandids[  oct  ]  =   -1;
//	retCurrentNoteBackwards.primaryQuesSampleIdx =    currentSampleIdx;		 // ********  UNECESSARY,  not relevant???  **********************
//	retCurrentNoteBackwards.beginingSampleIdxFile    =     currentSampleIdx;	     //  ???????




	if(   m_logDFTtransform ==  NULL   ||    m_harmpairsTransform ==  NULL   )
	{
		retErrorMesg =  "SPitchCalc::Estimate_ScalePitch FAILED:   m_logDFTtransform  or  m_harmpairsTransform is  NULL " ;
		return  false;
	}

	ASSERT(  m_detectionSensitivityThreshold  > 0   );




	long     retLostNotesCount =  -1;   //  Check to see that this NEW CALCNote will have a CONTINUOUS TimeStamp in FinCircQue  by the  'currentSampleIdx'  parm   3/12
	CString  functionCode =  "Es";


	if(    ! Is_CurrentSampleIdx_Continuous(   currentSampleIdx,   pieSliceIdxAtDetection,   isPlayingBackward,    useNavigatorDelay,    retLostNotesCount,   fromPreRoll,  functionCode  )      )
	{

		int   dummy =   9;   // **** WHEN gets hit ???		3/1/12[ PreRoll]    3/1[move file slider]

				      // *** With the addition of  Post_Roll_NoteDetect(),  Add_False_CalcNotes_for_OutOfSync  should never have to be called again.   2/29/2012  ***

//		Add_False_CalcNotes_for_OutOfSync(   lostNotesCount,   currentSampleIdx,    isPlayingBackwards,    retCurrentNote   );    // CAREFUL !!!!
	}



																//   A)	  Use the   HarmPairs-VERTER   to fill in the values to the HPairs Transform 




//	short   filterCodeDFT =   ConvolutionMapFilter::DIALATEhORZ;    //   ( BAD )  will actually ERODE because of the  0 value for BLACK   9/12

//	short   filterCodeDFT =   ConvolutionMapFilter::ERODEhORIZ;   //  (  ok, but not that good  9/12 )     will actually DIALATE ( make Harmonics longer ) because of the  0 value for BLACK on GreyScale offmaps.   9/12

//	short   filterCodeDFT =   ConvolutionMapFilter::NONE;     //  Now with HI-RES, this seems good?!?!         This does NOT even to make much difference with Navigator   11/11


//	short   filterCodeDFT =   ConvolutionMapFilter::BLURHORZ;      //  ( good even with 3 wide kernal  )seems the most accurate for PepleStrange, others    8/2012


	short   filterCodeDFT =   ConvolutionMapFilter::MEDIANHORZ;   //  ( might be BEST, need the wide HI-RES kernal  )  resolution might be sharper, but makes more mistakes   8/2012
																								//
																								//   MEDIAN sorts the Grey-VALUES in order( BUBBLE SORT ),  and picks the MIDDLE value.
																							    //                                         EX:   {  3    56    '123'    230   250  } =   123    
																								//
																								//   Unlike the  BLUR(averaging)  filter,  the MEDIAN filter IGNORES the EXTREME values in its CALCULATIONS.


	long   dftStartPixel,   dftEndPixel;


	short  kernalWidthDFT  =    Get_logDFTs_Read_Kernal_Width();    //   5 for Navigator,   3 for Old PitchScope


	Get_logDFTs_Octave_Zone(   dftStartPixel,    dftEndPixel   );     //  for OCTAVE Pick    This is the DFTmap SEGMENT to gather HarmonicMags for OctaveCalc


	ASSERT(    dftStartPixel >= 0    &&    dftStartPixel <  m_dftMapsWidth     );    
	ASSERT(    dftEndPixel  >= 0     &&    dftEndPixel   <  m_dftMapsWidth    ); 


	ASSERT(  m_hpairsMapsWidth == 3   &&    m_dftMapsWidth  >=  3   );




	short    hPairsWriteColumn =   2;   //  ALWAYS



	short	  maxValueHp0  =	 m_harmpairsTransform->Copy_Xcolumn(      1,       0    );
	short	  maxValueHp1  =	 m_harmpairsTransform->Copy_Xcolumn(      2,       1    );

	m_harmpairsTransform->Assign_Xcolumn(   2,    0   );     //   erase the values in  column #2,  the write-column



	HarmPairsVerter  *harmsVerter  =   new   HarmPairsVerter(   *m_logDFTtransform,   m_harmpairsTransform,   numberComponents,  filterCodeDFT,   kernalWidthDFT   );  
	if(   harmsVerter  ==  NULL   )
	{
		retErrorMesg =  "SPitchCalc::Estimate_ScalePitch FAILED:   harmsVerter is  NULL " ;
		return  false;
	}
	

	m_harmpairsTransform->m_pixelReadOffset  =    0;   
	m_harmpairsTransform->m_pixelReadWidth  =    1;    

//	m_harmpairsTransform->m_inFloatmapMode =   true;  **** NO!!!	 ( but I do this for logDFT 9/12 )     12/2009	[  so that when HarmPairsVerter reads the logDFT, it will read with a KERNAL at x=1 column  ]

		harmsVerter->Set_ReadOffset(  0  );     // This NOW works in conjunction with  'm_overideDFTreadColumn'  below     

		harmsVerter->m_overideDFTreadColumn  =     Get_logDFTs_Read_Column();

		harmsVerter->m_hiResRatioLogDFT          =     m_hiResRatioLogDFT;   // *** NEW,   9/12   [   OK for OLD algo,  it just ignores this value ther algo ]



// ********************   HARDWIRED  *****************************

	double  harmPairsPixelBritenessFactor  =  75.0;     //   100.0   Controls the BRIGHTNESS of the PIXELS in the HarmPairsMap.  Does not impact the source calcs.
																		    //               but changing this impact the values on the RESPONSE SLIDER.   9/2012
																			//    100.0[ too bright? ]
//	if(   m_useDFTrowProbeCircQue   )
//	{
//		harmsVerter->Set_Volume(   harmPairsPixelBritenessFactor  );          
//	}
//	else
		harmsVerter->Set_Volume(   harmPairsPixelBritenessFactor  );   //  now use same value for both DFTrowProbe Algos   



		//    HarmPairsVerter calculates the ScalePitch ( 'ScalePitch' has 12 possible pitch values:  { E, F, F#, G, G#, A, A#, B, C, C#, D, D# }   )

	harmsVerter->Transform_Column(  hPairsWriteColumn  );    


	delete  harmsVerter;     
	harmsVerter =   NULL;





																//   B)	 Calc the  'SPitch value'   by looking for maximums in the HPairsTransform's vertical columns
	short     bestAvgToneScore     =  -1;
	short     bestAvgToneChannel =  -1;


	for(    short  channelIdx =0;     channelIdx <   12;       channelIdx++    )             //   from  CompositeVerter::Transform_Column()
	{																		

		short   retPixCount,   avgTone;  
					//  Have   three HORIZONTAL values   of   columnTone{0,1,2}   so   we can   get an average 3-pixelWide read   from  HarmPairs transform

		
		short    columnTone0 =    m_harmpairsTransform->Get_Xcolumns_Channel_Value(    0,   channelIdx,   
																																	pixThresholdHPairs,    //  ** NOT used anymore 12/09 ***  m_harmPairsMap.m_pixelThresholdHPt, 	 
																																	retPixCount   ); 

		short    columnTone1 =    m_harmpairsTransform->Get_Xcolumns_Channel_Value(    1,   channelIdx,   
																																	pixThresholdHPairs,   
																																	retPixCount   ); 

		short    columnTone2 =    m_harmpairsTransform->Get_Xcolumns_Channel_Value(    2,   channelIdx,   
																																	pixThresholdHPairs,     
																																	retPixCount   ); 


		if(    noHarmPairsPixelAveraging   )
		{
			avgTone  =    columnTone1  * 3;    // will this give more accurate results??    DEFINATELT NO,  
		}
		else
		{
		//	avgTone =          columnTone0   +  columnTone1    +  columnTone2;      // don't bother to divide by 3, we are just looking for a max value

			avgTone =     (    columnTone0  +   ( 3 * columnTone1 )   +  columnTone2    ) /2;         //  sharper focus

	//		avgTone  =     (   columnTone1   +  columnTone2 )  * 2;    //  Hard to say,  this might be more accurate.   1/21/10
		}



	//   MAYBE:   I could aslo keep track of the 2nd Best Score  and then compair the dirfference with the 1st candidate. A large difference would mean more conficence???   11/11

	// ***** CHORD approximation:  With a 2nd BEST candidate( or more )  I could also test for strongest  5th Pair  ( D-A,  G-D,  A-E ) and use that for a CHORD approximation.


		if(   avgTone  >  bestAvgToneScore   )   
		{
			bestAvgToneScore     =    avgTone;   //  ***  This is the DETECTION SCORE  that the Sensitivity Slider will throttle.  9/2012  ***

			bestAvgToneChannel  =   channelIdx;
		}

	}   //  	for(    short  channelIdx =0;     channelIdx <   12;  






																	//   C)	 If get a BAD SCORE,  then turn off any playing notes and EXIT  
	 CalcedNote   retCurrentNote;  



	if(     bestAvgToneScore   <   m_detectionSensitivityThreshold    )   //   LESS than the threshold, and so we try to turn off any playing notes
																									 //
	{																								 //    m_detectionSensitivityThreshold:   RANGE is  { 1  to  101 }   9/2012  
																									 //         ...So a ResponseSlider setting of 100, gives 1, which means any
		                                                                                             //             detection score whtsoever is converted to a Midi Note.

		   newDetectedNote.scalePitch  =   -1;          //   -1  No error, but the detection was too weak

		   newDetectedNote.detectScoreHarms         =     bestAvgToneScore;    //  was 0,  OK if we send out the score

  	       newDetectedNote.primaryQuesSampleIdx  =    currentSampleIdx;	     //  assign here or up above

		   newDetectedNote.pieSliceIdxAtDetection    =    pieSliceIdxAtDetection;    //  'pieSliceIdxAtDetection'   is input parm  for  Event-Number    ( also know as  FakeEvent  )


		Add_New_Note_to_CircularQue(   newDetectedNote   );   





		Get_New_Filtered_ReturnNote_CircularQue(   retCurrentNote  );     // ( sets the 'synthCode here )    this sets all our returned values for this funct

				retCurrentNote.curSampleWaveman         =     currentSampleIdx;

				retCurrentNote.beginingSampleIdxFile    =     currentSampleIdx   -    detectionDelayInSamps;   //  BEST     7/19/12

				retCurrentNote.pieSliceIdxAfterFiltration =    pieSliceIdxAtDetection;      //  NEW,  really just for debugging

			    retCurrentNote.pieSliceCounter            =     pieSliceCounter;  


		retCurrentNoteBackwards  =   retCurrentNote;     //  Now  RETURN our reults



		if(    useNavigatorDelay   )   //   1:    Navigator  
		{

			Add_New_Note_to_Final_CircularQue(   retCurrentNote   );  																       

			Update_DrivingViews_OffMap_with_Nudge(   retCurrentNote,   isPlayingBackward   );
		}

		return  true;
	}



	                 //  D)   Have a GOOD SCORE ( more than the threshold ),   so we will  TURN ON a NEW playing note
  


	newDetectedNote.scalePitch        =     bestAvgToneChannel;	    //   Return the good scores

	newDetectedNote.detectScoreHarms  =     bestAvgToneScore;      // sometimes as big a 300,   usually is about  80




																//  E)   Calculate the  OCTAVE  for the found scalePitch
 

	if(    newDetectedNote.scalePitch  >=  0    )     //  Calc this for ALL good  DetectionScores because I want to compair this info in TRACE functions of calling function   1/2010
	{										                           //  .....But we should really only calc Octaves for the START of a new note.     1/10
											    


		/******  NO,   for  NAVIGATOR and PLAYER  one   PIXEL (PieSlice)   is  1104  samples         9/2012

		long  startSample  =     dftStartPixel   *  m_horzScaleDown;       // *** CAREFUL,   m_horzScaleDown should be the same as  'chunkSize'  ******
		long  endSample    =     dftEndPixel     *  m_horzScaleDown;       //   Looks good so far.                *****  2048   or   better      ( dftEndPixel +1 ) -1  =  2559    ?????
		****/

		long   navChunkSize =    Get_ChunkSize();      //   always  1104  

		long  startSample  =     dftStartPixel   *  navChunkSize;       
		long  endSample    =     dftEndPixel     *  navChunkSize;      




		short  candidCount     =   4;
		bool   sameSPitchMode  =   true;
		bool   dynamicAllocFCandids =    true;


		long   topFrequencyLimit      =    4700;      //   NOT USED???  1/2010   These were from the filtering that was done by 'FundCandidCalcer' when it used 
		long   bottomFrequencyLimit =        82;      //									to do correlations, which it no longer does



		FundCandidCalcer    fCandCalcer(    candidCount,   
											sameSPitchMode,   
											startSample,     endSample,
										    dynamicAllocFCandids,   
											newDetectedNote.scalePitch,  
										   *m_logDFTtransform,   
											topFrequencyLimit,    bottomFrequencyLimit   );


		bool    enableMidiClosenessNeiborsFactor  =   false;
	    short   neighborhoodMidi   =   -1;    
		bool    retMarkSPitchRed   =  false;


				//   Now the Octave is calculated by examining all the possible Harmonics for all of the 4 Octave Candidates


		newDetectedNote.octaveIndex  =    fCandCalcer.Calc_Best_Octave_Candidate(   spikesWeight,
																					enableMidiClosenessNeiborsFactor,
																				    neighborhoodMidi,    
																					retMarkSPitchRed,   
																					retErrorMesg  );    



		newDetectedNote.detectAvgHarmonicMag   =     fCandCalcer.m_chosenFundamentalsAvgHarmonicMag; 	

//		newDetectedNote.detectScoreOctave      =     fCandCalcer.m_bestFinalScore;  


		for(    short oct =0;    oct <  kCountOfNavOctaveCandidates;    oct++   )     //   Now have 4 scores to save.
		{

			short  candidsScore  =   fCandCalcer.m_fundCandids[  oct  +3  ].m_finalOctvCandidScore;    // ***  +3    to make it a Virtual-Idx (cause of Sub-Octaves )   2/12  **********

			newDetectedNote.detectScoreOctaveCandids[  oct  ]  =    candidsScore;
		}
	}   
										



		newDetectedNote.primaryQuesSampleIdx    =    currentSampleIdx;	           //  assign here or up above,  is used by Get_New_Filtered_ReturnNote_CircularQue()

		newDetectedNote.pieSliceIdxAtDetection  =    pieSliceIdxAtDetection;      //  'pieSliceIdxAtDetection'   is input parm  for  Event-Number     ( also know as  'FakeEvent'  )


	Add_New_Note_to_CircularQue(   newDetectedNote   );





	Get_New_Filtered_ReturnNote_CircularQue(  retCurrentNote  );   //  Acts like a Linear MEDIAN filter. (  'retCurrentNote' is INITIALIZED with values of OLDEST TargSPitch Note in CircQue

		retCurrentNote.curSampleWaveman     =    currentSampleIdx;

		retCurrentNote.beginingSampleIdxFile  =    currentSampleIdx   -    detectionDelayInSamps;   //   BEST    7/19/12

		retCurrentNote.pieSliceIdxAfterFiltration =    pieSliceIdxAtDetection;      //  NEW,  really just for debugging

	    retCurrentNote.pieSliceCounter             =     pieSliceCounter;  



	retCurrentNoteBackwards  =   retCurrentNote;     // copy note-data to  'RETURN  parm'




	if(   useNavigatorDelay   )    //  1:  Navigator  
	{

		Add_New_Note_to_Final_CircularQue(  retCurrentNote   );      //  The note will receive its  time-DELAY  in the adjustable  Final-CircQue. 
											  
		Update_DrivingViews_OffMap_with_Nudge(    retCurrentNote,    isPlayingBackward   );    //  This draws the colored note to the OffMap
	}


	return  true;
}



											////////////////////////////////////////


void	   SPitchCalc::Initialize_for_Play()
{

		 //  Call this at the start of play(Player or Navcigator).  These vars are  used in    SPitchCalc::Get_New_Filtered_ReturnNote_CircularQue()     8/2012



	//  CALLED BY:   PsNavigatorDlg::Do_PreRoll_Redraw_Display(),    PsNavigatorDlg::Change_Midi_Source(),  
	//
	//				                EventMan::Finish_Process_For_PPlayer(),    	NoteGenerator::Init_NoteGenerator(),    BitSourceStreaming::Initialize_For_Playing() 										



	//   INDIRECTLY :   PsNavigatorDlg::On_BnClicked_Play_Window_Audio_Button(),  On_ReversePlay_Button(),    On_ContinuePlay_Button(),   RePlay_a_Lick(),  
	//
	//										Do_PreRoll_Redraw_Display(),				PsNavigatorDlg::Change_Midi_Source()




	m_aNoteIsPlaying  =   false;   //  used in  SPitchCalc::Get_New_Filtered_ReturnNote_CircularQue()


	m_prevFoundNote =    -1;      //  used in  SPitchCalc::Get_New_Filtered_ReturnNote_CircularQue()


	m_prevPlayingNotesAvgHarmonicMag  =    0;	    // need this for  m_letNotesSustain =  true
}



											////////////////////////////////////////


void		SPitchCalc::Initialze_For_File_Position_Change()
{

		//  Also call this  for a CHANGE of DIRECTION   ... should get called for any PreRoll execution.  9/2012


													//  **********************  NEW,  add more to this funct  9/3/2012  *****************


	Initialize_All_DFTprobe_CircularQues_ArrayElements_and_Sums();   //  Might take a while ... there are THOUSANDS of Array ELEMENTS,  but now use memset()

//					When TESTING this,  remember that with the stop and ContinueButton(with Erase of CircQue sums) the EFFECT would be seen at the
//					TOP of a Vertical Driving view,  where NEW Notes are first being formed.   9/4/2012
//
//					Go to PsNavigatorDlg::Scroll_ThinColumn_to_Driving_View() and have it display the LogDFT while playing -- that would be the easiest way 
//					to see a POSSIBLE DISRUPTION by this to  LogDFT.   I have not seen such a disruption.   9/12
//
//					BUT,  this USED to happen on the PLAY Button, and I was afraid it might take too long and throw off some Lost Slices.  9/2012 
	




	/******  NEW funct,  9/3/2012    ....These are possible things to add to this function.  Many are being executed elsewhere.   


 	Initialize_SndSample_CircularQue();    //  1/12

	Erase_the_UndoNotes_Data();


	void		 Erase_CircularQues_and_DrivingBitmap();

	void		 Erase_logDFTmap_and_HarmPairsMap();   	//   NEW,   7/2012    Will this help problems with accuract of NoteDetect after a PreRoll ???? 



	Initialize_Notes_CircularQue();                                                ***  Does this do too much ????  ****

	Initialize_Notes_Final_CircularQue(   short  cuesSize  );        ***  Does this do too much ????  ****

	***/
}



											////////////////////////////////////////


bool   SPitchCalc::Search_NoteList_for_AudioMusical_Values(    long   currentSampleIdx,    CalcedNote&   retCurrentNote,   bool isPlayingBackwards,    
													                                           bool  useNavigatorDelay,   long  pieSliceIdxAtDetection,     																							   
																		                      long  expectedEventNumber,   long  realEventNumber,    long  pieSliceCounter,  
																			                                        short  fromPreRoll,    CString&  retErrorMesg )
{

			//   ONLY CALLED by  (audio functs):    EventMan::Process_Event_Notification_PPlayer()    and    EventMan::Process_Event_Notification_PPlayer_NoAudio()



			//   a   'PIE-SLICE'   is almost like a   'PIXEL'   in the  audio-rendering  process

			//   This is very SIMILAR to   SPitchCalc::Estimate_ScalePitch().      '.synthCode'   gets SET here.


	long   delayInSamps     =      Get_PlayNoteList_Audio_Delay_InSamples();        //  CAREFULL,   just for AUDIO   8/2012


	long   targetSampleIdx  =      currentSampleIdx   -   delayInSamps;



	bool	        goingBackwardALT   =         Is_Playing_Backwards();  

//	ASSERT(   goingBackwardALT  ==   isPlayingBackwards   );   **** FAILS in Notelist-Mode,  when just starting to go backwards    4/26



	retErrorMesg.Empty();

	retCurrentNote.scalePitch    =   -1;
	retCurrentNote.octaveIndex =   -1;
	retCurrentNote.detectScoreHarms =  -1;
	retCurrentNote.detectAvgHarmonicMag =  0;		
	retCurrentNote.synthCode  =   -9;  
	retCurrentNote.primaryQuesSampleIdx =    currentSampleIdx;   //  Record what time this was detected, used in Get_New_Filtered_ReturnNote_CircularQue()  
//	retCurrentNote.detectScoreOctave =  -1;
	for(    short oct =0;    oct <  kCountOfNavOctaveCandidates;    oct++   )
			retCurrentNote.detectScoreOctaveCandids[  oct  ]  =   -1;




	CalcedNote  newDetectedNote;

	newDetectedNote.scalePitch    =   -1;
	newDetectedNote.octaveIndex =   -1;
	newDetectedNote.detectScoreHarms =  -1;
	newDetectedNote.detectAvgHarmonicMag =  0;		
	newDetectedNote.synthCode  =   -9;    
	newDetectedNote.primaryQuesSampleIdx =    currentSampleIdx;   //  Record what time this was detected, used in Get_New_Filtered_ReturnNote_CircularQue()  
//	newDetectedNote.detectScoreOctave =  -1;
	for(    short oct =0;    oct <  kCountOfNavOctaveCandidates;    oct++   )
		newDetectedNote.detectScoreOctaveCandids[  oct  ]  =   -1;




	if(    m_calcedNoteListMasterApp  ==  NULL  )
	{
		retErrorMesg  =   "SPitchCalc::Search_NoteList_for_AudioMusical_Values  FAILED,     m_calcedNoteListMasterApp is NULL."  ;
		return  false;
	}


	short        retScalePitchIdx = -1,  retDetectScore = -1,  retAvgHarmonicMag = -1,   retOctaveIdx = -1,   retSynthCode =  -9,    retPixelBoundaryType =  -9;   
	MidiNote*  retFoundNote =  NULL;




	if(   targetSampleIdx  >=  0   )
	{

		if(    ! Get_Pixel_Info_from_Notelist(   targetSampleIdx,    retScalePitchIdx,   retDetectScore,   retAvgHarmonicMag,   retOctaveIdx,   m_calcedNoteListMasterApp,  
																		m_calcedNoteListMasterApp->m_lastFoundCalcNoteLink,  isPlayingBackwards, 	
																		        retSynthCode,   retPixelBoundaryType,   &retFoundNote,  retErrorMesg  )     )
		{
			ASSERT( 0 );
			return  false;
		}

	}
	else
	{	int   dummy  =   9;  }   // Land her when going to File Start   because of    EventMan::Process_Event_Notification_PPlayer_NoAudio()
	


	


//	long   notesPieSliceIdx =     pieSliceIdxAtDetection  -   primeCircQuesDelayNotes;

//	long   notesPieSliceIdx =     pieSliceIdxAtDetection;   //   Sames a Make_NoteList_No_Audio().  Since for DEBUG, better keep it simple.  
																			   //   'primeCircQuesDelayNotes'  can VARY if user changes the DetailSlider.  2/8/2012 


//	newDetectedNote.pieSliceIdxAtDetection   =    pieSliceIdxAtDetection;     //  NO, let old valye persist              // notesPieSliceIdx;     //  'pieSliceIdx'   is input parm  for  Event-Number  ( also know as  FakeEvent  )

	newDetectedNote.pieSliceIdxAfterFiltration  =   pieSliceIdxAtDetection;    

	newDetectedNote.pieSliceCounter             =     pieSliceCounter;   




// **************************************************************************************************************************
// ***********   WHY the HELL did I do this???   Was there a valid REASON???    This is for NoteList play so every thing SHOULD be shown.    9/9/2012

/****
	if(     retDetectScore   <   m_detectionSensitivityThreshold    )   //  OLD:  (  LESS than the threshold, and so we try to turn off any playing notes  )
		newDetectedNote.scalePitch =   -1;                                     //   OLD:  (  -1:  No error, but the detection was too weak  )	
	else
		newDetectedNote.scalePitch =   retScalePitchIdx;  
***/	
	newDetectedNote.scalePitch     =    retScalePitchIdx;  






	newDetectedNote.detectScoreHarms        =    retDetectScore;    //  was 0,  OK if we send out the score
	newDetectedNote.detectAvgHarmonicMag =    retAvgHarmonicMag; 
	newDetectedNote.octaveIndex                 =    retOctaveIdx;

//	newDetectedNote.primaryQuesSampleIdx =   currentSampleIdx;	 // *****    NOT USED after this function ????
	newDetectedNote.primaryQuesSampleIdx =   -118;






		// *******************  BIG  Time-STAMP  here   [ ...or NOT?   7/2012  ]   **************************************

	newDetectedNote.curSampleWaveman   = 	  currentSampleIdx;  



	newDetectedNote.beginingSampleIdxFile =     targetSampleIdx;    //     currentSampleIdx  -   (  weirdHardwareDelay  *  sampsInPieSpeedOne   );
																									//     currentSampleIdx  -   (    always is   6624  )
				//   Do NOT think that   .beginingSampleIdxFile   ever gets  used or read for this case [  Search_NoteList_for_AudioMusical_Values()  ]
		





	if(   m_aNoteIsPlaying    )          //   A)    a Note is PLAYING
	{


		if(    newDetectedNote.scalePitch  >=  0   )    
		{
								//   DETECTED a note

			if(    newDetectedNote.scalePitch   !=   m_prevFoundNote   )       //  Only if this is a DIFFERENT Pitch, will we mess with the synthesizer
			{							

				newDetectedNote.synthCode  =    1;    //  1:   turn on synth


				if(   retPixelBoundaryType  !=  1  )      //    1:   BEGIN Boundary 
				{	
					if(   retPixelBoundaryType  !=  2  )    //  2:   the last pixel in the Note   ...its END  Boundary 
					{  int   dummy =  9;  }    //                 sometimes get a 0   here,  at move of Slider,  and a lot at SlowedSpeeds								
				}    


				m_aNoteIsPlaying =   true;    	//		TRACE( "START  synthesizer    SPitchCalc" );

			} 		
			else
			{  newDetectedNote.synthCode  =   -1;   //   -1:  do nothing    


				if(   retPixelBoundaryType  !=  0  )      //    0:   Notes INTERIOR    ****** ...sometimes this is  2   ******/
				{	
					if(   retPixelBoundaryType  !=  2  )    //  2:   the last pixel in the Note   ...its END  Boundary 
					{  int   dummy =  9;   }		  //   could use   ASSERT( 0 );  Land here  with a '1'  during reverse play			
				}
			}
			

			m_prevFoundNote  =     newDetectedNote.scalePitch;    // ***********   OK   here  ????
		}
		else
		{                 //   could  NOT  DETECT a note,   bad detection score

			newDetectedNote.synthCode  =   0;     //  0:   turn OFF the playing note


			if(   retPixelBoundaryType  >=  0  )      //   < 0 :    No Note HIT
			{	int   dummy =  9;   }    //   could use   ASSERT( 0 );   


			m_prevFoundNote =    -1;      

			m_aNoteIsPlaying =   false;    
		}

	}   //  if(    m_aNoteIsPlaying


	else										//   B)    NOTHING is playing 
	{												

		if(    newDetectedNote.scalePitch  >=  0   )
		{												
						   //   DETECTED a note

			newDetectedNote.synthCode  =      1;     //   1: START playing a NEW note on synth


			if(   retPixelBoundaryType  !=  1  )      //    1:   BEGIN Boundary 
			{	
				if(   retPixelBoundaryType  !=  0 )      //  Land here when stop and restart   with a value of  '0',  think it is if Play was stopped in the middle of a note.
				{   int   dummy =  9;   }      //     sometimes get a 2   here,  with Play Start after a  move of Slider	
			}    
		

			ASSERT(   m_prevFoundNote  < 0  );  	 //		TRACE( "START  synthesizer    SPitchCalc" );
			m_prevFoundNote  =   newDetectedNote.scalePitch; 


			m_aNoteIsPlaying =   true;     //   show that  synthesizer will NOW be playing with the new PITCH  ( calling function will automatically turn off old note and start new one )													
		}
		else
		{              //   could  NOT  DETECT  

			newDetectedNote.synthCode =   -1;
			

			if(   retPixelBoundaryType  >=  0  )      //   < 0 :    No Note HIT
			{	int   dummy =  9;   }      //   could use   ASSERT( 0 );   

			ASSERT(  m_prevFoundNote  ==   -1   );

			m_prevFoundNote =    -1;   //  -1:   nothing was found ( NO scalePitch )


			m_aNoteIsPlaying =   false;    
		}   

	}   //  end of   NOTHING is playing 


	

/******
	Add_New_Note_to_CircularQue(   newDetectedNote   );    ******************   BAD,   I was applying the MEDIAN FILTER twice.  
	
	Get_New_Filtered_ReturnNote_CircularQue(   retCurrentNote,   notesPieSliceIdx,    pieSliceIdx   );     // ( sets the 'synthCode' here )    this sets all our returned values for this funct
		//     TRACE(  "Notes  [ pieClock  %d  ]   Virt-PieSlice [  %d  ]     note.beginingSampleIdxFile [  %d  ]  \n ",     pieSliceIdx,   retCurrentNote.pieSliceIdx,    retCurrentNote.beginingSampleIdxFile   );

*****/



	retCurrentNote  =    newDetectedNote;    //   RETURN  results,   for Backwards play,  and Player     12/11



	newDetectedNote.expectedEventNumber  =     expectedEventNumber;

	newDetectedNote.realEventNumber          =     realEventNumber;


	

//	newDetectedNote.pieSliceIdxAfterFiltration  =   pieSliceIdxAtDetection;    //   NEW,  

//	newDetectedNote.pieSliceCounter             =     pieSliceCounter;    done above



//   Search_NoteList_for_AudioMusical_Values()




	long    retLostNotesCount =  -1;     //  Check to see that this NEW CALCNote will have a CONTINUOUS TimeStamp in FinCircQue  by the  'currentSampleIdx'  parm   3/12

	CString  functionCode =  "Sn";


	if(    ! Is_CurrentSampleIdx_Continuous(   currentSampleIdx,   pieSliceIdxAtDetection,    isPlayingBackwards,    useNavigatorDelay,    	retLostNotesCount,    
																																		 fromPreRoll,   functionCode  )      )
	{

		int   dummy =   9;   // **** WHEN gets hit ???		
				// *** With the addition of  Post_Roll_NoteDetect(),  Add_False_CalcNotes_for_OutOfSync  should never have to be called again.   2/29/2012  ***

//		Add_False_CalcNotes_for_OutOfSync(   lostNotesCount,   currentSampleIdx,    isPlayingBackwards,    retCurrentNote   );    // CAREFUL !!!!
	}




	if(   useNavigatorDelay   )   
	{

		Add_New_Note_to_Final_CircularQue(   newDetectedNote   );  	

		Update_DrivingViews_OffMap_with_Nudge(   newDetectedNote,   isPlayingBackwards   );
	}

	return  true;
}



										////////////////////////////////////////////////


void    SPitchCalc::Add_False_CalcNotes_for_OutOfSync(   long  noteCount,   long  currentSampleIdx,   bool isPlayingBackwards,  CalcedNote&  currentNote    )
{



	if(   noteCount  <=  0  )
		return;


	bool	 goingBackwardALT  =     Is_Playing_Backwards();  

	ASSERT(   goingBackwardALT  ==   isPlayingBackwards   );





	long	sampsInPieSlice  =		 Calc_Samples_In_PieEvent(  m_playSpeedFlt  );  

	CalcedNote  retNotePrevious;  
	Get_Newest_Note_from_Final_CircularQue(   retNotePrevious   );	


	long  curSampleWavemanPrevNote     =     retNotePrevious.curSampleWaveman;

		curSampleWavemanPrevNote  +=    sampsInPieSlice;   // *****   CHANGE for BACKWARDS Play ******


	long  pieSliceIdxAtDetectionPrevNote  =     retNotePrevious.pieSliceIdxAtDetection;

		pieSliceIdxAtDetectionPrevNote++;	      // *****   CHANGE for BACKWARDS Play ******


	long  beginingSampleIdxFilePrevNote =     retNotePrevious.beginingSampleIdxFile;

		beginingSampleIdxFilePrevNote    +=    sampsInPieSlice;   // *****   CHANGE for BACKWARDS Play ******



	short  scalePitchBoth =  -1;

	if(          retNotePrevious.scalePitch  ==   currentNote.scalePitch   
		  &&   currentNote.scalePitch  >=  0     )
		scalePitchBoth =   retNotePrevious.scalePitch;




	for(   short  nt = 0;    nt < noteCount;     nt++   )
	{

		CalcedNote   nuCalcNote;

		nuCalcNote =   retNotePrevious;      //  for a start,  INITIALIZE with previous notes values

			nuCalcNote.synthCode =  -1;
			nuCalcNote.beginingSampleIdxFile  =   beginingSampleIdxFilePrevNote;  
			nuCalcNote.curSampleWaveman    =    curSampleWavemanPrevNote;
			nuCalcNote.pieSliceIdxAtDetection =    pieSliceIdxAtDetectionPrevNote;

			

		if(   scalePitchBoth  >=  0    )
		{
			nuCalcNote.scalePitch  =   scalePitchBoth;

//			long  mag   =   nuCalcNote.detectAvgHarmonicMag;    // *********   DEBUG,  see that they have reasonable values  
//			long  score =   nuCalcNote.detectScoreHarms;
		}
		else
			nuCalcNote.scalePitch  =  -1;



		Add_New_Note_to_Final_CircularQue(           nuCalcNote   );  

		Update_DrivingViews_OffMap_with_Nudge(    nuCalcNote,   isPlayingBackwards   );


		pieSliceIdxAtDetectionPrevNote++;	
		curSampleWavemanPrevNote  +=    sampsInPieSlice;      // *****   CHANGE for BACKWARDS Play ( If I implement it )  ????? ******
		beginingSampleIdxFilePrevNote   +=    sampsInPieSlice; 
	}



	if(    curSampleWavemanPrevNote  !=  currentSampleIdx   )
	{
		int  dummy =  9;   //   EVER gets HIT?   3/3/12    [   
	}
}



										////////////////////////////////////////////////


long   SPitchCalc::Get_SndSamples_Valid_Count(  double  slowSpeed   )
{	

			//   As we do our  Slow-Speeds,  a typical data load will have LESS sample because the 8bit samples in the SndSample
			//   for note analysis  are in  NormalSpeed,  and are thus fewer than the EXPANDED Count of  SlowSamples for the WAV play  3/11


		//  results for a normal MP3 file:
        //
	    //  11040 [ speed 1 ]      7360[ 1.5 ]       5520[ 2 ]       3680[ 3 ]      2760[ 4 ]      1840[ 6 ]     1380[  8 ]


	long   retSampleCount  =  -1;


	long   bytesPerSample  =  	Get_Bytes_Per_16bit_Sample();  // *****  CAREFUL,  not always  **********************


	
	long   samplesInDataFetch  =      ( m_byteCountTwoSeconds /2 )    /  bytesPerSample;   //    11,040   


	            retSampleCount  =    (long)(      (double)samplesInDataFetch   /  slowSpeed   );     //  is ACCURATE for   1.5 SlowSpeed     1/2012
	return   retSampleCount;
}



											////////////////////////////////////////


bool		SPitchCalc::Calc_logDFTs_Values(   SndSample&  spitchCalcsSample,    long  eventsOffsetInSamples,    CString&  retErrorMesg   )
{

	if(   m_useDFTrowProbeCircQue   )
		return   Calc_logDFTs_Values_CircQue(   spitchCalcsSample,   eventsOffsetInSamples,   retErrorMesg   );  
	else
		return   Calc_logDFTs_Values_Regular(   spitchCalcsSample,   eventsOffsetInSamples,   retErrorMesg   );   // *** BEST,   default    2/11
}




											////////////////////////////////////////


bool		SPitchCalc::Calc_logDFTs_Values_Regular(   SndSample&  spitchCalcsSample,    long  eventsOffsetInSamples,    CString&  retErrorMesg   )
{


	//	 eventsOffsetInSamples  -  our index INTO the sound sample ( how far into to start the read   ...because of the different PIE-SLICES in the SoundBuffer 	

	//  **** CAREFULL,  assumes that   m_logDFTtransform    is 10 pixel wide bitmap   ( install parameter or tests ???  3/11  )


	bool   disablePointerAdjustments =   false;    //    false    is  DEFAULT  (  'false'  for RELEASE  ) ******************************************





	retErrorMesg.Empty();


	if(   m_logDFTtransform ==  NULL   ||    m_harmpairsTransform ==  NULL   )
	{
		retErrorMesg =  "SPitchCalc::Calc_logDFTs_Values_Regular  FAILED:   m_logDFTtransform  or  m_harmpairsTransform is  NULL " ;
		return  false;
	}

	/***
	if(   m_kernalWidthDFT  !=  3   )
	{
		retErrorMesg =   "SPitchCalc::Calc_logDFTs_Values_Regular  FAILED, kernalWidthDFT  is not 3." ;
		return  false;
	}
	***/



	bool   couldNotLoadData =    false;

	if(    m_indexLastSndSampleBlockload  <  0    )  
	{
		couldNotLoadData =    true;
		retErrorMesg =   "SPitchCalc::Calc_logDFTs_Values_Regular  FAILED,  could NOT load  SndSampleBlock." ;
		return  false;
	}




	long    lengthSndSample =      spitchCalcsSample.Get_Length(); 


	long    countOfDataSamples  =    Get_SndSamples_Valid_Count(  m_playSpeedFlt  );   //  Been TESTING since 1/28/2012,  and seems fine
		//  Sometimes only the FIRST part of  the SndSample bytes are filled DATA, and the bytes after that are zeroes. This happens when
		//  we play at SlowSpeeds.  So 'validSampleCount'  tell us if we are only supposed to consider the FIRST 'validSampleCount'  number 
	    //  of bytes at the START of the oversized  SndSample.   3/2011


	short    dftWriteColumn =   Get_logDFTs_Write_Column();



																					//  A)     Need to set these up so that we can have  '3 pixels'  wide  DFTmap for Kernal reads																		         
	short	  dummyRet;


	for(   short  x = 0;    x <  dftWriteColumn;    x++    )				   //   9  is write column     Shift values of columns and erase the LAST column
	{
		dummyRet  =	   m_logDFTtransform->Copy_Xcolumn(   x +1,    x   ); 
	}




	m_logDFTtransform->Assign_Xcolumn(   dftWriteColumn,    0   );    //   erase the  LAST  column ( carefull if change )

	m_logDFTtransform->m_inFloatmapMode  =   true;     // so that when HarmPairsVerter reads the logDFT, it will read with a KERNAL at x=1 column





	ListIterator< DFTrowProbe >   iter(  m_rowList   );	 
	short    rowIdx =  0; 
	short    countIncompleteProbes =  0;
	short    countAdjustedProbes =  0;
	
				//           ******   NOW using a different method to deal with  RUNNING out of samples.     1/2012  ******
				//
				//  If we do not have enough VALID samples in the SndSample,   we just adjust  'pointerIntoSndSample'  BACKWARD in the SndSample
	            //  so we have enough SAMPLES to do a full PROBE with  Calc_Magnitude_of_Cell.  It's better than nothing.
				//
				//   BELOW are the failure rates with the old for loop up above ( they seem to be significant ):   Events are  { 0 - 9 }
				//  
				//  Speed 1    :    event 6 [ 2 fails ],      7 [  7 fails ],     8 [  14  fails BELOW  369 hz ],     event  9 [ 26  fails  698 hz ], 
				//
				//  Speed 1.5 :   event  4 [ 2  fails  ],    5 [  5 fails  ],    6 [ 9 fails  ],    7 [  14 fails  ],    8 [  21  fails  ],    9 [  33  fails  ],    


	
	long   maxSamplesToUse         =     lengthSndSample       -  eventsOffsetInSamples;   //  If  eventsOffsetInSamples is big, and we are near the end of SndSample, may not have enough
																										          //  samples for some probes to do their job.  ( see below )

	long   maxSamplesToUseREAL  =    countOfDataSamples   -  eventsOffsetInSamples;   //    NEW much more accurate  ,  1/2012


	char   *sndSamplesStartByte  =     spitchCalcsSample.Get_StartByte();    //  should be after Apply_Full_Filtering(), might allocate a new memorySpace  12/09 

//	char   *circQueStartByte         =    m_circularSndSample   +   m_indexLastSndSampleBlockload;




	for(     iter.First();      ! iter.Is_Done();      iter.Next()     )									
	{					

		short  magnitude;
		bool   doTheProbeAlready =  true;
		bool   readFromCircularSndsample =   false;
		long   indexToCircSndsamplesProbeStart =  -99;
		char  *pointerIntoSndSample  =   NULL;   


		DFTrowProbe&   rowProbe =     iter.Current_Item();

																									
															//   Do want to read OUT of BOUNDs on the  SMALL SndSample

      
		if(    rowProbe.m_cellLength    <=   maxSamplesToUseREAL    )   //  Is ther enough length left in SndSample, such that we do NOT
		{																							   //  have to create an  'ADJUSTED-Pointer'  to the SndSample.


		//	/*********  This might be the fastest way,  when there is room in the SMALL  SndSample  

			pointerIntoSndSample          =     sndSamplesStartByte   +  eventsOffsetInSamples;     //   ** OLD way,   1/2012

			readFromCircularSndsample =   false;   //   false:   Use the SMALL SndSample  (  m_spitchCalcsSndSample  )
	//		***/


			/***
			pointerIntoSndSample =  NULL;      //  NOT USED inside Read_SndSample_CircularQue_Value() will NOT use ( it inside of  Calc_Magnitude_of_Cell() 

			indexToCircSndsamplesProbeStart  =    m_indexLastSndSampleBlockload    +   eventsOffsetInSamples;  //  a good TEST of the algo  2/2012

			readFromCircularSndsample =   true;     //  true:    Read from the bigger  CIRCULAR-SndSample
			***/
		}
		else
		{              //  here we create an   'Backwards-ADJUSTED Pointer'   to the SndSample,  so that that we can always make a Probe-CALCULATION

			countAdjustedProbes++;    //   PROBE-Calcs are "complete"  but really  "COMPROMISED"  becase we offseted BACKWARDS in time

		//	long   spaceLeft  =     countOfDataSamples  -   rowProbe.m_cellLength    - 1;   



			if(   	   disablePointerAdjustments    )
				doTheProbeAlready =   false;      //   can DISABLE this mechanism for debugging in  Read_SndSample_CircularQue_Value()    
			else
			{
				long   indexToLastSampleInArray  =    m_indexLastSndSampleBlockload   - 1;

				if(    indexToLastSampleInArray  <  0   )
					indexToLastSampleInArray  =    m_sizeCircularSndSample  -  1;      // ***************   OK ?????  1/29/12   *******************


				long   backedUpOffset  =    indexToLastSampleInArray   -   rowProbe.m_cellLength   - 1;     //  -1 :   padding,  OK   ???


				if(     backedUpOffset  <  0   )
					indexToCircSndsamplesProbeStart  =    backedUpOffset   +   m_sizeCircularSndSample;   // ******* CAREFUL, is this really right??  1/29/12  ****************************
				else
					indexToCircSndsamplesProbeStart =     backedUpOffset;   


				ASSERT(  indexToCircSndsamplesProbeStart  >= 0   );

				readFromCircularSndsample =   true;       //  true:    Read from the bigger  CIRCULAR-SndSample
			}
		}




		if(     (long)(  rowProbe.m_midiNumber  )   <    m_bottomFreqCutoffInMidi   )      // Cut Off  FILTER:    SKIP rows that are BELOW this filtering point
			doTheProbeAlready =  false;




		if(    doTheProbeAlready   )
		{  

			ASSERT(  pointerIntoSndSample  ||  readFromCircularSndsample  );


			magnitude  =      rowProbe.Calc_Magnitude_of_Cell(   pointerIntoSndSample,   readFromCircularSndsample,   indexToCircSndsamplesProbeStart,    retErrorMesg   );   //  ********** BIG CAUNA  **********														
			if(  magnitude  >=  0  )
			{

				if(     m_logDFTtransform->Is_MidiPitch_On_Map(  rowProbe.m_midiNumber  )     )    //  ****  May not need this test ********
				{

					long  yCoord =	m_logDFTtransform->Pitch_2Ycoord(   rowProbe.m_midiNumber  ); 

					m_logDFTtransform->Write_Pixel(    dftWriteColumn,  yCoord,      magnitude,  magnitude,  magnitude   );      //    0:   x coord for the first and only column in the bitmap
				}
				else
				{	ASSERT( 0 );   }
			}
			else
			{  ASSERT( 0 );   }  //   error ?????
		}
		else
			countIncompleteProbes++;


		rowIdx++;
	}



	if(    countAdjustedProbes  > 0    )      //  Not a big deal
	{
		int  dummy =  9;
	}


	if(    countIncompleteProbes  > 0    )     //  this is a real PROBLEM !!!!    1/28/2012
	{
		int  dummy =  9;
	}

	return   true;
}


											////////////////////////////////////////


void     SPitchCalc::Initialize_All_DFTprobe_CircularQues_ArrayElements_and_Sums()
{


	// ****  CALLED BY:    SPitchCalc::Initialze_For_File_Position_Change()      SoundHelper::Pause_Play()   ....INSTALL to more FUNCTIONS  ****


	//   *** CAREFUL:    Might take a while ... there are THOUSANDS of Array ELEMENTS, though I now use memset()   9/2012
	//
	//
	//  ***     If a SUM in the CIRC-QUE get CORRUPTED all hell breaks loose (I have seen this ...Horizontal streaking on the logDFT ).  
	//            INSTALL this often to allow possible recovery.  



	if(   ! m_useDFTrowProbeCircQue   )
		return;   //  no error,  just make it easier for calling functions



	if(    m_rowList.Count()   <=  0      )
	{
		ASSERT( 0 );
		return;
	}


	ListIterator< DFTrowProbe >   iter(  m_rowList   );	 
	short    rowIdx =  0; 


	for(    iter.First();    ! iter.Is_Done();    iter.Next()    )									
	{									

		DFTrowProbe&     rowProbe   =     iter.Current_Item();

		DFTrowProbeCircQue   *rowProbeCircQue   =       dynamic_cast< DFTrowProbeCircQue* >(  &rowProbe  );  
		if(   rowProbeCircQue  ==  NULL   )
		{  
			ASSERT( 0 );
		//	retErrorMesg =   "Initialize_All_DFTprobe_CircularQues_ArrayElements_and_Sums  FAILED,  could not dcast to DFTrowProbeCircQue." ;
			return;
		}

		rowProbeCircQue->Init_CircleQue_ArrayElements_and_Sums();

		rowIdx++;
	}
}


											////////////////////////////////////////


bool		SPitchCalc::Calc_logDFTs_Values_CircQue(   SndSample&  spitchCalcsSample,    long  eventsOffset,    CString&  retErrorMesg   )
{

							//   CALLED BY:    SPitchCalc::Calc_logDFTs_Values()

	
	//	  Calculate the values for the logDFTtrForm (stored as an OffMap) with a DFTrowProbeCircQue 




	retErrorMesg.Empty();


	         //	 eventsOffset  -  our index INTO the sound sample ( how far into to start the read   ...because of the different PIE-SLICES in the SoundBuffer 	

	char  *samplesStartByte         =     spitchCalcsSample.Get_StartByte();    //  should be after Apply_Full_Filtering(), might allocate a new memorySpace  12/09 

	char  *pointerIntoSndSample  =     samplesStartByte   +   eventsOffset;   



	if(   m_logDFTtransform ==  NULL   ||    m_harmpairsTransform ==  NULL   )
	{
		retErrorMesg =  "SPitchCalc::Calc_logDFTs_Values_CircQue  FAILED:   m_logDFTtransform  or  m_harmpairsTransform is  NULL " ;
		return  false;
	}


	if(    m_hiResRatioLogDFT  <  2    )    //  Do NOT submit 1, not tested.    9/2012
	{
		retErrorMesg =  "SPitchCalc::Calc_logDFTs_Values_CircQue  FAILED:   m_hiResRatioLogDFT has a BAD value." ;
		return  false;
	}


	short         appCode   =  Get_PitchScope_App_Code_GLB();         //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope

	ASSERT(    appCode !=  3    &&      appCode !=  2    );    // This function is ONLY for Navigator and Player   9/12


	ASSERT(  m_useDFTrowProbeCircQue  );




//	double    questionablePlaySpeed =   m_playSpeedFlt;     //  This would seem to be correct,  but if FAILS bigtime in the audio.  Do I need to make corrections for this to work??   9/12

	double    questionablePlaySpeed =        1.0;                  //  *** It WAS like this when I first write this thing, months/years ago.   9/2012  [ always  1104 in this case ]
                                                                                     //   The reason it works for Speed 1 might be...   because the SndSample is NEVER altered for SPEED,
																					 //   despite the fact that  'eventsOffset'  gets SMALLER for every SlowSpeed step.  9/2012

	long   samplesToProcess  =   Calc_Samples_In_PieEvent(    questionablePlaySpeed    );   

	

	long           maxSamplesToUse  =      spitchCalcsSample.Get_Length()   -   eventsOffset;

	ASSERT(    samplesToProcess   <=    maxSamplesToUse    );


											
																																							         
	short	  dummyRet;
	long    dftWriteX  =     Get_logDFTs_Write_Column(); 

															//   Adjusting  for HI-RES,   so that LogDFT can have MULTIPLE Write-Columns     [  Calc_logDFTs_Values_CircQue() 

	for(   long   shiftIdx =0;   shiftIdx  <  m_hiResRatioLogDFT;   shiftIdx++     )
	{
			
		for(    short  x =0;    x <  dftWriteX;       x++    )	
		{
			dummyRet =	   m_logDFTtransform->Copy_Xcolumn(   x +1,    x  );      // SHIFT values of columns to LEFT,  and LATER erase the LAST column ( write column )
		}
	}




	long  clearX =   dftWriteX;


	for(   long   shiftIdx =0;   shiftIdx  <  m_hiResRatioLogDFT;   shiftIdx++     )
	{
		m_logDFTtransform->Assign_Xcolumn(   clearX,   0    );    //   ERASE the LAST columns  of DFTmap,  they are the write columns

		clearX--;
	}




	m_logDFTtransform->m_inFloatmapMode  =   true;     //  Is USED!!!   ...so that when HarmPairsVerter reads the logDFT, it will read with a KERNAL at x=1 column
																				  //
																				  //   NOTE:   Though we use  FloatmapMode  for LogDFT,  we do NOT use FloatmapMode for HarmPairs.  WHY?  9/12 
																				  //               And the LogDFT must be initialized so that  Needs_Offsetting()  returns FALSE.   9/2012



	ListIterator< DFTrowProbe >   iter(  m_rowList   );	 
	short    rowIdx =  0; 


	for(    iter.First();    ! iter.Is_Done();    iter.Next()    )									
	{									

		DFTrowProbe&     rowProbe   =     iter.Current_Item();

		DFTrowProbeCircQue   *rowProbeCircQue   =       dynamic_cast< DFTrowProbeCircQue* >(  &rowProbe  );  
		if(   rowProbeCircQue  ==  NULL   )
		{  
			retErrorMesg =   "SPitchCalc::Calc_logDFTs_Values_CircQue  FAILED,  could not dcast to DFTrowProbeCircQue." ;
			return  false;
		}



		bool   doTheProbeAlready =  true;      

		if(     (long)(  rowProbe.m_midiNumber  )   <    m_bottomFreqCutoffInMidi   )      // Cut Off  FILTER:    SKIP rows that are BELOW this filtering point
			doTheProbeAlready =  false;



		if(    doTheProbeAlready   )
		{  
			if(    ! rowProbeCircQue->Transform_Row(   spitchCalcsSample,    eventsOffset,   samplesToProcess,   retErrorMesg   )     )
			{
				ASSERT( 0 );   //  do not return,  just keep going and hope for the best for the user.  9/12
			}
		}

		rowIdx++;
	}

	return   true;
}




						////////////////////////////////////////


void  SPitchCalc::Dump_Final_CurQue()
{



	short   travIndex =   m_currentIndexFinalCircque;    //  INIT index to point at the  OLDEST  CalcedNote


	short   xWriteColumn =  0;  


	for(   short  i =0;      i <  ( m_sizeOfFinalNotesCircque );     i++    )
	{

		CalcedNote&   calcedNote  =     m_circQueFinalNotes[  travIndex  ]; 


		TRACE(   "[ %d  ]    scalePitch [  %d  ]     beginingSampleIdxFile [ %d ]    curSampleWaveman [ %d ]       pieSliceIdxAtDetection[ %d ]      \n",			
		      travIndex,   calcedNote.scalePitch,  calcedNote.beginingSampleIdxFile,   calcedNote.curSampleWaveman,    calcedNote.pieSliceIdxAtDetection   );


		xWriteColumn++;

		travIndex++;    //  Need to move   Clockwise  to go from   OLDEST notes   to   NEWEST notes

		if(     travIndex  >=   m_sizeOfFinalNotesCircque   )
			travIndex =   0;
	}
}




										////////////////////////////////////////


long   SPitchCalc::Approximate_Note_Count_In_FinalCircQue_with_BoundaryRounding(  double  playSpeed,   bool useBoundaryRounding,  long bytesPerSample   )
{



	long   blockSizeLocked  =    m_byteCountTwoSeconds / 2;       //     44160  =     BLOCKLOCKEDSIZE

	//			so 1 PieSlice =  1/10 second    or    1 PieSlice =   100 milli-seconds




	if(     ! useBoundaryRounding    )
	{
		int  dummy =   9;    // *****  Where does this still happen??    8/2012   On_Phrase_Menu_Load_Phrase()
	}





//	long	 pieSampleCount  =      Calc_Samples_In_PieEvent(  playSpeed  ); 
	long	 pieSampleCount  =      Calc_Samples_In_PieEvent(       1  );    // ******  THINK need Speed 1,  cause this is VIRTUAL ???  Read below.    8/2012  ***************



	long   notesInDataLoad  =      blockSizeLocked   /    (  pieSampleCount  *   bytesPerSample );   // need to always be 10 ( number of events[PieSlices] for a dataLoad )   3/12

	if(      notesInDataLoad  !=  10   )
	{

		int  dummy =  9;      //    Last Time Hit here since  8/12/2012 : 

		notesInDataLoad  =  10;   // ***   EXPERIMENT ( seems fine with 10 as the Modulous [ 10 Events in DataLoad ] *******   
	}





	long    earlyNotesCountApprox  =     m_sizeOfFinalNotesCircque;   

	long    earlyNotesCount            =     m_sizeOfFinalNotesCircque;  //   default, with no rounding  



	if(    useBoundaryRounding    )
	{

		earlyNotesCount =  ( ( earlyNotesCountApprox /  notesInDataLoad )  +1)    *   notesInDataLoad;    // *** BIG,  Keep to the  DataLoad BOUNDARIES
																						//  because they are the same Boundaries for the WAV-Delay Buffer.  If we do not keep on the
																						//  boundaries of the WAV-Blocks in its delay, the sound will sound disturbed as the
																					    //  WAV-delay Buffer finished its first cycle.  Tests well on all speeds.    3/27/11
	}
	else
		earlyNotesCount  =   earlyNotesCountApprox;  
	


	return  earlyNotesCount;
}




										////////////////////////////////////////


long   SPitchCalc::Approximate_Sample_Count_In_FinalCircQue_with_BoundaryRounding(   double  playSpeed,   bool useBoundaryRounding,  long bytesPerSample   )
{


		              //  This is always done in OUTPUT BYTES Format [44hz 16bit],  as are the PreRoll functions.   8/2012


	/****
	long   blockSizeLocked  =      m_byteCountTwoSeconds / 2;               //  44160;    //  BLOCKLOCKEDSIZE

	//	   1 PieSlice =  1/10 second    or    1 PieSlice =   100 milli-seconds



	long	 pieSampleCount  =    Calc_Samples_In_PieEvent(  playSpeed  );              // *****  OK ????  changed on  7/13/2012  ******************  



	long   earlyNotesCount  =  -1;
	long   earlyNotesCountApprox  =     m_sizeOfFinalNotesCircque;   //  'm_sizeOfFinalNotesCircque'   will change size according to Speed.



	long   notesInDataLoad  =      blockSizeLocked   /    (  pieSampleCount  *   bytesPerSample );   // need to always be 10 ( number of events[PieSlices] for a dataLoad )   3/12

	if(      notesInDataLoad  !=  10   )
	{
		int  dummy =  9;      //    Last Time Hit here ????   [  3/1/12   on 2x speed is 20  ]


		notesInDataLoad  =  10;   // **************   EXPERIMENT  **************************   
	}




	if(    useBoundaryRounding    )
	{
		earlyNotesCount =  ( ( earlyNotesCountApprox /  notesInDataLoad )  +1)    *   notesInDataLoad;    // *** BIG,  Keep to the  DataLoad BOUNDARIES
																						//  because they are the same Boundaries for the WAV-Delay Buffer.  If we do not keep on the
																						//  boundaries of the WAV-Blocks in its delay, the sound will sound disturbed as the
																					    //  WAV-delay Buffer finished its first cycle.  Tests well on all speeds.    3/27/11
	}
	else
		earlyNotesCount  =   earlyNotesCountApprox;  
	*****/



	ASSERT(    bytesPerSample  ==    4   );    // *****  NEW,  thin this is always in Virtual OUTPUT Samples    8/23/2012    ********************




//	long	 pieSampleCount  =      Calc_Samples_In_PieEvent(       1       );  	
	long	 pieSampleCount  =      Calc_Samples_In_PieEvent(  playSpeed  );              // *****  OK ????  changed on  7/13/2012  ******************  




	long  earlyNotesCount   =    Approximate_Note_Count_In_FinalCircQue_with_BoundaryRounding(   playSpeed,    useBoundaryRounding,   bytesPerSample   );




	long       earlySamplesCount   =     earlyNotesCount   *   pieSampleCount;
	return    earlySamplesCount;  
}






											////////////////////////////////////////
											////////////////////////////////////////


bool    SPitchCalc::Is_NoteList_TimeOrdered(    ListDoubLinkMemry<MidiNote>&   noteList   )
{

		//  This is called just before  a SORT,  to see if the list really needs to be time ordered.   Not a big deal if it fails.


		//   This COULD also alert us to    "CON-CURRENCY  Violations" ( 2 notes that overlap in time )   as well      1/12


	long   count  =     noteList.Count();  
    if(      count  <=   1L   )                
		return  true;      //   not enough to be unsorted

							

	long   prevTimeStart =  -1,  cnt = 0;
//	long    prevTimeEnd =  -1;
	long   badCount =  0;

	ListsDoubleLink< MidiNote >   *travLink =   noteList.Get_Head_Link();  



//	for(    short  i = 0;      i < (count -1 );        i++    )   *****   BAD,  this would miss the last one  2/4/12   *******   
	for(    short  i = 0;      i <    count;        i++    ) 
	{

		ASSERT(  travLink  );

		MidiNote&   sPitch =    travLink->_item;	


	//	if(    prevTimeEnd     >    sPitch.beginingSampleIdxFile   )
		if(    prevTimeStart   >    sPitch.beginingSampleIdxFile   )   // Stick with the  'start',  this makes it more consistant with Sort_NoteList_by_TimePosition()   1/12
		{

			badCount++;

	        return  false;    //  ****   PUT BACK!!!    ..as soon as we find even ONE Note that is out of order,  then we need to proceed to the bubble-sort
		}



	//	prevTimeEnd     =    sPitch.endingSampleIdxFile;
		prevTimeStart   =    sPitch.beginingSampleIdxFile;


		travLink =    travLink->_next;

		cnt++;
	}


	if(   badCount  >  0   )
		return  false;
	else
		return  true;
}



											////////////////////////////////////////


bool    SPitchCalc::Check_NoteLists_Intergity(    ListDoubLinkMemry<MidiNote>&   spitchList,    CString&  retErrorMesg   )
{


	bool   foundError =  false;
	retErrorMesg.Empty();


	long   count     =     spitchList.Count();  
    if(      count  <=   1L   )                
		return  0;      //   not enough to sort



	ListsDoubleLink< MidiNote >   *nextLink= NULL,    *prevLink= NULL,    *travLink= NULL;  


	travLink =  spitchList.Get_Head_Link();  



	for(    short  i = 0;      i < (count -1 );        i++    ) 
	{

		if(   travLink ==  NULL  )
		{
			ASSERT( 0 );
			retErrorMesg.Format(   "Check_NoteLists_Intergity failed.   travLink is  NULL  for i = %d " ,   i    );
			return  false;
		}



		nextLink  =     travLink->_next;


		if(   nextLink ==  NULL   )
		{
			ASSERT( 0 );
			retErrorMesg.Format(   "Check_NoteLists_Intergity failed.  nextLink is  NULL  for i = %d " ,   i    );
			return  false;
		}
		else
			travLink =  nextLink;
	}



	if(    travLink->_next   !=   NULL   )
	{
		ASSERT( 0 );
		retErrorMesg.Format(   "Check_NoteLists_Intergity failed.  Last link's  _next  is not NULL."   );
		return  false;
	}



			/////////////////   2nd test  ///////////////////////////////////////


			//	travLink  should be pointing to the  LAST element in the list


	for(    short  i = 0;      i < (count -1 );        i++    ) 
	{

		if(   travLink ==  NULL  )
		{
			ASSERT( 0 );
			retErrorMesg.Format(   "Check_NoteLists_Intergity failed.  2nd Pass.    travLink is  NULL  for i = %d " ,   i    );
			return  false;
		}



		prevLink  =     travLink->_prev;


		if(   prevLink ==  NULL   )
		{
			ASSERT( 0 );
			retErrorMesg.Format(   "Check_NoteLists_Intergity failed.   2nd Pass.   prevLink is  NULL  for i = %d " ,   i    );
			return  false;
		}
		else
			travLink =  prevLink;
	}



	if(         travLink  !=   spitchList._head		
		  ||   travLink->_prev   !=   NULL   )
	{
		ASSERT( 0 );
		retErrorMesg.Format(   "Check_NoteLists_Intergity failed.  2nd Pass.  travLink is NOT the Head of list "   );
		return  false;
	}


	return  true;
}




											////////////////////////////////////////


long    SPitchCalc::Sort_NoteList_by_TimePosition(    ListDoubLinkMemry<MidiNote>&  theList,     CString&  retErrorMesg   )
{


		//  returns <0  if error   [ 0 and greater is number of swaps performed  ]


	    //  **** Should  FAIL,  if the NoteSubject  has any ComponentViews in  'ListMemry<ComponentView> m_componentViews'


						   // *** SHOULD this be a GENERIC List function ( need OVERIDE-FUNCTION for  the COMPARISON-Operator  )

	long        totalSwaps = 0;
	long        count, i, j;     
	CString   retLocalErrorMesg;
 

//	bool    didHeadSwap =  false;

	retErrorMesg.Empty();




                                  
    count  =     theList.Count();  
    if(   count  <=   1L   )                
		return  0;      //   not enough to sort
	




	if(     SPitchCalc::Is_NoteList_TimeOrdered(  theList  )     )            //     a FAST test,   just in case we have no work to do on this
		return   0;									                          //     ( the  BELOW code is SLOW     ).  




	     // *****************************   OMIT when satisfied with performance  1/12  **********************

	if(    ! SPitchCalc::Check_NoteLists_Intergity(   theList,   retLocalErrorMesg   )     )
	{
		ASSERT( 0 );
		return  -1;
	}


	/****
	long  stepLimit =  3;   //  *** ADJUST ****
	long  step =   count / 20;


	if(   step  >  stepLimit  )  //  Is so FAST now that do not really even need the progress bar. Discontinue in future???  9/06
	{
		Begin_ProgressBar_Position_GLB(   "Sorting notes..."    );   
		Set_ProgressBar_Position_GLB(  5  );
	}
	****/








//	for(    i = 0;                     i<  (count -1L);    ++i   )				//   BUBBLE SORT 
	for(    i = (count  -1L);      i >= 0;                 i--    )  //  --i
    { 

	//	ListsDoubleLink< MidiNote >   *priorLink,  *linkAfterCurlink,   *curLink,    *priorPriorLink =  NULL;
		ListsDoubleLink< MidiNote >   *priorLink,   *curLink;

		priorLink =   theList._head;



//	    for(    j = (count  -1L);      j > i;          --j        )   
		for(    j = 1;                     j <=  i;         j++   )   
		{ 		

			/****  OLD
            ScalepitchSubject&   priorItem  =    theList.Get(  j -1L  );  //   const Item*   Get(  long index  )   const  ... HERE a 'POINTER to a pointer' 
            ScalepitchSubject&   curItem    =    theList.Get(  j        );       //  ... HERE, a 'POINTER to a pointer' ( must DEreference to get the pointers value )  


			if(    priorItem._startOffset   >    curItem._startOffset    )					   //  Do we need to swap POINTERs to dataObjs?
			{

				priorLink  =    theList.Find(  priorItem );    //  get pointers to  'ListLINKs'
                curLink    =    theList.Find(  curItem     );		    //   ListItem<Item>*    Find(  Item  itm   ) 
          
                theList.ReAssign_Links_Item(   *priorLink,    curItem     );		//  priorLink->item= curItem;
                theList.ReAssign_Links_Item(   *curLink,      priorItem   );	    // curLink->item    = priorItem;

				totalSwaps++;
			}

			****/

			curLink  =    priorLink->_next;    // NEW method is MUCH faster. Just swaps links   9/06

			ASSERT(  curLink !=  NULL  );


	//		linkAfterCurlink =   curLink->_next;   //  might be NULL

			

			/***
			 bool  curLinkWasTail =   false;

			 if(     curLink->_next  ==   NULL     )
				 curLinkWasTail =   true;
			***/
 
																															 //  Do we need to SWAP  neighboring DataObjs ??
		  
			if(    priorLink->_item.beginingSampleIdxFile   >   curLink->_item.beginingSampleIdxFile    )					  
			{


				/***   ///////////////////////   Make this into a NEW FUNCTION for    ListDoubLinkMemry::Swap_Neighbor_Objects_ListOrder()   ??????  

				if(    priorLink   ==    theList._head    )    //  special case for _head
				{

					theList._head =   curLink;    //  assign it as the Head

						curLink->_prev =  NULL;               //   link  A    (  because  curLink   is now the head 


					priorLink->_next =     linkAfterCurlink;   //  link  C

						if(    linkAfterCurlink  !=  NULL   )
							linkAfterCurlink->_prev =   priorLink;    


					curLink->_next  =     priorLink;           //  link  B

						priorLink->_prev =    curLink;


																			
					if(   curLinkWasTail   )
					{
						theList._tail  =    priorLink;    // Since we swapped their order, now  priorLink   becomes the new TAIL of list  
						
						ASSERT(   priorLink->_next  ==  NULL  );
					}

																			//   Reassign:   priorPriorLink,   priorLink   
					priorPriorLink =    theList._head;  

					priorLink        =   theList._head->_next;       // SAME:  priorPriorLink->_next;   ( Increment for the next Iteration  )
				}
				else
				{ 
					priorPriorLink->_next =    curLink;				 //   link  A

						curLink->_prev =    priorPriorLink;


					priorLink->_next  =     linkAfterCurlink;        //   link  C      WAS:  curLink->_next;  

						if(    linkAfterCurlink  !=  NULL   )
							linkAfterCurlink->_prev  =    priorLink;


					curLink->_next  =     priorLink;                   //   link  B

							priorLink->_prev =    curLink;



					if(    curLinkWasTail    )
					{
						theList._tail  =    priorLink;    // Since we swapped their order, now  priorLink   becomes the new TAIL of list  
						
						ASSERT(   priorLink->_next  ==  NULL  );
					}

														    							//  Reassign:   priorPriorLink,   priorLink    ( Increment for the next Iteration  )
					priorPriorLink =   curLink;     //  

			   //  'priorLink'    does not need to be reassigned, because the swap has ALREADY incremented(assigned) it 				
				}			
				///////////////////////				
				****/

				theList.Swap_Neighbor_Objects_ListOrder(   *priorLink,    *curLink  ); 


					//  NOW do NOT need to reassign:  1/12     [  OLD:   Reassign:   priorPriorLink,   priorLink    ( Increment for the next Iteration  )   ]
					//      ...because   'priorLink's  order  has changed,   and now  'priorLink'  correctly follows AFTER  'curLink'     



				//  *****************************   OMIT when satisfied with performance  9/18/06 **********************

				if(    ! SPitchCalc::Check_NoteLists_Intergity(   theList,   retLocalErrorMesg  )     )   // **** TEMP DEBUG *****
				{
					ASSERT( 0 );
					return  -3;
				}	
			

				totalSwaps++;
			}
			else
			{  //  priorPriorLink =    priorLink;       //   Reassign:   priorPriorLink,   priorLink        ( Increment for the next Iteration  )
				priorLink  =    curLink;
			}

		} 


		/***
		if(           step  >  stepLimit
			 &&   ( i  % step )   ==   0    )
		{
			double   percentDone =    (   (double)( count  -  i )  /  (double)count   )    *   100.0;
			Set_ProgressBar_Position_GLB(    (long)percentDone    );
		}
		***/
	}




	// *****************************   OMIT when satisfied with performance  9/18/06 **********************

	if(    ! SPitchCalc::Check_NoteLists_Intergity(  theList,   retLocalErrorMesg  )     )   // **** TEMP DEBUG *****
	{
		ASSERT( 0 );
		return  -43;
	}				



	/***
	if(   step  >  stepLimit  )
		End_ProgressBar_Position_GLB();
	***/

	return  totalSwaps;
}



											////////////////////////////////////////


ListDoubLinkMemry< MidiNote >*    SPitchCalc::Make_Clone_List(   long  startOffset,   long endOffset,    ListDoubLinkMemry<MidiNote>&  srcList  )
{

			//  If do NOT want to only clone a segment of the,  just set    startOffset =  endOffset =  -1			1/12


	ListDoubLinkMemry<MidiNote>*    nuList =      new    ListDoubLinkMemry< MidiNote >();
	if(  nuList ==   NULL  )
	{
		ASSERT( 0 );
		return  NULL;
	}
		
	nuList->Set_Dynamic_Flag(  true  );


	long   cnt =  0;


	ListsDoubleLink<MidiNote>                  *startLink =     srcList.Get_Head_Link();  

	SpeedIndexDoubleIterator<MidiNote>    iter(   srcList,   startLink  );  



	for(     iter.First();      !iter.Is_Done();      iter.Next()    )									
	{							

		MidiNote&  cNoteRover =     iter.Current_Item();



		bool  cloneThisObj =    true;    //  assume success


		if(   startOffset  >= 0     &&     endOffset  >= 0  )        //  did the user specify a specific segment to copy ???  
		{
			if(            cNoteRover.endingSampleIdxFile     <=   startOffset    
				    ||    cNoteRover.beginingSampleIdxFile   >=   endOffset       )   			 
				cloneThisObj =   false;
		}



		if(   cloneThisObj    )
		{

			MidiNote  *nuSPitch   =     new   MidiNote();
			if(  nuSPitch  !=  NULL  )
			{

				*nuSPitch =    cNoteRover;       //  the assignment is fine   1/12

				nuList->Add_Tail(  *nuSPitch  );  

				cnt++;
			}
		}
	}

	return  nuList;
}


											////////////////////////////////////////


bool	  SPitchCalc::Calc_StartOffset_EndOffset_for_NoteLists_Merge(   long&  retStartOffset,  long& retEndOffset,   ListDoubLinkMemry<MidiNote>&  theList,  CString&  retErrorMesg   )
{

		//   In order to AVOID PARTIALLY Detected notes, we ignore the First and Last Notes in the newly detected Notelist.    1/2012
		//
		//   So we now use the  StartOffset  and  EndOffset  of  {  2nd Note  and  Second-to-Last  Note }  to calulate this  'time BoundBox' 


	retErrorMesg.Empty();
	retStartOffset =  retEndOffset =  -1;


	long   listCount  =     theList.Count();  

	if(     listCount   <  4    )
	{
		retErrorMesg =  "SPitchCalc::Calc_StartOffset_EndOffset_for_NoteLists_Merge FAILED,  there are NOT ENOUGH notes in the list." ;
		return  false;
	}

	/****************   An earlier version, BUT not appropriate for this problem

	long  biggestEndOffset    =   -9;
	long  smallestStartOffset =    2000000000;      //   2,147,483,647   is biggest long

	ListsDoubleLink<MidiNote>     *startLink =     theList.Get_Head_Link();  

	SpeedIndexDoubleIterator<MidiNote>    iter(   theList,    startLink  );  

	for(     iter.First();      ! iter.Is_Done();      iter.Next()     )									
	{			
		MidiNote&  calcNote =     iter.Current_Item();

		if(   calcNote.beginingSampleIdxFile  <   smallestStartOffset   )
			smallestStartOffset =   calcNote.beginingSampleIdxFile;

		if(     calcNote.endingSampleIdxFile   >   biggestEndOffset   )
			biggestEndOffset =   calcNote.endingSampleIdxFile;
	}

	retStartOffset =   smallestStartOffset;		
	retEndOffset   =   biggestEndOffset;
	****/


									//   Now use the start and end of INSIDE detected notes ( ignore First and Last Note in TempList,  they might be partials )  1/12


	MidiNote&   calcNoteSecond           =     theList.Get( 1 ); 

	MidiNote&   calcNoteSecondToLast =     theList.Get(   listCount  - 2  ); 



	retStartOffset  =    calcNoteSecond.beginingSampleIdxFile;      //  uses the  START offset

	retEndOffset   =     calcNoteSecondToLast.endingSampleIdxFile;        //  uses the  END offset
 

	return  true;
}



											////////////////////////////////////////


bool 	 SPitchCalc::Are_Notes_Equal(   MidiNote&  noteA,    MidiNote&  noteB    )      
{

								//   Just checks the  Start and End sampleOffsets,  and the ScalPitch

	if(    noteA.beginingSampleIdxFile    !=    noteB.beginingSampleIdxFile   )
		return  false;

	if(    noteA.endingSampleIdxFile      !=     noteB.endingSampleIdxFile   )
		return  false;


	if(    noteA.scalePitch     !=     noteB.scalePitch   )
		return  false;

	return  true;
}


											////////////////////////////////////////


bool	 SPitchCalc::Are_Rectangles_Concurrent(  long  alphaStart,   long  alphaEnd,   long  betaStart,   long  betaEnd,   bool  useEqualSigns   )
{


	if(			    alphaStart    >     betaEnd         //  easiest, quickest way to show no intersection for MOST notes
		     ||    alphaEnd      <    betaStart    )
		return   false;



	bool    noteIsConcurrent  =    false;    //   assume fail for these PARTIAL touching tests 



	if(   useEqualSigns   )
	{

		if(			 alphaStart    >=     betaStart            //   alpha's  Start-Border  is INSIDE of  beta
			&&     alphaStart    <=     betaEnd        )
							 noteIsConcurrent  =     true;


		if(		     betaStart    >=    alphaStart               //   beta's  Start-Border  is INSIDE of  alpha
			&&     betaStart    <=    alphaEnd  )
							 noteIsConcurrent  =     true;




		if(		     alphaEnd    >=    betaStart                 //   alpha's  End-Border  is INSIDE of  beta
			&&     alphaEnd    <=    betaEnd  )
							 noteIsConcurrent  =     true;


		if(		     betaEnd    >=    alphaStart               //   beta's  END-Border  is INSIDE of  alpha
			&&     betaEnd    <=    alphaEnd  )
							 noteIsConcurrent  =     true;





		if(			 alphaStart    <=     betaStart        //   alpha   ENCLOSES   beta
			&&     alphaEnd      >=     betaEnd      )
							noteIsConcurrent  =     true;


		if(			 alphaStart    >=     betaStart       //    beta   ENCLOSES    alpha
			&&     alphaEnd      <=     betaEnd  )
								noteIsConcurrent  =     true;
	}

	else
	{
		if(			 alphaStart    >     betaStart            //   alpha's  Start-Border  is INSIDE of  beta
			&&     alphaStart    <     betaEnd        )
							 noteIsConcurrent  =     true;


		if(		     betaStart    >    alphaStart               //   beta's  Start-Border  is INSIDE of  alpha
			&&     betaStart    <    alphaEnd  )
							 noteIsConcurrent  =     true;




		if(		     alphaEnd    >    betaStart                 //   alpha's  End-Border  is INSIDE of  beta
			&&     alphaEnd    <    betaEnd  )
							 noteIsConcurrent  =     true;


		if(		     betaEnd    >    alphaStart               //   beta's  END-Border  is INSIDE of  alpha
			&&     betaEnd    <    alphaEnd  )
							 noteIsConcurrent  =     true;





		if(			 alphaStart    <     betaStart        //   alpha   ENCLOSES   beta
			&&     alphaEnd      >     betaEnd      )
							noteIsConcurrent  =     true;


		if(			 alphaStart    >     betaStart       //    beta   ENCLOSES    alpha
			&&     alphaEnd      <     betaEnd  )
								noteIsConcurrent  =     true;
	}

	return   noteIsConcurrent;
}




											////////////////////////////////////////


long	 SPitchCalc::Count_Enclosed_Notes_within_TimeSegment(   long  startOffset,   long endOffset,    ListDoubLinkMemry<MidiNote>&   masterList  )
{

				//   MidiNotes must be fully CONTAINED in the TimeZone,  or they are NOT counted.

	long   retNoteCount =   0;


	if(     masterList.Is_Empty()     )
		return  0;


	if(        startOffset   <    0     
		||    endOffset    <=   0     )
	{
		ASSERT( 0 );     // *****  BAD input parms,  want an error message ????  **********************
		return  -1;
	}


	if(    startOffset  >   endOffset   )     //  Put in right order.   Can this happen?  6/16/2012
	{
		ASSERT( 0 );    // *********   Can this happen??? *************************
		long   tp     =    startOffset;
		startOffset  =    endOffset;
		endOffset    =   tp;
	}




	long   notesProcessed =  0;
	ListsDoubleLink< MidiNote >                   *startLinkMaster  =     masterList.Get_Head_Link();  	
	SpeedIndexDoubleIterator< MidiNote >    iterDST(    masterList,    startLinkMaster   );  
	bool   keepGoing =  true;


	iterDST.First(); 

	do
	{			
		MidiNote&  midiNote =     iterDST.Current_Item();


		iterDST.Next();       //   Call  Next()  early,   so we still have a  "VALID pointer"  after the deletion to continue the iteration with.   1/12



		if(          midiNote.beginingSampleIdxFile   >=    startOffset            //  Must be completely enclosed in the TimeSegment
			  &&   midiNote.endingSampleIdxFile     <=    endOffset     )
		{
			retNoteCount++;
		}	


	
		if(    midiNote.beginingSampleIdxFile  >  endOffset   )     //  IF the list is time-ordered,  then these notes are BEYOND the END of target zone { endOffset } 
			keepGoing  =   false;  


		if(    iterDST.Is_Done()    ) 
			keepGoing  =   false;  


		notesProcessed++;

	}  while(   keepGoing   );



	return  retNoteCount;
}



											////////////////////////////////////////


long	 SPitchCalc::Count_Concurrent_Notes_within_TimeSegment(   long  startOffset,   long endOffset,    ListDoubLinkMemry<MidiNote>&   masterList  )
{

			//   MidiNotes do NOT have to be fully CONTAINED in the TimeZone,  they only need to PARTIALLY INTERSECT  that  TimeZone.    8/12


	long   retNoteCount =   0;


	if(     masterList.Is_Empty()     )
		return  0;


	if(        startOffset   <    0     
		||    endOffset    <=   0     )
	{
		ASSERT( 0 );     // *****  BAD input parms,  want an error message ????  **********************
		return  -1;
	}


	if(    startOffset  >   endOffset   )     //  Put in right order.   Can this happen?  6/16/2012
	{
		ASSERT( 0 );    // *********   Can this happen??? *************************
		long   tp     =    startOffset;
		startOffset  =    endOffset;
		endOffset    =   tp;
	}




	long   notesProcessed =  0;
	ListsDoubleLink< MidiNote >                   *startLinkMaster  =     masterList.Get_Head_Link();  	
	SpeedIndexDoubleIterator< MidiNote >    iterDST(    masterList,    startLinkMaster   );  
	bool   keepGoing =  true;


	iterDST.First(); 

	do
	{			
		MidiNote&  midiNote =     iterDST.Current_Item();


		iterDST.Next();       //   Call  Next()  early,   so we still have a  "VALID pointer"  after the deletion to continue the iteration with.   1/12



		if(        (   midiNote.endingSampleIdxFile    >=  startOffset      &&     midiNote.endingSampleIdxFile   <=  endOffset      )         //  front end of timeZone
			||    (   midiNote.beginingSampleIdxFile  >=  startOffset      &&     midiNote.beginingSampleIdxFile   <=  endOffset    )    )   //  rear end of timeZone
		{																															           //  This will also count notes that are TOTALLY ENCLOSED
			retNoteCount++;
		}	


	
		if(    midiNote.beginingSampleIdxFile  >  endOffset   )     //  IF the list is time-ordered,  then these notes are BEYOND the END of target zone { endOffset } 
			keepGoing  =   false;  


		if(    iterDST.Is_Done()    ) 
			keepGoing  =   false;  


		notesProcessed++;

	}  while(   keepGoing   );



	return  retNoteCount;
}



											////////////////////////////////////////


bool	 SPitchCalc::Delete_Notes_in_TimeSpan(   long  startOffset,   long endOffset,     short&  retDeleteCount,     ListDoubLinkMemry<MidiNote>&   masterList, 
																							 ListDoubLinkMemry<MidiNote>&  tempList, 	   CString&  retErrorMesg   )
{

		//  CALLED BY:    NoteGenerator::Merge_TempList_to_Master_Notelist()



	bool    doDebugComparison =  false;     //  false     Default   false   ( set false for release  )


	retErrorMesg.Empty();
	retDeleteCount =  0;


	if(     masterList.Is_Empty()     )
		return  true;


	long   notesProcessed =  0;
	ListsDoubleLink< MidiNote >                   *startLinkMaster  =     masterList.Get_Head_Link();  	
	SpeedIndexDoubleIterator< MidiNote >    iterDST(    masterList,    startLinkMaster   );  

	bool   keepGoing =  true;



	iterDST.First(); 

	do
	{			
		MidiNote&  midiNote =     iterDST.Current_Item();

		long   holdNoteStart =   midiNote.beginingSampleIdxFile;   //   After the delete,  midiNote.beginingSampleIdxFile will have an INVALID value 



		iterDST.Next();       //   Call  Next()  early,   so we still have a  "VALID pointer"  after the deletion to continue the iteration with.   1/12



//		bool  useEqualSigns =   true;   //   OLD:  ( Even if  a NOTE'S  BOUNDARY touches another note, it is a concurrency violation   1/2012  )

		bool  useEqualSigns =   false;   // *** Above is a good idea,  BUT  PitchScope2007 files allow neighboring notes to SHARE a time BOUNDARY.    9/2012  ***




		bool   areConcurrent   =    SPitchCalc::Are_Rectangles_Concurrent(    midiNote.beginingSampleIdxFile,   midiNote.endingSampleIdxFile,    
																										                      startOffset,   endOffset,   useEqualSigns  );
		if(      areConcurrent   )
		{

			bool    removeSuccess =  true;  
			long    retNotesIndexInList =  -2;  
			MidiNote  *tempNote =  NULL;




			if(    doDebugComparison    )       // **************   TEMP DEBUG,  switch at top  *******************
			{

				bool  useEqualSignsDebug =   false;    // ****  HOW should this be set ????



				tempNote =	  SPitchCalc::Find_A_Concurrent_Note(   tempList, 	midiNote.beginingSampleIdxFile,    midiNote.endingSampleIdxFile,    
																													  retNotesIndexInList,   useEqualSignsDebug,     retErrorMesg   );
				if(   tempNote  !=  NULL  )
				{
					if(     SPitchCalc::Are_Notes_Equal(  midiNote,  *tempNote  )      )   //  compairs  START and END  Time-Offsets,   and the SCALEPITCH
					{

						TRACE(  "Found      EQUAL         note  [ %d ]:     DeleteOld[  %d,   %d  ] %d     \n",   retNotesIndexInList,
							midiNote.beginingSampleIdxFile,             midiNote.endingSampleIdxFile,            midiNote.scalePitch   );
					}
					else
					{  TRACE(  "Found  CONCURRENT  note  [ %d ]:     DeleteOld[  %d,   %d  ] %d         New[  %d,  %d  ] %d   \n",   retNotesIndexInList,
										midiNote.beginingSampleIdxFile,             midiNote.endingSampleIdxFile,            midiNote.scalePitch,
										tempNote->beginingSampleIdxFile,          tempNote->endingSampleIdxFile,   tempNote->scalePitch     );
					}
				}
				else
					AfxMessageBox(  retErrorMesg  );
			}  

																										


			masterList.Remove_Item(   midiNote   );


			if(   ! removeSuccess   )        //  **** How could I test that   Remove_Item()  failed ???    1/12
				return  false;
			else
				retDeleteCount++;
		}	


	
		if(    holdNoteStart  >  endOffset   )    //  IF the list is time-ordered,  then these notes are BEYOND the END of target zone { endOffset } 
			keepGoing  =   false;  


		if(    iterDST.Is_Done()    ) 
			keepGoing  =   false;  


		notesProcessed++;

	}  while(   keepGoing   );


	return  true;
}



											////////////////////////////////////////


bool	 SPitchCalc::Find_And_Kill_MidiNote(   MidiNote  *note,    CString&  retErrorMesg   )
{


	if(    m_calcedNoteListMasterApp  ==  NULL   )
	{
		retErrorMesg =   "Find_And_Kill_MidiNote  FAILED,  m_calcedNoteListMasterApp is NULL." ;
		return  false;
	}

	if(   note  ==  NULL   )
	{
		retErrorMesg =   "Find_And_Kill_MidiNote  FAILED,  note is NULL." ;
		return  false;
	}
	
	if(   m_calcedNoteListMasterApp->Count()  <=  0   )
	{
		retErrorMesg =   "Find_And_Kill_MidiNote  FAILED,  there were no notes in the Source Notelist." ;
		return  false;
	}



	Erase_the_UndoNotes_Data();   //  Dont really need to do this,  it will be ASSIGNED in the Loop below.   2/12



	ListsDoubleLink< MidiNote >                   *startLinkMaster  =     m_calcedNoteListMasterApp->Get_Head_Link();  	
	SpeedIndexDoubleIterator< MidiNote >    iter(   *m_calcedNoteListMasterApp,    startLinkMaster   );  
	bool  keepGoing      =    true;
	bool  foundTheNote  =  false;



	iter.First(); 

	do
	{	MidiNote&   cNoteRover =     iter.Current_Item();

		iter.Next();     //  must call this BEFORE deletion,  otherwise the Deletion of Current would loose the  ->_next   value
				

		if(    &cNoteRover  ==   note    )     
		{

			m_deletedListNote  =    *note;    //  1st SAVE  the Notes values in case of   Edit-UNDO


			m_calcedNoteListMasterApp->Remove_Item(  *note   );

			keepGoing      =   false;   // since we have done the job,  we can go now
			foundTheNote  =  true;
		}	


		if(    iter.Is_Done()    ) 
			keepGoing  =   false;  

	}  while(   keepGoing   );



	if(   ! foundTheNote    )
	{
		retErrorMesg =   "Find_And_Kill_MidiNote  FAILED,  it could NOT FIND the note in the notelist." ;
		return  false;
	}

	return  true;
}


											////////////////////////////////////////


bool    SPitchCalc::Delete_Note(   MidiNote  *note,    CString&  retErrorMesg   )
{


	if(    m_calcedNoteListMasterApp  ==  NULL   )
	{
		retErrorMesg =   "Delete_Note  FAILED,  m_calcedNoteListMasterApp is NULL." ;
		return  false;
	}

	if(   note  ==  NULL   )
	{
		retErrorMesg =   "Delete_Note  FAILED,  note is NULL." ;
		return  false;
	}
	
	if(   m_calcedNoteListMasterApp->Count()  <=  0   )
	{
		retErrorMesg =   "Delete_Note  FAILED,  there were no notes in the Source Notelist." ;
		return  false;
	}


	OffMap  *drivingOffMap;

	if(    m_doVerticalDrivingview   )
		drivingOffMap =    m_drivingOffMapVert; 
	else
		drivingOffMap =    m_drivingOffMapHorz; 



	if(     ! Find_And_Kill_MidiNote(   note,   retErrorMesg   )     )
		return  false;




	if(    ! Check_NoteLists_Intergity(    *m_calcedNoteListMasterApp,   retErrorMesg  )    )   // ********  TEMP,  DEBUG  2/12   *************************
		AfxMessageBox(  retErrorMesg  );


	if(    ! Is_NoteList_TimeOrdered(   *m_calcedNoteListMasterApp  )    )              // ********  TEMP,  DEBUG  2/12   *************************
		AfxMessageBox(  retErrorMesg  );



	/***
	long	swapCount =	  Sort_NoteList_by_TimePosition(    *m_calcedNoteListMasterApp,    retErrorMesg  );   // **** NOT NEEDED for Delete ??? 2/12 ****************

	if(    ! retErrorMesg.IsEmpty()   )
		AfxMessageBox(  retErrorMesg  );
	***/



//	bool  isPlayingBackward  =   false;                              // ********* Was a BUG:   User might be playing in reverse   6/2012  ***********
	bool  isPlayingBackward  =    Is_Playing_Backwards(); 




	if(    !  Cleanup_FinalCircQue_from_NoteList(   isPlayingBackward,   *drivingOffMap,   m_doVerticalDrivingview,   retErrorMesg  )     )
		return  false;    
// **************************   NEW,  to fix bug of PHANTOM note after delete  **********************************






	if(    ! ReDraw_DrivingViews_OffMap_from_NoteList(    isPlayingBackward,   *drivingOffMap,   m_doVerticalDrivingview,   retErrorMesg  )    )
		return  false;


	return  true;
}




											////////////////////////////////////////


void	  SPitchCalc::Erase_the_UndoNotes_Data()
{

		   //  Needs to be called whenever the user  LOADS or CREATES a NOTE-LIST    2/12

			//     BUT do NOT put it inside of of  Release_Selection


	m_deletedListNote.beginingSampleIdxFile =    -9;
	m_deletedListNote.endingSampleIdxFile    =   -9;             //  will hold Note's time END boundaries when doing a FILE save to NoteList


//	m_deletedListNote.curSampleWaveman =  -9;


	m_deletedListNote.scalePitch =  -5;


	m_deletedListNote.octaveIndex =  -9;		

	m_deletedListNote.detectScoreHarms  =  -11;

	m_deletedListNote.detectAvgHarmonicMag =  -11;


//	m_deletedListNote.detectScoreOctave  =  -11;
	for(    short oct =0;    oct <  kCountOfNavOctaveCandidates;    oct++   )
			m_deletedListNote.detectScoreOctaveCandids[  oct  ]  =   -11;





//	m_deletedListNote.primaryQuesSampleIdx =   -9;
//	m_deletedListNote.synthCode =   -9;
//	m_deletedListNote.pieSliceIdxAtDetection =  -9;
}



											////////////////////////////////////////


bool   SPitchCalc::Replace_Undo_Note_to_NoteList(   CString&  retErrorMesg   )
{


	if(    m_calcedNoteListMasterApp  ==  NULL   )
	{
		retErrorMesg =   "Replace_Undo_Note_to_NoteList  FAILED,  m_calcedNoteListMasterApp is NULL." ;
		return  false;
	}
	

	if(   !  m_deletedListNote.Has_Valid_Data()     )
	{
		retErrorMesg =   "Replace_Undo_Note_to_NoteList  FAILED,  m_deletedListNote does NOT have VALID data." ;
		return  false;
	}
		


	OffMap  *drivingOffMap;

	if(    m_doVerticalDrivingview   )
		drivingOffMap =    m_drivingOffMapVert; 
	else
		drivingOffMap =    m_drivingOffMapHorz; 




	MidiNote  *nuCalcNote       =        new   MidiNote();
	if(                 nuCalcNote  ==  NULL  )
	{
		retErrorMesg =   "Replace_Undo_Note_to_NoteList  FAILED,  could NOT allocate a CalcedNote." ;
		return  false;
	}


	bool  newNoteIsLaterThanTail =   false;
	long  noteStartLastNote          =    m_calcedNoteListMasterApp->Get_Last_Link()->_item.beginingSampleIdxFile;

	if(    nuCalcNote->beginingSampleIdxFile  >  noteStartLastNote   )
		newNoteIsLaterThanTail =   true;



	*nuCalcNote =    m_deletedListNote;        //  the  'ASSIGNMENT op' is fine    1/12


	m_calcedNoteListMasterApp->Add_Tail(  *nuCalcNote   );



	if(    ! Check_NoteLists_Intergity(    *m_calcedNoteListMasterApp,   retErrorMesg  )    )   // ********  TEMP,  DEBUG  2/12   *************************
		AfxMessageBox(  retErrorMesg  );




	long  swapCount     =       SPitchCalc::Sort_NoteList_by_TimePosition(   *m_calcedNoteListMasterApp,   retErrorMesg  );  // since we blindly added the notes to the end of list  
	if(     swapCount  ==  0 
		&&   ! newNoteIsLaterThanTail   )   //  if  the user deleted the LAST note in the list, then this would NOT be an error.  2/12
	{
		ASSERT( 0 );     //  This already caught a bad BUG in  Is_NoteList_TimeOrdered()    2/4/2012						
	}




	if(    ! Check_NoteLists_Intergity(    *m_calcedNoteListMasterApp,   retErrorMesg  )    )   // ********  TEMP,  DEBUG  2/12   *************************
		AfxMessageBox(  retErrorMesg  );



	if(    ! Is_NoteList_TimeOrdered(   *m_calcedNoteListMasterApp  )    )              // ********  TEMP,  DEBUG  2/12   *************************
		AfxMessageBox(  retErrorMesg  );




	long    retConcurentCount = -1; 

	if(    ! SPitchCalc::Count_Concurrent_Notes_in_List(   *m_calcedNoteListMasterApp,    retConcurentCount,    retErrorMesg   )    )   // check for error in my Algo.   1/21/2012
		AfxMessageBox(  retErrorMesg  );	



	if(   retConcurentCount  >  0  )
	{	 

		ASSERT( 0 );     //  ****   If  ( useEqualSigns =  TRUE )  in Count_Concurrent_Notes_in_List,   PitchScope2007 FILES will throw this assertion   9/7/12 ****
	}





//	bool  isPlayingBackward  =    false;                              // ********* Was a BUG:   User might be playing in reverse   6/2012  ***********
	bool  isPlayingBackward  =    Is_Playing_Backwards(); 






	if(     ! Cleanup_FinalCircQue_from_NoteList(   isPlayingBackward,   *drivingOffMap,   m_doVerticalDrivingview,   retErrorMesg    )    )
		return  false;
				
	//  ********  NEW   6/2012,     to CLEANUP  the data in the CalcNotes,  now that the note has been replaced 





	if(    ! ReDraw_DrivingViews_OffMap_from_NoteList(    isPlayingBackward,   *drivingOffMap,   m_doVerticalDrivingview,   retErrorMesg  )    )
		return  false;


	return  true;
}



											////////////////////////////////////////


bool	  SPitchCalc::Add_TempLists_Notes_to_Master_Notelist(   ListDoubLinkMemry<MidiNote>&  srcList,    ListDoubLinkMemry<MidiNote>&  masterList,   
																								  long  startRecordSampleIdx,    long  endRecordSampleIdx,     
																																long&  retNoteCountAdded,    CString&  retErrorMesg   )
{
	retErrorMesg.Empty();
	retNoteCountAdded =  0;


	if(   srcList.Count()  <=  0   )
	{
		retErrorMesg =   "Add_TempLists_Notes_to_Master_Notelist  FAILED,  there were no notes in the Source Notelist." ;
		return  false;
	}

	ASSERT(   startRecordSampleIdx  >=  0     &&    endRecordSampleIdx > 0    );



	long   noteCount =  0;

	ListsDoubleLink<MidiNote>                  *startLink =   srcList.Get_Head_Link();  
	SpeedIndexDoubleIterator<MidiNote>    iter(  srcList,   startLink  );  



	for(     iter.First();      ! iter.Is_Done();      iter.Next()     )									
	{			

		bool   isLikelyAPartialNote =  true;    //  A  'Partial Note'   straddles the  Time Recording Boundaries, and is NOT complete. So we do NOT add it.
														      //   However,   Delete_Notes_in_TimeSpan() will NOT delete the same note, becase it sees that the note
															 //   "straddles Time Recording Boundaries".    1/20/2012

		MidiNote&   srcCalcNote =     iter.Current_Item();



		if(           srcCalcNote.beginingSampleIdxFile   >=    startRecordSampleIdx
			  &&    srcCalcNote.endingSampleIdxFile      <=    endRecordSampleIdx     ) 
		{
			isLikelyAPartialNote =  false;
		}
		else
		{  int   dummy =  9;   }



		if(   ! isLikelyAPartialNote    )
		{

			MidiNote  *nuCalcNote       =        new   MidiNote();
			if(                 nuCalcNote !=  NULL  )
			{
				*nuCalcNote =    srcCalcNote;       //  the  'ASSIGNMENT op' is fine    1/12

				masterList.Add_Tail(  *nuCalcNote   );

				noteCount++;
				retNoteCountAdded++;
			}
		}
	}

	return  true;
}



											////////////////////////////////////////


bool	 SPitchCalc::Count_Concurrent_Notes_in_List(   ListDoubLinkMemry<MidiNote>&  noteList, 	long&  retConcurentCount,    CString&  retErrorMesg   )
{

												//    This runs 2 Iterators on the SAME List ( is that OK ??  )  
	retErrorMesg.Empty();
	retConcurentCount =  0;


	if(   noteList.Count()  <=  0   )
	{
		retErrorMesg =   "SPitchCalc::Count_Concurrent_Notes_in_List  FAILED,  there were no notes in the Source Notelist." ;
		return  false;
	}


	long   notesIndex =  0;

	ListsDoubleLink<MidiNote>                  *startLink =   noteList.Get_Head_Link();  
	SpeedIndexDoubleIterator<MidiNote>    iterOut(  noteList,   startLink  );  



	for(     iterOut.First();      ! iterOut.Is_Done();      iterOut.Next()     )	
	{

		MidiNote&  midiNote  =     iterOut.Current_Item();


		SpeedIndexDoubleIterator<MidiNote>    iterInside(   noteList,   noteList.Get_Head_Link()   );  


		for(     iterInside.First();      ! iterInside.Is_Done();      iterInside.Next()     )	
		{

			bool   noteIsConcurrent =   false;


			MidiNote&  calcNoteInside  =     iterInside.Current_Item();





//			bool  useEqualSigns =   true;   //   OLD:  ( Even if  a NOTE'S  BOUNDARY touches another note, it is a concurrency violation   1/2012  )

			bool  useEqualSigns =   false;   // *** Above is a good idea,  BUT  PitchScope2007 files allow neighboring notes to SHARE a time BOUNDARY.    9/2012  ***




			if(    &calcNoteInside  !=    &midiNote    )   //  Do NOT try to compair the SAME Note
			{
				noteIsConcurrent =   SPitchCalc::Are_Rectangles_Concurrent(    midiNote.beginingSampleIdxFile,            midiNote.endingSampleIdxFile,    
				                                                                                                calcNoteInside.beginingSampleIdxFile,   calcNoteInside.endingSampleIdxFile,   useEqualSigns  );
			}


			if(    noteIsConcurrent    )
			{
				retConcurentCount++;
		
				TRACE(  "Found  2 CONCURRENT  notes:     Out[  %d,   %d  ] %d         In[  %d,  %d  ] %d   \n",   
								midiNote.beginingSampleIdxFile,             midiNote.endingSampleIdxFile,            midiNote.scalePitch,
								calcNoteInside.beginingSampleIdxFile,    calcNoteInside.endingSampleIdxFile,   calcNoteInside.scalePitch     );
			}

		}  //   for(     iterInside.First()


		notesIndex++;

	}    //    for(     iterOut.First()
	


	TRACE(  "\n\n A total of  %d  notes were found to be CONCURRENT.   \n" ,   retConcurentCount   );

	return  true;
}




											////////////////////////////////////////


MidiNote* 	 SPitchCalc::Find_A_Concurrent_Note(    ListDoubLinkMemry<MidiNote>&  noteList, 	  long  startOffset,     long  endOffset,  
																							 long&  retNotesIndexInList,   bool  useEqualSigns,     CString&  retErrorMesg   )
{

			//   CALLED BY:    PsNavigatorDlg::On_Insert_Note_EditMenu()      SPitchCalc::Delete_Notes_in_TimeSpan()


	MidiNote  *retConcurentNote =  NULL; 
	retNotesIndexInList =  -1;

	retErrorMesg.Empty();



	if(   noteList.Count()  <=  0   )
	{
//		retErrorMesg =   "SPitchCalc::Find_A_Concurrent_Note  FAILED,  there were no notes in the Source Notelist." ;
		return  NULL;                            // ***  Do NOT want to seethis message  ****
	}



	long   noteIndex =  0;

//	ListsDoubleLink<MidiNote>                  *startLink =   noteList.Get_Head_Link();  
	SpeedIndexDoubleIterator<MidiNote>    iter(  noteList,   noteList.Get_Head_Link()  );  



	for(     iter.First();      ! iter.Is_Done();      iter.Next()     )	
	{

		MidiNote&  midiNote  =     iter.Current_Item();



		bool   noteIsConcurrent =   SPitchCalc::Are_Rectangles_Concurrent(    midiNote.beginingSampleIdxFile,    midiNote.endingSampleIdxFile,    
				                                                                                  startOffset,    endOffset,       useEqualSigns  );
		if(    noteIsConcurrent    )
		{
			retConcurentNote    =   &midiNote;
			retNotesIndexInList =     noteIndex;

			break;
		}

		noteIndex++;
	}

	return    retConcurentNote;
}




											////////////////////////////////////////


MidiNote* 	 SPitchCalc::Find_Neighbor_MidiNote(   MidiNote&  selectedMidiNote,    ListDoubLinkMemry<MidiNote>&  noteList,   bool  backSearchFlag,   CString&  retErrorMesg  )
{


	MidiNote  *retFoundNote =  NULL; 	
	bool      justFoundSelectNote  =  false;

	SpeedIndexDoubleIterator<MidiNote>    iter(   noteList,    noteList.Get_Head_Link()   );  



	if(   backSearchFlag    )						   //   looking for the PREVIOUS Midi note
	{

		for(     iter.First();      ! iter.Is_Done();      iter.Next()    )									
		{				

			MidiNote&  noteRover =     iter.Current_Item();

			if(    &selectedMidiNote  ==   &noteRover   )
			{

				justFoundSelectNote =   true;


				if(    iter.m_currentLink->_prev  !=   NULL   )
				{
					retFoundNote  =     &(    iter.m_currentLink->_prev->_item   );				
				}
				else
					retFoundNote  =   NULL;   

				break;
			}
		}
	}
	else                     //       looking for the NEXT Midi note
	{	 

		for(     iter.First();      ! iter.Is_Done();      iter.Next()    )									
		{							
			MidiNote&  noteRover =     iter.Current_Item();

			if(    justFoundSelectNote    )
			{
				retFoundNote =   &noteRover;
				break;
			}

			if(    &selectedMidiNote  ==   &noteRover   )
				 justFoundSelectNote =   true; 
		}
	}



	if(        justFoundSelectNote  ==  true   
		&&   retFoundNote  ==   NULL   )
	{

		if(   backSearchFlag   )
			retErrorMesg =   "You have reached the BEGINNING of the NoteList." ;
		else
			retErrorMesg =   "You have reached the END of the NoteList." ;

		return  NULL;
	}

	return   retFoundNote;
}



											////////////////////////////////////////


MidiNote* 	 SPitchCalc::Find_Next_MidiNote(   long  sampleIndex,    ListDoubLinkMemry<MidiNote>&  noteList,  CString&  retErrorMesg  )
{


	MidiNote  *retFoundNote =  NULL; 	
	bool          justFoundSampleIdx  =  false;


	if(   sampleIndex  <  0   )
		return  NULL;



	SpeedIndexDoubleIterator<MidiNote>    iter(   noteList,    noteList.Get_Head_Link()   );  



	for(     iter.First();      ! iter.Is_Done();      iter.Next()    )									
	{							

		MidiNote&  noteRover =     iter.Current_Item();

		/***
		if(    justFoundSelectNote    )
		{
			retFoundNote =   &noteRover;
			break;
		}
		****/


//		if(    &selectedMidiNote  ==   &noteRover   )
		if(       noteRover.beginingSampleIdxFile  >=   sampleIndex  
			||    noteRover.endingSampleIdxFile    >     sampleIndex    )     //   This will include notes that straddle the Feed Line
		{
			justFoundSampleIdx =   true; 

			retFoundNote =   &noteRover;
			break;
		}
	}
	


	if(      justFoundSampleIdx  ==  true   
		&&   retFoundNote  ==   NULL   )
	{

		retErrorMesg =   "You have reached the END of the NoteList." ;
		return  NULL;
	}

	return   retFoundNote;
}



										/////////////////////////////////////////////////


long     SPitchCalc::Calc_DrivingViews_First_SampleIdx(   bool   isPlayingBackward  )
{
																			//  GOOD  function,  use it more  4/2012



//	long    firstSampleIdxInWindow    =     Calc_Virtual_SampleIdx_for_FilePosition_StaticText();     //  ****  Never that accurate, BUT...



												      //  ...THIS is a BETTER  way to calulate  the current   "TIME  Sample-Offset"  of  SPitchCalc's  OFFMAP   ****
	
	CalcedNote   retCalcNote;    
					 //  For Horizontal play, fetch the  CalcedNote at the  FAR LEFT  of DrivingView ( regardless if forward or backwards ),    [ is same for Vertial, too ]     2/12


	if(    isPlayingBackward    )   
		Get_Newest_Note_from_Final_CircularQue(   retCalcNote  );
	else
	    Get_Oldest_Note_from_Final_CircularQue(   retCalcNote   );   



	long      firstSampleIdxInWindow  =    retCalcNote.beginingSampleIdxFile; 
	return    firstSampleIdxInWindow; 
}



										/////////////////////////////////////////////////


long    SPitchCalc::Calc_DrivingViews_Last_SampleIdx(   bool  isPlayingBackward   )
{
																			//  GOOD  function,  use it mopre  4/12


			 //  For Horizontal play, fetch the  CalcedNote at the  FAR LEFT  of DrivingView ( regardless if forward or backwards ),    [ is same for Vertial, too ]     2/12
	
	CalcedNote   retCalcNote;    

	if(    isPlayingBackward    )   
		Get_Oldest_Note_from_Final_CircularQue(   retCalcNote   );   
	else
		Get_Newest_Note_from_Final_CircularQue(   retCalcNote  );



	long      lastSampleIdxInWindow  =    retCalcNote.beginingSampleIdxFile; 
	return    lastSampleIdxInWindow; 
}




						////////////////////////////////////////


bool    SPitchCalc::Is_DrivingView_at_First_TimeFrame(   bool   isPlayingBackward   )
{

 
	long   pieSliceTolerance =  1;   // ***************   ADJUST  *****************************


	bool   isAtFirstTimeFrame =  false;


	if(   isPlayingBackward   )   // **************  Is this an ISSUE ???  ***************************
	{
	//	AfxMessageBox(    "You must be playing music FORWARD for this function to work."   );
		return  false;
	}


	double   playSpeedFlt     =     m_playSpeedFlt;
	long     sampsInPieSlice =     Calc_Samples_In_PieEvent(   playSpeedFlt   ); 


//	long	    startSampleIdxWind =    Calc_Windows_First_SampleIdx();   
	long	    startSampleIdxWind =    Calc_DrivingViews_First_SampleIdx(  isPlayingBackward  );


	long       startPieSliceWindow  =     startSampleIdxWind  /  sampsInPieSlice;



	if(    startPieSliceWindow   <=   pieSliceTolerance    )  
		isAtFirstTimeFrame =   true;


	return  isAtFirstTimeFrame; 
}



						////////////////////////////////////////


bool    SPitchCalc::Is_DrivingView_at_Last_TimeFrame(   bool   isPlayingBackward,   long  filesLastSample   )
{

 
	long   pieSliceTolerance =  50;     //  ***Very TRICKY***    50[ set on 5/4/2012 ]     ***************   ADJUST  *****************************

		//   Remember,  a Windows WIDTH is about  140 PieSlices wide  ( kMAXsIZEofFINALnoteCircQUErealistic  = 140  )


	bool   isAtLastTimeFrame =  false;

 


	if(   isPlayingBackward   )      // *******  Is this an ISSUE ???  ********
	{
	//	AfxMessageBox(    "You must be playing music FORWARD for this function to work."   );
		return  false;
	}



	double    playSpeedFlt         =     m_playSpeedFlt;
	long      sampsInPieSlice      =     Calc_Samples_In_PieEvent(   playSpeedFlt   ); 

	long	  endSampleIdxWind     =     Calc_DrivingViews_Last_SampleIdx(   isPlayingBackward   );


	long      endPieSliceOfWindow   =    endSampleIdxWind  /  sampsInPieSlice;

	long      filesLastPieSlice     =    filesLastSample  /   sampsInPieSlice; 


	long      differenceInPieSlices  =   filesLastPieSlice -   endPieSliceOfWindow; 




		TRACE(  "\nPie Slice DIFFERENCE is  %d      [ Tolerance is %d  ] \n"  ,    differenceInPieSlices,    pieSliceTolerance     );


	if(    differenceInPieSlices   <   pieSliceTolerance   )
	{
		isAtLastTimeFrame =   true;
	}


	return  isAtLastTimeFrame; 
}






////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


DFTrowProbe::DFTrowProbe(   SPitchCalc&  sPitchCalc,    long  cellLength   )   
								:  m_sPitchCalc( sPitchCalc ),   m_cellLength( cellLength )
{

			//	  Calculates the values for the logDFTtrForm (stored as an OffMap) 


	m_yVirtual  =   -1;

	m_samplesInPeriod =  -1;


	m_trigTabl.numEntries =    0;         // init a RESIDENT trig-table
    m_trigTabl.curAngFreq =  0.0;

	m_trigTabl.Cos =   NULL;
	m_trigTabl.Sin  =   NULL;

}


											////////////////////////////////////////

DFTrowProbe::~DFTrowProbe()
{

	if(   m_trigTabl.Cos  !=  NULL  )		
		free(  m_trigTabl.Cos );

	if(   m_trigTabl.Sin  !=  NULL  )
		free(  m_trigTabl.Sin );
}


											////////////////////////////////////////


bool    DFTrowProbe::Create_TrigTable(   double  angFreq,    long  numSamps,    bool  doWindow    )
{

		//  Do NOT use  'Windowing'  for  CircleQue  and  logDFTinchVerter ( 'inch' )   [ when was this written ???  ] 



		//   PROBLEM:  This is similar to a Gabor wave,  but NO Gausian 'Windowing-function'  has been applied. But the book
		//			   says that if the number of samples in Kernal is a multiple of Period, no window is necessary




	if(     (  m_trigTabl.Cos =   ( long* )malloc(    (numSamps * 4L)   )   )    == NULL    )      //   4 bytes in a long
	{
		ASSERT( 0 );
	//	retErrorMesg =   "DFTrowProbe::Create_TrigTable  failed,  could not allocate  TrigTable  COS."  ;
		return  false;
	}


	if(     (  m_trigTabl.Sin =   ( long* )malloc(    (numSamps * 4L)   )   )     == NULL    )      //   4 bytes in a long
	{
		ASSERT( 0 );
	//	retErrorMesg =   "DFTrowProbe::Create_TrigTable  failed,  could not allocate  TrigTable  SIN."  ;
		return  false;
	}


	m_samplesInPeriod =      (long)(    1.0  / angFreq    );



    long       i;                  // double  tp,  ang;    
    double   quickFact;
    
     
    
    if(   m_trigTabl.curAngFreq   !=   angFreq   )    // currently have correct table size( entries?? )
	{
        
     //  sprintf( strg,"Create TABLE...(sin-cos)  %ld entries", numSamps );  TRACE( strg );

        quickFact =   65536.0  *  angFreq  *  twoPI;      // remove some calcs
    	
    	
    	for(   i=0L;   i< numSamps;   i++   )
		{

          /***********************
       	  	ang =   (double)i  *  angFreq    *   twoPI;
        	tp = sin( ang  );
            sinNEW = tble->Sin[ i ] =  (long)(  tp * SPFRACFACT  );     // make it a Frac ( 2**30 ) **** WHY NOT??? *******
       	    tp = cos( ang );
            cosNEW = tble->Cos[ i ] =  (long)(  tp * SPFRACFACT  );     // make it a Frac ( 2**30 ) **** WHY NOT??? *******
          *********************/
          
			/****  ...current for MAC:
			 long       fxAng;  
            fxAng =  (long)(  (double)i  *      65536.0  *  angFreq  * twoPI  );            
        //  fxAng =  (long)(  (double)i  *     quickFact    );   // use SUBTITUTION
                                   
            tble->Sin[  i ]  =    FracSin(   fxAng  );
            tble->Cos[ i ]  =    FracCos(  fxAng  );
			*****/


      // ******************* WOULD a  'Windowing-function' give better data ???  Do not think so.   ******************** 

       	  	double  ang,  windowFunct,  part,   cosComp,  sinComp;


			part  =   (    (double)i   -   0.5 * ((double)(numSamps -1))    )     /   (   0.5 *  ((double)(numSamps +1))    );   

			windowFunct =   1.0  -  ( part  *  part );  //   a 'Welch' window(   "Signal and Image Processing with Neural Networks",  pp 93 )

						//	TRACE(   "Window [  %d  ]  =    %f  \n",       i,    windowFunct  );



			ang  =    (double)i    *  angFreq    *   twoPI;


			if(   doWindow   )
			{
        		sinComp =   sin( ang  )  *  windowFunct;
				m_trigTabl.Sin[ i ]  =    (long)(  sinComp * SPFRACFACT  );   // make it a Frac ( 2**30 ) **** WHY NOT??? *******

       			cosComp =   cos( ang )  *   windowFunct;
				m_trigTabl.Cos[ i ]  =   (long)(  cosComp * SPFRACFACT  );   // make it a Frac ( 2**30 ) **** WHY NOT??? *******
			}
			else
			{			
        		sinComp =  sin( ang  );
				m_trigTabl.Sin[ i ]  =    (long)(  sinComp * SPFRACFACT  );   // make it a Frac ( 2**30 ) **** WHY NOT??? *******

       			cosComp =  cos( ang );
				m_trigTabl.Cos[ i ]  =   (long)(  cosComp * SPFRACFACT  );   // make it a Frac ( 2**30 ) **** WHY NOT??? *******
			}



			if(    i  >=     ( numSamps/2 )    )
			{
				int  dumBreak =   9;   //   Why did I want to stop halfway for this DEBUG look???    11/2009
			}


						 // ??? *****PROB: will 'i' make this OVERFLOW( will I have to 'UNWRAP angle'???? )
			int  dummyBreak =   9;
		}
  



 //     m_trigTabl.numEntries  =   0;           // Flag for REinit if other routine ********   BAD ?????
		m_trigTabl.numEntries  =   numSamps;    //  ******   OK ???   11/2009


  	    m_trigTabl.curAngFreq  =   angFreq;  // show that table is INITIALIZED with this global 
  	
   
		return  true; 
	}
  

    return  true;   // **** SHOULD test for OVERFLOW, and return 'FAIL'
}




											////////////////////////////////////////


short    DFTrowProbe::Get_ShiftFactor(   long  numSamples    )  
{

	short    shiftFact =   -1;

	
     			    			 
   				   // How much to reduce PRECISION with 'large SUMMATION' in FreqProbe_Grain()

    if(  numSamples <  64  )    shiftFact =  0;      		
	else 
	{ 
		if(  numSamples <  128  )    shiftFact =  1;            
	    else 
		{
		   if(  numSamples <  256  )    shiftFact =  2; 
           else 
		   { 
			  if(  numSamples <  512  )     shiftFact =  3; 
              else   
			  {
				  if(  numSamples <  1024  )     shiftFact =  4; 
				  else
				  {
					  if(  numSamples <  2048  )    shiftFact =  5; 
					  else
					  {
						 if(  numSamples <  4096  )    shiftFact =  6; 
						 else
						 {
							if(  numSamples <  8192  )    shiftFact =  7; 
							else
							{
							   if(  numSamples <  16384  )    shiftFact =  8; 
							   else
							   {   ASSERT( 0 );
								   shiftFact =  9; 
							   }

							}
						 }
					  }
				  }
			  }				
		   }       		 	
       }
	}

	return  shiftFact;
}
 



											////////////////////////////////////////


short 	DFTrowProbe::Calc_Magnitude_of_Cell(    char  *samplesBits,    bool useAlternateRead,   long  indexToCircSndsamplesProbeStart,    CString&   retErrorMesg    )
{


	//   If  useAlternateRead == TRUE,   then  'samplesBits'   is NOT used,  just  indexToCircSndsamplesProbeStart, which is in SAMPLES


	//   like:    bool	DFTrow::Transform_Row(   CString&   retErrorMesg    )


	retErrorMesg.Empty();


	if(       ! useAlternateRead
		 &&     samplesBits  ==  NULL   )
	{
		retErrorMesg  =   "DFTrowProbe::Calc_Magnitude_of_Cell  FAILED,  samplesBits is NULL." ;
		return  -9;
	}

   			
	long    totalSamps  =   m_cellLength;    



   									//  shiftFact:   How much to reduce PRECISION with 'large SUMMATION' 

	short  shiftFact  =     Get_ShiftFactor(   m_cellLength   );  


			/***     These 2 vars( sampleCalcFineRedux, nuVolmFactCell  ) can be SIMULTANEOUSLY change to give less TRUNCATION in fixedpoint calcs

					sampleCalcFineRedux			nuVolmFactCell				Effect

							  12							 7000.0                     ehh	   ..kind of distorted, too much overbrite					


					          11							14000.0					   ehh	   ..kind of distorted, too much overbrite	
					          11							21000.0					  better ,  but a little weak


							  10							28000.0                     Not  bad   ..kind of distorted, too much overbrite	
							  10							38000.0                   Better	
							  10							48000.0                   Pretty good	[  close to OVERFLOW by 74551   ...OK ???  ]

							   9							99000.0                  Pretty good  [  close to OVERFLOW by  1289  ...TOO CLOSE !!!    ]	 

			***/
	        

	short    sampleCalcFineRedux =     11;   //   11[ BEST  9/2012 ]              10[ overflow below, 9/2012 ]
															   //    Highway 49  might be the LOUDEST recording I have to test it on. User can always adjust source boost upward.
																//
	                                                            //  OLD ( obsolete) ( NOT below 11 !!! ,  even with 10 it comes witn  )    additional adjustment to reduce ' INDIVIDUAL  sampleCalcs'  ( and adjust volume( 13 is too much )
						

	double   nuVolmFactCell  =    m_sPitchCalc.m_logDFTdarknessFactor;    //   48000.0;  the bigger,   the smaller DARKER the LogDFT   [ same as in  DFTrowProbeCircQue::Transform_Row   9/12



						//  'trigValueShiftRedux'   shifts/reduces the accuracy of the Values in the lookup TRIG-table.  If too big
					    //									big,  then  calculation are TRUNCATED.   BUT if too small, then the sums might OVERFLOW( install test ) 

	short      trigValueShiftRedux =     shiftFact    +   sampleCalcFineRedux;    //   sampleCalcFineRedux:    additional adjustment to adjust volume( 13 is too much )


	long       denom  =     1  <<   shiftFact;    //   As MORE samples exist in a Kernal,   'shiftFact'  gets BIGGER  to make sums( and calcs ) smaller and not go OUTofBOUNDS of long integer 


	double    cellSumsDivideRedux =        (   (double)m_cellLength   /   (double)denom   )     *  nuVolmFactCell; 

		//    As  circCueLength  gets SMALLER,  denom  also gets smaller,  and so cellSumsDivideRedux  also 
	    //    gets SMALLER( cause less members in the kernal to sum )
 
//	double    chunkSumsDivideRedux =    (   (double)m_cellLength   /   (double)denom   )     *  nuVolmFactChunk; 





//	double   testSumCos,   testSumSin;
	long      sampVal,   indexVirtual,   sumCosCell = 0L,    sumSinCell = 0L;      //   SHOULD be initialized for every cell,  unlike the Circular Buffer

	char     *src  =  samplesBits;   //  'samplesBits'	 HAS  the   'eventsOffset'  calced into it.   	 //  8-bit samples in MONO  




	for(    long  sampleIdx =0;      sampleIdx <  totalSamps;      sampleIdx++    )
	{


		if(    useAlternateRead    )
		{
			indexVirtual  =       indexToCircSndsamplesProbeStart   +    sampleIdx;     //   Looks good,   1/29/12

			sampVal       =     (long)(     m_sPitchCalc.Read_SndSample_CircularQue_Value(  indexVirtual  )       );
		}
		else
		{	sampVal       =     (long)(  *src   );

		    src++;    //  only want to advance this for THIS case only, don't read out of bounds 1/12
		}

																										//   calc new coeffecients for the Sample

		/**************   BEST test routine for OVERFLOW    9/2012

		long  cosVal =     sampVal   *    (    m_trigTabl.Cos[ sampleIdx ]    >>   trigValueShiftRedux    );	
		long  sinVal  =     sampVal   *    (    m_trigTabl.Sin[  sampleIdx ]    >>   trigValueShiftRedux    );   


		testSumCos =      (double)sumCosCell   +    (double)cosVal;
		testSumSin  =      (double)sumSinCell    +    (double)sinVal;


		if(          testSumCos  >=   2147483647.0   )
		{  int  dummy =   9;  }                                   //  set breakpoint here
		else if (   testSumCos  <=  -2147483647.0   )
		{  int  dummy =   9;  }

		if(          testSumSin  >=   2147483647.0   )
		{  int  dummy =   9;  }
		else if (   testSumSin  <=  -2147483647.0   )
		{  int  dummy =   9;  }

		sumCosCell  +=     cosVal;   //  a)   Keep an 'UNsubtracted sum'  for REconstruction(  m_magReconstruct,  m_phaseReconstruct  )	
		sumSinCell   +=     sinVal;	
		****/
	

		sumCosCell  +=     sampVal   *    (    m_trigTabl.Cos[  sampleIdx ]    >>   trigValueShiftRedux    );   //  should be faster this way
		sumSinCell  +=     sampVal   *    (    m_trigTabl.Sin[  sampleIdx ]    >>   trigValueShiftRedux    );	



//		transForm.Find_Long_Minmax(  sumCosCell  );     transForm.Find_Long_Minmax(  sumSinCell  );    ******  this might be a BAD test 9/2012
	}   




	double  realCoef   =      (double)sumCosCell     /   cellSumsDivideRedux;  
	double  imagCoef   =      (double)sumSinCell     /   cellSumsDivideRedux;
			                      

	short     mag      =      (short)(     sqrt(    realCoef * realCoef    +    imagCoef * imagCoef    )      ); 
	if(       mag  >  255  )
		mag =   255;
	else if(  mag  < 0  )
	{  
		ASSERT( 0 );     //   Not mathematically possible  ( never a negative Square Root )   9/2012
		mag =   0;
	}


	return  mag;
}






////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


DFTrowProbeCircQue::DFTrowProbeCircQue(   SPitchCalc&  sPitchCalc,    long  cellLength    )  
																						:   DFTrowProbe(   sPitchCalc,  cellLength  )
{

			//	  Calculates the values for the logDFTtrForm (stored as an OffMap) 


	/***

		NEED to have Phase Data as well,  for  new function    Tap_CircQue_Write_Pixel_to_logDFT() 

	Is now working well     9/2012

	***/


	m_cosCircQue =   m_sinCircQue  =   NULL;
	m_chunksProcessed =  0;
	m_sumCosQue =   0;      //  init SUMS of circQue...
	m_sumSinQue  =   0; 

	m_circQuesIndex =  0;      //   ...and  its COUNTER.



	m_sinCircQue    =        new    long[   cellLength   ];		
	if(   m_sinCircQue  ==  NULL   )
	{
		ASSERT( 0 );
		AfxMessageBox(   "DFTrowProbeCircQue::DFTrowProbeCircQue  FAILED, could not alloc  m_sinCircQue."   );
		return;
	}

	m_cosCircQue    =        new    long[   cellLength   ];		
	if(   m_cosCircQue  ==  NULL   )
	{
		ASSERT( 0 );
		AfxMessageBox(   "DFTrowProbeCircQue::DFTrowProbeCircQue  FAILED, could not alloc  m_cosCircQue."   );
		return;
	}



	Init_CircleQue_ArrayElements_and_Sums();
}




											////////////////////////////////////////


void   DFTrowProbeCircQue::Init_CircleQue_Sums_Only()
{


  ASSERT( 0 );   // *****************  NEVER USE this function...  it CORRUPTS the CircQue , causes OVERFLOW  **************
						  // 
						  //  You can NOT zero the sums without seroing the ArrayELEMENTS, because th algo later SUBTRACTS those
	                      //  ArrayELEMENTS from the Zeroed-Sum  --  a sum that it expects to be the right size to handle the subtractions.  9/4/2012 


	m_sumCosQue =   0;      //  init SUMS of circQue...   Do NOT want occasional error or OVERFLOW to persist there.  9/12
	m_sumSinQue  =   0; 
}



void   DFTrowProbeCircQue::Init_CircleQue_ArrayElements_and_Sums()
{


    //  CALLED by:    SPitchCalc::Initialize_All_DFTprobe_CircularQues_ArrayElements_and_Sums()        DFTrowProbeCircQue::DFTrowProbeCircQue()    
	
 

	if(  m_sinCircQue ==  NULL    ||    m_cosCircQue ==  NULL   ) 
	{
		ASSERT( 0 );
		AfxMessageBox(   "DFTrowProbeCircQue::Init_CircleQue_ArrayElements_and_Sums  FAILED,   m_sinCircQue or  m_cosCircQue is NULL. "   );
		return;
	}



	/***
	for(   long  by =0;      by <  m_cellLength;      by++    )           //   m_cellLength :   how many samples for 18 periods   
	{
		m_sinCircQue[   by  ] =   0;
		m_cosCircQue[   by  ] =   0;
	}
	***/
	if(   m_cellLength  > 0   )  
	{
		memset(    &( m_sinCircQue[0]  ),     0,      m_cellLength  *  sizeof( long )    );    //  should be a lot faster   9/4/12

		memset(    &( m_cosCircQue[0]  ),     0,      m_cellLength  *  sizeof( long )    );
	}
	else
	{	ASSERT( 0 );   }


	
												//   Initialize  sums,  indexes,   counters 

	m_sumCosQue =   0;      //  init SUMS of circQue...
	m_sumSinQue  =   0; 

	m_circQuesIndex =  0;      //   ...and  its INDEX


	m_chunksProcessed =  0;
}




											////////////////////////////////////////

DFTrowProbeCircQue::~DFTrowProbeCircQue()
{

	if(   m_sinCircQue  !=  NULL  )		
		delete  m_sinCircQue;

	if(   m_cosCircQue  !=  NULL  )		
		delete  m_cosCircQue;
}



											////////////////////////////////////////


void	  DFTrowProbeCircQue::Tap_CircQue_Write_Pixel_to_logDFT(    long  x,     double  chunkSumsDivideRedux   )
{


ASSERT( 0 );     //   NOT connected, but might want to keep around for a while because of its concepts.  9/2012



	// ********************  OPTIMIZE this   9/12  *********************************


			//  Examines the current  AVERAGING in  the CircQue,  and  calcs a Magnitude based on that average.

    /***
	if(   !  m_sPitchCalc.m_logDFTtransform->Is_MidiPitch_On_Map(  m_midiNumber  )    )
	{
		ASSERT( 0 );      // ****************  TAKE 
		return;
	}
	***/

	/***
	if(         x   <    0    
		  ||   x  >=   m_sPitchCalc.m_logDFTtransform->m_width   )
	{
		ASSERT( 0 );	
		return;
	}
	***/



	double  realCoef    =      (  (double)m_sumCosQue  )     /  chunkSumsDivideRedux;     // ***???  cellSumsDivideRedux   OK ???

	double  imagCoef   =     (  (double)m_sumSinQue   )      /  chunkSumsDivideRedux;



	short  mag   =       (short)(     sqrt(    realCoef * realCoef    +    imagCoef * imagCoef    )     ); 

	if(      mag > 255   )
	{
			//	if(    mag   >   transForm.m_biggestOutValPixel     )
			//		transForm.m_biggestOutValPixel  =    mag;

		mag =   255;     //  ***  TOO  BIG  (  550 )  *********************************

			//	transForm.m_overBriteCnt++;
	}
	else if(  mag < 0  )
	{
		ASSERT( 0 );
		mag =   0;
	}



	/****  Reinstall if use this map
						
	if(     m_sPitchCalc.m_logDFTtransformDEBUG   !=  NULL    )   //   ** DEBUG ONLY **
	{
			
			// *******************************  hard,  calc where on DFTmap to write the pixel ( Navigator and Old PitchScope use DIFFERENT samples/pixel
			//																	PitchScope  512 samps per pixel,  and  Navigator is 1104 sampsper pixel    9/2012  


		ASSERT( 0 );   // ********   NEED to rethink this,  because of the changes done in  9/2012  ********************

		x =     m_chunksProcessed;      //  this is the way the DFT is calced in VoxSep
//		x =     m_chunksProcessed    -    m_lagXwriteDFTpixel;      //    *********  TEMP test  ,  should suck


		if(   x  < 0    ||     x  >=  m_sPitchCalc.m_logDFTtransformDEBUG->m_width  )
		{
			ASSERT( 0 );
			return;
		}
	}  
	****/


//	short   y =     ( m_sPitchCalc.m_logDFTtransform->m_height  -1 )    -  m_yVirtual;    //  ***OMIT,  just for debug ****  m_dftChunkMap is a DIB, and INVERTED


	/****
	if(     m_sPitchCalc.m_logDFTtransformDEBUG   !=  NULL    )      //   ** DEBUG ONLY **
	{
	    
		ASSERT( 0 );   ///   *** Need to RETHINK this  *********************************************

		long  yCoord =	 m_sPitchCalc.m_logDFTtransformDEBUG->Pitch_2Ycoord(   m_midiNumber   ); 


		m_sPitchCalc.m_logDFTtransformDEBUG->Write_Pixel(    x,  yCoord,    mag,mag,mag  );
	}
	else
	{ 
	***/
		
		long  yCoord =	 m_sPitchCalc.m_logDFTtransform->Pitch_2Ycoord(   m_midiNumber   ); 


		m_sPitchCalc.m_logDFTtransform->Write_Pixel(   x,  yCoord,   mag, mag, mag  );
//	}

}




											////////////////////////////////////////


bool	 DFTrowProbeCircQue::Transform_Row(   SndSample&  sndSample,    long  eventsOffset,   long  samplesToProcess,   CString&   retErrorMesg   )
{


	retErrorMesg.Empty();
	ASSERT(  m_sPitchCalc.m_logDFTtransform  );



	long   totalSamps  =     sndSample.Get_Length();  

    char   *src   =    sndSample.Get_StartByte()   +   eventsOffset;			 //  8-bit samples  in  MONO  


	long  circCueLength  =   m_cellLength;    




	short  shiftFact  =     Get_ShiftFactor(  circCueLength    );    

			/***     These 2 vars( sampleCalcFineRedux, nuVolmFactCell  ) can be SIMULTANEOUSLY change to give less TRUNCATION in fixedpoint calcs

					sampleCalcFineRedux			nuVolmFactCell				Effect

							  12							  7000.0                     ehh	   ..kind of distorted, too much overbrite					


					          11							14000.0					   ehh	   ..kind of distorted, too much overbrite	
					          11							21000.0					  better ,  but a little weak


							  10							 28000.0                     Not  bad   ..kind of distorted, too much overbrite	
							  10							 38000.0                   Better	
							  10							 48000.0                   Pretty good	[  close to OVERFLOW by 74551   ...OK ???  ]

							   9							  99000.0                  Pretty good  [  close to OVERFLOW by  1289  ...TOO CLOSE !!!    ]	 

			***/
	        
	short     sampleCalcFineRedux =     11;   //  11[ no overflow below. 9/2012 ]      10[  BAD, get overflow below with  { testSumCos,   testSumSin }  ]   9/3/12         
	
	                                       //   OLD( not applicable ):    is it true  ( NOT below 9 !!! ,  even with 9 it comes witin 9013 of OVERFLOW)    additional adjustment to reduce ' INDIVIDUAL  sampleCalcs'  ( and adjust volume( 13 is too much )



						//  'trigValueShiftRedux'   shifts/reduces the accuracy of the Values in the lookup TRIG-table.  If too big
					    //									big,  then  calculation are TRUNCATED.   BUT if too small, then the sums might OVERFLOW( install test ) 

	short      trigValueShiftRedux =     shiftFact    +   sampleCalcFineRedux;    //   sampleCalcFineRedux:    additional adjustment to adjust volume( 13 is too much )


	long       denom  =     1  <<   shiftFact;  	//   As MORE samples exist in a Kernal,   'shiftFact'  gets BIGGER  to make sums( and calcs ) smaller and not go OUTofBOUNDS of long integer 





						//     the bigger  nuVolmFactChunk,   the  DARKER the logDFT  

	double   nuVolmFactChunk =   m_sPitchCalc.m_logDFTdarknessFactor;  //    48000.0;         same as in  DFTrowProbe::Calc_Magnitude_of_Cell() 
														               //    Highway 49  might be the LOUDEST recording I have to test it on. User can always adjust source boost upward.


						//    As  circCueLength  gets SMALLER,   denom  also gets smaller,   and so  cellSumsDivideRedux  also 
						//    gets SMALLER( cause less members in the kernal to sum )
 
	double    chunkSumsDivideRedux =    (   (double)circCueLength   /   (double)denom   )     *  nuVolmFactChunk; 





	long   xWriteColumn       =     m_sPitchCalc.Get_logDFTs_Write_Column();  

	long    firstXvalue           =     xWriteColumn   -   m_sPitchCalc.m_hiResRatioLogDFT   + 1;   //   for  m_hiResRatioLogDFT = 2,   write at  18 and  19   [ map is 20 pixels wide ]

	long    writeGapInSamps =    samplesToProcess  /   m_sPitchCalc.m_hiResRatioLogDFT;

	long    writeGapMinusOne =   writeGapInSamps  -1;

	long    samplesToProcessMinusOne  =   samplesToProcess  -1; 

	long    yCoord =	  m_sPitchCalc.m_logDFTtransform->Pitch_2Ycoord(   m_midiNumber   ); 



//	double   testSumCos,    testSumSin;
	long     cosVal,  sinVal,    sampVal;
	short    xWrite;
	char     tapCount =   0;                                //    DFTrowProbeCircQue::Transform_Row()



	for(    long  sampleIdx =0;      sampleIdx <  samplesToProcess;      sampleIdx++    )    			
	{

															   //  *** Try to keep this LOOP as FAST and as SIMPLE as possible.  ****
		sampVal =     (long)(  *src++  );

																																	

	    m_sumCosQue   -=     m_cosCircQue[   m_circQuesIndex   ];	     //  SUBTRACT out  the oldest Que element
		m_sumSinQue   -=     m_sinCircQue[    m_circQuesIndex  ];	



		cosVal =     sampVal   *    (    m_trigTabl.Cos[ m_circQuesIndex ]    >>   trigValueShiftRedux    );     //  calc the new sample	
		sinVal =     sampVal   *    (    m_trigTabl.Sin[  m_circQuesIndex ]    >>   trigValueShiftRedux    );   


		/**************   BEST test routine for OVERFLOW ( test BEFORE the new element is ADDED to the large sum )     9/2012

		testSumCos =      (double)m_sumCosQue   +    (double)cosVal;
		testSumSin  =      (double)m_sumSinQue    +    (double)sinVal;


		if(          testSumCos  >=   2147483647.0   )
		{  int  dummy =   9;  }                                   //  set breakpoint here
		else if (   testSumCos  <=  -2147483647.0   )
		{  int  dummy =   9;  }

		if(          testSumSin  >=   2147483647.0   )
		{  int  dummy =   9;  }
		else if (   testSumSin  <=  -2147483647.0   )
		{  int  dummy =   9;  }
		
		****/

		m_sumCosQue	 +=    cosVal;            //  ADD in  the NEW Cue element
		m_sumSinQue	 +=    sinVal;	



// **************************************  BAD OVERFLOW test below.  ( NOT that good, one above is better )  9/2012  ****************************************************

//	m_sPitchCalc.Find_Long_Minmax(  m_sumCosQue  );    	m_sPitchCalc.Find_Long_Minmax(  m_sumSinQue  );      //  the BEST to test for overflow
//           ....See the TRACE dump in  PsNavigatorDlg::On_Pause_Button()




		m_cosCircQue[  m_circQuesIndex  ]  =    cosVal;         //  ASSIGN  the NEW Cue element  in the circular que for future  'subtraction'
		m_sinCircQue[   m_circQuesIndex  ]  =    sinVal;

		


										             //  we   "Tap the CircQue-Sums"   every  N  number of samples to write a Pixel to the LogDFT  

		 xWrite =  -1;    //  init


		if(              sampleIdx   ==   samplesToProcessMinusOne                      //    Case  A.      ( samplesToProcess -1  )    
				 &&    tapCount   <   m_sPitchCalc.m_hiResRatioLogDFT    )    // sometimes we hit Case B  enough times that we do NOT have to do this one

			xWrite =   firstXvalue  +   m_sPitchCalc.m_hiResRatioLogDFT  - 1;    //  this is the LAST write column. Sometimes do the LAST with Case B.

		else if(   ( sampleIdx  %  writeGapInSamps )  ==  writeGapMinusOne  )  //   Case  B.  ( writeGapInSamps -1  ) )   ...for 2,   happens at    sampleIdx = 551  

			xWrite =    firstXvalue  +   sampleIdx / writeGapInSamps;		  //   for  m_hiResRatioLogDFT = 2,   offsets[ sampleIdx / writeGapInSamps ] are   {  0,  1  }		 



		if(   xWrite  >=  0    )        //  use xwrite as the boolean flag.     This gets hit every 368 samples   ( 1104 / 3 =  368  )    		
	    {
//			ASSERT(   xWrite  >=   0   );                                  *** Try to keep this LOOP as FAST and as SIMPLE as possible.  ****
//			ASSERT(   xWrite  <=   xWriteColumn   );
                      

//			Tap_CircQue_Write_Pixel_to_logDFT(    xWrite,    chunkSumsDivideRedux   );      ...hope that this INLINE code will be faster than the function call.  9/2012
			double  realCoef    =      (  (double)m_sumCosQue  )    /  chunkSumsDivideRedux;      
			double  imagCoef   =     (  (double)m_sumSinQue   )     /  chunkSumsDivideRedux;

			short    mag    =       (short)(     sqrt(    realCoef * realCoef    +    imagCoef * imagCoef    )       ); 
			if(        mag  > 255   )
				mag =   255;  
			/*********  Not mathematically possible  ( never a negative Square Root )   9/2012
			else if(    mag  < 0  )
			{  ASSERT( 0 );   mag =   0;   }
			***/

			m_sPitchCalc.m_logDFTtransform->Write_Pixel(   xWrite,  yCoord,    mag, mag, mag   );


			tapCount++;
			m_chunksProcessed++;    //    in a virtual DFTmap, this is the amout of pixels written to
	   }



		m_circQuesIndex++;

		if(    m_circQuesIndex   >=   m_cellLength    )     //  do not do any calcs,  just keep the index 'circular'
			m_circQuesIndex  =    0;      //  init  COUNTER of circQue ( but NOT its SUMS !!!  )

	}   //	  for(  sampleIdx =   ...totalSamples



	ASSERT(   tapCount   ==   m_sPitchCalc.m_hiResRatioLogDFT   );  

	return   true;
}











