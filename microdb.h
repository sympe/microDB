/* microdb.h - 共通定義ファイル */

/* Result -成功or失敗を返す- */
typedef enum {OK, NG} Result;

/* PAGE_SIZE -- ファイルアクセスの単位 */
#define PAGE_SIZE 4096

/* MAX_FILENAME -- オープンするファイル名の長さの上限*/
#define MAX_FILENAME 256

/* MAX_FIELD -- 1レコードに含まれるフィールド数の上限*/
#define MAX_FIELD 40

/* MAX_FIELD_NAME -- フィールド名の長さの上限(バイト数)*/
#define MAX_FIELD_NAME 20

/* MAX_STRING -- 文字列型データの長さの上限*/
#define MAX_STRING 20

/*snprintfの定義*/
#define snprintf _snprintf

/* DataType -- データベースに保存するデータの型*/
typedef enum DataType DataType;
enum DataType {
	TYPE_UNKNOWN = 0,			/* データ型不明 */
	TYPE_INTEGER = 1,			/* 整数型 */
	TYPE_STRING = 2			/* 文字列型 */
};

/* OpratorType -- 比較演算子を表す列挙型 */
typedef enum OpratorType OperatorType;
enum OperatorType{
	OPR_EQUAL,					/* =  */
	OPR_NOT_EQUAL,				/* != */
	OPR_GREATER_THAN,			/* >  */
	OPR_LESS_THAN,				/* <  */
	OPR_GREATER_THAN_OR_EQUAL,	/* >= */
	OPR_LESS_THAN_OR_EQUAL		/* <= */
};

typedef enum {NOT_DISTINCT = 0,DISTINCT=1} distinctFlag;

/* Condition -- レコードがどのような条件であるかを表現する構造体*/
typedef struct Condition Condition;
struct Condition
{
	char name[MAX_FIELD_NAME];	/*フィールド名*/
	DataType dataType;	/* データタイプ */
	OperatorType operator;	/*比較演算子*/
	int intValue;	/*integer型の場合の値*/
	char stringValue[MAX_STRING];	/*string型の場合の値*/
	distinctFlag distinct;
};

/* FieldInfo -- フィールドの情報を表現する構造体*/
typedef struct FieldInfo FieldInfo;
struct FieldInfo {
	char name[MAX_FIELD_NAME];		/* フィールド名 */
	DataType dataType;			/* フィールドのデータ型 */
};

/* FieldData -- 1つのフィールドデータを表現する構造体*/
typedef struct FieldData FieldData;
struct FieldData
{
	char name[MAX_FIELD_NAME];	//フィールド名
	DataType dataType;	//フィールドのデータ型
	int intValue;	//integer型の場合の値
	char stringValue[MAX_STRING];	//string型の場合の値
};

/* RecordData -- 1つのレコードのデータを表現する構造体 */
typedef struct RecordData RecordData;
struct RecordData
{
	int numField;	//フィールド数
	FieldData fieldData[MAX_FIELD];	//フィールド情報
	RecordData *next;	//次のレコードへのポインタ
};

/* RecordSet -- レコードの集合を表現する構造体 */
typedef struct RecordSet RecordSet;
struct RecordSet
{
	int numRecord;	/* レコード数 */
	RecordData *head; /* レコードのリストの先頭要素へのポインタ */
	RecordData *tail; /*レコードのリストへの最後の要素へのポインタ*/
};

/* File -オープンしたファイルの情報を保持する構造体*/
typedef struct File File;
struct File{
	int desc;	//ファイルディスクリプタ
	char name[MAX_FILENAME];	//ファイル名
};

/* TableInfo -- テーブルの情報を表現する構造体*/
typedef struct TableInfo TableInfo;
struct TableInfo {
	int numField;			/* フィールド数 */
	FieldInfo fieldInfo[MAX_FIELD];		/* フィールド情報の配列 */
};

/*file.cに定義されている関数群*/
extern Result initializeFileModule();	/*ファイルアクセスモジュールの初期化処理*/
extern Result finalizeFileModule();	 /*ファイルアクセスモジュールの終了処理*/
extern Result createFile(char *);	/*ファイルの作成*/
extern Result deleteFile(char *);	/*ファイルの削除*/
extern File *openFile(char *);		/*ファイルオープン*/
extern Result closeFile(File *);	/*ファイルクローズ*/
extern Result readPage(File *, int, char *);	/*1ページ分のデータのファイルへの読み出し*/
extern Result writePage(File *, int, char *);	/*1ページ分のデータのファイルへの書き出し*/
extern int getNumPages(char *);		/*ファイルのページ数の取得*/
extern void printBufferList();		/*バッファリストの表示*/

/*datadef.cに定義されている関数群*/
extern Result initializeDataDefModule();		/*データ定義モジュールの初期化処理*/
extern Result finalizeDataDefModule();			/*データ定義モジュールの終了処理*/
extern Result createTable(char *, TableInfo *);	/*テーブルの作成*/
extern Result dropTable(char *);				/*テーブルの削除*/
extern TableInfo *getTableInfo(char *);			/*テーブル情報の取得*/
extern void freeTableInfo(TableInfo *);			/*データ定義情報を収めたメモリ領域の解放*/

/*datamanip.cに定義されている関数群*/
extern Result initializeDataManipModule();	/*データ操作モジュールの初期化処理*/
extern Result finalizeDataManipModule();	/*データ操作モジュールの終了処理*/
extern Result insertRecord(char *, RecordData *);	/*レコードの作成*/
extern Result deleteRecord(char *, Condition *);	/*レコードの削除*/
extern RecordSet *selectRecord(char *, Condition *);	/*レコードの検索*/
extern void freeRecordSet(RecordSet *);	/*レコード集合を保存したメモリ領域の解放*/
extern Result createDataFile(char *);	/*データファイルの作成*/
extern Result deleteDataFile(char *);	/*データファイルの削除*/
extern void printTableData(char *,int);		/*全てのデータの表示*/
extern void printRecordSet(RecordSet *, char [MAX_FIELD][MAX_FIELD_NAME],int);	/*レコード集合の表示*/