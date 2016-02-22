//   PitchGauge.cpp
//
//  Copyright (C) 2007   Creative Detectors (of San Francisco, California).  All Rights Reserved.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include  "stdafx.h"



#include   "..\comnFacade\UniEditorAppsGlobals.h"



#include  "..\comnFacade\VoxAppsGlobals.h"

//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"      //  #include <FixMath.h>    ...Mac's version of Fixedpoint 

#include  "..\ComnGrafix\CommonGrafix.h"      //  #include "commonmy.h"   		
#include  "..\ComnGrafix\OffMap.h"  
#include  "..\comnGrafix\TransformMap.h"

#include  "..\ComnGrafix\AnimeStream.h"



#include   "..\ComnAudio\TransMapAdmin.h"



#include   "..\ComnGrafix\AnimePacket.h"
#include   "..\ComnGrafix\AnimeAdmin.h"

#include  "..\comnGrafix\FrameArtist.h"
#include  "..\comnGrafix\AudioFrameartist.h"


//#include  "..\comnAudio\SequencerMidi.h"		
#include  "..\comnAudio\FundamentalCandidate.h"
//////////////////////////////////////////////////     

//#include   "..\ComnAudio\BeatTheory.h"

#include   "..\comnAudio\StaffChannel.h"





///////////////////////////							 ...INTERFACE
#include  "..\comnInterface\gnStatevar.h"

#include  "..\comnCatalog\External.h"
#include  "..\comnInterface\gnUniBasic.h"

#include  "..\comnInterface\ClipBoardj.h"  


#include  "..\comnInterface\gnVuCues.h"	 

#include  "..\comnInterface\gnPane.h"   
#include  "..\ComnGrafix\PaneMemory.h"

#include  "..\comnInterface\gnView.h"



#include   "..\comnInterface\ListComponentSubject.h"      
#include   "..\comnInterface\ListComponentView.h"

#include  "..\comnInterface\gnProperty.h"    
#include    "..\comnInterface\gnManipulators.h"
#include  "..\comnInterface\gnCommands.h"


#include   "..\comnFacade\UniEditorApp.h"





#include  "..\comnInterface\gnEditor.h"   	

//#include  "..\comnCatalog\TransformMapSubject.h"

#include   "..\comnCatalog\ScalepitchSubject.h"
#include   "..\comnCatalog\ScalepitchlistSubject.h"
#include   "..\comnAudio\DetectZone.h"

#include   "..\comnCatalog\ChannelDetectionSubject.h"
///////////////////////////


#include   "..\comnCatalog\AnimatedSubjects.h"







#include  "PitchGauge.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////



void    SolidGray_Rect_Frame(   RectShrt rct,   void *CDevC,   short val,    short  penWidth   );

void     Xor_Line(     short x0,  short y0,     short x1,  short y1,     short  penWidth,   void  *CDevC  );   // new,  could OPTIMIZE !!! 

void     Paint_Line(   short x0,  short y0,     short x1,  short y1,       short red,  short green,  short  blue,  short  penWidth,   void  *CDevC  );


void     Get_ScalePitch_LetterName(  short  sclPitch,   char *firLet,  char *secLet  );





////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


PitchGauge::PitchGauge(   PointFIX &upLft,    PointFIX &botRgt,     short  leftBorderFixed,      short   topBorderFixed,		
																			                    short  rightBorderFixed,    short  bottomBorderFixed,   
															long  fundamentalMapHeight,    Viewj&  viewj )   
									  :   Pane(   false,    upLft,  botRgt,      leftBorderFixed,       topBorderFixed,
																							rightBorderFixed,    bottomBorderFixed,
																							512,     1   ),    //  512:  horzScaleDown[ chunkSize,  HARDWIRED  
											m_fundamentalMapHeight(  fundamentalMapHeight  ),    m_viewj( viewj )
{  
		//   leftBorderFixed,  topBorderFixed :    For  PREVIOUS behavior( autoSizing ) set these <0,  if positive vals then
		//														  that 'fixed' value is used for these borders.

	m_hasFundamentalHeight  =    false;

	m_hideText =   false;

	m_channelDetectionSubject =   NULL;
}


										////////////////////////////////////////


PitchGauge::~PitchGauge()
{
}



										////////////////////////////////////////


