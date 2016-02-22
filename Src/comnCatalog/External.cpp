/////////////////////////////////////////////////////////////////////////////
//
//  External.cpp   -   
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

//  #include  "Mmsystem.h"     //  for MIDI


#include   "..\comnFacade\UniEditorAppsGlobals.h"

#include  "..\comnFacade\VoxAppsGlobals.h"



//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"    

#include  "..\ComnGrafix\CommonGrafix.h"    
#include  "..\ComnGrafix\OffMap.h"  
#include  "..\comnGrafix\TransformMap.h"

#include "..\comnInterface\DetectorAbstracts.h"    // **** NEW ****



/*****
///////////////////////////

#include  <mmsystem.h>   // Is good on Sparky  ( ProgramFiles\Microsoft Visual Studio\VC\PlatformSDK\Include )


#include  "..\ComnAudio\dsoundJM.h"        //  I copied it in, bigger than the VC++ version


// #include  "..\ComnAudio\WaveJP.h"

#include  "..\ComnAudio\EventMan.h"      //  My encapsulating class for Direct Sound


#include  "..\comnAudio\BitSourceAudio.h"
///////////////////////////
*****/




//#include  "..\ComnAudio\AnalyzerAudio.h"

//#include  "..\ComnFacade\SoundMan.h"   


//  #include   "..\ComnAudio\SPitchCalc.h"



//#include "..\ComnFacade\PitchDetectorApp.h"




#include "External.h"

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


External::External(void)
{


}

											////////////////////////////////////////

External::~External(void)
{


}



											////////////////////////////////////////

	
void    External::Write_FileCreator_Tag(   CFile&  file,    unsigned char  version  )     
{

					//  Always gets written first   (  move to  higher parent level,    ComponentExternal  ????   ) 


	char  *prjString =    "MEALDRED"  ;     //   to identify as one of MY files  


	file.Write(   prjString,     8   );

	file.Write(   &version,     1  );    	//	 Navigator:  12  ( exe 2.0)         OLD  PitchScope   2    ( exe 1.0 )          8/2012
}



											////////////////////////////////////////

	
bool    External::Verify_FileCreator_Tag(   CFile&  file,    unsigned char&  retVersion,   CString&  retErrorMesg  )    
{

	retErrorMesg.Empty();
	retVersion =  0;


	CString   prjString =    "MEALDRED"  ;     //   to identify as one of the files from my COMPANY 
	CString   filesStr  =  "        ";   // This fixes the old bug by insereting a TERMINATOR to the string


	file.Read(   filesStr.GetBuffer( 8 ),    8  );



	if(    prjString.CompareNoCase(  filesStr  )   !=   0   )
	{
		retErrorMesg =   "This is not a Creative Detectors file format. Load a file of the correct type."   ;
		return  false;
	}


	file.Read(   &retVersion,    1  );     //   File-VERSION  Is   '12'    for Navigator,     OLD PitchScope files were   '10'         8/2012


	return  true;
}



											////////////////////////////////////////


