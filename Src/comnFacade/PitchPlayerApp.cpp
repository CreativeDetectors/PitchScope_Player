/////////////////////////////////////////////////////////////////////////////
//
//  PitchPlayerApp.cpp   -    High level functions for a family of apps
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


#include  "Mmsystem.h"     //  for MIDI     Version 4.00    from the  SDK(careful)



#include   "..\comnFacade\UniEditorAppsGlobals.h"

#include   "..\comnFacade\VoxAppsGlobals.h"



//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"      //  #include <FixMath.h>    ...Mac's version of Fixedpoint 
#include  "..\ComnGrafix\CommonGrafix.h"     	
//////////////////////////////////////////////////     



#include   "..\ComnGrafix\OffMap.h" 
#include   "..\comnGrafix\TransformMap.h"


#include "..\comnInterface\DetectorAbstracts.h"  



#include "..\comnCatalog\External.h"
#include "..\comnCatalog\CalcedNoteExternal.h"


#include  "..\ComnAudio\NoteGenerator.h"



///////////////////////////

#include  "..\ComnAudio\dsoundJM.h"        //  I copied it in, bigger than the VC++ version

#include  "..\comnAudio\BitSourceAudio.h"



///////////////////////////

#include  "..\ComnAudio\PlayBuffer.h"

#include  "..\ComnAudio\AudioPlayer.h" 



#include   "..\comnMisc\FileUni.h"  
#include   "..\comnAudio\Mp3Decoder.h"      
#include   "..\comnAudio\ReSampler.h"
#include  "..\ComnAudio\FFTslowDown.h"

#include  "..\comnAudio\WavConvert.h"



#include   "..\ComnAudio\CalcNote.h"
#include   "..\ComnAudio\SPitchCalc.h"



#include  "..\ComnAudio\EventMan.h"   

#include  "..\ComnFacade\SoundHelper.h"   



#include "PitchPlayerApp.h"


/////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
////////////////////////////////////////////////////////////////////////


EventMan&         Get_EventMan();       
SoundHelper&      GetSoundHelper();

HINSTANCE    Get_Modules_hInstance();  



void    Animate_All_Viewjs_PitchPlayer_GLB(   long  curSample,    short curScalePitch    );

void    Draw_LastFrame_Bullets_Gagues_PitchPlayer_GLB();



////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


PitchPlayerApp::PitchPlayerApp()     
{

	CString   retErrorMesg;

	short    appCode =    Get_PitchScope_App_Code_GLB();         //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope




	/***
	m_exeVersionNumber  =   10;     //   (  10  is  1.0  )   This must be  >=  to Files VERSION-number, for files to load

			//  Alway keep FILE versions 9 digits lower than the current EXE version[ 10 ],  to that I can release
	        //  CORRECTED File verions upward without mandating a release of a new EXE( with a higher version )   1/06


	m_lastObjectFilesPathLoaded.Empty();
	*****/
	m_exeVersionNumber =    20;    //    ( 2.0 ) 

	m_filesVersionNumber =   12;       //   12 is for  Navigator 1.0  and  PitchScope 2.0(Scriber)   ...A newFAMILY of new apps (Player.exe)    2/12
													  //
													  //   ADDITIONS for  12:     LickList,   no real DetectZones(dummy )



	m_detectionSensitivityThreshold =  31;    //   31[ syncs with slider default position ]    Make sensitivity a little LOW (high numbers )  so NEW users
															    //                                     hav to increase,  rather than decrease from a confusing over-sensiive state.



	m_rightChannelPercent =  50;    //  init


	m_wavsVolumeApp  =    50;         //		80 [ overpowers MID on some machines ]  	{ 0 - 100  }   ...in percent 

	m_midiVolume   =    50;



	m_numberNeededToMatch  =    4;     //      ***  ADJUST ***
	m_sizeOfPrimNotesCircque =     7;    //      ***  ADJUST from HERE?? ***



	m_stereoChannelCode  =  TransformMap::CENTEREDj;    //  NOT used much anywhere,  bu Allocate_AudioPlayer() needs its address.

//	 m_chunkSize =       TransformMap::Get_ChunkSize_OLD_PitchScope();   // **********   BAD,  do NOT use this  ******************




	 m_sourceFilesPath.Empty();

	 m_lastObjectFilesPathLoaded.Empty();


	GetSoundHelper().Set_BitSource_Streaming(  NULL  );   


	m_lastFilePosition =  -1;  


	m_currentBitSourceCode =  -1;

	 
	if(    ! Initialize_SourceAdmin(  retErrorMesg  )    )     // *** TROUBLE,  decides wrong bitsource for MP3   1/26/10
	{
		ASSERT( 0 );
		AfxMessageBox(  retErrorMesg  );
	}


	if(    ! Switch_Current_BitSource(   BitSource::SOURCEmp3,   retErrorMesg  )     )    
	{
		ASSERT( 0 );
		AfxMessageBox(  retErrorMesg  );
	}



	m_playSpeedFlt  =   1.0;


	m_supressNotesTextDrivingViews =  false;     // so far,  ONLY Navigator has DrivingViews


	m_calcedNoteListTemp.Set_Dynamic_Flag(         true  );    //  true:   just like the Subjects in  UniBasic object
	m_calcedNoteListMaster.Set_Dynamic_Flag(  true  );    



	m_lickList.Set_Dynamic_Flag(  true  );

	m_dstListIsModifiedOLD =   false;



													//   For  PLAYER.exe  ONLY...    small   WAV-DELAY        7/2012


	m_baseDelay     =   -1;    //  -1: Disable       (   Sparky[ 0 ]      Fido[ 2 ]     Lassie[ 0- ]       ....But more computers are like Sparky,  so use value [ 1 ]

	if(   appCode  ==   0   )    //   0:  Player.exe
		m_baseDelay =   1;            //  1:  this will INITIALIZE and ENABLE  the small WAVdelay for Player,  inside of  WavConvert

											                                 
	m_overideDelay =   -1;    //        Special MODE:   If this has a value  >=0,  then that is the REAL DELAY to use,  and  m_baseDelay is processed as ZERO   




	m_noteListHardwareDelayForPlayer =   2;     //    ????   DECIDE ???      ***INIT:   Fido[ 0 ]    Sparky[ 5 ]     Lassie [  10 ]      8/2012
									//  REMEMBER,  this is ONLY for Player with NoteList PLAY,   and it will be RARE that Player.exe gets a notelist in reality.  8/2012




														                                           //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope	
	if(         appCode  ==   1   )     //  Navigator  
	{

		m_usingCircQueDFTprobes       =   true;   //  true

		m_computerPerformanceFactor =       0;    //     0: Fast     1: Average     2: Slow

	}
	else if(   appCode  ==   0   )    //  Player  
	{

		m_usingCircQueDFTprobes       =   false;    //   false:  is too expensive for the processor

		m_computerPerformanceFactor =       1;     //  Does NOT really matter,  Player does NOT do  SlowDown  speeds    9/2012
	}
	else
	{	ASSERT( 0 );   }

}


											////////////////////////////////////////


PitchPlayerApp::~PitchPlayerApp()
{
}


										////////////////////////////////////////


bool   PitchPlayerApp::SRC_File_Is_Loaded()
{

	if(    m_sourceFilesPath.IsEmpty()    )
		return false;
	else
		return true;
}


											////////////////////////////////////////

bool	 PitchPlayerApp::Is_DST_List_Empty()
{

	if(    m_calcedNoteListMaster.Is_Empty()     )
		return  true;
	else
		return  false;
}


											////////////////////////////////////////


bool    PitchPlayerApp::Has_A_MP3()  
{

	if(    ! SRC_File_Is_Loaded()    )
		return  false;


	bool       isAMP3file  =    External::Does_FileName_Have_Extension(  "mp3",   m_sourceFilesPath   );
	return    isAMP3file;
}


											////////////////////////////////////////


WavConvert*    PitchPlayerApp::Get_WavConvert(   CString&  retErrorMesg      )   
{

	WavConvert    *retWavConvertPtr =   NULL;
	SoundHelper&   soundHelper  =    GetSoundHelper();


	/******  BAD, assumption,  can still have a  BitSourceStreaming and a  WavConvert.  In fact the  WavConvert does NOT get released till the end of the application. 

	if(    ! SRC_File_Is_Loaded()     )
	{
		retErrorMesg =  "Load a Music File." ;
		return  NULL;
	}
	******/

	BitSourceStreaming  *bitSourceStream         =     soundHelper.Get_BitSource_Streaming();
	if(                  bitSourceStream ==  NULL  )   
	{
		retErrorMesg =  "PitchPlayerApp::Get_WavConvert  FAILED,  BitSourceStreaming is NULL." ;
		return  NULL;
	}


	retWavConvertPtr         =        bitSourceStream->m_wavConvert;
	if(  retWavConvertPtr  ==  NULL )
	{
		retErrorMesg =  "PitchPlayerApp::Get_WavConvert  FAILED,  m_wavConvert is NULL." ;
		return  NULL;
	}
	else
		return  retWavConvertPtr;
}



											////////////////////////////////////////


bool    PitchPlayerApp::Change_LogDFT_Probe_Algo(   bool    useCircularQues,   CString&  retErrorMesg    )
{


	//	CALLED BY:     PitchPlayerApp::Set_Computer_Performance_Factor()        PsNavigatorDlg::Toggle_LogDFT_Probe_Algo()


	SoundHelper&     soundHelper  =     GetSoundHelper();
	SPitchCalc        *spCalcer        =     soundHelper.Get_SPitchCalc();



	short     appCode  =    Get_PitchScope_App_Code_GLB();         //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope

	if(       appCode  ==  0   )       //    0:  Player      *** This is ONLY for NAVIGATOR ***      9/2012
	{
		ASSERT( 0 );
		return  true;
	}



	if(   spCalcer  ==  NULL   )
	{
		m_usingCircQueDFTprobes  =    useCircularQues;      // When a song is eventually loaded,  this will dictate the correct algo.
		return  true;
	}



	if(     ! spCalcer->Allocate_DFT_Probes(   useCircularQues,    0,    retErrorMesg    )      )
	{
		return  false;
	}


	m_usingCircQueDFTprobes  =    useCircularQues;


	return  true;
}



										////////////////////////////////////////


void	  PitchPlayerApp::Set_Computer_Performance_Factor(   long  computerPerformanceFactor   )
{


	       //   CALLED by      PsNavigatorDlg::On_Settings_optionsMenu()      PitchPlayerApp::Initialize_BitSource()


	SoundHelper&    soundHelper  =    GetSoundHelper(); 
	CString             retErrorMesg;


	short   appCode  =    Get_PitchScope_App_Code_GLB();         //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope

	if(       appCode   !=   1    )     //   1:   Only for NAVIGATOR
	{
		return;   
	}



	m_computerPerformanceFactor =    computerPerformanceFactor;          //     0: Fast     1: Average     2: Slow




	bool  useDFTCircularQues  =   false;

	if(    computerPerformanceFactor  ==   0   ) 
		useDFTCircularQues  =    true;         //  Only for the  'FAST Computer'   setting   ...is expensive algo.



	if(     ! Change_LogDFT_Probe_Algo(   useDFTCircularQues,   retErrorMesg  )     )    //  this has a valid function,  even if a song is NOT loaded.
	{
		AfxMessageBox(   retErrorMesg   );
	}	



//	if(   ! SRC_File_Is_Loaded()  )   **** NO!!!!!   Somtimes the below part does NEED EXECUTION,  even if SPitchCalc has not yet been allocted.  
//		return;														[    see  PitchPlayerApp::Initialize_BitSource()     ]



	BitSourceStreaming  *bitSource         =  	   soundHelper.Get_BitSource_Streaming();    //  even if  NOT NULL,   a song could  NOT be LOADED
	if(                             bitSource ==  NULL   )
	{  ASSERT( 0 );
		return;
	}

	bitSource->Set_Computer_Performance_Factor(   computerPerformanceFactor   );    // 
}




											////////////////////////////////////////


