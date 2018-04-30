#ifndef _LOG_LEVELS_H_
#define _LOG_LEVELS_H_

/* ************************************************************************** */

// Logging levels
#define LOG_SILENT      0
#define LOG_ERROR       1
#define LOG_EVENTS      2
#define LOG_LABELS      3
#define LOG_INFO        4
#define LOG_DETAILS     5
#define LOG_ALL         10

// Entire log system enable/disable
#define LOG_LEVEL_SYSTEM LOG_SILENT

// Subsystems
#define LOG_LEVEL_DISPLAY LOG_SILENT
#define LOG_LEVEL_EEPROM LOG_SILENT
#define LOG_LEVEL_FLASH LOG_SILENT
#define LOG_LEVEL_FLAGS LOG_SILENT
#define LOG_LEVEL_MEMORY LOG_SILENT
#define LOG_LEVEL_MENUS LOG_SILENT
#define LOG_LEVEL_RF_SENSOR LOG_SILENT
#define LOG_LEVEL_RELAYS LOG_SILENT
#define LOG_LEVEL_STARTUP LOG_SILENT
#define LOG_LEVEL_TUNING LOG_SILENT

/* ************************************************************************** */

#if LOG_LEVEL_SYSTEM > LOG_SILENT
    #define print_ln();                 log_ln();
    #define print_int(val);             log_int(val);

    #define print_cat(val, val2);       log_cat(val, val2);
    #define print_cat_ln(val, val2);    log_cat_ln(val, val2);
    #define print_catf(val, val2);      log_catf(val, val2);
    #define print_catf_ln(val, val2);   log_catf_ln(val, val2);
#else
    #define print_ln();
    #define print_int(val);

    #define print_cat(val, val2);
    #define print_cat_ln(val, val2);
    #define print_catf(val, val2);
    #define print_catf_ln(val, val2);
#endif


#endif // <- include guard