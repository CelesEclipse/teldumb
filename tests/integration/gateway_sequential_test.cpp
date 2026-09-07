#include <bits/stdint-uintn.h>
#include <cstddef>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <future>

extern "C" {
    #include "gateway/core/command.h"
    #include "gateway/core/connection.h"
    #include "gateway/network/gwsocket.h"
}

static GWCommand_t test_get_cmd_enum(const std::string & cmd_str)
{
    if (cmd_str.rfind("PING", 0) == 0)  return PING;
    if (cmd_str.rfind("REGISTER", 0) == 0) return REGISTER;
    if (cmd_str.rfind("AUTH", 0) == 0) return AUTH;
    if (cmd_str.rfind("GET_STATUS", 0) == 0) return GET_STATUS;
    if (cmd_str.rfind("DISCONNECT", 0) == 0) return DISCONNECT;
    return (GWCommand_t)-1;
}

static std::string test_get_cmd_arg(const std::string& cmd_str) {
    size_t space_pos = cmd_str.find(' ');
    if (space_pos != std::string::npos) {
        return cmd_str.substr(space_pos + 1);
    }
    return "";
}

static std::string make_frame(const std::string & payload)
{
    uint32_t len = htonl(static_cast<uint32_t>(payload.size()));
    std::string frame;
    frame.append(reinterpret_cast<const char *>(&len), sizeof(len));
    frame.append(payload);
    return frame;
}

class ConnectionSequentialTest : public ::testing::Test
{
protected:
    GWCnt_State_t * cnt;
    GWCmd_t * cmd;
    uint16_t test_port;
    uint8_t tx_buf[1024];

    void SetUp() override
    {
        test_port = 8080;
    }

    void TearDown() override
    {

    }
};

TEST_F(ConnectionSequentialTest, ProtocolTest)
{
    signal(SIGPIPE, SIG_IGN);
    int listen_fd = gw_socket_create(test_port);
    ASSERT_GT(listen_fd, 0);

    /* phase 1: CONNECT -> PING -> REGISTER user123 -> PING -> AUTH wrong_user
                    -> [expected reject/disconnect]
        Use std::async to create a virtual terminal b to run command instead of
        manually opening another terminal
    */
    std::vector<std::string> Cmds_01 = {
        "PING",
        "REGISTER user123",
        "PING",
        "AUTH wrong_user"
    };

    {
        auto terminal_b = std::async(std::launch::async, [this, &Cmds_01](){
            int client_fd = gw_socket_connect("127.0.0.1", test_port);
            if (client_fd <= 0) return;
            for (const auto& cmd : Cmds_01) {
                send(client_fd, cmd.c_str(), cmd.length(), 0);
                std::this_thread::sleep_for(std::chrono::milliseconds(25));
            }
            gw_socket_close(client_fd);
        });

        int server_client_fd = gw_socket_accept(listen_fd);
        ASSERT_GT(server_client_fd, 0);

        GWCnt_State_t * cnt1 = gw_cntstate_alloc();
        GWCmd_t * cmd1 = gw_parse_alloc();
        int status_state = 0;

        for (size_t i = 0; i < Cmds_01.size(); ++i) {
            ssize_t bytes = gw_socket_recv(server_client_fd, tx_buf, sizeof(tx_buf));
            SCOPED_TRACE("Phase1: " + Cmds_01[i]);
            EXPECT_GT(bytes, 0);

            GWCommand_t c = test_get_cmd_enum(Cmds_01[i]);
            std::string a = test_get_cmd_arg(Cmds_01[i]);
            gw_parse_setcmd(cmd1, c);
            gw_parse_setarg(cmd1, a.c_str());

            int res = gw_cntstate_process(cnt1, cmd1, tx_buf, sizeof(tx_buf), &status_state);

            if (Cmds_01[i].rfind("AUTH", 0) == 0 && a == "wrong_user") {
                EXPECT_EQ(res, -1);
                EXPECT_EQ(status_state, -5);
            }
        }
        terminal_b.get();
        gw_parse_destroy(cmd1);
        gw_cntstate_destroy(cnt1);
        gw_socket_close(server_client_fd);   // tear down phase 1's connection
    }

    /* phase 2: CONNECT -> REGISTER user123 -> AUTH user123 -> PING -> GET_STATUS -> DISCONNECT
                    -> [expected reject/disconnect]
    */
    std::vector<std::string> Cmds_02 = {
        "REGISTER user123",
        "AUTH user123",
        "PING",
        "GET_STATUS",
        "DISCONNECT"
    };

    {
        auto terminal_b2 = std::async(std::launch::async, [this, &Cmds_02](){
            int client_fd = gw_socket_connect("127.0.0.1", test_port);
            if (client_fd <= 0) return;
            for (const auto& cmd : Cmds_02) {
                send(client_fd, cmd.c_str(), cmd.length(), 0);
                std::this_thread::sleep_for(std::chrono::milliseconds(25));
            }
            gw_socket_close(client_fd);
        });

        int server_client_fd2 = gw_socket_accept(listen_fd);
        ASSERT_GT(server_client_fd2, 0);

        GWCnt_State_t * cnt2 = gw_cntstate_alloc();
        GWCmd_t * cmd2 = gw_parse_alloc();
        int status_state = 0;

        for (size_t i = 0; i < Cmds_02.size(); ++i) {
            ssize_t bytes = gw_socket_recv(server_client_fd2, tx_buf, sizeof(tx_buf));
            SCOPED_TRACE("Phase2: " + Cmds_02[i]);
            EXPECT_GT(bytes, 0);

            GWCommand_t c = test_get_cmd_enum(Cmds_02[i]);
            std::string a = test_get_cmd_arg(Cmds_02[i]);
            gw_parse_setcmd(cmd2, c);
            gw_parse_setarg(cmd2, a.c_str());

            int res = gw_cntstate_process(cnt2, cmd2, tx_buf, sizeof(tx_buf), &status_state);
            EXPECT_GT(res, 0);
            EXPECT_EQ(status_state, 1);
        }
        terminal_b2.get();
        gw_parse_destroy(cmd2);
        gw_cntstate_destroy(cnt2);
        gw_socket_close(server_client_fd2);
    }

    gw_socket_close(listen_fd);
}

