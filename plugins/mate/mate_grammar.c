/* Driver template for the LEMON parser generator.
**
** Copyright 1991-1995 by D. Richard Hipp.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**
** Modified 1997 to make it suitable for use with makeheaders.
* Updated to sqlite lemon version 1.36
*/
/* First off, code is included that follows the "include" declaration
** in the input grammar file. */
#include <stdio.h>
#line 1 "mate_grammar.lemon"


/* mate_grammar.lemon
* MATE's configuration language grammar
*
* Copyright 2005, Luis E. Garcia Ontanon <luis@ontanon.org>
*
* Wireshark - Network traffic analyzer
* By Gerald Combs <gerald@wireshark.org>
* Copyright 1998 Gerald Combs
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "mate.h"
#include "mate_grammar.h"
#include <wsutil/file_util.h>

#define DUMMY void*

typedef struct _extraction {
	gchar* as;
	header_field_info* hfi;
	struct _extraction* next;
	struct _extraction* last;
} extraction_t;

typedef struct _pdu_criteria_t {
	AVPL* criterium_avpl;
	avpl_match_mode criterium_match_mode;
	accept_mode_t criterium_accept_mode;
} pdu_criteria_t;

typedef struct _gop_options {
	gop_tree_mode_t pdu_tree_mode;
	gboolean drop_unassigned;
	gboolean show_times;
	float expiration;
	float idle_timeout;
	float lifetime;
	AVPL* start;
	AVPL* stop;
	AVPL* extras;
} gop_options_t;

typedef struct _gog_statements {
	float expiration;
	gop_tree_mode_t gop_tree_mode;
	GPtrArray* transform_list;
	AVPL* extras;
	LoAL* current_gogkeys;
} gog_statement_t;

typedef struct _transf_match_t {
    avpl_match_mode match_mode;
    AVPL* avpl;
} transf_match_t;

typedef struct _transf_action_t {
    avpl_replace_mode replace_mode;
    AVPL* avpl;
} transf_action_t;

static void configuration_error(mate_config* mc, const gchar* fmt, ...) {
	static gchar error_buffer[256];
	const gchar* incl;
	gint i;
	mate_config_frame* current_frame;
	va_list list;

	va_start( list, fmt );
	g_vsnprintf(error_buffer,sizeof(error_buffer),fmt,list);
	va_end( list );

	i = (gint) mc->config_stack->len;

	while (i--) {

		if (i>0) {
			incl = "\n   included from: ";
		} else {
			incl = " ";
		}

		current_frame = (mate_config_frame *)g_ptr_array_index(mc->config_stack,(guint)i);

		g_string_append_printf(mc->config_error,"%s%s at line %u",incl, current_frame->filename, current_frame->linenum);
	}

	g_string_append_printf(mc->config_error,": %s\n",error_buffer);

	THROW(MateConfigError);

}

static AVPL_Transf* new_transform_elem(AVPL* match, AVPL* replace, avpl_match_mode match_mode, avpl_replace_mode replace_mode) {
	 AVPL_Transf* t = (AVPL_Transf *)g_malloc(sizeof(AVPL_Transf));

	 t->name = NULL;
	 t->match = match;
	 t->replace = replace;
	 t->match_mode = match_mode;
	 t->replace_mode = replace_mode;

	 t->map = NULL;
	 t->next = NULL;

	 return t;
}

static gchar* recolonize(mate_config* mc, gchar* s) {
	GString* str = g_string_new("");
	gchar** vec;
	gchar* r;
	guint i,v;
	gchar c;

	vec = g_strsplit(s,":",0);

	for (i = 0; vec[i]; i++) {
		g_ascii_strdown(vec[i], -1);

		v = 0;
		switch ( strlen(vec[i]) ) {
		 case 2:
		    c = vec[i][1];
			vec[i][1] = vec[i][0];
			vec[i][0] = c;
			if (vec[i][0] >= '0' && vec[i][0] <= '9') {
				v += (vec[i][1] - '0' )*16;
			} else {
				v += (vec[i][1] - 'a' + 10)*16;
			}
		 case 1:
			if (vec[i][0] >= '0' && vec[i][0] <= '9') {
				v += (vec[i][0] - '0' );
			} else {
				v += (vec[i][0] - 'a' + 10);
			}
		 case 0:
			break;
		  default:
			configuration_error(mc,"bad token %s",s);
		}

		g_string_append_printf(str,":%.2X",v);
	}

	g_strfreev(vec);

	g_string_erase(str,0,1);

	r = str->str;

	g_string_free(str,FALSE);

	return r;
}

#line 200 "mate_grammar.c"
/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/*
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands.
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash
**                       table.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    MateParserTOKENTYPE     is the data type used for minor tokens given
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is MateParserTOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    MateParserARG_SDECL     A static variable declaration for the %extra_argument
**    MateParserARG_PDECL     A parameter declaration for the %extra_argument
**    MateParserARG_STORE     Code to store %extra_argument into yypParser
**    MateParserARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define YYCODETYPE short
#define YYNOCODE 139
#define YYACTIONTYPE short
#define MateParserTOKENTYPE  gchar* 
typedef union {
  MateParserTOKENTYPE yy0;
  transf_match_t* yy16;
  pdu_criteria_t* yy41;
  AVPL_Transf* yy71;
  transf_action_t* yy77;
  float yy83;
  gchar* yy88;
  gop_tree_mode_t yy93;
  AVP* yy110;
  AVPL* yy170;
  gog_statement_t* yy171;
  avpl_match_mode yy172;
  accept_mode_t yy188;
  header_field_info* yy202;
  gop_options_t* yy203;
  avpl_replace_mode yy215;
  gboolean yy216;
  GPtrArray* yy231;
  LoAL* yy243;
  extraction_t* yy259;
  int yy277;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define MateParserARG_SDECL  mate_config* mc ;
#define MateParserARG_PDECL , mate_config* mc 
#define MateParserARG_FETCH  mate_config* mc  = yypParser->mc 
#define MateParserARG_STORE yypParser->mc  = mc 
#define YYNSTATE 287
#define YYNRULE 149
#define YYERRORSYMBOL 62
#define YYERRSYMDT yy277
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* The yyzerominor constant is used to initialize instances of
** YYMINORTYPE objects to zero. */
static const YYMINORTYPE yyzerominor = { 0 };

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif

