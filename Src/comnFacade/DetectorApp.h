// DetectorApp.h  -  
//
//
//  Copyright (C) 2007   Creative Detectors (of San Francisco, California).  All Rights Reserved.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#if !defined(AFX_DETECTORAPP_H__9EEB09B9_F884_44B2_ABD2_ADA7038BE3DF__INCLUDED_)
#define AFX_DETECTORAPP_H__9EEB09B9_F884_44B2_ABD2_ADA7038BE3DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////    NOTES ( start  )     ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
/***

I) When creating a NEW UniEditorApp,  must define and create:
	A)  New ChildComponent  (  ScalepitchSunhect, OvalSubject  )
		1)  Define Clone,  =Operator,  etc  ....everything like OvalSubject
	B)  New subclasses of Commandsj, even for editing  (  CommandSPitch  )
	C)  New subclasses of  Propertyj  for the specific new  ChildComponent 
	D)  Define new File save/load with  Create_ComponentExternal()
	E)  In  main.cpp, install all  the GLOBAL function ( GenerateEditors, OpenWindow_GLB, etc )


II)  ...new  'DetectorApp'
	A)  New Bitsources for the source and separation file
	B)  A new Detector  ( like ChannelDetector )   ...want new Detector class and subclass 
	c)  New strategy for showing Bitmaps  from the Detector  ( new abstract class  BitmapAdmin  ???  )



***/
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////





class  ComponentSubject;
class  ComponentView;
class  ListComponentSubject;


class   SourceAdmin;
class    BitSource;


// class   ProjectObjectDetector;   ...NO!!!

//class  Toolj;
//class  Editorj;

//class  Viewj;
//class  UniWindow;


//class  AnimePacket;   //  FeedBack-Packet  



////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


class   DetectorApp     :  public  UniEditorApp			
{

public:
	DetectorApp();
	virtual ~DetectorApp();



	virtual     bool				Init_New_Project(   short  projectCode,    CString&  retErrorMesg   );

	virtual     void				Reset_ProjectSubject();             //  Not as involved as Init_New_Project(),  deletes DZones, etc 


	virtual     bool			    Is_Projectsubject_Loaded();       //  Now works OK?? 9/06,     seems OK[ 4/07 ]

	virtual     bool				Uses_Same_Project_That_Is_Loaded(   CString&  newProjectsPath,   bool&  retUsesSameProject, 
														                                           CString&  retErrorMesg   );




																		//    SOURCE-file  functions
	
	virtual     bool				Choose_SRC_File_for_Viewing(   CString&  retErrorMesg   ) =0;

	virtual     bool	            Find_and_Init_Bitsource_SRC_File(   ComponentExternal&  componentExternal,    CString&  sourceFilesPath,  
																		              bool&  retSRCfilePathWasChanged,    CString&  retErrorMesg  );



//	virtual     bool			    Initialize_BitSource(   CString&  srcFilesPath,    bool&  retFileAcessError,    CString&  retErrorMesg  ) =0;   
	virtual     BitSource*		Initialize_BitSource(   CString&  srcFilesPath,    bool&  retFileAcessError,    CString&  retErrorMesg  ) =0;   





	virtual     bool	            SRC_File_Is_Loaded();

	virtual     CString&			Get_Source_Files_Path()     {   return   m_sourceFilesPath;   } 

	virtual     void				Release_and_Close_SRCfile();   


	virtual     SourceAdmin&       Get_SourceAdmin()   =0;              // these have to do with loading SEPARATIONS in the SRC-files place   5/07
	virtual     bool			          Initialize_SourceAdmin(   CString&  retErrorMesg   ) =  0;    

	virtual     bool				      Switch_Current_BitSource(    short bitSourceCode,    CString&  retErrorMesg   ) =  0;    //  no file loading
	virtual     bool		              Choose_Current_BitSource(  short bitSourceCode,    CString&  retErrorMesg   ) =  0;    //  done from the MENU










	virtual     CString&	          Get_Project_Files_Path()     {   return  m_projectFilesPath;  }
																			// This value is funny because it is empty when loads first Project or Notelist, but
																			// even if the user does a Notelist Only load, it then gets a value. Do Not use this
																			// to test if a Project is loaded( use the function )   4/07
	


