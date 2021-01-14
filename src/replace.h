#pragma once
#include <string>
#include <vector>
#include <unordered_map>

void read_file(std::string filename, std::vector<std::string>& lines);

template <typename Iter>
std::string join(Iter begin, Iter end, std::string const& separator);

void split(std::string s, std::string delimiter, std::vector<std::string>& out);
void revert();
void replace(const std::string& osu_path, const std::unordered_map<std::string, std::string>& edit_params, const std::string& title, const std::string& diff);