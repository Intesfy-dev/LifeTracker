#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "date.h"
#include "task.h"

class task_tracker{

  private:
    int total, done, tbd, missed;
    std::vector<task> tasks;

    static bool higher(const task& a, const task& b);

    void recount();

  public:
    explicit task_tracker(const std::vector<task>& ta = {});

    int get_total() const { return total; }
    int get_done() const { return done; }
    int get_tbd() const { return tbd; }
    int get_missed() const { return missed; }
    const std::vector<task>& get_tasks() const { return tasks; }
    void set_tasks(const std::vector<task>& ta) { tasks = ta; recount(); }

    void add_task(const task& t);

    bool reopen(const std::string& task_title, const date& today);

    bool remove_task(const std::string& task_title, const date& deadline);

    bool complete(const std::string& task_title);

    void update_missed(const date& today);

    int escalate_due_soon(const date& today, int threshold_days);

    void focus_plan(const date& today, int max_items) const;

    double completion() const;

    void rank_level(int level);

    std::vector<task> prioritize() const;

    friend std::ostream& operator<<(std::ostream& os, const task_tracker& tt);
};