bool    PitchPlayerApp::Change_PlaySpeed_Member_Variables(    double   nuPlaySpeed,    bool   wasPlaying,    CString&  retErrorMesg      )   
{


			   //  Must ULTIMATELY change speed values in   {  WavConvert,   AudioPlayer,   BitSource,  SPitchCalc,   and  PitchPlayerApp }    


//  CALLED by:     PsNavigatorDlg::Execute_Speed_Change(),     PitchPlayerApp::Choose_SRC_File_for_Viewing(),    PsNavigatorDlg::Restore_Default_Settings_to_Controls()
                  
	SoundHelper&  soundHelper  =     GetSoundHelper();

	retErrorMesg.Empty();


	if(      nuPlaySpeed   <  1.0   )
//		||   nuPlaySpeed   >  2.0    )    *********  WANT a limit ??????
	{
		retErrorMesg =  "PitchPlayerApp::Change_PlaySpeed_Member_Variables  FAILED,  input bad SlowDown play speed." ;
		return  false;
	}


	short   appCode  =   Get_PitchScope_App_Code_GLB();         //    0:  Player      1:  Navigator      2: VoxSep     3:  PitchScope

	if(       appCode  ==   0  
		&&  nuPlaySpeed  !=  1.0   )    //   1.0   Need to allow for initialization   6/12
	{
		retErrorMesg =    "No speed change for Player" ;     //  No   Speed Change   allowed for Player.exe   6/2012
		return  false;
	}




	AudioPlayer             *audioPlayer  =    soundHelper.Get_Streaming_Audioplayer();  

	BitSourceStreaming  *bitSource    =	soundHelper.Get_BitSource_Streaming();

	SPitchCalc                *spCalcer    =     soundHelper.Get_SPitchCalc();


	if(   spCalcer  ==  NULL    )      //  No song is  'COMPLETELY Loaded'  and  initialized.
	{
		ASSERT(    audioPlayer  ==   NULL   );     
//		ASSERT(    bitSource  ==   NULL   );     *** NO,   can have an UNINITIALIZED  BITSOURCE present when this is called in   Choose_SRC_File_for_Viewing()
	}



	
	WavConvert  *wavConvert      =     Get_WavConvert(  retErrorMesg  );    //    its funny, but this can hang around even if  ! SRC_File_Is_Loaded() because  
	if(          wavConvert  ==  NULL   )						            //    there are 2 WavConvert, one for each BitSource (mp3 and WAV).  
	{
		//  Get here when first loading a file.  Think is OK to procede.   'nuPlaySpeed'   equals 1 when get here  11/30/11

		ASSERT(   nuPlaySpeed  ==  1.0   );

		m_playSpeedFlt  =    nuPlaySpeed;       
	}
	else
	{  if(    ! wavConvert->Change_PlaySpeed(    nuPlaySpeed,    m_computerPerformanceFactor,   m_numberNeededToMatch,    retErrorMesg  )    )  // ***** SLOPPY
		{
			m_playSpeedFlt  =    nuPlaySpeed;  
			return  false;
		}
	}


	
	if(    spCalcer   !=  NULL   )               //  Sometimes this is NULL, like when app first starts and LOADS a file  [  Choose_SRC_File_for_Viewing()  ].   11/2011
	{       
		spCalcer->m_playSpeedFlt =    nuPlaySpeed;  
	}




	if(    audioPlayer  !=  NULL    )        //  Is NULL  when a new File is loaded
	{

		if(    ! audioPlayer->Set_PlaySpeed(   nuPlaySpeed   )     )      //  just changes  memberVars
		{
			ASSERT( 0 );
		}


		audioPlayer->m_lastWindowPlayStartSample =  -1;     //   Is no Longer valid.  Need to have it reinitialize itself.   
											                //      [ lack of this INIT caused a very UGLY BUG, now fixed   8/29/2012  ] 


		audioPlayer->Fill_Buffer_With_Silence();              //  this will also  zero-out   the BIG  WAV Delay-Buffer  ( but NOT the small Delay in WavConvert  )
	}
		



	m_playSpeedFlt  =    nuPlaySpeed;  


	//   Do NOT do  PreRoll()  here,  havecalling functions must do that.    11/2011

	return   true;
}




								////////////////////////////////////////////////////////////////////////////////////////////
								///////////    3+  Functions below  are very IMPORTANT to future code   3/2012    //////////
								///////////   ( but need better names and clarifilcation of duties.   //////////////////////
								////////////////////////////////////////////////////////////////////////////////////////////



void    PitchPlayerApp::Animate_All_Viewjs(   long  curSample,    short curScalePitch    )   
{

		//  Use this to render while PLAY  *IS HAPPENING*     (  Only moves the   **FILE-SLIDER**   in Player  during   *PLAY-TIME*   )      

																

	//   Is CALLED  by   EventMan::Process_Event_Notification_PPlayer()    and  PitchPlayerApp::On_View_SRC_File,    SoundHelper::Continue_Play_Backwards
	//						      SoundHelper::Pause_Play(),      SoundHelper::Move_to_Files_Position


	Animate_All_Viewjs_PitchPlayer_GLB(   curSample,    curScalePitch   );   
}


											////////////////////////////////////////


void    PitchPlayerApp::ReDraw_All_Anime_Viewjs()
{

		//  Use this to render when PLAY  *STOPPED*     (  the  "Last Frame"  of  **BULLETS**   of Revolver and Gagues  )      2/2012


	//		CALLED BY:    EventMan::Finish_Process_For_PPlayer(),    SoundHelper::Pause_Play(),   SoundHelper::Move_to_Files_Position()


	Draw_LastFrame_Bullets_Gagues_PitchPlayer_GLB();    //  Used in Navigator to draw the  "Last Frame"   ( Bullets of Revolver and Gagues )     1/2012
}



											////////////////////////////////////////


void   PitchPlayerApp::ReDraw_Bitmaps(   bool  renderWithoutText  )
{


	     //  Use this to render when PLAY  *STOPPED*      (  the  **DRIVING-VIEWS**    SHOULD I use this in OTHER  'UniApplication'   APPS???       2/2012


	//		CALLED BY:   EventMan::Finish_Process_For_PPlayer()     and   SoundHelper::Pause_Play()


	ReDraw_DrivingView_OnDialog_GLB(  renderWithoutText  );
}



											////////////////////////////////////////


void   PitchPlayerApp::ReDraw_Bitmaps()
{

	     //  Use this to render when PLAY  *STOPPED*      (  the  **DRIVING-VIEWS**    SHOULD I use this in OTHER  'UniApplication'   APPS???       2/2012


	//		CALLED BY:   EventMan::Finish_Process_For_PPlayer()     and   SoundHelper::Pause_Play()


	ReDraw_DrivingView_OnDialog_GLB(   m_supressNotesTextDrivingViews  );
}



											////////////////////////////////////////
											////////////////////////////////////////


void	    PitchPlayerApp::Update_FirstSampleIdx_All_Viewjs(   long  sampleIdx   ) 
{
			
ASSERT( 0 );  //  NOT CALLED in Navigator or  Player   1/2012   MAYBE need to keep this,  becase  DetectorApp  affects a lot of things 

			 //  *****  MAYBE need to keep this,  becase  DetectorApp  affects a lot of things  2/12   ************

//	short   curScalePitch =  -1;
//	Animate_All_Viewjs_PitchPlayer_GLB(   sampleIdx,    curScalePitch   );
}



void	    PitchPlayerApp::Update_FirstSampleIdx_All_AnimeViewjs(   long  sampleIdx   ) 
{

ASSERT( 0 );  //  NOT CALLED in Navigator or  Player   1/2012

			  //  Stop having function call this,  use  Animate_All_Viewjs()  instead  2/12

//	short   curScalePitch =  -1;
//	Animate_All_Viewjs_PitchPlayer_GLB(   sampleIdx,    curScalePitch   );  
}


void	    PitchPlayerApp::Update_FirstSampleIdx_Active_RankingViewj(   long  sampleIdx   ) 
{

ASSERT( 0 );  //  NOT CALLED in Navigator or  Player   1/2012

				 //  Stop having function call this,  use  Animate_All_Viewjs()  instead  2/12

//	short   curScalePitch =  -1;
//	Animate_All_Viewjs_PitchPlayer_GLB(   sampleIdx,    curScalePitch   );  
}




											////////////////////////////////////////
											////////////////////////////////////////


bool	   PitchPlayerApp::Initialize_SourceAdmin(   CString&  retErrorMesg   )
{

									//   just allocates some BitSources for its List,   but does  NOT Initialize  them

	retErrorMesg.Empty();

	BitSourceStreaming   *bitSourceStreaming;
	BitSource                  *retBitSource=  NULL;


																							//   main  SOURCE   for WAV files

	if(   ! Get_SourceAdmin().Alloc_BitSource(   BitSource::SOURCEmp3,   &retBitSource,   retErrorMesg   )     )
		return  false;

	bitSourceStreaming     =       dynamic_cast< BitSourceStreaming* >(  retBitSource  );  
	if(   bitSourceStreaming  ==  NULL   )
	{  
		retErrorMesg =   "PitchPlayerApp::Initialize_SourceAdmin  FAILED,  could not dcast to BitSourceStreaming [ SOURCEmp3 ] ." ;
		return  false;
	}

	bitSourceStreaming->m_midiVolumeSoundmanAddr    =      &m_midiVolume;     
	bitSourceStreaming->m_wavVolumeSoundmanAddr    =      &m_wavsVolumeApp;    






	if(   ! Get_SourceAdmin().Alloc_BitSource(   BitSource::SOURCEsTREAMwav,   &retBitSource,   retErrorMesg   )     )
		return  false;

	bitSourceStreaming     =       dynamic_cast< BitSourceStreaming* >(  retBitSource  );  
	if(   bitSourceStreaming  ==  NULL   )
	{  
		retErrorMesg =   "PitchPlayerApp::Initialize_SourceAdmin  FAILED,  could not dcast to BitSourceStreaming [ SOURCEsTREAMwav ] ." ;
		return  false;
	}

	bitSourceStreaming->m_wavVolumeSoundmanAddr    =      &m_wavsVolumeApp;     
	bitSourceStreaming->m_midiVolumeSoundmanAddr    =      &m_midiVolume;     



	/****
																							//   LEAD

	if(   ! Get_SourceAdmin().Alloc_BitSource(   BitSource::LEAD,   &retBitSource,   retErrorMesg   )     )
		return  false;

	bitSourceStreaming     =       dynamic_cast< BitSourceStreamingMS* >(  retBitSource  );  
	if(   bitSourceStreaming  ==  NULL   )
	{  
		retErrorMesg =   "PitchDetectorApp::Initialize_SourceAdmin  FAILED,  could not dcast to BitSourceStreamingMS [ LEAD ] ." ;
		return  false;
	}
	bitSourceStreaming->m_wavVolumeSoundmanAddr    =      &(    Get_SoundMan().m_wavsVolume    );      




																							//   BACKGROUND

	if(   ! Get_SourceAdmin().Alloc_BitSource(   BitSource::BACKGROUND,   &retBitSource,   retErrorMesg   )     )
		return  false;

	bitSourceStreaming     =       dynamic_cast< BitSourceStreamingMS* >(  retBitSource  );  
	if(   bitSourceStreaming  ==  NULL   )
	{  
		retErrorMesg =   "PitchDetectorApp::Initialize_SourceAdmin  FAILED,  could not dcast to BitSourceStreamingMS [ BACKGROUND ] ." ;
		return  false;
	}
	bitSourceStreaming->m_wavVolumeSoundmanAddr    =      &(    Get_SoundMan().m_wavsVolume    );      



																							//   MODIFIED

	if(   ! Get_SourceAdmin().Alloc_BitSource(   BitSource::MODIFIEDwAV,   &retBitSource,   retErrorMesg   )     )
		return  false;

	bitSourceStreaming     =       dynamic_cast< BitSourceStreamingMS* >(  retBitSource  );  
	if(   bitSourceStreaming  ==  NULL   )
	{  
		retErrorMesg =   "PitchDetectorApp::Initialize_SourceAdmin  FAILED,  could not dcast to BitSourceStreamingMS [ MODIFIED ] ." ;
		return  false;
	}
	bitSourceStreaming->m_wavVolumeSoundmanAddr    =      &(    Get_SoundMan().m_wavsVolume    );      

	*****/


	if(    ! Get_SourceAdmin().Set_Current_BitSource(   BitSource::SOURCEmp3,    retErrorMesg   )     )    //  a partial assignment,  not initialization
		return  false;
	else
		return  true;
}



								/////////////////////////////////////////////////////


