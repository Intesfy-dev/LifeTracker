#include "roadmap_node.h"

#include <algorithm>

roadmap_node::roadmap_node(int i, int p, const std::string& ti, double w, bool d)
  : id{i}, parent_id{p}, title{ti}, weight{std::max(0.1, w)}, done{d} {}

void roadmap_node::finish() {
  done = true;
}

void roadmap_node::reopen() {
  done = false;
}

std::istream& operator>>(std::istream& is, roadmap_node& n) {
  char sep = ',';
  if(is >> n.id >> sep >> n.parent_id >> sep){
    std::getline(is, n.title, ',');
    is >> n.weight >> sep >> n.done;
    n.weight = std::max(0.1, n.weight);
  }
  return is;
}

std::ostream& operator<<(std::ostream& os, const roadmap_node& n) {
  return os << "[" << (n.done ? "x" : " ") << "] #" << n.id << " (parent " << n.parent_id << ") " << n.title << " | weight " << n.weight << "\n";
}
