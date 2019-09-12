#include "modules/mod_comm.h"

static bool s_sending;
static SendHandler s_send_handler;
static FailedHandler s_failed_handler;

static void print_result(AppMessageResult result) {
    switch(result) {
        case APP_MSG_OK:
            APP_LOG(APP_LOG_LEVEL_WARNING, "All good, operation was successful.");
            break;

        case APP_MSG_SEND_TIMEOUT:
            APP_LOG(APP_LOG_LEVEL_WARNING, "The other end did not confirm receiving the sent data with an (n)ack in time.");
            break;

        case APP_MSG_SEND_REJECTED:
            APP_LOG(APP_LOG_LEVEL_WARNING, "The other end rejected the sent data, with a \"nack\" reply.");
            break;

        case APP_MSG_NOT_CONNECTED:
            APP_LOG(APP_LOG_LEVEL_WARNING, "The other end was not connected.");
            break;

        case APP_MSG_APP_NOT_RUNNING:
            APP_LOG(APP_LOG_LEVEL_WARNING, "The local application was not running.");
            break;

        case APP_MSG_INVALID_ARGS:
            APP_LOG(APP_LOG_LEVEL_WARNING, "The function was called with invalid arguments.");
            break;

        case APP_MSG_BUSY:
            APP_LOG(APP_LOG_LEVEL_WARNING, "There are pending (in or outbound) messages that need to be processed first before new ones can be received or sent.");
            break;

        case APP_MSG_BUFFER_OVERFLOW:
            APP_LOG(APP_LOG_LEVEL_WARNING, "The buffer was too small to contain the incoming message.");
            break;

        case APP_MSG_ALREADY_RELEASED:
            APP_LOG(APP_LOG_LEVEL_WARNING, "The resource had already been released.");
            break;

        case APP_MSG_CALLBACK_ALREADY_REGISTERED:
            APP_LOG(APP_LOG_LEVEL_WARNING, "The callback was already registered.");
            break;

        case APP_MSG_CALLBACK_NOT_REGISTERED:
            APP_LOG(APP_LOG_LEVEL_WARNING, "The callback could not be deregistered, because it had not been registered before.");
            break;

        case APP_MSG_OUT_OF_MEMORY:
            APP_LOG(APP_LOG_LEVEL_WARNING, "The system did not have sufficient application memory to perform the requested operation.");
            break;

        case APP_MSG_CLOSED:
            APP_LOG(APP_LOG_LEVEL_WARNING, "App message was closed.");
            break;

        case APP_MSG_INTERNAL_ERROR:
            APP_LOG(APP_LOG_LEVEL_WARNING, "An internal OS error prevented AppMessage from completing an operation.");
            break;

        case APP_MSG_INVALID_STATE:
            APP_LOG(APP_LOG_LEVEL_WARNING, "The function was called while App Message was not in the appropriate state.");
            break;
        default:
            APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown AppMessageResult");
    }
}

static void out_failed_handler(DictionaryIterator *iter, AppMessageResult result, void *context) {
    print_result(result);
    if (s_sending)
        s_failed_handler();
}

static void outbox_sent_handler(DictionaryIterator *iterator, void *context) {
    if (s_sending)
        s_send_handler();
}

DictionaryIterator *comm_begin() {
    DictionaryIterator *out;
    AppMessageResult result = app_message_outbox_begin(&out);
    if (result != APP_MSG_OK)
        print_result(result);
    return out;
}

void comm_send() {
    AppMessageResult result = app_message_outbox_send();
    if (result != APP_MSG_OK)
        print_result(result);
}

void comm_open(SendHandler send_handler, FailedHandler failed_handler) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Opening Communication, subscribing handlers");
    app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
    app_message_register_outbox_sent(outbox_sent_handler);
    app_message_register_outbox_failed(out_failed_handler);
    app_message_open(COMM_SIZE_INBOX, COMM_SIZE_OUTBOX);
    s_sending = false;
    s_send_handler = send_handler;
    s_failed_handler = failed_handler;
}

void comm_close() {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Closing communication, unsubscribing handlers");
    app_message_deregister_callbacks();
}

void comm_pause() {
    s_sending = false;
}

void comm_resume() {
    s_sending = true;
    s_send_handler();
}
