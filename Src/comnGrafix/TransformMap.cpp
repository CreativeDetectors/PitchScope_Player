/////////////////////////////////////////////////////////////////////////////
//
//  TransformMap.cpp   -   
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

/////////////////////////////////////////////////           

#include  "..\ComnGrafix\OffMap.h"  

#include  "..\ComnGrafix\mapFilters.h"  

//////////////////////////////////////////////////     






#include  "TransformMap.h"

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


TransformMap::TransformMap(   long  width,   long  height,    int  depth,    long  horzScaleDown    )
													    :  OffMap( width, height, depth ),   m_horzScaleDown( horzScaleDown ) 
{

	m_hasChannels  =    false;    //   ALL default  are for NO channels

	m_channelCount =    1;

	m_channelWidth =  1;			
	m_channelDataWidth  =  1;


	m_topPad =  0;       //  the single padding at the top of the map
	m_channelPad =  0;    //  NEW



	m_undoMap =  NULL;

	m_maskMap =  NULL;
	m_maskMapHasData =   false;



// **********************   HARDWIRE,  fix with Parms Structure ****************************

//	m_duraRectPixelThresh =  -1;    //   for   Get_DurationRectangle  ...tricky to pass this thry polmorphism
//	m_duraRectPixelCount  =  -1;     //   INIT with Verter





																	//   'FLOAT-MAP'  mode....
	m_inFloatmapMode  =   false;	
	
	m_readersOffset      =   0;

	m_verterFloatMap    =    NULL;

	m_sourceMapFloat   =    NULL;
	m_sourceMapFloat2 =    NULL;
	m_sourceMapFloat3 =    NULL;
	
	m_lastLoadedXColumnLeader =    -99;    // ***** OK initialize ????  ******  JPM 




	m_pixelReadOffset  =   0;     //  To allow  TransformAnalysis to occur at  'OFFSETTED-segments'   within Sample.     See   ReDetect_TimeZone( )
	m_pixelReadWidth  =   0;   
}




												////////////////////////////////////////


TransformMap::~TransformMap()
{

	Release_UndoMap();

	Release_MaskMap();


	if(     m_verterFloatMap  !=   NULL   )		// ***** OK ???? 
		delete   m_verterFloatMap;
}


												////////////////////////////////////////


bool     TransformMap::Needs_Offsetting()
{																			//    for NAVIGATOR's  LogDFT,    this is always RETURNS  'FALSE ' 

	if(        m_pixelReadOffset   <=  0
		&&   m_pixelReadWidth   <=  0     )		//  normally is true,  
		return  false;
	else
		return  true;
}


												////////////////////////////////////////


bool   TransformMap::Create_UndoMap()
{

	if(    m_undoMap  !=  NULL   )
	{
		delete  m_undoMap;
		m_undoMap =  NULL;    //  flag release
	}


	m_undoMap  =    new  TransformMap(   this->m_width,   this->m_height,   this->m_depth,   this->m_horzScaleDown   );

	if(   m_undoMap  ==  NULL  )
		return  false;



	*m_undoMap =    *this;   // *** DANGEROUS   ASSIGNMENT OPERATOR  ***  

															  // ******************************************************************
	m_undoMap->m_undoMap  =   NULL;    // *****SLOPPY:  careful NOT to copy the 'm_undoMap'  pointer,  or get stack overflows.
															  // ******************************************************************
															  //     ...really need to write a better  ASSIGNMENT OPERATOR for  TransformMap class   6/02


	
	m_undoMap->m_maskMap  =    this->m_maskMap;    // ***  ??? whan to copy this POINTER too ???   ...or  set NULL ?? 





	/***
	bool		m_inFloatmapMode;

	short		m_lastLoadedXColumnLeader;    

	short       m_readersOffset;    //  If this is a SourceMap for another READING-map,  the 'reader' must add this to its LOCAl xlead to find the Load_Leading_XColumn() column

	TransformMap	 *m_sourceMapFloat;	
	TransformMap	 *m_sourceMapFloat2;	
	TransformMap	 *m_sourceMapFloat3;

	FowardMapVerter     *m_verterFloatMap;     //   For the  DST-map( 'this' map )   ( not  for SRC-maps   ...though it will access them )
	***/


	m_undoMap->m_verterFloatMap  =    NULL;     //  another unintended assignment from  'ASSIGNMENT OPERATOR'

//	m_undoMap->m_inFloatmapMode =  0  ;         //   ??????  ****** WHAT do I want copied ???? ********

//	m_undoMap->m_sourceMapFloat   =    NULL;     //  ?????
//	m_undoMap->m_sourceMapFloat2   =    NULL;     //  ?????
//	m_undoMap->m_sourceMapFloat3   =    NULL;     //  ?????


	return  true;
}




												////////////////////////////////////////


