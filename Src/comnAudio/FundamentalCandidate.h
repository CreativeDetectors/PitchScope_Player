/////////////////////////////////////////////////////////////////////////////
//
//  FundamentalCandidate.h   -   used for Octave Detection
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined( _FUNDAMENTALCANDIDATE_H_  )

#define _FUNDAMENTALCANDIDATE_H_

/////////////////////////////////////////////////////////////////////


class  PolyMemberVar;

class  SndSample;



#define  MEMBERvARaDDRESScOUNTfc  24


#define  NEIGHBORcOUNT  24


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////




typedef struct    // **** TRIM this down( to be saved in DetectZone )   ...do I use all these vars ??   10/03 *********
{  

	short   filterCodeCompos;		//   CompositeMap   FILTERING ( for Ymax creation )
	short   kernalWidthCompos;    



	long    scalepitchWeitSupScore;						  //   Transform data  WEIGHTS

	long    harmDensityWeitSupScore;

	long    octaveCountWeitSupScore;

	long    hPairsToneSupScore;



	////////////////////////////////////////////////////////////////////////////////

																	    //  Y-MAX  Enhancement
	bool    lonePixelFilterEnabled; 

	short   holePlugKernalWidth;
	short	  holePlugKernalPercentage;
	short   holePlugPixelDetectThresh;
	bool    holePlugHolesOnly;
	short   holePlugMaxIterations;



	short   octvPruneScoreThreshAvgTone;   
	
	bool    octaveListFilterEnabled;






						/////////////////  New  BitmapObjectCreator   vars  for    //////////////////


	short   pixCountInBriteRunBMOC;    // =     6;		    **** ADJUST***************  
	short   pixCountInDarkRunBMOC;   //  =     4;		    **** ADJUST***************

	bool    doKernalReadBMOC;      //   =    false;

	short   pixThresholdBMOC;   //  **** This determines the 




	short   notesDefaultLoudness;   //  NEW,  always return to this value when opening the detct dialog

	short   notesLoudness;    //   in percent[0 - 100],   How brite( in grey ) the notes will be on Edit View 




	short   detectionValuePruneThresh;   //   new,  prune all notes whese detection value is lower than this.   5/07  



} SPitchListCreateParms;		






		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////



typedef struct  
{  

	short   filterCodeDft;				 //   reading logDFT with  FILTERING ( for DerivativeMaps creation 
	short   kernalWidthDft;    



	short   pixThresholdHPairs;    //  15  [ 10 - 50 ?? ]     when CompositeVerter reads from HPairsTrans, for a Harm-Row to be counted it must be lighter than this


			//  ****  Any NEW FIELDS  must be added to :    
			//							1)   SPitchListWindow::On_Rebuild_DZone_Bitmaps_shSPL() 
			//							2)   lastDerivitiveMapsCreateParmsGLB
			//		...what else( 9/06 )


} DerivitiveMapsCreateParms;	





							////////////////////////////////////////////////////////////////

typedef struct 
{  

// **** CAREFUL  if  MODIFY,     must sync with    lastOctaveCorrectParmsGLB *********** 


	short   midiNeighborhoodPixelWidth;       //  for   Calc_Neighborhoods_AverageMidi()


	long    gappedWeight;

	long    spikesWeight;

	long    disorderWeight;    //  Now used for the targets M2 values as a disorder to be subtracted




	long    closenessNeibWeight;
										  //   ....SUBparms  for Closeness Weit calc
		bool    useSquarerootPixelCntClose;

		bool    cubeDifferencesClose;        //   false :   SQUARE differences

		long    boostForCubedDiffsClose;
		
		long    boostForSquaredDiffsClose; 





	long    changeIn1belowM2;   


	long    oneAboveWeight;       //   NEW,   7/07     was:   autoCorrelation;


	long    harmPairsWeightPercent;
	long    harmPairsWeightRawScore;


	long    harmsComponentCount;



	bool    doIterationsForOctaveCorrection;   //   see  Octave_Adjustment_Iterated()


	//   long    spikeTrimCode;   *** DISABLED 6/07  *** 



	//   ****  ADD new members to   OctaveAdjustDlg::Save_OctaveCorrectParms_Settings_toGlobal()  *****


} OctaveCorrectParms;	





////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


union   MemberVarValue
{
	short	   shortValue;
	long	   longValue;
	double	 doubleValue;
};

									/////////////////////////////////////////////////////////////////////////
									/////////////////////////////////////////////////////////////////////////


class   PolyMemberVar
{

public:
	PolyMemberVar();
	~PolyMemberVar();


	void		SetType(  int  type  );
	int			  GetCType()                  {   return    m_Sql_C_Type;   }


	void       	Zero_Value();
	void		Max_Out_Value();
	void        Initialize_Value(   short  opCode   );




	operator   short();           //  Different kinds of CASTS
	operator   long();
	operator   double();


	short&       operator =    ( short );
	long&        operator =    ( long );
	double&     operator =   ( double );

	short&       operator +=   ( short );
	long&        operator +=   ( long  );
	double&     operator +=  ( double );


	bool       operator >   ( short );
	bool       operator >   ( long  );
	bool       operator >   ( double );

	bool       operator <   ( short );
	bool       operator <   ( long  );
	bool       operator <   ( double );



