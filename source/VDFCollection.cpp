/*
*
*  This program is free software; you can redistribute it and/or modify it
*  under the terms of the GNU General Public License as published by the
*  Free Software Foundation; either version 2 of the License, or (at
*  your option) any later version.
*
*  This program is distributed in the hope that it will be useful, but
*  WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
*  General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software Foundation,
*  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/**  
 *	@author		commonbullet
 *	@version	1.07
 */

#include <string.h>
#include "VDFCollection.h"


// --- VDFCollection implementation ---

VDFCollection::VDFCollection()
{
	treeCounter	= 0;
	searchCounter = 0;
	//openForwards = 0;
	//parseForwards = 0;
	vdfTrees = NULL;
	logger = NULL;

	parseForward = new ParseForward*[MAX_PARSE_FORWARDS];
	openForward = new OpenForward*[MAX_OPEN_FORWARDS];
	
	memset(parseForward, 0, MAX_PARSE_FORWARDS);
	memset(openForward, 0, MAX_OPEN_FORWARDS);
	
}

VDFCollection::~VDFCollection()
{
	Destroy();
}

void VDFCollection::SetLogger(IErrorLogger *logger)
{
	this->logger = logger;
}


/**
 *	Object freeing
 */
void VDFCollection::Destroy()
{
	size_t i;
	
	for(i = 0; i < treeCounter; i++) {
		if(vdfTrees[i] == NULL)
			continue;
		Finalize(vdfTrees[i]->vdfTree);
		FinalizeArray(vdfTrees[i]->vdfFile);
	}
	FinalizeArray(vdfTrees);

	for(i = 0; i < searchCounter; i++) {
		if(vdfSearch[i] == NULL)
			continue;
		Finalize(vdfSearch[i]);
	}
	
	FinalizeArray(vdfSearch);
	FinalizeArray(parseForward);
	FinalizeArray(openForward);

	searchCounter = 0;
	treeCounter = 0;
}

/**
 *	Looks for a free parse slot. The max number of simultaneos parsing
 *	(nested) is defined by MAX_PARSE_FORWARDS.
 *	@return		The index of an available slot or -1 if all slots are being used.
 */
int VDFCollection::GetFreeParserID()
{
	
	for(UINT i = 0; i < MAX_PARSE_FORWARDS; i++) {
		if(parseForward[i] == NULL)
			return i;
	}
	return -1;
}

/**
 *	Looks for a free parse slot. The max number of simultaneos parsing
 *	(nested) is defined by MAX_PARSE_FORWARDS.
 *	@return		The index of an available slot or -1 if all slots are being used.
 */
int VDFCollection::GetFreeOpenTreeID()
{
	
	for(UINT i = 0; i < MAX_PARSE_FORWARDS; i++) {
		if(openForward[i] == NULL)
			return i;
	}
	return -1;
}

/**
 *	Starts parsing process.
 *	@param	filename	Name of the file to be parsed.
 *	@param	pFW     	Forward settings.
 */
void VDFCollection::ParseTree(const char *filename, ParseForward *pFW)
{
	VDFEventReader parser = VDFEventReader(this->logger);
	
	if(pFW == NULL)
		return;
	
	parser.ParseVDF(filename, pFW);
}


/**
 *	Adds a new vdf tree to collection
 *	@param	filename	Name of the tree file. Can be NULL if you're
 *						creating a new Tree.
 *	@param	create  	Set it to <code>true</code> if you're creating a Tree
 *						rather than opening an existing one.
 *	@return				The VDFTree pointer or NULL on fail.
 */
VDFTree *VDFCollection::AddTree(const char *filename, bool create, OpenForward *openFW)
{
	VDFTreeFile	parser = VDFTreeFile(logger);
	VDFTree		*vdfTree;
	
	vdfTree  = NULL;

	if(create) {
		vdfTree = new VDFTree;
		vdfTree->CreateTree();
	}
	else {
		if(!parser.OpenVDF(filename, &vdfTree, openFW))
			return	NULL;
	}

	GrowPArray(&vdfTrees, treeCounter);
	
	vdfTrees[treeCounter] = new VDFEnum;
	vdfTrees[treeCounter]->vdfFile = new char[strlen(filename) + 1];	
	strcpy(vdfTrees[treeCounter]->vdfFile, filename);
	vdfTrees[treeCounter]->vdfTree = vdfTree;	
	vdfTree->treeId = (UINT) treeCounter++;

	return vdfTree;
}

/**
 *	Adds a new Search object to collection
 *	@return A pointer to the new Search object.
 */
VDFSearch *VDFCollection::AddSearch()
{
	VDFSearch *newSearch;	
	
	GrowPArray(&vdfSearch, searchCounter);	
	newSearch = new VDFSearch;
	newSearch->searchId = (UINT)searchCounter;
	vdfSearch[searchCounter++] = newSearch;
	
	return newSearch;
}

/**
 *	Sets a search string/type to a search pointer included with <code>AddSearch</code>.
 *	@param	search		Target search pointer.
 *	@param	tree		Tree in which search will be performed.
 *	@param	searchStr	String to be searched.
 *	@param	type		Search for key or value - VDF_MATCH_KEY or VDF_MATCH_VALUE.
 *						<br>Note: searching for both at the same time isn't supported.
 *	@param	level		Checks which level to perform search. If -1 (default) all levels match.
 *	@param	ignoreCase	If different from 0 it doesn't match case.
 */
void VDFCollection::SetSearch(VDFSearch *search, VDFTree *tree, char *searchStr, UINT type,
										int level, UINT ignoreCase)
{
	UINT flags;
	
	if(search == NULL || tree == NULL)
		return;

	flags = type | ((ignoreCase) ? VDF_IGNORE_CASE : 0);
	
	search->SetSearch(tree, searchStr, flags, level);	
}

/**
 *	Removes a search from list
 *	param	@index	Index of the search to be removed.
 */
void VDFCollection::RemoveSearch(const UINT index)
{
	if(index < searchCounter)
		Finalize(vdfSearch[index]);
}

/**
 *	Removes a specific tree
 *	@param	index	Index of a tree.
 */
void VDFCollection::RemoveTree(const UINT index)
{
	Finalize(vdfTrees[index]);
}

/**
 *	Removes a specific tree
 *	@param	tree	Tree object to be destroyed.
 */
void VDFCollection::RemoveTree(VDFTree **tree)
{
	if(*tree != NULL) {
		vdfTrees[(*tree)->treeId] = NULL;
		(*tree)->DestroyTree();
	}
}

/**
 *	Get a contatiner by its index
 *	@param	unsigned int	Target index
 *	@return					The container or null if it's been deleted
 *							or if index is out of bounds.
 */
VDFEnum *VDFCollection::GetContainerById(const UINT index)
{
	if(index < treeCounter)
		return vdfTrees[index];
	return NULL;
}

