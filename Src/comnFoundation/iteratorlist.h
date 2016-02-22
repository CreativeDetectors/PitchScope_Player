/////////////////////////////////////////////////////////////////////////////
//
//  IteratorList.h   -  
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ITERATORLIST_H__16A7FEE4_4FC0_11D3_9507_ACE109C10000__INCLUDED_)
#define AFX_ITERATORLIST_H__16A7FEE4_4FC0_11D3_9507_ACE109C10000__INCLUDED_


#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

template <class Item>    class   List;             




///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////


template <class Item>    class   ListIterator      :  public   Iterator<Item>        
{


public:
	ListIterator(    List<Item>&   aList    )      :    _list( aList ),    _current( 0 )    
	{ }

	virtual ~ListIterator()  
	{  } 



	virtual     void   First()	   
	{   
		_current =  0;	  //  A little redundant( done with constructor ), unless we want to RESET the list
	}



    virtual     void   Next()     
	{							//  This does NOT test for VALIDITY, must later call  Is_Done() to know 
		_current++; 	   
	}



    virtual     BOOL   Is_Done() 
	{   
		if(    _current   >=   _list.Count()    )
			          return   true;
		else          return   false;    
	}



    virtual     Item&   Current_Item()       //   const
	{
		if(     Is_Done()     )
		{
			ASSERT( false );
			return    _list.Get(  0  );    // OK ?????
		}
		else
		{  Item&    curItm =    _list.Get(  _current  );    // ****** OPTIMIZE... this must be slow(has to WALK each time ) ******
			
			return    curItm;
		}
	}


protected:
	 List< Item >&    _list; 
	 long             _current;
};



												////////////////////////////////////////////////////////
												////////////////////////////////////////////////////////
												////////////////////////////////////////////////////////


template <class Item>    class   SpeedIndexIterator      :  public   Iterator<Item>        //  new,  5/2002
{

	/********    ....This is how to Initialize the constructor( get the  *startLink  )  if there are NO indexes:

  	                                       ListLink< ScalepitchView >*    startLink =    m_componentviewListPane.Get_Head_Link();  

					   ...see      Iterator< ComponentView >*    DrivingTestView::Get_Childrens_Iterator()
    ***/


public:
	SpeedIndexIterator(   List< Item >&   aList,      ListLink< Item >   *startLink    )      
														                           :   _list( aList ),   m_currentLink( startLink ) 
	{ 
		ASSERT(   startLink  !=  NULL   );

		m_nextLink     =   m_currentLink->_next;

		m_previousLink =  NULL;
	}


	virtual ~SpeedIndexIterator()  
	{  
	} 



	virtual     void   First()	   
	{   
		//  'm_currentLink'   SHOULD already be initialize to some link in the list  ....OR FAIL !!!     6/2002
	}



    virtual     void   Next()     
	{							//  This does NOT test for VALIDITY, must later call  Is_Done() to know 
		

		m_previousLink =    m_currentLink;   //  NEW,  12/11.    Save this cause sometimes need to go back one.

		m_currentLink  =    m_nextLink;   // this way,  it is OK if  Remove_Item()  turns  m_currentLink  NULL, or invalid


		if(   m_currentLink  !=  NULL  )
			m_nextLink  =   m_currentLink->_next;    //  Fetch the value EARLY, so we could call   Remove_Item()  
		else
			m_nextLink  =   NULL;

	}



    virtual     BOOL   Is_Done() 
	{   
		if(    m_currentLink  ==   NULL    )    
			return  true;
		else
			return  false;
	}



    virtual     Item&   Current_Item()       //   const
	{

		if(     Is_Done()     )
		{
			ASSERT( false );

			Item&     curItm =     m_currentLink->_item;
			return    curItm;    // *** DUMMY return to keep compiler happy  

		}
		else
		{   Item&     curItm =     m_currentLink->_item;
			return    curItm;
		}
	}



public:
	ListLink< Item >   *m_currentLink;

	ListLink< Item >   *m_previousLink;    //   NEW,  12/11     for searches in   SPitchCalc::Get_Pixel_Info_from_Notelist()



protected:
	 List< Item >&       _list; 

	 ListLink< Item >   *m_nextLink;     //   In order for Remove_Item()  to work,  we save the address here
};





						//////////////////////////////////////////////


