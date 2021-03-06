/*****************************************************************************
 *                                                                           *
 *                        <Routing - AStar Algorithm>                        *
 *             Copyright (C) <2018>   <Municoy, M., Salgado, D.>             *
 *                                                                           *
 *   Contact the authors at: mail@martimunicoy.com                           *
 *                           daniel.salgado@e-campus.uab.cat                 *
 *                                                                           *
 *   This program is free software: you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation, either version 3 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *****************************************************************************/

// Include Libraries
#include "routing.h"
#include "util.h"
#include "constants.h"
#include "graph.h"

/*
void RemoveNewLine(char* string)
{
    //Source: https://stackoverflow.com/questions/2693776/removing-trailing-new
    //                line-character-from-fgets-input/28462221
    size_t length;
    if( (length =strlen(string) ) >0)
        if(string[length-1] == '\n')
            string[length-1] ='\0';
}
*/

void CopyString(char *target, char *source)
{
    /*
    Source: https://www.w3schools.in/c-program/copy-string-using-pointers/
    */
    while(*source)
    {
        *target = *source;
        source++;
        target++;
    }
    *target = '\0';
}

bool EqualStrings(const char *s1, const char *s2)
{
    return strcmp(s1, s2) == 0;
}

bool StartsWith(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

bool EndsWith(const char *suffix, const char *str)
{
  int str_len = strlen(str);
  int suffix_len = strlen(suffix);

  return (str_len >= suffix_len) &&
         (0 == strcmp(str + (str_len-suffix_len), suffix));
}

char *SplitFields(char *str, char const *delims)
{
    /*
    Source: https://stackoverflow.com/questions/8705844/need-to-know-when-no-da
                    ta-appears-between-two-token-separators-using-strtok
    Source2: https://stackoverflow.com/questions/30294129/i-need-a-mix-of-strto
                     k-and-strtok-single/30295426#30295426
    */
    static char  *src = NULL;
    char  *p,  *ret = 0;

    if (str != NULL)
        src = str;

    if (src == NULL || *src == '\0')
        return NULL;

    ret = src;
    if ((p = strpbrk(src, delims)) != NULL)
    {
        *p  = 0;
        src = ++p;
    }
    else
        src += strlen(src);

    return ret;
}

int FieldsCounter(const char str[], char delimiter[])
{
    char *array_ptr = strdup(str);
    char *fields = SplitFields(array_ptr, delimiter);
    int n_fields = 0;

    while (fields)
    {
        n_fields++;
        fields = SplitFields(NULL, delimiter);
    }

    free(array_ptr);

    return n_fields;
}

char *GetField(const char *str, char delimiter[], int field_num)
{
    int i;
    char *array_ptr = strdup(str);
    char *fields = SplitFields(array_ptr, delimiter);

    for (i = 0; i < field_num; i++)
        fields = SplitFields(NULL, delimiter);

    return fields;
}

//@ToDo
bool ParseYesNo()
{
    printf("   Yes[Y] or No[N]? ");

    char ans[100];
    if( scanf("%s", ans) == EOF )
        ExitError("when scaning 'ans' in ParseYesNo function", -10);

    if (StartsWith(ans, "Yes") || StartsWith(ans, "yes"))
        return true;
    else if (StartsWith(ans, "No") || StartsWith(ans, "no"))
        return false;

    return ParseYesNo();
}

void ExitError(char * message, int num)
{
    fprintf(stderr, "Error %d: %s\n", num, message);
    exit(num);
}

FILE *OpenFile(const char file_dir[], char mode[], int error_num)
{
    // Check if folders to file exist. Create required path otherwise.
    // Only if we want to write a file
    if (StartsWith("w", mode))
    {
        int i, j;
        char *folder = SplitFields(strdup(file_dir), "/");
        int n_folders = FieldsCounter(file_dir, "/") - 1;

        for (i = 0; i < n_folders; i++)
        {
            char *path = strdup(folder);
            for (j = 1; j <= i; j++)
                path = Concat(path, Concat("/", GetField(file_dir, "/", j)));

            if (access(path, F_OK) == -1)
            {
                char *file_name = GetField(file_dir, "/", n_folders);
                printf(" - Creating folder \'%s\' to store file \'%s\'\n",
                       path, file_name);
                mkdir(path, 0700);
            }

            free(path);
        }
    }

    // Open file
    FILE *f;
    f = fopen(file_dir, mode);

    // Check if a file was found. Maybe it is compressed.
    if (f == NULL)
    {
        char *dir = Concat(SplitFields(strdup(file_dir), "."), ".cmap");
        if (access(dir, F_OK) != -1)
        {
            Decompress(file_dir);
            free(dir);
            return OpenFile(file_dir, mode, error_num);
        }
        else
        {
            free(dir);
            ExitError("when reading input file", error_num);
        }
    }

    // If everything was okay, return file
    return f;

}

const char *SingleArgumentParser(int argc, char *argv[])
{
    if (argc == 1)
        return NULL;

    else if (argc != 2)
        ExitError("when parsing program arguments", 45);

    return argv[1];
}

char *Concat(const char *s1, const char *s2)
{
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);

    char *result = malloc(len1+len2+1);
    memcpy(result, s1, len1);
    memcpy(result+len1, s2, len2+1);

    return result;
}

