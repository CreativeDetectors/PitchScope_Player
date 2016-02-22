/////////////////////////////////////////////////////////////////////////////
//
//  SPitchCalc.h   -   
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined(  _VOX_APPS_GLOBALS_H_  )

#define _VOX_APPS_GLOBALS_H_

////////////////////////////////////////////////////////////////////////////


/**********************************
NOTES:

1)   Text-search  for   "RELEASEjm"    for any odd notes for final compilations   6/06

**********************************/


#define VC_CREATIVEdETECTORSaPPS    //  my flag for this series of apps





#define  MAX_ENCRYPT_CHARS_BIG_WRITEsTRING   241      //   like users  NAME  or  EMAIL   fields


#define  MAX_ENCRYPT_CHARS_SMALL_WRITEsTRING  55      //   55   Encrypt-Chars  allows   18 SOURCE-Chars  in PBuile WriteString.  (  WAS  41  )  







#define  NUMPLAYeventsPitchPLAYER   21        //    


#define  NUMPLAYeventsPitchNAVIGATOR   21        //   21 events :  20 PlayEvents{0-19},  one StopEvent{20},  and   22 also messages to go through
																           //   


                                                 //    Fails for  71 and above.... WHY ???      [ 61,  368 sampls per EventSlice  lots of resolution  ]

//  BUG  Is 61 why events come out-of-order???  Do not have this problem when 21.



#define  FUNDAMENTALTEMPLATEoCTAVECOUNT  4

#define  kMIDIsTARTnOTE  52                 //    'midi  52'    is   164 hz   [    E,    Lowest E on Guitar   ]  



#define  kDETECTscoresFILECompat   2.5       //  (Never Change!!)  Must multiply up the DetectScores so that Navigator is compatible with PitchScope2007




#define  logDFTdOWNWARDrEAD   12       //   "Base Harmonic For Read"   is   '12'  semiPitches   BELOW  'Theoretical'  base
//#define  logDFTdOWNWARDrEAD   0   





#define   noteFreqArrayFIRSTMIDI   40         //   these two are for  noteFreq[]  in  DFTtransforms.cpp

#define   noteFreqArrayTOTALCOUNT    96   //  



#define   MAXcharsInLICKnAME  35      //  60  ***** NEVER CHANGE this value, or Files will not be compatible   12/2011  ******************



short    Get_PitchScope_App_Code_GLB();         //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope





				/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//////////// Do NOT  CHANGE these structs ( need be same for BACKWARD COMPATABILY  12/2011 )   //////////////////
				/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


typedef struct 
{													     //  Always keep SRC-file paths at top for easy later editing


		char    projectFilesPath[  255  ];        //   .ppj     keep 1st in struct for ALL  CreativeDetectors files,   The PROJECT file is the 

		char    srcWavFilesPath[  255  ];        //   .WAV

																									 //   a)    'SAMPLE'   info

		long    sampRate;																
		long    samplesPerChunk;      //    'chunkSize'   

		long    totalSampleBytes;	    //    byteCount   for  Sample-DATA only  ( both channels )

		 long    sampleScalePercent;   //   how much the WAV's Amplitude Bars should be enlarged during rendering.



                    

    																								//    b)    'NOTE'   info  

		
		short    detectedNotesChannel;   //    0: Left stereo    1:  Right stereo    2: Center		

		long     numNotesBoth;               //  ***OMIT ? ***   count of ALL SPitches in all  VIRTUAL channels

		short    notemapHeight;				//   the   Fundamental map				
		short    notemapMidiStartPitch;    


   																								    //    c)    'PROJECT'  stuff


		long     detectZoneCount;   //  detectZone count from ALL 3 channels


		short    musicalKey;	 //   >0:  Number of Sharps,     <0:  Number of Flats     0: Zero sharps or flats[ key of C ]
		short    isMinorKey;	  //     0:  False     1: True( is minor )



//    INSTALL ( + other Midi  properties??  )  ???
//
//		short     midiTicksPerQuarternote;   //   might not ALWAYS be  120       1/2004



		long    byRowFileLogDFT; 
		long    totMapBytesLogDFT;


		short   midiInstrument;   //  NEW,  9/07
		short   midiTimingDelay;  


		char      pad[ 512 ];    //   Keep this for MINOR CHANGES to the struct, without changing the size of the struct
									    //   so older .EXEs  can still read ok.   ( too sloppy ???   7/07 )


					
} SPitchListHEADER;    //  for a  NoteList-FILE   [  *******  Do NOT  CHANGE this struct ( need be same for BACKWARD COMPATABILITY  in FILES.   12/2011 ) 





						/////////////////////////////////////////////////////////////////////////////////////////////
						/////////////////////////////////////////////////////////////////////////////////////////////


#define   SPITCHnumSUBoCTAVES   3     //  How many octaves  'BELOW'  OctCand0  for  SPIKE Scores 

#define   NumOCTAVEcandSCORESfile  8    //  only need four, but plan for the future