	virtual     short				  Get_Project_Code() = 0;   //   like   Editorj::PITCHEDITOR




	virtual     bool		        Peek_At_ProjectFiles_SRC_File_Path(   ComponentExternal&  compProjectExternal,    CString&  projectFilesPath,    CString&  retScrWAVfilesPath,   
																					            short&  retFilesVersion,       CString&  retErrorMesg   );





	virtual	  bool				Query_User_For_Additional_File_Save(   ComponentExternal&  compExternal,    CString&  headersProjectPath,    bool& retUserBailsOut,     CString&  retErrorMesg   ); 

	virtual     bool				Query_User_For_Project_Save(             ComponentExternal&  compProjectExternal,    bool&  retUserBailsOut,    CString&  retErrorMesg   );


	virtual     bool				Query_User_For_Project_Load_Before_ObjList_Load(    ComponentExternal&  componentExternal,   CString& retHeadersSRCfilePath,   
																							                              bool&  retUserCanceledProjectLoad,      CString&  retErrorMesg   );

	virtual     bool				Query_User_For_SRCfile_Location(    ComponentExternal&  projectExternal,   CString&  retHeadersSRCfilePath,  											 
																								    	bool&    retSRCfilePathWasChanged, 	  CString&  retErrorMesg   ); 






	virtual     void				Record_Loaded_Projects_Name(   CString&  projectsFilePath   );      


	virtual     bool		        Load_ProjectSubject(     ComponentExternal&  compProjectExternal,    CString&  projectFilesPath,  
																											bool&  retUserCanceled,      CString&  retErrorMesg   );

	virtual     bool				Do_Sources_PostLoad_Calcs(   CString&  retErrorMesg   )   {  return  true;  }   //   dummy,  do things like build Amplitude maps etc 


	virtual     bool	            Save_ProjectSubject(    ComponentExternal&  compProjectExternal,    CString&  filePath,   CString&  retErrorMesg   );







																			//   INTERFACE

	virtual     Viewj*		                 Get_An_Open_Animation_Viewj(   short  viewjType,    CString&   retErrorMesg   );

	virtual     ComponentView*      Get_An_Open_PROJECTpane(   Viewj  **retViewPtr,    CString&   retErrorMesg   ); 
												                  //  PROJECTpane:    a window that shows SOURCE DATA( not a detected result like the OutputList pane )

	virtual     bool						 Open_Detector_Windows(   CString&  retErrorMesg   ); 

	virtual     bool						 Open_Animtion_Window(    short  viewjType,     CString&  retErrorMesg  );




																		//   MENU Commands   (  High-level  )


	virtual     bool			 On_Load_ProjectFile(   ComponentExternal&  compProjectExternal,    CString&  retErrorMesg  ); 

	virtual     bool			 On_Save_Current_ProjectFile(   ComponentExternal&  compProjectExternal,   CString&  retMesg  );    
//  virtual     bool			 On_Save_ProjectFile_As(   CString&  retMesg  );    ...BAD idea,  never do it.  4/07

	virtual     bool			 On_New_Project(  ComponentExternal&  compProjectExternal,    CString&  retErrorMesg   );       //  DUMMY,  for forwarding the calls



	virtual     bool             On_View_SRC_File(  ComponentExternal&  compProjectExternal,   CString&  retErrorMesg   );








public:
	CString       m_sourceFilesPath;		  //   WAS:   m_streamingFilesPath     'FILE'  :   (  the  'Primary-DATA'   Source )

	CString       m_projectFilesPath;      //  of the  'PROJECT-file'    ( NOT the  Source-file )


	CString       m_lastProjectFileLoaded;   // to help figure out  what and if the Currently Load project is with  Is_Projectsubject_Loaded()



	bool          m_projectIsModified;	   //  the triggers that remind the user to save UNsaved changes
};





////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_DETECTORAPP_H__9EEB09B9_F884_44B2_ABD2_ADA7038BE3DF__INCLUDED_)
