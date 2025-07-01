#ifndef AVL_TREE_HPP
#define AVL_TREE_HPP

#include <stack>        // usado no iterador
#include <utility>      // std::pair
#include <algorithm>    // std::max
#include <stdexcept>
#include <string>

// Estrutura que representa um nó da árvore AVL
template <typename Key, typename Data>
struct avl_node {
    std::pair<Key, Data> key;   // par armazenado no nodo
    avl_node* m_left;           // ponteiro para o filho esquerdo
    avl_node* m_right;          // ponteiro para o filho direito
    int m_height;               // altura do nodo

    // Construtor do nó com chave e dado
    avl_node(Key k, Data d)
        : key(std::make_pair(k, d)), m_left(nullptr), m_right(nullptr), m_height(1) {}
};

// Classe genérica de uma árvore AVL
template <typename Key, typename Data, typename Compare = std::less<Key>>
class avl_tree {
private:
    avl_node<Key, Data>* m_root = nullptr;      // raiz da árvore
    unsigned int m_size = 0;                    // número de elementos na árvore
    Compare m_compare;                          // objeto de comparação
    unsigned int num_comparisons = 0;           // número de comparação realizadas
    unsigned int num_rotation = 0;              // número de rotações realizadas

    // Libera todos os nós da árvore
    void _clear(avl_node<Key, Data>* p) {
        if(p == nullptr) return;
        _clear(p->m_left);
        _clear(p->m_right);
        delete p;
    }

    // Retorna a altura de um nó, ou 0 se ele for nulo
    int _height(avl_node<Key, Data>* p) const {
        return p ? p->m_height : 0;
    }

    // Calcula o fator de balanceamento de um nó e o retorna
    int _balance(avl_node<Key, Data>* p) const {
        return _height(p->m_right) - _height(p->m_left);
    }

    // Rotação simples para a direita
    avl_node<Key, Data>* _right_rotation(avl_node<Key, Data>* p) {
        num_rotation++;
        avl_node<Key, Data>* u = p->m_left;
        p->m_left = u->m_right;
        u->m_right = p;

        // atualiza alturas após rotação
        p->m_height = 1 + std::max(_height(p->m_left), _height(p->m_right));
        u->m_height = 1 + std::max(_height(u->m_left), _height(u->m_right));
        return u;   // nova raiz da subárvore
    }

    // Rotação simples para a esquerda
    avl_node<Key, Data>* _left_rotation(avl_node<Key, Data>* p) {
        num_rotation++;
        avl_node<Key, Data>* u = p->m_right;
        p->m_right = u->m_left;
        u->m_left = p;

        // atualiza alturas após rotação
        p->m_height = 1 + std::max(_height(p->m_left), _height(p->m_right));
        u->m_height = 1 + std::max(_height(u->m_left), _height(u->m_right));
        return u;   // nova raiz da subárvore
    }

    // Insere recursivamente um novo nó e mantém a árvore balanceada
    avl_node<Key, Data>* _insert(avl_node<Key, Data>* p, Key k, Data d) {
        num_comparisons++;
        if(p == nullptr) {
            m_size++;
            return new avl_node<Key, Data>(k, d);   // insere novo nó
        }

        // decide para qual lado inserir
        num_comparisons++;
        if (m_compare(k, p->key.first)) {
            p->m_left = _insert(p->m_left, k, d);
        } 
        else if (m_compare(p->key.first, k)) {
            num_comparisons++;
            p->m_right = _insert(p->m_right, k, d);
        } else {
            return p;   // chave ja existe
        }

        // corrige e balanceia a subárvore após inserção
        return _fixup_avl_node(p, k);
    }

    // Rebalanceia o nó se estiver desbalanceado após inserção ou remoção
    avl_node<Key, Data>* _fixup_avl_node(avl_node<Key, Data>* p, Key k) {
        // atualiza a altura do nó
        p->m_height = 1 + std::max(_height(p->m_left), _height(p->m_right));
        int bal = _balance(p);  // calcula o fator de balanceamento

        // casos de rotação baseados no tipo de desbalanceamento
        num_comparisons++;
        if(bal < -1 && m_compare(k, p->m_left->key.first)) {
            return _right_rotation(p);
        }
        num_comparisons++;
        if(bal < -1 && m_compare(p->m_left->key.first, k)) {
            p->m_left = _left_rotation(p->m_left);
            return _right_rotation(p);
        }
        num_comparisons++;
        if(bal > 1 && m_compare(p->m_right->key.first, k)) {
            return _left_rotation(p);
        }
        num_comparisons++;
        if(bal > 1 && m_compare(k, p->m_right->key.first)) {
            p->m_right = _right_rotation(p->m_right);
            return _left_rotation(p);
        }
        return p;           // nó já balanceado
    }

