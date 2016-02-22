// DetectorApp.cpp: implementation of the DetectorApp class.
//
//
//  Copyright (C) 2007   Creative Detectors (of San Francisco, California).  All Rights Reserved.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"



#include   "..\comnFacade\UniEditorAppsGlobals.h"




//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"      //  #include <FixMath.h>    ...Mac's version of Fixedpoint 
#include  "..\ComnGrafix\CommonGrafix.h"      //  #include "commonmy.h"   	



///////////////////////////							 ...INTERFACE
#include  "..\comnInterface\gnStatevar.h"

#include  "..\comnCatalog\External.h"
#include  "..\comnInterface\gnUniBasic.h"

#include  "..\comnInterface\gnVuCues.h"	 

#include  "..\comnInterface\gnPane.h"   
#include  "..\ComnGrafix\PaneMemory.h"

#include   "..\comnInterface\ListComponentSubject.h"       //  NEW	
#include   "..\comnInterface\ListComponentView.h"

#include  "..\comnInterface\gnView.h"

#include  "..\comnInterface\gnEditor.h"   	 

//#include  "..\comnInterface\gnProperty.h"     
//#include  "..\comnInterface\gnManipulators.h"
//#include  "..\comnInterface\gnCommands.h"
//#include  "..\comnInterface\ClipBoardj.h"



#include "..\comnInterface\DetectorAbstracts.h"    // **** NEW ****

///////////////////////////




#include "..\comnFacade\UniEditorApp.h"

#include "DetectorApp.h"

/////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////



//  UniEditorApp&     Get_UniEditorApp();    ...keep encapsulated
//  DetectorApp&     Get_DetectorApp();  





////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


DetectorApp::DetectorApp()
{

	CString  retErrorMesg;

	m_projectIsModified =    false; 	
	
	m_lastProjectFileLoaded.Empty();
}


									/////////////////////////////////////////////////////


DetectorApp::~DetectorApp()
{
}




									/////////////////////////////////////////////////////


void	  DetectorApp::Record_Loaded_Projects_Name(   CString&  projectsFilePath   )      
{


	if(   projectsFilePath.IsEmpty()   )
	{
		m_lastProjectFileLoaded.Empty();
		return;
	}


	CString   origFileName;

	int   slashPos  =    projectsFilePath.ReverseFind(  '\\'  ); 
	if(    slashPos > 0   )
				origFileName =    projectsFilePath.Right(    projectsFilePath.GetLength()  -  (slashPos +1)    );
	else      origFileName =    projectsFilePath;


	ASSERT(   ! origFileName.IsEmpty()    );


	m_lastProjectFileLoaded  =   origFileName;
}



									/////////////////////////////////////////////////////


bool	  DetectorApp::Is_Projectsubject_Loaded()     
{


	if(     m_projectFilesPath.IsEmpty()     ) 
		return  false;


	if(     m_lastProjectFileLoaded.IsEmpty()     )    //  just the name,  no path
		return  false;



			//   Because a Notelist-ONLY load must also write the 'unloaded' ProjectName( m_projectFilesPath ) ,  we 
			//   must also VERIFY with m_lastProjectFileLoaded 

	CString   curProjectName;

	int   slashPos  =    m_projectFilesPath.ReverseFind(  '\\'  ); 
	if(    slashPos > 0   )
				curProjectName =    m_projectFilesPath.Right(    m_projectFilesPath.GetLength()  -  (slashPos +1)    );
	else      curProjectName =    m_projectFilesPath;



	if(    m_lastProjectFileLoaded.CompareNoCase(  curProjectName  )  ==  0   )
		return   true;
	else
		return   false;
}



									/////////////////////////////////////////////////////


void	  DetectorApp::Reset_ProjectSubject()     
{

	Get_Source_Files_Path().Empty();  

	//  ????  Should   Release_and_Close_SRCfile()   be called with this ???


	m_lastProjectFileLoaded.Empty();   // ***** THIS OK too  ???  ******
}



									/////////////////////////////////////////////////////


void		DetectorApp::Release_and_Close_SRCfile()
{

								//  Child classes, like  PitchDetectorApp,  MIGHT want overide this funct 

	Get_SourceAdmin().Release_All_BitSources_Files();    //  OK ????   5/16/07

	Get_Source_Files_Path().Empty();   // *****  WANT this too ???     5/16/07  ********  
}


									/////////////////////////////////////////////////////


bool   DetectorApp::SRC_File_Is_Loaded()
{

	if(    m_sourceFilesPath.IsEmpty()    )
		return false;
	else
		return true;
}

									/////////////////////////////////////////////////////


