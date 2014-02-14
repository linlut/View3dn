#define View3D "View3DN"

/*!
\page page_install Installation
 
First go to the download page to get the latest distribution, if you did not have it already.
This section is divided into the following sections: 
- \ref sec_compwindows 
- \ref sec_compunix 
- \ref sec_insbinunix

\section sec_compwindows Compiling from source on Windows
Currently, I have only compiled View3D for Windows using Microsoft's Visual C++ (version 6.0). For other compilers you may need to edit the ./view3dn.pro a bit. Let me know what you had to change if you got View3D working with another compiler. If you have Visual Studio you can also use the .dsp file found in the root directory. Note that this file is created using TrollTech's qmake. If you have Visual C++ 6.0, and the source distribution, you can easily build View3D using the project files in the root directory. 

Here is what is required: 

- Microsoft Visual C++ (I only tested with version 6.0). Use the vcvars32.bat batch file to set the environment variables (if you did not select to do this automatically during installation). Borland C++ or MINGW (see http://www.mingw.org/) are also supported.

- Qt. Qt is a comprehensive development framework developed by Trolltech Company (http://www.trolltech.com). It includes an extensive array of features, capabilities and tools that enable development of high-performance, cross-platform rich-client and server-side applications. Qt sets the standard for high-performance, cross-platform application development. It includes a C++ class library and tools for cross-platform development and internationalization.

- libQGLViewer. libQGLViewer is a C++ library based on Qt that eases the creation of OpenGL 3D viewers. It can be downloaded from ( ). It provides some of the typical 3D viewer functionalities, such as the possibility to move the camera using the mouse, which lacks in most of the other APIs. Other features include mouse manipulated frames, interpolated keyFrames, object selection, stereo display, screenshot saving and much more. It can be used by OpenGL beginners as well as to create complex applications, being fully customizable and easy to extend. Since it is based on the Qt toolkit, it compiles on any architecture (Unix-Linux, Mac, Windows). Full reference documentation and many examples are provided. libQGLViewer does not display 3D scenes in various formats: It is more likely to be the starting point for the coding of such a viewer.

Compilation is now done by performing the following steps:

\subsection subsec_libqglviewer Compiling libQGLViewer

Open a dos box. Make sure the compliler tools (e.g., nmake and ncc), are accessible from the command-line (add them to the PATH environment variable if needed).
Go to the doxygen root dir and type:

    make.bat msvc

This should build the executables doxygen.exe and doxytag.exe using Microsoft's Visual C++ compiler (The compiler should not produce any serious warnings or errors).

You can use also the bcc argument to build executables using the Borland C++ compiler, or mingw argument to compile using GNU gcc.


To build the examples, go to the examples subdirectory and type:

    nmake


To generate the doxygen documentation, go to the doc subdirectory and type:

    nmake

The generated HTML docs are located in the ..\html subdirectory.

The sources for LaTeX documentation are located in the ..\latex subdirectory. From those sources, the DVI, PostScript, and PDF documentation can be generated



*  \section sec_compunix Compiling from source on Unix
If you downloaded the source distribution, you need at least the following to build the executable: 
The GNU tools flex, bison and make 
In order to generate a Makefile for your platform, you need perl . 
To take full advantage of doxygen's features the following additional tools should be installed.

*  \section sec_insbinunix Installing the binaries on Unix 
 
   Known compilation problems for Unix 
    Compiling from source on Windows 
    Installing the binaries on Windows 
    Tools used to develop doxygen 



Troll Tech's GUI toolkit Qt version 3.2 or higher. This is needed to build the GUI front-end doxywizard. 
A  distribution: for instance teTeX 1.0 . This is needed for generating LaTeX, Postscript, and PDF output. 
the Graph visualization toolkit version 1.8.10 or higher . Needed for the include dependency graphs, the graphical inheritance graphs, and the collaboration graphs. If you compile graphviz yourself, make sure you do include freetype support (which requires the freetype library and header files), otherwise the graphs will not render proper text labels. 
The ghostscript interpreter. To be found at www.ghostscript.com. 
*/
 

/*!
\page page_start Start
 
  
\section sec_sss Installation
First go to the download page to get the latest distribution, if you did not have it already.
This section is divided into the following sections:  
 \image html popolan.jpg
 \image latex popolan.jpg "My application" width=10cm
 
 */



/*! 
\mainpage Introduction
  
View3D is a post-processing tool for datasets in Tecplot's .PLT format. 
These data are usually generated from finite-element-based simulation packages, such as
CIA3D. Besides PLT format, it can also recgonize other graphics file formats, 
such as PLY, NFF, STL, etc.

View3D can help you in several ways: 
    <ol>      
        <li>Render geometric objects using different styles, such as hiddenline mode and line mode.
        <li>Display vertex attributes using textures.
        <li>Make animation with a serrial of datasets.     
    </ol>

View3D is developed under Microsoft Windows platform, but is set-up to be highly portable. 
As a result, it runs on most Linux platforms as well. It has been tested on Mac OS/X and Ubuntu Linux. 
Furthermore, executables for Windows are available.


This manual is divided in the following sections:
- \subpage page_install 
- \subpage page_start
*/
