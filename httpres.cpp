#include "httpres.h"

/*
HttpRes* HttpRes::getInstance(){
    HttpRes *http = new HttpRes();
    if(http){
        return http;
    }
    return nullptr;
}
*/

void HttpRes::debug(string fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt.c_str(), ap);
    va_end(ap);
}

void HttpRes::clear_socket(int sockfd){
    close(sockfd);
    httpsocketfd = INVALID_SOCKET;
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
        Response result = httpTransmit(httphead, httpsocketfd);
        /*if(result != ""){
            response = result;
            return 0;
        }*/
    }

    httpsocketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(httpsocketfd < 0){
        debug("socket error errorno:%d------>msg:%s\n", errno, strerror(errno));
        return -1;
    }

    int iport = getPortfromUrl(url);
    if(iport > 0){
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
        Response result = httpTransmit(httphead, httpsocketfd);
        /*if(result == ""){
            clear_socket(httpsocketfd);
            return -1;
        }else{
            response = result;
            return 0;
        }*/
    }else if(res < 0){
        if(errno != EINPROGRESS){
            return 0;
        }
    }

    res = socketfdcheck(httpsocketfd);
    if(res > 0){
        Response result = httpTransmit(httphead, httpsocketfd);
        /*if(result == ""){
            clear_socket(httpsocketfd);
            return -1;
        }else{
            response = result;
            return 0;
        }*/
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
    head.append(" HTTP/1.1\r\n");
    head.append("Accept:*/*\r\n");
    head.append("Accept-Language:cn\r\n");
    head.append("User-Agent:Mozilla/4.0\r\n");
    head.append("Host:");
    head.append(host);
    head.append("\r\n");
    head.append("Cache-Control:no-cache\r\n");
    head.append("Connection:Keep-Alive\r\n");
    if(method == "post"){
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

string HttpRes::gethostfromurl(string url){
    int start, end;
    if(url.find("http://") != -1){
        start = 7;
    }else if(url.find("https://") != -1){
        start = 8;
    }else{
        debug("url not correct\n");
        return nullptr;
    }

    string tmp_host = url.substr(start);

    end = tmp_host.find_first_of("/");
    if(end == -1){
        debug("url not coorrect\n");
        return nullptr;
    }
    string host = tmp_host.substr(0, end);
    return host;
}

string HttpRes::getipfromurl(string url){
    string host = gethostfromurl(url);
    string ip;
    int end = host.find(":");
    if(end != -1){
        return host.substr(0, end);
    }

    ip = host.substr(0, end);
    struct hostent *ht = gethostbyname(ip.c_str());
    if(ht == nullptr){
            debug("gethostbyname error\n");
            return nullptr;
    }
    struct in_addr** addr_list = (struct in_addr **)ht->h_addr_list;
    for(int i = 0; addr_list[i] != nullptr; i++){
        return inet_ntoa(*addr_list[i]);
    }

    return nullptr;
}

int HttpRes::getPortfromUrl(string url){
    string host = gethostfromurl(url);
    int start = host.find(":");
    if(start == -1){
        return -1;
    }
    
    int port = atoi(host.substr(start+1).c_str());

    return port;
}

string HttpRes::getparamfromurl(string url){
    int start;
    if(url.find("http://") != -1){
        start = 7;
    }else if(url.find("https://") != -1){
        start = 8;
    }else{
        debug("url not correct\n");
        return nullptr;
    }

    string host = url.substr(start);

    int pos = host.find("/");
    if(pos == host.size()-1){
        return "";
    }

    return host.substr(pos+1);
}

int HttpRes::socketfdcheck(const int socketfd){
    struct timeval timeout;
    fd_set rset, wset;
    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_SET(socketfd, &rset);
    FD_SET(socketfd, &wset);
    timeout.tv_sec = 600;
    timeout.tv_usec = 0;

    int ret = select(socketfd+1, &rset, &wset, NULL, &timeout);
    if(ret > 0){
        int r = FD_ISSET(socketfd, &rset);
        int w = FD_ISSET(socketfd, &wset);
        if(!r && w){
            char error[4] = "";
            socklen_t len = sizeof(error);
            int res = getsockopt(socketfd, SOL_SOCKET, SO_ERROR, error, &len);
            if(res == 0){
                if(!strcmp(error, "")){
                    return ret;
                }else{
                    debug("%s %s [%d] getsocketopt error code:%d, message:%s\n", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
                }
            }else{
                 debug("%s %s [%d] getsocketopt error code:%d, message:%s\n", __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
            }
        }else{
             debug("%s %s [%d] readfd:%d, writefd:%d\n", __FILE__, __FUNCTION__, __LINE__, r, w);
        }
    }else if(ret == 0){
        debug("timeout");
        return 0;
    }else{
        return -1;
    }

    return 0;
}


Response HttpRes::httpTransmit(string httphead, int sockfd){

    const char crlf[] = {'\r', '\n'};
    vector <char> responsedata;
    bool firstline = true;
    bool parsedHeader = true;
    bool contengLengthReceived = false;
    unsigned long contengLength = 0;
    bool chunkedResponse = false;
    size_t expectedChunksize = 0;
    bool removeCrlfAfterChunk = false;

    char buf[BUFSIZE];
    memset(buf, 0, sizeof(buf));

    Response response;

    const auto size = send(sockfd, (void*)httphead.c_str(), httphead.size(), 0);
    
    while(true){
        const auto size = recv(sockfd, buf, BUFSIZE, 0);
        if(size < 0){
            debug("recv error\n");
            return ;
        }else if(size == 0)
            break;

        responsedata.insert(responsedata.end(), buf, buf+size);
        if(!parsedHeader){
            while(true){
                const auto pos = search(responsedata.begin(), responsedata.end(), begin(crlf), end(crlf));
                if(pos == responsedata.end()) break;

                const string line(responsedata.begin(), pos);
                responsedata.erase(responsedata.begin(), pos+2);

                if(line.empty){
                    parsedHeader = true;
                    break;
                }else if(firstline){
                    firstline = false;
                    string::size_type lastpos = 0;
                    const auto length = line.length();
                    vector<string> parts;

                    while(lastpos < length + 1){
                        auto p = line.find(' ', lastpos);
                        if(p == string::npos) p = length;

                        if(p != lastpos)
                            parts.emplace_back(line.data()+lastpos, (string::size_type)p - lastpos);

                        lastpos = p +1;
                    }

                    if(parts.size() >= 2)
                        response.status = stoi(parts[1]);
                }else{

                }

            }
        }

    }
    return response;
}