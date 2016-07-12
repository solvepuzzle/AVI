//分離avi影音
#include"stdio.h"
#include"stdlib.h"

#include "read.h"
#include "type.h"

int main(int argc ,char* argv[])
{
	FILE *fp;
	struct AVIDATA* aviData;
	aviData = (struct AVIDATA*)calloc(1, sizeof(struct AVIDATA));
	
	printf("請輸入avi檔案名稱 : ");
	scanf("%s", argv[0]);

	if((fp = fopen(argv[0], "rb+")) == NULL){
		printf("開啟 %s 失敗\n!", argv[0]);
		exit(EXIT_FAILURE);
	}
 
	readFileInfo(fp, aviData);//讀取檔案
   
	if(aviData->aviMainHeader.dwStreams > 1){//如果本檔包含流的個數大於1就開始執行
		extractAudio(fp, aviData);//分出音訊檔
		extractVideo(fp, aviData);//分出影像檔
	}
	
	free(aviData->indexEntry);
	free(aviData);
	fclose(fp);
	return 0;
}

void readFileInfo(FILE *fp, struct AVIDATA* aviData)
{
	int i;
		
	//讀取一開始的基本資料
	fread(&aviData->riffHeader, sizeof(struct RIFFHEADER), 1, fp);			//RIFFHEADER
	fread(&aviData->list1, sizeof(struct LIST), 1, fp);						//LIST
	fread(&aviData->aviMainHeader, sizeof(struct AVIMAINHEADER), 1, fp);	//AVI Main Header
	
	for(i=0;i<aviData->aviMainHeader.dwStreams;i++){				
		fread(&aviData->subList[i], sizeof(struct LIST), 1, fp);			//sub LIST
		fread(&aviData->aviStreamHeader[i], sizeof(struct AVISTREAMHEADER), 1, fp);	//AVI Stream Header
		
		switch(matchStreamType(aviData->aviStreamHeader[i].fccType)){//判斷是什麼檔案, 0 = vids, 1 = auds
			case 0:	/* vids */
				fread(&aviData->bitmapInfoHeader, sizeof(struct BITMAPINFOHEADER), 1, fp);//BitMap Header
				//跳到下一個sub LIST
				fseek(fp, sizeofFCC+sizeofCB+aviData->subList[i].cb-(sizeof(struct BITMAPINFOHEADER)+sizeof(struct LIST)+sizeof(struct AVISTREAMHEADER)), SEEK_CUR);
				
				if(i == 0){
					aviData->vidsIDb = x0db_fcc;	/* vids(uncompressed) */	
					aviData->vidsIDc = x0dc_fcc;	/* vids(compressed) */
				}else{
					aviData->vidsIDb = x1db_fcc;
					aviData->vidsIDc = x1dc_fcc;
				}
				
				break;
			case 1:	/* auds */
				fread(&aviData->waveFormatex, sizeof(struct WAVEFORMATEX), 1, fp);
				//跳到下一個sub LIST
				fseek(fp, sizeofFCC+sizeofCB+aviData->subList[i].cb-(sizeof(struct WAVEFORMATEX)+sizeof(struct LIST)+sizeof(struct AVISTREAMHEADER)), SEEK_CUR);				
				
				if(i == 0){
					aviData->audsID = x0wb_fcc;	/* auds */
				}else{
					aviData->audsID = x1wb_fcc;
				}
				
				break;
			default:
				printf("aviStreamHeader[%d] fccType %#x : 錯誤", i,aviData->aviStreamHeader[i].fccType);				
				break;				 		
		}	
	}
	
	fseek(fp, sizeof(struct RIFFHEADER)+sizeofFCC+sizeofCB+aviData->list1.cb, SEEK_SET);
   	
	//其他的資訊都不要 直到找到movi
	fread(&aviData->list2, sizeof(struct LIST), 1, fp);	
	while(aviData->list2.subFcc != movi_fcc){
		//因為要判斷型態	所以多讀了4byte 要在減回去
		fseek(fp, -sizeofFCC+aviData->list2.cb, SEEK_CUR);
		fread(&aviData->list2, sizeof(struct LIST), 1, fp);
	} 
   
   aviData->posOfFirstDataBlock = ftell(fp);//紀錄目前的位置
   
	//如果一個AVI檔包含有索引塊，則應在主AVI資訊頭的描述中，也就是AVIMAINHEADER結構的dwFlags中包含一個AVIF_HASINDEX標記。
	if((aviData->aviMainHeader.dwFlags & AVIF_HASINDEX)){	
		fseek(fp,-sizeofFCC+aviData->list2.cb, SEEK_CUR); //減去sizeofFCC(4byte)就剛好到movi位置
    	fread(&aviData->avioldIndex, sizeof(struct AVIOLDINDEX), 1, fp);    	
    	//確定這些是不要得
		while(aviData->avioldIndex.fcc != idx1_fcc){
			fseek(fp, aviData->avioldIndex.cb, SEEK_CUR);
    		fread(&aviData->avioldIndex, sizeof(struct AVIOLDINDEX), 1, fp);
		}    	    	
		//算出有幾塊IndexItem
		aviData->numOfIndexItem = aviData->avioldIndex.cb/sizeof(struct AVIOLDINDEX_ENTRY);
		
    	aviData->indexEntry = (struct AVIOLDINDEX_ENTRY*)calloc(aviData->numOfIndexItem, sizeof(struct AVIOLDINDEX_ENTRY));
		//讀進去
    	fread(aviData->indexEntry, sizeof(struct AVIOLDINDEX_ENTRY), aviData->numOfIndexItem, fp);    	
    	//確定偏移的錯誤
		aviData->posError = aviData->posOfFirstDataBlock-aviData->indexEntry[0].dwOffset;		
	}else{ 
		printf("抱歉！這個程式無支援, 沒有index資訊的影像！");
		exit(EXIT_FAILURE);
	}
}

