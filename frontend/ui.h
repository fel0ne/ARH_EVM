#ifndef UI_H
#define UI_H

#include "imgui.h"
#include "api_client.h"
#include <string>
#include <sstream>
#include <map>

class UI {
private:
    ApiClient* api;
    int selectedCategoryId = -1;
    std::vector<ProductDTO> products;
    std::vector<std::pair<int, std::string>> categories;
    bool categoriesLoaded = false;
    char search[128] = "";
    int sortMode = 0;

    bool openViewPopup = false;
    bool openEditPopup = false;

    // buffers для добавления товара
    char nameBuf[128] = "";
    char brandBuf[128] = "";
    float priceBuf = 0.0f;
    int quantityBuf = 0;
    char addressBuf[128] = "";
    char articleBuf[64] = "";
    char specsBuf[256] = "";
    bool availableBuf = true;

    // buffers для редактирования
    char editName[128] = "";
    char editBrand[128] = "";
    float editPrice = 0.0f;
    int editQuantity = 0;
    char editAddress[128] = "";
    char editArticle[64] = "";
    char editSpecs[256] = "";
    bool editAvailable = true;

    int selectedIndex = -1;

public:
    UI(ApiClient* apiClient) : api(apiClient) {}

    void update() {
        ImGui::Begin("AutoParts Manager");

        drawCategories();
        ImGui::SameLine();
        drawProducts();

        ImGui::End();
    }

private:

    std::map<std::string, std::string> parseSpecs(const std::string& specs) {
        std::map<std::string, std::string> result;
        std::stringstream ss(specs);
        std::string pair;

        while (ss >> pair) {
            size_t pos = pair.find(':');
            if (pos != std::string::npos) {
                std::string key = pair.substr(0, pos);
                std::string value = pair.substr(pos + 1);
                result[key] = value;
            }
        }
        return result;
    }


    void drawCategories() {
        ImGui::BeginChild("Categories", ImVec2(200, 0), true);

        ImGui::Text("Категории");
        ImGui::Separator();

        static bool loadedOnce = false;
        if (!loadedOnce) {
            categories = api->getCategories();
            loadedOnce = true;
        }

        for (auto& [id, name] : categories) {
            if (ImGui::Selectable(name.c_str(), selectedCategoryId == id)) {
                selectedCategoryId = id;
                std::string sort = (sortMode == 1) ? "price" : "";
                products = api->getProducts(id, sort);
                selectedIndex = -1;
            }
        }

        ImGui::EndChild();
    }

