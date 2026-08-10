#include <stdio.h>
#include <string.h>

#include "investment.h"
#include "common.h"
#include "persistence.h"
#include "utility.h"

typedef struct
{
    char name[30];
    char examples[80];
    char description[150];
    char duration[40];
    char expectedReturns[40];
    int hasDisclaimer;
    float allocationPercent;
} InvestmentOption;

/* Illustrative reference data for a POC - not real market rates.
   Percentages are a simple rule-of-thumb split, not personalized
   financial advice (see printDisclaimer). */
static const InvestmentOption investmentOptions[4] =
{
    {
        "Mutual Funds",
        "e.g. Groww, Angel One, 5paisa",
        "A pooled investment professionally managed across stocks "
        "and bonds. Value moves with the market.",
        "3-5 years (long-term)",
        "~10-12% p.a. (not guaranteed)",
        1,
        50.0f
    },
    {
        "Gold",
        "e.g. Paytm Gold",
        "Digital or physical gold, often used as a hedge against "
        "inflation. Value tracks the market gold price.",
        "1-3 years",
        "~7-9% p.a. (not guaranteed)",
        1,
        20.0f
    },
    {
        "Health Insurance",
        "e.g. PolicyBazaar, Star Health Plan",
        "A policy that covers medical expenses in exchange for a "
        "periodic premium. Protects savings from medical emergencies.",
        "Ongoing (annual renewal)",
        "N/A - protection, not growth",
        0,
        10.0f
    },
    {
        "Emergency Fund",
        "Personal Savings",
        "Liquid personal savings set aside for unexpected expenses. "
        "Not invested in the market - available anytime.",
        "N/A - always liquid",
        "~3-4% p.a. (savings-equivalent)",
        0,
        20.0f
    }
};

static void printDisclaimer(void)
{
    printf("\n----------------------------------------\n");
    printf(" Disclaimer\n");
    printf("----------------------------------------\n");
    printf("Market-linked investments carry risk and returns are\n");
    printf("not guaranteed. Figures shown are illustrative only\n");
    printf("(POC placeholders), not financial advice. Please\n");
    printf("consult a certified financial advisor before investing.\n");
    printf("----------------------------------------\n");
}

static void emergencyFundMenu(void)
{
    int option;
    float amount;

    while(1)
    {
        printf("\n----------------------------------------\n");
        printf(" Emergency Fund - Balance: %.2f\n", emergencyFundBalance);
        printf("----------------------------------------\n");
        printf("1. Deposit\n");
        printf("2. Withdraw\n");
        printf("3. Back\n");
        printf("Enter Choice: ");

        if(!readValidInt(&option))
            continue;

        if(option == 1)
        {
            printf("Enter Amount to Deposit (or -1 to cancel): ");

            if(!readValidFloat(&amount))
                continue;

            if(amount == -1)
            {
                printf("Deposit Cancelled.\n");
                continue;
            }

            if(amount <= 0)
            {
                printf("Amount must be greater than zero.\n");
                continue;
            }

            emergencyFundBalance += amount;
            saveEmergencyFund();
            printf("Deposited Successfully! New Balance: %.2f\n", emergencyFundBalance);
        }
        else if(option == 2)
        {
            printf("Enter Amount to Withdraw (or -1 to cancel): ");

            if(!readValidFloat(&amount))
                continue;

            if(amount == -1)
            {
                printf("Withdrawal Cancelled.\n");
                continue;
            }

            if(amount <= 0)
            {
                printf("Amount must be greater than zero.\n");
                continue;
            }

            if(amount > emergencyFundBalance)
            {
                printf("Insufficient Balance! Current Balance: %.2f\n", emergencyFundBalance);
                continue;
            }

            emergencyFundBalance -= amount;
            saveEmergencyFund();
            printf("Withdrawn Successfully! New Balance: %.2f\n", emergencyFundBalance);
        }
        else if(option == 3)
        {
            return;
        }
        else
        {
            printf("Invalid Choice\n");
        }
    }
}

static void printInvestmentDetail(int index)
{
    const InvestmentOption *opt = &investmentOptions[index];

    printf("\n========================================\n");
    printf(" %s\n", opt->name);
    printf("========================================\n");
    printf("Examples         : %s\n", opt->examples);
    printf("How it works     : %s\n", opt->description);
    printf("Typical Duration : %s\n", opt->duration);
    printf("Expected Returns : %s\n", opt->expectedReturns);

    if(opt->hasDisclaimer)
        printDisclaimer();

    /* Only Emergency Fund (index 3) gets deposit/withdraw access,
       since it's the one liquid category users actively manage. */
    if(index == 3)
        emergencyFundMenu();
}

static void viewInvestmentCategories(void)
{
    int choice;

    while(1)
    {
        printf("\n----------------------------------------\n");
        printf(" Investment Categories\n");
        printf("----------------------------------------\n");
        printf("1. Mutual Funds\n");
        printf("2. Gold\n");
        printf("3. Health Insurance\n");
        printf("4. Emergency Funds\n");
        printf("5. Back\n");
        printf("Enter Choice: ");

        if(!readValidInt(&choice))
            continue;

        if(choice >= 1 && choice <= 4)
            printInvestmentDetail(choice - 1);
        else if(choice == 5)
            return;
        else
            printf("Invalid Choice\n");
    }
}

static void getInvestmentRecommendation(void)
{
    float amount;
    int i;
    int hasMarketLinked = 0;
    char shareStr[16];

    printf("Enter Amount to Invest (or -1 to cancel): ");

    if(!readValidFloat(&amount))
        return;

    if(amount == -1)
    {
        printf("Cancelled.\n");
        return;
    }

    if(amount <= 0)
    {
        printf("Amount must be greater than zero.\n");
        return;
    }

    printf("\n========================================\n");
    printf(" Suggested Allocation for %.2f\n", amount);
    printf("========================================\n");
    printf("%-18s%-10s%-12s%-26s%s\n",
           "CATEGORY", "SHARE", "AMOUNT", "DURATION", "EXPECTED RETURNS");
    printf("--------------------------------------------------------------------------------\n");

    for(i = 0; i < 4; i++)
    {
        float share = amount * investmentOptions[i].allocationPercent / 100.0f;

        snprintf(shareStr, sizeof(shareStr), "%.0f%%", investmentOptions[i].allocationPercent);

        printf("%-18s%-10s%-12.2f%-26s%s\n",
               investmentOptions[i].name,
               shareStr,
               share,
               investmentOptions[i].duration,
               investmentOptions[i].expectedReturns);

        if(investmentOptions[i].hasDisclaimer)
            hasMarketLinked = 1;
    }

    printf("--------------------------------------------------------------------------------\n");
    printf("This is a simplified rule-of-thumb split for illustration (POC)\n");
    printf("purposes only, not personalized financial advice.\n");

    if(hasMarketLinked)
        printDisclaimer();
}

void investmentMenu(void)
{
    int choice;

    while(1)
    {
        printf("\n========================================\n");
        printf(" INVESTMENT\n");
        printf("========================================\n");
        printf("1. View Investment Categories\n");
        printf("2. Get Recommendation Based on Amount\n");
        printf("3. Back to Main Menu\n");
        printf("Enter Choice: ");

        if(!readValidInt(&choice))
            continue;

        switch(choice)
        {
            case 1:
                viewInvestmentCategories();
                break;

            case 2:
                getInvestmentRecommendation();
                break;

            case 3:
                return;

            default:
                printf("Invalid Choice\n");
        }
    }
}
