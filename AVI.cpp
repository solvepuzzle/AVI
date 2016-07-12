//����avi�v��
#include"stdio.h"
#include"stdlib.h"

#include "read.h"
#include "type.h"

int main(int argc ,char* argv[])
{
	FILE *fp;
	struct AVIDATA* aviData;
	aviData = (struct AVIDATA*)calloc(1, sizeof(struct AVIDATA));
	
	printf("�п�Javi�ɮצW�� : ");
	scanf("%s", argv[0]);

	if((fp = fopen(argv[0], "rb+")) == NULL){
		printf("�}�� %s ����\n!", argv[0]);
		exit(EXIT_FAILURE);
	}
 
	readFileInfo(fp, aviData);//Ū���ɮ�
   
	if(aviData->aviMainHeader.dwStreams > 1){//�p�G���ɥ]�t�y���ӼƤj��1�N�}�l����
		extractAudio(fp, aviData);//���X���T��
		extractVideo(fp, aviData);//���X�v����
	}
	
	free(aviData->indexEntry);
	free(aviData);
	fclose(fp);
	return 0;
}

void readFileInfo(FILE *fp, struct AVIDATA* aviData)
{
	int i;
		
	//Ū���@�}�l���򥻸��
	fread(&aviData->riffHeader, sizeof(struct RIFFHEADER), 1, fp);			//RIFFHEADER
	fread(&aviData->list1, sizeof(struct LIST), 1, fp);						//LIST
	fread(&aviData->aviMainHeader, sizeof(struct AVIMAINHEADER), 1, fp);	//AVI Main Header
	
	for(i=0;i<aviData->aviMainHeader.dwStreams;i++){				
		fread(&aviData->subList[i], sizeof(struct LIST), 1, fp);			//sub LIST
		fread(&aviData->aviStreamHeader[i], sizeof(struct AVISTREAMHEADER), 1, fp);	//AVI Stream Header
		
		switch(matchStreamType(aviData->aviStreamHeader[i].fccType)){//�P�_�O�����ɮ�, 0 = vids, 1 = auds
			case 0:	/* vids */
				fread(&aviData->bitmapInfoHeader, sizeof(struct BITMAPINFOHEADER), 1, fp);//BitMap Header
				//����U�@��sub LIST
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
				//����U�@��sub LIST
				fseek(fp, sizeofFCC+sizeofCB+aviData->subList[i].cb-(sizeof(struct WAVEFORMATEX)+sizeof(struct LIST)+sizeof(struct AVISTREAMHEADER)), SEEK_CUR);				
				
				if(i == 0){
					aviData->audsID = x0wb_fcc;	/* auds */
				}else{
					aviData->audsID = x1wb_fcc;
				}
				
				break;
			default:
				printf("aviStreamHeader[%d] fccType %#x : ���~", i,aviData->aviStreamHeader[i].fccType);				
				break;				 		
		}	
	}
	
	fseek(fp, sizeof(struct RIFFHEADER)+sizeofFCC+sizeofCB+aviData->list1.cb, SEEK_SET);
   	
	//��L����T�����n ������movi
	fread(&aviData->list2, sizeof(struct LIST), 1, fp);	
	while(aviData->list2.subFcc != movi_fcc){
		//�]���n�P�_���A	�ҥH�hŪ�F4byte �n�b��^�h
		fseek(fp, -sizeofFCC+aviData->list2.cb, SEEK_CUR);
		fread(&aviData->list2, sizeof(struct LIST), 1, fp);
	} 
   
   aviData->posOfFirstDataBlock = ftell(fp);//�����ثe����m
   
	//�p�G�@��AVI�ɥ]�t�����޶��A�h���b�DAVI��T�Y���y�z���A�]�N�OAVIMAINHEADER���c��dwFlags���]�t�@��AVIF_HASINDEX�аO�C
	if((aviData->aviMainHeader.dwFlags & AVIF_HASINDEX)){	
		fseek(fp,-sizeofFCC+aviData->list2.cb, SEEK_CUR); //��hsizeofFCC(4byte)�N��n��movi��m
    	fread(&aviData->avioldIndex, sizeof(struct AVIOLDINDEX), 1, fp);    	
    	//�T�w�o�ǬO���n�o
		while(aviData->avioldIndex.fcc != idx1_fcc){
			fseek(fp, aviData->avioldIndex.cb, SEEK_CUR);
    		fread(&aviData->avioldIndex, sizeof(struct AVIOLDINDEX), 1, fp);
		}    	    	
		//��X���X��IndexItem
		aviData->numOfIndexItem = aviData->avioldIndex.cb/sizeof(struct AVIOLDINDEX_ENTRY);
		
    	aviData->indexEntry = (struct AVIOLDINDEX_ENTRY*)calloc(aviData->numOfIndexItem, sizeof(struct AVIOLDINDEX_ENTRY));
		//Ū�i�h
    	fread(aviData->indexEntry, sizeof(struct AVIOLDINDEX_ENTRY), aviData->numOfIndexItem, fp);    	
    	//�T�w���������~
		aviData->posError = aviData->posOfFirstDataBlock-aviData->indexEntry[0].dwOffset;		
	}else{ 
		printf("��p�I�o�ӵ{���L�䴩, �S��index��T���v���I");
		exit(EXIT_FAILURE);
	}
}

