#ifndef USER_HOST
#define USER_HOST
#include <stdlib.h>
#include "protocolStructure.h"


/**
 * @brief sendTPtoRouter: Function passes a local transferPackage from user to user's router
 * @param tp: package passed for sending
 * @param um: user model used for package sending
 * @return negative numbers in case of an error, otherwise 0
 */
int sendTPtoRouter(transferPackage* tp, userModel * um);


/**
 * @brief receiveTPfromRouter: Function passes a received transferPackage from user's router to user
 * @param tp: receiving package
 * @param rm: user model used for package reception
 * @return negative numbers in case of an error, otherwise 0
 */
int receiveTPfromRouter(transferPackage * tp, userModel * um);
#endif
