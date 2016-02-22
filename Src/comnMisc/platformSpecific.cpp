/////////////////////////////////////////////////////////////////////////////
//
//  platformSpecific.cpp   -   For WINDOWS(  not Mac ) 
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



//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"      //  #include <FixMath.h>    ...Mac's version of Fixedpoint 
#include  "..\ComnGrafix\CommonGrafix.h"      //  #include "commonmy.h"   	

#include  "..\ComnGrafix\OffMap.h"  
#include  "..\comnGrafix\TransformMap.h"     
//////////////////////////////////////////////////     





// ???  no header,  just selectively copy function signatures around


////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////



										//  BELOW are contained Global Function in this module



//void     Clear_MachineSpec_Window(  void  *CDevC   );
void      Clear_MachineSpec_Window(  CDC&  devContext,    RectShrt  rct    );


void     Blit_Font_Character_to_Display(   void  *cDevC,     long xDest,  long yDest,     long width,  long height,      bool doXOR,  
																		                                long  xOffsetSrc,  long yOffsetSrc, 	  CBitmap&  fontMask  );   //   NEW,  2/12



void     Blit_Offmap_2PCwindow(                                       OffMap *offMap,   void  *CDevC,      RectShrt&  srcRect,    RectShrt&  dstRect   );
void     Blit_Offmap_2PCwindow_TransparentBackground(   OffMap *offMap,   void  *CDevC,      RectShrt&  srcRect,    RectShrt&  dstRect   );

void     Blit_Offmap_2PCwindow_Summed_RGB_Values(     OffMap *offMap,   void  *CDevC,    RectShrt&  srcRect,    RectShrt&  dstRect,
																			                      short  hueOffset,  short  saturationOffset,  short valueOffset,   short  regionCode  );   //   NEW,  8/08


unsigned char*   Get_Copy_of_Bitmaps_Bits_HBITMAP(   HBITMAP   hBmp,         HDC  hDC,     bool drawBlackBorder,     	BITMAPINFOHEADER  *retBmInfoHeader,   CString&  retErrorMesg   );
unsigned char*   Get_Copy_of_Bitmaps_Bits_CBitmap(    CBitmap&  msBitMap,   HDC hDC,     bool drawBlackBorder,     bool  drawBlackMask,    BITMAPINFOHEADER  *retBmInfoHeader,   CString&  retErrorMesg   );



unsigned char*   Convert_Bitmap_Bits_from32_to24(   unsigned char  *pBits,    bool drawBlackBorder,    bool  drawBlackMask,    bool  invertMap,   BITMAPINFOHEADER&  bmiHeader,   CString&  retErrorMesg   );


bool     Blit_Text_with_MaskBlt_TEMP(  void  *cDevC,    short   charCode,     long  xOffset,   long yOffset,   CBitmap& fontMask,   CString&  retErrorMesg   );




void     Rectangle_Fill(   RectShrt rct,    short redFill,  short greenFill,  short  blueFill,    CDC&  devContext   );

void     SolidGray_Rect_Frame(   RectShrt rct,   void *CDevC,   short val,    short  penWidth   );
void     SolidColor_Rect_Frame(   RectShrt rct,   void *CDevC,     short red,  short green,  short  blue,   short  penWidth   );
void     Rectangle_Filled_wOutline(   RectShrt rct,     short redOut,  short greenOut,  short  blueOut,   
												       short redFill,  short greenFill,  short  blueFill,   short  width,   void *CDevC   );
void    Outline_Color_Rect(  RectShrt rct,   void *CDevC,     short red,  short green,  short  blue,    short  penWidth   );


void    Fill_SolidGrey_Rect(    RectShrt&  rect,    short  greyVal,      CDC&  dc    );   //   NEW
void    Fill_SolidColor_Rect(    RectShrt&  rect,    COLORREF  clr,     CDC&  dc    );   //   NEW


void     Xor_box_Outline(   RectShrt  rct,   void  *CDevC,   short  penWidth     );
void     Xor_box_Filled(      RectShrt  rct,   void  *CDevC   );   //  new

void     Colored_Box_Outline(   RectShrt  rct,    short red,  short green,  short  blue,   short  width,    void  *CDevC   );





void     Draw_SolidGray_Oval(      RectShrt  rct,    void *CDevC,    short val    );
void     Draw_OutlineGrey_Oval(   RectShrt  rct,    void *CDevC,    short val,     short  penWidth    );

void     Draw_Solid_Colored_Oval(   RectShrt  rct,    void *CDevC,    short red,  short green,  short  blue  );   //  NEW




void     Xor_Line(   short x0,  short y0,     short x1,  short y1,   short  penWidth,   void  *CDevC  );   // new,  could OPTIMIZE !!! 
void     Xor_Line_Fancy(   short x0,  short y0,     short x1,  short y1,   short  penWidth,   void  *CDevC,    int   fancyCode  );


void     Paint_Line(   short x0,  short y0,     short x1,  short y1,    short red,  short green,  short  blue,  short  penWidth,   void  *CDevC  );

void     Paint_Line_Fancy(   short x0,  short y0,     short x1,  short y1,    short red,  short green,  short  blue,   
																			short  penWidth,   void  *CDevC,    int   fancyCode   );




void     Colored_Polygon_Outline(   CPoint  pts[],   short  totalPoints,    short red,  short green,  short  blue,   
																										short  width,    void  *CDevC   );

void     Colored_Polygon_Fill(   CPoint  pts[],   short  totalPoints,       short redOut,  short greenOut,  short  blueOut,   
												  short redFill,  short greenFill,  short  blueFill,   short  width,   void *CDevC   );


void     Paint_Arc_for_MusicalTie(   short x0, short x1,   short y,     short  halfHeight,   	short red,  short green,  short  blue,
															     short  penWidth,   void  *CDevC  );




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////



void     Xor_box_Outline(   RectShrt  rct,   void  *CDevC,    short  penWidth   )
{

	if(  CDevC  ==  NULL  )
	{
		//   ASSERT( 0 );    ...Handles sometime do this cause i do not first set to NOT visable
		return;
	}
	CDC   *devContext =   ( CDC* )CDevC;    //   ****   CAREFUL with a cast !!! *******


	int  nDrawMode =    devContext->GetROP2( );

	CPen   penBlack;  
    if(      !penBlack.CreatePen(  PS_SOLID,   penWidth,   RGB(0,0,0)    )     )
    {
		int dummy = 9;
	}

												// Select it into the device context.  Save the old pen at the same time
    CPen   *pOldPen =    devContext->SelectObject(  &penBlack  );
   
	devContext->SetROP2(   R2_NOT    );       //   R2_NOT    R2_XORPEN    


	devContext->MoveTo(   rct.left,     rct.top  );     
		devContext->LineTo(    rct.right,        rct.top   );   
		devContext->LineTo(     rct.right,        rct.bottom  );      // ******** TEST,  might miss points   1/02  *******
		devContext->LineTo(    rct.left,      rct.bottom   );
		devContext->LineTo(    rct.left,          rct.top    );    


	devContext->SelectObject( pOldPen );           // Restore the old pen to the device context
	devContext->SetROP2(  nDrawMode   );
}


											////////////////////////////////////////


void     Xor_box_OutlinePenWidth(   RectShrt  rct,   short  penWidth,    void  *CDevC   )
{

	if(   penWidth  <=  0  )
	{
		ASSERT( 0 );
		return;
	}



	if(  CDevC  ==  NULL  )
	{
		//   ASSERT( 0 );    ...Handles sometime do this cause i do not first set to NOT visable
		return;
	}
	CDC   *devContext =   ( CDC* )CDevC;    //   ****   CAREFUL with a cast !!! *******


	int  nDrawMode =    devContext->GetROP2( );

	CPen   penBlack;  
    if(      !penBlack.CreatePen(   PS_SOLID,    penWidth,   RGB(0,0,0)    )     )
    {
		int dummy = 9;
	}

												// Select it into the device context.  Save the old pen at the same time
    CPen   *pOldPen =    devContext->SelectObject(  &penBlack  );
   
	devContext->SetROP2(   R2_NOT    );       //   R2_NOT    R2_XORPEN    


	devContext->MoveTo(   rct.left,     rct.top  );     
		devContext->LineTo(    rct.right,        rct.top   );   
		devContext->LineTo(     rct.right,        rct.bottom  );      // ******** TEST,  might miss points   1/02  *******
		devContext->LineTo(    rct.left,      rct.bottom   );
		devContext->LineTo(    rct.left,          rct.top    );    


	devContext->SelectObject( pOldPen );           // Restore the old pen to the device context
	devContext->SetROP2(  nDrawMode   );
}





											////////////////////////////////////////


void     Xor_box_Filled(   RectShrt  rct,   void  *CDevC   )
{

	if(  CDevC  ==  NULL  )
	{
		//   ASSERT( 0 );    ...Handles sometime do this cause i do not first set to NOT visable
		return;
	}
	CDC   *devContext =   ( CDC* )CDevC;    //   ****   CAREFUL with a cast !!! *******


	int  nDrawMode =    devContext->GetROP2( );

	CPen   penBlack;  
    if(      !penBlack.CreatePen(   PS_SOLID,   1,    RGB(0,0,0)     )     )
    {
		ASSERT( 0 );
	}
												// Select it into the device context.  Save the old pen at the same time
    CPen   *pOldPen =    devContext->SelectObject(  &penBlack  );
   

																									 // and a solid black brush
    CBrush      brushRed(   RGB(0,0,0)     );
    CBrush*    pOldBrush =       devContext->SelectObject(&brushRed);


	devContext->SetROP2(   R2_NOT    );   // *** OK ???
   

	CRect  rect(  rct.left,   rct.top,      rct.right,   rct.bottom  );
   
    devContext->Rectangle(  rect  );   



	devContext->SelectObject( pOldPen    );           // Restore the old pen to the device context
	devContext->SelectObject( pOldBrush );

	devContext->SetROP2(  nDrawMode   );
}



											////////////////////////////////////////


void     Colored_Polygon_Outline(   CPoint  pts[],   short  totalPoints,    short red,  short green,  short  blue,   
																										short  width,    void  *CDevC   )
{
	if(       CDevC  ==  NULL 
		||    totalPoints  <=  0   )
	{
		//   ASSERT( 0 );    ...Handles sometime do this cause i do not first set to NOT visable
		return;
	}
	CDC   *devContext =   ( CDC* )CDevC;    //   ****   CAREFUL with a cast !!! *******


	int  nDrawMode =    devContext->GetROP2( );

	CPen   penBlack;  
    if(      !penBlack.CreatePen(  PS_SOLID,   width,  RGB(red, green,  blue)    )     )
    {
		int dummy = 9;
	}
												// Select it into the device context.  Save the old pen at the same time
    CPen   *pOldPen =    devContext->SelectObject(  &penBlack  );
   


	/***																							
    CBrush      brushRed(   RGB( 255, 0, 0 )    );							 // and a solid red brush
    CBrush*    pOldBrush =       devContext->SelectObject(&brushRed);
	***/

	devContext->SetROP2(   R2_COPYPEN    ); 
   

	devContext->Polygon(  pts,  totalPoints  );



	devContext->SelectObject( pOldPen    );           // Restore the old pen to the device context
//	devContext->SelectObject( pOldBrush );

	devContext->SetROP2(  nDrawMode   );
}



											////////////////////////////////////////


void     Colored_Polygon_Fill(   CPoint  pts[],   short  totalPoints,       short redOut,  short greenOut,  short  blueOut,   
												  short redFill,  short greenFill,  short  blueFill,   short  width,   void *CDevC   )
{

	if(       CDevC  ==  NULL 
		||    totalPoints  <=  0   )
	{
		//   ASSERT( 0 );    ...Handles sometime do this cause i do not first set to NOT visable
		return;
	}
	CDC   *devContext =   ( CDC* )CDevC;    //   ****   CAREFUL with a cast !!! *******


	int  nDrawMode =    devContext->GetROP2( );



	CPen   penBlack;  
    if(      !penBlack.CreatePen(   PS_SOLID,   width,    RGB( redOut, greenOut, blueOut )    )     )
    {
		ASSERT( 0 );
	}
												// Select it into the device context.  Save the old pen at the same time
    CPen   *pOldPen =    devContext->SelectObject(  &penBlack  );
   

																									 // and a solid red brush
    CBrush      brushRed(   RGB(  redFill,   greenFill,    blueFill  )    );
    CBrush*    pOldBrush =       devContext->SelectObject(&brushRed);


	devContext->SetROP2(   R2_COPYPEN    );   // *** OK ???
   

	devContext->Polygon(  pts,  totalPoints  );



	devContext->SelectObject( pOldPen    );           // Restore the old pen to the device context
	devContext->SelectObject( pOldBrush );

	devContext->SetROP2(  nDrawMode   );
}







											////////////////////////////////////////


