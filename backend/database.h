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
    const std::string BASE_DIR = "../base/";
    std::vector<Category> categories;

    // Внутренняя функция парсинга конкретного файла (например, аккумулятор.txt)
    std::vector<Product> parseProductFile(std::string filename) {
        std::vector<Product> products;
        std::ifstream file(BASE_DIR + filename);
        
        if (!file.is_open()) {
            std::cerr << "Предупреждение: файл не найден: " << filename << std::endl;
            return products;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            
            std::stringstream ss(line);
            Product p;
            std::string sep, availableStr;

            // Парсим по формату: Название - Бренд Цена Наличие Адрес Кол-во Артикул Спеки
            // Пример: Аккумулятор_12V_60Ah - Tudor 7512 Да Москва...
            ss >> p.name >> sep >> p.brand >> p.price >> availableStr >> p.address >> p.quantity >> p.article;

            // Убираем нижние подчеркивания для красивого вывода
            std::replace(p.name.begin(), p.name.end(), '_', ' ');
            std::replace(p.address.begin(), p.address.end(), '_', ' ');

            p.available = (availableStr == "Да");
            
            // Считываем остаток строки (характеристики)
            std::getline(ss, p.specs); 
            if (!p.specs.empty() && p.specs[0] == ' ') p.specs.erase(0, 1);

            products.push_back(p);
        }
        file.close();
        return products;
    }

public:
    // Главная функция: читает auto_parts_main.txt и запускает парсинг всех файлов
    void init() {
        categories.clear();
        std::ifstream mainFile(BASE_DIR + "auto_parts_main.txt");
        
        if (!mainFile.is_open()) {
            std::cerr << "Ошибка: Главный файл базы не найден!" << std::endl;
            return;
        }

        std::string line;
        while (std::getline(mainFile, line)) {
            if (line.empty()) continue;
            
            std::stringstream ss(line);
            Category cat;
            // Формат: ID Название (остальное пока не важно для структуры)
            ss >> cat.id >> cat.name;

            // Готовим имя файла: "Аккумулятор" -> "аккумулятор.txt"
            std::string filename = cat.name;
            for(auto &c : filename) c = tolower((unsigned char)c);
            filename += ".txt";

            // Загружаем товары этой категории
            cat.products = parseProductFile(filename);
            categories.push_back(cat);
        }
        mainFile.close();
    }

    // Метод для получения всех данных (для алгоритмов)
    std::vector<Category>& getCategories() {
        return categories;
    }

    // Поиск категории по имени (например для API)
    Category* findCategory(std::string name) {
        for (auto& cat : categories) {
            if (cat.name == name) return &cat;
        }
        return nullptr;
    }

    void saveCategoryToFile(const Category& cat) {
        std::string filename = cat.name;
        // Приводим к нижнему регистру для имени файла
        for(auto &c : filename) c = tolower((unsigned char)c);
        
        // Путь относительно бинарника в build/ к папке ../base/
        std::ofstream outFile("../base/" + filename + ".txt");
        
        if (outFile.is_open()) {
            for (const auto& p : cat.products) {
                // Форматируем обратно: заменяем пробелы на подчеркивания
                std::string nameSave = p.name;
                std::replace(nameSave.begin(), nameSave.end(), ' ', '_');
                
                std::string addrSave = p.address;
                std::replace(addrSave.begin(), addrSave.end(), ' ', '_');

                outFile << nameSave << " - " << p.brand << " " 
                        << p.price << " " << (p.available ? "Да" : "Нет") << " "
                        << addrSave << " " << p.quantity << " " 
                        << p.article << " " << p.specs << "\n";
            }
            outFile.close();
        }
    }
};

#endif