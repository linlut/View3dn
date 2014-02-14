/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 beta 2      *
*                (c) 2006-2007 MGH, INRIA, USTL, UJF, CNRS                    *
*                                                                             *
* This library is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This library is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this library; if not, write to the Free Software Foundation,     *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
*                                                                             *
* Authors: J. Allard, P-J. Bensoussan, S. Cotin, C. Duriez, H. Delingette,    *
* F. Faure, S. Fonteneau, L. Heigeas, C. Mendoza, M. Nesme, P. Neumann,       *
* and F. Poyer                                                                *
******************************************************************************/
#ifndef _CIA3D_XML_OBJECTFACTORY_H
#define _CIA3D_XML_OBJECTFACTORY_H

#include <xml/ObjectElement.h>

namespace cia3d
{
namespace xml
{


typedef sofa::simulation::tree::xml::ObjectElement::Factory ObjectFactory;
typedef sofa::simulation::tree::xml::Element<core::objectmodel::BaseObject> ObjectDescription;


} // namespace xml
} // namespace sofa

#endif
