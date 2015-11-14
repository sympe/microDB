/*
* datamanip.c -- �f�[�^���샂�W���[��
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "microdb.h"

/*
* DATA_FILE_EXT -- �f�[�^�t�@�C���̊g���q
*/
#define DATA_FILE_EXT ".dat"

/*
* initializeDataManipModule -- �f�[�^���샂�W���[���̏�����
*/
Result initializeDataManipModule()
{
	return OK;
}

/*
* finalizeDataManipModule -- �f�[�^���샂�W���[���̏I������
*/
Result finalizeDataManipModule()
{
	return OK;
}

/*
* getRecordSize -- 1���R�[�h���̕ۑ��ɕK�v�ȃo�C�g���̌v�Z
*
* ����:
*	tableInfo: �f�[�^��`�������߂��\����
*
* �Ԃ�l:
*	tableInfo�̃e�[�u���Ɏ��߂�ꂽ1�̃��R�[�h��ۑ�����̂�
*	�K�v�ȃo�C�g��
*/
static int getRecordSize(TableInfo *tableInfo)
{
	int total = 0;
	int i;

	for (i = 0; i < (tableInfo->numField); i++) {
		///* i�Ԗڂ̃t�B�[���h��INT�^��STRING�^�����ׂ� */
		switch (tableInfo->fieldInfo[i].dataType) {
		/* INT�^�Ȃ�total��sizeof(int)�����Z */
		case TYPE_INTEGER:
			total += sizeof(int);
			break;
		/* STRING�^�Ȃ�total��MAX_STRING�����Z */
		case TYPE_STRING:
			total += MAX_STRING;
			break;
		/* ���̑��̏ꍇ(�����ɂ��邱�Ƃ͂Ȃ��͂�)*/
		default:
			break;
		}
	}

	/* �t���O�̕���1�𑫂� */
	total++;

	return total;
}

