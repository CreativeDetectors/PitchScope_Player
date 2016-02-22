/////////////////////////////////////////////////////////////////////////////
//
//  NoteGenerator.cpp   -   
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


#include  <math.h>     //  for trig functions


#include  "..\comnFacade\VoxAppsGlobals.h"

#include   "..\comnFacade\UniEditorAppsGlobals.h"




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



#include  "..\comnInterface\DetectorAbstracts.h"   

#include  "..\comnAudio\FundamentalCandidate.h"




#include  "DFTtransforms.h"

#include  "..\ComnAudio\HarmPairsTrForm.h"
#include  "..\ComnAudio\HarmPairsVerter.h"


#include  "..\ComnAudio\sndSample.h"





//////////////////////////
///////////////////////////

#include  <mmsystem.h>   // Is good on Sparky  ( ProgramFiles\Microsoft Visual Studio\VC\PlatformSDK\Include )



#include  "..\comnAudio\BitSourceAudio.h"
///////////////////////////


#include "FundCandidCalcer.h"


#include  "..\ComnAudio\CalcNote.h"

#include  "..\ComnAudio\SPitchCalc.h"



#include "..\comnCatalog\External.h"
#include "..\comnCatalog\CalcedNoteExternal.h"    //  new




#include  "..\ComnAudio\NoteGenerator.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////


void       Get_ScalePitch_LetterName(  short  sclPitch,   char *firLet,  char *secLet  );





////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


NoteGenerator::NoteGenerator() 
{


	m_volumeBoost  =   3.0;     //   3.0         ************   Do NOT ADJUST !!!!   ******************************
											 //      ...Need to boost these scores for OLD PitchScope ( NoteBrightnes  is based  on DETECTION SCORE   2/2012



	m_filesVersionNumber =   -1;



	m_bytesPerSample  =   4;    // *************   HARDWIRED,  OK ????   ********************

	m_tempNoteList =   NULL;
    m_masterNoteList =  NULL;


	m_lickListPtr = NULL;


	m_lastStartPlayingNote  =  NULL;
	m_lastStartPlayingNote  =  new   CalcedNote();    //  easier for compile if I do this     3/11

	m_synthsCurrntSPitch =  -1;


	m_sampleIdxFirstRecordedNote =   -1;
	m_sampleIdxLastPause  =   -1;


	m_sampleIdxRecordingStart =   -1;    //  probably earlier than  'm_sampleIdxFirstRecordedNote'
	m_sampleIdxRecordingEnd =   -1;



	m_totalSrcFilesBytes =  -1;

	m_sourceFilesPath.Empty();

	m_musicalKey =  0;
}



											////////////////////////////////////////


NoteGenerator::~NoteGenerator() 
{

	if(   m_lastStartPlayingNote  !=  NULL   )
	{
		delete  m_lastStartPlayingNote;
		m_lastStartPlayingNote =  NULL;
	}
}


											////////////////////////////////////////


SPitchCalc*    NoteGenerator::Get_SPitchCalcer()
{

	if(   m_bitSourceStreaming  ==  NULL   )
	{
		ASSERT( 0 );
		return  NULL;
	}

	SPitchCalc  *retSPitchCalc  =    m_bitSourceStreaming->m_sPitchCalc;

	return  retSPitchCalc;
}



											////////////////////////////////////////


