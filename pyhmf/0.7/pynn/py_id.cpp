#include "py_id.h"
#include "py_current.h"
#include "py_population_base.h"

using namespace euter;

PyID::PyID(const size_t v) :
	mId(v), mParent()
{
}

PyID::PyID(const size_t v, const boost::shared_ptr<PyPopulationBase>& parent) :
	mId(v), mParent(parent)
{
}

PyID::~PyID()
{
}

void PyID::inject(const boost::shared_ptr<PyCurrentSource>& current_source)
{
	current_source->inject_into(*this);
}

size_t PyID::id() const
{
	return mId;
}

boost::shared_ptr<PyPopulationBase const> PyID::parent() const
{
	return mParent;
}

boost::shared_ptr<PyPopulationBase> PyID::parent()
{
	return mParent;
}

std::ostream& operator<<(std::ostream& out, const PyID& id)
{
	out << id.id();
	return out;
}
