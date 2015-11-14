/*file.c ファイルアクセスモジュール*/

/*ヘッダファイル読み込み*/
#include "microdb.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define NUM_BUFFER 10 /*ファイルアクセスモジュールが管理するバッファの大きさ*/

typedef enum { UNMODIFIED = 0, MODIFIED = 1 } modifiFlag; /*変更フラグ*/

/*
* Buffer -- 1ページ分のバッファを記憶する構造体
*/
typedef struct Buffer Buffer;
struct Buffer{
	File *file;				/*バッファの内容が格納されたファイル*/
	int pageNum;			/*ページ番号*/
	char page[PAGE_SIZE];	/*ページの内容を格納する配列*/
	struct Buffer *prev;	/*1つ前のバッファへのポインタ*/
	struct Buffer *next;	/*1つ後ろのバッファへのポインタ*/
	modifiFlag modified;	/*ページの内容が更新されたかどうかを示すフラグ*/
};

/*
* bufferListHead -- LRUリストの先頭へのポインタ (NULLにしておく)
*/
static Buffer *bufferListHead = NULL;

/*
* bufferListTail -- LRUリストの最後へのポインタ (NULLにしておく)
*/
static Buffer *bufferListTail = NULL;

/*
* initializeBufferList --バッファリストの初期化
* initializeFileModule() で一回だけ呼び出す
* 初期化に成功すればOK　失敗すればNGを返す
*/
static Result initializeBufferList(){
	Buffer *oldBuf = NULL;
	Buffer *buf;
	int i;

	/*NUM_BUFFER分のバッファを用意して、ポインタをつなげて両方向リストにする*/
	for (i = 0; i < NUM_BUFFER; i++){
		/*1個分のバッファをマロック*/
		if ((buf = (Buffer *)malloc(sizeof(Buffer))) == NULL){
			/*error*/
			fprintf(stderr, "Cannot Buffer malloc.\n");
			return NG;
		}

		/*Buffer構造体の初期化*/
		buf->file = NULL;
		buf->pageNum = -1;
		buf->modified = UNMODIFIED;
		memset(buf->page, 0, PAGE_SIZE);
		buf->prev = NULL;
		buf->next = NULL;

		/*両方向リストをつくる*/
		if (oldBuf != NULL)
		{
			oldBuf->next = buf;
		}
		buf->prev = oldBuf;

		/*リストの一番最初の要素のポインタを保存*/
		if (buf->prev == NULL){
			bufferListHead = buf;
		}
		if (i == NUM_BUFFER - 1){
			bufferListTail = buf;
		}

		oldBuf = buf;
	}
	return OK;
}

/*
* finalizeBufferList --バッファリストの終了処理
* finalizeFileModule() で一回だけ呼び出す
* メモリの解放に成功すればOK　失敗すればNGを返す
*/
static Result finalizeBufferList(){
	Buffer *p;
	Buffer *q;
	/*確保したメモリの解放*/
	for (p = bufferListHead; p != NULL;){
		q = p;
		p = p->next;
		free(q);
	}
	return OK;
}

/*バッファをリストの先頭へ移動させる関数*/
static void moveBufferToListHead(Buffer *buf){
	Buffer *tmpBuffer;	/*バッファへのポインタの一時的保存*/

	/* 1.bufの一つ前のバッファのnextをbufの一つ後のバッファに指定*/
	if (buf->prev != NULL){
		tmpBuffer = buf->prev;
		tmpBuffer->next = buf->next;
	}
	else{		/*bufが先頭なら何もしない*/
		return;
	}

	/* 2.bufの1つあとのバッファのprevをbufの一つ前のバッファに指定*/
	if (buf->next != NULL){
		tmpBuffer = buf->next;
		tmpBuffer->prev = buf->prev;
	}
	else{	/*bufが最後なら一つ前をtailに*/
		bufferListTail = buf->prev;
	}

	/* 3.bufのprevとnextをNULLに*/
	buf->next = NULL;
	buf->prev = NULL;

	/* 4.bufを先頭に*/
	tmpBuffer = bufferListHead;
	buf->next = tmpBuffer;
	tmpBuffer->prev = buf;
	bufferListHead = buf;
}

