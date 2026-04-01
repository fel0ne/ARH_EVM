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

    std::vector<Product> parseProductFile(const std::string& filename) {
        std::vector<Product> products;
        std::ifstream file(BASE_DIR + filename);
        if (!file.is_open()) return products;

        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;

            Product p;
            std::stringstream ss(line);

            ss >> p.name;
            std::replace(p.name.begin(), p.name.end(), '_', ' ');

            ss >> p.article;
            p.article.erase(0, p.article.find_first_not_of(" \t\r\n"));
            p.article.erase(p.article.find_last_not_of(" \t\r\n") + 1);

            ss >> p.brand;
            ss >> p.price;

            std::string availableStr;
            ss >> availableStr;
            p.available = (availableStr == "Да");

            ss >> p.address;
            std::replace(p.address.begin(), p.address.end(), '_', ' ');

            ss >> p.quantity;

            // --- Новое: читаем картинки ---
            std::string imagesStr;
            ss >> imagesStr;
            if (imagesStr != "-") {
                // Разбиваем "00101_00102_00103" по '_'
                std::stringstream imgss(imagesStr);
                std::string img;
                while (std::getline(imgss, img, '_')) {
                    if (!img.empty()) p.images.push_back(img);
                }
            }

            std::string separator;
            ss >> separator; // проглатываем "-"
            std::getline(ss, p.specs);
            if (!p.specs.empty() && p.specs[0] == ' ') p.specs.erase(0, 1);

            products.push_back(p);
        }
        file.close();
        return products;
    }

public:
    void init() {
        categories.clear();
        std::ifstream mainFile(BASE_DIR + "auto_parts_main.txt");
        if (!mainFile.is_open()) {
            std::cerr << "Ошибка: auto_parts_main.txt не найден в " << BASE_DIR << std::endl;
            return;
        }

        std::string line;
        while (std::getline(mainFile, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;

            std::stringstream ss(line);
            Category cat;

            // Читаем ID
            if (!(ss >> cat.id)) continue;

            // Читаем слова названия, пока не встретим число
            cat.name = "";
            std::string token;
            std::vector<std::string> extras;
            while (ss >> token) {
                // Если токен — число, это уже some_val, не часть имени
                bool isNumber = !token.empty() && 
                    std::all_of(token.begin(), token.end(), ::isdigit);
                if (isNumber) {
                    extras.push_back(token);
                } else {
                    if (!cat.name.empty()) cat.name += " ";
                    cat.name += token;
                }
            }
            // extras[0] = some_val1, extras[1] = some_val2
            cat.some_val1 = (extras.size() > 0) ? std::stoi(extras[0]) : 0;
            cat.some_val2 = (extras.size() > 1) ? std::stoi(extras[1]) : 0;

            if (cat.name.empty()) continue;

            std::string filename = cat.name;
            std::replace(filename.begin(), filename.end(), ' ', '_');
            for (auto &c : filename) c = tolower((unsigned char)c);
            filename += ".txt";

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

            // Собираем картинки обратно через '_'
            std::string imagesSave = "-";
            if (!p.images.empty()) {
                imagesSave = "";
                for (size_t i = 0; i < p.images.size(); i++) {
                    if (i > 0) imagesSave += "_";
                    imagesSave += p.images[i];
                }
            }

            outFile << nameSave << " "
                    << p.article << " "
                    << p.brand << " "
                    << p.price << " "
                    << (p.available ? "Да" : "Нет") << " "
                    << addrSave << " "
                    << p.quantity << " "
                    << imagesSave << " - "
                    << p.specs << "\n";
        }
        outFile.close();
    }
};

#endif