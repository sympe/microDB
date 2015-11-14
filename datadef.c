/*
* datadef.c - �f�[�^��`���W���[��
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "microdb.h"

/*
* DEF_FILE_EXT -- �f�[�^��`�t�@�C���̊g���q
*/
#define DEF_FILE_EXT ".def"

/*�f�[�^��`���W���[���̏�����*/
Result initializeDataDefModule()
{
	return OK;	//�����̏ꍇOK��Ԃ�
}

/*�f�[�^��`���W���[���̏I������*/
Result finalizeDataDefModule()
{
	return OK;	//�����̏ꍇOK��Ԃ�
}

/* �\(�e�[�u��)�̍쐬
*
* ����:
*	tableName: �쐬����\�̖��O
*	tableInfo: �f�[�^��`���
*
* �Ԃ�l:
*	�����Ȃ�OK�A���s�Ȃ�NG��Ԃ�
*
* �f�[�^��`�t�@�C���̍\��(�t�@�C����: tableName.def)
*   +-------------------+----------------------+-------------------+----
*   |�t�B�[���h��       |�t�B�[���h��          |�f�[�^�^           |
*   |(sizeof(int)�o�C�g)|(MAX_FIELD_NAME�o�C�g)|(sizeof(int)�o�C�g)|
*   +-------------------+----------------------+-------------------+----
* �ȍ~�A�t�B�[���h���ƃf�[�^�^�����݂ɑ����B
*/
Result createTable(char *tableName, TableInfo *tableInfo)
{
	File *file;
	char page[PAGE_SIZE];
	char *p;
	int len;
	char *filename;

	/*�y�[�W�̓��e���N���A����*/
	memset(page, 0, PAGE_SIZE);
	p = page;

	/* �y�[�W�̐擪�Ƀt�B�[���h����ۑ�����*/
	memcpy(p, &(tableInfo->numField), sizeof(tableInfo->numField));	//�t�B�[���h����������
	p += sizeof(tableInfo->numField);				//�|�C���^�����炷

	/*�t�B�[���h���̕������A�z��page�Ƀt�B�[���h����f�[�^�^���R�s�[����*/
	for (int i = 0; i < tableInfo->numField; i++){
		memcpy(p, &(tableInfo->fieldInfo[i].name), sizeof(tableInfo->fieldInfo[i].name));	//�t�B�[���h����������
		p += sizeof(tableInfo->fieldInfo[i].name);		//�|�C���^�����炷
		memcpy(p, &(tableInfo->fieldInfo[i].dataType), sizeof(tableInfo->fieldInfo[i].dataType));	//�f�[�^�^��������
		p += sizeof(tableInfo->fieldInfo[i].dataType);		//�|�C���^�����炷
	}

	
	/*�f�[�^��`�����i�[����t�@�C���̍쐬*/
	/*�t�@�C�����̍쐬*/
	len = strlen(tableName) + strlen(DEF_FILE_EXT) + 1;
	if ((filename = malloc(len)) == NULL){
		return NG;
	}
	snprintf(filename, len, "%s%s", tableName, DEF_FILE_EXT);

	/*�t�@�C���쐬*/
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
	/*�������܂ł��f�[�^��`�t�@�C���̍쐬*/

	/*����������f�[�^�t�@�C���̍쐬*/
	if (createDataFile(tableName) != OK){
		fprintf(stderr, "Cannot create file.\n");
		return NG;
	}

	return OK;
}

/*
* dropTable -- �\(�e�[�u��)�̍폜
*
*/
Result dropTable(char *tableName)
{
	/*�t�@�C�����̍쐬*/
	char *filename;
	int len;
	len = strlen(tableName) + strlen(DEF_FILE_EXT) + 1;
	if ((filename = malloc(len)) == NULL){
		return NG;
	}
	snprintf(filename, len, "%s%s", tableName, DEF_FILE_EXT);
	
	/*�e�[�u���̍폜*/
	if (deleteFile(filename) == NG) {
		fprintf(stderr, "Cannot delete file.\n");
		return NG;
	}

	/*����������f�[�^�t�@�C���̍폜*/
	if (deleteDataFile(tableName) != OK){
		fprintf(stderr, "Cannot delete file.\n");
		return NG;
	}
	return OK;
}

