/////////////////////////////////////////////////////////////////////////////
//
//  BitSourceAudio.h   -   
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


////////////////////////////////////////////////////////////////////////////////




//  #define  TWOSECBYTES  352800    //                           2 sec  [  was    "m_pwfx->nAvgBytesPerSec * 2"  ]    in  SetupStreamBuffer


#define  TWOSECBYTES  88320L          //  NOW is really    1/2  sec       Fetches  sample data every 1/4 sec ( 2x for every rotation of the CircularBuffer )

//#define  TWOSECBYTES  (88320L  * 2L)     //  BAD:   THINK that I MIGHT loose time resolution when this is increased. Maybe because 'eventsOffset' gets bigger, not smaller.  1/10   Does not sound as accurate. 
//#define  TWOSECBYTES  44160L                //  WORKS:    to give more resolution... but do not need that much resolution


//    88320 / ( 2 bytesPerSamp * 2 stereoChannels )  =    22,080  samples     (  WAV is 44100  samples/sec )



#define  BLOCKLOCKEDSIZE    ( TWOSECBYTES / 2L )      //    44160    This is HALF of TWOSECBYTES, becase there are 2 data fetches per revolution of the PlayBuffer




		//   { kSoundDelayFetchCount  and  kSizeOfFinalNotesCircque1xSlow }  work together. I change kSoundDelayFetchCount then must make big changes 
		//             to  kSizeOfFinalNotesCircque1xSlow  ( it does the fine-tuning of delay,  kSoundDelayFetchCount  does the major changes in delay.   3/2011

#define   kSoundDelayFetchCount  14   //   This delays the sound.  Can adjust it, but would have to recalc all delays.   3/2011      [ see  StreamingAudioplayer::StreamingAudioplayer()
													      //      ****  Keep this FIXED  at  14    3/2011  ***********************



		/////////////////////////////////////////////////////////


class   TransformMap;

class   SndSample;

class    BitSource;
class    BitSourceStreamingMP3;
class    BitSourceStreamingWAV;    //  NEW,  2/10
class    BitSourceAudioMS;

class   FileUni;
class   WavConvert;

class   SPitchCalc;

class   CalcedNote;
class   MidiNote;

class   NoteGenerator;




