#ifndef OPEN_HASH_HPP
#define OPEN_HASH_HPP

#include <cmath>        // sqrt
#include <utility>      // std::pair
#include <algorithm>    // std::sort
#include <functional>   // std::hash
#include <stdexcept>
#include <string>
#include <vector>

// Classe genérica de uma tabela hash com endereçamento aberto
template <typename Key, typename Data, typename Hash = std::hash<Key>, typename Compare = std::less<Key>>
class open_hash_table {
private:
// Estados possíveis de uma célula na tabela
    enum entry_state { EMPTY, ACTIVE, DELETED };

    // Estrutura que representa uma célula da tabela
    struct entry {
        Key key{};                  // chave armazenada
        Data data{};                // dado armazenado
        entry_state state{EMPTY};   // estado da célula
    };


    std::vector<entry> m_table;    // vetor com todas as células
    size_t m_number_of_elements;   // total de elementos na tabela
    size_t m_table_size;           // tamanho atual da tabela
    float m_load_factor;           // fator de carga atual
    float m_max_load_factor;       // fator de carga máximo permitido antes de rehash
    Hash m_hashing;                // functor para função de hash
    Compare m_compare;              // functor de comparação

    std::vector<std::pair<Key, Data>> m_sorted_pairs;   // vetor ordenado (para iterador)
    bool m_needs_update;                                // marca se precisa reordenar os pares
    mutable std::size_t num_comparisons;                // número de comparação realizadas
    mutable std::size_t num_collisions;                 // número de colisões realizadas

    // Retorna o próximo número primo maior que x
    size_t get_next_prime(size_t x) {
        if(x <= 2) return 3;
        x = (x % 2 == 0) ? x + 1 : x;
        while (true) {
            bool is_prime = true;
            for (size_t i = 3; i <= sqrt(x); i += 2) {
                num_comparisons++;
                if (x % i == 0) {
                    is_prime = false;
                    break;
                }
            }
            if (is_prime) return x;
            x += 2;
        }
    }

    // Calcula o índice de uma chave com sondagem dupla
    size_t hash_code(const Key& key, size_t i) const {
        size_t h1 = m_hashing(key);
        size_t h2 = 1 + (h1 % (m_table_size - 1));
        return (h1 + i * h2) % m_table_size;
    }

    // Atualiza vetor com os pares ordenados
    void update_sorted_pairs() {
        m_sorted_pairs.clear();
        m_sorted_pairs.reserve(m_number_of_elements);       // reserva espaço
        for (const auto& e : m_table) {
            num_comparisons++;                                
            if (e.state == ACTIVE) {
                m_sorted_pairs.emplace_back(e.key, e.data);   // adiciona chave ativa
            }
        }
        std::sort(m_sorted_pairs.begin(), m_sorted_pairs.end(),
                  [this](const auto& a, const auto& b) {
                      num_comparisons++;
                      return m_compare(a.first, b.first);    // ordena por chave
                  });
        m_needs_update = false; // atualização feita
    }

public:
    // Construtor
    open_hash_table(size_t table_size = 19, const Hash& hf = Hash())
        : m_number_of_elements(0),
          m_table_size(get_next_prime(table_size)),
          m_table(m_table_size),
          m_load_factor(0),
          m_max_load_factor(0.5),
          m_hashing(hf),
          m_compare(Compare()),
          m_needs_update(true),
          num_comparisons(0),
          num_collisions(0) {}

    // Destrutor
    ~open_hash_table() { clear(); }

    // Retorna número de slots
    size_t bucket_count() const { return m_table_size; }

    // Retorna número total de elementos
    size_t size() const { return m_number_of_elements; }

    // Retorna true se estiver vazia
    bool empty() const { return m_number_of_elements == 0; }

    // Retorna o fator de carga máximo
    float max_load_factor() const { return m_max_load_factor; }

    // Retorna número de colisões registradas
    unsigned int collisions() const { return num_collisions; }

    // Retorna número de comparações feitas
    unsigned int comparisons() const { return num_comparisons; }

    // Retorna o fator de carga atual
    float load_factor() const { 
        return static_cast<float>(m_number_of_elements) / m_table_size; 
    }

    // Remove todos os dados da tabela
    void clear() {
        m_table.clear();
        m_table.resize(m_table_size);
        m_number_of_elements = 0;
        m_needs_update = true;
    }

    // Redimensiona a tabela e redistribui os elementos
    void rehash(size_t new_size) {
        num_comparisons++;
        if (new_size <= m_table_size) return;
        new_size = get_next_prime(new_size);

        std::vector<entry> new_table(new_size);
        for (const auto& e : m_table) {
            num_comparisons++;
            if (e.state == ACTIVE) {
                size_t j = 0;
                size_t index;
                do {
                    index = (m_hashing(e.key) + j * (1 + (m_hashing(e.key) % (new_size - 1)))) % new_size;
                    j++;
                    num_comparisons++;
                } while (new_table[index].state == ACTIVE);

                new_table[index] = {e.key, e.data, ACTIVE};
            }
        }
        m_table = std::move(new_table);
        m_table_size = new_size;
        m_needs_update = true;
    }

