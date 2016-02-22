/////////////////////////////////////////////////////////////////////////////
//  CalcNote.h   -   properties for a new midi note:  ScalePitch, Octave, Duration
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2009-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////

#pragma once




#define   kCountOfNavOctaveCandidates   4    




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   CalcedNote     
{
						
public:
	CalcedNote();


	bool   Has_Valid_Data();




public:
	short   scalePitch;       
	short   octaveIndex;  



	long    beginingSampleIdxFile;  //  for FILE save,  this is the Physical BEGINNING of the note.   Is now very accurate cause tested with NoAudion noteLists.  8/2012


	long     curSampleWaveman;        //  Is very similar to 'beginingSampleIdxFile' , but does NOT have the Detection-DELAY subtracted out. Is assigned in  Estimate_ScalePitch()
													 //		  ....KEEP,  good for occasional debugging.   12/2011




	long    pieSliceIdxAtDetection;        //   Also 'FakeEvent' in  Make_NoteList_No_Audio().     Maybe useful in Get_New_Filtered_ReturnNote_CircularQue()  instead 
													   //    of  'primaryQuesSampleIdx'.    Is almost useless  AFTER  Get_New_Filtered_ReturnNote_CircularQue()
													   //    REMEMBER:  PieSlice CHANGES in SIZE for different SLOW-SPEEDS,  but  { primaryQuesSampleIdx  } are absolute.  

	long    pieSliceIdxAfterFiltration;     //  this is the  'curPieSliceIdx'  when the FilteredNote comes OUT of 1st CircQue [ in Detect_Note_for_Pie_Slice,  Search_NoteList_for_AudioMusical_Values   ],  
	                                                   //   and is just then ADDED to the  FinalCircQue.




	long     pieSliceCounter;      //  is only initialized at App start.  Counts  ALL the pieslices during a session with the app.  When should I initialize it?? Between songs?    3/3/12





	short   synthCode;    //   1:  start a new note       0:  turn off the current note       -1: Do nothing  


	short   detectScoreHarms;       //  for ScalePitch determination
	short	  detectAvgHarmonicMag;   // should control the VOLUME of the playing Midi Notes.  Also their BRIGHTNESS on the display.


	short	  detectScoreOctaveCandids[   kCountOfNavOctaveCandidates   ];   //  4



	long    primaryQuesSampleIdx;     //  the  TIME-STAMP we put on the note during Estimate_ScalePitch(),  just before it goes into the PRIMARY CircQue


	long  expectedEventNumber;  
	long  realEventNumber;   

//  short   playSpeedAtDetection    ***** From this we could calc the  "PIE-SLICE's number of SAMPLES"   and get the time width for this.
};







////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   MidiNote     
{
			//     MidiNote  represent a  "INTERVAL in Time",   that has a ScalePitch and Octave  
			//
			//    MidiNote:    For the  creation of a PitchScope NoteList within Navigator.  We first detect a  ListMemry<CalcedNote>  before the save to file. 
			//
			//   ENHANCE:   Have an Envelope-Descriptor?  


public:
	MidiNote();


	bool    Has_Valid_Data();

	void    Copy_In_CalcNotes_Data(   CalcedNote&  cNote   );




public:
	long    beginingSampleIdxFile;          //   for FILE save,   this is the BEGIN of the note
	long    endingSampleIdxFile;             //   for FILE save,   this is the END of the note



	short   scalePitch;       
	short   octaveIndex;  

	short   detectScoreHarms;       //  for ScalePitch determination
	short	  detectAvgHarmonicMag;   // should control the VOLUME of the playing Midi Notes.  Also their BRIGHTNESS on the display.


//	short	  detectScoreOctaveCandids[   kCountOfNavOctaveCandidates   ];  
	long	  detectScoreOctaveCandids[   kCountOfNavOctaveCandidates   ];   //  4

};