bool   TransformMap::Create_MaskMap()
{


	Release_MaskMap();


	m_maskMap  =    new  TransformMap(   this->m_width,   this->m_height,   1,   this->m_horzScaleDown   );

	if(   m_maskMap  ==  NULL  )
		return  false;


	m_maskMap->Clear( 0 );


	m_maskMapHasData =   false;


															  // ******************************************************************
	m_maskMap->m_undoMap   =   NULL;    // *****SLOPPY:  careful NOT to copy the 'm_undoMap'  pointer,  or get stack overflows.
															  // ******************************************************************
															  //     ...really need to write a better  ASSIGNMENT OPERATOR for  TransformMap class   6/02
	m_maskMap->m_maskMap  =   NULL;  

	return  true;
}



												////////////////////////////////////////


void     TransformMap::Release_UndoMap()
{

	if(    m_undoMap  !=  NULL   )
	{
		delete  m_undoMap;
		m_undoMap =  NULL;    //  flag release
	}
}


												////////////////////////////////////////


void     TransformMap::Release_MaskMap()
{

	if(    m_maskMap  !=  NULL   )
	{
		delete  m_maskMap;
		m_maskMap =  NULL;
	}
}



												////////////////////////////////////////


/***  NOT necessary,  the built in Assignment operator will copy all membervars ...just need to REwrite Offmap's assignment operator

		...HOWEVER, I now have a POINER( m_undoMap )  that could be a problem !!!!    6/02

TransformMap&   TransformMap::operator=(  TransformMap  &other  )
{

	//  OffMap::operator=   other;

	OffMap   *thisOffmap    =     this;
	OffMap   *otherOffmap  =     &other;


	*thisOffmap  =   *otherOffmap;     //  Will this  ASSIGNMENT  copy all of the  BASE class elements ???



   ...need this, or does the compiler do this ???

	m_horzScaleDown =     other.m_horzScaleDown;

	m_hasChannels =     other.m_hasChannels;

	m_channelCount =     other.m_channelCount;

	m_channelWidth =     other.m_channelWidth;

	m_channelDataWidth =     other.m_channelDataWidth;	

	m_topPad =     other.m_topPad;



	return  *this;
}
***/



	 

											////////////////////////////////////////


long	 TransformMap::Get_Channels_Top_Yval(   long   channelIdx   )
{

								//   should work for    EVEN and ODD    'm_channelDataWidth'


    long    yCoord =    (m_height  -1)   -    
												  (   (channelIdx  *  m_channelWidth)   +  m_topPad    );   

	return  yCoord;
}



												////////////////////////////////////////


long	  TransformMap::Get_Channels_Bottom_Yval(   long   channelIdx   )
{
	
									//   should work for EVEN and ODD  'm_channelDataWidth'

    long    yCoord =    (m_height  -1)   -    
										(   (channelIdx * m_channelWidth)   +   m_topPad   +    ( m_channelDataWidth  -1 )       );   

	return  yCoord;	
}





												////////////////////////////////////////