bool     DetectorApp::Peek_At_ProjectFiles_SRC_File_Path(   ComponentExternal&  componentExternal,    CString&  projectFilesPath,    CString&  retScrWAVfilesPath,   
																					            short&  retFilesVersion,       CString&  retErrorMesg   )
{
	retErrorMesg.Empty();
	retScrWAVfilesPath.Empty();
	retFilesVersion =  -1;


	try
    {  CFile      file(    projectFilesPath, 
	                           CFile::modeRead      //   |   CFile::shareExclusive 								
				  //	   |  CFile::typeBinary   ...BUGGY ????    5/02    Sets binary mode (used in derived classes only).
					     );


		unsigned char   retVersion;									//	A)    load  the   File-CREATOR	 TAG
		long                 retChunksSize;  

		if(    ! componentExternal.Verify_FileCreator_Tag(   file,   retVersion,   retErrorMesg  )     )
			return  false;

		retFilesVersion  =    retVersion;





																				//	B)    load  PART OF  the   'List-HEADER'

		if(   ! componentExternal.Goto_Files_Header(   file,    retVersion,   retChunksSize,   retErrorMesg  )    )
			return  false;



		HEADERsIntro   projectHeaderStruct;    //  just the first part

		long  ldSize  =    ( long )sizeof(  HEADERsIntro  );        
		file.Read(   &projectHeaderStruct,    ldSize  );


		retScrWAVfilesPath =     projectHeaderStruct.sibblingFilesPath;  
		if(    retScrWAVfilesPath.IsEmpty()    )
		{
			retErrorMesg  =  "RegionDetectorApp::Peek_At_ProjectFiles_SRC_File_Path  failed,  projectFilesPath is empty." ;
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

		retErrorMesg.Format(   "Can not open project-file because:  %s" ,    strRaw   );   //  Message might go to user.  9/06
		return  false;
	}

	return  true;
}




								/////////////////////////////////////////////////////
 

bool		DetectorApp::Query_User_For_Project_Save(    ComponentExternal&  componentExternal,     bool&  retUserBailsOut,     CString&  retErrorMesg   ) 
{

	bool  retVal       =   true;
	retUserBailsOut =   false;


	if(   m_projectIsModified   )  
	{


//		int   reslt    =       AfxMessageBox( "You have unsaved changes.  Do you wish to SAVE the current Project File?",  MB_YESNO     );
		int   reslt    =      AfxMessageBox(   "You have unsaved changes.  Do you wish to SAVE the current Project File?",   MB_YESNOCANCEL | MB_ICONQUESTION     );    	//   MB_YESNO      );
		if(    reslt  ==   IDYES   )
		{

			if(    ! On_Save_Current_ProjectFile(  componentExternal,   retErrorMesg   )     )
			{
				//   AfxMessageBox(  retErrorMesg  );
				retVal =    false;   //  let calling funcion launch a Error Message Box   
			}
			else
			{	m_projectIsModified  =    false; 
				AfxMessageBox(   "Project File was SAVED."   );
			}
		}
		else if(   reslt  ==   IDCANCEL   )
			retUserBailsOut =   true;
	}


	return   retVal;
}




								/////////////////////////////////////////////////////


bool		DetectorApp::Query_User_For_Project_Load_Before_ObjList_Load(    ComponentExternal&  componentExternal,   CString&  headersProjectPath,    													 
																					  							bool&  retUserCanceledProjectLoad,    CString&  retErrorMesg   ) 
{

			//  Only called by    DetectorApp::Query_User_For_Additional_File_Save()


		//  Before this function was called, we verified that the OBJlist uses a DIFFERENT project than the one loaded ( or NOT loaded )   5/07

		//  ***NOTE:  After every 	Reset_ProjectSubject();  that   'returns true',  we must REwrite the value to m_projectFilesPath( cause is a NotelistONLY Load ) 


	retErrorMesg.Empty();
	retUserCanceledProjectLoad =  false;


	CString   retErrorMesgLoc,  origFileName, mesg,  retObjectName,    finalPath =  headersProjectPath;

	componentExternal.Get_ListObjects_Name(   retObjectName  );


	bool  doesNotHaveAprojectName =   false;
	if(     headersProjectPath.IsEmpty()    )
		doesNotHaveAprojectName =   true;   //  Could be a Notelist from Navigator (has no ProjectPath in header )    3/11



	mesg.Format(  "Did you also want to load the PROJECT File for this %sList File?",   retObjectName   );

	int   reslt    =      AfxMessageBox(  mesg,   MB_YESNO | MB_ICONQUESTION    );
	if(    reslt  !=   IDYES   )
	{				

		Reset_ProjectSubject();   // **** CAREFUL,  will this mess up the load of the ????   Looks good so far  5/5/05 
		m_projectFilesPath =    headersProjectPath;   //  Even for ObjectLIST only loads, we preserve this value in case the list is later modified and needs to be saved.

		AfxMessageBox(  "Since you have chosen to load this file WITHOUT its Project File, you will NOT have access to the Project File's data (Detection Zones, etc)."  );
		retUserCanceledProjectLoad =  true;
		return  true;
	}



	if(  doesNotHaveAprojectName  )
	{
		origFileName  =   "Unknown Project"  ;
	}
	else
		Get_Paths_Target_FileName_GLB(  headersProjectPath,   origFileName );





	bool  projectNeededBrowsing =    false;  //  Though this is a OBJlist load, the OBJlist might contain a bad ProjectFile PATH (that might get corrected with the browse below



	if(      doesNotHaveAprojectName   || 
		   ! Is_File_Present(  headersProjectPath,   retErrorMesgLoc  )     )
	{

		projectNeededBrowsing =   true;     //    Load_ProjectSubject() will reset this value, so we save it for a restore

		Set_DSTlist_Modified(  true  );  


		CString   mesg,    retFilesNewPath;
					
		mesg.Format(   "%s \nDid you want to BROWSE for  %s  in another folder?" ,    retErrorMesgLoc,   origFileName    );
		int  reslt    =      AfxMessageBox(  mesg,   MB_YESNO | MB_ICONQUESTION   );
		if(   reslt  !=   IDYES   )
		{
			AfxMessageBox(  "Since you have chosen to load a this file WITHOUT its Project File, you will NOT have access to the Project File's data (Detection Zones, etc)."  );															
			retUserCanceledProjectLoad =  true;

			Reset_ProjectSubject();


			if(  doesNotHaveAprojectName   )
				m_projectFilesPath.Empty();
			else
				m_projectFilesPath =    headersProjectPath;   //  Even for ObjectLIST only loads, we preserve this value in case the list is later modified and needs to be saved.

			return  true;
		}
		 

			
		if(    ! Browse_For_Missing_File(   headersProjectPath,   retUserCanceledProjectLoad,    retFilesNewPath,     retErrorMesg  )    )  //  
		{
			ASSERT( 0 );    //  this is a real bug
			Reset_ProjectSubject();  
			return  false;  
		}
				


		if(    retUserCanceledProjectLoad   )  
		{
			Reset_ProjectSubject(); 
		    m_projectFilesPath =    headersProjectPath;   //  Even for ObjectLIST only loads, we preserve this value in case the list is later modified and needs to be saved.

			return true;
		}



		projectNeededBrowsing =  true;     //    Load_ProjectSubject() will reset this value, so we save it for a restore
		Set_DSTlist_Modified(  true  );


		finalPath                  =    retFilesNewPath;
	}





	if(    ! Load_ProjectSubject(  componentExternal,   finalPath,   retUserCanceledProjectLoad,   retErrorMesg  )     )
	{
		Reset_ProjectSubject(); 
		return  false;
	}							


	if(    ! Open_Animtion_Window(   Editorj::REVOLVERaNIME,  retErrorMesg  )     )    //  will NOT open a window if it is alread there   6/07
		return  false;

	if(    ! Open_Detector_Windows(  retErrorMesg  )   )
		return  false;

	if(    ! Open_ListEditor_Windows(  retErrorMesg  )   )
		return  false;





	if(   projectNeededBrowsing   )
		Set_DSTlist_Modified(  true  ); 		//    Load_ProjectSubject() will reset this value, so we save it for a restore


	/****

	if(   retUserCanceledProjectLoad   )
	{

		ASSERT( 0  );   // *******************  DO not think that it is possible to get to here anymore   5/5/07
		Reset_ProjectSubject(); 

		m_projectFilesPath   =   finalPath;  // Even if the user decided not to load the ProjectFile, we keep this value for aListOnly Save ( m_projectFilesPath is indide ObjList files

		m_dstListIsModified =   true;   // Only works some of the time,  CALLING function should REALLY set this.    Because the   PROJECT's Path   is stored in the   OBJECTlist file
	}
	****/

	return  true;
}



								/////////////////////////////////////////////////////


bool		DetectorApp::Query_User_For_SRCfile_Location(    ComponentExternal&  projectExternal,   CString&  retHeadersSRCfilePath,   												 
																					                          bool&    retSRCfilePathWasChanged,	   CString&  retErrorMesg   ) 
{

						//  If the user had to BROWSE and go to a new path,  the  'retHeadersSRCfilePath'  holds the new path value

			//  *** IMPORTANT:  if( retSRCfilePathWasChanged)   Calling functions must  set  m_projectIsModified = true at the APPROIATE PLACE   5/07 ********************************


	retErrorMesg.Empty();
	retSRCfilePathWasChanged   =  false;



	CString   retErrorMesgLoc,  retLocalMesg,   origFileName,  finalPath =  retHeadersSRCfilePath;
	CString   mesg,   retFilesNewPath;



	if(    Is_File_Present(  retHeadersSRCfilePath,   retErrorMesgLoc  )     )
		return  true;



	retSRCfilePathWasChanged  =    true; 
	


	Get_Paths_Target_FileName_GLB(  retHeadersSRCfilePath,   origFileName );

	mesg.Format(   "%s \nIf you can not find  %s  in another folder, the Project File can NOT be loaded. \nA file dialog box will next appear so that you can BROWSE for %s in another folder." ,    
																		retErrorMesgLoc,   origFileName,   origFileName  );
	AfxMessageBox(  mesg  );		
			



	bool    retUserCanceledBrowsing;  

	if(    ! Browse_For_Missing_File(   retHeadersSRCfilePath,   retUserCanceledBrowsing,     retFilesNewPath,     retErrorMesg  )     )  
	{
		ASSERT( 0 );  //  this is a REAL error
		Reset_ProjectSubject(); 
		return  false;
	}
				


	if(   retUserCanceledBrowsing   )
	{	

		Reset_ProjectSubject(); 
		retErrorMesg.Format(  "User canceled the load of %s" ,   origFileName  ); 

		return  false;
	}

		
	retHeadersSRCfilePath =    retFilesNewPath;    //  this is how we return the NEW path to the calling funct 		
	



/******  This is very bad.  If the ListFile has a bad Project address, and the user cances on loading the project,  this
			version check wilol fail and leave the user with out 	

	CString   retScrWAVfilesPath;     //  now that we have finally found the file,  we can check its version 

	if(    ! Peek_At_ProjectFiles_SRC_File_Path(    projectExternal,   projectFilesPath,   retScrWAVfilesPath,    retFilesVersion,     retErrorMesg   )  )
		return  false;
*****/	


	return  true;
}




									/////////////////////////////////////////////////////


bool    DetectorApp::On_New_Project(   ComponentExternal&  componentExternal,    CString&  retErrorMesg   ) 
{

	ASSERT( 0 );
	retErrorMesg  =   "DetectorApp::On_New_Project  is NOT yet implemented."  ;     //   4/07
	return  false;
}



									/////////////////////////////////////////////////////


bool	 DetectorApp::Init_New_Project(   short  projectCode,    CString&  retErrorMesg   )
{

			//  subclasses can overide,  but this is good enough for RegionDetectorApp   5/07


	retErrorMesg.Empty();



	Release_and_Close_SRCfile();

	m_projectFilesPath.Empty();

	m_lastProjectFileLoaded.Empty();   // ****  OK ????????????  Seems good  10/05/06  ******



	m_projectIsModified =    false;		//  since they are both now empty, there is noting to save
	Set_DSTlist_Modified(  false  );



	Hide_and_Release_All_Selections();




	if(    ! New_DSTlist(  retErrorMesg  )      )		//  *****  NEW,  OK ???? *******
		return  false;




//	if(     ! Close_All_Windows(  retErrorMesg  )     )    **** BAD, keep existing window up,  and just redraw
//		return  false;



	return  true;		
}


											////////////////////////////////////////


bool		DetectorApp::On_Save_Current_ProjectFile(    ComponentExternal&  componentExternal,   CString&  retMesg  )    
{

									//  HIGH Level,   calls  Save_ProjectSubject()



		//  Project file is like 'WorkSpace' to MS C++.  Once the Workspace has been created, the user can NOT
		//  save it in a NEW location( like a NoteList or .cpp file ).  

	retMesg.Empty();

	CString  projectFilesPath  =   Get_Project_Files_Path();



	Begin_Wait_Cursor_GLB();


	if(    ! Save_ProjectSubject(   componentExternal,    projectFilesPath,   retMesg   )     )    
	{
		End_Wait_Cursor_GLB(); 
		return  false;
	}
	else
		retMesg.Format(   "Project File was SAVED to  %s" ,   projectFilesPath  );

	End_Wait_Cursor_GLB(); 


	return true;	
}
					

											////////////////////////////////////////


bool		DetectorApp::On_Load_ProjectFile(   ComponentExternal&  componentExternal,    CString&  retErrorMesg  )
{

									//  HIGH Level,    it calls  Load_ProjectSubject()


	bool        retUserCanceled =  false,    retUserBailsOut= false;      //  issues are the Project extension and new definition for   Open_Detector_Windows()
	CString    projectFilesPath;


	retErrorMesg.Empty();


	ComponentExternal   *projectExternal =       componentExternal.Get_Enclosed_ComponentExternal();
	if(  projectExternal  ==  NULL   )
	{
		retErrorMesg =   "DetectorApp::On_Load_ProjectFile  failed,  projectExternal is NULL." ;
		return false;
	}

	Hide_and_Release_All_Selections();





	CString   retObjectName,   retExtension,    strMesg;

	componentExternal.Get_ListObjects_Name(       retObjectName  );
	projectExternal->Get_Files_Extension(    retExtension    );

	strMesg.Format(    "%s List Files (*.%s)|*.%s||" ,    retObjectName,   retExtension,   retExtension   );




	CFileDialog    dlg(   TRUE,
							   _T( retExtension ),   // **** MODIFY for other  Project  SUFFIXES  **********
							   NULL, 
								  OFN_HIDEREADONLY   |    OFN_OVERWRITEPROMPT,
								  _T(    strMesg )     );   // **** MODIFY for other  Project  FILE types *****

	LONG  nResult    =      dlg.DoModal();
	if(       nResult  !=  IDOK  )
		return  true;	  //  no error	

	projectFilesPath  =    dlg.GetPathName();




	if(    ! Query_User_For_ObjectList_Save(  componentExternal,  retUserCanceled,   retUserBailsOut,    retErrorMesg   )   )   //  always after the user has made a pick from the dialog
		return  false; 

	if(   retUserBailsOut   )
		return  true;    //  user decides that they do not want to do the  'CALLING command'    7/07





	if(    ! Query_User_For_Project_Save(  componentExternal,   retUserBailsOut,    retErrorMesg  )     )
		return  false;    //  or do I want just a AfxMessageBox()   ?????

	if(   retUserBailsOut   )
		return  true;    //  user decides that they do not want to do the  'CALLING command'    7/07



	Begin_Wait_Cursor_GLB();  

	if(    ! Load_ProjectSubject(   componentExternal,    projectFilesPath,  retUserCanceled,   retErrorMesg  )     ) 
	{
		End_Wait_Cursor_GLB(); //  WANT to close all woindows,  this is segious  
		return  false;
	}

	End_Wait_Cursor_GLB();



//	if(   ! retUserCanceled  )   *****  BAD,  user might have browsed.  
//		Get_DetectorApp().m_sourceFilesPath  =    retScrWAVfilesPath;     


	if(   ! Open_Detector_Windows( retErrorMesg  )     )
		return  false;

	if(    ! Open_ListEditor_Windows(  retErrorMesg  )   )
		return  false;


	Update_FirstSampleIdx_All_Viewjs(  0  );

	Draw_All_Viewjs_Titles();


	return  true;
}


									/////////////////////////////////////////////////////


bool	  DetectorApp::Load_ProjectSubject(   ComponentExternal&  componentExternal,   CString&  projectFilesPath,    bool&  retUserCanceled,      CString&  retErrorMesg   )
{

												//    LOW level,   calls  ComponentExternal->Receive()


			//  OK if  projectFilesPath IsEmpty,  a dialog will be launched.


	retErrorMesg.Empty();

	retUserCanceled =   false;


	ComponentExternal  *projectExternal =   componentExternal.Get_Enclosed_ComponentExternal(); 
	if(  projectExternal  ==  NULL  )
	{
		retErrorMesg =   "DetectorApp::Load_ProjectSubject  failed,  projectExternal is NULL" ;
		return  false;
	}

																		//   erase any selections																		
	Hide_and_Release_All_Selections();







	CString    retScrWAVfilesPath,   origSRCFileName,  progFileName; 
	short       retFilesVersion = 0;

	if(    ! Peek_At_ProjectFiles_SRC_File_Path(    *projectExternal ,    projectFilesPath,   retScrWAVfilesPath,     retFilesVersion,     retErrorMesg   )  )
		return  false;
	
	Get_Paths_Target_FileName_GLB(  projectFilesPath,        progFileName );



	if(      Get_EXEs_Version_Number()   <   retFilesVersion  )    // retFilesVersion    )
	{
		CString   mesg;					
		mesg.Format(   "The Project File( %s ) that you are trying to load is of a LATER VERSION[ %.1f ] that that of the Application[  %.1f  ]. It may or may not load properly.\n Do you want to try and load it anyway?",
												                                          progFileName,     ((double)retFilesVersion)/10.0,  	((double)Get_EXEs_Version_Number() )/10.0   );		
		int  reslt    =      AfxMessageBox(  mesg,   MB_YESNO | MB_ICONQUESTION   );
		if(   reslt  !=   IDYES   )
		{
			retUserCanceled =  true;
			return  true;   
		}
	}




	if(    ! Init_New_Project(   Get_Project_Code(),   retErrorMesg  )     )     //   Reset_ProjectSubject();    ****************  BETTER??? *****************   TEST
		return  false;    





//    /*******  OK to move down ????  NO,  creates trouble for VoxSep  

	bool        retSRCfilePathWasChanged =   false;  

	if(   ! Find_and_Init_Bitsource_SRC_File(   componentExternal,   retScrWAVfilesPath,   retSRCfilePathWasChanged,   retErrorMesg  )   )
		return  false;
//        ************************/




																			///////////////////////////////

	if(    ! New_DSTlist(  retErrorMesg  )      )
		return  false;
	

	ComponentSubject   *retObjectPtr =  NULL;  

	try
    {  CFile   file(     projectFilesPath, 
	                        CFile::modeRead      //   |   CFile::shareExclusive 								
				 //	   |  CFile::typeBinary   ...BUGGY ????    5/02    Sets binary mode (used in derived classes only).
					  );

		if(    ! projectExternal->Receive(   file,   &retObjectPtr,   retErrorMesg  )     )  
			return  false;
	

		m_projectFilesPath  =    projectFilesPath;      //  save for later ReLoad of logDFT SEGMENTs  

	}
	catch(   CFileException   *pException   )
	{
		TCHAR    szCause[ 255 ];  
		CString   strRaw;
        pException->GetErrorMessage(  szCause,  255  );
		pException->Delete();
		strRaw =  szCause;
		retErrorMesg.Format(   "DetectorApp::Load_ProjectSubject:  could not load Project file because %s." ,   strRaw  );
		return  false;
	}




/*******************  Bad for PitchScopoe if down here  8/08   ****************************
	bool        retSRCfilePathWasChanged =   false;  

	if(   ! Find_and_Init_Bitsource_SRC_File(   componentExternal,   retScrWAVfilesPath,   retSRCfilePathWasChanged,   retErrorMesg  )   )
		return  false;
********/




	if(    ! Do_Sources_PostLoad_Calcs(   retErrorMesg   )     )   //  Build the Amplitude array...  etc.
		return  false;



	Record_Loaded_Projects_Name(   m_projectFilesPath   ); 



	if(    retSRCfilePathWasChanged	)			//  Did the user have to BROWSE to a different location for the SRC-wav file?
	{
	    m_projectIsModified =   true;  
		//  m_dstListIsModified =   false;     Not probably necessary because of  New_DSTlist()
	}
	else
	{  m_projectIsModified =   false;  
		Set_DSTlist_Modified(  false  );
	}


	return  true;
}




											////////////////////////////////////////


bool   DetectorApp::On_View_SRC_File(  ComponentExternal&  compProjectExternal,   CString&  retErrorMesg   ) 
{																	

	bool       retUserCanceled,   retUserBailsOut=  false;				//   called from MENU,   		Just for VIEWING,  no ObjList or Project		
	CString  retFilesPath;





	if(    ! Query_User_For_ObjectList_Save(   compProjectExternal,   retUserCanceled,  retUserBailsOut,   retErrorMesg   )   )
		return  false;   

	if(   retUserBailsOut   )
		return  true;    //  user decides that they do not want to do the  'CALLING command'    7/07



		
	if(    ! Query_User_For_Project_Save(  compProjectExternal,   retUserBailsOut,   retErrorMesg  )     )
		return  false;  

	if(   retUserBailsOut   )
		return  true;    //  user decides that they do not want to do the  'CALLING command'    7/07





	if(     !Close_All_Windows(  retErrorMesg  )     )
		return  false;  



						
	if(    ! Init_New_Project(   Editorj::PITCHEDITOR,   retErrorMesg   )     )
		return  false; 




	Begin_Wait_Cursor_GLB();   


	if(     ! Choose_SRC_File_for_Viewing(   retErrorMesg  )     )
	{
		End_Wait_Cursor_GLB();  
		return  false;
	}

	End_Wait_Cursor_GLB();   
		



	retFilesPath   =       Get_Source_Files_Path();
	if(   retFilesPath.IsEmpty()    )    
		return  true;     //  OK...     will be empty if user cancel,  and so no message is deserved.


	if(    ! Do_Sources_PostLoad_Calcs(  retErrorMesg   )     )    //  only opens one if it is not there   6/07
		return  false; 





	if(    ! Open_ListEditor_Windows(  retErrorMesg   )     )    //  only opens one if it is not there   6/07
		return  false; 


	Update_FirstSampleIdx_All_Viewjs(  0  );

	Draw_All_Viewjs_Titles();


	ReDraw_All_Viewjs();  


	return  true;
}



									/////////////////////////////////////////////////////


bool	 DetectorApp::Find_and_Init_Bitsource_SRC_File(   ComponentExternal&  componentExternal,   CString&  sourceFilesPath, 
																			bool&  retSRCfilePathWasChanged,     CString&  retErrorMesg  )
{


	CString   retLocalMesg,   origSRCFileName,   retHeadersSRCfilePath  =    sourceFilesPath,     finalPath = sourceFilesPath;
    short       retFilesVersionDummy = 0;


	retErrorMesg.Empty();


	ComponentExternal  *projectExternal =   componentExternal.Get_Enclosed_ComponentExternal(); 
	if(  projectExternal  ==  NULL  )
	{
		retErrorMesg =   "DetectorApp::Find_and_Init_Bitsource_SRC_File  failed,  projectExternal is NULL" ;
		return  false;
	}



	Get_Paths_Target_FileName_GLB(  sourceFilesPath,   origSRCFileName );



	if(   ! Query_User_For_SRCfile_Location(   *projectExternal,    retHeadersSRCfilePath,    retSRCfilePathWasChanged,   retLocalMesg  )    )
	{  Reset_ProjectSubject(); 

		retErrorMesg.Format(  "The Project File could NOT be loaded because its SOURCE file( %s ) could not be loaded[ %s ]."  ,   origSRCFileName,   retLocalMesg   );
		return  false;
	}

	finalPath  =    retHeadersSRCfilePath;   //  might be changed by a browse





	bool  retFileAcessError =  false;


	BitSource  *bitSource  =   Initialize_BitSource(    finalPath,   retFileAcessError,    retLocalMesg  ) ;


	if(   bitSource ==  NULL   ) 
	{

		if(   retFileAcessError   )
			retErrorMesg.Format(  "%s could not be loaded because:  %s" ,  origSRCFileName,   retLocalMesg  );
		else
			retErrorMesg  =  retLocalMesg;

		Reset_ProjectSubject(); 
		Release_and_Close_SRCfile();  

		return  false;
	}



	m_sourceFilesPath	   =   finalPath;    // Not the same as the header's value,  the user may have had to BROWSE

	return  true;
}




									/////////////////////////////////////////////////////


bool	 DetectorApp::Save_ProjectSubject(   ComponentExternal&  componentExternal,    CString&  filePath,   CString&  retErrorMesg  )
{

												//    LOW level,   calls  ComponentExternal->Emit()


			//   IF want the Dialog to go up,   INPUT  'filePath'  as Empty.  Otherwise  filePath is used.


	if(    ! Is_Projectsubject_Loaded()     )
	{
		retErrorMesg =  "First load or create a project."  ;
		return  false;
	}



	ComponentExternal   *projectExternal  =      componentExternal.Get_Enclosed_ComponentExternal();
	if(   projectExternal ==  NULL   )
	{
		retErrorMesg =  "DetectorApp::Save_ProjectSubject failed,  projectExternal is NULL"  ;
		return  false;
	}
	


	CString   currentProjName;

	if(   m_lastProjectFileLoaded.IsEmpty()   )
	{
		ASSERT( 0 );
	}
	else
		currentProjName =  m_lastProjectFileLoaded;



	CString  retObjectName,  retProjectExtension,  strMesg;  

	componentExternal.Get_ListObjects_Name(  retObjectName  );
	projectExternal->Get_Files_Extension(  retProjectExtension  );

	strMesg.Format(    "%s Project Files (*.%s)|*.%s||" ,    retObjectName,   retProjectExtension,   retProjectExtension   );





	CString   filesPathReal;


	if(    filePath.IsEmpty()   )
	{

		CFileDialog    dlg(   FALSE,
								   _T( retProjectExtension ), 
								   currentProjName,    // NULL, 
								   OFN_HIDEREADONLY   |    OFN_OVERWRITEPROMPT,
								   _T(  strMesg )      );

		LONG  nResult    =      dlg.DoModal();
		if(       nResult  !=   IDOK  )
		{
			retErrorMesg =   "Project was not saved because of cancel."  ;  
			return  false;			//  Do not want CALLING function to give a confirmation message
		}

		filesPathReal =    dlg.GetPathName();
	}
	else
		filesPathReal =    filePath;  




																	//   |   CFile::shareExclusive 		???EXPERIMENT ??  
	try
    {  CFile   file(    filesPathReal, 
						//	  CFile::typeBinary     ...BUGGY ????    5/02    Sets binary mode (used in derived classes only).

	                    //   CFile::modeCreate	 |  CFile::modeWrite      
						     CFile::modeCreate    |  CFile::modeWrite     |   CFile::modeNoTruncate
					  );					 // ***NOTE:   'modeNoTruncate'   allows APPEND to existing fie,  does not EMPTY/ERASE an EXISTING file



		if(    ! projectExternal->Emit(   file,    NULL,    retErrorMesg  )     )
		{
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
		retErrorMesg.Format(   "Save_ProjectSubject  failed,  could not save Project File because:  %s." ,   strRaw  );
		return  false;
	}
	


	m_projectIsModified  =    false;    //   false:    because  there exist  'NO UnSaved Changes'


	return  true;
}



											////////////////////////////////////////


bool	   DetectorApp::Query_User_For_Additional_File_Save(   ComponentExternal&  compExternal,    CString&  headersProjectPath,   bool& retUserBailsOut,   CString&  retErrorMesg   )
{

						 //  this is called by UniEditorApp::On_Load_ObjectList_File(),  so we need to know if we need a new Project  

				//  This has 2 purposes:
				//							a)  Give the user a chance to save the EXISTING Project
				//							b)  Give the user a chance to  { load or cancel }   the NEW Project  of the ObjList that is corrently loading


									

	bool   retProjectPathWasChanged =   false;

	bool   retUsesSameProject =   false;  
	bool   retUserCanceledProjectLoad =  false;





	if(    ! Uses_Same_Project_That_Is_Loaded(   headersProjectPath,    retUsesSameProject,    retErrorMesg   )    )
		return  false;   // this is an error,  not a result



	if(   retUsesSameProject  )
		return  true;    // 



	if(  ! Query_User_For_Project_Save(   compExternal,   retUserBailsOut,   retErrorMesg   )   )
		return  false; 

	if(   retUserBailsOut   )
		return  true;    //  user decides that they do not want to do the  'CALLING command'    7/07






									//   MOVE this to calling function ????  No, the calling function is a UniEditorApp,  and this is a Project function (  DetectorApp:: )

	if(    ! Query_User_For_Project_Load_Before_ObjList_Load(   compExternal,   headersProjectPath,    retUserCanceledProjectLoad,   retErrorMesg  )     ) 
		return  false;   //  false is a real error.  But the user might have opted to cancel the load of the Project file, which is fine   


	return  true;
}



											////////////////////////////////////////


ComponentView*    DetectorApp::Get_An_Open_PROJECTpane(   Viewj  **retViewPtr,    CString&   retErrorMesg   ) 
{

								//  a Window that shows SOURCE DATA( not a detected result like the OutputList pane )
								//                             (  ex:  SOURCEdETECTORsPITCH,   GRAFIXsRC   )


	ComponentView   *retPaneDST =  NULL;   


	retErrorMesg.Empty();

	if(     retViewPtr   !=  NULL    )    //  Init,.   TRICKY,  user might want to 
		*retViewPtr =   NULL;




	Viewj   *thisView       =       Get_Active_ListSubject_Viewj();   //   Get_Viewj();    //  a  UniWindow::Get_Viewj()   *** WHAT about Revolver ???  ...SPitchListWindow is descendant of this.    
	if(         thisView   !=   NULL
		&&    thisView->Has_ListChildren()  
		&&    thisView->m_isAprojectWindow   )   //   **********************  
	{
		retPaneDST =   thisView->Get_DSTlist_ComponentView();
		if(   retPaneDST  !=  NULL  )
		{
			if(     retViewPtr   !=  NULL    )
				*retViewPtr =   thisView;

			return  retPaneDST;
		}
	}
		
	
	Viewj   *activeView        =      Get_Active_Viewj();   
	if(         activeView   !=   NULL
		&&    activeView->Has_ListChildren()  
		&&    thisView->m_isAprojectWindow    )   //   **********************  

	{
		retPaneDST =   activeView->Get_DSTlist_ComponentView();
		if(   retPaneDST  !=  NULL   )
		{
			if(     retViewPtr   !=  NULL    )
				*retViewPtr =   activeView;

			return  retPaneDST;
		}
	}



	ListIterator<Viewj>    iter(     Get_ViewjList()    );      


    for(   iter.First();   !iter.Is_Done();   iter.Next()   )
    {  		
		Viewj&  vw =    iter.Current_Item();   

		if(    vw.Has_ListChildren()    
			 &&  vw.m_isAprojectWindow   )     //   **********************  
		{
			retPaneDST =   vw.Get_DSTlist_ComponentView();

			if(   retPaneDST  !=  NULL   )
			{
				if(     retViewPtr   !=   NULL    )
					*retViewPtr =   &vw;

				return  retPaneDST;
			}
		}
	}


	retErrorMesg =  "Open a window with an object list."  ;
	return  NULL;
}



											////////////////////////////////////////



Viewj*	  DetectorApp::Get_An_Open_Animation_Viewj(   short  viewjType,    CString&   retErrorMesg   )
{

	retErrorMesg.Empty();


	/***
	Viewj   *thisView       =       Get_Active_ListSubject_Viewj();   //   Get_Viewj();    //  a  UniWindow::Get_Viewj()   *** WHAT about Revolver ???  ...SPitchListWindow is descendant of this.    
	if(         thisView   !=   NULL
		&&    thisView->Has_ListChildren()  
		&&    thisView->m_isAprojectWindow   )   //   **********************  
	{
		retPaneDST =   thisView->Get_DSTlist_ComponentView();
		if(   retPaneDST  !=  NULL  )
		{
			if(     retViewPtr   !=  NULL    )
				*retViewPtr =   thisView;

			return  retPaneDST;
		}
	}
		
	
	Viewj   *activeView        =      Get_Active_Viewj();   
	if(         activeView   !=   NULL
		&&    activeView->Has_ListChildren()  
		&&    thisView->m_isAprojectWindow    )   //   **********************  

	{
		retPaneDST =   activeView->Get_DSTlist_ComponentView();
		if(   retPaneDST  !=  NULL   )
		{
			if(     retViewPtr   !=  NULL    )
				*retViewPtr =   activeView;

			return  retPaneDST;
		}
	}
	***/

/****
		short      viewjType =     m_viewj->m_viewjType;
	switch(   viewjType   )
	{

		case    Editorj::REVOLVERaNIME :

			if(     !Add_Pane_Revolver(   widthRevolver,     retErrorMesg  )     )
				return   false;
		break;
****/

	ListIterator<Viewj>    iter(     Get_ViewjList()    );      


    for(   iter.First();   !iter.Is_Done();   iter.Next()   )
    {  		
		Viewj&  vw =    iter.Current_Item();   


		if(  	    vw.Is_A_Anime_Viewj()   
			&&    vw.m_viewjType  ==  viewjType    )
		{
			return  &vw;			
		}
	}


	retErrorMesg =  "Open an Animation window."  ;
	return  NULL;
}



											////////////////////////////////////////


bool    DetectorApp::Uses_Same_Project_That_Is_Loaded(   CString&  newProjectsPath,  bool&  retUsesSameProject, 
														                                      CString&  retErrorMesg   ) 
{
			// Only returns false if an error.

			//  Really means, does this new, loading ListFile need to also load a different Project ( so if there is 
			//  NO project loaded, then it also needs to laod a project    4/07

	retUsesSameProject =  false;			


	if(    Get_Project_Files_Path().IsEmpty()     )
	{
		retUsesSameProject  =   false;		
		return  true;
	}


	if(   ! Is_Projectsubject_Loaded()    )
	{
		retUsesSameProject  =   false;		
		return  true;
	}




	bool   retUsingSameProjectLocal;   




	if(   newProjectsPath.IsEmpty()    )    //  Loading a Notelist from Navigator that has NO PROJECT ( temp, allow the same Project to persist   3/11
	{


	// **** FIX:   Should check that the same sourceWAV file is also used 3/11  *******************************




	/**************  INSTALL this test    DO I also want to test the different  WAV-Target  FileNames  ???	3/11*****************

				Also want to compair the SRC (WAV( file nakmes as well ???  ) .  Not necessary, but could be an extra safeguard.  3/11


		if(    ! pitchDetApp.Are_FileTargets_The_Same(   currentProjectsWAVfilePath,  strDstWAVfilePath,   usingSameProjectWAVs,   retErrorMesg  )    )
		{
			AfxMessageBox(  retErrorMesg  );			
			ASSERT( 0 );  //  What should I do in this case ???
			usingSameProject =   false;
		}


...Compair    pitchDetApp.m_sourceFilesPath     and   write a function similar to this(below)  to get the target

	  virtual     bool				  Peek_At_ListFiles_ProjectPath(   ComponentExternal&  compProjectExternal,    CString&  listFilesPath,    CString&  retProjectFilesPath,   
																					         short&  retFilesVersion,     CString&  retErrorMesg   );   

****/


		retUsesSameProject  =   true;		
		return  true;
	}





	if(    ! Are_FileTargets_The_Same(    newProjectsPath,    Get_Project_Files_Path(),   
									                                       retUsingSameProjectLocal,    retErrorMesg  )    )
	{			
		ASSERT( 0 );  //  What should I do in this case ???

		retUsesSameProject  =   false;		

		return  false;
	}



	
	if(    retUsingSameProjectLocal    )
		retUsesSameProject  =   true;	
	else
		retUsesSameProject  =   false;	
		

	return  true;
}



											////////////////////////////////////////


bool		 DetectorApp::Open_Detector_Windows(   CString&  retErrorMesg  )
{

	Viewj  *retViewPtr;							//   If no windows are open,  open one


	ComponentView  *cmpView =     Get_An_Open_PROJECTpane(   &retViewPtr,   retErrorMesg   ); 
	if(  cmpView ==  NULL   )
	{			
		
		/***  INSTALL    If I knew the user was doing a ProjectOnly load, it might make sense to close any ListEditor windows.


		if(    ! Close_All_Windows(  retErrorMesg  )     )
		{
			//  AfxMessageBox(  retErrorMesg  );
	//		return;    ...Let it run, might not be that big a deal.    
			return  false;
		}	
		****/

		/***
		OnNuAnimeRevolver();   //  do this first, so that EditWindow will be the active one
		Open_Edit_Window(   Editorj::SOURCEdETECTORsPITCH   );    //  Want to show bitmapBackround and 12 channels    UniBasic::    SINGLEpANEsPITCH  );  
		***/
		Open_DetectorWindows_GLB(); 
	}




	if(     !ReCreate_Child_ComponentViews_AllViewjs(  retErrorMesg  )     )
		return  false;
	
	Notify_All_ComponentViews_forReDraw();
	ReDraw_All_Viewjs();


	return  true;
}



											////////////////////////////////////////


bool		DetectorApp::Open_Animtion_Window(    short  viewjType,     CString&  retErrorMesg  )
{

					//   If no windows are open,  open one


	//  ComponentView  *cmpView =     Get_An_Open_PROJECTpane(   &retViewPtr,   retErrorMesg   ); 
	Viewj   *retViewPtr  =	   Get_An_Open_Animation_Viewj(   viewjType,   retErrorMesg   );
	if(  retViewPtr ==  NULL   )
	{			
		
		/***  INSTALL    If I knew the user was doing a ProjectOnly load, it might make sense to close any ListEditor windows.


		if(    ! Close_All_Windows(  retErrorMesg  )     )
		{
			//  AfxMessageBox(  retErrorMesg  );
	//		return;    ...Let it run, might not be that big a deal.    
			return  false;
		}	
		****/

		/***
		OnNuAnimeRevolver();   //  do this first, so that EditWindow will be the active one
		Open_Edit_Window(   Editorj::SOURCEdETECTORsPITCH   );    //  Want to show bitmapBackround and 12 channels    UniBasic::    SINGLEpANEsPITCH  );  
		***/


		if(    viewjType  ==   Editorj::REVOLVERaNIME   )
				Open_Revolver_Window_GLB(); 
	//	else if(   viewjType  ==   Editor::THREEdimSCALEpITCHaNIME   )
	//	{   }
		else
		{	retErrorMesg =  "DetectorApp::Open_Animtion_Window FAILED,  unknown  viewjType." ;
		}
	}



	/****
	if(     !ReCreate_Child_ComponentViews_AllViewjs(  retErrorMesg  )     )
		return  false;
	
	Notify_All_ComponentViews_forReDraw();
	ReDraw_All_Viewjs();
	***/


	return  true;
}











