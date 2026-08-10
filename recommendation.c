#include <stdio.h>
#include <string.h>

#include "recommendation.h"
#include "common.h"
#include "budget.h"

void generateRecommendations(void)
{
    float income = 0, expense = 0;
    int i;
    int hasEmergencyFund = 0;
    int printedAny = 0;

    for(i = 0; i < transactionCount; i++)
    {
        if(strcmp(transactions[i].type, "Income") == 0)
            income += transactions[i].amount;
        else
            expense += transactions[i].amount;
    }

    for(i = 0; i < goalCount; i++)
    {
        if(strstr(goals[i].name, "Emergency") != NULL ||
           strstr(goals[i].name, "emergency") != NULL)
        {
            hasEmergencyFund = 1;
            break;
        }
    }

    printf("\n=================================================\n");
    printf("        SMART FINANCIAL RECOMMENDATIONS\n");
    printf("=================================================\n");

    if(transactionCount == 0)
    {
        printf("Start logging your income and expenses to receive\n");
        printf("personalized financial recommendations.\n");
        printf("=================================================\n");
        return;
    }

    if(expense > income)
    {
        printf("- Your expenses currently exceed your income. Consider\n");
        printf("  reviewing your largest spending categories and cutting\n");
        printf("  back to avoid debt.\n");
        printedAny = 1;
    }
    else if(income > 0 && ((income - expense) / income) < 0.20f)
    {
        printf("- Your savings rate is below 20%%. Try to trim discretionary\n");
        printf("  spending to build a healthier savings buffer.\n");
        printedAny = 1;
    }
    else if(income > 0)
    {
        printf("- Great job! Your savings rate is healthy. Consider investing\n");
        printf("  your surplus savings for long-term growth.\n");
        printedAny = 1;
    }

    for(i = 0; i < budgetCount; i++)
    {
        float spent = getSpentForCategory(budgets[i].category);

        if(spent > budgets[i].limit)
        {
            printf("- You are over budget in '%s'. Consider lowering spending\n", budgets[i].category);
            printf("  in this category next month.\n");
            printedAny = 1;
        }
    }

    for(i = 0; i < goalCount; i++)
    {
        if(goals[i].targetAmount > 0 &&
           (goals[i].savedAmount / goals[i].targetAmount) < 0.5f)
        {
            printf("- Your goal '%s' is less than halfway funded. Consider\n", goals[i].name);
            printf("  setting up regular contributions.\n");
            printedAny = 1;
        }
    }

    if(!hasEmergencyFund)
    {
        printf("- You don't have an emergency fund goal yet. Consider creating\n");
        printf("  one to cover 3-6 months of essential expenses.\n");
        printedAny = 1;
    }

    if(!printedAny)
        printf("- Your finances look well managed. Keep up the good habits!\n");

    printf("=================================================\n");
}
