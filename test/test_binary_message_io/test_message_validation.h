#ifndef TEST_MESSAGE_VALIDATION_H
#define TEST_MESSAGE_VALIDATION_H

void test_validateMessage_ok();
void test_validateMessage_invalidStartByte();
void test_validateMessage_invalidDeviceID();
void test_validateMessage_invalidMessageID();
void test_validateMessage_invalidSize();
void test_validateMessage_handshake_sizeShouldBe3();
void test_validateMessage_ping_payloadShouldBeEmpty();
void test_validateMessage_confirm_payloadShouldBeEmpty();
void test_validateMessage_rejected_payloadShouldBeEmpty();
void test_validateMessage_disconnect_payloadShouldBeEmpty();
void test_validateMessage_incorrectFormat_payloadShouldBeEmpty();
void test_validateMessage_incorrectChecksum_payloadShouldBeEmpty();

#endif //TEST_MESSAGE_VALIDATION_H