/////////////////////////////////////////////////////////////////////////////
//  SPitchCalc.h   -   calculate properties for a new midi note:  ScalePitch, Octave, Duration
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 


/*
   'ScalePitch' has 12 possible pitch values:  { E, F, F#, G, G#, A, A#, B, C, C#, D, D# } because there is NO octave calulation yet. 
	
	Do, Re, Mi, Fa, So, La, Ti, Do is another way to express ScalePitch because it also contains no octave assignment.  
*/


////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_SPITCHCALC_H__119__INCLUDED_)
#define AFX_SPITCHCALC_H__119__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   DFTrowProbe;

class   logDFTtrForm;
class   HarmPairsTrForm;
class   HarmPairsVerter;

class   SndSample;

class   DFTrowProbeCircQue;

class   OffMap;

class    CalcedNote;

class    MidiNote;




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define   kMAXsIZEofNOTEcircQUE   220      //   Need 200 if DetailSlider goes to MAX of 25  ( 25 * 8speed = 200 )         80   ******* Is this too SMALL ???   11/11  ********************


#define   kMAXsIZEofFINALnoteCircQUE   360      //   the m_drivingOffMapHorz  should tentatively have this width

#define   kMAXsIZEofFINALnoteCircQUErealistic   140    //  NEW,  this determines the background width of the DrivingView   10/11



					         //   kSizeOfFinalNotesCircque    ***  Changing this value changes the DELAY in SoundPlay and render to DrivingView

#define   kSizeOfFinalNotesCircque1xSlow   124       //     122              was  122 [7/25/12]       was  124 [1/5/12]     was  126 [12/14/11],     was 121 [11/6/11 ],           
																			//		before userTweak and compensation for Primary CircQue delay
																		    //  In December, after installing the MidiSync Slider on the main dialog, it seems that 126 was too high.  12/16/2011


#define   kSizeOfCircularSndSample    16000    //  MUST be bigger that the biggest  BLOCK-load of samples, or will overwrite its own NEW data.  1/12




#define   kEarlySamplesCountForPrinting   135792     //   Need a constant for printing  7/12.      135792 =  123 x  1104     [ 1104:  Samples in speed1  PieSlice  ]  



#define  kMAXhiResValueLogDFT   4    //   Can only increae the resolution by 4 x ,  now using  3    


////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   Lick     //  :  public  ???
{

public:

	Lick()  
	{
//		m_startSample =   -1;
//		m_endSample  =  -1;


		m_startSampleVirt =   -1;    //   "Virtual" :    What the Phrase boundaries look like in the DrivingView  ( m_startSampleVirt  is far LEFT time column )
		m_endSampleVirt  =   -1;


	//	m_nickName.Empty();
	}



	Lick(   long  startSampleVirt,   long  endSampleVirt,   CString&  licksName   )   :    m_startSampleVirt( startSampleVirt  ),    m_endSampleVirt( endSampleVirt  )
	{

		/***
		ASSERT(   startSample >= 0   );
		ASSERT(   endSample  >   0   );

		ASSERT(   endSample  >   startSample   );
		***/

		ASSERT(   startSampleVirt >= 0   );
		ASSERT(   endSampleVirt  >   0   );

		ASSERT(   endSampleVirt  >   startSampleVirt   );



		ASSERT(   ! licksName.IsEmpty()     );

		m_nickName =   licksName;
	}


	virtual ~Lick()
	{   } 


	long				Get_Length_In_Samples()        {  // return  ( m_endSample  -  m_startSample );    
																			return  ( m_endSampleVirt  -  m_startSampleVirt );    
											                          }

	CString&		NickName()                             {  return  m_nickName;   }       //  SHOULD also be able to   ASSIGN to memberVar   from this    12/11




public:
//	long	   m_startSample;
//	long	   m_endSample;


	long	   m_startSampleVirt;    //   "Virtual" :    What the Phrase boundaries look like in the DrivingView  ( m_startSampleVirt  is far LEFT time column )
	long	   m_endSampleVirt;



//    m_timeStamp      time created  ( so can keep/reDisplay in same order as created 


	CString    m_nickName;     //  Do I have to make sure that they are UNIQUE?? ( search list before user gets clearance)    What about LIST SEARCHES ??    12/11
};





