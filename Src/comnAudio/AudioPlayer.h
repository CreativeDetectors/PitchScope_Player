/////////////////////////////////////////////////////////////////////////////
//
//  AudioPlayer.h   -   
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


//////////////////////////////////////////////////////////////////////




class  SndSample;


class  BitSourceAudioMS;
class  BitSourceStreaming;
class  BitSourceStreamingMS;
class  BitSourceStreamingMP3;

//class  BitSourceStatic;
//class  BitSourceStreamingMS;

class  UniApplication;


class  TransformMap;

//  class  CWaveSoundWrite;

class  PlayBufferStatic;

class  SPitchCalc;


class  EventMan;



class  UniApplication;



/////////////////////////////////////////////////////////


#define   SPEEDZONESIZE   4096












////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   AudioPlayer 
{


	//   can also ACCESS from AudioPlayer:    	EventMan&   m_waveMan     &     UniApplication&     m_theApp   


public:
	AudioPlayer(  EventMan&  eventMan,    UniApplication&   theApp   );
	virtual  ~AudioPlayer();



	UniApplication&            Get_UniApplication()    {  return  m_theApp;  }  


	virtual    BitSource&	                    Get_BitSource()  = 0;     //      pulled    1/10



	virtual    BitSourceStreaming*         Get_BitSourceStreaming(   CString&  retErrorMesg   ) =0;	

	virtual    BitSourceStreamingMS*     Get_BitSourceStreamingMS(   CString&  retErrorMesg   ) =0;	



	virtual    HMMIO*	         Get_MediaFile()  = 0;  



	virtual	 SPitchCalc*      Get_SPitchCalc()    {   return NULL;   }      //  dummy default         


	virtual    EventMan&        Get_Waveman()      {  return  m_waveMan;   }




	virtual    bool 				Set_BitSource(   BitSource  *bitSource,     CString&  retErrorMesg    ) =  0;    



	virtual    long		           Get_Biggest_SampleIndex_With_SpeedExpansion();                                  //   has the expansion for SlowedDown    ****  CAREFUL for Overflow *****

	virtual    long				   Get_Biggest_SampleIndex_No_SpeedExpansion();     //  does NOT have the  EXPANSION for SlowedDown 



	virtual    BYTE*			   Get_Waves_DataBits_Start()         {  return  NULL;  }     // dummy default   
	
	virtual    long				   Get_Waves_ByteCount_toEnd()     {  return   0L;  }     // dummy default   



	virtual	void		           Mute_Audio_for_Backwards_Play(  bool  makeQuiet  )   {  m_muteReversePlayAudio =   makeQuiet;   }  



	
	virtual    void      Initialize_for_Playing_Section( );  //  NEW,  3/11



	virtual    bool      StartPlaying(   short  playMode,   double  speed,   bool  backwards,     long  startSample,  
																	long  endSample,    bool  justSpeedChange,    bool  preventFileSeek,    CString&  retErrorMesg  ) =0;
	virtual    void      StopPlaying_Hardware();


	virtual    void		 Fill_Buffer_With_Silence();    //  NEW    2/11





	virtual    bool	     IsOK()              {  return   m_bufferIsOK;  }

	virtual    bool      Is_Playing()	   {   return   m_isPlaying;   } 

	virtual    void      Set_Play_Mode(  short	 playMode  )	     {  m_playMode =    playMode;  }	 

	virtual    bool	     Set_PlaySpeed(   double  speedSlowRatio   );  



	virtual    void      Draw_Last_AnimeFrame()	 {   }    //  dummy for Overides




	virtual    long		    Get_LastPlayed_SampleIdx();

	virtual    long		    Get_Last_SingleNotePlay_SampleIdx();      //  Only for  'Single-Note Play'  BUTTONS


	virtual    void		    Set_LastPlayed_SampleIdx(   long  sampleIdx  );

	virtual    void		    Attach_LastPlayed_SampleIdx(   long   *lastSampleIdxAddr   )  {  m_lastPlayedSampleAddr =   lastSampleIdxAddr;  }








	virtual    WAVEFORMATEX*    Get_Wave_Format()                   {  return  NULL;  }     // dummy default 

//	virtual    MMCKINFO&	        Get_Media_ChunkInfo();

	virtual    bool         ExamineWavFile(  CString&   filePath,     CString&  retErrorMesg   )  =0;       //   like:     SetupStreamBuffer(  LPSTR  lpzFileName );

	virtual    bool		    Initialize_Compatable_Buffer(   CString&   filePath,     CString&  retErrorMesg    )  =  0;




	virtual    void         Load_Next_DataFetch_Forward_PPlayer(      unsigned long  iEvent    )     { ASSERT( 0 );   }    //   dummy default    1/10    
	virtual    void		    Load_Next_DataFetch_Backward_PPlayer(   unsigned long   iEvent   )      { ASSERT( 0 );   }    //   dummy default    1/10   





	virtual    void         Load_Next_DataFetch_Forward(     unsigned long  iEvent   )  =0;      //  Loads the next 'chunk' of sample data from file and plays
	virtual    void		    Load_Next_DataFetch_Backward(  unsigned long   iEvent   ) = 0;




	virtual    long			Calc_Current_Sample_Forward(               unsigned long  iEvent   );
	virtual    long			Calc_Current_Sample_Forward_PPlayer(   unsigned long  iEvent   );

	virtual    long			Calc_Current_Sample_Backward(      unsigned long  iEvent   );
	virtual    long			Calc_Current_Sample_Backward_PPlayer(  unsigned long  iEvent   );

	virtual    long		    Calc_SampleIdx_Forward_PPlayer(  unsigned long  iEvent,   long   dataFetchCount   );   //   *** NEW,   3/11


	

	virtual    bool			Is_Byte_In_ScalepitchSubj(   long  sampleIdx   );

	virtual    void		    Set_Anime_Maps(   TransformMap   **onsetMaskAddrLeft,     TransformMap  **onsetMaskAddrRight,
														                               TransformMap   **onsetMaskAddrCenter     );



	virtual    short        Get_StereoChannelCode();   

	virtual    void			Set_Focus_StereoChannel_Addr(   short  *soundmanVarAddr  );
													





// protected:
	virtual    BOOL    FillBufferWithSilence(   LPDIRECTSOUNDBUFFER   lpDsb   );   //  also see   Fill_Buffer_With_Silence(); 




public:
	PlayBufferStatic       m_shortPlayBuffer;      //   2nd Buffer.   Does  NoteDuration play:    a)  Synthesized    b)  SampleSegment( w/ Note's duration )   c)  Chunk play   d)  Short FFT-filter play


	BitSourceStatic       m_bitSourceOnTheFlySynthed;        //  For OnTheFly play of just synthesized notes


	EventMan&           m_waveMan;

	UniApplication&     m_theApp;   



	CString   m_filePath;



	TransformMap     **m_onsetMaskAddrLeft;      //  the  ADDRESS( in soundman )ScalePitchMask  that is used for animation  
	TransformMap     **m_onsetMaskAddrRight;       
	TransformMap     **m_onsetMaskAddrCenter;    
	

//	bool          *m_detectFromLeftStereoAddr;
	short          *m_stereoChannelCodeAddr;



																				//   1st   DirectSound  'PLAY'   Buffer 
	 
	IDirectSoundBuffer   *m_infcDSoundBuffer;    

	WAVEFORMATEX       *m_pwfx;      //  wave format structure ,  how long do I need this, think it is destroyed after the file is read and SBuffer is created

	MMCKINFO                 m_mmChunkInfoParent; 




	bool           m_bufferIsOK;     //  Since I only do a STREAMING-buffer,  the FILE and Buffer are tied together  ( really ??   1/02  )

	long           m_slowPlayStep;       //  so far fixed at  4096

	short          m_auditionCode;    //  same val in SoundMan




	long		    m_bytesPerSample;   //  4 :   for  16-bit  stereo.  (  2 bytes per 16bit sample  x  2 stereo channels )

	int               m_bytesInFetch;    //     ***ALWAYS Constant   at  44160    Bytes Per Fetch for Buffer [  SoundBuffer::Lock() 

	long		     m_animationSamplesDelay;      //  Force  'out of sync'  animation  to render  'this much in the future'

	DWORD	   m_dwMidBuffer;      //   Number of bytes to lock when in  Load_Next_DataFetch_Forward()



												
																		//   a)    input  'PARMS'    ...for  Play

	short	  m_playMode;   

	bool       m_playingBackward;

	long       m_startSample,   m_endSample;        //  defines the play range  in  16bit samples( samples,  not bytes )

	long	   m_playingCurNotesEndOffset;	


	double	   m_playSpeedFlt;    


	long         m_sampleCountInViewjsWidth;


	bool			m_muteReversePlayAudio;  




	bool      m_wasPlayingBackwardsPrev;   // ****  NEW  3/11     Were we going backwards during the PREVIOUS play-segment ???  



																	    //  b)    Play  'STATE'

	bool	     m_isPlaying;    

	DWORD    m_dataFetchCount;      //  BIG,  used to calc the current READposition in the memory.   To count how many times Load_Next_DataFetch_Forward() was called

	int            m_curSubFetch;	  //   to keep track of Virtual DataFetches  at SlowSpeed,   needed for  Calc_Current_Sample_Forward() 

	DWORD    m_srcBytesProcessed;   //  is set to ZERO whenever we start to Play anything

	bool         m_doneFetchingPlayBytes;   



																		//  c)     PREVIOUS    Play  'POSITION'

	long	     m_prevStartSamplePlay;			   
	long	     m_prevEndSamplePlay;	//   of the last played sound

	long	     m_lastWindowPlayStartSample;	  // *******   NEW,  5/12/2012,   need lots of INITIALIZATIONS to -1    *********************



	bool         m_prevBackwardDirectionPlay;






										//  the 2 below are for  playModes:   {  LASTnOTEpLAYfORWARD,   LASTnOTEpLAYbACKWARD   }

	long         m_lastNotePlaySample;     //  is  ABSOLUTELY-lastPlayed-Sample[  see EventMan::Process_Event_Notification()  ]     

//  **** NOW want to change this if user selects a Note.  For the Next Note buttons.



	long         m_firstNotePlaySample;    //       (  ?? Possibly REPLACE with  'm_prevStartSamplePlay'  ???   2/03    Be careful.  )




protected:	
	long        *m_lastPlayedSampleAddr;       //  resides in  UniBasic object (  ),  must assign this on creation.  




public:
	enum   audioPlayerTypes {   STATICBUFR,     STREAMINGBUFR    };


	enum   playModes {    NORMALpLAY,    RETURNtoPLAYSTART,    LASTnOTEpLAYfORWARD,   LASTnOTEpLAYbACKWARD,   LOOPpLAYsELCT,
	                                                            PLAYwINDOW             };
};









				/////////////////////////////////////////////////////////////////////////////////////////
				////////////////////////     SUBclasses of   'AudioPlayer'    //////////////////////
				/////////////////////////////////////////////////////////////////////////////////////////


