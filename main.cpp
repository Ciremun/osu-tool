#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>

std::string appdata_local_path()
{
    TCHAR szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath)))
    {
        return std::string(szPath);
    }
    return std::string("");
}

#endif

using namespace std;

unordered_map<string, string> map_params = {
    {"AR", "ApproachRate"},
    {"OD", "OverallDifficulty"},
    {"CS", "CircleSize"}};

unordered_map<string, string> edit_params;

string command;
string osu_path;
string prev_file_path;
vector<string> prev_file_contents;

void read_file(string filename, vector<string> &lines)
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

template <typename Iter>
string join(Iter begin, Iter end, string const &separator)
{
    ostringstream result;
    if (begin != end)
        result << *begin++;
    while (begin != end)
        result << separator << *begin++;
    return result.str();
}

void split(string s, string delimiter, vector<string> &out)
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

void get_osu_path(int argc, char **argv)
{
    if (argc > 1)
    {
        osu_path = argv[1];
    }
    else
    {
#ifdef _WIN32
        osu_path = appdata_local_path() + "\\osu!";
#endif
    }
    while (!filesystem::exists(osu_path))
    {
        cout << "error: osu! path not found, try again" << endl;
        cout << "osu! path: ";
        getline(cin, osu_path);
        cout << endl;
    }
    osu_path += "\\Songs";
}

void revert()
{
    if (prev_file_contents.empty())
    {
        return;
    }
    const vector<string> &fc = prev_file_contents;
    ofstream song;
    song.open(prev_file_path);
    song << join(fc.begin(), fc.end(), "\n");
    song.close();
    cout << "revert " << prev_file_path << endl;
}

void next()
{
    edit_params.clear();
    cout << "next song: ";
    getline(cin, command);
    cout << endl;
    revert();
    vector<string> cmd_parts;
    vector<string> param_parts;
    split(command, " ", cmd_parts);
    if (cmd_parts.size() < 3)
    {
        cout << "error: insufficient command args" << endl;
        return;
    }
    for (size_t i = 2; i < cmd_parts.size(); i++)
    {
        param_parts.clear();
        string &param = cmd_parts[i];
        split(param, "=", param_parts);
        if (param_parts.size() != 2)
        {
            cout << "error: bad parameter " << param << endl;
            return;
        }
        string &param_short = param_parts[0];
        string &param_val = param_parts[1];
        float param_val_i = stof(param_val);
        if (!(0.1f <= param_val_i && param_val_i <= 10.0))
        {
            cout << "error: parameter " << param_short << "=" << param_val << " is out of range 0.1 - 10.0" << endl;
            return;
        }
        transform(param_short.begin(), param_short.end(), param_short.begin(), ::toupper);
        if (!map_params.contains(param_short))
        {
            cout << "error: unknown map parameter: " << param_short << endl;
            return;
        }
        string &param_long = map_params[param_short];
        edit_params[param_long] = param_val;
    }
    string song_folder;
    string folder_str;
    string file_str;
    string &title_query = cmd_parts[0];
    string &diff_query = cmd_parts[1];
    transform(title_query.begin(), title_query.end(), title_query.begin(), ::tolower);
    transform(diff_query.begin(), diff_query.end(), diff_query.begin(), ::tolower);
    for (const auto &folder : filesystem::directory_iterator(osu_path))
    {
        folder_str = folder.path().stem().string();
        transform(folder_str.begin(), folder_str.end(), folder_str.begin(), ::tolower);
        if (folder_str.find(title_query) != string::npos)
        {
            song_folder = osu_path + "\\" + folder_str;
            for (const auto &file : filesystem::directory_iterator(song_folder))
            {
                file_str = file.path().stem().string();
                transform(file_str.begin(), file_str.end(), file_str.begin(), ::tolower);
                if (file_str.find(diff_query) != string::npos)
                {
                    vector<string> lines;
                    read_file(file.path().string(), lines);
                    string file_path = file.path().string();
                    prev_file_path = file_path;
                    prev_file_contents = lines;
                    for (auto &line : lines)
                    {
                        for (const auto &p : edit_params)
                        {
                            const string &param_name = p.first;
                            const string &param_val = p.second;
                            if (line.rfind(param_name, 0) == 0)
                            {
                                vector<string> param_line_parts;
                                split(line, ":", param_line_parts);
                                line = param_name + ":" + param_val;
                            }
                        }
                    }
                    if (prev_file_contents == lines)
                    {
                        cout << "nothing to replace" << endl;
                        return;
                    }
                    ofstream song;
                    song.open(file_path);
                    song << join(lines.begin(), lines.end(), "\n");
                    song.close();
                    cout << "modified file " << file_str << endl;
                    return;
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    get_osu_path(argc, argv);
    while (true)
    {
        next();
    }
}