////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   SPitchCalc
{

public:
	SPitchCalc(   long   numberOfWavemanEvents,    long  byteCountTwoSeconds,    long  numSamplesInBlockLocked,    
		                                          ListDoubLinkMemry< MidiNote > *noteList,     bool usingCircQueDFTprobes   );
	virtual   ~SPitchCalc(void);



	
	void		   Initialize_for_Play();            //  Should Call this at the start of  almost any PLAY ( NoteList or Detection )   8/2012

	void		   Initialze_For_File_Position_Change();    



	bool				    Is_Playing_Backwards();  


	long					Calc_DrivingViews_First_SampleIdx(   bool   isPlayingBackward   );
	long					Calc_DrivingViews_Last_SampleIdx(    bool   isPlayingBackward   );


	bool					Is_DrivingView_at_First_TimeFrame(   bool   isPlayingBackward    );
	bool					Is_DrivingView_at_Last_TimeFrame(    bool   isPlayingBackward,   long  filesLastSample   );




	long					Get_ChunkSize();    //     512  or  1104   dependant on App    9/2012


	long					Calc_Samples_In_PieEvent(  double  playSpeed  );    //  takes into account the REDUX for SPEED-slowDown.  Can INPUT with  SPitchCalc::m_playSpeed 

	static		long		Get_Bytes_Per_16bit_Sample()     {   return  4;   }    


	long				    Get_Lowest_MidiStartBand();      //  kMIDIsTARTbANDdft  52 	



	void					Find_Long_Minmax(  long  number  );   //  for DEBUG of FIXED-POINT  values

	void					Initialize_MinMax_Variables();




	long				Get_SndSamples_Valid_Count(  double  slowSpeed   );   //  at slow speeds, not all the Samples in the 

	bool				Allocate_SndSample_for_SPitchCalc(   long  numSamplesInBlockLocked,   long  chunkSize,   CString&  retErrorMesg   );





															//   A)   DETECTION  of Notes   


	bool		Search_NoteList_for_AudioMusical_Values(   long   currentSampleIdx,   CalcedNote&   retCurrentNote,   bool isPlayingBackwards,  
													                          bool  useNavigatorDelay,  long  pieSliceIdxAtDetection,   long  expectedEventNumber,  
															                  long  realEventNumber,  long  pieSliceCounter,   short   fromPreRoll,      CString&  retErrorMesg    );
															    
	bool		Detect_Note_for_Pie_Slice(    long   newNotesSampleIdx,    long  eventsOffsetAdj,     CalcedNote&   retCurrentNotePlayer,    bool isPlayingBackwards,  
		                                                     SndSample&  sndSample,   bool  useNavigatorDelay,  long  pieSliceIdxAtDetection,    short  fromPreRoll,  
																											long  pieSliceCounter,  CString&  retErrorMesg    );


	bool		Estimate_ScalePitch(   long  currentSampleIdx,    bool  isPlayingBackward,    CalcedNote&  retCurrentNoteBackwards,    bool  useNavigatorDelay,
																							     long  pieSliceIdxAtDetection,   short  fromPreRoll,    long  pieSliceCounter,  CString&  retErrorMesg   );   


	long		Get_logDFTs_Write_Column();
	long		Get_logDFTs_Read_Column();

	short	    Get_logDFTs_Read_Kernal_Width();

	void        Get_logDFTs_Octave_Zone(    long&  retXstart,    long&  retXend   );  

	void        Set_Bottom_Frequency_Cutoff_DFTrows(   long  bottomRowsToSkip   );    //  Change so it takes MIDI units  ( bottom row is 52 midi )



	bool		Allocate_DFT_Probes(    bool  useCircQueProbes,   short  bottomDFTrowsToSkip,    CString&  retErrorMesg   );

	void		Initialize_All_DFTprobe_CircularQues_ArrayElements_and_Sums();


	bool		Calc_logDFTs_Values(   SndSample&  spitchCalcsSample,    long  eventsOffsetInSamples,    CString&  retErrorMesg   );    //  calcs/writes values for DFTmap's pixels from latest byte

	bool		Calc_logDFTs_Values_Regular(   SndSample&  spitchCalcsSample,    long  eventsOffsetInSamples,    CString&  retErrorMesg   );
	bool		Calc_logDFTs_Values_CircQue(   SndSample&  spitchCalcsSample,    long  eventsOffsetInSamples,    CString&  retErrorMesg   );


	bool		Is_CurrentSampleIdx_Continuous(   long  currentSampleIdx,   long  curPieSliceIdx,    bool  isPlayingBackward,     bool  usesNavigatorDelay,   
		                                                                                                                          long&  retLostNotesCount,    short  fromPreRoll,   CString& functionCode     );




															//   B)     SELECTION  functions   


	//				CalcedNote*    Find_DrivingViews_CalcedNote_in_FinCircQue(       PointLong   ptOffmap,     long&  retIndexToFinCircque,      bool  isPlayingBackward    );     
					CalcedNote*    Find_DrivingViews_CalcedNote_in_FinCircQue_NEW(   PointLong   ptOffmap,     long&  retIndexToFinCircque,    short  backwardsCode   );		


					MidiNote*       Search_NoteList_For_MidiNote(   long  targetSampleIdx   );



					bool			Find_ColorRun_Midpoint_by_SampleIdx(   long  targSampleIdx,     short  scalePitch,    PointLong&  retMidPoint,   bool  doVertical,   CString&  retErrorMesg  );


					bool			Find_DrivingViews_ColorRun(     PointLong  pt,      long&  retStartPixel,    long&  retEndPixel,    short&  retScalePitch,
																				   long&  retBeginSampleIdx,   long&  retEndSampleIdx,     short  backwardsCode  );    //  backwardsCode:  Positive values cause an OVERIDE 


					bool			Find_Next_Forward_ColorRun(     long  startCoord,     PointLong&  retRunsMidpoint,   short&  retScalePitch,    short  musicalKeyControlsValue,   CString& retErrorMesg  );
					bool			Find_Next_Backward_ColorRun(    long  startCoord,     PointLong&  retRunsMidpoint,   short&  retScalePitch,    short  musicalKeyControlsValue,   CString& retErrorMesg  );



					short			Calc_ColorRuns_Octave_and_DetectScores(   short  scalePitch,      long  startPixel,  long  endPixel,      bool  doVertical,   
														                         short&  retAvgDetectScore,    short&  retAvgHarmMag,   long  retOctaveScores[]  );



					bool          Find_FinalCircQues_ScalePitch_Run(  PointLong  pt,    long&  retStartFinCirqueIdx,   long&  retEndFinCirqueIdx,   short&  retScalePitch,    
														                     long&  retPixelLength,   long&  retIntervalBeginSampIdx,  long&  retIntervalEndSampIdx,   	
													                         bool  isPlayingBackward  );    //   *** NO CALLERS ***   but keep around for ideas.  5/2012   





															//   C)     NOTELIST  functions   


	                bool		    Get_Pixel_Info_from_Notelist(   long  targetSampleIdx,     short&  retScalePitchIdx,    short&  retDetectScore,    short&  retAvgHarmonicMag,    
																short& retOctaveIdx,     ListDoubLinkMemry<MidiNote> *noteList,    ListsDoubleLink<MidiNote> *lastFoundCalcNoteLink, 
																bool  isPlayingBackward,  short&  retSynthCode,   short&  retPixelBoundaryType,   MidiNote **retFoundNote,  CString& retErrorMesg  );

					void		    Assign_NoteLists_Cache_ListLink(   ListsDoubleLink<MidiNote>  *cacheLink  );  // Should only be CALLED by  Get_Pixel_Info_from_Notelist()  2/12



	static			long			Sort_NoteList_by_TimePosition(    ListDoubLinkMemry<MidiNote>&   theList,        CString&  retErrorMesg  );

	static		    bool			Check_NoteLists_Intergity(           ListDoubLinkMemry<MidiNote>&   spitchList,    CString&  retErrorMesg  );

	static		    bool			Is_NoteList_TimeOrdered(             ListDoubLinkMemry<MidiNote>&   noteList   );



	static		    ListDoubLinkMemry< MidiNote >*    Make_Clone_List(   long  startOffset,   long endOffset,    ListDoubLinkMemry<MidiNote>&  srcList  );


	static		    bool			Add_TempLists_Notes_to_Master_Notelist(   ListDoubLinkMemry<MidiNote>&  srcList,   ListDoubLinkMemry<MidiNote>&  masterList, 
		                                                                                               long  startRecordSampleIdx,  long  endRecordSampleIdx,  long&  retNoteCountAdded,   CString&  retErrorMesg   );

				    bool			Delete_Note(   MidiNote  *note,    CString&  retErrorMesg   );

					bool			Find_And_Kill_MidiNote(   MidiNote  *note,    CString&  retErrorMesg   );

					bool			Replace_Undo_Note_to_NoteList(   CString&  retErrorMesg   );
		
					void			Erase_the_UndoNotes_Data();




	static		    long			Count_Enclosed_Notes_within_TimeSegment(      long  startOffset,   long endOffset,     ListDoubLinkMemry<MidiNote>&   masterList  );

	static		    long			Count_Concurrent_Notes_within_TimeSegment(   long  startOffset,   long endOffset,     ListDoubLinkMemry<MidiNote>&   masterList  );




	static		    bool			Delete_Notes_in_TimeSpan(   long  startOffset,   long endOffset,      short&  retDeleteCount,    ListDoubLinkMemry<MidiNote>&  masterList, 	
																						                        ListDoubLinkMemry<MidiNote>&  tempList, 	 CString&  retErrorMesg   );

	static		    bool		    Calc_StartOffset_EndOffset_for_NoteLists_Merge(   long&  retStartOffset,   long& retEndOffset,    ListDoubLinkMemry<MidiNote>&   theList,  CString&  retErrorMesg   );



	static		    bool			Are_Rectangles_Concurrent(    long alphaStart,  long alphaEnd,     long betaStart,  long betaEnd,    bool useEqualSigns  );

	static		    bool 			Are_Notes_Equal(   MidiNote&  noteA,    MidiNote&  noteB   );      





	static		    bool		     Count_Concurrent_Notes_in_List(   ListDoubLinkMemry<MidiNote>&  noteList,  	long&  retConcurentCount,   CString&  retErrorMesg   );

	static		    MidiNote*	     Find_A_Concurrent_Note(     ListDoubLinkMemry<MidiNote>&  noteList, 	    long  startOffset,    long  endOffset,  
																		     long&  retNotesIndexInList,    bool  useEqualSigns,    CString&  retErrorMesg   );


	static		    MidiNote* 	     Find_Neighbor_MidiNote(   MidiNote&  selectedMidiNote,   ListDoubLinkMemry<MidiNote>&  noteList,   bool  backSearchFlag,   CString&  retErrorMesg    );

	static		    MidiNote*  	     Find_Next_MidiNote(    long  sampleIndex,    ListDoubLinkMemry<MidiNote>&  noteList,    CString&  retErrorMesg   );




															  //   D)    'PRIMARY'  Circular Que   ( the  1st  of 2 Circular Ques  )
	void		Initialize_Notes_CircularQue();

	void        Add_New_Note_to_CircularQue(   CalcedNote&   newNote   );


	bool		Get_Most_Common_ScalePitch_In_CircularQue(  short  minimumMatchCount,   short&  retFoundSPitch,  short&  retBestOctaveIdx,  short&  retOctaveScore       );

	void		Get_New_Filtered_ReturnNote_CircularQue(  CalcedNote&   retNote   );   


	short       Calc_Median_Filter_Ratio_by_Speed(   double   playSpeed,   double  widthSpeedOneDecimal,   short&  retMatchCount   );    //  that is the SIZE of the PrimCircQue.  11/11


	static		short     Calc_Position_Nudge_for_Detail_Slider(   double  speedFlt    );

	short		Get_Notes_AbsoluteIndex_from_RelativeHistoryIndex_CircularQue(  short  relativeHistoryIndex  );    // ******  NOT USED,  is this worthwhile ????   3/11  *****




															  //  E)    'FINAL'  Circular Que,   use this que to create a DELAY so we can draw the FUTURE pixels in the DrivingOffmap.  4/10

	void		 Initialize_Notes_Final_CircularQue(   short  cuesSize  );    

	void		 Add_New_Note_to_Final_CircularQue(   CalcedNote&   newNote  );


	void		 Get_Oldest_Note_from_Final_CircularQue(    CalcedNote&   retNote    );

	void		 Get_Newest_Note_from_Final_CircularQue(   CalcedNote&   retNote   );		


	void	     Change_Final_CircularQue_Size(  long  newQueSize  );    

	long	     Calc_Ideal_FinalCircQue_Size(   short  newFilterSize,    short&  retUserTweakValueCalced   );  



	long		 Approximate_Note_Count_In_FinalCircQue_with_BoundaryRounding(      double  playSpeed,   bool useBoundaryRounding,  long bytesPerSample  );   

	long         Approximate_Sample_Count_In_FinalCircQue_with_BoundaryRounding(  double  playSpeed,   bool useBoundaryRounding,  long bytesPerSample   );



	void		 Dump_Final_CurQue();    //  to TRACE()

	void		 Add_False_CalcNotes_for_OutOfSync(   long  noteCount,    long  currentSampleIdx,      // ****** NO   CALLERS, maybe keep for info.  ********
		                                                                                       bool isPlayingBackwards,   CalcedNote&  currentNote   );




																//  F)    And also a   BIG  CircularQue for  'SndSample'   8bit Samples   1/2012

	void			Initialize_SndSample_CircularQue();

	void			Add_SndSample_Samples_to_SndSampleCircularQue(   long   bytesToProcess   );

	char			Read_SndSample_CircularQue_Value(  long  indexVirtual   );




																//   G)    various  DELAY  functions

		
	long		Calc_Primary_CircQues_Delay_In_CalcedNotes();    // ***** BIG change to the algo,  might be one short


	long		Calc_Final_CircQues_Delay_In_CalcedNotes();

	long		Calc_LogDFTs_Delay_In_CalcedNotes();      


	long		Get_DetectionDelay_InPieSlices();   //  based on what I learned in   Make_NoteList_No_Audio()   7/2012 
	long		Get_DetectionDelay_InSamples();   



	long        Read_Players_NoteList_Audio_Delay_inPieSlices();      //   New value is now  stored in   PitchPlayerApp




	long		Get_PlayNoteList_Audio_Delay_InPieSlices();     //    EventMan::Process_Event_Notification_PPlayer(),    Process_Event_Notification_PPlayer_NoAudio()
																					 //    to FineTune   the SYNC  of Audio and Midi  in Player and Navigator   [  Search_NoteList_for_AudioMusical_Values()   ]     NEW,  7/2012    
	long		Get_PlayNoteList_Audio_Delay_InSamples();




	long		Get_NoteList_Position_Delay_InPieSlices();     //  Need this value for  SPitchCalc::ReDraw_DrivingViews_OffMap_from_NoteList(),  so I can  visually ALAIGN  
															   //  the DETECTION-DriveView,  with  DriveView that is drawn from the NOTELIST, and with XORbox SELECTION.    8/2012
	long		Get_NoteList_Position_Delay_InSamples();     

																	   																		


												//   H)    DrivingView's  OFFMAP  relates DIRECTLY to the size of the Final-CirceQue ( its pixel-width is a multile of the size of  the Final-CirceQue


	void		 Update_DrivingViews_OffMap_with_Nudge(   CalcedNote&   calcedNote,   bool  isPlayingBackward   );      //  called by  Add_New_Note_to_Final_CircularQue()

	void		 Update_DrivingViews_OffMap_DEBUG(   short  scalePitch,    short  octaveIndex,     bool  isPlayingBackward   );



	void		 ReDraw_DrivingViews_OffMap_from_FinalCircQues_Array(   bool  isPlayingBackward,   OffMap&  drivingOffMap,    bool  doVertical   );



	bool         ReDraw_DrivingViews_OffMap_from_NoteList(    bool  isPlayingBackward,    OffMap&  drivingOffMap,     bool doVertical,     CString&  retErrorMesg   );


	bool		 Cleanup_FinalCircQue_from_NoteList(   bool  isPlayingBackward,   OffMap&  drivingOffMap,   bool  doVertical,   CString&  retErrorMesg  );  





	void		 Erase_CircularQues_and_DrivingBitmap();

	void		 Erase_logDFTmap_and_HarmPairsMap();   	//  Will this help problems with accuract of NoteDetect after a PreRoll ???? 




	void         Write_DrivingViews_X_Column_Raw(   short  scalePitch,   short  detectAvgHarmonicMag,    short  octaveIndex,    short  xWriteColumn,      
																									short   keyInSPitch,    OffMap&  drivingOffMap   );

//  ****  REPLACE   Write_DrivingViews_Y_Row()    with   Write_DrivingViews_Y_Row_RAW()   **************************
//	void            Write_DrivingViews_Y_Row(     CalcedNote&   calcedNote,    short  yWriteRow,         short   keyInSPitch,   OffMap&  drivingOffMap  );   //   for vertical driving view  11/11

	void		 Write_DrivingViews_Y_Row_RAW(  short  scalePitch,   short  detectAvgHarmonicMag,    short  octaveIndex,      short  yWriteRow,    
																									short   keyInSPitch,   OffMap&  drivingOffMap   );


	static		 long		Calculate_Briteness_Divisor(  long  displayBritnessFactor  );


	short					Calc_Offmaps_DeadZone_WriteRow(  short  mapsDimension  );

	void					Transpose_DrivingViews_OffMap_by_MusicalKey(  short  newMusicalKey,   bool  isGoingBackwards,    bool  doVertical   );





																//   I)    detection of   MUSICAL KEY   and  ScalePitch TEXT-Names


//	static	   void		     Get_Text_of_Musical_Key(             short  musicalKeyIdx,       CString&  retKeysName  );
	static      void	     Get_ScalePitch_LetterName_NEW(   short  scalePitch,    short  userPrefMusicalKeyAccidentals,   CString&  retNotesName   );

	static      void		  Get_ScalePitch_LetterName(      short  scalePitch,   char *firLet,  char *secLet,    short  userPrefMusicalKeyAccidentals,    CString&  retNotesName    );   //  similar funct in  DFTtransforms.cpp

	static		void          Get_Notes_Numeral_Name(         short  scalePitch,    short  keyInScalePitch,    CString&  retNotesName   );



	static		  short		Calc_FileCode_from_MusicalKey(   short  keyInScalePitch,     CString&  retMKeysName    );

	static		  short		Get_Musical_Key_from_FileCode(   short  keysFileCode,   short&  retSharpCount,   short&  retFlatCount,   short&  retScalePitch,   bool&  retIsMinor   );



				bool	    Calculate_Musical_Key_From_NoteList(    short&  retBestCount,    short&  retBestScore,    CString&  retErrorMesg   );

	static		bool		Calculate_Musical_Key(  short  usersMusicalKeyGuess,    long   noteScoresWaveMan[],     long  noteCount,     short&  retMKeyBestProfiles,  
		                                                        short&  retMKeySecondBestProfiles,   short&  retMKeyThirdBestProfiles,  	CString&  retErrorMesg   );

	static		void		Calc_MusicalKey_by_ScaleProfile(   long  pitchScores[],   short&  retMusicalKeyBest,    short&  retMusicalKeySecondBest, 
		                                                                                                                       short&  retMusicalKeyThirdBest,      long    retProfileScores[]    );	


	static		  bool		Does_MusicalKey_Use_Sharps(    short  musicalKeyIdx   );

	static		  bool		Get_KeySignature_Name_and_ScalePitch(   short  numberAccidentals,   bool  useSharps,   bool  isMinor,    CString&  retName,   short&  retScalePitch,   CString&  retErrorMesg   );



	static		  void		Get_ScalePitchs_Color(    short  scalePitchIdx,     short  keyInSPitch,      short&  retRed,    short&  retGreen,   short&  retBlue    );  

	static      short		Get_ScalePitchs_MusicalKey_Transposed_Position(    short  scalePitchIdx,     short  keyInSPitch  );  



	static      void		    Calc_Time_in_MinutesSecsMills(   long  sampleIdx,    long& retMinutes,   long& retSeconds,    long& retMilliSeconds   );

	static      void		    Calc_Time_Format_String(           long  sampleIdx,     CString&  retTextString   );






public:
		ListDoubLinkMemry< MidiNote >   *m_calcedNoteListMasterApp;   //   SPitchCalc does NOT know about Recording NoteLists,  or PitchPlayerApp::m_calcedNoteListTemp

		SndSample    *m_spitchCalcsSndSample;    //   Resides here.     Raw data for the Fourier analysis.


		short		  m_playModeUsingNotelist;     //   0:  Detection     1:  Play-Notelist      2:  ????    


//		long         m_noteListHardwareDelayForPlayer;      //   NEW,  only Player.exe   ...should deal with this value
		long        *m_noteListHardwareDelayForPlayerAddr;      


		char        *m_circularSndSample;     //  This CircularQue  is like a HUGE SndSample ( it hold the previous samples that were in  'm_spitchCalcsSndSample' )

		long	     m_sizeCircularSndSample;      

		long		 m_indexCircularSndSample; 


		long		 m_indexLastSndSampleBlockload;   // We fill this thing with a 'BLOCK-Load',  right after  Fetch_Streaming_Samples_Direct_PPlayer()

		long		 m_countValidBytesCircularSndSample;   // *** NOT used, but it could be a SAFETY mechanism to warn of bad Data-Reads.  1/12
													//  Careful,  this can be as high as 16,000.  Do not confuse with Valid-Count for the smaller  m_spitchCalcsSndSample.  



		double      m_playSpeedFlt;   // make sure it is always updated  



		short       m_numPeriods;     //  18 is best ??   ...BUT is 17 a MAGIC number for FourierSeries ???   See  Get_Best_Rational_Number_SevenTeenNumer()


		long		m_bottomFreqCutoffInMidi;    


		MidiNote	m_deletedListNote;     //   Find_And_Kill_MidiNote() will update this    2/2012




		short	  m_musicalKey;   //   Is this   ALWAYS the SAME   as   PsNavigatorDlg::m_musicalKeyControlsValue   5/2012 


		double	  m_logDFTdarknessFactor;   //   Makes the DFT lighter or darker,  but does not impact the SOURCE-calcs  
										    //        ...just boost the logDFT Pixel briteness with a SMALLER value for  m_logDFTdarknessFactor.    9/2012

		long      m_displayBritnessFactor;    //    5 vals     {  0, 1, 2, 3, 4  }      



		long      m_numberOfWavemanEvents;   

		long      m_byteCountTwoSeconds;      //    same as   TWOSECBYTES


												

		short     m_detectionSensitivityThreshold;    //  ADJUSTABLE    *******BIG*****

		short	  m_componentCountHPairs;            //  ADJUSTABLE    {  4  -  10 }    Lower might be better.  Martin Piszczalski favored 4. 
	
		short	  m_userFinalCircqueSizeTweak;   // **** NEW    user can 'tweak'  the actualy time of delay for Midi sync  



		bool	  m_letNotesSustain;    //   Do NOT CHANGE!!  *** A GREAT Threshold technique...  3 forces in competition:  a  'NewPitch',  the  'CurrentPitch', and the 'SilencePitch'     1/2012


		long      m_sampleRate;        //  44100



  

														     //  A)   for the  FIRST  circular cue of notes, as we filter and smooth-out the results. 

		CalcedNote		m_circQuePrimNotes[  kMAXsIZEofNOTEcircQUE  ];     

		short	        m_sizeOfPrimNotesCircque;     


		short           m_numberNeededToMatch;        //   ADJUSTABLE,  in decision making when analyizing the CircQue

		short		    m_currentIndexPrimCircque;    //  to the Circular Cue  of notes



									                     //   B)     These 3 are exclusively used in  in  Get_New_Filtered_ReturnNote_CircularQue and  Initialize_for_Play() 
									                     //           ( this is a later funtion to convert the  'raw impulses'   into SEGMENTS ( CalcedNote with duration )


		bool      m_aNoteIsPlaying;		//    is a note playing on the Synthesizer  

		short	  m_prevFoundNote;		//   the last scalepitch that was  returned from  Get_New_Filtered_ReturnNote_CircularQue()  

		short 	  m_prevPlayingNotesAvgHarmonicMag;	    //  need this for  m_letNotesSustain =  true



														//   C)   and still ANOTHER CircularQue for the Filtered and Final notes that come out ( for display


		CalcedNote		m_circQueFinalNotes[  kMAXsIZEofFINALnoteCircQUE  ];     // ****     for the circular cue of notes, as we filter and smooth-out the results.  

		short	        m_sizeOfFinalNotesCircque;      

		short			m_currentIndexFinalCircque; 




		//   ****  These functs should probably be part of a new   'Scrolling CircularQue'  CLASS  ( a virtual Pane ) *****
		//													( its always useful to be able to display any raw data ) 

		OffMap       *m_drivingOffMapHorz;   //  to Blit to Navigators 2nd dialog window.   This is a REFLECTION of what is placed in  m_circQueFinalNotes.  The two must always COORDINATE.  
		OffMap       *m_drivingOffMapVert;   //  For a VERTICAL flow of notes.   11/11   This is a REFLECTION of what is placed in  m_circQueFinalNotes.  The two must always COORDINATE.  

		bool          m_useScalePitchDrivingOffMap;     //  ALWAYS true.   the  DrivingOffMap  can show  SchalePitches(12 pixes high),  or  4-Octaves

		bool		  m_doVerticalDrivingview;      //  the switch for    m_drivingOffMapHorz    and    m_drivingOffMapVert





														//    logDFT   SUPPORT

		ListMemry< DFTrowProbe >    m_rowList;  


		long        m_totalRows;		     //  kTOTALcELLdftBANDs   76

		short       m_startBandMidi;    //    ( Midi  91,   G )    is near  high string, 12th fret  ( really is a  G ) 



		bool		  m_useDFTrowProbeCircQue;    //   Now have 2 ways to calc/write the values to the DFTmaps pixels

		long		  m_hiResRatioLogDFT;            // ****  NEW,  can make more HiRes when using CircQueLogDFT algo,  just read thye Que at more Tap-Points    9/2012




		logDFTtrForm       *m_logDFTtransform;		
		HarmPairsTrForm    *m_harmpairsTransform;

		logDFTtrForm       *m_logDFTtransformDEBUG;		 // ** OMIT **


		short         m_hpairsMapsWidth;

		short         m_dftMapsWidth;



		double    m_biggestInt,   m_smallestInt;       //  To check for Overflow,  these are really BAD tests...   see  DFTrowProbeCircQue::Transform_Row()  9/12

		double    m_closestDiffToTop,    m_closestDiffToBottom;   
};






