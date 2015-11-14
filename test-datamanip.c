/*
* �f�[�^���샂�W���[���e�X�g�v���O����
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "microdb.h"

#define TABLE_NAME "student"

/*
* test1 -- ���R�[�h�̑}��
*/
Result test1()
{
	RecordData record;
	TableInfo *tableInfo;
	int i,numField;

	/*
	* �ȉ��̃��R�[�h��}��
	* ('i04001', 'Mickey', 20, 'Urayasu')
	*/
	i = 0;
	strcpy(record.fieldData[i].name, "id");
	record.fieldData[i].dataType = TYPE_STRING;
	strcpy(record.fieldData[i].stringValue, "i04001");
	i++;

	strcpy(record.fieldData[i].name, "name");
	record.fieldData[i].dataType = TYPE_STRING;
	strcpy(record.fieldData[i].stringValue, "Mickey");
	i++;

	strcpy(record.fieldData[i].name, "age");
	record.fieldData[i].dataType = TYPE_INTEGER;
	record.fieldData[i].intValue = 20;
	i++;

	strcpy(record.fieldData[i].name, "address");
	record.fieldData[i].dataType = TYPE_STRING;
	strcpy(record.fieldData[i].stringValue, "Urayasu");
	i++;

	record.numField = i;

	if (insertRecord(TABLE_NAME, &record) != OK) {
		fprintf(stderr, "Cannot insert record.\n");
		return NG;
	}

	/*
	* �ȉ��̃��R�[�h��}��
	* ('i04001', 'Mickey', 20, 'Urayasu')
	*/
	i = 0;
	strcpy(record.fieldData[i].name, "id");
	record.fieldData[i].dataType = TYPE_STRING;
	strcpy(record.fieldData[i].stringValue, "i04001");
	i++;

	strcpy(record.fieldData[i].name, "name");
	record.fieldData[i].dataType = TYPE_STRING;
	strcpy(record.fieldData[i].stringValue, "Mickey");
	i++;

	strcpy(record.fieldData[i].name, "age");
	record.fieldData[i].dataType = TYPE_INTEGER;
	record.fieldData[i].intValue = 20;
	i++;

	strcpy(record.fieldData[i].name, "address");
	record.fieldData[i].dataType = TYPE_STRING;
	strcpy(record.fieldData[i].stringValue, "Urayasu");
	i++;

	record.numField = i;

	if (insertRecord(TABLE_NAME, &record) != OK) {
		fprintf(stderr, "Cannot insert record.\n");
		return NG;
	}

	/*
	* �ȉ��̃��R�[�h��}��
	* ('i04002', 'Minnie', 19, 'Urayasu')
	*/
	i = 0;
	strcpy(record.fieldData[i].name, "id");
	record.fieldData[i].dataType = TYPE_STRING;
	strcpy(record.fieldData[i].stringValue, "i04002");
	i++;

	strcpy(record.fieldData[i].name, "name");
	record.fieldData[i].dataType = TYPE_STRING;
	strcpy(record.fieldData[i].stringValue, "Minnie");
	i++;

	strcpy(record.fieldData[i].name, "age");
	record.fieldData[i].dataType = TYPE_INTEGER;
	record.fieldData[i].intValue = 19;
	i++;

	strcpy(record.fieldData[i].name, "address");
	record.fieldData[i].dataType = TYPE_STRING;
	strcpy(record.fieldData[i].stringValue, "Urayasu");
	i++;

	record.numField = i;

	if (insertRecord(TABLE_NAME, &record) != OK) {
		fprintf(stderr, "Cannot insert record.\n");
		return NG;
	}

	/*
	* �ȉ��̃��R�[�h��}��
	* ('i04003', 'Donald', 17, 'Florida')
	*/
	i = 0;
	strcpy(record.fieldData[i].name, "id");
	record.fieldData[i].dataType = TYPE_STRING;
	strcpy(record.fieldData[i].stringValue, "i04003");
	i++;

	strcpy(record.fieldData[i].name, "name");
	record.fieldData[i].dataType = TYPE_STRING;
	strcpy(record.fieldData[i].stringValue, "Donald");
	i++;

	strcpy(record.fieldData[i].name, "age");
	record.fieldData[i].dataType = TYPE_INTEGER;
	record.fieldData[i].intValue = 17;
	i++;

	strcpy(record.fieldData[i].name, "address");
	record.fieldData[i].dataType = TYPE_STRING;
	strcpy(record.fieldData[i].stringValue, "Florida");
	i++;

	record.numField = i;

	if (insertRecord(TABLE_NAME, &record) != OK) {
		fprintf(stderr, "Cannot insert record.\n");
		return NG;
	}

	/*
	* �ȉ��̃��R�[�h��}��
	* ('i04003', 'Donald', 17, 'Florida')
	*/
	i = 0;
	strcpy(record.fieldData[i].name, "id");
	record.fieldData[i].dataType = TYPE_STRING;
	strcpy(record.fieldData[i].stringValue, "i04003");
	i++;

	strcpy(record.fieldData[i].name, "name");
	record.fieldData[i].dataType = TYPE_STRING;
	strcpy(record.fieldData[i].stringValue, "Donald");
	i++;

	strcpy(record.fieldData[i].name, "age");
	record.fieldData[i].dataType = TYPE_INTEGER;
	record.fieldData[i].intValue = 17;
	i++;

	strcpy(record.fieldData[i].name, "address");
	record.fieldData[i].dataType = TYPE_STRING;
	strcpy(record.fieldData[i].stringValue, "Florida");
	i++;

	record.numField = i;

	if (insertRecord(TABLE_NAME, &record) != OK) {
		fprintf(stderr, "Cannot insert record.\n");
		return NG;
	}

	/*
	* �ȉ��̃��R�[�h��}��
	* ('i04004', 'Daisy', 15, 'Florida')
	*/
	i = 0;
	strcpy(record.fieldData[i].name, "id");
	record.fieldData[i].dataType = TYPE_STRING;
	strcpy(record.fieldData[i].stringValue, "i04004");
	i++;

	strcpy(record.fieldData[i].name, "name");
	record.fieldData[i].dataType = TYPE_STRING;
	strcpy(record.fieldData[i].stringValue, "Daisy");
	i++;

	strcpy(record.fieldData[i].name, "age");
	record.fieldData[i].dataType = TYPE_INTEGER;
	record.fieldData[i].intValue = 15;
	i++;

	strcpy(record.fieldData[i].name, "address");
	record.fieldData[i].dataType = TYPE_STRING;
	strcpy(record.fieldData[i].stringValue, "Florida");
	i++;

	record.numField = i;

	if (insertRecord(TABLE_NAME, &record) != OK) {
		fprintf(stderr, "Cannot insert record.\n");
		return NG;
	}
	/* �e�[�u���̏����擾���� */
	if ((tableInfo = getTableInfo(TABLE_NAME)) == NULL) {
		/* �G���[���� */
		return NG;
	}
	numField = tableInfo->numField;
	/* �f�[�^��\������ */
	printTableData(TABLE_NAME,numField);
	freeTableInfo(tableInfo);
	return OK;
}

