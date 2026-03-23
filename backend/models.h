#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>

struct Product {
    std::string name;        // Название (с нижними подчеркиваниями)
    std::string brand;       // Бренд
    double price;            // Цена
    bool available;          // Наличие (Да/Нет -> true/false)
    std::string address;     // Адрес склада
    int quantity;            // Количество
    std::string article;     // Артикул (в файлах пока '-')
    std::string specs;       // Дополнительные характеристики (строка до конца файла)

    // Для QuickSort по умолчанию (по цене)
    bool operator<(const Product& other) const {
        return price < other.price;
    }
};

struct Category {
    int id;
    std::string name;
    int some_val1; // Значения из auto_parts_main.txt
    int some_val2;
    std::vector<Product> products;
};

#endif