void     SolidColor_Rect_Frame(   RectShrt rct,   void *CDevC,     short red,  short green,  short  blue,    short  penWidth   )
{

																	//  used in     TBoxButton::Draw()
			//  **** INSTALL:   penWidth

	if(  CDevC  ==  NULL  )
	{
		ASSERT( 0 );				
		return;
	}

	CDC   *devContext =   ( CDC* )CDevC;    //   ****   CAREFUL with a cast !!! *******


	COLORREF   clr =    RGB(  red,  green,  blue  );


//   devContext->FillSolidRect(     rct.left,   rct.top,     rct.right,  rct.bottom,      clr    );      WRONG!!! 

	short   width =       (rct.right     -   rct.left)   +1;
	short   heit   =       (rct.bottom  -  rct.top)    +1;

	devContext->FillSolidRect(     rct.left,  rct.top,     width,   heit,     clr   );

				// *****  FrameRect( LPCRECT lpRect, CBrush* pBrush );   ****** BETTER????
}




											////////////////////////////////////////


void    Outline_Color_Rect(  RectShrt rct,   void *CDevC,     short red,  short green,  short  blue,    short  penWidth   )
{


	if(  CDevC  ==  NULL  )
	{
		//   ASSERT( 0 );    ...Handles sometime do this cause i do not first set to NOT visable
		return;
	}
	CDC   *devContext =   ( CDC* )CDevC;    //   ****   CAREFUL with a cast !!! *******


	int  nDrawMode =    devContext->GetROP2( );

	CPen   penBlack;  
    if(      !penBlack.CreatePen(  PS_SOLID,  penWidth,  RGB(  red,   green,   blue  )    )     )
    {
		int dummy = 9;
	}

												// Select it into the device context.  Save the old pen at the same time
    CPen   *pOldPen =    devContext->SelectObject(  &penBlack  );
   


	devContext->MoveTo(   rct.left,     rct.top  );     
		devContext->LineTo(    rct.right,        rct.top   );   
		devContext->LineTo(     rct.right,        rct.bottom  );   
		devContext->LineTo(    rct.left,      rct.bottom   );
		devContext->LineTo(    rct.left,          rct.top -1    );     //  -1:   Need that to close the upper left corner   8/07



	devContext->SelectObject( pOldPen );           // Restore the old pen to the device context
//	devContext->SetROP2(  nDrawMode   );
}


											////////////////////////////////////////


void    Paint_Text(   RectShrt rct,   void *CDevC,    CString&  text,     short red,  short green,  short  blue,
		                                    short redBk,  short greenBk,  short  blueBk   )		   
{


	if(  CDevC  ==  NULL  )
	{
		//   ASSERT( 0 );    ...Handles sometime do this cause i do not first set to NOT visable
		return;
	}
	CDC   *devContext =   ( CDC* )CDevC;    //   ****   CAREFUL with a cast !!! *******


	/*****
	int  nDrawMode =    devContext->GetROP2( );

	CPen   penBlack;  
    if(      !penBlack.CreatePen(  PS_SOLID,  penWidth,  RGB(  red,   green,   blue  )    )     )
    {
		int dummy = 9;
	}

												// Select it into the device context.  Save the old pen at the same time
    CPen   *pOldPen =    devContext->SelectObject(  &penBlack  );
   
//	devContext->SetROP2(   R2_NOT    );       //   R2_NOT    R2_XORPEN    


	devContext->MoveTo(   rct.left,     rct.top  );     
		devContext->LineTo(    rct.right,        rct.top   );   
		devContext->LineTo(     rct.right,        rct.bottom  );      // ******** TEST,  might miss points   1/02  *******
		devContext->LineTo(    rct.left,      rct.bottom   );
		devContext->LineTo(    rct.left,          rct.top    );    


	devContext->SelectObject( pOldPen );           // Restore the old pen to the device context
//	devContext->SetROP2(  nDrawMode   );
     *****/


	COLORREF   clrText            =      RGB(   red,       green,       blue       );
	COLORREF   clrBackground =      RGB(   redBk,   greenBk,    blueBk   );

	devContext->SetTextColor(  clrText  );
	devContext->SetBkColor(    clrBackground  );


	long   x =    rct.left;
	long   y =    rct.top;


	devContext->TextOut(   x, y,     text    );
}





											////////////////////////////////////////


void     Rectangle_Filled_wOutline(   RectShrt rct,   short redOut,  short greenOut,  short  blueOut,   
												       short redFill,  short greenFill,  short  blueFill,   short  width,   void *CDevC   )
{

	if(   CDevC  ==  NULL   )
	{
		//   ASSERT( 0 );    ...Handles sometime do this cause i do not first set to NOT visable
		return;
	}
	CDC   *devContext =   ( CDC* )CDevC;    //   ****   CAREFUL with a cast !!! *******


	int  nDrawMode =    devContext->GetROP2( );



	CPen   penBlack;  
    if(      !penBlack.CreatePen(   PS_SOLID,   width,    RGB( redOut, greenOut, blueOut )    )     )
    {
		ASSERT( 0 );
	}
												// Select it into the device context.  Save the old pen at the same time
    CPen   *pOldPen =    devContext->SelectObject(  &penBlack  );
   

																									 // and a solid red brush
    CBrush      brushRed(   RGB(  redFill,   greenFill,    blueFill  )    );
    CBrush*    pOldBrush =       devContext->SelectObject(&brushRed);


	devContext->SetROP2(   R2_COPYPEN    );   // *** OK ???
   

	CRect  rect(  rct.left,   rct.top,      rct.right,   rct.bottom  );
   
    devContext->Rectangle(  rect  );   // draw a thick black rectangle filled with blue



	devContext->SelectObject( pOldPen    );           // Restore the old pen to the device context
	devContext->SelectObject( pOldBrush );

	devContext->SetROP2(  nDrawMode   );
}



											////////////////////////////////////////


void     Rectangle_Fill(    RectShrt  rct,    short redFill,  short greenFill,  short  blueFill,    CDC&  devContext   )
{

			// draw a filled rectangle,   **** does NOT draw the bottom and right coords ****


	short   outlineWidth =  1;   // ****  OK ???  *****

	/***
	if(   CDevC  ==  NULL   )
	{
		//   ASSERT( 0 );    ...Handles sometime do this cause i do not first set to NOT visable
		return;
	}

//	CDC   *devContext =   ( CDC* )CDevC;    
	CDC   *devContext =     CDevC;   // no more sloopy casts
	***/

	int  nDrawMode =    devContext.GetROP2( );


	
	CPen   penBlack;  
    if(      !penBlack.CreatePen(   PS_SOLID,   outlineWidth,    RGB( redFill,   greenFill,    blueFill )    )     )
    {
		ASSERT( 0 );
	}
												// Select it into the device context.  Save the old pen at the same time
    CPen   *pOldPen =    devContext.SelectObject(  &penBlack  );
	
   

																									 // and a solid red brush
    CBrush      brushRed(   RGB(  redFill,   greenFill,    blueFill  )    );
    CBrush*    pOldBrush =       devContext.SelectObject(&brushRed);


	devContext.SetROP2(   R2_COPYPEN    );   // *** OK ???
   

	CRect  rect(  rct.left,   rct.top,      rct.right,   rct.bottom  );
   



    devContext.Rectangle(  rect  );   // draw a filled rectangle,  does NOT draw the bottom and right coords
	/***
	The rectangle extends up to, but does not include, the right and bottom coordinates. This means that 
	the height of the rectangle is y2 – y1 and the width of the rectangle is x2 – x1. Both the width and the 
	height of a rectangle must be greater than 2 units and less than 32,767 units.
	***/


	devContext.SelectObject( pOldPen    );           // Restore the old pen to the device context
	devContext.SelectObject( pOldBrush );

	devContext.SetROP2(  nDrawMode   );
}



											////////////////////////////////////////


void    Fill_SolidGrey_Rect(    RectShrt&  rect,    short  greyVal,     CDC&  dc    )
{


	COLORREF   color =    RGB(  greyVal,  greyVal,  greyVal  );


	short   width =       (rect.right     -   rect.left);    //  +1;
	short   heit   =       (rect.bottom  -   rect.top);   //   +1;

	CRect   msRect(   rect.left,  rect.top,      rect.right,   rect.bottom    );

//  rect.DeflateRect(  20,  20  );    // shrink our rect 20 pixels in each direction   **** Try this some time


																			// create and select a solid black brush
   CBrush   blackBrush(  color   );

   CBrush   *pOldBrush =    dc.SelectObject(  &blackBrush  );


																	// create and select a    NULL  pen
   CPen penBlack;
   penBlack.CreatePen(   PS_SOLID,  0,  color   );

   CPen* pOldPen = dc.SelectObject(  &penBlack  );



// *****************************  WEIRD BUG    2/2012  *******************************

//	 dc.FillSolidRect(  rect.left,  rect.top,  width,   heit,  color   );   ******  FAILS for  BulletRevolver::Hide      ABADON this function,  think   CDC::FillSolidRect()  has a BUG   2/2012  ***************************
	dc.Rectangle(   &msRect    );  

// *****************************  WEIRD BUG    2/2012  *******************************



														 // put back the old objects
	dc.SelectObject(  pOldBrush  );
	dc.SelectObject(  pOldPen     );
}




void    Fill_SolidColor_Rect(    RectShrt&  rect,    COLORREF  color,     CDC&  dc    )
{

	//  COLORREF   clr =    RGB(  greyVal,  greyVal,  greyVal  );

	short   width =       (rect.right     -   rect.left);   //  +1;
	short   heit   =       (rect.bottom  -   rect.top);  //  +1;


	CRect   msRect(   rect.left,  rect.top,      rect.right,   rect.bottom    );


																			// create and select a solid  brush
   CBrush   blackBrush(  color   );

   CBrush   *pOldBrush =    dc.SelectObject(  &blackBrush  );



																	// create and select a pen
   CPen  penBlack;
   penBlack.CreatePen(   PS_SOLID,  0,  color   );

   CPen* pOldPen =  dc.SelectObject(  &penBlack  );



//	dc.FillSolidRect(     rect.left,  rect.top,     width,   heit,     clr   );  ******  DANGEROUS Function,   see above   2/2012********************
	dc.Rectangle(   &msRect    );  


														 // put back the old objects
	dc.SelectObject(  pOldBrush  );
	dc.SelectObject(  pOldPen     );
}





											////////////////////////////////////////


void     SolidGray_Rect_Frame(   RectShrt rct,   void *CDevC,   short val,    short  penWidth   )
{





//    ASSERT(  0  );  //   ***************   ABADON this function,  think   CDC::FillSolidRect()  has a BUG   2/2012  ***************************

//		*********************  But this is still used in OLD PitchScope   2/2012  *******************



																	//  used in     TBoxButton::Draw()
			//  **** INSTALL:   penWidth

	if(  CDevC  ==  NULL  )
	{
		ASSERT( 0 );				
		return;
	}



	CDC   *devContext =   ( CDC* )CDevC;    //   ****   CAREFUL with a cast !!! {  try with Dynamic cast ??? ] *******




	COLORREF   clr =    RGB(  val,  val,  val  );


//   devContext->FillSolidRect(     rct.left,   rct.top,     rct.right,  rct.bottom,      clr    );      WRONG!!! 

	short   width =       (rct.right     -   rct.left)   +1;
	short   heit   =       (rct.bottom  -  rct.top)    +1;


	devContext->FillSolidRect(     rct.left,  rct.top,     width,   heit,     clr   );


				// *****  FrameRect( LPCRECT lpRect, CBrush* pBrush );   ****** BETTER????
}





											////////////////////////////////////////


