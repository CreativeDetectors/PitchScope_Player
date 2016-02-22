// AnimeStream.cpp  -     
//
//  Copyright (C) 2007   Creative Detectors (of San Francisco, California).  All Rights Reserved.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"




#include  "..\comnFacade\VoxAppsGlobals.h"

//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"      //  #include <FixMath.h>    ...Mac's version of Fixedpoint 

#include  "..\ComnGrafix\CommonGrafix.h"      //  #include "commonmy.h"   		
#include  "..\ComnGrafix\OffMap.h"  
#include  "..\comnGrafix\TransformMap.h"

#include   "..\ComnGrafix\AnimePacket.h"
#include  "..\comnGrafix\FrameArtist.h"

#include  "..\comnAudio\FundamentalCandidate.h"
//////////////////////////////////////////////////     



#include "..\comnAudio\FundamentalTemplateTrForm.h"

#include  "..\ComnAudio\CompositeTrForm.h"  



#include   "AnimeStream.h"

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


AnimeStream::AnimeStream()
{

	m_mapInDetectZone =     false;     //   **** BIG default


	m_detectZonesOffset =   0;


	m_chunkSize           =         TransformMap::Get_ChunkSize_OLD_PitchScope();       //  default

	m_backroundTone   =      255;       // **** HARDWIRE ****       255: WHITE


	m_maxPixelValue        =   200;       // ****  HARDWIRED ********** WHY this limit ???  *******



	m_streamCode    =        AnimeStream::UNKNOWN;      //   TransMapAdmin::NONE   &   

	m_isAsrcStream   =    false;    //  default

	m_transformMap =    NULL;
}



											////////////////////////////////////////


AnimeStream::~AnimeStream()
{

	Delete_Map();
}


											////////////////////////////////////////


void	AnimeStream::Delete_Map()   
{    

	if(         ! m_mapInDetectZone  
		 &&     m_transformMap  !=   NULL    )																					
	{  
		delete  m_transformMap;      		
		m_transformMap = NULL;   
	}		
}


											////////////////////////////////////////


short   AnimeStream::Get_Transforms_Height()
{

										//  really is the  'VIRTUAL-Range'  of the transfom   [ ex: Though the MidiMASK is only 
										//                                       1 pixel heigh,  its  'VirtualRANGE'  in ScalePitches is  12  ]
	short   height =  -1;


	switch(   m_streamCode   )
	{

		case   MIDImASK:  	         height =   12;			   	  break;

		case   VOLUMEmASK:  	  height =   -2;			   	break;      //  undefined



		case   YMAXvIRT:  	               height =   12;			   						     break;   

		case   FUNDAMENTALvIRT:  	 height =   FUNDtEMPLATEmapHEIT;	   break;   

		case   COMPOSITEvIRT:  	       height =   kCompositeMAPHEIT;	      break;   



 
		case   AMPLITUDEmASK:  	         
		case   AMPLITUDEeDGEmASK :	
													height =   1;			   	  break;    // *****  1:  OK ???  ********





		case   UNKNOWN:  	         height =   -3;			   	  break;      //  undefined

		default:	ASSERT( 0 ); 	 height =  -1;				break;
	}


	return   height;
}





											////////////////////////////////////////


void	AnimeStream::Erase()
{

	if(    m_transformMap  ==  NULL   )
	{
		//  retErrorMesg =  "AnimeStream::Erase failed,  map is null." ;
		ASSERT( 0 );
		return;
	}


	if(    m_mapInDetectZone    )
	{
		//  retErrorMesg =  "AnimeStream::Erase only resident maps can be Erased." ;
		ASSERT( 0 );
		return;
	}


	m_transformMap->Clear(  m_backroundTone   );      //   255:  white  ...play it safe in case this is called repetively    
}




											////////////////////////////////////////


