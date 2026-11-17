#pragma once

#include <iostream>
#include <string>
#include "date.h"

class task{

  private:
    std::string title, description;
    int urgency;
    date task_deadline;
    double progress, prev_progress{0};
    bool done, missed;

    static constexpr int max_level = 3;

    int days_left(const date& today) const;

  public:
    static int levels(){ return max_level; }

    explicit task(const std::string& ti = "unspecified", const std::string& desc = "unspecified", int u = 3, const date& t_d = date{}, double p = 0, bool d = false, bool m = false);

    const std::string& get_title() const { return title; }
    const std::string& get_description() const { return description; }
    const date& get_deadline() const { return task_deadline; }
    int get_urgency() const { return urgency; }
    double get_progress() const { return progress; }
    bool is_done() const { return done; }
    bool is_missed() const { return missed; }
    bool has_level(int l) const;
    int deadline_diff(const task& o) const;

    double urgency_score(const date& today) const;

    void finish();

    void reopen();

    void set_progress(double p);

    void postpone(int extra_days);

    void escalate();

    void check_missed(const date& today);

    bool more_urgent(const task& o) const;

    friend std::istream& operator>>(std::istream& is, task& t);

    friend std::ostream& operator<<(std::ostream& os, const task& t);
};