void     Draw_SolidGray_Oval(   RectShrt  rct,    void *CDevC,   short val   )
{

											//  NEW,  finish 
	if(  CDevC  ==  NULL  )
	{
		ASSERT( 0 );				
		return;
	}

	CDC   *devContext =   ( CDC* )CDevC;    //   ****   CAREFUL with a cast !!! *******


	COLORREF    clr            =    RGB(  val,  val,  val  );
	short            penWidth  =  1;


	CPen   penBlack;   // Construct it,  then initialize
    if(      !penBlack.CreatePen(   PS_SOLID,    penWidth,   clr   )     )
		ASSERT( 0 );
												// Select it into the device context.  Save the old pen at the same time
    CPen   *pOldPen =    devContext->SelectObject(  &penBlack  );



	CBrush   brush;   // Construct it,  then initialize
	if(      !brush.CreateSolidBrush(  clr  )    )
		ASSERT( 0 );
												// Select it into the device context.  Save the old pen at the same time
    CBrush   *pOldBrush =    devContext->SelectObject(  &brush  );


	devContext->Ellipse(    rct.left,   rct.top,      rct.right,   rct.bottom  );


	devContext->SelectObject(  pOldBrush  );           // Restore the old pen to the device context
	devContext->SelectObject(  pOldPen     );           // Restore the old pen to the device context
}



											////////////////////////////////////////


void     Draw_OutlineGrey_Oval(   RectShrt  rct,    void *CDevC,     short val,     short  penWidth   )
{

											//  NEW,  finish 
	if(  CDevC  ==  NULL  )
	{
		ASSERT( 0 );				
		return;
	}

	CDC   *devContext =   ( CDC* )CDevC;    //   ****   CAREFUL with a cast !!! *******


	COLORREF    clr            =    RGB(  val,  val,  val  );


	CPen   penBlack;   // Construct it,  then initialize
    if(      !penBlack.CreatePen(   PS_SOLID,    penWidth,   clr   )     )
		ASSERT( 0 );
												// Select it into the device context.  Save the old pen at the same time
    CPen   *pOldPen =    devContext->SelectObject(  &penBlack  );



	/***
	CBrush   brush;   // Construct it,  then initialize
	if(      !brush.CreateSolidBrush(  clr  )    )
		ASSERT( 0 );
												// Select it into the device context.  Save the old pen at the same time
    CBrush   *pOldBrush =    devContext->SelectObject(  &brush  );
	***/
	devContext->Ellipse(    rct.left,   rct.top,      rct.right,   rct.bottom  );


//	devContext->SelectObject(  pOldBrush  );           // Restore the old pen to the device context

	devContext->SelectObject(  pOldPen     );           // Restore the old pen to the device context
}




											////////////////////////////////////////


void     Colored_Box_Outline(   RectShrt  rct,    short red,  short green,  short  blue,   short  width,    void  *CDevC   )
{

	if(  CDevC  ==  NULL  )
	{
		//   ASSERT( 0 );    ...Handles sometime do this cause i do not first set to NOT visable
		return;
	}
	CDC   *devContext =   ( CDC* )CDevC;    //   ****   CAREFUL with a cast !!! *******


	int  nDrawMode =    devContext->GetROP2( );

	CPen   penBlack;  
    if(      !penBlack.CreatePen(  PS_SOLID,   width,  RGB(red, green,  blue)    )     )
    {
		int dummy = 9;
	}

												// Select it into the device context.  Save the old pen at the same time
    CPen   *pOldPen =    devContext->SelectObject(  &penBlack  );
   
//	devContext->SetROP2(   R2_NOT    );       //   R2_NOT    R2_XORPEN    
	devContext->SetROP2(   R2_COPYPEN    ); 
   


	devContext->MoveTo(   rct.left,     rct.top  );     
		devContext->LineTo(    rct.right,        rct.top   );   
		devContext->LineTo(     rct.right,        rct.bottom  );      // ******** TEST,  might miss points   1/02  *******
		devContext->LineTo(    rct.left,      rct.bottom   );
		devContext->LineTo(    rct.left,          rct.top    );    


	devContext->SelectObject( pOldPen );           // Restore the old pen to the device context
	devContext->SetROP2(  nDrawMode   );
}



											////////////////////////////////////////


void     Draw_Solid_Colored_Oval(   RectShrt  rct,    void *CDevC,    short red,  short green,  short  blue    )
{

											//  NEW,  finish 
	if(  CDevC  ==  NULL  )
	{
		ASSERT( 0 );				
		return;
	}

	CDC   *devContext =   ( CDC* )CDevC;    //   ****   CAREFUL with a cast !!! *******


	COLORREF    clr            =    RGB(  red,  green,  blue  );
	short            penWidth  =  1;


	CPen   penBlack;   // Construct it,  then initialize
    if(      !penBlack.CreatePen(   PS_SOLID,    penWidth,   clr   )     )
		ASSERT( 0 );
												// Select it into the device context.  Save the old pen at the same time
    CPen   *pOldPen =    devContext->SelectObject(  &penBlack  );



	CBrush   brush;   // Construct it,  then initialize
	if(      !brush.CreateSolidBrush(  clr  )    )
		ASSERT( 0 );
												// Select it into the device context.  Save the old pen at the same time
    CBrush   *pOldBrush =    devContext->SelectObject(  &brush  );


	devContext->Ellipse(    rct.left,   rct.top,      rct.right,   rct.bottom  );


	devContext->SelectObject(  pOldBrush  );           // Restore the old pen to the device context
	devContext->SelectObject(  pOldPen     );           // Restore the old pen to the device context
}


											////////////////////////////////////////


void     Clear_MachineSpec_Window(  CDC&  devContext,    RectShrt  rct    )
{

			//  REFINE  someday!!! *******   JPM

	short  white =  255;

	/*****
	if(  CDevC  ==  NULL  )
	{
		ASSERT( 0 );
		return;
	}
	CDC   *devContext =   ( CDC* )CDevC;    
	

	int   width   =    30000;		// ****** FIX this sloppy HACK !!!!  ***************************   JPM
	int   height  =    30000;

	devContext->PatBlt(   0, 0,   width,  height,   WHITENESS    );   //   WHITENESS    BLACKNESS
	*****/

	Rectangle_Fill(   rct,      white,  white,  white,     devContext   );
}




				////////////////////////////////////////


/****
void    Blit_Offmap_2PCwindow(   OffMap *offMap,   void  *CDevC,    RectShrt&  srcRect,   RectShrt&  dstRect   )
{

										// ******* WANT  +1  ( see below ???? *********************   JPM
	if(       offMap ==  NULL
		||   CDevC ==  NULL    )
	{
		ASSERT( 0 );
		return;
	}

	CDC   *devContext =   ( CDC* )CDevC;    



	  int XDest =   dstRect.left;                   // x-coord of destination upper-left corner
	  int YDest =   dstRect.top;                    // y-coord of destination upper-left corner

	  int nDestWidth =   ( dstRect.right      -    dstRect.left )  // +1 ;      	// ******* WANT  +1  ?????        // width of destination rectangle
	  int nDestHeight =  ( dstRect.bottom   -   dstRect.top )   // +1 ;              // height of destination rectangle


	  int XSrc =   srcRect.left;                     // x-coord of source upper-left corner
	  int YSrc =   srcRect.top;                     // y-coord of source upper-left corner

	  int nSrcWidth =   ( srcRect.right       -   srcRect.left )    // +1 ;         // ******* WANT  +1  ?????         // width of source rectangle
	  int nSrcHeight =  ( srcRect.bottom   -    srcRect.top )   // +1  ;            // height of source rectangle
  



	if(     !StretchDIBits(          devContext->m_hDC,     // handle to DC

										   XDest,     
										   YDest,       
										   nDestWidth,       
										   nDestHeight,        

										   XSrc,             
										   YSrc,              
										   nSrcWidth,        
										   nSrcHeight,    

											offMap->Get_Bits_Start(), 
											offMap->Get_BitmapInfo(),
											DIB_RGB_COLORS,  
											SRCCOPY   )   )
	{  
		ASSERT( 0 );

		DWORD  error =   GetLastError();
		CString  strError;
		strError.Format(  " ***ERR:  Blit_Offmap_2PCwindow()   [  ErrorNum:  %d  ]  \n"  ,   error   );
		TRACE(  strError  );		
		return;
	}	
}										
***/
void    Blit_Offmap_2PCwindow(   OffMap *offMap,   void  *CDevC,    RectShrt&  srcRect,   RectShrt&  dstRect   )
{

		//   ALTERED for UPSIDE down DIBS  ( 7/2002 )



														// ******* WANT  +1  ( see below ???? *********************   JPM
	if(       offMap ==  NULL
		||   CDevC ==  NULL    )
	{
		ASSERT( 0 );
		return;
	}

	CDC   *devContext =   ( CDC* )CDevC;    



	  int XDest =   dstRect.left;                   // x-coord of destination upper-left corner
	  int YDest =   dstRect.top;                    // y-coord of destination upper-left corner

	  int nDestWidth =   ( dstRect.right      -    dstRect.left )   +1;     // ******* WANT  +1  ?? [ +1 :  Inclusive Counting ]       // width of destination rectangle	 		  
	  int nDestHeight =  ( dstRect.bottom   -   dstRect.top )   +1;              // height of destination rectangle





	  int XSrc         =      srcRect.left;                                      // x-coord of source upper-left corner
	  int nSrcWidth =    ( srcRect.right  -  srcRect.left )  +1;         // width of source  rectangle [ +1 :  Inclusive Counting ]    
	 




  	   /***   OK for Mac,  but must change for INVERTED Windows DIBs
	
	  int YSrc =   srcRect.top;                     // y-coord of source upper-left corner
	  int nSrcHeight =  ( srcRect.bottom   -    srcRect.top )   // +1  ;            // height of source rectangle
	   ***/ 

							//  the input  RectShrt  was in VIRTUAL-coords,  so must Invert for the UPSIDE-DOWN Windows DIBs

	  int   yTopTrans       =    ( offMap->m_height  -1 )    -    srcRect.top; 
	  int   yBottomTrans  =    ( offMap->m_height  -1 )    -    srcRect.bottom; 

	  int   YSrc           =        yBottomTrans;                                      // y-coord of source upper-left corner
	  int   nSrcHeight  =      ( yTopTrans  -  yBottomTrans )  +1;              // height of source rectangle,  [ +1 :  Inclusive Counting ]




	BYTE  *bitsStart   =     offMap->Get_Bits_Start(); 
	if(        bitsStart  ==  NULL  )
	{
		ASSERT( 0 );   //  get here from not reinitializing the  curSeparation   10/20/08
		return;
	}

	BITMAPINFO  *bInfo  =     offMap->Get_BitmapInfo();




	int  reslt  =    StretchDIBits(    devContext->m_hDC,     // handle to DC

										   XDest,     
										   YDest,       
										   nDestWidth,       
										   nDestHeight,        

										   XSrc,             
										   YSrc,              
										   nSrcWidth,        
										   nSrcHeight,    

											bitsStart, 
											bInfo,
											DIB_RGB_COLORS,  
											SRCCOPY   );
	
	
	
	if(   reslt  ==   GDI_ERROR   )     //    GDI_ERROR  =     (0xFFFFFFFFL)
	{  
		//   ASSERT( 0 );

		DWORD  error  =    GetLastError();    // Only for NT !!!!

		CString  strError;
		strError.Format(  " ***ERR:   Blit_Offmap_2PCwindow  FAILED.    [  ErrorNum:  %d  ]  \n"  ,   error   );
		TRACE(  strError  );		

		return;
	}	
}										



											////////////////////////////////////////


