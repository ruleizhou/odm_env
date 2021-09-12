/*
*   Copyright (c) 2009, Dmitry Peskov (dmitrypeskov@users.sourceforge.net)
*
*   This source code is released for free distribution under the terms of the
*   GNU General Public License version 2 or (at your option) any later version.
*
*   File: ttcn.c
*   Description: Adds TTCN-3 support to Exuberant ctags
*   Version: 0.4
*   Author: Dmitry Peskov (dmitrypeskov@users.sourceforge.net)
*   Based on ETSI ES 201 873-1 V3.4.1 appendix A
*   Version history:
*   0.1     Initial version
*   0.2     Enhancement: faster binary search in keyword list
*           Fix: skip tag generation from import definitions (import/except lists)
*   0.3     Fix: expression parser no longer relies on optional semicolon
*           after a const/var/timer/modulepar definition to find where the initializer
*           actually ends
*           Fix: handle multiple module parameters not separated by semicolons
*   0.31    Fix: lexer bug, missing character after a charstring close-quote
*   0.4     Enhancement: tags for record/set/union members, enum values, port instances
*           within components
*/

/* INCLUDE FILES */

#include "general.h"    /* always include first */
#include <stdlib.h>
#include <string.h>     /* to declare strxxx() functions */
#include <ctype.h>      /* to define isxxx() macros */
#include "parse.h"      /* always include */
#include "read.h"       /* to define file fileReadLine() */
#include "routines.h"
#include "debug.h"

/*    DATA    */

/* Tag kinds generated by parser */
typedef enum {
	K_MODULE,
	K_TYPE,
	K_CONST,
	K_TEMPLATE,
	K_FUNCTION,
	K_SIGNATURE,
	K_TESTCASE,
	K_ALTSTEP,
	K_GROUP,
	K_MODULEPAR,
	K_VAR,
	K_TIMER,
	K_PORT,
	K_MEMBER,
	K_ENUM,
	K_NONE  /* No tag */
} ttcnKind_t;

static kindDefinition ttcnKinds [] = {
	{ true, 'M', "module",    "module definition" },
	{ true, 't', "type",      "type definition" },
	{ true, 'c', "const",     "constant definition" },
	{ true, 'd', "template",  "template definition" },
	{ true, 'f', "function",  "function definition" },
	{ true, 's', "signature", "signature definition" },
	{ true, 'C', "testcase",  "testcase definition" },
	{ true, 'a', "altstep",   "altstep definition" },
	{ true, 'G', "group",     "group definition" },
	{ true, 'P', "modulepar", "module parameter definition" },
	{ true, 'v', "var",       "variable instance" },
	{ true, 'T', "timer",     "timer instance" },
	{ true, 'p', "port",      "port instance" },
	{ true, 'm', "member",    "record/set/union member" },
	{ true, 'e', "enum",      "enumeration value" }
};

/* TTCN token types */
typedef enum {
	/* Values up to 255 are reserved for single-char tokens: '+', '-', etc. */
	T_ID = 256,   /* Identifier */
	T_LITERAL,    /* Literal: integer, real, (char|hex|octet|bit)string */
	/* Keywords */
	T_ACTION, T_ACTIVATE, T_ADDRESS, T_ALIVE, T_ALL, T_ALT, T_ALTSTEP, T_AND,
	T_AND4B, T_ANY, T_ANYTYPE, T_BITSTRING, T_BOOLEAN, T_CASE, T_CALL, T_CATCH,
	T_CHAR, T_CHARSTRING, T_CHECK, T_CLEAR, T_COMPLEMENT, T_COMPONENT,
	T_CONNECT, T_CONST, T_CONTROL, T_CREATE, T_DEACTIVATE, T_DEFAULT,
	T_DISCONNECT, T_DISPLAY, T_DO, T_DONE, T_ELSE, T_ENCODE, T_ENUMERATED,
	T_ERROR, T_EXCEPT, T_EXCEPTION, T_EXECUTE, T_EXTENDS, T_EXTENSION,
	T_EXTERNAL, T_FAIL, T_FALSE, T_FLOAT, T_FOR, T_FROM, T_FUNCTION,
	T_GETVERDICT, T_GETCALL, T_GETREPLY, T_GOTO, T_GROUP, T_HEXSTRING, T_IF,
	T_IFPRESENT, T_IMPORT, T_IN, T_INCONC, T_INFINITY, T_INOUT, T_INTEGER,
	T_INTERLEAVE, T_KILL, T_KILLED,    T_LABEL, T_LANGUAGE, T_LENGTH, T_LOG, T_MAP,
	T_MATCH, T_MESSAGE, T_MIXED, T_MOD, T_MODIFIES, T_MODULE, T_MODULEPAR,
	T_MTC, T_NOBLOCK, T_NONE, T_NOT, T_NOT4B, T_NOWAIT, T_NULL, T_OCTETSTRING,
	T_OF, T_OMIT, T_ON, T_OPTIONAL, T_OR, T_OR4B, T_OUT, T_OVERRIDE, T_PARAM,
	T_PASS, T_PATTERN, T_PORT, T_PROCEDURE, T_RAISE, T_READ, T_RECEIVE,
	T_RECORD, T_RECURSIVE, T_REM, T_REPEAT, T_REPLY, T_RETURN, T_RUNNING,
	T_RUNS,    T_SELECT, T_SELF, T_SEND, T_SENDER, T_SET, T_SETVERDICT,
	T_SIGNATURE, T_START, T_STOP, T_SUBSET, T_SUPERSET, T_SYSTEM, T_TEMPLATE,
	T_TESTCASE, T_TIMEOUT, T_TIMER, T_TO, T_TRIGGER, T_TRUE, T_TYPE, T_UNION,
	T_UNIVERSAL, T_UNMAP, T_VALUE, T_VALUEOF, T_VAR, T_VARIANT, T_VERDICTTYPE,
	T_WHILE, T_WITH, T_XOR, T_XOR4B,
	/* Double-char operators (single-char operators are returned "as is") */
	T_OP_TO /* .. */, T_OP_EQ /* == */, T_OP_NE /* != */,
	T_OP_LE /* <= */, T_OP_GE /* >= */,    T_OP_ASS /* := */, T_OP_ARROW /* -> */,
	T_OP_SHL /* << */, T_OP_SHR /* >> */, T_OP_ROTL /* <@ */, T_OP_ROTR /* @> */
} ttcnTokenType_t;

