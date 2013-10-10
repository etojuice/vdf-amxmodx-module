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
 *	@version	1.07
 */


#include "sdk/amxxmodule.h"
#include "VDFCollection.h"


#if defined __GNUC__
#define _snprintf snprintf
#endif

class VDFErrorLogger: public IErrorLogger
{
private:
	AMX *context;
	//char errorMessage[512];
public:
	void SetAmxContext(AMX *amx)
	{
		context = amx;
	}
	void printError(const char *filename, const char *message, int line = 0, int charpos = 0)
	{
		//sprintf(errorMessage, "Error parsing %s - %s at line %d\n", filename, message, line);
		MF_LogError(context, AMX_ERR_FORMAT, "Error parsing %s - %s at line %d", filename, message, line);
	}
};


VDFCollection vdfCollection;
VDFErrorLogger logger;


/**
 *	Converts String to Float
 *	Taken from default amxx str_to_float native
 *	by AMXModX Team
 */
cell StringToFloat(char *str)
{
	bool neg = false;
	unsigned long part1 = 0;

	if (*str == '-') 
	{
		neg = true;
		++str;
	}
	else if (*str == '+')
		++str;

	while (*str)
	{
		if (*str == '.')
		{
			++str;
			break;
		}
		
		if (*str < '0' || *str > '9')
		{
			REAL fl = neg ? -static_cast<REAL>(part1) : static_cast<REAL>(part1);
			return amx_ftoc(fl);
		}

		part1 *= 10;
		part1 += *str - '0';

		++str;
	}

	unsigned long part2 = 0;
	unsigned long div = 1;
	
	while (*str)
	{
		if (*str < '0' || *str > '9')
			break;

		part2 *= 10;
		part2 += *str - '0';
		div *= 10;
		++str;
	}

	REAL fl = static_cast<REAL>(part1) + (static_cast<REAL>(part2) / div);
	
	if (neg)
		fl = -fl;
	
	return amx_ftoc(fl);
}

/**
 *	Performs "keypairs" forward.
 *	@param	fwid		Registered forward id (script).
 *	@param	filename	File being parsed.
 *	@param	key			Captured key.
 *	@param	value		Captured value;
 *	@param	level		Depth in tree.
 */
int ExecParseForward(int fwid, const char *filename, const char *key,
					 const char *value, int level)
{
	return MF_ExecuteForward(fwid, filename, key, value, level);
}

/**
 *	Fires parsing start event
 *	@param fwid			Registered forward id (script).
 *	@param filename		Name of the file being parsed.
 */
int ExecParseBofForward(int fwid, const char *filename)
{
	return MF_ExecuteForward(fwid, filename);	
}

/** 
 *	Fires parsing end event
 *	@param fwid			Registered forward id (script).
 *	@param filename		Name of the file being parsed.
 */
int ExecParseEofForward(int fwid, const char *filename)
{
	return MF_ExecuteForward(fwid, filename);
}

/*
 *	Fires "node added to tree" event
 *	@param	fwid			Registered forward id (script).
 *	@param	filename		Name of the file being opened.
 *	@param	tree			Tree being generated.
 *	@param	node			Node added.
 *	@param	level			Depth of the node in tree hierarchy.
 */
int ExecOpenTreeForward(int fwid, const char* filename, VDFTree* tree,
							VDFNode* node, int level)
{
	return MF_ExecuteForward(fwid, filename, tree, node, level);	
}



