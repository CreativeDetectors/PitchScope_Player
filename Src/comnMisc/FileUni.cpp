/////////////////////////////////////////////////////////////////////////////
//
//  FileUni.cpp   -   
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





#include   "FileUni.h"

//////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////


FileUni::FileUni()
{

	m_inVirtualMode =   false;

	m_virtualOffset  =   0;
	m_virtualLength =  -1;
}

						///////////////////////////////////////////


FileUni::~FileUni()
{
}


						///////////////////////////////////////////


void   FileUni::Set_Virtual_Mode_On(   int  offset,     int  virtLength   )
{


	ASSERT(   ! m_inVirtualMode   );   //  do NOT call this function TWICE  ( This is a BIG DEAL if it happens...    FileUni::length() gives bad values, and  
													//									Mp3Reader::getheader( )  will fail.  Happens on the 2nd load of a MP3 file.  1/28/10



	ASSERT(   offset       >= 0   );
	ASSERT(   virtLength >   0   );


	m_virtualOffset   =    offset;

	m_virtualLength  =    virtLength;


	m_inVirtualMode =  true;
}


						///////////////////////////////////////////


int    FileUni::close()
{

	if(            m_file.m_hFile   !=   CFile::hFileNull   
		    &&   m_file.m_hFile   !=    INVALID_HANDLE_VALUE   )      //   added this 1/28/10  to stop it from throwing exceptions from being called 2x
		m_file.Close();
	else
	{				//  It was alread closed.    
		int   dummy =   9;
	}



	m_inVirtualMode =   false;   //  Pretty sure this needs to be re-initialized or when we open a SECOND mp3 file  FileUni::length()  will give the wrong value.  1/28/10 

	m_virtualOffset  =   0;
	m_virtualLength =  -1;



	return  0;    //  no error, throws an exception
}


						///////////////////////////////////////////



int   FileUni::open(  const char *name,   int type  )
{

				//     errstat   sbinfile::open(  const char *name,   int type  )
 

	if(   name ==  NULL   )
	{
		ASSERT( 0 );
		return -1;
	}



	UINT   nOpenFlags =   CFile::modeRead;      //   |   CFile::shareExclusive 								
				          //	   |  CFile::typeBinary   ...BUGGY ????    5/02    Sets binary mode (used in derived classes only).



	//   LPCTSTR lpszFileName,   UINT nOpenFlags 

	if(   !m_file.Open(   name,    nOpenFlags  )    )
	{
		return  0;
	}
	else
		return   1;

	/****
  close();


  int   omode =   O_BINARY;
  int   fmode  =   0;


  switch (  type  &  openiomode   )
  {

	  case openis: fmode=moderead; omode|=O_RDONLY; 
		  break;

	  case openos: fmode=modewrite; omode|=O_WRONLY; 
		  break;

	  case openro: fmode=moderead|modeseek; omode|=O_RDONLY; 
		  break;

	  case openrw: fmode=moderead|modewrite|modeseek|modeappend; omode|=O_RDWR; 
		  break;

	  default: return -1;
  }



  switch ( type &  opencrmode  )
  {
	  case openex: omode|=0; break;

	  case opencr: omode|=O_CREAT; break;

	  case opentr: omode|=O_CREAT|O_TRUNC; break;

	  case opencn: omode|=O_CREAT|O_EXCL; break;

	  default: return -1;
  }



  handle   =   ::open(  name,   omode,  S_IREAD|S_IWRITE   );

  if (handle<0)
    return -1;


  binfilepos len;

  if (  fmode  !=  modewrite  )
  {
    len=   lseek(  handle, 0, SEEK_END  );

    lseek(  handle, 0, SEEK_SET  );
  }
  else
  {
    len=  0;
    lseek(  handle, 0, SEEK_END  );
  }

  openmode(  fmode, 0,  len  );
  trunc=0;


  return 0;   //  sucess
  ****/
}








						///////////////////////////////////////////


int   FileUni::read(   void  *buf,    int len    )
{


	if(   buf  ==  NULL  )
	{
		ASSERT( 0 );
		return  0;
	}



    //    UINT    Read( void* lpBuf, UINT nCount );


	//  nBytesRead  :    The number of bytes transferred to the buffer. Note that for all CFile classes, 
	//                           the return value may be less than nCount if the end of file was reached. 

	UINT    nBytesRead   =    m_file.Read(   buf,    len  );

	return  nBytesRead;
}




						///////////////////////////////////////////



