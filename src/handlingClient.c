/**
 * \file handlingClient.c
 * \brief Główny plik zawierający metody obsługujące żądania ze 
 * strony klienta. 
 */

#include <json-c/linkhash.h>
#include "json/json.h"
#include "handlingFiles.h"
#include "md5Converter.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//nalezy pamietac zeby zmieniac przy dodawaniu/usuwaniu komend do tablicy
int tab_len = 11;
//!< Brief description after the member
char* instructions[]={  "1view_results", //0
                        "0get_exams", //1
                        "2add_group", //2
                        "2assign_to_group", //3
                        "1check_answears", //4
                        "1add_exam", //5
                        "3log_in", //6
                        "0add_answear", //7
                        "0get_answear", //8
                        "3log_out", //9
                        "1assign_exam_to_group" //10
}; //**!< Detailed description after the member 

/**
 * Metoda, której głównym zadaniem jest obsługa żądań klienta. Każde z żądań 
 * jest traktowana jako oddzielna instrukacja. 
 * @param buf
 * @return 
 */
int getClientInstruction(char* buf){
    int i;
    
    //parsowanie komunikatu
    struct json_object *jarray = json_tokener_parse(buf); 

    //wyluskanie stringa z trescia komunikatu
    const char *instr = (char*)(json_object_get_object(jarray)->head)->k;
    
    for(i=0; i<tab_len; i++){
        if( strcmp(instr, &instructions[i][1]) == 0 )
            return i;
    }
    
    return -1; 
}

/*!< To jest krótki opis zmiennej */
/**
 * Metoda obsługująca logowanie - cały proces uwierzytelniający użytkownika 
 * w systemie. Metoda przyjmuje informacje od klienta o loginie i haśle (MD5) 
 * w postaci ciągu znaków typu json i sprawdza zgodność podanych danych z . 
 * plikie Users.json, w którym znajdują się dane o użytkownikach systemu. 
 * @param buf - zawiera dane w formacie json do logawania podane przez użytkownika
 * @return 
 */
char* logInChecker(char* buf){
    json_object *obj, *login, *haslo;
    const char *user, *pass;
    
    //parsowanie komunikatu klienta
    struct json_object *jarray = json_tokener_parse(buf);
    
    //wydobywanie tablicy
    obj = json_object_object_get(jarray, "log_in");
    
    //wydobywanie obiektów login i haslo
    login = json_object_object_get(obj, "login");
    haslo = json_object_object_get(obj, "haslo");
    
    //zapisywanie loginu i hasła jako string
    user = json_object_get_string(login);
    pass = json_object_get_string(haslo);
    
    int rola = checkUserCredentials(user, pass);
    int tim = time(NULL);
    
    if ( rola >= 0 ){
        char* md5 = str2md5( int2char(tim) );
        //rola jest dolaczana do md5 po jej utworzeniu
        char* id_sesji = strcat( md5 , int2char(rola) );
        char* srv_response;
        
        //strdup: pozbycie sie const
        addSession(id_sesji, strdup(user), int2char(rola), tim);
        
        //generowanie odpowiedzi do klienta
        json_object *message = json_object_new_object();
        
        //dodanie do obiektu JSON klucza "message" i wartosci będącej id_sesji
        addJsonObject(message, "logged", id_sesji);

        srv_response = strdup( json_object_get_string(message) );
        
        return srv_response;
    }
    else{
        char* message = "{ \"error\": \"Podano bledne dane.\" }";
        writeToLog(tim, buf, message);
        return message;
    }
}

/**
 * Metoda zamieniająca liczbę typu int na ciąg znaków typu char.
 * Wykorzystywana przy tworzeniu hash'y. 
 * @param i - liczba do zamiany 
 * @return 
 */
char* int2char(int i){
    char* str = malloc(16);
    snprintf(str, 16, "%d", i);
        /* @param str - cięg znaków typu char */
    return str; 
}

/**
 * Metoda służąca do logowania informacji o działaniach klienta na serwerze. 
 * Każde z działanie klienta będzie logowane w pliku log.txt w katalogu resources. 
 * W pliku logowane będą: \n 
 * - czas żądania \n 
 * - informację o rodzaju żądania do serwera \n 
 * - odpowiedź do klienta jakiej udzielił serwer. 
 * @param timestamp - zwraca czas żądania w sekundach
 * @param *client_message - informacja o żądaniu od klienta
 * @param *serv_response - jaką informację wysłał klientowi serwer

 */
int writeToLog(int timestamp, char* client_message, char* srv_response) {
    FILE *log; ///< FILE 
    char* file_name = "../resources/log.txt";    
    //otwieranie pliku do dodawania
    log = fopen(file_name, "a");
    fprintf(log, "%d; %s; %s\n", timestamp, client_message, srv_response);
    
    return fclose(log);
}

/**
 * 
 * @param buf
 * @return 
 */
char* loggingOut(char* buf){
    json_object *session;
    char* session_id, *srv_response;
    
    //parsowanie komunikatu klienta
    struct json_object *obj = json_tokener_parse(buf);
    
    //wydobywanie tablicy
    session = json_object_object_get(obj, "log_out");
    
    //wydobywanie id sesji
    session_id = strdup(json_object_get_string(session));
    
    srv_response = removeSession(session_id);
    
    return srv_response;
}

/**
 * 
 * @param buf
 * @return 
 */
char* addingGroup(char* buf){
    json_object *obj, *nazwaGrupy, *sessionID;
    const char *nazwa, *id;
    
    //parsowanie komunikatu klienta
    struct json_object *jarray = json_tokener_parse(buf);
    
    //wydobywanie tablicy
    obj = json_object_object_get(jarray, "add_group");
    
    //wydobywanie obiektów login i haslo
    nazwaGrupy = json_object_object_get(obj, "name");
    sessionID = json_object_object_get(obj, "session_id");
    
    //zapisywanie loginu i hasła jako string
    nazwa = json_object_get_string(nazwaGrupy);
    id = json_object_get_string(sessionID);
    
    //sprawdzenie czy uzytkownik ma dostep do uslugi
    if ( checkPermissions(strdup(id), "add_group") != 1){
        char* message = "{ \"error\": \"Brak uprawnien do uslugi!\" }";
        writeToLog(time(NULL), buf, message);
        return message;
    }
    
    //sprawdzanie czy grupa zostala dodana prawidlowo
    if ( addGroup( strdup(nazwa) ) == 1){
        return "{ \"message\": \"Dodano grupe!\" }";
    }
    else{
        char* message = "{ \"error\": \"Wystapil blad przy dodawaniu grupy!\" }";
        writeToLog(time(NULL), buf, message);
        return message;
    }
    
}

/**
 * Metoda do sprawdzania czy klient o przeslanym id sesji ma dostep do żądanej
 * uslugi.
 * 
 * @param session_id
 * @param req
 * @return 
 */
int checkPermissions(char* session_id, char* req){
    int i, role;
    
    //sprawdzenie czy id sesji ma poprawna dlugosc
    if( strlen(session_id) != 32 )
        return -1;
    
    for(i=0; i<tab_len; i++){
        if( strcmp(req, &instructions[i][1] ) == 0 ){
            role = instructions[i][0];
            break;
        }
    }
    
    if (session_id[32] == role)
        return 1;
    else
        return -1;
    
}