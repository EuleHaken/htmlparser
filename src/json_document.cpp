
#include <algorithm>
#include <iostream>
#include <sstream>

#include "string_helper.h"
#include "json_document.h"
#include "json_object.h"
#include "json_value.h"
#include "json_array.h"
#include "string_renderer.h"

PARSER_BEGIN_NAMESPACE

int json_document::token(int n)
{
    return isprint(n)
            && n != '{' && n != '}'
            && n != '[' && n != ']'
            && n != ':' && n != ';' && n != ',' && n != ' ';
}

json_document::json_document() : token_parser(), _root(nullptr)
{
    this->init();
}

json_document::json_document(json_array *root)
 : token_parser(), _root(root)
{
    init();
}

json_document::json_document(json_object *root)
 : token_parser(), _root(root)
{
    init();
}

std::string json_document::to_string(print_type type) const
{
    string_renderer r(type);
    _root->render(r);
    return r.to_string();
}

json_value *json_document::find(const std::string &path)
{
    if (!_root)
        return nullptr;

    bool ok = std::all_of(path.begin(), path.end(), [](int n){
        return std::isalpha(n) || std::isdigit(n) || n == '.' || n == '_';
    });
    if (!ok) {
        std::cerr << "Invalid path: " << path << std::endl;
        return nullptr;
    }

    std::vector<std::string> strings;
    std::istringstream f(path);
    std::string s;

    auto get = [](const std::string &q, json_value *v) -> json_value* {
        auto arr = dynamic_cast<json_array*>(v);

        if (arr) {
            if (!string_helper::is_integer(q)) {
                std::cerr << "Invalid index: " << q << std::endl;
                return nullptr;
            }
            return arr->at(static_cast<size_t>(std::stoi(q)));
        }

        auto obj = dynamic_cast<json_object*>(v);

        if (obj)
            return obj->get(q);

        return nullptr;
    };
    json_value *v = _root;
    while (getline(f, s, '.')) {
        v = get(s, v);
        if (!v)
            return nullptr;
//        strings.push_back(s);
    }
    return v;
}

bool json_document::is_array() const
{
    if (!_root)
        return false;
    return _root->type() == json_value::type_t::array_t;
}

bool json_document::is_object() const
{
    if (!_root)
        return false;
    return _root->type() == json_value::type_t::object_t;
}

json_array *json_document::to_array()
{
    if (!_root)
        return nullptr;
    return _root->to_array();
}

json_object *json_document::to_object()
{
    if (!_root)
        return nullptr;
    return _root->to_object();
}

json_object *json_document::parse_object()
{
    json_object *obj = new json_object;
    json_value *value = nullptr;
    /*
     name       0
     :          1
     value      2
     ,          3
     */
    int step = 0;
    std::string name;
    bool exec_loop = true;
    while(exec_loop) {

        auto token = take_token();

        if (token.empty())
            break;

//        if (token == "}")
//            break;

        switch (step) {
        case 0: {
            std::string content;
            if (token == "\"" || token == "'") {
                auto begin_quote = token;
                name = take_token();
                auto close_quote = take_token();

                if (begin_quote != close_quote) {
                    print_invalid_token_message(close_quote, begin_quote);
                }
            } else {
                name = token;
            }
            step++;
            continue;
        }
        case 1:
            if (token != ":") {
                print_invalid_token_message(token, ":");
                delete obj;
                return nullptr;
            }

            step++;
            continue;

        case 2: {
            value = parse_value(token);
            step++;
            continue;
        }

        case 3:
            step = 0;
            obj->insert(name, value);

            if (token == "}")
                exec_loop = false;
            continue;
        }

    }
    return obj;
}

json_array *json_document::parse_array()
{
    int step = 0;
    auto arr = new json_array;
    while (true) {
        auto token = take_token();
        if (token.empty())
            break;

        if (token == "]")
            break;

        if (step == 1) {
            //token must be ,
            if (token != ",")
                print_invalid_token_message(token, ",");
            step = 0;
            continue;
        }
        auto tmp = parse_value(token);
        arr->add(tmp);
        step++;
    }
    return arr;
}

void json_document::parse()
{
    _root = parse_value();
}

void json_document::init()
{
    _literals.push_back(new literal_t{"/*", "*/", "", false, false});
    _literals.push_back(new literal_t{"'", "'", "", true, true});
    _literals.push_back(new literal_t{"\"", "\"", "\\\"", true, true});

    _check_fns.push_back(&json_document::token);
}

json_value *json_document::parse_value(const std::string &token)
{
    if (token == "{")
        return parse_object();

    if (token == "[")
        return parse_array();


    if (token == "\"" || token == "'") {
        auto begin_quote = token;
        auto content = take_token();
        auto close_quote = take_token();

        if (begin_quote != close_quote) {
            print_invalid_token_message(close_quote, begin_quote);
        }

        return new json_value(content);
    } else {
        if (token == "null")
            return new json_value();

        if (token == "true" || token == "false")
            return new json_value(token == "true");

        size_t idx = 0;
        try {
            int n = std::stoi(token, &idx);
            if (idx == token.length())
                return new json_value(n);
        } catch (std::exception ex) { }

        try {
            float f = std::stof(token, &idx);
            if (idx == token.length())
                return new json_value(f);
        } catch (std::exception ex) { }

        print_invalid_token_message(token);
        return nullptr;
    }
}

json_value *json_document::parse_value()
{
    auto token = take_token();
    return parse_value(token);
}

PARSER_END_NAMESPACE