void    Blit_Font_Character_to_Display(   void  *cDevC,     long xDest,  long yDest,     long width,  long height,      bool doXOR,  
																		                                             long  xOffsetSrc,  long yOffsetSrc, 	  CBitmap&  fontMask  )
{

	if(       cDevC  ==   NULL
		||   xDest  <  0      )
	{
		ASSERT( 0 );
		return;
	}


	CString    retErrorMesg;
	CDC      *dc =   ( CDC* )cDevC;    //  Or is   CClientDC    more accurate ???    2/12



	CDC  memDCfonts;                  //   create a  DRAWING ENGINE ( CDC)   so that we can work with the  CBitmap  fontMask
     
	memDCfonts.CreateCompatibleDC(  dc  );
	     
	CBitmap  *oldBitmap =    memDCfonts.SelectObject(   &fontMask   );     // Select the bitmap into the in-memory DC




	DWORD  rasterOp =   0x00220326;    //  (  DSna )   For black letter with transparent background  ( source bitmap must be a NEGATIVE image of fonts, in Monochrome )

	if(   doXOR   )
		rasterOp =   SRCINVERT;      //  this will XOR and draw only the Letter,  not the background



	if(   ! dc->BitBlt(    xDest, yDest,    width,  height,     &memDCfonts,     xOffsetSrc,  yOffsetSrc,    rasterOp  )     ) 
	{
		retErrorMesg   =    "Blit_Font_Character_to_Display  FAILED,  BitBlt"  ;
		AfxMessageBox( retErrorMesg  );

		memDCfonts.SelectObject(   oldBitmap   );
		return;
	}


											
	memDCfonts.SelectObject(   oldBitmap   );   	//   CLEANUP


/*****  If the  FONT Map is a NEGATIVE ( what we want.   2/2012  )


SRCCOPY      dest = source       -   black background and white letter  

SRCPAINT           dest = source OR dest       -  No background and Letter is WHITE  

SRCAND                dest = source AND dest      Letter is transparent and the Background is BLACK

SRCINVERT          dest = source XOR dest   -   Does XOR on the letter,  and does NOT show a background.  ( GREAT )   ************

SRCERASE            dest = source AND (NOT dest )   -   Background is BLACK, and letter is XOR 

NOTSRCCOPY       dest = (NOT source)       -  Does  Black Letter with a white background

NOTSRCERASE        dest = (NOT src) AND (NOT dest)   -  Letter is BLACK and the background is XOR

 MERGEPAINT         dest = (NOT source) OR dest     -   Letter is transparent,  and  Background is white

 DSTINVERT          dest = (NOT dest)    -   get an XOR  BoundBox

 0x007801E5     PDSax    -  Letter is XOR,  background is white

0x008701C5    PDSaxn  -   transparent letter and  background is black

0x00220326     DSna   -   Letter is Black  and  Bacckground is TRANSPARENT   ( GREAT )   *********************************8

**************/


/***   Font Map is Positive                Ternary raster operations (  Think that I need to use the  'Ternary raster operations',   NOT   ' Binary raster ops'

#define SRCCOPY            dest = source                           -  shows letter,  but the SRC background shows
#define SRCPAINT           dest = source OR dest            -   does  XOR,   but it is the Letter that is transparent    *** 
#define SRCAND             dest = source AND dest           -   drops out the background and Paints the LETTER in BLACK  **********
#define SRCINVERT         dest = source XOR dest           -  does and XOR,  but still is NOT transparent  *****
#define SRCERASE            dest = source AND (NOT dest )    -   shows letter but  background is still showing
#define NOTSRCCOPY        dest = (NOT source)                -   a straight copy of the NEGATIVE image    
#define NOTSRCERASE        dest = (NOT src) AND (NOT dest)   -   does XOR on both  background and Letter
#define MERGECOPY          dest = (source AND pattern)       -  a strat copy of SRC,  both  background and Letter
#define MERGEPAINT         dest = (NOT source) OR dest     -  drops out Backgground,  but  paints the Letter in White
#define PATCOPY             dest = pattern                -   just a white  BoundBox-Rectangle  
#define PATPAINT            dest = DPSnoo                  -   just a white  BoundBox-Rectangle  
#define PATINVERT         dest = pattern XOR dest       -  XOR's the entire  BoundBox-Rectangle  
#define DSTINVERT          dest = (NOT dest)       -     -   just a XOR  BoundBox-Rectangle  
#define BLACKNESS          dest = BLACK   
#define WHITENESS          dest = WHITE   

007801E5     PDSax    -   Nothing
008701C5    PDSaxn  -    Letter is BLACk and the  background is transparent
00220326     DSna   -  The Letter is transparent,  but background is black   
****/
}



					/////////////////////////////////////////////////


void   Draw_Transparent_TEST(   CDC *pDC,   int x,  int y,    COLORREF crColour,   CBitmap&  theBitmap  )                             
{       



			// ********  TEST ONLY,  is not called.  Need to experiment with  CDC::BitBlt()   and different RasterOps  2/12 ***************


	
	//   http://www.codeguru.com/Cpp/G-M/bitmap/article.php/c1753




	long	 srcHeight  =      14;  

	long   srcWidth   =      10;   //  100     ************   INSTALL,  need to create a funtions to find the Letter in the bitmap  ********
	


	COLORREF   crWhite  =    RGB(  255,  255,   255   );   //  jm addition   ???????????    OK ???????????????????/

	COLORREF   crBlack  =     RGB(     0,      0,      0   );


	COLORREF  crOldBack =   pDC->SetBkColor(      crWhite );
	COLORREF  crOldText  =   pDC->SetTextColor(   crBlack  );



	CDC dcImage, dcTrans;


													// Create two memory dcs for the image and the mask

	dcImage.CreateCompatibleDC( pDC );

	dcTrans.CreateCompatibleDC( pDC );



									// Select the image into the appropriate dc

	CBitmap* pOldBitmapImage =   dcImage.SelectObject(  &theBitmap  );             //   this );



											// Create the mask bitmap
	CBitmap bitmapMask;

	int nWidth  =   srcWidth;
	int nHeight =  srcHeight;

	bitmapMask.CreateBitmap(  nWidth, nHeight,  1,  1,   NULL  );



						// Select the MASK bitmap into the appropriate dc

	CBitmap* pOldBitmapTrans =   dcTrans.SelectObject(  &bitmapMask );




									//  Build MASK based on transparent colour   ?????      What does this do ????

	dcImage.SetBkColor(  crColour  );

	dcTrans.BitBlt( 0, 0,   nWidth, nHeight,   &dcImage,   0, 0,   SRCCOPY   );





										//  Do the work - True Mask method - cool if not actual display


	pDC->BitBlt(  x, y, nWidth, nHeight,    &dcImage,    0, 0,   SRCINVERT  );   // With ONLY this,  letter is transparent and background is  XOR

	pDC->BitBlt(  x, y, nWidth, nHeight,    &dcTrans,     0, 0,   SRCAND       );    //  without this,   nothing shows

	pDC->BitBlt(  x, y, nWidth, nHeight,    &dcImage,    0, 0,   SRCINVERT  );    //  without this,  the XOR bacground shows



	// Restore settings

	dcImage.SelectObject(  pOldBitmapImage  );
	dcTrans.SelectObject(   pOldBitmapTrans  );

	pDC->SetBkColor(crOldBack);
	pDC->SetTextColor(crOldText);
}




					/////////////////////////////////////////////////


bool     Blit_Text_with_MaskBlt_TEMP(  void  *cDevC,    short   charCode,     long  xOffset,   long yOffset,   CBitmap& fontMask,   CString&  retErrorMesg   )
{


			// ********  TEST ONLY,  is not called.  Need to experiment with  CDC::MaskBlt()   ***************


	long   srcWidth =    10,     srcHeight =  14;  
	long   destWidth =  10,     destHeight = 14;



	if(       cDevC  ==   NULL
		||   xOffset  <  0      )
	{
		ASSERT( 0 );
		retErrorMesg =  "Blit_Text_with_MaskBlt  FAILED, incorrect input parms." ;
		return false;
	}



	CDC      *dc =   ( CDC* )cDevC;    //  Or is   CClientDC    more accurate ???    2/12



	CDC  memDC;     
	memDC.CreateCompatibleDC(  dc  );
	     
	CBitmap  *oldBitmap =    memDC.SelectObject(   &fontMask   );     // Select the bitmap into the in-memory DC



//	DWORD   MAKEROP4(  DWORD fore,       // foreground ternary raster operation code
//									  DWORD back  ); // background ternary raster operation code

//	DWORD   rasterOp  =      MAKEROP4(    SRCAND ,   DSTINVERT     );


	DWORD   rasterOp  =    0x007801E5   ;




/***  Ternary raster operations (  Think that I need to use the  'Ternary raster operations',   NOT   ' Binary raster ops'


#define SRCCOPY            dest = source                           -  shows letter,  but the SRC background shows

#define SRCPAINT           dest = source OR dest            -   does  XOR,   but it is the Letter that is transparent    *** 

#define SRCAND             dest = source AND dest           -   drops out the background and Paints the LETTER in BLACK  **********

#define SRCINVERT         dest = source XOR dest           -  does and XOR,  but still is NOT transparent  *****

#define SRCERASE            dest = source AND (NOT dest )    -   shows letter but  background is still showing

#define NOTSRCCOPY        dest = (NOT source)                -   a straight copy of the NEGATIVE image    

#define NOTSRCERASE        dest = (NOT src) AND (NOT dest)   -   does XOR on both  background and Letter

#define MERGECOPY          dest = (source AND pattern)       -  a strat copy of SRC,  both  background and Letter

#define MERGEPAINT         dest = (NOT source) OR dest     -  drops out Backgground,  but  paints the Letter in White

#define PATCOPY             dest = pattern                -   just a white  BoundBox-Rectangle  

#define PATPAINT            dest = DPSnoo                  -   just a white  BoundBox-Rectangle  

#define PATINVERT         dest = pattern XOR dest       -  XOR's the entire  BoundBox-Rectangle  

#define DSTINVERT          dest = (NOT dest)       -     -   just a XOR  BoundBox-Rectangle  

#define BLACKNESS          dest = BLACK   

#define WHITENESS          dest = WHITE       


007801E5     PDSax    -   Nothing

008701C5    PDSaxn

00220326     DSna
****/


					//	 MaskBlt(   int x, int y,   int nWidth, int nHeight,    CDC* pSrcDC,   int xSrc, int ySrc,    CBitmap& maskBitmap, int xMask, int yMask,   DWORD dwRop );


	int  result  = 	 dc->MaskBlt(   xOffset, yOffset,    destWidth,  destHeight,     &memDC,    destWidth,  destHeight,   	fontMask,   0,0,   rasterOp  );      



	if(    result  ==   GDI_ERROR    )     //    GDI_ERROR  =     (0xFFFFFFFFL)
	{  
		DWORD  error  =    GetLastError();    // Only for NT !!!!
		CString  strError;
		strError.Format(  " Blit_Text_with_MaskBlt  FAILED:  MaskBlt,   [  ErrorNum:  %d  ]  \n"  ,   error   );
		TRACE(  strError  );	
		//   Restore and delete intermediate Bitmap and HDC
//		SelectObject(  memDC,  oldBMap  );
//		DeleteObject(  memBMap      );
//		DeleteDC(       memDC  );
		return  false;
	}

																				//   CLEANUP
	memDC.SelectObject(   oldBitmap   );

	return  true;
}





											////////////////////////////////////////
											////////////////////////////////////////


