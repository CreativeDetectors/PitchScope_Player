/////////////////////////////////////////////////////////////////////////////
//
//  WavConvert.h   -   
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_WAVCONVERT_H__C38AB9E0_137C_4991_8350_59D8811E55B6__INCLUDED_)
#define AFX_WAVCONVERT_H__C38AB9E0_137C_4991_8350_59D8811E55B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



class  FileUni;
class  Mp3Reader;
class  ReSampler;
class  FFTslowDown;

class  SndSample;


#define  DEFAULTsAMPLINGrATEmp3   44100


#define  MP3rEADERbYTESpERsAMPLE  4     //   stereo 16 bit samples

#define  STREAMINGmp3BUFFERSIZE    4416      //  [ 4416 x 10reads =  44160  my DirectSound readSize   ]         4608    can adjust,   jst allow it to be divisable by 4    1/27/10



#define  ReSAMPLEbUFFERsIZE   (  STREAMINGmp3BUFFERSIZE  /  MP3rEADERbYTESpERsAMPLE  )     //   careful,  in SAMPLES, not bytes




////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


class   WavConvert  
{

public:
	WavConvert();
	virtual ~WavConvert();



	void		   Initialize_for_Play();                           // ****  NEW,   allow anything to get initialized 

	void		   Initialze_For_File_Position_Change();    //  NEW,    9/2012




	void			Set_Computer_Performance_Values(   long  computerPerformanceFactor    );


	long			Calc_Samples_In_PieEvent(  double  playSpeed  );
	



	long		    Calc_Files_Total_Output_Bytes_no_SpeedExpansion();

	long			Calc_Files_Total_Output_Bytes_With_SpeedExpansion();  



	long			Calc_Files_Total_Source_Samples();    //   *******************  NEW,  8/2012   ***********************************





																	//   allows a  SMALL DELAY in Audio sound,  but only for Player at Speed1.     7/2012

	long          Get_Base_AudioDelay_inPieSlices();         //   in   
	long          Get_Overide_AudioDelay_inPieSlices(); 


	bool			ReAllocate_Players_AudioDelay_MemBuffer(   long  sampleCountNotSlowDown,    long  numberNeededToMatch   );

	void			Erase_Players_AudioDelay_MemBuffer();


	char*		Get_Oldest_Block_from_AudioDelays_CircularQue();		
	char*		Get_Newest_Block_from_AudioDelays_CircularQue();






	bool			Is_A_StreamingFile_Loaded();

	bool			Is_A_MP3_File_Loaded();




	bool          Does_FileName_Have_Extension(   char* fileExtensionPtr,    CString&   filePath   );





	bool			Change_PlaySpeed(    double  newSlowDownRatio,      long  computerPerformanceFactor,     long  numberNeededToMatch,     CString&   retErrorMesg   );




	bool			Initialize_for_Streaming(   bool  isMP3file,     long  numberNeededToMatch,   CString&   retErrorMesg  );
	void			Cleanup_After_Streaming();


	bool			Initialize_for_WAV_Streaming(    CString&   filePath,    bool&  fileAcessError,     CString&   retErrorMesg   );




	bool		    Alloc_OutputBuffer(   long   sampleCountNotSlowDown,    long  numberNeededToMatch,     CString&   retErrorMesg  );


	bool		    Alloc_ReSampling_Arrays(   long  numberOfSamples,   CString&  retErrorMesg   );
	void			Release_ReSampling_Arrays();

 



	bool	        Fetch_Current_Sample_and_Increment(     bool&   retEndOfFile,    long  currentByteIdx,    
														                                        char&  retChVal0,     char&  retChVal1,    char&  retChVal2,     char&  retChVal3,           
									                                           bool  backwardsFlag,	 SndSample  *sndSample,	long  sampIdxBlock,	short  rightChannelPercent,	
													                                             bool&  retDidBlockLoad,    bool  isPlayingNoteList,     CString&   retErrorMesg  );																															
																		


	bool		    Load_Next_MemoryBlock_Frame(   bool&  retEndOfFile,      long&  retBytesRead,    bool  backwardsFlag,    CString&  retErrorMesg   );   //  fills the output buffer

	bool		    Load_Next_MP3_Frame(    bool&   retEndOfFile,      long&  retBytesRead,    long& retOutputByteCount,    CString&   retErrorMesg   );
	bool			Load_Next_WAV_Frame(   bool&   retEndOfFile,      long&  retBytesRead,   long& retOutputByteCount,     CString&   retErrorMesg   );



	bool			Load_Ten_MemoryBlock_Frames(   bool&   retEndOfFile,    long&  retBytesRead,    long  byteCountToRead,    CString&   retErrorMesg   );



