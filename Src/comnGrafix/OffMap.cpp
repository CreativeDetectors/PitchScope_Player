/////////////////////////////////////////////////////////////////////////////
//
//  OffMap.cpp   -  for a Bitmap that is stored in memory. Is also used to store data for Transforms like the log DFT (logDFTtrForm ) 
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
      
#include  "..\comnGrafix\CommonGrafix.h"      



#include   "OffMap.h"


#include  "mapFilters.h"


//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////





PairShort  cliquePairsOMAP[ 12 ]  =   
	{
		{0,1},   	{1,2},       {2,3},       {3,4},      {4,5},      {5,6},   	  {6,7},   	  {7,0},     //   'direct'        neighbors
		{1,3},      {3,5},       {5,7},       {7,1}																	 //   'adjacent'    neighbors
	};


short    offsetTableGLB[ 32 ]  =   //   for  Convolution filtering 
{  +1,-1,   +2,-2,   +3,-3,   +4,-4,  +5,-5,   +6,-6,   +7,-7,  +8,-8,   +9,-9,   +10,-10,   +11,-11,   +12,-12,   +13,-13,   +14,-14,   +15,-15,   +16,-16   };




CoordShort   kernalThreeOMAP[ 8 ] =		//  the  8 surrounding( clockwise ) neighborPixel-OFFSETS( from center ) for a 3x3 kernal of Pixels
	{     {-1, -1},     {0, -1},    {1, -1},    {1, 0},      {1, 1},      {0, 1},      {-1, 1},      {-1, 0}     };    





//////////////////////////////////////////////////////////////////////

/*****
short    totalTextureMapMapCountGLB =      12;

textureMapInfo        textureMapsInfGLB[     12    ]    =      //     name,      width,   height             ...List from smallest to biggest
{

	"Gr_small_1",				88,		76,                 //   from    /smooth  folder

	"OrgYel_small_1",		88,		80,

	"Gr_small_2",				82,		137,

	"Red_small_2",			116,		118,

	"Red_small_1",			207,		183,



	"Br_1",		848,			677,  

	"Br_2",		896,			729, 

	"Mg_1", 	921,			752, 

	"B_3", 		1081,		728, 

	"B_2", 		1440,		1265, 

	"B_1",		1680,		1281,

	"V_1",       1832,		1201,

};
******/


short    totalTextureMapMapCountGLB =      16;

textureMapInfo        textureMapsInfGLB[     16    ]    =      //     name,      width,   height             ...List from smallest to biggest
{

	"Orange_sm_1",			30,		30,
	"Orange_sm_2",			30,		30,


	"Green_sm_6",			46,		45,


	"Red_sm_3",				70,		70,
	"Green_sm_7",			70,		70,



//	"OrgYel_small_1",		88,		80,      ...Too varied
	"Gr_small_2",				82,		137,                //   from    /TestMap1  folder



	"Red_small_2",			116,		118,



	"Green_ruf_5",			225,		225,



	"Blue_ruf_4",				300,		300,


	"Green_ruf_3",			406,		338,
	"Green_ruf_4",			455,		448,



	"Blue_ruf_7",				522,		487,


	"Blue_ruf_5",				700,		700,



	"Blue_ruf_6",				1051,	1000,


	"B_2", 						1440,		1265, 


	"B_1",						1680,		1281,

};

						/////////////////////////////////////////////////////////////////////




short   Get_Median(   short  *data,   short  numEnts   );



bool    OffMap::m_seedFillModeColor =   true;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


short   Get_Median(   short  *data,   short  numEnts   )   // **** MOVE this GLOBAL function to another file ??? ******
{

    short  i,j,  median, temp;
    

    if(   numEnts <= 0   )       
		return  0;
    else if(   numEnts == 1   )       
		return  data[ 0 ];
      
     									             
    for(  i= 0;   i< numEnts -1;   ++i  )       //    Find MEDIAN with  BUBBLE SORT
	{ 
		for(  j= numEnts -1;   i< j;   --j  )     
        { 
            if(  data[ j-1 ]  <  data[ j ]   )
            {  
				temp       =   data[ j-1 ]; 
                data[ j-1 ] =   data[ j ];
                data[ j ]   =   temp;
             }
         } 
	}
      

	if(  ( numEnts % 2 ) == 1  )                // ODD, so use  'MIDDLE value' 
		median =      data[  numEnts/2      ];  
    else    
		median =   (  data[  numEnts/2  -1  ]  +  data[ numEnts/2 ]  ) /2;    // get AVERAGE if 'smWnWid' is EVEN                                 

                                             
	return    median;
}






////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


OffMap::OffMap(   long wid,    long heit,    long  depth  )   
									:   m_width( wid ),   m_height( heit ),   m_depth( depth )  
{

		//   for a MSoft WINDOWS  DIB( device Independant Bitmap )  the image is  UPSIDE DOWN,  that is the
		//   the bottom  Y- row will BLIT to the top when displayed.    3/2002


		//   Careful on BIG MEMORY allocations...    [  biggest long value:  2,147,483,647   ]
		//
		//   ...what is the limit for  malloc()  anyway ???    7/06



	BOOL   succs =  FALSE;
	int        i,   palletSize;
	int        wNumColors = 2;      //  FIX: derive from m_depth     ( now HARDWIRED )
                   
	
	m_dib =  NULL;   //  init,   flag that it is EMPTY




	if(   depth  ==  8   )
	{

		wNumColors =  256;      //  FIX: derive from m_depth     ( now HARDWIRED )


		palletSize =    sizeof( PALETTEENTRY )	*  wNumColors; 


		//////////////////////////
		m_byRow =    wid;      //  one byte per pixel( gray )

		if(    (m_byRow % 4)   !=   0    )       // Need padding ???    [  pad-up to nearest LONG( 4 bytes )  ]
		{
			long  numLongs =   wid / 4;    

			numLongs++;      //  we know we need SOME padbyte(s),  so add another long  
			m_byRow  =    numLongs  *  4L;
		}

		m_totMapBytes =   heit  *  m_byRow;   



		//////////////////////////
		ASSERT(   m_totMapBytes >= 1    );

		m_totalAllocatedBytes =   sizeof( BITMAPINFOHEADER )   +   palletSize   +    m_totMapBytes;


		m_dib   =   ( BYTE* )malloc(   m_totalAllocatedBytes   );    //   ALLOC the memory
		if(   m_dib  ==  NULL   )
		{
			ASSERT( 0 );		
			return;
		}

 
		PALETTEENTRY  *palletPtr  =   ( PALETTEENTRY* )(    m_dib   +   sizeof( BITMAPINFOHEADER )    );   //  Offset it

		for(   i= 0;    i < wNumColors;    i++   )
		{
			palletPtr[ i ].peRed    =   i;
			palletPtr[ i ].peGreen =   i;
			palletPtr[ i ].peBlue   =    i;
			palletPtr[ i ].peFlags  =   0;
		}


		BITMAPINFOHEADER   *bmInfoHdr =   (   BITMAPINFOHEADER*   )m_dib;

		bmInfoHdr->biSize   =  40;
		bmInfoHdr->biWidth =   wid;    // better with padded up info ???  
		bmInfoHdr->biHeight =   heit;   
		bmInfoHdr->biPlanes =  1;
		bmInfoHdr->biBitCount =  8;
		bmInfoHdr->biCompression = 0;
		bmInfoHdr->biSizeImage = 0;
		bmInfoHdr->biXPelsPerMeter = 0;
		bmInfoHdr->biYPelsPerMeter = 0;
		bmInfoHdr->biClrUsed =  256;
		bmInfoHdr->biClrImportant = 0;


		m_bits =    m_dib    +   sizeof( BITMAPINFOHEADER )   +   palletSize;
	}

	else if(   depth  ==  1   )
	{		

		long  numLongs =    wid  / 32;        //  8 pixels per byte( monocrome ),    32 pixels per LONG

		if(    (m_byRow % 32)   !=   0    )       //  pad-up to nearest LONG( 4 bytes )  
			numLongs++;      


		m_byRow         =     numLongs    *  4L;    //  4 bytes in a long

		m_totMapBytes =     heit   *   m_byRow;   



		//////////////////////////
		ASSERT(   m_totMapBytes >= 1    );

	//	m_totalAllocatedBytes =   sizeof( BITMAPINFOHEADER )   +   palletSize   +    m_totMapBytes;
		m_totalAllocatedBytes =    m_totMapBytes;

		m_dib  =          ( BYTE* )malloc(   m_totalAllocatedBytes   );   
		if(   m_dib  ==  NULL   )
		{
			ASSERT( 0 );		
			return;
		}

		/***
		PALETTEENTRY  *palletPtr  =   ( PALETTEENTRY* )(    m_dib   +   sizeof( BITMAPINFOHEADER )    );   //  Offset it

		for(   i= 0;    i < wNumColors;    i++   )
		{
			palletPtr[ i ].peRed    =   i;
			palletPtr[ i ].peGreen =   i;
			palletPtr[ i ].peBlue   =    i;
			palletPtr[ i ].peFlags  =   0;
		}


		BITMAPINFOHEADER   *bmInfoHdr =   (   BITMAPINFOHEADER*   )m_dib;

		bmInfoHdr->biSize   =  40;
		bmInfoHdr->biWidth =   wid;    // better with padded up info ???  
		bmInfoHdr->biHeight =   heit;   
		bmInfoHdr->biPlanes =  1;
		bmInfoHdr->biBitCount =  8;
		bmInfoHdr->biCompression = 0;
		bmInfoHdr->biSizeImage = 0;
		bmInfoHdr->biXPelsPerMeter = 0;
		bmInfoHdr->biYPelsPerMeter = 0;
		bmInfoHdr->biClrUsed =  256;
		bmInfoHdr->biClrImportant = 0;
		***/


	//	m_bits =    m_dib    +   sizeof( BITMAPINFOHEADER )   +   palletSize;
		m_bits =    m_dib;
	}

	else if(   depth  ==  24   )
	{

		palletSize =  0;   //   sizeof( PALETTEENTRY )	*  wNumColors; 



		//////////////////////////
		m_byRow =    wid * 3;      //   3 bytes per pixel


		if(    (m_byRow % 4)   !=   0    )       // Need padding ???    [  pad-up to nearest LONG( 4 bytes )  ]
		{
			long  numLongs =   ( wid * 3 ) /4;    

			numLongs++;      //  we know we need SOME padbyte(s),  so add another long  
			m_byRow  =    numLongs  *  4L;
		}

		m_totMapBytes =   heit  *  m_byRow;   





		//////////////////////////
		ASSERT(   m_totMapBytes >= 1    );

		m_totalAllocatedBytes =   sizeof( BITMAPINFOHEADER )   +   palletSize   +    m_totMapBytes;


		m_dib   =   ( BYTE* )malloc(   m_totalAllocatedBytes   );    //   ALLOC the memory
		if(   m_dib  ==  NULL   )
		{
			ASSERT( 0 );		
			return;
		}

		/***
		PALETTEENTRY  *palletPtr  =   ( PALETTEENTRY* )(    m_dib   +   sizeof( BITMAPINFOHEADER )    );   //  Offset it

		for(   i= 0;    i < wNumColors;    i++   )
		{
			palletPtr[ i ].peRed    =   i;
			palletPtr[ i ].peGreen =   i;
			palletPtr[ i ].peBlue   =    i;
			palletPtr[ i ].peFlags  =   0;
		}
		***/


		BITMAPINFOHEADER   *bmInfoHdr =   (   BITMAPINFOHEADER*   )m_dib;

		bmInfoHdr->biSize   =  40;
		bmInfoHdr->biWidth =   wid;    // better with padded up info ???  
		bmInfoHdr->biHeight =   heit;   
		bmInfoHdr->biPlanes =  1;
		bmInfoHdr->biBitCount =  24;
		bmInfoHdr->biCompression = 0;
		bmInfoHdr->biSizeImage = 0;
		bmInfoHdr->biXPelsPerMeter = 0;
		bmInfoHdr->biYPelsPerMeter = 0;
		bmInfoHdr->biClrUsed =  0;				// ******  ?????
		bmInfoHdr->biClrImportant = 0;


		m_bits =    m_dib    +   sizeof( BITMAPINFOHEADER )   +   palletSize;
	}




	Clear(  0  );	   //	Clear(  200  );	   BAD....   For transforms this creates BOGUS data       make it a neurtral gray			                        
}


											////////////////////////////////////////

OffMap::~OffMap()
{
	if(    m_dib  !=   NULL    )
	{
		free(  m_dib  );
	}
}



											////////////////////////////////////////


OffMap&   OffMap::operator=(  OffMap  &other  )
{

	// *** NEED this OVERIDE cause any POINTERS will be wroing when copied byt he BuiltIn  Assignment operator ****


		//   this ASSIGNMENT operator,   is automatically called by ChildClasses


//  *****   Want an Is_OK()  function to verify if it went wrong( couold not get memory ) ????   *****


	if(    other.m_totalAllocatedBytes   <=  0    )
	{
		ASSERT( 0 );   //  BIG mess 
		return  *this;
	}


	if(    m_dib  !=   NULL   )
	{
		free(  m_dib  );     m_dib =  NULL;
	}


	m_totalAllocatedBytes  =  0;   //   init for fail


																								//  Fix the 1st  'BAD POINTER'

	m_dib   =   ( BYTE* )malloc(   other.m_totalAllocatedBytes   );   
	if(   m_dib  ==  NULL   )
	{
		ASSERT( 0 );		
		return   *this;
	}


	m_totalAllocatedBytes  =    other.m_totalAllocatedBytes;

	m_width  =    other.m_width;				
	m_height =    other.m_height;      
	m_depth  =	  other.m_depth;
	m_byRow         =   other.m_byRow;  
	m_totMapBytes =   other.m_totMapBytes;



	for(    int i=0;    i<  m_totalAllocatedBytes;    i++   )
		m_dib[ i ] =    other.m_dib[ i ]; 



																//  Now fix the 2nd  'BAD POINTER'

	if(          m_depth  ==   1   )
	{
		m_bits =    m_dib;        //  No  header
	}
	else if(   m_depth  ==   8    )
	{

		int  wNumColors =  256;      //  FIX: derive from m_depth     ( now HARDWIRED )

		int  palletSize =    sizeof( PALETTEENTRY )	*  wNumColors; 

		m_bits =    m_dib    +   sizeof( BITMAPINFOHEADER )   +   palletSize;    //  this SECOND POINTER must also be fixed 
	}
	else if(   m_depth  ==   24    )
	{
		int  palletSize =   0;    //  no pallet for 

		m_bits =    m_dib    +   sizeof( BITMAPINFOHEADER )   +   palletSize;    //  this SECOND POINTER must also be fixed 
	}
	else
		ASSERT( 0 );


	return  *this;
}


											////////////////////////////////////////


void    OffMap::Get_TextureMaps_Folder(   CString&  retFolderPath  )
{

	retFolderPath  =    "C:\\Users\\JamesM\\TextureMaps\\TestMaps1\\"  ;
}


											////////////////////////////////////////


short    OffMap::Get_Total_TextureMap_Count()
{

	return    totalTextureMapMapCountGLB;
}



											////////////////////////////////////////


textureMapInfo&     OffMap::Get_TextureMap_Array(   short  index   )
{

	short   count  =    Get_Total_TextureMap_Count();


	if(    index  >=    count )
	{
		ASSERT( 0 );

		return   textureMapsInfGLB[   ( count -1 )   ];   //  give it the last in the array and hope for the best
	}


	return   textureMapsInfGLB[ index ];
}


											////////////////////////////////////////


void    OffMap::Copy_Out_Scaled_Bits(   long  scaleDownFactor,    OffMap&  destMap   )
{

		//  reads from THIS offmap,  but writes to the   'smaller 'destMap


			//  OK if maps are different sizes,  but what if scaling factor cause out of bounds read ????    8/08

								
	short  rd,  gr,  bl;  


	for(    long y=0;    y < destMap.m_height;     y++   )
	{
		for(  long x=0;    x < destMap.m_width;     x++   ) 
		{

			Read_Pixel(                     (x * scaleDownFactor),    (y * scaleDownFactor),     &rd,  &gr,  &bl   );  

			destMap.Write_Pixel(         x,                                   y,                                    rd,     gr,   bl   );
		}
	}
}




											////////////////////////////////////////


void    OffMap::Copy_Out_Scaled_And_Modified_Bits(   long  scaleDownFactor,   OffMap&  destMap,   short  hueOffset,  short  saturationOffset,  short valueOffset   )
{


			//  reads from THIS, the RegionSubject's BIG  offmap,  and then writes to the RegionSubject's smaller bitmap ('destMap')

			//  OK if maps are different sizes,  but what if scaling factor cause out of bounds read ????    8/08




	/****************************   Really is not a big deal if too big, because Read_Pixel_ColorModified(0 will correct if the numbers are too big.   

	if(         hueOffset          <= -360
		   ||   saturationOffset <  -255
		   ||   valueOffset        <  -255 

		   ||   hueOffset          >=  360
		   ||   saturationOffset  >  255
		   ||   valueOffset         >  255  		 // ****** BUG here,  can get values over  255 
	  )
	{
		ASSERT( 0 );
		AfxMessageBox(  "OffMap::Copy_Out_Scaled_And_Modified_Bits  failed,  parameters out of range."   );
		return;
	}
	*****/





	if(    hueOffset ==  0    &&   saturationOffset ==  0    &&   valueOffset ==  0   )    //  no color changes, so use the faster function
	{
		Copy_Out_Scaled_Bits(   scaleDownFactor,    destMap  );
		return;
	}




	short  rd,  gr,  bl;


	for(    long y=0;    y < destMap.m_height;     y++   )
	{
		for(  long x=0;    x < destMap.m_width;     x++   ) 
		{

			Read_Pixel_ColorModified(     (x * scaleDownFactor),    (y * scaleDownFactor),    hueOffset,  saturationOffset,  valueOffset,     rd,  gr, bl   );

			destMap.Write_Pixel(   x,  y,      rd,  gr,  bl    );
		}
	}
}




											////////////////////////////////////////


short     OffMap::Calc_Masks_Coverage_In_Percent()
{

			//   What percent of this mask's pixes are BLACK ( have been written to )     Mostly for DEBUG


    short   rd,  gr,  bl,    retPercent = 0;  

	long   blackPixelCnt =  0;



	for(    long y=0;    y < m_height;     y++   )
	{
		for(  long x=0;    x <  m_width;     x++   ) 
		{

			Read_Pixel(  x, y,    &rd,  &gr,  &bl   );  

			if(   rd  ==  0  )
				blackPixelCnt++;
		}
	}



	long   totalPixels =    m_width  *   m_height;

	if(  totalPixels ==  0  )
		return  -1;



	retPercent =    (short)(      (  blackPixelCnt  *  100L  )   /  totalPixels        ); 
	
	return  retPercent;
}



											////////////////////////////////////////


long     OffMap::Calc_Average_RGB_Components(    short&  retAvgRed,    short&  retAvgGreen,    short&  retAvgBlue   )
{


					//  ONLY average for  NON-White( PIGMENTED) pixels.  Since this is a watercolor scan, color should be uniform.    10/08

	
	short		hiCompValue =    Component_Almost_White();    //   250,  is a static variable.  eep consistant throughout the app    

								
	short               rd,  gr,  bl;  
	unsigned long   redTotal = 0,  greenTotal = 0,  blueTotal =   0,      nonWhitePixelCount =  0;



	for(    long y=0;    y < m_height;     y++   )
	{
		for(  long x=0;    x <  m_width;     x++   ) 
		{

			Read_Pixel(  x, y,    &rd,  &gr,  &bl   );  


			if(		   rd   <    hiCompValue			//  **** ONLY average for  NON-White( PIGMENTED) pixels.  Since this is a watercolor scan, color should be uniform
				||    gr    <   hiCompValue
				||    bl    <   hiCompValue    )
			{
				//   isWhite =    false;

				redTotal     +=     ( unsigned long )rd;
				greenTotal  +=     ( unsigned long )gr;
				blueTotal    +=     ( unsigned long )bl;


				nonWhitePixelCount++;
			}
		}
	}




	if(   nonWhitePixelCount  ==    0   )       // careful,  divide by zero, 
	{

	//	ASSERT( 0 );    //  is this an error to call this for no pixels ???    get here if NOT yet built the subtractMap.     10/15/08    

	//	retAvgRed =  retAvgGreen =   retAvgBlue  =   0; 
		retAvgRed =  retAvgGreen =   retAvgBlue  =   255;  // More realistic, if no pigmented pixels,  then this is a white area    11/08

		return   0;
	}



	retAvgRed     =     (short)(    redTotal    /   nonWhitePixelCount     );
	retAvgGreen  =     (short)(    greenTotal /   nonWhitePixelCount     );
	retAvgBlue    =     (short)(     blueTotal   /   nonWhitePixelCount    );


	return  (long)nonWhitePixelCount;
}



											////////////////////////////////////////


