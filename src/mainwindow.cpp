#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QListWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->article_list, &QListWidget::itemDoubleClicked, this, &MainWindow::open_selected_article);
    connect(ui->search_bar, &QLineEdit::returnPressed, this, &MainWindow::on_search_button_clicked);
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

std::string MainWindow::query_to_request(std::string query){
    size_t space_pos = query.find(' ', 0);

    while(space_pos != std::string::npos){
        query.replace(space_pos, 1, "%20");
        space_pos = query.find(' ', space_pos);
    }

    std::string request = "https://content.guardianapis.com/search?q=" + query + "&api-key=" + "41cb60b3-01b8-4607-91a6-65d99b9e0b02";
    return request;
}

std::vector<Article> MainWindow::recieve_articles(const std::string& query){
    std::vector<Article> result;

    nlohmann::json json = nlohmann::json::parse(recieve_json(query_to_request(query)));

    if(json.empty()){
        throw std::string("Recieved an empty json...\n");
    }

    json = json["response"];


    int num = json["pages"];
    qDebug() << num << '\n';

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
    articles.clear();

    for(size_t i = 1, last = ui->search_size->text().toUInt(); i <= last; ++i){
        auto page = recieve_articles(ui->search_bar->text().toStdString() + "&page=" + std::to_string(i));
        articles.insert(std::end(articles), std::begin(page), std::end(page));
    }

    std::for_each(std::begin(article_listing), std::end(article_listing), [](auto& ptr){ delete ptr; });
    article_listing.clear();
    for(auto& article : articles){
        article_listing.emplace_back(new QListWidgetItem(article.web_title.c_str(), ui->article_list));
    }
}

void MainWindow::open_selected_article(){
    for(size_t i = 0, size = article_listing.size(); i < size; ++i){
        if(article_listing[i]->isSelected()){
            articles[i].open_url();
        }
    }
}
