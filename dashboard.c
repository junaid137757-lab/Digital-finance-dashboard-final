#include <stdio.h>
#include <string.h>

#include "dashboard.h"
#include "common.h"
#include "budget.h"

void financialDashboard(void)
{
    float income = 0, expense = 0, savings = 0;
    int i, overBudgetCount = 0;

    for(i = 0; i < transactionCount; i++)
    {
        if(strcmp(transactions[i].type, "Income") == 0)
            income += transactions[i].amount;
        else
            expense += transactions[i].amount;
    }

    savings = income - expense;

    for(i = 0; i < budgetCount; i++)
    {
        if(getSpentForCategory(budgets[i].category) > budgets[i].limit)
            overBudgetCount++;
    }

    printf("\n=================================================\n");
    printf("             FINANCIAL DASHBOARD\n");
    printf("=================================================\n");
    printf("User             : %s\n", currentUser);
    printf("Total Income     : %.2f\n", income);
    printf("Total Expense    : %.2f\n", expense);
    printf("Net Savings      : %.2f\n", savings);

    if(income > 0)
        printf("Savings Rate     : %.2f%%\n", (savings / income) * 100);

    printf("Budgets Set      : %d (%d over limit)\n", budgetCount, overBudgetCount);
    printf("Savings Goals    : %d\n", goalCount);
    printf("Total Records    : %d\n", transactionCount);
    printf("=================================================\n");
}