long     OffMap::Calc_Average_RGB_Components(    short&  retAvgRed,  short&  retAvgGreen,  short&  retAvgBlue,      RectShrt&  boundBoxOriginal,   											 
											                                              RectShrt&  boundBoxCurrent,   OffMap *maskMap,    short&  retPercentPixelsMeasured   )
{

			//  Need 2 boundBoxes because if the user has MOVED or STRETCHED the SUBregion, then the source-BoundBox( m_maskmapsBoundBox ) 
			//   will not be same as the actual bound box of the Region's GRAPHIC.    Tests OK( 10/20/08 )


				//  ONLY average for  NON-White( PIGMENTED)   OR  Masked  pixels.  If MaskMap is supplied, OK if colors are NOT uniform.   10/08

	retAvgRed =  retAvgGreen =   retAvgBlue  =   -1; 
	retPercentPixelsMeasured =  -1;
	

	short		hiCompValue =    Component_Almost_White();    //   250,  is a static variable.  eep consistant throughout the app    

								
	short               rd,  gr,  bl;  
	unsigned long   redTotal = 0,  greenTotal = 0,  blueTotal =   0,      nonWhitePixelCount =  0;




	long   x0, y0 ,   xLast, yLast,    xMask,  yMask;


	if(    boundBoxOriginal.left  < 0    ||    boundBoxOriginal.right < 0      ||    boundBoxOriginal.top    < 0      ||    boundBoxOriginal.bottom < 0   )
	{

		ASSERT( 0 );    //  This has NOT been tested, and seems to need more code.   10/08

		x0 = y0= 0;                        //  do the whole bitmap if any value is negative
		xLast =   m_width -1;     
		yLast =   m_height -1;
	}
	else
	{	x0 =   boundBoxCurrent.left;        //  boundBoxOriginal.left;    *****NO, what if Region was moved by user
		y0 =   boundBoxCurrent.top;        //  boundBoxOriginal.top;  

		xLast =     boundBoxCurrent.right;      //  boundBoxOriginal.right;
		yLast =     boundBoxCurrent.bottom;     //  boundBoxOriginal.bottom; 
	}


	long   totalPixelsInMask  =    (long)( maskMap->m_width )    *   (long)(    maskMap->m_height   ); 




	long   widthMask    =     boundBoxOriginal.Get_Width();
	long   heightMask  =      boundBoxOriginal.Get_Height(); 

	long   widthCurrent    =     boundBoxCurrent.Get_Width();
	long   heightCurrent  =      boundBoxCurrent.Get_Height(); 


	if(        maskMap  !=  NULL    
		&&   (   widthCurrent  ==  0    ||      heightCurrent  ==  0    )    )    //  no divide by zero
	{
		AfxMessageBox(   "OffMap::Calc_Average_RGB_Components  failed,  widthCurrent or  heightCurrent equals zero."  );
		return -1;
	}

	if(        maskMap  !=  NULL    
		&&   (   widthCurrent  <  0    ||      heightCurrent <  0    )    )    //  no divide by zero
	{
		AfxMessageBox(   "OffMap::Calc_Average_RGB_Components  failed,  widthCurrent or  heightCurrent is LESS than ZERO."  );
		return -1;
	}





	for(    long y= y0;    y <= yLast;     y++   )
	{
		for(  long x= x0;    x <=  xLast;     x++   ) 
		{

			Read_Pixel(  x, y,    &rd,  &gr,  &bl   );  


			if(   maskMap  ==  NULL    )
			{
				if(		   rd   <    hiCompValue			//   ONLY average for  NON-White( PIGMENTED) pixels.  Since this is a watercolor scan, color should be uniform
					||    gr    <   hiCompValue
					||    bl    <   hiCompValue    )
				{
					redTotal     +=     ( unsigned long )rd;
					greenTotal  +=     ( unsigned long )gr;
					blueTotal    +=     ( unsigned long )bl;

					nonWhitePixelCount++;
				}
			}
			else     // Since there is a MaskMap,  we only gather color-data the pixels where the maskmap has a BLACK pixel
			{      			

				/***
				xMask =    x  -  boundBoxOriginal.left;   **** OLD and WRONG,  only works if both bound boxes are in same place, but the Region's boundbox might have been offseted or stretched
				yMask =    y -   boundBoxOriginal.top;
				***/

				xMask  =    (   (x  -  x0)  *       widthMask   )     / widthCurrent;   //  must scale down the DISTANCE from Current's ORIGIN to find proper offsetting into the SOURCE-maskmap
				yMask  =    (   (y  -  y0)  *       heightMask   )    / heightCurrent;  



				if(    maskMap->MaskPix_Is_Written(   xMask,  yMask,     0  )    )   //  Must offset back, becase MaskMap is smaller than the ColorMap(this)
				{
					redTotal     +=     ( unsigned long )rd;
					greenTotal  +=     ( unsigned long )gr;
					blueTotal    +=     ( unsigned long )bl;

					nonWhitePixelCount++;
				}
			}

		}
	}



	if(   nonWhitePixelCount  ==    0   )       // careful,  divide by zero
	{
		//  ASSERT( 0 );    //  is this an error to call this for no pixels ???     10/15/08

		retAvgRed =  retAvgGreen =   retAvgBlue  =   -1; 
		retPercentPixelsMeasured =  0;

		return   0;
	}
	else
	{   
		if(    totalPixelsInMask  > 0    )		
			retPercentPixelsMeasured   =   (short)(      ( nonWhitePixelCount  * 100L )  /   totalPixelsInMask      );
		else
			retPercentPixelsMeasured   =  0;
	}



	retAvgRed     =     (short)(    redTotal    /   nonWhitePixelCount     );
	retAvgGreen  =     (short)(    greenTotal /   nonWhitePixelCount     );
	retAvgBlue    =     (short)(     blueTotal   /   nonWhitePixelCount    );


	return  (long)nonWhitePixelCount;
}






											////////////////////////////////////////


long     OffMap::Calc_Average_HSV_Components(    short&  retHue,    short&  retSaturation,    short&  retValue   )
{

	short   retAvgRed,  retAvgGreen,  retAvgBlue;  


	long   nonWhitePixelCount  =     Calc_Average_RGB_Components(  retAvgRed,   retAvgGreen,  retAvgBlue  );
  


	RGB_to_HSV(    retAvgRed,  retAvgGreen,  retAvgBlue,          retHue,   retSaturation,   retValue    );

	return  nonWhitePixelCount;
}



											////////////////////////////////////////


void    OffMap::Read_Pixel_ColorModified(    long x, long y,    short  hueOffset,  short  saturationOffset,  short valueOffset,    short&  rd,  short& gr,  short& bl   )
{

		//   Will apply the  ColorModification  specs   to the source pixel from the  RegionSubject's  offmap
 

	short  rdOrg,  grOrg,  blOrg,        retHue,  retSaturation,  retValue;	
	short                      nuHue,  nuSaturation,  nuValue;  



	Read_Pixel(   x,  y,     &rdOrg,  &grOrg,  &blOrg   ); 



										//    A)		NO ColorModification Specification, so just write out original values

	if(   hueOffset ==  0    &&    saturationOffset ==  0    &&   valueOffset ==  0    )   
	{
		rd =   rdOrg;
		gr =   grOrg;
		bl =   blOrg;

		return;
	}


										//    B)		Have request for  ColorModification ( in RegionSubject ),  so process the pixels components


	if(    rdOrg == 255    &&   grOrg == 255     &&   blOrg == 255   )    //  if it is a WHITE pixel on the separation of RegionSubject, then we do NO modification   8/08
	{
		rd =   gr =    bl  =   255;      //  just write white back out
		return;
	}




	RGB_to_HSV(   rdOrg,  grOrg,  blOrg,       retHue,   retSaturation,   retValue   );


	nuHue  =    retHue  +  hueOffset;

	if(  nuHue  >  360  )
		nuHue  -=  360;
	else if(  nuHue  <  0  )
		nuHue  +=  360;


	nuSaturation  =     retSaturation  +  saturationOffset;

	if(   nuSaturation  >  255   )
		nuSaturation  =    255;
	else if(   nuSaturation  <  0   )
		nuSaturation  =     0;


	nuValue  =     retValue  +  valueOffset;

	if(   nuValue  >  255   )
		nuValue  =    255;
	else if(   nuValue  <  0   )
		nuValue  =     0;


				
	HSV_to_RGB(   nuHue,  nuSaturation,  nuValue,    rd,  gr,  bl  );
}




											////////////////////////////////////////


bool    OffMap::Fit_To_HSV_Color_OffMap(  short  hueTarg,   short   saturationTarg,   short valueTarg,     bool& retWantsAWhiteRegion,   
																	short&  retHueOffset,	short&  retSaturationOffset,   short&  retValueOffset,  	CString&   retErrorMesg   )
{

		//   BAD NAME,  but want ot be aware of similarity to  RegionSubject::Fit_To_HSV_Color()     3/10
		

		//			This functions calulates the  COLOR OFFSETS { retHueOffset, retSaturationOffset, retValueOffset  } that would allow the 'ORIGINAL bitmap values' 
		//          to have the same INPUT spec { hueTarg, saturationTarg, valueTarg} when being operated  on by  OffMap::Read_Pixel_ColorModified()
		//
		//		    BUT, when the functions ends, the Offmap will be RE-COLORED to have a color that roughly matches the inputColor { hueTarg, saturationTarg, valueTarg }
		//
		//   Is designed to work on OffMaps whose { color & tone } is kind of CONSISTANT  ( like the watercolor TextureMaps  )     3/10
		


	short  maxIters  =      12;      //  6   ****** ADJUST *******


	short  tolerance  =       4;      //  4   ****** ADJUST *******


	short   almostWhiteComponent =   240;      //   ****** ADJUST  ??     Is just for error testing.  *******



	retErrorMesg.Empty();
	retHueOffset =  retSaturationOffset =  retValueOffset  =   0;   //  init with default values
	retWantsAWhiteRegion =   false;



	OffMap  *origMap    =         new   OffMap(  m_width,   m_height,   m_depth  );    //  need a dupicate of the  ORIGINAL OffMap  for the algo
	if(           origMap ==  NULL  )
	{
		retErrorMesg  =    "OffMap::Fit_To_HSV_Color_OffMap  failed,  could not alloc  origMap." ;
		return  false;
	}
		
	origMap->Copy_Bits(   *this,    retErrorMesg  );    
	if(    ! retErrorMesg.IsEmpty()    )
	{
		ASSERT( 0 );
		return  false;
	}




	short   retHueTrav,   retSaturationTrav,  retValueTrav,       hueDiff,   satDiff,  valueDiff; 
	short   redLast= -1,  greenLast= -1,  blueLast= -1;
	/***
	short  hueOffsetOld          =   	Get_Hue_Offset();             // save old values in cse of error
	short	 saturationOffsetOld  =    Get_Saturation_Offset();
	short	 valueOffsetOld         =    Get_Value_Offset();
	****/

	long   nonWhitePixelCount  =      Calc_Average_HSV_Components(   retHueTrav,    retSaturationTrav,   retValueTrav  );


	bool    isNotCloseEnough =  true;
	short   i=  0;



	while(          isNotCloseEnough  
		       &&    i <  maxIters    )
	{

		hueDiff     =      OffMap::Subtract_Degrees(   hueTarg,   retHueTrav  );        //  Does UN-WRAPPING of degrees,   Gives values:   {   -180  < value  <  +180    )

		satDiff      =      saturationTarg   -    retSaturationTrav;

		valueDiff  =      valueTarg          -     retValueTrav;




		if(    absj( hueDiff )  <=  tolerance      &&     absj( satDiff )  <=  tolerance     &&     absj( valueDiff )  <=  tolerance   )   // want to apply the change/update before exiting
			isNotCloseEnough =   false;
		else
		{

			retHueOffset      +=       hueDiff;    

			if(   retHueOffset  >=   360   )     // *****  NEED INSTALL more un-wrapping for  360 degree values ?????   Dont think so, it is testing fine.    10/08  **************************************
				retHueOffset   -=  360;

			if(   retHueOffset  >=   360   )
				retHueOffset   -=  360;

			if(   retHueOffset  <=  -360  )
				retHueOffset   +=  360;

			if(   retHueOffset  <=  -360  )
				retHueOffset   +=  360;

			ASSERT(    retHueOffset  <  360    &&    retHueOffset  > -360   ); 	



			retValueOffset        +=       valueDiff;   //  ***** Though I can get values over 255, it is not a big deal because  Copy_Out_Scaled_And_Modified_Bits() 
																	 //              and  Read_Pixel_ColorModified() wiol correct if a number is too big.  However it does show  that I am having a \
					                                                 //				  convergence problem.  Usually when tryin to create a white region.     11/08
			retSaturationOffset  +=      satDiff;





	//		if(     ! Update_SmallBitmap_Bits_wColorModify(  retErrorMesg  )     )    ***** Need to simulate this function for the algo to work ( see below) 
	//			return  false;
			short    rd,  gr, bl;  


			for(    long y=0;    y < m_height;     y++   )
			{  for(  long x=0;    x < m_width;     x++   ) 
				{					

					origMap->Read_Pixel_ColorModified(   x, y,    retHueOffset,  retSaturationOffset,  retValueOffset,     rd,  gr, bl   );

					Write_Pixel(   x, y,      rd,  gr,  bl   );    //   Copy the new color-approzimation 
				}
			}



			Calc_Average_HSV_Components(   retHueTrav,    retSaturationTrav,   retValueTrav  );


			HSV_to_RGB(    retHueTrav,  retSaturationTrav,  retValueTrav,     redLast,  greenLast,  blueLast     );
		}

		i++;
	}




	if(   origMap  !=  NULL    )
	{
		delete  origMap;    origMap =   NULL;
	}



	if(   isNotCloseEnough   )
	{							   
		   //  Frequently get here when trying to make the region WHITE (   redLast,  greenLast,  blueLast  near 255 ).  So just let it go


		if(      redLast  >=  almostWhiteComponent    &&      greenLast  >=  almostWhiteComponent    &&     blueLast  >=  almostWhiteComponent  ) 
			retWantsAWhiteRegion =   true;


		if(    retWantsAWhiteRegion   )
			TRACE(  "\n******ERROR: [ OffMap::Fit_To_HSV_Color_OffMap ]   Hit  MAX-iterations[ %d ],  wants a WHITE Region.    LastRGB[ %d, %d, %d ]  \n",     i,    redLast,  greenLast,  blueLast     );   
		else
			TRACE(  "\n******ERROR: [ OffMap::Fit_To_HSV_Color_OffMap ]   Hit  MAX-iterations[ %d ]    LastRGB[ %d, %d, %d ]  \n",     i,    redLast,  greenLast,  blueLast     );   


		/****  NO,  let it keep the values for a while.  11/6/08

		retHueOffset           =     hueOffsetOld;             //  RESTORE  old values,  or  use the last apporoximation ??  
		retSaturationOffset  =     saturationOffsetOld;
		Get_Value_Offset()        =	    valueOffsetOld;

//		if(     ! Update_SmallBitmap_Bits_wColorModify(  retErrorMesg  )     )
//			return  false;
		*****/



	//	retErrorMesg =   "RegionSubject::Fit_To_HSV_Color failed, hit MAX ITERATIONS without an acceptable result."  ;

		return   true;      //   false;    ****let it go,  see how it works. 
	}
	else
	{	//  TRACE(  "%d  iters for  Fit_To_HSV_Colorto suceed.\n",   i    );
	}


	return   true;
}





											////////////////////////////////////////


OffMap*    OffMap::Make_ScaledDown_Copy(   long  scaleDownFactor,   CString&   retErrorMesg   )
{


	retErrorMesg.Empty();

	if(   scaleDownFactor  <=  0   )
	{
		retErrorMesg =  "OffMap::Make_ScaledDown_Copy  failed,  scaleDownFactor <= 0"  ;
		return  NULL;
	}



	if(     m_totalAllocatedBytes   <=  0    )
	{
		ASSERT( 0 );   //  BIG mess 
		retErrorMesg =  "OffMap::Make_ScaledDown_Copy  failed, source map has no bytes."  ;
		return  NULL;
	}



	long  nuWidth   =     m_width     /  scaleDownFactor;
	long  nuHeight  =     m_height    /  scaleDownFactor;


	OffMap   *nuOffMap  =        new     OffMap(   nuWidth,   nuHeight,       m_depth   );    
	if(            nuOffMap ==  NULL  )
	{
		retErrorMesg =  "OffMap::Make_ScaledDown_Copy  failed,  nuOffMap is NULL."  ;
		return  NULL;
	}




										//   Copy in the scaled bits

	Copy_Out_Scaled_Bits(   scaleDownFactor,   *nuOffMap   );





																//  Now fix the 2nd  'BAD POINTER'

	if(          m_depth  ==   1   )
	{
		nuOffMap->m_bits =    nuOffMap->m_dib;        //  No  header
	}
	else if(   m_depth  ==   8    )
	{

		int  wNumColors =  256;      //  FIX: derive from m_depth     ( now HARDWIRED )

		int  palletSize =    sizeof( PALETTEENTRY )	*  wNumColors; 

		nuOffMap->m_bits =    nuOffMap->m_dib    +   sizeof( BITMAPINFOHEADER )   +   palletSize;    //  this SECOND POINTER must also be fixed 
	}
	else if(   m_depth  ==   24    )
	{
		int  palletSize =   0;    //  no pallet for 

		nuOffMap->m_bits =    nuOffMap->m_dib    +   sizeof( BITMAPINFOHEADER )   +   palletSize;    //  this SECOND POINTER must also be fixed 
	}
	else
		ASSERT( 0 );


	return  nuOffMap;
}



											////////////////////////////////////////


OffMap*    OffMap::Make_ScaledDown_ColorModified_Copy(   long  scaleDownFactor,     short  hueOffset,  short  saturationOffset,  short valueOffset,   CString&   retErrorMesg   )
{


	retErrorMesg.Empty();

	if(   scaleDownFactor  <=  0   )
	{
		retErrorMesg =  "OffMap::Make_ScaledDown_ColorModified_Copy  failed,  scaleDownFactor <= 0"  ;
		return  NULL;
	}

	if(     m_totalAllocatedBytes   <=  0    )
	{
		ASSERT( 0 );   //  BIG mess 
		retErrorMesg =  "OffMap::Make_ScaledDown_ColorModified_Copy  failed, source map has no bytes."  ;
		return  NULL;
	}


	long  nuWidth   =     m_width     /  scaleDownFactor;
	long  nuHeight  =     m_height    /  scaleDownFactor;

	OffMap   *nuOffMap  =        new     OffMap(   nuWidth,   nuHeight,       m_depth   );    
	if(            nuOffMap ==  NULL  )
	{
		retErrorMesg =  "OffMap::Make_ScaledDown_ColorModified_Copy  failed,  nuOffMap is NULL."  ;
		return  NULL;
	}




										//   Copy in the scaled bits

	Copy_Out_Scaled_And_Modified_Bits(   scaleDownFactor,   *nuOffMap,     hueOffset,  saturationOffset,  valueOffset   );




																//  Now fix the 2nd  'BAD POINTER'

	if(          m_depth  ==   1   )
	{
		nuOffMap->m_bits =    nuOffMap->m_dib;        //  No  header
	}
	else if(   m_depth  ==   8    )
	{
		int  wNumColors =  256;      //  FIX: derive from m_depth     ( now HARDWIRED )

		int  palletSize =    sizeof( PALETTEENTRY )	*  wNumColors; 

		nuOffMap->m_bits =    nuOffMap->m_dib    +   sizeof( BITMAPINFOHEADER )   +   palletSize;    //  this SECOND POINTER must also be fixed 
	}
	else if(   m_depth  ==   24    )
	{
		int  palletSize =   0;    //  no pallet for 

		nuOffMap->m_bits =    nuOffMap->m_dib    +   sizeof( BITMAPINFOHEADER )   +   palletSize;    //  this SECOND POINTER must also be fixed 
	}
	else
		ASSERT( 0 );

	return  nuOffMap;
}



											////////////////////////////////////////


void   OffMap::Clear(   short   greyVal   )   
{
					 	//  MAC:  will write all BLACK, or all WHITE,    PC: for GRAY 256 fill in any input value
	BYTE   val;


	if(    m_depth  ==   8   )    //  &&  Grayscale
	{
		ASSERT(   greyVal  <=  255   );

		val  =   ( BYTE )greyVal;

		for(   long i=0;    i<  m_totMapBytes;    i++   )
			m_bits[ i ] =   val;            
	}
	else if(    m_depth  ==   1   )   
	{

		if(   greyVal  ==  0   )
			val  =     0;
		else
			val  =  255;    //   fill all bytes with ones


		for(   long i=0;    i<  m_totMapBytes;    i++   )
			m_bits[ i ] =   val;            
	}
	else if(    m_depth  ==   24   )   
	{

		if(         greyVal  ==  0   )
			val  =       0;    //   black
		else if(   greyVal  ==  255   )
			val  =    255;
		else
		{	ASSERT( 0 );    // Need to write something for this ?????    4/08
			val  =    0;
		}


		for(   long i=0;    i<  m_totMapBytes;    i++   )
			m_bits[ i ] =   val;            
	}
	else
		ASSERT( 0 );
}




											////////////////////////////////////////


