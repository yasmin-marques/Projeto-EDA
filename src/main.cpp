#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

// Inclusão das estruturas de dados e componentes necessários
#include "../ED's/avl_tree.hpp"
#include "../ED's/rb_tree.hpp"
#include "../ED's/chained_hash.hpp"
#include "../ED's/open_hash.hpp"
#include "../ED's/compare.hpp"
#include "../ED's/dictionary.hpp"
#include "processing.hpp"
using namespace std;

// Função auxiliar que remove caracteres especiais somente de uma palavra,
// e converte para letras minúsculas, usada somente no modo iterativo
// Trata também caracteres UTF-8 corretamente.
std::string clean_word(const std::string& input) {
    std::string result;
    for (size_t i = 0; i < input.size(); ) {
        unsigned char c = input[i];

        if (c < 128) {
            if (std::isalnum(c) || c == '-') {
                result += std::tolower(c);
            } else {
                result += ' ';
            }
            ++i;
        } else if ((c & 0xE0) == 0xC0 && i + 1 < input.size()) {
            result += input[i];
            result += input[i + 1];
            i += 2;
        } else if ((c & 0xF0) == 0xE0 && i + 2 < input.size()) {
            result += input[i];
            result += input[i + 1];
            result += input[i + 2];
            i += 3;
        } else if ((c & 0xF8) == 0xF0 && i + 3 < input.size()) {
            result += input[i];
            result += input[i + 1];
            result += input[i + 2];
            result += input[i + 3];
            i += 4;
        } else {
            result += ' ';
            ++i;
        }
    }
    return result;
}


// Mostra instruções de uso no terminal
void help(const char* program_name) {
    cerr << "  USO:\n";
    cerr << "    " << program_name << " <modo_estrutura> <arquivo>\n\n";
    cerr << "  Onde:\n";
    cerr << "  <arquivo>            Nome do arquivo de entrada (na pasta 'in')\n";
    cerr << "  <modo_estrutura>     Estrutura de dados a ser usada:\n";
    cerr << "                       - dictionary_avl\n";
    cerr << "                       - dictionary_rb\n";
    cerr << "                       - dictionary_chained_hash\n";
    cerr << "                       - dictionary_open_hash\n\n";
    cerr << "  Exemplo:\n";
    cerr << "    " << program_name << " dictionary_avl texto.txt\n";
    cerr << " \n";
    cerr << "  Modo interativo disponível:\n";
    cerr << "    interativo_avl | interativo_rb | interativo_chained_hash | interativo_open_hash\n";
    cerr << "    Exemplo: " << program_name << " interativo_avl modo\n";
    }

