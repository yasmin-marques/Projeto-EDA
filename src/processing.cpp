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

#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>

// Lê o conteúdo de um arquivo de texto, converte para minúsculas e
// remove pontuação (exceto hífen entre letras ou números), mantendo caracteres UTF-8 válidos
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

    std::string result;                     // string onde será montado o texto processado

    // percorre todos os caracteres do conteúdo original
    for (size_t i = 0; i < raw_text.size(); ) {
        unsigned char c = raw_text[i];

        // se o caractere for comum (sem acento, ASCII padrão)
        if (c < 128) {
            // se for letra, número, mantém o caractere minúsculo
            if (std::isalnum(c)) {
                result += std::tolower(c);  // converte para minúsculo
            }
            // mantém o hífen apenas se estiver entre dois caracteres alfanuméricos
            else if (c == '-' && i > 0 && i + 1 < raw_text.size() &&
                     std::isalnum(raw_text[i - 1]) && std::isalnum(raw_text[i + 1])) {
                result += '-';
            } else {
                result += ' ';              // substitui símbolos por espaço
            }
            ++i;
        } 
        // se for uma letra com acento ou outro símbolo especial (UTF-8)
        else {
            // UTF-8 de 2 bytes (ex: ç, é)
            if ((c & 0xE0) == 0xC0 && i + 1 < raw_text.size()) {
                unsigned char c2 = raw_text[i + 1];
                // apenas aceita se for letra acentuada comum (evita frações e símbolos)
                if ((unsigned char)c == 0xC3 && (c2 >= 0x80 && c2 <= 0xBF)) {
                    // converte letras acentuadas maiúsculas para minúsculas
                    if (c2 >= 0x80 && c2 <= 0x9E && c2 != 0x97) {
                        c2 += 0x20; // diferença entre maiúscula/minúscula no UTF-8
                    }
                    result += c;
                    result += c2;
                }
                else {
                    result += ' ';
                }
                i += 2;
            } 
            // UTF-8 de 3 bytes ou mais
            else if (((c & 0xF0) == 0xE0 && i + 2 < raw_text.size()) ||
                     ((c & 0xF8) == 0xF0 && i + 3 < raw_text.size())) {
                result += ' ';  // ignora emojis, frações, aspas especiais etc.
                if ((c & 0xF0) == 0xE0) i += 3;
                else i += 4;
            } 
            // se for um byte inválido ou incompleto
            else {
                result += ' ';
                ++i;
            }
        }
    }

    return result;  // retorna o texto limpo e processado
}

// Salva o resultado formatado da contagem de palavras em um arquivo de saída
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