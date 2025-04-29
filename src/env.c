/*
 * Copyright (c) 2002-2004 Viliam Holub
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 *  Environment variables
 *
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "device/cpu/mips_r4000/cpu.h"
#include "device/cpu/mips_r4000/debug.h"
#include "device/cpu/riscv_rv32ima/debug.h"
#include "env.h"
#include "fault.h"
#include "parser.h"
#include "utils.h"

/*
 * Instruction disassembling
 */
bool iaddr = true;
bool iopc = false;
bool icmt = true;
bool iregch = true;
unsigned int r4k_ireg = 2;
unsigned int __rv_ireg_mock = 1; // this variable is never written to,
                                 // but it has to be here, because of
                                 // the parsing algorithm

/*
 * Boolean constants
 */
static const char *t_bool[] = {
    "on",
    "true",
    "yes",
    "off",
    "false",
    "no",
    NULL
};

static const char *t_true_all[] = {
    "on",
    "t",
    "tr",
    "tru",
    "true",
    "y",
    "ye",
    "yes",
    NULL
};

static const char *t_false_all[] = {
    "off",
    "f",
    "fa",
    "fal",
    "fals",
    "false",
    "n",
    "no",
    NULL
};

/** Description of simulator environment variables.
 *
 * name  The name of a variable as it is visible to the user.
 * desc  Short description text. It is displayed as the help text to
 *       the variable.
 * descf Full description of the variable. Displayed when user asks
 *       for help about that variable.
 * type  Variable type - bool, int, str, etc.
 * val   Pointer where to store the value.
 * func  Function which is called to change a variable
 *
 * name == NULL means no other variables
 * name != NULL && val == NULL means this is not a real variable but
 *              a separator which is used to display a more user-friendly
 *              help output
 * func != NULL means that a func function have to be called instead direct
 *              write to the variable
 */
typedef struct {
    const char *const name; /**< Name of the variable */
    const char *const desc; /**< Brief textual description */
    const char *const descf; /**< Full textual description */
    var_type_t type; /**< Variable type */
    void *val; /**< Where to store a value */
    void *const func; /**< Function to be called */
} env_t;

#define LAST_ENV \
    { \
        NULL, \
        NULL, \
        NULL, \
        vt_uint, \
        NULL, \
        NULL \
    }

static const env_t *last_env;
static const char **last_bool;
static const env_t *last_benv;

typedef bool (*set_uint_t)(unsigned int);
typedef bool (*set_bool_t)(bool);
typedef bool (*set_str_t)(const char *);

/** Change the r4k_ireg variable
 *
 * @return true if successful
 *
 */
static bool change_ireg(unsigned int i)
{
    if (i >= R4K_REG_VARIANTS) {
        error("Index out of range 0..%u", R4K_REG_VARIANTS - 1);
        return false;
    }

    r4k_ireg = i;
    r4k_regname = r4k_reg_name[i];

    return true;
}

/*
 * Description of variables
 */
const env_t global_env[] = {
    { "runtime",
            "Run-time features",
            NULL,
            vt_uint,
            NULL,
            NULL },
    { "disassembling",
            "Disassembling features",
            NULL,
            vt_uint,
            NULL,
            NULL },
    { "iaddr",
            "Display instruction addresses",
            "Display address of each disassembled "
            "instruction. This feature is useful "
            "especially in conjuction with the trace "
            "variable.",
            vt_bool,
            &iaddr,
            NULL },
    { "iopc",
            "Display instruction opcodes",
            "Display instruction opcodes in hexa. Althrow an instruction "
            "opcode is not a human friendly representation, there are "
            "cases when the opcode can help, for example with debugging "
            "random writes.",
            vt_bool,
            &iopc,
            NULL },
    { "icmt",
            "Display instruction comments",
            "Display additional information about the disassembled "
            "instructions, e.g. hexa to decimal parameter conversion.",
            vt_bool,
            &icmt,
            NULL },
    { "iregch",
            "Display register changes",
            "Show registers which has been modified during instruction "
            "execution together with the previous and the new "
            "value.",
            vt_bool,
            &iregch,
            NULL },
    { "r4k_ireg",
            "MIPS Register name mode",
            "Mode 0 (technical): r0, r12, r22, etc.\n"
            "Mode 1 (at&t): $0, $12, $22, etc.\n"
            "Mode 2 (textual): at, t4, s2, etc.",
            vt_uint,
            &r4k_ireg,
            change_ireg },
    { "rv_ireg",
            "RISC-V Register name mode",
            "Mode 0 (numerical): x0, x12, x22, etc.\n"
            "Mode 1 (ABI): zero, sp, a0, s5, etc.\n",
            vt_uint,
            &__rv_ireg_mock, // unused
            rv32_debug_change_regnames },
    { "debugging",
            "Debugging features",
            NULL,
            vt_uint,
            NULL,
            NULL },
    { "trace",
            "Disassemble instructions as they are executed",
            "Disassemble and display instructions as they are executed.",
            vt_bool,
            &machine_trace,
            NULL },
    LAST_ENV
};

