#ifndef XML_HPP
#define XML_HPP

#include <string>
#include "tinyxml2.h"

namespace xml
{

typedef tinyxml2::XMLDocument Doc;
typedef tinyxml2::XMLElement Element;

void load_file(const std::string& path, Doc& to_doc);

Element* first_child(Doc& doc);

Element* first_child(Element* e, const std::string name = "");

bool has_child(Element* e, const std::string name);

Element* next_sibling(Element* e, const std::string name = "");

std::string get_text_str(const Element* const e);

bool get_text_bool(const Element* const e);

int get_text_int(const Element* const e);

std::string get_attribute_str(const Element* const e, const std::string name);

int get_attribute_int(const Element* const e, const std::string name);

bool try_get_attribute_str(const Element* const e,
                           const std::string name,
                           std::string& result);

bool try_get_attribute_int(const Element* const e,
                           const std::string name,
                           int& result);

bool try_get_attribute_bool(const Element* const e,
                            const std::string name,
                            bool& result);

} // xml

#endif // XML_HPP
