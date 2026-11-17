#include "calendar_event.h"

#include <algorithm>

int calendar_event::start_minutes() const {
  return start_hour * 60;
}

int calendar_event::end_minutes() const {
  return start_hour * 60 + duration_min;
}

calendar_event::calendar_event(const std::string& ti, const date& d, int sh, int dur, bool sy)
  : title{ti}, event_date{d}, start_hour{std::clamp(sh, 0, 23)},
    duration_min{std::max(15, dur)}, synced{sy} {}

void calendar_event::mark_synced() {
  synced = true;
}

void calendar_event::reschedule(const date& d, int sh) {
  event_date = d;
  start_hour = std::clamp(sh, 0, 23);
  synced = false;
}

bool calendar_event::overlaps(const calendar_event& o) const {
  if(!event_date.same_day(o.event_date)) return false;
  return start_minutes() < o.end_minutes() && o.start_minutes() < end_minutes();
}

void calendar_event::write_ics(std::ostream& os) const {
  os << "BEGIN:VEVENT\nSUMMARY:" << title << "\nDTSTART:" << event_date.ics_code() << "T" << (start_hour < 10 ? "0" : "") << start_hour << "0000\nDURATION:PT" << duration_min << "M\nEND:VEVENT\n";
}

std::istream& operator>>(std::istream& is, calendar_event& e) {
  char sep = ',';
  std::getline(is, e.title, ',');
  if(is >> e.event_date >> sep >> e.start_hour >> sep >> e.duration_min >> sep >> e.synced){
    e.start_hour = std::clamp(e.start_hour, 0, 23);
    e.duration_min = std::max(15, e.duration_min);
  }
  return is;
}

std::ostream& operator<<(std::ostream& os, const calendar_event& e) {
  return os << e.title << " | " << e.event_date << " at " << e.start_hour << ":00 (" << e.duration_min << "m) | " << (e.synced ? "google-synced" : "local-modified") << "\n";
}