BitSourceStreaming*	   PitchPlayerApp::Initialize_BitSource(   CString&  srcFilesPath,    bool&  retFileAcessError,    CString&  retErrorMesg  ) 
{

													//   ONLY called by     PitchPlayerApp::Choose_SRC_File_for_Viewing() 

	SoundHelper&   soundHelper  =     GetSoundHelper();  
	EventMan&        eventMan      =     Get_EventMan();

	retErrorMesg.Empty();


	if(   srcFilesPath.IsEmpty()    )
	{
		retErrorMesg =    "PitchPlayerApp::Initialize_BitSource  failed,  filePath is empty."  ;
		return  false;
	}



	long  chunkSize =   512;    // ******** OK ????    12/09  *******************

	long  numberBytesIn16bitSample =   4;

	long  numberOfWavemanEvents  =    Get_EventMan().m_numberOfNotificationEvents;  

	long  numSamplesInBlockLocked  =     BLOCKLOCKEDSIZE  /   numberBytesIn16bitSample;    //  11,040  samples in  Fetch_Streaming_Samples_Direct() block-load
	
	long  totalSamplesInPieEvent  =     (long)(  TWOSECBYTES  /  (  numberOfWavemanEvents  -1 )   )    / 4;     // How many bytes in an Event 



	//  **** HAVE one PROBLEM with this here.  If user tries to load the SAME mp3 file that is already loaded, the file dialog will complain that the file is already loaded.

	//	m_sourceAdmin.Release_All_BitSources_Files();   //   need to initialize BOTH of the BitSources  or we could leave a file open when switching from a WAV to a MP3  




	bool   isAWAVfile =    BitSource::Does_FileName_Have_Extension(  "wav",   srcFilesPath   );

	short   bitSourceKind =  -1;

	unsigned short    retFormatTypeWAVsrc;          
	short                  retChannelsCountWAVsrc;     
	short                  retBitsPerSampleWAVsrc;      
	long                    retSampleRateWAVsrc;        
	long                    retTotalSamplesBytesWAVsrc;																										



	if(   isAWAVfile   )
	{

		if(  ! WavConvert::Get_WAV_Header_Info(   srcFilesPath,              //  will open and Close the file
																		retFormatTypeWAVsrc,          
																		retChannelsCountWAVsrc,     
																		retBitsPerSampleWAVsrc,      
																		retSampleRateWAVsrc,        
																		retTotalSamplesBytesWAVsrc,								
																		retErrorMesg  )     )
				return  NULL;


		/*****
		if(   retSampleRateWAVsrc  ==  DEFAULTsAMPLINGrATEmp3   )    //  44100
			bitSourceKind =   BitSource::SOURCEwav;           // *****************  FIX,  needs to read this some how.
		else
			bitSourceKind =   BitSource::SOURCEsTREAMwav; //    *****************  FIX,  needs to read this some how.
		*****/

		bitSourceKind =    BitSource::SOURCEsTREAMwav;   // ********  TEMP,  keep using this for all speeds.   Still OK,  1/2012  **********
	}
	else
		bitSourceKind =    BitSource::SOURCEmp3;


	


	bool    cleanOtherBitsource =  false; 

	if(    bitSourceKind  !=   m_currentBitSourceCode  )
		cleanOtherBitsource  =   true;



	if(   ! Switch_Current_BitSource(  bitSourceKind,   retErrorMesg   )     )    //  INITIALIZE  to the essential wav file before load
		return  NULL;



	BitSourceStreaming  *bitSourceToClean  = 	Get_Unused_BitSource(   bitSourceKind,    retErrorMesg    );

	if(            bitSourceToClean  ==   NULL   
		&&    !  retErrorMesg.IsEmpty()    )
	{
		return  NULL;
	}
													                
															//   The SOURCE-Wave MUST be present, or the project and or notelist should not load


	BitSourceStreaming   *bitsStreaming      =      Get_BitSource_Streaming(    bitSourceKind,   retErrorMesg   );  
	if(                              bitsStreaming  ==  NULL   )
	{  
		retErrorMesg =   "PitchPlayerApp::Initialize_BitSource failed,  could not dcast to BitSourceStreaming." ;
		return  NULL;
	}



	if(     bitsStreaming->m_wavConvert   ==   NULL    )
	{

		bitsStreaming->m_wavConvert           =        new   WavConvert();    // Is this the only place the constructor is called
		if(   bitsStreaming->m_wavConvert ==  NULL   )
		{
			ASSERT( 0 );
			retErrorMesg =   "PitchPlayerApp::Initialize_BitSource  FAILED,  could not alloc  WavConvert  for BitSource." ;
			return  NULL;
		}		


		bitsStreaming->m_wavConvert->m_baseDelayAddr      =     &(   m_baseDelay   );

		bitsStreaming->m_wavConvert->m_overideDelayAddr  =     &(   m_overideDelay   );
	}



	long   chunkSizeNav =     Get_ChunkSize_Navigator();




	if(     ! bitsStreaming->Initialize(    chunkSizeNav,    srcFilesPath,    retFileAcessError,    m_numberNeededToMatch,         //  will alloc  SPitchCalc  
		                                                                                           &m_noteListHardwareDelayForPlayer,   m_usingCircQueDFTprobes,   retErrorMesg  )     ) 
	{		
														//   SPitchCalc  will consume  1.3 MegaBytes  of memory, if using circular cues !!!    9/12
		if(    retFileAcessError   )
			retErrorMesg.Format(   "PitchPlayerApp::Initialize_BitSource  FAILED,  %s  could not be registered because:  %s."  ,  srcFilesPath,  retErrorMesg  ); 
		
		Release_and_Close_SRCfile(); 
		return  NULL;			
	}



	Set_Computer_Performance_Factor(   m_computerPerformanceFactor   );    //  TRICKY,  but works OK and keeps all vars in correct state.    9/4/2012





	if(   bitsStreaming->m_sPitchCalc  ==  NULL  )           //   INITIALIZE   the new  SPitchCalc
	{
		ASSERT( 0 );
		retErrorMesg =   "PitchPlayerApp::Initialize_BitSource  FAILED,   m_sPitchCalc is  NULL ." ;
		return  NULL;
	}
														
		bitsStreaming->m_sPitchCalc->m_calcedNoteListMasterApp =    &m_calcedNoteListMaster;      //  Finish odd  INITIALIZE
 
		bitsStreaming->m_sPitchCalc->m_displayBritnessFactor      =    soundHelper.m_displayBritnessFactor;



	bitsStreaming->m_parentFilePath =   srcFilesPath;




	if(        cleanOtherBitsource    
		&&   bitSourceToClean   !=  NULL    )   //  If we have NOT exited this function at this point,  then there will NOT be
	{
		bitSourceToClean->Clean_All_Resources(); 
	}

   

	Get_Source_Files_Path()  =    srcFilesPath;    //   Marks the success of the load

	return  bitsStreaming;
}



											////////////////////////////////////////


BitSourceStreaming*		PitchPlayerApp::Get_Unused_BitSource(  short   currentBitSourceCode,   CString&  retErrorMesg     )
{

			//   If we are loading a  MP3 (  currentBitSourceCode  ),  then we want the Bitsource of the  BitSourceStreamingWAV,   and vice-versa   11/11 

	
	short   codeOfOtherBitsource =  -1;


	if(          currentBitSourceCode ==   BitSource::SOURCEmp3   )
		codeOfOtherBitsource =    BitSource::SOURCEsTREAMwav;
	else if(    currentBitSourceCode ==   BitSource::SOURCEsTREAMwav   )
		codeOfOtherBitsource =    BitSource::SOURCEmp3;
	else
	{	ASSERT( 0 );
	//	AfxMessageBox(  "PitchPlayerApp::Get_Unused_BitSource()  failed"   );
		retErrorMesg  =  "PitchPlayerApp::Get_Unused_BitSource() FAILED,  unknown  BitSource-Code case." ;
		return  NULL;
	}


	BitSourceStreaming  *otherBitsource =   Get_BitSource_Streaming(    codeOfOtherBitsource,   retErrorMesg   );
	if(                             otherBitsource ==   NULL   )
	{
		ASSERT( 0 );
	}	

	return   otherBitsource;
}



											////////////////////////////////////////


bool   PitchPlayerApp::Switch_Current_BitSource(   short  bitSourceCode,    CString&  retErrorMesg   ) 
{				

		//   Only CALLED by    PitchPlayerApp::Initialize_BitSource()

		//   Only   're-ASSIGNS member variables'    ...does  NOT try to initialize, create, or load the file  [ see Choose_Current_BitSource  ]

	SoundHelper&   soundHelper  =     GetSoundHelper();  
	retErrorMesg.Empty();


	BitSourceStreaming   *bitSourceStreaming    =     Get_BitSource_Streaming(   bitSourceCode,   retErrorMesg   );  //  bitSourceCode:   5 or 6  
	if(                              bitSourceStreaming  ==  NULL   )
		return  false;



	soundHelper.Set_BitSource_Streaming(   bitSourceStreaming  );     



	if(   soundHelper.m_audioPlayer   !=  NULL   )     //  NO error if  m_audioPlayer is NULL, this get called early for initialization in   PitchPlayerApp::PitchPlayerApp()
	{
		if(    ! soundHelper.m_audioPlayer->Set_BitSource(   bitSourceStreaming,    retErrorMesg  )     )       //   does above   12/09
			return  false;
	}


	m_currentBitSourceCode =    bitSourceCode;

	return  true;
}



											////////////////////////////////////////


BitSourceStreaming*	  PitchPlayerApp::Get_BitSource_Streaming(    short bitSourceCode,    CString&  retErrorMesg   )
{

												//  save some ugly dynamic-cast  in the code

	BitSourceStreaming  *bitSrcStreaming =  NULL;

	retErrorMesg.Empty();



	BitSource  *bitSource  =      Get_SourceAdmin().Get_BitSource( bitSourceCode );
	if(   bitSource   ==   NULL   )
	{
		retErrorMesg =  "PitchPlayerApp::Get_BitSource_Streaming  FAILED,   Get_BitSource_Streaming returned NULL."  ;
		return  false;
	}


	bitSrcStreaming    =     dynamic_cast< BitSourceStreaming* >(  bitSource  );  
	if(   bitSrcStreaming  ==  NULL   )
	{  
		retErrorMesg =   "PitchPlayerApp::Get_BitSource_Streaming  FAILED,  could not dcast to BitSourceStreaming." ;
		return  NULL;
	}


	return  bitSrcStreaming;
}



											////////////////////////////////////////
											//////////////    FILEs   //////////////
											////////////////////////////////////////

	