void    Blit_Offmap_2PCwindow_TransparentBackground(   OffMap *offMap,   void  *CDevC,    RectShrt&  srcRect,    RectShrt&  dstRect   )
{

					//   NEW,  4/08.    To NOT paint the background color(white).  Must create an intermediate DEVICEbitmap in memory to convert the DIBitmap (offMap).

														// ******* WANT  +1  ( see below ???? *********************   JPM
	if(       offMap ==  NULL
		||   CDevC ==  NULL    )
	{
		ASSERT( 0 );
		return;
	}

	CDC   *devContext =   ( CDC* )CDevC;    




	int  srcWidth  =    ( srcRect.right      -  srcRect.left )  +1;         // width of source  rectangle [ +1 :  Inclusive Counting ]    
	int  srcHeight =    ( srcRect.bottom  -  srcRect.top )  +1;    	


	int XDest =   dstRect.left;                   // x-coord of destination upper-left corner
	int YDest =   dstRect.top;                    // y-coord of destination upper-left corner

	int nDestWidth =   ( dstRect.right      -    dstRect.left )   +1;     // ******* WANT  +1  ?? [ +1 :  Inclusive Counting ]       // width of destination rectangle	 		  
	int nDestHeight =  ( dstRect.bottom   -   dstRect.top )   +1;              // height of destination rectangle




							//  the input  RectShrt  was in VIRTUAL-coords,  so must Invert for the UPSIDE-DOWN Windows DIBs

	int   yTopTrans       =    ( offMap->m_height  -1 )    -    srcRect.top; 
	int   yBottomTrans  =    ( offMap->m_height  -1 )    -    srcRect.bottom; 

	int   YSrc           =        yBottomTrans;                                      // y-coord of source upper-left corner
	int   nSrcHeight  =      ( yTopTrans  -  yBottomTrans )  +1;              // height of source rectangle,  [ +1 :  Inclusive Counting ]


	int    XSrc         =      srcRect.left;                                      // x-coord of source upper-left corner
	int    nSrcWidth =    ( srcRect.right  -  srcRect.left )  +1;         // width of source  rectangle [ +1 :  Inclusive Counting ]    




												//   Create an intermediate bitmap and HDC, similar to DISPLAY'S  bitmap,     to receive conversion from the DIB format

	HDC         memDC    =     CreateCompatibleDC (       *devContext  );     //   memDC =    CreateCompatibleDC( ScreenDC);

	HBITMAP  memBMap =     CreateCompatibleBitmap(   *devContext ,   srcWidth,  srcHeight  );  //  Create an intermediate bitmap, similar to display bitmap,  to convert from the DIB format

	HGDIOBJ  oldBMap    =     SelectObject(  memDC,   memBMap  );   



	/***							//   Get the DIB's palette, then select it into DC       *******   NEED this ??????  4/8/08  ********
	if (  pPal != NULL)
	{
		hPal = (HPALETTE) pPal->m_hObject;
														
		hOldPal =   ::SelectPalette(hDC, hPal, TRUE);    // Select as background since we havealready realized in forground if needed
	}
	***/
	

//	  SetStretchBltMode(   hDC,  COLORONCOLOR   );     // Make sure to use the stretching mode best for color pictures      *********   NEED this ??????  4/8/08  ************




	/***
	int  reslt  =    StretchDIBits(    devContext->m_hDC,     // handle to DC

										   XDest,     
										   YDest,       
										   nDestWidth,       
										   nDestHeight,        

										   XSrc,             
										   YSrc,              
										   nSrcWidth,        
										   nSrcHeight,    

											offMap->Get_Bits_Start(), 
											offMap->Get_BitmapInfo(),
											DIB_RGB_COLORS,  
											SRCCOPY   );
	***/
	
	/****   WORKS
	int  reslt  =    SetDIBitsToDevice(    devContext->m_hDC,     // handle to DC

										   XDest,     
										   YDest,       
										   nSrcWidth,        //  nDestWidth,       ****  Which is best  ***** ????? 
										   nSrcHeight,       //  nDestHeight,        

										   XSrc,             
										   YSrc,              

										   0,                           // nStartScan
										   nSrcHeight,      //  nDestHeight   ****  Which is best  ***** ?????          //   (WORD)DIBHeight(  lpDIBHdr  ),    //  nNumScans

											offMap->Get_Bits_Start(), 
											offMap->Get_BitmapInfo(),
											DIB_RGB_COLORS  );
	*****/

	int  reslt  =    SetDIBitsToDevice(   memDC,     // handle to DC in MEMORY,  for memory bitmap  

										   0,  //  XDest,        ...because this is an INTERMEDIATE bitmap, it is not offset.  Use all SRC-coords, stretch will happen with  TransparentBlt() 
										   0,   // YDest,       
										   nSrcWidth,        //  nDestWidth,       ****  Which is best  ***** ????? 
										   nSrcHeight,       //  nDestHeight,        

										   0,   // XSrc,             
										   0,   //  YSrc,              

										   0,                    //  nStartScan
										   nSrcHeight,      //  nDestHeight   ****  Which is best  ***** ?????          //   (WORD)DIBHeight(  lpDIBHdr  ),    //  nNumScans

											offMap->Get_Bits_Start(), 
											offMap->Get_BitmapInfo(),
											DIB_RGB_COLORS  );



//           BOOL    BitBlt( __in HDC hdc, __in int x, __in int y, __in int cx, __in int cy, __in_opt HDC hdcSrc, __in int x1, __in int y1, __in DWORD rop);

//	int  reslt2  =    BitBlt(   devContext->m_hDC,    XDest, YDest,    nSrcWidth, nSrcHeight,     memDC,    0, 0,   SRCCOPY  );  // *** TEST,  that the secondary bitmap works [  ] 



//  BitBlt (                 hdc,    100, 120,    48, 48,     memDC,   0, 0,             SRCCOPY   );
//	 TransparentBlt (   hdc,    100, 120,    48, 48,     memDC,   0, 0,    48, 48,      RGB (255, 255, 255));

//  BOOL  TransparentBlt(    __in HDC hdcDest,__in int xoriginDest, __in int yoriginDest, __in int wDest, __in int hDest, __in HDC hdcSrc,
//                                           __in int xoriginSrc, __in int yoriginSrc, __in int wSrc, __in int hSrc, __in UINT crTransparent);

	int  reslt2  =    TransparentBlt(   devContext->m_hDC,    XDest, YDest,    nDestWidth, nDestHeight,     memDC,   0, 0,    nSrcWidth, nSrcHeight,    RGB(255, 255, 255)    );



	
	if(      reslt   ==   GDI_ERROR   
		||  reslt2  ==   GDI_ERROR    )     //    GDI_ERROR  =     (0xFFFFFFFFL)
	{  
		//   ASSERT( 0 );

		DWORD  error  =    GetLastError();    // Only for NT !!!!

		CString  strError;
		strError.Format(  " ***ERR:   Blit_Offmap_2PCwindow_Transparent  FAILED.    [  ErrorNum:  %d  ]  \n"  ,   error   );
		TRACE(  strError  );		

																//   Restore and delete intermediate Bitmap and HDC
		SelectObject(  memDC,  oldBMap  );
		DeleteObject(  memBMap      );
		DeleteDC(       memDC  );

		return;
	}	


																//   Restore and delete intermediate Bitmap and HDC
	SelectObject(   memDC,    oldBMap  );
    DeleteObject(  memBMap      );
    DeleteDC(       memDC  );
}

/**************************   Keep for a while,  is just sample code   4/08  ***************************

BOOL WINAPI  PaintDIB(    HDC       hDC,
										LPRECT  lpDCRect,
										OffMap *offMap,    //   HDIB     hDIB,
										LPRECT  lpDIBRect,
										CPalette* pPal  )
{
	LPSTR    lpDIBHdr;            // Pointer to BITMAPINFOHEADER
	LPSTR    lpDIBBits;           // Pointer to DIB bits

	BOOL     bSuccess=FALSE;      // Success/fail flag
	HPALETTE hPal=NULL;           // Our DIB's palette
	HPALETTE hOldPal=NULL;        // Previous palette

	// Check for valid DIB handle 
	if (hDIB == NULL)
		return FALSE;



	// Lock down the DIB, and get a pointer to the beginning of the bit  buffer
	 
	lpDIBHdr  = (LPSTR) ::GlobalLock((HGLOBAL) hDIB);
	lpDIBBits = ::FindDIBBits(lpDIBHdr);



	// Get the DIB's palette, then select it into DC
	if (pPal != NULL)
	{
		hPal = (HPALETTE) pPal->m_hObject;

		// Select as background since we have
		// already realized in forground if needed
		hOldPal = ::SelectPalette(hDC, hPal, TRUE);
	}



	// Make sure to use the stretching mode best for color pictures 

	::SetStretchBltMode( hDC, COLORONCOLOR  );


	// Determine whether to call StretchDIBits() or SetDIBitsToDevice() 

	if ((RECTWIDTH(lpDCRect)  == RECTWIDTH(lpDIBRect)) &&
	   (RECTHEIGHT(lpDCRect) == RECTHEIGHT(lpDIBRect)))
		bSuccess = ::SetDIBitsToDevice(hDC,                    // hDC
								   lpDCRect->left,             // DestX
								   lpDCRect->top,              // DestY
								   RECTWIDTH(lpDCRect),        // nDestWidth
								   RECTHEIGHT(lpDCRect),       // nDestHeight
								   lpDIBRect->left,            // SrcX
								   (int)DIBHeight(lpDIBHdr) -
									  lpDIBRect->top -
									  RECTHEIGHT(lpDIBRect),   // SrcY
								   0,                          // nStartScan
								   (WORD)DIBHeight(lpDIBHdr),  // nNumScans
								   lpDIBBits,                  // lpBits
								   (LPBITMAPINFO)lpDIBHdr,     // lpBitsInfo
								   DIB_RGB_COLORS);            // wUsage
   else
	  bSuccess = ::StretchDIBits(hDC,                          // hDC
							   lpDCRect->left,                 // DestX
							   lpDCRect->top,                  // DestY
							   RECTWIDTH(lpDCRect),            // nDestWidth
							   RECTHEIGHT(lpDCRect),           // nDestHeight
							   lpDIBRect->left,                // SrcX
							   lpDIBRect->top,                 // SrcY
							   RECTWIDTH(lpDIBRect),           // wSrcWidth
							   RECTHEIGHT(lpDIBRect),          // wSrcHeight
							   lpDIBBits,                      // lpBits
							   (LPBITMAPINFO)lpDIBHdr,         // lpBitsInfo
							   DIB_RGB_COLORS,                 // wUsage
							   SRCCOPY);                       // dwROP

   ::GlobalUnlock((HGLOBAL) hDIB);

	// Reselect old palette 
	if (hOldPal != NULL)
	{
		::SelectPalette(hDC, hOldPal, TRUE);
	}

   return bSuccess;
}
****/



											////////////////////////////////////////


