#include "replace.h"

#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

unordered_map<string, string> map_params = {
    {"AR", "ApproachRate"},
    {"OD", "OverallDifficulty"},
    {"CS", "CircleSize"} };

string prev_file_path;
vector<string> prev_file_contents;

void read_file(string filename, vector<string>& lines)
{
    ifstream file(filename);
    if (file.is_open())
    {
        string line;
        while (getline(file, line))
        {
            lines.push_back(line);
        }
        file.close();
    }
}

template <class Iter>
string join(Iter begin, Iter end, string const& separator)
{
    ostringstream result;
    if (begin != end)
        result << *begin++;
    while (begin != end)
        result << separator << *begin++;
    return result.str();
}

void split(string s, string delimiter, vector<string>& out)
{
    size_t pos = 0;
    while ((pos = s.find(delimiter)) != string::npos)
    {
        out.push_back(s.substr(0, pos));
        s.erase(0, pos + delimiter.length());
    }
    if (!s.empty())
    {
        out.push_back(s);
    }
}

void revert()
{
    if (prev_file_contents.empty())
    {
        return;
    }
    ofstream song;
    song.open(prev_file_path);
    song << join(prev_file_contents.begin(), prev_file_contents.end(), "\n");
    song.close();
}

void replace(const string& osu_path, const unordered_map<string, string>& edit_params, const string& title, const string& diff)
{
    revert();
    for (const auto& folder : filesystem::directory_iterator(osu_path))
    {
        string folder_str = folder.path().stem().string();
        if (folder_str.find(title) != string::npos)
        {
            string song_folder = osu_path + folder_str;
            for (const auto& file : filesystem::directory_iterator(song_folder))
            {
                string file_str = file.path().stem().string();
                if (file_str.find(diff) != string::npos)
                {
                    vector<string> lines;
                    read_file(file.path().string(), lines);
                    prev_file_contents = lines;
                    for (auto& line : lines)
                    {
                        for (const auto& p : edit_params)
                        {
                            const string& param_name = map_params[p.first];
                            const string& param_val = p.second;
                            if (line.rfind(param_name, 0) == 0)
                            {
                                vector<string> param_line_parts;
                                split(line, ":", param_line_parts);
                                line = param_name + ":" + param_val;
                            }
                        }
                    }
                    string file_path = file.path().string();
                    prev_file_path = file_path;
                    if (prev_file_contents == lines)
                    {
                        return;
                    }
                    ofstream song;
                    song.open(file_path);
                    song << join(lines.begin(), lines.end(), "\n");
                    song.close();
                    return;
                }
            }
        }
    }
}