bool    OffMap::Copy_Bits(   OffMap&  sourceMap,    CString&   retErrorMesg    )
{


     // ********* FUTURE: make a 'Overload' ( ASSIGN  '='   ????? )********
  		
  		// for total copy   ...assumes SAME depth and size( what about size mismatch????)

  		// *** maybe make default size adjust for different  wid,heit
    

  
     
	if((     sourceMap.Is_Empty()  )
		||(         this->Is_Empty()  ))
	{  
		retErrorMesg =   "OffMap::Copy_Bits() failed,  Bitmaps are NOT in MEMORY. ";     
		return  false;  
	}
     

     if((      sourceMap.m_width    !=    this->m_width     )
		 ||(  sourceMap.m_height   !=    this->m_height    )
         ||(  sourceMap.m_depth    !=    this->m_depth     ))
	{  
		 retErrorMesg =    "OffMap::Copy_Bits() failed,  Bitmaps are NOT same SIZE. " ;     
		 return  false;  
	 }
     

    /****
     Rect          rct; 
     PixMapHandle  SrcPxBase,  DstPxBase;
     RGBColor      myWhite, myBlack;
     GWorldPtr  currPort;  // Saves the current port prior to setting up  offscreen world
     GDHandle   currDev;   // Saves the current device prior to setting up offscreen world

	 myBlack.red= myBlack.green= myBlack.blue=  0x0000; // **** NEEDED???? not sure
	 myWhite.red= myWhite.green= myWhite.blue=  0xFFFF;   
                  
     GetGWorld( &currPort, &currDev );    // save old GPort and Gdevice( IMPORTANT, 11/95 )
     SetRect(  &rct,   0,0,   sourceMap.m_width,   sourceMap._heit  );         

     SrcPxBase = GetGWorldPixMap( sourceMap._gWorld ); // prefered way to get map, do NOT dereference from struct
     DstPxBase = GetGWorldPixMap(        _gWorld ); 
         
     LockPixels( SrcPxBase );        
	 LockPixels( DstPxBase );       

     SetGWorld(  _gWorld,  nil  );	   // *** very IMPORTANT( 11/95)***

     RGBBackColor( &myWhite );   RGBForeColor( &myBlack );   // **** NEEDED???? not sure
         
     CopyBits(    (BitMap*)(*SrcPxBase),   (BitMap*)(*DstPxBase),   &rct,  &rct,  srcCopy,  nil   );  
        										        // **** or  'srcOr'  ???????? ***

     SetGWorld(  currPort, currDev );       
     UnlockPixels( SrcPxBase );       
	 UnlockPixels( DstPxBase );  
	 ***/


	for(     long i= 0;      i<  m_totMapBytes;     i++     )
		m_bits[ i ] =   sourceMap.m_bits[ i ]; 

// ****** Later  OPTIMIZE  with  a Block Memory copy command ********************


	 return  true;
}




											////////////////////////////////////////


bool    OffMap::Copy_xSegment(    long  xSRCoffset,    long  xDSToffset,    long width,    OffMap&  sourceMap,   CString&   retErrorMesg  )
{


			//   2 maps must have the SAME:  {  Height,  colorTable,  etc )

// ****************************  OPTIMIZE  3/03  **********************************

	retErrorMesg.Empty();


	if((     sourceMap.Is_Empty()  )
		||(         this->Is_Empty()  ))
	{  
		retErrorMesg =   "OffMap::Copy_xSegment failed,  Bitmaps are NOT in MEMORY. ";     
		return  false;  
	}     

    if((      sourceMap.m_height   !=    this->m_height    )
         ||(  sourceMap.m_depth    !=    this->m_depth     ))
	{  
		 retErrorMesg =    "OffMap::Copy_xSegment failed,  Bitmaps are NOT same SIZE. " ;     
		 return  false;  
	 }



	short    val,  gr,  bl,   xWrite,   cnt;
	long    xStop  =    xSRCoffset  +   width;     //  careful of  'Inclusive Counting'


	for(   long y =0;       y < m_height;	     y++    )
	{

		cnt =  0;

		for(   long  x =  xSRCoffset;      x <  xStop;     x++    )     //  careful of  'Inclusive Counting'
		{

			xWrite  =    xDSToffset    +  cnt;


			sourceMap.Read_Pixel(    x,         y,       &val,  &gr,  &bl   ); 

			Write_Pixel(                    xWrite,  y,         val,    gr,    bl   ); 

			cnt++;
		}
	}

	return  true;
}



											////////////////////////////////////////


bool    OffMap::Mask_Bits(   OffMap&  maskMap,   short  pixThresh,  short  replaceValue,    bool  greaterThan,						  
																							CString&   retErrorMesg    )
{

       
	if((       maskMap.Is_Empty()   )
		||(         this->Is_Empty()    ))
	{  
		retErrorMesg =   "OffMap::Mask_Bits() failed,  Bitmaps are NOT in MEMORY. ";     
		return  false;  
	}
     

     if((      maskMap.m_width    !=    this->m_width     )
		 ||(  maskMap.m_height   !=    this->m_height    )
         ||(  maskMap.m_depth    !=    this->m_depth     ))
	{  
		 retErrorMesg =    "OffMap::Mask_Bits() failed,  Bitmaps are NOT same SIZE. " ;     
		 return  false;  
	 }
 

     if(    maskMap.m_depth    !=   8  )
	{  
		 retErrorMesg =    "OffMap::Mask_Bits() failed,  Bitmaps are NOT 8 deep. " ;     
		 return  false;  
	 }



	for(   long i=0;    i<  m_totMapBytes;    i++   )   // currently this ASSUMES both maps are 8-bit grey( 255 is white )
	{

		if(   greaterThan   )
		{

			if(    maskMap.m_bits[ i ]     >=     pixThresh    )		//   MaskMap is  GREATER than...
				m_bits[ i ] =    ( unsigned char )replaceValue;
			else
			{   }    // do nothing,  keep value
		}
		else
		{
			if(    maskMap.m_bits[ i ]     <=    pixThresh    )        	//   MaskMap is  LESS than...
				m_bits[ i ] =   ( unsigned char )replaceValue;
			else
			{   }    // do nothing,  keep value
		}

	}


	return  true;
}



											////////////////////////////////////////


bool	OffMap::Merge_Bits(   OffMap&  otherMap,     CString&   retErrorMesg    )
{


	retErrorMesg.Empty();


	if((       otherMap.Is_Empty()   )
		||(         this->Is_Empty()    ))
	{  
		retErrorMesg =   "OffMap::Merge_Bits() failed,  Bitmaps are NOT in MEMORY. ";     
		return  false;  
	}
     

     if((      otherMap.m_width    !=    this->m_width     )
		 ||(  otherMap.m_height   !=    this->m_height    )
         ||(  otherMap.m_depth    !=    this->m_depth     ))
	{  
		 retErrorMesg =    "OffMap::Merge_Bits() failed,  Bitmaps are NOT same SIZE. " ;     
		 return  false;  
	 }
 



	for(   long i=0;    i<  m_totMapBytes;    i++   )   // currently this ASSUMES both maps are 8-bit grey( 255 is white )
	{

		unsigned char   curVal =   m_bits[ i ];

		short   val  =      (    (short)curVal /2    )    +     (     (short)( otherMap.m_bits[ i ] )  /2     );


		if(  val  > 255  )
			m_bits[ i ]  =    255;
		else if(  val  < 0  )
			m_bits[ i ]  =    0;
		else
			m_bits[ i ]  =   ( unsigned char )val;
	}


	 return  true;
}


											////////////////////////////////////////


short    OffMap::Get_Max_Value()
{

	short    maxValue = -1,     rd, gr, bl;

 
	for(   short  y=0;     y<  m_height;     y++   )
	{

		for(   short  x=0;     x < m_width;      x++   )
		{
			Read_Pixel(  x,     y,    &rd,  &gr,  &bl  ); 

			if(   rd  >  maxValue   )
				maxValue  =    rd;   
		}
	}

	return   maxValue;
}


											////////////////////////////////////////


long   OffMap::Get_Ycoord_of_Columns_MaxValue(  long  x,    short&  retValue  )     
{                 		

                                    //  returns  -1     for OUT of BOUNDs

									//  returns  -1     if all pixels are Zero  !!!

  
    short  y,   val,gr,bl,   biggest= 0,   bigestY= -1;
       
	retValue =  -1;
   


    if(   x < 0    ||   x >= m_width    )      
	{
//		ASSERT( 0 );    ...WANT this ??? ***********************
		return  -1;							 // OUT of BOUNDs 
	}
   
  
    Lock_Bits();				


    for(    y= 0;    y< m_height;    y++   )       
    { 

         Read_Pixel(   x, y,    &val,  &gr,&bl    );

         if(   val   >   biggest    )  
         {  
              biggest =   val;
              bigestY =     y;   // *** NEVER gets assigned UNLESS one pixel has a value GREATER that ZERO !!!     
         }
    }  


    Unlock_Bits();  
    

	retValue =   biggest;

    return    bigestY;   //  returns  -1  if all pixels are Zero
}



											////////////////////////////////////////


long   OffMap::Get_Ycoord_of_Columns_MaxValue_By_3Components(   long  x,    long&  retComponentColor  )     
{                 		

                                    //  returns  -1     for OUT of BOUNDs

									//  returns  -1     if all pixels are Zero  !!!
  
    short  y,   val,gr,bl;
       
	retComponentColor =  -1;
   

    if(   x < 0    ||   x >= m_width    )      
	{
//		ASSERT( 0 );    ...WANT this ??? ***********************
		return  -1;							 // OUT of BOUNDs 
	}
   
  
    Lock_Bits();		

	long  componentCombo =  0,     biggestColorVal= 0,     bigestY= -1 ;



	for(    y= 0;    y< m_height;    y++   )      
	{ 
			
		 Read_Pixel(   x, y,    &val,  &gr,&bl    );

		 componentCombo =   RGBjm(  val,  gr,  bl   );   

	
		 if(   componentCombo   >   biggestColorVal    )     //  NOTE:  must  find a value  'GREATER THAN zero'
		 {  
			  biggestColorVal  =   componentCombo;

			  bigestY  =     y;   // *** NEVER gets assigned UNLESS one pixel has a value GREATER that ZERO !!!     
		 }
	}  


	if(   bigestY  <  0   )   //  no value was greater than zero, so we can do nothing
	{
		return  -3;   // ***** DOES this HAPPEN ????    2/7/2012  **********************************************
	}


    Unlock_Bits();  
    


	retComponentColor =   biggestColorVal;

    return    bigestY;   //  returns  -1  if all pixels are Zero
}




											////////////////////////////////////////


long   OffMap::Get_Xcoord_of_Rows_MaxValue_By_3Components(   long  y,    long&  retComponentColor  )     
{                 		

                                    //  returns  -1     for OUT of BOUNDs

									//  returns  -1     if all pixels are Zero  !!!
  
    short  x,   val,gr,bl;
       
	retComponentColor =  -1;
   

    if(   y < 0    ||   y >= m_height    )      
	{
//		ASSERT( 0 );    ...WANT this ??? ***********************
		return  -1;							 // OUT of BOUNDs 
	}
   
  
    Lock_Bits();		

	long  componentCombo =  0,     biggestColorVal= 0,     bigestX= -1 ;



	for(    x= 0;    x< m_width;    x++   )      
	{ 
			
		 Read_Pixel(   x, y,    &val,  &gr,&bl    );

		 componentCombo =   RGBjm(  val,  gr,  bl   );   

	
		 if(   componentCombo   >   biggestColorVal    )     //  NOTE:  must  find a value  'GREATER THAN zero'
		 {  
			  biggestColorVal  =   componentCombo;

			  bigestX  =     x;   // *** NEVER gets assigned UNLESS one pixel has a value GREATER that ZERO !!!     
		 }
	}  


	if(   bigestX  <  0   )   //  no value was greater than zero, so we can do nothing
	{
		return  -3;   // ***** DOES this HAPPEN ????    2/7/2012  **********************************************
	}


    Unlock_Bits();  
    


	retComponentColor =   biggestColorVal;

    return    bigestX;   //  returns  -1  if all pixels are Zero
}



											////////////////////////////////////////


short   OffMap::Find_Current_Interval(   long  x,    long&  retXstart,     long&  retXend     )     
{                 	


	//	CALLED by:   AnimeStream::Get_Current_ColorRun()    and    DrivingMusicStaffView::Render_NotationArray()   2/12


	//   TESTS for that the  Pixel's  "RED-Component"   is the SAME for every Pixel in the calculated INTERVAL.   2/12


	/***     
			RETURNS:   the pixel GreyVal  for the  current run   ....OR

					returns  -2     for OUT of BOUNDs

					returns  -3     if all pixels are Zero  !!!  ( Only for maps that are Bigger than  1 Pixel high.  )  
	***/
   

    short  y,   val,gr,bl,     targVal,   bigestY= -1;
	long   xTrav;
       

	targVal    =  -1;
    retXstart  =  -1;
    retXend   =  -1;


    if(       x  <    0					 //  check OUT of BOUNDs 
		||   x  >=  m_width    )      
	{
		return  -2;						
	}
   

	Lock_Bits();					 



	if(    m_height  ==   1    )
	{
		bigestY =  0;     // only one row, so this is our  'ROW of Focus'

		Read_Pixel(   x, bigestY,    &targVal,   &gr,&bl    );
	}
	else
	{
	//	ASSERT( 0 );      //    ( need to test this !!!   6/2002 )    Now finally testing  2/2012    ***** WORKS OK,  2/4/12


						//  Find the Y-row with the bigest value, and ASSUME this is the SegmentRun we want to detect

	    for(    y= 0;    y< m_height;    y++   )      
		{ 
			 Read_Pixel(   x, y,    &val,  &gr,&bl    );

			 if(   val   >   targVal    )     //  NOTE:  must  find a value  'GREATER THAN zero'
			 {  
				  targVal  =   val;
				  bigestY  =     y;   // *** NEVER gets assigned UNLESS one pixel has a value GREATER that ZERO !!!     
			 }
		}  


		if(   bigestY  <  0   )   //  no value was greater than zero, so we can do nothing
			return  -3;
	}



														//  Go backward till we hit a new pixel value

	if(    x  ==  0   )
		retXstart =   0;
	else
	{
		for(    xTrav =  ( x  - 1 );      xTrav  >=  0;      xTrav--    )
		{

			Read_Pixel(   xTrav,  bigestY,    &val,  &gr,  &bl    );
				

			if(        val   !=   targVal	      //   Ending a  NOTE,
				||    xTrav  ==   0        )		//   ...or at  Map's beginning.
			{

				if(    xTrav  ==   0    )			
					retXstart  =   0;
				else
					retXstart  =    xTrav  + 1;

				break;   		
			}   
		} 
	}



														//  Go FOWARD till we hit a new pixel value

	if(    x   ==   ( m_width  -1 )     )
	{
	//	retXstart  =    m_width  -1;		 *******  WAS wrong for Vertical,  should be wrong here too   4/23/12	   	// **** PUT this logic up above ????????????

		retXend  =     m_width  -1;    //   ***** OK   ????      1/2004
	}
	else
	{
 		for(    xTrav =  ( x  + 1 );      xTrav  < m_width;      xTrav++    )
		{

			Read_Pixel(   xTrav,  bigestY,    &val,  &gr,  &bl    );
				


			if(        val   !=   targVal	                            //   Beginning of a new NOTE,
				||    xTrav  ==   ( m_width  -1 )      )		//   ...or at  Map's end.
			{

				if(    xTrav  ==    ( m_width  -1 )     &&     val  ==  targVal    )			
				{
					retXend  =   ( m_width  -1 );
				}
				else
					retXend  =    xTrav  - 1;

				break;   		
			}   
		} 
	}
 

    Unlock_Bits();  
    
    return    targVal;  
}




											////////////////////////////////////////


long    OffMap::Find_Current_Interval_By_3Components(   long  x,    long&  retXstart,     long&  retXend     )     
{                 	


	//	CALLED by:   AnimeStream::Get_Current_ColorRun()    and    DrivingMusicStaffView::Render_NotationArray()   2/12


	//   TESTS that the  "GROUP of all 3 components"   is the SAME for every Pixel in the calculated INTERVAL.       2/12
	//      So it test that the EXACT COLOR  is the SAME  in every PIXEL in the run.



	/***     
			RETURNS:   the pixel GreyVal  for the  current run   ....OR

					returns  -2     for OUT of BOUNDs

					returns  -3     if all pixels are Zero  !!!  ( Only for maps that are Bigger than  1 Pixel high.  )  
	***/
   

    short  y,     val, gr, bl,       bigestY= -1;
	long   xTrav;

	long   componentCombo,    targVal;


       

	targVal    =  -1;
    retXstart  =  -1;
    retXend   =  -1;


    if(       x  <    0					 //  check OUT of BOUNDs 
		||   x  >=  m_width    )      
	{
		return  -2;						
	}
   

	Lock_Bits();					 



	if(    m_height  ==   1    )
	{
		bigestY =  0;     // only one row, so this is our  'ROW of Focus'

		Read_Pixel(   x, bigestY,    &val,   &gr, &bl    );

	//	componentSum      =   val  +  gr  +  bl; 

		componentCombo  =   RGBjm(  val,  gr,  bl   );       
	}
	else
	{
	//	ASSERT( 0 );      //    ( need to test this !!!   6/2002 )    Now finally testing  2/2012    ***** WORKS OK,  2/4/12


						//  Find the Y-row with the bigest value, and ASSUME this is the SegmentRun we want to detect

	    for(    y= 0;    y< m_height;    y++   )      
		{ 

			 Read_Pixel(   x, y,    &val,  &gr,&bl    );

	//		 componentSum =   val  +  gr  +  bl; 

			 componentCombo  =   RGBjm(  val,  gr,  bl   );       


			 if(   componentCombo   >   targVal    )     //  NOTE:  must  find a value  'GREATER THAN zero'
			 {  
				  targVal  =   componentCombo;

				  bigestY  =     y;   // *** NEVER gets assigned UNLESS one pixel has a value GREATER that ZERO !!!     
			 }
		}  


		if(   bigestY  <  0   )   //  no value was greater than zero, so we can do nothing
			return  -3;
	}



														//  Go backward till we hit a new pixel value

	if(    x  ==  0   )
		retXstart =   0;
	else
	{
		for(    xTrav =  ( x  - 1 );      xTrav  >=  0;      xTrav--    )
		{

			Read_Pixel(   xTrav,  bigestY,    &val,  &gr,  &bl    );

		//	componentSum =   val  +  gr  +  bl; 

			componentCombo  =   RGBjm(  val,  gr,  bl   );       

				

			if(        componentCombo   !=   targVal	      //   Ending a  NOTE,
				||    xTrav  ==   0        )		                  //   ...or at  Map's beginning.
			{

				if(    xTrav  ==   0    )			
					retXstart  =   0;
				else
					retXstart  =    xTrav  + 1;

				break;   		
			}   
		} 
	}



														//  Go FOWARD till we hit a new pixel value

	if(    x   ==   ( m_width  -1 )     )
	{
//		retXstart  =    m_width  -1;	 *******  WAS wrong for Vertical,  should be wrong here too   4/23/12	   *** PUT this logic up above ????????????

		retXend  =     m_width  -1;   
	}
	else
	{
 		for(    xTrav =  ( x  + 1 );      xTrav  < m_width;      xTrav++    )
		{

			Read_Pixel(   xTrav,  bigestY,    &val,  &gr,  &bl    );
				
		//	componentSum =   val  +  gr  +  bl; 

			componentCombo  =   RGBjm(  val,  gr,  bl   );       



			if(        componentCombo   !=   targVal	                            //   Beginning of a new NOTE,
				||    xTrav  ==   ( m_width  -1 )      )		//   ...or at  Map's end.
			{

				if(         xTrav  ==    ( m_width  -1 )     
					&&    componentCombo  ==  targVal    )			
				{
					retXend  =   ( m_width  -1 );
				}
				else
					retXend  =    xTrav  - 1;

				break;   		
			}   
		} 
	}
 

    Unlock_Bits();  
    
    return    targVal;  
}



											////////////////////////////////////////