long   TransformMap::Get_Channels_Center_Yval(  long  channelIdx   )  
{   

													//  Gives the Ycoord of the   'CENTER Row'   of the channel


	ASSERT(     ( m_channelDataWidth  %  2 ) ==  1    );     //  make sure  m_channelDataWidth  is ODD



    long  yCoord =    (m_height  -1)   -    
									(   (channelIdx  *  m_channelWidth)    +  m_topPad   +   (  m_channelDataWidth/2 )     );   


	//  (  m_channelDataWidth/2 )     moves us from   FIRST-row  to the   CENTER-row,   IF  m_channelDataWidth is ODD !!!!   5/02


	return   yCoord;				 //  ( 12notes x 14 )[168]  +  HARMMAPTopPAD( 8 )  + 1    =  177    
}




												////////////////////////////////////////


long    TransformMap::Ycoord_2ChannelIdx(   long  y  )  
{   

								//  SHOULD(?)  be the same as   Ycoord_2Pitch()  ***TEST***


    long   ycoordVirt   =     ( m_height  - 1)    -  y;   	//  what the coord would be if the map were  NOT INVERTED,  
			

	long   yOffsetted   =      ycoordVirt    -   m_topPad;    //  .... and NOT offseted by PAD at top 


					
	long      retScalePitch  =    yOffsetted  / m_channelWidth;   //  Now OK to DIVIDE, since no offsetting is applied. 													
	return   retScalePitch;
}





												////////////////////////////////////////


void     TransformMap::ColorFill_Channel_Segment(   long  x0,   long  x1,     short  channelIdx,     short  greyVal   )
{



	if(   x0  >  x1   )		//  'sort'   to have smallest first...
	{
		long  tmp =   x0;
		x0      =    x1;
		x1      =    tmp;
	}
	ASSERT(   x1  >=   x0   );



	short   channelWidth  =    Get_Channel_Width();
	short	dataWidth       =    Get_DataChannel_Width();
										
	

	long	yTop         =     Get_Channels_Top_Yval(        channelIdx   );  
	long	yBottom    =     Get_Channels_Bottom_Yval(   channelIdx   );  




	if(    yTop  >  yBottom   )		//  'sort'   to have smallest first...
	{
		long  tmp =   yTop;
		yTop      =    yBottom;
		yBottom =    tmp;
	}
	ASSERT(  yBottom >=   yTop  );




	if(    x0   <     0       )
		x0 =  0;

	if(    x1   >   (m_width -1)    )   //  If we make a new DZone that is smaller than the previous one
		x1 =   m_width -1;              //  one that generated the existing NoteList,  it may want to draw out of bounds.  3/04





	for(    long  x = x0;     x  <=  x1;      x++    )
	{
		for(    long y =  yTop;      y <=  yBottom;      y++    )
			Write_Pixel(   x, y,    greyVal,  greyVal,  greyVal   );		 
	}
}



												////////////////////////////////////////


