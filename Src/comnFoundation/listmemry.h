/////////////////////////////////////////////////////////////////////////////
//
//  ListsMemry.h   -  
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_LISTS_H__16A7FEE2_4FC0_11D3_9507_ACE109C10000__INCLUDED_)
#define AFX_LISTS_H__16A7FEE2_4FC0_11D3_9507_ACE109C10000__INCLUDED_


#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

																	
template <class Item>    class   ListLink;

template <class Item>    class   ListsDoubleLink;


template <class Item>     class  SpeedIndexIterator;  

template <class Item>     class  SpeedIndexDoubleIterator;  





///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////


template <class Item>    class   ListMemry      :  public    List<Item>        
{

	//  DEFAULT:  creates a list for DYNAMICALLY allocated objects.  Call 



	//  *******************************************************************************
	//
	//  ***IMPORTANT,  can no longer create list of POINTERS !!!   [  ex:  ListMemry< Knob* > 
	//    ...must do  ListMemry< Knob >   ...cause  ListMemry  now uses REFERENCES !!!  ***********  1/2002
	//
	//  *******************************************************************************



		//  ****  INSTALL an exception trowing mechanism if  FAIL(  see get...  )  ****   JPM

   
		//    Add_2Tail()      ...for  CompositePart::Add_Child()     ***INSTALL...



public:   
        ListMemry()  
		{  
			m_hasDynamicObjs =  true;   // **** IMPORTANT:  if use objects that are NOT DYNAMICALLY allocated
													 	  //  MUST call  'Set_Dynamic_Flag(  FALSE )'   right after creation !!!! ******	
			_Count =   0L;  

			_head   =    NULL;   

			m_lastAddedLink =   NULL;
		}      
 		
								////////////////////////////////

		
        virtual  ~ListMemry() 
		{           
			          //  SOMETIMES  call  'Free_Dynamic_Objs()' , if a list of POINTERS to dynamically allocated objects                   
			/***
            if(   m_hasDynamicObjs   )
					                    Free_Dynamic_Objs();			
			Kill_Links();
			***/
			Empty();
		}          



										////////////////////////////////

		ListMemry&   operator=(  ListMemry  &other  )			//  new  ASSIGNMENT operator ...is it OK ???
		{																			 //     ****had problems in  8/2002 [ see AnalyzerAudio::OctaveTrim_SRC_SPitchLists_Aliases()	

			// *** NEED this OVERIDE cause any POINTERS will be wrong when copied by the BuiltIn  Assignment operator ****


			Empty();	// ***Not necessary,  it is empty at this point

			/***
			  ListLink< Item >   *_head;      //  list's head
			  long         _Count;
			  bool        m_hasDynamicObjs;  // If its objects we allocated dynamically, then destructor must ALSO deallocate them
			  ListLink< Item >   *m_lastAddedLink;     //  New, so I can create Indexes   5/02 
			***/

			_head =   NULL;

			m_lastAddedLink =   NULL;   //  Below,  Add_Tail()  will soon write to this


			m_hasDynamicObjs =   other.m_hasDynamicObjs;  // ***** TROUBLE ??( is this obvious to CallingFunct ? ) ********




            ListLink< Item >   *otherTrav =     other._head;
   

            for(    long  i=0;     i <  other._Count;      i++    )  
            {															
				ASSERT(  otherTrav );

                Item&   otherItem =   otherTrav->_item;  // get address stored in 'Item', assign to pointer                                              



				Item    *nuItem =    new    Item(  otherItem  );   // *** CopyConstructor,  or use Assignment operator ??? *****  
// **** TROUBLE???   ...what if my 'Item' has a BAD,  BUILT-IN Copy constructor ??? **********  8/02


				Add_Tail(   *nuItem   );

                otherTrav =   otherTrav->_next;      // cur was NOT destroyed, just its  'POINTED TO' object
             }


			return   *this;    //  allows     destlist =  destlist  -  cloneList 
		}



										////////////////////////////////

		/*********   maybe finish some day....  Read notes below


		ListMemry&   operator-(  ListMemry  &other  )		
		{
			m_hasDynamicObjs =   other.m_hasDynamicObjs;

            ListLink< Item >   *otherTrav =     other._head;
   
            for(    long  i=0;     i <  other._Count;      i++    )  
            {															
				ASSERT(  otherTrav );
                Item&   otherItem =   otherTrav->_item;  // get address stored in 'Item', assign to pointer                                              

				Delete_If_Same(  otherItem  );  //  BIG PROBLEM, mus add many operators( == ) to esisting classes

                otherTrav =   otherTrav->_next;      // cur was NOT destroyed, just its  'POINTED TO' object
             }
			return   *this;    //  allows     destlist =  destlist  -  cloneList 
		}


										////////////////////////////////

        virtual     void   Delete_If_Same(   Item&  otherItem   )    
		{ 
					 //  BIG PROBLEM, mus add many operators( == ) to esisting classes

            ListLink< Item >   *trav =    _head;
   
            while(    trav  !=  NULL    )
            {															
				ASSERT(  trav );
                Item&                   item   =     trav->_item;  // get address stored in 'Item', assign to pointer                                              
				ListLink< Item >   *next  =     trav->_next;   // hod this before deletion

				if(    otherItem  ==   item    )   //  Will this equality operator work on ALL my  objects
					Remove_Item(  item  );

                trav =   next;      
             }
		}
		***/

								////////////////////////////////


        virtual     void   Set_Dynamic_Flag(  bool  isDynamic  )    
		{ 
			//  call  'Set_Dynamic_Flag(  FALSE )' , if a list will NOT be composed of DYNAMICALLY allocated objects
			
			m_hasDynamicObjs =  isDynamic;
        }     
		


								////////////////////////////////

	  
        virtual     void   Kill_Links()    
		{                 // Kills ONLY links if list of pointers( NOT    DYNAMIC   allocd OBJECTS  [ 'new' ]   ), but if
                                 // 'Item' is not a pointer, should dealloc all memory of list.
            while(    !Is_Empty()    )     
			{
				//  Remove_Head(); 
				Remove_Head_LinkOnly();
			}
        }     

		

								////////////////////////////////

	  
        virtual     void   Empty()       //   NEW,  is it OK ????
		{                 
            if(   m_hasDynamicObjs   )
					                    Free_Dynamic_Objs();			
			Kill_Links();
        }     



								////////////////////////////////


		virtual     void   Remove_Head()      // only kills  BOTH  the LINK and its  'POINTED TO' object ( if Dynamic )  
		{

					// *** CHANGED  1/02 to also delet the item,  careful with old Mac code!!! ******

                //  Item                    retval  =   _head->_item;
                   ListLink<Item>  *temp   =   _head;
                  
                  
                  _head =  _head->_next;    // assign '_head' the next link's value



					if(    m_hasDynamicObjs    )     // ***** TEST,  is this OK ????     1/02
					{

						Item&   itmHead =   temp->_item;  // get address stored in 'Item', assign to pointer    	
						
						delete   &itmHead;    // free the link's  'POINTED TO' object 
					}



                  delete  temp;          // delete the old _head LINK
                  
                  _Count--;
		}
 


								////////////////////////////////


		virtual     void   Remove_Head_LinkOnly()      // only kills the LINK, not its  'POINTED TO' object  
		{

 
			ListLink<Item>  *temp =  NULL;
                  

			if(                                _head    !=    NULL  )					  //  hold a VALID address for deletion  
			//	&&    (( unsigned int )_head)   !=    0xdddddddd   ) 
				temp  =    _head;



			if(                                  _head->_next      !=     NULL   )   //  try to REassign  _head  to a VALID  '_next'  address
				// &&    (( unsigned int )(_head->_next))    !=     0xdddddddd   )
				_head =   _head->_next;        //   REassign  '_head'   the   next link's   value
			else
				_head =   NULL;
                  


			if(    temp  !=  NULL   )
			{
				delete  temp;          // delete the old _head LINK
                  
				if(    _Count  >  0    )    // do NOT let it get a negative value
					_Count--;
			}
		}




								////////////////////////////////
		

		void   Free_Dynamic_Objs()   
		{      
					 //  **** Sometimes want to RELEASE the list( of dynamic objs ) without KILLING the list itself, 
					 //       so call:    Free_Dynamic_Objs();   Kill_Links();      ...and continue using the list.

                   // only CALL if a  'list of POINTERs'  to dynamically alloc memory OBJECTs
                  

			long             i;       
            ListLink< Item >  *cur= _head;
    
            for(   i=0;   i< _Count;    i++  )  
            {
															//   "Item&"       NEW as a reference .... OK ?????

                Item&   itm =   cur->_item;  // get address stored in 'Item', assign to pointer                       
                       
                delete   &itm;    // free the link's  'POINTED TO' object 
                    
                cur =   cur->_next;      // cur was NOT destroyed, just its  'POINTED TO' object
             }
		}
		


								////////////////////////////////


		void    Remove_Item(   Item&   obj   )
		{

			if(   _Count  <=  0    )
			{
				//   ASSERT( 0 );   ...no big deal.   SOMETIMES user tried to remove an item that was NEVER added to the list.
				return;  
			}
						
			
			if(    &( _head->_item )  ==   &obj    )
			{
				Remove_Head();
				return;
			}



			ListLink<Item>  *cur,   *prev; 
			bool       notFound =  true;

			cur =      _head->_next;
			prev =    _head;  


			while(       notFound   
					&&   cur  !=  NULL   )
			{

//			   if(     &obj  ==        cur->_item   )
				if(    &obj   ==    &( cur->_item )    )		//  ***get  May 4th  bug here ******
				{
					
					prev->_next =   cur->_next;      //  correct linking by bypassing the intended delete
				


					if(    m_hasDynamicObjs    )   // ***** TEST,  is this OK ????  
					{

						Item&   itm =   cur->_item;  // get address stored in 'Item', assign to pointer    	
						
						delete   &itm;    // free the link's  'POINTED TO' object 
					}



					delete   cur;		  //  now delete the LINK
					_Count--;

					notFound =  false;
				}
				else
				{  prev =   cur;				//  save for relinking  
					cur   =   cur->_next;  
					
					if(   cur  ==  NULL    ) 
					{
						notFound =   false;			//  ...say this,  just to EXIT the loop

						ASSERT( 0 );   // we failed to FIND the item  [ Get here if Hit the Undo Menu command when nothing left to undo  9/06  
					}
				}
			}   //  while(       notFound   
		}



												////////////////////////////////


		bool    Is_Contained(   Item&   itm   )       
        {                       		

			
				//	  ****   ...the OLD mac version ..do I need it ?		[  too close to  ListLink,  let the List object deal with that   1/02 ]

											//  Get 'link's address' for MATCHING item 
			ListLink<Item>   *cur,    *fnd= NULL;
    
    		if(    Count()  ==  0   )     
				return  false; 
			

    		 cur =   _head;
			
			 do
			 {  
				if(   &itm   ==    &( cur->_item )    )
				{
					fnd =   cur;
					break;
				}
              			 
				cur =   cur->_next; 

			 }while((   cur !=  NULL   )&&(   fnd ==  NULL   ));
         		    

			//  return  fnd;      			

			if(   fnd   !=   NULL   )
				return  true;
			else
				return  false;
		}
	


										////////////////////////////////


		ListLink< Item >*    Find(   Item&   itm   )       
        {                       		

			
		//	  ****   ...the OLD mac version ..do I need it ?		[  too close to  ListLink,  let the List object deal with that   1/02 ]

											//  Get 'link's address' for MATCHING item 
			ListLink<Item>   *cur,    *fnd= NULL;
    
    		if(    Count()  == 0   )     
				return  NULL; 
			

    		 cur =   _head;
			
			 do
			 {  
		   //   if(     cur->item  ==   itm   )     
				if(   &itm   ==    &( cur->_item )    )
					fnd =   cur;
              			 
				cur =   cur->_next; 

			 }while((   cur !=  NULL   )&&(   fnd ==  NULL   ));
         		    

			return  fnd;      			
		}
	




		long    Find_Index(   Item&   itm   )       //  NEW,  returns the index, and user can use  Get( index )
        {                       		

											//  Get 'link's address' for MATCHING item 
			ListLink<Item>   *cur,    *fnd= NULL;
			long                    index =  -1L;
    
    		if(     Count() == 0    )     
				return   -1; 
			

    		 cur    =   _head;
			 index =   0;
			
			 do
			 {  
				//  if(     cur->item  ==   itm   )     
				if(    &itm   ==    &( cur->_item )    )
					fnd =   cur;
				else
				{  cur =   cur->_next; 
					index++;
				}
              			 				

			 }while((   cur !=  NULL   )&&(   fnd ==  NULL   ));
         		    

			return  index;      			
		}



								////////////////////////////////


        virtual     long    Count()   const     
		{ 
			return   _Count;  
		}   
 

								////////////////////////////////


		virtual     Item&   Get(  long  index  )   //  a  SLOW function  ( could I optimize ??? )
		{  

									 //  Gets a 'REFERENCE' to 'Link's ITEM' ( which itself might  be a pointer )    
			
			ListLink<Item>  *cur; 
		    long            i;      
		 
   
			if(    ( index +1 )   >  Count()    )      //  Index is zero-based 
			{
				ASSERT( false );
				return   _head->_item;       // error,     ****  BAD, no error handling    INSTALL ****
			}
            else  
			{  cur =  _head;
                             
			    for(    i=0;    i< index;     i++   )       //  SLOW  ...have to walk the ENTIRE list.
						cur =   cur->_next;             
				
				return    cur->_item;
			}
		}



								////////////////////////////////


		void       ReAssign_Links_Item(   ListLink< Item >&   lnk,    Item&  nuItem    )   
		{ 
																   //   for 'SORTING items' in list 

			ListLink<Item>   *nextLink =    lnk._next;   

			ListLink<Item>   *priorLink  =   NULL;


			if(   _head   !=   &lnk    )    //   If head is NOT the targetLink,  find the link BEFORE the one we work on
			{
				priorLink =   _head;

				while(     priorLink->_next    !=    &lnk   )
					priorLink =   priorLink->_next;      
			}



			delete   &lnk;   //  must kill the old link,  cause we now use REFERENCES and can NOT reassign a reference.


			ListLink< Item >*   nuLink  =      new   ListLink< Item >(   nextLink,   nuItem   );   // Create and set 1 link
 
			if(     priorLink  !=   NULL   )
				priorLink->_next  =     nuLink;   //  now assign the 2nd link
			else
				_head =    nuLink;      //  If it is the HEAD we must delete,  then reassign to it
		}




								////////////////////////////////

		
		virtual     Item&   First()   //  const
 		{  
									 //  Gets a 'POINTER' to 'Link's Item' ( which itself might  be a pointer )     
			if(   Is_Empty()    )   
			{
				ASSERT( false );
				return   _head->_item;       // error,     ****  BAD, no error handling    INSTALL ****
			}
            else   
				return    _head->_item;
		}
 


								////////////////////////////////


		virtual     Item&   Last()   //  const
		{  
									 //  Gets a 'REFERENCE' to 'Link's ITEM' ( which itself might  be a pointer )     
			ListLink<Item>  *cur; 
   
			if(    Is_Empty()    )   
			{
				ASSERT( false );
				return  _head->_item;       // error,     ****  BAD, no error handling    INSTALL ****
			}
            else  
			{  bool     bailOut =   false;
				cur =    _head;
                             
				while(   !bailOut   )
				{
					if(   cur->_next  !=   NULL  )
								    cur        =   cur->_next;  
					else          bailOut =   true;
				}

				return     cur->_item;
			}
		}


								////////////////////////////////
       

		virtual     void   Add_Head(   Item  &obj   )  
		{  
				// *************   Is this right???    SHOULD I not be checking that  -head is not NULL,  and pushing forward ????  ************** 

            _head  =    new    ListLink<Item>(   _head,   obj   );   
			_Count++;

			m_lastAddedLink =   _head;     //  New, so I can create Indexes  
		}




								////////////////////////////////


		virtual    void      Add_Tail(   Item  &obj   )      // ****** NEW,  2/02 *****
		{
  
			if(    Is_Empty()    )   
			{
				Add_Head(  obj   );  

				m_lastAddedLink =   _head;
			}
            else  
			{  ListLink< Item >  *cur =  _head;
				                             
				while(    cur->_next   !=   NULL    )
					cur  =   cur->_next;  


				ListLink<Item>  *nuLink =    new    ListLink<Item>(   NULL,  obj   );   

				cur->_next =   nuLink;


				m_lastAddedLink =   nuLink;   //  New, so I can create Indexes  


				_Count++;
			}
		}
      

								////////////////////////////////
	        
     
      virtual     BOOL   Is_Empty()  const       
	  { 
		  if(   _Count   <=   0L    )     
						return   true;
		  else		 return   false;
	  }



								////////////////////////////////
	        
     
      virtual     ListLink< Item >*   Get_Last_Added_Link()  
	  { 
		  return   m_lastAddedLink;
	  }


	  								////////////////////////////////
	        
     
      virtual     ListLink< Item >*   Get_Head_Link()  
	  { 
		  return   _head;
	  }




								////////////////////////////////
								////////////////////////////////


		virtual      void   Push(   Item  &obj   )  
		{  
            _head  =    new    ListLink<Item>(   _head,   obj   );   
			_Count++;
		}


								////////////////////////////////


		virtual     Item&   Pop()      // only kills the LINK, not its  'POINTED TO' object 
												//
		{										//   TRICKY, if this list has Dynamic allocated objects, calling function needs to delete the object, or memory leak   10/08
												//   See:    OffMap::Pop_From_LineSegment_Stack()
		
             Item&                  retval  =   _head->_item;
             ListLink<Item>  *temp   =   _head;
                  
             _head =  _head->_next;    // assign '_head' the next link's value
                  
             delete  temp;          // delete the old _head LINK
                  
             _Count--;

			 return   retval;     // *** CAREFUL,  untested    ******
		}



		virtual     Item&   Top()
 		{  
			if(   Is_Empty()    )   
			{
				ASSERT( false );
				return   _head->_item;       // error,     ****  BAD, no error handling    INSTALL ****
			}
            else   
				return    _head->_item;
		}



   
public:
// ***** CAREFUL:  any new MemberVars must be added to OVERIDE of   'operator='  function   **************


      ListLink< Item >   *_head;      //  list's head

      long         _Count;

	  bool        m_hasDynamicObjs;  // If its objects we allocated dynamically, then destructor must ALSO deallocate them



	  ListLink< Item >   *m_lastAddedLink;     //  New, so I can create Indexes   5/02 

//	  Item      _nullItem;    // ***  to return bad value  **** INSTALL  [  need  InitializeNullItem()  ???  ]


// ***** CAREFUL:  any new MemberVars must be added to OVERIDE of   'operator='  function   **************
};






		////////////////////////////////////////////////////


