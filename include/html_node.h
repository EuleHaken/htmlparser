#ifndef HTMLTAG_H
#define HTMLTAG_H

#include "global.h"
#include "css_doc.h"
#include "css_parser.h"
#include <string>
#include <vector>

class css_node;
class string_renderer;
class html_node
{
    html_node *_parent;

public:
    html_node();
    virtual ~html_node();
    html_node *parent() const;
    void set_parent(html_node *parent);
    virtual std::wstring outter_html()
    {
        return  L"";
    }
    virtual std::wstring inner_text() const
    {
        return  L"";
    }
//protected:
    virtual void append(string_renderer &r) = 0;
};

class text_node : public html_node
{

    std::wstring _text;

public:
    text_node();
    std::wstring text() const;
    void setText(const std::wstring &text);
    std::wstring outter_html();
    std::wstring inner_text() const;

private:
    void append(string_renderer &r) override;
};

class html_tag : public html_node
{

    std::map<std::wstring, std::wstring> _attributes;
    std::vector<html_node *> _childs;
    bool _has_close_tag;
    css_node *_css;

public:
    std::vector<std::wstring> _classes;
    std::wstring name;

    html_tag();
    virtual ~html_tag();

    std::wstring id();
    std::wstring attr(const std::wstring &name);
    std::wstring data(const std::wstring &name);
    void set_attr(const std::wstring &name, const std::wstring &value);
    void add_class(const std::wstring &name);
    void remove_class(const std::wstring &name);
    bool has_class(const std::wstring &name) const;

    virtual void add_child(html_node *child);

    std::wstring outter_html();
    virtual std::wstring inner_html() const;
    std::wstring inner_text() const;
//    std::wstring name() const;
//    void setName(const std::wstring &name);
    bool hasCloseTag() const;
    void setHasCloseTag(bool hasCloseTag);
    std::vector<html_node *> childs() const;

    std::wstring to_string(print_type type = print_type::compact);

private:
    void append(string_renderer &r) override;
    void append_begin_tag(string_renderer &r);
    void append_inner_html(string_renderer &r);
    void append_end_tag(string_renderer &r);

    friend class html_parser;
};

class style_tag : public html_tag
{

public:
    css_parser rules;
    style_tag();

    void add_child(html_node *child);
//    css_doc rules() const;
//    void setRules(const css_doc &rules);

    std::wstring inner_html() const;

private:
    void append(string_renderer &r) override;
};

#endif // HTMLTAG_H
