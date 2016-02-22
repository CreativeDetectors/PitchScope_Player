/////////////////////////////////////////////////////////////////////////////
//
//  FundamentalCandidate.cpp   -   used for Octave Detection
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




#include  "..\comnFacade\VoxAppsGlobals.h"

//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"      //  #include <FixMath.h>    ...Mac's version of Fixedpoint 
#include  "..\ComnGrafix\CommonGrafix.h"      //  #include "commonmy.h"   		

#include  "..\ComnGrafix\OffMap.h"  
#include  "..\comnGrafix\TransformMap.h"





///////////////////////////
#include  "..\ComnAudio\sndSample.h"


#include  "FundamentalCandidate.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////


extern  OctaveCorrectParms    lastOctaveCorrectParmsGLB;




double    Get_Hertz_Freq_from_MidiNumber(  short  midiNum  );

extern   double   noteFreq[];



////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


FundamentalCandidate::FundamentalCandidate()
{
		
	m_midiPitch    =   -1;		
	m_octaveIdx  =    -1;


	m_avgHarmonicMag  =  -1;



	m_spikeScore =   -1;

	m_finalOctvCandidScore =   -1;



	for(    short  harm =0;      harm <  Harmonic_Count_Extra();       harm++    )
		Set_Hamonics_Mag(   harm,    INITIALIZEhMAG    );
	

	m_isAsubOctave =   false;



	m_missedHarmonicBandCnt =  -1;



	for(  short  n=0;   n < NEIGHBORcOUNT;   n++   )
	{
		m_neighborMags[ n ].L =   -99;
		m_neighborMags[ n ].R =  -99;
	}



	m_harmsNeighborhoodStrength =   0.0;
}



											////////////////////////////////////////


short     FundamentalCandidate::Get_Octave_Index(  double  freq  )        
{

	 short   octvCalc =  -9;
	 double  borders[  16  ],     travBorder;



	  //  IDEAL freqs =   		D#  =  9956.0,       E =   10548.0     ....Midpoint is   10252.0

	 travBorder =   10252.0;    //  make this a little lower so cut-Off so have a little TOLERANCE from the ideal freq



	for(  short ov =  10;      ov >=  0;      ov--    )
	{
		borders[  ov  ]  =    travBorder; 		
		travBorder      =     travBorder  /  2.0;
	}




	if((        freq >=    borders[ 0 ]    )&&(       freq  <   borders[ 1 ]   ))     //  ov =  0		   10.3  ... is lower Border  
		 octvCalc  =  -4;		

    else if((    freq >=     borders[ 1 ]    )&&(    freq  <   borders[ 2 ]    ))     //  ov = 1	   20.6
		 octvCalc  =  -3;		

     else if((    freq >=    borders[ 2 ]    )&&(    freq  <  borders[ 3 ]    ))     //  ov = 2		   42.20
		 octvCalc  =  -2;		

     else if((    freq >=    borders[ 3 ]    )&&(    freq  <  borders[ 4 ]   ))     //  ov = 3			82.41 
		 octvCalc  =  -1;													


    else if((    freq >=    borders[ 4 ]    )&&(   freq  <    borders[ 5 ]    ))     //  ov = 4		   164.81   ...1st string on guitar
		 octvCalc  =  0;																											

     else if((   freq >=    borders[ 5 ]     )&&(   freq  <   borders[ 6 ]    ))     //  ov = 5			311.13
		 octvCalc  =  1; 
     
     else if((   freq >=    borders[ 6 ]     )&&(   freq   <  borders[ 7 ]    ))     //  ov = 6			659.26
		 octvCalc  =  2;

     else if((   freq >=    borders[ 7 ]     )&&(   freq  <  borders[ 8 ]   ))     //  ov = 7			1318.5 
		 octvCalc  =  3;
  
     else if((   freq >=    borders[ 8 ]    )&&(   freq <   borders[ 9 ]   ))     //  ov = 8			2637.0
		 octvCalc  =  4;

     else if((   freq >=      borders[ 9 ]     )&&(   freq <  borders[ 10 ]   ))    //  ov = 9			2637.0
		 octvCalc  =  5;   

     else if((   freq >=     borders[ 10 ]    )&&(   freq <  (2 *borders[ 10 ])    ))    //  ov = 10			10548.0 
		 octvCalc  =  6;   	 
	 else
	 {	
		 //   ASSERT( 0 );    //   6/07  get here  with 9hz.   Just give it an undefined.  
		  octvCalc  =  -9;		
	 }


     return   octvCalc;    //    octv;  
} 



											////////////////////////////////////////


