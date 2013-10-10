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
#include <stdio.h>
#include <ctype.h>
#include "VDFParser.h"

static int spaceChars[256] = {0};




/**
 * Finds the next symbol in current parsing file
 * @param 
 *
 */
int VDFReader::GetNextSymbol(char **target, int tokenMax)
{
	//Token newToken = {0};
	int tokenPos = 0;
	//int lineLength = (int) strlen( (line + cursor ) );
	bool tokenOpened = false;

	for(;line[cursor]; cursor++) 
	{

		if(!tokenOpened)
		{
			// eat spaces
			while(line[cursor] && spaceChars[(int)line[cursor]]) cursor++;

			// new kv string starting
			if(line[cursor] == '\"')
			{
				tokenOpened = true;
				*target = &line[cursor + 1];
				continue;
			}
			// closing branch
			if(line[cursor] == '}') {cursor++; return KV_CLOSE;}
			// opening branch
			if(line[cursor] == '{') {cursor++; return KV_OPEN;}

			if(line[cursor] == '/' && line[cursor + 1] == '/')
			{
				return KV_NONE;
			}

		} else 
		{
			// end of kv string
			if(line[cursor] == '\"')
			{
				*( *target + tokenPos ) = '\0';
				cursor++;
				
				return KV_NEWSTRING;
			}
			// add char to current string marker
			if (tokenPos < tokenMax) {
				tokenPos++;					
			}
		}		

	}
	return KV_NONE;
}

VDFReader::VDFReader(IErrorLogger *logger)
{
	this->pFile = NULL;
	this->logger = logger;
	spaceChars[(int)'\t'] = 1;
	spaceChars[(int)' '] = 1;
}

void VDFReader::Open()
{
	this->Close();
	
	if(filename) this->pFile = fopen(filename, "r");
	else this->pFile = NULL;

	this->currentDepth = 0;
	line[0] = '\0';
	lineLength = 0;
	lineCounter = 0;
	status = 1 << KV_EXP_NEWKV;
}

void VDFReader::Open(const char* filename)
{
	this->filename = filename;
	this->Open();
}					

void VDFReader::Close()
{
	if(this->pFile)
	{
		fclose(pFile);
		pFile = NULL;
	}
}


bool VDFReader::NextKeyValue()
{
	unsigned int max;
	char **target;
	bool keyRead;
	int res;

	if(!pFile) return false;

	char *pKey = NULL;
	char *pValue = NULL;

	keyRead = false;	

	while(true)
	{		
		if(! (*line) ||  cursor >= lineLength) {
			if(!fgets(line, MAX_LINE_SIZE, pFile)) break;
			lineCounter++;
			lineLength = strlen(line);
			cursor = 0;
		}
		
		if(strlen(line) == 0) {
			continue;
		}

		if(keyRead) {
			max = 512;
			target = &pValue;
		} else {
			max = 256;
			target = &pKey;
		}
		
		res = GetNextSymbol(target, max);

		if(res <= KV_NEWSTRING && (! (status & (1 << res))) )
		{
			if(this->logger)
				logger->printError(this->filename, "unexpected symbol", lineCounter, cursor);
			// TODO: keep parsing on error should be an option in this class.
			continue;
		}
	
		switch(res)
		{
			case KV_CLOSE :
				if(currentDepth > 0) 
				{
					currentDepth --;
					if( currentDepth == 0) return false;
					status = 1 << KV_EXP_CLOSE | 1 << KV_EXP_NEWKV;
					keyRead = false;
					
				} else {
					// error
				}
				break;
			case KV_OPEN :		
				currentDepth++;
				status = 1 << KV_EXP_CLOSE | 1 << KV_EXP_NEWKV;
				if(keyRead) {
					DispatchToParser(pKey, pValue, currentDepth - 1);
					keyRead = false;
					return true;
				} else {
					
				}
				break;
			case KV_NEWSTRING :
				if(keyRead) 
				{
					status = 1 << KV_EXP_CLOSE | 1 << KV_EXP_NEWKV;
					DispatchToParser(pKey, pValue, currentDepth);
					keyRead = false;
					return true;
				} else
				{
					status = 1 << KV_EXP_OPEN | 1 << KV_EXP_CLOSE | 1 << KV_EXP_NEWKV;
					keyRead = true;
				}
				break;
			case KV_NONE :
				*line = '\0';
				if(keyRead)						{
					DispatchToParser(pKey, pValue, currentDepth);						
					return true;
				}
		}
	}
	return false;
}