/*
* test2 -- ����
*/
Result test2()
{
	RecordSet *recordSet;
	Condition condition;
	TableInfo *tableInfo;
	char sendFieldName[MAX_FIELD][MAX_FIELD_NAME];
	int i,numField;

	/*
	* �ȉ��̌��������s
	* select * from TABLE_NAME where age > 17
	*/
	strcpy(condition.name, "age");
	condition.dataType = TYPE_INTEGER;
	condition.operator = OPR_GREATER_THAN;
	condition.intValue = 17;
	condition.distinct = NOT_DISTINCT;

	if ((recordSet = selectRecord(TABLE_NAME, &condition)) == NULL) {
		fprintf(stderr, "Cannot select records.\n");
		return NG;
	}

	/* �e�[�u���̏����擾���� */
	if ((tableInfo = getTableInfo(TABLE_NAME)) == NULL) {
		/* �G���[���� */
		return NG;
	}
	for (i = 0; i < tableInfo->numField; i++){
		strcpy(sendFieldName[i], tableInfo->fieldInfo[i].name);
	}
	numField = tableInfo->numField;
	/* ���ʂ�\�� */
	printf("age > 17, not distinct\n");
	printRecordSet(recordSet,sendFieldName,numField);

	/* ���ʂ���� */
	freeRecordSet(recordSet);

	/*
	* �ȉ��̌��������s
	* select distinct * from TABLE_NAME where age > 17
	*/
	strcpy(condition.name, "age");
	condition.dataType = TYPE_INTEGER;
	condition.operator = OPR_GREATER_THAN;
	condition.intValue = 17;
	condition.distinct = DISTINCT;

	if ((recordSet = selectRecord(TABLE_NAME, &condition)) == NULL) {
		fprintf(stderr, "Cannot select records.\n");
		return NG;
	}

	/* ���ʂ�\�� */
	printf("age > 17, distinct\n");
	printRecordSet(recordSet, sendFieldName, numField);

	/* ���ʂ���� */
	freeRecordSet(recordSet);

	/*
	* �ȉ��̌��������s
	* select * from TABLE_NAME where address != 'Florida'
	*/
	strcpy(condition.name, "address");
	condition.dataType = TYPE_STRING;
	condition.operator = OPR_NOT_EQUAL;
	strcpy(condition.stringValue, "Florida");
	condition.distinct = NOT_DISTINCT;

	if ((recordSet = selectRecord(TABLE_NAME, &condition)) == NULL) {
		fprintf(stderr, "Cannot select records.\n");
		return NG;
	}

	/* ���ʂ�\�� */
	printf("address != 'Florida', not distinct\n");
	printRecordSet(recordSet, sendFieldName, numField);

	/* ���ʂ���� */
	freeRecordSet(recordSet);

	/*
	* �ȉ��̌��������s
	* select distinct * from TABLE_NAME where address != 'Florida'
	*/
	strcpy(condition.name, "address");
	condition.dataType = TYPE_STRING;
	condition.operator = OPR_NOT_EQUAL;
	strcpy(condition.stringValue, "Florida");
	condition.distinct = DISTINCT;

	if ((recordSet = selectRecord(TABLE_NAME, &condition)) == NULL) {
		fprintf(stderr, "Cannot select records.\n");
		return NG;
	}

	/* ���ʂ�\�� */
	printf("address != 'Florida', distinct\n");
	printRecordSet(recordSet, sendFieldName, numField);

	/* ���ʂ���� */
	freeRecordSet(recordSet);
	freeTableInfo(tableInfo);

	return OK;
}

