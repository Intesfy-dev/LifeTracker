#pragma once

#include "account_manager.h"

class terminal_ui {
private:
  account_manager& mgr;

  void clear_screen() const;
  void print_banner(const std::string& subtitle) const;
  void pause_prompt() const;

  void run_dashboard(lifetracker& app, double initial_balance);

public:
  explicit terminal_ui(account_manager& m);
  int run();
};
