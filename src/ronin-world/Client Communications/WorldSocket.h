/***
 * Demonstrike Core
 */

// Class WorldSocket - Main network code functions, handles
// reading/writing of all packets.

#pragma once

#pragma pack(PRAGMA_PACK)

struct ClientPktHeader
{
    uint16 size;
    uint32 cmd;
};

struct ServerPktHeader
{
    uint32 size;
    uint8 header[5];
    uint8 headerLength;

    void SetData(uint32 _size, uint16 _cmd)
    {
        size = _size;
        headerLength = 0;
        memset(&header, 0, sizeof(uint8)*5);

        if (size > 0x7FFF)
            header[headerLength++] = 0x80 | (0xFF & (_size >> 16));
        header[headerLength++] = 0xFF & (_size >> 8);
        header[headerLength++] = 0xFF & _size;
        header[headerLength++] = 0xFF & _cmd;
        header[headerLength++] = 0xFF & (_cmd >> 8);
    }

    uint8 getHeaderLength() { return headerLength; }
};

#pragma pack(PRAGMA_POP)

#define WORLDSOCKET_SENDBUF_SIZE 131078
#define WORLDSOCKET_RECVBUF_SIZE 16384

class WorldPacket;
class SocketHandler;
class WorldSession;

enum OUTPACKET_RESULT
{
    OUTPACKET_RESULT_SUCCESS = 1,
    OUTPACKET_RESULT_NO_ROOM_IN_BUFFER = 2,
    OUTPACKET_RESULT_NOT_CONNECTED = 3,
    OUTPACKET_RESULT_SOCKET_ERROR = 4,
    OUTPACKET_RESULT_PACKET_ERROR = 5
};

class SERVER_DECL WorldSocket : public TcpSocket
{
public:
    WorldSocket(SOCKET fd, const sockaddr_in * peer);
    ~WorldSocket();

    RONIN_INLINE void SendPacket(WorldPacket* packet)
    {
        if(packet == NULL)
            return;
        OutPacket(packet->GetOpcode(), packet->size(), (packet->size() ? (const void*)packet->contents() : NULL));
    }

    void __fastcall OutPacket(uint16 opcode, size_t len, const void* data, bool compressed = false);
    OUTPACKET_RESULT __fastcall _OutPacket(uint16 opcode, size_t len, const void* data, bool compressed = false);

    RONIN_INLINE uint32 GetLatency() { return _latency; }
    RONIN_INLINE bool isAuthed() { return m_authed; }

    void Authenticate();
    void InformationRetreiveCallback(WorldPacket & recvData, uint32 requestid);

    void SendAuthResponse(uint8 code, bool holdsPosition, uint32 position = 0);
    void SendAddonPacket(WorldSession *pSession);

    void OnRecvData();
    void OnConnect();
    void OnDisconnect();
    void UpdateQueuedPackets();

    RONIN_INLINE void SetSession(WorldSession * session) { mSession = session; }
    RONIN_INLINE WorldSession * GetSession() { return mSession; }

protected:
    void _HandleAuthSession(WorldPacket* recvPacket);
    void _HandlePing(WorldPacket* recvPacket);

private:
    uint32 mOpcode, mRemaining, mUnaltered;
    uint32 mSeed, mRequestID;
    uint8 hashDigest[20];
    uint32 mClientSeed;
    uint16 mClientBuild;
    WorldPacket *addonPacket;

    WorldSession *mSession;
    FastQueue<WorldPacket*, DummyLock> _queue;
    Mutex queueLock;

    WowCrypt _crypt;
    uint32 _latency;
    bool m_authed, mQueued, m_nagleEanbled, isBattleNetAccount;
    std::string * m_fullAccountName;

    // Packet recv and send headers
    ClientPktHeader _recvHeader;
    ServerPktHeader _sendHeader;
};
