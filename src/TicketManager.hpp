#ifndef TICKETMANAGER_HPP
#define TICKETMANAGER_HPP
#include "TrainManager.hpp"
#include "UserManager.hpp"
#include "utility.hpp"
namespace sjtu {
struct Order {
    FixedString<20> username; FixedString<20> trainID; FixedString<30> from; FixedString<30> to; int leavingTime; int arrivingTime; int price; int num; int status; int timestamp; int day; int fromIdx; int toIdx;
    Order() : leavingTime(0), arrivingTime(0), price(0), num(0), status(0), timestamp(0), day(0), fromIdx(0), toIdx(0) {}
    bool operator==(const Order &o) const { return timestamp == o.timestamp; }
};
struct PendingOrder {
    int timestamp; FixedString<20> username; int num; int fromIdx; int toIdx; int orderTimestamp;
    bool operator==(const PendingOrder &o) const { return timestamp == o.timestamp; }
};
class TicketManager {
    TrainManager &tm; UserManager &um; BPlusTree<pair<FixedString<20>, int>, Order, 20> orders; BPlusTree<pair<pair<FixedString<20>, int>, int>, PendingOrder, 20> pending_queue; int timestamp_counter;
public:
    TicketManager(TrainManager &_tm, UserManager &_um) : tm(_tm), um(_um), orders("orders.db"), pending_queue("pending.db"), timestamp_counter(0) {
        std::fstream f("ts.db", std::ios::in | std::ios::out | std::ios::binary);
        if (!f) { f.open("ts.db", std::ios::out | std::ios::binary); timestamp_counter = 0; f.write(reinterpret_cast<char*>(&timestamp_counter), sizeof(int)); }
        else { f.read(reinterpret_cast<char*>(&timestamp_counter), sizeof(int)); } f.close();
    }
    ~TicketManager() { std::fstream f("ts.db", std::ios::out | std::ios::binary); f.write(reinterpret_cast<char*>(&timestamp_counter), sizeof(int)); f.close(); }
    struct TicketResult {
        FixedString<20> trainID; int leavingTime; int arrivingTime; int price; int seat; int duration;
        void print(const std::string &s, const std::string &t) const { std::cout << trainID.c_str() << " " << s << " " << day_to_date(leavingTime / 1440) << " " << min_to_time(leavingTime % 1440) << " -> " << t << " " << day_to_date(arrivingTime / 1440) << " " << min_to_time(arrivingTime % 1440) << " " << price << " " << seat << std::endl; }
    };
    void query_ticket(const std::string &s, const std::string &t, int d, const std::string &p) {
        sjtu::vector<pair<FixedString<20>, int>> trains_s, trains_t; tm.get_trains_by_station(s, trains_s); tm.get_trains_by_station(t, trains_t);
        sjtu::vector<TicketResult> results;
        for (size_t i = 0; i < trains_s.size(); ++i) {
            for (size_t j = 0; j < trains_t.size(); ++j) {
                if (trains_s[i].first == trains_t[j].first) {
                    int idx_s = trains_s[i].second, idx_t = trains_t[j].second;
                    if (idx_s < idx_t) {
                        TrainInfo train; tm.get_train(trains_s[i].first.c_str(), train); if (!train.released) continue;
                        int time_to_s = train.startTime; for (int k = 0; k < idx_s; ++k) { time_to_s += train.travelTimes[k]; if (k < idx_s - 1) time_to_s += train.stopoverTimes[k]; }
                        int dep_day = d - time_to_s / 1440;
                        if (dep_day >= train.saleDateStart && dep_day <= train.saleDateEnd) {
                            TicketResult res; res.trainID = train.trainID; res.leavingTime = dep_day * 1440 + time_to_s;
                            int time_to_t = train.startTime; for (int k = 0; k < idx_t; ++k) { time_to_t += train.travelTimes[k]; if (k < idx_t - 1) time_to_t += train.stopoverTimes[k]; }
                            res.arrivingTime = dep_day * 1440 + time_to_t; res.duration = res.arrivingTime - res.leavingTime; res.price = train.prices[idx_t] - train.prices[idx_s];
                            SeatArray sa; tm.get_seats(train.trainID.c_str(), dep_day, sa); int min_seat = 1e9; for (int k = idx_s; k < idx_t; ++k) min_seat = std::min(min_seat, sa.seats[k]);
                            res.seat = min_seat; results.push_back(res);
                        }
                    }
                }
            }
        }
        if (p == "time") sort(&results[0], &results[0] + results.size(), [](const TicketResult &a, const TicketResult &b) { if (a.duration != b.duration) return a.duration < b.duration; return a.trainID < b.trainID; });
        else sort(&results[0], &results[0] + results.size(), [](const TicketResult &a, const TicketResult &b) { if (a.price != b.price) return a.price < b.price; return a.trainID < b.trainID; });
        std::cout << results.size() << std::endl; for (size_t i = 0; i < results.size(); ++i) results[i].print(s, t);
    }
    void query_transfer(const std::string &s, const std::string &t, int d, const std::string &p) {
        sjtu::vector<pair<FixedString<20>, int>> trains_s, trains_t; tm.get_trains_by_station(s, trains_s); tm.get_trains_by_station(t, trains_t);
        bool found = false; TicketResult best1, best2; int best_val = 2e9, best_val2 = 2e9; FixedString<30> best_transfer;
        for (size_t i = 0; i < trains_s.size(); ++i) {
            TrainInfo train1; tm.get_train(trains_s[i].first.c_str(), train1); if (!train1.released) continue;
            int idx_s = trains_s[i].second; int time_to_s = train1.startTime; for (int k = 0; k < idx_s; ++k) { time_to_s += train1.travelTimes[k]; if (k < idx_s - 1) time_to_s += train1.stopoverTimes[k]; }
            int dep_day1 = d - time_to_s / 1440; if (dep_day1 < train1.saleDateStart || dep_day1 > train1.saleDateEnd) continue;
            for (int idx_tr1 = idx_s + 1; idx_tr1 < train1.stationNum; ++idx_tr1) {
                FixedString<30> tr_st = train1.stations[idx_tr1]; int time_to_tr = train1.startTime; for (int k = 0; k < idx_tr1; ++k) { time_to_tr += train1.travelTimes[k]; if (k < idx_tr1 - 1) time_to_tr += train1.stopoverTimes[k]; }
                int arr_tr = dep_day1 * 1440 + time_to_tr; sjtu::vector<pair<FixedString<20>, int>> trains_tr; tm.get_trains_by_station(std::string(tr_st.c_str()), trains_tr);
                for (size_t j = 0; j < trains_tr.size(); ++j) {
                    if (trains_tr[j].first == train1.trainID) continue;
                    TrainInfo train2; tm.get_train(trains_tr[j].first.c_str(), train2); if (!train2.released) continue;
                    int idx_tr2 = trains_tr[j].second; int time_to_tr2 = train2.startTime; for (int k = 0; k < idx_tr2; ++k) { time_to_tr2 += train2.travelTimes[k]; if (k < idx_tr2 - 1) time_to_tr2 += train2.stopoverTimes[k]; }
                    int dep_day2 = (arr_tr - time_to_tr2 + 1439) / 1440; if (dep_day2 < train2.saleDateStart) dep_day2 = train2.saleDateStart; if (dep_day2 > train2.saleDateEnd) continue;
                    for (size_t k = 0; k < trains_t.size(); ++k) {
                        if (trains_t[k].first == train2.trainID) {
                            int idx_t = trains_t[k].second; if (idx_tr2 < idx_t) {
                                int time_to_t = train2.startTime; for (int l = 0; l < idx_t; ++l) { time_to_t += train2.travelTimes[l]; if (l < idx_t - 1) time_to_t += train2.stopoverTimes[l]; }
                                int arr_t = dep_day2 * 1440 + time_to_t; int cost = (train1.prices[idx_tr1] - train1.prices[idx_s]) + (train2.prices[idx_t] - train2.prices[idx_tr2]); int time = arr_t - (dep_day1 * 1440 + time_to_s);
                                bool upd = false; if (p == "time") { if (time < best_val) upd = true; else if (time == best_val && cost < best_val2) upd = true; } else { if (cost < best_val) upd = true; else if (cost == best_val && time < best_val2) upd = true; }
                                if (upd) { found = true; best_val = (p == "time" ? time : cost); best_val2 = (p == "time" ? cost : time); best_transfer = tr_st; best1.trainID = train1.trainID; best1.leavingTime = dep_day1 * 1440 + time_to_s; best1.arrivingTime = arr_tr; best1.price = train1.prices[idx_tr1] - train1.prices[idx_s]; SeatArray sa1; tm.get_seats(train1.trainID.c_str(), dep_day1, sa1); int ms1 = 1e9; for (int l = idx_s; l < idx_tr1; ++l) ms1 = std::min(ms1, sa1.seats[l]); best1.seat = ms1; best2.trainID = train2.trainID; best2.leavingTime = dep_day2 * 1440 + time_to_tr2; best2.arrivingTime = arr_t; best2.price = train2.prices[idx_t] - train2.prices[idx_tr2]; SeatArray sa2; tm.get_seats(train2.trainID.c_str(), dep_day2, sa2); int ms2 = 1e9; for (int l = idx_tr2; l < idx_t; ++l) ms2 = std::min(ms2, sa2.seats[l]); best2.seat = ms2; }
                            }
                        }
                    }
                }
            }
        }
        if (!found) std::cout << "0" << std::endl; else { best1.print(s, std::string(best_transfer.c_str())); best2.print(std::string(best_transfer.c_str()), t); }
    }
    long buy_ticket(const std::string &u, const std::string &i, int d, int n, const std::string &f, const std::string &t, bool q) {
        if (!um.is_logged_in(u)) return -1; TrainInfo train; if (!tm.get_train(i, train) || !train.released || n > train.seatNum) return -1;
        int idx_f = -1, idx_t = -1; for (int j = 0; j < train.stationNum; ++j) { if (std::string(train.stations[j].c_str()) == f) idx_f = j; if (std::string(train.stations[j].c_str()) == t) idx_t = j; }
        if (idx_f == -1 || idx_t == -1 || idx_f >= idx_t) return -1;
        int time_to_f = train.startTime; for (int k = 0; k < idx_f; ++k) { time_to_f += train.travelTimes[k]; if (k < idx_f - 1) time_to_f += train.stopoverTimes[k]; }
        int dep_day = d - time_to_f / 1440; if (dep_day < train.saleDateStart || dep_day > train.saleDateEnd) return -1;
        SeatArray sa; tm.get_seats(i, dep_day, sa); int min_seat = 1e9; for (int k = idx_f; k < idx_t; ++k) min_seat = std::min(min_seat, sa.seats[k]);
        Order order; order.username = FixedString<20>(u); order.trainID = FixedString<20>(i); order.from = FixedString<30>(f); order.to = FixedString<30>(t); order.leavingTime = dep_day * 1440 + time_to_f;
        int time_to_t = train.startTime; for (int k = 0; k < idx_t; ++k) { time_to_t += train.travelTimes[k]; if (k < idx_t - 1) time_to_t += train.stopoverTimes[k]; }
        order.arrivingTime = dep_day * 1440 + time_to_t; order.price = train.prices[idx_t] - train.prices[idx_f]; order.num = n; order.timestamp = ++timestamp_counter; order.day = dep_day; order.fromIdx = idx_f; order.toIdx = idx_t;
        if (min_seat >= n) { for (int k = idx_f; k < idx_t; ++k) sa.seats[k] -= n; tm.update_seats(i, dep_day, sa); order.status = 0; orders.insert(pair<FixedString<20>, int>(order.username, -order.timestamp), order); return (long)order.price * n; }
        else if (q) { order.status = 1; orders.insert(pair<FixedString<20>, int>(order.username, -order.timestamp), order); PendingOrder po; po.timestamp = order.timestamp; po.username = order.username; po.num = n; po.fromIdx = idx_f; po.toIdx = idx_t; po.orderTimestamp = order.timestamp; pending_queue.insert(pair<pair<FixedString<20>, int>, int>(pair<FixedString<20>, int>(order.trainID, dep_day), po.timestamp), po); std::cout << "queue" << std::endl; return 0; }
        else return -1;
    }
    void query_order(const std::string &u) {
        if (!um.is_logged_in(u)) { std::cout << "-1" << std::endl; return; }
        sjtu::vector<pair<pair<FixedString<20>, int>, Order>> res; orders.range_search(pair<FixedString<20>, int>(FixedString<20>(u), -2000000000), pair<FixedString<20>, int>(FixedString<20>(u), 0), res);
        std::cout << res.size() << std::endl; for (size_t i = 0; i < res.size(); ++i) { Order &o = res[i].second; std::cout << "[" << (o.status == 0 ? "success" : (o.status == 1 ? "pending" : "refunded")) << "] " << o.trainID.c_str() << " " << o.from.c_str() << " " << day_to_date(o.leavingTime / 1440) << " " << min_to_time(o.leavingTime % 1440) << " -> " << o.to.c_str() << " " << day_to_date(o.arrivingTime / 1440) << " " << min_to_time(o.arrivingTime % 1440) << " " << o.price << " " << o.num << std::endl; }
    }
    int refund_ticket(const std::string &u, int n) {
        if (!um.is_logged_in(u)) return -1; sjtu::vector<pair<pair<FixedString<20>, int>, Order>> res; orders.range_search(pair<FixedString<20>, int>(FixedString<20>(u), -2000000000), pair<FixedString<20>, int>(FixedString<20>(u), 0), res);
        if (n > (int)res.size()) return -1; Order &o = res[n - 1].second; if (o.status == 2) return -1;
        if (o.status == 0) {
            o.status = 2; orders.update(res[n - 1].first, o); SeatArray sa; tm.get_seats(o.trainID.c_str(), o.day, sa); for (int k = o.fromIdx; k < o.toIdx; ++k) sa.seats[k] += o.num;
            sjtu::vector<pair<pair<pair<FixedString<20>, int>, int>, PendingOrder>> pq; pending_queue.range_search(pair<pair<FixedString<20>, int>, int>(pair<FixedString<20>, int>(o.trainID, o.day), 0), pair<pair<FixedString<20>, int>, int>(pair<FixedString<20>, int>(o.trainID, o.day), 2000000000), pq);
            for (size_t i = 0; i < pq.size(); ++i) { PendingOrder &po = pq[i].second; Order po_o; if (orders.find(pair<FixedString<20>, int>(po.username, -po.orderTimestamp), po_o) && po_o.status == 1) { bool can = true; for (int k = po.fromIdx; k < po.toIdx; ++k) if (sa.seats[k] < po.num) { can = false; break; } if (can) { for (int k = po.fromIdx; k < po.toIdx; ++k) sa.seats[k] -= po.num; po_o.status = 0; orders.update(pair<FixedString<20>, int>(po.username, -po.orderTimestamp), po_o); pending_queue.remove(pq[i].first, po); } } }
            tm.update_seats(o.trainID.c_str(), o.day, sa);
        } else {
            o.status = 2; orders.update(res[n - 1].first, o);
            PendingOrder po; po.orderTimestamp = o.timestamp; po.username = o.username; po.num = o.num; po.fromIdx = o.fromIdx; po.toIdx = o.toIdx; po.timestamp = o.timestamp;
            pending_queue.remove(pair<pair<FixedString<20>, int>, int>(pair<FixedString<20>, int>(o.trainID, o.day), o.timestamp), po);
        } return 0;
    }
    void clean() { orders.clear(); pending_queue.clear(); timestamp_counter = 0; }
};
}
#endif
