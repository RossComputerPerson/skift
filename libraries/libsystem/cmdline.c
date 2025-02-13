/* Copyright © 2018-2019 N. Van Bossuyt.                                      */
/* This code is licensed under the MIT License.                               */
/* See: LICENSE.md                                                            */

/* cmdline.c: skiftOS command line application utils                          */

#include <libsystem/cstring.h>
#include <libsystem/iostream.h>
#include <libsystem/cmdline.h>
#include <libsystem/process.h>
#include <libsystem/convert.h>

/* --- Private functions ---------------------------------------------------- */

cmdline_option_t *get_long_option(cmdline_t *cmdline, const char *name)
{
    for (int i = 0; cmdline->options[i].type != CMDLINE_END; i++)
        if (strcmp(cmdline->options[i].long_name, name) == 0)
            return &cmdline->options[i];

    return NULL;
}

cmdline_option_t *get_short_option(cmdline_t *cmdline, const char name)
{
    for (int i = 0; cmdline->options[i].type != CMDLINE_END; i++)
        if (cmdline->options[i].short_name == name)
            return &cmdline->options[i];

    return NULL;
}

void do_option(cmdline_t *cmdline, cmdline_option_t *option, int i, int argc, char **argv)
{
    switch (option->type)
    {
    case CMDLINE_BOOLEAN:
    {
        if (option->value != NULL)
        {
            *((bool *)option->value) = true;
        }

        break;
    }
    case CMDLINE_STRING:
    {
        if (option->value != NULL)
        {
            if (i + 1 < argc && argv[i + 1] != NULL)
            {
                *((char **)option->value) = argv[i + 1];
                argv[i + 1] = NULL;
            }
        }

        break;
    }
    case CMDLINE_INTEGER:
    {
        if (option->value != NULL)
            if (i + 1 < argc && argv[i + 1] != NULL)
            {
                *((int *)option->value) = convert_string_to_uint(argv[i + 1], strnlen(argv[i + 1], 33), 10);
                argv[i + 1] = NULL;
            }

        break;
    }

    default:
        break;
    }

    if (option->callback != NULL)
    {
        option->callback(cmdline, option);
    }
}

void cmdline_callback_help(cmdline_t *cmdline, cmdline_option_t *option)
{
    UNUSED(option);

    if (cmdline->prologue)
    {
        printf(cmdline->prologue);
        printf("\n\n");
    }

    if (cmdline->usages != NULL)
    {
        printf("\e[1mUsages:\e[0m ");

        for (int i = 0; cmdline->usages[i]; i++)
        {
            printf("%s %s\n\t", cmdline->name, cmdline->usages[i]);
        }

        printf("\n");
    }

    printf("\e[1mOptions:\e[0m");
    for (int i = 0; cmdline->options[i].type != CMDLINE_END; i++)
    {
        cmdline_option_t *opt = &cmdline->options[i];
        if (opt->type == CMDLINE_SEPARATOR)
        {
            //printf("\n");
        }
        else if (opt->type == CMDLINE_SECTION)
        {
            printf("\e[1m%s:\e[0m", opt->long_name);
        }
        else
        {

            if (opt->short_name != '\0')
            {
                printf(" -%c", opt->short_name);
            }
            else
            {
                printf("    ");
            }

            if (opt->long_name != NULL)
            {
                printf(", --%s ", opt->long_name);
            }
            else
            {
                printf("  ");
            }

            if (opt->type == CMDLINE_STRING ||
                opt->type == CMDLINE_INTEGER)
            {
                printf("[value] ");
            }

            if (opt->long_name == NULL)
            {
                printf("  ");
            }

            printf("\t");

            if (opt->help != NULL)
            {
                printf("%s", opt->help);
            }
        }

        printf("\n\t");
    }
    printf("\n");

    if (cmdline->epiloge)
    {
        printf(cmdline->epiloge);
        printf("\n");
    }

    process_exit(0);
}

/* --- Public functions ----------------------------------------------------- */

int cmdline_parse(cmdline_t *cmdline, int argc, char **argv)
{
    cmdline->name = argv[0];

    // Parsing arguments
    for (int i = 1; i < argc; i++)
    {
        char *arg = argv[i];

        if (arg[0] == '-')
        {
            cmdline_option_t *opt = NULL;

            if (arg[1] == '-')
            {
                opt = get_long_option(cmdline, arg + 2);

                if (opt != NULL)
                {
                    do_option(cmdline, opt, i, argc, argv);
                }
                else
                {
                    iostream_printf(err_stream, "Unknow option '%s'!\n", arg + 2);
                    process_exit(-1);
                }
            }
            else
            {
                for (int j = 0; arg[1 + j]; j++)
                {
                    opt = get_short_option(cmdline, arg[1 + j]);

                    if (opt != NULL)
                    {
                        do_option(cmdline, opt, i, argc, argv);
                    }
                    else
                    {
                        iostream_printf(err_stream, "Unknow option '%c'!\n", arg[1 + j]);
                        process_exit(-1);
                    }
                }
            }

            argv[i] = NULL;
        }
    }

    // Packing argv
    int index = 0;

    for (int i = 0; i < argc; i++)
    {
        if (argv[i] != NULL)
        {
            argv[index] = argv[i];
            index++;
        }
    }

    return index;
}