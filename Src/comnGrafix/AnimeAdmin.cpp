// AnimeAdmin.cpp   
//
//  Copyright (C) 2007   Creative Detectors (of San Francisco, California).  All Rights Reserved.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include  "stdafx.h"



//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"      //  #include <FixMath.h>    ...Mac's version of Fixedpoint 

#include  "..\ComnGrafix\CommonGrafix.h"      //  #include "commonmy.h"   		
#include  "..\ComnGrafix\OffMap.h"  


#include  "..\comnGrafix\TransformMap.h"    //  establishes  'chunkSize'  


#include   "..\ComnGrafix\AnimeStream.h"
//////////////////////////////////////////////////     





#include   "AnimeAdmin.h"

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


AnimeAdmin::AnimeAdmin()
{

//	m_animeStreams.Set_Dynamic_Flag(  false  );     //  false:   Need  'special'  De-Alloc routine( some are NOT resident  )
	m_animeStreams.Set_Dynamic_Flag(  true  );  // **** BETTER,  see  Delete_Streams()  ****



	m_chunkSize          =     TransformMap::Get_ChunkSize_OLD_PitchScope();    

	m_backroundTone  =  255;       // **** HARDWIRE ****


	m_virtualPixelCountAddr =   NULL;   

	m_auditionCode  =   0;     //  for  AUDIO - 'Animation'  (  user-FEEDBACK  is unifying issue )


	m_stereoChannelCode =    -1;
}


											////////////////////////////////////////


AnimeAdmin::~AnimeAdmin()
{

	bool  justUsesPointers =   false ;


	/***   No longer use this mode

	if(   m_stereoChannelCode ==  TransformMap::CENTEREDj   )
		justUsesPointers =   false;
	else
		justUsesPointers =   true;
	****/
	
	Delete_Streams(  true,    justUsesPointers   );



	Delete_Streams(  false,    false );
}


											////////////////////////////////////////


long	AnimeAdmin::Get_VirtualPixel_Width()
{

	long  width =  -1;

	if(    m_virtualPixelCountAddr  ==  NULL  )
	{
		ASSERT( 0 );
		return  -2;
	}
	else
	{  width  =      *m_virtualPixelCountAddr;
		return  width;
	}
}


											////////////////////////////////////////


AnimeStream*       AnimeAdmin::Get_AnimeStream(   short  streamCode  )
{


	AnimeStream   *stream  =   NULL;

	ListIterator< AnimeStream >    iter(   m_animeStreams   ); 


	 for(    iter.First();     !iter.Is_Done();     iter.Next()    )
	 { 
		AnimeStream&    streamCur =     iter.Current_Item();    


		if(    streamCur.m_streamCode  ==   streamCode    )
		{
			stream  =     &streamCur;
			break;
		}
	 }

	return  stream;
}


											////////////////////////////////////////


TransformMap*     AnimeAdmin::Get_AnimeStreams_Map(   short  streamCode  )
{


	TransformMap   *map  =   NULL;

	ListIterator< AnimeStream >    iter(   m_animeStreams   ); 


	 for(    iter.First();     !iter.Is_Done();     iter.Next()    )
	 { 
		AnimeStream&    stream =     iter.Current_Item();    


		if(    stream.m_streamCode  ==   streamCode    )
		{
			map  =     stream.m_transformMap;
			break;
		}
	 }

	return  map;
}



											////////////////////////////////////////


TransformMap*     AnimeAdmin::Get_MidiPitch_Map()        
{   


	TransformMap  *map =  NULL; 
	

	ListIterator< AnimeStream >   iter(   m_animeStreams   ); 


	 for(    iter.First();     !iter.Is_Done();     iter.Next()    )
	 { 
		AnimeStream&    stream =     iter.Current_Item();      //   get address stored in 'Item', assign to pointer


		if(    stream.m_streamCode  ==   AnimeStream::MIDImASK    )
		{
			map  =     stream.m_transformMap;
			break;
		}
	 }

	return  map;
}   


											////////////////////////////////////////


TransformMap*     AnimeAdmin::Get_Volume_Map()        
{   


	TransformMap  *map =  NULL; 
	
	ListIterator< AnimeStream >   iter(   m_animeStreams   ); 


	 for(    iter.First();     !iter.Is_Done();     iter.Next()    )
	 { 
		AnimeStream&    stream =     iter.Current_Item();      //   get address stored in 'Item', assign to pointer

		if(    stream.m_streamCode  ==   AnimeStream::VOLUMEmASK    )
		{
			map  =     stream.m_transformMap;
			break;
		}
	 }

	return  map;
}   



											////////////////////////////////////////


TransformMap**     AnimeAdmin::Get_MidiPitch_Map_Addr()     
{   


	TransformMap   **mapAddr =   NULL;

	ListIterator< AnimeStream >   iter(   m_animeStreams   ); 


	 for(    iter.First();     !iter.Is_Done();     iter.Next()    )
	 { 
		AnimeStream&    stream =     iter.Current_Item();      //   get address stored in 'Item', assign to pointer

		if(    stream.m_streamCode  ==   AnimeStream::MIDImASK    )
		{
			mapAddr  =     stream.Get_TransformMap_Addr();
			break;
		}
	 }


	return   mapAddr;   
}     




											////////////////////////////////////////