long   OffMap::Find_Current_Interval_By_3Components_Horizontal(    long  x,     long&  retXstart,   long&  retXend,    bool useNonZeroMode     )     
{                 	


	//	CALLED by:   


	//  if(  useNonZeroMode == FALSE )    TESTS that the  "GROUP of all 3 components"   is the SAME for every Pixel in the calculated INTERVAL.    2/12
	//                                               So it test that the EXACT COLOR  is the SAME  in every PIXEL in the run.   But when in useHueMode...
	//
	//   BUT if ( useHueMode ==TRUE )  then we just test for CONSTANT Hue,  which is what we see  after DETECTION  in  SPitchCalc::m_drivingOffMapHorz



	/***     
			RETURNS:   the pixel GreyVal  for the  current run   ....OR

					returns  -2     for OUT of BOUNDs

					returns  -3     if all pixels are Zero  !!!  ( Only for maps that are Bigger than  1 Pixel high.  )  
	***/
   

    short  y,     val, gr, bl,       bigestY= -1;
//	long   xTrav;

	long   componentCombo,    targVal;
//	short  retHue,  retSaturation,   retValue;  

//	bool   hasSameComponentValues =   false;
       


	targVal    =  0;        //  WAS  -1    :  NO!!!!   Only want a find if it is NOT black    6/15/2012   


    retXstart  =  -1;
    retXend   =  -1;


    if(       x  <    0					 //  check OUT of BOUNDs 
		||   x  >=  m_width    )      
	{
		return  -2;						
	}
   
	Lock_Bits();					 



	if(    m_height  ==   1    )
	{
		bigestY =  0;     // only one row, so this is our  'ROW of Focus'

		Read_Pixel(   x,   bigestY,    &val,   &gr, &bl    );

		componentCombo  =   RGBjm(  val,  gr,  bl   );       
	}
	else
	{
						//  Find the Y-row with the biggest value,  and ASSUME this is the SegmentRun we want to detect

	    for(    y= 0;    y< m_height;    y++   )      
		{ 
			
			 Read_Pixel(   x, y,    &val,  &gr,&bl    );

			 componentCombo =   RGBjm(  val,  gr,  bl   );   

	
			 if(   componentCombo   >   targVal    )     //  NOTE:  must  find a value  'GREATER THAN zero'
			 {  
				  targVal  =   componentCombo;

				  bigestY  =     y;   // *** NEVER gets assigned UNLESS one pixel has a value GREATER that ZERO !!!     
			 }
		}  


		if(   bigestY  <  0   )   //  no value was greater than zero, so we can do nothing
		{
			return  -3;   // ***** DOES this HAPPEN ????    2/7/2012  **********************************************
		}
	}


	long  targColorVal  =   Find_Current_Intervals_EndPoints_By_3Components_Horizontal(   x,   bigestY,    retXstart,  retXend,    useNonZeroMode   ); 


   //    'targColorVal'    should equal   'targVal'   ????    4/2012


    Unlock_Bits();  
    
    return    targVal;  
}



											////////////////////////////////////////


long   OffMap::Find_Current_Intervals_EndPoints_By_3Components_Horizontal(    long  x,  long  y,    long&  retXstart,   long&  retXend,    bool useNonZeroMode     )    
{


	short   val,  gr,  bl; 
	long    xTrav,    componentCombo;


	retXstart = retXend =   -1; 


	Read_Pixel(   x, y,    &val,   &gr, &bl    );   //   ********  Get the color of the SEED PIXEL ************


	long   targVal  =   RGBjm(  val,  gr,  bl   );    

	if(      targVal  <=  0  )
	{
		ASSERT( 0 );
		return  0;    // *******************  is this an ERROR by the CALLING function ????  ********************
	}



	if(    x  ==  0   )
	{
		retXstart =   0;
	}
	else
	{
		for(    xTrav =  ( x  - 1 );      xTrav  >=  0;      xTrav--    )
		{

			Read_Pixel(   xTrav,   y,     &val,  &gr,  &bl    );

			componentCombo  =   RGBjm(  val,  gr,  bl   );   


			if(     (        ! useNonZeroMode  &&   (  componentCombo   !=   targVal	 )   
				       ||      useNonZeroMode  &&   (  componentCombo  <=    0          )    )        //   Ending a  NOTE,

				||    xTrav  ==   0        )		                  //   ...or at  Map's beginning.
			{
				if(    xTrav  ==   0    )			
					retXstart  =   0;
				else
					retXstart  =    xTrav  + 1;

				break;   		
			}   
		} 
	}


														//  Go  RIGHT  till we hit a new pixel value

	if(    x   ==   ( m_width  -1 )     )
	{
		retXend  =     m_width  -1;    
	}
	else
	{
 		for(    xTrav =  ( x  + 1 );      xTrav  < m_width;      xTrav++    )
		{

			Read_Pixel(    xTrav,   y,   &val,  &gr,  &bl    );

			componentCombo  =   RGBjm(  val,  gr,  bl   );   

						

			if(     (        ! useNonZeroMode  &&   (  componentCombo   !=   targVal	 )   
				       ||      useNonZeroMode  &&   (  componentCombo  <=    0          )    )    //   Beginning of a new NOTE,

				||    xTrav  ==   ( m_width  -1 )      )		        //   ...or at  Map's end.
			{
				if(         xTrav  ==    ( m_width  -1 )     
	//				&&    componentCombo  ==  targVal      NOT Necessary ??????    2/12
				 )			
				{
					retXend  =   ( m_width  -1 );
				}
				else
					retXend  =    xTrav  - 1;

				break;   		
			}   
		} 
	}


	return  targVal;
}



											////////////////////////////////////////


long   OffMap::Find_Current_Intervals_EndPoints_By_3Components_Vertical(   long x,  long y,    long&  retYstart,   long&  retYend,    bool useNonZeroMode   )    
{


	short   val,  gr,  bl; 
	long    yTrav,    componentCombo;


	retYstart = retYend =   -1; 


	Read_Pixel(   x, y,    &val,   &gr, &bl    );   //   ********  Get the color of the SEED PIXEL ************


	long   targVal  =   RGBjm(  val,  gr,  bl   );    

	if(      targVal  <=  0  )
	{
		ASSERT( 0 );
		return  0;    // *******************  is this an ERROR by the CALLING function ????  ********************
	}




	if(    y  ==  0   )
		retYstart =   0;
	else
	{
		for(    yTrav =  ( y  - 1 );      yTrav  >=  0;      yTrav--    )
		{

			Read_Pixel(  x,   yTrav,    &val,  &gr,  &bl    );

			componentCombo  =   RGBjm(  val,  gr,  bl   );   
			

			if(     (        ! useNonZeroMode  &&   (  componentCombo   !=   targVal	 )   
				       ||      useNonZeroMode  &&   (  componentCombo  <=    0          )    )        //   Ending a  NOTE,

				||    yTrav  ==   0        )		                  //   ...or at  Map's beginning.     
			{
				if(    yTrav  ==   0    )			
					retYstart  =   0;
				else
					retYstart  =    yTrav  + 1;

				break;   		
			}   
		} 
	}


														//  Go  DOWNWARD  till we hit a new pixel value

	if(    y   ==   ( m_height  -1 )     )
	{
//		retYstart  =    m_height  -1;		 *******   'start'  :   WAS wrong for Vertical,  should be wrong here too   4/23/12	                PUT this logic up above ????????????   *****************   WRONG ???  4/23   ****************

		retYend  =     m_height  -1;    
	}
	else
	{
 		for(    yTrav =  ( y  + 1 );      yTrav  < m_height;      yTrav++    )
		{

			Read_Pixel(   x,  yTrav,    &val,  &gr,  &bl    );

			componentCombo  =   RGBjm(  val,  gr,  bl   );   
						


			if(     (        ! useNonZeroMode  &&   (  componentCombo   !=   targVal	 )   
				       ||      useNonZeroMode  &&   (  componentCombo  <=    0          )    )    //   Beginning of a new NOTE,

				||    yTrav  ==   ( m_height  -1 )      )		        //   ...or at  Map's end.
			{

				if(         yTrav  ==    ( m_height  -1 )     
			//		&&    componentCombo  ==  targVal     NOT Necessary ?????? 2/12
				)			
				{
					retYend  =   ( m_height  -1 );
				}
				else
					retYend  =    yTrav  - 1;

				break;   		
			}   
		} 
	}


	return  targVal;
}





											////////////////////////////////////////


long   OffMap::Find_Current_Interval_By_3Components_Vertical(   long  y,    long&  retYstart,     long&  retYend,    bool useNonZeroMode     )     
{                 	


	//	CALLED by:   


	//  if(  useHueMode == FALSE )    TESTS that the  "GROUP of all 3 components"   is the SAME for every Pixel in the calculated INTERVAL.    2/12
	//                                               So it test that the EXACT COLOR  is the SAME  in every PIXEL in the run.   But when in useHueMode...
	//
	//   BUT if ( useHueMode ==TRUE )  then we just test for CONSTANT Hue,  which is what we see  after DETECTION  in  SPitchCalc::m_drivingOffMapHorz



	/***     
			RETURNS:   the pixel GreyVal  for the  current run   ....OR

					returns  -2     for OUT of BOUNDs

					returns  -3     if all pixels are Zero  !!!  ( Only for maps that are Bigger than  1 Pixel high.  )  
	***/
   

    short  x,     val, gr, bl,       bigestX= -1;
//	long   yTrav;

	long   componentCombo,    targVal;
//	short  retHue,  retSaturation,   retValue;  

//	bool   hasSameComponentValues =   false;
       


//	targVal    =  -1;   
	targVal    =  0;        //  WAS  -1    :  NO!!!!   Only want a find if it is  "NOT black"    6/15/2012   



    retYstart  =  -1;
    retYend   =  -1;


    if(       y  <    0					 //  check OUT of BOUNDs 
		||   y  >=  m_height    )      
	{
		return  -2;						
	}
   
	Lock_Bits();					 



	if(    m_width  ==   1    )
	{
		bigestX =  0;     // only one row, so this is our  'ROW of Focus'

		Read_Pixel(   bigestX,  y,    &val,   &gr, &bl    );

		componentCombo  =   RGBjm(  val,  gr,  bl   );       
	}
	else
	{
						//  Find the X-row with the biggest value,  and ASSUME this is the SegmentRun we want to detect

	    for(    x= 0;    x< m_width;    x++   )      
		{ 
			
			 Read_Pixel(   x, y,    &val,  &gr,&bl    );

			 componentCombo =   RGBjm(  val,  gr,  bl   );   
			

			 if(   componentCombo   >   targVal    )     //  NOTE:  must  find a value  'GREATER THAN zero'
			 {  
				  targVal  =   componentCombo;
				  bigestX  =     x;   // *** NEVER gets assigned UNLESS one pixel has a value GREATER that ZERO !!!     
			 }
		}  


		if(   bigestX  <  0   )   //  no value was greater than zero, so we can do nothing
		{
			return  -3;   // ***** DOES this HAPPEN ????    2/7/2012  **********************************************
		}
	}



	long   targColor =    Find_Current_Intervals_EndPoints_By_3Components_Vertical(   bigestX,  y,     retYstart,   retYend,    useNonZeroMode   );    



    Unlock_Bits();  
    

    return    targVal;  
}



											////////////////////////////////////////


long    OffMap::Get_Colors_PairMember_Clique_Score(   long x,  long y,    short  targColor,    short  kernalWidth   )
{

	//  while  'targColor'  is the Posterized-GreyTone of the region,   it also acts as a  'RegionID'   ( 255 is undefined )


	ASSERT(   kernalWidth  ==  3  );      //  **** BELOW is ONLY for a 3x3 kernal   ....WRITE more cases.   7/02



	long    beta =  1;    // use UNIITARY,  the result can then be weighted by calling function,  and have same effect as adjusting an INPUT beta

	long    retScore =  0;
	short   val0,  val1,   rd, gr,     neibLeft,  neibRight;
	short   xKern,  yKern,     kernalMags[ 8 ];

	

																//  First get the mags for the 8  Kernal pixels
	for(    short i =0;     i <  8;     i++    )	
	{

		xKern  =     kernalThreeOMAP[ i ].X;
		yKern  =     kernalThreeOMAP[ i ].Y;


		if(         (xKern +x)  >=  0       &&      (xKern +x)  <  m_width   
			&&    (yKern +y)  >=  0       &&      (yKern +y)  <  m_height     ) 
		{
			Read_Pixel(    (xKern +x),   (yKern +y),      &val0,    &rd, &gr     );

			kernalMags[ i ] =    val0;
		}
		else
			kernalMags[ i ] =  -1;    //  -1  UNdefined 
	}



							  //  Go thru all  clique's   'Neighbor Pairs'   and test for  'same color'  ( a sign of 'clustering' )

	for(   short i =0;     i <  12;      i++    )   
	{

		neibLeft    =     cliquePairsOMAP[ i ].L;
		neibRight  =     cliquePairsOMAP[ i ].R;

		val0  =   	kernalMags[  neibLeft    ];
		val1  =     kernalMags[  neibRight  ];

																							
		if(        val0  >=  0      &&    val0  !=  255
			&&   val1  >=  0      &&    val1  !=  255     )   //  do not create cliques  with UNdefined or UNread pixel-Values
		{

			if(   val0  ==   val1   )    //  do we have a  'Clique'  ( ie:  a group of pixels with matching color )...
			{
				if(   targColor  ==   val0   )   
					retScore  +=    beta;			
				else																							//  **** NEED to reverse signs ???? *****
				{
					retScore  -=     beta;   //  ***REDUNDANT in Total Score ???      //  SUBTRACT if the Clique does NOT match the center pixel's color
				}
			}
		}
	}


	return   retScore;
}





											////////////////////////////////////////


long    OffMap::Get_Colors_SingleMember_Clique_Score(   long x,  long y,    short  targColor,    short  kernalWidth   )
{

		

	//  while  'targColor'  is the Posterized-GreyTone of the region,   it also acts as a  'RegionID'   ( 255 is undefined )


	ASSERT(   kernalWidth  ==  3  );      //  **** BELOW is ONLY for a 3x3 kernal   ....WRITE more cases.   7/02


	short   alpha =  1;

	long    retScore =  0;
	short   val,   rd, gr;
	short   xKern,  yKern;

	

																//  First get the mags for the 8  Kernal pixels
	for(    short i =0;     i <  8;     i++    )	
	{

		xKern  =     kernalThreeOMAP[ i ].X;
		yKern  =     kernalThreeOMAP[ i ].Y;


		if(         (xKern +x)  >=  0       &&      (xKern +x)  <  m_width   
			&&    (yKern +y)  >=  0       &&      (yKern +y)  <  m_height     ) 
		{

			Read_Pixel(    (xKern +x),   (yKern +y),      &val,    &rd, &gr     );


			if(         val   >=   0      
				&&    val   !=    255    )     //  do not create cliques  with UNdefined or UNread pixel-Values
			{

				if(     targColor   ==    val    )   
					retScore  +=    alpha;			
				else																						
					retScore  -=     alpha;       //  SUBTRACT if the Clique does NOT match the center pixel's color
			}
		}
	}


	return   retScore;
}




											////////////////////////////////////////


short     OffMap::Copy_Xcolumn(    long  xSRC,    long  xDST    )
{


	short   retMaxValue =  -1;


	if(			(    xSRC  ==  xDST   )

			||  (   xSRC  >=   m_width    )
			||  (   xDST  >=   m_width    )

			||  (   xSRC  <   0    )
			||  (   xDST  <   0    )		)
	{
		ASSERT( 0 );
		return -1;
	}


	short   rd,  gr, bl;

	for(    short y=0;     y <  m_height;		y++     )
	{

		Read_Pixel(   xSRC,  y,    &rd,  &gr,  &bl  );  

		if(    rd  >   retMaxValue    )
			retMaxValue =   rd;

		Write_Pixel(   xDST,  y,     rd,     gr,    bl    );  
	}


	return   retMaxValue;
}



											////////////////////////////////////////


short     OffMap::Copy_Yrow(    long  ySRC,    long  yDST    )
{


	short   retMaxValue =  -1;


	if(			(    ySRC  ==  yDST   )

			||  (   ySRC  >=   m_height    )
			||  (   yDST  >=   m_height    )

			||  (   ySRC  <   0    )
			||  (   yDST  <   0    )		)
	{
		ASSERT( 0 );
		return -1;
	}


	short   rd,  gr, bl;

	for(    short x=0;     x <  m_width;		x++     )
	{

		Read_Pixel(   x,  ySRC,    &rd,  &gr,  &bl  );  

		if(    rd  >   retMaxValue    )
			retMaxValue =   rd;

		Write_Pixel(   x,  yDST,     rd,     gr,    bl    );  
	}


	return   retMaxValue;
}




											////////////////////////////////////////


void     OffMap::Scroll_Horizontally(    bool  scrollLeft,    long  pixelsToScroll    )
{

			//  Later OPTIMIZE this   

	if(   pixelsToScroll  < 1   )
	{
		AfxMessageBox(   "OffMap::Scroll_Horizontally FAILED,  pixelsToScroll is too small."   );
		return;
	}

	long   xSrc,   xDst;



	if(   scrollLeft    )
	{

		for(    xSrc =   pixelsToScroll;      xSrc <  m_width;     xSrc++   ) 
		{

			xDst  =   xSrc -  pixelsToScroll;

			Copy_Xcolumn(   xSrc,    xDst   );
		}
	}
	else    //   scroll  right
	{       

	//	ASSERT( 0 );    //   Need to INSTALL and TEST    this     3/10


		for(    xSrc =  (  (m_width  -1)  - pixelsToScroll  );       xSrc  >=  0;      xSrc--   ) 
		{

			xDst  =   xSrc  +  pixelsToScroll;

			Copy_Xcolumn(   xSrc,    xDst   );
		}
	}
}



											////////////////////////////////////////


void     OffMap::Scroll_Vertically(    bool  scrollUp,    long  pixelsToScroll    )
{

			//  Later OPTIMIZE this   

	if(   pixelsToScroll  < 1   )
	{
		AfxMessageBox(   "OffMap::Scroll_Vertically FAILED,  pixelsToScroll is too small."   );
		return;
	}

	long   ySrc,   yDst;



	if(   scrollUp    )
	{

		for(    ySrc =   pixelsToScroll;      ySrc <  m_height;     ySrc++   ) 
		{

			yDst  =   ySrc -  pixelsToScroll;

			Copy_Yrow(   ySrc,    yDst   );
		}
	}
	else    //   scroll  down
	{       

	//	ASSERT( 0 );    UNTESTED  ....Looks OK   11/19/11


		for(    ySrc =  (  (m_height  -1)  - pixelsToScroll  );       ySrc  >=  0;      ySrc--   ) 
		{

			yDst  =   ySrc  +  pixelsToScroll;

			Copy_Yrow(   ySrc,    yDst   );
		}
	}
}



											////////////////////////////////////////


void    OffMap::Assign_Yrow(    long  y,    short  greyVal    )
{

	if(	   y  >=  m_height    ||     y < 0     )
	{
		ASSERT( 0 );
		return;
	}


	for(    short x=0;     x <  m_width;		x++     )
		Write_Pixel(   x,  y,     greyVal,  greyVal, greyVal   );  
}




											////////////////////////////////////////


void    OffMap::Assign_Xcolumn(    long  x,    short  greyVal    )
{

	if(	   x  >=  m_width    ||     x < 0     )
	{
		ASSERT( 0 );
		return;
	}


	for(    short y=0;     y <  m_height;		y++     )
		Write_Pixel(   x,  y,     greyVal,  greyVal, greyVal   );  
}




											////////////////////////////////////////


void    OffMap::Write_Pixel(   long x,  long y,     short  rd,  short  gr,  short  bl   )
{

	// *******  CORRECTED for 24bit color write ******************************


		//  For  256 GRAY,  just use  'rd'  for value
	
											//  ( MAC Only: MUST call 'Lock_Bits'  & 'Unlock_Bits' before and after )

	ASSERT(   m_dib  !=  NULL   );


	if(    y >=  m_height    )
	{
	//	ASSERT( 0 );     //   int   dummyBreak =  9;     //  Get here if I make my regions too big with List Ops   12/3/08
		return;
	}


	if(    x >=  m_width    )
	{
//		ASSERT( 0 );     //   int   dummyBreak =  9; 
		return;
	}




	if(     y < 0     ||     x < 0     )
	{
		//   ASSERT( 0 );     
		int   dummyBreak =  9;    // ***** BUG,  reactivate this and TEST !!!!! ************
		return;
	}



	if(    m_depth ==  8    )
	{

		int            val  =   rd;     //  For  256 GRAY,  just use rd for value
		DWORD    idx;


		if(          val  > 255    )	    
			val =  255;
		else if(   val  <  0     )	  
			val =   0;

		
		long  realY =   ( m_height  -1 )   -   y;      //  Have to flip the Y-coords   for a  'DIB' ( this is enough, just write to it UPSIDEDOWN and all else is OK  ...blits etc )


		idx  =    (  ((DWORD)realY)  *  ((DWORD)m_byRow)  )   +    (  (DWORD)x  );    

		if(    idx  >   (DWORD)m_totMapBytes    )
		{
			ASSERT( 0 );
			return;
		}
		else
			m_bits[ idx ] =   val;  
	}

	else if(    m_depth  ==  1    )			//   IS  inverted   !!!!!
	{

	    unsigned char   mask = 0x80,     *ptr;       //  use 'rd' for 8bit & 1bit

		mask   >>=   ( x  %  8 );           
		


		long  realY =   ( m_height  -1 )   -   (long)y;      //  Have to flip the Y-coords   for a  'DIB' ( this is enough, just write to it UPSIDEDOWN and all else is OK  ...blits etc )


        ptr  =     m_bits    +   realY * m_byRow      +     (long)( x /8  );
   
		/***
        if(  rd ==  255   &&    gr == 255    &&   bl  == 255     )  
			*ptr   &=   (~mask);      // white,    rgb= 255 
         else                                 
			 *ptr   |=      mask;        // bit SET is black on Mac 1bit maps 
        ***/
        if(       rd ==  0   &&    gr == 0    &&   bl  == 0     )  
			*ptr   &=   (~mask);       //  white,    rgb= 255 
         else                                 
			 *ptr   |=      mask;        //   bit SET is ,   white on my maps		
	}

	else if(    m_depth ==  24    )
	{

		DWORD    idx;

		if(          rd  > 255    )	    
			rd =  255;
		else if(   rd  <  0     )	  
			rd =   0;

		if(          gr  > 255    )	    
			gr =  255;
		else if(   gr  <  0     )	  
			gr =   0;

		if(          bl  > 255    )	    
			bl =  255;
		else if(   bl  <  0     )	  
			bl =   0;


		
		long  realY =   ( m_height  -1 )   -    y;      //  Have to flip the Y-coords   for a  'DIB' ( this is enough, just write to it UPSIDEDOWN and all else is OK  ...blits etc )


		idx  =    (  ((DWORD)realY)  *  ((DWORD)m_byRow)  )   +    (    (DWORD)x  *3    );     //   3  bytes per pixel



		if(    ( idx +2 )   >   (DWORD)m_totMapBytes    )
		{
			ASSERT( 0 );
			return;
		}
		else
		{  		
			/*********************************************** CORRECTION,  swap bytes   8/08 ********************************
			m_bits[    idx         ]  =    (unsigned char)rd;  
			m_bits[  ( idx +1 ) ]  =    (unsigned char)gr;  
			m_bits[  ( idx +2 ) ]  =    (unsigned char)bl;  
			****/
			m_bits[    idx         ]  =    (unsigned char)bl;  
			m_bits[  ( idx +1 ) ]  =    (unsigned char)gr;  
			m_bits[  ( idx +2 ) ]  =    (unsigned char)rd;  
		}
	}
	else
		ASSERT( 0 );
}



											////////////////////////////////////////


