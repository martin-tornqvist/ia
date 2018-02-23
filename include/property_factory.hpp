#ifndef PROPERTY_FACTORY_H
#define PROPERTY_FACTORY_H

#include "property_data.hpp"

class Prop;

namespace property_factory
{

Prop* make(const PropId id);

} // prop_factory

#endif // PROPERTY_FACTORY_H
