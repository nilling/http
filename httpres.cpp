#include "httpres.h"

HttpRes* HttpRes::getInstance(){
    HttpRes *http = new HttpRes();
    if(http){
        return http;
    }
    return nullptr;
}

void HttpRes::debug(string fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt.c_str(), ap);
    va_end(ap);
}

int HttpRes::httpGet(string url, int port, string& response){
    return httpRequest(url, port, "get", "", response);
}

int HttpRes::httpPost(string url, int port, string data, string& response){
    return httpRequest(url, port, "post", data, response);
}

int HttpRes::httpRequest(string url, int port, string method, string data, string& response){
    if(url == ""){
        debug("url is empty\n");
        return -1;
    }

    if(port == -1){
        port = 80;
    }
    
    string httphead = httpHeadCreate(url, port, method, data);

    if(httphead == ""){
        debug("head create error\n");
        return -1;
    }

    if(httpsocketfd != INVALID_SOCKET){
        string result = httpTransmit(httphead, httpsocketfd);
        if(result != ""){
            response = result;
            return 0;
        }
    }

    httpsocketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(httpsocketfd < 0){
        debug("socket error errorno:%d------>msg:%s\n", errno, strerror(errno));
        return -1;
    }

}