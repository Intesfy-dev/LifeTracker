#include "roadmap.h"

#include <algorithm>

std::vector<std::size_t> roadmap::children_of(int pid) const {
  std::vector<std::size_t> ch;
  for(std::size_t i = 0; i < nodes.size(); i++)
    if(nodes[i].get_parent() == pid) ch.push_back(i);
    return ch;
}

double roadmap::subtree_progress(std::size_t idx) const {
  std::vector<std::size_t> order;
  order.push_back(idx);
  for(std::size_t k = 0; k < order.size(); k++)
    for(std::size_t c : children_of(nodes[order[k]].get_id()))
      order.push_back(c);
  std::vector<double> prog(nodes.size(), 0.0);
  for(std::size_t k = order.size(); k > 0; k--){
    std::size_t u = order[k - 1];
    std::vector<std::size_t> ch = children_of(nodes[u].get_id());
    if(ch.empty()) prog[u] = nodes[u].is_done() ? 100.0 : 0.0;
    else{
      double sum_w = 0, done_w = 0;
      for(std::size_t c : ch){
        double w = nodes[c].get_weight();
        sum_w += w;
        done_w += w * (prog[c] / 100.0);
      }
      prog[u] = sum_w > 0 ? (done_w / sum_w) * 100.0 : 0.0;
    }
  }
  return prog[idx];
}

double roadmap::subtree_effort(std::size_t idx) const {
  std::vector<std::size_t> queue;
  queue.push_back(idx);
  double rem = 0;
  for(std::size_t k = 0; k < queue.size(); k++){
    std::size_t u = queue[k];
    std::vector<std::size_t> ch = children_of(nodes[u].get_id());
    if(ch.empty()){
      if(!nodes[u].is_done()) rem += nodes[u].get_weight();
    }else{
      for(std::size_t c : ch) queue.push_back(c);
    }
  }
  return rem;
}

void roadmap::refresh_parents() {
  // Un nod cu copii este "gata" doar daca tot sub-arborele lui e gata (si invers):
  // asa corectiile (redeschidere / stergere) se propaga corect spre radacina.
  for(std::size_t pass = 0; pass < nodes.size(); pass++){
    bool changed = false;
    for(std::size_t i = 0; i < nodes.size(); i++){
      std::vector<std::size_t> ch = children_of(nodes[i].get_id());
      if(ch.empty()) continue;
      bool all_done = subtree_progress(i) >= 99.9;
      if(all_done && !nodes[i].is_done()){ nodes[i].finish(); changed = true; }
      else if(!all_done && nodes[i].is_done()){ nodes[i].reopen(); changed = true; }
    }
    if(!changed) break;
  }
}

std::vector<std::size_t> roadmap::subtree_indices(int id) const {
  std::vector<std::size_t> order;
  for(std::size_t i = 0; i < nodes.size(); i++)
    if(nodes[i].get_id() == id){ order.push_back(i); break; }
  for(std::size_t k = 0; k < order.size(); k++)
    for(std::size_t c : children_of(nodes[order[k]].get_id()))
      order.push_back(c);
  return order;
}

std::string roadmap::find_leaf(int start_pid) const {
  std::vector<std::size_t> st;
  std::vector<std::size_t> roots = children_of(start_pid);
  for(std::size_t i = roots.size(); i > 0; i--) st.push_back(roots[i - 1]);
  while(!st.empty()){
    std::size_t u = st.back();
    st.pop_back();
    if(nodes[u].is_done()) continue;
    std::vector<std::size_t> ch = children_of(nodes[u].get_id());
    if(ch.empty()) return nodes[u].get_title();
    for(std::size_t i = ch.size(); i > 0; i--) st.push_back(ch[i - 1]);
  }
  return "";
}

void roadmap::build_critical_path(int start_pid, std::vector<std::string>& path) const {
  int cur_pid = start_pid;
  for(std::size_t step = 0; step <= nodes.size(); step++){
    std::vector<std::size_t> ch = children_of(cur_pid);
    double best_effort = -1.0;
    std::size_t best_idx = 0;
    for(std::size_t c : ch){
      if(nodes[c].is_done()) continue;
      double eff = subtree_effort(c);
      if(eff > best_effort){
        best_effort = eff;
        best_idx = c;
      }
    }
    if(best_effort <= 0) break;
    path.push_back(nodes[best_idx].get_title());
    cur_pid = nodes[best_idx].get_id();
  }
}

void roadmap::print_branch(std::ostream& os, std::size_t root_idx, const std::string& base_prefix, bool root_last) const {
  struct frame {
    std::size_t idx{0};
    std::string prefix{};
    bool is_last{false};
  };
  std::vector<frame> st;
  st.push_back(frame{root_idx, base_prefix, root_last});
  while(!st.empty()){
    frame cur = st.back();
    st.pop_back();
    os << cur.prefix << (cur.is_last ? "\\-- " : "|-- ") << "[" << (nodes[cur.idx].is_done() ? "x" : " ") << "] " << nodes[cur.idx].get_title() << " (" << subtree_progress(cur.idx) << "%, rem weight " << subtree_effort(cur.idx) << ")\n";
    std::vector<std::size_t> ch = children_of(nodes[cur.idx].get_id());
    std::string next_prefix = cur.prefix + (cur.is_last ? "    " : "|   ");
    for(std::size_t i = ch.size(); i > 0; i--)
      st.push_back(frame{ch[i - 1], next_prefix, i == ch.size()});
  }
}