int    FileUni::length()
{

	DWORD   len =   -1;

	/***   This is wrong,  but it worked....  carefdull 

	if(   m_inVirtualMode   )
	{
		len =   m_file.GetLength();
	}
	else
	{  ASSERT(  m_virtualLength  > 0  );   // has this been set

		len =   m_virtualLength;
	}
	*****/

	if(   m_inVirtualMode   )
	{

		ASSERT(  m_virtualLength  > 0  );   // has this been set

		len =   m_virtualLength;
	}
	else
	{   len =   m_file.GetLength();
	}



	return     len;
}


						///////////////////////////////////////////


int    FileUni::Get_File_Position_Virtual()
{

	DWORD  pos  =    m_file.GetPosition();

	pos   +=      m_virtualOffset;

	return  pos;
}


						///////////////////////////////////////////


int    FileUni::Get_File_Position_Actual()
{

	DWORD  pos  =    m_file.GetPosition();
	return     pos;
}



						///////////////////////////////////////////


int     FileUni::eof()
{
    //  *****************************   TEST  ************************************


	if(   m_inVirtualMode   )
	{

		ASSERT(  m_virtualLength  > 0  );   // has this been set


		int   pos      =      m_file.GetPosition();

		int   lenVirt  =     FileUni::length();


		if(    pos  >=  ( lenVirt - 1 )     )
			return  1;    //  8/15   Got here from  ampegdecoder::refillbits()  ...seems to work OK
		else  
			return  0;
	}
	else
	{  DWORD  pos  =   m_file.GetPosition();
		DWORD  len  =    m_file.GetLength(); 

		if(    pos  >=  ( len - 1 )     )
			return  1;
		else  
			return  0;
	}
}




						///////////////////////////////////////////


int    FileUni::seek(  int  pos  )
{

	long      byteOffset =  -1;


	if(   m_inVirtualMode   )              //   virtual offset is usually   4352  ???     7/2012
	{

		ASSERT(  m_virtualLength  > 0  );   // has this been set

		byteOffset  =    m_file.Seek(    pos +  m_virtualOffset,    CFile::begin  );
	}
	else
	{   byteOffset  =    m_file.Seek(    pos,    CFile::begin  );
	}


	return   byteOffset;
}



						///////////////////////////////////////////


int       FileUni::seekcur(  int  pos  )
{

		//  *****  ??????   Not an issue for VirtualMode,  because has a relative move.  ****** ??????


	long      lActual =    m_file.Seek(  pos,   CFile::current );
	return   lActual;
}


						///////////////////////////////////////////


int     FileUni::seekend(  int  pos  )
{

	ASSERT(   pos  <=  0  );    //  positive values will go off the end of the file

	long      lActual =    m_file.Seek(  pos,   CFile::end    );
	return   lActual;
}




						///////////////////////////////////////////


int	    FileUni::peek(   void  *buf,    int len   )
{

	if(   buf  ==  NULL  )
	{
		ASSERT( 0 );
		return  0;
	}


	if(   len <=  0   )
	{
		ASSERT( 0 );
		return  0;
	}




//  if(  mode  &  modeseek  )
// {

 
//  int  len =   readunlogged(  buf,  len );
	int  readLen  =   read(  buf,  len );

 
    seekcur( -readLen  );          //  seek back to file start   ...calls   	seek(  filepos  +p   );
//	long  lActual =    m_file.Seek(  -readLen,   CFile::current );


    return  readLen;
// }







//    binfilepos   binfile::peek(  void *buf,   binfilepos len  )
//  {

	//  4/16/06   May have to modify this to find the header start and fix bug


 

  //if (   !(  mode &  moderead  )     )
//    return 0;


/******
  if(  mode  &  modeseek  )
  {											**** ALWAYS goes here ******

    // insert!!

	  //  ABOVE( insert!! ) is THEIR comment...  what do they mean???  Needs more work???      JPM

    len =  readunlogged(buf,len);

    seekcur( -len  );   //  seek back to file start

    return  len;
  }



  if ( !buffer  )
  {
															****  NEVER goes here
    binfilepos l=rawpeek(buf, len);

    if (readfill!=-1)
      memset((int1*)buf+l, readfill, len-l);

    if (l!=len)
      readerr= 1;

    return l;
  }
  else
  {
															****  NEVER goes here
    if (   len>(buflen-bufpos)   )
    {

      memmove(  buffer,  buffer+bufpos,   buflen-bufpos  );

      buflen  -=  bufpos;
      bufpos  =   0;
      buflen  +=  rawread(  buffer+buflen,  bufmax-buflen) ;


      if (len>buflen)
      {
        readerr=1;

        if (readfill!=-1)
          memset(  (int1*)buf+buflen,  readfill,  len-buflen  );

        len = buflen;
      }
    } 


    memcpy(buf,buffer+bufpos,len);
    return len;
  }

*****/

//	return 0;   // ***********   TAKE out after reqrite ***************************
}