class     StreamingAudioplayer         :   public   AudioPlayer
{

public:
	StreamingAudioplayer(    EventMan&  eventMan,   BitSourceStreaming   *bitSource,    UniApplication&  theApp,     bool  doRealtimePitchDetection     );    
	virtual ~StreamingAudioplayer();



//	virtual    EventMan&               Get_Waveman()         {  return  m_waveMan;   }   ***  YES,   CAN also CALL this,  it is declared in   AudioPlayer  parent class   2/12

//	virtual    UniApplication&        Get_UniApplication()    {  return  m_theApp;  }   ***  YES,   CAN also CALL this,  it is declared in   AudioPlayer  parent class   2/12

	virtual	 SPitchCalc*              Get_SPitchCalc();     



	virtual	 BitSourceStreaming*      Get_BitSources_Pointer()     {   return  m_bitSource;  }

	virtual    BitSourceStreaming*      Get_BitSourceStreaming(   CString&  retErrorMesg    );

	virtual    BitSource&	                  Get_BitSource()       {   ASSERT( m_bitSource );     return  *m_bitSource;   }




	virtual    BitSourceStreamingMS*       Get_BitSourceStreamingMS(   CString&  retErrorMesg   );	

	virtual    bool 			  Set_BitSource(   BitSource  *bitSource,     CString&  retErrorMesg    );



//	virtual    long		         Get_Biggest_SampleIndex_With_SpeedExpansion();     //  an overide,  has the necessary SLOWDOWN expansion factor for 

