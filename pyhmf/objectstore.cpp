#include "pyhmf/objectstore.h"

using namespace euter;

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
