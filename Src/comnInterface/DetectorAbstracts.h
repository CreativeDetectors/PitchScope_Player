/////////////////////////////////////////////////////////////////////////////
//
//  DetectorAbstracts.h   -   
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined(DETECTORaBSTRACTS_H__INCLUDED_)
#define DETECTORaBSTRACTS_H__INCLUDED_


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////    NOTES ( start  )     ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
/***

New Classes :   5/07


1)  A new Detector  ( like ChannelDetector )   ...want new Detector class and subclass  ChannelDetector   from it ???

2)  New strategy for show ing Bitmaps  from the Detector  ( new abstract class  BitmapAdmin ???  and suclass  TransmapAdmin from it.

...try to move as much of their function to DetectorApp.   

		...first implement these in   GfxRegionDetector


***/
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////


class   BitSource   
{

public:
	BitSource();
	virtual ~BitSource();



	virtual     long			Get_Files_Total_Samples_No_SpeedExpansion()   =0;    

	virtual     long		    Calc_Files_Total_Output_Bytes_With_SpeedExpansion()  =0;    



	virtual		short			Get_Bitsource_Kind()    {  return  m_bitSourceKind;   }


	virtual     void		    Get_Wav_Filename(     CString&   retFilePath   )    {   retFilePath =  m_strWavFilePath;    }

	virtual     void		    Get_ParentFiles_Path(  CString&   retFilePath   )    {   retFilePath =  m_parentFilePath;   }



	virtual     bool            Is_Initialized()  {  return  m_isInitialized;  }     //  for PScope,  means that the file is OPEN,  and ready to fewtch data   5/07

	virtual     void            Release_All_Resources() =  0;    //  closes all the files, but keeps the BitStreams around for the next.   Kind of a DE-initialization


	virtual	  void			Initialize_For_Playing()   {  ASSERT( 0 );  }       //  NEW,  2/8/2012,  to make it easier to keep track of play




	static	     bool				Does_FileName_Have_Extension(   char* fileExtensionPtr,    CString&   filePath   );




public:
	CString   m_strWavFilePath;         //  might be the name for  {  LEAD,   BACKGROUND,  MODIFIEDwAV,    or even    SOURCEwav  ] 

	CString   m_parentFilePath;       //  always the name for    'SOURCEwav'
	
	short      m_bitSourceKind;


	bool       m_isInitialized;


	bool       m_sepIsCurrent;       //  true :  means that all the Separations are in sync with the current Notelist  


	long	     m_currentByteIdx;      //  more to upper level




	enum  srcKinds {  UNKNOWN,   SOURCEwav,   LEAD,   BACKGROUND,   MODIFIEDwAV,    SOURCEmp3,   SOURCEsTREAMwav,    SOURCEgrafix   };
							//   SOURCEsTREAMwav is new   2/10



	enum   directionCodes  {  FORWARD,   BACKWARDS,   FORCEiNIT,    PREROLLinit    };		//  just for Audio
};     






			//////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////


class   SourceAdmin     //   similar to   AnimeAdmin
{

public:
	SourceAdmin();
	virtual ~SourceAdmin();


   virtual     bool						Set_Current_BitSource(   short   bitsourceCode,    CString&  retErrorMesg   );


	virtual     BitSource*          Get_Current_BitSource()                        {  return  m_currentBitSource;   }


	virtual    BitSource*           Get_BitSource(                short   bitsourceCode   );
	virtual    bool			           Get_BitSources_FilePath(   short   bitsourceCode,    CString&   retFilePath,    CString&   retErrorMesg   );



   virtual    void					  Release_All_BitSources_Files();    

   virtual    void					  Delete_BitSources();     //  *** OMIT, not used cause the List has garbage-collection  5/07



   virtual    bool					  Alloc_BitSource(    short  bitSourceKindCode,    BitSource  **retBitSource,   CString&  retErrorMesg   ) =0;  


   virtual    bool                    Is_Separation_Updated(     short   bitsourceCode  );                                   
   virtual    void					  Set_Separation_Updated(   short   bitsourceCode,    bool  isUpdated );   



	
public:
	ListMemry< BitSource >     m_bitSources;        //  want pointer or real thing ????


	BitSource        *m_currentBitSource;
};





////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif   // !defined(DETECTORaBSTRACTS_H__INCLUDED_)
