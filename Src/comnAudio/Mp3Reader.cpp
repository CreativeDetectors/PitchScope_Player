/////////////////////////////////////////////////////////////////////////////
//   Mp3Reader.cpp   -  decoding of mp3 files                      
//
//   PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
//   Copyright (c)  2009-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
//   This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
//   as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
//   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
//   Many ideas regarding decoding of mp3 files in Mp3Reader.cpp were taken from Niklas Beisert's GNU release  
////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"





//#include <stdlib.h>
//#include <string.h>
#include <math.h>




#include   "..\comnMisc\FileUni.h"  







#include "Mp3Decoder.h"

////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
////////////////////////////////////////////////////////////



int   Mp3Reader::ratetab[2][3][16]=
{
  {
    {  0, 32, 64, 96,128,160,192,224,256,288,320,352,384,416,448,  0},
    {  0, 32, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,384,  0},
    {  0, 32, 40, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,  0},
  },
  {
    {  0, 32, 48, 56, 64, 80, 96,112,128,144,160,176,192,224,256,  0},
    {  0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,  0},
    {  0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,  0},
  },
};








Mp3Reader::sballoc Mp3Reader::alloc[17];


Mp3Reader::sballoc *Mp3Reader::atab4[]=
{
  0        , &alloc[0], &alloc[1], &alloc[2], &alloc[4], &alloc[5], &alloc[6], &alloc[7],
  &alloc[8], &alloc[9], &alloc[10],&alloc[11],&alloc[12],&alloc[13],&alloc[14],&alloc[15]
};



Mp3Reader::sballoc *Mp3Reader::atab2[]=
{
  0        , &alloc[0], &alloc[1], &alloc[3], &alloc[2], &alloc[4], &alloc[5], &alloc[16]
};

Mp3Reader::sballoc *Mp3Reader::atab3[]=
{
  0        , &alloc[0], &alloc[1], &alloc[16]
};


Mp3Reader::sballoc *Mp3Reader::atab5[]=
{
  0        , &alloc[0], &alloc[1], &alloc[3], &alloc[2], &alloc[4], &alloc[5], &alloc[6],
  &alloc[7], &alloc[8], &alloc[9], &alloc[10],&alloc[11],&alloc[12],&alloc[13],&alloc[14]
};


Mp3Reader::sballoc *Mp3Reader::atab1[]=
{
  0        , &alloc[0], &alloc[1], &alloc[3], &alloc[2], &alloc[4], &alloc[5], &alloc[6],
  &alloc[7], &alloc[8], &alloc[9], &alloc[10],&alloc[11],&alloc[12],&alloc[13],&alloc[16]
};


Mp3Reader::sballoc *Mp3Reader::atab0[]=
{
  0        , &alloc[0], &alloc[3], &alloc[4], &alloc[5], &alloc[6], &alloc[7], &alloc[8],
  &alloc[9], &alloc[10],&alloc[11],&alloc[12],&alloc[13],&alloc[14],&alloc[15],&alloc[16]
};




Mp3Reader::sballoc **Mp3Reader::alloctabs[3][32]=
{
  {
    atab0, atab0, atab0, atab1, atab1, atab1, atab1, atab1, atab1, atab1,
    atab1, atab2, atab2, atab2, atab2, atab2, atab2, atab2, atab2, atab2,
    atab2, atab2, atab2, atab3, atab3, atab3, atab3, atab3, atab3, atab3
  },
  {
    atab4, atab4, atab4, atab4, atab4, atab4, atab4, atab4, atab4, atab4,
    atab4, atab4
  },
  {
    atab5, atab5, atab5, atab5, atab4, atab4, atab4, atab4, atab4, atab4,
    atab4, atab4, atab4, atab4, atab4, atab4, atab4, atab4, atab4, atab4,
    atab4, atab4, atab4, atab4, atab4, atab4, atab4, atab4, atab4, atab4
  }
};







const int Mp3Reader::alloctabbits[3][32]=
{
  {4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2},
  {4,4,3,3,3,3,3,3,3,3,3,3},
  {4,4,4,4,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2}
};



const int Mp3Reader::alloctablens[5]={27,30,8,12,30};



int   Mp3Reader::freqtab[ 4 ]  =  {   44100,  48000,  32000  };


int   Mp3Reader::lsftab[ 4 ]    =   {  2,  3,  1, 0   };


float Mp3Reader::rangefac[16];

float Mp3Reader::multiple[64];

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


Mp3Reader::Mp3Reader()
{

	mode = 0;   //  from binfile

	m_frameRetardFactor =   0;    // ****  MINE *****
}


					/////////////////////////////////////////////


Mp3Reader::~Mp3Reader()
{
}


					/////////////////////////////////////////////


void	  Mp3Reader::Set_Frame_Timing_Correction(  long  newValue  )
{

			//  ******* BIG and DANGEROUS ******  Helps the seek be more accurate.  Need to test with all formats.    7/2012 


	m_frameRetardFactor =   newValue; 	
}



					/////////////////////////////////////////////


long   Mp3Reader::Seek_fromBeginning_in_OutputCoords(  long  newPos  )
{

				//   HIGH-LEVEL  function.     [   seek_binfile()   will also deal with the buffer,  and reinitialize it   ]     2/10

				//  This funct used to give me trouble  [ NoteLists were inacurate after a PreRoll SEEK ],   but after the SEEK bug was fixed
				//	 with  'm_frameRetardFactor'  this function seems to be fine.    8/1/2012

	bool   dataReadNotThatAccurate =  false;



	long     changedPos  =     seek_binfile(  newPos  );      //    'modNewPos'    would be better
	return  changedPos;
}




					/////////////////////////////////////////////


