/////////////////////////////////////////////////////////////////////////////
//
//  SequencerMidi.cpp   -   for Midi sequencer  
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


#include "Mmsystem.h"


//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"    
//////////////////////////////////////////////////     



#include "SequencerMidi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


SequencerMidi::SequencerMidi()
{
	
	m_midiIsOpen    =   false;

	m_curInstrument  =   0;      //  0:  Is this what WINDOWS will initialize as   ???
	m_curChannel =  0;			  //  0:  Is this what WINDOWS will initialize as   ???

	m_curPitch =  -1;    //  <0   means no note is currently playing


	m_overalVolume =  100;    //    in percent,  this value is set from Soundman from the Wave/Mid slider on the  Toolbox

	m_tickCount =   0;

	m_deviceID  =  -1;

	m_isTheInternalSynth =  false;

	m_silenceMidi =  false;
}



											////////////////////////////////////////


SequencerMidi::~SequencerMidi()
{

	CString   retErrorMesg;

	if(     ! Close_Midi_Device(  retErrorMesg  )     )    //  actually Soundman will EXPLICITLY close the Device in its destructor
		AfxMessageBox(  retErrorMesg  );
}



											////////////////////////////////////////


bool   SequencerMidi::Open_Midi_Device(    UINT  deviceID,    CString&   retErrorMesg   ) 
{

		//  ONLY CALLED by   SoundHelper::Alloc_Midi_Sequencer()    and    SoundHelper::Assign_and_Open_MidiDevice()       1/12


	retErrorMesg.Empty();


	if(   m_midiIsOpen   )
		return  true;


				//   midiOutGetNumDevs()  retrieves the number of MIDI output devices present in the system. 


	int   numDevs    =         midiOutGetNumDevs();     //   4
	if(    numDevs  ==  0   )
	{
		 retErrorMesg   =  "Open_Midi_Device failed,  no midi devices exist on this computer. Midi playback will not be possible." ;
		 return  false;
	}



	CString   extraErrorText,    retErrorText;


	UINT  res  =			midiOutOpen(     &m_midiOutDev,   //   LPHMIDIOUT   lphmo,          
														   deviceID,            
														   0,            //    DWORD  dwCallback,          
														   0,            //    DWORD  dwCallbackInstance,  
														    0    );     //    DWORD  dwFlags              
	if(   res   !=   MMSYSERR_NOERROR   )
	{
		 m_midiIsOpen =   false;

	//	 m_deviceID     =   -1;   ?????  Want this ????  1/12  *********************
		 

		 if(    Fetch_Midi_Error_Text(  res,   retErrorText  )     )
			retErrorMesg.Format(  "Open_Midi_Device FAILED,  midiOutOpen() could not open deviceID [%d ] because of error:\n %s [ %d ]",    deviceID,   retErrorText,    res );
		 else
			retErrorMesg.Format(  "Open_Midi_Device FAILED,  midiOutOpen() could not open deviceID [%d ] because of error[ %d ]",    deviceID,     res );

	//	AfxMessageBox(  retErrorMesg   );   // ***  TEMP,  DEBUG  ***
		return  false;
	} 
	

	m_midiIsOpen =   true;	

	m_deviceID    =    deviceID;    //  save this so we can test agains this in the future. 





	 CString             mesg2;
	 MIDIOUTCAPS   moc;


	 MMRESULT   mmr                  =                midiOutGetDevCaps(   deviceID,   &moc,   sizeof(moc)   );         // **** Do NOT need to do this.  *****
	 if(                mmr  !=  MMSYSERR_NOERROR   )
	 {
		 if(    Fetch_Midi_Error_Text(  mmr,   retErrorText  )     )
			mesg2.Format(   "Open_Midi_Device FAILED,  midiOutGetDevCaps could NOT access deviceID [%d] because of error:\n %s [ %d ]",  	deviceID,     retErrorText,    mmr );
		 else
			mesg2.Format(  "Open_Midi_Device FAILED,  midiOutGetDevCaps could NOT access deviceID [%d] because of error [ %d ].",    deviceID,     mmr );

		AfxMessageBox(   mesg2  );
    }
	else
	{
		/********
		typedef struct tagMIDIOUTCAPSA 
		{
			WORD    wMid;                  // manufacturer ID
			WORD    wPid;                  // product ID 
			MMVERSION vDriverVersion;      // version of the driver 
			CHAR    szPname[MAXPNAMELEN];  // product name (NULL terminated string) 
			WORD    wTechnology;           // type of device 

			WORD    wVoices;               // # of voices              (internal synth only) 
			WORD    wNotes;                // max # of notes        (internal synth only) 
			WORD    wChannelMask;          // channels used     (internal synth only) 

			DWORD   dwSupport;             // functionality supported by driver 

		} MIDIOUTCAPSA,        *PMIDIOUTCAPSA, *NPMIDIOUTCAPSA, *LPMIDIOUTCAPSA;


		wTechnology           // type of device     flags for wTechnology field of MIDIOUTCAPS structure 
				#define MOD_MIDIPORT    1   output port 
				#define MOD_SYNTH       2   generic internal synth 
				#define MOD_SQSYNTH     3   square wave internal synth
				#define MOD_FMSYNTH     4   FM internal synth 
				#define MOD_MAPPER      5   MIDI mapper 
				#define MOD_WAVETABLE   6   hardware wavetable synth 
				#define MOD_SWSYNTH     7   software synth 
		*********/


		switch(   moc.wTechnology    )    // *****************  I use BEFOREHAND in   SoundHelper::List_All_Midi_Devices(),  OMIT it here??      1/12    *****
		{
			case       MOD_MIDIPORT:     //	 1   output port 
				break;

			case       MOD_SYNTH:	 //	 2   generic internal synth 
				break;

			case       MOD_SQSYNTH:    //	   3   square wave internal synth
				break;

			case       MOD_FMSYNTH:    //	 4   FM internal synth 
				break;

			case       MOD_MAPPER:    //	 5   MIDI mapper 
				break;

			case       MOD_WAVETABLE:    //	  6   hardware wavetable synth 
				break;

			case       MOD_SWSYNTH:    //	  7   software synth 
				break;

			 default:              
				 break;    
		}

	}   //   got  midiOutGetDevCaps    OK




   /****   Have the CALLING functions do this.    1/2012    It's good not to put too much in this function because if it fails, I have a major mess. 

   if(    ! Initialize_New_Midi_Device(  channelID,   mesg1  )     )  
 
    *****/

   return  true;
}



											////////////////////////////////////////


