/////////////////////////////////////////////////////////////////////////////
//
//  NoteGenerator.h   -   
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_NOTEGENERATOR_H__0121__INCLUDED_)
#define AFX_NOTEGENERATOR_H__0121__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////////////



class  BitSourceStreaming;


class   OffMap;

class   SndSample;

class   SPitchCalc;

class   CalcedNote;
class   MidiNote;

class   Lick;






////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   NoteGenerator
{
public:
	NoteGenerator(  );
	virtual   ~NoteGenerator(void);



	SPitchCalc*   Get_SPitchCalcer();      //   gets it from the 'BitSource'



// ***********************   WHY don't I have these functions  in   SPitchCalc???     Is the  PlayerApp  the upperlevel class I thought Note generator would be ????


	bool		Init_NoteGenerator(    ListDoubLinkMemry<MidiNote>*  listsPtr,   ListDoubLinkMemry<MidiNote>*  masterListsPtr,   ListMemry< Lick >*  lickListPtr,    CString&  sourceFilesPath,  //   long  totalSrcFilesBytes,    
															BitSourceStreaming  *bitSource,    short  midiInstrumentPatch,   	CString&  retErrorMesg    );


	void		Add_CalcedNote_to_Record_List(   CalcedNote&  calcedNote,    long  endSampleIdx,     bool   justSwitchingToANewPitch   );


	bool		Record_Midi_Note(   CalcedNote&  calcedNote,     CString&  retErrorMesg  );   // is very similar to  Process_Midi_Event() 		





	bool		Ask_User_Merge_or_Save_New_Notes(   long  lastProcessedSampleIdx,    bool&  retUserDidMerge,    CString&  retErrorMesg   );


	bool	    Save_Recorded_Notes_withDialog(         long  lastProcessedSampleIdx,     CString&  retErrorMesg   );    //  launches a Dialog to ask user for a FileName for the Notelist


	bool		Merge_TempList_to_Master_Notelist(   CString&  retErrorMesg   );




/*************    New FUNCTIONS:


Populate_Bitmap_from_Notelist()

PreRoll_Fill_Final_CircQue()

***/



public:
	ListDoubLinkMemry< MidiNote >    *m_tempNoteList;        //   it resides in  PitchPlayerApp

	ListDoubLinkMemry< MidiNote >    *m_masterNoteList;    //   it resides in  PitchPlayerApp


	ListMemry< Lick >		  *m_lickListPtr;      //   Short Musical PHRASES




	short        m_filesVersionNumber;




	BitSourceStreaming     *m_bitSourceStreaming; 


	CalcedNote  *m_lastStartPlayingNote;

	short             m_synthsCurrntSPitch;    //  Need to persist  during recording process.  Used to be  'retSynthsCurPitch'  in  Process_Event_Notification_PPlayer()



	double	m_volumeBoost;     //  weird,  but I like to boost the volue by 3x  when creating the recorded not.  2/2012


	short    m_midiInstrumentPatch;

	
	long     m_sampleIdxRecordingStart;     //  probably earlier than  'm_sampleIdxFirstRecordedNote'
	long     m_sampleIdxRecordingEnd;  



	long		m_sampleIdxFirstRecordedNote;   //   Ver SLOPPY measurement,  adds some arbitrary padding 
	long		m_sampleIdxLastPause;



	CString     m_sourceFilesPath;	

	long          m_totalSrcFilesBytes;

	long          m_bytesPerSample;


	short        m_musicalKey;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_NOTEGENERATOR_H__0121__INCLUDED_)
