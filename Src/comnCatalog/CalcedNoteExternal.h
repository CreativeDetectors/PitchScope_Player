/////////////////////////////////////////////////////////////////////////////
//
//  CalcedNoteExternal.h   -   
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_CALCEDnOTEeXTERNALS_H____INCLUDED_)
#define AFX_CALCEDnOTEeXTERNALS_H____INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


///////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////


class    CalcedNote;
class    MidiNote;

class    Lick;

class	   External;




////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////


class   CalcedNoteListExternal    :   public  External       // ( Install  NEW parent for both this and     ComponentExternal  of  gnUniBasic.h  )      
{

public:
	CalcedNoteListExternal();
	virtual ~CalcedNoteListExternal();



	  virtual	     void		  Get_Files_Extension(    CString&   retExtension   )          {   retExtension =  "pnl" ;  }

	  virtual      void       Get_ListObjects_Name(  CString&  objectName  )           {   objectName =   "Note"  ;  }


	
	virtual	     bool       Receive(           CFile&  file,    /*  ComponentSubject   **retObjectAddr,  */    CString&  retErrorMesg   );
	virtual	     bool       Emit_NoteList(    CFile&  file,      void*  extraData,     CString&  retErrorMesg   );



//	virtual	     bool       Emit_DetectZone(   CFile&  file,   void*  extraData,     CString&  retErrorMesg   );

													  



	  virtual      bool		Save_ObjectList_Chunk(    short  chunkCode,    unsigned char  version,    //  ComponentSubject&	unknownSubject,  
												                                CFile&  file,     short  specialCode,    short detectZoneCount,      CString&  retErrorMesg    ); 

	  virtual      bool		Load_ObjectList_Chunk(    short  chunkCode,    unsigned char  version,  
																		//    ComponentSubject&	unknownSubject,    ComponentExternal&   listObjectExternal,      
																				CFile&  file,    CString&  retErrorMesg    );   





	  virtual      void		Assign_FileObject_ScalePitch(   SPitchFileObj&   fileObj,      MidiNote&   midiNote,    bool   savingOldPitchScope2007File 
		                                                          //   long  startOffset,     long  endOffset,    short  scalePitch,    short fundamentalCandIdx,  short  scoreDetectionAvgTone,   short  avgHarmonicMag   
							                                       );

					void	    AssignMe_from_FileObject_ScalePitch(   SPitchFileObj&   fileObj,    MidiNote&  nuCalcNote,   bool  oldPitchScopeFile    );



	  virtual      void		Assign_FileObject_Lick(    LickFileObj&   fileObj,    long  startSample,  long  endSample,    CString&  nickName   );  			

					 void	    AssignMe_from_FileObject_Lick(   LickFileObj&   fileObj,    Lick&  nuLick  );




	  virtual      void		Assign_FileObject_DetectZone(   DetectZoneFileObj&   fileObj,    short  channelCode,    long   startOffset,    long   endOffset   );		  




//	  virtual      bool		Goto_Files_Header(  CFile&  file,      unsigned char&  retVersion,        // almost the same as Goto_Chunks_Header() but with know header constant  4/07
//																		                           long&  retChunksSize,   CString&  retErrorMesg  );





public:
	ListDoubLinkMemry< MidiNote >    *m_calcedNoteList;

	ListMemry< Lick >	     *m_lickList;




								//    Alway keep FILE versions 9 digits lower than the current EXE version[ 10 ],  to that I can release
								//    CORRECTED File verions upward without mandating a release of a new EXE( with a higher version )   1/06

//	short       m_exesVersionNumber;  //   '20'    (  2.0,    OLD PitchScope was  1.0 )


	short		  m_filesVersionNumber;  //     '12'     is for  Navigator 1.0  and   PitchScope 2.0 (Scriber)   ...A newFAMILY of new apps (Player.exe)    2/12
													 //			
													 //     ADDITIONS for  12:     LickList,   no real DetectZones(dummy )
													 //    
													 //     OLD PitchScope was   '2'

	short       m_loadedFilesVersionNumber;    //  New,  to help resolving  OLD PitchScope 2007 files    2/12




	long           m_totalSampleBytes;


	CString      m_sourceWAVpath;


	CString      m_projectFilesPath;   //   SHOULD I be saving this value from OLD PitchScope files ?
													//
													//    PROBLEM:  if Navigator saves an OLD PitchScope file, it will loose { DFTmaps, DetectZones,  ??  ) ...it get mangled
													//                     but could ultimately save the file in  12 Version for Navigator/Player        



	long     m_startOffsetDetectZone;
	long     m_endOffsetDetectZone;


	short   m_midiInstrumentNumber;


	short	  m_musicalKey;
};




////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

/*****
class   CalcedNoteExternal    // :   public  ComponentExternal      //   external   REPRESENTATION      ...write to FILE        4/07
{

public:
	CalcedNoteExternal();
	virtual ~CalcedNoteExternal();

	
//	virtual	     bool       Emit(        CFile&  file,      void*  extraData,     CString&  retErrorMesg   );
//	virtual	     bool       Receive(            CFile&  file,       ComponentSubject   **retObjectAddr,        CString&  retErrorMesg   );


	  virtual	     void		Get_Files_Extension(    CString&   retExtension   )    {  retExtension =       "pnl" ;     }

	  virtual      void     Get_ListObjects_Name(  CString&  objectName  )     {  objectName  =   "Note"  ;  }


	  virtual      void		Assign_FileObject(    SPitchFileObjPlayer&   fileObj  );


public:

		//  do NOT need to know owning view, cause THIS view list resides in a Viewj   ...but could HOLD in a memberVar( so Update does NOT need a parm )
//	ComponentView    *m_listComponentView;    //  {  ListComponentView,   DrivingBitmapView,    DrivingCylindersView,   etc.    }
};
****/




////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_CALCEDnOTEeXTERNALS_H____INCLUDED_)