	bool			Fetch_Blocks_Bytes(   bool&   retEndOfFile,    long&  retBytesRead,     long  currentByteIdx,   CString&   retErrorMesg   );  // **** OMIT ******





	long		    Get_Current_FilePos_ReadersMemberVar_UNcompressed_Output_Bytes();

	long			Get_Current_FilePos_In_UNcompressed_Output_Bytes();

	long			Get_Current_SampleIndex_ReSamplerExpanded();




	bool			Seek_to_BytePosition_in_OutputCoords_for_StreamingMP3(  long   byteIdx,   long&  retNewFilePos,    CString& retErrorMesg  );

	bool			Seek_to_Frame_Position_for_Streaming(   long   frameIdx,    CString&   retErrorMesg  );        //  only used in test functions


	void		    Set_MP3reader_Timing_Correction(  long  newValue   );   //  helps make more accurate SEEK  for   NoteRecording 


											//   ABOVE  are for Streaming mp3  Reads   2/10




	short			Get_Compatability_Code_for_Music_File(    CString&  srcWavFilePath,     CString&   retErrorMesg  );


	bool			Create_VoxSep_Compatable_WAV_File(    CString&  srcFilePath,     CString&  retNewFileName,  
																										           CString&   retErrorMesg   );    




																		//   MP3  to  WAV

	void          Set_SRC_Sound_File(  CString&  srcWavFilePath,   bool&  retIsAmp3File    );  // This will choose to call EITHER  Set_SRC_MP3_File()  or   Set_SRC_WAV_File() 


	void			Set_SRC_MP3_File(  CString&  srcMP3FilePath  )       {  m_srcMP3FilePath =   srcMP3FilePath;  }

	void			Set_SRC_WAV_File(  CString&  srcWavFilePath  )      {  m_srcWavFilePath =   srcWavFilePath;  }





	bool			Get_MP3_Header_Info(   int&  layer,   int&  lsf,   int&  freq,  int&  stereo,   int&  rate,  CString&   retErrorMesg  );

	bool			Convert_MP3_to_WAV(    CString&   retNewFileName,   CString&   retErrorMesg   );    // soup to nuts





																		//   WAV  to  WAV   ( different formats ... dest is  44.1 at 16bit  )


	static			bool		Get_WAV_Header_Info(  CString&   wavFilePath,
																	unsigned short&    retFormatTypeWAVsrc,          
																	short&                  retChannelsCountWAVsrc,     
																	short&                  retBitsPerSampleWAVsrc,      
																	long&                    retSampleRateWAVsrc,        
																	long&           retTotalSamplesBytesWAVsrc,																										
										                            CString&      retErrorMesg  );



	void          Release_WaveFormatEx();
	void			Release_Media_File();

	bool			Open_Get_WavFormatEx(   CString&   filePath,    CString&   retErrorMesg   );



	bool			Convert_WAV_to_WAV(   CString&   retNewFileName,   CString&   retErrorMesg   );     // soup to nuts


	bool			Create_Test_WAV_File(    CString&  filePath,    CString&  retErrorMesg   );





private:
	bool			Get_MP3_Version_Set_Offset(    FileUni&  mp3File,    CString&   retErrorMesg   );


													//  ReSample functions

	bool			Set_ReSample_Rates(   long  srcSampRate,   long  dstSampRate,      CString&   retErrorMesg   );



													//  WAV file creation

	void		    Write_WAV_Header(   CFile &file,    int rate,   int stereo,   int bit16,    int len   );

	void			Write_WAVfiles_Length(   CFile &file   );


	bool		    Process_Sample_Packet(   unsigned  char  packetBuffer[],     long  packetSize,    long  bufSampleIdx,
															 long  bufferSize,    float  *retSampleArrayLeft,     float  *retSampleArrayRight, 
															 bool&  retProcessBuffer,    CString&  retErrorMesg  );

	bool          Process_Sample_Packet_NoResamp(     unsigned char  packetBuffer[],   long  packetSize,   long  bufSampleIdx,
										                                        long  bufferSize,    unsigned char   *dstSamplesPtr,
																                 bool&  retProcessBuffer,    CString&  retErrorMesg   );



public:
	bool		m_isInitializedStreaming;


	FileUni	          m_fileUniStreaming;  

	Mp3Reader      m_mp3DecoderStreaming;  

	FFTslowDown	  m_fftSlowDown;



	short		  m_playModeUsingNotelist;     // ******   NEW,  keep it UPdated     0:  Detection     1:  Play-Notelist      2:  ????    8/2012   *************  
														     //             [    BitSourceStreaming::Set_MidiSource_UsingNotelist_Variables()     ]   8/2012
	


