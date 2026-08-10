#include <stdio.h>

#include "persistence.h"
#include "common.h"
#include "budget.h"
#include "category_index.h"
#include "logger.h"
#include "transaction.h"

void loadTransactions(void)
{
    char filename[60];

    snprintf(filename, sizeof(filename), "data/%s_transactions.dat", currentUser);

    FILE *fp = fopen(filename, "rb");

    transactionCount = 0;

    if(fp == NULL)
    {
        {
            char logMsg[100];

            snprintf(logMsg, sizeof(logMsg), "no transactions file yet for user '%s'", currentUser);
            LOG_DEBUG_MSG(logMsg);
        }
        return;
    }

    if(fread(&transactionCount, sizeof(int), 1, fp) != 1)
    {
        fclose(fp);
        transactionCount = 0;
        {
            char logMsg[120];

            snprintf(logMsg, sizeof(logMsg), "transactions file for '%s' is short/corrupted - could not read count", currentUser);
            LOG_WARN_MSG(logMsg);
        }
        return;
    }

    if(fread(transactions, sizeof(Transaction), (size_t)transactionCount, fp) != (size_t)transactionCount)
    {
        char logMsg[120];

        snprintf(logMsg, sizeof(logMsg), "transactions file for '%s' is short/corrupted - could not read all records", currentUser);
        LOG_WARN_MSG(logMsg);
        transactionCount = 0;
    }

    fclose(fp);

    /* Older files (saved before renumbering existed) can have
       gaps or duplicate ids left over from deletions. Renumbering
       here self-repairs that automatically the next time this
       user logs in, instead of leaving it broken forever. */
    renumberTransactionIds();
}

void saveTransactions(void)
{
    char filename[60];

    snprintf(filename, sizeof(filename), "data/%s_transactions.dat", currentUser);

    FILE *fp = fopen(filename, "wb");

    if(fp == NULL)
    {
        {
            char logMsg[150];

            snprintf(logMsg, sizeof(logMsg), "could not open '%s' for writing - transactions not saved", filename);
            LOG_ERROR_MSG(logMsg);
        }
        return;
    }

    fwrite(&transactionCount, sizeof(int), 1, fp);
    fwrite(transactions, sizeof(Transaction), (size_t)transactionCount, fp);

    fclose(fp);
}

void loadBudgets(void)
{
    char filename[60];

    snprintf(filename, sizeof(filename), "data/%s_budgets.dat", currentUser);

    FILE *fp = fopen(filename, "rb");

    budgetCount = 0;

    if(fp == NULL)
    {
        {
            char logMsg[100];

            snprintf(logMsg, sizeof(logMsg), "no budgets file yet for user '%s'", currentUser);
            LOG_DEBUG_MSG(logMsg);
        }
        /* No file for this user yet: still rebuild the index
           so it reflects budgetCount == 0 instead of holding
           stale entries left over from whichever user was
           logged in before. */
        categoryIndexRebuild();
        return;
    }

    if(fread(&budgetCount, sizeof(int), 1, fp) != 1)
    {
        fclose(fp);
        budgetCount = 0;
        {
            char logMsg[120];

            snprintf(logMsg, sizeof(logMsg), "budgets file for '%s' is short/corrupted - could not read count", currentUser);
            LOG_WARN_MSG(logMsg);
        }
        categoryIndexRebuild();
        return;
    }

    if(fread(budgets, sizeof(Budget), (size_t)budgetCount, fp) != (size_t)budgetCount)
    {
        char logMsg[120];

        snprintf(logMsg, sizeof(logMsg), "budgets file for '%s' is short/corrupted - could not read all records", currentUser);
        LOG_WARN_MSG(logMsg);
        budgetCount = 0;
    }

    fclose(fp);

    /* Enforce the sorted-by-category invariant that
       findBudgetIndex()'s binary search depends on,
       regardless of what order the file was written in
       (defensive - setBudget() always writes it sorted, but
       an older or externally-written file might not be),
       then rebuild the in-memory hash index to match. */
    sortBudgetsByCategory();
    categoryIndexRebuild();
}

