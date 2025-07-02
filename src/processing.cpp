// Este módulo contém funções para leitura e pré-processamento de arquivos de texto,
// bem como templates para processar palavras utilizando diferentes estruturas de dados
// (árvores e tabelas hash), medindo desempenho e salvando os resultados.

#include "processing.hpp"

// Inclusão das estruturas de dados e componentes necessários
#include "../EDs/dictionary.hpp"
#include "../EDs/avl_tree.hpp"
#include "../EDs/rb_tree.hpp"
#include "../EDs/chained_hash.hpp"
#include "../EDs/open_hash.hpp"
#include "../EDs/compare.hpp"

#include <cctype>
#include <chrono>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std;
using namespace std::chrono;

// Lê o conteúdo de um arquivo de texto e retorna uma string com o texto processado
std::string read_file(const std::string& filename) {
    std::ifstream input_file(filename);
    // verifica se o arquivo foi aberto corretamente
    if (!input_file.is_open()) {
        std::cerr << "Erro ao abrir arquivo: " << filename << std::endl;
        exit(1);                            // encerra se não conseguir abrir
    }

    std::ostringstream buffer;
    buffer << input_file.rdbuf();           // lê o arquivo inteiro 
    std::string raw_text = buffer.str();    // guarda o texto lido

    std::string result;                     // string onde sera montado o texto processado
    // percorre todos os caracteres do conteúdo original
    for (size_t i = 0; i < raw_text.size(); ) {
        unsigned char c = raw_text[i];

        // se o caractere for comum (sem acento)
        if (c < 128) {
            // se for letra, número ou hífen, mantém o caractere
            if (std::isalnum(c) || c == '-') {
                result += std::tolower(c);  // converte para minúsculo
            } else {
                result += ' ';              // substitui símbolos por espaço
            }
            ++i;
        } 
        // se for uma letra com acento ou outro símbolo especial
        else {
            // UTF-8 de 2 bytes (ex: ç)
            if ((c & 0xE0) == 0xC0 && i + 1 < raw_text.size()) {
                result += raw_text[i];
                result += raw_text[i + 1];
                i += 2;
            } 
            // UTF-8 de 3 bytes (ex: á, é, í)
            else if ((c & 0xF0) == 0xE0 && i + 2 < raw_text.size()) {
                result += raw_text[i];
                result += raw_text[i + 1];
                result += raw_text[i + 2];
                i += 3;
            } 
            // UTF-8 de 4 bytes (ex: emojis, caracteres de línguas asiáticas)
            else if ((c & 0xF8) == 0xF0 && i + 3 < raw_text.size()) {
                result += raw_text[i];
                result += raw_text[i + 1];
                result += raw_text[i + 2];
                result += raw_text[i + 3];
                i += 4;
            } else {
                // se for um byte invalido ou incompleto, ignora com espaço
                result += ' ';
                ++i;
            }
        }
    }

    return result;  // retorna o texto limpo e processado
}

// Salva uma string em um arquivo texto
void save_to_file(const std::string& filename, const std::string& raw_text) {
    std::ofstream output_file(filename);    // abre arquivo para escrita
    if(!output_file.is_open()) {
        std::cerr << "Erro ao salvar: " << filename << std::endl;
        exit(1);                            // encerra se não conseguir salvar
    }

    output_file << raw_text;                // escreve conteúdo no arquivo
    output_file.close();                    // fecha arquivo
}

// Template para processar e salvar resultados usando estruturas de árvore
template <typename dict_type>
void process_and_save_dict_tree(dict_type& dict, const std::string& ED, const std::string& filename) {
    std::string file = read_file("in/" + filename);             // lê e processa o texto
    auto start = std::chrono::high_resolution_clock::now();
    dict.insert_text(file);                                     // insere todas as palavras no dicionário
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    
    std::cout << "Resultado salvo em: out/" << ED << "_" << filename << "\n";

    std::string out_filename = "out/" + ED + "_" + filename;    // define nome do arquivo de saída
    dict.save_for_tree(out_filename, duration);                 // salva resultado formatado
}

// Template para processar e salvar resultados usando tabelas hash
template <typename dict_type>
void process_and_save_dict_hash(dict_type& dict, const std::string& ED, const std::string& filename) {
    std::string file = read_file("in/" + filename);             // lê e processa o texto
    auto start = std::chrono::high_resolution_clock::now();
    dict.insert_text(file);                                     // insere todas as palavras no dicionário
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    std::cout << "Resultado salvo em: out/" << ED << "_" << filename << "\n";
    
    std::string out_filename = "out/" + ED + "_" + filename;    // define nome do arquivo de saída
    dict.save_for_hash(out_filename, duration);                 // salva resultado formatado
}

// Instanciações explícitas dos templates acima para que o linker saiba como gerar os binários
template void process_and_save_dict_tree<dictionary<avl_tree<std::string, int, Compare>>>(
    dictionary<avl_tree<std::string, int, Compare>>&, const std::string&, const std::string&);

template void process_and_save_dict_tree<dictionary<red_black_tree<std::string, int, Compare>>>(
    dictionary<red_black_tree<std::string, int, Compare>>&, const std::string&, const std::string&);

template void process_and_save_dict_hash<dictionary<chained_hash_table<std::string, int, std::hash<std::string>, Compare>>>(
    dictionary<chained_hash_table<std::string, int, std::hash<std::string>, Compare>>&, const std::string&, const std::string&);

template void process_and_save_dict_hash<dictionary<open_hash_table<std::string, int, std::hash<std::string>, Compare>>>(
    dictionary<open_hash_table<std::string, int, std::hash<std::string>, Compare>>&, const std::string&, const std::string&);