
/*----------------------------------
Class for asking the player various
input parameters. It does not print
messages, it is only concerned with
the user input/answer.
----------------------------------*/

#ifndef QUERY_H
#define QUERY_H

#include "ConstTypes.h"

class Engine;

class Query
{
public:
	Query(Engine* engine) : eng(engine) {}
   void waitForKeyPress() const;

   bool yesOrNo() const;

   coord direction() const;

   void waitForEscOrSpace() const;

private:
   int readKeys() const;

   Engine* eng;
};

#endif