//vdf_parse(const filename[], const keypairs_func[], const start_func[] = "", const end_func = "")
static cell AMX_NATIVE_CALL vdf_parse(AMX *amx, cell *params)
{
	char	*filename;
	char	*mdFilename;
	char	*keypairsFunc;
	char	*startFunc;
	char	*endFunc;
	int		len;
	FILE	*file;
	ParseForward *pfw;
	int		fwid;	


	mdFilename = MF_GetAmxString(amx, params[1], 0, &len);
	filename = g_fn_BuildPathname("%s", mdFilename);
	keypairsFunc =  MF_GetAmxString(amx, params[2], 1, &len);

	if(!len)
		return 0;

	if((file = fopen(filename, "r")) == NULL)
		return 0;
	fclose(file);

	startFunc = MF_GetAmxString(amx, params[3], 2, &len);
	endFunc = MF_GetAmxString(amx, params[4], 3, &len);
	logger.SetAmxContext(amx);

	if((fwid = vdfCollection.GetFreeParserID()) == -1)
		return 0;
	
	vdfCollection.parseForward[fwid] = new ParseForward;
	pfw = vdfCollection.parseForward[fwid];
	
	pfw->mdFilename = mdFilename;

	// register forwards
	pfw->fwidParser = MF_RegisterSPForwardByName(amx, keypairsFunc,
		FP_STRING, FP_STRING, FP_STRING, FP_CELL, FP_DONE);
	pfw->pfnParser = &ExecParseForward;

	
	if(*startFunc) {
		pfw->fwidStart = MF_RegisterSPForwardByName(amx, startFunc,
			FP_STRING, FP_DONE);
		pfw->pfnStart = &ExecParseBofForward;
	}
	else
		pfw->pfnStart = NULL;

	if(*endFunc) {
		pfw->fwidEnd = MF_RegisterSPForwardByName(amx, endFunc,
			FP_STRING, FP_DONE);
		pfw->pfnEnd = &ExecParseEofForward;
	}
	else
		pfw->pfnEnd = NULL;

	// parse
	vdfCollection.ParseTree(filename, pfw);
	
	// unregister forwards
	MF_UnregisterSPForward(pfw->fwidParser);
	if(pfw->pfnStart != NULL)
		MF_UnregisterSPForward(pfw->fwidStart);
	if(pfw->pfnEnd != NULL)
		MF_UnregisterSPForward(pfw->fwidEnd);
	
	delete(pfw);
	vdfCollection.parseForward[fwid] = NULL;

	return 1;

}

/**
 *	<code> native vdf_open(filename, const node_add_func[] = "") </code>
 *	@return	Returns the pointer of the vdf tree if suceeded.
 */
static cell AMX_NATIVE_CALL vdf_open(AMX *amx, cell *params) 
{
	int len;
	char *mdFilename;
	char *filename;
	char *openFunc;
	OpenForward *openFW;
	int fwid;
	VDFTree *tree;
	FILE *file;
	
	mdFilename = MF_GetAmxString(amx, params[1], 0, &len);
	filename = g_fn_BuildPathname("%s", mdFilename);
	openFunc = MF_GetAmxString(amx, params[2], 1, &len);
	openFW = NULL;

	file = fopen(filename, "r");	
	if(file == NULL)
		return 0;
	fclose(file);
	fwid = 0;

	logger.SetAmxContext(amx);

	if(*openFunc) {
		if((fwid = vdfCollection.GetFreeOpenTreeID()) > -1) {
			vdfCollection.openForward[fwid] = new OpenForward;
			openFW = vdfCollection.openForward[fwid];
			openFW->fwdid = MF_RegisterSPForwardByName(amx, openFunc, FP_STRING, FP_CELL,
				FP_CELL, FP_CELL, FP_DONE);
			openFW->mdFilename = mdFilename;
			openFW->pfnOpen = &ExecOpenTreeForward;
		}
	}

	tree = vdfCollection.AddTree(filename, false, openFW);

	if(openFW) {
		MF_UnregisterSPForward(openFW->fwdid);
		delete(openFW);
		vdfCollection.openForward[fwid] = NULL;
	}
	
	return (cell)tree;
}

/**
 *	<code> native vdf_save(vdftree, saveas[] = "") </code>
 *	@return	Returns 1 if suceeded, 0 on fail.
 */
