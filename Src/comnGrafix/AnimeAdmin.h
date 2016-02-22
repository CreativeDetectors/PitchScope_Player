//  AnimeAdmin.h -  
//
//
//  Copyright (C) 2007   Creative Detectors (of San Francisco, California).  All Rights Reserved.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_ANIMEADMIN_H__56011302_26EB_11D7_9A48_00036D156F73__INCLUDED_)
#define AFX_ANIMEADMIN_H__56011302_26EB_11D7_9A48_00036D156F73__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////////




class   TransformMap; 



class   AnimePacket;

class   AnimeStream;


//  class   ScalepitchSubject;       ...TRY to keep these    'INVISIBLE  to  AnimeAdmin'  
//  class   ScalepitchlistSubject;





////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


//	i)   What is the  COUPLING[ interface issues ]   of the ScalepitchLIST  and  AnimeAdmin

		//  do NOT want    AnimeAdmin     to 'KNOW'    about   'ScalepitchlistSubject' ( its  DataSource  &  Creator  )
		//        ...but OK for  ScalepitchlistSubject   to  SEE INTO   AnimeAdmin   


//	ii)	  SHOULD  the   BackroundMaps ( or mechanism )   also be CONTAINED( or REFERENCED )  in  AnimeAdmin  ???? 


//	iii)   A function that returns all  CompSubjects/Views that intersect current 'Frame' ( temporal BoundBox ...3D ??  




////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


class     AnimeAdmin		   
{

				//   TRICKY,   have  2 'types':   {  for  DSTlists    &    SRC-WAVfile [ Amplitude mask ]    }   ...should split up.

public:
	AnimeAdmin();
	virtual ~AnimeAdmin();



    virtual    bool					    Alloc_DST_Streams(   CString&   retErrorMesg    );    //   'Anime-Masks' :    the two  1 Pixel High bitmaps{  Midi + Volume ) 
		
   virtual    bool					    Alloc_SRC_Streams(   CString&  retErrorMesg   );    //   only for   'Amplitude'-Transform

   virtual    void					   Delete_Streams(   bool   doSRCstreams,    bool   justSetMapPointers   );




	virtual    AnimeStream*       Get_AnimeStream(           short   streamCode    );

	virtual    TransformMap*      Get_AnimeStreams_Map(   short  streamCode    );



	virtual    TransformMap**     Get_MidiPitch_Map_Addr();




	virtual    TransformMap*       Get_MidiPitch_Map();      //  Necessary OVERIDES from FrameArtist
	virtual    TransformMap*       Get_Volume_Map();





	virtual    long						Get_VirtualPixel_Width();        
	virtual    void						Set_VirtualPixel_Width_Addr(  long  *addr  )    {   m_virtualPixelCountAddr =   addr;   }	




    virtual    void					    Erase_AnimeMask_Pair();   //   'MASK':   the two  1 Pixel High bitmaps{  Midi + Volume ) 






    virtual    bool					Get_Current_Objects_Start(   long  xTransformCurrent,    long&  retCurNoteStart,    																				       
																										short&  retCurNotePitch,   CString&  retErrorMesg   );  

    virtual    bool					Get_Next_Objects_Start(   long  xTransformCurrent,    long&  retNextNoteStart,    																				       
																										 short&  retNextNotePitch,    CString&  retErrorMesg  );





private:
 




public:
	ListMemry< AnimeStream >     m_animeStreams;    


	short              m_stereoChannelCode;    


	short             m_auditionCode;     //  now  AUDIO !!! 

	long			*m_virtualPixelCountAddr;    //  ( in ScalepitchListSubject )  How many  VIRTUAL-pixels[ chunks ]  would the  'ENTIRE Sample'  possibly consume

	long			 m_chunkSize;  



	short            m_backroundTone;     //  really is a  PLACE-HOLDER   for   "Unassigned Value'
};






////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_ANIMEADMIN_H__56011302_26EB_11D7_9A48_00036D156F73__INCLUDED_)
