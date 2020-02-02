#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QListWidgetItem>
#include <QDebug>
#include <QTimer>
#include <QDate>

#include <exception>
#include <algorithm>
#include <thread>
#include <future>
#include <chrono>

#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , articles()
    , default_page_count(20)
    , refresh_timer(new QTimer(this))
    , refresh_timer_duration(20)
    , refresh_timer_status_updater(new QTimer(this))
    , refresh_timer_status_updater_duration(200)

{   
    ui->setupUi(this);

    connect(ui->search_bar, &QLineEdit::returnPressed, this, &MainWindow::on_search_button_clicked);
    connect(ui->article_list, &QListWidget::itemActivated, this, &MainWindow::open_selected_article);
    connect(refresh_timer, &QTimer::timeout, this, &MainWindow::refresh_articles);
    connect(refresh_timer_status_updater, &QTimer::timeout, this, &MainWindow::update_refresh_timer_status);

    ui->search_size->setValue(default_page_count);

    refresh_timer->setInterval(refresh_timer_duration);
    refresh_timer_status_updater->setInterval(refresh_timer_status_updater_duration);

    if(ui->refresh_toggle->isChecked()){
        refresh_timer->start();
        refresh_timer_status_updater->start();
    }
}

MainWindow::~MainWindow(){
    delete ui;
    delete refresh_timer;
    delete refresh_timer_status_updater;
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

void MainWindow::validate_page_count(const std::string& query){
    const unsigned int max_page_count = recieve_guardian_response(query)["pages"];
    auto requested_page_count = ui->search_size->text().toUInt();

    //* Resolve the amount of pages to retrieve
    if(requested_page_count > max_page_count){
        ui->search_size->setValue(max_page_count);
    }
}

/*
 * Implemented this way due to the fact that the max page size is 200.
 * Recieving one large page isalso much slower than multiple smaller ones.
*/
std::vector<Article> MainWindow::recieve_articles(const std::string& query, const size_t page_count){
    std::vector<Article> result;

    //* Fetch articles in seperate threads
    std::vector<std::future<nlohmann::json>> futures(page_count);
    for(size_t i = 0; i < page_count; ++i){
        futures[i] = std::async(&MainWindow::recieve_guardian_response, this, query + "&page=" + std::to_string(i + 1));
    }

    //* Insert all the articles into a single vector
    for(auto& future : futures){
        auto page = parse_articles(future.get());
        result.insert(std::end(result), std::begin(page), std::end(page));
    }

    return result;
}

void MainWindow::sync_articles_with_listings(){
    //* Clear the previous headlines and append new ones
    ui->article_list->clear();
    for(const auto& article : articles){
        const auto headline = article.get_web_title() + " | " + article.get_web_publication_date() + " | " + article.get_section_name();
        ui->article_list->addItem(headline.c_str());
    }
}

void MainWindow::on_search_button_clicked(){
    const auto query = ui->search_bar->text().toStdString();
    validate_page_count(query);

    const auto page_count = ui->search_size->text().toUInt();

    articles = recieve_articles(query, page_count);
    sync_articles_with_listings();
}

void MainWindow::open_selected_article(){
    articles[ui->article_list->currentRow()].open_url();
}

void MainWindow::on_refresh_toggle_stateChanged(const int state){
    if(state){
        refresh_timer_status_updater->start();
        refresh_timer->start();
    }
    else{
        ui->refresh_status->setText("");

        refresh_timer_status_updater->stop();
        refresh_timer->stop();
    }
}

//TODO: Make this efficent
void MainWindow::refresh_articles(){
    auto query = ui->search_bar->text().toStdString();
    if(query.empty()){
        return;
    }

    query += "&from-date=" + QDate::currentDate().toString("yyyy-MM-dd").toStdString();

    const unsigned int page_count = recieve_guardian_response(query)["pages"];

    //* Recieve new articles and insert them if they are actually new
    bool got_something_new = false;

    auto new_articles = recieve_articles(query, page_count);
    for(size_t i = 0, last = new_articles.size(); i < last; ++i){
        if(std::find(std::begin(articles), std::end(articles), new_articles[i]) == std::end(articles)){
            articles.insert(std::begin(articles), new_articles[i]);
            got_something_new = true;
        }
    }

    if(got_something_new){
        sync_articles_with_listings();
    }
}

void MainWindow::update_refresh_timer_status(){
    std::string text;

    if(refresh_timer->remainingTime() / 1000){
        text = "Refreshing in " + std::to_string(refresh_timer->remainingTime() / 1000) + "...";
    }
    else{
        text = "Refreshing...";
    }

    ui->refresh_status->setText(text.c_str());
}
