#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "authentication.h"
#include "common.h"
#include "utility.h"
#include "logger.h"
#include "activity_log.h"

/* Checks: at least 8 characters, with at least one uppercase,
   one lowercase, one digit, and one symbol. Not exposed outside
   this file - only registerUser/forgotPassword/changePassword
   need it. */
static int isPasswordValid(const char *password)
{
    int hasUpper = 0, hasLower = 0, hasDigit = 0, hasSymbol = 0;
    size_t len = strlen(password);
    size_t i;

    if(len < 8)
        return 0;

    for(i = 0; i < len; i++)
    {
        unsigned char c = (unsigned char)password[i];

        if(isupper(c))
            hasUpper = 1;
        else if(islower(c))
            hasLower = 1;
        else if(isdigit(c))
            hasDigit = 1;
        else if(ispunct(c))
            hasSymbol = 1;
    }

    return hasUpper && hasLower && hasDigit && hasSymbol;
}

static void printPasswordRequirements(void)
{
    printf("\n----------------------------------------\n");
    printf(" Password Requirements\n");
    printf("----------------------------------------\n");
    printf(" - At least 8 characters\n");
    printf(" - At least 1 uppercase letter (A-Z)\n");
    printf(" - At least 1 lowercase letter (a-z)\n");
    printf(" - At least 1 digit (0-9)\n");
    printf(" - At least 1 symbol (e.g. ! @ # $ %%)\n");
    printf("----------------------------------------\n\n");
}

void registerUser(void)
{
    User user, temp;

    printf("Enter Username (or 0 to cancel): ");
    readLine(user.username, sizeof(user.username));

    if(strcmp(user.username, "0") == 0)
    {
        printf("Registration Cancelled.\n");
        return;
    }

    FILE *fp = fopen("data/users.dat", "ab+");

    if(fp == NULL)
    {
        LOG_ERROR_MSG("could not open data/users.dat for registration");
        printf("Error Opening Users File\n");
        return;
    }

    rewind(fp);

    while(fread(&temp, sizeof(User), 1, fp))
    {
        if(strcmp(temp.username, user.username) == 0)
        {
            printf("Username Already Exists!\n");
            fclose(fp);
            return;
        }
    }

    printPasswordRequirements();

    while(1)
    {
        printf("Enter Password (or 0 to cancel): ");
        readLine(user.password, sizeof(user.password));

        if(strcmp(user.password, "0") == 0)
        {
            printf("Registration Cancelled.\n");
            fclose(fp);
            return;
        }

        if(isPasswordValid(user.password))
            break;

        printf("Password does not meet the requirements. Please try again.\n");
    }

    fwrite(&user, sizeof(User), 1, fp);

    fclose(fp);

    char filename[60];

    snprintf(filename, sizeof(filename), "data/%s_transactions.dat", user.username);

    FILE *userFile = fopen(filename, "wb");

    if(userFile != NULL)
    {
        int count = 0;
        fwrite(&count, sizeof(int), 1, userFile);
        fclose(userFile);
    }

    printf("Account Created Successfully!\n");
}

int loginUser(void)
{
    User user;

    char username[30];
    char password[30];

    FILE *fp = fopen("data/users.dat", "rb");

    if(fp == NULL)
    {
        LOG_ERROR_MSG("failed to open data/users.dat - no users registered yet");
        printf("\nNo Users Registered Yet!\n");
        printf("Please Register First.\n");
        return -1;
    }

    printf("Enter Username (or 0 to cancel): ");
    readLine(username, sizeof(username));

    if(strcmp(username, "0") == 0)
    {
        printf("Login Cancelled.\n");
        fclose(fp);
        return -1;
    }

    printf("Enter Password (or 0 to cancel): ");
    readLine(password, sizeof(password));

    if(strcmp(password, "0") == 0)
    {
        printf("Login Cancelled.\n");
        fclose(fp);
        return -1;
    }

    while(fread(&user, sizeof(User), 1, fp))
    {
        if(strcmp(user.username, username) == 0 &&
           strcmp(user.password, password) == 0)
        {
            strcpy(currentUser, username);

            fclose(fp);

            {
                char logMsg[80];

                snprintf(logMsg, sizeof(logMsg), "user '%s' logged in successfully", username);
                LOG_INFO_MSG(logMsg);
            }

            printf("Login Successful!\n");

            return 1;
        }
    }

    fclose(fp);

    {
        char logMsg[80];

        snprintf(logMsg, sizeof(logMsg), "failed login attempt for username '%s'", username);
        LOG_WARN_MSG(logMsg);
    }

    printf("Invalid Username or Password!\n");

    return 0;
}