short     FundamentalCandidate::Read_Hamonics_Mag(  short  harmIdx,   bool&  isUndefined   )
{


	isUndefined =  false;   //  init

													//  need to check for flagged,  undefined vals

	short  val  =    m_harmonicMags[  harmIdx  ];


	if(   val   >=  0    )
	{
		isUndefined =  false;
		return  val;
	}


	
	if(   val   ==   UNDEFINEDhMAG   )
	{
		isUndefined =  true;
		return  val;
	}
	else
	{	ASSERT(  0  );    // was initialized or calced wrong  

		isUndefined =  true;
		return  val;
	}
}


											////////////////////////////////////////


void     FundamentalCandidate::Set_Hamonics_Mag(    short  harmIdx,    short  val    )
{

	if(    harmIdx  >=  Harmonic_Count_Extra()    )
	{
		ASSERT( 0 );
		return;
	}


	m_harmonicMags[  harmIdx  ]  =   val;
}




											////////////////////////////////////////


long  	FundamentalCandidate::Get_neighborMags_Magnitude(   short  midiOffset    )
{

	long   retMagnitude =  -1;

	bool  foundIt =   false;


	for(   short  neib = 0;   neib < NEIGHBORcOUNT;   neib++   )
	{

		if(   m_neighborMags[  neib  ].L   ==   midiOffset   )
		{

			foundIt  =   true;

			retMagnitude  =    m_neighborMags[  neib  ].R;
			
			return  retMagnitude;
		}
	}


	ASSERT( 0 );   //  ****  Big error, we should have found it. If get here I buit my arrays incorrectly in  Calc_Neighborhoods_AverageMag  ****
	return  -33;
}




											////////////////////////////////////////


