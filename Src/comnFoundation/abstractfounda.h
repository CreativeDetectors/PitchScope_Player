/////////////////////////////////////////////////////////////////////////////
//
//  AbstractFounda.h   -  abstract foundations of classes
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_ABSTRACTFOUNDA_H__16A7FEE1_4FC0_11D3_9507_ACE109C10000__INCLUDED_)
#define AFX_ABSTRACTFOUNDA_H__16A7FEE1_4FC0_11D3_9507_ACE109C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
///////////////////////////////////////////////////////////////////////////////////////////////////


class     LeafPart;
class     CompositePart;
class     Visitor;



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////


template <class Item>     class   List 			//  *** LIST ***     ( an ABSTRACT class )
{
	public:

        virtual   ~List()   //   Need this for  POLYMORPHISM ????   ... and  'delete'  ?????
		{
		}


//     virtual    List&    operator=(  List  &other  )  =0;    *** NO!!  creates all types of compiler problems


        virtual    long     Count()    const   =0;
 
		virtual    Item&    Get(  long  index  )   =0;

		virtual    Item&   First()   =0;
		virtual    Item&   Last()   =0;  

		virtual    void      Add_Head(   Item  &obj   )   =0;  
		virtual    void      Remove_Head()     =0;					  //  NEW change,   removes BOTH the link and the ITEM    1/2002
		virtual    BOOL      Is_Empty()  const  =0;     

		virtual    void      Kill_Links()  =0;        //  careful
		virtual    void      Add_Tail(   Item  &obj   )   =0; 

		virtual    void      Empty() = 0;    // *** NEW,  4/02   ...OK ????


		virtual    void     Push(  Item  &obj   )  =0;      //  ** STACK ops **
		virtual    Item&    Pop()  =0;     
		virtual    Item&    Top()  =0;   
};



//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////


template <class Item>     class   Iterator		 	//   *** ITERATOR ***     ( an ABSTRACT class )
{
    public: 
		virtual   ~Iterator()   //   Need this for  POLYMORPHISM ????   ... and  'delete'  ?????
		{
		}
		
		virtual    void      First()            =0;
		virtual    void      Next()	          =0;
		virtual    BOOL    Is_Done()     =0;
		virtual    Item&   Current_Item()    =0;
};


														//////////////////   SUB classes   /////////////////


template <class Item>     class  NullIterator      :  public   Iterator<Item>  
{
public: 

	NullIterator()   
	{ 
	}

	virtual   ~NullIterator()   
	{ 
	}

	virtual   void   First()                 
	{  }

    virtual   void   Next()                
	{   }


    virtual    BOOL   Is_Done()          
	{ 
		return   TRUE; 
	}


    virtual     Item&   Current_Item()   
	{
		ASSERT( false );

		Item      *itm;      //  *** DANGEROUS,  not initialized ***
		Item&      curItm =   *itm;       //  Keep compiler happy,   have it return ANYTHING

		return    curItm;
	}

};


//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////


class  Part			 //  an   'ABSTRACT  parent'   for Composites
{ 
	public: 
		virtual   ~Part()   //   Need this for  POLYMORPHISM ????   ... and  'delete'  ?????
		{
		}

		virtual    Iterator<Part>*    Create_Iterator()   =0;    

		virtual    void      Add_Child(   Part&  nw   )   =0;    

		virtual    BOOL    Has_Children_inList()   =0;



	    virtual    void      Accept(   Visitor&   vistr   )   = 0;

		virtual    bool      Execute()   = 0;     //  or    Do_Job()   or   Invoke()    ???


		virtual    void      Get_Description(   CString&   strAddr   )   =0;      //  OPTIONAL
};                             




//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////


class  Adjuster			 //   similar 'pattern' to a visitor
{ 
	public: 
		virtual   ~Adjuster()   
		{
		}


		virtual	bool			Modify(   CString&  retErrorMesg  ) =   0;


//		virtual	bool			Reverse_Modification(   CString&  retErrorMesg  ) =   0;
		virtual	bool			Reverse_Modification(   CString&  retErrorMesg  )    {  retErrorMesg =   "Adjuster::  Not implemented.";     return false;   };     //  DUMMY default
};                             


//////////////////////////////////////////////////////////////////////////////////////
////////////    (below ) some commonsly used STRUCTS    ///////////////////
//////////////////////////////////////////////////////////////////////////////////////



//     Recently moved here from CommonGrafix.h    6/2007

typedef struct  
{  
	short  L;                             
	short  R;  
	
}  PairShort;



typedef struct  
{  
	long  L;                             
	long  R;  
	
}  PairLong;




typedef struct  
{  
	double  Left;  
	double  Right;  
	
}  PairDouble;      //  New,  used for stereo values in FFTfilters   6/07




typedef struct  
{  
	short    index;    	
	double  value;  
	
}  DoubleIndexed;   //  used for Bubble sorts of double values   7/06



typedef struct  
{  
	short    index;    	
	long     value0;  
	long     value1;
	
}  LongIndexed;   //  used for Bubble sorts of double values   7/06




typedef struct  
{  
	short  X;	    	//   NEW,   7/2002
    short  Y;    

}	CoordShort;



typedef struct  
{  
	
	CoordShort  A;			//  UpperLeft   pixel
	CoordShort  B;	
	CoordShort  C;	
	CoordShort  D;	

	CoordShort  E;	
	CoordShort  F;	
	CoordShort  G;	
	CoordShort  H;	

}  KernalCubeThree;     //  the  8 surrounding ( clockwise ) neighbors for a 3x3 kernal of Pixels



typedef struct  {  short  h;   
                   short  v;    }  myPair;





typedef struct     //   used to generate a list shared harmonics in the ScalePitches  4 octave Candidate family   
{  

	short              midiNumber;   //  relative

	short              harmCount;
	
	LongIndexed   harmList[ 6 ];    //  should only need 4 


} HarmonicShared;	



												///////////////////////////////////////////////////


typedef struct      // new,   10/2008
{  

	short         red;   
	short         green;
	short         blue;

} jmColor;	




typedef struct      // new,   11/2008
{  

	char*   textName;  

	short     red;   
	short     green;
	short     blue;

//	short    index;   //  need this  ?????


} jmColorSeparation;	





typedef struct     
{  

	long      index;   

	long     value0;  
	long     value1;

//	char*   textName;  
	CString   cString;


} IndexedString;	 //   NEW   1/2012






//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////



typedef   struct   tagPageSearchParms     
{

	char*   m_tagString;                 //  [REQUIRED]  ...either  <A>   or   <IMG>   
	
	char*   m_fileExtensionTest;    //  [ REQUIRED ??? ]

	char*   m_requiredString;              //   [OPTIONAL ]Should be an ARRAY of strings to test
	char*   m_forbidString;					 //   [OPTIONAL ]Should be an ARRAY of strings to test


	struct   tagPageSearchParms   *m_nextPageSearchParms;


} PAGESEARCHPARMS;    




///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////


#endif    // !defined(AFX_ABSTRACTFOUNDA_H__16A7FEE1_4FC0_11D3_9507_ACE109C10000__INCLUDED_)