void    Blit_Offmap_2PCwindow_Summed_RGB_Values(   OffMap *offMap,   void  *CDevC,    RectShrt&  srcRect,    RectShrt&  dstRect,
																			                   short  hueOffset,  short  saturationOffset,  short valueOffset,   
																							        short  regionCode 	)
{
			//		regionCode-     0- of no consequence here.      1- is a PencilOutline region


	bool   useSquaredPencilReducer =   false;      //  ****************  ADJUST  (true give fainter pencil lines in dark watercolor washes ********************



	bool   drawBrownOutlines =  true;   // ********  NEW   12/10 ******************

	short		  hueBrown           =       44;             //            ****** ADJUST,   move to the top
	short       saturationBrown  =     168;           //  168     ****** ADJUST,   move to the top     66% = 168    50% = 128




					//   NEW,  8/08.   Create an intermediate DEVICEbitmap in memory to convert the DIBitmap (offMap).

														// ******* WANT  +1  ( see below ???? *********************   JPM

	bool   useExtraBitmap =     false;   // more efficient if i read directly from the Offmap, and not create a separate MSoft bitmap.   8/08

	if(       offMap ==  NULL
		||   CDevC ==  NULL    )
	{
		ASSERT( 0 );
		return;
	}

	CDC   *devContext =   ( CDC* )CDevC;    




	int  srcWidth  =    ( srcRect.right      -  srcRect.left )  +1;         // width of source  rectangle [ +1 :  Inclusive Counting ]    
	int  srcHeight =    ( srcRect.bottom  -  srcRect.top )  +1;    	


	int XDest =   dstRect.left;                   // x-coord of destination upper-left corner
	int YDest =   dstRect.top;                    // y-coord of destination upper-left corner

	int nDestWidth =   ( dstRect.right      -    dstRect.left )   +1;     // ******* WANT  +1  ?? [ +1 :  Inclusive Counting ]       // width of destination rectangle	 		  
	int nDestHeight =  ( dstRect.bottom   -   dstRect.top )   +1;              // height of destination rectangle




							//  the input  RectShrt  was in VIRTUAL-coords,  so must Invert for the UPSIDE-DOWN Windows DIBs

	int   yTopTrans       =    ( offMap->m_height  -1 )    -    srcRect.top; 
	int   yBottomTrans  =    ( offMap->m_height  -1 )    -    srcRect.bottom; 

	int   YSrc           =        yBottomTrans;                                      // y-coord of source upper-left corner
	int   nSrcHeight  =      ( yTopTrans  -  yBottomTrans )  +1;              // height of source rectangle,  [ +1 :  Inclusive Counting ]


	int    XSrc         =      srcRect.left;                                      // x-coord of source upper-left corner
	int    nSrcWidth =    ( srcRect.right  -  srcRect.left )  +1;         // width of source  rectangle [ +1 :  Inclusive Counting ]    




												//   Create an intermediate bitmap and HDC, similar to display bitmap,  to receive conversion from the DIB format

	HDC          memDC;  
	HBITMAP   memBMap;
	HGDIOBJ   oldBMap; 


	if(   useExtraBitmap   )
	{
		memDC    =     CreateCompatibleDC (       *devContext  );     //   memDC =    CreateCompatibleDC( ScreenDC);

		memBMap =     CreateCompatibleBitmap(   *devContext ,   srcWidth,  srcHeight  );  //  Create an intermediate bitmap, similar to display bitmap,  to convert from the DIB format

		oldBMap    =     SelectObject(  memDC,   memBMap  );   



		int  reslt  =    SetDIBitsToDevice(   memDC,     // handle to DC in MEMORY,  for memory bitmap  

											   0,  //  XDest,        ...because this is an INTERMEDIATE bitmap, it is not offset.  Use all SRC-coords, stretch will happen with  TransparentBlt() 
											   0,   // YDest,       
											   nSrcWidth,        //  nDestWidth,       ****  Which is best  ***** ????? 
											   nSrcHeight,       //  nDestHeight,        

											   0,   // XSrc,             
											   0,   //  YSrc,              

											   0,                    //  nStartScan
											   nSrcHeight,      //  nDestHeight   ****  Which is best  ***** ?????          //   (WORD)DIBHeight(  lpDIBHdr  ),    //  nNumScans

												offMap->Get_Bits_Start(), 
												offMap->Get_BitmapInfo(),
												DIB_RGB_COLORS  );
		
		if(      reslt   ==   GDI_ERROR   )     //    GDI_ERROR  =     (0xFFFFFFFFL)
		{  
			DWORD  error  =    GetLastError();    // Only for NT !!!!

			CString  strError;
			strError.Format(  " ***ERR:   Blit_Offmap_2PCwindow_Transparent  FAILED.    [  ErrorNum:  %d  ]  \n"  ,   error   );
			TRACE(  strError  );		
																	//   Restore and delete intermediate Bitmap and HDC
			SelectObject(  memDC,  oldBMap  );
			DeleteObject(  memBMap      );
			DeleteDC(       memDC  );
			return;
		}	
	}   //   if(  useExtraBitmap  )





	                 //  Now traverse the DEST-pixels and subtract out the ColorComponent(RGB) light that the SRCregion's pixels would subtract.     8/08

	int       x=0,   y=0;
	int       xDestMap,  yDestMap;
	int       xSrcMap,   ySrcMap;
	COLORREF  dstPixelRGB;


	for(   y=0;     y< nDestHeight;     y++   )    // Traverse the DESTmap's  coords
	{
		for(   x=0;     x< nDestWidth;     x++   )
		{
				
			xDestMap  =    dstRect.left  +  x;       //  get DEST pixel 
			yDestMap  =    dstRect.top  +  y; 

			xSrcMap =    ( x  *  nSrcWidth )    /  nDestWidth;     //  scale back to get the   Region's SRCmap coords
			ySrcMap =    ( y  *  nSrcHeight )   /  nDestHeight;




			short   srcRed,  srcGreen,   srcBlue;    
			COLORREF    srcPixelRGB;

			if(   useExtraBitmap   )
			{
				srcPixelRGB  =     GetPixel(    memDC,      xSrcMap,    ySrcMap  );

				srcRed    =     GetRValue(  srcPixelRGB  ); 
				srcGreen =     GetGValue(  srcPixelRGB  ); 
				srcBlue    =     GetBValue(  srcPixelRGB  ); 
			}



			short   srcRedJM,  srcGreenJM,   srcBlueJM;    //   faster if use my function ???


//			offMap->Read_Pixel(                         xSrcMap,  ySrcMap,                                                                  &srcRedJM,  &srcGreenJM,  &srcBlueJM    );     // Now is fixed,  8/19/08
			offMap->Read_Pixel_ColorModified(   xSrcMap,  ySrcMap,     hueOffset, saturationOffset, valueOffset,      srcRedJM,  srcGreenJM,  srcBlueJM  );



			srcBlue     =    srcBlueJM ;    //  just reassign the values
			srcGreen  =    srcGreenJM;
			srcRed     =    srcRedJM ;





			if(     ! (  srcRed ==  255   &&    srcGreen ==  255  &&    srcBlue ==  255  )     )     // Only if SRC pixel is NOT white, do we mix it with the DSTpixel's value
			{

				dstPixelRGB  =     GetPixel(    devContext->m_hDC,     xDestMap,  yDestMap  );

				BYTE  dstRed    =     GetRValue(  dstPixelRGB  ); 
				BYTE  dstGreen =     GetGValue(  dstPixelRGB  ); 
				BYTE  dstBlue    =     GetBValue(  dstPixelRGB  ); 


						//							R         G       B
						//
						//		Cyan is    (     0,     255,    255    ) 
						//		Yellow     (   255,    255,       0    ) 
						//		Magenta  (   255,        0,    255     ) 
 

						//  "Since ideally CMY is the complement of RGB, the following linear equations (known also as masking equations) were initially used to convert between RGB and CMY:"
						//
						//	   C = 1 - R				R = 1 - C
						//	   M = 1 - G				G = 1 - M
						//	   Y = 1 - B				B = 1 - Y



				/*****   BAD,  want it to return to ColorBit boundaries

				short  nuRed     =     (short)dstRed       -    ( 255 -  (short)srcRed );    //  The   'TOP'[255 - srcRed]     has the amount of component light that will be filtered out by SRCpixel
				short  nuGreen  =     (short)dstGreen   -    ( 255 -  (short)srcGreen);
				short  nuBlue     =     (short)dstBlue     -    ( 255 -  (short)srcBlue);
				*****/
				short   nuRed =0,     nuGreen =0,     nuBlue =0;



				short  redSubtract      =     256 -  (short)srcRed;   // *** also see  OffMap::Subtract_Pixels_Color()   for same ALGEBRA of color subtraction *****************
				short  greenSubtract  =     256 -  (short)srcGreen;
				short  blueSubtract    =     256 -  (short)srcBlue;

				if(   srcRed    == 255 )      redSubtract =  0;       //  to keep on boundaries
				if(   srcGreen == 255 )     greenSubtract =  0; 
				if(   srcBlue    == 255 )     blueSubtract =  0; 





				short  valueThreshold =   255;       //    255:  do for all values      ****** ADJUST ???  Or just keep permanent value of 255?     7/09   ************************

				/***
				short  retHue,  retSaturation,  retDestValue; 
				OffMap::RGB_to_HSV(    dstRed,  dstGreen,  dstBlue,      retHue,  retSaturation,  retDestValue    );  //  retValue is just the MAXIMUM of { dstRed,  dstGreen,  dstBlue } in  RGB_to_HSV()
				****/
				short        retDestValue  =    max3j(   dstRed,   dstGreen,   dstBlue   );	            //  retValue is just the MAXIMUM of { dstRed,  dstGreen,  dstBlue }

				ASSERT(   retDestValue >=  0      &&    retDestValue  <=  255   );





				if(        regionCode ==  1          //   regionSubj->m_renderStyle == RegionSubject::PENCILlINE    ...Special case for rendering. Do NOT want the pencil to be too dark over dark watercolor washes.  7/09
					 &&  	 retDestValue  <=  valueThreshold   ) 	                            
				{															//   WANT to test that the color does NOT get too dark, and make it lighter if that is the case.    7/15/09
																			//			( for this to work right, the pencil regions should be done LAST in the rendering order.


					short     whitenessThreshold  =     230;       //  200     **** ADJUST *****     230[good]

					double   reducerPerCent       =      1.5;   //   ************  TEMP     1.0;           **** ADJUST *****




					short  graySubtractValue  =       256   -   srcRed;     //   default

					double   reduxFactor        =     (double)retDestValue    /    (double)valueThreshold;  



					if(     srcRed   <   whitenessThreshold    )     //   [ 170 ]   Is there a real pencil line here, and not just a lite grey background
					{



						if(    drawBrownOutlines    )     //  this will make Outline Seps be drawn in brown and not black ink
						{

							short  retRdBrown,   retGrBrown,    retBlBrown; 

							OffMap::HSV_to_RGB(   hueBrown,  saturationBrown,   srcRedJM,      retRdBrown,  retGrBrown,  retBlBrown   );  // ***** ?????  valueOffset  ???????????   





// *************** FIX and clean up this (  see below logic )    12/16    [  Blit_Offmap_2PCwindow_Summed_RGB_Values()    ]    ***************************************************




							if(    retDestValue  <=    190   )     // ******   ADJUST   [170 not bad
							{

								if(     useSquaredPencilReducer     )
									reducerPerCent =    reduxFactor   *   reduxFactor;   //  a SQUARED relationship
								else
									reducerPerCent =    reduxFactor;                            //  a LINEAR relationship
							}

							reducerPerCent =   1.0;     // ******* TEMP,  for brown,  force lines to be lighter and NOT apply 'reducer'




							/***
							short  redSubtractBrn      =     256 -  (short)retRdBrown;  
							short  greenSubtractBrn  =     256 -  (short)retGrBrown;
							short  blueSubtractBrn    =     256 -  (short)retBlBrown;
							****/
							short  redSubtractBrn      =      (short)(      reducerPerCent    *       (double)(     ( 256 - (short)retRdBrown )    )          ); 
							short  greenSubtractBrn  =      (short)(      reducerPerCent    *       (double)(     ( 256 - (short)retGrBrown )    )          );  
							short  blueSubtractBrn    =      (short)(      reducerPerCent    *       (double)(     ( 256 - (short)retBlBrown )    )          ); 



							
							nuRed     =	  (short)dstRed     -    redSubtractBrn;		   //  similar to   (short)dstRed     -   redSubtract;    ...down below    
							nuGreen =      (short)dstGreen  -    greenSubtractBrn;
							nuBlue    =     (short)dstBlue     -    blueSubtractBrn;							
							/***
							nuRed     =	  retRdBrown;		   //   **** TEMP ******  ?????   OR   Just assign the brown,  do NOT make darker
							nuGreen =      retGrBrown;
							nuBlue    =     retBlBrown;
							***/


						}
						else    
						{        //  this is the old default for gray lines

							if(    retDestValue  <=    190   )     // ******   ADJUST   [170 not bad
							{

								if(     useSquaredPencilReducer     )
									reducerPerCent =    reduxFactor   *   reduxFactor;   //  a SQUARED relationship
								else
									reducerPerCent =    reduxFactor;                            //  a LINEAR relationship
							}


							graySubtractValue  =      (short)(      reducerPerCent    *       (double)(     ( 256 - (short)srcRed )    )          );  //  since the PencilOutline is gray,  { srcRed, srcGreen,  srcBlue } should all have similar values


							nuRed     =	  (short)dstRed     -    graySubtractValue;		   //  similar to   (short)dstRed     -   redSubtract;    ...down below    
							nuGreen =      (short)dstGreen  -    graySubtractValue;
							nuBlue    =     (short)dstBlue    -    graySubtractValue;
						}


					}
					else
					{											//   Just some very lite grey background
						graySubtractValue  =  0;

						nuRed     =	  (short)dstRed     -    graySubtractValue;		   //  similar to   (short)dstRed     -   redSubtract;    ...down below    
						nuGreen =      (short)dstGreen  -    graySubtractValue;
						nuBlue    =     (short)dstBlue    -    graySubtractValue;
					}





					/***   Moved up to each case   12/16/10

					nuRed     =	  (short)dstRed     -    graySubtractValue;		   //  similar to   (short)dstRed     -   redSubtract;    ...down below    
					nuGreen =      (short)dstGreen  -    graySubtractValue;
					nuBlue    =     (short)dstBlue    -    graySubtractValue;
					******/



					/****
					if(    dstRed ==  255   )        // ******   ???????  not sure if I need these tests  ???????????   ****************
						 nuRed  =     256  -  redSubtract;

					if(    dstGreen ==  255   )
						 nuGreen  =     256  -  greenSubtract;

					if(    dstBlue ==  255    )
						 nuBlue  =     256  -  blueSubtract;
					****/
				}
				else
				{  nuRed     =      (short)dstRed     -   redSubtract;      
					nuGreen  =     (short)dstGreen  -   greenSubtract;
					nuBlue     =     (short)dstBlue    -   blueSubtract;




					if(    dstRed ==  255   )
						 nuRed  =     256  -  redSubtract;

					if(    dstGreen ==  255   )
						 nuGreen  =     256  -  greenSubtract;

					if(    dstBlue ==  255    )
						 nuBlue  =     256  -  blueSubtract;
				}





				if(         nuRed  <  0   )	        nuRed =   0;
				else if(  nuRed  >= 256   )	    nuRed =   255;

				if(         nuGreen  < 0   )	            nuGreen =   0;
				else if(  nuGreen  >= 256   )	    nuGreen =   255;

				if(         nuBlue  <  0   )	        nuBlue =   0;
				else if(  nuBlue  >= 256   )	    nuBlue =   255;


				//  Earlier TEST:    For now just write the src pixels value:	SetPixel(    devContext->m_hDC,       xDestMap,  yDestMap,    srcPixelRGB  );

				SetPixel(    devContext->m_hDC,       xDestMap,  yDestMap,      RGB(  nuRed, nuGreen, nuBlue )      );  //  final,  write out the 
			}   
			
		}
	}


	if(   useExtraBitmap   )
	{
				//   Restore and delete intermediate Bitmap and HDC
		SelectObject(   memDC,    oldBMap  );
		DeleteObject(  memBMap      );
		DeleteDC(       memDC  );
	}
}




											////////////////////////////////////////


