/////////////////////////////////////////////////////////////////////////////
//
//  CalcedNoteExternal.cpp   -   
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



#include   "..\comnFacade\UniEditorAppsGlobals.h"

#include   "..\comnFacade\VoxAppsGlobals.h"



//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"    

#include  "..\ComnGrafix\CommonGrafix.h"    
#include  "..\ComnGrafix\OffMap.h"  
#include  "..\comnGrafix\TransformMap.h"

#include "..\comnInterface\DetectorAbstracts.h"    // **** NEW ****



#include   "..\ComnAudio\CalcNote.h"
#include   "..\ComnAudio\SPitchCalc.h"




#include "External.h"

#include "CalcedNoteExternal.h"

/////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


CalcedNoteListExternal::CalcedNoteListExternal()
{


	m_calcedNoteList =   NULL;

	m_lickList =  NULL;

	m_totalSampleBytes =  -1;

	m_sourceWAVpath.Empty();

	m_projectFilesPath.Empty();


	m_startOffsetDetectZone  =   -1;
	m_endOffsetDetectZone   =   -1;


	m_midiInstrumentNumber =  -5;

	m_musicalKey =   0;



	m_filesVersionNumber =   -1;

//	m_exesVersionNumber  =   -1;

	m_loadedFilesVersionNumber =   -1;   
}


											////////////////////////////////////////


CalcedNoteListExternal::~CalcedNoteListExternal()
{
}



											////////////////////////////////////////


