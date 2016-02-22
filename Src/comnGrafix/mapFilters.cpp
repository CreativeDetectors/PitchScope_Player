/////////////////////////////////////////////////////////////////////////////
//
//  mapFilters.cpp   -   
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


#include <math.h>    //   for  pow()



//////////////////////////////////////////////////           
#include  "..\comnFoundation\AbstractFounda.h"  
#include  "..\comnFoundation\ListMemry.h"    
#include  "..\comnFoundation\IteratorList.h"

#include  "..\comnFoundation\myMath.h"      //  #include <FixMath.h>    ...Mac's version of Fixedpoint 
#include  "..\ComnGrafix\CommonGrafix.h"      //  #include "commonmy.h"   	

#include  "..\ComnGrafix\OffMap.h"  	
//////////////////////////////////////////////////     





#include  "mapFilters.h"

//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////



CoordShort   kernalCircBase[ 36 ] =		//  the  8 surrounding( clockwise ) neighborPixel-OFFSETS( from center ) for a 3x3 kernal of Pixels
{     
		

											   {0, -1},       

							{-1, 0},     				      {1, 0},    
											
							 					{0, 1}, 	                                  //   get this kernal for   kernalSize = 4   pixels   ( use for skeleton  algo




				            {-1, -1},                          {1, -1}, 


                            {-1, 1},                            {1, 1},                //   get this kernal for   kernalSize = 8  pixels 








							{-1, -2},     {0, -2},    {1, -2},     

				{-2, -1},											{ 2, -1},    

				{-2,  0},											{ 2,  0},     

				{-2,  1},											{ 2,  1},    

							{-1, 2},     {0, 2},    {1, 2},                       //   get this kernal for   kernalSize = 20  pixels 





				{-2, -2},											{ 2, -2},	



				{-2,  2},											{ 2,  2},	







							{-1, -3},     {0, -3},    {1, -3},     

{-3, -1},																			{ 3, -1 },

{-3,  0 },																			{ 3,  0 },

{-3,  1 },																			{ 3,  1 },	
							{-1,  3},     {0,  3},    {1,  3}     


};    





////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
  

ConvolutionMapFilter::ConvolutionMapFilter(   OffMap  *freqMap,      OffMap  *undoMap,     short filterType,     short  parm1   )
{


	//     'freqMap'    is the target map.    
	//		Now  NULL can be passed in for the 'undoMap',  and one will be automatically allocated.


	m_didAllocUndoMap =   false;
	 _UndoMap =  _Map  =  NULL; 


	if(    filterType  ==   ConvolutionMapFilter::NONE    )
	{
		ASSERT( 0 );
		return;
	}

	if(   freqMap   ==  NULL	)        
	{
		ASSERT( 0 );
		return;
	}



     _Map              =   freqMap;    

     m_filterCode   =    filterType;  

	 _Parm1           =   parm1;
     




     /***
     if(       freqMap   ==  NULL	 
		 ||   undoMap  ==  NULL    )        
            _HasUndoMap =   0;  
     else      
		   _HasUndoMap  =   1;        // show sucess...
	****/

	 if(       undoMap   !=   NULL    )
		 _UndoMap =    undoMap; 
	 else
	 {
		_UndoMap =       new    OffMap(  (short)freqMap->m_width,   (short)freqMap->m_height,    (short)freqMap->m_depth  );
		if(  _UndoMap ==  NULL )
		{
			ASSERT( 0 );
			AfxMessageBox(  "ConvolutionMapFilter::ConvolutionMapFilter failed,  could not create _UndoMap."  );
			return;
		}


		*_UndoMap  =    *freqMap;    //   copy the data

		m_didAllocUndoMap =   true;
	 }





							//  So far  m_parm2   and   m_parm3  are only used by OIL, so can set the defulat values for this here.   7/09
	m_parm2  =    4;     //   4 :  bits Color   ( for  OIL  Filter  only )
	m_parm3  =    1;     //   1:   oilLighten   ( for  OIL  Filter  only )
}  



											////////////////////////////////////////


ConvolutionMapFilter::~ConvolutionMapFilter()
{


	if(    m_didAllocUndoMap    )
	{
		if(   _UndoMap  ==  NULL   )
		{	
			//  ASSERT( 0 );      Not a problem,  user might have alread called    Release_OnFly_UndoMap()   
		}
		else
		{  delete   _UndoMap;
		}
	}
}



											////////////////////////////////////////


void    ConvolutionMapFilter::Release_OnFly_UndoMap()
{

	if(    m_didAllocUndoMap    )
	{
		if(   _UndoMap  ==  NULL   )
		{	
			ASSERT( 0 );     //    this should really not happen,  but is not a problem if it does    4/10
		}
		else
		{  delete   _UndoMap;
			 _UndoMap =  NULL; 
		}
	}
}



											////////////////////////////////////////


