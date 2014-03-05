/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "Bruinbase.h"
#include "SqlEngine.h"
//#include <iostream>
//#include "BTreeNode.h"
//#include "RecordFile.h"
using namespace std;

/*
int main()
{
  // run the SQL engine taking user commands from standard input (console).
  SqlEngine::run(stdin);

  return 0;
}

/*
#include "BTreeIndex.h"
#include <iostream>
using namespace std;
int main()
{
	/*
	BTLeafNode testnode;
	testnode.initBuffer();
	cout << testnode.getKeyCount();
	RecordId id;
	id.pid=11; id.sid=12;
	for(int i = 1; i < 86; i++)
		testnode.insert(i,id);
	BTLeafNode sibling;
	sibling.initBuffer();
	int sibkey;
	testnode.insertAndSplit(100,id,sibling,sibkey);
	
	sibling.printBuffer();
	//cout << sibkey;
	return 0;
	
	BTreeIndex bindex;
	bindex.open('test','w');
	cout << bindex.rootPid << endl << bindex.treeHeight;
}*/

#include <iostream>
#include "BTreeIndex.h"
#include "RecordFile.h"
#include "BTreeNode.h"
using namespace std;


int main()
{
  BTreeIndex index;
  IndexCursor cursor;
  RecordFile rf;

  int key;
  string value;
  RecordId rid;
  int count;

  if(rf.open("xsmall.tbl",'r'))
    cout << "Could not open record file" << endl;

  if(index.open("xsmall.idx", 'w'))
  {
    cout << "Index failed to open" << endl;
  }

  if(index.locate(0,cursor))
    cout << "Could not locate cursor" << endl;

  count = 0;

  BTLeafNode ln;
  PageFile pf;
  ln.read(cursor.pid,pf);
  ln.printBuffer();
  
  while (!index.readForward(cursor, key, rid))
  {
    cout << "Rid: (" << rid.pid << "," << rid.sid << ") ";
    rf.read(rid, key, value);
    cout << "Key: " << key << " Value: " << value << endl;
    count++;
  }

  cout << count << endl;
}
