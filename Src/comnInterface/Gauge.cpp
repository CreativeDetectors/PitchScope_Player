/////////////////////////////////////////////////////////////////////////////
//
//  Gauge.cpp   -   
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#include "StdAfx.h"




#include  <math.h>     //  for trig functions


#include   "..\comnFacade\UniEditorAppsGlobals.h"    // OK here ???    12/11


#include  "..\comnFacade\VoxAppsGlobals.h"



//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"     
#include  "..\ComnGrafix\CommonGrafix.h"      

#include  "..\comnGrafix\OffMap.h"  


#include  "..\ComnAudio\CalcNote.h"

#include  "..\ComnAudio\SPitchCalc.h"


#include  "..\comnGrafix\FontsCd.h" 



#include "..\comnInterface\Gauge.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////


void     Xor_box_Outline(   RectShrt  rct,   void  *CDevC,   short  penWidth     );


void     Rectangle_Fill(   RectShrt rct,    short redFill,  short greenFill,  short  blueFill,    CDC&  devContext   );

void     Fill_SolidGrey_Rect(    RectShrt&  rect,    short  greyVal,     CDC&  dc    );   //   NEW
void     Fill_SolidColor_Rect(    RectShrt&  rect,    COLORREF  clr,     CDC&  dc    ); 


void     Draw_SolidGray_Oval(      RectShrt  rct,    void *CDevC,    short val    );

void     Draw_OutlineGrey_Oval(   RectShrt  rct,    void *CDevC,    short val,     short  penWidth    );

void     Draw_Solid_Colored_Oval(   RectShrt  rct,    void *CDevC,    short red,  short green,  short  blue  );   //  NEW




//   void     Blit_Offmap_2PCwindow(   OffMap *offMap,   void  *CDevC,      RectShrt&  srcRect,    RectShrt&  dstRect   );






////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


GaugeAnimated::GaugeAnimated(void)
{

	m_width =  m_height  =  m_xOffset  =  m_yOffset  =  -1;     

	m_channelCount =  0;

	m_backgroundGray =  0;    //  0 :  Default,  Black


	m_currentVisibleBulletIdx =  -1;


	m_orientation =  -1;              //   0 :  Horizontal,     1 : Vertical 

	m_bulletsShapeCode =  0;    //    0:   Rectangle      1:  Oval
}


											////////////////////////////////////////


GaugeAnimated::~GaugeAnimated(void)
{
}

											////////////////////////////////////////


bool	 GaugeAnimated::Font_Is_Installed()
{
	if(   m_fontCustom.m_height > 0   )
		return  true;
	else
		return  false;
}

											////////////////////////////////////////


void	  GaugeAnimated::Render_Gauges_Background(  CDC& dc  )
{


	RectShrt   rctPane(      m_xOffset,    //  UpperLeft              
						             m_yOffset,   
							  m_xOffset   +  m_width,      //  lowerRight
							  m_yOffset   +  m_height    ); 



	Fill_SolidGrey_Rect(   rctPane,   m_backgroundGray,    dc   );   //  *****  TEMP  DISABLE,  when want to test the  BULLETS Rendering and ERASE functions.  2/12



	m_currentVisibleBulletIdx =  -1;

	Mark_Bullets_Invisible();   //  Reset all the MemberVars in the BulletList to show that they are invisible
}



											////////////////////////////////////////


void	  GaugeAnimated::Render_Gauges_Bullet(   CDC& dc,   short  radialPosition,    short  scalePitch,    COLORREF  color,    short  specialCode   )
{

							//   COULD  automatically ERASE the current Bullet,  BUT it might LIMIT POSSIBILITIES in future  


	ListIterator< Bullet >    iter(   Get_BulletList()   );      


	for(    iter.First();     ! iter.Is_Done();     iter.Next()    )
	{  		

		Bullet&   bullet =    iter.Current_Item();  


		if(     bullet.m_positionID  ==   radialPosition   )       //   We must first find a BULLET in the right POSITION,  and 
		{																            //   then  'scalePitch'  will describe the TEXT for that bullet.     2/12

			bullet.Draw(  dc,  color,   scalePitch,   specialCode  ); 


			m_currentVisibleBulletIdx  =   radialPosition;   
			break;
		}  
	}
}


											////////////////////////////////////////


