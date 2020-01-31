#pragma once

#include <string>
#include <json.h>

class Article{
    public:

    Article(const nlohmann::json& json);
    void open_url() const;

    const std::string& get_section_name() const;
    const std::string& get_web_publication_date() const;
    const std::string& get_web_title() const;
    const std::string& get_web_url() const;

    private:

    std::string section_name;
    std::string web_publication_date;
    std::string web_title;
    std::string web_url;
};
