#include <iostream>
#include <string>
#include <sstream>
#include "UserManager.hpp"
#include "TrainManager.hpp"
#include "TicketManager.hpp"
using namespace sjtu;
int main() {
    UserManager um; TrainManager tm; TicketManager tkm(tm, um);
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line); std::string ts_str, cmd; ss >> ts_str >> cmd; std::cout << ts_str << " ";
        if (cmd == "add_user") {
            std::string cur_u, u, p, n, m; int g = -1; std::string key;
            while (ss >> key) { if (key == "-c") ss >> cur_u; else if (key == "-u") ss >> u; else if (key == "-p") ss >> p; else if (key == "-n") ss >> n; else if (key == "-m") ss >> m; else if (key == "-g") ss >> g; }
            std::cout << um.add_user(cur_u, u, p, n, m, g) << std::endl;
        } else if (cmd == "login") {
            std::string u, p; std::string key; while (ss >> key) { if (key == "-u") ss >> u; else if (key == "-p") ss >> p; }
            std::cout << um.login(u, p) << std::endl;
        } else if (cmd == "logout") {
            std::string u; std::string key; while (ss >> key) { if (key == "-u") ss >> u; }
            std::cout << um.logout(u) << std::endl;
        } else if (cmd == "query_profile") {
            std::string c, u; std::string key; while (ss >> key) { if (key == "-c") ss >> c; else if (key == "-u") ss >> u; }
            if (um.query_profile(c, u) == -1) std::cout << "-1" << std::endl;
        } else if (cmd == "modify_profile") {
            std::string c, u, p, n, m; int g = -1; std::string key;
            while (ss >> key) { if (key == "-c") ss >> c; else if (key == "-u") ss >> u; else if (key == "-p") ss >> p; else if (key == "-n") ss >> n; else if (key == "-m") ss >> m; else if (key == "-g") ss >> g; }
            if (um.modify_profile(c, u, p, n, m, g) == -1) std::cout << "-1" << std::endl;
        } else if (cmd == "add_train") {
            std::string i, s_raw, p_raw, t_raw, o_raw, d_raw; int n, m, x; char y; std::string key;
            while (ss >> key) { if (key == "-i") ss >> i; else if (key == "-n") ss >> n; else if (key == "-m") ss >> m; else if (key == "-s") ss >> s_raw; else if (key == "-p") ss >> p_raw; else if (key == "-x") { std::string tmp; ss >> tmp; x = time_to_min(tmp); } else if (key == "-t") ss >> t_raw; else if (key == "-o") ss >> o_raw; else if (key == "-d") ss >> d_raw; else if (key == "-y") ss >> y; }
            sjtu::vector<std::string> s; size_t pos = 0; while ((pos = s_raw.find('|')) != std::string::npos) { s.push_back(s_raw.substr(0, pos)); s_raw.erase(0, pos + 1); } s.push_back(s_raw);
            auto parse_ints = [](std::string raw) { sjtu::vector<int> res; size_t pos = 0; while ((pos = raw.find('|')) != std::string::npos) { res.push_back(std::stoi(raw.substr(0, pos))); raw.erase(0, pos + 1); } if (!raw.empty()) res.push_back(std::stoi(raw)); return res; };
            sjtu::vector<int> p = parse_ints(p_raw), t = parse_ints(t_raw), o = parse_ints(o_raw);
            int d_start = date_to_day(d_raw.substr(0, 5)), d_end = date_to_day(d_raw.substr(6, 5));
            std::cout << tm.add_train(i, n, m, s, p, x, t, o, d_start, d_end, y) << std::endl;
        } else if (cmd == "release_train") {
            std::string i; std::string key; while (ss >> key) if (key == "-i") ss >> i;
            std::cout << tm.release_train(i) << std::endl;
        } else if (cmd == "query_train") {
            std::string i, d_raw; std::string key; while (ss >> key) { if (key == "-i") ss >> i; else if (key == "-d") ss >> d_raw; }
            if (tm.query_train(i, date_to_day(d_raw)) == -1) std::cout << "-1" << std::endl;
        } else if (cmd == "delete_train") {
            std::string i; std::string key; while (ss >> key) if (key == "-i") ss >> i;
            std::cout << tm.delete_train(i) << std::endl;
        } else if (cmd == "query_ticket") {
            std::string s, t, d_raw, p = "time"; std::string key; while (ss >> key) { if (key == "-s") ss >> s; else if (key == "-t") ss >> t; else if (key == "-d") ss >> d_raw; else if (key == "-p") ss >> p; }
            tkm.query_ticket(s, t, date_to_day(d_raw), p);
        } else if (cmd == "query_transfer") {
            std::string s, t, d_raw, p = "time"; std::string key; while (ss >> key) { if (key == "-s") ss >> s; else if (key == "-t") ss >> t; else if (key == "-d") ss >> d_raw; else if (key == "-p") ss >> p; }
            tkm.query_transfer(s, t, date_to_day(d_raw), p);
        } else if (cmd == "buy_ticket") {
            std::string u, i, d_raw, f, t; int n; bool q = false; std::string key;
            while (ss >> key) { if (key == "-u") ss >> u; else if (key == "-i") ss >> i; else if (key == "-d") ss >> d_raw; else if (key == "-n") ss >> n; else if (key == "-f") ss >> f; else if (key == "-t") ss >> t; else if (key == "-q") { std::string tmp; ss >> tmp; q = (tmp == "true"); } }
            long res = tkm.buy_ticket(u, i, date_to_day(d_raw), n, f, t, q); if (res > 0) std::cout << res << std::endl; else if (res == -1) std::cout << "-1" << std::endl;
        } else if (cmd == "query_order") {
            std::string u; std::string key; while (ss >> key) if (key == "-u") ss >> u;
            tkm.query_order(u);
        } else if (cmd == "refund_ticket") {
            std::string u; int n = 1; std::string key; while (ss >> key) { if (key == "-u") ss >> u; else if (key == "-n") ss >> n; }
            std::cout << tkm.refund_ticket(u, n) << std::endl;
        } else if (cmd == "clean") { um.clean(); tm.clean(); tkm.clean(); std::cout << "0" << std::endl; }
        else if (cmd == "exit") { std::cout << "bye" << std::endl; break; }
    }
    return 0;
}