bool    External::Get_Chunks_TagString(   short  chunkCode,   CString&  retTagString,   CString&  retErrorMesg    )
{

							//  *********** These MUST  be   8 CHARS in  length !!!!    4/07  	 **************		

							//  GET all these strings in same place for all Millardo Apps.  Think about new External

	retTagString.Empty();



	/****
	if(         chunkCode  ==   NOTELISTfILEhEADERcHKPlayer    )	        //  the 'HEADER' Chunk  of the  FILE
		retTagString  =       "NtLstFil"  ; 
	else if(   chunkCode  ==   SpITCHLISTcHKPlayer    )	     //  the  'ScalepitchList  ITSELF'   chunk in the file 
		retTagString  =       "SpICHLST" ; 	

	else if(   chunkCode  ==   DZONELISTchkLEFTplayer    )	     
		retTagString  =       "DZonLeft" ; 	
	else if(   chunkCode  ==   DZONELISTchkRIGHTplayer    )	     
		retTagString  =       "DZonRght" ; 	
	else if(   chunkCode  ==   DZONELISTchkCENTERplayer    )	    
		retTagString  =       "DZonCent" ; 	
	*****/


	switch(   chunkCode  )
	{

		case   External::PITCHPROJECTfILEhEADERcHK :	//  the  'HEADER' for  a   PitchScope  PROJECT   file
			retTagString  =    "PtPrgFil" ; 
		break;

		case   External::NOTELISTfILEhEADERcHK :	        //  the  'HEADER' (a type of chunk)  for the NoteLIST   File
			retTagString  =      "NtLstFil"  ; 
		break;

		case   External::SpITCHLISTcHK :	         //  the  'ScalepitchList  ITSELF'   chunk in the file 
			retTagString  =    "SpICHLST" ; 			
		break;


		case   External::DZONELISTchkLEFT :	
			retTagString  =    "DZonLeft" ; 
		break;

		case   External::DZONELISTchkRIGHT :	
			retTagString  =    "DZonRght" ; 
		break;

		case   External::DZONELISTchkCENTER :	
			retTagString  =    "DZonCent" ; 
		break;



		case   External::OVALLISTfILEhEADERcHK :	   //   the 'HEADER' Chunk  of the  FILE
			retTagString  =    "OvLstFil"  ; 			
		break;

		case   External::OVALLISTcHK :	              //  the  'OvalList  ITSELF'   Chunk   in the file 
			retTagString  =    "OvalLIST" ; 			
		break;




		case   External::REGIONLISTfILEhEADERcHK :	   //   the 'HEADER' Chunk  of the  FILE
			retTagString  =    "RegLstFl"  ; 			
		break;

		case   External::REGIONLISTcHK :	              //  the  'regionList  ITSELF'   Chunk   in the file 
			retTagString  =    "RegnLIST" ; 			
		break;

		case   External::REGIONPROJECTfILEhEADERcHK :	     	//  the  'HEADER' for  the Project FILE
			retTagString  =     "RgPrgFil" ; 			
		break;




		case   External::LICKlISTcHK :	      	//   a list of  'LICKS"    ...musical phrases     12/2011
			retTagString  =     "LickLIST" ; 		
		break;




		default:      
			retErrorMesg =  "External::Get_Chunks_TagString FAILED,  missing chunkCode case." ;  
			return  false;
		break;
	}





	long  numChars  =   retTagString.GetLength();
	if(    numChars !=  8  )
	{
		retErrorMesg =  "External::Get_Chunks_TagString FAILED,  Tag Strings can only be 8 chars in length." ;  
		return  false;
	}

	return  true;
}



											////////////////////////////////////////


void    External::Write_Chunks_Length(    CFile&  file,   long  tagsLengthFilePos    )
{


	long   curFilePos  =     file.GetPosition();    //  Go back to the TAG and now write the length to the previous tag

	file.Seek(   tagsLengthFilePos,    CFile::begin    );



	long   dataLength  =   curFilePos  -  (  tagsLengthFilePos  + 4  );   //  +4 bytes:  for the version field in the ChunkTag to the end of tag.


	file.Write(   &dataLength,   4   );


	file.Seek(   curFilePos,    CFile::begin    );   // and return to current position
}



										////////////////////////////////////////


void    External::Write_Lists_Count(   long  numberOfListObjects,   CFile&  file  )
{

	long   ldSize  =    ( long )sizeof(       long            );     //  My current  PATHETIC EXCUSE  for a  File-HEADER     4/07

	file.Write(   &numberOfListObjects,    ldSize  );
}



										////////////////////////////////////////


long    External::Read_Lists_Count(   CFile&  file  )
{

			//  In the file, right after the List-Chunk's TAG,  are 4 bytes for a long of the number of objects in the file's ListSubject

	long  numberOfObjects =  0;


	long  ldSize  =    ( long )sizeof(     long       );      

	file.Read(   &numberOfObjects,    ldSize  ); 

	return  numberOfObjects;
}


											////////////////////////////////////////

	
bool    External::Write_Chunks_Header(   CFile&  file,    short  chunkCode,   unsigned char version,   long&  retSizeFilePos,   CString&  retErrorMesg  )
{


	retErrorMesg.Empty();
	retSizeFilePos =  -1;


	long   sizeBytesAfterChkHdr =  0;    //  will write in later



	CString   retTagString;


	if(    ! Get_Chunks_TagString(   chunkCode,    retTagString,   retErrorMesg  )     )
		return  false;




	file.Write(   retTagString.GetBuffer( 8 ),    8   );

	file.Write(   &version,    1  );     //	unsigned char   version =   10;    ....   1.0



	retSizeFilePos  =      file.GetPosition();     //  return filePos 'just before' we write the 4 bytes for chunk's size

	file.Write(   &sizeBytesAfterChkHdr,    4  );


	return  true;
}



											////////////////////////////////////////

	
