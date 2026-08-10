#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "transaction.h"
#include "common.h"
#include "persistence.h"
#include "budget.h"
#include "utility.h"
#include "activity_log.h"
#include "logger.h"

/* Comparator for qsort(): orders transactions chronologically
   by date. Dates are stored as "YYYY-MM-DD" strings, which
   happen to sort correctly with a plain strcmp() because the
   format is fixed-width and most-significant-field-first -
   no need to parse them into a struct tm just to compare. */
static int compareByDate(const void *a, const void *b)
{
    const Transaction *ta = (const Transaction *)a;
    const Transaction *tb = (const Transaction *)b;

    return strcmp(ta->date, tb->date);
}

void sortTransactionsByDate(void)
{
    qsort(transactions, (size_t)transactionCount, sizeof(Transaction), compareByDate);
}

/* Comparator for qsort(): orders transactions by amount,
   smallest first. Uses explicit comparisons rather than
   subtracting the floats, since a bare subtraction can't be
   safely truncated to an int the way it can for integer keys. */
static int compareByAmount(const void *a, const void *b)
{
    const Transaction *ta = (const Transaction *)a;
    const Transaction *tb = (const Transaction *)b;

    if(ta->amount < tb->amount)
        return -1;

    if(ta->amount > tb->amount)
        return 1;

    return 0;
}

void sortTransactionsByAmount(void)
{
    qsort(transactions, (size_t)transactionCount, sizeof(Transaction), compareByAmount);
}

void viewTransactions(void)
{
    int i;
    float balance = 0;

    if(transactionCount == 0)
    {
        printf("No Transactions Available\n");
        return;
    }

    printf("\n--------------------------------------------------------------------------------\n");
    printf("%-5s%-10s%-16s%-12s%-14s%-12s\n", "ID", "TYPE", "CATEGORY", "AMOUNT", "DATE", "BALANCE");
    printf("--------------------------------------------------------------------------------\n");

    for(i = 0; i < transactionCount; i++)
    {
        if(strcmp(transactions[i].type, "Income") == 0)
            balance += transactions[i].amount;
        else
            balance -= transactions[i].amount;

        printf("%-5d%-10s%-16s%-12.2f%-14s%-12.2f\n",
               transactions[i].id,
               transactions[i].type,
               transactions[i].category,
               transactions[i].amount,
               transactions[i].date,
               balance);
    }

    printf("--------------------------------------------------------------------------------\n");
    printf("Running Balance (Total Income - Total Expense): %.2f\n", balance);
}

void editTransaction(void)
{
    int id, i, found = -1;
    char newCategory[30] = "";
    float newAmount;

    viewTransactions();

    if(transactionCount == 0)
        return;

    printf("\nEnter ID of Transaction to Edit (or 0 to cancel): ");

    if(!readValidInt(&id))
        return;

    if(id == 0)
    {
        printf("Edit Cancelled.\n");
        return;
    }

    for(i = 0; i < transactionCount; i++)
    {
        if(transactions[i].id == id)
        {
            found = i;
            break;
        }
    }

    if(found == -1)
    {
        printf("Transaction Not Found!\n");
        return;
    }

    printf("Enter New Category (or 0 to cancel): ");
    readLine(newCategory, sizeof(newCategory));

    if(strcmp(newCategory, "0") == 0)
    {
        printf("Edit Cancelled.\n");
        return;
    }

    printf("Enter New Amount (or -1 to cancel): ");

    if(!readValidFloat(&newAmount))
        return;

    if(newAmount == -1)
    {
        printf("Edit Cancelled.\n");
        return;
    }

    (void)pthread_mutex_lock(&dataMutex);
    strcpy(transactions[found].category, newCategory);
    transactions[found].amount = newAmount;
    saveTransactions();
    (void)pthread_mutex_unlock(&dataMutex);

    printf("Transaction Updated Successfully!\n");

    {
        char logMsg[80];

        snprintf(logMsg, sizeof(logMsg), "Edited Transaction #%d: %s %.2f", id, newCategory, newAmount);
        logActivity(logMsg);
    }

    if(strcmp(transactions[found].type, "Expense") == 0)
        checkBudgetAlert(transactions[found].category);
}

