#include "account_manager.h"
#include "io_utils.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

account_manager::account_manager(const std::string& dir) : base_dir{dir} {
  fs::create_directories(base_dir);
  fs::create_directories(base_dir + "/users");

  // Daca nu exista accounts.csv, dar exista user.txt, importam contul implicit
  if (!fs::exists(accounts_file()) && fs::exists(base_dir + "/user.txt")) {
    std::ifstream uf{base_dir + "/user.txt"};
    std::string u, p;
    date d;
    double bal = 0;
    char sep = ',';
    std::getline(uf, u, ',');
    std::getline(uf, p, ',');
    if (uf >> d >> sep >> bal && !u.empty()) {
      register_account(u, p, d, bal, true);
    }
  }
}

std::string account_manager::accounts_file() const {
  return base_dir + "/accounts.csv";
}

std::string account_manager::user_dir(const std::string& username) const {
  return base_dir + "/users/" + username;
}

void account_manager::ensure_directories(const std::string& username) const {
  fs::create_directories(user_dir(username));
}

std::vector<account_record> account_manager::list_accounts() const {
  std::vector<account_record> list;
  std::ifstream in{accounts_file()};
  std::string line;
  while (std::getline(in, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line.empty() || line[0] == '#') continue;
    std::istringstream ss{line};
    account_record rec;
    char sep = ',';
    std::getline(ss, rec.username, ',');
    std::getline(ss, rec.password, ',');
    if (ss >> rec.start_date >> sep >> rec.initial_balance) {
      list.push_back(rec);
    }
  }
  return list;
}

bool account_manager::account_exists(const std::string& username) const {
  for (const auto& a : list_accounts()) {
    if (a.username == username) return true;
  }
  return false;
}

bool account_manager::register_account(const std::string& username, const std::string& password, const date& start_d, double initial_balance, bool seed_demo) {
  if (username.empty() || password.empty() || account_exists(username)) return false;

  ensure_directories(username);
  std::ofstream out{accounts_file(), std::ios::app};
  out << username << "," << password << "," << start_d << "," << initial_balance << "\n";

  // Actualizam si data/user.txt cu ultimul cont creat/activ
  {
    std::ofstream uout{base_dir + "/user.txt"};
    uout << username << "," << password << "," << start_d << "," << initial_balance << "\n";
  }

  std::string udir = user_dir(username);
  const std::vector<std::string> files = {
    "transactions.csv", "goals.csv", "tasks.csv", "events.csv", "roadmap.csv", "habits.csv", "workouts.csv"
  };

  for (const auto& fname : files) {
    std::string target = udir + "/" + fname;
    std::string template_path = base_dir + "/" + fname;
    if (seed_demo && fs::exists(template_path)) {
      fs::copy_file(template_path, target, fs::copy_options::overwrite_existing);
    } else if (!fs::exists(target)) {
      std::ofstream empty_f{target};
      if (fname == "roadmap.csv") {
        empty_f << "1,0,Obiectiv Principal,2.0,0\n2,1,Primul Pas,1.0,0\n";
      }
    }
  }
  return true;
}

bool account_manager::authenticate(const std::string& username, const std::string& password, account_record& out_rec) const {
  for (const auto& a : list_accounts()) {
    if (a.username == username && a.password == password) {
      out_rec = a;
      return true;
    }
  }
  return false;
}

lifetracker account_manager::load_user_app(const account_record& rec) const {
  std::string udir = user_dir(rec.username);
  if (!fs::exists(udir)) {
    udir = base_dir;
  }

  auto txs = load_all<transaction>(udir + "/transactions.csv");
  auto goals = load_all<money_goal>(udir + "/goals.csv");
  auto tasks = load_all<task>(udir + "/tasks.csv");
  auto events = load_all<calendar_event>(udir + "/events.csv");
  auto nodes = load_all<roadmap_node>(udir + "/roadmap.csv");
  auto habits = load_all<habit>(udir + "/habits.csv");
  auto workouts = load_all<workout_session>(udir + "/workouts.csv");

  if (nodes.empty()) {
    nodes.push_back(roadmap_node{1, 0, "Obiectiv Principal", 2.0, false});
  }

  budget b{rec.initial_balance, txs, goals};
  task_tracker tt{tasks};
  calendar cal{rec.username + "@gmail.com", events};
  roadmap rm{"Launch Life Tracker", nodes};
  habit_tracker ht{habits};
  fitness_tracker ft{2500, workouts};

  return lifetracker{rec.username, rec.password, rec.start_date, b, tt, cal, rm, ht, ft};
}

