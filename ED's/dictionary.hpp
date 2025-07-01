#ifndef DICTIONARY_HPP
#define DICTIONARY_HPP

#include <algorithm>    // std::transform   
#include <fstream>      // leitura e escrita de arquivos   
#include <cctype>       // std::tolower
#include <chrono>       // medir tempo de execução
#include <string>
#include <sstream>

// Funções auxiliares para leitura e gravação
#include "../src/processing.hpp"   
// Função para normalizar palavras
#include "compare.hpp"

// Classe genérica de dicionário baseado em alguma estrutura de dados
template <typename type>
class dictionary {
private:
    type m_dict;    // instância da ED usada internamente

public:
    // Construtor padrão
    dictionary() {}

    // Insere uma palavra no dicionário, ou incrementa a frequencia se ela já existir
    void insert(const std::string& word) {
        std::string norm = Compare::normalize(word);
        try {
            auto& value = m_dict.search(norm);  // tenta encontrar a palavra já existente
            value += 1;                         // incrementa a contagem se encontrada

        } catch(std::out_of_range&) {   
            m_dict.insert(norm, 1);             // se não, insere com frequencia 1
        }
    }

    // Recebe um texto e insere palavra por palavra no dicionário
    void insert_text(const std::string& text) {
        std::istringstream iss(text);           // cria uma stream a partir do texto
        std::string word;
        while(iss >> word) {                    // extrai palavra por palavra
            insert(word);                       // insere cada palavra no dicionário
        }
    }

    // Remove uma palavra dicionário
    void remove(const std::string& word) {
        m_dict.remove(Compare::normalize(word));
    }

    // Verifica se uma palavra está no dicionário
    bool contains(const std::string& word) {
        return m_dict.contains(Compare::normalize(word));
    }

    // Atualiza a frequencia de uma palavra
    void at(const std::string& word, int at) {
        m_dict.at(Compare::normalize(word), at);
    }

    // Retorna a frequencia de uma palavra 
    int search(const std::string& word) {
        return m_dict.search(Compare::normalize(word));
    }

    // Calcula o comprimento de uma string UTF-8
    // Utilizado na formatação de saída
    size_t utf8_length(const std::string& s) {
        size_t len = 0;
        for (size_t i = 0; i < s.size(); ) {
            unsigned char c = s[i];
            if ((c & 0x80) == 0) i += 1;
            else if ((c & 0xE0) == 0xC0) i += 2;
            else if ((c & 0xF0) == 0xE0) i += 3;
            else if ((c & 0xF8) == 0xF0) i += 4;
            else i += 1; // caractere inválido, pula 1 byte
            ++len;
        }
        return len;
    }

    // Limpa todos os dados do dicionário
    void clear() { m_dict.clear(); }

    // Retorna o número de palavras no dicionário
    unsigned int size() { return m_dict.size(); }

    // Verifica se o dicionário está vazio
    bool empty() { return m_dict.empty(); }

    // Retorna o número de comparações feitas na estrutura
    unsigned int comparisons() { return m_dict.comparisons(); }

    // Salva os dados e métricas em um arquivo (usado para árvores)
    void save_for_tree(const std::string& filename, std::chrono::milliseconds duration) {
        std::string result;
        result += "Dicionário usando " + m_dict.name() + "\n";
        result += "---------------------------------------------\n";
        result += "Tempo para montar a tabela: " + std::to_string(duration.count()) + " milissegundos\n";
        result += "Tamanho do dicionário:      " + std::to_string(this->size()) + "\n";
        result += "Número de comparações:      " + std::to_string(m_dict.comparisons()) + '\n';
        result += "Número de rotações:         " + std::to_string(m_dict.rotations()) + '\n';
        result += "---------------------------------------------\n";
        result += "Palavra                  | Frequência\n";
        result += "---------------------------------------------\n";

        // percorre todos os pares (chave, dado) da estrutura e formata para salvar
        for (const auto& e : m_dict) {
            result += e.first;
            size_t display_len = utf8_length(e.first);
            result += std::string(25 - std::min(display_len, size_t(25)), ' ');
            result += "| ";
            result += std::to_string(e.second);
            result += '\n';
        }
        save_to_file(filename, result); // grava resultado no arquivo
    }

    // Salva os dados e métricas em um arquivo (usado para hashes)
    void save_for_hash(const std::string& filename, std::chrono::milliseconds duration) {
        std::string result;
        result += "Dicionário usando " + m_dict.name() + "\n";
        result += "---------------------------------------------\n";
        result += "Tempo para montar a tabela: " + std::to_string(duration.count()) + " milissegundos\n";
        result += "Tamanho do dicionário:      " + std::to_string(this->size()) + "\n";
        result += "Número de comparações:      " + std::to_string(m_dict.comparisons()) + '\n';
        result += "Número de colisões:         " + std::to_string(m_dict.collisions()) + '\n';
        result += "Média de acesso:            " + std::to_string(m_dict.average_access_length()) + '\n';
        result += "Acesso máximo:              " + std::to_string(m_dict.max_access_length()) + '\n';
        result += "---------------------------------------------\n";
        result += "Palavra                  | Frequência\n";
        result += "---------------------------------------------\n";

        // percorre todos os pares (chave, dado) da estrutura e formata para salvar
        for (const auto& e : m_dict) {
            result += e.first;
            result += std::string(25 - std::min<size_t>(e.first.size(), 25), ' ');
            result += "| ";
            result += std::to_string(e.second);
            result += '\n';
        }
        save_to_file(filename, result); // grava resultado no arquivo
    }
};

#endif