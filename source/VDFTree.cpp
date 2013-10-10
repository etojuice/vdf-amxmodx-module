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

#include <stdlib.h>
#include <string.h>

#include "VDFTree.h"


// --- VDFTree class implementation ---

VDFTree::VDFTree()
{
	nodeCount    =  0;
	rootNode     =  NULL;
	nodeIndex	 =  NULL;
	treeId		 =  0;
}

VDFTree::~VDFTree()
{
	DestroyTree();
}


/**
 *  Initializes tree root node
 */
void VDFTree::CreateTree()
{
	if(rootNode != NULL) {
		DestroyTree();
	}
	rootNode = CreateNode();
}

/**
 * 	Creates an empty node and adds it to node index
 *
 * 	@param	parentNode	node's parent, not required (default = NULL)
 * 	@return				The created node or NULL if it fails.
 */
VDFNode *VDFTree::CreateNode(VDFNode *parentNode)
{
	VDFNode  *node;
	node = new VDFNode;
	return node;
}

/**
 *  Appends a new node into a given node level
 *
 *	@param	Node		pointer to node to get the level
 *	@param	newNode		pointer to node to be added
 */
void VDFTree::AppendNode(VDFNode *Node, VDFNode *newNode)
{
	VDFNode *lastNode;
	
	if(Node == NULL)
		return;

	lastNode = GetLastNode(Node);
	lastNode->nextNode = newNode;
	newNode->parentNode = lastNode->parentNode;
	newNode->previousNode = lastNode;
}

/**
 *	Adds a child node into a given node.
 *
 *  @param	Node		node pointer which the child will be added, 
 *						if the node already has a child,<br>
 *						it's appended to node's child level.
 *  @param	childNode	pointer to the noded to be appended as a child
 */
void VDFTree::AppendChild(VDFNode *Node, VDFNode *childNode)
{
	if(Node == NULL)
		return;

	childNode->parentNode = Node;

	if(Node->childNode)
		AppendNode(Node->childNode, childNode);
	else
		Node->childNode = childNode;
}

/**
 *  Tree memory freeing.
 */
void VDFTree::DestroyTree()
{
	if(this->rootNode)
	{

		DeleteNode(this->rootNode);
		FinalizeArray(rootNode->key);
		FinalizeArray(rootNode->value);
		delete this->rootNode;

		this->rootNode = NULL;
	}

}

/**
 *	Get the node in tree index
 *
 *  @param	id		index of the node.
 *  @return			The target node or NULL if not valid or not found.
 */
VDFNode *VDFTree::GetNodeById(UINT id)
{
	if(id < nodeCount)
		return nodeIndex[id];
	return NULL;
}

/**
 *	Gets the node next to a given one.
 *
 *  @param	Node		node to get the next one.	
 *  @return				Returns the next node or NULL if it doesn't
 *						have a next node.
 */ 
VDFNode *VDFTree::GetNextNode(VDFNode *Node)
{
	return Node->nextNode;
}

/**
 *	Gets the first node from a given node level.
 *
 *	@param	Node		node to get level in tree
 *  @return				The first node in that level.
 */
VDFNode *VDFTree::GetFirstNode(VDFNode *Node)
{
	VDFNode *firstNode;
	firstNode = (Node->parentNode) ? Node->parentNode->childNode : VDFTree::GetRootNode(Node);
	return firstNode;
}

/**
 *	Gets the last node from a given node level
 *
 *	@param	Node		node to get level in tree
 *  @return				The last node in that level.
 */
VDFNode *VDFTree::GetLastNode(VDFNode *Node)
{
	VDFNode *lastNode;
	
	lastNode = Node;

	while(true) {
		if(!lastNode->nextNode)
			break;
		lastNode = lastNode->nextNode;
	}
	return lastNode;
}

/**
 *	Although the tree object has a 'rootNode' pointer, this static
 *	function is designed to<br>find the rootNode without the 
 *  vdf tree information.
 *
 *	@param	Node		Node to get the previous one.
 *  @return				The previous node or NULL if there's no previous.
 */
VDFNode *VDFTree::GetRootNode(VDFNode *Node)
{
	VDFNode *rootNode;
	rootNode = Node;
	while(rootNode->parentNode)
		rootNode = rootNode->parentNode;
	while(rootNode->previousNode)
		rootNode = rootNode->previousNode;
	return rootNode;
}

/**
 *	Gets the previous node from a given node.
 *
 *	@param	Node		Node to get the previous one.
 *  @return				The previous node or NULL if there's no previous.
 */
VDFNode *VDFTree::GetPreviousNode(VDFNode *Node)
{
	return Node->previousNode;
}

/**
 *	Gets the next node in traverse.
 *  @param  node    Origin of traverse
 *  @param  depth   Relative depth of the traverse node from the origin (1st parameter)
 */
