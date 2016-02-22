/////////////////////////////////////////////////////////////////////////////
//   Mp3Reader.h   -  decoding of mp3 files                      
//
//   PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
//   Copyright (c)  2009-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
//   This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
//   as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
//   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
//   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
//   Many ideas regarding decoding of mp3 files in Mp3Reader.cpp were taken from Niklas Beisert's GNU release  
////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_MP3READER_H__D1E6D6F4_7AAA_4CC6_A012_A8D934B4EE5B__INCLUDED_)
#define AFX_MP3READER_H__D1E6D6F4_7AAA_4CC6_A012_A8D934B4EE5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
////////////////////////////////////////////////////////



#pragma warning (disable: 4244)    // conversion from 'double' to 'float', possible loss of data
#pragma warning (disable: 4305)    // truncation from 'const double' to 'float'





typedef  unsigned int     uintm;    // ***** REPLACE  **********************
typedef  unsigned char   uint1;    // ***** REPLACE  **********************

//typedef  signed char       int1;     // ***** REPLACE  ********  'char' in MS means 'signed char
typedef   char       int1; 

typedef  unsigned int    uint4;		// ***** REPLACE  **********************

// typedef uint4  uintl4;
typedef  unsigned int    uintl4;   // same as above			// ***** REPLACE  **********************




typedef  int   boolm;      //  zero: false,    nonzero: true
typedef  int   errstat;      // negative:error,    nonnegative:retval,   zero:ok,   positive:warning



inline    uint4  swaplb4(  uint4 v  )  { return ((v&0xFF)<<24)|((v&0xFF00)<<8)|((v>>8)&0xFF00)|((v>>24)&0xFF); }



///////////////////////////////////////////////////////////////////////////////////////

class  uintb4
{
private:
  uint4 i;

public:
  uintb4() {}
  uintb4(uint4 v)    { i=swaplb4(v); }     //  goes here

  operator uint4()  { return swaplb4(i); }
};

///////////////////////////////////////////////////////////////////////////////////////







class  FileUni;


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


class  Mp3Reader  
{

public:
	Mp3Reader();
	virtual ~Mp3Reader();



	static    int           getheader(   FileUni &in,    int &layer,   int &lsf,   int &freq,   int &stereo,   int &rate  );

                int           open(          FileUni &in,    int &freq,   int &stereo,   int fmt,   int down,   int chn   );


				int			Get_Number_Frames()    {    return  nframes;   }     //  mp3  decodes in BLOCKS of original file bites.  Such a block to decompress is called a FRAME.

				int			Get_Frame_Size()          {    return  framesize;   }   //   the size of a mp3 block of bytes to INDEPENDANTLY uncompress


				long		Get_Total_FileSize_inBytes_OutputCoords()   {   return  (nframes *  framesize);    }



				int			Get_MemberVar_FilePosition_OutputCoords()         {  return  filepos;   }     //   OUTPUT-BYTES  file position,   offsetted from the header

				int			Get_Calced_FilePosition_OutputCoords()    	          {  return    (  ( curframe -1 ) * framesize     +   framepos  );    }   //  this is correct,  in Virtual  Output-Coords
				
				                    //  where the file pos would be if there was NO compression( expanded to resemble the  'OUTPUT index'  of current byte after un-compress )   2/10
									//    ( actuall it seems like it gets the same value as    Get_MemberVar_FilePosition_OutputCoords() 




				void		Set_Frame_Timing_Correction(  long  newValue  );    // Needed this to fix the SEEK bug.   8/2012

				long		Seek_fromBeginning_in_OutputCoords(   long  newPos   );        //  Now works fine.    8/2012


				long      Seek_fromBeginning_in_OutputCoords_BlockAlaign(  long  newPos  );                  //   DEBUG Only:    newPos  must be MOD value with 4608 Frame Size
				long		Seek_WalkfromBeginning_in_OutputCoords(  long  newPos,   long  blockSize   );   //   DEBUG Only:    the most  ACCURATE   7/2012






			//	unsigned int   Get_Mode_Var()    {  return  mode;  }     //  NEW,  have weird BUG 

