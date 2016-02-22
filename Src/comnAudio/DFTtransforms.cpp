/////////////////////////////////////////////////////////////////////////////
//
//  DFTtransforms.cpp  -  calculate a Discrete Fourier Transform (DFT) that has its frequency rows spaced logarithmically (corresponding to location of musical harmonics) 
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


#include  <math.h>       //  for trig functions

#include  <mmsystem.h>   //  ( ProgramFiles\Microsoft Visual Studio\VC\PlatformSDK\Include )



#include  "..\comnFacade\VoxAppsGlobals.h"


//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"      //  #include <FixMath.h>    ...Mac's version of Fixedpoint 
#include  "..\ComnGrafix\CommonGrafix.h"      //  #include "commonmy.h"   		

#include  "..\ComnGrafix\OffMap.h"  
#include  "..\comnGrafix\TransformMap.h"

#include  "..\comnAudio\FundamentalCandidate.h"

#include  "..\comnGrafix\mapFilters.h"
//////////////////////////////////////////////////     



#include  "DFTtransforms.h"   

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


extern   long   Get_InputOne();		//  Easy to get these  TEMP values with 'extern'  declare
extern   long   Get_InputTwo();
extern   long   Get_InputThree();




/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


double   noteFreq[   noteFreqArrayTOTALCOUNT   ]  =      // so far, only 5 octaves( is accuracy good enough???
{

		// ****** IF   CHANGE  array,  make corrections to :  Get_Hertz_Freq_from_MidiNumber() ********



//    E                F               F#            G               G#            A     
	  82.41,        87.31,       92.5,        98.0,        103.83,      110.00,	 			// 	 82:    Lowest E on  BASS   ( Midi  40 )


//    A#              B              C              C#             D              D#   
     116.5,        123.5,         130.8,        138.6,        146.8,       155.6,




//		0				1				  2				  3				   4				5		...ScalePitch vals

 //    E                F               F#            G               G#            A     
	164.81,      174.61,      185.00,      196.00,      207.66,      220.00,	 	// 	164:    Lowest E on Guitar   ( Midi  52 )
     

//		6				7				8				9				10				11	

 //    A#              B              C              C#             D              D#   
     233.08,      246.94,      261.63,      277.18,      293.66,      311.13,




	
 //    E                F               F#            G               G#            A     
     329.63,      349.23,      369.99,      392.00,      415.30,      440.00,     // 	329:  E ( 2rd fret,  D string )  ( Midi  64 )
     
//    A#              B              C              C#             D              D#   
     466.16,      493.88,      523.25,      554.37,      587.33,      622.25,




 //    E                F               F#            G               G#            A     
     659.26,      698.46,      739.99,      783.99,      830.61,      880.00,       //  659:   E ( 6th string open )   ( Midi  76 )

//    A#              B              C              C#             D              D#   
     932.33,      987.77,      1046.5,      1108.7,      1174.7,      1244.5,



 //    E                F               F#            G               G#            A     
     1318.5,      1396.9,      1480.0,      1568.0,      1661.2,      1760.0,      //  1318:  E (  12th Fret,  6th string  )  ( Midi  88 )

//    A#              B              C              C#             D              D#   
     1864.7,      1975.5,      2093.0,      2217.5,      2349.3,      2489.0,     //        (  2349 :     LAST note  on Guitar[ d ]   )




 //    E                F               F#            G               G#            A     
     2637.0,      2793.8,      2960.0,      3136.0,      3322.4,      3520.0,    //   Midi  100
//    A#              B              C              C#             D              D#   
     3729.3,      3951.0,      4186.0,      4434.9,      4698.6,      4978.0,





 //    E                F               F#            G               G#            A     
     5274.0,      5587.6,      5920.0,      6272.0,      6644.8,      7040.0,    //   Midi  112   //  DOUBLE chek these figures !!!!    2/02

//    A#              B              C              C#             D              D#   
     7458.6,      7902.0,      8372.0,      8869.8,      9397.2,      9956.0,




 //    E                 F               F#             G( last DFT )         G#             A     
     10548.0,    11175.2,     11840.0,    12544.0,               13289.6,     14080.0,    //   10548.0    Midi  124       //  DOUBLE chek these figures !!!!    6/02

//    A#              B                C              C#              D               D#   
     14917.2,    15804.0,     16744.0,     17739.6,    18794.4,     19912.0


  };  





short  scalFreqs[ 72 ] =                 //  36  for FASTER searches     
  {  
        330,         349,         370,         392,         415,         440,   
        466,         494,         523,         554,         587,         622,   
      
        659,         698,         740,         784,         831,         880,   
        932,         988,        1046,        1109,        1175,        1245,   
    
       1319,        1397,        1480,        1568,        1661,        1760,   
       1865,        1976,        2093,        2217,        2349,        2489,


       2637,      2793,      2960,      3136,      3322,      3520,		//   Midi  100
       3729,      3951,      4186,      4434,      4698,      4978,


     5274,      5587,      5920,      6272,      6644,      7040,		//   Midi  112   
     7458,      7902,      8372,      8869,      9397,      9956,


      10548,    11175,     11840,    1254,     13289,     14080,    //   Midi  124    
      14917,    15804,     16744,     17739,    18794,     19912
  };



/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

double    Get_Hertz_Freq_from_MidiNumber(  short  midiNum  );

short    Find_Freqs_ScalePitch(  short  freq  );

short    Get_ScalePitch_4MidiNumber(  short  midiNum  );
short    Get_OctaveCode_4MidiNumber(  short  midiNum  );   // new

void     Get_ScalePitch_LetterName(  short  sclPitch,   char *firLet,  char *secLet  );


void     Get_ScalePitchs_Color_GLB(    short  scalePitchIdx,     short  keyInSPitch,   
														short&  retRed,      short&  retGreen,   short&  retBlue    );   //  New ugly global    3/10  

short     Get_noteFreq_Array_Count();     //   the double array,  noteFreq[],    in  DFTtransforms.cpp
short     Get_noteFreq_Array_FirstMidi();




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
     

logDFTtrForm::logDFTtrForm(   long  width,   long chunkSize,   short usePhaseData,   long horzScaleDown   ) 
                                          :  TransformMap(  width,   kTOTALdftBANDs,  8,   horzScaleDown ),     _timeWidth( chunkSize )   
{			                                      	       
       

	m_sharedHarmonicsBaseMidi =   64;     // **** ????   Is this the same as  FundamentalMap's first midi [  kMIDIsTARTnOTE   ]????  

	m_totalFloatmapPixels =  -1;


	
	m_hasChannels =   false;

	m_channelCount =   kTOTALdftBANDs;


	m_topPad           =   LOGdFTmapTOPpAD;   //   0   
	


	m_startBandMidi    =    kMIDIsTARTbANDdft;    //  52 (   E,  164, lowest E on guitar )    NEW,  use it


    short  startBand =   kMIDIsTARTbANDdft  - 52;    // **** 'kMIDIsTARTbANDdft' can NOT go below  '52'  without fixing  'noteFreq[]' ...create a Functio, and do away with that funky array  5/07      
//   startBand  =  0 ...WHY bother ????
	     

           //  startBand WAS 7,  for   kMIDIsTARTbANDdft = 59         
                   
                    //  noteFreq[ 19 ] =  B [ 493 hz ]

                   //  if CHANGE  19,    ...CORRECT   'kMIDIsTARTbANDdft'  (71)   
                   //                                         '71'   is  B  [  493 hz ]    
    


								 //   for DIB bitmap( MSoft ),  the image is UPSIDE DOWN( but will invert on blit to display ).  


	for(    int y= ( m_height -1);     y >=  0;      y--   )     //  do in DESCENDING ORDER so inversion is not apparent   ( 36 rows 
	{  					          


		short  yInvert  =   (m_height -1)  -  y;     //  this makes the image 'seem' right side up( from Ycoords point of view )
         


//		int      noteFreqIdx  =    startBand  + y;   //   startBand is 0 ...WHY bother here ????


//		_Freq[  yInvert  ] =     noteFreq[   noteFreqIdx   ];         //  54 rows,   Piszczalski  was just  500Hz to 3000 Hz 

		_Freq[ y ]         =     noteFreq[   yInvert    +  startBand    ];        //  assign value to member array       




		short   pitchAbs  =    Ycoord_2Pitch(  y  );               // just to test 
		long    yRecip    =    Pitch_2Ycoord(  pitchAbs  );        // just to test 



		short   scalePitch =     Find_Freqs_ScalePitch(      (short)(  _Freq[ y ]   )      );

		char   firLet,  secLet;
		Get_ScalePitch_LetterName(   scalePitch,    &firLet,  &secLet   );


//	  TRACE(   "DFT:   Y= %d [ %d ]    [ yInvert  %d ] :     ScalePitch=  %d[ %d ],    Freq = %d     %c%c    \n",       		  
//						         y,  yRecip,  yInvert,   scalePitch,   pitchAbs,    (short)(  _Freq[ y ]  ),    firLet,  secLet   );

		int  dummyBreak = 9;
	}



    
   	m_horzScaleDown =    horzScaleDown; 

     
									// **** OK ????   
     if(  usePhaseData  )            
               _phaseMap =     new    TransformMap(  width,  m_height,  8,  horzScaleDown  );    
     else      _phaseMap =   NULL;
}




											////////////////////////////////////////


logDFTtrForm::~logDFTtrForm()
{												       // release acessory memory 

     if(    _phaseMap   !=   NULL    )
     {  
		 delete   _phaseMap;     
		 _phaseMap = NULL; 
	 }
}



											////////////////////////////////////////


long    logDFTtrForm::Pitch_2Ycoord(   short   midiPitch   )
{             

     short  note,   y = SHDELIN;    // in case fail OUT-of-BOUNDs ???

     												 
     if((           midiPitch   >=   kMIDIsTARTbANDdft   )
		   && (   midiPitch   <      kMIDIsTARTbANDdft    +  m_height     ))
     {
    
         note =    midiPitch    -  kMIDIsTARTbANDdft;       
      
         y     =    ( m_height  -1 )   -   note;       //  says that the DIB is actuall upside down  
     }
                                 
     return  y;       // ****** sends 'SHDELIN'  if  OUT-of-BOUNDs ******
}



											////////////////////////////////////////


short   logDFTtrForm::Ycoord_2Pitch(  long  y  )  
{             
																						

     short   midiPitch;    // ***MUST SYNC with  'Pitch_2Ycoord()'  *******

     
	 midiPitch  =    (  ( m_height -1 )    -  y  )      +    kMIDIsTARTbANDdft;    //   kMIDIsTARTnOTE = 64    'midi 64'  is  329 Hz

     return   midiPitch;
}


  
 											////////////////////////////////////////

 