bool   AnimeStream::Alloc_Map(   long  width,   long height,    long  depth,    CString&   retErrorMesg   )
{


				//  Calling function must call :    Build_AnimeMask(  sPitchList,   retErrorMesg   )


	bool   hasChannels =    false;    // **** HARDWIRED ****



	retErrorMesg.Empty();


	if(    m_mapInDetectZone    )
	{
		retErrorMesg =    "AnimeStream::Alloc_Map  failed,  trying to delete non-Resident map." ;   //  in DetectZone
		return  false;
	}



	if(    m_transformMap   !=   NULL    )				
		 Delete_Map();




	ASSERT(          m_transformMap  ==   NULL    
				   &&   width  > 0    );


	m_transformMap     =  	 new     TransformMap(   width,   height,   depth,  	m_chunkSize   );   
	if(  m_transformMap  ==  NULL  )
	{
		retErrorMesg =  "AnimeAdmin::Alloc_Map  failed,  not alloc map." ;  
		return  false;
	}


	m_transformMap->m_hasChannels  =              hasChannels;

	m_transformMap->m_channelCount =    (short)height;

	m_transformMap->Clear(   m_backroundTone   );       //   default:   255,   white


	return  true;
}




											////////////////////////////////////////


bool	 AnimeStream::Get_TimeSlice_Vals(   long x,    short  retArray[],   short  arraySize,    bool  reverseOrder,  
																												CString&  retErrorMesg   )
{

	retErrorMesg.Empty();




	long   xRelative =   x;			//   DetectZone map OFFSETTING for  ITS maps 
								

	if(    m_mapInDetectZone    )
	{
		xRelative  =     x   -   m_detectZonesOffset;  
	}
	else
		xRelative  =    x;





	for(    short  i =0;     i < arraySize;     i++   )       //   init  results
		retArray[ i ]  =    0;


	TransformMap  *map    =	         Get_TransformMap();    //  Maybe have to search  DetectZones
	if(                     map == NULL  )
	{
		retErrorMesg =  "AnimeStream::Get_TimeSlice_Vals failed,  transformMap is NULL." ;
		return   false;
	}
	



	short   val, gr, bl;      //    val:   Grey-maps only


	if(    reverseOrder    )
	{

		short   aryIdx =  0;
		

		for(  short  i=  ( arraySize -1 );     i >= 0;       i--    )     
		{

			map->Read_Pixel(   xRelative,    i,    &val,  &gr, &bl   );    

			retArray[ aryIdx ]  =    val;      //  but must INDEX in reverse
			aryIdx++;
		}
	}
	else
	{  for(  short  i= 0;        i <  arraySize;          i++   )     
		{

			map->Read_Pixel(    xRelative,    i,    &val,  &gr, &bl   );    
			retArray[ i ]  =    val;      
		}
	}


	return  true;
}





											////////////////////////////////////////

/***
bool   AnimeStream::Link_AnimeMask(   TransformMap&   transformMap,    CString&   retErrorMesg   )
{


// ***********************  FIX *****************************

				// **** OK ???   Will be difficult for   DetectZones  MULTIPLE maps   *******


	retErrorMesg.Empty();


	if(    ! m_mapInDetectZone    )
	{
		retErrorMesg =    "AnimeStream::Link_AnimeMask  failed,  trying to link a Resident map." ;
		return  false;
	}


	m_transformMap =      &transformMap;

	return  true;
}
***/



											////////////////////////////////////////


TransformMap*	  AnimeStream::Get_TransformMap()
{

	if(    m_mapInDetectZone    )
	{
		return   m_transformMap;   //   OK,   PitchProject::Initialize_AnimePacket()  assigned this,  
												//            MIGHT be NULL  if  DetectZone did not intersect current TimePosition
	}
	else
	 	return   m_transformMap;
}



											////////////////////////////////////////


void    AnimeStream::Write_TimeSpans_Pixel_Values(   long xStart,   long  xEnd,      long  pitch   )
{


	CString   retErrorMesg;


	TransformMap    *midiPitchMap  =      Get_TransformMap(); 

	if(   midiPitchMap  ==  NULL  )     //  we  'really'  tested this previously with  Reset()
	{
		ASSERT( 0 );
		return;
	}





	short    val,  gr,  bl;

	for(   long  x=  xStart;      x <=  xEnd;      x++    )
	{							//  Only write where   'NOTyet written'   allows ATTACK to be preserved for Concurrent lists  

		midiPitchMap->Read_Pixel(   x, 0,    &val,  &gr,&bl   );    

		if(    val  ==  m_backroundTone   )
			midiPitchMap->Write_Pixel(    x, 0,    (short)pitch,  (short)pitch,  (short)pitch   ); 
	}
}





											////////////////////////////////////////