/* TTCN keywords. List MUST be sorted in alphabetic order!!! */
static struct s_ttcnKeyword {
	const ttcnTokenType_t id;
	const char * name;
	const ttcnKind_t kind;  /* Corresponding tag kind, K_NONE if none */
} ttcnKeywords [] = {
	{T_ACTION,      "action",       K_NONE},
	{T_ACTIVATE,    "activate",     K_NONE},
	{T_ADDRESS,     "address",      K_NONE},
	{T_ALIVE,       "alive",        K_NONE},
	{T_ALL,         "all",          K_NONE},
	{T_ALT,         "alt",          K_NONE},
	{T_ALTSTEP,     "altstep",      K_ALTSTEP},
	{T_AND,         "and",          K_NONE},
	{T_AND4B,       "and4b",        K_NONE},
	{T_ANY,         "any",          K_NONE},
	{T_ANYTYPE,     "anytype",      K_NONE},
	{T_BITSTRING,   "bitstring",    K_NONE},
	{T_BOOLEAN,     "boolean",      K_NONE},
	{T_CASE,        "case",         K_NONE},
	{T_CALL,        "call",         K_NONE},
	{T_CATCH,       "catch",        K_NONE},
	{T_CHAR,        "char",         K_NONE},
	{T_CHARSTRING,  "charstring",   K_NONE},
	{T_CHECK,       "check",        K_NONE},
	{T_CLEAR,       "clear",        K_NONE},
	{T_COMPLEMENT,  "complement",   K_NONE},
	{T_COMPONENT,   "component",    K_NONE},
	{T_CONNECT,     "connect",      K_NONE},
	{T_CONST,       "const",        K_CONST},
	{T_CONTROL,     "control",      K_NONE},
	{T_CREATE,      "create",       K_NONE},
	{T_DEACTIVATE,  "deactivate",   K_NONE},
	{T_DEFAULT,     "default",      K_NONE},
	{T_DISCONNECT,  "disconnect",   K_NONE},
	{T_DISPLAY,     "display",      K_NONE},
	{T_DO,          "do",           K_NONE},
	{T_DONE,        "done",         K_NONE},
	{T_ELSE,        "else",         K_NONE},
	{T_ENCODE,      "encode",       K_NONE},
	{T_ENUMERATED,  "enumerated",   K_NONE},
	{T_ERROR,       "error",        K_NONE},
	{T_EXCEPT,      "except",       K_NONE},
	{T_EXCEPTION,   "exception",    K_NONE},
	{T_EXECUTE,     "execute",      K_NONE},
	{T_EXTENDS,     "extends",      K_NONE},
	{T_EXTENSION,   "extension",    K_NONE},
	{T_EXTERNAL,    "external",     K_NONE},
	{T_FAIL,        "fail",         K_NONE},
	{T_FALSE,       "false",        K_NONE},
	{T_FLOAT,       "float",        K_NONE},
	{T_FOR,         "for",          K_NONE},
	{T_FROM,        "from",         K_NONE},
	{T_FUNCTION,    "function",     K_FUNCTION},
	{T_GETVERDICT,  "getverdict",   K_NONE},
	{T_GETCALL,     "getcall",      K_NONE},
	{T_GETREPLY,    "getreply",     K_NONE},
	{T_GOTO,        "goto",         K_NONE},
	{T_GROUP,       "group",        K_GROUP},
	{T_HEXSTRING,   "hexstring",    K_NONE},
	{T_IF,          "if",           K_NONE},
	{T_IFPRESENT,   "ifpresent",    K_NONE},
	{T_IMPORT,      "import",       K_NONE},
	{T_IN,          "in",           K_NONE},
	{T_INCONC,      "inconc",       K_NONE},
	{T_INFINITY,    "infinity",     K_NONE},
	{T_INOUT,       "inout",        K_NONE},
	{T_INTEGER,     "integer",      K_NONE},
	{T_INTERLEAVE,  "interleave",   K_NONE},
	{T_KILL,        "kill",         K_NONE},
	{T_KILLED,      "killed",       K_NONE},
	{T_LABEL,       "label",        K_NONE},
	{T_LANGUAGE,    "language",     K_NONE},
	{T_LENGTH,      "length",       K_NONE},
	{T_LOG,         "log",          K_NONE},
	{T_MAP,         "map",          K_NONE},
	{T_MATCH,       "match",        K_NONE},
	{T_MESSAGE,     "message",      K_NONE},
	{T_MIXED,       "mixed",        K_NONE},
	{T_MOD,         "mod",          K_NONE},
	{T_MODIFIES,    "modifies",     K_NONE},
	{T_MODULE,      "module",       K_MODULE},
	{T_MODULEPAR,   "modulepar",    K_MODULEPAR},
	{T_MTC,         "mtc",          K_NONE},
	{T_NOBLOCK,     "noblock",      K_NONE},
	{T_NONE,        "none",         K_NONE},
	{T_NOT,         "not",          K_NONE},
	{T_NOT4B,       "not4b",        K_NONE},
	{T_NOWAIT,      "nowait",       K_NONE},
	{T_NULL,        "null",         K_NONE},
	{T_OCTETSTRING, "octetstring",  K_NONE},
	{T_OF,          "of",           K_NONE},
	{T_OMIT,        "omit",         K_NONE},
	{T_ON,          "on",           K_NONE},
	{T_OPTIONAL,    "optional",     K_NONE},
	{T_OR,          "or",           K_NONE},
	{T_OR4B,        "or4b",         K_NONE},
	{T_OUT,         "out",          K_NONE},
	{T_OVERRIDE,    "override",     K_NONE},
	{T_PARAM,       "param",        K_NONE},
	{T_PASS,        "pass",         K_NONE},
	{T_PATTERN,     "pattern",      K_NONE},
	{T_PORT,        "port",         K_PORT},
	{T_PROCEDURE,   "procedure",    K_NONE},
	{T_RAISE,       "raise",        K_NONE},
	{T_READ,        "read",         K_NONE},
	{T_RECEIVE,     "receive",      K_NONE},
	{T_RECORD,      "record",       K_NONE},
	{T_RECURSIVE,   "recursive",    K_NONE},
	{T_REM,         "rem",          K_NONE},
	{T_REPEAT,      "repeat",       K_NONE},
	{T_REPLY,       "reply",        K_NONE},
	{T_RETURN,      "return",       K_NONE},
	{T_RUNNING,     "running",      K_NONE},
	{T_RUNS,        "runs",         K_NONE},
	{T_SELECT,      "select",       K_NONE},
	{T_SELF,        "self",         K_NONE},
	{T_SEND,        "send",         K_NONE},
	{T_SENDER,      "sender",       K_NONE},
	{T_SET,         "set",          K_NONE},
	{T_SETVERDICT,  "setverdict",   K_NONE},
	{T_SIGNATURE,   "signature",    K_SIGNATURE},
	{T_START,       "start",        K_NONE},
	{T_STOP,        "stop",         K_NONE},
	{T_SUBSET,      "subset",       K_NONE},
	{T_SUPERSET,    "superset",     K_NONE},
	{T_SYSTEM,      "system",       K_NONE},
	{T_TEMPLATE,    "template",     K_TEMPLATE},
	{T_TESTCASE,    "testcase",     K_TESTCASE},
	{T_TIMEOUT,     "timeout",      K_NONE},
	{T_TIMER,       "timer",        K_TIMER},
	{T_TO,          "to",           K_NONE},
	{T_TRIGGER,     "trigger",      K_NONE},
	{T_TRUE,        "true",         K_NONE},
	{T_TYPE,        "type",         K_TYPE},
	{T_UNION,       "union",        K_NONE},
	{T_UNIVERSAL,   "universal",    K_NONE},
	{T_UNMAP,       "unmap",        K_NONE},
	{T_VALUE,       "value",        K_NONE},
	{T_VALUEOF,     "valueof",      K_NONE},
	{T_VAR,         "var",          K_VAR},
	{T_VARIANT,     "variant",      K_NONE},
	{T_VERDICTTYPE, "verdicttype",  K_NONE},
	{T_WHILE,       "while",        K_NONE},
	{T_WITH,        "with",         K_NONE},
	{T_XOR,         "xor",          K_NONE},
	{T_XOR4B,       "xor4b",        K_NONE}
};
static const int ttcnKeywordCount = ARRAY_SIZE(ttcnKeywords);