/*
* test3 -- �폜
*/
Result test3()
{
	Condition condition;
	TableInfo *tableInfo;
	int numField;

	/*
	* �ȉ��̌��������s
	* delete from TABLE_NAME where name != 'Mickey'
	*/
	strcpy(condition.name, "name");
	condition.dataType = TYPE_STRING;
	condition.operator = OPR_NOT_EQUAL;
	strcpy(condition.stringValue, "Mickey");

	if (deleteRecord(TABLE_NAME, &condition) != OK) {
		fprintf(stderr, "Cannot delete records.\n");
		return NG;
	}

	/* �f�[�^��\������ */

	/* �e�[�u���̏����擾���� */
	if ((tableInfo = getTableInfo(TABLE_NAME)) == NULL) {
		/* �G���[���� */
		return NG;
	}
	numField = tableInfo->numField;
	/* �f�[�^��\������ */
	printTableData(TABLE_NAME, numField);
	freeTableInfo(tableInfo);

	return OK;
}

/*
* main -- �f�[�^���샂�W���[���̃e�X�g
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

	/* �f�[�^����W���[�������������� */
	if (initializeDataManipModule() != OK) {
		fprintf(stderr, "Cannot initialize data manipulation module.\n");
		exit(1);
	}

	/*
	* ���̃v���O�����̑O��̎��s�̎��̃f�[�^��`�c���Ă���\��������̂ŁA
	* �Ƃ肠�����폜����
	*/
	dropTable(TABLE_NAME);

	/*
	* �ȉ��̃e�[�u�����쐬
	* create table student (
	*   id string,
	*   name string,
	*   age integer,
	*   address string
	* )
	*/
	strcpy(tableName, TABLE_NAME);
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

	/* �}���e�X�g */
	fprintf(stderr, "test1: Start\n\n");
	if (test1() == OK) {
		fprintf(stderr, "test1: OK\n\n");
	}
	else {
		fprintf(stderr, "test1: NG\n\n");
	}

	/* �����e�X�g */
	fprintf(stderr, "test2: Start\n\n");
	if (test2() == OK) {
		fprintf(stderr, "test2: OK\n\n");
	}
	else {
		fprintf(stderr, "test2: NG\n\n");
	}

	/* �폜�e�X�g */
	fprintf(stderr, "test3: Start\n\n");
	if (test3() == OK) {
		fprintf(stderr, "test3: OK\n\n");
	}
	else {
		fprintf(stderr, "test3: NG\n\n");
	}

	/* ��n�� */
	dropTable(TABLE_NAME);
	finalizeDataManipModule();
	finalizeDataDefModule();
	finalizeFileModule();
}