bool    CalcedNoteListExternal::Emit_NoteList(   CFile&  file,    void*  extraData,     CString&  retErrorMesg   )
{



//	unsigned char  version =   12;    //   12 is for  Navigator 1.0  and  PitchScope 2.0(Scriber)  FAMILY of new apps (Player.exe)    2/12


	unsigned char  version =    m_filesVersionNumber ;       //   12                NOT  m_exesVersionNumber  !!!

	ASSERT(  version  > 0   );

			//  Alway keep FILE versions 8 digits lower than the current EXE version[ 10 ],  to that I can release
	        //  CORRECTED  File-VERSIONS upward without mandating a release of a new EXE( with a higher version )   1/06

			//  ***  Navigator will be under  PitchScope 2.0  [ 20 ],   so  my new FILE-version NUMBER is   "1.2:  [ 12 ]   2/2012  *** 




	short   detectZoneSavedCount  =   1;    //  can I adjutst this to  0 ???   Why would I want top

	

	retErrorMesg.Empty();

	if(   m_calcedNoteList  ==   NULL   )  
	{
		retErrorMesg  =    "CalcedNoteListExternal::Emit_NoteList  FAILED,  m_calcedNoteList  is  NULL. " ;
		return false;
	}



	long  retSizeFilePos; 
																	    //  A)  Identifies this file as a   CreativeDetector's file   
	Write_FileCreator_Tag(   file,    version  );       



																		//   B)  Write the   File's Chunk   TAG


	if(   ! Write_Chunks_Header(   file,    NOTELISTfILEhEADERcHK,    version,    retSizeFilePos,    retErrorMesg  )    )
		return  false;						   //    NOTELISTfILEhEADERcHKPlayer :   the 'HEADER' Chunk  of the  FILE



	//////////////////////////   ( first gather the Header data )

	SPitchListHEADER   headerStruct;   // ***** WANT some INITIALIZATION here ???  ...too much happens in   Save_SixteenBit_SoundSample() *****  JPM  5/02 
	

//	ListMemry< ScalepitchSubject >&    spitchList    =    ( ListMemry<ScalepitchSubject >& )(  pitchDetApp.Get_DST_ScalepitchList().Get_ChildSubjects_List()   );    

	headerStruct.sampRate            =      44100L;				 //   ****INSTALL,    read from  WAVEFORMATEX

	headerStruct.samplesPerChunk =     512;   //   soundMan.m_chunkSize;      //   is really   'SAMPLES per Chunk'

	headerStruct.totalSampleBytes  =     m_totalSampleBytes;     //    soundMan.Bitsource_Sample_Count()    *   pitchDetApp.Get_BytesPer_Sample();	  

	headerStruct.sampleScalePercent  =    244;     //   Get_PitchDetectorApp().m_sampleScalePercent;
			  


								//  Save the CURRENT  stereoChannel of focus, for when the list is next loaded

	headerStruct.detectedNotesChannel  =    2;     //  Get_PitchDetectorApp().m_stereoChannelCode;

	headerStruct.numNotesBoth           =      m_calcedNoteList->Count();      //   pitchList.Count();    //  only one list now

	headerStruct.notemapHeight           =     FUNDAMENTALTEMPLATEoCTAVECOUNT   *  12;    //    FUNDtEMPLATEmapHEIT;
	headerStruct.notemapMidiStartPitch =     kMIDIsTARTnOTE;   //   Midi 52( Guitar's low E )   ...is used to figure out 'RELATIVE' OctaveIdx assignments in OutputLists 





	if(    ! m_sourceWAVpath.IsEmpty()      )     //   ! pitchDetApp.Get_Source_Files_Path().IsEmpty()    ) "C:\\Users\\JamesM\\VoxSep\\Projects\\LatestTest\\BWoman1.wav"
	{	  
		int      len  =    m_sourceWAVpath.GetLength();
		char  *ptr  =    m_sourceWAVpath.GetBuffer( 0 );

		strcpy(    headerStruct.srcWavFilesPath,     ptr   );     // directly access CString buffer

		headerStruct.srcWavFilesPath[  len  ]   =    '\0' ;    //  ???? Do I need to  TERMINATE here ????
	}
	else
	{  retErrorMesg   =  "CalcedNoteListExternal::Emit_NoteList   failed,  source file's path is empty."  ;
		return  false;	
	}




	CString&  projectFilePath  =    m_projectFilesPath;    //  pitchDetApp.Get_Project_Files_Path();    "C:\\Users\\JamesM\\VoxSep\\Projects\\LatestTest\\BlackMagic.ppj"	

	if(     ! projectFilePath.IsEmpty()      )     
	{	  
		int      len  =    projectFilePath.GetLength();
		char  *ptr  =    projectFilePath.GetBuffer( 0 );

		strcpy(    headerStruct.projectFilesPath,     ptr   );     // directly access CString buffer

		headerStruct.projectFilesPath[  len  ]   =    '\0' ;    //  ???? Do I need to  TERMINATE here ????
	}
	else
	{  //  retErrorMesg   =  "CalcedNoteListExternal::Emit_NoteList  failed,  project file's path is empty." ;
		//   return  false;	
		headerStruct.projectFilesPath[  0  ]   =    '\0' ;    //  will this seal off the string ???
	}



//	ListMemry< DetectZone >&	 leftListDZone   =        pitchDetApp.Get_ChannelDetectionSubject(  TransformMap:: LEFTj  ).Get_DetectZones_MemList();
//	ListMemry< DetectZone >&	 rightListDZone =        pitchDetApp.Get_ChannelDetectionSubject(  TransformMap:: RIGHTj  ).Get_DetectZones_MemList();
//	ListMemry< DetectZone >&	 centerListDZone =      pitchDetApp.Get_ChannelDetectionSubject(  TransformMap:: CENTEREDj  ).Get_DetectZones_MemList();
	headerStruct.detectZoneCount  =   detectZoneSavedCount;            //      Only 1 for Navigator   



	headerStruct.midiInstrument   =   m_midiInstrumentNumber;   //   68  or 41  soundMan.m_midiInstrumentPatch;  *************  FIX,  should pass in *************




	//   (  OLD and wrong.  Must use OLD PitchScopes file format. )  	    headerStruct.musicalKey  =   	m_musicalKey;   

	CString    retMKeysName;

	headerStruct.isMinorKey   =   0;    // **************  ALWAYS ???  ******************************


	short  fileCodeMKey        =     SPitchCalc::Calc_FileCode_from_MusicalKey(   m_musicalKey,   retMKeysName  );

	headerStruct.musicalKey =   	fileCodeMKey;   





	headerStruct.midiTimingDelay  =   18;     //    18   ****  WANT/Use   this for  Navigator ?????   ***********

	/////////////////////////




	long   ldSize  =    ( long )sizeof(  SPitchListHEADER           );     //   Finish writing the  FILE's HEADER 

	file.Write(   &headerStruct,    ldSize  );


	Write_Chunks_Length(   file,   retSizeFilePos   );   //  Go back to the TAG-location( files HeaderStruct) ,  and  write the length to the  'just-saved'  tag




																			//	C)  save  'DetectZones'

	short  saveDFTs =  0;   //  no DFTs for the Notelist

//	ListComponentSubject&	 dZoneSubjectLeft      =    Get_PitchDetectorApp().Get_ChannelDetectionSubject(  TransformMap:: LEFTj        ).Get_DetectZone_ListSubject();
//	ListComponentSubject&	 dZoneSubjectRight    =    Get_PitchDetectorApp().Get_ChannelDetectionSubject(  TransformMap:: RIGHTj      ).Get_DetectZone_ListSubject();
//	ListComponentSubject&	 dZoneSubjectCenter  =    Get_PitchDetectorApp().Get_ChannelDetectionSubject(  TransformMap:: CENTEREDj  ).Get_DetectZone_ListSubject();
	/***
	if(         ! Save_ObjectList_Chunk(   DZONELISTchkLEFTplayer,        version,    dZoneSubjectLeft,     file,   saveDFTs,   retErrorMesg   )  
		  ||   ! Save_ObjectList_Chunk(   DZONELISTchkRIGHTplayer,      version,    dZoneSubjectRight,     file,   saveDFTs,  retErrorMesg   )  
		  ||   ! Save_ObjectList_Chunk(   DZONELISTchkCENTERplayer,    version,    dZoneSubjectCenter,     file,   saveDFTs,   retErrorMesg   )    )
		return  false;
	***/


					//  Now saving a DUMMY STRIPPED down DetectZone for  DZONELISTchkCENTERplayer

	if(     ! Save_ObjectList_Chunk(    DZONELISTchkLEFT,        version,        file,   saveDFTs,             0,                        retErrorMesg   )    )  // ****  NEED this ???? *****
		return  false;

	if(    ! Save_ObjectList_Chunk(    DZONELISTchkRIGHT,       version,       file,   saveDFTs,             0,                        retErrorMesg   )    )
		return  false;

	if(     ! Save_ObjectList_Chunk(   DZONELISTchkCENTER,    version,        file,   saveDFTs,   detectZoneSavedCount,   retErrorMesg   )    )
		return  false;






																			//   D)  save  Scalepitch LIST

	if(   ! Save_ObjectList_Chunk(   SpITCHLISTcHK,    version,    file,   0,   0,   retErrorMesg   )     )    //  Sames as Above ????
		return  false;



																			//   E)  save  LICK list

	if(   ! Save_ObjectList_Chunk(   LICKlISTcHK,        version,    file,   0,   0,   retErrorMesg   )     )    //  Sames as Above ????
		return  false;



	return   true;
}




											////////////////////////////////////////


