#include <iostream>
#include <algorithm>
#include <filesystem> // Для удаления файлов
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "database.h"
#include "algorithms.h"

using json = nlohmann::json;
using namespace httplib;
namespace fs = std::filesystem;

int main() {
    Server svr;
    Database db;
    db.init();

    // CORS настройки
    svr.set_post_routing_handler([](const auto& req, auto& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    });

    svr.Options(R"(/api/.*)", [](const Request&, Response& res) {
        res.status = 200;
    });

    // --- КАТЕГОРИИ ---

    // 1. Получить все категории
    svr.Get("/api/categories", [&](const Request&, Response& res) {
        json j = json::array();
        for (const auto& cat : db.getCategories()) {
            j.push_back({{"id", cat.id}, {"name", cat.name}});
        }
        res.set_content(j.dump(), "application/json");
    });

    // 2. Создать категорию
    svr.Post("/api/categories", [&](const Request& req, Response& res) {
        try {
            auto j = json::parse(req.body);
            Category newCat;
            newCat.name = j.at("name").get<std::string>();
            
            // Генерируем новый ID
            int maxId = 0;
            for(const auto& c : db.getCategories()) if(c.id > maxId) maxId = c.id;
            newCat.id = maxId + 1;

            db.getCategories().push_back(newCat);
            
            // В реальной системе тут нужно обновить auto_parts_main.txt
            // Для упрощения просто создаем пустой файл категории
            db.saveCategoryToFile(newCat); 
            
            res.status = 201;
            res.set_content(json({{"id", newCat.id}}).dump(), "application/json");
        } catch (...) { res.status = 400; }
    });

    // 3. Удалить категорию
    svr.Delete(R"(/api/categories/(\d+))", [&](const Request& req, Response& res) {
        int id = std::stoi(req.matches[1]);
        auto& cats = db.getCategories();
        
        auto it = std::remove_if(cats.begin(), cats.end(), [id](const Category& c) {
            return c.id == id;
        });

        if (it != cats.end()) {
            // Тут можно добавить логику удаления файла категории через fs::remove
            cats.erase(it, cats.end());
            res.status = 204;
        } else {
            res.status = 404;
        }
    });

    // --- ТОВАРЫ ---

    // 4. Получить товары категории (с QuickSort)
    svr.Get("/api/products", [&](const Request& req, Response& res) {
        if (!req.has_param("categoryId")) return (void)(res.status = 400);
        int catId = std::stoi(req.get_param_value("categoryId"));
        
        auto& cats = db.getCategories();
        auto it = std::find_if(cats.begin(), cats.end(), [catId](const Category& c){ return c.id == catId; });

        if (it != cats.end()) {
            auto products_copy = it->products;
            if (!products_copy.empty()) quickSort(products_copy, 0, products_copy.size() - 1);

            json j = json::array();
            for (const auto& p : products_copy) {
                j.push_back({{"name", p.name}, {"price", p.price}, {"article", p.article}, {"quantity", p.quantity}});
            }
            res.set_content(j.dump(), "application/json");
        } else { res.status = 404; }
    });

    // 5. Поиск по артикулу (Дерево А1)
    svr.Get("/api/search", [&](const Request& req, Response& res) {
        std::string art = req.get_param_value("article");
        OptimalSearchTree index;
        for(auto& cat : db.getCategories()) index.buildA1(cat.products);

        Product* found = index.search(art);
        if (found) {
            res.set_content(json({{"name", found->name}, {"price", found->price}}).dump(), "application/json");
        } else { res.status = 404; }
    });

    // 6. Добавить товар
    svr.Post("/api/products", [&](const Request& req, Response& res) {
        try {
            auto j = json::parse(req.body);
            int catId = j.at("categoryId").get<int>();
            
            auto& cats = db.getCategories();
            auto it = std::find_if(cats.begin(), cats.end(), [catId](Category& c){ return c.id == catId; });

            if (it != cats.end()) {
                Product p;
                p.name = j.at("name").get<std::string>();
                p.price = j.at("price").get<double>();
                p.article = j.at("article").get<std::string>();
                p.quantity = j.at("quantity").get<int>();
                p.brand = j.value("brand", "-");
                p.available = p.quantity > 0;

                it->products.push_back(p);
                db.saveCategoryToFile(*it);
                res.status = 201;
            } else { res.status = 404; }
        } catch (...) { res.status = 400; }
    });

    // 7. Удалить товар (по артикулу)
    svr.Delete(R"(/api/products/([\w-]+))", [&](const Request& req, Response& res) {
        std::string article = req.matches[1];
        bool found = false;

        for (auto& cat : db.getCategories()) {
            auto it = std::remove_if(cat.products.begin(), cat.products.end(), [&](const Product& p) {
                return p.article == article;
            });

            if (it != cat.products.end()) {
                cat.products.erase(it, cat.products.end());
                db.saveCategoryToFile(cat); // Сохраняем изменения в файл
                found = true;
                break;
            }
        }
        res.status = found ? 204 : 404;
    });

    std::cout << ">>> Server running on http://localhost:8080" << std::endl;
    svr.listen("0.0.0.0", 8080);
    return 0;
}