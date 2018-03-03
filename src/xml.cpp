#include "xml.hpp"

#include "rl_utils.hpp"

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
// In cases like getting an attribute or a child element, if the given name
// string is empty, we should not call the tinyxml2 function with the c string
// of the empty name string, but rather with a nullptr. Use this function to
// convert std::string's to c strings, rather than std::string::c_str().
static const char* to_c_str(const std::string str)
{
        return str.empty() ? nullptr : str.c_str();
}

// -----------------------------------------------------------------------------
// xml
// -----------------------------------------------------------------------------
namespace xml
{

void load_file(const std::string& path, Doc& to_doc)
{
        TRACE << "Loading xml file at: " << path << std::endl;

        const auto load_result = to_doc.LoadFile(path.c_str());

        if (load_result != tinyxml2::XML_SUCCESS)
        {
                TRACE_ERROR_RELEASE << "Failed to load xml file at: "
                                    << path
                                    << std::endl;

                PANIC;
        }
}

Element* first_child(Doc& doc)
{
        return doc.FirstChildElement();
}

Element* first_child(Element* e, const std::string name)
{
        return e->FirstChildElement(to_c_str(name));
}

bool has_child(Element* e, const std::string name)
{
        return e->FirstChildElement(to_c_str(name)) != nullptr;
}

Element* next_sibling(Element* e, const std::string name)
{
        return e->NextSiblingElement(to_c_str(name));
}

std::string get_text_str(const Element* const e)
{
        std::string str = "";

        const char* c_str = e->GetText();

        if (c_str)
        {
                str = c_str;
        }

        return str;
}

bool get_text_bool(const Element* const e)
{
        bool value = false;

        const auto result = e->QueryBoolText(&value);

        if (result != tinyxml2::XML_SUCCESS)
        {
                TRACE_ERROR_RELEASE
                        << "While parsing boolean value from "
                        << "xml element \""
                        << e->Value()
                        << "\", tinyxml2 reported error code: "
                        << result << std::endl;

                PANIC;
        }

        return value;
}

int get_text_int(const Element* const e)
{
        int value = false;

        const auto result = e->QueryIntText(&value);

        if (result != tinyxml2::XML_SUCCESS)
        {
                TRACE_ERROR_RELEASE
                        << "While parsing integer value from "
                        << "xml element \""
                        << e->Value()
                        << "\", tinyxml2 reported error code: "
                        << result << std::endl;

                PANIC;
        }

        return value;
}

std::string get_attribute_str(const Element* const e, const std::string name)
{
        return e->Attribute(to_c_str(name));
}

int get_attribute_int(const Element* const e, const std::string name)
{
        const std::string str = e->Attribute(to_c_str(name));

        return to_int(str);
}

bool try_get_attribute_int(const Element* const e,
                           const std::string name,
                           int& result)
{
        auto conv_result = e->QueryAttribute(name.c_str(), &result);

        return (conv_result == tinyxml2::XML_SUCCESS);
}

bool try_get_attribute_bool(const Element* const e,
                            const std::string name,
                            bool& result)
{
        auto conv_result = e->QueryAttribute(name.c_str(), &result);

        return (conv_result == tinyxml2::XML_SUCCESS);
}

} // xml