long   Mp3Reader::Seek_fromBeginning_in_OutputCoords_BlockAlaign(  long  newPos  )
{

				//   DEBUG ONLY:   More accurate than  Seek_fromBeginning_in_OutputCoords(),  but is still a little wrong for RecordingNotes.  7/2012


	bool   dataReadNotThatAccurate =  false;


	long         mp3BlockSize =   4608;   // ************   ALWAYS ???   7/2012   ************************

	ASSERT(   framesize  ==   4608    );


	long   modNewPos  =     (  newPos  /  mp3BlockSize  )   *    mp3BlockSize;



//   /*******************************  TEMP,  put back   **********************************************

	if(    newPos  !=   modNewPos    )
	{
		dataReadNotThatAccurate =   true;   //  Fetching on the BLOCK Boundaries cause an immediate Decode of the Frame, gives
														     //   results that are not too bad  ( when RecordNotes,  notes might be 70 MilliSeconds late.   7/2012

		return  -1;   //  FAIL,  calling function must do homework
	}
//    ***********/


	long     changedPos  =     seek_binfile(  newPos  );      //    'modNewPos'    would be better
	return  changedPos;
}





					/////////////////////////////////////////////


long   Mp3Reader::Seek_WalkfromBeginning_in_OutputCoords(   long  newPos,   long  blockSize   )
{

		//  DEBUG ONLY:  So far,  this is the ONLY way to get a   truely ACCURATE Seek   that will give the same DATA as it we  RecordNotes  with PlayUp  to the
		//	 filePosition for Recording start.    7/2012


	if(   blockSize <= 0  )
		return  -1;


	long    modPos    =    ( newPos  /  blockSize )   *   blockSize;

	if(       modPos  !=   newPos   )  
		return  -2;



	long   numberBlocks  =    newPos  /  blockSize;



	///////////   Method  A  /////////////////////////////

	char*  outputBufferStreaming     =       new   char[  blockSize  ]; 
	if(       outputBufferStreaming == NULL   )
		return  -3;


	long  changedPos =    seek_binfile(  0  );     //  go to the beginning of the file for our WALK


	for(   long  block =0;     block < numberBlocks;    block++    )
	{
		read_binfile(   outputBufferStreaming,   blockSize   );
	}


	if(   outputBufferStreaming  !=  NULL   )
		delete   outputBufferStreaming;

	////////////////////////////////////////




	/*  ////////////   Method  B   is off by quite a bit   [  tried this but as off as anything else  ]

	long  changedPos  =    seek_binfile(  0  );     //  go to the beginning of the file for our WALK

	long  changedPos2 =    seek_binfile(  newPos  );    //   the same as calling:     Mp3Reader::seek_binfile(  filepos +  p  );
	///////////////////////////  *****/



	return   newPos;   //  assume this is right,  I do NOT read or calc anything    7/2012
}





					/////////////////////////////////////////////


int   Mp3Reader::getheader(  FileUni &in,   int &layer,   int &lsf,   int &freq,   int &stereo,   int &rate  )
{   

//   BUG is now  FIXED.  8/16/06  :
//
//	 Can sometimes fail here if the HEADER has too much stuff. See his notes that they 
//    have a bug with finding the Headers start(  0xFF,  hdr[1]<0xE0  ).
//		If I manually trim the .mp3 file in VC++ to  0xFF, then all seems to work.
//
//     FIX:   Rewrite this seek so that it traverses to   (  0xFF,  hdr[1]<0xE0  )
//


  int totrate=0;	



//  int stream =   !(  in.getmode()&modeseek  );
	int stream = 0;					// ************************  CAREFUL   mealdred



  int i,  skJm;


  if(  ! stream   )		
    in.seek( 0 );   //  goes to file start



////////////////    TEMP  DEBUG,  look into the bytes to see what's wrong with   Highway49_SOLO_48hz.mp3 

   unsigned char  blockDbg[ 100 ];

   int nPos =    in.peek(   blockDbg,  100   ); 



	in.seek( 0 );   //  go back to original position.    Because we are in virtual mode, this seek actually puts us back to  1254 for MILaw 
//	in.seek( 2 );
//////////////////





  for(  i=0;   i < 8;   i++   )
  {

    unsigned char   hdr[ 4 ];


    int nr =   in.peek(  hdr,  4   );  //  should return filepos back to file start


    if(  (  nr != 4  )  &&  !i    )
      return 0;



													  // **** BIG,  these 2 bytes are what I must find ******

    if(   hdr[ 0 ]  !=  0xFF  )   
	{
		return 0;    //  **** FAILS here for the file   Highway49_SOLO_48hz.mp3 ( createdwith shareware mp3convert.exe )     2/10
	}


    if (  hdr[ 1 ]  <   0xE0  )
	{
		return 0;
	}




    lsf  =   lsftab[   ( (hdr[1]>>3 )&3)   ];

    if (lsf==3)
      return 0;


    layer=  3-((hdr[1]>>1)&3); 

    if (layer==3)
      return 0;


    if ((lsf==2)&&(layer!=2))
      return 0;



    int pad=   (hdr[2]>>1)&1;

    stereo=  ((hdr[3]>>6)&3)!=3;

    freq=   freqtab[(hdr[2]>>2)&3]>>lsf;


    if (!freq)
      return 0;


    rate =  ratetab[lsf?1:0][layer][(hdr[2]>>4)&15]*1000;

    if (!rate)
      return 0;


    if (stream||(layer!=2))
      return 1;



    totrate +=   rate;



	skJm =   (layer==0)?(((12*rate)/freq+pad)*4):(((lsf&&(layer==2))?72:144)*rate/freq+pad); 
			//   522

    in.seekcur(  (layer==0)?(((12*rate)/freq+pad)*4):(((lsf&&(layer==2))?72:144)*rate/freq+pad)   );
			//  what does this do ????
  }



  rate =  totrate/i;

  in.seek( 0 );    //  go back to file start 

  return 1;
}




					/////////////////////////////////////////////