void   ConvolutionMapFilter::Filter()
{     

// **** INSTALL some kind of mechanism to warn user if  PARM1 ( or 2 )  need to have a non-zero value ****  8/02



	if(    m_filterCode  ==   ConvolutionMapFilter::NONE    )
	{
		ASSERT( 0 );
		return;
	}


	if(    _Map->m_depth  ==   24   )
	{
		Filter_24bit();
		return;
	}



	bool    getOutOfLoop =  false;
	short   colorStep =   0,    kernalAdjusted= 0;

	short  oilNumberOfColors =  256;


	short   oilFilterNumberOfColorBits =  3;       //       [ 2, 3,  4, or 5  ]      4: gives 4096 colors       3: gives 512 colors      2: gives 64 colors
																    //		    BAD:  5:  32768 colors( but does not look like it, not smooth, hardly wort it )

	if(   m_parm2  > 0  )    //   so far just for OIL
	{



// ****************   FIX,  this is wrong for B&W  **********************
		
		if(     m_filterCode  ==   ConvolutionMapFilter::OIL    )     // *************   NOT USED here so far for GREYSCALE  10/09  ***************
		{
			/****
			oilFilterNumberOfColorBits =    m_parm2;

			if(          oilFilterNumberOfColorBits   ==  5   )
				colorStep =  8;
			else if(   oilFilterNumberOfColorBits   ==  4   )
				colorStep =  16;
			else if(   oilFilterNumberOfColorBits   ==  3   )
				colorStep =  32;
			else if(   oilFilterNumberOfColorBits   ==  2   )
				colorStep =  64;
			else 
			{	ASSERT( 0 );   }
			*****/

			oilNumberOfColors =   (short)(	   pow(   2.0,     (double)m_parm2   )           );	

		}
	}     //   if(   m_parm2  > 0





	bool    makeBrighterOil =   false;     

	short   oilLightenSteps =  0;     //	short   colorStepsUp =   0;


	if(    m_parm3  > 0   )    //   so far just for OIL
	{

		if(     m_filterCode  ==   ConvolutionMapFilter::OIL    )
		{

//			ASSERT(   oilFilterNumberOfColorBits  > 0    );

			makeBrighterOil =    true;   

//			colorStepsUp  =     m_parm3   *   colorStep;
			oilLightenSteps  =    m_parm3;
		}
	}







	short   darkThresh =  64;    // ***** HARDWIRE *********

	short   N =  3;						 // ***** HARDWIRE *********



    short   x,y,   i,j,  max, min,   val,  rd, gr, bl;
    short   histo[ 256 ],  dx,dy,  mfp,   hld,  tp, tp2;


	ListMemry< LINESEGMENToffmap >    pixelListCircKern;  //  will autmatically deallocate   
	short     retPixelCountCircKern =  0; 



	short     blBest=0;
	double   tmpDoub0 =0.0,   tmpDoub1=0.0,    tmpDoub2=0.0;     //   just for on the fly calcs



	short   cnt =  0;
 

	short    offsetTable[ 32 ]  =   
	{  +1,-1,   +2,-2,   +3,-3,   +4,-4,  +5,-5,   +6,-6,   +7,-7,  +8,-8,   +9,-9,   +10,-10,   +11,-11,   +12,-12,   +13,-13,   +14,-14,   +15,-15,   +16,-16   };


     
     if(       _Map        ==  NULL  
		 ||   _UndoMap ==  NULL    )      
	 {
		 ASSERT( 0 );
		 return;  
	 }




	if(   _Parm1  >=   2      //  _Parm1   is the radius                       //   only do this for functions that use the circular kernal ( not gamma brighten) 
		   &&  (       m_filterCode ==ConvolutionMapFilter::OIL 
					||   m_filterCode ==ConvolutionMapFilter::DIALATE 
					||   m_filterCode ==ConvolutionMapFilter::ERODE 

					||   m_filterCode ==ConvolutionMapFilter::DIALATEsEP 
					||   m_filterCode ==ConvolutionMapFilter::ERODEsEP 

					||   m_filterCode ==ConvolutionMapFilter::BLURaDJUST 
		         )    
	  )
	{  short  radWorking  =   _Parm1;


		if(      m_filterCode ==   ConvolutionMapFilter::BLURaDJUST    )
		{	
			if(   radWorking  >=  3   )
			{
				radWorking  =    radWorking  -1;
			    ASSERT(  radWorking  >=  2  );

				OffMap::Make_Circular_Kernal(   radWorking,     retPixelCountCircKern,    pixelListCircKern   );
			}
		}
		else						
			OffMap::Make_Circular_Kernal(       radWorking,     retPixelCountCircKern,    pixelListCircKern   );



		if(   m_filterCode  ==  ConvolutionMapFilter::OIL   )   // ******************  NOT need for Greyscale ???? ************
		{

			if(  retPixelCountCircKern  >=  256    )   // are we COUNTING too many pixels than will fit in a unsigned char (  unsigned char   histoHuge[]   )
			{														   //     retPixelCountCircKern =  221   when  _Parm1 = 8 (radius ),  is the 

				/****  NOT applicable for Greyscale ????   
					//  Could FIX this so that  histoHuge[] is only used for 5-Bit Oil,    and use  histo[4097]   is used for 4-Bit and smaller... that way could have 

				if(    oilFilterNumberOfColorBits  ==   5  )    // now   UCHAR histoHuge[]   is only used for 5-Bit Oil  where we canonly count to 255 (the size of a UCHAR)
				{
					ASSERT( 0 );
					return;
				}
				****/

			}
		}
	}



   //  ***Now done with:  'Backup_2UndoMap()'    _UndoMap->Copy( *_Map );    //  Copy source  TO  Undomap( new SRC ), _Map is now DST
     
     
     _Map->Clear( 0 );		       //  ...and 1st erase DST	  
     

     _Map->Lock_Bits();	   
	 _UndoMap->Lock_Bits();	
     



     for(   y=0;     y< _Map->m_height;     y++    )
     { 
		 for(   x=0;     x< _Map->m_width;      x++    )       
         {  



             switch(   m_filterCode   )
             {


                 case    ConvolutionMapFilter::SHARPEN  :									     // ** SHARPEN **
                   _UndoMap->Read_Pixel(  x,     y,  &rd,  &gr,&bl  );  val=  5 * rd;
                   _UndoMap->Read_Pixel(  x-1,   y,  &rd,  &gr,&bl  );   val -=  rd;
                   _UndoMap->Read_Pixel(  x+1,   y,  &rd,  &gr,&bl  );   val -=  rd;
                   _UndoMap->Read_Pixel(  x,   y-1,  &rd,  &gr,&bl  );   val -=  rd;
                   _UndoMap->Read_Pixel(  x,   y+1,  &rd,  &gr,&bl  );   val -=  rd;

               //    val =  val /3;    // reduce volume
                 break;



                 case   ConvolutionMapFilter::BLUR  :   //  kMapFiltBLUR:  	    				         // ** BLUR **
                   _UndoMap->Read_Pixel(  x   ,  y   ,  &rd,  &gr,&bl  ); val=  rd;
                   _UndoMap->Read_Pixel(  x -1,  y   ,  &rd,  &gr,&bl  );   val +=  rd;
                   _UndoMap->Read_Pixel(  x +1,  y   ,  &rd,  &gr,&bl  );   val +=  rd;
                   _UndoMap->Read_Pixel(  x   ,  y -1,  &rd,  &gr,&bl  );   val +=  rd;
                   _UndoMap->Read_Pixel(  x   ,  y +1,  &rd,  &gr,&bl  );   val +=  rd;
                   val /=  5;              // could use BIGGER kernal( see book )
                 break;
           


                case   ConvolutionMapFilter::BLURaDJUST  :   //    input  {1 to anything }    Works with BOTH the Circular kernal   and   kernalCircBase

				   	ASSERT(  _Parm1  >=  1  );   

					_UndoMap->Read_Pixel(  x, y,    &rd,  &gr,&bl   );  
					cnt  =    1;
					val  =   rd;

					if(   _Parm1  <=  2   )
					{
						if(   _Parm1  ==  2  )
							kernalAdjusted =   8;
						else
							kernalAdjusted =   4;    //   _Parm1  ==  1


						for(   i =0;     i <   kernalAdjusted;     i++   )
						{
							if(          ( y  + kernalCircBase[ i ].Y )   <  _UndoMap->m_height     &&      (y  + kernalCircBase[ i ].Y ) >=  0     
								&&      (x  + kernalCircBase[ i ].X)    < _UndoMap->m_width        &&     (x  + kernalCircBase[ i ].X)  >=  0     )
							{
								_UndoMap->Read_Pixel(    (  x  + kernalCircBase[ i ].X  ),   (  y  + kernalCircBase[ i ].Y  ),     &rd,  &gr, &bl  );  
								val  +=  rd;
								cnt++;
							}
						}

					}   //   _Parm1  <=  2  
					else
					{																							//  use circular kernal for _Parm1  that are bigger than 2 
						for(   i = 0;      i < pixelListCircKern.Count();      i++    )
						{
							LINESEGMENToffmap&   lineSeg  =     pixelListCircKern.Get(  i  );

							for(    short  xOff =   lineSeg.xl;      xOff  <=   lineSeg.xr;       xOff++   )
							{
																												//  NEED to tell if mapread is off map
								if(      ( y + lineSeg.y) <  _UndoMap->m_height     &&     (y +  lineSeg.y) >=  0     &&      (x  + xOff)  < _UndoMap->m_width      &&    (x  + xOff)  >=  0     )
								{
									_UndoMap->Read_Pixel(    (  x  + xOff  ),     (  y  +  lineSeg.y  ),  	  &rd,  &gr, &bl  ); 
									val  +=  rd;
									cnt++;
								}					
							}   //   for(   i =
						}
					}   //    _Parm1  >  2


					ASSERT(  cnt != 0  );
					val  /=  cnt;
                break;




                 case   ConvolutionMapFilter::RELIEF  :   //   2:  	    								 // ** RELIEF **
                   _UndoMap->Read_Pixel(  x,       y,         &rd,  &gr,&bl  );   val    =  rd;
                   _UndoMap->Read_Pixel(  x +2,  y +2,    &rd,  &gr,&bl  );   val   -=  rd;
                 break;



		
                 case   ConvolutionMapFilter::OIL  :   //   3: 								         // ** OIL **

                 //  N =  2;       //	1        kernal size  1: (3x3) 	 // almost a 'median' filter

                   N =  _Parm1;   // set kernal size[ 3(7x7),  1(3x3) ]	   ...NOY necessary for new	
                  
                   for(   dx=0;    dx< 256;   dx++    )    
					   histo[ dx ] =   0;				// zero count buckets


					/*****
                   for(   dy=  y -N;       dy <=   y +N;      dy++    )            // count TIMES each tone is used in kernal
                   {
					   for(   dx=  x -N;       dx <=   x +N;       dx++    )
                       {
                           _UndoMap->Read_Pixel(   dx, dy,    &rd,  &gr,&bl    ); 
                           histo[  rd  ]++;
                       }
				   }
				   ******/
					for(   i = 0;      i < pixelListCircKern.Count();      i++    )
					{
						LINESEGMENToffmap&   lineSeg  =     pixelListCircKern.Get(  i  );

						for(    short  xOff =   lineSeg.xl;      xOff  <=   lineSeg.xr;       xOff++   )
						{
							  _UndoMap->Read_Pixel(    (  x  + xOff  ),     (  y  +  lineSeg.y  ),  	  &rd,  &gr, &bl  );  

								/***
								clr64  =     ( ( valRd >> 5 )  <<  6  )     +     ( ( gr >> 5 )   <<  3  )    +    ( bl >> 5  );    //  just use 3 bits per compnent, instead of 4

							   histo[  clr64  ]++;
							   *****/

							  if(    oilNumberOfColors  <=  0    )
							  {
								  ASSERT(0);
							  }
							  else
							  {  dx =    256 /  oilNumberOfColors;

						//		 val  =         (rd / dx)                                *   dx;
								 val  =    (   (rd / dx)  +  oilLightenSteps   )     *  dx;    //    rd / dx os almost the INDEX to which of '' colors.  So we boost it by the  'step'(oilLightenSteps)

//  *** RETHINK how this mechanism works. Basically I want to control BRITENESS and CONTRAST for image. However I keep loosing the dark blacks with a 
//                      high value for  oilLightenSteps.  I also use GAMMAbRIGHTEN to control contrast.   10/09


								 if(      val > 255 )   
									val =   255;       // test for out of bounds
								else if( val <   0 )   
									val =     0;

								 rd   =   val;
							  }


                           histo[  rd  ]++;
						}
					}

                        

                   for(   dx=dy=0;    dx <  256;    dx++    )     //   uses dy for MAXcount,   dx for toneVal
                   {
					   if(    histo[ dx ]  >  dy    )  
					   {  
						   dy   =   histo[ dx ];   
					       mfp =   dx; 
					   }
				   }                   

                   val =  mfp;                   // try 'CIRCULAR kernal' sometime
                   					// val= mfp * (short)parm2;   // AMPLIFY
              //   val  *=  2;  // AMPLIFY    **** BETTER to do this OUTSIDE of function,  or to have a INPUT  PARAMETER   8/02
                 break;
                 
                 



                 case   ConvolutionMapFilter::POSTERIZE  :   //   4:  	    							 // ** POSTERIZE **
                   N =   _Parm1;    // N is number of new shades [ 8 is default ]

                   if(( N >256 )||( N <= 0 ))     
					   N= 256;   // OPTIONAL safeguard

                   dx= 256 /N;   
                   _UndoMap->Read_Pixel( x,y, &rd,  &gr,&bl );    val=  rd;
                   val=  ( val/dx ) *dx;
                 break;                       //  N: ( 32:normal, 8:abstract )


                 case   ConvolutionMapFilter::CUTOFF  :   //    kMapFiltDARKdROP:  	       // 5, ** bottomTONE CUTOFF **
                   N = _Parm1;    
                   if(( N >256 )||( N <= 0 ))     N= 64;   // OPTIONAL safeguard
                   _UndoMap->Read_Pixel( x,y,  &rd,  &gr,&bl );    val= rd;
                   if( val < N )  val= 0;          // give zero to 'faint' freqs
                 break;                       


                 case   ConvolutionMapFilter::THRESHOLD  :   //   6:  	    						    // ** THRESHOLD **
                   N = _Parm1;   // 128 is middle, default 
                   if(( N >256 )||( N <= 0 ))     N= 128;   // OPTIONAL safeguard
                   _UndoMap->Read_Pixel( x,y, &rd,  &gr,&bl );    val= rd;
                   if( val < N )  val=   0;              // give zero to faint freqs
                   else           val= 128;     // was 255, reduce amplitude
                 break;                       


                 case   ConvolutionMapFilter::LITEN  :   //    kMapFiltLITEN:  	    				 // ** LITEN 150% **
                   _UndoMap->Read_Pixel( x,y,  &rd,  &gr,&bl );    val= rd;

              //     val=  ( val * 3 ) /2; 

					  val  =    (short)(    ( (float)val  *  (float)_Parm1  ) / 100.0     );
                 break;                     

               
                 case   ConvolutionMapFilter::DARKEN  :   //    kMapFiltDARKEN:  	    	  	    // ** DARKEN 75% **
                   _UndoMap->Read_Pixel( x,y,  &rd,  &gr,&bl );    val= rd;
                   val=  ( val * 3 ) /4; 
                 break;                     
               
               

                 case   ConvolutionMapFilter::LONEPIXCUTVERT  :   //   kMapFiltLONEPIXCUTVERT:	        // **LONEpixCUT  'VERTlines'
                   _UndoMap->Read_Pixel(  x-1,  y,   &rd,  &gr,&bl  );    val= rd;
                   _UndoMap->Read_Pixel(  x+1,  y,   &rd,  &gr,&bl  );    tp = rd;
                   
                   if(( val <  _Parm1  )&&( tp <  _Parm1  ))
                        val= 0;            // val= (val + tp) /2;  ***write AVERAGE ?????****
                   else    _UndoMap->Read_Pixel(  x,y,   &val,  &gr,&bl );   // use original value                   
                 break;
 
 

                 case   ConvolutionMapFilter::CONTRASTUP  :   //    kMapFiltCONTRASTuP:  	    	       // ** Contrast UP **
                   _UndoMap->Read_Pixel( x,y, &rd,  &gr,&bl );      val= rd;
                   val=  (  (val -128 )*5 /4  )  + 128;
                 break;                     
                 

                 case   ConvolutionMapFilter::CONTRASTDOWN  :   //    kMapFiltCONTRASTdOWN:  	    		   // **Contrast DOWN **
                   _UndoMap->Read_Pixel( x,y, &rd,  &gr,&bl );    val= rd;
                   val=  (  (val -128 )*4 /5  )  + 128;
                 break;                       


                 
                 case   ConvolutionMapFilter::SHARPENHORZ  :   //    kMapFiltVERTSHARPlow:  // ** ( subtract VERY-LOW[big average] ) sharpen VERTICAL **

                   _UndoMap->Read_Pixel(  x, y,  &rd,  &gr,&bl  );    val= 8 * rd;
                   
                   _UndoMap->Read_Pixel(  x,   y-1,  &rd,  &gr,&bl  );   val -=  rd;  // HI-bosst( ORIG - LOWpass( mean )
                   _UndoMap->Read_Pixel(  x,   y+1,  &rd,  &gr,&bl  );   val -=  rd; 
                   _UndoMap->Read_Pixel(  x,   y-2,  &rd,  &gr,&bl  );   val -=  rd; 
                   _UndoMap->Read_Pixel(  x,   y+2,  &rd,  &gr,&bl  );   val -=  rd; 
                   _UndoMap->Read_Pixel(  x,   y-3,  &rd,  &gr,&bl  );   val -=  rd; 
                   _UndoMap->Read_Pixel(  x,   y+3,  &rd,  &gr,&bl  );   val -=  rd; 
                   _UndoMap->Read_Pixel(  x,   y-4,  &rd,  &gr,&bl  );   val -=  rd; 
                   _UndoMap->Read_Pixel(  x,   y+4,  &rd,  &gr,&bl  );   val -=  rd; 
             	   
             	   val =  val / 5; 
                 break;
 



                 case   ConvolutionMapFilter::BLURHORZ  :   //    kMapFiltBLURhORZ
					
					 ASSERT(  _Parm1  >  1  );    //   _Parm1  is the   'KernalWIDTH'   to traverse		 
                   
                    _UndoMap->Read_Pixel(   x, y,    &rd,  &gr,&bl    );  
					val  =   rd;
					cnt  =    1;


				    for(   i =0;     i <  ( _Parm1 -1 );     i++   )
					{
					   _UndoMap->Read_Pixel(     (   x   + offsetTable[ i ]   ),     y,       &rd,  &gr, &bl   );  
					    val  +=   rd; 
						cnt++;
					}
                   
                   val  /=   cnt;            
                 break;


				/***
				 case   ConvolutionMapFilter::BLURhORZxx  :   //    kMapFiltBLURhORZxx:   // blur HORIZONTAL  extreme
                   
                   _UndoMap->Read_Pixel(  x   ,  y   ,  &rd,  &gr,&bl  );  val  =  4 * rd;  
                   _UndoMap->Read_Pixel(  x -1,  y   ,  &rd,  &gr,&bl  );  val +=  2 * rd; 
                   _UndoMap->Read_Pixel(  x +1,  y   ,  &rd,  &gr,&bl  );  val +=  2 * rd;
                   _UndoMap->Read_Pixel(  x -2,  y   ,  &rd,  &gr,&bl  );  val +=      rd;
                   _UndoMap->Read_Pixel(  x +2,  y   ,  &rd,  &gr,&bl  );  val +=      rd;
                   val /=  10;                                                
                 break;
				 ***/


                 case   ConvolutionMapFilter::OILHORZ  :       // ** oil HORIZONTAL **

                   ASSERT(  _Parm1 > 2  );									// almost a 'median' filter

                   N = _Parm1;  // set kernal size  [ 3(7x1), 2(5x1), 1(3x1) ]		
                  
                   for( dx=0; dx< 256; dx++ )    
					   histo[ dx ]= 0;						// zero count buckets

                   dy =  y;   //  for( dy= y-N;  dy<= y+N;  dy++ )  
                     

                   for(   dx= x-N;    dx<= x+N;    dx++    )  // count TIMES each tone is used in kernal
                   {
                       _UndoMap->Read_Pixel( dx,dy,  &rd,  &gr,&bl ); 
                       histo[  rd  ]++;
                   }
                   
                   for(   dx=dy=0;    dx< 256;    dx++    )   // uses dy for MAXcount,   dx for toneVal
				   {
					   if(   histo[ dx ]  >  dy   )  
					   {  
						   dy   =  histo[ dx ];    
					       mfp =  dx;
					   }
				   }
                   
                   val =  mfp;                   // try 'CIRCULAR kernal' sometime  *************************
                 break;



                 case   ConvolutionMapFilter::MEDIAN  :   //   15:  	     // ** median **
                   // N = _Parm1;  // set kernal size from VIEW		
                  
                   N = 5;       // kernal size( always ODD )  // ** MEDIAN(round) **
                   
                   _UndoMap->Read_Pixel(  x,     y,     &rd,  &gr,&bl);
                   histo[ 0 ]=  rd;
                   
                   _UndoMap->Read_Pixel(  x,     y -1,  &rd,  &gr,&bl);   
                   histo[ 1 ]=  rd;
                   
                   _UndoMap->Read_Pixel(  x,     y +1,  &rd,  &gr,&bl);  
                   histo[ 2 ]=  rd;
                   
                   _UndoMap->Read_Pixel(  x -1,  y,     &rd,  &gr,&bl); 
                   histo[ 3 ]=  rd;
                   
                   _UndoMap->Read_Pixel(  x +1,  y,     &rd,  &gr,&bl); 
                   histo[ 4 ]=  rd;
                 
                 
                 /*****************
                   N = 5;       // kernal size( always ODD )  // ** MEDIAN(horz) **
                   for( i=0; i< N; i++ )   		// get kernals values
                        histo[ i ]=  _UndoMap->Read_Pixel(  x + i -(N/2),  y, &rd,  &gr,&bl); 
                 ******************/
                 

                   for(  i= 0;   i< N-1;   ++i  )    // sort in order( BUBBLE SORT )
                     { for(  j= N-1;   i< j;   --j  )   
                         { if(  histo[ j-1 ]  <  histo[ j ]   )
                               { tp           =   histo[ j-1 ]; 
                                 histo[ j-1 ] =   histo[ j ];
                                 histo[ j ]   =   tp;
                               }
                         } 
                     }
                   val = histo[ N/2 ];  // use MIDDLE value( careful on centering ) 
                 break;

                 
                 
                 case   ConvolutionMapFilter::MEDIANHORZ  :   //    kMapFiltHORZMEDIAN:  	          // ** median HORIZONTAL **
                   N = _Parm1;       // 3, 4, 5:  kernal size( ODD or EVEN ) 
                   
                   for(  i=0;  i< N;   i++  )   		// get kernals values
                     {
                       _UndoMap->Read_Pixel(  x + i -(N/2),  y,   &rd,  &gr,&bl ); 
                       histo[ i ]=  rd;  
                     }

                   for(  i= 0;   i< N-1;   ++i  )    // sort in order( BUBBLE SORT )
                     { for(  j= N-1;   i< j;   --j  )   
                         {  if(  histo[ j-1 ]  <  histo[ j ]   )
                               { tp           =   histo[ j-1 ]; 
                                 histo[ j-1 ] =   histo[ j ];
                                 histo[ j ]   =   tp;
                               }
                         } 
                     }

                   val =  histo[ N/2 ];  // use MIDDLE value( careful on centering ) 
                 break;


                
                 case   ConvolutionMapFilter::MILDVERTSHARP  :   //   kMapFiltVERTSHARPhi:	// **HI ( subtract MIDfreqs [small average] )  sharpen VERTICAL **

                   _UndoMap->Read_Pixel(  x, y,  &rd,  &gr,&bl  );    val =  2 * rd;
                   
                   _UndoMap->Read_Pixel(  x,   y-1,  &rd,  &gr,&bl  );   val -=  rd;   // HI-bosst( ORIG - LOWpass( mean )
                   _UndoMap->Read_Pixel(  x,   y+1,  &rd,  &gr,&bl  );   val -=  rd; 
             	        //  val =   (val* 2) /3;     // reduce volume
                 break;




                 case   ConvolutionMapFilter::DIALATEhORZ  :       //    kMapFiltDIALATEhORZ:	   18 ** DIALATE  -HORZ
                   
					ASSERT(  _Parm1  >  1  );    //   _Parm1  is the   'KernalWIDTH'   to traverse in finding a maximum
					 
					max= 0;     
                   
                    _UndoMap->Read_Pixel(  x,   y,  &val,  &gr,&bl  );  
                    if(   val  >  max   )  
					   max=  val;

				    for(   i =0;     i <  ( _Parm1 -1 );     i++   )
					{
					   _UndoMap->Read_Pixel(    (  x  + offsetTable[ i ]  ) ,   y,       &val,  &gr, &bl  );  

					   if(   val  >  max    )  
						   max =   val;
					}

                    val =  max;
                 break;




                 case   ConvolutionMapFilter::ERODEvERT  :   //   kMapFiltERODEvERT:	               // 19 **ERODE  -VERT
                   min= 255;     
                   
                   _UndoMap->Read_Pixel(  x, y,    &val,  &gr,&bl  );
                   if( val < min  )  min=  val;
                   _UndoMap->Read_Pixel(  x, y-1,  &val,  &gr,&bl  );
                   if( val < min  )  min=  val;
                   _UndoMap->Read_Pixel(  x, y+1,  &val,  &gr,&bl  );
                   if( val < min  )  min=  val;
                   
                   val=  min;
                 break;



                 case   ConvolutionMapFilter::ERODEhORIZ  :   //  
                   min= 255;     
                   
                   _UndoMap->Read_Pixel(   x,      y,    &val,  &gr,&bl  );
                   if( val < min  )  min=  val;
                   _UndoMap->Read_Pixel(   x -1,  y,   &val,   &gr,&bl   );
                   if( val < min  )  min=  val;
                   _UndoMap->Read_Pixel(   x +1, y,   &val,   &gr,&bl   );
                   if( val < min  )  min=  val;
                   
                   val=  min;
                 break;




                 case   ConvolutionMapFilter::LONEPIXCUTHORZ  :   //    kMapFiltLONEPIXCUTHORZ:	      // **LONEpixCUT  'HORZlines'
                   _UndoMap->Read_Pixel(  x,  y -1,   &rd,  &gr,&bl  );    val= rd;
                   _UndoMap->Read_Pixel(  x,  y +1,   &rd,  &gr,&bl  );    tp = rd;
                   
                   if(( val < _Parm1 )&&( tp < _Parm1  ))
                    // val=  ( val + tp ) / 2;    // ***write AVERAGE ?????****
                        val= 0;
                   else   
					   _UndoMap->Read_Pixel(  x,y,   &val,  &gr,&bl );   // use original value                   
                 break;





                 case   ConvolutionMapFilter::DIALATE  :   //    kMapFiltDIALATE:	              // 21 ** DIALATE  
                   max= 0;     

				   	ASSERT(  _Parm1  >=  1  );    //   _Parm1  is the   'KernalWIDTH'   to traverse in finding a maximum


					_UndoMap->Read_Pixel(  x,   y,  &val,  &gr,&bl  );  
                    if(   val  >  max   )  
					   max=  val;

				    for(   i =0;     i <  ( _Parm1  /* -1 */  );     i++   )
					{
					   _UndoMap->Read_Pixel(    (  x  + kernalCircBase[ i ].X  ),   (  y  + kernalCircBase[ i ].Y  ), 		&val,  &gr, &bl  );  

					   if(   val  >  max    )  
						   max =   val;
					}
                   
                   val=  max;
                 break;




                case   ConvolutionMapFilter::DIALATEsEP  :   //    kMapFiltDIALATE:	              // 21 ** DIALATE  

                    min  =     9999;   //  25555;     //   255;   
					max =   0;   

					getOutOfLoop =  false;

				   	ASSERT(  _Parm1  >  1  );    //   _Parm1  is the   'KernalWIDTH'   to traverse in finding a maximum


					for(   i = 0;      i < pixelListCircKern.Count();      i++    )
					{

						if(   getOutOfLoop   )
							break;


						LINESEGMENToffmap&   lineSeg  =     pixelListCircKern.Get(  i  );

						for(    short  xOff =   lineSeg.xl;      xOff  <=   lineSeg.xr;       xOff++   )
						{
																											//  NEED to tell if mapread is off map
							if(      ( y + lineSeg.y) <  _UndoMap->m_height     &&     (y +  lineSeg.y) >=  0     &&      (x  + xOff)  < _UndoMap->m_width      &&    (x  + xOff)  >=  0     )
							{

							    _UndoMap->Read_Pixel(    (  x  + xOff  ),     (  y  +  lineSeg.y  ),  	  &val,  &gr, &bl  );  

								/***********************************  TEMP   From  Filter_24bit
							    tp =   valRd  +   gr   +   bl;  

								if(   tp <  min   )  
								{
									min =   tp;   
									rdBest= valRd;    grBest=  gr;     blBest=  bl;							
								}		

								if(     valRd ==  0   &&    gr ==  0   &&   bl ==  0     )    //  if the pixel is black, we have found an ABSOLUTE Min, so leave early
								{
									rdBest= valRd;    grBest=  gr;     blBest=  bl;
									getOutOfLoop =  true;
									break;
								}
								***/
								if(   val  >  max    )      //   see above for grey
									max =   val;

							}
					
						}   //   for(   i =
					}
			

					/***
                    valRd  =    rdBest;    //  the 3  write value
				    grOrig  =   grBest;  
					blOrig  =    blBest;
					***/
					val=  max;
                 break;
 




                case   ConvolutionMapFilter::ERODE  :   //    kMapFiltDIALATE:	              // 21 ** DIALATE  
                   min= 255;       

				   	ASSERT(  _Parm1  >  1  );    //   _Parm1  is the   'KernalWIDTH'   to traverse in finding a maximum


					_UndoMap->Read_Pixel(  x,   y,  &val,  &gr,&bl  );  
					 if( val < min  )  min=  val;


				    for(   i =0;     i <  _Parm1;     i++   )
					{
					   _UndoMap->Read_Pixel(    (  x  + kernalCircBase[ i ].X  ),   (  y  + kernalCircBase[ i ].Y  ), 	  &val,  &gr, &bl  );  

						if( val < min  )  min=  val;
					}
                   
                   val=  min;
                 break;



 
                 case   ConvolutionMapFilter::ERODEsEP    :   //   for 24 bit color this ERODES because it favors WHITE pixels for the kernal decision  11/08

                    max=   -9999;    //   0;      Foce it to set a value
                    min= 255;    

					getOutOfLoop =  false;

				   	ASSERT(  _Parm1  >  1  );    //   _Parm1  is the   'KernalWIDTH'   to traverse in finding a maximum

																											

					for(   i = 0;      i < pixelListCircKern.Count();      i++    )                  // for my new   CircularKernal Generator    10/08
					{

						if(   getOutOfLoop   )
							break;


						LINESEGMENToffmap&   lineSeg  =     pixelListCircKern.Get(  i  );

						for(    short  xOff =   lineSeg.xl;      xOff  <=   lineSeg.xr;       xOff++   )
						{

																											//  NEED to tell if mapread is off map
							if(      ( y + lineSeg.y) <  _UndoMap->m_height     &&     (y +  lineSeg.y) >=  0     &&      (x  + xOff)  < _UndoMap->m_width      &&    (x  + xOff)  >=  0     )
							{

							    _UndoMap->Read_Pixel(    (  x  + xOff  ),     (  y  +  lineSeg.y  ),  	  &val,  &gr, &bl  );  

								/****
							    tp =   valRd  +   gr   +   bl;  
							    if(   tp >  max    )  
							    {
									max =   tp;   
									rdBest= valRd;    grBest=  gr;     blBest=  bl;
								 }

								if(   valRd ==  255   &&    gr ==  255   &&   bl ==  255  )  //  if the pixel is white, we have found an ABSOLUTE Max, so leave early
								{
									rdBest= valRd;    grBest=  gr;     blBest=  bl;
									getOutOfLoop =  true;
									break;
								}
								*****/
								if(   val  <  min    )  
									min=  val;

							}	
						}   //   for(   i =
					}


                   /***
                    valRd  =    rdBest;    //  the 3  write value
				    grOrig  =   grBest;  
					blOrig  =    blBest;
					***/
                    val=  min;
                 break;

 


                case    ConvolutionMapFilter::PREWITT  :									     //  new    ** Edge Detect **   for Grey maps

                   _UndoMap->Read_Pixel(  x,  y,    &rd,  &gr, &bl  );   
				   
				    val =  0;     
			//	   val =  rd;     
					

                   _UndoMap->Read_Pixel(  x-1,   y- 1,    &rd,  &gr, &bl  );				val -=  rd;          //  this does VERTICAL lines,  should have one for Horizontal too
                   _UndoMap->Read_Pixel(  x-1,       y,    &rd,  &gr, &bl  );				val -=  2*rd;   
                   _UndoMap->Read_Pixel(  x-1,   y +1,    &rd,  &gr, &bl  );			val -=  rd;      

                   _UndoMap->Read_Pixel(  x+1,   y- 1,    &rd,  &gr, &bl  );			val +=  rd;     
                   _UndoMap->Read_Pixel(  x+1,       y,    &rd,  &gr, &bl  );			val +=  2*rd;  
                   _UndoMap->Read_Pixel(  x+1,   y +1,    &rd,  &gr, &bl  );			val +=  rd;     


					if(        val  >  255   )
						val =   255;
					else if(  val  <    0    ) 
						val =     0;

					val  =    255  -  val;     //  take the opposite to get a Negative image... that is one with White as the background color
                 break;





                 case   ConvolutionMapFilter::HOLEPLUGhORZ  :   //    kMapFiltHOLEPLUGhORZ:	      // **HolePlug 'HORZ lines'
                   
                   darkThresh =  40;   // ***** ADJUST ********
                                        // [64:not bad] [128 too hard???
                   
                   _UndoMap->Read_Pixel(  x,y,   &hld,   &gr,&bl  ); 
                   val =  hld;       // just use old value if fail
                   
                   if(  hld  <  darkThresh  )
                     {
                       _UndoMap->Read_Pixel(  x  -1,  y,   &tp,   &gr,&bl  );  
                       _UndoMap->Read_Pixel(  x +1,  y,   &tp2,  &gr,&bl  );    
                    
                       if((  tp > darkThresh  )&&(  tp2 > darkThresh  )) 
                          {    
                            val =  (tp + tp2) /2;  // Write AVERAGE value if Plug with lighter pixel
                          }
                     } 
                 break;
 

                 
                 case   ConvolutionMapFilter::HOLEPLUGvERT  :   //    kMapFiltHOLEPLUGvERT:	      // **HolePlug 'VERT lines'
                   
                   darkThresh= 128;   // ******** ADJUST *********
                   
                   _UndoMap->Read_Pixel(  x,y,   &rd,  &gr,&bl  );  hld = rd;
                   val =  hld;       // just use old value if fail
                   
                   if(  hld  <  darkThresh  )
                     {
                       _UndoMap->Read_Pixel(  x,  y -1,   &rd,  &gr,&bl  );   val= rd;
                       _UndoMap->Read_Pixel(  x,  y +1,   &rd,  &gr,&bl  );    tp= rd;
                    
                       if((  val > darkThresh  )&&(  tp > darkThresh  )) 
                              val =  (val+ tp ) /2;  // Write AVERAGE value if Plug with lighter pixel
                     } 
                 break;
 
 



                 case   ConvolutionMapFilter::BLURvERT  :   //    kMapFiltBLURvERT:  	            //  ** blur VERtical **
                   _UndoMap->Read_Pixel(  x,       y,     &rd,  &gr,&bl  );   val  = rd;
                   _UndoMap->Read_Pixel(  x,  y  -1,     &rd,  &gr,&bl  );   val +=  rd; 
                   _UndoMap->Read_Pixel(  x,  y +1,     &rd,  &gr,&bl  );   val +=  rd; 
                   val /= 3;              
                 break;
 



                 case   ConvolutionMapFilter::HOLEPLUGhORZ2X  :   //    kMapFiltHOLEPLUGhORZ2X:	  // **HolePlug [ 2 PixWide] 'HORZ lines'
                   
                   darkThresh =  40;   // ***** ADJUST ********

                                        // [64:not bad] [128:too hard]
                   
                   _UndoMap->Read_Pixel(  x,y,   &hld,   &gr,&bl  ); 
            
              //  _UndoMap->Read_Pixel(  x,y,   &hld2,  &gr,&bl  ); 
                   
                   val =  hld;       // just WRITE old value later,  if fail
                
                   if(  hld <  darkThresh  )
                     {
                       _UndoMap->Read_Pixel(  x -2,  y,   &tp,    &gr,&bl  );   
                       _UndoMap->Read_Pixel(  x +1,  y,   &tp2,   &gr,&bl  );   
                    
                       if((  tp > darkThresh  )&&(  tp2 > darkThresh  )) 
                          {  
                             val =  (tp + tp2 ) /2;     // Write AVERAGE value if Plug with lighter pixel
                          
                          //  *** write 'SECOND pixel'( x+1)  right away....*******
                          
                             if(      val > 255 )   val= 255;       // test for out of bounds
                             else if( val <   0 )   val=   0;
                             _Map->Write_Pixel(  x -1,  y,   val,val,val  ); // NO problem of REwrite if write BEHIND...
                                          //  _Map->Write_Pixel(  x +1,  y,   val,val,val  ); // NO... wil be RE-written on next X ????  by 'hld' and val
                          }
                     } 
                 break;




                 case   ConvolutionMapFilter::HOLEPLUGhORZ3X  :   //    kMapFiltHOLEPLUGhORZ3X:	  // **HolePlug [ 3 PixWide] 'HORZ lines'
                   
                   darkThresh =  40;   // ***** ADJUST ********
                                        // [64:not bad] [128:too hard]
                   
                   _UndoMap->Read_Pixel(  x,y,   &hld,   &gr,&bl  ); 
             
              //   _UndoMap->Read_Pixel(  x,y,   &hld2,  &gr,&bl  ); 
                   
                   val =  hld;       // just WRITE old value later,  if fail
                
                   if(  hld <  darkThresh  )
                     {
                       _UndoMap->Read_Pixel(  x -3,  y,   &tp,    &gr,&bl  );   
                       _UndoMap->Read_Pixel(  x +1,  y,   &tp2,   &gr,&bl  );   
                    
                       if((  tp > darkThresh  )&&(  tp2 > darkThresh  )) 
                          {  
                             val =  (tp + tp2 ) /2;     // Write AVERAGE value if Plug with lighter pixel
                          
                          //  *** write '2nd & 3rd pixels'( x+1)  right away....*******
                          
                             if(      val > 255 )   val= 255;       // test for out of bounds
                             else if( val <   0 )   val=   0;
                             
                             _Map->Write_Pixel(  x -1,  y,   val,val,val  ); // NO problem of REwrite if write BEHIND...
                             _Map->Write_Pixel(  x -2,  y,   val,val,val  ); // NO problem of REwrite if write BEHIND...
                          }
                     } 
                 break;
 


                 case   ConvolutionMapFilter::LITENpARM  :   //    kMapFiltLITENpARM:	  

					_UndoMap->Read_Pixel( x,y,  &rd,  &gr, &bl  );    

					if(    _Parm1  <=  0    )
						val = rd;
					else
						val  =      (short)(       (   (long)rd * 255L  )    /    (long)_Parm1     ); 
                 break;

				 

                 case   ConvolutionMapFilter::LINEdETECThORZ  :   //  

                   hld =  0;     
                   
                   //  _UndoMap->Read_Pixel(    x,	y,     &val,  &gr,&bl  );


                   _UndoMap->Read_Pixel(    x -1,	  y,		 &val,  &gr,&bl  );
				   hld  +=		-1  *  val;

                   _UndoMap->Read_Pixel(    x +1,	 y,			&val,  &gr,&bl  );
				   hld  +=		+1  *  val;
 

//  SMALLER kernal better ??? 

              //     _UndoMap->Read_Pixel(    x -2,	  y,		 &val,  &gr,&bl  );
			   //	   hld  +=		-1  *  val;

                //     _UndoMap->Read_Pixel(    x +2,	 y,			&val,  &gr,&bl  );
				//   hld  +=		+1  *  val;
                   

                   val =    hld;			//  hld  / 2;
                 break;



               case   ConvolutionMapFilter::GAMMAbRIGHTEN  :   //    kMapFiltCONTRASTuP:  	    	       // ** Contrast UP **


                   _UndoMap->Read_Pixel(  x,y,   &rd,  &gr, &bl  );       
				   //    valRd= rd;    **********8  IS this needed???    10/09 



				    ASSERT(    _Parm1  >=  0     &&      _Parm1  <=  300   );    // *******   could go higher,  >100 would make the image darker )  **************
																	  //    _Parm1 in  'tenths'   ( divide down later 

					N =  _Parm1;


					/****
					OffMap::RGB_to_HSV(      rd,  gr,  bl,      retHue,  retSaturation,  retValue   );
					****/



//					blBest  =   (short)(     (     ( (double)retValue / 255.0 )  ** 0.7   )   * 255.0                      );    ...Whoops no such operator as  **

					tmpDoub0 =    ((double)rd)   / 255.0; 

					tmpDoub1 =	   pow(  tmpDoub0,    ((double)N) / 100.0     );					  //        .4     .7         pow(  tmpDoub0,   0.7   );     ...tmpDoub0  "raised to power of"   0.7  

					blBest       =    (short)(   tmpDoub1  *  255.0  );



					/***
					if(         blBest  >  255    )
						blBest =   255;
					else if(   blBest  <    0    )
						blBest =     0;


					OffMap::HSV_to_RGB(    retHue,  retSaturation,   blBest,       rd,  gr,  bl   );

					valRd   =   rd; 
					grOrig  =   gr;    
					blOrig   =   bl;			
					***/

					val =    blBest;		
                 break;                                      



                 default:	  ASSERT( 0 );     
				 break;
               }
             

             
             if(      val > 255 )   
				 val =   255;       // test for out of bounds
             else if( val <   0 )   
				 val =     0;
     
             _Map->Write_Pixel(  x, y,   val,val,val  ); 
         }           
     }


	_Map->Unlock_Bits();	   
	_UndoMap->Unlock_Bits();	
}




											////////////////////////////////////////


