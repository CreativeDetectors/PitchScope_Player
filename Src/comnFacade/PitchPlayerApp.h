/////////////////////////////////////////////////////////////////////////////
//
//  PitchPlayerApp.h   -   
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////

#pragma once



////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////    NOTES ( start  )     ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
/***

 When creating a NEW UniApplication,  must define and create:

	A)  New ChildComponent  (  ScalepitchSunhect, OvalSubject  )
		1)  Define Clone,  =Operator,  etc  ....everything like OvalSubject
	B)  New subclasses of Commandsj, even for editing  (  CommandSPitch  )
	C)  New subclasses of  Propertyj  for the specific new  ChildComponent 
	D)  Define new File save/load with  Create_ComponentExternal(),  need a FileObj struct,   AssignMe_from_FileObject(),  
	E)  In  main.cpp, install all  the GLOBAL function ( GenerateEditors, OpenWindow_GLB, etc )

***/
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////




class   UniApplication;
class   SourceAdminAudioMS;


class   BitSource;
class   BitSourceStreaming;
class   BitSourceStreamingMS;

class   WavConvert;

class   CalcedNoteListExternal;

class   CalcedNote;
class   MidiNote;

class   Lick;



////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


class    PitchPlayerApp      :   public   UniApplication    
{


public:
	PitchPlayerApp();
	virtual ~PitchPlayerApp();



	virtual		BitSourceStreaming*	  Get_BitSource_Streaming(   short bitSourceCode,   CString&  retErrorMesg   );

	virtual		WavConvert*           Get_WavConvert(   CString&  retErrorMesg   );    //  LOW Level,   should   **REMOVE**   from this class



																								//   NOTELIST  ops 


	virtual       bool		  Record_Notes(   CString&  retErrorMesg   );     

	virtual		bool		  Make_NoteList_No_Audio(    CString&  srcFilesPath,   short  musicalKey,     CString&  retErrorMesg   );   


	virtual      void				Set_DSTlist_Modified(  bool  isModified )   {  m_dstListIsModifiedOLD  =  isModified;  }    //  SUBclasses overide this 

	virtual      bool				Is_DSTlist_Modified()                              {  return  m_dstListIsModifiedOLD;    }

	virtual      bool				Is_DST_List_Empty();



																								//   FILE ops 


	virtual		long          Get_ChunkSize_Navigator()    {   return  1104;   }    //  NEW   9/2012


					long		    Get_Last_FilePosition()                                    {   return  m_lastFilePosition;  }
					void		    Set_Last_FilePosition(  long  lastSampleIdx  )    {   m_lastFilePosition =   lastSampleIdx;  }

	
	virtual		short			Get_Files_Version_Number()     {   return    m_filesVersionNumber;   }	 //    12,   and can moveup to  19  for exeVersionNumber = 20
	virtual		short			Get_EXEs_Version_Number()     { return    m_exeVersionNumber;    }	 //    20   ( 20: Navigator, Player )   [ should NOT be used much ]



	virtual		bool			  SRC_File_Is_Loaded();

	virtual		bool			  Has_A_MP3();  

	virtual		CString&	      Get_Source_Files_Path()     {   return   m_sourceFilesPath;   } 




					bool			On_View_SRC_File(    bool&  retUserCanceled,      CString&  retErrorMesg   );

	 virtual		bool			Choose_SRC_File_for_Viewing(    bool&  retUserCanceled,     CString&  filePathNoDialog,     CString&  retErrorMesg  );       //  is a virtual function for DetectorApps

	 virtual     void			Release_and_Close_SRCfile();

	
	virtual		bool			Load_NoteList(   CString&  listFilesPath,     bool&  retUserCanceled,     CString&  retErrorMesg   );
	virtual		bool		    Save_DSTlist(    bool&  retUserCanceled,     CString&  retListFilesPath,    CString&  retErrorMesg   );


	virtual		bool			Peek_At_NoteList_Files_Header(   CalcedNoteListExternal&  compExternal,   CString&  listFilesPath,   short&  retFilesVersion,    CString&  retErrorMesg   );

	virtual		bool			Browse_For_Missing_File(  CString&  filesOldPath,   bool&  retUserCanceled,    CString&  retFilesNewPath,   CString&  retErrorMesg   );