bool	  NoteGenerator::Init_NoteGenerator(   ListDoubLinkMemry<MidiNote>  *listsPtr,    ListDoubLinkMemry<MidiNote>*  masterListsPtr,  
											                                         ListMemry< Lick >*  lickListPtr,      CString&  sourceFilesPath,  
											                                        BitSourceStreaming  *bitSource,   short  midiInstrumentPatch,    CString&  retErrorMesg     )
{

							//  Make sure CALLING FUNCTION   set     	BitSourceStreaming::m_recordNotesNow  =   true 

							//   CALLED by:     PitchPlayerApp::Record_Notes()      &    PitchPlayerApp::Make_NoteList_No_Audio()


	retErrorMesg.Empty();


	if(   bitSource ==  NULL   )
	{
		retErrorMesg =   "NoteGenerator::Init_NoteGenerator  FAILED,   bitSource is  NULL" ;
		return  false;
	}



	if(   listsPtr ==  NULL   )
	{
		bitSource->m_recordNotesNow  =    false;   

		retErrorMesg =   "NoteGenerator::Init_NoteGenerator  FAILED,   listsPtr is  NULL" ;
		return  false;
	}


	if(   masterListsPtr ==  NULL   )
	{
		bitSource->m_recordNotesNow  =    false;   

		retErrorMesg =   "NoteGenerator::Init_NoteGenerator  FAILED,   masterListsPtr is  NULL" ;
		return  false;
	}



	if(   lickListPtr ==  NULL   )
	{
		bitSource->m_recordNotesNow  =    false;   

		retErrorMesg =   "NoteGenerator::Init_NoteGenerator  FAILED,   lickListPtr is  NULL" ;
		return  false;
	}





		
	listsPtr->Empty();    //  get rid of any earlier noteLists.     ****** Do I always WANT this ????  



	m_sourceFilesPath    =    sourceFilesPath;	

	m_tempNoteList           =   listsPtr; 

	m_masterNoteList =    masterListsPtr;

	m_lickListPtr             =    lickListPtr;

	m_bitSourceStreaming  =   bitSource;

	m_midiInstrumentPatch  =   midiInstrumentPatch;



//	m_totalSrcFilesBytes =   totalSrcFilesBytes;   
	m_totalSrcFilesBytes =     bitSource->Calc_Files_Total_Output_Bytes_With_SpeedExpansion();   //   Number ius EXPANDED by the SLOW-speed Ratio    ******* UN TESTED  ********** 



	SPitchCalc    *sPitchCalc  =      Get_SPitchCalcer();     //  need to do this after   'm_bitSourceStreaming'   is assigned

	if(    sPitchCalc  !=   NULL   )
	{
		sPitchCalc->Initialize_for_Play(); 

		m_musicalKey =   sPitchCalc->m_musicalKey; 
	}
	else
	{	int    dummy  =   9;  
		ASSERT( 0 );
	}



	m_sampleIdxFirstRecordedNote =   -1;   //  init

	m_sampleIdxLastPause             =   -1;   //  init


	m_synthsCurrntSPitch =    -1;



	bitSource->m_recordNotesNow  =    true;  

	return  true;
}



											////////////////////////////////////////


