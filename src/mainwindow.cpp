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
    //* Replace spaces with "%20"
    size_t space_pos = query.find(' ', 0);

    while(space_pos != std::string::npos){
        query.replace(space_pos, 1, "%20");
        space_pos = query.find(' ', space_pos);
    }

    return "https://content.guardianapis.com/search?q=" + query + "&api-key=" + "41cb60b3-01b8-4607-91a6-65d99b9e0b02";
}

nlohmann::json MainWindow::recieve_guardian_response(const std::string& query){
    const auto request = query_to_request(query);
    auto json = nlohmann::json::parse(recieve_json(request));

    if(json.empty()){
        throw std::runtime_error("Recieved an empty json!\nRequest -> " + request + '\n');
    }

    json = json["response"];

    if(const std::string& status = json["status"]; status != "ok"){
       throw std::runtime_error("Status error -> " + status + "\nRequest -> " + request + '\n');
    }

    return json;
}

std::vector<Article> MainWindow::parse_articles(nlohmann::json json){
    json = json["results"];

    return std::vector<Article>{std::begin(json), std::end(json)};
}

void MainWindow::on_search_button_clicked(){
    const auto query = ui->search_bar->text().toStdString() + "&page=";

    //* Resolve the amount of pages to retrieve
    const unsigned int max_page_count = recieve_guardian_response(query + std::to_string(1))["pages"];
    auto requested_page_count = ui->search_size->text().toUInt();

    if(requested_page_count > max_page_count){
        requested_page_count = max_page_count;
        ui->search_size->setValue(requested_page_count);
    }

    //* Fetch articles in seperate threads
    std::vector<std::future<nlohmann::json>> futures(requested_page_count);
    for(size_t i = 0, last = futures.size(); i < last; ++i){
       futures[i] = std::async(&MainWindow::recieve_guardian_response, this, query + std::to_string(i + 1));
    }

    //* Clear the previuos articles and append new ones
    articles.clear();
    for(auto& future : futures){
        auto page = parse_articles(future.get());
        articles.insert(std::end(articles), std::begin(page), std::end(page));
    }

    //* Clear the previous headlines and append new ones
    ui->article_list->clear();
    for(auto& article : articles){
        const auto headline = article.get_web_title() + " | " + article.get_web_publication_date() + " | " + article.get_section_name();
        ui->article_list->addItem(headline.c_str());
    }
}

void MainWindow::open_selected_article(){
    articles[ui->article_list->currentRow()].open_url();
}