    void drawProducts() {
        ImGui::BeginChild("Products", ImVec2(0, 0), true);

        if (selectedCategoryId == -1) {
            ImGui::Text("Выберите категорию");
            ImGui::EndChild();
            return;
        }

        ImGui::Text("Категория ID: %d", selectedCategoryId);

        ImGui::InputText("Поиск", search, IM_ARRAYSIZE(search));
        ImGui::RadioButton("Без сортировки", &sortMode, 0);
        ImGui::SameLine();
        ImGui::RadioButton("По цене", &sortMode, 1);

        if (ImGui::Button("Обновить")) {
            std::string sort = (sortMode == 1) ? "price" : "";
            products = api->getProducts(selectedCategoryId, sort);
        }

        ImGui::Separator();

        if (ImGui::BeginTable("ProductsTable", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Название");
            ImGui::TableSetupColumn("Бренд");
            ImGui::TableSetupColumn("Цена");
            ImGui::TableSetupColumn("Наличие");
            ImGui::TableSetupColumn("Адрес");
            ImGui::TableSetupColumn("Кол-во");
            ImGui::TableSetupColumn("Артикул");
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < products.size(); ++i) {
                auto& p = products[i];

                if (strlen(search) > 0 && p.name.find(search) == std::string::npos)
                    continue;

                ImGui::PushID((int)i);

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                if (ImGui::Selectable(p.name.c_str())) {
                    selectedIndex = i;

                    strcpy(editName, p.name.c_str());
                    strcpy(editBrand, p.brand.c_str());
                    strcpy(editAddress, p.address.c_str());
                    strcpy(editArticle, p.article.c_str());
                    strcpy(editSpecs, p.specs.c_str());
                    editPrice = p.price;
                    editQuantity = p.quantity;
                    editAvailable = p.available;

                    openViewPopup = true;
                }

                ImGui::TableNextColumn(); ImGui::Text("%s", p.brand.c_str());
                ImGui::TableNextColumn(); ImGui::Text("%.2f", p.price);
                ImGui::TableNextColumn(); ImGui::Text(p.available ? "Да" : "Нет");

                ImGui::TableNextColumn();
                if (p.available)
                    ImGui::Text("%s", p.address.c_str());
                else
                    ImGui::Text("-");

                ImGui::TableNextColumn(); ImGui::Text("%d", p.quantity);
                ImGui::TableNextColumn(); ImGui::Text("%s", p.article.c_str());

                ImGui::PopID();
            }

            ImGui::EndTable();
        }

        // --- Popup просмотра ---
        if (openViewPopup) {
            ImGui::OpenPopup("ViewPopup");
            openViewPopup = false;
        }

        if (selectedIndex >= 0 && ImGui::BeginPopupModal("ViewPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

            auto& p = products[selectedIndex];

            ImGui::Text("%s", p.name.c_str());
            ImGui::Separator();

            ImGui::Text("Бренд: %s", p.brand.c_str());
            ImGui::Text("Цена: %.2f", p.price);
            ImGui::Text("Наличие: %s", p.available ? "Да" : "Нет");

            if (p.available)
                ImGui::Text("Адрес: %s", p.address.c_str());

            ImGui::Text("Количество: %d", p.quantity);

            ImGui::Separator();
            ImGui::Text("Характеристики:");

            auto specs = parseSpecs(p.specs);
            for (auto& [k, v] : specs) {
                ImGui::BulletText("%s: %s", k.c_str(), v.c_str());
            }

            ImGui::Separator();

            if (ImGui::Button("Редактировать")) {
                openEditPopup = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("Удалить")) {
                if (!p.article.empty()) {
                    api->deleteProduct(p.article);
                } else {
                    std::cout << "ERROR: empty article on frontend" << std::endl;
                }
                std::string sort = (sortMode == 1) ? "price" : "";
                products = api->getProducts(selectedCategoryId, sort);
                selectedIndex = -1;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Закрыть")) {
                selectedIndex = -1;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
            if (openEditPopup) {
                ImGui::OpenPopup("EditPopup");
                openEditPopup = false;
            }
        }

        // --- Popup редактирования ---
        if (ImGui::BeginPopupModal("EditPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

            ImGui::Text("Редактирование");
            ImGui::Separator();

            ImGui::InputText("Название", editName, IM_ARRAYSIZE(editName));
            ImGui::InputText("Бренд", editBrand, IM_ARRAYSIZE(editBrand));
            ImGui::InputFloat("Цена", &editPrice);
            ImGui::Checkbox("В наличии", &editAvailable);
            ImGui::InputText("Адрес", editAddress, IM_ARRAYSIZE(editAddress));
            ImGui::InputInt("Количество", &editQuantity);
            ImGui::InputText("Артикул", editArticle, IM_ARRAYSIZE(editArticle));
            ImGui::InputText("Характеристики", editSpecs, IM_ARRAYSIZE(editSpecs));

            if (ImGui::Button("Сохранить")) {
                ProductDTO dto;
                dto.name = editName;
                dto.brand = editBrand;
                dto.price = editPrice;
                dto.available = editAvailable;
                dto.address = editAddress;
                dto.quantity = editQuantity;
                dto.article = editArticle;
                dto.specs = editSpecs;

                api->updateProduct(dto);
                products = api->getProducts(selectedCategoryId);
                selectedIndex = -1;

                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Отмена")) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::Separator();

        drawAddProduct();

        ImGui::EndChild();
    }

    void drawAddProduct() {
        if (ImGui::CollapsingHeader("Добавить товар")) {

            ImGui::InputText("Название", nameBuf, IM_ARRAYSIZE(nameBuf));
            ImGui::InputText("Бренд", brandBuf, IM_ARRAYSIZE(brandBuf));
            ImGui::InputFloat("Цена", &priceBuf);
            ImGui::Checkbox("В наличии", &availableBuf);
            ImGui::InputText("Адрес", addressBuf, IM_ARRAYSIZE(addressBuf));
            ImGui::InputInt("Количество", &quantityBuf);
            ImGui::InputText("Артикул", articleBuf, IM_ARRAYSIZE(articleBuf));
            ImGui::InputText("Характеристики", specsBuf, IM_ARRAYSIZE(specsBuf));

            if (ImGui::Button("Добавить")) {
                ProductDTO p;
                p.name = nameBuf;
                p.brand = brandBuf;
                p.price = priceBuf;
                p.available = availableBuf;
                p.address = addressBuf;
                p.quantity = quantityBuf;
                p.article = articleBuf;
                p.specs = specsBuf;

                api->addProduct(selectedCategoryId, p);
                products = api->getProducts(selectedCategoryId);
                selectedIndex = -1;

                clearForm();
            }
        }
    }

    void clearForm() {
        nameBuf[0] = 0;
        brandBuf[0] = 0;
        addressBuf[0] = 0;
        articleBuf[0] = 0;
        specsBuf[0] = 0;
        priceBuf = 0.0f;
        quantityBuf = 0;
        availableBuf = true;
    }
};

#endif