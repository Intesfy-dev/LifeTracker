#include "lifetracker.h"

#include <vector>

lifetracker::lifetracker(const std::string& user, const std::string& pass, const date& c_d, const budget& u_b, const task_tracker& u_t, const calendar& u_c, const roadmap& u_r, const habit_tracker& u_h, const fitness_tracker& u_f)
  : User{user}, Password{pass}, current_date{c_d}, user_budget{u_b}, user_tasks{u_t},
    user_calendar{u_c}, user_roadmap{u_r}, user_habits{u_h}, user_fitness{u_f} {}

bool lifetracker::login(const std::string& u, const std::string& p) const {
  return u == User && p == Password;
}

void lifetracker::add_transaction(const transaction& t) {
  user_budget.add_transaction(t);
}

void lifetracker::add_task(const task& t) {
  user_tasks.add_task(t);
}

void lifetracker::add_event(const calendar_event& e) {
  user_calendar.add_event(e);
}

void lifetracker::add_habit(const habit& h) {
  user_habits.add_habit(h);
}

void lifetracker::add_workout(const workout_session& w) {
  user_fitness.add_session(w);
}

bool lifetracker::check_in_habit(const std::string& hab_title) {
  return user_habits.log_habit(hab_title, current_date);
}

bool lifetracker::undo_check_in_habit(const std::string& hab_title) {
  return user_habits.undo_log(hab_title, current_date);
}

int lifetracker::sync_deadlines(const std::string& ics_path) {
  for(const auto& t : user_tasks.prioritize())
    if(t.has_level(1)){
      int slot = user_calendar.find_free_slot(t.get_deadline(), 60);
      user_calendar.add_event(calendar_event{"Deadline: " + t.get_title(), t.get_deadline(), slot != -1 ? slot : 9, 60, false});
    }
  return user_calendar.sync_google(ics_path);
}

double lifetracker::life_score() const {
  double t_score = user_tasks.completion();
  double r_score = user_roadmap.overall_progress();
  double h_score = user_habits.average_consistency();
  double f_score = user_fitness.goal_completion(current_date);
  double b_score = user_budget.get_left() > 0 ? 100.0 : 0.0;
  return (t_score + r_score + h_score + f_score + b_score) / 5.0;
}

void lifetracker::next_day() {
  current_date = current_date.tomorrow();
  user_tasks.update_missed(current_date);
  user_tasks.escalate_due_soon(current_date, 2);
  user_habits.update_streaks(current_date);
}

void lifetracker::summary() const {
  std::cout << "=== " << User << ", " << current_date << " (" << current_date.weekday_name() << ", Q" << current_date.quarter() << ") ===\n";
  user_budget.period_report(current_date);
  user_budget.category_report(current_date);
  user_budget.spending_split(current_date);
  user_budget.subscriptions_status(current_date);
  user_budget.forecast_balance(3, current_date);
  user_budget.goals_status(current_date);
  std::cout << "Tasks done: " << user_tasks.completion() << "%\n";
  user_tasks.focus_plan(current_date, 3);
  std::vector<task> top = user_tasks.prioritize();
  if(!top.empty()) std::cout << "Next up: " << top.front();
  user_calendar.week_agenda(current_date);
  user_roadmap.render_tree();
  user_roadmap.print_critical_path();
  std::cout << "Next roadmap step: " << user_roadmap.next_step() << "\n";
  std::cout << user_habits;
  user_fitness.sport_breakdown();
  std::cout << "Weekly fitness load: " << user_fitness.weekly_load(current_date) << " (" << user_fitness.goal_completion(current_date) << "% of kcal goal)\n";
  std::cout << "Overall Life Score: " << life_score() << "/100\n";
}

std::ostream& operator<<(std::ostream& os, const lifetracker& l) {
  return os << "User: " << l.User << " | date " << l.current_date << "\n" << l.user_budget << l.user_tasks << l.user_calendar << l.user_roadmap << l.user_habits << l.user_fitness;
}
