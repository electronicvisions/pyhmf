#pragma once

#ifdef PYPLUSPLUS
namespace std
{
template<class T>
class unique_ptr;
}
#else
#include <memory>
#endif
