/////////////////////////////////////////////////////////////////////////////
//
//  CommonGrafix.cpp   -   
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



#include  "..\comnFoundation\myMath.h"      


#include   "CommonGrafix.h"        

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////
////////////////////////  POINT  ///////////////////////////


PointShrt::PointShrt( short x,  short y )  
	{ X= x;   Y= y; } 


PointShrt::PointShrt( const PointShrt &cpt )               // copy constructor
	{ X= cpt.X;  Y= cpt.Y; }        

void  PointShrt::operator=(  const  PointShrt &other  )
	{ X= other.X;   Y= other.Y;  } 


void  PointShrt::operator += (  const  PointShrt &other  )
	{ X += other.X;   Y += other.Y;  } 

void  PointShrt::operator -= (  const  PointShrt &other  )
	{ X -= other.X;   Y -= other.Y;  } 


PointShrt::PointShrt(  PointFIX cpt  )        // conversion...
  {  
     X =   (short)(   cpt.X   >>  FXPT  );  
     Y =   (short)(   cpt.Y   >>  FXPT  );
  }      


void  PointShrt::operator=(  const  PointFIX &other  )  //  { X= other.X;   Y= other.Y;  } 
  {  
     X =   (short)(   other.X   >>  FXPT  );     // conversion of Point classes...
     Y =   (short)(   other.Y   >>  FXPT  );
  }      
 
			
			//////////  FIX  ////////////////// 
 

PointFIX::PointFIX( long x,  long y )  
	{ X= x;   Y= y; } 


PointFIX::PointFIX( const PointFIX &cpt )               // copy constructor
                                    { X= cpt.X;  Y= cpt.Y; }        

void  PointFIX::operator=(  const  PointFIX &other  )
                                    { X= other.X;   Y= other.Y;  } 


void  PointFIX::operator += (  const  PointFIX &other  )
                                    { X += other.X;   Y += other.Y;  } 


void  PointFIX::operator -= (  const  PointFIX &other  )
                                    { X -= other.X;   Y -= other.Y;  } 



PointFIX::PointFIX(  PointShrt cpt  )        // conversion of Point classes...
  {  
     X =   (  (long)cpt.X  )  <<  FXPT;  
     Y =   (  (long)cpt.Y  )  <<  FXPT;
  }      



void  PointFIX::operator = (  const  PointShrt &other  )    // { X= other.X;   Y= other.Y;  } 
  {  
     X =   (  (long)other.X  )  <<  FXPT;        // conversion of Point classes...
     Y =   (  (long)other.Y  )  <<  FXPT;
  }      
    




////////////////////////////////////////////////////////////
////////////////////////  Point  -  LONG  ///////////////////////////


PointLong::PointLong( long x,  long y )  
	{ X= x;   Y= y; } 


PointLong::PointLong( const PointLong &cpt )               // copy constructor
	{ X= cpt.X;  Y= cpt.Y; }        

void  PointLong::operator = (  const  PointLong &other  )
	{ X= other.X;   Y= other.Y;  } 


void  PointLong::operator  = (  const  PointShrt &other  )
	{ X= other.X;   Y= other.Y;  } 


void  PointLong::operator += (  const  PointLong &other  )
	{ X += other.X;   Y += other.Y;  } 

void  PointLong::operator -= (  const  PointLong &other  )
	{ X -= other.X;   Y -= other.Y;  } 


PointLong::PointLong(  PointFIX cpt  )        // conversion...
  {  
     X =   (long)(   cpt.X   >>  FXPT  );  
     Y =   (long)(   cpt.Y   >>  FXPT  );
  }      


PointLong::PointLong(  PointShrt cpt  )        // conversion...
 	{ X= cpt.X;  Y= cpt.Y; }        



void  PointLong::operator=(  const  PointFIX &other  )  //  { X= other.X;   Y= other.Y;  } 
  {  
     X =   (long)(   other.X   >>  FXPT  );     // conversion of Point classes...
     Y =   (long)(   other.Y   >>  FXPT  );
  }      



//////////////////////////////////////////////////////////////
////////////////////////  RECTs  /////////////////////////////


BOOL  RectShrt::Intersected_by(  PointShrt pt  )
						{ 
                            if(  pt.X  <  left   )     return FALSE;
                            if(  pt.X  >  right  )     return FALSE;
                            if(  pt.Y  <  top     )    return FALSE;
                            if(  pt.Y  >  bottom  )    return FALSE;
     
                            return  TRUE;  // default, if passes all 4 tests 
                          }



