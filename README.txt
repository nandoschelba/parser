Programa de Análise Sintática LL(1)

Este programa implementa um analisador sintático baseado em uma tabela LL(1) para verificar se uma entrada fornecida está de acordo com a gramática especificada. 
Ele utiliza um arquivo contendo o código-fonte em uma linguagem específica e realiza a validação conforme a gramática implementada.

Compilação: 
    gcc p3.c parser.c -o p3
    ./p3 nome-do-arquivo
    
    Ex: ./p3 input-aceito-1.txt
        ./p3 input-aceito-2.txt
        ./p3 input-negado-1.txt
        ./p3 input-negado-2.txt