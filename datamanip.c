/*
* datamanip.c -- データ操作モジュール
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "microdb.h"

/*
* DATA_FILE_EXT -- データファイルの拡張子
*/
#define DATA_FILE_EXT ".dat"

/*
* initializeDataManipModule -- データ操作モジュールの初期化
*/
Result initializeDataManipModule()
{
	return OK;
}

/*
* finalizeDataManipModule -- データ操作モジュールの終了処理
*/
Result finalizeDataManipModule()
{
	return OK;
}

/*
* getRecordSize -- 1レコード分の保存に必要なバイト数の計算
*
* 引数:
*	tableInfo: データ定義情報を収めた構造体
*
* 返り値:
*	tableInfoのテーブルに収められた1つのレコードを保存するのに
*	必要なバイト数
*/
static int getRecordSize(TableInfo *tableInfo)
{
	int total = 0;
	int i;

	for (i = 0; i < (tableInfo->numField); i++) {
		///* i番目のフィールドがINT型かSTRING型か調べる */
		switch (tableInfo->fieldInfo[i].dataType) {
		/* INT型ならtotalにsizeof(int)を加算 */
		case TYPE_INTEGER:
			total += sizeof(int);
			break;
		/* STRING型ならtotalにMAX_STRINGを加算 */
		case TYPE_STRING:
			total += MAX_STRING;
			break;
		/* その他の場合(ここにくることはないはず)*/
		default:
			break;
		}
	}

	/* フラグの分の1を足す */
	total++;

	return total;
}

