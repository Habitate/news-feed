#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QListWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow(){
    delete ui;
}

std::string MainWindow::recieve_json(const std::string& url){
    cURLpp::Easy easyhandle;
    std::ostringstream response;

    easyhandle.setOpt(curlpp::Options::Url(url));
    easyhandle.setOpt(curlpp::Options::CaInfo("cacert.pem"));
    easyhandle.setOpt(cURLpp::Options::WriteStream(&response));

    easyhandle.perform();

    return response.str();
}

std::string MainWindow::query_to_request(std::string request){
    size_t space_pos = request.find(' ', 0);

    while(space_pos != std::string::npos){
        request.replace(space_pos, 1, "%20");
        space_pos = request.find(' ', space_pos);
    }

    std::stringstream request_s;
    request_s << "https://content.guardianapis.com/search?q=" << request << "&api-key=" << "41cb60b3-01b8-4607-91a6-65d99b9e0b02";

    return request_s.str();
}

std::vector<Article> MainWindow::recieve_articles(const std::string& query){
    std::vector<Article> result;

    nlohmann::json json = nlohmann::json::parse(recieve_json(query_to_request(query)));

    if(json.empty()){
        throw std::string("Recieved an empty json...\n");
    }

    json = json["response"];

    if(const std::string& status = json["status"]; status != "ok"){
        throw std::string("Status error -> " + status + '\n');
    }

    auto articles_j = json["results"];

    for(auto& article_j : articles_j){
        result.emplace_back(Article{article_j});
    }

    return result;
}


void MainWindow::on_search_button_clicked(){
    articles = recieve_articles(ui->search_bar->text().toStdString());

    std::for_each(std::begin(article_listing), std::end(article_listing), [](auto& ptr){ delete ptr; });
    article_listing.clear();

    for(auto& article : articles){
        article_listing.emplace_back(new QListWidgetItem(article.web_title.c_str(), ui->article_list));
    }
}

void MainWindow::on_open_button_clicked(){
    for(size_t i = 0, size = article_listing.size(); i < size; ++i){
        if(article_listing[i]->isSelected()){
            articles[i].open_url();
        }
    }
}
