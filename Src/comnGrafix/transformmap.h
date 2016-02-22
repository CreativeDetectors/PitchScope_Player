/////////////////////////////////////////////////////////////////////////////
//
//  TransformMap.h   -    ALSO...    FowardMapVerter
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_TRANSFORMMAP_H__4B2975E1_70DD_11D6_9A48_00036D156F73__INCLUDED_)
#define AFX_TRANSFORMMAP_H__4B2975E1_70DD_11D6_9A48_00036D156F73__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////////////////////////////////////////////////////




#define  kTOTALdftBANDs   76    //  here cause used as the   'MAXIMUM Rows'    in ANY Transformmap   



		///////////////////////////////////////////////////////////////////////


class  OffMap;

class  FowardMapVerter;



////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

class    TransformMap      :  public   OffMap			//   also known as a   'CHANNEL-map'
{


	//  SHOULD   TransformMaps with CHANNELS always have an ODD number of channels so we 
	//                  can easily find the CENTER val ???     5/02


	//  Try to keep as GENERIC as possible ( try to  'hide'  the AUDIO  aspects of this  )


public:
	TransformMap(    long  width,   long  height,    int  depth,      long  horzScaleDown   );
	virtual ~TransformMap();




	static	      long	      Get_ChunkSize_OLD_PitchScope()    {   return  512;   }     //  Let subclasses overide.                  [ WRONG should be the ONLY DEFINITION for this value   7/06




	virtual	  bool		 Create_UndoMap();
	virtual	  bool		 Create_MaskMap();

	virtual	  void		 Release_UndoMap();
	virtual	  void		 Release_MaskMap();


	virtual	  bool		 Has_Channels()			 	    {   return   m_hasChannels;   }

	virtual	  short		 Get_Channel_Count()         {  return   m_channelCount;  }  	  



	virtual	  short		 Get_Channel_Width()          {  return   m_channelWidth;        }   

	virtual	  short		 Get_DataChannel_Width()   {  return   m_channelDataWidth;  }  



																	// these functs returns SAME row for maps WITHOUT channels
	 virtual   long	  Get_Channels_Top_Yval(        long   channelIdx   );   
	 virtual   long	  Get_Channels_Bottom_Yval(   long   channelIdx   );   
	 
	 virtual   long      Get_Channels_Center_Yval(   long   channelIdx  );  // is same as  Pitch_2Ycoord(  scalePitch  )


	 virtual   long	      Ycoord_2ChannelIdx(  long  y  );     //  coord conversion for   Y's  and  Channels





	 virtual   bool		  Merge_Bits(    TransformMap&  otherMap,    CString&   retErrorMesg    );

	 virtual   void		  ColorFill_Channel_Segment(    long  x0,   long  x1,     short  channelIdx,     short  greyVal    );






	 virtual   bool		  Get_DurationRectangle(   long x,  long y,	   
		                                                           short  pixelThresh,   short  minPixelCount,   
																   long&  retStartPix,    long&  retEndPix,     
																   long&  retChannelIdx,      short&  retAverageRunBriteness,    CString&  retErrorMesg   );



	 virtual   bool		  Apply_Mask(   CString&  retErrorMesg   );


	 virtual   void		  Read_Pixel_Masked(   long x,  long y,     short   filterCode,   short  horzKernalWidth,  
																								short *rd,  short *gr,  short *bl  );    // Get >0 only if maskMap doe NOT have a zeroPixel


	 virtual   short	  Read_MaskMap_Pixel(   long x,   long y   );  




	 virtual   short      Get_Xcolumns_Channel_Value(  long x,  int  channelIdx,  short  pixThresh,   short&  retPixCount  );




													//   a)   new   'FLOAT-MAP'   functs

	 virtual   bool		  Load_Leading_XColumn(   long  xLead,    CString&  retErrorMesg   );




													//   b)   new   'OFFSETTING'   functs  (

	 virtual   bool       Needs_Offsetting();     //   for  'OFFSET-SEGMENT'  mode...  (  for  DetectZone's  backroudmap,  See 	{  m_pixelReadOffset m_pixelReadWidth   }   
															   //  	
															   //   However for NAVIGATOR we use a VARIATION  on  this   SMALL  floating bitmap.   9/2012




public:
	TransformMap   *m_undoMap;    // ***CAREFUL,  dangerous pointer if use built in ASSIGNMENT OPERATOR ****




	TransformMap   *m_maskMap;   //  only 1-bit deep,  monocrome

	bool                   m_maskMapHasData;     //   has the MaskMap been  MADE yet ???





																			 //   CHANNEL  stuff....
	bool	m_hasChannels;

	short    m_channelCount;

	short   m_channelWidth;			  	//  includes 'IN-BETWEEN'  padding
	short   m_channelDataWidth;					



	short   m_channelPad;     //  Subtract   ( m_channelWidth -  m_channelDataWidth ) to get IN-BETWEEN padding

