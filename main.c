#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "utility.h"
#include "authentication.h"
#include "persistence.h"
#include "income.h"
#include "expense.h"
#include "transaction.h"
#include "budget.h"
#include "savings_goal.h"
#include "dashboard.h"
#include "reports.h"
#include "recommendation.h"
#include "investment.h"
#include "activity_log.h"
#include "logger.h"
#include "autosave.h"

int main(void)
{
    int choice;

    /* Force output to appear immediately, rather than sitting in a
       buffer until it happens to flush. Without this, some terminals
       (notably certain VS Code setups) can delay a prompt's text
       until after the next read, making it look like a field was
       skipped even though it wasn't. */
    setvbuf(stdout, NULL, _IONBF, 0);

    if(!logInit("data/app.log"))
        printf("Warning: could not open log file - continuing without file logging.\n");

    while(1)
    {
        printf("\n=====================================\n");
        printf(" DIGITAL PERSONAL FINANCE PLATFORM\n");
        printf("=====================================\n");

        printf("1. Register\n");
        printf("2. Login\n");
        printf("3. Forgot Password\n");
        printf("4. Exit\n");

        printf("Enter Choice: ");

        if(!readValidInt(&choice))
            continue;

        switch(choice)
        {
            case 1:
                registerUser();
                break;

            case 3:
                forgotPassword();
                break;

            case 4:
                printf("Thank You!\n");
                stopAutosaveThread();
                LOG_INFO_MSG("user exited application normally");
                logClose();
                exit(0);

            case 2:
            {
                int result = loginUser();

                if(result == -1 || result == 0)
                {
                    break;
                }

                if(result == 1)
                {
                    int option;
                    char logMsg[80];

                    loadTransactions();
                    loadBudgets();
                    loadGoals();
                    loadEmergencyFund();

                    startAutosaveThread();

                    /* The activity log is one shared in-memory
                       buffer for the whole running process (see
                       activity_log.c) - without this reset, option
                       19 would keep showing every previous user's
                       history from this same run of the app, which
                       is a real privacy problem, not just clutter.
                       Resetting here means each login starts a
                       clean, this-session-only activity feed. */
                    resetActivityLog();

                    snprintf(logMsg, sizeof(logMsg), "Logged in as %s", currentUser);
                    logActivity(logMsg);

                    while(1)
                    {
                        printf("\n=====================================\n");
                        printf("Welcome %s\n", currentUser);
                        printf("=====================================\n");

                        printf(" 1. Add Income\n");
                        printf(" 2. Add Expense\n");
                        printf(" 3. View Transactions\n");
                        printf(" 4. Edit Transaction\n");
                        printf(" 5. Delete Transaction\n");
                        printf(" 6. Search Transactions by Category\n");
                        printf(" 7. Set Budget\n");
                        printf(" 8. View Budgets\n");
                        printf(" 9. Set Savings Goal\n");
                        printf("10. Contribute to Savings Goal\n");
                        printf("11. View Savings Goals\n");
                        printf("12. Financial Dashboard\n");
                        printf("13. Category-wise Report\n");
                        printf("14. Monthly Report\n");
                        printf("15. Smart Recommendations\n");
                        printf("16. Investment\n");
                        printf("17. Change Password\n");
                        printf("18. Logout\n");
                        printf("19. Recent Activity\n");

                        printf("Enter Choice: ");

                        if(!readValidInt(&option))
                            continue;

                        switch(option)
                        {
                            case 1:
                                addIncome();
                                break;

                            case 2:
                                addExpense();
                                break;

                            case 3:
                                viewTransactions();
                                break;

                            case 4:
                                editTransaction();
                                break;

                            case 5:
                                deleteTransaction();
                                break;

                            case 6:
                                searchTransactionsByCategory();
                                break;

                            case 7:
                                setBudget();
                                break;

                            case 8:
                                viewBudgets();
                                break;

                            case 9:
                                setSavingsGoal();
                                break;

                            case 10:
                                contributeToGoal();
                                break;

                            case 11:
                                viewSavingsGoals();
                                break;

                            case 12:
                                financialDashboard();
                                break;

                            case 13:
                                categoryWiseReport();
                                break;

                            case 14:
                                monthlyReport();
                                break;

                            case 15:
                                generateRecommendations();
                                break;

                            case 16:
                                investmentMenu();
                                break;

                            case 17:
                                changePassword();
                                break;

                            case 18:
                                stopAutosaveThread();
                                saveTransactions();
                                saveBudgets();
                                saveGoals();
                                saveEmergencyFund();
                                printf("Logged Out Successfully!\n");
                                goto logout;

                            case 19:
                                printActivityLog();
                                break;

                            default:
                                printf("Invalid Choice\n");
                        }
                    }

logout:
                    ;
                }

                break;
            }

            default:
                printf("Invalid Choice\n");
        }
    }

    return 0;
}
