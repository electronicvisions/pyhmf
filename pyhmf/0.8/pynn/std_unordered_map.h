#pragma once

#ifdef PYPLUSPLUS
namespace std
{
template<class Key, class T>
class unordered_map;
}
#else
#include <unordered_map>
#endif