bool	TransformMap::Merge_Bits(   TransformMap&  otherMap,     CString&   retErrorMesg    )
{

	retErrorMesg.Empty();


	if((       otherMap.Is_Empty()   )
		||(         this->Is_Empty()    ))
	{  
		retErrorMesg =   "Merge_Bits::Merge_Bits() failed,  Bitmaps are NOT in MEMORY. ";     
		return  false;  
	}
     

     if((      otherMap.m_width    !=    this->m_width     )
         ||(  otherMap.m_depth    !=    this->m_depth     ))
	{  
		 retErrorMesg =    "Merge_Bits::Merge_Bits() failed,  Bitmaps do not have the same  WIDTH and DEPTH. " ;     
		 return  false;  
	 }
 

	if(    otherMap.Get_DataChannel_Width()   !=  1   )   // ***Could enhance.... ****
	{  
		 retErrorMesg =    "Merge_Bits::Merge_Bits() failed,  otherMap's channels are wider than one. " ;     
		 return  false;  
	 }



	if(    otherMap.m_height   ==    this->m_height    )
	{
		if(    !OffMap::Merge_Bits(   otherMap,   retErrorMesg   )     )
			return  false;  
		else
			return  true;
	}
	else												 //   it can still happen by going channel by channel
	{														
		long  thisChannelCnt  =    Get_Channel_Count();
		


		for(   long  channel =0;     channel <  thisChannelCnt;     channel++    )
		{

			long  yOtherCenter =     otherMap.Get_Channels_Center_Yval(  channel  );  


			long   yTop =     this->Get_Channels_Top_Yval(        channel   );   
			long	yBot  =     this->Get_Channels_Bottom_Yval(   channel   );   

			if(   yTop  >  yBot   )
			{
				long   tmp =  yTop;

				yTop =   yBot;
				yBot  =   tmp;
			}



			for(    long  x=0;    x< m_width;     x++     )
			{

				short  otherVal,  thisVal,  nuVal,    gr,bl;

				otherMap.Read_Pixel(   x,  yOtherCenter,     &otherVal,   &gr, &bl  ); 


				for(   long y =  yTop;      y <=  yBot;      y++    )
				{

					this->Read_Pixel(   x,  y,     &thisVal,   &gr, &bl  ); 

					short  val   =        (  thisVal /2  )    +     (  otherVal  /2  );
					if(  val  > 255  )
						nuVal  =    255;
					else if(  val  < 0  )
						nuVal  =    0;
					else
						nuVal  =   val;

					Write_Pixel(   x,  y,     nuVal,  nuVal, nuVal  );
				}

			}   //  for(  x=
		}
	}


	return  true;
}




												////////////////////////////////////////


