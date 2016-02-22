/////////////////////////////////////////////////////////////////////////////
//
//  commonGrafix.h   -   'GEOMETRIC-DATA structures'   plus some   simple   String Functions
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


//   ******************  ALSO has some odd GLOBAL functions at the bottom  ****************************



#if !defined( _COMMONGRAFIX_H_  )

#define _COMMONGRAFIX_H_

/////////////////////////////////////////////////////////////////////


class    PointFIX;   
class    RectLong;



////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class   Dataj      
  { 					          // my  'most-base'  class ( for FILE storage )
    
    public:  
      virtual  long  Get_ByteCount()  {  return 0; }   // default  ??? OK ????

      virtual  long  Get_Count()      {  return 1; }     // for Collections( overide )

    
    public:
              //  FIELDS ???       short  X, Y;
  };



////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef  unsigned long    COLORREFjm;

#define RGBjm(r,g,b)          ((COLORREFjm)( ((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)) )

#define GetRValueJm(rgb)      (LOBYTE(rgb))
#define GetGValueJm(rgb)      (LOBYTE(((WORD)(rgb)) >> 8))
#define GetBValueJm(rgb)      (LOBYTE((rgb)>>16))


//   Usage      COLORREF   clr =    RGB(  val,  val,  val  );


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////  Point ( short )  //////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class    PointShrt  :  public  Dataj
  { 
 
    public:       
      PointShrt()  {  }
      PointShrt( short x, short y );                    // PointShrt( CPoint cpt ); **IBM**
      
      PointShrt(  PointFIX  cpt  );          // conversion of Pointj classes...
      
      
      PointShrt( const  PointShrt &cpt );    // copy constructor
 
      void  operator += (  const  PointShrt &other  );
      void  operator -= (  const  PointShrt &other  );
      
      void  operator  = (  const  PointShrt &other  );

      void  operator  = (  const  PointFIX  &other  );   // conversion of Pointj classes...    					



      long  Get_ByteCount()  {  return  4L; }      // is this OK????   					
      					
      					// void  operator = (  const  PointShrt    &other  ); **IBM**
    
    public:
      short  X, Y;
  };



class   PointFIX :  public  Dataj   // **COULD:  'PointShrt' but reconcile same 'X,Y' fields
  { 
    public:  
      PointFIX()  {  }
      PointFIX( long x, long y );                   // PointShrt( CPoint cpt ); **IBM**
      
      PointFIX(  PointShrt cpt  );         // conversion of Point classes...   
      
      
      PointFIX( const  PointFIX &cpt );    // copy constructor
 
      void  operator += (  const  PointFIX  &other  );
      void  operator -= (  const  PointFIX  &other  );
      
      void  operator  = (  const  PointFIX   &other  );

      void  operator  = (  const  PointShrt  &other  );   // conversion of Pointj classes...     					
      					
      					// void  operator = (  const  PointShrt    &other  ); **IBM**

      long  Get_ByteCount()  {  return  8L; }      // is this OK????   					


    public:
      long   X, Y;
  };




class    PointLong      :  public  Dataj
{ 
 
public:       
      PointLong()  {  }
      PointLong( long x, long y );                    //  PointLong( CPoint cpt ); **IBM**
      
      PointLong(   PointFIX   cpt   );          // conversion of Pointj classes...      
      PointLong(   PointShrt  cpt   ); 


      PointLong( const  PointLong &cpt );    // copy constructor
 
      void  operator +=  (  const  PointLong &other  );
      void  operator -=  (  const  PointLong &other  );
      
      void  operator  =   (  const  PointLong &other  );

      void  operator  =   (  const  PointFIX  &other  );   // conversion of Pointj classes...    					

	  void  operator  = (  const  PointShrt &other  );




      long  Get_ByteCount()  {  return  4L; }      // is this OK????   					
      					
      					// void  operator = (  const  PointLong    &other  ); **IBM**
    
public:
      long  X, Y;
};




//////////////////////////////////////////////////////////////////////////
///////////  RECTANGLE ( short )  //////////////////////////////////////// 


class    RectShrt      :  public  Dataj
{ 
public:  
      RectShrt() { }

      RectShrt(  short Left,  short Top,   short Right,  short Bottom  ); 
      
      RectShrt(  const  RectShrt &rct  );                 // copy constructor
      
      RectShrt(  const  RectLong &rct  ); 



	  void    Clip_By_Big_Rect(   RectShrt&   rectBig  );    //   NEW,  2/12  **********



      BOOL  Intersected_by(  PointShrt pt  );

