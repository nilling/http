//
//httpres.h
//
//author:nil
//


#ifndef httpres
#define httpres

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <stdarg.h>

#define INVALID_SOCKET -1

using namespace std;

class HttpRes{
public:
    HttpRes():httpsocketfd(INVALID_SOCKET){};
    ~HttpRes(){};

    HttpRes* getInstance();
    void debug(string fmt, ...);
    int httpGet(string url, int port, string& response);
    int httpPost(string url, int port, string data, string& response);

private:
    int httpRequest(string url, int port, string method, string data, string& reponse);
    string httpHeadCreate(string url, int port, string method, string data);
    string httpTransmit(string httpHead, int sockfd);

    int getPortfromUrl(string url);
    string getipfromurl(string url);
    string getparamfromurl(string url);
    string gethostfromurl(string url);

    int sokcetfdcheck(const int socketfd);

    int httpsocketfd;
};

#endif