/* TTCN double-char operators */
static struct s_ttcnOp {
	const ttcnTokenType_t id;
	const char name[3];
} ttcnOps [] = {
	{T_OP_TO,       ".."},
	{T_OP_EQ,       "=="},
	{T_OP_NE,       "!="},
	{T_OP_LE,       "<="},
	{T_OP_GE,       ">="},
	{T_OP_ASS,      ":="},
	{T_OP_ARROW,    "->"},
	{T_OP_SHL,      "<<"},
	{T_OP_SHR,      ">>"},
	{T_OP_ROTL,     "<@"},
	{T_OP_ROTR,     "@>"}
};
static const int ttcnOpCount = ARRAY_SIZE(ttcnOps);

/* Token */
typedef struct s_ttcnToken {
	ttcnTokenType_t type;
	vString * value;    /* Semantic value (T_ID and T_LITERAL only) */
	ttcnKind_t kind;    /* Corresponding tag kind (keywords only) */
} ttcnToken_t;

/*    LEXER    */

/* Functions forward declarations */
static void findTTCNKeyword(ttcnToken_t * pTok);
static void freeToken(ttcnToken_t * pTok);
static int getNonWhiteSpaceChar (void);
static ttcnToken_t * getToken (void);
static void ungetToken (void);

static int ttcnKeywordsCompare (const void * key, const void * member)
{
	return strcmp(key, ((struct s_ttcnKeyword *)member)->name);
}

