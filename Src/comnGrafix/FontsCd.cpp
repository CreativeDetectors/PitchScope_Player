/////////////////////////////////////////////////////////////////////////////
//
//  FontsCd.cpp   -   
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



#include   "..\comnFacade\UniEditorAppsGlobals.h"

#include  "..\comnFacade\VoxAppsGlobals.h"


//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"      //  #include <FixMath.h>    ...Mac's version of Fixedpoint 
#include  "..\ComnGrafix\CommonGrafix.h"     

#include  "..\ComnGrafix\OffMap.h"  
//#include  "..\comnGrafix\TransformMap.h"

////////////////////////////////////////////////// 





#include  "..\comnGrafix\FontsCd.h"     

////////////////////////////////////////////////////////////////////////////////////////////////////////////




LongIndexed   fontOffsets6[  29  ] =    //   Ariel Narrow  Bold ???       (  12,  but height is 11   )
{
// scalePitch			frontOffset	        backOffset             arrayINDEX

		0,						   1,					 7,			//	E			0

		1,						   11,					16,		//	F			1


		2,						  20,					35,		//	F#			2
		2,						  39,					54,		//  Gb		3


		3,						  58,					65,		//	G			4


		4,						  69,					86,		//	G#		5
		4,						  89,				  104,		//	Ab			6


		5,						107,				  115,		//	A			7	


		6,						119,				  136,		//	A#		8
		6,						140,				  154,		//	Bb			9


		7,						158,				  164,		//	B			10


	   8,						168,				  175,		//	C			11


	   9,						179,			      196,		//	C#		12
	   9,						200,				  214,		//	Db		13


	  10,						218,				  224,		//	D			14


	  11,						228,				  244,		//	D#		15
	  11,						248,				  262,		   //	Eb		    16


		0,						 268,			      271,		//	1st		17

		1,						 286,				  297,		//	1+		18	

		2,						 300,			      305,		//	2			19

		3,						 323,				  336,		//	b3			20

		4,						 341,				  356,		//	M3		21

		5,						 361,				  367,		//	4th		22

		6,						 382,				  395,		//	b5			23

		7,						 399,				  404,		//	5			24

	    8,						 420,				  433,		//	5+		25

	    9,						 437,			      441,		//	6			26

	  10,						 457,				  469,		//	b7			27

	  11,						 473,				  489		    //	M7		28
};




LongIndexed   fontOffsets7[  29  ] =                                         //   Ariel Narrow   (  12,  but height is 11   )   *** Most NARROW  ******
{
// scalePitch			frontOffset	        backOffset             arrayINDEX

		0,						   2,					 7,			//	E			0

		1,						   9,				   14,		//	F			1

		2,						  16,				   26,		//	F#			2
		2,						  30,				   42,		//  Gb		3

		3,						  45,				   52,		//	G			4

		4,						  54,				   67,		//	G#		5
		4,						  71,				   84,		//	Ab			6

		5,						87,				   95,		    //	A			7	

		6,						97,				  109,		//	A#		8
		6,						114,				  125,		//	Bb			9

		7,						128,				  134,		//	B			10

	   8,						136,				  143,		//	C			11

	   9,						145,			      157,		//	C#		12
	   9,						162,				  173,		//	Db		13

	  10,						176,				  182,		//	D			14

	  11,						184,				  196,		//	D#		15
	  11,						200,				  210,		    //	Eb		  16


		0,						 268,			      272,   //   282,		//	1st		17

		1,						 286,				  297,		//	1+		18	

		2,						 300,			      305,  // 317,		//	2			19

		3,						 324,				  336,		//	b3			20

		4,						 342,				  355,		//	M3		21

		5,						 361,				  367,  // 378,		//	4th		22

		6,						 383,				  395,		//	b5			23

		7,						 399,				  404,    //  416,		//	5			24

	    8,						 420,				  432,		//	5+		25

	    9,						 437,			      441,  // 453,		//	6			26

	  10,						 458,				  469,		//	b7			27

	  11,						 474,				  486		    //	M7		28
};




