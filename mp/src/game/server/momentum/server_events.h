#ifndef SERVER_EVENTS_H
#define SERVER_EVENTS_H
#ifdef _WIN32
#pragma once
#endif

namespace Momentum {

void OnServerDLLInit();
void GameInit();
} // namespace Momentum

#endif // SERVER_EVENTS_H