double 	FundamentalCandidate::Calc_Harms_Neighborhood_Strength(   short  harmIdx,   bool  useWeights,    bool&  retIsUndefinedDev     )
{

	/*
		My clever algorithm to detect which of the possible Octave-Candidates is the most accurate for the time duration.  James Paul Millard

		To understand how this works you really need to graph what the 4 Harmonic-Profiles look like for each of the Octave-Candidates, while paying
		attention to what the correct octave's Harmonic Profile actually looks like. 

		**** STUDY  \jmNotes\Harmonic-Profiles_and_NeighborHarmonics-Dominance.jpg  ****  
		

		'Harmonic-Profile' -  Bar graphs with each of the MAGNITUDES of the note's 8+ Harmonics extending vertically for each possible Octave-Candidate Note.

		'Neighbor-Harmonics Graph' -  Shows the Core-Harmonic in the center, surrounded by the competing Harmonics from the Octave-Candidate that is just one octave below.
		                              The bigger the central RED length of the Core-Harmonic, the more dominant the Core-Harmonic is when in COMPETITION
									  with the below Octave-Candidate's harmonics. The BLUE portion of the Core-Harmonic is the average magnitude of the
									  competing Neighbor-Harmonics from the other Octave-Candidate which is just below. 
									  The Octave-Candidate Harmonic-Profile with the most amount of RED in its Core-Harmonics is the correct octave. That is the "Spike Score."

									  The "Neighborhood Harmonics" are from the Octave-Candidate which is just ONE OCTAVE BELOW this Core Octave-Candidate, and must
									  NOT have the same frequency as any of the harmonics in the Core Octave-Candidate. 

									  The Harmonic-Profile of the Octave-Candidate which is one octave below the TRUE Octave frequently looks like the "smile of an old man
									  who is missing many teeth." That principle drives this algorithm.
	*/


	ASSERT(  useWeights  );	                     //  useWeights :    is always true,  tests well   7/2007 


	double  weitIncrease  =    2.0;     //  *********  Adjust ***********


	double   retHarmsStrength =   0.0;
	double   avgNeiborVal= 0.0,   harmsMag;
	bool       isUndefinedMag;				//  Only use  8  harms for this function.  



	double	 adjWeits[ 12 ]  =   
//			  {  1.00,         0.61,       0.61,      0.39,           0.61,      0.26,       0.61,       0.23,                            0.48,      0.18,      0.38,       0.14     };  **BAD**,   by  Anssi Klapuri  on interfering signal OVERLAP probabatilities 
//            {   .70,         1.00,       1.00,      1.00,            .80,       .60,        .40,        .20,                             .15,        .10,        .10,          .5       };    //  OLD:   by  Millardo    ....mimic the mags of an ideal Harmonic profile  

               {   .70,       1.00,        1.00,         .85,           .70,        .60,         .45,         .38,                           .30,        .25,        .20,          .20       };
 //               [ 70 ],    [ 100 ],    [ 100 ],       [ 85 ],        [ 70 ],     [ 60 ],      [ 45 ],      [ 38 ],                        [ 30 ],      [ 25 ],     [ 20 ],      [ 20 ]   ...see Detection_items.rtf    7/07  ....mimic the mags of an ideal Harmonic profile  



	retIsUndefinedDev =    false;   //  init result



													//  a) 	First get the magnitude of the Core-Harmonic from THIS Core Octave-Candidate's Harmonic-Profile


	harmsMag   =      Read_Hamonics_Mag(    harmIdx,     isUndefinedMag    );     
	if(   isUndefinedMag   )
	{
		retIsUndefinedDev =   true;     //  ..get here a lot for the SUB octaves
		return  0.0;
	}



													//  b)  Calculate the average value of the neighbor-harmonics from the Octave-Candidate which is one octave below
	bool   retNeibUndefined;

	avgNeiborVal     =       Calc_Neighborhoods_AverageMag(   harmIdx,   retNeibUndefined   );
	if(  retNeibUndefined  )
	{
		retIsUndefinedDev =   true;     //  ..get here a lot for the SUB octaves
		return  0.0;
	}


	
													//  c)  Assess the strength of the Core-Harmonic relative to its competing neighbor-harmonics
	double   diff= 0.0;

	if(   harmsMag  >=   avgNeiborVal   )
		diff =    harmsMag  -  avgNeiborVal;   //  The RED section on the graph is this difference. The competing neighboring-harmonics are subtracted from the Core-Harmonic.
	else 			                             
		diff =   0.0;            //   harmsMag is less than the neighborhood,  do I want to send back a negative penalty-value or just zero ?
	


	if(    useWeights    )
		retHarmsStrength  =  	adjWeits[ harmIdx ]    *    ( diff  *  diff )     *     weitIncrease;
	else
		retHarmsStrength  =     diff  *  diff;        // ***  TEST,  do I want squares or just diff ???  ***


	return   retHarmsStrength;
}




											////////////////////////////////////////


