#pragma once

#include <iostream>
#include <string>
#include "date.h"

class workout_session{

  private:
    std::string title, sport;
    int duration_min, calories, intensity;
    date session_date;

  public:
    explicit workout_session(const std::string& ti = "unspecified", const std::string& sp = "general", int dur = 45, int cal = 300, int in_lvl = 5, const date& sd = date{});

    const std::string& get_title() const { return title; }
    const std::string& get_sport() const { return sport; }
    int get_duration() const { return duration_min; }
    int get_calories() const { return calories; }
    int get_intensity() const { return intensity; }
    const date& get_date() const { return session_date; }

    double training_load() const;

    friend std::istream& operator>>(std::istream& is, workout_session& w);

    friend std::ostream& operator<<(std::ostream& os, const workout_session& w);
};
