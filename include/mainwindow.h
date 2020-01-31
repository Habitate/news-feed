#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <string>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <sstream>
#include <vector>

#include "article.h"
#include <QDebug>
#include <json.h>
#include <QListWidgetItem>
#include <curlpp/Easy.hpp>

namespace Ui{
    class MainWindow;
}

class MainWindow : public QMainWindow{
        Q_OBJECT

    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

        std::string recieve_json(const std::string& url);

        std::string query_to_request(std::string query);

        std::vector<Article> recieve_articles(const std::string& query);

        void open_selected_article();

    private slots:
        void on_search_button_clicked();

    private:
        Ui::MainWindow* ui;

        std::vector<Article> articles;
};
#endif // MAINWINDOW_H