double  	FundamentalCandidate::Calc_Neighborhoods_AverageMag(   short  harmIdx,    bool&  retIsUndefinedDev   )
{

	/*
		My clever algorithm to detect which of the possible Octave-Candidates is the most accurate for the time duration.  James Paul Millard

		To understand how this works you really need to graph what the 4 Harmonic-Profiles look like for each of the Octave-Candidates, while paying
		attention to what the correct octave's Harmonic Profile actually looks like. 

		**** STUDY  \jmNotes\Harmonic-Profiles_and_NeighborHarmonics-Dominance.jpg  ****  
		

		'Harmonic-Profile' -  Bar graphs with each of the MAGNITUDES of the note's 8+ Harmonics extending vertically for each possible Octave-Candidate Note.

		'Neighbor-Harmonics Graph' -  Shows the Core-Harmonic in the center, surrounded by the competing Harmonics from the Octave-Candidate that is just one octave below.
		                              The bigger the central RED length of the Core-Harmonic, the more dominant the Core-Harmonic is when in COMPETITION
									  with the below Octave-Candidate's harmonics. The BLUE portion of the Core-Harmonic is the average magnitude of the
									  competing Neighbor-Harmonics from the other Octave-Candidate which is just below. 
									  The Octave-Candidate Harmonic-Profile with the most amount of RED in its Core-Harmonics is the correct octave. That is the "Spike Score."

									  The "Neighborhood Harmonics" are from the Octave-Candidate which is just ONE OCTAVE BELOW this Core Octave-Candidate, and must
									  NOT have the same frequency as any of the harmonics in the Core Octave-Candidate. 

									  The Harmonic-Profile of the Octave-Candidate which is one octave below the TRUE Octave frequently looks like the "smile of an old man
									  who is missing many teeth." That principle drives this algorithm.
	*/



	bool   useMaximum  =    false;    //    false seems best   7/2007  

		
	ASSERT(   harmIdx  <  10    );    //  The arrays below can only handle  8 harms,  I would need to expland them.




// HrmIdx:  0              1                     2                     3                       4                     5                    6                      7

	PairShort    m2pairs [ 10 ]      =  
		
	{  { -12,  +7 },   { +7,  +16 },   { +16, +22 },   { +22, +26 },     { +26, +30 },  { +30, +32 },   { +32, +35  },    { +35, +37 },   { 37,  39 },   { 39,  41 },  /* { 41,  43 }  */      };  


	PairShort    m2outer[ 10 ]     =    
	{ { -36,  +16 },  { -12,  +22 },    { +7,  +26 },   { +16,  +30 },   { +22,  +32 },  { +26,  +35 },  { +30,  +37 },   { +32,  +39 },  { 35,  41  },   { 37,  43 }      };   //  for calcMethod =  2



	retIsUndefinedDev =   false;   //  init

	double  retAvgNeiborVal= 0.0; 


	long   neibMags[  10  ];
	long   neibCount;
	short  curNeibMagsIdx =  0;



	for(   short  i=0;    i< 10;    i++   )    //  init
		neibMags[  i  ] =  -88;



																			  	//   A)    get the first two INNER neighbor-Harmonics' magnitudes  
	long  leftM2Idx    =    m2pairs[  harmIdx ].L;
	long  rightM2Idx   =    m2pairs[  harmIdx ].R;

	neibMags[  curNeibMagsIdx  ]  =   	Get_neighborMags_Magnitude(   leftM2Idx   );             //  if value is < 0,  then it was off the logDFT map, and is undefined
	curNeibMagsIdx++;

	neibMags[  curNeibMagsIdx  ]  =    Get_neighborMags_Magnitude(   rightM2Idx   );
	curNeibMagsIdx++;




	short    actualNeibCnt =     0;
	double   sumNeibMags   =   0.0;

	neibCount =   4;




	leftM2Idx   =    m2outer[  harmIdx ].L;                                  //    B)    now get the two OUTER neighbor-Harmonics' magnitudes  
	rightM2Idx  =    m2outer[  harmIdx ].R;


	neibMags[  curNeibMagsIdx  ]  =   	Get_neighborMags_Magnitude(   leftM2Idx   );       
	curNeibMagsIdx++;

	neibMags[  curNeibMagsIdx  ]  =    Get_neighborMags_Magnitude(   rightM2Idx   );
	curNeibMagsIdx++;		
  	



	if(   useMaximum   )
	{
		ASSERT( 0 );

		long   maxVal =  -9;                               //   A)   NO!!!   Averaging below is best way to evaluate


		for(   short nb =0;     nb <  neibCount;     nb++  )
		{

			if(    neibMags[  nb ]  >=  0   )       //  if it is unDefined, its value is negative
			{
				if(   neibMags[  nb ]  >   maxVal   )
					maxVal =    neibMags[  nb ] ; 

				actualNeibCnt++;
			}
		}


		if(  actualNeibCnt  ==  0   )
		{
			retIsUndefinedDev =   true;     //  ..get here a lot for the SUB octaves
			return  0.0;
		}

		retAvgNeiborVal  =      maxVal;      //   calc the average
	}
	else
	{																//  B)   Averaging is BEST!!
		for(   short nb =0;     nb <  neibCount;     nb++  )
		{

			if(   neibMags[  nb ]  >=  0   )       //  if it is unDefined, its value is negative
			{
				sumNeibMags  +=     (double)neibMags[  nb ] ; 
				actualNeibCnt++;
			}
		}


		if(  actualNeibCnt  ==  0   )
		{
			retIsUndefinedDev =   true;     //  ..get here a lot for the SUB octaves
			return  0.0;
		}

		retAvgNeiborVal  =      sumNeibMags   /   (double)actualNeibCnt;      //   calc the average
	}


	return   retAvgNeiborVal;
}




											////////////////////////////////////////


