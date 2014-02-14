#ifndef __INC_NAME_TABLE2007NOV_H__
#define __INC_NAME_TABLE2007NOV_H__

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

class QDomNode;

namespace cia3d{
namespace io{

#include <nametable.h>

//accquired from doc/generateDoc.h of sofa
inline std::string xmlencode(const std::string& str)
{
    std::string res;
    for (unsigned int i=0; i<str.length(); ++i)
    {
        switch(str[i])
        {
        case '<': res += "&lt;"; break;
        case '>': res += "&gt;"; break;
        case '&': res += "&amp;"; break;
        case '"': res += "&quot;"; break;
        case '\'': res += "&apos;"; break;
        default:  res += str[i];
        }
    }
    return res;
}

inline bool generateFactoryHTMLDoc(const std::string& filename)
{
	/*
    //sofa::core::ObjectFactory::getInstance()->dump();
//    std::ofstream ofile(filename.c_str());
    std::ofstream ofile(filename);
    ofile << "<html><body>\n";
    sofa::core::ObjectFactory::getInstance()->dumpHTML(ofile);
    ofile << "</body></html>\n";
	*/
    return true;
}


}//end io
}//end cia3d

#endif