static cell AMX_NATIVE_CALL vdf_save(AMX *amx, cell *params)
{
	int			len;
	VDFTree*	vdfTree;
	bool		ret;	
	VDFTreeFile	fileHandler;
	VDFEnum		*container;

	char		*saveAs = g_fn_BuildPathname("%s", MF_GetAmxString(amx, params[2], 0, &len));
	
	vdfTree = reinterpret_cast<VDFTree*>(params[1]);
	ret	= 0;	
	container = vdfCollection.GetContainerById(vdfTree->treeId);

	if(vdfTree == NULL)
		return 0;

	if(len)
		ret = fileHandler.SaveVDF(saveAs, vdfTree);
	else
		ret = fileHandler.SaveVDF(vdfCollection.GetContainerById(vdfTree->treeId)->vdfFile,
				vdfTree);	

	return ret == true ? 1 : 0;
}

/**
 *	<code> native vdf_get_root_node(vdftree) </code>
 *	@return	Returns a pointer of the root node.
 */
static cell AMX_NATIVE_CALL vdf_get_root_node(AMX *amx, cell *params)
{
	VDFTree *vdfTree;

	vdfTree = reinterpret_cast<VDFTree*>(params[1]);
	
	if(vdfTree == NULL)
		return 0;

	return (cell)(vdfTree->rootNode);
}

/**
 *	<code> native vdf_get_first_node(node) </code>
 *	@return	Returns a pointer to first node in a branch.
 */
static cell AMX_NATIVE_CALL vdf_get_first_node(AMX *amx, cell *params)
{
	VDFNode	*vdfNode;
	
	vdfNode = reinterpret_cast<VDFNode*>(params[1]);

	if(vdfNode == NULL)
		return 0;

	return (cell)(VDFTree::GetFirstNode(vdfNode));
}

/**
 *	<code> native vdf_get_last_node(node) </code>
 *	@return	Returns a pointer to the last node in a branch.
 */
static cell AMX_NATIVE_CALL vdf_get_last_node(AMX *amx, cell *params)
{
	VDFNode	*vdfNode;

	vdfNode = reinterpret_cast<VDFNode*>(params[1]);

	if(vdfNode == NULL)
		return 0;

	return (cell)(VDFTree::GetLastNode(vdfNode));
}

/**
 *	<code> native vdf_get_previous_node(node) </code>
 *	@return	Returns a pointer to the previous.
 */
static cell AMX_NATIVE_CALL vdf_get_previous_node(AMX *amx, cell *params)
{
	VDFNode	*vdfNode;

	vdfNode = reinterpret_cast<VDFNode*>(params[1]);

	if(vdfNode == NULL)
		return 0;

	return (cell)(VDFTree::GetPreviousNode(vdfNode));
}

/**
 *	<code> native vdf_get_child_node(node) </code>
 *	@return	Returns a pointer to the last tree in a branch.
 */
static cell AMX_NATIVE_CALL vdf_get_child_node(AMX *amx, cell *params)
{
	VDFNode	*vdfNode;

	vdfNode = reinterpret_cast<VDFNode*>(params[1]);

	if(vdfNode == NULL)
		return 0;

	return (cell)(vdfNode->childNode);
}

/**
 *	<code> native vdf_get_next_node(node) </code>
 *	@return	Returns a pointer to the next node.
 */
static cell AMX_NATIVE_CALL vdf_get_next_node(AMX *amx, cell *params)
{
	VDFNode	*vdfNode;
	
	vdfNode = reinterpret_cast<VDFNode*>(params[1]);

	if(vdfNode == NULL)
		return 0;

	return (cell)(vdfNode->nextNode);
}

/**
 *	<code>native vdf_get_parent_node(node)</code>
 *	@return	Returns a pointer to the parent node.
 */
static cell AMX_NATIVE_CALL vdf_get_parent_node(AMX *amx, cell *params)
{
	VDFNode	*vdfNode;

	vdfNode = reinterpret_cast<VDFNode*>(params[1]);

	if(vdfNode == NULL)
		return 0;

	return (cell)(vdfNode->parentNode);
}

/**
 *	<code>native vdf_next_in_traverse(node)</code>
 *	@return	Returns a pointer to the next node in tree traverse.
 */
