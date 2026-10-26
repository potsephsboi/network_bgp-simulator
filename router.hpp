#pragma once

#include <bits/stdc++.h>
#include <iostream>
#include <poll.h>
#include <cerrno>
#include <sys/socket.h>

using namespace std;

void inform(map<int,int>,  vector<int>& path);
vector<int> receive_path(vector<pollfd>& watched);
void dump_table();