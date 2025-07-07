#ifndef CHAINED_HASH_HPP
#define CHAINED_HASH_HPP

#include <cmath>        // sqrt
#include <utility>      // std::pair
#include <algorithm>    // std::sort
#include <functional>   // std::hash
#include <stdexcept>
#include <string>
#include <vector>
#include <list>

// Classe genérica de uma tabela hash com encadeamento
template <typename Key, typename Data, typename Hash = std::hash<Key>, typename Compare = std::less<Key>>
class chained_hash_table {
private:
    size_t m_number_of_elements;   // total de elementos na tabela
    size_t m_table_size;           // tamanho atual da tabela
    std::vector<std::list<std::pair<Key, Data>>>* m_table; // ponteiro para o vetor de listas de pares
    float m_load_factor;           // fator de carga atual
    float m_max_load_factor;       // fator de carga máximo permitido antes de rehash
    Hash m_hashing;                // functor para função de hash
    Compare m_compare;             // objeto comparador de chaves

    std::vector<std::pair<Key, Data>> m_sorted_pairs;   // vetor ordenado (para iterador)
    bool m_needs_update;                                // marca se precisa reordenar os pares
    mutable size_t num_comparisons;                       // número de comparações realizadas
    mutable size_t num_collisions;                        // número de colisões realizadas

    // Retorna o próximo número primo maior que x
    size_t get_next_prime(size_t x) {
        if(x <= 2) return 3;
        x = (x % 2 == 0) ? x + 1 : x;
        bool not_prime = true;
        while(not_prime) {
            not_prime = false;
            for(int i = 3; i <= sqrt(x); i+=2) {
                if(x % i == 0) {
                    not_prime = true;
                    break;
                }
            }
            x += 2;
        }
        return x - 2;
    }

    // Calcula o índice de uma chave
    size_t hash_code(const Key& k) const { 
        return m_hashing(k) % m_table_size; 
    }

    // Atualiza o vetor auxiliar com os pares ativos, ordenados por chave
    void update_sorted_pairs() {
        m_sorted_pairs.clear();
        m_sorted_pairs.reserve(m_number_of_elements);       // reserva espaço
        for(size_t i = 0; i < m_table_size; i++) {
            for(const auto& p : (*m_table)[i]) {
                m_sorted_pairs.push_back(p);                // adiciona chave
            }
        }
        std::sort(m_sorted_pairs.begin(), m_sorted_pairs.end(),
                  [this](const auto& a, const auto& b) {
                      return m_compare(a.first, b.first);   // ordena por chave
                  });
        m_needs_update = false;
    }

public:
    // Construtor
    chained_hash_table(size_t table_size = 19, const Hash& hf = Hash())
    : m_number_of_elements(0),
      m_table_size(get_next_prime(table_size)),
      m_table(new std::vector<std::list<std::pair<Key, Data>>>(m_table_size)),
      m_load_factor(0),
      m_max_load_factor(0.5),
      m_hashing(hf),
      m_compare(Compare()),
      m_needs_update(true),
      num_comparisons(0),
      num_collisions(0) {}

    // Destrutor
    ~chained_hash_table() {
        clear();
        delete m_table;
    }

    // Retorna número de slots
    size_t bucket_count() const { return m_table_size; }

    // Retorna número total de elementos
    size_t size() const { return m_number_of_elements; }

    // Retorna true se estiver vazia    
    bool empty() const { return m_number_of_elements == 0; }    
    
    // Retorna o fator de carga máximo
    float max_load_factor() const { return m_max_load_factor; }

    // Retorna número de comparações feitas
    unsigned int comparisons() const { return num_comparisons; }

    // Retorna número de colisões registradas
    unsigned int collisions() const { return num_collisions; }

    // Retorna o fator de carga atual
    float load_factor() const { 
        return static_cast<float>(m_number_of_elements) / m_table_size; 
    }

    // Remove todos os dados da tabela
    void clear() {
        for(auto& bucket : *m_table) bucket.clear();
        m_number_of_elements = 0;
        m_needs_update = true;
    }

    // Insere um novo elemento com chave e dado
    bool insert(const Key& k, const Data& d) {
        // verifica se a tabela precisa ser redimensionada com base no fator de carga
        if (load_factor() >= m_max_load_factor) {
            rehash(2 * m_table_size);
        }

        size_t slot = hash_code(k);
        std::list<std::pair<Key, Data> >& bucket = (*m_table)[slot];

        // verifica se a chave já existe na lista (bucket)
        for(typename std::list<std::pair<Key, Data> >::iterator it = bucket.begin(); it != bucket.end(); ++it) {
            num_comparisons++;
            if (it->first == k) return false;
        }

        // se o bucket já possui elementos, então houve colisão
        if(!bucket.empty()) {
            num_collisions++;
        }

        // insere o novo par no bucket
        bucket.push_back(std::make_pair(k, d));
        m_number_of_elements++;
        m_needs_update = true;

        return true;
    }

