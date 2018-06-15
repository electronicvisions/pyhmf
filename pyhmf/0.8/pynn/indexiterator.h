#pragma once

#include <cstring>
#include <boost/operators.hpp>

// Note boost python need a signed type, since slices can run backwards
class index_iterator : public boost::random_access_iterator_helper<
        index_iterator, ssize_t, ssize_t, ssize_t>
{
public:
	typedef index_iterator self;
	typedef ssize_t type;
	typedef ssize_t & Reference;
	typedef ssize_t Distance;

	explicit index_iterator(type ii = 0) : mIndex(ii)
	{}
	Reference operator*()
	{ return mIndex; }
	self & operator++()
	{ ++mIndex; return *this; }
	self & operator--()
	{ --mIndex; return *this; }
	self & operator+=(Distance n)
	{ mIndex += n; return *this; }
	self & operator-=(Distance n)
	{ mIndex -= n; return *this; }
	bool operator==(const self& other) const
	{ return mIndex == other.mIndex; }
	bool operator<(const self& other) const
	{ return mIndex < other.mIndex; }

	friend Distance operator-(const self& x, const self& y)
	{ return x.mIndex - y.mIndex; }

private:
	type mIndex;
};
