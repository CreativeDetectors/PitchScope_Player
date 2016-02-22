/////////////////////////////////////////////////////////////////////////////
//
//  Gague.h   -   
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#pragma once

///////////////////////////////////////////////////////////////////////////////////////


class  SPitchCalc;

class  Graphic;

class  FontsCd;



////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class  Bullet
{

public:
	Bullet(void);
	~Bullet(void)  {  }

	virtual		void			Draw(   CDC& dc,    COLORREF  color,    short  scalePitch,    short  specialCode   )    {  ASSERT( 0 );   }   //  DUMMY
	virtual		void			Hide(    CDC& dc );

	virtual		bool          Is_Visable()     {  return  m_visible; }  

	virtual		void	        Get_BoundBox(   RectShrt&   rect  );

	virtual       bool			Font_Is_Installed();


public:
	Graphic   m_graphic;     //  For Mouse-HIT  detection to the DrivingView

	bool        m_visible;   

	long		  m_positionID;      //  scalePitch

	short                m_shapeCode;   //   0: Rectangle     1: Oval

	unsigned long   m_backgroundGray;   //   usually Black   [  Could also do RGB colors,   COLORREF is also  a  'unsigned long'   ]    2/12

	short		m_orientationOfGauge;         //   it's PATENT enclosing struct.     0 :  Horizontal,    1 : Vertical 

	FontsCd	   *m_fontPtr;  
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class  GaugeAnimated
{

public:
	GaugeAnimated(void);
	~GaugeAnimated(void);


	virtual		void			                 Initialize_BulletList() =0;

	virtual       ListMemry<Bullet>&    Get_BulletList()      {  return  m_bulletList;  }     


	virtual       bool			Font_Is_Installed();

	virtual       void			Render_Gauges_Background(  CDC& dc  );  



	virtual       void			Render_Gauges_Bullet(   CDC& dc,   short  radialPosition,    short  scalePitch,    COLORREF color,   short  specialCode   ); 

	virtual       void			Erase_Current_Bullet(  CDC& dc  );    

	virtual       void			Erase_All_Bullets(        CDC& dc  );



	virtual		void          Mark_Bullets_Invisible();



//	void			Paint_Bullets_to_Stationary_Gagues();   //  NEW,  to paint the Notes TEXT to Gagues and Revolver after play has stopped.




public:
	ListMemry<Bullet>   m_bulletList;

	FontsCd	   m_fontCustom;         //   my custom  Font  for Musical NotesNames,  as they appear in the XOR Selection BoundBox


	short		m_currentVisibleBulletIdx;


	short   m_width,       m_height;      
	short   m_xOffset,    m_yOffset;
//	Graphic   m_graphic;     // WANT ???


	short		m_orientation;         //   0 :  Horizontal,    1 : Vertical 

	short     m_channelCount;      //    12  scalePitches


	short     m_bulletsShapeCode;    //   0:   Rectangle      1: Oval


	unsigned long   m_backgroundGray;    //   usually Black   [  Could also do RGB colors,   COLORREF is also  a  'unsigned long'   ]    2/12
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class    GaugeDrivingview     :  public   GaugeAnimated
{

public:
	GaugeDrivingview(void);
	~GaugeDrivingview(void);

	virtual		void		Initialize_BulletList();

public:

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class    GaugeRevolver     :  public   GaugeAnimated
{

public:
	GaugeRevolver(void);
	~GaugeRevolver(void);

	virtual		void		Initialize_BulletList();

public:

};



////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class  BulletDrivingview     :  public   Bullet
{

public:
	BulletDrivingview(void);
	~BulletDrivingview(void)  {  }

	virtual		void			Draw(   CDC& dc,    COLORREF  color,    short  scalePitch,    short  specialCode   ); 

//	virtual		void			Hide(    CDC& dc );
//	virtual		bool          Is_Visable()     {  return  m_visible; }  
//	virtual		void	        Get_BoundBox(   RectShrt&   rect  );


public:
//	Graphic   m_graphic;     //  For Mouse-HIT  detection to the DrivingView
//	bool        m_visible;   
//	long		  m_positionID;      //  scalePitch
//	short                m_shapeCode;   //   0: Rectangle     1: Oval
//	unsigned long   m_backgroundGray;   //   usually Black   [  Could also do RGB colors,   COLORREF is also  a  'unsigned long'   ]    2/12
//	FontsCd		*m_fontPtr;  
};




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class  BulletRevolver     :  public   Bullet
{

public:
	BulletRevolver(void);
	~BulletRevolver(void)  {  }

	virtual		void			Draw(   CDC& dc,    COLORREF  color,    short  scalePitch,    short  specialCode   ); 


public:

};
