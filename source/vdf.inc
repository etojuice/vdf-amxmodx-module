
#if defined _vdf_included
  #endinput
#endif
#define _vdf_included

#if AMXX_VERSION_NUM >= 175
 #pragma reqlib vdf
 #if !defined AMXMODX_NOAUTOLOAD
  #pragma loadlib vdf
 #endif
#else
 #pragma library vdf
#endif

#define VDF_NULL_TREE 	VdfTree:0
#define VDF_NULL_NODE 	VdfNode:0
#define VDF_NULL_SEARCH VdfSearch:0

#define VDFPARSER_CONTINUE 0
#define VDFPARSER_STOP 1

#define VDF_OPEN_CONTINUE 0
#define VDF_OPEN_SILENT 1
#define	VDF_OPEN_BREAK 2

#define VDF_MATCH_KEY 0
#define VDF_MATCH_VALUE 1



/** 
 *	Opens a vdf tree. Optionally you can handle nodes addition event, the forwarded function has
 *	the following syntax :
 *	<code>(const filename[], VdfTree:tree, VdfNode:node, level)</code>
 *	The return types for the forwarded function are:
 *	<li>VDF_OPEN_CONTINUE - keep moving.</li>
 *	<li>VDF_OPEN_SILENT - open process will go on without calling forward anymore.</li>
 *	<li>VDF_OPEN_BREAK - open process is interrupted generating a partial tree.</li> 
 *
 *	@param	filename	File to be opened.
 *	@param	node_added	(optional) Function to be fired when a new node is added.
 *	@return				If file exists returns the vdf tree otherwise 0.
 */
native VdfTree:vdf_open(const filename[], const node_added[] = "");


/** 
 *	Saves a vdf tree. 
 *	@param vdftree	Tree to be saved.
 *	@param saveas	Alternative filename.
 *	@return			If it's a valid vdf saves the file. Returns 0 on error.
 */
native vdf_save(VdfTree:tree, const saveas[] = "");


/**
 *	Gets the root node of a tree.
 *	@param vdftree	Target tree.
 *	@return			The root node of vdf tree, or 0 on error
 */
native VdfNode:vdf_get_root_node(VdfTree:tree);


/**
 *	Parses a tree using event model - no tree object is created.<br>
 *	It may be stopped by returning VDFPARSER_STOP.
 *	In order to handle events, the fowarded functions should support these formats:
 *	<ul>
 *	<li>keypairs_func : <code>(const filename[], const key[], const value[], level)</code></li>
 *	<li>start_func : <code>(const filename[])</code></li>
 *	<li>end_func : <code>(const filename[])</code></li>
 *
 *	@param	filename		Name of the file to be parsed.
 *	@param	keypairs_func	Function to be fired when a new key pair is read.
 *	@param	start_func		(optional) Function to be fired when parsing starts.
 *	@param	end_func		(optional)	Function to be fired when parsing is over.
 */
native vdf_parse(const filename[], const keypairs_func[], const start_func[] = "", const end_func[] = "");


/** 
 *	Gets the first node of a tree branch.
 *	@param	node	Node in the target branch.
 *	@return			The first node in this branch, or 0 on error.
 */
native VdfNode:vdf_get_first_node(VdfNode:node);


/**
 *	Gets the last node of a tree branch.
 *	@param	node	Node in target branch.
 *	@return			The last node is this branch, or 0 on error
 */
native VdfNode:vdf_get_last_node(VdfNode:node);


/**
 *	Gets the previous node.
 *	@param	node	Node to get previous.
 *	@return			The previous node, or 0 if it doesn't have a previous on error
 */
native VdfNode:vdf_get_previous_node(VdfNode:node);


/**
 *	Gets the parent node.
 *	@param	node	Node to get parent.
 *	@return			The parent node, 0 on error or if it doesn't have a parent
 */
native VdfNode:vdf_get_parent_node(VdfNode:node);


/**
 *	Gets the child node.
 *	@param	node	Node to get the child.
 *	@return			The child node, 0 on error or if doesn't have a child
 */