bool    External::Goto_Chunks_Header(    CFile&  file,    short  chunkCode,   unsigned char&  retVersion,
																		                           long&  retChunksSize,   CString&  retErrorMesg  )
{

				// ****   want  OPTIMIZE search    9/06  ???    **********************


	CString   retTagString;
	long       curPos =  0L;  // ****** Always starts from the BEGINNING of the file. Is that good???  It is thourough.  9/06 ****


	if(    ! Get_Chunks_TagString(  chunkCode,   retTagString,   retErrorMesg  )     )
		return  false;



	long  filesLength =   file.GetLength();

	file.Seek(   curPos,    CFile::begin   );  // ***** BAD,  only occasionally search from beginning ******



	bool   stringNotFound =   true;

	CString    travStr =  "        ";   // cheezy way to put a NULL terminator in the string


	do			
	{  file.Read(   travStr.GetBuffer( 8 ),    8   );


		if(     retTagString.CompareNoCase(  travStr  )  ==   0     )
			stringNotFound =   false;
		else
		{  curPos++;

			if(    (curPos  +  8)   >=   filesLength    )    //   check for EOF  
			{
				retErrorMesg.Format(   "External::Goto_Chunks_Header FAILED,  could not find tag's string [ %s ] in the file( hit eof )." ,    retTagString  );
				return  false;
			}


			file.Seek(   curPos,    CFile::begin   );  
		}

	}while(  stringNotFound  );




	file.Read(   &retVersion,          1  ); 

	file.Read(   &retChunksSize,    4  );



//	long   curFilePos  =     file.GetPosition();   //   ...want to return this ???   **** DEBUG ONLY ****


	return  true;
}



											////////////////////////////////////////


bool   External::Does_FileName_Have_Extension(   char* fileExtensionPtr,    CString&   filePath   )
{

	bool   hasTheExtention =  false;						//   new,   1/10


	if(    filePath.IsEmpty()    )
	{
		ASSERT( 0 );    // ERROR,  should have something
		return   false;
	}

	if(   fileExtensionPtr ==  NULL    )
	{
		ASSERT( 0 );    // ERROR,  should have something
		return   false;
	}



	CString  strOrigExten;


	int   pos =    filePath.ReverseFind(  '.'  );    // Strip out the extension
	if(   pos  <= 0  )  
	{
		ASSERT( 0 );    // ERROR,  should have something
		return   false;
	}


	strOrigExten  =    filePath.Right(   filePath.GetLength()  - pos   -1  ); 


	if(    strOrigExten.CompareNoCase(  fileExtensionPtr  )  ==   0   )
		hasTheExtention =   true;
	else
		hasTheExtention =   false;



	return  hasTheExtention;
}



											////////////////////////////////////////


bool   External::Get_Files_Name_from_Path(    CString&   filesPath,    CString&   retFilesName    )
{


	retFilesName.Empty();


	int   slashPos  =    filesPath.ReverseFind(  '\\'  ); 


	if(    slashPos > 0   )
	{
		retFilesName =    filesPath.Right(    filesPath.GetLength()  -  (slashPos +1)    );

		return  true;
	}
	else      
	{	ASSERT( 0 );   // *************   IS this what I want ??????   12/11
		retFilesName =    filesPath;

		return  false;
	}
}



											////////////////////////////////////////


bool	 External::Is_File_Present(   CString&  filesPath,    CString&  retErrorMesg   )
{


	retErrorMesg.Empty();

	CString   exErrorMesg;
	int          causeCode =   CFileException::none;


	try
    {  CFile      file(    filesPath, 
	                          CFile::modeRead      //   |   CFile::shareExclusive 								
				  //	   |  CFile::typeBinary   ...BUGGY ????    5/02    Sets binary mode (used in derived classes only).
					     );


		//  Do nothing,  it will close the file with the destructor  

	}
	catch(   CFileException   *pException   )
	{
		TCHAR    szCause[ 255 ];  

		causeCode  =   pException->m_cause;

        pException->GetErrorMessage(  szCause,  255  );
		pException->Delete();
		exErrorMesg =  szCause;

		retErrorMesg.Format(   "Can not load file because:  %s" ,    exErrorMesg   );   //  Message might go to user.  9/06
	}



	/***      ... causeCodes :

	enum {
		none,
		genericException,
		fileNotFound,
		badPath,
		tooManyOpenFiles,
		accessDenied,
		invalidFile,
		removeCurrentDir,
		directoryFull,
		badSeek,
		hardIO,
		sharingViolation,
		lockViolation,
		diskFull,
		endOfFile
	};
	***/

	if(     causeCode  ==   CFileException::fileNotFound   )    //  duh !
		return  false;

																			//  Want to add other ACCEPTABLE errors to my  'return true'    ????

	if(      causeCode  ==   CFileException::none
		||   causeCode  ==   CFileException::sharingViolation    )   //  sharingViolation:   file is alread open   [  NEW,  5/16/07  ]
		return  true;



	if(  causeCode  ==   CFileException::none   )   //  a catch-all for anything of precedence above.
		return  true;
	else
		return  false;
}








