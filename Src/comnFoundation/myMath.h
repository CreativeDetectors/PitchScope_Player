//   myMath.h       [  FIX and other math ]
//
//  Copyright (C) 2007   Creative Detectors (of San Francisco, California).  All Rights Reserved.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#if !defined( _MYMATH_H_  )

#define _MYMATH_H_

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////





// Defines, constants, and global variables

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////




typedef  long   FIX;
typedef  long   FRAC; 

#define SPFRAC long     //  my 'SPFRAC' ( is it different than MAC  Do NOT think so
                        //                 ...in 'types.h'   "  typedef long Fract;  "



//////////////////////////////////////////////////////////////////////////////////

typedef struct {  float a;    float b;
		          float c;    float d;
		          float e;    float f;   } MATRIX;


typedef struct {  float  ep0X, ep0Y;
		          float  ep1X, ep1Y;
		          float  ep2X, ep2Y;
		          float  ep3X, ep3Y;    } FloatBOX;  // has 4 points( not 2 like 'Rect' )



typedef struct {  long hRat;      long vRat; 	  // for 'fixed-op scale' on SHORTs (5/94)
                  long hRatInv;   long vRatInv;   
                  short   unDefFlg;               } CoordTRANS;





								/////////////////////////////////////////////


typedef struct {   long      numEntries;       
                   SPFRAC   *Cos;		  
				   SPFRAC   *Sin;
				   double    curAngFreq;    				   
} TRIGTABLEfrc;



       //////////////  general  ///////////////////////////////////////////////////////


#define PI 3.141592653589793
#define twoPI 6.283185307179586


#define absj(x) (((x) < 0) ? -(x) : (x))
#define signj(x) ((x) > 0 ? 1 : ((x) == 0 ? 0 : (-1)))
#define maxj(x,y) (((x) > (y)) ? (x) : (y))
#define minj(x,y) (((x) < (y)) ? (x) : (y))


#define max3j( x, y, z )    (    (   (((x) > (y)) ? (x) : (y)) > (z)  ) ? (((x) > (y)) ? (x) : (y))  : (z)   )

#define min3j( x, y, z )    (((((x) < (y)) ? (x) : (y)) < (z)) ? (((x) < (y)) ? (x) : (y)) : (z))





//////////////  my ARITHMETIC  /////////////////////////////////////////////////

/**** 
          SUMMARY of fixed point use:
  1) dont really need a lot of accuracy, really only for getting DATA to screen image
  2) careful 'printer-gports' are a lot bigger( 2400 pix wide )

*****/



#define FXPT 8

#define FIXDEC   256.0          //   2 ** 8     for quick calc



#define FRCPT 16

#define FRCDEC  65536.0      //   2 ** 16   for quick calc



#define SPFRACPT 30

#define SPFRACFACT 1073741824.0    // mult to get 'SPFRAC' from 'double'  { 2**30 }




#define NUPT2 4     //  must adjust if chang  FXPT or FRCPT 






				      ////////////////////////////////////////////

#define FIXMASK  0x00FFL


#define Fx2I(A) (A >> FXPT)
#define I2Fx(a) (a << FXPT)

#define Frc2I(F) (F >> FRCPT)
#define I2Frc(f) (f << FRCPT)   // rarely used 

#define Frc2Fx(F) (F >> (FRCPT - FXPT))
#define Fx2Frc(f) (f << (FRCPT - FXPT))



#define  FXDELIN  (-32768L << FXPT)
#define  SHDELIN   -32000              //  was:   #define DELIN -32000   



#define HybMul(a,b) ((((b>>NUPT2)*(a>>FXPT))>>NUPT2)+((b*(a & FIXMASK))>>FRCPT))
 //  hybrid: FIX x FRAC= FIX:  ABOVE is very accurate with a high limit(see blk\fixpoint.c) 


#define FxMul(a,b) ( (a*b) >> FXPT )      //   3/1994  test 


FRAC    MakFrc(  long u,  long l  );          



// #define  FracMulJM(a,b) (  (a>>15)*(b>>15)  )       //    BEST ???   6/2000  test 
#define  FracMulJM(a,b) (  (a >> 15)*(b >> 15)  ) 



////////////////////////////////////////////////


#endif   
