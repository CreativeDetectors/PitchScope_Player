/////////////////////////////////////////////////////////////////////////////
//
//  OffMap.h   -    for a Bitmap that is stored in memory. Is also used to store data for Transforms like the log DFT (logDFTtrForm ) 
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_OFFMAP_H__1FE3D609_E8C1_4284_9691_2335FFE5172C__INCLUDED_)
#define AFX_OFFMAP_H__1FE3D609_E8C1_4284_9691_2335FFE5172C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000







class  RectShrt;


////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


typedef  struct 
{ 
	short   xl,  xr,   y,    dy;    //  was int

} LINESEGMENToffmap;			//  used for Filling regions




////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   textureMapInfo
{
public:
	char*     textName;  

	short     width;   
	short     height;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////


typedef struct  
{  

	short   algoCode;    //  TARGEThUE,    COMPONENTsUBTRACT,    REMAINDERsEPARATION,   OPAQUEsEPARATION,       etc. 


																//		A.   For creation of the Bitmap-Separation

								             
	short   targetHue;		 //  For use with  algoCode =  TARGEThUE
	short   hueSpread;  
	short   valueLimit;


								              //  For use with  algoCode =  COMPONENTsUBTRACT
	short   huntRed;  
	short   huntGreen;  
	short   huntBlue;


	short   componentAlgoTolerance;     //  new set at 10 for default


																//		B.   BELOW are parms for  'RegionCreation'  [ see    RegionDetectorApp::Create_Separation_and_Generate_SubRegions()  

	long    maxPixels4Hole;               //    300;     ***  ADJUST   BIGGER??    

	long    minimumPixelsInRegion;   //    800;      ********  ADJUST   


//	bool    disableErosion;     
	short   morphType;        //   0:  Disable       1: Open         2: Close

		short   erosionDiameter;         //     8[ Not bad ]     12[ too chunky? ]

		short   dialationDiameter;        //  should be a little smaller than erosionDiameter to break up too-big regions



enum   algoTypes     {   TARGEThUE,   COMPONENTsUBTRACT,    REMAINDERsEPARATION,     OPAQUEsEPARATION    };    //   for   algoCode


} SeparationCreateParms;		





////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   OffMap       //   also has some   'Transform'  functions 
{									

														//  (  on Windows  is a  'Device INDEPENDANT'   bitmap  )
public:
	OffMap(   long  wid,   long  heit,    long  depth   );      
	virtual  ~OffMap();


						  	// *** Need this overide of Assignment cause DYNAMIC allocated memory in Members ***  

	virtual   OffMap&   operator=(  OffMap  &other  );			//  new  ASSIGNMENT operator,  is automatically called by ChildClasses
// ********* ADD any new member vars to this function !!!!  **********




															//	  Raster  COPY functions


	virtual   bool			Copy_Bits(   OffMap&  sourceMap,  CString&   retErrorMesg  );      //  ONLY IF maps have same dimensions and ColorTables   

	virtual   bool			Mask_Bits(   OffMap&  maskMap,   short  pixThresh,  short  replaceValue,    bool  greaterThan,						  
																					CString&   retErrorMesg  );   //  ONLY IF maps have same dimensions and ColorTables   

	virtual   bool			Merge_Bits(   OffMap&  otherMap,   CString&   retErrorMesg  );      //  ONLY IF maps have same dimensions and ColorTables   



	virtual   bool			Copy_xSegment(   long  xSRCoffset,   long  xDSToffset,   long width,     OffMap&  sourceMap,   CString&   retErrorMesg  );  


	virtual   void			Copy_Out_Scaled_Bits(                        long  scaleDownFactor,    OffMap&  destMap    );
	virtual   void		    Copy_Out_Scaled_And_Modified_Bits(   long  scaleDownFactor,    OffMap&  destMap,    short  hueOffset,  short  saturationOffset,  short valueOffset   );



	virtual   OffMap*    Make_ScaledDown_Copy(                        long  scaleDownFactor,          CString&   retErrorMesg   );  

	virtual   OffMap*    Make_ScaledDown_ColorModified_Copy(   long  scaleDownFactor,     short  hueOffset,  short  saturationOffset,  short valueOffset,   CString&   retErrorMesg   );



															//   Color  Separation  function


	virtual   bool			Calc_Color_Separation(   SeparationCreateParms&  parms,   OffMap&  sourceMap,   short& retAvgRed,   short& retAvgGreen,  short& retAvgBlue,  CString&  retErrorMesg  );


	virtual   bool			Subtract_Separation(      OffMap&  separationBMap,    CString&   retErrorMesg  );

	virtual   void          Subtract_RegionSubjects_Offmap(   OffMap&  offMapRegion,      RectShrt&  rectRegion,    RectShrt&  rectBigMap,
																			                                                       short  hueOffset,  short  saturationOffset,  short valueOffset 	);

	static    void			Subtract_Pixels_Color(    short  redSrc,     short  greenSrc,     short blueSrc,     //  support function 
								                                     short  redSubtr,  short  greenSubtr,  short blueSubtr,         short&  nuRed,   short&  nuGreen,   short&  nuBlue    );


	virtual   short			Calc_Masks_Coverage_In_Percent();




	virtual   BYTE*         Get_Bits_Start()    {  return  m_bits;  }

	virtual   BITMAPINFO*   Get_BitmapInfo()    {  return   ( BITMAPINFO* )m_dib;   }



	virtual   void		  Clear(  short  Cval   );          // will write all BLACK, or all WHITE



	virtual   void		  Write_Pixel(   long x, long y,     short   rd,  short   gr,  short   bl  );   //  ( MAC Only: MUST call 'Lock_Bits'  & 'Unlock_Bits' before and after )
	virtual   void		  Read_Pixel(    long x, long y,     short *rd,  short *gr,  short *bl  );    //  ( MAC Only: MUST call 'Lock_Bits'  & 'Unlock_Bits' before and after )


	virtual   void		  Read_Pixel_Filtered_Horz(    long x, long y,     short  filterCode,   short  kernalWidth,     short *rd,  short *gr,  short *bl   );   

	virtual   void		  Read_Pixel_ColorModified(    long x, long y,    short  hueOffset,  short  saturationOffset,  short valueOffset,    short&  rd,  short& gr,  short& bl   );  //  new,  8/08




	virtual   short		  Copy_Xcolumn(   long   xSRC,    long  xDST   );

	virtual   void		  Assign_Xcolumn(   long  x,    short  greyVal    );

	virtual   void		  Scroll_Horizontally(    bool  scrollLeft,    long  pixelsToScroll    );   //  ***** NEW,  optimize it    3/10   ******



	virtual   short		  Copy_Yrow(    long  ySRC,    long  yDST    );     //   NEW  11/11     For Vertical DrivingViews  

	virtual   void		  Assign_Yrow(    long  y,    short  greyVal    );

	virtual   void		  Scroll_Vertically(    bool  scrollUp,    long  pixelsToScroll    );





	virtual   void		  Lock_Bits()    {    }   //  dummy on Windows,  necessary on Mac
	virtual   void		  Unlock_Bits()  {    }   
	
	virtual   BOOL	      Is_Empty()      {   if(  m_dib  )    return  false;        //  did we FAIL to ALLOC the memory???
										      else             return  true;   } 	

	virtual   void		  Dump_Max_Column_Values();




	virtual   long		 Get_Ycoord_of_Columns_MaxValue(                              long  x,    short&  retValue    );

	virtual   long		 Get_Ycoord_of_Columns_MaxValue_By_3Components(   long  x,    long&  retComponentColor   );
	virtual   long		 Get_Xcoord_of_Rows_MaxValue_By_3Components(        long  y,    long&  retComponentColor   );




	virtual   short		 Find_Current_Interval(   long  x,     long&  retXstart,   long&  retXend    );

	virtual   long		 Find_Current_Interval_By_3Components(               long  x,    long&  retXstart,  long&  retXend     );     // used by OLD PitchScope, do NOT change it.  4/12



	virtual   long		 Find_Current_Interval_By_3Components_Horizontal(    long  x,    long&  retXstart,   long&  retXend,    bool useNonZeroMode  );

	virtual   long		 Find_Current_Intervals_EndPoints_By_3Components_Horizontal(    long  x,  long  y,    long&  retXstart,   long&  retXend,    bool useNonZeroMode  );  



	virtual   long		 Find_Current_Interval_By_3Components_Vertical(       long  y,    long&  retYstart,    long&  retYend,   bool useNonZeroMode  );   // NEW,  2/12   

	virtual   long		 Find_Current_Intervals_EndPoints_By_3Components_Vertical(    long  x,  long  y,    long&  retYstart,   long&  retYend,    bool useNonZeroMode   );



	virtual   long		 Get_Colors_PairMember_Clique_Score(     long x,  long y,    short  targColor,    short  kernalWidth   );  //  cliques with 2 pixels
	virtual   long		 Get_Colors_SingleMember_Clique_Score(   long x,  long y,    short  targColor,    short  kernalWidth   );   //  cliques with 1 pixel



	static   void       RGB_to_HSV(   short   rd,  short   gr,  short   bl,     short&  retHue,   short& retSaturation,   short&   retValue    );

	static   void		HSV_to_RGB(   short hue,  short saturation,   short value,   short&  retRd,  short&  retGr,  short&  retBl    );
	


	static   short		Subtract_Degrees(   short  number,     short  subtractor  );

	static   short		Get_Degree_Quadrant(   short  degrees   );


	virtual   bool      Fit_To_HSV_Color_OffMap(  short  hueTarg,   short   saturationTarg,   short valueTarg,     bool& retWantsAWhiteRegion,   
																	 short&  retHueOffset,	short&  retSaturationOffset,   short&  retValueOffset,  	CString&   retErrorMesg   );




													//    FLOOD FILL   ops

	static	  short		  Component_Almost_White()     {  return  250;  }    //  this is the   'hiCompValue'   for   Color_Is_Almost_White()  and is used in SeedFill ops   


	virtual   bool		  Color_Is_Almost_White(   short  x,   short y,    short hiCompValue   );   //  support  for Fill routines

	virtual   bool		  Color_Is_Fillable(       short  x,   short y,      short  redTarg,  short greenTarg,  short blueTarg   );



	virtual   bool		  MaskPix_Is_Written(   short  x,   short y,    short  masksWriteVal   );


	virtual   bool		  Seed_Fill_With_White(   int x, int y,     short  redNu,  short greenNu,  short blueNu,      CString&  retErrorMesg  );




	virtual   void		  Push_To_LineSegment_Stack(    short    xl,    short   xr,    short   y,     short   dy,        ListMemry< LINESEGMENToffmap >&  pixelStack    );   // for the TOO SLOW function
	static    void        Pop_From_LineSegment_Stack(   short&  xl,    short&  xr,   short&  y,    short&  dy,       ListMemry< LINESEGMENToffmap >&   pixelStack   );     // for the TOO SLOW function



	virtual   bool        SeedFill_MaskMap_Create_from_NonWhite_Pixels_SLOW(   short x, short y,      bool writeToSRCmapPixels,     short  redNu,  short greenNu,  short blueNu,     
																               short hiCompValue,     OffMap&  maskMap,     RectShrt&  retBoundBox,   CString&  retErrorMesg ); 

	virtual   bool        SeedFill_MaskMap_Create_from_NonWhite_Pixels(   int x, int y,      bool writeToSRCmapPixels,     short  redNu,  short greenNu,  short blueNu,     
																		  short hiCompValue,     OffMap&  maskMap,  OffMap  *totalMaskMap,     RectShrt&  retBoundBox,    
																		  long&  retPixelsChangedCnt,     CString&  retErrorMesg ); 

	virtual   bool        SeedFill_MaskMap_Create_from_Colored_Pixels(    int x, int y,      short  redTarg,  short greenTarg,  short blueTarg,     OffMap&  maskMap,    
											                              OffMap  *totalMaskMap,     RectShrt&   retBoundBox,    long&  retPixelsChangedCnt,   CString&  retErrorMesg    ); 


	virtual   OffMap*     SeedFill_24ColorMap_Create_from_Colored_Pixels(   int x, int y,       short  redTarg,  short greenTarg,  short blueTarg,     
											                                OffMap&  totalMaskMap,    long  minimumPixelsInRegion,   long   minimumMapDimension,
																	        RectShrt&   retBoundBox,    long&  retPixelsChangedCnt,    CString&  retErrorMesg   ); 



	virtual   bool		  Hole_Plug_TwoTone(       long maxPixels,    bool  plugWhitePixelHoles,    CString&  retErrorMesg  );

	virtual   bool		  Do_Open_Morphology(    short  erodeDiameter,   short  dialateDiameter,    CString&  retErrorMesg  );



	virtual   void		  Mask_Blits_Color(   OffMap& mskMap,   OffMap *tMskMap,    RectShrt  *bBox,    short rd, short gr, short bl  );



															//   ***   TEXTURE-MAP  algo  ***

	static	   short               Get_Total_TextureMap_Count();

	static	   textureMapInfo&     Get_TextureMap_Array(  short  index  );


	static   short		  Pick_A_TextureMap(   long  scaleDownThresh,    long  regionsWidth,   long  regionsHeight,    CString&  retErrorMesg   );

	static   void		  Get_TextureMaps_Folder(   CString&  retFolderPath  ); 



													//    total map's  MEASUREMENTS

	virtual   short       Get_Max_Value();


	virtual   long		  Calc_Average_RGB_Components(    short&  retAvgRed,    short&  retAvgGreen,      short&  retAvgBlue      );   //  returns the  'nonWhite PixelCount'
	virtual   long		  Calc_Average_RGB_Components(    short&  retAvgRed,  short&  retAvgGreen,  short&  retAvgBlue,      RectShrt&  boundBoxOriginal,   											 
											                       RectShrt&  boundBoxCurrent,     OffMap *maskMap,     short&  retPercentPixelsMeasured     ); 


	virtual   long		  Calc_Average_HSV_Components(    short&  retHue,         short&  retSaturation,      short&  retValue    );    //  returns the  'nonWhite PixelCount'




	virtual	   void       Set_Seed_FillMode(   bool  doColor  )      {  m_seedFillModeColor =   doColor;   }    //  for the classes STATIC variable   10/08




	static		void	  Add_Mirrored_Line(    short  x,  short  y,    short&  retPixelCount,      ListMemry< LINESEGMENToffmap >&  lineList    );

	static		void      Make_Circular_Kernal(     long  rad,           short&  retPixelCount,     ListMemry< LINESEGMENToffmap >&  lineList    );






public:	 	// ***** CAREFUL:  any new MemberVars must be added to OVERIDE of   'operator='  function   **************


	static  bool  m_seedFillModeColor;    //  static variable for the whole



	long     m_width,  m_height,      m_depth;


	long     m_byRow,   m_totMapBytes;

	long     m_totalAllocatedBytes;      //  need later for  AssignmentOperator


	unsigned char   *m_dib;      //  start of   BITMAPINFOHEADER struct,  and the start of ALL the Allocated memory( 1 allocation )

	unsigned char   *m_bits;     //  Bit-data,  really points INSIDE of   m_dib  memoryBlock ( BETTER for ASSIGN-op  if save numerical offset )


	// ***** CAREFUL:  any new MemberVars must be added to OVERIDE of   'operator='  function  **************
};




////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_OFFMAP_H__1FE3D609_E8C1_4284_9691_2335FFE5172C__INCLUDED_)