static cell AMX_NATIVE_CALL vdf_next_in_traverse(AMX *amx, cell *params)
{
	VDFNode	*vdfNode;
	cell *curDepth;
	int  depth;
	cell ret;

	vdfNode = reinterpret_cast<VDFNode*>(params[1]);
	curDepth = MF_GetAmxAddr(amx, params[2]);
	depth = (int)(*curDepth);

	if(vdfNode == NULL)
		return 0;

	ret = (cell)(VDFTree::GetNextTraverseStep(vdfNode, depth));
	*curDepth = (cell)depth;

	return ret;
}

/**
 *	<code> native vdf_delete_node(vdftree, node) </code>
 *	@return	Returns 1 if it's been delete or 0 if it's already been deleted
 */
static cell AMX_NATIVE_CALL vdf_delete_node(AMX *amx, cell *params)
{
	VDFNode		*vdfNode;
	VDFTree		*vdfTree;

	vdfNode = reinterpret_cast<VDFNode*>(params[2]);
	vdfTree = reinterpret_cast<VDFTree*>(params[1]);

	if(vdfNode == NULL || vdfTree == NULL)
		return 0;
	
	vdfTree->DeleteNode(vdfNode);
	
	return 1;
}

/**
 *	<code>native vdf_get_node_key(node, key[], maxlen)</code>
 *	@return	Returns 1 if succeeded
 */
static cell AMX_NATIVE_CALL vdf_get_node_key(AMX *amx, cell *params)
{
	cell		*key;
	char		*nodekey;
	VDFNode		*vdfNode;
	size_t		maxlen;

	key     =  MF_GetAmxAddr(amx, params[2]);
	vdfNode =  reinterpret_cast<VDFNode*>(params[1]);
	maxlen  =  (size_t)params[3];
	
	if(vdfNode == NULL)
		return 0;
	
	nodekey = vdfNode->key;

	if(nodekey == NULL)
		return 0;	

	while(maxlen-- && nodekey)
		*key++ = (cell)*nodekey++;
	*key = 0;

	return 1;		
}

/**
 *	<code> native vdf_get_node_value(node, value[], maxlen) </code>
 *	@return	Returns 1 if succeeded
 */
static cell AMX_NATIVE_CALL vdf_get_node_value(AMX *amx, cell *params)
{
	cell		*value;
	char		*nodevalue;
	VDFNode		*vdfNode;
	size_t		maxlen;
	
	char  blank[1] = {'\0'};

	value   =  MF_GetAmxAddr(amx, params[2]);
	vdfNode =  reinterpret_cast<VDFNode*>(params[1]);
	maxlen  =  (size_t)params[3];
	
	if(vdfNode == NULL)
		return 0;
	
	nodevalue = vdfNode->value;

	if(nodevalue == NULL)
		nodevalue = blank;	

	while(maxlen-- && nodevalue)
		*value++ = (cell)*nodevalue++;
	*value = 0;

	return 1;
}

/**
 *	<code> native vdf_create_tree(const filename[]) </code>
 *	@return	Returns a pointer to the new tree if succeeded, 0 on fail.
 */
static cell AMX_NATIVE_CALL vdf_create_tree(AMX *amx, cell *params)
{
	int 	len;

	char *vdfFile = g_fn_BuildPathname("%s", MF_GetAmxString(amx, params[1], 0, &len));
	return (cell)vdfCollection.AddTree(vdfFile, true);
}

/**
 *	<code> native vdf_get_append_node(vdftree, node, key = "", value = "") </code>
 *	@return	Returns a pointer to the new appended node if succeeded, 0 on fail
 */
static cell AMX_NATIVE_CALL vdf_append_node(AMX *amx, cell *params)
{
	char		*key   = NULL;
	char		*value = NULL;
	int			lenk, lenv;
	VDFNode		*newNode;
	VDFNode		*refNode;
	VDFTree		*vdfTree;

	vdfTree    = reinterpret_cast<VDFTree*>(params[1]);
	refNode    = reinterpret_cast<VDFNode*>(params[2]);
	key		   = MF_GetAmxString(amx, params[3], 0, &lenk);
	value	   = MF_GetAmxString(amx, params[4], 1, &lenv);
	
	if(vdfTree == NULL || refNode == NULL)
		return 0;

	newNode = vdfTree->CreateNode();
	
	if(key && lenk)
		vdfTree->SetKeyPair(newNode, key);
	if(value && lenv)
		vdfTree->SetKeyPair(newNode, NULL, value);

	vdfTree->AppendNode(refNode, newNode);
	
	return (cell)newNode;
}