	virtual		bool			Query_User_For_SRCfile_Location(   CString&  retHeadersSRCfilePath,   bool&    retSRCfilePathWasChanged,	   CString&  retErrorMesg   ); 

	virtual		bool			Query_User_For_ObjectList_Save(     bool&  retUserCanceled,     bool& retUserBailsOut,    CString&  retErrorMesg    );
	virtual		void			Query_User_For_ObjectList_Save_On_App_Exit(); 



																								//   ODD   functions


	                bool		  Change_LogDFT_Probe_Algo(    bool   useCircularQues,   CString&  retErrorMesg   );


																							//   MISC   functs


					void			Set_Computer_Performance_Factor(   long  computerPerformanceFactor   );   //   writes to WavConvert,   is ONLY for NAVIGATOR
					long			Get_Computers_Performance_Factor()   {   return   m_computerPerformanceFactor;   }     //   3 values,       0: Fast     1: Average     2: Slow

			

	virtual		bool		  Change_PlaySpeed_Member_Variables(   double   nuPlaySpeed,    bool   wasPlaying,    CString&  retErrorMesg  );


	static			ListMemry< LongIndexed >*    Make_Event_List(   long  startOffset,  long endOffset,     short stereoChannelCode,  
		                                                                                        ListDoubLinkMemry<MidiNote> *noteList,    long&  retNoteCount,   CString&  retErrorMesg  );




																									//   BITSOURCE  Administrator

	virtual     SourceAdmin&      Get_SourceAdmin()      {  return  m_sourceAdmin;   }

	virtual     bool	                  Initialize_SourceAdmin(   CString&  retErrorMesg   );    


	 virtual	 BitSourceStreaming*	  Initialize_BitSource(   CString&  srcFilesPath,    bool&  retFileAcessError,    CString&  retErrorMesg  );

	 virtual      bool						      Switch_Current_BitSource(     short  bitSourceCode,    CString&  retErrorMesg   );  

	 virtual      BitSourceStreaming*	  Get_Unused_BitSource(  short   currentBitSourceCode,   CString&  retErrorMesg   );  




																								//   VIEW  rendering   functions
	

	virtual		void		  ReDraw_All_Anime_Viewjs();           //   used to draw Last FRAME of some animating objects:   BULLETS for Gauges

	virtual		void		  Animate_All_Viewjs(  long  curSample,   short curScalePitch  );   //   Moves the FILE-SLIDER on the dialog  ( CALLED by  Process_Event_Notification_PPlayer() 


	virtual		void		  ReDraw_Bitmaps();
	virtual		void		  ReDraw_Bitmaps(   bool  renderWithoutText   );    //   NEW,  just to redraw the DrivingBitmaps.   Can I use this in OTHER  'UniApplication'  APPS???     2/2012




																							//   DUMMY   functions to keep compiler happy


	virtual		bool		  Create_Animating_CDCs_All_Viewjs()         {   return true;  }    //  dummy default
	virtual		void		  Release_Animating_CDCs_All_Viewjs()        {   }   //  dummy default

	 virtual		void			 Update_FirstSampleIdx_All_Viewjs(            long  sampleIdx   );  //   Moves the Scroll slider for the main dialog
	 virtual		void             Update_FirstSampleIdx_All_AnimeViewjs(       long  sampleIdx   );      
	 virtual		void		     Update_FirstSampleIdx_Active_RankingViewj(   long  sampleIdx   );  





public:
	SourceAdminAudioMS    m_sourceAdmin;    //   this wrapper hold ALL the BitSources ( one for WAV files and one for MP3 files  )


	ListDoubLinkMemry< MidiNote >    m_calcedNoteListMaster;     //  These are the notes that we HEAR and SEE on the display.

	ListDoubLinkMemry< MidiNote >    m_calcedNoteListTemp;       //  Recorded notes go directly here,  then they can be saved to hard-disk


	ListMemry< Lick >	    m_lickList;      //   Short Musical  PHRASES



												//    'FILE'  variables  


	CString       m_sourceFilesPath;		             //   WAS:   m_streamingFilesPath     'FILE'  :   (  the  'Primary-DATA'   Source )



