cmake_minimum_required(VERSION 3.9)
project(mpd_discord_richpresence)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O2 -s -DNDEBUG")

include_directories(
        ${CMAKE_SOURCE_DIR}/lib/discord-rpc/build/include/)

set(SOURCE_FILES
        main.cpp
        DiscordPresenceRpc.cpp
        DiscordPresenceRpc.h
        MpdClient.cpp
        MpdClient.h
        TrackInfo.h)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/mpd-discord.service
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_executable(mpd_discord_richpresence ${SOURCE_FILES})

target_link_libraries(
        mpd_discord_richpresence
        pthread
        mpdclient
        ${CMAKE_SOURCE_DIR}/lib/discord-rpc/build/lib64/libdiscord-rpc.a)
