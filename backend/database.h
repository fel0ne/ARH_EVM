#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include "models.h"

class Database {
private:
    const std::string BASE_DIR = "base/";
    std::vector<Category> categories;

    // Чтение конкретного файла (например, аккумулятор.txt)
    std::vector<Product> parseProductFile(const std::string& filename) {
        std::vector<Product> products;
        std::ifstream file(BASE_DIR + filename);
        
        if (!file.is_open()) {
            // Если нужно видеть, какие файлы не открылись, расскоментируй:
            // std::cerr << "Файл не найден: " << filename << std::endl;
            return products;
        }

        std::string line;
        while (std::getline(file, line)) {
            // Убираем \r для Windows-файлов
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;

            Product p;
            std::stringstream ss(line);

            // 1. Название (заменяем _ на пробел для красоты)
            ss >> p.name;
            std::replace(p.name.begin(), p.name.end(), '_', ' ');

            // 2. Артикул
            ss >> p.article;

            // 3. Производитель
            ss >> p.brand;

            // 4. Цена
            ss >> p.price;

            // 5. Наличие
            std::string availableStr;
            ss >> availableStr;
            p.available = (availableStr == "Да");

            // 6. Адрес
            ss >> p.address;
            std::replace(p.address.begin(), p.address.end(), '_', ' ');

            // 7. Количество
            ss >> p.quantity;

            // 8. Характеристики (всё после символа " - ")
            std::string separator;
            ss >> separator; // Проглатываем "-"
            std::getline(ss, p.specs);
            // Удаляем лишний пробел в начале характеристик
            if (!p.specs.empty() && p.specs[0] == ' ') p.specs.erase(0, 1);

            products.push_back(p);
        }

        file.close();
        return products;
    }

public:
    // Инициализация базы из auto_parts_main.txt
    void init() {
        categories.clear();
        std::ifstream mainFile(BASE_DIR + "auto_parts_main.txt");
        if (!mainFile.is_open()) {
            std::cerr << "Ошибка: auto_parts_main.txt не найден в " << BASE_DIR << std::endl;
            return;
        }

        std::string line;
        while (std::getline(mainFile, line)) {
            // Чистим строку от мусора в конце (\r)
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;

            std::stringstream ss(line);
            Category cat;

            // Читаем ID категории (цифра в начале)
            if (!(ss >> cat.id)) continue;

            // Читаем название категории (остаток строки)
            std::getline(ss, cat.name);
            
            // Удаляем лишние пробелы по краям названия
            cat.name.erase(0, cat.name.find_first_not_of(" \t"));
            cat.name.erase(cat.name.find_last_not_of(" \t") + 1);

            // Формируем имя файла: "фильтр масляный" -> "фильтр_масляный.txt"
            std::string filename = cat.name;
            std::replace(filename.begin(), filename.end(), ' ', '_');
            for (auto &c : filename) c = tolower((unsigned char)c);
            filename += ".txt";

            // Загружаем товары
            cat.products = parseProductFile(filename);
            categories.push_back(cat);
        }

        mainFile.close();
    }

    std::vector<Category>& getCategories() { return categories; }

    Category* findCategory(const std::string& name) {
        for (auto& cat : categories) {
            if (cat.name == name) return &cat;
        }
        return nullptr;
    }

    void saveCategoryToFile(const Category& cat) {
        std::string filename = cat.name;
        std::replace(filename.begin(), filename.end(), ' ', '_');
        for (auto &c : filename) c = tolower((unsigned char)c);

        std::ofstream outFile(BASE_DIR + filename + ".txt");
        if (!outFile.is_open()) return;

        for (const auto& p : cat.products) {
            std::string nameSave = p.name;
            std::replace(nameSave.begin(), nameSave.end(), ' ', '_');

            std::string addrSave = p.address;
            std::replace(addrSave.begin(), addrSave.end(), ' ', '_');

            outFile << nameSave << " " 
                    << p.article << " "
                    << p.brand << " "
                    << p.price << " " 
                    << (p.available ? "Да" : "Нет") << " "
                    << addrSave << " " 
                    << p.quantity << " - " 
                    << p.specs << "\n";
        }
        outFile.close();
    }
};

#endif