/* Check if token is a TTCN-3 keyword or not */
static void findTTCNKeyword(ttcnToken_t * pTok)
{
	struct s_ttcnKeyword * k;
	if (!pTok || !(pTok->value) || !(vStringValue(pTok->value)))
		return;
	/* Binary search */
	k = bsearch (vStringValue (pTok->value), ttcnKeywords,
		 ttcnKeywordCount, sizeof (*ttcnKeywords),
		 ttcnKeywordsCompare);
	if (k)
	{
		pTok->type = k->id;
		pTok->kind = k->kind;
	}
	else
	{
		pTok->type = T_ID;
	}
}

static ttcnToken_t * pTtcnToken = NULL;
static int repeatLastToken = false;

static void freeToken(ttcnToken_t * pTok)
{
	if (pTok)
	{
		if (pTok->value)
			vStringDelete(pTok->value);
		eFree (pTok);
	}
}

/* This function skips all whitespace and comments */
static int getNonWhiteSpaceChar (void)
{
	int c, c2;
	while (1)
	{
		/* Skip whitespace */
		while (isspace(c = getcFromInputFile()) && (c != EOF));
		/* Skip C/C++-style comment */
		if (c=='/')
		{
			c2 = getcFromInputFile();
			if (c2=='/')
			{
				/* Line comment */
				while (((c = getcFromInputFile()) != EOF) && (c != '\n'));
				continue;
			}
			else if (c2=='*')
			{
				/* Block comment */
				while ((c = getcFromInputFile()) != EOF)
				{
					if (c=='*')
					{
						c2 = getcFromInputFile();
						if (c2 == '/')
							break;
						ungetcToInputFile(c2);
					}
				}
				continue;
			}
			else
			{
				/* Not a comment */
				ungetcToInputFile(c2);
				break;
			}
		}
		break;
	}
	Assert (c == EOF || !isspace(c));
	return c;
}

