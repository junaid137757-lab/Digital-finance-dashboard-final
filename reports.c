#include <stdio.h>
#include <string.h>

#include "reports.h"
#include "common.h"
#include "transaction.h"

void categoryWiseReport(void)
{
    char seen[MAX][30];
    float totals[MAX];
    int seenCount = 0;
    int i, j;
    float remainingBalance = 0;

    if(transactionCount == 0)
    {
        printf("No Transactions Available\n");
        return;
    }

    sortTransactionsByDate();

    for(i = 0; i < transactionCount; i++)
    {
        int found = -1;

        for(j = 0; j < seenCount; j++)
        {
            if(strcmp(seen[j], transactions[i].category) == 0)
            {
                found = j;
                break;
            }
        }

        if(found == -1)
        {
            strcpy(seen[seenCount], transactions[i].category);
            totals[seenCount] = (strcmp(transactions[i].type, "Income") == 0)
                                     ? transactions[i].amount
                                     : -transactions[i].amount;
            seenCount++;
        }
        else
        {
            totals[found] += (strcmp(transactions[i].type, "Income") == 0)
                                  ? transactions[i].amount
                                  : -transactions[i].amount;
        }
    }

    printf("\n--------------------------------------------------------------------\n");
    printf("CATEGORY-WISE REPORT (Positive = Net Income, Negative = Net Expense)\n");
    printf("--------------------------------------------------------------------\n");
    printf("%-20s%s\n", "CATEGORY", "NET AMOUNT");
    printf("--------------------------------------------------------------------\n");

    for(i = 0; i < seenCount; i++)
    {
        printf("%-20s%.2f\n", seen[i], totals[i]);
        remainingBalance += totals[i];
    }

    printf("--------------------------------------------------------------------\n");
    printf("Remaining Balance (Total Income - Total Expense): %.2f\n", remainingBalance);
}

void monthlyReport(void)
{
    char months[MAX][8];
    float income[MAX];
    float expense[MAX];
    int monthCount = 0;
    int i, j;

    if(transactionCount == 0)
    {
        printf("No Transactions Available\n");
        return;
    }

    sortTransactionsByDate();

    for(i = 0; i < transactionCount; i++)
    {
        char monthKey[8];
        int found = -1;

        strncpy(monthKey, transactions[i].date, 7);
        monthKey[7] = '\0';

        for(j = 0; j < monthCount; j++)
        {
            if(strcmp(months[j], monthKey) == 0)
            {
                found = j;
                break;
            }
        }

        if(found == -1)
        {
            strcpy(months[monthCount], monthKey);
            income[monthCount] = 0;
            expense[monthCount] = 0;
            found = monthCount;
            monthCount++;
        }

        if(strcmp(transactions[i].type, "Income") == 0)
            income[found] += transactions[i].amount;
        else
            expense[found] += transactions[i].amount;
    }

    printf("\n--------------------------------------------------------------------\n");
    printf("MONTHLY REPORT\n");
    printf("--------------------------------------------------------------------\n");
    printf("%-12s%-14s%-14s%s\n", "MONTH", "INCOME", "EXPENSE", "NET");
    printf("--------------------------------------------------------------------\n");

    for(i = 0; i < monthCount; i++)
    {
        printf("%-12s%-14.2f%-14.2f%.2f\n",
               months[i], income[i], expense[i], income[i] - expense[i]);
    }
}