void    NoteGenerator::Add_CalcedNote_to_Record_List(   CalcedNote&   calcedNote,    long  endSampleIdx,   bool   justSwitchingToANewPitch    )
{

						                                  //   Only CALLED  by    Record_Midi_Note()  


	double    volumeBoost  =   m_volumeBoost;           //  3.0 ;   //   1.5     *************   ADJUST *******************************


	long   chunkSize =   512;

	long   detectZonePaddingInSamples =    chunkSize   *  4;   //   4[ harly visible]   ***********  ADJUST  ******************


	bool    showTraceText =   false;


	if(   m_tempNoteList  ==  NULL   )     //  think that calling function does this test 
	{
		AfxMessageBox(  "Add_CalcedNote_to_Record_List  FAILED,   m_tempNoteList is NULL.   "    );
		return;
	}


	MidiNote   *nuCalcNote     =         new   MidiNote();
	if(              nuCalcNote ==  NULL  )
	{
		AfxMessageBox(  "Add_CalcedNote_to_Record_List  FAILED, could NOT alloc  CalcedNote."    );
		return;
	}




	if(    m_sampleIdxFirstRecordedNote   <  0   )     //  not yet written to for this recording session ?
	{

					//   This will be saved in the file as the DetectZone's  start.

		long   detectZoneStart  =    calcedNote.beginingSampleIdxFile   -   detectZonePaddingInSamples;  

		if(     detectZoneStart  <  0    )
			detectZoneStart =   0;


		m_sampleIdxFirstRecordedNote =    detectZoneStart;        // ***** SLOPPY,  but use this for the start of detect zone
	}





	short   harmonicsMagnitude   =    calcedNote.detectAvgHarmonicMag;   //  we use this for AUDIBLE Volumne control in  Process_Midi_Event()

	short   detectScoreHarms      =    calcedNote.detectScoreHarms;          //  BUT  ScoreHarms is what determines volume for PitchScope ???
																										     //   and the Britness of the notes on the display.


// *********   NOT sure why I change this score,  maybe for OLD PitchScope,  maybe for when PixelBrightness depended on Score and not HarmMag  2/6/2012  *****************

	short   nuDetectScoreHarms  =    (short)(    volumeBoost    *    (double)detectScoreHarms    );  //  make bigger

// *************************************************************************************




//	*nuCalcNote      =      calcedNote;                                 //    Copy all of the struct for a ROUGH  initialization...
	 nuCalcNote->Copy_In_CalcNotes_Data(  calcedNote  );

		nuCalcNote->endingSampleIdxFile =    endSampleIdx;      //  ...then refine the initialization

		nuCalcNote->detectScoreHarms    =    nuDetectScoreHarms;



	m_tempNoteList->Add_Tail(    *nuCalcNote   );  




	if(   showTraceText   )
	{
		if(   justSwitchingToANewPitch   )
			TRACE( "      ...added NEW NOTE [ %d ]  to list   ( start - stop sampleIdx:     %d      %d   )   Just PITCH-SWITCH   \n",
											 nuCalcNote->scalePitch,    nuCalcNote->beginingSampleIdxFile,     nuCalcNote->endingSampleIdxFile   );
		else
			TRACE( "      ...added NEW NOTE [ %d ]  to list   ( start - stop sampleIdx:     %d      %d   )    \n",
											 nuCalcNote->scalePitch,    nuCalcNote->beginingSampleIdxFile,     nuCalcNote->endingSampleIdxFile   );
	}
}




											////////////////////////////////////////