int   Mp3Reader::decode(  void *outsamp  )
{

  int rate;


  if(  init  )
  {

//    stream =  !(  file->getmode()   &  modeseek   );   //  maybe omit????  Or put in ASSERT() ????
	stream =  0;    // **************  CAREFUL   mealdred  


    int layer,lsf,freq,stereo;

    if (   !getheader(   *m_file,  layer, lsf, freq, stereo, rate  )    )
      return 0;


    if(  stream  )
    {
		ASSERT( 0 );    //   8/12/06
		rate=0;    //   never hit  
	}

    atend=0;
  }


  if (atend)
    return 0;


  if (   !decodehdr( init )   )
    if (init)
      return 0;
    else
      atend=1;



  if (init)
  {
    seekinitframes=0;

    if (         orglay== 0 )
      openlayer1(rate);
    else  if ( orglay== 1 )
      openlayer2(rate);
    else  if ( orglay== 2 )
      openlayer3(rate);
    else
      return 0;


	int  fileLen  =    m_file->length();  //  JPM   ...works


    if (rate)
	{
	//	nframes  =  (long)floor(  (double)(   file->length() +  slotsize )  *   slotdiv/((nslots*slotdiv+fslots)*slotsize)+0.5);
		nframes  =  (long)floor(  (double)(   fileLen +  slotsize )  *   slotdiv/((nslots*slotdiv+fslots)*slotsize)+0.5);
	}
    else
      nframes=0;
  }



  if (        orglay == 0  )			// ****  BIG decode support functions  ******
    decode1();
  else if ( orglay == 1  )
    decode2();
  else
    decode3();



  if(  init  )
  {
    srcchan=   (orgstereo==3)?1:2;

    opensynth();

    framepos=   0;

    framesize=  (36*32*samplesize*dstchan)>>ratereduce;

    curframe=1;
  }


  synth(  outsamp,  fraction[0],  fraction[1]   );       //  ***  Big,  try to understand it.  8/06

  return 1;
}




					/////////////////////////////////////////////


void   Mp3Reader::refillbits()
{

			// **** BIG function:   All the physical File access( reads ) come from here *******************


  if (   mainbufpos  >  (8*mainbuflen)   )
    mainbufpos = mainbuflen * 8;

  int p =   mainbufpos >> 3;

  if (  ( mainbuflen - p )  >  mainbuflow  )
    return;


  memmove(  mainbuf,   mainbuf+p,  mainbuflen - p  );

  mainbufpos  -=   p*8;
  mainbuflen   -=   p;



  while(  1  )
  {				
						//  this is hit repetively for ultimate file access  

    mainbuflen +=   m_file->read(   mainbuf + mainbuflen,    mainbufsize - mainbuflen   );
												//  always takes you to  sbinfile::rawread( )  for the PHYSICAL read of file




	boolm  subEnd    =      m_file->eof();    //  { return ioctl( ioctlreof ); }

//	boolm  actualEnd  =    file->ioctl(   file->ioctlreof   );

//	ASSERT(   subEnd  ==   actualEnd   );    //  Test that  eof()  is good enough



// *******************  CAREFUL with this substitution   mealdred  **************************************

//    if(       actualEnd	
      if(        subEnd		   

		|| (  mainbuflen  >=  mainbufmin )     )
      break;
  }



  memset(   mainbuf + mainbuflen,   0,   mainbufsize - mainbuflen   );
}




					/////////////////////////////////////////////


void Mp3Reader::setbufsize(int size, int low)
{

  mainbufsize  =  (size>mainbufmax)?mainbufmax:size;

  mainbuflow  =   (low>(mainbufsize-16))?(mainbufsize-16):(low<mainbufmin)?mainbufmin:low;
}



					/////////////////////////////////////////////


int   Mp3Reader::openbits()
{

  mainbufsize  =    mainbufmax;




/******  CAREFUL  with substitution 

  unsigned int  modeLoc =    file->getmode();    //   jpm

  ASSERT(  file->modeseek  ==  8  );   //   jpm

  //   mainbufmin  =  2048

  mainbuflow =  (  file->getmode()  &   file->modeseek  )?mainbufmin:mainbufmax;
//  returns   2048    ... for all files ???   When is this thing read ???? 


  ASSERT(  mainbuflow ==  2048   );     //   jpm
*****/
   mainbuflow =   mainbufmin;   //   carefull



  mainbufpos=0;

  mainbuflen=0;

  return 1;
}


					/////////////////////////////////////////////


void Mp3Reader::getbytes(void *buf2, int n)
{

  memcpy(  buf2,   bitbuf + (*bitbufpos>>3),   n   );

  *bitbufpos+=n*8;
}


					/////////////////////////////////////////////


int    Mp3Reader::sync7FF()
{

  mainbufpos   =  (mainbufpos+7)&~7;


  while (1)
  {
    refillbits();		// ****  file reads happen from here

    if (mainbuflen<4)
      return 0;


    while ((((mainbufpos>>3)+1)<mainbuflen)&&((mainbuf[mainbufpos>>3]!=0xFF)||(mainbuf[(mainbufpos>>3)+1]<0xE0)))
    {
      mainbufpos+=8;
    }


    while( 1   /*(((mainbufpos>>3)+1)<mainbuflen)&&(mainbuf[mainbufpos>>3]==0xFF)&&(mainbuf[(mainbufpos>>3)+1]>=0xE0)*/)
    {

      if (((mainbufpos>>3)+1)>=mainbuflen)
        break;

      if (mainbuf[mainbufpos>>3]!=0xFF)
        break;

      if (mainbuf[(mainbufpos>>3)+1]<0xE0)
        break;

      mainbufpos+=8;
    }


    if ((mainbufpos>>3)<mainbuflen)
    {
      mainbufpos+=3;

      refillbits();     //  *****  file reads happen from here

      return 1;
    }

  }
}


					/////////////////////////////////////////////


int  Mp3Reader::decodehdr(  int init  )
{

  while (1)
  {
    if(  !sync7FF()   )
    {
      hdrbitrate=0;
      return 0;
    }


    bitbuf  =    mainbuf;

    bitbufpos =    &mainbufpos;

    hdrlsf =  lsftab[  mpgetbits(2)  ];


    hdrlay =  3  -  mpgetbits( 2 );   //  ****** BIG, determines which decoder to use 


    hdrcrc =  ! mpgetbit();


    hdrbitrate   =  mpgetbits( 4 );

    hdrfreq      =   mpgetbits( 2 );

    hdrpadding =  mpgetbit();


    mpgetbit(); // extension

    hdrmode = mpgetbits( 2 );

    hdrmodeext = mpgetbits( 2 );



    mpgetbit();    // copyright

    mpgetbit();     // original

    mpgetbits(2);  // emphasis


    if(  init  )
    {

      orglay =  hdrlay;   //  ****  BIG,  decides which of the 3 decoders to use ****


      orglsf =  hdrlsf;

      orgfreq    =   hdrfreq;

      orgstereo =   (  hdrmode==1  )?0:hdrmode;
    }


    if ((hdrlsf!=orglsf)||(hdrlay!=orglay)||(hdrbitrate==0)||(hdrbitrate==15)||(hdrfreq!=orgfreq)||(((hdrmode==1)?0:hdrmode)!=orgstereo))
    {
      *bitbufpos  -=  20;
      continue;
    }


    if ( hdrcrc )
      mpgetbits( 16 );


    return 1;
  }
}



					/////////////////////////////////////////////


