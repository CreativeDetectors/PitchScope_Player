// AnimeStream.h -   
//
//
//  Copyright (C) 2007   Creative Detectors (of San Francisco, California).  All Rights Reserved.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_ANIMESTREAM_H__7154B801_1C18_11D8_9A49_00036D156F73__INCLUDED_)
#define AFX_ANIMESTREAM_H__7154B801_1C18_11D8_9A49_00036D156F73__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////////


class   TransformMap; 





////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


class    AnimeStream  
{

public:
	AnimeStream();
	virtual ~AnimeStream();



	virtual    bool				  Is_A_SRC_Stream()			{   return   m_isAsrcStream;   }



	virtual    bool				 Get_TimeSlice_Vals(   long x,    short  retArray[],    short  arraySize,   bool  reverseOrder,   CString&   retErrorMesg   );

	virtual    void				 Write_TimeSpans_Pixel_Values(    long xStart,   long  xEnd,     long  pitch   );



	virtual    short			Get_Transforms_Height();    //  Virtual 'RANGE'  of values.    ...for  MidiMASK this is 12( like YMax ),  even though map is 1 Pixel high





																//   'EVENT' :     a Note,   but NOT  Rests
 
	virtual    bool				 Get_Current_Events_Start(   long   x,    long&  retRunsStartXcoord,    																				       
																		                      short&  retRunsPixelValue,   	 CString&  retErrorMesg   );  

	virtual    bool				 Get_Next_Events_Start(      long   x,    long&  retNextRunsStartXcoord,    																				       
																		                    short&  retNextRunsPixelValue,   	 CString&  retErrorMesg   );  




																//   'ColorRun' :    BOTH  Notes  AND  Rests
 
	virtual    short	         Get_Current_ColorRun(   long  x,     long&  retXstart,     long&  retXend   );     







    virtual    bool				 Alloc_Map(     long  width,   long height,    long  depth,    CString&   retErrorMesg   );    //   'Mask' :   MULTIPLE bitmaps
	virtual    void				 Delete_Map();   

	virtual    void				 Erase();    //   WAS   Reset()   



	virtual    TransformMap**       Get_TransformMap_Addr()      {   return   &m_transformMap;   }   //  Not used ????   7/06


	virtual    TransformMap*		 Get_TransformMap();     //   for  DetectZone Map  'SELECTION'






public:
	TransformMap    *m_transformMap;	   	 //	Can be NULL    if( m_mapInDetectZone ),  but DZone is NOT in current timeFocus


	bool					m_mapInDetectZone;   //  tells if VIRTUAL[ residing in DetectZone ]   ...BUT,   m_transformMap can be NULL    12/03


	short			        m_streamCode;          //  ??  SHOULD be same vals as in  'TransMapAdmin',   OR BELOW ??  {  MIDIaNIMEmASK,  VOLUMEaNIMEmASK    

	bool					m_isAsrcStream;



	long				m_detectZonesOffset;			//  can be -1 if no DetectZone intersects


	long				m_chunkSize;

	short               m_backroundTone;     //  really is a  PLACE-HOLDER   for   "Unassigned Value'




	short               m_maxPixelValue;    // ***** WEIRD,  re-examine    10/03 *********





	enum   auditionTypes    
		{  NORMAL,     MIDIandSAMPLE,     AllLEFT,  AllRIGHT,     JUSTMIDI,     MIDIandLEFT,   MIDIandRIGHT    };  
//	 MIDIandSAMPLE( mid plus stereo ),  MIDIandLEFT( midi and JustLeft WAV  )											




	enum   animeStreamTypes     
		{  UNKNOWN,         MIDImASK,    VOLUMEmASK,    AMPLITUDEmASK,   AMPLITUDEeDGEmASK,    YMAXvIRT,    FUNDAMENTALvIRT,   COMPOSITEvIRT      };  
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_ANIMESTREAM_H__7154B801_1C18_11D8_9A49_00036D156F73__INCLUDED_)