/**
 *	<code> native vdf_append_childnode(vdf, node, key[] = "", value[] = "") </code>
 *	@return	Returns a pointer to the new appended child if succeeded, 0 on fail.
 */
static cell AMX_NATIVE_CALL vdf_append_child_node(AMX *amx, cell *params)
{
	char		*key   = NULL;
	char		*value = NULL;
	int			lenk, lenv;
	VDFNode		*newNode;
	VDFNode		*refNode;
	VDFTree		*vdfTree;

	vdfTree		= reinterpret_cast<VDFTree*>(params[1]);
	refNode		= reinterpret_cast<VDFNode*>(params[2]);
	key			= MF_GetAmxString(amx, params[3], 0, &lenk);
	value		= MF_GetAmxString(amx, params[4], 1, &lenv);
	
	if(vdfTree == NULL || refNode == NULL)
		return 0;

	newNode = vdfTree->CreateNode();
	
	if(key != NULL && lenk)
		vdfTree->SetKeyPair(newNode, key);
	if(value != NULL && lenv)
		vdfTree->SetKeyPair(newNode, NULL, value);

	vdfTree->AppendChild(refNode, newNode);
	
	return (cell)newNode;
}


/**
 *	<code> native vdf_set_node_key(node, const key[]) </code>
 *	@return	Returns 1 if succeeded, 0 on fail
 */
static cell AMX_NATIVE_CALL vdf_set_node_key(AMX *amx, cell *params)
{
	VDFNode *vdfNode;
	int		lenk;
	char	*key;

	key		=  MF_GetAmxString(amx, params[2], 0, &lenk);
	vdfNode =  reinterpret_cast<VDFNode*>(params[1]);
	
	if(vdfNode == NULL)
		return 0;

	VDFTree::SetKeyPair(vdfNode, key);
	return 1;
}

/**
 *	<code> native vdf_set_node_value(node, const value[]) </code>
 */
static cell AMX_NATIVE_CALL vdf_set_node_value(AMX *amx, cell *params)
{
	VDFNode *vdfNode;
	int		lenv;
	char	*value;

	value		=  MF_GetAmxString(amx, params[2], 0, &lenv);
	vdfNode		=  reinterpret_cast<VDFNode*>(params[1]);
	
	if(vdfNode == NULL)
		return 0;

	VDFTree::SetKeyPair(vdfNode, NULL, value);
	return 1;
}


/**
 *	<code> native vdf_remove_tree(vdftree) </code>
 *	@return	Returns 1 if succeeded, 0 on fail
 */
static cell AMX_NATIVE_CALL vdf_remove_tree(AMX *amx, cell *params)
{
	VDFTree *tree;
	
	tree = reinterpret_cast<VDFTree*>(params[1]);

	if(tree == NULL)
		return 0;
	
	vdfCollection.RemoveTree(&tree);
	return 1; 
}

/**
 *	<code> native vdf_count_branch_nodes(node) <code>
 *	@return	Returns number of nodes in current branch.
 */
static cell AMX_NATIVE_CALL vdf_count_branch_nodes(AMX *amx, cell *params)
{
	VDFNode *refNode;

	refNode = reinterpret_cast<VDFNode*>(params[1]);
	
	if(refNode != NULL)
		return VDFTree::CountBranchNodes(refNode);

	return 0;
}

/**
 *	<code> vdf_create_search() </code>
 *	@return	Returns search pointer.
 */
static cell AMX_NATIVE_CALL vdf_create_search(AMX *amx, cell *params)
{
	return (cell)vdfCollection.AddSearch();
}

// set_search