bool	 PitchPlayerApp::Browse_For_Missing_File(   CString&  filesOldPath,    bool&  retUserCanceled,    CString&  retFilesNewPath,   CString&  retErrorMesg   )
{

	

	CString  pitchscopeProjectExtension  =    "pnl" ;     //  pnl:  NoteList               

	                                                       //   ppj :     OLD PitchScope  'PROJECT'  file   


	retErrorMesg.Empty();
	retFilesNewPath.Empty();  //  RETURNS :  false if file could not be found  ...BUT is NOT a program error 
	retUserCanceled =  false;


	bool   headerDoesNotHaveAprojectName =  false;      // Could be a Notelist that was created by Navigator    3/11

	LPCTSTR   lpszOrigFileName =  NULL;



	CString  strExt,   strFilter,   origFileName,    strFileName;

	if(    filesOldPath.IsEmpty()    )
	{
		//    retErrorMesg =  "Browse_For_Missing_File FAILEDThe file name and/or path is empty."  ;
		//    return  false;

		headerDoesNotHaveAprojectName =  true;

		strExt =   pitchscopeProjectExtension;   // ************   HARDWIRED,  fix   3/11  **********************

		lpszOrigFileName =  NULL;
	}
	else
	{
		Get_Paths_Target_FileName_GLB(  filesOldPath,  origFileName  );

		strExt =   filesOldPath.Right( 3 );  

		lpszOrigFileName =     origFileName.GetBuffer( 0 );    //  works OK 
	}


//	strFilter.Format(   "*%s||" ,    origFileName    );  ...BAD
	strFilter.Format(   "Files:  (*.%s)|*.%s||" ,    strExt,   strExt    );    



	bool  userStillTries =  true;

	do
	{
		CFileDialog    dlg(   TRUE,
									_T( strExt ), 
									  lpszOrigFileName,   //   NULL, 
									  OFN_HIDEREADONLY   |    OFN_OVERWRITEPROMPT,		                          
									  strFilter    // _T(    "Files (*.pnl) | *.pnl  ||" )    							   
								 );      


		LONG  nResult    =      dlg.DoModal();
		if(       nResult  !=  IDOK  )
		{

			//  retExplanationMesg.Format(  "Browsing for  %s  was canceled user" ,   origFileName   );
			retUserCanceled =  true;

			userStillTries =  false;
			return  true;
		}


	//	if(  dlg.OnFileNameOK( )    ...would this ok   ????


		CString  mesg,  foundFileName  =    dlg.GetFileName();


		if(          ! headerDoesNotHaveAprojectName     //  that is,    origFileName>IsEmpty()
			   &&    foundFileName.CompareNoCase(  origFileName   )  !=   0    )
		{
			mesg.Format(   "%s  does NOT have the same NAME as the original file,  %s.  \n\nPick a file with the SAME NAME, or hit the Cancel button on the 'Open' dialog box." , 
																								   foundFileName,   origFileName  );
			AfxMessageBox(  mesg  );
		}
		else
		{  retFilesNewPath  =   dlg.GetPathName();
			userStillTries =  false;  //  Not trying because we have our result
		}

	}while(  userStillTries  );



	retUserCanceled =  false;

	return  true;
}



								/////////////////////////////////////////////////////


bool		PitchPlayerApp::Query_User_For_SRCfile_Location(   CString&  retHeadersSRCfilePath,    bool&    retSRCfilePathWasChanged,	  CString&  retErrorMesg   ) 
{

						//  If the user had to BROWSE and go to a new path,  the  'retHeadersSRCfilePath'  holds the new path value

			//  *** IMPORTANT:  if( retSRCfilePathWasChanged)   Calling functions must  set  m_projectIsModified = true at the APPROIATE PLACE   5/07 ********************************


	CString   retErrorMesgLoc,  retLocalMesg,   origFileName,  finalPath =  retHeadersSRCfilePath;
	CString   mesg,   retFilesNewPath;


	retErrorMesg.Empty();
	retSRCfilePathWasChanged   =  false;


	if(    External::Is_File_Present(  retHeadersSRCfilePath,   retErrorMesgLoc  )     )
		return  true;



	retSRCfilePathWasChanged  =    true; 
	


	Get_Paths_Target_FileName_GLB(  retHeadersSRCfilePath,   origFileName );

	mesg.Format(   "%s \nIf you can not find  %s  in another folder, then the NoteList File can NOT be loaded. \n\nA file dialog box will next appear so that you can BROWSE for %s in another folder." ,    
																		retErrorMesgLoc,   origFileName,   origFileName  );
	AfxMessageBox(  mesg  );		
			



	bool    retUserCanceledBrowsing;  


	if(   ! Browse_For_Missing_File(   retHeadersSRCfilePath,   retUserCanceledBrowsing,     retFilesNewPath,     retErrorMesg  )     )  
	{
		ASSERT( 0 );  //  this is a REAL error
		return  false;
	}
				

	if(   retUserCanceledBrowsing   )
	{	
		retErrorMesg.Format(  "User canceled the load of %s" ,   origFileName  ); 
		return  false;
	}

	retHeadersSRCfilePath =    retFilesNewPath;    //  this is how we return the NEW path to the calling funct 		
	

/***************************  This is very BAD.     If the ListFile has a bad Project address, and the user cances on loading the project,  this
			                                                                    version check wilol fail and leave the user with out 	

	CString   retScrWAVfilesPath;     //  now that we have finally found the file,  we can check its version 

	if(    ! Peek_At_ProjectFiles_SRC_File_Path(    projectExternal,   projectFilesPath,   retScrWAVfilesPath,    retFilesVersion,     retErrorMesg   )  )
		return  false;
*****/	

	return  true;
}


									/////////////////////////////////////////////////////


bool	  PitchPlayerApp::Query_User_For_ObjectList_Save(    bool&  retUserCanceled,   bool& retUserBailsOut,   CString&  retErrorMesg   )
{


			//   ALSO send up messageBoxes and Dialogs as part of its

	bool  returnVal   =  true;

	retUserCanceled =  false;
	retUserBailsOut  =   false;   // the user decides that they do NOT want to proceed with the initial command  7/07



	if(          Is_DSTlist_Modified()  
//		&&   ! Is_DST_List_Empty()      **** NO,   a user could have an EMPTY NoteList that still have  PHRASES stored in it.   5/2012
	  )   
	{
		CString   retNoteListFileName,   retMesg,   mesg2;


		if(   ! m_lastObjectFilesPathLoaded.IsEmpty()   )
		{
			if(   ! External::Get_Files_Name_from_Path(   m_lastObjectFilesPathLoaded,    retNoteListFileName   )    )
			{
				ASSERT( 0 );
				retNoteListFileName =    m_lastObjectFilesPathLoaded;    // ***** Do I want this ????   12/11
			}

			retMesg.Format(   "You have unsaved changes.  Do you wish to SAVE the current NoteList [  %s  ] to FILE?" ,  retNoteListFileName  );
		}
		else
			retMesg.Format(   "You have unsaved changes.  Do you wish to SAVE the current NoteList to FILE?"  );



		int   reslt    =      AfxMessageBox(  retMesg,	   MB_YESNOCANCEL | MB_ICONQUESTION     );   
		if(    reslt  ==   IDYES   )
		{
			void*      extraData =   NULL;
			CString  listFilesPath; 


			if(   ! Save_DSTlist(   retUserCanceled,    listFilesPath,    retErrorMesg  )    )
			{
				returnVal =   false;
			}
			else
			{  if(    ! retErrorMesg.IsEmpty()    )
			        AfxMessageBox(  retErrorMesg  );    //  show a CONFIRMATION  messageBox    4/07
			}
		}
		else if(    reslt  ==   IDCANCEL    )
		{
		//	retUserCanceled =  true;    // want this instead ????  
			retUserBailsOut =   true;   
		}
	}

	return  returnVal;
}



									/////////////////////////////////////////////////////


void	  PitchPlayerApp::Query_User_For_ObjectList_Save_On_App_Exit()
{

		//   I can NOT use   Query_User_For_ObjectList_Save() because at APP-EXIT,   I can NOT Cancel and keep Navigator still running.  5/2012


	if(          Is_DSTlist_Modified()  
//		&&   ! Is_DST_List_Empty()      **** NO,   a user could have an EMPTY NoteList that still has  PHRASES stored in it.   5/2012
		)   
	{
		CString   retNoteListFileName,   retMesg,   mesg2;


		if(   ! m_lastObjectFilesPathLoaded.IsEmpty()   )
		{

			if(   ! External::Get_Files_Name_from_Path(   m_lastObjectFilesPathLoaded,    retNoteListFileName   )    )
			{
				ASSERT( 0 );
				retNoteListFileName =    m_lastObjectFilesPathLoaded;    // ***** Do I want this ????   12/11
			}

			retMesg.Format(   "You have unsaved changes.  Do you wish to SAVE the current NoteList [  %s  ] to FILE?" ,  retNoteListFileName  );
		}
		else
			retMesg.Format(   "You have unsaved changes.  Do you wish to SAVE the current NoteList to FILE?"  );



		int   reslt    =      AfxMessageBox(   retMesg,	   MB_YESNO | MB_ICONQUESTION     );   
		if(    reslt  ==   IDYES   )
		{
			CString  listFilesPath,     retErrorMesg; 
			bool   retUserCanceled =  false;


			if(   ! Save_DSTlist(   retUserCanceled,    listFilesPath,    retErrorMesg  )    )
			{
				if(            ! retUserCanceled     
					    &&   ! retErrorMesg.IsEmpty()    )
					AfxMessageBox(  retErrorMesg  );    //  Show the ERROR and still close the App.   5/2012
			}
		}
	}
}


									/////////////////////////////////////////////////////


void		PitchPlayerApp::Release_and_Close_SRCfile()
{

								//  Child classes, like  PitchDetectorApp,  MIGHT want overide this funct 

	Get_SourceAdmin().Release_All_BitSources_Files();   

	Get_Source_Files_Path().Empty();  
}



											////////////////////////////////////////


bool   PitchPlayerApp::On_View_SRC_File(   bool&  retUserCanceled,   CString&  retErrorMesg   ) 
{																	


	SoundHelper&  soundHelper  =    GetSoundHelper();
	CString                retFilesPath,   emptyPath;



	bool   retUserBailsOut =  false;	

	if(    ! Query_User_For_ObjectList_Save(   retUserCanceled,   retUserBailsOut,    retErrorMesg   )   )
		return  false;   

	if(   retUserBailsOut   )
		return  true;    //  user decides that they do not want to do the  'CALLING command'    7/07




	emptyPath.Empty();    // This will cause a File Dialog to go up in Choose_SRC_File_for_Viewing()



	if(     ! Choose_SRC_File_for_Viewing(   retUserCanceled,   emptyPath,    retErrorMesg  )     )
		return  false;

	if(   retUserCanceled   )
		return  true;



	SPitchCalc    *sPitchCalc  =   NULL;    
	AudioPlayer  *audioPlayer      =    soundHelper.m_audioPlayer;

	if(     audioPlayer  !=  NULL    )
		sPitchCalc  =    audioPlayer->Get_SPitchCalc();   //  Will stay NULL for  On_File_Open_Menu  and  Choose_SRC_File_for_Viewing




	m_calcedNoteListMaster.Empty();   //  now Player.exe can also do NoteLists  2/12

	m_lickList.Empty();



	if(    sPitchCalc != NULL )
		sPitchCalc->Erase_the_UndoNotes_Data();    //  Anytime the Notelist is erased,  this has to be called. 
	else
	{	ASSERT( 0 );   }   // SHOULD always be there





	retFilesPath   =       Get_Source_Files_Path();
	if(   retFilesPath.IsEmpty()    )    
	{
		return  true;     //  OK...     will be empty if user cancel,  and so no message is deserved.
	}

//	if(    ! Do_Sources_PostLoad_Calcs(  retErrorMesg   )     )    //  only opens one if it is not there   6/07
//		return  false; 

//	if(    ! Open_ListEditor_Windows(  retErrorMesg   )     )        //  only opens one if it is not there   6/07
//		return  false; 


																							//  Need to update the current Samples index to a couple of mechanisms
	if(     audioPlayer!=  NULL    )
		audioPlayer->Set_LastPlayed_SampleIdx(  0  );
	else
	{  ASSERT( 0 ); }    //  Need ERROR reporting ????  Or is this never reached?     1/12/10



//	Update_FirstSampleIdx_All_Viewjs(  0  );   
	Animate_All_Viewjs(   0,    -1   );   


	return  true;
}



									/////////////////////////////////////////////////////


