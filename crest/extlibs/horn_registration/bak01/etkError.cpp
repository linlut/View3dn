/*! \file Win32\etkError.cpp
    \brief Messages, warnings and errors for Win32 platform.
 */

#include <stdio.h>
#include <stdlib.h>
#include "etkErrorAPI.hpp"

//#ifdef WIN32
//#include <windows.h>

// ---------------------------------------------------------------------------------
// Diplay a message
// ---------------------------------------------------------------------------------
/*
void etkMessage (char* pcText)
{
  (void) MessageBox (NULL, pcText, "Message", MB_OK);
}

// ---------------------------------------------------------------------------------
// Warning message (display a message, continue execution)
// ---------------------------------------------------------------------------------

void etkWarning (char* pcText)
{
  (void) MessageBox (NULL, pcText, "Warning", MB_OK);
}

// ---------------------------------------------------------------------------------
// Error message (display a message, stop execution)
// ---------------------------------------------------------------------------------

void etkError (char* pcText)
{
  (void) MessageBox (NULL, pcText, "Error", MB_OK);
  exit (-1);
}

// ---------------------------------------------------------------------------------
// Assertions
// ---------------------------------------------------------------------------------

void etkAssert (long lCondition, char* pcText)
{
  if (!lCondition)
  {
    (void) MessageBox (NULL, pcText, "Assertion failed", MB_OK);
    exit (-1);
  }
}

#else
*/
// ---------------------------------------------------------------------------------
// Diplay a message
// ---------------------------------------------------------------------------------

void etkMessage (char* pcText)
{
  fprintf (stderr, "Message: %s", pcText);
}

// ---------------------------------------------------------------------------------
// Warning message (display a message, continue execution)
// ---------------------------------------------------------------------------------

void etkWarning (char* pcText)
{
  fprintf (stderr, "Warning: %s", pcText);
}

// ---------------------------------------------------------------------------------
// Error message (display a message, stop execution)
// ---------------------------------------------------------------------------------

void etkError (char* pcText)
{
  fprintf (stderr, "Error: %s", pcText);
  exit (-1);
}

// ---------------------------------------------------------------------------------
// Assertions
// ---------------------------------------------------------------------------------

void etkAssert (long lCondition, char* pcText)
{
  if (!lCondition)
  {
    fprintf (stderr, "Assertion failed");
    exit (-1);
  }
}
//#endif