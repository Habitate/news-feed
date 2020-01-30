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

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

        std::string recieve_json(const std::string& url);

        std::string query_to_request(std::string request);

        void recieve_articles(const std::string& query);

    private slots:
        void on_search_button_clicked();

        void on_open_button_clicked();

    private:
        Ui::MainWindow *ui;

        std::string request;
        std::vector<Article> articles;
        std::vector<QListWidgetItem*> article_listing;
};
#endif // MAINWINDOW_H