    // Busca um nó pela chave
    avl_node<Key, Data>* _search(avl_node<Key, Data>* p, Key k) {
        num_comparisons++; 
        if(p == nullptr) return nullptr;    // chave não encontrada

        // se for igual, retorna o nó
        num_comparisons += 2;
        if(!m_compare(k, p->key.first) && !m_compare(p->key.first, k)) return p;

        // decide para onde continuar a busca
        num_comparisons++;
        return m_compare(k, p->key.first) ? _search(p->m_left, k) : _search(p->m_right, k);
    }

    // Remove um nó da árvore e mantém o balanceamento
    avl_node<Key, Data>* _remove(avl_node<Key, Data>* p, Key k) {
        if (p == nullptr) return nullptr;       // chave não encontrada

        num_comparisons++;
        // se a chave procurada é menor, vai para a subárvore esquerda
        if (m_compare(k, p->key.first)) {
            p->m_left = _remove(p->m_left, k);
        }
        // se a chave procurada é maior, vai para a subárvore direita
        else if (m_compare(p->key.first, k)) {
            num_comparisons++;
            p->m_right = _remove(p->m_right, k);
        }
        // Achou o nó a ser removido
        else {
            num_comparisons++;
            // caso com um ou nenhum filho
            if (p->m_left == nullptr || p->m_right == nullptr) {
                avl_node<Key, Data>* temp = p->m_left ? p->m_left : p->m_right;
                delete p;
                m_size--;
                return temp;
            }

            // caso com dois filhos: encontrar o menor da subárvore direita
            avl_node<Key, Data>* temp = p->m_right;
            while (temp->m_left != nullptr) {
                num_comparisons++;
                temp = temp->m_left;
            }

            // copia os dados do sucessor para o nó atual
            p->key = temp->key;

            // remove o sucessor
            p->m_right = _remove(p->m_right, temp->key.first);
        }

        return _fixup_avl_node(p, k);   // rebalanceia a árvore
    }

public:
    // Construtor da árvore AVL
    avl_tree() : m_compare(Compare()) {}

    // Destrutor: libera memória dos nós
    ~avl_tree() { _clear(m_root); }

    // Insere um novo elemento com chave e dado
    void insert(Key k, Data d) { m_root = _insert(m_root, k, d); }

    // Remove um elemento pela chave
    void remove(Key k) { m_root = _remove(m_root, k); }

    // Limpa todos os elementos da árvore
    void clear() { _clear(m_root); m_root = nullptr; m_size = 0; }

    // Retorna true se a árvore estiver vazia
    bool empty() const { return m_size == 0; }

    // Retorna o número total de elementos na árvore
    unsigned int size() const { return m_size; }

    // Retorna a quantidade de comparações feitas
    unsigned int comparisons() const { return num_comparisons; }

    // Retorna a quantidade de rotações feitas
    unsigned int rotations() const { return num_rotation; }

    // Busca e retorna referência para o dado associado à chave
    Data& search(Key k) {
        avl_node<Key, Data>* n = _search(m_root, k);
        if(n) return n->key.second;
        throw std::out_of_range("Key not found");   // lança erro se não achar
    }

    // Verifica se a chave existe na árvore
    bool contains(Key k) {
        return _search(m_root, k) != nullptr; 
    }

    // Atualiza o valor associado a uma chave
    void at(Key k, Data d) {
        avl_node<Key, Data>* n = _search(m_root, k);
        if (!n) throw std::out_of_range("Key not found");
        n->key.second = d;
    }

    // Retorna o nome da estrutura (usado no relatório/saída)
    std::string name() const {
        return "Árvore AVL";
    }

    // Classe interna que permite percorrer a árvore em ordem alfabética
    class iterator {
    private:
        std::stack<avl_node<Key, Data>*> _stack;    // pilha para simular travessia
        avl_node<Key, Data>* _current;              // nó atual da iteração

        // Empilha todos os nós à esquerda
        void _push_left(avl_node<Key, Data>* n) {
            while(n) { 
                _stack.push(n); 
                n = n->m_left; 
            }
        }

    public:
        // Construtor: começa a travessia pela raiz
        iterator(avl_node<Key, Data>* root) : _current(nullptr) {
            _push_left(root);   // empilha a parte esquerda
            if(!_stack.empty()) { 
                _current = _stack.top(); _stack.pop(); 
            }
        }

        // Avança para o próximo elemento em ordem
        iterator& operator++() {
            if(!_current) return *this;
            if(_current->m_right) { 
                _push_left(_current->m_right);
            }
            if(_stack.empty()){
                _current = nullptr;
            }
            else {
                _current = _stack.top();
                _stack.pop();
            }
            return *this;
        }

        // Compara dois iteradores (fim da travessia)
        bool operator!=(const iterator& other) const {
            return _current != other._current;
        }

        // Acessa o valor atual (par chave, dado)
        const std::pair<Key, Data>& operator*() const { return _current->key; }
        const std::pair<Key, Data>* operator->() const { return &_current->key; }
    };

    // Retorna o início da travessia (primeira chave em ordem)
    iterator begin() { return iterator(m_root); }

    // Retorna o fim da travessia
    iterator end() { return iterator(nullptr); }
};

#endif