void   ConvolutionMapFilter::Filter_24bit()
{     


	short   colorStep =   0;


	short   oilFilterNumberOfColorBits =  3;       //       [ 2, 3,  4, or 5  ]      4: gives 4096 colors       3: gives 512 colors      2: gives 64 colors
																    //		    BAD:  5:  32768 colors( but does not look like it, not smooth, hardly wort it )

	if(   m_parm2  > 0  )    //   so far just for OIL
	{

		if(     m_filterCode  ==   ConvolutionMapFilter::OIL    )
		{
			oilFilterNumberOfColorBits =    m_parm2;

			if(          oilFilterNumberOfColorBits   ==  5   )
				colorStep =  8;
			else if(   oilFilterNumberOfColorBits   ==  4   )
				colorStep =  16;
			else if(   oilFilterNumberOfColorBits   ==  3   )
				colorStep =  32;
			else if(   oilFilterNumberOfColorBits   ==  2   )
				colorStep =  64;
			else 
			{	ASSERT( 0 );   }
		}

	}




	bool    makeBrighterOil =   false;     
	short   colorStepsUp =   0;


	if(    m_parm3  > 0   )    //   so far just for OIL   **  NO, also get here for LITEN [ WatercolorFilter_DSTmap()   8/11   ***************  
	{

		if(     m_filterCode  ==   ConvolutionMapFilter::OIL    )
		{

			ASSERT(   oilFilterNumberOfColorBits  > 0    );

			makeBrighterOil =    true;   

			colorStepsUp =     m_parm3   *   colorStep;
		}
	}





	if(    m_filterCode  ==   ConvolutionMapFilter::NONE    )
	{
		ASSERT( 0 );
		return;
	}



	short   darkThresh =  64;    // ***** HARDWIRE *********

	short   N =  3;						 // ***** HARDWIRE *********






    short   x,y,   i,j,  max, min,     valRd,       rd, gr, bl,    rdOrig,  grOrig,  blOrig;


//  short   tp,   dx,dy,    cnt =  0;
    long   tp,   dx,dy,    cnt =  0;   //  changed  5/20/09    OK ?????



    short                histo[ 4097 ];                   //       histo[ 361 ]                      histo[  4097  ];
	unsigned char   histoHuge[  32768  ];		//    histoHuge[  4096  ];		      char    histoHuge[  4096  ];      //   short   histoHuge[  4096  ];


	jmColor    tpCompColor,    compColors [  50  ];    // *********  CAREFILL with the limit on this array ????? ************************



	long    clr64;  
	short   rdBest=0,  grBest=0,  blBest=0;

	short     retHue=0,   retSaturation=0,  retValue=0,      kernalAdjusted= 0;

	double   tmpDoub0 =0.0,   tmpDoub1=0.0,    tmpDoub2=0.0;     //   just for on the fly calcs


	bool    getOutOfLoop =  false;


	histo[ 0 ] =   0;   //  just to keep compiler happy (not want an unreferenced variable )

 

	short    offsetTable[ 32 ]  =   
	{  +1,-1,   +2,-2,   +3,-3,   +4,-4,  +5,-5,   +6,-6,   +7,-7,  +8,-8,   +9,-9,   +10,-10,   +11,-11,   +12,-12,   +13,-13,   +14,-14,   +15,-15,   +16,-16   };





	ListMemry< LINESEGMENToffmap >    pixelListCircKern;  //  will autmatically deallocate   
	short     retPixelCountCircKern; 



	if(   _Parm1  >=   2      //  _Parm1   is the radius                       //   only do this for functions that use the circular kernal ( not gamma brighten) 
		   &&  (       m_filterCode ==ConvolutionMapFilter::OIL 
					||   m_filterCode ==ConvolutionMapFilter::DIALATE 
					||   m_filterCode ==ConvolutionMapFilter::ERODE 

					||   m_filterCode ==ConvolutionMapFilter::DIALATEsEP 
					||   m_filterCode ==ConvolutionMapFilter::ERODEsEP 

					||   m_filterCode ==ConvolutionMapFilter::BLURaDJUST 
		         )    
	  )
	{  OffMap::Make_Circular_Kernal(   _Parm1,     retPixelCountCircKern,    pixelListCircKern   );


		if(   m_filterCode  ==  ConvolutionMapFilter::OIL   )
		{

			if(  retPixelCountCircKern  >=  256    )   // are we COUNTING too many pixels than will fit in a unsigned char (  unsigned char   histoHuge[]   )
			{														   //     retPixelCountCircKern =  221   when  _Parm1 = 8 (radius ),  is the 

					//  Could FIX this so that  histoHuge[] is only used for 5-Bit Oil,    and use  histo[4097]   is used for 4-Bit and smaller... that way could have 

				if(    oilFilterNumberOfColorBits  ==   5  )    // now   UCHAR histoHuge[]   is only used for 5-Bit Oil  where we canonly count to 255 (the size of a UCHAR)
				{
					ASSERT( 0 );
					return;
				}

			}
		}
	}





     
     if(       _Map        ==  NULL  
		 ||   _UndoMap ==  NULL    )      
	 {
		 ASSERT( 0 );
		 return;  
	 }


   //  ***Now done with:  'Backup_2UndoMap()'    _UndoMap->Copy( *_Map );    //  Copy source  TO  Undomap( new SRC ), _Map is now DST
     
     
     _Map->Clear( 0 );		       //  ...and 1st erase DST	  
     

     _Map->Lock_Bits();	   
	 _UndoMap->Lock_Bits();	
     



     for(   y=0;     y< _Map->m_height;     y++    )
     { 
		 for(   x=0;     x< _Map->m_width;      x++    )       
         {  


			 _UndoMap->Read_Pixel(   x, y,    &rdOrig,   &grOrig,   &blOrig  );  


             switch(   m_filterCode   )
             {


                 case   ConvolutionMapFilter::ERODE    :   //   for 24 bit color this ERODES because it favors WHITE pixels for the kernal decision  11/08

                    max=   -9999;    //   0;      Foce it to set a value

				   	ASSERT(  _Parm1  >  1  );    //   _Parm1  is the   'KernalWIDTH'   to traverse in finding a maximum


					/*****
					_UndoMap->Read_Pixel(  x,   y,    &valRd,  &gr, &bl   );  
				
//					tp  =      max3j(  valRd,   gr,   bl    );	 //  this fails for my Color  Separations, cause even if it has color, one componet might be 255
				    tp =   valRd  +   gr   +   bl;  
					if(   tp >  max   )  
					{
						max =   tp;    
						rdBest= valRd;    grBest=  gr;     blBest=  bl;
					}
					
	


				    for(   i =0;     i <   _Parm1;     i++   )
					{
					   _UndoMap->Read_Pixel(    (  x  + kernalCircBase[ i ].X  ),   (  y  + kernalCircBase[ i ].Y  ), 		&valRd,  &gr, &bl  );  

						
				//     tp  =      max3j(  valRd,   gr,   bl    );	 
					   tp =   valRd  +   gr   +   bl;  
					   if(   tp >  max   )  
						{
							max =   tp;   
							rdBest= valRd;    grBest=  gr;     blBest=  bl;
						}
					}
					****/


					//  *****   just a test for my new   CircularKernal Generator    10/08

					for(   i = 0;      i < pixelListCircKern.Count();      i++    )
					{

						LINESEGMENToffmap&   lineSeg  =     pixelListCircKern.Get(  i  );

						for(    short  xOff =   lineSeg.xl;      xOff  <=   lineSeg.xr;       xOff++   )
						{
																											//  NEED to tell if mapread is off map
							if(      ( y + lineSeg.y) <  _UndoMap->m_height     &&     (y +  lineSeg.y) >=  0     &&      (x  + xOff)  < _UndoMap->m_width      &&    (x  + xOff)  >=  0     )
							{

							    _UndoMap->Read_Pixel(    (  x  + xOff  ),     (  y  +  lineSeg.y  ),  	  &valRd,  &gr, &bl  );  

			//					tp  =      max3j(  valRd,   gr,   bl    );	 //  this fails for my Color  Separations, cause even if it has color, one componet might be 255
							    tp =   valRd  +   gr   +   bl;  
							    if(   tp >  max    )  
							    {
									max =   tp;   
									rdBest= valRd;    grBest=  gr;     blBest=  bl;
								 }
							}		
						}
					}
			//		****/



                   
                    valRd  =    rdBest;    //  the 3  write value
				    grOrig  =   grBest;  
					blOrig  =    blBest;
                 break;
 


                 case   ConvolutionMapFilter::ERODEsEP    :   //   for 24 bit color this ERODES because it favors WHITE pixels for the kernal decision  11/08

                    max=   -9999;    //   0;      Foce it to set a value

					getOutOfLoop =  false;

				   	ASSERT(  _Parm1  >  1  );    //   _Parm1  is the   'KernalWIDTH'   to traverse in finding a maximum

																											

					for(   i = 0;      i < pixelListCircKern.Count();      i++    )                  // for my new   CircularKernal Generator    10/08
					{

						if(   getOutOfLoop   )
							break;


						LINESEGMENToffmap&   lineSeg  =     pixelListCircKern.Get(  i  );

						for(    short  xOff =   lineSeg.xl;      xOff  <=   lineSeg.xr;       xOff++   )
						{

																											//  NEED to tell if mapread is off map
							if(      ( y + lineSeg.y) <  _UndoMap->m_height     &&     (y +  lineSeg.y) >=  0     &&      (x  + xOff)  < _UndoMap->m_width      &&    (x  + xOff)  >=  0     )
							{

							    _UndoMap->Read_Pixel(    (  x  + xOff  ),     (  y  +  lineSeg.y  ),  	  &valRd,  &gr, &bl  );  

			//					tp  =      max3j(  valRd,   gr,   bl    );	 //  this fails for my Color  Separations, cause even if it has color, one componet might be 255
							    tp =   valRd  +   gr   +   bl;  
							    if(   tp >  max    )  
							    {
									max =   tp;   
									rdBest= valRd;    grBest=  gr;     blBest=  bl;
								 }

								if(   valRd ==  255   &&    gr ==  255   &&   bl ==  255  )  //  if the pixel is white, we have found an ABSOLUTE Max, so leave early
								{
									rdBest= valRd;    grBest=  gr;     blBest=  bl;
									getOutOfLoop =  true;
									break;
								}
							}	
						}   //   for(   i =
					}
			//		****/

                   
                    valRd  =    rdBest;    //  the 3  write value
				    grOrig  =   grBest;  
					blOrig  =    blBest;
                 break;
 




                case   ConvolutionMapFilter::DIALATE  :   //    kMapFiltDIALATE:	              // 21 ** DIALATE  

                    min=   25555;     //   255;       

				   	ASSERT(  _Parm1  >  1  );    //   _Parm1  is the   'KernalWIDTH'   to traverse in finding a maximum

				
		
					_UndoMap->Read_Pixel(  x,   y,  &valRd,  &gr, &bl  );  

					tp  =      min3j(  valRd,   gr,   bl    );	 
					if(   tp <  min   )  
					{
						min =   tp;    
						rdBest= valRd;    grBest=  gr;     blBest=  bl;
					}
					
					
				    for(   i =0;      i <  _Parm1;     i++   )
					{
					   _UndoMap->Read_Pixel(    (  x  + kernalCircBase[ i ].X  ),   (  y  + kernalCircBase[ i ].Y  ), 	  &valRd,  &gr, &bl  );  

					   tp  =      min3j(  valRd,   gr,   bl    );	 
					    if(   tp <  min   )  
						{
							min =   tp;   
							rdBest= valRd;    grBest=  gr;     blBest=  bl;
						}
					}   
					
				

					/*****   just a test for my new   CircularKernal Generator    10/08

					for(   i = 0;      i < pixelListCircKern.Count();      i++    )
					{

						LINESEGMENToffmap&   lineSeg  =     pixelListCircKern.Get(  i  );

						for(    short  xOff =   lineSeg.xl;      xOff  <=   lineSeg.xr;       xOff++   )
						{
																											//  NEED to tell if mapread is off map
							if(      ( y + lineSeg.y) <  _UndoMap->m_height     &&     (y +  lineSeg.y) >=  0     &&      (x  + xOff)  < _UndoMap->m_width      &&    (x  + xOff)  >=  0     )
							{

							    _UndoMap->Read_Pixel(    (  x  + xOff  ),     (  y  +  lineSeg.y  ),  	  &valRd,  &gr, &bl  );  

							   tp  =      min3j(  valRd,   gr,   bl    );	 
								if(   tp <  min   )  
								{
									min =   tp;   
									rdBest= valRd;    grBest=  gr;     blBest=  bl;							
								}		
							}
						}
					}
					****/

                    valRd  =    rdBest;    //  the 3  write value
				    grOrig  =   grBest;  
					blOrig  =    blBest;
                 break;
 


                case   ConvolutionMapFilter::DIALATEsEP  :   //    kMapFiltDIALATE:	              // 21 ** DIALATE  

                    min=     9999;   //  25555;     //   255;     

					getOutOfLoop =  false;

				   	ASSERT(  _Parm1  >  1  );    //   _Parm1  is the   'KernalWIDTH'   to traverse in finding a maximum


					for(   i = 0;      i < pixelListCircKern.Count();      i++    )
					{

						if(   getOutOfLoop   )
							break;


						LINESEGMENToffmap&   lineSeg  =     pixelListCircKern.Get(  i  );

						for(    short  xOff =   lineSeg.xl;      xOff  <=   lineSeg.xr;       xOff++   )
						{
																											//  NEED to tell if mapread is off map
							if(      ( y + lineSeg.y) <  _UndoMap->m_height     &&     (y +  lineSeg.y) >=  0     &&      (x  + xOff)  < _UndoMap->m_width      &&    (x  + xOff)  >=  0     )
							{

							    _UndoMap->Read_Pixel(    (  x  + xOff  ),     (  y  +  lineSeg.y  ),  	  &valRd,  &gr, &bl  );  


				//			    tp  =      min3j(  valRd,   gr,   bl    );	  //  this fails for my Color  Separations, cause even if it has color, one componet might be 255
							    tp =   valRd  +   gr   +   bl;  

								if(   tp <  min   )  
								{
									min =   tp;   
									rdBest= valRd;    grBest=  gr;     blBest=  bl;							
								}		


								if(     valRd ==  0   &&    gr ==  0   &&   bl ==  0     )    //  if the pixel is black, we have found an ABSOLUTE Min, so leave early
								{
									rdBest= valRd;    grBest=  gr;     blBest=  bl;
									getOutOfLoop =  true;
									break;
								}
							}
					
						}   //   for(   i =
					}
			

                    valRd  =    rdBest;    //  the 3  write value
				    grOrig  =   grBest;  
					blOrig  =    blBest;
                 break;
 


				/***
                 case   ConvolutionMapFilter::OIL  :  		      // ** OIL **            ....try 'CIRCULAR kernal' sometime

                   N =  _Parm1;   // set kernal size[ 3(7x7),  1(3x3) ]		
                  
                   for(   dx=0;    dx< 361;   dx++    )    
				   {
					   histo[   dx ] =   0;				// zero count buckets
					   histo2[ dx ] =   0;
					   histo3[ dx ] =   0;
				   }

                   for(   dy=  y -N;       dy <=   y +N;      dy++    )       // count TIMES each  TARGET  'tone'   is used in kernal
                   {
					   for(   dx=  x -N;       dx <=   x +N;       dx++    )
                       {

                           _UndoMap->Read_Pixel(   dx, dy,    &rd,  &gr,  &bl    ); 
						   _UndoMap->RGB_to_HSV(    rd,  gr,  bl,     retHue,   retSaturation,   retValue   );

						    if(   retHue  ==   -1  )
								retHue =    360;		   //  use this TopSpot to hold the Undefined values.

							histo[     retHue            ]++; 	 
							histo2[   retSaturation   ]++;
							histo3[   retValue          ]++;
                       }
				   }
                        

                   for(   dx=dy=0;    dx <  361;    dx++    )     //   uses dy for MAXcount,   dx for toneVal
                   {
					   if(    histo[ dx ]  >  dy    )  
					   {  
						   dy   =   histo[ dx ];   
					       mfp =   dx; 
					   }
				   }                   
                   retHue =  mfp;           
				   
			       if(   retHue  ==   360  )
						retHue =    -1;				//   Convert back for  HSV_to_RGB()
				   

                   for(   dx=dy=0;    dx <  256;    dx++    )    
                   {
					   if(    histo2[ dx ]  >  dy    )  
					   {  
						   dy   =   histo2[ dx ];   
					       mfp =   dx; 
					   }
				   }                   
                   retSaturation =  mfp;            


                   for(   dx=dy=0;    dx <  256;    dx++    )   
                   {
					   if(    histo3[ dx ]  >  dy    )  
					   {  
						   dy   =   histo3[ dx ];   
					       mfp =   dx; 
					   }
				   }                   
                   retValue =  mfp;            

				   _UndoMap->HSV_to_RGB(    retHue,   retSaturation,   retValue,      valRd,   grOrig,   blOrig  );

                  break;
				****/

                 case   ConvolutionMapFilter::OIL  :					     // ** OIL   ( currently used in RegionDetection App     10/08  **

                   N =  _Parm1;   // set kernal size[ 3(7x7),  1(3x3) ]		    kernal size  1: (3x3) 	 // almost a 'median' filter
                  

                  for(   dx= 0;     dx <  4096;     dx++   )    //  4096  is  16 cubed.  So can use RGB as 16 seps for each. ( 4 bits for cach Component instead of 8 )
				//	   histoHuge[ dx ] =   0;				// zero count buckets
					   histo[        dx ] =   0;				   // zero count buckets,   NOW use the smaller array of SHORTS so can have a BIG PixelList in the KERNAL   7/09




           //      for(   dy=  y -N;       dy <=   y +N;      dy++    )            // count TIMES each tone is used in kernal
              //     {
				//	   for(   dx=  x -N;       dx <=   x +N;       dx++    )
              //         {
               //            _UndoMap->Read_Pixel(   dx, dy,    &rd,  &gr, &bl   ); 
				//		   clr64  =     ( ( rd >> 4 )  <<  8  )     +     ( ( gr >> 4 )   <<  4  )    +    ( bl >> 4  ); 
                //           histoHuge[  clr64  ]++;
                //       }
				//   }





					if(   oilFilterNumberOfColorBits  ==   5   )   //   5 bits per RGB component gives 4096 colors     ( 32 x 32 x 32 =  32768 )
					{

						for(   dx= 0;     dx <  32768;     dx++   )    //  32768  is  32 cubed.  So can use RGB as 16 seps for each. ( 5 bits for cach Component instead of 8 )
								histoHuge[ dx ] =   0;				// zero count buckets

						
						for(   i = 0;      i < pixelListCircKern.Count();      i++    )
						{
							LINESEGMENToffmap&   lineSeg  =     pixelListCircKern.Get(  i  );

							for(    short  xOff =   lineSeg.xl;      xOff  <=   lineSeg.xr;       xOff++   )
							{
																																		//  NEED to tell if map-read is off map
							    if(               ( y + lineSeg.y) <  _UndoMap->m_height      &&     (y +  lineSeg.y) >=  0     
									    &&      (x  + xOff)        < _UndoMap->m_width         &&     (x  + xOff)        >=  0     )
								{
								   _UndoMap->Read_Pixel(    (  x  + xOff  ),     (  y  +  lineSeg.y  ),  	  &valRd,  &gr, &bl  );  

								   clr64  =     ( ( valRd >> 3 )  <<  10  )     +     ( ( gr >> 3 )   <<  5  )    +    ( bl >> 3  ); 
								   histoHuge[  clr64  ]++;
								}

							}
						}
	                       

					   for(   dx=dy= 0;     dx <  32768;     dx++   )     //   uses dy for MAXcount,   dx for toneVal
					   {
						   if(    histoHuge[ dx ]   >  dy    )  
						   {  
							   dy     =   histoHuge[ dx ];   
							   clr64 =   dx; 
						   }
					   }                   


						blBest  =     (short)(        clr64   &   0x0000001f                 );
						grBest  =     (short)(    (  clr64   &   0x000003e0  )   >>  5   );
						rdBest  =     (short)(    (  clr64   &   0x00007c00  )   >>  10   );

						valRd  =    rdBest  <<  3;    //  the 3  RGB  write values
						grOrig  =   grBest  <<  3;  
						blOrig  =    blBest  <<  3;
					}


					else if(   oilFilterNumberOfColorBits  ==   4   )   //   4 bits per RGB component gives 4096 colors     ( 16 x 16 x 16 =  4096 )
					{
						/***
						for(   i =0;      i <  _Parm1;     i++   )    //   try circular kernal instead 
						{
						   _UndoMap->Read_Pixel(    (  x  + kernalCircBase[ i ].X  ),   (  y  + kernalCircBase[ i ].Y  ), 	   &valRd,  &gr, &bl  );  
						   clr64  =     ( ( valRd >> 4 )  <<  8  )     +     ( ( gr >> 4 )   <<  4  )    +    ( bl >> 4  ); 
						   histoHuge[  clr64  ]++;
						}        
						***/
						for(   i = 0;      i < pixelListCircKern.Count();      i++    )
						{
							LINESEGMENToffmap&   lineSeg  =     pixelListCircKern.Get(  i  );

							for(    short  xOff =   lineSeg.xl;      xOff  <=   lineSeg.xr;       xOff++   )
							{
																																		//  NEED to tell if map-read is off map
							    if(               ( y + lineSeg.y) <  _UndoMap->m_height      &&     (y +  lineSeg.y) >=  0     
									    &&      (x  + xOff)        < _UndoMap->m_width         &&     (x  + xOff)        >=  0     )
								{
								   _UndoMap->Read_Pixel(    (  x  + xOff  ),     (  y  +  lineSeg.y  ),  	  &valRd,  &gr, &bl  );  

								   clr64  =     ( ( valRd >> 4 )  <<  8  )     +     ( ( gr >> 4 )   <<  4  )    +    ( bl >> 4  ); 
								   histo[  clr64  ]++;
								}
							}
						}
	                       

					   for(   dx=dy= 0;     dx <  4096;     dx++   )     //   uses dy for MAXcount,   dx for toneVal
					   {
						   if(    histo[ dx ]   >  dy    )  
						   {  
							   dy     =   histo[ dx ];   
							   clr64 =   dx; 
						   }
					   }                   

						blBest  =     (short)(        clr64   &   0x0000000f                 );
						grBest  =     (short)(    (  clr64   &   0x000000f0  )   >>  4   );
						rdBest  =     (short)(    (  clr64   &   0x00000f00  )   >>  8   );

						valRd  =    rdBest  <<  4;    //  the 3  write value
						grOrig  =   grBest  <<  4;  
						blOrig  =    blBest  <<  4;
					}

					else if(     oilFilterNumberOfColorBits  ==   3   )     //   3 bits per RGB component gives 512 colors     ( 8 x 8 x 8 =  512 )
					{
						/***
						for(   i =0;      i <  _Parm1;     i++   )    //   try circular kernal instead 
						{
						   _UndoMap->Read_Pixel(    (  x  + kernalCircBase[ i ].X  ),   (  y  + kernalCircBase[ i ].Y  ), 		&valRd,  &gr, &bl  );  

					//	   clr64  =     ( ( valRd >> 4 )  <<  8  )     +     ( ( gr >> 4 )   <<  4  )    +    ( bl >> 4  ); 
							clr64  =     ( ( valRd >> 5 )  <<  6  )     +     ( ( gr >> 5 )   <<  3  )    +    ( bl >> 5  );    //  just use 3 bits per compnent, instead of 4

						   histo[  clr64  ]++;
						}         
						***/
						for(   i = 0;      i < pixelListCircKern.Count();      i++    )
						{
							LINESEGMENToffmap&   lineSeg  =     pixelListCircKern.Get(  i  );

							for(    short  xOff =   lineSeg.xl;      xOff  <=   lineSeg.xr;       xOff++   )
							{
							   _UndoMap->Read_Pixel(    (  x  + xOff  ),     (  y  +  lineSeg.y  ),  	  &valRd,  &gr, &bl  );  

						//	   clr64  =     ( ( valRd >> 4 )  <<  8  )     +     ( ( gr >> 4 )   <<  4  )    +    ( bl >> 4  ); 
								clr64  =     ( ( valRd >> 5 )  <<  6  )     +     ( ( gr >> 5 )   <<  3  )    +    ( bl >> 5  );    //  just use 3 bits per compnent, instead of 4

							   histo[  clr64  ]++;
							}
						}
	                       

					   for(   dx=dy= 0;     dx <  512;     dx++   )     //   uses dy for MAXcount,   dx for toneVal
					   {
						   if(    histo[ dx ]   >  dy    )  
						   {  
							   dy     =   histo[ dx ];   
							   clr64 =   dx; 
						   }
					   }                   

						blBest  =     (short)(        clr64   &   0x00000007                 );
						grBest  =     (short)(    (  clr64   &   0x00000038  )   >>  3   );
						rdBest  =     (short)(    (  clr64   &   0x000001c0  )   >>  6   );

						valRd  =    rdBest  <<  5;    //  the 3  write value
						grOrig  =   grBest  <<  5;  
						blOrig  =    blBest  <<  5;
					}

					else if(     oilFilterNumberOfColorBits  ==   2   )     //   2 bits per RGB component gives 64 colors     ( 4 x 4 x 4 =  64 )
					{
						/***
						for(   i =0;      i <  _Parm1;     i++   )    //   try circular kernal instead 
						{
						   _UndoMap->Read_Pixel(    (  x  + kernalCircBase[ i ].X  ),   (  y  + kernalCircBase[ i ].Y  ), 		&valRd,  &gr, &bl  );  
							clr64  =     ( ( valRd >> 6 )  <<  4  )     +     ( ( gr >> 6 )   <<  2  )    +    ( bl >> 6  );  
						   histo[  clr64  ]++;
						}        
						***/
						for(   i = 0;      i < pixelListCircKern.Count();      i++    )
						{
							LINESEGMENToffmap&   lineSeg  =     pixelListCircKern.Get(  i  );

							for(    short  xOff =   lineSeg.xl;      xOff  <=   lineSeg.xr;       xOff++   )
							{
							   _UndoMap->Read_Pixel(    (  x  + xOff  ),     (  y  +  lineSeg.y  ),  	  &valRd,  &gr, &bl  );  

					//			clr64  =     ( ( valRd >> 5 )  <<  6  )     +     ( ( gr >> 5 )   <<  3  )    +    ( bl >> 5  );   
								clr64  =     ( ( valRd >> 6 )  <<  4  )     +     ( ( gr >> 6 )   <<  2  )    +    ( bl >> 6  );        //  just use 2 bits per compnent

							   histo[  clr64  ]++;
							}
						}
	                       


					   for(   dx=dy= 0;     dx <  64;     dx++   )     //   uses dy for MAXcount,   dx for toneVal
					   {
						   if(    histo[ dx ]   >  dy    )  
						   {  
							   dy     =   histo[ dx ];   
							   clr64 =   dx; 
						   }
					   }                   

						
						blBest  =     (short)(        clr64   &   0x00000003                 );
						grBest  =     (short)(    (  clr64   &   0x0000000c  )   >>  2   );
						rdBest  =     (short)(    (  clr64   &   0x00000030  )   >>  4   );

						valRd  =    rdBest  <<  6;    //  the 3  write value
						grOrig  =   grBest  <<  6;  
						blOrig  =    blBest  <<  6;				
					}
					else
					{	ASSERT( 0 );   }



					if(    makeBrighterOil   )        //   Is this the best way to get what I want ?????   11/08
					{
						valRd   +=   colorStepsUp;                   //   Could set it to what ever I want. It just insures that I will have
						grOrig  +=   colorStepsUp;                    //   a white,  and that all else will fall into 2-power boundaries ( 0, 64, 128, 192 )
						blOrig   +=   colorStepsUp;						
					}

					/***   Not needed,  is done below for all cases 
					if(   valRd  >  255  )     valRd =  255;     
					if(   grOrig  >  255  )    grOrig =  255;
					****/
                 break;





                 case    ConvolutionMapFilter::SHARPEN  :									     // ** SHARPEN **

                   _UndoMap->Read_Pixel(  x,  y,    &rd,  &gr, &bl  );


				   if(   _Parm1  ==  2   )     //   2:  Much sharpening
				   {
					   valRd  =   9 * rd;      grOrig =   9 * gr;      blOrig  =   9 * bl;
						
					   _UndoMap->Read_Pixel(  x-1,    y,    &rd,  &gr, &bl  );		valRd -=  rd;    grOrig -=  gr;    blOrig -=  bl;
					   _UndoMap->Read_Pixel(  x+1,   y,    &rd,  &gr, &bl  );	   valRd -=  rd;    grOrig -=  gr;    blOrig -=  bl;
					   _UndoMap->Read_Pixel(  x,     y-1,   &rd,  &gr, &bl  );		valRd -=  rd;    grOrig -=  gr;    blOrig -=  bl;
					   _UndoMap->Read_Pixel(  x,    y+1,   &rd,  &gr, &bl  );	   valRd -=  rd;    grOrig -=  gr;    blOrig -=  bl;

					   _UndoMap->Read_Pixel(  x-1,    y-1,    &rd,  &gr, &bl  );		valRd -=  rd;    grOrig -=  gr;    blOrig -=  bl;   // also get the four corners
					   _UndoMap->Read_Pixel(  x+1,   y-1,    &rd,  &gr, &bl  );	   valRd -=  rd;    grOrig -=  gr;    blOrig -=  bl;
					   _UndoMap->Read_Pixel(  x-1,    y+1,    &rd,  &gr, &bl  );		valRd -=  rd;    grOrig -=  gr;    blOrig -=  bl;
					   _UndoMap->Read_Pixel(  x+1,   y+1,    &rd,  &gr, &bl  );	   valRd -=  rd;    grOrig -=  gr;    blOrig -=  bl;
				   }
				   else             //   DEFAULT:    _Parm1  ==  1    ...or even 0  or  -1 
				   {
					   valRd  =   5 * rd;      grOrig =   5 * gr;      blOrig  =   5 * bl;			

					   _UndoMap->Read_Pixel(  x-1,    y,    &rd,  &gr, &bl  );		valRd -=  rd;    grOrig -=  gr;    blOrig -=  bl;
					   _UndoMap->Read_Pixel(  x+1,   y,    &rd,  &gr, &bl  );	   valRd -=  rd;    grOrig -=  gr;    blOrig -=  bl;
					   _UndoMap->Read_Pixel(  x,     y-1,   &rd,  &gr, &bl  );		valRd -=  rd;    grOrig -=  gr;    blOrig -=  bl;
					   _UndoMap->Read_Pixel(  x,    y+1,   &rd,  &gr, &bl  );	   valRd -=  rd;    grOrig -=  gr;    blOrig -=  bl;
				   }
                 break;




                 case   ConvolutionMapFilter::BLUR  :   //  kMapFiltBLUR:  	    				         // ** BLUR **
                   _UndoMap->Read_Pixel(  x   ,       y   ,  &rd,  &gr,&bl  );   valRd =   rd;     grOrig   =  gr;    blOrig   =  bl;

					/***
                   _UndoMap->Read_Pixel(  x -1,      y   ,  &rd,  &gr,&bl  );   valRd +=  rd;    grOrig +=  gr;    blOrig +=  bl;
                   _UndoMap->Read_Pixel(  x +1,     y   ,  &rd,  &gr,&bl  );   valRd +=  rd;    grOrig +=  gr;    blOrig +=  bl;
                   _UndoMap->Read_Pixel(  x   ,   y -1,  &rd,  &gr,&bl  );      valRd +=  rd;    grOrig +=  gr;    blOrig +=  bl;
                   _UndoMap->Read_Pixel(  x   ,   y +1,  &rd,  &gr,&bl  );     valRd +=  rd;    grOrig +=  gr;    blOrig +=  bl;
                   valRd /=  5;	    grOrig /=  5;	   blOrig /=  5;	   		 // could use BIGGER kernal( see book )
				   ***/
                   _UndoMap->Read_Pixel(  x -1,      y   ,  &rd,  &gr,&bl  );   valRd +=  rd;    grOrig +=  gr;    blOrig +=  bl;
                   _UndoMap->Read_Pixel(  x +1,     y   ,  &rd,  &gr,&bl  );   valRd +=  rd;    grOrig +=  gr;    blOrig +=  bl;
                   _UndoMap->Read_Pixel(  x   ,   y -1,  &rd,  &gr,&bl  );      valRd +=  rd;    grOrig +=  gr;    blOrig +=  bl;
                   _UndoMap->Read_Pixel(  x   ,   y +1,  &rd,  &gr,&bl  );     valRd +=  rd;    grOrig +=  gr;    blOrig +=  bl;

                   _UndoMap->Read_Pixel(  x -1,      y -1   ,  &rd,  &gr,&bl  );   valRd +=  rd;    grOrig +=  gr;    blOrig +=  bl;    // also get the four corners
                   _UndoMap->Read_Pixel(  x +1,     y -1   ,  &rd,  &gr,&bl  );   valRd +=  rd;    grOrig +=  gr;    blOrig +=  bl;
                   _UndoMap->Read_Pixel(  x -1,      y +1   ,  &rd,  &gr,&bl  );   valRd +=  rd;    grOrig +=  gr;    blOrig +=  bl;
                   _UndoMap->Read_Pixel(  x +1,     y +1   ,  &rd,  &gr,&bl  );   valRd +=  rd;    grOrig +=  gr;    blOrig +=  bl;
 
                   valRd /=  9;	    grOrig /=  9;	   blOrig /=  9;	   		
                 break;
           


                case   ConvolutionMapFilter::BLURaDJUST  :   //    input  {1 to anything }    Works with BOTH the Circular kernal   and   kernalCircBase

				   	ASSERT(  _Parm1  >=  1  );   

					_UndoMap->Read_Pixel(  x, y,    &rd,  &gr,&bl   );     valRd =   rd;     grOrig   =  gr;    blOrig   =  bl;
					cnt  =    1;


					if(   _Parm1  <=  2   )
					{
						if(   _Parm1  ==  2  )
							kernalAdjusted =   8;
						else
							kernalAdjusted =   4;    //   _Parm1  ==  1


						for(   i =0;     i <   kernalAdjusted;     i++   )
						{
							if(          ( y  + kernalCircBase[ i ].Y )   <  _UndoMap->m_height     &&      (y  + kernalCircBase[ i ].Y ) >=  0     
								&&      (x  + kernalCircBase[ i ].X)    < _UndoMap->m_width        &&     (x  + kernalCircBase[ i ].X)  >=  0     )
							{
								_UndoMap->Read_Pixel(    (  x  + kernalCircBase[ i ].X  ),   (  y  + kernalCircBase[ i ].Y  ),     &rd,  &gr, &bl  );  

								valRd +=  rd;    grOrig +=  gr;    blOrig +=  bl;								
								cnt++;
							}
						}

					}   //   _Parm1  <=  2  
					else
					{																							//  use circular kernal for _Parm1  that are bigger than 2 
						for(   i = 0;      i < pixelListCircKern.Count();      i++    )
						{
							LINESEGMENToffmap&   lineSeg  =     pixelListCircKern.Get(  i  );

							for(    short  xOff =   lineSeg.xl;      xOff  <=   lineSeg.xr;       xOff++   )
							{
																												//  NEED to tell if mapread is off map
								if(      ( y + lineSeg.y) <  _UndoMap->m_height     &&     (y +  lineSeg.y) >=  0     &&      (x  + xOff)  < _UndoMap->m_width      &&    (x  + xOff)  >=  0     )
								{
									_UndoMap->Read_Pixel(    (  x  + xOff  ),     (  y  +  lineSeg.y  ),  	  &rd,  &gr, &bl  );

									valRd +=  rd;    grOrig +=  gr;    blOrig +=  bl;
									cnt++;
								}					
							}   //   for(   i =
						}
					}   //    _Parm1  >  2


					ASSERT(  cnt != 0  );
					valRd /=  cnt;	    grOrig /=  cnt;	   blOrig /=  cnt;      //    val  /=  cnt;
                break;




                 case    ConvolutionMapFilter::PREWITT  :									     // ** Edge Detect **

                   _UndoMap->Read_Pixel(  x,  y,    &rd,  &gr, &bl  );   
				   
				    valRd =  0;      grOrig =  0;      blOrig =  0;
			//	   valRd =  rd;      grOrig =  gr;      blOrig =  bl;
					
                   _UndoMap->Read_Pixel(  x-1,   y- 1,    &rd,  &gr, &bl  );		   valRd -=  rd;    grOrig -=  gr;    blOrig -=  bl;
                   _UndoMap->Read_Pixel(  x-1,       y,    &rd,  &gr, &bl  );		   valRd -=  2*rd;    grOrig -=  2*gr;    blOrig -=  2*bl;
                   _UndoMap->Read_Pixel(  x-1,   y +1,    &rd,  &gr, &bl  );	   valRd -=  rd;    grOrig -=  gr;    blOrig -=  bl;

                   _UndoMap->Read_Pixel(  x+1,   y- 1,    &rd,  &gr, &bl  );		valRd +=  rd;    grOrig +=  gr;    blOrig +=  bl;
                   _UndoMap->Read_Pixel(  x+1,       y,    &rd,  &gr, &bl  );		valRd +=  2*rd;    grOrig +=  2*gr;    blOrig +=  2*bl;
                   _UndoMap->Read_Pixel(  x+1,   y +1,    &rd,  &gr, &bl  );		valRd +=  rd;    grOrig +=  gr;    blOrig +=  bl;


					valRd  =   255  -  valRd;     //  take the opposite to get a Negative image... that is one with White as the background color
					grOrig =   255  -  grOrig;
					blOrig  =   255  -  blOrig;
                 break;






                 case   ConvolutionMapFilter::RELIEF  :   //   2:  	    								        // ** RELIEF **
                   _UndoMap->Read_Pixel(  x,       y,         &rd,  &gr,&bl  );   valRd    =  rd;
                   _UndoMap->Read_Pixel(  x +2,  y +2,    &rd,  &gr,&bl  );   valRd   -=  rd;
                 break;
                 



                 case   ConvolutionMapFilter::POSTERIZE  :   //   4:  	    							 // ** POSTERIZE **

                   N =   _Parm1;    // N is number of new shades [ 8 is default ]

					_UndoMap->Read_Pixel(    x, y,	     &valRd,  &gr, &bl  );  


					if(   N  ==   5   )   //   5 bits per RGB component gives  32768  colors     ( 32 x 32 x 32 =  32768 )
					{
						clr64  =     ( ( valRd >> 3 )  <<  10  )     +     ( ( gr >> 3 )   <<  5  )    +    ( bl >> 3  ); 

						blBest  =     (short)(        clr64   &   0x0000001f                 );
						grBest  =     (short)(    (  clr64   &   0x000003e0  )   >>  5   );
						rdBest  =     (short)(    (  clr64   &   0x00007c00  )   >>  10   );

						blBest  +=  1;    //  make a little lighter so can have white
						grBest  +=  1;  
						rdBest  +=  1;   

						valRd  =    rdBest  <<  3;    //  the 3  write value
						grOrig  =   grBest  <<  3;  
						blOrig  =    blBest  <<  3;
					}

					else if(   N  ==   4   )   //   4 bits per RGB component gives 4096 colors     ( 16 x 16 x 16 =  4096 )
					{
						clr64  =     ( ( valRd >> 4 )  <<  8  )     +     ( ( gr >> 4 )   <<  4  )    +    ( bl >> 4  ); 

						blBest  =     (short)(        clr64   &   0x0000000f                 );
						grBest  =     (short)(    (  clr64   &   0x000000f0  )   >>  4   );
						rdBest  =     (short)(    (  clr64   &   0x00000f00  )   >>  8   );

						blBest  +=  1;    //  make a little lighter so can have white
						grBest  +=  1;  
						rdBest  +=  1;   

						valRd  =    rdBest  <<  4;    //  the 3  write value
						grOrig  =   grBest  <<  4;  
						blOrig  =    blBest  <<  4;
					}

					else if(     N  ==   3   )     //   3 bits per RGB component gives 512 colors     ( 8 x 8 x 8 =  512 )
					{
						clr64  =     ( ( valRd >> 5 )  <<  6  )     +     ( ( gr >> 5 )   <<  3  )    +    ( bl >> 5  );    //  just use 3 bits per compnent, instead of 4

						blBest  =     (short)(        clr64   &   0x00000007                 );
						grBest  =     (short)(    (  clr64   &   0x00000038  )   >>  3   );
						rdBest  =     (short)(    (  clr64   &   0x000001c0  )   >>  6   );

						blBest  +=  1;    //  make a little lighter so can have white
						grBest  +=  1;  
						rdBest  +=  1;   

						valRd  =    rdBest  <<  5;    //  the 3  write value
						grOrig  =   grBest  <<  5;  
						blOrig  =    blBest  <<  5;
					}

					else if(     N  ==   2   )     //   2 bits per RGB component gives 64 colors     ( 4 x 4 x 4 =  64 )
					{
						clr64  =     ( ( valRd >> 6 )  <<  4  )     +     ( ( gr >> 6 )   <<  2  )    +    ( bl >> 6  );        //  just use 2 bits per compnent

						blBest  =     (short)(        clr64   &   0x00000003                 );
						grBest  =     (short)(    (  clr64   &   0x0000000c  )   >>  2   );
						rdBest  =     (short)(    (  clr64   &   0x00000030  )   >>  4   );

						blBest  +=  1;    //  make a little lighter so can have white
						grBest  +=  1;  
						rdBest  +=  1;   

						valRd  =    rdBest  <<  6;    //  the 3  write value
						grOrig  =   grBest  <<  6;  
						blOrig  =    blBest  <<  6;
					}
					else
					{	ASSERT( 0 );   }


					/***   Not needed,  is done below for all cases 
					if(   valRd  >  255  )    valRd =  255;     // a white,  and that all else will fall into 2-power boundaries ( 0, 64, 128, 192 )
					if(   grOrig  >  255  )    grOrig =  255;
					if(   blOrig  >  255  )    blOrig =  255;
					****/
                 break;                       //  N: ( 32:normal, 8:abstract )





                 case   ConvolutionMapFilter::CUTOFF  :   //    kMapFiltDARKdROP:  	       // 5, ** bottomTONE CUTOFF **
                   N = _Parm1;    
                   if(( N >256 )||( N <= 0 ))     N= 64;   // OPTIONAL safeguard
                   _UndoMap->Read_Pixel( x,y,  &rd,  &gr,&bl );    valRd= rd;
                   if( valRd < N )  valRd= 0;          // give zero to 'faint' freqs
                 break;                       


                 case   ConvolutionMapFilter::THRESHOLD  :   //   6:  	    						    // ** THRESHOLD **
                   N = _Parm1;   // 128 is middle, default 
                   if(( N >256 )||( N <= 0 ))     N= 128;   // OPTIONAL safeguard
                   _UndoMap->Read_Pixel( x,y, &rd,  &gr,&bl );    valRd= rd;
                   if( valRd < N )  valRd=   0;              // give zero to faint freqs
                   else           valRd= 128;     // was 255, reduce amplitude
                 break;                       





                 case   ConvolutionMapFilter::LITEN  :   //    kMapFiltLITEN:  	   ****** FIX for   24 bit color and  RenderMan::WatercolorFilter_DSTmap  

                   _UndoMap->Read_Pixel( x,y,   &rd,  &gr, &bl );        valRd= rd;    grOrig =  gr;    blOrig =  bl;

  
					if(    _Parm1  <=  0    )   // if no input value
				    {
						valRd   =   ( valRd * 3 ) /2;		  // ** LITEN 150% **
						grOrig  =   ( grOrig * 3 ) /2;		
						blOrig  =   ( blOrig * 3 ) /2;		
				    }
				    else
				    {  valRd   =    (short)(    ( (float)valRd   *  (float)_Parm1  ) / 100.0     );       //   8/11   Good for 
						grOrig  =    (short)(    ( (float)grOrig  *  (float)_Parm1  ) / 100.0     );
						blOrig   =    (short)(    ( (float)blOrig  *  (float)_Parm1  ) / 100.0     );
				    }
                 break;                     
               




                 case   ConvolutionMapFilter::DARKEN  :   //    kMapFiltDARKEN:  	    	  	 
                   _UndoMap->Read_Pixel( x,y,  &rd,  &gr,&bl );    valRd= rd;
                   valRd=  ( valRd * 3 ) /4;                                                      // ** DARKEN 75% **
                 break;                     
               



                 case   ConvolutionMapFilter::CONTRASTUP  :   //    kMapFiltCONTRASTuP:  	    	       // ** Contrast UP **
                   _UndoMap->Read_Pixel( x,y, &rd,  &gr,&bl );      valRd= rd;
                   valRd=  (  (valRd -128 )*5 /4  )  + 128;
                 break;                                      

                 case   ConvolutionMapFilter::CONTRASTDOWN  :   //    kMapFiltCONTRASTdOWN:  	    		   // **Contrast DOWN **
                   _UndoMap->Read_Pixel( x,y, &rd,  &gr,&bl );    valRd= rd;
                   valRd=  (  (valRd -128 )*4 /5  )  + 128;
                 break;                       




                 case   ConvolutionMapFilter::GAMMAbRIGHTEN  :   //    kMapFiltCONTRASTuP:  	    	       // ** Contrast UP **

                   _UndoMap->Read_Pixel(  x,y,   &rd,  &gr, &bl  );       valRd= rd;



				    ASSERT(    _Parm1  >=  0     &&      _Parm1  <=  300   );    // *******   could go higher,  >100 would make the image darker )  **************
																	  //    _Parm1 in  'tenths'   ( divide down later 

					N =  _Parm1;


					OffMap::RGB_to_HSV(      rd,  gr,  bl,      retHue,  retSaturation,  retValue   );



//					blBest  =   (short)(     (     ( (double)retValue / 255.0 )  ** 0.7   )   * 255.0                      );    ...Whoops no such operator as  **

					tmpDoub0 =    ((double)retValue)   / 255.0; 

					tmpDoub1 =	   pow(  tmpDoub0,    ((double)N) / 100.0     );					  //        .4     .7         pow(  tmpDoub0,   0.7   );     ...tmpDoub0  "raised to power of"   0.7  

					blBest       =    (short)(   tmpDoub1  *  255.0  );


					if(         blBest  >  255    )
						blBest =   255;
					else if(   blBest  <    0    )
						blBest =     0;


					OffMap::HSV_to_RGB(    retHue,  retSaturation,   blBest,       rd,  gr,  bl   );


					valRd   =   rd; 
					grOrig  =   gr;    
					blOrig   =   bl;
                 break;                                      



                case   ConvolutionMapFilter::SATURATIONbOOST  :   //    kMapFiltCONTRASTuP:  	    	     

                   _UndoMap->Read_Pixel(  x,y,   &rd,  &gr, &bl  );       valRd= rd;



				    ASSERT(    _Parm1  >=  0     &&      _Parm1  <=  300   );    // *******   could go higher,  >100 would make the image darker )  **************
																	  //    _Parm1 in  'tenths'   ( divide down later 

					N =  _Parm1;


					OffMap::RGB_to_HSV(      rd,  gr,  bl,      retHue,  retSaturation,  retValue   );



//					blBest  =   (short)(     (     ( (double)retValue / 255.0 )  ** 0.7   )   * 255.0                      );    ...Whoops no such operator as  **

					tmpDoub0 =    ((double)retSaturation)   / 255.0; 

					tmpDoub1 =	   pow(  tmpDoub0,    ((double)N) / 100.0     );					  //        .4     .7         pow(  tmpDoub0,   0.7   );     ...tmpDoub0  "raised to power of"   0.7  

					blBest       =    (short)(   tmpDoub1  *  255.0  );


					if(         blBest  >  255    )
						blBest =   255;
					else if(   blBest  <    0    )
						blBest =     0;


					OffMap::HSV_to_RGB(    retHue,   blBest,   retValue,       rd,  gr,  bl   );


					valRd   =   rd; 
					grOrig  =   gr;    
					blOrig   =   bl;
                 break;                                      






                 case   ConvolutionMapFilter::MEDIAN  :   //   15:  	     // ** median **
                   // N = _Parm1;  // set kernal size from VIEW		
                  

                   N = 9;       // kernal size( always ODD )   ...gives a square
                   
                   _UndoMap->Read_Pixel(    x, y,     &rd,  &gr, &bl   );
					OffMap::RGB_to_HSV(     rd,  gr,  bl,      retHue,  retSaturation,  retValue   );    //  Now going to load with the HSV value of pixels color
						compColors[ 0 ].red     =   retHue;
						compColors[ 0 ].green  =   retSaturation;
						compColors[ 0 ].blue    =   retValue;             //   histo[ 0 ]=  rd;

                   _UndoMap->Read_Pixel(  x,     y -1,  &rd,  &gr, &bl  );   
 					OffMap::RGB_to_HSV(     rd,  gr,  bl,      retHue,  retSaturation,  retValue   );
						compColors[ 1 ].red     =   retHue;
						compColors[ 1 ].green  =   retSaturation;
						compColors[ 1 ].blue    =   retValue;             
                   
                   _UndoMap->Read_Pixel(  x,     y +1,  &rd,  &gr, &bl  );  
					OffMap::RGB_to_HSV(     rd,  gr,  bl,      retHue,  retSaturation,  retValue   );
 						compColors[ 2 ].red     =   retHue;
						compColors[ 2 ].green  =   retSaturation;
						compColors[ 2 ].blue    =   retValue;           
                  
                   _UndoMap->Read_Pixel(  x -1,  y,     &rd,  &gr, &bl  ); 
					OffMap::RGB_to_HSV(     rd,  gr,  bl,      retHue,  retSaturation,  retValue   );
 						compColors[ 3 ].red     =   retHue;
						compColors[ 3 ].green  =   retSaturation;
						compColors[ 3 ].blue    =   retValue;             
                  
                   _UndoMap->Read_Pixel(  x +1,  y,     &rd,  &gr, &bl  ); 
					OffMap::RGB_to_HSV(     rd,  gr,  bl,      retHue,  retSaturation,  retValue   );
 						compColors[ 4 ].red     =   retHue;
						compColors[ 4 ].green  =   retSaturation;
						compColors[ 4 ].blue    =   retValue;            
																								///////////////////////////


                   _UndoMap->Read_Pixel(  x -1,     y -1,  &rd,  &gr, &bl  );                                         //  now get the 4 corners
 					OffMap::RGB_to_HSV(     rd,  gr,  bl,      retHue,  retSaturation,  retValue   );
						compColors[ 5 ].red     =   retHue;
						compColors[ 5 ].green  =   retSaturation;
						compColors[ 5 ].blue    =   retValue;             //                   histo[ 5 ]=  rd;


                   _UndoMap->Read_Pixel(  x -1,     y +1,  &rd,  &gr, &bl  );   
 					OffMap::RGB_to_HSV(     rd,  gr,  bl,      retHue,  retSaturation,  retValue   );
						compColors[ 6 ].red     =   retHue;
						compColors[ 6 ].green  =   retSaturation;
						compColors[ 6 ].blue    =   retValue;                            //    histo[ 6 ]=  rd;

                   _UndoMap->Read_Pixel(  x +1,     y -1,  &rd,  &gr, &bl  );     
 					OffMap::RGB_to_HSV(     rd,  gr,  bl,      retHue,  retSaturation,  retValue   );
						compColors[ 7 ].red     =   retHue;
						compColors[ 7 ].green  =   retSaturation;
						compColors[ 7 ].blue    =   retValue;             //                   histo[ 7 ]=  rd;

                   _UndoMap->Read_Pixel(  x +1,     y +1,  &rd,  &gr, &bl  );   
 					OffMap::RGB_to_HSV(     rd,  gr,  bl,      retHue,  retSaturation,  retValue   );
						compColors[ 8 ].red     =   retHue;
						compColors[ 8 ].green  =   retSaturation;
						compColors[ 8 ].blue    =   retValue;                       //         histo[ 8 ]=  rd;


					/***
                   for(  i= 0;    i< N-1;   ++i  )    // sort in order( BUBBLE SORT )
                     { 
						 for(    j= N-1;    i< j;    --j   )   
                         { 
							 if(  histo[ j-1 ]  <  histo[ j ]   )
                               { 
								   tp                =   histo[ j-1 ]; 
                                   histo[ j-1 ]   =   histo[ j ];
                                   histo[ j ]      =   tp;
                               }
                         } 
                     }

                   valRd =   histo[  N/2  ];  // use MIDDLE value( careful on centering ) 
					****/

                  for(  i= 0;    i< N-1;   ++i  )    //   sort  by  VALUE(intensity)      in order( BUBBLE SORT )
                     { 
						 for(    j= N-1;    i< j;    --j   )   
                         { 

//							 if(              histo[ j-1 ]            <             histo[ j ]    )
							 if(    compColors[ j-1 ].blue     <  compColors[ j ].blue    )
                             { 
								   /****
								   tp                =   histo[ j-1 ]; 
                                   histo[ j-1 ]   =   histo[ j ];
                                   histo[ j ]      =   tp;
								   ***/
								   tpCompColor        =  compColors[ j-1 ]; 
                                   compColors[ j-1 ] =   compColors[ j ];
                                   compColors[ j ]    =   tpCompColor;
                             }
                         } 
                     }



               //    valRd =   histo[  N/2  ];             //    " N/2" :  use MIDDLE value( careful on centering the array

				  	OffMap::HSV_to_RGB(    compColors[ N/2 ].red,    compColors[ N/2 ].green,   compColors[ N/2 ].blue,       rd,  gr,  bl    );
					valRd   =   rd; 
					grOrig  =   gr;    
					blOrig   =   bl;
                 break;
                				 


                 default:    ASSERT( 0 );     break;
               }
             


             
             if(      valRd > 255 )   
				 valRd =   255;       // test for out of bounds
             else if( valRd <   0 )   
				 valRd =     0;
                  
             if(      grOrig > 255 )   
				 grOrig =   255;      
             else if( grOrig <   0 )   
				 grOrig =     0;

             if(      blOrig > 255 )   
				 blOrig =   255;      
             else if( blOrig <   0 )   
				 blOrig =     0;


             _Map->Write_Pixel(  x, y,    valRd,   grOrig,  blOrig   ); 
         }           
     }


	_Map->Unlock_Bits();	   
	_UndoMap->Unlock_Bits();	
}





