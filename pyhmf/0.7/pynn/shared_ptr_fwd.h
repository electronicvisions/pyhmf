#pragma once

#include <boost/shared_ptr.hpp>

namespace euter {
class Projection;
}
typedef boost::shared_ptr<euter::Projection> ProjectionPtr;

class PyConnector;
typedef boost::shared_ptr<PyConnector> PyConnectorPtr;

class PyProjection;
typedef boost::shared_ptr<PyProjection> PyProjectionPtr;