/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
*/
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   287,  161,  275,  156,  217,  216,  212,  213,  197,  196,
 /*    10 */   102,  163,  141,  175,  153,  259,  257,  256,  254,  190,
 /*    20 */   189,  188,  242,   77,  119,  224,  245,  244,    7,  186,
 /*    30 */     4,  223,    4,  204,  280,  355,   98,  198,    6,  286,
 /*    40 */    45,   61,  101,  215,   68,   18,    3,  283,  209,    5,
 /*    50 */   195,  282,  281,  278,  277,  274,  272,  211,  210,  199,
 /*    60 */    71,  208,  220,  139,  145,   12,   99,   52,  104,  215,
 /*    70 */   250,  248,  238,  205,  128,   54,   73,  170,  155,   59,
 /*    80 */   202,  100,  181,  437,    1,   19,   65,  193,  194,  133,
 /*    90 */   118,  168,  143,  112,  203,  144,  171,  129,  218,   60,
 /*   100 */    14,  111,  134,  114,  162,  138,  146,  180,  124,  125,
 /*   110 */   173,   68,  239,  182,  151,  149,  174,    8,   43,  117,
 /*   120 */   179,  142,  285,   37,   44,  176,   27,   15,   46,   33,
 /*   130 */   135,   49,   57,   16,   42,  201,   34,   58,   28,   51,
 /*   140 */    10,   11,  172,  103,   20,   40,  159,  126,  207,  136,
 /*   150 */   166,  169,  109,  158,  147,  191,  165,  152,  206,  262,
 /*   160 */   123,  102,   75,   85,   53,   76,   29,  214,   55,   61,
 /*   170 */    12,  131,   92,   64,  219,  221,   66,  160,   67,   24,
 /*   180 */    26,   70,   25,   38,   30,   87,    2,  132,  121,   39,
 /*   190 */    88,   36,   89,   35,   91,  233,   31,   21,   80,   22,
 /*   200 */    78,   23,   81,   96,   82,   41,   17,   83,  184,   94,
 /*   210 */   107,   90,   84,   93,   97,  122,  155,  137,  183,  178,
 /*   220 */   263,   45,   18,  185,  177,   32,   47,  268,  148,  234,
 /*   230 */    48,  260,  261,  108,  229,  279,  276,   13,  258,  228,
 /*   240 */   110,  231,  235,  273,  269,  115,  227,  255,  267,   62,
 /*   250 */   226,  164,  270,   56,  187,   63,  237,  113,  116,  271,
 /*   260 */   236,  240,  150,   86,  154,  167,  157,  127,  225,  192,
 /*   270 */   232,  438,  253,  249,  120,  130,  243,   79,  200,   50,
 /*   280 */   222,  251,  241,  438,  266,  252,  438,  438,   72,   74,
 /*   290 */     9,  105,  264,  438,  230,   95,  438,  247,  106,  438,
 /*   300 */   284,  246,  438,  265,  140,   69,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */     0,    1,   63,    3,    7,    8,    2,   10,   29,   30,
 /*    10 */     8,   11,   12,   13,   14,   44,   45,   46,   47,   26,
 /*    20 */    27,   28,   47,   86,   24,    5,   51,   52,   65,   66,
 /*    30 */    67,   66,   67,    2,   97,   56,  106,  107,   96,   97,
 /*    40 */    53,   54,  102,  103,   40,   25,    4,   50,   81,   82,
 /*    50 */    81,  112,  113,  114,  115,  116,  117,   60,   61,   57,
 /*    60 */    55,   99,   57,    7,    8,   24,  104,   98,  102,  103,
 /*    70 */    48,   49,   10,    5,   68,    4,   70,  100,   37,   69,
 /*    80 */   108,  109,   75,  110,  111,   77,   55,   35,   36,   72,
 /*    90 */    75,   72,  100,   75,  108,   72,   72,   72,  108,   69,
 /*   100 */    58,   75,   75,   75,   75,   72,   72,   75,   12,   74,
 /*   110 */    74,   40,   50,   75,   75,   72,   13,   56,  124,   75,
 /*   120 */   125,  105,    8,   39,   71,   80,   17,   59,   79,   18,
 /*   130 */    11,  135,  133,   21,  123,    8,   16,  136,   34,   76,
 /*   140 */    33,  101,  105,    6,   23,   22,  137,    9,    8,   73,
 /*   150 */   103,  105,   11,  105,   12,  105,  105,  103,   99,    8,
 /*   160 */    13,    8,   84,   86,   42,   85,   22,  103,   43,   54,
 /*   170 */    24,  105,   98,  118,  107,  105,  119,   89,  120,   18,
 /*   180 */    20,  121,   19,   16,   22,   98,    4,  122,   64,   18,
 /*   190 */    90,   19,   91,   20,   92,    2,   18,   23,  129,   16,
 /*   200 */    95,   17,  130,   87,  131,   15,   21,  132,    2,  126,
 /*   210 */   128,   94,  133,  127,   88,  134,   37,   89,    2,    5,
 /*   220 */     2,   53,   25,    2,    5,   16,   78,    2,    8,    2,
 /*   230 */     4,    2,    2,    5,    2,    2,    2,   32,    2,    2,
 /*   240 */     9,    2,    2,    2,    2,   10,    2,    2,    2,   38,
 /*   250 */     2,    8,    2,   31,    2,    4,    2,   10,    9,    2,
 /*   260 */     2,    2,    9,    8,    8,    5,    5,    5,    2,    2,
 /*   270 */     2,  138,    2,    2,    5,   10,    2,    4,    2,    4,
 /*   280 */     2,    2,    2,  138,    2,    2,  138,  138,   25,    4,
 /*   290 */     4,   10,    2,  138,    2,    4,  138,    2,    5,  138,
 /*   300 */     2,    2,  138,    2,    8,   41,
};
#define YY_SHIFT_USE_DFLT (-30)
#define YY_SHIFT_MAX 182
static const short yy_shift_ofst[] = {
 /*     0 */   -30,    0,  197,   -3,  -21,   41,  -13,   20,    2,  168,
 /*    10 */   127,  179,  140,  127,   -3,   -3,  -29,  -29,   -7,   -7,
 /*    20 */   -25,  -25,   22,   22,   62,   62,   62,   22,   52,   22,
 /*    30 */    22,   62,   22,   62,   22,   62,   62,   22,   22,   62,
 /*    40 */    22,   22,   96,  103,   61,  114,   84,  109,  111,  112,
 /*    50 */   119,  120,  104,   61,  107,   61,  127,  121,  123,   61,
 /*    60 */    61,   61,  127,  137,  138,  140,  141,  142,  127,  151,
 /*    70 */   147,  153,   61,   61,  122,  125,  115,  146,  144,  161,
 /*    80 */   163,  160,  167,  112,  162,  146,  182,  171,  172,  173,
 /*    90 */   174,  209,  178,  184,  183,  190,  185,  144,    5,   31,
 /*   100 */    68,    4,   42,   56,   71,  225,  227,  228,  232,  231,
 /*   110 */   235,  237,  240,  242,  244,  246,  247,  248,  254,  255,
 /*   120 */   258,  266,  269,  249,  273,  259,  265,  250,  278,  280,
 /*   130 */   282,  285,  262,  268,  274,  291,  263,  293,  299,  301,
 /*   140 */   264,  296,  298,  295,  292,  290,  271,  253,  286,  283,
 /*   150 */   281,  279,  276,  275,  211,  256,  251,  270,  267,  261,
 /*   160 */   260,  257,  252,  243,  222,  245,  205,  241,  239,  236,
 /*   170 */   234,  233,  230,  229,  226,  220,  219,  221,  218,  214,
 /*   180 */   216,  206,  193,
};
#define YY_REDUCE_USE_DFLT (-71)
#define YY_REDUCE_MAX 97
static const short yy_reduce_ofst[] = {
 /*     0 */   -27,  -61,  -37,  -28,    6,  -31,  -63,  -35,  -70,  -58,
 /*    10 */   -60,  -33,  -38,  -34,  -10,  -14,   36,   35,   30,   10,
 /*    20 */    -8,  -23,   28,   26,   23,   19,   17,    7,    8,   15,
 /*    30 */    18,   24,   27,   25,   29,   33,   34,   32,   38,   43,
 /*    40 */    39,   44,   -6,   -5,   16,   53,   45,   49,   -4,   -1,
 /*    50 */    11,  148,   63,   37,   40,   46,   47,    1,    9,   48,
 /*    60 */    50,   51,   54,   55,   57,   59,   58,   60,   64,   76,
 /*    70 */    65,   67,   66,   70,   78,   80,   77,   74,   88,   69,
 /*    80 */    72,   73,   75,   79,   81,   87,  124,  100,  101,  102,
 /*    90 */   105,  116,  117,   82,   86,   83,  126,  128,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   289,  436,  346,  436,  347,  417,  415,  346,  436,  436,
 /*    10 */   436,  436,  436,  436,  436,  436,  436,  436,  349,  349,
 /*    20 */   436,  436,  436,  436,  436,  436,  436,  436,  361,  436,
 /*    30 */   436,  436,  436,  436,  436,  436,  436,  436,  436,  436,
 /*    40 */   436,  436,  319,  333,  436,  436,  372,  370,  335,  329,
 /*    50 */   311,  368,  359,  436,  357,  436,  436,  337,  339,  436,
 /*    60 */   436,  436,  436,  300,  302,  436,  304,  306,  436,  436,
 /*    70 */   308,  436,  436,  436,  380,  382,  415,  417,  386,  321,
 /*    80 */   323,  325,  327,  329,  331,  417,  436,  388,  390,  392,
 /*    90 */   404,  378,  406,  317,  315,  313,  384,  386,  436,  436,
 /*   100 */   436,  436,  426,  436,  436,  436,  436,  436,  436,  436,
 /*   110 */   436,  436,  436,  436,  436,  436,  436,  436,  436,  436,
 /*   120 */   436,  436,  436,  436,  436,  436,  436,  436,  436,  436,
 /*   130 */   436,  436,  436,  436,  436,  436,  436,  436,  436,  436,
 /*   140 */   436,  436,  436,  436,  436,  436,  436,  436,  436,  436,
 /*   150 */   436,  436,  436,  436,  436,  436,  436,  436,  436,  436,
 /*   160 */   436,  436,  436,  436,  436,  436,  436,  436,  436,  436,
 /*   170 */   436,  436,  436,  436,  436,  436,  436,  436,  436,  436,
 /*   180 */   436,  436,  436,  371,  369,  356,  343,  367,  352,  351,
 /*   190 */   350,  345,  360,  362,  363,  364,  354,  353,  424,  422,
 /*   200 */   366,  375,  429,  428,  416,  427,  418,  420,  419,  365,
 /*   210 */   435,  434,  358,  433,  373,  374,  431,  430,  425,  423,
 /*   220 */   421,  348,  344,  342,  341,  340,  312,  314,  316,  310,
 /*   230 */   320,  322,  324,  326,  376,  330,  318,  385,  401,  400,
 /*   240 */   383,  334,  409,  377,  408,  407,  391,  336,  398,  389,
 /*   250 */   397,  338,  387,  332,  396,  414,  395,  394,  381,  393,
 /*   260 */   328,  379,  399,  309,  299,  298,  301,  303,  305,  307,
 /*   270 */   297,  296,  295,  402,  294,  293,  403,  292,  291,  405,
 /*   280 */   410,  290,  288,  432,  412,  413,  411,
};
#define YY_SZ_ACTTAB (int)(sizeof(yy_action)/sizeof(yy_action[0]))

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
**
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyidxMax;                 /* Maximum value of yyidx */
#endif
  int yyerrcnt;                 /* Shifts left before out of the error */
  MateParserARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/*
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void MateParserTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ){
    yyTracePrompt = 0;
  }else if( yyTracePrompt==0 ){
    yyTraceFILE = 0;
  }
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = {
  "$",             "DONE_KW",       "SEMICOLON",     "DEBUG_KW",    
  "OPEN_BRACE",    "CLOSE_BRACE",   "FILENAME_KW",   "QUOTED",      
  "NAME",          "LEVEL_KW",      "INTEGER",       "PDU_KW",      
  "GOP_KW",        "GOG_KW",        "DEFAULT_KW",    "LAST_EXTRACTED_KW",
  "DROP_UNASSIGNED_KW",  "DISCARD_PDU_DATA_KW",  "EXPIRATION_KW",  "IDLE_TIMEOUT_KW",
  "LIFETIME_KW",   "SHOW_TREE_KW",  "SHOW_TIMES_KW",  "GOP_TREE_KW", 
  "TRANSFORM_KW",  "MATCH_KW",      "STRICT_KW",     "EVERY_KW",    
  "LOOSE_KW",      "REPLACE_KW",    "INSERT_KW",     "PROTO_KW",    
  "TRANSPORT_KW",  "PAYLOAD_KW",    "CRITERIA_KW",   "ACCEPT_KW",   
  "REJECT_KW",     "EXTRACT_KW",    "FROM_KW",       "LAST_PDU_KW", 
  "SLASH",         "ON_KW",         "START_KW",      "STOP_KW",     
  "NO_TREE_KW",    "PDU_TREE_KW",   "FRAME_TREE_KW",  "BASIC_TREE_KW",
  "TRUE_KW",       "FALSE_KW",      "FLOATING",      "NULL_TREE_KW",
  "FULL_TREE_KW",  "MEMBER_KW",     "EXTRA_KW",      "COMMA",       
  "OPEN_PARENS",   "CLOSE_PARENS",  "AVP_OPERATOR",  "PIPE",        
  "DOTED_IP",      "COLONIZED",     "error",         "transform_decl",
  "transform_body",  "transform_statements",  "transform_statement",  "transform_match",
  "transform_action",  "match_mode",    "action_mode",   "gop_name",    
  "time_value",    "pdu_name",      "gop_tree_mode",  "true_false",  
  "criteria_statement",  "accept_mode",   "pdu_drop_unassigned_statement",  "discard_pdu_data_statement",
  "last_extracted_statement",  "extraction_statement",  "extraction_statements",  "gop_options", 
  "gop_start_statement",  "gop_stop_statement",  "extra_statement",  "gop_drop_unassigned_statement",
  "show_goptree_statement",  "show_times_statement",  "gop_expiration_statement",  "idle_timeout_statement",
  "lifetime_statement",  "gog_statements",  "gog_expiration_statement",  "gog_goptree_statement",
  "gog_key_statements",  "gog_key_statement",  "transform_list_statement",  "transform",   
  "gop_tree_type",  "payload_statement",  "proto_stack",   "field",       
  "transform_list",  "avpl",          "avps",          "avp",         
  "value",         "avp_oneoff",    "mate_config",   "decls",       
  "decl",          "pdu_decl",      "gop_decl",      "gog_decl",    
  "defaults_decl",  "debug_decl",    "dbgfile_default",  "dbglevel_default",
  "pdu_dbglevel_default",  "gop_dbglevel_default",  "gog_dbglevel_default",  "pdu_defaults",
  "gop_defaults",  "gog_defaults",  "pdu_last_extracted_default",  "pdu_drop_unassigned_default",
  "pdu_discard_default",  "gop_expiration_default",  "gop_idle_timeout_default",  "gop_lifetime_default",
  "gop_drop_unassigned_default",  "gop_tree_mode_default",  "gop_show_times_default",  "gog_expiration_default",
  "gog_goptree_default",  "gog_show_times_default",
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "mate_config ::= decls",
 /*   1 */ "decls ::= decls decl",
 /*   2 */ "decls ::=",
 /*   3 */ "decl ::= pdu_decl",
 /*   4 */ "decl ::= gop_decl",
 /*   5 */ "decl ::= gog_decl",
 /*   6 */ "decl ::= transform_decl",
 /*   7 */ "decl ::= defaults_decl",
 /*   8 */ "decl ::= debug_decl",
 /*   9 */ "decl ::= DONE_KW SEMICOLON",
 /*  10 */ "debug_decl ::= DEBUG_KW OPEN_BRACE dbgfile_default dbglevel_default pdu_dbglevel_default gop_dbglevel_default gog_dbglevel_default CLOSE_BRACE SEMICOLON",
 /*  11 */ "dbgfile_default ::= FILENAME_KW QUOTED SEMICOLON",
 /*  12 */ "dbgfile_default ::= FILENAME_KW NAME SEMICOLON",
 /*  13 */ "dbgfile_default ::=",
 /*  14 */ "dbglevel_default ::= LEVEL_KW INTEGER SEMICOLON",
 /*  15 */ "dbglevel_default ::=",
 /*  16 */ "pdu_dbglevel_default ::= PDU_KW LEVEL_KW INTEGER SEMICOLON",
 /*  17 */ "pdu_dbglevel_default ::=",
 /*  18 */ "gop_dbglevel_default ::= GOP_KW LEVEL_KW INTEGER SEMICOLON",
 /*  19 */ "gop_dbglevel_default ::=",
 /*  20 */ "gog_dbglevel_default ::= GOG_KW LEVEL_KW INTEGER SEMICOLON",
 /*  21 */ "gog_dbglevel_default ::=",
 /*  22 */ "defaults_decl ::= DEFAULT_KW OPEN_BRACE pdu_defaults gop_defaults gog_defaults CLOSE_BRACE SEMICOLON",
 /*  23 */ "pdu_defaults ::= PDU_KW OPEN_BRACE pdu_last_extracted_default pdu_drop_unassigned_default pdu_discard_default CLOSE_BRACE SEMICOLON",
 /*  24 */ "pdu_defaults ::=",
 /*  25 */ "pdu_last_extracted_default ::= LAST_EXTRACTED_KW true_false SEMICOLON",
 /*  26 */ "pdu_last_extracted_default ::=",
 /*  27 */ "pdu_drop_unassigned_default ::= DROP_UNASSIGNED_KW true_false SEMICOLON",
 /*  28 */ "pdu_drop_unassigned_default ::=",
 /*  29 */ "pdu_discard_default ::= DISCARD_PDU_DATA_KW true_false SEMICOLON",
 /*  30 */ "pdu_discard_default ::=",
 /*  31 */ "gop_defaults ::= GOP_KW OPEN_BRACE gop_expiration_default gop_idle_timeout_default gop_lifetime_default gop_drop_unassigned_default gop_tree_mode_default gop_show_times_default CLOSE_BRACE SEMICOLON",
 /*  32 */ "gop_defaults ::=",
 /*  33 */ "gop_expiration_default ::= EXPIRATION_KW time_value SEMICOLON",
 /*  34 */ "gop_expiration_default ::=",
 /*  35 */ "gop_idle_timeout_default ::= IDLE_TIMEOUT_KW time_value SEMICOLON",
 /*  36 */ "gop_idle_timeout_default ::=",
 /*  37 */ "gop_lifetime_default ::= LIFETIME_KW time_value SEMICOLON",
 /*  38 */ "gop_lifetime_default ::=",
 /*  39 */ "gop_drop_unassigned_default ::= DROP_UNASSIGNED_KW true_false SEMICOLON",
 /*  40 */ "gop_drop_unassigned_default ::=",
 /*  41 */ "gop_tree_mode_default ::= SHOW_TREE_KW gop_tree_mode SEMICOLON",
 /*  42 */ "gop_tree_mode_default ::=",
 /*  43 */ "gop_show_times_default ::= SHOW_TIMES_KW true_false SEMICOLON",
 /*  44 */ "gop_show_times_default ::=",
 /*  45 */ "gog_defaults ::= GOG_KW OPEN_BRACE gog_expiration_default gop_tree_mode_default gog_goptree_default gog_show_times_default CLOSE_BRACE SEMICOLON",
 /*  46 */ "gog_defaults ::=",
 /*  47 */ "gog_expiration_default ::= EXPIRATION_KW time_value SEMICOLON",
 /*  48 */ "gog_expiration_default ::=",
 /*  49 */ "gog_goptree_default ::= GOP_TREE_KW gop_tree_type SEMICOLON",
 /*  50 */ "gog_goptree_default ::=",
 /*  51 */ "gog_show_times_default ::= SHOW_TIMES_KW true_false SEMICOLON",
 /*  52 */ "gog_show_times_default ::=",
 /*  53 */ "transform_decl ::= TRANSFORM_KW NAME transform_body SEMICOLON",
 /*  54 */ "transform_body ::= OPEN_BRACE transform_statements CLOSE_BRACE",
 /*  55 */ "transform_statements ::= transform_statements transform_statement",
 /*  56 */ "transform_statements ::= transform_statement",
 /*  57 */ "transform_statement ::= transform_match transform_action SEMICOLON",
 /*  58 */ "transform_match ::= MATCH_KW match_mode avpl",
 /*  59 */ "transform_match ::=",
 /*  60 */ "transform_action ::=",
 /*  61 */ "transform_action ::= action_mode avpl",
 /*  62 */ "match_mode ::=",
 /*  63 */ "match_mode ::= STRICT_KW",
 /*  64 */ "match_mode ::= EVERY_KW",
 /*  65 */ "match_mode ::= LOOSE_KW",
 /*  66 */ "action_mode ::= REPLACE_KW",
 /*  67 */ "action_mode ::= INSERT_KW",
 /*  68 */ "action_mode ::=",
 /*  69 */ "pdu_decl ::= PDU_KW NAME PROTO_KW field TRANSPORT_KW proto_stack OPEN_BRACE payload_statement extraction_statements transform_list_statement criteria_statement pdu_drop_unassigned_statement discard_pdu_data_statement last_extracted_statement CLOSE_BRACE SEMICOLON",
 /*  70 */ "payload_statement ::=",
 /*  71 */ "payload_statement ::= PAYLOAD_KW proto_stack SEMICOLON",
 /*  72 */ "criteria_statement ::=",
 /*  73 */ "criteria_statement ::= CRITERIA_KW accept_mode match_mode avpl SEMICOLON",
 /*  74 */ "accept_mode ::=",
 /*  75 */ "accept_mode ::= ACCEPT_KW",
 /*  76 */ "accept_mode ::= REJECT_KW",
 /*  77 */ "extraction_statements ::= extraction_statements extraction_statement",
 /*  78 */ "extraction_statements ::= extraction_statement",
 /*  79 */ "extraction_statement ::= EXTRACT_KW NAME FROM_KW field SEMICOLON",
 /*  80 */ "pdu_drop_unassigned_statement ::= DROP_UNASSIGNED_KW true_false SEMICOLON",
 /*  81 */ "pdu_drop_unassigned_statement ::=",
 /*  82 */ "discard_pdu_data_statement ::= DISCARD_PDU_DATA_KW true_false SEMICOLON",
 /*  83 */ "discard_pdu_data_statement ::=",
 /*  84 */ "last_extracted_statement ::= LAST_PDU_KW true_false SEMICOLON",
 /*  85 */ "last_extracted_statement ::=",
 /*  86 */ "proto_stack ::= proto_stack SLASH field",
 /*  87 */ "proto_stack ::= field",
 /*  88 */ "field ::= NAME",
 /*  89 */ "gop_decl ::= GOP_KW NAME ON_KW pdu_name MATCH_KW avpl OPEN_BRACE gop_start_statement gop_stop_statement extra_statement transform_list_statement gop_expiration_statement idle_timeout_statement lifetime_statement gop_drop_unassigned_statement show_goptree_statement show_times_statement CLOSE_BRACE SEMICOLON",
 /*  90 */ "gop_drop_unassigned_statement ::= DROP_UNASSIGNED_KW true_false SEMICOLON",
 /*  91 */ "gop_drop_unassigned_statement ::=",
 /*  92 */ "gop_start_statement ::= START_KW avpl SEMICOLON",
 /*  93 */ "gop_start_statement ::=",
 /*  94 */ "gop_stop_statement ::= STOP_KW avpl SEMICOLON",
 /*  95 */ "gop_stop_statement ::=",
 /*  96 */ "show_goptree_statement ::= SHOW_TREE_KW gop_tree_mode SEMICOLON",
 /*  97 */ "show_goptree_statement ::=",
 /*  98 */ "show_times_statement ::= SHOW_TIMES_KW true_false SEMICOLON",
 /*  99 */ "show_times_statement ::=",
 /* 100 */ "gop_expiration_statement ::= EXPIRATION_KW time_value SEMICOLON",
 /* 101 */ "gop_expiration_statement ::=",
 /* 102 */ "idle_timeout_statement ::= IDLE_TIMEOUT_KW time_value SEMICOLON",
 /* 103 */ "idle_timeout_statement ::=",
 /* 104 */ "lifetime_statement ::= LIFETIME_KW time_value SEMICOLON",
 /* 105 */ "lifetime_statement ::=",
 /* 106 */ "gop_tree_mode ::= NO_TREE_KW",
 /* 107 */ "gop_tree_mode ::= PDU_TREE_KW",
 /* 108 */ "gop_tree_mode ::= FRAME_TREE_KW",
 /* 109 */ "gop_tree_mode ::= BASIC_TREE_KW",
 /* 110 */ "true_false ::= TRUE_KW",
 /* 111 */ "true_false ::= FALSE_KW",
 /* 112 */ "pdu_name ::= NAME",
 /* 113 */ "time_value ::= FLOATING",
 /* 114 */ "time_value ::= INTEGER",
 /* 115 */ "gog_decl ::= GOG_KW NAME OPEN_BRACE gog_key_statements extra_statement transform_list_statement gog_expiration_statement gog_goptree_statement show_times_statement CLOSE_BRACE SEMICOLON",
 /* 116 */ "gog_goptree_statement ::= GOP_TREE_KW gop_tree_type SEMICOLON",
 /* 117 */ "gog_goptree_statement ::=",
 /* 118 */ "gog_expiration_statement ::= EXPIRATION_KW time_value SEMICOLON",
 /* 119 */ "gog_expiration_statement ::=",
 /* 120 */ "gop_tree_type ::= NULL_TREE_KW",
 /* 121 */ "gop_tree_type ::= FULL_TREE_KW",
 /* 122 */ "gop_tree_type ::= BASIC_TREE_KW",
 /* 123 */ "gog_key_statements ::= gog_key_statements gog_key_statement",
 /* 124 */ "gog_key_statements ::= gog_key_statement",
 /* 125 */ "gog_key_statement ::= MEMBER_KW gop_name avpl SEMICOLON",
 /* 126 */ "gop_name ::= NAME",
 /* 127 */ "extra_statement ::= EXTRA_KW avpl SEMICOLON",
 /* 128 */ "extra_statement ::=",
 /* 129 */ "transform_list_statement ::= TRANSFORM_KW transform_list SEMICOLON",
 /* 130 */ "transform_list_statement ::=",
 /* 131 */ "transform_list ::= transform_list COMMA transform",
 /* 132 */ "transform_list ::= transform",
 /* 133 */ "transform ::= NAME",
 /* 134 */ "avpl ::= OPEN_PARENS avps CLOSE_PARENS",
 /* 135 */ "avpl ::= OPEN_PARENS CLOSE_PARENS",
 /* 136 */ "avps ::= avps COMMA avp",
 /* 137 */ "avps ::= avp",
 /* 138 */ "avp ::= NAME AVP_OPERATOR value",
 /* 139 */ "avp ::= NAME",
 /* 140 */ "avp ::= NAME OPEN_BRACE avp_oneoff CLOSE_BRACE",
 /* 141 */ "avp_oneoff ::= avp_oneoff PIPE value",
 /* 142 */ "avp_oneoff ::= value",
 /* 143 */ "value ::= QUOTED",
 /* 144 */ "value ::= NAME",
 /* 145 */ "value ::= FLOATING",
 /* 146 */ "value ::= INTEGER",
 /* 147 */ "value ::= DOTED_IP",
 /* 148 */ "value ::= COLONIZED",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void yyGrowStack(yyParser *p){
  int newSize;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->yystack = pNew;
    p->yystksz = newSize;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
}
#endif

