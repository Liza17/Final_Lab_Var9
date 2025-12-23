#include "../include/simple_json.hpp"
#include <stdexcept>
#include <cctype>

namespace json {
    std::wstring Value::asString() const {
        if (std::holds_alternative<std::wstring>(data)) return std::get<std::wstring>(data);
        throw std::runtime_error("Type error: expected String");
    }
    double Value::asNumber() const {
        if (std::holds_alternative<double>(data)) return std::get<double>(data);
        throw std::runtime_error("Type error: expected Number");
    }
    const ArrayType& Value::asArray() const {
        if (std::holds_alternative<ArrayType>(data)) return std::get<ArrayType>(data);
        throw std::runtime_error("Type error: expected Array");
    }
    const ObjectType& Value::asObject() const {
        if (std::holds_alternative<ObjectType>(data)) return std::get<ObjectType>(data);
        throw std::runtime_error("Type error: expected Object");
    }

    Value Parser::parse(const std::wstring& content) {
        Parser p;
        p.src = content;
        p.skipWhitespace();
        return p.parseValue();
    }

    wchar_t Parser::peek() { return pos < src.size() ? src[pos] : L'\0'; }
    wchar_t Parser::consume() { return pos < src.size() ? src[pos++] : L'\0'; }

    void Parser::skipWhitespace() {
        while (pos < src.size() && iswspace(src[pos])) pos++;
    }

    Value Parser::parseValue() {
        skipWhitespace();
        wchar_t c = peek();
        if (c == L'{') return parseObject();
        if (c == L'[') return parseArray();
        if (c == L'"') return parseString();
        if (iswdigit(c) || c == L'-') return parseNumber();
        if (c == L't') return parseKeyword(L"true", true);
        if (c == L'f') return parseKeyword(L"false", false);
        if (c == L'n') return parseKeyword(L"null", Value());
        throw std::runtime_error("Unexpected char at position " + std::to_string(pos));
    }

    Value Parser::parseObject() {
        consume(); // {
        ObjectType obj;
        skipWhitespace();
        if (peek() == L'}') { consume(); return obj; }
        while (true) {
            skipWhitespace();
            std::wstring key = parseString().asString();
            skipWhitespace();
            if (consume() != L':') throw std::runtime_error("Expected :");
            obj[key] = parseValue();
            skipWhitespace();
            if (peek() == L'}') { consume(); break; }
            if (consume() != L',') throw std::runtime_error("Expected ,");
        }
        return obj;
    }

    Value Parser::parseArray() {
        consume(); // [
        ArrayType arr;
        skipWhitespace();
        if (peek() == L']') { consume(); return arr; }
        while (true) {
            arr.push_back(parseValue());
            skipWhitespace();
            if (peek() == L']') { consume(); break; }
            if (consume() != L',') throw std::runtime_error("Expected ,");
        }
        return arr;
    }

    Value Parser::parseString() {
        consume(); // "
        std::wstring res;
        while (peek() != L'"' && peek() != L'\0') {
            wchar_t c = consume();
            if (c == L'\\') {
                wchar_t esc = consume();
                if (esc == L'n') res += L'\n'; else res += esc;
            }
            else {
                res += c;
            }
        }
        consume(); // "
        return res;
    }

    Value Parser::parseNumber() {
        size_t start = pos;
        if (peek() == L'-') consume();
        while (iswdigit(peek())) consume();
        if (peek() == L'.') {
            consume();
            while (iswdigit(peek())) consume();
        }
        std::wstring numStr = src.substr(start, pos - start);
        return std::stod(numStr);
    }

    Value Parser::parseKeyword(const std::wstring& keyword, Value val) {
        for (wchar_t c : keyword) {
            if (consume() != c) throw std::runtime_error("Expected keyword");
        }
        return val;
    }
}