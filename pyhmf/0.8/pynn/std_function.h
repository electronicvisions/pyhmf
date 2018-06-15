#pragma once

#ifdef PYPLUSPLUS
namespace std
{
template< typename T>
class function;
}
#else
#include <functional>
#endif