bool   NoteGenerator::Record_Midi_Note(   CalcedNote&  calcedNote,    CString&  retErrorMesg  )
{


	double   volumeBoost =   5.0;     //  ***ADJUST***  [3.0]weak with new fixed-filtering    [ 4.0 sill weak ]   [ 6 little strong ]


	bool      dumpToTrace =   false;



	retErrorMesg.Empty();


	ASSERT(   m_lastStartPlayingNote    );


	 short  synthCode  =   calcedNote.synthCode;   // The same 


	/***
	if(    sPitchCalcer  ==   NULL   )
	{
		ASSERT( 0 );
		retErrorMesg =    "NoteGenerator::Record_Midi_Note  FAILED,  can NOT record notes,  SPitchCalc is NULL. ";
	//	m_recordNotesNow =   false;    Calling function does this
		return  false;
	}
	****/


	if(   m_tempNoteList ==  NULL   )    //   CalcedNote
	{
		ASSERT( 0 );
		retErrorMesg =    "NoteGenerator::Record_Midi_Note  FAILED,  can NOT record notes,  m_tempNoteList is NULL. ";
	//	m_recordNotesNow =   false;
		return  false;
	}
	 


	bool   haveAnewNoteStart  =    false;


	if(            calcedNote.synthCode  ==   1              //  1:  start of new note
		    &&   calcedNote.beginingSampleIdxFile  >=  0
			&&   calcedNote.scalePitch   >=   0     )    
	{
		haveAnewNoteStart  =   true;
	}





	if(    synthCode  ==  1   )
	{

		
		if(       calcedNote.octaveIndex   <   0  
			||   calcedNote.octaveIndex   >=  4  )    
		{  
			if(   dumpToTrace   )
			{	TRACE( "*** Can NOT find the octave***    NoteGenerator::Record_Midi_Note \n"  );   } 
		}
		else
		{	ASSERT(    calcedNote.scalePitch    >=  0   );       

			long    midiVolumeApp =   100;
			short   auditionCode =         1;      //  1:   AnimeStream::MIDIandSAMPLE 



			if(   calcedNote.synthCode  !=  1  )    //   **** REDUNDANT???  it already has this value  *************************************
			{	ASSERT( 0 );   }

			calcedNote.synthCode =   1;   //    **** REDUNDANT???    big,   record what we did with this note   



			short   pitchMidi  =     52   +   calcedNote.scalePitch   +    (  calcedNote.octaveIndex *  12 );        //   52  =   kMIDIsTARTnOTE
					



			bool   justSwitchingToANewPitch  =   false;

 
			if(             pitchMidi   !=   m_synthsCurrntSPitch        //   Now persists as a member var...  OK ???   1/23/12      
//				&&   (   m_midiSequencer->m_curPitch >= 0   &&   m_midiSequencer->m_curPitch  <= 11  )     ****** WANT this ???  NO!!! *****     3/11   
			  )   
			{   
				justSwitchingToANewPitch  =   true;     //  think this is also true if the current pitch, notes are  UNDEFINED ( "nothing"  )
			


				if(          m_lastStartPlayingNote->scalePitch  >= 0    &&   m_lastStartPlayingNote->scalePitch  <= 11 
					  &&   m_lastStartPlayingNote->beginingSampleIdxFile  >=  0    )     				
				{	
					Add_CalcedNote_to_Record_List(    *m_lastStartPlayingNote,    calcedNote.beginingSampleIdxFile  -1,     justSwitchingToANewPitch  );  
				}
				

				*m_lastStartPlayingNote =   calcedNote;   //  only is updated at the START of a RUN of SIMILAR  scalePitch  CalcedNotes
			}

							

			m_synthsCurrntSPitch   =     pitchMidi; 


			 if(   dumpToTrace   )
			 {
				/****
				 char   firLet,   secLet;	
				 Get_ScalePitch_LetterName(   calcedNote.scalePitch,   &firLet,  &secLet   );

				 TRACE(   "START synth:  [ sampIdx  %d,    wavsSampleIdx  %d,    Delay[ %d,  %d ]     [  event %d,  dataFetch  ]      %c%c   %d    [  DetScore =    OctScore =  %d    AvgMag = %d  ] \n", 
							        calcedNote.beginingSampleIdxFile,   wavsSampleIdx,  (calcedNote.beginingSampleIdxFile - wavsSampleIdx),  playSpeed * (calcedNote.beginingSampleIdxFile - wavsSampleIdx), 
									eventNumber, 
								//	dataFetchCount, 
									 firLet,  secLet,   calcedNote.octaveIndex,   
								//	 m_curScalePitchDetectionScore,  
									 calcedNote.detectScoreOctave,    calcedNote.detectAvgHarmonicMag	);
				 ****/
			 }
		}
	
	}   //   if(    synthCode  ==  1 


	else if(   synthCode  ==  0   )
	{  

//		if(   dumpToTrace   )		{   TRACE(  "STOP synth  "   );   }


		if(   calcedNote.synthCode  !=  0  )    //   **** REDUNDANT???  it already has this value  *************************************
		{	ASSERT( 0 );   }

		calcedNote.synthCode =   0;   //  **** REDUNDANT???     BIG,   record what we did with this note



		if(   dumpToTrace   )		
		{   
			/****
				TRACE(  "...STOP synth [ sampIdx  %d,    wavsSampleIdx  %d,     event %d,  dataFetch  ]     ..add new Note [ %d ]     [  %d  -  %d  ]  \n",   
					                      calcedNote.beginingSampleIdxFile,    wavsSampleIdx,   eventNumber,   //   dataFetchCount, 
										  m_lastStartPlayingNote->scalePitch,   m_lastStartPlayingNote->beginingSampleIdxFile,   calcedNote.beginingSampleIdxFile    );  
			  ****/
		}


		if(         m_lastStartPlayingNote->scalePitch  >= 0    &&   m_lastStartPlayingNote->scalePitch  <= 11  
             &&   m_lastStartPlayingNote->beginingSampleIdxFile  >=  0    )     	
		{	
			Add_CalcedNote_to_Record_List(    *m_lastStartPlayingNote,   calcedNote.beginingSampleIdxFile  -1,     false   );
		}



		*m_lastStartPlayingNote =   calcedNote;   //  only is updated at the START of a run of similar scalePitch  CalcedNotes.
																	 //  BUT in this case they are all 'nothing' notes ( scalePitch = -1  )
																		 //     I need this,  but do not sure why.     3/11

		m_synthsCurrntSPitch  =   -1;
	}

	else if(   synthCode  ==  -1   )  	
	{   

		if(   calcedNote.synthCode  !=  -1  )    //   **** REDUNDANT???  it already has this value  *************************************
		{	ASSERT( 0 );   }

		calcedNote.synthCode =   -1;   //  **** REDUNDANT???       will probably not process these,  but record the  synth-state anyway    ************  MIGHT not need this assignment   3/30   ***********

	}   //   do nothing


	



	if(   calcedNote.scalePitch ==  -1   )     // means do NOT render a bullet, our detection was too weak
	{
		if(   dumpToTrace   )
		{	
			/*****
			TRACE( "       none   [ sampIdx  %d,    wavsSampleIdx  %d,    Delay[ %d,  %d ],     [  event %d,  dataFetch  ]   \n",   
																		calcedNote.beginingSampleIdxFile,    wavsSampleIdx,   
													(calcedNote.beginingSampleIdxFile - wavsSampleIdx),    playSpeed * (calcedNote.beginingSampleIdxFile - wavsSampleIdx), 
															eventNumber   //  ,  dataFetchCount  	);    
			****/
		}
	}
	else
	{  if(        dumpToTrace 
		    &&   synthCode  !=  1 )    //  do NOT duplicate results above
		{						

		   /****
		   char   firLet,   secLet;	
		   Get_ScalePitch_LetterName(   calcedNote.scalePitch,   &firLet,  &secLet   );

			TRACE( "              [ sampIdx  %d,    wavsSampleIdx  %d,    Delay[ %d,  %d ],     [  event %d,  dataFetch   ]   %c%c    %d  [  DetScore = %d,    OctScore = %d    AvgMag = %d   ]     \n"  ,   
														calcedNote.beginingSampleIdxFile	,  wavsSampleIdx, 
											(calcedNote.beginingSampleIdxFile - wavsSampleIdx),   playSpeed * (calcedNote.beginingSampleIdxFile - wavsSampleIdx), 
																			eventNumber,  // dataFetchCount,
																			firLet,  secLet,   calcedNote.octaveIndex,  
															  	calcedNote.detectScoreHarms,     calcedNote.detectScoreOctave,   calcedNote.detectAvgHarmonicMag   );
			****/
		}
	}

	return  true;
}





										////////////////////////////////////////