LongIndexed   fontOffsets8[  29  ] =         //   Ariel bold   26   ( in Resource Editor,    ( actual height is 25  ) )    
{
// scalePitch			frontOffset	        backOffset             arrayINDEX

		0,						   1,				   18,		//	E			0

		1,						 24,				   39,		//	F			1


		2,						 45,				   79, //	81,		//	F#			2
		2,						 83,				  128,		//  Gb		3


		3,						131,				  154,		//	G			4


		4,						158,				  201,  // 202,		//	G#		5
		4,						203,				  245,		//	Ab			6


		5,						247,			      269,		    //	A			7	


		6,						270,				  309, //  311,		//	A#		8
		6,						315,				  355,		//	Bb			9


		7,						360,				  379,		//	B			10


	   8,						382,				  403,		//	C			11


	   9,						407,			      447,  //  449,		//	C#		12
	   9,						453,				  494,		//	Db		13


	  10,						499,				  519,		//	D			14


	  11,						524,				  563, //  564,		//	D#		15
	  11,						568,				  607,		//	Eb		  16



		0,						 618,			      628,		//	1st		17

		1,						 659,				  687,		//	1+		18	

		2,						 694,			      709,		//	2			19

		3,						 748,				  782,		//	b3			20

		4,						 790,				  832,		//	M3		21

		5,						 838,				  855,		//	4th		22

		6,						 885,				  919,		//	b5			23

		7,						 926,				  941,		//	5th			24

	    8,						 969,				 1001,		//	5+		25

	    9,						1008,			 1023,		//	6			26

	  10,						1054,			 1088,		//	b7			27

	  11,						1094,			 1136		    //	M7		28
};








LongIndexed   fontOffsets9[  29  ] =         //   Ariel Narrow bold   18   ( in Resource Editor )
{
// scalePitch			frontOffset	        backOffset             arrayINDEX

		0,						   1,					11,		//	E			0

		1,						 14,					23,		//	F			1


		2,						 26,					46,		//	F#			2
		2,						 49,				    71,		//  Gb		3


		3,						 76,				   88,		     //	G			4


		4,						 91,				  115,		//	G#		5
		4,						116,				  137,		//	Ab			6


		5,						142,			      154,		    //	A			7	


		6,						156,				  178,		//	A#		8
		6,						182,				  203,		//	Bb			9


		7,						208,				  219,		//	B			10


	   8,						222,				  233,		//	C			11


	   9,						236,			      258,		//	C#		12
	   9,						261,				  282,		//	Db		13


	  10,						287,				  298,		//	D			14


	  11,						301,				  323,		//	D#		15
	  11,						326,				  346,		    //	Eb		  16



		0,						 355,			     360,  // 375,		//	1st		17

		1,						 382,				  397,		//	1+		18	

		2,						 404,			      412,  // 432,		//	2			19

		3,						 439,				  458,		//	b3			20

		4,						 465,				  489,		//	M3		21

		5,						 494,				  503,  // 518,		//	4th		22

		6,						 526,				  544,		//	b5			23

		7,						 549,				  557,  //  572,		//	5th			24

	    8,						 578,				  595,		//	5+		25

	    9,						 600,			      608,  // 624,		//	6			26

	  10,						 631,				  649,		//	b7			27

	  11,						 657,				  681		    //	M7		28
};










void   Blit_Font_Character_to_Display(  void *cDevC,    long xDest,  long yDest,      long width,  long height,      bool doXOR,  
																		                     long  xOffsetSrc,  long yOffsetSrc, 	  CBitmap&  fontMask  );   //   NEW,  2/12



////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


FontsCd::FontsCd( void )
{
		// Must initialize   m_fontsBitmap

	m_height =  -1;

	fontOffsetsTable =    &(   fontOffsets6[0]   );
}


										////////////////////////////////////////

FontsCd::~FontsCd(void)
{
}




										////////////////////////////////////////