bool	 PitchPlayerApp::Choose_SRC_File_for_Viewing(    bool&  retUserCanceled,   CString&  filePathNoDialog,    CString&  retErrorMesg    )
{

			//     called from MENU to just for VIEWING,  no ObjList or Project		  ******????  WANT to move to  PitchProjectExternal  ??? *********

							//  Only called from  On_View_SRC_File

	SoundHelper&   soundHelper =    GetSoundHelper();    
	CString     strMesg,   retFilesNewPath;


	retUserCanceled =  false;
	retErrorMesg.Empty();



	/*****  MOVED down  OK ???  
	m_sourceAdmin.Release_All_BitSources_Files();   //   need to initialize BOTH of the BitSources  or we could leave a file open when switching from a WAV to a MP3  
																			//   Need to do this very early in case the user tries to load the current file that is load3ed
	m_sourceFilesPath.Empty();    //   this will tell buttons and application that no file is loaded.
	*****/


	bool   noDialog =   false;


	if(    ! filePathNoDialog.IsEmpty()    )
	{
		retFilesNewPath =    filePathNoDialog;

		noDialog =   true;
	}
	else
	{
		CFileDialog   dlg(	  TRUE,		  //  TRUE:  File open
									_T(    "WAV"   ),
									NULL,                             //   _T(  "*.WAV"  ),
									OFN_HIDEREADONLY,     // |   OFN_OVERWRITEPROMPT,

							   //    _T(   "Sound File  (*.WAV)|*.WAV|"   )        
									  _T(  "Music Files  (*.wav;*.mp3) | *.wav;*.mp3||"  ) 			
									);


//		dlg.SetWindowText(  "Open Music File"    );    *** No Good




		if(   dlg.DoModal()  !=   IDOK  )
		{
									//  retErrorMesg =  "User canceled the load of a WAV file." ;

			//  soundMan.Get_Source_Files_Path().Empty();      *** WANT it ????   or will it disturb what is Already registered.
			retUserCanceled =   true;
			return  true;
		}
			
		retFilesNewPath =    dlg.GetPathName();
	}



	if(    retFilesNewPath.IsEmpty()    )
	{
		retErrorMesg =  "No file name was given for loading." ;
		return  false;
	}



	bool    retSRCfilePathWasChanged =    false;     // Possibly may have to BROWSE  for a missing   SOURCE  Music File ( wav or MP3 )  


	if(   noDialog   )   
	{				                     //  The NoteList file has given us a path and FileName for the Music File ( SOURCE file )

		if(   ! Query_User_For_SRCfile_Location(    retFilesNewPath,   retSRCfilePathWasChanged,    retErrorMesg   )    )
		{
			return  false;                                          //  If user has to Browse for a new path,  then the NEW Path is returned to  'retFilesNewPath' 
		}


		if(    retSRCfilePathWasChanged   )
			Set_DSTlist_Modified(  true  );     //  So can PROMPT the user to SAVE this correction (source files path )  to the NoteList File   8/2012  
	}






	if(    ! Change_PlaySpeed_Member_Variables(   1.0,    false,   retErrorMesg  )    )      //  SLOPPY, too early,  the file is not yet loaded. No AudioPlayer, SPCalc,  BitSource.  But it                    
		return  false;													          //              this still works because the new object are all initialized at speed = 1.  3/11




	m_sourceAdmin.Release_All_BitSources_Files();   //   need to initialize BOTH of the BitSources  or we could leave a file open when switching from a WAV to a MP3  
																			//   Need to do this very early in case the user tries to load the current file that is loaded

	m_sourceFilesPath.Empty();    //   this will tell buttons and application that no file is loaded.

		


	bool   isAWAVfile =    BitSource::Does_FileName_Have_Extension(  "wav",   retFilesNewPath   );
	bool   isAMP3file  =    BitSource::Does_FileName_Have_Extension(  "mp3",   retFilesNewPath   );

	if(   ! isAWAVfile     &&   ! isAMP3file   )
	{
		retErrorMesg =  "Only pick a .WAV file or a .MP3 file." ;    // do not think this can happen
		return  false;
	}



	CString    retNewFileName =   retFilesNewPath;      
	bool         retFileAcessError;            // **** Want to do something with this ????   ****



	m_numberNeededToMatch    =    4;     //   Must   INITIALIZED these to DEFAULT values before  Initialize_BitSource()    12/11
	m_sizeOfPrimNotesCircque   =    7;   

	soundHelper.m_userFinalCircqueSizeTweak  =   0;

	m_detectionSensitivityThreshold =    31;



	
	BitSourceStreaming  *bitsStreaming  =    Initialize_BitSource(    retNewFileName,   retFileAcessError,    retErrorMesg   );     //  a LOT goes on in here.   It will allocate a new SPitchCalc.

	if(   bitsStreaming ==  NULL  )  
		return  false;

								

												       //   Better to call   SoundHelper::Allocate_AudioPlayer()  here,   instead of inside  Initialize_BitSource()    1/12

	if(    ! soundHelper.Allocate_AudioPlayer(   *this,     &m_stereoChannelCode,   	&m_lastFilePosition,  	retErrorMesg  )     )    //  only called here
	{  
		Release_and_Close_SRCfile(); 
		return  false;
	}


											//   can  INITIALIZE   SPitchCalc  here,    it was just CREATED with the load of a new song, and creation of a NEW AudioPlayer


	SPitchCalc   *sPitchCalcer       =          bitsStreaming->m_sPitchCalc;     
	if(                 sPitchCalcer   ==   NULL   )
	{  ASSERT( 0 );     }   //  Can this happen ?????    11/11
	else
	{  
		sPitchCalcer->m_calcedNoteListMasterApp  =     &m_calcedNoteListMaster;     //  Attach the  NoteList in   PitchPlayerApp
		sPitchCalcer->m_numberNeededToMatch    =     m_numberNeededToMatch;     //  BOTH mus be changed at exactly the same time
		sPitchCalcer->m_sizeOfPrimNotesCircque          =     m_sizeOfPrimNotesCircque;   
		sPitchCalcer->m_userFinalCircqueSizeTweak    =    	soundHelper.m_userFinalCircqueSizeTweak;
		sPitchCalcer->m_detectionSensitivityThreshold  =    m_detectionSensitivityThreshold;   // UNECESSARY ???   also gets set by  PsNavigatorDlg::WriteOut_SPitchCalc_Parms()   3/11


//		sPitchCalcer->m_useDFTrowProbeCircQue  =   m_usingCircQueDFTprobes;    ******  NEED this ????   9/4/2012  **************************  

	}
		
	return  true;
}



									/////////////////////////////////////////////////////


bool	 PitchPlayerApp::Peek_At_NoteList_Files_Header(    CalcedNoteListExternal&  compExternal,   CString&  listFilesPath,   short&  retFilesVersion,    CString&  retErrorMesg   )
{

	retErrorMesg.Empty();
	retFilesVersion =  -1;


	try
    {  CFile      file(    listFilesPath, 
	                           CFile::modeRead      //   |   CFile::shareExclusive 								
				  //	   |  CFile::typeBinary   ...BUGGY ????    5/02    Sets binary mode (used in derived classes only).
					     );



		unsigned char   retVersion;									//	A)    load  the   File-CREATOR	 TAG
		long                 retChunksSize;  

		if(    ! compExternal.Verify_FileCreator_Tag(   file,   retVersion,   retErrorMesg  )     )    //  Will tell if this is NOT a  "Creative Detector's file"
			return  false;

		retFilesVersion  =    retVersion;


																				//	B)    load  PART OF  the   'HEADER'


		if(   ! compExternal.Goto_Chunks_Header(   file,    External::NOTELISTfILEhEADERcHK,   retVersion,   retChunksSize,   retErrorMesg  )    )
			return  false;      //  it looks up the  'TAG's  identifying string'  (  "NtLstFil" for  NOTELISTfILEhEADERcHKPlayer ),  and descends into the File
								      //  until that STRING is found in the file.



//     HEADERsIntro2   ...could create/use a smaller  'template' STRUCT,  like I used in OLD PitchScope  [  HEADERsIntro  ]
		SPitchListHEADER   dstListHeaderStruct;
  
		long  ldSize  =    ( long )sizeof(  SPitchListHEADER  );     

		file.Read(   &dstListHeaderStruct,    ldSize  );



		/****  NO  PROJECT-files   for Navigator,  but I could get the  NoteList File    8/2012

		retProjectFilesPath =     dstListHeaderStruct.sibblingFilesPath;  
		if(    retProjectFilesPath.IsEmpty()    )
		{
			// ************************** BAD for  NoteLists without Projects   11/2011  ***************************************
			//					   ...see  UniEditorApp::On_Load_ObjectList_File()

			retErrorMesg  =  "PitchPlayerApp::Peek_At_NoteList_Files_Header  failed,  projectFilesPath is empty." ;   
			return  false;																																 
		}
		***/

	}
	catch(   CFileException   *pException   )
	{
		TCHAR    szCause[ 255 ];  
		CString   strRaw;
        pException->GetErrorMessage(  szCause,  255  );
		pException->Delete();
		strRaw =  szCause;

		retErrorMesg.Format(   "Can not open list-file because:  %s" ,    strRaw   );   //  Message might go to user.  9/06
		return  false;
	}

	return  true;
}


											////////////////////////////////////////