	short           m_exeVersionNumber;    //    20    ( 2.0 )

	short			m_filesVersionNumber;   //     12    
			//  Alway keep FILE versions 9 or 8  digits lower than the current EXE version[ 10 ],  to that I can release
	        //  CORRECTED File verions upward without mandating a release of a new EXE( with a higher version )   1/06



	CString       m_lastObjectFilesPathLoaded;   //   for NoteList from file:  

	CString       m_currentListFileName;            //   for NoteList from file:     Name of last NoteList saved or loaded, if still with the current Project    NEW,  6/2012 


	bool            m_dstListIsModifiedOLD;   



	long            m_lastFilePosition;                   //    this  is/was  AudioPlayer::m_lastPlayedSample     In VoxSep this is in Editorj

	short			m_currentBitSourceCode;       //   used by    PitchPlayerApp::Switch_Current_BitSource()




												 //    secondary  SMALL  'Wav-DELAY'  for  Player.exe
	
	long		m_baseDelay; 
	long		m_overideDelay;    //  Special MODE:   If this has a value  >=0,  then that is the REAL DELAY to use,  and  m_baseDelay is processed as ZERO   

	long        m_noteListHardwareDelayForPlayer;     //   Player's  DELAY  for   NOTELIST-play  MidiSource MODE   [  ***INIT:   Fido[ 0 ]    Sparky[ 5 ]     Lassie [  10 ]      8/2012
                                                                         //   ***REMEMBER,  this is ONLY for Player with NoteList-PLAY,  and it will be RARE that Player.exe LOADS a notelist, in reality.  8/2012




												//    'STATE'  variables   (  typically variables that the  Dialog's CONTROLs  can change  )


                // *************   Many of the  Shared Settings  of { Navigator, Player }   should be located in SoundHelper instead of App.   ************ 


	short        m_detectionSensitivityThreshold;

	short		 m_rightChannelPercent;             //  In Realtime Pitch Detection, this describes the ratio of left/right signals to analyize
																   //  It can also affect the AUDIBLE sound as well, dependant on how I set the switch   3/11

	bool         m_supressNotesTextDrivingViews;



	short		m_sizeOfPrimNotesCircque;       //   controls   "Note DETAIL"      this is to SMOOTH out the ouputed Note Results 

	short		m_numberNeededToMatch;       //   controls   "Note DETAIL"     ...How many in   NotesCircque   must match to make a decion to play a different note. 



	double		m_playSpeedFlt;     // ****  This might belong in SoudHelper   8/2012  ********



	bool		m_usingCircQueDFTprobes;      //   NEW,  2 ways to do the calc   9/2012

	long        m_computerPerformanceFactor;    //   3 values       0: Fast     1: Average     2: Slow



	long         m_wavsVolumeApp;         //    { 0 - 100  }   ...in percent    One for All channels   ( moved here from SoundHelper  ...better encapsulation

	long         m_midiVolume;               //    { 0 - 100  }   ...in percent 


	short        m_stereoChannelCode;    //   does this ever get used now ???    NO,  But  Allocate_AudioPlayer() needs its address.