bool     CalcedNoteListExternal::Receive(   CFile&  file,    CString&  retErrorMesg   )
{


	long   bytesPerSample =   4;    //  always 4  ???????????????????  *******************************


	retErrorMesg.Empty();


	if(    m_calcedNoteList  ==  NULL    ||    m_lickList == NULL   )
	{
		retErrorMesg  =    "CalcedNoteListExternal::Receive FAILED,   CalcedNoteListExternal was not completely initialized."  ;
		return  false;
	}



	unsigned char   retVersion;									//	A)    load  the   File-CREATOR	 TAG
	long                 retChunksSize;  

	if(    ! Verify_FileCreator_Tag(   file,   retVersion,   retErrorMesg  )     )
		return  false;



																			//	B)    go to the  File header TAG

	if(   ! Goto_Chunks_Header(   file,    NOTELISTfILEhEADERcHK,   retVersion,   retChunksSize,   retErrorMesg  )    )
		return  false;      //  it looks up the  'TAG's  identifying string'  (  "NtLstFil" for  NOTELISTfILEhEADERcHKPlayer ),  and descends into the File
								 //  until that STRING is found in the file.


	m_loadedFilesVersionNumber  =     (short)retVersion;   //    '12'  for Navigator,     '2'  for OLD PitchScope


 

																			//	C)    load  the   'List-HEADER'

	SPitchListHEADER   dstListHeaderStruct;


	long  ldSize  =    ( long )sizeof(  SPitchListHEADER  );       //  Why didn't I use  'retChunksSize'    ...or at least do a verification

	ASSERT(  ldSize  ==   retChunksSize   );    //  Does this ever get HIT ????     NO.


	file.Read(   &dstListHeaderStruct,    ldSize  );




	CString	  headersWAVpath =     dstListHeaderStruct.srcWavFilesPath;  
	if(    headersWAVpath.IsEmpty()    )
	{
		retErrorMesg  =  "CalcedNoteListExternal::Receive  failed,  source .WAV filename is empty." ;
		return  false;
	}

	CString   headersProjectPath   =    dstListHeaderStruct.projectFilesPath;  


//	pitchDetApp.m_sampleScalePercent  =   dstListHeaderStruct.sampleScalePercent;  **** CAN I use this for the "Source Boost"  ????  12/11  *************





	 short  retSharpCount= -1,    retFlatCount = -1,    retScalePitch = -1;   
	 bool   retIsMinor; 


//	m_musicalKey  =   dstListHeaderStruct.musicalKey;   **** BAD, is not the Format from OLD PitchScope   1/12

	SPitchCalc::Get_Musical_Key_from_FileCode(   dstListHeaderStruct.musicalKey,    retSharpCount,    retFlatCount,   retScalePitch,    retIsMinor   );


	ASSERT(   retScalePitch  >= 0     &&    retScalePitch  <= 11   );

	m_musicalKey  =   retScalePitch;







																//  ...make SOME assignments to 'system' based on  ProjectHEADERpScope's state

	ASSERT(   dstListHeaderStruct.samplesPerChunk   );


	long   totalSamples    =      dstListHeaderStruct.totalSampleBytes   /  bytesPerSample;    //  pitchDetApp.Get_BytesPer_Sample();

	long   virtPixelsWidth  =      totalSamples  /   dstListHeaderStruct.samplesPerChunk;     //   is really   'SAMPLES per Chunk'



//	Get_PitchDetectorApp().Get_DST_ScalepitchList().m_virtualPixelCount    =      virtPixelsWidth;  //  do this BEFORE  Load_DST_SPitchList()


	/****
	pitchDetApp.m_musicalKey  =      dstListHeaderStruct.musicalKey;

	if(    dstListHeaderStruct.isMinorKey  ==  0    )
		pitchDetApp.m_isMinorKey =   false;
	else
		pitchDetApp.m_isMinorKey =   true;
	****/


//	soundMan.m_animationSamplesDelay =    (long)(  dstListHeaderStruct.midiTimingDelay  ); 


	/****
	if(   soundMan.m_midiSequencer  !=  NULL   )
	{
		soundMan.m_midiInstrumentPatch  =    dstListHeaderStruct.midiInstrument;

		if(    ! soundMan.Change_Midi_Instrument(   soundMan.m_midiInstrumentPatch,   retErrorMesg   )    )
			AfxMessageBox(  retErrorMesg  );	
	}
	else
		AfxMessageBox(   "CalcedNoteListExternal::Receive:  Could NOT set the Midi Instrument Patch because a Midi DEVICE has not been established."   );	
	****/




										    //       Now that we have verified the file,  ERASE the PREVIOUS  list and its ComponentViews (  Load_ProjectSubject()  might have done this )


	if(    m_calcedNoteList  !=   NULL   )   //  Clear out the list for new  CalcedNote.
		m_calcedNoteList->Empty();
	else
	{	ASSERT( 0 );   }


	if(    m_lickList  !=   NULL   )              //  Clear out the list for new  LICKs.
		m_lickList->Empty();
	else
	{	ASSERT( 0 );   }





																			//	 E)   load  'DetectZones'   ( BUT I do not use them in Navigator )



																			//  Then we need to load the MINI-DZones( no dft ) from the 
		/*****
		//	bool   retSRCfilePathWasChanged =   false;

		ListComponentSubject&	 dZoneSubjectLeft      =    Get_PitchDetectorApp().Get_ChannelDetectionSubject(  TransformMap:: LEFTj          ).Get_DetectZone_ListSubject();
		ListComponentSubject&	 dZoneSubjectRight    =    Get_PitchDetectorApp().Get_ChannelDetectionSubject(  TransformMap:: RIGHTj        ).Get_DetectZone_ListSubject();
		ListComponentSubject&	 dZoneSubjectCenter  =    Get_PitchDetectorApp().Get_ChannelDetectionSubject(  TransformMap:: CENTEREDj  ).Get_DetectZone_ListSubject();
		if(       ! Load_ObjectList_Chunk(   DZONELISTchkLEFT,        retVersion,   dZoneSubjectLeft,   *listObjectExternal,   file,    retErrorMesg  )    
			||   ! Load_ObjectList_Chunk(   DZONELISTchkRIGHT,      retVersion,   dZoneSubjectRight,   *listObjectExternal,   file,    retErrorMesg  ) 
			||   ! Load_ObjectList_Chunk(   DZONELISTchkCENTER,    retVersion,   dZoneSubjectCenter,   *listObjectExternal,   file,    retErrorMesg  )    )
		{
			delete  listObjectExternal;
			return  false;
		}
			
		delete  listObjectExternal;

		pitchDetApp.Get_ChannelDetectionSubject(  TransformMap:: LEFTj          ).Sort_DetectZone_List_by_TimePosition();  
		pitchDetApp.Get_ChannelDetectionSubject(  TransformMap:: RIGHTj        ).Sort_DetectZone_List_by_TimePosition();  
		pitchDetApp.Get_ChannelDetectionSubject(  TransformMap:: CENTEREDj  ).Sort_DetectZone_List_by_TimePosition();  
		*****/																											//  since there are no DFTs,  ther are NO Derivative transforms to build


//		if(    ! Get_DetectorApp().Do_Sources_PostLoad_Calcs(   retErrorMesg   )     )   //  Build the Amplitude array...  etc.
//			return  false;


//		if(   retSRCfilePathWasChanged  )   
//			Get_UniEditorApp().Set_DSTlist_Modified(  true  );  //  Unlike RegionDetectorApp,  the NotelistFile also has a string to the   SRC file.






	m_sourceWAVpath  =      headersWAVpath;    // *******  OK to assign here ????    Need this to load wav file in  PitchPlayerApp::Load_NoteList()



																	//    F)   load  the ScalePitches


	if(    ! Load_ObjectList_Chunk(   SpITCHLISTcHK,   retVersion,  file,    retErrorMesg  )     ) 
		return  false;


																	//    G)   load  the  Licks


	if(    ! Load_ObjectList_Chunk(   LICKlISTcHK,    retVersion,  file,    retErrorMesg  )     )  // OLD PitchScope file will NOT have this tag, so must not fail
	{

		TRACE(   "\n\n Loaded FILE does NOT have a LICK-LIST.  It is probably from PitchScope 2007.  Is NOT a problem.\n\n"  );


		retErrorMesg.Empty();    //  Do NOT pass this  NON-CRITICAL error message to the calling function.  2/12
				

		//   return  false;   ****  NO!!!!   It is OK if there is NOT a LickList,  File could be from PitchScope 2007    *************
	}




	/***************************   INSTALL this sort   12/11

	long  swapCount   =     Get_PitchDetectorApp().Get_DST_ScalepitchList().Sort_List_by_TimePosition( retErrorMesg );       // ***** * NEED this ????  ************
	if(     swapCount < 0   )
		return   false;

	if(     ! pitchDetApp.Create_All_AnimeMasks(  retErrorMesg  )     )
		return  false;
	****/


					  

//	bool   retWasChanged;                   // think we need to do this AFTER  Maybe_Load_ObjLists_ProjectFile()  because it inits this value in Init_New_Project()   5/07

//	Get_PitchDetectorApp().Set_Focused_StereoChannel(    dstListHeaderStruct.detectedNotesChannel,     retWasChanged  ); 

																																				//  Need to have it do this to    RESET   some of the    panes's MEMBERVARS
//	if(     ! Get_UniEditorApp().Change_StereoChannel_All_Viewjs(  dstListHeaderStruct.detectedNotesChannel,   retErrorMesg )     )
//		return  false;    //AfxMessageBox(  retErrorMesg  );	


//	Get_UniEditorApp().Notify_All_ComponentViews_forReDraw();

	return  true;
}




											////////////////////////////////////////


