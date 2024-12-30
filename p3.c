#include "parser.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

// Pilha para o parsing
char *stack[MAX_STACK];
int top = -1;

void log_pilha() {
    printf("PILHA ATUAL: ");
    for (int i = top; i >= 0; i--) {
        printf("%s ", stack[i]);
    }
    printf("\n");
}

//Adiciona um símbolo ao topo da pilha.
void push(const char *symbol) {
    if (top < MAX_STACK - 1) {
        stack[++top] = strdup(symbol);
        if (stack[top] == NULL) {
            printf("Erro: Falha ao alocar memória!\n");
            exit(1);
        }
    } else {
        printf("Erro: Pilha cheia!\n");
        exit(1);
    }
}

//Remove e retorna o símbolo do topo da pilha.
const char *pop() {
    if (top >= 0) {
        char *popped = stack[top--];
        return popped;
    } else {
        printf("Erro: Pilha vazia!\n");
        exit(1);
    }
}

//Retorna o símbolo no topo da pilha sem removê-lo.
const char *peek()
{
    if (top >= 0)
    {
        return stack[top];
    }
    else
    {
        return NULL;
    }
}

void parse(const char *inputLine) {
    char buffer[MAX_INPUT];
    strcpy(buffer, inputLine);

    char *inputTokens[300];
    int inputCount = 0;
    char *tk = strtok(buffer, " ");
    while (tk != NULL) {
        inputTokens[inputCount++] = tk;
        tk = strtok(NULL, " ");
    }

    int inputIndex = 0;
    push("$");
    push("S");

    printf("Iniciando parsing...\n\n");

    while (1) {
        const char *top_symbol = peek();
        const char *current_input = (inputIndex < inputCount) ? inputTokens[inputIndex] : NULL;

        if (top_symbol == NULL) {
            printf("Erro: Pilha vazia antes do fim da entrada!\n");
            return;
        }

        if (strcmp(top_symbol, "$") == 0) {
            if (current_input != NULL && strcmp(current_input, "$") == 0) {
                pop();
                printf("Entrada aceita!\n");
            } else {
                printf("Erro: Entrada não terminou em $!\n");
            }
            return;
        }

        int terIndex = getTerminalIndex(top_symbol);
        if (terIndex != -1) {
            // Símbolo do topo é terminal
            if (current_input != NULL && strcmp(top_symbol, current_input) == 0) {
                printf("Match: %s\n", top_symbol);
                pop();
                log_pilha();
                inputIndex++;
            } else {
                printf("Erro sintático: Esperava '%s', obteve '%s'\n", top_symbol, current_input ? current_input : "EOF");
                return;
            }
        } else {
            // Símbolo do topo é não-terminal
            int row = getNonTerminalIndex(top_symbol);
            int col = (current_input != NULL) ? getTerminalIndex(current_input) : -1;

            if (row == -1 || col == -1 || table[row][col] == NULL) {
                printf("Erro sintático: Não há produção para <%s> com lookahead '%s'\n", top_symbol, current_input ? current_input : "EOF");
                return;
            }

            const char *production = table[row][col];
            printf("Produção usada: %s -> %s\n", top_symbol, (strlen(production) > 0) ? production : "ε");
            //printf("DEBUG: Token atual '%s' determinou a produção: %s -> %s\n",
            //current_input ? current_input : "EOF", top_symbol, 
            //production && strlen(production) > 0 ? production : "ε");
            pop();

            if (strlen(production) > 0) {
                // Limpa espaços no final da produção
                char temp[MAX_INPUT];
                strcpy(temp, production);
                int end = (int)strlen(temp) - 1;
                while (end >= 0 && isspace((unsigned char)temp[end])) {
                    temp[end--] = '\0';
                }

                char tokens[50][MAX_INPUT];
                int count = tokenize_production(temp, tokens, 50);
                if (count == -1) {
                    printf("Erro ao tokenizar produção!\n");
                    return;
                }

                // Depuração: Imprime os tokens obtidos
                for (int i = 0; i < count; i++) {
                    // Verifica se o token é válido (opcional)
                    if (getNonTerminalIndex(tokens[i]) == -1 && getTerminalIndex(tokens[i]) == -1) {
                        printf("Erro: Token inválido '%s' encontrado na produção\n", tokens[i]);
                        return;
                    }
                }

                for (int i = count - 1; i >= 0; i--) {
                    if (strcmp(tokens[i], "ε") != 0) {
                        push(tokens[i]);
                    }
                }
                log_pilha();
            } else {
                // Produção vazia (ε)
                continue;
            }
        }
        printf("INPUT: ");
        for (int i = inputIndex; i < inputCount; i++) {
            printf("%s%s", inputTokens[i], (i < inputCount - 1) ? " " : "");
        }
        printf("\n");
    }
}