void   logDFTtrForm::Get_Columns_Biggest_FreqComponents(   long x,   short  compFreq[],    short  compAmpl[],   
																						     short  numComponents,   bool  harmonicMasked,
																							 short  filterCode,   short  kernalWidthHorz    )
{
		//    Only CALLED FROM :      HarmPairsVerter::Transform()


		//   *** This is a very straight forward  BITMAP FUNCTION...   consider moving it to  'OffMap'  or  'TransformMap'    9/2012  ***

                                                      
  
     short   y, i,j,  smallest, smalIdx, bandsFreq,   val = -1,   gr,bl;    // 'compFreq,  compAmpl'  are arrays to return results in 

     smallest= -1;      smalIdx= 0;                   // INITialize

     for(   i=0;    i< numComponents;    i++   )    
	 {  
		 compFreq[i]  =  -1;   
		 compAmpl[i] =  -1; 
	 }



        
     Lock_Bits();					 //  find 4 BIGGEST 'freq COMPONENTs'
     

     for(   y =0;    y < m_height;      y++   )       
      {  


//		if(    hiResRatioLogDFT  ==   1   )
//		{                                                   
			 if(    harmonicMasked    )      //   DEFAULT,  the way I have long been doing this.   9/2012

			 {
				 ASSERT( 0 );   //  do I want this ???

				 Read_Pixel_Masked(   x, y,     filterCode,  kernalWidthHorz,     &val,  &gr,&bl   );    //  masked by  Harmonic-Filter( was PitchEl-filter )
			 }
			 else
			 {  
				 if(    filterCode  ==   ConvolutionMapFilter::NONE    )
					Read_Pixel(  x, y,   &val,  &gr,&bl  );
				 else
					Read_Pixel_Filtered_Horz(  x, y,    filterCode,  kernalWidthHorz,     &val,  &gr,&bl  );
			 }


//		}

		/********   Can acomplish the SAME thing by just having the calling function  ADJUST  'kernalWidthHorz'   whatever way it wants   9/2/2012

		else
		{			//   HI-RES      [    hiResRatioLogDFT >  1    ...so must use diff corrdinates for oversized logDFT   9/12 


//			long     xAdj     =    x           *  hiResRatioLogDFT;
			long     xAdj     =   x;          //  has already beem multiplied by   hiResRatioLogDFT


			short    kernalWidthAdj  =    kernalWidthHorz   *  hiResRatioLogDFT;


			if(    filterCode  ==   ConvolutionMapFilter::NONE    )
			{
				Read_Pixel(   xAdj,  y,     &val,  &gr,&bl   );
			}
			else
				Read_Pixel_Filtered_Horz(    xAdj,  y,       filterCode,   kernalWidthAdj,     &val,  &gr,&bl  );
		}
		****/




          if(   val  >  smallest   )   // Is this new value greater than the smallest existing component( save from previous search )
          {  

               bandsFreq =    (short)(   _Freq[ y ]   );      //   ****???  Is  _Freq   OK for windows ???    *******  JPM

               compAmpl[  smalIdx  ] =   val; 
               compFreq[   smalIdx  ] =   bandsFreq;

		//    I double-checked this logic with the TRACE dumps.     1/2012
		//
		//	   	TRACE(   "\n %d   [ %d, %d ],     [ %d, %d ],     [ %d, %d ],     [ %d, %d ]       [ %d, %d ]  \n" ,     
		//			y, 	compAmpl[0], compFreq[0],     compAmpl[1], compFreq[1],      compAmpl[2], compFreq[2],      compAmpl[3], compFreq[3],      compAmpl[4], compFreq[4]    );

                 				

               smallest =  29000;      // init for search to find the   "NEW smallest Component "  in the Array

               for(   j=0;   j< numComponents;   j++   )    //  
               {        
				   if(   compAmpl[ j ]   <   smallest   )  
				   { 
					   smallest  =  compAmpl[ j ];  
					   smalIdx  =  j; 
				   }     
			   }

           }   //    if(   val  >  smallest   ) 
            
       }    //    for(   y= 0;    y< m_height
      



//	 TRACE(   "\n -------- \n"  );

//	 TRACE(   "\n %d   [ %d, %d ],     [ %d, %d ],     [ %d, %d ],     [ %d, %d ]       [ %d, %d ]  \n" ,     
//					y, 	compAmpl[0], compFreq[0],     compAmpl[1], compFreq[1],      compAmpl[2], compFreq[2],      compAmpl[3], compFreq[3],      compAmpl[4], compFreq[4]    );

//	  TRACE(  "\n\n ******  \n\n"  );

     Unlock_Bits();    
}



 
 											////////////////////////////////////////


short     logDFTtrForm::Make_SharedHarmonics_List(   short scalePitch0,  short scalePitch1,    short  sharedHarmonicsBaseMidi,
																												short  retShareHarms[]   )  
{  
																				//   ex:    E       A

ASSERT( 0 );      //    Not used anybmore.     Reuse it in future ???    2/2004



	short   sharedCount = 0;

	short      totHarms =   TOTALhARMONICS;    // 7...   
    short      templOffst[ 8 ] =  {   0,    12,    19,    24,      28,     31,     34,                36   };  // only really need seven
//   EX:  in E						         E      E       B      E        G#      B      D( bad fit )      E

//	        Harm Ratio:	                        1:2    1:3    1:4	   1:5     1:6    1:7                1:8



	bool    foundSharedHarm[ 12 ];    //  the 12 buckets represent the 12 ScalePitches



//	short     fundamentalMapFirstMidi    =    TransMapAdmin::Get_Maps_First_Midi(   TransMapAdmin::FUNDAMENTALTEMPLATE  );   //   64;   //    kMIDIsTARTnOTE;   //  64        //  **** HARDWIRED  ***************** 
//	short     sharedHarmonicsBaseMidi  =    m_sharedHarmonicsBaseMidi;     //    TransMapAdmin::Get_Maps_First_Midi(   TransMapAdmin::FUNDAMENTALTEMPLATE  );   


	short   midiStart0  =    sharedHarmonicsBaseMidi     +  scalePitch0;        //  64  is a   midi E    ***** 64:  HARDWIRED ******
	short   midiStart1  =    sharedHarmonicsBaseMidi     +  scalePitch1;




	for(   short i=0;    i< 12;    i++    )			//  init
		foundSharedHarm[ i ] =   false;




	for(    short  harm0 =0;      harm0 <  totHarms;       harm0++    )    //  for all of E's  Harmonics
	{

		short  curHarmsMidi0       =      midiStart0   +   templOffst[ harm0 ]; 

		short  curHarmsScalePitch0 =      Get_ScalePitch_4MidiNumber(  curHarmsMidi0  );



		for(    short  harm1 =0;      harm1 <  totHarms;       harm1++    )							   //  for all of A's  Harmonics
		{

			short  curHarmsMidi1          =      midiStart1   +   templOffst[ harm1 ]; 
			short  curHarmsScalePitch1 =      Get_ScalePitch_4MidiNumber(  curHarmsMidi1  );


			if(    curHarmsScalePitch1   ==   curHarmsScalePitch0    )
				foundSharedHarm[  curHarmsScalePitch1  ]  =    true;
		}
	}



	sharedCount = 0;

	for(   short scalePitch=0;     scalePitch< 12;     scalePitch++   )
	{

		if(    foundSharedHarm[ scalePitch ]  ==   true    )
		{
			retShareHarms[  sharedCount ]  =    scalePitch; 
			sharedCount++;  
		}
	}

	return  sharedCount;  
}



											////////////////////////////////////////


bool	 logDFTtrForm::Sum_All_Harmonics_Magnitudes_inSpan(   long  x0,   long  x1,    short  midiPitch,    short  numHarms,
																					               FundamentalCandidate&  fundCandid,
																									short&  retMissedBandCnt,   CString&  retErrorMesg   )
{

//   CALLED  by    ScalepitchSubject::Read_HarmonicMags_from_logDFT_OctaveGroup()    ...for   HarmonicProfile-DIALOG  &  Octave-Correcton Algos[  Calc_Best_Octave_Candidate()   ]
//				and    FundCandidEvaluator::Read_Harmonic_Mags_from_logDFT( 



		//  x0,  x1 :   are in VirtualWORLD coords,  OFFSETTING for  ShortDFTs is done automatically below.  3/03



//	short   octaveOffset  =   0;    //   ****???? ADJUST ******     Make all reads  this  many octave offsetted
												//   Weird, but I always offset down one octave  -12  ]   ************************


	retMissedBandCnt =  0;
	short   maxHarmCount =  FundamentalCandidate::Harmonic_Count_Extra();   //  18   see  




//	short      templOffst[ 12 ]=  {   0,       12,       19,    24,         28,    31,    34,    36,         38,     40,     42,     43    };    // only really need seven
//   EX:  in E					     E          E         B      E          G#     B       D      E          F#      G#      A#       B

//   2nd  'E' is the fundamental:          Fund( 12 )


	short      templOffst[ 18 ]=  {   0,       12,       19,    24,        28,    31,    34,    36,        
                                                                38,     40,     42,     43,         44,        46,      47,       48,      49,      50 };    
//								    B            C	        D        D#        E         F         F#							
//                                         { C or C# }  ...a sloppy fit


	retErrorMesg.Empty();



	long   x0work = -1,    x1work = -1;


	if(     Needs_Offsetting()     )		//  BIG!!!  
	{

	    x0work  =     x0   -   m_pixelReadOffset;
		x1work  =     x1   -   m_pixelReadOffset;

		if(       (   x0work  < 0     ||   x0work  >=  m_width     )   
			||    (   x1work  < 0     ||   x1work  >=  m_width     )     )
		{
			retErrorMesg =  "Sum_All_Harmonics_Magnitudes_inSpan failed,  offsetting went out of bounds." ;
			return  false;
		}
	}
	else
	{  x0work  =    x0;     //   NAVIGATOR goes here when calcing OCTAVES.     9/2012
		x1work  =    x1;
	}




	if(   numHarms  >  maxHarmCount   )
	{
		retErrorMesg =   "Sum_All_Harmonics_Magnitudes_inSpan  failed,  input too large a harmonic count." ; 
		return  false;
	}



	for(    short  i=0;     i <  numHarms;     i++    )	 //   init  for FAIL if not calc data 
		fundCandid.Set_Hamonics_Mag(    i,    INITIALIZEhMAG    );





	long  pixelCount     =      ( x1   -  x0 )   +1;     //  inclusive counting
	if(     pixelCount  <=  0  )
	{
		retErrorMesg =   "Sum_All_Harmonics_Magnitudes_inSpan  failed,  input zero pixels," ;
		return  false;
	}




	for(   short  harm= 0;      harm <  numHarms;       harm++    )                  //  for all Harmonics
	{
              
		short   bandMidi;
		long    totalSum =  0L,  yDFT; 
              

		short   downwardReadOffset  =     logDFTdOWNWARDrEAD;     //   12     **** BIG !!!! ****



// ***** WANT this offsetting downward ???? *************************************************************
              
	//	bandMidi =    midiPitch   +   templOffst[ harm ]    - 12;         // get band index for this Harmonic
                                                                          // -12: **** shift 'read of DFTmap' DOWN an octave.  Why I don't know.   
             
		bandMidi =    midiPitch   +   templOffst[ harm ]    -   downwardReadOffset;   

// ***** WANT this offsetting downward ???? *************************************************************

              


		yDFT         =           Pitch_2Ycoord(   bandMidi   );       // returns  'SHDELIN'  for OUT of BOUNDs ****              
		if(  yDFT != SHDELIN  )                  
		{                  

			for(     long  x =  x0work;        x <=   x1work;         x++    )
			{
				short  val, gr, bl;
				Read_Pixel(    x,   yDFT,      &val,  &gr,  &bl   );                   

				totalSum  +=    val;
			}

			fundCandid.Set_Hamonics_Mag(    harm,    (short)(  totalSum / pixelCount  )      );
		}
		else
		{  fundCandid.Set_Hamonics_Mag(    harm,    UNDEFINEDhMAG      );   //  -9

			retMissedBandCnt++;
		}



		fundCandid.m_missedHarmonicBandCnt  =    retMissedBandCnt;
	}  


	return  true;
}




											////////////////////////////////////////