bool	  AnimeAdmin::Alloc_DST_Streams(    CString&  retErrorMesg   )
{

								//  also   ALLOCs  BITMAPs   for  'Resident-Map'   streams for   DSTlist  ONLY !!!



	short   midiMaskHeight =  1;   //   just an array of values 0 -255 
	short    midiMaskDepth =  8;




	retErrorMesg.Empty();


	long  width   =        Get_VirtualPixel_Width();    //  *** BUG, sometimes returns -1  *****   3/03  
	if(     width  <=  0   )
	{
		retErrorMesg =     "AnimeAdmin::Alloc_DST_Streams  failed,  width is too small. " ;
		return  false;	
	}



	Delete_Streams(  false,   false  );    //  CAREFUL 



																		//   ALLOC the streams


	AnimeStream   *midiStream   =     new   AnimeStream();    //  small memory usage

	if(   !midiStream->Alloc_Map(   width,   midiMaskHeight,   midiMaskDepth,   retErrorMesg  )   )
		return  false;	

	midiStream->m_streamCode =   	AnimeStream::MIDImASK;
	m_animeStreams.Add_Tail(   *midiStream       ); 





	AnimeStream   *volumeStream  =      new   AnimeStream();   
	
	if(   !volumeStream->Alloc_Map(   width,   midiMaskHeight,   midiMaskDepth,   retErrorMesg  )   )
		return  false;	
	
	volumeStream->m_streamCode =	  AnimeStream::VOLUMEmASK;
	m_animeStreams.Add_Tail(   *volumeStream   ); 




								//   BELOW  are   'VIRTUAL'  Streams  (  NO residentMap,  bitmap is in DetectZone  )


	AnimeStream   *yMaxStream  =      new   AnimeStream();   
	
	/********  NO, this is a   'VIRTUAL-Stream'  ( NO residentMap )

	if(   !yMaxStream->Alloc_Map(   width,   SCALEPITCHmaskHEIGHT,   SCALEPITCHmaskDEPTH,   retErrorMesg  )   )
		return  false;	
	****/	
	yMaxStream->m_streamCode          =	    AnimeStream::YMAXvIRT;
	yMaxStream->m_mapInDetectZone  =     true;    //   **BIG**    ...tells that this is  a    'VIRTUAL'  AnimeStream
	m_animeStreams.Add_Tail(   *yMaxStream   ); 




	AnimeStream   *fundamentalStream  =      new   AnimeStream();    
	
	fundamentalStream->m_streamCode          =	    AnimeStream::FUNDAMENTALvIRT;
	fundamentalStream->m_mapInDetectZone  =     true;    //    'VIRTUAL'  AnimeStream
	m_animeStreams.Add_Tail(   *fundamentalStream   ); 




	AnimeStream   *compositeStream  =      new   AnimeStream();    
	
	compositeStream->m_streamCode          =	    AnimeStream::COMPOSITEvIRT;
	compositeStream->m_mapInDetectZone  =     true;    //    'VIRTUAL'  AnimeStream
	m_animeStreams.Add_Tail(   *compositeStream   ); 



	return  true;
}




											////////////////////////////////////////


bool	  AnimeAdmin::Alloc_SRC_Streams(    CString&  retErrorMesg   )
{

								//  also   ALLOCs  BITMAPs   for  'Resident-Map'   streams for   DSTlist  ONLY !!!


	retErrorMesg.Empty();



	long  width   =        Get_VirtualPixel_Width();    //  *** BUG, sometimes returns -1  *****   3/03  
	if(     width  <=  0   )
	{
		retErrorMesg =     "AnimeAdmin::Alloc_SRC_Streams  failed,  width is too small." ;
		return  false;	
	}



	bool   doSRCstreams =   true;

	Delete_Streams(   doSRCstreams,    false   );   



																		//   ALLOC the streams

	AnimeStream   *amplitudeStream  =     new   AnimeStream();    //  small memory usage


		
	if(   !amplitudeStream->Alloc_Map(   width,   1,   8,   retErrorMesg  )   )
		return  false;	



	amplitudeStream->m_streamCode   =   	AnimeStream::AMPLITUDEmASK;
	amplitudeStream->m_isAsrcStream =   true;
	m_animeStreams.Add_Tail(   *amplitudeStream       ); 




	/****  Discontinued, takes too long

	AnimeStream   *amplitudeEdgeStream   =     new   AnimeStream();   

	if(   !amplitudeEdgeStream->Alloc_Map(   width,   1,   8,   retErrorMesg  )   )
		return  false;	

	amplitudeEdgeStream->m_streamCode   =   	AnimeStream::AMPLITUDEeDGEmASK;
	amplitudeEdgeStream->m_isAsrcStream =   true;
	m_animeStreams.Add_Tail(   *amplitudeEdgeStream       ); 
	*****/


	return  true;
}



											////////////////////////////////////////