	/****
	virtual		ListComponentSubject&	   Get_DST_List()      {  return  m_DSTobjectsListSubject;  }

	virtual      bool			                      Is_DST_List_Empty();        



	virtual      void				Set_DSTlist_Modified(  bool  isModified )   {  m_dstListIsModifiedOLD  =  isModified;  }    //  SUBclasses overide this 

	virtual      bool				Is_DSTlist_Modified()                              {  return  m_dstListIsModifiedOLD;    }




	virtual		ComponentView*                         Get_An_Open_DSTpane(   Viewj  **retViewPtr,    CString&   retErrorMesg   ); 

	virtual		ComponentView*					      Get_An_Open_ListEditor_Pane(   Viewj  **retViewPtr,    CString&   retErrorMesg   ); 


	virtual     ListMemry<ComponentView>&        Get_Total_Pane_List();   //  See   listSubject.Attach()   in  GrafixListWindow::Add_Pane_ObjectDraw()




																	//    'EDITOR'   and Selection

	 virtual		Editorj*			                   Get_Cur_Editor()		{  return  m_curEditor;   }  


	virtual		ComponentSubject*          Get_A_Selected_ListObject(  CString&   retErrorMesg   ); 

	 virtual		void								   Hide_and_Release_All_Selections();

	 virtual		bool								   Select_Next_ComponentView_In_List(        CString&   retErrorMesg  );
	 virtual		bool								   Select_Previous_ComponentView_In_List(  CString&   retErrorMesg  );

	 


																//    File-CURSOR  ( curSampleIdx )  


	 virtual		void			 Update_FirstSampleIdx_All_Viewjs(   long  sampleIdx   );

	 virtual		void           Update_FirstSampleIdx_All_AnimeViewjs(   long  sampleIdx   );

	 virtual		void		     Update_FirstSampleIdx_Active_RankingViewj(   long  sampleIdx   );    // will do AnimeViewj if no EditViewj is Present

	 virtual		bool			 Sync_All_Viewjs_To_ActiveViewj(   CString&  retErrorMesg   );   // does zoom and offset



																//   'FILE'  ops


	 virtual		short				 Get_EXEs_Version_Number()     {   return    m_exeVersionNumber;   }	
													    // test that files with a GREATER version number are not loaded to this build( version number )	  
	 

	virtual		bool				  Are_FileTargets_The_Same(  CString&  path1,  CString&  path2,    bool& retAreTheSame,   CString&  retErrorMesg   );



	 virtual		bool				  Is_File_Present(   CString&  filesPath,    CString&  retErrorMesg   );


	  virtual     bool				  Peek_At_ListFiles_ProjectPath(   ComponentExternal&  compProjectExternal,    CString&  listFilesPath,    CString&  retProjectFilesPath,   
																					         short&  retFilesVersion,     CString&  retErrorMesg   );   


	 virtual		bool				  Browse_For_Missing_File(   CString&  filesOldPath,   bool&  retUserCanceled,   CString&  retFilesNewPath,    CString&  retErrorMesg   );



	 virtual		bool			      Load_DSTlist(   ComponentExternal&  compExternal,   CString&  listFilesPath,     CString&  retErrorMesg    ); 
	 virtual		bool			      Save_DSTlist(    ComponentExternal&  compExternal,    void* extraData,    bool&  retUserCanceled,  CString&  listFilesPath,  CString&  retErrorMesg    ); 

	 virtual		bool				  New_DSTlist(   CString&  retErrorMesg   );


	 virtual		void				  Record_Loaded_ObjectFiles_Path(   CString&  objectFilesPath   );



	 virtual		bool				  Query_User_For_ObjectList_Save(   ComponentExternal&  compExternal,    bool&  retUserCanceled,    bool& retUserBailsOut,    CString&  retErrorMesg   );    //  new,  4/07

	 virtual		bool				  Query_User_For_Additional_File_Save(   ComponentExternal&  compExternal,   CString&  headersProjectPath,   bool& retUserBailsOut,   CString&  retErrorMesg   ) 
																							                          {  return  true; }    // DUMMY,   is overidden in DetectorApp to save Project files




																//    'Viewj  ADMISTRATION' 



	 virtual		ListMemry<Viewj>&	   Get_ViewjList()          {  return   m_viewjList;     }     //  ( ***BETTER in Editorj ?? ***   3/03  )


	 virtual		Viewj*       Get_Active_Viewj();				      //   includes   AnimeViewjs  like   Revolver,  3Dmountains

	 virtual		Viewj*       Get_Active_ListSubject_Viewj();     //   EXCLUDES  Revolver


	 virtual		bool		    Set_ListSubjects_BackroundMap(   int  mapCode,    CString&  retErrorMesg   ); 



	 virtual		void			Draw_All_Viewjs_Titles();

	 virtual	 void				ReDraw_Titles_On_Special_Viewjs(   short  viewjsConfigurationCode,   CString&  newTitle  );




	virtual     bool		  Open_ListEditor_Windows(   CString&  retErrorMesg   ); 

	virtual     bool		  Open_Animtion_Window(    short  viewjType,     CString&  retErrorMesg  )   {   retErrorMesg =  "Open_Animtion_Window NOT implemented. ";   return  false;  }



	 

	 virtual		bool		   ReBuild_ReDraw_All_ChildComponentViews(   CString&   retErrorMesg   ); 
	 
	 virtual		void		   ReDraw_All_Viewjs();      // ***??? Is this the CLEANEST way to do this[ better with Notify() ???  ***
	 virtual		void		   ReDraw_All_Anime_Viewjs();
	 virtual		void		   ReDraw_Active_Viewj();
	 virtual		void		   ReDraw_Active_ListViewj();

	 virtual		void		   ReDraw_Only_Special_Viewjs(   short  viewjsConfigurationCode  );      //  for overides      NEW,  8/08




	 virtual		int             Count_Viewjs();

	 virtual		Viewj*       Fetch_Viewj(  void  *machineWind   );      

	 virtual		void           Activate_Viewj(   Viewj&  view   );
	 virtual		void		     Activate_A_ListEditor_Viewj();


	 virtual		bool		   Register_Viewj(   UniWindow  *windAddress,    ListMemry< ComponentView >&  stationaryComponentViewList,  
											                       RECT&  winBoundBox, 	Viewj  *nuViewj,    CString&  retErrorMesg    );

	 virtual		bool		   Register_Viewj_Simple(   UniWindow  *windAddress,    RECT&  winBoundBox, 	Viewj   *nuViewj,    CString&  retErrorMesg    );




	 virtual		Viewj*		   Alloc_New_Viewj(    Editorj&  editor,    short  channelCode   );

	 virtual		bool			Open_Editors_Window(   short channelCode,   int  iconResourceID,     int menuResourceID,   
															                int   popupMenuResourceID, CMDIFrameWnd  *parentCMDIFrameWnd,  
																			short  configurationCode,   CString&   retErrorMesg   ); 


	 virtual		bool             Open_UniWindow(    short channelCode,  	  int  iconResourceID,     int  menuResourceID, 
											         int	 popupMenuResourceID,    CMDIFrameWnd  *parentCMDIFrameWnd,  
											         short  viewMajorType,    RECT&  winBoundBox,    short  paneCount,   
													 long   windowStyleFlags,   short  configurationCode,   UniWindow  **retUniWindow,    CString&  retErrorMesg   );

	 virtual		bool		     Open_Anime_Window(    short channelCode,  	 int  iconResourceID,     int  menuResourceID, 
													       int	 popupMenuResourceID,     CMDIFrameWnd  *parentCMDIFrameWnd,  
											               short  viewMajorType,    RECT&  winBoundBox,   short  paneCount,   
														   long   windowStyleFlags,    CString&  retErrorMesg   );

	 virtual		void			Get_Edit_Windows_Title(    short  configurationCode,    short  viewMajorType,    short channelCode,     
																	           CString&  retTitle,   	CString&  retName    ); 	 




	 virtual		bool           Close_Window(   CWnd  *wind,    CString&  retErrorMesg   );  //  total dealloc with Window's OS 
	 virtual		bool		   Close_All_Windows(    CString&  retErrorMesg    );


	 virtual		bool		   Update_Interface_After_ChildComponent_Edit(  CString&  retErrorMesg  );

	 virtual        bool            ReBuild_All_AmineMasks(  CString&  retErrorMesg   )   //  dummy function for OVERLOADING by subclasses
																										{   return  true;   }


	 virtual		void		   Notify_All_ComponentViews_forReDraw();   //  If there is a Project-class,  will just forward Command to  'ProjectObjectDetector'  



	 virtual		bool		   ReCreate_Child_ComponentViews_AllViewjs(    CString&  retErrorMesg   );   																					
	 virtual		bool           Delete_All_Child_ComponentViews_AllViewjs(   CString&  retErrorMesg   );    //   new  1/04   ...USE IT!!!

	 virtual		bool		   Add_Child_ComponentView_AllViewjs(  ComponentSubject&   componentSubject,   ComponentView&  paneViewUser, 
																		             ComponentView   **retNewChildView,   	CString&  retErrorMesg   );



	 virtual		void		   Update_Horizontal_Zoom_Active_Viewj(    bool   minusFlg    );   //  give more   'GENERIC'  Input-PARMS

	 virtual		void		   Update_Vertical_Zoom_Active_Viewj(        bool   minusFlg    ); 
	 virtual		void		   Set_Vertical_Zoom_Active_Viewj(  short  zoomValue  );

	 virtual		void		   Update_Vertical_Offset_Active_Viewj(  long  offset   );


	 virtual		bool         Get_Minimum_Vert_Zoom(    long&  retMinZoom,    CString&  retErrorMesg   );



																//    too   'Context-SPECIFIC'  ???     ...MOVE these functions,  make generic.
																
																//  VISITOR Pattern:  could do it by passing in an ABSTRACT class whose subclasses ( VisitorStereoValue.. etc ) 
																//   know how to change a variable
												


	 virtual		bool		   Change_StereoChannel_All_Viewjs(   short  channelCode,    CString&   retErrorMesg   );

	 virtual		void		   Update_3D_RenderCode_All_Viewjs(  short  renderCode  );




				//   Animation  &   Feedback   ( ***MOVE to another CLASS??  But is a VIEWJ administration, so belongs here.  Think abstract.  *** ) 
				//												....MAYBE use the   'NOTIFICATION mechanism'   to do animation.   4/03


	 virtual		bool		  Create_Animating_CDCs_All_Viewjs();
	 virtual		void		  Release_Animating_CDCs_All_Viewjs();    

	 virtual		void		  Set_Anime_Range_Mode_AllPanes(   short  animeMode   );  //  COULD also set OTHER Parms as well...


	 virtual		void		  Animate_All_Viewjs(   long  curSample,    short curScalePitch    )    { ASSERT( 0 );  }           	//  dummy default 




	 virtual		AnimePacketAbstr*    Alloc_AnimePacket(   CString&  retErrorMesg   )  
																			{      return  NULL;  }  //  OK to let this go
																							// when there is NO real AnimePacket ( UniDraw,  GFXRegion  )


	 virtual		bool		                   Initialize_AnimePacket(    long  curSample,      long  lastSamplePane,    //  DUMMY default
																		                 AnimePacketAbstr&  retAnimePacket,    CString&  retErrorMesg   );


															//   MENU Commands   (  High-level  )

	virtual		bool		On_EditMenu_Copy(   CString&   retErrorMesg   ); 
	virtual		bool		On_EditMenu_Undo(   CString&   retErrorMesg   ); 
	virtual		bool		On_EditMenu_Paste(   CString&   retErrorMesg   ); 
	virtual		bool		On_EditMenu_Cut(   CString&   retErrorMesg   ); 
	virtual		bool		On_EditMenu_Delete(   CString&   retErrorMesg   ); 
	virtual		bool		On_EditMenu_SelectAll(   CString&   retErrorMesg   ); 



																						//  generic  File  menu commands for UniEditorApps

	virtual       bool		On_Save_Current_ObjectList_File(   ComponentExternal&  compExternal,      CString&  retMesg  );    
	virtual       bool		On_Save_ObjectList_File_As(   ComponentExternal&  compExternal,    bool&  retUserCanceled,   CString&  retMesg  );   

	virtual       bool		On_Load_ObjectList_File(   ComponentExternal&  compExternal,   CString&  retMesg  );   

	virtual     bool		On_New_ObjectList_File(   ComponentExternal&  compExternal,   CString&  retErrorMesg   );    


//	virtual     void		Make_A_Base52_Numeric_String(   short  number,    CString&  retNewNumberString   );    *** CREATE this some day    10/08 *******




public:
 	 ListMemry< Viewj >         m_viewjList;      //  they RESIDE here ( ***BETTER in Editorj ?? ***   3/03  )


	 ListComponentSubject      m_DSTobjectsListSubject;  //  NEW,   not always used if UniEditorApp subclasses( ScalepitchList )
																				  //   have their own lists.    5/06

	 CString      m_currentListFileName;   //  name of last NoteList saved or loaded, if still with the current Project

	 CString      m_lastObjectFilesPathLoaded;  


	 Editorj     *m_curEditor;  
														//  list  ALL  possible   'ToolPallet CONFIGURATIONS' ( i.e.  Editor TYPES )
	 int            m_toolPalletCount;

	 short       m_exeVersionNumber;


private:
	bool         m_dstListIsModifiedOLD;   

	****/

};



