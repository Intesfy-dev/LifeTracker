#pragma once

#include "account_manager.h"

class terminal_ui {
private:
  account_manager& mgr;

  static void clear_screen();
  void print_banner(const std::string& subtitle) const;
  static void pause_prompt();

  void run_dashboard(lifetracker& app, double initial_balance);

public:
  explicit terminal_ui(account_manager& m);
  int run();
};