static ttcnToken_t * getToken (void)
{
	int c, c2;
	int i;
	if (repeatLastToken)
	{
		/* If ungetToken() has been called before, return last token again */
		repeatLastToken = false;
		return pTtcnToken;
	}
	else
	{
		/* Clean up last token */
		freeToken(pTtcnToken);
		pTtcnToken = NULL;
	}
	/* Handle EOF and malloc errors */
	if ((c = getNonWhiteSpaceChar()) == EOF)
		return NULL;

	pTtcnToken = xMalloc (1, ttcnToken_t);
	pTtcnToken->type = 0;
	pTtcnToken->value = NULL;
	pTtcnToken->kind = K_NONE;

	/* Parse tokens */
	if (isalpha(c))
	{
		/* Identifier or keyword */
		pTtcnToken->value = vStringNew();
		do
		{
			vStringPut(pTtcnToken->value, c);
			c = getcFromInputFile();
		} while (isalnum(c) || c == '_');
		/* Push back last char */
		ungetcToInputFile(c);
		/* Is it a keyword or an identifier? */
		findTTCNKeyword(pTtcnToken);
	}
	else if (c == '\'')
	{
		/* Octetstring, bitstring, hexstring */
		pTtcnToken->type = T_LITERAL;
		pTtcnToken->value = vStringNew();
		vStringPut(pTtcnToken->value, c);
		/* Hex digits only (NB: 0/1 only in case of bitstring) */
		while (isxdigit(c = getcFromInputFile()))
			vStringPut(pTtcnToken->value, c);
		/* Must be terminated with "'O", "'B" or "'H" */
		if (c != '\'')
			pTtcnToken->type = 0;
		else
		{
			vStringPut(pTtcnToken->value, c);
			c = getcFromInputFile();
			if ((c != 'O') && (c != 'H') && (c != 'B'))
				pTtcnToken->type = 0;
			else
				vStringPut(pTtcnToken->value, c);
		}
	}
	else if (c == '"')
	{
		/* Charstring */
		pTtcnToken->type = T_LITERAL;
		pTtcnToken->value = vStringNew();
        vStringPut(pTtcnToken->value, c);
		while((c = getcFromInputFile()) != EOF)
		{
			vStringPut(pTtcnToken->value, c);
            /* consume escaped characters */
            if(c == '\\' && ((c2 = getcFromInputFile()) != EOF))
            {
                vStringPut(pTtcnToken->value, c2);
                continue;
            }
            /* Double '"' represents '"' within a Charstring literal */
			if (c == '"')
			{
				c2 = getcFromInputFile();
                /* consume "" */
				if (c2 == '"')
                {
                    vStringPut(pTtcnToken->value, c2);
                    continue;
                }
                /* c is " that close string, c2 is out of string */
                if(c2 != EOF)
                    ungetcToInputFile(c2);
                break;
			}
		}
		if (c != '"')
			pTtcnToken->type = 0;
	}
	else if (isdigit(c))
	{
		/* Number */
		pTtcnToken->type = T_LITERAL;
		pTtcnToken->value = vStringNew();
		/* Integer part */
		do
			vStringPut(pTtcnToken->value, c);
		while (isdigit(c = getcFromInputFile()));
		/* Decimal dot */
		if (c == '.')
		{
			vStringPut(pTtcnToken->value, c);
			/* Fractional part */
			while (isdigit(c = getcFromInputFile()))
				vStringPut(pTtcnToken->value, c);
		}
		/* Exponent */
		if ((c == 'E') || (c == 'e'))
		{
			vStringPut(pTtcnToken->value, c);
			/* Exponent sign */
			if ((c = getcFromInputFile()) == '-')
				vStringPut(pTtcnToken->value, c);
			else
				ungetcToInputFile(c);
			/* Exponent integer part */
			while (isdigit(c = getcFromInputFile()))
				vStringPut(pTtcnToken->value, c);
			/* Exponent decimal dot */
			if (c == '.')
			{
				vStringPut(pTtcnToken->value, c);
				/* Exponent fractional part */
				while (isdigit(c = getcFromInputFile()))
					vStringPut(pTtcnToken->value, c);
			}
		}
		/* Push back last char */
		ungetcToInputFile(c);
	}
	else
	{
		/* Operator, 1 or 2 chars, need to look 1 char ahead */
		c2 = getcFromInputFile();
		for (i = 0; i<ttcnOpCount; i++)
		{
			if ((c == ttcnOps[i].name[0]) && (c2 == ttcnOps[i].name[1]))
			{
				pTtcnToken->type = ttcnOps[i].id;
				break;
			}
		}
		if (i == ttcnOpCount)
		{
			/* No double-char operator found => single-char operator */
			pTtcnToken->type = c;
			/* Push back the second char */
			ungetcToInputFile(c2);
		}
	}
	/* Only identifier and literal tokens have a value */
	if ((pTtcnToken->type != T_ID) && (pTtcnToken->type != T_LITERAL))
	{
		vStringDelete(pTtcnToken->value);
		pTtcnToken->value = NULL;
	}
	return pTtcnToken;
}

static void ungetToken (void) { repeatLastToken = true; }

/*    PARSER    */

/* Functions forward declarations */
static ttcnToken_t * matchToken (ttcnTokenType_t toktype);
static int matchBrackets (const char * br);
static int matchExprOperator (void);
static int parseExprOperand (void);
static int parseSimpleExpression(void);
static int parseID (ttcnKind_t kind);
static int parseStringLength (void);
static int parseExtendedFieldReference (void);
static int parseArrayDef (void);
static int parseInitializer (void);
static int parseNameInitList (ttcnKind_t kind);
static int parseType (void);
static int parseSignature (void);
static int parseStructure (void);
static int parseEnumeration(void);
static int parseNestedTypeDef (void);
static int parseTypeDefBody (void);
static void parseTTCN (void);

