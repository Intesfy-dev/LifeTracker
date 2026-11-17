#pragma once

#include <iostream>
#include <string>

class roadmap_node{

  private:
    int id, parent_id;
    std::string title;
    double weight;
    bool done;

  public:
    explicit roadmap_node(int i = 1, int p = 0, const std::string& ti = "unspecified", double w = 1.0, bool d = false);

    int get_id() const { return id; }
    int get_parent() const { return parent_id; }
    const std::string& get_title() const { return title; }
    double get_weight() const { return weight; }
    bool is_done() const { return done; }
    void finish();
    void reopen();

    friend std::istream& operator>>(std::istream& is, roadmap_node& n);

    friend std::ostream& operator<<(std::ostream& os, const roadmap_node& n);
};