bool	 logDFTtrForm::Calc_Average_PixelMagnitude_inSpan(   long  x0,   long  x1,    short  midiPitch,    long&  retAvgMag,  
																				             bool&  retOffMap,    CString&  retErrorMesg   )  
{
	
	long    yDFT;
	long     totalSum =  0L,   pixelCountActual=0; 
	short    val, gr, bl;

              

	retOffMap  =    false;
	retAvgMag  =  0;
	retErrorMesg.Empty();             



	long   x0work = -1,    x1work = -1;


	if(     Needs_Offsetting()     )		//  BIG!!!  
	{
	    x0work  =     x0   -   m_pixelReadOffset;
		x1work  =     x1   -   m_pixelReadOffset;

		if(       (   x0work  < 0     ||   x0work  >=  m_width     )   
			||    (   x1work  < 0     ||   x1work  >=  m_width     )     )
		{
			retErrorMesg =  "Calc_Average_PixelMagnitude_inSpan failed,  offsetting went out of bounds." ;
			ASSERT( 0 );
			return  false;
		}
	}
	else
	{  x0work  =    x0;   // NAVIGATOR goes here when  during OCTAVE calcs.    9/2012
		x1work  =    x1;
	}



	long  pixelCount     =      ( x1   -  x0 )   +1;     //  inclusive counting
	if(     pixelCount  <=  0  )
	{
		retErrorMesg =   "Calc_Average_PixelMagnitude_inSpan  failed,  input zero pixels," ;
		return  false;
	}





	yDFT         =           Pitch_2Ycoord(   midiPitch   );       // returns  'SHDELIN'  for OUT of BOUNDs ****              
	if(  yDFT == SHDELIN  )              
	{  
		retOffMap =   true;

		return   0;      //  want a bad values ??
	}
		



	for(     long  x =  x0work;        x <=   x1work;         x++    )
	{
				
		Read_Pixel(    x,   yDFT,      &val,  &gr,  &bl   );                   

		totalSum  +=    val;

		pixelCountActual++;
	}

	ASSERT(  pixelCountActual  ==   pixelCount   );

		

		
	retAvgMag =      totalSum  /  pixelCount;


	return  true;
}



											////////////////////////////////////////


bool	 logDFTtrForm::Is_HarmonicBand_On_Map(   short  fundamentalPitch,   short  harmonicIdx  )
{

					//  Performs the automatic  DOWNWARD-READ of an octave

	

	short   downwardReadOffset  =     logDFTdOWNWARDrEAD;     //   12     **** BIG !!!! ****


	short     templOffst[ 12 ]=  {   0,       12,       19,    24,        28,    31,    34,    36,        38,     40,     42,     43    };    // only really need seven
//   EX:  in E							     E          E         B      E          G#     B       D      E          F#     G#      A#      B

//   2nd  'E' is the fundamental:          Fund( 12 )



	if(     harmonicIdx  >=   FundamentalCandidate::Harmonic_Count()    )
	{ 
		ASSERT( 0 );
		return  false;
	}




// ***** WANT this offsetting downward ???? *************************************************************
              
	//	bandMidi =    midiPitch   +   templOffst[ harm ]    -12;         // get band index for this Harmonic
                                                                             // -12: **** shift 'read of DFTmap' DOWN an octave.  Why I don't know.   
             
	short	bandMidi =    fundamentalPitch   +   templOffst[ harmonicIdx ]    -   downwardReadOffset;   

// ***** WANT this offsetting downward ???? *************************************************************

              



	long	yDFT    =       Pitch_2Ycoord(   bandMidi   );       // returns  'SHDELIN'  for OUT of BOUNDs ****              
	
	if(    yDFT  !=   SHDELIN    )   
		return  true;
	else
		return  false;		        
}


									                               
											////////////////////////////////////////


bool	 logDFTtrForm::Is_MidiPitch_On_Map(   short  midiPitch   )
{

					//  Performs NO  DOWNWARD-READ

	if(   midiPitch  <=  0   )
		return   false;
	

	long	yDFT    =       Pitch_2Ycoord(   midiPitch   );       // returns  'SHDELIN'  for OUT of BOUNDs ****              
	
	if(    yDFT  !=   SHDELIN    )   
		return  true;
	else
		return  false;		        
}



											////////////////////////////////////////


long   logDFTtrForm::Get_FundamentalTemplate_Score(  short  midiPitch,   long x,   short  harmMags[],   short  harmonicThreshold, 
																				  short&   retHarmonicCount,  short  filterCode,   short  kernalWidthHorz      )
{


	ASSERT(  0 );    // ***** NOT used   3/04  ???  ***********************

	// **** If ReINSTALL,  get new  Harmonic array from  FundamentalTemplateVerter::Transform_Column()   *****





	//  Adds all  'PixelMagnitudes' of any in HarmonicInteger ratios to the input Fundamental( midiPitch  ) to make 'score' 


				//  Called by    PitchelVerter::Transform_Column() 


     short   totHarms =  7;      // 7:  TOTALhARMONICS   
	 


     short      templOffst[ 8 ] =  {   0,         12,         19,         24,           28,          31,         34,                36   };  // only really need seven
//   EX:  in E							      E          E            B            E            G#           B       D( bad fit )          E


     long   adjWeits[ 7 ]=       {    100,       61,         61,         39,            61,         26,          61                   };    // by  Anssi Klapuri  on interfering signal OVERLAP probabatilities 


	/***
     long   adjWeits[ 7 ] =      {      50,      100,       100,        100,          75,          30,         10          }; 		

     long   adjWeits[ 7 ] =      {    100,        97,         94,          91,          89,          86,         83          };   // .97 reduction each time, like  'Piszczalski's  article'
	***/          





     short   harm,  bandMidi,    val,gr,bl,   harmFndCnt=0;
     long    retScore= 0L,   yDFT,    adjVal;
     
//    long    scoreThresh  =   200L;     //  ***** ADJUST *****
//	 short    hrmFndThresh =    40;    //  20,  64,  32    *** ADJUST ***



//  short   goodHarmCnt  =     2;    //  2   *** NOT USED ??? ***      
										//  goodHarmCnt= 3,  '3' is too DEMANDING, many times are just two harmonics
                                        //  goodHarmCnt= 2   insures that at least  ONE PAIR of (integerRelated)Harmonics are found [ no SINGLE component writes ]
    
 
	retHarmonicCount =  0;

	Lock_Bits();	


	for(    harm= 0;     harm <  totHarms;      harm++    )                  //  for all Harmonics
	{
              
		harmMags[ harm ] = 0;    
		val  = 0;					   // init, in case  'not read'  [  Pitch_2Ycoord() return of 'SHDELIN'  ] 
              
              
		bandMidi =    midiPitch  +  templOffst[ harm ]   -  logDFTdOWNWARDrEAD;         // get band index for this Harmonic
                                                                         // -      12:     Shift  'read of DFTmap'  DOWN an octave for ''Western Pick of Fundamental Octave'
             
              
		yDFT    =      Pitch_2Ycoord(   bandMidi   );       // returns 'SHDELIN' for OUT of BOUNDs ****              
		if(   yDFT  !=  SHDELIN   )                  
		{                  

		//	Read_Pixel(               x,  yDFT,    &val,  &gr,  &bl   );  
			Read_Pixel_Masked(   x,  yDFT,    filterCode,  kernalWidthHorz,    &val,  &gr,  &bl   );  
                 

			if(    val  >   harmonicThreshold    )                
				retHarmonicCount++;            // Count SUBSTANTIAL harmonics FOUND            

			
			harmMags[ harm ] =    val;     //    (RETURN VALUE)  *** NO WEIGHTING( later used for reconstruction )*** 
                  
                                    
			adjVal   =     (   (long)val  *  adjWeits[ harm ]   )    / 100L;    //  'adjWeits[]'  is in hundredth's ( do NOT change 100L )                   
                  
			retScore  +=     adjVal;    //  We add ALL pixels(even small mags),  but only COUNT( retHarmonicCount )  if significant( above threshold )                                                     
		}
	}  


  
//	 long   subWeit= -70L;  	***** did NOT work well......									                     
//    subWeit =   inpThree;                        

//     bandMidi =   midiPitch  +  ( -12 )   -12;     //  ( -12 )  is  'SUB-harmonic'
 //    yDFT =      Pitch_2Ycoord( bandMidi );   // returns 'SHDELIN' for OUT of BOUNDs ****
//     if(  yDFT  !=  SHDELIN  )  
//        {  Read_Pixel(  x,  yDFT,    &val,  &gr,&bl  );  
//           adjVal   =  (  (long)val *   subWeit  ) / 100L; 
//           retScore   +=    adjVal;
//       }   
     Unlock_Bits();	
     
     return   retScore; 
}



											////////////////////////////////////////


