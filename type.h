#ifndef _AVIDEFINITION_H_
#define _AVIDEFINITION_H_

/* conversion for windows type */
#define DWORD int
#define FOURCC int
#define WORD short
#define LONG long

/* size (bytes) for each element */
#define sizeofFCC 4
#define sizeofCB 4

/* four character code reference ***/
#define LIST_fcc 0x5453494C
#define RIFF_fcc 0x46464952
#define AVI_fcc  0x20495641
#define WAVE_fcc 0x45564157

#define hdrl_fcc 0x6c726468
#define avih_fcc 0x68697661

#define strl_fcc 0x6c727473
#define strh_fcc 0x68727473
#define strf_fcc 0x66727473
#define strd_fcc 0x64727473
#define strn_fcc 0x6e727473

#define vids_fcc 0x73646976
#define auds_fcc 0x73647561
#define txts_fcc 0x73747874
#define odml_fcc 0x6c6d646f

#define movi_fcc 0x69766f6d
#define rec_fcc  0x20636572
#define JUNK_fcc 0x4b4e554a
#define idx1_fcc 0x31786469
#define fmt_fcc  0x20746d66
#define data_fcc 0x61746164

#define x0wb_fcc 0x62773030
#define x1wb_fcc 0x62773130
#define x0dc_fcc 0x63643030
#define x1dc_fcc 0x63643130
#define x0db_fcc 0x62643030
#define x1db_fcc 0x62643130

/***********dwFlag**********/
#define AVIF_HASINDEX           0x00000010
#define AVIF_MUSTUSEINDEX       0x00000020
#define AVIF_ISINTERLEAVED      0x00000100
#define AVIF_TRUSTCKTYPE        0x00000800
#define AVIF_WASCAPTUREFILE     0x00010000
#define AVIF_COPYRIGHTED        0x00020000

/*== essential structure definition for RIFF header ==*/
struct RIFFHEADER{
	FOURCC fcc; 		/* 'RIFF' */
	DWORD  cb;			/* total file size ( the first 8 bytes are not included ) */      	
	DWORD  dwType;		/* '型態' */
};

/*== essential structure definition for list 1 ==*/
struct LIST{
	FOURCC fcc;			/* 'LIST' */
	DWORD  cb;			/* size of the list block */
	FOURCC subFcc;		/* 'hdrl', 'movi', 'strl'*/
};

/*== essential structure definition for AVI main header ==*/
struct AVIMAINHEADER{
    FOURCC fcc;					// 必須為'avih'
    DWORD  cb;					// 本資料結構的大小，不包括最初的8個位元組（fcc和cb兩個域）
    DWORD  dwMicroSecPerFrame;	// 視頻幀間隔時間（以毫秒為單位）
    DWORD  dwMaxBytesPerSec;	// 這個AVI檔的最大資料率
    DWORD  dwReserved1;		
    DWORD  dwFlags;				// AVI檔的全局標記，比如是否含有索引塊等
    DWORD  dwTotalFrames;		// 總幀數
    DWORD  dwInitialFrames;		// 為交互格式指定初始幀數（非交互格式應該指定為0）
    DWORD  dwStreams;			// 本檔包含的流的個數
    DWORD  dwSuggestedBufferSize;// 建議讀取本檔的緩存大小（應能容納最大的塊）
    DWORD  dwWidth;				// 視頻圖像的寬（以圖元為單位）
    DWORD  dwHeight;			// 視頻圖像的高（以圖元為單位）
    DWORD  dwScale;
    DWORD  dwRate;
    DWORD  dwStart;
    DWORD  dwLength;
};

/* vids */
/*== essential structure definition for AVI stream header ==*/
struct rcRECT{
	short int left;
	short int top;
	short int right;
	short int bottom;
};// 指定這個流（視頻流或文字流）在視頻主視窗中的顯示位置
// 視頻主視窗由AVIMAINHEADER結構中的dwWidth和dwHeight決定

/*== essential structure definiton for bitmap data ==*/ 
struct AVISTREAMHEADER{
	FOURCC fcc;				/* 'strh' */ 
	DWORD  cb; 
	FOURCC fccType;    		// 流的類型:‘auds’（音頻流）、‘vids’（視頻流）、
							//‘mids’（MIDI流）、‘txts’（文字流）			
	FOURCC fccHandler; 		// 指定流的處理者，對於音視頻來說就是解碼器				
	DWORD  dwFlags;    		// 標記：是否允許這個流輸出？調色板是否變化？			
	DWORD  dwPriority;  	// 流的優先順序（當有多個相同類型的流時優先順序最高的為默認流）				
	DWORD  dwInitialFrames; // 為交互格式指定初始幀數				
	DWORD  dwScale;   		// 這個流使用的時間尺度				
	DWORD  dwRate;
	DWORD  dwStart;   		// 流的開始時間				
	DWORD  dwLength;  		// 流的長度（單位與dwScale和dwRate的定義有關）				
	DWORD  dwSuggestedBufferSize; // 讀取這個流資料建議使用的緩存大小 			
	DWORD  dwQuality;   	// 流資料的品質指標（0 ~ 10,000）				
	DWORD  dwSampleSize; 	// Sample的大小				
	struct rcRECT rcFrame;
};

struct BITMAPINFOHEADER{
    FOURCC fcc;			/* 'strf' */
    DWORD  cb;
    DWORD  biSize;
    LONG   biWidth;
    LONG   biHeight;
    WORD   biPlanes;
    WORD   biBitCount;
    DWORD  biCompression;
    DWORD  biSizeImage;
    LONG   biXPelsPerMeter;
    LONG   biYPelsPerMeter;
    DWORD  biClrUsed;
    DWORD  biClrImportant;
};

/* auds */
/*== essential structure definition for wave data ==*/
struct WAVEFORMATEX{
    FOURCC fcc;
    DWORD  cb;    
    WORD  wFormatTag;      
    WORD  nChannels;       
    DWORD nSamplesPerSec;  
    DWORD nAvgBytesPerSec; 
    WORD  nBlockAlign;     
    WORD  wBitsPerSample;    
};

/*== essential structure definition for AVI index ==*/
struct AVIOLDINDEX{
	FOURCC  fcc;  // 必須為'idx1'
	DWORD   cb;
};

struct AVIOLDINDEX_ENTRY{
	DWORD   dwChunkId;   // 表徵本資料塊的四字元碼
	DWORD   dwFlags;     // 說明本資料塊是不是關鍵幀、是不是 'rec' 列表等資訊
	DWORD   dwOffset;    // 本資料塊在文件中的偏移量
	DWORD   dwSize;      // 本資料塊的大小
};


#endif

