#include <iostream>
#include "httpres.h"

using namespace std;

int main(){
    string r;
    HttpRes * http = new HttpRes();
    http->httpGet("https://cn.bing.com/search?q=search&qs=n&form=QBLH&sp=-1&pq=search&sc=8-6&sk=&cvid=1C294987CB3241C486C1C57468764144", 80, r);
    cout << r << endl;
    return 0;
}