	ReSampler       m_reSamplerStreaming;     //   This is to ReSample   .WAVs  that might not be at   44160 hz

	bool			m_doResamplingStreaming;

	float		   *m_reSampleArrayLeft,   *m_reSampleArrayRight;    //     the two are intermediate buffers for RE-Sampling              2 intermediate samples



	long      m_totalSourceSamples;    //  NEW,  8/12    Will be different if file Needs resampling.   NOT yet INSYALLED for MP3 



	long      m_totalSamplesBytesWAVsrc;    //  should also assign this for  MP3

	long		m_defaultSamplingFreqOutput;        //     44100      DEFAULTsAMPLINGrATEmp3

	long		m_defaultBytesPerSampleOutput;     //   always 4   ( 16bit stereo )





									//    INPUT data:

	long		m_bytesPerSampleSRCstream;    //    4(16bit)  or  2(8bit)   for stereo,       2  and 1  for mono

	long		m_bitsPerSampleSRCstream;      //    16 or  8  NEW   ***CAREFULL***    is  'BITS',  not 'bytes'

	bool      m_stereoFlagSRCstream;




									//    OUTPUT BUFFER vars:     For STREAMING play,  the Output Buffer contains 16bit-stereo bytes 

	char*    m_outputBufferStreaming;        

	long	    m_outputBufferSizeStreaming;    // in BYTES.     Can VARY in size if the ReSampler is invoked, or if using slowedDown play, or convert monoSRC to stereo  

	long	    m_outputBufferSampleCount;     

	long		m_currentByteIndexInBufferStreaming;   //  SLOWED-Down sample







	char*    m_outputBufferStreamingDelayed;     // ****   NEW,  for Player.exe  SMALL   WAV-Delay  7/2012   ***

	long      m_indexIntoBufferStreamingDelayed;   


	long	    m_outputBufferSizeStreamingDelayed;    // in BYTES.  

	long      m_delayInPieSlices;   


	long		*m_baseDelayAddr;        //   Now in  PitchPlayerApp,  so setting can persist as different songs are loaded.  8/12
	long		*m_overideDelayAddr;    //  Special MODE:   If this has a value  >=0,  then that is the REAL DELAY to use,  and  m_baseDelay is processed as ZERO   





	char*    m_outputBufferStreamingNoSlowDown;      // NEW,  for original data  

	long		m_outputBufferSampleCountNotSlowDown;    //  will  sat   the same after basic Initialization. 

	long		m_currentByteIndexInBufferStreamingNotSlowDown;     //  NEW,  1/2012



	long      m_indexIntoSndSampleNotSlow;    //  NEW,  1/2012

	long		m_biggestIndexToSndSample;    //  *** TEMP,  debug




	BYTE*     m_backwardsPlayBuffer;              //    44160   bytes,  but not always.  12/11 
	long        m_backwardsPlayBufferSize;

	long        m_backwardsPlayBufferBlockIndex;    //  0 - 9      [  10 blocks of  4416 bytes,   but NOT always.  


	long        m_maxNumberOfbackwardsPlayBufferBlocks;    //  usually 10,   but can be less.

	long		  m_numberOfTenBlocksLoaded;    //  sometimes can only load PART of the 10 subBlocks.  This tells us how many





														 //   SOURCE  .WAV-file  properties ( from  WAVEFORMATEX struct )

	unsigned short        m_formatTypeWAVsrc;            //   format type 
	short                      m_channelsCountWAVsrc;       //   number of channels (i.e. mono, stereo...) 
	short                      m_bitsPerSampleWAVsrc;        //   number of bits per sample of mono data 
	long                       m_sampleRateWAVsrc;             //   sample rate 
//	unsigned short        m_blockAlignWAV;              //   block size of data 
//	unsigned short        m_countBytesSizeWAV;       //   the count in bytes of the size of 'extra information' (after cbSize) 
//	unsigned long          m_avgBytesPerSec;    //  for buffer estimation 



	WAVEFORMATEX   *m_wavFormat; 

	HMMIO          m_mmIO;   	             //    handle to an  OPEN .wav  FILE 

	MMCKINFO    m_mmChunkInfo;       //    'CHUNK'      contains information about a  'CHUNK'  in a  RIFF file
	MMCKINFO    m_chunkInfoParent;    //    important to have this data PERSIST between Play stops




private:
	CString    m_srcSoundFilesPath;   // could be either  .WAV  or .MP3

	CString    m_srcMP3FilePath;
	CString    m_srcWavFilePath;   //  should also be able to covert ODD WAV formats

	long      m_srcSampRate,    m_dstSampRate;   //  for the resampler
};




////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_WAVCONVERT_H__C38AB9E0_137C_4991_8350_59D8811E55B6__INCLUDED_)