////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

class   DFTrowProbe  
{

	//	  Calculates the values for the logDFTtrForm (stored as an OffMap).  Uses Fixed Point calculations for speed. 

public:
	DFTrowProbe(   SPitchCalc&  sPitchCalc,     long  cellLength  );   
	virtual ~DFTrowProbe();


	virtual	bool				Create_TrigTable(   double  angFreq,    long  numSamps,     bool  doWindow    );

	virtual	short				Get_ShiftFactor(   long  numSamples   );

	virtual	short 			    Calc_Magnitude_of_Cell(    char  *samplesBits,     bool useAlternateRead,   long  indexToCircSndsamplesProbeStart,    CString&   retErrorMesg    );      //   returns  <0  if fail


	virtual	long				Get_ChunkSize()     {   return   m_sPitchCalc.Get_ChunkSize();   }  




public:
	SPitchCalc&      m_sPitchCalc;

	TRIGTABLEfrc     m_trigTabl;         // RESIDES here,   ( not just pointer )


	long       m_cellLength;      //  how many samples for 18 periods.   Determines the bandwidth.  

	long	   m_samplesInPeriod;       //   =     (long)(    1.0  / angFreq    );


	short      m_yVirtual;

	double     m_frequency;     //   ( in Hertz )

	short	    m_scalePitchIdx;