/** Print all variables and its states
 *
 */
static void print_all_variables(void)
{
    printf("[Group               ] [Variable] [Value   ]\n");

    const env_t *env;
    for (env = global_env; env->name; env++) {
        if (env->val) {
            /* Variable */
            printf("                       %-10s ", env->name);
            switch (env->type) {
            case vt_uint:
                printf("%u", *(unsigned int *) env->val);
                break;
            case vt_str:
                printf("%s", (const char *) env->val);
                break;
            case vt_bool:
                printf("%s", *(bool *) env->val ? "on" : "off");
                break;
            default:
                intr_error("Unexpected variable type");
            }
        } else {
            /* Label */
            printf("%s", env->desc);
        }
        printf("\n");
    }
}

/** Check whether the variable exists
 *
 */
bool env_check_varname(const char *name, var_type_t *type)
{
    if (name == NULL) {
        name = "";
    }

    bool ret = false;
    const env_t *env;

    for (env = global_env; env->name; env++) {
        if ((env->val) && (!strcmp(name, env->name))) {
            ret = true;
            if (type) {
                *type = env->type;
            }
            break;
        }
    }

    return ret;
}

/** Check whether the variable of a given type
 *
 */
bool env_check_type(const char *name, var_type_t type)
{
    if (name == NULL) {
        name = "";
    }

    bool ret = false;
    const env_t *env;

    for (env = global_env; env->name; env++) {
        if ((env->val) && (!strcmp(name, env->name))) {
            ret = (env->type == type);
            break;
        }
    }

    return ret;
}

unsigned int env_cnt_partial_varname(const char *name)
{
    if (name == NULL) {
        name = "";
    }

    const env_t *env;
    unsigned int cnt = 0;

    for (env = global_env; env->name; env++) {
        if ((env->val) && (prefix(name, env->name))) {
            cnt++;
        }
    }

    return cnt;
}

static bool env_by_partial_varname(const char *name, const env_t **envp)
{
    if (name == NULL) {
        name = "";
    }

    const env_t *env = ((envp) && (*envp)) ? (*envp + 1) : global_env;

    for (; env->name; env++) {
        if ((env->val) && (prefix(name, env->name))) {
            break;
        }
    }

    if ((env->name) && (envp)) {
        *envp = env;
    }

    return (!!env->name);
}

/** Search for the variable description
 *
 * SE: Also print an error message when
 * the variable name was not found.
 *
 * @param var_name The variable name.
 *
 * @return Variable descripion
 *
 */
static const env_t *search_variable(const char *name)
{
    ASSERT(name != NULL);

    const env_t *env;

    for (env = global_env; env->name; env++) {
        if ((env->val) && (!strcmp(name, env->name))) {
            break;
        }
    }

    if (!env->name) {
        error("Unknown variable \"%s\"", name);
        return NULL;
    }

    return env;
}

/** Print help text about variables
 *
 * There are two types of help. The short one when the user typed "set help"
 * prints just all variables with a short description. The long variant is
 * used when the user specify the variable name ("set help=var_name").
 *
 * @param parm Parameter points to the token following the "help" token.
 */
static void show_help(token_t *parm)
{
    ASSERT(parm != NULL);

    const env_t *env;

    if (parm_type(parm) == tt_end) {
        printf("[Group               ] [Variable] [Description\n");
        for (env = global_env; env->name; env++) {
            if (env->val) {
                /* Variable */
                printf("                       %-10s %s", env->name,
                        env->desc);
            } else {
                /* Label */
                printf("%s", env->desc);
            }
            printf("\n");
        }
    } else {
        /* Skip '=' constant */
        ASSERT(strcmp(parm_str(parm), "=") == 0);
        parm_next(&parm);

        env = search_variable(parm_str_next(&parm));
        if (env) {
            printf("%s\n", env->descf);
        }
    }
}

/** Set an integer variable
 *
 */
static bool set_uint(const env_t *env, token_t *parm)
{
    ASSERT(env != NULL);
    ASSERT(parm != NULL);

    if (parm_type(parm) != tt_uint) {
        error("Integer parameter expected");
        return false;
    }

    if (env->func) {
        ((set_uint_t) env->func)(parm_uint(parm));
    } else {
        *((unsigned int *) env->val) = parm_uint(parm);
    }

    return true;
}

/** Set a boolean variable
 *
 */
static bool set_bool(const env_t *env, token_t *parm)
{
    ASSERT(env != NULL);
    ASSERT(parm != NULL);

    bool val = !!parm_uint(parm);

    if (env->func) {
        ((set_bool_t) env->func)(val);
    } else {
        *((bool *) env->val) = val;
    }

    return true;
}

/** Set a string variable
 *
 */
static bool set_str(const env_t *env, token_t *parm)
{
    ASSERT(env != NULL);
    ASSERT(parm != NULL);

    if (env->func) {
        ((set_str_t) env->func)(parm_str(parm));
    } else {
        free(env->val);
        *((char **) env->val) = safe_strdup(parm_str(parm));
    }

    return true;
}

