#pragma once

#include <iterator>
#include <boost/shared_ptr.hpp>
#include "py_id.h"
#include "py_population_base.h"

template<typename ParentType>
class CellIterator : public std::iterator<
					 std::input_iterator_tag,
					 PyID,
					 std::ptrdiff_t,
					 PyID*,
					 PyID>
{
public:
	CellIterator(size_t position, const boost::shared_ptr<ParentType>& parent) : mPosition(position), mParent(parent)
	{
	}

	PyID operator*()
	{
		if(!mParent)
		{
			throw std::string("Lost pointer to parent population! Oh, noez!!11");
		}

		return (*mParent)[mPosition];
	}

	CellIterator<ParentType>& operator++()
	{
		mPosition++;
		return *this;
	}

	CellIterator<ParentType> operator++(int)
	{
		CellIterator<ParentType> tmp(*this);
		operator++();
		return tmp;
	}

	bool operator==(const CellIterator<ParentType>& right)
	{
		return (this->mPosition == right.mPosition);
	}

	bool operator!=(const CellIterator<ParentType>& right)
	{
		return !(*this == right);
	}
private:
	size_t mPosition;
	boost::shared_ptr<ParentType> mParent;
};