bool 	 FundamentalCandidate::Calc_All_Harms_NeighborhoodStrength(   bool  useWeights,    CString&  retErrorMesg   )
{

	retErrorMesg.Empty();
	m_harmsNeighborhoodStrength =   0.0;    //  receives the result


	
	short    maxHarms =  8;    //  8   [ 8 - 10 ]   Though I can go to 10,  it seems like the last two are not really worth it.  See the ScalePitch Details diagram.   7/07   

	ASSERT(  maxHarms  <=  10  );       //  thats all my tables in  Calc_Neighborhoods_AverageMag()  can handle right now,  may be all I really need. 



	short    harmCount = 0;   
	double  totalStrength =  0.0;


	for(    short harm = 0;      harm < maxHarms;     harm++   )
	{

		bool      retIsUndefinedDev; 

		double 	 harmStren     =       Calc_Harms_Neighborhood_Strength(   harm,    useWeights,   retIsUndefinedDev   );
		if(  ! retIsUndefinedDev  )
		{
			totalStrength  +=     harmStren;
			harmCount++;
		}
	}



	if(  harmCount  <=  0  )   
	{												 //  Can get here with the subOctaves
		int dummy  =   9;
	}


	m_harmsNeighborhoodStrength  =    totalStrength;

	return   true;
}



											////////////////////////////////////////


short	 FundamentalCandidate::Get_ScalePitch()
{

	if(    m_midiPitch   <  4   )      // do not  want a negative answer
		return  -1;


	short     scalePitch  =       ( m_midiPitch  -  4  )   %   12;
	return   scalePitch;
}







////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


PolyMemberVar::PolyMemberVar()     :   m_FV( m_RealFV )     //  ,   m_strValue(  m_RealstrValue  )
{


	m_Sql_C_Type =     UNKNOWNtYPEjm;   


	m_memberVarAddress =   NULL;


	m_opCode =   PolyMemberVar::UNKNOWoPcODEpm;


	m_membervarID  =    FundamentalCandidate::UNKNOWNmEMBRid;
}


												/////////////////////////////////////////


PolyMemberVar::~PolyMemberVar()
{
}


												/////////////////////////////////////////


void	PolyMemberVar::SetType(  int  type  )
{


	m_Sql_C_Type =   type; 

// 	m_sumOfValues.SetType(  type  );   ********  ?????  Want this.  Think not  *********************
}


											////////////////////////////////////////


void   PolyMemberVar::Initialize_Value(   short  opCode   )
{

	switch(   opCode   )
	{
		case   PolyMemberVar::AVERAGEpm:   Zero_Value();            break;
	
		case   PolyMemberVar::MAXpm:          Zero_Value();              break;

		case   PolyMemberVar::MINpm:           Max_Out_Value();         break;
		
		default:    
			ASSERT( 0 );	
			Zero_Value();    //  lousy solution,  but a good guess
		break;
	}
}


												/////////////////////////////////////////