/* Check if next token is of a specified type */
static ttcnToken_t * matchToken (ttcnTokenType_t toktype)
{
	ttcnToken_t * pTok = getToken();
	if (pTok && (pTok->type == toktype))
		return pTok;
	ungetToken();
	return NULL;
}

/* Count nested brackets, return when brackets are balanced */
static int matchBrackets (const char * br)
{
	if (matchToken(br[0]))
	{
		int brcount = 1;
		while (brcount > 0)
		{
			if (matchToken(br[0]))       /* Open */
				brcount++;
			else if (matchToken(br[1]))  /* Close */
				brcount--;
			else if (!getToken())        /* EOF */
				return 0;
		}
		return 1;
	}
	return 0;
}
static const char BR_CUR[] = "{}";
static const char BR_PAR[] = "()";
static const char BR_SQ[] = "[]";
static const char BR_ANG[] = "<>";

/* List of TTCN operators.
   A dot (.) is not a TTCN operator but it is included to simplify
   the expression parser. Instead of treating X.Y.Z as a single primary
   the parser treats it as an expression with 3 primaries and two dots. */
static const ttcnTokenType_t ttcnExprOps[] = {
	T_OR, T_XOR, T_AND, T_NOT, T_OP_EQ, T_OP_NE, '>', '<', T_OP_GE, T_OP_LE,
	T_OP_SHL, T_OP_SHR, T_OP_ROTL, T_OP_ROTR, T_OR4B, T_XOR4B, T_AND4B, T_NOT4B,
	'+', '-', '&', '*', '/', T_MOD, T_REM, '.'
};
static const int ttcnExprOpCount = ARRAY_SIZE(ttcnExprOps);

/* Check if next token is an expression operator */
static int matchExprOperator (void)
{
	ttcnToken_t * pTok = getToken();
	int i;
	if (!pTok)
		return 0;
	for (i = 0; i < ttcnExprOpCount; i++)
		if (pTok->type == ttcnExprOps[i])
			return 1;
	/* Not found */
	ungetToken();
	return 0;
}

static int parseExprOperand (void)
{
	ttcnToken_t * pTok;
	if (matchBrackets(BR_PAR)) /* Nested expression in brackets */
		return 1;
	if (!(pTok = getToken()))
		return 0;
	switch (pTok->type)
	{
		case T_CREATE:
			/* Create statement */
			matchBrackets(BR_PAR);
			matchToken(T_ALIVE);
			return 1;
		case T_EXECUTE:
		case T_MATCH:
		case T_VALUEOF:
		case T_ACTIVATE:
		case T_CHAR:
			/* These tokens must be followed by something in parentheses */
			return matchBrackets(BR_PAR);
		case T_SELF:
		case T_SYSTEM:
		case T_MTC:
		case T_RUNNING:
		case T_ALIVE:
		case T_GETVERDICT:
		case T_READ:
		case T_ANY:
		case T_LITERAL:
		case T_TRUE:
		case T_FALSE:
		case T_PASS:
		case T_FAIL:
		case T_INCONC:
		case T_NONE:
		case T_ERROR:
		case T_NULL:
		case T_OMIT:
			return 1;
		case T_ID:
			if (!matchBrackets(BR_PAR))          /* Function call OR ... */
				while (matchBrackets(BR_SQ));    /* ... array indexing */
			return 1;
		default:
			break;
	}
	ungetToken();
	return 0;
}

/* Too lazy to really parse expressions so the parser is rather simplistic:
   an expression is a series of operands separated by 1 or more operators. */
static int parseSimpleExpression(void)
{
	while (matchExprOperator());    /* Skip leading unary ops */
	while (parseExprOperand() && matchExprOperator())
		while (matchExprOperator());
	return 1;
}

/* Check if next token is an identifier, make a tag of a specified kind */
static int parseID (ttcnKind_t kind)
{
	ttcnToken_t * pTok = matchToken(T_ID);
	if (pTok)
	{
		if (kind < K_NONE)
			makeSimpleTag(pTok->value, kind);
		return 1;
	}
	return 0;
}

/* StringLength ::= "length" '(' ... ')' */
static int parseStringLength (void)
{
	return (matchToken(T_LENGTH) && matchBrackets(BR_PAR));
}

/* ExtendedFieldReference ::= { '.' ID | '[' ... ']' }+ */
static int parseExtendedFieldReference (void)
{
	int res = 0;
	while ((matchToken('.') && matchToken(T_ID)) || matchBrackets(BR_SQ))
		res = 1;
	return res;
}

