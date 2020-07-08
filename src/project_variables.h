#ifndef _PROJECT_VARIABLES_H_
#define _PROJECT_VARIABLES_H_

#include <stdint.h>

/* ************************************************************************** */
/*  Project Information

    This is data that's pulled from project.yaml and other sources
*/
/* [[[cog

cog.outl(f'#define PRODUCT_NAME "{utils.project.name}"')
cog.outl()
cog.outl(f'#define PRODUCT_V_MAJOR {utils.project.sw_version.major}')
cog.outl(f'#define PRODUCT_V_MINOR {utils.project.sw_version.minor}')
cog.outl(f'#define PRODUCT_V_PATCH {utils.project.sw_version.patch}')

]]] */
#define PRODUCT_NAME "AT-600ProII"

#define PRODUCT_V_MAJOR 0
#define PRODUCT_V_MINOR 5
#define PRODUCT_V_PATCH 0
/* [[[end]]] */

/* ************************************************************************** */
/*  Project Variables

    Preprocessor definitions aren't available at runtime, so let's shove them
    into variables.
*/

// product name
extern const char productName[];

// product software version
extern const uint16_t productVerMajor;
extern const uint16_t productVerMinor;
extern const uint16_t productVerPatch;

// compilation information
extern const uint16_t xc8Version;
extern const char compileDate[];
extern const char compileTime[];

#endif // _PROJECT_VARIABLES_H_