bool   SequencerMidi::Initialize_New_Midi_Device(    short  channelID,     CString&   retErrorMesg    )
{


				//   Should be called after   SoundHelper::Alloc_Midi_Sequencer()   Not really sure how important this function is  1/2012 

	
	CString   mesg1,    composMesg,   retErrorText;   


	m_silenceMidi =  false;


	
	if(   ! Change_Channel(  channelID,   mesg1  )    )    //   This will call   StopPlaying_All_Channels()  
	{
		
		composMesg.Format(  "Initialize_New_Midi_Device had PROBLEMS Changing the Channel,  %s  " ,  mesg1   );

		AfxMessageBox(  composMesg  );     //  return  false;
	}




	if(    m_isTheInternalSynth    )   //   **** These functions FAIL when going to the Hammond XK-2 ( Error:  MMSYSERR_NOTSUPPORTED   "function isn't supported" 
	{


		DWORD  retVolume =  0x00000000;    //   Hammond Organ always fails for this function.  Do I need it ??   (  midiOutGetVolume  )     1/2012


   		MMRESULT   result       =            midiOutGetVolume(   m_midiOutDev,     &retVolume   );    //   0xFFFFFFFF   is full volume 
		if(               result  !=  MMSYSERR_NOERROR   )     
		{
			retErrorMesg.Format(    "Initialize_New_Midi_Device  FAILED,  midiOutGetVolume  returned error [ %d ]." ,   result   );
		//	return  false;
			AfxMessageBox(  retErrorMesg  );   // Not that important to fail,  just let the user know the problem.
		}


		

		DWORD  dwVolume  =  0xFFFFFFFF;    //  full volume


		MMRESULT   result2  =             midiOutSetVolume(  m_midiOutDev,   dwVolume  );    //  HAMMOND Organ always fails for this function.   (  midiOutGetVolume  )     1/2012
		if(                result2  !=   MMSYSERR_NOERROR   )   
		{
			if(     Fetch_Midi_Error_Text(   result2,  retErrorText   )      ) 
				composMesg.Format(    "midiOutSetVolume  had error [ %d ]:  %s (Initialize_New_Midi_Device)" ,   result2,   retErrorText   );
			else
				composMesg.Format(    "midiOutSetVolume  had error [ %d ]  (Initialize_New_Midi_Device)." ,   result2   );
		
			AfxMessageBox(  composMesg  );   //  //	return  false;            Not that important to fail,  just let the user know the problem.
		}

	}   //  if(   m_isTheInternalSynth

			
	return  true;
}



											////////////////////////////////////////


