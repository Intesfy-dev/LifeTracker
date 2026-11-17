#pragma once

#ifdef USE_QT_GUI

#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include <QLabel>
#include <QTableWidget>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QProgressBar>
#include <QListWidget>

#include "account_manager.h"

class RoadmapGraphWidget;

class gui_window : public QMainWindow {
private:
  account_manager& mgr;
  lifetracker current_app;
  double current_initial_balance{0.0};
  bool logged_in{false};

  QStackedWidget* root_stack{nullptr};

  // Auth widgets
  QLineEdit* login_user_edit{nullptr};
  QLineEdit* login_pass_edit{nullptr};
  QLabel* auth_status_lbl{nullptr};

  QLineEdit* reg_user_edit{nullptr};
  QLineEdit* reg_pass_edit{nullptr};
  QLineEdit* reg_date_edit{nullptr};
  QDoubleSpinBox* reg_balance_spin{nullptr};
  QCheckBox* reg_seed_check{nullptr};

  // Main App Shell widgets
  QLabel* header_user_lbl{nullptr};
  QLabel* header_date_lbl{nullptr};
  QLabel* header_score_lbl{nullptr};
  QProgressBar* header_score_bar{nullptr};
  QStackedWidget* module_stack{nullptr};
  std::vector<QPushButton*> nav_buttons;

  // Dashboard page widgets
  QLabel* card_budget_val{nullptr};
  QLabel* card_tasks_val{nullptr};
  QLabel* card_roadmap_val{nullptr};
  QLabel* card_habits_val{nullptr};
  QLabel* card_fitness_val{nullptr};
  std::vector<QProgressBar*> card_vbars;
  QTextEdit* dashboard_console{nullptr};

  // Budget page widgets
  QTableWidget* tx_table{nullptr};
  QTableWidget* goals_table{nullptr};
  QLineEdit* tx_title_edit{nullptr};
  QComboBox* tx_cat_combo{nullptr};
  QLineEdit* tx_date_edit{nullptr};
  QDoubleSpinBox* tx_amount_spin{nullptr};
  QCheckBox* tx_sub_check{nullptr};
  QCheckBox* tx_win_check{nullptr};
  QLineEdit* goal_title_edit{nullptr};
  QDoubleSpinBox* goal_amount_spin{nullptr};
  QLineEdit* goal_date_edit{nullptr};
  QTextEdit* budget_report_box{nullptr};

  // Tasks page widgets
  QTableWidget* tasks_table{nullptr};
  QLineEdit* task_title_edit{nullptr};
  QLineEdit* task_desc_edit{nullptr};
  QSpinBox* task_level_spin{nullptr};
  QLineEdit* task_date_edit{nullptr};
  QDoubleSpinBox* task_prog_spin{nullptr};
  QComboBox* task_complete_combo{nullptr};
  QSpinBox* rank_level_spin{nullptr};
  QTextEdit* tasks_focus_box{nullptr};

  // Calendar page widgets
  QTableWidget* events_table{nullptr};
  QLineEdit* ev_title_edit{nullptr};
  QLineEdit* ev_date_edit{nullptr};
  QSpinBox* ev_hour_spin{nullptr};
  QSpinBox* ev_dur_spin{nullptr};
  QTextEdit* cal_report_box{nullptr};

  // Roadmap page widgets (Interactive Skill-Tree & Obsidian Node Graph)
  RoadmapGraphWidget* roadmap_graph_canvas{nullptr};
  QLabel* rm_selected_title_lbl{nullptr};
  QLabel* rm_selected_meta_lbl{nullptr};
  QListWidget* rm_subtasks_list{nullptr};
  QComboBox* rm_parent_combo{nullptr};
  QLineEdit* rm_sub_edit{nullptr};
  QDoubleSpinBox* rm_weight_spin{nullptr};
  QComboBox* rm_complete_combo{nullptr};
  int selected_rm_node_id{-1};

  // Habits & Fitness page widgets
  QTableWidget* habits_table{nullptr};
  QTableWidget* workouts_table{nullptr};
  QComboBox* hab_checkin_combo{nullptr};
  QLineEdit* hab_title_edit{nullptr};
  QLineEdit* hab_cat_edit{nullptr};
  QSpinBox* hab_days_spin{nullptr};
  QLineEdit* ws_title_edit{nullptr};
  QLineEdit* ws_sport_edit{nullptr};
  QSpinBox* ws_dur_spin{nullptr};
  QSpinBox* ws_cal_spin{nullptr};
  QSpinBox* ws_int_spin{nullptr};

  void apply_emerald_theme();
  QWidget* build_auth_page();
  QWidget* build_main_app_page();

  QWidget* build_dashboard_tab();
  QWidget* build_budget_tab();
  QWidget* build_tasks_tab();
  QWidget* build_calendar_tab();
  QWidget* build_roadmap_tab();
  QWidget* build_habits_fitness_tab();

  void switch_nav_tab(int idx);
  void refresh_all();
  void update_roadmap_inspector();

  void handle_login();
  void handle_register();
  void handle_logout();
  void handle_next_day();
  void handle_binary_rank_gui(int level);

public:
  explicit gui_window(account_manager& m, QWidget* parent = nullptr);
};

#endif