/**
 *	<code> vdf_set_search(search, tree, const searchstr[], searchtype = 0,
 *							level = -1, ignorecase = 0) </code>
 */
static cell AMX_NATIVE_CALL vdf_set_search(AMX *amx, cell *params)
{
	int			level;
	UINT		searchType;
	UINT		ignoreCase;
	char		*searchStr;
	VDFTree		*tree;
	VDFSearch	*search;
	int			len;

	search		=	reinterpret_cast<VDFSearch*>(params[1]);
	tree		=	reinterpret_cast<VDFTree*>(params[2]);
	searchStr	=	MF_GetAmxString(amx, params[3], 0, &len);
	searchType	=	(UINT)params[4];
	level		=	(int)params[5];
	ignoreCase	=	(UINT)params[6];

	if(tree == NULL || search == NULL)
		return 0;

	vdfCollection.SetSearch(search, tree, searchStr, searchType,
											  level, ignoreCase);

	return 1;
}


/**
 *	<code>	vdf_find_next_match(search, startnode = 0)	</code>
 *	@return	Returns next node or 0 if nothing is found.
 */
static cell AMX_NATIVE_CALL vdf_find_next_match(AMX *amx, cell *params)
{
	VDFSearch *search;
	VDFNode   *node;
	
	search = reinterpret_cast<VDFSearch*>(params[1]);
	node   = reinterpret_cast<VDFNode*>(params[2]);

	if(search == NULL)
		return 0;

	node = search->FindNextNode(node);
	
	return (cell)node;
}

/**
 *	<code>	vdf_close_search(search)	</code>
 */
static cell AMX_NATIVE_CALL vdf_close_search(AMX *amx, cell *params)
{
	VDFSearch *search;
	
	search = reinterpret_cast<VDFSearch*>(params[1]);
	
	if(search == NULL)
		return 0;
	
	vdfCollection.RemoveSearch(search->searchId);
	return 1;
}

/*
 * <code>	vdf_get_node_level(node) </code>
 *	@return	The level of a node. Because a node level 0 is valid, it returns -1 on error.
 */

static cell AMX_NATIVE_CALL vdf_get_node_level(AMX *amx, cell *params)
{
	VDFNode	*vdfNode;

	vdfNode = reinterpret_cast<VDFNode*>(params[1]);

	if(vdfNode == NULL)
		return -1;
	

	return (cell)(VDFTree::GetNodeLevel(vdfNode));
}

//vdf_get_node_value_int(node)
static cell AMX_NATIVE_CALL vdf_get_node_value_num(AMX *amx, cell *params)
{
	VDFNode *vdfNode;

	vdfNode = reinterpret_cast<VDFNode*>(params[1]);

	if(vdfNode == NULL)
		return 0;

	return (cell)atoi(vdfNode->value);
}

//vdf_get_node_value_float(node, Float:value)
static cell AMX_NATIVE_CALL vdf_get_node_value_float(AMX *amx, cell *params)
{
	VDFNode *vdfNode;

	vdfNode = reinterpret_cast<VDFNode*>(params[1]);

	if(vdfNode == NULL)
		return 0;

	return StringToFloat((vdfNode->value == NULL) ? 0 : vdfNode->value);	
}

//vdf_get_node_value_vector(node, Float:vector[3])
static cell AMX_NATIVE_CALL vdf_get_node_value_vector(AMX *amx, cell *params)
{
	char	*value;
	char	sep[4];
	char	*token;
	cell	*vec;
	VDFNode *node;
	size_t	len;
	
	vec = MF_GetAmxAddr(amx, params[2]);
	node = reinterpret_cast<VDFNode*>(params[1]);

	if(node == NULL)
		return 0;
	
	if(node->value == NULL) {
		vec[0] = 0;
		vec[1] = 0;
		vec[2] = 0;
		return 0;
	}
	
	value = new char[strlen(node->value) + 1];
	strcpy(value, node->value);

	sprintf(sep, " ,");
	token = strtok(value, sep);
	len = 0;
	
	while(len < 3 && token != NULL) {
		vec[len++] = StringToFloat(token);
		token = strtok(NULL, sep);
	}
	
	delete(value);

	if(len)
		return 1;

	return 0;
}