unsigned char*   Convert_Bitmap_Bits_from32_to24(   unsigned char  *pBits,    bool drawBlackBorder,    bool  drawBlackMask,      bool  invertMap,    
																																				BITMAPINFOHEADER&  bmiHeader,  	CString&  retErrorMesg   )
{

		// *****  NEW experiment to write a black mask ***********************************************



										//  Convert  pBits  to 24bit color so do not need a color table.
										//  CAREFUL,  need to pad up each scan line to 4 byte boundaries, and also subtract out the extra pixel 
										//  at each row end, and last line as well.


	long  bmpWidthTrue  =      bmiHeader.biWidth      - 1;     // subtract out for later file write
	long  bmpHeightTrue =      bmiHeader.biHeight     - 1;    


	if(  pBits  ==   NULL  )
	{
		retErrorMesg =     "Convert_Bitmap_Bits_from32_to24 failed,  pBits is NULL." ;
		return false;
	}



	long   byRow24Bit =     bmpWidthTrue  * 3;      //   3 bytes per pixel for 24bit color

	if(    (byRow24Bit % 4)   !=   0    )       // Need padding ???    [  pad-up to nearest LONG( 4 bytes )  ]
	{
		long  numLongs =    ( bmpWidthTrue * 3 ) /4;    

		numLongs++;      //  we know we need SOME padbyte(s),  so add another long  
		byRow24Bit  =    numLongs  *  4L;
	}

	long   pixelByteCount24bit  =     bmpHeightTrue   *  byRow24Bit;   

	long   byRow32Bit              =     bmiHeader.biWidth *  4;         // never need padding if always in 4byte pixels( 32bit )   





	unsigned char*    bits24    =        ( unsigned char* )malloc(   pixelByteCount24bit   );    //   ALLOC the memory

	long                    filesize   =        pixelByteCount24bit  +  54L;       //  54  =    sizeof(  BITMAPFILEHEADER  )   +   sizeof(  BITMAPINFOHEADER  ) 



	unsigned char   *src,  *dst;
	bool  writeBlackPix =  false;


	for(    long row = 1;      row < bmiHeader.biHeight;    row++   )   //  1:  Skip the LAST row ( remember a DIB is upside down, so first row in memory is last scan line )
	{
						
																			//  RE-initialize for each row because of padding issues

		if(   invertMap  )    //  need to turn map upside down ?
			src =     pBits    +    (     ( bmiHeader.biHeight   - row  -1)       *  byRow32Bit   );       //  -1:    "Skip the LAST row..." 
		else
			src =     pBits    +    (                        row                              * byRow32Bit   );  
		


		dst =     bits24   +   (    (row -1)  * byRow24Bit   );     //  (row -1) :   ...need to start at  DSTrow = 0,   or will overwrite buffer at end of for loop




		for(   long x=0;     x < bmpWidthTrue;    x++  )
		{

			unsigned char   srcRed,   srcGreen,  srcBlue,   srcLast,     *trav;
			bool                 pixelIsWhite;
			/***
			srcRed    =     *src;
			srcGreen =   *( src + 1L );
			srcBlue    =   *( src + 2L );
			***/
			trav  =  src;
			srcRed    =     *trav;						trav++;
			srcGreen =    *trav;						trav++;
			srcBlue    =    *trav;		                trav++;
			srcLast    =    *trav;	

			if(   srcRed  ==  255     &&     srcGreen  ==  255     &&     srcBlue  ==  255   )      //  a white pixel on the map
				pixelIsWhite =   true;
			else
				pixelIsWhite =   false;
	


			writeBlackPix =  false;     //  for all cases



			if(   drawBlackBorder   )
			{

				if(           x == 0      ||    x ==   (bmpWidthTrue -1)    
					    ||   row == 1   ||    row ==  (bmiHeader.biHeight - 1)     )
					writeBlackPix =   true;
			}



			if(   drawBlackMask   &&    ! pixelIsWhite   )
					writeBlackPix =   true;                  




		/***
			if(  writeBlackPix  )     *dst =  0;
			else                          *dst  =  *src;			
			dst++;   src++;

			if(  writeBlackPix  )     *dst =  0;
			else                          *dst  =  *src;				
			dst++;   src++;

			if(  writeBlackPix  )     *dst =  0;
			else                          *dst  =  *src;			
			dst++;   src++;

			src++;   //  skip the last reserved-byte in 32bit,  and keep going

		*****/

			if(  writeBlackPix  )     *dst =  0;
			else                          
			{  if(  drawBlackMask  )
					*dst =  255;          //  mask mode,  but wring the OTHER color( white )
				else
					*dst =  *src;		//  nomal mode,  copy in value	
			}

			dst++;   src++;



			if(  writeBlackPix  )     *dst =  0;
			else                          
			{  if(  drawBlackMask  )
					*dst =  255;          //  mask mode,  but wring the OTHER color( white )
				else
					*dst =  *src;		//  nomal mode,  copy in value	
			}

			dst++;   src++;



			if(  writeBlackPix  )     *dst =  0;
			else                          
			{  if(  drawBlackMask  )
					*dst =  255;          //  mask mode,  but wring the OTHER color( white )
				else
					*dst =  *src;		//  nomal mode,  copy in value	
			}

			dst++;   src++;


			src++;   //  skip the last reserved-byte in 32bit,  and keep going
		}
	}


	return  bits24;
}


											////////////////////////////////////////


unsigned char*   Get_Copy_of_Bitmaps_Bits_HBITMAP(   HBITMAP hBmp,   HDC hDC,    bool drawBlackBorder,   bool  drawBlackMask,   BITMAPINFOHEADER  *retBmInfoHeader,  CString&  retErrorMesg   )
{


	    //  ****** REMBER, have the CALLING FUNCTION    free()   the returned value  **************************



	if(  retBmInfoHeader ==  NULL   )
	{
		retErrorMesg  =    "RenderMan::Save_Bitmap_File failed,   CreateCompatibleBitmap." ;
		return false;
	}



	long  xPelsPerMeter =  	7872;    // *******  HARDWIRED,    FIX,  need to calc this.     NOT sure what this means ************************
	long  yPelsPerMeter =  	7872;   

	long  value1 =  	7870;    
	long  value2 =  	7870;   
												 //  ********* BUG:  Last 2 vars get changed by callGet_DIB_Bits()   *****************************   
	long  value3 =  	7871;      //  gets changed by last call to Get_DIB_Bits
	long  value4 =  	7871;      //  gets changed by last call to Get_DIB_Bits


	retErrorMesg.Empty();


																	// Let the graphics engine to retrieve the dimension of the bitmap for us          
																	// GetDIBits uses the size to determine if it's BITMAPCOREINFO or BITMAPINFO
																	// if BitCount != 0, color table will be retrieved
    BITMAPINFO  bmi;
    
    bmi.bmiHeader.biSize       =    0x28;        //   0x28 = 40,   GDI need this to work.
    bmi.bmiHeader.biBitCount =    0;             //   0:   don't get the color table

    if(  (   GetDIBits(     hDC,    
		                         hBmp,      
						         0, 0, 
							     ( LPSTR )NULL, 
							    &bmi, 
							      DIB_RGB_COLORS  )  ) == 0   ) 
	{  retErrorMesg  =    "RenderMan::Save_Bitmap_File failed,  GetDIBits #1." ;
       return NULL;
    }



		// ****** PROBLEM:     with calls GetBitmap()  and GetDIBits() is that they make the bitmap an extra +1 in both dimensions and just put black pixels
		// *************        In the far right row's pixel and the last scan line.  Want to trim out the last pixel and line with values below.   8/08

	long  retBmpWidthTrue  =      bmi.bmiHeader.biWidth      - 1;     // subtract out for later file write
	long  retBmpHeightTrue =      bmi.bmiHeader.biHeight     - 1;    



	long                    pixelByteCount =    bmi.bmiHeader.biSizeImage;  //  NEED bmi's biSizeImage or  GetDIBits() will fail

	unsigned char*    pBits =        ( unsigned char* )malloc(   pixelByteCount   );   




													//     "A bitmap can't (already?) be selected into a DC when calling GetDIBits."  
													//
													//   "Assume that the hDC is the DC where the bitmap would have been selected, if indeed it has been selected"

    HBITMAP   hTmpBmp,  hBmpOld; 
    
    if(    ( hTmpBmp =   CreateCompatibleBitmap(   hDC,     bmi.bmiHeader.biWidth,   bmi.bmiHeader.biHeight  )   )  ==  0  )
	{	
		free( pBits );
		retErrorMesg  =    "RenderMan::Save_Bitmap_File failed,   CreateCompatibleBitmap." ;
		return false;
    }


    hBmpOld =   ( HBITMAP )SelectObject(  hDC,   hTmpBmp  );    //  Select the temp-Bitmap into the hDC so that hBmp is FREE for GetDIBits() to work on  




    if(  (    GetDIBits(    hDC,								  // This will finally copy the CBitmap's bits to the  pBits's buffer 
		                         hBmp,    
			                     0,    bmi.bmiHeader.biHeight, 
					           ( LPSTR )pBits, 
							     &bmi, 
								  DIB_RGB_COLORS  )   )  ==0   )    // *****???? This call changes the value for bitMapStruct.Width to 255???  why
	{  
	    SelectObject(  hDC,  hBmpOld  );     
	    DeleteObject(  hTmpBmp  );		
		free( pBits );
		retErrorMesg  =    "RenderMan::Save_Bitmap_File failed,   GetDIBits #2." ;
		return NULL;
	}




//	BITMAPINFOHEADER  bmInfoHeader;

	retBmInfoHeader->biSize    =   40; 	
	retBmInfoHeader->biWidth  =   bmi.bmiHeader.biWidth;         //  input to Convert_Bitmap_Bits_from32_to24()  the values from the 32bit map
	retBmInfoHeader->biHeight =   bmi.bmiHeader.biHeight;    
	retBmInfoHeader->biPlanes =       1;	
	retBmInfoHeader->biBitCount =   24;	

	retBmInfoHeader->biCompression =   BI_RGB;     //  0
	retBmInfoHeader->biSizeImage =  0; 	//   This may be set to zero for BI_RGB bitmaps. 

	retBmInfoHeader->biXPelsPerMeter =      xPelsPerMeter;    // ******* NOT sure what this means ************************
	retBmInfoHeader->biYPelsPerMeter =      yPelsPerMeter;    // ******* NOT sure what this means ************************
	retBmInfoHeader->biClrUsed  = 	0;
	retBmInfoHeader->biClrImportant  =  0;	




	 bool  invertMap =   false;

	unsigned char  *bits24   =     Convert_Bitmap_Bits_from32_to24(   pBits,    drawBlackBorder,   drawBlackMask,    invertMap,   *retBmInfoHeader,    retErrorMesg   );
	if(                     bits24 ==  NULL  )
	{

		SelectObject(  hDC,  hBmpOld  );      // Now that the conversion is complete, we can return the bitmap to its hDC and free the pBits that we extracted
		DeleteObject(  hTmpBmp  );

		free(  pBits  );   
		return  false;
	}




	retBmInfoHeader->biWidth   =    retBmpWidthTrue;    //  correct for the 24bit map
	retBmInfoHeader->biHeight  =    retBmpHeightTrue;    

	
    SelectObject(  hDC,  hBmpOld  );      // Now that the conversion is complete, we can return the bitmap to its hDC and free the pBits that we extracted
    DeleteObject(  hTmpBmp  );

	free(  pBits  );   


	return  bits24;
}



											////////////////////////////////////////


