#include "unity.h"
#include "bt_access_control.h"

TEST_CASE("AccessControlInit initializes without error", "[middleware][access_control]")
{
    TEST_ASSERT_TRUE(AccessControlInit());
}

TEST_CASE("Empty whitelist allows any BDA", "[middleware][access_control]")
{
    uint8_t bda[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    TEST_ASSERT_TRUE(AccessControlIsAllowed(bda));
}

TEST_CASE("Blacklisted BDA is denied", "[middleware][access_control]")
{
    uint8_t bda[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    TEST_ASSERT_TRUE(AccessControlAdd(bda, ACL_BLACKLIST));
    TEST_ASSERT_FALSE(AccessControlIsAllowed(bda));
}

TEST_CASE("Whitelisted BDA is allowed", "[middleware][access_control]")
{
    uint8_t bda[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    TEST_ASSERT_TRUE(AccessControlAdd(bda, ACL_WHITELIST));
    TEST_ASSERT_TRUE(AccessControlIsAllowed(bda));
}

TEST_CASE("Non-whitelisted BDA denied when whitelist active", "[middleware][access_control]")
{
    uint8_t listed[]   = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    uint8_t unlisted[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    TEST_ASSERT_TRUE(AccessControlAdd(listed, ACL_WHITELIST));
    TEST_ASSERT_FALSE(AccessControlIsAllowed(unlisted));
}

TEST_CASE("Remove BDA from lists", "[middleware][access_control]")
{
    uint8_t bda[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01};
    TEST_ASSERT_TRUE(AccessControlAdd(bda, ACL_BLACKLIST));
    TEST_ASSERT_TRUE(AccessControlIsBlacklisted(bda));
    TEST_ASSERT_TRUE(AccessControlRemove(bda));
    TEST_ASSERT_FALSE(AccessControlIsBlacklisted(bda));
}

TEST_CASE("Duplicate add is idempotent", "[middleware][access_control]")
{
    uint8_t bda[] = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60};
    TEST_ASSERT_TRUE(AccessControlAdd(bda, ACL_WHITELIST));
    TEST_ASSERT_TRUE(AccessControlAdd(bda, ACL_WHITELIST));
    TEST_ASSERT_TRUE(AccessControlIsWhitelisted(bda));
}

TEST_CASE("NULL BDA returns false", "[middleware][access_control]")
{
    TEST_ASSERT_FALSE(AccessControlIsAllowed(NULL));
    TEST_ASSERT_FALSE(AccessControlAdd(NULL, ACL_WHITELIST));
    TEST_ASSERT_FALSE(AccessControlRemove(NULL));
}
