#ifndef __VDFPARSER_H__
#define __VDFPARSER_H__


#include "VDFTree.h"

//typedef void (*VDFReaderFW) (const char* key, const char *value, UINT depth);


/** Constants used in event parser */
enum
{
	RETURN_VDFPARSER_CONTINUE = 0,
	RETURN_VDFPARSER_STOP
};

/** Constants used in tree parser */
enum
{
	RETURN_TREEPARSER_CONTINUE = 0,
	RETURN_TREEPARSER_SILENT,
	RETURN_TREEPARSER_BREAK
};

class IErrorLogger
{
public:
	virtual void printError(const char *filename, const char *message, int line = 0, int charpos = 0) {};
};

/**
 *	Abstract class for reading vdf files.
 *	All required methods for reading are implemented,
 *  the inheriting class must implement the event handler method.
 */
class VDFReader
{
protected:
	const char *filename;
	//VDFReaderFW parser;
	FILE *pFile;
	UINT currentDepth;
	UINT lineCounter;
	IErrorLogger *logger;

	
	char line[1024];
	UINT cursor;
	size_t lineLength;
	int status;

	/** constants for file 'symbols' return by "GetNextSymbol" method */
	enum VdfSymbols
	{
		KV_CLOSE = 0,
		KV_OPEN,
		KV_NEWSTRING,
		KV_NONE,
		KV_ERROR
	};

	enum VdfReaderStatus
	{
		KV_EXP_CLOSE = 0,
		KV_EXP_NEWKV = 2,
		KV_EXP_OPEN = 1
		
	};

	int GetNextSymbol              (char **target, int tokenMax);
	virtual void DispatchToParser  (const char* key = NULL, const char *value= NULL, UINT depth = 0) {};

public:
	//VDFReader          (const char *filename, VDFReaderFW parser = NULL);
	VDFReader		   (IErrorLogger *logger = NULL);
	void Open          ();
	void Open          (const char* filename);
	void Close         ();
	bool NextKeyValue  ();
	/*virtual ~VDFReader () {};*/
	
	/*struct ReaderStatus
	{
		ReaderStatus(){};
		UINT cursor;
		size_t lineLength;
		int currentStatus;
	}*/

};

/* definition for vdf object parsing forward */
typedef int (*PFN_VDFOPEN)			(int fwid, const char* filename, VDFTree* tree,
									 VDFNode* node, int level);

/* definition for vdf event parser forward */
typedef int (*PFN_VDFPARSE_KPAIR)	(int fwid, const char *filename,
									 const char* key,
									 const char* value,
									 int level);

/* parser forward definition for "start" event */
typedef int	(*PFN_VDFPARSE_BOF)		(int fwid, const char *filename);

/* parser forward definition for "end" event*/
typedef int (*PFN_VDFPARSE_EOF)		(int fwid, const char *filename);

/**
 *	Container for parse forwards
 */
struct ParseForward
{
	int fwidParser;
	int fwidStart;
	int	fwidEnd;
	char *mdFilename;
	PFN_VDFPARSE_KPAIR	pfnParser;
	PFN_VDFPARSE_BOF	pfnStart;
	PFN_VDFPARSE_EOF	pfnEnd;
};

/**
 *	Container for open forwards
 */
struct OpenForward
{
	int fwdid;
	char *mdFilename;
	PFN_VDFOPEN pfnOpen;
};

class VDFTreeFile : public VDFReader
{
private:
	int returnVal;
	OpenForward *currentParser;
	VDFTree *currentTree;
	VDFNode *currentNode;
	UINT currentDepth;
	void DispatchToParser(const char* key = NULL, const char *value= NULL, UINT depth = 0);
	class TabFill
	{
	public:
		TabFill(char *tabs) : tabs(tabs), currentPos(-1) {}
		char *tabs;
		int currentPos;	
		void SetTabPos(int pos) {
			if(currentPos > -1) {
				tabs[currentPos] = '\t';
			}
			currentPos = pos;
			tabs[pos] = '\0';
		}
	};
public:
	bool OpenVDF	(const char *filename, VDFTree **vdfTree, OpenForward *openFW = NULL);
	bool SaveVDF	(const char *filename, VDFTree *vdfTree);
	VDFTreeFile		(IErrorLogger *logger = NULL): VDFReader(logger) {};
	
};

class VDFEventReader : public VDFReader
{
private:
	int returnVal;
	ParseForward *currentParser;
	void DispatchToParser(const char* key = NULL, const char *value= NULL, UINT depth = 0);
public:	
	VDFEventReader (IErrorLogger *logger = NULL) : VDFReader(logger) {currentParser = NULL;};
	bool ParseVDF  (const char *filename, ParseForward *parseFW = NULL);	
};


#endif //__VDFPARSER_H__