//vdf_set_node_value_num(VdfNode:node, value)
static cell AMX_NATIVE_CALL vdf_set_node_value_num(AMX *amx, cell *params)
{
	VDFNode *node;
	char	stNum[22];
	int		num;

	*stNum = '\0';
	node = reinterpret_cast<VDFNode*>(params[1]);
	num = (int)params[2];
	
	if(node == NULL)
		return 0;
	
	scanf(stNum, "%d", num);

	if(strlen(stNum))
		VDFTree::SetKeyPair(node, NULL, stNum);

	return 1;
}

//vdf_set_node_value_float(VdfNode:node, Float:value)
static cell AMX_NATIVE_CALL vdf_set_node_value_float(AMX *amx, cell *params)
{
	char	value[22];
	VDFNode *node;

	node = reinterpret_cast<VDFNode*>(params[1]);
	
	if(node == NULL)
		return 0;

	_snprintf(value, 22, "%f", amx_ctof(params[2]));
	VDFTree::SetKeyPair(node, NULL, value);	

	return 1;
}

//vdf_set_node_value_vector(VdfNode:node, Float:vector[3])
static cell AMX_NATIVE_CALL vdf_set_node_value_vector(AMX *amx, cell *params)
{
	char		value[70];
	int			len;
	VDFNode		*node;
	cell		*vector;

	node = reinterpret_cast<VDFNode*>(params[1]);

	if(node == NULL)
		return 0;
	
	vector = MF_GetAmxAddr(amx, params[2]);

	len = _snprintf(value, 22, "%f ", amx_ctof(vector[0]));
	len += _snprintf(value + len, 22, "%f ", amx_ctof(vector[1]));
	len += _snprintf(value + len, 21, "%f", amx_ctof(vector[2]));
	
	VDFTree::SetKeyPair(node, NULL, value);

	return 1;

}

//vdf_set_node_value_vector(VdfNode:node, Float:vector[3])
static cell AMX_NATIVE_CALL vdf_sort_branch(AMX *amx, cell *params)
{
	VDFTree *tree;
	VDFNode *node;
	UINT	byValue;
	UINT	asNumber;

	tree = reinterpret_cast<VDFTree*>(params[1]);
	node = reinterpret_cast<VDFNode*>(params[2]);
	byValue = (UINT)params[3];
	asNumber = (UINT)params[4];

	if(tree == NULL || node == NULL)
		return 0;

	tree->SortBranchNodes(node, byValue == 1, asNumber == 1);
	return (cell) VDFTree::GetFirstNode(node);
}

//vdf_move_to_branch(VdfTree:tree, VdfNode:moveNode, anchorNode, bool:insertAfter = true) 
static cell AMX_NATIVE_CALL vdf_move_to_branch(AMX *amx, cell *params)
{
	VDFTree *tree;
	VDFNode *moveNode;
	VDFNode *anchorNode;
	UINT	insertAfter;

	tree = reinterpret_cast<VDFTree*>(params[1]);
	moveNode = reinterpret_cast<VDFNode*>(params[2]);
	anchorNode = reinterpret_cast<VDFNode*>(params[3]);
	insertAfter = (UINT)params[4];

	if(moveNode == NULL || anchorNode == NULL || moveNode == anchorNode)
		return 0;

	tree->MoveToBranch(anchorNode, moveNode, (insertAfter) ? VDF_MOVEPOS_AFTER : VDF_MOVEPOS_BEFORE);
	
	return 1;

}


//vdf_move_as_child(VdfTree:tree, VdfNode:moveNode, VdfNode:parentNode) 
static cell AMX_NATIVE_CALL vdf_move_as_child(AMX *amx, cell *params)
{
	VDFTree *tree;
	VDFNode *moveNode;
	VDFNode *parentNode;

	tree = reinterpret_cast<VDFTree*>(params[1]);
	moveNode = reinterpret_cast<VDFNode*>(params[2]);
	parentNode = reinterpret_cast<VDFNode*>(params[3]);

	if(moveNode == NULL || parentNode == NULL || moveNode->parentNode == parentNode)
		return 0;

	tree->MoveAsChild(parentNode, moveNode);
	
	return 1;

}