/*
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to MateParser and MateParserFree.
*/
void *MateParserAlloc(void *(*mallocProc)(gsize)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (gsize)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyidxMax = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    yyGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(yyParser *yypParser, YYCODETYPE yymajor, YYMINORTYPE *yypminor){
  MateParserARG_FETCH;
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
      /* TERMINAL Destructor */
    case 1: /* DONE_KW */
    case 2: /* SEMICOLON */
    case 3: /* DEBUG_KW */
    case 4: /* OPEN_BRACE */
    case 5: /* CLOSE_BRACE */
    case 6: /* FILENAME_KW */
    case 7: /* QUOTED */
    case 8: /* NAME */
    case 9: /* LEVEL_KW */
    case 10: /* INTEGER */
    case 11: /* PDU_KW */
    case 12: /* GOP_KW */
    case 13: /* GOG_KW */
    case 14: /* DEFAULT_KW */
    case 15: /* LAST_EXTRACTED_KW */
    case 16: /* DROP_UNASSIGNED_KW */
    case 17: /* DISCARD_PDU_DATA_KW */
    case 18: /* EXPIRATION_KW */
    case 19: /* IDLE_TIMEOUT_KW */
    case 20: /* LIFETIME_KW */
    case 21: /* SHOW_TREE_KW */
    case 22: /* SHOW_TIMES_KW */
    case 23: /* GOP_TREE_KW */
    case 24: /* TRANSFORM_KW */
    case 25: /* MATCH_KW */
    case 26: /* STRICT_KW */
    case 27: /* EVERY_KW */
    case 28: /* LOOSE_KW */
    case 29: /* REPLACE_KW */
    case 30: /* INSERT_KW */
    case 31: /* PROTO_KW */
    case 32: /* TRANSPORT_KW */
    case 33: /* PAYLOAD_KW */
    case 34: /* CRITERIA_KW */
    case 35: /* ACCEPT_KW */
    case 36: /* REJECT_KW */
    case 37: /* EXTRACT_KW */
    case 38: /* FROM_KW */
    case 39: /* LAST_PDU_KW */
    case 40: /* SLASH */
    case 41: /* ON_KW */
    case 42: /* START_KW */
    case 43: /* STOP_KW */
    case 44: /* NO_TREE_KW */
    case 45: /* PDU_TREE_KW */
    case 46: /* FRAME_TREE_KW */
    case 47: /* BASIC_TREE_KW */
    case 48: /* TRUE_KW */
    case 49: /* FALSE_KW */
    case 50: /* FLOATING */
    case 51: /* NULL_TREE_KW */
    case 52: /* FULL_TREE_KW */
    case 53: /* MEMBER_KW */
    case 54: /* EXTRA_KW */
    case 55: /* COMMA */
    case 56: /* OPEN_PARENS */
    case 57: /* CLOSE_PARENS */
    case 58: /* AVP_OPERATOR */
    case 59: /* PIPE */
    case 60: /* DOTED_IP */
    case 61: /* COLONIZED */
{
#line 179 "mate_grammar.lemon"

	(void) mc; /* Mark unused, similar to Q_UNUSED */
	if ((yypminor->yy0)) g_free((yypminor->yy0));

#line 918 "mate_grammar.c"
}
      break;
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int yy_pop_parser_stack(yyParser *pParser){
  YYCODETYPE yymajor;
  yyStackEntry *yytos;

  if( pParser->yyidx<0 ){
    return 0;
  }
  yytos = &pParser->yystack[pParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor(pParser, yymajor, &yytos->minor);
  pParser->yyidx--;
  return yymajor;
}

/*
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from MateParserAlloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void MateParserFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
  if( pParser==0 ){
    return;
  }
  while( pParser->yyidx>=0 ){
    yy_pop_parser_stack(pParser);
  }
#if YYSTACKDEPTH<=0
  free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int MateParserStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyidxMax;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;

  if( stateno>YY_SHIFT_MAX
    || (i = yy_shift_ofst[stateno])==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    if( iLookAhead>0 ){
#ifdef YYFALLBACK
      YYCODETYPE iFallback;            /* Fallback token */
      if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
             && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
             yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
        }
#endif
        return yy_find_shift_action(pParser, iFallback);
      }
#endif
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if( j>=0 && j<YY_SZ_ACTTAB && yy_lookahead[j]==YYWILDCARD ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
    }
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_MAX ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_MAX );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_SZ_ACTTAB );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser, YYMINORTYPE *yypMinor _U_){
   MateParserARG_FETCH;
   yypParser->yyidx--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yyidx>=0 ) {
     yy_pop_parser_stack(yypParser);
   }
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
   MateParserARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer to the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( yypParser->yyidx>yypParser->yyidxMax ){
    yypParser->yyidxMax = yypParser->yyidx;
  }