////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   BitSourceStreaming  :    public   BitSource  
{                   

				//   will be an abstract PARENT  to    BitSourceStreamingMP3,     BitSourceStreamingWAV ( these 2 used by Player and Navigator )
				//
				//    AND also  BitSourceStreamingMS,   but  BitSourceStreamingMS   is NOT used by Player, only PitchScope    12/11

public:
	BitSourceStreaming();    //  
	virtual ~BitSourceStreaming();




	virtual     void		Initialize_For_Playing();                       //  NEW,  8/21/2012   Initializes  WavConvert  and   SPitchCalc


	virtual     void		Initialze_For_File_Position_Change();    //  NEW,  2/8/2012  




	virtual	  long		Calc_Files_Total_Output_Bytes_With_SpeedExpansion();         //  does  HAVE          slowDown expansion (    forward value  from   WavConvert   )  

	virtual     long		Get_Files_Total_Samples_No_SpeedExpansion();                    //  does  NOT have   slowDown expansion



	virtual     long		Get_Load_Blocks_Size()      {   return   BLOCKLOCKEDSIZE;    }    


	virtual     long		Get_Biggest_SndSample_Index();   //  NEW,   1/2012



	virtual     void		Set_MidiSource_UsingNotelist_Variables(     bool  usingNotelist   );    //  also sets a variable in   { WavConvert  and   SPitchCalc }

	virtual     void		Set_Computer_Performance_Factor(   long  computerPerformanceFactor    );   //   works with   WavConvert  







	virtual     void	    Increment_Pie_Clock();




	virtual     bool		Initialize(   long  chunkSize,   CString&  filePath,    bool&  fileAcessError,    long  numberNeededToMatch,   
		                                                         long  *noteListWeirdDelayAddr,   bool  usingCircQueDFTprobes,    CString&  retErrorMesg  );  //  for both  MP3 and WAV bitsource subclasses

	

	virtual     bool		            Allocate_SPitchCalc(    long  numberOfWavemanEvents,    long  byteCountTwoSeconds,    long  numSamplesInBlockLocked, 
																			ListDoubLinkMemry< MidiNote >  *noteList ,   long  *noteListWeirdDelayAddr,	   bool  usingCircQueDFTprobes,   CString&  retErrorMesg   );

	virtual     NoteGenerator*	Allocate_NoteGenerator(    short   filesVersionNumber,     CString&  retErrorMesg   );






	virtual     bool		Allocate_WAV_Circular_Que(  short  appCode,    CString&  retErrorMesg   );

	virtual    BYTE*		Get_DelayBuffer_Read_Byte(   long  iEvent,    long  dataFetchCount    );     //  for the new  Delay Buffer     [  m_soundDelayBuffer  ]

	virtual    BYTE*		Get_DelayBuffer_Write_Byte(                          long  iEvent,    long  dataFetchCount    );
	virtual    BYTE*		Get_DelayBuffer_Write_Byte_BackwardsPlay(   long  iEvent,   long  dataFetchCount   );

	virtual    void			Erase_DelayBuffer();

	virtual    long			Calc_WAV_DelayBuffers_Ideal_Delay_In_Samples(   double  playSpeed,    long  bytesPerSample   );



				//   And another smaller  WAVdelay for Player

	virtual    void			Erase_Players_AudioDelay_MemoryBuffer(); 

	virtual  	 bool			ReAllocate_Players_AudioDelay_MemoryBuffer(    long  numberNeededToMatch   );



													//    also   FFTSlowDown     object ( inside of WavConvert  )  

	virtual  	 void			Erase_FFTSlowDowns_Buffers();





	virtual    void			 Adjust_Volume_on_CopyBytes(  	char  chVal0,    char chVal1,   char chVal2,    char chVal3,   BYTE  *destBufferBytes,  
		                                                                                                   UINT  byteIdx,     short  rightChannelPercent      );  

	virtual     void		 Clean_All_Resources()    {  ASSERT( 0 );    }    //  Call this when I want to put BitSource to sleep ( when it is UN-Initialized, waiting for a new file )  12/11


	virtual     void		 Release_All_Resources();   

	virtual     void         Release_WaveFormatEx();

	virtual     void		 Release_Media_File();

	virtual     void	     Release_WAV_CircularQue();




	virtual     bool		 Move_To_DataStart(   CString&   retErrorMesg   )  = 0;  

	virtual     bool		 Seek_New_DataPosition_Fast(   long    offsetFromDataStart,   short directionCode,     CString&   retErrorMesg   )  =0;




	virtual     bool	     Fetch_PieSlice_Samples(   long  samplesToLoad,    BYTE  *destBuffersPieSlicePtr,    bool&  retHitEOF,   //  NEWEST,  prefered  3/2012  
												                                    bool  fowardFlag,    short  rightChannelPercent,   long  startOffsetByteBackwards,	  CString&   retErrorMesg   );



	
	virtual     bool        Fetch_Streaming_Samples_Direct_PPlayer(     long       startOffsettedByte,      // OLD,  maybe obsolete  3/2012
																									UINT      byteCountToRead,    
																									BYTE    *destBufferBytes,  
																									UINT     *retActualBytesRead,  
																			 						long       srcBytesProcessed,																				
																									bool&       retHitEOF,   
																									bool		  fowardFlag, 
																									short       auditionCode,  
																									bool        soundManControlsVolume,  
																									short        rightChannelPercent, 
																									double      slowDownSpeed,
																									CString&      retErrorMesg   );     


	virtual     bool         Fetch_Streaming_Samples_Direct(     long       startOffsettedByte,     //    **** DUMMY *****   function to keep compiler happy
																						UINT      byteCountToRead,    
																						BYTE    *destBufferBytes,  
																						UINT     *retActualBytesRead,  
																			 			long       srcBytesProcessed,																				
																						bool&       retHitEOF,   
																						bool		  fowardFlag,  	
																						short       auditionCode,    
																						bool   soundManControlsVolume,  
																						CString&      retErrorMesg   )  
									{   ASSERT( 0 );     
										return  false;   }







public:
	SPitchCalc        *m_sPitchCalc;            //   resides here

	WavConvert     *m_wavConvert;          //   resides here

	NoteGenerator   *m_noteGenerator;     //   resides here




	long    m_sampleIdxLastBlockLoadNotSlowExpanded;   //  In OUTPUT BYTES Format [44hz 16bit], even for ReSampled.   
																				    //  This var to enable simpler, accurate CALC of  "curSample"  in Process_Event_Notification_PPlayer()


	long    m_currentPieClockVal;       //   For each  'BlockLoad'  there will be  10 Events (slices of pie)  in the  BLOCK (the pie). This new var to also enable a 
	                                                 //    simpler CALC of   "curSample"   in Process_Event_Notification_PPlayer.  Works with  'm_sampleIdxLastBlockLoadNotSlowExpanded' 




	BYTE   *m_pieSlicesBuffer;                //   BUFFER    44,160   

	long      m_byteIndexToBufferPieSlices;

	long      m_sizeOfPieSlicesBuffer;   //  use it  for BlockSize.   Can vary the size of the Blocks that are loaded...  see  Make_NoteList_No_Audio().   2/12





//	BYTE      *m_backupInputBuffer;      //   for NEW slowDown algo   and built in Resampling


	long	 	   m_chunkSize;     //   **** HARDWIRED*****   

	long         m_bytesPerSample;     //  4,  for 16bit Stereo    **** HARDWIRED*****   


	long         m_sampleRate;   // ***INSTALL,  should be read from file ****

	long         m_numberOfWavemanEvents; 



	long	     *m_wavVolumeSoundmanAddr;         //  the  ADDRESS( in PitchPlayerApp ),  use one for all 

	long	     *m_midiVolumeSoundmanAddr; 



	WAVEFORMATEX   *m_wavFormat; 



	long		m_startPlaySampleIdx,     m_endPlaySampleIdx;      //  NEW,  are set by   StreamingAudioplayer::StartPlaying()


											//   Navigator's   new NOTELIST creation  functions     3/11

	bool		m_recordNotesNow;



	long	   m_inputScalePercent;  

	long     m_topFreqFilterLimit;
	long     m_bottomFreqFilterLimit;   




//	m_inputScalePercent  =    500;     // this will REALLY get INITIALIZED on OpenFile   by  EventMan::Set_VolumeBoost_Factor(  short   factorCode   )


														 //  a  CircularQue  of  WAV-Buffers  to give Navigator a delay in sound.
	BYTE       *m_soundDelayBuffer;  
	long          m_soundDelayBufferCount;       //   in bytes  ( 4bytes per 16bit sample )
	long          m_soundDelayFetchCount;       //    44,160  bytes in a fetch
	long	        m_soundDelayFetchIndex;      
};






			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////