		//		void				Set_Mode_Var(  unsigned int  value  )    {  mode =   value;  }




// ***  New additions from  binfile ( former parent class of  ampegdecoder   )


						//  Need 2 verions of  'rawioctl()'   ....one from  ampegdecoder  and the other from binfile

	    int   rawioctl(            int code,   void *buf,   int len   );    //   ...Moved up to public from  virtual-protected
	    int   rawioctl_binfile(  int code,   void *buf,   int len   );   


		int   ioctl(  int code, void *buf, int len  )     {  return   rawioctl(   code,  buf, len );   }

		int   ioctl_binfile( int code )                      {   return   ioctl(  code, 0, 0 );        }




		boolm        eof()             {  return  ioctl_binfile( ioctlreof );  }

		int             read_binfile( void *buf,  int len );

		errstat       close_binfile();

		void           openmode_binfile(  uintm m,  int pos, int len   );

		void		   closemode_binfile();

		int              readunlogged_binfile(  void *buf,  int len  );

		void		   reset_binfile();

		boolm		  wsyncbyte_binfile();

		boolm		  invalidatebuffer_binfile(   int force );

		boolm		  invalidatewbuffer_binfile( int force );

		boolm	      setbuffer_binfile( int len );

		boolm	      setwbuffer_binfile( int len );

		int	             seekcur_binfile(int p);

		boolm	      putbits_binfile(uintm b, int n);

		int               rawwrite_binfile(const void *, int);

		int	              seek_binfile( int p );

		int               peek_binfile(  void *buf,   int len  );

		boolm         ewrite(const void *buf, int len);

		int              write_binfile(  const void *buf,   int len  );


	




protected:
  uintm   mode;     //  from binfile



private:
  enum { minibuflen = 8 };


  FileUni      *m_file;  


  long    m_frameRetardFactor;



  uint1       *buffer;
  uint1       *wbuffer;
  uint1         wminibuf[  minibuflen  ];				
  uint1         minibuf[    minibuflen]  ;


  int buflen;
  int filepos;
  int filewpos;
  int readfill;

  int        readerr;
  int bufpos;
  int bufmax;
  int bufstart;
  int         bufdirty;

  int filelen;
  boolm bitmode;
  int bitpos;
  boolm wbitmode;
  int wbitpos;
  uintm wbitbuf;
  int wbitfill;
  int writeerr;
  int wbufmax;
  int wbufpos;



  
											 // bitstream
  unsigned char  *bitbuf;
  int                   *bitbufpos;


  int   mpgetbit()
  {
    int val  =  (bitbuf[*bitbufpos>>3]>>(7-(*bitbufpos&7)))&1;

    (*bitbufpos)++;
    return val;
  }


  long  mpgetbits(  int n  )
  {
    if (!n)
      return 0; 

     unsigned long val =  (((((unsigned char)(bitbuf[(*bitbufpos>>3)+0]))<<24)|(((unsigned char)(bitbuf[(*bitbufpos>>3)+1]))<<16)|(((unsigned char)(bitbuf[(*bitbufpos>>3)+2]))<<8))>>(32-(*bitbufpos&7)-n))&((1<<n)-1);
    *bitbufpos+=n;

    return val;
  }


  void  getbytes(void *buf2, int n);



					// mainbitstream
  enum
  {
    mainbufmin  =   2048,
    mainbufmax =  16384		// *****  JPM   is this the max that I can read on one shot ????
  };


  unsigned char   mainbuf[  mainbufmax  ];
  int mainbufsize;
  int mainbuflow;
  int mainbuflen;
  int mainbufpos;


  void  setbufsize(  int size,   int min  );

  int    openbits();

  void  refillbits();   //   **************  BIG function,  ultimately call the read *************

  int    sync7FF();




													// decoder
  static int lsftab[4];
  static int freqtab[4];
  static int ratetab[2][3][16];

  int hdrlay;
  int hdrcrc;
  int hdrbitrate;
  int hdrfreq;
  int hdrpadding;
  int hdrmode;
  int hdrmodeext;
  int hdrlsf;