int  Mp3Reader::rawseek(  int  pos  )
{

		//    pos :    Is in the Index to   UNCOMPRESSED  bytes  of a Virtual File  ( of the output bytes )     2/10

		//    BIG function to understand how frames and file position are calculated 


  if(  stream  )
    return 0;


  if(   pos <0  )
    pos = 0;


  if(   pos  >=  nframes  *  framesize    )     //  if past the END of file
    pos  =   nframes  *   framesize;



  int  fr      =    pos  /   framesize;      //  the NEW current  FRAME INDEX

  int  frpos =    pos  % framesize;     //   and the OFFSET into that  FRAME's data block   




  if(  ( curframe -1 )   ==   fr    )     //  if we are already in the CURRENT frame
  {

    framepos =    pos  %  framesize;

    return (  curframe - 1  )   *    framesize +  framepos ;
  }





			//  Hard to say what they are doing with  'discard'.  'fr'  is a Frame-Index to the raw MFC file, where we start the DataRead.
			//  Looks like they want to push  'fr'  further back in time to make a more accurate Seek.  
			//  My new sucessful  'm_frameRetardFactor'  seems to just push  'fr'  further back than the original developer intended ??    8/2012


	int discard = 0;

	curframe  =     fr;    //  assign a   LASTING  Frame-Position   to  this almost global MemberVar 


	int  extra =  (  seekmode  ==  seekmodeexact  )?1:0;    //   If in  'seekmodeexact'  MODE, then wants to push  'fr'  further back in time.  [   is extra always  1  ????

	fr  -=   seekinitframes  +  extra;


	if(   fr  <  0   )
		discard  =  -fr;


	fr  +=   discard;







	int  frOriginal =    fr;    //   save for DEBUG


//  ************  OK???   My big change to the algo [ 8/2012 ]    Will it cause a PROBLEM ????  *****************************


	fr =    fr  -  m_frameRetardFactor;    //   - 4 [  syncs good !!  ]       - 8 [ 216,  not 114 ]    -2[ 70,  not  114 ]  ******* my experiment   7/31/2012  *******


					//  Is my  'm_frameRetardFactor'   just another way to make the  'seekinitframes'   BIGGER ? 


	if(   fr < 0   )
	{                        //	  land here when hit   GoToFileStart  Button  
		fr =   0;   
	}
// **********************************************************************************************************




					// 	  'fr'   looks like a reliable and modifiable   INDEX to any FRAME   in the raw data file  [ MFC  FileUni  ]    7/2012


	int   nuFilePosition  =      (   fr * nslots   +   ( fr * fslots )/ slotdiv  )     * slotsize;


	m_file->seek(   nuFilePosition  );   //  calls  FileUni::seek( pos )  which uses MFC to Seek to a PHYSICAL file position.



			// ??????   What do   'nslots'[ 1044 ],    fslots'[ 39600 ],    'slotdiv'[ 44100 ] ,   and   'slotsize'[ 1 ]    DO ???    7/2012





  mainbufpos  = 0;	

  mainbuflen  = 0;

  atend  =0;



  if(  orglay ==  2  )
  {
	  seekinit3(   discard   );    //   *** BIG,  think it Initializes the   Decoder's Pipeline
  }




  if(  extra   )
  {

	if(    discard   !=   ( seekinitframes + extra )    )
		Mp3Reader::decode( 0 );
    else
		resetsynth();
  }




  if(  frpos  )      //   ( frpos > 0 ):     If we are INSIDE the Frame  ( NOT at its start Boundary  )
  {							

    if(   decode(  framebuf  )    )
    {
      curframe++;
      framepos =   frpos;
    }
    else
      framepos =   framesize;
  }
  else
    framepos =   framesize;    //   assign  this INDEX  to a BYTE within the Frame,   to point to Frame's  Upper-Limit  ( the SIZE of the frame ) 




  int        newPos =      ( curframe -1 ) *  framesize     +   framepos;
  return   newPos;
}




					/////////////////////////////////////////////


int Mp3Reader::rawpeek(  void *buf,   int len  )
{


  if ( framepos == framesize )
    if (decode(framebuf))
    {
      framepos=0;
      curframe++;
    }


  int l  =   framesize-framepos;


  if (  l  >  len  )
    l = len;


  memcpy(   buf,     framebuf + framepos,   l   );

  return l;
}




					/////////////////////////////////////////////


int    Mp3Reader::rawread(   void *buf,     int len   )
{

									 //  *** BIG,    Mp3Reader::decode()

  long rd =0;


  while(  rd  <   len  )
  {

    if(    ( framepos == framesize )   &&   (   ( len - rd )  >=  framesize )      )
    {

      if(   ! decode(  (short*)(  (char*)buf+rd)  )    )			// **** the BIG Kahuna ******
        break;

      curframe++;
      rd  +=  framesize;

      continue;
    }




    if(   framepos  ==  framesize   )
	{  
		
	  if(   decode( framebuf )   )				// **** the BIG Kahuna ******
      {
          framepos =   0;
          curframe++;
      }
      else
          break;
	}



    int l  =   framesize  -  framepos;


    if(  l  >  (  len - rd)  )
      l =  len  -  rd;


    memcpy(    (char*)buf + rd,     framebuf  +  framepos,    l   );    //   'framebuf'    is where the   DataBytes are   


    framepos +=  l;
    rd +=  l;
  }


  return  rd;
}



					/////////////////////////////////////////////


