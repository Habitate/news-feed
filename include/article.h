#pragma once

#include <string>

#include <json.h>

class Article{
    public:

    Article(const nlohmann::json& json);

    friend bool operator==(const Article& obj_1, const Article& obj_2){
        if(obj_1.get_web_publication_date() == obj_2.get_web_publication_date()){
            if(obj_1.get_web_title() == obj_2.get_web_title()){
                return true;
            }
        }

        return false;
    }

    const std::string& get_section_name() const;
    const std::string& get_web_publication_date() const;
    const std::string& get_web_title() const;
    const std::string& get_web_url() const;

    void open_url() const;

    private:

    std::string section_name;
    std::string web_publication_date;
    std::string web_title;
    std::string web_url;
};