/*ファイルアクセスモジュールの初期化処理*/
Result initializeFileModule(){
	if (initializeBufferList() == NG){
		return NG;
	}
	return OK;	//成功の場合OKを返す
}

/*ファイルアクセスモジュールの終了処理*/
Result finalizeFileModule(){
	if (finalizeBufferList() == NG){
		return NG;
	}
	return OK;	//成功の場合OKを返す
}

/*ファイルの作成*/
Result createFile(char *filename){
	int fd;	//ファイルディスクリプタ
	if ((fd = creat(filename, S_IREAD | S_IWRITE)) == -1){	//読み込みと書き込みの権利を与えてファイルを作成
		return NG;
	}

	return OK;
}

/*ファイルの削除*/
Result deleteFile(char *filename){
	if (unlink(filename) == -1){	//ファイルを削除する
		perror("unlink");
		return NG;
	}
	return OK;
}

/*ファイルオープン*/
File *openFile(char *filename){
	File *file;
	int fd;

	/*ファイル構造体の用意*/
	file = malloc(sizeof(File));
	if (file == NULL){
		return NULL;
	}

	/*ファイルをオープンする*/
	if ((fd = open(filename, O_RDWR)) == -1){
		return NULL;
	}

	/*ファイル構造体へ情報の保存*/
	file->desc = fd;
	strcpy(file->name, filename);

	return file;
}

/*ファイルクローズ*/
Result closeFile(File *file){
	Buffer *buf;			/*走査用*/
	/*バッファに空きがないかと、要求されたページがあるかどうか調べる*/
	for (buf = bufferListHead; buf != NULL; buf = buf->next){
		/*変更フラグが立っていたら内容を書き戻す*/
		if (buf->modified == MODIFIED){
			/*書き出し位置の移動*/
			if (lseek(buf->file->desc, (buf->pageNum)*PAGE_SIZE, SEEK_SET) == -1){
				return NG;
			}

			/*書き込む*/
			if (_write(buf->file->desc, buf->page, PAGE_SIZE) < PAGE_SIZE){
				return NG;
			}

			/*変更フラグを0に*/
			buf->modified = UNMODIFIED;
		}
		buf->file = NULL;
	}
	close(file->desc);

	/*File構造体の解放*/
	free(file);

	return OK;
}

/*1ページ分のデータのファイルへの読み出し*/
Result readPage(File *file, int pageNum, char *page){
	Buffer *buf;			/*走査用*/
	Buffer *emptyBuf;		/*空きバッファ保存用*/
	emptyBuf = NULL;
	int x;

	/*バッファに空きがないかと、要求されたページがあるかどうか調べる*/
	for (buf = bufferListHead; buf != NULL; buf = buf->next){
		/*もしバッファに空きが見つかったらbreak*/
		if (buf->file == NULL){
			emptyBuf = buf;
			break;
		}
		if (buf->file == file && buf->pageNum == pageNum){
			/*要求されたページがバッファにあったので、その内容を引数のページにコピーする*/
			memcpy(page, buf->page, PAGE_SIZE);

			/*アクセスされたバッファをリストの先頭に移動させる*/
			moveBufferToListHead(buf);

			/*OKを返す*/
			return OK;
		}
	}

	/*空きがなかったら一番最後のバッファを空ける*/
	if (emptyBuf == NULL){
		buf = bufferListTail;
		/*変更フラグが立っていたら内容を書き戻す*/
		if (buf->modified == MODIFIED){
			/*書き出し位置の移動*/
			if (_lseek(buf->file->desc, (buf->pageNum)*PAGE_SIZE, SEEK_SET) == -1){
				return NG;
			}

			/*書き込む*/
			if (_write(buf->file->desc, buf->page, PAGE_SIZE) < PAGE_SIZE){
				return NG;
			}

			/*変更フラグを0に*/
			buf->modified = UNMODIFIED;
		}
		/*最後のバッファを空け、そこに新たなページの内容を読み込む*/
		emptyBuf = buf;
	}

	/*空きバッファに引数で指定されたファイルの
	　ページの内容を保存する*/
	/*読み出し位置の移動*/
	if (lseek(file->desc, pageNum * PAGE_SIZE, SEEK_SET) == -1) {
		return NG;
	}
	/*readシステムコールによるファイルへのアクセス*/
	if (x = read(file->desc, emptyBuf->page, PAGE_SIZE) < PAGE_SIZE){
		return NG;
	}
	/*バッファの内容を引数のpageにコピー*/
	memcpy(page, emptyBuf->page, PAGE_SIZE);

	/*Buffer構造体に各種情報の保存*/
	emptyBuf->file = file;
	emptyBuf->pageNum = pageNum;

	/*アクセスされたバッファをリストの先頭に移動させる*/
	moveBufferToListHead(emptyBuf);
	return OK;
}