long   logDFTtrForm::Sum_All_Octaves_ScalePitch_Mags_inSpan(   short  midiPitch,      long x0,   long x1,   
																								 short  filterCode,   short  kernalWidth,     CString&  retErrorMesg  )   
{


	//  EX:   for E( sPitch = 0),   sums magnitudes of  ALL E's   in different octaves of DFT.   OK  if(  x0 = x1 ) for single Y-Column


	//					  NEW,   CALLED by   Get_HarmonicJudgement_Data()		3/04



//	short  filterCode     =    ConvolutionMapFilter::BLURHORZ;
//	short  kernalWidth  =    3;     



	retErrorMesg.Empty();


	long  pixelDistance  =    x1  -  x0;         //   Distance of  ZERO is OK,   just 1 pixel in run

	if(     pixelDistance  < 0  )
	{
		retErrorMesg =   "Sum_All_Octaves_ScalePitch_Mags_inSpan  failed,  input zero pixels." ;
		return  -1;
	}



	long   x0work = -1,    x1work = -1;
	short   logDFTheight  =   kTOTALdftBANDs;

	short	numOctavesDFT  =      logDFTheight  /  12;

	if(    ( logDFTheight  %  12 )    !=   0     )
		numOctavesDFT++;





	if(     Needs_Offsetting()     )		//  BIG!!!  
	{

	    x0work  =     x0   -   m_pixelReadOffset;
		x1work  =     x1   -   m_pixelReadOffset;

		if(       (   x0work  < 0     ||   x0work  >=  m_width     )   
			||    (   x1work  < 0     ||   x1work  >=  m_width     )     )
		{
			retErrorMesg =  "Sum_All_Octaves_ScalePitch_Mags_inSpan  failed,  horizontal offsetting went out of bounds." ;
			return  -2;
		}
	}
	else
	{  x0work  =    x0;
	   x1work  =    x1;
	}




	long   totalSum =0,    totalPixels=0;


	for(    short  oct =0;       oct <   numOctavesDFT;       oct++    )                 
	{
              
		short   bandMidi;
		long     yDFT;
              
             
		bandMidi  =       kMIDIsTARTbANDdft     +    midiPitch    +     ( oct  *  12 );   

              

		yDFT         =           Pitch_2Ycoord(   bandMidi   );       // returns  'SHDELIN'  for OUT of BOUNDs ****              
		if(  yDFT != SHDELIN  )                  
		{                  

			for(    long  x =  x0work;       x  <=   x1work;        x++    )
			{
				short  val, gr, bl;


//				Read_Pixel(                        x,   yDFT,                                                  &val,  &gr,  &bl   );           				
				Read_Pixel_Filtered_Horz(    x,   yDFT,       filterCode,    kernalWidth,      &val,  &gr,  &bl   );       


				totalSum  +=    val;
				totalPixels++;
			}
		}
	}  



	long      retAverageMag   =       totalSum   /  totalPixels;   // **** WANT  to AMPLIFY this score ??? ****

	if(    retAverageMag   >  256   )
		ASSERT( 0 );


	return   retAverageMag;
}





											////////////////////////////////////////


long   logDFTtrForm::Get_ScalepitchTemplate_Score(  short  scalePitch,   long x,   short  harmMags[],    short   totHarms, 
																			   short  harmonicThreshold, 	short&   retHarmonicCount,    
																			   long&  retRawScore,	  short  filterCode,    short  kernalWidthHorz    )
{


	ASSERT( 0 );     // ***** Not used anymore.  If I wanted to use it, should extend the template to Midi 76    [  7/06



			//  Adds all  'PixelMagnitudes' of any Harmonics for the input scalePitch    ...but for ALL Octaves.  


    //  short   totHarms =  15; ...safer to pass in.  Calling function is more aware of the size of  input 'harmMags[]'
	 


     short      templOffst[ 15 ] =  
		{   0,       12,       19,       24,       28,       31,       34,        36,        40,        43,       46,       48,       52,       55,       58     };  // to 76   !!!!
//           E        E          B          E        G#        B          D          E          G#         B         D         E         G#        B         D
  

     long   adjWeits[ 15 ]=  
		{ 100,     161,      61,      200,      61,       87,       61,       200,       61,        87,      61,       100,       61,       87,      61    };    // by  Anssi Klapuri  on interfering signal OVERLAP probabatilities 



			//  The weights are the cumulative sums of Klapuri weights,  but for ALL  Fundamental possibilties  5/02





//  *******  INSTALL:    Extend these to TOP of  DFT map    kTOTALdftBANDs   76    *****************

//  ******* INSTALL:    Should really  AVERAGE the score   by number of Harms, so I change the SIZE of template and not



    short   harm,   bandMidi,    val,gr,bl,    harmFndCnt=  0;
    long    yDFT,   adjVal;
    long    retWeightedScore =   0L;   

	short   dftStartMidi =		kMIDIsTARTbANDdft;			 //    SHOULD   this offset down by 12?   That  is where I start to look
																				 //    NO !!!     because ReadPixel() wil just return -1 for 'off of map'  values.




	retHarmonicCount  =  0;
	retRawScore  =    0;


	ASSERT(   totHarms  <=  15   );   //  ***  15 is the majic number ****





	for(    harm= 0;     harm<  totHarms;      harm++    )                  //  for all Harmonics
	{
              
		harmMags[ harm ] = 0;    
		val  = 0;					   // init, in case  'not read'  [  Pitch_2Ycoord() return of 'SHDELIN'  ] 
              	
              
		bandMidi =    dftStartMidi   +  scalePitch   +   templOffst[ harm ];    //  Always start in the FIRST octave of DFT    
		

              
		yDFT    =      Pitch_2Ycoord(   bandMidi   );     // returns 'SHDELIN' for OUT of BOUNDs( off the freq-range of the DFT )           
		if(  yDFT  !=  SHDELIN  )                  
		{                  

			Read_Pixel_Masked(   x,  yDFT,     filterCode,  kernalWidthHorz,    &val,  &gr,  &bl   );  

			if(    val  >   harmonicThreshold    )                
				retHarmonicCount++;            // Count SUBSTANTIAL harmonics FOUND            

			
			harmMags[ harm ] =    val;     //    (RETURN VALUE)  *** NO WEIGHTING( later used for reconstruction )*** 
                  
                                    
			adjVal   =     (   (long)val  *  adjWeits[ harm ]   )    / 100L;    //  'adjWeits[]'  is in hundredth's ( do NOT change 100L )                   
                  

			retWeightedScore  +=    adjVal;    //  We add ALL pixels(even small mags),  but only COUNT( retHarmonicCount )  if significant( above threshold )                                                     

			retRawScore         +=     val;   //  also  sum the raw score,  not sure how effective current weights are  5/02
		}
	}  

     
	return   retWeightedScore; 
}




											////////////////////////////////////////


bool   logDFTtrForm::Get_HarmonicJudgement_Data(   long x0,   long x1,     short  retFirstFifthScores[],   
																				   short  retThirdSeventhScores[],     CString&  retErrorMesg  )
{

											//    OK for  ( x0 == x1 ),    to do a single Y-Column 


	retErrorMesg.Empty();



	short   filterCode,   kernalWidth;

	if(   x0  ==   x1   )
	{
		filterCode     =      ConvolutionMapFilter::BLURHORZ;      //   In COLUMN-Mode, need smoothing for transform.  [  see  ScalepitchVerter::Transform_Column_Harmonic_Judgement()  
		kernalWidth  =      3;     
	}
	else
	{  filterCode     =      ConvolutionMapFilter::NONE;      //   If analyzing a SPAN of Pixels, do NOT need an AVERAGED( filtered )  read
		kernalWidth  =      0;     
	}






	for(    short  sPitch =0;      sPitch<  12;       sPitch++    )			//   Init in case of fail
	{
		retFirstFifthScores[         sPitch  ]  =     -1;
		retThirdSeventhScores[  sPitch  ]  =     -1;
	}



	for(    short   sPitch =0;      sPitch<  12;       sPitch++   )
	{

		short   firstMidiPitch        =    sPitch;
		short   fifthMidiPitch        =    sPitch    +   7      +  (  1 *  12 )   -   logDFTdOWNWARDrEAD;		//  1:    first  'USED-Fifith'  does not happen till the 2nd OCTAVE
		short   thirdMidiPitch       =    sPitch    +   4      +  (  2 *  12 )   -   logDFTdOWNWARDrEAD;       //  2:    first  'USED-Third'  does not happen till the 3rd OCTAVE
		short   seventhMidiPitch  =    sPitch    +  10     +  (  2 *  12 )   -   logDFTdOWNWARDrEAD;    //   'logDFTdOWNWARDrEAD'  :   Emulate how  Octave-Candidates  'look for'  harmonics



		short  avgMagFirst      =       (short)Sum_All_Octaves_ScalePitch_Mags_inSpan(   firstMidiPitch,     x0,  x1,   
																									                         filterCode,   kernalWidth,	  retErrorMesg  );
		if(      avgMagFirst  < 0  )
			return  false;


		short  avgMagFifth      =       (short)Sum_All_Octaves_ScalePitch_Mags_inSpan(   fifthMidiPitch,     x0,  x1,   
																									                         filterCode,   kernalWidth,	  retErrorMesg  );
		if(      avgMagFifth  < 0  )
			return  false;


		short  avgMagThird     =       (short)Sum_All_Octaves_ScalePitch_Mags_inSpan(   thirdMidiPitch,     x0,  x1,   
																									                         filterCode,   kernalWidth,	  retErrorMesg  );
		if(      avgMagThird  < 0  )
			return  false;


		short  avgMagSeventh =       (short)Sum_All_Octaves_ScalePitch_Mags_inSpan(  seventhMidiPitch,   x0,  x1,   
																									                         filterCode,   kernalWidth,	  retErrorMesg  );
		if(      avgMagSeventh  < 0  )
			return  false;



		/***
		retFirstFifthScores[         sPitch  ]  =      (  avgMagFirst     +    avgMagFifth         )  /2;   // ****   '/2'    So range is in  { 0 - 256 }  ????   ********
		retThirdSeventhScores[  sPitch  ]  =      (  avgMagThird    +    avgMagSeventh   )  /2; 
		***/
		retFirstFifthScores[         sPitch  ]  =        avgMagFirst     +    avgMagFifth;         //  Want accuracy instead ???   3/04
		retThirdSeventhScores[  sPitch  ]  =        avgMagThird    +    avgMagSeventh; 
	}


	return  true;
}



											////////////////////////////////////////


