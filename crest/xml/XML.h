//FILE: xml.h

#ifndef _CIA3D_TREE_XML_XML_H
#define _CIA3D_TREE_XML_XML_H

#include <xml/Element.h>

namespace cia3d
{
namespace xml
{


BaseElement* load(const char *filename);

bool save(const char *filename, BaseElement* root);


} // namespace xml
} // namespace sofa

#endif
