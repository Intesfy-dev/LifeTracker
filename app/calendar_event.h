#pragma once

#include <iostream>
#include <string>
#include "date.h"

class calendar_event{

  private:
    std::string title;
    date event_date;
    int start_hour, duration_min;
    bool synced;

    int start_minutes() const;
    int end_minutes() const;

  public:
    explicit calendar_event(const std::string& ti = "unspecified", const date& d = date{}, int sh = 9, int dur = 60, bool sy = false);

    const std::string& get_title() const { return title; }
    const date& get_date() const { return event_date; }
    int get_start_hour() const { return start_hour; }
    int get_duration() const { return duration_min; }
    bool is_synced() const { return synced; }

    void mark_synced();

    void reschedule(const date& d, int sh);

    bool overlaps(const calendar_event& o) const;

    void write_ics(std::ostream& os) const;

    friend std::istream& operator>>(std::istream& is, calendar_event& e);

    friend std::ostream& operator<<(std::ostream& os, const calendar_event& e);
};