roadmap::roadmap(const std::string& mg, const std::vector<roadmap_node>& no)
: main_goal{mg}, nodes{no} {
  refresh_parents();
}

bool roadmap::split_problem(const std::string& parent_title, const std::string& sub_title, double sub_weight) {
  if(sub_title.empty()) return false;
  if(parent_title.empty() || parent_title == "ROOT" || parent_title.find("[Arbore Nou") != std::string::npos) {
    return add_root_tree(sub_title, sub_weight);
  }
  int pid = 0, max_id = 0;
  for(const auto& n : nodes){
    max_id = std::max(max_id, n.get_id());
    if(n.get_title() == parent_title) pid = n.get_id();
  }
  if(pid == 0 && !nodes.empty()) return false;
  nodes.push_back(roadmap_node{max_id + 1, pid, sub_title, sub_weight, false});
  refresh_parents();
  return true;
}

bool roadmap::add_root_tree(const std::string& root_title, double root_weight) {
  if(root_title.empty()) return false;
  int max_id = 0;
  for(const auto& n : nodes){
    max_id = std::max(max_id, n.get_id());
    if(n.get_title() == root_title) return false;
  }
  nodes.push_back(roadmap_node{max_id + 1, 0, root_title, root_weight, false});
  return true;
}

bool roadmap::complete_node(int id) {
  std::vector<std::size_t> sub = subtree_indices(id);
  if(sub.empty()) return false;
  bool any = false;
  for(std::size_t i : sub)
    if(!nodes[i].is_done()){ nodes[i].finish(); any = true; }
  refresh_parents();
  return any;
}

bool roadmap::complete_step(const std::string& step_title) {
  for(const auto& n : nodes)
    if(n.get_title() == step_title && !n.is_done())
      return complete_node(n.get_id());
  return false;
}

bool roadmap::reopen_node(int id) {
  std::vector<std::size_t> sub = subtree_indices(id);
  if(sub.empty()) return false;
  bool any = false;
  for(std::size_t i : sub)
    if(nodes[i].is_done()){ nodes[i].reopen(); any = true; }
  refresh_parents();
  return any;
}

bool roadmap::reopen_step(const std::string& step_title) {
  for(const auto& n : nodes)
    if(n.get_title() == step_title && n.is_done())
      return reopen_node(n.get_id());
  return false;
}

int roadmap::subtree_count(int id) const {
  return static_cast<int>(subtree_indices(id).size());
}

int roadmap::remove_node(int id) {
  std::vector<std::size_t> sub = subtree_indices(id);
  if(sub.empty()) return 0;
  std::vector<bool> kill(nodes.size(), false);
  for(std::size_t i : sub) kill[i] = true;
  std::vector<roadmap_node> keep;
  for(std::size_t i = 0; i < nodes.size(); i++)
    if(!kill[i]) keep.push_back(nodes[i]);
  nodes = keep;
  refresh_parents();
  return static_cast<int>(sub.size());
}

double roadmap::node_progress(int node_id) const {
  for(std::size_t i = 0; i < nodes.size(); i++)
    if(nodes[i].get_id() == node_id) return subtree_progress(i);
    return 0.0;
}

double roadmap::node_remaining_effort(int node_id) const {
  for(std::size_t i = 0; i < nodes.size(); i++)
    if(nodes[i].get_id() == node_id) return subtree_effort(i);
    return 0.0;
}

double roadmap::overall_progress() const {
  std::vector<std::size_t> roots = children_of(0);
  if(roots.empty()) return 0;
  double sum_w = 0, done_w = 0;
  for(std::size_t r : roots){
    double w = std::max(0.1, nodes[r].get_weight());
    sum_w += w;
    done_w += w * (subtree_progress(r) / 100.0);
  }
  return sum_w > 0 ? (done_w / sum_w) * 100.0 : 0.0;
}

std::string roadmap::next_step() const {
  std::string step = find_leaf(0);
  return step.empty() ? "All steps completed!" : step;
}

void roadmap::print_critical_path() const {
  std::vector<std::string> path;
  build_critical_path(0, path);
  std::cout << "Critical path (" << path.size() << " steps): ";
  for(std::size_t i = 0; i < path.size(); i++)
    std::cout << (i ? " -> " : "") << path[i];
  std::cout << "\n";
}

void roadmap::render_tree(std::ostream& os) const {
  os << "Roadmap [" << main_goal << "] (" << overall_progress() << "%):\n";
  std::vector<std::size_t> roots = children_of(0);
  for(std::size_t i = 0; i < roots.size(); i++)
    print_branch(os, roots[i], "  ", i + 1 == roots.size());
}

std::ostream& operator<<(std::ostream& os, const roadmap& rm) {
  rm.render_tree(os);
  for(const auto& n : rm.nodes) os << "  " << n;
  return os;
}
