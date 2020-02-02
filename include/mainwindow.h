#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QListWidgetItem>
#include <QMainWindow>
#include <QDebug>

#include <sstream>
#include <string>
#include <vector>

#include <json.h>

#include "article.h"

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

        nlohmann::json recieve_guardian_response(const std::string& query);

        std::vector<Article> parse_articles(nlohmann::json query);

        std::vector<Article> recieve_articles(const std::string& query, const size_t page_count);

        void sync_articles_with_listings();

        void open_selected_article();

        void refresh_articles();
        void update_refresh_timer_status();

        void validate_page_count(const std::string& query);

    private slots:
        void on_search_button_clicked();

        void on_refresh_toggle_stateChanged(const int state);

    private:
        Ui::MainWindow* ui;

        std::vector<Article> articles;
        const unsigned int default_page_count;

        QTimer* refresh_timer;
        const std::chrono::seconds refresh_timer_duration;

        QTimer* refresh_timer_status_updater;
        const std::chrono::milliseconds refresh_timer_status_updater_duration;
};
#endif // MAINWINDOW_H