int   Mp3Reader::open(    FileUni &in,    int &freq,    int &stereo,   int fmt,   int downSampleRatio,   int chn   )
{

// *****  CAREFUL this has a new  RETURN values *******************



	//  downSampleRatio = 1   means  1/2 the file size
	//                             =2    means  1/4 the file size   ...etc.  It is input to change the downsampling.



			//  does not really do any file activity(  'in' is already open ),  just init the  Mp3Reader  structure
 

  close_binfile();   //  Does Nothing...  could omit *********************


  init12();

  init3();


  m_file =   &in;   


  openbits();

  dstchan =  chn;

  ratereduce =  (downSampleRatio<0)?0:(downSampleRatio>2)?2:downSampleRatio;   // **** Downsampling ?????  

  samplesize =   fmt?2:4;



  init = 1;

  if (   ! decode(  framebuf  )     )   // *********  
    return 0;



  init= 0;

  freq    =    freqtab[ orgfreq ]  >>  ( orglsf + ratereduce );

  stereo =   ( dstchan == 2 )?1:0;



  seekmode = 0;


  openmode_binfile(    moderead | (stream?0:modeseek),   0,    nframes * framesize  );
					//  All this does is initialize the vars in  Mp3Reader,  no file activity

  return  1;
}



					/////////////////////////////////////////////


errstat Mp3Reader::rawclose()
{
  closemode_binfile();

  return 0;
}


					/////////////////////////////////////////////


int Mp3Reader::rawioctl(  int code,   void *buf,   int len   )
{

  int old;

  switch (code)
  {

	  case ioctlseekmode: old=seekmode; seekmode=len?1:0; return old;
	  case ioctlseekmodeget: return seekmode;
	  case ioctlsetvol: setvol(buf?*(float*)buf:1); return 0;
	  case ioctlsetstereo: setstereo((float*)buf); return 0;

	  case ioctlsetequal32:
		if (orglay==2)
		  setl3equal(0);

		setequal((float*)buf);
		return 0;


	  case ioctlsetequal576:
		if (orglay==2)
		{
		  setl3equal((float*)buf);
		  return 0;
		}
		float eq32[32];
		int i,j;
		for (i=0; i<32; i++)
		{
		  eq32[i]=0;
		  for (j=0; j<18; j++)
			eq32[i]+=((float*)buf)[i*18+j];
		  eq32[i]/=18;
		}
		setequal(eq32);
		return 0;


	  case ioctlrreof:
		return (framepos==framesize)&&atend;


//	  default: return binfile::rawioctl(code,buf,len);
	   default: return rawioctl_binfile(code,buf,len);   
  }
}




/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////   binfile copies   ...jpm   //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////


int  Mp3Reader::rawioctl_binfile(  int code, void *buf, int len  )
{

  int ret=0;


  switch ( code )
  {
	  case ioctlreof:
		if (!(mode&moderead))
		     return 1;
		if (ioctl(ioctlrbufget, 0, 0))
		     return 0;
		return ioctl(ioctlrreof, 0, 0);


	   case ioctlrbufget:    
		   return buflen-bufpos;

	   case ioctlrreof:      
		   return filelen==filepos;

	   default: 
		  ASSERT( 0 );   //  incrementall add needed cases 
		break;
  }

  return ret;
}




int   Mp3Reader::read_binfile(  void *buf,   int len  )
{


  if(   ! (  mode & moderead  )||(  len  <=  0 )  )     //  This get called after the SEEK from a change in the File Slider position. [ from  Fetch_PieSlice_Samples()
    return 0;




  if(   buf   )
  {
    int  l  =   readunlogged_binfile(   buf,   len   );   //  goes here   JPM

    return  l;
  }

  else
  {
    int             skipped=0;    //  Do I ever go here ?????    7/2012

    const  int   skipbuflen =  256;


    int1    skipbuf[  skipbuflen  ];


    while (  skipped  !=   len  )
    {

      int l =    len - skipped;

      if(   l  >  skipbuflen  )
        l =  skipbuflen;


      l =    readunlogged_binfile(   skipbuf,   l   );



      skipped  +=  l;

      if(  l  !=  skipbuflen)
        break;
    }

    return  skipped;
  }
}




errstat   Mp3Reader::close_binfile()
{

  if (!mode)
    return 0;

  int r =   rawclose();

  if(  r  >=  0  )
    reset_binfile();

  return r;
}



void   Mp3Reader::openmode_binfile(  uintm m,  int pos, int len   )
{

		//  Comment all out,  and slowly add NEW member vars as needed    JPM


  filepos=pos;
  filelen=len;
  mode=m|modeopen;
  filewpos=0;
  buffer=0;
  wbuffer=0;
  bufmax=0;
  buflen=0;
  bufpos=0;
  bufstart=0;
  bufdirty=0;
  wbufmax=0;
  wbufpos=0;
  bitmode=0;
  bitpos=0;
  bitbuf=0;
  wbitmode=0;
  wbitpos=0;
  wbitbuf=0;
  wbitfill=0;
  readfill=-1;
  readerr=0;
  writeerr=0;
// reset pars
}



void  Mp3Reader::closemode_binfile()
{

  wsyncbyte_binfile();

  invalidatebuffer_binfile( 1 );

  buflen=0;

  setbuffer_binfile(0);


  if(  ! (  mode  &  modeseek  )   )
  {
    invalidatewbuffer_binfile( 1 );

    setwbuffer_binfile( 0) ;
  }
  else
    rawseek( filepos );
}




