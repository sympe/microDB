/*
* main.c -- ���C�����W���[��
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>
#include <string.h>
#include "microdb.h"

/*
* MAX_INPUT -- ���͍s�̍ő啶����
*/
#define MAX_INPUT 256

/*
* TEST_SIZE -- �}�����郌�R�[�h��
*/
#define TEST_SIZE 1000

#define MY_MIN 1
#define MY_MAX 5

/*
* inputString -- �����͒��̕���������߂�z��
*
* �傫�����u3 * MAX_INPUT�v�Ƃ���̂́A���͍s�̂��ׂĂ̕�������؂�L���ł�
* �o�b�t�@�I�[�o�[�t���[���N�����Ȃ��悤�ɂ��邽�߁B
*/
static char inputString[3 * MAX_INPUT];

/*
* nextPosition -- �����͒��̏ꏊ
*/
static char *nextPosition;

/*
* setInputString -- �����͂��镶����̐ݒ�
*
* ����:
*	string: ��͂��镶����
*
* �Ԃ�l:
*	�Ȃ�
*/
static void setInputString(char *string)
{
	char *p = string;
	char *q = inputString;

	/* ���͍s���R�s�[���ĕۑ����� */
	while (*p != '\0') {
		/* ���p���̏ꍇ�ɂ́A���̈��p���܂œǂݔ�΂� */
		if (*p == '\'' || *p == '"') {
			char *quote;
			quote = p;
			*q++ = *p++;

			/* ������p��(�܂��͕�����̍Ō�)�܂ŃR�s�[���� */
			while (*p != *quote && *p != '\0') {
				*q++ = *p++;
			}

			/* ������p�����̂��R�s�[���� */
			if (*p == *quote) {
				*q++ = *p++;
			}
			continue;
		}

		/* ��؂�L���̏ꍇ�ɂ́A���̑O��ɋ󔒕��������� */
		if (*p == ',' || *p == '(' || *p == ')' || *p == '.') {
			*q++ = ' ';
			*q++ = *p++;
			*q++ = ' ';
		}
		else {
			*q++ = *p++;
		}
	}

	*q = '\0';

	/* getNextToken()�̓ǂݏo���J�n�ʒu�𕶎���̐擪�ɐݒ肷�� */
	nextPosition = inputString;
}

/*
* getNextToken -- setInputString�Őݒ肵�������񂩂�̎���̎��o��
*
* ����:
*	�Ȃ�
*
* �Ԃ�l:
*	���̎���̐擪�Ԓn��Ԃ��B�Ō�܂ł��ǂ蒅������NULL��Ԃ��B
*/
static char *getNextToken()
{
	char *start;
	char *end;
	char *p;

	/* �󔒕��������������Ă�����A���̕�nextPosition���ړ������� */
	while (*nextPosition == ' ') {
		nextPosition++;
	}
	start = nextPosition;

	/* nextPosition�̈ʒu��������̍Ō�('\0')��������A��͏I�� */
	if (*nextPosition == '\0') {
		return NULL;
	}

	/* nextPosition�̈ʒu�ȍ~�ŁA�ŏ��Ɍ�����󔒕����̏ꏊ��T�� */
	p = nextPosition;
	while (*p != ' ' && *p != '\0') {
		/* ���p���̏ꍇ�ɂ́A���̈��p���܂œǂݔ�΂� */
		if (*p == '\'' || *p == '"') {
			char *quote;
			quote = p;
			p++;

			/* ������p��(�܂��͕�����̍Ō�)�܂œǂݔ�΂� */
			while (*p != *quote && *p != '\0') {
				p++;
			}

			/* ������p�����̂��ǂݔ�΂� */
			if (*p == *quote) {
				p++;
			}
		}
		else {
			p++;
		}

	}
	end = p;

	/*
	* �󔒕������I�[�����Œu��������ƂƂ��ɁA
	* �����getNextToken()�̌Ăяo���̂��߂�
	* nextPosition�����̎��̕����Ɉړ�����
	*/
	if (*end != '\0') {
		*end = '\0';
		nextPosition = end + 1;
	}
	else {
		/*
		* (*end == '\0')�̏ꍇ�͕�����̍Ō�܂ŉ�͂��I����Ă���̂ŁA
		* �����getNextToken()�̌Ăяo���̂Ƃ���NULL���Ԃ�悤�ɁA
		* nextPosition�𕶎���̈�ԍŌ�Ɉړ�������
		*/
		nextPosition = end;
	}

	/* ����̐擪�Ԓn��Ԃ� */
	return start;
}