/*
* getTableInfo -- �\�̃f�[�^��`�����擾����֐�
*
* ����:
*	tableName: ����\������\�̖��O
*
* �Ԃ�l:
*	tableName�̃f�[�^��`����Ԃ�
*	�G���[�̏ꍇ�ɂ́ANULL��Ԃ�
*
* ***����***
*	���̊֐����Ԃ��f�[�^��`�������߂��������̈�́A�s�v�ɂȂ�����
*	�K��freeTableInfo�ŉ�����邱�ƁB
*/
TableInfo *getTableInfo(char *tableName)
{
	File *file;
	TableInfo *tableInfo;
	char page[PAGE_SIZE];	//�t�@�C������̓ǂݏo���Ɏg���z��
	char *p;
	p = page;
	char *filename;
	int len;

	/*�t�@�C�����̍쐬*/
	len = strlen(tableName) + strlen(DEF_FILE_EXT) + 1;
	if ((filename = malloc(len)) == NULL){
		return NULL;
	}
	snprintf(filename, len, "%s%s", tableName, DEF_FILE_EXT);
	
	/*TableInfo�\���̗̂p��*/
	tableInfo = malloc(sizeof(TableInfo));
	if (tableInfo == NULL){
		return NULL;
	}

	/* �t�@�C�����I�[�v������ */
	if ((file = openFile(filename)) == NULL) {
		fprintf(stderr, "Cannot open file.\n");
		return NULL;
	}

	/* �t�@�C����0�y�[�W�ڂ���l��ǂݏo�� */
	if (readPage(file, 0, page) != OK) {
		fprintf(stderr, "Cannot read page.\n");
		return NULL;
	}

	/*TableInfo�\���̂ɒ�`����ۑ�*/
	memcpy(&tableInfo->numField, p, sizeof(tableInfo->numField));	//�t�B�[���h����������
	p += sizeof(tableInfo->numField);

	/*�t�B�[���h���̕������A�z��page�Ƀt�B�[���h����f�[�^�^���R�s�[����*/
	for (int i = 0; i < tableInfo->numField; i++){
		memcpy(&(tableInfo->fieldInfo[i].name), p, sizeof(tableInfo->fieldInfo[i].name));	//�t�B�[���h��
		p += sizeof(tableInfo->fieldInfo[i].name);
		memcpy(&(tableInfo->fieldInfo[i].dataType), p, sizeof(tableInfo->fieldInfo[i].dataType)); //�t�B�[���h�̃f�[�^�^
		p += sizeof(tableInfo->fieldInfo[i].dataType);
	}

	/*�t�@�C�����N���[�Y����*/
	if (closeFile(file) != OK){
		fprintf(stderr, "Cannot close file.\n");
		return NULL;
	}
	return tableInfo;
}

/*
* freeTableInfo -- �f�[�^��`�������߂��������̈�̉��
*
*
* ***����***
*	�֐�getTableInfo���Ԃ��f�[�^��`�������߂��������̈�́A
*	�s�v�ɂȂ�����K�����̊֐��ŉ�����邱�ƁB
*/
void freeTableInfo(TableInfo *tableInfo)
{
	free(tableInfo);
}

/*
* printTableInfo -- �e�[�u���̃f�[�^��`����\������(����m�F�p)
*
* ����:
*	tableName: ����\������e�[�u���̖��O
*
* �Ԃ�l:
*	�Ȃ�
*/
void printTableInfo(char *tableName)
{
	TableInfo *tableInfo;
	int i;

	/* �e�[�u�������o�� */
	printf("\nTable %s\n", tableName);

	/* �e�[�u���̒�`�����擾���� */
	if ((tableInfo = getTableInfo(tableName)) == NULL) {
		/* �e�[�u�����̎擾�Ɏ��s�����̂ŁA��������߂ĕԂ� */
		return;
	}

	/* �t�B�[���h�����o�� */
	printf("number of fields = %d\n", tableInfo->numField);

	/* �t�B�[���h����ǂݎ���ďo�� */
	for (i = 0; i < tableInfo->numField; i++) {
		/* �t�B�[���h���̏o�� */
		printf("  field %d: name = %s, ", i + 1, tableInfo->fieldInfo[i].name);

		/* �f�[�^�^�̏o�� */
		printf("data type = ");
		switch (tableInfo->fieldInfo[i].dataType) {
		case TYPE_INTEGER:
			printf("integer\n");
			break;
		case TYPE_STRING:
			printf("string\n");
			break;
		default:
			printf("unknown\n");
		}
	}

	/* �f�[�^��`����������� */
	freeTableInfo(tableInfo);

	return;
}