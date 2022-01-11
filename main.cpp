#include "crow.h"
#include "sqlite_manager.h"
#include <map>

typedef crow::json::rvalue RValue;
typedef crow::json::wvalue WValue;

typedef std::vector<RValue> RValues;
typedef std::vector<WValue> WValues;
typedef crow::json::type JsonType;

sqlite_manager manager = sqlite_manager();

static const char *file_not_found_error = "File with this id not found in collection";
static const char *illegal_format_error = "Json cannot be parsed";

class checker {
private:
    char *key;
    char *value = nullptr;

    bool contains_key_value(RValue &json_obj, char *key_param, char *value_param) {
        auto json_type = json_obj.t();
        if (json_type == JsonType::Object || json_type == JsonType::List) {
            if (json_obj.has(key_param)) {
                if (value_param == nullptr || json_obj[key_param].s() == value_param)
                    return true;
            }
            for (auto rvalue: json_obj.lo()) {
                if (contains_key_value(rvalue, key_param, value_param))
                    return true;
            }
        }
        return false;
    }

public:
    checker(char *key, char *value) : key(key), value(value) {}

    checker(char *key) : key(key) {}

    bool operator()(crow::json::rvalue json_obj) { return contains_key_value(json_obj, key, value); }
};


crow::response build_response_from_docs(const WValues &documents);

WValues filtered_wvalue_documents(const RValues &documents, checker checker);


int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/").methods(crow::HTTPMethod::POST)([](const crow::request &req) {
        auto x = crow::json::load(req.body);
        if (!x)
            return crow::response(crow::status::BAD_REQUEST, illegal_format_error);
        long long int id = manager.insert_doc(req.body);
        return crow::response(crow::status::OK, std::to_string(id));
    });

    CROW_ROUTE(app, "/<int>").methods(crow::HTTPMethod::PUT)([](const crow::request &req, int id_for_update) {
        auto x = crow::json::load(req.body);
        if (!x)
            return crow::response(crow::status::BAD_REQUEST, illegal_format_error);
        manager.update_by_id(id_for_update, req.body);
        return crow::response(crow::status::OK);
    });

    CROW_ROUTE(app, "/<int>").methods(crow::HTTPMethod::DELETE)([](int id_for_delete) {
        manager.delete_by_id(id_for_delete);
        return crow::response(crow::status::OK);
    });

    CROW_ROUTE(app, "/<int>").methods(crow::HTTPMethod::GET)
            ([](int id_for_get) {
                auto doc_str = manager.get_doc_by_id(id_for_get);
                if(doc_str.empty())
                    return crow::response(crow::status::BAD_REQUEST, file_not_found_error);
                return crow::response(WValue(crow::json::load(doc_str)));
            });

    CROW_ROUTE(app, "/").methods(crow::HTTPMethod::GET)
            ([](const crow::request &req) {
                auto key_param = req.url_params.get("key");
                auto value_param = req.url_params.get("value");
                auto docs = manager.get_docs();
                if (key_param == nullptr) {
                    WValues documents;
                    transform(docs.begin(), docs.end(), back_inserter(documents),
                              [](const std::string& val) { return crow::json::load(val); });
                    return build_response_from_docs(documents);
                } else {
                    RValues documents;
                    transform(docs.begin(), docs.end(), back_inserter(documents),
                              [](const std::string &val) { return crow::json::load(val); });
                    return build_response_from_docs(
                            filtered_wvalue_documents(documents, checker(key_param, value_param)));
                }
            });

    app.port(8084).run();
}

crow::response build_response_from_docs(const WValues &documents) {
    return crow::response(WValue(documents));
}

WValues filtered_wvalue_documents(const RValues &documents, checker checker) {
    WValues filtered_docs;
    std::copy_if(documents.begin(), documents.end(), std::back_inserter(filtered_docs), checker);
    return filtered_docs;
}