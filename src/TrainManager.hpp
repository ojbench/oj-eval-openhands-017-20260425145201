#ifndef TRAINMANAGER_HPP
#define TRAINMANAGER_HPP
#include "BPlusTree.hpp"
#include "utility.hpp"
#include <string>
#include <iostream>
#include <iomanip>
namespace sjtu {
inline int date_to_day(const std::string &d) {
    int m = (d[0] - '0') * 10 + (d[1] - '0'); int day = (d[3] - '0') * 10 + (d[4] - '0');
    if (m == 6) return day - 1; if (m == 7) return 30 + day - 1; if (m == 8) return 30 + 31 + day - 1; return -1;
}
inline std::string day_to_date(int d) {
    int m, day; if (d < 30) { m = 6; day = d + 1; } else if (d < 61) { m = 7; day = d - 30 + 1; } else { m = 8; day = d - 61 + 1; }
    char buf[10]; sprintf(buf, "%02d-%02d", m, day); return std::string(buf);
}
inline int time_to_min(const std::string &t) { return ((t[0] - '0') * 10 + (t[1] - '0')) * 60 + (t[3] - '0') * 10 + (t[4] - '0'); }
inline std::string min_to_time(int m) { int hr = (m / 60) % 24; int mi = m % 60; char buf[10]; sprintf(buf, "%02d:%02d", hr, mi); return std::string(buf); }
struct TrainInfo {
    FixedString<20> trainID; int stationNum; FixedString<30> stations[100]; int prices[100]; int travelTimes[100]; int stopoverTimes[100]; int startTime; int saleDateStart; int saleDateEnd; int seatNum; char type; bool released; bool deleted;
    TrainInfo() : stationNum(0), startTime(0), saleDateStart(0), saleDateEnd(0), seatNum(0), type(' '), released(false), deleted(false) {}
};
struct SeatArray { int seats[100]; };
class TrainManager {
    BPlusTree<FixedString<20>, TrainInfo, 10> trains; BPlusTree<pair<FixedString<30>, FixedString<20>>, int, 60> station_idx; BPlusTree<pair<FixedString<20>, int>, SeatArray, 20> seats;
public:
    TrainManager() : trains("trains.db"), station_idx("station_idx.db"), seats("seats.db") {}
    int add_train(const std::string &i, int n, int m, const sjtu::vector<std::string> &s, const sjtu::vector<int> &p, int x, const sjtu::vector<int> &t, const sjtu::vector<int> &o, int d_start, int d_end, char y) {
        TrainInfo check; if (trains.find(FixedString<20>(i), check) && !check.deleted) return -1;
        TrainInfo train; train.trainID = FixedString<20>(i); train.stationNum = n; train.seatNum = m;
        for (int j = 0; j < n; ++j) train.stations[j] = FixedString<30>(s[j]);
        train.prices[0] = 0; for (int j = 0; j < n - 1; ++j) train.prices[j + 1] = train.prices[j] + p[j];
        train.startTime = x; for (int j = 0; j < n - 1; ++j) train.travelTimes[j] = t[j];
        for (int j = 0; j < n - 2; ++j) train.stopoverTimes[j] = o[j];
        train.saleDateStart = d_start; train.saleDateEnd = d_end; train.type = y; train.released = false; train.deleted = false;
        if (check.deleted) trains.update(FixedString<20>(i), train); else trains.insert(FixedString<20>(i), train); return 0;
    }
    int release_train(const std::string &i) {
        TrainInfo train; if (!trains.find(FixedString<20>(i), train) || train.deleted || train.released) return -1;
        train.released = true; trains.update(FixedString<20>(i), train);
        for (int j = 0; j < train.stationNum; ++j) station_idx.insert(pair<FixedString<30>, FixedString<20>>(train.stations[j], train.trainID), j);
        SeatArray sa; for (int j = 0; j < 100; ++j) sa.seats[j] = train.seatNum;
        for (int d = train.saleDateStart; d <= train.saleDateEnd; ++d) seats.insert(pair<FixedString<20>, int>(train.trainID, d), sa); return 0;
    }
    int delete_train(const std::string &i) {
        TrainInfo train; if (!trains.find(FixedString<20>(i), train) || train.deleted || train.released) return -1;
        train.deleted = true; trains.update(FixedString<20>(i), train); return 0;
    }
    int query_train(const std::string &i, int d) {
        TrainInfo train; if (!trains.find(FixedString<20>(i), train) || train.deleted || d < train.saleDateStart || d > train.saleDateEnd) return -1;
        SeatArray sa; seats.find(pair<FixedString<20>, int>(train.trainID, d), sa);
        std::cout << train.trainID.c_str() << " " << train.type << std::endl;
        int curr_time = train.startTime;
        for (int j = 0; j < train.stationNum; ++j) {
            std::cout << train.stations[j].c_str() << " ";
            if (j == 0) std::cout << "xx-xx xx:xx -> "; else std::cout << day_to_date(d + curr_time / 1440) << " " << min_to_time(curr_time) << " -> ";
            if (j > 0 && j < train.stationNum - 1) curr_time += train.stopoverTimes[j - 1];
            if (j == train.stationNum - 1) std::cout << "xx-xx xx:xx "; else { std::cout << day_to_date(d + curr_time / 1440) << " " << min_to_time(curr_time) << " "; curr_time += train.travelTimes[j]; }
            std::cout << train.prices[j] << " "; if (j == train.stationNum - 1) std::cout << "x" << std::endl; else std::cout << sa.seats[j] << std::endl;
        }
        return 0;
    }
    void clean() { trains.clear(); station_idx.clear(); seats.clear(); }
    bool get_train(const std::string &i, TrainInfo &train) { return trains.find(FixedString<20>(i), train) && !train.deleted; }
    void get_trains_by_station(const std::string &s, sjtu::vector<pair<FixedString<20>, int>> &res) {
        sjtu::vector<pair<pair<FixedString<30>, FixedString<20>>, int>> raw;
        station_idx.range_search(pair<FixedString<30>, FixedString<20>>(FixedString<30>(s), FixedString<20>("")), pair<FixedString<30>, FixedString<20>>(FixedString<30>(s), FixedString<20>("zzzzzzzzzzzzzzzzzzzz")), raw);
        for (size_t i = 0; i < raw.size(); ++i) res.push_back(pair<FixedString<20>, int>(raw[i].first.second, raw[i].second));
    }
    bool get_seats(const std::string &i, int d, SeatArray &sa) { return seats.find(pair<FixedString<20>, int>(FixedString<20>(i), d), sa); }
    void update_seats(const std::string &i, int d, const SeatArray &sa) { seats.update(pair<FixedString<20>, int>(FixedString<20>(i), d), sa); }
};
}
#endif