#endif
#if YYSTACKDEPTH>0
  if( yypParser->yyidx>=YYSTACKDEPTH ){
    yyStackOverflow(yypParser, yypMinor);
    return;
  }
#else
  if( yypParser->yyidx>=yypParser->yystksz ){
    yyGrowStack(yypParser);
    if( yypParser->yyidx>=yypParser->yystksz ){
      yyStackOverflow(yypParser, yypMinor);
      return;
    }
  }
#endif
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
  yytos->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->yyidx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 110, 1 },
  { 111, 2 },
  { 111, 0 },
  { 112, 1 },
  { 112, 1 },
  { 112, 1 },
  { 112, 1 },
  { 112, 1 },
  { 112, 1 },
  { 112, 2 },
  { 117, 9 },
  { 118, 3 },
  { 118, 3 },
  { 118, 0 },
  { 119, 3 },
  { 119, 0 },
  { 120, 4 },
  { 120, 0 },
  { 121, 4 },
  { 121, 0 },
  { 122, 4 },
  { 122, 0 },
  { 116, 7 },
  { 123, 7 },
  { 123, 0 },
  { 126, 3 },
  { 126, 0 },
  { 127, 3 },
  { 127, 0 },
  { 128, 3 },
  { 128, 0 },
  { 124, 10 },
  { 124, 0 },
  { 129, 3 },
  { 129, 0 },
  { 130, 3 },
  { 130, 0 },
  { 131, 3 },
  { 131, 0 },
  { 132, 3 },
  { 132, 0 },
  { 133, 3 },
  { 133, 0 },
  { 134, 3 },
  { 134, 0 },
  { 125, 8 },
  { 125, 0 },
  { 135, 3 },
  { 135, 0 },
  { 136, 3 },
  { 136, 0 },
  { 137, 3 },
  { 137, 0 },
  { 63, 4 },
  { 64, 3 },
  { 65, 2 },
  { 65, 1 },
  { 66, 3 },
  { 67, 3 },
  { 67, 0 },
  { 68, 0 },
  { 68, 2 },
  { 69, 0 },
  { 69, 1 },
  { 69, 1 },
  { 69, 1 },
  { 70, 1 },
  { 70, 1 },
  { 70, 0 },
  { 113, 16 },
  { 101, 0 },
  { 101, 3 },
  { 76, 0 },
  { 76, 5 },
  { 77, 0 },
  { 77, 1 },
  { 77, 1 },
  { 82, 2 },
  { 82, 1 },
  { 81, 5 },
  { 78, 3 },
  { 78, 0 },
  { 79, 3 },
  { 79, 0 },
  { 80, 3 },
  { 80, 0 },
  { 102, 3 },
  { 102, 1 },
  { 103, 1 },
  { 114, 19 },
  { 87, 3 },
  { 87, 0 },
  { 84, 3 },
  { 84, 0 },
  { 85, 3 },
  { 85, 0 },
  { 88, 3 },
  { 88, 0 },
  { 89, 3 },
  { 89, 0 },
  { 90, 3 },
  { 90, 0 },
  { 91, 3 },
  { 91, 0 },
  { 92, 3 },
  { 92, 0 },
  { 74, 1 },
  { 74, 1 },
  { 74, 1 },
  { 74, 1 },
  { 75, 1 },
  { 75, 1 },
  { 73, 1 },
  { 72, 1 },
  { 72, 1 },
  { 115, 11 },
  { 95, 3 },
  { 95, 0 },
  { 94, 3 },
  { 94, 0 },
  { 100, 1 },
  { 100, 1 },
  { 100, 1 },
  { 96, 2 },
  { 96, 1 },
  { 97, 4 },
  { 71, 1 },
  { 86, 3 },
  { 86, 0 },
  { 98, 3 },
  { 98, 0 },
  { 104, 3 },
  { 104, 1 },
  { 99, 1 },
  { 105, 3 },
  { 105, 2 },
  { 106, 3 },
  { 106, 1 },
  { 107, 3 },
  { 107, 1 },
  { 107, 4 },
  { 109, 3 },
  { 109, 1 },
  { 108, 1 },
  { 108, 1 },
  { 108, 1 },
  { 108, 1 },
  { 108, 1 },
  { 108, 1 },
};

static void yy_accept(yyParser *yypParser);  /* Forward declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  MateParserARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0
        && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
      yyRuleName[yyruleno]);
  }
#endif /* NDEBUG */

  /* Silence complaints from purify about yygotominor being uninitialized
  ** in some cases when it is copied into the stack after the following
  ** switch.  yygotominor is uninitialized when a rule reduces that does
  ** not set the value of its left-hand side nonterminal.  Leaving the
  ** value of the nonterminal uninitialized is utterly harmless as long
  ** as the value is never used.  So really the only thing this code
  ** accomplishes is to quieten purify.
  **
  ** 2007-01-16:  The wireshark project (www.wireshark.org) reports that
  ** without this code, their parser segfaults.  I'm not sure what there
  ** parser is doing to make this happen.  This is the second bug report
  ** from wireshark this week.  Clearly they are stressing Lemon in ways
  ** that it has not been previously stressed...  (SQLite ticket #2172)
  */
  /*memset(&yygotominor, 0, sizeof(yygotominor));*/
  yygotominor = yyzerominor;
  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 0: /* mate_config ::= decls */
      case 1: /* decls ::= decls decl */
      case 2: /* decls ::= */
      case 3: /* decl ::= pdu_decl */
      case 4: /* decl ::= gop_decl */
      case 5: /* decl ::= gog_decl */
      case 6: /* decl ::= transform_decl */
      case 7: /* decl ::= defaults_decl */
      case 8: /* decl ::= debug_decl */
      case 13: /* dbgfile_default ::= */
      case 15: /* dbglevel_default ::= */
      case 17: /* pdu_dbglevel_default ::= */
      case 19: /* gop_dbglevel_default ::= */
      case 21: /* gog_dbglevel_default ::= */
      case 24: /* pdu_defaults ::= */
      case 26: /* pdu_last_extracted_default ::= */
      case 28: /* pdu_drop_unassigned_default ::= */
      case 30: /* pdu_discard_default ::= */
      case 32: /* gop_defaults ::= */
      case 34: /* gop_expiration_default ::= */
      case 36: /* gop_idle_timeout_default ::= */
      case 38: /* gop_lifetime_default ::= */
      case 40: /* gop_drop_unassigned_default ::= */
      case 42: /* gop_tree_mode_default ::= */
      case 44: /* gop_show_times_default ::= */
      case 46: /* gog_defaults ::= */
      case 48: /* gog_expiration_default ::= */
      case 50: /* gog_goptree_default ::= */
      case 52: /* gog_show_times_default ::= */