void Compress(const char *bin_dir)
{
    printf("------------------------------------------------------------\n");
    printf("Compressing binary file...\n");
    printf("------------------------------------------------------------\n");

    char *command, *dir, *c1, *c2;

    // Run command to compress file
    command = Concat("gzip -f ", bin_dir);
    if( system(command) == -1)
        ExitError("when calling system() to compress a file, from Compress()" , -1);

    // Free memory
    free(command);

    // Run command to change file extension
    dir = Concat(SplitFields(strdup(bin_dir), "."), ".cmap");
    c1 = Concat(bin_dir, ".gz ");
    c2 = Concat("mv ", c1);
    command = Concat(c2, dir);
    if( system(command) == -1)
        ExitError("when calling system() to change file extension, from Compress()" , -1);

    // Print out success
    printf("------------------------------------------------------------\n");
    printf("Completed.\n");
    printf("------------------------------------------------------------\n\n");

    // Free memory
    free(dir);
    free(c1);
    free(c2);
    free(command);
}

void Decompress(const char *bin_dir)
{
    printf("------------------------------------------------------------\n");
    printf("Decompressing binary file...\n");
    printf("------------------------------------------------------------\n");

    char *command, *dir, *c1, *c2;

    // Run command to change file extension
    dir = Concat(bin_dir, ".gz");
    c1 = Concat(SplitFields(strdup(bin_dir), "."), ".cmap ");
    c2 = Concat("mv ", c1);
    command = Concat(c2, dir);
    if( system(command) == -1)
        ExitError("when calling system() to change file extension, from Decompress()" , -1);

    //Free memory
    free(dir);
    free(c1);
    free(c2);
    free(command);

    // Run command to decompress binary file
    command = Concat("gzip -d ", Concat(bin_dir, ".gz"));
    if( system(command) == -1)
        ExitError("when calling system() to decompress a file, from Decompress()" , -1);

    // Print out success
    printf("------------------------------------------------------------\n");
    printf("Completed.\n");
    printf("------------------------------------------------------------\n\n");

    // Free memory
    free(command);
}

double ToRadians(double degrees)
{
    return degrees * PI / 180;
}

unsigned int MakeAQuery(char query[], unsigned int min, unsigned int max)
{
    unsigned int choice;

    printf(" - %s\n", query);
    printf("   Enter your choice (%d-%d): ", min, max);

    char *p, input[100];
    while (fgets(input, sizeof(input), stdin))
    {
        choice = strtol(input, &p, 10);
        if (p == input || *p != '\n' || choice < min || choice > max)
            printf("   Wrong input. Enter your choice (%d-%d): ", min, max);
        else
            break;
    }

    printf("\n");
    return choice;
}

