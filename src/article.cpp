#include "article.h"
#include "windows.h"
#include <json.h>

Article::Article(const nlohmann::json& json){
    pillar_name = json["pillarName"];
    section_name = json["sectionName"];
    web_publication_date = json["webPublicationDate"];
    web_title = json["webTitle"];
    web_url = json["webUrl"];
}

void Article::open_url(){
    system(("start " + web_url).c_str());
}