template <class Item>    class   ListDoubLinkMemry      :  public    List<Item>        
{

	//  DEFAULT:  creates a list for DYNAMICALLY allocated objects.  Call 
	//
	//		Has DOUBLE LINKS, so is good for reverse searches    12/2011


	//  ******  MANY of these function are untested, I created this in a rush.  12/7/11 ******************************



	//  *******************************************************************************
	//
	//  ***IMPORTANT,  can no longer create list of POINTERS !!!   [  ex:  ListMemry< Knob* > 
	//    ...must do  ListMemry< Knob >   ...cause  ListMemry  now uses REFERENCES !!!  ***********  1/2002
	//
	//  *******************************************************************************

		//  ****  INSTALL an exception trowing mechanism if  FAIL(  see get...  )  ****   JPM

 
public:   
        ListDoubLinkMemry()  
		{  
			m_hasDynamicObjs =  true;   // **** IMPORTANT:  if use objects that are NOT DYNAMICALLY allocated
													 	  //  MUST call  'Set_Dynamic_Flag(  FALSE )'   right after creation !!!! ******	
			_Count =   0L;  

			_head   =   _tail  =    NULL;   

			m_lastAddedLink =   NULL;

			m_lastFoundCalcNoteLink =  NULL;
		}      
 		
								////////////////////////////////

		
        virtual  ~ListDoubLinkMemry() 
		{           
			Empty();
		}          


										////////////////////////////////


		ListDoubLinkMemry&   operator=(  ListDoubLinkMemry  &other  )			//  new  ASSIGNMENT operator ...is it OK ???
		{																			 
			                                     //     ****had problems in  8/2002 [ see AnalyzerAudio::OctaveTrim_SRC_SPitchLists_Aliases()	

			// *** NEED this OVERIDE cause any POINTERS will be wrong when copied by the BuiltIn  Assignment operator ****


	ASSERT( 0 );   // ******* TEST this and rewrite  **************************************


			Empty();	// ***Not necessary,  it is empty at this point

			/***
			  ListLink< Item >   *_head;      //  list's head
			  long         _Count;
			  bool        m_hasDynamicObjs;  // If its objects we allocated dynamically, then destructor must ALSO deallocate them
			  ListLink< Item >   *m_lastAddedLink;     //  New, so I can create Indexes   5/02 
			***/

			_head =   NULL;

			m_lastAddedLink =   NULL;   //  Below,  Add_Tail()  will soon write to this


			m_hasDynamicObjs =   other.m_hasDynamicObjs;  // ***** TROUBLE ??( is this obvious to CallingFunct ? ) ********




            ListsDoubleLink< Item >   *otherTrav =     other._head;
   

            for(    long  i=0;     i <  other._Count;      i++    )  
            {															
				ASSERT(  otherTrav );

                Item&   otherItem =   otherTrav->_item;  // get address stored in 'Item', assign to pointer                                              



				Item    *nuItem =    new    Item(  otherItem  );   // *** CopyConstructor,  or use Assignment operator ??? *****  
// **** TROUBLE???   ...what if my 'Item' has a BAD,  BUILT-IN Copy constructor ??? **********  8/02


				Add_Tail(   *nuItem   );

                otherTrav =   otherTrav->_next;      // cur was NOT destroyed, just its  'POINTED TO' object
             }


			return   *this;    //  allows     destlist =  destlist  -  cloneList 
		}



										////////////////////////////////


        virtual     void   Set_Dynamic_Flag(  bool  isDynamic  )    
		{ 
			//  call  'Set_Dynamic_Flag(  FALSE )' , if a list will NOT be composed of DYNAMICALLY allocated objects
			
			m_hasDynamicObjs =  isDynamic;
        }     
		


								////////////////////////////////

	  
        virtual     void   Kill_Links()    
		{                 // Kills ONLY links if list of pointers( NOT    DYNAMIC   allocd OBJECTS  [ 'new' ]   ), but if
                                 // 'Item' is not a pointer, should dealloc all memory of list.
            while(    ! Is_Empty()    )     
			{
				//  Remove_Head(); 
				Remove_Head_LinkOnly();
			}
        }     

		

								////////////////////////////////

	  
        virtual     void   Empty()       //   NEW,  is it OK ????    Need full Initalization for this ( like setting   m_lastFoundCalcNoteLink   to NULL  )   2/12
		{                 
            if(    m_hasDynamicObjs    )    // Previous got bugs were m_lastAddedLink was NOT NULL, but still had a bad value.   2/12 
				Free_Dynamic_Objs();	

			Kill_Links();


		// ***************************************   NEW,  2/27/2012     Is this a problem?   We need through initialization  ************************
			_head   =   _tail  =    NULL;   

			m_lastAddedLink =   NULL;

			m_lastFoundCalcNoteLink =  NULL;

		// ***************************************   NEW,  2/27/2012     Is this a problem?   We need through initialization  ************************
        }     




								////////////////////////////////


		virtual     void   Remove_Head()      // only kills  BOTH  the LINK and its  'POINTED TO' object ( if Dynamic )  
		{

					// *** CHANGED  1/2002 to also delet the item,  careful with old Mac code!!! ******


					if(    _head  ==  NULL   )
					{
						ASSERT( 0 );
						return;
					}


                   ListsDoubleLink<Item>  *temp   =   _head;
                  

                  
                  _head =  _head->_next;    // assign '_head' the next link's value    ....might be NULL ??? 


						if(   _head  ==  NULL  )
						{
							_tail =  NULL;
						}
						else        
						{			    //  _head is NOT null

							_head->_prev =    NULL;      //    *****  NEW,  OK ???    12/11


							if(    _head->_next   ==   NULL   )
								_tail =    _head;
						}




					if(    m_hasDynamicObjs    )     // ***** TEST,  is this OK ????     1/02
					{

						Item&   itmHead =   temp->_item;  // get address stored in 'Item', assign to pointer    	
						
						delete   &itmHead;    // free the link's  'POINTED TO' object 
					}



                  delete  temp;          // delete the old _head LINK
                  
                  _Count--;
		}
 




		virtual     void   Remove_Head_LinkOnly()      // only kills the LINK, not its  'POINTED TO' object  
		{


					//   ASSERT( 0 );     ***  TESTED on  1/18/12  ,   seems fine  ***

 
			ListsDoubleLink<Item>  *temp =  NULL;
                  

			if(                                _head    !=    NULL  )					  //  hold a VALID address for deletion  
			//	&&    (( unsigned int )_head)   !=    0xdddddddd   ) 
				temp  =    _head;



			if(                                  _head->_next      !=     NULL   )   //  try to REassign  _head  to a VALID  '_next'  address
			{
				// &&    (( unsigned int )(_head->_next))    !=     0xdddddddd   )
				_head =   _head->_next;        //   REassign  '_head'   the   next link's   value

				_head->_prev  =  NULL;   
			}
			else
				_head =   NULL;
                  


			if(    temp  !=  NULL   )
			{
				delete  temp;          // delete the old _head LINK
                  
				if(    _Count  >  0    )    // do NOT let it get a negative value
					_Count--;
			}
		}




								////////////////////////////////
		

		void   Free_Dynamic_Objs()   
		{      
					 //  **** Sometimes want to RELEASE the list( of dynamic objs ) without KILLING the list itself, 
					 //       so call:    Free_Dynamic_Objs();   Kill_Links();      ...and continue using the list.

                   // only CALL if a  'list of POINTERs'  to dynamically alloc memory OBJECTs
                  

			long             i;       
            ListsDoubleLink< Item >  *cur= _head;
    
            for(   i=0;   i< _Count;    i++  )  
            {
															//   "Item&"       NEW as a reference .... OK ?????

                Item&   itm =   cur->_item;  // get address stored in 'Item', assign to pointer                       
                       
                delete   &itm;    // free the link's  'POINTED TO' object 
                    
                cur =   cur->_next;      // cur was NOT destroyed, just its  'POINTED TO' object
             }
		}
		


								////////////////////////////////


		void    Remove_Item(   Item&   obj   )
		{

			      //  Looks like it is working OK,  since the double-link and  _tail  enhancements.   2/2012


			if(   _Count  <=  0    )
			{
				//   ASSERT( 0 );   ...no big deal.   SOMETIMES user tried to remove an item that was NEVER added to the list.
				return;  
			}
						
			
			if(    &(  _head->_item  )  ==   &obj    )
			{
				Remove_Head();
				return;
			}



			ListsDoubleLink<Item>  *cur,   *prev,   *next; 
			bool       notFound =  true;

			cur =      _head->_next;
			prev =    _head;  



			while(          notFound   
					   &&   cur  !=  NULL   )
			{

//			   if(      &obj  ==          cur->_item  )
				if(     &obj   ==    &(  cur->_item  )     )		//  ***get  May 4th  bug here  2012  ******
				{
					

					next =    cur->_next;    // ***********   NEW


					bool   curIsTail =  false;

					if(   cur->_next  ==   NULL   )
						 curIsTail =   true;




			//		prev->_next =   cur->_next;      //  correct linking by bypassing the intended delete
					prev->_next =   next; 

					    if(    next   !=  NULL  )    //   happens if Cur is the 
							next->_prev =   prev;   // *******************   TESTED   2/4/2012




					if(    curIsTail    )
						_tail  =    prev;       //  reassign  _tail  if necessary  




					if(    m_hasDynamicObjs    )   // ***** TEST,  is this OK ????  
					{

						Item&   itm =   cur->_item;  // get address stored in 'Item', assign to pointer    	
						
						delete   &itm;    // free the link's  'POINTED TO' object 
					}



					delete   cur;		  //  now delete the LINK
					_Count--;

					notFound =  false;
				}

				else
				{  prev =   cur;				//  save for relinking  

					cur   =   cur->_next;  
					
					if(   cur  ==  NULL    ) 
					{
						notFound =   false;			//  ...say this,  just to EXIT the loop

						ASSERT( 0 );   // we failed to FIND the item  [ Get here if Hit the Undo Menu command when nothing left to undo  9/06  
					}
				}

			}   //  while(       notFound   

		}




												////////////////////////////////


		bool    Is_Contained(   Item&   itm   )       
        {                       		

			
//	  ****   ...the OLD mac version ..do I need it ?		[  too close to  ListsDoubleLink,  let the List object deal with that   1/02 ]

											//  Get 'link's address' for MATCHING item 
			ListsDoubleLink<Item>   *cur,    *fnd= NULL;
    
    		if(    Count()  ==  0   )     
				return  false; 
			

    		 cur =   _head;
			
			 do
			 {  
				if(   &itm   ==    &( cur->_item )    )
				{
					fnd =   cur;
					break;
				}
              			 
				cur =   cur->_next; 

			 }while((   cur !=  NULL   )&&(   fnd ==  NULL   ));
         		    

			//  return  fnd;      			

			if(   fnd   !=   NULL   )
				return  true;
			else
				return  false;
		}
	


								////////////////////////////////


        virtual     long    Count()   const     
		{ 
			return   _Count;  
		}   
 


								////////////////////////////////

		
		virtual     Item&   Get(  long  index  )   //  a  SLOW function  ( could I optimize ??? )
		{  

									 //  Gets a 'REFERENCE' to 'Link's ITEM' ( which itself might  be a pointer )    
			
			ListsDoubleLink<Item>  *cur; 
		    long            i;      
		 
   
			if(    ( index +1 )   >  Count()    )      //  Index is zero-based 
			{
				ASSERT( false );
				return   _head->_item;       // error,     ****  BAD, no error handling    INSTALL ****
			}
            else  
			{  cur =  _head;
                             
			    for(    i=0;    i< index;     i++   )       //  SLOW  ...have to walk the ENTIRE list.
						cur =   cur->_next;             
				
				return    cur->_item;
			}
		}



								////////////////////////////////


		void       ReAssign_Links_Item(   ListsDoubleLink< Item >&   lnk,    Item&  nuItem    )   
		{ 
																   //   for 'SORTING items' in list 


	ASSERT( 0 );   // *********************  REWRITE and TEST    12/7/11,   this is bad  ******************



			ListsDoubleLink<Item>   *nextLink =    lnk._next;   

			ListsDoubleLink<Item>   *priorLink  =   NULL;


			if(   _head   !=   &lnk    )    //   If head is NOT the targetLink,  find the link BEFORE the one we work on
			{
				priorLink =   _head;

				while(     priorLink->_next    !=    &lnk   )
					priorLink =   priorLink->_next;      
			}



			delete   &lnk;   //  must kill the old link,  cause we now use REFERENCES and can NOT reassign a reference.


// ****************   THIS CONSTRUCTOR look wrong ( not enough parms..  why no compiler error ???   12/11   ****************************

			ListsDoubleLink< Item >*   nuLink  =      new   ListsDoubleLink< Item >(   nextLink,   nuItem   );   // Create and set 1 link
 

			if(     priorLink  !=   NULL   )
			{
				priorLink->_next  =     nuLink;   //  now assign the 2nd link
			}
			else
			{	_head =    nuLink;      //  If it is the HEAD we must delete,  then reassign to it

			}

		}



								////////////////////////////////


		void       Swap_Neighbor_Objects_ListOrder(    ListsDoubleLink<Item>&   priorLink,      ListsDoubleLink<Item>&   curLink    ) 
		{ 

									
			          //  ****  NEW,  1/2012   use this GREAT new algo!!!!        for   Bubble 'SORTING'  items in a list  *******************


			ListsDoubleLink< Item >    *linkAfterCurlink =   curLink._next;   //  might be NULL

			ListsDoubleLink< Item >    *priorPriorLink    =   priorLink._prev;   //  might be NULL


			bool  curLinkWasTail =   false;

			 if(     curLink._next  ==   NULL     )
				 curLinkWasTail =   true;



			if(    &priorLink   ==    _head    )    //  special case for _head
			{


				_head =   &curLink;    //  assign it as the Head

					curLink._prev =  NULL;               //   link  A    (  because  curLink   is now at the head 



				priorLink._next =     linkAfterCurlink;   //  link  C

					if(    linkAfterCurlink  !=  NULL   )
						linkAfterCurlink->_prev =   &priorLink;    



				curLink._next  =     &priorLink;           //  link  B

					priorLink._prev =    &curLink;


																			
				if(   curLinkWasTail   )
				{
					_tail  =    &priorLink;    // Since we swapped their order, now  priorLink   becomes the new TAIL of list  
						
					ASSERT(   priorLink._next  ==  NULL  );
				}
			}

			else
			{ 
				priorPriorLink->_next =    &curLink;				 //   link  A

					curLink._prev =    priorPriorLink;



				priorLink._next  =     linkAfterCurlink;        //   link  C   

					if(    linkAfterCurlink  !=  NULL   )
						linkAfterCurlink->_prev  =    &priorLink;



				curLink._next  =     &priorLink;                   //   link  B

					priorLink._prev =    &curLink;



				if(    curLinkWasTail    )
				{
					_tail  =    &priorLink;    // Since we swapped their order, now  priorLink   becomes the new TAIL of list  
						
					ASSERT(   priorLink._next  ==  NULL  );
				}
			}			
		}




								////////////////////////////////

		
		virtual     Item&   First()   //  const
 		{  
									 //  Gets a 'POINTER' to 'Link's Item' ( which itself might  be a pointer )     
			if(   Is_Empty()    )   
			{
				ASSERT( false );
				return   _head->_item;       // error,     ****  BAD, no error handling    INSTALL ****
			}
            else   
				return    _head->_item;
		}
 


								////////////////////////////////


		virtual     Item&   Last()   //  const
		{  

									 //  Gets a 'REFERENCE' to 'Link's ITEM' ( which itself might  be a pointer )     
			
  
			if(    Is_Empty()    )   
			{
				ASSERT( false );
				return  _head->_item;       // error,     ****  BAD, no error handling    INSTALL ****
			}
            else  
			{ 
				if(   _tail  ==  NULL   )
				{ 
					ASSERT( 0 );

					if(   _head ==  NULL   )
					{
						ASSERT( 0 );
					}

					return   _head->_item;
				}
				else
					return   _tail->_item;   // Normal,  should always hit this
			}
		}



								////////////////////////////////
       

		virtual     void   Add_Head(   Item  &obj   )  
		{  

			              //  	ListsDoubleLink(     ListsDoubleLink<Item>  *next,      ListsDoubleLink<Item>  *prev,           Item&  itm    )     

			 ListsDoubleLink< Item >  *nuLink  =     new    ListsDoubleLink<Item>(   _head,   NULL,  obj   );   



			if(   _head  ==  NULL    )   
			{
				_head  =   nuLink;

				_tail    =   nuLink;    //  this is the first Item in the list
			}
			else
			{	nuLink->_next  =    _head;

				_head->_prev  =    nuLink;

				_head  =   nuLink;    //  now save to change the assignment of    _head
			}



			_Count++;

			m_lastAddedLink =    nuLink;          //     _head;    ***** OK ?????????                     //  New, so I can create Indexes  
		}




								////////////////////////////////


		virtual    void      Add_Tail(   Item  &obj   )      
		{
  
			if(    Is_Empty()    )   
			{

				Add_Head(  obj   );  


				m_lastAddedLink =   _head;

				_tail =     _head;  
			}
            else  
			{  
				ListsDoubleLink< Item >  *cur =  _tail;

				ASSERT(  _tail  !=  NULL  );



				ListsDoubleLink<Item>  *nuLink =    new    ListsDoubleLink<Item>(   NULL,   NULL,   obj   );   

					cur->_next      =    nuLink;
		
					nuLink->_prev =    cur;   // *** NEW, for DOUBLE linking    12/11



				_tail =    nuLink;    // New   1/18


				m_lastAddedLink =   nuLink;   //  New, so I can create Indexes  

				_Count++;
			}
		}
      

								////////////////////////////////
	        
     
      virtual     BOOL   Is_Empty()  const       
	  { 
		  if(   _Count   <=   0L    )     
					 return   true;
		  else		 return   false;
	  }



								////////////////////////////////
	        
     
      virtual     ListsDoubleLink< Item >*   Get_Last_Added_Link()  
	  { 
		  return   m_lastAddedLink;
	  }


	  								////////////////////////////////
	        
     
      virtual     ListsDoubleLink< Item >*   Get_Head_Link()  
	  { 
		  return   _head;
	  }



	  virtual     ListsDoubleLink< Item >*   Get_Last_Link()  
	  { 
													// ******   NEW,   12/11  

		  
			if(    Is_Empty()    )   
			{
				ASSERT( 0 );     // *********   PROBLEM ???   ***************

				return  _head;     
			}
            else
			{   return  _tail;     
			}		  
 	 }




								////////////////////////////////
								////////////////////////////////


		virtual      void   Push(   Item  &obj   )  
		{  
			ASSERT( 0 );    // ******  INSTALL for DOUBLE linked lists   12/11   ****************
			/****
            _head  =    new    ListsDoubleLink<Item>(   _head,   obj   );   
			_Count++;
			****/
		}

								////////////////////////////////


		virtual     Item&   Pop()      // only kills the LINK, not its  'POINTED TO' object 
												//
		{										//   TRICKY, if this list has Dynamic allocated objects, calling function needs to delete the object, or memory leak   10/08
												//   See:    OffMap::Pop_From_LineSegment_Stack()
		
             Item&                  retval  =   _head->_item;
             ListsDoubleLink<Item>  *temp   =   _head;
                  

             _head =   _head->_next;    // assign '_head' the next link's value


			_head->_prev =    NULL;   // *****************   TEST,   12/11

                  
             delete  temp;          // delete the old _head LINK
                  
             _Count--;

			 return   retval;     // *** CAREFUL,  untested    ******
		}




		virtual     Item&   Top()
 		{  
			if(   Is_Empty()    )   
			{
				ASSERT( false );
				return   _head->_item;       // error,     ****  BAD, no error handling    INSTALL ****
			}
            else   
				return    _head->_item;
		}



public:
// ***** CAREFUL:  any new MemberVars must be added to OVERIDE of   'operator='  function   **************


      ListsDoubleLink< Item >   *_head;      //  list's head
      ListsDoubleLink< Item >   *_tail;       //  NEW,    1/18/2012


      long         _Count;

	  bool        m_hasDynamicObjs;  // If its objects we allocated dynamically, then destructor must ALSO deallocate them



	  ListsDoubleLink< Item >   *m_lastAddedLink;     //  New, so I can create Indexes   5/02 

//	  Item      _nullItem;    // ***  to return bad value  **** INSTALL  [  need  InitializeNullItem()  ???  ]



	  ListsDoubleLink< Item >     *m_lastFoundCalcNoteLink;   //  Use this to make SEARCHES faster when playing in forware or backward direction
	                                                                                     //  [ see   Get_Pixel_Info_from_Notelist()   ]    This used to be in  SPitchCalc     1/2012


// ***** CAREFUL:  any new MemberVars must be added to OVERIDE of   'operator='  function   **************
};






