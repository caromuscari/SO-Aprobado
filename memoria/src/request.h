/*
 * request.h
 *
 *  Created on: 7 jul. 2019
 *      Author: utnso
 */

#ifndef SRC_REQUEST_H_
#define SRC_REQUEST_H_


int mandarCreate(st_create * create);
int mandarDrop(st_drop * drop);
char * mandarDescribe(st_describe * describe, int codigo);


#endif /* SRC_REQUEST_H_ */