void account_manager::save_user_app(const lifetracker& app, double initial_balance) const {
  std::string udir = user_dir(app.get_user());
  ensure_directories(app.get_user());

  // 1. Actualizare data curenta in accounts.csv
  auto all = list_accounts();
  {
    std::ofstream out{accounts_file()};
    for (auto& a : all) {
      if (a.username == app.get_user()) {
        a.start_date = app.get_date();
        a.initial_balance = initial_balance;
      }
      out << a.username << "," << a.password << "," << a.start_date << "," << a.initial_balance << "\n";
    }
  }

  // Sincronizam si in data/user.txt
  {
    std::ofstream uout{base_dir + "/user.txt"};
    uout << app.get_user() << "," << app.get_password() << "," << app.get_date() << "," << initial_balance << "\n";
  }

  // 2. Salvare transactions.csv
  {
    std::ofstream out{udir + "/transactions.csv"};
    out << "# title,category,d.m.y,amount,membership,win\n";
    for (const auto& t : app.budget_ref().get_transactions()) {
      out << t.get_title() << "," << t.category_name() << "," << t.get_date() << ","
          << t.raw_amount() << "," << (t.is_membership() ? 1 : 0) << "," << (t.is_win() ? 1 : 0) << "\n";
    }
  }

  // 3. Salvare goals.csv
  {
    std::ofstream out{udir + "/goals.csv"};
    out << "# title,money,d.m.y\n";
    for (const auto& g : app.budget_ref().get_goals()) {
      out << g.get_title() << "," << g.get_money() << "," << g.get_deadline() << "\n";
    }
  }

  // 4. Salvare tasks.csv
  {
    std::ofstream out{udir + "/tasks.csv"};
    out << "# title,description,urgency,d.m.y,progress\n";
    for (const auto& t : app.tasks_ref().get_tasks()) {
      out << t.get_title() << "," << t.get_description() << "," << t.get_urgency() << ","
          << t.get_deadline() << "," << (t.is_done() ? 100.0 : t.get_progress()) << "\n";
    }
  }

  // 5. Salvare events.csv
  {
    std::ofstream out{udir + "/events.csv"};
    out << "# title,d.m.y,start_hour,duration_min,synced\n";
    for (const auto& e : app.calendar_ref().get_events()) {
      out << e.get_title() << "," << e.get_date() << "," << e.get_start_hour() << ","
          << e.get_duration() << "," << (e.is_synced() ? 1 : 0) << "\n";
    }
  }

  // 6. Salvare roadmap.csv
  {
    std::ofstream out{udir + "/roadmap.csv"};
    out << "# id,parent_id,title,weight,done\n";
    for (const auto& n : app.roadmap_ref().get_nodes()) {
      out << n.get_id() << "," << n.get_parent() << "," << n.get_title() << ","
          << n.get_weight() << "," << (n.is_done() ? 1 : 0) << "\n";
    }
  }

  // 7. Salvare habits.csv
  {
    std::ofstream out{udir + "/habits.csv"};
    out << "# title,category,target_days_per_week,current_streak,best_streak,last_completed\n";
    for (const auto& h : app.habits_ref().get_habits()) {
      out << h.get_title() << "," << h.get_category() << "," << h.get_target_days() << ","
          << h.get_streak() << "," << h.get_best_streak() << "," << h.get_last_completed() << "\n";
    }
  }

  // 8. Salvare workouts.csv
  {
    std::ofstream out{udir + "/workouts.csv"};
    out << "# title,sport,duration_min,calories,intensity,d.m.y\n";
    for (const auto& w : app.fitness_ref().get_sessions()) {
      out << w.get_title() << "," << w.get_sport() << "," << w.get_duration() << ","
          << w.get_calories() << "," << w.get_intensity() << "," << w.get_date() << "\n";
    }
  }
}
