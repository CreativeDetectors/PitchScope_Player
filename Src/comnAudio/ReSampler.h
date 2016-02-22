/////////////////////////////////////////////////////////////////////////////
//
//  ReSampler.h   -   
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_RESAMPLER_H__5FF3D650_2035_4B09_AFE2_C77E94C44744__INCLUDED_)
#define AFX_RESAMPLER_H__5FF3D650_2035_4B09_AFE2_C77E94C44744__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//////////////////////////////////////////////////////////////////////////




struct  stage_rec 
{
	int	     up;
	double	 cutoff;
	int	     down;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


class   ReSampler  
{

public:
	ReSampler();
	virtual  ~ReSampler();


	bool	    Initialize(     int  srcFreq,   int   dstFreq,     long    srcStreamBufferSizeInBytes,   long&  retDstStreamBufferSizeInSamples, 
													 long  bytesPerSample,   bool stereoFlag,    CString&   retErrorMesg  );


	void		Resample_Signal(  	float x[],   int xlen,    float *( *yptpt ),    int *ylen   );




public:
	void		Calc_Resample_Ratios(   double x,  double accuracy,   int *nstages,   struct stage_rec  *procede     );

	void		Get_Filter_Coefficants(   double  stopedge,    int  *order,     double  *( *adircoeffs )    );


	void        order(   int	nnum,   int  num[32],   int nden,   int den[32],   int *nstages,   struct stage_rec procede[]   );

	void		factor(   int	numer,   int  denom,   int *nnum,  int  num[],    int *nden,   int  den[]   );

	void		rationalize(   double x,    double eta,    int *numer,   int *denom   );


	void		Interpolate_Data(   float  samplesArrayPtr[],   int  xlen,    float  *( *yaddpt ),     int *ylen,     struct stage_rec  *procede,  
							                                             int	order,	  double	a[],    int	nstage,   int nstages   );



public:
	bool     m_isInitialized;   //   new   2/2010



	long	 m_dstSampleCount;   //  how many samples it wil make from the input buffer of   4416    size  ( will be MORE than  4416   ) 

	char    *m_dDstSamplesPtr;   //   final  STREAMING  dsamples at    new freq   for STREAMING  decode of MP3



	/******  Dont try this again.  The dynamic memory alloc in  Interpolate_Data() has to stay the way it is, because of the NESTING of 
												sampleArrays in  Resample_Signal()  if it has more than one 'm_nstages'.    12/2011

			GreySolo_22hz.wav    has more than one  'm_nstages', and dmonstrated why this can NOT work


	long         m_sizeOfSampleArraysLeft;   //  = 0,    if  'm_leftSampleArray'  NOT allocated

	long         m_sizeOfSampleArraysRight;  // the 2 buffers can act independantly.  They have separate funtions to allocate and delete

	float       *m_leftSampleArray,   *m_rightSampleArray;      //  used to be  'yptptLeft'    in   WavConvert::Apply_SlowDown_wPitchShift()    12/11
	****/



	long	    m_srcStreamBufferSizeInSamples;    //   usually 4416


	double		m_sampleExpansionRatio;   //  multiply this times the  input  mp3SampleIdx  to get what the index is in  ReSampled-Coords





private:
	int      m_nstages; 

	struct  stage_rec    m_procede[ 32 ];    //  *** need to initialize ????  


	static   int         primearr[];

	static   double   coeff02[],   coeff03[],   coeff04[],   coeff05[],   coeff06[],   coeff07[],   coeff08[],   coeff09[];
};



////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_RESAMPLER_H__5FF3D650_2035_4B09_AFE2_C77E94C44744__INCLUDED_)