/*
*condtionSet -- select����delete����condition�����肷��
*
* ����:token
* :condition
*/
static void conditionSet(char *token, Condition *condition){

	char *p;	/*string��''�ň͂܂�Ă��邩�𒲂ׂ�̂ɗp����*/
	char *q;	/*''����菜����������������*/
	char *quote;	/*�V���O���R�[�e�[�V����*/

	/*�t�B�[���h�̃f�[�^�^���T���ł��Ȃ��Ƃ��́A���@�G���[*/
	if (condition->dataType == TYPE_UNKNOWN){
		printf("�w�肵���t�B�[���h�͂���܂���%s\n", condition->name);
		return;
	}

	/*���̃g�[�N���ł����r���Z�q��ǂݍ���*/
	if ((token = getNextToken()) == NULL){
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B:%s\n", token);
		return;
	}

	/*��r���Z�q��condition�ɐݒ�*/
	if (strcmp(token, "=") == 0){
		condition->operator=OPR_EQUAL;
	}
	else if (strcmp(token, "!=") == 0){
		condition->operator=OPR_NOT_EQUAL;
	}
	else if (strcmp(token, ">") == 0){
		condition->operator=OPR_GREATER_THAN;
	}
	else if (strcmp(token, "<") == 0){
		condition->operator=OPR_LESS_THAN;
	}
	else if (strcmp(token, ">=") == 0){
		condition->operator=OPR_GREATER_THAN_OR_EQUAL;
	}
	else if (strcmp(token, "<=") == 0){
		condition->operator=OPR_LESS_THAN_OR_EQUAL;
	}
	else {
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B:%s\n", token);
		return;
	}

	/*���̃g�[�N���ł����������ǂݍ���*/
	if ((token = getNextToken()) == NULL){
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B:%s\n", token);
		return;
	}

	/*�������̒l��condition�ɐݒ�*/
	switch (condition->dataType){
	case TYPE_INTEGER:
		/*�g�[�N���̕�����𐮐��ɕϊ����ē�����I*/
		condition->intValue = atoi(token);
		break;
	case TYPE_STRING:
		/*'�ň͂܂�Ă��邩�𒲂ׂ�*/
		p = token;
		/*����'���ǂ������ׂ�*/
		if (*p == '\'') {
			quote = p;
			p++;
			q = p;	/*'���Ƃ΂���q�ɒl���i�[���Ă���*/
			/*	������̍Ō�܂œǂݍ��� */
			while (*p != *quote && *p != '\0') {
				p++;
			}

			/* �I��肪'����Ȃ������Ƃ��� */
			if (*p != '\'') {
				printf("�������̎w��ɊԈႢ������܂�%s\n", token);
				return;
			}
		}
		else{
			printf("�������̎w��ɊԈႢ������܂�%s\n", token);
			return;
		}
		*p = '\0';
		strcpy(condition->stringValue, q);
		break;
	default:
		printf("�f�[�^�^�C�v���K�؂ł͂���܂���ł���\n");
		break;
	}
}
/*
* callCreateTable -- create���̍\����͂�createTable�̌Ăяo��
*
* ����:
*	�Ȃ�
*
* �Ԃ�l:
*	�Ȃ�
*
* create table�̏���:
*	create table �e�[�u���� ( �t�B�[���h�� �f�[�^�^, ... )
*/
void callCreateTable()
{
	char *token;
	char *tableName;
	int numField;
	TableInfo tableInfo;

	/* create�̎��̃g�[�N����ǂݍ��݁A���ꂪ"table"���ǂ������`�F�b�N */
	token = getNextToken();
	if (token == NULL || strcmp(token, "table") != 0) {
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B:%s\n", token);
		return;
	}

	/* �e�[�u������ǂݍ��� */
	if ((tableName = getNextToken()) == NULL) {
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B:%s\n", tableName);
		return;
	}

	/* ���̃g�[�N����ǂݍ��݁A���ꂪ"("���ǂ������`�F�b�N */
	token = getNextToken();
	if (token == NULL || strcmp(token, "(") != 0) {
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B%s\n", token);
		return;
	}

	/*
	* ��������A�t�B�[���h���ƃf�[�^�^�̑g���J��Ԃ��ǂݍ��݁A
	* �z��fieldInfo�ɓ���Ă����B
	*/
	numField = 0;
	for (;;) {
		/* �t�B�[���h���̓ǂݍ��� */
		if ((token = getNextToken()) == NULL) {
			/* ���@�G���[ */
			printf("���͍s�ɊԈႢ������܂��B%s\n", token);
			return;
		}

		/* �ǂݍ��񂾃g�[�N����")"��������A���[�v���甲���� */
		if (strcmp(token, ")") == 0) {
			break;
		}

		/* �t�B�[���h����z��ɐݒ� */
		strcpy(tableInfo.fieldInfo[numField].name, token);

		/* �f�[�^�^�̓ǂݍ��� */
		token = getNextToken();
		if (token == NULL){
			/* ���@�G���[ */
			printf("���͍s�ɊԈႢ������܂��B%s\n", token);
			return;
		}
		/* �f�[�^�^��z��ɐݒ� (�^�ɂ���ďꍇ����)*/
		if (strcmp(token, "integer") == 0){
			tableInfo.fieldInfo[numField].dataType = TYPE_INTEGER;
		}
		else if (strcmp(token, "string") == 0){
			tableInfo.fieldInfo[numField].dataType = TYPE_STRING;
		}
		else{
			/* ���@�G���[ */
			printf("���͍s�ɊԈႢ������܂��B%s\n", token);
			return;
		}

		/* �t�B�[���h�����J�E���g���� */
		numField++;

		/* �t�B�[���h��������𒴂��Ă�����G���[ */
		if (numField > MAX_FIELD) {
			printf("�t�B�[���h��������𒴂��Ă��܂��B\n");
			return;
		}

		/* ���̃g�[�N���̓ǂݍ��� */
		if ((token = getNextToken()) == NULL) {
			/* ���@�G���[ */
			printf("���͍s�ɊԈႢ������܂��B%s\n", token);
			return;
		}

		/* �ǂݍ��񂾃g�[�N����")"��������A���[�v���甲���� */
		if (strcmp(token, ")") == 0) {
			break;
		}
		else if (strcmp(token, ",") == 0) {
			/* ���̃t�B�[���h��ǂݍ��ނ��߁A���[�v�̐擪�� */
			continue;
		}
		else {
			/* ���@�G���[ */
			printf("���͍s�ɊԈႢ������܂��B\n");
			return;
		}
	}

	tableInfo.numField = numField;

	/* createTable���Ăяo���A�e�[�u�����쐬 */
	if (createTable(tableName, &tableInfo) == OK) {
		printf("�e�[�u�����쐬���܂����B\n");
	}
	else {
		printf("�e�[�u���̍쐬�Ɏ��s���܂����B\n");
	}
}