	virtual    long				Get_SndSamples_Valid_Count(   long  slowSpeed   );




	virtual    HMMIO*	    Get_MediaFile();     //  do NOT think this will get called



	virtual    bool      Initialize_Compatable_Buffer(   CString&   filePath,     CString&  retErrorMesg   );

	virtual    bool      ExamineWavFile(                     CString&   filePath,     CString&  retErrorMesg   );    



	virtual    bool      StartPlaying(   short  playMode,   double  speed,   bool  backwards,    long  startSample,  
																		long  endSample,     bool  justSpeedChange,    bool  preventFileSeek,     CString&  retErrorMesg   );

	virtual    void      Draw_Last_AnimeFrame();  

	
	virtual    void		 Fill_Buffer_With_Silence();    //  NEW    2/11    ...an overide to also clear out the delay buffer   


	virtual    bool		 Apply_Volume_Adjust_PPlayer(  long  byteCount,   long  volumeInPercent,    BYTE *srcByte,   BYTE *dstByte,   
																						 bool  modifyStereoBalance,     CString&  retErrorMesg   );


	virtual    long		 Get_Waves_ByteCount_toEnd();    //  think this is supposed to be in OutputBytes.    Trouble for PitchScope ???



	virtual    void		 Load_Next_DataFetch_Forward_PPlayer(    unsigned long   iEvent    );     //  CURRENT   3/2012
	virtual    void		 Load_Next_DataFetch_Backward_PPlayer(   unsigned long   iEvent    );





