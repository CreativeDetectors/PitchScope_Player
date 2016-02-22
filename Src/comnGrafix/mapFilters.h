/////////////////////////////////////////////////////////////////////////////
//
//  mapFilters.h   -   
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined( _MapFILTERS_H_  )

#define _MapFILTERS_H_

/////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////		
/////////////////////////////////////////////////////////////////////



class   MapFilter             //  ABSTRACT superclass 
{ 		                           		  
public:  

      virtual   void   Filter() =0;        // could pass  'TransformMap*'  as PARM ???
      

    //  virtual   void   Filter_Column(  short x  )   {   }  // DEFAULT dummy   
    //  virtual   void   Filter_Row(     short y  )     {   }  // DEFAULT dummy         
};





			////////////////////////////////////////////////
			////////////////////////////////////////////////


class   OnTheFlyMapFilter     :  public   MapFilter             
{ 		 

			//  creates an UNdo map on the fly, and automatically releases if


public:  
	 OnTheFlyMapFilter(    OffMap&  targMap,       short parm1,   short parm2,   short parm3   );      
	 virtual ~OnTheFlyMapFilter();


 //     virtual    void   Filter() =0;       

	  virtual    void	   Release_OnFly_UndoMap();
      


public:  
	OffMap&    m_targMap;  

    OffMap     *m_UndoMap;    


    short     m_Parm1,     m_Parm2,     m_Parm3;
};




								
								////////////////////////////
								////////////////////////////
								////////////////////////////


class   ConvolutionMapFilter  :   public   MapFilter      //  **** SHOULD be a descendant of the new   OnTheFlyMapFilter  class
{ 		       
	
public:  
     ConvolutionMapFilter(   OffMap  *freqMap,   OffMap  *undoMap,     short filterType,    short parm1   );  
	 virtual ~ConvolutionMapFilter();


     virtual	  void		  Filter();                   // could pass  'TransformMap*'  as PARM ???

     virtual	  void		  Filter_24bit();            




//     virtual	  BOOL		Has_UndoMap()      {  return   _HasUndoMap;  }

    virtual    void			  Release_OnFly_UndoMap();     //  NEW,  cause now we can dynamically allocate the  _UndoMap



   
     virtual	  void		  Set_Parameter(  short  val   )      {  _Parm1 =   val;   }    
  






public:  
	OffMap   *_Map;  
    OffMap   *_UndoMap;    


//    short     _HasUndoMap;
	bool      m_didAllocUndoMap;     // now it will create one if it is not there   4/10

      
    short     m_filterCode;   


    short     _Parm1;



	short    m_parm2;     //  for OIL filter:  oilNumberOfBits,         for  SHARPEN  1: little sharpen    2: much sharpen  

	short    m_parm3;     //  for OIL filter,      oilLightenFactor   





	enum   filterModes  
	{  
		NONE,   //  for  ParameterPassing in functions

																	//  1)    HORZ  first     ( I could Identify by LAST HORZ-filter ID )
	    
		BLURHORZ,    MEDIANHORZ,    DIALATEhORZ,    OILHORZ,    HOLEPLUGhORZ,   HOLEPLUGhORZ2X,   HOLEPLUGhORZ3X,   
			LONEPIXCUTHORZ,     SHARPENHORZ,    ERODEhORIZ,
		


																	//  2)    VERT 

		BLURvERT,     MILDVERTSHARP,    HOLEPLUGvERT,     LONEPIXCUTVERT,     ERODEvERT,
		


																	//  3)    2-Dimensional

		SHARPEN,    BLUR,    MEDIAN,     RELIEF,    OIL,    POSTERIZE, 	  LITEN,  	LITENpARM,     DARKEN,
		CUTOFF,    THRESHOLD,     CONTRASTUP,   CONTRASTDOWN,     DIALATE,   ERODE,   
		
		PREWITT,     LINEdETECThORZ,       DIALATEsEP,   ERODEsEP,   GAMMAbRIGHTEN,   SATURATIONbOOST,
		BLURaDJUST
	};


};


/***

		SHARPEN,    BLUR,    RELIEF,    OIL,   POSTERIZE,	 CUTOFF,   THRESHOLD,    LITEN,   DARKEN,
		LONEPIXCUTVERT,     CONTRASTUP,    CONTRASTDOWN,   SHARPENHORZ,   BLURHORZ,   OILHORZ,
		MEDIAN,    MEDIANHORZ,    MILDVERTSHARP,
		DIALATEhORZ,   ERODEvERT,   LONEPIXCUTHORZ,    DIALATE,     HOLEPLUGhORZ,    HOLEPLUGvERT,
		BLURhORZxx,    BLURvERT,     HOLEPLUGhORZ2X,   HOLEPLUGhORZ3X,   	LITENpARM
***/





/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

class   EdgeDetectingMapFilter      :  public   OnTheFlyMapFilter       
{ 		       
					

public:  
     EdgeDetectingMapFilter(    OffMap&  targMap,     short  kernalSize,    short parm2,   short parm3    );  
	 virtual ~EdgeDetectingMapFilter();

	 virtual	  void    Filter();    
};



/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

class   SkeletonMapFilter      :  public   OnTheFlyMapFilter       
{ 		       


public:  
     SkeletonMapFilter(    OffMap&  targMap,     short  toneDetectThreshold,    short maxIters,    short  fineLineMode   );  
	 virtual ~SkeletonMapFilter()   {  }    //  think I need this just so  OnTheFlyMapFilter::~OnTheFlyMapFilter()   will  get called    4/10 

	 virtual	  void    Filter();    
};









/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

class   ContrastingMapFilter  :   public   MapFilter       
{ 		       

						//  Works great on FundamentalMap.  See notes in .cpp    7/06
public:  
     ContrastingMapFilter(    OffMap&  offMap,   short  calcMode,   double  valueSpread,    double  biggestDifference   );  



	virtual		bool		   Filter_For_Contrast(  CString&  retErrorMesg  );    	//  the big Kahuna

	virtual		void		   Filter()    {   }                      //  dummy for compiler




protected:
     virtual	  bool		   Make_ColorTable_Squared(   CString&  retErrorMesg   );  

     virtual	  bool		   Make_ColorTable_Linear(     CString&  retErrorMesg   );     


				  short		 Get_New_GreyValue(  short  oldGreyVal   );




public:  
	OffMap&    m_offMap;  

	short         m_calcMode;


	double      m_valueSpread;

	double       m_biggestDiff;      //  the  biggest difference in GreyVal at the upper end of the colortable


	double    m_diffTable[  256  ];

	short      m_diffTableEntries;


	short      m_finalTable[  256  ];
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif   // __H_

