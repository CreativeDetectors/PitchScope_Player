/////////////////////////////////////////////////////////////////////////////
//
//  SequencerMidi.h   -   
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_SEQUENCERMIDI_H__30CC326C_2AFD_11D7_9A48_00036D156F73__INCLUDED_)
#define AFX_SEQUENCERMIDI_H__30CC326C_2AFD_11D7_9A48_00036D156F73__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class    SequencerMidi  
{

public:
	SequencerMidi();
	virtual ~SequencerMidi();



	static			void		Delay_MilliSeconds(   long  miliSecs   );    //   Play_Silent_Delay()  better name ??



	bool				Fetch_Midi_Error_Text(   MMRESULT  errorCode,    CString&  retErrorText    );     //   NEW,  use it   1/2012


	bool				Open_Midi_Device(    UINT  deviceID,    CString&   retErrorMesg   );	
	bool				Close_Midi_Device(             CString&   retErrorMesg   );

	bool				Initialize_New_Midi_Device(    	short  channelID,     CString&   retErrorMesg    );






	bool				StopPlaying_All_Channels(  CString&   retErrorMesg   );

	bool				Change_Instrument(   short  patchID,       CString&   retErrorMesg   );
	bool				Change_Channel(       short  channelID,    CString&   retErrorMesg   );


	bool				Start_Play_curNote(   short  pitchID,     short  velocity,   bool  overideToOrigVolume,    CString&   retErrorMesg   );
	bool				Stop_Play_curNote(    CString&   retErrorMesg    );


	bool				Play_Note_wDuration(   short  pitchID,   short  velocity,  long  duration,    CString&   retErrorMesg  );


	bool				MMCapsDetailMidiOut(   int  nDevId,     CString&   retErrorMesg   ); 





public:	
	short          m_overalVolume;  


	HMIDIOUT       m_midiOutDev;


	long           m_deviceID;   //  this was used in  Open_Midi_Device()


	bool		   m_isTheInternalSynth;      //   "Microsoft GS Wavetable Synth"



	bool	       m_midiIsOpen;  

	DWORD       m_curInstrument;
	DWORD       m_curChannel;


	short          m_curPitch;     //  what is the Midi pitch number for the currently playing note [ <0   means no note is currently playing  ]


	long          m_tickCount;    //  for debug


	bool		  m_silenceMidi;      //  if true,  then user does NOT hear Midi  
};




////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_SEQUENCERMIDI_H__30CC326C_2AFD_11D7_9A48_00036D156F73__INCLUDED_)