/*
* insertRecord -- ���R�[�h�̑}��
*
* ����:
*	tableName: ���R�[�h��}������e�[�u���̖��O
*	recordData: �}�����郌�R�[�h�̃f�[�^
*
* �Ԃ�l:
*	�}���ɐ���������OK�A���s������NG��Ԃ�
*/
Result insertRecord(char *tableName, RecordData *recordData)
{
	TableInfo *tableInfo;	/*TableInfo���i�[����|�C���^�ϐ�*/
	int recordSize;	/*���R�[�h�T�C�Y���i�[����ϐ�*/
	int numPage;	/*�y�[�W�T�C�Y���i�[����ϐ�*/
	int i,j;			/*�J��Ԃ��p�̕ϐ�*/
	char *record;	/*���R�[�h�����i�[����|�C���^�ϐ�*/
	char *p;	/*record�𑀍삷�邽�߂̃|�C���^�ϐ�*/
	char *q;	/*page�𑀍삷�邽�߂̃|�C���^�ϐ�*/
	char *filename;	/*�t�@�C���𑀍삷��Ƃ��̃t�@�C�������i�[����|�C���^�ϐ�*/
	int len;	/*������̒������i�[����|�C���^�ϐ�*/
	File *file;	/*�t�@�C�������i�[����|�C���^�ϐ�*/
	char page[PAGE_SIZE];	/*�t�@�C������̓ǂݏo���Ɏg���z��*/
	
	/* �e�[�u���̏����擾���� */
	if ((tableInfo = getTableInfo(tableName)) == NULL) {
		/* �G���[���� */
		return NG;
	}

	/* 1���R�[�h���̃f�[�^���t�@�C���Ɏ��߂�̂ɕK�v�ȃo�C�g�����v�Z���� */
	recordSize = getRecordSize(tableInfo);

	/* �K�v�ȃo�C�g�����̃��������m�ۂ��� */
	if ((record = malloc(recordSize)) == NULL) {
		/* �G���[���� */
		return NG;
	}
	p = record;

	/* �擪�ɁA�u�g�p���v���Ӗ�����t���O�𗧂Ă� */
	memset(p, 1, 1);
	p += 1;

	/* �m�ۂ����������̈�ɁA�t�B�[���h���������A�����f�[�^�𖄂ߍ��� */
	for (i = 0; i < tableInfo->numField; i++) {
		switch (tableInfo->fieldInfo[i].dataType) {
		case TYPE_INTEGER:
			memcpy(p, &(recordData->fieldData[i].intValue), sizeof(int));	/*int�^�f�[�^��������*/
			p += sizeof(int);												/*�|�C���^���炷*/
			break;
		case TYPE_STRING:
			memcpy(p, &(recordData->fieldData[i].stringValue),MAX_STRING);	/*string�^�f�[�^��������*/
			p += MAX_STRING;												/*�|�C���^���炷*/
			break;
		default:
			/* �����ɂ��邱�Ƃ͂Ȃ��͂� */
			freeTableInfo(tableInfo);
			free(record);
			return NG;
		}
	}

	/* �g�p�ς݂�tableInfo�f�[�^�̃�������������� */
	freeTableInfo(tableInfo);
	/*
	* �����܂łŁA�}�����郌�R�[�h�̏��𖄂ߍ��񂾃o�C�g��record���ł�������
	*/

	/* �f�[�^�t�@�C�����I�[�v������ *
	/*�t�@�C�����̍쐬*/
	len = strlen(tableName) + strlen(DATA_FILE_EXT) + 1;	/*������̒����𒲂ׂ�*/
	if ((filename = malloc(len)) == NULL){					/*malloc��len���̗̈���m��*/
		return NG;
	}
	snprintf(filename, len, "%s%s", tableName, DATA_FILE_EXT); /*snprintf�ɂ���ĕ������A��*/
	/*���O��[filename]�̃t�@�C�����I�[�v������*/
	if ((file = openFile(filename)) == NULL) {
		fprintf(stderr, "Cannot open file.\n");
		return NG;
	}

	/* �f�[�^�t�@�C���̃y�[�W���𒲂ׂ� */
	numPage = getNumPages(filename);

	/* ���R�[�h��}���ł���ꏊ��T�� */
	for (i = 0; i < numPage; i++) {
		/* 1�y�[�W���̃f�[�^��ǂݍ��� */
		if (readPage(file, i, page) != OK) {
			free(record);
			return NG;
		}

		/* page�̐擪����recordSize�o�C�g����тȂ���A�擪�̃t���O���u0�v(���g�p)�̏ꏊ��T�� */
		for (j = 0; j < (PAGE_SIZE / recordSize); j++) {
			q = &page[j*recordSize];
			if (*q == 0) {	/*����������*/
				/* �������󂫗̈�ɏ�ŗp�ӂ����o�C�g��record�𖄂ߍ��� */
				memcpy(q, record, recordSize);

				/* �t�@�C���ɏ����߂� */
				if (writePage(file, i, page) != OK) {
					free(record);
					return NG;
				}
				closeFile(file);
				free(record);
				return OK;
			}
		}
	}

	/*
	* �t�@�C���̍Ō�܂ŒT���Ă����g�p�̏ꏊ��������Ȃ�������
	* �t�@�C���̍Ō�ɐV������̃y�[�W��p�ӂ��A�����ɏ�������
	*/
	/*�y�[�W�̓��e���N���A����*/
	memset(page, 0, PAGE_SIZE);		/*�V�����y�[�W��0�Ŗ��߂�*/
	q = page;						
	memcpy(q, record, recordSize);		/*�y�[�W�̐擪�Ƀo�C�g��record�𖄂ߍ���*/
	
	/*�t�@�C���̍Ō�̃y�[�W��1�y�[�W���̃f�[�^����������*/
	if (writePage(file, numPage, page) != OK) {
		fprintf(stderr, "Cannot write page.\n");
		return NG;
	}

	/*�f�[�^�t�@�C�����N���[�Y����*/
	closeFile(file);
	free(record);
	return OK;
}