    // Insere um novo elemento com chave e dado
    bool insert(const Key& key, const Data& data) {
        num_comparisons++;
        if (load_factor() >= m_max_load_factor) {
            rehash(2 * m_table_size);
        }

        size_t first_deleted = m_table_size; // marca slot reutilizável

        for (size_t i = 0; i < m_table_size; ++i) {
            size_t slot = hash_code(key, i);

            if (m_table[slot].state == ACTIVE) {
                num_comparisons++;
                if (m_table[slot].key == key) return false; // já existe
                num_collisions++;   // houve colisão
            } else if (m_table[slot].state == DELETED) {
                num_comparisons++;
                if (first_deleted == m_table_size) first_deleted = slot;
            } else if (m_table[slot].state == EMPTY) {
                num_comparisons++;
                size_t target = (first_deleted != m_table_size) ? first_deleted : slot;
                m_table[target] = {key, data, ACTIVE};
                m_number_of_elements++;
                m_needs_update = true;
                return true;
            }
        }

        if (first_deleted != m_table_size) {
            num_comparisons++;
            m_table[first_deleted] = {key, data, ACTIVE};
            m_number_of_elements++;
            m_needs_update = true;
            return true;
        }

        return false;
    }

    // Remove um par pela chave
    bool remove(const Key& key) {
        for (size_t i = 0; i < m_table_size; ++i) {
            size_t slot = hash_code(key, i);
            num_comparisons++;
            if (m_table[slot].state == EMPTY) return false;
            num_comparisons++;
            if (m_table[slot].state == ACTIVE && m_table[slot].key == key) {
                m_table[slot].state = DELETED;
                m_number_of_elements--;
                m_needs_update = true;
                return true;
            }
        }
        return false;
    }

    // Busca e retorna referência para o dado associado a chave
    Data& search(const Key& key) {
        for (size_t i = 0; i < m_table_size; ++i) {
            size_t slot = hash_code(key, i);
            num_comparisons++;
            if (m_table[slot].state == EMPTY) break;
            num_comparisons++;
            if (m_table[slot].state == ACTIVE && m_table[slot].key == key)
                return m_table[slot].data;
        }
        throw std::out_of_range("Key not found");
    }

    // Retorna true se a chave estiver presente
    bool contains(const Key& key) const {
        for (size_t i = 0; i < m_table_size; ++i) {
            size_t slot = hash_code(key, i);
            num_comparisons++;
            if (m_table[slot].state == EMPTY) return false;
            num_comparisons++;
            if (m_table[slot].state == ACTIVE && m_table[slot].key == key)
                return true;
        }
        return false;
    }

    // Atualiza o valor da chave (se existir)
    void at(const Key& key, const Data& value) {
        for (size_t i = 0; i < m_table_size; ++i) {
            size_t slot = hash_code(key, i);
            num_comparisons++;
            if (m_table[slot].state == ACTIVE && m_table[slot].key == key) {
                m_table[slot].data = value;
                return;
            }
        }
        throw std::out_of_range("Key not found");
    }

    // Retorna o nome da estrutura (usado no relatório/saída)
    std::string name() const { return "Open Hash Table"; }

    // Classe iterator para percorrer a hash table em ordem
    class iterator {
    private:
        const open_hash_table* m_ht;    // ponteiro para a hash table
        size_t m_index;                 // índice atual no vetor ordenado

    public:
        // Construtor
        iterator(const open_hash_table* ht, size_t index)
            : m_ht(ht), m_index(index) {}

        // Retorna referência para o par (chave, dado) atual
        const std::pair<Key, Data>& operator*() const {
            return m_ht->m_sorted_pairs[m_index];
        }

        // Permite acesso direto aos membros do par atual
        const std::pair<Key, Data>* operator->() const {
            return &m_ht->m_sorted_pairs[m_index];
        }

        // Avança o iterador para o próximo elemento
        iterator& operator++() {
            m_index++;
            return *this;
        }

        // Compara dois iteradores
        bool operator!=(const iterator& it) const {
            return m_index != it.m_index;
        }
    };

    // Retorna o início da iteração (posição 0 no vetor ordenado)
    // Se o vetor ainda não foi atualizado, atualiza antes
    iterator begin() {
        if(m_needs_update) update_sorted_pairs();
        return iterator(this, 0);
    }

    // Retorna o fim da iteração (última posição)
    // Também atualiza se necessário
    iterator end() {
        if(m_needs_update) update_sorted_pairs();
        return iterator(this, m_sorted_pairs.size());
    }
};

#endif