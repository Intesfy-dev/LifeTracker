#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Template-urile trebuie sa stea in header (compilatorul are nevoie de corpul lor in fiecare .cpp care le foloseste).

template<typename T>
bool parse_line(const std::string& line, T& item){
  std::istringstream row{line};
  return static_cast<bool>(row >> item);
}

template<typename T>
std::vector<T> load_all(const std::string& path){
  std::vector<T> items;
  std::ifstream file{path};
  if(!file) std::cerr << "Cannot open " << path << "\n";
  std::string line;
  while(std::getline(file, line)){
    if(!line.empty() && line.back() == '\r') line.pop_back();
    if(line.empty() || line[0] == '#') continue;
    T item;
    if(parse_line(line, item)) items.push_back(item);
    else std::cerr << path << ": skipped \"" << line << "\"\n";
  }
  return items;
}

template<typename T>
bool read_item(const std::string& prompt, T& item){
  std::string line;
  std::cout << prompt;
  std::getline(std::cin >> std::ws, line);
  if(!line.empty() && line.back() == '\r') line.pop_back();
  std::cout << "\n";
  if(parse_line(line, item)) return true;
  std::cout << "Invalid input\n";
  return false;
}