void   OffMap::Read_Pixel(   long x,  long y,      short *rd,  short *gr,  short *bl   )
{

	// ***************** BUG for 24bit color ???   Read about  Read_Pixel_Hacked()  *********************************  8/08


		//  For  256 GRAY,  just need  'rd'  for value
	
											//  ( MAC Only: MUST call 'Lock_Bits'  & 'Unlock_Bits' before and after )
	ASSERT(   m_dib  !=  NULL   );


	/**  assign different values by map depth
	if(      y  >=   m_height    ||   y   <     0     ||   x  >=   m_width      ||   x  <     0    )
	{
		//   ASSERT( 0 );     ....Not a REAL big deal,  get this from  ConvolutionMapFilter::Filter()  ...FUTURE, clean up   3/2002
		*rd = *gr = *bl =   0;
		return;
	}
	****/





	if(    m_depth ==  8    )
	{

		if(      y  >=   m_height    ||   y   <     0     ||   x  >=   m_width      ||   x  <     0    )
		{
			//   ASSERT( 0 );     ....Not a REAL big deal,  get this from  ConvolutionMapFilter::Filter()  ...FUTURE, clean up   3/2002
			*rd = *gr = *bl =   0;
			return;
		}


		long  realY =   ( m_height  -1 )   -   y;        //  Have to flip the Y-coords   for a  'DIB' 


		long   idx  =     (  realY  *  m_byRow  )   +   x;
		if(      idx  >  m_totMapBytes    )
		{



// **** PUT trace back,  comeup with a better solution !!!!!!!

//			TRACE(   "***WARN:  OffMap::Read_Pixel()   ...Trying to read OFF the OffMap.\n"   );



			//  ASSERT( 0 );   ....Not a REAL big deal,  get this from  ConvolutionMapFilter::Filter()  ...FUTURE, clean up   3/2002

			*rd = *gr = *bl =    0;
			return;
		}
		else	
			*rd = *gr = *bl =   m_bits[  idx ];  
	}

	else if(    m_depth ==  1   )
	{

		if(      y  >=   m_height    ||   y   <     0     ||   x  >=   m_width      ||   x  <     0    )
		{
			//   ASSERT( 0 );     ....Not a REAL big deal,  get this from  ConvolutionMapFilter::Filter()  ...FUTURE, clean up   3/2002
			*rd = *gr = *bl =   0;
			return;
		}

		unsigned char   mask = 0x80,    *ptr;		//   use  'rd'  for  8bit  &  1bit

		mask   >>=    ( x % 8 );


		long  realY =   ( m_height  -1 )   -   y;      //  Have to flip the Y-coords   for a  'DIB' ( this is enough, just write to it UPSIDEDOWN and all else is OK  ...blits etc )


        ptr  =    m_bits    +     ( realY * m_byRow )     +    ( x /8 );
               
        if(   ( *ptr  &  mask )   ==   mask   )    
			*rd= *gr= *bl=   255;   //  bit is SET,  so is white 
        else                          
			*rd= *gr= *bl=      0;
	}

	else if(    m_depth ==  24    )
	{

		if(      y  >=   m_height    ||   y   <     0     ||   x  >=   m_width      ||   x  <     0    )
		{
			*rd = *gr = *bl =    255;   //   0;    ***** TEST with white    11/08  ***************
			return;
		}



		long  realY =   ( m_height  -1 )   -   y;        //  Have to flip the Y-coords   for a  'DIB' 


		long   idx  =     (  realY  *  m_byRow  )   +  ( x * 3 );
		if(  (  idx +2 )  >  m_totMapBytes    )
		{



// **** PUT trace back,  comeup with a better solution !!!!!!!

//			TRACE(   "***WARN:  OffMap::Read_Pixel()   ...Trying to read OFF the OffMap.\n"   );



			//  ASSERT( 0 );   ....Not a REAL big deal,  get this from  ConvolutionMapFilter::Filter()  ...FUTURE, clean up   3/2002

			*rd = *gr = *bl =    0;
			return;
		}
		else	
		{   

			if(      y  >=   m_height    ||   y   <     0     ||   x  >=   m_width      ||   x  <     0    )
			{
				*rd = *gr = *bl =    255;   //   0;   ***** TEST with white    11/08  ***************
				return;
			}

			
			
			// *******************************************************************************************************************   
			// **********  A BUG here causes a bug where the RED and BLUE values are switched???   See  Read_Pixel_Hacked()  below.   8/08  
			// *******************************************************************************************************************   

			/****
			*rd  =   m_bits[  idx ];  
			*gr  =   m_bits[  idx +1 ];  
			*bl  =   m_bits[  idx +2 ];  
			*****/
			*rd  =   m_bits[  idx +2 ];      //  Now swapped and working OK
			*gr  =   m_bits[  idx +1 ];  
			*bl  =   m_bits[  idx      ];  
		}
	}

	else
		ASSERT( 0 );
}



											////////////////////////////////////////


void   OffMap::Read_Pixel_Filtered_Horz(   long x,  long y,    short  filterCode,   short  kernalWidth,    short *rdPtr,  short *grPtr,  short *blPtr   )
{


// ???? *****   " Gets tricky  for OnFly reads  and  Get_DurationRectange()"   ...what did I mean by that ???   2/03 *************  JPM


	
	if(   kernalWidth >  32   )
	{
		ASSERT( 0 );
		Read_Pixel(   x, y,     rdPtr,  grPtr,  blPtr   );
	}


	short   cnt =  0;
	short   i, j,  tp,   max,  min,  val,  rd, gr, bl;
	short   histo[ 256 ];

//    short   histo[ 256 ],  dx,dy,  mfp,   hld,  tp, tp2;



	switch(   filterCode   )
	{

		case   ConvolutionMapFilter::NONE : 

			Read_Pixel(   x, y,     rdPtr,  grPtr,  blPtr   );
		break;



		case   ConvolutionMapFilter::BLURHORZ : 
					
			ASSERT(  kernalWidth  >  1  );   
                   
			Read_Pixel(   x, y,    &rd,  &gr, &bl    );  
			val  =   rd;
			cnt  =    1;

			for(   i =0;     i <  ( kernalWidth -1 );     i++   )
			{
				Read_Pixel(     (   x   + offsetTableGLB[ i ]   ),     y,       &rd,  &gr, &bl   );  
			    val  +=   rd; 
				cnt++;
			}
                   
            val  /=   cnt;       
			
			*rdPtr = *grPtr = *blPtr =    val;
		break;



		case   ConvolutionMapFilter::MEDIANHORZ : 
					
			ASSERT(  kernalWidth  >  1  );    // 3, 4, 5:  kernal size( ODD or EVEN )                    

			/***
            for(  i =0;  i< kernalWidth;   i++  )   		// get kernals values
            {
				Read_Pixel(   x +  i -(kernalWidth/2),    y,     &rd,  &gr,&bl  ); 
				histo[ i ]=  rd;  
			}
			***/
			Read_Pixel(   x, y,    &rd,  &gr, &bl    );     
			histo[ 0 ] =  rd;  

			for(   i =0;     i <  ( kernalWidth -1 );     i++   )
			{
				Read_Pixel(     (   x   + offsetTableGLB[ i ]   ),     y,       &rd,  &gr, &bl   );  
				histo[ i +1 ] =  rd;
			}



			for(  i= 0;   i< kernalWidth -1;   ++i  )          //     sort the  GREY-Values in order( BUBBLE SORT ),  and use the middle value.
			{  
				for(  j= kernalWidth -1;   i< j;   --j   )        //   EX:     {   20    56     123     230   250  }   =    123            
				{  
					if(    histo[ j-1 ]  <   histo[ j ]     )         //  Unlike BLUR filter,  the MEDIAN filter IGNORES the EXTREME values in its CALCULATIONS.
					{ 
						tp              =    histo[ j-1 ]; 
                        histo[ j-1 ] =    histo[ j ];
                        histo[ j ]    =    tp;
                    }
                 } 
			}

			val =  histo[  kernalWidth /2  ];    // use MIDDLE value( careful on centering ) 
			
			*rdPtr = *grPtr = *blPtr =    val;
		break;



		case   ConvolutionMapFilter::DIALATEhORZ :     //  If I use this on a GREYSCALE map it becomes EROSION because Black is '0'   ...not a MAX value.  9/2012
					
			ASSERT(  kernalWidth  >  1  );    // 3, 4, 5:  kernal size( ODD or EVEN )                    

			max = 0;     
                   
			Read_Pixel(   x,  y,    &val,  &gr,&bl  );  
			if(   val  >  max   )  
			    max =   val;

			for(   i =0;     i <  ( kernalWidth -1 );     i++   )
			{
				Read_Pixel(    (  x  + offsetTableGLB[ i ]  ) ,   y,       &val,  &gr, &bl  );  

				if(    val  >  max    )  
				    max =   val;
			}
			
			*rdPtr = *grPtr = *blPtr =    max;
		break;



		case   ConvolutionMapFilter::ERODEhORIZ :     //  If I use this on a GREYSCALE map it becomes DIALATE because Black is '0'   ...not a MAX value.  9/2012
					
			ASSERT(  kernalWidth  >  1  );    // 3, 4, 5:  kernal size( ODD or EVEN )                    

			min = 255;     
                   
			Read_Pixel(   x,  y,    &val,  &gr,&bl  );  
			if(   val  <  min   )  
			    min =   val;


			for(   i =0;     i <  ( kernalWidth -1 );     i++   )
			{
				Read_Pixel(    (  x  + offsetTableGLB[ i ]  ) ,   y,       &val,  &gr, &bl  );  

				if(   val  <  min   )  
				    min =   val;
			}
			
			*rdPtr = *grPtr = *blPtr =    min;
		break;



        default:    ASSERT( 0 );     break; 
	}
}





											////////////////////////////////////////


void   OffMap::Dump_Max_Column_Values()
{


	if(   m_width   > 15   )
	{
	//	int   dummyBreak =   9;
		TRACE(  "\n		FLOAT-map:    *** Too BIG for  Dump_Max_Column_Values()  \n"    );
		return;
	}




	long   maxs[  18  ];

	for(  long  x=0;   x <  m_width;      x++  )
		maxs[  x  ] =   0;


	for(   long  x=0;     x <  m_width;      x++   )
	{
		short  retValue =  0;

		long  y  =   Get_Ycoord_of_Columns_MaxValue(  x,   retValue  );
		
		if(   retValue   >=   0   )
			maxs[  x  ] =    retValue;
	}


	TRACE(  "\n		FLOAT-map:   0[ %d ],  1[ %d ],  2[ %d ],  3[ %d ],  4[ %d ],  5[ %d ],  6[ %d ],  7[ %d ],  8[ %d ],  9[ %d ],  10[ %d ],  11[ %d ],  12[ %d ],  13[ %d ],  14[ %d ]   \n",
		maxs[ 0 ], maxs[ 1 ],  maxs[ 2 ],  maxs[ 3 ],  maxs[ 4 ],  maxs[ 5 ],  maxs[ 6 ],  maxs[ 7 ],  maxs[ 8 ],  maxs[ 9 ],  maxs[ 10 ],  maxs[ 11 ],  maxs[ 12 ],  maxs[ 13  ],   maxs[ 14 ]    ); 
}





											////////////////////////////////////////


void   OffMap::RGB_to_HSV(   short  rd,  short  gr,  short  bl,      short&   retHue,    short& retSaturation, 
																								 	short&   retValue    )
{
	retHue  =  -9;
	retSaturation =  -9;
	retValue =  -9; 

	
	short   minComp  =    minj(    rd,   gr   );
	minComp            =    minj(    minComp,   bl   );


	short   maxComp  =    maxj(   rd,   gr   );
	maxComp            =    maxj(   maxComp,   bl   );

	retValue =    maxComp;



	short   testMax  =    max3j(  rd,   gr,   bl    );	 

	ASSERT(    testMax  ==    maxComp    ); 




	if(   maxComp   !=  0   )
	{
		retSaturation  =   (short)(        (   ((long)( maxComp  -  minComp ))    *  255L   )  /   (long)maxComp     );
	}
	else
		retSaturation  =  0;



	if(   retSaturation  ==  0   )
		retHue =  -1;     //  UNDEFINED
	else
	{
		double  hue;
		double  delta =    (double)(   maxComp  -  minComp   );


		if(       rd  ==  maxComp   )
		{
			hue  =                         (double)( gr - bl )   / delta;
		}
		else if(  gr  ==  maxComp   )
		{
			hue  =      2.0   +    (    (double)( bl - rd )   / delta    );
		}
		else if(  bl  ==  maxComp   )
		{
			hue  =      4.0   +    (    (double)( rd - gr )   / delta    );
		}
 

		hue  *=    60.0;

		if(   hue  <  0.0   )
			hue  +=    360.0;


		retHue  =   (short)hue;
	}
}



											////////////////////////////////////////


void   OffMap::HSV_to_RGB(   short hue,   short saturation,    short value,     short&  retRd,   
																											short&  retGr,    short&  retBl    )
{

	// ***** DO I need tho make sure that   hue,  saturation,  value, are in the correct range for this to work OK ??  See bottom.  11/08

	retRd =  -9;
	retGr =  -9;
	retBl =  -9;


	if(    saturation  ==   0    )
	{

		if(   hue  ==  -1   )
		{
			retRd =  value;
			retGr =  value;
			retBl  =  value;
		}
		else
		{	//  ASSERT( 0 );    // **** ERROR *****

			retRd =  value;			//   ...better than nothing
			retGr =  value;
			retBl  =  value;
		}

		return;
	}



	if(   hue  ==  -1   )        // *****  NEW,    3/2010      Is this OK ?????    Seems consistent with what is above
	{
		retRd =  value;
		retGr =  value;
		retBl  =  value;

		return;
	}







	if(   hue  ==   360   )
		hue =  0;



	double   hueDB  =     (double)hue  /  60.0;


	long         i  =         (long)hueDB;


	double  fDB =         hueDB   -   (double)i;    //  get the fraction part of  hueDB



/****   OLD and VERY buggy   ...fixed below 8/08

	short  p   =    value  *   (    255  -                                  saturation                    );			//    value  *   ( 1 -  saturation  );
	short  q  =     value  *   (    255  -     (short)(    ( (double)saturation   *             fDB      )         )       );
	short  t  =     value  *   (    255  -     (short)(    ( (double)saturation    *   ( 1.0 - fDB )    )           )       );
*****/


	double   saturationFloat =      ((double)saturation)  /  255.0;
	double   valueFloat        =      ((double)value)        /  255.0;



	double  pFloat   =     valueFloat   *    ( 1.0 -  saturationFloat  );


//	double  qFloat  =     value        *    (    255  -     (short)(    ( (double)saturation   *             fDB      )         )       );
	double  qFloat  =     valueFloat  *   (     1.0  -       ( saturationFloat   *  fDB  )                         );


//	double  tFloat  =     value        *   (    255  -     (short)(    ( (double)saturation    *   ( 1.0 - fDB )    )           )       );
	double  tFloat  =     valueFloat  *   (    1.0  -       (   saturationFloat    *   ( 1.0 - fDB )    )         );




	short  p  =      (short)(    pFloat   *   255.0    );
	short  q  =      (short)(    qFloat   *   255.0    );
	short  t  =       (short)(    tFloat   *   255.0    );

	ASSERT(   p  >=  0    &&     p <= 255  );
	ASSERT(   q  >=  0    &&     q <= 255  );
	ASSERT(   t  >=  0    &&      t <= 255  );




	switch(  i  )
	{
		case  0:	   retRd =   value;              retGr =          t;                retBl =        p;				 break;  

		case  1:	   retRd =         q;              retGr =   value;                retBl =        p;				 break;  

		case  2:	   retRd =         p;              retGr =   value;                retBl =         t;				 break;  


		case  3:	   retRd =         p;              retGr =         q;                retBl =   value;				 break;  

		case  4:	   retRd =         t;               retGr =         p;                retBl =   value;				 break;  

		case  5:	   retRd =   value;              retGr =         p;                retBl =         q;				 break;  



		default:     ASSERT( 0 );         break;
	}




	bool   hadBug =   false;


	if(   retRd > 255  )      {  retRd =  255;   hadBug =   true;   }
	if(   retRd <    0  )      {  retRd =      0;   hadBug =   true;   }

	if(   retGr > 255  )      {  retGr =  255;   hadBug =   true;   }
	if(   retGr <    0  )      {  retGr =      0;   hadBug =   true;   }

	if(   retBl > 255  )      {  retBl =  255;   hadBug =   true;   }
	if(   retBl <    0  )      {  retBl =      0;   hadBug =   true;   }


	if(   hadBug  )
	{
		TRACE(  "*****ERROR [ OffMap::HSV_to_RGB ] :  Overflow value. \n"   );
	}
}






short     OffMap::Get_Degree_Quadrant(   short  degrees   )
{

	// ********************************   UNTESTED, and unused    10/08   ********************************

			//  quadrants are numbered,   clockwise    {  0,   1,   2,   3   }

				//   Utility for unwinding  HUE vaues  (  0 -360 deg ).  gives negative values 

	if(   degrees  >  360  )
		degrees  -=   360;

	if(   degrees  >  360  )
		degrees  -=   360;



	if(   degrees  <  -360  )
		degrees  +=   360;

	if(   degrees  <  -360  )
		degrees  +=   360;





	short  quadrant =  9;


	if(    degrees  ==  0   )	
		return  0;

	if(    degrees  ==  360    )    //  ???   OK  ?????
		return  3;     //  or 0  ???



	if(     degrees > 0      )     //  degrees is POSITIVE
	{

		if(           degrees >=    0    &&    degrees  <  90    )
			quadrant =   0;
		else if(    degrees >=   90    &&    degrees  <  180    )
			quadrant =   1;
		else if(    degrees >=  180    &&    degrees  <  270    )
			quadrant =   2;
		else if(    degrees >=  270    &&    degrees  <  360    )
			quadrant =   3;
		else
		{	ASSERT(  0  );   }
	}

	else      //  degrees is POSITIVE
	{

		if(        degrees   <  -360      &&    degrees   >  -270      )
			quadrant =   0;
		else if(  degrees  <=  -270    &&    degrees  >  -180    )
			quadrant =   1;
		else if(  degrees  <=  -180    &&    degrees  >    -90    )
			quadrant =   2;
		else if(  degrees  <=    -90    &&    degrees  >      0    )
			quadrant =   3;
		else
		{	ASSERT(  0  );   }
	}


	ASSERT(  quadrant  <=   3   );


	return   quadrant;
}



											////////////////////////////////////////


short     OffMap::Subtract_Degrees(   short  number,      short  subtractor   )
{

					// ***** SEEMS to work,  but be careful    10/08  *****************


				//   Utility for unwinding  HUE vaues  (  0 - 360 deg ).  Gives values   {   -180  < value  <  +180    )

	short  answer;


	if(   number  >  360  )     //  get into range of    -360   to   +360
		number  -=   360;

	if(   number  >  360  )
		number  -=   360;

	if(   number  <  -360  )
		number  +=   360;

	if(   number  <  -360  )
		number  +=   360;



	if(   subtractor  >  360  )
		subtractor  -=   360;

	if(   subtractor  >  360  )
		subtractor  -=   360;

	if(   subtractor  <  -360  )
		subtractor  +=   360;

	if(   subtractor  <  -360  )
		subtractor  +=   360;




	if(  number  < 0   )				//  make both numbers positive
		number  +=    360;     

	if(  subtractor  < 0   )				
		subtractor  +=    360;     



	answer  =    number   -   subtractor;


	if(   answer  <  -180   )
		answer +=   360;


	if(   answer  >  180   )
		answer -=   360;



	ASSERT(     answer <=  180      &&    answer  >=  -180    );

	return   answer;
}




											////////////////////////////////////////


