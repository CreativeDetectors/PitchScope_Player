/////////////////////////////////////////////////////////////////////////////
//
//  WaveJP.h   -   
//
//  PitchScope Player  -  an animated pitch-detecting mp3 player (also used in PitchScope Navigtor)  
//
// Copyright (c)  2007-2016  James Paul Millard  (dba Creative Detectors)  <creativedetectors@gmail.com>      
// This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
////////////////////////////////////////////////////////////////////////////


#if !defined(WAVEJPH_118_INCLUDED)
#define WAVEJPH_118_INCLUDED

////////////////////////////////////////////////////////////////////////////////////////////////////////




int    WavFile_Open(   TCHAR*,   HMMIO *,   WAVEFORMATEX **,   MMCKINFO *  );
//  int    WaveOpenFile(   TCHAR*,   HMMIO *,   WAVEFORMATEX **,   MMCKINFO *  );
//	******		....see  wave.h  for MSoft sample code that is similar to these   12/06    *************



int    WavFile_Close_ReadingFile(  HMMIO *,   WAVEFORMATEX **  );
//  int    WaveCloseReadFile(  HMMIO *,   WAVEFORMATEX **  );




int    WavFile_Start_Reading_Data(  HMMIO *,   MMCKINFO *,   MMCKINFO *   );
//  int    WaveStartDataRead(  HMMIO *,   MMCKINFO *,   MMCKINFO *   );





int    WavFile_Load_Data_And_Format(    char           *pszFileName,      // (IN)
													UINT         *cbSize,                    // (OUT)
													WAVEFORMATEX  **ppwfxInfo,   // (OUT)   CALLING function must realease
													BYTE        **ppbData  	);             // (OUT)


long    Get_WavFiles_Current_FilePosition(   HMMIO  mmIO   );



bool    Seek_To_Virtual_FilePosition_WavFile(   HMMIO  *filesHandle,
														            MMCKINFO   *chunkInfo,			     //  Chunk Info
														            MMCKINFO   *parentChunkInfo,        //  Chunk Info PARENT
																    long       absoluteFilePosition,
																    CString&    retErrorMesg   );


////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // !defined(WAVEJPH_118_INCLUDED)
