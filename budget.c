#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "budget.h"
#include "common.h"
#include "persistence.h"
#include "notification.h"
#include "utility.h"
#include "category_index.h"
#include "activity_log.h"

float getSpentForCategory(const char *category)
{
    float spent = 0;
    int i;

    for(i = 0; i < transactionCount; i++)
    {
        if(strcmp(transactions[i].type, "Expense") == 0 &&
           strcmp(transactions[i].category, category) == 0)
        {
            spent += transactions[i].amount;
        }
    }

    return spent;
}

/* Returns the index of the first budget whose category is
   >= the given category in the (kept-sorted) budgets[]
   array - the classic "lower bound" binary search. If every
   category is smaller, returns budgetCount, which is also
   the correct insertion point at the end of the array. */
static int budgetLowerBound(const char *category)
{
    int lo = 0;
    int hi = budgetCount; /* one past the last valid index */

    while(lo < hi)
    {
        int mid = lo + (hi - lo) / 2;

        if(strcmp(budgets[mid].category, category) < 0)
            lo = mid + 1;
        else
            hi = mid;
    }

    return lo;
}

int findBudgetIndex(const char *category)
{
    int pos = budgetLowerBound(category);

    if(pos < budgetCount && strcmp(budgets[pos].category, category) == 0)
        return pos;

    return -1;
}

static int compareBudgetByCategory(const void *a, const void *b)
{
    const Budget *ba = (const Budget *)a;
    const Budget *bb = (const Budget *)b;

    return strcmp(ba->category, bb->category);
}

void sortBudgetsByCategory(void)
{
    qsort(budgets, (size_t)budgetCount, sizeof(Budget), compareBudgetByCategory);
}

void setBudget(void)
{
    char category[30] = "";
    char existingCategories[50][30];
    int existingCount = 0;
    float limit;
    int i, j, found;
    int choice;

    if(budgetCount >= MAX_BUDGETS)
    {
        printf("Budget Limit Reached!\n");
        return;
    }

    /* Only let the user set a budget for a category that's actually
       been used in a transaction, rather than free-typing a name
       that might not exist or have a typo - show the real
       categories first, then require picking one of them. */
    for(i = 0; i < transactionCount && existingCount < 50; i++)
    {
        int alreadyListed = 0;

        for(j = 0; j < existingCount; j++)
        {
            if(strcmp(existingCategories[j], transactions[i].category) == 0)
            {
                alreadyListed = 1;
                break;
            }
        }

        if(!alreadyListed)
        {
            strcpy(existingCategories[existingCount], transactions[i].category);
            existingCount++;
        }
    }

    if(existingCount == 0)
    {
        printf("No transaction categories available yet. Add an income or\n");
        printf("expense first, then set a budget for that category.\n");
        return;
    }

    printf("\nExisting Categories:\n");

    for(i = 0; i < existingCount; i++)
        printf("  %d. %s\n", i + 1, existingCategories[i]);

    printf("Select a Category by Number (or 0 to cancel): ");

    if(!readValidInt(&choice))
        return;

    if(choice == 0)
    {
        printf("Set Budget Cancelled.\n");
        return;
    }

    if(choice < 1 || choice > existingCount)
    {
        printf("Invalid Choice\n");
        return;
    }

    strcpy(category, existingCategories[choice - 1]);

    printf("Enter Monthly Budget Limit (or -1 to cancel): ");

    if(!readValidFloat(&limit))
        return;

    if(limit == -1)
    {
        printf("Set Budget Cancelled.\n");
        return;
    }

    found = findBudgetIndex(category);

    (void)pthread_mutex_lock(&dataMutex);

    if(found != -1)
    {
        budgets[found].limit = limit;
        printf("Budget Updated Successfully!\n");
    }
    else
    {
        int pos = budgetLowerBound(category);
        int k;

        /* Shift everything from the insertion point right by
           one slot to make room, keeping budgets[] sorted by
           category at all times - this is what makes the
           binary search in findBudgetIndex() valid. */
        for(k = budgetCount; k > pos; k--)
            budgets[k] = budgets[k - 1];

        strcpy(budgets[pos].category, category);
        budgets[pos].limit = limit;
        budgetCount++;

        printf("Budget Set Successfully!\n");
    }

    categoryIndexRebuild();

    saveBudgets();

    (void)pthread_mutex_unlock(&dataMutex);

    {
        char logMsg[80];

        snprintf(logMsg, sizeof(logMsg), "Set Budget: %s %.2f", category, limit);
        logActivity(logMsg);
    }
}

void viewBudgets(void)
{
    int i;

    if(budgetCount == 0)
    {
        printf("No Budgets Set\n");
        return;
    }

    printf("\n--------------------------------------------------------------------------------\n");
    printf("%-16s%-12s%-12s%-14s%s\n", "CATEGORY", "LIMIT", "SPENT", "REMAINING", "STATUS");
    printf("--------------------------------------------------------------------------------\n");

    for(i = 0; i < budgetCount; i++)
    {
        float spent = getSpentForCategory(budgets[i].category);
        float remaining = budgets[i].limit - spent;

        printf("%-16s%-12.2f%-12.2f%-14.2f",
               budgets[i].category,
               budgets[i].limit,
               spent,
               remaining);

        if(spent > budgets[i].limit)
            printf("[OVER BUDGET]");
        else if(budgets[i].limit > 0 && spent >= 0.9f * budgets[i].limit)
            printf("[NEAR LIMIT]");
        else
            printf("[OK]");

        printf("\n");
    }
}

void checkBudgetAlert(const char *category)
{
    int idx = categoryIndexLookup(category);

    if(idx != -1)
    {
        float spent = getSpentForCategory(category);

        notifyBudgetStatus(category, spent, budgets[idx].limit);
    }
}