bool    logDFTtrForm::Calc_ScalePitch_Strength_in_Spectrum(   long  x,    short  scalepitchIdx,    long&  retScore,   
																				short  filterCode,   short  kernalWidthHorz,   bool  readDftMasked,    CString&  retErrorMesg   )
{


	//  For the input ScalePitch VALUE,  will sum all the MAGNITUDES of that ScalePitch in different octaves of logDFT



	/**********************************************************************

	 **** ENHANCE:   Create new function, like Calc_ScalePitch_Strength_in_Spectrum(),  but just gets GrossMag 
							of  the HarmonicNumber(  Fund,  Fifth,  MajThird,   FlatThird  )  and  UPWARDLY offsets  the  
							"startMidi"   to realistic value [ ex:  For  pitch E,  G# is a Harmonic,  but it does NOT appear till
							the  2nd Octave  ABOVE the fundamental.   

			PROBLEM with this algo is it uses too many lower bands  that are not realistic withthe HarmonicEnvelope.  2/03

			...however at this time( 2/03 )  this is only used for the FifthHarmonic Test  in  ScalepitchVerter::Transform_Column_Dicrim_Template()
			   which is NOT too critical ???   Or is it ???     
   ***/


	retScore =  0L;


	retErrorMesg.Empty();



	short   totalOctaves =  5;			//  4     Will attempt  4,  but  no problem if goes out of range 



	short   octave,   bandMidi,    val,gr,bl,    harmFndCnt=  0;
	long    yDFT;

	short   dftStartMidi =   kMIDIsTARTbANDdft;



	for(    octave= 0;     octave<  totalOctaves;      octave++    )          
	{
              
              
		bandMidi  =     dftStartMidi   +   scalepitchIdx   +    ( octave * 12  );    //  Always start in the FIRST octave of DFT    
		

              
		yDFT    =      Pitch_2Ycoord(   bandMidi   );     // returns 'SHDELIN' for OUT of BOUNDs( off the freq-range of the DFT )           
		if(  yDFT  !=  SHDELIN  )                  
		{                  

			if(    readDftMasked   )
				Read_Pixel_Masked(           x,  yDFT,      filterCode,  kernalWidthHorz,      &val,  &gr,  &bl    );  
			else
				Read_Pixel_Filtered_Horz(   x,  yDFT,      filterCode,   kernalWidthHorz,      &val,  &gr,  &bl    );


			/***
			if(    val  >   harmonicThreshold    )                
				retHarmonicCount++;            //   only  Count SUBSTANTIAL harmonics FOUND            
			
			harmMags[ octave ] =    val;     //    (RETURN VALUE)  *** NO WEIGHTING( later used for reconstruction )***                   
                                    
			adjVal   =     (   (long)val  *  adjWeits[ octave ]   )    / 100L;    //  'adjWeits[]'  is in hundredth's ( do NOT change 100L )                   
                  
			retWeightedScore  +=    adjVal;    //  We add ALL pixels(even small mags),  but only COUNT( retHarmonicCount )  if significant( above threshold )                                                     
			***/

			retScore   +=     val;   //  also  sum the raw score,  not sure how effective current weights are  5/02
		}
	}  


	return  true;
}




											////////////////////////////////////////


long   logDFTtrForm::Get_ScalepitchTemplate_Score_wExclusions(  short  scalePitch,   long x,    short  sharedHarms[],   
																								short  sharedCount,   long&  retRawScore,
																								short  filterCode,   short  kernalWidthHorz   )
{

						//  Adds all  'PixelMagnitudes' of any Harmonics for the input scalePitch    ...but for ALL Octaves.  

	short   totHarms =  15; 
	 

	short      templOffst[ 15 ] =  
		{   0,       12,       19,       24,       28,       31,       34,        36,        40,        43,       46,       48,       52,       55,       58     };  
//           E        E          B          E        G#        B          D          E          G#         B         D         E         G#        B         D
  
     long   adjWeits[ 15 ]=  
		{ 100,     161,      61,      200,      61,       87,       61,       200,       61,        87,      61,       100,       61,       87,      61  };    // by  Anssi Klapuri  on interfering signal OVERLAP probabatilities 

			//  The weights are the cumulative sums of Klapuri weights,  but for ALL  Fundamental possibilties  5/02



    short   harm,   bandMidi,   val,gr,bl,    harmFndCnt=  0;
	long    yDFT;
    long    adjVal;
	short   dftStartMidi =   kMIDIsTARTbANDdft;
    long    retWeightedScore =   0L;   


	retRawScore =  0;

	if(    sharedCount  <=  0    )      //  Calling funct should not call, if there are NO shared to exclude.
	{
		ASSERT( 0 );
		return 0;
	}




	Lock_Bits();	


	for(    harm= 0;     harm<  totHarms;      harm++    )                  //  for all Harmonics
	{
              
		val  = 0;					   // init, in case  'not read'  [  Pitch_2Ycoord() return of 'SHDELIN'  ] 
              	              
		bandMidi =    dftStartMidi   +  scalePitch   +   templOffst[ harm ];    //  Always start in the FIRST octave of DFT    
		


		short   curHarmsScalePitch =      Get_ScalePitch_4MidiNumber(  bandMidi  );
		bool    skipShared =  false;

		for(   short ex=0;      ex <  sharedCount;      ex++    )    //  see if this harmonics's Scalepitch value is on the Excluded list
		{
			if(   sharedHarms[ ex ]   ==   curHarmsScalePitch    )
			{
				skipShared =   true;
				break;
			}
		}


              
		yDFT =    Pitch_2Ycoord(   bandMidi   );     // returns 'SHDELIN' for OUT of BOUNDs( off the freq-range of the DFT ) 
		
		if(         yDFT  !=  SHDELIN
			&&   !skipShared    )
		{                  

//			Read_Pixel(               x,  yDFT,    &val,  &gr,  &bl   );  
			Read_Pixel_Masked(   x,  yDFT,      filterCode,  kernalWidthHorz,     &val,  &gr,  &bl   );  
                 

	//		if(    val  >   harmonicThreshold    )                
	//			retHarmonicCount++;            // Count SUBSTANTIAL harmonics FOUND            
			
	//		harmMags[ harm ] =    val;     //    (RETURN VALUE)  *** NO WEIGHTING( later used for reconstruction )*** 
                  
                                    
			adjVal   =     (   (long)val  *  adjWeits[ harm ]   )    / 100L;    //  'adjWeits[]'  is in hundredth's ( do NOT change 100L )                   
                  
			retWeightedScore  +=    adjVal;    //  We add ALL pixels(even small mags),  but only COUNT( retHarmonicCount )  if significant( above threshold )                                                     

			retRawScore         +=     val;   //  also  sum the raw score,  not sure how effective current weights are  5/02
		}

	}  

	Unlock_Bits();	
     
	return   retWeightedScore; 
}




											////////////////////////////////////////


long    logDFTtrForm::FindBest_ScalepitchTemplate_Score_2Phase(  long  x,    short&  retScalePitch,   bool  useWeightedScores,
														   bool&  retDidSwap,    short&  retScalepitchSecond,   long&  retScoreSecond    )
{


ASSERT( 0 );     //  NOT USED    8/2002



	short  filterCode  =  0;  // ****** INSTALL if use this function  
	short  kernalWidthHorz  =  0; 


	short    sharedHarmonicsBaseMidi  =  -9999;
//	short    sharedHarmonicsBaseMidi  =   TransMapAdmin::Get_Maps_First_Midi(   TransMapAdmin::FUNDAMENTALTEMPLATE  );   
// ********************** INSTALL if  reuse this function  ******************************




	//  Only called from     ScalepitchVerter::Transform_Column_Ymax_Template()


		//   *** OLD Algo to create the Y-max   8/02  ***************************



	double   firstRatioThresh  =    2.0;     //  { 1.5  -  3.0  }  ***** ADJUST *****

	double   secondRatioThresh =    3.0;     //  { 2.0  -  4.0  }    ********** ADJUST *****

/***
	double   firstRatioThresh   =  (double)Get_InputOne()  /  10.0;    

	double   secondRatioThresh =   (double)Get_InputTwo()  /  10.0;
***/


	/**
		1.5   means  top-Max must be    50 %   greater   to avoid a CONTROVERSY ( double write
		1.3   means						         30 %   greater
		1.2   means						         20 %   greater
	**/


	short   harmonicThreshold =    40;    // ***** ADJUST *****    ..but not really used.   6/02



	long    weitedScore = 0;
	short   totHarms =  15;
	short   harmMags[  15  ];  
	short   retHarmonicCount,      maxScalePitch,  maxScalePitch2;
	long    retRawScore,   weitScores[ 12 ],  rawScores[ 12 ],     maxScore,  maxScore2,      targScore;  


	retScalePitch =  -1;
	retDidSwap =  false;
	retScoreSecond =  -1;

	maxScalePitch =  -1;    //  init for maximum searc among the 12 scalePitch possibilities
	maxScore =  -1;

	maxScalePitch2 =  -1;
	maxScore2 =  -1;



	for(    short  scalePitch =0;      scalePitch <  12;      scalePitch++    )
	{

		weitScores[ scalePitch ] =    Get_ScalepitchTemplate_Score(  scalePitch,  x,   harmMags,   15,   harmonicThreshold, 
																							  retHarmonicCount,    retRawScore,  filterCode,  kernalWidthHorz  );
		rawScores[ scalePitch ] =     retRawScore;


		if(    useWeightedScores   )
			targScore =    weitScores[ scalePitch ];     //   Toggle which score to use
		else
			targScore =    retRawScore;



		if(    targScore   >   maxScore   )    // Is it big enough to replace the current TOP score ??
		{
			maxScore2       =     maxScore;        //  First bump the current Max to the Second,
			maxScalePitch2 =    maxScalePitch;					   //				******SHOULD this ALWAYS happen ??? **********

			maxScore       =     targScore;
			maxScalePitch =    scalePitch; 
		}
		else
		{														//  ...if not,  is it big enough to replace the current SECOND score
			if(    targScore  >   maxScore2   )
			{
				maxScore2       =     targScore;
				maxScalePitch2 =    scalePitch; 
			}
		}
	}





	bool   useSecondInstead =   false;     	    //  Test if the   'SecondBest'   might be the true pitch


	if(   maxScore2  > 0   )						
	{

		double  ratio =     (double)maxScore   /   (double)maxScore2;

		if(    ratio  <=   firstRatioThresh    )      //  Is the  difference between the too,  TOO small ??(  a Controversie )
		{

								//  Want to find the COMMON(shared) harmonic-ScalePitches between the 2 candidates, and RE-score
								//  the template WITHOUT them, to  test/distinguish  with the more subtle harmonics.
			short  retShareHarms[ 12 ];  
			short  numShared  =     Make_SharedHarmonics_List(    maxScalePitch,   maxScalePitch2,   sharedHarmonicsBaseMidi,
																														retShareHarms   );  


			if(   numShared ==  0   )
			{
				int  dummyBreak = 8;        //  get this when pitches are separated by a 1/2 step
			}
			else if(   numShared  >  1   )   
			{
				int  dummyBreak =  8;
				if(    numShared  >  2   )      //  Have not yet seen a value GREATER than 2
					ASSERT( 0 );
			}



			if(   numShared  >=  1   )   //  Only if there ARE Shared-Harmonics( that might cause 'confusion' or interference) do we RE-measure without the shared.  
			{

				long   scoreExclWeits0,  scoreExclWeits1,     scoreExclRaw0,  scoreExclRaw1;


				scoreExclWeits0 =   Get_ScalepitchTemplate_Score_wExclusions(  maxScalePitch,     x,    retShareHarms,   
																											numShared,   scoreExclRaw0,  
																											filterCode,  kernalWidthHorz   );

				scoreExclWeits1 =   Get_ScalepitchTemplate_Score_wExclusions(  maxScalePitch2,   x,    retShareHarms,   
																												numShared,   scoreExclRaw1, 
																												filterCode,  kernalWidthHorz   );


				if(    scoreExclWeits0  ==   0    ) 			
					useSecondInstead =   true;
				else							//   if this difference is not above a certain tolerance ...  maybe use the raw score, or just NOT swap choices )
				{
					double  secRatio  =     (double)scoreExclWeits1   /   (double)scoreExclWeits0;


					if(    scoreExclWeits1  >  scoreExclWeits0    )
					{
						if(    secRatio  >   secondRatioThresh    )
							useSecondInstead =   true;
						else
							int  dummyBreak =  9;
					}
				}

			}
		}
	}


																			//   get the  'best'  value in every column 

	if(      (   maxScalePitch    >=  0     &&     !useSecondInstead    )
		||   (   maxScalePitch2  >=  0     &&      useSecondInstead    )		)
	{

		if(    useSecondInstead   )
		{
			weitedScore  =    maxScore2;
			retScalePitch =    maxScalePitch2;

			retDidSwap =   true;

			retScalepitchSecond =    maxScalePitch;       //  return the runner up
			retScoreSecond       =    maxScore;
		}
		else
		{  weitedScore  =    maxScore;
			retScalePitch =    maxScalePitch;

			retDidSwap =   false;

			retScalepitchSecond =    maxScalePitch2;  
			retScoreSecond       =    maxScore2;
		}
	}
	else
		ASSERT( 0 );


	return  weitedScore;
}



											////////////////////////////////////////


