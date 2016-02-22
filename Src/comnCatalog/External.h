/////////////////////////////////////////////////////////////////////////////
//
//  External.h   -   
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





////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////


class  External
{

public:
	External(void);
	virtual ~External(void);


	virtual	  void		Get_Files_Extension(      CString&   retExtension  ) = 0;     //      {   retExtension =  "pnl" ;  }

	virtual     void        Get_ListObjects_Name(  CString&  objectName   )  { ASSERT( 0 );     // *****  DUMMY default    12/2011  
																											objectName =   "Object"  ;  
																										  }




																	//   STATIC  Utility  functions 


	static      bool			Does_FileName_Have_Extension(   char* fileExtensionPtr,    CString&   filePath   );

	static      bool		    Get_Files_Name_from_Path(    CString&   filesPath,    CString&   retFilesName  );


	static      bool			Is_File_Present(   CString&  filesPath,    CString&  retErrorMesg   );   //  this funct is from  DetectorApp  and  UniEditorApp






															//  These 2 can also be used to FIND and write to the  'FILE  HEADER',   as well as other chunks    12/11

	static      bool			Goto_Chunks_Header(    CFile&  file,    short  chunkCode,   unsigned char&  retVersion,   long&  retChunksSize,   CString&  retErrorMesg  );

	static      bool			Write_Chunks_Header(   CFile&  file,    short  chunkCode,   unsigned char  version,        long&  retSizeFilePos,    CString&  retErrorMesg  );



	static      bool			Get_Chunks_TagString(   short  chunkCode,   CString&  retTagString,   CString&  retErrorMesg  );  // Lists ALL String-Codes for ALL Millardo Apps

	static      void			Write_Chunks_Length(    CFile&  file,   long  tagsLengthFilePos   );





	static      void			Write_FileCreator_Tag(    CFile&  file,    unsigned char  version   );	

	static      bool			Verify_FileCreator_Tag(   CFile&  file,    unsigned char&  retVersion,   CString&  retErrorMesg  );



	static      void			Write_Lists_Count(   long  numberOfListObjects,   CFile&  file  );

	static      long			Read_Lists_Count(   CFile&  file  );






public:

// ********   NEVER change the  ORDER or NAMING  of the below Enumeration ( need for backward  FILE compatibility  )  12/2011  *************
//
//     ...and keep in SYNC  with   CalcedNoteExternal.h
// **********************************************************************************************************************

	enum  chunkCodes                                 // ***BIG:   ALL new entries must be written in External::Get_Chunks_TagString()   4/07
		{  
			NOTELISTfILEhEADERcHK,   PITCHPROJECTfILEhEADERcHK,    SpITCHLISTcHK,       DZONELISTchkLEFT,   DZONELISTchkRIGHT,   DZONELISTchkCENTER,     
		    OVALLISTfILEhEADERcHK,  OVALLISTcHK,      REGIONLISTfILEhEADERcHK,   REGIONLISTcHK,    REGIONPROJECTfILEhEADERcHK,
					LICKlISTcHK			//  Place new values on this line   12/2011
	    };     
};
