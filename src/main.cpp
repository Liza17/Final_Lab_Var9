#include "../include/simple_json.hpp"
#include "../include/utils.hpp"
#include "../include/analyzer.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <string>
#include <filesystem>
#include <limits>

#ifdef _WIN32
#include <windows.h>
#endif


std::string to_utf8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
#ifdef _WIN32
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
#else
    return std::string(wstr.begin(), wstr.end());
#endif
}

void writeStudentTable(std::wostream& out, const std::map<std::wstring, StudentStats>& data) {
    out << L"\n" << std::wstring(80, L'-') << L"\n";
    out << std::left
        << std::setw(25) << L"Студент" << L" | "
        << std::setw(8) << L"Вход" << L" | "
        << std::setw(8) << L"Выход" << L" | "
        << std::setw(8) << L"Прогул" << L" | "
        << std::setw(12) << L"Часы" << L"\n";
    out << std::wstring(80, L'-') << L"\n";

    for (const auto& pair : data) {
        out << std::left
            << std::setw(25) << pair.first << L" | "
            << std::setw(8) << pair.second.inCount << L" | "
            << std::setw(8) << pair.second.outCount << L" | "
            << std::setw(8) << pair.second.absenceCount << L" | "
            << std::setw(12) << std::fixed << std::setprecision(1) << pair.second.totalHours << L"\n";
    }
    out << std::wstring(80, L'-') << L"\n";
}

void processFile(std::wstring inputFile, std::wstring outputFile) {
    std::wstringstream report;
    try {
        auto start = std::chrono::high_resolution_clock::now();

        std::wcout << L"\n[1/4] Поиск файла (" << inputFile << L")... ";
        if (!std::filesystem::exists(inputFile)) {
            if (std::filesystem::exists(L"../Generator/data/big_data.json")) inputFile = L"../Generator/data/big_data.json";
            else if (std::filesystem::exists(L"C:\\Projects\\FinalLab\\Generator\\data\\big_data.json")) inputFile = L"C:\\Projects\\FinalLab\\Generator\\data\\big_data.json";
        }
        if (!std::filesystem::exists(inputFile)) {
            std::wcout << L"ОШИБКА!\n";
            throw std::runtime_error("Файл не найден. Проверьте путь или запустите Generator.");
        }
        std::wcout << L"OK.\n";

        std::wcout << L"[2/4] Чтение и Парсинг... ";
        std::wstring content = utils::readFile(inputFile);
        json::Value root = json::Parser::parse(content);
        std::wcout << L"OK.\n";

        auto mid = std::chrono::high_resolution_clock::now();

        std::wcout << L"[3/4] Анализ данных... ";
        Analyzer analyzer;
        analyzer.load(root);
        std::wcout << L"OK.\n";

        auto end = std::chrono::high_resolution_clock::now();
        auto stats = analyzer.getFullStats();

        report << L"\n==========================================\n";
        report << L" ИТОГОВЫЙ ОТЧЕТ\n";
        report << L"==========================================\n";
        writeStudentTable(report, stats);

        std::chrono::duration<double> t1 = mid - start;
        std::chrono::duration<double> t2 = end - mid;

        report << L"\n[ПРОИЗВОДИТЕЛЬНОСТЬ]\n";
        report << L"Всего записей:    " << analyzer.size() << L"\n";
        report << L"Чтение + Парсинг: " << std::fixed << std::setprecision(4) << t1.count() << L" сек\n";
        report << L"Анализ + Расчет:  " << std::fixed << std::setprecision(4) << t2.count() << L" сек\n";

    }
    catch (const std::exception& e) {
        std::wcout << L"\n[ОШИБКА] " << utils::utf8_to_wstring(e.what()) << L"\n";
        return;
    }

    std::wcout << report.str();

    if (!outputFile.empty()) {
        std::ofstream outFile(outputFile);
        if (outFile.is_open()) {
            outFile << to_utf8(report.str());
            std::wcout << L"\n[ФАЙЛ] Отчет сохранен в: " << outputFile << L"\n";
        }
        else {
            std::wcerr << L"\n[ОШИБКА] Не удалось записать файл: " << outputFile << L"\n";
        }
    }
}

void showHelp() {
    std::wcout << L"\n=== СПРАВКА ===\n";
    std::wcout << L"Консольный режим: Main.exe [опции]\n";
    std::wcout << L"  --input <путь>   Указать входной файл\n";
    std::wcout << L"  --output <путь>  Сохранить отчет в файл\n";
    std::wcout << L"  --help           Показать это сообщение\n";
    std::wcout << L"Если запустить без опций, откроется интерактивное меню.\n";
}

int main(int argc, char* argv[]) {
    utils::setupConsole();

    if (argc > 1) {
        std::wstring inputFile = L"data/big_data.json";
        std::wstring outputFile = L"";

        for (int i = 1; i < argc; ++i) {
            std::wstring arg = utils::utf8_to_wstring(argv[i]);
            if (arg == L"--help" || arg == L"-h") { showHelp(); return 0; }
            else if ((arg == L"--input" || arg == L"-i") && i + 1 < argc) inputFile = utils::utf8_to_wstring(argv[++i]);
            else if ((arg == L"--output" || arg == L"-o") && i + 1 < argc) outputFile = utils::utf8_to_wstring(argv[++i]);
        }
        processFile(inputFile, outputFile);
        return 0;
    }

    std::wstring currentInput = L"data/big_data.json";
    std::wstring currentOutput = L"";

    while (true) {
        std::wcout << L"\n=========================================\n";
        std::wcout << L"   СИСТЕМА УЧЕТА ПОСЕЩАЕМОСТИ (МЕНЮ)     \n";
        std::wcout << L"=========================================\n";
        std::wcout << L"Текущий файл: " << currentInput << L"\n";
        std::wcout << L"Сохранение в: " << (currentOutput.empty() ? L"(не сохранять)" : currentOutput) << L"\n\n";

        std::wcout << L"1. Анализировать текущий файл\n";
        std::wcout << L"2. Изменить путь к входному файлу\n";
        std::wcout << L"3. Настроить выходной файл (отчет)\n";
        std::wcout << L"4. Справка (--help)\n";
        std::wcout << L"0. Выход\n";
        std::wcout << L"> Ваш выбор: ";

        int choice;
        if (!(std::wcin >> choice)) {
            std::wcin.clear();
            std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), L'\n');
            continue;
        }

        if (choice == 0) break;

        switch (choice) {
        case 1:
            processFile(currentInput, currentOutput);
            std::wcout << L"\n(Нажмите Enter, чтобы вернуться в меню...)";
            std::wcin.ignore(); std::wcin.get();
            break;
        case 2:
            std::wcout << L"Введите путь к JSON: ";
            std::wcin >> currentInput;
            break;
        case 3:
            std::wcout << L"Введите имя файла для отчета (или '-' чтобы убрать): ";
            std::wcin >> currentOutput;
            if (currentOutput == L"-") currentOutput = L"";
            break;
        case 4:
            showHelp();
            std::wcout << L"\n(Нажмите Enter...)";
            std::wcin.ignore(); std::wcin.get();
            break;
        default:
            std::wcout << L"Неверный выбор.\n";
        }
    }

    return 0;
}