#pragma once

#include <string>
#include <json.h>

class Article{
    public:

    Article(const nlohmann::json& json);
    void open_url();

    std::string section_name;
    std::string web_publication_date;
    std::string web_title;
    std::string web_url;
};
