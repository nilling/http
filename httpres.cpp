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

    int iport = getPortfromUrl(url);
    if(iport < 0){
        port = iport;
    }

    string ip = getipfromurl(url);
    if(ip == ""){
        debug("get ip from url failed'n");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if(inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0){
        debug("inet_pton error, errorno:%d, error msg:%s\n", errno, strerror(errno));
        clear_socket(httpsocketfd);
        return -1;
    }

    int flags = fcntl(httpsocketfd, F_SETFL, 0);
    if(fcntl(httpsocketfd, F_SETFL, flags|O_NONBLOCK) == -1){
        clear_socket(httpsocketfd);
        debug("fcntl error, errorno:%d, error msg:%s", errno, strerror(errno));
        return -1;
    }

    int res = connect(httpsocketfd, (struct sockaddr*)&addr, sizeof(addr));
    if(res == 0){
        string result = httpTransmit(httphead, httpsocketfd);
        if(result == ""){
            clear_socket(httpsocketfd);
            return -1;
        }else{
            response = result;
            return 0;
        }
    }else if(res < 0){
        if(errno != EINPROGRESS){
            return 0;
        }
    }

    res = socketfdcheck(httpsocketfd);
    if(res > 0){
        string result = httpTransmit(httphead, httpsocketfd);
        if(result == ""){
            clear_socket(httpsocketfd);
            return -1;
        }else{
            response = result;
            return 0;
        }
    }else{
        clear_socket(httpsocketfd);
        return -1;
    }

}

string HttpRes::httpHeadCreate(string url, int port, string method, string data){
    string host = gethostfromurl(url);
    string param = getparamfromurl(url);
    
    string head;
    head.append(method);
    head.append(" /");
    head.append(param);
    head.append("HTTP/1.1\r\n");
    head.append("Accept:*/*\r\n");
    head.append("Accept-Language:cn\r\n");
    head.append("User-Agent:Mozilla/4.0\r\n");
    head.append("Host:");
    head.append(host);
    head.append("\r\n");
    head.append("Cache-Control:no-cache\r\n");
    head.append("Connection:Keep-Alive\r\n");
    if(method == "POST"){
        char len[8] = {0};
        unsigned long  ilen = data.size();
        sprintf(len, "%lu", ilen);

        head.append("Content-Type:application/x-www-form-urlencoded\r\n");
        head.append("Content-Length:");
        head.append(len);
        head.append("\r\n\r\n");
        head.append(data);
        
    }
    head.append("\r\n\r\n");
    return head;
}