int   Mp3Reader::readunlogged_binfile(  void *buf,  int len  )
{

  // reading past end of file????


  int   l1,  l2;						 //  *** BIG,   ultimately calls   Mp3Reader::decode()

  if(   ! buffer   )
  {

    l2  =   rawread(  buf,   len  );   //  goes here...    JPM


    filepos  +=   l2;


    if(  l2  !=  len  )
    {
      readerr =  1;

      if(  (  readfill  !=  -1 )   &&   buf    )
	  {
		  memset(    ( int1* )buf + l2,     readfill,     len  -  l2    );
	  }
    }


    return l2;
  }




  l1=len;


  if (l1>(buflen-bufpos))
  {

    if (buflen<bufmax)
    {
      if (bufdirty)
        rawseek(bufstart+buflen);

      buflen += rawread( buffer+buflen, bufmax-buflen );  
    }
    if (l1>(buflen-bufpos))
      l1=buflen-bufpos;
  }



  if (l1)
  {
    if (buf)
      memcpy(buf, buffer+bufpos, l1);

    bufpos+=l1;
    filepos+=l1;
  }


  if (l1==len)
    return l1;


  invalidatebuffer_binfile( 0 );


  if (buf)
    *(int1**)&buf+=l1;
  len-=l1;

  if (bufpos)
    l2=0;
  else if (len>=bufmax)
  {
    l2=rawread(buf, len);
    bufstart+=l2;
  }
  else
  {
    l2=len;

    buflen += rawread(  buffer+bufpos,    bufmax-buflen);


    if (l2>(buflen-bufpos))
      l2=buflen-bufpos;
    if (buf)
      memcpy(buf, buffer+bufpos, l2);
    bufpos+=l2;
  }


  if (l2!=len)
  {
    readerr=1;
    if ((readfill!=-1)&&buf)
      memset((int1*)buf+l2, readfill, len-l2);
  }


  filepos+=l2;
  return l1+l2;
}



void  Mp3Reader::reset_binfile()
{
  mode=0;

  filepos=0;

  filelen=0;

//  pipefile=0;
}



boolm   Mp3Reader::wsyncbyte_binfile()
{


  if (    mode & modeseek   )
  {
    if (  ! bitpos   )
      return 1;

    seekcur_binfile(1);
    bitpos=0;

    return 1;
  }
  else
  {
    if (  ! wbitpos   )
      return 1;


    if (   ! wbitmode  )
      return putbits_binfile(wbitfill, 8-wbitpos);
    else
      return putbits_binfile(wbitfill, 8-wbitpos);
  }
}




boolm  Mp3Reader::invalidatebuffer_binfile( int force )
{

  boolm ret=1;

  if (bufdirty)
  {

    rawseek(bufstart);


    int l=  rawwrite_binfile(buffer,buflen);   //  should not be called, will not write ????



    if (l!=buflen)
    {
      if (!force)
      {
        memmove(buffer, buffer+l, buflen-l);
        buflen-=l;
        bufpos-=l;
        bufstart+=l;
        return 0;
      }
      writeerr=1;
      ret=0;
      if (filelen==(bufstart+buflen))
      {
        filelen=bufstart+l;
        if (filepos>filelen)
          filepos=filelen;
      }
    }
    bufdirty=0;
  }
  if (!(mode&modeseek))
    if (force==2)
      ret=!buflen;
    else
      return !buflen;
  bufstart=filepos;
  bufpos=0;
  buflen=0;
  return ret;
}



boolm   Mp3Reader::invalidatewbuffer_binfile(int force)
{

  if (mode&modeseek)
    return invalidatebuffer_binfile(force);

  if (!wbufpos)
    return 1;

  boolm ret;

  if (force==2)
  {
    ret=!wbufpos;
    wbufpos=0;
  }
  else
  {
    int l=rawwrite_binfile(wbuffer,wbufpos);

    ret=l==wbufpos;
    wbufpos-=force?wbufpos:l;

    memmove(wbuffer, wbuffer+l, wbufpos);
  }
  return ret;
}



boolm  Mp3Reader::setbuffer_binfile(int len)
{

  invalidatebuffer_binfile(0);

  if (buflen)
    return 0;

  if (bufmax>minibuflen)
    delete buffer;

  buffer=  len?(len<=minibuflen)?minibuf:new uint1 [len]:0;

  bufmax=buffer?len:0;

  return buffer||!len;
}


boolm  Mp3Reader::setwbuffer_binfile( int len )
{

  invalidatewbuffer_binfile(0);

  if (wbufpos)
    return 0;
  if (wbufmax>minibuflen)
    delete wbuffer;

  wbuffer=len?(len<=minibuflen)?wminibuf:new uint1 [len]:0;

  wbufmax=wbuffer?len:0;

  return wbuffer||!len;
}



int  Mp3Reader::seekcur_binfile(  int  p  )
{

  return   seek_binfile(  filepos +  p  );
}



boolm  Mp3Reader::putbits_binfile(uintm b, int n)
{

//  if (pipefile)
 //   return pipefile->putbits(b,n);

  if (!(mode&modewrite))
    return 0;

  boolm r;
  b&=(1<<n)-1;

  if (mode&modeseek)
  {
    uintl4 p;
    int rf=readfill;
    readfill=wbitfill&0xFF;

    peek_binfile(&p, (bitpos+n+7)>>3);

    readfill=rf;

    if (!bitmode)
      p=(p&~(((1<<n)-1)<<bitpos))|(b<<bitpos);
    else
      (*(uintb4*)&p)=((*(uintb4*)&p)&~(((1<<n)-1)<<(32-bitpos-n)))|(b<<(32-bitpos-n));


    r=  ewrite(&p, (bitpos+n+7)>>3);

    bitpos=(bitpos+n)&7;

    if (bitpos&&r)
      seekcur_binfile(-1);
  }
  else
  {
    uintl4 p;
    if (!wbitmode)
      p=b<<wbitpos;
    else
    {
      uintb4 t=b<<(32-wbitpos-n);
      p=*(uintl4*)&t;
    }


    p=p|wbitbuf;
    wbitpos+=n;
    r=ewrite(&p, wbitpos>>3);
    wbitbuf=p>>(wbitpos&~7);
    wbitpos&=7;
  }
  return r;
}




int  Mp3Reader::rawwrite_binfile(const void *, int)
{
  return 0;   // ***** OK ????? *******
}




int Mp3Reader::seek_binfile(  int p  )
{

			//  I use this to seek to a file position in Output-Coords   jpm  2/2010



 // if (pipefile)
 //   return pipefile->seek(p);


  if(   ! (  mode & modeseek  )   ||   (  filepos == p  )     )
    return  filepos;


  if(  p <0  )
    p = 0;


  if(  ! buffer  )
  {
    filepos =   rawseek(  p  );       //  Goes here...      JPM


 //   if (logfile)
 //     logfile->seek( filepos );


    return   filepos;
  }



  if(  (  p >= bufstart  )  &&  (  p <  ( bufstart + buflen )  )   )
  {

    bufpos  =  p  -  bufstart;

    filepos  =  p;


//    if (logfile)
 //     logfile->seek(filepos);

    return   filepos;
  }



  invalidatebuffer_binfile( 1 );    //  big


  if(  p  >  filelen  )
    p  =  filelen;


  filepos    =   rawseek(  p  );

  bufstart  =    filepos;


 // if (logfile)
 //   logfile->seek(filepos);


  return   filepos;
}




