/*
* datadef.c - データ定義モジュール
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "microdb.h"

/*
* DEF_FILE_EXT -- データ定義ファイルの拡張子
*/
#define DEF_FILE_EXT ".def"

/*データ定義モジュールの初期化*/
Result initializeDataDefModule()
{
	return OK;	//成功の場合OKを返す
}

/*データ定義モジュールの終了処理*/
Result finalizeDataDefModule()
{
	return OK;	//成功の場合OKを返す
}

/* 表(テーブル)の作成
*
* 引数:
*	tableName: 作成する表の名前
*	tableInfo: データ定義情報
*
* 返り値:
*	成功ならOK、失敗ならNGを返す
*
* データ定義ファイルの構造(ファイル名: tableName.def)
*   +-------------------+----------------------+-------------------+----
*   |フィールド数       |フィールド名          |データ型           |
*   |(sizeof(int)バイト)|(MAX_FIELD_NAMEバイト)|(sizeof(int)バイト)|
*   +-------------------+----------------------+-------------------+----
* 以降、フィールド名とデータ型が交互に続く。
*/
Result createTable(char *tableName, TableInfo *tableInfo)
{
	File *file;
	char page[PAGE_SIZE];
	char *p;
	int len;
	char *filename;

	/*ページの内容をクリアする*/
	memset(page, 0, PAGE_SIZE);
	p = page;

	/* ページの先頭にフィールド数を保存する*/
	memcpy(p, &(tableInfo->numField), sizeof(tableInfo->numField));	//フィールド数書き込み
	p += sizeof(tableInfo->numField);				//ポインタをずらす

	/*フィールド数の分だけ、配列pageにフィールド名やデータ型をコピーする*/
	for (int i = 0; i < tableInfo->numField; i++){
		memcpy(p, &(tableInfo->fieldInfo[i].name), sizeof(tableInfo->fieldInfo[i].name));	//フィールド名書き込み
		p += sizeof(tableInfo->fieldInfo[i].name);		//ポインタをずらす
		memcpy(p, &(tableInfo->fieldInfo[i].dataType), sizeof(tableInfo->fieldInfo[i].dataType));	//データ型書き込み
		p += sizeof(tableInfo->fieldInfo[i].dataType);		//ポインタをずらす
	}

	
	/*データ定義情報を格納するファイルの作成*/
	/*ファイル名の作成*/
	len = strlen(tableName) + strlen(DEF_FILE_EXT) + 1;
	if ((filename = malloc(len)) == NULL){
		return NG;
	}
	snprintf(filename, len, "%s%s", tableName, DEF_FILE_EXT);

	/*ファイル作成*/
	if (createFile(filename) != OK) {
		fprintf(stderr, "Cannot create file.\n");
		return NG;
	}

	/* ファイルをオープンする */
	if ((file = openFile(filename)) == NULL) {
		fprintf(stderr, "Cannot open file.\n");
		return NG;
	}

	/*ファイルの先頭ページに1ページ分のデータを書き込む*/
	if (writePage(file, getNumPages(filename), page) != OK) {
		fprintf(stderr, "Cannot write page.\n");
		return NG;
	}

	/*ファイルをクローズする*/
	if (closeFile(file) != OK){
		fprintf(stderr, "Cannot close file.\n");
		return NG;
	}
	/*↑ここまでがデータ定義ファイルの作成*/

	/*↓ここからデータファイルの作成*/
	if (createDataFile(tableName) != OK){
		fprintf(stderr, "Cannot create file.\n");
		return NG;
	}

	return OK;
}

/*
* dropTable -- 表(テーブル)の削除
*
*/
Result dropTable(char *tableName)
{
	/*ファイル名の作成*/
	char *filename;
	int len;
	len = strlen(tableName) + strlen(DEF_FILE_EXT) + 1;
	if ((filename = malloc(len)) == NULL){
		return NG;
	}
	snprintf(filename, len, "%s%s", tableName, DEF_FILE_EXT);
	
	/*テーブルの削除*/
	if (deleteFile(filename) == NG) {
		fprintf(stderr, "Cannot delete file.\n");
		return NG;
	}

	/*↓ここからデータファイルの削除*/
	if (deleteDataFile(tableName) != OK){
		fprintf(stderr, "Cannot delete file.\n");
		return NG;
	}
	return OK;
}