bool    PitchPlayerApp::Load_NoteList(   CString&  listFilesPath,    bool&  retUserCanceled,     CString&  retErrorMesg   ) 
{


	retErrorMesg.Empty();
	retUserCanceled =   false;
	SoundHelper&   soundHelper  =     GetSoundHelper();  


	m_calcedNoteListMaster.Empty();

	Set_DSTlist_Modified(  false );     //  If it is empty, then there is nothing to WARN the user to SAVE to file.  



	CString   retFileName;

	if(    ! External::Get_Files_Name_from_Path(   listFilesPath,    retFileName   )    )
	{
		ASSERT( 0 );
		retFileName =    listFilesPath;    // ***** Do I want this ????   12/11
	}




	CalcedNoteListExternal    compExternal;

		compExternal.m_calcedNoteList          =     &m_calcedNoteListMaster;   
		compExternal.m_lickList                     =     &m_lickList;
		compExternal.m_filesVersionNumber   =    Get_Files_Version_Number();    
//		compExternal.m_exesVersionNumber  =    Get_EXEs_Version_Number();   NOT used



	short    retFilesVersion; 


	if(    ! Peek_At_NoteList_Files_Header(    compExternal,    listFilesPath,     retFilesVersion,    retErrorMesg   )      )
	{

	//  return  false;     // ***** BAD to comment this out.  I still need to be BACKWARDLY COMPATIBLE with PitchScope2007  [  WANT to change this so it can load a Notelist  without a ProjectFile (Navigator created Notelist  3/11 )
								//           ...I need to be aware of this difficulty as I continue to develope new PitchScope apps.
								//
								//    FIX:   If Navigator fills out the Project  [ .m_projectFilesPath =  "C:\\NoProject.ppj" ] like this,  PS2007 users can load Navigator's Projectless Notelist    11/11 
								//   ...see  NoteGenerator::Save_Recorded_Notes_withDialog() 
		return  false;
	}




//  retFilesVersion =  13;   // *****************  TEMP,  DEBUG test for  fail  **********************


	if(    m_filesVersionNumber  <   retFilesVersion    )
	{
		CString   mesg;		
		mesg.Format(   "The NoteList File ( %s )  that you are trying to load is of a LATER VERSION[ %.1f ] that that of the Application[  %.1f  ].\n\n It may or may not load properly.\n Do you want to try and load it anyway?",
												                               retFileName,     (  (double)retFilesVersion) /10.0,  	  (  (double)m_exeVersionNumber ) /10.0   );		

		int  reslt    =      AfxMessageBox(  mesg,   MB_YESNO | MB_ICONQUESTION   );
		if(   reslt  !=   IDYES   )
		{
			retUserCanceled =   true;     //    NO,  is not an error   retErrorMesg =  "User canceled because of VERSION conflict."  ;
			return   true;
		}
	}



	try
    {  CFile      file(    listFilesPath, 
	                          CFile::modeRead      //   |   CFile::shareExclusive 								
				  //	   |  CFile::typeBinary   ...BUGGY ????    5/02    Sets binary mode (used in derived classes only).
					     );


		if(    ! compExternal.Receive(   file,   retErrorMesg  )    )	//  also allocates the  'AudioPLAYER'
		{

	//		Record_Loaded_ObjectFiles_Path(   emptyPath   );   // ***** WANT this ?????  *********   12/11
			m_lastObjectFilesPathLoaded.Empty();

			return  false;
		}
	}
	catch(   CFileException   *pException   )
	{

//		Record_Loaded_ObjectFiles_Path(   emptyPath   );   // ***** WANT this ?????  *********   12/11
		m_lastObjectFilesPathLoaded.Empty();


		TCHAR    szCause[ 255 ];  
		CString   strRaw;
        pException->GetErrorMessage(  szCause,  255  );
		pException->Delete();
		strRaw =  szCause;

//		retErrorMesg.Format(   "Could not load %sList file because:  %s" ,   retObjectName,   strRaw   );   //  Message might go to user.  1/04
		retErrorMesg.Format(   "Could not load      List file because:  %s" ,     strRaw   );  
		return  false;
	}



	m_lastObjectFilesPathLoaded =   listFilesPath;
									
	m_currentListFileName          =   retFileName;      //   save this for future   "SAVE AS"   dialog box's Initialization    6/12



	bool        retUserCanceledLocal;    
	CString   filePathNoDialog =   compExternal.m_sourceWAVpath;



	if(     ! Choose_SRC_File_for_Viewing(   retUserCanceledLocal,    filePathNoDialog,    retErrorMesg  )      )    //  will call   Query_User_For_SRCfile_Location() 
		return  false;



														//  Now that  ALL 2 Files are loaded,  make ASSIGNMENTS  based on File's settings


	BitSourceStreaming  *bitSourceStreaming    =      soundHelper.Get_BitSource_Streaming();
	if(                             bitSourceStreaming  ==  NULL   )
	{
		retErrorMesg =    "PitchPlayerApp::Load_NoteList  FAILED,   bitSourceStreaming is NULL." ; 
		return  false;	
	}


	SPitchCalc   *sPitchCalcer      =     bitSourceStreaming->m_sPitchCalc; 	
	if(                 sPitchCalcer  ==   NULL   )
	{
		retErrorMesg =    "PitchPlayerApp::Load_NoteList  FAILED,   SPitchCalc is NULL." ; 
		return  false;	
	}
		

	sPitchCalcer->m_musicalKey  =   compExternal.m_musicalKey;



	long  numberSwaps    =      sPitchCalcer->Sort_NoteList_by_TimePosition(   m_calcedNoteListMaster,   retErrorMesg  );
	if(     numberSwaps  > 0  )
	{
		ASSERT( 0 );  // This would mean that I am saving NoteLists in BAD FORM.    2/12   [  land here once  7/16/2012  
	}

	if(   ! retErrorMesg.IsEmpty()    )
		return  false;


//	Set_DSTlist_Modified(  false );    Moved above
	

//	if(    ! ReCreate_Child_ComponentViews_AllViewjs(  retErrorMesg  )     )
//		return  false;		
//	Notify_All_ComponentViews_forReDraw();


	return  true;
}



								/////////////////////////////////////////////////////


bool	 PitchPlayerApp::Save_DSTlist(    bool&  retUserCanceled,    CString&  retListFilesPath,     CString&  retErrorMesg   )
{

	
	short    midInstrumentNumber =  17;     // **** HARDWIRED *****


	SoundHelper&     soundHelper  =    GetSoundHelper();  
	EventMan&          eventMan  =      Get_EventMan();
	CString    mesg;


	retListFilesPath.Empty();


	SPitchCalc   *sPitchCalcer     =    eventMan.Get_SPitchCalc();
	if(                 sPitchCalcer  ==  NULL  )
	{  retErrorMesg =   "Load a music file." ;
		return  false;
	}


	if(        m_calcedNoteListMaster.Count()  <=  0  
		&&   m_lickList.Count()                       <=  0    )   // A NoteList could be EMPTY but still have some Phrases.  5/12
	{  
		retErrorMesg  =  "LOAD a Notelist,  or RECORD some Notes,  or SAVE some Phrases." ;
		return  false;
	}



	BitSourceStreaming   *bitSource  =  	soundHelper.Get_BitSource_Streaming();

	ASSERT(     bitSource  !=  NULL    );
	ASSERT(   ! m_sourceFilesPath.IsEmpty()    );    // ***********  SLOPPY,  want   Error Handling ???    3/11
	ASSERT(     midInstrumentNumber  >=  0   );



	
	long   noteCount  =    m_calcedNoteListMaster.Count();

	long   lickCount   =    m_lickList.Count();

	long	  sampleIdxFirstRecordedNote = 0,      lastProcessedSampleIdx =  0;


	if(    noteCount  >  0   )
	{
		long  swapCount     =       SPitchCalc::Sort_NoteList_by_TimePosition(   m_calcedNoteListMaster,   retErrorMesg  );  // since we blindly added the notes to the end of list  
		if(     swapCount  !=  0  )
		{
			ASSERT( 0 );    //    int  dummy =  0;    NEW,  2/12
		}


		MidiNote&  firstNoteInList      =    m_calcedNoteListMaster.Get( 0 );

		sampleIdxFirstRecordedNote =    firstNoteInList.beginingSampleIdxFile;  


		MidiNote&  lastNoteInList      =    m_calcedNoteListMaster.Get(   noteCount -1   );

		lastProcessedSampleIdx      =    lastNoteInList.endingSampleIdxFile;

	}   //   if(   noteCount > 0 




	long    totalOutputBytes  =     bitSource->Calc_Files_Total_Output_Bytes_With_SpeedExpansion();    //   m_totalSrcFilesBytes

	long    totalSamples       =     totalOutputBytes  /   bitSource->m_bytesPerSample;




	char      *curName =  NULL;
	bool        noNameWasPresent;

	if(     m_currentListFileName.IsEmpty()     )
		noNameWasPresent =    true;
	else
	{  curName                 =    m_currentListFileName.GetBuffer( 0 );
		noNameWasPresent =    false;
	}




	CString   filesExtension =    "pnl"  ;  
	CString   objectName   =    "Note"  ; 

	CString   strMesg;  
//	strMesg.Format(    "%s List Files (*.%s)|*.%s||" ,    objectName,   objectName,   filesExtension   );
	strMesg.Format(    "NoteList Files (*.%s)|*.%s||" ,                         filesExtension,   filesExtension   );



	CFileDialog    dlg(     FALSE,
								   _T( filesExtension ),      //   _T( "pnl" ),          pnl:     "Vox  Object List "
								   curName,   //   NULL, 
								   OFN_HIDEREADONLY   |    OFN_OVERWRITEPROMPT,
									_T(   strMesg  )		//   _T(    "VoxSep ObjectList Files (*.pnl)|*.pnl||" )   
							);

	LONG  nResult    =      dlg.DoModal();
	if(       nResult  !=  IDOK  )
	{  
		retUserCanceled  =  true;
		return  false;
	}

	retListFilesPath   =    dlg.GetPathName();  



	CString   retFilesName;  
	
	if(    !  External::Get_Files_Name_from_Path(   retListFilesPath,   retFilesName  )     )
	{
		ASSERT( 0 );
		m_currentListFileName.Empty();
	}
//	else
//		m_currentListFileName =  retFilesName;    Better down below.




	CalcedNoteListExternal    cNoteExternal;
																									//   Input the PARMS   for the  CalcedNoteListExternal  object
		cNoteExternal.m_calcedNoteList       =     &(  m_calcedNoteListMaster  );

		cNoteExternal.m_lickList                  =     &(  m_lickList  );

		cNoteExternal.m_totalSampleBytes  =     totalOutputBytes;   
 
		cNoteExternal.m_sourceWAVpath    =     m_sourceFilesPath;  

		cNoteExternal.m_musicalKey         =      sPitchCalcer->m_musicalKey;

		cNoteExternal.m_filesVersionNumber   =    Get_Files_Version_Number();    
//		cNoteExternal.m_exesVersionNumber  =    Get_EXEs_Version_Number();   //  not really used

		cNoteExternal.m_midiInstrumentNumber =    midInstrumentNumber;

		cNoteExternal.m_startOffsetDetectZone  =     sampleIdxFirstRecordedNote;   

		cNoteExternal.m_endOffsetDetectZone   =     lastProcessedSampleIdx;      //   m_curAudioPlayer->m_prevEndSamplePlay;   // **** DELAY a problem here too ?????
															                                                          //  test:     totalSamples  -1;      


//		cNoteExternal.m_projectFilesPath.Empty();    ***** BAD if empty, then notelist will NOT load to PitchScope2007   11/11   [ BEFORE:   now OK if empty,  had to change PitchScope load-code a little   3/11
//		cNoteExternal.m_projectFilesPath  =     "C:\\Unknown\\"    ;      **** FAILS, because PitchScope2007  will get the FileExtension wrong, and not let user BROWSE for the ProjkectFile


		cNoteExternal.m_projectFilesPath  =     "C:\\NoProject.ppj"    ;  //  ***** POOR, but acceptable.  It lets the user of PScope2007 load the NoteList without a ProjectFile,
																									//            but if the user can not BROWSE and load a Valid Project, unless the Project's name was NoProject.ppj   11/2011

//	   cNoteExternal.m_projectFilesPath  =   "C:\\Users\\JamesM\\VoxSep\\Projects\\LatestTest\\GreyMare_Proj.ppj"  ;  //   ***** DEBUG ONLY *********** 





	void  *extraData  =   NULL;    //   ****  Do I ever use this anymore ?????  *****    3/2011


	try
    {  CFile   file(    retListFilesPath, 
	                       CFile::modeCreate   |  CFile::modeWrite      	//   |   CFile::shareExclusive 		   
					//	   |  CFile::typeBinary     ...BUGGY ????    5/02    Sets binary mode (used in derived classes only).
					  );


		if(    ! cNoteExternal.Emit_NoteList(    file,    extraData,    retErrorMesg  )     )  
			return  false;

	}
	catch(   CFileException   *pException   )
	{
		TCHAR    szCause[ 255 ];  
		CString   strRaw;
        pException->GetErrorMessage(  szCause,  255  );
		pException->Delete();
		strRaw =  szCause;
		retErrorMesg.Format(   "PsNavigatorDlg::On_FileMenu_Save_Notelist FAILED,   could not save List file because %s." ,    strRaw  );
		return  false;
	}


	m_currentListFileName           =   retFilesName;      // Only assign if it was sucessully saved.

	m_lastObjectFilesPathLoaded  =   retListFilesPath;


	Set_DSTlist_Modified(  false );

	return  true;
}


										////////////////////////////////////////
										////////////////////////////////////////


