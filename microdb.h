/* microdb.h - ���ʒ�`�t�@�C�� */

/* Result -����or���s��Ԃ�- */
typedef enum {OK, NG} Result;

/* PAGE_SIZE -- �t�@�C���A�N�Z�X�̒P�� */
#define PAGE_SIZE 4096

/* MAX_FILENAME -- �I�[�v������t�@�C�����̒����̏��*/
#define MAX_FILENAME 256

/* MAX_FIELD -- 1���R�[�h�Ɋ܂܂��t�B�[���h���̏��*/
#define MAX_FIELD 40

/* MAX_FIELD_NAME -- �t�B�[���h���̒����̏��(�o�C�g��)*/
#define MAX_FIELD_NAME 20

/* MAX_STRING -- ������^�f�[�^�̒����̏��*/
#define MAX_STRING 20

/*snprintf�̒�`*/
#define snprintf _snprintf

/* DataType -- �f�[�^�x�[�X�ɕۑ�����f�[�^�̌^*/
typedef enum DataType DataType;
enum DataType {
	TYPE_UNKNOWN = 0,			/* �f�[�^�^�s�� */
	TYPE_INTEGER = 1,			/* �����^ */
	TYPE_STRING = 2			/* ������^ */
};

/* OpratorType -- ��r���Z�q��\���񋓌^ */
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

/* Condition -- ���R�[�h���ǂ̂悤�ȏ����ł��邩��\������\����*/
typedef struct Condition Condition;
struct Condition
{
	char name[MAX_FIELD_NAME];	/*�t�B�[���h��*/
	DataType dataType;	/* �f�[�^�^�C�v */
	OperatorType operator;	/*��r���Z�q*/
	int intValue;	/*integer�^�̏ꍇ�̒l*/
	char stringValue[MAX_STRING];	/*string�^�̏ꍇ�̒l*/
	distinctFlag distinct;
};

/* FieldInfo -- �t�B�[���h�̏���\������\����*/
typedef struct FieldInfo FieldInfo;
struct FieldInfo {
	char name[MAX_FIELD_NAME];		/* �t�B�[���h�� */
	DataType dataType;			/* �t�B�[���h�̃f�[�^�^ */
};

/* FieldData -- 1�̃t�B�[���h�f�[�^��\������\����*/
typedef struct FieldData FieldData;
struct FieldData
{
	char name[MAX_FIELD_NAME];	//�t�B�[���h��
	DataType dataType;	//�t�B�[���h�̃f�[�^�^
	int intValue;	//integer�^�̏ꍇ�̒l
	char stringValue[MAX_STRING];	//string�^�̏ꍇ�̒l
};

/* RecordData -- 1�̃��R�[�h�̃f�[�^��\������\���� */
typedef struct RecordData RecordData;
struct RecordData
{
	int numField;	//�t�B�[���h��
	FieldData fieldData[MAX_FIELD];	//�t�B�[���h���
	RecordData *next;	//���̃��R�[�h�ւ̃|�C���^
};

/* RecordSet -- ���R�[�h�̏W����\������\���� */
typedef struct RecordSet RecordSet;
struct RecordSet
{
	int numRecord;	/* ���R�[�h�� */
	RecordData *head; /* ���R�[�h�̃��X�g�̐擪�v�f�ւ̃|�C���^ */
	RecordData *tail; /*���R�[�h�̃��X�g�ւ̍Ō�̗v�f�ւ̃|�C���^*/
};

/* File -�I�[�v�������t�@�C���̏���ێ�����\����*/
typedef struct File File;
struct File{
	int desc;	//�t�@�C���f�B�X�N���v�^
	char name[MAX_FILENAME];	//�t�@�C����
};

/* TableInfo -- �e�[�u���̏���\������\����*/
typedef struct TableInfo TableInfo;
struct TableInfo {
	int numField;			/* �t�B�[���h�� */
	FieldInfo fieldInfo[MAX_FIELD];		/* �t�B�[���h���̔z�� */
};

/*file.c�ɒ�`����Ă���֐��Q*/
extern Result initializeFileModule();	/*�t�@�C���A�N�Z�X���W���[���̏���������*/
extern Result finalizeFileModule();	 /*�t�@�C���A�N�Z�X���W���[���̏I������*/
extern Result createFile(char *);	/*�t�@�C���̍쐬*/
extern Result deleteFile(char *);	/*�t�@�C���̍폜*/
extern File *openFile(char *);		/*�t�@�C���I�[�v��*/
extern Result closeFile(File *);	/*�t�@�C���N���[�Y*/
extern Result readPage(File *, int, char *);	/*1�y�[�W���̃f�[�^�̃t�@�C���ւ̓ǂݏo��*/
extern Result writePage(File *, int, char *);	/*1�y�[�W���̃f�[�^�̃t�@�C���ւ̏����o��*/
extern int getNumPages(char *);		/*�t�@�C���̃y�[�W���̎擾*/
extern void printBufferList();		/*�o�b�t�@���X�g�̕\��*/

/*datadef.c�ɒ�`����Ă���֐��Q*/
extern Result initializeDataDefModule();		/*�f�[�^��`���W���[���̏���������*/
extern Result finalizeDataDefModule();			/*�f�[�^��`���W���[���̏I������*/
extern Result createTable(char *, TableInfo *);	/*�e�[�u���̍쐬*/
extern Result dropTable(char *);				/*�e�[�u���̍폜*/
extern TableInfo *getTableInfo(char *);			/*�e�[�u�����̎擾*/
extern void freeTableInfo(TableInfo *);			/*�f�[�^��`�������߂��������̈�̉��*/

/*datamanip.c�ɒ�`����Ă���֐��Q*/
extern Result initializeDataManipModule();	/*�f�[�^���샂�W���[���̏���������*/
extern Result finalizeDataManipModule();	/*�f�[�^���샂�W���[���̏I������*/
extern Result insertRecord(char *, RecordData *);	/*���R�[�h�̍쐬*/
extern Result deleteRecord(char *, Condition *);	/*���R�[�h�̍폜*/
extern RecordSet *selectRecord(char *, Condition *);	/*���R�[�h�̌���*/
extern void freeRecordSet(RecordSet *);	/*���R�[�h�W����ۑ������������̈�̉��*/
extern Result createDataFile(char *);	/*�f�[�^�t�@�C���̍쐬*/
extern Result deleteDataFile(char *);	/*�f�[�^�t�@�C���̍폜*/
extern void printTableData(char *,int);		/*�S�Ẵf�[�^�̕\��*/
extern void printRecordSet(RecordSet *, char [MAX_FIELD][MAX_FIELD_NAME],int);	/*���R�[�h�W���̕\��*/