#include "includes.h"
#include "uart.h"
#include "pps.h"

/* ************************************************************************** */

#if LOG_LEVEL_SYSTEM > LOG_SILENT
/* ************************************************************************** */

/*  Use this string to store the result of the conversion. 
    -2,147,483,648 is the longest number possible for int32_t
    
    10 digits, + 1 for the sign, + 1 for the null
*/
#define NUMBER_OF_DIGITS 12
char intString[NUMBER_OF_DIGITS]; 
int status;

/* ************************************************************************** */

// wrap the UART driver so we don't have to repeat the terminator 30 times
void log_tx_string(const char *string)
{
    UART2_tx_string(string, '\0');
}
/* -------------------------------------------------------------------------- */

/*  i_to_a() converts a signed 32 bit integer into a string
    containing the base ten digits representing that number.
    
    The range of an int32_t is from '2,147,483,647' to '-2,147,483,648'.
    
    There's a known bug that interferes with converting the largest negative
    value.  This is caused by the first if block, which catches a negative 
    input, prepends the sign onto the output string, and inverts the input.
    
    The bug is due to the maximum negative value being one greater than the
    maximum positive value.  When the max negative value is inverted to be
    positive, it overflows and fills the output string with garbage.
    
    This bug has been deemed non-critical and not worth fixing, due to the
    dev-only nature of this module. The odds of needing to print negative-two-
    billion-and-whatever seem rather remote.
*/
char* i_to_a(int32_t value)
{
    char tempChar;
    char buffer[NUMBER_OF_DIGITS];
    
    uint8_t bufferIndex = NUMBER_OF_DIGITS;
    uint8_t stringIndex = 0;
    
    // If 'value' is negative, grab the sign and then flip 'value' to positive
    if (value < 0) 
    {
        intString[stringIndex++] = '-';
        value *= -1;
    }
    
    // Loop through dividing 'value' by the base, to get the digit for each place
    do
    {
        tempChar = '0' + (value % 10);
        if ( tempChar > '9')
        {
            tempChar += 'a' - '9' - 1;
        }
        buffer[--bufferIndex] = tempChar;
        value /= 10;
    } while (value != 0);

    /*  At this point, the result in 'buffer' is right-justified, and 
        bufferIndex is at the leftmost digit of the converted result.
        This loop copies buffer into intString, and makes the result left-justified.
    */
    while (bufferIndex < NUMBER_OF_DIGITS) 
    {
        intString[stringIndex++] = buffer[bufferIndex++];
    }

    // Append null terminator
    intString[stringIndex] = '\0';
    
    return intString;
}

/*  Prints a newline to the output.  This is useful when 'building' custom debug
    formats.
    
    In UART-Land, a newline is a Carriage Return('\r') AND a Line Feed('\n').
    
    This can change based on the configuration of the device that captures the
    debug output.
*/
void log_ln(void)
{
    log_tx_string("\r\n");
}

// Print a string.  Must be null-terminated.
void log_str(const char *string)
{
    log_tx_string(string);
}

// Same as log_str(), but also appends a CRLF.
void log_str_ln(const char *string)
{
    log_str(string);

    log_ln();
}

// Converts an integer to a string and prints it
void log_int(int32_t value)
{
    log_str(i_to_a(value));
}

// Common use case of printing "label: <value>".
void log_cat(const char *string, int32_t value)
{ 
    log_str(string);
    log_int(value);
}

// Same as log_cat(), but also appends a CRLF.
void log_cat_ln(const char *string, int32_t value)
{ 
    log_str(string);
    log_int(value);

    log_ln();
}

// Common use case of printing "label: <value>".
void log_catf(const char *string, double value)
{ 
    log_str(string);
    log_str(ftoa(value, &status));
}

// Same as log_catf(), but also appends a CRLF.
void log_catf_ln(const char *string, double value)
{ 
    log_str(string);
    log_str(ftoa(value, &status));

    log_ln();
}

#endif