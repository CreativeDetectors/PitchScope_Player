/////////////////////////////////////////////////////////////////////////////
//  CalcNote.cpp   -   properties for a new midi note:  ScalePitch, Octave, Duration
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2009-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////

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


#include  "..\comnAudio\FundamentalCandidate.h"




#include  "DFTtransforms.h"
  

#include  "..\ComnAudio\HarmPairsTrForm.h"
#include  "..\ComnAudio\HarmPairsVerter.h"


#include  "sndSample.h"



#include "FundCandidCalcer.h"



#include  "..\ComnAudio\CalcNote.h"


#include  "..\ComnAudio\SPitchCalc.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////






////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


CalcedNote::CalcedNote()
{


	beginingSampleIdxFile =    -9;     //    'CalcedNote'  is not a INTERVAL in Time,  it just represents  One PIE-SLICE  of pitch-Detection data.   2/2012  

//	beginingSampleIdxFileALT =    -9;   



	primaryQuesSampleIdx =   -9;

	curSampleWaveman =  -9;


	scalePitch =  -5;
	synthCode =   -9;
	octaveIndex =  -9;		

	detectScoreHarms  =  -11;

	detectAvgHarmonicMag =  -11;



	for(   short i =0;    i <  kCountOfNavOctaveCandidates;    i++  )
		detectScoreOctaveCandids[  i  ]  =   -3;





	pieSliceIdxAtDetection =  -9;


	pieSliceIdxAfterFiltration =   -8;


	expectedEventNumber  =  -9; 

	realEventNumber  =  -9;  

	pieSliceCounter =  -9;


//	outOfSync  =  false;   

}



											////////////////////////////////////////


bool   CalcedNote::Has_Valid_Data()
{


	if(   beginingSampleIdxFile   < 0  )	   
		return  false;



//		if(   endingSampleIdxFile    <  0  )	   return  false;


//		if(   scalePitch  < 0  )   < -1      return  false;    ****** NOT sure if I WANT this...   2/4/12 


	return  true;
}





////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


MidiNote::MidiNote()
{

	beginingSampleIdxFile =    -9;
	endingSampleIdxFile    =   -9;             //  will hold Note's time END boundaries when doing a FILE save to NoteList

	scalePitch =  -5;
	octaveIndex =  -9;		// *******  NOT needed?    Not receiving data ????   2/7/12

	detectScoreHarms  =  -11;
	detectAvgHarmonicMag =  -11;

	for(   short i =0;    i <  kCountOfNavOctaveCandidates;    i++  )
		detectScoreOctaveCandids[  i  ]  =   -11;
}




											////////////////////////////////////////


bool   MidiNote::Has_Valid_Data()
{

	if(   beginingSampleIdxFile   < 0  )	   
		return  false;


	if(   endingSampleIdxFile    <  0  )	   
		return  false;


//	if(   scalePitch  < 0  )   < -1      return  false;    ****** NOT sure if I WANT this...   2/4/12 


	return  true;
}



											////////////////////////////////////////


void    MidiNote::Copy_In_CalcNotes_Data(   CalcedNote&  cNote   )
{


	beginingSampleIdxFile =    cNote.beginingSampleIdxFile;


//	curSampleWaveman =  cNote.curSampleWaveman;

	scalePitch     =   cNote.scalePitch;

	octaveIndex =    cNote.octaveIndex;		

	detectScoreHarms       =  cNote.detectScoreHarms;

	detectAvgHarmonicMag =  cNote.detectAvgHarmonicMag;


	for(   short i =0;    i <  kCountOfNavOctaveCandidates;    i++  )
		detectScoreOctaveCandids[  i  ]  =   cNote.detectScoreOctaveCandids[  i  ];
}

