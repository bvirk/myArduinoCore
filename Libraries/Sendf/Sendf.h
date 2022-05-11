#ifndef Sendf_h
#define Sendf_h
#include <WString.h>

// This library is free software; you can redistribute it and/or 
// modify it under the terms of the GNU Lesser General Public 
// License as published by the Free Software Foundation; either 
// version 2.1 of the License, or (at your option) any later version.

#define SENDLIBVERSION "0.1.0"

#define DOUBLE_DEFAULT_PRECISION 2
#define FP(pgmVar) (reinterpret_cast<const __FlashStringHelper *>(pgmVar))

//! Serial 'printf' for NON float and NON double arguments. 
/*!
  A fixed buffer af length 0x40 bytes is used - the formatted stream will maximal be 0x3f bytes long. Care is taken not to overflow, 
  as it is vsnprintf that is used.

  \param fmt is like the format string of printf, except %e and %f must not be used
  \param variadic arguments can not be of type float or double.
  \return length of sendt stream.
*/
int8_t sendf(const char * fmt, ...);
int8_t sendf(const __FlashStringHelper *ffmt, ...);
int8_t sendendl();

//! Serial 'printf' for ONLY float or double arguments. 
/*!
  sendff for sendf-floats.  A fixed buffer af length 0x40 bytes is used - the formatted stream will maximal be 0x3f bytes long. 
  Care is taken not to overflow, as it is vsnprintf that is used.
  Only format specifiers %[.d]e and %[.d]f will be reconized - d, the optional precision specifier, is a single 
  digit in range {0 .. 7} - a 8 or 9 means 7. Absense of precision specifier defaults it to DOUBLE_DEFAULT_PRECISION.
  A midtrange area between not big numbers and not small numbers, will, when %f is used, be sendt in normal decimal notation - 
  outside that range the format is switched to scientific notation. The switching occurs when:
  
  	abs(number) > 9999999*pow(10,-precision) or abs(number) < pow(10,precision-8) as presision is in range {0 .. 7} 
  
  In that way it is adequate to offer dtostre or dtostrf a 0xf bytes buffer.  

  \param fmt is like the format string of printf, except ONLY %e OR %f format specifiers is converted.
  \param arguments can only be of type float or double.
  \return length of sendt stream.
*/
int8_t sendff(const char * fmt, ...);
int8_t sendff(const __FlashStringHelper *ffmt, ...);


//! convert, in scientific notation, float or double to char *
/*!
  This is an overload of stdlib char * dtostre(... using a dedicated buffer and leading space chosen before the scientific
  notation number.
  
  \param d is double (or float) to be converted to char buffer
  \param prec is precision which is limited to 7.
  \return char buffer of converted value
*/
inline char * dtostre(double d, uint8_t prec);


//! convert, in decimal notation, float or double to char *
/*!
  This is an overload of stdlib char * dtostrf(... using a dedicated buffer and a width parameter which is 0. Care must to taken
  not to overflow the buffer. The used buffer has room for 14 characters - if the number of digits inclusive the period  exceeds 14 bytes the memory will be corrupted with UNDEFINED consequences! See doc for function sendff 
  
  \param d is double (or float) to be converted to char buffer
  \param prec is precision which is limited to 7.
  \return char buffer of converted value
*/
inline char * dtostrf(double d, uint8_t prec);

#endif