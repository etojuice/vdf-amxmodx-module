#ifndef __VDFTREE_H__
#define __VDFTREE_H__

#include "common.h"

enum
{
	VDF_MOVEPOS_AFTER = 0,
	VDF_MOVEPOS_BEFORE,
};

/**
 *  Simple node structure
 */
struct VDFNode
{
	VDFNode(): nextNode(NULL), childNode(NULL), parentNode(NULL), previousNode(NULL), key(NULL), value(NULL) {}
	VDFNode						*nextNode;
	VDFNode						*childNode;
	VDFNode						*parentNode;
	VDFNode						*previousNode;
	char						*key;
	char						*value;
};

/**
 *  VDF tree handling.
 */
class VDFTree
{
public:	
					VDFTree			     ();
					~VDFTree		     ();
	void			CreateTree		     ();
	void			DestroyTree		     ();
	size_t			GetLength		     ();
	VDFNode			*CreateNode		     (VDFNode *parentNode = NULL);
	VDFNode			*GetNodeById	     (UINT id);
	void			DeleteNode		     (VDFNode *Node);
	
	static void		AppendNode		     (VDFNode *Node, VDFNode *newNode);
	static void		AppendChild		     (VDFNode *Node, VDFNode *childNode);	
	static void		SetKeyPair	         (VDFNode *Node, const char *key = NULL, const char *value = NULL);
	void			SortBranchNodes	     (VDFNode *refNode, bool byKey = true, bool byNumber = false);
	static VDFNode	*GetRootNode	     (VDFNode *Node);
	static VDFNode	*GetLastNode	     (VDFNode *Node);
	static VDFNode	*GetPreviousNode     (VDFNode *Node);
	static VDFNode	*GetFirstNode        (VDFNode *Node);
	static VDFNode	*GetNextNode         (VDFNode *Node);
	static VDFNode  *GetNextTraverseStep (VDFNode *Node, int &depth);
	static size_t	CountBranchNodes     (VDFNode *refNode);
	static size_t	GetNodeLevel	     (VDFNode *Node);
	void			MoveToBranch	     (VDFNode *anchorNode, VDFNode *moveNode, UINT position);
	void			MoveAsChild		     (VDFNode *parentNode, VDFNode *moveNode);

protected:
	inline bool		IsTreeNode		   (VDFNode *node);

public:
	VDFNode		*rootNode;
	size_t		nodeCount;
	UINT		treeId;

protected:
	VDFNode					**nodeIndex;
};




#endif //__VDFPARSER_H__
