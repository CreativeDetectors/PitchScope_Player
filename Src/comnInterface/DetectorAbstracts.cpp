/////////////////////////////////////////////////////////////////////////////
//
//  DetectorAbstracts.cpp   -   
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


#include  "..\comnFacade\VoxAppsGlobals.h"


//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"    

#include  "..\ComnGrafix\CommonGrafix.h"    
#include  "..\ComnGrafix\OffMap.h"  
#include  "..\comnGrafix\TransformMap.h"


//////////////////////////////////////////////////        









#include   "DetectorAbstracts.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


BitSource::BitSource()
{
	m_bitSourceKind =  UNKNOWN;

	m_isInitialized =  false;


	m_sepIsCurrent =  false;  	//   false:   if it is new, then it has NOT been Notelist Processed.  But if it has been loaded from file.

	m_currentByteIdx =  0;
}

									/////////////////////////////////////////////////////

BitSource::~BitSource()
{
}



									/////////////////////////////////////////////////////


bool   BitSource::Does_FileName_Have_Extension(   char* fileExtensionPtr,    CString&   filePath   )
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




			//////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////


SourceAdmin::SourceAdmin()
{

	//	m_bitSources.Set_Dynamic_Flag(  false  );     //  false:   Need  'special'  De-Alloc routine( some are NOT resident  )
	m_bitSources.Set_Dynamic_Flag(  true  );  // **** BETTER,  see  Delete_Streams()  ****


	m_currentBitSource =   NULL;
}


									/////////////////////////////////////////////////////

SourceAdmin::~SourceAdmin()
{
}


									/////////////////////////////////////////////////////


bool     SourceAdmin::Is_Separation_Updated(     short   bitsourceCode  )
{

	BitSource  *bitSrc =   Get_BitSource(   bitsourceCode   );   // also works fine if the Separation is not initialized, or is missing  5/07
	if(  bitSrc ==  NULL  )
	{
		return  true;   //  nothing to update, so our work is done
	}


	return  bitSrc->m_sepIsCurrent;
}

									/////////////////////////////////////////////////////


void	 SourceAdmin::Set_Separation_Updated(   short   bitsourceCode,    bool  isUpdated )
{

	BitSource  *bitSrc =   Get_BitSource(  bitsourceCode   );
	if(  bitSrc ==  NULL  )
	{
		return;   //  undefined
	}


	bitSrc->m_sepIsCurrent  =     isUpdated;
}


									/////////////////////////////////////////////////////


BitSource*     SourceAdmin::Get_BitSource(   short   bitsourceCode   )
{
	
	//   CALLING function s will have to be sure that it is NOT tring to do 


	BitSource  *foundBitSource =  NULL; 
	

	ListIterator< BitSource >   iter(   m_bitSources   ); 


	 for(    iter.First();     !iter.Is_Done();     iter.Next()    )
	 { 

		BitSource&    bitSource =     iter.Current_Item();      //   get address stored in 'Item', assign to pointer


		if(    bitSource.m_bitSourceKind  ==   bitsourceCode    )
		{
			foundBitSource  =     &bitSource;
			break;
		}
	 }


	return  foundBitSource;
}


									/////////////////////////////////////////////////////


bool     SourceAdmin::Get_BitSources_FilePath(   short   bitsourceCode,    CString&   retFilePath,    CString&   retErrorMesg      )
{
	
	//   CALLING function s will have to be sure that it is NOT tring to do 

	retErrorMesg.Empty();
	retFilePath.Empty();


	BitSource  *foundBitSource =  NULL; 
	

	ListIterator< BitSource >   iter(   m_bitSources   ); 


	 for(    iter.First();     !iter.Is_Done();     iter.Next()    )
	 { 

		BitSource&    bitSource =     iter.Current_Item();      //   get address stored in 'Item', assign to pointer


		if(    bitSource.m_bitSourceKind  ==   bitsourceCode    )
		{
			foundBitSource  =     &bitSource;
			break;
		}
	 }



	 if(   foundBitSource ==  NULL  )
	 {
		retErrorMesg.Format(   "Get_BitSources_FilePath  FAILED, it could find no bitSource for bitsourceCode = %d ",   bitsourceCode   ); 
		return  false;
	 }


	retFilePath =    foundBitSource->m_strWavFilePath;


	if(   retFilePath.IsEmpty()    )
	 {
		retErrorMesg.Format(   "Get_BitSources_FilePath  FAILED, the file path that it found was empty [ bitsourceCode = %d ]",   bitsourceCode   ); 
		return  false;
	 }


	return  true;
}



									/////////////////////////////////////////////////////


bool     SourceAdmin::Set_Current_BitSource(   short   bitsourceCode,      CString&  retErrorMesg      )
{


	ListIterator< BitSource >   iter(   m_bitSources   ); 


	 for(    iter.First();     !iter.Is_Done();     iter.Next()    )
	 { 
		BitSource&    bitSource =     iter.Current_Item();      //   get address stored in 'Item', assign to pointer


		if(    bitSource.m_bitSourceKind  ==   bitsourceCode    )
		{
			m_currentBitSource =     &bitSource;

			return true;
		}
	 }



	retErrorMesg.Format(  "SourceAdmin::Set_Current_BitSource FAILED to set  Bitsource of this kindCode[ %d ].",    bitsourceCode   );
	return  false;
}





									/////////////////////////////////////////////////////


void	SourceAdmin::Delete_BitSources(  /*   bool   justSetMapPointers */    )    
{    

	ASSERT( 0  );   //  never gets called???    


	if(    m_bitSources.Is_Empty()     )		//   always need this TEST before  'SpeedIndexIterator'
		return;


	
//	ListIterator< AnimeStream >   iter(  m_bitSources );  *****With  SpeedIndexIterator  I can use Remove_Item() ****
	ListLink< BitSource >*                    startLink =    m_bitSources.Get_Head_Link();  
	SpeedIndexIterator< BitSource >    iter(   m_bitSources,   startLink   );  



	 for(    iter.First();     !iter.Is_Done();     iter.Next()    )
	 { 

		BitSource&    bitSource =     iter.Current_Item();      //   get address stored in 'Item', assign to pointer

//		if(   justSetMapPointers   )
//			stream.m_transformMap =   NULL;   //  set it NULL, cause it points to CENTERchannels map

		m_bitSources.Remove_Item(   bitSource  );   // **** Will this fails if NULL pointer ???
	 }



//	  m_bitSources.Empty();       *** Now   Remove_Item  does this
}



									/////////////////////////////////////////////////////


void	SourceAdmin::Release_All_BitSources_Files()    
{    

			//  does NOT kill the lists, just closes all the files  and sets   m_isInitialized =  false    


	if(    m_bitSources.Is_Empty()     )		//   always need this TEST before  'SpeedIndexIterator'
		return;


	ListLink< BitSource >*                    startLink =    m_bitSources.Get_Head_Link();  
	SpeedIndexIterator< BitSource >    iter(   m_bitSources,   startLink   );  


	 for(    iter.First();     !iter.Is_Done();     iter.Next()    )
	 { 

		BitSource&    bitSource =     iter.Current_Item();      //   get address stored in 'Item', assign to pointer

		bitSource.Release_All_Resources();
	 }
}