bool     AnimeStream::Get_Current_Events_Start(   long  x,    long&  retRunsStartXcoord,    																				       
																		        short&  retRunsPixelValue,   	 CString&  retErrorMesg   )  
{	

			 //   'Event' :      FORBIDS  Rests( 255 or 254 ?? )

		//   RETURNS:   If the error message is empty, then it was NOT a real error ...just no more Notes at end of DetectZone



	short    bottomMidiVal  =      64;      // ****  HARDWIRED ******    the Midi value of the bottom row on the Glyph


	short    gr, bl, curPixVal,    lastPixVal = -1;
	long    xTrav;


	retErrorMesg.Empty();
	retRunsStartXcoord   =  -1;    
	retRunsPixelValue   =  -1; 



	TransformMap    *midiPitchMap	=	 Get_TransformMap(); 
	if(   midiPitchMap  ==   NULL  )
	{
		retErrorMesg =  "AnimeStream::Get_Current_Events_Start failed,  transformMap is NULL." ;
		return  false;
	}




	if(      midiPitchMap  ==  NULL 
		||  midiPitchMap->m_height  <=  0   )
	{
		retErrorMesg =  "AnimeStream::Get_Current_Events_Start  failed,  scalePitchMask is null." ;
		return  false;
	}




	midiPitchMap->Read_Pixel(   x,  0,      &curPixVal,  &gr,  &bl    );

	lastPixVal =   curPixVal;  


	if(     retRunsPixelValue ==  -1      &&    curPixVal   !=  m_backroundTone   )   //  keep trying to get the pitch
		retRunsPixelValue =   curPixVal;


	if(   x ==  0  )
	{
		if(   retRunsPixelValue ==  -1   )
			retRunsPixelValue  =   bottomMidiVal;

		retRunsStartXcoord =  0;
		return  true;
	}




	for(    xTrav =  ( x  -1 );      xTrav  >=  0;      xTrav--    )
	{

		midiPitchMap->Read_Pixel(   xTrav,  0,    &curPixVal,  &gr,  &bl    );

		if(     retRunsPixelValue ==  -1      &&    curPixVal   !=  255   )    //  keep trying to get the CORRECT pitch
			retRunsPixelValue =   curPixVal;

		


		if(      ( curPixVal   !=   lastPixVal	   &&  	  lastPixVal  !=  m_backroundTone    )  //   Ending either a { NOTE or REST  },  but STARTING a new Note   
			||   (   xTrav  ==   0   )     )		//   ...or at  Map's beginning.
		{


			if(   xTrav  ==   0   )				 //  the 'exception' case
			{

				retRunsStartXcoord =   0;

				if(    curPixVal   ==  255     &&    retRunsPixelValue ==  -1    )
					retRunsPixelValue  =   bottomMidiVal;

				return  true;
			}



			retRunsStartXcoord  =    xTrav   + 1;
			retRunsPixelValue    =    lastPixVal;

			break;    //  We are DONE !!!			
		}   


		lastPixVal =   curPixVal;
	} 



	if(          retRunsStartXcoord  < 0 
		||   (  retRunsPixelValue   < 0      ||    retRunsPixelValue  > m_maxPixelValue  )     )  //  FORBIDS  Rests( 255 or 254 ?? )
	{
//		retErrorMesg =   "AnimeStream::Get_Current_Events_Start  failed to get results." ;
		//   NOT a real error   ...just no more Notes at end of DetectZone    7/06

		return  false;
	}
	else
		return  true;
}



											////////////////////////////////////////


