#pragma once

#include <pebble.h>

#define COMM_SIZE_INBOX     APP_MESSAGE_INBOX_SIZE_MINIMUM
#define COMM_SIZE_OUTBOX    256

typedef void (*SendHandler)();
typedef void (*FailedHandler)();

void comm_open(SendHandler send_handler, FailedHandler failed_handler);
void comm_close();
void comm_pause();
void comm_resume();

DictionaryIterator *comm_begin();
void comm_send();