/*
* callDropTable -- drop���̍\����͂�dropTable�̌Ăяo��
*
* ����:
*	�Ȃ�
*
* �Ԃ�l:
*	�Ȃ�
*
* drop table�̏���:
*	drop table �e�[�u����
*/
void callDropTable()
{
	char *token;
	char *tableName;

	/*drop�̎��̃g�[�N����ǂݍ��݁A���ꂪ"table"���ǂ������`�F�b�N */
	token = getNextToken();
	if (token == NULL || strcmp(token, "table") != 0) {
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B:%s\n", token);
		return;
	}

	/* �e�[�u������ǂݍ��� */
	if ((tableName = getNextToken()) == NULL) {
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B:%s\n", tableName);
		return;
	}

	/* createTable���Ăяo���A�e�[�u�����폜 */
	if (dropTable(tableName) == OK) {
		printf("�e�[�u�����폜���܂����B\n");
	}
	else {
		printf("�e�[�u���̍폜�Ɏ��s���܂����B\n");
	}
}

/*
* callInsertRecord -- insert���̍\����͂�insertRecord�̌Ăяo��
*
* ����:
*	�Ȃ�
*
* �Ԃ�l:
*	�Ȃ�
*
* insert�̏���:
*	insert into �e�[�u���� values ( �t�B�[���h�l , ... )
*/
void callInsertRecord()
{
	char *token;
	char *tableName;
	int numField;
	RecordData *recordData;
	TableInfo *tableInfo;
	int i;

	/*insert�̎��̃g�[�N����ǂݍ��݁A���ꂪ"into"���ǂ������`�F�b�N */
	token = getNextToken();
	if (token == NULL || strcmp(token, "into") != 0) {
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B:%s\n", token);
		return;
	}

	/* �e�[�u������ǂݍ��� */
	if ((tableName = getNextToken()) == NULL) {
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B:%s\n", tableName);
		return;
	}

	/* ���̃g�[�N����ǂݍ��݁A���ꂪ"values"���ǂ������`�F�b�N */
	token = getNextToken();
	if (token == NULL || strcmp(token, "values") != 0) {
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B%s\n", token);
		return;
	}

	/* ���̃g�[�N����ǂݍ��݁A���ꂪ"("���ǂ������`�F�b�N */
	token = getNextToken();
	if (token == NULL || strcmp(token, "(") != 0) {
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B%s\n", token);
		return;
	}

	/* �e�[�u���̏����擾���� */
	if ((tableInfo = getTableInfo(tableName)) == NULL) {
		/* �G���[���� */
		return;
	}

	/*RecordData�\���̂̃��������m��*/
	if ((recordData = (RecordData *)malloc(sizeof(RecordData))) == NULL){
		fprintf(stderr, "Malloc error.\n");
		return;
	}

	numField = 0;
	for (i = 0; i < tableInfo->numField; i++) {
		/* �t�B�[���h���̓ǂݍ��� */
		if ((token = getNextToken()) == NULL) {
			/* ���@�G���[ */
			printf("���͍s�ɊԈႢ������܂��B%s\n", token);
			freeTableInfo(tableInfo);
			return;
		}

		switch (tableInfo->fieldInfo[i].dataType) {
		case TYPE_INTEGER:
			recordData->fieldData[i].intValue = atoi(token);
			break;
		case TYPE_STRING:
			strcpy(recordData->fieldData[i].stringValue, token);
			break;
		default:
			freeTableInfo(tableInfo);
			return;
		}
		numField++;

		/* ���̃g�[�N���̓ǂݍ��� */
		token = getNextToken();
		if (token == NULL){
			/* ���@�G���[ */
			printf("���͍s�ɊԈႢ������܂��B%s\n", token);
			freeTableInfo(tableInfo);
			return;
		}

		if (strcmp(token, ")") == 0){
			/*���[�v����o��*/
			break;
		}
		else if (strcmp(token, ",") == 0) {
			/* ���̃t�B�[���h��ǂݍ��ނ��߁A���[�v�̐擪�� */
			continue;
		}
		else {
			/* ���@�G���[ */
			printf("���͍s�ɊԈႢ������܂��B\n");
			freeTableInfo(tableInfo);
			return;
		}
	}

	if (numField != tableInfo->numField){
		return;
	}
	freeTableInfo(tableInfo);

	/* insertRecord���Ăяo���A���R�[�h��}�� */
	if (insertRecord(tableName, recordData) == OK) {
		printf("���R�[�h��}�����܂����B\n");
	}
	else {
		printf("���R�[�h�̑}���Ɏ��s���܂����B\n");
	}
	free(recordData);
}

