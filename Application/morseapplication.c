#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>  /* open */
#include <unistd.h> /* exit */
#include <errno.h>

#define MAX_LEN 63
#define NO_ERROR 0
#define EDIT_OPERAND 1
#define NOT_EDIT_OPERAND 0
#define CORRECT_RET_VAL_FROM_SCANF 1
#define ENABLE_CONVERSION_VALUE 1

#define DEVICE_FILE_NAME "/dev/morse_Dev" /* device file */

int operator_f_flag = 0;
int operator_s_flag = 0;
int operator_b_flag = 0;
int operator_e_flag = 0; /* enable flag */

/* prototipes */
int isOperand(const char *arg, const int editOp, int file_desc);
int error(char *msg, int fp);
int setOperand(int argc, char **argv, int i, int file_desc);
int getString(int argc, char **argv, char *str, int i, int file_desc);
int readFromFile(char *str, const char *path, int file_desc);
void printHelp();
int lenMessage(char *str, int maxSize);

/* main function */
int main(int argc, char **argv)
{

    char str[MAX_LEN + 1], c; /* final string with the entire message */
    int i = 0, err, end_flag = 0, cnt = 0, file_desc, result;

    file_desc = open(DEVICE_FILE_NAME, O_RDWR | O_SYNC); /* Opening device file */
    if (file_desc < 0)
    {
        return error("can't open device file: /dev/morse_Dev", -1);
    }

    switch (argc) /* handle wrong number of arguments */
    {
    case 0:
        return error("command", file_desc);
        break;
    case 1:
        return error("command", file_desc);
        break;
    default:
        break;
    }

    i = setOperand(argc, argv, 1, file_desc);

    if (operator_f_flag)
    {
        /* read from file */
        err = readFromFile(str, argv[i], file_desc);
        if (err != NO_ERROR)
            return err;
        if (operator_b_flag)
            printf("%s\n", str);
        /* write string into device file */
        result = write(file_desc, (void *)str, lenMessage(str, MAX_LEN + 1));
        if (result != 0)
            return error("something went wrong writing the device file", file_desc);
    }
    else if (operator_s_flag)
    {
        /* read from command line */
        err = getString(argc, argv, str, i, file_desc);
        if (err != NO_ERROR)
            return err;
        if (operator_b_flag)
            printf("%s\n", str);
        /* write string into device file */
        result = write(file_desc, (void *)str, lenMessage(str, MAX_LEN + 1));
        if (result != 0)
            return error("something went wrong writing the device file", file_desc);
    }

    if (operator_e_flag)
    {
        /* start conversion writing command into device file */
        result = unlocked_ioctl(NULL, ENABLE_CONVERSION_VALUE, 0);
        if (result != 0)
            return error("something went wrong writing the device file", file_desc);
    }

    close(file_desc); /* close device file */
    return 0;
}

/* gets length of the string */
int lenMessage(char *str, int maxSize)
{
    int i = 0;
    for (i = 0; i < maxSize && str[i] != '\n' && str[i] != '\0'; i++)
        ;
    return i;
}

/* sets flags for operands */
int isOperand(const char *arg, const int editOp, int file_desc)
{
    regex_t regex;
    if (regcomp(&regex, "^-.*", 0) != 0)
    {
        return -1;
    }

    int result = regexec(&regex, arg, 0, NULL, 0);
    regfree(&regex);

    if (result == 0 && editOp) /* set all flags based on the operand */
    {
        if (strcmp(arg, "-f") == 0 || strcmp(arg, "--file") == 0)
        {
            operator_f_flag = 1;
        }
        else if (strcmp(arg, "-s") == 0 || strcmp(arg, "--string") == 0)
        {
            operator_s_flag = 1;
        }
        else if (strcmp(arg, "-b") == 0 || strcmp(arg, "--back") == 0)
        {
            operator_b_flag = 1;
        }
        else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
        {
            printHelp();
        }
        else if (strcmp(arg, "-e") == 0 || strcmp(arg, "--enable") == 0)
        {
            operator_e_flag = 1;
        }
        else
        {
            return error("operand not found", file_desc);
        }
        return 1;
    }

    return (result == 0);
}

/* function that prints error messages */
int error(char *msg, int fp)
{
    printf("morse: %s error\n", msg);
    if (fp != -1)
        printf("Try \'morse --help\' for more information.\n");
    if (fp != -1)
        close(fp);
    return 1;
}

/* set all operands */
int setOperand(int argc, char **argv, int i, int file_desc)
{
    /* handle operand */
    for (; i < argc && isOperand(argv[i], EDIT_OPERAND, file_desc); i++)
        ;
    return i;
}

int getString(int argc, char **argv, char *str, int i, int file_desc)
{
    /* handle string */
    for (; i < argc; i++)
    {
        if (isOperand(argv[i], NOT_EDIT_OPERAND, file_desc))
        {
            return error("operator", file_desc);
        }
        if ((strlen(argv[i]) + strlen(str)) <= MAX_LEN)
        {
            strcat(str, argv[i]); /* adding word to string in tail */
            if (i != (argc - 1))
                strcat(str, " "); /* adding space */
        }
        else
        {
            return error("string length", file_desc); /* max length exceeded */
        }
    }
    /* no string provided in input */
    if (strlen(str) == 0)
    {
        return error("no string", file_desc);
    }
    return NO_ERROR;
}

/* function that read a string from file */
int readFromFile(char *str, const char *path, int file_desc)
{
    FILE *file = fopen(path, "r"); /* opening file */

    if (file == NULL)
        return error("open file", file_desc); /* check file pointer */

    size_t bytesRead = fread(str, sizeof(char), MAX_LEN, file); /* read file */

    fclose(file); /* closing file */

    if (bytesRead == MAX_LEN)
    { /* check the lenght of the string read from file */
        return error("file length", file_desc);
    }

    str[bytesRead] = '\0'; /* end of the string */

    return NO_ERROR;
}

/* function that print help message */
void printHelp()
{
    printf("DESCRIPTION:\n"
           "  The \"morse\" application is a simple tool that allows you to convert text to Morse code and play it from the PYNQ Z2 board.\n\n"
           "USAGE:\n"
           "  morse [OPTIONS] [TEXT]\n\n"
           "OPTIONS:\n"
           "  -h, --help            Display this help message and exit.\n"
           "  -f, --file            Message from file given the path.\n"
           "  -s, --string          Message from string sent through command line.\n"
           "  -b, --back            Message printed back.\n"
           "  -e, --enable          Start message conversion.\n"

           "EXAMPLES:\n"
           "  Convert text to Morse from file:\n"
           "    morse --file --enable ./home/test_file.txt\n\n"
           "  Convert text to Morse from command line:\n"
           "    morse --string --enable your string here\n\n"
           "  Convert text to Morse from file and print it back:\n"
           "    morse --file --back --enable ./home/test_file.txt\n\n"
           "  Convert text to Morse from command line and print it back:\n"
           "    morse --string --enable --back your string here\n\n"
           "  Write text from command line and print it back:\n"
           "    morse --string --back your string here\n\n"
           "  Start message conversion written before:\n"
           "    morse --enable\n\n"
           "NOTES:\n"
           "  - The path cannot contain spaces.\n"
           "  - The maximum length of the string or the file content is 63 characters.\n"
           "AUTHOR:\n"
           "  Group 1 <Politecnico di Torino>\n");
}