bool    TransformMap::Get_DurationRectangle(    long x,  long y,	   
																	   short  pixelThresh,   short  minPixelCount,
											                           long&  retStartPix,    long&  retEndPix,   
											                           long&  retChannelIdx,   
																	   short&  retAverageRunBriteness,    CString&  retErrorMesg   )
{

		//   Does a virtual SeedFill horizontally to find the start and finish of a segment whose pixels are greater than 'pixelThresh'
		//   and whose length is greater than  'minPixelCount'  


	long   totalRunTone =  0,     totalReadCnt = 0;     



	retErrorMesg.Empty();

	retStartPix =  -1;
	retEndPix   =  -1;
	retChannelIdx  =  -1;
	retAverageRunBriteness =  -2;


	retChannelIdx  =    Ycoord_2ChannelIdx(  y  );   //  Really is ScalePitch code


	if(    retChannelIdx  < 0     )        //   ||   retChannelIdx  > 11  )  
	{   
		ASSERT( 0 );

		// NO!!!     retErrorMesg =  "FundamentalGroupTrForm::Get_DurationRectangle()  failed,  Ycoord_2Pitch()."  ;

		// ** Sometimes is NOT an error if the user hit a pad Row  BETWEEN  scalePitch lanes   ...***** FIX,  CLEANUP ****

		retChannelIdx =  -1;
		return  false;
	}




	long    yKern[ 32 ];    
	short   val[ 32 ];      //  dataWidth is 

	short   dataWidth  =	   Get_DataChannel_Width();   



	long	yTop       =     Get_Channels_Top_Yval(        retChannelIdx   );  
	long	yBottom  =     Get_Channels_Bottom_Yval(   retChannelIdx   );  
	long   ix = 0;


	if(    yTop  >  yBottom   )
	{
		long  tmp =   yTop;

		yTop      =    yBottom;
		yBottom =    tmp;
	}
	ASSERT(  yBottom >=   yTop  );



	for(    long  yKer =  yTop;      yKer  <=  yBottom;      yKer++    )    	//  fill with the  'Y-coords'   of the kernal
	{
		yKern[ ix ]  =   yKer; 
		ix++;
	}
	ASSERT(   ix  ==  dataWidth   );







//	if(   m_duraRectPixelCount  >  dataWidth    )   
	if(    minPixelCount   >  dataWidth    )   
	{

	//	ASSERT(0 );      //  Problem...  should be LESS than dataWidth    ****** GET here from not clickin on a Note with the PlayNote Tool  7/06 

// ****  NO big deal, app deals with this OK  10/06 ****  Get here when do a quick click that does not hit anything 
// *************************************************************************
	}









	long     xStart= -1,   xEnd= -1,    xTrav= x;
	short   anyVal= 0,    gr, bl,   goodCount; 
	bool    keepGoing =  true;


	while(   keepGoing    )									//  Search backwards to find START of region
	{

		goodCount = 0;

		for(    short i= 0;     i< dataWidth;    i++    )
		{
			Read_Pixel(   xTrav,  yKern[i] ,    &( val[i]  ),  &gr,&bl   );   

			if(   val[ i ]  >   pixelThresh    )   //     m_duraRectPixelThresh   )
				goodCount++;


			totalRunTone  +=    val[ i ] ;    //  ************??/ OK if I count the  BAD,  'terminating value'  ???  
			totalReadCnt++;
		}



		if(    goodCount   >=  minPixelCount   )     //    m_duraRectPixelCount   )
		{
			xStart =  xTrav;
			xTrav =   xStart   - 1;    //  -:   backward

			if(  xTrav  < 0   )
				keepGoing =   false;
		}
		else
			keepGoing =   false;
	}




	keepGoing =   true;    //   reset for forward search	
	xTrav =  x;


	while(         keepGoing   
			  &&   xStart  >=  0    )							//  Search forwards to find END of region
	{
		goodCount = 0;

		for(    short i= 0;     i< dataWidth;    i++    )
		{
			Read_Pixel(   xTrav,  yKern[i] ,    &( val[i]  ),  &gr,&bl   );   

			if(   val[ i ]  >    pixelThresh    )     //    m_duraRectPixelThresh   )
				goodCount++;


			totalRunTone  +=    val[ i ];    //  ************??/ OK if I count the  BAD,  'terminating value'  ???  
			totalReadCnt++;
		}


		if(    goodCount  >=   minPixelCount    )     //     m_duraRectPixelCount   )
		{
			xEnd   =  xTrav;
			xTrav =   xEnd   + 1;    //  +:   forward

			if(  xTrav  >=  m_width   )
				keepGoing =   false;
		}
		else
			keepGoing =   false;
	}



																				//   return results
	if(    xStart >= 0     &&     xEnd >= 0    )
	{  
		retStartPix =   xStart;
		retEndPix   =   xEnd;


		if(   totalReadCnt  >  0    ) 
			retAverageRunBriteness  =    (short)(    totalRunTone   /   totalReadCnt    );
		else
		{  ASSERT( 0 );
			retAverageRunBriteness  =   0;
		}

		return  true;
	}
	else
	{  retStartPix =  -1;
		retEndPix   =  -1;
		retChannelIdx  =  -1;
		retAverageRunBriteness =  -2;
		return  false;
	}
}




												////////////////////////////////////////


short    TransformMap::Get_Xcolumns_Channel_Value(  long x,   int  channelIdx,   short  pixThresh,   short&  retPixCount  )
{


	  	//  RETURN:   the  'average Pixel Magnitude'   for all the pixels in the vertical column of the 'channel'

		//		retPixCount:   the number of pixels that were larger than the input  'pixThreshold'



//  CALLED from:    CompositeVerter::Transform_Column(),		  ...NO issue, it just copies derivativeMaps data directly toComposite map   

//				   	        HarmPairsTrForm::Find_ThreeBest_Scalepitch_Scores(),    ...NOT used anymore  8/02

//							HarmPairsTrForm::Get_DuraRects_HarmonicDensity_Gapped(),    ???  Gets average density for SPitchOBJ  ..no issue ??

//					 	    Detect_ChannelRegions_Absolute( ),				 ...NO filtering ??




	retPixCount = 0;


	short   channelWidth  =    Get_Channel_Width();
	short	dataWidth       =    Get_DataChannel_Width();
																										
	long	yTop         =     Get_Channels_Top_Yval(        channelIdx   );  
	long	yBottom    =     Get_Channels_Bottom_Yval(   channelIdx   );  


	short    val, gr, bl;  
	short    totalScore = 0,   numReads=0,    avg;  


	if(    yTop  >  yBottom   )		//  'sort'   to have smallest first...
	{
		long  tmp =   yTop;
		yTop      =    yBottom;
		yBottom =    tmp;
	}
	ASSERT(  yBottom >=   yTop  );




// ***** Will this work OK if  (  yTop ==  yBottom )  ???  step-debug ********************************

	for(    long y=  yTop;      y <=  yBottom;      y++    )
	{

		Read_Pixel(  x,  y,      &val,  &gr,&bl   );		  //  offset  UPWARDS from center row
		totalScore  +=   val;				numReads++;

		if(    val  >=   pixThresh   )
			retPixCount++;
	}



	avg =     totalScore  /  numReads;

	return  avg;
}



												////////////////////////////////////////


