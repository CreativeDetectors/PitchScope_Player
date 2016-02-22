//   UniEditorAppsGlobals.h
//
//  Copyright (C) 2007   Creative Detectors (of San Francisco, California).  All Rights Reserved.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#define  USEnEWaPPcLASSES      //  ...the switch  TO use  UniEditorApp(  instead of UniBasic )



void  Begin_Wait_Cursor_GLB();
void  End_Wait_Cursor_GLB();


void  Open_DetectorWindows_GLB();         
void  Open_EditorWindows_GLB();     
void  Open_Revolver_Window_GLB();      

void  Open_AllWindows_GLB();     


void  ReDraw_DrivingView_OnDialog_GLB(   bool  renderWithoutText  );   // calls  Render_DrivingViews_Pane_wFinalCircQue() ,  but really just for Navigator 


void  Write_To_StatusBar_GLB(   CString&   mesg   );     //   NEW,  3/2012




#define	 kWithNOkey       0           //  key qualifiers ( can be bit masked )

//#define	 kWithOPTIONkey   2
#define	 kWithALTkey   2

#define	 kWithSHIFTket    4
#define	 kWithCONTROLkey  8   