#line 250 "mate_grammar.lemon"
{
}
#line 1389 "mate_grammar.c"
        break;
      case 9: /* decl ::= DONE_KW SEMICOLON */
#line 261 "mate_grammar.lemon"
{
  yy_destructor(yypParser,1,&yymsp[-1].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1397 "mate_grammar.c"
        break;
      case 10: /* debug_decl ::= DEBUG_KW OPEN_BRACE dbgfile_default dbglevel_default pdu_dbglevel_default gop_dbglevel_default gog_dbglevel_default CLOSE_BRACE SEMICOLON */
#line 266 "mate_grammar.lemon"
{
  yy_destructor(yypParser,3,&yymsp[-8].minor);
  yy_destructor(yypParser,4,&yymsp[-7].minor);
  yy_destructor(yypParser,5,&yymsp[-1].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1407 "mate_grammar.c"
        break;
      case 11: /* dbgfile_default ::= FILENAME_KW QUOTED SEMICOLON */
#line 268 "mate_grammar.lemon"
{ mc->dbg_facility = ws_fopen(yymsp[-1].minor.yy0,"w"); if (mc->dbg_facility == NULL) report_open_failure(yymsp[-1].minor.yy0,errno,TRUE);   yy_destructor(yypParser,6,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1414 "mate_grammar.c"
        break;
      case 12: /* dbgfile_default ::= FILENAME_KW NAME SEMICOLON */
#line 269 "mate_grammar.lemon"
{ mc->dbg_facility = ws_fopen(yymsp[-1].minor.yy0,"w"); if (mc->dbg_facility == NULL) report_open_failure(yymsp[-1].minor.yy0,errno,TRUE);    yy_destructor(yypParser,6,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1421 "mate_grammar.c"
        break;
      case 14: /* dbglevel_default ::= LEVEL_KW INTEGER SEMICOLON */
#line 272 "mate_grammar.lemon"
{ mc->dbg_lvl = (int) strtol(yymsp[-1].minor.yy0,NULL,10);   yy_destructor(yypParser,9,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1428 "mate_grammar.c"
        break;
      case 16: /* pdu_dbglevel_default ::= PDU_KW LEVEL_KW INTEGER SEMICOLON */
#line 275 "mate_grammar.lemon"
{ mc->dbg_pdu_lvl = (int) strtol(yymsp[-1].minor.yy0,NULL,10);   yy_destructor(yypParser,11,&yymsp[-3].minor);
  yy_destructor(yypParser,9,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1436 "mate_grammar.c"
        break;
      case 18: /* gop_dbglevel_default ::= GOP_KW LEVEL_KW INTEGER SEMICOLON */
#line 278 "mate_grammar.lemon"
{ mc->dbg_gop_lvl = (int) strtol(yymsp[-1].minor.yy0,NULL,10);   yy_destructor(yypParser,12,&yymsp[-3].minor);
  yy_destructor(yypParser,9,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1444 "mate_grammar.c"
        break;
      case 20: /* gog_dbglevel_default ::= GOG_KW LEVEL_KW INTEGER SEMICOLON */
#line 281 "mate_grammar.lemon"
{ mc->dbg_gog_lvl = (int) strtol(yymsp[-1].minor.yy0,NULL,10);   yy_destructor(yypParser,13,&yymsp[-3].minor);
  yy_destructor(yypParser,9,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1452 "mate_grammar.c"
        break;
      case 22: /* defaults_decl ::= DEFAULT_KW OPEN_BRACE pdu_defaults gop_defaults gog_defaults CLOSE_BRACE SEMICOLON */
#line 288 "mate_grammar.lemon"
{
  yy_destructor(yypParser,14,&yymsp[-6].minor);
  yy_destructor(yypParser,4,&yymsp[-5].minor);
  yy_destructor(yypParser,5,&yymsp[-1].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1462 "mate_grammar.c"
        break;
      case 23: /* pdu_defaults ::= PDU_KW OPEN_BRACE pdu_last_extracted_default pdu_drop_unassigned_default pdu_discard_default CLOSE_BRACE SEMICOLON */
#line 290 "mate_grammar.lemon"
{
  yy_destructor(yypParser,11,&yymsp[-6].minor);
  yy_destructor(yypParser,4,&yymsp[-5].minor);
  yy_destructor(yypParser,5,&yymsp[-1].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1472 "mate_grammar.c"
        break;
      case 25: /* pdu_last_extracted_default ::= LAST_EXTRACTED_KW true_false SEMICOLON */
#line 293 "mate_grammar.lemon"
{ mc->defaults.pdu.last_extracted = yymsp[-1].minor.yy216;   yy_destructor(yypParser,15,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1479 "mate_grammar.c"
        break;
      case 27: /* pdu_drop_unassigned_default ::= DROP_UNASSIGNED_KW true_false SEMICOLON */
#line 296 "mate_grammar.lemon"
{ mc->defaults.pdu.drop_unassigned = yymsp[-1].minor.yy216;   yy_destructor(yypParser,16,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1486 "mate_grammar.c"
        break;
      case 29: /* pdu_discard_default ::= DISCARD_PDU_DATA_KW true_false SEMICOLON */
#line 299 "mate_grammar.lemon"
{ mc->defaults.pdu.discard = yymsp[-1].minor.yy216;   yy_destructor(yypParser,17,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1493 "mate_grammar.c"
        break;
      case 31: /* gop_defaults ::= GOP_KW OPEN_BRACE gop_expiration_default gop_idle_timeout_default gop_lifetime_default gop_drop_unassigned_default gop_tree_mode_default gop_show_times_default CLOSE_BRACE SEMICOLON */
#line 302 "mate_grammar.lemon"
{
  yy_destructor(yypParser,12,&yymsp[-9].minor);
  yy_destructor(yypParser,4,&yymsp[-8].minor);
  yy_destructor(yypParser,5,&yymsp[-1].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1503 "mate_grammar.c"
        break;
      case 33: /* gop_expiration_default ::= EXPIRATION_KW time_value SEMICOLON */
      case 47: /* gog_expiration_default ::= EXPIRATION_KW time_value SEMICOLON */
#line 305 "mate_grammar.lemon"
{ mc->defaults.gop.expiration = yymsp[-1].minor.yy83;   yy_destructor(yypParser,18,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1511 "mate_grammar.c"
        break;
      case 35: /* gop_idle_timeout_default ::= IDLE_TIMEOUT_KW time_value SEMICOLON */
#line 308 "mate_grammar.lemon"
{ mc->defaults.gop.idle_timeout = yymsp[-1].minor.yy83;   yy_destructor(yypParser,19,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1518 "mate_grammar.c"
        break;
      case 37: /* gop_lifetime_default ::= LIFETIME_KW time_value SEMICOLON */
#line 311 "mate_grammar.lemon"
{ mc->defaults.gop.lifetime = yymsp[-1].minor.yy83;   yy_destructor(yypParser,20,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1525 "mate_grammar.c"
        break;
      case 39: /* gop_drop_unassigned_default ::= DROP_UNASSIGNED_KW true_false SEMICOLON */
#line 314 "mate_grammar.lemon"
{ mc->defaults.gop.drop_unassigned = yymsp[-1].minor.yy216;   yy_destructor(yypParser,16,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1532 "mate_grammar.c"
        break;
      case 41: /* gop_tree_mode_default ::= SHOW_TREE_KW gop_tree_mode SEMICOLON */
#line 317 "mate_grammar.lemon"
{ mc->defaults.gop.pdu_tree_mode = (gop_pdu_tree_t)yymsp[-1].minor.yy93;   yy_destructor(yypParser,21,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1539 "mate_grammar.c"
        break;
      case 43: /* gop_show_times_default ::= SHOW_TIMES_KW true_false SEMICOLON */
#line 320 "mate_grammar.lemon"
{ mc->defaults.gop.show_times = yymsp[-1].minor.yy216;   yy_destructor(yypParser,22,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1546 "mate_grammar.c"
        break;
      case 45: /* gog_defaults ::= GOG_KW OPEN_BRACE gog_expiration_default gop_tree_mode_default gog_goptree_default gog_show_times_default CLOSE_BRACE SEMICOLON */
#line 323 "mate_grammar.lemon"
{
  yy_destructor(yypParser,13,&yymsp[-7].minor);
  yy_destructor(yypParser,4,&yymsp[-6].minor);
  yy_destructor(yypParser,5,&yymsp[-1].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1556 "mate_grammar.c"
        break;
      case 49: /* gog_goptree_default ::= GOP_TREE_KW gop_tree_type SEMICOLON */
#line 329 "mate_grammar.lemon"
{ mc->defaults.gog.gop_tree_mode = yymsp[-1].minor.yy93;   yy_destructor(yypParser,23,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1563 "mate_grammar.c"
        break;
      case 51: /* gog_show_times_default ::= SHOW_TIMES_KW true_false SEMICOLON */
#line 332 "mate_grammar.lemon"
{ mc->defaults.gog.show_times = yymsp[-1].minor.yy216;   yy_destructor(yypParser,22,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1570 "mate_grammar.c"
        break;
      case 53: /* transform_decl ::= TRANSFORM_KW NAME transform_body SEMICOLON */
#line 339 "mate_grammar.lemon"
{
	AVPL_Transf* c;

	if ( g_hash_table_lookup(mc->transfs,yymsp[-2].minor.yy0) ) {
		configuration_error(mc,"yygotominor.yy71 transformation called '%s' exists already",yymsp[-2].minor.yy0);
	}

	for ( c = yymsp[-1].minor.yy71; c; c = c->next )
		c->name = g_strdup(yymsp[-2].minor.yy0);

	g_hash_table_insert(mc->transfs,yymsp[-1].minor.yy71->name,yymsp[-1].minor.yy71);

	yygotominor.yy71 = NULL;
  yy_destructor(yypParser,24,&yymsp[-3].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1590 "mate_grammar.c"
        break;
      case 54: /* transform_body ::= OPEN_BRACE transform_statements CLOSE_BRACE */
#line 354 "mate_grammar.lemon"
{ yygotominor.yy71 = yymsp[-1].minor.yy71;   yy_destructor(yypParser,4,&yymsp[-2].minor);
  yy_destructor(yypParser,5,&yymsp[0].minor);
}
#line 1597 "mate_grammar.c"
        break;
      case 55: /* transform_statements ::= transform_statements transform_statement */
#line 356 "mate_grammar.lemon"
{
    AVPL_Transf* c;

	for ( c = yymsp[-1].minor.yy71; c->next; c = c->next ) ;
	c->next = yymsp[0].minor.yy71;
	yygotominor.yy71 = yymsp[-1].minor.yy71;
}
#line 1608 "mate_grammar.c"
        break;
      case 56: /* transform_statements ::= transform_statement */
#line 364 "mate_grammar.lemon"
{ yygotominor.yy71 = yymsp[0].minor.yy71; }
#line 1613 "mate_grammar.c"
        break;
      case 57: /* transform_statement ::= transform_match transform_action SEMICOLON */
#line 366 "mate_grammar.lemon"
{
	yygotominor.yy71 = new_transform_elem(yymsp[-2].minor.yy16->avpl,yymsp[-1].minor.yy77->avpl,yymsp[-2].minor.yy16->match_mode,yymsp[-1].minor.yy77->replace_mode);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1621 "mate_grammar.c"
        break;
      case 58: /* transform_match ::= MATCH_KW match_mode avpl */
#line 370 "mate_grammar.lemon"
{
    yygotominor.yy16 = (transf_match_t *)g_malloc(sizeof(transf_match_t));
    yygotominor.yy16->match_mode = yymsp[-1].minor.yy172;
    yygotominor.yy16->avpl = yymsp[0].minor.yy170;
  yy_destructor(yypParser,25,&yymsp[-2].minor);
}
#line 1631 "mate_grammar.c"
        break;
      case 59: /* transform_match ::= */
#line 376 "mate_grammar.lemon"
{
    yygotominor.yy16 = (transf_match_t *)g_malloc(sizeof(transf_match_t));
    yygotominor.yy16->match_mode = AVPL_STRICT;
    yygotominor.yy16->avpl = new_avpl("");

}
#line 1641 "mate_grammar.c"
        break;
      case 60: /* transform_action ::= */
#line 383 "mate_grammar.lemon"
{
    yygotominor.yy77 = (transf_action_t *)g_malloc(sizeof(transf_action_t));
    yygotominor.yy77->replace_mode = AVPL_INSERT;
    yygotominor.yy77->avpl = new_avpl("");
}
#line 1650 "mate_grammar.c"
        break;
      case 61: /* transform_action ::= action_mode avpl */
#line 388 "mate_grammar.lemon"
{
    yygotominor.yy77 = (transf_action_t *)g_malloc(sizeof(transf_action_t));
    yygotominor.yy77->replace_mode = yymsp[-1].minor.yy215;
    yygotominor.yy77->avpl = yymsp[0].minor.yy170;
}
#line 1659 "mate_grammar.c"
        break;
      case 62: /* match_mode ::= */
#line 394 "mate_grammar.lemon"
{ yygotominor.yy172 = AVPL_STRICT; }
#line 1664 "mate_grammar.c"
        break;
      case 63: /* match_mode ::= STRICT_KW */
#line 395 "mate_grammar.lemon"
{ yygotominor.yy172 = AVPL_STRICT;   yy_destructor(yypParser,26,&yymsp[0].minor);
}
#line 1670 "mate_grammar.c"
        break;
      case 64: /* match_mode ::= EVERY_KW */
#line 396 "mate_grammar.lemon"
{ yygotominor.yy172 = AVPL_EVERY;   yy_destructor(yypParser,27,&yymsp[0].minor);
}
#line 1676 "mate_grammar.c"
        break;
      case 65: /* match_mode ::= LOOSE_KW */
#line 397 "mate_grammar.lemon"
{ yygotominor.yy172 = AVPL_LOOSE;   yy_destructor(yypParser,28,&yymsp[0].minor);
}
#line 1682 "mate_grammar.c"
        break;
      case 66: /* action_mode ::= REPLACE_KW */
#line 399 "mate_grammar.lemon"
{ yygotominor.yy215 = AVPL_REPLACE;   yy_destructor(yypParser,29,&yymsp[0].minor);
}
#line 1688 "mate_grammar.c"
        break;
      case 67: /* action_mode ::= INSERT_KW */
#line 400 "mate_grammar.lemon"
{ yygotominor.yy215 = AVPL_INSERT;   yy_destructor(yypParser,30,&yymsp[0].minor);
}
#line 1694 "mate_grammar.c"
        break;
      case 68: /* action_mode ::= */
#line 401 "mate_grammar.lemon"
{ yygotominor.yy215 = AVPL_INSERT; }
#line 1699 "mate_grammar.c"
        break;
      case 69: /* pdu_decl ::= PDU_KW NAME PROTO_KW field TRANSPORT_KW proto_stack OPEN_BRACE payload_statement extraction_statements transform_list_statement criteria_statement pdu_drop_unassigned_statement discard_pdu_data_statement last_extracted_statement CLOSE_BRACE SEMICOLON */
#line 417 "mate_grammar.lemon"
{

	mate_cfg_pdu* cfg  = new_pducfg(yymsp[-14].minor.yy0);
	extraction_t *extraction, *next_extraction;
	GPtrArray* transport_stack = g_ptr_array_new();
	int i;

	if (! cfg ) configuration_error(mc,"could not create Pdu %s.",yymsp[-14].minor.yy0);

	cfg->hfid_proto = yymsp[-12].minor.yy202->id;

	cfg->last_extracted = yymsp[-2].minor.yy216;
	cfg->discard = yymsp[-3].minor.yy216;
	cfg->drop_unassigned = yymsp[-4].minor.yy216;

	g_string_append_printf(mc->protos_filter,"||%s",yymsp[-12].minor.yy202->abbrev);

	/* flip the transport_stack */
	for (i = yymsp[-10].minor.yy231->len - 1; yymsp[-10].minor.yy231->len; i--) {
		g_ptr_array_add(transport_stack,g_ptr_array_remove_index(yymsp[-10].minor.yy231,i));
	}

	g_ptr_array_free(yymsp[-10].minor.yy231,FALSE);

	cfg->transport_ranges = transport_stack;
	cfg->payload_ranges = yymsp[-8].minor.yy231;

	if (yymsp[-5].minor.yy41) {
		cfg->criterium = yymsp[-5].minor.yy41->criterium_avpl;
		cfg->criterium_match_mode = yymsp[-5].minor.yy41->criterium_match_mode;
		cfg->criterium_accept_mode = yymsp[-5].minor.yy41->criterium_accept_mode;
	}

	cfg->transforms = yymsp[-6].minor.yy231;

	for (extraction = yymsp[-7].minor.yy259; extraction; extraction = next_extraction) {
		next_extraction = extraction->next;

		if ( ! add_hfid(extraction->hfi, extraction->as, cfg->hfids_attr) ) {
			configuration_error(mc,"MATE: failed to create extraction rule '%s'",extraction->as);
		}

		g_free(extraction);
	}
  yy_destructor(yypParser,11,&yymsp[-15].minor);
  yy_destructor(yypParser,31,&yymsp[-13].minor);
  yy_destructor(yypParser,32,&yymsp[-11].minor);
  yy_destructor(yypParser,4,&yymsp[-9].minor);
  yy_destructor(yypParser,5,&yymsp[-1].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1754 "mate_grammar.c"
        break;
      case 70: /* payload_statement ::= */
#line 463 "mate_grammar.lemon"
{ yygotominor.yy231 = NULL; }
#line 1759 "mate_grammar.c"
        break;
      case 71: /* payload_statement ::= PAYLOAD_KW proto_stack SEMICOLON */
#line 464 "mate_grammar.lemon"
{ yygotominor.yy231 = yymsp[-1].minor.yy231;   yy_destructor(yypParser,33,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1766 "mate_grammar.c"
        break;
      case 72: /* criteria_statement ::= */
#line 466 "mate_grammar.lemon"
{ yygotominor.yy41 = NULL; }
#line 1771 "mate_grammar.c"
        break;
      case 73: /* criteria_statement ::= CRITERIA_KW accept_mode match_mode avpl SEMICOLON */
#line 467 "mate_grammar.lemon"
{
	yygotominor.yy41 = (pdu_criteria_t *)g_malloc(sizeof(pdu_criteria_t));
	yygotominor.yy41->criterium_avpl = yymsp[-1].minor.yy170;
	yygotominor.yy41->criterium_match_mode = yymsp[-2].minor.yy172;
	yygotominor.yy41->criterium_accept_mode = yymsp[-3].minor.yy188;
  yy_destructor(yypParser,34,&yymsp[-4].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1783 "mate_grammar.c"
        break;
      case 74: /* accept_mode ::= */
#line 474 "mate_grammar.lemon"
{ yygotominor.yy188 = ACCEPT_MODE; }
#line 1788 "mate_grammar.c"
        break;
      case 75: /* accept_mode ::= ACCEPT_KW */
#line 475 "mate_grammar.lemon"
{ yygotominor.yy188 = ACCEPT_MODE;   yy_destructor(yypParser,35,&yymsp[0].minor);
}
#line 1794 "mate_grammar.c"
        break;
      case 76: /* accept_mode ::= REJECT_KW */
#line 476 "mate_grammar.lemon"
{ yygotominor.yy188 = REJECT_MODE;   yy_destructor(yypParser,36,&yymsp[0].minor);
}
#line 1800 "mate_grammar.c"
        break;
      case 77: /* extraction_statements ::= extraction_statements extraction_statement */
#line 478 "mate_grammar.lemon"
{ yygotominor.yy259 = yymsp[-1].minor.yy259; yygotominor.yy259->last = yygotominor.yy259->last->next = yymsp[0].minor.yy259; }
#line 1805 "mate_grammar.c"
        break;
      case 78: /* extraction_statements ::= extraction_statement */
#line 479 "mate_grammar.lemon"
{ yygotominor.yy259 = yymsp[0].minor.yy259; yygotominor.yy259->last = yygotominor.yy259; }
#line 1810 "mate_grammar.c"
        break;
      case 79: /* extraction_statement ::= EXTRACT_KW NAME FROM_KW field SEMICOLON */
#line 481 "mate_grammar.lemon"
{
	yygotominor.yy259 = (extraction_t *)g_malloc(sizeof(extraction_t));
	yygotominor.yy259->as = yymsp[-3].minor.yy0;
	yygotominor.yy259->hfi = yymsp[-1].minor.yy202;
	yygotominor.yy259->next = yygotominor.yy259->last = NULL;
  yy_destructor(yypParser,37,&yymsp[-4].minor);
  yy_destructor(yypParser,38,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1823 "mate_grammar.c"
        break;
      case 80: /* pdu_drop_unassigned_statement ::= DROP_UNASSIGNED_KW true_false SEMICOLON */
      case 90: /* gop_drop_unassigned_statement ::= DROP_UNASSIGNED_KW true_false SEMICOLON */
#line 489 "mate_grammar.lemon"
{ yygotominor.yy216 = yymsp[-1].minor.yy216;   yy_destructor(yypParser,16,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1831 "mate_grammar.c"
        break;
      case 81: /* pdu_drop_unassigned_statement ::= */
#line 490 "mate_grammar.lemon"
{ yygotominor.yy216 =  mc->defaults.pdu.drop_unassigned; }
#line 1836 "mate_grammar.c"
        break;
      case 82: /* discard_pdu_data_statement ::= DISCARD_PDU_DATA_KW true_false SEMICOLON */
#line 492 "mate_grammar.lemon"
{ yygotominor.yy216 = yymsp[-1].minor.yy216;   yy_destructor(yypParser,17,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1843 "mate_grammar.c"
        break;
      case 83: /* discard_pdu_data_statement ::= */
#line 493 "mate_grammar.lemon"
{ yygotominor.yy216 =  mc->defaults.pdu.discard; }
#line 1848 "mate_grammar.c"
        break;
      case 84: /* last_extracted_statement ::= LAST_PDU_KW true_false SEMICOLON */
#line 495 "mate_grammar.lemon"
{ yygotominor.yy216 = yymsp[-1].minor.yy216;   yy_destructor(yypParser,39,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1855 "mate_grammar.c"
        break;
      case 85: /* last_extracted_statement ::= */
#line 496 "mate_grammar.lemon"
{ yygotominor.yy216 = mc->defaults.pdu.last_extracted; }
#line 1860 "mate_grammar.c"
        break;
      case 86: /* proto_stack ::= proto_stack SLASH field */
#line 498 "mate_grammar.lemon"
{
	int* hfidp = (int *)g_malloc(sizeof(int));

	g_string_append_printf(mc->fields_filter,"||%s",yymsp[0].minor.yy202->abbrev);

	*hfidp = yymsp[0].minor.yy202->id;
	g_ptr_array_add(yymsp[-2].minor.yy231,hfidp);
	yygotominor.yy231 = yymsp[-2].minor.yy231;
  yy_destructor(yypParser,40,&yymsp[-1].minor);
}
#line 1874 "mate_grammar.c"
        break;
      case 87: /* proto_stack ::= field */
#line 508 "mate_grammar.lemon"
{
	int* hfidp = (int *)g_malloc(sizeof(int));
	*hfidp = yymsp[0].minor.yy202->id;

	g_string_append_printf(mc->fields_filter,"||%s",yymsp[0].minor.yy202->abbrev);

	yygotominor.yy231 = g_ptr_array_new();
	g_ptr_array_add(yygotominor.yy231,hfidp);
}
#line 1887 "mate_grammar.c"
        break;
      case 88: /* field ::= NAME */
#line 518 "mate_grammar.lemon"
{
	yygotominor.yy202 = proto_registrar_get_byname(yymsp[0].minor.yy0);
}
#line 1894 "mate_grammar.c"
        break;
      case 89: /* gop_decl ::= GOP_KW NAME ON_KW pdu_name MATCH_KW avpl OPEN_BRACE gop_start_statement gop_stop_statement extra_statement transform_list_statement gop_expiration_statement idle_timeout_statement lifetime_statement gop_drop_unassigned_statement show_goptree_statement show_times_statement CLOSE_BRACE SEMICOLON */
#line 536 "mate_grammar.lemon"
{
	mate_cfg_gop* cfg;

	if (g_hash_table_lookup(mc->gopcfgs,yymsp[-17].minor.yy0)) configuration_error(mc,"yygotominor.yy0 Gop Named '%s' exists already.",yymsp[-17].minor.yy0);
	if (g_hash_table_lookup(mc->gops_by_pduname,yymsp[-15].minor.yy88) ) configuration_error(mc,"Gop for Pdu '%s' exists already",yymsp[-15].minor.yy88);

	cfg = new_gopcfg(yymsp[-17].minor.yy0);
	g_hash_table_insert(mc->gops_by_pduname,yymsp[-15].minor.yy88,cfg);
	g_hash_table_insert(mc->gopcfgs,cfg->name,cfg);

	cfg->on_pdu = yymsp[-15].minor.yy88;
	cfg->key = yymsp[-13].minor.yy170;
	cfg->drop_unassigned = yymsp[-4].minor.yy216;
	cfg->show_times = yymsp[-2].minor.yy216;
	cfg->pdu_tree_mode = (gop_pdu_tree_t)yymsp[-3].minor.yy93;
	cfg->expiration = yymsp[-7].minor.yy83;
	cfg->idle_timeout = yymsp[-6].minor.yy83;
	cfg->lifetime = yymsp[-5].minor.yy83;
	cfg->start = yymsp[-11].minor.yy170;
	cfg->stop = yymsp[-10].minor.yy170;
	cfg->transforms = yymsp[-8].minor.yy231;

	merge_avpl(cfg->extra,yymsp[-9].minor.yy170,TRUE);
	delete_avpl(yymsp[-9].minor.yy170,TRUE);
      yy_destructor(yypParser,12,&yymsp[-18].minor);
  yy_destructor(yypParser,41,&yymsp[-16].minor);
  yy_destructor(yypParser,25,&yymsp[-14].minor);
  yy_destructor(yypParser,4,&yymsp[-12].minor);
  yy_destructor(yypParser,5,&yymsp[-1].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1929 "mate_grammar.c"
        break;
      case 91: /* gop_drop_unassigned_statement ::= */
#line 563 "mate_grammar.lemon"
{ yygotominor.yy216 =  mc->defaults.gop.drop_unassigned; }
#line 1934 "mate_grammar.c"
        break;
      case 92: /* gop_start_statement ::= START_KW avpl SEMICOLON */
#line 565 "mate_grammar.lemon"
{ yygotominor.yy170 = yymsp[-1].minor.yy170;   yy_destructor(yypParser,42,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1941 "mate_grammar.c"
        break;
      case 93: /* gop_start_statement ::= */
      case 95: /* gop_stop_statement ::= */
#line 566 "mate_grammar.lemon"
{ yygotominor.yy170 = NULL; }
#line 1947 "mate_grammar.c"
        break;
      case 94: /* gop_stop_statement ::= STOP_KW avpl SEMICOLON */
#line 568 "mate_grammar.lemon"
{ yygotominor.yy170 = yymsp[-1].minor.yy170;   yy_destructor(yypParser,43,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1954 "mate_grammar.c"
        break;
      case 96: /* show_goptree_statement ::= SHOW_TREE_KW gop_tree_mode SEMICOLON */
#line 571 "mate_grammar.lemon"
{ yygotominor.yy93 = yymsp[-1].minor.yy93;   yy_destructor(yypParser,21,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1961 "mate_grammar.c"
        break;
      case 97: /* show_goptree_statement ::= */
#line 572 "mate_grammar.lemon"
{ yygotominor.yy93 = (gop_tree_mode_t)mc->defaults.gop.pdu_tree_mode; }
#line 1966 "mate_grammar.c"
        break;
      case 98: /* show_times_statement ::= SHOW_TIMES_KW true_false SEMICOLON */
#line 574 "mate_grammar.lemon"
{ yygotominor.yy216 = yymsp[-1].minor.yy216;   yy_destructor(yypParser,22,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1973 "mate_grammar.c"
        break;
      case 99: /* show_times_statement ::= */
#line 575 "mate_grammar.lemon"
{ yygotominor.yy216 = mc->defaults.gop.show_times; }
#line 1978 "mate_grammar.c"
        break;
      case 100: /* gop_expiration_statement ::= EXPIRATION_KW time_value SEMICOLON */
      case 118: /* gog_expiration_statement ::= EXPIRATION_KW time_value SEMICOLON */
#line 577 "mate_grammar.lemon"
{ yygotominor.yy83 = yymsp[-1].minor.yy83;   yy_destructor(yypParser,18,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 1986 "mate_grammar.c"
        break;
      case 101: /* gop_expiration_statement ::= */
      case 103: /* idle_timeout_statement ::= */
      case 105: /* lifetime_statement ::= */
#line 578 "mate_grammar.lemon"
{ yygotominor.yy83 = mc->defaults.gop.lifetime; }
#line 1993 "mate_grammar.c"
        break;
      case 102: /* idle_timeout_statement ::= IDLE_TIMEOUT_KW time_value SEMICOLON */
#line 580 "mate_grammar.lemon"
{ yygotominor.yy83 = yymsp[-1].minor.yy83;   yy_destructor(yypParser,19,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 2000 "mate_grammar.c"
        break;
      case 104: /* lifetime_statement ::= LIFETIME_KW time_value SEMICOLON */
#line 583 "mate_grammar.lemon"
{ yygotominor.yy83 = yymsp[-1].minor.yy83;   yy_destructor(yypParser,20,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 2007 "mate_grammar.c"
        break;
      case 106: /* gop_tree_mode ::= NO_TREE_KW */
#line 586 "mate_grammar.lemon"
{ yygotominor.yy93 = (gop_tree_mode_t)GOP_NO_TREE;   yy_destructor(yypParser,44,&yymsp[0].minor);
}
#line 2013 "mate_grammar.c"
        break;
      case 107: /* gop_tree_mode ::= PDU_TREE_KW */
#line 587 "mate_grammar.lemon"
{ yygotominor.yy93 = (gop_tree_mode_t)GOP_PDU_TREE;   yy_destructor(yypParser,45,&yymsp[0].minor);
}
#line 2019 "mate_grammar.c"
        break;
      case 108: /* gop_tree_mode ::= FRAME_TREE_KW */
#line 588 "mate_grammar.lemon"
{ yygotominor.yy93 = (gop_tree_mode_t)GOP_FRAME_TREE;   yy_destructor(yypParser,46,&yymsp[0].minor);
}
#line 2025 "mate_grammar.c"
        break;
      case 109: /* gop_tree_mode ::= BASIC_TREE_KW */
#line 589 "mate_grammar.lemon"
{ yygotominor.yy93 = (gop_tree_mode_t)GOP_BASIC_PDU_TREE;   yy_destructor(yypParser,47,&yymsp[0].minor);
}
#line 2031 "mate_grammar.c"
        break;
      case 110: /* true_false ::= TRUE_KW */
#line 591 "mate_grammar.lemon"
{ yygotominor.yy216 = TRUE;   yy_destructor(yypParser,48,&yymsp[0].minor);
}
#line 2037 "mate_grammar.c"
        break;
      case 111: /* true_false ::= FALSE_KW */
#line 592 "mate_grammar.lemon"
{ yygotominor.yy216 = FALSE;   yy_destructor(yypParser,49,&yymsp[0].minor);
}
#line 2043 "mate_grammar.c"
        break;
      case 112: /* pdu_name ::= NAME */
#line 594 "mate_grammar.lemon"
{
	mate_cfg_pdu* c;
	if (( c =  (mate_cfg_pdu *)g_hash_table_lookup(mc->pducfgs,yymsp[0].minor.yy0) )) {
		yygotominor.yy88 = c->name;
	} else {
		configuration_error(mc,"No such Pdu: '%s'",yymsp[0].minor.yy0);
	}
}
#line 2055 "mate_grammar.c"
        break;
      case 113: /* time_value ::= FLOATING */
      case 114: /* time_value ::= INTEGER */
#line 604 "mate_grammar.lemon"
{
	yygotominor.yy83 = (float) strtod(yymsp[0].minor.yy0,NULL);
}
#line 2063 "mate_grammar.c"
        break;
      case 115: /* gog_decl ::= GOG_KW NAME OPEN_BRACE gog_key_statements extra_statement transform_list_statement gog_expiration_statement gog_goptree_statement show_times_statement CLOSE_BRACE SEMICOLON */
#line 622 "mate_grammar.lemon"
{
	mate_cfg_gog* cfg = NULL;

	if ( g_hash_table_lookup(mc->gogcfgs,yymsp[-9].minor.yy0) ) {
		configuration_error(mc,"Gog '%s' exists already ",yymsp[-9].minor.yy0);
	}

	cfg = new_gogcfg(yymsp[-9].minor.yy0);

	cfg->expiration = yymsp[-4].minor.yy83;
	cfg->gop_tree_mode = yymsp[-3].minor.yy93;
	cfg->transforms = yymsp[-5].minor.yy231;
	cfg->keys = yymsp[-7].minor.yy243;
	cfg->show_times = yymsp[-2].minor.yy216;

	merge_avpl(cfg->extra,yymsp[-6].minor.yy170,TRUE);
	delete_avpl(yymsp[-6].minor.yy170,TRUE);
  yy_destructor(yypParser,13,&yymsp[-10].minor);
  yy_destructor(yypParser,4,&yymsp[-8].minor);
  yy_destructor(yypParser,5,&yymsp[-1].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 2089 "mate_grammar.c"
        break;
      case 116: /* gog_goptree_statement ::= GOP_TREE_KW gop_tree_type SEMICOLON */
#line 641 "mate_grammar.lemon"
{ yygotominor.yy93 = yymsp[-1].minor.yy93;   yy_destructor(yypParser,23,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 2096 "mate_grammar.c"
        break;
      case 117: /* gog_goptree_statement ::= */
#line 642 "mate_grammar.lemon"
{ yygotominor.yy93 = mc->defaults.gog.gop_tree_mode; }
#line 2101 "mate_grammar.c"
        break;
      case 119: /* gog_expiration_statement ::= */
#line 645 "mate_grammar.lemon"
{ yygotominor.yy83 = mc->defaults.gog.expiration; }
#line 2106 "mate_grammar.c"
        break;
      case 120: /* gop_tree_type ::= NULL_TREE_KW */
#line 647 "mate_grammar.lemon"
{ yygotominor.yy93 = GOP_NULL_TREE;   yy_destructor(yypParser,51,&yymsp[0].minor);
}
#line 2112 "mate_grammar.c"
        break;
      case 121: /* gop_tree_type ::= FULL_TREE_KW */
#line 648 "mate_grammar.lemon"
{ yygotominor.yy93 = GOP_FULL_TREE;   yy_destructor(yypParser,52,&yymsp[0].minor);
}
#line 2118 "mate_grammar.c"
        break;
      case 122: /* gop_tree_type ::= BASIC_TREE_KW */
#line 649 "mate_grammar.lemon"
{ yygotominor.yy93 = GOP_BASIC_TREE;   yy_destructor(yypParser,47,&yymsp[0].minor);
}
#line 2124 "mate_grammar.c"
        break;
      case 123: /* gog_key_statements ::= gog_key_statements gog_key_statement */
#line 651 "mate_grammar.lemon"
{
	loal_append(yymsp[-1].minor.yy243,yymsp[0].minor.yy170);
	yygotominor.yy243 = yymsp[-1].minor.yy243;
}
#line 2132 "mate_grammar.c"
        break;
      case 124: /* gog_key_statements ::= gog_key_statement */
#line 656 "mate_grammar.lemon"
{
	yygotominor.yy243 = new_loal("");
	loal_append(yygotominor.yy243,yymsp[0].minor.yy170);
}
#line 2140 "mate_grammar.c"
        break;
      case 125: /* gog_key_statement ::= MEMBER_KW gop_name avpl SEMICOLON */
#line 662 "mate_grammar.lemon"
{
	rename_avpl(yymsp[-1].minor.yy170,yymsp[-2].minor.yy88);
	yygotominor.yy170 = yymsp[-1].minor.yy170;
  yy_destructor(yypParser,53,&yymsp[-3].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 2150 "mate_grammar.c"
        break;
      case 126: /* gop_name ::= NAME */
#line 667 "mate_grammar.lemon"
{
	mate_cfg_gop* c;
	if (( c = (mate_cfg_gop *)g_hash_table_lookup(mc->gopcfgs,yymsp[0].minor.yy0) )) {
		yygotominor.yy88 = c->name;
	} else {
		configuration_error(mc,"No Gop called '%s' has been already declared",yymsp[0].minor.yy0);
	}
}
#line 2162 "mate_grammar.c"
        break;
      case 127: /* extra_statement ::= EXTRA_KW avpl SEMICOLON */
#line 679 "mate_grammar.lemon"
{ yygotominor.yy170 = yymsp[-1].minor.yy170;   yy_destructor(yypParser,54,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 2169 "mate_grammar.c"
        break;
      case 128: /* extra_statement ::= */
#line 680 "mate_grammar.lemon"
{ yygotominor.yy170 = new_avpl(""); }
#line 2174 "mate_grammar.c"
        break;
      case 129: /* transform_list_statement ::= TRANSFORM_KW transform_list SEMICOLON */
#line 682 "mate_grammar.lemon"
{ yygotominor.yy231 = yymsp[-1].minor.yy231;   yy_destructor(yypParser,24,&yymsp[-2].minor);
  yy_destructor(yypParser,2,&yymsp[0].minor);
}
#line 2181 "mate_grammar.c"
        break;
      case 130: /* transform_list_statement ::= */
#line 683 "mate_grammar.lemon"
{ yygotominor.yy231 = g_ptr_array_new(); }
#line 2186 "mate_grammar.c"
        break;
      case 131: /* transform_list ::= transform_list COMMA transform */
#line 685 "mate_grammar.lemon"
{
	yygotominor.yy231 = yymsp[-2].minor.yy231;
	g_ptr_array_add(yymsp[-2].minor.yy231,yymsp[0].minor.yy71);
  yy_destructor(yypParser,55,&yymsp[-1].minor);
}
#line 2195 "mate_grammar.c"
        break;
      case 132: /* transform_list ::= transform */
#line 690 "mate_grammar.lemon"
{
	yygotominor.yy231 = g_ptr_array_new();
	g_ptr_array_add(yygotominor.yy231,yymsp[0].minor.yy71);
}
#line 2203 "mate_grammar.c"
        break;
      case 133: /* transform ::= NAME */
#line 695 "mate_grammar.lemon"
{
	AVPL_Transf* t;

	if (( t = (AVPL_Transf *)g_hash_table_lookup(mc->transfs,yymsp[0].minor.yy0) )) {
		yygotominor.yy71 = t;
	} else {
		configuration_error(mc,"There's no such Transformation: %s",yymsp[0].minor.yy0);
	}
}
#line 2216 "mate_grammar.c"
        break;
      case 134: /* avpl ::= OPEN_PARENS avps CLOSE_PARENS */
#line 705 "mate_grammar.lemon"
{ yygotominor.yy170 = yymsp[-1].minor.yy170;   yy_destructor(yypParser,56,&yymsp[-2].minor);
  yy_destructor(yypParser,57,&yymsp[0].minor);
}
#line 2223 "mate_grammar.c"
        break;
      case 135: /* avpl ::= OPEN_PARENS CLOSE_PARENS */
#line 706 "mate_grammar.lemon"
{ yygotominor.yy170 = new_avpl("");   yy_destructor(yypParser,56,&yymsp[-1].minor);
  yy_destructor(yypParser,57,&yymsp[0].minor);
}
#line 2230 "mate_grammar.c"
        break;
      case 136: /* avps ::= avps COMMA avp */
#line 708 "mate_grammar.lemon"
{ yygotominor.yy170 = yymsp[-2].minor.yy170; if ( ! insert_avp(yymsp[-2].minor.yy170,yymsp[0].minor.yy110) ) delete_avp(yymsp[0].minor.yy110);   yy_destructor(yypParser,55,&yymsp[-1].minor);
}
#line 2236 "mate_grammar.c"
        break;
      case 137: /* avps ::= avp */
#line 709 "mate_grammar.lemon"
{ yygotominor.yy170 = new_avpl(""); if ( ! insert_avp(yygotominor.yy170,yymsp[0].minor.yy110) ) delete_avp(yymsp[0].minor.yy110); }
#line 2241 "mate_grammar.c"
        break;
      case 138: /* avp ::= NAME AVP_OPERATOR value */
#line 711 "mate_grammar.lemon"
{ yygotominor.yy110 = new_avp(yymsp[-2].minor.yy0,yymsp[0].minor.yy88,*yymsp[-1].minor.yy0); }
#line 2246 "mate_grammar.c"
        break;
      case 139: /* avp ::= NAME */
#line 712 "mate_grammar.lemon"
{ yygotominor.yy110 = new_avp(yymsp[0].minor.yy0,"",'?'); }
#line 2251 "mate_grammar.c"
        break;
      case 140: /* avp ::= NAME OPEN_BRACE avp_oneoff CLOSE_BRACE */
#line 713 "mate_grammar.lemon"
{ yygotominor.yy110 = new_avp(yymsp[-3].minor.yy0,yymsp[-1].minor.yy88,'|');   yy_destructor(yypParser,4,&yymsp[-2].minor);
  yy_destructor(yypParser,5,&yymsp[0].minor);
}
#line 2258 "mate_grammar.c"
        break;
      case 141: /* avp_oneoff ::= avp_oneoff PIPE value */
#line 715 "mate_grammar.lemon"
{ yygotominor.yy88 = g_strdup_printf("%s|%s",yymsp[-2].minor.yy88,yymsp[0].minor.yy88);   yy_destructor(yypParser,59,&yymsp[-1].minor);
}
#line 2264 "mate_grammar.c"
        break;
      case 142: /* avp_oneoff ::= value */
#line 716 "mate_grammar.lemon"
{ yygotominor.yy88 = g_strdup(yymsp[0].minor.yy88); }
#line 2269 "mate_grammar.c"
        break;
      case 143: /* value ::= QUOTED */
      case 144: /* value ::= NAME */
      case 145: /* value ::= FLOATING */
      case 146: /* value ::= INTEGER */
      case 147: /* value ::= DOTED_IP */
#line 718 "mate_grammar.lemon"
{ yygotominor.yy88 = g_strdup(yymsp[0].minor.yy0); }
#line 2278 "mate_grammar.c"
        break;
      case 148: /* value ::= COLONIZED */
#line 723 "mate_grammar.lemon"
{ yygotominor.yy88 = recolonize(mc,yymsp[0].minor.yy0); }
#line 2283 "mate_grammar.c"
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact < YYNSTATE ){
#ifdef NDEBUG
    /* If we are not debugging and the reduce action popped at least
    ** one element off the stack, then we can push the new element back
    ** onto the stack here, and skip the stack overflow test in yy_shift().
    ** That gives a significant speed improvement. */
    if( yysize ){
      yypParser->yyidx++;
      yymsp -= yysize-1;
      yymsp->stateno = (YYACTIONTYPE)yyact;
      yymsp->major = (YYCODETYPE)yygoto;
      yymsp->minor = yygotominor;
    }else
#endif
    {
      yy_shift(yypParser,yyact,yygoto,&yygotominor);
    }
  }else{
    assert( yyact == YYNSTATE + YYNRULE + 1 );
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  MateParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) {
    yy_pop_parser_stack(yypParser);
  }
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
#line 190 "mate_grammar.lemon"

	configuration_error(mc,"Parse Error");
#line 2335 "mate_grammar.c"
  MateParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser _U_,       /* The parser */
  int yymajor _U_,               /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  MateParserARG_FETCH;
#define TOKEN (yyminor.yy0)
#line 186 "mate_grammar.lemon"

	configuration_error(mc,"Syntax Error before %s",yyminor);
#line 2354 "mate_grammar.c"
  MateParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  MateParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ){
    yy_pop_parser_stack(yypParser);
  }
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
  MateParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "MateParserAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void MateParser(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  MateParserTOKENTYPE yyminor       /* The value for the token */
  MateParserARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
  int yyendofinput;     /* True if we are at the end of input */
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
#if YYSTACKDEPTH<=0
    if( yypParser->yystksz <=0 ){
      /*memset(&yyminorunion, 0, sizeof(yyminorunion));*/
      yyminorunion = yyzerominor;
      yyStackOverflow(yypParser, &yyminorunion);
      return;
    }
#endif
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
  }
  yyminorunion.yy0 = yyminor;
  yyendofinput = (yymajor==0);
  MateParserARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact<YYNSTATE ){
      assert( !yyendofinput );  /* Impossible to shift the $ token */
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      yymajor = YYNOCODE;
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else{
#ifdef YYERRORSYMBOL
      int yymx;
#endif
      assert( yyact == YY_ERROR_ACTION );
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_reduce_action(
                        yypParser->yystack[yypParser->yyidx].stateno,
                        YYERRORSYMBOL)) >= YYNSTATE
          ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yypParser, (YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser, (YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}
