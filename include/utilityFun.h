#ifndef UTILITY_FUN
#define UTILITY_FUN
#include "protocolStructure.h"


/**
 * @brief convertTPackageToArray: Function serializes a transferPackage into an unsigned char array
 * @param tp: target transferPackage
 * @param buffer: target unsigned char array
 */
void convertTPackageToArray(transferPackage * tp, unsigned char buffer[CONVBUFFSIZETP]);


/**
 * @brief convertArrayToTPackage: Function de-serializes an unsigned char array into a transferPackage
 * @param buffer: target unsigned char array
 * @param tp: target transferPackage
 */
void convertArrayToTPackage(unsigned char buffer[CONVBUFFSIZETP], transferPackage * tp);

#endif
