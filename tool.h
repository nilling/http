#ifndef TOOL
#define TOOL

#include <string.h>
#include <iostream>
#include <algorithm>
#include <error.h>

using namespace std;

void ltrim(string &str);
void rtrim(string &str);
void trim(string &str);

void throw_error(string msg);

int http_status(string str);

int get_errno();
string get_errmsg();

#endif