////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
  

ContrastingMapFilter::ContrastingMapFilter(     OffMap&  offMap,    short  calcMode,     double  valueSpread,   
																					                        double  biggestDifference   )
			                    :  m_offMap(  offMap  ),   m_calcMode( calcMode ),    m_valueSpread( valueSpread  ),
																				m_biggestDiff(  biggestDifference  )
{


//	valueSpread settings:  [  see  FundamentalTemplateVerter  in   AnalyzerAudio::Do_Transform()   ]  
//
//
//	   1.0035[  botCut= 127, way too contrasty  ]   
//	   1.0030[  botCut= 110,  REAL contrasty ]       1.0025[ 93, very contrasty ]    
//	   1.0020[  74,  not bad,  little contrasty  ]        1.0015[ 55,  Best? ]          1.0010[ 37,  Nice ]     
//	   1.0005[  19,  too flat( too many horz lines )  ]




//	ASSERT(  calcMode ==  0   );
//		calcMode should always be  0, ( mode = 1 has never been tested )


	m_diffTableEntries =   0;
}  


											////////////////////////////////////////


short   ContrastingMapFilter::Get_New_GreyValue(  short  oldGreyVal   )
{     

	if(          oldGreyVal ==  0  )
		return      0;
	else if(   oldGreyVal >= 255   )
		return   255;


	short    retGreyValue  =     m_finalTable[  oldGreyVal  ];
	return   retGreyValue;
}



											////////////////////////////////////////