	PolyMemberVar&     operator =   (PolyMemberVar &);



	
//  	CString&	GetStringValue(  LPCTSTR  lpszFmt = NULL  );

/***
	PolyMemberVar&     operator =   (PolyMemberVar &);

  	operator   const char*();

	PolyMemberVar(  PolyMemberVar &Fld  );
***/

	
public:
	MemberVarValue      m_RealFV;	//  the actual FieldValues,  but NOT used if( m_memberVarAddress != NULL  ) 

	MemberVarValue	   &m_FV;		//  was  'FV'    When one Field points to another **** ????  What for ???  ****


	void       *m_memberVarAddress;   // If this is NULL,  only then is   m_RealFV   accessed




	short       m_membervarID;         //    UNKNOWNmEMBRid,     MEMBERspikeScoreID,     MEMBERtargetsM2id   

	CString    m_memberVarName;

	short	     m_Sql_C_Type;           //    UNKNOWNtYPEjm,   LONGjm,     SHORTjm,    DOUBLEjm   };  






	short       m_opCode;       //   will it Average( accumulate a sum,  find Min/Max    


//	short       m_sPitchListType;




	enum  memberVArTypes     {   UNKNOWNtYPEjm,       LONGjm,     SHORTjm,    DOUBLEjm   };   

	enum  opCodes                  {   UNKNOWoPcODEpm,   AVERAGEpm,     MINpm,   MAXpm      };   


	enum  sPitchListType           {   UNKNOWlISTtYPEpm,   FILElistTYPEpm,     CALCEDlistTYPEpm      };   
	
};




////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class   FundamentalCandidate    
{ 		
public:  							 
    FundamentalCandidate();
	virtual	 ~FundamentalCandidate()    {  }	



				short		Get_MidiPitch()          {   return   m_midiPitch;   }

				short		Get_ScalePitch();



	static    short			Get_Octave_Candidate_Count()    {   return 4;  }     //  Make many changes if vary this number !!!
      

	static    short         Get_Octave_Index(  double  freq  );   //  CHANGE if Oct-0 gets a new frequency       



	static    short			Harmonic_Count()            {    return  12;   }
	static    short			Harmonic_Count_Extra()      {    return  18;   }   //  Set this equal to 12 if not want to do a large harm set




			double 			Calc_Harms_Neighborhood_Strength(   short  harmIdx,    bool  useWeights,     bool&  retIsUndefinedDev     );   // ***  NEW,   6/07

			double 			Calc_Neighborhoods_AverageMag(       short  harmIdx,     bool&  retIsUndefinedDev     );



				long  		Get_neighborMags_Magnitude(   short  midiOffset    );

				bool 		Calc_All_Harms_NeighborhoodStrength(    bool  useWeights,   CString&  retErrorMesg    );


 														                                         
				short       Read_Hamonics_Mag(    short  harmIdx,   bool&  isUndefined   );
				void        Set_Hamonics_Mag(     short  harmIdx,   short  val           );





public:          
	short    m_midiPitch;	
	
	short    m_octaveIdx;       //    { 0 -  3 }  The VIRTUAL octaveIndex ABOVE the SUBoctaves in array

	bool     m_isAsubOctave;    //  dummy-Candidates just for calc of   {  m_1belowM2,  m_2belowM4,   m_3belowM8 }





	short       m_harmonicMags[  18   ];	           //    18  stores,     -9 if undefined( ie:   Band was off DFTmap
	
	PairLong    m_neighborMags[  NEIGHBORcOUNT  ];    //     24  is the  current count




	short		m_missedHarmonicBandCnt;      //   If we try to read off the bottom of the logDFT,  we count these here	





															//     ***  DECISION Critera  ****



	long		m_spikeScore;            //  the Spike ENVIRONMENT( in Neighboring 3 OctCandids ) create a POSITIVE score for THIS FundCandidate


	double      m_harmsNeighborhoodStrength;    //  **** BIG,  really decides the Octave choice 1/2016 ******            New,  6/2007

	
	short		m_avgHarmonicMag;     //  Another way to calculate the STRENGTH of harmProfile  (  the  'AREA'  under the  Harmonic-Envelope  )




	long		m_finalOctvCandidScore;    //  *** A FINAL result ***     See   FundCandidEvaluator::Calc_Best_Octave_Candidate()



	CString		m_ultimateScoresCode;     //    An ALPHABETICAL Code :  its digits reflect its BestScores for the 4 major scores






public:


enum  membervarIDs
{  UNKNOWNmEMBRid,     MEMBERspikeScoreID,     MEMBERtargetsM2id,     MEMBzEROcROSSINGs,   MEMBaUTOcORRELATION,
	MEMB1bELOWSm2,       MEMBaVGgAPPED,    MEMBdETCTIONsCORE  
};


/***

enum  minimumVetos     //   for   m_failMinScoreCode
	{  NOFAIL,   AVGgAPPEDmAG,  SPIKEsCORE,   M2sCORE,  CENTROIDsCORE,   UPHILLsCORE,   ONEaBOVEfAIL,  
	    AVGhEIGHT, ONEBELOWnOTaM2,   DETECTIONsCORE      };
***/	

// when install new membervarIDs,  do a full search for use of previous ones




									//  Flag values for HarmonicMags   
#define  INITIALIZEhMAG  -1
#define  UNDEFINEDhMAG  -9
};






////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif   // __H_