void	CalcedNoteListExternal::Assign_FileObject_ScalePitch(   SPitchFileObj&  fileObj,     MidiNote&  midiNote,    bool   savingOldPitchScope2007File   )															 															 
{


	short  stereoChannelCode =  2;    // ************  ALWAYS ???  ******************************


	/****
	char	 stereoChannel;     //  0:  left,    1:  Right,    3:  Center( L+R ) 
	
	long      startOffset;  
	long      endOffset;   

	short     channelIdx;              //    scalePITCH  value  [ 0-11 ]       0: E      11:  Eb

	short     chosenFundamentalCandIdx;			//    a)   OCTAVE and  Fundamental    Measurements

	short     editedFundamentalCandIdx;  

	short     scoreDetectionAvgTone;			   //	 b)   Region-DETECTION      Measurement  ( from YMax )

	long      octaveCandidScores[  NumOCTAVEcandSCORESfilePlayer   ];   //  only need four,   keep for   OctaveCompair Dialog 
	***/


	fileObj.startOffset  =     midiNote.beginingSampleIdxFile;     //   startOffset;  
	fileObj.endOffset   =     midiNote.endingSampleIdxFile;       //    endOffset;   

	fileObj.channelIdx  =    midiNote.scalePitch;     // scalePitch;          

	fileObj.chosenFundamentalCandIdx  =     midiNote.octaveIndex;   //     fundamentalCandIdx;   	




// *********************************************************************************************************************

	if(    savingOldPitchScope2007File    )
	{

	ASSERT( 0 );    // Will this ever GET USED ????     9/2012


		fileObj.scoreDetectionAvgTone       =     midiNote.detectScoreHarms;


		fileObj.editedFundamentalCandIdx  =     midiNote.detectAvgHarmonicMag;   //  ******  REMOVE  ????   *******   Works with OLD PitchScope, think it would only give trouble to
																												   //   VoxSep.exe,  because this was a DEBUG-Only MemberVar.    2/12
	}
	else
	{                        //   NAVIGATOR :   NEED save with higher numbers,  to be compatible with OLD PitchScope  { 100  -  225   )

		double   detectScoresMult  =   kDETECTscoresFILECompat;


		short   adjustedHarmMag  =   (short)(     (double)(  midiNote.detectAvgHarmonicMag  )    *  detectScoresMult   );   // 2.5   works good.  9/2012 

		if(    adjustedHarmMag  >  255    )
			adjustedHarmMag =  255;     //  *****  Would have a BIG BUG in OLD-PitchScope if this is NOT adjusted this way ( note is too dark ).    9/6/2012  ******


		fileObj.scoreDetectionAvgTone  =    adjustedHarmMag;     //    Notice that  OLD-PitchScope determines its  Note's BRITENESS   by the  DetectScore, 
																						    //    and NOT the AverageHarmonicMagnitude  like  Navigator and Player.   9/6/2012



		fileObj.editedFundamentalCandIdx =     midiNote.detectAvgHarmonicMag;     //   Works with OLD-PitchScope, but think it would only give trouble to
																												       //    VoxSep.exe,  because this was a DEBUG-Only MemberVar.    2/2012
																													   //	
																													   //   ( if really want in VoxSep, have Vox look at VersionNumber and adjust for
	}                                                                                                                 //      Navigator new function for  '.editedFundamentalCandIdx '    9/2012
// *********************************************************************************************************************



	fileObj.stereoChannel      =  	(char)stereoChannelCode;   //  2     ALWAYS ???    12/2011




																//		First initialize all,  then copy in REAL Values

	for(    short i =0;     i <  NumOCTAVEcandSCORESfile;   i++   )    //   8       Actually 8 in the Array for SPitchFileObj, but we only use the First 4 Entries,
		fileObj.octaveCandidScores[ i ]   =   -30;                                 //           in both  PitchScope 2007 and Navigator.   2/12




	for(    short oct =0;    oct <  kCountOfNavOctaveCandidates;    oct++   )     //    4   =   kCountOfNavOctaveCandidates
	{
		fileObj.octaveCandidScores[  oct  ] 	 =	    midiNote.detectScoreOctaveCandids[  oct  ];
	}





	fileObj.extraDataCode      =  0;      //  Initialize to NO extradata  (  extraDataCode =  0   )
	fileObj.extraDataByteSize =  0;    
}




											////////////////////////////////////////


