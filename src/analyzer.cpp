#include "../include/analyzer.hpp"
#include "../include/utils.hpp"
#include <algorithm>
#include <iostream>

void Analyzer::load(const json::Value& root) {
    try {
        const auto& arr = root.asArray();
        records.reserve(arr.size());

        for (const auto& val : arr) {
            try {
                const auto& obj = val.asObject();
                if (!obj.count(L"student") || !obj.count(L"type") || !obj.count(L"ts")) continue;

                Record r;
                r.student = obj.at(L"student").asString();
                r.type = obj.at(L"type").asString();
                r.timestamp = utils::parseISO8601(obj.at(L"ts").asString());

                if (r.timestamp > 0) {
                    records.push_back(r);
                }
            }
            catch (...) { continue; }
        }
    }
    catch (...) {}
}

std::map<std::wstring, StudentStats> Analyzer::getFullStats() const {
    std::map<std::wstring, StudentStats> stats;

    std::map<std::wstring, std::vector<Record>> studentRecords;
    for (const auto& r : records) {
        studentRecords[r.student].push_back(r);

        if (r.type == L"in") stats[r.student].inCount++;
        else if (r.type == L"out") stats[r.student].outCount++;
        else if (r.type == L"absence") stats[r.student].absenceCount++;
    }

    for (auto& [name, recs] : studentRecords) {
        std::sort(recs.begin(), recs.end(), [](const Record& a, const Record& b) {
            return a.timestamp < b.timestamp;
            });

        double totalSeconds = 0;
        time_t lastInTime = 0;
        bool isInside = false;

        for (const auto& r : recs) {
            if (r.type == L"in") {
                if (!isInside) {
                    lastInTime = r.timestamp;
                    isInside = true;
                }
            }
            else if (r.type == L"out") {
                if (isInside) {
                    totalSeconds += difftime(r.timestamp, lastInTime);
                    isInside = false;
                }
            }
        }

        stats[name].totalHours = totalSeconds / 3600.0;
    }

    return stats;
}