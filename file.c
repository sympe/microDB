/*file.c �t�@�C���A�N�Z�X���W���[��*/

/*�w�b�_�t�@�C���ǂݍ���*/
#include "microdb.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define NUM_BUFFER 10 /*�t�@�C���A�N�Z�X���W���[�����Ǘ�����o�b�t�@�̑傫��*/

typedef enum { UNMODIFIED = 0, MODIFIED = 1 } modifiFlag; /*�ύX�t���O*/

/*
* Buffer -- 1�y�[�W���̃o�b�t�@���L������\����
*/
typedef struct Buffer Buffer;
struct Buffer{
	File *file;				/*�o�b�t�@�̓��e���i�[���ꂽ�t�@�C��*/
	int pageNum;			/*�y�[�W�ԍ�*/
	char page[PAGE_SIZE];	/*�y�[�W�̓��e���i�[����z��*/
	struct Buffer *prev;	/*1�O�̃o�b�t�@�ւ̃|�C���^*/
	struct Buffer *next;	/*1���̃o�b�t�@�ւ̃|�C���^*/
	modifiFlag modified;	/*�y�[�W�̓��e���X�V���ꂽ���ǂ����������t���O*/
};

/*
* bufferListHead -- LRU���X�g�̐擪�ւ̃|�C���^ (NULL�ɂ��Ă���)
*/
static Buffer *bufferListHead = NULL;

/*
* bufferListTail -- LRU���X�g�̍Ō�ւ̃|�C���^ (NULL�ɂ��Ă���)
*/
static Buffer *bufferListTail = NULL;