typedef struct 
{    

	char	 stereoChannel;     //  0:  left,    1:  Right,    3:  Center( L+R ) 
	
	long      startOffset;  
	long      endOffset;   

	short     channelIdx;              //    scalePITCH  value  [ 0-11 ]       0: E      11:  Eb


	short     chosenFundamentalCandIdx;			//    a)   OCTAVE and  Fundamental    Measurements



	// **********************************************************************************************************

	short     editedFundamentalCandIdx;  //  *** NAVIGATOR uses this field for  'detectAvgHarmonicMag'.   MUST adjust future versions of  
														   //   PitchScope ( Scriber ) to treat this field as   'detectAvgHarmonicMag'.    2/2012
														   //   See notes in   CalcedNoteListExternal::Emit_NoteList()

	// **********************************************************************************************************




	short     scoreDetectionAvgTone;			   //	 b)   Region-DETECTION      Measurement  ( from YMax )



	long      octaveCandidScores[  NumOCTAVEcandSCORESfile   ];   //  only need four,   keep for   OctaveCompair Dialog 


	short   extraDataCode;      // For future expansion,  with this we could tell of an extra structure to load after the normal stuff   7/2007
	long 	  extraDataByteSize;    



	char      pad[ 64 ];     //   Keep this for MINOR CHANGES to the struct, without changing the size of the struct
									//   so older .EXEs  can still read ok.   ( too sloppy ???   9/2006  )


} SPitchFileObj;    //  [  *******  Do NOT  CHANGE this struct ( need be same for BACKWARD COMPATABILITY  in FILES.    12/2011 ) 





						/////////////////////////////////////////////////////////////////////////////////////////////


typedef struct 
{    

	char	 stereoChannel;     //  0:  left,    1:  Right,     ... 3:  Mono ???     ...4: Center( new centered logDFT )
	
	long     startOffset;  
	long     endOffset;  
	

																	//   Musical METER info( the beat )

	long			m_samplesPerBeat;	

	long			m_beatsInOneBar;              //  NUMERATOR in:       4/4,    3/4      new,  only gives default[  same for ALL DZones ?? ]
	
	long            m_beatsPerQuarterNote;     //  DENOMINATOR in:    4/4,    3/4    ...Used ???    1/04

    long		    m_firstRestBeatsInFirstBar;						

	long			originalOffsetForBeat;
	long		    userOffsetForBeat;        //   in Chunks(  chunkSize )


														//    filtering parms used on the 8 bit sample for later correlation 

	long   m_topFrequencyLimit;				
	long   m_bottomFrequencyLimit;
	long   m_volumeAdjPercent;



									//   new vars for new  'embedded DFT'  in DZone   5/06


	long       m_logDFTtotalBytes;    //  if ==  -1       then no DFT is supposed to be present(  as for DSTlist saves )
	BYTE    *m_DFTsBits;               //  If == NULL     then no DFT is supposed to be present				 logDFT->m_bits; 


	long       m_logDFTwidth;        //  Actual map width
	long       m_bytesInRow;


																													// ***NEW  ( 9/06 ) ****
	short       m_dftNumberOfPeriods;   //  amount to BandSpread on dialog.  
	short       m_dftsMidiStartBand;       //  may change if we start to analyze BASS instruments
	long        m_dftHeight;                   //   may make a taller DFT in the future

	short       m_filterCodeDFTmapReads;    //  usually MEDIAN                           ( to create Derivative-Maps )
	short       m_filterWidthDFTmapReads;   //  the size of the kernal that filter      ( to create Derivative-Maps )
	short       m_pixThresholdHPairs;           //                                           ( to create Derivative-Maps )


 	
//   (  Should print these values in STATIC-text on the DZone properties Dialog? [ Only if the user will be abole to change
//      them with a CONTROL remember, will have new Detection-Dialogs )
//
//   SPitchListCreateParms    ...NO!!!!!!     This is about NOTE-detection, which is independant of DZone-creation   9/06
//			m_detectionAlgoID;
//			OctaveCorrectParms  
//		   m_detectionsParmsStruct;  



	short   extraDataCode;      // For future expansion,  with this we could tell of an extra structure to load after the normal stuff   7/07
	long 	  extraDataByteSize;    


	char      pad[ 128 ];    //   Keep this for MINOR CHANGES to the struct, without changing the size of the struct
									//   so older .EXEs  can still read ok.   ( too sloppy ???   9/06  )


} DetectZoneFileObj;    //   [  ******* Do NOT  CHANGE this struct ( need be same for BACKWARD COMPATABILITY  in FILES.   12/2011 ) 





						/////////////////////////////////////////////////////////////////////////////////////////////


typedef struct 
{    
	
	long      startSample;  
	long      endSample;   

	char      nickName[   MAXcharsInLICKnAME  + 2  ];     //   [  30,  big enough???  Can never change  12/2011



	short   extraDataCode;      // For future expansion,  with this we could tell of an extra structure to load after the normal stuff   12/11
	long 	  extraDataByteSize;    

	char      pad[ 64 ];     //   Keep this for MINOR CHANGES to the struct, without changing the size of the struct
									//   so older .EXEs  can still read ok.   ( too sloppy ???   9/06  )


} LickFileObj;    //  [  *******  Do NOT  CHANGE this struct ( need be same for BACKWARD COMPATABILITY  in FILES.    12/2011 ) 





////////////////////////////////////////////////////////////////////////////

#endif   //   _VOX_APPS_GLOBALS_H_