void	AnimeAdmin::Delete_Streams(   bool   doSRCstreams,     bool   justSetMapPointers   )    
{    


	ASSERT(   ! justSetMapPointers   );     //  No longer use this mode  (  9/06 )



	if(    m_animeStreams.Is_Empty()     )		//   always need this TEST before  'SpeedIndexIterator'
		return;


	
//	ListIterator< AnimeStream >   iter(  m_animeStreams );  *****With  SpeedIndexIterator  I can use Remove_Item() ****
	ListLink< AnimeStream >*                    startLink =    m_animeStreams.Get_Head_Link();  
	SpeedIndexIterator< AnimeStream >    iter(   m_animeStreams,   startLink   );  



	 for(    iter.First();     !iter.Is_Done();     iter.Next()    )
	 { 

		AnimeStream&    stream =     iter.Current_Item();      //   get address stored in 'Item', assign to pointer


		/***
		if(   doSRCstreams   )
		{
			if(         stream.Is_A_SRC_Stream()
			   &&   ! stream.m_mapInDetectZone    	)	  	//  only delete RESIDENT maps
			{	
				stream.Delete_Map();

			//	delete   &stream;	 //  also delete the dynamically allocated AnimeStream( even if its Map RESIDES in a DetectZone )		
				m_animeStreams.Remove_Item(   stream  );
			}
		}
		else
		{
			if(       !  stream.Is_A_SRC_Stream()
			   &&   ! stream.m_mapInDetectZone    	)	  	//  only delete RESIDENT maps
			{
				stream.Delete_Map();

		//		delete   &stream;	
				m_animeStreams.Remove_Item(   stream  );
			}
		}
		***/

		if(    doSRCstreams    )
		{

			if(    stream.Is_A_SRC_Stream()  	)	  
			{	

			//	stream.Delete_Map();   *** Now   Remove_Item  does this
			//	delete   &stream;	 //  also delete the dynamically allocated AnimeStream( even if its Map RESIDES in a DetectZone )		

				if(   justSetMapPointers   )
					stream.m_transformMap =   NULL;   //  set it NULL, cause it points to CENTERchannels map


				m_animeStreams.Remove_Item(   stream  );   // **** Will this fails if NULL pointer ???
			}
		}
		else
		{
			if(    !  stream.Is_A_SRC_Stream()   	)	  
			{
			//	stream.Delete_Map();

		//		delete   &stream;	
				m_animeStreams.Remove_Item(   stream  );
			}
		}
	 }



//	m_animeStreams.Empty();       *** Now   Remove_Item  does this
}




											////////////////////////////////////////


void	AnimeAdmin::Erase_AnimeMask_Pair()
{

							//     'MASK':   the two  1 Pixel High bitmaps{  Midi + Volume ) 


	AnimeStream   *midiStream        =          Get_AnimeStream(   AnimeStream::MIDImASK   );
	if(                     midiStream  !=  NULL  )
		midiStream->Erase();
	else
		ASSERT( 0 );


	AnimeStream   *volumeStream    =         Get_AnimeStream(   AnimeStream::VOLUMEmASK   );
	if(                     volumeStream  !=  NULL  )
		volumeStream->Erase();
	else
		ASSERT( 0 );
}





											////////////////////////////////////////
											////////////////////////////////////////


bool     AnimeAdmin::Get_Current_Objects_Start(   long   xTransformCurrent,    long&  retCurNoteStart,    																				       
																		     short&  retCurNotePitch,   	 CString&  retErrorMesg   )  
{	

					// If the error message is empty, then it was NOT a real error.

	retErrorMesg.Empty();

	retCurNoteStart   =  -1;    
	retCurNotePitch   =  -1; 


	AnimeStream  *animeStream   =           Get_AnimeStream(   AnimeStream::MIDImASK   );
	if(                    animeStream ==  NULL )
	{
		retErrorMesg =  "AnimeAdmin::Get_Current_Objects_Start  failed,  animeStream is null." ;
		return  false;
	}


	if(   !animeStream->Get_Current_Events_Start(   xTransformCurrent,   retCurNoteStart,   retCurNotePitch,     retErrorMesg  )    )
		return  false;			// If the error message is empty, then it was NOT a real error.
	else
		return  true;
}




											////////////////////////////////////////


bool     AnimeAdmin::Get_Next_Objects_Start(   long   xTransformCurrent,    long&  retNextNoteStart,    																				       
																		       short&  retNextNotePitch,   	 CString&  retErrorMesg   )  
{	

			// If the error message is empty, then it was NOT a real error.

	AnimeStream  *animeStream   =           Get_AnimeStream(   AnimeStream::MIDImASK   );
	if(                    animeStream ==  NULL )
	{
		retErrorMesg =   "AnimeAdmin::Get_Next_Objects_Start  failed,  animeStream is null." ;
		return  false;
	}


	if(   !animeStream->Get_Next_Events_Start(    xTransformCurrent,    retNextNoteStart,   retNextNotePitch,    retErrorMesg  )     )
		return  false;						// If the error message is empty, then it was NOT a real error.
	else
		return  true;
}