bool  RectShrt::Intersected_by(   RectShrt&   rectOther )
{ 

	if(   rectOther.right  <  left   )
		return  false;

	if(   rectOther.left  >  right   )
		return  false;


	if(   rectOther.bottom  <  top    )
		return  false;

	if(   rectOther.top     >  bottom   )
		return  false;

	return  true;
}



void    RectShrt::Clip_By_Big_Rect(   RectShrt&   rectBig  )
{ 

	if(    left  <   rectBig.left   )             //   NEW,   2/2012
		left =    rectBig.left;

	if(    right  >   rectBig.right   )
		right =    rectBig.right;


	if(    top  <   rectBig.top   )
		top =    rectBig.top;

	if(    bottom  >   rectBig.bottom   )
		bottom =    rectBig.bottom;
}



      
RectShrt::RectShrt(  short Left,  short Top,  short Right,  short Bottom  ) 
         { left=  Left;     top=    Top;  
           right= Right;    bottom= Bottom; 
         } 


RectShrt::RectShrt(  const  RectShrt &rct  )          // copy constructor
         { left=  rct.left;     top=    rct.top;  
           right= rct.right;    bottom= rct.bottom; 
         } 


RectShrt::RectShrt(  const  RectLong &rct  )
         { left=  (short)rct.left;      top=    (short)rct.top;			//  CAREFUL for  loos of accuracy !!!!
           right= (short)rct.right;    bottom= (short)rct.bottom; 
         } 


void  RectShrt::Set_Data(  short Left,  short Top,  short Right,  short Bottom  ) 
         { left=  Left;     top=    Top;  
           right= Right;    bottom= Bottom; 
         } 


         
void   RectShrt::operator=( const RectShrt &other )
                          { left= other.left;      top= other.top;  
                            right= other.right;    bottom= other.bottom; } 



void   RectShrt::operator = ( const RectLong &other )
                          { left= (short)other.left;       top= (short)other.top;  
                            right= (short)other.right;    bottom= (short)other.bottom; } 



void  RectShrt::Offset_by(  PointShrt pt  )
               		   { left +=  pt.X;     right  +=  pt.X;    
                   	     top  +=  pt.Y;     bottom +=  pt.Y;    
                       }

      
void  RectShrt::Get_Center(  PointShrt *pt  )
         		{ pt->X= (right  - left)/2  + left; 
         		  pt->Y= (bottom - top)/2   + top; 
                }


void  RectShrt::Get_LowerRight(  PointShrt *pt  )
         		{ pt->X= right;  pt->Y= bottom;   }


void  RectShrt::Get_UpperLeft(  PointShrt *pt  )
         		{ pt->X= left;   pt->Y= top;   }



void  RectShrt::Set_LowerRight( const  PointShrt& pt  )
         		{ right= pt.X;  bottom= pt.Y;   }


void  RectShrt::Set_UpperLeft(  const  PointShrt& pt  )
         		{ left= pt.X;  top= pt.Y;   }






//////////////////////////////////////////////////////////////
////////////////////////  RECT  LONG  /////////////////////////////


BOOL  RectLong::Intersected_by(  PointLong pt  )
						{ 
                            if(  pt.X  <  left   )     return FALSE;
                            if(  pt.X  >  right  )     return FALSE;
                            if(  pt.Y  <  top     )    return FALSE;
                            if(  pt.Y  >  bottom  )    return FALSE;
     
                            return  TRUE;  // default, if passes all 4 tests 
                          }




bool  RectLong::Intersected_by(   RectLong&   rectOther )
{ 

	if(   rectOther.right  <  left   )
		return  false;

	if(   rectOther.left  >  right   )
		return  false;


	if(   rectOther.bottom  <  top    )
		return  false;

	if(   rectOther.top     >  bottom   )
		return  false;

	return  true;
}





      
RectLong::RectLong(  long Left,  long Top,  long Right,  long Bottom  ) 
         { left=  Left;     top=    Top;  
           right= Right;    bottom= Bottom; 
         } 




RectLong::RectLong(  const  RectLong &rct  )          // copy constructor
         { left=  rct.left;     top=    rct.top;  
           right= rct.right;    bottom= rct.bottom; 
         } 




RectLong::RectLong(   PointLong  upLeftPt,     PointLong  lowerRightPt   )
{
	left  =  upLeftPt.X;	
	top  =   upLeftPt.Y;

	right  =  lowerRightPt.X;	
	bottom  = lowerRightPt.Y;        
}





void  RectLong::Set_Data(  long Left,  long Top,  long Right,  long Bottom  ) 
         { left=  Left;     top=    Top;  
           right= Right;    bottom= Bottom; 
         } 