bool   OffMap::Calc_Color_Separation(    SeparationCreateParms&  parms,    OffMap&  sourceMap,   
																							short& retAvgRed,   short& retAvgGreen,  short& retAvgBlue,    CString&   retErrorMesg  )
{


	short   topCompVal =  256;     //  was 255, but  want to haved all colors on PowerOfTwo boundaries  7/09



	retErrorMesg.Empty();

	retAvgRed =  retAvgGreen  =  retAvgBlue  =   -3;


	if(        m_width    !=   sourceMap.m_width
		||    m_height   !=   sourceMap.m_height
		||    m_depth    !=   sourceMap.m_depth      )
	{
		retErrorMesg =    "OffMap::Make_Color_Separation  failed,  bitmaps are NOT the same size" ;
		return  false;
	}





																						//   Calc  the local parms by the algo 
	short   upperHueLimit= 0,        lowerHueLimit=0;  
	short   upperHueLimitOrig= 0,  lowerHueLimitOrig=0;  
	bool    hasDisjointedSet  =   false;
	short   cHunt, mHunt, yHunt,    cHuntOrig, mHuntOrig, yHuntOrig;   



	if(          parms.algoCode  ==   SeparationCreateParms::TARGEThUE   )
	{

		upperHueLimit  =      parms.targetHue     +   parms.hueSpread;
		lowerHueLimit   =      parms.targetHue     -    parms.hueSpread;

		upperHueLimitOrig =     upperHueLimit;
		lowerHueLimitOrig =     lowerHueLimit;


		// ******************************  an unwrapping routine   10/08  ***************************************

		if(   lowerHueLimit < 0   )     
			lowerHueLimit   =    360   -    ( -1 * lowerHueLimit );    //  want the absolute value  of  lowerHueLimit   to be subtracted,  winding counter-clockwise
		

		if(   upperHueLimit > 360   )     
			upperHueLimit   =    upperHueLimit  -  360;
		

		if(    lowerHueLimit  >   upperHueLimit  )
			hasDisjointedSet =   true;         //  need to check 2 areas of unit circle:    from lowerHueLimit to 360  ...AND    from 0 degrees to UpperLimit

		// *************************************************************************************************

	}
	else if(        parms.algoCode  ==   SeparationCreateParms::COMPONENTsUBTRACT   )
	{


//  *********  Is ther a better ALGO that would preserve some variety of Briteness or Saturation Hue,  like the  TARGEThUE-algo???  **************
//						... just increase the concept of tolerance for more matches and put the tolerance parm on the Dialog for adjustment by user.   10/08


		//  double   tolerance =  .05;     SHOULD the tolerance be a percentage of the total INK ( cyan, mag, yessow for the color ????  )    10/08



/****************************  TEMP disable ****************************************

		short  tolerance =   parms.componentAlgoTolerance;     //  10    ADJUSTable  
************************************/
		short  tolerance =  0;




		/***
		cHunt   =   ( topCompVal  -   parms.huntRed  )    /  huntRedux;    ********** DOES this have any potential, instead of tolerance ??? *******************
		mHunt  =   ( topCompVal  -   parms.huntGreen )  /  huntRedux;
		yHunt   =   ( topCompVal  -   parms.huntBlue   )  /  huntRedux;
		***/


		cHuntOrig   =    topCompVal  -   parms.huntRed;
		if(  parms.huntRed ==  255   )    cHuntOrig =  0;    //  special case for 255, not on a numeric boundary


		mHuntOrig  =    topCompVal  -   parms.huntGreen;
		if(  parms.huntGreen ==  255   )    mHuntOrig =  0; 


		yHuntOrig   =    topCompVal  -   parms.huntBlue;
		if(  parms.huntBlue ==  255   )    yHuntOrig =  0; 




		cHunt   =   cHuntOrig      -   tolerance;
		mHunt  =   mHuntOrig    -   tolerance;
		yHunt   =   yHuntOrig     -   tolerance;




		if(  cHunt  <  0   )
			cHunt =   0;

		if(  mHunt  <  0   )
			mHunt =   0;

		if(  yHunt  <  0   )
			yHunt =   0;


		retAvgRed     =     parms.huntRed; 
		retAvgGreen  =    parms.huntGreen;
		retAvgBlue    =     parms.huntBlue;
	}
	else if(        parms.algoCode  ==   SeparationCreateParms::OPAQUEsEPARATION   )
	{

		ASSERT( 0 );    // ************   INSTALL   *****************
	}

	else
	{  retErrorMesg =    "OffMap::Make_Color_Separation  failed,  unknown algoCode" ;
		return  false;
	}
///////////////////////////////////////////////////////////////////////////////////////




	short    rdOrig, grOrig, blOrig,     nuRed=0,nuGreen=0,nuBlue=0,     retHue, retSaturation, retValue;

	unsigned long   redTotal=0,   greenTotal = 0,   blueTotal=0,   pixelsWithColorCount=0;



    for(   long y=0;     y< m_height;     y++    )
     { 
		 for(   long  x=0;     x< m_width;      x++    )       
         {  

			sourceMap.Read_Pixel(      x, y,    &rdOrig,   &grOrig,   &blOrig  );  



			if(    parms.algoCode  ==   SeparationCreateParms::TARGEThUE   )
			{

				RGB_to_HSV(   rdOrig, grOrig, blOrig,       retHue,   retSaturation,   retValue    );    //  can return -1



				bool   isInRange =    false;

				if(    hasDisjointedSet    )       //  values are at the top of the unit circle, and split about the  0,360 degree radii
				{
					if(       retHue  >=   lowerHueLimit         // seems wacky, but must check in both hemispheres about the  Y-axis  ( the  0/180  line )   10/08
						||   retHue   <    upperHueLimit    )
							isInRange =  true;
				}
				else
				{  if(        retHue  >=    lowerHueLimit
						&&   retHue  <      upperHueLimit     )    //   <, not  <=   :     do not want simultaneous results
							isInRange =  true;
				}




				if(         isInRange   
					&&    retHue  >=  0    )  
				{

				//	short   modValue  =     retValue   *  2;      // ********  Maybe ATTENUATE,  ...but how ???  ****************       
					short   modValue  =    retValue;        


					if(     modValue   <   parms.valueLimit   )      // do NOT let values get DARKER than this value  ( big-value is a LIGHT color )
						modValue  =   parms.valueLimit;

					if(         modValue   >  255  )
						modValue =   255;
					else if(   modValue   <    0  )
						modValue =      0;



					short   modSaturation  =     retSaturation;  //    /2  ????     // ********   ADJUST   ...but how ???  ****************
	


					HSV_to_RGB(    retHue,  modSaturation,  modValue,       nuRed,  nuGreen,  nuBlue  );

					Write_Pixel(  x, y,     nuRed,  nuGreen,    nuBlue   ); 


					redTotal    +=    (unsigned long)nuRed;  
					greenTotal +=    (unsigned long)nuGreen;  
					blueTotal   +=    (unsigned long)nuBlue;  

					pixelsWithColorCount ++;
				}
			}

			else if(   parms.algoCode  ==   SeparationCreateParms::COMPONENTsUBTRACT   )
			{

				short  cTarg   =    topCompVal  -   rdOrig;
				short  mTarg  =    topCompVal  -   grOrig;
				short  yTarg   =    topCompVal  -   blOrig;

//		cHuntOrig   =    topCompVal  -   parms.huntRed;
				if(  rdOrig ==  255   )    cTarg =  0;    //  special case for 255, not on a numeric boundary

//		mHuntOrig  =    topCompVal  -   parms.huntGreen;
				if(  grOrig ==  255   )    mTarg =  0; 

//		yHuntOrig   =    topCompVal  -   parms.huntBlue;
				if(  blOrig ==  255   )    yTarg =  0; 





				if(         cTarg    >=     cHunt       //  is there enough Cyan Ink in source bmap such that the separations Cyan Ink can be SUBTRACTED OUT, and still have positive Cyan Ink
					&&    mTarg   >=    mHunt
					&&	yTarg    >=     yHunt    )
				{

					/***
					nuRed    =     topCompVal  -   cHunt;      //  must calc in case of redux
					nuGreen =     topCompVal  -   mHunt;
					nuBlue    =     topCompVal  -   yHunt;
					***/
					nuRed    =     topCompVal  -   cHuntOrig;      //  Use actual values, not the ones attunated by the Tolerance.   10/08
					nuGreen =     topCompVal  -   mHuntOrig;
					nuBlue    =     topCompVal  -   yHuntOrig;



					if(          nuRed  <     0  )
						nuRed =    0;
					else if(   nuRed  >=  256  )
						nuRed =   255;

					if(          nuGreen   <    0  )
						nuGreen =    0;
					else if(   nuGreen  >=  256  )
						nuGreen =   255;

					if(          nuBlue    <    0  )
						nuBlue =    0;
					else if(   nuBlue  >=  256  )
						nuBlue =   255;


					Write_Pixel(   x, y,     nuRed,  nuGreen,    nuBlue   );  
				}
			}

			else if(   parms.algoCode  ==   SeparationCreateParms::OPAQUEsEPARATION   )
			{

				ASSERT( 0 );    // *****  INSTALL   9/09  ******************************************
			}

			else
			{	ASSERT( 0 );   }

		 }  // for(   long  x=0;     
	}  // for(   long y=0; 




	if(        parms.algoCode  ==   SeparationCreateParms::TARGEThUE      //   ***********   what about   OPAQUEsEPARATION  ????????  *********************
		&&   pixelsWithColorCount >  0    )
	{
		retAvgRed     =   (short)(    redTotal      /  pixelsWithColorCount   ); 
		retAvgGreen  =   (short)(    greenTotal  /  pixelsWithColorCount   ); 
		retAvgBlue    =   (short)(     blueTotal    /  pixelsWithColorCount   ); 
	}


	return  true;
}





											////////////////////////////////////////


void    OffMap::Subtract_Pixels_Color(    short  redSrc,     short  greenSrc,     short blueSrc, 
								                           short  redSubtr,  short  greenSubtr,  short blueSubtr,       short&  nuRed,   short&  nuGreen,   short&  nuBlue    )
{

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



/***			 						
				short   cyanSource        =   255  -   redSrc;        //  Rough approximation for amount of Cyan ink that would be on the paper for that RGB value.
				short   magentaSource  =   255  -   greenSrc;    //   ...the   'TOP'[ 255 - srcRed ]   has the amount of component light(cyan) that will be filtered out. 
				short   yellowSource      =   255  -   blueSrc;    

				short   cyanSep        =   255  -   redSubtr;
				short   magentaSep  =   255  -   greenSubtr;
				short   yellowSep      =   255  -   blueSubtr;



				short   cyanDiff        =    cyanSource        -   cyanSep;         //  Calc the amount of Cyan Ink that will not be necessary because the separation will supply some Cyan Ink( cyanSep )
				short   magentaDiff  =    magentaSource  -   magentaSep;
				short   yellowDiff     =    yellowSource      -   yellowSep;


				if(  cyanDiff  > 255 )	    
					cyanDiff =   255;
				else if(  cyanDiff  <  0   )
					cyanDiff =   0;

				if(  magentaDiff  > 255 )	    
					magentaDiff =   255;
				else if(  magentaDiff  <  0   )
					magentaDiff =   0;

				if(  yellowDiff  > 255 )	    
					yellowDiff =   255;
				else if(  yellowDiff  <  0   )
					yellowDiff =   0;



				nuRed    =    255   -   cyanDiff;
				nuGreen =    255   -   magentaDiff;
				nuBlue    =    255   -   yellowDiff;
***/



				//  use substitution on above:  				short   cyanDiff        =     -redSrc   +  redSubtr            //  ( 256  -   redSrc )        -    ( 256  -   redSubtr );  
				//
				//   *****   Also see  Blit_Offmap_2PCwindow_Summed_RGB_Values()  for the same  ALGEBRA for color subtraction   *******

				if(  redSubtr ==  255   )       redSubtr =  256;    //  need this to keep on colorBit Boundaries ( posterization in Oil  EX: 3bit color should be on multiples of 32 [ 2 ** (8 - 3 ) = 32
				if(  redSrc ==  255   )       redSrc =  256; 

				if(  greenSubtr ==  255   )       greenSubtr =  256; 
				if(  greenSrc  ==  255   )       greenSrc    =  256; 

				if(  blueSubtr ==  255   )       blueSubtr =  256; 
				if(  blueSrc ==  255   )       blueSrc =  256; 



				nuRed      =    256   -   (  redSubtr      -  redSrc  );      

				nuGreen  =     256   -   (  greenSubtr  -  greenSrc  );      

				nuBlue     =     256  -   (  blueSubtr     -  blueSrc  );  



				if(  nuRed  > 255 )	    
					nuRed =   255;
				else if(  nuRed  <  0   )
					nuRed =   0;

				if(  nuGreen  > 255  )	
					nuGreen =   255;
				else if(  nuGreen  <  0   )
					nuGreen =   0;

				if(  nuBlue   > 255   )	    
					nuBlue =   255;
				else if(  nuBlue  <  0   )
					nuBlue =   0;
}



											////////////////////////////////////////


bool   OffMap::Subtract_Separation(   OffMap&  separationBMap,    CString&   retErrorMesg   )
{


	retErrorMesg.Empty();


	if(        m_width    !=   separationBMap.m_width
		||    m_height   !=   separationBMap.m_height
		||    m_depth    !=   separationBMap.m_depth      )
	{
		retErrorMesg =    "OffMap::Subtract_Separation  failed,  bitmaps are NOT the same size" ;
		return  true;
	}



	short    redBigMap,  greenBigMap,  blueBigMap,      redSep,  greenSep,  blueSep,      nuRed, nuGreen, nuBlue; 



	for(   long  y=0;     y< m_height;     y++   )    // Traverse the DESTmap's  coords
	{
		for(   long  x=0;     x< m_width;     x++   )
		{

			separationBMap.Read_Pixel(    x, y,        &redSep,  &greenSep,  &blueSep    );     

			Read_Pixel(                            x, y,        &redBigMap,   &greenBigMap,   &blueBigMap    );     



			if(         !  ( redSep == 255    &&   greenSep == 255    &&    blueSep == 255 ) 

		//		&&    !  ( redBigMap == 255     &&   greenBigMap == 255     &&    blueBigMap == 255 ) 		..******Does this make sense TOO...  I'm not sure.  8/18/08 ********				
	  		  )                                     //  only process if the  Separation and the SOURCE pixels   are NOT white [ has some filtered outvalue ]
			{

				Subtract_Pixels_Color(   redBigMap,  greenBigMap,  blueBigMap,      redSep,  greenSep,  blueSep,       nuRed, nuGreen, nuBlue   );


				Write_Pixel(    x, y,      nuRed,  nuGreen,  nuBlue   );     

			}   //    if(         !  ( redSep == 255    &&   greenSep == 255    &&    blueSep == 255 ) 

		}
	}   //    for(   long  y=0;   

	return  true;
}




											////////////////////////////////////////


void    OffMap::Subtract_RegionSubjects_Offmap(   OffMap&  offMapRegion,       RectShrt&  rectRegion,    RectShrt&  rectBigMap,
																			                                                       short  hueOffset,  short  saturationOffset,  short valueOffset 	)
{

					//   Similar to  Blit_Offmap_2PCwindow_Summed_RGB_Values()  but strictly for offmaps



														// ******* WANT  +1  ( see below ???? *********************   JPM

	int widthBigMap  =   ( rectBigMap.right      -    rectBigMap.left );    //   cause bug   +1;     // ******* WANT  +1  ?? [ +1 :  Inclusive Counting ]       // width of destination rectangle	 		  
	int heightBigMap =   ( rectBigMap.bottom   -   rectBigMap.top );  //    +1;              // height of destination rectangle


	int   widthRegion   =    ( rectRegion.right      -  rectRegion.left  );   //  +1;         // width of source  rectangle [ +1 :  Inclusive Counting ]    
	int   heightRegion  =    ( rectRegion.bottom  -  rectRegion.top   );  //   +1;    	




	                 //  Now traverse the DEST-pixels and subtract out the ColorComponent(RGB) light that the SRCregion's pixels would subtract.     8/08

	int       x=0,   y=0;
	int       xBigMap,  yBigMap;
	int       xRegion,   yRegion;

	short    redBigMap,  greenBigMap,  blueBigMap,      redSep,  greenSep,  blueSep,      nuRed, nuGreen, nuBlue; 




	for(   y=0;     y< heightBigMap;     y++   )    // Traverse the     DESTmap's  coords   ( Big map in RendMan )
	{
		for(   x=0;     x< widthBigMap;     x++   )
		{
				
			xBigMap  =    rectBigMap.left  +  x;       //  get DEST pixel 
			yBigMap  =    rectBigMap.top  +  y; 


			xRegion =    ( x  *  widthRegion )    /  widthBigMap;     //  SCALE back to get the   Region's SRCmap coords
			yRegion =    ( y  *  heightRegion )   /  heightBigMap;




			offMapRegion.Read_Pixel_ColorModified(   xRegion,  yRegion,     hueOffset, saturationOffset, valueOffset,     redSep,  greenSep,  blueSep  );


			if(    !  ( redSep == 255    &&   greenSep == 255    &&    blueSep == 255 )   )       // Only if   SRC-pixel( RegionSubject )  is NOT white, do we mix it with the DSTpixel's value
			{

				Read_Pixel(    xBigMap,  yBigMap,      &redBigMap,   &greenBigMap,   &blueBigMap   );  


				/****
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

				short  nuRed     =     (short)dstRed       -    ( 255 -  (short)srcRed );    //  The   'TOP'[255 - srcRed]     has the amount of component light that will be filtered out by SRCpixel
				short  nuGreen  =     (short)dstGreen   -    ( 255 -  (short)srcGreen);
				short  nuBlue     =     (short)dstBlue     -    ( 255 -  (short)srcBlue);

				if(  nuRed  <  0   )	    nuRed =   0;
				if(  nuGreen  < 0   )	nuGreen =   0;
				if(  nuBlue  <  0   )	    nuBlue =   0;
				****/
				Subtract_Pixels_Color(   redBigMap,  greenBigMap,  blueBigMap,      redSep,  greenSep,  blueSep,       nuRed, nuGreen, nuBlue   );


				Write_Pixel(  xBigMap,  yBigMap,    nuRed,  nuGreen,  nuBlue  );  
			}   			
		}
	}
}




											////////////////////////////////////////
											////////    Flood Fill Ops  ///////////
											////////////////////////////////////////
											////////////////////////////////////////


bool    OffMap::Color_Is_Almost_White(   short  x,   short y,    short hiCompValue   )
{

	bool   isWhite =  true;

//	short  hiCompValue  =   250;   


	short   red,  green,  blue;


	Read_Pixel(   x, y,    &red,  &green,   &blue  );


	if(			  red       <   hiCompValue
			||    green   <   hiCompValue
			||    blue     <   hiCompValue    )
		 isWhite =    false;



	if(   m_seedFillModeColor  )
		return   isWhite;
	else
		return  ! isWhite;   
}



											////////////////////////////////////////


bool    OffMap::Color_Is_Fillable(   short  x,   short y,      short  redTarg,  short greenTarg,  short blueTarg    )
{
	
									//	**********  Want a tolerance for CLOSENESS to the component ???   10/08  ***************

	bool   isFillable =  false;


	short   red,  green,  blue;

	Read_Pixel(   x, y,    &red,  &green,   &blue  );


	if(			    red       ==    redTarg
			&&    green   ==    greenTarg
			&&    blue     ==    blueTarg     )
		 isFillable =    true;


	return  isFillable;  
}



											////////////////////////////////////////


bool    OffMap::MaskPix_Is_Written(   short  x,   short y,    short  masksWriteVal   )
{

	bool   isWrittenTo =  false;


	short   red,  green,  blue;

	Read_Pixel(   x, y,    &red,  &green,   &blue  );


	if(	   red  ==   masksWriteVal   )      //  mask are always 1-bit, so just need to examine red component
		isWrittenTo =    true;


	return   isWrittenTo;
}



											////////////////////////////////////////




#define MAXDEPTH 90000       //    10000 Biggest I've seen is only about  2000              10/2008


#define POPoffmap( XL, XR, Y, DY ) \
    { --sp; XL = sp->xl; XR = sp->xr; Y = sp->y+(DY = sp->dy); }


#define PUSHoffmap( XL, XR, Y, DY ) \
    if( sp < stack+MAXDEPTH && Y+(DY) >= 0 && Y+(DY) <= maxYdim ) \
    { sp->xl = XL; sp->xr = XR; sp->y = Y; sp->dy = DY; ++sp; }




																		////////////////////////////////////


