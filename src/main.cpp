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
}

int main(int argc, char* argv[]) {
    // Verifica se o número de argumentos está correto e mostra ajuda se não
    if (argc != 3) {
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
    
    // Se o modo informado não for reconhecido, mostra ajuda e encerra
    else {
        cerr << "Error: estrutura de dados invalida" << endl;
        help(argv[0]);
        return 1;
    }

    return 0;
}