void PrintOutDistOptions()
{
    printf(" - Distance functions available:\n");
    printf("\t1: Haversine\n");
    printf("\t2: Spherical law of cosines\n");
    printf("\t3: Equirectangular approximation\n");
    printf("\t4: Haversine with variable Earth radius\n");
    printf("\t5: Spherical law of cosines with variable Earth radius\n");
    printf("\t6: Equirectangular approximation with variable Earth radius\n");
    printf("\t7: Zero distance (equal to 0.0)\n");
    printf("\t8: Uniform distance (equal to 1.0)\n");
    printf("\n");
}

void AStarWelcome()
{
    printf("------------------------------------------------------------\n");
    printf("AStar Algorithm execution\n");
    printf("------------------------------------------------------------\n");
}

void PrintOutIterationInfo(unsigned int current_iteration, double g, double h)
{
    printf("               +----------------------------+\n");
    printf("               | Iteration number %8u  |\n", current_iteration);
    printf("               +----------------------------+\n");
    printf("               | g(n) = %7.0f m           |\n", g);
    printf("               | h(n) = %7.0f m           |\n", h);
    printf("               | f(n) = %7.0f m           |\n", g + h);
    printf("               +----------------------------+\n\n");
}

void PrintOutResults(unsigned int current_iteration, double g, double h,
                     double AStar_CPU_time)
{
    printf(" - Optimal path found!\n\n");
    printf("               +----------------------------+\n");
    printf("               |       AStar Results        |\n");
    printf("               +----------------------------+\n");
    printf("               | Total iterations: %8u |\n", current_iteration);
    printf("               | Total CPU time: %7.2lf  s |\n", AStar_CPU_time);
    printf("               +----------------------------+\n");
    printf("               | g(n) = %7.0f m           |\n", g);
    printf("               | h(n) = %7.0f m           |\n", h);
    printf("               | f(n) = %7.0f m           |\n", g + h);
    printf("               +----------------------------+\n\n");
}

void PrintOutReaderCLUsage()
{
    printf("usage: bincreator.exe [file]\n");
    printf("optional arguments:\n");
    printf("\t[-o directory] [-f]\n");
    printf("\t[-h] to obtain more information\n");
    ExitError("wrong command-line argument", 356);
}

void PrintOutReaderCLHelp()
{
    printf("Usage:\n");
    printf("bincreator.exe [file] [...]\n");
    printf("List of command-line arguments:\n");
    printf(" - Mandatory arguments:\n");
    printf("\t[file]         path to a suitable input map file\n");
    printf(" - Optional arguments:\n");
    printf("\t[-o directory] path to the directory where the binary\n"
           "\t               file will be stored\n");
    printf("\t[-f]           do not minimize graph inconsistencies\n"
           "\t               (faster script but worse graph)\n");
    printf("\t[-h]           prints this message and exits\n");
    exit(0);
}
void SetDefaultReaderArgs(ReaderArguments *args)
{
    args->input_file = NULL;
    args->output_file = strdup(DEFAULT_BIN_DIR);
}

void ParseReaderArgs(int argc, char **argv, ReaderArguments *args)
{
    char *ptr, *optarg;
    if (argc > 1)
    {
        optarg = *(argv + 1);
        args->input_file = strdup(optarg);
    }
    else
        PrintOutReaderCLUsage();

    unsigned int i;
    for (i = 1; i < argc; i++)
    {
        if (strcmp(*(argv + i), "-h") == 0)
            PrintOutReaderCLHelp();
        else if (strcmp(*(argv + i), "-f") == 0)
            args->fast = true;
        else if (i + 1 < argc)
        {
            if (strcmp(*(argv + i), "-o") == 0)
            {
                optarg = *(argv + ++i);
                if (EndsWith("/", optarg))
                    args->output_file = Concat(optarg, "map.bin");
                else
                    args->output_file = Concat(optarg, "/map.bin");
            }
        }
        else if (i == 1)
            continue;
        else
            PrintOutReaderCLUsage();
    }
}