bool   RectLong::Has_Valid_Data() 
{ 		 
	if(    left   < 0   )
		return  false;
			 
	if(    right   < 0   )
		return  false;

	if(    top   < 0   )
		return  false;

	if(    bottom   < 0   )
		return  false;

	return  true;
} 



         
void  RectLong::operator=( const RectLong &other )
                          { left= other.left;      top= other.top;  
                            right= other.right;    bottom= other.bottom; } 


void   RectLong::operator = ( const RectShrt &other )
                          { left= other.left;      top= other.top;  
                            right= other.right;    bottom= other.bottom; } 


void  RectLong::Offset_by(  PointLong pt  )
               		   { left +=  pt.X;     right  +=  pt.X;    
                   	     top  +=  pt.Y;     bottom +=  pt.Y;    
                       }

      
void  RectLong::Get_Center(  PointLong *pt  )
         		{ pt->X= (right  - left)/2  + left; 
         		  pt->Y= (bottom - top)/2   + top; 
                }


void  RectLong::Get_LowerRight(  PointLong *pt  )
         		{ pt->X= right;  pt->Y= bottom;   }


void  RectLong::Get_UpperLeft(  PointLong *pt  )
         		{ pt->X= left;   pt->Y= top;   }



void  RectLong::Set_LowerRight( const  PointLong& pt  )
         		{ right= pt.X;  bottom= pt.Y;   }


void  RectLong::Set_UpperLeft(  const  PointLong& pt  )
         		{ left= pt.X;  top= pt.Y;   }




void   RectLong::Clip_Rect(   const  RectLong&  clipRect    )  
{
	
	
	//  bool   offPane =   false;			//  May have to do some CLIPPING on the left and right sides


	if(    this->left   <   clipRect.left   )				//  clip in HORIZONTAL
		this->left =  clipRect.left;
	else if(    this->left   >   clipRect.right   )
	{
		this->left   =   clipRect.right;  
		//  offPane =  true;
	}

	if(		 this->right   >   clipRect.right 	  )
		this->right =   clipRect.right;
	else if(  	this->right   <   clipRect.left    )
	{
		this->right   =   clipRect.left; 
		//  offPane =  true;
	}


																		//  now Clip in  VERTICAL  
	if(    this->top   <   clipRect.top   )
		this->top =  clipRect.top;
	else if(    this->top   >   clipRect.bottom   )
	{
		this->top   =   clipRect.bottom;  
		//  offPane =  true;
	}

	if(		 this->bottom   >   clipRect.bottom 	  )
		this->bottom =   clipRect.bottom;
	else if(  	this->bottom   <   clipRect.top   )
	{
		this->bottom   =   clipRect.top; 
		//  offPane =  true;
	}
} 




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


IntersectDetector::IntersectDetector(   long  start,    long  end   )     
													                     :  m_resStart( start ),   m_resEnd( end )
{  
			//  input parms do NOT have to be 'in order'


	if(    start  >  end    )      //  get values in ASCENDING order
	{
		m_resStart  =    end;  
		m_resEnd   =    start;    //  order is wrong, so swap values
	}
}



/***
		bool  doesIntersect =   false;


		if(		     m_resEnd   <=   nuStart         //   Complete  MISS     d)  &  e)
			   ||   m_resStart   >=   nuEnd     )
			doesIntersect =  false;
		else
		{
			if(    nuStart  >=   m_resStart     &&     nuEnd  <=  m_resEnd     )      //  a)   FFT  fits COMPLETELY inside the  DFTmask
				doesIntersect =   true;   
			else
			{																									    //  c)  FFT  'more than contains'  the  DFTmask
				if(    nuStart  <=   m_resStart     &&     nuEnd  >=  m_resEnd     )     //       ...happens a lot for BASS/andMOST notes  cause  FFT is much wider than DFT bands )
					doesIntersect =   true;   //  ???   Want to  Interpolate ...but HOW ????? ***** 
				else
				{							
					if(    transferPartialIntersections    )     	//   PARTIAL intersects of   FFT  row  
					{

						if(          (    nuEnd < m_resEnd     &&      nuEnd  >  m_resStart   )          //  b)
							   ||	(    m_resEnd <  nuEnd     &&     m_resEnd  >  nuStart    )   )     //   f)
							doesIntersect =   true;					//    ****** ???   Want to  Interpolate ...but HOW ????? ***** 
					}
				}
			}
		}


***/
											////////////////////////////////////////