/*
* checkCondition -- ���R�[�h�������𖞑����邩�ǂ����̃`�F�b�N
*
* ����:
*	recordData: �`�F�b�N���郌�R�[�h
*	condition: �`�F�b�N�������
*
* �Ԃ�l:
*	���R�[�hrecord������condition�𖞑������OK�A�������Ȃ����NG��Ԃ�
*/
static Result checkCondition(RecordData *recordData, Condition *condition)
{
	int i;

	/* condition�Ɏw�肳�ꂽ�t�B�[���h�������邩�T��*/
	for (i = 0; i < recordData->numField; i++){
		/*�t�B�[���h�������邩�`�F�b�N*/
		if (strcmp(recordData->fieldData[i].name,condition->name)==0){
			/* �f�[�^�^�ɂ��ꍇ���� */
			switch (recordData->fieldData[i].dataType){
			case TYPE_INTEGER:
				/* ��r���Z�q�ɂ��ꍇ���� */
				switch (condition->operator)
				{
				case OPR_EQUAL:	
					if (condition->intValue == recordData->fieldData[i].intValue){
						return OK;
					}
					break;
				case OPR_NOT_EQUAL:
					if (condition->intValue != recordData->fieldData[i].intValue){
						return OK;
					}
					break;
				case OPR_GREATER_THAN:
					if (condition->intValue < recordData->fieldData[i].intValue){
						return OK;
					}
					break;
				case OPR_LESS_THAN:
					if (condition->intValue > recordData->fieldData[i].intValue){
						return OK;
					}
					break;
				case OPR_GREATER_THAN_OR_EQUAL:
					if (condition->intValue <= recordData->fieldData[i].intValue){
						return OK;
					}
					break;
				case OPR_LESS_THAN_OR_EQUAL:
					if (condition->intValue >= recordData->fieldData[i].intValue){
						return OK;
					}
					break;
				}
				break;
			case TYPE_STRING:
				/* ��r���Z�q�ɂ��ꍇ���� */
				switch (condition->operator)
				{
				case OPR_EQUAL:
					if (strcmp(recordData->fieldData[i].stringValue, condition->stringValue) == 0){
						return OK;
					}
					break;
				case OPR_NOT_EQUAL:
					if (strcmp(recordData->fieldData[i].stringValue,condition->stringValue)!=0){
						return OK;
					}
					break;
				default:
					printf("wrong condition select");
					break;
				}
			}
		}
	}
	return NG;
}

