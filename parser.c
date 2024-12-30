#include "parser.h"
#include <ctype.h>

const char *nonTerminals[] = {
    "S", "MAIN", "FLIST", "FLISTP", "FDEF",
    "PARLIST", "PARLISTP", "VARLIST", "VARLISTP",
    "STMT", "ATRIBST", "PRINTST", "RETURNST",
    "RETURNSTP", "IFSTMT", "IFSTMTTAIL",
    "STMTLIST", "STMTLISTP",
    "EXPR", "EXPRP",
    "NUMEXPR", "NUMEXPRP",
    "TERM", "TERMP",
    "FACTOR", "FACTORP",
    "PARLISTCALL", "PARLISTCALLP"
};

const char *terminals[] = {
    "def", "int", "id", "num", "if", "else", "return", "print",
    "{", "}", "(", ")", ",", ";", ":=",
    "<", "<=", ">", ">=", "=", "<>", "==", "+", "-", "*", "/",
    "$"
};

const char *table[MAX_NONTERMINALS][MAX_TERMINALS];

int getNonTerminalIndex(const char *symbol)
{
    if (symbol == NULL)
    {
        return -1;
    }
    for (int i = 0; i < MAX_NONTERMINALS; i++)
    {
        if (strcmp(nonTerminals[i], symbol) == 0)
        {
            return i;
        }
    }
    return -1;
}

int getTerminalIndex(const char *symbol)
{
    if (symbol == NULL)
    {
        return -1;
    }
    for (int i = 0; i < MAX_TERMINALS; i++)
    {
        if (strcmp(terminals[i], symbol) == 0)
        {
            return i;
        }
    }
    return -1;
}

int tokenize_production(const char *production, char tokens[][MAX_INPUT], int max_tokens) {
    int count = 0;
    const char *current = production;
    char token[MAX_INPUT];
    int token_index = 0;

    while (*current != '\0') {
        if (isspace(*current)) {
            // Ignorar espaços em branco
            current++;
        } else if (isalnum(*current) || *current == '_') { 
            // Inicia um terminal ou não-terminal
            token_index = 0;
            while (isalnum(*current) || *current == '_') { // Aceita letras, números e '_'
                token[token_index++] = *current++;
            }
            token[token_index] = '\0'; // Finaliza o token
            if (count < max_tokens) {
                strcpy(tokens[count++], token);
            } else {
                printf("Erro: Limite de tokens excedido!\n");
                return -1;
            }
        } else {
            // Verificar operadores compostos
            if ((*current == ':' && *(current + 1) == '=') ||
                (*current == '<' && (*(current + 1) == '=' || *(current + 1) == '>')) ||
                (*current == '>' && *(current + 1) == '=') ||
                (*current == '=' && *(current + 1) == '=')) {
                token_index = 0;
                token[token_index++] = *current++;
                token[token_index++] = *current++;
                token[token_index] = '\0';
            } else {
                // Trata caracteres únicos como tokens
                token_index = 0;
                token[token_index++] = *current++;
                token[token_index] = '\0';
            }

            if (count < max_tokens) {
                strcpy(tokens[count++], token);
            } else {
                printf("Erro: Limite de tokens excedido!\n");
                return -1;
            }
        }
    }

    return count; // Retorna o número de tokens encontrados
}