bool   ContrastingMapFilter::Filter_For_Contrast(  CString&  retErrorMesg  )
{     

															//  the big Kahuna
	retErrorMesg.Empty();


	if(    m_offMap.m_depth  !=   8   )
	{
		retErrorMesg  =   "ContrastingMapFilter::Filter  failed,  is not a 8 bit bitmap." ;
		return  false;
	}



	if(        m_calcMode  ==   0   )
	{
		if(     ! Make_ColorTable_Squared(  retErrorMesg  )     )
			return   false;
	}
	else if(  m_calcMode  ==   1   )
	{
		if(     ! Make_ColorTable_Linear(  retErrorMesg  )     )
			return   false;
	}
	else
		ASSERT( 0 );




    short   x,y,    val,  rd, gr, bl;
 
	m_offMap.Lock_Bits();	   



	for(   y=0;     y< m_offMap.m_height;     y++    )
	{ 
		for(   x=0;     x< m_offMap.m_width;      x++    )       
		{  

			m_offMap.Read_Pixel(    x,  y,   &rd,  &gr,&bl   );  

			val  =      Get_New_GreyValue(  rd   );
             

             if(      val > 255 )   
				 val =    255;         // test for out of bounds
             else if( val <   0 )   
				 val =      0;
    

             m_offMap.Write_Pixel(   x, y,   val,val,val    ); 
		}           
	}



	m_offMap.Unlock_Bits();	   

	return  true;
}




											////////////////////////////////////////