	  bool    Intersected_by(   RectShrt&   rectOther );

      

      void  Offset_by(  PointShrt pt  );

      void  Get_Center(  PointShrt *pt  );
      void  Get_LowerRight(  PointShrt *pt  );
      void  Get_UpperLeft(   PointShrt *pt  );
      
      void  Set_LowerRight( const  PointShrt& pt  );
      void  Set_UpperLeft(  const  PointShrt& pt  );

      

      void  operator = ( const RectShrt &other );

      
      void  operator = ( const RectLong &other );   //  NEW 


      void  Set_Data( short Left, short Top,  short Right, short Bottom );  // NEW*** 

      
      long  Get_ByteCount()  {  return  8L; }      // is this OK????   		



	short  Get_Width()    {    return  ( right    - left     +1 );    }    //  +1  need inclusiuve counting
    
    short  Get_Height()   {    return  ( bottom - top    +1 );     }    //  +1  need inclusiuve counting




    
    public:
      short    left, top,  right, bottom;
  };                                      



//////////////////////////////////////////////////////////////////////////
///////////  RECTANGLE ( long )  //////////////////////////////////////// 


class    RectLong  :  public  Dataj
{ 

public:  
      RectLong() { }

      RectLong(  long Left,  long Top,   long Right,  long Bottom  ); 
      
      RectLong(  const  RectLong &rct  );                 // copy constructor
     

      RectLong(   PointLong  upLeftPt,     PointLong  lowerRightPt   ); 




      BOOL      Intersected_by(  PointLong pt  );

	  bool		Intersected_by(  RectLong&   rectOther  );   //  NEW,  11/08


	  bool		Has_Valid_Data();    //  NEW,  2/12    ...All values must be zero or greater

      
      void		Offset_by(  PointLong pt  );

      void		Get_Center(  PointLong *pt  );
      void		Get_LowerRight(  PointLong *pt  );
      void		Get_UpperLeft(   PointLong *pt  );
      
      void		Set_LowerRight( const  PointLong& pt  );
      void		Set_UpperLeft(  const  PointLong& pt  );

      

      void		operator = ( const  RectLong  &other );


	  void		operator = ( const  RectShrt  &other );   //  NEW 

      
      void		Set_Data(   long Left, long Top,    long Right, long Bottom  );  // NEW*** 

      
      long		Get_ByteCount()  {  return  8L; }      // is this OK????   					
    


	  void    Clip_Rect(   const  RectLong&  clipRect    );     // NEW*** 

    
    
public:
      long    left, top,  right,   bottom;
};                                      




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   Graphic      
{      
					//  Can DRAW itself  and do  'HIT-DETECTION'   ,     optionally have a  'Mobility'  property
					//  also  Hilight()   and  UNhilight()   commands 
					//  might be the PARENT class  for  {   Handlej   and  HandleFrame  }    ...what are SHARED function???
public:
	Graphic()                {   m_offset.X  =  -18888;   }      //  to flag that this is un-initialized


	virtual ~Graphic()    {   }       

	
// 		  ...INSTALL later     ( from original documentation )
//
 //    virtual   void     Hilight();			 
 //    virtual   void     UnHilight();        


	virtual			void         Get_BoundBox(   RectLong&  rct   )     //  old docs called this   'Bounds()'  
																{   rct.left =  m_offset.X;                        rct.top =  m_offset.Y; 
																     rct.right =  m_offset.X  + m_width;  	 rct.bottom =  m_offset.Y   +  m_height;
																}


	virtual			bool         Is_Hit(   const  PointLong&  pt   )   
																{	RectLong  rct;
																	 Get_BoundBox(  rct  );     // retrieve BoundBOX     
																	 if(     rct.Intersected_by( pt )    )    return   true;						
																	 else											  return   false;
																}



	virtual			void         Assign_Data(   RectLong&  rct   )    //  NEW  6/02,     use it
	{
		m_offset.X =   	rct.left;  
		m_offset.Y  =   rct.top;

		m_width =    rct.right    -   rct.left;

		m_height =  rct.bottom  -  rct.top;
	}



	virtual			void         Assign_OutOfBounds( )
	{
		m_offset.X  =   -1;       
		m_offset.Y  =   -1;       
		m_width     =   0;
		m_height    =   0;
	}


//	 virtual		 void         Draw( );  	???
//   virtual		 void         Translate_By(   PointShrt&  pt   );        ?????



public:
	PointLong    m_offset;       //	  ( WAS:  short  _xOfset, _yOfset )       ...the  rendering OFFSET  in   WINDOW-coords  ???

    long           m_width,   m_height;             //  the  rendering DIMENSIONS  in    WINDOW-coords 
};