template<typename Dictionary>
void modo_interativo(Dictionary& dict) {
    std::string command;
    std::cout << "\n[ INTERACTIVE MODE ]\n";
    std::cout << "Available commands:\n";
    std::cout << "  create       -> reset dictionary\n";
    std::cout << "  insert       -> insert key-value pair\n";
    std::cout << "  update       -> update value of an existing key\n";
    std::cout << "  remove       -> delete a key\n";
    std::cout << "  get          -> access value by key\n";
    std::cout << "  contains     -> check if key exists\n";
    std::cout << "  iterate      -> list all key-value pairs\n";
    std::cout << "  size         -> show total number of elements\n";
    std::cout << "  clear        -> clear dictionary\n";
    std::cout << "  exit         -> leave interactive mode\n";

    // Loop principal para receber comandos
    while (true) {
        std::cout << "\n> ";
        std::getline(std::cin, command);

        if (command == "exit") break;

        // Limpa ou reinicia o dicionário
        else if (command == "create" || command == "clear") {
            dict.clear();
            std::cout << "[OK] Dictionary cleared.\n";
        }

        // Insere um novo par chave-valor
        else if (command == "insert") {
            std::string key;
            int value;
            std::cout << "Enter key: ";
            std::getline(std::cin, key);
            key = clean_word(key);          // limpa caracteres especiais
            std::cout << "Enter value: ";
            std::cin >> value;
            std::cin.ignore();
            dict.insert(key, value);
            std::cout << "[OK] Pair inserted.\n";
        }

        // Atualiza valor de uma chave existente
        else if (command == "update") {
            std::string key;
            int value;
            std::cout << "Enter key: ";
            std::getline(std::cin, key);
            std::cout << "Enter new value: ";
            std::cin >> value;
            std::cin.ignore();
            if(dict.contains(key)) {
                dict.at(key, value);
                std::cout << "[OK] Value updated.\n";
            } else {
                std::cout << "[ERROR] Key not found.\n";
            }
        }

        // Remove uma chave do dicionário
        else if (command == "remove") {
            std::string key;
            std::cout << "Enter key to remove: ";
            std::getline(std::cin, key);
            if(dict.contains(key)) {
                dict.remove(key);
                std::cout << "[OK] Key removed.\n";
            } else {
                std::cout << "[ERROR] Key not found.\n";
            }
        }

        // Acessa o valor de uma chave
        else if (command == "get") {
            std::string key;
            std::cout << "Enter key: ";
            std::getline(std::cin, key);
            if (dict.contains(key)) {
                std::cout << "Value: " << dict.search(key) << "\n";
            } else {
                std::cout << "[ERROR] Key not found.\n";
            }
        }

        // Verifica se uma chave está presente
        else if (command == "contains") {
            std::string key;
            std::cout << "Enter key: ";
            std::getline(std::cin, key);
            std::cout << (dict.contains(key) ? "[YES] Exists.\n" : "[NO] Does not exist.\n");
        }

        // Lista todos os elementos (ordenados)
        else if (command == "iterate") {
            auto elements = dict.items();
            std::sort(elements.begin(), elements.end());
            if (elements.empty()) {
                std::cout << "[INFO] Dictionary is empty.\n";
            } else {
                std::cout << "Items:\n";
                for (const auto& pair : elements) {
                    std::cout << "  " << pair.first << ": " << pair.second << "\n";
                }
            }
        }

        // Exibe o tamanho atual
        else if (command == "size") {
            std::cout << "Size: " << dict.size() << "\n";
        }

        // Comando não reconhecido
        else {
            std::cout << "[ERROR] Unknown command.\n";
        }
    }

    // Mensagem final ao sair
    std::cout << "\n[INFO] Exiting interactive mode.\n";
}


int main(int argc, char* argv[]) {
    // Verifica se o número de argumentos está correto e mostra ajuda se não
    if(argc != 3) {
        help(argv[0]);
        return 1;
    }

    // Salva a estrutura de dados e o nome do arquivo
    string ED = argv[1];
    string filename = argv[2];

    // Seleciona a estrutura de dados conforme argumento passado
    if(ED == "dictionary_avl") {
        // Usa árvore AVL
        dictionary<avl_tree<string, int, Compare>> dict;
        process_and_save_dict_tree(dict, ED, filename); // Processa e salva resultado
    } 
    
    else if(ED == "dictionary_rb") {
        // Usa árvore Rubro-Negra
        dictionary<red_black_tree<string, int, Compare>> dict;
        process_and_save_dict_tree(dict, ED, filename); // Processa e salva resultado
    } 
    
    else if(ED == "dictionary_chained_hash") {
        // Usa tabela hash com encadeamento
        dictionary<chained_hash_table<string, int, hash<string>, Compare>> dict;
        process_and_save_dict_hash(dict, ED, filename); // Processa e salva resultado
    } 
    
    else if(ED == "dictionary_open_hash") {
        // Usa tabela hash com endereçamento aberto
        dictionary<open_hash_table<string, int, hash<string>, Compare>> dict;
        process_and_save_dict_hash(dict, ED, filename); // Processa e salva resultado
    } 

    else if(ED == "interativo_avl") {
        dictionary<avl_tree<string, int, Compare>> dict;
        modo_interativo(dict);
    }
    else if(ED == "interativo_rb") {
        dictionary<red_black_tree<string, int, Compare>> dict;
        modo_interativo(dict);
    }
    else if(ED == "interativo_chained_hash") {
        dictionary<chained_hash_table<string, int, hash<string>, Compare>> dict;
        modo_interativo(dict);
    }
    else if(ED == "interativo_open_hash") {
        dictionary<open_hash_table<string, int, hash<string>, Compare>> dict;
        modo_interativo(dict);
    }
    
    // Se o modo informado não for reconhecido, mostra ajuda e encerra
    else {
        cerr << "Error: estrutura de dados invalida" << endl;
        help(argv[0]);
        return 1;
    }

    return 0;
}