#include "router.hpp"


const int RECV_BUF_SIZE = 128;

using namespace std;

void inform(map<int,int> neighbors_sockets, vector<int>& path) {
    string path_repr; 
    for (int node : path) {
        path_repr += to_string(node);
        path_repr += ' ';
    }

    path_repr += '\0';

    for (auto& [node, fd] : neighbors_sockets) {
        size_t sent = 0;

        while (sent < path_repr.size()) {
            ssize_t n = send(fd, path_repr.data() + sent,
                            path_repr.size() - sent, MSG_NOSIGNAL);

            if (n < 0 && errno == EINTR) continue;
            if (n <= 0) throw runtime_error("send failed");

            sent += n;
        }
    }


}



map<int, string> buffers; // map fd used to receive, to the path being built after consecutive recv's

vector<int> receive_path(vector<pollfd>& watched) { 
    while (true) {
        // A previous recv may already have delivered several complete paths
        // Consume those before waiting for more bytes in the kernel
        for (auto& p : watched) {
            string& buf = buffers[p.fd];
            size_t end = buf.find('\0');
            if (end != string::npos) {
                istringstream input(buf.substr(0, end));
                vector<int> res;
                int id;
                while (input >> id)
                    res.push_back(id);
                buf.erase(0, end + 1);
                return res;
            }
        }

        // poll and check for stupid errors
        if (poll(watched.data(), watched.size(), -1) < 0) {
            if (errno == EINTR) continue;
            throw system_error(errno, generic_category(), "poll");
        }

        // poll unblocked and we can receive something
        for (auto& p : watched) {

            if (p.revents & (POLLIN | POLLHUP | POLLERR)) {
                char chunk[RECV_BUF_SIZE];
                ssize_t n = recv(p.fd, chunk, sizeof(chunk), MSG_DONTWAIT);

                if (chunk[0] == 'S')
                    dump_table();
                else
                    buffers[p.fd].append(chunk, n);
            }
        }

    }
}


extern map<int, vector<int>> routes; 
extern int master_socket;

void dump_table() {
    string routes_repr;

    for (auto& [target, path] : routes) {
        for (int node : path) {
            routes_repr += to_string(node); 
            routes_repr += ' ';
        }
        routes_repr += '\n';
    }
    routes_repr += '\0';



    size_t sent = 0;
    while (sent < routes_repr.size()) {
        ssize_t n = send(master_socket, routes_repr.data() + sent,
                        routes_repr.size() - sent, MSG_NOSIGNAL);

        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) throw runtime_error("send failed");

        sent += n;
    }
}

