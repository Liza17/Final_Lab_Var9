#include "../include/simple_json.hpp"
#include "../include/analyzer.hpp"
#include "../include/utils.hpp"
#include <iostream>
#include <cassert>
#include <cmath>
#include <filesystem>

void assert_equal(int actual, int expected, const std::wstring& label) {
    if (actual == expected) {
        std::wcout << L"[OK] " << label << L"\n";
    }
    else {
        std::wcerr << L"[ОШИБКА] " << label << L" | Ожидалось: " << expected << L", Получено: " << actual << L"\n";
        exit(1);
    }
}

void assert_double(double actual, double expected, const std::wstring& label) {
    if (std::fabs(actual - expected) < 0.001) {
        std::wcout << L"[OK] " << label << L"\n";
    }
    else {
        std::wcerr << L"[ОШИБКА] " << label << L" | Ожидалось: " << expected << L", Получено: " << actual << L"\n";
        exit(1);
    }
}

int main() {
    utils::setupConsole();
    std::wcout << L"Запуск всех тестов (Unit + Integration)...\n";

    std::wcout << L"\n--- Тест 1: Парсинг времени (ISO 8601) ---\n";

    std::wstring timeStrStart = L"2025-10-01T10:00:00Z";
    std::wstring timeStrEnd = L"2025-10-01T12:30:00Z";

    time_t tStart = utils::parseISO8601(timeStrStart);
    time_t tEnd = utils::parseISO8601(timeStrEnd);

    if (tStart > 0 && tEnd > 0) {
        std::wcout << L"[OK] Строки даты успешно преобразованы в числа\n";
    }
    else {
        std::wcerr << L"[ОШИБКА] Функция parseISO8601 вернула 0\n";
        exit(1);
    }
    double diffHours = std::difftime(tEnd, tStart) / 3600.0;
    assert_double(diffHours, 2.5, L"Расчет разницы во времени (2.5 часа)");

    std::wcout << L"\n--- Тест 2: Сценарии посещаемости (Малый пример) ---\n";

    std::wstring jsonInput = L"["
        L"{ \"student\": \"Иванова\",  \"ts\": \"2025-10-01T10:00:00Z\", \"type\": \"in\" },"
        L"{ \"student\": \"Иванова\",  \"ts\": \"2025-10-01T12:30:00Z\", \"type\": \"out\" },"
        L"{ \"student\": \"Петров\",   \"ts\": \"2025-10-01T09:00:00Z\", \"type\": \"in\" },"
        L"{ \"student\": \"Сидорова\", \"ts\": \"2025-10-01T08:00:00Z\", \"type\": \"absence\" }"
        L"]";

    json::Value root = json::Parser::parse(jsonInput);
    Analyzer analyzer;
    analyzer.load(root);
    auto stats = analyzer.getFullStats();

    assert_equal(stats[L"Иванова"].inCount, 1, L"Иванова: Входов (IN)");
    assert_double(stats[L"Иванова"].totalHours, 2.5, L"Иванова: Часы");
    assert_equal(stats[L"Петров"].outCount, 0, L"Петров: Выходов (OUT)");
    assert_double(stats[L"Петров"].totalHours, 0.0, L"Петров: Часы (незамкнутая пара)");
    assert_equal(stats[L"Сидорова"].absenceCount, 1, L"Сидорова: Прогулов");


    std::wcout << L"\n--- Тест 3: Интеграция с Big Data (Реальный файл) ---\n";

    std::wstring filename = L"C:\\Projects\\FinalLab\\Generator\\data\\big_data.json";

    if (!std::filesystem::exists(filename)) {
        if (std::filesystem::exists(L"C:\\Projects\\FinalLab\\Generator\\data\\big_data.json")) {
            filename = L"C:\\Projects\\FinalLab\\Generator\\data\\big_data.json";
        }
        else {
            std::wcout << L"[ПРОПУСК] Файл big_data.json не найден. Запустите сначала Generator.\n";
            std::wcout << L"Тест считается пройденным условно (логика проверена в Тесте 2).\n";
            return 0;
        }
    }

    try {
        std::wcout << L"Чтение файла: " << filename << L"...\n";
        std::wstring content = utils::readFile(filename);

        std::wcout << L"Парсинг JSON...\n";
        json::Value bigRoot = json::Parser::parse(content);

        std::wcout << L"Анализ данных...\n";
        Analyzer bigAnalyzer;
        bigAnalyzer.load(bigRoot);

        // ПРОВЕРКИ:
        size_t count = bigAnalyzer.size();
        std::wcout << L"Загружено записей: " << count << L"\n";

        if (count > 0) {
            std::wcout << L"[OK] Большой файл успешно загружен и проанализирован\n";
        }
        else {
            std::wcerr << L"[ОШИБКА] Файл прочитан, но записей 0 (возможно, формат неверен)\n";
            exit(1);
        }

        auto bigStats = bigAnalyzer.getFullStats();
        if (!bigStats.empty()) {
            std::wcout << L"[OK] Статистика по студентам сформирована. Найдено студентов: " << bigStats.size() << L"\n";
        }
        else {
            std::wcerr << L"[ОШИБКА] Статистика пуста\n";
            exit(1);
        }

    }
    catch (const std::exception& e) {
        std::wcerr << L"[КРИТИЧЕСКАЯ ОШИБКА] Исключение во время работы с Big Data:\n";
        std::cout << e.what() << "\n";
        exit(1);
    }

    std::wcout << L"\n===================================\n";
    std::wcout << L"УСПЕХ: Все тесты (включая Big Data) пройдены!\n";
    std::wcout << L"===================================\n";

    return 0;
}