////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class    IntersectDetector   
{ 
			//  Used to get   Intersections   and  Close-Intersections

public:       
	IntersectDetector(   long  start,    long  end   );     
	virtual	 ~IntersectDetector()    {  }	



	virtual		bool			Intersects_At_All(    long   nuStart,    long  nuEnd   );


	virtual		bool			Barely_Not_Intersects(    long   nuStart,   long  nuEnd,     long  percentTolerance   );    
										//  Even if it touces( by a percentage of the resident distance), we still say it 'Does NOT intersect'



public:
	long   m_resStart,    m_resEnd;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class    ToneSegment
{ 
		
public:       
	ToneSegment();     
	virtual	 ~ToneSegment()    	{  }	



	virtual		long		Get_Length();

	virtual		short		Get_Average_Tone();





public:
	long     m_startPixel;
	long     m_endPixel;

	long	 m_totalTone;
};





///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////  DUMMY abstract class to keep the compiler happy  ////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////


class    UniApplication     //     ABSTRACT TopLevel parent class for  any of my TINY Apps like the   PitchscopePlayer   12/09
{

		    //   UniEditorApp  knows NOTHING( cannot access ) about   SoundHelper, etc  !!!   
	        //
			//	  	...only knows  Fundamental Commands  of :  {   Viewj,    Editorj,    ComponentSubject,    ComponentView,   Toolj   }  

	//   1)  Should each  Viewj  be capable of loading its OWN Editorj( or should an Editorj CREATE its own Viewjs )  ???


public:
//	UniApplication();								 //  Keep out so that this can be an ABSTRACT class   12/2009
	virtual ~UniApplication()  {   }



	virtual		bool		   SRC_File_Is_Loaded() =  0;


	virtual      void		   Set_DSTlist_Modified(  bool  isModified )   {  }     //  Dummy for subclasses


	virtual      bool		   Has_A_MP3()   {   return  false;   }     //  Dummy for subclasses




	virtual		bool		  Create_Animating_CDCs_All_Viewjs()   =0;
	virtual		void		  Release_Animating_CDCs_All_Viewjs()  =0;


																							//  do I want an ENCLOSING class for these???  (  AnimeAdmin )


	virtual		void		  Animate_All_Viewjs(   long  curSample,    short curScalePitch    )  =0;     //  called by   Process_Event_Notification___()


	virtual		void		  ReDraw_All_Anime_Viewjs()  =0;    //  Used in Navigator  in draw the  "Last Frame"   ( Bullets of Revolver and Gagues )     1/2012




	virtual		void		  ReDraw_Bitmaps(  bool  renderWithoutText  )     {   ASSERT( 0 );     return;  }    //  NEW, needed for Navigator.  Can I use this in other apps???

	virtual		void		  ReDraw_Bitmaps()                                            {   ASSERT( 0 );     return;  }  





	 virtual		void		   Update_FirstSampleIdx_All_Viewjs(   long  sampleIdx   )  =0;

	 virtual		void           Update_FirstSampleIdx_All_AnimeViewjs(   long  sampleIdx   )  =0;

	 virtual		void		   Update_FirstSampleIdx_Active_RankingViewj(   long  sampleIdx   )  =0;   // will do AnimeViewj if no EditViewj is Present
};





////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////  DUMMY abstract class to keep the compiler happy  ////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   AnimePacketAbstr  
{

public:
	AnimePacketAbstr();
	virtual ~AnimePacketAbstr();



public:
	bool        m_animeBackwards;      // BAD,  just to keep compiler happy   

	bool        m_isFrozenFrame;    //  false :   Is part of an animated sequence     

};





////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


void    Status_Mesg(  char *stg  );

void    Do_Error(  char *errorString,   BOOL fatal  );     // my sub

void    My_Delay(  short  halfSecs  );             // different capital letters than previous



void	Get_Paths_Target_FileName_GLB(  CString&  path,             CString&  retFileName  );

void    Get_Targets_Folder_Path_GLB(      CString&  targetsPath,   CString&  retFolderPath  );



bool     Is_A_SubRegion_File_GLB(  CString&  filesTitle     );

bool     Get_SubRegion_Files_Centroid_ParentSeps_FileName_GLB(  CString&  filesTitle,   long&  retXcoord,  long&  retYcoord,   CString&  retSepsName  );


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////


#endif   // __H_
  
