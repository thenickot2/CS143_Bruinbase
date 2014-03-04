/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "BTreeIndex.h"
#include "BTreeNode.h"

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
	if(pf.open(indexname, mode))
		return 1;
	char* buffer;
  // initialize
  if (pf.endPid() == 0)
  {
    rootPid = -1;
    treeHeight = 0;
    return 0;
  }
  //already initialized
  if (pf.read(0, buffer))
    return 1;
  rootPid = *((PageId*) buffer);
  treeHeight = *((int*) (buffer+sizeof(PageId)));
  return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
    //save to file
    char* buffer;
    *((PageId*) buffer) = rootPid;
    *((int*) (buffer+sizeof(PageId))) = treeHeight;
    pf.write(0,buffer);

    return pf.close();
}

RC BTreeIndex::update_root(bool push, int key, RecordId& rid, PageId pid){
	if(push==false){
		BTLeafNode leaf;
		leaf.insert(key, rid);
		rootPid = pf.endPid();
		treeHeight++;
		leaf.write(rootPid, pf);
	}else{
		BTNonLeafNode newRoot;
		newRoot.initializeRoot(rootPid, key, pid);
		rootPid = pf.endPid();
		newRoot.write(rootPid, pf);
	}
	return 0;
}

RC BTreeIndex::insert_leaf(int key, const RecordId& rid, PageId pid, int& overflowKey, PageId& overflowPid){
	BTLeafNode leafNode;
    leafNode.read(pid, pf);
    if (leafNode.insert(key, rid)) //overflow
    {
      BTLeafNode leafNode2;
      if (leafNode.insertAndSplit(key, rid, leafNode2, overflowKey))
        return 1;

      overflowPid = pf.endPid();
      leafNode2.setNextNodePtr(leafNode.getNextNodePtr());
      leafNode.setNextNodePtr(overflowPid);
      if (leafNode2.write(ofPid, pf))
        return 1;
    }
    if (leafNode.write(pid, pf))
      return 1;
}

RC BTreeIndex::insert_recursive(int key, const RecordId& rid, PageId pid, int level, int& overflowKey, PageId& overflowPid){
  overflowKey = 0;

  if (level == treeHeight){
    insert_leaf(key, rid, pid, overflowKey, overflowPid)
  }else{
    BTNonLeafNode nonLeaf;
    int eid;
    PageId child;

    nonLeaf.read(pid, pf);
    nonLeaf.locate(key, eid);
    nonLeaf.readEntry(eid, child);
    insert_recursive(key, rid, child, level+1, overflowKey, overflowPid); //WE MUST GO DEEPER
	//BEGINNING TO SURFACE, must fix the overflow at this level
    if (overflowKey > 0) //overflow not fixed
    {
      if (nonLeaf.insert(overflowKey, overflowPid)) //overflow
      {
        int midKey;
        BTNonLeafNode sibling;

        if (nonLeaf.insertAndSplit(overflowKey, overflowPid, sibling, midKey))
          return 1;
        overflowKey = midKey;
        overflowPid = pf.endPid();
        if (sibling.write(overflowPid, pf))
          return 1;
      }
      else
      {
        overflowKey = 0;
      }
      nonLeaf.write(pid, pf);
    }
  }
  return 0;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	if (treeHeight == 0) //new tree?
	{
		update_root(false,key,rid,NULL);
	}
	
	int overflowKey=0;
	PageId overflowPid;
	
	if (insert_recursive(key, rid, rootPid, 1, overflowKey, overflowPid))
		return 1;

	if (overflowKey > 0){ //create new root???
		update_root(true,overflowKey,NULL,overflowPid); 
		treeHeight++;
	}
	return 0;
}

/*
 * Find the leaf-node index entry whose key value is larger than or 
 * equal to searchKey, and output the location of the entry in IndexCursor.
 * IndexCursor is a "pointer" to a B+tree leaf-node entry consisting of
 * the PageId of the node and the SlotID of the index entry.
 * Note that, for range queries, we need to scan the B+tree leaf nodes.
 * For example, if the query is "key > 1000", we should scan the leaf
 * nodes starting with the key value 1000. For this reason,
 * it is better to return the location of the leaf node entry 
 * for a given searchKey, instead of returning the RecordId
 * associated with the searchKey directly.
 * Once the location of the index entry is identified and returned 
 * from this function, you should call readForward() to retrieve the
 * actual (key, rid) pair from the index.
 * @param key[IN] the key to find.
 * @param cursor[OUT] the cursor pointing to the first index entry
 *                    with the key value.
 * @return error code. 0 if no error.
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
    PageId pid = rootPid; // Start at top of the tree
	BTNonLeafNode nontemp;
	int eid;
	
	// Traverse the tree until you reach the leaf height
	for(int i = 0; i < treeHeight-1; i++) {// -1 because bottom is leaf
		if(nontemp.read(pid, pf)) // Read in the appropriate page
			return 1;
		nontemp.locateChildPtr(searchKey, pid); // Update pid
	}
	
	// At this point, pid is pointing to the correct leaf page
	BTLeafNode leaftemp;
	if(leaftemp.read(pid, pf))
		return 1;
	
	leaftemp.locate(searchKey, cursor.eid); // Update eid
	cursor.pid = pid; // Update pid
	return 0;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
	if (cursor.pid <= 0 || cursor.pid >= pf.endPid()) // Check for valid pid
		return 1;
		
	BTLeafNode temp;	// Initialize temporary leaf node
	temp.read(cursor.pid, pf);	// Read the page into the temporary buffer
	temp.readEntry(cursor.eid,key,rid); // Read the entry
	
	// Increment cursor
	cursor.eid++;
	// Check if eid goes past current node's contents
	if(cursor.eid > temp.getKeyCount()) {
		cursor.eid = 0; // Set to beginning of next file
		cursor.pid = temp.getNextNodePtr(); // Set pointer to the next page
	}
	return 0;
}