bool   SequencerMidi::Close_Midi_Device(    CString&   retErrorMesg   ) 
{

	                            //   if  input   'deviceID'    is less than zero,  then   'm_midiOutDev'    is used

	retErrorMesg.Empty();
	CString   errMesgLoc,   retErrorText;

	if(    ! m_midiIsOpen    )		//   already closed...    *****  CAREFULL   am I really sure that it was properly closed ???  1/12  ******
		return  true;



	MMRESULT     reslt        =             midiOutReset(  m_midiOutDev   );  
	if(                 reslt  !=  MMSYSERR_NOERROR  )
	{
		 if(    Fetch_Midi_Error_Text(  reslt,   retErrorText  )     )
			errMesgLoc.Format(   " midiOutReset  failed,  on deviceID [%d] because of error:\n %s [ %d ] (Close_Midi_Device)",    m_deviceID,     retErrorText,    reslt );
		 else
			errMesgLoc.Format(  "midiOutReset   failed,  on deviceID [%d] because of error [ %d ] (Close_Midi_Device).",    m_deviceID,     reslt );

//		return  false;   // ***************  NO return ??  Keep going and see if  it will still close *****************
		AfxMessageBox(  errMesgLoc   );
	}
	


	MMRESULT     reslt2                =                midiOutClose(  m_midiOutDev   );
	if(                reslt2  !=  MMSYSERR_NOERROR  )
	{
		 if(    Fetch_Midi_Error_Text(  reslt2,   retErrorText  )     )
			retErrorMesg.Format(   "midiOutClose  failed on deviceID [%d] because of error:\n %s [ %d ]  (Close_Midi_Device)",  m_deviceID,     retErrorText,    reslt2 );
		 else
			retErrorMesg.Format(  "midiOutClose  failed on deviceID [%d] because of error [ %d ]  (Close_Midi_Device).",  	m_deviceID,     reslt2  );

//		m_midiIsOpen =    true;	  It is already set at true
		return  false;
	}
	

	m_midiIsOpen =    false;	 //  Show the sucess of the function.

	return  true;
}


											//////////////////////////////////////////