/*
* selectRecord -- ���R�[�h�̌���
*
* ����:
*	tableName: ���R�[�h����������e�[�u���̖��O
*	condition: �������郌�R�[�h�̏���
*
* �Ԃ�l:
*	�����ɐ��������猟�����ꂽ���R�[�h(�̏W��)�ւ̃|�C���^��Ԃ��A
*	�����Ɏ��s������NULL��Ԃ��B
*	�����������ʁA�Y�����郌�R�[�h��1���Ȃ������ꍇ���A���R�[�h��
*	�W���ւ̃|�C���^��Ԃ��B
*
* ***����***
*	���̊֐����Ԃ����R�[�h�̏W�������߂��������̈�́A�s�v�ɂȂ�����
*	�K��freeRecordSet�ŉ�����邱�ƁB
*/
RecordSet *selectRecord(char *tableName, Condition *condition)
{
	RecordSet *recordSet;	/*RecordSet�\���̗̂p��*/
	char *filename;	/*�t�@�C���𑀍삷��Ƃ��̃t�@�C�������i�[����|�C���^�ϐ�*/
	int len;	/*������̒������i�[����|�C���^�ϐ�*/
	File *file;	/*�t�@�C�������i�[����|�C���^�ϐ�*/
	int numPage;	/*�y�[�W�T�C�Y���i�[����ϐ�*/
	int i, j, k;			/*�J��Ԃ��p�̕ϐ�*/
	int fieldValueEqualFlag;	/*�d���������Ȃ��ꍇ��field�̒l�������ł��邩�𒲂ׂ�t���O*/
	char page[PAGE_SIZE];	/*�t�@�C������̓ǂݏo���Ɏg���z��*/
	TableInfo *tableInfo;	/*TableInfo���i�[����|�C���^�ϐ�*/
	int recordSize;	/*���R�[�h�T�C�Y���i�[����ϐ�*/
	RecordData *recordData;	/*���R�[�h�f�[�^���i�[����\����*/
	RecordData *p;	/*recordSet�𑖍����邽�߂̃|�C���^*/
	char *q;	/*page�𑀍삷�邽�߂̃|�C���^�ϐ�*/

	/* �e�[�u���̏����擾���� */
	if ((tableInfo = getTableInfo(tableName)) == NULL) {
		/* �G���[���� */
		return NULL;
	}

	/* 1���R�[�h���̃f�[�^���t�@�C���Ɏ��߂�̂ɕK�v�ȃo�C�g�����v�Z���� */
	recordSize = getRecordSize(tableInfo);


	/*RecordSet�\���̂̃��������m��*/
	if ((recordSet = (RecordSet*)malloc(sizeof(RecordSet))) == NULL){
		fprintf(stderr, "Malloc error.\n");
		return NULL;
	}
	recordSet->numRecord = 0;

	/*�t�@�C�����̍쐬*/
	len = strlen(tableName) + strlen(DATA_FILE_EXT) + 1;	/*������̒����𒲂ׂ�*/
	if ((filename = malloc(len)) == NULL){					/*malloc��len���̗̈���m��*/
		return NULL;
	}
	snprintf(filename, len, "%s%s", tableName, DATA_FILE_EXT); /*snprintf�ɂ���ĕ������A��*/
	/*���O��[filename]�̃f�[�^�t�@�C�����I�[�v������*/
	if ((file = openFile(filename)) == NULL) {
		fprintf(stderr, "Cannot open file.\n");
		return NULL;
	}

	/*�f�[�^�t�@�C���̃y�[�W�����擾*/
	numPage = getNumPages(filename);

	/*�f�[�^�t�@�C���̃y�[�W���������J��Ԃ�*/
	for (i = 0; i < numPage; i++){
		/* 1�y�[�W���̃f�[�^��ǂݍ��� */
		if (readPage(file, i, page) != OK) {
			return NULL;
		}
		/* page�̐擪����recordSize�o�C�g����тȂ���A�擪�̃t���O���u1�v(�g�p��)�̏ꏊ��T�� */
		for (j = 0; j < (PAGE_SIZE / recordSize); j++) {
			q = &page[j*recordSize];
			if (*q == 1) {	/*����������*/
				/* �擪�́A�u�g�p���v���Ӗ�����t���O�Ȃ̂łƂ΂� */
				q += 1;
				/*RecordData�\���̂̃��������m��*/
				if ((recordData = (RecordData *)malloc(sizeof(RecordData))) == NULL){
					fprintf(stderr, "Malloc error.\n");
					return NULL;
				}
				/* ���R�[�h�f�[�^�\���̂Ƀt�B�[���h���������A�����f�[�^�𖄂ߍ��� */
				recordData->numField = tableInfo->numField;
				recordData->next = NULL;	/*�����N��������*/
				for (k = 0; k < tableInfo->numField; k++) {
					strcpy(recordData->fieldData[k].name, tableInfo->fieldInfo[k].name);	/*name�̕ۑ�*/
					recordData->fieldData[k].dataType = tableInfo->fieldInfo[k].dataType;		/*dataType�̕ۑ�*/
					switch (tableInfo->fieldInfo[k].dataType) {
					case TYPE_INTEGER:
						memcpy(&recordData->fieldData[k].intValue,q, sizeof(recordData->fieldData[k].intValue));	/*int�^�f�[�^��������*/
						q += sizeof(recordData->fieldData[k].intValue);												/*�|�C���^���炷*/
						break;
					case TYPE_STRING:
						memcpy(&recordData->fieldData[k].stringValue,q, sizeof(recordData->fieldData[k].stringValue));	/*string�^�f�[�^��������*/
						q += sizeof(recordData->fieldData[k].stringValue);												/*�|�C���^���炷*/
						break;
					default:
						/* �����ɂ��邱�Ƃ͂Ȃ��͂� */
						free(recordData);
						return NULL;
					}
				}
				/*�d���������Ȃ��ꍇ*/
				if (condition->distinct == DISTINCT && recordSet->numRecord != 0){
					fieldValueEqualFlag = 0;
					for (p = recordSet->head; p != recordSet->tail->next; p = p->next){
						for (k = 0; k < p->numField; k++){
							switch (p->fieldData[k].dataType){
							case TYPE_INTEGER:
								if (p->fieldData[k].intValue == recordData->fieldData[k].intValue){
									fieldValueEqualFlag = 1;
								}
								else{
									fieldValueEqualFlag = 0;
								}
								break;
							case TYPE_STRING:
								if (strcmp(p->fieldData[k].stringValue, recordData->fieldData[k].stringValue) == 0){
									fieldValueEqualFlag = 1;
								}
								else{
									fieldValueEqualFlag = 0;
								}
								break;
							default:
								/* �����ɂ��邱�Ƃ͂Ȃ��͂� */
								free(recordData);
								return NULL;
							}
							if (fieldValueEqualFlag == 0){
								break;
							}
						}
						if (fieldValueEqualFlag == 1){
							break;
						}
					}
					if (fieldValueEqualFlag == 1){
						continue;
					}
				}
				/*�����𖞑����邩����ׂ�*/
				if (checkCondition(recordData, condition) == OK){		/*recordData�����������ɓ��Ă͂܂����Ƃ�*/
					if (recordSet->numRecord==0){						/*recordSet����̂Ƃ�*/
						recordSet->head = recordSet->tail = recordData;	/*head��recordData���w���悤�ɂ���*/
					}
					else{												/*recordSet���󂶂�Ȃ��Ƃ�*/
						recordSet->tail->next =recordData;				/*tail��next��recordData���w���悤�ɂ���*/
						recordSet->tail = recordData;					/*tail��recordData���w���悤�ɂ���*/
					}
					recordSet->numRecord++;
				}
			}
		}

	}

	/*�f�[�^�t�@�C�����N���[�Y����*/
	freeTableInfo(tableInfo);
	closeFile(file);
	return recordSet;
}

