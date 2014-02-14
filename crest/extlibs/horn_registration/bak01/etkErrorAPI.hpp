
/*! \file etkErrorAPI.hpp
    \brief Messages, warnings and errors API.
 */

#ifndef etkErrorAPI_hpp
#define etkErrorAPI_hpp

 /// Diplay a message
 void etkMessage (char* pcText);
 
 /// Warning message (display a message, continue execution)
 void etkWarning (char* pcText);
 
 /// Error message (display a message, stop execution)
 void etkError (char* pcText);
 
 /// Assertions
 void etkAssert (long lCondition, char* pcText);

#endif