native VdfNode:vdf_get_child_node(VdfNode:node);


/**
 *	Gets the next node.
 *	@param	node	Node to get the next.
 *	@return			The next node, 0 on error or if doesn't have a next node
 */
native VdfNode:vdf_get_next_node(VdfNode:node);

/**
 *	Gets the pointer of the next node in tree traverse
 *  @param  node    Node position to get next in traverse
 *  @param  depth	Relative depth of the next node in traverse
 *	@return	Pointer to the next node in tree traverse.
 */
native VdfNode:vdf_next_in_traverse(VdfNode:node, &depth = 0);


/**
 *	Deletes a node from a vdf tree.
 *	@param	vdftree	Target tree.
 *	@param	node	Target node.
 */
native vdf_delete_node(VdfTree:tree, VdfNode:node);


/** 
 *	Gets the key of a node.
 *	@param	node	Node to get key.
 *	@param	key		Copy key to this string.
 *	@param	maxlen	Max string size.
 */ 
native vdf_get_node_key(VdfNode:node, key[], maxlen);


/** 
 *	Gets the value of a node.
 *	@param	node	Node to get value.
 *	@param	value	Copy value to this string.
 *	@param	maxlen	Max string size.
 */ 
native vdf_get_node_value(VdfNode:node, value[], maxlen);


/**
 *	Sets the key of a node.
 *	@param	node	Node to set key.
 *	@param	key		New key.
 */  
native vdf_set_node_key(VdfNode:node, const key[]);


/** 
 *	Sets the value of a node.
 *	@param	node	Node to set value.
 *	@param	value	New value.
 */ 
native vdf_set_node_value(VdfNode:node, const value[]);

/**
 *	Converts number to string a copies to value.
 *	@param	node	Node to set value.
 *	@param	value	Value as an integer.
 */
native vdf_set_node_value_num(VdfNode:node, value);


/**
 *	Converts float to string and copies to value. 
 *	@param	node	Node to set value.
 *	@param	value	Value as a float.
 */
native vdf_set_node_value_float(VdfNode:node, Float:value);


/**
 *	Formats a vector into a string and copies to value. 
 *	@param	node	Node to set value.
 *	@param	value	Value as a vector.
 */
native vdf_set_node_value_vector(VdfNode:node, Float:vector[3]);


/** 
 *	Creates a vdf tree with one node (rootnode).
 *  You've got to call vdf_save to save it into a file (it's not created in this function).
 *  @param	filename	Name of new vdf file.
 *	@return				The new vdf tree
 */
native VdfTree:vdf_create_tree(const filename[]);


/**
 *	Appends a new node into a existing one
 *	@param	vdftree		Target tree.
 *	@param	destnode	Target node.
 *	@param	key			New node key.
 *	@param	value		New node value.
 *	@return				The new appended node
 */
native VdfNode:vdf_append_node(VdfTree:tree, VdfNode:destnode, key[] = "", value[] = "");


/**
 *	Appends a new child node into a node.
 *	@param	vdftree		Target tree.
 *	@param	destnode	Target node.
 *	@param	key			New node key.
 *	@param	value		New node value.
 *	@return				The new appended child node
 */
native VdfNode:vdf_append_child_node(VdfTree:tree, VdfNode:destnode, key[] = "", value[] = "");


/**
 *	Removes a tree.
 *	@param	vdftree		Target tree.
 */
native vdf_remove_tree(VdfTree:tree);


/**
 *	Counts the number of nodes in a branch
 *	@param	node	Reference node (there are no "branch" specific handlers 
 *					so nodes are used as reference to get a branch).
 *	@return			Number of nodes on that branch.
 */
native vdf_count_branch_nodes(VdfNode:node);


/**
 *	Creates a new search
 *	@return	The new search pointer. It's important to store it in a variable
 *			to set up a 'search' properly in vdf_set_search.
 */
native VdfSearch:vdf_create_search();