/*
* freeRecordSet -- ���R�[�h�W���̏������߂��������̈�̉��
*
* ����:
*	recordSet: ������郁�����̈�
*
* �Ԃ�l:
*	�Ȃ�
*
* ***����***
*	�֐�selectRecord���Ԃ����R�[�h�̏W�������߂��������̈�́A
*	�s�v�ɂȂ�����K�����̊֐��ŉ�����邱�ƁB
*/
void freeRecordSet(RecordSet *recordSet)
{
	free(recordSet);
}

/*
* deleteRecord -- ���R�[�h�̍폜
*
* ����:
*	tableName: ���R�[�h���폜����e�[�u���̖��O
*	condition: �폜���郌�R�[�h�̏���
*
* �Ԃ�l:
*	�폜�ɐ���������OK�A���s������NG��Ԃ�
*/
Result deleteRecord(char *tableName, Condition *condition)
{
	char *filename;	/*�t�@�C���𑀍삷��Ƃ��̃t�@�C�������i�[����|�C���^�ϐ�*/
	int len;	/*������̒������i�[����|�C���^�ϐ�*/
	File *file;	/*�t�@�C�������i�[����|�C���^�ϐ�*/
	int numPage;	/*�y�[�W�T�C�Y���i�[����ϐ�*/
	int i, j, k;			/*�J��Ԃ��p�̕ϐ�*/
	char page[PAGE_SIZE];	/*�t�@�C������̓ǂݏo���Ɏg���z��*/
	TableInfo *tableInfo;	/*TableInfo���i�[����|�C���^�ϐ�*/
	int recordSize;	/*���R�[�h�T�C�Y���i�[����ϐ�*/
	RecordData *recordData;	/*���R�[�h�f�[�^���i�[����\����*/
	char *q;	/*page�𑀍삷�邽�߂̃|�C���^�ϐ�*/

	/* �e�[�u���̏����擾���� */
	if ((tableInfo = getTableInfo(tableName)) == NULL) {
		/* �G���[���� */
		return NG;
	}

	/* 1���R�[�h���̃f�[�^���t�@�C���Ɏ��߂�̂ɕK�v�ȃo�C�g�����v�Z���� */
	recordSize = getRecordSize(tableInfo);

	/*RecordDara�\���̂̃��������m��*/
	if ((recordData = (RecordData *)malloc(sizeof(RecordData))) == NULL){
		fprintf(stderr, "Malloc error.\n");
		return NG;
	}

	/*�t�@�C�����̍쐬*/
	len = strlen(tableName) + strlen(DATA_FILE_EXT) + 1;	/*������̒����𒲂ׂ�*/
	if ((filename = malloc(len)) == NULL){					/*malloc��len���̗̈���m��*/
		return NG;
	}
	snprintf(filename, len, "%s%s", tableName, DATA_FILE_EXT); /*snprintf�ɂ���ĕ������A��*/
	/*���O��[filename]�̃f�[�^�t�@�C�����I�[�v������*/
	if ((file = openFile(filename)) == NULL) {
		fprintf(stderr, "Cannot open file.\n");
		return NG;
	}

	/*�f�[�^�t�@�C���̃y�[�W�����擾*/
	numPage = getNumPages(filename);

	/*�f�[�^�t�@�C���̃y�[�W���������J��Ԃ�*/
	for (i = 0; i < numPage; i++){
		/* 1�y�[�W���̃f�[�^��ǂݍ��� */
		if (readPage(file, i, page) != OK) {
			return NG;
		}
		/* page�̐擪����recordSize�o�C�g����тȂ���A�擪�̃t���O���u1�v(�g�p��)�̏ꏊ��T�� */
		for (j = 0; j < (PAGE_SIZE / recordSize); j++) {
			q = &page[j*recordSize];
			if (*q == 1) {	/*����������*/
				/* �擪�́A�u�g�p���v���Ӗ�����t���O�Ȃ̂łƂ΂� */
				q += 1;

				/* ���R�[�h�f�[�^�\���̂Ƀt�B�[���h���������A�����f�[�^�𖄂ߍ��� */
				recordData->numField = tableInfo->numField;
				for (k = 0; k < tableInfo->numField; k++) {
					strcpy(recordData->fieldData[k].name, tableInfo->fieldInfo[k].name);
					recordData->fieldData[k].dataType=tableInfo->fieldInfo[k].dataType;
					switch (tableInfo->fieldInfo[k].dataType) {
					case TYPE_INTEGER:
						memcpy(&recordData->fieldData[k].intValue, q, sizeof(int));	/*int�^�f�[�^��������*/
						q += sizeof(int);												/*�|�C���^���炷*/
						break;
					case TYPE_STRING:
						memcpy(&recordData->fieldData[k].stringValue, q, MAX_STRING);	/*string�^�f�[�^��������*/
						q += MAX_STRING;												/*�|�C���^���炷*/
						break;
					default:
						/* �����ɂ��邱�Ƃ͂Ȃ��͂� */
						free(recordData);
						return NG;
					}
				}
				/*�����𖞑����邩����ׂ�*/
				if (checkCondition(recordData, condition) == OK){	/*recordData�����������ɓ��Ă͂܂����Ƃ�*/
					/*���R�[�h���폜����*/
					page[j*recordSize] = 0;
				}
			}
			/*�폜���ĕύX���ꂽ�y�[�W�̓��e������������*/
			if (writePage(file, i, page) != OK){
				fprintf(stderr, "Cannot write page.\n");
				return NG;
			}
		}

	}

	/*�f�[�^�t�@�C�����N���[�Y����*/
	free(recordData);
	freeTableInfo(tableInfo);
	closeFile(file);
	return OK;
}