bool     AnimeStream::Get_Next_Events_Start(   long  x,    long&  retNextRunsStartXcoord,    																				       
																		             short&  retNextRunsPixelValue,   	 CString&  retErrorMesg   )  
{	


			//   RETURNS:   If the error message is empty, then it was NOT a real error ...just no more Notes at end of DetectZone




			  //   'Event' :      FORBIDS  Rests( 255 or 254 ?? )


	short     gr, bl, curPixVal,    lastPixVal = -1;
	long      xTrav;

	short   bottomMidiVal  =   64;   //  the Midi value of the bottom row on the Glyph


	retErrorMesg.Empty();

	retNextRunsStartXcoord =  -1;    
	retNextRunsPixelValue   =  -1; 


	TransformMap    *midiPitchMap	=	 Get_TransformMap(); 
	if(   midiPitchMap  ==   NULL  )
	{
		retErrorMesg =  "AnimeStream::Get_Next_Events_Start  failed,  transformMap is NULL." ;
		return  false;
	}




	if(       midiPitchMap  ==  NULL 
		||   midiPitchMap->m_height   <=  0   )
	{
		retErrorMesg =  "AnimeStream::Get_Next_Events_Start  failed,  scalePitchMask is null." ;
		return  false;
	}




	midiPitchMap->Read_Pixel(   x,  0,      &curPixVal,  &gr,  &bl    );

	lastPixVal =   curPixVal;  


	if(     retNextRunsPixelValue ==  -1      &&    curPixVal   !=  m_backroundTone   )   //  keep trying to get the pitch
		retNextRunsPixelValue =   curPixVal;


	if(    x   ==    (midiPitchMap->m_width -1)    )
	{
		if(   retNextRunsPixelValue ==  -1   )
			retNextRunsPixelValue  =   bottomMidiVal;

		retNextRunsStartXcoord  =   (midiPitchMap->m_width -1);
		return  true;
	}




	for(    xTrav =  ( x  +1 );      xTrav <  midiPitchMap->m_width;      xTrav++    )
	{

		midiPitchMap->Read_Pixel(   xTrav,  0,    &curPixVal,  &gr,  &bl    );
		

		if(      ( curPixVal   !=   lastPixVal	   &&  	 curPixVal  !=  m_backroundTone   )  //   Ending either a { NOTE or REST  },  but STARTING a new Note   
			||   (   xTrav  ==   (midiPitchMap->m_width -1)    )       )  //   ...or at  Map's beginning.
		{


			if(    xTrav  ==   (midiPitchMap->m_width -1)    )		 //  the 'exception' case
			{
				retNextRunsStartXcoord =    midiPitchMap->m_width  - 1;

				if(    curPixVal   ==  255     &&    retNextRunsPixelValue ==  -1    )
					retNextRunsPixelValue  =   bottomMidiVal;

				return  true;
			}



			retNextRunsStartXcoord  =    xTrav;
			retNextRunsPixelValue  =    curPixVal;

			if(   retNextRunsPixelValue  ==  m_backroundTone   )    //   might happen  for [  (   xTrav  ==   (midiPitchMap->m_width -1)   ]  
			{
				ASSERT( 0 );    //    int  dummyBreak =    9;
				retNextRunsPixelValue  =   bottomMidiVal;  
			}


			break;    //  We are DONE !!!			
		}   


		lastPixVal =   curPixVal;
	} 





	if(          retNextRunsStartXcoord  < 0 
		||   (  retNextRunsPixelValue < 0    ||    retNextRunsPixelValue  > m_maxPixelValue  )     )     //  FORBIDS  Rests( 255 or 254 ?? )
	{
//		retErrorMesg =   "AnimeStream::Get_Next_Events_Start  failed to get reasonable results." ;

	//   NOT a real error   ...just no more Notes at end of DetectZone    7/06		
		
		return  false;
	}
	else
		return  true;
}



											////////////////////////////////////////


short	  AnimeStream::Get_Current_ColorRun(   long  x,     long&  retXstart,     long&  retXend   )
{

	CString   retErrorMesg;


	TransformMap   *map     = 	     Get_TransformMap();
	if(                      map ==  NULL )
	{
		ASSERT( 0 );
		return  -1;
	}
		

	short     eventsPitch  =     map->Find_Current_Interval(    x,   retXstart,    retXend   );  
	return   eventsPitch;    
}