bool   PitchPlayerApp::Record_Notes(   CString&  retErrorMesg   )
{


	SoundHelper&   soundHelper =    GetSoundHelper();
	EventMan&        eventMan     =    Get_EventMan();      
	
	retErrorMesg.Empty();


	BitSourceStreaming   *bitSource =    eventMan.Get_BitSourceStreaming();
	if(   bitSource  ==  NULL  ) 
	{
		retErrorMesg =  "PitchPlayerApp::Record_Notes  FAILED,   bitSource  is  NULL."   ;
		return  false;
	}
	

	long   lastFilePositionApp =     m_lastFilePosition;    //  Only good if   soundHelper.m_lastTransportDirection = CONTINUEfORWARD    12/11


	m_calcedNoteListTemp.Empty();       //   INIT



	NoteGenerator  *noteGenerator        =    bitSource->Allocate_NoteGenerator(   Get_Files_Version_Number(),     retErrorMesg   );
	if(                      noteGenerator  ==  NULL   )
	{
		retErrorMesg   =   "PitchPlayerApp::Record_Notes  FAILED,   noteGenerator  is  NULL."   ;
		return  false;		
	}


	if(   ! noteGenerator->Init_NoteGenerator(    &m_calcedNoteListTemp,   &m_calcedNoteListMaster,   &m_lickList,     bitSource->m_strWavFilePath,  
																							bitSource,   soundHelper.m_midiInstrumentPatch,   retErrorMesg  )    )
		return  false;
	


//	noteGenerator->m_sampleIdxRecordingStart =    lastFilePositionApp;      BAD,  is too late a figure.           NEW,  to detect if First aned Last Notes are PARTIAL, because on a boundary.  1/12
	noteGenerator->m_sampleIdxRecordingStart =    -1;    //  Initialize,   Process_Event_Notification_PPlayer() will assign this shortly.

	noteGenerator->m_sampleIdxRecordingEnd   =   -1;



	if(     ! soundHelper.Continue_Play_Forward(   m_playSpeedFlt,    false,    lastFilePositionApp,     retErrorMesg  )     )   //  'lastFilePositionApp' is a better estimate
		return  false;

	return  true;
}




					////////////////////////////////////////////////////////


ListMemry< LongIndexed >*    PitchPlayerApp::Make_Event_List(    long  startOffset,  long endOffset,     short stereoChannelCode, 
														                                           ListDoubLinkMemry<MidiNote> *noteList,    long&  retNoteCount,   CString&  retErrorMesg  )
{


						//   if either  startOffset   OR  endOffset  is less than zero,  do the entire notelist    

	retNoteCount =  -1;


	if(   noteList ==  NULL  )
	{
		ASSERT( 0 );
		retErrorMesg	 =   "Make_Event_List  FAILED,  noteList is NULL."  ;
		return  NULL;
	}



	bool   doEntireList;

	if(   startOffset < 0    ||    endOffset  < 0    ) 
		doEntireList =   true;
	else
		doEntireList =   false;



			//     ******CALLING FUNCTION  must delete this list when done !!!!  


	ListMemry< LongIndexed >*    nuList =      new   ListMemry< LongIndexed >();
	if(  nuList ==   NULL  )
	{
		ASSERT( 0 );
		retErrorMesg	 =   "Make_Event_List  FAILED,  could not alloc new ListMemry."  ;
		return  NULL;
	}
	
	nuList->Set_Dynamic_Flag(  true  );




//	long  changeCnt     =	   Sort_List_by_TimePosition(   retErrorMesg   );    //   important for later list searches 

	long	 changeCnt     =		SPitchCalc::Sort_NoteList_by_TimePosition(    *noteList,    retErrorMesg  );
	if(     changeCnt  <  0   )
	{	
		ASSERT( 0 );
		return false;
	}


	long   noteCount =  0;

//	ListIterator< ScalepitchSubject >     iter(    Get_ChildScalepitch_List()    );
	ListsDoubleLink<MidiNote>                  *startLink =     noteList->Get_Head_Link();  

	SpeedIndexDoubleIterator<MidiNote>    iter(   *noteList,   startLink  );  




	for(     iter.First();      !iter.Is_Done();      iter.Next()    )									
	{							

		MidiNote&  midiNote =     iter.Current_Item();


	//	if(        stereoChannelCode  ==  scalePitchObj.m_stereoChannelCode



		if(        doEntireList   
			  ||     (  midiNote.beginingSampleIdxFile  >=  startOffset      &&      midiNote.endingSampleIdxFile  <=  endOffset  )      )		  
		{

			LongIndexed   *nuLongIdx  =      new   LongIndexed();
			if(  nuLongIdx  ==  NULL   )
			{
				retErrorMesg	 =   "Make_Event_List  FAILED,  nuLongIdx is NULL."  ;
				return  NULL;
			}


			nuLongIdx->index    =    midiNote.scalePitch;

			nuLongIdx->value0  =    midiNote.beginingSampleIdxFile;
			nuLongIdx->value1  =    midiNote.endingSampleIdxFile;


			nuList->Add_Tail(  *nuLongIdx  );   
			noteCount++;
		}
	}


	retNoteCount =    noteCount;


	return  nuList;
}



											////////////////////////////////////////
											////////////////////////////////////////
											////////////////////////////////////////


