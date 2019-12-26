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

HttpRes::HttpRes(string url, int port, string method, string data):httpsocketfd(INVALID_SOCKET), url(url), port(port), method(method), data(data){
    this->param = getparamfromurl(url);
    this->host = gethostfromurl(url);
    if(host == ""){
        throw_error("get host error\n");
    }
    this->ip = getipfromurl(url);
    if(ip == ""){
        throw_error("get ip error\n");
    }
    int p = getPortfromUrl(url);
    if(p != -1){
        this->port = p;
    }else if (port == -1){
        this->port = 80;
    }
}

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

/*int HttpRes::httpGet(string url, int port, string& response){
    return httpRequest(url, port, "get", "", response);
}

int HttpRes::httpPost(string url, int port, string data, string& response){
    return httpRequest(url, port, "post", data, response);
}
*/

int HttpRes::httpRequest(Response &response){
    return _httpRequest(response);
}

int HttpRes::_httpRequest(Response& response){
    httpheader = httpHeadCreate();

    if(httpsocketfd != INVALID_SOCKET){
        response = httpTransmit();
        return 0;
    }

    httpsocketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(httpsocketfd < 0){
        debug("socket error errorno:%d------>msg:%s\n", get_errno(), get_errmsg().c_str());
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if(inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0){
        debug("inet_pton error, errorno:%d, error msg:%s\n", get_errno(), get_errmsg().c_str());
        clear_socket(httpsocketfd);
        return -1;
    }

    int flags = fcntl(httpsocketfd, F_SETFL, 0);
    if(fcntl(httpsocketfd, F_SETFL, flags|O_NONBLOCK) == -1){
        clear_socket(httpsocketfd);
        debug("fcntl error, errorno:%d, error msg:%s", get_errno(), get_errmsg().c_str());
        return -1;
    }

    int res = connect(httpsocketfd, (struct sockaddr*)&addr, sizeof(addr));
    if(res == 0){
        response = httpTransmit();
    }else if(res < 0){
        if(get_errno() != EINPROGRESS){
            debug("connect error\n");
            return -1;
        }
    }

    res = socketfdcheck(httpsocketfd);
    if(res > 0){
        response = httpTransmit();
    }else{
        clear_socket(httpsocketfd);
        return -1;
    }
    return 0;
}

string HttpRes::httpHeadCreate(){
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
    if(url.find("http://") != string::npos){
        start = 7;
    }else if(url.find("https://") != string::npos){
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
    
    int p = atoi(host.substr(start+1).c_str());

    return p;
}

string HttpRes::getparamfromurl(string url){
    int start;
    if(url.find("http://") != string::npos){
        start = 7;
    }else if(url.find("https://") != string::npos){
        start = 8;
    }else{
        debug("url not correct\n");
        return nullptr;
    }

    string host = url.substr(start);

    int pos = host.find("/");
    if(pos + 1 == host.size()){
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
        if(w && !r){
            char error[4] = "";
            socklen_t len = sizeof(error);
            int res = getsockopt(socketfd, SOL_SOCKET, SO_ERROR, error, &len);
            if(res == 0){
                if(!strcmp(error, "")){
                    return ret;
                }else{
                  debug("%s %s [%d] getsocketopt error code:%d, message:%s\n", __FILE__, __FUNCTION__, __LINE__, get_errno(), get_errmsg().c_str());
                }
            }else{
                 debug("%s %s [%d] getsocketopt error code:%d, message:%s\n", __FILE__, __FUNCTION__, __LINE__, get_errno(), get_errmsg().c_str());
            }
        }else{
             debug("%s %s [%d] readfd:%d, writefd:%d\n", __FILE__, __FUNCTION__, __LINE__, r, w);
        }
    }else if(ret == 0){
        debug("timeout\n");
        return 0;
    }else{
        return -1;
    }

    return 0;
}


Response HttpRes::httpTransmit(){

    const char crlf[] = {'\r', '\n'};
    vector <char> responsedata;
    bool firstline = true;
    bool parsedHeader = false;
    bool contengLengthReceived = false;
    unsigned long contengLength = 0;
    bool chunkedResponse = false;
    size_t expectedChunksize = 0;
    bool removeCrlfAfterChunk = false;

    char buf[BUFSIZE];
    memset(buf, 0, sizeof(buf));

    Response response;

    auto send_size = httpheader.size();
    size_t sent = 0;
    while(send_size > 0){
        const auto size = send(httpsocketfd, httpheader.data()+sent, send_size, 0);
        if(size < 0){
            clear_socket(httpsocketfd);
            debug("send error, errno:%d, errmsg:%s", get_errno(), get_errmsg().c_str());
            throw_error("send error");
        }
        send_size -= size;
        sent += size;
        
    }
    
    while(true){
        const auto size = recv(httpsocketfd, buf, BUFSIZE, 0);
        if(size < 0){
            if(get_errno() == EINTR || get_errno() == EWOULDBLOCK || get_errno() == EAGAIN)
                continue;
            else{
                clear_socket(httpsocketfd);
                debug("recv error, errno:%d, errmsg:%s\n", get_errno(), get_errmsg().c_str());
                throw_error("recv errror\n");
            }
        }else if(size == 0)
            break;

        responsedata.insert(responsedata.end(), buf, buf+size);
        if(!parsedHeader){
            while(true){
                const auto pos = search(responsedata.begin(), responsedata.end(), begin(crlf), end(crlf));
                if(pos == responsedata.end()) break;

                const string line(responsedata.begin(), pos);
                responsedata.erase(responsedata.begin(), pos+2);

                if(line.empty()){
                    parsedHeader = true;
                    break;
                }else if(firstline){
                    firstline = false;
                    response.status = http_status(line);
                }else{//header
                    response.headers.push_back(line);

                    const auto pos = line.find(':');

                    if(pos != string::npos){
                        string headername = line.substr(0, pos);
                        string headervalue = line.substr(pos+1);
                        //trim
                        trim(headervalue);
                        
                        if(headername == "Content-Length"){
                            contengLength = stoul(headervalue);
                            contengLengthReceived = true;
                            response.body.reserve(contengLength);
                        }else if(headername == "Transfer-Encoding"){
                            if(headervalue == "chunked")
                                chunkedResponse = true;
                            else{
                                clear_socket(httpsocketfd);
                                throw runtime_error("Unsupported transfer encoding: " + headervalue);
                            }
                        }
                    }
                }
            }
        }
        
        if(parsedHeader){
            if(chunkedResponse){
                bool dataReceived = false;
                while(true){
                    if(expectedChunksize > 0){
                        const auto toWrite = min(expectedChunksize, responsedata.size());
                        response.body.insert(response.body.end(), responsedata.begin(), responsedata.begin() + toWrite);
                        responsedata.erase(responsedata.begin(), responsedata.begin() + toWrite);
                        expectedChunksize -= toWrite;

                        if(expectedChunksize == 0) removeCrlfAfterChunk = true;
                        if(responsedata.empty()) break;
                    }else{
                        if(removeCrlfAfterChunk){
                            if(responsedata.size() >= 2){
                                removeCrlfAfterChunk = false;
                                responsedata.erase(responsedata.begin(), responsedata.begin() + 2);
                            }else break;
                        }
                        const auto i = search(responsedata.begin(), responsedata.end(), begin(crlf), end(crlf));

                        if(i == responsedata.end()) break;

                        const string line(responsedata.begin(), i);
                        responsedata.erase(responsedata.begin(), i+2);

                        expectedChunksize = stoul(line, nullptr, 16);

                        if(expectedChunksize == 0){
                            dataReceived = true;
                            break;
                        }
                    }
                }

                if(dataReceived) break;

            }else{
                response.body.insert(response.body.end(), responsedata.begin(), responsedata.end());
                responsedata.clear();

                if(contengLengthReceived && response.body.size() >= contengLength)
                    break;
            }
        }
    }
    return response;
}