bool	  NoteGenerator::Save_Recorded_Notes_withDialog(   long   lastProcessedSampleIdx,    CString&  retErrorMesg    )
{

													//   Now save the list to disc as Scalepitches

	CString   filesExtension =    "pnl"  ;  
	CString   objectName   =    "Note"  ; 


	retErrorMesg.Empty();
																				
//	CString   finalPath  =         "c:\\Users\\JamesM\\VoxSep\\Projects\\LatestTest\\CalcedNotes_2.pnl"   ;   // *********  TEMP **************
	CString   finalPath;

	ASSERT(  m_bitSourceStreaming  );



	if(   m_tempNoteList  ==  NULL   )
	{
		retErrorMesg =   "NoteGenerator::Save_Recorded_Notes_withDialog  FAILED,  m_tempNoteList is NULL. " ;
		m_bitSourceStreaming->m_recordNotesNow =   false;             //  Must always ASSIGN this  for the Bitsource from this function   3/11
		return   false;
	}


	if(   m_bitSourceStreaming->m_recordNotesNow  ==   false   )
	{
		retErrorMesg =   "NoteGenerator::Save_Recorded_Notes_withDialog  FAILED,   m_recordNotesNow  is  false " ;
		return   false;
	}



	long   listCount  =    m_tempNoteList->Count();
	if(      listCount  <=  0   )
	{
		retErrorMesg =   "NoteGenerator::Save_Recorded_Notes_withDialog  FAILED,  there are NO NOTES to save to file. " ;
		m_bitSourceStreaming->m_recordNotesNow =   false;         //  Must always ASSIGN this  for the Bitsource from this function   3/11
		return   false;
	}



//	ASSERT(   m_sampleIdxFirstRecordedNote  >= 0    );   *******************   WANT this ???   3/12/11 *********************

	if(    m_sampleIdxFirstRecordedNote  < 0    )
	{
		ASSERT( 0 );
		m_sampleIdxFirstRecordedNote =    0;    //  this might be good enough
	}



	long    totalSamples  =    m_totalSrcFilesBytes;   //   m_bitSourceStreaming->Calc_Files_Total_Output_Bytes_With_SpeedExpansion();

	totalSamples            =     totalSamples  /  m_bytesPerSample;



//	CString   sourceWAVpath;
//	m_bitSourceStreaming->Get_Wav_Filename(   sourceWAVpath  );

	ASSERT(   ! m_sourceFilesPath.IsEmpty()    );   // ***********  SLOPPY,  want   Error Handling ???    3/11





	bool  retUserCanceled = false;      //  Open a dialog and ask the user for a FileName for the newly saved Notelist
	CString   strMesg;  
//	strMesg.Format(    "%s NoteList Files (*.%s)|*.%s||" ,    objectName,   objectName,   filesExtension   );
	strMesg.Format(    "NoteList Files (*.%s)|*.%s||" ,                                filesExtension,   filesExtension   );


	CFileDialog    dlg(     FALSE,
								   _T( filesExtension ),      //   _T( "pnl" ),          pnl:     "Vox  Object List "
								   NULL,   //    curName,   //   NULL, 
								   OFN_HIDEREADONLY   |    OFN_OVERWRITEPROMPT,
									_T(   strMesg  )		//   _T(    "VoxSep ObjectList Files (*.pnl)|*.pnl||" )   
							);

	LONG  nResult    =      dlg.DoModal();
	if(       nResult  !=  IDOK  )
	{  
		retUserCanceled  =  true;
		m_bitSourceStreaming->m_recordNotesNow =   false;  
		return  true;
	}

	finalPath  =    dlg.GetPathName();  




		
	ASSERT(  m_filesVersionNumber > 0  );

	

	CalcedNoteListExternal    cNoteExternal;
																									//   Input the PARMS   for the  CalcedNoteListExternal  object
		cNoteExternal.m_calcedNoteList       =     m_tempNoteList;

		cNoteExternal.m_lickList                  =     m_lickListPtr;                 //  can not be NULL

		cNoteExternal.m_totalSampleBytes  =     m_totalSrcFilesBytes;   
 
		cNoteExternal.m_sourceWAVpath    =     m_sourceFilesPath;  

		cNoteExternal.m_musicalKey         =      m_musicalKey;

		cNoteExternal.m_filesVersionNumber  =    m_filesVersionNumber;



//		cNoteExternal.m_projectFilesPath.Empty();    ***** BAD if empty, then notelist will NOT load to PitchScope2007   11/11   [ BEFORE:   now OK if empty,  had to change PitchScope load-code a little   3/11
//		cNoteExternal.m_projectFilesPath  =     "C:\\Unknown\\"    ;      **** FAILS, because PitchScope2007  will get the FileExtension wrong, and not let user BROWSE for the ProjkectFile




		cNoteExternal.m_projectFilesPath  =     "C:\\NoProject.ppj"    ;  //   SLOPPY , but acceptable.  It lets the user of PS2007 load the NoteList without a ProjectFile,
																									//   but if the user can not BROWSE and load a Valid Project, unless the Project's name was NoProject.ppj   11/11

//  *******************  POSSIBLY could make a DEFAULT ProjectName from the Navigator NoteList name ( ex:  GreyMare_DefaultProj.ppj  ),  BUT
//  *******************                  might be too confusing for user -- makes it sound like there is a project file, when there really insnt one.  This
//  *******************                  would only be for PitchScope2007 users who want to have a Project file for a Navigator notelist ????   9/9/2012



//	   cNoteExternal.m_projectFilesPath  =   "C:\\Users\\JamesM\\VoxSep\\Projects\\LatestTest\\GreyMare_Proj.ppj"  ;  //  





		cNoteExternal.m_midiInstrumentNumber  =   m_midiInstrumentPatch;

		cNoteExternal.m_startOffsetDetectZone  =     m_sampleIdxFirstRecordedNote;    // ******  HACK,  this give us a reasonable value  3/11

		cNoteExternal.m_endOffsetDetectZone   =     lastProcessedSampleIdx;      //   m_curAudioPlayer->m_prevEndSamplePlay;   // **** DELAY a problem here too ?????
															                                                        //  test:     totalSamples  -1;      




	void  *extraData  =   NULL;    //   *******  OK    Do I ever use this ?????    ******    3/11


	try
    {  CFile   file(    finalPath, 
	                       CFile::modeCreate   |  CFile::modeWrite      	//   |   CFile::shareExclusive 		   
					//	   |  CFile::typeBinary     ...BUGGY ????    5/02    Sets binary mode (used in derived classes only).
					  );


		if(    ! cNoteExternal.Emit_NoteList(   file,    extraData,   retErrorMesg  )     )  
		{
			m_bitSourceStreaming->m_recordNotesNow =   false;  
			return  false;
		}

	}
	catch(   CFileException   *pException   )
	{
		TCHAR    szCause[ 255 ];  
		CString   strRaw;
        pException->GetErrorMessage(  szCause,  255  );
		pException->Delete();
		strRaw =  szCause;
		retErrorMesg.Format(   "NoteGenerator::Save_Recorded_Notes_withDialog failed,   could not save List file because %s." ,    strRaw  );

		m_bitSourceStreaming->m_recordNotesNow =   false;  
		return  false;
	}


	m_bitSourceStreaming->m_recordNotesNow =   false;    //  Must always ASSIGN this  for the Bitsource from this function   3/11

	return  true;
}



											////////////////////////////////////////