/*
* callSelectRecord -- select���̍\����͂�selectRecord�̌Ăяo��
*
* ����:
*	�Ȃ�
*
* �Ԃ�l:
*	�Ȃ�
*
* select�̏���:
*	select * from �e�[�u���� where ������
*	select �t�B�[���h�� , ... from �e�[�u���� where ������ (���W�ۑ�)
*/
void callSelectRecord()
{
	char *token;
	char *tableName;
	char *selectField;
	char *p;
	Condition condition;
	TableInfo *tableInfo;
	RecordSet *recordSet;
	int i, j;
	int numField;
	int allSelectFlag; /* *���ǂ����̔��f */
	char sendFieldName[MAX_FIELD][MAX_FIELD_NAME];

	for (i = 0; i < MAX_FIELD; i++){
		memset(&sendFieldName[i], '\0', MAX_FIELD_NAME);
	}

	/*select�̎��̃g�[�N����ǂݍ��݁Adistinct�����݂��邩���`�F�b�N */
	token = getNextToken();
	if (token == NULL) {
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B:%s\n", token);
		return;
	}

	if (strcmp(token, "distinct") == 0){
		condition.distinct = DISTINCT;

		/*���̃g�[�N����ǂݍ��݁A���ꂪ"*"���ǂ������`�F�b�N */
		token = getNextToken();
		if (token == NULL) {
			/* ���@�G���[ */
			printf("���͍s�ɊԈႢ������܂��B:%s\n", token);
			return;
		}
	}
	else{
		condition.distinct = NOT_DISTINCT;
	}

	selectField = (char *)malloc(sizeof(char)*MAX_FIELD_NAME*MAX_FIELD);
	p = selectField;
	numField = 0;
	if (strcmp(token, "*") != 0){
		allSelectFlag = 0;
		for (;;) {
			memcpy(p, token, strlen(token) + 1);
			p += strlen(token);

			/* �t�B�[���h�����J�E���g���� */
			numField++;

			/* ���̃g�[�N���̓ǂݍ��� */
			if ((token = getNextToken()) == NULL) {
				/* ���@�G���[ */
				printf("���͍s�ɊԈႢ������܂��B%s\n", token);
				return;
			}

			/* �ǂݍ��񂾃g�[�N����"from"��������A���[�v���甲���� */
			if (strcmp(token, "from") == 0) {
				break;
			}
			else if (strcmp(token, ",") == 0) {
				token = " ";
				memcpy(p, token, strlen(token) + 1);
				p += strlen(token);
				/* ���̃g�[�N���̓ǂݍ��� */
				if ((token = getNextToken()) == NULL) {
					/* ���@�G���[ */
					printf("���͍s�ɊԈႢ������܂��B%s\n", token);
					return;
				}
				continue;
			}
			else {
				/* ���@�G���[ */
				printf("���͍s�ɊԈႢ������܂��B%s\n", token);
				return;
			}
		}
	}
	/* *�������� */
	else if (strcmp(token, "*") == 0){
		allSelectFlag = 1;
		token = getNextToken();
		if (strcmp(token, "from") != 0) {
			/* ���@�G���[ */
			printf("���͍s�ɊԈႢ������܂��B:%s\n", token);
			return;
		}
	}
	/* �e�[�u������ǂݍ��� */
	if ((tableName = getNextToken()) == NULL) {
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B:%s\n", tableName);
		return;
	}

	/*���̃g�[�N����ǂݍ��݁A���ꂪ"where"���ǂ������`�F�b�N */
	token = getNextToken();

	/*���������͂���Ȃ������炷�ׂẴ��R�[�h��\��*/
	if (token == NULL) {
		/* �e�[�u���̏����擾���� */
		if ((tableInfo = getTableInfo(tableName)) == NULL) {
			/* �G���[���� */
			return;
		}
		numField = tableInfo->numField;
		printTableData(tableName, numField);
		freeTableInfo(tableInfo);
		free(selectField);
		return;
	}

	if (strcmp(token, "where") != 0) {
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B:%s\n", token);
		return;
	}

	/* �������̃t�B�[���h����ǂݍ��� */
	if ((token = getNextToken()) == NULL){
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B:%s\n", token);
		return;
	}
	strcpy(condition.name, token);

	/* �e�[�u���̏����擾���� */
	if ((tableInfo = getTableInfo(tableName)) == NULL) {
		/* �G���[���� */
		return;
	}

	/*�������Ɏw�肳�ꂽ�t�B�[���h�̃f�[�^�^�𒲂ׂ�*/
	condition.dataType = TYPE_UNKNOWN;
	for (i = 0; i < tableInfo->numField; i++){
		if (strcmp(tableInfo->fieldInfo[i].name, condition.name) == 0){
			/* �f�[�^�^��condition�ɃR�s�[���ă��[�v����ʂ���*/
			condition.dataType = tableInfo->fieldInfo[i].dataType;
			break;
		}
	}

	/*condition�����肷��*/
	conditionSet(token, &condition);

	/* selectRecord���Ăяo���A���R�[�h������ */
	if ((recordSet = selectRecord(tableName, &condition)) == NULL) {
		printf("���R�[�h�̌����Ɏ��s���܂����B\n");
	}

	/*selectField���t�B�[���h���ɂ��邩���ׂ�*/
	if (allSelectFlag == 0){
		setInputString(selectField);
		token = getNextToken();
		for (i = 0; i < MAX_FIELD; i++){
			if (token == NULL){
				break;
			}
			for (j = 0; j < tableInfo->numField; j++){
				/*token���t�B�[���h���ƈ�v������*/
				if (strcmp(tableInfo->fieldInfo[j].name, token) == 0){
					strcpy(sendFieldName[i], token);
					token = getNextToken();
					break;
				}
				if (j == tableInfo->numField - 1){
					printf("���̃t�B�[���h���͑��݂��܂���%s\n", token);
					return;
				}
			}
		}

	}

	/* *�̏ꍇ�� */
	else{
		for (i = 0; i < tableInfo->numField; i++){
			strcpy(sendFieldName[i], tableInfo->fieldInfo[i].name);
		}
		numField = tableInfo->numField;
	}

	/*�����������R�[�h�W����\��*/
	printRecordSet(recordSet, sendFieldName, numField);

	/*���*/
	free(selectField);
	freeTableInfo(tableInfo);
	freeRecordSet(recordSet);

}

