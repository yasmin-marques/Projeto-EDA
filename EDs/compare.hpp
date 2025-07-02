#ifndef COMPARE_HPP
#define COMPARE_HPP

#include <algorithm> // std::transform
#include <cctype>    // std::tolower
#include <string>

// Estrutura de comparação de strings sem diferenciar maiúsculas de minúsculas
struct Compare {
    // Operador de comparação usado para ordenar strings
    // Compara duas strings após normalizá-las para minúsculas
    bool operator()(const std::string& left_side, const std::string& right_side) const {
        return normalize(left_side) < normalize(right_side);
    }

    // Função auxiliar para converter uma string inteira para minúsculas
    static std::string normalize(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
            return std::tolower(c); // converte cada caractere para minúsculo
        });
        return result;
    }
};

#endif