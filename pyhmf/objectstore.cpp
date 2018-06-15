#include "pyhmf/objectstore.h"

namespace {
ObjectStore store;
}

void resetStore()
{
	store = ObjectStore();
}

ObjectStore& getStore()
{
	return store;
}
