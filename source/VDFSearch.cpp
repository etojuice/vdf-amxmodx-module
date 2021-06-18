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


#include "VDFSearch.h"



// --- VDFSearch Implementation ---

VDFSearch::VDFSearch()
{	
	cmpBuffer = new char[512];
	searchBuffer = new char[512];
	Reset();
}

VDFSearch::~VDFSearch()
{
	FinalizeArray(cmpBuffer);
	FinalizeArray(searchBuffer);
}


/**
 *	Finds next node in current search
 *	@return	true if found a/another node that matches search
 */
bool VDFSearch::FindNext()
{	
	if(!*searchBuffer)
		return false;

	bool	ignoreTrav;

	VDFNode *anchor;

	ignoreTrav = false;


	if(cursor == NULL) {
		cursor = searchTree->rootNode;
	}

	while(cursor)
	{		
		if(Match(cursor))
			return true;

		else if(searchLevel > -1) // search optimization - ignores undesired levels
		{
			
			if(currentLevel >= searchLevel)
			{
				if(cursor->nextNode && currentLevel == searchLevel) 
				{
					ignoreTrav = true;
					cursor = cursor->nextNode;
				} else
				{
					// try to jump into the closest neighbor
					anchor = cursor;
					while( (anchor = anchor->parentNode) )
					{
						currentLevel --;
						if(currentLevel <= searchLevel && anchor->nextNode)
						{
							anchor = anchor->nextNode;
							ignoreTrav = true;
							break;
						}
					}
					if(!anchor) break; 
					cursor = anchor;					
				}
			}
		}

		if(!ignoreTrav)  cursor = VDFTree::GetNextTraverseStep( cursor, currentLevel );
		else ignoreTrav = false;		
		
	}
	
	return false;
}

/**
 *	Sets the correct position of a search before calling FindNext
 *	@param	refNode	Search after this one.
 *	@return			Returns the node or NULL on end.
 */
VDFNode *VDFSearch::FindNextNode(VDFNode *refNode)
{
	size_t level;

	if(refNode != cursor) {

		if(refNode != NULL) {
			level = VDFTree::GetNodeLevel(refNode);
			currentLevel = (int)level;
			cursor = refNode;
		}
		else {
			cursor = NULL;
			currentLevel = 0;
		}
	} else {
		if(refNode && cursor) {
			if(!(cursor = VDFTree::GetNextTraverseStep(cursor, currentLevel))) {
				return NULL;
			}
		}
	}
	if(FindNext())
		return cursor;
	return NULL;
}		


/**
 *	Checks if a node matches with the current search.
 *	@param	VDFNode*	Node to compare.
 *	@return				true if it matches.
 */
bool VDFSearch::Match(VDFNode *matchNode)
{
	char	*param;
	bool	match;
	bool	mayCheckMatch;

	mayCheckMatch = (searchLevel > -1) ? (currentLevel == searchLevel) : true;
	
	if(!mayCheckMatch)
		return false;	

	*cmpBuffer = '\0';
	param = (searchFlags & VDF_MATCH_VALUE) ? matchNode->value : matchNode->key;

	if(param == NULL)
		return false;

	if(strcmp(searchBuffer, "*") == 0)
		return true;

	if(searchFlags & VDF_IGNORE_CASE) {		
		ToLowerCase(param, cmpBuffer);
		param = cmpBuffer;
	}	

	match = (strcmp(param, searchBuffer) == 0);


	return match;	
}



/**
 *	Sets a new search, if there's already a search taking place, it's reset.
 *	@param	schTree		Vdf tree to look into.
 *	@param	search		Search string.
 *	@param	flags		Default search is for key case sensitive,
 *						might be changed by setting:<br><code>
 *						VDF_MATCH_VALUE </code><br>and<code>
 *						VDF_IGNORE_CASE
 *	@param	level		Level in which search will be valid. When -1 (default)
 *						all levels match.				
 */
void VDFSearch::SetSearch(VDFTree *schTree, char *search, UINT flags, int level)
{
	Reset();
	searchLevel =	level;
	searchFlags =	flags;
	searchTree	=	schTree;

	
	if(flags & VDF_IGNORE_CASE)
		ToLowerCase(search, searchBuffer); 
	else 	
		strcpy(searchBuffer, search);
}

/**
 *	Resets search.
 */

void VDFSearch::Reset()
{
	*searchBuffer = '\0';
	searchLevel	=	-1;
	searchFlags	=	VDF_MATCH_KEY;
	currentLevel = 0;
	cursor = NULL;
}