void deleteTransaction(void)
{
    int id, i, found = -1;

    viewTransactions();

    if(transactionCount == 0)
        return;

    printf("\nEnter ID of Transaction to Delete (or 0 to cancel): ");

    if(!readValidInt(&id))
        return;

    if(id == 0)
    {
        printf("Delete Cancelled.\n");
        return;
    }

    for(i = 0; i < transactionCount; i++)
    {
        if(transactions[i].id == id)
        {
            found = i;
            break;
        }
    }

    if(found == -1)
    {
        char logMsg[80];

        snprintf(logMsg, sizeof(logMsg), "delete requested for unknown transaction id %d", id);
        LOG_WARN_MSG(logMsg);

        printf("Transaction Not Found!\n");
        return;
    }

    {
        char logMsg[80];

        snprintf(logMsg, sizeof(logMsg), "Deleted Transaction: %s %s %.2f",
                 transactions[found].type, transactions[found].category, transactions[found].amount);

        (void)pthread_mutex_lock(&dataMutex);

        for(i = found; i < transactionCount - 1; i++)
            transactions[i] = transactions[i + 1];

        transactionCount--;

        saveTransactions();

        (void)pthread_mutex_unlock(&dataMutex);

        printf("Transaction Deleted Successfully!\n");

        logActivity(logMsg);
    }
}

static int printMatchingTransactions(const char *category)
{
    int i, found = 0;

    for(i = 0; i < transactionCount; i++)
    {
        if(strcmp(transactions[i].category, category) == 0)
        {
            if(!found)
            {
                printf("\n--------------------------------------------------------------------\n");
                printf("%-5s%-10s%-16s%-12s%-14s\n", "ID", "TYPE", "CATEGORY", "AMOUNT", "DATE");
                printf("--------------------------------------------------------------------\n");
            }

            printf("%-5d%-10s%-16s%-12.2f%-14s\n",
                   transactions[i].id,
                   transactions[i].type,
                   transactions[i].category,
                   transactions[i].amount,
                   transactions[i].date);

            found = 1;
        }
    }

    return found;
}

void searchTransactionsByCategory(void)
{
    char category[30] = "";
    char suggestions[10][30];
    int suggestionCount = 0;
    int i, j;
    int suggestionChoice;

    printf("Enter Category to Search (or 0 to cancel): ");
    readLine(category, sizeof(category));

    if(strcmp(category, "0") == 0)
    {
        printf("Search Cancelled.\n");
        return;
    }

    if(printMatchingTransactions(category))
        return;

    printf("No Transactions Found In This Category\n");

    /* No exact match - offer "Did you mean" suggestions based on a
       case-insensitive partial match, the way a search engine would,
       instead of just giving up (e.g. searching "income" when the
       actual category is "stocks income"). */
    for(i = 0; i < transactionCount && suggestionCount < 10; i++)
    {
        int alreadyListed = 0;

        for(j = 0; j < suggestionCount; j++)
        {
            if(strcmp(suggestions[j], transactions[i].category) == 0)
            {
                alreadyListed = 1;
                break;
            }
        }

        if(!alreadyListed &&
           (caseInsensitiveContains(transactions[i].category, category) ||
            caseInsensitiveContains(category, transactions[i].category)))
        {
            strcpy(suggestions[suggestionCount], transactions[i].category);
            suggestionCount++;
        }
    }

    if(suggestionCount == 0)
        return;

    printf("\nDid you mean:\n");

    for(i = 0; i < suggestionCount; i++)
        printf("  %d. %s\n", i + 1, suggestions[i]);

    printf("Enter number to search that category (or 0 to skip): ");

    if(!readValidInt(&suggestionChoice))
        return;

    if(suggestionChoice < 1 || suggestionChoice > suggestionCount)
        return;

    printMatchingTransactions(suggestions[suggestionChoice - 1]);
}