bool    FontsCd::Change_To_New_Font(   short fontID   )
{


	//  *****   Must SYNC  the CASE Statements with     PsNavigatorDlg::Change_FontCd()    *********************



	bool   functionSucess =   true;


	switch(   fontID   )
	{
		case   6 :	
			fontOffsetsTable =    &(   fontOffsets6[0]   );
		break;

		case   7 :	
			fontOffsetsTable =    &(   fontOffsets7[0]   );    //   Ariel Narrow
		break;

		case   8 :	
			fontOffsetsTable =    &(   fontOffsets8[0]   );    //   Ariel bold  26    ( actual height is 25  )
		break;

		case   9 :	
			fontOffsetsTable =    &(   fontOffsets9[0]   );    //   Ariel Narrow bold  18   ( actual height is 17  )
		break;



		default:    
			ASSERT( 0 );    
			fontOffsetsTable =    &(   fontOffsets6[0]   );

			AfxMessageBox(  "PsNavigatorDlg::FontsCd::Change_To_New_Font  FAILED,  missing Case."  );
			functionSucess =  false;
		break; 
	}

	return   functionSucess;
}




					/////////////////////////////////////////////////


bool	  FontsCd::Blit_Musical_Notes_NameText(   short scalePitch,   short userPrefMusicalKeyAccidentals,   long xDest,  long yDest,   bool doXOR,   CDC& dc,  CString&  retErrorMesg   )
{


		//   userPrefMusicalKeyAccidentals:     0:  No preference     1:  Use Sharps    2:  UseFlats   3: Use NUMERALS      


		//   if(   doXOR  ==  false )    then Letter is Blitted in BLACK



	long   srcYoffset =    1;  // *******************  ALWAYS ???  2/2012   ******************


	retErrorMesg.Empty();

	if(   scalePitch  <  0  )
	{
		retErrorMesg =   "Blit_Musical_Notes_NameText  FAILED,   charCode < 0" ;
		return  false;
	}


	long   retXoffsetSrc,   retWidth,   retHeight;


	CSize  fontmapSize  =    m_fontsBitmap.GetBitmapDimension();    //  ****  FAILS,   I need to record whis when I create the Bitmap  ************



	Calc_NoteText_BoundBox_and_xOffset(   scalePitch,   userPrefMusicalKeyAccidentals,   retXoffsetSrc,    retWidth,  retHeight   );

	if(   retXoffsetSrc  <  0  )
	{
		retErrorMesg =   "Blit_Musical_Notes_NameText  FAILED,   Calc_NoteText_BoundBox_and_xOffset" ;
		return  false;
	}
	


//		 COLORREF   whiteColor   =   RGB(  255,  255,  255   ); 
	//	if(    ! Blit_XOR_Text_MaskBlt(    charCode,   xText,  yText,    retErrorMesg  )     )   //  lots of problems
//		if(   !  Blit_XOR_Text(    charCode,    xText,  yText,    m_fontsBitmap1,    retErrorMesg   )   )
	//	Draw_Transparent(  &dc,    xText,  yText,   whiteColor,   m_fontsBitmap1  );       
	
	Blit_Font_Character_to_Display(   &dc,    xDest, yDest,    retWidth,  retHeight,    doXOR,    
		                                                                                  retXoffsetSrc,  srcYoffset,    m_fontsBitmap  );   //  NEW,  in   platformSpecific.cpp   2/12


	return  true;
}


										////////////////////////////////////////


long	  FontsCd::Get_Characters_Width(   short  scalePitch,    short  userPrefMusicalKeyAccidentals   )
{

	if(       scalePitch  <   0  
		||   m_height  <=  0   )
	{
		ASSERT( 0 );
		return -1;
	}

	long   retXoffsetSrc,   retWidth,    retHeight;


	Calc_NoteText_BoundBox_and_xOffset(   scalePitch,    userPrefMusicalKeyAccidentals,    retXoffsetSrc,  	 retWidth,  retHeight    );

	
//	retWidth =   retWidth   + 1;   //  Inclusive Counting    This is done in 

	return  retWidth;
}



										////////////////////////////////////////