void    PitchGauge::Render_Animation_Frame(   long  curSample,    AnimePacketAbstr&  animePacketAbs,   bool  staticMode,   
																														Viewj&  vw,     CDC  *cdc   )																															  
{

				//  CALLED  by  Pane::Draw()


	short   textSpaceDivisor =  4;     //   4 or 3   ...can adjust [  also see  PitchGauge::Paint_Letter_and_Dash()



	if(   ! staticMode   )    //   Do NOT render during Animation( it does NOT change with new samples 
		return;



	AnimePacket  *animePacket		=    dynamic_cast<  AnimePacket*  >(  &animePacketAbs  );     //  down cast,  returns NULL if fail   
	if(   animePacket  ==  NULL   )
	{  
		ASSERT(0);
		return;
	}



	CString          retErrorMesg;
	UniWindow   *window  =      vw.Get_Platforms_Window();                  
	CDC            *vuCDC  =      vw.Get_Views_CDC();         

	if(		  window   ==   NULL   
		||   vuCDC ==   NULL    )
	{
		int  dummyBreak =  9;     //   ASSERT(0);   ..For this too??      "Landing here for 3D volume   ...BUT might not be a big deal,
											  //                     11/03                            Happens from end of an Animation(  Animate_All_Views()  on last frame when player->SopPlaying has been called )
		return;
	}

	if(    Get_Subject()  ==  NULL    )
	{
		ASSERT(0);
		return;
	}

	if(   m_optionalFont  ==  NULL   )
	{
		ASSERT(0);
		return;
	}


	Pane  *paneDSTlist  =      m_viewj.Get_DSTlist_Pane();    // ** the SIBBLING Pane that the Gauge must respond to( magUp, Offset  )
	if(        paneDSTlist  ==  NULL  )
	{
		ASSERT(0);
		return;
	}








	Graphic&   graphicPane  =    Get_Graphic();      //  in  ComponentVIEW,  not Subject

	RectLong   r1(   graphicPane.m_offset.X,                           
						   graphicPane.m_offset.Y,   
                           graphicPane.m_offset.X    +    graphicPane.m_width    -1,    
						   graphicPane.m_offset.Y    +    graphicPane.m_height   -1    ); 
                 
	SolidGray_Rect_Frame(   r1,   vuCDC,    0,    1   );   //  Temp just to put on screen if no transform


	if(   m_hideText   )
		return;					// **ESCAPE**    ...User doe NOT want text on this   'Dummy'  Gauge




																	//   Calc number of Channels.   
	long  channelCnt =   12;

	if(    m_hasFundamentalHeight    )
		channelCnt =     m_fundamentalMapHeight;


	if(          ( 2 *  channelCnt )   >   graphicPane.m_height   
		   &&   paneDSTlist->Get_Vertical_Magnification()  ==  1      )      
		return;					//    ESCAPE,  LineDraw and Letters  if it is too small  






	CFont   *oldFont   =       vuCDC->SelectObject(   m_optionalFont   );
	if(          oldFont ==  NULL  )
	{ 
		ASSERT( 0 );   //     retErrorMesg =   "PitchGauge::Render_Animation_Frame  failed,  SelectObject." ;
		return;
	}


	COLORREF  textColor   =    RGB(   255,   255,    255   );
	COLORREF  backColor  =    RGB(     0,        0,       0   );

	vuCDC->SetTextColor(   textColor  );       // also want the  TextBoundBox to have the right backround color
	vuCDC->SetBkColor(      backColor  );       



	CSize   sz =      vuCDC->GetTextExtent(  "E"  );    
	long          textHeight   =     sz.cy; 
	ASSERT(   textHeight  > 0   );

	short    retLastLettersYcoord =    sz.cy / textSpaceDivisor;    // ***  OK to init to like this ???   8/06 *********




	for(     long  channelIdx= 0;      channelIdx <  channelCnt;      channelIdx++     )
	{

		if(    ! Paint_Letter_and_Dash(   channelIdx,    *vuCDC,   vw,   retLastLettersYcoord,   retErrorMesg  )     )
		{

			if(   oldFont  !=  NULL  )
			{
				CFont  *optionalFont  =    vuCDC->SelectObject(  oldFont  );
			}

			ASSERT( 0 );
			return;
		}
	} 

	



	if(   oldFont  !=  NULL  )
	{
		CFont  *optionalFont    =        vuCDC->SelectObject(   oldFont   );
		if(         optionalFont == NULL )
		{  
			ASSERT( 0 );	 //     retErrorMesg =   "PitchGauge::Render_Animation_Frame  failed,  SelectObject on oldFont." ;
			return;
		}
	}
	else
	   ASSERT( 0 );		  //     retErrorMesg =   "PitchGauge::Render_Animation_Frame  failed,  lost oldFont." ;
}





										////////////////////////////////////////