/**
 *	Sets a new search.
 *	@param	search		Search pointer.
 *	@param	tree		Tree pointer.
 *	@param	searchstr	String to look up.
 *	@param	searchtype	Search for key or value - VDF_MATCH_KEY or VDF_MATCH_VALUE.
 *						Note: searching for both key and at the same time isn't supported.
 *	@param	level		Checks which level to perform search. If -1 (default) all levels match.
 *	@param	ignoreCase	If different from 0 it doesn't match case.
 */
native vdf_set_search(VdfSearch:search, VdfTree:tree, const searchstr[], searchtype = 0, level = -1, ignorecase = 0);


/**
 *	Checks next match.
 *	@param	search		Search pointer.
 *	@param	startnode	Start at this node (0 to search from tree beginning)
 *	@return				Next match (node) or 0 if nothing is found.
 */
native VdfNode:vdf_find_next_match(VdfSearch:search, VdfNode:startnode = VDF_NULL_NODE);


/**
 *	Closes a search.
 *	@param	search	Search pointer.
 */
native vdf_close_search(VdfSearch:search);

/**
 *	Gets the depth of a node in a tree.
 *	@param node Node to get depth.
 *	@return		The node level. As 0 is a valid depth it returns -1 on error.
 */
native vdf_get_node_level(VdfNode:node);


/**
 *	Gets a node value as a num.
 *	@param	node	Node to get the value.
 *	@return			The node value as an integer.
 */
native vdf_get_node_value_num(VdfNode:node);


/**
 *	Gets a node value as a float.
 *	@param	node	Node to get the value.
 *	@param	value	The node value as a float.
 */
native Float:vdf_get_node_value_float(VdfNode:node);


/**
 *	Gets a node value as a float vector.
 *	The string may be split by spaces or commas.
 *	Value examples: "12.3 -43.2 333.2"; "123.4,1298.0,190.1"
 *	@param	node	Current node.
 *	@param	value	The node value as a float vector.
 */
native vdf_get_node_value_vector(VdfNode:node, Float:vector[3]);

/**
 *	Find nodes in a branch.
 *	@param	node		Start node.
 *	@param	schstring	String to search.
 *	@param	bykey		If true looks for a key that matches string otherwise,
 *						looks for a value.
 *	@param	ignorecase	If true, ignores case to match.
 */
native VdfNode:vdf_find_in_branch(VdfNode:node, const schstring[], bool:bykey = true, bool:ignorecase = false) 


/**
 *	Sorts nodes in a branch.
 *	@param	refnode		Pick up the branch of this node
 *	@param	bykey		If true sort nodes by key, otherwise use values.
 *	@param	asnumbers	If true keys/values are considered as numbers (integers) for sorting.
*/
native vdf_sort_branch(VdfTree:tree, VdfNode:refnode, bool:bykey = true, bool:asnumbers = false)


/**
 *	Moves a tree node to other branch.
 *	@param	movenode		Node to be moved.
 *	@param	anchornode		Reference node.
 *	@param	byNumber		If true keys/values are considered as numbers (integers) for sorting.
*/
native vdf_move_to_branch(VdfTree:tree, VdfNode:movenode, VdfNode:anchornode, bool:insertafter = true); 


/**
 *	Moves a child node to another parent.
 *	@param	movenode	Node to be moved.
 *	@param	parentnode	New parent node.
 *	@param	byNumber	If true keys/values are considered as numbers (integers) for sorting.
*/
native vdf_move_as_child(VdfTree:tree, VdfNode:movenode, VdfNode:parentnode);


/**
 *	Checks if it's the last node in a branch.
 *	@param	node	Check this node.
 *	@return			1 if it's the last branch node, otherwise 0.
 */
stock VdfNode:vdf_is_last_node(VdfNode:node) return (vdf_get_next_node(node)) ? 0 : 1;


/**
 *	Checks if it's the first node in a branch.
 *	@param	node	Check this node.
 *	@return			1 if it's the first branch node, otherwise 0.
 */
stock VdfNode:vdf_is_first_node(VdfNode:node) return (vdf_get_previous_node(node)) ? 0 : 1;