void	 CalcedNoteListExternal::AssignMe_from_FileObject_ScalePitch(   SPitchFileObj&   fileObj,     MidiNote&  nuCalcNote,   bool  oldPitchScopeFile    )
{
	

	if(   m_loadedFilesVersionNumber  <= 0  )
	{
		ASSERT( 0 );
		AfxMessageBox(   "AssignMe_from_FileObject_ScalePitch  FAILED,  m_loadedFilesVersionNumber was NOT assigned."  );
	}


	double detectScoresMult  =   kDETECTscoresFILECompat;


	nuCalcNote.beginingSampleIdxFile   =      fileObj.startOffset;  
	nuCalcNote.endingSampleIdxFile     =       fileObj.endOffset;   

	nuCalcNote.scalePitch      =     fileObj.channelIdx;          

	nuCalcNote.octaveIndex  =     fileObj.chosenFundamentalCandIdx;   	



//  ****************************************************************

	if(    oldPitchScopeFile   )
	{


		short   adjHarmMagScore   =    (short)(       (double)(  fileObj.scoreDetectionAvgTone )   /   detectScoresMult    );  //  AvgHarmMag scores are much SMALLER than DetectScores

		nuCalcNote.detectAvgHarmonicMag  =    adjHarmMagScore;     //   .detectAvgHarmonicMag :    this controls the  BRITENESS  of the note or bullet in Navigator



		nuCalcNote.detectScoreHarms  =     fileObj.scoreDetectionAvgTone;  	



		for(    short oct =0;    oct <  kCountOfNavOctaveCandidates;    oct++   )    //  4 :     just 4 of the 8 in a  fileObj    ****** BUG for OldPitchScope
		{

			long   fileValue =    fileObj.octaveCandidScores[  oct  ]; 
			if(      fileValue  <  0   )                                             //   PitchScope2007  sometimes has large negative scores  (  -3000  )
				fileValue =   0;

			nuCalcNote.detectScoreOctaveCandids[  oct  ]  =   fileValue;      //  Is OK...  INDEXES are in sync with Old PitchScope Files   2/2012
		}

	}
	else
	{	nuCalcNote.detectAvgHarmonicMag =    fileObj.editedFundamentalCandIdx;    //    CURRENT   ( yes,  this works with OLD PitchScope )   9/2012



		short     adjDetScore  =     (short)(      (double)(  fileObj.scoreDetectionAvgTone  )   /   detectScoresMult   );    //   REDUCE the boosted score to be compitible with Navigator

		nuCalcNote.detectScoreHarms   =    adjDetScore;  



		for(    short oct =0;    oct <  kCountOfNavOctaveCandidates;    oct++   )    //  4 :     just 4 of the 8 in a  fileObj    ****** BUG for OldPitchScope
		{
			nuCalcNote.detectScoreOctaveCandids[  oct  ]  =    fileObj.octaveCandidScores[  oct  ];  //  Is OK...  INDEXES are in sync with Old PitchScope Files   2/2012
		}
	}

// *****************************************************************


	int   dummy  =  9;
}



											////////////////////////////////////////