bool     IntersectDetector::Intersects_At_All(    long   nuStart,     long  nuEnd    )
{

								//  input parms do NOT have to be 'in order'

	bool  doesIntersect;


	if(    nuStart  >  nuEnd    )      //  get values in ASCENDING order
	{
		long   tmp =  nuStart;

		nuStart  =    nuEnd;	//  order is wrong, so swap values
		nuEnd    =    tmp;    
	}



	if(		      m_resEnd    <=   nuStart           //   Complete  MISS     d)  &  e)
			||   m_resStart   >=   nuEnd     )
	{

		doesIntersect =   false;

//		return  false;

	}
	else
	{
		doesIntersect =   true;
	}



	return   doesIntersect;
}



											////////////////////////////////////////


bool     IntersectDetector::Barely_Not_Intersects(    long   nuStart,   long  nuEnd,     long  percentTolerance   )
{

				//  Even if it touches( by a percentage of the resident distance),  we still say it  'Does NOT intersect'



	//  percentTolerance :   { 0 -  100 }   The overlap must be THIS small, or smaller to say NO-Intersection,  
	//													  if(  percentTolerance ==  0  )   then tolerance MEASUREMENT is disabled.  


	if(    percentTolerance  <  0     ||     percentTolerance >  100    )
	{
		AfxMessageBox(   "Barely_Not_Intersects() failed, bad input parms."    );
		return  false;
	}



	if(    nuStart  >  nuEnd    )      //  get values in ASCENDING order
	{
		long   tmp =  nuStart;

		nuStart  =    nuEnd;	//  order is wrong,  so swap values
		nuEnd    =    tmp;    
	}



	if(     !Intersects_At_All(  nuStart,   nuEnd  )     )
		return   true;											//  there has to be SOME intersection,  to examine with a tolerance





								//  Now we know there is SOME intersection,  now measure HOW MUCH overlap there is


	if(    nuStart  >=   m_resStart     &&     nuEnd  <=  m_resEnd    )      //  a)   New  fits COMPLETELY inside the Resident
	{
		return   false;     //  Intersects  completely
	}

	if(    nuStart  <=   m_resStart     &&     nuEnd  >=  m_resEnd    )     //  b)   Resident is COMPLETELY covered by New 
	{
		return   false;     //  Intersects  completely 
	}





								//  the following cases involve  'PARTIAL intersection'   ...so our Tolerance now comes into play


	if(    percentTolerance  ==  0    )
		return   false;     //  ANY  Intersection  qualifies,  cause  NO( zero ) tolerance 'checking'  was specified



	long   toleratedDistance,  touchDist,    resLength =  m_resEnd  -  m_resStart;

	ASSERT(   resLength  >=  0   );


	toleratedDistance  =    (  resLength  *  percentTolerance  )   / 100L;   //   This is the  'smallest distance'  that is allowable to 
																							 //   still say that they  "Do NOT touch"
	if(   toleratedDistance  <=  0   )
		return  false;   // We have to say that they intersect,  because we have NO  'Distance of Toleration'  to make an exception





	if(    m_resEnd  >=   nuStart												//   c)  
											 	&&    m_resStart   <   nuEnd    )
	{
		touchDist  =     m_resEnd  -   nuStart;

		ASSERT(  touchDist  >=  0  );


		if(    touchDist   <=   toleratedDistance   )  
			return   true;	   //  true:   Because the distance of intersection is SMALL ENOUGH,  we say they 'DoNOT intersect' 
		else
			return   false;
	}
	else if(   m_resStart  <=  nuEnd											//   d)  
													&&    m_resEnd  >   nuStart   )
	{
		touchDist  =     nuEnd  -   m_resStart;


		ASSERT(  touchDist  >=  0  );

		if(    touchDist   <=   toleratedDistance   )
			return   true;
		else
			return   false;
	}
	else
	{  ASSERT( 0 );
	}


	return   false;      //  AMBIGUOUS result,   sould never get to here
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


ToneSegment::ToneSegment()     
{  

	m_startPixel  =  -1;
	m_endPixel   =  -1;

	m_totalTone =  0;
}



											////////////////////////////////////////


long	ToneSegment::Get_Length()     
{  

	if(   m_startPixel  ==  -1      ||    m_endPixel  ==  -1    )
		return  0;


	long      len =     m_endPixel    -    m_startPixel;
	return   len;
}



											////////////////////////////////////////


short	ToneSegment::Get_Average_Tone()     
{  

	if(   m_startPixel  ==  -1      ||    m_endPixel  ==  -1    )
		return  0;


	long    len =     m_endPixel    -    m_startPixel;

	if(    len  <=  0    )
		return  0;


	short     avgTone  =     (short)(      m_totalTone   /   len      );
	return   avgTone;
}





////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


AnimePacketAbstr::AnimePacketAbstr()
{

	m_animeBackwards =      false;     //  these should really not be here
	m_isFrozenFrame     =     false;  
}



AnimePacketAbstr::~AnimePacketAbstr()
{
}



									///////////////////////////////////////////////////////////////////////////
									///////////// below are some String functions   //////////////////////
									///////////////////////////////////////////////////////////////////////////


void     Get_Paths_Target_FileName_GLB(  CString&  path,   CString&  retFileName  )
{

			// Do not need error checking here( too excessive), the calling function will complain if an empty string comes out

	retFileName.Empty();


	if(   path.IsEmpty()   )
	{
		//  retErrorMesg   =   "Get_Paths_Target_FileName failed, the string was empty."  ;
//		ASSERT( 0 );   *************   3/11    Think this is OK cause want to load Navigator's Notelist files which have NO Project fileName in header 
		return;
	}



	CString  mesg,   origFileName;

	int   slashPos  =    path.ReverseFind(  '\\'  ); 
	if(    slashPos > 0   )
	{
		retFileName =    path.Right(    path.GetLength()  -  (slashPos +1)    );
		return;
	}
	else      
	{  retFileName =    path;  // ****** OK???  Is there a way that this sould fail and give a message
		return;
	}
}


									/////////////////////////////////////////////////////


void    Get_Targets_Folder_Path_GLB(  CString&  targetsPath,   CString&  retFolderPath )
{

			// Do not need error checking here( too excessive), the calling function will complain if an empty string comes out

	retFolderPath.Empty();

	if(   targetsPath.IsEmpty()   )
	{
		//  retErrorMesg   =   "Get_Paths_Target_FileName failed, the string was empty."  ;
		ASSERT( 0 );
		return;
	}



	CString  mesg,   origFileName;

	int   slashPos  =    targetsPath.ReverseFind(  '\\'  ); 
	if(    slashPos > 0   )
	{
//		retFileName =    path.Right(    path.GetLength()  -  (slashPos +1)    );

		retFolderPath  =    targetsPath.Left(  slashPos  +1  );    //  includes the ending slash   '\'

		return;
	}
	else      
	{  ASSERT( 0 );    //  Sounds like a mistake

		retFolderPath =    targetsPath;  // ****** OK???  Is there a way that this sould fail and give a message
		return;
	}
}





									/////////////////////////////////////////////////////


bool    Is_A_SubRegion_File_GLB(  CString&  filesTitle     )
{

				//  For the GfxRegionDetector FORMAT of  subRegions:       xSub_1065-604_Scan_A_trees_Crop_square.bmp


	int   findPos =   filesTitle.Find(   "xSub_"  );

	if(    findPos  >=  0   )    //  really  is   =  0
		return  true;
	else
		return  false;
}


									/////////////////////////////////////////////////////


bool     Get_SubRegion_Files_Centroid_ParentSeps_FileName_GLB(  CString&  filesTitle,   long&  retXcoord,  long&  retYcoord,   CString&  retSepsName   )
{

				//  For the GfxRegionDetector FORMAT of  subRegions:       xSub_1065-604_Scan_A_trees_Crop_square.bmp



	retXcoord =   retYcoord =   -1;   //  init for fail
	retSepsName.Empty();

	CString    strFileExtension  =    ".bmp"  ;



	long     lenString  =      filesTitle.GetLength();


	CString	 strRight   =    filesTitle.Right(   lenString -  5   );     //    5  is len of  "xSub_"              1065-604_Scan_A_trees_Crop_square


	long   underscorePos    =     strRight.Find(  '_'  );
	if(      underscorePos  < 0  )
	{
		ASSERT( 0 );
		return  false;
	}


	CString   coordString  =    strRight.Left(   underscorePos   );


	long   dashPos     =     coordString.Find(  '-'  );
	if(      dashPos  < 0  )
	{
		ASSERT( 0 );
		return  false;
	}



	retSepsName  =    strRight.Right(    strRight.GetLength()   -   coordString.GetLength()   - 1    );


	retSepsName    +=    strFileExtension;     //  want this ???  




	CString   xCoordStrg  =     coordString.Left(  dashPos  );

	CString   yCoordStrg  =     coordString.Right(    (  coordString.GetLength() -1  )   -  dashPos   ); 



	//   strtol(   const char *nptr,  char **endptr,  int base   );

	retXcoord  =    strtol(   xCoordStrg,    NULL,  10  );      //  string to integer   function
	retYcoord  =    strtol(   yCoordStrg,    NULL,  10  );


	return  true;
}