/*1ページ分のデータのファイルへの書き出し*/
Result writePage(File *file, int pageNum, char *page){
	Buffer *buf;			/*走査用*/
	Buffer *emptyBuf;		/*空きバッファ保存用*/
	emptyBuf = NULL;
	/*バッファに空きがないかと、要求されたページがあるかどうか調べる*/
	for (buf = bufferListHead; buf != NULL; buf = buf->next){
		/*もしバッファに空きが見つかったらbreak*/
		if (buf->file == NULL){
			emptyBuf = buf;
			break;
		}
		if (buf->file == file && buf->pageNum == pageNum){
			/*要求されたページがバッファにあったので、バッファにコピーする*/
			memcpy(buf->page, page, PAGE_SIZE);

			/*変更フラグを1に*/
			buf->modified = MODIFIED;
			/*アクセスされたバッファをりすとの先頭に移動させる*/
			moveBufferToListHead(buf);

			/*OKを返す*/
			return OK;
		}
	}

	/*空きがなかったら一番最後のバッファを空ける*/
	if (emptyBuf == NULL){
		buf = bufferListTail;
		/*変更フラグが立っていたら内容を書き戻す*/
		if (buf->modified == MODIFIED){
			/*書き出し位置の移動*/
			if (lseek(buf->file->desc, (buf->pageNum)*PAGE_SIZE, SEEK_SET) == -1){
				return NG;
			}

			/*書き込む*/
			if (write(buf->file->desc, buf->page, PAGE_SIZE) < PAGE_SIZE){
				return NG;
			}

			/*変更フラグを0に*/
			buf->modified = UNMODIFIED;
			/*最後のバッファを空ける*/
		}
		emptyBuf = buf;
	}

	/*引数のpageを、バッファにコピーする*/
	memcpy(emptyBuf->page, page, PAGE_SIZE);

	/*Buffer構造体に各種情報の保存*/
	emptyBuf->file = file;
	emptyBuf->pageNum = pageNum;
	/*変更フラグを1に*/
	emptyBuf->modified = MODIFIED;

	/*アクセスされたバッファをリストの先頭に移動させる*/
	moveBufferToListHead(emptyBuf);
	return OK;
}

/*ファイルのページ数の取得*/
int getNumPages(char *filename){
	struct stat stat_buf;
	if (stat(filename, &stat_buf) == -1){
		return -1;
	}
	else{
		return (stat_buf.st_size) / PAGE_SIZE;	//ファイルサイズをページサイズで割ったものを返す
	}
}

/*バッファリスト内容の出力（テスト用）*/
void printBufferList(){
	Buffer *buf;

	printf("Buffer List:");

	/* それぞれのバッファの最初の3バイトだけを出力する*/
	for (buf = bufferListHead; buf != NULL; buf = buf->next){
		if (buf->file == NULL){
			printf("(empty)");
		}
		else{
			printf("	%c%c%c", buf->page[0], buf->page[1], buf->page[2]);
		}
	}
	printf("\n");
}