/*
* callDeleteRecord -- delete���̍\����͂�deleteRecord�̌Ăяo��
*
* ����:
*	�Ȃ�
*
* �Ԃ�l:
*	�Ȃ�
*
* delete�̏���:
*	delete from �e�[�u���� where ������
*/
void callDeleteRecord()
{
	char *token;
	char *tableName;
	Condition condition;
	TableInfo *tableInfo;
	int i;

	/*delete�̎��̃g�[�N����ǂݍ��݁A���ꂪ"from"���ǂ������`�F�b�N */
	token = getNextToken();
	if (token == NULL || strcmp(token, "from") != 0) {
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B:%s\n", token);
		return;
	}


	/* �e�[�u������ǂݍ��� */
	if ((tableName = getNextToken()) == NULL) {
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B:%s\n", tableName);
		return;
	}

	/*���̃g�[�N����ǂݍ��݁A���ꂪ"where"���ǂ������`�F�b�N */
	token = getNextToken();
	if (token == NULL || strcmp(token, "where") != 0) {
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B:%s\n", token);
		return;
	}

	/* �������̃t�B�[���h����ǂݍ��� */
	if ((token = getNextToken()) == NULL){
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B:%s\n", token);
		return;
	}
	strcpy(condition.name, token);

	/* �e�[�u���̏����擾���� */
	if ((tableInfo = getTableInfo(tableName)) == NULL) {
		/* �G���[���� */
		return;
	}

	/*�������Ɏw�肳�ꂽ�t�B�[���h�̃f�[�^�^�𒲂ׂ�*/
	condition.dataType = TYPE_UNKNOWN;
	for (i = 0; i < tableInfo->numField; i++){
		if (strcmp(tableInfo->fieldInfo[i].name, condition.name) == 0){
			/* �f�[�^�^��condition�ɃR�s�[���ă��[�v����ʂ���*/
			condition.dataType = tableInfo->fieldInfo[i].dataType;
			break;
		}
	}
	freeTableInfo(tableInfo);

	/*condition�����肷��*/
	conditionSet(token, &condition);

	/* dereteRecord���Ăяo���A���R�[�h���폜 */
	if (deleteRecord(tableName, &condition) == OK) {
		printf("���R�[�h���폜���܂����B\n");
	}
	else {
		printf("���R�[�h�̍폜�Ɏ��s���܂����B\n");
	}
}

