//					PitchGauge.h
//
//  Copyright (C) 2007   Creative Detectors (of San Francisco, California).  All Rights Reserved.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_PITCHGAUGE_H__arf__INCLUDED_)
#define AFX_PITCHGAUGE_H__arf__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////////



class  AnimePacket;
class  ChannelDetectionSubject;
class  Viewj;


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////



class   PitchGauge      :  public   Pane       //    ->  ComponentView
{

	//     does NOT have a MemoryBitmap   ...draws directly on screen.  Different COORD-conversion necessary( inverted DIB's )


public:
	PitchGauge(    PointFIX &upLft,    PointFIX &botRgt,    short  leftBorderFixed,      short   topBorderFixed,		
																			     short  rightBorderFixed,    short  bottomBorderFixed,
										long  fundamentalMapHeight,     Viewj&   viewj    );
	virtual ~PitchGauge();




	virtual			void					 Render_Animation_Frame(   long  curSample,     AnimePacketAbstr&  animePacket,   
																				          bool  staticMode,    Viewj&  vw,    CDC  *cdc   );																															  

 //   virtual		void					Draw(   Viewj&  vw   );	  ****NO,  use  Render_Animation_Frame() for custom renders	




	virtual     ComponentView*        Get_Hit_ComponentView(    Eventj&  evt    )    {  return  NULL;  }   
																									//   Denies Selection for this object




protected:
	virtual     bool			Paint_Letter_and_Dash(   long  channelIdx,     CDC&  vuCDC,     Viewj&  vw, 
																		                short&   retLastLettersYcoord,    CString&  retErrorMesg  );





public:
	bool		m_hasFundamentalHeight;

	long        m_fundamentalMapHeight;

	bool        m_hideText;


	ChannelDetectionSubject   *m_channelDetectionSubject;      //  It is like its Subject  5/07

	Viewj&      m_viewj; 
};







////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_PITCHGAUGE_H__arf__INCLUDED_)