  int init;
  int orglay;
  int orgfreq;
  int orglsf;
  int orgstereo;


  int stream;		//  Can this be set for a streaming read????     1/10

  int slotsize;
  int nslots;

  int fslots;
  int slotdiv;

  int   seekinitframes;
  int   seekmode;



  char framebuf[  2 * 32 * 36 * 4  ];


  int  curframe;   //  need to subtract 1  to get the index to the currently load frame

  int  framepos;  //   is set during   Mp3Reader::rawseek().   It is the index  of the  'Current BYTE'  within the  frames buffer


  int  nframes;   //  total number of frames in the file

  int  framesize;   //  Each frame fetch will get this amount of UN-COMPRESSED  bytes.    framesize * nframes =  UNCOMPRESSED number of bytes in file 


  int     atend;
  float  fraction[2][36][32];

  int decodehdr(int);
  int decode(void *);



										//   ****   synth   ****************
  static float dwin[1024];
  static float dwin2[512];
  static float dwin4[256];
  static float sectab[32];

  int synbufoffset;
  float synbuf[2048];

  inline float muladd16a(float *a, float *b);
  inline float muladd16b(float *a, float *b);


  int convle16( int x )   {   return  x;  }


  int cliptoshort( float x )
  {
	 int foo  =  (int)x;
     return (foo<-32768)?convle16(-32768):(foo>32767)?convle16(32767):convle16(foo);
  }


  static void fdctb32(float *out1, float *out2, float *in);
  static void fdctb16(float *out1, float *out2, float *in);
  static void fdctb8(float *out1, float *out2, float *in);


  int tomono;
  int tostereo;
  int dstchan;
  int ratereduce;
  int usevoltab;
  int srcchan;
  int dctstereo;
  int samplesize;
  float stereotab[3][3];
  float equal[32];
  int equalon;
  float volume;

  int    opensynth();
  void synth(void *, float (*)[32], float (*)[32]);
  void resetsynth();

  void setstereo(const float *);
  void setvol(float);
  void setequal(const float *);




										//  ****************     layer 1/2   ******************8
  struct sballoc
  {
    unsigned int steps;
    unsigned int bits;
    int scaleadd;
    float scale;
  };

  static sballoc alloc[17];
  static sballoc *atab0[];
  static sballoc *atab1[];
  static sballoc *atab2[];
  static sballoc *atab3[];
  static sballoc *atab4[];
  static sballoc *atab5[];
  static const int alloctablens[5];
  static sballoc **alloctabs[3][32];
  static const int alloctabbits[3][32];

  static float multiple[64];

  static float rangefac[16];

  float scale1[2][32];
  unsigned int bitalloc1[2][32];
  float scale2[2][3][32];
  int scfsi[2][32];
  sballoc *bitalloc2[2][32];

  static void init12();
  void openlayer1(int);
  void decode1();
  void openlayer2(int);
  void decode2();
//  void decode2mc();




									//   ************   layer 3  *************************
  struct grsistruct
  {
    int gr;
    int ch;

    int blocktype;
    int mixedblock;

    int grstart;
    int tabsel[4];
    int regionend[4];
    int grend;

    int subblockgain[3];
    int preflag;
    int sfshift;
    int globalgain;

    int sfcompress;
    int sfsi[4];

    int ktabsel;
  };



  static int htab00[],htab01[],htab02[],htab03[],htab04[],htab05[],htab06[],htab07[];
  static int htab08[],htab09[],htab10[],htab11[],htab12[],htab13[],htab14[],htab15[];
  static int htab16[],htab24[];
  static int htaba[],htabb[];
  static int *htabs[34];
  static int htablinbits[34];
  static int sfbtab[7][3][5];
  static int slentab[2][16];
  static int sfbands[3][3][14];
  static int sfbandl[3][3][23];
  static float citab[8];
  static float csatab[8][2];
  static float sqrt05;
  static float ktab[3][32][2];
  static float sec12[3];
  static float sec24wins[6];
  static float cos6[3];
  static float sec36[9];
  static float sec72winl[18];
  static float cos18[9];
  static float winsqs[3];
  static float winlql[9];
  static float winsql[12];
  static int pretab[22];
  static float pow2tab[65];
  static float pow43tab[8207];
  static float ggaintab[256];

