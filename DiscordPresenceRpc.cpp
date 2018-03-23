#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DiscordPresenceRpc.h"

DiscordPresenceRpc::~DiscordPresenceRpc()
{
    Discord_Shutdown();
}


static void handleDiscordSpectate(const char* secret)
{
    char command[80];
    strcpy(command, "xdg-open");
    strcat(command, secret);
    system(command);
}


void DiscordPresenceRpc::setApp(const char* app)
{
    DiscordEventHandlers h;
    h.spectateGame = handleDiscordSpectate;
    Discord_Shutdown();
    Discord_Initialize(app, &h, 1, 0);
}

void DiscordPresenceRpc::send(DiscordRichPresence& payload)
{
    Discord_UpdatePresence(&payload);
}