#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QListWidgetItem>

#include <thread>
#include <future>
#include <exception>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , articles()
{   
    ui->setupUi(this);

    connect(ui->search_bar, &QLineEdit::returnPressed, this, &MainWindow::on_search_button_clicked);
    connect(ui->article_list, &QListWidget::itemActivated, this, &MainWindow::open_selected_article);

    ui->search_size->setValue(10);
}

MainWindow::~MainWindow(){
    delete ui;
}

std::string MainWindow::recieve_json(const std::string& url){
    std::ostringstream response;

    //? Can't be static because it's being used by threads
    curlpp::Easy easy;
    easy.setOpt(curlpp::Options::CaInfo("cacert.pem"));
    easy.setOpt(curlpp::Options::Url(url));
    easy.setOpt(cURLpp::Options::WriteStream(&response));
    easy.perform();

    return response.str();
}

std::string MainWindow::query_to_request(std::string query){
    size_t space_pos = query.find(' ', 0);

    while(space_pos != std::string::npos){
        query.replace(space_pos, 1, "%20");
        space_pos = query.find(' ', space_pos);
    }

    return "https://content.guardianapis.com/search?q=" + query + "&api-key=" + "41cb60b3-01b8-4607-91a6-65d99b9e0b02";
}

std::vector<Article> MainWindow::recieve_articles(const std::string& query){
    nlohmann::json json = nlohmann::json::parse(recieve_json(query_to_request(query)));

    if(json.empty()){
        throw std::runtime_error("Recieved an empty json...\n");
    }

    json = json["response"];

    if(const std::string& status = json["status"]; status != "ok"){
       throw std::runtime_error("Status error -> " + status + '\n');
    }

    json = json["results"];

    return std::vector<Article>{begin(json), end(json)};
}

void MainWindow::on_search_button_clicked(){
    const auto item_count = ui->search_size->text().toUInt();
    const auto query = ui->search_bar->text().toStdString() + "&page=";

    //* Fetch articles in seperate threads
    std::vector<std::future<std::vector<Article>>> futures(item_count);
    for(size_t i = 0, last = futures.size(); i < last; ++i){
       futures[i] = std::async(&MainWindow::recieve_articles, this, query + std::to_string(i + 1));
    }

    //* Clear the previuos articles and append new ones
    articles.clear();
    for(auto& future : futures){
        auto page = future.get();
        articles.insert(std::end(articles), std::begin(page), std::end(page));
    }

    //* Clear the previous headlines and append new ones
    ui->article_list->clear();
    for(auto& article : articles){
        ui->article_list->addItem(article.web_title.c_str());
    }
}

void MainWindow::open_selected_article(){
    articles[ui->article_list->currentRow()].open_url();
}