class    BitSourceStreamingWAV    :   public   BitSourceStreaming             //  used by  NAVIGATOR  and  PLAYER
{							   

public:
	BitSourceStreamingWAV();
	virtual   ~BitSourceStreamingWAV();


	virtual     bool		 Move_To_DataStart(   CString&   retErrorMesg   );


	virtual     bool         Seek_New_DataPosition_Fast(    long   offsetFromDataStart,   short directionCode,    CString&   retErrorMesg    );   //  BUGGY ????


	virtual     void		 Clean_All_Resources();   //   NEW,   11/11

public:

};



			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////


class    BitSourceStreamingMP3    :   public   BitSourceStreaming        //   used by  NAVIGATOR  and  PLAYER
{							   
														
public:
	BitSourceStreamingMP3();
	virtual   ~BitSourceStreamingMP3();


	virtual     bool		Move_To_DataStart(   CString&   retErrorMesg   );

	virtual     bool		Seek_New_DataPosition_Fast(    long  offsetFromDataStart,     short directionCode,       CString&   retErrorMesg   );


	virtual     void		Clean_All_Resources();    //  belongs here 


public:

};






			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////



class    BitSourceStreamingMS    :   public   BitSourceStreaming     //       BitSourceAudioMS      BitSourceStreaming                       was  ,      now must get along with new MP3  reader
{			

			//  **************   This is used by PitchScope,    BUT is NOT used by Navigator or Player   12/2011  ****************

public:
	BitSourceStreamingMS();
	virtual   ~BitSourceStreamingMS();



	virtual     long		 Calc_Files_Total_Output_Bytes_With_SpeedExpansion();     

	virtual     long		 Get_Files_Total_Samples_No_SpeedExpansion();    //  does NOT have slowDown expansion



	virtual     int		     Number_Bytes();   
	virtual     int			 Number_Samples();   



	virtual     void		 Release_BufferBits();


	virtual     bool		 Alloc_LocalBuffer(   long  bufferSize  );

	virtual     void         Release_All_Resources();





