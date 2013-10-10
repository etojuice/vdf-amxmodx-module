#ifndef __VDFCOLLECTION_H__
#define __VDFCOLLECTION_H__

#include "VDFSearch.h"
#include "VDFParser.h"


/**
 *  Container type for handling collection of trees.
 */
struct VDFEnum
{
	VDFTree *vdfTree;
	char	*vdfFile;
};


/**
 *  This class is designed to handle all trees, searches in a plugin session. 
 *	It implements convenient methods to store and destroy trees and searches. 
 */
class VDFCollection
{
public:	
	int			GetFreeParserID 	();
	int			GetFreeOpenTreeID	();
	void		ParseTree			(const char *filename, ParseForward *parseForward);
	
	VDFTree		*AddTree			(const char *filename, bool create = false, OpenForward *openForward = NULL);
	VDFSearch	*AddSearch			();
	void		SetSearch			(VDFSearch *search,VDFTree *tree, char *searchStr,
									 UINT type, int level = -1, UINT ignoreCase = 0);	
	
	void		RemoveTree			(const UINT index);
	void		RemoveTree			(VDFTree **tree);
	void		RemoveSearch		(const UINT index);
	VDFEnum		*GetContainerById	(const UINT index);

	void		killOpenForward		(int fwid);
	void		killParseForward	(int fwid);
	void		SetLogger(IErrorLogger *logger);

	void		Destroy();
				VDFCollection ();
				~VDFCollection();

	size_t		treeCounter;
	VDFEnum		**vdfTrees;	
	size_t		searchCounter;
	VDFSearch	**vdfSearch;
	IErrorLogger *logger;

	OpenForward		**openForward;
	ParseForward	**parseForward;

};


#endif //__VDFCOLLECTION_H__