/*
* showTables -- show tables�����͂��ꂽ�Ƃ��ɁA�f�[�^�x�[�X���̃e�[�u����\��
*/
void showTables(){
	char *token;
	char plus = '+';
	char *hyphen = "----------------";
	char vertical = '|';
	char *tables = "tables";
	wchar_t datName[260];
	WIN32_FIND_DATA fd;
	HANDLE h;

	/*show�̎��̃g�[�N����ǂݍ��݁A���ꂪ"tables"���ǂ������`�F�b�N */
	token = getNextToken();
	if (token == NULL || strcmp(token, "tables") != 0) {
		/* ���@�G���[ */
		printf("���͍s�ɊԈႢ������܂��B:%s\n", token);
		return;
	}
	printf("%c%s%c\n", plus, hyphen, plus);						/* +-------------+ */
	printf("%c %-14s %c\n", vertical, tables, vertical);		/* | tables      | */
	printf("%c%s%c\n", plus, hyphen, plus);						/* +-------------+ */

	h = FindFirstFile(_T("C:\\Users\\shimpei\\Documents\\GitHub\\microdb\\microdatabase\\*.dat"), &fd);
	if (h != INVALID_HANDLE_VALUE){
		do{
			_tcscpy(datName, fd.cFileName);
			_tprintf(_T("%c %-14s %c\n"),vertical , datName ,vertical);
		} while (FindNextFile(h, &fd));
		FindClose(h);
	}
	printf("%c%s%c\n", plus, hyphen, plus);						/* +-------------+ */


}