/*
* createDataFile -- �f�[�^�t�@�C���̍쐬
*
* ����:
*	tableName: �쐬����e�[�u���̖��O
*
* �Ԃ�l:
*	�쐬�ɐ���������OK�A���s������NG��Ԃ�
*/
Result createDataFile(char *tableName)
{
	char *filename;	/*�t�@�C���𑀍삷��Ƃ��̃t�@�C�������i�[����|�C���^�ϐ�*/
	int len;	/*������̒������i�[����|�C���^�ϐ�*/
	File *file;
	char page[PAGE_SIZE];
	char *p;

	/*�y�[�W�̓��e���N���A����*/
	memset(page, 0, PAGE_SIZE);
	p = page;
	
	/*�t�@�C�����̍쐬*/
	len = strlen(tableName) + strlen(DATA_FILE_EXT) + 1;
	if ((filename = malloc(len)) == NULL){
		return NG;
	}
	snprintf(filename, len, "%s%s", tableName, DATA_FILE_EXT);

	/*�t�@�C���̍쐬*/
	if (createFile(filename) != OK) {
		fprintf(stderr, "Cannot create file.\n");
		return NG;
	}

	/* �t�@�C�����I�[�v������ */
	if ((file = openFile(filename)) == NULL) {
		fprintf(stderr, "Cannot open file.\n");
		return NG;
	}

	/*�t�@�C���̐擪�y�[�W��1�y�[�W���̃f�[�^����������*/
	if (writePage(file, getNumPages(filename), page) != OK) {
		fprintf(stderr, "Cannot write page.\n");
		return NG;
	}

	/*�t�@�C�����N���[�Y����*/
	if (closeFile(file) != OK){
		fprintf(stderr, "Cannot close file.\n");
		return NG;
	}

	return OK;
}