bool    PitchPlayerApp::Make_NoteList_No_Audio(   CString&  srcFilesPath,   short  musicalKey,  CString&  retErrorMesg   ) 
{

			//   BELOW could be   Input PARMS  ( do I want a structure to hold them?  )
 


	//   ****** PERFORMANCE BUG: ********   If we use slowDown speeds,  make sure that I SKIP the expensive time slowing algos for the 16-bit wave
	//														...need some parmter for   WavConvert::Fetch_Current_Sample_and_Increment( )  to exclude slow down.  



			//  ***** Seems to be WORKING FINE !!!   11/25/11.   Tested it with  .WAV and Project  in  C:\\Users\\JamesM\\VoxSep\\Projects\\LatestTest\\ 
			//
			//			"C:\\Users\\JamesM\\VoxSep\\Projects\\LatestTest\\BWoman1.wav"  ;  


			//  Where does this function ultimately belong ??  Must be HERE !!!   
			//		(  It dynamically allocates  BitSource,  SPitchCalc,  WavConvert  ...so it could NOT be inside of onbe of those classes.			



	double   playSpeedFlt =    1.0 ;    //   1.0     CAN ADJUST,  is really a parm for  RESOLUTION  (  TESTED Vals:  1, 1.5, 2, 3, 4, 6, 8  )   7/2012




	double  primCircQueFilterWidthSpeedOne  =      7.5 ;    //   7.5   is DEFAULT,  see    PsNavigatorDlg::Set_SPitchCalcs_Detail_Threshold()               
//	double  primCircQueFilterWidthSpeedOne  =   15.75 ;   //    15.75     gives   8/15 at speed1




	short    detectionSensitivityThreshold  =   31;   //  31: default,  see   PsNavigatorDlg::Restore_Default_Settings_to_Controls( 



//	long     inputScalePercent  =      500;     //  500:  default,   see  SoundHelper::Set_VolumeBoost_Factor()    [  500,  1000,   1500,   etc.  step is 500
	long     inputScalePercent  =    1000;     //  500:  default,   see  SoundHelper::Set_VolumeBoost_Factor()    [  500,  1000,   1500,   etc.  step is 500




	long    topFreqFilterLimit      =       19500;         //    19500 always   [ see  SoundHelper::Execute_FreqFilter_Change()	
	long    bottomFreqFilterLimit =           20;         //     Full:  20      Middle:  300          High:  1200   [ skip bottom 12 dft rows ]        


	short    rightChannelPercent =     50;     //    50%    ******* ADJUST*****


	short    midiInstrumentPatch =  17;   // ********* HARDWIRED *********


	bool    isPlayingBackwards =   false;  // With some work,  I could detect in reverse ...MORE ACCURATE ???  
	bool    fowardFlag =  true;


//   ...The above variables look like the new   "DETECTION  PROFILE"   class




	long   wavsVolume  =    100;
	long   midiVolume   =    100;


	short    midInstrumentNumber =  17;     // ********* HARDWIRED ************

	long     numberOfEvents  =    	NUMPLAYeventsPitchNAVIGATOR;




	retErrorMesg.Empty();


	ListDoubLinkMemry< MidiNote >    localCalcedNoteList;       // *** BIG,  list is LOCAL,  not the one in PitchPlayerApp

	localCalcedNoteList.Set_Dynamic_Flag(  true  );    


	ListMemry< Lick >		 localLickList;				// ************   DUMMY Lick List,   just keep compiler happy  12/11  ***************
	localLickList.Set_Dynamic_Flag(  true  );    




	long  numberBytesIn16bitSample =   4;

	long  hemisEvent  =   ( numberOfEvents -1 )   / 2;   // The event at 180 degrees,    for Player is 10


	long  numberOfBytesInFetch         =     BLOCKLOCKEDSIZE;      //   44160

	long  numSamplesInBlockLocked  =     BLOCKLOCKEDSIZE  /   numberBytesIn16bitSample;    //  11,040  samples in  Fetch_Streaming_Samples_Direct() block-load


	long   samplesInDataFetchSpeedRedux  =   (long)(    (double)numSamplesInBlockLocked   /   playSpeedFlt   );


	
	long  totalSamplesInPiesliceSpeedOne  =     (long)(  TWOSECBYTES  /  (  numberOfEvents  -1 )   )   / 4;     //  [ 1104 ]  How many samples in an Event  
						                //  loads  44160 bytes ( 11,040 samples ) at a time.     11,040 / 10  hemisphereEvents =   1104  samples in  a PieSection    [ 3/11


	long  sampCountInPieSliceWithSpeedRedux  =    (long)(       (double)totalSamplesInPiesliceSpeedOne  /  playSpeedFlt    );




	
	bool   isAWAVfile =    External::Does_FileName_Have_Extension(  "wav",   srcFilesPath   );
	bool   isAMP3file  =    External::Does_FileName_Have_Extension(  "mp3",   srcFilesPath   );

	if(   ! isAWAVfile     &&   ! isAMP3file   )
	{
		retErrorMesg =  "Only pick a .WAV file or a .MP3 file." ;    // do not think this can happen
		return  false;
	}



	BitSourceStreaming   *bitSourceStreaming  =   NULL; 

	if(   isAWAVfile   )
	{
		bitSourceStreaming   =     new   BitSourceStreamingWAV();   
		ASSERT(   bitSourceStreaming  !=  NULL  );

		bitSourceStreaming->m_bitSourceKind =   	BitSource::SOURCEsTREAMwav;  
	}
	else
	{	bitSourceStreaming   =     new   BitSourceStreamingMP3();   
		ASSERT(   bitSourceStreaming  !=  NULL  );

		bitSourceStreaming->m_bitSourceKind =   	BitSource::SOURCEmp3;
	}

		
	bitSourceStreaming->m_wavVolumeSoundmanAddr  =      &wavsVolume;     //  the controls 
	bitSourceStreaming->m_midiVolumeSoundmanAddr  =      &midiVolume;     




	bitSourceStreaming->m_wavConvert       =        new   WavConvert();
	if(   bitSourceStreaming->m_wavConvert ==  NULL   )
	{
		ASSERT( 0 );
		retErrorMesg =   "Make_NoteList_No_Audio  FAILED,  could not alloc  WavConvert  for BitSource." ;
		return  false;
	}



	long   baseDelayDummy     =    -1;    //    -1,  disable the WAV delay    [ think it is disableled anyway, because this is only called from  NAVIGATOR.exe            
	long   overideDelayDummy =     0;

	bitSourceStreaming->m_wavConvert->m_baseDelayAddr      =     &(   baseDelayDummy   );  
	bitSourceStreaming->m_wavConvert->m_overideDelayAddr  =     &(   overideDelayDummy   );




	bool   retFileAcessError =  false;

	long   chunkSizeNav =       Get_ChunkSize_Navigator();




	if(   ! bitSourceStreaming->Initialize(   chunkSizeNav,   srcFilesPath,   retFileAcessError,   m_numberNeededToMatch,   &m_noteListHardwareDelayForPlayer,   
		                                                                                         m_usingCircQueDFTprobes,   retErrorMesg  )    )   // allocates  SPitchCalc  and  SndSample   
	{		
		if(    retFileAcessError   )
			retErrorMesg.Format(   "PitchPlayerApp::Make_NoteList_No_Audio  FAILED,  %s  could not be registered because:  %s."  ,  srcFilesPath,  retErrorMesg  ); 
		
//		Release_and_Close_SRCfile();   // ****  WANT this ????   NO,  but should do other cleanup.  3/11
		return  false;			
	}



	bitSourceStreaming->Release_WAV_CircularQue();   //  *** SLOPPY, but works...   Since we are in Navigator.exe it got AUTOMATICALLY allocated, but we do NOT need it.   12/10/11




	SPitchCalc   *sPitchCalcer      =     bitSourceStreaming->m_sPitchCalc; 	
	if(           sPitchCalcer  ==   NULL   )
	{
		retErrorMesg =    "PitchPlayerApp::Make_NoteList_No_Audio  FAILED,   SPitchCalc is NULL." ; 
		return  false;	
	}

	ASSERT(  sPitchCalcer->m_spitchCalcsSndSample  !=  NULL  );   // Should have been allocated in bitSourceStreaming->Initialize() within SPitchCalc::SPitchCalc()

	bitSourceStreaming->m_sPitchCalc->m_calcedNoteListMasterApp  =    &m_calcedNoteListTemp;      //  Finish odd  INITIALIZE

	bitSourceStreaming->m_parentFilePath           =    srcFilesPath;
	bitSourceStreaming->m_inputScalePercent      =    inputScalePercent;
	bitSourceStreaming->m_topFreqFilterLimit       =    topFreqFilterLimit;
	bitSourceStreaming->m_bottomFreqFilterLimit  =    bottomFreqFilterLimit;




	if(     ! bitSourceStreaming->m_wavConvert->Change_PlaySpeed(   playSpeedFlt,    m_computerPerformanceFactor,  m_numberNeededToMatch,   retErrorMesg  )    )
		return   false;																				 //  *** can get an ERROR if this is called BEFORE   BitSourceStreaming::Initialize()   1/12



																								//   Pick a SIZE for the MEDIAN Filter  

	double widthSpeedOneDecimal  =    primCircQueFilterWidthSpeedOne;      //     7.5   is default,     7.5  should give   4/7   for filter  at Slow1 

	short   retNumberNeededToMatch  =   -1;         //  BOTH must be changed at exactly the same time

	short   sizeOfNotesCircque  =    sPitchCalcer->Calc_Median_Filter_Ratio_by_Speed(   playSpeedFlt,   widthSpeedOneDecimal,   retNumberNeededToMatch   );  
																		



	sPitchCalcer->m_detectionSensitivityThreshold  =    detectionSensitivityThreshold;      //  30              
	sPitchCalcer->m_userFinalCircqueSizeTweak    =     0;   
	sPitchCalcer->m_playSpeedFlt                         =     playSpeedFlt;   
	sPitchCalcer->m_musicalKey                           =	    musicalKey;





	NoteGenerator  *noteGenerator        =    bitSourceStreaming->Allocate_NoteGenerator(   Get_Files_Version_Number(),    retErrorMesg  );
	if(                      noteGenerator  ==   NULL  )
	{
		retErrorMesg =    "PitchPlayerApp::Make_NoteList_No_Audio  FAILED,   NoteGenerator is NULL." ; 
		bitSourceStreaming->m_recordNotesNow =   false;   //  NOT necessary,  this  LOCAL BitSource   will be  deleted.
		return  false;	
	}

	if(    ! noteGenerator->Init_NoteGenerator(   &localCalcedNoteList,   &m_calcedNoteListMaster,    &localLickList,   srcFilesPath,   bitSourceStreaming, 
																															midiInstrumentPatch,   retErrorMesg  )   )  
		return  false;	
	



	BYTE    *destBufferBytes =   NULL;     //  Allocate DUMMY buffer for Audio, which is NOT used here

	destBufferBytes  =    new    BYTE[   BLOCKLOCKEDSIZE   ];      //    44160  bytes  for   11,040  samples
	if(  destBufferBytes  ==  NULL   )
	{
		retErrorMesg =    "PitchPlayerApp::Make_NoteList_No_Audio  FAILED,   destBufferBytes is  NULL ."  ; 
		return  false;	
	}




	long	 totalSamplesWAVsrc  =      bitSourceStreaming->Get_Files_Total_Samples_No_SpeedExpansion();
//	long	 totalSamplesWAVsrc  =     44160  *  240;      //    120[ 2 pages ]       240[4 pages]       360 sec[6+pages]      53 seconds[1 page ] 



	long   totalEvents                =      totalSamplesWAVsrc   /  sampCountInPieSliceWithSpeedRedux;  

	long   curSample =  0;  


	long   flushEvent =       ( 50 *  44160 )  / sampCountInPieSliceWithSpeedRedux    ;    //    15 sec:    599       ****  TEMP, DEBUG ****





	bitSourceStreaming->m_sampleIdxLastBlockLoadNotSlowExpanded =  0;  

	bitSourceStreaming->m_byteIndexToBufferPieSlices =  0;   //   **** CAREFUL with this initializie


	bitSourceStreaming->Initialize_For_Playing();   ///////////////// Event Initializer /////  Initialize_Pie_Clock();  //////////////////////////////////////






	for(    long  fakeEvent =0;      fakeEvent <  totalEvents;      fakeEvent++   )  
	{

		bool    retHitEOF =   false;


	    long   iEvent =     ( fakeEvent   %   (numberOfEvents -1)    );

		long   eventNum =   iEvent;    //  can be { 0 to 19 },  but reload with Load_Next_DataFetch_Forward() on  iEvent = 0 or 10

		if(    iEvent  >=   hemisEvent   )
			eventNum =    iEvent  -  hemisEvent;   
		else
			eventNum =   iEvent;    //  eventNum is { 0 thru 9 }   		



/******
//		if(    flushEvent  ==   fakeEvent   )    // *******  TEMP,   DEBUG **********
		if(    fakeEvent  <=    flushEvent    ) 
		{
			sPitchCalcer->Initialize_Notes_CircularQue();

			sPitchCalcer->Erase_logDFTmap_and_HarmPairsMap();   
		}
******/


		BYTE  *destBuffersPieSlicePtr   =   destBufferBytes          //  I do NOT use the data in this buffer...   this would be for AUDIO of SlowedDown samples     7/2012
			                         +    ( eventNum *  totalSamplesInPiesliceSpeedOne  *  numberBytesIn16bitSample  );  




		if(   !  bitSourceStreaming->Fetch_PieSlice_Samples(   totalSamplesInPiesliceSpeedOne,     //   'totalSamplesInPiesliceSpeedOne'  is in SlowedDown-Samples,  and  
			                                                                              destBuffersPieSlicePtr,                  //   Fetch_PieSlice_Samples will DECIMATE that sample count for the 
																						                 retHitEOF,   fowardFlag,  //  SndSample of detection, when calcing for SlowedDown-Speeds.    7/2012                       
													 						                                            rightChannelPercent,   0,  retErrorMesg   )    )
		{	if(   retHitEOF   )
			{
				break;
			}
			else if(  ! retErrorMesg.IsEmpty()    )
			{
				ASSERT( 0 );
				return  false;   
			}
		}
	
		curSample  =   ( bitSourceStreaming->m_sampleIdxLastBlockLoadNotSlowExpanded  -   samplesInDataFetchSpeedRedux  )   
				                                                              +      (   bitSourceStreaming->m_currentPieClockVal   *  sampCountInPieSliceWithSpeedRedux  );

		//  If at  SlowedDown of 2,   then sequence of  curSample   is  {  0,   552,   1104,   1656,   2208,  etc  }        7/2012





							//   Data in m_spitchCalc's SndSample is REFRESHED  on iEvent = { 0, 10 },  eventsOffset tells  Calc_logDFTs_Values() how far to offset into that SndSample.

		long    eventsOffset     =     (long)(   eventNum   *  sampCountInPieSliceWithSpeedRedux  );    //  eventNum is { 0 thru 9 }   ...10 pieSlices.					
 
		long    eventsOffsetAdj  =     (long)(    (double)eventsOffset   /   playSpeedFlt   );    // if using SlowedDown play speeds,  still use same SndSample for REGULAR speed (1)


		if(    ! sPitchCalcer->Calc_logDFTs_Values(    *( sPitchCalcer->m_spitchCalcsSndSample ),     eventsOffsetAdj,    retErrorMesg  )    )
			return  false;  			     
		





		bool              useNavigatorDelay  =   false;  //    false :    This means that  SPitchCalc does NOT need HorzVert-Offmaps to do its calc of pitch.
		CalcedNote    retCurrentNote; 
		short             preRollCode =  0;   // ******   OK ???  *******


		if(    ! sPitchCalcer->Estimate_ScalePitch(   curSample,   isPlayingBackwards,   retCurrentNote,   useNavigatorDelay,  fakeEvent,  preRollCode,  fakeEvent,   retErrorMesg  )    )    //  put the new note in the CircularQues   {  Smoothing, FinalForDelay  }
			return  false;  

				//	curScalePitch                 =      retCurrentNote.scalePitch;                    //   debug only
				//	curScalePitchDetectionScore   =      retCurrentNote.detectScoreHarms;              //   debug only




		long  measuredDelay  =    curSample  -   retCurrentNote.beginingSampleIdxFile;   //  this  CONFIRMS   SPitchCalc::Get_CurSampleIdx_Wierd_Delay_forPlay()
												//  ********  is always  6624,  even for SlowedSpeeds.   Is this OK ???  7/19/2012  [ see  Estimate_ScalePitch  ]  ********
												//   ( It might be that a bigger MEDIAN Filter would create an additional DELAY that I do NOT account for??  7/12 )



//		long  measuredDelayALT =    curSample  -   retCurrentNote.beginingSampleIdxFileALT;   
															//  speed 1:   11040  

		
//		retCurrentNote.beginingSampleIdxFile  =    retCurrentNote.beginingSampleIdxFileALT;  // ******   SEE what happens with new calc.  7/19  ******************  


// *********************  FIX this ******************************




		if(   ! noteGenerator->Record_Midi_Note(   retCurrentNote,    retErrorMesg  )    )     
		{   						
			bitSourceStreaming->m_recordNotesNow   =   false;
			AfxMessageBox(  retErrorMesg  );
		}
		



		if(    ! bitSourceStreaming->m_recordNotesNow    )
			break;    //  works OK.   The  for-loop can usually not complete,  the last iteration brings us here.  Usually we exit with this at   fakeEvent =  ( totalEvents -1 )
 

		bitSourceStreaming->Increment_Pie_Clock();    //  NEW,  a counter of Relative events   ****************************

	}   //   for(   fakeEvent =





	long    lastProcessedSampleIdx =    curSample;    //   ...like in DetectionZone         ***  OK to make this assumption ???    3/2011   ****

	bool   functSucess;



	if(    ! noteGenerator->Save_Recorded_Notes_withDialog(   lastProcessedSampleIdx,    retErrorMesg  )   )
		functSucess =  false;
	else
		functSucess =  true;



														 //  CLEANUP
	if(   destBufferBytes  !=  NULL   )       
		delete   destBufferBytes;

	if(   bitSourceStreaming  !=  NULL   )
		delete   bitSourceStreaming;					//   should release many dynamically allocated objects


	return  functSucess;
}











