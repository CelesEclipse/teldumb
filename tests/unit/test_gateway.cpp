#include <cstdint>
#include <gtest/gtest.h>

extern "C" {
    #include "gateway/core/connection.h"
    #include "gateway/core/command.h"
}

class ConnectionStateTest : public ::testing::Test
{
protected:
    GWCnt_State_t * cnt;
    GWCmd_t * cmd;
    uint8_t tx_buf[1024];

    void SetUp() override
    {
        cnt = gw_cntstate_alloc();
        cmd = gw_parse_alloc();
        memset(tx_buf, 0, sizeof(tx_buf));
    }

    void TearDown() override
    {
        gw_parse_destroy(cmd);
        gw_cntstate_destroy(cnt);
    }
};

TEST_F(ConnectionStateTest, RejectsAuthBeforeRegister)
{
    int status_state = 0;
    gw_parse_setcmd(cmd, AUTH);
    gw_parse_setarg(cmd, "UE001");

    int result = gw_cntstate_process(cnt, cmd, tx_buf, sizeof(tx_buf), &status_state);

    EXPECT_EQ(result, -1);
    EXPECT_EQ(status_state, -2);
}

TEST_F(ConnectionStateTest, HandlesValidRegistrationTransition)
{
    int status_state = 0;
    gw_parse_setcmd(cmd, REGISTER);
    gw_parse_setarg(cmd, "UE001");

    int result = gw_cntstate_process(cnt, cmd, tx_buf, sizeof(tx_buf), &status_state);

    EXPECT_GT(result, 0);
    EXPECT_EQ(status_state, 1);

    // state is now STATE_REGISTERED
    gw_parse_setcmd(cmd, AUTH);
    result = gw_cntstate_process(cnt, cmd, tx_buf, sizeof(tx_buf), &status_state);
    EXPECT_GT(result, 0);
    EXPECT_EQ(status_state, 1);
}

TEST_F(ConnectionStateTest, RejectsMismatchedIdentityOnAuth)
{
    int status_state = 0;
    gw_parse_setcmd(cmd, REGISTER);
    gw_parse_setarg(cmd, "UE001");
    gw_cntstate_process(cnt, cmd, tx_buf, sizeof(tx_buf), &status_state);

    // set another client name to see if the gateway reject or not
    gw_parse_setcmd(cmd, AUTH);
    gw_parse_setarg(cmd, "UE006");
    int result = gw_cntstate_process(cnt, cmd, tx_buf, sizeof(tx_buf), &status_state);

    EXPECT_EQ(result, -1);
    EXPECT_EQ(status_state, -5);
}
