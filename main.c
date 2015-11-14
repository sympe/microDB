/*
* main.c -- メインモジュール
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>
#include <string.h>
#include "microdb.h"

/*
* MAX_INPUT -- 入力行の最大文字数
*/
#define MAX_INPUT 256

/*
* TEST_SIZE -- 挿入するレコード数
*/
#define TEST_SIZE 1000

#define MY_MIN 1
#define MY_MAX 5

/*
* inputString -- 字句解析中の文字列を収める配列
*
* 大きさを「3 * MAX_INPUT」とするのは、入力行のすべての文字が区切り記号でも
* バッファオーバーフローを起こさないようにするため。
*/
static char inputString[3 * MAX_INPUT];

/*
* nextPosition -- 字句解析中の場所
*/
static char *nextPosition;

/*
* setInputString -- 字句解析する文字列の設定
*
* 引数:
*	string: 解析する文字列
*
* 返り値:
*	なし
*/
static void setInputString(char *string)
{
	char *p = string;
	char *q = inputString;

	/* 入力行をコピーして保存する */
	while (*p != '\0') {
		/* 引用符の場合には、次の引用符まで読み飛ばす */
		if (*p == '\'' || *p == '"') {
			char *quote;
			quote = p;
			*q++ = *p++;

			/* 閉じる引用符(または文字列の最後)までコピーする */
			while (*p != *quote && *p != '\0') {
				*q++ = *p++;
			}

			/* 閉じる引用符自体もコピーする */
			if (*p == *quote) {
				*q++ = *p++;
			}
			continue;
		}

		/* 区切り記号の場合には、その前後に空白文字を入れる */
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

	/* getNextToken()の読み出し開始位置を文字列の先頭に設定する */
	nextPosition = inputString;
}

/*
* getNextToken -- setInputStringで設定した文字列からの字句の取り出し
*
* 引数:
*	なし
*
* 返り値:
*	次の字句の先頭番地を返す。最後までたどり着いたらNULLを返す。
*/
static char *getNextToken()
{
	char *start;
	char *end;
	char *p;

	/* 空白文字が複数続いていたら、その分nextPositionを移動させる */
	while (*nextPosition == ' ') {
		nextPosition++;
	}
	start = nextPosition;

	/* nextPositionの位置が文字列の最後('\0')だったら、解析終了 */
	if (*nextPosition == '\0') {
		return NULL;
	}

	/* nextPositionの位置以降で、最初に見つかる空白文字の場所を探す */
	p = nextPosition;
	while (*p != ' ' && *p != '\0') {
		/* 引用符の場合には、次の引用符まで読み飛ばす */
		if (*p == '\'' || *p == '"') {
			char *quote;
			quote = p;
			p++;

			/* 閉じる引用符(または文字列の最後)まで読み飛ばす */
			while (*p != *quote && *p != '\0') {
				p++;
			}

			/* 閉じる引用符自体も読み飛ばす */
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
	* 空白文字を終端文字で置き換えるとともに、
	* 次回のgetNextToken()の呼び出しのために
	* nextPositionをその次の文字に移動する
	*/
	if (*end != '\0') {
		*end = '\0';
		nextPosition = end + 1;
	}
	else {
		/*
		* (*end == '\0')の場合は文字列の最後まで解析が終わっているので、
		* 次回のgetNextToken()の呼び出しのときにNULLが返るように、
		* nextPositionを文字列の一番最後に移動させる
		*/
		nextPosition = end;
	}

	/* 字句の先頭番地を返す */
	return start;
}

/*
*condtionSet -- select文とdelete文のconditionを決定する
*
* 引数:token
* :condition
*/
static void conditionSet(char *token, Condition *condition){

	char *p;	/*stringが''で囲まれているかを調べるのに用いる*/
	char *q;	/*''を取り除いた文字列をいれる*/
	char *quote;	/*シングルコーテーション*/

	/*フィールドのデータ型が探索できないときは、文法エラー*/
	if (condition->dataType == TYPE_UNKNOWN){
		printf("指定したフィールドはありません%s\n", condition->name);
		return;
	}

	/*次のトークンである比較演算子を読み込む*/
	if ((token = getNextToken()) == NULL){
		/* 文法エラー */
		printf("入力行に間違いがあります。:%s\n", token);
		return;
	}

	/*比較演算子をconditionに設定*/
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
		/* 文法エラー */
		printf("入力行に間違いがあります。:%s\n", token);
		return;
	}

	/*次のトークンである条件式を読み込む*/
	if ((token = getNextToken()) == NULL){
		/* 文法エラー */
		printf("入力行に間違いがあります。:%s\n", token);
		return;
	}

	/*条件式の値をconditionに設定*/
	switch (condition->dataType){
	case TYPE_INTEGER:
		/*トークンの文字列を整数に変換して入れるよ！*/
		condition->intValue = atoi(token);
		break;
	case TYPE_STRING:
		/*'で囲まれているかを調べる*/
		p = token;
		/*頭が'かどうか調べる*/
		if (*p == '\'') {
			quote = p;
			p++;
			q = p;	/*'をとばしてqに値を格納していく*/
			/*	文字列の最後まで読み込む */
			while (*p != *quote && *p != '\0') {
				p++;
			}

			/* 終わりが'じゃなかったときは */
			if (*p != '\'') {
				printf("条件式の指定に間違いがあります%s\n", token);
				return;
			}
		}
		else{
			printf("条件式の指定に間違いがあります%s\n", token);
			return;
		}
		*p = '\0';
		strcpy(condition->stringValue, q);
		break;
	default:
		printf("データタイプが適切ではありませんでした\n");
		break;
	}
}
/*
* callCreateTable -- create文の構文解析とcreateTableの呼び出し
*
* 引数:
*	なし
*
* 返り値:
*	なし
*
* create tableの書式:
*	create table テーブル名 ( フィールド名 データ型, ... )
*/
void callCreateTable()
{
	char *token;
	char *tableName;
	int numField;
	TableInfo tableInfo;

	/* createの次のトークンを読み込み、それが"table"かどうかをチェック */
	token = getNextToken();
	if (token == NULL || strcmp(token, "table") != 0) {
		/* 文法エラー */
		printf("入力行に間違いがあります。:%s\n", token);
		return;
	}

	/* テーブル名を読み込む */
	if ((tableName = getNextToken()) == NULL) {
		/* 文法エラー */
		printf("入力行に間違いがあります。:%s\n", tableName);
		return;
	}

	/* 次のトークンを読み込み、それが"("かどうかをチェック */
	token = getNextToken();
	if (token == NULL || strcmp(token, "(") != 0) {
		/* 文法エラー */
		printf("入力行に間違いがあります。%s\n", token);
		return;
	}

	/*
	* ここから、フィールド名とデータ型の組を繰り返し読み込み、
	* 配列fieldInfoに入れていく。
	*/
	numField = 0;
	for (;;) {
		/* フィールド名の読み込み */
		if ((token = getNextToken()) == NULL) {
			/* 文法エラー */
			printf("入力行に間違いがあります。%s\n", token);
			return;
		}

		/* 読み込んだトークンが")"だったら、ループから抜ける */
		if (strcmp(token, ")") == 0) {
			break;
		}

		/* フィールド名を配列に設定 */
		strcpy(tableInfo.fieldInfo[numField].name, token);

		/* データ型の読み込み */
		token = getNextToken();
		if (token == NULL){
			/* 文法エラー */
			printf("入力行に間違いがあります。%s\n", token);
			return;
		}
		/* データ型を配列に設定 (型によって場合分け)*/
		if (strcmp(token, "integer") == 0){
			tableInfo.fieldInfo[numField].dataType = TYPE_INTEGER;
		}
		else if (strcmp(token, "string") == 0){
			tableInfo.fieldInfo[numField].dataType = TYPE_STRING;
		}
		else{
			/* 文法エラー */
			printf("入力行に間違いがあります。%s\n", token);
			return;
		}

		/* フィールド数をカウントする */
		numField++;

		/* フィールド数が上限を超えていたらエラー */
		if (numField > MAX_FIELD) {
			printf("フィールド数が上限を超えています。\n");
			return;
		}

		/* 次のトークンの読み込み */
		if ((token = getNextToken()) == NULL) {
			/* 文法エラー */
			printf("入力行に間違いがあります。%s\n", token);
			return;
		}

		/* 読み込んだトークンが")"だったら、ループから抜ける */
		if (strcmp(token, ")") == 0) {
			break;
		}
		else if (strcmp(token, ",") == 0) {
			/* 次のフィールドを読み込むため、ループの先頭へ */
			continue;
		}
		else {
			/* 文法エラー */
			printf("入力行に間違いがあります。\n");
			return;
		}
	}

	tableInfo.numField = numField;

	/* createTableを呼び出し、テーブルを作成 */
	if (createTable(tableName, &tableInfo) == OK) {
		printf("テーブルを作成しました。\n");
	}
	else {
		printf("テーブルの作成に失敗しました。\n");
	}
}

/*
* callDropTable -- drop文の構文解析とdropTableの呼び出し
*
* 引数:
*	なし
*
* 返り値:
*	なし
*
* drop tableの書式:
*	drop table テーブル名
*/
void callDropTable()
{
	char *token;
	char *tableName;

	/*dropの次のトークンを読み込み、それが"table"かどうかをチェック */
	token = getNextToken();
	if (token == NULL || strcmp(token, "table") != 0) {
		/* 文法エラー */
		printf("入力行に間違いがあります。:%s\n", token);
		return;
	}

	/* テーブル名を読み込む */
	if ((tableName = getNextToken()) == NULL) {
		/* 文法エラー */
		printf("入力行に間違いがあります。:%s\n", tableName);
		return;
	}

	/* createTableを呼び出し、テーブルを削除 */
	if (dropTable(tableName) == OK) {
		printf("テーブルを削除しました。\n");
	}
	else {
		printf("テーブルの削除に失敗しました。\n");
	}
}

/*
* callInsertRecord -- insert文の構文解析とinsertRecordの呼び出し
*
* 引数:
*	なし
*
* 返り値:
*	なし
*
* insertの書式:
*	insert into テーブル名 values ( フィールド値 , ... )
*/
void callInsertRecord()
{
	char *token;
	char *tableName;
	int numField;
	RecordData *recordData;
	TableInfo *tableInfo;
	int i;

	/*insertの次のトークンを読み込み、それが"into"かどうかをチェック */
	token = getNextToken();
	if (token == NULL || strcmp(token, "into") != 0) {
		/* 文法エラー */
		printf("入力行に間違いがあります。:%s\n", token);
		return;
	}

	/* テーブル名を読み込む */
	if ((tableName = getNextToken()) == NULL) {
		/* 文法エラー */
		printf("入力行に間違いがあります。:%s\n", tableName);
		return;
	}

	/* 次のトークンを読み込み、それが"values"かどうかをチェック */
	token = getNextToken();
	if (token == NULL || strcmp(token, "values") != 0) {
		/* 文法エラー */
		printf("入力行に間違いがあります。%s\n", token);
		return;
	}

	/* 次のトークンを読み込み、それが"("かどうかをチェック */
	token = getNextToken();
	if (token == NULL || strcmp(token, "(") != 0) {
		/* 文法エラー */
		printf("入力行に間違いがあります。%s\n", token);
		return;
	}

	/* テーブルの情報を取得する */
	if ((tableInfo = getTableInfo(tableName)) == NULL) {
		/* エラー処理 */
		return;
	}

	/*RecordData構造体のメモリを確保*/
	if ((recordData = (RecordData *)malloc(sizeof(RecordData))) == NULL){
		fprintf(stderr, "Malloc error.\n");
		return;
	}

	numField = 0;
	for (i = 0; i < tableInfo->numField; i++) {
		/* フィールド名の読み込み */
		if ((token = getNextToken()) == NULL) {
			/* 文法エラー */
			printf("入力行に間違いがあります。%s\n", token);
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

		/* 次のトークンの読み込み */
		token = getNextToken();
		if (token == NULL){
			/* 文法エラー */
			printf("入力行に間違いがあります。%s\n", token);
			freeTableInfo(tableInfo);
			return;
		}

		if (strcmp(token, ")") == 0){
			/*ループから出る*/
			break;
		}
		else if (strcmp(token, ",") == 0) {
			/* 次のフィールドを読み込むため、ループの先頭へ */
			continue;
		}
		else {
			/* 文法エラー */
			printf("入力行に間違いがあります。\n");
			freeTableInfo(tableInfo);
			return;
		}
	}

	if (numField != tableInfo->numField){
		return;
	}
	freeTableInfo(tableInfo);

	/* insertRecordを呼び出し、レコードを挿入 */
	if (insertRecord(tableName, recordData) == OK) {
		printf("レコードを挿入しました。\n");
	}
	else {
		printf("レコードの挿入に失敗しました。\n");
	}
	free(recordData);
}

/*
* callSelectRecord -- select文の構文解析とselectRecordの呼び出し
*
* 引数:
*	なし
*
* 返り値:
*	なし
*
* selectの書式:
*	select * from テーブル名 where 条件式
*	select フィールド名 , ... from テーブル名 where 条件式 (発展課題)
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
	int allSelectFlag; /* *かどうかの判断 */
	char sendFieldName[MAX_FIELD][MAX_FIELD_NAME];

	for (i = 0; i < MAX_FIELD; i++){
		memset(&sendFieldName[i], '\0', MAX_FIELD_NAME);
	}

	/*selectの次のトークンを読み込み、distinctが存在するかをチェック */
	token = getNextToken();
	if (token == NULL) {
		/* 文法エラー */
		printf("入力行に間違いがあります。:%s\n", token);
		return;
	}

	if (strcmp(token, "distinct") == 0){
		condition.distinct = DISTINCT;

		/*次のトークンを読み込み、それが"*"かどうかをチェック */
		token = getNextToken();
		if (token == NULL) {
			/* 文法エラー */
			printf("入力行に間違いがあります。:%s\n", token);
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

			/* フィールド数をカウントする */
			numField++;

			/* 次のトークンの読み込み */
			if ((token = getNextToken()) == NULL) {
				/* 文法エラー */
				printf("入力行に間違いがあります。%s\n", token);
				return;
			}

			/* 読み込んだトークンが"from"だったら、ループから抜ける */
			if (strcmp(token, "from") == 0) {
				break;
			}
			else if (strcmp(token, ",") == 0) {
				token = " ";
				memcpy(p, token, strlen(token) + 1);
				p += strlen(token);
				/* 次のトークンの読み込み */
				if ((token = getNextToken()) == NULL) {
					/* 文法エラー */
					printf("入力行に間違いがあります。%s\n", token);
					return;
				}
				continue;
			}
			else {
				/* 文法エラー */
				printf("入力行に間違いがあります。%s\n", token);
				return;
			}
		}
	}
	/* *だったら */
	else if (strcmp(token, "*") == 0){
		allSelectFlag = 1;
		token = getNextToken();
		if (strcmp(token, "from") != 0) {
			/* 文法エラー */
			printf("入力行に間違いがあります。:%s\n", token);
			return;
		}
	}
	/* テーブル名を読み込む */
	if ((tableName = getNextToken()) == NULL) {
		/* 文法エラー */
		printf("入力行に間違いがあります。:%s\n", tableName);
		return;
	}

	/*次のトークンを読み込み、それが"where"かどうかをチェック */
	token = getNextToken();

	/*条件が入力されなかったらすべてのレコードを表示*/
	if (token == NULL) {
		/* テーブルの情報を取得する */
		if ((tableInfo = getTableInfo(tableName)) == NULL) {
			/* エラー処理 */
			return;
		}
		numField = tableInfo->numField;
		printTableData(tableName, numField);
		freeTableInfo(tableInfo);
		free(selectField);
		return;
	}

	if (strcmp(token, "where") != 0) {
		/* 文法エラー */
		printf("入力行に間違いがあります。:%s\n", token);
		return;
	}

	/* 条件式のフィールド名を読み込む */
	if ((token = getNextToken()) == NULL){
		/* 文法エラー */
		printf("入力行に間違いがあります。:%s\n", token);
		return;
	}
	strcpy(condition.name, token);

	/* テーブルの情報を取得する */
	if ((tableInfo = getTableInfo(tableName)) == NULL) {
		/* エラー処理 */
		return;
	}

	/*条件式に指定されたフィールドのデータ型を調べる*/
	condition.dataType = TYPE_UNKNOWN;
	for (i = 0; i < tableInfo->numField; i++){
		if (strcmp(tableInfo->fieldInfo[i].name, condition.name) == 0){
			/* データ型をconditionにコピーしてループからぬける*/
			condition.dataType = tableInfo->fieldInfo[i].dataType;
			break;
		}
	}

	/*conditionを決定する*/
	conditionSet(token, &condition);

	/* selectRecordを呼び出し、レコードを検索 */
	if ((recordSet = selectRecord(tableName, &condition)) == NULL) {
		printf("レコードの検索に失敗しました。\n");
	}

	/*selectFieldがフィールド名にあるか調べる*/
	if (allSelectFlag == 0){
		setInputString(selectField);
		token = getNextToken();
		for (i = 0; i < MAX_FIELD; i++){
			if (token == NULL){
				break;
			}
			for (j = 0; j < tableInfo->numField; j++){
				/*tokenがフィールド名と一致したら*/
				if (strcmp(tableInfo->fieldInfo[j].name, token) == 0){
					strcpy(sendFieldName[i], token);
					token = getNextToken();
					break;
				}
				if (j == tableInfo->numField - 1){
					printf("そのフィールド名は存在しません%s\n", token);
					return;
				}
			}
		}

	}

	/* *の場合は */
	else{
		for (i = 0; i < tableInfo->numField; i++){
			strcpy(sendFieldName[i], tableInfo->fieldInfo[i].name);
		}
		numField = tableInfo->numField;
	}

	/*検索したレコード集合を表示*/
	printRecordSet(recordSet, sendFieldName, numField);

	/*解放*/
	free(selectField);
	freeTableInfo(tableInfo);
	freeRecordSet(recordSet);

}

/*
* callDeleteRecord -- delete文の構文解析とdeleteRecordの呼び出し
*
* 引数:
*	なし
*
* 返り値:
*	なし
*
* deleteの書式:
*	delete from テーブル名 where 条件式
*/
void callDeleteRecord()
{
	char *token;
	char *tableName;
	Condition condition;
	TableInfo *tableInfo;
	int i;

	/*deleteの次のトークンを読み込み、それが"from"かどうかをチェック */
	token = getNextToken();
	if (token == NULL || strcmp(token, "from") != 0) {
		/* 文法エラー */
		printf("入力行に間違いがあります。:%s\n", token);
		return;
	}


	/* テーブル名を読み込む */
	if ((tableName = getNextToken()) == NULL) {
		/* 文法エラー */
		printf("入力行に間違いがあります。:%s\n", tableName);
		return;
	}

	/*次のトークンを読み込み、それが"where"かどうかをチェック */
	token = getNextToken();
	if (token == NULL || strcmp(token, "where") != 0) {
		/* 文法エラー */
		printf("入力行に間違いがあります。:%s\n", token);
		return;
	}

	/* 条件式のフィールド名を読み込む */
	if ((token = getNextToken()) == NULL){
		/* 文法エラー */
		printf("入力行に間違いがあります。:%s\n", token);
		return;
	}
	strcpy(condition.name, token);

	/* テーブルの情報を取得する */
	if ((tableInfo = getTableInfo(tableName)) == NULL) {
		/* エラー処理 */
		return;
	}

	/*条件式に指定されたフィールドのデータ型を調べる*/
	condition.dataType = TYPE_UNKNOWN;
	for (i = 0; i < tableInfo->numField; i++){
		if (strcmp(tableInfo->fieldInfo[i].name, condition.name) == 0){
			/* データ型をconditionにコピーしてループからぬける*/
			condition.dataType = tableInfo->fieldInfo[i].dataType;
			break;
		}
	}
	freeTableInfo(tableInfo);

	/*conditionを決定する*/
	conditionSet(token, &condition);

	/* dereteRecordを呼び出し、レコードを削除 */
	if (deleteRecord(tableName, &condition) == OK) {
		printf("レコードを削除しました。\n");
	}
	else {
		printf("レコードの削除に失敗しました。\n");
	}
}

/*
* showTables -- show tablesが入力されたときに、データベース内のテーブルを表示
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

	/*showの次のトークンを読み込み、それが"tables"かどうかをチェック */
	token = getNextToken();
	if (token == NULL || strcmp(token, "tables") != 0) {
		/* 文法エラー */
		printf("入力行に間違いがあります。:%s\n", token);
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
* test1 -- rundomにinsertしまくる
* 1.先に乱数をつくる関数(initializeRandomGeneratorとgetRandomInteger)を用意
* 2.test1()を実行する
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
	/*乱数を初期化して発生させる*/
	srand((unsigned int)time(NULL));
	/*100個のレコードを挿入する*/
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
* main -- マイクロDBシステムのエントリポイント
*/
int main()
{
	char input[MAX_INPUT];
	char *token;

	/* ファイルモジュールの初期化 */
	if (initializeFileModule() != OK) {
		fprintf(stderr, "Cannot initialize file module.\n");
		exit(1);
	}

	/* データ定義ジュールの初期化 */
	if (initializeDataDefModule() != OK) {
		fprintf(stderr, "Cannot initialize data definition module.\n");
		exit(1);
	}

	/* データ操作ジュールの初期化 */
	if (initializeDataManipModule() != OK) {
		fprintf(stderr, "Cannot initialize data manipulation module.\n");
		exit(1);
	}

	/* ウェルカムメッセージを出力 */
	printf("マイクロDBMSを起動しました。\n");

	/* 1行ずつ入力を読み込みながら、処理を行う */
	for (;;) {
		/* プロンプトの出力 */
		printf("\nDDLまたはDMLを入力してください。\n");
		printf("> ");

		/* キーボード入力を1行読み込む */
		fgets(input, MAX_INPUT, stdin);

		/* 入力の最後の改行を取り除く */
		if (strchr(input, '\n') != NULL) {
			*(strchr(input, '\n')) = '\0';
		}

		/* 字句解析するために入力文字列を設定する */
		setInputString(input);

		/* 最初のトークンを取り出す */
		token = getNextToken();

		/* 入力が空行だったら、ループの先頭に戻ってやり直し */
		if (token == NULL) {
			continue;
		}

		/* 入力が"quit"だったら、ループを抜けてプログラムを終了させる */
		if (strcmp(token, "quit") == 0) {
			printf("マイクロDBMSを終了します。\n\n");
			break;
		}

		/* 最初のトークンが何かによって、呼び出す関数を決める */
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
			/* 入力に間違いがあった */
			printf("入力に間違いがあります。:%s\n", token);
			printf("もう一度入力し直してください。\n\n");
		}
	}

	/* 各モジュールの終了処理 */
	finalizeDataManipModule();
	finalizeDataDefModule();
	finalizeFileModule();
}