int   Mp3Reader::peek_binfile(  void *buf,   int len  )
{

	//  4/16/06   May have to modify this to find the header start and fix bug


//  if (pipefile)
 //   return pipefile->peek( buf, len );


  if(   ! (  mode  &  moderead )   ||   (len<=0)   )
    return 0;



  if(  mode&modeseek  )
  {

    // insert!!


	  //  ABOVE( insert!! ) is THEIR comment...  what do they mean???  Needs more work???      JPM


    len =     readunlogged_binfile(  buf,   len  );


    seekcur_binfile(  -len  );

    return  len;
  }



  if ( ! buffer  )
  {

    int l=rawpeek(buf, len);


    if (   readfill!=-1  )
      memset((int1*)buf+l, readfill, len-l);


    if (l!=len)
      readerr= 1;


    return l;
  }
  else
  {

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
}



boolm   Mp3Reader::ewrite( const void *buf,  int len )   
{ 

	return  write_binfile(buf,len) == len; 
}




int  Mp3Reader::write_binfile(  const void *buf,   int len  )
{

													//  the BIG catalysing function  jpm
//  if (pipefile)						
 //   return pipefile->write(buf,len);


  if (!(mode&modewrite)||(len<=0))
    return 0;

  int l1,l2;


  if ((!(mode&modeseek)&&!wbuffer)||((mode&modeseek)&&!buffer))
  {

    l2=rawwrite_binfile(buf,len);
    if (l2!=len)
      writeerr=1;


    if (mode&modeseek)
    {
      filepos+=l2;
      if (filelen<filepos)
        filelen=filepos;
    }
    else
      filewpos+=l2;


    return l2;
  }


  if (!(mode&modeseek))
  {
    l1=len;
    if (l1>=(wbufmax-wbufpos))
      l1=wbufmax-wbufpos;

    memcpy(wbuffer+wbufpos, buf, l1);

    wbufpos+=l1;
    filewpos+=l1;

    if (l1==len)
      return l1;

    invalidatewbuffer_binfile(0);

    len-=l1;
    *(int1**)&buf+=l1;


    if (!wbufpos&&(len>wbufmax))
      l2=rawwrite_binfile(buf,len);
    else
    {
      l2=len;

      if (l2>(wbufmax-wbufpos))
        l2=wbufmax-wbufpos;

      memcpy(wbuffer+wbufpos, buf, l2);

      wbufpos+=l2;
    }


    if (l2!=len)
      writeerr=1;

    filewpos+=l2;

    return l1+l2;
  }


  l1=len;

  if (l1>=bufmax)
    l1=0;


  if (l1>(bufmax-bufpos))
    if (!bufdirty)
      invalidatebuffer_binfile(0);
    else
      l1=bufmax-bufpos;


  if (l1)
  {
    memcpy(buffer+bufpos, buf, l1);

    bufpos+=l1;
    if (buflen<bufpos)
      buflen=bufpos;

    bufdirty=1;
    filepos+=l1;

    if (filelen<filepos)
      filelen=filepos;
  }


  if (l1==len)
    return l1;

  invalidatebuffer_binfile(0);
  *(int1**)&buf+=l1;
  len-=l1;


  if (!bufpos&&(len>=bufmax))
  {
    l2=rawwrite_binfile(buf,len);
    bufstart+=l2;
  }
  else
  {
    l2=len;
    memcpy(buffer+bufpos, buf, l2);
    bufdirty=1;
    bufpos+=l2;

    if (bufpos>buflen)
      buflen=bufpos;
  }


  if (l2!=len)
    writeerr=1;

  filepos+=l2;

  if (filelen<filepos)
    filelen=filepos;

  return l1+l2;
}

/////////////////////////////////  end of  binfile substitues   ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// was  ampdec1.cpp     /////////////////////////////////////


void  Mp3Reader::openlayer1(  int rate  )
{

  if (rate)
  {
    slotsize  =  4;

    slotdiv  =  freqtab[ orgfreq ] >> orglsf;

    nslots   =   (36*rate) / (  freqtab[orgfreq] >> orglsf  );

    fslots   =   (36*rate)%slotdiv;
  }
}




void   Mp3Reader::decode1()
{

  int i,j,q,fr;


  for (fr=0; fr<3; fr++)
  {
    if (fr)
      decodehdr(0);
    if (!hdrbitrate)
    {
      for (q=0; q<12; q++)
        for (j=0; j<2; j++)
          for (i=0; i<32; i++)
            fraction[j][12*fr+q][i]=0;
      continue;
    }


    int bitend=mainbufpos-32-(hdrcrc?16:0)+(hdrpadding?32:0)+12*1000*ratetab[hdrlsf?1:0][0][hdrbitrate]/(freqtab[hdrfreq]>>hdrlsf)*32;


    int jsbound=(hdrmode==1)?(hdrmodeext+1)*4:(hdrmode==3)?0:32;

    int stereo=(hdrmode==3)?1:2;


    for (i=0;i<32;i++)
      for (j=0;j<((i<jsbound)?2:1);j++)
      {
        bitalloc1[j][i] = mpgetbits(4);
        if (i>=jsbound)
          bitalloc1[1][i] = bitalloc1[0][i];
      }

    for (i=0;i<32;i++)
      for (j=0;j<stereo;j++)
        if (bitalloc1[j][i])
          scale1[j][i]=multiple[mpgetbits(6)]*rangefac[bitalloc1[j][i]];



    for (q=0;q<12;q++)
      for (i=0;i<32;i++)
        for (j=0;j<((i<jsbound)?2:1);j++)
          if (bitalloc1[j][i])
          {
            int s=mpgetbits(bitalloc1[j][i]+1)-(1<<bitalloc1[j][i])+1;
            fraction[j][12*fr+q][i]=scale1[j][i]*s;
            if (i>=jsbound)
              fraction[1][12*fr+q][i]=scale1[1][i]*s;
          }
          else
          {
            fraction[j][12*fr+q][i]=0;
            if (i>=jsbound)
              fraction[1][12*fr+q][i]=0;
          }


    mpgetbits(bitend-mainbufpos);
  }
}

///////////////////////////////  end of   ampdec1.cpp     /////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// was  ampdec2.cpp  ////////////////////////////////////////


void  Mp3Reader::init12()
{

  int i;




  /***
  for (i=0; i<63; i++)
    multiple[i] =  exp(  log(2)*(3-i)/3.0)  ;      //  2005JPM   
  ****/
  for (i=0; i<63; i++)
    multiple[i] =    exp(     log(2.0)   *   (3-i)/3.0     )  ;      //  Better ?????




  multiple[63]=0;


  for (i=0; i<16; i++)
    rangefac[i]=2.0/((2<<i)-1);


  for (i=0; i<3; i++)
  {
    alloc[i].scaleadd=1<<i;
    alloc[i].steps=2*alloc[i].scaleadd+1;
    alloc[i].bits=(10+5*i)>>1;
    alloc[i].scale=2.0/(2*alloc[i].scaleadd+1);
  }


  for (i=3; i<17; i++)
  {
    alloc[i].steps=0;
    alloc[i].bits=i;
    alloc[i].scaleadd=(1<<(i-1))-1;
    alloc[i].scale=2.0/(2*alloc[i].scaleadd+1);
  }
}



void Mp3Reader::openlayer2(int rate)
{
  if (rate)
  {
    slotsize=1;
    slotdiv=freqtab[orgfreq]>>orglsf;
    nslots=(144*rate)/(freqtab[orgfreq]>>orglsf);
    fslots=(144*rate)%slotdiv;
  }
}



void Mp3Reader::decode2()
{
  int i,j,k,q;

  if (!hdrbitrate)
  {
    for (q=0; q<36; q++)
      for (j=0; j<2; j++)
        for (i=0; i<32; i++)
          fraction[j][q][i]=0;
    return;
  }



  int bitend=mainbufpos-32-(hdrcrc?16:0)+144*1000*ratetab[hdrlsf?1:0][1][hdrbitrate]/(freqtab[hdrfreq]>>hdrlsf)*8+(hdrpadding?8:0);


  int stereo=(hdrmode==3)?1:2;
  int brpch=ratetab[0][1][hdrbitrate]/stereo;


  int tabnum;


  if (hdrlsf)
    tabnum=4;
  else
  if (((hdrfreq==1)&&(brpch>=56))||((brpch>=56)&&(brpch<=80)))
    tabnum=0;
  else
  if ((hdrfreq!=1)&&(brpch>=96))
    tabnum=1;
  else
  if ((hdrfreq!=2)&&(brpch<=48))
    tabnum=2;
  else
    tabnum=3;
  sballoc ***alloc=alloctabs[tabnum>>1];
  const int *allocbits=alloctabbits[tabnum>>1];
  int sblimit=alloctablens[tabnum];
  int jsbound;
  if (hdrmode==1)
    jsbound=(hdrmodeext+1)*4;
  else
  if (hdrmode==3)
    jsbound=0;
  else
    jsbound=sblimit;



  for (i=0; i<sblimit; i++)
    for (j=0; j<((i<jsbound)?2:1); j++)
    {
      bitalloc2[j][i]=alloc[i][mpgetbits(allocbits[i])];
      if (i>=jsbound)
        bitalloc2[1][i]=bitalloc2[0][i];
    }



  for (i=0; i<sblimit; i++)
    for (j=0; j<stereo; j++)
      if (bitalloc2[j][i])
        scfsi[j][i]=mpgetbits(2);



  for (i=0;i<sblimit;i++)
    for (j=0;j<stereo;j++)
      if (bitalloc2[j][i])
      {
        int si[3];
        switch (scfsi[j][i])
        {
        case 0:
          si[0]=mpgetbits(6);
          si[1]=mpgetbits(6);
          si[2]=mpgetbits(6);
          break;
        case 1:
          si[0]=si[1]=mpgetbits(6);
          si[2]=mpgetbits(6);
          break;
        case 2:
          si[0]=si[1]=si[2]=mpgetbits(6);
          break;
        case 3:
          si[0]=mpgetbits(6);
          si[1]=si[2]=mpgetbits(6);
          break;
        }
        for (k=0; k<3; k++)
          scale2[j][k][i]=multiple[si[k]]*bitalloc2[j][i]->scale;
      }



  for (q=0;q<12;q++)
  {
    for (i=0; i<sblimit; i++)
      for (j=0; j<((i<jsbound)?2:1); j++)
        if (bitalloc2[j][i])
        {
          int s[3];
          if (!bitalloc2[j][i]->steps)
            for (k=0; k<3; k++)
              s[k]=mpgetbits(bitalloc2[j][i]->bits)-bitalloc2[j][i]->scaleadd;
          else
          {
            int nlevels=bitalloc2[j][i]->steps;
            int c=mpgetbits(bitalloc2[j][i]->bits);
            for (k=0; k<3; k++)
            {
              s[k]=(c%nlevels)-bitalloc2[j][i]->scaleadd;
              c/=nlevels;
            }
          }

          for (k=0; k<3; k++)
            fraction[j][q*3+k][i]=s[k]*scale2[j][q>>2][i];
          if (i>=jsbound)
            for (k=0;k<3;k++)
              fraction[1][q*3+k][i]=s[k]*scale2[1][q>>2][i];
        }
        else
        {
          for (k=0;k<3;k++)
            fraction[j][q*3+k][i]=0;
          if (i>=jsbound)
            for (k=0;k<3;k++)
              fraction[1][q*3+k][i]=0;
        }

    for (i=sblimit; i<32; i++)
      for (k=0; k<3; k++)
        for (j=0; j<stereo; j++)
          fraction[j][q*3+k][i]=0;
  }


//  if ((bitend-mainbufpos)>=16)
//    decode2mc();


  mpgetbits(bitend-mainbufpos);
}


//////////////////////////////// end of   ampdec2.cpp  ////////////////////////////////////////