// Verifica se uma string é uma palavra reservada
bool is_reserved_word(const char *word)
{
    const char *reserved[] = {"int", "if", "else", "def", "print", "return"};
    int num_reserved = sizeof(reserved) / sizeof(reserved[0]);
    for (int i = 0; i < num_reserved; i++)
    {
        if (strcmp(word, reserved[i]) == 0)
        {
            return true;
        }
    }
    return false;
}

// Substitui identificadores por 'id', números por 'num', e separa os tokens
void tokenize_input(char *input)
{
    char buffer[MAX_INPUT];
    int j = 0;

    for (int i = 0; input[i] != '\0'; i++)
    {
        if (isalpha(input[i]))
        { // Detecta identificadores ou palavras reservadas
            char temp[50];
            int k = 0;

            // Captura a palavra completa
            while (isalnum(input[i]) && k < 49)
            {
                temp[k++] = input[i++];
            }
            temp[k] = '\0'; // Finaliza a string temporária
            i--;            // Reposiciona o índice

            if (is_reserved_word(temp))
            {
                // É uma palavra reservada
                if (j > 0 && !isspace(buffer[j - 1]))
                {
                    buffer[j++] = ' ';
                }
                strcpy(&buffer[j], temp);
                j += strlen(temp);
                buffer[j++] = ' ';
            }
            else
            {
                // É um identificador genérico
                if (j > 0 && !isspace(buffer[j - 1]))
                {
                    buffer[j++] = ' ';
                }
                buffer[j++] = 'i';
                buffer[j++] = 'd';
                buffer[j++] = ' ';
            }
        }
        else if (isdigit(input[i]))
        { // Detecta números
            if (j > 0 && !isspace(buffer[j - 1]))
            {
                buffer[j++] = ' ';
            }
            while (isdigit(input[i]))
            { // Consome todo o número
                i++;
            }
            buffer[j++] = 'n';
            buffer[j++] = 'u';
            buffer[j++] = 'm';
            buffer[j++] = ' ';
            i--; // Reposiciona o índice
        }
        else if (ispunct(input[i]))
        { // Detecta operadores ou delimitadores
            if (j > 0 && !isspace(buffer[j - 1]))
            {
                buffer[j++] = ' ';
            }

            // Trata ':=' como token único
            if (input[i] == ':' && input[i + 1] == '=')
            {
                buffer[j++] = ':';
                buffer[j++] = '=';
                i++;
            }
            // Trata operadores relacionais
            else if ((input[i] == '<' || input[i] == '>') && input[i + 1] == '=')
            {
                buffer[j++] = input[i];
                buffer[j++] = '=';
                i++;
            }
            else if ((input[i] == '<' && input[i + 1] == '>'))
            { // <>
                buffer[j++] = '<';
                buffer[j++] = '>';
                i++;
            }
            else
            {
                buffer[j++] = input[i];
            }

            buffer[j++] = ' ';
        }
        else if (!isspace(input[i]))
        { // Outros caracteres não-espaço
            buffer[j++] = input[i];
        }
    }

    // Remove espaços extras
    int final_len = strlen(buffer);
    while (final_len > 0 && isspace((unsigned char)buffer[final_len - 1]))
    {
        buffer[--final_len] = '\0';
    }

    strcpy(input, buffer);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Uso: %s <caminho_para_arquivo>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file)
    {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    char input[MAX_INPUT] = ""; // String onde será montada a entrada completa
    char line[MAX_INPUT];       // Para ler cada linha individualmente
    size_t current_length = 0;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        // Remove quebra de linha
        line[strcspn(line, "\n")] = '\0'; // Remove '\n'
        line[strcspn(line, "\r")] = '\0'; // Remove '\r'

        // Ignorar linhas vazias
        if (strlen(line) == 0)
        {
            continue;
        }

        // Checar se o novo conteúdo cabe no buffer
        size_t line_length = strlen(line);
        if (current_length + line_length + 1 >= MAX_INPUT)
        {
            printf("Erro: O arquivo de entrada é muito grande. Aumente MAX_INPUT.\n");
            fclose(file);
            return 1;
        }

        // Concatenar a linha no buffer de entrada
        strcat(input, line);
        strcat(input, " "); // Adiciona espaço para separar as linhas (se necessário)
        current_length += line_length + 1;
    }

    fclose(file);

    // Remove espaços extras no final da string concatenada
    size_t len = strlen(input);
    while (len > 0 && isspace((unsigned char)input[len - 1]))
    {
        input[--len] = '\0';
    }

    // Verifica se a entrada termina com '$'
    if (len == 0 || input[len - 1] != '$')
    {
        printf("Erro: A entrada deve terminar com '$'. Última parte encontrada: '%s'\n", input);
        return 1;
    }

    printf("Entrada completa: %s\n", input);

    initialize_table();

    tokenize_input(input);
    parse(input);

    return 0;
}