void forgotPassword(void)
{
    User user;
    char username[30];
    int found = 0;

    FILE *fp = fopen("data/users.dat", "rb+");

    if(fp == NULL)
    {
        printf("\nNo Users Registered Yet!\n");
        return;
    }

    printf("Enter Username (or 0 to cancel): ");
    readLine(username, sizeof(username));

    if(strcmp(username, "0") == 0)
    {
        printf("Cancelled.\n");
        fclose(fp);
        return;
    }

    while(fread(&user, sizeof(User), 1, fp) == 1)
    {
        if(strcmp(user.username, username) == 0)
        {
            found = 1;
            break;
        }
    }

    if(!found)
    {
        printf("Username Not Found!\n");
        fclose(fp);
        return;
    }

    printPasswordRequirements();

    while(1)
    {
        printf("Enter New Password (or 0 to cancel): ");
        readLine(user.password, sizeof(user.password));

        if(strcmp(user.password, "0") == 0)
        {
            printf("Cancelled.\n");
            fclose(fp);
            return;
        }

        if(isPasswordValid(user.password))
            break;

        printf("Password does not meet the requirements. Please try again.\n");
    }

    fseek(fp, -(long)sizeof(User), SEEK_CUR);
    fwrite(&user, sizeof(User), 1, fp);

    fclose(fp);

    printf("Password Reset Successfully! You can now log in.\n");
}

void changePassword(void)
{
    User user;
    char oldPassword[30];
    int found = 0;

    FILE *fp = fopen("data/users.dat", "rb+");

    if(fp == NULL)
        return;

    while(fread(&user, sizeof(User), 1, fp) == 1)
    {
        if(strcmp(user.username, currentUser) == 0)
        {
            found = 1;
            break;
        }
    }

    if(!found)
    {
        printf("Account Record Not Found!\n");
        fclose(fp);
        return;
    }

    printf("Enter Current Password (or 0 to cancel): ");
    readLine(oldPassword, sizeof(oldPassword));

    if(strcmp(oldPassword, "0") == 0)
    {
        printf("Cancelled.\n");
        fclose(fp);
        return;
    }

    if(strcmp(user.password, oldPassword) != 0)
    {
        char logMsg[80];

        snprintf(logMsg, sizeof(logMsg), "incorrect current password entered by '%s'", currentUser);
        LOG_WARN_MSG(logMsg);

        printf("Incorrect Password!\n");
        fclose(fp);
        return;
    }

    printPasswordRequirements();

    while(1)
    {
        printf("Enter New Password (or 0 to cancel): ");
        readLine(user.password, sizeof(user.password));

        if(strcmp(user.password, "0") == 0)
        {
            printf("Cancelled.\n");
            fclose(fp);
            return;
        }

        if(isPasswordValid(user.password))
            break;

        printf("Password does not meet the requirements. Please try again.\n");
    }

    fseek(fp, -(long)sizeof(User), SEEK_CUR);
    fwrite(&user, sizeof(User), 1, fp);

    fclose(fp);

    printf("Password Changed Successfully!\n");

    logActivity("Changed Password");

    {
        char logMsg[80];

        snprintf(logMsg, sizeof(logMsg), "user '%s' changed their password", currentUser);
        LOG_INFO_MSG(logMsg);
    }
}