	virtual     bool		 Initialize(   long  chunkSize,    CString&  filePath,     bool&  fileAcessError,    CString&  retErrorMesg  );   //  WAS:  	Register_WavFile_for_Streaming()




	virtual     SndSample*     Create_8bit_Sample_Segment(   long  startOffset,   long  endOffset,    short  channelCode,    CString&  retErrorMesg  );

	virtual     bool		        Copy_To_8bit_Sample_Segment(   long  startOffset,    long  numberOfSamples,       short  channelCode,    
															                                                       SndSample&  nuSample,   CString&  retErrorMesg  );



	virtual     bool		Fetch_Chunks_Samples_for_StreamingAnalysis(    bool&  retHitEOF,    CString&  retErrorMesg   );   //  called by  logDFTBankVerter::Load_Samples_Bits()




	virtual     bool         Fetch_Streaming_Samples_Direct(     long       startOffsettedByte,  
																						UINT      byteCountToRead,    
																						BYTE    *destBufferBytes,  
																						UINT     *retActualBytesRead,  
																			 			long       srcBytesProcessed,																				
																						bool&       retHitEOF,   
																						bool		  fowardFlag,  	
																						short       auditionCode,    
																						bool   soundManControlsVolume,  
																						CString&      retErrorMesg   );


	
	virtual     bool         Fetch_Streaming_Samples_Direct_PPlayer(     long       startOffsettedByte,  
																									UINT      byteCountToRead,    
																									BYTE    *destBufferBytes,  
																									UINT     *retActualBytesRead,  
																			 						long       srcBytesProcessed,																				
																									bool&       retHitEOF,   
																									bool		  fowardFlag, 

																									short       auditionCode,  
																									bool        soundManControlsVolume,  

																								    SndSample*    sndSample,
																									short        rightChannelPercent, 

																									CString&      retErrorMesg   );





	virtual     bool         Seek_New_DataPosition(            long   offsetFromDataStart,     CString&   retErrorMesg   );   //  NOT used 

	virtual     bool         Seek_New_DataPosition_Fast(    long   offsetFromDataStart,   short directionCode,    CString&   retErrorMesg    );   //  BUGGY ????




	virtual     void		Release_Media_File();

	virtual     bool		Open_Get_WavFormatEx(   long  chunkSize,    CString&   filePath,     CString&   retErrorMesg   );

	virtual     bool		Move_To_DataStart(  CString&   retErrorMesg   );





public:
	char	 *m_readBuffersBits;         	//   used for the  SpeedZone buffer  (  4096   bytes )  

	char	 *m_readBuffersBitsRight;   //  used as an odd buffer [  see   BitSourceStreamingMS::Create_8bit_Sample_Segment( 


	long     m_bufferSize;               //  how many bites are in    'm_readBuffersBits'



    long		   m_totalBytes;       //     is not used by all subClasses  ******       total Bytes,   NOT samples.   For  STREAMING,  this is the ENTIRE samples size in file


	long     m_reduxInVolumeWhenPlaying;   //  in percent.  Because the WAV is so much louder thatn the Midi, we reduce bu this amount when the sample is played.  6/07



	bool	     m_usingSecondaryBuffer;     //  *******  UGLY,    change this to SUBclass


	long		 m_filesDataStart;     //  NEW,  for streaming file navigation    ...but  NOT yet   USED !!!    10/02



	HMMIO          m_mmIO;   	             //    handle to an  OPEN .wav  FILE 
    MMIOINFO     m_mmIOinfo;             //    'INFO'         current STATUS    of  <mmIO>    ...used during  MemoryBlock fetches

	MMCKINFO    m_mmChunkInfo;       //    'CHUNK'      contains information about a  'CHUNK'  in a  RIFF file
	MMCKINFO    m_chunkInfoParent;    //    important to have this data PERSIST between Play stops
};