/** Convert a boolean string to integer
 *
 */
static bool bool_sanitize(token_t *parm)
{
    ASSERT(parm != NULL);

    const char **str;

    if (parm_type(parm) == tt_str) {
        /* Test for true */
        for (str = t_true_all; *str; str++) {
            if (!strcmp(parm_str(parm), *str)) {
                break;
            }
        }

        if (*str) {
            parm_set_uint(parm, 1);
            return true;
        }

        /* Test for false */
        for (str = t_false_all; *str; str++) {
            if (!strcmp(parm_str(parm), *str)) {
                break;
            }
        }

        if (*str) {
            parm_set_uint(parm, 0);
            return true;
        }
    }

    return false;
}

/** Set variable
 *
 */
static bool set_variable(const char *name, token_t *parm)
{
    ASSERT(parm != NULL);

    const env_t *env = search_variable(name);

    if (!env) {
        return false;
    }

    switch (env->type) {
    case vt_uint:
        return set_uint(env, parm);
    case vt_bool:
        if (!bool_sanitize(parm)) {
            error("Boolean parameter expected");
            return false;
        }
        return set_bool(env, parm);
    case vt_str:
        return set_str(env, parm);
    default:
        intr_error("Unexpected variable type");
        return false;
    }
}

/** Decode and set or unset a boolean variable
 *
 */
static bool set_bool_variable(const char *name, bool val)
{
    ASSERT(name != NULL);

    const env_t *env = search_variable(name);

    if (env) {
        token_t *token = safe_malloc_t(token_t);
        parm_init(token);
        parm_set_uint(token, val);

        bool ret = set_bool(env, token);

        safe_free(token);
        return ret;
    }

    return false;
}

/** Set command implementation
 *
 */
bool env_cmd_set(token_t *parm)
{
    ASSERT(parm != NULL);

    if (parm_type(parm) == tt_end) {
        /* Show all variables */
        print_all_variables();
        return true;
    }

    const char *name = parm_str_next(&parm);

    if (strcmp(name, "help") == 0) {
        /* Show help */
        show_help(parm);
        return true;
    }

    /* Implicit boolean? */
    if (parm_type(parm) == tt_end) {
        return set_bool_variable(name, true);
    }

    /* Skip '=' constant */
    ASSERT(strcmp(parm_str(parm), "=") == 0);
    parm_next(&parm);

    /* Set the variable */
    return set_variable(name, parm);
}

/** Unset command implementation
 *
 */
bool env_cmd_unset(token_t *parm)
{
    ASSERT(parm != NULL);

    const char *name = parm_str(parm);
    return set_bool_variable(name, false);
}

/** Generate a list of environment variables
 *
 * For TAB completion.
 *
 */
char *generator_env_name(token_t *parm, const void *data, unsigned int level)
{
    ASSERT(parm != NULL);
    ASSERT((parm_type(parm) == tt_str) || (parm_type(parm) == tt_end));

    /* Iterator initialization */
    if (level == 0) {
        last_env = NULL;
    }

    const char *name = (parm_type(parm) == tt_str) ? parm_str(parm) : "";

    /* Find next suitable entry */
    bool fnd = env_by_partial_varname(name, &last_env);

    if (fnd) {
        return safe_strdup(last_env->name);
    }

    return NULL;
}

/** Generate a list of boolean constants
 *
 * For TAB competion.
 *
 */
char *generator_env_booltype(token_t *parm, const void *data,
        unsigned int level)
{
    ASSERT(parm != NULL);
    ASSERT((parm_type(parm) == tt_str) || (parm_type(parm) == tt_end));

    /* Iterator initialization */
    if (level == 0) {
        last_bool = t_bool;
    } else {
        last_bool++;
    }

    /* Find next suitable entry */
    while (*last_bool) {
        if (prefix(parm_str(parm), *last_bool)) {
            break;
        }
        last_bool++;
    }

    if (*last_bool) {
        return safe_strdup(*last_bool);
    }

    return NULL;
}

/** Generate a list of boolean environment variables
 *
 * For TAB competion.
 *
 */
char *generator_bool_envname(token_t *parm, const void *data,
        unsigned int level)
{
    ASSERT(parm != NULL);
    ASSERT((parm_type(parm) == tt_str) || parm_type(parm) == tt_end);

    /* Iterator initialization */
    if (level == 0) {
        last_benv = NULL;
    }

    /* Find next suitable entry */
    bool fnd;
    do {
        const char *name = (parm_type(parm) == tt_str) ? parm_str(parm) : "";

        fnd = env_by_partial_varname(name, &last_benv);
    } while ((fnd) && (last_benv->type != vt_bool));

    if (fnd) {
        return safe_strdup(last_benv->name);
    }

    return NULL;
}

/** Generate an equal mark
 *
 */
char *generator_equal_char(token_t *token, const void *data,
        unsigned int level)
{
    ASSERT(token != NULL);
    ASSERT((parm_type(token) == tt_str) || (parm_type(token) == tt_end));

    if (level == 0) {
        return safe_strdup("=");
    }

    return NULL;
}