bool     OffMap::Seed_Fill_With_White(    int x, int y,     short  redNu,  short greenNu,  short blueNu,     CString&  retErrorMesg    )
{	

			//  This is more a test routine to refine this code


	 short hiCompValue   =      Component_Almost_White();   // ************  ADJUST *******************



	long   pixelsChangedCnt =  0;
    int                    left, x1, x2, dy;

    LINESEGMENToffmap   stack[ MAXDEPTH ],   *sp = stack;


	/***
	DWORD  redNu    =    GetRValue(    new_color   ); 
	DWORD  greenNu =   GetGValue(    new_color   );
	DWORD  blueNu   =    GetBValue(    new_color   );
	***/

	int   nMinX = 0;

	int   maxXdim  =    m_width     -1;
	int   maxYdim  =    m_height    -1;     //   max dimensions  


	retErrorMesg.Empty();



//    Read_Pixel(   x, y,   &rd,  &gr, &bl  );    //  old_color =   GetPixel( hDC, x, y );      

	if(   Color_Is_Almost_White(  x,y,   hiCompValue )     )            // if( old_color  == new_color  )
	{
    //  short   rd,   gr,   bl; 
	//	Read_Pixel(   x, y,   &rd,  &gr, &bl  ); 
	//	TRACE(   "Hit  pixel [ %d, %d ]  was white  [ %d,  %d,  %d ] \n\n",    x, y,     rd, gr, bl    );
		return  true;
	}



    if( x < nMinX || x > maxXdim || y < nMinX || y > maxYdim )
        return  true;



    PUSHoffmap( x, x,   y,      1  );         //  needed in some cases
    PUSHoffmap( x, x,   y+1, -1  );    //  seed segment (popped 1st)  



    while(  sp >  stack  ) 
	{


        POPoffmap(   x1, x2,   y,  dy  );


 //     for(    x = x1;     x >= nMinX        &&     GetPixel(x, y) == old_color; --x )
		for(    x = x1;      x >= nMinX      &&    ! Color_Is_Almost_White( x,y,  hiCompValue );      x--   )
		{
			Write_Pixel(   x, y,    redNu,  greenNu,  blueNu  );    //  SetPixel(  hDC,  x, y,  new_color   );  
			pixelsChangedCnt++;
		}


        if(  x  >=  x1  )
            goto SKIP;


        left =   x +1;


        if(  left  <  x1  )
            PUSHoffmap(  left,   x1-1,   y,  -dy  );     //    PUSHoffmap(y, left, x1-1, -dy);    // leak on left?       // *************** Looks wrong


        x =   x1  +1;



        do 
		{
//          for(      ;     x<= maxXdim     && GetPixel(x, y) == old_color;            ++x )
			for(      ;    x <=  maxXdim        &&     ! Color_Is_Almost_White( x,y,  hiCompValue );    x++      )
			{
				Write_Pixel(   x, y,    redNu,  greenNu,  blueNu  );    //   SetPixel(  hDC,  x, y,  new_color   );  
				pixelsChangedCnt++;
			}


            PUSHoffmap(  left,   x-1,   y,   dy  );



            if( x  >  x2+1  )
                PUSHoffmap( x2+1,  x-1,  y,  -dy);    // leak on right?




//SKIP:  for(    ++x;     x <= x2    &&    GetPixel(x, y) != old_color;    ++x   )     {;}
SKIP:    for(    x++;     x <= x2    &&    Color_Is_Almost_White( x,y,  hiCompValue );        x++ )           //  Keep advancing x to the right  UNTIL find a pixel that is 'NOT white'
			{    
				;
			}


            left =   x;


        } while(  x  <=  x2  );
    }


	return  true;
}




																		////////////////////////////////////


bool     OffMap::SeedFill_MaskMap_Create_from_NonWhite_Pixels(    int x, int y,        bool writeToSRCmapPixels,         short  redNu,  short greenNu,  short blueNu,     
											                                               short hiCompValue,     OffMap&  maskMap,    OffMap  *totalMaskMap,        RectShrt&   retBoundBox,   long&  retPixelsChangedCnt,   CString&  retErrorMesg    )
{	

		//  Can be used to just BUILD a MaskMap from a solid region on a colorMap,  with or without writing any pixels to the 

		//  This verion is much faster than the one with
		
	 //   short hiCompValue   =   250;  

	
	retBoundBox.left      =   -1;      //  Init for returning TRUE because we  x,y  is a white pixel.  CALLING FUNCTION need sto test for this
	retBoundBox.top      =   -1;
	retBoundBox.right    =    -1;
	retBoundBox.bottom =    -1;

	retPixelsChangedCnt =   0;


    int      left, x1, x2, dy;

    LINESEGMENToffmap   stack[ MAXDEPTH ],   *sp = stack;
	long   curStackHeight=0,  biggestStackHeight=0;

	short  masksWriteVal =   0;
	short   xMin = 30000,  yMin =30000,   xMax= -1,  yMax= -1;
	int   maxXdim  =    m_width     -1;
	int   maxYdim  =    m_height    -1;     //   max dimensions  





	retErrorMesg.Empty();

	retPixelsChangedCnt =   0;


	if(   Color_Is_Almost_White(  x,y,   hiCompValue )     )            // if( old_color  == new_color  )
		return  true;
	

    if( x < 0 || x > maxXdim || y < 0 || y > maxYdim )
        return  true;


    PUSHoffmap( x, x,   y,      1  );      curStackHeight++;         //  needed in some cases
    PUSHoffmap( x, x,   y+1, -1  );      curStackHeight++;      //  seed segment (popped 1st)  



    while(  sp >  stack  ) 
	{

        POPoffmap(   x1, x2,   y,  dy  );      curStackHeight--;  
										
		for(    x = x1;      
					x >= 0    &&    ! Color_Is_Almost_White( x,y,  hiCompValue )   &&    ! maskMap.MaskPix_Is_Written(  x,y,  masksWriteVal  );      
															x--   )
		{
			if(   writeToSRCmapPixels  )
				Write_Pixel(   x, y,    redNu,  greenNu,  blueNu  );    //  SetPixel(  hDC,  x, y,  new_color   );  

			maskMap.Write_Pixel(   x, y,    0, 0, 0  );  

			if(    totalMaskMap  !=  NULL    )      
				totalMaskMap->Write_Pixel(   x, y,    0, 0, 0  );  

			if(  x >  xMax  )     xMax =    x;
			if(  y >  yMax  )     yMax =    y;
			if(  x <   xMin  )     xMin  =    x; 
			if(  y <   yMin  )     yMin  =    y; 

			retPixelsChangedCnt++;
		}


        if(  x  >=  x1  )
            goto SKIP;


        left =   x +1;

        if(  left  <  x1  )
		{
			PUSHoffmap(  left,   x1-1,   y,  -dy  );     curStackHeight++;       // leak on left?    

			if(   curStackHeight  >  biggestStackHeight    )
			{
				biggestStackHeight  =    curStackHeight;
				if(    curStackHeight  >=   ( MAXDEPTH - 4)     )
				{	ASSERT( 0 );   }
			}
		}


        x =   x1  +1;

        do 
		{
			for(      ;    
						x <=  maxXdim    &&     ! Color_Is_Almost_White( x,y,  hiCompValue )    &&    ! maskMap.MaskPix_Is_Written(  x,y,  masksWriteVal  );    
																																x++      )
			{  	if(   writeToSRCmapPixels  )
					Write_Pixel(   x, y,    redNu,  greenNu,  blueNu  );    


				maskMap.Write_Pixel(   x, y,    0, 0, 0  );  

				if(    totalMaskMap  !=  NULL    )      
					totalMaskMap->Write_Pixel(   x, y,    0, 0, 0  );  


				if(  x >  xMax  )     xMax =    x;
				if(  y >  yMax  )     yMax =    y;
				if(  x <   xMin  )     xMin  =    x; 
				if(  y <   yMin  )     yMin  =    y; 

				retPixelsChangedCnt++;
			}



            PUSHoffmap(  left,   x-1,   y,   dy  );     curStackHeight++;  

			if(   curStackHeight  >  biggestStackHeight    )
			{
				biggestStackHeight  =    curStackHeight;
				if(    curStackHeight  >=   ( MAXDEPTH - 4)     )
				{	ASSERT( 0 );   }
			}


            if( x  >  x2+1  )
			{
				PUSHoffmap( x2+1,  x-1,  y,  -dy);        curStackHeight++;      // leak on right?

				if(   curStackHeight  >  biggestStackHeight    )
				{
					biggestStackHeight  =    curStackHeight;
					if(    curStackHeight  >=   ( MAXDEPTH - 4)     )
					{	ASSERT( 0 );   }
				}
			}


SKIP:    for(    x++;     
								x <= x2    &&    Color_Is_Almost_White( x,y,  hiCompValue );        // ******  NEED  MaskPix_Is_Written() in this somhow ???  *********************
																		x++ )           //  Keep advancing x to the right  UNTIL find a pixel that is 'NOT white'
			{   ;  }  

            left =   x;

        } while(  x  <=  x2  );
    }


	retBoundBox.left      =    xMin;      //  return the bound box for the fill
	retBoundBox.top      =    yMin;

	retBoundBox.right    =    xMax;
	retBoundBox.bottom =    yMax;

	return  true;
}




																		////////////////////////////////////


OffMap*     OffMap::SeedFill_24ColorMap_Create_from_Colored_Pixels(    int x, int y,       short  redTarg,  short greenTarg,  short blueTarg,     
											                                                               OffMap&  totalMaskMap,     long  minimumPixelsInRegion,  long   minimumMapDimension,
																										   RectShrt&   retBoundBox,    long&  retPixelsChangedCnt,    CString&  retErrorMesg    )
{	

				//	 NEW function.  It could replace some in  RegionSubject      3/10  



	if(   x < 0    ||    y < 0   )    //  should these be in the LARGE map's display coords ...  the World coords, ause we work on the BIG map
	{
		retErrorMesg =   "OffMap::SeedFill_24ColorMap_Create_from_Colored_Pixels failed, x or y coord is off the bitmap." ;
		return  NULL;
	}

																			//   1)   Allocate a mask to hold the FloodedRegion's pixel positions  

	OffMap  *maskMap       =      new   OffMap(   m_width,   m_height,    1   );
	if(           maskMap ==  NULL  )
	{
		retErrorMesg  =    "OffMap::SeedFill_24ColorMap_Create_from_Colored_Pixels  failed,  could not alloc maskMap." ;
		return  NULL;
	}

	maskMap->Clear(  255  );      //  should be white
	


																				//   2)   Run the Flood fill to find the FloodedRegion's pixels and boundBox 

	bool     writeToSRCmapPixels =  false;   


	if(   !  SeedFill_MaskMap_Create_from_Colored_Pixels( x, y,    redTarg,  greenTarg,  blueTarg,     *maskMap,   &totalMaskMap,    retBoundBox,   retPixelsChangedCnt,   retErrorMesg  )   )
	{
		if( maskMap !=  NULL  )      delete  maskMap;   
		return  NULL;
	}




	if(      retBoundBox.left   < 0   
		||  retPixelsChangedCnt  <  minimumPixelsInRegion   )
	{
		if( maskMap !=  NULL  )      
			delete  maskMap;   		

		return  NULL;			// ******  NO Error,  user clicked on a White pixel or a TOO-SMALL region.   CALLING FUNCTION should test for empty  'retErrorMesg'  *************
	}




																				//   3)   Create an new Offmap as big as the Flooded-Regions boundBox for the new RegionSubject

	short   nuMapsWidth  =       ( retBoundBox.right      -   retBoundBox.left )     +1;      //   +1:  inclusive counting
	short   nuMapsHeight =       ( retBoundBox.bottom  -   retBoundBox.top )      +1;  




	if(       nuMapsWidth  <  minimumMapDimension                //  buggy if map is only 1 pixel high  
		||    nuMapsHeight  <  minimumMapDimension    )
	{
		if( maskMap !=  NULL  )      
			delete  maskMap;   

		return  NULL;			// ******  NO Error,  user clicked on a White pixel or a TOO-SMALL region.   CALLING FUNCTION should test for empty  'retErrorMesg'  *************
	}




	OffMap  *nuOffMap  =  	new   OffMap(   nuMapsWidth,  nuMapsHeight,     m_depth   );   
	if(          nuOffMap ==  NULL  )
	{
		retErrorMesg  =   "OffMap::SeedFill_24ColorMap_Create_from_Colored_Pixels  failed,  could not  alloc Offmap."  ;
		if( maskMap !=  NULL  )      delete  maskMap;   
		return  NULL;
	}

	nuOffMap->Clear(  255  );	





																				//	4)	  COPY the flooded region's COLOR-pixel info to the new bitmap


	unsigned long    masksPixelCount =  0,    xCentroidTotal =0,   yCentroidTotal =0;
	short                rd,gr,bl,  rdMsk,grMsk,blMsk,   xCentroid= -1,  yCentroid=-1 ;



	for(   short  yTrav = 0;      yTrav < nuMapsHeight;      yTrav++   )
	{  for(   short  xTrav = 0;      xTrav < nuMapsWidth;      xTrav++   )
		{

			maskMap->Read_Pixel(     xTrav +  retBoundBox.left,    yTrav +  retBoundBox.top,      &rdMsk,  &grMsk, &blMsk   ); 

			if(   rdMsk  ==  0   )     //  a mask-pixel has been set
			{

				Read_Pixel(     xTrav +  retBoundBox.left,    yTrav +  retBoundBox.top,       &rd,  &gr,  &bl   ); 

				nuOffMap->Write_Pixel(      xTrav,                                 yTrav,               rd,  gr,  bl  );  

				masksPixelCount++;
			}
		}
	}



	if(   maskMap  !=  NULL   )
		delete  maskMap;


	return  nuOffMap;
}




																		////////////////////////////////////


bool     OffMap::SeedFill_MaskMap_Create_from_Colored_Pixels(    int x, int y,      short  redTarg,  short greenTarg,  short blueTarg,     OffMap&  maskMap,    
											                                                     OffMap  *totalMaskMap,     RectShrt&   retBoundBox,    long&  retPixelsChangedCnt,   CString&  retErrorMesg    )
{	

		//  Can be used to just BUILD a MaskMap from a SOLID REGION on a colorMap,  without writing any pixels.  Is used on RendMan's SRCmap and SubtractedMap

		//  This verion is much faster than the one with

	
	retBoundBox.left      =   -1;      //  Init for returning TRUE because we  x,y  is a white pixel.  CALLING FUNCTION need sto test for this
	retBoundBox.top      =   -1;
	retBoundBox.right    =    -1;
	retBoundBox.bottom =    -1;

	retPixelsChangedCnt =   0;


    int      left, x1, x2, dy;

    LINESEGMENToffmap   stack[ MAXDEPTH ],   *sp = stack;
	long   curStackHeight=0,  biggestStackHeight=0;

	short  masksWriteVal =   0;
	short   xMin = 30000,  yMin =30000,   xMax= -1,  yMax= -1;
	int   maxXdim  =    m_width     -1;
	int   maxYdim  =    m_height    -1;     //   max dimensions  





	retErrorMesg.Empty();

	retPixelsChangedCnt =   0;


	if(   ! Color_Is_Fillable(  x,y,   redTarg, greenTarg, blueTarg  )     )            // if( old_color  == new_color  )
		return  true;
	

    if( x < 0 || x > maxXdim || y < 0 || y > maxYdim )
        return  true;


    PUSHoffmap( x, x,   y,      1  );      curStackHeight++;         //  needed in some cases
    PUSHoffmap( x, x,   y+1, -1  );      curStackHeight++;      //  seed segment (popped 1st)  



    while(  sp >  stack  ) 
	{

        POPoffmap(   x1, x2,   y,  dy  );      curStackHeight--;  
										
		for(    x = x1;      
					x >= 0    &&    Color_Is_Fillable( x,y,   redTarg, greenTarg, blueTarg )   &&    ! maskMap.MaskPix_Is_Written(  x,y,  masksWriteVal  );      
															x--   )
		{
			maskMap.Write_Pixel(   x, y,    0, 0, 0  );  

			if(    totalMaskMap  !=  NULL    )      
				totalMaskMap->Write_Pixel(   x, y,    0, 0, 0  );  

			if(  x >  xMax  )     xMax =    x;
			if(  y >  yMax  )     yMax =    y;
			if(  x <   xMin  )     xMin  =    x; 
			if(  y <   yMin  )     yMin  =    y; 

			retPixelsChangedCnt++;
		}


        if(  x  >=  x1  )
            goto SKIP;


        left =   x +1;

        if(  left  <  x1  )
		{
			PUSHoffmap(  left,   x1-1,   y,  -dy  );     curStackHeight++;       // leak on left?    

			if(   curStackHeight  >  biggestStackHeight    )
			{
				biggestStackHeight  =    curStackHeight;
				if(    curStackHeight  >=   ( MAXDEPTH - 4)     )
				{	ASSERT( 0 );   }
			}
		}


        x =   x1  +1;

        do 
		{
			for(      ;    
						x <=  maxXdim    &&     Color_Is_Fillable( x,y,   redTarg, greenTarg, blueTarg )    &&    ! maskMap.MaskPix_Is_Written(  x,y,  masksWriteVal  );    
																																x++      )
			{  
				maskMap.Write_Pixel(   x, y,    0, 0, 0  );  

				if(    totalMaskMap  !=  NULL    )      
					totalMaskMap->Write_Pixel(   x, y,    0, 0, 0  );  


				if(  x >  xMax  )     xMax =    x;
				if(  y >  yMax  )     yMax =    y;
				if(  x <   xMin  )     xMin  =    x; 
				if(  y <   yMin  )     yMin  =    y; 

				retPixelsChangedCnt++;
			}



            PUSHoffmap(  left,   x-1,   y,   dy  );     curStackHeight++;  

			if(   curStackHeight  >  biggestStackHeight    )
			{
				biggestStackHeight  =    curStackHeight;
				if(    curStackHeight  >=   ( MAXDEPTH - 4)     )
				{	ASSERT( 0 );   }
			}


            if( x  >  x2+1  )
			{
				PUSHoffmap( x2+1,  x-1,  y,  -dy);        curStackHeight++;      // leak on right?

				if(   curStackHeight  >  biggestStackHeight    )
				{
					biggestStackHeight  =    curStackHeight;
					if(    curStackHeight  >=   ( MAXDEPTH - 4)     )
					{	ASSERT( 0 );   }
				}
			}


SKIP:    for(    x++;     
								x <= x2    &&    ! Color_Is_Fillable( x,y,   redTarg, greenTarg, blueTarg );        // ******  NEED  MaskPix_Is_Written() in this somhow ???  *********************
																		x++ )           //  Keep advancing x to the right  UNTIL find a pixel that is 'NOT white'
			{   ;  }  

            left =   x;

        } while(  x  <=  x2  );
    }


	retBoundBox.left      =    xMin;      //  return the bound box for the fill
	retBoundBox.top      =    yMin;

	retBoundBox.right    =    xMax;
	retBoundBox.bottom =    yMax;

	return  true;
}



																		////////////////////////////////////
																		////////////////////////////////////
																		////////////////////////////////////


void     OffMap::Push_To_LineSegment_Stack(   short  xl,    short xr,   short y,    short dy,    ListMemry< LINESEGMENToffmap >&   pixelStack )
{	

	/***
	#define PUSHoffmap( XL, XR, Y, DY ) \

    if(  sp < stack+MAXDEPTH   &&   Y+(DY) >= 0    &&    Y+(DY) <= maxYdim )    \        ******************  NEED test
    { 
		sp->xl = XL; 
		sp->xr = XR; 
		sp->y = Y; 
		sp->dy = DY; 
		++sp; 
	}
	****/

	if(     (     (y + dy)   <  0   )
		|| (     (y + dy)   >=   m_height    )      )
	{
		return;   //  off the map...  this happens
	}


	LINESEGMENToffmap  *nuLineSeg1        =        new   LINESEGMENToffmap();
	if(                               nuLineSeg1 ==  NULL  )
	{  ASSERT( 0 );    
		return;   
	}

		nuLineSeg1->xl   =    xl;
		nuLineSeg1->xr   =    xr;	
		nuLineSeg1->y   =      y;
		nuLineSeg1->dy   =    dy;

	pixelStack.Push(   *nuLineSeg1   ); 
}



																		////////////////////////////////////


void     OffMap::Pop_From_LineSegment_Stack(   short&  xl,    short&  xr,   short&  y,    short&  dy,    ListMemry< LINESEGMENToffmap >&   pixelStack )
{	
	/***
		#define POPoffmap( XL, XR, Y, DY ) \
    {    --sp; 
	       XL = sp->xl; 
		   XR = sp->xr; 
	       Y =   sp->y  +  (DY = sp->dy); 
	}
	****/
	LINESEGMENToffmap&   lineSeg  =   pixelStack.Pop();    // *** NOTE, this is a reference...  not a local object.

		xl   =    lineSeg.xl;
		xr   =   lineSeg.xr; 
		dy  =    lineSeg.dy; 
		y    =    lineSeg.y    +   lineSeg.dy;


	delete  &lineSeg;   //  YES, need to delete the Dynamically allocated  item this LINESEGMENToffmap way???   10/08
}




																		////////////////////////////////////


