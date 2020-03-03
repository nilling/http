#include <iostream>
#include "httpres.h"

using namespace std;

int main(){
    Response result;
    HttpRes *http = new HttpRes("", 80, "GET", "");
    http->httpRequest(result);
    cout << result.status <<endl;
    cout << result.body.size() << endl;
    for(auto h: result.headers){
        cout << h << endl;
    }
    for(auto c : result.body){
        cout << c;
    }
    cout << endl;

    return 0;
}