long    logDFTtrForm::Find_ThreeBest_ScalepitchTemplate_Scores(  long  x,    bool  useWeightedScores,
																								short  retScalePitches[],    long  retScores[],
																								short   filterCode,   short  kernalWidthHorz   )															   
{


	short   harmonicThreshold =    40;    // *** ADJUST ***    ..but not really used.   6/02


	short   candidateCount =   3;


	short   totHarms =  15;
	short   harmMags[  15  ];  

	short   retHarmonicCount;
	long    retRawScore,   weitScores[ 12 ],  rawScores[ 12 ],      targScore;  



	for(    short i=0;     i< candidateCount;     i++    )
	{
		retScalePitches[ i ] =  -1;  		
		retScores[ i ]         =  -1;  
	}




	for(    short  scalePitch =0;      scalePitch <  12;      scalePitch++    )
	{

		weitScores[ scalePitch ] =    Get_ScalepitchTemplate_Score(  scalePitch,  x,    harmMags,   15,   harmonicThreshold, 
																								 retHarmonicCount,    retRawScore,
																								 filterCode,    kernalWidthHorz   );
		rawScores[ scalePitch ] =     retRawScore;


		if(    useWeightedScores   )
			targScore =    weitScores[ scalePitch ];     //   Toggle which score to use
		else
			targScore =    retRawScore;




		if(    targScore   >   retScores[ 0 ]   )    // Is it big enough to replace the current TOP score ??
		{

			retScores[ 2 ]         =    retScores[ 1 ];        //  First bump the Second Max to the Third,
			retScalePitches[ 2 ] =    retScalePitches[ 1 ];				

			retScores[ 1 ]         =    retScores[ 0 ];        //  ...bump the current Max to the Second,
			retScalePitches[ 1 ] =    retScalePitches[ 0 ];					


			retScores[ 0 ]         =     targScore;
			retScalePitches[ 0 ] =     scalePitch; 
		}
		else
		{														//  ...if not,  is it big enough to replace the current SECOND score
			if(    targScore  >   retScores[ 1 ]   )
			{
				retScores[ 2 ]         =    retScores[ 1 ];        //   bump the Second Max to the Third
				retScalePitches[ 2 ] =    retScalePitches[ 1 ];				


				retScores[ 1 ]         =    targScore;
				retScalePitches[ 1 ] =    scalePitch; 
			}
			else
			{  if(    targScore  >   retScores[ 2 ]   )
				{
					retScores[ 2 ]         =     targScore;
					retScalePitches[ 2 ] =     scalePitch; 
				}
			}

		}
	}


	long     lastDiff  =     retScores[ 1 ]   -   retScores[ 2 ];      //  How TIGHT was the last match ??
	return  lastDiff;
}




											////////////////////////////////////////


long    logDFTtrForm::Get_ScalepitchTemplate_Score_xSpan(  short  scalePitch,   long x0,   long x1,   
																						long&  retRawScore   )
{


ASSERT( 0 );    // NOT USED    8/02


	short  filterCode =  0;      //   ***** INSTALL fi use this function again
	short  kernalWidth =  0;


															//  gets the AVERAGE score over the SPAN of  x-pixels
	short   totHarms =  15;
	short   harmMags[  15  ];  
	short   cnt=0,    retHarmonicCount,  harmonicThreshold;     
	long    retWeightedScore,   totalWeitedScore=0,    totalRawScore=0,    weitScore;


	harmonicThreshold  =    40;    // ***** ADJUST *****


	retRawScore =  0;


	for(    long  xDFT=  x0;      xDFT <=  x1;     xDFT++   )
	{

		weitScore =     Get_ScalepitchTemplate_Score(   scalePitch,  xDFT,   harmMags,   totHarms,   harmonicThreshold, 
																			   retHarmonicCount,   retRawScore,   filterCode,  kernalWidth   );
		totalWeitedScore  +=    weitScore;
		totalRawScore      +=    retRawScore;  
		cnt++;
	}
	
	ASSERT( cnt );


	retWeightedScore =     totalWeitedScore  / cnt;     //  from the NEW  'Scalepitch-PitchEl'   also
	retRawScore        =     totalRawScore  / cnt;  


	return   retWeightedScore; 
}



											////////////////////////////////////////


bool    logDFTtrForm::Assign_Pitchels(    ListMemry< PitchEl >&   pitchelList,     CString&  retErrorMesg    )
{

	ASSERT( 0 );   // ***NOT used anymore   ...has the OLD Pitchel list 


	/***
     short      templOffst[ 8 ] =  {   0,    12,    19,    24,      28,     31,     34,                36   };  // only really need seven
//   EX:  in E							      E      E       B      E        G#      B      D( bad fit )      E



     short  i,  midi, y,  val;
     
	 retErrorMesg.Empty();


     if(    pitchelList.Is_Empty()    )        
	 {
		 retErrorMesg =    "Run the PitchEl transform." ;
		 return  false;
	 }

     
    Clear( 0 );          // 0: BLACK  ...erase DEST

    Lock_Bits();	     //  backMap->Lock_Bits();  ...BACKmap is NOT used here	


    ListIterator< PitchEl >    iter(   pitchelList   ); 


	for(    iter.First();     !iter.Is_Done();    iter.Next()    )
    {        
		PitchEl&   pchEl  =     iter.Current_Item();   
		  
																				//  For the 7 harmonics,  write the magnitudes to the DFT's pixels
		for(   i=0;     i<  TOTALhARMONICS;    i++    )
		{


// **** WANT this ???? **************
 
			midi =     (short)( pchEl._MidiPitch )   -12    +     templOffst[ i ];      //    this Harm's Midi value
                         //  NOTE:    need to DROP ( -12 ) cause 'FundamentalTemplateVerter' offsets down when it reads the DFT



            y    =     Pitch_2Ycoord(  midi );			 //  Midi's ycoord

            val  =    (short)(   pchEl._HarmMag[ i ]    );     // Get Harm's intensity value

            Write_Pixel(   pchEl._X,  y,    val, val, val   );
		}
	} 
     

	Unlock_Bits();	  //  backMap->Unlock_Bits();	 ...BACKmap is NOT used here	
	***/
	return  true;
}



											////////////////////////////////////////

