#ifndef USERMANAGER_HPP
#define USERMANAGER_HPP
#include "BPlusTree.hpp"
#include "utility.hpp"
#include <string>
namespace sjtu {
struct User {
    FixedString<20> username; FixedString<30> password; FixedString<20> name; FixedString<30> mailAddr; int privilege; bool logged_in;
    User() : privilege(0), logged_in(false) {}
    User(const std::string &u, const std::string &p, const std::string &n, const std::string &m, int priv)
        : username(u), password(p), name(n), mailAddr(m), privilege(priv), logged_in(false) {}
};
class UserManager {
    BPlusTree<FixedString<20>, User> users;
public:
    UserManager() : users("users.db") {}
    int add_user(const std::string &cur_u, const std::string &u, const std::string &p, const std::string &n, const std::string &m, int g) {
        if (users.empty()) { User newUser(u, p, n, m, 10); users.insert(FixedString<20>(u), newUser); return 0; }
        User curUser; if (!users.find(FixedString<20>(cur_u), curUser) || !curUser.logged_in) return -1;
        if (curUser.privilege <= g) return -1;
        User check; if (users.find(FixedString<20>(u), check)) return -1;
        User newUser(u, p, n, m, g); users.insert(FixedString<20>(u), newUser); return 0;
    }
    int login(const std::string &u, const std::string &p) {
        User user; if (!users.find(FixedString<20>(u), user)) return -1;
        if (user.logged_in || std::string(user.password.c_str()) != p) return -1;
        user.logged_in = true; users.update(FixedString<20>(u), user); return 0;
    }
    int logout(const std::string &u) {
        User user; if (!users.find(FixedString<20>(u), user) || !user.logged_in) return -1;
        user.logged_in = false; users.update(FixedString<20>(u), user); return 0;
    }
    int query_profile(const std::string &c, const std::string &u) {
        User curUser, targetUser; if (!users.find(FixedString<20>(c), curUser) || !curUser.logged_in) return -1;
        if (!users.find(FixedString<20>(u), targetUser)) return -1;
        if (curUser.privilege < targetUser.privilege && c != u) return -1;
        std::cout << targetUser.username.c_str() << " " << targetUser.name.c_str() << " " << targetUser.mailAddr.c_str() << " " << targetUser.privilege << std::endl;
        return 0;
    }
    int modify_profile(const std::string &c, const std::string &u, const std::string &p, const std::string &n, const std::string &m, int g) {
        User curUser, targetUser; if (!users.find(FixedString<20>(c), curUser) || !curUser.logged_in) return -1;
        if (!users.find(FixedString<20>(u), targetUser)) return -1;
        if (curUser.privilege < targetUser.privilege && c != u) return -1;
        if (g != -1 && g >= curUser.privilege) return -1;
        if (!p.empty()) targetUser.password = FixedString<30>(p);
        if (!n.empty()) targetUser.name = FixedString<20>(n);
        if (!m.empty()) targetUser.mailAddr = FixedString<30>(m);
        if (g != -1) targetUser.privilege = g;
        users.update(FixedString<20>(u), targetUser);
        std::cout << targetUser.username.c_str() << " " << targetUser.name.c_str() << " " << targetUser.mailAddr.c_str() << " " << targetUser.privilege << std::endl;
        return 0;
    }
    void clean() { users.clear(); }
    bool is_logged_in(const std::string &u) { User user; if (users.find(FixedString<20>(u), user)) return user.logged_in; return false; }
};
}
#endif