bool    TransformMap::Apply_Mask(   CString&  retErrorMesg   )
{

			//  only CALLED from    Apply_HarmonicMask_to_logDFT()  which is currently NOT USED  8/02


	retErrorMesg.Empty();


	if(   m_maskMap  ==  NULL   )
	{
		retErrorMesg =  "Apply_Mask failed,  no maskMap." ;
		return  false;
	}


	for(   short  y=0;     y < m_height;     y++    )
	{

		for(   short  x=0;     x < m_width;      x++    )
		{
															//  Only need to 'WRITE zero' where the MaskMap has a ZERO
			short   maskVal,  maskGr,  maskBl;

			m_maskMap->Read_Pixel(    x,  y,     &maskVal,  &maskGr,  &maskBl   );

			if(    maskVal  ==  0   )
				Write_Pixel(   x,  y,    0,0,0   );
		}
	}

	return  true;
}


												////////////////////////////////////////


void    TransformMap::Read_Pixel_Masked(    long x,  long y,   	short   filterCode,   short  horzKernalWidth,  
																				short *rd,  short *gr,  short *bl   )
{


	 	//  Will only return a   NON-zero 'grey' value    if the MASK does   NOT have a ZERO   for the pixel



	if(         m_maskMap  ==  NULL 
		||   ! m_maskMapHasData       )
	{




		ASSERT( 0 );     // ************* Is a big deal!!!    YES  ...I need to know if this function was called INCORRECTLY !!!!    9/03  
//  Laned here in TScribe:    9/22/03, from  DFTrow::ReConstruct_Cell_Segment( PlayTool )




		Read_Pixel(   x,  y,     rd,  gr,  bl   );
	}
	else
	{  short   maskVal,  maskGr,  maskBl;



// **** NEED to filter  the  MASK read as well ??? *******************************
		m_maskMap->Read_Pixel(    x,  y,    &maskVal,  &maskGr,  &maskBl  );

		if(    maskVal  ==  0   )
		{
			*rd=  *gr =  *bl  =    0; 
			return;
		}
// *** THINK( ? )  that the MASKING is only BY ROWS ( Harmonics are SKIPPED  FundCandids[2]  by X-column ]
//					...so  If the  coord[x, y]  is cleared for a read by the MASK, then it is ok to AVERAGE what we see in the logDFT    8/02
//					(  if it is good for   THIS-x's  Y-value,    then it is good for ALL  same  'Y-valued Pixels'   in the Horizontal-Kernal )  



		if(     filterCode  ==   ConvolutionMapFilter::NONE    )  
			Read_Pixel(   x,  y,     rd,  gr,  bl   );  
		else
			Read_Pixel_Filtered_Horz(    x,  y,     filterCode,   horzKernalWidth,      rd,  gr,  bl   );
	}
}



												////////////////////////////////////////



short	   TransformMap::Read_MaskMap_Pixel(   long x,   long y   )
{

	short   maskVal,  maskGr,  maskBl;


	if(        m_maskMap  ==  NULL 
		||   !m_maskMapHasData       )
	{

		ASSERT( 0 );     // ******* Is a big deal!!!    YES  ...I need to know if this function was called INCORRECTLY !!!!    9/03  

		return  -1;
	}


	m_maskMap->Read_Pixel(    x,  y,    &maskVal,  &maskGr,  &maskBl    );

	return   maskVal;
}





												////////////////////////////////////////


