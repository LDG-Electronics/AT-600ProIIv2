#ifndef _PROJECT_VARIABLES_H_
#define _PROJECT_VARIABLES_H_

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

#endif // _PROJECT_VARIABLES_H_