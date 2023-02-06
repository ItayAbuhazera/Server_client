#include "../include/event.h"
#include "../include/json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
using json = nlohmann::json;

Event::Event(std::string team_a_name, std::string team_b_name, std::string name, int time,
             std::map<std::string, std::string> game_updates, std::map<std::string, std::string> team_a_updates,
             std::map<std::string, std::string> team_b_updates, std::string discription)
    : team_a_name(team_a_name), team_b_name(team_b_name), name(name),
      time(time), game_updates(game_updates), team_a_updates(team_a_updates),
      team_b_updates(team_b_updates), description(discription)
{
}

Event::Event(): team_a_name(""), team_b_name(""), name(""), time(0), game_updates(), team_a_updates(), team_b_updates(), description("") {}

Event::~Event()
{
}

const std::string &Event::get_team_a_name() const
{
    return this->team_a_name;
}

const std::string &Event::get_team_b_name() const
{
    return this->team_b_name;
}

const std::string &Event::get_name() const
{
    return this->name;
}

int Event::get_time() const
{
    return this->time;
}

const std::map<std::string, std::string> &Event::get_game_updates() const
{
    return this->game_updates;
}

const std::map<std::string, std::string> &Event::get_team_a_updates() const
{
    return this->team_a_updates;
}

const std::map<std::string, std::string> &Event::get_team_b_updates() const
{
    return this->team_b_updates;
}

const std::string &Event::get_discription() const
{
    return this->description;
}

Event::Event(const std::string &frame_body) : team_a_name(""), team_b_name(""), name(""), time(0), game_updates(), team_a_updates(), team_b_updates(), description("")
{
    std::string msg(frame_body);
    std::string temp;
    int idx, nl, seg1, seg2;
    msg = msg.substr(msg.find("team a: "));

    idx = msg.find("team a: ") + 8;
    nl = msg.find('\n');
    team_a_name = msg.substr(idx, nl - idx);
    msg = msg.substr(nl + 1);

    idx = msg.find("team b: ") + 8;
    nl = msg.find('\n');
    team_b_name = msg.substr(idx, nl - idx);
    msg = msg.substr(nl + 1);

    idx = msg.find("event name: ") + 12;
    nl = msg.find('\n');
    name = msg.substr(idx, nl - idx);
    msg = msg.substr(nl + 1);

    idx = msg.find("time: ") + 6;
    nl = msg.find('\n');
    time = stoi(msg.substr(idx, nl - idx));
    msg = msg.substr(nl + 1);

    seg1 = msg.find("general game updates:");
    msg = msg.substr(seg1 + 22);
    seg2 = msg.find("team a updates:");
    nl = msg.find('\n');
    while(nl < seg2){
        idx = msg.find(": ") + 2;
        game_updates[msg.substr(0, idx - 2)] = msg.substr(idx, nl - idx);
        msg = msg.substr(nl + 1);
        seg2 = msg.find("team a updates:");
        nl = msg.find("\n");
    }

    seg1 = msg.find("team a updates:");
    msg = msg.substr(seg1 + 16);
    seg2 = msg.find("team b updates:");
    nl = msg.find('\n');
    while(nl < seg2){
        idx = msg.find(": ") + 2;
        team_a_updates[msg.substr(0, idx - 2)] = msg.substr(idx, nl - idx);
        msg = msg.substr(nl + 1);
        seg2 = msg.find("team b updates:");
        nl = msg.find("\n");
    }

    seg1 = msg.find("team b updates:");;
    msg = msg.substr(seg1 + 16);
    seg2 = msg.find("description:");
    nl = msg.find('\n');
    while(nl < seg2){
        idx = msg.find(": ") + 2;
        team_b_updates[msg.substr(0, idx - 2)] = msg.substr(idx, nl - idx);
        msg = msg.substr(nl + 1);
        seg2 = msg.find("description:");
        nl = msg.find("\n");
    }

    idx = msg.find("description:") + 14;
    description = msg.substr(idx);
}

names_and_events parseEventsFile(std::string json_path)
{
    std::ifstream f(json_path);
    json data = json::parse(f);

    std::string team_a_name = data["team a"];
    std::string team_b_name = data["team b"];

    // run over all the events and convert them to Event objects
    std::vector<Event> events;
    for (auto &event : data["events"])
    {
        std::string name = event["event name"];
        int time = event["time"];
        std::string description = event["description"];
        std::map<std::string, std::string> game_updates;
        std::map<std::string, std::string> team_a_updates;
        std::map<std::string, std::string> team_b_updates;
        for (auto &update : event["general game updates"].items())
        {
            if (update.value().is_string())
                game_updates[update.key()] = update.value();
            else
                game_updates[update.key()] = update.value().dump();
        }

        for (auto &update : event["team a updates"].items())
        {
            if (update.value().is_string())
                team_a_updates[update.key()] = update.value();
            else
                team_a_updates[update.key()] = update.value().dump();
        }

        for (auto &update : event["team b updates"].items())
        {
            if (update.value().is_string())
                team_b_updates[update.key()] = update.value();
            else
                team_b_updates[update.key()] = update.value().dump();
        }
        
        events.push_back(Event(team_a_name, team_b_name, name, time, game_updates, team_a_updates, team_b_updates, description));
    }
    names_and_events events_and_names{team_a_name, team_b_name, events};

    return events_and_names;
}

void Event::printEvent() const{
    std::cout << std::endl;
    std::cout << "team a name: " << team_a_name << std::endl;
    std::cout << "team b name: " << team_b_name << std::endl;
    std::cout << "event name: " << name << std::endl;
    std::cout << "time: " << time << std::endl;

    std::cout << std::endl << "general game updates: " << std::endl;
    for (auto const &pair: game_updates)
        std::cout << pair.first << ":" << pair.second << std::endl;

    std::cout << std::endl << "team a updates:" << std::endl;
    for (auto const &pair: team_a_updates)
        std::cout << pair.first << ":" << pair.second << std::endl;

    std::cout << std::endl << "team b updates:" << std::endl;
    for (auto const &pair: team_b_updates)
        std::cout << pair.first << ":" << pair.second << std::endl;

    std::cout << std::endl << "description:" << std::endl;
    std::cout << description << std::endl;
}