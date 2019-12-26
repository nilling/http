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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <regex>
#include "tool.h"

#define INVALID_SOCKET -1
#define BUFSIZE 4096

using namespace std;

struct Response final
    {
        enum Status
        {
            STATUS_CONTINUE = 100,
            STATUS_SWITCHINGPROTOCOLS = 101,
            STATUS_PROCESSING = 102,
            STATUS_EARLYHINTS = 103,

            STATUS_OK = 200,
            STATUS_CREATED = 201,
            STATUS_ACCEPTED = 202,
            STATUS_NONAUTHORITATIVEINFORMATION = 203,
            STATUS_NOCONTENT = 204,
            STATUS_RESETCONTENT = 205,
            STATUS_PARTIALCONTENT = 206,
            STATUS_MULTISTATUS = 207,
            STATUS_ALREADYREPORTED = 208,
            STATUS_IMUSED = 226,

            STATUS_MULTIPLECHOICES = 300,
            STATUS_MOVEDPERMANENTLY = 301,
            STATUS_FOUND = 302,
            STATUS_SEEOTHER = 303,
            STATUS_NOTMODIFIED = 304,
            STATUS_USEPROXY = 305,
            STATUS_TEMPORARYREDIRECT = 307,
            STATUS_PERMANENTREDIRECT = 308,

            STATUS_BADREQUEST = 400,
            STATUS_UNAUTHORIZED = 401,
            STATUS_PAYMENTREQUIRED = 402,
            STATUS_FORBIDDEN = 403,
            STATUS_NOTFOUND = 404,
            STATUS_METHODNOTALLOWED = 405,
            STATUS_NOTACCEPTABLE = 406,
            STATUS_PROXYAUTHENTICATIONREQUIRED = 407,
            STATUS_REQUESTTIMEOUT = 408,
            STATUS_CONFLICT = 409,
            STATUS_GONE = 410,
            STATUS_LENGTHREQUIRED = 411,
            STATUS_PRECONDITIONFAILED = 412,
            STATUS_PAYLOADTOOLARGE = 413,
            STATUS_URITOOLONG = 414,
            STATUS_UNSUPPORTEDMEDIATYPE = 415,
            STATUS_RANGENOTSATISFIABLE = 416,
            STATUS_EXPECTATIONFAILED = 417,
            STATUS_IMATEAPOT = 418,
            STATUS_MISDIRECTEDREQUEST = 421,
            STATUS_UNPROCESSABLEENTITY = 422,
            STATUS_LOCKED = 423,
            STATUS_FAILEDDEPENDENCY = 424,
            STATUS_TOOEARLY = 425,
            STATUS_UPGRADEREQUIRED = 426,
            STATUS_PRECONDITIONREQUIRED = 428,
            STATUS_TOOMANYREQUESTS = 429,
            STATUS_REQUESTHEADERFIELDSTOOLARGE = 431,
            STATUS_UNAVAILABLEFORLEGALREASONS = 451,

            STATUS_INTERNALSERVERERROR = 500,
            STATUS_NOTIMPLEMENTED = 501,
            STATUS_BADGATEWAY = 502,
            STATUS_SERVICEUNAVAILABLE = 503,
            STATUS_GATEWAYTIMEOUT = 504,
            STATUS_HTTPVERSIONNOTSUPPORTED = 505,
            STATUS_VARIANTALSONEGOTIATES = 506,
            STATUS_INSUFFICIENTSTORAGE = 507,
            STATUS_LOOPDETECTED = 508,
            STATUS_NOTEXTENDED = 510,
            STATUS_NETWORKAUTHENTICATIONREQUIRED = 511
        };

        int status = 0;
        std::vector<std::string> headers;
        std::vector<char> body;
    };


class HttpRes{
public:
    HttpRes(string url, int port, string method, string data);
    ~HttpRes(){};

    //HttpRes* getInstance();
    void debug(string fmt, ...);
    //int httpGet(string url, int port, string& response);
    //int httpPost(string url, int port, string data, string& response);
    int httpRequest(Response& response);

private:
    int _httpRequest(Response &response);
    string httpHeadCreate();
    Response httpTransmit();

    int getPortfromUrl(string url);
    string getipfromurl(string url);
    string getparamfromurl(string url);
    string gethostfromurl(string url);

    int socketfdcheck(const int socketfd);

    void clear_socket(int socketfd);

    int httpsocketfd;
    string httpheader;
    string url;
    int port;
    string host;
    string ip;
    string method;
    string data;
    string param;
};



#endif