	short	    m_midiNumber;  
};



									//////////////////////////////////////////////////////
									//////////////////////////////////////////////////////


class   DFTrowProbeCircQue     :   public   DFTrowProbe              
{

	//	  Calculates the values for the logDFTtrForm (stored as an OffMap), but with a Circular Que for speed and accuracy 

public:
	DFTrowProbeCircQue(    SPitchCalc&  sPitchCalc,    long  cellLength    );   
	virtual ~DFTrowProbeCircQue();



	void			Init_CircleQue_ArrayElements_and_Sums();     //  erases Sums and Array Elements

	void			Init_CircleQue_Sums_Only();      // **** CAREFUL,  had a lot of problems with this when it was in  SPitchCalc::Initialize_for_Play()    9/2012



	bool			Transform_Row(   SndSample&  sndSample,   long  eventsOffset,    long  samplesToProcess,    CString&   retErrorMesg   );    //   will process ALL the bytes in the incoming  SndSample  [ similar to DFTrow::Transform_Row() 


	void			Tap_CircQue_Write_Pixel_to_logDFT(    long  xWrite,     double  chunkSumsDivideRedux  );



public:
	long    *m_sinCircQue;
	long    *m_cosCircQue;


	long      m_sumCosQue,    m_sumSinQue;     

	long      m_circQuesIndex;     //  an index to the CircQue


	long      m_chunksProcessed;    //  NEW,  careful    I do NOT think that I use this info.   9/2012

	short     m_lagXwriteDFTpixel;    //  ( NOT USED??  9/2012 )     how far  horizontally-back  in the DFTmap do we have to write the pixelMag so that it is centered.
};




////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_SPITCHCALC_H__119__INCLUDED_)