bool     OffMap::SeedFill_MaskMap_Create_from_NonWhite_Pixels_SLOW(    short x, short y,        bool writeToSRCmapPixels,         short  redNu,  short greenNu,  short blueNu,     
											                                                        short hiCompValue,     OffMap&  maskMap,      RectShrt&   retBoundBox,   CString&  retErrorMesg    )
{	

	ASSERT( 0 );   //  Dont use it,  too slow.

			// **** TOO SLOW!!!! *********       This VERSION uses a much more reliable stack, but it is a VERY slow caouse it dynamically allocateds   10/08


		//  Can be used to just BUILD a MaskMap from a solid region on a colorMap,  with or without writing any pixels to the 



		
	 //   short hiCompValue   =   250;  ...typical

	
	retBoundBox.left      =   -1;      //  Init for returning TRUE because we  x,y  is a white pixel.  CALLING FUNCTION need sto test for this
	retBoundBox.top      =   -1;
	retBoundBox.right    =    -1;
	retBoundBox.bottom =    -1;



	long   pixelsChangedCnt =  0;
    short  left, x1, x2, dy;



 //   LINESEGMENToffmap   stack[ MAXDEPTH ],   *sp = stack;     ****BAD, could have undetected stach overflow.  
	ListMemry< LINESEGMENToffmap >  pixelStack;
	pixelStack.Set_Dynamic_Flag( true );



	short  masksWriteVal =   0;


	short   xMin = 30000,  yMin =30000,   xMax= -1,  yMax= -1;

	short   maxXdim  =    m_width     -1;
	short   maxYdim  =    m_height    -1;     //   max dimensions  

	retErrorMesg.Empty();




	if(   Color_Is_Almost_White(  x,y,   hiCompValue )     )            //   if(   old_color  ==  new_color   )
 		return  true;
	



    if( x < 0 || x > maxXdim || y < 0 || y > maxYdim )
        return  true;



	/***
    PUSHoffmap( x, x,   y,      1  );         //  needed in some cases
    PUSHoffmap( x, x,   y+1, -1  );    //  seed segment (popped 1st)  
	***/
	Push_To_LineSegment_Stack(   x, x,        y,    1,       pixelStack  );
	Push_To_LineSegment_Stack(   x, x,    y+1,   -1,       pixelStack  );




    while(    ! pixelStack.Is_Empty()    )   //  sp >  stack  ) 
	{


   //  POPoffmap(           x1, x2,   y,  dy      );
		Pop_From_LineSegment_Stack(  x1, x2,   y,  dy,    pixelStack );

										
		for(    x = x1;      
					x >= 0    &&    ! Color_Is_Almost_White( x,y,  hiCompValue )   &&    ! maskMap.MaskPix_Is_Written(  x,y,  masksWriteVal  );      
															x--   )
		{
			if(   writeToSRCmapPixels  )
				Write_Pixel(   x, y,    redNu,  greenNu,  blueNu  );  

			maskMap.Write_Pixel(   x, y,    0, 0, 0  );  

			if(  x >  xMax  )     xMax =    x;
			if(  y >  yMax  )     yMax =    y;
			if(  x <   xMin  )     xMin  =    x; 
			if(  y <   yMin  )     yMin  =    y; 

			pixelsChangedCnt++;
		}


        if(  x  >=  x1  )
		{
			//   ASSERT( 0 );       Yes, can this can happen    10/21/08
			goto SKIP;
		}




        left =   x + 1;       //  left is the last PixelWrite to the left??


        if(   left  <  x1   )
      //   PUSHoffmap(                            left,    x1 -1,    y,  -dy   );     
			Push_To_LineSegment_Stack(  left,     x1 -1,    y,  -dy,      pixelStack  );   //   leak on left?    



        x =   x1 + 1;   //  now going to fill the RIGHT side of line y

        do 
		{
			for(      ;    
						x <=  maxXdim    &&     ! Color_Is_Almost_White( x,y,  hiCompValue )    &&    ! maskMap.MaskPix_Is_Written(  x,y,  masksWriteVal  );    
																																x++      )
			{  
				if(   writeToSRCmapPixels  )
					Write_Pixel(   x, y,    redNu,  greenNu,  blueNu  );    


				maskMap.Write_Pixel(   x, y,    0, 0, 0  );  

				if(  x >  xMax  )     xMax =    x;
				if(  y >  yMax  )     yMax =    y;
				if(  x <   xMin  )     xMin  =    x; 
				if(  y <   yMin  )     yMin  =    y; 

				pixelsChangedCnt++;
			}


       //  PUSHoffmap(                             left,   x-1,   y,   dy      );
			Push_To_LineSegment_Stack(    left,   x-1,   y,   dy,       pixelStack  );



            if(   x   >   x2 +1   )
   //           PUSHoffmap(                            x2+1,  x-1,  y,  -dy    );   
				Push_To_LineSegment_Stack(    x2+1,  x-1,  y,  -dy,       pixelStack  );        // leak on right?





SKIP:    for(    x++;     
								x <= x2    &&    Color_Is_Almost_White( x,y,  hiCompValue );        // ******  NEED  MaskPix_Is_Written() in this somhow ???  *********************
																		x++ )           //  Keep advancing x to the right  UNTIL find a pixel that is 'NOT white'
			{   ;  }  


            left =   x;


        } while(  x  <=  x2  );
    }



	retBoundBox.left      =    xMin;      //  return the bound box for the fill
	retBoundBox.top      =    yMin;

	retBoundBox.right    =    xMax;
	retBoundBox.bottom =    yMax;


	return  true;
}



																		////////////////////////////////////


void   OffMap::Mask_Blits_Color(   OffMap& mskMap,   OffMap *tMskMap,    RectShrt  *bBox,    short rd, short gr, short bl   )
{ 

				//   OK for   tMskMap to be NULL,      OK for    bBox   equal NULL

	
		//  FROM:    Mask_Blit_Color(    MealMAP *mskMap,    MealMAP *tMskMap,   MealMAP *dstMap,    Rect *bBox,   short rd, short gr, short bl  ) 



    short     h,v,   Xmax,Ymax, Xmin,Ymin,   MKrd, MKgr, MKbl;  


	if(  bBox == NULL  )  
	{ 
		Xmin= 0;    
		Xmax=   mskMap.m_width  -1;   // -1, INCLUSUVE to rect-Borders
                                
		Ymin= 0;    
		Ymax=    mskMap.m_height -1;                              
	}					
	else                
	{   Xmin  =   bBox->left;    
		Xmax =   bBox->right; 
        Ymin  =    bBox->top;     
		Ymax =   bBox->bottom;  
    }


       
	for(  v= Ymin;   v <= Ymax;   v++  )		// '<='  cause INCLUSUVE to rect-Borders
	{ 
		for(  h= Xmin;   h <= Xmax;   h++  )
		{

			mskMap.Read_Pixel(   h, v,     &MKrd, &MKgr, &MKbl  ); 

			if(    MKrd == 0   )		   // Mask pix is black
			{ 
				Write_Pixel(  h,v,    rd,gr,bl  ); 

				if(    tMskMap  !=  NULL    )   
					tMskMap->Write_Pixel(    h,v,   0,0,0   ); 
  			}	
		}
	}
}



																		////////////////////////////////////


bool   OffMap::Hole_Plug_TwoTone(    long maxPixels,   bool  plugWhitePixelHoles,   CString&  retErrorMesg  )
{ 

		//  ONLY works on AutoSeparations  ( only 2 colors in the map...  white and a base-color ) 



	long   plugCount =  0,    maxPlugs =  30000;    //   3000000;   ***************  ADJUST,  just for DEBUG ***************

    short    h,v,   redTotal, gr,bl; 



	OffMap  *totalMaskmap       =      new   OffMap(   m_width,  m_height,    1   );                  //  'totalMaskmap' is LOCAL on-the-fly map
	if(           totalMaskmap ==  NULL  )
	{
		retErrorMesg  =    "OffMap::Hole_Plug_TwoTone  failed,  could not alloc  totalMaskmap." ;
		return  NULL;
	}
	totalMaskmap->Clear(  255  );      //  should be white
	


	OffMap  *MSKmap       =      new   OffMap(   m_width,  m_height,    1   );              
	if(           MSKmap ==  NULL  )
	{
		retErrorMesg  =    "OffMap::Hole_Plug_TwoTone  failed,  could not alloc  MSKmap." ;
		return  NULL;
	}
	MSKmap->Clear(  255  );      //  should be white
     
     

 
	OffMap  *SRCmap       =      new   OffMap(   m_width,  m_height,    m_depth   );        //  Make a copy of the sourceMap, so that our pixel-writes do NOT affect the algo       
	if(           SRCmap ==  NULL  )
	{
		retErrorMesg  =    "OffMap::Hole_Plug_TwoTone  failed,  could not alloc  SRCmap." ;
		return  NULL;
	}

	if(   ! SRCmap->Copy_Bits(   *this,   retErrorMesg  )      )
	{
		ASSERT( 0 );
		return  false;
	}



		short    rdSrc=255,  grSrc=255,  blSrc=255;
		bool     gotMapsBasicColor =  false;


		if(   plugWhitePixelHoles   )
			Set_Seed_FillMode(  false  );    //   true is the DEFAULT mode ( look for COLOR pixels to fill ).  REVERSES the behavior for SeedFill_MaskMap_Create_from_NonWhite_Pixels()



        for(  v= 0;     v < SRCmap->m_height;    v++ )
		{ 
			for(   h=0;     h < SRCmap->m_width;    h++ )
			{

				totalMaskmap->Read_Pixel(   h,v,    &redTotal,   &gr,&bl    ); 

				bool  isWhiteSrcPixel =    Color_Is_Almost_White(  h, v,   250   ); 


				if(              plugWhitePixelHoles  
						&&   ! gotMapsBasicColor
					    &&     isWhiteSrcPixel    )
				{
					SRCmap->Read_Pixel(    h,v,    &rdSrc,   &grSrc,  &blSrc    );
					gotMapsBasicColor =  true;
				}



                if(             redTotal  !=  0           //  0:  mask pix is not black
					 &&    ! isWhiteSrcPixel     ) 
				{  

					 MSKmap->Clear(  255  );    //  Erase_mealMap( &MSKmap );    


					 RectShrt   retBoundBox;
					 bool         writeToSRCmapPixels =   false;
					 short        hiCompValue =   Component_Almost_White();
					 long         retFoundPixelCount;




// ****** BETTER to get the CORE-COLOR around here for  'each new WhiteHole'  to plug ( plugWhitePixelHoles mode only )  ********************** 



					 if(   ! SeedFill_MaskMap_Create_from_NonWhite_Pixels(  h,v,  writeToSRCmapPixels,   0,0,0,   hiCompValue,      *MSKmap,   totalMaskmap,    
																																					retBoundBox,    retFoundPixelCount,     retErrorMesg )    )
					 {  ASSERT( 0 );
						Set_Seed_FillMode(  true  );    //  always restore to the default
						return  false;
					 }



  //                if(      (   retFoundPixelCount   >=  0L         )     ????   Why do 0L   there is nothing in the mask??   More efficient if I write to the totalMaskMap?    10/08
					if(      (   retFoundPixelCount    >   0L          )
						&& (   retFoundPixelCount    <   maxPixels    ))    
					{ 

						if(   plugWhitePixelHoles   )
						{

							if(  ! gotMapsBasicColor  )
							{
								int  dummy =  9;    // **** ERROR,  need a better routine to get the Map's other color ******
							}
							else
								Mask_Blits_Color(   *MSKmap,    totalMaskmap,    &retBoundBox,    rdSrc, grSrc,  blSrc     );      // writes WHITE to  'THISmap'  to cover up LONE WHITE pixes   

						}
						else
							Mask_Blits_Color(   *MSKmap,    totalMaskmap,    &retBoundBox,     255,      255,   255    );      // writes WHITE to  'THISmap'  to cover up LONE WHITE pixes   
    					                    


						 plugCount++;
					//	 if(    plugCount  >  maxPlugs   )
					//		 break;


						if(   (plugCount  %  800 ) ==  0   )
						{
							//   TRACE(   "  [ (%d, %d)  Num pixels in PLUG-fill:  %d,     SeedFillFunctCount = %d  ]    \n" ,   h,v,   retFoundPixelCount,   plugCount  );  
						}
					}



//				if(    plugCount  >  maxPlugs   )
//					break;		
			}

//			if(    plugCount  >  maxPlugs   )
//				break;
		}   //  for h =

//	 if(    plugCount  >  maxPlugs   )
//		break;
	}    //   for   v=  




    ////////////////////////////////////  

	Set_Seed_FillMode(  true  );    //  *** IMPORTANT,  always restore to the default *******
    
 

	if(  totalMaskmap   !=  NULL  )
		delete  totalMaskmap;

	if(   MSKmap  !=  NULL   )
		delete   MSKmap;

	if(   SRCmap  !=  NULL   )
		delete   SRCmap;
	

	return  true;
}



																		////////////////////////////////////


bool   OffMap::Do_Open_Morphology(    short  erodeDiameter,    short  dialateDiameter,    CString&  retErrorMesg  )
{ 


	if(   erodeDiameter  <= 0    ||     dialateDiameter   <= 0  )
	{
		retErrorMesg  =    "Do_Open_Morphology  failed,  improper input diameters ( <0 )." ;
		return  false;
	}



	OffMap  *undoMap       =      new   OffMap(   m_width,  m_height,    m_depth   );        //  Make a copy of the sourceMap, so that our pixel-writes do NOT affect the algo       
	if(           undoMap ==  NULL  )
	{
		retErrorMesg  =    "Do_Open_Morphology  failed,  could not alloc  undoMap." ;
		return  false;
	}

	if(   ! undoMap->Copy_Bits(   *this,   retErrorMesg  )      )
	{
		ASSERT( 0 );
		return  false;
	}


						//   an  'Opening'   is an erosion followed by a dialation

	ConvolutionMapFilter   filterErode(   this,   undoMap,     ConvolutionMapFilter::ERODEsEP,    erodeDiameter  );     //      ConvolutionMapFilter::ERODEsEP
	filterErode.Filter();


	if(   ! undoMap->Copy_Bits(   *this,   retErrorMesg  )      )
	{
		AfxMessageBox(  "Do_Open_Morphology failed,  Copy_Bits()"  );			
		return  false;
	}



	ConvolutionMapFilter   filterDialate(   this,   undoMap,     ConvolutionMapFilter::DIALATEsEP,    dialateDiameter  );     //      ConvolutionMapFilter::ERODEsEP
	filterDialate.Filter();



	delete  undoMap;     undoMap =  NULL;

	return  true;
}



																		////////////////////////////////////


void    OffMap::Add_Mirrored_Line(   short  x,  short  y,    short&  retPixelCount,      ListMemry< LINESEGMENToffmap >&  lineList    )
{


	LINESEGMENToffmap  *nuLineSeg        =        new   LINESEGMENToffmap();     //  drawPixel(  x,  y  )
	if(                               nuLineSeg ==  NULL  )
	{  ASSERT( 0 );     return;   }

		nuLineSeg->xl   =     -x;
		nuLineSeg->xr   =      x;	
		nuLineSeg->y   =       y;
		nuLineSeg->dy   =     2*x   +1;

	lineList.Add_Tail(   *nuLineSeg   );

	retPixelCount   +=    nuLineSeg->dy;


//	TRACE(    "CircleKernal:   X =  [ %d   to   %d ],      Y =  %d      ( count = %d ) \n"   ,   nuLineSeg->xl,   nuLineSeg->xr,    nuLineSeg->y,     retPixelCount   );

	if(   y  !=  0     )      //  no  matching  'symetric line'   when at the  X-axis
	{

		LINESEGMENToffmap  *nuLineSeg2        =        new   LINESEGMENToffmap();     //  drawPixel(  x,  y  )
		if(                               nuLineSeg2 ==  NULL  )
		{  ASSERT( 0 );     return;   }

			nuLineSeg2->xl   =     -x;
			nuLineSeg2->xr   =      x;	
			nuLineSeg2->y   =       -y;
			nuLineSeg2->dy   =     2*x   +1;

		lineList.Add_Tail(   *nuLineSeg2   );

		retPixelCount   +=    nuLineSeg2->dy;

//		TRACE(    "                X =  [ %d   to   %d ],      Y =  %d      ( count = %d ) \n\n"   ,   nuLineSeg2->xl,   nuLineSeg2->xr,    nuLineSeg2->y,     retPixelCount   );
	}
}



																		////////////////////////////////////


void     OffMap::Make_Circular_Kernal(   long  rad,     short&  retPixelCount,     ListMemry< LINESEGMENToffmap >&  lineList   )
{

							  //  uses  Bresenham's Circle Drawing Algorithm. 
						      //
							  //  This becomes tricky, because we do not want multiple writes on the same Pixel or Horiz-scanline     Tests OK.    10/08

	retPixelCount =   0;                  

	long   lastY =  rad;
	long  x1 =   0;
	long  y1 =   rad;
	long  d  =   3 -  2*rad;


	while(  x1  <=  y1  )
	{
		//	TRACE(   "\n  [  %d,    %d   ]  \n"  ,   y1,  x1  );             //  CLUMSEY ME:  have to swap x and y values so that Y has unique values in his algo

		Add_Mirrored_Line(    y1,  x1,      retPixelCount,    lineList   );              
		lastY =  x1;


		if(  d < 0 )
		{
			d  +=   4*x1  +6;
		}
		else
		{  d  +=   4 * ( x1 - y1 )   +10;
			y1--;
		}

		x1++;
	}

	//   TRACE(  "\n\n"  );



								                          //  Now go backwards through the list and swap x & y coords to make the rest of the QuarterHemisphere,  for the first new 'yvalye' we encounter
												          //    ...this helps us NOT have duplicated Horiz scan lines ( each LineSegment has a unique Y value )   10/08
	long     idx =   lineList.Count()   -1;

	while(   idx  >=  0  )
	{

		LINESEGMENToffmap&   lineSeg  =    lineList.Get(  idx );

		if(           lineSeg.xr   !=    lastY  
			  &&    lineSeg.xr   !=    absj( lineSeg.y )    )    //  remember since we put in pairs of scan lines[  Add_Mirrored_Line()  ],  the  Y-coord might be negative, so just use the absolute value
		{
			Add_Mirrored_Line(    absj( lineSeg.y ),    lineSeg.xr,      retPixelCount,    lineList   );  

			lastY  =   lineSeg.xr;
		}

		idx--;
	}
}



											////////////////////////////////////////


short     OffMap::Pick_A_TextureMap(   long  scaleDownThresh,    long  regionsWidth,   long  regionsHeight,    CString&  retErrorMesg   )
{

		//  REWRITE so that there is more randomizing in selecting


	short   maxNumberOfChoices  =   3;      //   2  **************   ADJUST  ***********************************



	short   acceptableChoices[  8  ],    acceptableChoicesSize =  8;



	if(    scaleDownThresh < 0    )     //  cheezy test
		scaleDownThresh =  1;


//	long   regionsWidth  =   m_SRCoffMap->m_width;
//	long   regionsHeight =   m_SRCoffMap->m_height;


	for(   short i= 0;     i < acceptableChoicesSize;    i++  )
		acceptableChoices[ i ]  =  -1;     //  init the array





	short   travIndex =  0,   choicesFoundCount = 0;



															//    1)  Fill the choices array with some reasonable possible TextureMaps

	while(           choicesFoundCount   <  maxNumberOfChoices       //   index < 0   
				&&   travIndex  <   OffMap::Get_Total_TextureMap_Count()   )
	{

		if(         Get_TextureMap_Array(   travIndex  ).width     >=    ( regionsWidth * scaleDownThresh )  
			 &&   Get_TextureMap_Array(   travIndex  ).height    >=    ( regionsHeight * scaleDownThresh )    )
		{

			acceptableChoices[  choicesFoundCount ]  =    travIndex;       //   index =    travIndex;
			choicesFoundCount++;

			travIndex++;
		}
		else
			travIndex++;
	}



	if(    choicesFoundCount   <= 0    )
	{
		retErrorMesg =  "RegionSubject::Pick_A_TextureMap FAILED,  could not find ANY TextureMaps that were big enough."  ;
		return  -2;
	}



															//    2)  Use a random number generator to pick one of the 'acceptableChoices'

	short   index =  -1;   //  init for fail
	int       nuRand,   indexChoices;   


	nuRand =   rand();

	indexChoices  =     nuRand  %    (int)choicesFoundCount;

//	TRACE(  "  [  Rand() gave %d ]  "   ,   indexChoices   );     // ******************  TEMP,  debug only ************


	index   =      acceptableChoices[  indexChoices  ]; 

	if(  index  <  0  )
	{
		retErrorMesg =  "RegionSubject::Pick_A_TextureMap FAILED,  could not find a TextureMap that is big enough."  ;
		return  -1;
	}



								//  Calc what the acual scaledown is that we will use   [   ****  DEBUG only *****  ]

	long    xScale =    Get_TextureMap_Array(   index  ).width      /  regionsWidth;       
	long    yScale =    Get_TextureMap_Array(   index  ).height    /  regionsHeight;

	long    workingScale  =   minj(  xScale,   yScale  );

		
	
	return  (short)index;	
}