void	PolyMemberVar::Zero_Value()
{


	if(   m_Sql_C_Type  ==   PolyMemberVar::UNKNOWNtYPEjm    )
	{
		ASSERT( 0 );
		return;
	}


	switch(   m_Sql_C_Type   )
	{
		case   PolyMemberVar::SHORTjm:	     m_FV.shortValue =   (short)0;   	       break;
		
		case   PolyMemberVar::LONGjm:	      m_FV.longValue   =   (long)0;	             break;

		case   PolyMemberVar::DOUBLEjm:     m_FV.doubleValue =  (double)0.0;          break;

	
		default:    ASSERT( 0 );			break;
	}
}



												/////////////////////////////////////////


void	PolyMemberVar::Max_Out_Value()
{

						//  Initialized its value to a very large number so a MINIMUM test can be done 


	if(    m_Sql_C_Type   ==   PolyMemberVar::UNKNOWNtYPEjm    )
	{
		ASSERT( 0 );
		return;
	}


	switch(   m_Sql_C_Type   )
	{

		case   PolyMemberVar::SHORTjm:

			m_FV.shortValue    =  (short)32760;    //  32767 is really it
		break;

		
		case  PolyMemberVar::LONGjm:

			m_FV.longValue    =  (long)2147483600;    //  is really it   2147483647
		break;


		case  PolyMemberVar::DOUBLEjm:

			m_FV.doubleValue    =  (double)99999999.0;
		break;


		default:    ASSERT( 0 );			
		break;
	}
}


													/////////////////////////////////////////



PolyMemberVar::operator   double()
{

	int   cType =   GetCType();

	double   retVal =  -1.0;
	

	if(      m_memberVarAddress  !=   NULL     )
	{
		ASSERT(  cType  ==   DOUBLEjm   );

		retVal =    *(     (double*)m_memberVarAddress     );
	}
	else
		retVal =      m_FV.doubleValue;


	return  retVal;
}


												/////////////////////////////////////////


PolyMemberVar::operator   short()
{

	int   cType =   GetCType();

	short   retVal =  -1;
	

	if(      m_memberVarAddress  !=   NULL     )
	{
		ASSERT(  cType  ==   SHORTjm   );

		retVal =    *(     (short*)m_memberVarAddress     );
	}
	else
		retVal =      m_FV.shortValue;


	return  retVal;
}



												/////////////////////////////////////////


PolyMemberVar::operator   long()
{

	int   cType =   GetCType();

	long   retVal =  -1;


	if(      m_memberVarAddress  !=   NULL     )
	{
		ASSERT(  cType  ==   LONGjm   );

		retVal =    *(     (long*)m_memberVarAddress     );
	}
	else
		retVal =      m_FV.longValue;


	return  retVal;
}




												/////////////////////////////////////////


short&   PolyMemberVar::operator =(  short shortValue  )
{

	ASSERT(   m_memberVarAddress  ==   NULL     );   //  NEED to write this case too ?????



	ASSERT (  GetCType() == SHORTjm  );

	m_FV.shortValue =   shortValue;

	return   (  short &  )m_FV.shortValue;
}


short&   PolyMemberVar::operator  +=(  short shortValue  )
{

	ASSERT(   m_memberVarAddress  ==   NULL     );   //  NEED to write this case too ?????


	ASSERT (  GetCType() == SHORTjm  );

	m_FV.shortValue   +=   shortValue;

	return   (  short &  )m_FV.shortValue;
}




												/////////////////////////////////////////


long&   PolyMemberVar::operator =  ( long  longValue )
{


	ASSERT(   m_memberVarAddress  ==   NULL     );   //  NEED to write this case too ?????


	ASSERT (  GetCType() == LONGjm  );

	m_FV.longValue =    longValue;

	return    m_FV.longValue;
}




long&   PolyMemberVar::operator +=  ( long  longValue )
{

	ASSERT(   m_memberVarAddress  ==   NULL     );   //  NEED to write this case too ?????


	ASSERT (  GetCType() == LONGjm  );

	m_FV.longValue  +=    longValue;

	return    m_FV.longValue;
}



													/////////////////////////////////////////

