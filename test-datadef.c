/*
* �f�[�^��`���W���[���e�X�g�v���O����
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "microdb.h"

/*
* main -- �f�[�^��`���W���[���̃e�X�g
*/
int main(int argc, char **argv)
{
	char tableName[20];
	TableInfo tableInfo;
	int i;

	/* �t�@�C�����W���[�������������� */
	if (initializeFileModule() != OK) {
		fprintf(stderr, "Cannot initialize file module.\n");
		exit(1);
	}

	/* �f�[�^��`�W���[�������������� */
	if (initializeDataDefModule() != OK) {
		fprintf(stderr, "Cannot initialize data definition module.\n");
		exit(1);
	}

	/*
	* ���̃v���O�����̑O��̎��s�̎��̃f�[�^��`�c���Ă���\��������̂ŁA
	* �Ƃ肠�����폜����
	*/
	dropTable("student");
	dropTable("teacher");

	/*
	* �ȉ��̓��͂�z�肵�ăe�X�g
	* create table student (
	*   id string,
	*   name string,
	*   age integer,
	*   address string
	* )
	*/
	strcpy(tableName, "student");
	i = 0;

	strcpy(tableInfo.fieldInfo[i].name, "id");
	tableInfo.fieldInfo[i].dataType = TYPE_STRING;
	i++;

	strcpy(tableInfo.fieldInfo[i].name, "name");
	tableInfo.fieldInfo[i].dataType = TYPE_STRING;
	i++;

	strcpy(tableInfo.fieldInfo[i].name, "age");
	tableInfo.fieldInfo[i].dataType = TYPE_INTEGER;
	i++;

	strcpy(tableInfo.fieldInfo[i].name, "address");
	tableInfo.fieldInfo[i].dataType = TYPE_STRING;
	i++;

	tableInfo.numField = i;

	/* �e�[�u���̍쐬 */
	if (createTable(tableName, &tableInfo) != OK) {
		/* �e�[�u���̍쐬�Ɏ��s */
		fprintf(stderr, "Cannot create table.\n");
		exit(1);
	}

	/* �쐬�����e�[�u���̏����o�� */
	printTableInfo(tableName);

	/*
	* �ȉ��̓��͂�z�肵�ăe�X�g
	* create table teacher (
	*   id string,
	*   name string,
	*   title string,
	*   age integer,
	*   address string,
	*   class integer
	* )
	*/
	strcpy(tableName, "teacher");
	i = 0;

	strcpy(tableInfo.fieldInfo[i].name, "id");
	tableInfo.fieldInfo[i].dataType = TYPE_STRING;
	i++;

	strcpy(tableInfo.fieldInfo[i].name, "name");
	tableInfo.fieldInfo[i].dataType = TYPE_STRING;
	i++;

	strcpy(tableInfo.fieldInfo[i].name, "title");
	tableInfo.fieldInfo[i].dataType = TYPE_STRING;
	i++;

	strcpy(tableInfo.fieldInfo[i].name, "age");
	tableInfo.fieldInfo[i].dataType = TYPE_INTEGER;
	i++;

	strcpy(tableInfo.fieldInfo[i].name, "address");
	tableInfo.fieldInfo[i].dataType = TYPE_STRING;
	i++;

	strcpy(tableInfo.fieldInfo[i].name, "class");
	tableInfo.fieldInfo[i].dataType = TYPE_INTEGER;
	i++;

	tableInfo.numField = i;

	/* �e�[�u���̍쐬 */
	if (createTable(tableName, &tableInfo) != OK) {
		/* �e�[�u���̍쐬�Ɏ��s */
		fprintf(stderr, "Cannot create table.\n");
		exit(1);
	}

	/* �쐬�����e�[�u���̏����o�� */
	printTableInfo(tableName);

	/* ��n�� */
	dropTable("student");
	dropTable("teacher");
	finalizeDataDefModule();
	finalizeFileModule();
}