bool    SequencerMidi::Fetch_Midi_Error_Text(   MMRESULT  errorCode,    CString&  retErrorText    )
{



/**********************************************  Looks like I could have used this.  But would it also do the System, MMSYSERR_  ,  as well?   1/12
UINT   midiOutGetErrorText(  MMRESULT mmrError,  
											  LPSTR lpText,       
											  UINT cchText   )        
);
************/



		if(         errorCode  ==   MIDIERR_NODEVICE   )       //  "No MIDI port was found. This error occurs only when the mapper is opened:
			retErrorText  =   "No MIDI port was found" ;   // 


/***
	#define MIDIERR_UNPREPARED    (MIDIERR_BASE + 0)      header not prepared 
	#define MIDIERR_STILLPLAYING  (MIDIERR_BASE + 1)    still something playing  
	#define MIDIERR_NOMAP         (MIDIERR_BASE + 2)        no configured instruments  
	#define MIDIERR_NOTREADY      (MIDIERR_BASE + 3)     hardware is still busy  
	#define MIDIERR_NODEVICE      (MIDIERR_BASE + 4)         port no longer connected 
	#define MIDIERR_INVALIDSETUP  (MIDIERR_BASE + 5)     invalid MIF 
	#define MIDIERR_BADOPENMODE   (MIDIERR_BASE + 6)    operation unsupported w/ open mode  
	#define MIDIERR_DONT_CONTINUE (MIDIERR_BASE + 7)    thru device 'eating' a message 
	#define MIDIERR_LASTERROR     (MIDIERR_BASE + 7)         last error in range
***/
		else if(   errorCode  ==   MIDIERR_NOTREADY  )
			retErrorText  =   "hardware is still busy." ;

		else if(   errorCode  ==  MIDIERR_BADOPENMODE   )
			retErrorText  =   "operation unsupported w/ open mode." ;

		else if(   errorCode  ==   MIDIERR_UNPREPARED  )
			retErrorText  =   "header not prepared." ;

		else if(   errorCode  ==   MIDIERR_NOMAP  )
			retErrorText  =   "no configured instruments." ;

		else if(   errorCode  ==   MIDIERR_DONT_CONTINUE  )
			retErrorText  =   " through device is eating-up a message ." ;

		else if(   errorCode  ==   MIDIERR_INVALIDSETUP  )
			retErrorText  =   "invalid MIF." ;

		else if(   errorCode  ==  MIDIERR_STILLPLAYING   )
			retErrorText  =   "still something playing." ;

		else if(   errorCode  ==  MIDIERR_LASTERROR   )
			retErrorText  =   "last error in range." ;


/***
	#define MMSYSERR_ERROR        (MMSYSERR_BASE + 1)   unspecified error 
	#define MMSYSERR_BADDEVICEID  (MMSYSERR_BASE + 2)   device ID out of range 
	#define MMSYSERR_NOTENABLED   (MMSYSERR_BASE + 3)   driver failed enable 
	#define MMSYSERR_ALLOCATED    (MMSYSERR_BASE + 4)   device already allocated 
	#define MMSYSERR_INVALHANDLE  (MMSYSERR_BASE + 5)   device handle is invalid
	#define MMSYSERR_NODRIVER     (MMSYSERR_BASE + 6)   no device driver present 
	#define MMSYSERR_NOMEM        (MMSYSERR_BASE + 7)   memory allocation error
	#define MMSYSERR_NOTSUPPORTED (MMSYSERR_BASE + 8)   function isn't supported 
	#define MMSYSERR_BADERRNUM    (MMSYSERR_BASE + 9)   error value out of range 
	#define MMSYSERR_INVALFLAG    (MMSYSERR_BASE + 10)  invalid flag passed 
	#define MMSYSERR_INVALPARAM   (MMSYSERR_BASE + 11)  invalid parameter passed 
	#define MMSYSERR_HANDLEBUSY   (MMSYSERR_BASE + 12)  handle being used 
****/
		else if(   errorCode  ==   MMSYSERR_ERROR  )				
			retErrorText  =   "unspecified error" ;

		else if(   errorCode  ==   MMSYSERR_BADDEVICEID  )
			retErrorText  =   "the specified device identifier is out of range" ;

		else if(   errorCode  ==  MMSYSERR_NOTENABLED   )				
			retErrorText  =   "driver failed enable" ;

		else if(   errorCode  ==   MMSYSERR_ALLOCATED  )				//  *************  MultiMedia  SYSTEM messages.  are they OK ??? **************
			retErrorText  =   "the device already allocated " ;

		else if(   errorCode   ==    MMSYSERR_INVALHANDLE   )
			retErrorText   =  "the specified device handle is invalid" ;

		else if(   errorCode  ==  MMSYSERR_NODRIVER   )				
			retErrorText  =   "no device driver present " ;

		else if(   errorCode  ==  MMSYSERR_NOMEM   )
			retErrorText  =   "the system is unable to allocate or lock memory" ;

		else if(   errorCode  ==  MMSYSERR_NOTSUPPORTED    )				
			retErrorText  =   "the function is not supported" ;

		else if(   errorCode  ==  MMSYSERR_BADERRNUM   )				
			retErrorText  =   "error value out of range" ;

		else if(   errorCode  ==  MMSYSERR_INVALFLAG   )				
			retErrorText  =   "invalid flag passed" ;

		else if(   errorCode  ==   MMSYSERR_INVALPARAM  )
			retErrorText  =    "the specified pointer or structure is invalid --  invalid parameter passed";                //    invalid parameter passed          "The specified pointer or structure is invalid." ;

		else if(   errorCode  ==   MMSYSERR_HANDLEBUSY   )				
			retErrorText  =   "handle being used" ;

		else
		{  retErrorText  =   "Unknown error jm." ;

			return  false;   // ********
		}

	return  true;
}



											//////////////////////////////////////////