VDFNode  *VDFTree::GetNextTraverseStep(VDFNode *node, int &depth)
{
	VDFNode* trav;
	VDFNode* temp;

	trav = node;	
	
	if( (temp = trav->childNode) ) {
		trav = temp;
		depth++;
	}
	else {		
		if(! (temp = trav->nextNode) ) {

			for(temp = NULL; temp == NULL; temp = trav->nextNode) {
				trav = trav->parentNode;
				if(trav == NULL || trav == node) break;
				depth --;
			}
		}
		trav = temp;		
	}

	return trav;
}

/**
 *	Gets the number of a nodes on a given node.
 *
 *	@param	refNode		Node to check level.
 *  @return				The previous node or NULL if there's no previous.
 */
size_t VDFTree::CountBranchNodes(VDFNode *refNode)
{
	VDFNode *firstNode;
	size_t counter;

	firstNode = GetFirstNode(refNode);
	counter = 0;

	while(firstNode) {
		counter++;
		firstNode = firstNode->nextNode;
	}

	return counter;
}

/**
 *	Sorts nodes in a branch.
 *	@param	refNode		Pick up the branch of this node
 *	@param	byKey		If true sort nodes by key, otherwise use values.
 *	@param	byNumber	If true keys/values are considered as numbers (integers) for sorting.
*/
void VDFTree::SortBranchNodes(VDFNode *refNode, bool byKey, bool byNumber)
{
	VDFNode *firstNode;
	VDFNode *parentNode;
	VDFNode *Node;
	VDFNode **sort;
	size_t length;
	int	 ind;
	char *comp1;
	char *comp2;
	int	 guide;
	int	 p,q;
	int p1;
	bool comp;

	

	firstNode = VDFTree::GetFirstNode(refNode);

	if(firstNode == NULL)
		return;

	parentNode = firstNode->parentNode;
	length = VDFTree::CountBranchNodes(firstNode);
	
	if(!length)
		return;
	
	sort = new VDFNode*[length];
	memset(sort, 0, length * sizeof(VDFNode*));
	
	Node = firstNode;
	ind = 0;

	for(;Node; Node = Node->nextNode)
		sort[ind++] = Node;

	/*while(Node) {
		sort[ind++] = Node;
		Node = Node->nextNode;
	}*/
	
	
	// shell sort
	guide = 1;

	while(guide < (int)length)
		guide *= 2;

	while(guide /= 2) {
		
		for(p = 0; p < (int)length - guide; p++) {
			
			p1 = p;

			while(p1 > -1) {

				q = p1 + guide;
				
				if(q >= (int)length)
					break;			
				
				
				comp2 = (byKey) ? sort[p1]->key : sort[p1]->value;
				comp1 = (byKey) ? sort[q]->key : sort[q]->value;

				if(comp1 == NULL)
					continue;
				
				comp = (comp2 == NULL) ? false : (!byNumber) ? (strcmp(comp1, comp2) < 0) 
						: atoi(comp1) < atoi(comp2);

				if(comp) {
					Node = sort[q];
					sort[q] = sort[p1];
					sort[p1] = Node;
				}
				p1 -= guide;
			}
		}
	}

	for(ind = 0; ind < (int)length; ind++) {		
		sort[ind]->nextNode = (ind + 1 < (int)length) ? sort[ind + 1] : NULL;
		sort[ind]->previousNode = (ind - 1 >= 0) ? sort[ind - 1] : NULL;
	}

	if(parentNode)
		parentNode->childNode = sort[0];
	else if(firstNode == rootNode)
		this->rootNode = sort[0];

	delete(sort);

}

bool VDFTree::IsTreeNode(VDFNode *node) {
	
	size_t count;
	
	for(count = 0; count < this->nodeCount; count++) {
		if(this->nodeIndex[count] == node)
			return true;
	}

	return false;

}

void VDFTree::MoveAsChild(VDFNode *parentNode, VDFNode *moveNode)
{
	VDFNode *childNode;
	VDFNode *movePrevious;
	VDFNode *moveNext;
	VDFNode *moveParent;	

	if( parentNode == NULL ||
		moveNode == NULL ||
		moveNode->parentNode == parentNode)
		return;

	if(!IsTreeNode(moveNode) || !IsTreeNode(parentNode))
		return;

	movePrevious = moveNode->previousNode;
	moveNext = moveNode->nextNode;
	moveParent = moveNode->parentNode;
	childNode = parentNode->childNode;

	if(childNode == NULL) {
		parentNode->childNode = moveNode;
		moveNode->nextNode = NULL;
		moveNode->previousNode = NULL;
	}

	else {
		childNode = VDFTree::GetLastNode(childNode);
		childNode->nextNode = moveNode;
		moveNode->nextNode = NULL;
		moveNode->previousNode = childNode;
	}

	moveNode->parentNode = parentNode;

	if(moveParent) {
		if(moveNext) {
			moveParent->childNode = moveNext;
			moveNext->previousNode = NULL;
		}
		else
			moveParent->childNode = NULL;
	}
}