/*
* deleteDataFile -- �f�[�^�t�@�C���̍폜
*
* ����:
*	tableName: �폜����e�[�u���̖��O
*
* �Ԃ�l:
*	�폜�ɐ���������OK�A���s������NG��Ԃ�
*/
Result deleteDataFile(char *tableName)
{
	/*�t�@�C�����̍쐬*/
	char *filename;
	int len;
	len = strlen(tableName) + strlen(DATA_FILE_EXT) + 1;
	if ((filename = malloc(len)) == NULL){
		return NG;
	}
	snprintf(filename, len, "%s%s", tableName, DATA_FILE_EXT);

	/*�e�[�u���̍폜*/
	if (deleteFile(filename) == NG) {
		fprintf(stderr, "Cannot delete file.\n");
		return NG;
	}
	return OK;
}

/*
* printTableData -- �S�Ẵf�[�^�̕\��
*
* ����:
*   tableName: �f�[�^��\������e�[�u���̖��O
*/
void printTableData(char *tableName, int numField)
{
	TableInfo *tableInfo;
	File *file;
	int len;
	int i, j, k;
	int recordSize;
	int numPage;
	char *filename;
	char page[PAGE_SIZE];
	char *p;
	int intValue;
	char stringValue[MAX_STRING];
	char plus = '+';
	char *hyphen = "----------------";
	char vertical = '|';

	/* �e�[�u���̃f�[�^��`�����擾 */
	if ((tableInfo = getTableInfo(tableName))==NULL){
		return;
	}

	/*1���R�[�h���̃o�C�g�����擾*/
	recordSize = getRecordSize(tableInfo);

	/*�������̈�̊m��*/
	len = strlen(tableName) + strlen(DATA_FILE_EXT) + 1;
	if ((filename = malloc(len)) == NULL){
		freeTableInfo(tableInfo);
		return;
	}

	/*�t�@�C�����쐬*/
	snprintf(filename, len, "%s%s", tableName, DATA_FILE_EXT);

	/*�f�[�^�t�@�C���̃y�[�W���擾*/
	numPage = getNumPages(filename);

	/*�f�[�^�t�@�C�����I�[�v������*/
	if ((file = openFile(filename))==NULL){
		free(filename);
		freeTableInfo(tableInfo);
		return;
	}

	free(filename);
	
	/*�ォ�珇�Ԃɕ`�悵�Ă���*/		/* +----------+---------*/
	for (i = 0; i < tableInfo->numField; i++){
		printf("%c%s", plus, hyphen);
	}
	printf("%c\n",plus);

	/*�t�B�[���h����\��*/				/*|name		|name		*/
	for (i = 0; i < tableInfo->numField; i++){
		printf("%c %-14s ", vertical, tableInfo->fieldInfo[i].name);
	}
	printf("%c\n", vertical);
	
	/*�͂�*/		/* +----------+---------*/
	for (i = 0; i < tableInfo->numField; i++){
		printf("%c%s", plus, hyphen);
	}
	printf("%c\n", plus);

	/* ���R�[�h��1�����o���A�\������*/
	for (i = 0; i < numPage; i++){
		readPage(file, i, page);

		for (j = 0; j < (PAGE_SIZE / recordSize); j++){
			/*�擪�̃t���O��0�̏ꍇ�͓ǂݔ�΂�*/
			p = &page[recordSize * j];
			if (*p == 0){
				continue;
			}
			/*1�����ڂ̓t���O�Ȃ̂œǂݔ�΂�*/
			p++;

			/*1���R�[�h����\��*/
			for (k = 0; k < tableInfo->numField; k++){
				switch (tableInfo->fieldInfo[k].dataType){
				case TYPE_INTEGER:
					memcpy(&intValue, p, sizeof(int));
					p += sizeof(int);
					printf("%c %-14d ", vertical,intValue);
					break;
				case TYPE_STRING:
					memcpy(stringValue, p, MAX_STRING);
					p += MAX_STRING;
					printf("%c %-14s ", vertical,stringValue);
					break;
				default:
					return;
				}
			}
			printf("%c\n", vertical);
		}
	}
	/*�͂�*/		/* +----------+---------*/
	for (i = 0; i < tableInfo->numField; i++){
		printf("%c%s", plus, hyphen);
	}
	printf("%c\n", plus);

	freeTableInfo(tableInfo);
}