void   SequencerMidi::Delay_MilliSeconds(   long  miliSecs   )
{

		   	//   Now takes TRUE  Milliseconds   8/2012


	long   goal,   curClock;        	//  MOVE to   StdUtils.cpp  in folder  \Common


	goal =   miliSecs  +  clock();     


	/***   from    clock()

	The elapsed wall-clock time since the start of the process (elapsed time in seconds times CLOCKS_PER_SEC). 
	If the amount of elapsed time is unavailable, the function returns –1, cast as a clock_t.

	The clock function tells how much time the calling process has used. A timer tick is approximately equal to 1/CLOCKS_PER_SEC second. 

	CLOCKS_PER_SEC  =    1000     ...so one tick is    1 millisecond    1/1000

	***/

	do
	{  curClock =    clock(); 

	}while(   goal  >  curClock  );
}


											////////////////////////////////////////


bool	 SequencerMidi::Change_Instrument(   short  patchID,   CString&   retErrorMesg   )
{

	/***
		short    clarinet =    71;

	short    sax      =    64;      //  64 -  67  

	short    violin     =    40;
	short    viola      =    41;
	short    cello    =      42;
	short    strings   =    48;

	short     guitarOverdrive    =    29;
	short     guitarDistort         =    30;

	short     reedOrgan   =    20;      //  16 - 18,   20
	short     harpsicord   =     6;

	short     oboe    =      68;
	short     fiddle    =    110;
	***/


	retErrorMesg.Empty();

	CString  retErrorText;


	if(    patchID  < 0      ||     patchID  > 127   )
	{
		retErrorMesg   =  "SequencerMidi::Change_Instrument  failed,   incorect input parm." ;
		return  false;
	}


	if(   ! m_midiIsOpen   )
	{
		retErrorMesg   =  "SequencerMidi::Change_Instrument  failed,   no MIDI device is open." ;
		return  false;
	}


	if(     ! StopPlaying_All_Channels(  retErrorMesg  )      )
		return  false;




	DWORD   programChange  =    0x0000C0;    //  always

	DWORD   patchInstrument  =      (   (DWORD)patchID  )   <<  8;       //   0x006800;	   	//  68 is hex for  104

	DWORD   midiMesg =     patchInstrument    +    programChange    +    m_curChannel;  
	


	MMRESULT   reslt              =               midiOutShortMsg(   m_midiOutDev,   midiMesg   );
	if(               reslt  !=  MMSYSERR_NOERROR )
	{

	//	retErrorMesg   =  "SequencerMidi::Change_Instrument  failed,   midiOutShortMsg." ;

		 if(    Fetch_Midi_Error_Text(  reslt,   retErrorText  )     )
			retErrorMesg.Format(   "midiOutShortMsg  failed on deviceID [%d] because of error:\n %s [ %d ]  (Change_Instrument)",  	m_deviceID,     retErrorText,    reslt );
		 else
			retErrorMesg.Format(  "midiOutShortMsg  failed on deviceID [%d] because of error [ %d ]  (Change_Instrument).",  	m_deviceID,    reslt );

		return  false;
	}


	m_curInstrument  =    patchID;

	return  true;
}



											////////////////////////////////////////


bool	 SequencerMidi::Change_Channel(   short  channelID,   CString&   retErrorMesg   )
{

	retErrorMesg.Empty();

	if(    channelID  < 0      ||     channelID  > 127   )
	{
		retErrorMesg   =  "SequencerMidi::Change_Channel  failed,   incorect input parm." ;
		return  false;
	}


	if(     ! StopPlaying_All_Channels(  retErrorMesg  )      )
		return  false;


	m_curChannel  =    channelID;

	return  true;
}


											////////////////////////////////////////


bool	 SequencerMidi::StopPlaying_All_Channels(  CString&   retErrorMesg   )
{


	retErrorMesg.Empty();

	CString   retErrorText;


	if(    ! m_midiIsOpen    )
		return  true;


				//   The  midiOutReset   function turns off all notes on all MIDI channels for the specified MIDI output device.


	MMRESULT  reslt            =              midiOutReset(  m_midiOutDev   );
	if(              reslt  !=  MMSYSERR_NOERROR    )
	{

	//	retErrorMesg.Format(   "SequencerMidi::StopPlaying_All_Channels  failed,  midiOutReset  [ %d ]." ,    reslt   );


		 if(    Fetch_Midi_Error_Text(  reslt,   retErrorText  )     )
			retErrorMesg.Format(  "midiOutReset  failed on deviceID [%d] because of error:\n %s [ %d ]  (StopPlaying_All_Channels)",  	m_deviceID,     retErrorText,    reslt );
		 else
			retErrorMesg.Format(  "midiOutReset  failed on deviceID [%d] because of error [ %d ]  (StopPlaying_All_Channels).",   m_deviceID,     reslt  );
		return  false;
	}
	else
		return  true;
}



											////////////////////////////////////////
											////////////////////////////////////////