//分出影像
void extractVideo(FILE *fp, struct AVIDATA* aviData)
{	
	int i = 0;
	int vidRawSize = 0;	
	int numVidIndex = 0;
	DWORD temp;
	char *buffer;
	
	//建立一個新的avi影像檔,只有影像
	FILE *avifp;
	if((avifp = fopen("video.avi", "wb+")) == NULL){
		printf("創造影像失敗！\n");
		exit(EXIT_FAILURE);
	}

	printf("影像分出中......\n");   
   
	//照者格式存檔
	struct RIFFHEADER riffHeader;
	riffHeader.fcc = RIFF_fcc;
	riffHeader.dwType = AVI_fcc;
	fwrite(&riffHeader, sizeof(struct RIFFHEADER), 1, avifp);//RIFF
	
	aviData->list1.cb = sizeof(struct LIST)-(sizeofFCC+sizeofCB)+sizeof(struct AVIMAINHEADER)+sizeof(struct LIST)+sizeof(struct AVISTREAMHEADER)+sizeof(struct BITMAPINFOHEADER);
	aviData->aviMainHeader.dwStreams = 1;
	fwrite(&aviData->list1, sizeof(struct LIST), 1, avifp);//LIST
	fwrite(&aviData->aviMainHeader, sizeof(struct AVIMAINHEADER), 1, avifp);/* 'avih' */
	
	if(aviData->aviStreamHeader[0].fccType == vids_fcc){
		aviData->subList[0].cb = sizeof(struct LIST)+sizeof(struct AVISTREAMHEADER)+sizeof(struct BITMAPINFOHEADER)-(sizeofFCC+sizeofCB);
		fwrite(&aviData->subList[0], sizeof(struct LIST), 1, avifp);					//sub LIST
		fwrite(&aviData->aviStreamHeader[0], sizeof(struct AVISTREAMHEADER), 1, avifp);	/* 'str.h' */
		fwrite(&aviData->bitmapInfoHeader, sizeof(struct BITMAPINFOHEADER), 1, avifp);	/* 'strf' */
	}else{//同上
		aviData->subList[1].cb = sizeof(struct LIST)+sizeof(struct AVISTREAMHEADER)+sizeof(struct BITMAPINFOHEADER)-(sizeofFCC+sizeofCB);
		fwrite(&aviData->subList[0], sizeof(struct LIST), 1, avifp);
		fwrite(&aviData->aviStreamHeader[1], sizeof(struct AVISTREAMHEADER), 1, avifp);
		fwrite(&aviData->bitmapInfoHeader, sizeof(struct BITMAPINFOHEADER), 1, avifp);
	}	
	
	//JUNK，用來填充內部資料，應用程式應該忽略這些資料塊的實際意義。
	fwrite(&aviData->list2 ,sizeof(struct LIST), 1, avifp);
	
	i=0;
	//把原始的IndexItem先存入暫存器內
	while(i < aviData->numOfIndexItem){							     
		//如果是db或dc就要寫入
		if(aviData->indexEntry[i].dwChunkId == aviData->vidsIDb || aviData->indexEntry[i].dwChunkId == aviData->vidsIDc){                               		  		
    		fseek(fp, aviData->posError+aviData->indexEntry[i].dwOffset, SEEK_SET);//對齊偏移
    		aviData->indexEntry[i].dwOffset = ftell(avifp);	//修改偏移量
    		fread(&temp, sizeof(DWORD), 1, fp);				//讀識別碼
    		fwrite(&temp, sizeof(DWORD), 1, avifp);			//寫識別碼
			
    		fread(&temp, sizeof(DWORD), 1, fp);				//讀資料塊的大小
    		fwrite(&temp, sizeof(DWORD), 1, avifp);			//寫資料塊的大小
			
    		buffer = (char*)calloc(temp, sizeof(char));		//用來暫存video資料
    		fread(buffer, sizeof(char), temp, fp);			//讀video資料
    		fwrite(buffer, sizeof(char), temp, avifp);		//寫video資料
			
    		free(buffer);
    		
    		vidRawSize += (temp+sizeofFCC+sizeofCB);		//增加大小
    		numVidIndex++;									//加一塊
		}
    	i++;
	}
   
	//保存index大小
	struct AVIOLDINDEX avioldIndex;
	avioldIndex.fcc = idx1_fcc;
	avioldIndex.cb = sizeof(struct AVIOLDINDEX_ENTRY)*numVidIndex;
	fwrite(&avioldIndex, sizeof(struct AVIOLDINDEX), 1, avifp);
	
	//寫index
	i=0;
	while(i < aviData->numOfIndexItem){
		if(aviData->indexEntry[i].dwChunkId == aviData->vidsIDb||aviData->indexEntry[i].dwChunkId == aviData->vidsIDc)
			fwrite(&aviData->indexEntry[i], sizeof(struct AVIOLDINDEX_ENTRY), 1, avifp);
		i++;
	}	
	
	//保存檔案大小總數
	riffHeader.cb = ftell(avifp)-(sizeofFCC+sizeofCB);
	rewind(avifp);
	fwrite(&riffHeader, sizeof(struct RIFFHEADER), 1, avifp);
	
	//修改list2 因為這個新檔案不使用JUNK
	fseek(avifp, sizeofFCC+sizeofCB+aviData->list1.cb, SEEK_CUR);	//沒有JUNK的大小
	aviData->list2.cb = vidRawSize+sizeofFCC;						//加上movi標頭檔
	fwrite(&aviData->list2, sizeof(struct LIST), 1, avifp);			//寫
	
	fclose(avifp);
	printf("成功分離影像！！！檔名是: video.avi \n");
}
//分出音訊
void extractAudio(FILE *fp, struct AVIDATA *aviData)
{	
	int i=0;
	DWORD temp;
	int audRawSize=0;
	char *buffer;
	
	printf("音訊檔分離中......\n");
	
	//建立一個新的wav檔,只有音訊
	FILE *wavfp;
	if((wavfp = fopen("audio.wav","wb+")) == NULL){
		printf("創造音訊檔失敗！\n");
		exit(EXIT_FAILURE);
	}
    
	//也是照者格式存檔,比avi的格式簡單許多
	struct RIFFHEADER riffHeader;
	riffHeader.fcc = RIFF_fcc;			//riff
	riffHeader.dwType = WAVE_fcc;		//wav
	//寫入riff標頭檔
	fwrite(&riffHeader, sizeof(struct RIFFHEADER), 1, wavfp);
	
	aviData->waveFormatex.fcc = fmt_fcc;	//wav的id fmt
	aviData->waveFormatex.cb = sizeof(struct WAVEFORMATEX)-(sizeofFCC+sizeofCB);//檔案大小
	//先寫入
	fwrite(&aviData->waveFormatex, sizeof(struct WAVEFORMATEX), 1, wavfp);
	
	temp = data_fcc;//編碼方式
	fwrite(&temp, sizeof(DWORD), 1, wavfp);
	
	//存音訊檔大小
	fwrite(&audRawSize, sizeof(DWORD), 1, wavfp);	
	
	i=0;								
	while(i < aviData->numOfIndexItem){		
		if(aviData->indexEntry[i].dwChunkId == aviData->audsID){                      
			fseek(fp, aviData->posError+aviData->indexEntry[i].dwOffset+sizeofFCC, SEEK_SET); 
        	fread(&temp, sizeof(DWORD), 1, fp);
			buffer = (char*)calloc(temp, sizeof(char));
			fread(buffer, sizeof(char), temp, fp);
        	fwrite(buffer, sizeof(char), temp, wavfp);
        	free(buffer);
        	audRawSize += temp;			
		}
		i++;
	}
	
	riffHeader.cb = ftell(wavfp)-(sizeofFCC-sizeofCB);
	rewind(wavfp);
	fwrite(&riffHeader, sizeof(struct RIFFHEADER), 1, wavfp);
		
	fseek(wavfp, sizeof(struct WAVEFORMATEX)+sizeofFCC, SEEK_CUR);
	
	fwrite(&audRawSize, sizeof(DWORD), 1, wavfp);
	
	fclose(wavfp);
	printf("成功分離音訊檔！！！檔名是: audio.wav \n");
}
int matchStreamType(FOURCC sType)
{
	if(sType == vids_fcc) return 0;
	if(sType == auds_fcc) return 1;
	return -1;	
}
