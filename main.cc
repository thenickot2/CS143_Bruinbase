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
	RecordId id;
	id.pid=11; id.sid=12;
	testnode.insert(10,id);
	
	char* buffer = testnode.printBuffer();
	for(int i=0; i<1024;i++)
		cout << buffer[i];
	return 0;
}