void		CalcedNoteListExternal::Assign_FileObject_DetectZone(   DetectZoneFileObj&   fileObj,    short  channelCode,      long   startOffset,  long   endOffset   )  
{


	fileObj.stereoChannel =     (char)channelCode;


	fileObj.startOffset  =     startOffset;   
	fileObj.endOffset    =     endOffset;   


	fileObj.m_samplesPerBeat             =    -1;    //  m_samplesPerBeat;	
	fileObj.originalOffsetForBeat           =     0;    //  m_originalOffsetForBeat;   
	fileObj.userOffsetForBeat				=    0;    //   m_userOffsetForBeat;
	fileObj.m_firstRestBeatsInFirstBar      =    0;   // m_firstRestBeatsInFirstBar;						
	fileObj.m_beatsInOneBar         =    4;   //  m_beatsInOneBar;            
	fileObj.m_beatsPerQuarterNote =   1;   //   m_beatsInOneBar;    *******  WRONG in PitchScope should be 1 *******   


	fileObj.m_logDFTtotalBytes  =   -1;    //     totalDFTbytes;   //  if -1,      then a DFT is NOT supposed to be saved
	fileObj.m_DFTsBits	            =    NULL;    //   logDFTsBits;      //  if NULL,  then a DFT is NOT supposed to be saved ( thes 2 vars together define a proper NotSaved )
	fileObj.m_bytesInRow    =   -1;    //  bytesInRow;
	fileObj.m_logDFTwidth    =   -1;    //   dftMapWidth;  //  calculation is wierd.  see  TransMapAdminRelease::Alloc_Read_logDFT_from_Files_Segment()  5/06



    /****   OLD,  from PitchScope2007   ( do I use these values at all ???  )

	fileObj.m_topFrequencyLimit       =    4700;    // m_topFrequencyLimit;					//  Save the filtering parms used on the 8 bit sample for later CORRELATION 
	fileObj.m_bottomFrequencyLimit  =     82;      //  m_bottomFrequencyLimit;       // *****************  INSTALL *****************
	***/
	fileObj.m_topFrequencyLimit       =     19500;        //  m_topFrequencyLimit;		    Navigator ALWAYS uses this for the top	  9/12 		
	fileObj.m_bottomFrequencyLimit  =          20;      //  m_bottomFrequencyLimit;       Bottom for  FULL-Filter  in Navigator   [ only really used in FundCandidEvaluator() 




	fileObj.m_volumeAdjPercent       =    100;      //  m_volumeAdjPercent;    // **********  INSTALL   ...or is this OK??    *****************


	fileObj.m_dftNumberOfPeriods  =    18;   //   m_dftNumberOfPeriods;  
	fileObj.m_dftsMidiStartBand      =    52;    //    m_dftsMidiStartBand;     
	fileObj.m_dftHeight                   =   76;    //   m_dftHeight;                  


	fileObj.m_filterCodeDFTmapReads   =   2;   //    m_filterCodeDFTmapReads;    
	fileObj.m_filterWidthDFTmapReads  =    6;    //   m_filterWidthDFTmapReads;    ******** SHOULD my runtime detector do this ???  3/11   ********************
	fileObj.m_pixThresholdHPairs          =    15;   // m_pixThresholdHPairs;  



	fileObj.extraDataCode      =  0;      // Show that NO extra data is present    7/07
	fileObj.extraDataByteSize =  0;    
}



											////////////////////////////////////////


void    CalcedNoteListExternal::Assign_FileObject_Lick(    LickFileObj&   fileObj,    long  startSample,  long  endSample,    CString&  nickName    )  
{


	fileObj.startSample  =     startSample;   

	fileObj.endSample   =     endSample;   


	if(     ! nickName.IsEmpty()      )     
	{	  
		char  *ptr  =    nickName.GetBuffer( 0 );
		int      len  =    nickName.GetLength();


		if(   len  >   MAXcharsInLICKnAME    ||    len <= 0  )
		{   ASSERT( 0 );  }    // Is it possible ???     NO...  5/2012


//		strcpy(    headerStruct.projectFilesPath,     ptr   );     // directly access CString buffer
		strcpy(    fileObj.nickName,   ptr  );


		fileObj.nickName[  len  ]  =    '\0' ;    
	}
	else
	{  //  retErrorMesg   =  "CalcedNoteListExternal::Emit_NoteList  failed,  project file's path is empty." ;
		//   return  false;	

		fileObj.nickName[  0  ]  =    '\0' ;    
	}
}


											////////////////////////////////////////


