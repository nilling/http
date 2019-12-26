#include "tool.h"

void ltrim(string &str){
    str.erase(str.begin(), find_if(str.begin(), str.end(), 
                                    [](int c) {return !isspace(c);}));
}

void rtrim(string &str){
    str.erase(find_if(str.rbegin(), str.rend(), 
                        [](int c) {return !isspace(c);}).base(),
                str.end());
}

void trim(string &str){
    ltrim(str);
    rtrim(str);
}

void throw_error(string msg){
    throw runtime_error(msg);
}

int get_errno(){
    return errno;
}

string get_errmsg(){
    return strerror(errno);
}

int http_status(string str){
    string::size_type lastpos = 0;
    const auto length = str.length();
    vector<string> parts;

    while(lastpos < length + 1){
        auto p = str.find(' ', lastpos);
        if(p == string::npos) p = length;

        if(p != lastpos)
            parts.emplace_back(str.data()+lastpos, (string::size_type)p - lastpos);

        lastpos = p +1;
    }

    if(parts.size() >= 2)
        return stoi(parts[1]);
    else return 0;
}