bool VDFEventReader::ParseVDF(const char *filename, ParseForward *pFW)
{
	if(pFW == NULL)
		return false;

	this->currentParser = pFW;

	if(filename == NULL || pFW->pfnParser == NULL)
		return false;

	this->Open(filename);
	if(!this->pFile) return false;

	this->returnVal = RETURN_VDFPARSER_CONTINUE;

	if(pFW->pfnStart != NULL)
		(*(pFW->pfnStart))(pFW->fwidStart, pFW->mdFilename);	

	while(this->NextKeyValue()) {
		if(this->returnVal == RETURN_VDFPARSER_STOP) break;
	}

	if(pFW->pfnEnd != NULL)
		(*(pFW->pfnEnd))(pFW->fwidEnd, pFW->mdFilename);

	this->Close();

	return true;

}

void VDFEventReader::DispatchToParser(const char *key, const char *value, UINT depth)
{	
	this->returnVal = (*(currentParser->pfnParser))(currentParser->fwidParser, currentParser->mdFilename, key, value, depth);
}


bool VDFTreeFile::OpenVDF(const char *filename, VDFTree **vdfTree, OpenForward *openFW)
{	

	if(filename == NULL) {
		return false;
	}
	
	this->Open(filename);
	if(this->pFile == NULL) return false;

	this->currentParser = openFW;

	// make sure output tree is empty
	if(*vdfTree) {
		delete(*vdfTree);
		*vdfTree = NULL;
	}

	*vdfTree = new VDFTree;
	this->currentTree = *vdfTree;
	this->returnVal = RETURN_TREEPARSER_CONTINUE;

	this->currentNode = (*vdfTree)->rootNode;
	this->currentDepth = 0;

	while(this->NextKeyValue()) {
		if(this->returnVal == RETURN_TREEPARSER_BREAK) break;
	}

	this->Close();

	if(!currentTree->rootNode) currentTree->CreateTree();

	return true;

}

void VDFTreeFile::DispatchToParser(const char *key, const char *value, UINT depth)
{	

	VDFNode *refNode = currentNode;
	currentNode = this->currentTree->CreateNode();
	VDFTree::SetKeyPair(currentNode, key, value);
	
	if(depth > currentDepth)
	{	
		VDFTree::AppendChild(refNode, currentNode);		
	} else if(depth < currentDepth)
	{
		while(depth < currentDepth--)
			refNode = refNode->parentNode;
		VDFTree::AppendNode(refNode, currentNode);
	} else
	{
		if(!depth) currentTree->rootNode = currentNode;
		else VDFTree::AppendNode(refNode, currentNode);
	}

	currentDepth = depth;

	if(currentParser && this->returnVal != RETURN_TREEPARSER_SILENT)
	{		
		this->returnVal = (*(currentParser->pfnOpen))(currentParser->fwdid, currentParser->mdFilename, currentTree, currentNode, depth);
	}
}


 
bool VDFTreeFile::SaveVDF(const char *filename, VDFTree *vdfTree)
{
	FILE*   pFile;
	VDFNode *node;
	int     depth;
	int		bufferLen = 0;
	char	tabs[256] = {'\t'};
	char	buffer[2048];

	node   =  vdfTree->rootNode;

	if(filename == NULL || node == NULL)
		return false;

	TabFill tabFill = TabFill(tabs);	
	depth  =  0;
	currentDepth = 0;
	
	pFile = fopen(filename, "w+");
	if(!pFile)
		return false;	
	

	while(node != NULL || currentDepth != 0 )
	{
		*buffer = '\0';
		bufferLen = 0;

		if((int) currentDepth < depth)
		{
			tabFill.SetTabPos(depth - 1);
			bufferLen += sprintf(buffer, "\n%s{", tabs);
		} else if( (int) currentDepth > depth)
		{
			while( (int) currentDepth-- > depth)
			{
				tabFill.SetTabPos(currentDepth);
				bufferLen += sprintf(buffer + bufferLen, "\n%s}", tabs);
			}
		}
		tabFill.SetTabPos(depth);
		currentDepth = depth;

		if(node && node->key && *(node->key))
		{
			bufferLen += sprintf(buffer + bufferLen, "%s%s\"%s\"", (currentDepth == 0 ? "" : "\n"), tabs, node->key);
			if(node->value && *(node->value))
			{
				bufferLen += sprintf(buffer + bufferLen, " \"%s\"", node->value);
			}
			node = VDFTree::GetNextTraverseStep(node, depth);
		}
		fputs(buffer, pFile);
	}
	fclose(pFile);
	return true;
}