bool	 SequencerMidi::Start_Play_curNote(   short  pitchID,    short  velocity,   bool  overideToOrigVolume,    CString&   retErrorMesg    )
{


			//  Now used soundman to control the Midi voluem too.


	retErrorMesg.Empty();


	if(   ! m_midiIsOpen   )
	{
		retErrorMesg   =  "SequencerMidi::Play_Note  failed,   no MIDI device is open." ;
		return  false;
	}



	if(   m_curPitch  >=  0   )					//  last note is  UNEXPECTEDLY  still playing,  so turn all notes off
	{
	//  if(     ! StopPlaying_All_Channels(  retErrorMesg  )     )
		if(     ! Stop_Play_curNote(            retErrorMesg   )     )       // *** Is this BETTER  ????  *****
			return  false;

		TRACE( "Start_Play_curNote unexpected \n" );
	}



	if(       pitchID  < 0     
  //	||   pitchID  > 127				***INSTALL   upper limit  *****
		)
	{
		retErrorMesg   =  "SequencerMidi::Play_Note  failed,   incorect pitch input parm." ;
		return  false;
	}											//	 Lowest E   on Guitar  is  Midi  52 


	if(    velocity  < 0      ||     velocity  > 127   )
	{
		retErrorMesg   =  "SequencerMidi::Change_Channel  failed,   incorrect velocity input parm." ;
		return  false;
	}



	CString  retErrorText;
	short   adjVelocity;

	if(    overideToOrigVolume    )   //  In the  Midi-Only AuditionMode we play at the original voluem, and are not affected by the WAV/Midi slider
		adjVelocity  =   velocity;
	else
		adjVelocity  =     (short)(       ((double)m_overalVolume   / 100.0 )    *   (double)velocity         );    //  6/07


	ASSERT(   adjVelocity  >=  0   &&    adjVelocity  <=  127   );
 
// ****************************  is 127 OK????   1/10   ***************************



	DWORD   noteOn      =      0x000090; 
	DWORD   noteOff      =      0x000080; 

	DWORD   pitchVal     =    ( (DWORD)pitchID )         <<    8;
	DWORD   velocityVal =    ( (DWORD)adjVelocity )   <<   16;
	
	//  DWORD   pitch          =      0x003C00;      //   3C  is Middle c 
	//   DWORD   velocityOn =      0x600000;      //   40  is  64

		/***
			NoteOn  =  0x9x   x is channel
			MiddleC  =  0x3C
			Velocity of  64 =  NoteOn =   0x40

		   midiMesg  =   0x403C90    +   midiChannel;
		***/

	DWORD   midiMesg  =     velocityVal   +    pitchVal    +   noteOn    +    m_curChannel;  


	MMRESULT    reslt   =         midiOutShortMsg(   m_midiOutDev,    midiMesg   );
	if(  reslt  !=  MMSYSERR_NOERROR )
	{

	//	retErrorMesg   =  "SequencerMidi::Play_Note  failed,   midiOutShortMsg." ;

	    if(    Fetch_Midi_Error_Text(  reslt,   retErrorText  )     )
			retErrorMesg.Format(  "midiOutShortMsg  failed on deviceID [%d] because of error:\n %s [ %d ]  (Play_Note)",  	m_deviceID,     retErrorText,    reslt );
	    else
			retErrorMesg.Format(  "midiOutShortMsg  failed on deviceID [%d] because of error [ %d ]  (Play_Note).",    m_deviceID,    reslt );

		return  false;
	}
	

	m_curPitch =    pitchID;      //   record our CURRENT playing pitch for later turnOFF

	return  true;
}




											////////////////////////////////////////


