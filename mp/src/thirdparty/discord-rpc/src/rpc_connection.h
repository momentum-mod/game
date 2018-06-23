#pragma once

#include "connection.h"
#include "serialization.h"

// I took this from the buffer size libuv uses for named pipes; I suspect ours would usually be much
// smaller.
constexpr size_t MaxRpcFrameSize = 64 * 1024;

struct RpcConnection {
    enum class ErrorCode : int {
        Success = 0,
        PipeClosed = 1,
        ReadCorrupt = 2,
    };

    enum class Opcode : uint32_t {
        Handshake = 0,
        Frame = 1,
        Close = 2,
        Ping = 3,
        Pong = 4,
    };

    struct MessageFrameHeader {
        Opcode opcode;
        uint32_t length;
    };

    struct MessageFrame : public MessageFrameHeader {
        char message[MaxRpcFrameSize - sizeof(MessageFrameHeader)];
    };

    enum class State : uint32_t {
        Disconnected,
        SentHandshake,
        AwaitingResponse,
        Connected,
    };

    BaseConnection* connection;
    State state;
    void (*onConnect)(JsonDocument& message);
    void (*onDisconnect)(int errorCode, const char* message);
    char appId[64];
    int lastErrorCode;
    char lastErrorMessage[256];
    RpcConnection::MessageFrame sendFrame;

    static RpcConnection* Create(const char* applicationId);
    static void Destroy(RpcConnection*&);

    inline bool IsOpen() const { return state == State::Connected; }

    void Open();
    void Close();
    bool Write(const void* data, size_t length);
    bool Read(JsonDocument& message);

    RpcConnection() {
        connection = nullptr;
        state = State::Disconnected;
        onConnect = nullptr;
        onDisconnect = nullptr;
        appId[0] = '\0';
        lastErrorCode = 0;
        lastErrorMessage[0] = '\0';
    }
};