////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   BitSourceAudioMS      :    public   BitSource
{
														//  used by  PitchScope
public:
	BitSourceAudioMS();
	virtual   ~BitSourceAudioMS();


	virtual     int			  Number_Samples();                // ***** FIX:  so that it gets these values from the WAV file


	virtual     int			  Number_Bytes()                          {  return  m_totalBytes;  } 
	virtual	  long		  Calc_Files_Total_Output_Bytes_With_SpeedExpansion()     {  return  m_totalBytes;  }      

	virtual     long		 Get_Files_Total_Samples_No_SpeedExpansion()  {  return    Number_Samples();   }




	virtual     void         Release_All_Resources();

	virtual     bool		 Initialize(   long  chunkSize,    CString&   filePath,     CString&   retErrorMesg   )   {   ASSERT( 0 );  return  false; }

	static      long          Bytes_Per_Sample()     {   return  4;  }   //  HARDWIRED for 16 bit stereo WAV files 



	
	virtual     WAVEFORMATEX*    Alloc_New_WAVEFORMATEX();		
	virtual     void				        Release_WaveFormatEx();

	virtual     void                        Release_Media_File();




	virtual     bool		 Open_Get_WavFormatEx(   long  chunkSize,    CString&   filePath,     CString&   retErrorMesg   );  

	virtual     bool		 Move_To_DataStart(   CString&   retErrorMesg   );    // **** ENHANCE??? *****



	
	virtual     WAVEFORMATEX*    Get_Wav_Format_Struct()     {   return   m_wavFormat;  }  






public:
//	long	     m_currentByteIdx;      //  more to upper level


	WAVEFORMATEX   *m_wavFormat; 


	bool	     m_usingSecondaryBuffer;     //  *******  UGLY,    change this to SUBclass



	long	 	    m_chunkSize;     //   **** HARDWIRED*****   

	long         m_bytesPerSample;   //  4,  for 16bit Stereo    **** HARDWIRED*****   


	long         m_sampleRate;   // ***INSTALL,  should be read from file ****

	int			   m_totalBytes;       //   total Bytes,   NOT samples.   For  STREAMING,  this is the ENTIRE samples size in file





	long		 m_filesDataStart;     //  NEW,  for streaming file navigation    ...but  NOT yet   USED !!!    10/02




	HMMIO          m_mmIO;   	             //    handle to an  OPEN .wav  FILE 
    MMIOINFO     m_mmIOinfo;             //    'INFO'         current STATUS    of  <mmIO>    ...used during  MemoryBlock fetches

	MMCKINFO    m_mmChunkInfo;       //    'CHUNK'      contains information about a  'CHUNK'  in a  RIFF file
	MMCKINFO    m_chunkInfoParent;    //    important to have this data PERSIST between Play stops



	long	*m_wavVolumeSoundmanAddr;         //  the  ADDRESS( in soundman ),  use one for all 
};



							/////////////////////////////////////////////////////////////////////////////
							/////////////////////////////////////////////////////////////////////////////


class    BitSourceStatic	     :   public   BitSourceAudioMS			//  NEED this to play the Synthesized FFT on fly functions 5/07  [  ReConstruct_onFly_FFT_Pitched_Segment_2BitSource ]
{																					// But do NOT need one in Soundman.  

public:
	BitSourceStatic();
	virtual   ~BitSourceStatic();




	virtual     SndSample*     Create_8bit_Sample_DynamicAlloc(    short  channelCode,   CString&  retErrorMesg   );


	virtual     bool                 Copy_From_Transform_Samples(   SndSample&  srcSndSample,    short  channelCode,   CString&  retErrorMesg   );

	virtual     bool                 Copy_To_Transform_Samples(       SndSample&  dstSndSample,    short  channelCode,   CString&  retErrorMesg   );   //  not used  5/07






	virtual     bool		 Initialize(   long  chunkSize,    CString&  filePath,    CString&  retErrorMesg  );  // WAS:   Load_Wav_File() 







	virtual     char*		Get_Start_Byte()              {   return   m_staticBits;     }


	virtual     char*		Alloc_Static_Block(   long  numBytes   );
	virtual     void		 Release_Static_Block();   


	virtual     bool		 Is_Empty()						 {   if( m_staticBits )   return true;       else   return false;   }


	virtual     bool			 Create_Secondary_Buffer(   long   byteCount   );   //  when playing SYNTHESIZED notes
	virtual     bool			 Erase_SecondaryBuffer();




public:
	char	   *m_staticBits;   


};




			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////


class   SourceAdminAudioMS     :  public   SourceAdmin
{

public:
	SourceAdminAudioMS();
	virtual ~SourceAdminAudioMS();


   //virtual    BitSource*          Get_Current_BitSource()                        {  return  m_currentBitSource;   }
   //virtual    void					Delete_BitSources(  /*   bool   justSetMapPointers */    );    

   virtual		bool					Alloc_BitSource(    short  bitSourceKindCode,     BitSource  **retBitSource,   CString&  retErrorMesg   );  

	
public:

};


