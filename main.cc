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
#include <iostream>
#include "BTreeNode.h"
#include "RecordFile.h"
using namespace std;
/*
int main()
{
  // run the SQL engine taking user commands from standard input (console).
  SqlEngine::run(stdin);

  return 0;
}
*/
int main()
{
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
}
