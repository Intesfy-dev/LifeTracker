#pragma once

#include <iostream>
#include <string>
#include "date.h"
#include "transaction.h"
#include "task.h"
#include "budget.h"
#include "task_tracker.h"
#include "calendar_event.h"
#include "calendar.h"
#include "roadmap.h"
#include "habit.h"
#include "habit_tracker.h"
#include "workout_session.h"
#include "fitness_tracker.h"

class lifetracker{

  private:
    std::string User, Password;
    date current_date;
    budget user_budget;
    task_tracker user_tasks;
    calendar user_calendar;
    roadmap user_roadmap;
    habit_tracker user_habits;
    fitness_tracker user_fitness;

  public:
    explicit lifetracker(const std::string& user = "JhonDoe", const std::string& pass = "password1234", const date& c_d = date{}, const budget& u_b = budget{}, const task_tracker& u_t = task_tracker{}, const calendar& u_c = calendar{}, const roadmap& u_r = roadmap{}, const habit_tracker& u_h = habit_tracker{}, const fitness_tracker& u_f = fitness_tracker{});

    bool login(const std::string& u, const std::string& p) const;

    const std::string& get_user() const { return User; }
    const std::string& get_password() const { return Password; }
    const date& get_date() const { return current_date; }

    budget& budget_ref() { return user_budget; }
    const budget& budget_ref() const { return user_budget; }

    task_tracker& tasks_ref() { return user_tasks; }
    const task_tracker& tasks_ref() const { return user_tasks; }

    calendar& calendar_ref() { return user_calendar; }
    const calendar& calendar_ref() const { return user_calendar; }

    roadmap& roadmap_ref() { return user_roadmap; }
    const roadmap& roadmap_ref() const { return user_roadmap; }

    habit_tracker& habits_ref() { return user_habits; }
    const habit_tracker& habits_ref() const { return user_habits; }

    fitness_tracker& fitness_ref() { return user_fitness; }
    const fitness_tracker& fitness_ref() const { return user_fitness; }

    void add_transaction(const transaction& t);
    void add_task(const task& t);
    void add_event(const calendar_event& e);
    void add_habit(const habit& h);
    void add_workout(const workout_session& w);

    bool check_in_habit(const std::string& hab_title);

    bool undo_check_in_habit(const std::string& hab_title);

    int sync_deadlines(const std::string& ics_path);

    double life_score() const;

    void next_day();

    void summary() const;

    friend std::ostream& operator<<(std::ostream& os, const lifetracker& l);
};
