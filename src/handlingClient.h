/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   handlingClient.h
 * Author: mariusz
 *
 * Created on May 26, 2016, 3:11 PM
 */

#ifndef HANDLINGCLIENT_H
#define HANDLINGCLIENT_H

#ifdef __cplusplus
extern "C" {
#endif
    
char* logInChecker(char* buf);
int getClientInstruction(char* buf);
char* int2char(int i);
int writeToLog(int timestamp, char* client_message, char* srv_response);

#ifdef __cplusplus
}
#endif

#endif /* HANDLINGCLIENT_H */

#include "handlingClient.c"