bool    PitchGauge::Paint_Letter_and_Dash(   long  channelIdx,     CDC&  vuCDC,      Viewj&  vw, 
																                             short&  retLastLettersYcoord,    CString&  retErrorMesg  )
{


			//   'transMap'  might be NULL,   if no detectZone( just a render of Amplitudes  )  


	short   textSpaceDivisor =  4;     //   4 or 3   ...can adjust [  also see  PitchGauge::Render_Animation_Frame()




	retErrorMesg.Empty();


	Pane  *paneDriving     =        m_viewj.Get_DSTlist_Pane();    // ** the SIBBLING Pane that the Gauge must respond to( magUp, Offset  )
	if(        paneDriving  ==  NULL  )
	{
		ASSERT(0);
		retErrorMesg =   "PitchGauge::Paint_Letter_and_Dash failed,   paneDriving is NULL" ;
		return  false;
	}


	DrivingListView  *drivingVw  =     dynamic_cast<  DrivingListView*  >(  paneDriving  );     //  down cast,  returns NULL if fail   
	if(   drivingVw  ==   NULL   )
	{
		ASSERT( 0 );
		retErrorMesg =   "PitchGauge::Paint_Letter_and_Dash failed,   drivingVw is NULL" ;
		return  false;
	}


	Graphic&   graphicPane =    Get_Graphic();      //  in  ComponentVIEW,  not Subject


	long  channelCnt =   12;

	if(    m_hasFundamentalHeight    )
		channelCnt =     m_fundamentalMapHeight;


	short   pitch =      ( channelCnt -1 )    -   (short)channelIdx;     //   INVERTING  y-coords  for MS-bitmaps ??? 


	short   x0 =     graphicPane.m_offset.X     +      (      (graphicPane.m_width     * 2 ) /3      );	
	short   x1 =     graphicPane.m_offset.X     +      (        graphicPane.m_width    -1               );






																		//  Get some data about the  'SIBBLING Pane'(  DrivingListView  ) 

	short   backMapCode  =	   paneDriving->Get_BackgroundMap_Code(); 
	long	worldHeight     =	   paneDriving->Get_WorldGrid_Height(  backMapCode   );	


	long  yTop        =  	 TransMapAdmin::Get_Channels_Top_Yval(         (short)channelIdx,          backMapCode   );
//	long  yBot         =      TransMapAdmin::Get_Channels_Bottom_Yval(    (short)channelIdx,         backMapCode   );   Buggy...

	long  yTopNext  =	   TransMapAdmin::Get_Channels_Top_Yval(         (short)channelIdx +1,     backMapCode   );


	long  yTopInv        =    ( worldHeight  - 1 )     -   yTop;
	long  yTopNextInv =    ( worldHeight  - 1 )     -   yTopNext;


	PointLong   pt(          0,    yTopInv          ); 
	PointLong   ptNext(   0,     yTopNextInv   ); 

	paneDriving->WorldGrid_2Viewj(    pt,           vw   );     // Ape the same logic used in rendering the ScalePitchSubjects
	paneDriving->WorldGrid_2Viewj(    ptNext,    vw   );     //  ( only call on DrivingPane, PitchGauge-pane might be buggy.  8/06  )

	ASSERT(    ptNext.Y   >=     pt.Y   );

	short  y =     pt.Y      +      (  ptNext.Y  -  pt.Y ) /2;




	if(              y  >= 1     
		    &&     y  <   ( graphicPane.m_offset.Y  +  graphicPane.m_height )     )    
		Paint_Line(    x0,  y,     x1,  y,      255,  255,  255,     1,     &vuCDC    );





	char        firLet,  secLet;
	CString   scaleNoteName;

	if(    m_hasFundamentalHeight    )
	{
		short  scalePitchIdx =     (  pitch  %  12  );       //   cause we start with 'E'

		Get_ScalePitch_LetterName(    scalePitchIdx,    &firLet,  &secLet    );
	}
	else
		Get_ScalePitch_LetterName(    pitch,    &firLet,  &secLet    );
			

	scaleNoteName.Format(   "%c%c" ,   firLet,  secLet  );



	CSize   sz =      vuCDC.GetTextExtent(  scaleNoteName  );      //  Text ORIGIN is in UpperLeft

	int     yOffsetText   =     y   -    ( sz.cy /2 ); 




	bool  hasRoom;

	if(      (  y  -  ( sz.cy / textSpaceDivisor )  )    >=    retLastLettersYcoord     )
		hasRoom =   true;
	else  
		hasRoom =   false;



	if(    hasRoom    )
	{
		if(          (y  +  sz.cy)   <    ( graphicPane.m_offset.Y   +   graphicPane.m_height  )    	//  do not let the last letter write outside the BoudBox
			   &&  (y   -  sz.cy)   >       graphicPane.m_offset.Y   )
			vuCDC.TextOut(    4,    yOffsetText,     scaleNoteName    );


		retLastLettersYcoord  =    (short)(  y   +   sz.cy  );
	}		
	

	return  true;
}








