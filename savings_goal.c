#include <stdio.h>
#include <string.h>

#include "savings_goal.h"
#include "common.h"
#include "persistence.h"
#include "notification.h"
#include "utility.h"
#include "activity_log.h"

void setSavingsGoal(void)
{
    char name[50];
    float targetAmount;

    if(goalCount >= MAX_GOALS)
    {
        printf("Goal Limit Reached!\n");
        return;
    }

    printf("Enter Goal Name (or 0 to cancel): ");
    readLine(name, sizeof(name));

    if(strcmp(name, "0") == 0)
    {
        printf("Set Savings Goal Cancelled.\n");
        return;
    }

    printf("Enter Target Amount (or -1 to cancel): ");

    if(!readValidFloat(&targetAmount))
        return;

    if(targetAmount == -1)
    {
        printf("Set Savings Goal Cancelled.\n");
        return;
    }

    (void)pthread_mutex_lock(&dataMutex);
    strcpy(goals[goalCount].name, name);
    goals[goalCount].targetAmount = targetAmount;
    goals[goalCount].savedAmount = 0;

    goalCount++;

    saveGoals();
    (void)pthread_mutex_unlock(&dataMutex);

    printf("Savings Goal Set Successfully!\n");

    {
        char logMsg[110];

        snprintf(logMsg, sizeof(logMsg), "Set Savings Goal: %s (Target %.2f)", name, targetAmount);
        logActivity(logMsg);
    }
}

void contributeToGoal(void)
{
    char name[50];
    float amount;
    int i, found = -1;

    viewSavingsGoals();

    if(goalCount == 0)
        return;

    printf("\nEnter Goal Name to Contribute To (or 0 to cancel): ");
    readLine(name, sizeof(name));

    if(strcmp(name, "0") == 0)
    {
        printf("Contribution Cancelled.\n");
        return;
    }

    for(i = 0; i < goalCount; i++)
    {
        if(strcmp(goals[i].name, name) == 0)
        {
            found = i;
            break;
        }
    }

    if(found == -1)
    {
        printf("Goal Not Found!\n");
        return;
    }

    printf("Enter Amount to Add (or -1 to cancel): ");

    if(!readValidFloat(&amount))
        return;

    if(amount == -1)
    {
        printf("Contribution Cancelled.\n");
        return;
    }

    (void)pthread_mutex_lock(&dataMutex);
    goals[found].savedAmount += amount;
    saveGoals();
    (void)pthread_mutex_unlock(&dataMutex);

    printf("Contribution Added Successfully!\n");

    {
        char logMsg[80];

        snprintf(logMsg, sizeof(logMsg), "Contributed %.2f to Goal: %s", amount, goals[found].name);
        logActivity(logMsg);
    }

    if(goals[found].savedAmount >= goals[found].targetAmount)
        notifyGoalAchieved(goals[found].name);
}

void viewSavingsGoals(void)
{
    int i;

    if(goalCount == 0)
    {
        printf("No Savings Goals Set\n");
        return;
    }

    printf("\n--------------------------------------------------------------------------------\n");
    printf("%-20s%-14s%-14s%s\n", "GOAL", "TARGET", "SAVED", "PROGRESS");
    printf("--------------------------------------------------------------------------------\n");

    for(i = 0; i < goalCount; i++)
    {
        float progress = 0;

        if(goals[i].targetAmount > 0)
            progress = (goals[i].savedAmount / goals[i].targetAmount) * 100;

        printf("%-20s%-14.2f%-14.2f%.2f%%\n",
               goals[i].name,
               goals[i].targetAmount,
               goals[i].savedAmount,
               progress);
    }
}