void	 CalcedNoteListExternal::AssignMe_from_FileObject_Lick(   LickFileObj&   fileObj,     Lick&  nuLick    )
{

	nuLick.m_startSampleVirt  =      fileObj.startSample;  
	nuLick.m_endSampleVirt    =     fileObj.endSample;   

	nuLick.m_nickName      =     fileObj.nickName;    // OK to assign a CString this way
}



											////////////////////////////////////////


bool    CalcedNoteListExternal::Save_ObjectList_Chunk(    short  chunkCode,    unsigned char  version,          // ComponentSubject&	unknownSubject,  
												                                         CFile&  file,    short  specialCode,    short detectZoneCount,     CString&  retErrorMesg    )   
{							

							          //   if(   specialCode ==  NULL  ),   then nothing special is done in Emit()

	retErrorMesg.Empty();





// ***************************   CAREFULL  *************************************

	bool   savingOldPitchScope2007File =   false;

	if(    version   <=  10   )                 //  Navigator and PitchPlayer   file versions are  "12",    OLD PitchScope are "10"     2/2012
	{
		savingOldPitchScope2007File =    true;
	}

//  *****************************************************************








	if(          chunkCode  !=  SpITCHLISTcHK     &&     chunkCode  != LICKlISTcHK  
		 &&    chunkCode  !=  DZONELISTchkLEFT  &&   chunkCode  !=  DZONELISTchkRIGHT &&   chunkCode  !=  DZONELISTchkCENTER   )
	{
		retErrorMesg  =  "CalcedNoteListExternal::Save_ObjectList_Chunk FAILED,   missing case for  'chunkCode'. " ;
		return  false;
	}


	short   channelCodeStereo  =  2;      //   2 :  CENTER  StereoCode,     default     ONLY need this for DetectZones

	if(           chunkCode  ==   DZONELISTchkLEFT    ) 
		channelCodeStereo =   0; 
	else if(    chunkCode  ==   DZONELISTchkRIGHT    ) 
		channelCodeStereo =   1; 




	long   numberOfListObjects =    0;  
	long   retSizeFilePos   =   -1;  


	if(   chunkCode  ==  DZONELISTchkLEFT  ||   chunkCode  ==  DZONELISTchkRIGHT ||  chunkCode  ==  DZONELISTchkCENTER  )  
	{
		if(        detectZoneCount  ==  1   )
			numberOfListObjects =   1;
		else if(  detectZoneCount  ==  0   ) 
			numberOfListObjects =   0; 
		else
		{  ASSERT( 0 );    //  3/11     Only should save one DetectZone from Navigator
			numberOfListObjects =   0; 
		}
	}
	else if(  chunkCode  ==  SpITCHLISTcHK  )
	{  
		if(   m_calcedNoteList  ==  NULL   )
		{
			retErrorMesg =     "CalcedNoteListExternal::Save_ObjectList_Chunk  failed,   m_calcedNoteList  is NULL ."   ;
			return  false;
		}

		numberOfListObjects  =     m_calcedNoteList->Count();
	}
	else if(  chunkCode  ==  LICKlISTcHK  )
	{

		if(   m_lickList  ==  NULL   )
		{
			retErrorMesg =     "CalcedNoteListExternal::Save_ObjectList_Chunk  failed,   m_lickList  is NULL ."   ;
			return  false;
		}

		numberOfListObjects  =     m_lickList->Count();
	}
	else
	{	ASSERT( 0 );  }


																					//	A)   write the   List-Chunk's   TAG 


	if(   ! Write_Chunks_Header(   file,    chunkCode,    version,    retSizeFilePos,    retErrorMesg  )    )      //  OVALLISTcHK :   the  'OvalList  ITSELF'   Chunk  of the file 
	{																	
		return  false;     //  returns the CurrentFilePosition of the Tag with 'retSizeFilePos',  for a later write with  Write_Chunks_Length()
	}


																				    //	B)   ALWAYS write the count of ListObjects right after the Chunk's Tag  
	Write_Lists_Count(   numberOfListObjects,    file  );



																					//	C)   write the  List's 'OBJECTS'
	long   listCount =   0;


	if(    numberOfListObjects  >  0   )
	{

		if(   chunkCode  ==  SpITCHLISTcHK  )
		{

			ListIterator< MidiNote >   iter(   *m_calcedNoteList    );	 


			for(    iter.First();    !iter.Is_Done();    iter.Next()    )			//  save list's   CalcedNote  as  ScalePitchSubjects 						
			{						

				MidiNote&         midiNote =     iter.Current_Item();																			
				SPitchFileObj    fileObj;    


				Assign_FileObject_ScalePitch(   fileObj,    midiNote,    savingOldPitchScope2007File    );
												

				long   ldSize  =    ( long )sizeof(  SPitchFileObj  );   
				file.Write(   &fileObj,    ldSize  );

				listCount++;
			}
		}   

		else if(   chunkCode  ==  DZONELISTchkLEFT  ||   chunkCode  ==  DZONELISTchkRIGHT ||  chunkCode  ==  DZONELISTchkCENTER  )  
		{				

			DetectZoneFileObj    fileObjDZone;  


			Assign_FileObject_DetectZone(   fileObjDZone,    channelCodeStereo,     m_startOffsetDetectZone,    m_endOffsetDetectZone   );


			long   ldSize  =    ( long )sizeof(  DetectZoneFileObj  );   
			file.Write(   &fileObjDZone,    ldSize  );

			listCount++;

		}   

		else if(   chunkCode  ==  LICKlISTcHK    )  
		{				

			ListIterator< Lick >   iter(   *m_lickList    );	 


			for(    iter.First();    ! iter.Is_Done();     iter.Next()    )								
			{						
				Lick&           lick =    iter.Current_Item();																			
				LickFileObj   fileObj;    


				Assign_FileObject_Lick(   fileObj,    lick.m_startSampleVirt,    lick.m_endSampleVirt,    lick.m_nickName   );
										

				long   ldSize  =    ( long )sizeof(  LickFileObj  );   
				file.Write(   &fileObj,    ldSize  );

				listCount++;
			}
		}   
		else
		{	ASSERT( 0 );   }
	}   //    if(    numberOfListObjects  >  0   )



	ASSERT(   listCount  ==   numberOfListObjects   );

																				//	D)   write the  'LENGTH'  of the Chunk to the TAG

	Write_Chunks_Length(   file,   retSizeFilePos   );                   //  (  Go back to the TAG-location, and  write the length to the  'just-saved'  tag  )

	return   true;
}



											////////////////////////////////////////