/**
 *	Moves a node to other branch.
 *	@param	refNode		Reference node.
 *	@param	moveNode	Node to be moved.
 *	@param	byNumber	If true keys/values are considered as numbers (integers) for sorting.
*/
void VDFTree::MoveToBranch(VDFNode *refNode, VDFNode *moveNode, UINT position)
{
	VDFNode *targetPrevious;
	VDFNode *targetNext;
	VDFNode *srcPrevious;
	VDFNode *srcNext;

	if(refNode == NULL || moveNode == NULL || refNode == moveNode)
		return;

	if(!IsTreeNode(refNode) || !IsTreeNode(moveNode))
		return;
	
	targetPrevious = refNode->previousNode;
	targetNext = refNode->nextNode;
	srcPrevious = moveNode->previousNode;
	srcNext = moveNode->nextNode;

	if(position == VDF_MOVEPOS_AFTER) {

		if(srcPrevious == NULL) {
			if(moveNode->parentNode)
				moveNode->parentNode->childNode = srcNext;
		}

		refNode->nextNode = moveNode;
		moveNode->previousNode = refNode;
		moveNode->nextNode = targetNext;
		if(targetNext != NULL)
			targetNext->previousNode = moveNode;
		
		if(moveNode == rootNode)
			rootNode = GetRootNode(moveNode);

	}	
	else {
		if(srcPrevious == NULL) {			
			if(moveNode->parentNode)
				moveNode->parentNode->childNode = srcNext;
		}
		
		refNode->previousNode = moveNode;
		moveNode->nextNode = refNode;
		moveNode->previousNode = targetPrevious;
		
		if(moveNode == rootNode)
			rootNode = GetRootNode(moveNode);
		else if(refNode == rootNode)
			rootNode = moveNode;

	}
	
	if(srcNext)
		srcNext->previousNode = srcPrevious;
	
	if(srcPrevious)
		srcPrevious->nextNode = srcNext;

}

/**
 *	Gets the node depth in a tree
 *	@param	Node	Node to check.
 *	@return			Node depth.
 */
size_t VDFTree::GetNodeLevel(VDFNode *Node)
{
	size_t counter;

	counter = 0;

	while((Node = Node->parentNode))
		counter++;

	return counter;
}

/**
 *	Deletes a node in a given tree's node index.
 *
 *	@param	index	Index of the node to be deleted.
 */
/*void VDFTree::DeleteNode(UINT index)
{
	if(index < nodeCount) {
		if(nodeIndex[index])
			DeleteNode(nodeIndex[index]);
	}
}*/

/**
 *	Deletes a node in vdf tree.
 *
 *	@param	Node	Node to be deleted.
 */
void VDFTree::DeleteNode(VDFNode *Node)
{
	VDFNode *temp;
	VDFNode *next;
	
	temp = Node;

	while(Node != NULL && temp != NULL) {
		if(temp->childNode) {
			temp = temp->childNode;
			continue;
		}

		if(temp == Node)
		{
			if( temp->parentNode ) {
				if(temp->parentNode->childNode == temp) {
					temp->parentNode->childNode = temp->nextNode;
				}				
			}
			if(temp->nextNode) {
				temp->nextNode->previousNode = temp->previousNode;
				//if(Node == rootNode) rootNode = temp->nextNode;
			} //else if(Node == rootNode) rootNode = this->CreateNode();
			if(temp->previousNode) {
				temp->previousNode->nextNode = temp->nextNode;
			}
			next = NULL;
		} else {
			if( temp->parentNode ) {
				temp->parentNode->childNode = temp->nextNode;
			}
			if( temp->nextNode ) {
				temp->nextNode->previousNode = NULL;
				next = temp->nextNode;
			} else {
				next = temp->parentNode;
			}
		}

		if(temp != rootNode) {
			FinalizeArray(temp->key);
			FinalizeArray(temp->value);	
			Finalize(temp);
		}

		temp = next;


	}
}

/**
 *	Gets the actual number of nodes in a tree.
 *
 *	@return		The number of nodes in a tree.
 */
size_t VDFTree::GetLength()
{
	UINT ind;
	size_t length;
	length = 0;
	for(ind = 0; ind < nodeCount; ind++) {
		if(nodeIndex[ind])
			length++;
	}
	return length;
}


/**
 *	Sets key/value in a tree. Use NULL if you need to set
 *	only key or only value.
 *
 *	@param	Node	Target node
 *	@param	key		Node key, if NULL it's ignored.
 *	@param	value	Node value, if NULL it's ignored.
 */
void VDFTree::SetKeyPair(VDFNode *Node, const char *key, const char *value)
{
	if(key) {
		if(Node->key)
			delete(Node->key);
		Node->key = new char[strlen(key) + 1];
		strcpy(Node->key, key);
	}

	if(value) {
		if(Node->value)
			delete(Node->value);
		Node->value = new char[strlen(value) + 1];
		strcpy(Node->value, value);
	}
}