void	  GaugeAnimated::Erase_Current_Bullet(  CDC& dc  )
{


	if(   m_currentVisibleBulletIdx  >=  0   )
	{

		ListIterator< Bullet >    iter(   Get_BulletList()   );      

		for(    iter.First();     ! iter.Is_Done();     iter.Next()    )
		{  		
			Bullet&   bullet =    iter.Current_Item();  

			if(          bullet.m_positionID  ==   m_currentVisibleBulletIdx  )
			{
				bullet.Hide( dc );

				m_currentVisibleBulletIdx =  -8;
				break;
			}  
		}
	}
}


											////////////////////////////////////////


void	  GaugeAnimated::Erase_All_Bullets(  CDC& dc  )
{

	ListIterator< Bullet >    iter(   Get_BulletList()   );      


	for(    iter.First();     ! iter.Is_Done();     iter.Next()    )
	{  		
		Bullet&   bullet =    iter.Current_Item();  

		bullet.Hide( dc );
	}
}

											////////////////////////////////////////


void	  GaugeAnimated::Mark_Bullets_Invisible()
{

					//   Just resets the MemberVars,  does NO Graphic Operation

	ListIterator< Bullet >    iter(   Get_BulletList()   );      


	for(    iter.First();     ! iter.Is_Done();     iter.Next()    )
	{  		
		Bullet&   bullet =    iter.Current_Item();  

		bullet.m_visible =  false;
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


Bullet::Bullet(void)
{

	 m_visible =  false; 

	 m_positionID =    -999;    //   scalePitch

	 m_shapeCode =   0;

	 m_backgroundGray =  0;

	 m_fontPtr =   0;

	 m_orientationOfGauge =  -1;

//	 m_graphic   ****** caller must initialize  *****
}



											////////////////////////////////////////

bool	  Bullet::Font_Is_Installed()
{

	if(    m_fontPtr  ==   NULL   )
		return  false;


	if(    m_fontPtr->m_height  > 0   )
		return  true;
	else
		return  false;
}

											////////////////////////////////////////


void   Bullet::Get_BoundBox(   RectShrt&  rect   )
{  

	RectLong  rectLg;

	m_graphic.Get_BoundBox(  rectLg  );  

	rect =   rectLg;
}  
  

											////////////////////////////////////////


void   Bullet::Hide(  CDC& dc  )
{ 


    if(   ! m_visible   )        // ********************   CAREFUL  2/21/2012    ************************************
		return;		


	short  greyVal  =    m_backgroundGray;

	RectShrt   rect;
	Get_BoundBox(   rect   );


//	Rectangle_Fill(             rect,     0, 0, 0,      dc  );   // bad dimensions

	Fill_SolidGrey_Rect(   rect,   greyVal,    dc   );  // ******  Previously had a BAD BUG with this,  but is now fixed.   2/21/2012  ********


	m_visible =   false;      //  **** NOT needed ???  
}





////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


GaugeDrivingview::GaugeDrivingview(void)
{

  m_channelCount =  12;

  m_orientation =  1;

//	 m_graphic   ****** caller must initialize  *****
}


											////////////////////////////////////////

GaugeDrivingview::~GaugeDrivingview(void)
{
}

											////////////////////////////////////////


void	  GaugeDrivingview::Initialize_BulletList()
{

	if(       m_orientation      < 0  
		||   m_channelCount  < 0   )
	{
		ASSERT( 0 );
		return;
	}


	m_bulletList.Empty();



	for(   short  sPitch =0;     sPitch <  m_channelCount;      sPitch++   )
	{

		BulletDrivingview   *nuBullet =     new   BulletDrivingview();

			nuBullet->m_positionID                       =     sPitch;
			nuBullet->m_shapeCode          =     m_bulletsShapeCode;
			nuBullet->m_backgroundGray  =     m_backgroundGray;
			nuBullet->m_fontPtr                =    &m_fontCustom;
			nuBullet->m_orientationOfGauge =   m_orientation;


		if(   m_orientation ==   0   )    //  0:  Horizontal
		{
 
			short  channelWidth =     m_width  /  m_channelCount;  

			nuBullet->m_graphic.m_width   =   channelWidth;
			nuBullet->m_graphic.m_height =    m_height;


			short  xOffset  =    sPitch   *  channelWidth;

			nuBullet->m_graphic.m_offset.X  =     xOffset  +    m_xOffset;         //        relative to BoundBox o containing  Gauge  
			nuBullet->m_graphic.m_offset.Y  =             0   +    m_yOffset;    

		}
		else if(   m_orientation ==   1  )    //  1:  Vertical
		{

			short  channelWidth =     m_height  /  m_channelCount;

			nuBullet->m_graphic.m_width   =   m_width;
			nuBullet->m_graphic.m_height =    channelWidth;


			short  yOffset  =   ( 11  -  sPitch )   *  channelWidth;

			nuBullet->m_graphic.m_offset.X  =             0   +    m_xOffset;       
			nuBullet->m_graphic.m_offset.Y  =     yOffset   +    m_yOffset; 
		}  
		else
		{	ASSERT( 0 );    }
		

		m_bulletList.Add_Tail(  *nuBullet   );

	}   //   for(   sPitch =0    
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
GaugeRevolver::GaugeRevolver(void)
{

  m_channelCount =  12;

  m_orientation =  0;   //   0:   Horizontal,   always for Revolvers

//	 m_graphic   ****** caller must initialize  *****
}


											////////////////////////////////////////

GaugeRevolver::~GaugeRevolver(void)
{
}


											////////////////////////////////////////


void	  GaugeRevolver::Initialize_BulletList()
{

	if(       m_orientation      < 0  
		||   m_channelCount  < 0   )
	{
		ASSERT( 0 );
		return;
	}


	m_bulletList.Empty();



	for(   short  sPitch =0;     sPitch <  m_channelCount;      sPitch++   )
	{

		BulletRevolver   *nuBullet =     new   BulletRevolver();

			nuBullet->m_positionID                       =      sPitch;
			nuBullet->m_shapeCode          =      1;      //  m_bulletsShapeCode;
			nuBullet->m_backgroundGray  =      m_backgroundGray;
			nuBullet->m_fontPtr                =     &m_fontCustom;
			nuBullet->m_orientationOfGauge =    m_orientation;


		//////////////////////////////
		short   quadrantPos =   sPitch;    //  This is the Radial Position of the Bullet,  if the MusicalKey is other than E,  then this bullet will show text tht is NOT "E"


		ASSERT(    quadrantPos  >=  0   &&     quadrantPos  <  12   );

		short      x, y,      xCenter=  m_width/2,     yCenter =   m_height/2;
		short      numMapChannels  =   12;          

		double    radiusHorz =    (double)m_width  / 3.0;   // ***ADJUST ***
		double    radiusVert =    (double)m_height  / 3.0;  // ***ADJUST ***

		short     bulletDiaHorz  =      ( m_width * 10)   /  55;       //  60   ****** ADJUST   Ratio ******
		short     bulletDiaVert  =      ( m_height * 10)   /  55;       //  60   ****** ADJUST   Ratio ******


		short   rotatedPixVal =     quadrantPos  +  (  numMapChannels / 4 );   //  rotate by 90 deg so  E (Primary)  is at circles BOTTOM

		if(    rotatedPixVal  >=  numMapChannels   )			//  keep in range  { 0 - 11 }
			rotatedPixVal  =     rotatedPixVal  -  numMapChannels;  

		ASSERT(    rotatedPixVal >=  0     &&     rotatedPixVal  <  numMapChannels     );


		double  angle =     (   (double)rotatedPixVal   /    (double)numMapChannels   )     *    6.283185307179586;      //    twoPI

		x =     (short)(    cos( angle )   *  radiusHorz   );          // angle is in  RADIAN S
		y =     (short)(    sin( angle )    *  radiusVert   );

		short   xOvalCenter  =      xCenter  +  x;
		short   yOvalCenter  =      yCenter  +  y;

		RectShrt   rct(      ( xOvalCenter    -   (bulletDiaHorz/2)  )    + m_xOffset,    //  UpperLeft              
								  ( yOvalCenter    -   (bulletDiaVert/2)  )     + m_yOffset,   

								  ( xOvalCenter   +   (bulletDiaHorz/2)  )     + m_xOffset,      //  lowerRight
								  ( yOvalCenter   +   (bulletDiaVert/2)  )     + m_yOffset     ); 
		//////////////////////////////
		

		nuBullet->m_graphic.m_width   =   rct.Get_Width();
		nuBullet->m_graphic.m_height =    rct.Get_Height();

		nuBullet->m_graphic.m_offset.X  =     rct.left;
		nuBullet->m_graphic.m_offset.Y  =     rct.top;



		m_bulletList.Add_Tail(  *nuBullet   );

	}   //   for(   sPitch =0    
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


BulletDrivingview::BulletDrivingview(void)
{

	 m_visible =  false; 

	 m_positionID =    -999;    //   scalePitch

	 m_shapeCode =   0;

	 m_backgroundGray =  0;

	 m_fontPtr =   0;


//	 m_graphic   ****** caller must initialize  *****
}




											////////////////////////////////////////


void	   BulletDrivingview::Draw(   CDC& dc,    COLORREF  color,    short  scalePitch,   short  specialCode  )     
{   


	short  userPrefMusicalKeyAccidentals  =   specialCode;   //  ********* CLUMSY    ***************************************


//	if(   color  ==     0                       )   
	if(   color  ==  m_backgroundGray  )    //
	{
		return;   // ******  OK ???  *****************************
	}




	CString      retErrorMesg;   
	RectShrt    rect;
	Get_BoundBox(   rect   );


//   COLORREF    crColor;     crColor  =    RGB(   redFill,    greenFill,   blueFill    );

	short  red    =    GetRValue(  color  );
	short  green =    GetGValue(  color  );
	short  blue    =   GetBValue(  color  );


	if(          m_shapeCode  ==   0   )
		Fill_SolidColor_Rect(            rect,   color,   dc    ); 
	else if(   m_shapeCode  ==   1    )  
		Draw_Solid_Colored_Oval(   rect,    &dc,     red, green,  blue   ); 
	else
	{	ASSERT( 0 );   }



																	//   Now write the TEXT,  if a FontsCd  is allocated    2/12
	long   xDest = -1,     yDest = -1;

	short   widthBoundBox  =    rect.Get_Width()   -1;     //  15   for  Narrow
	short   heightBoundBox =    rect.Get_Height()  -1;




//	short   rightwardNudge  =  8;         // **************   ADJUST *************************

//	short   leftwardNudge    =  0;      //             **************   ADJUST *************




	if(    Font_Is_Installed()    )   
	{		

		short   scalePitchTextIdx =   scalePitch;

		if(    userPrefMusicalKeyAccidentals ==  3   )    //  3:  Use NUMERALS,  which rely on the POSITION of the Bullet, and doesn't care about what the pitch really is.
			scalePitchTextIdx  =   m_positionID;


		long	charWidth =	  m_fontPtr->Get_Characters_Width(   scalePitchTextIdx,    userPrefMusicalKeyAccidentals   );



//  DrivingView

		if(         m_orientationOfGauge ==   0   )    //   0:  HORIZONTAL  gauge  (  of a  'Vertical'  DrivingView   )   ...like the Narrow DriveView
		{

			xDest  =     rect.left   +    widthBoundBox/2   -   (  charWidth /2  /* +  leftwardNudge  */ );               // **************   ADJUST *************************


	//		yDest  =     rect.top    +                                       m_fontPtr->m_height /2;
			yDest  =     rect.top    +    heightBoundBox/2   -    m_fontPtr->m_height /2;
		}
		else if(   m_orientationOfGauge ==   1   )    //   1: VERTICAL  gauge   (  of a  'Horizontal'  DrivingView   )
		{

	
		//	xDest  =     rect.left    +     rightwardNudge;
			xDest  =     rect.left    +     widthBoundBox/2   -   charWidth /2 ;  


	//		yDest  =     rect.top    +     m_fontPtr->m_height /3;
			yDest  =     rect.top    +     heightBoundBox/2   -   m_fontPtr->m_height /2;
		}
		else
		{	ASSERT( 0 );   }



		if(    ! m_fontPtr->Blit_Musical_Notes_NameText(    scalePitchTextIdx,   userPrefMusicalKeyAccidentals,   xDest, yDest,   false,   dc,   retErrorMesg  )    )
		{
			AfxMessageBox( retErrorMesg  );
		}
	}


	m_visible =   true;
}  



////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


BulletRevolver::BulletRevolver(void)
{

	 m_visible =  false; 

	 m_positionID =    -999;    //   scalePitch

	 m_shapeCode =   1;    //   1:  Oval

	 m_backgroundGray =  0;

	 m_fontPtr =   0;

	 m_orientationOfGauge =  0;  //  0 :  Horizontal,  always for Revolvers
}



											////////////////////////////////////////


void	   BulletRevolver::Draw(   CDC& dc,    COLORREF  color,    short  scalePitch,    short  specialCode  )     
{   


	short  userPrefMusicalKeyAccidentals  =   specialCode;   //  ********* CLUMSY    ***************************************


//	if(   color  ==     0                       )   
	if(   color  ==  m_backgroundGray  )    //
	{
		return;   // ******  OK ???  *****************************
	}



	CString      retErrorMesg;   
	RectShrt    rect;
	Get_BoundBox(   rect   );


//   COLORREF    crColor;     crColor  =    RGB(   redFill,    greenFill,   blueFill    );

	short  red    =    GetRValue(  color  );
	short  green =    GetGValue(  color  );
	short  blue    =   GetBValue(  color  );


	if(          m_shapeCode  ==   0   )
		Fill_SolidColor_Rect(            rect,   color,   dc    ); 
	else if(   m_shapeCode  ==   1    )  
		Draw_Solid_Colored_Oval(   rect,    &dc,     red, green,  blue   ); 
	else
	{	ASSERT( 0 );   }



																	//   Now write the TEXT,  if a FontsCd  is allocated    2/12
	long   xDest = -1,     yDest = -1;


	short   widthBoundBox  =    rect.Get_Width()   -1;
	short   heightBoundBox =    rect.Get_Height()  -1;





	bool   testSystemFonts  =   false;   //   false:   RELEASE          [   Must COORDINATE with    PsNavigatorDlg::Render_Revolvers_Bullet_ToOffmap()   ]


	if(    testSystemFonts    )
	{
		short   charWidth  =  20;
		short   charHeight =  20;

		xDest  =     rect.left    +    widthBoundBox/2    -   charWidth /2 ;              
		yDest  =     rect.top    +    heightBoundBox/2   -   charHeight /2;

		CString    retScaleNoteName;
		SPitchCalc::Get_ScalePitch_LetterName_NEW(   scalePitch,    userPrefMusicalKeyAccidentals,   retScaleNoteName  );


	//	CSize   sz =      dc.GetTextExtent(  retScaleNoteName  );   //  Text ORIGIN is in UpperLeft

	//	int  xOffsetText       =     (  xOvalCenter   -    ( sz.cx /2 )   )     +   m_xOffsetRevolver; 	
	//	int  yOffsetText       =     (  yOvalCenter   -    ( sz.cy /2 )   )     +   m_yOffsetRevolver; 

		dc.SetBkColor(  color  );       // also want the  TextBoundBox to have the right backround color

		dc.TextOut(   xDest,   yDest,     retScaleNoteName   );

	}   //   if(   testSystemFonts 

	else
	{																				//  use  Creative Detectors  Fonts
		if(    Font_Is_Installed()    )   
		{		

			short   scalePitchTextIdx =   scalePitch;

			if(    userPrefMusicalKeyAccidentals ==  3   )   //  3:  Use NUMERALS,  which rely on the POSITION of the Bullet, and doesn't care about what the pitch really is.
				scalePitchTextIdx  =    m_positionID;


			long	charWidth =	  m_fontPtr->Get_Characters_Width(   scalePitchTextIdx,    userPrefMusicalKeyAccidentals   );

		
	// Revolver

			if(   m_orientationOfGauge ==   0   )    //  0:  Horizontal   (  of the  Revolver's GAUGE,  Not   its Bullets  ) 
			{
				xDest  =     rect.left    +    widthBoundBox/2    -   charWidth /2 ;              
				yDest  =     rect.top    +    heightBoundBox/2   -   m_fontPtr->m_height /2;
			}
			else if(   m_orientationOfGauge ==   1   )    //   1: Vertical   (  of the  Revolver's GAUGE,  Not   its Bullets  ) 
			{
			ASSERT( 0 );  //  ***  Not used yet 2/12   Revolver is always Horizontal  ***

				xDest  =     rect.left    +    widthBoundBox/2    -   charWidth /2 ;   
				yDest  =     rect.top    +    heightBoundBox/2   -   m_fontPtr->m_height /2;
			}
			else
			{	ASSERT( 0 );   }



			if(    ! m_fontPtr->Blit_Musical_Notes_NameText(   scalePitchTextIdx,   userPrefMusicalKeyAccidentals,   xDest, yDest,   false,   dc,   retErrorMesg  )    )
			{
				AfxMessageBox( retErrorMesg  );
			}
		}
	}   //  use  CD Fonts


	m_visible =   true;
}  
