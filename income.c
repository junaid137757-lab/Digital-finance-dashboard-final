#include <stdio.h>
#include <string.h>

#include "income.h"
#include "common.h"
#include "utility.h"
#include "persistence.h"
#include "activity_log.h"

void addIncome(void)
{
    Transaction t;

    if(transactionCount >= MAX)
    {
        printf("Transaction Limit Reached!\n");
        return;
    }

    strcpy(t.type, "Income");

    printf("Enter Income Category (or 0 to cancel): ");
    readLine(t.category, sizeof(t.category));

    if(strcmp(t.category, "0") == 0)
    {
        printf("Add Income Cancelled.\n");
        return;
    }

    printf("Enter Amount (or -1 to cancel): ");

    if(!readValidFloat(&t.amount))
        return;

    if(t.amount == -1)
    {
        printf("Add Income Cancelled.\n");
        return;
    }

    t.id = transactionCount + 1;
    getCurrentDate(t.date);

    (void)pthread_mutex_lock(&dataMutex);
    transactions[transactionCount++] = t;
    saveTransactions();
    (void)pthread_mutex_unlock(&dataMutex);

    printf("Income Added Successfully!\n");

    {
        char logMsg[80];

        snprintf(logMsg, sizeof(logMsg), "Added Income: %s %.2f", t.category, t.amount);
        logActivity(logMsg);
    }
}