bool	 SequencerMidi::Stop_Play_curNote(   CString&   retErrorMesg    )
{


	retErrorMesg.Empty();

	if(   ! m_midiIsOpen   )
	{
		retErrorMesg   =  "SequencerMidi::Stop_Play_curNote  failed,   no MIDI device is open." ;
		return  false;
	}


	if(   m_curPitch   < 0   )		//   UNEXPECTED,  says none is playing   ...sp PLAY SAFE:   turn all notes off
	{
		if(     ! StopPlaying_All_Channels(  retErrorMesg  )     )
			return  false;
		else
			return  true;
	}


		/***
			NoteOff   =   0x8x   x is channel         [  	NoteOn   =   0x9x   x is channel

			MiddleC  =   0x3C
			Velocity of  0 =  NoteOff =   0x00

			midiMesg  =   0x003C90    +   midiChannel;
		***/

	CString   retErrorText;

	DWORD	noteOff      =     0x000080;   
	DWORD   velocityOff =     0x000000;  

	DWORD   pitchVal    =    ( (DWORD)m_curPitch )   <<   8;

	DWORD	midiMesg   =     velocityOff    +    pitchVal    +    noteOff    +    m_curChannel;  



	MMRESULT      reslt    =           midiOutShortMsg(   m_midiOutDev,    midiMesg   );
	if(  reslt  !=  MMSYSERR_NOERROR )
	{

	//	retErrorMesg   =  "SequencerMidi::Stop_Play_curNote  failed,   midiOutShortMsg." ;

	    if(    Fetch_Midi_Error_Text(  reslt,   retErrorText  )     )
			retErrorMesg.Format(   "midiOutShortMsg  failed on deviceID [%d] because of error:\n %s [ %d ]  (Stop_Play_curNote)",  m_deviceID,     retErrorText,    reslt );
	    else
			retErrorMesg.Format(   "midiOutShortMsg  failed on deviceID [%d] because of error [ %d ]  (Stop_Play_curNote).",  	m_deviceID,    reslt );


		m_curPitch =  -1;   // **** ????   WANT to assume that the note is NOT playing ??? *****    1/03
		return  false;
	}
	else
	{	m_curPitch =  -1;     //   Flag that no note is CURRENTLY playing
		 return  true;
	}
}



											////////////////////////////////////////


bool	SequencerMidi::Play_Note_wDuration(   short  pitchID,   short  velocity,   long  duration,    CString&   retErrorMesg    )
{

				//    duration  =     noteWidthInPixels ( or PieSlices )    *    PsNavigatorDlg::m_midiPlayDurationFactor  


	retErrorMesg.Empty();


	if(   ! m_midiIsOpen   )
	{
		retErrorMesg   =  "SequencerMidi::Play_Note_wDuration  failed,   no MIDI device is open." ;
		return  false;
	}


	if(   m_curPitch  >=  0   )					//  last note is  UNEXPECTEDLY  still playing,  so turn all notes off
	{
	//  if(     ! StopPlaying_All_Channels(  retErrorMesg  )     )
		if(     ! Stop_Play_curNote(            retErrorMesg   )     )       // *** Is this BETTER  ????  *****
			return  false;
	}


	if(       pitchID  < 0     
  //	||   pitchID  > 127				***INSTALL   upper limit  *****
		)
	{  retErrorMesg   =  "SequencerMidi::Play_Note_wDuration  failed,   incorect pitch input parm." ;
		return  false;
	}											//	 Lowest E   on Guitar  is  Midi  52 



	if(    velocity  < 0      ||     velocity  > 127   )   // **** ???  is 127 the maximum for  Velocity ??? 
	{
		retErrorMesg   =  "SequencerMidi::Change_Channel  failed,   incorect velocity input parm." ;
		return  false;
	}




	bool  overideToOrigVolume =  true;   //  Always for this function ????   6/07


	if(    ! Start_Play_curNote(   pitchID,    velocity,   overideToOrigVolume,   retErrorMesg   )     )
	{
		m_curPitch =   -1;      //   record our CURRENT playing pitch for later turnOFF
		return  false;
	}


												//   Pause code execution for the duration of the note

	Delay_MilliSeconds(  duration  );       //   now calling function INPUTS in TRUE MiliSeconds     8/2012 




													
	if(    ! Stop_Play_curNote(  retErrorMesg  )     )
	{
		m_curPitch =   -1;     //  might not set this if there is an error ??? 
		return  false;
	}
	else
		return  true;
}