/************  need to keep this out for PsPlayer.exe   12/09

long    logDFTtrForm::Assign_Specific_Pitchels(    ListMemry< PitchEl >&   pitchelList,    short  targetScalePitch,   long  startOffset,  
												                        long  endOffset,    bool  atMapStart,   ListLink<PitchEl>*   indexesListlink   )
{


ASSERT( 0 );     //  NOT connected



		//  'atMapStart'  is used for ShortSynthesised play with an onTheFly SMALL DFT, and we want to write the pitchels
		//                     to the BEGINNING of the DFT-map( not offseted to the xCoord of the Pitchel itself )    4/20002

		//  'indexesListlink'  :   OK if NULL

	long        y;
	short      midi,  val,  thisScalePitch;
    short      templOffst[ 8 ] =  {   0,    12,    19,    24,      28,     31,     34,                36   };  // only really need seven
//   EX:  in E							     E      E       B      E        G#      B      D( bad fit )      E



	long   pitchelCount = 0;   //  Really for DEBUG,   want to see it ANY are found after this Index 


    if(    pitchelList.Is_Empty()    )        
	{
		ASSERT( 0 );
		return  0;
	}

	if(    _timeWidth  <=  0    )
	{
		ASSERT( 0 );
		return  0;
	}


	long   startPixel  =     (short)(    startOffset  /  _timeWidth    );		//   Pitchels have timePos in Pixels...   
	long   endPixel   =     (short)(    endOffset   /  _timeWidth    );

	short    pitch[ 12 ];

	for(   short j =0;    j< 12;    j++    )   
		pitch[ j ] = 0;


	long  totalPitchelCount =    pitchelList.Count();     




    Lock_Bits();	     


	if(    indexesListlink  ==  NULL   )
	{
																		
	    ListIterator< PitchEl >       iter(   pitchelList   ); 
// NOT...  SpeedIndexIterator< PitchEl >    iter(   pitchelList,   indexesListlink  );    



		for(    iter.First();     !iter.Is_Done();    iter.Next()    )
		{        

			PitchEl&   pchEl  =     iter.Current_Item();   

			thisScalePitch  = 	pchEl.Get_ScalePitch();  


			if(    startPixel  <=  pchEl._X     &&     pchEl._X  <=  endPixel    )     //  is the Pitchel in our TimeSegment..
			{

				if(    thisScalePitch  ==   targetScalePitch    )
				{
					for(    short i=0;     i<  TOTALhARMONICS;    i++    )    //  For the 7 harmonics,  write the magnitudes to the DFT's pixels
					{
 

						midi =     (short)( pchEl._MidiPitch )   -12    +     templOffst[ i ];      //    this Harm's Midi value
//  ***NOTE:    need to DROP ( -12 ) cause 'FundamentalTemplateVerter' offsets down when it reads the DFT


						y    =     Pitch_2Ycoord(  midi );			 //  Midi's ycoord
						val  =    (short)(   pchEl._HarmMag[ i ]    );     // Get Harm's intensity value


						long   xDst;
						if(   atMapStart   )
							xDst =   pchEl._X  -  startPixel;   // Here we want to OFFSET the xCoord to the beginning of a 'Small DFT' for ShortTimePlay of a single HarmPair timeZone
						else
							xDst =   pchEl._X;

						Write_Pixel(   xDst,  y,    val, val, val   );
					}


					pitch[  targetScalePitch  ]++;
				}
				else															//  count
				{   if(    thisScalePitch  >= 0     &&    thisScalePitch  <= 11    )
						pitch[ thisScalePitch ]++;
					else
						ASSERT( 0 );
				}	
				

				pitchelCount++;   //  Really for DEBUG,   want to see it ANY are found after this Index 
			}
		}      
		
	}
	else						//   OPTIMIZED:  list of indexes of the Pitchel list  (  SpeedIndexIterator  )
	{											
		SpeedIndexIterator< PitchEl >    iter(   pitchelList,   indexesListlink  );    // ****NEW, maybe apply to regular ListIterator **** JPM  5/02


		for(    iter.First();  	  !iter.Is_Done();     iter.Next()    )
		{        
			PitchEl&   pchEl  =     iter.Current_Item();   

			thisScalePitch  = 	pchEl.Get_ScalePitch();  


			if(    startPixel  <=  pchEl._X     &&     pchEl._X  <=  endPixel    )     //  is the Pitchel in our TimeSegment..
			{

				if(    thisScalePitch  ==   targetScalePitch    )
				{
					for(    short i=0;     i<  TOTALhARMONICS;    i++    )    //  For the 7 harmonics,  write the magnitudes to the DFT's pixels
					{
 

						midi =     (short)( pchEl._MidiPitch )   -12    +     templOffst[ i ];      //    this Harm's Midi value
//  ****NOTE:    need to DROP ( -12 ) cause 'FundamentalTemplateVerter' offsets down when it reads the DFT


						y    =     Pitch_2Ycoord(  midi );			 //  Midi's ycoord
						val  =    (short)(   pchEl._HarmMag[ i ]    );     // Get Harm's intensity value


						long   xDst;

						if(   atMapStart   )
							xDst =   pchEl._X  -  startPixel;   // Here we want to OFFSET the xCoord to the beginning of a 'Small DFT' for ShortTimePlay of a single HarmPair timeZone
						else
							xDst =   pchEl._X;

						Write_Pixel(   xDst,  y,    val, val, val   );
					}

					pitch[  targetScalePitch  ]++;
				}
				else															//  get population of  OTHER pitchels  in the timeZone
				{   if(    thisScalePitch  >= 0     &&    thisScalePitch  <= 11    )
						pitch[ thisScalePitch ]++;
					else
						ASSERT( 0 );
				}	


				pitchelCount++;   //  Really for DEBUG, wan to see it ANY are found after this Index 
				
			}   //  if(    startPixel  <=  pchEl._X     &&     pchEl._X  <=  endPixel

			else if(   pchEl._X  >  endPixel   )      
			{
				break;   //  since we KNOW that the list is SORTED by  x-values, we can stop when past the end
			}
			
		}    //  for(     		
	}


	Unlock_Bits();	  


	//TRACE(  "%d  targetPitch   [%d - %d],    0[%d],  1[%d],  2[%d],  3[%d],  4[%d],  5[%d]  6[%d],  7[%d],  8[%d],  9[%d],  10[%d],  11[%d] \n\n",
	//			targetScalePitch,  startPixel,  endPixel,     pitch[0],  pitch[1],  pitch[2],  pitch[3],  pitch[4],  pitch[5],  
	//																		 pitch[6],  pitch[7],  pitch[8],  pitch[9],  pitch[10],  pitch[11]  );

	return  pitchelCount;
}
*****/



												////////////////////////////////////////


bool	 logDFTtrForm::Load_Leading_XColumn(   long  xLead,    CString&  retErrorMesg   )
{


	retErrorMesg.Empty();

	if(    !m_inFloatmapMode   )
	{
		retErrorMesg =  "Load_Leading_XColumn  failed,   is NOT in FloatMap mode." ;
		return  false;
	}



	if(    xLead   ==    m_lastLoadedXColumnLeader    ) 
		return   true;		//  is is ALREADY loaded  



	if(         xLead   !=    ( m_lastLoadedXColumnLeader  +1 )   
		&&    xLead   !=      0     )
	{
		int  dummyBreak =  9;   // ***** REwrite test,  corrds are bad. ***********
	}




																//	a)    Prepare the SOURCE-data for read...
																//							...this gets done in   logDFTBankVerter::Transform_Column()  [ the samples go into SndSample  ]


	/*********   NOT needed here( NO SourceMap ).     ...this gets done in   logDFTBankVerter::Transform_Column()  [ the samples go into SndSample  ]

	if(    m_sourceMapFloat->m_inFloatmapMode    )
	{
		short   xSrcLead =    xLead    +   m_sourceMapFloat->m_readersOffset;
		if(    !m_sourceMapFloat->Load_Leading_XColumn(    xSrcLead,    retErrorMesg   )   )     //  Update the SOURCEmap first( with RECURSION )  
			return  false;
	}			//   (  might be attached to a REGULAR map,  in which case we do NOTexplicitly have to load a column  )
	***/




																//	b)    Scroll the previous columns to the LEFT...


	for(     short  xSRC =  1;       xSRC  < m_width;        xSRC++     )
		Copy_Xcolumn(   xSRC,    (xSRC -1)    );	     


	Assign_Xcolumn(    ( m_width -1 ),   0  );   //  ERASE( init ) the Last column,  cause  Transform_Column_FloatMap() does NOT ERASE 





														 	//	c)       ...and write to LAST column.  

	if(          xLead   >=   0   
	     &&    xLead   <     m_totalFloatmapPixels    )     //  avoid a  'MemoryAccess error'   by reading too far
	{
		m_verterFloatMap->Transform_Column(  xLead  );	   						
	}
	else
	{    }    	//  Just allow a BLANK-column to assign if there is  NO real data yet in the Pipeline  ****** ??????	



	m_lastLoadedXColumnLeader =   xLead;

	return  true;
}







///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////////     ORPHAN subs (below)     /////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/***
short  scalFreqs[ 72 ] =                 //  36  for FASTER searches     
  {  
        330,         349,         370,         392,         415,         440,   
        466,         494,         523,         554,         587,         622,   
      
        659,         698,         740,         784,         831,         880,   
        932,         988,        1046,        1109,        1175,        1245,   
    
       1319,        1397,        1480,        1568,        1661,        1760,   
       1865,        1976,        2093,        2217,        2349,        2489,



       2637,      2793,      2960,      3136,      3322,      3520,		//   Midi  100
       3729,      3951,      4186,      4434,      4698,      4978,


     5274,      5587,      5920,      6272,      6644,      7040,		//   Midi  112   
     7458,      7902,      8372,      8869,      9397,      9956,


      10548,    11175,     11840,    1254,     13289,     14080,    //   Midi  124    
      14917,    15804,     16744,     17739,    18794,     19912
  };
****/

short    Find_Freqs_ScalePitch(  short  freq  )
{

		//  With the way that this is written,  only REALLY need values in scalFreqs[]   for   330  to  659    6/07



     short  i,  bestNote= -99;     //  Ignores 'OCTAVES',  just  returns:    0(E) to 11(Eb) [ LetterNotes ]
     short  upBord, botBord;            
     



     if(  freq < 10  )   
     {  
		 //   NEED this ???   TRACE( "***ERR: #1 Find_Freqs_ScalePitch ******\n" );   // Goes off a lot   1/02
            
		 return -99;                                  // too low to be real?????
     }
     

     if(  freq < 320  )    
	 {  
		 do
		 {  
			 freq *= 2;   // get in 'noteFreq[]' array's range                                
		 }while(  freq < 320  );      
	 } 


     if(  640 < freq  )   
	 {  
		 do{  
				freq /= 2;                                
		 }while(  640 < freq  );     
	 } 
     
     
     if(        freq  <=  338  )      
		 bestNote =   0;					// cutoff BORDERS are midpoints between notes IDEAL freqs
     else if(   604  <=  freq )      
		 bestNote =  11;
     else
     { 
		 for(  i=1;   i <  (12 -1);    i++ )   //  elements in array  'noteFreq[]'
         {
             upBord  =   (  scalFreqs[ i +1 ]  +  scalFreqs[ i    ]   )  /2;  
             botBord =   (  scalFreqs[ i    ]    +  scalFreqs[ i -1 ]  )  /2;
             
             if((   botBord <= freq  )&&(  freq <= upBord  ))    
			 {
				 bestNote= i;   
				 break;
			 }
         } 
       }   // *** FURTHER optimize with LOOKUP-Tables for endpoints, 
           //    and a 'Do - Loop' for fast escape on find
       



     if(    bestNote  ==  -99   )   
	 {
		 ASSERT( 0 );
		 TRACE( "***ERR: #2 Find_Freqs_ScalePitch ******" ); 
	 }
     else                     
		 bestNote =   ( bestNote % 12 );         // force to octave
     


     return    bestNote;
}





											////////////////////////////////////////