template <class Item>    class   SpeedIndexDoubleIterator      :  public   Iterator<Item>        //  new,   12/2011
{

		// For new   DOUBLE-Linked   list class   12/2011


	/********    ....This is how to Initialize the constructor( get the  *startLink  )  if there are NO indexes:

  	            ListLink< ScalepitchView >*    startLink =    m_componentviewListPane.Get_Head_Link();  

					   ...see      Iterator< ComponentView >*    DrivingTestView::Get_Childrens_Iterator()
    ***/


public:
	SpeedIndexDoubleIterator(   ListDoubLinkMemry< Item >&   aList,      ListsDoubleLink< Item >   *startLink    )      
														                           :   _list( aList ),   m_currentLink( startLink ) 
	{ 
		ASSERT(   startLink  !=  NULL   );    //  pas in the  HEAD-link  if  just starting

		m_startLink =   startLink;   //  save it to test integrithy


		m_nextLink  =   m_currentLink->_next;
		m_prevLink  =   m_currentLink->_prev;


		m_previousLinkCache =   NULL;
		m_latterLinkCache      =   NULL;
	}



	virtual ~SpeedIndexDoubleIterator()  
	{  
	} 



	virtual     void   First()	   
	{   

		//       'm_currentLink'   SHOULD already be initialize to some link in the list  ....OR FAIL !!!     6/02
		ASSERT(   m_currentLink  ==  m_startLink   );
	}



	virtual     void   Last()	   
	{   
		//  _current =  0;	

		//       'm_currentLink'   SHOULD already be initialize to some link in the list  ....OR FAIL !!!     6/02
		ASSERT(   m_currentLink  ==  m_startLink   );
	}




    virtual     void   Previous()      //  NEW,  12/2011  
	{								
		
						                //  This does NOT test for VALIDITY, must later call  Is_Done() to know 

		m_latterLinkCache =    m_currentLink;     //   Save this cause sometimes need to go back one,  in doing a faster search.   12/11


		m_currentLink     =    m_prevLink;        //   m_nextLink;   // this way,  it is OK if  Remove_Item()  turns  m_currentLink  NULL, or invalid


		if(   m_currentLink  !=  NULL  )
			m_prevLink  =   m_currentLink->_prev;    //  Fetch the value EARLY, so we could call   Remove_Item()  
		else
			m_prevLink  =   NULL;
	}




    virtual     void   Next()     
	{							//  This does NOT test for VALIDITY, must later call  Is_Done() to know 
		

		// ******************************  NEED to only assign if NOT Null ????  ****************************


		m_previousLinkCache =   m_currentLink;   //  NEW,  12/2011.    Save this cause sometimes need to go back one.


		m_currentLink  =    m_nextLink;     // this way,    it is OK if  Remove_Item()  turns  m_currentLink  NULL, or invalid




		if(   m_currentLink  !=  NULL  )
			m_nextLink  =   m_currentLink->_next;    //  Fetch the value EARLY, so we could call   Remove_Item()  
		else
			m_nextLink  =   NULL;
	}




    virtual     BOOL   Is_Done() 
	{   

		if(    m_currentLink  ==   NULL    )    
			return  true;
		else
			return  false;
	}




    virtual     Item&   Current_Item()       //   const
	{

		if(     Is_Done()     )
		{
			ASSERT( false );

		//  return    _list.Get(  0  );    // ****  TERRIBLE,  very slow.  12/11

			Item&     curItm =     m_currentLink->_item;
			return    curItm;    // *** DUMMY return to keep compiler happy  

		}
		else
		{  
		//	Item&    curItm =    _list.Get(  _current  );    // *** OPTIMIZE... this must be slow(has to WALK each time ) ****			
			Item&    curItm =     m_currentLink->_item;

			return    curItm;
		}
	}



public:
	ListsDoubleLink< Item >   *m_currentLink;


	ListsDoubleLink< Item >   *m_previousLinkCache;    //   NEW,  12/11     for  FORWARD  searches in   SPitchCalc::Get_Pixel_Info_from_Notelist()
	ListsDoubleLink< Item >   *m_latterLinkCache;         //   NEW,  12/11     for  FORWARD  searches in   SPitchCalc::Get_Pixel_Info_from_Notelist()


	 ListsDoubleLink< Item >            *m_nextLink;     //  In order for   Remove_Item()   to work, we save the address here
	 ListsDoubleLink< Item >            *m_prevLink;     //   NEW,  12/11.      In order for   Remove_Item()   to work, we save the address here


protected:
	 ListDoubLinkMemry< Item >&      _list; 

	 ListsDoubleLink< Item >       *m_startLink;  
};





												////////////////////////////////////////////////////////
												////////////////////////////////////////////////////////
												////////////////////////////////////////////////////////


template <class Item>    class   ReverseIterator      :  public   Iterator<Item>        
{
										//   NEW,    11/2008

public:
	ReverseIterator(    List<Item>&   aList    )      :    _list( aList )
	{ 
		_current =    _list.Count()  -1;
	}

	virtual ~ReverseIterator()  
	{  } 



	virtual     void   First()	   
	{   
		_current =    _list.Count()  -1; 	  //  A little redundant( done with constructor ), unless we want to RESET the list
	}



    virtual     void   Next()     
	{							//  This does NOT test for VALIDITY, must later call  Is_Done() to know 
		_current--; 	   
	}



    virtual     BOOL   Is_Done() 
	{   
	//	if(    _current   >=   _list.Count()    )
		if(     _current   <  0   )
			return   true;
		else        
			return   false;    
	}



    virtual     Item&   Current_Item()       //   const
	{
		if(     Is_Done()     )
		{
			ASSERT( false );
			return    _list.Get(  0  );    // OK ?????
		}
		else
		{  Item&    curItm =    _list.Get(  _current  );    // ****** OPTIMIZE... this must be slow(has to WALK each time ) ******
			
			return    curItm;
		}
	}



protected:
	 List< Item >&    _list; 
	 long             _current;
};




///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////


#endif // !defined(AFX_ITERATORLIST_H__16A7FEE4_4FC0_11D3_9507_ACE109C10000__INCLUDED_)
