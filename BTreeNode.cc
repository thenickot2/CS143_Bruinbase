#include "BTreeNode.h"

using namespace std;

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{
	return pf.read(pid,buffer);
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{ 
	return pf.write(pid,buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
	int maxKeyCount=(PageFile::PAGE_SIZE-sizeof(PageId*))/(sizeof(Entry));
	int keyCount=0;
	Entry* entry=(Entry*) buffer;
	for(int count=0;count<maxKeyCount;count++){
		if((entry+count)->key!=0)
			keyCount++;
	}
	return keyCount;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
	int keyCount=getKeyCount();
	int maxKeyCount=(PageFile::PAGE_SIZE-sizeof(PageId*))/(sizeof(Entry));
	if (keyCount == maxKeyCount)
		return 1;	//buffer full
	//get position to insert
	int insertPosition;
	int errorCheck=locate(key,insertPosition);
	if (errorCheck!=0)
		insertPosition=keyCount;
	//Shift any larger entries to the right of the array
	Entry* entryBuffer=(Entry*) buffer; //buffer typecasted
	int amountToShift=keyCount-insertPosition;
	for (int i=amountToShift;i>=0;i--){
		(entryBuffer+insertPosition+i+1)->key=(entryBuffer+insertPosition+i)->key;
		(entryBuffer+insertPosition+i+1)->rid=(entryBuffer+insertPosition+i)->rid;
	}
	//modify entry/insert
	(entryBuffer+insertPosition)->key=key;
	(entryBuffer+insertPosition)->rid=rid;
	return 0;
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{ 
	int eid; //insert position
	if (locate(key,eid))
		return 1;
	int keyCount=getKeyCount();
	int sid=(keyCount+1)/2; //starting position of entries for siblings(even split)
	//do normal insert
	//Shift any larger entries to the right of the array
	Entry* entryBuffer=(Entry*) buffer; //buffer typecasted
	Entry temp; //hold last entry since it might be overwritten
	temp.key=(entryBuffer+keyCount-1)->key;
	temp.rid=(entryBuffer+keyCount-1)->rid;
	int amountToShift=keyCount-eid;
	for (int i=keyCount-1;i>eid;i--){
		(entryBuffer+i)->key=(entryBuffer+i-1)->key;
		(entryBuffer+i)->rid=(entryBuffer+i-1)->rid;
	}
	//modify entry/insert
	(entryBuffer+eid)->key=key;
	(entryBuffer+eid)->rid=rid;
	
	//split
	sibling.insert(temp.key,temp.rid);
	for(int i=sid;i<keyCount;i++){
		sibling.insert((entryBuffer+i)->key,(entryBuffer+i)->rid);
		(entryBuffer+i)->key=0;
	}
	return 0;
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{
	eid=0;
	int keyCount=getKeyCount();
	Entry* entry=(Entry*) buffer;
	
	while (eid<keyCount){
		// Found the key
		if(entry->key>=searchKey)
			return 0;
		eid++;
	}
	
	// Reached the end of page without finding the key
	return 1;
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{
	if (eid < 0 || eid >= getKeyCount())
	return 1;

	Entry* entry = (Entry*) buffer + eid;
	rid = entry->rid;
	key = entry->key;
	return 0;
}

/*
 * Return the pid of the next sibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr()
{
	// Pointer at the end
	PageId* pid = (PageId*) (buffer+PageFile::PAGE_SIZE) - 1;
	return *pid;
}

/*
 * Set the pid of the next sibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
	PageId* sib = (PageId*) (buffer+PageFile::PAGE_SIZE) - 1;
	*sib = pid;
	return 0;
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{
	return pf.read(pid,buffer);
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{
	return pf.write(pid,buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
	int maxKeyCount=(PageFile::PAGE_SIZE-sizeof(PageId*))/(sizeof(Entry));
	int keyCount=0;
	Entry* entry=(Entry*) buffer;
	for(int count=0;count<maxKeyCount;count++){
		if((entry+count)->key!=0)
			keyCount++;
	}
	return keyCount;
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ return 0; }

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{ return 0; }

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{ return 0; }

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{ return 0; }