/*
* printRecordSet -- ���R�[�h�W���̕\��
*
* ����:
*	recordSet: �\�����郌�R�[�h�W��
*
* �Ԃ�l:
*	�Ȃ�
*/
void printRecordSet(RecordSet *recordSet,char receiveFieldName[MAX_FIELD][MAX_FIELD_NAME],int numField)
{
	int i,j;
	RecordData *p;	/*���X�g�ɃA�N�Z�X���邽�߂̃|�C���^*/
	char plus = '+';
	char *hyphen = "----------------";
	char vertical = '|';
	printf("Number of Records: %d\n", recordSet->numRecord);
	
	/*�ォ�珇�Ԃɕ`�悵�Ă���*/		/* +----------+---------*/
	printf("%c", plus);
	for (i = 0; i < numField; i++){
			printf("%s%c", hyphen, plus);
	}
	printf("\n");
	
	/*�t�B�[���h����\��*/				/*|name		|name		*/	
	for (i = 0; i < numField; i++){
		printf("%c %-14s ", vertical, receiveFieldName[i]);
	}
	printf("%c\n",vertical);
	
	printf("%c", plus);					/* +-----------+---------*/
	for (i = 0; i < numField; i++){
			printf("%s%c", hyphen, plus);
	}
	printf("\n");

	for (p = recordSet->head; p!=NULL; p=p->next){
		for (i = 0; i < numField; i++){
			for (j = 0; j < p->numField;j++){
				if (strcmp(p->fieldData[j].name,receiveFieldName[i]) == 0){
					switch (p->fieldData[j].dataType){
					case TYPE_INTEGER:
						printf("%c %-14d ", vertical, p->fieldData[j].intValue);
						break;
					case TYPE_STRING:
						printf("%c %-14s ", vertical, p->fieldData[j].stringValue);
						break;
					default:
						return;
					}
				}
			}
		}
		printf("%c\n", vertical);
	}

	printf("%c", plus);					/* +-----------+---------*/
	for (i = 0; i < numField; i++){
			printf("%s%c", hyphen, plus);
	}
	printf("\n");
}