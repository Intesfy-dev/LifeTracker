#pragma once

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include "roadmap_node.h"

class roadmap{

private:
  std::string main_goal;
  std::vector<roadmap_node> nodes;

  std::vector<std::size_t> children_of(int pid) const;

  double subtree_progress(std::size_t idx) const;

  double subtree_effort(std::size_t idx) const;

  void refresh_parents();

  std::vector<std::size_t> subtree_indices(int id) const;

  std::string find_leaf(int start_pid) const;

  void build_critical_path(int start_pid, std::vector<std::string>& path) const;

  void print_branch(std::ostream& os, std::size_t root_idx, const std::string& base_prefix, bool root_last) const;

public:
  explicit roadmap(const std::string& mg = "Main Goal", const std::vector<roadmap_node>& no = {});

  const std::vector<roadmap_node>& get_nodes() const { return nodes; }

  bool split_problem(const std::string& parent_title, const std::string& sub_title, double sub_weight);

  bool add_root_tree(const std::string& root_title, double root_weight = 2.0);

  bool complete_step(const std::string& step_title);

  bool complete_node(int id);

  bool reopen_step(const std::string& step_title);

  bool reopen_node(int id);

  int subtree_count(int id) const;

  int remove_node(int id);

  double node_progress(int node_id) const;

  double node_remaining_effort(int node_id) const;

  double overall_progress() const;

  std::string next_step() const;

  void print_critical_path() const;

  void render_tree(std::ostream& os = std::cout) const;

  friend std::ostream& operator<<(std::ostream& os, const roadmap& rm);
};