bool    ContrastingMapFilter::Make_ColorTable_Squared(  CString&  retErrorMesg  )
{

	
//  different  m_valueSpread   vals:

		//	   1.0035[  botCut = 127, way too contrasty  ]   
		//	   1.0030[  botCut= 110,  REAL contrasty ]       1.0025[ 93, very contrasty ]    
		//	   1.0020[  74,  not bad,  little contrasty  ]        1.0015[ 55,  Best? ]          1.0010[ 37,  Nice ]     
		//	   1.0005[  19,  too flat( too many horz lines )  ]


	retErrorMesg.Empty();


	if(   m_biggestDiff   <  1.0  )
	{
		retErrorMesg  =   "ContrastingMapFilter::Make_ColorTable_Squared  failed,   biggestDiff must be 1 or greater." ;
		return  false;
	}



	for(    short i= 0;     i< 256;     i++    )
		m_diffTable[ i ]   =   0.0;

	for(    short i= 0;     i< 256;     i++    )
		m_finalTable[ i ]   =   0;



																				//    a)    Make the DIFFERENCE array


	double   diffTrav =    1.0;     //  ***** USE A LOWER VALUE ???? ( INPUT AS PARM ??  ) ***********


	short  lasti = 0;

	for(  short  i= 0;     i<  256;     i++   )
	{	

		diffTrav  =     diffTrav   *   m_valueSpread;		 //   'MULTIPLY'  recursively


		m_diffTable[ i ] =     diffTrav;


		if(    (short)( m_diffTable[ i ] )   >=    (short)m_biggestDiff    )        //  only need for Grey Pixel  values to 256
		{	
			lasti =  i;   // *****  NEW,  2005  jpm
			break;
		}
	}


	m_diffTableEntries =   lasti;







																				//    b)    Make the FINAL lookUp  colortable

	short    diffIdx =    m_diffTableEntries  -1,  firstZero = -1;

	double  val,    prevVal =  255.0;


	m_finalTable[ 255 ] =    255;



	for(  short  i=  254;    i >= 0;      i--    )
	{

		if(    diffIdx  >=   0    )
		{
			val =     prevVal    -    m_diffTable[  diffIdx  ];
		}
		else
		{  val =     prevVal    -     m_diffTable[  0  ];     //  Apply  'constant difference'  after this point
		}



		if(     (short)val  >  0    )
			m_finalTable[ i ] =   (short)val;
		else
		{  m_finalTable[ i ] =      0;

			if(   firstZero  ==  -1  ) 
				firstZero =   i;
		}
		



		prevVal =    val;

		diffIdx--;
	}



																	//    c)    Fill in the bottom zero-values with flat response


										 //  Fill in the lower table's zeros with small ammounts to give continuous 
										//   grey for CALCULATION from the map


	short   modVal =   3;   //   ****  ADJUST ****

	short   travVal =   1,   backFillIdx =  -1;


	if(    firstZero  >   0   )    
	{

		for(   short i= ( modVal +1 );     i<  256;     i++   )
		{	

			if(    m_finalTable[ i ]   >=    travVal    )   //  When the  'travVal' catches up with existing values,  stop
			{
				backFillIdx =   i;
				break;
			}

		
			m_finalTable[ i ]  =     travVal;


			if(     ( i  %  modVal )   ==   0     )
				travVal++;
		}
	}




	if(   backFillIdx   >   128   )
		ASSERT( 0 );    // Something is very wrong if that much backfill??   7/06
	


	return  true;      //  put breakpoint here to see the new,  CONTRAST  color tables 
}




											////////////////////////////////////////


