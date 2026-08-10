#include <stdio.h>
#include <string.h>

#include "expense.h"
#include "common.h"
#include "utility.h"
#include "persistence.h"
#include "budget.h"
#include "activity_log.h"
#include "transaction.h"

void addExpense(void)
{
    Transaction t;

    if(transactionCount >= MAX)
    {
        printf("Transaction Limit Reached!\n");
        return;
    }

    strcpy(t.type, "Expense");

    printf("Enter Expense Category (or 0 to cancel): ");
    readLine(t.category, sizeof(t.category));

    if(strcmp(t.category, "0") == 0)
    {
        printf("Add Expense Cancelled.\n");
        return;
    }

    printf("Enter Amount (or -1 to cancel): ");

    if(!readValidFloat(&t.amount))
        return;

    if(t.amount == -1)
    {
        printf("Add Expense Cancelled.\n");
        return;
    }

    getCurrentDate(t.date);

    (void)pthread_mutex_lock(&dataMutex);
    t.id = nextTransactionId();
    transactions[transactionCount++] = t;
    saveTransactions();
    (void)pthread_mutex_unlock(&dataMutex);

    printf("Expense Added Successfully!\n");

    {
        char logMsg[80];

        snprintf(logMsg, sizeof(logMsg), "Added Expense: %s %.2f", t.category, t.amount);
        logActivity(logMsg);
    }

    checkBudgetAlert(t.category);
}