TEST_F(ConnectionSequentialTest, FramingReassemblyTest)
{
    /* phase 3: CONNECT -> [frame A + frame B + partial frame C]
                -> recv() -> recv()
                -> [expected ok]
    */
    signal(SIGPIPE, SIG_IGN);
    int listen_fd = gw_socket_create(test_port);
    ASSERT_GT(listen_fd, 0);

    std::string frame_a = make_frame("PING");
    std::string frame_b = make_frame("REGISTER user123");
    std::string frame_c = make_frame("GET_STATUS");

    // split frame c
    size_t split_point = frame_c.size() - 3;
    std::string frame_c_part1 = frame_c.substr(0, split_point);
    std::string frame_c_part2 = frame_c.substr(split_point);

    auto terminal_b3 = std::async(std::launch::async, [&](){
        int client_fd = gw_socket_connect("127.0.0.1", test_port);
        if (client_fd <= 0) return;
    
        // step 1: send A + B + a part of C 
        std::string first_chunk = frame_a + frame_b + frame_c_part1;
        send(client_fd, first_chunk.data(), first_chunk.size(), 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // step 2: rest of C
        send(client_fd, frame_c_part2.data(), frame_c_part2.size(), 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        gw_socket_close(client_fd);
    });

    // server
    int server_client_fd = gw_socket_accept(listen_fd);
    ASSERT_GT(server_client_fd, 0);

    uint8_t stream_buffer[8 * MAX_BUF_LEN];
    size_t buffer_size = 0;
    std::vector<std::string> parsed_payloads;

    for (int recv_round = 0; recv_round < 2; ++recv_round) {
        uint8_t temp_rx_buf[MAX_BUF_LEN];
        ssize_t bytes = gw_socket_recv(server_client_fd, temp_rx_buf, sizeof(temp_rx_buf));
        SCOPED_TRACE("recv round " + std::to_string(recv_round));
        ASSERT_GT(bytes, 0);

        ASSERT_LE(buffer_size + (size_t)bytes, sizeof(stream_buffer));
        memcpy(stream_buffer + buffer_size, temp_rx_buf, (size_t)bytes);
        buffer_size += (size_t)bytes;

        while (1) {
            char clean_payload[MAX_ARG_LEN];
            size_t consumed = 0;

            int status = gw_parse_msglen_prefixing(stream_buffer, buffer_size,
                                &consumed, clean_payload, sizeof(clean_payload));
            if (status == 0) break;
            ASSERT_GE(status, 0);

            parsed_payloads.emplace_back(clean_payload);

            size_t remaining = buffer_size - consumed;
            if (remaining > 0) {
                memmove(stream_buffer, stream_buffer + consumed, remaining);
            }
            buffer_size = remaining;
        }
    }
    terminal_b3.get();
    gw_socket_close(server_client_fd);
    gw_socket_close(listen_fd);

    ASSERT_EQ(parsed_payloads.size(), 3u);
    EXPECT_EQ(parsed_payloads[0], "PING");
    EXPECT_EQ(parsed_payloads[1], "REGISTER user123");
    EXPECT_EQ(parsed_payloads[2], "GET_STATUS");
}
