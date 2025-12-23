#pragma once
#include <string>
#include <vector>
#include <map>
#include "simple_json.hpp"

struct Record {
    std::wstring student;
    time_t timestamp = 0;
    std::wstring type;
};

struct StudentStats {
    int inCount = 0;
    int outCount = 0;
    int absenceCount = 0;
    double totalHours = 0.0;
};

class Analyzer {
private:
    std::vector<Record> records;

public:
    void load(const json::Value& root);
    std::map<std::wstring, StudentStats> getFullStats() const;
    size_t size() const { return records.size(); }
};