/*
* initializeBufferList --�o�b�t�@���X�g�̏�����
* initializeFileModule() �ň�񂾂��Ăяo��
* �������ɐ��������OK�@���s�����NG��Ԃ�
*/
static Result initializeBufferList(){
	Buffer *oldBuf = NULL;
	Buffer *buf;
	int i;

	/*NUM_BUFFER���̃o�b�t�@��p�ӂ��āA�|�C���^���Ȃ��ė��������X�g�ɂ���*/
	for (i = 0; i < NUM_BUFFER; i++){
		/*1���̃o�b�t�@���}���b�N*/
		if ((buf = (Buffer *)malloc(sizeof(Buffer))) == NULL){
			/*error*/
			fprintf(stderr, "Cannot Buffer malloc.\n");
			return NG;
		}

		/*Buffer�\���̂̏�����*/
		buf->file = NULL;
		buf->pageNum = -1;
		buf->modified = UNMODIFIED;
		memset(buf->page, 0, PAGE_SIZE);
		buf->prev = NULL;
		buf->next = NULL;

		/*���������X�g������*/
		if (oldBuf != NULL)
		{
			oldBuf->next = buf;
		}
		buf->prev = oldBuf;

		/*���X�g�̈�ԍŏ��̗v�f�̃|�C���^��ۑ�*/
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
* finalizeBufferList --�o�b�t�@���X�g�̏I������
* finalizeFileModule() �ň�񂾂��Ăяo��
* �������̉���ɐ��������OK�@���s�����NG��Ԃ�
*/
static Result finalizeBufferList(){
	Buffer *p;
	Buffer *q;
	/*�m�ۂ����������̉��*/
	for (p = bufferListHead; p != NULL;){
		q = p;
		p = p->next;
		free(q);
	}
	return OK;
}

/*�o�b�t�@�����X�g�̐擪�ֈړ�������֐�*/
static void moveBufferToListHead(Buffer *buf){
	Buffer *tmpBuffer;	/*�o�b�t�@�ւ̃|�C���^�̈ꎞ�I�ۑ�*/

	/* 1.buf�̈�O�̃o�b�t�@��next��buf�̈��̃o�b�t�@�Ɏw��*/
	if (buf->prev != NULL){
		tmpBuffer = buf->prev;
		tmpBuffer->next = buf->next;
	}
	else{		/*buf���擪�Ȃ牽�����Ȃ�*/
		return;
	}

	/* 2.buf��1���Ƃ̃o�b�t�@��prev��buf�̈�O�̃o�b�t�@�Ɏw��*/
	if (buf->next != NULL){
		tmpBuffer = buf->next;
		tmpBuffer->prev = buf->prev;
	}
	else{	/*buf���Ō�Ȃ��O��tail��*/
		bufferListTail = buf->prev;
	}

	/* 3.buf��prev��next��NULL��*/
	buf->next = NULL;
	buf->prev = NULL;

	/* 4.buf��擪��*/
	tmpBuffer = bufferListHead;
	buf->next = tmpBuffer;
	tmpBuffer->prev = buf;
	bufferListHead = buf;
}

/*�t�@�C���A�N�Z�X���W���[���̏���������*/
Result initializeFileModule(){
	if (initializeBufferList() == NG){
		return NG;
	}
	return OK;	//�����̏ꍇOK��Ԃ�
}

/*�t�@�C���A�N�Z�X���W���[���̏I������*/
Result finalizeFileModule(){
	if (finalizeBufferList() == NG){
		return NG;
	}
	return OK;	//�����̏ꍇOK��Ԃ�
}

/*�t�@�C���̍쐬*/
Result createFile(char *filename){
	int fd;	//�t�@�C���f�B�X�N���v�^
	if ((fd = creat(filename, S_IREAD | S_IWRITE)) == -1){	//�ǂݍ��݂Ə������݂̌�����^���ăt�@�C�����쐬
		return NG;
	}

	return OK;
}

/*�t�@�C���̍폜*/
Result deleteFile(char *filename){
	if (unlink(filename) == -1){	//�t�@�C�����폜����
		perror("unlink");
		return NG;
	}
	return OK;
}

/*�t�@�C���I�[�v��*/
File *openFile(char *filename){
	File *file;
	int fd;

	/*�t�@�C���\���̗̂p��*/
	file = malloc(sizeof(File));
	if (file == NULL){
		return NULL;
	}

	/*�t�@�C�����I�[�v������*/
	if ((fd = open(filename, O_RDWR)) == -1){
		return NULL;
	}

	/*�t�@�C���\���̂֏��̕ۑ�*/
	file->desc = fd;
	strcpy(file->name, filename);

	return file;
}

/*�t�@�C���N���[�Y*/
Result closeFile(File *file){
	Buffer *buf;			/*�����p*/
	/*�o�b�t�@�ɋ󂫂��Ȃ����ƁA�v�����ꂽ�y�[�W�����邩�ǂ������ׂ�*/
	for (buf = bufferListHead; buf != NULL; buf = buf->next){
		/*�ύX�t���O�������Ă�������e�������߂�*/
		if (buf->modified == MODIFIED){
			/*�����o���ʒu�̈ړ�*/
			if (lseek(buf->file->desc, (buf->pageNum)*PAGE_SIZE, SEEK_SET) == -1){
				return NG;
			}

			/*��������*/
			if (_write(buf->file->desc, buf->page, PAGE_SIZE) < PAGE_SIZE){
				return NG;
			}

			/*�ύX�t���O��0��*/
			buf->modified = UNMODIFIED;
		}
		buf->file = NULL;
	}
	close(file->desc);

	/*File�\���̂̉��*/
	free(file);

	return OK;
}

/*1�y�[�W���̃f�[�^�̃t�@�C���ւ̓ǂݏo��*/
Result readPage(File *file, int pageNum, char *page){
	Buffer *buf;			/*�����p*/
	Buffer *emptyBuf;		/*�󂫃o�b�t�@�ۑ��p*/
	emptyBuf = NULL;
	int x;

	/*�o�b�t�@�ɋ󂫂��Ȃ����ƁA�v�����ꂽ�y�[�W�����邩�ǂ������ׂ�*/
	for (buf = bufferListHead; buf != NULL; buf = buf->next){
		/*�����o�b�t�@�ɋ󂫂�����������break*/
		if (buf->file == NULL){
			emptyBuf = buf;
			break;
		}
		if (buf->file == file && buf->pageNum == pageNum){
			/*�v�����ꂽ�y�[�W���o�b�t�@�ɂ������̂ŁA���̓��e�������̃y�[�W�ɃR�s�[����*/
			memcpy(page, buf->page, PAGE_SIZE);

			/*�A�N�Z�X���ꂽ�o�b�t�@�����X�g�̐擪�Ɉړ�������*/
			moveBufferToListHead(buf);

			/*OK��Ԃ�*/
			return OK;
		}
	}

	/*�󂫂��Ȃ��������ԍŌ�̃o�b�t�@���󂯂�*/
	if (emptyBuf == NULL){
		buf = bufferListTail;
		/*�ύX�t���O�������Ă�������e�������߂�*/
		if (buf->modified == MODIFIED){
			/*�����o���ʒu�̈ړ�*/
			if (_lseek(buf->file->desc, (buf->pageNum)*PAGE_SIZE, SEEK_SET) == -1){
				return NG;
			}

			/*��������*/
			if (_write(buf->file->desc, buf->page, PAGE_SIZE) < PAGE_SIZE){
				return NG;
			}

			/*�ύX�t���O��0��*/
			buf->modified = UNMODIFIED;
		}
		/*�Ō�̃o�b�t�@���󂯁A�����ɐV���ȃy�[�W�̓��e��ǂݍ���*/
		emptyBuf = buf;
	}

	/*�󂫃o�b�t�@�Ɉ����Ŏw�肳�ꂽ�t�@�C����
	�@�y�[�W�̓��e��ۑ�����*/
	/*�ǂݏo���ʒu�̈ړ�*/
	if (lseek(file->desc, pageNum * PAGE_SIZE, SEEK_SET) == -1) {
		return NG;
	}
	/*read�V�X�e���R�[���ɂ��t�@�C���ւ̃A�N�Z�X*/
	if (x = read(file->desc, emptyBuf->page, PAGE_SIZE) < PAGE_SIZE){
		return NG;
	}
	/*�o�b�t�@�̓��e��������page�ɃR�s�[*/
	memcpy(page, emptyBuf->page, PAGE_SIZE);

	/*Buffer�\���̂Ɋe����̕ۑ�*/
	emptyBuf->file = file;
	emptyBuf->pageNum = pageNum;

	/*�A�N�Z�X���ꂽ�o�b�t�@�����X�g�̐擪�Ɉړ�������*/
	moveBufferToListHead(emptyBuf);
	return OK;
}

/*1�y�[�W���̃f�[�^�̃t�@�C���ւ̏����o��*/
Result writePage(File *file, int pageNum, char *page){
	Buffer *buf;			/*�����p*/
	Buffer *emptyBuf;		/*�󂫃o�b�t�@�ۑ��p*/
	emptyBuf = NULL;
	/*�o�b�t�@�ɋ󂫂��Ȃ����ƁA�v�����ꂽ�y�[�W�����邩�ǂ������ׂ�*/
	for (buf = bufferListHead; buf != NULL; buf = buf->next){
		/*�����o�b�t�@�ɋ󂫂�����������break*/
		if (buf->file == NULL){
			emptyBuf = buf;
			break;
		}
		if (buf->file == file && buf->pageNum == pageNum){
			/*�v�����ꂽ�y�[�W���o�b�t�@�ɂ������̂ŁA�o�b�t�@�ɃR�s�[����*/
			memcpy(buf->page, page, PAGE_SIZE);

			/*�ύX�t���O��1��*/
			buf->modified = MODIFIED;
			/*�A�N�Z�X���ꂽ�o�b�t�@���肷�Ƃ̐擪�Ɉړ�������*/
			moveBufferToListHead(buf);

			/*OK��Ԃ�*/
			return OK;
		}
	}

	/*�󂫂��Ȃ��������ԍŌ�̃o�b�t�@���󂯂�*/
	if (emptyBuf == NULL){
		buf = bufferListTail;
		/*�ύX�t���O�������Ă�������e�������߂�*/
		if (buf->modified == MODIFIED){
			/*�����o���ʒu�̈ړ�*/
			if (lseek(buf->file->desc, (buf->pageNum)*PAGE_SIZE, SEEK_SET) == -1){
				return NG;
			}

			/*��������*/
			if (write(buf->file->desc, buf->page, PAGE_SIZE) < PAGE_SIZE){
				return NG;
			}

			/*�ύX�t���O��0��*/
			buf->modified = UNMODIFIED;
			/*�Ō�̃o�b�t�@���󂯂�*/
		}
		emptyBuf = buf;
	}

	/*������page���A�o�b�t�@�ɃR�s�[����*/
	memcpy(emptyBuf->page, page, PAGE_SIZE);

	/*Buffer�\���̂Ɋe����̕ۑ�*/
	emptyBuf->file = file;
	emptyBuf->pageNum = pageNum;
	/*�ύX�t���O��1��*/
	emptyBuf->modified = MODIFIED;

	/*�A�N�Z�X���ꂽ�o�b�t�@�����X�g�̐擪�Ɉړ�������*/
	moveBufferToListHead(emptyBuf);
	return OK;
}

/*�t�@�C���̃y�[�W���̎擾*/
int getNumPages(char *filename){
	struct stat stat_buf;
	if (stat(filename, &stat_buf) == -1){
		return -1;
	}
	else{
		return (stat_buf.st_size) / PAGE_SIZE;	//�t�@�C���T�C�Y���y�[�W�T�C�Y�Ŋ��������̂�Ԃ�
	}
}

/*�o�b�t�@���X�g���e�̏o�́i�e�X�g�p�j*/
void printBufferList(){
	Buffer *buf;

	printf("Buffer List:");

	/* ���ꂼ��̃o�b�t�@�̍ŏ���3�o�C�g�������o�͂���*/
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