#pragma once
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <optional>

namespace json {

    struct Value;

    using ObjectType = std::map<std::wstring, Value>;
    using ArrayType = std::vector<Value>;

    struct Value {
        std::variant<std::monostate, double, std::wstring, bool, ArrayType, ObjectType> data;

        Value() : data(std::monostate{}) {}
        Value(double d) : data(d) {}
        Value(const std::wstring& s) : data(s) {}
        Value(const wchar_t* s) : data(std::wstring(s)) {}
        Value(bool b) : data(b) {}
        Value(const ArrayType& a) : data(a) {}
        Value(const ObjectType& o) : data(o) {}

        std::wstring asString() const;
        double asNumber() const;
        const ArrayType& asArray() const;
        const ObjectType& asObject() const;
    };

    class Parser {
    public:
        static Value parse(const std::wstring& content);

    private:
        std::wstring src;
        size_t pos = 0;

        wchar_t peek();
        wchar_t consume();
        void skipWhitespace();

        Value parseValue();
        Value parseObject();
        Value parseArray();
        Value parseString();
        Value parseNumber();
        Value parseKeyword(const std::wstring& keyword, Value val);
    };
}