void	  FontsCd::Calc_NoteText_BoundBox_and_xOffset(   short  scalePitch,   short userPrefMusicalKeyAccidentals,    long&  retXoffsetSrc,    
													                                                                  long&  retWidth,   long&  retHeight  )
{

	retXoffsetSrc   =  -1;
	retWidth  =   0;
	retHeight  =  0;  


	LongIndexed  *offsetTable =     fontOffsetsTable;       //    &(   fontOffsets7[ 0 ]   );      read from a TABLE that is an array  

	if(   offsetTable  ==   NULL  )
	{
		AfxMessageBox(   "Calc_NoteText_BoundBox_and_xOffset  FAILED,  fontOffsetsTable is NULL."   );
		return;
	}


	if(       scalePitch  <   0  
		||   m_height  <=  0   )
	{
		ASSERT( 0 );
		return;
	}

	retHeight =   m_height;  



	bool   useNumerals =  false;  
	bool   useSharps =  false;

	if(          userPrefMusicalKeyAccidentals  ==  3   )    //   0:  No preference     1:  Use Sharps     2:  UseFlats    3: Use Numerals  
		useNumerals =  true; 
	else if(   userPrefMusicalKeyAccidentals  ==  1  )  
		useSharps     =  true;




	switch(   scalePitch   )
	{

		case   0 :			//   E
			if(   useNumerals  )
			{
				retXoffsetSrc =   offsetTable[ 0 + 17 ].value0;
				retWidth        =   offsetTable[ 0 + 17 ].value1   -   offsetTable[ 0 + 17 ].value0;  
			}
			else
			{  retXoffsetSrc =   offsetTable[ 0 ].value0;
				retWidth        =   offsetTable[ 0 ].value1   -   offsetTable[ 0 ].value0;  
			}
		break;


		case   1 :			 //   F
			if(   useNumerals  )
			{
				retXoffsetSrc =   offsetTable[ 1 + 17 ].value0;
				retWidth        =   offsetTable[ 1 + 17 ].value1   -   offsetTable[ 1 + 17 ].value0;  
			}
			else
			{  retXoffsetSrc =   offsetTable[ 1 ].value0;
				retWidth        =   offsetTable[ 1 ].value1   -   offsetTable[ 1 ].value0;  
			}
		break;


		case   2 :  
			if(   useNumerals  )
			{  
				retXoffsetSrc =   offsetTable[ 2 + 17 ].value0;
				retWidth        =   offsetTable[ 2 + 17 ].value1   -   offsetTable[ 2 + 17 ].value0;  
			}
			else
			{  if(   useSharps   )						//   F#
				{  retXoffsetSrc =   offsetTable[ 2 ].value0;
					retWidth        =   offsetTable[ 2 ].value1   -   offsetTable[ 2 ].value0;  
				}
				else
				{  retXoffsetSrc =   offsetTable[ 3 ].value0;		//   Gb
					retWidth        =   offsetTable[ 3 ].value1   -   offsetTable[ 3 ].value0;  
				}
			}
		break;


		case   3 :				//   G
			if(   useNumerals  )
			{  
				retXoffsetSrc =   offsetTable[ 3 + 17 ].value0;
				retWidth        =   offsetTable[ 3 + 17 ].value1   -   offsetTable[ 3 + 17 ].value0;  
			}
			else
			{  retXoffsetSrc =   offsetTable[ 4 ].value0;
				retWidth        =   offsetTable[ 4 ].value1   -   offsetTable[ 4 ].value0;  
			}
		break;


		case   4 :  
			if(   useNumerals  )
			{  
				retXoffsetSrc =   offsetTable[ 4 + 17 ].value0;
				retWidth        =   offsetTable[ 4 + 17 ].value1   -   offsetTable[ 4 + 17 ].value0;  
			}
			else
			{ 	if(   useSharps   )						//   G#
				{  retXoffsetSrc =   offsetTable[ 5 ].value0;
					retWidth        =   offsetTable[ 5 ].value1   -   offsetTable[ 5 ].value0;  
				}
				else
				{  retXoffsetSrc =   offsetTable[ 6 ].value0;		//   Ab
					retWidth        =   offsetTable[ 6 ].value1   -   offsetTable[ 6 ].value0;  
				}
			}
		break;


		case   5 :			 //   A
			if(   useNumerals  )
			{  
				retXoffsetSrc =   offsetTable[ 5 + 17 ].value0;
				retWidth        =   offsetTable[ 5 + 17 ].value1   -   offsetTable[ 5 + 17 ].value0;  
			}
			else
			{ 	retXoffsetSrc =   offsetTable[ 7 ].value0;
				retWidth        =   offsetTable[ 7 ].value1   -   offsetTable[ 7 ].value0;  
			}
		break;


		case   6 :  
			if(   useNumerals  )
			{  
				retXoffsetSrc =   offsetTable[ 6 + 17 ].value0;		
				retWidth        =   offsetTable[ 6 + 17 ].value1   -   offsetTable[ 6 + 17 ].value0;  
			}
			else
			{ 	if(   useSharps   )						//   A#
				{  retXoffsetSrc =   offsetTable[ 8 ].value0;
					retWidth        =   offsetTable[ 8 ].value1   -   offsetTable[ 8 ].value0;  
				}
				else
				{  retXoffsetSrc =   offsetTable[ 9 ].value0;		//   Bb
					retWidth        =   offsetTable[ 9 ].value1   -   offsetTable[ 9 ].value0;  
				}
			}
		break;


		case   7 :				//   B
			if(   useNumerals  )
			{  
				retXoffsetSrc =   offsetTable[ 7 + 17 ].value0;
				retWidth        =   offsetTable[ 7 + 17 ].value1   -   offsetTable[ 7 + 17 ].value0;  
			}
			else
			{  	retXoffsetSrc =   offsetTable[ 10 ].value0;
				retWidth        =   offsetTable[ 10 ].value1   -   offsetTable[ 10 ].value0;  
			}
		break;


		case   8 :			 //   C
			if(   useNumerals  )
			{  
				retXoffsetSrc =   offsetTable[ 8 + 17 ].value0;
				retWidth        =   offsetTable[ 8 + 17 ].value1   -   offsetTable[ 8 + 17 ].value0;  
			}
			else
			{  	retXoffsetSrc =   offsetTable[ 11 ].value0;
				retWidth        =   offsetTable[ 11 ].value1   -   offsetTable[ 11 ].value0;  
			}
		break;


		case   9 :  
			if(   useNumerals  )
			{  
				retXoffsetSrc =   offsetTable[ 9 + 17 ].value0;		
				retWidth        =   offsetTable[ 9 + 17 ].value1   -   offsetTable[ 9 + 17 ].value0;  
			}
			else
			{  	if(   useSharps   )						//   C#
				{  retXoffsetSrc =   offsetTable[ 12 ].value0;
					retWidth        =   offsetTable[ 12 ].value1   -   offsetTable[ 12 ].value0;  
				}
				else
				{  retXoffsetSrc =   offsetTable[ 13 ].value0;		//   Db
					retWidth        =   offsetTable[ 13 ].value1   -   offsetTable[ 13 ].value0;  
				}
			}
		break;


		case   10 :				 //   D
			if(   useNumerals  )
			{  
				retXoffsetSrc =   offsetTable[ 10 + 17 ].value0;
				retWidth        =   offsetTable[ 10 + 17 ].value1   -   offsetTable[ 10 + 17 ].value0;  
			}
			else
			{ 	retXoffsetSrc =   offsetTable[ 14 ].value0;
				retWidth        =   offsetTable[ 14 ].value1   -   offsetTable[ 14 ].value0;  
			}

		break;


		case   11 :
			if(   useNumerals  )
			{  
				retXoffsetSrc =   offsetTable[ 11 + 17 ].value0;
				retWidth        =   offsetTable[ 11 + 17 ].value1   -   offsetTable[ 11 + 17 ].value0;  
			}
			else
			{  if(   useSharps   )						//   D#
				{  retXoffsetSrc =   offsetTable[ 15 ].value0;
					retWidth        =   offsetTable[ 15 ].value1   -   offsetTable[ 15 ].value0;  
				}
				else
				{  retXoffsetSrc =   offsetTable[ 16 ].value0;		//   Eb
					retWidth        =   offsetTable[ 16 ].value1   -   offsetTable[ 16 ].value0;  
				}
			}
		break;


		default:    
			ASSERT( 0 );    
			retXoffsetSrc   =  -1;
		break; 
	}


	ASSERT(   retWidth  > 0  );   //  get this if  read out-of-bounds  in the Array   2/12



	retWidth =   retWidth   + 1;   //  Inclusive Counting
}