//VdfNode:vdf_find_in_branch(VdfNode:node, const schstring[], bool:bykey = true, bool ignorecase = false) 
static cell AMX_NATIVE_CALL vdf_find_in_branch(AMX *amx, cell *params)
{
	VDFNode *startNode;
	char	*sch;
	char	*searchStr;
	char	*cmp;
	UINT	byKey;
	UINT	ignoreCase;
	int		len;

	startNode = reinterpret_cast<VDFNode*>(params[1]);

	if(startNode == NULL)
		return 0;

	sch	=	MF_GetAmxString(amx, params[2], 0, &len);
	byKey = (UINT)params[3];
	ignoreCase = (UINT)params[4];
	

	if(ignoreCase) {
		searchStr = new char[strlen(sch) + 1];
		ToLowerCase(sch, searchStr);
	}
	else
		searchStr = sch;

	while(startNode) {
		
		if((cmp = (byKey) ? startNode->key : startNode->value) != NULL) {			
			if(strcmp(cmp, searchStr) == 0)
				break;
		}

		startNode = startNode->nextNode;
	}
	
	if(ignoreCase)
		delete(searchStr);

	return (cell)startNode;

}


AMX_NATIVE_INFO vdfNatives[] = 
{
	{"vdf_open",					vdf_open},
	{"vdf_save",					vdf_save},
	{"vdf_parse",					vdf_parse},
	{"vdf_get_first_node",			vdf_get_first_node},
	{"vdf_get_child_node",			vdf_get_child_node},
	{"vdf_get_parent_node",			vdf_get_parent_node},
	{"vdf_get_root_node",			vdf_get_root_node},
	{"vdf_get_last_node",			vdf_get_last_node},
	{"vdf_get_previous_node",		vdf_get_previous_node},
	{"vdf_get_next_node",			vdf_get_next_node},
	{"vdf_get_node_key",			vdf_get_node_key},
	{"vdf_get_node_value",			vdf_get_node_value},
	{"vdf_get_node_value_vector",	vdf_get_node_value_vector},
	{"vdf_get_node_value_float",	vdf_get_node_value_float},
	{"vdf_get_node_value_num",		vdf_get_node_value_num},
	{"vdf_set_node_key",			vdf_set_node_key},
	{"vdf_set_node_value",			vdf_set_node_value},
	{"vdf_set_node_value_num",		vdf_set_node_value_num},
	{"vdf_set_node_value_float",	vdf_set_node_value_float},
	{"vdf_set_node_value_vector",	vdf_set_node_value_vector},
	{"vdf_delete_node",				vdf_delete_node},
	{"vdf_append_node",				vdf_append_node},
	{"vdf_append_child_node",		vdf_append_child_node},
	{"vdf_create_tree",				vdf_create_tree},
	{"vdf_count_branch_nodes",		vdf_count_branch_nodes},
	{"vdf_remove_tree",				vdf_remove_tree},
	{"vdf_create_search",			vdf_create_search},
	{"vdf_find_next_match",			vdf_find_next_match},
	{"vdf_set_search",				vdf_set_search},
	{"vdf_get_node_level",			vdf_get_node_level},
	{"vdf_close_search",			vdf_close_search},
	{"vdf_sort_branch",				vdf_sort_branch},
	{"vdf_move_to_branch",			vdf_move_to_branch},
	{"vdf_move_as_child",			vdf_move_as_child},
	{"vdf_find_in_branch",			vdf_find_in_branch},
	{"vdf_next_in_traverse",        vdf_next_in_traverse},
	{NULL,							NULL},
};

void OnAmxxAttach()
{
	vdfCollection.SetLogger(&logger);
	MF_AddNatives(vdfNatives);
}

void OnAmxxDetach()
{
	vdfCollection.Destroy();
}