	short   m_topPad;           //   The single padd at the top of a Map( composmap )



	long    m_horzScaleDown;    // how much will this map be scaled in LENGTH, relative to number of total File's SAMPLES ( ? the 'size' of the file )


													   //   a)    'FLOAT-MAP'  mode....
	bool		m_inFloatmapMode;

	long		m_lastLoadedXColumnLeader;    

	short       m_readersOffset;    //  If this is a SourceMap for another READING-map,  the 'reader' must add this to its LOCAl xlead to find the Load_Leading_XColumn() column

	TransformMap	 *m_sourceMapFloat;	
	TransformMap	 *m_sourceMapFloat2;	
	TransformMap	 *m_sourceMapFloat3;

	FowardMapVerter     *m_verterFloatMap;     //   For the  DST-map( 'this' map )   ( not  for SRC-maps   ...though it will access them )




													   //   b)     'OFFSET-SEGMENT'  mode....   (  for  DetectZone's  backgroudmap
	long    m_pixelReadOffset;      
	long    m_pixelReadWidth;    //  USUALLY same as  Offmap::m_height,  but need for Needs_Offsetting() in case
											  //				    Offsetting,  but ( m_pixelReadOffset = 0 )


	enum   stereoChannelTypes        {   LEFTj,   RIGHTj,     CENTEREDj,       SEPARATEstereo      };    //     Keep in SYNC with  'Viewj::'  values




public:
	enum   backgroundMapTypes   
		{   NONE,        FFT,     DFTLOG,    HARMPAIRS,    FUNDAMENTALTEMPLATE,       AUTOCORREL,   
			 AUTOCORRELELLIS,     PERIODOGRAM,     FUNDAMENTALGROUP,    POWER,     HARMONICoBJ,    PITCHEL,  
			 CELLDFT,																			CELLDFTmASKpITCHELS,   //   CELLDFTmASKpITCHELS  is only for reconstruction
			 HARMONICMASK,    SCALEPITCHyMAX,   COMPOSITE,  SCALEPITCH2cANDID,   

			 SCALEPITCHharmonicJUDGEMENT,    	// my current 'ScalPitchMap'  ...is like the 3rd screen in the ScalePitchDetails Dialog  6/07	

			 AMPLITUDEmASK,    AMPLITUDEeDGEmASK,   MIDIsTAFF,    DFTeDGE,     DUMMYmAP         //  a Virtual BackgrroundMap  ...to suppy  GridFunctionality	

		//	,  GFxSRCpANE,   GFxDSTpANE,       ...moved back to Pane for RegionDetectorApp    5/2007

	//					,   MIDIaNIMEmASK,    VOLUMEaNIMEmASK     **** ??? ALSO ????  
		};																				

};






////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////



class   FowardMapVerter             //  ABSTRACT superclass 
{ 		      
	
public:  
	  FowardMapVerter()                  
	  {  
		  m_usingOffsetting    =   false;    //  default 
		  m_pixelReadOffset  =    0L;

//		  m_readingDetectzonesDFT =   false;
	  }

      virtual   ~FowardMapVerter()    
	  {  }



      virtual   void   Transform()  =0;   
      

      virtual   void   Transform_Column(  long x   )	  {   ASSERT( 0 );   }  // DEFAULT dummy   


      virtual   void   Transform_Row(     long y    )     {   }  // DEFAULT dummy   
     



	  
	  virtual   void    Set_ReadOffset(   long   pixelReadOffset   )     {   m_pixelReadOffset =   pixelReadOffset;   
																									m_usingOffsetting =   true;   }

	  virtual   bool    Needs_Offsetting()					 {   return  m_usingOffsetting;   }
		



	/****
	  virtual   bool    Using_DetectZones_logDFT()		                {   return  m_readingDetectzonesDFT;  }    //   reading a SEGMENT of total WAV's logDFT...


	  virtual   void    Enable_DetectZones_logDFT_Reading(   bool  doEnable  )    
															{   
																m_readingDetectzonesDFT =    doEnable;  
															}				
		****/






public: 
	bool    m_usingOffsetting;      //  offsetting into the SOURCEtransform( logDFT ) 

	long    m_pixelReadOffset;    //  when offsetting into the SOURCEtransform's VirtualCoords  for  'partial'  Transforms


//	bool	m_readingDetectzonesDFT; 


		//   TransformMap   *m_sourceMap;      **** INSTALL ??? ******
};





			/////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////



class    InverseMapVerter             //  ABSTRACT superclass 
{ 		           
	
public:  
      virtual   void	   ReConstruct()  =0;   
      
      virtual   void	   ReConstruct_Column(  short x  )   {   }  // DEFAULT dummy   
      virtual   void	   ReConstruct_Row(       short y  )   {   }  // DEFAULT dummy   
      
public:  

};



////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_TRANSFORMMAP_H__4B2975E1_70DD_11D6_9A48_00036D156F73__INCLUDED_)
