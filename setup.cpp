#include "setup.hpp"


using namespace std;


const int NODES = 5;

vector<vector<int>> GRAPH(NODES);


// master process interacts with user
// user wants to read router data
// associate router id with socket fd used for master <-> router IPC
map<int, int> node_socket; 

int main() {

    vector<map<int,int>> socket_map(NODES); // for each node, map neighbor id to socket fd used at conversation
    build_graph(GRAPH);

    for (int nid = 0; nid < NODES; nid++) {


        for (int neigh : GRAPH[nid]) {
            if (neigh > nid) { // examine only bigger nodes => avoid examining same edge twice

                int ends[2];
                if (socketpair(AF_UNIX, SOCK_STREAM, 0, ends) == -1) {
                    perror("socketpair");
                    return 1;
                }
                
                socket_map[nid][neigh] = ends[0];
                socket_map[neigh][nid] = ends[1];
            }

        }
    }

    vector<pid_t> children;
    for (int nid = 0; nid < NODES; nid++) {
        
        // create master <-> router IPC
        int master_socket[2];
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, master_socket) == -1) {
            perror("socketpair");
            return 1;
        }
        node_socket[nid] = master_socket[0];

        pid_t p = fork();

        if (p == 0) {
            vector<string> args = {"./router", to_string(nid)};
           
            // close unrelated socket file descriptors
            for (int other_node = 0; other_node < NODES; other_node++) {
                if (other_node == nid) continue;

                for (auto& mapping : socket_map[other_node])
                    close(mapping.second);   
            }

            close(master_socket[0]);

            // create argument list with pairs (neighbor, socket fd)
            for (int neigh : GRAPH[nid]) {
                args.push_back(to_string(neigh));
                args.push_back(to_string(socket_map[nid][neigh]));
                args.push_back(to_string(master_socket[1]));
            }
            
            vector<char*> argv;
            for (string& arg : args)
                argv.push_back(arg.data());
            
            argv.push_back(nullptr);

            execv("./router", argv.data());
            
        }

        close(master_socket[1]);
        children.push_back(p);
    }


    
    // cleanup sockets not needed by master
    for (int node = 0; node < NODES; node++) {
        for (auto& mapping : socket_map[node])
            close(mapping.second);
    }


    // read stdin, accept only valid router 0 < id < NODES,  
    // send 'S' to the correct socket, recv the answer and write to stdout
    string line;
    cout << "\nEnter router ID to get routing table: ";
    while (getline(cin, line)) {


        istringstream input(line);
        int id;
        string extra;
        if (!(input >> id) || (input >> extra) || id < 0 || id >= NODES)
            continue;

        int fd = node_socket[id];
        send(fd, "S", 1, 0);

        string reply;
        char c;
        while (recv(fd, &c, 1, 0) == 1 && c != '\0')
            reply += c;

        cout << "\nROUTER " << id << " TABLE:\n"
            << "TARGET    |    PATH\n"
            << "----------+------------------------\n";

        istringstream table(reply);
        string row;
        while (getline(table, row)) {
            istringstream nodes(row);
            int target = id, node;
            string path;

            while (nodes >> node) {
                target = node;
                if (!path.empty()) path += " -> ";
                path += to_string(node);
            }

            cout << left << setw(10) << target << "|    "
                << (path.empty() ? "(local)" : path) << '\n';
            
            
        }

        cout << "\nEnter router ID to get routing table: ";
    }

    return 0;

}


void build_graph(vector<vector<int>>& g) {
    // temporary

    g[0] = {1, 2};
    g[1] = {0, 2, 3};
    g[2] = {0, 1, 3, 4};
    g[3] = {1, 2, 4};
    g[4] = {2, 3};
    
}
