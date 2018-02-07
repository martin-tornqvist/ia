#ifndef PROPERTY_FACTORY_H
#define PROPERTY_FACTORY_H

#include "property_data.hpp"
#include "property.hpp"

class Prop;

namespace property_factory
{

Prop* mk(const PropId id,
         PropTurns turns_init,
         const int nr_turns = -1);

} // prop_factory

#endif // PROPERTY_FACTORY_H
