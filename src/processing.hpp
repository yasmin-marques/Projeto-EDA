#ifndef PROCESSING_HPP
#define PROCESSING_HPP

#include <iostream>
#include <chrono>
#include <string>

// Declaração da função que lê um arquivo texto e retorna seu conteúdo como string
std::string read_file(const std::string& file_path);

// Declaração da função que grava uma string em um arquivo texto
void save_to_file(const std::string& file_path, const std::string& raw_text);

// Template que processa um texto usando uma estrutura de árvore,
// mede o tempo de execução, e salva o resultado e métricas em arquivo
template <typename dict_type>
void process_and_save_dict_tree(dict_type& dict, const std::string& ED, const std::string& filename);

// Template que processa um texto usando uma estrutura de hash table,
// mede o tempo de execução, e salva o resultado e métricas em arquivo
template <typename dict_type>
void process_and_save_dict_hash(dict_type& dict, const std::string& ED, const std::string& filename);

#endif 