# Dicionário de Frequência de Palavras

Trabalho da disciplina de Estrutura de Dados Avançada (QXD0115) do curso de Ciência da Computação da UFC - Campus Quixadá, ministrada pelo professor Atílio Gomes Luiz no semestre 2025.1.

Desenvolvido por Yasmin de Lima Marques.

## Descrição do Projeto

Este projeto consiste no desenvolvimento de uma aplicação em C++ para calcular a frequência de palavras em um texto, ordená-las alfabeticamente e realizar comparações de desempenho entre diferentes estruturas de dados usadas para armazenar essas palavras. O objetivo é analisar as métricas de desempenho dessas estruturas. Para mais detalhes, consulte o relatório disponível na pasta docs.

## Estruturas de Dados Implementadas

O projeto compara diferentes estruturas de dados:

- Árvore AVL
- Árvore Rubro-Negra
- Tabela Hash com Encadeamento Exterior
- Tabela Hash com Endereçamento Aberto

## Como compilar e executar o projeto

É necessário ter o C++ (versão 17 ou superior) instalado.
Acesse a pasta src e compile o projeto com o seguinte comando:

```
g++ -std=c++17 main.cpp processing.cpp -o freq
```

Após isso, será gerado um executável chamado `freq`. Para executar o projeto:

```
./freq <estrutura_de_dado> <arquivo_de_entrada.txt>
```

Onde:

- `<estrutura_de_dado>` é um dos valores: dictionary_avl, dictionary_rb, dictionary_chained_hash ou dictionary_open_hash.
- `<arquivo_de_entrada.txt>` é o nome do arquivo de texto, que deve estar na pasta src/in.

Exemplo:

```
./freq dictionary_avl kjv-bible.txt
```

A saída será um arquivo chamado `dictionary_avl_kjv-bible.txt` na pasta src/out.

## Modo Interativo (para testes isolados)

Também é possível testar as estruturas individualmente em modo interativo. Para isso, use:

```
./freq <parametro> modo
```

Parâmetros disponíveis:

- interativo_avl
- interativo_rb
- interativo_chained_hash
- interativo_open_hash

Exemplo:

```
./freq interativo_rb modo
```

## Relatórios e Documentação

A documentação contendo as métricas e detalhes de implementação se encontra na pasta docs.
