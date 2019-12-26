#include <iostream>
#include "httpres.h"

using namespace std;

int main(){
    Response result;
    HttpRes *http = new HttpRes("https://blog.csdn.net/baidu_18607183/article/details/51200652", -1, "GET", "");
    http->httpRequest(result);
    cout << result.status <<endl;
    cout << result.body.size() << endl;
    for(auto c : result.body){
        cout << c;
    }
    cout << endl;

    return 0;
}