void saveBudgets(void)
{
    char filename[60];

    snprintf(filename, sizeof(filename), "data/%s_budgets.dat", currentUser);

    FILE *fp = fopen(filename, "wb");

    if(fp == NULL)
    {
        {
            char logMsg[150];

            snprintf(logMsg, sizeof(logMsg), "could not open '%s' for writing - budgets not saved", filename);
            LOG_ERROR_MSG(logMsg);
        }
        return;
    }

    fwrite(&budgetCount, sizeof(int), 1, fp);
    fwrite(budgets, sizeof(Budget), (size_t)budgetCount, fp);

    fclose(fp);
}

void loadGoals(void)
{
    char filename[60];

    snprintf(filename, sizeof(filename), "data/%s_goals.dat", currentUser);

    FILE *fp = fopen(filename, "rb");

    goalCount = 0;

    if(fp == NULL)
    {
        {
            char logMsg[100];

            snprintf(logMsg, sizeof(logMsg), "no goals file yet for user '%s'", currentUser);
            LOG_DEBUG_MSG(logMsg);
        }
        return;
    }

    if(fread(&goalCount, sizeof(int), 1, fp) != 1)
    {
        fclose(fp);
        goalCount = 0;
        {
            char logMsg[120];

            snprintf(logMsg, sizeof(logMsg), "goals file for '%s' is short/corrupted - could not read count", currentUser);
            LOG_WARN_MSG(logMsg);
        }
        return;
    }

    if(fread(goals, sizeof(SavingsGoal), (size_t)goalCount, fp) != (size_t)goalCount)
    {
        char logMsg[120];

        snprintf(logMsg, sizeof(logMsg), "goals file for '%s' is short/corrupted - could not read all records", currentUser);
        LOG_WARN_MSG(logMsg);
        goalCount = 0;
    }

    fclose(fp);
}

void saveGoals(void)
{
    char filename[60];

    snprintf(filename, sizeof(filename), "data/%s_goals.dat", currentUser);

    FILE *fp = fopen(filename, "wb");

    if(fp == NULL)
    {
        {
            char logMsg[150];

            snprintf(logMsg, sizeof(logMsg), "could not open '%s' for writing - goals not saved", filename);
            LOG_ERROR_MSG(logMsg);
        }
        return;
    }

    fwrite(&goalCount, sizeof(int), 1, fp);
    fwrite(goals, sizeof(SavingsGoal), (size_t)goalCount, fp);

    fclose(fp);
}

void loadEmergencyFund(void)
{
    char filename[60];

    snprintf(filename, sizeof(filename), "data/%s_emergencyfund.dat", currentUser);

    FILE *fp = fopen(filename, "rb");

    emergencyFundBalance = 0;

    if(fp == NULL)
    {
        {
            char logMsg[100];

            snprintf(logMsg, sizeof(logMsg), "no emergency fund file yet for user '%s'", currentUser);
            LOG_DEBUG_MSG(logMsg);
        }
        return;
    }

    if(fread(&emergencyFundBalance, sizeof(float), 1, fp) != 1)
    {
        char logMsg[120];

        snprintf(logMsg, sizeof(logMsg), "emergency fund file for '%s' is short/corrupted", currentUser);
        LOG_WARN_MSG(logMsg);
        emergencyFundBalance = 0;
    }

    fclose(fp);
}

void saveEmergencyFund(void)
{
    char filename[60];

    snprintf(filename, sizeof(filename), "data/%s_emergencyfund.dat", currentUser);

    FILE *fp = fopen(filename, "wb");

    if(fp == NULL)
    {
        {
            char logMsg[150];

            snprintf(logMsg, sizeof(logMsg), "could not open '%s' for writing - emergency fund not saved", filename);
            LOG_ERROR_MSG(logMsg);
        }
        return;
    }

    fwrite(&emergencyFundBalance, sizeof(float), 1, fp);

    fclose(fp);
}