/* ArrayDef ::= { '[' ... ']' }+ */
static int parseArrayDef (void)
{
	int res = 0;
	while (matchBrackets(BR_SQ))
		res = 1;
	return res;
}

/* Initializer ::= ":=" Expression */
static int parseInitializer (void)
{
	if (matchToken(T_OP_ASS))
	{
		/* Compound expression */
		if (matchBrackets(BR_CUR))
			return 1;
		/* Simple expression */
		return parseSimpleExpression();
	}
	return 0;
}

/* NameInitList ::= ID [ArrayDef] [Initializer]
   { ',' ID [ArrayDef] [Initializer] }
   Used for parsing const/modulepar/var/timer/port definitions. */
static int parseNameInitList (ttcnKind_t kind)
{
	do
	{
		if (!parseID(kind))
			return 0;
		/* NB: ArrayDef is not allowed for modulepar */
		parseArrayDef();
		/* NB: Initializer is mandatory for constants, not allowed for ports */
		parseInitializer();
	} while (matchToken(','));
	return 1;
}

/* A.1.6.3 Type ::= PredefinedType | ReferencedType */
static int parseType (void)
{
	ttcnToken_t * pTok = getToken();
	if (!pTok)
		return 0;
	switch (pTok->type)
	{
		/* PredefinedType */
		case T_BITSTRING:
		case T_BOOLEAN:
		case T_CHARSTRING:
		case T_INTEGER:
		case T_OCTETSTRING:
		case T_HEXSTRING:
		case T_VERDICTTYPE:
		case T_FLOAT:
		case T_ADDRESS:
		case T_DEFAULT:
		case T_ANYTYPE:
			return 1;
		case T_UNIVERSAL:
			if (!matchToken(T_CHARSTRING))
				break;
			else
				return 1;
		/* ReferencedType ::= [ModuleID '.'] TypeID
		   [TypeActualParList] [ExtendedFieldReference] */
		case T_ID:
			/* ModuleID.TypeID */
			if (matchToken('.') && !(matchToken(T_ID)))
				break;
            /* FormalTypeParList */
            matchBrackets(BR_ANG);
			/* TypeActualParList */
			matchBrackets(BR_PAR);
			/* ExtendedFieldReference */
			parseExtendedFieldReference();
			return 1;
		default :
			break;
	}
	ungetToken();
	return 0;
}

/* A.1.6.1.5 Signature ::= "signature" [ModuleID '.'] ID */
static int parseSignature (void)
{
	return (matchToken(T_SIGNATURE) && matchToken(T_ID) && (!matchToken('.') || matchToken(T_ID)));
}

/* A.1.6.1.1 Structure ::= "{" [FieldDef {"," FieldDef}] "}"    */
static int parseStructure (void)
{
	if (!matchToken('{'))
		return 0;
	/* Comma-separated list of fields */
	do
	{
		/* StructFieldDef ::= (Type | NestedTypeDef) ID
		[ArrayDef] [SubTypeSpec] ["optional"] */
		if (!((parseType() || parseNestedTypeDef()) && parseID(K_MEMBER)))
			return 0;
		parseArrayDef();
		/* SubTypeSpec */
		matchBrackets(BR_PAR);  /* AllowedValues */
		parseStringLength();    /* StringLength */
		matchToken(T_OPTIONAL); /* "optional" keyword */
	} while (matchToken(','));
	if (!matchToken('}'))
		return 0;
	return 1;
}

/* A.1.6.1.1 Enumeration ::= "{" [EnumDef {"," EnumDef}] "}" */
static int parseEnumeration(void)
{
	if (!matchToken('{'))
		return 0;
	/* Comma-separated list of names and values */
	do
	{
		/* EnumDef ::= ID ['(' ['-'] Value ')'] */
		if (!parseID(K_ENUM))
			return 0;
		matchBrackets(BR_PAR);    /* Value */
	} while (matchToken(','));
	if (!matchToken('}'))
		return 0;
	return 1;
}

/* NestedTypeDef ::= NestedRecordDef | NestedUnionDef | NestedSetDef |
   NestedRecordOfDef | NestedSetOfDef | NestedEnumDef */
static int parseNestedTypeDef (void)
{
	ttcnToken_t * pTok = getToken();
	if (!pTok)
		return 0;
	switch (pTok->type)
	{
		case T_RECORD:
		case T_SET:
			/* StringLength (optional), applies to RecordOf and SetOf */
			parseStringLength();
			/* RecordOf/SetOf */
			if (matchToken(T_OF))
				return (parseType() || parseNestedTypeDef());
			/* This is correct, no break here! */
		case T_UNION:
			/* Parse Record/Set/Union structure */
			return parseStructure();
		case T_ENUMERATED:
			/* Parse Enumeration values */
			return parseEnumeration();
		default :
			break;
	}
	/* No match */
	ungetToken();
	return 0;
}

