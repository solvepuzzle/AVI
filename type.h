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
	DWORD  dwType;		/* '���A' */
};

/*== essential structure definition for list 1 ==*/
struct LIST{
	FOURCC fcc;			/* 'LIST' */
	DWORD  cb;			/* size of the list block */
	FOURCC subFcc;		/* 'hdrl', 'movi', 'strl'*/
};

/*== essential structure definition for AVI main header ==*/
struct AVIMAINHEADER{
    FOURCC fcc;					// ������'avih'
    DWORD  cb;					// ����Ƶ��c���j�p�A���]�A�̪쪺8�Ӧ줸�ա]fcc�Mcb��Ӱ�^
    DWORD  dwMicroSecPerFrame;	// ���W�V���j�ɶ��]�H�@�����^
    DWORD  dwMaxBytesPerSec;	// �o��AVI�ɪ��̤j��Ʋv
    DWORD  dwReserved1;		
    DWORD  dwFlags;				// AVI�ɪ������аO�A��p�O�_�t�����޶���
    DWORD  dwTotalFrames;		// �`�V��
    DWORD  dwInitialFrames;		// ���椬�榡���w��l�V�ơ]�D�椬�榡���ӫ��w��0�^
    DWORD  dwStreams;			// ���ɥ]�t���y���Ӽ�
    DWORD  dwSuggestedBufferSize;// ��ĳŪ�����ɪ��w�s�j�p�]����e�ǳ̤j�����^
    DWORD  dwWidth;				// ���W�Ϲ����e�]�H�Ϥ������^
    DWORD  dwHeight;			// ���W�Ϲ������]�H�Ϥ������^
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
};// ���w�o�Ӭy�]���W�y�Τ�r�y�^�b���W�D����������ܦ�m
// ���W�D������AVIMAINHEADER���c����dwWidth�MdwHeight�M�w

/*== essential structure definiton for bitmap data ==*/ 
struct AVISTREAMHEADER{
	FOURCC fcc;				/* 'strh' */ 
	DWORD  cb; 
	FOURCC fccType;    		// �y������:��auds���]���W�y�^�B��vids���]���W�y�^�B
							//��mids���]MIDI�y�^�B��txts���]��r�y�^			
	FOURCC fccHandler; 		// ���w�y���B�z�̡A��󭵵��W�ӻ��N�O�ѽX��				
	DWORD  dwFlags;    		// �аO�G�O�_���\�o�Ӭy��X�H�զ�O�O�_�ܤơH			
	DWORD  dwPriority;  	// �y���u�����ǡ]���h�ӬۦP�������y���u�����ǳ̰������q�{�y�^				
	DWORD  dwInitialFrames; // ���椬�榡���w��l�V��				
	DWORD  dwScale;   		// �o�Ӭy�ϥΪ��ɶ��ث�				
	DWORD  dwRate;
	DWORD  dwStart;   		// �y���}�l�ɶ�				
	DWORD  dwLength;  		// �y�����ס]���PdwScale�MdwRate���w�q�����^				
	DWORD  dwSuggestedBufferSize; // Ū���o�Ӭy��ƫ�ĳ�ϥΪ��w�s�j�p 			
	DWORD  dwQuality;   	// �y��ƪ��~����С]0 ~ 10,000�^				
	DWORD  dwSampleSize; 	// Sample���j�p				
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
	FOURCC  fcc;  // ������'idx1'
	DWORD   cb;
};

struct AVIOLDINDEX_ENTRY{
	DWORD   dwChunkId;   // ��x����ƶ����|�r���X
	DWORD   dwFlags;     // ��������ƶ��O���O����V�B�O���O 'rec' �C����T
	DWORD   dwOffset;    // ����ƶ��b��󤤪������q
	DWORD   dwSize;      // ����ƶ����j�p
};


#endif

