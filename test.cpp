#include <iostream>
#include "httpres.h"

using namespace std;

int main(){
    string r;
    HttpRes * http = new HttpRes();
    http->httpGet("https://192.168.10.205/", 80, r);
    cout << r << endl;
    return 0;
}