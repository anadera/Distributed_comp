#pragma once

#include "lab2.h"
#include "banking.h"
#include "ipc.h"
#include "io.h"
#include "pa2345.h"

void set_start_balance(local_id self, BalanceHistory* h, int* array);
void set_balance(BalanceHistory* history, balance_t amount);
void handle_transfer(PROCESS* p, Message * msgIN, BalanceHistory* h, FILENAME* f);
