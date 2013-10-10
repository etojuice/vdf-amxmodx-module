#ifndef __VDFSEARCH_H__
#define __VDFSEARCH_H__

#include "VDFTree.h"


// flags for search class
#define VDF_MATCH_KEY			0
#define VDF_MATCH_VALUE			1
#define VDF_IGNORE_CASE			1 << 1

/**
 *	VDF Searching
 */
enum VEC_DIR
{
	DIR_CHILD = 0,
	DIR_NEXT
};

/**
 *
 */
class VDFSearch
{
public:
				VDFSearch		();
				~VDFSearch		();
	bool		FindNext		();
	VDFNode		*FindNextNode	(VDFNode *refNode);
	void		SetSearch		(VDFTree *inTree, char *search,
								 UINT flags = VDF_MATCH_KEY, int level = -1);
	void		Reset			();
protected:
	bool		Match			(VDFNode *matchNode);
public:
	VDFNode		*cursor;
	UINT		searchId;
	int			currentLevel;
protected:
	int 		searchLevel;	
	UINT		searchFlags;
	VDFTree		*searchTree;	
	size_t		levelCount;
	char		*cmpBuffer;
	char		*searchBuffer;
	VDFNode		*nextInLevel;
	
};

/*
class VDFSearch
{
public:
				VDFSearch		();
				~VDFSearch		();
	bool		FindNext		();
	VDFNode		*FindNextNode	(VDFNode *refNode);
	void		SetSearch		(VDFTree *inTree, char *search,
								 UINT flags = VDF_MATCH_KEY, int level = -1);
	void		Reset			();

protected:
	bool		Match			(VDFNode *matchNode);
	bool		Walk			(UINT index);
	void		MarkAnchor		(VDFNode *anchor);
	VDFNode		*GetAnchorNode	(UINT index);

public:
	VDFNode		*cursor;
	UINT		searchId;
	int			currentLevel;
protected:
	int 		searchLevel;	
	UINT		searchFlags;
	char		*searchStr;
	VDFTree		*searchTree;	
	VDFNode		**levelAnchor;
	size_t		levelCount;
	
};
*/


#endif //__VDFSEARCH_H__