bool    ContrastingMapFilter::Make_ColorTable_Linear(  CString&  retErrorMesg  )
{



	retErrorMesg.Empty();


	if(   m_biggestDiff   <  1.0  )
	{
		retErrorMesg  =   "ContrastingMapFilter::Make_ColorTable_Linear  failed,   biggestDiff must be 1 or greater." ;
		return  false;
	}



	for(    short i= 0;     i< 256;     i++    )
		m_diffTable[ i ]   =   0.0;

	for(    short i= 0;     i< 256;     i++    )
		m_finalTable[ i ]   =   0;


																				//    a)    Make the DIFFERENCE array


	double   diffTrav =   m_biggestDiff,   lastDiff; 


	m_diffTable[ 255 ] =    m_biggestDiff;

	lastDiff =   m_biggestDiff;



	for(  short  i=  254;    i >= 0;      i--    )
	{

		diffTrav  =     diffTrav  -  m_valueSpread;        // **** SUBTRACT  recursively  ****



		if(   diffTrav  >  0.0   )
		{
			m_diffTable[ i ]  =     diffTrav;
			lastDiff              =     diffTrav;
		}
		else
		{  		
			if(   lastDiff  >  0.0   )
				m_diffTable[ i ] =    lastDiff;    //     =    0.0;   better??????
			else
			{  ASSERT( 0 );
				m_diffTable[ i ] =    0.0;
			}

		}
	}


	m_diffTableEntries =   255;




																				//    b)    Make the FINAL lookUp  colortable

	short    diffIdx =    m_diffTableEntries  -1;

	double  val,    prevVal =  255.0;


	m_finalTable[ 255 ] =    255;



	for(  short i=  254;    i >= 0;      i--    )
	{


		if(    diffIdx  >=   0    )
		{
			val =     prevVal    -    m_diffTable[  diffIdx  ];
		}
		else
		{   val =     prevVal    -    m_diffTable[  0  ];     //  Apply  'constant difference'  after this point
		}



		if(     (short)val  >  0    )
			m_finalTable[ i ] =   (short)val;
		else
			m_finalTable[ i ] =      0;
		


		prevVal =    val;

		diffIdx--;
	}


	return  true;
}