///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////



template <class Item>     class   ListLink
{
											//  'Item'  can be a POINTER  or  an OBJECT   ...really????  5/02 ???????


    friend   class   ListMemry<Item>;                // make members accessable to  'ListMemry<Item>'
    friend   class   SpeedIndexIterator<Item>;  

	friend   class   ListDoubLinkMemry<Item>;   


private:
	ListLink(   ListLink<Item>  *next,     Item&  itm    )     
							                       :   _item( itm ),    _next(   (ListLink<Item>*)next   )  
	{ 
		int  dummy =   9;
	}				 //  so far this constructor is only used in   ::Add_Head( Item &obj )    and   Add_Tail( Item &obj )


	virtual  ~ListLink()     //   ALWAYS have a virtual destructor  in case of polymorphism
	{
	}

                               
  
//private:
public:
      ListLink<Item>   *_next;
      Item&                  _item;      //  *** NOTE:  it is a reference!!!   If want a list of POINTERS must 
												//                     dereference it before  pass to list   [   Push(  *ptr  )   ]   
};




				////////////////////////////////////////////


template <class Item>     class   ListsDoubleLink
{
											//  'Item'  can be a POINTER  or  an OBJECT   ...really????  5/02 ???????


// friend   class   ListMemry<Item>;                
    friend   class   ListDoubLinkMemry<Item>;  
	

    friend   class   SpeedIndexIterator<Item>;    // make members accessable to  'ListMemry<Item>'

    friend   class   SpeedIndexDoubleIterator<Item>;     //  use this for fast searches from starting from the END of the list.  12/11


	friend   class   ListDoubLinkMemry<Item>;   



private:
	ListsDoubleLink(   ListsDoubleLink<Item>  *next,      ListsDoubleLink<Item>  *prev,           Item&  itm    )     
							              :   _item( itm ),    _next(  (ListsDoubleLink<Item>*)next  ),    _prev(  (ListsDoubleLink<Item>*)prev  )    
	{ 
		int  dummy =   9;
	}				 //  so far this constructor is only used in   ::Add_Head( Item &obj )    and   Add_Tail( Item &obj )


	virtual  ~ListsDoubleLink()     //   ALWAYS have a virtual destructor  in case of polymorphism
	{
	}

                               
  
//private:
public:
      ListsDoubleLink<Item>   *_next;
      ListsDoubleLink<Item>   *_prev;

      Item&                        _item;      //  *** NOTE:  it is a reference!!!   If want a list of POINTERS must   dereference it before  pass to list   [   Push(  *ptr  )   ]   
};





///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////


#endif // !defined(AFX_LISTS_H__16A7FEE2_4FC0_11D3_9507_ACE109C10000__INCLUDED_)
