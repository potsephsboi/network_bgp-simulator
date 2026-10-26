#include "router.hpp"

using namespace std;

int router_id; 
int NEIGH_COUNT;

map<int,int> neighbors_sockets;

map<int, vector<int>> routes; // map destination to path

int master_socket; // file descriptor

int main(int argc, char* argv[]) {


    router_id = stoi(argv[1]);

    for (int i = 2; i < argc; i+=3) {
        neighbors_sockets[stoi(argv[i])] = stoi(argv[i+1]);
        master_socket = stoi(argv[i+2]);
    }

    NEIGH_COUNT = neighbors_sockets.size();
    

    // advertize self to neighbors 
    routes[router_id] = {};

    vector<int> self = {router_id};
    inform(neighbors_sockets, self);

    vector<pollfd> watched;

    for (auto [neighbor, fd] : neighbors_sockets)
        watched.push_back({fd, POLLIN, 0});

    watched.push_back({master_socket, POLLIN, 0});
    

    while (1) {
        vector<int> P = receive_path(watched);

        if (P.empty()) continue; // this probably never happens

        // check if P creates a cycle
        bool cycle = false;
        for (int node : P)
            if (node == router_id)
                cycle = true;

        if (cycle) continue;
        
        
        int target = P.back();
        int neigh = P.front();

        bool path_changed = false;

        if (!routes.count(target)) {
            routes[target] = P;
            path_changed = true;
        }
        else {

            vector<int>& existing = routes[target];

            if (neigh == existing.front() && P != existing) { // existing path originates from same neighbour => update
                existing = P;
                path_changed = true;
            }

            else if ( P.size() < existing.size() || 
                     (P.size() == existing.size() && P.front() < existing.front())) 
                    
                    {
                        existing = P;
                        path_changed = true;
                    }
            
        }

        if (path_changed) {
            P.insert(P.begin(), router_id);
            inform(neighbors_sockets, P);
        }
    }
    
}