////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
 

OnTheFlyMapFilter::OnTheFlyMapFilter(    OffMap&  targMap,   short parm1,   short parm2,   short parm3   )   :   m_targMap( targMap)
{


				//  this is an INTERMEDIATE   PARENT class   for my new scripted map ops   4/10

				//   'OnTheFly'    because it automatically creates an UndoMap ( for the source)

	 m_UndoMap =   NULL;    

	 m_Parm1 =   parm1;
	 m_Parm2 =   parm2;
	 m_Parm3 =   parm3;


	m_UndoMap =       new    OffMap(  (short)targMap.m_width,   (short)targMap.m_height,    (short)targMap.m_depth  );
	if(  m_UndoMap ==  NULL )
	{
		ASSERT( 0 );
		AfxMessageBox(  "OnTheFlyMapFilter::OnTheFlyMapFilter  failed,  could not create m_UndoMap."  );
		return;
	}


	*m_UndoMap  =    targMap;    //   copy the  bits and color table
}  



											////////////////////////////////////////


OnTheFlyMapFilter::~OnTheFlyMapFilter()
{

//	if(    m_didAllocUndoMap    )
//	{

		if(   m_UndoMap  ==  NULL   )
		{	
			//  ASSERT( 0 );      Not a problem,  user might have alread called    Release_OnFly_UndoMap()   
		}
		else
		{  delete   m_UndoMap;
		}

//	}
}


											////////////////////////////////////////


void    OnTheFlyMapFilter::Release_OnFly_UndoMap()
{

//	if(    m_didAllocUndoMap    )
//	{

		if(   m_UndoMap  ==  NULL   )
		{	
			ASSERT( 0 );     //    this should really not happen,  but is not a real problem if it does    4/10
		}
		else
		{  delete   m_UndoMap;
			 m_UndoMap =  NULL; 
		}

//	}
}





////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
 

EdgeDetectingMapFilter::EdgeDetectingMapFilter(    OffMap&  targMap,   short kernalSize,   short parm2,   short parm3   )  
																							:  OnTheFlyMapFilter(   targMap,     kernalSize,   parm2,   parm3  )
{
}  

											////////////////////////////////////////


EdgeDetectingMapFilter::~EdgeDetectingMapFilter()   
{
		//  think I need this just so  OnTheFlyMapFilter::~OnTheFlyMapFilter()   will  get called    4/10
}  



											////////////////////////////////////////


void	  EdgeDetectingMapFilter::Filter()
{

				//   Needs a   8-bit greyscale .BMP  

				//   Almost the same results as PhotoShop's  "Find Edges"  filter
	
				//	 The code for this algo was taken from  "Computer Imaging Recipes In C" by Mylar and Weeks   ( pp.  127  )


				//    m_Parm1   can be   { 1  or anything higher    ...see below   }



	short  blacknessBoostFact =   9;      //  6     *****  ADJUST ********   {  5 -  10 }        ********????? WANT to make this an input parm ???   4/10



	CString   retErrorMesg;   //   want it to return an error ???  


	retErrorMesg.Empty();


	if(	   m_UndoMap  ==   NULL    )
	{
		AfxMessageBox(   "EdgeDetectingMapFilter::Filter failed,   m_UndoMap is  NULL."   );
		return;
	}


	if(    m_Parm1  <=  0  )
	{
		AfxMessageBox(   "EdgeDetectingMapFilter::Filter failed,   m_Parm1 is bad."   );
		return;
	}


	long  width   =   m_UndoMap->m_width;
	long  height =    m_UndoMap->m_height;


	m_targMap.Clear( 255 );						//  Erase the DST maps to white,  they will hold the results




	OffMap*  	tempMap =       new    OffMap(    (short)width,    (short)height,    (short)m_UndoMap->m_depth   );
	if(  tempMap ==  NULL )
	{
		AfxMessageBox(  "EdgeDetectingMapFilter::Filter  failed,  could not create tempMap."  );
		return;
	}

	tempMap->Clear( 255 );


	/*****   Book says that for greyScale should also do an Erode,  but it just makes lines too big

	OffMap*  	extraMap =       new    OffMap(  (short)m_sourceMapOversize->m_width,   (short)m_sourceMapOversize->m_height,    (short)m_sourceMapOversize->m_depth  );
	if(  extraMap ==  NULL )
	{
		retErrorMesg  =  "Outline_Morph_Filter_DSTmap failed,  could not create extraMap." ;
		return  false;
	}
	extraMap->Clear( 255 );
	****/




	if(    m_Parm1   <=  2     )
	{
		short  kernalCount =  4;

		if(          m_Parm1 ==  1   )    //  need to change the parms for   DIALATE   as opposed to   DIALATESP
			kernalCount  =  4;
		else if(   m_Parm1  ==  2   )
			kernalCount  =  8;
		else   { ASSERT( 0 );    }

		ConvolutionMapFilter   dialate(     tempMap,     m_UndoMap,          ConvolutionMapFilter::DIALATE,         kernalCount   );       //  m_UndoMap  is the source
		dialate.Filter();        //   puts more WHITE pixels into the image  ( makes grey shapes SMALLER...     really is like erode in book )
	}
	else
	{  ConvolutionMapFilter   dialate(     tempMap,     m_UndoMap,          ConvolutionMapFilter::DIALATEsEP,     ( m_Parm1 -1)   );   
	    dialate.Filter();																																		      //    -1:   Want to start at radius 2
	}






	short   rdUndo,  grUndo,  blUndo,   rdTemp, grTemp,  blTemp;    
	short   pixelDiff;


	for(  long y = 0;    y < height;    y++   )
	{
		for(  long x = 0;    x < width;    x++   )
		{

			m_UndoMap->Read_Pixel(    x, y,      &rdUndo,   &grUndo,   &blUndo   );       //   "Image"  in book

			tempMap->Read_Pixel(        x, y,      &rdTemp,  &grTemp,   &blTemp   );       //   "Filter"  in book


			short  undoBlackness =    255  -   rdUndo;
			short  tempBlackness =    255  -   rdTemp;



//			short  blackDiff  =       undoBlackness  -   tempBlackness;
			short  blackDiff  =    (  undoBlackness  -   tempBlackness  )   *  blacknessBoostFact;

			if(         blackDiff   > 255   )
				blackDiff =  255;
			else if(  blackDiff   <     0   )
				blackDiff =  0;



			pixelDiff   =     255  -  blackDiff;

			if(         pixelDiff   > 255   )
				pixelDiff =  255;
			else if(  pixelDiff   <     0   )
				pixelDiff =  0;

			m_targMap.Write_Pixel(    x, y,     pixelDiff, pixelDiff, pixelDiff   );
		}
	}

	
	if(  tempMap !=  NULL )
		delete  tempMap;
}







////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
 

SkeletonMapFilter::SkeletonMapFilter(    OffMap&  targMap,   short toneDetectThreshold,   short maxIters,   short fineLineMode   )  
																							:  OnTheFlyMapFilter(   targMap,     toneDetectThreshold,   maxIters,   fineLineMode  )
{
}  


											////////////////////////////////////////


void	  SkeletonMapFilter::Filter()
{

		//   Needs a   8-bit greyscale .BMP     [    BEST(?)   if it has been  THRESHOLDED so it only has values of   { 0, 255 }      

		//   The code for this algo was adapted from  "Computer Imaging Recipes In C" by Mylar and Weeks   ( pp.  134  )




	short    darkerFact        =    3;     //     4    **** ADJUST ******    {  2 -  8  ?? }    4[dark]



	short    kernalSizeLocal =    4;      //    always???    ..or is ADJUSTING worth it ?????????  When it is bigger, it seems like I just get a mess.  





	CString   retErrorMesg;   //   want it to return an error ???  

	retErrorMesg.Empty();


	short    detectThreshold  =    m_Parm1;
	short    threshBoundary;
	bool     fineLineModeForThresh;   


	if(	   m_UndoMap  ==   NULL    )
	{
		AfxMessageBox(   "SkeletonMapFilter::Filter failed,   m_UndoMap is  NULL."   );
		return;
	}

	
	if(    m_Parm1  <=  0  )
	{
		AfxMessageBox(   "SkeletonMapFilter::Filter failed,   m_Parm2 is bad ( Detection Tone Threshold )."   );
		return;
	}

	if(    m_Parm2  <=  0  )
	{
		AfxMessageBox(   "SkeletonMapFilter::Filter failed,   m_Parm2 is bad ( maxIters)."   );
		return;
	}

	if(    m_Parm3  <  0  )
	{
		AfxMessageBox(   "SkeletonMapFilter::Filter failed,   m_Parm3 is bad (fine Line mode)."   );
		return;
	}



	if(   m_Parm3  ==   0   )
		fineLineModeForThresh =   false;
	else 
	{
		fineLineModeForThresh =   true;
		threshBoundary   =   m_Parm1;
	}






	if(   m_targMap.m_depth   !=  8    )    //  might be ok for a 1-bit map.  try it later  4/10
	{
		retErrorMesg.Format(   "Do NOT attempt Skeleton on anything other than an  8-bit Gray bitmap ( this has depth = %d  )." ,   m_targMap.m_depth  );
		AfxMessageBox(  retErrorMesg  );
		return;
	}



	long  width   =   m_targMap.m_width;
	long  height =    m_targMap.m_height;


	m_targMap.Clear( 255 );	




	OffMap*  	tempMap =       new    OffMap(  (short)width,   (short)height,    (short)m_targMap.m_depth  );
	if(  tempMap ==  NULL )
	{
		AfxMessageBox(   "Skeleton_Filter_DSTmap failed,  could not create tempMap."   );
		return;
	}
	tempMap->Clear( 255 );


	OffMap*  	extraMap =       new    OffMap(  (short)width,   (short)height,    (short)m_targMap.m_depth  );
	if(  extraMap ==  NULL )
	{
		AfxMessageBox(  "Skeleton_Filter_DSTmap failed,  could not create tempMap."  );
		return;
	}
	extraMap->Clear( 255 );





	short  iterCount =  0;
	bool   stillHasPixelsOn =  true;



	while(   stillHasPixelsOn   )
	{

		short   rdUndo,  grUndo,  blUndo,   rdExtra, grExtra,  blExtra;     


		stillHasPixelsOn =  false;    //  must INIT for fail on each iter



		ConvolutionMapFilter   dialate(     tempMap,     m_UndoMap,          ConvolutionMapFilter::DIALATE,     kernalSizeLocal   );   //    m_undoMapOversize  is the source
		dialate.Filter();        //   puts more WHITE pixels into the image  ( makes grey shapes SMALLER...     really is like erode in book )


		ConvolutionMapFilter   erode(     extraMap,      tempMap,               ConvolutionMapFilter::ERODE,         kernalSizeLocal   );   //    m_undoMapOversize  is the source
		erode.Filter();          //   puts more BLACK pixels into the image   ( makes grey shapes BIGGER...     really is like dialate in book )




		for(  long y = 0;    y < height;    y++   )
		{
			for(  long x = 0;    x < width;    x++   )
			{

				m_UndoMap->Read_Pixel(    x, y,      &rdUndo,   &grUndo,   &blUndo   );       //   "Image"  in book

				extraMap->Read_Pixel(        x, y,      &rdExtra,   &grExtra,   &blExtra   );       //   "Filter"  in book




				if(     fineLineModeForThresh    )    //  ****BIG:    For this mode the bitmap must be first THRESHOLDED to  { 0, 255 }
				{

					/****
					if(         rdUndo  ==     0          //      0:     The  ImageSrc  is  black    and
						&&    rdExtra  ==  255   )     //  255:      the  FilterMap    is  white...     then write a  BLACK outline pixel to the destMap
					****/

					/***
					if(         rdUndo   <     detectThreshold                    //      0:     The  ImageSrc  is  black    and
						&&    rdExtra   >    (255 - detectThreshold)   )     //  255:      the  FilterMap    is  white...     then write a  BLACK outline pixel to the destMap
					***/

					if(         rdUndo   <       threshBoundary                    //      0:     The  ImageSrc  is  black    and
						&&    rdExtra   >=    threshBoundary   )     //  255:      the  FilterMap    is  white...     then write a  BLACK outline pixel to the destMap
					{

						m_targMap.Write_Pixel(    x, y,    0,0,0   );

						stillHasPixelsOn =  true;					// since we found a pixel to change, we must do another iteration
					}
				}
				else      //  this mode can work on grayScale bitmaps  .. it automatically does the thresholding by the parms
				{												
					short   undoBlackness  =    255   -   rdUndo;
					short   extraBlackness  =    255   -   rdExtra;



					if(     undoBlackness   >    ( extraBlackness     + detectThreshold )      )
					{

						short   pixelDiff  =      undoBlackness  -   extraBlackness;
		
						short   diffVal     =     (  255 -  pixelDiff  )  / darkerFact;        //  is this the best way to do this ????


						m_targMap.Write_Pixel(    x, y,    diffVal, diffVal, diffVal   );

						stillHasPixelsOn =   true;    // since we found a pixel to change, we must do another iteration
					}
				
				}   //  NOT the  fineLineModeForThresh   algo

			}
		}




											//  apply an erosion to the Source-bitmap for the algo

		/***
		*extraMap  =   *m_undoMapOversize;      //  copy  the undoMap to the  extraMap

		ConvolutionMapFilter   erodeIt(    m_undoMapOversize,     extraMap,     ConvolutionMapFilter::ERODE,         kernalSize   );   //    extraMap  is the source
		erodeIt.Filter();
		***/
		*m_UndoMap  =    *tempMap;    //  can just copy it becase we already did this inside the loop



		iterCount++;

		if(    iterCount   >=   m_Parm2   )
			stillHasPixelsOn =   false;
	}

	


	if(  tempMap !=  NULL )
		delete  tempMap;

	if(  extraMap !=  NULL )
		delete  extraMap;


	TRACE(   "\n\nSkeleton filter  has COMPLETED.  ( %d   iterations  out of a possible %d iters. )  \n\n" ,      iterCount,   m_Parm2    );
}

	