/*
* insertRecord -- レコードの挿入
*
* 引数:
*	tableName: レコードを挿入するテーブルの名前
*	recordData: 挿入するレコードのデータ
*
* 返り値:
*	挿入に成功したらOK、失敗したらNGを返す
*/
Result insertRecord(char *tableName, RecordData *recordData)
{
	TableInfo *tableInfo;	/*TableInfoを格納するポインタ変数*/
	int recordSize;	/*レコードサイズを格納する変数*/
	int numPage;	/*ページサイズを格納する変数*/
	int i,j;			/*繰り返し用の変数*/
	char *record;	/*レコード情報を格納するポインタ変数*/
	char *p;	/*recordを操作するためのポインタ変数*/
	char *q;	/*pageを操作するためのポインタ変数*/
	char *filename;	/*ファイルを操作するときのファイル名を格納するポインタ変数*/
	int len;	/*文字列の長さを格納するポインタ変数*/
	File *file;	/*ファイル情報を格納するポインタ変数*/
	char page[PAGE_SIZE];	/*ファイルからの読み出しに使う配列*/
	
	/* テーブルの情報を取得する */
	if ((tableInfo = getTableInfo(tableName)) == NULL) {
		/* エラー処理 */
		return NG;
	}

	/* 1レコード分のデータをファイルに収めるのに必要なバイト数を計算する */
	recordSize = getRecordSize(tableInfo);

	/* 必要なバイト数分のメモリを確保する */
	if ((record = malloc(recordSize)) == NULL) {
		/* エラー処理 */
		return NG;
	}
	p = record;

	/* 先頭に、「使用中」を意味するフラグを立てる */
	memset(p, 1, 1);
	p += 1;

	/* 確保したメモリ領域に、フィールド数分だけ、順次データを埋め込む */
	for (i = 0; i < tableInfo->numField; i++) {
		switch (tableInfo->fieldInfo[i].dataType) {
		case TYPE_INTEGER:
			memcpy(p, &(recordData->fieldData[i].intValue), sizeof(int));	/*int型データ書き込み*/
			p += sizeof(int);												/*ポインタずらす*/
			break;
		case TYPE_STRING:
			memcpy(p, &(recordData->fieldData[i].stringValue),MAX_STRING);	/*string型データ書き込み*/
			p += MAX_STRING;												/*ポインタずらす*/
			break;
		default:
			/* ここにくることはないはず */
			freeTableInfo(tableInfo);
			free(record);
			return NG;
		}
	}

	/* 使用済みのtableInfoデータのメモリを解放する */
	freeTableInfo(tableInfo);
	/*
	* ここまでで、挿入するレコードの情報を埋め込んだバイト列recordができあがる
	*/

	/* データファイルをオープンする *
	/*ファイル名の作成*/
	len = strlen(tableName) + strlen(DATA_FILE_EXT) + 1;	/*文字列の長さを調べる*/
	if ((filename = malloc(len)) == NULL){					/*mallocでlen分の領域を確保*/
		return NG;
	}
	snprintf(filename, len, "%s%s", tableName, DATA_FILE_EXT); /*snprintfによって文字列を連結*/
	/*名前が[filename]のファイルをオープンする*/
	if ((file = openFile(filename)) == NULL) {
		fprintf(stderr, "Cannot open file.\n");
		return NG;
	}

	/* データファイルのページ数を調べる */
	numPage = getNumPages(filename);

	/* レコードを挿入できる場所を探す */
	for (i = 0; i < numPage; i++) {
		/* 1ページ分のデータを読み込む */
		if (readPage(file, i, page) != OK) {
			free(record);
			return NG;
		}

		/* pageの先頭からrecordSizeバイトずつ飛びながら、先頭のフラグが「0」(未使用)の場所を探す */
		for (j = 0; j < (PAGE_SIZE / recordSize); j++) {
			q = &page[j*recordSize];
			if (*q == 0) {	/*見つかったら*/
				/* 見つけた空き領域に上で用意したバイト列recordを埋め込む */
				memcpy(q, record, recordSize);

				/* ファイルに書き戻す */
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
	* ファイルの最後まで探しても未使用の場所が見つからなかったら
	* ファイルの最後に新しく空のページを用意し、そこに書き込む
	*/
	/*ページの内容をクリアする*/
	memset(page, 0, PAGE_SIZE);		/*新しいページを0で埋める*/
	q = page;						
	memcpy(q, record, recordSize);		/*ページの先頭にバイト列recordを埋め込む*/
	
	/*ファイルの最後のページに1ページ分のデータを書き込む*/
	if (writePage(file, numPage, page) != OK) {
		fprintf(stderr, "Cannot write page.\n");
		return NG;
	}

	/*データファイルをクローズする*/
	closeFile(file);
	free(record);
	return OK;
}



/*
* checkCondition -- レコードが条件を満足するかどうかのチェック
*
* 引数:
*	recordData: チェックするレコード
*	condition: チェックする条件
*
* 返り値:
*	レコードrecordが条件conditionを満足すればOK、満足しなければNGを返す
*/
static Result checkCondition(RecordData *recordData, Condition *condition)
{
	int i;

	/* conditionに指定されたフィールド名があるか探す*/
	for (i = 0; i < recordData->numField; i++){
		/*フィールド名があるかチェック*/
		if (strcmp(recordData->fieldData[i].name,condition->name)==0){
			/* データ型による場合分け */
			switch (recordData->fieldData[i].dataType){
			case TYPE_INTEGER:
				/* 比較演算子による場合分け */
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
				/* 比較演算子による場合分け */
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
* selectRecord -- レコードの検索
*
* 引数:
*	tableName: レコードを検索するテーブルの名前
*	condition: 検索するレコードの条件
*
* 返り値:
*	検索に成功したら検索されたレコード(の集合)へのポインタを返し、
*	検索に失敗したらNULLを返す。
*	検索した結果、該当するレコードが1つもなかった場合も、レコードの
*	集合へのポインタを返す。
*
* ***注意***
*	この関数が返すレコードの集合を収めたメモリ領域は、不要になったら
*	必ずfreeRecordSetで解放すること。
*/
RecordSet *selectRecord(char *tableName, Condition *condition)
{
	RecordSet *recordSet;	/*RecordSet構造体の用意*/
	char *filename;	/*ファイルを操作するときのファイル名を格納するポインタ変数*/
	int len;	/*文字列の長さを格納するポインタ変数*/
	File *file;	/*ファイル情報を格納するポインタ変数*/
	int numPage;	/*ページサイズを格納する変数*/
	int i, j, k;			/*繰り返し用の変数*/
	int fieldValueEqualFlag;	/*重複を許さない場合のfieldの値が同じであるかを調べるフラグ*/
	char page[PAGE_SIZE];	/*ファイルからの読み出しに使う配列*/
	TableInfo *tableInfo;	/*TableInfoを格納するポインタ変数*/
	int recordSize;	/*レコードサイズを格納する変数*/
	RecordData *recordData;	/*レコードデータを格納する構造体*/
	RecordData *p;	/*recordSetを走査するためのポインタ*/
	char *q;	/*pageを操作するためのポインタ変数*/

	/* テーブルの情報を取得する */
	if ((tableInfo = getTableInfo(tableName)) == NULL) {
		/* エラー処理 */
		return NULL;
	}

	/* 1レコード分のデータをファイルに収めるのに必要なバイト数を計算する */
	recordSize = getRecordSize(tableInfo);


	/*RecordSet構造体のメモリを確保*/
	if ((recordSet = (RecordSet*)malloc(sizeof(RecordSet))) == NULL){
		fprintf(stderr, "Malloc error.\n");
		return NULL;
	}
	recordSet->numRecord = 0;

	/*ファイル名の作成*/
	len = strlen(tableName) + strlen(DATA_FILE_EXT) + 1;	/*文字列の長さを調べる*/
	if ((filename = malloc(len)) == NULL){					/*mallocでlen分の領域を確保*/
		return NULL;
	}
	snprintf(filename, len, "%s%s", tableName, DATA_FILE_EXT); /*snprintfによって文字列を連結*/
	/*名前が[filename]のデータファイルをオープンする*/
	if ((file = openFile(filename)) == NULL) {
		fprintf(stderr, "Cannot open file.\n");
		return NULL;
	}

	/*データファイルのページ数を取得*/
	numPage = getNumPages(filename);

	/*データファイルのページ数分だけ繰り返し*/
	for (i = 0; i < numPage; i++){
		/* 1ページ分のデータを読み込む */
		if (readPage(file, i, page) != OK) {
			return NULL;
		}
		/* pageの先頭からrecordSizeバイトずつ飛びながら、先頭のフラグが「1」(使用中)の場所を探す */
		for (j = 0; j < (PAGE_SIZE / recordSize); j++) {
			q = &page[j*recordSize];
			if (*q == 1) {	/*見つかったら*/
				/* 先頭は、「使用中」を意味するフラグなのでとばす */
				q += 1;
				/*RecordData構造体のメモリを確保*/
				if ((recordData = (RecordData *)malloc(sizeof(RecordData))) == NULL){
					fprintf(stderr, "Malloc error.\n");
					return NULL;
				}
				/* レコードデータ構造体にフィールド数分だけ、順次データを埋め込む */
				recordData->numField = tableInfo->numField;
				recordData->next = NULL;	/*リンクを初期化*/
				for (k = 0; k < tableInfo->numField; k++) {
					strcpy(recordData->fieldData[k].name, tableInfo->fieldInfo[k].name);	/*nameの保存*/
					recordData->fieldData[k].dataType = tableInfo->fieldInfo[k].dataType;		/*dataTypeの保存*/
					switch (tableInfo->fieldInfo[k].dataType) {
					case TYPE_INTEGER:
						memcpy(&recordData->fieldData[k].intValue,q, sizeof(recordData->fieldData[k].intValue));	/*int型データ書き込み*/
						q += sizeof(recordData->fieldData[k].intValue);												/*ポインタずらす*/
						break;
					case TYPE_STRING:
						memcpy(&recordData->fieldData[k].stringValue,q, sizeof(recordData->fieldData[k].stringValue));	/*string型データ書き込み*/
						q += sizeof(recordData->fieldData[k].stringValue);												/*ポインタずらす*/
						break;
					default:
						/* ここにくることはないはず */
						free(recordData);
						return NULL;
					}
				}
				/*重複を許さない場合*/
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
								/* ここにくることはないはず */
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
				/*条件を満足するかしらべる*/
				if (checkCondition(recordData, condition) == OK){		/*recordDataが検索条件に当てはまったとき*/
					if (recordSet->numRecord==0){						/*recordSetが空のとき*/
						recordSet->head = recordSet->tail = recordData;	/*headがrecordDataを指すようにする*/
					}
					else{												/*recordSetが空じゃないとき*/
						recordSet->tail->next =recordData;				/*tailのnextがrecordDataを指すようにする*/
						recordSet->tail = recordData;					/*tailがrecordDataを指すようにする*/
					}
					recordSet->numRecord++;
				}
			}
		}

	}

	/*データファイルをクローズする*/
	freeTableInfo(tableInfo);
	closeFile(file);
	return recordSet;
}

/*
* freeRecordSet -- レコード集合の情報を収めたメモリ領域の解放
*
* 引数:
*	recordSet: 解放するメモリ領域
*
* 返り値:
*	なし
*
* ***注意***
*	関数selectRecordが返すレコードの集合を収めたメモリ領域は、
*	不要になったら必ずこの関数で解放すること。
*/
void freeRecordSet(RecordSet *recordSet)
{
	free(recordSet);
}

/*
* deleteRecord -- レコードの削除
*
* 引数:
*	tableName: レコードを削除するテーブルの名前
*	condition: 削除するレコードの条件
*
* 返り値:
*	削除に成功したらOK、失敗したらNGを返す
*/
Result deleteRecord(char *tableName, Condition *condition)
{
	char *filename;	/*ファイルを操作するときのファイル名を格納するポインタ変数*/
	int len;	/*文字列の長さを格納するポインタ変数*/
	File *file;	/*ファイル情報を格納するポインタ変数*/
	int numPage;	/*ページサイズを格納する変数*/
	int i, j, k;			/*繰り返し用の変数*/
	char page[PAGE_SIZE];	/*ファイルからの読み出しに使う配列*/
	TableInfo *tableInfo;	/*TableInfoを格納するポインタ変数*/
	int recordSize;	/*レコードサイズを格納する変数*/
	RecordData *recordData;	/*レコードデータを格納する構造体*/
	char *q;	/*pageを操作するためのポインタ変数*/

	/* テーブルの情報を取得する */
	if ((tableInfo = getTableInfo(tableName)) == NULL) {
		/* エラー処理 */
		return NG;
	}

	/* 1レコード分のデータをファイルに収めるのに必要なバイト数を計算する */
	recordSize = getRecordSize(tableInfo);

	/*RecordDara構造体のメモリを確保*/
	if ((recordData = (RecordData *)malloc(sizeof(RecordData))) == NULL){
		fprintf(stderr, "Malloc error.\n");
		return NG;
	}

	/*ファイル名の作成*/
	len = strlen(tableName) + strlen(DATA_FILE_EXT) + 1;	/*文字列の長さを調べる*/
	if ((filename = malloc(len)) == NULL){					/*mallocでlen分の領域を確保*/
		return NG;
	}
	snprintf(filename, len, "%s%s", tableName, DATA_FILE_EXT); /*snprintfによって文字列を連結*/
	/*名前が[filename]のデータファイルをオープンする*/
	if ((file = openFile(filename)) == NULL) {
		fprintf(stderr, "Cannot open file.\n");
		return NG;
	}

	/*データファイルのページ数を取得*/
	numPage = getNumPages(filename);

	/*データファイルのページ数分だけ繰り返し*/
	for (i = 0; i < numPage; i++){
		/* 1ページ分のデータを読み込む */
		if (readPage(file, i, page) != OK) {
			return NG;
		}
		/* pageの先頭からrecordSizeバイトずつ飛びながら、先頭のフラグが「1」(使用中)の場所を探す */
		for (j = 0; j < (PAGE_SIZE / recordSize); j++) {
			q = &page[j*recordSize];
			if (*q == 1) {	/*見つかったら*/
				/* 先頭は、「使用中」を意味するフラグなのでとばす */
				q += 1;

				/* レコードデータ構造体にフィールド数分だけ、順次データを埋め込む */
				recordData->numField = tableInfo->numField;
				for (k = 0; k < tableInfo->numField; k++) {
					strcpy(recordData->fieldData[k].name, tableInfo->fieldInfo[k].name);
					recordData->fieldData[k].dataType=tableInfo->fieldInfo[k].dataType;
					switch (tableInfo->fieldInfo[k].dataType) {
					case TYPE_INTEGER:
						memcpy(&recordData->fieldData[k].intValue, q, sizeof(int));	/*int型データ書き込み*/
						q += sizeof(int);												/*ポインタずらす*/
						break;
					case TYPE_STRING:
						memcpy(&recordData->fieldData[k].stringValue, q, MAX_STRING);	/*string型データ書き込み*/
						q += MAX_STRING;												/*ポインタずらす*/
						break;
					default:
						/* ここにくることはないはず */
						free(recordData);
						return NG;
					}
				}
				/*条件を満足するかしらべる*/
				if (checkCondition(recordData, condition) == OK){	/*recordDataが検索条件に当てはまったとき*/
					/*レコードを削除する*/
					page[j*recordSize] = 0;
				}
			}
			/*削除して変更されたページの内容を書き換える*/
			if (writePage(file, i, page) != OK){
				fprintf(stderr, "Cannot write page.\n");
				return NG;
			}
		}

	}

	/*データファイルをクローズする*/
	free(recordData);
	freeTableInfo(tableInfo);
	closeFile(file);
	return OK;
}

/*
* createDataFile -- データファイルの作成
*
* 引数:
*	tableName: 作成するテーブルの名前
*
* 返り値:
*	作成に成功したらOK、失敗したらNGを返す
*/
Result createDataFile(char *tableName)
{
	char *filename;	/*ファイルを操作するときのファイル名を格納するポインタ変数*/
	int len;	/*文字列の長さを格納するポインタ変数*/
	File *file;
	char page[PAGE_SIZE];
	char *p;

	/*ページの内容をクリアする*/
	memset(page, 0, PAGE_SIZE);
	p = page;
	
	/*ファイル名の作成*/
	len = strlen(tableName) + strlen(DATA_FILE_EXT) + 1;
	if ((filename = malloc(len)) == NULL){
		return NG;
	}
	snprintf(filename, len, "%s%s", tableName, DATA_FILE_EXT);

	/*ファイルの作成*/
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

	return OK;
}

/*
* deleteDataFile -- データファイルの削除
*
* 引数:
*	tableName: 削除するテーブルの名前
*
* 返り値:
*	削除に成功したらOK、失敗したらNGを返す
*/
Result deleteDataFile(char *tableName)
{
	/*ファイル名の作成*/
	char *filename;
	int len;
	len = strlen(tableName) + strlen(DATA_FILE_EXT) + 1;
	if ((filename = malloc(len)) == NULL){
		return NG;
	}
	snprintf(filename, len, "%s%s", tableName, DATA_FILE_EXT);

	/*テーブルの削除*/
	if (deleteFile(filename) == NG) {
		fprintf(stderr, "Cannot delete file.\n");
		return NG;
	}
	return OK;
}

/*
* printTableData -- 全てのデータの表示
*
* 引数:
*   tableName: データを表示するテーブルの名前
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

	/* テーブルのデータ定義情報を取得 */
	if ((tableInfo = getTableInfo(tableName))==NULL){
		return;
	}

	/*1レコード分のバイト数を取得*/
	recordSize = getRecordSize(tableInfo);

	/*メモリ領域の確保*/
	len = strlen(tableName) + strlen(DATA_FILE_EXT) + 1;
	if ((filename = malloc(len)) == NULL){
		freeTableInfo(tableInfo);
		return;
	}

	/*ファイル名作成*/
	snprintf(filename, len, "%s%s", tableName, DATA_FILE_EXT);

	/*データファイルのページ数取得*/
	numPage = getNumPages(filename);

	/*データファイルをオープンする*/
	if ((file = openFile(filename))==NULL){
		free(filename);
		freeTableInfo(tableInfo);
		return;
	}

	free(filename);
	
	/*上から順番に描画していく*/		/* +----------+---------*/
	for (i = 0; i < tableInfo->numField; i++){
		printf("%c%s", plus, hyphen);
	}
	printf("%c\n",plus);

	/*フィールド名を表示*/				/*|name		|name		*/
	for (i = 0; i < tableInfo->numField; i++){
		printf("%c %-14s ", vertical, tableInfo->fieldInfo[i].name);
	}
	printf("%c\n", vertical);
	
	/*囲む*/		/* +----------+---------*/
	for (i = 0; i < tableInfo->numField; i++){
		printf("%c%s", plus, hyphen);
	}
	printf("%c\n", plus);

	/* レコードを1つずつ取り出し、表示する*/
	for (i = 0; i < numPage; i++){
		readPage(file, i, page);

		for (j = 0; j < (PAGE_SIZE / recordSize); j++){
			/*先頭のフラグが0の場合は読み飛ばす*/
			p = &page[recordSize * j];
			if (*p == 0){
				continue;
			}
			/*1文字目はフラグなので読み飛ばす*/
			p++;

			/*1レコード分を表示*/
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
	/*囲む*/		/* +----------+---------*/
	for (i = 0; i < tableInfo->numField; i++){
		printf("%c%s", plus, hyphen);
	}
	printf("%c\n", plus);

	freeTableInfo(tableInfo);
}


/*
* printRecordSet -- レコード集合の表示
*
* 引数:
*	recordSet: 表示するレコード集合
*
* 返り値:
*	なし
*/
void printRecordSet(RecordSet *recordSet,char receiveFieldName[MAX_FIELD][MAX_FIELD_NAME],int numField)
{
	int i,j;
	RecordData *p;	/*リストにアクセスするためのポインタ*/
	char plus = '+';
	char *hyphen = "----------------";
	char vertical = '|';
	printf("Number of Records: %d\n", recordSet->numRecord);
	
	/*上から順番に描画していく*/		/* +----------+---------*/
	printf("%c", plus);
	for (i = 0; i < numField; i++){
			printf("%s%c", hyphen, plus);
	}
	printf("\n");
	
	/*フィールド名を表示*/				/*|name		|name		*/	
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