bool    NoteGenerator::Merge_TempList_to_Master_Notelist(  CString&  retErrorMesg   ) 
{


	retErrorMesg.Empty();
																				

	if(   m_tempNoteList  ==  NULL   )      //  the TEMP-list has the newly recorded notes   ( Recorded notes go directly here,  then they can be saved to harddisk
	{
		retErrorMesg =   "NoteGenerator::Merge_TempList_to_Master_Notelist  FAILED,  m_tempNoteList is NULL. " ;
		return   false;
	}

	if(   m_masterNoteList  ==  NULL   )     //  These are the notes that we HEAR and SEE on the display.
	{
		retErrorMesg =   "NoteGenerator::Merge_TempList_to_Master_Notelist  FAILED,  m_masterNoteList is NULL. " ;
		return   false;
	}


	if(   m_tempNoteList->Count()  <  3   )
	{
		retErrorMesg =   "Record some notes.  (not enough notes were recorded)" ;   // also get this if not in Detection for  Mini-Source Mode  
		return   false;
	}

	
	
	long     retStartOffset = -1,   retEndOffset = -1,    retConcurentCount = -1,    retNoteCountAdded = -1; 
	short    retDeleteCount= -1;



	if(    ! SPitchCalc::Calc_StartOffset_EndOffset_for_NoteLists_Merge(    retStartOffset,   retEndOffset,    *m_tempNoteList,    retErrorMesg  )    )
		return  false;     //  Now excluding the FIRST and LAST detected-notes 




	if(    ! SPitchCalc::Delete_Notes_in_TimeSpan(   retStartOffset,   retEndOffset,      retDeleteCount,	  *m_masterNoteList,   *m_tempNoteList,   retErrorMesg  )   )
		return  false;    // Its OK for this to NOT delete anyting.  Just means that the new  notes' Zone  did not intersect any old notes.




	if(    ! SPitchCalc::Add_TempLists_Notes_to_Master_Notelist(   *m_tempNoteList,     *m_masterNoteList,      
		                                                                                          //    m_sampleIdxRecordingStart,     m_sampleIdxRecordingEnd, 	
																								         retStartOffset,                         retEndOffset, 
																										                              retNoteCountAdded,      retErrorMesg  )    )
		return  false;



	long  swapCount     =       SPitchCalc::Sort_NoteList_by_TimePosition(   *m_masterNoteList,   retErrorMesg  );  // since we blindly added the notes to the end of list  
	if(     swapCount  ==  0  )
	{
		int  dummy =  0;    //  Can easily happen,  if user's  Recorded-Notes  go into a  LATER Time-Zone  that is EMPTY of notes
	}

	


	if(    ! SPitchCalc::Count_Concurrent_Notes_in_List(   *m_masterNoteList,    retConcurentCount,    retErrorMesg   )    )   // check for error in my Algo.   1/21/2012
		AfxMessageBox(  retErrorMesg  );	




	long  totalNotesInMasterList  =    m_masterNoteList->Count();

	long  totalNotesInTempList  =    m_tempNoteList->Count();



	m_masterNoteList->m_lastFoundCalcNoteLink  =  NULL;    //  Reset the CACHE.   BIG,  the next play command will have to do a FULL Notelist search.    1/23/2012





	TRACE(  "MERGE is completed.  [ NoteCOUNT in TempList   %d  ]    [  %d notes deleted  ]      [ %d  Notes added  ]     [ Master %d notes ]    \n" ,   
														 totalNotesInTempList,          retDeleteCount,    retNoteCountAdded,   totalNotesInMasterList    );




	if(   retConcurentCount  >=  1   )    // ********************   TEMP,   DEBUG **************** 
	{

		CString  mesg;
		mesg.Format(    "*** MERGE PROBLEM:   Found   %d  CONCURRENT Notes." ,    retConcurentCount   );

		AfxMessageBox(  mesg  );   // ********  OMIT for RELEASE ****************
	}


	//  *******  Should I automatically switch to  NoteList-Mode  for Midi-Source  ??????    1/12  *********


	return  true;
}



											////////////////////////////////////////


