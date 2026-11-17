#pragma once

#include <string>
#include <vector>
#include "lifetracker.h"

struct account_record {
  std::string username;
  std::string password;
  date start_date;
  double initial_balance{0.0};
};

class account_manager {
private:
  std::string base_dir;

  std::string accounts_file() const;
  std::string user_dir(const std::string& username) const;

public:
  explicit account_manager(const std::string& dir = "data");

  void ensure_directories(const std::string& username) const;

  std::vector<account_record> list_accounts() const;

  bool account_exists(const std::string& username) const;

  bool register_account(const std::string& username, const std::string& password, const date& start_d, double initial_balance, bool seed_demo = true);

  bool authenticate(const std::string& username, const std::string& password, account_record& out_rec) const;

  lifetracker load_user_app(const account_record& rec) const;

  void save_user_app(const lifetracker& app, double initial_balance) const;
};