/*
* test1 -- rundom��insert���܂���
* 1.��ɗ���������֐�(initializeRandomGenerator��getRandomInteger)��p��
* 2.test1()�����s����
*/
int getRandomInteger(int min, int max){
	int r;
	r = min + (int)(max - min + 1)*(rand() / (RAND_MAX + 1.0));
	return r;
}
void test1(){
	RecordData record;
	int i;
	int j;
	int r;
	char id[MAX_STRING];
	/*���������������Ĕ���������*/
	srand((unsigned int)time(NULL));
	/*100�̃��R�[�h��}������*/
	for (i = 0; i < TEST_SIZE; i++){
		j = 0;
		r = getRandomInteger(MY_MIN, MY_MAX);
		switch (r){
		case 1:
			sprintf(id, "i0000%c", '0' + r);
			strcpy(record.fieldData[j].name, "id");
			record.fieldData[j].dataType = TYPE_STRING;
			strcpy(record.fieldData[j].stringValue, id);
			j++;

			strcpy(record.fieldData[j].name, "name");
			record.fieldData[j].dataType = TYPE_STRING;
			strcpy(record.fieldData[j].stringValue, "Mickey");
			j++;

			strcpy(record.fieldData[j].name, "age");
			record.fieldData[j].dataType = TYPE_INTEGER;
			record.fieldData[j].intValue = 85;
			j++;

			strcpy(record.fieldData[j].name, "address");
			record.fieldData[j].dataType = TYPE_STRING;
			strcpy(record.fieldData[j].stringValue, "Urayasu");
			j++;

			record.numField = j;
			break;
		case 2:
			sprintf(id, "i0000%c", '0' + r);
			strcpy(record.fieldData[j].name, "id");
			record.fieldData[j].dataType = TYPE_STRING;
			strcpy(record.fieldData[j].stringValue, id);
			j++;

			strcpy(record.fieldData[j].name, "name");
			record.fieldData[j].dataType = TYPE_STRING;
			strcpy(record.fieldData[j].stringValue, "Minnie");
			j++;

			strcpy(record.fieldData[j].name, "age");
			record.fieldData[j].dataType = TYPE_INTEGER;
			record.fieldData[j].intValue = 85;
			j++;

			strcpy(record.fieldData[j].name, "address");
			record.fieldData[j].dataType = TYPE_STRING;
			strcpy(record.fieldData[j].stringValue, "Urayasu");
			j++;

			record.numField = j;
			break;
		case 3:
			sprintf(id, "i0000%c", '0' + r);
			strcpy(record.fieldData[j].name, "id");
			record.fieldData[j].dataType = TYPE_STRING;
			strcpy(record.fieldData[j].stringValue, id);
			j++;

			strcpy(record.fieldData[j].name, "name");
			record.fieldData[j].dataType = TYPE_STRING;
			strcpy(record.fieldData[j].stringValue, "Pooh");
			j++;

			strcpy(record.fieldData[j].name, "age");
			record.fieldData[j].dataType = TYPE_INTEGER;
			record.fieldData[j].intValue = 4;
			j++;

			strcpy(record.fieldData[j].name, "address");
			record.fieldData[j].dataType = TYPE_STRING;
			strcpy(record.fieldData[j].stringValue, "Canada");
			j++;

			record.numField = j;
			break;
		case 4:
			sprintf(id, "i0000%c", '0' + r);
			strcpy(record.fieldData[j].name, "id");
			record.fieldData[j].dataType = TYPE_STRING;
			strcpy(record.fieldData[j].stringValue, id);
			j++;

			strcpy(record.fieldData[j].name, "name");
			record.fieldData[j].dataType = TYPE_STRING;
			strcpy(record.fieldData[j].stringValue, "Pocahontas");
			j++;

			strcpy(record.fieldData[j].name, "age");
			record.fieldData[j].dataType = TYPE_INTEGER;
			record.fieldData[j].intValue = 16;
			j++;

			strcpy(record.fieldData[j].name, "address");
			record.fieldData[j].dataType = TYPE_STRING;
			strcpy(record.fieldData[j].stringValue, "America");
			j++;

			record.numField = j;
			break;
		case 5:
			sprintf(id, "i0000%c", '0' + r);
			strcpy(record.fieldData[j].name, "id");
			record.fieldData[j].dataType = TYPE_STRING;
			strcpy(record.fieldData[j].stringValue, id);
			j++;

			strcpy(record.fieldData[j].name, "name");
			record.fieldData[j].dataType = TYPE_STRING;
			strcpy(record.fieldData[j].stringValue, "Simba");
			j++;

			strcpy(record.fieldData[j].name, "age");
			record.fieldData[j].dataType = TYPE_INTEGER;
			record.fieldData[j].intValue = 20;
			j++;

			strcpy(record.fieldData[j].name, "address");
			record.fieldData[j].dataType = TYPE_STRING;
			strcpy(record.fieldData[j].stringValue, "PrideLand");
			j++;

			record.numField = j;
			break;
		}
		if (insertRecord("student", &record) != OK){
			fprintf(stderr, "cannot insert record\n");
			return;
		}
	}

}