    // Remove um par pela chave
    bool remove(const Key& k) {
        size_t slot = hash_code(k);
        for(auto it = (*m_table)[slot].begin(); it != (*m_table)[slot].end(); ++it) {
            num_comparisons++;
            if(it->first == k) {
                (*m_table)[slot].erase(it);
                m_number_of_elements--;
                m_needs_update = true;      // sinaliza necessidade de atualizar vetor auxiliar
                return true;
            }
        }
        return false;
    }

    // Busca e retorna referência para o dado associado a chave
    Data& search(const Key& k) {
        size_t slot = hash_code(k);
        // Percorre o bucket correspondente
        for (auto& p : (*m_table)[slot]) {
            num_comparisons++;
            if (p.first == k) return p.second;
        }
        throw std::out_of_range("Key not found");
    }

    // Retorna true se a chave estiver presente
    bool contains(const Key& k) const {
        size_t slot = hash_code(k);
        for(const auto& p : (*m_table)[slot]) {
            num_comparisons++;
            if(p.first == k) {
                return true;
            }
        }
        return false;
    }

    // Atualiza o valor da chave (se existir)
    void at(const Key& k, const Data& d) {
        size_t slot = hash_code(k);
        for (auto& p : (*m_table)[slot]) {
            num_comparisons++;
            if (p.first == k) {
                p.second = d;
                m_needs_update = true;
                return;
            }
        }
        throw std::out_of_range("Key not found");
    }

    // Retorna uma referência ao valor associado à chave.
    // Se a chave não existir, insere um novo par com valor padrão.
    Data& operator[](const Key& k) {
        size_t slot = hash_code(k);
        for(auto& p : (*m_table)[slot]) {
            num_comparisons++;
            if(p.first == k) {
                return p.second;
            }
        }
        // chave não encontrada, insere com valor padrão
        (*m_table)[slot].emplace_back(k, Data());
        m_number_of_elements++;
        m_needs_update = true;
        return (*m_table)[slot].back().second;
    }

    // Retorna uma referência constante ao valor associado à chave.
    // Lança exceção std::out_of_range se a chave não for encontrada.
    const Data& operator[](const Key& k) const {
        size_t slot = hash_code(k);
        for(const auto& p : (*m_table)[slot]) {
            num_comparisons++;
            if(p.first == k) return p.second;
        }
        throw std::out_of_range("Key not found");
    }

    // Redimensiona a tabela e redistribui os elementos
    void rehash(size_t new_size) {
        if(new_size <= m_table_size) return;
        new_size = get_next_prime(new_size);
        auto new_table = new std::vector<std::list<std::pair<Key, Data>>>(new_size);
        // reinsere todos os pares existentes na nova tabela
        for(const auto& bucket : *m_table) {
            for(const auto& p : bucket) {
                size_t i = m_hashing(p.first) % new_size;
                (*new_table)[i].push_back(p);
            }
        }
        delete m_table;
        m_table = new_table;
        m_table_size = new_size;
        m_needs_update = true;
    }

    // Retorna o comprimento médio das listas não vazias (buckets com colisões).
    // Indica quantos elementos, em média, precisam ser percorridos por acesso.
    float average_access_length() const {
        int total = 0;
        int used = 0;

        for (const auto& bucket : *m_table) {
            if (!bucket.empty()) {
                total += bucket.size();
                used++;
            }
        }

        if (used == 0) return 0.0f;
        return static_cast<float>(total) / used;
    }

    // Retorna o comprimento máximo entre todas as listas (bucket com mais elementos).
    // Indica o pior caso de acesso em termos de colisões.
    int max_access_length() const {
        int max_len = 0;
        for (const auto& bucket : *m_table) {
            if (bucket.size() > max_len) {
                max_len = bucket.size();
            }
        }
        return max_len;
    }

    // Retorna o nome da estrutura (usado no relatório/saída)
    std::string name() const { return "Chained Hash Table"; }

    // Retorna um vetor contendo todos os pares armazenados na tabela hash
    std::vector<std::pair<Key, Data>> all_elements() const {
        std::vector<std::pair<Key, Data>> result;
        for (const auto& bucket : (*m_table)) {
            for (const auto& pair : bucket) {
                result.push_back(pair);
            }
        }
        return result;
    }

    // Classe iterator para percorrer a hash table em ordem linear de índice
    class iterator {
    private:
        const chained_hash_table* m_ht; // ponteiro para a hash table 
        size_t m_index;                 // índice atual

    public:
        // Construtor
        iterator(const chained_hash_table* ht, size_t i)
            : m_ht(ht), m_index(i) {}

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

    // Retorna o início da iteração
    // Se o vetor ainda não foi atualizado, atualiza antes
    iterator begin() {
        if(m_needs_update) update_sorted_pairs();
        return iterator(this, 0);
    }

    // Retorna o fim da iteração
    // Também atualiza se necessário
    iterator end() {
        if(m_needs_update) update_sorted_pairs();
        return iterator(this, m_sorted_pairs.size());
    }
};

#endif