void initialize_table()
{
    for (int i = 0; i < MAX_NONTERMINALS; i++)
    {
        for (int j = 0; j < MAX_TERMINALS; j++)
        {
            table[i][j] = NULL;
        }
    }

    int r, c; // row (não-terminal), col (terminal)

    // S ::= MAIN
    r = getNonTerminalIndex("S");
    {
        const char *s_first[] = {"def", "int", "id", "print", "return", "if", "{", ";"};
        int nf = sizeof(s_first) / sizeof(s_first[0]);
        for (int i = 0; i < nf; i++)
        {
            c = getTerminalIndex(s_first[i]);
            if (c >= 0)
                table[r][c] = "MAIN";
        }
        // MAIN pode derivar ε, adicionar FOLLOW(S) = {$}
        c = getTerminalIndex("$");
        if (c >= 0)
            table[r][c] = "MAIN";
    }

    // MAIN ::= STMT | FLIST | ''
    r = getNonTerminalIndex("MAIN");
    {
        // MAIN ::= STMT
        const char *main_stmt_first[] = {"int", "id", "print", "return", "if", "{", ";"};
        int nm = sizeof(main_stmt_first) / sizeof(main_stmt_first[0]);
        for (int i = 0; i < nm; i++)
        {
            c = getTerminalIndex(main_stmt_first[i]);
            if (c >= 0)
                table[r][c] = "STMT";
        }

        // MAIN ::= FLIST
        const char *main_flist_first[] = {"def"};
        int nf = sizeof(main_flist_first) / sizeof(main_flist_first[0]);
        for (int i = 0; i < nf; i++)
        {
            c = getTerminalIndex(main_flist_first[i]);
            if (c >= 0)
                table[r][c] = "FLIST";
        }

        // MAIN ::= ''
        // FOLLOW(MAIN) = FOLLOW(S) = {$}
        c = getTerminalIndex("$");
        if (c >= 0 && table[r][c] == NULL)
            table[r][c] = "";
    }

    // FLIST ::= FDEF FLISTP
    r = getNonTerminalIndex("FLIST");
    c = getTerminalIndex("def");
    if (c >= 0)
        table[r][c] = "FDEF FLISTP";

    // FLISTP ::= FDEF FLISTP | ''
    r = getNonTerminalIndex("FLISTP");
    {
        // FLISTP ::= FDEF FLISTP
        const char *flistp_def_first[] = {"def"};
        int nf = sizeof(flistp_def_first) / sizeof(flistp_def_first[0]);
        for (int i = 0; i < nf; i++)
        {
            c = getTerminalIndex(flistp_def_first[i]);
            if (c >= 0)
                table[r][c] = "FDEF FLISTP";
        }

        // FLISTP ::= ''
        // FOLLOW(FLISTP) = FOLLOW(FLIST) = {$}
        c = getTerminalIndex("$");
        if (c >= 0 && table[r][c] == NULL)
            table[r][c] = "";
    }

    // FDEF ::= def id ( PARLIST ) { STMTLIST }
    r = getNonTerminalIndex("FDEF");
    c = getTerminalIndex("def");
    if (c >= 0)
        table[r][c] = "def id ( PARLIST ) { STMTLIST }";

    // PARLIST ::= int id PARLIST' | ''
    r = getNonTerminalIndex("PARLIST");
    {
        // PARLIST ::= int id PARLIST'
        const char *parlist_int_first[] = {"int"};
        int ni = sizeof(parlist_int_first) / sizeof(parlist_int_first[0]);
        for (int i = 0; i < ni; i++)
        {
            c = getTerminalIndex(parlist_int_first[i]);
            if (c >= 0)
                table[r][c] = "int id PARLISTP";
        }

        // PARLIST ::= ''
        // FOLLOW(PARLIST) = {")"}
        c = getTerminalIndex(")");
        if (c >= 0 && table[r][c] == NULL)
            table[r][c] = "";
    }

    // PARLISTP ::= , int id PARLIST' | ''
    r = getNonTerminalIndex("PARLISTP");
    {
        // PARLISTP ::= , int id PARLIST'
        const char *parlistp_comma_first[] = {","};
        int nc = sizeof(parlistp_comma_first) / sizeof(parlistp_comma_first[0]);
        for (int i = 0; i < nc; i++)
        {
            c = getTerminalIndex(parlistp_comma_first[i]);
            if (c >= 0)
                table[r][c] = ", int id PARLISTP";
        }

        // PARLISTP ::= ''
        // FOLLOW(PARLISTP) = {")"}
        c = getTerminalIndex(")");
        if (c >= 0 && table[r][c] == NULL)
            table[r][c] = "";
    }

    // VARLIST ::= id VARLIST'
    r = getNonTerminalIndex("VARLIST");
    c = getTerminalIndex("id");
    if (c >= 0)
        table[r][c] = "id VARLISTP";

    // VARLISTP ::= , id VARLIST' | ''
    r = getNonTerminalIndex("VARLISTP");
    {
        // VARLISTP ::= , id VARLIST'
        const char *varlistp_comma_first[] = {","};
        int nc = sizeof(varlistp_comma_first) / sizeof(varlistp_comma_first[0]);
        for (int i = 0; i < nc; i++)
        {
            c = getTerminalIndex(varlistp_comma_first[i]);
            if (c >= 0)
                table[r][c] = ", id VARLISTP";
        }

        // VARLISTP ::= ''
        // FOLLOW(VARLISTP) = {";", "}"}
        const char *varlistp_follow[] = {";", "}"};
        int nvf = sizeof(varlistp_follow) / sizeof(varlistp_follow[0]);
        for (int i = 0; i < nvf; i++)
        {
            c = getTerminalIndex(varlistp_follow[i]);
            if (c >= 0 && table[r][c] == NULL)
                table[r][c] = "";
        }
    }

    // STMT ::= int VARLIST ; | ATRIBST ; | PRINTST ; | RETURNST ; | IFSTMT | { STMTLIST } | ;
    r = getNonTerminalIndex("STMT");
    {
        // STMT ::= int VARLIST ;
        c = getTerminalIndex("int");
        if (c >= 0)
            table[r][c] = "int VARLIST ;";

        // STMT ::= ATRIBST ;
        c = getTerminalIndex("id");
        if (c >= 0)
            table[r][c] = "ATRIBST ;";

        // STMT ::= PRINTST ;
        c = getTerminalIndex("print");
        if (c >= 0)
            table[r][c] = "PRINTST ;";

        // STMT ::= RETURNST ;
        c = getTerminalIndex("return");
        if (c >= 0)
            table[r][c] = "RETURNST ;";

        // STMT ::= IFSTMT
        c = getTerminalIndex("if");
        if (c >= 0)
            table[r][c] = "IFSTMT";

        // STMT ::= { STMTLIST }
        c = getTerminalIndex("{");
        if (c >= 0)
            table[r][c] = "{ STMTLIST }";

        // STMT ::= ;
        c = getTerminalIndex(";");
        if (c >= 0)
            table[r][c] = ";";
    }

    // ATRIBST ::= id := EXPR
    r = getNonTerminalIndex("ATRIBST");
    c = getTerminalIndex("id");
    if (c >= 0)
        table[r][c] = "id := EXPR";

    // PRINTST ::= print EXPR
    r = getNonTerminalIndex("PRINTST");
    c = getTerminalIndex("print");
    if (c >= 0)
        table[r][c] = "print EXPR";

    // RETURNST ::= return RETURNST'
    r = getNonTerminalIndex("RETURNST");
    c = getTerminalIndex("return");
    if (c >= 0)
    {
        // RETURNST ::= return RETURNST'
        table[r][c] = "return RETURNSTP";

        // RETURNST ::= return
        // FOLLOW(RETURNST) = {";"}
        c = getTerminalIndex(";");
        if (c >= 0 && table[r][c] == NULL)
            table[r][c] = "return";
    }

    // RETURNST' ::= id | ''
    r = getNonTerminalIndex("RETURNSTP");
    {
        // RETURNST' ::= id
        c = getTerminalIndex("id");
        if (c >= 0)
            table[r][c] = "id";

        // RETURNST' ::= ''
        // FOLLOW(RETURNST') = {";"}
        c = getTerminalIndex(";");
        if (c >= 0 && table[r][c] == NULL)
            table[r][c] = "";
    }

    // IFSTMT ::= if ( EXPR ) STMT IFSTMTTAIL
    r = getNonTerminalIndex("IFSTMT");
    c = getTerminalIndex("if");
    if (c >= 0)
        table[r][c] = "if ( EXPR ) STMT IFSTMTTAIL";

    // IFSTMTTAIL ::= else STMT | ''
    r = getNonTerminalIndex("IFSTMTTAIL");
    {
        // IFSTMTTAIL ::= else STMT
        c = getTerminalIndex("else");
        if (c >= 0)
            table[r][c] = "else STMT";

        // IFSTMTTAIL ::= ''
        // FOLLOW(IFSTMTTAIL) = {";", "}"}
        const char *ifstmttail_follow[] = {";", "}"};
        int nft = sizeof(ifstmttail_follow) / sizeof(ifstmttail_follow[0]);
        for (int i = 0; i < nft; i++)
        {
            c = getTerminalIndex(ifstmttail_follow[i]);
            if (c >= 0 && table[r][c] == NULL)
                table[r][c] = "";
        }
    }

    // STMTLIST ::= STMT STMTLIST'
    r = getNonTerminalIndex("STMTLIST");
    {
        const char *stmtlist_first[] = {"int", "id", "print", "return", "if", "{", ";"};
        int n_stmt = sizeof(stmtlist_first) / sizeof(stmtlist_first[0]);
        for (int i = 0; i < n_stmt; i++)
        {
            c = getTerminalIndex(stmtlist_first[i]);
            if (c >= 0)
                table[r][c] = "STMT STMTLISTP";
        }
    }

    // STMTLISTP ::= STMT STMTLIST' | ''
    r = getNonTerminalIndex("STMTLISTP");
    {
        // STMTLISTP ::= STMT STMTLISTP
        const char *stmtlistp_first[] = {"int", "id", "print", "return", "if", "{", ";"};
        int n_stmtp = sizeof(stmtlistp_first) / sizeof(stmtlistp_first[0]);
        for (int i = 0; i < n_stmtp; i++)
        {
            c = getTerminalIndex(stmtlistp_first[i]);
            if (c >= 0)
                table[r][c] = "STMT STMTLISTP";
        }

        // STMTLISTP ::= ''
        // FOLLOW(STMTLISTP) = {"}"}
        c = getTerminalIndex("}");
        if (c >= 0 && table[r][c] == NULL)
            table[r][c] = "";
    }

    // EXPR ::= NUMEXPR EXPR'
    r = getNonTerminalIndex("EXPR");
    {
        const char *expr_first[] = {"id", "num", "("};
        int ne = sizeof(expr_first) / sizeof(expr_first[0]);
        for (int i = 0; i < ne; i++)
        {
            c = getTerminalIndex(expr_first[i]);
            if (c >= 0)
                table[r][c] = "NUMEXPR EXPRP";
        }
    }

    // EXPR' ::= < NUMEXPR | <= NUMEXPR | > NUMEXPR | >= NUMEXPR | == NUMEXPR | <> NUMEXPR | ''
    r = getNonTerminalIndex("EXPRP");
    {
        const char *exprp_ops[] = {"<", "<=", ">", ">=", "==", "<>"};
        const char *exprp_prods[] = {
            "< NUMEXPR",
            "<= NUMEXPR",
            "> NUMEXPR",
            ">= NUMEXPR",
            "== NUMEXPR",
            "<> NUMEXPR"
        };
        int n_exprp_ops = sizeof(exprp_ops) / sizeof(exprp_ops[0]);
        for (int i = 0; i < n_exprp_ops; i++)
        {
            c = getTerminalIndex(exprp_ops[i]);
            if (c >= 0)
                table[r][c] = exprp_prods[i];
        }

        // EXPR' ::= ''
        // FOLLOW(EXPR') = {";", ")", "}", ","}
        const char *exprp_follow[] = {";", ")", "}", ","};
        int n_exprp_follow = sizeof(exprp_follow) / sizeof(exprp_follow[0]);
        for (int i = 0; i < n_exprp_follow; i++)
        {
            c = getTerminalIndex(exprp_follow[i]);
            if (c >= 0 && table[r][c] == NULL)
                table[r][c] = "";
        }
    }

    // NUMEXPR ::= TERM NUMEXPR'
    r = getNonTerminalIndex("NUMEXPR");
    {
        const char *numexpr_first[] = {"id", "num", "("};
        int nn = sizeof(numexpr_first) / sizeof(numexpr_first[0]);
        for (int i = 0; i < nn; i++)
        {
            c = getTerminalIndex(numexpr_first[i]);
            if (c >= 0)
                table[r][c] = "TERM NUMEXPRP";
        }
    }

    // NUMEXPR' ::= + TERM NUMEXPR' | - TERM NUMEXPR' | ''
    r = getNonTerminalIndex("NUMEXPRP");
    {
        // NUMEXPR' ::= + TERM NUMEXPR'
        c = getTerminalIndex("+");
        if (c >= 0)
            table[r][c] = "+ TERM NUMEXPRP";

        // NUMEXPR' ::= - TERM NUMEXPR'
        c = getTerminalIndex("-");
        if (c >= 0)
            table[r][c] = "- TERM NUMEXPRP";

        // NUMEXPR' ::= ''
        // FOLLOW(NUMEXPRP) = {"<", "<=", ">", ">=", "==", "<>", ";", ")", "}", ",", "$"}
        const char *numexprp_follow[] = {"<", "<=", ">", ">=", "==", "<>", ";", ")", "}", ",", "$"};
        int n_numexprp_follow = sizeof(numexprp_follow) / sizeof(numexprp_follow[0]);
        for (int i = 0; i < n_numexprp_follow; i++)
        {
            c = getTerminalIndex(numexprp_follow[i]);
            if (c >= 0 && table[r][c] == NULL)
                table[r][c] = "";
        }
    }

    // TERM ::= FACTOR TERMP
    r = getNonTerminalIndex("TERM");
    {
        const char *term_first[] = {"id", "num", "("};
        int nt = sizeof(term_first) / sizeof(term_first[0]);
        for (int i = 0; i < nt; i++)
        {
            c = getTerminalIndex(term_first[i]);
            if (c >= 0)
                table[r][c] = "FACTOR TERMP";
        }
    }

    // TERMP ::= * FACTOR TERMP | / FACTOR TERMP | ''
    r = getNonTerminalIndex("TERMP");
    {
        // TERMP ::= * FACTOR TERMP
        c = getTerminalIndex("*");
        if (c >= 0)
            table[r][c] = "* FACTOR TERMP";

        // TERMP ::= / FACTOR TERMP
        c = getTerminalIndex("/");
        if (c >= 0)
            table[r][c] = "/ FACTOR TERMP";

        // TERMP ::= ''
        // FOLLOW(TERMP) = {"+", "-", "<", "<=", ">", ">=", "==", "<>", ";", ")", "}", ",", "$"}
        const char *termp_follow[] = {"+", "-", "<", "<=", ">", ">=", "==", "<>", ";", ")", "}", ",", "$"};
        int n_termp_follow = sizeof(termp_follow) / sizeof(termp_follow[0]);
        for (int i = 0; i < n_termp_follow; i++)
        {
            c = getTerminalIndex(termp_follow[i]);
            if (c >= 0 && table[r][c] == NULL)
                table[r][c] = "";
        }
    }

    // FACTOR ::= num | ( NUMEXPR ) | id FACTOR'
    r = getNonTerminalIndex("FACTOR");
    {
        // FACTOR ::= num
        c = getTerminalIndex("num");
        if (c >= 0)
            table[r][c] = "num";

        // FACTOR ::= ( NUMEXPR )
        c = getTerminalIndex("(");
        if (c >= 0)
            table[r][c] = "( NUMEXPR )";

        // FACTOR ::= id FACTOR'
        c = getTerminalIndex("id");
        if (c >= 0)
            table[r][c] = "id FACTORP";
    }

    // FACTOR' ::= '' | ( PARLISTCALL )
    r = getNonTerminalIndex("FACTORP");
    {
        // FACTOR' ::= ( PARLISTCALL )
        c = getTerminalIndex("(");
        if (c >= 0)
            table[r][c] = "( PARLISTCALL )";

        // FACTOR' ::= ''
        // FOLLOW(FACTOR') = {"+", "-", "*", "/", "<", "<=", ">", ">=", "==", "<>", ";", ")", "}", ","}
        const char *factorp_follow[] = {"+", "-", "*", "/", "<", "<=", ">", ">=", "==", "<>", ";", ")", "}", ","};
        int n_factorp_follow = sizeof(factorp_follow) / sizeof(factorp_follow[0]);
        for (int i = 0; i < n_factorp_follow; i++)
        {
            c = getTerminalIndex(factorp_follow[i]);
            if (c >= 0 && table[r][c] == NULL)
                table[r][c] = "";
        }
    }

    // PARLISTCALL ::= id PARLISTCALLP | ''
    r = getNonTerminalIndex("PARLISTCALL");
    {
        // PARLISTCALL ::= id PARLISTCALLP
        const char *parlistcall_id_first[] = {"id"};
        int ni = sizeof(parlistcall_id_first) / sizeof(parlistcall_id_first[0]);
        for (int i = 0; i < ni; i++)
        {
            c = getTerminalIndex(parlistcall_id_first[i]);
            if (c >= 0)
                table[r][c] = "id PARLISTCALLP";
        }

        // PARLISTCALL ::= ''
        // FOLLOW(PARLISTCALL) = {")"}
        c = getTerminalIndex(")");
        if (c >= 0 && table[r][c] == NULL)
            table[r][c] = "";
    }

    // PARLISTCALLP ::= , id PARLISTCALLP | ''
    r = getNonTerminalIndex("PARLISTCALLP");
    {
        // PARLISTCALLP ::= , id PARLISTCALLP
        const char *parlistcallp_comma_first[] = {","};
        int nc = sizeof(parlistcallp_comma_first) / sizeof(parlistcallp_comma_first[0]);
        for (int i = 0; i < nc; i++)
        {
            c = getTerminalIndex(parlistcallp_comma_first[i]);
            if (c >= 0)
                table[r][c] = ", id PARLISTCALLP";
        }

        // PARLISTCALLP ::= ''
        // FOLLOW(PARLISTCALLP) = {")"}
        c = getTerminalIndex(")");
        if (c >= 0 && table[r][c] == NULL)
            table[r][c] = "";
    }
}