				///////////////////////////////////////////////////////////////////////////////////////
	virtual    void      Load_Next_DataFetch_Forward_PPlayer_NoAudio(     unsigned long   iEvent   );        //    ***NOT USED ????
	virtual    void      Load_Next_DataFetch_Backward_PPlayer_NoAudio(   unsigned long   iEvent   );       //    ***NOT USED ????

	virtual    void      Load_Next_DataFetch_Forward_PPlayer_OMIT(            unsigned long  iEvent    );    //    ***NOT USED ????
	virtual    void		 Load_Next_DataFetch_Backward_PPlayer_OMIT(            unsigned long   iEvent   );   //    ***NOT USED ????
				///////////////////////////////////////////////////////////////////////////////////////




	virtual    void      Load_Next_DataFetch_Forward(                  unsigned long  iEvent    );              //   all these for   PitchScope 2007
	virtual    void		 Load_Next_DataFetch_Forward_RegularSpeed(                unsigned long   iEvent    );
	virtual    void		 Load_Next_DataFetch_Forward_SlowSpeed(        unsigned long   iEvent    );

	virtual    void		 Load_Next_DataFetch_Backward(   unsigned long   iEvent   );
	virtual    void		 Load_Next_DataFetch_Backward_RegularSpeed(   unsigned long   iEvent   );
	virtual    void		 Load_Next_DataFetch_Backward_SlowSpeed(   unsigned long   iEvent   );






public:
	BitSourceStreaming     *m_bitSource;  


	bool			m_doRealtimePitchDetection;   //  ********  KEEP IT.      Not used in Navigator or Player, but IS USED in PitchScope   12/2011

	short   		m_rightChannelPercent;    //  In Realtime Pitch Detection, this describes the ratio of left/right signals to analyize
															    //  It can also afect the AUDIBLE sound as well, dependant on how I set the switch   3/11


	BYTE       *m_speedZoneBuffer;

	long         m_speedZoneFetchCount;       //   in   4096   blocks    ******* DEBUG ONLY  *******

	
	BYTE	      *m_backwardsBits;   
	long          m_backwardsBitsSize;  
};







						/////////////////////////////////////////////////////////////////////////////////////////
						/////////////////////////////////////////////////////////////////////////////////////////


class     AudioStaticPlayer       :  public  AudioPlayer
{


public:
	AudioStaticPlayer(  EventMan&  eventMan,   BitSourceStatic  *bitSource,     UniApplication&  theApp     );
	virtual ~AudioStaticPlayer();



//	virtual    BitSourceAudioMS&	  Get_BitSource()                         {   ASSERT( m_bitSource );     return  *m_bitSource;   }
	virtual    BitSource&	              Get_BitSource()                         {   ASSERT( m_bitSource );     return  *m_bitSource;   }




	virtual    bool 				Set_BitSource(   BitSource  *bitSource,     CString&  retErrorMesg    );


	virtual    HMMIO*	         Get_MediaFile();



	virtual    bool      Initialize_Compatable_Buffer(   CString&   filePath,     CString&  retErrorMesg   );


	virtual    bool      ExamineWavFile(  CString&   filePath,     CString&  retErrorMesg  );   //  An override,  so can close the file




	virtual    bool      StartPlaying(   short  playMode,   double  speed,   bool  backwards,     long  startSample, 
																		long  endSample,     bool  justSpeedChange,    bool  preventFileSeek,     CString&  retErrorMesg   );





	virtual    void      Load_Next_DataFetch_Forward(     unsigned long  iEvent    );      //  Loads the next 'chunk' of sample data from file and plays
	virtual    void		 Load_Next_DataFetch_Backward(  unsigned long   iEvent   );



	virtual    WAVEFORMATEX*    Get_Wave_Format();	


	virtual    BYTE*       Get_Waves_DataBits_Start();      //  these 2 are modified to allow partial play
	virtual    long			 Get_Waves_ByteCount_toEnd();




public:
	BitSourceStatic    *m_bitSource;         //   just a pointer, does NOT reside here
};