/*
* main -- �}�C�N��DB�V�X�e���̃G���g���|�C���g
*/
int main()
{
	char input[MAX_INPUT];
	char *token;

	/* �t�@�C�����W���[���̏����� */
	if (initializeFileModule() != OK) {
		fprintf(stderr, "Cannot initialize file module.\n");
		exit(1);
	}

	/* �f�[�^��`�W���[���̏����� */
	if (initializeDataDefModule() != OK) {
		fprintf(stderr, "Cannot initialize data definition module.\n");
		exit(1);
	}

	/* �f�[�^����W���[���̏����� */
	if (initializeDataManipModule() != OK) {
		fprintf(stderr, "Cannot initialize data manipulation module.\n");
		exit(1);
	}

	/* �E�F���J�����b�Z�[�W���o�� */
	printf("�}�C�N��DBMS���N�����܂����B\n");

	/* 1�s�����͂�ǂݍ��݂Ȃ���A�������s�� */
	for (;;) {
		/* �v�����v�g�̏o�� */
		printf("\nDDL�܂���DML����͂��Ă��������B\n");
		printf("> ");

		/* �L�[�{�[�h���͂�1�s�ǂݍ��� */
		fgets(input, MAX_INPUT, stdin);

		/* ���͂̍Ō�̉��s����菜�� */
		if (strchr(input, '\n') != NULL) {
			*(strchr(input, '\n')) = '\0';
		}

		/* �����͂��邽�߂ɓ��͕������ݒ肷�� */
		setInputString(input);

		/* �ŏ��̃g�[�N�������o�� */
		token = getNextToken();

		/* ���͂���s��������A���[�v�̐擪�ɖ߂��Ă�蒼�� */
		if (token == NULL) {
			continue;
		}

		/* ���͂�"quit"��������A���[�v�𔲂��ăv���O�������I�������� */
		if (strcmp(token, "quit") == 0) {
			printf("�}�C�N��DBMS���I�����܂��B\n\n");
			break;
		}

		/* �ŏ��̃g�[�N���������ɂ���āA�Ăяo���֐������߂� */
		if (strcmp(token, "create") == 0) {
			callCreateTable();
		}
		else if (strcmp(token, "drop") == 0) {
			callDropTable();
		}
		else if (strcmp(token, "insert") == 0) {
			callInsertRecord();
		}
		else if (strcmp(token, "select") == 0) {
			callSelectRecord();
		}
		else if (strcmp(token, "delete") == 0) {
			callDeleteRecord();
		}
		else if (strcmp(token, "show") == 0){
			showTables();
		}
		else if (strcmp(token, "test1") == 0){
			test1();
		}
		else {
			/* ���͂ɊԈႢ�������� */
			printf("���͂ɊԈႢ������܂��B:%s\n", token);
			printf("������x���͂������Ă��������B\n\n");
		}
	}

	/* �e���W���[���̏I������ */
	finalizeDataManipModule();
	finalizeDataDefModule();
	finalizeFileModule();
}