  int rotab[3][576];
  float l3equall[576];
  float l3equals[192];
  int l3equalon;

  float prevblck[2][32][18];
  unsigned char huffbuf[4096];
  int huffbit;
  int huffoffset;

  int ispos[576];
  int scalefac0[2][39];
  float xr0[2][576];

  static void init3();
  inline int huffmandecoder(int *valt);
  static void imdct(float *out, float *in, float *prev, int blocktype);
  static void fdctd6(float *out, float *in);
  static void fdctd18(float *out, float *in);
  void readgrsi(grsistruct &si, int &bitpos);
  void jointstereo(grsistruct &si, float (*xr)[576], int *scalefacl);
  void readhuffman(grsistruct &si, float *xr);
  void doscale(grsistruct &si, float *xr, int *scalefacl);
  void readscalefac(grsistruct &si, int *scalefacl);
  void hybrid(grsistruct &si, float (*hout)[32], float (*prev)[18], float *xr);
  void readsfsi(grsistruct &si);
  void readmain(grsistruct (*si)[2]);

  void openlayer3(int);
  void decode3();
  void seekinit3(int);
  void setl3equal(const float *);



protected:
  virtual errstat      rawclose();
  virtual int            rawseek(int pos);
  virtual int            rawread(void *buf, int len);
  virtual int            rawpeek(void *buf, int len);



public:

  enum
  {
    ioctlsetvol=4096, 
	ioctlsetstereo, ioctlsetequal32, ioctlsetequal576,  ioctlseekmode, ioctlseekmodeget
  };


  enum { seekmodeexact=0, seekmoderelaxed=1 };



  enum ioctlenum    //  from binfile
  {
    ioctlrtell,
    ioctlwtell,
    ioctlreof,
    ioctlweof,
    ioctlrlen,
    ioctlwunderflow,
    ioctlwunderflowclr,
    ioctlroverflow,
    ioctlroverflowclr,
    ioctlrerr,
    ioctlrerrclr,
    ioctlwerr,
    ioctlwerrclr,
    ioctlrmax,
    ioctlwmax,

    ioctlrbufset,
    ioctlrbufgetlen,
    ioctlrbufget,
    ioctlwbufset,
    ioctlwbufgetlen,
    ioctlwbufget,

    ioctlrfill,
    ioctlrfillget,

    ioctlrbo,
    ioctlrboget,
    ioctlwbo,
    ioctlwboget,
    ioctlwbfill,
    ioctlwbfillget,

    ioctlrflush,
    ioctlrflushforce,
    ioctlrcancel,
    ioctlwflush,
    ioctlwflushforce,
    ioctlwcancel,

    ioctlupdlength,

    ioctltrunc,
    ioctltruncget,

    ioctlblocking,
    ioctlblockingget,

    ioctllinger,
    ioctllingerget,

    ioctlrshutdown,
    ioctlwshutdown,
    ioctlwshutdownforce,

    ioctlrrbufset,
    ioctlrrbufgetlen,
    ioctlrwbufset,
    ioctlrwbufgetlen,
    ioctlrreof,
    ioctlrweof,

    ioctlrsetlog,

    ioctluser= 4096,
  };



 

  enum			 //  from binfile
  {
    modeopen=1,
    moderead=2,
    modewrite=4,
    modeseek=8,
    modeappend=16,
  };




// possible combinations:
//   0                                        null
//   moderead                                 istream
//   modewrite                                ostream
//   moderead|modewrite                       iostream
//   modeseek|moderead                        ifile
//   modeseek|moderead|modewrite              iofile
//   modeseek|moderead|modewrite|modeappend   appendable iofile
};




////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_MP3READER_H__D1E6D6F4_7AAA_4CC6_A012_A8D934B4EE5B__INCLUDED_)