/*****
short   Get_Octave_Code(  short  freq  )        
{


	//  use    STATIC  short     FundamentalCandidate::Get_Octave_Index(  double  freq  )     instead


ASSERT( 0 );    	// ********* is this USED ???? ***********


     short  octv= -9;			                 //  OCTAVE '0' is 1st string on guitar

 
     if(                            freq <=   79 )       octv= -2;     // ERROR ????

     else if((    80 <= freq  )&&(  freq <=  159  ))     
		 octv= -1;   // ERROR ????

     else if((   160 <= freq  )&&(  freq <=  319  ))     
		 octv=  0;  // 1st string on guitar

     else if((   320 <= freq  )&&(  freq <=  639  ))     
		 octv=  1;
     
     else if((   640 <= freq  )&&(  freq <= 1279  ))     
		 octv=  2;

     else if((  1280 <= freq  )&&(  freq <= 2559  ))     
		 octv=  3;
  
     else if((  2560 <= freq  )&&(  freq <= 5119  ))     
		 octv=  4;

     else if(   5120 <= freq  )    octv=  5;   // ERROR ????

	 else
	 {	ASSERT( 0 );
	 }
  

// ******  EXTEND these values  ******************************* JPM   6/02


     return   octv;  
} 
*****/


											////////////////////////////////////////


short    Get_OctaveCode_4MidiNumber(  short  midiNum  )
{  

                      //  'oct= 0'  ...1st string on guitar  ***BAD... what if BASS ????
    
    
    midiNum -=   52;     // ******** NEW, test  
    
    if(  midiNum == 0  )         return  0;      // no LATER divide by zero
    
    return   (  midiNum  /  12  );
}



											////////////////////////////////////////


short    Get_ScalePitch_4MidiNumber(  short  midiNum  )
{
                         // Ignores 'OCTAVES',  returns:  0(E) to 11(Eb) [ LetterNotes ]

    midiNum  -=   4;
 
    if(  midiNum  ==  0   )         
	{
		ASSERT( 0 );
		return  0;      // no LATER divide by zero
	}
    
    return   (  midiNum  %  12  );
}



											////////////////////////////////////////


void   Get_ScalePitch_LetterName(  short  sclPitch,   char *firLet,  char *secLet  )
{

 //  ******* BETTER Version :   SPitchCalc::Get_ScalePitch_LetterName(   short  sclPitch,   char *firLet,  char *secLet,    bool  useSharps   );    ****
     

     if((  sclPitch > 11   )||(  sclPitch < 0    ))        
		 return;

     if((  firLet == NULL  )||(  secLet == NULL  ))        
		 return;
     
     switch(  sclPitch  )                
	  {
		 case  0:    *firLet = 'E';      *secLet = ' ';             break;
		 
		 case  1:    *firLet = 'F';      *secLet = ' ';             break;
		 case  2:    *firLet = 'F';      *secLet = '#';             break;
		 
		 case  3:    *firLet = 'G';      *secLet = ' ';             break;
		 case  4:    *firLet = 'G';      *secLet = '#';             break;
		 
		 case  5:    *firLet = 'A';      *secLet = ' ';             break;
		 case  6:    *firLet = 'A';      *secLet = '#';             break;
		 
		 case  7:    *firLet = 'B';      *secLet = ' ';             break;
		 
		 case  8:    *firLet = 'C';      *secLet = ' ';             break;
		 case  9:    *firLet = 'C';      *secLet = '#';             break;
		 
		 case 10:    *firLet = 'D';      *secLet = ' ';             break;
		 case 11:    *firLet = 'D';      *secLet = '#';             break;

    
	     default:   *firLet = ' ';       *secLet = ' ';   break;
	  }
}







											////////////////////////////////////////


void     Get_ScalePitchs_Color_GLB(    short  scalePitchIdx,     short  keyInSPitch,   
																							short&  retRed,      short&  retGreen,   short&  retBlue    )  
{


	// ***************  11/11  AVOID this function   use    SPitchCalc::Get_ScalePitchs_Color()   **************************************
	// *******************************************************************************************************************


//   ****   ScalepitchList::Get_Musical_Key(   short&  retSharpCount,    short&  retFlatCount,    bool&  isMinor   )



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



float     FIR_Filter_FLOAT(   float input,   float *coef,   int n,   float *history   )
{

    int     i;                  // *******ADAPT for FIX some time *********
    float   output;
    float  *hist_ptr,  *hist1_ptr,  *coef_ptr;

    hist_ptr   =   history;
    hist1_ptr =   hist_ptr;                       // use for history update 
    
    coef_ptr =   coef  +  n - 1;                    // point to last coef 


												// form output accumulation 
    output  =   *hist_ptr++   *   (*coef_ptr--);
    
    for(   i= 2;   i< n;    i++   ) 
    {
        *hist1_ptr++ =  *hist_ptr;                   // update history array 
        output  +=     (*hist_ptr++)   *   (*coef_ptr--);
    }
    

    output  +=    input  *  (*coef_ptr);               // input tap 
   
    *hist1_ptr =   input;                              // last history 


    return   output;
}



											////////////////////////////////////////


double    Get_Hertz_Freq_from_MidiNumber(  short  midiNum  )
{

     									// ******** FIX for all senarios ***********

     double  freq = 0.0,    redux= 1.0;                  
  

     short    arraysFirstMidi =    noteFreqArrayFIRSTMIDI;   //   '40'  is  E [ 82 hz ]  the START of    'noteFreq[]'  
	 short    arraysLen       =    noteFreqArrayTOTALCOUNT;   
  


     if(   midiNum   <   arraysFirstMidi  )         // adjust up,  and correct later
     {  
          do{  midiNum  +=  12;     
		         redux *=  2.0;       // keep RAISING by an octave...
                         //  TRACE(  "too LOW:  Get_Hertz_Freq_from_MidiNumber"   );
		  }while(   midiNum  <   arraysFirstMidi  ); 
     }


     if(  midiNum   >=   arraysFirstMidi  +  arraysLen  )
     {  
          do{  midiNum  -=  12;     
		         redux /=  2.0;       // keep LOWERING by an octave...
                         //  TRACE(  "too HIGH:  Get_Hertz_Freq_from_MidiNumber"   );
          }while(  midiNum  >=   arraysFirstMidi + arraysLen  ); 
     }

    
     if((         midiNum   <     arraysFirstMidi      )
		  ||(    midiNum   >=   arraysFirstMidi  +  arraysLen   ))
	 {  
		 TRACE(  "****ERR: Get_Hertz_Freq_from_MidiNumber"   );
		 ASSERT( 0 );
         return  freq;    // will cause FAIL.. later divide by zero
     }  
  
     
     freq  =    noteFreq[   midiNum  -  arraysFirstMidi   ];
     
     freq  /=  redux;         // apply calculation from above
  
     return    freq;  
}  
 


short   Get_noteFreq_Array_Count()
{
	return   noteFreqArrayTOTALCOUNT;
}


short   Get_noteFreq_Array_FirstMidi()
{
	return   noteFreqArrayFIRSTMIDI;
}





								///////////////////////////////////////////////////////////////////////////////////////
								/////////////////////////////////    OMITS    ////////////////////////////////////////
								///////////////////////////////////////////////////////////////////////////////////////


long   logDFTtrForm::Get_Columns_Biggest_Y_val(    long x,    short *pixVal,    bool  harmonicMasked,
																							 short  filterCode,   short  kernalWidthHorz     )
{


ASSERT( 0 );    //  NOT used   8/02



                                               // ONLY get  'the ONE'  MAXimum component...


     short  y, bigestVal, bigestY, val,gr,bl;   
  
     bigestVal= -1;      bigestY= 0;                  
        
     Lock_Bits();					 
     

     for(   y= 0;    y< m_height;    y++   )               
	 {  


		 /***
		 if(    harmonicMasked   )
			 Read_Pixel_Masked(   x,y,    &val,  &gr,&bl    );    //  masked by  Harmonic-Filter( was PitchEl-filter )
		 else
             Read_Pixel(  x,y,    &val,  &gr,&bl  );
		 ***/
		 if(    harmonicMasked    )
			 Read_Pixel_Masked(   x, y,     filterCode,  kernalWidthHorz,    &val,  &gr,&bl  );    //  masked by  Harmonic-Filter( was PitchEl-filter )
		 else
		 {  if(    filterCode  ==   ConvolutionMapFilter::NONE    )
				 Read_Pixel(   x, y,    &val,  &gr,&bl  );
			 else
				 Read_Pixel_Filtered_Horz(   x, y,     filterCode,  kernalWidthHorz,     &val,  &gr,&bl  );
		 }




		if(    val  >   bigestVal   )                                 
		{   
			bigestY    =    y;     
			bigestVal =  val;  
		}
     }  

     Unlock_Bits();    

     *pixVal =   bigestVal;
     
     return   bigestY;    
}







											////////////////////////////////////////

/***

short    Get_std_MidiNote_Index(  short  freq  )
  {                         		     
										// range is   330 hz  to  2489 [ 3 octaves, 36 notes ]
  

                        // 329 is  '2nd to lowest'  E on Guitar, [3rd string]  ****OK ????
  

     short  i,  bestNote= -99;     // Does 'OCTAVES',  just
     short  upBord, botBord;       
     
     if(  freq < 10  )   
         {  
            TRACE( "***ERR#1  Get_std_MidiNote_Index ******" ); 
            return -99;                                  // too low to be real?????
         }
    

     
     if(  freq < 320  )    {  do{  freq *= 2;   // get in 'noteFreq[]' array's range
                                }while(  freq < 320  );      } 


     if(  freq > 2559  )   {  do{  freq /= 2;
                                }while(  freq > 2559   );     } 
     
     

     if(       freq  <=   338  )    
		 bestNote =     0;					 // cutoff BORDERS are midpoints between notes IDEAL freqs
     else if(  freq  >=  2419  )    
		 bestNote =    35;
     else
     { 
		 for(  i=1;   i <  (36 -1);    i++ )   // 36 elements in array  'noteFreq[]'
         {

             upBord  =   (  scalFreqs[ i +1 ]  +  scalFreqs[ i    ]   )  /2;  
             botBord =   (  scalFreqs[ i    ]    +  scalFreqs[ i -1 ]  )  /2;
             
             if((   botBord <= freq  )&&(  freq <= upBord  ))    bestNote= i;            
         } 
     }   				// *** FURTHER optimize with LOOKUP-Tables for endpoints, 
           				//    and a 'Do - Loop' for fast escape on find
       



     if(   bestNote  ==  -99  )    
	 {
		 TRACE( "***ERR#2 Get_std_MidiNote_Index ******" );
		 ASSERT( 0 );
	 }
     else   
         bestNote +=    64;     //   64 is Midi value for 329  ( '2nd to lowest'  E on Guitar,  [3rd string]  ) ******* WHY ??? ******
   


     return    bestNote;
  }
****/