bool	 TransformMap::Load_Leading_XColumn(   long  xLead,    CString&  retErrorMesg   )
{


	retErrorMesg.Empty();



	if(    !m_inFloatmapMode   )
	{
		retErrorMesg =  "TransformMap::Load_Leading_XColumn  failed,   is NOT in FloatMap mode." ;
		return  false;
	}

	if(    m_sourceMapFloat   ==  NULL    )
	{
		retErrorMesg =  "TransformMap::Load_Leading_XColumn  failed,   sourcemap is NULL." ;
		return  false;
	}

	if(    m_verterFloatMap   ==  NULL    )
	{
		retErrorMesg =  "TransformMap::Load_Leading_XColumn  failed,   verter is NULL." ;
		return  false;
	}



	if(              xLead   ==    m_lastLoadedXColumnLeader    )        // ***** OK??? ******
		//   &&   xVirt  >  0    )    
		return  true;



	if(         xLead   !=    ( m_lastLoadedXColumnLeader  +1 )   
		&&    xLead   !=      -99     )
	{
		int  dummyBreak =  9;   // ***** REwrite ,  corrds are bad. ***********
	}





																//	a)    Prepare the SOURCE-data for read...

	/****
	if(    m_sourceMapFloat->m_inFloatmapMode    )
	{

		short   xSrcLead =    xLead    +   m_sourceMapFloat->m_readersOffset;


		if(    !m_sourceMapFloat->Load_Leading_XColumn(    xSrcLead,    retErrorMesg   )   )     //  Update the SOURCEmap first( with RECURSION )  
			return  false;
	}			//   (  might be attached to a REGULAR map,  in which case we do NOTexplicitly have to load a column  )
	***/


	if(    m_sourceMapFloat->m_inFloatmapMode    )				 //    Scalepitch 2Cand
	{
		short   xSrcLead =    xLead    +   m_sourceMapFloat->m_readersOffset;

		if(    !m_sourceMapFloat->Load_Leading_XColumn(    xSrcLead,    retErrorMesg   )   )     //  Update the SOURCEmap first( with RECURSION )  
			return  false;
	}			


	if(			m_sourceMapFloat2
		&&    m_sourceMapFloat2->m_inFloatmapMode    )			    //    Harm Pairs
	{

		short   xSrcLead =    xLead    +   m_sourceMapFloat->m_readersOffset;

		if(    !m_sourceMapFloat2->Load_Leading_XColumn(    xSrcLead,    retErrorMesg   )   )     //  Update the SOURCEmap first( with RECURSION )  
			return  false;
	}			


	if(			m_sourceMapFloat3
		&&    m_sourceMapFloat3->m_inFloatmapMode    )			   //    Fundamental
	{

		short   xSrcLead =    xLead    +   m_sourceMapFloat->m_readersOffset;

		if(    !m_sourceMapFloat3->Load_Leading_XColumn(    xSrcLead,    retErrorMesg   )   )     //  Update the SOURCEmap first( with RECURSION )  
			return  false;
	}			






																//	b)    Scroll the previous columns to the LEFT...


	for(     short  xSRC =  1;       xSRC  < m_width;        xSRC++     )
		Copy_Xcolumn(   xSRC,    (xSRC -1)    );	      //  Scroll  ( previous fetched )ColumnsValues   to  LEFT...


	Assign_Xcolumn(    ( m_width -1 ),   0  );   //  and ERASE the Last column,  cause  Transform_Column_FloatMap() does NOT ERASE 





														 	//	c)       ...write to LAST column.  

	if(   xLead  >=  0   )
		m_verterFloatMap->Transform_Column(   xLead  );	   
							//  Just allow a BLANKcolumn to enter if there is  NO real data yet in the Pipeline  ****** ??????
	



	m_lastLoadedXColumnLeader  =    xLead;

	return  true;
}