/* A.1.6.1.1 TypeDefBody ::= StructuredTypeDef | SubTypeDef */
static int parseTypeDefBody (void)
{
	ttcnToken_t * pTok = getToken();
	if (!pTok)
		return 0;
	switch (pTok->type)
	{
		/* StructuredTypeDef ::= RecordDef | UnionDef | SetDef |
		   RecordOfDef | SetOfDef | EnumDef | PortDef | ComponentDef */
		case T_RECORD:
		case T_SET:
			/* StringLength (optional), applies to RecordOf and SetOf */
			parseStringLength();
			if (matchToken(T_OF))    /* RecordOf/SetOf */
				return ((parseType() || parseNestedTypeDef()) && parseID(K_TYPE));
			/* This is correct, no break here! */
		case T_UNION:
			/* Parse Record/Set/Union structure */
			if (!(parseID(K_TYPE) || matchToken(T_ADDRESS)))
				return 0;
			matchBrackets(BR_PAR);    /* StructDefFormalParList */
			return parseStructure();
		case T_ENUMERATED:
			if (!(parseID(K_TYPE) || matchToken(T_ADDRESS)))
				return 0;
			return parseEnumeration();
		case T_PORT:
		case T_COMPONENT:
			/* Record/Set/Union/Enum/Port/Component */
			return parseID(K_TYPE);
		default:
			/* SubTypeDef */
			ungetToken();
			/* Stop after type name, no need to parse ArrayDef, StringLength, etc */
			return (parseType() && parseID(K_TYPE));
	}
}

static void parseTTCN (void)
{
	ttcnToken_t * pTok;
	ttcnKind_t kind;
	while ((pTok = getToken()))
	{
		kind = pTok->kind;
		switch (pTok->type)
		{
			case T_MODULE:      /* A.1.6.0 */
			case T_FUNCTION:    /* A.1.6.1.4 */
			case T_SIGNATURE:   /* A.1.6.1.5 */
			case T_TESTCASE:    /* A.1.6.1.6 */
			case T_ALTSTEP:     /* A.1.6.1.7 */
			case T_GROUP:       /* A.1.6.1.9 */
				/* Def ::= Keyword ID ... */
				parseID(kind);
				break;

			case T_VAR:
				/* A.1.6.2.1 VarInstance ::= "var" ["template"] Type List */
				matchToken(T_TEMPLATE);
			case T_CONST:   /* A.1.6.1.2 ConstDef ::= "const" Type List */
			case T_PORT:    /* A.1.6.1.1 PortInstance ::= "port" Type List */
				if (!parseType())
					break;
			case T_TIMER:   /* A.1.6.2.2 TimerInstance ::= "timer" List */
				parseNameInitList(kind);
				break;
			case T_TYPE:    /* A.1.6.1.1 */
				parseTypeDefBody();
				break;
			case T_TEMPLATE:
				/* A.1.6.1.3 TemplateDef ::= "template" (Type | Signature) ID ... */
				if (parseType() || parseSignature())
					parseID(K_TEMPLATE);
				break;
			case T_MODULEPAR:
				/* A.1.6.1.12 ModuleParDef ::= "modulepar"
				   (Type ModuleParList | '{' MultitypedModuleParList '}') */
				if (matchToken('{'))
				{
					/* MultitypedModuleParList ::= (Type ModuleParList [;])* */
					while (parseType() && parseNameInitList(K_MODULEPAR))
						/* Separating semicolons are optional */
						while (matchToken(';'));
				}
				else if (parseType())
				{
					/* Type ModuleParList */
					parseNameInitList(K_MODULEPAR);
				}
				break;
			case T_IMPORT:
				/* A.1.6.1.8 ImportDef = "import" "from" ModuleId ["recursive"]
				('{' ImportList '}') | ("all" ["except"] '{' ExceptList '}')
				Parse import definition not to generate tags but to avoid
				generating misleading tags from import/except lists */
				if (!matchToken(T_FROM) || ! matchToken(T_ID))
					break;
				matchToken(T_RECURSIVE);
				if (matchToken(T_ALL) && !matchToken(T_EXCEPT))
					break;
				/* Skip import/except list */
				matchBrackets(BR_CUR);
				break;
			default :
				break;
		}
	}
}

/* Parser definition */
extern parserDefinition * TTCNParser (void)
{
	static const char * const extensions [] = { "ttcn", "ttcn3", NULL };
	parserDefinition * def = parserNew ("TTCN");
	def->kindTable      = ttcnKinds;
	def->kindCount  = ARRAY_SIZE(ttcnKinds);
	def->extensions = extensions;
	def->parser     = parseTTCN;
	return def;
}
