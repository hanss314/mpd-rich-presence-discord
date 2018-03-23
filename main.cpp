#include <string>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include "MpdClient.h"
#include "unistd.h"
// POSIX systems are the majority of systems both running and created so it's okay
#include "DiscordPresenceRpc.h"

static void setAppSend(const char* app, DiscordRichPresence& payload, DiscordPresenceRpc& rpc)
{
    rpc.setApp(app);
    rpc.send(payload);
}

static DiscordRichPresence getPresenceForTrack(const TrackInfo& track)
{
    DiscordRichPresence payload = {};
    
    payload.partySize = track.TrackNumber;
    payload.partyMax = track.TotalTracks;
    payload.state = track.Artist.c_str();
    payload.details = track.TrackName.c_str();
    payload.startTimestamp = time(0) - track.PlayTimeSeconds;
    payload.largeImageKey = "mpd_large";
    payload.largeImageText = "Music Player Daemon";
    if(track.Artist == "by YouTube"){
        payload.spectateSecret = track.Comment.c_str();
        payload.matchSecret = "test";
    }
    
    return payload;
}

void sendIdle(DiscordPresenceRpc& rpc)
{
    const char* appIdle = "408834798614872064";
    
    DiscordRichPresence p = {};
    p.details = "Idle";
    p.largeImageKey = "mpd_large";
    setAppSend(appIdle, p, rpc);
}

void updatePresence(MpdClient& mpd, DiscordPresenceRpc& rpc)
{
    
    const char* appPlaying = "408834798614872064";
    const char* appPaused = "408834798614872064";
    
    MpdClient::State state = mpd.getState();
    switch(state)
    {
        case MpdClient::Playing:
        case MpdClient::Paused:
        {
            auto track = mpd.getCurrentTrack();
            auto p = getPresenceForTrack(track);
            if(state == MpdClient::Paused)
            {
                p.startTimestamp = 0;
                p.smallImageKey = "paused_small";
                p.smallImageText = "Paused";
                setAppSend(appPaused, p, rpc);
                break;
            }
            p.smallImageKey = "playing_small";
            p.smallImageText = "Playing";
            setAppSend(appPlaying, p, rpc);
            break;
        }
        case MpdClient::Idle:
        {
            sendIdle(rpc);
            break;
        }
    }
}

std::string getParam(const std::vector<std::string>& args, const std::string& param)
{
    for(const auto& arg : args)
    {
        auto start = arg.find(param);
        if(start == std::string::npos)
            continue;
        
        return arg.substr(start + param.size() + 1);
    }
    
    return {};
}

std::string getHostname(const std::vector<std::string>& args)
{
    auto host = getParam(args, "-h");
    if(host.empty())
        return "127.0.0.1";
    return  host;
}

unsigned getPort(const std::vector<std::string>& args)
{
    auto rawPort = getParam(args, "-p");
    if(rawPort.empty())
        return 6600;
    return static_cast<unsigned int>(std::stoi(rawPort));
}

std::string getPassword(const std::vector<std::string>& args)
{
    return getParam(args, "-P");
}

int main(int argc, char** args)
{
    auto vecArgs = std::vector<std::string>(args+1, args+argc);
    auto host = getHostname(vecArgs);
    auto pass = getPassword(vecArgs);
    auto port = getPort(vecArgs);
    
    bool isForked = false;
    int pid;
    if(std::find(vecArgs.begin(), vecArgs.end(), "--fork") != vecArgs.end())
    {
        pid = fork();
        if (pid != 0)
        {
            if (pid < 0)
            {
                std::cerr << "Failed to fork." << std::endl;
                return -1;
            }
            else
            {
                std::cout << "Forked. PID: " << pid << std::endl;
                return 0;
            }
        }
        isForked = true;
    }
    
    DiscordPresenceRpc rpc;
    int count = 0;
    const static int MaxExceptionsWhenForked = 10;
    
    while(true)
    {
        try
        {
            MpdClient mpd(host, port);
            mpd.connect(pass);
            
            while(true)
            {
                updatePresence(mpd, rpc);
                mpd.waitForStateChange(pass);
            }
        }
        catch(std::runtime_error e)
        {
            sendIdle(rpc);
            std::cout << "Exception: " << e.what() << ". reconnecting to MPD in 5 seconds." << std::endl;
    
            if(isForked && count++ == MaxExceptionsWhenForked)
                return -1;
            
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
}