void CheckReaderArgs(ReaderArguments *args)
{
    if (args->input_file == NULL)
        PrintOutReaderCLUsage();
}

void PrintOutRoutingCLUsage()
{
    printf("usage: routing.exe [file] [-s id/lat,lon] [-e id/lat,lon]\n");
    printf("optional arguments:\n");
    printf("\t[-o directory] [-d number] [-w number]\n");
    printf("\t[-h] to obtain more information\n");
    ExitError("wrong command-line argument", 356);
}

void PrintOutRoutingCLHelp()
{
    printf("Usage:\n");
    printf("routing.exe [file] [-s id/lat,lon] [-e id/lat,lon] [...]\n");
    printf("List of command-line arguments:\n");
    printf(" - Mandatory arguments:\n");
    printf("\t[file]         path to a suitable input binary or\n");
    printf("\t               cmap file\n");
    printf("\t[-s id]        id of the starting node\n");
    printf("\t[-s lat,lon]   latitude and longitude coordinates of\n"
           "\t               the starting node\n");
    printf("\t[-e id]        id of the ending node\n");
    printf("\t[-e lat,lon]   latitude and longitude coordinates of\n"
           "\t               the ending node\n");
    printf(" - Optional arguments:\n");
    printf("\t[-o directory] path to the directory where the final\n"
           "\t               route information will be stored\n");
    printf("\t[-d number]    selection of the method to calculate\n"
           "\t               the heuristic distance\n");
    printf("\t[-w number]    selection of the method to calculate\n"
           "\t               the edges weight\n");
    printf("\t[-h]           prints this message and exits\n");
    void PrintOutDistOptions();
    exit(0);
}
void SetDefaultRoutingArgs(RoutingArguments *args)
{
    // Mandatory arguments
    args->input_file = NULL;
    args->starting_point = 0;
    args->ending_point = 0;

    // Optional arguments
    args->output_file = NULL;
    args->heuristic_method = 0;
    args->weight_method = 0;
}

void ParseRoutingArgs(int argc, char **argv, RoutingArguments *args)
{
    char *ptr, *optarg;
    if (argc > 1)
    {
        optarg = *(argv + 1);
        args->input_file = strdup(optarg);
    }
    else
        PrintOutRoutingCLUsage();

    unsigned int i;
    for (i = 1; i < argc; i++)
    {
        if (strcmp(*(argv + i), "-h") == 0)
            PrintOutRoutingCLHelp();
        else if (i + 1 < argc)
        {
            if (strcmp(*(argv + i), "-o") == 0)
            {
                optarg = *(argv + ++i);
                if (EndsWith("/", optarg))
                    args->output_file = Concat(optarg, "path.out");
                else
                    args->output_file = Concat(optarg, "/path.out");
                continue;
            }
            else if (strcmp(*(argv + i), "-s") == 0)
            {
                optarg = *(argv + ++i);
                args->starting_point = strdup(optarg);
                continue;
            }
            else if (strcmp(*(argv + i), "-e") == 0)
            {
                optarg = *(argv + ++i);
                args->ending_point = strdup(optarg);
                continue;
            }
            else if (strcmp(*(argv + i), "-d") == 0)
            {
                optarg = *(argv + ++i);
                args->heuristic_method = strtoul(optarg, &ptr , 10);
                continue;
            }
            else if (strcmp(*(argv + i), "-w") == 0)
            {
                optarg = *(argv + ++i);
                args->weight_method = strtoul(optarg, &ptr , 10);
                continue;
            }
        }
        else if (i == 1)
            continue;
        else
            PrintOutRoutingCLUsage();
    }
}

void CheckRoutingArgs(RoutingArguments *args)
{
    if (args->input_file == NULL)
        PrintOutRoutingCLUsage();
    if (args->starting_point == 0)
        PrintOutRoutingCLUsage();
    if (args->ending_point == 0)
        PrintOutRoutingCLUsage();
}

double QuadranceDistance(double x, double y, double x0, double y0)
{
    return pow((x - x0), 2) + pow((y - y0), 2);
}