bool    NoteGenerator::Ask_User_Merge_or_Save_New_Notes(   long  lastProcessedSampleIdx,    bool&  retUserDidMerge,     CString&  retErrorMesg   ) 
{


	retErrorMesg.Empty();
	retUserDidMerge =  false;


	if(   m_tempNoteList  ==   NULL  )
	{
		retErrorMesg =   "NoteGenerator::Ask_User_Merge_or_Save_New_Notes  FAILED,   m_tempNoteList  is  NULL."  ;
		return  false;
	}


	if(   m_masterNoteList  ==   NULL  )
	{
		retErrorMesg =   "NoteGenerator::Ask_User_Merge_or_Save_New_Notes  FAILED,   m_masterNoteList  is  NULL."  ;
		return  false;
	}

	ASSERT(  m_bitSourceStreaming  !=  NULL  );




	long   noteCount    =      m_tempNoteList->Count();
	if(      noteCount  <  3   )
	{
		retErrorMesg =    "Not enough notes were recorded during your Recording Session. Try again to record more notes."  ;
		return  false;
	}


	
	int   reslt  =    AfxMessageBox(   "Do you want to MERGE the Recorded Notes to the Current NoteList? \n\n(   YES:  Merge,   NO:  Save notes to a new File   )",   
																														MB_YESNOCANCEL | MB_ICONQUESTION     );    
	if(   reslt  ==   IDYES   )
	{

		Begin_Wait_Cursor_GLB();


		if(     ! Merge_TempList_to_Master_Notelist(  retErrorMesg  )     )
		{

			m_bitSourceStreaming->m_recordNotesNow =   false;

			End_Wait_Cursor_GLB();
			return  false;
		}

		End_Wait_Cursor_GLB();


		retUserDidMerge =   true;   // tell calling function of this action
	}

	else if(   reslt  ==   IDNO   )  
	{

		if(    ! Save_Recorded_Notes_withDialog(   lastProcessedSampleIdx,   retErrorMesg  )     )   
		{
			m_bitSourceStreaming->m_recordNotesNow =   false;
			return  false;
		}
	}
	else if(    reslt  ==   IDCANCEL   )  
	{  
		int  dummy =   9;
	}

	return  true;
}