//���X�v��
void extractVideo(FILE *fp, struct AVIDATA* aviData)
{	
	int i = 0;
	int vidRawSize = 0;	
	int numVidIndex = 0;
	DWORD temp;
	char *buffer;
	
	//�إߤ@�ӷs��avi�v����,�u���v��
	FILE *avifp;
	if((avifp = fopen("video.avi", "wb+")) == NULL){
		printf("�гy�v�����ѡI\n");
		exit(EXIT_FAILURE);
	}

	printf("�v�����X��......\n");   
   
	//�Ӫ̮榡�s��
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
	}else{//�P�W
		aviData->subList[1].cb = sizeof(struct LIST)+sizeof(struct AVISTREAMHEADER)+sizeof(struct BITMAPINFOHEADER)-(sizeofFCC+sizeofCB);
		fwrite(&aviData->subList[0], sizeof(struct LIST), 1, avifp);
		fwrite(&aviData->aviStreamHeader[1], sizeof(struct AVISTREAMHEADER), 1, avifp);
		fwrite(&aviData->bitmapInfoHeader, sizeof(struct BITMAPINFOHEADER), 1, avifp);
	}	
	
	//JUNK�A�ΨӶ�R������ơA���ε{�����ө����o�Ǹ�ƶ�����ڷN�q�C
	fwrite(&aviData->list2 ,sizeof(struct LIST), 1, avifp);
	
	i=0;
	//���l��IndexItem���s�J�Ȧs����
	while(i < aviData->numOfIndexItem){							     
		//�p�G�Odb��dc�N�n�g�J
		if(aviData->indexEntry[i].dwChunkId == aviData->vidsIDb || aviData->indexEntry[i].dwChunkId == aviData->vidsIDc){                               		  		
    		fseek(fp, aviData->posError+aviData->indexEntry[i].dwOffset, SEEK_SET);//�������
    		aviData->indexEntry[i].dwOffset = ftell(avifp);	//�קﰾ���q
    		fread(&temp, sizeof(DWORD), 1, fp);				//Ū�ѧO�X
    		fwrite(&temp, sizeof(DWORD), 1, avifp);			//�g�ѧO�X
			
    		fread(&temp, sizeof(DWORD), 1, fp);				//Ū��ƶ����j�p
    		fwrite(&temp, sizeof(DWORD), 1, avifp);			//�g��ƶ����j�p
			
    		buffer = (char*)calloc(temp, sizeof(char));		//�ΨӼȦsvideo���
    		fread(buffer, sizeof(char), temp, fp);			//Ūvideo���
    		fwrite(buffer, sizeof(char), temp, avifp);		//�gvideo���
			
    		free(buffer);
    		
    		vidRawSize += (temp+sizeofFCC+sizeofCB);		//�W�[�j�p
    		numVidIndex++;									//�[�@��
		}
    	i++;
	}
   
	//�O�sindex�j�p
	struct AVIOLDINDEX avioldIndex;
	avioldIndex.fcc = idx1_fcc;
	avioldIndex.cb = sizeof(struct AVIOLDINDEX_ENTRY)*numVidIndex;
	fwrite(&avioldIndex, sizeof(struct AVIOLDINDEX), 1, avifp);
	
	//�gindex
	i=0;
	while(i < aviData->numOfIndexItem){
		if(aviData->indexEntry[i].dwChunkId == aviData->vidsIDb||aviData->indexEntry[i].dwChunkId == aviData->vidsIDc)
			fwrite(&aviData->indexEntry[i], sizeof(struct AVIOLDINDEX_ENTRY), 1, avifp);
		i++;
	}	
	
	//�O�s�ɮפj�p�`��
	riffHeader.cb = ftell(avifp)-(sizeofFCC+sizeofCB);
	rewind(avifp);
	fwrite(&riffHeader, sizeof(struct RIFFHEADER), 1, avifp);
	
	//�ק�list2 �]���o�ӷs�ɮפ��ϥ�JUNK
	fseek(avifp, sizeofFCC+sizeofCB+aviData->list1.cb, SEEK_CUR);	//�S��JUNK���j�p
	aviData->list2.cb = vidRawSize+sizeofFCC;						//�[�Wmovi���Y��
	fwrite(&aviData->list2, sizeof(struct LIST), 1, avifp);			//�g
	
	fclose(avifp);
	printf("���\�����v���I�I�I�ɦW�O: video.avi \n");
}
//���X���T
void extractAudio(FILE *fp, struct AVIDATA *aviData)
{	
	int i=0;
	DWORD temp;
	int audRawSize=0;
	char *buffer;
	
	printf("���T�ɤ�����......\n");
	
	//�إߤ@�ӷs��wav��,�u�����T
	FILE *wavfp;
	if((wavfp = fopen("audio.wav","wb+")) == NULL){
		printf("�гy���T�ɥ��ѡI\n");
		exit(EXIT_FAILURE);
	}
    
	//�]�O�Ӫ̮榡�s��,��avi���榡²��\�h
	struct RIFFHEADER riffHeader;
	riffHeader.fcc = RIFF_fcc;			//riff
	riffHeader.dwType = WAVE_fcc;		//wav
	//�g�Jriff���Y��
	fwrite(&riffHeader, sizeof(struct RIFFHEADER), 1, wavfp);
	
	aviData->waveFormatex.fcc = fmt_fcc;	//wav��id fmt
	aviData->waveFormatex.cb = sizeof(struct WAVEFORMATEX)-(sizeofFCC+sizeofCB);//�ɮפj�p
	//���g�J
	fwrite(&aviData->waveFormatex, sizeof(struct WAVEFORMATEX), 1, wavfp);
	
	temp = data_fcc;//�s�X�覡
	fwrite(&temp, sizeof(DWORD), 1, wavfp);
	
	//�s���T�ɤj�p
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
	printf("���\�������T�ɡI�I�I�ɦW�O: audio.wav \n");
}
int matchStreamType(FOURCC sType)
{
	if(sType == vids_fcc) return 0;
	if(sType == auds_fcc) return 1;
	return -1;	
}