/*
* getTableInfo -- 表のデータ定義情報を取得する関数
*
* 引数:
*	tableName: 情報を表示する表の名前
*
* 返り値:
*	tableNameのデータ定義情報を返す
*	エラーの場合には、NULLを返す
*
* ***注意***
*	この関数が返すデータ定義情報を収めたメモリ領域は、不要になったら
*	必ずfreeTableInfoで解放すること。
*/
TableInfo *getTableInfo(char *tableName)
{
	File *file;
	TableInfo *tableInfo;
	char page[PAGE_SIZE];	//ファイルからの読み出しに使う配列
	char *p;
	p = page;
	char *filename;
	int len;

	/*ファイル名の作成*/
	len = strlen(tableName) + strlen(DEF_FILE_EXT) + 1;
	if ((filename = malloc(len)) == NULL){
		return NULL;
	}
	snprintf(filename, len, "%s%s", tableName, DEF_FILE_EXT);
	
	/*TableInfo構造体の用意*/
	tableInfo = malloc(sizeof(TableInfo));
	if (tableInfo == NULL){
		return NULL;
	}

	/* ファイルをオープンする */
	if ((file = openFile(filename)) == NULL) {
		fprintf(stderr, "Cannot open file.\n");
		return NULL;
	}

	/* ファイルの0ページ目から値を読み出す */
	if (readPage(file, 0, page) != OK) {
		fprintf(stderr, "Cannot read page.\n");
		return NULL;
	}

	/*TableInfo構造体に定義情報を保存*/
	memcpy(&tableInfo->numField, p, sizeof(tableInfo->numField));	//フィールド数書き込み
	p += sizeof(tableInfo->numField);

	/*フィールド数の分だけ、配列pageにフィールド名やデータ型をコピーする*/
	for (int i = 0; i < tableInfo->numField; i++){
		memcpy(&(tableInfo->fieldInfo[i].name), p, sizeof(tableInfo->fieldInfo[i].name));	//フィールド名
		p += sizeof(tableInfo->fieldInfo[i].name);
		memcpy(&(tableInfo->fieldInfo[i].dataType), p, sizeof(tableInfo->fieldInfo[i].dataType)); //フィールドのデータ型
		p += sizeof(tableInfo->fieldInfo[i].dataType);
	}

	/*ファイルをクローズする*/
	if (closeFile(file) != OK){
		fprintf(stderr, "Cannot close file.\n");
		return NULL;
	}
	return tableInfo;
}

/*
* freeTableInfo -- データ定義情報を収めたメモリ領域の解放
*
*
* ***注意***
*	関数getTableInfoが返すデータ定義情報を収めたメモリ領域は、
*	不要になったら必ずこの関数で解放すること。
*/
void freeTableInfo(TableInfo *tableInfo)
{
	free(tableInfo);
}

/*
* printTableInfo -- テーブルのデータ定義情報を表示する(動作確認用)
*
* 引数:
*	tableName: 情報を表示するテーブルの名前
*
* 返り値:
*	なし
*/
void printTableInfo(char *tableName)
{
	TableInfo *tableInfo;
	int i;

	/* テーブル名を出力 */
	printf("\nTable %s\n", tableName);

	/* テーブルの定義情報を取得する */
	if ((tableInfo = getTableInfo(tableName)) == NULL) {
		/* テーブル情報の取得に失敗したので、処理をやめて返る */
		return;
	}

	/* フィールド数を出力 */
	printf("number of fields = %d\n", tableInfo->numField);

	/* フィールド情報を読み取って出力 */
	for (i = 0; i < tableInfo->numField; i++) {
		/* フィールド名の出力 */
		printf("  field %d: name = %s, ", i + 1, tableInfo->fieldInfo[i].name);

		/* データ型の出力 */
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

	/* データ定義情報を解放する */
	freeTableInfo(tableInfo);

	return;
}