unsigned char*   Get_Copy_of_Bitmaps_Bits_CBitmap(   CBitmap&  msBitMap,   HDC hDC,   bool drawBlackBorder,    bool  drawBlackMask,    BITMAPINFOHEADER  *retBmInfoHeader,  
																																	                                    CString&  retErrorMesg   )
{

	
	    //  ****** REMEMBER, have the CALLING FUNCTION    free()   the returned value  **************************

	ASSERT(  retBmInfoHeader  );



	long  xPelsPerMeter =  	7872;    // *******  HARDWIRED,    FIX,  need to calc this.     NOT sure what this means ************************
	long  yPelsPerMeter =  	7872;   

	long  value1 =  	7870;    
	long  value2 =  	7870;   
												 //  ********* BUG:  Last 2 vars get changed by callGet_DIB_Bits()   *****************************   
	long  value3 =  	7871;      //  gets changed by last call to Get_DIB_Bits
	long  value4 =  	7871;      //  gets changed by last call to Get_DIB_Bits


	retErrorMesg.Empty();




	BITMAP   bitMapStructExper;    // NOT really used, but keep aropund for sample code.  Actually this function and result structure are buggy.

	if(    msBitMap.GetBitmap(  &bitMapStructExper  )   ==  0   )                 //  fails to get a pointer to the 'bits', but gets other BMap info
	{
		retErrorMesg  =    "RenderMan::Save_Bitmap_File failed,  GetBitmap." ;
		return false;
	}                         // Bad for map's dimensions,  sometimes data changes in this function????  See notes at GetDIBits()   8/08 




	DWORD            dwCount  =      bitMapStructExper.bmWidth   *   bitMapStructExper.bmHeight   *   4;     //  4 bytes for 32bit color 

	unsigned char  *pBits      =      ( unsigned char* )malloc(   dwCount   );    //   ALLOC the memory


	DWORD            result     =      msBitMap.GetBitmapBits(   dwCount,   pBits  );     //  This will return the bits in Upside down fashon ( DIB ).





	long   retBmpWidthTrue    =    bitMapStructExper.bmWidth   - 1;
	long   retBmpHeightTrue   =    bitMapStructExper.bmHeight  - 1;



												//  Need to fill this out for  Convert_Bitmap_Bits_from32_to24()

	retBmInfoHeader->biSize    =   40; 	
	retBmInfoHeader->biWidth  =   bitMapStructExper.bmWidth;         //  input to Convert_Bitmap_Bits_from32_to24()  the values from the 32bit map
	retBmInfoHeader->biHeight =   bitMapStructExper.bmHeight;    
	retBmInfoHeader->biPlanes =       1;	
	retBmInfoHeader->biBitCount =   24;	

	retBmInfoHeader->biCompression =   BI_RGB;     //  0
	retBmInfoHeader->biSizeImage =  0; 	//   This may be set to zero for BI_RGB bitmaps. 

	retBmInfoHeader->biXPelsPerMeter =      xPelsPerMeter;    // ******* NOT sure what this means ************************
	retBmInfoHeader->biYPelsPerMeter =      yPelsPerMeter;    // ******* NOT sure what this means ************************
	retBmInfoHeader->biClrUsed  = 	0;
	retBmInfoHeader->biClrImportant  =  0;	




	bool  invertMap =   true;

	unsigned char  *bits24   =     Convert_Bitmap_Bits_from32_to24(   pBits,    drawBlackBorder,  drawBlackMask,    invertMap,  *retBmInfoHeader,    retErrorMesg   );
	if(                     bits24 ==  NULL  )
	{
		free(  pBits  );   
		return  false;
	}



	retBmInfoHeader->biWidth   =    retBmpWidthTrue;    //  correct for the 24bit map
	retBmInfoHeader->biHeight  =    retBmpHeightTrue;    

	free(  pBits  );   


	return  bits24;
}





											////////////////////////////////////////
											////////////////////////////////////////


void     Xor_Line(   short x0,  short y0,     short x1,  short y1,   short  penWidth,   void  *CDevC  )
{

	//   ***CAREFUL,  am I including the last and first points ??? 


	if(  CDevC  ==  NULL  )
	{
		//   ASSERT( 0 );    ...Handles sometime do this cause i do not first set to NOT visable
		return;
	}
	CDC   *devContext =   ( CDC* )CDevC;           //   ****   CAREFUL with a cast !!! *******



	int  nDrawMode =    devContext->GetROP2();

	CPen   penBlack;  
    if(      !penBlack.CreatePen(  PS_SOLID,   penWidth,   RGB(0,0,0)    )      )
    {
		ASSERT( 0 );
		int  dummyBreak = 9;
	}

												// Select it into the device context.  Save the old pen at the same time
    CPen   *pOldPen =    devContext->SelectObject(  &penBlack  );
   
	devContext->SetROP2(   R2_NOT    );       //   R2_NOT    R2_XORPEN    


	devContext->MoveTo(   x0,   y0    );     
	devContext->LineTo(     x1,   y1    );   


	devContext->SelectObject( pOldPen );           // Restore the old pen to the device context
	devContext->SetROP2(  nDrawMode   );
}



											////////////////////////////////////////


void     Xor_Line_Fancy(   short x0,  short y0,     short x1,  short y1,   short  penWidth,   void  *CDevC,    int   fancyCode  )
{


			//   fancyCode =  {    PS_SOLID,    PS_DASH,    PS_DOT


	//   ***CAREFUL,  am I including the last aned first points ??? 


	if(  CDevC  ==  NULL  )
	{
		//   ASSERT( 0 );    ...Handles sometime do this cause i do not first set to NOT visable
		return;
	}
	CDC   *devContext =   ( CDC* )CDevC;           //   ****   CAREFUL with a cast !!! *******



	int  nDrawMode =    devContext->GetROP2();

	CPen   penBlack;  
    if(      !penBlack.CreatePen(    fancyCode,     penWidth,   RGB(0,0,0)    )      )
    {
		ASSERT( 0 );
		int  dummyBreak = 9;
	}

												// Select it into the device context.  Save the old pen at the same time
    CPen   *pOldPen =    devContext->SelectObject(  &penBlack  );
   
	devContext->SetROP2(   R2_NOT    );       //   R2_NOT    R2_XORPEN    


	devContext->MoveTo(   x0,   y0    );     
	devContext->LineTo(     x1,   y1    );   


	devContext->SelectObject( pOldPen );           // Restore the old pen to the device context
	devContext->SetROP2(  nDrawMode   );
}






											////////////////////////////////////////


void     Paint_Arc_for_MusicalTie(   short x0, short x1,   short y,     short  halfHeight,
																short red,  short green,  short  blue,     short  penWidth,   void  *CDevC  )
{

	//   ***CAREFUL,  am I including the last aned first points ??? 


	if(  CDevC  ==  NULL  )
	{
		//   ASSERT( 0 );    ...Handles sometime do this cause i do not first set to NOT visable
		return;
	}
	CDC   *devContext =   ( CDC* )CDevC;           //   ****   CAREFUL with a cast !!! *******



	int  nDrawMode =    devContext->GetROP2();


	CPen   penBlack;  
    if(      !penBlack.CreatePen(  PS_SOLID,   penWidth,   RGB( red, green, blue )    )      )
    {
		ASSERT( 0 );
		int  dummyBreak = 9;
	}

												// Select it into the device context.  Save the old pen at the same time
    CPen   *pOldPen =    devContext->SelectObject(  &penBlack  );
   


//	devContext->SetROP2(   R2_NOT    );       //   R2_NOT    R2_XORPEN    
	devContext->SetROP2(   R2_COPYPEN    );   // *** OK ???




//	devContext->MoveTo(   x0,   y0    );     
//	devContext->LineTo(     x1,   y1    );   

	/***
	ptStart  -     Specifies the  coordinates of the point that defines the arc’s starting point (in logical units). This point does not have to lie exactly on the arc. You can pass either a POINT structure or a CPoint object for this parameter.

	ptEnd   -      Specifies the   coordinates of the point that defines the arc’s ending point (in logical units). This point does not have to lie exactly on the arc. You can pass either a POINT structure or a CPoint object for this parameter.

	***/

	CRect   rectClient;
		rectClient.left       =    x0;
		rectClient.right     =    x1;
		rectClient.top       =     y   -  halfHeight;
		rectClient.bottom =      y  +  halfHeight;


										//  Draws in COUNTER-Clockwise direction in RECT's bound box

	devContext->Arc(   rectClient,

							//	   CPoint(   rectClient.right,                   rectClient.CenterPoint().y   ),    //  start point
							//	   CPoint(   rectClient.CenterPoint().x,   rectClient.right                   )     //  end point   
								  
								   CPoint(   rectClient.left,     rectClient.CenterPoint().y    ),    //  start point
								   CPoint(   rectClient.right,   rectClient.CenterPoint().y    )     //  end point   
							 );





	devContext->SelectObject( pOldPen );           // Restore the old pen to the device context
	devContext->SetROP2(  nDrawMode   );
}






											////////////////////////////////////////


void     Paint_Line(   short x0,  short y0,     short x1,  short y1,    short red,  short green,  short  blue,   
																							short  penWidth,   void  *CDevC  )
{

	//   ***CAREFUL,  am I including the last aned first points ??? 


	if(  CDevC  ==  NULL  )
	{
		//   ASSERT( 0 );    ...Handles sometime do this cause i do not first set to NOT visable
		return;
	}
	CDC   *devContext =   ( CDC* )CDevC;           //   ****   CAREFUL with a cast !!! *******



	int  nDrawMode =    devContext->GetROP2();


	CPen   penBlack;  
    if(      !penBlack.CreatePen(  PS_SOLID,   penWidth,   RGB( red, green, blue )    )      )
    {
		ASSERT( 0 );
		int  dummyBreak = 9;
	}

												// Select it into the device context.  Save the old pen at the same time
    CPen   *pOldPen =    devContext->SelectObject(  &penBlack  );
   


//	devContext->SetROP2(   R2_NOT    );       //   R2_NOT    R2_XORPEN    
	devContext->SetROP2(   R2_COPYPEN    );   // *** OK ???




	devContext->MoveTo(   x0,   y0    );     
	devContext->LineTo(     x1,   y1    );   


	devContext->SelectObject( pOldPen );           // Restore the old pen to the device context
	devContext->SetROP2(  nDrawMode   );
}



											////////////////////////////////////////


void     Paint_Line_Fancy(   short x0,  short y0,     short x1,  short y1,    short red,  short green,  short  blue,   
												short  penWidth,   void  *CDevC,    int   fancyCode   )
{

		//   fancyCode =  {    PS_SOLID,    PS_DASH,    PS_DOT


	if(  CDevC  ==  NULL  )
	{
		//   ASSERT( 0 );    ...Handles sometime do this cause i do not first set to NOT visable
		return;
	}
	CDC   *devContext =   ( CDC* )CDevC;           //   ****   CAREFUL with a cast !!! *******



	int  nDrawMode =    devContext->GetROP2();


	CPen   penBlack;  
    if(      !penBlack.CreatePen(    fancyCode,     penWidth,   RGB( red, green, blue )    )      )
    {
		ASSERT( 0 );
		int  dummyBreak = 9;
	}

												// Select it into the device context.  Save the old pen at the same time
    CPen   *pOldPen =    devContext->SelectObject(  &penBlack  );
   


//	devContext->SetROP2(   R2_NOT    );       //   R2_NOT    R2_XORPEN    
	devContext->SetROP2(   R2_COPYPEN    );   // *** OK ???




	devContext->MoveTo(   x0,   y0    );     
	devContext->LineTo(     x1,   y1    );   


	devContext->SelectObject( pOldPen );           // Restore the old pen to the device context
	devContext->SetROP2(  nDrawMode   );
}

