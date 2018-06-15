#pragma once

#include <ostream>
#include <boost/shared_ptr.hpp>

class PyPopulationBase;
class PyCurrentSource;

class PyID
{
public:
	PyID(size_t id);
	PyID(size_t id, const boost::shared_ptr<PyPopulationBase>& parent);
	~PyID();

	void inject(const boost::shared_ptr<PyCurrentSource>& current_source);

	size_t id() const;
	boost::shared_ptr<PyPopulationBase const> parent() const;
	boost::shared_ptr<PyPopulationBase>       parent();

private:
	size_t mId;
	boost::shared_ptr<PyPopulationBase> mParent;
};

std::ostream& operator<<(std::ostream& out, const PyID& id);
