#ifndef TRANSACTION_H
#define TRANSACTION_H

void viewTransactions(void);
void editTransaction(void);
void deleteTransaction(void);
void searchTransactionsByCategory(void);

/* Sorts transactions[] in place, oldest first, using the
   standard library's qsort() - O(n log n). Called by the
   Reports module before generating output so reports read
   chronologically rather than in raw insertion order. */
void sortTransactionsByDate(void);

/* Sorts transactions[] in place, smallest amount first.
   Not currently wired into any menu option - available for
   future use (e.g. "largest expenses" views) and exercised
   directly by its own test. */
void sortTransactionsByAmount(void);

#endif
