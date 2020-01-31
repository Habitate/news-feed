#include "article.h"
#include "windows.h"
#include <json.h>
#include <QProcess>

Article::Article(const nlohmann::json& json){
    section_name = json["sectionName"];
    web_publication_date = json["webPublicationDate"];
    web_title = json["webTitle"];
    web_url = json["webUrl"];

    //Remove unnecessary time info (hh:mm:ss)
    web_publication_date.erase(web_publication_date.find("T", 0));
}

void Article::open_url() const{
    QProcess process;

    process.start(("cmd.exe /C start " + web_url).c_str());
    process.waitForFinished();
}

const std::string& Article::get_section_name() const{
    return section_name;
}
const std::string& Article::get_web_publication_date() const{
    return web_publication_date;
}
const std::string& Article::get_web_title() const{
    return web_title;
}
const std::string& Article::get_web_url() const{
    return web_url;
}