bool    CalcedNoteListExternal::Load_ObjectList_Chunk(   short  chunkCode,    unsigned char  version,  CFile&  file,    CString&  retErrorMesg    )   
{												 


	bool   loadingFromPitchScope2007File =    false;
	
	retErrorMesg.Empty();


	ASSERT(  m_calcedNoteList  );



	if(          chunkCode  !=  SpITCHLISTcHK   &&     chunkCode  != LICKlISTcHK  
		 &&    chunkCode  !=  DZONELISTchkLEFT  &&   chunkCode  !=  DZONELISTchkRIGHT &&   chunkCode  !=  DZONELISTchkCENTER   )
	{
		retErrorMesg  =  "CalcedNoteListExternal::Load_ObjectList_Chunk  FAILED,   missing case for  'chunkCode'. " ;
		return  false;
	}




																		 //	  A)    read the FILE-Chunk's  'TAG'  
	unsigned char   retVersion;								
	long                 retChunksSize;  


	if(   ! Goto_Chunks_Header(   file,    chunkCode,   retVersion,   retChunksSize,   retErrorMesg  )    )
		return  false;    //  it looks up the  'TAG's  identifying string'  (  "SpICHLST"	for  SpITCHLISTcHKPlayer ),  and descends into the File till the string is found.


//	m_loadedFilesVersionNumber  =     (short)retVersion;   Done previously





// ***************************   CAREFULL  *************************************


	if(    retVersion   <=  10   )                 //  Navigator and PitchPlayer   file versions are  "12",    OLD PitchScope are "10"     2/2012
	{
		loadingFromPitchScope2007File =    true;
	}

//  *****************************************************************




																		 //	  B)    read the  'count'  of Objects in the List    ( it is always written right after the List-Chunk's TAG )

	long   numberOfNotes     =      Read_Lists_Count(  file );  
	if(      numberOfNotes  ==  0  )  
		return  true;       //  Not an error,  it is OK to have an empty list in the file.


	if(    numberOfNotes  <  0  )  
	{
		retErrorMesg  =   "ComponentExternal::Load_ObjectList_Chunk  FAILED, has a negative count for list."  ;
		return  false;
	}
											

//	listSubject->Delete_Childrens_Subjects_And_CompViews(    &(  listSubject->m_componentViews  )    );    // NO,  have calling function do this



	long    listCount =   0;


	if(   chunkCode  ==  SpITCHLISTcHK   )
	{

		for(    long  i =0;     i < numberOfNotes;      i++    )
		{ 
			
			SPitchFileObj   fileObj; 

			long   ldSize  =    ( long )sizeof(  SPitchFileObj  );
			file.Read(   &fileObj,    ldSize   );


			MidiNote    *nuCalcNote  =     new   MidiNote();
			if(  nuCalcNote  ==  NULL    )
			{
				retErrorMesg =   "ScalepitchExternal::Receive  failed,  could not allocate new ListObject." ;
				return  false;
			}

			AssignMe_from_FileObject_ScalePitch(   fileObj,    *nuCalcNote,  loadingFromPitchScope2007File   );   	

			m_calcedNoteList->Add_Tail(   *nuCalcNote   );

			listCount++;
		}

	}

	else if(   chunkCode  ==  DZONELISTchkLEFT ||  chunkCode  ==  DZONELISTchkRIGHT  ||   chunkCode  ==  DZONELISTchkCENTER  )
	{		
		ASSERT( 0 );     // Need to do anything ?????    12/2011
	}

	else if(   chunkCode  ==  LICKlISTcHK    )  
	{				

		for(    long  i =0;     i < numberOfNotes;      i++    )						
		{						
			LickFileObj   fileObj;    

			long   ldSize  =    ( long )sizeof(  LickFileObj  );
			file.Read(   &fileObj,    ldSize   );


			Lick    *nuLick  =     new   Lick();
			if(  nuLick  ==  NULL    )
			{
				retErrorMesg =   "CalcedNoteListExternal::Load_ObjectList_Chunk FAILED,  could not allocate new Lick." ;
				return  false;
			}


	//		AssignMe_from_FileObject_ScalePitch(   fileObj,    *nuCalcNote   );   	
			AssignMe_from_FileObject_Lick(   fileObj,    *nuLick   );
										

			m_lickList->Add_Tail(   *nuLick   );
			listCount++;
		}
	}   
	else
	{	ASSERT( 0 );   }
	
	

	ASSERT(  listCount  ==   numberOfNotes  );

	return   true;
}