double&   PolyMemberVar::operator =  ( double doubleValue )
{

	ASSERT(   m_memberVarAddress  ==   NULL     );   //  NEED to write this case too ?????


	ASSERT ( GetCType() == DOUBLEjm );


	m_FV.doubleValue =     doubleValue;

	return  m_FV.doubleValue;
}


double&   PolyMemberVar::operator +=  ( double doubleValue )
{

	ASSERT(   m_memberVarAddress  ==   NULL     );   //  NEED to write this case too ?????


	ASSERT ( GetCType() == DOUBLEjm );


	m_FV.doubleValue   +=     doubleValue;

	return  m_FV.doubleValue;
}




												/////////////////////////////////////////

PolyMemberVar&   PolyMemberVar::operator =  (  PolyMemberVar  &OtherField   )
{


	ASSERT( 0  );   // ************   NOT USED???  Rewrite for  m_memberVarAddress *********************




	ASSERT(   m_memberVarAddress  ==   NULL     );   //  NEED to write this case too ?????


	ASSERT(   GetCType()  ==  OtherField.GetCType()   );



	/****
	m_strValue =   OtherField.m_strValue;
	if(     GetCType()  ==  I_C_DATETIME    )
	{
	    *(  m_FV.ptimeValue  ) =   (  CTime&  )OtherField;
	}
	else
	****/
	m_FV =   OtherField.m_FV;


	m_memberVarName    =     OtherField.m_memberVarName;


//	m_memberVarAddress =    OtherField.m_memberVarAddress;     NO... will mess up if copy ointers

//  m_sumOfValues      =     OtherField.m_sumOfValues;         ******* NO !!!!!! would fail in   Grab_FundCandids_MemberVars(


	return   *this;
}




												/////////////////////////////////////////


bool   PolyMemberVar::operator  >(  short  value  )
{

	ASSERT(   m_memberVarAddress  ==   NULL     );   //  NEED to write this case too ?????
	ASSERT (  GetCType() == SHORTjm  );

	if(    value  >   m_FV.shortValue   )
		return  true;
	else
		return  false;
}


												/////////////////////////////////////////


bool   PolyMemberVar::operator  >(  long  value  )
{

	ASSERT(   m_memberVarAddress  ==   NULL     );   //  NEED to write this case too ?????
	ASSERT (  GetCType() == SHORTjm  );

	if(    value  >   m_FV.longValue   )
		return  true;
	else
		return  false;
}


												/////////////////////////////////////////


bool   PolyMemberVar::operator  >(  double  value  )
{

	ASSERT(   m_memberVarAddress  ==   NULL     );   //  NEED to write this case too ?????
	ASSERT (  GetCType() == SHORTjm  );

	if(    value  >   m_FV.doubleValue   )
		return  true;
	else
		return  false;
}





												/////////////////////////////////////////
												/////////////////////////////////////////


bool   PolyMemberVar::operator  <(  short  value  )
{

	ASSERT(   m_memberVarAddress  ==   NULL     );   //  NEED to write this case too ?????
	ASSERT (  GetCType() == SHORTjm  );

	if(    value  <   m_FV.shortValue   )
		return  true;
	else
		return  false;
}


												/////////////////////////////////////////


bool   PolyMemberVar::operator  <(  long  value  )
{

	ASSERT(   m_memberVarAddress  ==   NULL     );   //  NEED to write this case too ?????
	ASSERT (  GetCType() == SHORTjm  );

	if(    value  <   m_FV.longValue   )
		return  true;
	else
		return  false;
}


												/////////////////////////////////////////


bool   PolyMemberVar::operator  <(  double  value  )
{

	ASSERT(   m_memberVarAddress  ==   NULL     );   //  NEED to write this case too ?????
	ASSERT (  GetCType() == SHORTjm  );

	if(    value  <   m_FV.doubleValue   )
		return  true;
	else
		return  false;
}

