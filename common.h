#ifndef COMMON_H
#define COMMON_H

#include <pthread.h>

/* =====================================================
   Shared structs, limits, and global data used by
   every module. Each module includes this to access
   the data it needs, but owns none of it directly -
   the actual storage lives in globals.c

   Threading: dataMutex guards every write to the shared
   arrays/counters below (transactions, budgets, goals,
   emergencyFundBalance) against the background autosave
   thread (see autosave.h). Every function that mutates
   one of these - addIncome, addExpense, editTransaction,
   deleteTransaction, setBudget, setSavingsGoal,
   contributeToGoal - takes dataMutex around the mutation
   and the immediate save*() call. Read-only functions
   (viewTransactions, reports, dashboard, ...) don't need
   it: the app is otherwise single-threaded on the main
   thread, so a read there can never run concurrently with
   another main-thread write, and the autosave thread only
   ever *reads* these arrays (to write them to disk) while
   holding the same mutex - so an unlocked read here never
   races with anything.
   ===================================================== */

#define MAX 1000
#define MAX_BUDGETS 100
#define MAX_GOALS 50

typedef struct
{
    char username[30];
    char password[30];
} User;

typedef struct
{
    int id;
    char type[20];
    char category[30];
    float amount;
    char date[11];
} Transaction;

typedef struct
{
    char category[30];
    float limit;
} Budget;

typedef struct
{
    char name[50];
    float targetAmount;
    float savedAmount;
} SavingsGoal;

extern Transaction transactions[MAX];
extern int transactionCount;

extern Budget budgets[MAX_BUDGETS];
extern int budgetCount;

extern SavingsGoal goals[MAX_GOALS];
extern int goalCount;

extern char currentUser[30];

extern float emergencyFundBalance;

extern pthread_mutex_t dataMutex;

#endif
