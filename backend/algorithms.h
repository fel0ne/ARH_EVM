#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <vector>
#include <string>
#include <algorithm>
#include "models.h"

// --- QUICK SORT (Адаптирован под массив объектов Product) ---
// Сортировка по цене (price)
void quickSort(std::vector<Product>& A, int L, int R) {
    double x = A[(L + R) / 2].price; // Опорный элемент - цена
    int i = L;
    int j = R;
    
    while (i <= j) {
        while (A[i].price < x) i++;
        while (A[j].price > x) j--;
        
        if (i <= j) {
            std::swap(A[i], A[j]);
            i++;
            j--;
        }
    }
    if (L < j) quickSort(A, L, j);
    if (i < R) quickSort(A, i, R);
}

// --- ДЕРЕВО ОПТИМАЛЬНОГО ПОИСКА (Алгоритм А1) ---
// Используется для быстрого поиска по артикулу (ключу)

struct NodeA1 {
    std::string article; // Ключ поиска
    Product data;        // Данные товара
    int weight;          // Частота обращения (вес)
    NodeA1 *left, *right;

    NodeA1(Product p) : article(p.article), data(p), weight(1), left(nullptr), right(nullptr) {
        // trim spaces from stored article
        article.erase(0, article.find_first_not_of(" \t\r\n"));
        article.erase(article.find_last_not_of(" \t\r\n") + 1);
    }
};

class OptimalSearchTree {
private:
    NodeA1* root;

    // Вспомогательная функция для добавления в дерево А1
    // (В методе А1 дерево строится путем вставки элементов в порядке убывания весов в БД)
    void insert(NodeA1*& p, Product pData) {
        if (p == nullptr) {
            p = new NodeA1(pData);
        } else if (pData.article < p->article) {
            insert(p->left, pData);
        } else {
            insert(p->right, pData);
        }
    }

public:
    OptimalSearchTree() : root(nullptr) {}

    // Построение дерева методом А1:
    // 1. Сортируем товары по весу (частоте запросов) через BubbleSort/QuickSort
    // 2. Вставляем их один за другим в обычное дерево поиска
    void buildA1(std::vector<Product>& products) {
    if (products.size() < 2) { // <-- защита от underflow
        for (const auto& p : products) insert(root, p);
        return;
    }
    for (size_t i = 0; i < products.size() - 1; i++) {
        for (size_t j = products.size() - 1; j > i; j--) {
            if (products[j].quantity > products[j - 1].quantity) {
                std::swap(products[j], products[j - 1]);
            }
        }
    }
    for (const auto& p : products) insert(root, p);
}

    // Быстрый поиск товара по артикулу
    Product* search(std::string article) {
        // trim spaces from input article
        article.erase(0, article.find_first_not_of(" \t\r\n"));
        article.erase(article.find_last_not_of(" \t\r\n") + 1);
        NodeA1* curr = root;
        while (curr) {
            if (article == curr->article) return &curr->data;
            if (article < curr->article) curr = curr->left;
            else curr = curr->right;
        }
        return nullptr; // Не найдено
    }
};

#endif