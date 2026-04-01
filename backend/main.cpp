#include <iostream>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "database.h"
#include "algorithms.h"

using json = nlohmann::json;
using namespace httplib;
namespace fs = std::filesystem;

// Читаем файл в бинарный вектор
static std::vector<char> readBinaryFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    return std::vector<char>((std::istreambuf_iterator<char>(f)),
                              std::istreambuf_iterator<char>());
}

// Из имени категории делаем имя папки: "глушитель задний" -> "глушитель_задний"
static std::string categoryToFolder(const std::string& catName) {
    std::string folder = catName;
    std::replace(folder.begin(), folder.end(), ' ', '_');
    for (auto& c : folder) c = tolower((unsigned char)c);
    return folder;
}

int main() {
    Server svr;
    Database db;
    db.init();

    OptimalSearchTree index;
    std::vector<Product> allProducts;
    for (auto& cat : db.getCategories()) {
        for (auto& p : cat.products) allProducts.push_back(p);
    }
    index.buildA1(allProducts);

    // CORS
    svr.set_post_routing_handler([](const auto&, auto& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    });
    svr.Options(R"(/api/.*)", [](const Request&, Response& res) { res.status = 200; });

    // --- КАТЕГОРИИ ---
    svr.Get("/api/categories", [&](const Request&, Response& res) {
        json j = json::array();
        for (const auto& cat : db.getCategories())
            j.push_back({{"id", cat.id}, {"name", cat.name}});
        res.set_content(j.dump(), "application/json");
    });

    svr.Post("/api/categories", [&](const Request& req, Response& res) {
        try {
            auto j = json::parse(req.body);
            Category newCat;
            newCat.name = j.at("name").get<std::string>();
            int maxId = 0;
            for (const auto& c : db.getCategories()) if (c.id > maxId) maxId = c.id;
            newCat.id = maxId + 1;
            db.getCategories().push_back(newCat);
            db.saveCategoryToFile(newCat);
            res.status = 201;
            res.set_content(json({{"id", newCat.id}}).dump(), "application/json");
        } catch (...) { res.status = 400; }
    });

    svr.Delete(R"(/api/categories/(\d+))", [&](const Request& req, Response& res) {
        int id = std::stoi(req.matches[1]);
        auto& cats = db.getCategories();
        auto it = std::remove_if(cats.begin(), cats.end(), [id](const Category& c) { return c.id == id; });
        if (it != cats.end()) { cats.erase(it, cats.end()); res.status = 204; }
        else res.status = 404;
    });

    // --- ТОВАРЫ ---
    svr.Get("/api/products", [&](const Request& req, Response& res) {
        if (!req.has_param("categoryId")) return (void)(res.status = 400);
        std::string sort = req.has_param("sort") ? req.get_param_value("sort") : "";
        int catId = std::stoi(req.get_param_value("categoryId"));

        auto& cats = db.getCategories();
        auto it = std::find_if(cats.begin(), cats.end(), [catId](const Category& c) { return c.id == catId; });

        if (it != cats.end()) {
            auto products_copy = it->products;
            if (sort == "price" && !products_copy.empty())
                quickSort(products_copy, 0, products_copy.size() - 1);

            json j = json::array();
            for (const auto& p : products_copy) {
                j.push_back({
                    {"name", p.name}, {"brand", p.brand}, {"price", p.price},
                    {"available", p.available}, {"address", p.address},
                    {"quantity", p.quantity}, {"article", p.article},
                    {"specs", p.specs}, {"images", p.images}  // <-- добавили
                });
            }
            res.set_content(j.dump(), "application/json");
        } else res.status = 404;
    });

    svr.Get(R"(/api/products/([\w-]+))", [&](const Request& req, Response& res) {
        std::string article = req.matches[1];
        Product* found = index.search(article);
        if (found) {
            json j = {
                {"name", found->name}, {"brand", found->brand}, {"price", found->price},
                {"available", found->available}, {"address", found->address},
                {"quantity", found->quantity}, {"article", found->article},
                {"specs", found->specs}, {"images", found->images}  // <-- добавили
            };
            res.set_content(j.dump(), "application/json");
        } else res.status = 404;
    });

    svr.Post("/api/products", [&](const Request& req, Response& res) {
        try {
            auto j = json::parse(req.body);
            int catId = j.at("categoryId").get<int>();
            auto& cats = db.getCategories();
            auto it = std::find_if(cats.begin(), cats.end(), [catId](Category& c) { return c.id == catId; });
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
            } else res.status = 404;
        } catch (...) { res.status = 400; }
    });

    svr.Put("/api/products", [&](const Request& req, Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string article = j.at("article");
            for (auto& cat : db.getCategories()) {
                for (auto& p : cat.products) {
                    if (p.article == article) {
                        p.name = j.value("name", p.name);
                        p.price = j.value("price", p.price);
                        p.quantity = j.value("quantity", p.quantity);
                        p.brand = j.value("brand", p.brand);
                        db.saveCategoryToFile(cat);
                        res.status = 200;
                        return;
                    }
                }
            }
            res.status = 404;
        } catch (...) { res.status = 400; }
    });

    svr.Delete(R"(/api/products/([\w-]+))", [&](const Request& req, Response& res) {
        std::string article = req.matches[1];
        if (article.empty()) { res.status = 400; return; }
        bool found = false;
        for (auto& cat : db.getCategories()) {
            auto it = std::remove_if(cat.products.begin(), cat.products.end(),
                [&](const Product& p) { return p.article == article; });
            if (it != cat.products.end()) {
                cat.products.erase(it, cat.products.end());
                db.saveCategoryToFile(cat);
                found = true;
                break;
            }
        }
        res.status = found ? 204 : 404;
    });

    // --- КАРТИНКИ ---
    // GET /api/images/{categoryFolder}/{filename}.jpg
    // Пример: /api/images/глушитель_задний/00101.jpg
    svr.Get(R"(/api/images/([^/]+)/([^/]+))", [&](const Request& req, Response& res) {
        std::string folder = req.matches[1];
        std::string filename = req.matches[2];

        // Защита от path traversal
        if (folder.find("..") != std::string::npos || filename.find("..") != std::string::npos) {
            res.status = 400; return;
        }

        std::string path = "base/" + folder + "/" + filename;
        auto data = readBinaryFile(path);
        if (data.empty()) { res.status = 404; return; }

        res.set_content(data.data(), data.size(), "image/jpeg");
    });

    std::cout << ">>> Server running on http://localhost:8080" << std::endl;
    svr.listen("0.0.0.0", 8080);
    return 0;
}